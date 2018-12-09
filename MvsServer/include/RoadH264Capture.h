// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef ROAD_H264_CAPTURE_H
#define ROAD_H264_CAPTURE_H

#include "global.h"

#define H264_ENCODE
#ifdef H264_ENCODE
	#include "RoadEncode.h"
#endif

#include "CvxText.h"

#ifndef DIO_RTSP
#define RTSP_ENCODE
#endif
#ifdef RTSP_ENCODE
#include "H264Streamer.hh"
#endif

#define MAX_VIDEO_SIZE 60

//h264转换成Avi录像
//#include "Avc2Avi.h"
#include "AviFile.h"


//#define MAX_VIDEO_SIZE 2
/******************************************************************************/
//	描述:H264全天录象类
/******************************************************************************/

class CRoadH264Capture
{
    public:
        //构造
        CRoadH264Capture();
        //析构
        ~CRoadH264Capture();

    public:
        void Init(int nChannelId,int nWidth,int nHeight);

        void UnInit();

        void RunRecorder();

        //添加一帧
        void AddFrame(unsigned char* pBuffer,SRIP_DETECT_HEADER sDetectHeader);
        //添加一帧-直接传入H264数据包
        void AddFrame2(unsigned char* pBuffer, Image_header_dsp &header);

        //获取一帧数据
        int PopFrame(unsigned char** pBuffer);
        //获取一帧数据
        //int PopFrameH264(unsigned char** pBuffer);

        void DecreaseSize();

        //ResizeImage
        void ResizeBigImagetoSmall(BYTE* pSrc,BYTE* pDest);

        //设置录像类型
        void SetCaptureType(CAPTURE_TYPE eType) { m_eCapType = eType;}

        //设置相机类型
        void SetCameraType(int nCameraType) { m_nCameraType = nCameraType; }

		//获取正在录像的文件名称
		std::string GetEncodeFileName(){return m_strH264FileName;}

		//更新数据库录像记录
		void VideoSaveUpdate();

		//创建数据库录像记录更新线程
		bool CreateVideoUpdateThread();
    protected:
        void CaptureVideo();
        void CaptureVideo2();
		void CaptureVideo3();
		void CaptureVideo4();

        //叠加文字信息
        void PutTextOnVideo(unsigned char* pBuffer);

    private:

        //信号互斥
        pthread_mutex_t m_video_Mutex;

        #ifdef H264_ENCODE
        RoadEncode m_H264Encode;
        #endif
		
		#ifdef RTSP_ENCODE
        LiveRTSPH264 m_LiveRTSPH264;
		#endif

        //线程ID
        pthread_t m_nThreadId;

        //通道编号
        int m_nChannelId;
		//相机编号
		int m_nCameraID;
        //结束录象
        bool m_bEndCapture;
        //开始时间
        int64_t m_uBeginTime;
        //结束时间
        int64_t m_uEndTime;
        //编码结果
        unsigned char* m_pEncodeData;
        //编码大小
        int m_nEncodeSize;

         //缓冲区队列
        unsigned char *m_FrameList[MAX_VIDEO_SIZE];
        //帧接收缓冲区
        unsigned char *m_pFrameBuffer;
        int m_nCurIndex;   //可存的内存块序号
        int m_nFirstIndex;//可取的内存块序号
        int m_nFrameSize ;//已经存储的内存块个数

        int m_uWidth;
        int m_uHeight;

        //录像类型
        CAPTURE_TYPE m_eCapType;

        //
        IplImage* m_pImageWord;
        //
        //图像文本信息
        CvxText m_cvText;
        //地点
        string m_strPlace;
        //方向
        string m_strDirection;

        //相机类型
        int m_nCameraType;

		//CAvc2Avi m_avc2avi;
		//录像最大缓冲数目
		int m_nMaxVideoSize;

		//当前录像文件名称
		std::string m_strH264FileName;

		FILE *m_fpOut;

		CAviFile m_AviFile;
};

#endif

