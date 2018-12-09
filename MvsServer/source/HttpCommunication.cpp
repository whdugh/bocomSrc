

//源代码如下：
/******* http客户端程序 httpclient.c ************/
#include "Common.h"
#include "CommonHeader.h"
#include "HttpCommunication.h"

#define MAX_IMAGE 5000000  //500万的图片的大小
CHttpCommunication g_HttpCommunication;
//////////////////////////////httpclient.c 开始///////////////////////////////////////////
//发送心跳
void* ThreadHTTPLink(void* pArg)
{
	//处理一条数据
	CHttpCommunication *pHttp = (CHttpCommunication *)pArg;
	string strReturn = "";
	if (pHttp)
	{
		while((!g_bEndThread))
		{
			if(pHttp->GetbCenterLink())
			{
				UINT32 nCaneraID = g_skpDB.GetCameraID(1);
				string strLinkMsg = pHttp->GetLinkTest(nCaneraID);
				if (!(pHttp->mvSendMsgToSocket(pHttp->GetCenterSocket(), strLinkMsg)))
				{
					pHttp->mvCloseSocket(pHttp->GetCenterSocket());
					pHttp->GetbCenterLink() = false;
					LogError("发送消息失败，连接断开\n");
				}
				else
				{
					strReturn = "";
					pHttp->mvRecvMsg(strReturn);
				}
			}
			sleep(10);
		}
		
	}

	pthread_exit((void *)0);
	return pArg;
}


//记录发送线程
void* ThreadHTTPResult(void* pArg)
{
	//处理一条数据
	g_HttpCommunication.DealResult();

    pthread_exit((void *)0);
	return pArg;
}

//历史记录发送线程
void* ThreadHTTPHistoryResult(void* pArg)
{
	//处理一条数据
	CHttpCommunication *pHttp = (CHttpCommunication *)pArg;
	if (pHttp)
	{
		pHttp->DealHistoryResult();
	}

    pthread_exit((void *)0);
	return pArg;
}


CHttpCommunication::CHttpCommunication()
{
    m_bCenterLink = false;
    m_nCenterSocket = 0;

    m_nThreadId = 0;
    m_nHistoryThreadId = 0;
	m_nLinkThreadId = 0;

    pthread_mutex_init(&m_Result_Mutex,NULL);
	pthread_mutex_init(&m_file_mutex,NULL);
}

CHttpCommunication::~CHttpCommunication()
{
	 pthread_mutex_destroy(&m_Result_Mutex);
	 pthread_mutex_destroy(&m_file_mutex);
}

//处理实时数据
void CHttpCommunication::DealResult()
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
			std::list<string>::iterator it = m_ResultList.begin();
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
void CHttpCommunication::DealHistoryResult()
{
		while(!g_bEndThread)
		{
			if(g_nSendHistoryRecord == 1)
			{
				StrList strListRecord;
				strListRecord.clear();
				if(g_skpDB.GetPlateHistoryRecord(strListRecord))
				{
						StrList::iterator it_b = strListRecord.begin();
						while(it_b != strListRecord.end())
						{
							string strPlate("");
							strPlate = *it_b;
									
							bool bSendStatus = false;
							bSendStatus = mvSendRecordToCS(strPlate);

							if(bSendStatus)
							{
								UINT32 uSeq = *(UINT32*)(strPlate.c_str()+sizeof(MIMAX_HEADER));
								g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, uSeq);
								sleep(5);
							}
							
							it_b++;
						}		
				}
				else
				{
						sleep(60);
				}
			}
			sleep(5);
		}
}

//添加一条数据
bool CHttpCommunication::AddResult(std::string& strResult)
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
bool CHttpCommunication::OnResult(std::string& result)
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
    return false;
}


/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CHttpCommunication::mvSendRecordToCS(const string &strMsg)
{
	string strReturn = "";
    if(!m_bCenterLink)
    {
        return false;
    }
    MIMAX_HEADER* mHeader = (MIMAX_HEADER *)strMsg.c_str();

    UINT32 uCameraID;

    if(MIMAX_EVENT_REP == mHeader->uCmdID)
    {
		return true;
    }
    else if (MIMAX_PLATE_REP == mHeader->uCmdID)
    {
		//转换
		string strFullMsg = GetRequest(strMsg);

		if (!mvSendMsgToSocket(m_nCenterSocket, strFullMsg))
		{
			strReturn = "";

			mvRecvMsg(strReturn);


			mvCloseSocket(m_nCenterSocket);
			m_bCenterLink = false;
			LogError("发送消息失败，连接断开\n");
			return false;
		}
		else
		{
			strReturn = "";
			mvRecvMsg(strReturn);
		}
		return true;
    }
}


//获取请求消息
string CHttpCommunication::GetRequest(const string& strMsg) 
{
	RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
	MIMAX_HEADER* mHeader = (MIMAX_HEADER *)strMsg.c_str();
	UINT32 uCameraID = mHeader->uCameraID;

	//把车身颜色用GA-24.8-2005编码表示
	char chColor;
	switch(pPlate->uCarColor1)
	{
		case WHITE: chColor = 'A';break;
		case GRAY: chColor = 'B';break;
		case YELLOW: chColor = 'C';break;
		case PINK: chColor = 'D';break;
		case RED: chColor = 'E';break;
		case PURPLE: chColor = 'F';break;
		case GREEN: chColor = 'G';break;
		case BLUE: chColor = 'H';break;
		case BROWN: chColor = 'I';break;
		case BLACK: chColor = 'J';break;
		case UNKNOWN: chColor = 'Z';break;
		default: chColor = 'Z';break;
	}
	
	//车辆类型转换
	char chCarType[4] = {0};
	switch(pPlate->uTypeDetail)
	{
		case TAXI: memcpy(chCarType,"K31",3);break;//小型客车
		case MIDDLEBUS_TYPE: memcpy(chCarType,"K21",3);break;//中型客车
		case BUS_TYPE: memcpy(chCarType,"K11",3);break;//大型客车

		case MINI_TRUCK: memcpy(chCarType,"H31",3);break;//小型货车
		case TRUCK_TYPE: memcpy(chCarType,"H11",3);break;//大型货车
		
		default: memcpy(chCarType,"X99",3);break;//其他
	}

	//车牌类型转换
	char chPlateType[3] = {0};
	switch(pPlate->uColor)
	{
		case CARNUM_BLUE: memcpy(chPlateType,"02",2);break;//蓝牌
		case CARNUM_YELLOW: memcpy(chPlateType,"01",2);break;//黄牌

		default: memcpy(chPlateType,"99",2);break;//其他
	}

	//车牌颜色转换
	char chPlateColor;
	switch(pPlate->uColor)
	{
		case CARNUM_BLUE: chPlateColor = '2';break;
		case CARNUM_BLACK: chPlateColor = '3';break;
		case CARNUM_YELLOW: chPlateColor = '1';break;
		case CARNUM_WHITE: chPlateColor = '0';break;
		default: chPlateColor = '4';break;
	}

	//车牌号码
	string strCarNum(pPlate->chText);
	if (!(strCarNum.compare("11111111")))
	{
		strCarNum = "";//不能识别开始可用空表示
	}

	//经过时间
	string strPassTime = GetTime(pPlate->uTime,2);
	char chUsec[4] = {0};
	sprintf(chUsec,"%03d",pPlate->uMiTime);

	//抓拍时间
	string strSnapTime = strPassTime + chUsec;

	//地点
	string strPlace(pPlate->chPlace);


	//获取图片
    string strImage;
	if(strMsg.size() == sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE))
	{
		String strPicPath(pPlate->chPicPath);
		strImage = GetImageByPath(strPicPath);
	}
	else
	{
		strImage.append((char*)strMsg.c_str() + sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE),strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE));
	}

	string strBoundary = GetBoundary();
	int nRam = GetRandTime();
	char pRequest[10240] = {0};
	//1.请求行
	sprintf(pRequest, "POST //toll-gate/home/upload?ram=%d&deviceId=%d&time=%s&type=%s&description=%s HTTP/1.1\r\n\
Host: %s:%d\r\n",nRam,uCameraID,strSnapTime.c_str(),"EventSnapshot","车牌识别",g_strControlServerHost.c_str(),g_nControlServerPort);

	string strRequest1(pRequest);

    //3.请求正文
	memset(pRequest, 0, 10240);
	sprintf(pRequest, "Content-Disposition: form-data; name=\"analyseResult\"\r\n\r\n\
<?xml version=\"1.0\" encoding=\"utf-16\"?>\r\n\
<analyseResult>\r\n\
	<vehicleInfo vehicleType=\"%s\" vehicleColor=\"%c\" vehicleLength=\"%d\" vehicleNoType=\"%s\" vehicleNo=\"%s\" vehicleNoColor=\"%c\">\r\n\
		<driveInfo time=\"%s\" location=\"%s\" speed=\"%d\" status=\"%d\"/>\r\n\
	</vehicleInfo>\r\n\
</analyseResult>\r\n\
-----------------------------%s\r\n\
Content-Disposition: form-data; name=\"vehicleImage\"; filename=\"%s\"\r\n\
Content-Type: image/jpeg\r\n\r\n",chCarType,chColor,300,chPlateType,strCarNum.c_str(),chPlateColor,strPassTime.c_str(),strPlace.c_str(),pPlate->uSpeed,0,strBoundary.c_str(),pPlate->chPicPath);

    string strRequest3(pRequest);

	strRequest3 += strImage;

	strRequest3 = strRequest3 + "\r\n-----------------------------" + strBoundary + "\r\n";

	char chTempBuf[512] = {0};
	memset(chTempBuf, 0, 512);
	sprintf(chTempBuf, "Content-Disposition: form-data; name=\"vehicleFeatureImage\"; filename=\"%s\"\r\n\
	Content-Type: image/jpeg\r\n\r\n", pPlate->chPicPath);
	strRequest3 += strImage;
	strRequest3 = strRequest3 + "\r\n-----------------------------" + strBoundary + "--"/* + "\r\n"*/;


	//2.请求参数
	int nlength = strRequest3.size();
	////消息报头：请求报头允许客户端向服务器端传递请求的附加信息以及客户端自身的信息。
	memset(pRequest, 0, 10240);
	sprintf(pRequest, "\
Content-Type: multipart/form-data;boundar=-----------------------------%s\r\n\
Content-Length:%d\r\n\
Cache-Control: no-cache\r\n\
Pragma: no-cache\r\n\
Accept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\n\
Connection: keep-alive\r\n\r\n\
-----------------------------%s\r\n",strBoundary.c_str(),nlength,strBoundary.c_str());
	string strRequest2(pRequest);

	string strRequest = strRequest1 + strRequest2 + strRequest3;
	return strRequest;
}



bool CHttpCommunication::Init()
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
	int nret=pthread_create(&m_nThreadId,&attr,ThreadHTTPResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadHTTPHistoryResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	nret=pthread_create(&m_nLinkThreadId,&attr,ThreadHTTPLink,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建心跳发送线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}
	

	pthread_attr_destroy(&attr);

	return true;
}

//释放
bool CHttpCommunication::UnInit()
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

	if(m_nLinkThreadId != 0)
	{
		pthread_join(m_nLinkThreadId,NULL);
		m_nLinkThreadId = 0;
	}

    m_ResultList.clear();
	return true;
}

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void CHttpCommunication::mvConnOrLinkTest()
{
    if (!g_bEndThread)
    {
        if (!m_bCenterLink)
        {
             if (mvConnectToCS())
             {
                 LogNormal("连接http服务器成功!\n");
                 m_bCenterLink = true;
				 SendRegister();
             }
			 else
			 {
				 LogNormal("连接http服务器失败!\n");
                 m_bCenterLink = false;
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
bool CHttpCommunication::mvConnectToCS()
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

    return true;
}


string CHttpCommunication::GetLinkTest(UINT32 uCameraID)
{
	char chLink[512] = {0};
	string strBoundary = GetBoundary();

	////////////////
	char tmpBuf[512] = {0};
	int VideoFlag = -1;
	if(g_skpDB.GetCameraState(uCameraID) != 0)
	{
		VideoFlag = 0;
	}
	else
	{
		VideoFlag = 1;
	}
	int DetetorValuetmp = GetDetectorValue(); 
	LogNormal("in GetLinkTest VideoFlag:%d,DetetorValuetmp:%d\n",VideoFlag,DetetorValuetmp);
	sprintf(tmpBuf,"<Message>\r\n<Device StateCode=\"1\">\r\n<Status Name=\"Detector\" Value=\"%d\"/>\r\n<Status Name=\"Flash\" Value=\"1\"/>\r\n<Status Name=\"Video\" Value=\"%d\"/>\r\n</Device>\r\n</Message>",DetetorValuetmp,VideoFlag);
	//LogNormal("in GetLinkTest tmpBuf:%s",tmpBuf);
	string strMsg = tmpBuf;
	///////////////

	//string strMsg = "<Message>\r\n<Device StateCode=\"1\">\r\n<Status Name=\"Detector\" Value=\"1\"/>\r\n<Status Name=\"Flash\" Value=\"1\"/>\r\n<Status Name=\"Video\" Value=\"1\"/>\r\n</Device>\r\n</Message>";
	sprintf(chLink,"POST /toll-gate/home/heartBeat?deviceId=%d HTTP/1.1\r\n\
Accept-Language: zh-cn\r\n\
Content-Type: multipart/form-data;\r\n\
Accept-Encoding: gzip, deflate\r\n\
boundary: ---------------------------%s\r\n\
Host: %s:%d\r\n\
Content-Length: %d\r\n\r\n\
%s",uCameraID,strBoundary.c_str(),g_strControlServerHost.c_str(),g_nControlServerPort,strMsg.size(),strMsg.c_str());

	string strLink(chLink);
	return strLink;
}


string CHttpCommunication::GetRegister(UINT32 uCameraID)
{
	char chRegister[512] = {0};
	sprintf(chRegister,"POST /toll-gate/home/regist?deviceId=%d HTTP/1.1\r\n\
Host: %s:%d\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length: 66\r\n\
Cache-Control: no-cache\r\n\
Connection: Keep-Alive\r\n\r\n\
deviceIp=%s&msgPort=1234&company=bocom&videoRegFlag= false",uCameraID,g_strControlServerHost.c_str(),g_nControlServerPort,g_ServerHost.c_str());
	string strRegister(chRegister);
	return strRegister;
}


bool CHttpCommunication::mvRecvMsg(string& strMsg)
{
	if (!strMsg.empty())
	{
		strMsg.clear();
	}
	if (m_nCenterSocket <= 0)
	{
		return false;
	}

	int nLeft = 0;
	int nBytes = 0;

	char chBuffer[SRIP_MAX_BUFFER] = {0};
	int nRet = recv(m_nCenterSocket, chBuffer,SRIP_MAX_BUFFER, MSG_NOSIGNAL);
	if (nRet < 0)
	{
		return false;
	}

	string strBuffer(chBuffer);
	///////////////for test
	if(!strBuffer.empty())
	{
		char* tmpMessage = (char*)strstr(strBuffer.c_str(),"CurrentDateTime=");
		if(tmpMessage != NULL)
		{
			tmpMessage = tmpMessage + 17;
			string tmpStr = tmpMessage;
			string tmpSubStr = tmpStr.substr(0,14);
			LogNormal("in mvRecvMsg CurrentDateTime=%s\n",tmpSubStr.c_str());
			OnSystemTime(tmpSubStr);
		}

		//SaveMessageToFile(strBuffer);
	}
	//////////////for test
	return (!strBuffer.empty());
}


int CHttpCommunication::GetRandTime()
{
	srand( (unsigned)time( NULL ) );
	int nRandTime = rand();
	return nRandTime;
}

//参数保留
bool CHttpCommunication::SendRegister(UINT32 uCameraID)
{
	UINT32 nCaneraID = g_skpDB.GetCameraID(1);//这里先用通道1的相机编号传参
	string strRegisterMsg = GetRegister(nCaneraID);
	string strReturn;
	if (!(mvSendMsgToSocket(GetCenterSocket(), strRegisterMsg)))
	{
		mvCloseSocket(GetCenterSocket());
		GetbCenterLink() = false;
		LogError("发送消息失败，连接断开\n");
		return false;
	}
	else
	{
		strReturn = "";
		mvRecvMsg(strReturn);
		return true;
	}
}

//获取边界
string CHttpCommunication::GetBoundary()
{
	char chBoundary[32] = {0};
	memset(chBoundary, 0, 32);
	time_t now;
	time( &now );
	sprintf(chBoundary, "%ld", now);
	string strBoundary(chBoundary);
	strBoundary = strBoundary.substr(0, 11);
	return strBoundary;
}

//将接收到响应消息存到文件
void CHttpCommunication::SaveMessageToFile(string strTmp)
{
	pthread_mutex_lock(&m_file_mutex);
	//LogNormal("in SaveMessageToFile 111111\n");
	FILE* fp = fopen("messageReturn.txt","a");
	if(fp == NULL)
	{
		cerr<<"open messageReturn.txt error"<<endl;
		return;
	}
	fwrite(strTmp.c_str(),strTmp.size(),1,fp);
	fclose(fp);
	pthread_mutex_unlock(&m_file_mutex);
	return;
}

//根据取到的线圈状态对车检器Detector的Value值做界定
int CHttpCommunication::GetDetectorValue()
{
	/*if (g_skpDB.GetDetectKind(1) != DETECT_LOOP)
	{
		LogNormal("No DETECT_LOOP,Default Detector Value is 1,%d\n",g_skpDB.GetDetectKind(1));
		return 1;
	}*/
	//if((g_skpDB.GetDetectKind(1) & DETECT_LOOP) == DETECT_LOOP)
	{
		//LogNormal("in GetDetectorValue: DETECT_LOOP\n");
		mapLoopStatus loopstaus = g_CenterServer.GetLoopStatus();
		if (loopstaus.size() > 0)
		{
			mapLoopStatus::iterator it = loopstaus.begin();
			while (it != loopstaus.end())
			{
				if (!(it->second))
				{
					//LogNormal("in GetDetectorValue() 车道%d线圈异常!",it->first);
					return 0;
				}
				it++;
			}
			return 1;
		}
		else
		{
			return 1;
		}
	}
	
	
}

//根据时间字符串(YYYYMMDDHHMMSS)还原成秒数
unsigned long CHttpCommunication::MyMakeTime(std::string& strTime)
{
	unsigned long uTime = 0;
	if(strTime.size()>0)
	{
		struct tm newTime;

		std::string subString = strTime.substr(0,4);
		int year = atoi(subString.c_str());
		subString = strTime.substr(4,2);
		int month = atoi(subString.c_str());
		subString = strTime.substr(6,2);
		int day = atoi(subString.c_str());

		subString = strTime.substr(8,2);
		int hour = atoi(subString.c_str());
		subString = strTime.substr(10,2);
		int minute = atoi(subString.c_str());
		subString = strTime.substr(12,2);
		int second = atoi(subString.c_str());

		newTime.tm_year = year - 1900;
		newTime.tm_mon = month -1;
		newTime.tm_mday = day;
		newTime.tm_hour = hour;
		newTime.tm_min = minute;
		newTime.tm_sec = second;

		uTime = mktime(&newTime);
	}
	return uTime;
}

//设置与卡口中心时间同步
bool CHttpCommunication::OnSystemTime(string timeTmp)
{
	if (timeTmp.size() <= 0)
	{
		return false;
	}
	else
	{
		UINT32 uTime = MyMakeTime(timeTmp);
		if (uTime > 0)
		{
			//LogNormal("in OnSystemTime:uTime=%ld\n",uTime);
			timeval timer;
			timer.tv_sec = uTime;
			timer.tv_usec = 0;
			if (settimeofday(&timer, NULL) == 0)
			{
				LogNormal("1111111中心校时成功=%s\n",GetTime(uTime).c_str());
				system("hwclock --systohc");
			}
		}
		return true;
		

	}

}
