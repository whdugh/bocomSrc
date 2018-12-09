// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   15:05
	filename: 	e:\BocomProjects\find_target_lib\include\NoPassingInfo.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	NoPassingInfo
	file ext:	h
	author:		Durong
	
	purpose:	车道的禁行信息。
*********************************************************************/

#ifndef NO_PASSING_INFO_H
#define NO_PASSING_INFO_H

//#define ODD_ODD 0x01 //奇数
//#define ODD_EVE 0x02

#define MON 0x01//周一
#define TUE 0x02//周二
#define WEN 0x04//周三
#define THU 0x08//周四
#define FRI 0x10//周五
#define SAT 0x20//周六
#define SUN 0x40//周日
#define WEK 0x7F//全周
#define WED 0x60//周六、周日
#define WKD 0x1F//周一到周五，工作日

// 禁止通行信息。
// 包含禁止通行的时间段，以及禁止通行的车辆类型。
typedef struct _NoPassingInfo
{
	unsigned char ucWeekDay; //禁止通行的周天。可以按位或。

	int nStart;   // 禁止通行时间断开始。从0时0分0秒到禁止通行时间的秒数。
	int nEnd;     // 比如从1点到两点是禁行时间，则nStart = 1*60*60, nEnd = 2*60*60

	int nVehType; // //0表示不禁行；1表示禁行小车；2表示禁行大车；3表示禁行所有车辆

	_NoPassingInfo()
	{
		ucWeekDay = WEK;
		nStart    = -1;
		nEnd      = -1;
		nVehType  = 0;
	}
} NoPassingInfo;

#endif