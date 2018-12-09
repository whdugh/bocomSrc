

//源代码如下：
/******* http客户端程序 httpclient.c ************/
#include "Common.h"
#include "CommonHeader.h"
#include "HttpUpload.h"
#include "ejHttpUpload.h"


#define MAX_IMAGE 5000000  //500万的图片的大小
CHttpUpload g_HttpUpload;
//////////////////////////////httpclient.c 开始///////////////////////////////////////////
//初始化车道线程
void *ThreadRoadWayInit(void * pArg)
{
	CHttpUpload *pHttp = (CHttpUpload *)pArg;
	if (pHttp)
	{
		pHttp->InitRoadWay();
	}

	pthread_exit((void *)0);
	return pArg;
}

//记录发送线程
void* ThreadHTTPUpLoadResult(void* pArg)
{
	//处理一条数据
	g_HttpUpload.DealResult();

    pthread_exit((void *)0);
	return pArg;
}

//历史记录发送线程
void* ThreadHTTPUpLoadHistoryResult(void* pArg)
{
	//处理一条数据
	CHttpUpload *pHttp = (CHttpUpload *)pArg;
	if (pHttp)
	{
		pHttp->DealHistoryResult();
	}

    pthread_exit((void *)0);
	return pArg;
}
//磁盘管理线程
void* ThreadHTTPUpLoadDiskManage(void* pArg)
{
	//处理一条数据
	CHttpUpload *pHttp = (CHttpUpload *)pArg;
	if (pHttp)
	{
		pHttp->DiskManage();
	}
	pthread_exit((void *)0);
	return pArg;
}

CHttpUpload::CHttpUpload()
{
	m_nCenterSocket = 0;
	m_nInitThreadId  = 0;
    m_nThreadId = 0;
    m_nHistoryThreadId = 0;
	m_nDiskManageThreadId = 0;

	pthread_mutex_init(&m_Init_mutex,NULL);
    pthread_mutex_init(&m_Result_Mutex,NULL);
	pthread_mutex_init(&m_Send_Mutex,NULL);

	m_sendErLog = 0;
	
	gettimeofday(&m_TheTimeLog,NULL);
	
	//pthread_mutex_init(&m_file_mutex,NULL);  //test for receive message
	//pthread_mutex_init(&m_test_mutex,NULL);  //test for send message
}

CHttpUpload::~CHttpUpload()
{
	pthread_mutex_destroy(&m_Result_Mutex);
	pthread_mutex_destroy(&m_Send_Mutex);
	//pthread_mutex_destroy(&m_file_mutex);
	pthread_mutex_destroy(&m_Init_mutex);
	//pthread_mutex_destroy(&m_test_mutex);

}
//初始化车道
void CHttpUpload::InitRoadWay()
{
	while(!g_bEndThread)
	{
		int nCount = 0;
		list<ChannelInitInfo>::iterator iterInit= m_lchannelInitInfo.begin();
		for (;iterInit != m_lchannelInitInfo.end();iterInit++)
		{
			if (iterInit->initState == 0)  //未初始化
			{
				InitHttpTrans(iterInit->chDeviceID,iterInit->nCameraId,iterInit->nChanWayType,iterInit->chCode,iterInit->initState);
	
			}
			else if (iterInit->initState == -1) // 初始化等待
			{
				struct timeval tv;
				gettimeofday(&tv,NULL);
				struct tm *oldTime,timenow;
				oldTime = &timenow;
				localtime_r( &tv.tv_sec,oldTime);
				map<int,struct tm>::iterator iter = m_initStartWaitTime.find(iterInit->nCameraId);
				if ((oldTime->tm_min - iter->second.tm_min)>10)
				{
					iterInit->initState = 0;
					m_initStartWaitTime.erase(iter);
				}
			}
			else  //if(iterInit->initState == 1)//初始化成功
			{
				nCount++;
				LogError("设备%s 通道 %d 初始化成功！",iterInit->chDeviceID,iterInit->nCameraId);

				InitDownChannel temp;
				memcpy(temp.chDeviceID,iterInit->chDeviceID,32);
				temp.nChannelID = iterInit->nChannelID;
			
				pthread_mutex_lock(&m_Init_mutex);	//加锁
				m_lInitDownChannel.push_back(temp);
				pthread_mutex_unlock(&m_Init_mutex);//解锁
			}	
		}
		
		if(nCount >= m_lchannelInitInfo.size())
		{
			break;
		}

		sleep(1);
	}
}

//处理实时数据
void CHttpUpload::DealResult()
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
void CHttpUpload::DealHistoryResult()
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
void CHttpUpload::DiskManage()
{
	int nCount = 0;
	while(!g_bEndThread)
	{
		nCount++;
		//if(nCount == 10)
		{
			if(g_sysInfo.fDisk>=85)
			{
				DelDir();
				LogError("磁盘剩余不足，删除时间目录");
			}
			nCount = 0;
		}
		sleep(10);
	}
}

//添加一条数据
bool CHttpUpload::AddResult(std::string& strResult)
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
bool CHttpUpload::OnResult(std::string& result)
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
		bool resultflag = mvSendRecordToCS(result);
		if (resultflag)
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

//获取请求消息
string CHttpUpload::GetRequest(const string& strMsg,char *chDeviceID,char * sServerPath,char *sFileName ,char *tzFileName)
{
	RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
	MIMAX_HEADER* mHeader = (MIMAX_HEADER *)strMsg.c_str();
	UINT32 uCameraID = mHeader->uCameraID;
	//方向类型转换
	char chfxlx[3] = {0};
	switch(pPlate->uDirection)
	{
		case 1:memcpy(chfxlx,"01",3);break;
		case 2:memcpy(chfxlx,"02",3);break;
		case 3:memcpy(chfxlx,"03",3);break;
		case 4:memcpy(chfxlx,"04",3);break;
		default:break;
	}
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
		case CARNUM_BLUE: chPlateColor = '4';break;
		case CARNUM_BLACK: chPlateColor = '1';break;
		case CARNUM_YELLOW: chPlateColor = '2';break;
		case CARNUM_WHITE: chPlateColor = '3';break;
		default: chPlateColor = '5';break;
	}

	//车牌号码
	string strCarNum(pPlate->chText);
	if (!strCarNum.compare("00000000") || !strCarNum.compare("11111111"))
	{
		strCarNum = string("无牌车");//不能识别开始可用空表示
	}

	//经过时间
	long tTime = pPlate->uTime;
	struct tm timenow;
	struct tm * newTime = &timenow;
	localtime_r(&tTime,newTime );
	char passTime[48];
	sprintf(passTime,"%d-%d-%d %d:%d:%2d",newTime->tm_year + 1900,newTime->tm_mon +1,newTime->tm_mday,newTime->tm_hour,newTime->tm_min,newTime->tm_sec);
	//违法类型
	string strViolationType = GetViolationType(pPlate->uViolationType);


	char pRequest[10240] = {0};
	//1.请求行
	memset(pRequest, 0, 10240);
	sprintf(pRequest,
"POST /WebServiceNC/services/WriteVehicleInfoService?wsdl HTTP/1.0\r\n\
Content-Type: text/xml; charset=utf-8\r\n\
Accept: application/soap+xml, application/dime, multipart/related, text/*\r\n\
User-Agent: Axis/1.4\r\n\
Host: %s:%d\r\n",g_strControlServerHost.c_str(),g_nControlServerPort);
	string strRequest1(pRequest);
    //3.请求正文
	memset(pRequest, 0, 10240);
	sprintf(pRequest,
"<?xml version=\"1.0\" encoding=\"UTF-8\"?><soapenv:Envelope \
xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" \
xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\
<soapenv:Body>\
<ns1:WriteVheicleInfo soapenv:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" \
xmlns:ns1=\"com.enjoyor.webservice\">\
<kkbh xsi:type=\"xsd:string\">%s</kkbh>\
<fxbh xsi:type=\"xsd:string\">%s</fxbh>\
<cdh xsi:type=\"xsd:string\">%d</cdh>\
<hphm xsi:type=\"xsd:string\">%s</hphm>\
<hpzl xsi:type=\"xsd:string\">%s</hpzl>\
<gcsj xsi:type=\"xsd:string\">%s</gcsj>\
<clsd xsi:type=\"xsd:string\">%d</clsd>\
<clxs xsi:type=\"xsd:string\">%d</clxs>\
<wfdm xsi:type=\"xsd:string\">%s</wfdm>\
<cwkc xsi:type=\"xsd:string\"></cwkc>\
<hpys xsi:type=\"xsd:string\">%c</hpys>\
<cllx xsi:type=\"xsd:string\">%s</cllx>\
<fzhpzl xsi:type=\"xsd:string\"></fzhpzl>\
<fzhphm xsi:type=\"xsd:string\"></fzhphm>\
<fzhpys xsi:type=\"xsd:string\"></fzhpys>\
<clpp xsi:type=\"xsd:string\"></clpp>\
<clwx xsi:type=\"xsd:string\"></clwx>\
<csys xsi:type=\"xsd:string\">%c</csys>\
<tplj xsi:type=\"xsd:string\">%s</tplj>\
<tp1 xsi:type=\"xsd:string\">%s</tp1>\
<tp2 xsi:type=\"xsd:string\"></tp2>\
<tp3 xsi:type=\"xsd:string\"></tp3>\
<tztp xsi:type=\"xsd:string\">%s</tztp>\
</ns1:WriteVheicleInfo>\
</soapenv:Body>\
</soapenv:Envelope>",
	 /*卡口编号  方向类型  车道号         号牌号码         车牌类型       经过时间     车辆速度       车辆限速  */
	 chDeviceID,chfxlx,pPlate->uRoadWayID,strCarNum.c_str(),chPlateType,passTime,pPlate->uSpeed,pPlate->uLimitSpeed,
	/*违法类型              号牌颜色    车辆类型    车身颜色 通行图片路径 通行图片名 特征图片名*/
	strViolationType.c_str(),chPlateColor,chCarType,chColor,sServerPath,sFileName,tzFileName);
    string strRequest3(pRequest) ;

	//LogNormal("strCarNum.size=%d",strCarNum.size());
	//2.请求参数
	int nlength = strRequest3.size();
	////消息报头：请求报头允许客户端向服务器端传递请求的附加信息以及客户端自身的信息。
	memset(pRequest, 0, 10240);
	sprintf(pRequest,
"Cache-Control: no-cache\r\n\
Pragma: no-cache\r\n\
SOAPAction: \"\"\r\n\
Content-Length: %d\r\n\r\n" ,nlength);

	string strRequest2(pRequest);
	string strRequest = strRequest1 + strRequest2 + strRequest3;
	return strRequest;
}
/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CHttpUpload::mvSendRecordToCS(const string &strMsg)
{
	//加锁
	pthread_mutex_lock(&m_Send_Mutex);

	string strReturn = "";
   
    MIMAX_HEADER* mHeader = (MIMAX_HEADER *)strMsg.c_str();
    UINT32 uCameraID;

    if(MIMAX_EVENT_REP == mHeader->uCmdID)
    {
		pthread_mutex_unlock(&m_Send_Mutex);
		return true;
    }
    else if (MIMAX_PLATE_REP == mHeader->uCmdID)
    {
		RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
		
		char chDeviceID[32] = "";   //通道设置中的设备编号，卡口为卡口编号；电警为地点编号
		bool isInit = false;

		pthread_mutex_lock(&m_Init_mutex);	//加锁
		list<InitDownChannel>::iterator iterInit= m_lInitDownChannel.begin();
		for (;iterInit != m_lInitDownChannel.end();iterInit++)
		{
			if (iterInit->nChannelID == pPlate->uChannelID)
			{
				strcpy(chDeviceID,iterInit->chDeviceID);
				//LogError("通道信息 chDeviceID=%s,uRoadWayID=%d",iterInit->chDeviceID,pPlate->uRoadWayID);
				isInit = true;
				break;	
			}
		}

		pthread_mutex_unlock(&m_Init_mutex);	//解锁
		if (!isInit)
		{
			//LogError("通道信息未初始化\n");
			pthread_mutex_unlock(&m_Send_Mutex); //解锁
			return false; 
		}

		char sUrl[128] = "/upload-service/service/upload";
		char sServerPath[256] = "";  //远程图片路径
		char qjFileName[256] = "";   //卡口全景图片名 或违章图片名
		char tzFileName[256] = "";

		string strTime = GetTime(pPlate->uTime,2); //过车时间
		long tTime = pPlate->uTime;
		struct tm timenow;
		struct tm * newTime = &timenow;
		localtime_r(&tTime,newTime ); 

		struct timeval tv;
		gettimeofday(&tv,NULL);
		if ( (tv.tv_sec - m_TheTimeLog.tv_sec) > (2*60*60) ) //每2小时记录失败日志记录清零
		{
			m_sendErLog = 0;
			m_TheTimeLog = tv;
		}

		sprintf(sServerPath,"%04d/%02d/%02d/%s/",newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday,chDeviceID);

		if (pPlate->uViolationType == 0)    //南昌卡口图片发处理
		{
			//卡口图片名称                                                             
			sprintf(qjFileName,"%s%s.%03d-%02d.jpg",chDeviceID, strTime.c_str(),pPlate->uMiTime,pPlate->uRoadWayID);

			char localSmallpicPath[128] = "";
			memcpy(localSmallpicPath,pPlate->chPicPath,MAX_VIDEO);
			unsigned int strlength = strlen(pPlate->chPicPath);
			localSmallpicPath[strlength-5] = localSmallpicPath[strlength-5] +1;         //本地小图片路径

			unsigned int uServerPortForPic = 8100;  
			//发送图片 先发全景图，再发小图
			//return: 0(suc); -100(create socket failed); -101(connect failed); -102(file open err); -103(http status bad); -104(unexpected err)
			int revSendPic1 =  httpUpload((char*)g_strControlServerHost.c_str(),uServerPortForPic,sUrl,pPlate->chPicPath,qjFileName,sServerPath);
			if( revSendPic1 < 0)
			{
				if (m_sendErLog < 10) //发送图片失败是否dao
				{
					LogError("发送卡口全景图片失败pic1:%d\n",revSendPic1);
					m_sendErLog++;
				}
				pthread_mutex_unlock(&m_Send_Mutex);
				return false;
			}
			int revSendPic2 = -1;
			if(access(localSmallpicPath,F_OK) == 0)
			{
				int namelength = strlen(qjFileName);
				char strTmp[] = "_tz.jpg";
				strncpy(tzFileName,qjFileName,namelength-4);
				strcat(tzFileName,strTmp);                     //tzFileName:卡口远程特征图片名

				revSendPic2 = httpUpload((char*)g_strControlServerHost.c_str(),uServerPortForPic,sUrl,localSmallpicPath,tzFileName,sServerPath);
				if ( revSendPic2 < 0)
				{
					if (m_sendErLog < 10)
					{
						LogError("发送卡口特征图片失败pic2:%d\n",revSendPic2);
						m_sendErLog++;
					}
					pthread_mutex_unlock(&m_Send_Mutex);
					return false;
				}
			}
		}
		else   //南昌电警图片发送处理
		{			//违章图片名称                 /* 地点编号    方向首字母大写                           时间             毫秒数         车道号    */
			sprintf(qjFileName,"%s%c%s%03dS%d.jpg",chDeviceID,GetCapitalOfDirection(pPlate->uDirection),strTime.c_str(),pPlate->uMiTime,pPlate->uRoadWayID);		
			
			unsigned int uServerPortForPic = 8100; //违章图片发送端口号
			//发送违章图片
			//return: 0(suc); -100(create socket failed); -101(connect failed); -102(file open err); -103(http status bad); -104(unexpected err)
			int SendPicRet =  httpUpload((char*)g_strControlServerHost.c_str(),uServerPortForPic,sUrl,pPlate->chPicPath,qjFileName,sServerPath);
			if( SendPicRet < 0)
			{
				if (m_sendErLog < 10) //发送违章图片失败
				{
					LogError("发送违章图片失败pic:%d\n",SendPicRet);
					m_sendErLog++;
				}
				pthread_mutex_unlock(&m_Send_Mutex);
				return false;
			}
		}

		//准备发送记录;
		if (!mvConnectToCS())
		{
			if (m_sendErLog < 10)
			{
				LogError("发送记录，连接到中心端失败\n");
				m_sendErLog++;
			}
			pthread_mutex_unlock(&m_Send_Mutex);
			return false;
		}
		string strFullMsg = GetRequest(strMsg,chDeviceID,sServerPath,qjFileName,tzFileName);
		bool sendMsgFlag = mvSendMsgToSocket(m_nCenterSocket,strFullMsg);
		
		int sendResult = 0;   //发送返回结果
		unsigned int uRet = 0;//用作记录返回的数据大小
		if (!sendMsgFlag)
		{
			mvCloseSocket(m_nCenterSocket);
			if (m_sendErLog < 10)
			{
				LogError("xml记录发送失败，连接断开\n");
				m_sendErLog++;
			}
			pthread_mutex_unlock(&m_Send_Mutex);
			return false;
		}
		else
		{
			strReturn = "";
			sendResult = mvRecvMsg(strReturn,uRet);  //接受并保存返回消息
			mvCloseSocket(m_nCenterSocket);	
		}
		if (sendResult == 1)  
		{
			pthread_mutex_unlock(&m_Send_Mutex);
			return true;   //记录发送成功
		}
		else
		{
			if (m_sendErLog < 10)
			{
				LogError("xml上传失败,返回值：%d,数据大小：%d\n",sendResult,uRet);
				m_sendErLog++;
			}
			pthread_mutex_unlock(&m_Send_Mutex);
			return false;//记录发送失败
		}
	}
}

//获取违法行为编码
string CHttpUpload::GetViolationType(unsigned int uViolationType)
{
	char chViolationType[6];
	switch(uViolationType)
	{
	case DETECT_RESULT_ALL:  //卡口过车
		{
			sprintf(chViolationType,"%04d",0000);
			break;
		}
	case DETECT_NO_DIRECTION_TRAVEL:     //不按导向车道行驶 
	case DETECT_RESULT_FORBID_LEFT:      //禁止左拐
	case DETECT_RESULT_FORBID_RIGHT:      //禁止右拐      
	case DETECT_RESULT_FORBID_STRAIGHT:    // 禁止前行
		{
			sprintf(chViolationType,"%4d",7006);
			break;
		}
	case DETECT_RESULT_RETROGRADE_MOTION_FLAG://70违反禁令标志
	case DETECT_RESULT_EVENT_GO_AGAINST://车辆逆行	
	case DETECT_RESULT_RETROGRADE_MOTION:///逆行1a 
	case DETECT_RESULT_YELLOW_CAR:             //50 黄标车     闯禁行
	case DETECT_RESULT_BIG_IN_FORBIDDEN_TIME:  //20 大车出现在禁行时间(大货禁行)  闯禁行
	case DETECT_RESULT_NOT_LOCAL_CAR:          //45 非本地车    闯禁行
		{
			sprintf(chViolationType,"%4d",1344);
			break;
		}
	case DETECT_RESULT_EVENT_GO_CHANGE: //违章变道
	case DETECT_RESULT_ELE_EVT_BIANDAO: //变道
	case DETECT_RESULT_PRESS_LINE:     //27 压黄线1b
	case DETECT_RESULT_PRESS_WHITELINE://52 压白线
		{
			sprintf(chViolationType,"%4d",1208);
			break;
		}
	case DETECT_RESULT_NO_TURNAROUND://59 禁止调头 
		{  
			sprintf(chViolationType,"%4d",1044);
			break;
		}

	case DETECT_MASK_PLATE:	//67 车牌遮挡
		{
			sprintf(chViolationType,"%4d",1718);
			break;
		}
	case DETECT_RESULT_NOCARNUM:             //38无牌车出现
		{
			sprintf(chViolationType,"%4d",1717);
			break;
		}
	case DETECT_RESULT_RED_LIGHT_VIOLATION://16 闯红灯
		{
			sprintf(chViolationType,"%4d",1625);
			break;
		}
	case DETECT_RESULT_PARKING_VIOLATION:  //17 违章停车11
	case DETECT_RESULT_NO_STOP://57 禁停
		{
			sprintf(chViolationType,"%4dC",1309);
			break;
		}
	case DETECT_RESULT_TAKE_UP_NONMOTORWAY:  //55 机占非
		{
			sprintf(chViolationType,"%4d",1018);
			break;
		}
	case DETECT_RESULT_OBV_TAKE_UP_BUSWAY:  //32占用公交道20
		{
			sprintf(chViolationType,"%4d",1019);
			break;
		}
	default:
		{
			sprintf(chViolationType,"%04d",0000);
			break;
		}
	}
	string strViolationType(chViolationType);
	return strViolationType;
}

char CHttpUpload::GetCapitalOfDirection(unsigned int uDirection)
{
	char capitalOfDirection  = 0;
	switch(uDirection)
	{
	case 1:capitalOfDirection = 'E';break;
	case 2:capitalOfDirection = 'W';break;
	case 3:capitalOfDirection = 'S';break;
	case 4:capitalOfDirection = 'N';break;
	default:break;
	}
	return capitalOfDirection;
}

void CHttpUpload::GetChannelInfo()
{
	SRIP_CHANNEL * sChanel;
	string channelsStr = g_skpDB.GetChannelList();
	int size = channelsStr.size()/sizeof(SRIP_CHANNEL);
	for (int i = 0; i < size; i++)
	{
		ChannelInitInfo temp;
		sChanel = (SRIP_CHANNEL*)( channelsStr.c_str() + i * sizeof(SRIP_CHANNEL) );
		memcpy(temp.chDeviceID,sChanel->chDeviceID,32);
		temp.nCameraId = sChanel->nCameraId;
		temp.nChannelID = sChanel->uId;
		temp.nChanWayType = sChanel->nChanWayType;
		memcpy(temp.chCode,sChanel->chCode,48);
		m_lchannelInitInfo.push_back(temp);
		//LogError("通道信息 chDeviceID=%s,uRoadWayID=%d",temp.chDeviceID,temp.nCameraId);
	}
}

void CHttpUpload::InitHttpTrans(char chKakouID[32],int nChannelId,int nChanWayType,char chKey[48],int & initstate )
{
	char pRequest[10240] = {0};
	memset(pRequest, 0, 10240);
	sprintf(pRequest,
"POST /WebServiceNC/services/WriteVehicleInfoService?wsdl HTTP/1.0\r\n\
Content-Type: text/xml; charset=utf-8\r\n\
Accept: application/soap+xml, application/dime, multipart/related, text/*\r\n\
User-Agent: Axis/1.4\r\n\
Host: %s:%d\r\n",g_strControlServerHost.c_str(),g_nControlServerPort);
	string strRequest1(pRequest);

    //3.请求正文
	memset(pRequest, 0, 10240);
	sprintf(pRequest, 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?><soapenv:Envelope \
xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" \
xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" \
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\
<soapenv:Body>\
<ns1:InitTrans soapenv:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" \
xmlns:ns1=\"com.enjoyor.webservice\">\
<kkbh xsi:type=\"xsd:string\">%s</kkbh>\
<fxbh xsi:type=\"xsd:string\">%d</fxbh>\
<cdh xsi:type=\"xsd:string\">%d</cdh>\
<key xsi:type=\"xsd:string\">%s</key>\
</ns1:InitTrans>\
</soapenv:Body>\
</soapenv:Envelope>",chKakouID,nChanWayType,nChannelId,chKey);
	string strRequest3(pRequest);
	//2.请求参数
	int nlength = strRequest3.size();
	//消息报头：请求报头允许客户端向服务器端传递请求的附加信息以及客户端自身的信息。
	memset(pRequest, 0, 10240);
	sprintf(pRequest,
"Cache-Control: no-cache\r\n\
Pragma: no-cache\r\n\
SOAPAction: \"\"\r\n\
Content-Length: %d\r\n\r\n" ,nlength);
	string strRequest2(pRequest);

	string strInitHttpMsg = strRequest1 + strRequest2 + strRequest3;

	if (!mvConnectToCS())
	{
		//LogError("初始化连接到中心端失败\n");
		return;
	}
	string strReturn = "";
	
	if (!mvSendMsgToSocket(m_nCenterSocket, strInitHttpMsg))
	{
		strReturn = "";
		//mvRecvMsg(strReturn);
		mvCloseSocket(m_nCenterSocket);
		LogError("http初始化消息发送失败\n");
	}
	else
	{
		strReturn = "";
		unsigned int uRet = 0; 
		initstate = mvRecvMsg(strReturn,uRet); //根据返回值判断是否初始化成功 
		mvCloseSocket(m_nCenterSocket);

		if (initstate == -1)  //开始记录初始化等待时间
		{
			struct timeval tv;
			gettimeofday(&tv,NULL);
			struct tm *oldTime,timenow;
			oldTime = &timenow;
			localtime_r( &tv.tv_sec,oldTime);
			if (m_initStartWaitTime.find(nChannelId) == m_initStartWaitTime.end())
			{
				m_initStartWaitTime.insert(make_pair(nChannelId,timenow));
			}	
		}
	}
}


bool CHttpUpload::Init()
{
	//获取通道信息
	GetChannelInfo(); 
	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//启动注册车道线程	
	int nret=pthread_create(&m_nInitThreadId,&attr,ThreadRoadWayInit,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建启动注册车道线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}
	
    //启动检测结果发送线程
	nret=pthread_create(&m_nThreadId,&attr,ThreadHTTPUpLoadResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}
	
	//启动历史数据处理线程
	nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadHTTPUpLoadHistoryResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}	
	//启动磁盘管理线程
	nret=pthread_create(&m_nDiskManageThreadId,&attr,ThreadHTTPUpLoadDiskManage,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建磁盘管理线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}	
	pthread_attr_destroy(&attr);
	return true;
}

//释放
bool CHttpUpload::UnInit()
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
	if(m_nInitThreadId != 0)
	{
		pthread_join(m_nInitThreadId,NULL);
		m_nInitThreadId = 0;
	}
	if(m_nDiskManageThreadId != 0)
	{
		pthread_join(m_nDiskManageThreadId,NULL);
		m_nDiskManageThreadId = 0;
	}

	m_lchannelInitInfo.clear();
	m_lInitDownChannel.clear();
	m_initStartWaitTime.clear();

    m_ResultList.clear();
	return true;
}

bool CHttpUpload::mvConnectToCS()
{
	//connect to center server and set socket's option.
	if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
	{
		
		//LogNormal("\n中心数据服务器连接参数异常:host=%s,port=%d\n", g_strControlServerHost.c_str(), g_nControlServerPort);
		return false;
	}

	if (!mvPrepareSocket(m_nCenterSocket))
	{
		//LogNormal("\n准备连接中心数据服务器套接字失败!\n");
		return false;
	}

	if (!mvWaitConnect(m_nCenterSocket, g_strControlServerHost, g_nControlServerPort,5))
	{
		//LogNormal("\n尝试连接中心数据服务器失败!Host=%s,Port=%d\n",g_strControlServerHost.c_str(), g_nControlServerPort);
		return false;
	}
	return true;
}

int CHttpUpload::mvRecvMsg(string& strMsg,unsigned int & xmlRet)
{
	if (!strMsg.empty())
	{
		strMsg.clear();
	}
	if (m_nCenterSocket <= 0)
	{
		return 0; 
	}

	char chBuffer[2048] = {0};
	char chBufferTmp[SRIP_MAX_BUFFER] = {0}; 
	int sumRet = 0;
	int nRet = recv(m_nCenterSocket,chBuffer,SRIP_MAX_BUFFER, MSG_NOSIGNAL);
	while(nRet > 0)
	{
		sumRet += nRet;
		nRet = recv(m_nCenterSocket,chBufferTmp,SRIP_MAX_BUFFER, MSG_NOSIGNAL);
		if (nRet > 0)
		{
			strncat(chBuffer,chBufferTmp,nRet);
			memset(chBufferTmp,0,SRIP_MAX_BUFFER);
		}	
	}
	if (sumRet <= 0)
	{
		xmlRet = sumRet;
		return 0;
	}
	string strBuffer(chBuffer);
	int initflag = 0;
	if(!strBuffer.empty())
	{
		char* tmpMessage = (char*)strstr(strBuffer.c_str(),"<return>");

		if(tmpMessage != NULL)
		{
			tmpMessage = tmpMessage + 8;
			char chReturn = *tmpMessage;
			if (chReturn == '1')
			{
				initflag = 1;     //注册或发送记录成功
			}
			else if (chReturn == '0')
			{
				initflag = 0;  //注册或发送记录失败
				xmlRet = sumRet;
			}
			else
			{
				initflag = -1;  //注册时间间隔于10分钟
			}
		}
	}
	return initflag;
}

//删除时间目录
void CHttpUpload::DelDir()
{
	char buf[1024] = {0};
	int nMinID = 0;
	int nType = 0;
	std::string strDelPathName("");

	for(int i = 0;i<2;i++)
	{
		string strFtpDir;

		if(i == 0)
	    {
		    strFtpDir = "/home/road/server/video";
		    if (IsDataDisk())
			{
				strFtpDir = "/detectdata/video";
			}

	    }
		else
		{
			strFtpDir = "/home/road/server/pic";
			if (IsDataDisk())
			{
				strFtpDir = "/detectdata/pic";
			}
		}

		if(strFtpDir.size() > 0)
		{
			std::string strPathName2 = strFtpDir;

			 unsigned int uCurrentTime = GetTimeStamp();
			unsigned int uDateTime = g_uDiskDay*DATE_TIME;   
			if(i == 0)
			{
				uDateTime = 8*DATE_TIME;                  //南昌电警全天视频保存8天
			}
			unsigned int uTime  = uCurrentTime - uDateTime;
						
			String strTimeNow = "";
			int nTimeNow = 0;
			if(uTime > 0)
			{
				strTimeNow = GetTime(uTime,13);
				nTimeNow = atoi(strTimeNow.c_str());
			}
			 LogNormal("nTimeNow=%d,i=%d\n",nTimeNow,i);

			IntMapDir intMapDir;

			DIR* dir=NULL;
			struct dirent* ptr=NULL;

			dir = opendir(strPathName2.c_str());

			if(dir)
			{
				int nID = 0;

				int nCountID = 0;

				while((ptr=readdir(dir))!=NULL)
				{
					if((strcmp(".",ptr->d_name)!=0)&&
						(strcmp("..",ptr->d_name)!=0))
					{
						std::string strPathName(ptr->d_name);

						printf("***************strPathName.c_str()=%s\n",strPathName.c_str());
						if(strPathName.size() == 8)
						{
							memset(buf,0,1024);
							sprintf(buf,"rm -rf %s/%s",strPathName2.c_str(),strPathName.c_str());
							system(buf);
						}
						else if( strPathName.size() == 10)
						{
							nID = atoi(strPathName.c_str());

							if((nID < nMinID) || (nMinID == 0))
							{
								nMinID = nID;
								nType = i;

								sprintf(buf,"%s/%d",strPathName2.c_str(),nMinID);
								std::string strPathName(buf);

								strDelPathName = strPathName;
							}

							if(nID <= nTimeNow)//删除30天前的数据
							{
								 //LogNormal("nID=%d\n",nID);
								intMapDir.insert(IntMapDir::value_type(nID, nID));
							}
						}
					}
				}
				closedir(dir);

				 LogNormal("intMapDir.size=%d\n",intMapDir.size());
				 ////////
				IntMapDir::iterator it = intMapDir.begin();
				while(it != intMapDir.end())
				{
						int nID = it->first;
						sprintf(buf,"%s/%d",strPathName2.c_str(),nID);
						std::string strPathName(buf);

						//LogNormal("%d-%s\n",i,strPathName.c_str());

						RemoveDir(strPathName.c_str(),i);//删掉日期目录

						it++;
				}
			}
		}
	}

	  //删除日期最老的目录
	   if(strDelPathName.size() > 0)
	   {
		   LogNormal("%d-%s\n",nType,strDelPathName.c_str());
		   RemoveDir(strDelPathName.c_str(),nType);//删掉日期目录
	   }
}

//递归删除目录
bool CHttpUpload::RemoveDir( const char* dirname,int nType)
{
	DIR* dir=NULL;
	struct dirent* ptr=NULL;
	dir = opendir(dirname);

	//LogNormal("dirname=%s\n",dirname);

	// 初始化为0
	char buf[1024] = {0};

	if(dir)
	{
		while((ptr=readdir(dir))!=NULL)
		{
			if((strcmp(".",ptr->d_name)!=0)&&
				(strcmp("..",ptr->d_name)!=0))
			{
				sprintf(buf,"%s/%s",dirname,ptr->d_name);
				std::string strPath(buf);
				//LogNormal("strPath=%s\n",strPath.c_str());

				//判断是否为目录
				struct stat st;
				stat(strPath.c_str(),&st);
				int n = S_ISDIR(st.st_mode);

				if(n==1)//目录
				{
					RemoveDir(strPath.c_str(),nType);
				}
				else if(n==0) //文件
				{
					//umount(strPath.c_str());
					remove(strPath.c_str());
					//删除已经存在的记录
					if(nType == 1)
					{
						g_skpDB.DeleteOldRecord(strPath,false,false);
					}
					else if(nType == 0)
					{
						g_skpDB.DeleteOldRecord(strPath,false,true);
					}
					usleep(1000*200);
				}
			}
		}
		closedir(dir);
	}

	if(dirname)
		rmdir(dirname);

	return true;
}

//将接收到响应消息存到文件
void CHttpUpload::SaveMessageToFile(string strTmp)
{
	strTmp += "\r\n\r\n";
	//pthread_mutex_lock(&m_file_mutex);
	//LogNormal("in SaveMessageToFile 111111\n");
	FILE* fp = fopen("messageReturn.txt","a");
	if(fp == NULL)
	{
		cerr<<"open messageReturn.txt error"<<endl;
		//pthread_mutex_unlock(&m_file_mutex);
		return;
	}
	fwrite(strTmp.c_str(),strTmp.size(),1,fp);
	fclose(fp);
	//pthread_mutex_unlock(&m_file_mutex);
	return;
}
