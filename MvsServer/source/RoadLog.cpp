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

#include "Common.h"
#include "CommonHeader.h"
#include "RoadLog.h"


CSkpRoadLog g_skpRoadLog;

CSkpRoadLog::CSkpRoadLog()
{
	return;
}

CSkpRoadLog::~CSkpRoadLog()
{

	return;
}

//初始化
bool CSkpRoadLog::Init()
{
	//日志存储在DB中,初始化无工作
	return true;
}

//释放
bool CSkpRoadLog::UnInit()
{
	//日志存储在DB中，释放无工作

	return true;
}

//普通日志,日志级别-1
bool CSkpRoadLog::LogNormal(const char* chNormal)
{
	//轮换为SQL 语句，添加到执行列表

//	printf("%s",chNormal);

	// Save to mySQL
//	g_skpDB.SaveSysEvent(GetCurrentTime(),chNormal,1);
	WriteLog(chNormal,ALARM_CODE_UNKNOWN);

	return true;
}
//警告日志,日志级别-2
bool CSkpRoadLog::LogWarning(const char* chWarning)
{
	//轮换为SQL 语句，添加到执行列表
//	printf("%s",chWarning);

	// Save to mySQL
//	g_skpDB.SaveSysEvent( GetCurrentTime(),chWarning,2);
	WriteLog(chWarning,ALARM_CODE_UNKNOWN);
	return true;
}
//错误日志,日志级别-3
bool CSkpRoadLog::LogError(const char* chError)
{
	//轮换为SQL 语句，添加到执行列表

//	printf("%s",chError);
	// Save to mySQL
//	g_skpDB.SaveSysEvent(GetCurrentTime(),chError,3);
	WriteLog(chError,ALARM_CODE_UNKNOWN);

	return true;
}


//写日志
void CSkpRoadLog::WriteLog(const char* chText,unsigned int uCode,bool bAlarm,int nCameraId)
{
	printf("%s",chText);

	String strTime = GetTimeCurrent();

	g_skpDB.SaveLog(strTime,chText,uCode,bAlarm,nCameraId);
}
