// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifdef HIKVISIONCAMERA
#include "Hikvision.h"
#include "Common.h"
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include "CSeekpaiDB.h"
#include "ippi.h"
#include "ippcc.h"
#include "AnalyzeDataNewInterface.h"


//H264解码线程
void* ThreadHikvisionH264Decode(void* pArg)
{
	//取类指针
	CHikvision* pHikvision = (CHikvision*)pArg;
	if(pHikvision == NULL)
		return pArg;

	pHikvision->DecodeFrame();
	pthread_exit((void *)0);
	return pArg;
}

void CHikvision::fCallback(int handle,const char* data,int size,void *pUser,int iDataType)
{
	//LogNormal("Enter Into fCallback\n");
	CHikvision* pHikvision = (CHikvision*)pUser;
	
	if (!pHikvision)
	{
		return;
	}
	
	if(pHikvision!= NULL)
	{
		if(data != NULL && size > 1)
		{
			pHikvision->CaptureFrame(const_cast<char*>(data),size);	
		}
	}
}


CHikvision::CHikvision(int nCameraType)
{
	m_nCameraType = nCameraType;

	m_nWidth = 0;
	m_nHeight = 0;

	m_nMarginX = 0;
	m_nMarginY = 0;
	m_pFrameBuffer = NULL;

	m_nMaxBufferSize = 5;
	m_nDeviceId = 1;
	//m_strData = "";
	m_hAnalyze = NULL;
}

CHikvision::~CHikvision()
{

}

void CHikvision::Init()
{
	if(m_nWidth > 0)
	{
		for(int i=0; i< m_nMaxBufferSize; i++)
		{
			m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3,sizeof(unsigned char));
		}

		m_uSeq = 0;
		m_nPreFieldSeq  = 0;
		m_nCurIndex = 0;
		m_nFirstIndex = 0;
		m_nFrameSize=0;
		m_pFrameBuffer = m_FrameList[0];
	}

	printf("Init==m_pFrameBuffer=%lld=m_nWidth=%d,m_nHeight=%d,m_nMaxBufferSize=%d\n",m_pFrameBuffer,m_nWidth,m_nHeight,m_nMaxBufferSize);


	//线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    int nret=pthread_create(&m_nThreadId,NULL,ThreadHikvisionH264Decode,this);

    if(nret!=0)
    {
        //失败
        LogError("创建采集线程失败,无法进行采集！\r\n");
    }

    pthread_attr_destroy(&attr);
}


bool CHikvision::Open()
{
	 printf("==CHikvision::Open\n");
	//读取相机参数
	m_nVideoType = 0;
	m_bReadFile = false;
	m_bEndCapture = false;
	//m_strData.clear();
	m_listStrData.clear();
	m_nWidth = 0;
    m_nHeight = 0;

	//Init();
	m_hAnalyze = NULL;
	m_bEndCapture = false;

	return true;
}

bool CHikvision::Close()
{
	m_bEndCapture = true;

	if (m_hStream >= 0)//播放成功
	{
		int nStopVideo = Plat_StopVideo(m_hStream);//停止视频播放
		if (nStopVideo == 0)
		{
			printf("停止播放成功！\n");
		}
		else
			printf("停止播放失败！\n");
	}

	if(m_hAnalyze != NULL)
	{
		HIKANA_Destroy(m_hAnalyze);
		m_hAnalyze = NULL;
	}

	if (m_nThreadId != 0)
    {
        pthread_join(m_nThreadId, NULL);
        m_nThreadId = 0;
    }

	//加锁
	pthread_mutex_lock(&m_FrameMutex);
	//释放内存
	for (int i=0; i<MAX_SIZE; i++)
	{
		if(m_FrameList[i] != NULL)
		{
			free(m_FrameList[i]);
			m_FrameList[i] = NULL;
		}
		m_nFrameSize = 0;
		m_pFrameBuffer = NULL;
	}
	//解锁
	pthread_mutex_unlock(&m_FrameMutex);

	m_nWidth = 0;
    m_nHeight = 0;

	return true;
}

//打开文件
bool CHikvision::OpenFile(const char* strFileName)
{
	m_nVideoType = 1;
	m_bReadFile = true;
	m_bEndCapture = false;
	string strFilePathName(m_vod_info.chFilePath);
	printf("strFilePathName=%s\n",strFilePathName.c_str());
	//Init();

	return true;
}


//reconnect the camera
bool CHikvision::ReOpen()
{
	if(g_nLoginState != 1)
	{
		return false;
	}

	Close();
	
	if(m_bReadFile)
	{
		OpenFile("");
	}
	else
	{
		Open();
	}

	PullStream();

	return true;
}


//h264图像采集
void CHikvision::CaptureFrame(char* pBuffer ,unsigned int BufferSize)
{
	/*FILE* fp = fopen("Capture.avi","wb");
	fwrite(pBuffer,1,BufferSize,fp);
	fclose(fp);*/

	//if(!m_bEndCapture)
	{			
				int hr                    = 0;
				int bSuc                  = 0;	
				unsigned int dwBytes      = 0;	
				unsigned int dwVideoFrame = 0;
				unsigned int dwAudioFrame = 0;
				unsigned long dwBufSize   = 0;
				FILE* hFile               = NULL;
				unsigned char* pDataBuf   = NULL;
				unsigned char pBuf[40]    = {0};
				PACKET_INFO_EX stInfoEx   = {0};

				if(m_hAnalyze == NULL)
				{
					memcpy(pBuf,pBuffer,40);
					m_hAnalyze = HIKANA_CreateStreamEx(dwBufSize,pBuf);
				}
				
				if(m_hAnalyze == NULL)
				{
					printf("=========m_hAnalyze == NULL\n");
					return;
				}

				bSuc = HIKANA_InputData(m_hAnalyze, (unsigned char*)pBuffer,BufferSize);	
				if (1 != bSuc)
				{
					LogError("========HIKANA_InputData error\n");
					return;
				}

				hr = HIKANA_GetOnePacketEx(m_hAnalyze, &stInfoEx);
				if (0 == hr)
				{
					if ((VIDEO_I_FRAME == stInfoEx.nPacketType) || 
						(VIDEO_P_FRAME == stInfoEx.nPacketType) || 
						(VIDEO_B_FRAME == stInfoEx.nPacketType))
					{
						//get video packet
						//dwVideoFrame++;
						if (VIDEO_I_FRAME == stInfoEx.nPacketType)
						{
							//get video resolution
							int width  = stInfoEx.uWidth;
							int heigth = stInfoEx.uHeight;
						
							printf("===HIKANA_GetOnePacketEx=====width=%d,heigth=%d\n",width,heigth);
						}
					}
					else
					{
						printf("========no video frame\n");
						return;
					}
				}
				else
				{
					return;
				}

				if(m_nWidth == 0)
				{
					if(stInfoEx.uWidth > 0&& stInfoEx.uWidth < 2000)
					{
						if(stInfoEx.uHeight == 1088)
						{
							stInfoEx.uHeight = 1080;
						}

						m_nWidth = stInfoEx.uWidth;
						m_nHeight = stInfoEx.uHeight;

						Init();
					}
				}

				if(stInfoEx.dwPacketSize <= 0)
				{
					return;
				}
				
				string strData;
				strData.append((char*)stInfoEx.pPacketBuffer,stInfoEx.dwPacketSize);

				printf("CaptureFrame strData.size=%d\n",strData.size());

				 //加锁缓冲区
				pthread_mutex_lock(&m_FrameMutex2);

				m_listStrData.push_back(strData);

				 pthread_mutex_unlock(&m_FrameMutex2);

				return;

	}
}

//获取视频流
void CHikvision::PullStream()
{
	//if (g_HikvisionCommunication.GetInit() == 0 )
	{ 
		int nHandle = g_HikvisionCommunication.GetLoginHandle();
		printf("PullStream nHandle=%d\n",nHandle);
				
		if ( nHandle >= 0)
		{
			//相机编号从断面地点中获取
			string strDeviceID = g_skpDB.GetPlaceByCamID(m_nDeviceId);
			//m_nDeviceId = atol(strDeviceID.c_str());

			char buf[64] = {0};
			sprintf(buf,"%d",m_nDeviceId);
			string strCameraID(buf); 
			LogNormal("strCameraID=%s\n",strCameraID.c_str());
			
			if(m_nVideoType == 0)//实时视频
			{
				m_pURL = Plat_QueryRealStreamURL(strCameraID.c_str(),nHandle);
			
				if (m_pURL)
				{	
					string sUrl = m_pURL;
					LogNormal("m_pURL:%s\n",sUrl.c_str());
					m_hStream = Plat_PlayVideo(m_pURL,NULL,nHandle,fCallback,this);
					LogNormal("m_hStream=%d\n",m_hStream);
				}
				else
				{
					int nError = Plat_GetLastError();
					LogNormal("nError=%d\n",nError);
				}
			}

			if(m_nVideoType == 1)//远程历史视频
			{
				LogNormal("Start To Get History Video.....\n");
				char chBuf[64] = {0};
				sprintf(chBuf,"%d",3);
				string strQueryCondition(chBuf); 

				LogNormal("strQueryCondition=%s,uBeginTime=%d,uEndTime=%d\n",strQueryCondition.c_str(),m_vod_info.uBeginTime,m_vod_info.uEndTime);
				int nRecord = Plat_QueryRecordFile(strCameraID.c_str(),m_vod_info.uBeginTime,m_vod_info.uEndTime,strQueryCondition.c_str(),nHandle);
				if (nRecord == -1)
				{	
					//查询录像信息失败
					LogNormal("Query Record File Error=====%d\n",Plat_GetLastError());
				}
				else
				{
					LogNormal("Success to Query Record File\n");

					while (Plat_MoveNext(nHandle) != -1)
					{

						string strEndime =  Plat_GetValue("endtime", nHandle);
						LogNormal("pEndEndime=====%s\n",strEndime.c_str() );

						string strFileName =  Plat_GetValue("filename", nHandle);
						LogNormal("strFileName=====%s\n",strFileName.c_str());

						string strFileSize =  Plat_GetValue("size", nHandle);
						LogNormal("strFileSize=====%d\n",strFileSize.c_str());

						string strStartTime = Plat_GetValue("starttime", nHandle);
						LogNormal("pStartTime=====%s\n",strStartTime.c_str());	
						
					}
				}
							
			}
		} 
		
	}
}

//h264解码
void CHikvision::DecodeFrame()
{
	#ifdef H264_DECODE
	m_Decoder.SetVideoSize(m_nWidth,m_nHeight);
	m_Decoder.SetVideoCodeID(0);
	m_Decoder.InitDecode(NULL);
	#endif

	while(!m_bEndCapture)
	{			
				string m_strData("");

				 //加锁缓冲区
				pthread_mutex_lock(&m_FrameMutex2);
				if(m_listStrData.size() > 0)
				  {
						m_strData = *(m_listStrData.begin());
						m_listStrData.pop_front();
				  }
				pthread_mutex_unlock(&m_FrameMutex2);

				if(m_strData.size() > 0)
				{
					
					printf("5555===m_nWidth=%d,m_nHeight=%d,m_nMaxBufferSize=%d\n",m_nWidth,m_nHeight,m_nMaxBufferSize);

					int nSize = 0;
					bool bRet = false;
					
					printf("m_pFrameBuffer=%lld,sizeof(yuv_video_buf)=%d\n",m_pFrameBuffer,sizeof(yuv_video_buf));
	#ifdef H264_DECODE		
					bRet = m_Decoder.DecodeFrame((unsigned char*)m_strData.c_str(),m_strData.size(),m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
	#endif
					if(bRet&& nSize > 0)
					{
								yuv_video_buf header;
								struct timeval tv;
								gettimeofday(&tv,NULL);
								header.ts = (int64_t)tv.tv_sec*1000000+tv.tv_usec;
								header.uTimestamp = tv.tv_sec;
								header.nFrameRate = 25;
								header.width = m_nWidth;
								header.height = m_nHeight;
								header.nSeq = m_uSeq;
								header.size = nSize;

								memcpy(m_pFrameBuffer,&header,sizeof(yuv_video_buf));

								printf("CaptureFrame tv_sec=%lld\n",tv.tv_sec);
								
								AddFrame(1);
								m_uSeq++;
					}
				}

				usleep(1000*1);
	}

	#ifdef H264_DECODE
	m_Decoder.UinitDecode();
	#endif
}

//云台控制
int CHikvision::ManualControl(CAMERA_CONFIG cfg)
{ 
	
	int nPTZCommand = 0;
	int nPreset = 0;
	int nParam = (int)cfg.fValue;

	if(cfg.nIndex == SET_PRESET)
	{
		nPTZCommand = 8;
		nPreset = nParam;
	}
	if(cfg.nIndex == GOTO_PRESET)
	{
		nPTZCommand = 39;
		nPreset = nParam;
	}
	if(cfg.nIndex == CLEAR_PRESET)
	{
		nPTZCommand = 9;
		nPreset = nParam;
	}
	if (cfg.nIndex == RIGHT_DIRECTION)//镜头向右
	{
		nPTZCommand = 24;
	}
	if (cfg.nIndex == DOWN_DIRECTION)//镜头向下
	{
		nPTZCommand = 22;
	}
	if (cfg.nIndex == LEFT_DIRECTION)//镜头向左
	{
		nPTZCommand = 23;
	}
	if (cfg.nIndex == UP_DIRECTION)//镜头向上
	{
		nPTZCommand = 21;
	}
	if (cfg.nIndex == ZOOM_NEAR)//镜头拉近
	{
		nPTZCommand = 11;
	}	
	if (cfg.nIndex == ZOOM_FAR)//镜头拉远
	{
		nPTZCommand = 12;
	}	
	if (cfg.nIndex == FOCUS_NEAR)//焦点前调
	{
		nPTZCommand = 13;
	}
	if (cfg.nIndex == FOCUS_FAR)//焦点后调
	{
		nPTZCommand = 14;
	}	
	
	int nHandle = g_HikvisionCommunication.GetLoginHandle();
	if (nHandle >= 0)
	{
		//Plat_LoadResource(nHandle);
		//相机编号从断面地点中获取
		string strControlCameraId = g_skpDB.GetPlaceByCamID(m_nDeviceId);

		LogNormal("=Camera[%s],Command[%d],nParam[%d],nPreset[%d]\n",strControlCameraId.c_str(),nPTZCommand,nParam,nPreset);

		int nControlCamera =  Plat_ControlCamera(strControlCameraId.c_str(),nPTZCommand,nParam, 1, nPreset, 0, nHandle);

		                      //Plat_ControlCamera(cId, iCmd, iParam, 1, iPreset, 0, NULL, g_iUserhandle);

		if (nControlCamera == 0)
		{
			LogNormal("success to Control Camera[%d]\n",nPTZCommand);
		}
		else
		{
			LogNormal("Fail to Control Camera,Error=[%d]\n",Plat_GetLastError());
		}

	}
		
	return 1;
}

#endif
