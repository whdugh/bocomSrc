// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef MATCH_CONSTDEF_H
#define MATCH_CONSTDEF_H

//特征信息(由检测器发往次控)
#define FEATURE_INFO  0x10100000
//特征标定信息(由检测器发往次控)
#define FEATURE_CALIBRATION  0x10100009
////////////////////////////////////////////////
//人脸特征信息(由检测器发往次控)
#define FACE_FEATURE_INFO  0x10200000

//通讯包头
typedef struct _MATCH_MSG_HEADER
{
	UINT32 uCameraID;     //相机编号
	UINT32 uCmdLen;		//命令长度
	UINT32 uCmdID;		//命令类型
	UINT32 uCmdFlag;		//命令标志

	_MATCH_MSG_HEADER ()
	{
		uCameraID = 0;
		uCmdLen = 0;
		uCmdID = 0;
		uCmdFlag = 0;
	}
}MATCH_MSG_HEADER;

#endif

