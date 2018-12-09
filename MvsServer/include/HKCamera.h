// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _HKCAMERA_H_
#define _HKCAMERA_H_

//#define HKCAMERA

#ifdef HKCAMERA

#include"AbstractCamera.h"
#ifdef H264_DECODE
#include "RoadDecode.h"
#endif
#include "StrmCore.h"
//#include "H264RtspRevDecode.h"

typedef struct _HK_VIDEO_HEADER
{
	unsigned short uVersion;
	unsigned short uSeq;
	unsigned int uTimeStamp;
	unsigned char chReserved[44];

}HK_VIDEO_HEADER;

class CHKCamera:public AbstractCamera
{
public:
	CHKCamera(int nCameraType);
	~CHKCamera();

	void BeginThread();

	virtual bool Open();
	virtual bool Close();
	//打开文件
	virtual bool OpenFile(const char* strFileName);

	//手动控制
	virtual int ManualControl(CAMERA_CONFIG cfg);

	//自动控制
	virtual int  AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn);

	//读取
	virtual int ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert = true);
	virtual bool ReOpen();
	//获取视频流
	virtual void PullStream();

	//设置设备编号
	virtual void SetDeviceID(int nDeviceID);

	//h264图像采集
	void CaptureFrame();
	//h264解码
	void DecodeFrame();
	//rtcp
	void DealRtcp();

	//直接通过rtsp获取视频
	void RecvRTSP();

private:
	//初始化
	void Init();

	//相机控制
	bool Control(CAMERA_CONFIG cfg);

	//相机参数设置
	void SetCaMeraPara();
	
	//建立UDP连接
	bool connect_udp(int nPort);
	//关闭udp连接
	void close_udp();

	//建立UDP连接
	bool connect_rtcp(int nPort);
	//关闭udp连接
	void close_rtcp();

	//获取随机端口
	int GetRandPort();

	//组播方式连接相机
	bool Connect_Multicast();
	
	
	//获取Receive report and source Description
	string GetRRSD(int nType = 0);

	//获取SSRC
	unsigned int GetRandSSRC();

	void SetPlace(std::string strPlace);

private:

#ifdef H264_DECODE
	RoadDecode m_Decoder;
#endif

	//H264RtspRevDecode m_H264RtspRev;

	//相机编号
	int m_nDeviceId;
	
	struct sockaddr_in m_s_addr;
	int m_udp_fd;

	//h264帧列表
	list<string> m_listImage;

	//存取互斥
	pthread_mutex_t m_ImageMutex;
	 //线程ID
	 pthread_t m_nImageThreadId;
	
	 struct sockaddr_in m_rtcp_addr;
	 int m_rtcp_fd;
	  //线程ID
	 pthread_t m_nRtcpThreadId;

	 unsigned int m_uRandSSrc;

	 unsigned int m_uSendSSrc;

	 unsigned short m_uSendSeq;

	unsigned int m_uLatSRT;

	bool m_bRtspMode;
	//是否组播方式
	bool m_bMulticast;

	CStrmParser m_StreamParser; //码流分帧库

	string m_strPlace;
};
#endif
#endif
