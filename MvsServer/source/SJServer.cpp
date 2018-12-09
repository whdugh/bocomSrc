#include "Common.h"
#include "CommonHeader.h"
#include "SJServer.h"
#include "FtpCommunication.h"
#include "ximage.h"
CSJServer g_SJServer;

//构造
CSJServer::CSJServer()
{
	m_nThreadId = 0;
	m_nHistoryThreadId = 0;
	m_ResultList.clear();
	pthread_mutex_init(&m_Result_Mutex,NULL);
	//m_FtpConnectedFlag = false;
}
//析构
CSJServer::~CSJServer()
{
	 pthread_mutex_destroy(&m_Result_Mutex);
}

////记录发送线程
//void* ThreadConnectFtp(void* pArg)
//{
//	//处理一条数据
//	while (!g_bEndThread)
//	{
//		g_SJServer.ConnectFtpServer();
//		sleep(5);
//	}
//	pthread_exit((void *)0);
//	return pArg;
//}

//实时记录发送线程
void* ThreadSJResult(void* pArg)
{
	//处理一条数据
	g_SJServer.DealResult();

	pthread_exit((void *)0);
	return pArg;
}

//实时记录发送线程
void* ThreadSJHistoryResult(void* pArg)
{
	g_SJServer.mvDealHistoryRecord();

	pthread_exit((void *)0);
}


/*
* 函数介绍：获取一条历史记录
* 输入参数：strMsg-要获取的历史记录存储变量
* 输出参数：strMsg-获取的历史记录
* 返回值 ：成功返回true，否则返回false
*/
void CSJServer::mvDealHistoryRecord()
{
	//取类指针
	while(!g_bEndThread)
	{

		//发送历史记录
		if(g_nSendHistoryRecord == 1)
		{
			mvOrdOneRecord();
		}
		usleep(1000*1000);
	}
}

//获取历史记录
void CSJServer::mvOrdOneRecord()
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

//启动侦听服务
bool CSJServer::Init()
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

	////启动检测结果发送线程
	//int nret=pthread_create(&m_nThreadId,&attr,ThreadConnectFtp,this);
	////成功
	//if(nret!=0)
	//{
	//	//失败
	//	LogError("CSJServer::Init()创建连接ftp线程失败,服务无法启动!\r\n");
	//	g_bEndThread = true;
	//	return false;
	//}
	int nret=pthread_create(&m_nThreadId,&attr,ThreadSJResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("CSJServer::Init()创建检测结果发送线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadSJHistoryResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("CSJServer::Init()创建历史结果发送线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);

	return true;
}

//释放
bool CSJServer::UnInit()
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

	//g_FtpCommunication.DoClose();
	m_ResultList.clear();
	return true;
}


//添加一条数据
bool CSJServer::AddResult(std::string& strResult)
{
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strResult.c_str();

	switch(sDetectHeader->uDetectType)
	{
	case MIMAX_EVENT_REP:	//事件(送中心端)
	//case MIMAX_PLATE_REP:   //车牌
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

string CSJServer::CarTypeConvert(UINT32 nCarType)
{
	string strReturn("");

	if (nCarType == SMALL_CAR)
	{
		//LogNormal("aaaa:nCarType:%d\n",nCarType);
		strReturn = "小型汽车";
	}
	else if (nCarType == MIDDLE_CAR)
	{
		strReturn = "中型汽车";
	}
	else if (nCarType == BIG_CAR)
	{
		strReturn = "大型汽车";
	}
	else if (nCarType == BUS_TYPE)
	{
		strReturn = "大巴";
	}
	else if (nCarType == TRUCK_TYPE)
	{
		strReturn = "卡车";
	}
	else if (nCarType == MIDDLEBUS_TYPE)
	{
		strReturn = "中巴";
	}
	else if (nCarType == MINI_TRUCK)
	{
		strReturn = "小型货车";
	}
	else if (nCarType == TAXI)
	{
		strReturn = "轿车";
	}
	else
	{
		LogError("CSJServer::CarTypeConvert():otherType:%d\n",nCarType);
	}
	return strReturn;
}

//通过ftp发送数据
bool CSJServer::SendDataByFtp(const string& strMsg,string& strRemotePath)
{
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
    {
        return false;
    }

    //if(ConnectFtpServer())
    {
        string strLocalPath;

        char filepath[255]={0};
		char FileTextBuf[1024] = {0};

        MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();

        int nCameraID = sHeader->uCameraID;
        long uTime = 0;
        UINT32 uMiTime = 0;
		UINT32 uID = 0;
		UINT32 uChannelID = 0;
		UINT32 uLimitSpeed = 0;
		UINT32 uSpeed = 0;
		string strCarnum("");
		string strCodeH("");
		string strCodeT("");
		string strCarType("");
		bool bRet = false;

		//cerr<<"1111111111"<<endl;
        if (MIMAX_EVENT_REP == sHeader->uCmdID)
        {
            RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
            strLocalPath = pEvent->chPicPath;
            uTime = pEvent->uEventBeginTime;
            uMiTime = pEvent->uMiEventBeginTime;
			uID = pEvent->uSeq;
			uChannelID = pEvent->uChannelID;
			
			uLimitSpeed = GetMaxSpeed(pEvent->uType, pEvent->uChannelID,pEvent->uRoadWayID);
			uSpeed = pEvent->uSpeed;

			////test
			/*memcpy(pEvent->chText,"沪DQ0023",8);
			pEvent->uType = 1;*/
			/*pEvent->chText = "沪DQ0023";*/
			////test
			//cerr<<"AAAAAAAAAAAA chText:"<<pEvent->chText<<endl;
			strCarnum = pEvent->chText;
			if (strCarnum.size() > 2)
			{
				g_skpDB.UTF8ToGBK(strCarnum);
				//cerr<<"BBBBBBBBBB strCarnum:"<<strCarnum<<endl;
				strCodeH = strCarnum.substr(0,2);
				//cerr<<"CCCCCCCCCCCC strCodeH:"<<strCodeH<<endl;
				strCodeT = strCarnum.substr(2,strCarnum.size()-2);
				//cerr<<"DDDDDDDDDDDD strCodeT:"<<strCodeT<<endl;
			}
		
			strCarType = CarTypeConvert(pEvent->uType);

			//cerr<<"strCarType:"<<strCarType<<endl;


        }
		//cerr<<"22222222222222222"<<endl;
       /* else if (MIMAX_PLATE_REP == sHeader->uCmdID)
        {
            RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
            strLocalPath = pPlate->chPicPath;
            uTime = pPlate->uTime;
            uMiTime = pPlate->uMiTime;
			uID = pPlate->uSeq;
			uChannelID = pPlate->uChannelID;
			uLimitSpeed = GetSpeed(pPlate->uChannelID,pPlate->uRoadWayID);
			uSpeed = pPlate->uSpeed;

			strCarnum = pPlate->chText;
			strCodeH = strCarnum.substr(0,2);
			strCodeT = strCarnum.substr(2,strCarnum.size()-2);

			strCarType = CarTypeConvert(pPlate->uType);

        }*/

		string strTimeInFile("");
		string strTime("");
		string strTmpTime("");
		string strPlace("");
		if (uTime > 0)
		{
			//cerr<<"3333333333333"<<endl;
			strTimeInFile = GetTime(uTime,0);
			strTime = GetTime(uTime,2);
			strTmpTime = strTime.substr(2,strTime.size()-2);
			strPlace = g_skpDB.GetPlace(uChannelID);
			g_skpDB.UTF8ToGBK(strPlace);
			//cerr<<"444444444444444444"<<endl;
		}
		else
		{
			return false;
		}
		if (strCarType.size() == 0)
		{
			//return false;
		}
		
		//cerr<<"5555555555555"<<endl;
		//先发送图片
		sprintf(filepath,"%s_%s_%d_",g_strDetectorID.c_str(),strTmpTime.c_str(),uID);
		int i = 0;
		char buf[12] = {0};
		for (;i<4;i++)
		{
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%d.jpg",i);
			strRemotePath.clear();
			strRemotePath += filepath;
			strRemotePath += buf;
			string strData("");
			strData = GetSingleImageByPath(strLocalPath,i);
			if (strData.size() <= 0)
			{
				cerr<<"GetSingleImageByPath Get Nothing strLocalPath:"<<strLocalPath<<" i:"<<i<<endl;
				return false;
			}
			//cerr<<"ssssssstrRemotePath:"<<strRemotePath.c_str()<<endl;
			bRet = g_FtpCommunication.VideoDoPut(NULL,(char*)strRemotePath.c_str(),strData,true,true,(char*)strRemotePath.c_str(),false);
			if (!bRet)
			{
				//cerr<<"SendDataByFtp error:ID:"<<uID<<" i:"<<i<<" strLocalPath:"<<strLocalPath.c_str()<<endl;
				LogError("SendDataByFtp error:ID:%d,i:%d\n",uID,i);
				LogError("strLocalPath:%s\n",strLocalPath.c_str());
				return false;
			}

		}
		//cerr<<"8888888888888888"<<endl;
		//再先发送.ini文件
			sprintf(filepath,"%s_%s_%d.ini",g_strDetectorID.c_str(),strTmpTime.c_str(),uID);
			strRemotePath.clear();
			strRemotePath += filepath;
			snprintf(FileTextBuf,1024,
				"[TDRADAR]\r\n"
				"TYPE=1\r\n"
				"DATE=%s\r\n"
				"ADDR=%s\r\n"
				"LSPEED=0\r\n"
				"CSPEED=0\r\n"
				/*"LSPEED=%d\r\n"
				"CSPEED=%d\r\n"*/
				"CODE_H=%s\r\n"
				"CODE_T=%s\r\n"
				"CARTYPE=%s\r\n"
				"TOTALFILE=4\r\n"
				"FILE0=d:\\radarre\\%s_%s_%d_0.jpg\r\n"
				"FILE1=d:\\radarre\\%s_%s_%d_1.jpg\r\n"
				"FILE2=d:\\radarre\\%s_%s_%d_2.jpg\r\n"
				"FILE3=d:\\radarre\\%s_%s_%d_3.jpg\r\n"
				"BASEPATH=d:\\radarre\\%s_%s_%d.ini\r\n",strTimeInFile.c_str(),strPlace.c_str(),\
				/*uLimitSpeed,uSpeed,*/strCodeH.c_str(),strCodeT.c_str(),strCarType.c_str(),\
				g_strDetectorID.c_str(),strTmpTime.c_str(),uID,\
				g_strDetectorID.c_str(),strTmpTime.c_str(),uID,\
				g_strDetectorID.c_str(),strTmpTime.c_str(),uID,\
				g_strDetectorID.c_str(),strTmpTime.c_str(),uID,\
				g_strDetectorID.c_str(),strTmpTime.c_str(),uID
				);
			//cerr<<"666666666666666666666"<<endl;
			string strFileTex = FileTextBuf;
			//cerr<<"strRemotePath:"<<strRemotePath.c_str()<<endl;
			bRet = g_FtpCommunication.VideoDoPut(NULL,(char*)strRemotePath.c_str(),strFileTex,true,true,(char*)strRemotePath.c_str(),false);
			
		//}
		/*else if (nType == 1)
		{
		*/
			
		//}
   
		if(bRet)
        {
			//cerr<<"111111SSSSuccess"<<endl;
            return true;
        }
        else
        {
            LogError("\n====SendDataByFtp 通过ftp发送数据失败!\n");
            return false;
        }
    }

    return false;
}



bool CSJServer::mvSendRecordToCS(const string &strMsg)
{
	/*if(!m_FtpConnectedFlag)
	{
		return false;
	}*/

	MIMAX_HEADER* mHeader = (MIMAX_HEADER *)strMsg.c_str();

	bool ret = false;

	if(MIMAX_EVENT_REP == mHeader->uCmdID)
	{
		RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

		string strRemotePath;
		ret = SendDataByFtp(strMsg,strRemotePath);
		//   memset(pEvent->chPicPath,0,sizeof(pEvent->chPicPath));
		//   memcpy(pEvent->chPicPath,strRemotePath.c_str(),strRemotePath.size());


	}
	//else if (MIMAX_PLATE_REP == mHeader->uCmdID)
	//{
	//	RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

	//	string strRemotePath;
	//	ret = SendDataByFtp(strMsg,strRemotePath);
	//	//memset(pPlate->chPicPath,0,sizeof(pPlate->chPicPath));
	//	//memcpy(pPlate->chPicPath,strRemotePath.c_str(),strRemotePath.size());

	//}
	else
	{
		cerr<<"not Event Return,do nothing, return"<<endl;
	}

	return ret;
}



//处理检测结果
bool CSJServer::OnResult(std::string& result)
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
	//case MIMAX_STATISTIC_REP:  //统计
	//case PLATE_LOG_REP:  //日志
	//case EVENT_LOG_REP:
	//case MIMAX_PLATE_REP:  //车牌
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
	/*else if(mHeader.uCmdID == MIMAX_PLATE_REP)
	{
		RECORD_PLATE* sPlate = (RECORD_PLATE*)(result.c_str());
		String strPicPath(sPlate->chPicPath);
		result = result + GetImageByPath(strPicPath);
	}*/

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
			/*if(bObject)
			{
				g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP,uSeq);
			}
			else
			{*/
				g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
			//}
			
			return true;
		}
	}
	return false;
}

//处理实时数据
void CSJServer::DealResult()
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
			SJ_Result::iterator it = m_ResultList.begin();
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


////连接ftp服务器
//bool CSJServer::ConnectFtpServer()
//{
//	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
//	{
//		return false;
//	}
//	if (m_FtpConnectedFlag)
//	{
//		return true;
//	}
//	//Ftp连接套接字
//	int nFtpSocket = 0;
//	if (!mvPrepareSocket(nFtpSocket))
//	{
//		printf("\n准备连接ftp服务器套接字失败!\n");
//		mvCloseSocket(nFtpSocket);
//		return false;
//	}
//
//	if (!mvWaitConnect(nFtpSocket, g_strFtpServerHost,g_nFtpPort,3))
//	{
//		printf("ftp connect fail\n");
//		LogError("\n尝试连接ftp服务器失败!\n");
//		mvCloseSocket(nFtpSocket);
//		return false;
//	}
//	cerr<<"CSJServer::ConnectFtpServer() success"<<endl;
//	mvCloseSocket(nFtpSocket);
//	cerr<<"CSJServer::ConnectFtpServer() close nFtpSocket"<<endl;
//	m_FtpConnectedFlag = true;
//	return true;
//}


string CSJServer::GetSingleImageByPath(string& strPicPath,int& index)
{
	/*cerr<<"1111plate.chText:"<<plate.chText<<endl;
	string strTime = GetTime(plate.uTime,0);
	cerr<<"Time:"<<strTime<<endl;
	cerr<<"1111PicWidth:"<<plate.uPicWidth<<endl;
	cerr<<"111PicHeight:"<<plate.uPicHeight<<endl;*/
	string strPicData("");
	string strSinglePicData("");

	strPicData = GetImageByPath(strPicPath);

	if (strPicData.size() > 0)
	{
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
				return strSinglePicData;
			}
			cvSetData(pPicture,image.GetBits(),pPicture->widthStep);

			IplImage *pPictureSnap = NULL;
			pPictureSnap = cvCreateImage(cvSize(uPicWidth,uPicHeight),8,3);
			if (pPictureSnap == NULL)
			{
				LogError("pPictureSnap CreateImage error\n");
				return strSinglePicData;
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
			if (index == 0 || index == 2)
			{
				dstRt1.x = 0;
				dstRt1.y = (index/2)*(uPicHeight/2);
			}
			else if (index == 1 || index == 3)
			{
				dstRt1.x = uPicWidth/2;
				dstRt1.y = (index/2)*(uPicHeight/2);
			}
			
			dstRt1.height = uPicHeight/2;
			dstRt1.width = uPicWidth/2;
			unsigned char* pJpgImage1 = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
			UINT32 nPicSize1 = GetSmallImage(pPictureSnap,pJpgImage1,dstRt1);

		
			if (nPicSize1 > 0)
			{
				strSinglePicData.append((char*)pJpgImage1,nPicSize1);
			}
			
			
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

		}
		else
		{
			LogError("==GetComposedSingleImageByPath image Decode error\n");
		}
	}
	else
	{
		LogError("=====GetComposedSingleImageByPath PicData size is 0!");
		char buf[1024];
		memset(buf, 0, 1024);
		sprintf(buf, "delete from TRAFFIC_EVENT_INFO WHERE PICPATH = '%s';",strPicPath.c_str());
		g_skpDB.execSQL(string(buf));
	}
	return strSinglePicData;
}



