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

#ifndef SKP_ROAD_LOG_H
#define SKP_ROAD_LOG_H

#include "global.h"

/******************************************************************************/
//	描述:智能交通检测系统日志处理模块。
//	作者:徐永丰
//	日期:2008-4-2
/******************************************************************************/
class CSkpRoadLog
{
public:
	//构造
	CSkpRoadLog();
	//析构
	~CSkpRoadLog();
public:
	//初始化
	bool Init();
	//释放
	bool UnInit();
	//错误日志
	bool LogError(const char* chError);
	//普通日志
	bool LogNormal(const char* chNormal);
	//警告日志
	bool LogWarning(const char* chWarning);

	//写日志
	void WriteLog(const char* chText,unsigned int uCode,bool bAlarm=false,int nCameraId = 0);
private:

};

extern CSkpRoadLog g_skpRoadLog;

#endif
