// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
*   文件：TravelServer.cpp
*   功能：旅行时间通讯类
*   作者：yuwenxian
*   时间：2010-8-4
**/

#include "Common.h"
#include "CommonHeader.h"
#include "TravelServer.h"
#include "RoadXmlData.h"
#include "XmlParaUtil.h"
#include "FtpCommunication.h"

//#define TravelCS_FILE_LOG

#ifdef TravelCS_FILE_LOG
        FILE *g_pCSRecvLog;
        FILE *g_pCSSendLog;
        FILE *g_pCSConnLog;
#endif

mvCTravelServer g_TravelServer;

//监控线程
void* ThreadTravelAccept(void* pArg)
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

        /*struct timeval timeo;
        socklen_t len = sizeof(timeo);
        timeo.tv_sec=5;
        timeo.tv_usec=0;//超时5s
        setsockopt(nClient, SOL_SOCKET, SO_RCVTIMEO, &timeo, len);*/


        #ifdef TravelCS_FILE_LOG
        fprintf(g_pCSConnLog, "before mvRecvCenterServerMsg=%s!\n",GetTimeCurrent().c_str());
        fflush(g_pCSConnLog);
        #endif

		//接收并处理消息
		g_TravelServer.mvRecvCenterServerMsg(nClient);

		//关闭套接字
		g_TravelServer.mvCloseSocket(nClient);


		#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSConnLog, "after mvRecvCenterServerMsg=%s!\n",GetTimeCurrent().c_str());
        fflush(g_pCSConnLog);
        #endif

		//10毫秒
		usleep(1000*10);
	}
	//LogNormal("22 accept exit\r\n");
	return pArg;
}

/*
* 函数介绍：接收消息线程入口
* 输入参数：pArg-线程入口参数，接收套接字
* 输出参数：无
* 返回值 ：无
*/
void *RecvTravelMsgThread(void *pArg)
{
    while (!g_bEndThread)
    {
        int nSocket = g_TravelServer.mvGetSocket();
        g_TravelServer.mvRecvCenterServerMsg(nSocket);

        usleep(100);
    }

    LogError("接收中心端消息线程退出\r\n");
    pthread_exit((void *)0);
}

//历史视频发送线程
void *HistoryVideoThread(void *pArg)
{
    if(!g_bEndThread)
    {
        g_TravelServer.mvDealHistoryVideo();
    }
    pthread_exit((void *)0);
}

//事件录像发送线程
void *EventVideoThread(void *pArg)
{
    if(!g_bEndThread)
    {
        g_TravelServer.mvDealEventVideo();
    }
    pthread_exit((void *)0);
}

//实时记录发送线程
void* ThreadTravelRecorderResult(void* pArg)
{
	//取类指针
	mvCTravelServer* pTravelServer = (mvCTravelServer*)pArg;
	if(pTravelServer == NULL) return pArg;

	//处理一条数据
	pTravelServer->DealResult();
    pthread_exit((void *)0);
	return pArg;
}

//历史记录发送线程
void* ThreadHistoryRecorderResult(void* pArg)
{
	//取类指针
	mvCTravelServer* pTravelServer = (mvCTravelServer*)pArg;
	if(pTravelServer == NULL) return pArg;

	//处理一条数据
	pTravelServer->DealHistoryResult();
    pthread_exit((void *)0);
	return pArg;
}

//ftp心跳检测线程
void* ThreadFTPLinkTest(void* pArg)
{
	//取类指针
	mvCTravelServer* pTravelServer = (mvCTravelServer*)pArg;
	if(pTravelServer == NULL) return pArg;

	//处理一条数据
	pTravelServer->DealFTPLinkTest();
    pthread_exit((void *)0);
	return pArg;
}

mvCTravelServer::mvCTravelServer()
{
#ifdef TravelCS_FILE_LOG
    g_pCSRecvLog = fopen("cs_recv.log", "w");
    g_pCSSendLog = fopen("cs_send.log", "w");
    g_pCSConnLog = fopen("cs_conn.log", "w");
#endif
    m_uCSLinkTime = 0;
    m_nCSLinkCount = 0;
    m_uRecvCSMsgThreadId = 0;
    m_bCenterLink = false;
    m_nCenterSocket = 0;
    m_nAcceptSocket = 0;

    m_nPort = 12350;
    //m_nCommunicationMode = 0;
    m_bAuthenticationOK = false;
    pthread_mutex_init(&m_mutexMsg, NULL);
    pthread_mutex_init(&m_Result_Mutex,NULL);

	m_nThreadId = 0;
	m_nHistoryThreadId = 0;
	m_nGapTime = 0;
	m_bFtpLink = false;
}


mvCTravelServer::~mvCTravelServer()
{
#ifdef TravelCS_FILE_LOG
    if (g_pCSRecvLog != NULL)
    {
        fclose(g_pCSRecvLog);
        g_pCSRecvLog = NULL;
    }
    if (g_pCSSendLog != NULL)
    {
        fclose(g_pCSSendLog);
        g_pCSSendLog = NULL;
    }
    if (g_pCSConnLog != NULL)
    {
        fclose(g_pCSConnLog);
        g_pCSConnLog = NULL;
    }
#endif
    if (!m_mapCSMsg.empty())
    {
        m_mapCSMsg.clear();
    }

    pthread_mutex_destroy(&m_mutexMsg);
    pthread_mutex_destroy(&m_Result_Mutex);

}


//启动侦听服务
bool mvCTravelServer::Init()
{
    system("rm -rf *.XML");
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
	int nret=pthread_create(&id,&attr,ThreadTravelAccept,(void*)&m_nAcceptSocket);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建接收线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动ftp心跳检测线程
	/*nret=pthread_create(&id,&attr,ThreadFTPLinkTest,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建ftp心跳检测线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}*/

    //启动检测结果发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	nret=pthread_create(&m_nThreadId,&attr,ThreadTravelRecorderResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动历史检测结果发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadHistoryRecorderResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建历史检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);

	g_nControlServerPort = 12349;

	return true;
}

//释放
bool mvCTravelServer::UnInit()
{
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

	mvCloseSocket(m_nAcceptSocket);

	g_FtpCommunication.DoClose();

    m_ChannelResultList.clear();
	return true;
}

//创建录像记录发送线程
bool mvCTravelServer::CreateVideoSendThread(string& strVideoPath)
{
    m_strEventVideoName = strVideoPath;
    //需要启动线程发送历史视频(FTP方式)
    pthread_attr_t   attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t id;
    if (pthread_create(&id, &attr, EventVideoThread,NULL) != 0)
    {
        pthread_attr_destroy(&attr);
        return false;
    }
    pthread_attr_destroy(&attr);
    return true;
}

//处理实时记录结果
void mvCTravelServer::DealResult()
{

    while(!g_bEndThread)
	{
		std::string response1;
		//////////////////////////////////////////////////////////先取检测
	    //加锁
	    pthread_mutex_lock(&m_Result_Mutex);

		//判断是否有命令
		if(m_ChannelResultList.size()>0)
		{
			//取最早命令
			TravelResultMsg::iterator it = m_ChannelResultList.begin();
			//保存数据
			response1 = *it;
			//删除取出的命令
			m_ChannelResultList.pop_front();
		}
		//解锁
		pthread_mutex_unlock(&m_Result_Mutex);

		//处理消息
		if(response1.size()>0)
		{
			OnResult(response1);
		}
		//1毫秒
		usleep(1000*100);
	}
}

//处理历史记录结果
void mvCTravelServer::DealHistoryResult()
{
    int nCount = 0;
    while(!g_bEndThread)
	{
		if(g_nServerType == 3)
		{
			if(nCount >= 3600)
			{
				//定时发送日志
				mvSendLogData();

				nCount = 0;
			}

			//每隔1小时校时一次
			if(nCount%700 == 0)
			{
				SynClock();
			}

			//每隔半小时获取一次密码
			if(nCount%300 == 0)
			{
				GetFtpLoginInfo();
			}
		}

         //5秒
		sleep(5);

        //处理历史记录
        mvDealHistoryRecord();

		nCount++;
	}
}

//添加一条数据
bool mvCTravelServer::AddResult(std::string& strMsg)
{
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strMsg.c_str();

	switch(sDetectHeader->uDetectType)
	{
       case MIMAX_EVENT_REP:	//事件(送中心端)
	   case MIMAX_PLATE_REP:   //车牌
	   case MIMAX_STATISTIC_REP:  //统计
	   case PLATE_LOG_REP:   //日志
	   case EVENT_LOG_REP:
	        //加锁
	        pthread_mutex_lock(&m_Result_Mutex);
	        if(m_ChannelResultList.size() > 5)//防止堆积的情况发生
	        {
	            //LogError("记录过多，未能及时发送!\r\n");
                m_ChannelResultList.pop_back();
	        }
			m_ChannelResultList.push_front(strMsg);
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
bool mvCTravelServer::OnResult(std::string& result)
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
        if(3 == g_nServerType || 22 == g_nServerType)//旅行时间
        {
            if( (mHeader.uCmdID == MIMAX_EVENT_REP)||
               (mHeader.uCmdID == MIMAX_PLATE_REP)||
               (mHeader.uCmdID == MIMAX_STATISTIC_REP)||
               (mHeader.uCmdID == PLATE_LOG_REP))
            {
                if(!bObject)//
                {
                    if (g_TravelServer.mvSendRecordToCS(result))
                    {
                        unsigned int uSeq =*((unsigned int*)(result.c_str()+sizeof(MIMAX_HEADER)));
                        g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
                        return true;
                    }
                }
                else
                {
                    //目标检测出的行人等无需发送给中心端
                    unsigned int uSeq =*((unsigned int*)(result.c_str()+sizeof(MIMAX_HEADER)));
                    g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
                    return true;
                }
            }
        }
    }
    return false;
}

//ftp心跳处理线程
void mvCTravelServer::DealFTPLinkTest()
{
    while(!g_bEndThread)
    {
        sleep(5);

        //获取ftp密码
        GetFtpLoginInfo();

        //判断ftp连接是否正常
        if(g_FtpCommunication.CheckFds() == 0)
        {
            #ifdef TravelCS_FILE_LOG
            fprintf(g_pCSSendLog, "\n连接ftp中心端(ip:%s,port:%d,user:%s,pass:%s)!\n",g_strFtpServerHost.c_str(),g_nFtpPort,g_strFtpUserName.c_str(),g_strFtpPassWord.c_str());
            fflush(g_pCSSendLog);
            #endif

            g_FtpCommunication.DoOpen((char*)g_strFtpServerHost.c_str(),g_nFtpPort,(char*)g_strFtpUserName.c_str(),(char*)g_strFtpPassWord.c_str());
        }
    }
}

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCTravelServer::mvConnOrLinkTest()
{
    if(!m_bAuthenticationOK)
    {
        mvAuthentication();
        //m_bAuthenticationOK = true;
    }
    else
    {
        //主动方式下ftp既发送实时记录又发送历史记录
        //被动方式下ftp只发送历史记录，socket用来发送实时记录
        if (g_nCommunicationMode == 1)//备用通讯方式
        {
            if (!m_bCenterLink)
            {
                if (mvConnectToCS())
                {
                    if (mvStartRecvThread())
                    {
                        LogNormal("备用通讯方式连接服务器成功!\n");
                        m_bCenterLink = true;
                        m_nCSLinkCount = 0;
                        m_uCSLinkTime = GetTimeStamp();
                        return;
                    }
                }
            }
            else
            {
                unsigned int uTimeStamp = GetTimeStamp();

                if ( (uTimeStamp >= m_uCSLinkTime+5) && (m_uCSLinkTime >0) )
                {
                    if (!mvSendLinkTest())
                    {
                        m_bCenterLink = false;
                        LogError("备用通讯方式发送心跳包失败\n");
                        return;
                    }
                    else
                    {
                        m_uCSLinkTime = uTimeStamp;
                    }
                }
            }
        }
    }
}

/*
* 函数介绍：连接到中心
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvConnectToCS()
{
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSConnLog, "\nmvConnectToCS, host=%s\tport=%d\n",
                            g_strControlServerHost.c_str(), g_nControlServerPort);
                    fflush(g_pCSConnLog);
#endif
    //connect to center server and set socket's option.
    if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
    {
        //printf("\n中心数据服务器连接参数异常:host=%s,port=%d\n", g_strControlServerHost.c_str(), g_nControlServerPort);
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSConnLog, "\n中心服务器连接参数异常! host=%s\tport=%d\n",
                            g_strControlServerHost.c_str(), g_nControlServerPort);
    fflush(g_pCSConnLog);
#endif
        return false;
    }

    if (!mvPrepareSocket(m_nCenterSocket))
    {
        //printf("\n准备连接中心数据服务器套接字失败!\n");
#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSConnLog, "\n准备连接中心服务器套接字失败!\n");
        fflush(g_pCSConnLog);
#endif
        return false;
    }

    if (!mvWaitConnect(m_nCenterSocket, g_strControlServerHost, g_nControlServerPort,2))
    {
        //printf("\n尝试连接中心数据服务器失败!\n");
#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSConnLog, "\n尝试连接中心服务器失败!\n");
        fflush(g_pCSConnLog);
#endif
        return false;
    }

    return true;
}

/*
* 函数介绍：接收消息线程入口
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvStartRecvThread()
{
    pthread_attr_t   attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    mvJoinThread(m_uRecvCSMsgThreadId);
    if (pthread_create(&m_uRecvCSMsgThreadId, &attr, RecvTravelMsgThread, NULL) != 0)
    {
        pthread_attr_destroy(&attr);
        return false;
    }

    pthread_attr_destroy(&attr);
    return true;
}


//接收中心端消息(消息需要立即处理)
bool mvCTravelServer::mvRecvCenterServerMsg(int nSocket)
{
    string strMsg("");
    //receive msg and push it into the msg queue.
    if (mvRecvMsg(nSocket, strMsg))
    {
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSRecvLog, "\n%x-success接收消息成功，套接字=%d，消息大小=%d(Bytes)，消息内容为: %s\n",
            (BYTE) (*(strMsg.c_str())),nSocket, (int)strMsg.size(),  strMsg.c_str());
    fflush(g_pCSRecvLog);
#endif
        return mvOnDealOneMsg(nSocket,strMsg);
    }
    else
    {
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSRecvLog, "\n%x-error接收消息失败，套接字=%d，消息大小=%d(Bytes)，消息内容为: %s\n",
            (BYTE) (*(strMsg.c_str())),nSocket, (int)strMsg.size(),  strMsg.c_str());
    fflush(g_pCSRecvLog);
#endif
        return false;
    }
}


/*
* 函数介绍：处理一条消息
* 输入参数：pCode-要处理的消息类型；strMsg-要处理的消息
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnDealOneMsg(int nSocket,const string &strMsg)
{
    string strNewMsg = strMsg;

    BYTE uCmdID = *((char*)strNewMsg.c_str());
    //去除包头
    strNewMsg.erase(0,1);

    //心跳信号
    if (uCmdID == 0xA0)
    {
        //
        printf("TRA_LINK_TEST=%s\n",strNewMsg.c_str());
        m_nCSLinkCount = 0;
        return true;
    }
    //实时过车文本数据回复
    else if (uCmdID == 0xA2)
    {
        //
        m_nCSLinkCount = 0;
        printf("mvOnRealTimeDataRep=%s\n",strNewMsg.c_str());
        return mvOnRealTimeDataRep(strNewMsg);
    }
    //点播业务响应
    else if(uCmdID == 0xA3)
    {
        printf("nSocket=%d,mvOnOrderOpration=%s\n",nSocket,strNewMsg.c_str());
        //解析中心到设备的点播业务请求
        return mvOnOrderOpration(nSocket,strNewMsg);
    }
    //询问
    else if(uCmdID == 0xA4)
    {
        //
        printf("mvOnDeviceStatus=%s\n",strNewMsg.c_str());
        return mvOnDeviceStatus(nSocket,strNewMsg);
    }
    //历史视频点播
    else if(uCmdID == 0xA5)
    {
        //
        printf("mvOnHistoryVideo=%s\n",strNewMsg.c_str());
        return mvOnHistoryVideo(nSocket,strNewMsg);
    }
    //实时视频点播
    else if (uCmdID == 0xA6)
    {
        printf("mvOnRealTimeVideo=%s\n",strNewMsg.c_str());
        //解析strMsg包内容
        return mvOnRealTimeVideo(nSocket,strNewMsg);
    }
    //业务控制参数协议
    else if (uCmdID == 0xA7)
    {
        printf("mvOnOprationControl=%s\n",strNewMsg.c_str());
        //解析strMsg包内容
        return mvOnOprationControl(nSocket,strNewMsg);
    }
    //设备认证规范回复
    else if(uCmdID == 0xA8)
    {
        //
        return mvOnAuthenticationRep(strNewMsg);
    }
}

/*
* 函数介绍：接收消息
* 输入参数：nSocket-要接收消息的套接字；strMsg-将接收到的消息内容
* 输出参数：strMsg-接收到的消息内容
* 返回值 ：成功返回true，否则false
*/
bool mvCTravelServer::mvRecvMsg(int nSocket, string& strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }
    if (nSocket <= 0)
    {
        return false;
    }

    UINT32 uDataLenth;

    char chBuffer[SRIP_MAX_BUFFER];

    if (recv(nSocket, (void*)&uDataLenth,sizeof(uDataLenth), MSG_NOSIGNAL) < 0)
    {
        return false;
    }

    int nLeft = (uDataLenth)+3; //数据包体长度
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

/*
* 函数介绍：发送心跳测试(A0)
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvSendLinkTest()
{
    printf("=====g_nCommunicationMode=%d====mvCTravelServer::mvSendLinkTest\n",g_nCommunicationMode);
    std::string strMsg("1"); //字符1
    return mvRebMsgAndSend(m_nCenterSocket, TRA_LINK_TEST, strMsg);
}

/*
* 函数介绍：发送认证请求(A8)
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
void mvCTravelServer::mvAuthentication()
{
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSConnLog, "\nmvAuthentication\n");
    fflush(g_pCSConnLog);
#endif
    if(mvConnectToCS())
    {
        std::string strMsg("1"); //字符1
        strMsg.append("00");
        strMsg.append("C");//系统编码
        strMsg.append("C02");//厂商编码
        char szBuff[15] = {0};
        sprintf(szBuff, "%15s",g_ServerHost.c_str());
        strMsg.append(szBuff);//IP地址

        if(mvRebMsgAndSend(m_nCenterSocket, TRA_DEVICE_SUPPLY, strMsg))
        {
            //接收认证回复消息
            mvRecvCenterServerMsg(m_nCenterSocket);

            mvCloseSocket(m_nCenterSocket);
        }
    }
}

/*
* 函数介绍：响应认证请求(A8)
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnAuthenticationRep(const string &strMsg)
{
    printf("mvOnAuthenticationRep=%s\n",strMsg.c_str());

    char chRes = *((char*)strMsg.c_str());
    char chCode = *((char*)strMsg.c_str()+3);
    string strMCode(""),strIp("");
    strMCode.append(strMsg.c_str() + 4, 3);
    strIp.append(strMsg.c_str() + 7, 15);
    char chAuth = *((char*)strMsg.c_str()+22);

    if((chRes == '0')&&
       (chCode == 'C')&&
       (chAuth == '0'))
    {
        m_bAuthenticationOK = true;
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSConnLog, "\n认证成功\n");
    fflush(g_pCSConnLog);
#endif
      LogNormal("\n认证成功\n");
    }
    else
    {
        m_bAuthenticationOK = false;
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSConnLog, "\n认证失败\n");
    fflush(g_pCSConnLog);
#endif
        LogNormal("\n认证失败\n");
    }

    CRoadXmlData XmlData;
    XmlData.GetDeviceInfo(strMsg.c_str()+23);

    return m_bAuthenticationOK;
}

/*
* 函数介绍：实时过车文本数据回复(A2)
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnRealTimeDataRep(const string &strMsg)
{
    //需要判断是否需要重新传递
    //printf("mvOnRealTimeDataRep=%s\n",strMsg.c_str());

    return true;
}

/*
* 函数介绍：点播业务响应(A3)
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnOrderOpration(int nSocket,const string &strMsg)
{
    //
    char chRes = *((char*)strMsg.c_str());
    if('1' == chRes)
    {
        string strNewMsg("");
        strNewMsg.append("0");
        strNewMsg.append(strMsg.c_str()+1,49);


        string strTime;
        strTime.append(strMsg.c_str()+17,17);

   //     printf("mvOnOrderOprationRep=%s\n",strTime.c_str());

        string strRecord;
        if(mvOrdOneRecord(strTime,strRecord))
        {
                strNewMsg.append("2");//找到
                //
                string strXmlData("");
                CRoadXmlData XmlData;
                string strPath = XmlData.AddCarNumberData(strRecord,strXmlData,3);//需要带图片但不需要存文件
                strNewMsg.append(strXmlData.c_str());
                //remove(strPath.c_str());
        }
        else //未找到数据
        {
                strNewMsg.append("1");//未找到
        }

        //通过tcp发送
        if(!mvRebMsgAndSend(nSocket, TRA_ORDER_OPRATION, strNewMsg))
        {
            return false;
        }
        //接收回复数据包
        //return mvRecvCenterServerMsg(nSocket);
        sleep(1);
        return true;
    }
    else if('0' == chRes)
    {
        //为中心响应，需要判断是否需要重新传递
        printf("mvOnOrderOprationRep=%s\n",strMsg.c_str());
        return true;
    }
    return false;
}

/*
* 函数介绍：处理历史视频消息(A5)
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnHistoryVideo(int nSocket,const string &strMsg)
{
    string strBeginTime,strEndTime,strPathList;

    strBeginTime.append(strMsg.c_str()+12,17);
    strEndTime.append(strMsg.c_str()+29,17);

    m_strVideoBeginTime = strBeginTime;
    m_strVideoEndTime = strEndTime;

    unsigned short nCount = g_skpDB.GetVideoFileList(strBeginTime,strEndTime,strPathList);

    string strNewMsg("");
    strNewMsg.append("0");
    strNewMsg.append(strMsg.c_str()+1,10);//车道编码
    strNewMsg.append("0");//状态
    printf("nCount = %d,strPathList=%s,strPathList.size()=%d\n",nCount,strPathList.c_str(),strPathList.size());
    //nCount = htons(nCount);
    printf("nCount = %x\n",nCount);
    strNewMsg.append((char*)&nCount,sizeof(unsigned short));//视频文件个数
    if(strPathList.size()>0)
    strNewMsg.append(strPathList.c_str(),strPathList.size());//视频文件列表
    printf("TRA_ORDER_VIDEOTIME = strNewMsg=%s,strNewMsg.size()=%d\n",strNewMsg.c_str(),strNewMsg.size());

    if(mvRebMsgAndSend(nSocket,TRA_ORDER_VIDEOTIME, strNewMsg))
    {
        if(nCount > 0)
        {
            //需要启动线程发送历史视频(FTP方式)
            pthread_attr_t   attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_t id;
            if (pthread_create(&id, &attr, HistoryVideoThread,NULL) != 0)
            {
                pthread_attr_destroy(&attr);
                return false;
            }

            pthread_attr_destroy(&attr);
        }
        return true;
    }
    else
    {
        return false;
    }
}

//事件录像发送处理
void mvCTravelServer::mvDealEventVideo()
{
    StrList strRemoteVideoPathList = g_skpDB.GetRemoteEventVideoPath(m_strEventVideoName);

    if(strRemoteVideoPathList.size()>0)
    {
        StrList::iterator it = strRemoteVideoPathList.begin();
        while(it != strRemoteVideoPathList.end())
        {
    #ifdef TravelCS_FILE_LOG
            fprintf(g_pCSSendLog, "\n=========发送事件录像=====>>>到旅行时间中心端!\n");
            fflush(g_pCSSendLog);
    #endif
            string strRemoteVideoPath = *it;

            string strMsg;
            if(m_bAuthenticationOK)
            g_TravelServer.SendDataByFtp(m_strEventVideoName,strRemoteVideoPath,strMsg,1);

            it++;

            sleep(1);
        }
    }
}

//历史视频发送处理
void mvCTravelServer::mvDealHistoryVideo()
{
    string strPathList;
    unsigned short nCount = g_skpDB.GetVideoFileList(m_strVideoBeginTime,m_strVideoEndTime,strPathList);
    for(int i =0;i<nCount;i++)
    {
        string strPath,strNewPath;
        strPath.append(strPathList.c_str()+i*42,41);

        strNewPath.append(g_strVideo.c_str());
        strNewPath.append("/");
        strNewPath.append(strPath.c_str()+20,8);
        strNewPath.append("/");
        strNewPath.append(strPath.c_str()+28,2);
        strNewPath.append("/");
        strNewPath.append(strPath.c_str()+30,2);
        strNewPath.append(".mp4");

        if(access(strNewPath.c_str(),F_OK)==0)//文件存在
        {
            string strMsg;
            SendDataByFtp(strNewPath,strPath,strMsg,1);
        }
    }
}

/*
* 函数介绍：处理业务控制消息(A7)
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnOprationControl(int nSocket,const string &strMsg)
{
    //需要解析xml判断是否进行备用方式传输

    CRoadXmlData XmlData;
    //解析业务控制消息
    XmlData.GetControlInfo(strMsg.c_str() + 3, m_travelControlInfo);

    int nCommunicationMode = m_travelControlInfo.m_bIsUploadRealTime;

    if(nCommunicationMode != g_nCommunicationMode)
    {
        g_nCommunicationMode = nCommunicationMode;
        CXmlParaUtil xml;
        xml.UpdateSystemSetting("OtherSetting","CommunicationMode");
    }

    if(g_nCommunicationMode == 1)
    {
        LogNormal("启用备用通讯方式!\n");
    }
    else
    {
        LogNormal("停止备用通讯方式!\n");
    }

    string strNewMsg("");
    strNewMsg.append("0");
    strNewMsg.append("21");
    strNewMsg.append("0");//是否成功

    return mvRebMsgAndSend(nSocket, TRA_OPRATION_CONTROL, strNewMsg);
}


/*
* 函数介绍：处理实时视频点播消息(A6)
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnRealTimeVideo(int nSocket,const string &strMsg)
{
    //需要通知进行实时视频传输
    char chSendRTSP = *((char*)strMsg.c_str()+11);
    /*if(chSendRTSP == '1')
    {
        g_nSendRTSP = 1;
    }
    else
    {
        g_nSendRTSP = 0;
    }*/
    LogNormal("订阅实时视频=%c\n",chSendRTSP);

    string strNewMsg("");
    strNewMsg.append("0");
    strNewMsg.append(strMsg.c_str()+1,11);
    strNewMsg.append("0");//状态

    //启动通过rtsp协议，视频传输
    return mvRebMsgAndSend(nSocket, TRA_ORDER_REALTIME_VIDEO, strNewMsg);
}


/*
* 函数介绍：检查设备状态(A4)
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvOnDeviceStatus(int nSocket,const string &strMsg)
{
    string strNewMsg("");
    strNewMsg.append("0");
    strNewMsg.append(strMsg.c_str()+1,8);//8位设备编码

    if(mvRebMsgAndSend(nSocket,TRA_ASK, strNewMsg))
    {
        //自动检测设备并通过ftp上传自检日志
        string strLogMsg;
        CRoadXmlData XmlData;
        string strPath = XmlData.AddLogData(strLogMsg);
        return SendDataByFtp(strPath,strPath,strLogMsg);
    }
    return false;
}


/*
* 函数介绍：发送设备类日志
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvSendLogData()
{
    if(!m_bAuthenticationOK)
    {
        return false;
    }
    //自动检测设备并通过ftp上传自检日志
    string strLogMsg;
    CRoadXmlData XmlData;
    string strPath = XmlData.AddLogData(strLogMsg);
    return SendDataByFtp(strPath,strPath,strLogMsg);
}


/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvSendRecordToCS(const string &strMsg)
{
	if(g_nServerType == 3)
	{
		if(!m_bAuthenticationOK)
		{
			return false;
		}
	}

    MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();

    if (MIMAX_EVENT_REP == sHeader->uCmdID)
    {
		if(g_nServerType != 3)
		{
			return true;
		}

		if(strMsg.size() <= sizeof(RECORD_EVENT)+sizeof(MIMAX_HEADER))
		{
			return true;
		}
#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSSendLog, "\n=========发送事件数据=====>>>到旅行时间中心端!\n");
        fflush(g_pCSSendLog);
#endif
        string strNewMsg("");
        CRoadXmlData XmlData;

        string strPath = XmlData.AddEventData(strMsg,strNewMsg);
        //通过ftp发送
        return SendDataByFtp(strPath,strPath,strNewMsg);
    }
    else if (MIMAX_PLATE_REP == sHeader->uCmdID)
    {
		if(strMsg.size() <= sizeof(RECORD_PLATE)+sizeof(MIMAX_HEADER))
		{
			return true;
		}

        string strNewMsg("");
        CRoadXmlData XmlData;

        if(g_nCommunicationMode == 0)
        {
#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSSendLog, "\n=========发送抓拍监测数据=====>>>到旅行时间中心端!\n");
        fflush(g_pCSSendLog);
#endif
            printf("=================AddCarNumberData\n");
            string strPath = XmlData.AddCarNumberData(strMsg,strNewMsg,1);//需要带图片
            //通过ftp发送
            return SendDataByFtp(strPath,strPath,strNewMsg);
        }
        else if(g_nCommunicationMode == 1)
        {
            //////如果是无牌车或大货禁行
            RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
            if(pPlate->uViolationType == DETECT_RESULT_NOCARNUM ||//无牌车
               pPlate->uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME)//大货禁行
            {
                string strPath = XmlData.AddCarNumberData(strMsg,strNewMsg,1);//需要带图片
                //通过ftp发送
                return SendDataByFtp(strPath,strPath,strNewMsg);
            }
            else if(pPlate->uViolationType == 0)
            {
                if(m_bCenterLink)
                {
    #ifdef TravelCS_FILE_LOG
            fprintf(g_pCSSendLog, "\n=========发送实时过车文本数据=====>>>到旅行时间中心端!\n");
            fflush(g_pCSSendLog);
    #endif
                    //
                    string strNewMsg("");
                    strNewMsg.append("1");
                    strNewMsg.append("06");

                    string strXmlData("");
                    string strPath = XmlData.AddCarNumberData(strMsg,strXmlData,2);//不需要带图片

                    //
                    char buf[47]={0};
                    memset(buf,0x20,47);//后补空格
                    memcpy(buf,strPath.c_str(),strPath.size());
                    strNewMsg.append(buf,47);//唯一标识（即存储的下端文件名）
                    strNewMsg = strNewMsg + strXmlData;

                    //通过tcp发送
                    if(!mvRebMsgAndSend(m_nCenterSocket, TRA_PLATE_INFO, strNewMsg))
                    {
                        m_bCenterLink = false;
                        return false;
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
    else if (MIMAX_STATISTIC_REP == sHeader->uCmdID)
    {
		if(g_nServerType != 3)
		{
			return true;
		}
#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSSendLog, "\n=========发送流量数据=====>>>到旅行时间中心端!\n");
        fflush(g_pCSSendLog);
#endif
        string strNewMsg("");
        CRoadXmlData XmlData;
        string strPath = XmlData.AddFlowData(strMsg,strNewMsg);
        //通过ftp发送
        return SendDataByFtp(strPath,strPath,strNewMsg);
    }
    else if (PLATE_LOG_REP == sHeader->uCmdID)
    {
		if(g_nServerType != 3)
		{
			return true;
		}
#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSSendLog, "\n=========发送运行日志数据=====>>>到旅行时间中心端!\n");
        fflush(g_pCSSendLog);
#endif
        string strNewMsg("");
        CRoadXmlData XmlData;
        string strPath = XmlData.AddRunLogData(strMsg,strNewMsg);
        //通过ftp发送
        return SendDataByFtp(strPath,strPath,strNewMsg);
    }

    return true;
    //return mvRebMsgAndSend(m_nCenterSocket,uCmdID, strNewMsg);
}

/*
* 函数介绍：重组消息并发送
* 输入参数：pCode-要发送的消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvRebMsgAndSend(int& nSocket,BYTE uCode, const string &strMsg)
{
    //rebuild the message according to the msg-code and then send it to the center server.
    string strFullMsg("");
    UINT32 uDataLenth = (strMsg.size());//需要注意"大小端"

    strFullMsg.append((char*)&uDataLenth,sizeof(UINT32));
    strFullMsg.append((char*)&uCode,1);//包头

    if (!strMsg.empty())
    {
        strFullMsg += strMsg;//包体
    }
    strFullMsg += "\r\n";//包尾

    //printf("mvRebMsgAndSend=%s,uCode=%d,strMsg.size()=%d,strFullMsg.size()=%d\n",strMsg.c_str(),uCode,strMsg.size(),strFullMsg.size());
    if (!mvSendMsgToSocket(nSocket, strFullMsg))
    {
#ifdef TravelCS_FILE_LOG
        fprintf(g_pCSSendLog, "\n%x-error发送消息失败，套接字=%d，消息大小=%d(Bytes)，消息内容为：%x\n",
                uCode,nSocket, (int)strFullMsg.size(),uCode);
        fflush(g_pCSSendLog);
#endif
        mvCloseSocket(nSocket);
        LogError("发送消息失败，连接断开\n");
        return false;
    }
#ifdef TravelCS_FILE_LOG
    fprintf(g_pCSSendLog, "\n%x-success发送消息成功，套接字=%d，消息大小=%d(Bytes)，消息内容为: %x\n",
            uCode,nSocket, (int)strFullMsg.size(), uCode);
    fflush(g_pCSSendLog);
#endif
    return true;
}

//记录点播
bool mvCTravelServer::mvOrdOneRecord(string strTime,string& strRecord)
{
    string strMsg;
    string strTime1;
    strTime1.append(strTime.c_str(),4);//year
    strTime1 += "-";
    strTime1.append(strTime.c_str()+4,2);//month
    strTime1 += "-";
    strTime1.append(strTime.c_str()+6,2);//day
    strTime1 += " ";
    strTime1.append(strTime.c_str()+8,2);//hour
    strTime1 += ":";
    strTime1.append(strTime.c_str()+10,2);//minute
    strTime1 += ":";
    strTime1.append(strTime.c_str()+12,2);//second

    string strTime2;
    strTime2.append(strTime.c_str()+14,3);//misecond

    int nMiTime = atoi(strTime2.c_str());

    {
        char buf[255]={0};
	    sprintf(buf,"Select * from NUMBER_PLATE_INFO where TIME = '%s' and MITIME = %d",strTime1.c_str(),nMiTime);
        string strSql(buf);
        MysqlQuery q = g_skpDB.execQuery(strSql);

        printf("mvOrdOneRecord strSql.c_str()=%s\n",strSql.c_str());

        if (!q.eof())
        {
            MIMAX_HEADER sHeader;

            RECORD_PLATE plate;

            plate.uSeq = q.getUnIntFileds("ID");
            string strCarNum = q.getStringFileds("NUMBER");
            g_skpDB.UTF8ToGBK(strCarNum);
            memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

            plate.uColor = q.getIntFileds("COLOR");
            plate.uCredit = q.getIntFileds("CREDIT");
            plate.uRoadWayID = q.getIntFileds("ROAD");

            string strTime = q.getStringFileds("TIME");
            plate.uTime = MakeTime(strTime);
            plate.uMiTime = q.getIntFileds("MITIME");

            plate.uSmallPicSize = q.getIntFileds("SMALLPICSIZE");
            plate.uSmallPicWidth = q.getIntFileds("SMALLPICWIDTH");
            plate.uSmallPicHeight = q.getIntFileds("SMALLPICHEIGHT");

            plate.uPicSize = q.getIntFileds("PICSIZE");
            plate.uPicWidth = q.getIntFileds("PICWIDTH");
            plate.uPicHeight = q.getIntFileds("PICHEIGHT");

            plate.uPosLeft = q.getIntFileds("POSLEFT");
            plate.uPosTop = q.getIntFileds("POSTOP");
            plate.uPosRight = q.getIntFileds("POSRIGHT");
            plate.uPosBottom = q.getIntFileds("POSBOTTOM");

            string strPicPath = q.getStringFileds("PICPATH");

            //车身颜色，车辆类型，速度，方向,地点等
            plate.uCarColor1 = q.getIntFileds("CARCOLOR");
            plate.uType = q.getIntFileds("TYPE");
            plate.uSpeed = q.getIntFileds("SPEED");
            plate.uWeight1 = q.getIntFileds("CARCOLORWEIGHT");
            plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
            plate.uWeight2 = q.getIntFileds("CARCOLORWEIGHTSECOND");

            //plate.uViolationType = q.getIntFileds(31);//事件类型

            bool bPlateInfo = true;
            if(plate.uType >= OTHER_TYPE)
            {
                bPlateInfo = false;
            }

            //if (bPlateInfo) //有牌车
            {
                if(plate.chText[0]=='*')//
                {
                    memcpy(plate.chText,"00000000",8);
                }
                memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

                sHeader.uCmdID = MIMAX_PLATE_REP;
                strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
                strMsg.append((char*)&plate, sizeof(RECORD_PLATE));

                strRecord = strMsg+GetImageByPath(strPicPath);
            }
        }
        q.finalize();
	}
	return (!strRecord.empty());
}

/*
* 函数介绍：获取一条历史记录
* 输入参数：strMsg-要获取的历史记录存储变量
* 输出参数：strMsg-获取的历史记录
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTravelServer::mvDealHistoryRecord()
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

				if(g_nSendImage == 1)
				{
					String strPicPath(sPlate->chPicPath);
					string strPic = GetImageByPath(strPicPath);
					MIMAX_HEADER* pHeader = (MIMAX_HEADER*)strPlate.c_str();
					pHeader->uCmdLen += strPic.size();
					strPlate.append((char*)strPic.c_str(),strPic.size());
				}
						
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

    //事件记录
	if(g_nServerType == 3)
	{
					StrList strListEvent;
					if(g_skpDB.GetEventHistoryRecord(strListEvent))//违章记录
					{
						StrList::iterator it_b = strListEvent.begin();
						while(it_b != strListEvent.end())
						{
							string strEvent("");
							strEvent = *it_b;
							
							bool bSendStatus = false;
							bSendStatus = mvSendRecordToCS(strEvent);

							if(bSendStatus)
							{
								UINT32 uSeq = *(UINT32*)(strEvent.c_str()+sizeof(MIMAX_HEADER));
								g_skpDB.UpdateRecordStatus(MIMAX_EVENT_REP, uSeq);
								sleep(5);
							}
							it_b++;
						}
					}
	}
}

//通过ftp发送数据(socket方式)
bool mvCTravelServer::SendDataByFtp(string strLocalPath,string strRemotePath,string& strMsg,int nMode)
{
	if(g_nServerType == 3)
	{
        if(!m_bAuthenticationOK)
        {
            return false;
        }
	}

        if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
        {
            return false;
        }

        bool bRet = false;

        if(nMode == 0)
        bRet = g_FtpCommunication.DoPut(NULL,(char *)strRemotePath.c_str(),strMsg);
        else if(nMode == 1)
        bRet = g_FtpCommunication.DoPut((char *)strLocalPath.c_str(),(char *)strRemotePath.c_str(),strMsg);

        if(bRet)
        {
            #ifdef TravelCS_FILE_LOG
            fprintf(g_pCSSendLog, "\n=========通过ftp发送数据=%s====>>>到旅行时间中心端!\n",strRemotePath.c_str());
            fflush(g_pCSSendLog);
            #endif

        }

    return bRet;
}

//获取ftp登录信息(匿名登录)
bool mvCTravelServer::GetFtpLoginInfo()
{
    CFtpCommunication tmpFtpCommunication;
    if(tmpFtpCommunication.DoOpen((char*)g_strFtpServerHost.c_str(),g_nFtpPort,"anonymous","anonymous"))
    //if(tmpFtpCommunication.DoOpen((char*)g_strFtpServerHost.c_str(),g_nFtpPort,(char*)g_strFtpUserName.c_str(),(char*)g_strFtpPassWord.c_str()))
    {
        string strRetCode;
        char chCmd[256] = {0};
        sprintf(chCmd,"LOGON %s",g_strFtpUserName.c_str());
        int iRetCode = tmpFtpCommunication.DoSite(chCmd,strRetCode);
        //LogNormal("LOGON=%s",strRetCode.c_str());
        //strRetCode = "200 lxsjlxsj lxsj";
        //iRetCode = 200;
        if(iRetCode == 200)//获取密码成功
        {
            char szCode[8] = {0};
            char szUser[32] = {0};
            char szPass[32] = {0};

            sscanf(strRetCode.c_str(), "%s %s %s", szCode, szUser, szPass);
            printf("szCode=%s, szUser=%s, szPass=%s\n",szCode, szUser, szPass);

            string strPasswd(szPass);

            if(g_strFtpPassWord != strPasswd)
            {
                LogError("获取ftp密码成功=%s!\n",strPasswd.c_str());
                g_strFtpPassWord = strPasswd;

                CXmlParaUtil xml;
                xml.UpdateSystemSetting("OtherSetting", "FtpPassWord");
            }
        }
        else
        {
            LogError("获取ftp密码失败=%s!\n",strRetCode.c_str());
        }
        tmpFtpCommunication.DoClose();

        return true;
    }
    else
    {
        return false;
    }
}

//时钟同步
bool mvCTravelServer::SynClock()
{
    CFtpCommunication tmpFtpCommunication;
    if(tmpFtpCommunication.DoOpen((char*)g_strFtpServerHost.c_str(),g_nFtpPort,"anonymous","anonymous"))
    {
        //获取校时前时间
        struct timeval tv;
        gettimeofday(&tv,NULL);
        struct tm *oldTime,timenow;
        oldTime = &timenow;
        localtime_r( &tv.tv_sec,oldTime);
        char szBuff[1024] = {0};
        sprintf(szBuff, "%4d-%02d-%02dT%02d:%02d:%02d.%03d",oldTime->tm_year+1900, oldTime->tm_mon+1, oldTime->tm_mday, oldTime->tm_hour, oldTime->tm_min, oldTime->tm_sec,(int)(tv.tv_usec/1000.0));
        string stroldTime(szBuff);

        string strRetCode;
        char chCmd[256] = {0};
        sprintf(chCmd,"TIME %s %s",g_strFtpUserName.c_str(),stroldTime.c_str());
        int iRetCode = tmpFtpCommunication.DoSite(chCmd,strRetCode);

        if(iRetCode == 200)//时钟同步成功
        {
            char szCode[8] = {0};
            char szDate[11] = {0};
            char szTime[13] = {0};
            sscanf(strRetCode.c_str(), "%s %10sT%12s", szCode, szDate,szTime);

            string strTime = szDate;
            strTime += " ";
            strTime.append(szTime, 8);
            if(strTime.size() > 18)
            {
				timeval timer;
				timer.tv_sec = MakeTime(strTime);
				timer.tv_usec = 0;
				if(settimeofday(&timer, NULL) == 0)
				{
					LogNormal("ftp校时成功=%s!size=%d\n",strTime.c_str(),strTime.size());
					system("hwclock --systohc");
				}
            }
        }
        else
        {
            LogError("ftp校时失败=%s!\n",strRetCode.c_str());
        }
        tmpFtpCommunication.DoClose();
        return true;
    }
    else
    {
        return false;
    }
}
