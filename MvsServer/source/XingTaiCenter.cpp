#include "RoadCarnumDetect.h"
#include "XingTaiCenter.h"
#include "ximage.h"


XingTaiCenter g_XingTaiCenter;

#define BUFF_SIZE 1024
//构造函数
XingTaiCenter::XingTaiCenter()
{

	m_nCenterSocket = 0;

	m_nCenterLink = false;

	m_nHeartBeatCount = 0;

	pthread_mutex_init(&m_Result_Mutex,NULL);
	pthread_mutex_init(&m_CenterLinkClose_Mutex,NULL);

	m_ChannelResultList.clear();

	//test
	m_nCount = 1;
	//test

}

//析构函数
XingTaiCenter::~XingTaiCenter()
{

	pthread_mutex_destroy(&m_Result_Mutex);
	pthread_mutex_destroy(&m_CenterLinkClose_Mutex);

	m_ChannelResultList.clear();

}


//连接中心端线程
void* ThreadConnectToCS(void* pArg)
{
	//取类指针
	XingTaiCenter* pCenterServer = (XingTaiCenter*)pArg;
	if(pCenterServer == NULL) return pArg;

	while (!g_bEndThread)
	{
		while (!pCenterServer->GetCenterLinkFlag())
		{
			pCenterServer->ConnectToCS();
			sleep(5);
		}
		
		usleep(1000);
	}
	
	pthread_exit((void *)0);
	return pArg;
}

//发送心跳给中心端线程
void* ThreadSendHeartBeatToCS(void* pArg)
{
	//取类指针
	XingTaiCenter* pCenterServer = (XingTaiCenter*)pArg;
	if(pCenterServer == NULL) return pArg;

	/*HeartBeatPacket nHeartBeatPacket;*/
	UINT16 nHeader = 0xAAAA;
	UINT16 nType = 1103;//包类型
	UINT32 nLength = 4;//值的长度
	UINT32 nValue = 0xAAAAAAAA;//值
	UINT16 nTail = 0x5555;//包尾

	string strHeader("");
	string strType("");
	string strLength("");
	string strValue("");
	string strTail("");

	string strData("");

	int nHeartBeatCount = 0;
	//strData.append((const char*)&nHeartBeatPacket,sizeof(HeartBeatPacket));
	strHeader.append((char*)&nHeader,sizeof(nHeader));
	strType.append((char*)&nType,sizeof(nType));
	strLength.append((char*)&nLength,sizeof(nLength));
	strValue.append((char*)&nValue,sizeof(nValue));
	strTail.append((char*)&nTail,sizeof(nTail));

	strData += strHeader;
	strData += strType;
	strData += strLength;
	strData += strValue;
	strData += strTail;

	while (!g_bEndThread)
	{
		if(pCenterServer->GetCenterLinkFlag())
		{
			sleep(3);

			if( !pCenterServer->SendMsg(pCenterServer->GetCenterSocket(),strData,0))
			{
				pCenterServer->CloseCenterSocket();
			}
		}

			//nHeartBeatCount = pCenterServer->GetHeartBeatFailureCount() + 1;

			//pCenterServer->SetHeartBeatCount(nHeartBeatCount);

			// (pCenterServer->GetHeartBeatFailureCount() == 10)

		usleep(1000);

	}

	pthread_exit((void *)0);
	return pArg;
}

//发送车牌信息图片信息给中心端线程
void* ThreadSendPlateMsgToCS(void* pArg)
{
	//取类指针
	XingTaiCenter* pCenterServer = (XingTaiCenter*)pArg;
	if(pCenterServer == NULL) return pArg;

	while (!g_bEndThread)
	{
		while (pCenterServer->GetCenterLinkFlag())
		{
			pCenterServer->DealResult();
			usleep(1000);
		}

		usleep(1000);
	}

	pthread_exit((void *)0);
	return pArg;
}

//发送历史车牌信息图片信息给中心端线程
void* ThreadSendHistoryPlateMsgToCS(void* pArg)
{
	//取类指针
	XingTaiCenter* pCenterServer = (XingTaiCenter*)pArg;
	if(pCenterServer == NULL) return pArg;
	
	string strHisRecord("");
	string strHisMsg("");
	RECORD_PLATE* nHisPlate;

	while (!g_bEndThread)
	{
		while (pCenterServer->GetCenterLinkFlag() && (g_nSendHistoryRecord == 1))
		{
			strHisRecord.clear();

			pCenterServer->GetPlateHistoryRecord(strHisRecord);

			if (strHisRecord.size() == 0)
			{
				sleep(3);
				continue;
			}
			nHisPlate = (RECORD_PLATE*)(strHisRecord.c_str());
			
			if (nHisPlate->uViolationType == DETECT_RESULT_EVENT_GO_FAST || nHisPlate->uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION || nHisPlate->uViolationType == DETECT_RESULT_RETROGRADE_MOTION \
				|| nHisPlate->uViolationType == DETECT_RESULT_PRESS_LINE || nHisPlate->uViolationType == DETECT_RESULT_FORBID_LEFT || nHisPlate->uViolationType == DETECT_RESULT_FORBID_RIGHT \
				|| nHisPlate->uViolationType == DETECT_RESULT_FORBID_STRAIGHT \
				|| nHisPlate->uViolationType == DETECT_RESULT_ALL || nHisPlate->uViolationType == DETECT_RESULT_NOCARNUM)
			{
				strHisMsg = pCenterServer->GeneratePlateAndPicMsg(nHisPlate);
				if (strHisMsg.size() <= 0)
				{
					cerr<<"1111ThreadSendHistoryPlateMsgToCS nHisPlate.chPicPath:"<<nHisPlate->chPicPath<<endl;
					sleep(3);
					continue;
				}
				if(pCenterServer->SendMsg(pCenterServer->GetCenterSocket(),strHisMsg,1))
				{
					unsigned int uSeq = *(unsigned int*)(strHisRecord.c_str());
					g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP,uSeq);
				}
				else
				{
					cerr<<"History Send strPlateAndPicMsg Error"<<endl;
					LogError("History Send strPlateAndPicMsg Error\n");
				}
			}
			else
			{
				unsigned int uSeq = *(unsigned int*)(strHisRecord.c_str());
				g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP,uSeq);
			}
			sleep(3);
		}

		usleep(1000);
	}

	pthread_exit((void *)0);
	return pArg;
}


//获取车牌历史记录
bool XingTaiCenter::GetPlateHistoryRecord(string &strRecord)
{
	if (!strRecord.empty())
	{
		strRecord.clear();
	}
	unsigned int uTimeStamp = GetTimeStamp()-180;//180秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
	unsigned int uMonSecs = 30*24*3600;
	unsigned int uBeginTimeStamp = GetTimeStamp()-uMonSecs;
	if (uBeginTimeStamp < 0)
	{
		uBeginTimeStamp = 0;
	}
	string strBeginTime = GetTime(uBeginTimeStamp);
	String strTime = GetTime(uTimeStamp);
	char buf[1024]= {0};
	sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 1",strTime.c_str(),strBeginTime.c_str());

	String strSql(buf);
	MysqlQuery q = g_skpDB.execQuery(strSql);

	if(!q.eof())
	{
		RECORD_PLATE plate;
		plate.uSeq = q.getUnIntFileds("ID");
		string strCarNum = q.getStringFileds("NUMBER");
		/*if(g_nServerType!=1 && g_nServerType!=5 && g_nServerType!=10)
			g_skpDB.UTF8ToGBK(strCarNum);*/
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
		////////////////////////////////////////////////
		plate.uColor = q.getIntFileds("COLOR");
		if(plate.uColor <= 0)
		{
			plate.uColor =  CARNUM_OTHER;
		}
		plate.uCredit = q.getIntFileds("CREDIT");
		int nChannel = q.getIntFileds("CHANNEL");
		plate.uChannelID = nChannel;
		plate.uRoadWayID = q.getIntFileds("ROAD");

		string strTime = q.getStringFileds("TIME");
		plate.uTime = MakeTime(strTime);
		plate.uMiTime = q.getIntFileds("MITIME");
		//LogNormal("plate.uMiTime=%d",plate.uMiTime);

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

		//车牌大图
		string strPicPath = q.getStringFileds("PICPATH");
		memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

		///////////////车身颜色，车辆类型，速度，方向,地点等
		plate.uCarColor1 = q.getIntFileds("CARCOLOR");
		plate.uType = q.getIntFileds("TYPE");

		plate.uSpeed = q.getIntFileds("SPEED");
		///////////////
		plate.uWeight1 = q.getIntFileds("CARCOLORWEIGHT");
		plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
		plate.uWeight2 = q.getIntFileds("CARCOLORWEIGHTSECOND");
		plate.uViolationType = q.getIntFileds("PECCANCY_KIND");//事件类型
		plate.uTypeDetail = q.getIntFileds("TYPE_DETAIL");//车型细分
		plate.uDirection = q.getIntFileds("DIRECTION");
		String strVideoPath = q.getStringFileds("VIDEOPATH");
		memcpy(plate.chVideoPath,strVideoPath.c_str(),strVideoPath.size());

		//第2车牌时间
		string strTime2 = q.getStringFileds("TIMESECOND");
		printf("strTime2.size()=%d\n",strTime2.size());
		if(strTime2.size() > 8)
			plate.uTime2 = MakeTime(strTime2);
		else
			plate.uTime2 = 0; 
		plate.uMiTime2 = q.getIntFileds("MITIMESECOND");

		strRecord.append((char*)&plate,sizeof(RECORD_PLATE));
	}
	q.finalize();

	return (!strRecord.empty());
}

//设置心跳计数器的值
void XingTaiCenter::SetHeartBeatCount(int& nHeartBeatCount)
{
	m_nHeartBeatCount = nHeartBeatCount;
}

//添加一条数据
bool XingTaiCenter::AddResult(std::string& strResult)
{
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strResult.c_str();

	switch(sDetectHeader->uDetectType)
	{
	//case MIMAX_EVENT_REP:	//事件(送中心端)
	case MIMAX_PLATE_REP:   //车牌
	//case MIMAX_STATISTIC_REP:  //统计
	//case PLATE_LOG_REP:   //日志
	//case EVENT_LOG_REP:
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
		LogError("11111未知数据[%x],取消操作!\r\n",sDetectHeader->uDetectType);
		return false;
	}
	return true;
}

//处理数据
void XingTaiCenter::DealResult()
{
	std::string response("");

	
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
		OnResult(response);
	}

	//1毫秒
	usleep(1000*1);
	
}

//处理检测结果
bool XingTaiCenter::OnResult(std::string& result)
{

	string strRecord("");
	//发送数据
	MIMAX_HEADER mHeader;
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();

	bool bSendToServer = false;
	bool bObject = false;
	bool bViolationType = false;
	string strPlateAndPicMsg("");
	bool SendFlag = false;
	//cerr<<"AAAAAAAAA"<<endl;
	switch(sDetectHeader->uDetectType)
	{
		case MIMAX_PLATE_REP:  //车牌
			{
				mHeader.uCmdID = (sDetectHeader->uDetectType);
				mHeader.uCmdFlag = sDetectHeader->uRealTime;
				mHeader.uCameraID = sDetectHeader->uChannelID;
				
				strRecord.append((char*)(result.c_str()+sizeof(SRIP_DETECT_HEADER)),result.size()-sizeof(SRIP_DETECT_HEADER));
				bSendToServer = true;
			}
			break;
		default:
			LogError("未知数据[%d],取消操作!\r\n",sDetectHeader->uChannelID);
			return false;
	}

	if(mHeader.uCmdID == MIMAX_PLATE_REP)
	{
		RECORD_PLATE* sPlate = (RECORD_PLATE*)(strRecord.c_str());
		if(g_nSendImage)
		{
			//cerr<<"JJJJJJJ"<<endl;
			if (sPlate->uViolationType == DETECT_RESULT_EVENT_GO_FAST || sPlate->uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION || sPlate->uViolationType == DETECT_RESULT_RETROGRADE_MOTION \
				|| sPlate->uViolationType == DETECT_RESULT_PRESS_LINE || sPlate->uViolationType == DETECT_RESULT_FORBID_RIGHT || sPlate->uViolationType == DETECT_RESULT_FORBID_LEFT \
				|| sPlate->uViolationType == DETECT_RESULT_FORBID_STRAIGHT \
				|| sPlate->uViolationType == DETECT_RESULT_ALL || sPlate->uViolationType == DETECT_RESULT_NOCARNUM)
			{
				SendFlag = true;
				strPlateAndPicMsg = GeneratePlateAndPicMsg(sPlate);
				if (strPlateAndPicMsg.size() <= 0)
				{
					return false;
				}
				//cerr<<"33333VioType:"<<sPlate->uViolationType<<endl;
			}

		}
	}

	//数据长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + strRecord.size();
	//添加头
	strRecord.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//cerr<<"BBBBBBBBBB"<<endl;
	//发送数据
	if(bSendToServer)
	{

		if (strPlateAndPicMsg.size() > 0)
		{
			//cerr<<"CCCCCCCCCCCCCCc"<<endl;
			if(SendMsg(m_nCenterSocket,strPlateAndPicMsg,1))
			{
				unsigned int uSeq =*((unsigned int*)(strRecord.c_str()+sizeof(MIMAX_HEADER)));
				g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
				return true;
			}
			else
			{
				cerr<<"Send strPlateAndPicMsg Error"<<endl;
				LogError("Send strPlateAndPicMsg Error\n");
			}
		}
		else//其他数据置标志位为已发送
		{
			if (!SendFlag)
			{
				//cerr<<"DDDDDDDDDDDDD"<<endl;
				unsigned int uSeq =*((unsigned int*)(strRecord.c_str()+sizeof(MIMAX_HEADER)));
				g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
			}
			
			return true;
		}
		
	}
	return false;
}


/*
* 函数介绍：接收消息
* 输入参数：nSocket-要接收消息的套接字
* 
* 返回值 ：成功返回true，否则false
*/
bool XingTaiCenter::mvRecvSocketMsg(int nSocket)
{
	if (nSocket <= 0)
	{
		return false;
	}

	UINT16 nHeader = 0;
	UINT16 nType = 0;
	UINT32 nlength = 0;
	UINT32 nValue = 0;
	UINT16 nTail = 0;
	SYSTEMTIME nSysTemTime;


	if (recv(nSocket, &nHeader, sizeof(nHeader), MSG_NOSIGNAL) <= 0)
	{
		return false;
	}

	if (nHeader  != 0xAAAA)
	{
		LogError("recv Error Header:%d\n",nHeader);
		return false;
	}

	if (recv(nSocket, &nType, sizeof(nType), MSG_NOSIGNAL) <= 0)
	{
		return false;
	}

	if (nType == 1103)//心跳包
	{
		m_nHeartBeatCount = 0;
		//cerr<<"mvRecvSocketMsg HeartBeat"<<endl;
		if (recv(nSocket, &nlength, sizeof(nlength), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg HeartBeat recv nlength error\n");
			return false;
		}
		if (recv(nSocket, &nValue, sizeof(nValue), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg HeartBeat recv nValue error\n");
			return false;
		}
		if (recv(nSocket, &nTail, sizeof(nTail), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg HeartBeat recv nTail error\n");
			return false;
		}
		//LogNormal("HeartBeat Package\n");
	}
	else if (nType == 1104)//车辆信息返回包
	{
		m_nHeartBeatCount = 0;
		//cerr<<"mvRecvSocketMsg PlateMsg Return"<<endl;
		if (recv(nSocket, &nlength, sizeof(nlength), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg mvRecvSocketMsg PlateMsg Return recv nlength error\n");
			return false;
		}
		if (recv(nSocket, &nValue, sizeof(nValue), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg mvRecvSocketMsg PlateMsg Return recv nValue error\n");
			return false;
		}
		if (recv(nSocket, &nTail, sizeof(nTail), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg mvRecvSocketMsg PlateMsg Return recv nTail error\n");
			return false;
		}
		//LogNormal("PlateMsg Return Package\n");
	}
	else if (nType == 1108)//网络校时包
	{
		m_nHeartBeatCount = 0;
		//cerr<<"mvRecvSocketMsg to make time right"<<endl;
		if (recv(nSocket, &nlength, sizeof(nlength), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg to make time right recv nlength error\n");
			return false;
		}
		if (recv(nSocket, &nSysTemTime, sizeof(nSysTemTime), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg to make time right recv SysTemTime error\n");
			return false;
		}
		if (recv(nSocket, &nTail, sizeof(nTail), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg to make time right recv nTail error\n");
			return false;
		}

		OnSysTimeSetup(nSysTemTime);
		//LogNormal("Package to modify our time\n");
	}
	else 
	{
		LogError("%d is not the Type we need\n",nType);
		return false;
	}

	return true;
}


/*
* 函数介绍：设置系统时间
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool XingTaiCenter::OnSysTimeSetup(SYSTEMTIME &nSysTemTime)
{

	//设置时间，精确到豪秒
	
	struct tm newTime;

	newTime.tm_year = nSysTemTime.wYear - 1900;
	newTime.tm_mon = nSysTemTime.wMonth -1;
	newTime.tm_mday = nSysTemTime.wDay;
	newTime.tm_hour = nSysTemTime.wHour;
	newTime.tm_min = nSysTemTime.wMinute;
	newTime.tm_sec = nSysTemTime.wSecond;

	UINT32 timeSec = mktime(&newTime);;
	timeval timer;
	timer.tv_sec = timeSec;
	timer.tv_usec = nSysTemTime.wMilliseconds* 1000;

	if (settimeofday(&timer, NULL) == 0)
	{
		char strTime[128] = {0};
		sprintf(strTime,"%u-%u-%u %u:%u:%u.%u",nSysTemTime.wYear,nSysTemTime.wMonth,nSysTemTime.wDay,nSysTemTime.wHour, \
			nSysTemTime.wMinute,nSysTemTime.wSecond,nSysTemTime.wMilliseconds);
		printf("================mvOnSysTimeSetup=%s\n",strTime);
		LogNormal("设置时间成功%s\r\n",strTime);
		system("hwclock --systohc");
		return true;
	}
	else
	{
		return false;
	}
}

void* ThreadRecvCenterMsg(void* pArg)
{
	//取类指针
	XingTaiCenter* pCenterServer = (XingTaiCenter*)pArg;
	if(pCenterServer == NULL) return pArg;
	while (!g_bEndThread)
	{
		while (pCenterServer->GetCenterLinkFlag())
		{
			pCenterServer->mvRecvSocketMsg(pCenterServer->GetCenterSocket());
		
			usleep(1000);
		}

		usleep(1000);
	}

	pthread_exit((void *)0);
	return pArg;
}



//初始化函数
bool XingTaiCenter::Init()
{
	//cerr<<"1111111sizeof(HeartBeatPacket)"<<sizeof(HeartBeatPacket)<<endl;
	cerr<<"11111sizeof(Type_Struct_Frt_Vhc_Data)"<<sizeof(Type_Struct_Frt_Vhc_Data)<<endl;
	LogNormal("11111sizeof(Type_Struct_Frt_Vhc_Data):%d\n",sizeof(Type_Struct_Frt_Vhc_Data));
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//连接中心端
	int nret=pthread_create(&id,&attr,ThreadConnectToCS,this);
	if(nret!=0)
	{
		//失败
		LogError("111Init,ThreadConnectToCS服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//接收中心端消息
	nret=pthread_create(&id,&attr,ThreadRecvCenterMsg,this);
	if(nret!=0)
	{
		//失败
		LogError("3333Init,ThreadRecvCenterMsg服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}
    
	//给中心端发送心跳
	nret=pthread_create(&id,&attr,ThreadSendHeartBeatToCS,this);
	if(nret!=0)
	{
		//失败
		LogError("2222Init,ThreadSendHeartBeatToCS服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//发送实时车牌信息给中心端
	nret=pthread_create(&id,&attr,ThreadSendPlateMsgToCS,this);
	if(nret!=0)
	{
		//失败
		LogError("3333Init,ThreadSendPlateMsgToCS服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//发送历史车牌信息给中心端
	nret=pthread_create(&id,&attr,ThreadSendHistoryPlateMsgToCS,this);
	if(nret!=0)
	{
		//失败
		LogError("3333Init,ThreadSendHistoryPlateMsgToCS服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);

	return true;

}

//释放函数
void XingTaiCenter::UnInit()
{
}

///*
//函数功能：获取各通道的限速map
//输入：无
//输出：无
//*/
//void XingTaiCenter::GetChanLimitSpeedMap()
//{
//	char szBuff[256] = {0};
//	string strTableName = "CHAN_INFO";
//	int uCount = 0;
//	int i = 1;
//	sprintf(szBuff, "select count(CHAN_ID) from %s;",strTableName.c_str());
//	MysqlQuery count = g_skpDB.execQuery(string(szBuff));
//	if (!count.eof())
//	{
//		uCount = count.getUnIntFileds(0);
//	}
//	count.finalize();
//	cerr<<"11111ChanCount:"<<uCount<<endl;
//	LogNormal("GetChanLimitSpeedMap 通道数：%d\n",uCount);
//	CXmlParaUtil xml;
//	for (i = 1; i <= uCount; i++)
//	{
//		map<UINT32,UINT32> nMaxSpeedMap;
//		nMaxSpeedMap.clear();
//		xml.GetMaxSpeed(nMaxSpeedMap,i);
//		cerr<<"11111Chan :"<<i<<"RoadCount:"<<nMaxSpeedMap.size()<<endl;
//		LogNormal("GetChanLimitSpeedMap 通道%d车道数:%d\n",i,nMaxSpeedMap.size());
//		m_uChanMaxSpeedMap.insert(mapChanMaxSpeed::value_type(i,nMaxSpeedMap));
//
//	}
//
//}


/*
函数功能：获取超速记录违法代码
输入：RECORD_PLATE类型的车牌记录
输出：该超速记录对应的违法格式代码
*/
string XingTaiCenter::GetOverSpeedCode(RECORD_PLATE& nPlate)
{
	string strCode("");
	UINT32 nSpeed = 0;
	
	nSpeed = GetMaxSpeed(nPlate.uType,nPlate.uChannelID,nPlate.uRoadWayID);

	if (nPlate.uSpeed <=  nSpeed*1.2 && nPlate.uSpeed > nSpeed)//机动车行驶超过规定时速20％以下的
	{
		strCode = "1352";
	}
	if (nPlate.uSpeed <=  nSpeed*1.5 && nPlate.uSpeed > nSpeed*1.2)//机动车行驶超过规定时速20％以上低于50%以下的
	{
		strCode = "1636";
	}
	//if (nPlate.uSpeed <=  (tmp->second)*1.5 && nPlate.uSpeed > (tmp->second))//机动车行驶超过规定时速50％以下的
	//{
	//	strCode = "1303";
	//}
	else if (nPlate.uSpeed <=  nSpeed*1.8 && nPlate.uSpeed > nSpeed*1.5)//在高速公路以外的道路上驾驶机动车超过规定时速50%以上80%以下
	{
		strCode = "17211";
	}
	else if (nPlate.uSpeed <=  nSpeed*2.0 && nPlate.uSpeed > nSpeed*1.8)//在高速公路以外的道路上驾驶机动车超过规定时速80%以上100%以下
	{
		strCode = "16034";
	}
	else if (nPlate.uSpeed > nSpeed*2.0)//在高速公路以外的道路上驾驶机动车超过规定时速100%以上
	{
		strCode = "16035";
	}

	return strCode;
}


/*
函数功能：获取车道的限速值
输入：RECORD_PLATE类型的车牌记录
输出：特定通道特定车道的限速值
*/
UINT32 XingTaiCenter::GetLimitSpeed(RECORD_PLATE& nPlate)
{
	UINT32 uLimtSpeed = 0;
	
	uLimtSpeed = GetMaxSpeed(nPlate.uType,nPlate.uChannelID,nPlate.uRoadWayID);

	return uLimtSpeed;
}

/*
函数功能：转换车牌颜色对应的值
输入：RECORD_PLATE类型的车牌记录
输出：转换后的车牌颜色值
*/
string XingTaiCenter::CarNumColorTansform(RECORD_PLATE& nPlate)
{
	string strCarnumColor("");

	if (nPlate.uColor == 1)
	{
		strCarnumColor = "2";
	}
	else if (nPlate.uColor == 2)
	{
		strCarnumColor = "3";
	}
	else if (nPlate.uColor == 3)
	{
		strCarnumColor = "1";
	}
	else if (nPlate.uColor == 4)
	{
		strCarnumColor = "0";
	}
	else
	{
		strCarnumColor = "4";
	}
	
	return strCarnumColor;
}


/*
函数功能：车辆类型转换，根据协议，其他类型不好转换，做不能识别处理
输入：RECORD_PLATE类型车牌记录
输出：转换后的车辆类型代码
*/
string XingTaiCenter::CarNumTypeCovert(RECORD_PLATE& nPlate)
{
	string strCarType("");

	if (nPlate.uColor == CARNUM_BLUE)
	{
		strCarType = "02";
	}
	else if (nPlate.uType == CARNUM_YELLOW)
	{
		strCarType = "01";
	}

	return strCarType;
}

/*
函数功能：车辆行驶方向转换
输入：RECORD_PLATE类型车牌记录
输出：转换后的方向值
*/
string XingTaiCenter::CarDirectionCovert(RECORD_PLATE& nPlate)
{
	string nStrDirection("");

	if (nPlate.uDirection == EAST_TO_WEST || nPlate.uDirection == SOUTHEAST_TO_NORTHWEST || nPlate.uDirection == NORTHEAST_TO_SOUTHWEST)
	{
		nStrDirection = "2";
	}
	else if (nPlate.uDirection == WEST_TO_EAST || nPlate.uDirection == NORTHWEST_TO_SOUTHEAST || nPlate.uDirection == SOUTHWEST_TO_NORTHEAST)
	{
		nStrDirection = "4";
	}
	else if (nPlate.uDirection == SOUTH_TO_NORTH)
	{
		nStrDirection = "1";
	}
	else if (nPlate.uDirection == NORTH_TO_SOUTH)
	{
		nStrDirection = "3";
	}

	return nStrDirection;
}


/*
函数功能：产生存储某条车牌记录图片的路径
输入：RECORD_PLATE类型车牌记录
输出：图片路径
*/
string XingTaiCenter::GetPlatePicPath(RECORD_PLATE& nPlate,int index)
{
	string strPath("");

	//时间
	string nStrTime;
	char buf[32] = {0};
	sprintf(buf,"%03d",nPlate.uMiTime);
	nStrTime = GetTime(nPlate.uTime,2);
	nStrTime.append(buf,strlen(buf));
	
	strPath += nStrTime;
	strPath += "-";

	//违法类型及代码
	if (nPlate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)//闯红灯
	{
		strPath += "1(1211)";
	}
	else if (nPlate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)//超速
	{
		strPath += "2";
		string strCode("");
		strPath += "(";
		strCode = GetOverSpeedCode(nPlate);
		strPath += strCode;
		strPath += ")";

	}
	else if (nPlate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION)//逆行
	{
		strPath += "3(1301)";
	}
	else if (nPlate.uViolationType == DETECT_RESULT_FORBID_LEFT || nPlate.uViolationType == DETECT_RESULT_FORBID_RIGHT || nPlate.uViolationType == DETECT_RESULT_FORBID_STRAIGHT)//不按机动车道行驶
	{
		strPath += "4(1208)";
	}
	else if (nPlate.uViolationType == DETECT_RESULT_PRESS_LINE)//压黄线 
	{
		strPath += "5(1208)";
	}
	else if (nPlate.uViolationType == DETECT_RESULT_NOCARNUM || nPlate.uViolationType == DETECT_RESULT_ALL)
	{
		strPath += "0()";
	}

	

	//车牌号
	strPath += "(";
	string strCarNum("");
	strCarNum = nPlate.chText;
	if (strcmp(strCarNum.c_str(),"*******") == 0 || strcmp(strCarNum.c_str(),"*-----*") == 0)
	{
		strCarNum = "#######";
	}
	strPath += strCarNum;
	strPath += ")";

	//车速值
	strPath += "(";
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%u",nPlate.uSpeed);
	strPath += buf;
	strPath += ")";

	//限速值
	strPath += "(";
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%u",GetLimitSpeed(nPlate));
	strPath += buf;
	strPath += ")";

	//车牌颜色
	strPath += "(";
	strPath += CarNumColorTansform(nPlate);
	strPath += ")"; 

	//号牌种类
	strPath += "(";
	strPath += CarNumTypeCovert(nPlate);
	strPath += ")";

	//编号
	strPath += "(";
	strPath += g_strDetectorID;
	strPath += CarDirectionCovert(nPlate);
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%02d",nPlate.uRoadWayID);
	strPath += buf;
	strPath += ")";

	//红灯时间
	if (nPlate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)//闯红灯
	{
		strPath += "(3)";
	}
	else
	{
		strPath += "()";
	}

	////图片张数序号
	//strPath += "(1)";

	//车辆类型
	if(nPlate.uType == BIG_CAR)
	{
		strPath += "(K11)";
	}
	else if(nPlate.uType == SMALL_CAR)
	{
		strPath += "(K31)";
	}
	else
	{
		strPath += "()";
	}

	//数据来源
	strPath += "(北京四通)";

	char bufTmp[8] = {0};
	sprintf(bufTmp,"-%d",index);

	strPath += bufTmp;
	//后缀名
	strPath += ".jpg";

	return strPath;
}


/*
函数功能：连接到指定IP和端口的服务器
输入：无
输出：无
返回值：连接成功返回true，否则返回false
*/
bool XingTaiCenter::ConnectToCS()
{
	cerr<<"ConnectToCS()"<<endl;
	if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
	{
		//LogError("111ConnectToCS 中心数据服务器连接参数异常:host=%s,port=%d\n",g_strControlServerHost.c_str(), g_nControlServerPort);
		return false;
	}

	if (!mvPrepareSocket(m_nCenterSocket))
	{
		LogError("222ConnectToCS准备连接中心数据服务器套接字失败!\n");
		return false;
	}

	if (!mvWaitConnect(m_nCenterSocket, g_strControlServerHost, g_nControlServerPort))
	{
		LogError("333ConnectToCS尝试连接中心数据服务器失败，host=%s,port=%d!\n",g_strControlServerHost.c_str(), g_nControlServerPort);
		return false;
	}
	LogNormal("Connect to CS %s,port:%d success\n",g_strControlServerHost.c_str(),g_nControlServerPort);
	m_nCenterLink = true;
	return true;
}

/*
函数功能：获取中心端连接成功标志
输入：无
输出：无
返回值：中心端连接成功标志
*/
bool XingTaiCenter::GetCenterLinkFlag()
{
	return m_nCenterLink;
}


/*
函数功能：获取中心端套接字
输入：无
输出：无
返回值：中心端套接字
*/
int XingTaiCenter::GetCenterSocket()
{
	return m_nCenterSocket;
}

/*
函数功能：获取心跳失败次数
输入：无
返回值：心跳失败次数
*/
int XingTaiCenter::GetHeartBeatFailureCount()
{
	//cerr<<"nHeartBeatCount:"<<m_nHeartBeatCount<<endl;
	return m_nHeartBeatCount;
}


/*
函数功能：关闭中心端套接字，并置中心端连接标志为false
输入：无
输出：无
返回值:无 
*/
void XingTaiCenter::CloseCenterSocket()
{
	mvCloseSocket(m_nCenterSocket);
	pthread_mutex_lock(&m_CenterLinkClose_Mutex);
	LogError("close the socket\n");
	/*shutdown(m_nCenterSocket,2);
	close(m_nCenterSocket);
	m_nCenterSocket = -1;*/
	m_nCenterLink = false;
	m_nHeartBeatCount = 0;
	pthread_mutex_unlock(&m_CenterLinkClose_Mutex);
}

/*
函数功能：向中心端发送心跳包和车牌信息包
输入：nSocket：中心端套接字，strData要发送的数据（心跳包不需加密，车牌包后续需加密），nType：0：心跳包，1：车牌包
输出：无
返回值：发送成功返回true，否则返回false
*/
bool XingTaiCenter::SendMsg(const int nSocket,const string& strData,int nType)
{
	if(nSocket <= 0)
	{
		return false;
	}

	string strFullMsg("");
	strFullMsg.append( (char *)strData.c_str(), strData.size() );

	if (nType == 0)
	{
		if(!mvSendMsgToSocket(nSocket,strFullMsg,true))
		{
			LogError("11111SendMsg SendHeartBeatMsg error\n");
			CloseCenterSocket();
			return false;
		}
	}
	else if (nType == 1)
	{
		if (strFullMsg.size() <= 10+sizeof(Type_Struct_Frt_Vhc_Data))//没有图片数据关闭连接
		{
			LogError("1111SendMsg no PicData\n");
			CloseCenterSocket();
		}
		if (!mvSendMsgToSocket(nSocket,strFullMsg,true))
		{
			LogError("2222SendMsg SendPlatAndPicMsg error\n");
			CloseCenterSocket();
			return false;
		}
	}
	m_nHeartBeatCount = 0;
	return true;
}



//加密二进制数据块
void XingTaiCenter::encryption(char* buf,size_t len)
{
	for(size_t i=0;i<len;i++){
		buf[i] = (char)0x3C ^ buf[i];
	}
}


/*
函数功能：获取违法类型所对应的违法代码
输入：RECORD_PLATE类型车牌记录
输出：违法代码
返回值：违法代码
*/
string XingTaiCenter::GenerateViolationCode(RECORD_PLATE& nPlate)
{
	string strVioCode("");

	if (nPlate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)//超速
	{
		strVioCode = GetOverSpeedCode(nPlate);
	}
	else if (nPlate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)//闯红灯 
	{
		strVioCode = "1211";
	}
	else if (nPlate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION)//逆行
	{
		strVioCode = "1301";
	}
	else if (nPlate.uViolationType == DETECT_RESULT_PRESS_LINE || nPlate.uViolationType == DETECT_RESULT_FORBID_LEFT || nPlate.uViolationType == DETECT_RESULT_FORBID_RIGHT || nPlate.uViolationType == DETECT_RESULT_FORBID_STRAIGHT)//压黄线
	{
		strVioCode = "1208";
	}
	else if (nPlate.uViolationType == DETECT_RESULT_ALL || nPlate.uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据
	{
		strVioCode = "";
	}

	return strVioCode;
}



//产生车牌图片消息体
string XingTaiCenter::GeneratePlateAndPicMsg(RECORD_PLATE *sPlate)
{

	string strPlateAndPicMsg("");
	if (sPlate == NULL)
	{
		return strPlateAndPicMsg;
	}

	UINT16 nHeader = 0xAAAA;
	UINT16 nType = 1101;//包类型

	strPlateAndPicMsg.insert(0,(char*)&nHeader,sizeof(nHeader));
	strPlateAndPicMsg.insert(sizeof(nHeader),(char*)&nType,sizeof(nType));

	///cerr<<"222size:"<<strPlateAndPicMsg.size()<<endl;
	
	Type_Struct_Frt_Vhc_Data nVhcData;
	memset(&nVhcData,0,sizeof(Type_Struct_Frt_Vhc_Data));

	//本包标志序号  （递增的）
	nVhcData.bid = sPlate->uSeq;

	//时间1
	string strTime("");
	strTime = GetTime(sPlate->uTime,0);
	memcpy(nVhcData.tgsj1,strTime.c_str(),strTime.size());

	//违法类型（字符串格式）
	string strViolationCode("");

	if (sPlate->uViolationType == DETECT_RESULT_ALL || sPlate->uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据
	{
		strViolationCode = "0";
	}
	if (sPlate->uViolationType == DETECT_RESULT_EVENT_GO_FAST)//超速
	{
		strViolationCode = "2";
	}
	else if (sPlate->uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)//闯红灯 
	{
		strViolationCode = "1";
	}
	else if (sPlate->uViolationType == DETECT_RESULT_RETROGRADE_MOTION)//逆行
	{
		strViolationCode = "3";
	}
	else if (sPlate->uViolationType == DETECT_RESULT_FORBID_LEFT || sPlate->uViolationType == DETECT_RESULT_FORBID_RIGHT || sPlate->uViolationType == DETECT_RESULT_FORBID_STRAIGHT)//不按机动车道行驶
	{
		strViolationCode = "4";
	}
	else if (sPlate->uViolationType == DETECT_RESULT_PRESS_LINE)//压黄线
	{
		strViolationCode = "5";
	}
	memcpy(nVhcData.wflx,strViolationCode.c_str(),strViolationCode.size());

	//文件名中的第3项
	string strVioCode("");
	strVioCode = GenerateViolationCode(*sPlate);
	memcpy(nVhcData.bl,strVioCode.c_str(),strVioCode.size());
	

	//车牌号码（第4项、GBK编码）
	string strCarNum = sPlate->chText;
	if (strcmp(strCarNum.c_str(),"*******") == 0 || strcmp(strCarNum.c_str(),"*-----*") == 0)
	{
		strCarNum = "#######";
	}
	g_skpDB.UTF8ToGBK(strCarNum);
	memcpy(nVhcData.cphm1,strCarNum.c_str(),strCarNum.size());

	//速度，float格式
	nVhcData.speed = sPlate->uSpeed;

	//限速，float格式
	nVhcData.speedlimit = GetLimitSpeed(*sPlate);

	//车牌颜色，数字格式
	string strCarNumColor("");
	strCarNumColor = CarNumColorTansform(*sPlate);
	nVhcData.cpys = atoi(strCarNumColor.c_str());

	//车牌类型，第8项，数字格式
	string strCarNumType("");
	strCarNumType = CarNumTypeCovert(*sPlate);
	nVhcData.cplx1 = atoi(strCarNumType.c_str());
	//cerr<<"CarNum Type："<<nVhcData.cplx1<<"aaa:"<<atoi(strCarNumType.c_str())<<endl;

	//设备编号的前5位，字符串格式(根据协议，最多不要超过8字节)
	memcpy(nVhcData.jcdid,g_strDetectorID.c_str(),g_strDetectorID.size());

	//行驶方向，第10项，数字格式
	string strDirection("");
	strDirection = CarDirectionCovert(*sPlate);
	nVhcData.xsfx = atoi(strDirection.c_str());

	//车道ID，第11项，字符串格式
	char buf[6] = {0};
	sprintf(buf,"%02d",sPlate->uRoadWayID);
	memcpy(nVhcData.cdid,buf,strlen(buf));

	//车辆类型，第12项，字符串格式
	string strCarType("");
	if (sPlate->uType == BIG_CAR)
	{
		strCarType = "K11";
	}
	else if (sPlate->uType == SMALL_CAR)
	{
		strCarType = "K31";
	}
	memcpy(nVhcData.cllx,strCarType.c_str(),strCarType.size());

	if (sPlate->uViolationType == DETECT_RESULT_ALL || sPlate->uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据
	{
		//图片张数
		nVhcData.tpzs = '1';
	}
	else
	{
		//图片张数
		nVhcData.tpzs = '3';
	}
	

	//图片长度，数字格式
	string strPicData("");
	//cerr<<"path:"<<sPlate->chPicPath<<endl;
	UINT32 nPicSize1 = 0;
	UINT32 nPicSize2 = 0;
	UINT32 nPicSize3 = 0;
	UINT32 nPicSize4 = 0;
	strPicData = GetComposedSingleImageByPath(*sPlate,nPicSize1,nPicSize2,nPicSize3,nPicSize4);
	if (sPlate->uViolationType == DETECT_RESULT_ALL || sPlate->uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据
	{
		if (strPicData.size() <= 0 || nPicSize1 <= 0)
		{
			return "";
		}
	}
	else
	{
		if (strPicData.size() <= 0 || nPicSize1 <= 0 || nPicSize2 <= 0 || nPicSize3 <= 0 /*|| nPicSize4 <= 0*/)
		{
			return "";
		}
	}
	
	nVhcData.cpcd1 = nPicSize1;
	nVhcData.cpcd2 = nPicSize2;
	nVhcData.cpcd3 = nPicSize3;
	/*nVhcData.cpcd4 = nPicSize4;*/

	/*cerr<<"pic1Size:"<<nVhcData.cpcd1<<endl;
	cerr<<"pic2Size:"<<nVhcData.cpcd2<<endl;
	cerr<<"pic3Size:"<<nVhcData.cpcd3<<endl;*/

	//图片名称，字符串格式，GBK编码
	string strPicName("");

	if (sPlate->uViolationType == DETECT_RESULT_ALL || sPlate->uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据
	{
		//拆分后第一张图片的名字
		strPicName = GetPlatePicPath(*sPlate,1);
		//cerr<<"1111picName:"<<strPicName<<endl;
		g_skpDB.UTF8ToGBK(strPicName);
		//cerr<<"picName:"<<strPicName<<endl;
		memcpy(nVhcData.tpid1,strPicName.c_str(),strPicName.size());
	}
	else
	{
		//拆分后第一张图片的名字
		strPicName = GetPlatePicPath(*sPlate,1);
		//cerr<<"1111picName:"<<strPicName<<endl;
		g_skpDB.UTF8ToGBK(strPicName);
		//cerr<<"picName:"<<strPicName<<endl;
		memcpy(nVhcData.tpid1,strPicName.c_str(),strPicName.size());

		//拆分后第二张图片的名字
		strPicName.clear();
		strPicName = GetPlatePicPath(*sPlate,2);
		//cerr<<"2222picName:"<<strPicName<<endl;
		g_skpDB.UTF8ToGBK(strPicName);
		//cerr<<"picName:"<<strPicName<<endl;
		memcpy(nVhcData.tpid2,strPicName.c_str(),strPicName.size());

		//拆分后第三张图片的名字
		strPicName.clear();
		strPicName = GetPlatePicPath(*sPlate,3);
		//cerr<<"3333picName:"<<strPicName<<endl;
		g_skpDB.UTF8ToGBK(strPicName);
		//cerr<<"picName:"<<strPicName<<endl;
		memcpy(nVhcData.tpid3,strPicName.c_str(),strPicName.size());

		////拆分后第四张图片的名字
		//strPicName.clear();
		//strPicName = GetPlatePicPath(*sPlate,4);
		////cerr<<"44444picName:"<<strPicName<<endl;
		//g_skpDB.UTF8ToGBK(strPicName);
		////cerr<<"picName:"<<strPicName<<endl;
		//memcpy(nVhcData.tpid4,strPicName.c_str(),strPicName.size());
	}
	


	//车牌信息加密
	encryption((char* )&nVhcData,sizeof(nVhcData));

	//图片信息
	

	//cerr<<"strPicData.size:"<<strPicData.size()<<endl;
	//length 
	UINT32 nLength = sizeof(nVhcData) + strPicData.size();
	strPlateAndPicMsg.insert(4,(char*)&nLength,sizeof(nLength));

	//车辆信息
	strPlateAndPicMsg.insert(8,(char*)&nVhcData,sizeof(nVhcData));

	encryption((char*)strPicData.c_str(),strPicData.size());

	//图片信息 
	//strPlateAndPicMsg.insert(8+sizeof(nVhcData),strPicData.c_str(),strPicData.size());

	strPlateAndPicMsg += strPicData;

	UINT16 nTail = 0x5555;
	strPlateAndPicMsg.insert(8+sizeof(nVhcData)+strPicData.size(),(char*)&nTail,sizeof(nTail));

	//cerr<<"111size:"<<strPlateAndPicMsg.size()<<endl;


	return strPlateAndPicMsg;
}

/*
函数功能：将一张2*2的违章图片拆分成四张单独的图片并组合（第一张图片+第二张图片+第三张图片+特写图片）

输入：plate：车牌记录，nPicSize1:用于返回第一张图片的大小，nPicSize2:用于返回第二张图片的大小，
nPicSize3:用于返回第三张图片的大小，nPicSize4:用于返回特写图片的大小，

输出：第一二三张图片及特写图片的大小

函数返回：分开组合后的图片消息体（第一张图片+第二张图片+第三张图片+特写图片）

*/
string XingTaiCenter::GetComposedSingleImageByPath(RECORD_PLATE& plate,UINT32& nPicSize1,UINT32& nPicSize2,UINT32& nPicSize3,UINT32& nPicSize4)
{
	/*cerr<<"1111plate.chText:"<<plate.chText<<endl;
	string strTime = GetTime(plate.uTime,0);
	cerr<<"Time:"<<strTime<<endl;
	cerr<<"1111PicWidth:"<<plate.uPicWidth<<endl;
	cerr<<"111PicHeight:"<<plate.uPicHeight<<endl;*/
	string strPicData("");
	string strComposedSinglePicData("");

	string strPicPath("");
	strPicPath = plate.chPicPath;
	if (strPicPath.size() <= 0)
	{
		return strComposedSinglePicData;
	}
	strPicData = GetImageByPath(plate.chPicPath);

	if (strPicData.size() > 0)
	{
		if (plate.uViolationType == DETECT_RESULT_ALL || plate.uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据
		{
			nPicSize1 = strPicData.size();
			return strPicData;
		}
		//需要解码jpg图像
		CxImage image;
		image.Decode((BYTE*)(strPicData.c_str()), strPicData.size(), 3); //违章合成大图（2*2 叠加小图）解码

		if(image.IsValid()&&image.GetSize()>0)
		{

			UINT32 uPicWidth = image.GetWidth();
			UINT32 uPicHeight = image.GetHeight();
			IplImage *pPicture = NULL;
			pPicture = cvCreateImageHeader(cvSize(uPicWidth,uPicHeight),8,3);
			if (pPicture == NULL)
			{
				LogError("pPicture cvCreateImageHeader error\n");
				return strComposedSinglePicData;
			}
			cvSetData(pPicture,image.GetBits(),pPicture->widthStep);

			IplImage *pPictureSnap = NULL;
			pPictureSnap = cvCreateImage(cvSize(uPicWidth,uPicHeight),8,3);
			if (pPictureSnap == NULL)
			{
				LogError("pPictureSnap CreateImage error\n");
				return strComposedSinglePicData;
			}

			CvRect dstRt;
			dstRt.x = 0;
			dstRt.y = 0;
			dstRt.height = uPicHeight;
			dstRt.width = uPicWidth;

			cvSetImageROI(pPictureSnap,dstRt);
			cvCopy(pPicture,pPictureSnap);
			cvResetImageROI(pPictureSnap);

			UINT32 uImageSize = (uPicWidth/2)*(uPicHeight/2);
			int srctep = 0;
			
			//第一张图片
			CvRect dstRt1;
			dstRt1.x = 0;
			dstRt1.y = 0;
			dstRt1.height = uPicHeight/2;
			dstRt1.width = uPicWidth/2;
			unsigned char* pJpgImage1 = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
			nPicSize1 = GetSmallImage(pPictureSnap,pJpgImage1,dstRt1);

			//第二张图片
			CvRect dstRt2;
			dstRt2.x = uPicWidth/2;
			dstRt2.y = 0;
			dstRt2.height = uPicHeight/2;
			dstRt2.width = uPicWidth/2;
			unsigned char* pJpgImage2 = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
			nPicSize2 = GetSmallImage(pPictureSnap,pJpgImage2,dstRt2);


			//第三张图片
			CvRect dstRt3;
			dstRt3.x = 0;
			dstRt3.y = uPicHeight/2;
			dstRt3.height = uPicHeight/2;
			dstRt3.width = uPicWidth/2;
			unsigned char* pJpgImage3 = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
			nPicSize3 = GetSmallImage(pPictureSnap,pJpgImage3,dstRt3);

			////特写图片
			//CvRect dstRt4;
			//dstRt4.x = uPicWidth/2;
			//dstRt4.y = uPicHeight/2;
			//dstRt4.height = uPicHeight/2;
			//dstRt4.width = uPicWidth/2;
			//unsigned char* pJpgImage4 = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
			//nPicSize4 = GetSmallImage(pPictureSnap,pJpgImage4,dstRt4);


			if (nPicSize1 <= 0 || nPicSize2 <= 0 || nPicSize3 <= 0 /*|| nPicSize4 <= 0*/)
			{
				cerr<<"==GetComposedSingleImageByPath nPicSize <= 0"<<endl;
				return strComposedSinglePicData;
			}
			strComposedSinglePicData.append((char*)pJpgImage1,nPicSize1);
			strComposedSinglePicData.append((char*)pJpgImage2,nPicSize2);
			strComposedSinglePicData.append((char*)pJpgImage3,nPicSize3);
			//strComposedSinglePicData.append((char*)pJpgImage4,nPicSize4);
			////test
		/*	SaveImageTest(pJpgImage1,nPicSize1);
			SaveImageTest(pJpgImage2,nPicSize2);
			SaveImageTest(pJpgImage3,nPicSize3);*/
			//SaveImageTest(pJpgImage4,nPicSize4);
			////test

			if (pPicture != NULL)
			{
				cvReleaseImageHeader(&pPicture);
				pPicture = NULL;
			}
			if (pPictureSnap != NULL)
			{
				cvReleaseImage(&pPictureSnap);
				pPictureSnap = NULL;
			}
			if (pJpgImage1 != NULL)
			{
				free(pJpgImage1);
				pJpgImage1 = NULL;
			}
			if (pJpgImage2 != NULL)
			{
				free(pJpgImage2);
				pJpgImage2 = NULL;
			}
			if (pJpgImage3 != NULL)
			{
				free(pJpgImage3);
				pJpgImage3 = NULL;
			}
			/*if (pJpgImage4 != NULL)
			{
				free(pJpgImage4);
				pJpgImage4 = NULL;
			}*/

		}
		else
		{
			LogError("==GetComposedSingleImageByPath image Decode error\n");
		}
	}
	else
	{
		cerr<<"=====GetComposedSingleImageByPath picPath:%s\n"<<plate.chPicPath;
		LogError("=====GetComposedSingleImageByPath PicData size is 0!");
		LogError("=====picPath:%s\n",plate.chPicPath);
		char buf[1024];
		memset(buf, 0, 1024);
		sprintf(buf, "delete from NUMBER_PLATE_INFO where PICPATH = '%s';",plate.chPicPath);
		g_skpDB.execSQL(string(buf));
	}
	return strComposedSinglePicData;
}

//存图测试
void XingTaiCenter::SaveImageTest(unsigned char* pJpgData,int srcstep)
{
	//string strPicPath("");
	char buf[32] = {0};
	sprintf(buf,"/home/road/server/%d.jpg",m_nCount);
	FILE* fp = NULL;
	
	fp = fopen(buf,"wb");

	if (fp == NULL)
	{
		cerr<<"fopen "<<buf<<"error"<<endl;
	}
	else
	{
		fwrite(pJpgData,srcstep,1,fp);
	}
	fclose(fp);
	m_nCount += 1;
}



