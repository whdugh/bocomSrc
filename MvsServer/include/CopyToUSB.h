// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _COPYTOUSB_H_
#define _COPYTOUSB_H_

#include "Common.h"
#include "global.h"
#include "MysqlTransitionSqliteCyToUSB.h"
#include <sys/stat.h>
#include <sys/mount.h>

#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <sys/types.h> 
#include <linux/kd.h> 


typedef struct { 

	int freq; /* 我们期望输出的频率，单位为Hz */ 

	int length;  /* 发声长度，以微秒为单位*/ 

	int reps;      /* 重复的次数*/ 

	int delay;      /* 两次发声间隔，以微秒为单位*/ 

} beep_parms_t;

//usb拷贝标识位
extern volatile bool g_bEndToUsb;

class CopyToUSB
{
public:
	CopyToUSB();
	~CopyToUSB();
	bool Init();
	void RunToCopy();

	int CopyFile(char *copyfrom, char *copyto,UINT32 isize);

	
	bool GetPlatePic();
	
	bool GetVideo();
	bool Close();
	int GetRandCode();

	int CopySql();     //copy 数据库的；
	void cpfile(char *spathname,char *tpathname);    // 把一个文件复制到另一个目录下面

	int CheckDataOutOfDate();
	int ProcDataOutOfLimitDate(int nLimitDays);
	int GetDayTimeSpan(int nCheckDay, struct tm *nowTime, struct tm *oldTime, unsigned long & ulStart, unsigned long & ulEnd);
	//UINT32 GetSpeed(int nChannel,int nRoadId);

	UINT32 GetHaveCopy_Sql();

	//更新记录状态
	bool UpdateRecordStatus(unsigned int uID);
	//更新记录状态
	bool UpdateRecordStatus(unsigned int uMinID,unsigned int uMaxID);
	bool HasUsb();
	bool MountUsb();
	bool IsExistUsb();
	void CopyLog();
protected:
	string strPicPath;
	string strVideoPath;
	UINT32 uPicSize;
	UINT32 uSmallPicSize;
	UINT32 uPicWidth;
	UINT32 uPicHeight;
	UINT32 uViolationType;
	//线程ID
	pthread_t m_nThreadId;
	pthread_t m_nThreadLogId;
	pthread_t m_nThread; //报警线程
	
	#ifdef SQLITE_OK
	MysqlTransitionSqlteCyToUSB * sqlteCp;
	#endif
	
	//车道限速值映射
	map<UINT32,UINT32> m_MapSpeed;

	UINT32 haveCopy_sql;
	struct tm m_nCheckTm;

};

//USB拷贝
extern CopyToUSB g_copyToUSB;

#endif

