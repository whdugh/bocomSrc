// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#include "DHCamera.h"
#include "XmlParaUtil.h"

//#ifdef DIO_RTSP

CDHCamera::CDHCamera(int nCameraType)
{
	m_nCameraType = nCameraType;
	m_nWidth = 1920;
	m_nHeight = 1080;
	m_nMarginX = 0;
	m_nMarginY = 0;
	m_pFrameBuffer = NULL;
	m_nMaxBufferSize = 10;
	m_nDeviceId = 1;

	pthread_mutex_init(&m_Event_Mutex,NULL);
	//相机编号
	m_nDeviceId = 0;
	m_bEndCapture = false;
	m_ubiStreamId = 0;
	m_pCameraCtrl = new COnvifCameraCtrl();
	m_EventList.clear();
	m_bEndRecorder = false;
	m_mapRecordEvent.clear();
	m_nPrintTime = 0;
	CreateEventThread();

}

CDHCamera::~CDHCamera()
{
	m_bEndRecorder = true;
	m_bEndCapture = true;
	delete m_pCameraCtrl;
	pthread_mutex_lock(&m_Event_Mutex);
	m_EventList.clear();
	pthread_mutex_unlock(&m_Event_Mutex);

	pthread_mutex_destroy(&m_Event_Mutex);
}

bool CDHCamera::Open()
{
	printf("---CDHCamera::Open()\n");

	return OpenDH();

	
	return true;
}

bool CDHCamera::Close()
{
	printf("---CDHCamera::Close()\n");

	return CloseDH();

	return true;
}

bool CDHCamera::OpenDH()
{
	printf("%s \n",__FUNCTION__);

	m_bEndCapture = false;
	m_Decoder.SetVideoSize(m_nWidth,m_nHeight);
  	m_Decoder.SetVideoCodeID(0);
	m_Decoder.InitDecode(NULL);
    for(int i=0; i< MAX_BUFFER_SIZE; i++)
    {
            m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf) + m_nWidth*m_nHeight*3, sizeof(unsigned char));
    }
    m_pFrameBuffer = m_FrameList[0];

#ifdef DIO_RTSP
	DIO::DIOInit(0);  //初始化DIO库
#endif
	char szURL[100]={0};
	sprintf(szURL,"rtsp://%s:%d",m_strCameraIP.c_str(),m_nCameraPort);
	printf("URL:%s \n",szURL);

	m_pCameraCtrl->Init(m_nCameraType,m_strCameraIP.c_str(),80);

	//请求rtsp库
	#ifdef DIO_RTSP
	m_ubiStreamId = DIO::OpenStrmEx(szURL, "admin", "admin",false);
	if(m_ubiStreamId > 0)
	{
		//设置rtsp回调函数
		printf("open rtsp ok\n");
		DIO::SetVideoStrmCallback(m_ubiStreamId, (void *)StrmCallback, this);
	}
	else
	{
		printf("open rtsp failed\n");
		return true; //不论成功与否，都返回成功，不然通道无法加载成功，
	}
	#else
	string strpasswd("admin:admin");
	printf("===========strpasswd=%s\n",strpasswd.c_str());
	#ifdef RTSPMULTICAST
		m_RtspClient.Init(m_strCameraIP.c_str(),m_nCameraPort,"stream1m");
		LogNormal("RTSPMULTICAST\n");
	#else
		m_RtspClient.SetAuthorizationData(strpasswd);
		m_RtspClient.Init(m_strCameraIP.c_str(),m_nCameraPort,"cam/realmonitor?channel=1&subtype=0");
		LogNormal("ERROR!!!!!!!!!!!!!!!\n");
	#endif
	#endif

	pthread_t m_hStreamId;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //设置线程分离
	if(pthread_create(&m_hStreamId, NULL, DealH264ToYUVStream, this) != 0)
	{
		pthread_attr_destroy(&attr);
		printf("Start Thread Failed. \n");
		return true;
	}
	pthread_attr_destroy(&attr);


	usleep(10000); //开启线程后暂停10毫秒
	
	return true;
}


bool CDHCamera::CloseDH()
{
	printf("%s \n",__FUNCTION__);

	m_bEndCapture = true;
	usleep(300*1000);

	//关闭码流
	#ifdef DIO_RTSP
	if(m_ubiStreamId > 0)
	{
		DIO::CloseStrm(m_ubiStreamId);
	}
	#else
	m_RtspClient.UnInit();
	#endif
	
	m_Decoder.UinitDecode();

	pthread_mutex_lock(&m_FrameMutex);
    //释放内存
    for (int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        if(m_FrameList[i] != NULL)
        {
                free(m_FrameList[i]);
                m_FrameList[i] = NULL;
        }
        m_nFrameSize = 0;
        m_pFrameBuffer = NULL;
    }
    pthread_mutex_unlock(&m_FrameMutex);

	return true;
}

bool CDHCamera::ReOpen()
{
	Close();
	Open();
	return true;
}

//切换工作方式
bool CDHCamera::ChangeMode()
{
	return true;
}

//获取相机默认模板
void CDHCamera::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{
	return ;
}

void CDHCamera::ConvertCameraParameter(CAMERA_CONFIG& cfg, bool bReverseConvert, bool bFirstLoad)
{
	return ;
}

//设置设备编号
void CDHCamera::SetDeviceID(int nDeviceID)
{
	m_nDeviceId = nDeviceID;
	return ;
}


int32_t CDHCamera::StrmCallback(uint64_t ubiStrmId, unsigned char *pData,  int32_t iDataLen, void *pParam)
{
	//printf("ubiStrmId=%llu,pData=%x,iDataLen=%d,pParam=%x \n",ubiStrmId,pData,iDataLen,pParam);
	if(iDataLen <= 0)
	{
		return 0;
	}

	CDHCamera *pThis = (CDHCamera*)pParam;
	if(pThis)
	{
		pThis->DealH264Stream(ubiStrmId,pData,iDataLen);
	}
	return 0;
}

int32_t CDHCamera::DealH264Stream(uint64_t ubiStrmId, unsigned char *pData,  int32_t iDataLen)
{
	//插入数据
	WriteFrameToRecordFile(pData,iDataLen);

	#ifdef DIO_RTSP
	m_StreamParser.InputData(pData,(int)iDataLen);
	#endif
	return 0;
}



void * CDHCamera::DealH264ToYUVStream(void * lpParam)
{
	CDHCamera *pThis = (CDHCamera *)lpParam;
	if(pThis)
	{
		pThis->DealH264ToYUVStreamFunc();
	}
	return NULL;
}
void * CDHCamera::DealH264ToYUVStreamFunc()
{
	int bRet = 0;
	int nSize = 0;
	bool isOk = false;
	while(!m_bEndCapture)
	{
		#ifdef DIO_RTSP
		if(!isOk)
		{
			//获取码流的宽高
			sStrmInfo si;
			if(m_StreamParser.StrmInfo(si) == 0)
			{
				printf("*********si.uWidth=%d,si.uHeight=%d,si.nFrameRate=%d  **********\n",si.uWidth,si.uHeight,(int)si.fFrameRate);
				isOk = true;
			}
			else
			{
				usleep(10*1000);
				continue;
			}
		}

		int iErr = m_StreamParser.NextFrame(&pMs);
		if(BM_ERR_OK == iErr && pMs)
		{
			//printf("***************uFrameIndex:%d  ***********\n",pMs->uFrameIndex);
			//解码
			bRet = m_Decoder.DecodeFrame(pMs->pData,pMs->iDataLen,m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
			yuv_video_buf  camera_header;
			if(bRet&& nSize > 0)
			{
				//printf("***************nSize:%d  ***********\n",nSize);
				//YUV头
				yuv_video_buf header;
				struct timeval tv;
				gettimeofday(&tv,NULL);
				camera_header.ts = tv.tv_sec*1000000+tv.tv_usec;
				camera_header.uTimestamp = tv.tv_sec;
				camera_header.nFrameRate = 10;
				camera_header.width = m_nWidth;
				camera_header.height = m_nHeight;
				camera_header.nSeq = m_uSeq;
				camera_header.size = nSize;
				memcpy(m_pFrameBuffer,&camera_header,sizeof(yuv_video_buf));
				AddFrame(1);
				m_uSeq++;
			}
			//printf("get frame: %d, Len: %d, type: %d\n", ++iFrames, pMs->iDataLen, pMs->eType);
			
			//释放资源
			m_StreamParser.ReleaseMediaSample(pMs);
		}
		else
		{
			usleep(10*1000);
		}
		#else
		std::string strFrame;
		m_RtspClient.PopFrame(strFrame);
		if(strFrame.size() > 0)
		{
			//printf("*********strFrame.size()=%d,%x-%x-%x-%x-%x\n",strFrame.size(),*(strFrame.c_str()),*(strFrame.c_str()+1),*(strFrame.c_str()+2),*(strFrame.c_str()+3),*(strFrame.c_str()+4));
			bRet = m_Decoder.DecodeFrame((unsigned char*)strFrame.c_str(),strFrame.size(),m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
			yuv_video_buf  camera_header;
			if(bRet&& nSize > 0)
			{
				//printf("***************nSize:%d  ***********\n",nSize);
				//YUV头
				yuv_video_buf header;
				struct timeval tv;
				gettimeofday(&tv,NULL);
				camera_header.ts = tv.tv_sec*1000000+tv.tv_usec;
				camera_header.uTimestamp = tv.tv_sec;
				camera_header.nFrameRate = 10;
				camera_header.width = m_nWidth;
				camera_header.height = m_nHeight;
				camera_header.nSeq = m_uSeq;
				camera_header.size = nSize;
				memcpy(m_pFrameBuffer,&camera_header,sizeof(yuv_video_buf));
				AddFrame(1);
				m_uSeq++;
			}
		}
		else
		{
			usleep(10*1000);
		}

		#endif
	}
	return NULL;
}

UINT32 CDHCamera::GetRealPlayId()
{
	return m_ubiStreamId;
}

//手动控制
int CDHCamera::ManualControl(CAMERA_CONFIG  cfg)
{
	printf("CDHCamera::ManualControl: cfg.nIndex=%d，cfg.fValue=%d \n",cfg.nIndex,(int)cfg.fValue);
	if(cfg.nIndex == LEFT_DIRECTION)  //向左
	{
		m_pCameraCtrl->PTZRelativeMove(-0.01,0.0,0.0);
	}
	else if(cfg.nIndex == RIGHT_DIRECTION) //向右
	{
		m_pCameraCtrl->PTZRelativeMove(0.01,0.0,0.0);
	}
	else if(cfg.nIndex == UP_DIRECTION) //向上
	{
		m_pCameraCtrl->PTZRelativeMove(0.0,0.01,0.0);
	}
	else if(cfg.nIndex == DOWN_DIRECTION)//向下
	{
		m_pCameraCtrl->PTZRelativeMove(0.0,-0.01,0.0);
	}
	else if(cfg.nIndex == ZOOM_FAR) //变倍 远景
	{
		m_pCameraCtrl->PTZRelativeMove(0.0,0.0,-0.02);
	}
	else if(cfg.nIndex == ZOOM_NEAR)//变倍 近景
	{
		m_pCameraCtrl->PTZRelativeMove(0.0,0.0,0.02);
	}

	else if(cfg.nIndex == SET_PRESET)//设置预置位
	{
		m_pCameraCtrl->PTZSetPreset((int)cfg.fValue);
	}
	else if(cfg.nIndex == CLEAR_PRESET) //删除预置位
	{
		m_pCameraCtrl->PTZRemovePreset((int)cfg.fValue);
	}
	else if(cfg.nIndex == GOTO_PRESET) //调用预置位
	{
		m_pCameraCtrl->PTZGotoPreset((int)cfg.fValue);
	}

	return 1;
}


int CDHCamera::AddRecordEvent(int nChannel,std::string result)
{
	m_nChannelId = nChannel;
	printf("CDHCamera::AddRecordEvent \n");
	pthread_mutex_lock(&m_Event_Mutex);
	//添加到事件列表
	m_EventList.push_back(result);
	pthread_mutex_unlock(&m_Event_Mutex);
	return 0;
}


std::string CDHCamera::PopEvent()
{
	std::string response;
	//加锁
	pthread_mutex_lock(&m_Event_Mutex);
	//判断是否有命令
	if(m_EventList.size() <= 0)
	{
		//解锁
		pthread_mutex_unlock(&m_Event_Mutex);
		return response;
	}
	//取最早命令
	EventMsg::iterator it = m_EventList.begin();
	//保存数据
	response = *it;
	//删除取出的命令
	m_EventList.pop_front();
	//解锁
	pthread_mutex_unlock(&m_Event_Mutex);
	return response;
}


void CDHCamera::CreateEventThread()
{
	pthread_t m_hStreamId;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //设置线程分离
	if(pthread_create(&m_hStreamId, NULL, DealEventRecordFunc, this) != 0)
	{
		pthread_attr_destroy(&attr);
		printf("Start Thread Failed. \n");
		return;
	}
	pthread_attr_destroy(&attr);
	usleep(10000); //开启线程后暂停10毫秒
	return;
}


void * CDHCamera::DealEventRecordFunc(void * lpParam)
{
	printf("CDHCamera::DealEventRecordFunc \n");
	CDHCamera *pRecord = (CDHCamera *)lpParam;
	if(pRecord)
	{
		printf("CDHCamera::DealEventRecordFunc1 \n");
		pRecord ->DealEventRecord();
	}
	printf("CDHCamera::DealEventRecordFunc2 \n");
	return NULL;
}
void * CDHCamera::DealEventRecord()
{
	while(!m_bEndRecorder)
	{
		usleep(20*1000);
		std::string strEvent = PopEvent();
		if((int)strEvent.size() > 0)
		{
			SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(strEvent.c_str());
			printf("sDetectHeader1->uRealTime=%d,CarId=%d \n",sDetectHeader1->uRealTime,sDetectHeader1->uTrafficSignal);
			//开始录像
			if(sDetectHeader1->uRealTime == 0)
			{
				//防止出现2次
				pthread_mutex_lock(&m_Event_Mutex);
				if(m_mapRecordEvent.find(sDetectHeader1->uTrafficSignal) != m_mapRecordEvent.end())
				{
					pthread_mutex_unlock(&m_Event_Mutex);
					continue;
				}
				pthread_mutex_unlock(&m_Event_Mutex);
			}
			else if(sDetectHeader1->uRealTime == 1) //车辆已离去，不在录像
			{
				pthread_mutex_lock(&m_Event_Mutex);
				for(mapEventINFO::iterator iter=m_mapRecordEvent.begin();iter!=m_mapRecordEvent.end();iter++)
				{
					if(sDetectHeader1->uTrafficSignal == iter->first)
					{
						fclose(iter->second->pOpenFile);
						unlink(iter->second->szRecordName);
						delete iter->second;
						m_mapRecordEvent.erase(iter->first);
						break;
					}
				}
				pthread_mutex_unlock(&m_Event_Mutex);
				continue;
			}
			else  //录像完成了
			{
				pthread_mutex_lock(&m_Event_Mutex);
				for(mapEventINFO::iterator iter=m_mapRecordEvent.begin();iter!=m_mapRecordEvent.end();iter++)
				{
					if(sDetectHeader1->uTrafficSignal == iter->first)
					{
						fclose(iter->second->pOpenFile);
						std::string strVideoPathTmp;
						strVideoPathTmp.append(iter->second->szRecordName);
						//g_skpDB.SaveVideo(m_nChannelId,iter->second->nBeginTime,GetTimeT(),strVideoPathTmp,2);
						delete iter->second;
						m_mapRecordEvent.erase(iter->first);
						break;
					}
				}
				pthread_mutex_unlock(&m_Event_Mutex);
				continue;
			}



			char szVideoName[100] = {0};
			memset(szVideoName,0,100);
			sprintf(szVideoName,"%s/%d.mp4",g_strVideo.c_str(),sDetectHeader1->uSeq);
			printf("VideoPath:%s \n",szVideoName);

			/*string strPlace;
			strPlace = g_skpDB.GetPlace(sDetectHeader1->uChannelID);

			string strDirection;
			int nDirection = g_skpDB.GetDirection(sDetectHeader1->uChannelID);
			strDirection =  GetDirection(nDirection);*/



			DHRecordEventInfo *pEventInfo = new DHRecordEventInfo;
			pEventInfo->bEndRecord = false;
			memset(pEventInfo->szRecordName,0,200);
			sprintf(pEventInfo->szRecordName,"%s",szVideoName);
			/*memset(pEventInfo->szPlace,0,200);
			sprintf(pEventInfo->szPlace,"%s",strPlace.c_str());
			memset(pEventInfo->szDirection,0,20);
			sprintf(pEventInfo->szDirection,"%s",strDirection.c_str());*/
			pEventInfo->nBeginTime = sDetectHeader1->uTimestamp;
			pEventInfo->pOpenFile = fopen(szVideoName,"w+");

			pthread_mutex_lock(&m_Event_Mutex);
			m_mapRecordEvent.insert(make_pair(sDetectHeader1->uTrafficSignal,pEventInfo));
			pthread_mutex_unlock(&m_Event_Mutex);

		}



		pthread_mutex_lock(&m_Event_Mutex);
		for(mapEventINFO::iterator iter=m_mapRecordEvent.begin();iter!=m_mapRecordEvent.end();iter++)
		{
			if(iter->second->bEndRecord)
			{
				fclose(iter->second->pOpenFile);
				unlink(iter->second->szRecordName);
				delete iter->second;
				m_mapRecordEvent.erase(iter->first);
				break;
			}
		}
		pthread_mutex_unlock(&m_Event_Mutex);

	}
	return NULL;
}


int CDHCamera::WriteFrameToRecordFile(unsigned char *pData,  int32_t iDataLen)
{
	pthread_mutex_lock(&m_Event_Mutex);
	for(mapEventINFO::iterator iter=m_mapRecordEvent.begin();iter!=m_mapRecordEvent.end();iter++)
	{
		//防止算法出错，造成录像一直在录
		if( abs(GetTimeT()- iter->second->nBeginTime ) > 600)
		{
			iter->second->bEndRecord = true;
		}
		else
		{
			if( abs(GetTimeT()-m_nPrintTime) > 30)
			{
				printf("Name:%s,size=%d,Carid=%d \n",iter->second->szRecordName,iDataLen,iter->first);
			}
			if(iter->second->pOpenFile)
			{
				fwrite(pData,1,iDataLen,iter->second->pOpenFile);
			}
		}
	}

	if( abs(GetTimeT()-m_nPrintTime) > 30)
	{
		m_nPrintTime = GetTimeT();
	}

	pthread_mutex_unlock(&m_Event_Mutex);
	return 0;
}

int CDHCamera::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}
//#endif


