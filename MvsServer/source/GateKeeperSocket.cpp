// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
*   文件：GateKeeperSocket.cpp
*   功能：远程监控看门设备通讯类
*   作者：yuwenxian
*   时间：2010-10-13
**/

#include "Common.h"
#include "CommonHeader.h"
#include "GateKeeperSocket.h"
#include "MvsCommunication.h"

//通讯服务
CGateKeeperSocket g_GateKeeperServer;

//监控线程
void* ThreadGateKeeperAccept(void* pArg)
{
	int nSocket =*(int*)pArg;

	printf("==========ThreadGateKeeperAccept(void* pArg)===nSocket=%d=======\n", nSocket);

	//客户端连接
	struct sockaddr_in clientaddr;
	//长度
	socklen_t sin_size = sizeof(struct   sockaddr_in);
	int nClient;
	while(!g_bEndThread)
	{
	    printf("==ThreadGateKeeperAccept===Receiving....==nSocket=%d===\n", nSocket);

		//接受连接
		if((nClient = accept(nSocket,(struct sockaddr*)&clientaddr,&sin_size)) ==  -1)
		{
			//断开连接
			if(g_bEndThread)
			{
	//		    LogNormal("11 accept exit\r\n");
                printf("====Disconnect....======ThreadGateKeeperAccept(void* pArg)=");
				return pArg;
			}
			//LogNormal("accept nClient = %d\r\n",nClient);

            //printf("====Connect....======ThreadGateKeeperAccept(void* pArg)=");

			//自动重启
			continue;
		}

		//输出用户连接
		LogNormal("环境监控连接[IP:%s][nClient = %d,端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));

		//g_GateKeeperServer.mvSetCenterSocket(nClient); //设置当前连接套接字

        g_GateKeeperServer.mvRecvCenterServerMsg(nClient); //启动接收环境监控信息线程

		g_GateKeeperServer.mvGetGateKeeperInfo();

		//1秒钟接收一次消息
		sleep(1);
	}

	//LogNormal("22 accept exit\r\n");
	return pArg;
}

//环境监控数据接收线程
void* ThreadGateKeeperRecv(void* pArg)
{
	int nSocket =*(int*)pArg;

	//LogNormal("=ThreadGateKeeperRecv==nSocket=%d=\n", nSocket);

	g_GateKeeperServer.mvSetCenterSocket(nSocket); //设置当前连接套接字

	//LogNormal("==after set the mvSetCenterSocket===\n");

	int nBytes = 0;

	char chBuffer[SRIP_MAX_BUFFER];
	while(!g_bEndThread)
	{
	    printf("========== ThreadGateKeeperRecv....\n");

	    GATEKEEPER_HEADER mHeader;
		std::string response;

		//接收头，一次性接收
		if((nBytes = recv(nSocket,(void*)&mHeader,sizeof(mHeader),MSG_NOSIGNAL)) < 0)
		{
			//断开连接
			if(g_bEndThread)
			{
				return pArg;
			}

			LogError("接收协议头出错，连接断开! socket = %d\r\n", nSocket);

			return pArg;
		}

		response.append((char*)&mHeader,sizeof(mHeader));

		//接收剩下的数据
		int nLeft = (mHeader.nDataLenthHigh * 256) + mHeader.nDataLenthLow;

		while(nLeft >  0)
		{
		    printf("==============ThreadGateKeeperRecv===nLeft=%d========", nLeft);
			//接收后面的数据
			if((nBytes = recv(nSocket,chBuffer,sizeof(chBuffer)<nLeft?sizeof(chBuffer):nLeft,MSG_NOSIGNAL)) < 0)
			{
				//断开连接
				if(g_bEndThread)
				{
					return pArg;
				}
				LogError("接收后续消息出错，连接断开!\r\n");

				return pArg;
			}
			//保存数据
			response.append(chBuffer,nBytes);
			nLeft -= nBytes;
		}

        if(response.size() > 0)
        {
            //printf("=============Receve2222============\n");
             g_GateKeeperServer.mvOnDealOneMsg(response);
        }


        //100毫秒
        //usleep(1000*100);
        sleep(1);
    }

	return pArg;
}

//环境监控数据轮询线程
void* ThreadGateKeeperAsk(void* pArg)
{
	int nSocket =*(int*)pArg;

	//LogNormal("==ThreadGateKeeperAsk=nSocket=%d==\n", nSocket);

	char chBuffer[SRIP_MAX_BUFFER];
	while(!g_bEndThread)
	{
	    //UINT32 time = GetTimeStamp();//系统当前时间
	    //std::string strTIME = GetTime(time, 2);
	    //LogNormal("###====[%d]==[%s]== ThreadGateKeeperAsk....===nSocket=%d====\n", time, strTIME.c_str(), nSocket);

	    //m_nCenterSocket = nSocket; //设置当前连接套接字

        if(g_nHasExpoMonitor) //获取环境监控设备信息
        {
            //bool bGetGateKeeperInfo = g_GateKeeperServer.mvGetGateKeeperInfo();
           g_GateKeeperServer.mvGetGateKeeperInfo();

            /*if(!bGetGateKeeperInfo)
            {
                LogError("==ThreadGateKeeperAsk===不能查找到环境监控设备信息!!!===\n");
            }
            */
        }

	    //每1分钟轮询一次
	    sleep(60);
	}

	return pArg;
}

CGateKeeperSocket::CGateKeeperSocket()
{
    m_nCenterSocket = 0;
    m_nAcceptSocket = 0;
    //m_nPort = g_ExpoMonitorInfo.uExpoMonitorPort; //4001
    m_nPort = 4001;
}


CGateKeeperSocket::~CGateKeeperSocket()
{
    mvCloseSocket(m_nAcceptSocket);
}

//启动侦听服务
bool CGateKeeperSocket::Init()
{
    printf("=$$$$$$=======CGateKeeperSocket::Init()===\n");

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

	//LogNormal("=m_nAcceptSocket=%d==m_nPort=%d==\n", m_nAcceptSocket, m_nPort);

	//开始监听
	if (mvStartListen(m_nAcceptSocket) == false)
	{
		printf("监听连接失败，服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	m_nPort = g_ExpoMonitorInfo.uExpoMonitorPort; //4001

	printf("=========CGateKeeperSocket::Init()=============m_nAcceptSocket=%d====m_nPort=%d=========\n", m_nAcceptSocket, m_nPort);

	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动环境监控线程
	int nret=pthread_create(&id,&attr,ThreadGateKeeperAccept,(void*)&m_nAcceptSocket);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建环境监控线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}
	pthread_attr_destroy(&attr);

    //LogNormal("===before==g_GateKeeperServer.mvStartGateKeeperAsk=\n");
	g_GateKeeperServer.mvStartGateKeeperAsk(m_nAcceptSocket); //启动环境监控信息轮询线程

	return true;
}


//释放
bool CGateKeeperSocket::UnInit()
{
    //需要关闭所有连接
    mvCloseSocket(m_nAcceptSocket);
    //关闭连接
    mvCloseSocket(m_nCenterSocket);
	return true;
}

//接收中心端消息(消息需要立即处理)
bool CGateKeeperSocket::mvRecvCenterServerMsg(int nSocket)
{
    printf("==CGateKeeperSocket::mvRecvCenterServerMsg=====nSocket=%d=m_nCenterSocket=%d=\n", nSocket, m_nCenterSocket);

    //g_GateKeeperServer.mvSetCenterSocket(nClient); //设置当前连接套接字

    //mvCloseSocket(m_nCenterSocket);
    //m_nCenterSocket = nSocket;

    //启动监控数据接收线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//启动监控线程
	int nret=pthread_create(&id,&attr,ThreadGateKeeperRecv,(void*)&nSocket);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建启动监控数据接收线程失败,连接断开!\r\n");
		return false;
	}
    pthread_attr_destroy(&attr);

    return true;
}

//启动环境监控数据轮询线程
bool CGateKeeperSocket::mvStartGateKeeperAsk(int nSocket)
{
   // UINT32 time = GetTimeStamp();//系统当前时间
    //std::string strTIME = GetTime(time, 2);

    //LogNormal("=CGateKeeperSocket::mvRecvCenterServerMsg==nSocket=%d==\n", nSocket);

    //mvCloseSocket(m_nCenterSocket);
    //m_nCenterSocket = nSocket;

    //启动环境监控数据轮询线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//启动监控线程
	int nret = pthread_create(&id,&attr,ThreadGateKeeperAsk,(void*)&nSocket);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建启动环境监控数据轮询线程失败,连接断开!\r\n");
		return false;
	}
    pthread_attr_destroy(&attr);

    return true;
}

//中心端接收消息
bool CGateKeeperSocket::mvRecvMsg(int nSocket, string& strMsg)
{
    mvCloseSocket(m_nCenterSocket);
    m_nCenterSocket = nSocket;

	if (!strMsg.empty())
    {
        strMsg.clear();
    }

    if (m_nCenterSocket <= 0)
    {
        return false;
    }

	GATEKEEPER_HEADER mHeader;

    char chBuffer[SRIP_MAX_BUFFER];

    if (recv(m_nCenterSocket, (void*)&mHeader,sizeof(mHeader), MSG_NOSIGNAL) < 0)
    {
        printf("========CGateKeeperSocket::mvRecvMsg=====error 1111=======\n");

        return false;
    }

    strMsg.append((char*)&mHeader,sizeof(mHeader));

    printf("====3333===CGateKeeperSocket::mvRecvMsg======strMsg.size()=%d===\n", strMsg.size());

    int nLeft = mHeader.nDataLenthLow; //数据包体长度
    int nBytes = 0;

    while(nLeft >  0)
    {
        nBytes = recv(m_nCenterSocket, chBuffer, nLeft, MSG_NOSIGNAL);
        if ( nBytes < 0)
        {
            printf("========CGateKeeperSocket::mvRecvMsg=========error 2222===strMsg.size()=%d====\n", strMsg.size());
            return false;
        }
        //保存数据
        strMsg.append(chBuffer,nBytes);
        nLeft -= nBytes;
    }

    return (!strMsg.empty());
}


//处理命令
bool CGateKeeperSocket::mvOnDealOneMsg(const string &strMsg)
{
	if(strMsg.size() < sizeof(GATEKEEPER_HEADER))
	{
		return false;
	}
    //UINT32 time = GetTimeStamp();//系统当前时间
    //std::string strTIME = GetTime(time, 2);
    //printf("=======[%s]====CGateKeeperSocket::mvOnDealOneMsg===strMsg.size()=%d===\n", strTIME.c_str(), strMsg.size());

  /*  if(strMsg.size() > 0)
    {
        unsigned char chTemp = '0';
        for(int i = strMsg.size() - 1; i >= 0; i--)
        {
            chTemp = *(BYTE*)(strMsg.c_str() + i);
            printf("=Rc==[%d]: %x \n", i, chTemp);
        }
    }*/
/*
    //建立临时文件
    FILE* fp = fopen("./GATE1.log","ab+");
    if(fp!=NULL)
    {
        //fprintf(fp,"\n========GATE1=======\n");
        //fflush(fp);
        fwrite(strMsg.c_str(), strMsg.size(), 1, fp);

        fclose(fp);
    }
*/
    GATEKEEPER_HEADER* mHeader = (GATEKEEPER_HEADER*)strMsg.c_str();

    int nSpknRev = (mHeader->nSpknHigh * 256) + mHeader->nSpknLow; //接收到请求报号
    mvSetRpkn(nSpknRev); //设置下一次的响应报号
/*
    //设置请求数据报号
    int nRpknRev = (mHeader->nRpknHigh * 256) + mHeader->nRpknLow; //接收到响应报号
    mvSetSpkn(nRpknRev + 1); //设置下一次的请求报号
*/
    printf("===CGateKeeperSocket::mvOnDealOneMsg===m_nSpkn=%d,=m_nRpkn=%d====", m_nSpkn, m_nRpkn);

    if( (mHeader->chCmd & 0xf0 ) == 0x60) //用户命令
    {
        printf("================用户命令==\n");

        BYTE chCMD1 = *( (BYTE*)(strMsg.c_str() + sizeof(mHeader)) );
        BYTE chCMD2 = *( (BYTE*)(strMsg.c_str() + sizeof(mHeader) + 1) );
        switch (chCMD1)
        {
            case 'T': //TD 控制命令，一般由主设备发出
            {
                break;
            }
            case 'S': //ST 设定控制参数，一般由主设备发出
            {
                break;
            }
            case 'R': //RD 读状态
            {
				LogNormal("==主动获取监控设备状态==\n");
                //BYTE chCmd3 = *( (BYTE*)(strMsg.c_str() + sizeof(mHeader) + 2) );

                //BYTE nCodeType;
                /*
                BYTE nACodeId[MAX_GATE_ARRAY_SIZE];
                BYTE nAAValueType[MAX_GATE_ARRAY_SIZE * MAX_GATE_ARRAY_SIZE];
                BYTE nAAAValue[MAX_GATE_ARRAY_SIZE * MAX_GATE_ARRAY_SIZE * MAX_GATE_ARRAY_SIZE];

                BYTE nACodeId[MAX_GATE_ARRAY_SIZE];
                BYTE nAAValueType[MAX_GATE_ARRAY_SIZE][MAX_GATE_ARRAY_SIZE];
                BYTE nAAAValue[MAX_GATE_ARRAY_SIZE][MAX_GATE_ARRAY_SIZE][MAX_GATE_ARRAY_SIZE];
                */

                //mvParseCmdString(strMsg, nCodeType, nACodeId, nAAValueType, nAAAValue);
                mvParseCmdString(strMsg, g_ExpoMonitorInfo, false);
                //printf("===[%s]==RD 读状态=====CGateKeeperSocket::mvOnDealOneMsg=====[%c %c], #%c, %d==\n", strTIME.c_str(), nCodeType, nCodeId, nValueType, nValue);

                break;
            }
            case 'W': //WO 主动上传数据
            {
                LogNormal("==监控设备主动报警==\n");

                bool bGetWO = true; //标记是否接收上传数据成功
                /*
                BYTE chCmd3 = *( (BYTE*)(strMsg.c_str() + sizeof(mHeader) + 2) );

                BYTE nCodeType;
                BYTE nCodeId;
                BYTE nValueType;
                BYTE nValue;

                //mvParseCmdString(strMsg, nCodeType, nCodeId, nValueType, nValue);
                //printf("=========CGateKeeperSocket::mvOnDealOneMsg=====[%c %d], #%d, %d==\n", nCodeType, nCodeId, nValueType, nValue);

                switch(chCmd3)
                {
                    case 'R': //无源触点
                    {
                        break;
                    }
                    case 'E': //设备
                    {
                        break;
                    }
                    case 'W': //警笛
                    {
                        break;
                    }
                    case 'A': //防拆警报--开关门状态
                    {
                        break;
                    }
                    case 'T': //温度
                    {
                        break;
                    }
                    case 'H': //湿度
                    {
                        break;
                    }
                    case 'P': //电源
                    {
                        break;
                    }
                    case 'D': //露点
                    {
                        break;
                    }
                    default:
                        bGetWO = false;

                }//End of switch(chCmd3)
                */

                bGetWO = mvParseCmdString(strMsg, g_ExpoMonitorInfo, true);

                //发送接收到主动上传数据标志
                if(bGetWO)
                {
                    LogNormal("===主动报警=WOOK=\n");

					if (13 == g_nServerType)
					{
						g_MyCenterServer.mvCheckDeviceStatus(2); //向Ver1.8中心端报警				
					}
					else
					{
						g_CenterServer.mvCheckDeviceStatus(2); //向中心端报警
					}
				


                    // WO+OK〈CR〉〈LF〉（表示接受成功）
                    std::string strMsg = "WOOK\r";
                    return mvRebMsgAndSend(m_nCenterSocket, GATE_WO_OK, strMsg);
                }
                else
                {
                    // WO+ER,出错代码〈CR〉〈LF〉（表示接受失败等）
                    //std::string strMsg = "WOER\r";
                    //return mvRebMsgAndSend(m_nCenterSocket, GATE_WO_ER, strMsg);
                }

                break;
            } //End of case 'W'
            default:
            {
                //...
            }

        }//End of switch (chCMD1)

    }
    else if( (mHeader->chCmd & 0xf0 ) == 0x70) //系统命令
    {
        printf("================系统命令==\n");
/*
        BYTE chCMD1 = *( (BYTE*)(strMsg.c_str() + sizeof(mHeader)) );
        BYTE chCMD2 = *( (BYTE*)(strMsg.c_str() + sizeof(mHeader) + 1) );
        switch (chCMD1)
        {
            case 'P': //PL ①	连接命令
            {
                break;
            }
        }

        BYTE chCmd3 = *( (BYTE*)(strMsg.c_str() + sizeof(mHeader) + 2) ); //DT

        BYTE nCodeType;
        BYTE nCodeId;
        BYTE nValueType;
        BYTE nValue;

        mvParseCmdString(strMsg, nCodeType, nCodeId, nValueType, nValue);
        printf("=========CGateKeeperSocket::mvOnDealOneMsg==系统命令===[%c %d], #%d, %d==\n", nCodeType, nCodeId, nValueType, nValue);
        */
    }
    else
    {
        BYTE chTemp = mHeader->chCmd & 0xf0;
        printf("==ERROR!!===其他命令====chTemp=0x %x====mHeader->chCmd=0x %x==\n", chTemp, mHeader->chCmd);
    }

	return true;
}

/*
* 函数介绍：发送心跳测试
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CGateKeeperSocket::mvSendLinkTest()
{
    //LogNormal("==CGateKeeperSocket::mvSendLinkTest()==\n");
    //LogNormal("==m_nCenterSocket=%d==m_nPort=%d=\n", m_nCenterSocket, m_nPort);

    if(m_nCenterSocket == 0)
     return false;


    UINT32 time = GetTimeStamp();//系统当前时间
    std::string strTIME = GetTime(time, 2);

    // GATE_LINK_TEST
    std::string strMsg = "PLDT," + strTIME + "\r";
    strMsg += "ID,MiddleWare\r";
    return mvRebMsgAndSend(m_nCenterSocket, GATE_LINK_TEST, strMsg);

    //获取门状态信息--无源点状态
    /*
    std::string strMsg = "RDR1,#1\r";
    bool bGetDoorStatus = mvRebMsgAndSend(m_nCenterSocket, GATE_GET_DOOR_STATUS, strMsg);
    if(!bGetDoorStatus)
    {
        LogError("=CGateKeeperSocket::mvSendLinkTest===获取门状态信息--失败!!!===\n");
    }

    return bGetDoorStatus; */
    //return true;
}


/*
* 函数介绍：重组消息并发送
* 输入参数：pCode-要发送的消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CGateKeeperSocket::mvRebMsgAndSend(int nSocket, UINT32 uCode, const string &strMsg)
{
    /*if(uCode != GATE_LINK_TEST)
    {
        LogNormal("=重组消息并发送=nSocket=%d==strMsg.size()=%d==\n", nSocket, strMsg.size());
    }*/


   // UINT32 time = GetTimeStamp();//系统当前时间
    //std::string strTIME = GetTime(time, 2);
    //printf("[%s]===========CGateKeeperSocket::mvRebMsgAndSend======nSocket=%d==\n", strTIME.c_str(), nSocket);
    //mvCloseSocket(m_nCenterSocket);
    //m_nCenterSocket = nSocket;

    //rebuild the message according to the msg-code and then send it to the center server.
    string strFullMsg("");
    unsigned char uDataLenth = strMsg.size();//需要注意"大小端"

    GATEKEEPER_HEADER m_GateHeader;
    m_GateHeader.chCmd = 0;
    /*
    m_GateHeader.nSpknHigh = 0;
    m_GateHeader.nSpknLow = 0;
    m_GateHeader.nRpknHigh = 0;
    m_GateHeader.nRpknLow = 0;
    */

    //新产生请求的数据报号加1
    m_nSpkn++;

    //设置发送消息时的请求和响应数据报号
    mvSetDataOrder(m_GateHeader);

    m_GateHeader.nDataLenthLow = 0;
    //m_GateHeader.nDataLenthHigh = 0;
    m_GateHeader.chPid = 0;

    if(uCode == GATE_LINK_TEST) //心跳维持
    {
        m_GateHeader.chCmd = 0x071;
        //m_GateHeader.nSpknLow = 0x01;
        //m_GateHeader.nSpknLow = 0;

        printf("=m_GateHeader.nDataLenthHigh=%d====m_GateHeader.nDataLenthLow=%d=====\n", m_GateHeader.nDataLenthHigh, m_GateHeader.nDataLenthLow);
        m_GateHeader.nDataLenthLow = (BYTE)(uDataLenth) + 1; //注意长度应为高低端数据拼接起来结果!!!
        //m_GateHeader.nDataLenthHigh = (BYTE)(uDataLenth);
    }
    else if(uCode == GATE_GET_DOOR_STATUS)
    {
        m_GateHeader.chCmd = 0x61;
        //m_GateHeader.nSpknLow = 0x01;

        m_GateHeader.nDataLenthLow = (BYTE)(uDataLenth) + 1;

        //LogNormal("===GATE_GET_DOOR_STATUS=====m_GateHeader.nDataLenthLow=%d=====\n", m_GateHeader.nDataLenthLow);
    }
    else if(uCode == GATE_GET_TEMPERATURE)
    {
        m_GateHeader.chCmd = 0x61;
        //m_GateHeader.nSpknLow = 0x01;

        m_GateHeader.nDataLenthLow = (BYTE)(uDataLenth) + 1;

        //LogNormal("===GATE_GET_TEMPERATURE======m_GateHeader.nDataLenthLow=%d=====\n", m_GateHeader.nDataLenthLow);
    }
    else if(uCode == GATE_SHAKE_STATUS) //震动报警
    {
        m_GateHeader.chCmd = 0x61;
        //m_GateHeader.nSpknLow = 0x01;

        m_GateHeader.nDataLenthLow = (BYTE)(uDataLenth) + 1;

        printf("===GATE_SHAKE_STATUS======m_GateHeader.nDataLenthLow=%d=====\n", m_GateHeader.nDataLenthLow);
    }
    else
    {
        printf("=CGateKeeperSocket::mvRebMsgAndSend==Other RD !!!====");
        m_GateHeader.chCmd = 0x61;;
        m_GateHeader.nDataLenthLow = (BYTE)(uDataLenth) + 1;
    }

    strFullMsg.append((char*)&m_GateHeader,sizeof(m_GateHeader)); //包头


    if (!strMsg.empty())
    {
        strFullMsg += strMsg;//包体
    }
    strFullMsg += "\n";//包尾

    //printf("====sizeof(BYTE)=%d==sizeof(unsigned short)=%d====sizeof(m_GateHeader)=%d=======\n", sizeof(BYTE), sizeof(unsigned short), sizeof(m_GateHeader));
    printf("mvRebMsgAndSend=%s,uCode=%x,strMsg.size()=%d,strFullMsg.size()=%d\n",strMsg.c_str(),uCode,strMsg.size(),strFullMsg.size());
/*
    //建立临时文件
    FILE* fp = fopen("./GATE_LOG.data","ab+");
    if(fp!=NULL)
    {
        char buf[4096];
        sprintf(buf, "mvRebMsgAndSend=%s,uCode=%x,strMsg.size()=%d,strFullMsg.size()=%d\n",strMsg.c_str(),uCode,strMsg.size(),strFullMsg.size());
        std::string strMsgT = buf;
        fwrite(strMsgT.c_str(), strMsgT.size(), 1, fp);
        fclose(fp);
    }
*/
    if (!mvSendMsgToSocket(m_nCenterSocket, strFullMsg))
    {
        mvCloseSocket(m_nCenterSocket);
        LogError("======ERROR Close connect...===发送消息失败，连接断开\n");
        return false;
    }

    return true;
}

/*
* 函数介绍：解析数据包
* 输入参数：strMsg-消息内容；bFlag:是否主动报警
* 输出参数：nCodeType:对应环境监控设备消息类型 nACodeId:消息类型编号 nAAValueType:值类型数组 nAAAValue:值数组
* 返回值 ：成功返回true，否则返回false
*/
bool CGateKeeperSocket::mvParseCmdString(const std::string &strMsg, EXPO_MONITOR_INFO & ExpoMonitorInfo, bool bFlag)
{
	if(strMsg.size() < sizeof(GATEKEEPER_HEADER)+2)
	{
		return false;
	}
    //eg strMsg = [Header] + RD [ T0,#1,32 \r T0,#2,0 \r T0,#4,60 \r T0,#5,-10 \r T0,#6,1 ];
    int nHeadSize = sizeof(GATEKEEPER_HEADER);
    char chCodeType = *( (char*)(strMsg.c_str() + nHeadSize+ 2) );
    bool bGetData = false;

    int nAAValue[MAX_GATE_ARRAY_SIZE][MAX_GATE_ARRAY_SIZE] = {0};
    int nCodeIdIndex = 0; //消息类型 0 - (8 - 1)
    int nValueTypeIndex = 0; //值类型 0 - (8*8 - 1)
    int nValueIndex = 0; //值 0 - (8*8*8 - 1)

    std::string::size_type nFlagPos = 0; //按每个项目的首尾index标--strMsg.c_str()开始--base 0
    std::string::size_type nFlagPosPrev = 0;

    std::string::size_type nFlagElemPos = 0;
    std::string::size_type nFlagElemPosPrev = 0;
    char chCR = '\r';
    char chLF = '\n';
    char chDouHao = ',';
    std::vector<std::string> strVec;

    std::string strTemp = "";
    std::string strTemp2 = "";
    char chTemp[2] = {0};
    char bufTemp[20] = {0};

    nFlagPosPrev = nHeadSize + 2;
    nFlagPos = nFlagPosPrev;
    while( (nFlagPos = strMsg.find_first_of(chCR, nFlagPos)) != std::string::npos )
    {
        strTemp = strMsg.substr(nFlagPosPrev, nFlagPos - nFlagPosPrev);
        strVec.push_back(strTemp);
        //strVec
        //LogNormal("=[%d,%d]===strVec.size()=%d==strTemp=%s \n", nFlagPosPrev, nFlagPos, strVec.size(), strTemp.c_str());
/*
        //建立临时文件
        FILE* fp = fopen("./GATE_LOG.data","ab+");
        if(fp!=NULL)
        {
            char buf[4096];
            sprintf(buf, "=[%d,%d]===strVec.size()=%d==strTemp=%s \n", nFlagPosPrev, nFlagPos, strVec.size(), strTemp.c_str());
            std::string strMsgT = buf;
            fwrite(strMsgT.c_str(), strMsgT.size(), 1, fp);
            fclose(fp);
        }
*/

        nFlagElemPos = 0;
        nFlagElemPosPrev = 0;
        ////////////////////////////////////////////////////////////////////////////////////
        //找逗号
        while( (nFlagElemPos = strTemp.find_first_of(chDouHao, nFlagElemPos)) != std::string::npos  )
        {
            strTemp2 = strTemp.substr(nFlagElemPosPrev,  nFlagElemPos - nFlagElemPosPrev);

            //strTemp2
            //LogNormal("=[%d,%d]=====strTemp2=%s \n", nFlagElemPosPrev, nFlagElemPos, strTemp2.c_str());


            //存储类型
            chTemp[0] = *( (char*)(strTemp2.c_str()) ); //T or # ...
            chTemp[1] = *( (char*)(strTemp2.c_str() + 1) );
            if(chTemp[0] == '#')
            {
                //值类型
                nValueTypeIndex = (char)chTemp[1] - '0';
            }
            else
            {
                //消息类型
                nCodeIdIndex = (char)chTemp[1] - '0';
            }


            nFlagElemPosPrev = nFlagElemPos + 1;
            nFlagElemPos++;
        }


        //..
        strTemp2 = strTemp.substr(nFlagElemPosPrev,  strTemp.size() - nFlagElemPosPrev);
        //LogNormal("=[%d,%d]=====strTemp2=%s \n", nFlagElemPosPrev, strTemp.size(), strTemp2.c_str());

        strcpy(bufTemp, strTemp2.c_str());
        //值
        nAAValue[nCodeIdIndex][nValueTypeIndex] = atoi(bufTemp);
        //LogNormal("=nAAValue[%d][%d]=%d\n", nCodeIdIndex, nValueTypeIndex, nAAValue[nCodeIdIndex][nValueTypeIndex]);
        ////////////////////////////////////////////////////////////////////////////////////

        chCodeType = *((char*)strTemp.c_str());
        //LogNormal("=chCodeType=%c=\n", chCodeType);
        switch(chCodeType)
        {
            case 'R': //无源触点
            {
                break;
            }
            case 'E': //设备
            {
                break;
            }
            case 'W': //警笛
            {
                break;
            }
            case 'A': //防拆警报--开关门状态
            {
                //A0,#2,X
				if(nValueTypeIndex == 2)
				{
					ExpoMonitorInfo.nGateValue = nAAValue[nCodeIdIndex][2];
					LogNormal("==机箱门状态[0:关闭1:打开]=%d=\n", ExpoMonitorInfo.nGateValue);     
				}
                break;
            }
            case 'T': //温度
            {
				if(!bFlag)
				{
					//T0,#1,X
					if(nValueTypeIndex == 1)
					{
						ExpoMonitorInfo.nTemperatureValue = nAAValue[nCodeIdIndex][1];
						LogNormal("==机箱当前温度:%d",ExpoMonitorInfo.nTemperatureValue);
					}
					//T0,#4,X
					else if(nValueTypeIndex == 4)
					{
						ExpoMonitorInfo.nTemperatureUp = nAAValue[nCodeIdIndex][4];
						LogNormal("==机箱温度上限:%d",ExpoMonitorInfo.nTemperatureUp);
					}
					//T0,#5,X
					else if(nValueTypeIndex == 5)
					{
						ExpoMonitorInfo.nTemperatureDown = nAAValue[nCodeIdIndex][5];
						LogNormal("==机箱温度下限:%d",ExpoMonitorInfo.nTemperatureDown);
					}
				}
                else //温度超限主动向平台报警
               {
				   //T0,#1,X
				   if(nValueTypeIndex == 1)
				   {
						ExpoMonitorInfo.nTemperatureValue = nAAValue[nCodeIdIndex][1];
						LogNormal("==机箱当前温度:%d",ExpoMonitorInfo.nTemperatureValue);
				   }
               }

                break;
            }
            case 'H': //湿度
            {
                break;
            }
            case 'P': //电源
            {
                break;
            }
            case 'D': //露点
            {
                break;
            }
            default:
                bGetData = false;
        }//End of switch(chCmd3)

        nCodeIdIndex = 0;
        nValueTypeIndex = 0;

        nFlagPosPrev = nFlagPos + 1;
        nFlagPos++;
    }//End of while

/*
    for(int i=0; i<MAX_GATE_ARRAY_SIZE; i++)
    {
        for(int j=0; j<MAX_GATE_ARRAY_SIZE; j++)
        {
            printf("==nAAValue[%d][%d]=%d=", i, j, nAAValue[i][j]);

            FILE* fp2 = fopen("./GATE_LOG_2.data","ab+");
            //建立临时文件
            if(fp2!=NULL)
            {
                char buf[256];
                sprintf(buf, "==nAAValue[%d][%d]=%d==============\n", i, j, nAAValue[i][j]);
                std::string strMsgT = buf;
                fwrite(strMsgT.c_str(), strMsgT.size(), 1, fp2);
                fclose(fp2);
            }

        }

        printf("\n");
    }
*/

    //printf("====================CGateKeeperSocket::mvParseCmdString====[%c %d], #%d, %d==\n", nCodeType, nCodeId, nValueType, nValue);



    return true;
}

//获取环境监控状态信息
bool CGateKeeperSocket::mvGetGateKeeperInfo()
{
    //UINT32 time = GetTimeStamp();//系统当前时间
    //std::string strTIME = GetTime(time, 2);
    //LogNormal("##[%s]##====CGateKeeperSocket::mvGetGateKeeperInfo()=\n",strTIME.c_str());

    if(m_nCenterSocket == 0)
    {
        LogNormal("==CGateKeeperSocket::mvGetGateKeeperInfo()==m_nCenterSocket=0!!=\n");
        return false;
    }


    std::string strMsg = "";
    //mvCloseSocket(m_nCenterSocket);
    //m_nCenterSocket = nSocket;
    bool bGetStatus = false;

    //获取门状态信息--无源点状态
   /* strMsg = "RDR0,#0\r";
    bool bGetDoorStatus = mvRebMsgAndSend(m_nCenterSocket, GATE_GET_DOOR_STATUS, strMsg);
    if(!bGetDoorStatus)
    {
        LogError("==获取门状态信息--失败!!!===\n");
    }*/

    //获取温度信息 #0
    strMsg = "RDT0,#0\r";
    bool bGetTemperature = mvRebMsgAndSend(m_nCenterSocket, GATE_GET_TEMPERATURE, strMsg);
    if(!bGetTemperature)
    {
        LogError("==获取监控设备温度信息--失败!!!===\n");
    }

    //E1-E2 #1
    /*for(int i=0; i<2; i++)
    {
        //strMsg = "RDE1,#1\r";
        char buf[128];
        sprintf(buf, "RDE%d,#0\r", i+1);
        strMsg = buf;

        bGetStatus = mvRebMsgAndSend(m_nCenterSocket, 0xefff0000, strMsg);
        if(!bGetStatus)
        {
            LogError("==获E1设备状态信息--失败!!!===\n");
        }
    }*/

    //W0（警笛）#0
   /* strMsg = "RDW0,#0\r";
    bGetStatus = mvRebMsgAndSend(m_nCenterSocket, 0xefff0000, strMsg);
    if(!bGetStatus)
    {
        LogError("==获警笛状态信息--失败!!!===\n");
    }*/

    //A0（防拆警报）#0
    strMsg = "RDA0,#0\r";
    bGetStatus = mvRebMsgAndSend(m_nCenterSocket, 0xefff0000, strMsg);
    if(!bGetStatus)
    {
        LogError("==获防拆警报状态信息--失败!!!===\n");
    }

    //获取温度信息 #0
    /*strMsg = "RDT0,#0\r";
    bool bGetTemperature = mvRebMsgAndSend(m_nCenterSocket, 0xefff0000, strMsg);
    if(!bGetTemperature)
    {
        LogError("==获取监控设备温度信息--失败!!!===\n");
    }*/

    //H0（湿度）#0
    /*strMsg = "RDH0,#0\r";
    bGetStatus = mvRebMsgAndSend(m_nCenterSocket, 0xefff0000, strMsg);
    if(!bGetStatus)
    {
        LogError("==获湿度信息--失败!!!===\n");
    }*/

    //P1—P6（电源）#0
    /*for(int i=0; i<6; i++)
    {
        //strMsg = "RDP1,#0\r";
        char buf[128];
        sprintf(buf, "RDP%d,#0\r", i+1);
        strMsg = buf;
        bGetStatus = mvRebMsgAndSend(m_nCenterSocket, 0xefff0000, strMsg);
        if(!bGetStatus)
        {
            LogError("==获电源P1信息--失败!!!===\n");
        }
    }*/


    //D0（露点）#1
    /*strMsg = "RDD0,#1\r";
    bGetStatus = mvRebMsgAndSend(m_nCenterSocket, 0xefff0000, strMsg);
    if(!bGetStatus)
    {
        LogError("==获露点状态信息--失败!!!===\n");
    }*/

    //GATE_SHAKE
    /*
    std::string strMsg("RDW0,#0\r");
    return mvRebMsgAndSend(nSocket, GATE_SHAKE, strMsg);
    */
    return true;
}

/*
* 函数介绍：设置请求和响应数据报号
* 输入参数：无
* 输出参数：m_GateHeader:消息头
* 返回值 ：成功返回true，否则返回false
*/
bool CGateKeeperSocket::mvSetDataOrder(GATEKEEPER_HEADER &m_GateHeader)
{
    //新产生请求的数据报号为前一个响应报号加1
    //m_nSpkn = m_nRpkn + 1;

//请求数据报号
    m_GateHeader.nSpknHigh = m_nSpkn / 256;
    m_GateHeader.nSpknLow = m_nSpkn % 256;
//响应数据报号
    m_GateHeader.nRpknHigh = m_nRpkn / 256;
    m_GateHeader.nRpknLow = m_nRpkn % 256;

    printf("=mvSetDataOrder==m_GateHeader.nSpknHigh=%d, m_GateHeader.nSpknLow=%d===\n", m_GateHeader.nSpknHigh, m_GateHeader.nSpknLow);
    printf("=mvSetDataOrder==m_GateHeader.nRpknHigh=%d, m_GateHeader.nRpknLow=%d===\n", m_GateHeader.nRpknHigh, m_GateHeader.nRpknLow);

    return true;
}



