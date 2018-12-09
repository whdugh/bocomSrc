#ifdef BASLER_CAMERA
#ifndef BASLER_CAMERA_H
#define BASLER_CAMERA_H

#include"AbstractCamera.h"
#include <pylon/PylonIncludes.h>
#include <iostream>
using namespace Pylon;
using namespace GenApi;
using namespace GenICam;

#define GAIN_DB_UNIT 51
#define GAIN_MAX 1023
#define GAIN_MIN 0
#define INFINITE	0xFFFFFFFF


class BaslerCamera :public AbstractCamera
{
public:
	BaslerCamera(int nCameraType);
	~BaslerCamera();

	//打开视频
	virtual bool Open();
	virtual bool Close();
	//打开文件
	//手动控制
	virtual int ManualControl(CAMERA_CONFIG cfg);
	//自动控制
	virtual int  AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn);
	//读取
	virtual int ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert = true);
	//重新打开
	virtual bool ReOpen();
	//切换工作方式
	virtual bool ChangeMode();
	//获取相机默认模板
	virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);
	virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

	//控制相机
	void ControlCamera(CAMERA_CONFIG cfg);
	//灯控制
	int   LightControl(bool bInit = false);

	//接收视频流
	void RecvVideoStream();

private:
	CTlFactory* m_TlFactory;
	IPylonDevice* m_Camera;
	BYTE* m_Buffer;

	DeviceInfoList_t m_Devices;
	IStreamGrabber* m_StreamGrabber;
	INodeMap* m_CameraNodeMap;

	StreamBufferHandle m_hBuffer;
	static WaitObjectEx m_WaitObjectEx;

	pthread_t m_pthreadId;

	bool m_bStopCamera;

	int m_uMaxPE;
	int m_uMaxGain;
	int m_uMaxGamma;
};
#endif
#endif

