// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2013 上海博康智能信息技术有限公司
// Copyright 2008-2013 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "DspDealVideo.h"
#include "Common.h"
#include "CommonHeader.h"

#include "global.h"

#include "CRtspClient.h"
#include "RoadH264Capture.h"
#include "CenterServerOneDotEight.h"

//回调函数
int DspStrmCallback(UINT64 ubiStrmId, unsigned char *pData,  int iDataLen, void *pParam)
{
	printf("DspStrmCallback id: %lld len: %d\n",ubiStrmId, iDataLen);
	//取类指针
	CDspDealVideo* pDspDealVideo = (CDspDealVideo*)pParam;
	if(pDspDealVideo == NULL)
		return 0;

	if(iDataLen > 0)
	{
		pDspDealVideo->RecvRtspDioData(pData, iDataLen);//		
	}

	return 1;
}

//H264采集线程
void* ThreadDspCaptureVideo(void* pArg)
{
	//取类指针
	CDspDealVideo* pDspDealVideo = (CDspDealVideo*)pArg;
	if(pDspDealVideo == NULL)
		return pArg;

	pDspDealVideo->CaptureVideo();
	pthread_exit((void *)0);
	return pArg;
}

CDspDealVideo::CDspDealVideo()
{
	m_bEndCapture = false;

	m_uBeginTime = 0;
	m_uEndTime = 0;
	m_nChannelId = 0;
	m_strH264FileName = "";
}

CDspDealVideo::~CDspDealVideo()
{
	//
}

//初始化
bool CDspDealVideo::Init(int nChannel)
{
	m_nChannelId = nChannel;
}

//初始化一个接收单元
bool CDspDealVideo::InitUnit(DspVideoClient_Fd &dspVideoCfd, 
	bool bH264Capture, bool bSkpRecorder, 
	CRoadH264Capture * pH264Capture, CSkpRoadRecorder * pSkpRecorder)
{
	if ((!bH264Capture && !bSkpRecorder) || (NULL == pH264Capture && NULL == pSkpRecorder))
	{
		printf("--bH264Capture-=%d, bSkpRecorder=%d--\n", bH264Capture, bSkpRecorder);
		return false;
	}

	m_cfd = dspVideoCfd;
	m_bH264Capture  = bH264Capture;
	m_bSkpRecorder = bSkpRecorder;

	m_pH264Capture = pH264Capture;	
	m_pSkpRecorder = pSkpRecorder;
	

	//初始化环形缓冲区
	for (int i=0; i<NUM_VIDEO_BUFF; i++)
	{
		listVideoBuff[i].sizeBuff = 0;
		listVideoBuff[i].bIsLocked = false;
		memset(listVideoBuff[i].pBuff, 0, MAX_VIDEO_BUFF);
	}
	

	//线程ID
	m_nThreadVideo = 0;
	memset(m_chRtspHead, 0, MAX_RTSP_HEAD);

	m_bEndCapture = false;

	return true;
}

//释放一个接收单元
bool CDspDealVideo::UnInitUnit(DspVideoClient_Fd &dspVideoCfd)
{
	if(m_pH264Capture != NULL)
	{
		m_pH264Capture->UnInit();
		m_pH264Capture = NULL;
		printf("===after m_pH264Capture->UnInit()==\n");
	}

	if(m_pSkpRecorder != NULL)
	{
		m_pSkpRecorder->UnInit();
		m_pSkpRecorder = NULL;

		printf("===after m_pSkpRecorder->UnInit()==\n");
	}

	m_bEndCapture = true;
}

bool CDspDealVideo::AddRtspcfd(
	DspVideoClient_Fd &dspVideoCfd,
	bool bH264Capture, 
	bool bSkpRecorder, 
	CRoadH264Capture * pH264Capture, 
	CSkpRoadRecorder * pSkpRecorder)
{
	bool bRet = false;
	bool bInit = InitUnit(dspVideoCfd, bH264Capture, bSkpRecorder, pH264Capture, pSkpRecorder);
	if(bInit)
	{
		printf("==CDspDealVideo::Init()==bH264Capture,bSkpRecorder,pH264Capture,pSkpRecorder--[%d=%d, %x, %x]==\n",\
			bH264Capture,bSkpRecorder,pH264Capture,pSkpRecorder);

		int nret = pthread_create(&m_nThreadVideo, NULL, ThreadDspCaptureVideo, this);
		if(0 != nret)
		{
			UnInitUnit(dspVideoCfd);
			//失败
			LogError("创建Dsp-H264码流采集线程失败,无法进行采集！\r\n");
			bRet = false;
		}
		else
		{
			bRet = true;
		}
	}
	else
	{
		printf("--AddRtspcfd---InitUnit--fail!\n");
	}

	return bRet;
}

#ifdef DIO_RTSP_RECV
//初始化rtsp库DIO
int CDspDealVideo::RtspDIOInit(UINT64 &ubiRtspStrmId, UINT64 &ubiTCPStrmId, const int iConnType,
		const char *pIp, const int uPort, const char *pUrl,
		const char *pUser, const char *pPw)
{
	int iErr = 0;
	if(pUrl)
	{
		LogNormal("pUrl=%s\n", pUrl);
		LogNormal("uPort=%d, pIp=%s\n", uPort, pIp);
		DIO::DIOInit();

		printf("--111--DIO::OpenStrm--\n");
		printf("pUrl=%s\n uPort=%d, pIp=%s\n", pUrl, uPort, pIp);
		ubiRtspStrmId = DIO::OpenStrm(pUrl, pUser, pPw);
		printf("--222--DIO::OpenStrm--\n");


		if(ubiRtspStrmId > 0)
		{
			printf("open rtsp: %s ok\n", pUrl);
			DIO::SetVideoStrmCallback(ubiRtspStrmId, (void *)(DspStrmCallback), this);
		}
		else
		{
			iErr = DIO::GetLastErr();
			printf("open rtsp: %s failed<0x%x>\n", pUrl, iErr);
		}
	}

	if(pIp && 0 != uPort && 0 != iConnType)
	{
		ubiTCPStrmId = DIO::OpenTcpStrm(pIp, uPort, iConnType);
		if(ubiTCPStrmId > 0)
		{
			printf("open <%s:%u, %d> ok\n", pIp, uPort, iConnType);
			DIO::SetVideoStrmCallback(ubiTCPStrmId, (void *)(DspStrmCallback), this);
		}
		else
		{
			iErr = DIO::GetLastErr();
			printf("open <%s:%u, %d> failed<0x%x>\n", pIp, uPort, iConnType, iErr);
		}
	}

	return iErr;
}
#endif

#ifdef DIO_RTSP_RECV
//接入基础RTSP库
void CDspDealVideo::RecvRTSP()
{
	//采集时间(以微妙为单位)
	struct timeval tv;
	bool bInitRtsp = false;

	int iErr = 0;		
	int nFrameSize = 0;
	int64_t tick = 0;

	while(!m_bEndCapture)
	{
		if(bInitRtsp)
		{
			if(m_bH264Capture || m_bSkpRecorder)
			{
				//std::string strData;
				//RtspClient.PopFrame(strData);
				pthread_mutex_lock(&m_rtspdio_mutx);

				nFrameSize = listVideoBuff[m_uPrevVideoBuff].sizeBuff;

				if(nFrameSize > 0)
				{
					gettimeofday(&tv,NULL);
					tick = tv.tv_sec * 1000000 + tv.tv_usec;

					CaptureVideoFrame(tick, &(listVideoBuff[m_uPrevVideoBuff]));
					/*
					if(m_pH264Capture != NULL && m_bH264Capture)
					{
						//printf("===bytes=*%d==\n", bytes);
						Image_header_dsp header; //添加H264图像头
						header.nType = DSP_IMG_H264_PIC; //标记H264码流类型						
						header.nWidth = 320;
						header.nHeight = 240;
						header.nFrameRate = 15;
						header.nSize = nFrameSize;
						//header.nSeq = uSeq;
						//取当前机器时间
						header.ts = tv.tv_sec * 1000000 + tv.tv_usec;

						m_pH264Capture->AddFrame2((unsigned char*)listVideoBuff[m_uPrevVideoBuff].pBuff, header);
					}


					if(m_pSkpRecorder != NULL && m_bSkpRecorder)
					{
						SRIP_DETECT_HEADER sDetectHeader;
						sDetectHeader.uChannelID = 1;		//通道ID FIX!!
						sDetectHeader.uDetectType = 1;		//0 图片 1 检测图片 2检测结果 ?? h264
						sDetectHeader.uTimestamp = tv.tv_sec;		//时间戳
						//sDetectHeader.uSeq = uSeq;				//帧序号
						sDetectHeader.uTime64 = tv.tv_sec * 1000000 + tv.tv_usec;			//时间戳
						sDetectHeader.uWidth = 320;			//宽度
						sDetectHeader.uHeight = 240;			//高度
						//sDetectHeader.uRealTime = 0;			//(客户端:线圈状态；中心端：实时信息)
						sDetectHeader.uVideoId = 1;        //录像编号(0:jpg,1:h264)
						sDetectHeader.dFrameRate = 10;   //帧率
						//sDetectHeader.uImageRegionType;   //图像区域类型
						//sDetectHeader.uTrafficSignal;//控制灯状态

						m_pSkpRecorder->AddH264Frame((char*)listVideoBuff[m_uPrevVideoBuff].pBuff, nFrameSize, sDetectHeader);
					}
					
					*/
					listVideoBuff[m_uPrevVideoBuff].sizeBuff = 0;//使用完size清0
				}
				pthread_mutex_unlock(&m_rtspdio_mutx);
			}
			else
			{					
				//RtspClient.UnInit();
				LogNormal("--Rtsp-CloseStrm--");
				DIO::CloseStrm((uint64_t)0);
				DIO::DIOUninit();
				bInitRtsp = false;
			}
		}//End of if(bInitRtsp)
		else
		{
			if(m_bH264Capture || m_bSkpRecorder)
			{					
				//RtspClient.Init(m_strCameraIP.c_str(),8557,"h264");
				iErr = RtspDIOInit(m_cfd.ubiRtspStrmId,m_cfd.ubiTCPStrmId,\
					m_cfd.nConnType,m_cfd.szIp,m_cfd.nPort,m_cfd.szUrl,m_cfd.szUser,m_cfd.szPw);

				if(m_cfd.ubiRtspStrmId > 0)
				{
					bInitRtsp = true;
				}
				else
				{
					LogNormal("-RtspDIOInit fail-iErr-%x--\n", iErr);
				}
			}
		}
		usleep(1000*5);
	}

	if(bInitRtsp)
	{
		//RtspClient.UnInit();
		DIO::CloseStrm((uint64_t)0);
		DIO::DIOUninit();
	}
}
#endif


//拷贝接受Rtsp数据
void CDspDealVideo::RecvRtspDioData(unsigned char *pData, const int iDataLen)
{
	//printf("-111-RecvRtspDioData-m_uCurrH264Buff=%d,m_uPrevH264Buff=%d \n", m_uCurrH264Buff, m_uPrevH264Buff);
	if(iDataLen > MAX_VIDEO_BUFF)
	{
		LogNormal("--RTSP--Data too large!\n", iDataLen);
		return;
	}

	if(iDataLen > 0)
	{
		pthread_mutex_lock(&m_rtspdio_mutx);

		if(iDataLen + m_nRtspHeadLen <= MAX_RTSP_HEAD)
		{
			memcpy(m_chRtspHead, pData, iDataLen);
			m_nRtspHeadLen = iDataLen;
		}
		else
		{
			if(0 == m_nRtspHeadLen)
			{
				listVideoBuff[m_uCurrVideoBuff].sizeBuff = iDataLen;
				memcpy((char*)listVideoBuff[m_uCurrVideoBuff].pBuff, pData, iDataLen);
			}
			else
			{
				listVideoBuff[m_uCurrVideoBuff].sizeBuff = iDataLen + m_nRtspHeadLen;
				memcpy((char*)listVideoBuff[m_uCurrVideoBuff].pBuff, (char*)(m_chRtspHead), m_nRtspHeadLen);
				memcpy((char*)listVideoBuff[m_uCurrVideoBuff].pBuff + m_nRtspHeadLen, pData, iDataLen);

				memset(m_chRtspHead, 0, MAX_RTSP_HEAD);
				m_nRtspHeadLen = 0;
			}
			printf("--data Size=%d -#", listVideoBuff[m_uCurrVideoBuff].sizeBuff);

			m_uPrevVideoBuff = m_uCurrVideoBuff;
			m_uCurrVideoBuff++;
			if(NUM_VIDEO_BUFF == m_uCurrVideoBuff)
			{
				m_uCurrVideoBuff = 0;
			}
		}
		pthread_mutex_unlock(&m_rtspdio_mutx);

		if(m_fpOut && iDataLen > 0)
		{
			bool bWrite = SafeWrite(m_fpOut, (char*)(pData), iDataLen);

			if(!bWrite)
			{
				LogNormal("Write dspVideo file error!");
			}
			/*
			int32_t iWrite = fwrite(pData, 1, iDataLen, m_fpOut);
			if(iWrite != iDataLen)
			{
				printf("11 write file failed<to write<%d bytes> writed<%d bytes>\n", iDataLen, iWrite);
				int iWrite2 = fwrite(pData+iWrite, 1, iDataLen - iWrite, m_fpOut);

				if(iWrite2 != iDataLen - iWrite)
				{
					LogNormal("22 write file failed<to write<%d bytes> writed<%d bytes>\n", iDataLen - iWrite, iWrite2);
				}
			}
			*/
		}

	}//End if(iDataLen > 0)
	//printf("-111-RecvRtspDioData-\n");
}

//采集录像数据
void CDspDealVideo::CaptureVideo()
{
	/*
	m_file = NULL;
	const char *pFile = "./DIOStrm.bcm";
	m_file = fopen(pFile, "wb");
	if(NULL == m_file)
	{
		printf("open write file<%s> failed\n", pFile);
	}
	*/

#ifdef DIO_RTSP_RECV
	RecvRTSP();
#endif
}

//采集数据存录像
void CDspDealVideo::CaptureVideoFrame(const int64_t &tick, const DSP_BUFFNODE * pDspBufferNode)
{
	struct timeval tv;
	int nFrameSize = pDspBufferNode->sizeBuff;
	const char * pBuffer = pDspBufferNode->pBuff;
	int iWrite = 0;

	if(nFrameSize > 0)
	{
		if(0 == m_uBeginTime)
		{
			m_uBeginTime = tick; //单位 us
			m_uEndTime = m_uBeginTime + g_VideoFormatInfo.nTimeLength * 60 * 1000 * 1000;

			if(g_nServerType == 13 && g_nFtpServer == 1)
			{
				RECORD_PLATE plate;
				plate.uTime = (m_uBeginTime/1000)/1000;
				plate.uMiTime = (m_uBeginTime/1000)%1000;
				plate.uRoadWayID = 1;
				g_MyCenterServer.GetPlatePicPath(plate,m_strH264FileName,2);
			}
			else
			{
				m_strH264FileName = g_FileManage.GetDspVideoPath(m_nChannelId);
			}
			g_skpDB.SaveVideo(m_nChannelId,(m_uBeginTime/1000)/1000,(m_uEndTime/1000)/1000,m_strH264FileName,0);

			m_fpOut = fopen(m_strH264FileName.c_str(), "ab+");
			if(NULL == m_fpOut)
			{
				LogNormal("Open file err! path=%s. \n", m_strH264FileName.c_str());
			}
		}		

		if(m_uBeginTime != 0)
		{
			//存信息
			if(m_fpOut != NULL)
			{
				bool bWrite = SafeWrite(m_fpOut, pBuffer, nFrameSize);

				if(!bWrite)
				{
					LogNormal("Write dspVideo file error!");
				}
				//printf("--H264--nFrameSize=%d--\n", nFrameSize);
				/*
				iWrite = fwrite(pBuffer, 1, nFrameSize, m_fpOut);

				if(iWrite != nFrameSize)
				{
					printf("11 write file failed<to write<%d bytes> writed<%d bytes>\n", nFrameSize, iWrite);
					int iWrite2 = fwrite(pBuffer+iWrite, 1, nFrameSize - iWrite, m_fpOut);

					if(iWrite2 != nFrameSize - iWrite)
					{
						LogNormal("22 write file failed<to write<%d bytes> writed<%d bytes>\n", nFrameSize - iWrite, iWrite2);
					}
				}
				*/
			}  

			if(tick >= m_uEndTime)
			{
				if(m_fpOut)
				{
					fclose(m_fpOut);
					m_fpOut = NULL;
				}				

				//通知事件录象完成           
				g_skpDB.VideoSaveUpdate(m_strH264FileName,m_nChannelId,1);
				m_uBeginTime = 0;
			}
		}//End of if(m_uBeginTime != 0)		
	}//End of if(nFrameSize > 0)
}
