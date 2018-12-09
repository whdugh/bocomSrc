
#include "Common.h"
#include "CommonHeader.h"
#include "ippi.h"
#include "ippcc.h"
#include "ximage.h"
#include "XmlParaUtil.h"
#include "DspDataManage.h"
#include "DspDataProcess.h"
#ifndef ALGORITHM_YUV
#ifndef DSP_GOJIAO_TEST
	#define DSP_GOJIAO_TEST
#endif

int CDspDataManage::m_DataExit = 0;

void* DoProcessFor200W(void* lpParam)
{
	PDspDataManage pThis = (PDspDataManage)lpParam;
	if(pThis == NULL) return NULL;

	CDspDataManage *pDataManage = (CDspDataManage *)(pThis->pManage);
	if(pDataManage != NULL)
	{
		pDataManage->DoDataProcessFor200W(pThis->nIndex);
	}
	else
	{
		printf("==pDataManage =DoProcessFor200W NULL=\n");
	}
	LogNormal("------********---OUT---thread---DoProcessFor200W---***************---\n");
	pthread_exit((void *)0);
	
	return NULL;

}

void* DoProcessFor500W(void* lpParam)
{
	PDspDataManage pThis = (PDspDataManage)lpParam;
	if(pThis == NULL) return NULL;

	CDspDataManage *pDataManage = (CDspDataManage *)(pThis->pManage);
	if(pDataManage != NULL)
	{
		pDataManage->DoDataProcessFor500W(pThis->nIndex);
	}
	else
	{
		printf("==pDataManage = DoProcessFor500W NULL=\n");
	}
	LogNormal("------********---OUT---thread---DoProcessFor500W---***************---\n");
	pthread_exit((void *)0);
	return NULL;
}

void* DoProcessOutPutTemp(void* lpParam)
{
	PDspDataManage pThis = (PDspDataManage)lpParam;
	if(pThis == NULL) return NULL;

	CDspDataManage *pDataManage = (CDspDataManage *)(pThis->pManage);
	if(pDataManage != NULL)
	{
		pDataManage->DoDataProcessOutPutTemp(pThis->nIndex);
	}
	else
	{
		printf("==pDataManage = DoProcessOutPutTemp NULL=\n");
	}
	LogNormal("------********---OUT---thread---DoProcessOutPutTemp---***************---\n");
	pthread_exit((void *)0);
	return NULL;
}

int CDspDataManage::DataProcessThreadFor200W()
{
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	struct   sched_param   param;

	param.sched_priority   =   20;
	pthread_attr_setschedparam(&attr,   &param);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//输出结果线程ID
	pthread_t nOutPutThreadId[m_nThreadCntFor200W];
	int nret = 0;
	//启动目标检测线程
	for(int nIndex = 0; nIndex < m_nThreadCntFor200W;nIndex++)
	{
		SDspDataManage dpsManage;
		dpsManage.nIndex = nIndex;
		dpsManage.pManage = (void*)this;

		nret = pthread_create(&nOutPutThreadId[nIndex],&attr,DoProcessFor200W,(void*)&dpsManage);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建目标检测线程失败,服务无法检测目标！\r\n");
			continue;
		}

		LogNormal("------********---cr---thread---DoProcessFor200W---***************---\n");
	}
	pthread_attr_destroy(&attr);

	return 0;
}

int CDspDataManage::DataProcessThreadFor500W()
{
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	struct   sched_param   param;

	param.sched_priority   =   20;
	pthread_attr_setschedparam(&attr,   &param);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//输出结果线程ID
	pthread_t nOutPutThreadId[m_nThreadCntFor500W];
	int nret = 0;
	//启动目标检测线程
	for(int nIndex = 0; nIndex < m_nThreadCntFor500W;nIndex++)
	{
		SDspDataManage dpsManage;
		dpsManage.nIndex = nIndex;
		dpsManage.pManage = (void*)this;

		nret = pthread_create(&nOutPutThreadId[nIndex],&attr,DoProcessFor500W,(void*)&dpsManage);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建目标检测线程失败,服务无法检测目标！\r\n");
			continue;
		}
		LogNormal("------********---cr---thread---DoProcessFor500W---***************---\n");
	}
	pthread_attr_destroy(&attr);

	return 0;
}


int CDspDataManage::DataProcessThreadOutPutTemp()
{
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	struct   sched_param   param;

	param.sched_priority   =   2;
	pthread_attr_setschedparam(&attr,   &param);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//输出结果线程ID
	pthread_t nOutPutThreadId[m_nThreadCntForTemp];
	int nret = 0;
	//启动目标检测线程
	for(int nIndex = 0; nIndex < m_nThreadCntForTemp;nIndex++)
	{
		SDspDataManage dpsManage;
		dpsManage.nIndex = nIndex;
		dpsManage.pManage = (void*)this;

		nret = pthread_create(&nOutPutThreadId[nIndex],&attr,DoProcessOutPutTemp,(void*)&dpsManage);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建输出缓存线程失败！\r\n");
			continue;
		}

		LogNormal("------********---cr---thread---DataProcessThreadOutPutTemp---***************---\n");
	}
	pthread_attr_destroy(&attr);

	return 0;
}

CDspDataManage::CDspDataManage(int nDataCount)
{
	m_nDataCount = nDataCount;
	m_nChannelId = 1;
	m_nExtentHeight = 60;
	m_nWordPos = 0;
	m_nWaitNumFor200W = 0;
	m_nWaitNumFor500W = 0;
	m_nThreadCntFor200W = nDataCount;
	m_nThreadCntFor500W = nDataCount;	

	//存取信号互斥
	pthread_mutex_init(&m_JpgFrameMutex,NULL);
	pthread_mutex_init(&m_WaitFor200WMutex,NULL);
	pthread_mutex_init(&m_WaitFor500WMutex,NULL);

	pthread_mutex_init(&m_OutPutMutex,NULL);
	
	m_ServerJpgFrameMap.clear();
	m_mapWaitListFor200W.clear();
	m_mapWaitListFor500W.clear();

	m_mapCarnumOut.clear();

	m_nTestSeq = 0;

	m_nWaitNumForTemp = 0;
	//m_nThreadCntForTemp = nDataCount;
	m_nThreadCntForTemp = 1;
	m_nDetectKind = DETECT_NONE;
}

CDspDataManage::~CDspDataManage()
{
	m_DataExit = 1;

	//存取信号互斥
	pthread_mutex_destroy(&m_JpgFrameMutex);
	pthread_mutex_destroy(&m_WaitFor200WMutex);
	pthread_mutex_destroy(&m_WaitFor500WMutex);

	pthread_mutex_destroy(&m_OutPutMutex);
}


//初始化
bool CDspDataManage::InitDspData(int nChannelId, CHANNEL_DETECT_KIND nDetectKind)
{
	m_nWordPos = g_PicFormatInfo.nWordPos;
	m_nExtentHeight = g_PicFormatInfo.nExtentHeight;
	m_nChannelId = nChannelId;
	m_nDetectKind = nDetectKind;

	DataProcessThreadFor200W();
	DataProcessThreadFor500W();
	DataProcessThreadOutPutTemp();

	m_nTestSeq = 0;
	return true;
	
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//添加信息
bool CDspDataManage::AddFrame(char* pBuffer)
{
	Image_header_dsp* pHeader = (Image_header_dsp*)pBuffer;
	if(pHeader->nSize == 0)
	{
		return false;
	}

	//添加当前数据进入处理时间戳,单位s.
	pHeader->uRecvTs = GetTimeStamp();
	
	//车牌记录
	if(pHeader->nType == DSP_IMG_PLATE_INFO || pHeader->nType == DSP_IMG_EVENT_INFO)// 违章
	{
		RECORD_PLATE_DSP *pPlate = (RECORD_PLATE_DSP*)(pBuffer + sizeof(Image_header_dsp));
		if(pPlate)
		{
			LogTrace("FindLog.txt", "AddDataM -[id:%d,type:%d,seq:%d, tsHead:%d, \n \
									 seqP:[%d,%d,%d], tsP:%d , plate:%s]---", \
				pHeader->uCameraId, pHeader->nType, pHeader->nSeq, (pHeader->ts/1000)/1000, \
				pPlate->uSeq, pPlate->uSeq2, pPlate->uSeq3, pPlate->uTime, pPlate->chText);
			pPlate = NULL;
		}		

		int nCameraType = 0;
		if(pHeader->nWidth == DSP_500_BIG_WIDTH_SERVER && pHeader->nHeight == DSP_500_BIG_HEIGHT_SERVER)
		{
			nCameraType = DSP_ROSEEK_500_335;
			string strData = "";
			strData.append(pBuffer, sizeof(Image_header_dsp) + sizeof(RECORD_PLATE_DSP));
			pthread_mutex_lock(&m_WaitFor500WMutex);
			m_nWaitNumFor500W++;
			m_mapWaitListFor500W.push_back(strData);
			pthread_mutex_unlock(&m_WaitFor500WMutex);
		}
		else if(pHeader->nWidth == DSP_200_BIG_WIDTH_SERVER && pHeader->nHeight == DSP_200_BIG_HEIGHT_SERVER)
		{
			nCameraType = DSP_ROSEEK_200_310;
			string strData = "";
			strData.append(pBuffer, sizeof(Image_header_dsp) + sizeof(RECORD_PLATE_DSP));
			pthread_mutex_lock(&m_WaitFor200WMutex);
			m_nWaitNumFor200W++;
			m_mapWaitListFor200W.push_back(strData);
			pthread_mutex_unlock(&m_WaitFor200WMutex);
		}
		else
		{
			printf("--OTHER ---Camera-----pHeader->nWidth=%d---pHeader->nHeight=%d--\n", \
				pHeader->nWidth, pHeader->nHeight);
			return false;
		}
	}//jpg大图
#ifdef DSP_GOJIAO_TEST
	else if(DSP_IMG_TEST_PIC == pHeader->nType)//模拟一条测试数据
	{
		//LogNormal("---Add --pHeader->nType---nSeq=%d nCameraId=%d \n", pHeader->nSeq, pHeader->uCameraId);
		pHeader->nType = 1;
		pHeader->nSeq += m_nTestSeq;
		m_nTestSeq++;
		AddJpgFrame((BYTE*)pBuffer);

		Image_header_dsp dspPlateHead;
		RECORD_PLATE_DSP dspPlate;
		
		memcpy((char*)(&dspPlateHead), pHeader, sizeof(Image_header_dsp));
		//dspPlateHead.uCameraId = ;
		dspPlate.uSeq = pHeader->nSeq;
		dspPlateHead.nType = 2;//测试图片类型，输出专门处理
		dspPlateHead.nSize = sizeof(RECORD_PLATE_DSP);
		
		char szText[16] = "dTest88";//沪TTTTTT
		memcpy(dspPlate.chText, szText, 7);
		dspPlate.uTime = (pHeader->ts / 1000 / 1000);
		dspPlate.uViolationType = 99;	
		dspPlate.uPosLeft = pHeader->nWidth/2;
		dspPlate.uPosTop = pHeader->nHeight/2;
		dspPlate.uPosRight = dspPlate.uPosLeft + 100;
		dspPlate.uPosBottom = dspPlate.uPosTop + 20;

		int nCameraType = 0;
		if(pHeader->nWidth == DSP_500_BIG_WIDTH_SERVER && pHeader->nHeight == DSP_500_BIG_HEIGHT_SERVER)
		{
			nCameraType = DSP_ROSEEK_500_335;
			string strData = "";
			strData.append((char*)(&dspPlateHead), sizeof(Image_header_dsp));
			strData.append((char*)(&dspPlate), sizeof(RECORD_PLATE_DSP));
			pthread_mutex_lock(&m_WaitFor500WMutex);
			m_nWaitNumFor500W++;
			m_mapWaitListFor500W.push_back(strData);
			pthread_mutex_unlock(&m_WaitFor500WMutex);
		}
		else if(pHeader->nWidth == DSP_200_BIG_WIDTH_SERVER && pHeader->nHeight == DSP_200_BIG_HEIGHT_SERVER)
		{
			nCameraType = DSP_ROSEEK_200_310;
			string strData = "";
			strData.append((char*)(&dspPlateHead), sizeof(Image_header_dsp));
			strData.append((char*)(&dspPlate), sizeof(RECORD_PLATE_DSP));
			pthread_mutex_lock(&m_WaitFor200WMutex);
			m_nWaitNumFor200W++;
			m_mapWaitListFor200W.push_back(strData);
			pthread_mutex_unlock(&m_WaitFor200WMutex);
		}
		else
		{
			printf("--OTHER ---Camera-----pHeader->nWidth=%d---pHeader->nHeight=%d--\n", \
				pHeader->nWidth, pHeader->nHeight);
			return false;
		}		
	}
#endif
	else if(DSP_LOG == pHeader->nType)
	{
		int HeaderLength = sizeof(Image_header_dsp);
		FILE *fpOut = NULL;
		char jpg_name[256] = {0};

		time_t now;
		struct tm *newTime,timenow;
		newTime = &timenow;
		time( &now );
		localtime_r( &now,newTime );

		time_t dspTime;
		dspTime = (pHeader->ts/1000/1000);
		struct tm *tm_dspTime, tm_dsptime_now;
		tm_dspTime = &tm_dsptime_now;
		localtime_r(&dspTime, tm_dspTime);
		//sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d", , newTime->tm_mon+1, newTime->tm_mday, newTime->tm_hour, newTime->tm_min, newTime->tm_sec);

		//log
		{
			sprintf(jpg_name, "./DspLog_%s_%4d_%02d_%02d_%02d_DSP_%4d_%02d_%02d_%02d.log", \
				pHeader->szCameraCode,newTime->tm_year+1900, newTime->tm_mon+1,newTime->tm_mday, newTime->tm_hour,
				tm_dspTime->tm_year+1900, tm_dspTime->tm_mon+1,tm_dspTime->tm_mday, tm_dspTime->tm_hour);
			fpOut = fopen(jpg_name, "ab+");

			char textEx[1024] = {0};
			sprintf(textEx, "dsp Time: %4d_%02d_%02d_%02d  %d:%d",\
				tm_dspTime->tm_year+1900, tm_dspTime->tm_mon+1,tm_dspTime->tm_mday, tm_dspTime->tm_hour, tm_dspTime->tm_min, tm_dspTime->tm_sec);

			if(fpOut)
			{
				fwrite((BYTE*)textEx, 1, strlen(textEx), fpOut);
				fwrite((BYTE*)pBuffer + HeaderLength, 1, pHeader->nSize, fpOut);
				fclose(fpOut);
			}
		}
	}
	else if(pHeader->nType == 1)
	{
		LogTrace("FindLog.txt", "111 AddJpgFrame ts = %d ", (pHeader->ts/1000) /1000);
		AddJpgFrame((BYTE*)pBuffer);
	}
	return true;
}

int CDspDataManage::DoDataProcessFor200W(int nIndex)
{
	if (m_nDataCount > nIndex)
	{
		CDspDataProcess* pDataProc = new CDspDataProcess(this);
		pDataProc->Init(m_nChannelId,DSP_200_BIG_WIDTH_SERVER,DSP_200_BIG_HEIGHT_SERVER,DSP_200_BIG_WIDTH_SERVER,DSP_200_BIG_HEIGHT_SERVER,m_nDetectKind);
		while(!m_DataExit)
		{
			if (m_nWaitNumFor200W > 0)
			{
				string pDataProcess = "";
				pthread_mutex_lock(&m_WaitFor200WMutex);
				vector<string>::iterator iterPro;
				for(iterPro = m_mapWaitListFor200W.begin(); iterPro != m_mapWaitListFor200W.end(); iterPro++)
				{
					pDataProcess = *iterPro;
					if (pDataProcess.size() > 0 )
					{
						m_nWaitNumFor200W--;
						m_mapWaitListFor200W.erase(iterPro);
						break;
					}
				}
				pthread_mutex_unlock(&m_WaitFor200WMutex);
				if (pDataProcess.size() > 0)
				{
					SDspOutPutKeyTemp outkeyTemp;
					//Picture_Key key;

					Image_header_dsp * pHeader = (Image_header_dsp *)(pDataProcess.c_str());
					outkeyTemp.key.uCameraId = pHeader->uCameraId;
					outkeyTemp.key.uSeq = pHeader->nSeq;
					outkeyTemp.ts = pHeader->ts;
					outkeyTemp.tsRecv = pHeader->uRecvTs;
					outkeyTemp.pDataProc = pDataProc;
					bool bCheck = false;

					if(DSP_IMG_PLATE_INFO == pHeader->nType)
					{
						bCheck = CheckCarNumOutPut(outkeyTemp.key, outkeyTemp.ts);
						LogTrace("FindLog.txt","DspEventOrder -1 nSeq:%d--\n", \
							pHeader->nSeq);
					}
					else if(DSP_IMG_EVENT_INFO == pHeader->nType)
					{
						//违章记录解析
						RECORD_PLATE_DSP *pPlate =	(RECORD_PLATE_DSP *)(pDataProcess.c_str() + sizeof(Image_header_dsp));
						//违章处理
						outkeyTemp.key.uSeq = pPlate->uSeq3;
						LogTrace("FindLog.txt","DspEventOrder -10 nSeq:%d--[%d,%d,%d]\n", \
							pHeader->nSeq, pPlate->uSeq, pPlate->uSeq2, pPlate->uSeq3);
					}
					else
					{
						//
					}

					if(bCheck)
					{
						pDataProc->AddPlateFrame((BYTE*)(pDataProcess.c_str()));
					}
					else
					{	
						pthread_mutex_lock(&m_OutPutMutex);

						//缓存输出
						m_mapCarnumOut.insert(make_pair(outkeyTemp, pDataProcess));
						m_nWaitNumForTemp++;

						if(m_mapCarnumOut.size() > 100)
						{
							CarnumMap::iterator iterPro;
							iterPro = m_mapCarnumOut.begin();
							LogNormal("1Big-deal-Temp-uSeq=%d--m_mapCarnumOut.size=%d \n", \
								iterPro->first.key.uSeq, m_mapCarnumOut.size());
							m_nWaitNumForTemp--;
							m_mapCarnumOut.erase(iterPro);
						}

						pthread_mutex_unlock(&m_OutPutMutex);
					}
				}
			}
			usleep(1000*1);
		}
		if (pDataProc)
		{
			delete pDataProc;
			pDataProc = NULL;
		}
	}
	return 0;
}

int CDspDataManage::DoDataProcessFor500W(int nIndex)
{
	if (m_nDataCount > nIndex)
	{
		CDspDataProcess* pDataProc = new CDspDataProcess(this);
		pDataProc->Init(m_nChannelId,DSP_500_BIG_WIDTH_SERVER,DSP_500_BIG_HEIGHT_SERVER,DSP_500_BIG_WIDTH_SERVER,DSP_500_BIG_HEIGHT_SERVER, m_nDetectKind);
		while(!m_DataExit)
		{
			if (m_nWaitNumFor500W > 0)
			{
				string pDataProcess = "";
				pthread_mutex_lock(&m_WaitFor500WMutex);
				vector<string>::iterator iterPro;
				for(iterPro = m_mapWaitListFor500W.begin(); iterPro != m_mapWaitListFor500W.end(); iterPro++)
				{
					pDataProcess = *iterPro;
					if (pDataProcess.size() > 0 )
					{
						m_nWaitNumFor500W--;
						m_mapWaitListFor500W.erase(iterPro);
						break;
					}
				}
				pthread_mutex_unlock(&m_WaitFor500WMutex);
				/*
				if (pDataProcess.size() > 0)
				{
					pDataProc->AddPlateFrame((BYTE*)(pDataProcess.c_str()));
				}
				*/
				if (pDataProcess.size() > 0)
				{
					SDspOutPutKeyTemp outkeyTemp;
					//Picture_Key key;

					Image_header_dsp * pHeader = (Image_header_dsp *)(pDataProcess.c_str());
					outkeyTemp.key.uCameraId = pHeader->uCameraId;
					outkeyTemp.key.uSeq = pHeader->nSeq;
					outkeyTemp.ts = pHeader->ts;
					outkeyTemp.tsRecv = pHeader->uRecvTs;

					outkeyTemp.pDataProc = pDataProc;

					bool bCheck = CheckCarNumOutPut(outkeyTemp.key, outkeyTemp.ts);

					if(bCheck)
					{
						pDataProc->AddPlateFrame((BYTE*)(pDataProcess.c_str()));
					}
					else
					{	
						pthread_mutex_lock(&m_OutPutMutex);

						//缓存输出
						m_mapCarnumOut.insert(make_pair(outkeyTemp, pDataProcess));
						m_nWaitNumForTemp++;

						if(m_mapCarnumOut.size() > 100)
						{
							CarnumMap::iterator iterPro;
							iterPro = m_mapCarnumOut.begin();
							//LogNormal("2Big-deal-Temp-uSeq=%d--m_mapCarnumOut.size=%d \n", \
								iterPro->first.key.uSeq, m_mapCarnumOut.size());
							m_nWaitNumForTemp--;
							m_mapCarnumOut.erase(iterPro);
						}

						pthread_mutex_unlock(&m_OutPutMutex);
					}
				}
			}
			usleep(1000*1);
		}
		if (pDataProc)
		{
			delete pDataProc;
			pDataProc = NULL;
		}
	}
	return 0;
}


//添加jpg图像帧
bool  CDspDataManage::AddJpgFrame(BYTE* pBuffer)
{
	if(pBuffer != NULL)
	{
		Image_header_dsp* pHeader = (Image_header_dsp*)pBuffer;

		if (!pHeader)
		{
			return false;
		}

		LogTrace("FindLog.txt", "AddDspServerJpg -AddJpgFrame--nSeq[%d]-picSize:%d -nSize=%d ts:%d \n", \
			pHeader->nSeq, pHeader->nSize, m_ServerJpgFrameMap.size(), (pHeader->ts/1000)/1000);

		RemoveServerJpg(pHeader);		
	
		yuv_video_buf camera_header;
		camera_header.nVideoType = 1;
		//取到实际图像宽高
		camera_header.height = pHeader->nHeight;
		camera_header.width = pHeader->nWidth;
		camera_header.size = pHeader->nSize;
		camera_header.nSeq = pHeader->nSeq; //帧号
		camera_header.uTimestamp = (pHeader->ts / 1000) / 1000; //单位秒s
		camera_header.ts = pHeader->ts;

		camera_header.nFrameRate = 10;

		//传入线圈信号
		camera_header.uSpeedSignal.SpeedSignal = pHeader->nSpeedSignal; //
		camera_header.uSpeedSignal.SystemTime =  pHeader->ts;//us
		camera_header.uSpeedSignal.SpeedTime = pHeader->SpeedTime;
		//红灯信号
		camera_header.uTrafficSignal = pHeader->nRedColor;

		camera_header.uFlashSignal = pHeader->nFlashFlag; //爆闪灯状态
		camera_header.data = pBuffer + sizeof(Image_header_dsp);

		string strPic;

		Picture_Elem picElem;
		picElem.key.uSeq = pHeader->nSeq;
		picElem.key.uCameraId = pHeader->uCameraId;
		picElem.ts = pHeader->ts;

		printf("*********** gcs W:%d H:%d Seq:%d ID:%d\n",pHeader->nWidth,pHeader->nHeight,pHeader->nSeq,pHeader->uCameraId);
		strPic.append((char*)&camera_header,sizeof(yuv_video_buf));
		strPic.append((char*)(pBuffer+sizeof(Image_header_dsp)),pHeader->nSize);

		//printf("***********22  strPic.size:%d \n", strPic.size());

		//加锁
		pthread_mutex_lock(&m_JpgFrameMutex);

		m_ServerJpgFrameMap.insert(JpgMap::value_type(picElem,strPic));
		//LogNormal("添加jpg图像帧成功,帧号：%lld，相机编号:%d\n",pHeader->nSeq,DspHeader->uCameraId);

		//printf("***********33  strPic.size:%d m_ServerJpgFrameMap.size:%d \n", strPic.size(), m_ServerJpgFrameMap.size());

		//解锁
		pthread_mutex_unlock(&m_JpgFrameMutex);

		return true;
	}
	return false;
}

//根据帧号和相机ID组成的关键字从Jpg缓冲区查找出对应的图片 add by wantao
bool CDspDataManage::GetImageByJpgKey(const Picture_Elem &PicElem,PLATEPOSITION *pTimeStamp,IplImage* pImage)
{
	bool bRet = false;
	if(m_DataExit)
	{
		bRet = false;
	}

	if(pImage == NULL)
	{
		printf("--pImage == NULL-\n");
		return false;
	}

	yuv_video_buf* buf = NULL;

	string strPic = "";
	int nCount = 3;//等待3秒

	if(m_mapCarnumOut.size() > 100)
	{
		nCount = 2;//等待2秒
	}

	bool bFindPic = false;

	while(nCount > 0)
	{	
		if(m_ServerJpgFrameMap.size() > 0)
		{
			//map<Picture_Key,string>::iterator it_map = m_ServerJpgFrameMap.find(Pic_Key);
			//找出对应帧号,对应时间点的图片.
			bFindPic =  FindPicFromJpgMap(PicElem.key, PicElem.ts, strPic);

			if(bFindPic)
			{
				//m_ServerJpgFrameMap.erase(it_map);
				LogTrace("FindLog.txt", \
					"从m_JpgServerFrameMap中取图成功，帧号nSeq=%lld 相机ID:%d , strPic:%d",\
					PicElem.key.uSeq, PicElem.key.uCameraId, strPic.size());
				bRet = true;
				break;
			}
			else
			{
				if(nCount == 1)				
				{
					LogTrace("FindLog.txt", \
						"未找到帧号nSeq=%lld,相机ID:%d\n",\
						PicElem.key.uSeq, PicElem.key.uCameraId);
					break;
				}
			}
		}
		else
		{
			//解锁
			LogError("No Data in m_ServerJpgFrameMap\n");
			bRet = false;
			break;
		}
		
		//usleep(100 * 1000);
		sleep(1);
		nCount --;
	}//End of while

		
	printf("-----TTT 1111--------------\n");
	if(strPic.size() > 0)
	{
		buf = (yuv_video_buf*)strPic.c_str();		

		if(buf)
		{
			printf("-----TTT 2222--------------\n");
			pTimeStamp->ts = buf->ts;
			pTimeStamp->uTimestamp = buf->uTimestamp;
			pTimeStamp->uFieldSeq = buf->nFieldSeq;
			pTimeStamp->uSeq = buf->nSeq;

			//需要解码jpg图像
			 CxImage image;
             image.Decode((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)), strPic.size() - sizeof(yuv_video_buf), 3); //解码

			 printf("-----TTT 3333--------------\n");
			 cvSet(pImage, cvScalar( 0,0, 0 ));
			 if(image.IsValid()&&image.GetSrcSize()>0)
			 {
				 printf("-----TTT 3333--1111------------\n");
				if(m_nWordPos == 0)//黑边在下方
				{
					//printf("GetBits:%d GetSize:%d\n",image.GetBits(),image.GetSrcSize());
					memcpy(pImage->imageData,image.GetBits(),image.GetSrcSize());
					printf("-----TTT 3333--2222------------\n");
				}
				else//黑边在上方
				{
					//printf("-pImage:%x-m_nExtentHeight:%d widthStep:%d image.GetSrcSize():%d \n", \
					//	pImage, m_nExtentHeight, pImage->widthStep, image.GetSrcSize());
					memcpy(pImage->imageData+m_nExtentHeight*pImage->widthStep,image.GetBits(),image.GetSrcSize());
				}	
			 }
			 else //解码失败！！
			 {
			 }
			 printf("-----TTT 4444--------------\n");
		}
	}
	else
	{
		//LogNormal("--uSeq-%d----no pic!\n", PicElem.key.uSeq);
		LogTrace("FindLog.txt","--uSeq-%d---ts:%d-no pic!",PicElem.key.uSeq, (PicElem.ts/1000)/1000);

		//map<Picture_Key,string>::iterator it_map_b = m_ServerJpgFrameMap.begin();
		JpgMap::iterator it_map_b = m_ServerJpgFrameMap.begin();
		for(;it_map_b != m_ServerJpgFrameMap.end(); it_map_b++)
		{
			LogTrace("FindLog.txt"," nSeq [%d] nCamera [%d] ts:%d, size:%d ", \
				it_map_b->first.key.uSeq, it_map_b->first.key.uCameraId, (it_map_b->first.ts/1000)/1000, it_map_b->second.size());
		}

		bRet = false;
	}

	printf("-----TTT 5555--------------\n");
	return bRet;
}

/*
//从jpg map中寻找一个最接近的违章图片
void CDspDataManage::GetPicByKeyFromMap(Picture_Key pickeyDst, string& strPic)
{
	bool bFind = false;

	UINT32 nDisSeq = 0;
	Picture_Key pickeyCur;
	Picture_Key pickeyPre;
	int nIndex = 0;

	map<Picture_Key,string>::iterator it_pre;
	map<Picture_Key,string>::iterator it;

	if(m_ServerJpgFrameMap.size() > 0)
	{
		it = m_ServerJpgFrameMap.begin();
		while( it!=m_ServerJpgFrameMap.end())
		{
			pickeyCur = it->first;
			if (pickeyDst.uCameraId != pickeyCur.uCameraId)
			{
				it++;
				continue;
			}
			strPic = it->second;
			//if( (nSeq <= nSeqCur) && ((nPreSeq == 0) ||(nSeqCur > nPreSeq)))//尾牌
			if(pickeyDst.uSeq <= pickeyCur.uSeq)
			{
				if (nIndex == 0)
				{
					it_pre = it;
				}
				

				pickeyPre = it_pre->first;

				nDisSeq = pickeyCur.uSeq - pickeyPre.uSeq;//帧号差

				//前后，帧差12以内取最近一张，否则取离摄像机更远的一张
				//由远及近：帧号更小的一张（时间靠前）
				//由近及远：帧号更大的一张（时间靠后）
				if(nDisSeq <= 12)
				{
					if(pickeyDst.uSeq <= (pickeyCur.uSeq + pickeyPre.uSeq) * 0.5) //择半查找，取更近的那一帧
					{
						strPic = it_pre->second;
					}
					else
					{
						strPic = it->second;
					}
				}
				else
				{
					//尾牌（由近及远）
					strPic = it->second;
				}

				//LogNormal("dd nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n", pickeyDst.uSeq,pickeyPre.uSeq,pickeyCur.uSeq);

				bFind = true;
				break;
			}

			it_pre = it;
			nIndex++;
			//LogNormal("##TT== nSeq=%d =TT##\n", pickeyCur.uSeq);
			it++;
		}

		if(!bFind)
		{
			//LogNormal("===find=pic==end,uSeq=%lld\n",(it->first).uSeq);
			///if(m_ServerJpgFrameMap.size() >0)
			//{
			//	it = --m_ServerJpgFrameMap.end();
			//	strPic = it->second;
			//	bFind = true;
			//	LogNormal("===find=pic==end,uSeq=%lld\n",(it->first).uSeq);
			//}
		}
	}
}
*/
int CDspDataManage::GetServerIpgCount()
{
	if(m_ServerJpgFrameMap.size() < 1)
	{
		printf("=err==m_ServerJpgFrameMap.size()=%d==\n", m_ServerJpgFrameMap.size());
		return 0;
	}
	return m_ServerJpgFrameMap.size();
}

int CDspDataManage::RemoveServerJpg(Image_header_dsp *pHeader)
{
	//printf("CDspDataManage::RemoveServerJpg 111 \n");
	if(m_ServerJpgFrameMap.size() >= IMAGEBUFUNIT)
	{
		//加锁
		pthread_mutex_lock(&m_JpgFrameMutex);

		JpgMap::iterator it_map_b = m_ServerJpgFrameMap.begin();

		int iSize = 0;
		while(it_map_b != m_ServerJpgFrameMap.end())
		{
			int tsDis = ((pHeader->ts - it_map_b->first.ts)/1000)/1000;
			if(pHeader->nSeq < it_map_b->first.key.uSeq)
			{
				LogTrace("FindLog.txt", \
					"EraseDspJpg 11-pHeader->nSeq=%d-ts:%d, eraze nSeq [%d] m_ServerJpgFrameMap.size()=%d ts=%d", \
					pHeader->nSeq, (pHeader->ts/1000)/1000, it_map_b->first.key.uSeq, m_ServerJpgFrameMap.size(), (it_map_b->first.ts/1000)/1000);
				m_ServerJpgFrameMap.erase(it_map_b);

				if(m_ServerJpgFrameMap.size() <= IMAGEBUFUNIT)
				{
					break;
				}				
			}
			else if(tsDis < -600 || tsDis > 600)//10分钟之前,之后数据删除
			{
				LogTrace("FindLog.txt", \
					"EraseDspJpg 22-pHeader->nSeq=%d-ts:%d, eraze nSeq [%d] m_ServerJpgFrameMap.size()=%d ts=%d", \
					pHeader->nSeq, (pHeader->ts/1000)/1000, it_map_b->first.key.uSeq, m_ServerJpgFrameMap.size(), (it_map_b->first.ts/1000)/1000);
				m_ServerJpgFrameMap.erase(it_map_b);
				if(m_ServerJpgFrameMap.size() <= IMAGEBUFUNIT)
				{
					break;
				}
			}
			else{}

			it_map_b++;
		}
		//解锁
		pthread_mutex_unlock(&m_JpgFrameMutex);
	}

	printf("CDspDataManage::RemoveServerJpg 222 \n");
}

//核查是否输出图片
bool CDspDataManage::CheckCarNumOutPut(const Picture_Key &Pic_Key, const int64_t &ts)
{
	bool bFindPic = false;
	string strPic = "";

	if(m_ServerJpgFrameMap.size() > 0)
	{
		bFindPic = FindPicFromJpgMap(Pic_Key, ts, strPic);
		if(bFindPic)
		{
			printf("从m_JpgServerFrameMap中Check图成功，帧号nSeq=%lld,相机ID:%d\n",\
				Pic_Key.uSeq,Pic_Key.uCameraId);				
		}
		else
		{
			printf("check未找到帧号nSeq=%lld,相机ID:%d\n",Pic_Key.uSeq,Pic_Key.uCameraId);
		}
	}
	else
	{
		//解锁
		LogError("No Data in m_ServerJpgFrameMap\n");
	}
	
	return bFindPic;
}


int CDspDataManage::DoDataProcessOutPutTemp(int nIndex)
{
	if (m_nDataCount > nIndex)
	{
		while(!m_DataExit)
		{
			if (m_nWaitNumForTemp > 0)
			{
				string pDataProcess = "";
				CDspDataProcess* pDataProc = NULL;
				bool bCheck = false;				
				int64_t uDisTs = 0;
				int64_t nTimeNow = GetTimeStamp();

				pthread_mutex_lock(&m_OutPutMutex);
				CarnumMap::iterator iterPro;				
				for(iterPro = m_mapCarnumOut.begin(); iterPro != m_mapCarnumOut.end(); iterPro++)
				{
					//处理当前可以处理的缓存记录
					//1.CheckCarNumOutPut 为真。					
					//2.ts 相差 300s以上,强制处理。
					bCheck = CheckCarNumOutPut(iterPro->first.key, iterPro->first.ts);	
					nTimeNow = GetTimeStamp();
					uDisTs = abs(nTimeNow - (iterPro->first.tsRecv));
					
					if(bCheck || uDisTs > MAX_TIME_DEAL)
					{
						//LogNormal("--nTimeNow=%d-uDisTs--=%d ts2=%d bCheck=%d\n", nTimeNow, uDisTs, (iterPro->first.ts / 1000000), bCheck);
						pDataProcess = (iterPro->second);
						pDataProc = iterPro->first.pDataProc;

						if (pDataProcess.size() > 0 )
						{
							//LogNormal("-deal-Temp-uSeq=%d--m_mapCarnumOut.size=%d \n", \
								iterPro->first.key.uSeq, m_mapCarnumOut.size());
							m_nWaitNumForTemp--;
							m_mapCarnumOut.erase(iterPro);
							break;
						}
					}
				}
			
				
				pthread_mutex_unlock(&m_OutPutMutex);

				if (pDataProcess.size() > 0)
				{
					if(pDataProc)
					{
						pDataProc->AddPlateFrame((BYTE*)(pDataProcess.c_str()));
					}					
				}
			}
			sleep(5); //5秒处理一次
		}
	}
	return 0;
}

//根据Picture_Key,时间戳,在JpgMap取图
bool CDspDataManage::FindPicFromJpgMap(Picture_Key key, int64 ts, string &strPic)
{
	bool bRet = false;

	int64 tsDisPre = 0;
	int nSameFlag = 0;

	pthread_mutex_lock(&m_JpgFrameMutex);

	JpgMap::iterator it_b = m_ServerJpgFrameMap.begin();

	JpgMap::iterator it_pre = it_b;
	for(; it_b != m_ServerJpgFrameMap.end(); it_b++)
	{
		if((it_b->first.key.uCameraId == key.uCameraId) && (it_b->first.key.uSeq == key.uSeq))
		{
			if(ts > 0 && it_b->first.ts > 0)
			{
				int64 tsDisTemp = abs((it_b->first.ts/ 1000/ 1000) - (ts/ 1000/ 1000));
				nSameFlag++;

				if(tsDisTemp < MAX_TIME_DIS)//只取MAX_TIME_DIS以内图片
				{
					bRet = true;

					if((nSameFlag > 1) && (it_b->first.key.uSeq == it_pre->first.key.uSeq) && (tsDisPre > 0))
					{
						LogTrace("FindLog.txt","same pic, uSeq1:%d size1:%d ts1:%d,tsDisPre:%d \n \
									uSeq2:%d size2:%d ts2:%d tsDisTemp:%d  \n", \
							it_pre->first.key.uSeq, it_pre->second.size(), (it_pre->first.ts)/1000/1000, tsDisPre, \
							it_b->first.key.uSeq, it_b->second.size(), (it_b->first.ts)/1000/1000, tsDisTemp);
						if(tsDisPre > tsDisTemp)
						{
							strPic = it_b->second;							
							LogTrace("FindLog.txt", "11 id:%d seq:%d same! size:%d!", \
								it_b->first.key.uCameraId, it_b->first.key.uSeq, strPic.size());
						}
						else
						{
							strPic = it_pre->second;
							LogTrace("FindLog.txt","22 id:%d seq:%d same! size:%d", \
								it_b->first.key.uCameraId, it_b->first.key.uSeq,strPic.size());
						}
					}
					else
					{
						strPic = it_b->second;
						tsDisPre = tsDisTemp;
						it_pre = it_b;
						LogTrace("FindLog.txt","get id:%d seq:%d timeDis=%d!", \
							it_b->first.key.uCameraId, it_b->first.key.uSeq, tsDisTemp);
					}
				}
				else
				{
					LogTrace("FindLog.txt","Time dis larg! nDis = %d! uSeq:%d tsMap:%d ,tsToFind:%d \n", \
						tsDisTemp, key.uSeq, (it_b->first.ts/1000)/1000, (ts/1000)/1000);
				}				
			}
			else
			{
				bRet = true;
				strPic = it_b->second;
				break;
			}
		}
	}
	pthread_mutex_unlock(&m_JpgFrameMutex);

	return bRet;
}

//确认是否,能根据Picture_Key,时间戳,在JpgMap取到图
bool CDspDataManage::IsFindPicFromJpgMap(Picture_Key key, int64 ts)
{
	bool bRet = false;

	pthread_mutex_lock(&m_JpgFrameMutex);

	JpgMap::iterator it_b = m_ServerJpgFrameMap.begin();
	for(; it_b != m_ServerJpgFrameMap.end(); it_b++)
	{
		if(it_b->first.key.uCameraId == key.uCameraId && it_b->first.key.uSeq == key.uSeq)
		{
			if(ts > 0 && it_b->first.ts > 0)
			{
				int64 tsDisTemp = (abs(it_b->first.ts - ts) / 1000) / 1000;

				if(tsDisTemp < MAX_TIME_DIS)//只取MAX_TIME_DIS钟以内图片 FIX
				{
					bRet = true;							
				}
				else
				{
					LogTrace("FindLog.txt","11 Time dis larg! nDis = %d! uSeq:%d \n", tsDisTemp, key.uSeq);
				}				
			}
			else
			{
				bRet = true;				
				break;
			}
		}
	}
	pthread_mutex_unlock(&m_JpgFrameMutex);

	return bRet;
}
#endif
