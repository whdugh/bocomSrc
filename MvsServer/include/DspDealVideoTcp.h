// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2013 上海博康智能信息技术有限公司
// Copyright 2008-2013 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _DSP_DEALVIDEO_TCP_H_
#define _DSP_DEALVIDEO_TCP_H_

#include "AbstractCamera.h"
#include "CSocketBase.h"
#include "mvdspapi.h"

#include "DspVideoUnit.h"

#define CIF_H264_DSP_WIDTH 320
#define CIF_H264_DSP_HEIGHT 240

#define H264_DSP_COUNT 500 //单位(10ms)--5秒
//#define MAX_DSP_NUMS 30


//相机，录像path,列表
typedef std::map<UINT32, VIDEO_PATH_INFO> MapIdPathInfo;

//相机，客户端IP,列表
typedef std::map<UINT32, std::string> MapIdIp;

//相机,视频处理单元指针，列表
typedef std::map<std::string, CDspVideoUnit*> MapDspVideoUnit;

class CDspDealVideoTcp
{
public:
	CDspDealVideoTcp();
	~CDspDealVideoTcp();	

	/**
	* 1.初始化接收码流
	*/
	bool InitTcp(bool bH264Capture, bool bSkpRecorder, 
		CRoadH264Capture * pH264Capture, CSkpRoadRecorder * pSkpRecorder);
	//开启录像服务线程
	bool StartServer();
	//建立接收h264码流sock
	bool h264Accept();	
	int& Geth264MoniSockFd(){ return m_nH264TcpFd; }	


	/**
	* 2.接收h26码流
	*/
	//TCP 接收数据
	//void RecvH264Fd(Client_Fd cfd);
	//接收h264数据包
	//bool SafeRecvh264Fd(Client_Fd& cfd,char*pBuf, int nLen, int &nRecvSize);

	/**
	* 3.关闭接收码流
	*/
	bool Close(); //关闭H264码流采集
	bool Close7();

	/**
	* 4.管理连接
	*/
	bool Addh264Client(DspClassAndFd& classcfd);
	bool Delh264Client(DspClassAndFd& classcfd);

	/**
	* 5.心跳处理
	*/
	//bool Seth264LinkFd(int& fd);		
	//void SendH264LinkFd(Client_Fd& cfd);//发送h264连接心跳包	

	void CaptureFrame2();

	//通过相机Ip获取相机id
	bool GetIdFromMap(const std::string strIp, UINT32 &uCamId);
	//更新ip,列表
	void RefreshIpMap(const UINT32 uCamId, const int nSocket, const std::string strIp);

	// 通过IP和Port查找Dsp是否存在
	CDspVideoUnit* IsExistDspClient(std::string strIp);

	//检查连接，并处理
	void DetectStatusAndDeal();
	bool CloseDetectStatus();
	//检查连接
	void DetectStatus();

private:

	int m_nH264TcpFd;
	pthread_t m_nThreadId7;//线程ID7接收公交h264码流线程
	
	pthread_t m_nThreadStatusId;

	mvCSocketBase *m_nH264SocketBase; //从侦听类中派生对象用于侦听
	
	DspVideoClient_Fd m_cfd;

	//h264录象类
	CRoadH264Capture *m_pH264Capture;
	//是否接收H264码流
	bool m_bH264Capture;

	//h264违章事件录象类
	CSkpRoadRecorder *m_pSkpRecorder;
	//是否接收违章事件H264码流
	bool m_bSkpRecorder;
	bool m_bEndCapture;
	int m_nChannelId;

	MapIdIp m_MapIdIp;//相机id,对应ip
	pthread_mutex_t m_mapIdIp_mutex;

	MapDspVideoUnit m_MapDspClient;
	pthread_mutex_t m_dspClientMapLock;	
};

#endif
