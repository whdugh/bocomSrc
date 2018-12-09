// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _H3CCAMERA_H_
#define _H3CCAMERA_H_

#define NOH3CCAMERA

#ifndef NOH3CCAMERA

#include "Common.h"
#include "AbstractCamera.h"
#include "RoadDecode.h"

#include "imos_def.h"
#include "imos_public.h"
#include "sdk_def.h"
#include "xp_func.h"
#include "sdk_func.h"
#include "sdk_struct.h"

#include "imos_as_def.h"
#include "imos_bp_def.h"
#include "imos_errcode.h"
#include "imos_mm_def.h"
#include "imos_mp_def.h"
#include "imos_terminal_def.h"
#include <stdio.h>

#define MAX_CHANNEL 25
#define MAX_BUFFER_SIZE 10

class H3CCamera: public AbstractCamera
{
public:
	H3CCamera(int nCameraType);
	~H3CCamera();

	virtual bool Open();
	virtual bool Close();

	virtual bool ReOpen();
	//切换工作方式
	virtual bool ChangeMode();

	//获取相机默认模板
	virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);

	//设置设备编号
	virtual void SetDeviceID(int nDeviceID);

	//解码
	bool DecodeFrame(unsigned char* pData,int dataLen,unsigned int pts,int iFrameRate,int nKeyFrame);

	bool imos_init(char szServerAdd[IMOS_IPADDR_LEN], ULONG ulServerPort);
	bool imos_clean();
	bool imos_login(char username[], char passwd[]);
	bool imos_logout();
	bool imos_startReal(char *pXpCode);
	bool imos_stopReal(char *pXpCode);
	bool imos_setVideoDataCallBack();
	bool imos_StartPlayer();
	bool imos_stopPlayer();

	int getWidth(){return m_nWidth;}
	int getHeight() {return m_nHeight;}
	void setWidth(int i){m_nWidth = i;}
	void setHeight(int i) {m_nHeight = i;}

	unsigned char* getFrameList(int i) {return m_FrameList[i];}
	unsigned char* getFrameBuffer() {return m_pFrameBuffer;}

	void setFrameList(int i, unsigned char* p) {m_FrameList[i] = p;}
	void setFrameBuffer(unsigned char* p)  {m_pFrameBuffer = p;}

	void addH3cFrame(int i){this->AddFrame(i);}

private:
	//初始化
	void Init();

	//将客户端设置的相机参数转换为相机自身能识别的参数
	virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

public:
	LOGIN_INFO_S  m_login;
	BOOL_T m_bLogin;
	PLAY_WND_INFO_S m_playInfo[MAX_CHANNEL];
	IN CHAR  m_serverHost[IMOS_IPADDR_LEN];
	IN ULONG  m_ulServerPort;
	RoadDecode m_Decoder;
private:

	pthread_t m_nKeepAliveId;

	//相机编号
	int m_nDeviceId;
};
#endif
#endif

