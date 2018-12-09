//#ifdef FBMATCHPLATE
#include "MatchPlateFortianjin.h"
#include "RoadImcData.h"
/*
#ifndef TEST_MATCH
	#define TEST_MATCH
#endif
*/
CMatchPlateFortianjin g_matchPlateFortianjin;

int CMatchPlateFortianjin::m_exit = 0;

CMatchPlateFortianjin::CMatchPlateFortianjin()
{
	pthread_mutex_init(&m_MatchList_Mutex,NULL);
	m_vcMatchList.clear();	

//#ifdef MATCH_LIU_YANG                                                                                                           
//	m_mvMatch2 = new MvFBMatch2();                                   
//#else
//	m_mvMatch = new MvFBMatch();
//#endif  

	m_pRecordTemp = NULL;
	
	for (int i = 0;i < 4 ; ++ i)
	{
		//m_pDataProc200W[i]= NULL;
		m_pDataProc500W[i] = NULL;
	}	
}

CMatchPlateFortianjin::~CMatchPlateFortianjin()
{
	CMatchPlateFortianjin::m_exit = 1;
	pthread_mutex_destroy(&m_MatchList_Mutex);	

//#ifdef MATCH_LIU_YANG
	/*if (m_mvMatch2)
	{
		delete m_mvMatch2;
		m_mvMatch2 = NULL;
	}*/
//#else
	//if (m_mvMatch)
	//{
	//	delete m_mvMatch;
	//	m_mvMatch = NULL;
	//}
//#endif
	
	//pool_destory();
	for (int i = 0;i < 4 ;++i)
	{
		/*if (m_pDataProc200W[i] != NULL)
		{
		delete m_pDataProc200W[i];
		m_pDataProc200W[i] = NULL;
		}*/
		if (m_pDataProc500W[i] != NULL)
		{
			delete m_pDataProc500W[i];
			m_pDataProc500W[i] = NULL;
		}
	}
}

int CMatchPlateFortianjin::InitMatchPlate()
{
	int nRet = 0;

	nRet = CheckMatchPlate();
	if (nRet != 0)
	{
		return nRet;
	}

	for (int i = 0;i < 4 ;++i)
	{
		/*if (m_pDataProc200W[i] == NULL)
		{
			m_pDataProc200W[i] = new CDspDataProcess(NULL);
			m_pDataProc200W[i]->Init(0,0,0,0,0);
		}*/
		if (m_pDataProc500W[i] == NULL)
		{
			m_pDataProc500W[i] = new CDspDataProcess(NULL);
			//m_pDataProc500W[i]->Init(0,0,0,0,0);
			m_pDataProc500W[i]->DoPopData();
		}
	}
	
	return nRet;
}

int CMatchPlateFortianjin::CheckMatchPlate()
{
	LogNormal("---CheckMatchPlate()---\n");
	pthread_t m_hId = 0;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	if(pthread_create(&m_hId, &attr, BeginCheckPlate, this) != 0)
	{
		printf("创建接收对比结果线程失败。");
		return -1;
	}
	pthread_attr_destroy(&attr);
	
	return 0;
}

//int CMatchPlateFortianjin::DealMatchPlate()
//{
//	LogNormal("---DealMatchPlate()---\n");
//	pthread_t hId = 0;
//	pthread_attr_t attr;
//	pthread_attr_init(&attr);
//	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
//	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
//
//	if(pthread_create(&hId, &attr, BeginDealPlate, this) != 0)
//	{
//		//printf("创建合成图对比结果线程失败.");
//		return -1;
//	}
//	pthread_attr_destroy(&attr);
//
//	return 0;
//}

threadfunc_t STDPREFIX CMatchPlateFortianjin::BeginCheckPlate(threadparam_t lpParam)
{
	CMatchPlateFortianjin *pThis = (CMatchPlateFortianjin *)lpParam;
	if (pThis)
	{
		pThis->CheckPlate();
	}
	return NULL;
}

//threadfunc_t STDPREFIX CMatchPlateFortianjin::BeginDealPlate(threadparam_t lpParam)
//{
//	CMatchPlateFortianjin *pThis = (CMatchPlateFortianjin *)lpParam;
//	if (pThis)
//	{
//		pThis->DealPlate();
//	}
//	return NULL;
//}

void CMatchPlateFortianjin::CheckPlate()
{
	time_t startTime;
	time_t nowTime;
	time(&startTime);

	while(CMatchPlateFortianjin::m_exit == 0)
	{
		time(&nowTime);
		int offTime = (nowTime-startTime);
		//if(offTime >= 60*2)//每2分钟检查一次
		//if(offTime >= 15)//每15s钟检查一次//test
		//if(offTime >= 30)//每30s钟检查一次//test
		if(offTime >= 5)//每5s钟检查一次
		{
			vector<MatchPair> vcMatchPlate;

//#ifdef MATCH_LIU_YANG
			vcMatchPlate = g_MvFBMatch2.mvOutput();
//#else
//			{
//				vcMatchPlate = m_mvMatch->mvOutput();
//			}
//#endif
			if (vcMatchPlate.size() > 0)
			{
				vector<MatchPair>::iterator iterPro;
				for(iterPro = vcMatchPlate.begin(); iterPro != vcMatchPlate.end(); iterPro++)
				{
					vector<RECORD_PLATE_DSP_MATCH> vcMatchPlateB;
					vcMatchPlateB = ((MatchPair)*iterPro).vecB;
					if (vcMatchPlateB.size() > 0)
					{
						vector<RECORD_PLATE_DSP_MATCH>::iterator iterProB;

						pthread_mutex_lock(&m_MatchList_Mutex);
						for(iterProB = vcMatchPlateB.begin(); iterProB != vcMatchPlateB.end(); iterProB++)
						{
							MatchPlate matchPlate;
							matchPlate.A = ((MatchPair)*iterPro).A;
							matchPlate.B = (RECORD_PLATE_DSP_MATCH)(*iterProB);
							
							m_vcMatchList.push_back(matchPlate);
								
							if(m_vcMatchList.size() > 10)
							{
								LogError("m_vcMatchList.size = %d\n",m_vcMatchList.size());
							}
							
						}
						pthread_mutex_unlock(&m_MatchList_Mutex);
					}
				}
			}
			else
			{
				//LogNormal("没有找到前后车牌对比结果。\n");
			}
			startTime = nowTime;
		}
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
	pthread_exit((void*)0);
}

//void CMatchPlateFortianjin::DealPlate()
//{
//	while(CMatchPlateFortianjin::m_exit == 0)
//	{
//		if (m_vcMatchList.size() > 0)
//		{
//			pthread_mutex_lock(&m_MatchList_Mutex);
//			vector<MatchPlate>::iterator iterPro = m_vcMatchList.begin();
//			MatchPlate matchPlate = (MatchPlate)*iterPro;		
//			m_vcMatchList.erase(iterPro);		
//			pthread_mutex_unlock(&m_MatchList_Mutex);
//			//DoDealPlateFunc(matchPlate.A,matchPlate.B); //test ok
//			sMatchPlateData *pMatchData = new sMatchPlateData;
//			pMatchData->param = this;
//			pMatchData->A = matchPlate.A;
//			pMatchData->B = matchPlate.B;
//			pool_add_worker(DoDealPlate,(void*)pMatchData);
//
//		}
//		RECORD_PLATE_DSP_MATCH plate;
//		if (m_mvMatch2->PopPlateNoMatch(plate))
//		{
//			//DoDealNoMatchPlate(plate);
//			//DoDealNoMatchPlateFunc(plate); //test 
//			sNoMatchPlateData *pNoMatchData = new sNoMatchPlateData;
//			pNoMatchData->param = this;
//			pNoMatchData->plate = plate;
//			pool_add_worker(DoDealNoMatchPlate,(void *)pNoMatchData);
//
//		}
//		usleep(1000*10);
//	}
//	pthread_exit((void*)0);
//}

//void CMatchPlateFortianjin::DoDealPlateFunc(RECORD_PLATE_DSP_MATCH &A, RECORD_PLATE_DSP_MATCH &B)
//{
//	//LogNormal("DoDealPlate A:%d, %lld w:%d h:%d \n", A.dspRecord.uChannelID, A.dspRecord.uTime, A.dspRecord.uPicWidth, A.dspRecord.uPicHeight);
//	//LogNormal("DoDealPlate B:%d, %lld w:%d h:%d \n", B.dspRecord.uChannelID, B.dspRecord.uTime, B.dspRecord.uPicWidth, B.dspRecord.uPicHeight);
//	//printf("A.dspRecord.uPicWidth=%d,B.dspRecord.uPicWidth=%d\n",A.dspRecord.uPicWidth,B.dspRecord.uPicWidth);
//	//printf("A.dspRecord.uPicHeight=%d,B.dspRecord.uPicHeight=%d\n",A.dspRecord.uPicHeight,B.dspRecord.uPicHeight);
//	//printf("A.dspRecord.uDetectDirection=%d,B.dspRecord.uDetectDirection=%d\n",A.dspRecord.uDetectDirection,B.dspRecord.uDetectDirection);
//	//客户端设定时间端输出
//	bool bDetect = CheckDetectTime(); 
//	if (bDetect == false) 
//	{
//		return; 
//	}
//
//	RECORD_PLATE plate;
//
//#ifdef TEMP_VIDEO_ON
//	if(m_pRecordTemp)
//	{
//		//输出违章前后牌匹配录像
//		std::string strVideoPath = DealOutPutMachVideo(A.dspRecord, B.dspRecord);
//		//memcpy(A.dspRecord.chVideoPath,strVideoPath.c_str(),strVideoPath.size());//事件录象路径
//		memcpy(plate.chVideoPath,strVideoPath.c_str(),strVideoPath.size());//事件录象路径
//		//LogNormal("th:%s \n", strVideoPath.c_str());
//	}
//#endif
//
////#ifdef MATCH_LIU_YANG
//	//27:浏阳比武测试项目
//	//if(g_nServerType == 27)
//	{
//		//LogNormal("A.dspRecord.uPicWidth:%d height:%d", \
//		//	A.dspRecord.uPicWidth, A.dspRecord.uPicHeight);
//
//		UINT32 uWidth = A.dspRecord.uPicWidth;
//		UINT32 uHeight = A.dspRecord.uPicHeight;
//
//		if(uWidth > 2000 && uWidth < 10000)
//		{
//			//if(NULL == m_pDataProc500W)
//			{
//				m_pDataProc500W = new CDspDataProcess(NULL);
//				m_pDataProc500W->Init(A.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
//			}
//		#ifdef MATCH_LIU_YANG_DEBUG	
//			m_pDataProc500W->OutPutVtsMatch2x2(A, B, plate);
//		#else
//			//LogNormal("[%s]uViolationType = %d\n car:%s",__FUNCTION__, A.dspRecord.uViolationType, A.dspRecord.chText);			
//			if (0 == A.dspRecord.uViolationType)  //卡口
//			{
//				m_pDataProc500W->OutPutVtsMatch1x3(A, B, plate);
//			}
//			else
//			{
//				m_pDataProc500W->OutPutVtsMatch2x3(A, B, plate);
//			}
//			
//		#endif
//		}
//		else if(uWidth > 0 && uWidth < 2000)
//		{
//			if(NULL == m_pDataProc200W)
//			{
//				m_pDataProc200W = new CDspDataProcess(NULL);
//				m_pDataProc200W->Init(A.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
//			}
//		#ifdef MATCH_LIU_YANG_DEBUG
//			m_pDataProc200W->OutPutVtsMatch2x2(A, B, plate);
//		#else
//			m_pDataProc200W->OutPutVtsMatch2x3(A, B, plate);
//		#endif
//		}
//		else{}
//
//		return;
//	}
////#endif //MATCH_LIU_YANG
//
//	//记录合成
//	if (A.dspRecord.uPicWidth == DSP_500_BIG_WIDTH_SERVER)// && record_plate.dspRecord.uPicHeight == (DSP_500_BIG_HEIGHT_SERVER + 80))
//	{
//		if(NULL == m_pDataProc500W)
//		{
//			m_pDataProc500W = new CDspDataProcess(NULL);
//			m_pDataProc500W->Init(1,DSP_500_BIG_WIDTH_SERVER,DSP_500_BIG_HEIGHT_SERVER,DSP_500_BIG_WIDTH_SERVER,DSP_500_BIG_HEIGHT_SERVER);
//		}
//		//m_pDataProc500W->SaveImgTest(record_plate.pImg,record_plate.dspRecord.chText);
//		//m_pDataProc500W->SaveImgTest(foundRecord.pImg,foundRecord.dspRecord.chText);
//		
//		printf("-------m_pDataProc->OutPutVtsMatchForTianJin-----1111----\n");
//		m_pDataProc500W->OutPutVtsMatchForTianJin(A, B, plate);
//		printf("-------m_pDataProc->OutPutVtsMatchForTianJin-----2222----\n");
//			
//	}
//	else if (A.dspRecord.uPicWidth == DSP_200_BIG_WIDTH_SERVER)// && record_plate.dspRecord.uPicHeight == (DSP_200_BIG_HEIGHT_SERVER + 80))
//	{
//		if(NULL == m_pDataProc200W)
//		{
//			m_pDataProc200W = new CDspDataProcess(NULL);
//			m_pDataProc200W->Init(1,DSP_200_BIG_WIDTH_SERVER,DSP_200_BIG_HEIGHT_SERVER,DSP_200_BIG_WIDTH_SERVER,DSP_200_BIG_HEIGHT_SERVER);
//		}
//		printf("-------m_pDataProc->OutPutVtsMatchForTianJin-----1111----\n");
//		m_pDataProc200W->OutPutVtsMatchForTianJin(A, B,plate);
//		printf("-------m_pDataProc->OutPutVtsMatchForTianJin-----2222----\n");
//	}
//	else if(A.dspRecord.uPicWidth == DSP_200_BIG_WIDTH)
//	{
//		if(NULL == m_pDataProc200W)
//		{
//			m_pDataProc200W = new CDspDataProcess(NULL);
//			m_pDataProc200W->Init(1,DSP_200_BIG_WIDTH,DSP_200_BIG_HEIGHT,DSP_200_BIG_WIDTH,DSP_200_BIG_HEIGHT);
//		}
//		//LogNormal("-------m_pDataProc->OutPutVtsMatchForTianJin-----1111----\n");
//		m_pDataProc200W->OutPutVtsMatchForTianJin(A, B,plate);
//		//LogNormal("-------m_pDataProc->OutPutVtsMatchForTianJin-----2222----\n");
//	}
//
//
//	//SendResult(plate,1);	
//}

// 输入卡口数据给对比处理
void CMatchPlateFortianjin::mvInput(RECORD_PLATE_DSP_MATCH &plate)
{
//#ifdef MATCH_LIU_YANG
	//if(27 == g_nServerType)
	{
		//if (m_mvMatch2)
		{
//#ifdef TEST_MATCH
			//LogNormal("mvInput2[%s]\n",plate.dspRecord.chText);
			//LogNormal("uSeq:%d pImg:%x, Arr[]:%x, %x, %x", \
			//	plate.dspRecord.uSeq, plate.pImg, plate.pImgArray[0], plate.pImgArray[1], plate.pImgArray[2]);
//#endif
			g_MvFBMatch2.mvInput(plate);
		}
	}
	//else
//#else
//	{
//		if (m_mvMatch)
//		{
//			LogNormal("mvInput[%s]\n",plate.dspRecord.chText);
//			m_mvMatch->mvInput(plate);
//
//	#ifdef TEST_MATCH
//			//LogNormal("111 mvInput 11");	
//
//			if(plate.dspRecord.chText[0] != '*')
//			{
//				plate.dspRecord.uTime += 10;
//				RECORD_PLATE_DSP_MATCH plateB;
//				memcpy((char*)(&plateB.dspRecord), (char*)(&plate.dspRecord), sizeof(plate.dspRecord));
//				plateB.pImg = plate.pImg;
//
//				plateB.dspRecord.uChannelID = plate.dspRecord.uChannelID + 1;
//				//plate.dspRecord.uTime = 1395036141;//2014-3-17 14:01:01
//				//plateB.dspRecord.uTime = 1395036141 + 10;
//				plateB.dspRecord.uTime = plate.dspRecord.uTime + 30;
//
//				plateB.dspRecord.uPosLeft = plateB.dspRecord.uPosLeft + 200;
//				plateB.dspRecord.uPosRight =  plateB.dspRecord.uPosRight + 200;
//			
//				this->DoDealPlate(plate, plateB);
//			}		
//	#endif
//		}
//		/*MatchPlate matchPlate;
//		matchPlate.A = plate;
//		matchPlate.B = plate;
//		pthread_mutex_lock(&m_MatchList_Mutex);
//		m_vcMatchList.push_back(matchPlate);
//		pthread_mutex_unlock(&m_MatchList_Mutex);*/
//	}//End of else
//#endif //End of else #MATCH_LIU_YANG
}


//输出违章前后牌匹配录像,返回录像路径
std::string CMatchPlateFortianjin::DealOutPutMachVideo(RECORD_PLATE &plateA, RECORD_PLATE &plateB)
{
	std::string strVideoPath = ""; //X-M
	std::string strVideoPath1 = "";
	std::string strVideoPath2 = "";
	if(m_pRecordTemp)
	{
		std::string strEventA = "";
		std::string strEventB = "";

		//更新数据库,录像路径
		plateA.uDetectDirection = 0;
		strVideoPath1 = GetVideoSavePath(plateA);//X-M-0.avi

		plateB.uDetectDirection = 1;
		strVideoPath2 = GetVideoSavePath(plateB);//X-M-1.avi

		strVideoPath = plateA.chVideoPath;
		
		//LogTrace("FBMach.log","strVideoPath:%s ", strVideoPath.c_str());
		//LogTrace("FBMach.log","strVideoPath1:%s ", strVideoPath1.c_str());
		//LogTrace("FBMach.log","strVideoPath2:%s ", strVideoPath2.c_str());

		//构造前后牌不匹配事件
		SetMatchEvent(strVideoPath1, plateA, false, strEventA);
		SetMatchEvent(strVideoPath2, plateB, true, strEventB);
		m_pRecordTemp->AddEvent(strEventA);
		m_pRecordTemp->AddEvent(strEventB);
	}

	return strVideoPath;
}

//构造前后牌不匹配事件
void CMatchPlateFortianjin::SetMatchEvent(
	const std::string &strVideoPath, 
	const RECORD_PLATE &plate, 
	const bool &bVideoFlag,
	std::string &strEvent)
{
	SRIP_DETECT_HEADER sHeader;

	sHeader.uChannelID = plate.uChannelID;
	sHeader.uSeq = plate.uSeq;
	sHeader.uTimestamp = plate.uTime;
	sHeader.uTime64 = (plate.uTime * 1000 * 1000) + (plate.uMiTime * 1000);

	RECORD_EVENT event;	
	event.uChannelID = plate.uChannelID;
	event.uSeq = plate.uSeq;	
	
	int nTimeLength = m_pRecordTemp->GetCaptureTime();
	if(bVideoFlag)
	{
		event.nVideoFlag = 1;

		event.uEventBeginTime = plate.uTime - 5;
		event.uEventEndTime = event.uEventBeginTime + nTimeLength;//FIX 固定事件持续事件为30s
	}
	else
	{
		event.nVideoFlag = 0;
		
		event.uEventEndTime = plate.uTime + 5;
		event.uEventBeginTime = event.uEventEndTime - nTimeLength;//FIX 固定事件持续事件为30s
	}
	memcpy(event.chVideoPath, strVideoPath.c_str(), strVideoPath.size());//事件录象路径

	//LogNormal("event tsB:%lld tsE:%lld %s \n",\
	//	event.uEventBeginTime, event.uEventEndTime, GetTime(event.uEventBeginTime, 3).c_str());

	strEvent.append((char*)&event,sizeof(RECORD_EVENT));
	strEvent.insert(0,(char*)&sHeader,sizeof(SRIP_DETECT_HEADER));
}

//获取录像路径
std::string CMatchPlateFortianjin::GetVideoSavePath(RECORD_PLATE& plate)
{			
	std::string strVideoPath("");
	std::string strTmpPath("");

	//需要判断磁盘是否已经满
	g_FileManage.CheckDisk(false,true);

	if(g_nServerType == 13 && g_nFtpServer == 1)
	{
		g_MyCenterServer.GetPlatePicPath(plate,strVideoPath,2);
	}
	else if(g_nServerType == 7)
	{
		g_RoadImcData.GetPlatePicPath(plate,strVideoPath,3);
		//LogNormal("g_RoadImcData.GetPlatePicPath strVideoPath=%s\n",strVideoPath.c_str());
	}
	else
	{
		strVideoPath = g_FileManage.GetDspEventVideoPath(g_uVideoId);
	}
	//LogTrace("Add-Event-Dsp.log", "=uSeq=%d=strVideoPath=%s=\n", sHeader.uSeq, strVideoPath);


	if(strVideoPath.size() > 0)
	{
		if(g_nServerType == 13 && g_nFtpServer == 1)
		{
			std::string strDataPath = "/home/road/dzjc";
			if (IsDataDisk())
			{
				strDataPath = "/detectdata/dzjc";
			}
			strTmpPath = strVideoPath.erase(0,strDataPath.size());
			strTmpPath = g_ServerHost+strTmpPath;
		}
		else if(g_nServerType == 7)
		{
			std::string strDataPath = "/home/road/red";
			if (IsDataDisk())
			{
				strDataPath = "/detectdata/red";
			}
			strTmpPath = strVideoPath.erase(0,strDataPath.size());
			strTmpPath = g_ServerHost+strTmpPath;
		}
		else
		{
			strTmpPath = strVideoPath.erase(0,g_strVideo.size());
			strTmpPath = g_ServerHost+strTmpPath;
		}

		strVideoPath = "ftp://"+strTmpPath;
		//memcpy(plate.chVideoPath,strVideoPath.c_str(),strVideoPath.size());//事件录象路径
	}

	return strVideoPath;
}

//void CMatchPlateFortianjin::DoDealNoMatchPlateFunc( RECORD_PLATE_DSP_MATCH &plate )
//{
//////#ifdef DEBUG_LIUYANG
////	LogNormal("[%s]:plate =%s,type=%d\n",__FUNCTION__,plate.dspRecord.chText,plate.dspRecord.uViolationType);
////	LogNormal("key[%d,%d,%d,%d] DoDealNoMatchPlate", \
////		plate.uKeyArray[0],plate.uKeyArray[1],plate.uKeyArray[2],plate.uKeyArray[3]);
//////#endif
//
//	//客户端设定时间端输出
//	bool bDetect = CheckDetectTime(); 
//	if (bDetect == false) 
//	{
//		return; 
//	}
//
//	//RECORD_PLATE plate;
//	UINT32 uWidth = plate.dspRecord.uPicWidth;
//	UINT32 uHeight = plate.dspRecord.uPicHeight;
//
//	//LogNormal("***[%s]:uW :%d,uH:%d,%s\n",__FUNCTION__,uWidth,uHeight,plate.dspRecord.chText);
//
//	if(uWidth > 2000 && uWidth < 10000)
//	{
//		if(NULL == m_pDataProc500W)
//		{
//			m_pDataProc500W = new CDspDataProcess(NULL);
//			m_pDataProc500W->Init(plate.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
//		}
////#ifdef DEBUG_LIUYANG
////		LogNormal("[%s]:plate =%s,type=%d\n",__FUNCTION__,plate.dspRecord.chText,plate.dspRecord.uViolationType);
////		LogNormal("key[%d,%d,%d,%d] DoDealNoMatchPlate", \
////			plate.uKeyArray[0],plate.uKeyArray[1],plate.uKeyArray[2],plate.uKeyArray[3]);
////#endif
//		if (0 == plate.dspRecord.uViolationType)  //卡口
//		{
//			m_pDataProc500W->OutPutNoMatch(plate);
//		}
//		else
//		{
//			m_pDataProc500W->OutPutVtsNoMatch(plate);
//		}
//	}
//	else if(uWidth > 0 && uWidth < 2000)
//	{
//		if(NULL == m_pDataProc200W)
//		{
//			m_pDataProc200W = new CDspDataProcess(NULL);
//			m_pDataProc200W->Init(plate.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
//		}
//		if (0 == plate.dspRecord.uViolationType)  //卡口
//		{
//			m_pDataProc200W->OutPutNoMatch(plate);
//		}
//		else
//		{
//			m_pDataProc200W->OutPutVtsNoMatch(plate);
//		}
//	}
//	else{}
//
//	return;
//}
//void* CMatchPlateFortianjin::DoDealPlate( void* lpParam )
//{
//	sMatchPlateData *pMatchData = (sMatchPlateData*)lpParam;
//	CMatchPlateFortianjin *pThis = (CMatchPlateFortianjin *)pMatchData->param;
//	//RECORD_PLATE_DSP_MATCH A = pMatchData->A;
//	//RECORD_PLATE_DSP_MATCH B = pMatchData->B;
//
//	if (pMatchData != NULL && pThis != NULL)
//	{
//		//LogNormal("before DoMatch id =0x%x[%s]\n",pthread_self(),pMatchData->A.dspRecord.chText);
//		pThis->DoDealPlateFunc(pMatchData->A,pMatchData->B);
//		//LogNormal("after DoMatch id =0x%x[%s]\n",pthread_self(),pMatchData->A.dspRecord.chText);
//		delete pMatchData;
//		pMatchData = NULL;
//	}
//	return NULL;
//}
//void* CMatchPlateFortianjin::DoDealNoMatchPlate( void* lpParam )
//{
//	sNoMatchPlateData *pNoMatchData = (sNoMatchPlateData*)lpParam;
//	CMatchPlateFortianjin *pThis = (CMatchPlateFortianjin *)pNoMatchData->param;
//	//RECORD_PLATE_DSP_MATCH plate = pNoMatchData->plate;
//
//	if (pNoMatchData != NULL && pThis != NULL)
//	{
//		//LogNormal("before DoNoMatch id =0x%x[%s]\n",pthread_self(),pNoMatchData->plate.dspRecord.chText);
//		pThis->DoDealNoMatchPlateFunc(pNoMatchData->plate);
//		//LogNormal("after DoNoMatch id=0x%x[%s]\n",pthread_self(),pNoMatchData->plate.dspRecord.chText);
//		delete pNoMatchData;
//		pNoMatchData = NULL;
//	}
//	return NULL;
//}

bool CMatchPlateFortianjin::PopMatchData(MatchPlate & sMatchPlate)
{
	bool bRet = false;
	pthread_mutex_lock(&m_MatchList_Mutex);
	if (m_vcMatchList.size() > 0)
	{		
		vector<MatchPlate>::iterator iterPro = m_vcMatchList.begin();
		sMatchPlate = (MatchPlate)*iterPro;		
		m_vcMatchList.erase(iterPro);		
		
		bRet = true;
	}
	pthread_mutex_unlock(&m_MatchList_Mutex);
	return bRet;
}

//#endif //FBMATCHPLATE
