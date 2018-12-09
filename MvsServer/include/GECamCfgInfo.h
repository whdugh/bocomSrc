// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _GECAMCFGINFO_H_
#define _GECAMCFGINFO_H_

#include "CMDToCamera.h"

typedef struct tagEE_EXPOSURE_INFO
{
	unsigned int	dwGain;
	unsigned int	dwShutterTime;
	unsigned int	dwGainLimit;
	unsigned int	dwShutterTimeUpperLimit;
	unsigned int	dwMeanBrightnessThreshold;
	unsigned int	dwMode;
	unsigned int	dwFlashLampEnable;
}EE_ExposureInfo;

typedef struct tagEE_APPERANCE_INFO
{
	unsigned int	dwWhiteBalanceEnable;
	unsigned int	dwWhiteBalanceMode;
	unsigned int	dwIMGSampleFMT;
	unsigned int	dwLUTEnable;
}EE_ApperanceInfo;

typedef struct tagEE_METERING_INFO
{
	unsigned int	dwAutoMR;
	unsigned int	dwAutoMRPeriod;
	unsigned int	nMRLeft;
	unsigned int	nMRTop;
	unsigned int	nMRRight;
	unsigned int	nMRBottom;
}EE_MeteringInfo;

typedef struct tagEE_GRAB_INFO
{
	char		szRemark[256];
	unsigned int		dwLaneNum;
	unsigned int		dwTrgType;
	unsigned int		dwCoilPitch;
	unsigned int		dwSwitchAutoGrab;
}EE_GrabInfo;

typedef struct tagEE_NETWORK_INFO
{
	unsigned int	dwIP;
	BYTE	aMAC[6];
}EE_NetworkInfo;

typedef struct tagEE_SYSTEM_INFO
{
	BYTE	aDataTime[7];
	unsigned int	dwWatchDogEnable;
	unsigned int	dwWatchDogTimer;
	float	fTemperature;
	unsigned int	dwSerial;
}EE_SystemInfo;

#define EE_CAM_IO_NUM			8
#define EE_CAM_IO_OPTIONLEN		16

typedef struct tagEE_IO_INFO
{
	BYTE	aIOOptions[EE_CAM_IO_NUM][EE_CAM_IO_OPTIONLEN];
}EE_IOInfo;

typedef struct tagEE_STREAM_INFO
{
	unsigned int	dwRunMode;
}EE_StreamInfo;

typedef struct tagEE_ISO_INFO
{
	unsigned int	dwInMode;
	unsigned int	dwOutMode;
	unsigned int	dwPowerSynEnable;
	unsigned int	dwPowerSynACMode;
	unsigned int	dwPowerSynDelayTime;
	unsigned int	dwFlashLampEnable;
	unsigned int	dwFlashLampMode;
	unsigned int	dwFlashLampOutWidth;
}EE_ISOInfo;

#define MAX_RUNMODE_COUNT	2
typedef struct tagEE_CONFIG_INFO
{
	EE_ExposureInfo			ExposureInfo[MAX_RUNMODE_COUNT];
	EE_ApperanceInfo		ApperanceInfo;
	EE_MeteringInfo			MeteringInfo;
	EE_GrabInfo				GrabInfo;
	EE_NetworkInfo			NetworkInfo;
	EE_SystemInfo			SystemInfo;
	EE_IOInfo				IOInfo;
	EE_StreamInfo			StreamInfo;
	EE_ISOInfo				ISOInfo;
}EE_CfgInfo;

class CGECamCfgInfo
{
public:
	CGECamCfgInfo();
	virtual ~CGECamCfgInfo();
public:
	EE_CfgInfo		m_CfgInfo;
	CCMDToCamera	*pCMDToCamera;
public:
	BOOL			GetAllCfgInfo(CAMERA_CONFIG& cam_cfg);
};

#endif



