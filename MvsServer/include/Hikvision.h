// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _HIKVISION_H_
#define _HIKVISION_H_


#ifdef HIKVISIONCAMERA

#include"AbstractCamera.h"
//#include "PlayM4.h"
#ifdef H264_DECODE
#include "RoadDecode.h"
#endif
#include "HikvisionCommunication.h"


class CHikvision:public AbstractCamera
{
public:
	CHikvision(int nCameraType);
	~CHikvision();

	virtual bool Open();
	virtual bool Close();
	//打开文件
	virtual bool OpenFile(const char* strFileName);

	virtual bool ReOpen();
	//获取视频流
	virtual void PullStream();

	//手动控制
	virtual int ManualControl(CAMERA_CONFIG cfg);
	
	//h264图像采集
	void CaptureFrame(char* pBuffer ,unsigned int BufferSize);

	static void fCallback(int handle,const char* data,int size,void *pUser,int iDataType);

	//h264解码
	void DecodeFrame();

private:
	//初始化
	void Init();

private:

#ifdef H264_DECODE
	RoadDecode m_Decoder;
#endif

	const char *m_pURL;//预览返回的播放路径
	int m_hStream;//PlayVideo返回值
	
	//string m_strData;

	list<string> m_listStrData;//码流数据

	void* m_hAnalyze;
};
#endif
#endif
