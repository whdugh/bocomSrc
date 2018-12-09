// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _DHCAMERA_H_
#define _DHCAMERA_H_

//#ifdef DIO_RTSP

#include "Common.h"
#include "AbstractCamera.h"
#include "RoadDecode.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#ifdef DIO_RTSP
#include "StrmCore.h"
#include "BMErrNo.h"
#else
#include "CRtspClient.h"
#endif

#include "structdef.h"
#include "OnvifCameraCtrl.h"
#include "FileManage.h"
#include "CSeekpaiDB.h"


struct DHRecordEventInfo
{
	char szRecordName[200];
	char szPlace[200];
	char szDirection[20];
	int  nBeginTime;  //开始录像时间
	bool bEndRecord;
	FILE *pOpenFile; //打开的文件句柄
};

#ifdef DIO_RTSP
#include "DIO.h"
using namespace DIO;
#endif

#define MAX_CHANNEL 25
#define MAX_BUFFER_SIZE 10

class CDHCamera: public AbstractCamera
{
public:
	CDHCamera(int nCameraType);
	~CDHCamera();

	virtual bool Open();
	virtual bool Close();

	bool OpenDH();
	bool CloseDH();

	virtual bool ReOpen();
	//切换工作方式
	virtual bool ChangeMode();

	//获取相机默认模板
	virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);

	//设置设备编号
	virtual void SetDeviceID(int nDeviceID);

	//解码
	bool DecodeFrame(unsigned char* pData,int dataLen,unsigned int pts,int iFrameRate,int nKeyFrame);

	int getWidth(){return m_nWidth;}
	int getHeight() {return m_nHeight;}
	void setWidth(int i){m_nWidth = i;}
	void setHeight(int i) {m_nHeight = i;}

	unsigned char* getFrameList(int i) {return m_FrameList[i];}
	unsigned char* getFrameBuffer() {return m_pFrameBuffer;}

	void setFrameList(int i, unsigned char* p) {m_FrameList[i] = p;}
	void setFrameBuffer(unsigned char* p)  {m_pFrameBuffer = p;}

	void addH3cFrame(int i){this->AddFrame(i);}

	//码流回调函数
	static int32_t StrmCallback(uint64_t ubiStrmId, unsigned char *pData,  int32_t iDataLen, void *pParam);
	int32_t DealH264Stream(uint64_t ubiStrmId, unsigned char *pData,  int32_t iDataLen);
	//H264编码转化为YUV编码
	static void * DealH264ToYUVStream(void * lpParam);
	void * DealH264ToYUVStreamFunc();

	UINT32 GetRealPlayId();

	//手动控制
	virtual int ManualControl(CAMERA_CONFIG cfg);

	//添加录像命令
	virtual int AddRecordEvent(int nChannel,std::string result);
	std::string PopEvent();


	void CreateEventThread();
	static void * DealEventRecordFunc(void * lpParam);
	void * DealEventRecord();

	int WriteFrameToRecordFile(unsigned char *pData,  int32_t iDataLen);
	int GetTimeT();

private:
	//初始化
	void Init();

	//将客户端设置的相机参数转换为相机自身能识别的参数
	virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

public:
	RoadDecode m_Decoder;   // h264转yuv编码库
	uint64_t m_ubiStreamId; //rtsp直播编号

	#ifdef DIO_RTSP
	CStrmParser m_StreamParser; //码流分帧库
	sMediaSample *pMs;//提取分帧后的数据
	#else
	CRtspClient m_RtspClient;
	#endif

	COnvifCameraCtrl *m_pCameraCtrl;
	

private:
	//相机编号
	int m_nDeviceId;
	pthread_mutex_t m_Event_Mutex;
	typedef std::list<std::string> RecorderEvent;
	RecorderEvent	m_EventList;
	bool m_bEndRecorder;

	typedef map<int,DHRecordEventInfo*> mapEventINFO;
	mapEventINFO m_mapRecordEvent;
	int m_nPrintTime;
	
};
#endif
//#endif

