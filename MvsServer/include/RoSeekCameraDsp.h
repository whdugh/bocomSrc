// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _ROSEEKCAMERADSP_H_
#define _ROSEEKCAMERADSP_H_

#include "AbstractCamera.h"

#include "CMDToCamera.h"
#include "GECamCfgInfo.h"
#include "RoadH264Capture.h"

#include "CSocketBase.h"
#include "VehicleConfig.h"

#ifndef ALGORITHM_YUV
#include "mvdspapi.h"
#else
#include "MvDspAPI.h"
#endif

#include "CRtspClient.h"

#define ROSEEK_DSP_TCP_PORT 25000
#define ROSEEK_DSP_TCP_CONTROL_PORT 35000
#define ROSEEK_DSP_TCP_H264_PORT 61000
#define MVS_SERVER_TCP_PORT 65000 //检测器作为服务器时的侦听端口号

#define NUM_BUFF	5					//buffer number for motion jpeg
#define MAX_PLATE_BUF 512
#define MAX_BUFF_TAG_SIZE 3000000

//struct of buffer node for loading MJPEG
typedef struct tag_BuffNode {
	int					sizeBuff;
	char				pBuff[MAX_BUFF_TAG_SIZE];
	bool				bIsLocked;

	tag_BuffNode()
	{
	    sizeBuff = 0;
	    memset(pBuff, 0, MAX_BUFF_TAG_SIZE);
	    bIsLocked = false;
	}
} BUFFNODE;

#define NUM_H264_BUFF	3

enum DSP_STRAM_TYPE
{
	DSP_HIGH_1600_1200 = 3, //1600 * 1200
	DSP_LOW_CIF = 10 //CIF格式
};

#define MAX_RTSP_HEAD 5

class CRoSeekCameraDsp:public AbstractCamera
{
    public:
        CRoSeekCameraDsp(int nCameraType);
        ~CRoSeekCameraDsp();

        virtual bool Open();
		virtual bool Close();

		//手动控制
		virtual int ManualControl(CAMERA_CONFIG cfg);
		//相机控制
        int Control(CAMERA_CONFIG  &cfg);

		virtual bool ReOpen();   
		//获取相机商标
		void GetCameraBrand();
        void CaptureFrame();
        void CaptureFrame2();
        //命令转换，数字->字符串
        bool ConvertCMDToString(const unsigned int dwCmd, char * pParam, const unsigned int &dwParamLen);
        
        bool DealOneFrame();//组织处理一帧接收的数据
        bool Close2(); //关闭H264码流采集
        bool Close3(); //关闭dsp校时

        bool SynClock(); //校时处理
		//每隔3分钟发一次心跳给相机
		bool DealCmdLink(int &nLinkFailCount);

        //车牌相关信息校验
        bool CheckPlateInfo(RECORD_PLATE_DSP &plate);
		//相机控制
		int ThreadControl(CAMERA_CONFIG  &cfg);
		bool DspCameraCtrl();
		bool CloseCamCtrl();		
		void RecvRTSP();
		//获取相机录像分辨率
		void GetCamVideoInfo(unsigned short &uWidth, unsigned short &uHeight);
		//获取相机帧率
		int GetDspCameraFrequency(const int &nCamType);
		//关闭fd
		void CloseFd(int &fd);
		//关闭线程
		void CloseJoinThread(pthread_t &nThreadId);
		//强行关闭线程
		void KillThread(pthread_t &nThreadId);

		//监测接收图片数据线程状态
		bool CheckCaptureStatus();
		//让相机时间与检测器时间同步,SetClock
		bool SetClock();
		//自动抓拍图片
		void AutoCapturePic();
		//自动产生一条记录
		void GenerateRecord(Image_header_dsp *pDsp_header);

    public:
        CCMDToCamera			m_CMDToCamera;
        CGECamCfgInfo			m_GECamCfgInfo;
        
	    pthread_t m_nThreadId2;//线程ID2	    
	    pthread_t m_nThreadId3;//线程ID3	    	
		pthread_t m_nThreadId5;//线程ID5		

    private:

        //初始化
        void Init();
        //接收
		void RecvData();
        //接收数据包
		bool SafeRecv(char*pBuf, const int nLen, int &nRecvSize); 
		//接收数据出错处理
		bool DealSafeRecvError(const int nErrCode);

		bool connect_tcp();//连接
		//等待接收缓冲有数据可读
		int WaitForSockCanRead(int sock, int nTimeOut /* = 1000 */);
		//处理相机日志
		bool DealDspLog(const char *pBuffer, const int nSize);
		//核查相机是否重启
		void CheckCameraReboot();

		//创建接收图片线程
		bool CreateCaptureThread();
		//创建接收h264录像线程
		bool CreateH264CaptureThread();
		//创建定时校时线程
		bool CreateSynClockThread();

		//设置数据包头
		void SetCameraHeader(const Image_header_dsp *pDsp_header, yuv_video_buf &camera_header);
		
	#ifdef LED_STATE
		//核查爆闪灯状态
		void CheckLedState(const char chLedState);
	#endif
    private:
		//数据
		int m_nTcpFd;
        //yuv port
        int m_nPort;

        BUFFNODE		listJpgBuff[NUM_BUFF];		//装载JPG图像数据的环形缓冲数组
        unsigned int	m_uPrevBuff;				//环形缓冲的前一次使用索引
        unsigned int	m_uCurrBuff;				//环形缓冲的当前使用索引
		pthread_mutex_t listJpgBuffMutex;//装载JPG图像数据缓冲区锁

        bool m_bOpenCamera; //打开相机标志        
        int m_nOriWidth;//原图像宽度        
        int m_nOriHeight;//原图像高度			

		int m_nCfgIndex;
		int m_nResetCount; //重启计数
		int m_nReadCount; //接收数据计数
		int m_nNetStatusFlag; //相机25000端口网络状况
		int m_nReOpenCount; //重连计数		
		int m_nRecvLastTime;//记录最后一条数据包的时间戳	
		int m_nRecvRtspFlag;//接收rtsp码流计数标记

		bool m_bSendCapturePic;//是否发送抓拍图片命令
		bool m_bAutoCaptureTime;//是否到达自动抓拍时间
	#ifdef LED_STATE	
		int m_nWarmLastTime;//上一次报警时间戳
		bool m_bPreLedArray[8];//上一次Led状态标记
	#endif
};


#endif
