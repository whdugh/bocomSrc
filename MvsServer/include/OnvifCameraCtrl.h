// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _ONVIFCAMERACTRL_H_
#define _ONVIFCAMERACTRL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Markup.h"
#include <iostream>
#include "string"
#include <map>
#include<math.h>
#include <sys/time.h>
#include <list>
using namespace std;

typedef struct tagOnvifZoomInfo
{
	int nPanTiltx;
	int nPanTilty;
	float fPanTiltz;
}OnvifZoomInfo;

struct AutoCameraPtzTrack
{
	int nTaken;
	int nPanTiltX;
	int nPanTiltY;
	int nPanTiltZoom;
	int nBeginTime;
};

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (unsigned int)(~0)
#endif


class COnvifCameraCtrl
{
public:
	COnvifCameraCtrl();
	~COnvifCameraCtrl();

	bool Init(int nCameraType,const char *szIp,int iPort);
	//相对移动
	bool PTZRelativeMove(double PanTiltX,double PanTiltY,double ZoomX);
	bool PTZRelativeMoveDH(double PanTiltX,double PanTiltY,double ZoomX);
	bool PTZRelativeMoveSanYo(double PanTiltX,double PanTiltY,double ZoomX);
	//调用预置位
	bool PTZGotoPreset(int iPresetToken);
	bool PTZGotoPresetDH(int iPresetToken);
	bool PTZGotoPresetSanYo(int iPresetToken);
	//设置预置位
	bool PTZSetPreset(int iPresetToken);
	//获取预置位
	bool PTZGetPreset(AutoCameraPtzTrack *&m_pPresetInfo,int &m_nPresetCount);
	//删除预置位
	bool PTZRemovePreset(int iPresetToken);
	//获取当前云台信息，
	bool GetPTZStatus(string &strPanTiltX,string &strPanTiltY,string &strZoomX);
	//获取计算后的ZOOM值 //入参为当前倍率 需要放大的倍率0 返回值为需要调整的ZOOM值
	double GetCalculateCameraZoom(double nNowZoom,double nZoom);
	//获取当前的倍率值
	double GetNowCameraZoom(double nNowZoom);
	float GetPanTilta();
	float GetPanTiltb();
    float GetPanTiltc();
	float GetPanTiltd();
	int   GetPanTiltZoom();

private:
	int WaitConnect(int nSocket, float fTimeout);
	int WaitRecv(int nSocket, float fTimeout);
	int CloseSocket(int& fd);//可重复调用
	bool GetPTZServiceAddr();
	bool GetPTZProfileToken();
	int GetTimeT();
	bool LoadConfig(char *pConfig);
private:
	string m_CameraIp;
	int m_iPort;

	int m_nCameraType;
	string m_PTZServiceAddr;
	string m_ProfileToken;

	//球机刻度
	float m_fPanTilta;
	float m_fPanTiltb;
	float m_fPanTiltc;
	float m_fPanTiltd;
	int   m_nPanTiltZoom;
	int m_nRectZoomCount;
	OnvifZoomInfo *pRectZoomInfo;
	int m_nAbsoluteZoomCount;
	OnvifZoomInfo *pAbsoluteZoomInfo;

};

#endif

