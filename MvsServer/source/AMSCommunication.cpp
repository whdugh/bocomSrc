#include "Common.h"
#include "CommonHeader.h"
#include "AMSCommunication.h"
#include "FtpCommunication.h"

CAMSCommunication g_AMSCommunication;
/*
* 函数介绍：接收消息线程入口
* 输入参数：pArg-线程入口参数，接收套接字
* 输出参数：无
* 返回值 ：无
*/
void *RecvAMSMsgThread(void *pArg)
{
    while (!g_bEndThread)
    {
        g_AMSCommunication.mvRecvCenterServerMsg();

        usleep(100);
    }

    LogError("接收AMS消息线程退出\r\n");

    pthread_exit((void *)0);
}

//记录发送线程
void* ThreadAMSResult(void* pArg)
{
	//处理一条数据
	g_AMSCommunication.DealResult();

    pthread_exit((void *)0);
	return pArg;
}

//历史记录发送线程
void* ThreadAMSHistoryResult(void* pArg)
{
	//处理一条数据
	CAMSCommunication *pAms = (CAMSCommunication *)pArg;
	if (pAms)
	{
	//	LogTrace("ams_communicain.log","begin to deal one record.");
		pAms->DealHistoryResult();
	}

    pthread_exit((void *)0);
	return pArg;
}

CAMSCommunication::CAMSCommunication()
{
    m_uRecvCSMsgThreadId = 0;
    m_bCenterLink = false;
    m_nCenterSocket = 0;
    m_nCSLinkCount = 0;
    m_nThreadId = 0;
    m_nHistoryThreadId = 0;

    pthread_mutex_init(&m_mutexMsg, NULL);
    pthread_mutex_init(&m_mutexVodFile, NULL);
    pthread_mutex_init(&m_Result_Mutex,NULL);
}


CAMSCommunication::~CAMSCommunication()
{
    if (!m_mapCSMsg.empty())
    {
        m_mapCSMsg.clear();
    }

    if (!m_mapVodFile.empty())
    {
        m_mapVodFile.clear();
    }

    pthread_mutex_destroy(&m_mutexMsg);
    pthread_mutex_destroy(&m_mutexVodFile);
    pthread_mutex_destroy(&m_Result_Mutex);
}

//启动侦听服务
bool CAMSCommunication::Init()
{
	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    //启动检测结果发送线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadAMSResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	if ( 1 == g_nServerType)
	{
		nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadAMSHistoryResult,this);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
			g_bEndThread = true;
			return false;
		}
	}

	pthread_attr_destroy(&attr);

	return true;
}

//释放
bool CAMSCommunication::UnInit()
{
    //停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}
	
	if ( 1 == g_nServerType)
	{
		if(m_nHistoryThreadId != 0)
		{
			pthread_join(m_nHistoryThreadId,NULL);
			m_nHistoryThreadId = 0;
		}
	}
	
	g_FtpCommunication.DoClose();
    m_ResultList.clear();
	return true;
}

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void CAMSCommunication::mvConnOrLinkTest(bool isInit)
{
    if (!g_bEndThread)
    {
        if (!m_bCenterLink)
        {
                if (mvConnCSAndRecvMsg())
                {
                    LogNormal("连接应用管理服务器成功!\n");
                    m_bCenterLink = true;
                    m_nCSLinkCount = 0;

					/*if (isInit)//手工调度
						mvSendLoadSettingReq();
					else*/
						mvSendChannelInfo();

			#ifdef MVSBAK
					//发送MVS通道列表到AMS服务器
					mvSendChannelListXml();
			#endif
                }
        }
        else
        {
            if (!mvSendLinkTest())
            {
                LogError("发送心跳包失败\n");
                return;
            }

            /*if (m_nCSLinkCount++ > SRIP_LINK_MAX)
            {
                mvCloseSocket(m_nCenterSocket);
                m_bCenterLink = false;
                m_nCSLinkCount = 0;
                LogError("长时间未收到心跳包，连接断开\n");
                return;
            }*/
        }
    }
}

/*
* 函数介绍：连接到中心并开启接收消息线程
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvConnCSAndRecvMsg()
{
    //connect to center server;
    if (!mvConnectToCS())
    {
		//LogNormal("通信接口异常");
        return false;
    }

    //start to receive cs's msgs;
    if (!mvStartRecvThread())
    {
        return false;
    }

    return true;
}

/*
* 函数介绍：连接到中心
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvConnectToCS()
{
	if(g_nServerType == 1)
	{
		//connect to center server and set socket's option.
		if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
		{
			//printf("\n中心数据服务器连接参数异常:host=%s,port=%d\n", g_strControlServerHost.c_str(), g_nControlServerPort);
			return false;
		}

		if (!mvPrepareSocket(m_nCenterSocket))
		{
			//printf("\n准备连接中心数据服务器套接字失败!\n");
			return false;
		}

		if (!mvWaitConnect(m_nCenterSocket, g_strControlServerHost, g_nControlServerPort,2))
		{
			//printf("\n尝试连接中心数据服务器失败!\n");
			return false;
		}

	}
	else
	{
		//connect to center server and set socket's option.
		string strAmsHost(g_AmsHostInfo.chAmsHost);
		if (g_AmsHostInfo.uHasAmsHost <= 0 || strAmsHost.empty() ||strAmsHost == "0.0.0.0" || g_AmsHostInfo.uAmsPort <= 0)
		{
			printf("\nAMS 中心数据服务器连接参数异常:host=%s,port=%d\n", g_strControlServerHost.c_str(), g_nControlServerPort);
			return false;
		}

		if (!mvPrepareSocket(m_nCenterSocket))
		{
			printf("\nAMS 准备连接中心数据服务器套接字失败!\n");
			return false;
		}

		if (!mvWaitConnect(m_nCenterSocket, strAmsHost, g_AmsHostInfo.uAmsPort,2))
		{
			printf("\nAMS 尝试连接中心数据服务器失败!\n");
			return false;
		}
	}
    
    return true;
}

/*
* 函数介绍：接收消息线程入口
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvStartRecvThread()
{
    pthread_attr_t   attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    mvJoinThread(m_uRecvCSMsgThreadId);
    if (pthread_create(&m_uRecvCSMsgThreadId, &attr, RecvAMSMsgThread, NULL) != 0)
    {
        pthread_attr_destroy(&attr);
        return false;
    }

    pthread_attr_destroy(&attr);
    return true;
}

//接收中心端消息
bool CAMSCommunication::mvRecvCenterServerMsg()
{
        string strMsg("");
        //receive msg and push it into the msg queue.
        if (mvRecvMsg(m_nCenterSocket, strMsg))
        {
            mvPushOneMsg(strMsg);
            return true;
        }
        else
        {
            return false;
        }
}

/*
* 函数介绍：压入一条消息
* 输入参数：strMsg-要压入的消息
* 输出参数：无
* 返回值 ：无
*/
void CAMSCommunication::mvPushOneMsg(string strMsg)
{
    if (strMsg.size()<sizeof(MIMAX_HEADER))
    {
        return ;
    }

    MIMAX_HEADER mHeader;
    memcpy(&mHeader,(char*)strMsg.c_str(),sizeof(MIMAX_HEADER));
	printf("<<<<<<<<<<uCameraID =0x%08x,uCmdID = 0x%08x,strMsg.size = %d, uFlag = 0x%08x \n",mHeader.uCameraID,mHeader.uCmdID,strMsg.size(),mHeader.uCmdFlag);

    if(mHeader.uCmdID == AMS_LINK_TEST) //心跳消息
    {
        m_nCSLinkCount = 0;
        return;
    }
    else if(mHeader.uCmdID == AMS_PLATE_INFO_REP) //车牌回复
    {
        m_nCSLinkCount = 0;
        return;
    }
    else if(mHeader.uCmdID == AMS_EVENT_INFO_REP) //事件回复
    {
        m_nCSLinkCount = 0;
        return;
    }
    else if(mHeader.uCmdID == AMS_STATISTIC_INFO_REP) //统计回复
    {
        m_nCSLinkCount = 0;
        return;
    }
    else if(mHeader.uCmdID == AMS_LOG_INFO_REP) //日志回复
    {
        m_nCSLinkCount = 0;
        return;
    }
	else if(mHeader.uCmdID == DETECT_SETTING_RESPONSE_INFO) //从AMS获取检测配置的响应
	{
		//cerr<<"从AMS获取检测配置的响应!"<<endl;
		m_nCSLinkCount = 0;

		//解析并保存xml文件
		CXmlParaUtil xmlUtil;
		string xmlRespones(strMsg.c_str()+sizeof(MIMAX_HEADER));
		xmlUtil.ParseAndUpdateSetting(xmlRespones);
		return;
	}
	else if(mHeader.uCmdID == DETECT_REQUEST_INFO) //通道检测设置(由AMS发往检测器)
	{
		//cerr<<"从AMS收到通道检测类型设置请求!"<<endl;
		m_nCSLinkCount = 0;
		CXmlParaUtil xmlUtil;
		string xmlRespones(strMsg.c_str()+sizeof(MIMAX_HEADER));
		int detectKind = xmlUtil.ParseAndSetDetectKind(xmlRespones);
		bool ret = false;
		if (detectKind > -1)
		{
			ret = g_skpDB.UpdateDetectKind(detectKind);
			//cerr<<"更新DetectKind成功; detectKind="<<detectKind<<endl;
		}
		//回复数据报文
		if( !mvRebMsgAndSend(m_nCenterSocket, 0, DETECT_RESPONSE_INFO, "", ret) )
		{
			LogError("向AMS发送消息失败\n");
		}
		//cerr<<"OK"<<endl;
		return;
	}
	else if (mHeader.uCmdID == SYSTEM_REQUEST_INFO) //系统设置(由AMS发往检测器)
	{
		//cerr<<"从AMS收到系统设置请求!"<<endl;
		m_nCSLinkCount = 0;
		int ret = false;
		CXmlParaUtil xmlUtil;
		string xmlRespones(strMsg.c_str()+sizeof(MIMAX_HEADER));
		ret = xmlUtil.ParseAndSetSystemSetting(xmlRespones);
		//回复消息
		if( !mvRebMsgAndSend(m_nCenterSocket, 0, SYSTEM_RESPONSE_INFO, "", ret) )
		{
			LogError("向AMS发送消息失败\n");
		}
		//cerr<<"OK"<<endl;
		return;
	}
	else if(mHeader.uCmdID == AMS_SWITCH_CAMERA)//视频切换请求
	{
		LogNormal("mHeader.uCmdID=%x\n",mHeader.uCmdID);

		m_nCSLinkCount = 0;
		CXmlParaUtil xmlUtil;
		string xmlRespones(strMsg.c_str()+sizeof(MIMAX_HEADER));

		xmlUtil.ParseSwitchCamera(xmlRespones);

		if( !mvRebMsgAndSend(m_nCenterSocket, 0, AMS_SWITCH_CAMERA_REP, "") )
		{
			LogError("向AMS发送消息失败\n");
		}
		return;
	}
	else if(mHeader.uCmdID == AMS_DELETE_CHANNEL)//删除分析通道请求
	{
		LogNormal("mHeader.uCmdID=%x\n",mHeader.uCmdID);
		m_nCSLinkCount = 0;
		//通道结构
		SRIP_CHANNEL sChannel;

		XMLNode xml, TepNode;
		xml = XMLNode::parseString(strMsg.c_str()+sizeof(MIMAX_HEADER));
		if(!xml.isEmpty())
		{
			XMLCSTR strText;

			TepNode = xml.getChildNode("CameraID");
			if(!TepNode.isEmpty())
			{
				strText = TepNode.getText();
				if(strText)
				{
					sChannel.nCameraId = xmltoi(strText);
				}
			}

			TepNode = xml.getChildNode("PreSetID");
			if(!TepNode.isEmpty())
			{
				strText = TepNode.getText();
				if(strText)
				{
					sChannel.nPreSet = xmltoi(strText);
				}
			}
		}

		//通道ID
		sChannel.uId = g_skpDB.GetChannelIDByCameraID(sChannel.nCameraId);
		//g_skpDB.DelChannel(sChannel, uCameraId);

		g_skpDB.DelChan(sChannel);

		string strRecord("");
		char buf[64] = {0};
		XMLNode ChannelDeleteNode,TempNode;
		ChannelDeleteNode = XMLNode::createXMLTopNode("ChannelDelete");

		TempNode = ChannelDeleteNode.addChild("ChannelID");
		sprintf(buf,"%d",sChannel.uId);
		TempNode.addText(buf);

		TempNode = ChannelDeleteNode.addChild("CameraID");
		sprintf(buf,"%d",sChannel.nCameraId);
		TempNode.addText(buf);

		TempNode = ChannelDeleteNode.addChild("PreSetID");
		sprintf(buf,"%d",sChannel.nPreSet);
		TempNode.addText(buf);

		int nSize;
		XMLSTR strData = ChannelDeleteNode.createXMLString(1, &nSize);
		if(strData)
		{
			strRecord.append(strData, sizeof(XMLCHAR)*nSize);

			freeXMLString(strData);
		}
		printf("%s \n",strRecord.c_str());
		if( !mvRebMsgAndSend(m_nCenterSocket, 0, AMS_DELETE_CHANNEL_REP, strRecord) )
		{
			LogError("向AMS发送消息失败\n");
		}
		return;
	}
#ifdef MVSBAK
	else if(mHeader.uCmdID == AMS_BAK_START_DSP)//AMS通知备份MVS接管DSP
	{
		LogNormal("mHeader.uCmdID=%x\n",mHeader.uCmdID);
		m_nCSLinkCount = 0;


		printf("--###\t-111-strMsg= \n %s strMsg.size=%d--\n", strMsg.c_str()+sizeof(MIMAX_HEADER), strMsg.size());

		CHANNEL_INFO_LIST chan_info_list;
		mvDePachXmlToChannelList(strMsg, chan_info_list);

		if(chan_info_list.size() > 0)
		{
			//建通道
			mvStartBakDsp(chan_info_list);
		}

		return;
	}
#endif
#ifdef MVSBAK
	else if(mHeader.uCmdID == AMS_BAK_STOP_DSP)//AMS通知备份MVS停止接管DSP
	{
		LogNormal("mHeader.uCmdID=%x\n",mHeader.uCmdID);
		m_nCSLinkCount = 0;

		printf("--###\t-222-strMsg= \n %s strMsg.size=%d--\n", strMsg.c_str()+sizeof(MIMAX_HEADER), strMsg.size());

		CHANNEL_INFO_LIST chan_info_list;
		mvDePachXmlToChannelList(strMsg, chan_info_list);

		if(chan_info_list.size() > 0)
		{
			//删除通道
			mvStopBakDsp(chan_info_list);
		}

		return;
	}
#endif
	else
	{}

    pthread_mutex_lock(&m_mutexMsg); //push msg into queue.

    m_mapCSMsg.insert(make_pair(mHeader.uCmdID, strMsg));

    pthread_mutex_unlock(&m_mutexMsg);
}

/*
* 函数介绍：弹出一条消息
* 输入参数：strCode-要弹出的消息类型变量；strMsg-要弹出的消息变量
* 输出参数：strCode-弹出的消息类型；strMsg-弹出的消息
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvPopOneMsg(UINT32& uCmdID, string &strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }

    pthread_mutex_lock(&m_mutexMsg);

    CS_MSG_MAP::iterator iter = m_mapCSMsg.begin();
    if (iter != m_mapCSMsg.end())
    {
        uCmdID = iter->first;
        strMsg = iter->second;
        m_mapCSMsg.erase(iter);
    }

    pthread_mutex_unlock(&m_mutexMsg);

    return (!strMsg.empty());
}

/*
* 函数介绍：处理一条消息
* 输入参数：pCode-要处理的消息类型；strMsg-要处理的消息
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvOnDealOneMsg(UINT32 uCmdID, const string &strMsg)
{
    //deal one message.
    if (uCmdID == AMS_VIDEOTIME_INFO) //历史视频分析时间查询
    {
        return mvSendVideoTimeRep(strMsg);
    }
    else if (uCmdID == AMS_VOD_REQUEST_REP) //历史视频分析请求回复
    {
        return mvVodRequestRep(strMsg);
    }
    else if (uCmdID == AMS_VOD_FINISH_REP) //历史视频分析完成报告回复
    {
        return true;
    }
	else
    {
        return false;
    }
}

/*
* 函数介绍：接收消息
* 输入参数：nSocket-要接收消息的套接字；strMsg-将接收到的消息内容
* 输出参数：strMsg-接收到的消息内容
* 返回值 ：成功返回true，否则false
*/
bool CAMSCommunication::mvRecvMsg(int nSocket, string& strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }
    if (nSocket <= 0)
    {
        return false;
    }

    int nLeft = 0;
    int nBytes = 0;
    MIMAX_HEADER mHeader;    //mHeader.

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
    nLeft = mHeader.uCmdLen - sizeof(mHeader);

    while(nLeft >  0)
    {
		nBytes = recv(nSocket, chBuffer, nLeft<SRIP_MAX_BUFFER?nLeft:SRIP_MAX_BUFFER, MSG_NOSIGNAL);
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
* 函数介绍：发送心跳测试
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvSendLinkTest()
{
	//printf("==== CAMSCommunication::mvSendLinkTest  Start !  g_nSwitchCamera = %d \n",g_nSwitchCamera);
    UINT32 uDeviceID = atol(g_strDetectorID.c_str());
	string strMsg("");

	//if(g_nSwitchCamera == 1) //需要切换相机
	{
		char buf[64] = {0};
		XMLNode StatusInfoNode,ChannelInfoNode,ChannelNode,TempNode;
		StatusInfoNode = XMLNode::createXMLTopNode("StatusInfo");

		TempNode = StatusInfoNode.addChild("CpuCore");
		int num_cpu = GetCpuCore();
		sprintf(buf,"%d",num_cpu);
		TempNode.addText(buf);
		//printf("===============CpuCore = %s \n",buf);

		//TempNode = StatusInfoNode.addChild("CpuInfo");
		//sprintf(buf,"%d",(int)g_sysInfo.fCpu);
		//TempNode.addText(buf);

		//TempNode = StatusInfoNode.addChild("MemInfo");
		//sprintf(buf,"%d",(int)g_sysInfo.fMemory);
		//TempNode.addText(buf);

		//TempNode = StatusInfoNode.addChild("DiskInfo");
		//sprintf(buf,"%d",(int)g_sysInfo.fDisk);
		//TempNode.addText(buf);

		ChannelInfoNode = StatusInfoNode.addChild("ChannelInfo");


		ChannelInfoList chan_info_list;
		g_skpChannelCenter.GetChannelInfo(chan_info_list);

		ChannelInfoList::iterator it = chan_info_list.begin();
		while(it != chan_info_list.end())
		{
			CHANNEL_INFO_RECORD chan_info;
			chan_info = *it;

			ChannelNode = ChannelInfoNode.addChild("Channel");

			TempNode = ChannelNode.addChild("CameraID");
			sprintf(buf,"%d",chan_info.uCameraID);
			//printf("===============CameraID = %s \n",buf);
			TempNode.addText(buf);

			TempNode = ChannelNode.addChild("PreSetID");
			sprintf(buf,"%d",g_skpDB.GetPreSet(chan_info.uChannelID));
			TempNode.addText(buf);

			TempNode = ChannelNode.addChild("ChannelID");
			sprintf(buf,"%d",chan_info.uChannelID);
			TempNode.addText(buf);
			//printf("===============ChannelID = %s \n",buf);
			
			TempNode = ChannelNode.addChild("ChannelStatus");
			sprintf(buf,"%d",chan_info.uWorkStatus);
			TempNode.addText(buf);
			//printf("===============ChannelStatus = %s \n",buf);
			
			chan_info.DetectorType = g_skpChannelCenter.GetChannelDetectKind(chan_info.uChannelID);
			TempNode = ChannelNode.addChild("DetectorType");
			sprintf(buf,"%d",chan_info.DetectorType);
			TempNode.addText(buf);
			//printf("===============DetectorType = %s \n",buf);

			//TempNode = ChannelNode.addChild("VideoType");
			//sprintf(buf,"%d",chan_info.uRealTime);
			//TempNode.addText(buf);

			it++;
		}
	    
		int nSize;
		XMLSTR strData = StatusInfoNode.createXMLString(1, &nSize);
		if(strData)
		{
			strMsg.append(strData, sizeof(XMLCHAR)*nSize);

			freeXMLString(strData);
		}
	}
	
    return mvRebMsgAndSend(m_nCenterSocket,uDeviceID,AMS_LINK_TEST,strMsg);
}

/*
* 函数介绍：发送通道情况报告
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvSendChannelInfo(CHANNEL_INFO_RECORD& chan_info)
{
    if(!m_bCenterLink)
    {
        return false;
    }

    chan_info.uDeviceID = atol(g_strDetectorID.c_str());
    string strNewMsg("");
    strNewMsg.append((char*)&chan_info,sizeof(CHANNEL_INFO_RECORD));
    return mvRebMsgAndSend(m_nCenterSocket,chan_info.uCameraID,AMS_CHANNEL_INFO, strNewMsg);
}

//发送通道情况
void CAMSCommunication::mvSendChannelInfo()
{
    ChannelInfoList chan_info_list;
    g_skpChannelCenter.GetChannelInfo(chan_info_list);

    ChannelInfoList::iterator it = chan_info_list.begin();
    while(it != chan_info_list.end())
    {
        CHANNEL_INFO_RECORD chan_info;
        chan_info = *it;
        mvSendChannelInfo(chan_info);
        it++;
    }
}

/*
* 函数介绍：发送历史视频分析时间回复
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvSendVideoTimeRep(const string &strMsg)
{
    MIMAX_HEADER mHeader;
    memcpy(&mHeader,(char*)strMsg.c_str(),sizeof(MIMAX_HEADER));

    string strNewMsg("");
    HISTORY_VIDEO_TIME_INFO info;
    info.uCameraID = mHeader.uCameraID;
    //
    strNewMsg.append((char*)&info,sizeof(HISTORY_VIDEO_TIME_INFO));

    return mvRebMsgAndSend(m_nCenterSocket,mHeader.uCameraID,AMS_VIDEOTIME_INFO_REP, strNewMsg);
}

/*
* 函数介绍：历史视频分析请求
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvSendVodRequest(UINT32 uCameraID)
{
    if(!m_bCenterLink)
    {
        return false;
    }

    return mvRebMsgAndSend(m_nCenterSocket,uCameraID,AMS_VOD_REQUEST, string(""));
}

/*
* 函数介绍：历史视频分析请求回复
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvVodRequestRep(const string &strMsg)
{
    if(strMsg.size() < sizeof(VOD_FILE_INFO)+sizeof(MIMAX_HEADER))
    {
        return false;
    }

    VOD_FILE_INFO vod_info;
    //
    memcpy(&vod_info,(char*)strMsg.c_str()+sizeof(MIMAX_HEADER),strMsg.size()-sizeof(MIMAX_HEADER));

    SetRemoteFile(vod_info.uCameraID,vod_info);

    return true;
}

/*
* 函数介绍：历史视频分析完成报告
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvSendVodFinish(UINT32 uCameraID,const string strFilePath)
{
    if(!m_bCenterLink)
    {
        return false;
    }

    return mvRebMsgAndSend(m_nCenterSocket,uCameraID,AMS_VOD_FINISH, strFilePath);
}

/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvSendRecordToCS(const string &strMsg)
{
    if(!m_bCenterLink)
    {
        return false;
    }

    MIMAX_HEADER* mHeader = (MIMAX_HEADER *)strMsg.c_str();

    UINT32 uCmdID,uCameraID;
    std::string strNewMsg("");

    if(MIMAX_EVENT_REP == mHeader->uCmdID)
    {
        RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
        uCmdID = AMS_EVENT_INFO;
        uCameraID = mHeader->uCameraID;

        string strRemotePath;
        if(g_strFtpServerHost.size() > 8 && !SendDataByFtp(strMsg,strRemotePath))
		{
			return false;
		}
     //   memset(pEvent->chPicPath,0,sizeof(pEvent->chPicPath));
     //   memcpy(pEvent->chPicPath,strRemotePath.c_str(),strRemotePath.size());

        strNewMsg.append((char*)strMsg.c_str()+sizeof(MIMAX_HEADER),sizeof(RECORD_EVENT));

    }
    else if (MIMAX_PLATE_REP == mHeader->uCmdID)
    {
        RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
        uCmdID = AMS_PLATE_INFO;
        uCameraID = mHeader->uCameraID;

        string strRemotePath;
        if(g_strFtpServerHost.size() > 8 && !SendDataByFtp(strMsg,strRemotePath))
		{
			return false;
		}
        //memset(pPlate->chPicPath,0,sizeof(pPlate->chPicPath));
        //memcpy(pPlate->chPicPath,strRemotePath.c_str(),strRemotePath.size());

        strNewMsg.append((char*)strMsg.c_str()+sizeof(MIMAX_HEADER),sizeof(RECORD_PLATE));
    }
    else if(MIMAX_STATISTIC_REP == mHeader->uCmdID)
    {
        RECORD_STATISTIC *pStatistic = (RECORD_STATISTIC *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
        uCmdID = AMS_STATISTIC_INFO;
        uCameraID = mHeader->uCameraID;
        strNewMsg.append((char*)strMsg.c_str()+sizeof(MIMAX_HEADER),sizeof(RECORD_STATISTIC));
    }
    else if(PLATE_LOG_REP == mHeader->uCmdID || EVENT_LOG_REP == mHeader->uCmdID)
    {
        RECORD_LOG *pLog = (RECORD_LOG *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
        uCmdID = AMS_LOG_INFO;
        uCameraID = mHeader->uCameraID;
        strNewMsg.append((char*)strMsg.c_str()+sizeof(MIMAX_HEADER),sizeof(RECORD_LOG));
    }
	else if(AMS_FEATURE_CALIBRATION == mHeader->uCmdID)
	{
		uCmdID = AMS_FEATURE_CALIBRATION;
		uCameraID = mHeader->uCameraID;
		strNewMsg.append((char*)strMsg.c_str()+sizeof(MIMAX_HEADER),strMsg.size()-sizeof(MIMAX_HEADER));
	}

    return mvRebMsgAndSend(m_nCenterSocket,uCameraID,uCmdID, strNewMsg);
}

/*
* 函数介绍：重组消息并发送
* 输入参数：pCode-要发送的消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::mvRebMsgAndSend(int& nSocket,UINT32 uCameraID,UINT32 uCode, const string &strMsg, UINT32 uFlag)
{
	printf("====nSocket = %d,uCameraID =0x%08x,uCode = 0x%08x,strMsg.size = %d, uFlag = 0x%08x \n",nSocket,uCameraID,uCode,strMsg.size(),uFlag);
    //rebuild the message according to the msg-code and then send it to the center server.
    string strFullMsg("");

    MIMAX_HEADER mHeader;
    mHeader.uCameraID = uCameraID;
    mHeader.uCmdID = uCode;
    mHeader.uCmdLen = sizeof(MIMAX_HEADER)+strMsg.size();
	mHeader.uCmdFlag = uFlag;

    strFullMsg.append((char*)&mHeader,sizeof(MIMAX_HEADER));

    if (!strMsg.empty())
    {
        strFullMsg += strMsg;

        //crc校验
        UINT32 crc32 = 0;
        strFullMsg.append( (char *)&crc32, sizeof(UINT32) );
        EncodeBuff( (char*)strFullMsg.c_str(), 0);
    }

    if (!mvSendMsgToSocket(nSocket, strFullMsg))
    {
        mvCloseSocket(nSocket);
        m_bCenterLink = false;
        LogError("发送消息失败，连接断开\n");
        return false;
    }

    return true;
}

/*
* 函数介绍：发送检测参数到ftp服务器
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::SendSettingsToCS(int nCameraID,int nPreSet,int nType)
{
    return true;
}

/*
* 函数介绍：从ftp服务器获取检测参数
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
void CAMSCommunication::GetSettingsFromCS(int nCameraID,int nPreSet)
{

}

/*
* 函数介绍：获取历史视频文件名称
* 输入参数：UINT32 uCameraID相机编号
* 输出参数：string& strRemoteFile 历史视频文件名称
* 返回值 ：成功返回true，否则返回false
*/
bool CAMSCommunication::GetRemoteFile(UINT32 uCameraID,VOD_FILE_INFO& strRemoteFile)
{
    pthread_mutex_lock(&m_mutexVodFile);

    VOD_FILE_MAP::iterator it = m_mapVodFile.find(uCameraID);

    if(it != m_mapVodFile.end())
    {
        strRemoteFile = it->second;
        pthread_mutex_unlock(&m_mutexVodFile);
        return true;
    }

    pthread_mutex_unlock(&m_mutexVodFile);
    return false;
}

/*
* 函数介绍：设置历史视频文件名称
* 输入参数：UINT32 uCameraID相机编号,string strRemoteFile 历史视频文件名称
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
void CAMSCommunication::SetRemoteFile(UINT32 uCameraID,VOD_FILE_INFO strRemoteFile)
{
    pthread_mutex_lock(&m_mutexVodFile);

    VOD_FILE_MAP::iterator it = m_mapVodFile.find(uCameraID);
    if(it != m_mapVodFile.end())
    {
        it->second = strRemoteFile;
    }
    else
    {
        m_mapVodFile.insert(make_pair(uCameraID,strRemoteFile));
    }
    pthread_mutex_unlock(&m_mutexVodFile);
}

//连接ftp服务器
bool CAMSCommunication::ConnectFtpServer()
{
    if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
    {
        return false;
    }

    //Ftp连接套接字
    int nFtpSocket = 0;
    if (!mvPrepareSocket(nFtpSocket))
    {
        printf("\n准备连接ftp服务器套接字失败!\n");
        mvCloseSocket(nFtpSocket);
        return false;
    }

    if (!mvWaitConnect(nFtpSocket, g_strFtpServerHost,g_nFtpPort,3))
    {
        printf("ftp connect fail\n");
        LogError("\n尝试连接ftp服务器失败!\n");
        mvCloseSocket(nFtpSocket);
        return false;
    }
    mvCloseSocket(nFtpSocket);
    return true;
}

//通过ftp发送数据
bool CAMSCommunication::SendDataByFtp(const string& strMsg,string& strRemotePath,int nType)
{
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
    {
        return false;
    }

    //if(ConnectFtpServer())
    {
        string strLocalPath;

        char filepath[255]={0};

        MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();

        int nCameraID = sHeader->uCameraID;
        long uTime = 0;
        UINT32 uMiTime = 0;

        if (MIMAX_EVENT_REP == sHeader->uCmdID)
        {
            RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
            strLocalPath = pEvent->chPicPath;
            uTime = pEvent->uEventBeginTime;
            uMiTime = pEvent->uMiEventBeginTime;
        }
        else if (MIMAX_PLATE_REP == sHeader->uCmdID)
        {
            RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
            strLocalPath = pPlate->chPicPath;
            uTime = pPlate->uTime;
            uMiTime = pPlate->uMiTime;
        }

        char buf[255]={0};
        string strDayTime,strMiTime;
        memset(buf, 0, sizeof(buf));
        struct tm *newTime,timenow;
        newTime = &timenow;
        localtime_r( &uTime,newTime );
        sprintf(buf, "%4d%02d%02d", newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday);
        strDayTime = buf;

        sprintf(buf, "%02d%02d%02d%03d", newTime->tm_hour, newTime->tm_min, newTime->tm_sec,uMiTime);
        strMiTime = buf;

        if(nType == 0)
        sprintf(filepath,"../pic/%s/%s/%d/%s.jpg",strDayTime.c_str(),g_ServerHost.c_str(),nCameraID,strMiTime.c_str());
        else
        sprintf(filepath,"../video/%s/%s/%d/%s.mp4",strDayTime.c_str(),g_ServerHost.c_str(),nCameraID,strMiTime.c_str());
        strRemotePath = filepath;

		//LogNormal("strRemotePath=%s\n",strRemotePath.c_str());
		 bool bRet = false;
        string strData("");
        bRet = g_FtpCommunication.VideoDoPut((char*)strLocalPath.c_str(),(char*)strRemotePath.c_str(),strData,true,true,(char*)strRemotePath.c_str(),true);

        /*sprintf(buf,"./shell/ams-ftp-send.sh %s %d %s %s %s %s %d %s %s", \
                g_strFtpServerHost.c_str(), g_nFtpPort, g_strFtpUserName.c_str(),g_strFtpPassWord.c_str(),strDayTime.c_str(),g_ServerHost.c_str(),nCameraID,strLocalPath.c_str(),strRemotePath.c_str());
        printf("===SendToFtpServer====buf=%s\n",buf);

        int errFlag = 0;
        errFlag = system(buf);

        if(errFlag == 0)
        {
            return true;
        }
        else
        {
            LogError("\n通过ftp发送数据失败!\n");
            return false;
        }*/
		if(bRet)
        {
            return true;
        }
        else
        {
            LogError("\n通过ftp发送数据失败!\n");
            return false;
        }
    }

    return false;
}

//添加一条数据
bool CAMSCommunication::AddResult(std::string& strResult)
{
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strResult.c_str();

	if( (g_AmsHostInfo.uHasAmsHost > 0) && (g_nServerType != 1))
	{
		//不传输检测结果
		if(sDetectHeader->uDetectType == MIMAX_EVENT_REP ||
		   sDetectHeader->uDetectType == MIMAX_PLATE_REP ||
		   sDetectHeader->uDetectType == MIMAX_STATISTIC_REP)
		{
			return false;
		}
	}

	switch(sDetectHeader->uDetectType)
	{
       case MIMAX_EVENT_REP:	//事件(送中心端)
	   case MIMAX_PLATE_REP:   //车牌
	   case MIMAX_STATISTIC_REP:  //统计
	   case PLATE_LOG_REP:   //日志
	   case EVENT_LOG_REP:
	   case AMS_FEATURE_CALIBRATION:
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
bool CAMSCommunication::OnResult(std::string& result)
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
		case AMS_FEATURE_CALIBRATION:
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
           if (mvSendRecordToCS(result))
           {
					if(mHeader.uCmdID != AMS_FEATURE_CALIBRATION)
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
					}
                    return true;
            }
    }
    return false;
}

//处理实时数据
void CAMSCommunication::DealResult()
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
			AMS_Result::iterator it = m_ResultList.begin();
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

//处理历史数据
void CAMSCommunication::DealHistoryResult()
{
	 int index = 0;
	 const short TYPE = 2;
    while(!g_bEndThread)
	{
	    if(g_nSendHistoryRecord == 1)
        {
            string strMsg;
            short sType = index%TYPE;
            if (mvGetPlateAndEventHistoryRecord(strMsg, sType))
            {
                if (mvSendRecordToCS(strMsg))
                {
                    MIMAX_HEADER* sHeader = (MIMAX_HEADER*)strMsg.c_str();
                    UINT32 uSeq =*((unsigned int*)(strMsg.c_str()+sizeof(MIMAX_HEADER)));

                    if (sType == 0)
                    {
                        g_skpDB.UpdateRecordStatus(MIMAX_EVENT_REP, uSeq);
                    }
                    else if (sType == 1)
                    {
                        g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, uSeq);
                    }
                    //LogTrace("ams_communation.log", "deal history record sucess!");
                }
            }
            index++;
            if (index >= 360000)
            {
                index = 0;
            }
        }
	    sleep(5);
	}
}
/*
 * 发送历史数据
 * sType 0:事件记录 1 车牌记录
*/
bool CAMSCommunication::mvGetPlateAndEventHistoryRecord(string &strMsg, short sType)
{
	if (!strMsg.empty())
	{
		strMsg.clear();
	}
	char buf[1024]={0};
	unsigned int uTimeStamp = GetTimeStamp()-30;//30秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
	string strTime = GetTime(uTimeStamp);

	if (0 == sType)
	{
		sprintf(buf,"Select TRAFFIC_EVENT_INFO.*, CHAN_INFO.CAMERA_ID,CHAN_INFO.CHAN_DIRECTION from TRAFFIC_EVENT_INFO, CHAN_INFO where  TRAFFIC_EVENT_INFO.CHANNEL= CHAN_INFO.CHAN_ID and STATUS = 0 and BEGIN_TIME <= '%s' ORDER BY BEGIN_TIME desc limit 1",strTime.c_str());
		string strSql(buf);
		MysqlQuery q = g_skpDB.execQuery(strSql);
	//	LogTrace("ams_communication.log", "send history record to AMS: %s", buf);
		if (!q.eof())
		{
			MIMAX_HEADER sHeader;
			sHeader.uCmdID = MIMAX_EVENT_REP;

			RECORD_EVENT event;

			event.uSeq = q.getUnIntFileds("ID");
			event.uRoadWayID = q.getIntFileds("ROAD");
			event.uCode = q.getIntFileds("KIND");
			string strTime = q.getStringFileds("BEGIN_TIME");
			event.uEventBeginTime = MakeTime(strTime);
			event.uMiEventBeginTime = q.getIntFileds("BEGIN_MITIME");
			strTime = q.getStringFileds("END_TIME");
			event.uEventEndTime = MakeTime(strTime);
			event.uMiEventEndTime = q.getIntFileds("END_MITIME");
			event.uPicSize = q.getIntFileds("PICSIZE");
			event.uPicWidth = q.getIntFileds("PICWIDTH");
			event.uPicHeight = q.getIntFileds("PICHEIGHT");
			event.uPosX = q.getIntFileds("POSX");
			event.uPosY = q.getIntFileds("POSY");

			//事件快照路径
			string strPicPath = q.getStringFileds("PICPATH");
			memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size());

			strTime = q.getStringFileds("BEGIN_VIDEO_TIME");
			event.uVideoBeginTime = MakeTime(strTime);
			event.uMiVideoBeginTime = q.getIntFileds("BEGIN_VIDEO_MITIME");
			strTime = q.getStringFileds("END_VIDEO_TIME");
			event.uVideoEndTime = MakeTime(strTime);
			event.uMiVideoEndTime = q.getIntFileds("END_VIDEO_MITIME");

			string strVideoPath = q.getStringFileds("VIDEOPATH");
			memcpy(event.chVideoPath,strVideoPath.c_str(),strVideoPath.size());

			//车身颜色，类型，速度，方向等
			event.uColor1 = q.getIntFileds("COLOR");
			event.uType = q.getIntFileds("TYPE");
			if(event.uType == 1)
			{
				event.uType = PERSON_TYPE; //行人
			}
			else
			{
				event.uType = OTHER_TYPE; //非机动车
			}

			if(event.uCode == DETECT_RESULT_EVENT_WRONG_CHAN)
            {
                event.uCode = 14;
            }
            else if(event.uCode == DETECT_RESULT_EVENT_APPEAR)
            {
                event.uCode = 10;
            }

			event.uSpeed = q.getIntFileds("SPEED");
			event.uWeight1 = q.getIntFileds("COLORWEIGHT");
			event.uColor2 = q.getIntFileds("COLORSECOND");
			event.uWeight2 = q.getIntFileds("COLORWEIGHTSECOND");
			event.uColor3 = q.getIntFileds("COLORTHIRD");
			event.uWeight3 = q.getIntFileds("COLORWEIGHTTHIRD");

			sHeader.uCameraID = q.getUnIntFileds("CAMERA_ID");
			event.uDirection = q.getUnIntFileds("DIRECTION");
			strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
			strMsg.append((char*)&event,sizeof(RECORD_EVENT));
		}
		q.finalize();
	}
	else if(1 == sType)
	{
		sprintf(buf,"Select NUMBER_PLATE_INFO.*, CHAN_INFO.CAMERA_ID,CHAN_INFO.CHAN_DIRECTION from NUMBER_PLATE_INFO, CHAN_INFO where CHAN_INFO.CHAN_ID=NUMBER_PLATE_INFO.CHANNEL and STATUS = 0 and TIME <= '%s' ORDER BY TIME desc limit 1",strTime.c_str());
		string strSql(buf);
		MysqlQuery q = g_skpDB.execQuery(strSql);
      // LogTrace("ams_communication.log", "send history record to AMS: %s", buf);
		if (!q.eof())
		{
			MIMAX_HEADER sHeader;

			RECORD_PLATE plate;

			plate.uSeq = q.getUnIntFileds("ID");
			string strCarNum = q.getStringFileds("NUMBER");
			//g_skpDB.UTF8ToGBK(strCarNum);
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
			sHeader.uCameraID = q.getUnIntFileds("CAMERA_ID");
			plate.uDirection = q.getUnIntFileds("DIRECTION");

			bool bPlateInfo = true;
			if(plate.uType >= OTHER_TYPE)
			{
				bPlateInfo = false;
			}

			if (bPlateInfo) //有牌车
			{
				if(plate.chText[0]=='*')
				{
					if((plate.chText[1]=='-'))//无牌车
                    {
                        memcpy(plate.chText,"11111111",8);
                    }
                    else//未识别出结果
                    {
                        memcpy(plate.chText,"00000000",8);
                    }
                    printf("====plate.chText=%s\n",plate.chText);
				}

				memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

				sHeader.uCmdID = MIMAX_PLATE_REP;
				strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
				strMsg.append((char*)&plate, sizeof(RECORD_PLATE));
			}
			else //行人非机动车以事件形式发送
			{
				RECORD_EVENT  event;
				event.uSeq = plate.uSeq;
				event.uEventBeginTime = plate.uTime;
				event.uEventEndTime = plate.uTime + 5;
				event.uMiEventBeginTime = plate.uMiTime;
				event.uType = plate.uType;

				if(plate.uViolationType == 0)
				{
					if(event.uType==PERSON_TYPE)
						event.uCode  = DETECT_RESULT_EVENT_PERSON_APPEAR;//行人出现
					else if(event.uType==OTHER_TYPE) //非机车
						event.uCode  =  DETECT_RESULT_EVENT_WRONG_CHAN;//非机动车出现
				}
				else
				{
					event.uCode = plate.uViolationType;
				}

				if(event.uCode == DETECT_RESULT_EVENT_WRONG_CHAN)
				{
					event.uCode = 14;
				}
				else if(event.uCode == DETECT_RESULT_EVENT_APPEAR)
				{
					event.uCode = 10;
				}

				event.uRoadWayID = plate.uRoadWayID;
				event.uPicSize = plate.uPicSize;
				event.uPicWidth = plate.uPicWidth;
				event.uPicHeight = plate.uPicHeight;
				event.uPosX = (plate.uPosLeft+plate.uPosRight)/2;
				event.uPosY = (plate.uPosTop+plate.uPosBottom)/2;
				//event.uType =  plate.uType;
				event.uColor1 = plate.uCarColor1;
				event.uSpeed = plate.uSpeed;
				event.uColor2 = plate.uCarColor2;
				event.uWeight1 = plate.uWeight1;
				event.uWeight2 = plate.uWeight2;
				event.uDirection = plate.uDirection;
				memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size());

				sHeader.uCmdID = MIMAX_EVENT_REP;
				strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
				strMsg.append((char*)&event, sizeof(RECORD_EVENT));
			}
		}
		q.finalize();
	}

	return (!strMsg.empty());
}

//从AMS控制服务器取得检测配置文件的请求
bool CAMSCommunication::mvSendLoadSettingReq()
{
	//取得相机ID
	vector<UINT32> cameraIdVec;
	SRIP_CHANNEL * sChanel;
	string channelsStr = g_skpDB.GetChannelList();
	int size = channelsStr.size()/sizeof(SRIP_CHANNEL);
	for (int i = 0; i < size; i++)
	{
		sChanel = (SRIP_CHANNEL*)( channelsStr.c_str() + i * sizeof(SRIP_CHANNEL) );
		cameraIdVec.push_back(sChanel->nCameraId);
	}

	//生成xml报文
	string xmlStr;
	CXmlParaUtil xmlUtil;
	xmlUtil.CreateRequestAMS(cameraIdVec, xmlStr);

	//发送数据报文
	if( !mvRebMsgAndSend(m_nCenterSocket, 0, DETECT_SETTING_REQUEST_INFO, xmlStr) )
	{
		LogError("向AMS发送消息失败\n");
		return false;
	}
	return true;
}

#ifdef MVSBAK
//组MVS通道列表xml包
bool CAMSCommunication::mvPachChannelListToXml(string &strMsg)
{	
	//组装xml
	CHANNEL_INFO_LIST chan_info_list;
	g_skpChannelCenter.GetAllChannelsInfo(chan_info_list);

	printf("-chan_info_list.size()=%d--\n", chan_info_list.size());

	char buf[256] = {0};
	XMLNode MvsNode, InfoNode, ChannelListNode, DspNode, TempNode;
	MvsNode = XMLNode::createXMLTopNode("MVS");

	InfoNode = MvsNode.addChild("info");
	TempNode = InfoNode.addChild("ip");
	//string strIp = "192.168.42.188";
	TempNode.addText(g_ServerHost.c_str());

	TempNode = InfoNode.addChild("type");
	sprintf(buf, "%d", g_AmsHostInfo.uBakType);
	LogNormal("-MVS g_AmsHostInfo.uBakType=%d -\n", g_AmsHostInfo.uBakType);
	TempNode.addText(buf);

	ChannelListNode = MvsNode.addChild("ChannelList");

	CHANNEL_INFO_LIST::iterator it = chan_info_list.begin();
	while(it != chan_info_list.end())
	{
		SRIP_CHANNEL chan_info;
		chan_info = *it;

		DspNode = ChannelListNode.addChild("DSP");
		TempNode = DspNode.addChild("IP");
		TempNode.addText(chan_info.chCameraHost);
		
		TempNode = DspNode.addChild("ID");
		sprintf(buf, "%d", chan_info.uId);
		TempNode.addText(buf);
		
		TempNode = DspNode.addChild("CameraType");
		sprintf(buf, "%d", chan_info.nCameraType);
		TempNode.addText(buf);

		TempNode = DspNode.addChild("CameraStatus");
		sprintf(buf, "%d", chan_info.nCameraStatus);
		TempNode.addText(buf);

		TempNode = DspNode.addChild("VideoFormat");
		sprintf(buf, "%d", chan_info.eVideoFmt);
		TempNode.addText(buf);

		TempNode = DspNode.addChild("DetectKind");
		sprintf(buf, "%d", chan_info.uDetectKind);
		TempNode.addText(buf);

		TempNode = DspNode.addChild("Run");
		sprintf(buf, "%d", chan_info.bRun);
		TempNode.addText(buf);

		TempNode = DspNode.addChild("Direction");
		sprintf(buf, "%d", chan_info.uDirection);
		TempNode.addText(buf);

		TempNode = DspNode.addChild("PlaceText");		
		TempNode.addText(chan_info.chPlace);
		LogNormal("--mvPach chPlace:%s \n", chan_info.chPlace);

		TempNode = DspNode.addChild("CapType");
		sprintf(buf, "%d", chan_info.eCapType);
		TempNode.addText(buf);
		//通道其他信息补齐

		it++;
	}

	int nSize = 0;
	XMLSTR strData = MvsNode.createXMLString(1, &nSize);
	if(strData)
	{
		strMsg.append(strData, sizeof(XMLCHAR)*nSize);
		freeXMLString(strData);
	}

	printf("--mvPachChannelListToXml-nSize:%d \n", nSize);

	return true;
}
#endif

#ifdef MVSBAK
//拆包: MVS通道列表xml
bool CAMSCommunication::mvDePachXmlToChannelList(string &strMsg, CHANNEL_INFO_LIST &chan_info_list)
{
	XMLNode MvsNode, InfoNode, ChannelListNode, DspNode, TempNode;

	MvsNode = XMLNode::parseString(strMsg.c_str() + sizeof(MIMAX_HEADER));
	if(!MvsNode.isEmpty())
	{		
		XMLCSTR strText;
		String strTemp;
		int nText;
		int nType = -1;

		InfoNode = MvsNode.getChildNode("info");		
		ChannelListNode = MvsNode.getChildNode("ChannelList");

		TempNode = InfoNode.getChildNode("ip");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				strTemp = strText;				
			}
		}
		TempNode = InfoNode.getChildNode("type");
		if(!TempNode.isEmpty())
		{
			strText = TempNode.getText();
			if(strText)
			{
				nType = xmltoi(strText);				
			}
		}
		LogNormal("-nType:%d, ip:%s--\n", nType, strTemp.c_str());

		int nDspSize = ChannelListNode.nChildNode();
		int i = 0;
		while (i < nDspSize)
		{			
			SRIP_CHANNEL chan_info;

			DspNode = ChannelListNode.getChildNode(i);//Node DSP
			TempNode = DspNode.getChildNode("IP");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					strTemp = strText;					
					memcpy(chan_info.chCameraHost, strTemp.c_str(), strTemp.size());

					printf("-chan_info.chCameraHost=%s---\n", chan_info.chCameraHost);
				}				
			}

			
			TempNode = DspNode.getChildNode("ID");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.uId = xmltoi(strText);
				}
			}

			TempNode = DspNode.getChildNode("CameraType");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.nCameraType = xmltoi(strText);
				}
			}

			TempNode = DspNode.getChildNode("CameraStatus");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.nCameraStatus = xmltoi(strText);
				}
			}

			TempNode = DspNode.getChildNode("VideoFormat");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.eVideoFmt = (VEDIO_FORMAT)(xmltoi(strText));
				}
			}

			TempNode = DspNode.getChildNode("DetectKind");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.uDetectKind = (CHANNEL_DETECT_KIND)(xmltoi(strText));
				}
			}

			TempNode = DspNode.getChildNode("Run");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.bRun = xmltoi(strText);
				}
			}

			TempNode = DspNode.getChildNode("Direction");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.uDirection = xmltoi(strText);
				}
			}

			TempNode = DspNode.getChildNode("PlaceText");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					strTemp = strText;
					memcpy(chan_info.chPlace, strTemp.c_str(), strTemp.size());
				}
			}

			TempNode = DspNode.getChildNode("CapType");
			if(!TempNode.isEmpty())
			{
				strText = TempNode.getText();
				if(strText)
				{
					chan_info.eCapType = (CAPTURE_TYPE)(xmltoi(strText));
				}
			}

			//通道其他信息补齐
			chan_info_list.push_back(chan_info);

			i++;
		}		
	}

	return true;
}
#endif

#ifdef MVSBAK
//发送MVS通道列表到AMS服务器
bool CAMSCommunication::mvSendChannelListXml()
{
	LogNormal("--mvSendChannelListXml-\n");
	string xmlStr;
	bool bPach = mvPachChannelListToXml(xmlStr);
	if(bPach)
	{
		//发送数据报文
		if( !mvRebMsgAndSend(m_nCenterSocket, 0, AMS_CHANNEL_LIST_INFO, xmlStr) )
		{
			LogError("向AMS发送消息失败\n");
			return false;
		}
	}
	
	return true;
}
#endif

#ifdef MVSBAK
//AMS通知备份MVS接管DSP
bool CAMSCommunication::mvStartBakDsp(CHANNEL_INFO_LIST &chan_info_list)
{
	LogNormal("---mvStartBakDsp--chan_info_list.size=%d ", chan_info_list.size());

	//添加通道数据
	g_skpChannelCenter.AddChannelList(chan_info_list);

	return true;
}
#endif

#ifdef MVSBAK
//AMS通知备份MVS停止接管DSP
bool CAMSCommunication::mvStopBakDsp(CHANNEL_INFO_LIST &chan_info_list)
{
	LogNormal("---mvStopBakDsp--chan_info_list.size=%d ", chan_info_list.size());

	//删除对应通道列表
	g_skpChannelCenter.DelChannelList(chan_info_list);

	return true;
}
#endif
