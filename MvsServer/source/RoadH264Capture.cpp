//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "RoadH264Capture.h"
#include "Common.h"
#include "CommonHeader.h"
#include "RoadImcData.h"
#include "ippi.h"
#include "ippcc.h"
#include "CenterServerOneDotEight.h"



//#define LiveRTSPH264_LOG

#ifdef LiveRTSPH264_LOG
        FILE *g_pLiveRTSPH264;
#endif

//采集线程
void* ThreadH264Capture(void* pArg)
{
    CRoadH264Capture* pRoadH264Capture = (CRoadH264Capture*)pArg;

    if(pRoadH264Capture)
    {
        pRoadH264Capture->RunRecorder();
    }
    pthread_exit((void *)0);
	return pArg;
}

//录像更新线程
void* ThreadVideoSaveUpdate(void* pArg)
{
	CRoadH264Capture* pRoadH264Capture = (CRoadH264Capture*)pArg;

	if(pRoadH264Capture)
	{
		pRoadH264Capture->VideoSaveUpdate();
	}
	pthread_exit((void *)0);
	return pArg;
}

CRoadH264Capture::CRoadH264Capture()
{
    m_nThreadId = 0;
    m_bEndCapture = false;
    m_uBeginTime = 0;
    m_uEndTime = 0;
    m_nEncodeSize = 0;
    m_pEncodeData = NULL;
    m_pFrameBuffer = NULL;

	m_nMaxVideoSize = 2;

    for(int i=0;i<MAX_VIDEO_SIZE;i++)
    {
        m_FrameList[i] = NULL;
    }
    m_nFrameSize = 0;
    m_nFirstIndex = 0;
    m_nCurIndex = 0;
    m_eCapType = CAPTURE_NO;
    m_pImageWord = NULL;

    m_nCameraType = 0;
	m_nCameraID = 0;

    pthread_mutex_init(&m_video_Mutex,NULL);

	m_strH264FileName = "";
	m_fpOut = NULL;
}

CRoadH264Capture::~CRoadH264Capture()
{
    pthread_mutex_destroy(&m_video_Mutex);

	if(m_fpOut != NULL)
	{
		fclose(m_fpOut);
		m_fpOut = NULL;
	}
}

void CRoadH264Capture::Init(int nChannelId,int nWidth,int nHeight)
{
	//LogNormal("==CRoadH264Capture::Init===\n");
    m_nChannelId = nChannelId;
    m_bEndCapture = false;
    m_uBeginTime = 0;
    m_uEndTime = 0;
    m_uWidth = nWidth;
    m_uHeight = nHeight;

    m_nEncodeSize = g_nVideoWidth*g_nVideoHeight*3;
	//LogNormal("--CRoadH264Capture::Init-w:%d,h:%d \n", g_nVideoWidth, g_nVideoHeight);

    m_pEncodeData = new unsigned char[m_nEncodeSize];
		
	if(g_nDetectMode == 2)
	{
		LogNormal("--CRoadH264Capture::Init-nWidth:%d,nHeight:%d \n", nWidth, nHeight);
		m_nEncodeSize = nWidth * nHeight * 4;
		m_nMaxVideoSize = MAX_VIDEO_SIZE;
	}

    for(int i=0;i<m_nMaxVideoSize;i++)
    {
        m_FrameList[i] = (unsigned char*)calloc(sizeof(SRIP_DETECT_HEADER)+m_nEncodeSize,sizeof(unsigned char));
    }
    m_nCurIndex = 0;   //可存的内存块序号
    m_nFirstIndex = 0;//可取的内存块序号
    m_nFrameSize = 0;//已经存储的内存块个数
    m_pFrameBuffer = m_FrameList[0];

    m_pImageWord = cvCreateImageHeader(cvSize(g_nVideoWidth,g_nVideoHeight),8,3);

    if(g_nSendRTSP == 1)
    {
        //system("ip ro del 224.0.0.0/4 dev eth0 scope global");
#ifdef RTSP_ENCODE
        m_LiveRTSPH264.RtspH264Init();
#endif
    }

#ifdef H264_ENCODE
	m_H264Encode.SetVideoSize(g_nVideoWidth,g_nVideoHeight,1);
	m_H264Encode.SetFrameRate(g_fFrameRate);

	if(m_eCapType != CAPTURE_FULL)
	{
		m_H264Encode.InitEncode(NULL);
	}
#endif

    #ifdef LiveRTSPH264_LOG

    g_pLiveRTSPH264 = fopen("LiveRTSPH264.log","wb");

    #endif
    m_cvText.Init(25);

    m_strPlace = g_skpDB.GetPlace(m_nChannelId);
	m_nCameraID = g_skpDB.GetCameraID(m_nChannelId);

    int nDirection = g_skpDB.GetDirection(m_nChannelId);
    m_strDirection = GetDirection(nDirection);

    //相机类型
    LogNormal("==CRoadH264Capture==m_nCameraType=%d==\n", m_nCameraType);

    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    //分离线程
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    //启动事件录像线程
    pthread_create(&m_nThreadId,&attr,ThreadH264Capture,this);

	LogNormal("==CRoadH264Capture=start=ThreadH264Capture=m_nThreadId=%x==\n", m_nThreadId);

    pthread_attr_destroy(&attr);	
}

void CRoadH264Capture::UnInit()
{
    m_bEndCapture = true;

    //停止录像线程
	if(m_nThreadId != 0)
	{
	    pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
	}
	#ifdef RTSP_ENCODE
    if(g_nSendRTSP == 1)
    m_LiveRTSPH264.RtspH264UnInit();
#endif

    #ifdef H264_ENCODE
    m_H264Encode.UinitEncode();
    #endif

    if(m_pEncodeData!=NULL)
    {
        delete []m_pEncodeData;
        m_pEncodeData = NULL;
    }

    if(m_pImageWord != NULL)
    {
        cvReleaseImageHeader(&m_pImageWord);
        m_pImageWord = NULL;
    }
    //加锁缓冲区
	pthread_mutex_lock(&m_video_Mutex);
    //释放内存
	if(m_nEncodeSize != 0)
	{
        for(int i=0;i<m_nMaxVideoSize;i++)
        {
            if(m_FrameList[i]!=NULL)
            {
                free(m_FrameList[i]);
                m_FrameList[i] = NULL;
            }
        }
        m_nFrameSize = 0;
        m_pFrameBuffer = NULL;
	}
	//解锁
	pthread_mutex_unlock(&m_video_Mutex);

    #ifdef LiveRTSPH264_LOG

    if(g_pLiveRTSPH264)
    {
        fclose(g_pLiveRTSPH264);
        g_pLiveRTSPH264 = NULL;
    }
    #endif
    m_cvText.UnInit();

	if(m_fpOut != NULL)
	{
		fclose(m_fpOut);
		m_fpOut = NULL;
	}
}

//ResizeImage
void CRoadH264Capture::ResizeBigImagetoSmall(BYTE* pSrc,BYTE* pDest)
{
	if((m_uWidth == g_nVideoWidth) && (m_uHeight == g_nVideoHeight))
	{
		memcpy(pDest,pSrc,m_uWidth*m_uHeight*3);
	}
	else
	{
    //RGB视频流resize
    IppiSize roi;
    roi.width  = m_uWidth;
    roi.height = m_uHeight;

    IppiSize   dstRoi;
    dstRoi.width = g_nVideoWidth;
    dstRoi.height = g_nVideoHeight;

    int lineStep   = roi.width*3 + IJL_DIB_PAD_BYTES(roi.width,3);
    int dstImgStep = dstRoi.width*3 + IJL_DIB_PAD_BYTES(dstRoi.width,3);
    double   ratioX = (double)dstRoi.width / (double)roi.width ;
    double   ratioY = (double)dstRoi.height / (double)roi.height ;
    IppiRect srcroi= {0,0,roi.width,roi.height};

    //double t = (double)cvGetTickCount();
    
	#ifndef ALGORITHM_DL
	ippiResize_8u_C3R(pSrc, roi, lineStep, srcroi, pDest, dstImgStep, dstRoi,ratioX, ratioY,IPPI_INTER_NN);
	#else
	//IppiPoint dstOffset={0,0};
	//ippiResizeLinear_8u_C3R(pSrc,lineStep,pDest, dstImgStep,dstOffset,dstRoi,ippBorderInMem,NULL,NULL, NULL);

	IplImage* pOrgImg = cvCreateImageHeader(cvSize(roi.width, roi.height), 8, 3);
	IplImage* pDestImg = cvCreateImageHeader(cvSize(dstRoi.width, dstRoi.height), 8, 3);
	cvSetData(pOrgImg,pSrc,pOrgImg->widthStep);
	cvSetData(pDestImg,pDest,pDestImg->widthStep);
	cvResize(pOrgImg,pDestImg);
	cvReleaseImageHeader(&pOrgImg);
	cvReleaseImageHeader(&pDestImg);

	#endif
    /*t = (double)cvGetTickCount() - t;
     double dt = t/((double)cvGetTickFrequency()*1000.) ;
     printf( "============CRoadH264Capture::ResizeBigImagetoSmall dt=%d ms\n",(int)dt);*/
	}
}

//添加一帧
void CRoadH264Capture::AddFrame(unsigned char* pBuffer,SRIP_DETECT_HEADER sDetectHeader)
{
    if(m_pFrameBuffer)
    {
        memcpy(m_pFrameBuffer,&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
        ResizeBigImagetoSmall(pBuffer,m_pFrameBuffer+sizeof(SRIP_DETECT_HEADER));

		//LogTrace("Capture264-2.txt", "---==uSeq=%d==", sDetectHeader.uSeq);
    }

	//加锁缓冲区
	pthread_mutex_lock(&m_video_Mutex);

        m_nFrameSize++;
        if(m_nFrameSize>m_nMaxVideoSize-1)//实际帧数
        {
            m_nFrameSize--;
        }
        else
        {
            m_nCurIndex++;
            m_nCurIndex %= m_nMaxVideoSize;
        }
        m_pFrameBuffer = m_FrameList[m_nCurIndex];
	//解锁缓冲区
    pthread_mutex_unlock(&m_video_Mutex);
}

//添加一帧-直接传入H264数据包
void CRoadH264Capture::AddFrame2(unsigned char* pBuffer, Image_header_dsp &header)
{
	//printf("---AddFrame2------>>>\n");
	if(pBuffer == NULL)
	{
		return;
	}

    if(m_pFrameBuffer)
    {
        //LogTrace("H264-Rec.log", "==AddFrame2===header.nSize=%d=m_nCurIndex=%d=m_nFirstIndex=%d=m_nFrameSize=%d\n",\
			header.nSize, m_nCurIndex, m_nFirstIndex, m_nFrameSize);
		//printf("==AddFrame2===header.nSize=%d=m_nCurIndex=%d=m_nFirstIndex=%d=m_nFrameSize=%d\n",\
			header.nSize, m_nCurIndex, m_nFirstIndex, m_nFrameSize);
        memcpy(m_pFrameBuffer, &header, sizeof(Image_header_dsp));
        memcpy(m_pFrameBuffer+sizeof(Image_header_dsp), pBuffer, header.nSize);
    }

    //加锁缓冲区
	pthread_mutex_lock(&m_video_Mutex);

        m_nFrameSize++;
        if(m_nFrameSize>m_nMaxVideoSize-1)//实际帧数
        {
			LogError("通道[%d],录像缓冲区满,m_nFrameSize=%d,uSeq=%d",m_nChannelId, m_nFrameSize, header.nSeq);
            m_nFrameSize--;
        }
        else
        {
            m_nCurIndex++;
            m_nCurIndex %= m_nMaxVideoSize;
        }
        m_pFrameBuffer = m_FrameList[m_nCurIndex];

		
	//解锁缓冲区
    pthread_mutex_unlock(&m_video_Mutex);
}

//获取一帧数据
int CRoadH264Capture::PopFrame(unsigned char** pBuffer)
{
	int nSize = 0;
	//加锁
	pthread_mutex_lock(&m_video_Mutex);
        if(m_nFrameSize >= 1) //只有一场
        {
            //LogTrace("TT11.log", "==###==CRoadH264Capture PopFrame()===m_nFrameSize=%d==m_nFirstIndex=%d\n",\
						m_nFrameSize, m_nFirstIndex);
			nSize = 1;
            if(m_FrameList[m_nFirstIndex] != NULL)
			{
				*pBuffer = m_FrameList[m_nFirstIndex];//指向当前可读取的位置

				//Image_header_dsp* header = (Image_header_dsp*)(*pBuffer);
				//int nFrameSize = header->nSize;
				//LogTrace("TT11.log", "==CRoadH264Capture=PopFrame==nFrameSize=%d===\n", nFrameSize);
			}
        }
    //解锁
	pthread_mutex_unlock(&m_video_Mutex);

	return nSize;
}


void CRoadH264Capture::DecreaseSize()
{
     //加锁
	pthread_mutex_lock(&m_video_Mutex);

    m_nFrameSize --;
    m_nFirstIndex = (m_nFirstIndex+1)%m_nMaxVideoSize;

	//printf("---m_nFirstIndex=%d, m_nMaxVideoSize=%d\n", m_nFirstIndex, m_nMaxVideoSize);

	//LogTrace("TT11.log", "==CRoadH264Capture=DecreaseSize==m_nFrameSize=%d==m_nFirstIndex=%d=m_nCurIndex=%d\n", \
		m_nFrameSize, m_nFirstIndex, m_nCurIndex);

	//解锁
	pthread_mutex_unlock(&m_video_Mutex);

}

void CRoadH264Capture::RunRecorder()
{
    while(!m_bEndCapture)
	{		
		if(g_nDetectMode == 2)
	    {
			if(g_nEncodeFormat == 1)
			{
				CaptureVideo4(); //h264裸流不频繁操作文件版本
			}
			else
			{
				CaptureVideo3(); //h264转成avi文件
			}
			
	        usleep(10*1);//
	    }
		else
	    {
	        //printf("===CaptureVideo()===\n");
	        //处理录象
           CaptureVideo();
           //等1毫秒响应下一个录象请求
           usleep(1000*1);//
	    }
	}
}

void CRoadH264Capture::CaptureVideo()
{
        unsigned char* pBuffer = NULL;
        int nSize = PopFrame(&pBuffer);

        if(nSize>0)
        {
            //叠加文字信息
			if(g_VideoFormatInfo.nSendH264 == 0)
			{
				PutTextOnVideo(pBuffer);
			}

            SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(pBuffer);
            #ifdef H264_ENCODE
			if(m_eCapType == CAPTURE_FULL)
            {
				if(m_uBeginTime == 0)
				{
					//LogNormal("tt111--uSeq=%d--\n", sDetectHeader->uSeq);
					//LogTrace("Capture264.txt", "-tt111--==uSeq=%d==", sDetectHeader->uSeq);
					m_uBeginTime = sDetectHeader->uTime64;
					m_uEndTime = m_uBeginTime+ g_VideoFormatInfo.nTimeLength * 60 * 1000 * 1000;
					//m_uEndTime = m_uBeginTime+  40 * 1000 * 1000;

					//printf("===CaptureVideo=sDetectHeader->dFrameRate/2=%d\n",sDetectHeader->dFrameRate/2);

					if(g_nServerType == 7)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2);
					}
					else if(g_nServerType == 13 && g_nFtpServer == 1)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uMiTime = (m_uBeginTime/1000)%1000;
						plate.uRoadWayID = 1;
						g_MyCenterServer.GetPlatePicPath(plate,m_strH264FileName,2);
					}
					else
					{
						m_strH264FileName = g_FileManage.GetVideoPath();
					}
#ifdef H264_ENCODE
                    m_H264Encode.InitEncode((char*)(m_strH264FileName.c_str()));
#endif
                    g_skpDB.SaveVideo(m_nChannelId,(m_uBeginTime/1000)/1000,(m_uEndTime/1000)/1000,m_strH264FileName,0);
                }
            }

            int nEncodeSize = 0;
            int nalArray[20] = {0};
            int nalCount = 0;

            #ifdef LogTime
            struct timeval tv;
            double t = (double)cvGetTickCount();
            gettimeofday(&tv,NULL);
            LogTrace("time-test.log","before H264Encode==uSeq=%lld,time = %s.%03d\n",sDetectHeader->uSeq,GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
            #endif

#ifdef H264_ENCODE
            m_H264Encode.WriteFrame((unsigned char*)(pBuffer+sizeof(SRIP_DETECT_HEADER)),m_nEncodeSize,m_pEncodeData,nEncodeSize,nalArray,&nalCount);
#endif
            /*t = (double)cvGetTickCount() - t;
             double dt = t/((double)cvGetTickFrequency()*1000.) ;
             printf( "============CRoadH264Capture::m_H264Encode.WriteFrame dt=%d ms\n",(int)dt);*/
             //printf( "============CRoadH264Capture::m_H264Encode.WriteFrame %x,%x,%x,%x,uSeq=%lld\n",m_pEncodeData[0],m_pEncodeData[1],m_pEncodeData[2],m_pEncodeData[3],sDetectHeader->uSeq);

            if(nEncodeSize > 0)
            {
                if(g_nSendRTSP == 1)
                {
					#ifdef RTSP_ENCODE
                    m_LiveRTSPH264.addFrame(m_pEncodeData,nEncodeSize,nalArray,nalCount,sDetectHeader->uTime64);
#endif
				}

				if(g_VideoFormatInfo.nSendH264 == 1)
				{
					sDetectHeader->uDetectType = SRIP_NORMAL_PIC;
					sDetectHeader->uWidth = g_nVideoWidth;
					sDetectHeader->uHeight = g_nVideoHeight;
					sDetectHeader->uVideoId = 1; //表示传输h264视频给客户端
					std::string pic;
					//图片数据头
					pic.append((char*)sDetectHeader,sizeof(SRIP_DETECT_HEADER));
					//图片数据
					pic.append((char*)m_pEncodeData,nEncodeSize);
					//添加到处理列表
					g_skpChannelCenter.AddResult(pic);

				  /*  FILE* fp = fopen("test3.avi","ab+");
					unsigned char buf[4] = {0xaa,0xbb,0xcc,0xdd};
					fwrite(buf,1,4,fp);
					fwrite(&nEncodeSize,1,4,fp);
					fwrite(m_pEncodeData,1,nEncodeSize,fp);
					fclose(fp);*/
				
					printf("===========send h264**************************g_nVideoWidth=%d,g_nVideoHeight = %d\n",g_nVideoWidth,g_nVideoHeight);

					printf("===========send h264**************************uSeq=%lld,nEncodeSize=%d,pEncodeData[0]=%x,pEncodeData[10] = %x,pEncodeData[size-1]=%x\n",sDetectHeader->uSeq,nEncodeSize,m_pEncodeData[0],m_pEncodeData[10],m_pEncodeData[nEncodeSize-1]);
                }
            }
            #ifdef LogTime
            t = (double)cvGetTickCount() - t;
            double dt = t/((double)cvGetTickFrequency()*1000.) ;
            gettimeofday(&tv,NULL);
            LogTrace("time-test.log","after H264Encode==nSeq=%lld,time = %s.%03d,dt=%d ms\n",sDetectHeader->uSeq,GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,(int)dt);
            #endif

			if(m_eCapType == CAPTURE_FULL)
			{
				if(sDetectHeader->uTime64 >= m_uEndTime)
				{
#ifdef H264_ENCODE
						m_H264Encode.UinitEncode();
#endif

						string strVideoPath = m_strH264FileName;
						/*
						//通知事件录象完成
						string strVideoPath = m_strH264FileName;
						g_skpDB.VideoSaveUpdate(strVideoPath,sDetectHeader->uChannelID,1);
						*/
						//创建录像记录更新线程
						this->CreateVideoUpdateThread();

						//LogNormal("tt222--uSeq=%d--\n", sDetectHeader->uSeq);
						//LogTrace("Capture264.txt", "-tt222--==uSeq=%d==", sDetectHeader->uSeq);

						//此时建立软链接
						if(g_nServerType == 10)
						{
							RECORD_PLATE plate;
							plate.uTime = (m_uBeginTime/1000)/1000;
							memcpy(plate.chVideoPath,strVideoPath.c_str(),strVideoPath.size());
							plate.uDirection = g_skpDB.GetDirection(m_nChannelId);

							g_TJServer.AddVideo(plate);
						} 
					m_uBeginTime = 0;
				}
			}

            #endif

            DecreaseSize();
        }
}

void CRoadH264Capture::CaptureVideo2()
{
    unsigned char* pBuffer = NULL;
    int nSize = this->PopFrame(&pBuffer);

	//LogTrace("TT9.log", "==CRoadH264Capture=CaptureVideo2==PopFrame=nSize=%d \n", nSize);

    struct timeval tv;
    int nFrameSize = 0;

    int nSkip = 0;
    UINT32 uSwitch = 0;
	int iWrite = 0;

    if(nSize>0)
    {
        Image_header_dsp* header = (Image_header_dsp*)(pBuffer);
        nFrameSize = header->nSize;
        //LogTrace("CaptureVideo-264.log", "==CRoadH264Capture=CaptureVideo2==nFrameSize=%d===\n", nFrameSize);
        uSwitch = header->nSeq;

        //LogTrace("TT2.log", "=####===uSwitch=%d=header->nFrameRate=%d, g_fFrameRate=%f, nSkip=%d==\n", \
			uSwitch, header->nFrameRate, g_fFrameRate, nSkip);

        if(nFrameSize > 0)
        {
            if(m_uBeginTime == 0)
            {
				//LogNormal("==m_nFrameSize=%d", m_nFrameSize);
                m_uBeginTime = header->ts; //单位 us
                m_uEndTime = m_uBeginTime + g_VideoFormatInfo.nTimeLength * 60 * 1000 * 1000;
				//m_uEndTime = m_uBeginTime+  40 * 1000 * 1000;
                printf("====1111=======m_uBeginTime=%lld==m_uEndTime=%lld===\n", m_uBeginTime, m_uEndTime);

                if(m_eCapType == CAPTURE_FULL)
                {
					if(g_nServerType == 13 && g_nFtpServer == 1)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uMiTime = (m_uBeginTime/1000)%1000;
						plate.uRoadWayID = 1;
						g_MyCenterServer.GetPlatePicPath(plate,m_strH264FileName,2);
					}
					else if(g_nServerType == 7)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2);
					}
					else
					{
						m_strH264FileName = g_FileManage.GetDspVideoPath(m_nChannelId);
					}
                    g_skpDB.SaveVideo(m_nChannelId,(m_uBeginTime/1000)/1000,(m_uEndTime/1000)/1000,m_strH264FileName,0);
					//LogTrace("CaptureVideo-264.log", "====m_nChannelId=%d==m_strH264FileName=%s=\n", \
						m_nChannelId, m_strH264FileName.c_str());
                }
            }
    
            if(m_eCapType == CAPTURE_FULL)
            {    
                    FILE *fpOut = NULL;

                    fpOut = fopen(m_strH264FileName.c_str(), "ab+");
                    //存信息
                    if(fpOut != NULL)
                    {
						bool bWrite = SafeWrite(fpOut, (char*)(pBuffer+sizeof(Image_header_dsp)), nFrameSize);

						if(!bWrite)
						{
							LogNormal("Write h264Video file error!");
						}
						//printf("--H264--nFrameSize=%d--\n", nFrameSize);
						/*
                        iWrite = fwrite(pBuffer+sizeof(Image_header_dsp), 1, nFrameSize, fpOut);

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

                        fclose(fpOut);
                    }  
            }//End of if(m_eCapType == CAPTURE_FULL)

            if(header->ts >= m_uEndTime)
            {
                if(m_eCapType == CAPTURE_FULL)
                {
		            //通知事件录象完成           
                    g_skpDB.VideoSaveUpdate(m_strH264FileName,m_nChannelId,1);
                }

                m_uBeginTime = 0;
            }

            //printf("====22222=======m_uBeginTime=%lld==m_uEndTime=%lld===\n", m_uBeginTime, m_uEndTime);
        }//End of if(nFrameSize > 0)

		DecreaseSize();
    }//End of if(nSize>0)
}

/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CRoadH264Capture::PutTextOnVideo(unsigned char* pBuffer)
{
    wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    int nWidth = 10;
    int nHeight = 30;

    cvSetData(m_pImageWord,pBuffer+sizeof(SRIP_DETECT_HEADER),m_pImageWord->widthStep);

    SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(pBuffer);

    //时间
    std::string strTime = GetTime(sDetectHeader->uTimestamp,0);
    sprintf(chOut,"时间:%s.%03d",strTime.c_str(),(sDetectHeader->uTime64/1000)%1000);
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    m_cvText.putText(m_pImageWord, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,0));


    //地点
    sprintf(chOut,"地点:%s",m_strPlace.c_str());
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    nHeight += 30;
    m_cvText.putText(m_pImageWord, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,255,0));

    //方向
    memset(wchOut,0,sizeof(wchOut));
    sprintf(chOut,"方向:%s",m_strDirection.c_str());
    UTF8ToUnicode(wchOut,chOut);
    nHeight += 30;
    m_cvText.putText(m_pImageWord, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,0));
}


void CRoadH264Capture::CaptureVideo3()
{
	unsigned char* pBuffer = NULL;
    int nSize = this->PopFrame(&pBuffer);

	//LogTrace("TT99.log", "==CRoadH264Capture=CaptureVideo3==PopFrame=nSize=%d \n", nSize);

    struct timeval tv;
    int nFrameSize = 0;
    //string strVideoPath = "";
    int nSkip = 0;
    UINT32 uSwitch = 0;
	bool bAddOneFrame = false;

    if(nSize>0)
    {
        Image_header_dsp* header = (Image_header_dsp*)(pBuffer);
        nFrameSize = header->nSize;

        if(nFrameSize > 0)
        {
            if(m_uBeginTime == 0)
            {
                m_uBeginTime = header->ts; //单位 us
				//2分钟，一段
                m_uEndTime = m_uBeginTime + g_VideoFormatInfo.nTimeLength * 60 * 1000 * 1000;

                if(m_eCapType == CAPTURE_FULL)
                {
                    //strVideoPath = g_FileManage.GetVideoPath();
					if(g_nServerType == 13 && g_nFtpServer == 1)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uMiTime = (m_uBeginTime/1000)%1000;
						plate.uRoadWayID = 1;
						g_MyCenterServer.GetPlatePicPath(plate,m_strH264FileName,2);
					}
					else if(g_nServerType == 7)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2);
						if(access(m_strH264FileName.c_str(),F_OK) == 0)//防止另外的通道存在相同文件名的录像
						{
							plate.uTime += 1;
							g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2);
						}
					}
					else if(g_nServerType == 23)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,7,0,m_nChannelId);
					}
					else if(g_nServerType == 29)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2,0,m_nChannelId);
					}
					else
					{
						m_strH264FileName = g_FileManage.GetDspVideoPath(m_nChannelId);
					}
					char filepath[256] = {0};
					memset(filepath, 0, 256);
					memcpy(filepath, m_strH264FileName.c_str(), m_strH264FileName.size());
					
					printf("filepath=%s\n",filepath);
					/*m_avc2avi.SetOutFilePath(filepath);					
					m_avc2avi.SetFps(g_fFrameRate);//设置帧率
					m_avc2avi.SetAviformat(header->nWidth, header->nHeight);
					m_avc2avi.Init();*/
					
					m_fpOut = fopen(filepath,"wb");
					char  fcc[5] = "H264";
					/* Init avi */
					if(m_fpOut != NULL)
					{
						m_AviFile.avi_init( &(m_AviFile.avi), m_fpOut, static_cast<float>(g_fFrameRate), fcc, header->nWidth, header->nHeight);
						m_AviFile.vbuf_init( &(m_AviFile.vb));
					}

                    g_skpDB.SaveVideo(m_nChannelId,(m_uBeginTime/1000)/1000,(m_uEndTime/1000)/1000,m_strH264FileName,0);
                }
            }


			struct timeval tv;
			gettimeofday(&tv,NULL);
			LogTrace("AddTT.log", "=t1==%06d:%d=\n", tv.tv_usec, tv.tv_sec);

			//处理一帧h264裸流
			//bAddOneFrame = m_avc2avi.ConvertAvc2Avi2((char*)(pBuffer+sizeof(Image_header_dsp)), nFrameSize);
			//printf("nFrameSize=%d,nData=%x,fp=%lld\n",nFrameSize,(pBuffer[sizeof(Image_header_dsp)+4]&0x1f),m_fpOut);
			if(m_fpOut != NULL)
			{
				//printf("avi_write one frame\n");
				bool bKeyFrame = false;
				if (((pBuffer[sizeof(Image_header_dsp)+4]&0x1f) == 0x07)|| ((pBuffer[sizeof(Image_header_dsp)+4]&0x1f) == 0x08)  || ((pBuffer[sizeof(Image_header_dsp)+4]&0x1f) == 0x05))
				{
					bKeyFrame = true;
				}
				m_AviFile.vbuf_add( &m_AviFile.vb, nFrameSize, (char*)(pBuffer+sizeof(Image_header_dsp)) );
				if( m_AviFile.vb.i_data > 0 )
				{
					m_AviFile.avi_write( &m_AviFile.avi, &m_AviFile.vb, bKeyFrame ? AVIIF_KEYFRAME : 0);
					m_AviFile.vbuf_reset(&m_AviFile.vb);
				}
			}

			gettimeofday(&tv,NULL);
			LogTrace("AddTT.log", "=t2==%06d:%d=\n", tv.tv_usec, tv.tv_sec);

			LogTrace("AddTT.log", \
				"###===frame[%d]=nFrameSize=%d=m_uBeginTime=%lld=header->ts=%lld==m_uEndTime=%lld, bAddOneFrame=%d=\n", \
				uSwitch, nFrameSize, m_uBeginTime, header->ts, m_uEndTime, bAddOneFrame);

            if(header->ts >= m_uEndTime)
            {
                if(m_eCapType == CAPTURE_FULL)
                {
					//停止一段录像
					//m_avc2avi.EndAvc2Avi();
					m_AviFile.avi.i_width  = header->nWidth;
					m_AviFile.avi.i_height = header->nHeight;
					m_AviFile.avi_end( &m_AviFile.avi );

					if(m_fpOut != NULL)
					{
						fclose(m_fpOut);
						m_fpOut = NULL;
					}

					LogTrace("AddTT.log", "###==EndAvc2Avi=frame[%d]=\n", uSwitch);

                    //通知事件录象完成
                    //strVideoPath = g_FileManage.GetEncodeFileName();
                    g_skpDB.VideoSaveUpdate(m_strH264FileName,m_nChannelId,1);
                }

                m_uBeginTime = 0;
            }
        }//End of if(nFrameSize > 0)

		DecreaseSize();
    }//End of if(nSize>0)
}


//更新数据库录像记录
void CRoadH264Capture::VideoSaveUpdate()
{
	//通知事件录象完成
	string strVideoPath = m_strH264FileName;
	g_skpDB.VideoSaveUpdate(strVideoPath,m_nChannelId,1);
}

//创建数据库录像记录更新线程
bool CRoadH264Capture::CreateVideoUpdateThread()
{
	//LogNormal("=CreateVideoUpdateThread==\n");
	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t id;
	if (pthread_create(&id, &attr, ThreadVideoSaveUpdate,this) != 0)
	{
		pthread_attr_destroy(&attr);
		return false;
	}
	pthread_attr_destroy(&attr);
	return true;
}

void CRoadH264Capture::CaptureVideo4()
{
	unsigned char* pBuffer = NULL;
	int nSize = this->PopFrame(&pBuffer);

	//LogTrace("TT9.log", "==CRoadH264Capture=CaptureVideo2==PopFrame=nSize=%d \n", nSize);

	struct timeval tv;
	int nFrameSize = 0;

	int nSkip = 0;
	UINT32 uSwitch = 0;
	int iWrite = 0;

	if(nSize>0)
	{
		Image_header_dsp* header = (Image_header_dsp*)(pBuffer);
		nFrameSize = header->nSize;
		//LogTrace("CaptureVideo-264.log", "==CRoadH264Capture=seq:%d ==nFrameSize=%d===\n", \
			header->nSeq, nFrameSize);


		uSwitch = header->nSeq;

		//LogTrace("TT2.log", "=####===uSwitch=%d=header->nFrameRate=%d, g_fFrameRate=%f, nSkip=%d==\n", \
		uSwitch, header->nFrameRate, g_fFrameRate, nSkip);

		if(nFrameSize > 0)
		{
/*
			LogTrace("CaptureVideo-264.log", "size:%d %x %x %x %x %x %x # ", \
				nFrameSize,\
				*(char*)(pBuffer + sizeof(Image_header_dsp)), \
				*(char*)(pBuffer + sizeof(Image_header_dsp) + 1), \
				*(char*)(pBuffer + sizeof(Image_header_dsp) + 2), \
				*(char*)(pBuffer + sizeof(Image_header_dsp) + 3), \
				*(char*)(pBuffer + sizeof(Image_header_dsp) + 4), \
				*(char*)(pBuffer + sizeof(Image_header_dsp) + 5));
*/
			if(m_uBeginTime == 0)
			{
				//LogNormal("==m_nFrameSize=%d", m_nFrameSize);
				m_uBeginTime = header->ts; //单位 us
				m_uEndTime = m_uBeginTime + g_VideoFormatInfo.nTimeLength * 60 * 1000 * 1000;
				//m_uEndTime = m_uBeginTime+  40 * 1000 * 1000;
				printf("====1111=======m_uBeginTime=%lld==m_uEndTime=%lld===\n", m_uBeginTime, m_uEndTime);

				if(m_eCapType == CAPTURE_FULL)
				{
					if(g_nServerType == 13 && g_nFtpServer == 1)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uMiTime = (m_uBeginTime/1000)%1000;
						plate.uRoadWayID = 1;
						g_MyCenterServer.GetPlatePicPath(plate,m_strH264FileName,2);
					}
					else if(g_nServerType == 7)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2);

						if(access(m_strH264FileName.c_str(),F_OK) == 0)//防止另外的通道存在相同文件名的录像
						{
							plate.uTime += 1;
							g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2);
						}
					}
					else if(g_nServerType == 23)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,7,0,m_nChannelId);
					}
					else if(g_nServerType == 29)
					{
						RECORD_PLATE plate;
						plate.uTime = (m_uBeginTime/1000)/1000;
						plate.uDirection = g_skpDB.GetDirection(m_nChannelId);
						g_RoadImcData.GetPlatePicPath(plate,m_strH264FileName,2,0,m_nChannelId);
					}
					else
					{
						m_strH264FileName = g_FileManage.GetDspVideoPath(m_nChannelId);
					}
					g_skpDB.SaveVideo(m_nChannelId,(m_uBeginTime/1000)/1000,(m_uEndTime/1000)/1000,m_strH264FileName,0);
					//LogTrace("CaptureVideo-264.log", "====m_nChannelId=%d==m_strH264FileName=%s=\n", \
						m_nChannelId, m_strH264FileName.c_str());

					m_fpOut = fopen(m_strH264FileName.c_str(), "ab+");
					if(NULL == m_fpOut)
					{
						LogNormal("Open file err! path=%s. \n", m_strH264FileName.c_str());
					}
				}
			}

			if(m_eCapType == CAPTURE_FULL)
			{				
				//存信息
				if(m_fpOut != NULL)
				{
					bool bWrite = SafeWrite(m_fpOut, (char*)(pBuffer+sizeof(Image_header_dsp)), nFrameSize);

					if(!bWrite)
					{
						LogNormal("Write h264Video file error!");
					}
					/*
					//printf("--H264--nFrameSize=%d--\n", nFrameSize);
					iWrite = fwrite(pBuffer+sizeof(Image_header_dsp), 1, nFrameSize, m_fpOut);

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
			}//End of if(m_eCapType == CAPTURE_FULL)

			if(header->ts >= m_uEndTime)
			{
				if(m_eCapType == CAPTURE_FULL)
				{
					if(m_fpOut != NULL)
					{
						fclose(m_fpOut);
						m_fpOut = NULL;
					}				

					//通知事件录象完成           
					g_skpDB.VideoSaveUpdate(m_strH264FileName,m_nChannelId,1);
				}

				m_uBeginTime = 0;
			}

			//printf("====22222=======m_uBeginTime=%lld==m_uEndTime=%lld===\n", m_uBeginTime, m_uEndTime);
		}//End of if(nFrameSize > 0)

		DecreaseSize();
	}//End of if(nSize>0)
}
