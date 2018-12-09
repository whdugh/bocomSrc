// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "Common.h"
#include "CommonHeader.h"
#include "BocomServer.h"
#include "XmlParaUtil.h"
#include "FtpCommunication.h"

#ifdef FLVFORMAT
#include "FlvConvert.h"
#endif


//监控线程
void* ThreadBocomAccept(void* pArg)
{
	//取类指针
	CBocomServer* pBocomServer = (CBocomServer*)pArg;
	if(pBocomServer == NULL)
		return pArg;

	int nSocket = pBocomServer->GetAcceptSocket();
	//中心端连接
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
                pthread_exit((void *)0);
				return pArg;
			}
	//		LogNormal("accept nClient = %d\r\n",nClient);
			//自动重启
			continue;
		}

		//输出用户连接
		LogNormal("中心端连接[IP:%s][nClient = %d,端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));

		SRIP_CLEINT sClient;
		sClient.nSocket = nClient;
		sClient.nPort = ntohs(clientaddr.sin_port);

		sClient.uTimestamp = GetTimeStamp();
		strcpy(sClient.chHost,inet_ntoa(clientaddr.sin_addr));

		//保存客户端,并启动接收数据线程
		pBocomServer->AddClient(sClient);

		//10毫秒
		usleep(1000*10);
	}
	//LogNormal("22 accept exit\r\n");
	pthread_exit((void *)0);
	return pArg;
}

//心跳检测线程
void* ThreadBocomLink(void* pArg)
{
	//取类指针
	CBocomServer* pBocomServer = (CBocomServer*)pArg;
	if(pBocomServer == NULL)
		return pArg;

	while(!g_bEndThread)
	{
		//检测状态一次
		pBocomServer->LinkTest();

		//5秒检测一次
		sleep(5);
	}
	pthread_exit((void *)0);
	return pArg;
}

//客户端接收线程
void* ThreadBocomRecv(void* pArg)
{
	//取类指针
	CBocomServer* pBocomServer = (CBocomServer*)pArg;
	if(pBocomServer == NULL)
		return pArg;

    pBocomServer->mvRecvCenterServerMsg();

    LogError("接收中心端消息线程退出\r\n");
    pthread_exit((void *)0);
}

//记录发送线程
void* ThreadBocomResult(void* pArg)
{
	//取类指针
	CBocomServer* pBocomServer = (CBocomServer*)pArg;
	if(pBocomServer == NULL)
		return pArg;

	//处理一条数据
	pBocomServer->DealResult();

    pthread_exit((void *)0);
	return pArg;
}


//历史记录发送线程
void* ThreadBocomHistoryResult(void* pArg)
{
	//取类指针
	CBocomServer* pBocomServer = (CBocomServer*)pArg;
	if(pBocomServer == NULL)
		return pArg;

	int nDayNight = 0;
     while(!g_bEndThread)
     {
		 //LogNormal("History Send before:%s\n",GetTimeCurrent().c_str());
        //处理一条数据
        pBocomServer->mvDealHistoryRecord();
		//LogNormal("History Send after:%s\n",GetTimeCurrent().c_str());
		//5秒
		struct timeval tv;
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
     }
     pthread_exit((void *)0);
	return pArg;
}

//录像发送失败重新发送线程
void* ThreadBocomReSendResult(void* pArg)
{
	//取类指针
	CBocomServer* pBocomServer = (CBocomServer*)pArg;
	if(pBocomServer == NULL)
		return pArg;

	int nDayNight = 0;
	time_t startTime;
	time_t nowTime;
	time(&startTime);

	while(!g_bEndThread)
	{
		time(&nowTime);
		int offTime = (nowTime-startTime);
		if(offTime >= 60*60)
		{
			g_skpDB.DeleteVideoRecord();
			startTime = nowTime;
		}
		pBocomServer->SendVideoForReSend();
		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
	pthread_exit((void *)0);
	return pArg;
}

//构造
CBocomServer::CBocomServer()
{
	pthread_mutex_init(&m_Msg_mutex,NULL);
	//发送信号

	pthread_mutex_init(&m_Result_Mutex,NULL);

	pthread_mutex_init(&m_thread_mutex,NULL);

	pthread_mutex_init(&m_Video_Mutex,NULL);

	m_nThreadId = 0;
	m_nHistoryThreadId = 0;
	m_nResendThreadId = 0;

	m_nAcceptSocket = 0;

	m_nCenterSocket = 0;

	m_nLinker = 0;

	m_bVerify = false;

	m_nVideoType = 0;

	m_nId = 0;

#ifdef _DEBUG
	m_nMsgCount = 0;//test
#endif

#ifdef KAFKA_SERVER
	m_kafkaManage = NULL;
#endif

	return;
}
	//析构
CBocomServer::~CBocomServer()
{
	//发送信号
	//
	pthread_mutex_destroy(&m_Msg_mutex);

	pthread_mutex_destroy(&m_Result_Mutex);

	pthread_mutex_destroy(&m_thread_mutex);

	pthread_mutex_destroy(&m_Video_Mutex);

	//关闭连接
	mvCloseSocket(m_nAcceptSocket);

#ifdef KAFKA_SERVER
	if(m_kafkaManage)
	{
		delete m_kafkaManage;
		m_kafkaManage = NULL;
	}	
#endif

	return;
}

//启动侦听服务
bool CBocomServer::Init(int nPort, int nId)
{
    //创建套接字
	if(mvCreateSocket(m_nAcceptSocket,1)==false)
	{
	    LogError("创建套接字失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//重复使用套接字
	if(mvSetSocketOpt(m_nAcceptSocket,SO_REUSEADDR)==false)
	{
	    LogError("设置重复使用套接字失败,服务无法启动!,%s\r\n",strerror(errno));
	    g_bEndThread = true;
		return false;
	}

	//////////////////////////
	m_nPort = nPort;
	m_nId = nId;
	//绑定服务端口
	if(mvBindPort(m_nAcceptSocket,m_nPort)==false)
	{
        LogError("绑定到 %d 端口失败,服务无法启动!\r\n",m_nPort);
        printf("%s\n",strerror(errno));
	    g_bEndThread = true;
		return false;
	}

	//开始监听
	if (mvStartListen(m_nAcceptSocket) == false)
	{
		LogError("监听连接失败，服务无法启动!\r\n");
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
	int nret=pthread_create(&id,&attr,ThreadBocomAccept,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建事件接收线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动心跳检测线程
	nret=pthread_create(&id,&attr,ThreadBocomLink,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建连接心跳检测线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动接收消息线程
	nret=pthread_create(&id,&attr,ThreadBocomRecv,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建接收消息线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动检测结果发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	nret=pthread_create(&m_nThreadId,&attr,ThreadBocomResult,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动历史结果发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadBocomHistoryResult,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动录像发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	nret=pthread_create(&m_nResendThreadId,&attr,ThreadBocomReSendResult,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建重新发送线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}
	pthread_attr_destroy(&attr);

#ifdef KAFKA_SERVER
	if (26 == g_nServerType && g_Kafka.uSwitchUploading == 1)
	{
		bool bInitKafka = InitKafkaClient();
		if(!bInitKafka)
		{
			LogError("初始化kafkaClient失败!");
		}
	}
#endif

	return true;
}

//释放
bool CBocomServer::UnInit()
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

	if(m_nResendThreadId != 0)
	{
		pthread_join(m_nResendThreadId,NULL);
		m_nResendThreadId = 0;
	}

    //需要关闭和删除掉所有的客户端连接
    DelClient();
    //关闭连接
    mvCloseSocket(m_nAcceptSocket);

    g_FtpCommunication.DoClose();

    m_ChannelResultList.clear();
    m_mapMsgList.clear();
	m_VideoName.clear();
	return true;
}

//处理数据
void CBocomServer::DealResult()
{
    std::string response("");

    while(!g_bEndThread)
	{
		//////////////////////////////////////////////////////////先取检测
		if (!response.empty())
        {
            response.clear();
        }

	    //加锁
	    pthread_mutex_lock(&m_Result_Mutex);

		//判断是否有命令
		if(m_ChannelResultList.size()>0)
		{
			//取最早命令
			MsgResult::iterator it = m_ChannelResultList.begin();
			//保存数据
			response = *it;
			//删除取出的命令
			m_ChannelResultList.pop_front();
		}
		//解锁
		pthread_mutex_unlock(&m_Result_Mutex);

		//处理消息
		if(response.size()>0)
		{
			//LogNormal("DealResult Send before:%s[%lld]\n",GetTimeCurrent().c_str(),response.size());
			OnResult(response);
			//LogNormal("DealResult Send after:%s[%lld]\n",GetTimeCurrent().c_str(),response.size());
		}

		//1毫秒
		usleep(1000*1);
	}
}

//添加一条数据
bool CBocomServer::AddResult(std::string& strResult)
{
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strResult.c_str();

	//LogNormal("CBocomServer::AddResult port:%d type:0x %x size:%d", m_nPort, sDetectHeader->uDetectType, strResult.size());
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
			m_ChannelResultList.push_front(strResult);
			//LogError("记录=%d\r\n",m_ChannelResultList.size());
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
bool CBocomServer::OnResult(std::string& result)
{
	//LogNormal("CBocomServer::OnResult port:%d size:%d \n", m_nPort, result.size());
    string strRecord("");

	//发送数据
	MIMAX_HEADER mHeader;
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();

	bool bSendToServer = false;
	bool bViolationType = false;

#ifdef _DEBUG
	m_nMsgCount++;
	LogNormal("port:%d OnMsg type:0x%x count:%d ", m_nPort, sDetectHeader->uDetectType, m_nMsgCount);
#endif

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
				//result.erase(0,sizeof(SRIP_DETECT_HEADER));
				strRecord.append((char*)(result.c_str()+sizeof(SRIP_DETECT_HEADER)),result.size()-sizeof(SRIP_DETECT_HEADER));
				bSendToServer = true;
			}
			break;
		default:
			LogError("未知数据[%d],取消操作!\r\n",sDetectHeader->uChannelID);
			return false;
	}

	//return true;

#ifdef KAF_DEBUG
	int nVtsType = 0;
#endif

    //此时去取图片
    if(mHeader.uCmdID == MIMAX_EVENT_REP)
    {
        BOCOM_RECORD_EVENT* sEvent = (BOCOM_RECORD_EVENT*)(strRecord.c_str());
		if(5 == g_nServerType)
		{
			//ConvertEvent(sEvent);
			sEvent->uColor1 += 1;//送中心端颜色值加1
			sEvent->uColor2 += 1;
			sEvent->uColor3 += 1;
		}
		
        if(g_nSendImage)
        {
            String strPicPath(sEvent->chPicPath);
            string strPic = GetImageByPath(strPicPath);
            strRecord.append((char*)strPic.c_str(),strPic.size());
        }
        else
        {
	        sEvent->uPicSize = 0;
	        sEvent->uPicWidth = 0;
	        sEvent->uPicHeight = 0;
        }
    }
    else if(mHeader.uCmdID == MIMAX_PLATE_REP)
    {
		RECORD_PLATE* sPlate = (RECORD_PLATE*)(strRecord.c_str());
        if(sPlate->uViolationType > 0 && sPlate->uViolationType != DETECT_RESULT_NOCARNUM)
        {
            bViolationType = true;
        }

		if(!((26 == g_nServerType)&&(!bViolationType)))
		{
			string strPlate = ConvertPlate(*sPlate);
			strRecord = strPlate;
		}
    }

	//LogNormal("CBocomServer::OnResult 222 ");
	//LogNormal("g_nSendImage:%d strRecord.size:%d bSendToServer:%d ",g_nSendImage, strRecord.size(), bSendToServer);

    //数据长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + strRecord.size();
	//添加头
	strRecord.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	
	bool bSendStatus = false;
    //发送数据
    if(bSendToServer)
	{
        if(26 == g_nServerType)
		{
			if(bViolationType)//违章,发往iVap平台
			{
				bSendStatus = SendToServer(strRecord);
			}			
			else //卡口,发往Kafka平台
			{
			    
				bSendStatus = SendTokafka(strRecord);
			}						
		}
        else if(4 == g_nServerType)//江宁电警
        {
            printf("==============g_nServerType=%d\n",g_nServerType);
            if( (mHeader.uCmdID == MIMAX_EVENT_REP)||(mHeader.uCmdID == MIMAX_PLATE_REP))
            {
                    if(bViolationType)//电警结果需要通过ftp发送
                    {
                        //
                        bSendStatus = SendToFtpServer(strRecord);
                    }
                    else
                    {
                        bSendStatus = SendToServer(strRecord);
                    }
            }
        }
		else//博康中心端
		{
			bSendStatus = SendToServer(strRecord);
		}
    }

    if(bSendStatus)
    {
       unsigned int uSeq =*((unsigned int*)(strRecord.c_str()+sizeof(MIMAX_HEADER)));
       g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq,m_nId);

       return true;
    }
	else
	{
		return false;
	}
}

//传送数据到kafka服务端
bool CBocomServer::SendTokafka(const string& strData)
{
	#ifdef KAFKA_SERVER
				if(g_Kafka.uSwitchUploading == 1)
				{
					//构造RECORD_PLATE记录					
					RECORD_PLATE *sPlate = (RECORD_PLATE *)(strData.c_str()+sizeof(MIMAX_HEADER));

					if (m_kafkaManage->GetServerStatus() == true)
					{
						string strKafkaMsg = m_kafkaManage->CreateKafkaMsg(sPlate);
						if (m_kafkaManage->SendMessage(strKafkaMsg))
						{
#ifdef KAF_DEBUG
							LogNormal("kafka SendMessage ok uSeq:%d", sPlate->uSeq);
#endif
							return true;
						}
						else
						{
							LogNormal("kafka SendMessage fail uSeq:%d", sPlate->uSeq);
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					return true;
				}
#endif
				return false;
}



//传送数据到服务端
bool CBocomServer::SendToServer(const string& strData)
{
	//LogNormal("CBocomServer::SendToServer strData.size=%d m_nCenterSocket:%d \n", strData.size(), m_nCenterSocket);
	//服务端的发送数据为向所有的连接发送数据
	bool bRet = false;
	//取所有连接，发送数据

    //加锁
    pthread_mutex_lock(&m_thread_mutex);
	if(m_nCenterSocket > 0)
	{
        //tcp发送视频数据和检测信息
        if(!SendMsg(m_nCenterSocket,strData))
        {
            bRet = false;

            LogError("SendToServer error\n");

            //关闭连接
            DelClient();
        }
        else
        {
            bRet = true;
        }
        /////////////////////////////////////
	}
	//解锁
    pthread_mutex_unlock(&m_thread_mutex);

	return bRet;
}

//添加一条命令
bool CBocomServer::AddMsg(const int nSocket,const UINT32 uCommand,const std::string request)
{
    //LogNormal("AddMsg uCommand=%x",uCommand);
    switch(uCommand)
	{
		case PLATE_LOGIN:		//中心端登录
        case EVENT_LOGIN:
			return OnLogin(nSocket, uCommand,request);
		case PLATE_LINK:			//心跳
		case EVENT_LINK:
			return OnLink(nSocket, request);
        case PLATE_REALTIME_LOG:
		case EVENT_REALTIME_LOG:
		case PLATE_NOREALTIME_LOG:
		case EVENT_NOREALTIME_LOG:
		case MIMAX_REALTIME_STATISTIC:
		case MIMAX_NOREALTIME_STATISTIC:
		case MIMAX_REALTIME_EVENT:
		case MIMAX_NOREALTIME_EVENT:
		case MIMAX_REALTIME_PLATE:
		case MIMAX_NOREALTIME_PLATE:    //是否实时推送
			return OnRealTime(nSocket, request);
		default:
			break;
	}

    // Other msgs will be put into queue.
	pthread_mutex_lock(&m_Msg_mutex);
	m_mapMsgList.insert(make_pair(nSocket, request));
	pthread_mutex_unlock(&m_Msg_mutex);
}

//删除命令
bool CBocomServer::DelMsg(int nSocket)
{
	//加锁
	pthread_mutex_lock(&m_Msg_mutex);

    //删除列表
    m_mapMsgList.clear();

	//解锁
	pthread_mutex_unlock(&m_Msg_mutex);

	return true;
}

//处理一条命令
bool CBocomServer::mvOnDealOneMsg()
{
    int nSocket = 0;
    string strMsg("");
    PopMsg(nSocket,strMsg);

    if(strMsg.size() > 0)
    {
        OnMsg(nSocket,strMsg);
    }
    return true;
}

//弹出消息
bool CBocomServer::PopMsg(int& nSocket,std::string& response)
{
	//std::string response("");
	//加锁
	pthread_mutex_lock(&m_Msg_mutex);
	//判断是否有命令
	if (!m_mapMsgList.empty())
    {
        ListMsgMap::iterator it = m_mapMsgList.begin(); //we can't fetch a message out by order of the time it was inserted in.
        //保存数据
        nSocket = it->first;
        response = it->second;
        m_mapMsgList.erase(it);
    }
	//解锁
	pthread_mutex_unlock(&m_Msg_mutex);

	return true;
}

bool CBocomServer::OnMsg(int nSocket,std::string response)
{
    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)response.c_str();
	//处理命令
	switch((sHeader->uMsgCommandID))
	{
        ////////////////////////////////////////////////////////////////////////////////////以下是博康命令处理函数
		case MIMAX_PLATE_REGION:		//获取车牌区域
			return OnPlateRegion(nSocket,response);
		case MIMAX_PLATE:
		case MIMAX_SEQ_PLATE:
		case MIMAX_NONE_PLATE://车牌查询
			return OnSearchPlate(nSocket,response);
		case MIMAX_EVENT:
		case MIMAX_SEQ_EVENT:
		case MIMAX_NONE_EVENT://事件查询
			return OnSearchEvent(nSocket,response);
		case MIMAX_STATISTIC:
		case MIMAX_SEQ_STATISTIC:
		case MIMAX_NONE_STATISTIC://统计查询
			return OnSearchStatistic(nSocket,response);
		case PLATE_LOG:
		case EVENT_LOG:
		case PLATE_SEQ_LOG:
		case EVENT_SEQ_LOG:
		case PLATE_NONE_LOG:
		case EVENT_NONE_LOG://日志查询
			return OnSearchLog(nSocket,response);
		case PLATE_LOG_STATUS:
		case EVENT_LOG_STATUS:
		case STATISTIC_STATUS:
		case EVENT_STATUS:
		case PLATE_STATUS:				//记录状态
			return OnRecordStatus(nSocket,response);
        //获取图片信息
        case CAPTURE_PIC_INFO:
            return OnCaptureOneFrame(nSocket,response);
        //设置停车检测区域
        case SET_STOPREGION_INFO:
            return OnSetStopRegionInfo(nSocket,response);
        //设置预置位
        case SET_PRESET_INFO:
            return OnSetPresetInfo(nSocket,response);
		//设置预置位模式
		case SET_PRESET_MODE:
			return OnSetPresetMode(nSocket,response);
		//设置系统时间
		case PLATE_SYSTIME_SETUP:
			return OnSysTime(nSocket,response);
		default:						//错误命令
			LogError("连接[%d]接收到错误的命令[%d]!\r\n",nSocket,(sHeader->uMsgCommandID));
			break;
	}
    return true;
}

//登录处理
bool CBocomServer::OnLogin(const int nSocket,const UINT32 uCommand,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//中心端登录
	if(uCommand == PLATE_LOGIN || uCommand == EVENT_LOGIN)
	{

		char chUserName[MIMAX_USERNAME] = {0};
		char chPass[MIMAX_USERPASS+1] = {0};
		memcpy(chUserName,request.c_str()+sizeof(SRIP_HEADER),MIMAX_USERNAME);
		memcpy(chPass,request.c_str()+sizeof(SRIP_HEADER)+MIMAX_USERNAME,MIMAX_USERPASS);

		std::string user(chUserName);

		std::string pass(chPass);


		printf("用户名=[%s],密码=[%s],size=%d\r\n",user.c_str(),pass.c_str(),pass.size());

		//判断密码是否正确
		uErrorCode = g_skpDB.CheckLogin(user,pass);

		std::string response;

		MIMAX_HEADER mHeader;
		//命令
		if(uCommand == PLATE_LOGIN)
		{
            mHeader.uCmdID = (PLATE_LOGIN_REP);
		}
		else if(uCommand == EVENT_LOGIN)
		{
		    mHeader.uCmdID = (EVENT_LOGIN_REP);
		}
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER);

		//判断结果
		if(uErrorCode == SRIP_OK)
		{
		    m_bVerify = true;
			mHeader.uCmdFlag = 0x00000000;
			LogNormal("中心端登录成功!\r\n");
		}
		else
		{
		    m_bVerify = false;
		    LogNormal("中心端登录失败!\r\n");
			mHeader.uCmdFlag = 0x00000001;
		}
		//加上头
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
		//发送数据
		if(!SendToServer(response))
			LogError("发送登录回复数据包失败[%d]！\r\n",nSocket);

		if(uErrorCode != SRIP_OK)
		DelClient();
	}

	return true;
}


//心跳处理
bool CBocomServer::OnLink(const int nSocket,std::string request)
{
    printf("=================================OnLink \r\n");
    //重置相关标识
    //if(!ResetLinker(nSocket))
    //return false;

    m_nLinker = 0;

	return true;
}

//是否推送实时记录(推送实时记录需要确认)
bool CBocomServer::OnRealTime(const int nSocket,std::string request)
{
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	MIMAX_HEADER mHeader;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER);
	if(sHeader->uMsgCommandID == PLATE_REALTIME_LOG)
	{
        g_skpChannelCenter.SetRealTimeLog(true);
        mHeader.uCmdID = (PLATE_REALTIME_LOG_REP);
	}
	else if(sHeader->uMsgCommandID == EVENT_REALTIME_LOG)
	{
        g_skpChannelCenter.SetRealTimeLog(true);
        mHeader.uCmdID = (EVENT_REALTIME_LOG_REP);
	}
	else if(sHeader->uMsgCommandID == MIMAX_REALTIME_STATISTIC)
	{
        g_skpChannelCenter.SetRealTimeStat(true);
        mHeader.uCmdID = (MIMAX_REALTIME_STATISTIC_REP);
	}
	else if(sHeader->uMsgCommandID == MIMAX_REALTIME_EVENT)
	{
        g_skpChannelCenter.SetRealTimeEvent(true, sHeader->uCmdFlag);
        mHeader.uCmdID = (MIMAX_REALTIME_EVENT_REP);
	}
	else if(sHeader->uMsgCommandID == MIMAX_REALTIME_PLATE)
	{
        g_skpChannelCenter.SetRealTimePlate(true, sHeader->uCmdFlag);
        mHeader.uCmdID = (MIMAX_REALTIME_PLATE_REP);
	}
	else if(sHeader->uMsgCommandID == EVENT_NOREALTIME_LOG)
	{
        g_skpChannelCenter.SetRealTimeLog(false);
        return true;
	}
	else if(sHeader->uMsgCommandID == PLATE_NOREALTIME_LOG)
	{
        g_skpChannelCenter.SetRealTimeLog(false);
        return true;
	}
	else if(sHeader->uMsgCommandID == MIMAX_NOREALTIME_STATISTIC)
	{
        g_skpChannelCenter.SetRealTimeStat(false);
        return true;
	}
	else if(sHeader->uMsgCommandID == MIMAX_NOREALTIME_EVENT)
	{
        g_skpChannelCenter.SetRealTimeEvent(false, sHeader->uCmdFlag);
        return true;
	}
	else if(sHeader->uMsgCommandID == MIMAX_NOREALTIME_PLATE)
	{
        g_skpChannelCenter.SetRealTimePlate(false, sHeader->uCmdFlag);
        return true;
	}
	std::string response;
	//数据
	response.append((char*)&mHeader,sizeof(mHeader));
	//发送数据
    if(!SendToServer(response))
        LogError("发送订阅请求回复数据包失败[%d]\r\n",nSocket);

        return true;
}

//获取车牌检测区域
bool CBocomServer::OnPlateRegion(const int nSocket,std::string request)
{
	std::string strPlateRegion = g_skpDB.GetPlateRegion();

	MIMAX_HEADER mHeader;

	mHeader.uCmdID = (MIMAX_PLATE_REGION_REP);
	mHeader.uCmdLen = sizeof(MIMAX_HEADER)+strPlateRegion.size();

	strPlateRegion.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));


	//发送数据
	if(!SendToServer(strPlateRegion))
		LogError("发送车牌检测区域回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//车牌查询
bool CBocomServer::OnSearchPlate(const int nSocket,std::string request)
{
	if(request.size() < sizeof(SRIP_HEADER)+8)
	{
		return false;
	}

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//是否需要带上大小图片
	UINT32 nType = sHeader->uCmdFlag;

	if(sHeader->uMsgCommandID==(MIMAX_PLATE))
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetPlate(uBeginTime,uEndTime);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_SEQ_PLATE))
	{
		
	}
	else if(sHeader->uMsgCommandID==(MIMAX_NONE_PLATE))
	{
	   
	}
	return true;
}

//事件查询
bool CBocomServer::OnSearchEvent(const int nSocket,std::string request)
{
	if(request.size() < sizeof(SRIP_HEADER)+8)
	{
		return false;
	}

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//是否需要发送快照图片
	UINT32 nType = sHeader->uCmdFlag;


	if(sHeader->uMsgCommandID==(MIMAX_EVENT))
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetTraEvent(uBeginTime,uEndTime);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_SEQ_EVENT))
	{
		
	}
	else if(sHeader->uMsgCommandID==(MIMAX_NONE_EVENT))
	{
	   
	}
	return true;
}

//统计查询
bool CBocomServer::OnSearchStatistic(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uMsgCommandID==(MIMAX_STATISTIC))
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetStatisticInfo(uBeginTime,uEndTime,0);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_SEQ_STATISTIC))
	{
		UINT32 uBeginSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetStatisticInfo(uBeginSeq,uEndSeq,1);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_NONE_STATISTIC))
	{
	    #ifdef SEND_NOREALTIME_RECORD
		g_skpDB.GetStatisticInfo(0,0,2,nSocket);
		#endif
	}

	return true;
}

//日志查询
bool CBocomServer::OnSearchLog(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uMsgCommandID==PLATE_LOG||sHeader->uMsgCommandID==EVENT_LOG)
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetLog(uBeginTime,uEndTime,0);
	}
	else if(sHeader->uMsgCommandID==PLATE_SEQ_LOG||sHeader->uMsgCommandID==EVENT_SEQ_LOG)
	{
		UINT32 uBeginSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetLog(uBeginSeq,uEndSeq,1);
	}
	else if(sHeader->uMsgCommandID==PLATE_NONE_LOG)
	{
	    #ifdef SEND_NOREALTIME_RECORD
		g_skpDB.GetLog(0,0,2,nSocket);
		#endif
	}

	return true;
}

/*
 * 函数介绍: 获取记录状态
 * 输入参数:
 * 输出参数:
 * 返回值:
 */
bool CBocomServer::OnRecordStatus(const int nSocket,std::string request)
{
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	int nKind = -1;
	MIMAX_HEADER mHeader;
	if(sHeader->uMsgCommandID==(PLATE_LOG_STATUS))
	{
		nKind = 4;
		mHeader.uCmdID = (PLATE_LOG_STATUS_REP);
	}
	else if(sHeader->uMsgCommandID==(EVENT_LOG_STATUS))
	{
		nKind = 4;
		mHeader.uCmdID = (EVENT_LOG_STATUS_REP);
	}
	else if(sHeader->uMsgCommandID==(PLATE_STATUS))
	{
		nKind = 3;
		mHeader.uCmdID = (PLATE_STATUS_REP);
	}
	else if(sHeader->uMsgCommandID==(STATISTIC_STATUS))
	{
		nKind = 1;
		mHeader.uCmdID = (STATISTIC_STATUS_REP);
	}
	else if(sHeader->uMsgCommandID==(EVENT_STATUS))
	{
		nKind = 0;
		mHeader.uCmdID = (EVENT_STATUS_REP);
	}
	RECORD_STATUS status = g_skpDB.GetRecordStatus(nKind);

	mHeader.uCmdLen = sizeof(MIMAX_HEADER)+sizeof(RECORD_STATUS);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)&status,sizeof(RECORD_STATUS));

	//发送数据
	if(!SendToServer(response))
	{
		LogError("发送记录状态回复数据包失败[%d]!\r\n",nSocket);
		return false;
	}

	return true;
}

//截取图片
bool CBocomServer::OnCaptureOneFrame(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


    std::string response;
    MIMAX_HEADER mHeader;
	ImageRegion imgRegion;

	if(sHeader->uMsgCommandID == CAPTURE_PIC_INFO)
	{
	    int nChannel = g_skpDB.GetChannelIDByCameraID(sHeader->uMsgSource);

        printf("====OnCaptureOneFrame uCameraID = %d,nChannel=%d\n",sHeader->uMsgSource,nChannel);

        if(nChannel > 0)
        {
            g_skpChannelCenter.CaptureOneFrame(response,nChannel,imgRegion);
        }
        else
        {
            mHeader.uCmdFlag = 0x00000001;
        }

        mHeader.uCameraID = sHeader->uMsgSource;
        mHeader.uCmdID = (CAPTURE_PIC_INFO_REP);
        //长度
        mHeader.uCmdLen = sizeof(MIMAX_HEADER)+response.size();

        response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

        //发送数据
        if(!SendToServer(response))
            LogError("发送截取大图回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}

//设置停车检测区域
bool CBocomServer::OnSetStopRegionInfo(const int nSocket, std::string request)
{
    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	std::string response;

	MIMAX_HEADER mHeader;
	mHeader.uCmdFlag = 0x00000001;

	int nChannel = g_skpDB.GetChannelIDByCameraID(sHeader->uMsgSource);
    printf("====OnSetStopRegionInfo uCameraID = %d,nChannel=%d\n",sHeader->uMsgSource,nChannel);
	if(nChannel > 0)
	{
	    CXmlParaUtil xml;
        string strStopRegion;
        if(sHeader->uCmdFlag == 0x00000000)//设置停车检测区域
        {
            if(request.size() > sizeof(SRIP_HEADER))
            {
                strStopRegion.append((char*)request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));
                if(xml.SetStopRegion(nChannel,strStopRegion))
                {
                    //需要重新启动检测
                    g_skpChannelCenter.ReloadDetect(nChannel);
                    mHeader.uCmdFlag = 0x00000000;
                }
            }
        }
        else if(sHeader->uCmdFlag == 0x00000001)//获取停车检测区域
        {
            if(xml.GetStopRegion(nChannel,response))
            {
                mHeader.uCmdFlag = 0x00000000;
            }
        }
	}

	mHeader.uCameraID = sHeader->uMsgSource;
	mHeader.uCmdID = (SET_STOPREGION_INFO_REP);
    //长度
    mHeader.uCmdLen = sizeof(MIMAX_HEADER)+response.size();

	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!SendToServer(response))
		LogError("发送设置停车检测区域回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//设置预置位信息
bool CBocomServer::OnSetPresetInfo(const int nSocket, std::string request)
{
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	UINT32 nPreSetID = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));

	CAMERA_CONFIG cfg;
	cfg.uType = 1;
	cfg.fValue = nPreSetID;

    if(sHeader->uCmdFlag == 0x00000000)//设置预置位
    {
        cfg.nIndex = SET_PRESET;
    }
    else if(sHeader->uCmdFlag == 0x00000001)//调用预置位
    {
        cfg.nIndex = GOTO_PRESET;
    }
    else if(sHeader->uCmdFlag == 0x00000002)//清除预置位
    {
        cfg.nIndex = CLEAR_PRESET;
    }

	std::string response;

	MIMAX_HEADER mHeader;

	int nChannel = g_skpDB.GetChannelIDByCameraID(sHeader->uMsgSource);

	printf("====OnSetPresetInfo uCameraID = %d,nChannel=%d,nPreSetID=%d,sHeader->uCmdFlag=%x\n",sHeader->uMsgSource,nChannel,nPreSetID,sHeader->uCmdFlag);

    if(nChannel > 0)
    {

	   int nPreSet = g_skpDB.GetPreSet(nChannel);

       g_skpChannelCenter.CameraControl(nChannel,cfg);

       if(cfg.nIndex == GOTO_PRESET)//调用预置位
       {
		   LogNormal("调用预置位nPreSetID=%d,nPreSet=%d",nPreSetID,nPreSet);

           if(nPreSet != nPreSetID)//预置位变换需要重新启动检测
           {
               g_skpChannelCenter.ReloadDetect(nChannel);
           }
       }
    }
    else
    {
        mHeader.uCmdFlag = 0x00000001;
    }

	mHeader.uCameraID = sHeader->uMsgSource;
	mHeader.uCmdID = (SET_PRESET_INFO_REP);
    //长度
    mHeader.uCmdLen = sizeof(MIMAX_HEADER)+response.size();

	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!SendToServer(response))
		LogError("发送设置预置位回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//设置预置位模式
bool CBocomServer::OnSetPresetMode(const int nSocket, std::string request)
{
	MIMAX_HEADER mHeader;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uCmdFlag == 0x00000000)//自动模式
	{
		g_nPreSetMode = 0;
		LogNormal("HuiTong PreSetMode=%d",g_nPreSetMode);
	}
	else if(sHeader->uCmdFlag == 0x00000001)//手动模式
	{
		g_nPreSetMode = 1;
		LogNormal("HuiTong PreSetMode=%d",g_nPreSetMode);
	}
	g_ytControlSetting.nPreSetMode = g_nPreSetMode;
	CXmlParaUtil xml;
	xml.UpdateSystemSetting("YunTaiSetting", "PreSetMode");

	std::string response;

	mHeader.uCameraID = sHeader->uMsgSource;
	mHeader.uCmdID = (SET_PRESET_MODE_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER);

	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!SendToServer(response))
		LogError("发送设置预置位模式回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//发送数据
bool CBocomServer::SendMsg(const int nSocket,const string& strData)
{
    if(!m_bVerify)
    {
        return false;
    }

    if(nSocket <= 0)
    {
      return false;
    }

    string strFullMsg("");
    strFullMsg.append( (char *)strData.c_str(), strData.size() );

     //数据加密
    if(strFullMsg.size()  > sizeof(MIMAX_HEADER))
    {
        //补充四个字节用于存放crc循环校验码
        UINT32 crc32 = 0;
        strFullMsg.append( (char *)&crc32, sizeof(UINT32) );
        EncodeBuff( (char*)strFullMsg.c_str(), 0);
    }

    if(!mvSendMsgToSocket(nSocket,strFullMsg,true))
    {
        return false;
    }
    //LogNormal("uCmdID=%x,发送信息成功!",mHeader->uCmdID);
    return true;
}

//添加客户端
bool CBocomServer::AddClient(SRIP_CLEINT sClient)
{
	LogNormal("CBocomServer::AddClient port:%d, sock:%d \n", m_nPort, sClient.nSocket);
    //加锁
    pthread_mutex_lock(&m_thread_mutex);

    DelClient();
    m_nCenterSocket = sClient.nSocket;

    //解锁
    pthread_mutex_unlock(&m_thread_mutex);
	return true;
}

//删除客户端
bool CBocomServer::DelClient()
{
    if(m_nCenterSocket > 0)
    {
        //删除命令列表中的数据
        DelMsg(m_nCenterSocket);
        //关闭连接
        mvCloseSocket(m_nCenterSocket);
        m_nCenterSocket = 0;

        m_nLinker = 0;
        m_bVerify = false;

        return true;
    }

	return false;
}

//心跳检测
void CBocomServer::LinkTest()
{
    //zhangyaoyao:该函数对一把锁进行了4次获取和释放，感觉结构有些不合理，暂时按优先级把发送心跳提前处理
	/////////////////////////////发送心跳信号
	if(m_nCenterSocket > 0)
	{
        //协议头
        MIMAX_HEADER mHeader;
        mHeader.uCmdID = (PLATE_LINK);
        //消息长度
        mHeader.uCmdLen = sizeof(mHeader);
        std::string request;
        //数据
        request.append((char*)&mHeader,sizeof(mHeader));
        //printf("*************************linktest\n");

        //发送心跳信号//心跳值大于设定的阀值,断开连接
        if( (!SendToServer(request)) )
        {
            //DelClient();
            LogError("LinkTest disconnect\n");
        }
        m_nLinker++;
	}

	return;
}

//处理历史数据
void CBocomServer::mvDealHistoryRecord()
{
    if(g_nSendHistoryRecord == 1)
    {
				 //车牌记录
				StrList strListRecord;
				std::list<unsigned int> listSeq;

				if(g_nServerType == 4)
				{
					listSeq.clear();
					strListRecord.clear();
					if(g_skpDB.GetPlateHistoryRecord(strListRecord, 0, m_nId))
					{
						StrList::iterator it_b = strListRecord.begin();
						while(it_b != strListRecord.end())
						{
							string strPlate("");
							strPlate = *it_b;

							UINT32 uSeq = *(UINT32*)(strPlate.c_str()+sizeof(MIMAX_HEADER));
							RECORD_PLATE* sPlate = (RECORD_PLATE*)(strPlate.c_str()+sizeof(MIMAX_HEADER));
							int uViolationType = sPlate->uViolationType;

							string strRecord = ConvertPlate(*sPlate);
							MIMAX_HEADER Header = *((MIMAX_HEADER*)strPlate.c_str());
							Header.uCmdLen = sizeof(MIMAX_HEADER)+strRecord.size();
							strPlate.clear();
							strPlate.append((char*)&Header,sizeof(MIMAX_HEADER));
							strPlate.append((char*)strRecord.c_str(),strRecord.size());
							
							bool bSendStatus = false;
							if(uViolationType > 0)
							{
								bSendStatus = SendToFtpServer(strPlate);
							}
							else
							{
								bSendStatus = SendToServer(strPlate);
							}
							
							if(bSendStatus)
							{
								listSeq.push_back(uSeq);
								sleep(3);
							}
							it_b++;	
						}
						if(listSeq.size() > 0)
						g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, listSeq, m_nId);
					}
					else
					{
						sleep(60);
					}
				}
				else if(g_nServerType == 26)
				{		
						listSeq.clear();
						strListRecord.clear();
						if(g_skpDB.GetPlateHistoryRecord(strListRecord, 0, m_nId))//违章记录
						{
							StrList::iterator it_b = strListRecord.begin();
							while(it_b != strListRecord.end())
							{
								string strPlate("");
								strPlate = *it_b;							

								RECORD_PLATE* sPlate = (RECORD_PLATE*)(strPlate.c_str()+sizeof(MIMAX_HEADER));
								int uViolationType = sPlate->uViolationType;
								
								UINT32 uSeq = *(UINT32*)(strPlate.c_str()+sizeof(MIMAX_HEADER));

								bool bSendStatus = false;
								if(uViolationType > 0)
								{
									string strRecord = ConvertPlate(*sPlate);
									MIMAX_HEADER Header = *((MIMAX_HEADER*)strPlate.c_str());
									Header.uCmdLen = sizeof(MIMAX_HEADER)+strRecord.size();
									strPlate.clear();
									strPlate.append((char*)&Header,sizeof(MIMAX_HEADER));
									strPlate.append((char*)strRecord.c_str(),strRecord.size());

									bSendStatus = SendToServer(strPlate);
								}
								else
								{
									if(g_Kafka.uSwitchUploading == 1)
									{
										bSendStatus = SendTokafka(strPlate);
									}
									else
									{
										bSendStatus = true;
										listSeq.push_back(uSeq);
										it_b++;
										continue;
									}
								}
								
								if(bSendStatus)
								{
									listSeq.push_back(uSeq);
									sleep(5);
								}
								it_b++;
							}
							if(listSeq.size() > 0)
							g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, listSeq, m_nId);
						}
						else
						{
							sleep(60);
						}
				}
				else
				{
					listSeq.clear();
					strListRecord.clear();
					if(g_skpDB.GetPlateHistoryRecord(strListRecord, 0, m_nId))
					{
						//printf("strListRecord.size()=%d\n",strListRecord.size());
						StrList::iterator it_b = strListRecord.begin();
						while(it_b != strListRecord.end())
						{
							string strPlate("");
							strPlate = *it_b;
							//printf("before sPlate\n");
							RECORD_PLATE* sPlate = (RECORD_PLATE*)(strPlate.c_str()+sizeof(MIMAX_HEADER));
							UINT32 uSeq = *(UINT32*)(strPlate.c_str()+sizeof(MIMAX_HEADER));
							
							//printf("before ConvertPlate\n");
							string strRecord = ConvertPlate(*sPlate);
							//printf("after  ConvertPlate =%d\n",strRecord.size());
							MIMAX_HEADER Header = *((MIMAX_HEADER*)strPlate.c_str());
							Header.uCmdLen = sizeof(MIMAX_HEADER)+strRecord.size();
							strPlate.clear();
							strPlate.append((char*)&Header,sizeof(MIMAX_HEADER));
							strPlate.append((char*)strRecord.c_str(),strRecord.size());

							bool bSendStatus = false;
							bSendStatus = SendToServer(strPlate);
							if(bSendStatus)
							{
								listSeq.push_back(uSeq);
								sleep(5);
							}
							it_b++;
						}
						//printf("listSeq.size()=%d\n",listSeq.size());
						if(listSeq.size() > 0)
						g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, listSeq, m_nId);
					}
					else
					{
						sleep(60);
					}
				}
				
				//事件记录
				if(g_nServerType != 26)
				{
					listSeq.clear();
					StrList strListEvent;
					if(g_skpDB.GetEventHistoryRecord(strListEvent))//违章记录
					{
						StrList::iterator it_b = strListEvent.begin();
						while(it_b != strListEvent.end())
						{
							string strEvent("");
							strEvent = *it_b;

							UINT32 uSeq = *(UINT32*)(strEvent.c_str()+sizeof(MIMAX_HEADER));
							
							bool bSendStatus = false;
							bSendStatus = SendToServer(strEvent);
							if(bSendStatus)
							{
								listSeq.push_back(uSeq);
								sleep(5);
							}
							it_b++;
						}
						if(listSeq.size() > 0)
						g_skpDB.UpdateRecordStatus(MIMAX_EVENT_REP, listSeq);
					}
				}
	}
}

//发送到ftp服务器
bool CBocomServer::SendToFtpServer(std::string& strMsg)
{
    if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
    {
        return false;
    }

 //   if(ConnectFtpServer())
    {
        MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();

        char filepath[255]={0};
        String strLocalPath,strRemotePath,strTime,strPlace,strPath;
        if (MIMAX_EVENT_REP == sHeader->uCmdID)
        {
            BOCOM_RECORD_EVENT *pEvent = (BOCOM_RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
            strLocalPath = pEvent->chPicPath;
            strTime = GetTime(pEvent->uEventBeginTime,2);
            strPlace = pEvent->chPlace;
            g_skpDB.UTF8ToGBK(strPlace);
            strPath = GetFtpPathNameByEvent(pEvent->uCode);
            sprintf(filepath,"%s/%s#%s#%d#%s-%u.jpg",strPath.c_str(),g_ServerHost.c_str(),strPlace.c_str(),pEvent->uRoadWayID,strTime.c_str(),pEvent->uSeq);
        }
        else if (MIMAX_PLATE_REP == sHeader->uCmdID)
        {
            RECORD_PLATE_SERVER *pPlate = (RECORD_PLATE_SERVER *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
            strLocalPath = pPlate->chPicPath;
            strTime = GetTime(pPlate->uTime,2);
            strPlace = pPlate->chPlace;
            g_skpDB.UTF8ToGBK(strPlace);
            strPath = GetFtpPathNameByEvent(pPlate->uViolationType);
            sprintf(filepath,"%s/%s#%s#%d#%s-%u.jpg",strPath.c_str(),g_ServerHost.c_str(),strPlace.c_str(),pPlate->uRoadWayID,strTime.c_str(),pPlate->uSeq);
        }
        strRemotePath = filepath;

        bool bRet = false;
        string strData;
        bRet = g_FtpCommunication.DoPut((char *)strLocalPath.c_str(),(char *)strRemotePath.c_str(),strData,false);

        return bRet;
    }
    return false;
}

//接收中心端消息
void CBocomServer::mvRecvCenterServerMsg()
{
    while (!g_bEndThread)
    {
        if(m_nCenterSocket > 0)
        {
            int nBytes = 0;

            char chBuffer[SRIP_MAX_BUFFER];


            MIMAX_HEADER mHeader;
            std::string response;
            //接收HTTP 头，一次性接收
            if((nBytes = recv(m_nCenterSocket,(void*)&mHeader,sizeof(mHeader),MSG_NOSIGNAL)) < 0)
            {
                LogError("接收协议头出错，连接断开! socket = %d,%s\r\n", m_nCenterSocket, strerror(errno));
                DelClient();
                continue;
            }

            if((nBytes == 0) && (errno == 0))
            {
                continue;
            }

            //判断结构中的数据长度，小于本身结构的长度，错误
            if(mHeader.uCmdLen < sizeof(mHeader))
            {
                LogError("接收协议头结构错误，连接断开!,%s,nBytes=%d,mHeader.uCmdLen=%d,errno=%d\r\n",strerror(errno),nBytes,mHeader.uCmdLen,errno);
                DelClient();
                continue;
            }
            ///////////将博康接口转换
            SRIP_HEADER sHeader;
            sHeader.uMsgCommandID = mHeader.uCmdID;
            sHeader.uCmdFlag = mHeader.uCmdFlag;
            sHeader.uMsgSource = mHeader.uCameraID;

            //保存头
            response.append((char*)&sHeader,sizeof(sHeader));

            //接收剩下的数据
            int nLeft = mHeader.uCmdLen - sizeof(mHeader);

            while(nLeft >  0)
            {
                //接收后面的数据
                if((nBytes = recv(m_nCenterSocket,chBuffer,sizeof(chBuffer)<nLeft?sizeof(chBuffer):nLeft,MSG_NOSIGNAL)) < 0)
                {
                    LogError("接收后续消息出错，连接断开!,%s\r\n",strerror(errno));
                    DelClient();
                    break;
                }
                //保存数据
                response.append(chBuffer,nBytes);
                nLeft -= nBytes;
            }

            //将命令传送到处理模块,继续处理下一个命令
            if(response.size() == (sizeof(sHeader)+mHeader.uCmdLen - sizeof(mHeader)))
            AddMsg(m_nCenterSocket,(sHeader.uMsgCommandID),response);
        }

        usleep(1000);
    }
}



bool CBocomServer::SendDataByFtp(string strLocalFilePath,int nCameraID)
{
    if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
    {
        return false;
    }

    bool bRet = false;
	{
		string strVideoTime = g_skpDB.GetVideoTime(strLocalFilePath,m_nVideoType);

		if(strVideoTime.size() >= 17)
		{
			//LogNormal("%s\n",strLocalFilePath.c_str());

			string strDayTime = strVideoTime.substr(0, 8);
			string strMiTime = strVideoTime.substr(8, 9);

			char chRemoteDirPath[SRIP_MAX_BUFFER] = {0};
			if(g_nEncodeFormat == 1 || g_nEncodeFormat == 2)
			{
				sprintf(chRemoteDirPath, "video/%s/%s/%d/%s.mp4",strDayTime.c_str(),g_ServerHost.c_str(),nCameraID,strMiTime.c_str());
			}
			else
			{
				sprintf(chRemoteDirPath, "video/%s/%s/%d/%s.avi",strDayTime.c_str(),g_ServerHost.c_str(),nCameraID,strMiTime.c_str());
			}
			
			//LogNormal("%s\n",chRemoteDirPath);
			string strMsg = "";
			bRet = g_FtpCommunication.VideoDoPut((char *)strLocalFilePath.c_str(),chRemoteDirPath,strMsg,true,true,chRemoteDirPath,true);
		}
	}

	return bRet;
}


bool CBocomServer::SendFlvDataByFtp(string strLocalFilePath,int nCameraID)
{
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
	{
		return false;
	}

	bool bRet = false;
	{
		string strVideoTime;
		String strmp4Path = strLocalFilePath;

		strmp4Path.erase(strLocalFilePath.size() - 3,3);
		strmp4Path.append("mp4");
		strVideoTime = g_skpDB.GetVideoTime(strmp4Path,m_nVideoType);


		if(strVideoTime.size() >= 17)
		{
			string strDayTime = strVideoTime.substr(0, 8);
			string strMiTime = strVideoTime.substr(8, 9);
			char chRemoteDirPath[SRIP_MAX_BUFFER] = {0};
			sprintf(chRemoteDirPath, "video/%s/%s/%d/%s.flv",strDayTime.c_str(),g_ServerHost.c_str(),nCameraID,strMiTime.c_str());

			string strMsg = "";
			bRet = g_FtpCommunication.VideoDoPut((char *)strLocalFilePath.c_str(),chRemoteDirPath,strMsg,true,true,chRemoteDirPath,true);
		}
	}

	return bRet;
}

bool CBocomServer::SendVideoForReSend()
{
	string strVideoPath;

	bool retCBocomServer = false;
	int nVideoType = 0;
	int nCameraID = 0;
	strVideoPath = g_skpDB.GetVideoRecord(nVideoType,nCameraID);
	m_nVideoType = nVideoType;
	//LogNormal("nVideoType=%d,nCameraID=%d\n",nVideoType,nCameraID);
	int nLoop = 0;

	if(strVideoPath.size() > 0)
	{
#ifdef FLVFORMAT
		String strflvPath = strVideoPath;
		strflvPath.erase(strVideoPath.size() - 3,3);
		strflvPath.append("flv");
		CFlvConvert *p = new CFlvConvert;
		if (p)
		{
			p->StartConvertFile(strVideoPath.c_str(), strflvPath.c_str());
			delete p;
			p = NULL;
		}

		retCBocomServer =  SendFlvDataByFtp(strflvPath,nCameraID);
#else
		retCBocomServer =  SendDataByFtp(strVideoPath,nCameraID);
#endif
		if (retCBocomServer == true)
		{
			g_skpDB.UpdateVideoRecord(strVideoPath,1);
		}
	}
	return retCBocomServer;

}

//系统时间设置
bool CBocomServer::OnSysTime(const int nSocket,std::string request)
{
	if(request.size() < sizeof(SRIP_HEADER)+4)
	{
		return false;
	}

    UINT32 uTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));

    if(uTime > 0)
    {
		timeval timer;
		timer.tv_sec = uTime;
		timer.tv_usec = 0;
        if (settimeofday(&timer, NULL) == 0)
		{
			LogNormal("中心校时成功=%s\n",GetTime(uTime).c_str());
			system("hwclock --systohc");
        }
    }
    return true;
}

#ifdef KAFKA_SERVER
//初始化kafkaClient服务
bool CBocomServer::InitKafkaClient()
{
	bool bRet = false;
	if (m_kafkaManage == NULL)
	{
		m_kafkaManage = new CKafakaManage();
		bRet = m_kafkaManage->Init();
	}

	return bRet;
}
#endif


string CBocomServer::ConvertPlate(RECORD_PLATE &plate)
{
			string strPlate("");
			RECORD_PLATE_SERVER br_plate;

			br_plate.uSeq = plate.uSeq;						//序列号
			br_plate.uTime = plate.uTime;						//识别车牌时间(秒)
			br_plate.uMiTime = plate.uMiTime;					//识别车牌时间(毫秒)
			memcpy(br_plate.chText, plate.chText, sizeof(plate.chText));					//车牌文本
			br_plate.uColor = plate.uColor;					//车牌类型（颜色）

			br_plate.uCredit = plate.uCredit;					//识别可靠度
			br_plate.uRoadWayID = plate.uRoadWayID;				//车道号

			br_plate.uType = plate.uType;						//车辆类型(低16位大、中、小，高16位卡车、巴士、轿车等）

			br_plate.uSmallPicSize = plate.uSmallPicSize;				//车牌小图大小
			br_plate.uSmallPicWidth = plate.uSmallPicWidth;			//车牌小图宽度
			br_plate.uSmallPicHeight = plate.uSmallPicHeight;			//车牌小图高度

			br_plate.uPicSize = plate.uPicSize;					//车牌全景图片大小
			br_plate.uPicWidth = plate.uPicWidth;					//车牌全景图片宽度
			br_plate.uPicHeight = plate.uPicHeight;				//车牌全景图片高度

			br_plate.uPosLeft = plate.uPosLeft;					//车牌在全景图片中的位置左
			br_plate.uPosTop = plate.uPosTop;					//车牌在全景图片中的位置上
			br_plate.uPosRight = plate.uPosRight;					//车牌在全景图片中的位置右
			br_plate.uPosBottom = plate.uPosBottom;				//车牌在全景图片中的位置下

			br_plate.uCarColor1 = plate.uCarColor1;				//车身颜色
			br_plate.uSpeed = plate.uSpeed;					//车速
			br_plate.uDirection = plate.uDirection;				//行驶方向
			br_plate.uCarBrand = plate.uCarBrand;				//产商标志
			#ifdef GLOBALCARLABEL //车标
				br_plate.uCarBrand = plate.uCarBrand + plate.uDetailCarBrand;
			#else
			if(br_plate.uCarBrand >= 1000)
			{
				br_plate.uCarBrand = 200000;
			}
			#endif
			
			string strFtpVideoPath(plate.chVideoPath);
			if(strncmp(strFtpVideoPath.c_str(),"ftp://",6) == 0 )
			{
					int nPos = g_ServerHost.size()+6;
					string strVideoPath = g_strVideo;
					printf("%s\n",strFtpVideoPath.c_str());
					strFtpVideoPath.insert(nPos,strVideoPath.c_str(),strVideoPath.size());
					printf("%s\n",strFtpVideoPath.c_str());
					strFtpVideoPath.insert(6,"road:road@",10);
					printf("%s\n",strFtpVideoPath.c_str());
					printf("plate.chVideoPath=%s\n",plate.chVideoPath);
					memset(plate.chVideoPath,0,MAX_VIDEO);
					printf("plate.chVideoPath=%s\n",plate.chVideoPath);
					memcpy(plate.chVideoPath,strFtpVideoPath.c_str(),strFtpVideoPath.size());
					printf("plate.chVideoPath=%s\n",plate.chVideoPath);
			}
			memcpy(br_plate.chVideoPath, plate.chVideoPath, sizeof(plate.chVideoPath));				//录像路径
			memcpy(br_plate.chPicPath, plate.chPicPath, sizeof(plate.chPicPath));				//大图片路径

			br_plate.uCarColor2 = plate.uCarColor2;                    //车身颜色2

			br_plate.uWeight1 = plate.uWeight1;                    //车身颜色权重1
			br_plate.uWeight2 = plate.uWeight2;                    //车身颜色权重2
			
			br_plate.uPlateType = plate.uPlateType;            //车牌结构

                //违章类型(闯红灯等)
				if(g_nServerType == 0 || g_nServerType == 26 || g_nServerType == 5)
				{
					br_plate.uCarColor1 = plate.uCarColor1+1;				//车身颜色
					br_plate.uCarColor2 = plate.uCarColor2+1;                    //车身颜色2

					if(plate.uViolationType == DETECT_RESULT_ALL) //没有违章报警
					{
						br_plate.uViolationType = 0;
					}
					else if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
					{
						br_plate.uViolationType = 1;
					}
					else if(plate.uViolationType == DETECT_RESULT_PARKING_VIOLATION)
					{
						br_plate.uViolationType = 2;
					}
					else if(plate.uViolationType == DETECT_RESULT_FORBID_LEFT)
					{
						br_plate.uViolationType = 3;
					}
					else if(plate.uViolationType == DETECT_RESULT_FORBID_RIGHT)
					{
						br_plate.uViolationType = 4;
					}
					else if(plate.uViolationType == DETECT_RESULT_FORBID_STRAIGHT)
					{
						br_plate.uViolationType = 5;
					}
					else if(plate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION)
					{
						br_plate.uViolationType = 6;
					}
					else if(plate.uViolationType == DETECT_RESULT_ELE_EVT_BIANDAO)
					{
						br_plate.uViolationType = 7;
					}
					else if(plate.uViolationType == DETECT_RESULT_NO_PASSING)
					{
						br_plate.uViolationType = 8;
					}
					else if(plate.uViolationType == DETECT_RESULT_PRESS_LINE || plate.uViolationType == DETECT_RESULT_PRESS_WHITELINE)
					{
						br_plate.uViolationType = 9;
					}
					else if(plate.uViolationType == DETECT_RESULT_CYC)
					{
						br_plate.uViolationType = 10;
					}
					else if(plate.uViolationType == DETECT_RESULT_NOT_LOCAL_CAR)
					{
						br_plate.uViolationType = 11;
					}
					else if(plate.uViolationType == DETECT_RESULT_EVENT_SHIELD)
					{
						br_plate.uViolationType = 12;
					}
					else if(plate.uViolationType == DETECT_RESULT_YELLOW_CAR)
					{
						br_plate.uViolationType = 13;
					}
					else if(plate.uViolationType == DETECT_RESULT_YELLOW_CRC)
					{
						br_plate.uViolationType = 14;
					}
					else if(plate.uViolationType ==  DETECT_RESULT_EVENT_CROSS)
					{
						br_plate.uViolationType = 15;
					}
					else if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
					{
						br_plate.uViolationType = 16;
					}
					else if(plate.uViolationType == DETECT_RESULT_EVENT_GO_SLOW)
					{
						br_plate.uViolationType = 17;
					}
					else if(plate.uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME)
					{
						br_plate.uViolationType = 18;
					}
					else if(plate.uViolationType == DETECT_RESULT_NO_PARKING)
					{
						br_plate.uViolationType = 19;
					}
					else if(plate.uViolationType == DETECT_RESULT_PRESS_LEAD_STREAM_LINE)
					{
						br_plate.uViolationType = 20;
					}
					else if(plate.uViolationType == DETECT_NO_DIRECTION_TRAVEL)
					{
						br_plate.uViolationType = 21;
					}
					else if(plate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION_FLAG)
					{
						br_plate.uViolationType = 22;
					}
					else if(plate.uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
					{
						br_plate.uViolationType = 23;
					}
					else if(plate.uViolationType == DETECT_RESULT_CROSSLINE_STOP)
					{
						br_plate.uViolationType = 24;
					}
					else if(plate.uViolationType == DETECT_RESULT_GASSER)
					{
						br_plate.uViolationType = 25;
					}
					else if(plate.uViolationType == DETECT_RESULT_TAKE_UP_NONMOTORWAY)
					{
						br_plate.uViolationType = 26;
					}
					else
					{
						br_plate.uViolationType = 1000;
					}
				}
				else
				{
					br_plate.uViolationType = plate.uViolationType;
				}

			if(plate.uTypeDetail==TRUCK_TYPE)//车型细分
            {
                br_plate.uDetailCarType = 1;
            }
            else if(plate.uTypeDetail==BUS_TYPE)
            {
                br_plate.uDetailCarType = 2;
            }
            else if(plate.uTypeDetail==MIDDLEBUS_TYPE)
            {
                br_plate.uDetailCarType = 3;
            }
            else if(plate.uTypeDetail==TAXI)
            {
                br_plate.uDetailCarType = 4;
            }
            else if(plate.uTypeDetail==TWO_WHEEL_TYPE)//两轮车
            {
                br_plate.uDetailCarType = 5;
            }
            else
            {
                br_plate.uDetailCarType = 6;
            }

		br_plate.uSeqID = plate.uSeqID;                      //帧序号
		
		br_plate.uTime2 = plate.uTime2;						//第二车牌时间(秒)
		br_plate.uMiTime2 = plate.uMiTime2;					//第二车牌时间(毫秒)

		br_plate.uRedLightBeginTime = plate.uRedLightBeginTime;
		br_plate.uRedLightBeginMiTime = plate.uRedLightBeginMiTime;

		br_plate.uRedLightEndTime = plate.uRedLightEndTime;
		br_plate.uRedLightEndMiTime = plate.uRedLightEndMiTime;

		br_plate.uLimitSpeed = plate.uLimitSpeed;
		br_plate.uOverSpeed = plate.uOverSpeed;

		 if(g_nSendImage == 1)
        {
            string strPicPath(br_plate.chPicPath);
            string strPic = GetImageByPath(strPicPath);

            br_plate.uSmallPicSize = strPic.size() - br_plate.uPicSize;
		    br_plate.uSmallPicWidth = plate.uPicWidth;
            br_plate.uSmallPicHeight = plate.uPicHeight;
			
			strPlate.append((char*)&br_plate,sizeof(RECORD_PLATE_SERVER));
            strPlate.append((char*)strPic.c_str(),strPic.size());
        }
		else
        {
	         br_plate.uPicSize = 0; 
	         br_plate.uPicWidth = 0; 
	         br_plate.uPicHeight = 0; 
             br_plate.uSmallPicSize = 0;
             br_plate.uSmallPicWidth = 0;
             br_plate.uSmallPicHeight = 0;
			 strPlate.append((char*)&br_plate,sizeof(RECORD_PLATE_SERVER));
        }
		 return  strPlate;
}

bool CBocomServer::CheckSocketStatus()
{
	LogNormal("[%s]:m_nCenterSocket = %d\n",__FUNCTION__,m_nCenterSocket);
	if (m_nCenterSocket == 0)
	{
		return false;
	}
	return true;
}
