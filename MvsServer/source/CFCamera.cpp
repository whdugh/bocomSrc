// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "CFCamera.h"
#include "Common.h"
#include <iostream>
#include "CSeekpaiDB.h"
#include "CFServer.h"

//#include "rmd_sdk.h"


#ifdef CFCAMERA
#include "rmd_sdk.h"
#include "nvr_common.h"

void hdr_callback(char *buff, int bytes, void *param)
{
// 	 	int i;
// 		printf("header length: %d [", bytes);
// 		for (i = 0; i < bytes; i++)
// 	 		printf("%c", buff[i]);
// 	 	printf("]\n");
}
void data_callback(unsigned long handle, char *buff, int bytes, void *param)
{
	CCFCamera* pHDTCPCamera = (CCFCamera*)param;
   
	printf("pHDTCPCamera=%lld,bytes=%d\n",pHDTCPCamera,bytes);

	if(pHDTCPCamera!= NULL)
	{
		if(buff != NULL && bytes > 1)
		{
			//printf("before pHDIPCamera->CaptureTcpFrame\n");

			pHDTCPCamera->CaptureTcpFrame(buff,bytes);

			//printf("after pHDIPCamera->CaptureTcpFrame\n");
		}
	}
	else
	{
		//printf("pHDIPCamera=NULL\n");
	}
}
//H264采集线程
void* ThreadCFH264Capture(void* pArg)
{
	//取类指针
	CCFCamera* pCFCamera = (CCFCamera*)pArg;
	if(pCFCamera == NULL)
		return pArg;

	pCFCamera->RecvData();
	pthread_exit((void *)0);
	return pArg;
}

void cbRecordData(unsigned int recHandle, char* pBuffer ,unsigned int BufferSize , unsigned int pDownparam)
{
	printf("recHandle= %d,BufferSize = %d,pDownparam=%lld\n",recHandle,BufferSize,pDownparam);

	CCFCamera* pHDIPCamera = (CCFCamera*)pDownparam;


	printf("pHDIPCamera=%lld\n",pHDIPCamera);

	if(pHDIPCamera!= NULL)
	{
		if(pBuffer != NULL && BufferSize > 1)
		{
			printf("before pHDIPCamera->CaptureFrame\n");

			pHDIPCamera->CaptureFrame(pBuffer,BufferSize);

			printf("after pHDIPCamera->CaptureFrame\n");
		}
	}
	else
	{
		printf("pHDIPCamera=NULL\n");
	}
}


CCFCamera::CCFCamera(int nCameraType)
{
	m_nCameraType = nCameraType;

	m_nWidth = 1920;
	m_nHeight = 1072;

	m_nMarginX = 0;
	m_nMarginY = 0;
	m_pFrameBuffer = NULL;

	m_nMaxBufferSize = 20;
	m_nDeviceId = 1;

	m_uResRecord = 0;
	
}

CCFCamera::~CCFCamera()
{

}

void CCFCamera::Init()
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

#ifdef H264_DECODE
	m_Decoder.SetVideoSize(m_nWidth,m_nHeight);
	m_Decoder.SetVideoCodeID(0);
	m_Decoder.InitDecode(NULL);
#endif

	m_bEndCapture = false;

}

bool CCFCamera::Open()
{   
	m_strData.clear();
	
    char szRtsp[256] = {0};
	if(m_nVideoType == 0)//实时视频
	{
		m_nWidth = 1920;
		m_nHeight = 1080;

		char szStream[256]= {0};

		char buf[128] = {0};
		sprintf(buf,"luxiang%d.avi",m_nDeviceId);
		fp=fopen(buf,"wb");

		if(m_nCameraType == CF_CAMERA)
		{
			sprintf(szStream,"real/1001-6-%d-1-main.h264",m_nDeviceId);
		}
		else if(m_nCameraType == CF_CAMERA_2)
		{
			m_nWidth = 1920;
			m_nHeight = 1072;
			sprintf(szStream,"real/1001-4-%d-1-main.h264",m_nDeviceId);
		}

		Init();

		printf("m_strCameraIP.c_str()=%s\n",m_strCameraIP.c_str());
		printf("m_nCameraPort=%d\n",m_nCameraPort);
		printf("szStream=%s\n",szStream);

		sprintf(szRtsp,"rtsp://%s:%d/%s",m_strCameraIP.c_str(),m_nCameraPort,szStream);

		getpid();

		int nRandCode = 8000+(int)(100*1.0*rand()/(RAND_MAX+1.0));


        if(m_nCameraType == CF_CAMERA)
		{
			//RtspClient.Init(m_strCameraIP.c_str(),m_nCameraPort,szStream);
			if(m_rtspReceiver.RegistServer(szRtsp,NULL,NULL,szStream,m_nCameraPort,nRandCode,"h264") > 0)
			{   
			m_rtspReceiver.StartRecv();
			}
		}
		 else if(m_nCameraType == CF_CAMERA_2)
		{
			//DealCFData(strData);
			if (rmd_sdk_init() != 0) {
				//printf("rmd_sdk_init() failed\n");
				
			}
			handle = rmd_realplay_start((char *)m_strCameraIP.c_str(), m_nCameraPort, 1,
				"1001", 4, m_nDeviceId, 1,	hdr_callback,this,  data_callback,this);
			if ((int)handle < 0) {
				//printf("rmd_realplay_start() failed\n");
				
			}
		}
       

		if(m_nCameraType == CF_CAMERA)
		{
			m_nThreadId = 0;

			//线程属性
			pthread_attr_t   attr;
			//初始化
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

			int nret=pthread_create(&m_nThreadId,NULL,ThreadCFH264Capture,this);

			if(nret!=0)
			{
				Close();
				//失败
				LogError("创建h264采集线程失败,无法进行采集！\r\n");
			}
			pthread_attr_destroy(&attr);
		}
		//线程ID
	

		g_nLoginState = 1;
	}
	else
	{
		m_nWidth = 1920;
		m_nHeight = 1072;

		Init();
	}

	return true;
}

bool CCFCamera::Close()
{
	m_bEndCapture = true;
	fclose(fp);

if(m_nCameraType == CF_CAMERA_2)
{
	if (rmd_realplay_stop(handle) != 0)
		printf("rmd_realplay_stop() failed\n");
	else
		printf("rmd_realplay_stop() succ\n");
}
	if(m_uResRecord > 0)
	{
		AVS_stopGetRecord(m_uResRecord);
		m_uResRecord = 0;
	}
	
	if(m_nVideoType == 0)
	{
		if(m_nThreadId!=0)
		{
			pthread_join(m_nThreadId, NULL);
			m_nThreadId = 0;
		}

		//RtspClient.UnInit();
	}

#ifdef H264_DECODE
	m_Decoder.UinitDecode();
#endif

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

	return true;
}


//reconnect the camera
bool CCFCamera::ReOpen()
{
	if(g_nLoginState != 1)
	{
		return false;
	}
	
	if(m_nVideoType == 1)
	{
		Close();
		
		Open();

		PullStream();
	}

	return true;
}

//处理长峰平台特有数据
void CCFCamera::DealCFData(string& strMsg)
{
	if( strMsg.size() >= 17 )
	{
			unsigned char p0 = *((unsigned char*)(strMsg.c_str()));
			unsigned char p1 = *((unsigned char*)(strMsg.c_str()+1));
			unsigned char p2 = *((unsigned char*)(strMsg.c_str()+2));
			unsigned char p3 = *((unsigned char*)(strMsg.c_str()+3));

			unsigned char p4 = *((unsigned char*)(strMsg.c_str()+4));
			unsigned char p5 = *((unsigned char*)(strMsg.c_str()+5));
			unsigned char p6 = *((unsigned char*)(strMsg.c_str()+6));
			unsigned char p7 = *((unsigned char*)(strMsg.c_str()+7));

			unsigned char p12 = *((unsigned char*)(strMsg.c_str()+12));
			unsigned char p13 = *((unsigned char*)(strMsg.c_str()+13));
			unsigned char p14 = *((unsigned char*)(strMsg.c_str()+14));
			unsigned char p15 = *((unsigned char*)(strMsg.c_str()+15));
			unsigned char p16 = *((unsigned char*)(strMsg.c_str()+16));
			printf("p0=%x,p1=%x,p2=%x,p3=%x,p4=%x,p5=%x,p6=%x,p7=%x,p12=%x,p13=%x,p14=%x,p15=%x,p16=%x\n",p0,p1,p2,p3,p4,p5,p6,p7,p12,p13,p14,p15,p16);

			if(!(p12 == 0x0 && p13 == 0x0 && p14 == 0x0 && p15 == 0x1))
			{
				strMsg = "";

				printf("!(p12 == 0x0 && p13 == 0x0 && p14 == 0x0 && p15 == 0x1)\n");
				return;
			}


			unsigned int uLen = *((unsigned int*)(strMsg.c_str()+4));
			printf("uLen = %d\n",uLen);
			unsigned int uTimeStamp = *((unsigned int*)(strMsg.c_str()+8));
			printf("uTimeStamp = %d\n",uTimeStamp);
			strMsg.erase(0,12);
			printf("========strMsg.size()=%d\n",strMsg.size());
	}
}

//接收h264数据
void CCFCamera::RecvData()
{
	unsigned char sps[24] = {0,0,0,1,0x67,0x42,0,0x29,0xe2,0x90,0x0f,0,0x44,0xfc,0xb8,0x0b,0x70,0x10,0x10,0x1e,0x1e,0x24,0x45,0x40};
	unsigned char pps[8] = {0,0,0,1,0x68,0xce,0x3c,0x80};
	bool bPPS = false;
	

	while(!m_bEndCapture)
	{
		std::string strData;
		if(m_nCameraType == CF_CAMERA)
		{
			//RtspClient.PopFrame(strData);
			m_rtspReceiver.PopOneFrameData(strData);
		}
		
		
		//2期数据需要特殊处理



     if(m_nCameraType == CF_CAMERA)
	 {
		if(strData.size() > 0)
		{
			int nSize = 0;
			bool bRet = false;
			
			
				if(*((unsigned char*)strData.c_str()+4) == 0x65 && !bPPS)
				{
					strData.insert(0,(char*)pps,8);
					strData.insert(0,(char*)sps,24);
					bPPS= true;
				}
			
			 

			#ifdef H264_DECODE		
				bRet = m_Decoder.DecodeFrame((unsigned char* )strData.c_str(),strData.size(),m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
			#endif

			if(bRet&& nSize > 0)
			{
					yuv_video_buf header;
					struct timeval tv;
					gettimeofday(&tv,NULL);
					header.ts = tv.tv_sec*1000000+tv.tv_usec;//(int64_t)pts*1000*1000;
					header.uTimestamp = tv.tv_sec;

					if(m_nCameraType == CF_CAMERA)
					{
						header.nFrameRate = 15;
					}
					else if(m_nCameraType == CF_CAMERA_2)
					{
						header.nFrameRate = 25;
					}

					header.width = m_nWidth;
					header.height = m_nHeight;
					header.nSeq = m_uSeq;
					header.size = nSize;
					
					memcpy(m_pFrameBuffer,&header,sizeof(yuv_video_buf));

					//	printf("===============CaptureFrame timenow =%lld, m_uSeq=%u\n",GetTimeStamp(),m_uSeq);

					AddFrame(1);
					m_uSeq++;
			}
		}
	  }
		usleep(1000*1);
	}
}

void CCFCamera::CaptureTcpFrame(char* buff ,unsigned int bytes)
{   
	//fwrite(buff,bytes,1,fp);
	printf("==========BufferSize = %d\n",bytes);
	int nSize = 0;
	bool bRet = false;
#ifdef H264_DECODE		
	bRet = m_Decoder.DecodeFrame((unsigned char* )buff,bytes,m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
#endif

	if(bRet&& nSize > 0)
	{
		yuv_video_buf header;
		struct timeval tv;
		gettimeofday(&tv,NULL);
		header.ts = tv.tv_sec*1000000+tv.tv_usec;//(int64_t)pts*1000*1000;
		header.uTimestamp = tv.tv_sec;

		if(m_nCameraType == CF_CAMERA)
		{
			header.nFrameRate = 15;
		}
		else if(m_nCameraType == CF_CAMERA_2)
		{
			header.nFrameRate = 25;
		}

		header.width = m_nWidth;
		header.height = m_nHeight;
		header.nSeq = m_uSeq;
		header.size = nSize;

		memcpy(m_pFrameBuffer,&header,sizeof(yuv_video_buf));

		//	printf("===============CaptureFrame timenow =%lld, m_uSeq=%u\n",GetTimeStamp(),m_uSeq);

		AddFrame(1);
		m_uSeq++;
	}
}

//h264图像采集
void CCFCamera::CaptureFrame(char* pBuffer ,unsigned int BufferSize)
{
	printf("==========BufferSize = %d\n",BufferSize);

	if(!m_bEndCapture)
	{			
		      //  return;

				m_strData.append(pBuffer,BufferSize);
				bool bIsAppend = true;
				unsigned char* pData = (unsigned char*)m_strData.c_str();

				int nStart = -1;
				
				bool bFindFrame = false;

				for(int i =0;i<(m_strData.size()-4);i++)
				{
						if((pData[i] == 0x00) && (pData[i+1] == 0x00) && (pData[i+2] == 0x00) && (pData[i+3] == 0x01) && ((pData[i+4] == 0x61)||(pData[i+4] == 0x67)) )
						{
								printf("=========find-onr- frame======%d\n",i);
							    bFindFrame = true;

								if(i > nStart && nStart >= 0)
								{
													int nSize = 0;
													bool bRet = false;

													printf("========before DecodeFrame\n");

										#ifdef H264_DECODE		
													bRet = m_Decoder.DecodeFrame(pData+nStart,i-nStart,m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
										#endif

													printf("=========after DecodeFrame\n");
														
													if(bRet&& nSize > 0)
													{
																yuv_video_buf header;
																struct timeval tv;
																gettimeofday(&tv,NULL);
																header.ts = tv.tv_sec*1000000+tv.tv_usec;//(int64_t)pts*1000*1000;
																header.uTimestamp = tv.tv_sec;
																header.nFrameRate = 25;
																header.width = m_nWidth;
																header.height = m_nHeight;
																header.nSeq = m_uSeq;
																header.size = nSize;

																memcpy(m_pFrameBuffer,&header,sizeof(yuv_video_buf));

															//	printf("===============CaptureFrame timenow =%lld, m_uSeq=%u\n",GetTimeStamp(),m_uSeq);

																AddFrame(1);
																m_uSeq++;
													}
													if (bIsAppend)
													{
														bIsAppend = false;
														continue;
													}
													usleep(1000*1);

								}
								nStart = i;
						}
				}

				if(bFindFrame)
				{
					m_strData.erase(0,nStart);
				}
	}
}

//设置设备编号
void CCFCamera::SetDeviceID(int nDeviceID)
{
	m_nDeviceId = nDeviceID;
}

//获取视频流
void CCFCamera::PullStream()
{
	//历史视频
	if(m_nVideoType == 1)
	{
		unsigned int loginID = g_CFCommunication.GetLoginId();
		if (loginID > 0 )
		{
			string strCamID = g_skpDB.GetPlaceByCamID(m_nDeviceId);//暂时用地点表示相机编号

			printf("====this=%lld\n",this);

			m_uResRecord = AVS_getRecord( loginID ,(char*)strCamID.c_str() ,1 ,m_vod_info.uBeginTime, m_vod_info.uEndTime , cbRecordData,this );
		}
	}
}


#endif
