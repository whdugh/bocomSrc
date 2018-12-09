// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
*   文件：LSServer.cpp
*   功能：丽水通讯类
*   作者：yufeng
*   时间：2010-8-31
**/
#include "Common.h"
#include "CommonHeader.h"
#include "LSServer.h"
#include "BrandSubSection.h"

mvCLSServer g_LSServer;

//监控线程
void* ThreadLSAccept(void* pArg)
{
	int nSocket =*(int*)pArg;
	//客户端连接
	struct sockaddr_in clientaddr;
	//长度
	socklen_t sin_size = sizeof(struct   sockaddr_in);
	int nClient;
	while(!g_bEndThread)
	{
		//接受连接
		if((nClient = accept(nSocket,(struct sockaddr*)&clientaddr,&sin_size)) ==  -1)
		{
			//断开连接
			if(g_bEndThread)
			{
	//		    LogNormal("11 accept exit\r\n");
				return pArg;
			}
	//		LogNormal("accept nClient = %d\r\n",nClient);
			//自动重启
			continue;
		}

		//输出用户连接
		LogNormal("中心端连接[IP:%s][nClient = %d,端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));

        g_LSServer.mvRecvCenterServerMsg(nClient);
		//10毫秒
		usleep(1000*10);
	}
	//LogNormal("22 accept exit\r\n");
	return pArg;
}

//记录发送线程
void* ThreadLSResult(void* pArg)
{
	//处理一条数据
	g_LSServer.DealResult();

    pthread_exit((void *)0);
	return pArg;
}


//历史记录发送线程
void* ThreadLSHistoryResult(void* pArg)
{
	if(access("LsTest.cfg",F_OK) == 0)
	{
		g_LSServer.mvTestRecord();	
	}
	else
	{
		g_LSServer.mvDealHistoryRecord();
	}
     
     pthread_exit((void *)0);
	return pArg;
}

mvCLSServer::mvCLSServer()
{
    m_nCenterSocket = 0;
    m_nAcceptSocket = 0;
    m_nPort = 41022;

    m_nThreadId = 0;
    m_nHistoryThreadId = 0;
	m_bAuthStatus = false;
    pthread_mutex_init(&m_Result_Mutex,NULL);
}


mvCLSServer::~mvCLSServer()
{
    mvCloseSocket(m_nAcceptSocket);
    pthread_mutex_destroy(&m_Result_Mutex);
}

//启动侦听服务
bool mvCLSServer::Init()
{
     //创建套接字
	if(mvCreateSocket(m_nAcceptSocket,1)==false)
	{
	    printf("创建套接字失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//重复使用套接字
	if(mvSetSocketOpt(m_nAcceptSocket,SO_REUSEADDR)==false)
	{
	    printf("设置重复使用套接字失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//////////////////////////
	//绑定服务端口
	if(mvBindPort(m_nAcceptSocket,m_nPort)==false)
	{
        printf("绑定到 %d 端口失败,服务无法启动!\r\n",m_nPort);
	    g_bEndThread = true;
		return false;
	}

	//开始监听
	if (mvStartListen(m_nAcceptSocket) == false)
	{
		printf("监听连接失败，服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动事件监控线程
	int nret=pthread_create(&id,&attr,ThreadLSAccept,(void*)&m_nAcceptSocket);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建接收线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动检测结果发送线程
	nret=pthread_create(&m_nThreadId,&attr,ThreadLSResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadLSHistoryResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);

	return true;
}


//释放
bool mvCLSServer::UnInit()
{
    //需要关闭所有连接
    mvCloseSocket(m_nAcceptSocket);
    //关闭连接
    mvCloseSocket(m_nCenterSocket);

    //停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}

	if(m_nHistoryThreadId != 0)
	{
		pthread_join(m_nHistoryThreadId,NULL);
		m_nHistoryThreadId = 0;
	}

    m_ResultList.clear();

	return true;
}

//接收中心端消息(消息需要立即处理)
bool mvCLSServer::mvRecvCenterServerMsg(int nSocket)
{
    //mvCloseSocket(m_nCenterSocket);
    m_nCenterSocket = nSocket;

    //mvBindPort(m_nCenterSocket, m_nPort);

    string strMsg("");
    //receive msg and push it into the msg queue.
    if (mvRecvMsg(nSocket, strMsg))
    {
        /*
        //建立临时文件
        FILE* fp = fopen("./LSLOG-sock.txt","a+");
        if(fp!=NULL)
        {
            LS_HEADER* mHeader = (LS_HEADER*)strMsg.c_str();
            fprintf(fp, "\n=====mHeader.uCmdID = %x==========mHeader.uCmdLen=%d====mHeader.uCmdCount=%d==\n", mHeader->uCmdID , mHeader->uCmdLen, mHeader->uCmdCount);
            fprintf(fp,"\n====mvCLSServer::mvRecvCenterServerMsg==m_nCenterSocket=%d=strMsg.size()=%d====m_nPort=%d=\n", m_nCenterSocket, strMsg.size(), m_nPort);
            fflush(fp);
            fwrite(strMsg.c_str(), strMsg.size(), 1, fp);

            fclose(fp);
        }
        */


        return mvOnDealOneMsg(strMsg);
    }
    else
    {
        return false;
    }
}


//中心端接收消息
bool mvCLSServer::mvRecvMsg(int nSocket, string& strMsg)
{
	if (!strMsg.empty())
    {
        strMsg.clear();
    }
    if (nSocket <= 0)
    {
        return false;
    }

	LS_HEADER mHeader;

    char chBuffer[SRIP_MAX_BUFFER];

    if (recv(nSocket, (void*)&mHeader,sizeof(mHeader), MSG_NOSIGNAL) < 0)
    {
        return false;
    }

    if(mHeader.uCmdLen < sizeof(mHeader))
    {
        return false;
    }
    strMsg.append((char*)&mHeader,sizeof(mHeader));


    int nLeft = mHeader.uCmdLen - sizeof(mHeader); //数据包体长度
    int nBytes = 0;

    while(nLeft >  0)
    {
        nBytes = recv(nSocket, chBuffer, nLeft, MSG_NOSIGNAL);
        if ( nBytes < 0)
        {
            return false;
        }
        //保存数据
        strMsg.append(chBuffer,nBytes);
        nLeft -= nBytes;
    }

    return (!strMsg.empty());
}


//处理命令
bool mvCLSServer::mvOnDealOneMsg(const string &strMsg)
{
    LS_HEADER* mHeader = (LS_HEADER*)strMsg.c_str();
/*
    //建立临时文件
	FILE* fp = fopen("./LSLOG-deal2.txt","wa+");
	int nWrite = 1;
	if(fp!=NULL)
	{
        fprintf(fp,"\n====mHeader->uCmdID=%d===strMsg.size()=%d====\n", mHeader->uCmdID, strMsg.size());
        fwrite(strMsg.c_str(), strMsg.size(), 1, fp);
        //fputs(strNewMsgFist, fp);

        //fprintf(fp, "\n=======nWrite=%d======\n", nWrite);
        fflush(fp);

        fclose(fp);
    }
*/

	if(mHeader->uCmdID == LS_AUTH_CODE)//中心端登录
	{
    /*******接收到：中心向前端设备发出验证包**************
    * 整体数据长度：96字节
    * 1	Char[44]	用户名--设备编号
    * 2	Char[44]	验证码
   ***************************************************/
        char chUser[44] = {0};
        char chUserKey[44] = {0};
        memcpy(chUser, strMsg.c_str() + sizeof(mHeader), 44);
        memcpy(chUserKey, strMsg.c_str() + sizeof(mHeader) + 44, 44);

        LogNormal("User:%s\n", chUser);
		LogNormal("UserKey:%s\n", chUserKey);


        //**********发出：前端设备向中心反馈包***********************
        //* 整体数据长度：12字节
        //* 1	UINT32	0：验证成功、1：设备编号错误、2：验证码错误
        //*********************************************************/
        string strNewMsg;

        UINT32 uRet = 0;

		/*if(strncmp(g_strDetectorID.c_str(),chUserKey,g_strDetectorID.size()) != 0)
		{
			m_bAuthStatus = false;
			uRet = 2;
		}
		else
		{
			m_bAuthStatus = true;
		}*/

        strNewMsg.append((char*)&uRet,sizeof(UINT32));

/*
        //建立临时文件
        fp = fopen("./LSLOG-auth.txt","wb+");
        if(fp!=NULL)
        {
            fprintf(fp,"\n========LS_AUTH_CODE=======\n");
            fflush(fp);
            fwrite(strNewMsg.c_str(), strNewMsg.size(), 1, fp);

            fclose(fp);
        }
        */
        if(mvRebMsgAndSend( LS_AUTH_REP, strNewMsg))
		{
			//验证失败断开连接
			/*if(!m_bAuthStatus)
			{
				LogNormal("验证失败断开连接\n");
				mvCloseSocket(m_nCenterSocket);
			}*/
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(mHeader->uCmdID == LS_PLATE_REP)
	{
    /*******接收到：中心向前端设备反馈接收数据反馈包**********
     * 整体数据长度：64字节
     *   1.		UINT64	数据编号
     *   2.		UINT32	回复接收数据状态：
     *                   0入库成功
     *                   1解析第一步失败
     *                   2解析第二步失败
     *                   3入库失败
     *   3.		Char[44]	通道编号
     *****************************************************/
        UINT64 uDataId = 0;
        UINT32 uDataState = 0;
        char chChannel[44] = {0};

        uDataId = *( (UINT64 *)(strMsg.c_str() + sizeof(mHeader)) );
        uDataState = *( (UINT32 *)(strMsg.c_str() + sizeof(mHeader) + sizeof(UINT64)) );
        memcpy(chChannel, strMsg.c_str() + sizeof(mHeader) + sizeof(UINT64) + sizeof(UINT32), 44);

        printf("===chChannel=%s===\n", chChannel);

        //处理接收到异常包，再次重发
        if(uDataState != 0)
        {
            //ERROR!!!!
            printf("===Center received Wrong Data!!!===\n");

        }

        return true;
	}
}

/*
* 函数介绍：发送心跳测试
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCLSServer::mvSendLinkTest()
{
    if(m_nCenterSocket == 0)
     return false;

    string strMsg("");
    return mvRebMsgAndSend(LS_LINK_TEST, strMsg);
}


/*
* 函数介绍：重组消息并发送
* 输入参数：pCode-要发送的消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCLSServer::mvRebMsgAndSend(UINT32 uCode, const string &strMsg)
{
    //rebuild the message according to the msg-code and then send it to the center server.
    string strFullMsg("");

    LS_HEADER mHeader;
    mHeader.uCmdID = uCode;
    mHeader.uCmdLen = sizeof(LS_HEADER) + strMsg.size();

    strFullMsg.append((char*)&mHeader, sizeof(mHeader));//包头

    if (!strMsg.empty())
    {
        strFullMsg += strMsg;//包体
    }

    //UINT32 c32 = Crc(strFullMsg.c_str(),strFullMsg.size());
    //strFullMsg.append((char*)&c32,sizeof(UINT32));//crc校验码

 /*   printf("mvRebMsgAndSend=%s,uCode=%x,strMsg.size()=%d,strFullMsg.size()=%d\n",strMsg.c_str(),uCode,strMsg.size(),strFullMsg.size());

    FILE *fp = fopen("./LSLOG-link.data","wb+");
    if(fp!=NULL)
    {
        fwrite(strFullMsg.c_str(), strFullMsg.size(), 1, fp);
        fclose(fp);
    }*/

    if (!mvSendMsgToSocket(m_nCenterSocket, strFullMsg))
    {
        mvCloseSocket(m_nCenterSocket);
        LogError("ls 发送消息失败，连接断开\n");
        return false;
    }

    return true;
}

/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCLSServer::mvSendRecordToCS(const string &strMsg)
{
    if(m_nCenterSocket == 0)
     return false;

    //including events and plates.
    //strMsg = MIMAX_HEADER+RECORD_EVENT/RECORD_PLATE+picture.
    char buf[24] = {0};

    //需要进行转换
    string strNewMsg("");
    string strMsgSend("");

    int nPicSize;
    MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();

    if (MIMAX_PLATE_REP == sHeader->uCmdID)
    {
        //图片的大小
        nPicSize = strMsg.size() - sizeof(MIMAX_HEADER) - sizeof(RECORD_PLATE);

        RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

    //一，关于全景图的信息
    /*
        1.	UINT64	图片编号，填默认值填0
        2.	UINT32	图片字节数
        3.	UINT32	关于图片的识别数据个数n条 //对于同一张图可有多条数据
        4.	UINT32	图片宽度
        5.	UINT32	图片高度
        6.	Char[48]	通道编号
        7.	Char[48]	图片存储路径，填默认值”\0”
    */
        LS_PIC_INFO picInfo;
        picInfo.uPicId = 0;
        picInfo.uPicSize = nPicSize;
        picInfo.uPicDataCount = 1;
        picInfo.uPicWidth = pPlate->uPicWidth;
        picInfo.uPicHeight = pPlate->uPicHeight;

	#ifdef LS_QINGTIAN_IVAP
		//设备编号
        sprintf(buf, "%15s%03d", g_strDetectorID.c_str(),pPlate->uChannelID);
	#else
		//设备编号
        sprintf(buf, "%15s%03d", g_strDetectorID.c_str(),sHeader->uCameraID);
	#endif
        
        //std::string strDetectorID(buf);
        //strNewMsg.append((char*)strDetectorID.c_str(), strDetectorID.size());

        memcpy(picInfo.chChannelId, buf, strlen(buf));
        memset(picInfo.chPicPath, 0, 48);

        printf("=====picInfo.chChannelId=%s===\n", picInfo.chChannelId);

        strNewMsg.append((char*)&picInfo, sizeof(LS_PIC_INFO));

    //二，图片数据
        //全景图信息
        strNewMsg.append(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE), nPicSize);

    //三，总数n条识别数据
    /*
    1.		UINT64	数据编号，填默认值0
    2.		UINT64	在设备上的数据编号8字节
    3.		time_t	过卡口时间8字节
    4.		UINT64	关联的图片信息编号，填默认值0
    5.		UINT32	过卡口时间毫秒
    6.		float	车辆速度单位公里
    7.		UINT32	车道号
    8.		float	识别精度
    9.		UINT32	方向代码
    10.		UINT32	车牌类型
    11.		UINT32	车牌颜色
    12.		UINT32	车辆类型
    13.		UINT32	车辆logo
    14.		UINT32	车辆颜色
    15.		UINT32	车牌结构
    16.		UINT32	在全景图中的特征图坐标x1如无填99999999
    17.		UINT32	在全景图中的特征图坐标y1如无填99999999
    18.		UINT32	在全景图中的特征图坐标x2如无填99999999
    19.		UINT32	在全景图中的特征图坐标y2如无填99999999
    20.		Char[20]	车牌号（未识别出以“00000000”八个零表示）
    */
        //1.数据编号，填默认值0
        UINT64 nDataId = 0;
        strNewMsg.append((char*)&nDataId, sizeof(UINT64));

        //2.在设备上的数据编号8字节
        UINT64 nID = pPlate->uSeq;
        strNewMsg.append((char*)&nID, sizeof(UINT64));

        //3.过卡口时间秒8字节
        UINT64 tm1 = pPlate->uTime;
        strNewMsg.append((char*)&tm1, sizeof(UINT64));

        //4.关联的图片信息编号，填默认值0
        UINT64 uPicId = 0;
        strNewMsg.append((char*)&uPicId, sizeof(UINT64));

        //5.过卡口时间毫秒
        UINT32 uMiTime = pPlate->uMiTime;
        strNewMsg.append((char*)&uMiTime, sizeof(UINT32));

        //6.车辆速度单位公里(float类型)
        float fSpeed = pPlate->uSpeed;
        strNewMsg.append((char*)&fSpeed, sizeof(float));

        //7.车道号
        UINT32 uRoadWayID = pPlate->uRoadWayID;
        strNewMsg.append((char*)&uRoadWayID, sizeof(UINT32));

        //8.识别精度(float类型)
        float fCredit = pPlate->uCredit;
        strNewMsg.append((char*)&fCredit, sizeof(float));

        //9.方向代码
        //行驶方向
        UINT32 uDirection = 0;
        switch(pPlate->uDirection)
        {
            case 1:
                uDirection = 1;
                break;
            case 2:
                uDirection = 2;
                break;
            case 3:
                uDirection = 3;
                break;
            case 4:
                uDirection = 4;
                break;
            case 5:
                uDirection = 6;
                break;
            case 6:
                uDirection = 8;
                break;
            case 7:
                uDirection = 5;
                break;
            case 8:
                uDirection = 7;
                break;
            default:
                uDirection = 99;
                break;
        }
        strNewMsg.append((char*)&uDirection, sizeof(UINT32));

        //10.车牌类型
        UINT32 uType = 0;
        switch(pPlate->uType)
        {
            case 1:
                uType = 2; //小车
                break;
            case 3:
                uType = 1; //大车
                break;
            default:
                uType = 99;
        }
        strNewMsg.append((char*)&uType, sizeof(UINT32));


        //11.车牌颜色
        UINT32 uPlateColor = pPlate->uColor;
        switch(uPlateColor)
        {
            case 1: //blue
                uPlateColor = 1;
                break;
            case 2: //black
                uPlateColor = 2;
                break;
            case 3: //yellow
                uPlateColor = 3;
                break;
            case 4: //white
                uPlateColor = 4;
                break;
            default: //other
                uPlateColor = 99;
        }
        //车牌颜色
        strNewMsg.append((char*)&uPlateColor, sizeof(UINT32));

        //12.车辆类型
        UINT32 uTypeDetail = 0;
        switch(pPlate->uTypeDetail)
        {
        case BUS_TYPE:
            uTypeDetail = 3001;//"大型客车";
            break;
        case TRUCK_TYPE:
            uTypeDetail = 3002;//"大型货车";
            break;
        case MIDDLEBUS_TYPE:
            uTypeDetail = 2001;//"中型客车";
            break;
        case TAXI:
            uTypeDetail = 1001;//"小型客车";
            break;
        case MINI_TRUCK:
            uTypeDetail = 1002;//"小型货车";
            break;
        case TWO_WHEEL_TYPE:
            uTypeDetail = 9001;//"两轮车";
            break;
        default:
            if(pPlate->uType == SMALL_CAR)
            {
                uTypeDetail = 1000;//"小车";
            }
            else if(pPlate->uType == MIDDLE_CAR)
            {
                uTypeDetail = 2000;//"中车";
            }
            else if(pPlate->uType == BIG_CAR)
            {
                uTypeDetail = 3000;//"大车";
            }
            else
            {
                uTypeDetail = 9000;//"";
            }
            break;
        }
        //车辆类型细分
        strNewMsg.append((char*)&uTypeDetail, sizeof(UINT32));


        //13.产商标志(车辆logo)
        UINT32 uCarBrand = pPlate->uCarBrand;
		#ifdef GLOBALCARLABEL
		#ifndef DETAIL_OLDBRAND
		CBrandSusection brandsub;
		uCarBrand = brandsub.GetOldBrandFromDetail(pPlate->uCarBrand+pPlate->uDetailCarBrand);
		#endif
		#endif
        if(uCarBrand < 100)
        {
            uCarBrand ++; //匹配丽水协议base 1
        }
        else
        {
            uCarBrand = 9999; //其他类型
        }
        //车辆logo
        strNewMsg.append((char*)&uCarBrand, sizeof(UINT32));

        //14.车身颜色
        UINT32  uCarColor = 0;
        switch(pPlate->uCarColor1)
        {
            case 0:
                uCarColor = 1;
                break;
            case 1:
                uCarColor = 2;
                break;
            case 2:
                uCarColor = 3;
                break;
            case 3:
                uCarColor = 4;
                break;
            case 4:
                uCarColor = 5;
                break;
            case 5:
                uCarColor = 6;
                break;
            case 6:
                uCarColor = 7;
                break;
            case 7:
                uCarColor = 8;
                break;
            case 8:
                uCarColor = 9;
                break;
            case 9:
                uCarColor = 10;
                break;
            case 10:
                uCarColor = 11;
                break;
            default:
                uCarColor = 99;
        }
        //车辆颜色
        strNewMsg.append((char*)&uCarColor, sizeof(UINT32));

        //15.车牌结构
        UINT32 uPlateType = 0;
        switch(pPlate->uPlateType)
        {
            case 1:
                uPlateType = 1; //单行
                break;
            case 2:
                uPlateType = 2; //双行
                break;
            default:
                uPlateType = 99; //其他
        }
        //车牌结构
        strNewMsg.append((char*)&uPlateType, sizeof(UINT32));

    //特征图信息-暂时填空
        //16.x1
        UINT32 uPosLeft = pPlate->uPosLeft;				//车牌在全景图片中的位置左
        //17.y1
        UINT32 uPosTop = pPlate->uPosTop;				//车牌在全景图片中的位置上
        //18.x2
        UINT32 uPosRight = pPlate->uPosRight;			//车牌在全景图片中的位置右
        //19.y2
        UINT32 uPosBottom = pPlate->uPosBottom;			//车牌在全景图片中的位置下

        if(uPosLeft == 0 && uPosTop == 0 && uPosRight == 0 && uPosBottom == 0)
        {
            uPosLeft = 99999999;
            uPosTop = 99999999;
            uPosRight = 99999999;
            uPosBottom = 99999999;
        }

        strNewMsg.append((char*)&uPosLeft, sizeof(UINT32));
        strNewMsg.append((char*)&uPosTop, sizeof(UINT32));
        strNewMsg.append((char*)&uPosRight, sizeof(UINT32));
        strNewMsg.append((char*)&uPosBottom, sizeof(UINT32));

        //20.车牌号
        char chCarNumber[20] = {0};
        memcpy(chCarNumber, pPlate->chText, strlen(pPlate->chText));
        //std::string strCarNumber(chCarNumber);
        strNewMsg.append((char*)&chCarNumber, 20);
		
#ifdef LS_DEBUG
	LogNormal("g_strDetectorID:%s uCameraID:%d", g_strDetectorID.c_str(), sHeader->uCameraID);
	LogNormal("picInfo chChannelId %s ", picInfo.chChannelId);
	LogNormal("id:%d %s vts:%d", pPlate->uChannelID, pPlate->chText, pPlate->uViolationType);
#endif
    }
    else if (MIMAX_EVENT_REP == sHeader->uCmdID)
    {
        //图片的大小
        nPicSize = strMsg.size() - sizeof(MIMAX_HEADER) - sizeof(RECORD_EVENT);

        RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

//一，关于全景图的信息
        LS_PIC_INFO picInfo;
        picInfo.uPicId = 0;
        picInfo.uPicSize = nPicSize;
        picInfo.uPicDataCount = 1;
        picInfo.uPicWidth = pEvent->uPicWidth;
        picInfo.uPicHeight = pEvent->uPicHeight;
        //20.设备编号

	#ifdef LS_QINGTIAN_IVAP
		sprintf(buf, "%15s%03d", g_strDetectorID.c_str(),pEvent->uChannelID);
	#else
        sprintf(buf, "%15s%03d", g_strDetectorID.c_str(),sHeader->uCameraID);
	#endif
		
        //std::string strDetectorID(buf);
        //strNewMsg.append((char*)strDetectorID.c_str(), strDetectorID.size());

        memcpy(picInfo.chChannelId, buf, strlen(buf));
        memset(picInfo.chPicPath, 0, 48);

        printf("=====picInfo.chChannelId=%s===\n", picInfo.chChannelId);

        strNewMsg.append((char*)&picInfo, sizeof(LS_PIC_INFO));

    //二，图片数据
        //23.全景图信息
        strNewMsg.append(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_EVENT), nPicSize);

    //三，总数n条识别数据
        //1.数据编号，填默认值0
        UINT64 nDataId = 0;
        strNewMsg.append((char*)&nDataId, sizeof(UINT64));

        //2.在设备上的数据编号8字节
        UINT64 nID = pEvent->uSeq;
        strNewMsg.append((char*)&nID, sizeof(UINT64));

        //3.过卡口时间秒8字节
        UINT64 tm1 = pEvent->uEventBeginTime;
        strNewMsg.append((char*)&tm1, sizeof(UINT64));

        //4.关联的图片信息编号，填默认值0
        UINT64 uPicId = 0;
        strNewMsg.append((char*)&uPicId, sizeof(UINT64));

        //5.过卡口时间毫秒
        UINT32 uMiTime = pEvent->uMiEventBeginTime;
        strNewMsg.append((char*)&uMiTime, sizeof(UINT32));

        //6.车辆速度单位公里(float类型)
        float fSpeed = pEvent->uSpeed;
        strNewMsg.append((char*)&fSpeed, sizeof(float));

        //7.车道号
        UINT32 uRoadWayID = pEvent->uRoadWayID;
        strNewMsg.append((char*)&uRoadWayID, sizeof(UINT32));

        //8.识别精度(float类型)
        float fCredit = 99.0;
        strNewMsg.append((char*)&fCredit, sizeof(float));

        //9.方向代码
        //行驶方向
        UINT32 uDirection = 0;
        switch(pEvent->uDirection)
        {
            case 1:
                uDirection = 1;
                break;
            case 2:
                uDirection = 2;
                break;
            case 3:
                uDirection = 3;
                break;
            case 4:
                uDirection = 4;
                break;
            case 5:
                uDirection = 6;
                break;
            case 6:
                uDirection = 8;
                break;
            case 7:
                uDirection = 5;
                break;
            case 8:
                uDirection = 7;
                break;
            default:
                uDirection = 99;
                break;
        }
        strNewMsg.append((char*)&uDirection, sizeof(UINT32));

        //10.车牌类型
        UINT32 uType = 99;
        strNewMsg.append((char*)&uType, sizeof(UINT32));


        //11.车牌颜色
        UINT32 uPlateColor = 99;
        //车牌颜色
        strNewMsg.append((char*)&uPlateColor, sizeof(UINT32));

        //12.车辆类型
        UINT32 uTypeDetail = 0;
        switch(pEvent->uDetailCarType)
        {
        case BUS_TYPE:
            uTypeDetail = 3001;//"大型客车";
            break;
        case TRUCK_TYPE:
            uTypeDetail = 3002;//"大型货车";
            break;
        case MIDDLEBUS_TYPE:
            uTypeDetail = 2001;//"中型客车";
            break;
        case TAXI:
            uTypeDetail = 1001;//"小型客车";
            break;
        case MINI_TRUCK:
            uTypeDetail = 1002;//"小型货车";
            break;
        case TWO_WHEEL_TYPE:
            uTypeDetail = 9001;//"两轮车";
            break;
        default:
            if(pEvent->uType == SMALL_CAR)
            {
                uTypeDetail = 1000;//"小车";
            }
            else if(pEvent->uType == MIDDLE_CAR)
            {
                uTypeDetail = 2000;//"中车";
            }
            else if(pEvent->uType == BIG_CAR)
            {
                uTypeDetail = 3000;//"大车";
            }
            else
            {
                uTypeDetail = 9000;//"";
            }
            break;
        }
        //车辆类型细分
        strNewMsg.append((char*)&uTypeDetail, sizeof(UINT32));


        //13.产商标志(车辆logo)
        UINT32 uCarBrand = 9999; //其他类型
        //车辆logo
        strNewMsg.append((char*)&uCarBrand, sizeof(UINT32));

        //14.车身颜色
        UINT32  uCarColor = 0;
        switch(pEvent->uColor1)
        {
            case 0:
                uCarColor = 1;
                break;
            case 1:
                uCarColor = 2;
                break;
            case 2:
                uCarColor = 3;
                break;
            case 3:
                uCarColor = 4;
                break;
            case 4:
                uCarColor = 5;
                break;
            case 5:
                uCarColor = 6;
                break;
            case 6:
                uCarColor = 7;
                break;
            case 7:
                uCarColor = 8;
                break;
            case 8:
                uCarColor = 9;
                break;
            case 9:
                uCarColor = 10;
                break;
            case 10:
                uCarColor = 11;
                break;
            default:
                uCarColor = 99;
        }
        //车辆颜色
        strNewMsg.append((char*)&uCarColor, sizeof(UINT32));

        //15.车牌结构
        UINT32 uPlateType = 99; //其他
        //车牌结构
        strNewMsg.append((char*)&uPlateType, sizeof(UINT32));

    //特征图信息-暂时填空
        //16.x1
        UINT32 uPosLeft = 0;				//车牌在全景图片中的位置左
        //17.y1
        UINT32 uPosTop = 0;				//车牌在全景图片中的位置上
        //18.x2
        UINT32 uPosRight = 0;			//车牌在全景图片中的位置右
        //19.y2
        UINT32 uPosBottom = 0;		//车牌在全景图片中的位置下

        if(uPosLeft == 0 && uPosTop == 0 && uPosRight == 0 && uPosBottom == 0)
        {
            uPosLeft = 99999999;
            uPosTop = 99999999;
            uPosRight = 99999999;
            uPosBottom = 99999999;
        }

        strNewMsg.append((char*)&uPosLeft, sizeof(UINT32));
        strNewMsg.append((char*)&uPosTop, sizeof(UINT32));
        strNewMsg.append((char*)&uPosRight, sizeof(UINT32));
        strNewMsg.append((char*)&uPosBottom, sizeof(UINT32));

        //20.车牌号
        char chCarNumber[20] = "00000000";
        //std::string strCarNumber(chCarNumber);
        //strNewMsg.append((char*)strCarNumber.c_str(), strCarNumber.size());
        strNewMsg.append((char*)&chCarNumber, 20);
    }

    printf("\n================发送====数据=====>>>到丽水中心端!=========\n");
    /* FILE *fp = fopen("./LSLOG-link.data","wb");
    if(fp!=NULL)
    {
        fwrite(strNewMsg.c_str(), strNewMsg.size(), 1, fp);
        fclose(fp);
    }*/

    return mvRebMsgAndSend(LS_PLATE_INFO, strNewMsg);
}


//添加一条数据
bool mvCLSServer::AddResult(std::string& strResult)
{
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strResult.c_str();

	switch(sDetectHeader->uDetectType)
	{
       case MIMAX_EVENT_REP:	//事件(送中心端)
	   case MIMAX_PLATE_REP:   //车牌
	   case MIMAX_STATISTIC_REP:  //统计
	   case PLATE_LOG_REP:   //日志
	   case EVENT_LOG_REP:
	        //加锁
	        pthread_mutex_lock(&m_Result_Mutex);
	        if(m_ResultList.size() > 3)//防止堆积的情况发生
	        {
	            //LogError("记录过多，未能及时发送!\r\n");
                m_ResultList.pop_back();
	        }
			m_ResultList.push_front(strResult);
			//解锁
	        pthread_mutex_unlock(&m_Result_Mutex);
			break;
	   default:
			LogError("未知数据[%x],取消操作!\r\n",sDetectHeader->uDetectType);
			return false;
	}
	return true;
}

//处理检测结果
bool mvCLSServer::OnResult(std::string& result)
{
	//发送数据
	MIMAX_HEADER mHeader;
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();

	bool bSendToServer = false;
	bool bObject = false;

	switch(sDetectHeader->uDetectType)
	{
			////////////////////////////////////////////
		case MIMAX_EVENT_REP:	//事件(送中心端)
		case MIMAX_STATISTIC_REP:  //统计
		case PLATE_LOG_REP:  //日志
		case EVENT_LOG_REP:
		case MIMAX_PLATE_REP:  //车牌
			{
				mHeader.uCmdID = (sDetectHeader->uDetectType);
				mHeader.uCmdFlag = sDetectHeader->uRealTime;
				mHeader.uCameraID = sDetectHeader->uChannelID;
			//	printf(" mHeader.uCmdID=%x ,sizeof(sDetectHeader)=%d\r\n",mHeader.uCmdID,sizeof(SRIP_DETECT_HEADER));
				//需要去掉SRIP_DETECT_HEADER头
				result.erase(0,sizeof(SRIP_DETECT_HEADER));
				bSendToServer = true;
			}
			break;
		default:
			LogError("未知数据[%d],取消操作!\r\n",sDetectHeader->uChannelID);
			return false;
	}

    //此时去取图片
    if(mHeader.uCmdID == MIMAX_EVENT_REP)
    {
        if( (mHeader.uCmdFlag & 0x00010000) == 0x00010000)
        {
            bObject = true;
            mHeader.uCmdFlag = 0x00000001;
        }
        RECORD_EVENT* sEvent = (RECORD_EVENT*)(result.c_str());
        String strPicPath(sEvent->chPicPath);
        result = result + GetImageByPath(strPicPath);
    }
    else if(mHeader.uCmdID == MIMAX_PLATE_REP)
    {
        RECORD_PLATE* sPlate = (RECORD_PLATE*)(result.c_str());
        String strPicPath(sPlate->chPicPath);
        result = result + GetImageByPath(strPicPath);
    }

    //数据长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + result.size();
	//添加头
	result.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

    //发送数据
    if(bSendToServer)
    {
        if(5 == g_nServerType)//丽水中心端
        {
            if( (mHeader.uCmdID == MIMAX_EVENT_REP)||(mHeader.uCmdID == MIMAX_PLATE_REP))
            {
                if(g_LSServer.mvSendRecordToCS(result))
                {
                    unsigned int uSeq =*((unsigned int*)(result.c_str()+sizeof(MIMAX_HEADER)));
                    if(bObject)
                    {
                        g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP,uSeq);
                    }
                    else
                    {
                        g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

//处理实时数据
void mvCLSServer::DealResult()
{
    while(!g_bEndThread)
	{
		std::string response1;
		//////////////////////////////////////////////////////////先取检测
	    //加锁
	    pthread_mutex_lock(&m_Result_Mutex);

		//判断是否有命令
		if(m_ResultList.size()>0)
		{
			//取最早命令
			LSResultMsg::iterator it = m_ResultList.begin();
			//保存数据
			response1 = *it;
			//删除取出的命令
			m_ResultList.pop_front();
		}
		//解锁
		pthread_mutex_unlock(&m_Result_Mutex);

		//处理消息
		if(response1.size()>0)
		{
			OnResult(response1);
		}

		//1毫秒
		usleep(1000*1);
	}
}

/*
* 函数介绍：获取一条历史记录
* 输入参数：strMsg-要获取的历史记录存储变量
* 输出参数：strMsg-获取的历史记录
* 返回值 ：成功返回true，否则返回false
*/
void mvCLSServer::mvDealHistoryRecord()
{
	 while(!g_bEndThread)
     {	
		if(g_nSendHistoryRecord == 1)
		{
			//车牌记录
			std::list<unsigned int> listSeq;
			listSeq.clear();
			StrList strListRecord;
			strListRecord.clear();
			if(g_skpDB.GetPlateHistoryRecord(strListRecord))
			{
					StrList::iterator it_b = strListRecord.begin();
					while(it_b != strListRecord.end())
					{
						string strPlate("");
						strPlate = *it_b;
						
						RECORD_PLATE* sPlate = (RECORD_PLATE*)(strPlate.c_str()+sizeof(MIMAX_HEADER));

						UINT32 uSeq = *(UINT32*)(strPlate.c_str()+sizeof(MIMAX_HEADER));

						String strPicPath(sPlate->chPicPath);
						string strPic = GetImageByPath(strPicPath);
						MIMAX_HEADER* pHeader = (MIMAX_HEADER*)strPlate.c_str();
						pHeader->uCmdLen += strPic.size();
						strPlate.append((char*)strPic.c_str(),strPic.size());
								
						bool bSendStatus = false;
						bSendStatus = mvSendRecordToCS(strPlate);

						if(bSendStatus)
						{
							listSeq.push_back(uSeq);
							sleep(5);
						}
						it_b++;
					}
					if(listSeq.size() > 0)
					g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, listSeq);
			}
			else
			{
					sleep(60);
			}
		}

		//5秒
		sleep(30);
     }
}

//测试图片发送机制
void mvCLSServer::mvTestRecord()
{
	String strPicPath = "LsTest.jpg";
	string strPic = GetImageByPath(strPicPath);

	string strPlate("");
	if(strPic.size() > 0)
	{
			MIMAX_HEADER Header;
			Header.uCmdID = MIMAX_PLATE_REP;
			Header.uCameraID = 1;
			Header.uCmdLen = strPic.size()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE);

			RECORD_PLATE Plate;
			Plate.uPicWidth = 1600;
			Plate.uPicHeight = 1200;
			Plate.uChannelID = 1;
			Plate.uDirection = 1;
			Plate.uType = 1;
			Plate.uColor = 1;
			Plate.uCredit = 90;
			Plate.uRoadWayID = 1;
			Plate.uTypeDetail = TAXI;
			Plate.uCarBrand = 1;
			Plate.uCarColor1 = 1;
			Plate.uPlateType = 1;
			memcpy(Plate.chText,"沪A12345",9);
			Plate.uPosLeft = 100;
			Plate.uPosTop = 100;
			Plate.uPosRight = 200;
			Plate.uPosBottom = 200;
			Plate.uSpeed = 10;
			Plate.uTime = GetTimeStamp();
			Plate.uMiTime = 0;
			Plate.uSeq = 1;

			strPlate.append((char*)&Header,sizeof(MIMAX_HEADER));
			strPlate.append((char*)&Plate,sizeof(RECORD_PLATE));
			strPlate.append((char*)strPic.c_str(),strPic.size());
	}

	 while(!g_bEndThread)
     {
		if(strPlate.size() > 0)
		{
			mvSendRecordToCS(strPlate);
		}
		usleep(10);
     }
}
