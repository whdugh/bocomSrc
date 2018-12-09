// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _CFCAMERA_H_
#define _CFCAMERA_H_

#ifdef CFCAMERA

#include"AbstractCamera.h"
#ifdef H264_DECODE
#include "RoadDecode.h"
#endif
#include "H264RtspRevDecode.h"


class CCFCamera:public AbstractCamera
{
public:
	CCFCamera(int nCameraType);
	~CCFCamera();

	virtual bool Open();
	virtual bool Close();

	//设置设备编号
	virtual void SetDeviceID(int nDeviceID);

	//获取视频流
	virtual void PullStream();

	virtual bool ReOpen();

	//h264图像采集
	void CaptureFrame(char* pBuffer ,unsigned int BufferSize);
    void CaptureTcpFrame(char* pBuffer ,unsigned int BufferSize);
	//接收h264数据
	void RecvData();
//     void hdr_callback(char *buff, int bytes, void *param);
//     void pure_data_callback(char *buff, int bytes, void *param);
//     void data_callback(char *buff, int bytes, void *param);

private:
	//初始化
	void Init();

	//处理长峰平台特有数据
	void DealCFData(string& strMsg);

private:

#ifdef H264_DECODE
	RoadDecode m_Decoder;
#endif

	unsigned int m_uResRecord;

	//h264图象
	string m_strData;
	unsigned long handle;
	FILE * fp;
	

	//CRtspClient RtspClient;
	H264RtspRevDecode m_rtspReceiver;
};
#endif
#endif
