
#include "YiHuaLuCenter.h"

CYiHuaLuCenter g_YiHuaLuCenter;

CYiHuaLuCenter::CYiHuaLuCenter()
{
	//pthread_mutex_init(&m_Result_Mutex,NULL);
	m_nThreadId = 0;

	m_bEndThread = false;
	m_bCenterLink = false;
	m_nSendSocket = 0;
}
CYiHuaLuCenter::~CYiHuaLuCenter()
{
	UnInit();
	m_bCenterLink = false;
}
bool CYiHuaLuCenter::Init()
{
	//启动监听线程
	//pthread_t id;
	pthread_attr_t attr;
	//线程属性初始化
	pthread_attr_init(&attr);

	if (pthread_create(&m_nThreadId, &attr,ThreadSendResult, this) == 0)
	{
		pthread_attr_destroy(&attr);
		LogNormal("start thread success!\n");
		return true;
	}

	pthread_attr_destroy(&attr);
	LogNormal("start thread fail!\n");
	return false;
}
bool CYiHuaLuCenter::UnInit()
{
	if (m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_bEndThread = false;
		m_nThreadId = 0;
	}
	//g_FtpCommunication.DoClose();

	return true;
}
void CYiHuaLuCenter::mvConnOrLinkTest()
{
	if (!m_bEndThread)
	{
		if (!m_bCenterLink)
		{
			if (mvConnectToCS())
			{
				LogNormal("[%s]:connect to control server success!\n",__FUNCTION__);
				m_bCenterLink = true;
			}
			else
			{
				LogNormal("[%s]:connect to control server fail!\n",__FUNCTION__);
			}
		}
		else
		{
			LogNormal("Link normal!ip=%s,port=%d,socket=%d\n",g_nControlServerPort,
				g_strControlServerHost.c_str(),m_nSendSocket);
		}
	}
}

void * CYiHuaLuCenter::ThreadSendResult( void *pArg )
{
	CYiHuaLuCenter *pCYiHuaLuCenter = (CYiHuaLuCenter *)pArg;

	if (pCYiHuaLuCenter != NULL)
	{
		pCYiHuaLuCenter->DealStatusResult();
	}

	return NULL;
}

void CYiHuaLuCenter::DealStatusResult()
{
	//LogNormal("[%s]:start!\n",__FUNCTION__);
	unsigned int uLastTime = getTime();
	std::string strDeviceID("");		/*设备ID*/
	std::string strDetectorId("");		/*检测器ID*/
	std::string strCameraDeviceID("");	/*相机设备ID*/
	char chCameraStatus[3] = {0};		/*相机状态*/
	char chIPCStatus[3]	   = {0};		/*工控机状态*/
	char szBuff[1024] = {0};
	int iCount  = 0;	

	while(!m_bEndThread && (1 != g_nGongJiaoMode))
	{		
		std::string response;
		unsigned int uNowTime = getTime();
	    
		if ((uNowTime - uLastTime) >= TIMER_INTERAL || 0 == iCount)
		{
			if (m_nSendSocket == 0)
			{
				mvConnOrLinkTest();
			}
			
			if (23 == g_nServerType)
			{
				//判断检测器与ftp连接是否正常
				{
					bool bRet = g_FtpCommunication.DoOpen((char*)g_strFtpServerHost.c_str(),g_nFtpPort,(char*)g_strFtpUserName.c_str(),(char*)g_strFtpPassWord.c_str());
					memset(chIPCStatus,0,sizeof(chIPCStatus));
					if (bRet)
					{
						memcpy(chIPCStatus,"00",2); //00正常
					}
					else
					{
						memcpy(chIPCStatus,"02",2); //02通讯中断
					}
					g_FtpCommunication.DoClose();
				}		 
			}
			else if (26 == g_nServerType)
			{
				bool bRet = g_BocomServerManage.GetServerDeviceStatus();
				
				memset(chIPCStatus,0,sizeof(chIPCStatus));
				if (bRet)
				{
					memcpy(chIPCStatus,"00",2);	//00正常
				}
				else
				{
					memcpy(chIPCStatus,"02",2); //02通讯中断
				}
			}
			
			//上报时间
			std::string strSendTime = GetTime(uNowTime,0);

			//获取检测器编号
			//LogNormal("detectorID = %s\n",g_strDetectorID.c_str());
			strDetectorId = g_strDetectorID;
			//获取通道个数
			int iChanCount = GetChannelCount();
			std::string strCameraMsg("");
			for (int i = 1 ;i <= iChanCount;++i)
			{
				//获取相机设备编号
				strCameraDeviceID = g_skpDB.GetDeviceByChannelId(i);
				//LogNormal("strCameraDeviceID = %s\n",strCameraDeviceID.c_str());

				int iCameraState = g_skpDB.GetCameraStateByChannelId(i);
				//LogNormal("iCameraState = %d\n",iCameraState);
				memset(chCameraStatus,0,sizeof(chCameraStatus));

				if(CM_OK == iCameraState)
				{
					memcpy(chCameraStatus,"00",2); //00正常
				}
				else if(CM_BREAK == iCameraState || CM_NO_VIDEO == iCameraState)
				{
					memcpy(chCameraStatus,"07",2); //07通讯中断
				}

				char szTempBuff[1024] = {0};
				if (1 == g_Kafka.uCheckModal)  //如果是卡口
				{
					sprintf(szTempBuff,"●%s,0101%s,%s,%s",strCameraDeviceID.c_str(),chCameraStatus,strSendTime.c_str(),strSendTime.c_str());			
				}
				else if (2 == g_Kafka.uCheckModal) //电警
				{
					sprintf(szTempBuff,"●%s,0201%s,%s,%s",strCameraDeviceID.c_str(),chCameraStatus,strSendTime.c_str(),strSendTime.c_str());
				}
				strCameraMsg.append(szTempBuff);
			}

			if (1 == g_Kafka.uCheckModal)
			{
				sprintf(szBuff, "{EHL●%s,0102%s,%s,%s,%s●EHL}",
					strDetectorId.c_str(),chIPCStatus,strSendTime.c_str(),strSendTime.c_str(),strCameraMsg.c_str());
			}
			else if (2 == g_Kafka.uCheckModal)
			{
				sprintf(szBuff, "{EHL●%s,0202%s,%s,%s,%s●EHL}",
					strDetectorId.c_str(),chIPCStatus,strSendTime.c_str(),strSendTime.c_str(),strCameraMsg.c_str());
			}
		
			std::string strResult("");
			strResult.append(szBuff);
			//LogNormal("[%s]: before!strResult.size()=%d,Model=%d\n",__FUNCTION__,strResult.size(),g_Kafka.uCheckModal);
			if (OnResult(strResult))
			{
				//LogNormal("Get Device Status Success!\n");
				mvCloseSocket(m_nSendSocket);
				m_bCenterLink = false;
				//LogNormal("[%s]:finish m_nSendSocket=%d\n",__FUNCTION__,m_nSendSocket);
			}			
			uLastTime = uNowTime;
			iCount++;
		}
		//1毫秒
		usleep(1000*1);
	}
}

bool CYiHuaLuCenter::OnResult( std::string &strMsg)
{
	//发送数据
	if (mvSendRecordToCS(strMsg))
	{
		return true;
	}

	return false;
}
bool CYiHuaLuCenter::mvConnectToCS()
{
	if (g_strControlServerHost == "0.0.0.0" || g_nControlServerPort == 0)
	{
		LogNormal("connect control-server abnormal:host=%s,port=%d\n", g_strControlServerHost.c_str(), g_nControlServerPort);
		return false;
	}

	//创建套接字
    if (!mvPrepareSocket(m_nSendSocket))
    {
        LogNormal("PreareSocket create fail\n");
        return false;
    }
	//连接中心端
    if (!mvWaitConnect(m_nSendSocket, g_strControlServerHost, g_nControlServerPort,2))
	{
		LogNormal("try connect to control-server fail!\n");
		return false;
	}

	return true;

}
bool CYiHuaLuCenter::mvSendRecordToCS( const std::string &strMsg )
{
	if(!m_bCenterLink)
	{
		return false;
	}

	//LogNormal("mvSendRecordToCS strMsg.size()=%d",strMsg.size());
	return mvRebMsgAndSend(m_nSendSocket,strMsg);
}

bool CYiHuaLuCenter::mvRebMsgAndSend( int& nSocket, const std::string &strMsg )
{
	if (!mvSendMsgToSocket(m_nSendSocket, strMsg))
	{
		mvCloseSocket(m_nSendSocket);
		m_bCenterLink = false;
		LogError("send device status fail!connect break\n");
		return false;
	}
	//LogNormal("[%s]: nSocket=%d,strResult.size()=%d\n",__FUNCTION__,nSocket,strMsg.size());
	//LogNormal("[%s]:send success!\n",__FUNCTION__);
	return true;

}

int CYiHuaLuCenter::getTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}

int CYiHuaLuCenter::GetChannelCount()
{
	char szBuff[256] = {0};
	string strTableName = "CHAN_INFO";
	int uChanCount = 0;
	int i = 1;
	sprintf(szBuff, "select count(CHAN_ID) from %s;",strTableName.c_str());
	MysqlQuery count = g_skpDB.execQuery(string(szBuff));
	if (!count.eof())
	{
		uChanCount = count.getUnIntFileds(0);
	}
	count.finalize();

	//LogNormal("chan count = %d\n",uChanCount);
	return uChanCount;
}

int CYiHuaLuCenter::SendCameraStatusOfGJMode(std::string strCameraID,CameraState state)
{
	char chCameraStatus[3] = {0};					/*相机状态*/
	char szBuff[1024] = {0};

	if (1 == g_nGongJiaoMode)
	{
		if (m_nSendSocket == 0)
		{
			mvConnOrLinkTest();
		}
	}
	if (CM_BREAK == state || CM_NO_VIDEO == state)  
	{
		memcpy(chCameraStatus,"07",2);	//异常
	}
	else if (CM_OK == state)
	{
		memcpy(chCameraStatus,"00",2);
	}
	
	std::string strResult("");
	unsigned int uNowTime = getTime();
	std::string strSendTime = GetTime(uNowTime,0);/*上报时间*/

	sprintf(szBuff, "{EHL●%s,0101%s,%s,%s●EHL}",\
		strCameraID.c_str(),chCameraStatus,strSendTime.c_str(),strSendTime.c_str());

	strResult.append(szBuff);
	if (OnResult(strResult))
	{
		mvCloseSocket(m_nSendSocket);
		m_bCenterLink = false;
	}
}


