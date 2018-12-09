// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef PICLIB_H
#define PICLIB_H

#include"AbstractCamera.h"

class PicLib :public AbstractCamera
{
public:
	//构造
	PicLib(int nCameraType,int nPixelFormat);
	bool OpenFile(const char* strFileName);
	void DealPicFrame();
	void Init();
	bool Close();
	bool ChangeMode(){}
	void GetDefaultCameraModel(CAMERA_CONFIG& cfg){}
	void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false){}
	//析构
	~PicLib();
private:
	std::string m_strFilePathName;
};

#endif
