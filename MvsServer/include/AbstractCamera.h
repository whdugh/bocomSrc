// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _ABSTRACTCAMERA_H_
#define _ABSTRACTCAMERA_H_

#include "global.h"
#include "RoadH264Capture.h"
#include "RoadRecorder.h"

#include "RoadTempCapture.h"

#define MAX_LEN	204800
#define MAX_SIZE 20

class AbstractCamera
{
	public:
	    //构造
		AbstractCamera();
		//析构
		virtual ~AbstractCamera();

        //打开视频
		virtual bool Open();
		virtual bool Close();
		//打开文件
		virtual bool OpenFile(const char* strFileName);
		//设置远程录像文件信息
		virtual void SetVodInfo(VOD_FILE_INFO& vod_info);
		//控制
		virtual int ManualControl(CAMERA_CONFIG cfg);
		//自动控制
		virtual int  AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn);
		//读取
		virtual int ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert = true);
		//reconnet video
		virtual bool ReOpen();
		//切换工作方式
		virtual bool ChangeMode();
		//获取视频流
		virtual void PullStream();
		//when reconnet video we should close first
        //获取相机默认模板
        virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);
        virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

		//设置设备编号
        virtual void SetDeviceID(int nDeviceID);

         /////////////////////////////////////////////////////////////////////////////////////////////////////////
        //读YUV视频流缓冲队列
         int PopFrame(unsigned char** response,int nBlock=1);
         //修改缓冲队列长度
         void DecreaseSize(int nBlock = 1);

         //读JPG视频流缓冲队列
         int PopJpgFrame(unsigned char** response, int nBlock=1);
         //修改JPG缓冲队列长度
         void DecreaseJpgSize(int nBlock=1);

         //弹出车牌列表
         int PopPlateList(DSP_PLATE_LIST &dspPlateList, int nBlock=1);

         //捕获一场JPG图象
        bool GetOneJpgFrame(unsigned char* pBuffer, int nBlock = 1);
         //捕获一场图象
        bool GetOneFrame(unsigned char* pBuffer, int nBlock = 1);

        //刷新JPG队列内容
        bool RefreshJpgFrame(int nBlock = 1);

        //获取yuv图像宽高
        int GetImageWidth();
        int GetImageHeight();
        //获取需要剪切的黑边大小
        int GetMarginX();
        int GetMarginY();
        //获取有效宽高
        int GetEffectImageWidth();
        int GetEffectImageHeight();
        //获取小图宽高
        int GetSmallImageWidth();
        int GetSmallImageHeight();

        //获取jpg与yuv图像之间的缩放比例
        double GetRatioX();
        double GetRatioY();

        //设置是否闯红灯检测
        void SetDetectRedSignal(bool bDetectRedSignal) {m_bDetectRedSignal = bDetectRedSignal;}
        //设置是否线圈检测
        void SetDetectLoopSignal(bool bDetectLoopSignal) {m_bDetectLoopSignal=bDetectLoopSignal;}

        //设置是否雷达检测
        void SetDetectRadarSignal(bool bDetectRadarSignal) {m_bDetectRadarSignal=bDetectRadarSignal;}

        //获取相机参数设置
        CAMERA_CONFIG GetCameraConfig();

        //设置采集H264码流类指针
        bool SetH264Capture(CRoadH264Capture &H264Capture);
        //设置是否采集采集H264码流
        void SetHaveH264Capture(bool bOpen) { m_bH264Capture = bOpen; }

		//设置相机号
		void SetCamID(int nCamID){ m_nCamID = nCamID; }

		//获取缓冲区大小
		int GetFrameSize();

		//设置采集违章事件H264码流类指针
		bool SetSkpRecorder(CSkpRoadRecorder &SkpRecorder);

		//设置是否采集采集违章事件H264码流
		void SetHaveSkpRecorder(bool bOpen) { m_bSkpRecorder = bOpen; }

		int GetDspReboot() { return m_bCamReboot;}

		int GetVideoType() { return m_nVideoType; }

		void SetVideoType(int nVideoType) { m_nVideoType = nVideoType; }
		//添加录像命令
		virtual int AddRecordEvent(int nChannel,std::string result);

		//设置缓存采集类指针
		bool SetTempCapture(CRoadTempCapture &TempCapture);

		//设置检测类型
		void SetDetectKind(const CHANNEL_DETECT_KIND dType)
		{
			m_nDetectKind = dType;
		}
		CHANNEL_DETECT_KIND GetDetectKind()
		{
			return m_nDetectKind;
		}

    public:
        //设置相机IP以及端口
        void SetCameraIpHost(std::string strCameraHost,int nCameraPort = 0)
        {
            m_strCameraIP = strCameraHost;
			m_nCameraPort = nCameraPort;
        }
        //获取相机IP
        std::string GetCameraIP()
        {
            return m_strCameraIP;
        }

		int GetCameraMode()
		{
			return m_mode;
		}
		int SetCameraMode(int nMode)
		{
			m_mode = nMode;
		}

		//返回通道ID
        int GetChannelID() { return m_nChannelId;}
        //设置通道ID
        void SetChannelID(const int nID) { m_nChannelId = nID;}

		int GetRecvNothingFlag() { return m_nRecvNothingFlag;}
		void DisConnectCamera() { m_bCameraState = false; }
		void ConnectCamera() { m_bCameraState = true; }
		bool GetCameraState() { return m_bCameraState; }

    protected:
         //写YUV视频流缓冲队列
        bool AddFrame(int nBlock = 1);
        //写JPG缓冲队列
        bool AddJpgFrame(int nBlock = 1);

        //增加一条车牌记录进列表
        bool AddPlateFrame(RECORD_PLATE_DSP &plate, int nBlock = 1);

        //判断文件格式是否有效
        bool IsValidFormat();

        //读取一场yuv文件数据
        int GetNextFileframe();

    protected:
        //相机类型
        int m_nCameraType;
		//相机商标1:锐视 2:大华 3:博康 4:英泰智 5:港宇 6:低噪度（英泰智）
		int m_nCameraBrand;
        //视频格式
        int m_nPixelFormat;
        //缓冲区大小
        int m_nMaxBufferSize;

        /******yuv信息*********/
        //裁切宽度
        int m_nMarginX;
        //裁切高度
        int m_nMarginY;

	      //yuv文件指针
        FILE* m_yuvFp;
        //是否读yuv文件
        bool m_bReadFile;
        //首场的时间戳
        int64_t m_uTs;
        //当前场号
        unsigned int m_uFieldSeq;
        //是否首帧(文件流)
        bool m_bFirstFrame;

        //block1缓冲区队列
        unsigned char *m_FrameList[MAX_SIZE];
        //block1帧接收缓冲区
        unsigned char *m_pFrameBuffer;

        //block2缓冲区队列
        unsigned char *m_FrameList2[MAX_SIZE];
        //block2帧接收缓冲区
        unsigned char *m_pFrameBuffer2;

        //图像宽度
        int m_nWidth;
        //图像高度
        int m_nHeight;
        unsigned int m_uSeq;  //帧号

        unsigned int m_nPreFieldSeq;

        int m_nCurIndex;   //可存的内存块序号
        int m_nFirstIndex;//可取的内存块序号
        int m_nFrameSize ;//已经存储的内存块个数

        int m_nCurIndex2;   //可存的内存块序号
        int m_nFirstIndex2;//可取的内存块序号
        int m_nFrameSize2;//已经存储的内存块个数

        DSP_PLATE_LIST m_dspPlatelist; //车牌列表

        //jpg与yuv图像之间的缩放比
        double m_ratioX;
        double m_ratioY;

        //是否闯红灯检测
        bool m_bDetectRedSignal;
        //是否线圈检测
        bool m_bDetectLoopSignal;
        //是否雷达检测
        bool m_bDetectRadarSignal;

          //线程ID
	    pthread_t m_nThreadId;

	    //存取互斥
	    pthread_mutex_t m_FrameMutex;
	    pthread_mutex_t m_FrameMutex2;
	    //pthread_mutex_t m_FrameMutex3;
	    pthread_mutex_t m_PlateFrameMutex;

         //控制结束线程
	    bool m_bEndCapture;

	    //是否控制过相机
	    bool m_bCameraControl;

	    //相机控制计数器(连续5次增益小于最大则去关灯)
	    unsigned int m_nCountGain;

	    //相机控制参数
	    CAMERA_CONFIG m_cfg;

        //h264录象类
        CRoadH264Capture *m_pH264Capture;
        //是否接收H264码流
        bool m_bH264Capture;
		
		//是否接收违章事件H264码流
		bool m_bSkpRecorder;

		//录像缓存类
		CRoadTempCapture *m_pTempCapture;
		//h264违章事件录象类
		CSkpRoadRecorder *m_pSkpRecorder;

public:
        //ip
        std::string m_strCameraIP;
        //NetGateWay
        std::string m_strNetGateWay;
        //NetMask
        std::string m_strNetMask;
		//相机端口
		int m_nCameraPort;
		//相机ID
		int m_nCamID;

		int m_mode;

		//实时还是历史视频
		int m_nVideoType;//0:实时视频，1：远程历史视频，2：本地历史视频

		//组播地址
		std::string m_strCameraMultiIP;

		//是否使用测试数据
		bool m_bUseTestResult;

		int m_nChannelId; //通道编号

		VOD_FILE_INFO m_vod_info;
		//相机编号
		int m_nDeviceId;

		int m_nRecvNothingFlag;//未收到数据计数
		int m_bCamReboot;//相机是否刚重启 1:刚重启的 0:已运行一段时间
		bool m_bCameraState; //fals:连接断开 true:连接正常

		CHANNEL_DETECT_KIND m_nDetectKind;//检测类型
};

#endif



