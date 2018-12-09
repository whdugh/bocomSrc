/* 版权所有 2013 上海博康智能信息技术有限公司
* Copyright (c) 2013，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：BaseComStruct.h
* 摘要: 公用结构体的整理归类
* 版本: V1.0
* 作者: 贺岳平
* 完成日期: 2013年12月
*/ 
//////////////////////////////////////////////////////////////////////

#ifndef __BASE_COM_STRUCT_H
#define __BASE_COM_STRUCT_H

#include <vector>
#include <algorithm>

#include <string>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>

#ifdef  OPENCV245
	#include "opencv2/opencv.hpp"
	#include "opencv2/objdetect/objdetect.hpp" 
	#include "opencv2/features2d/features2d.hpp" 
	#include "opencv2/highgui/highgui.hpp" 
	#include "opencv2/calib3d/calib3d.hpp" 
	#include "opencv2/imgproc/imgproc_c.h" 
	#include "opencv2/imgproc/imgproc.hpp"   
	#include "opencv2/nonfree/features2d.hpp"
	#include "opencv2/legacy/legacy.hpp"
	#include "opencv2/legacy/compat.hpp"

	#include <wtypes.h>
	using namespace cv; 
#else
	#include "cv.h"
	#include "highgui.h"
	#include "cxcore.h"
#endif


#ifdef LINUX
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>

	#include <sys/time.h>
	#include <sys/timeb.h>
	#include <sys/vfs.h>
	#include <sys/statvfs.h>

	#ifndef CHDIR
		#define CHDIR
		#define _chdir(a) chdir(a)
	#endif
	#ifndef MKDIR
		#define MKDIR
		#define _mkdir(a) mkdir(a, 0777)
	#endif
	#ifndef ACCESS
		#define ACCESS
		#define _access(a, b) access(a, b)
	#endif
#else
	#include <io.h>
	#include <direct.h>
#endif

using namespace std;

#ifndef USE_TIME_FOR_OUTPUT
	#define USE_TIME_FOR_OUTPUT
	//耗时输出类型
	enum useTimeForOutPut
	{
		ONLY_PRINTF = 10,
		ONLY_WRITE = 20,
		PRINTF_WRITE,
	};
#endif


//排序所用的结构体
typedef struct _sortStruct 
{
	double  dData;
	int     nIndex;
}SortStruct;



//---------------------时耗计算结构体---------------------//
//结构体在作为类的成员时，最好能在程序初始化的时候重新设置初值
typedef struct _AN_ComputeTime
{	
	double dFreq;
	int64  nStart;
	double time;

	_AN_ComputeTime( );	

	void initComputeTime( );	

	void setStartTime( );	

	double getUseTime( );	

	void printUseTime( char chPrintInfo[] );	//打印耗时到屏幕	

	void printAndWirte( char chDir[], 
		  char chTxtName[], char chPrintInfo[],
		  int nMod, const char chWriteMod[] );	//打印和输出到文本文件
	
}ComputeTime;


//---------------------日期的结构体---------------------//
typedef struct _AN_DateStruct
{
public:
	int nNowYear, nNowMon, nNowDay;
	int nNowHour, nNowMin, nNowSec;

	_AN_DateStruct( );	

	void getNowDateTime( );

}AnDateStruct, MvDateStruct;


//--------------------循环覆盖存储 结构体--------------------//
#ifndef MAX_CYCLE_SIZE
	#define MAX_CYCLE_SIZE  1000
#endif
typedef struct _cycleReplace
{
	int        m_nMaxSize;
	bool       m_bFull; 
	int        m_nNow;
	int64      m_nTotalCnt;
	double     m_dTsFristAdd; //第一个元素加入的时间
	double     m_dTsAdd[MAX_CYCLE_SIZE];

	_cycleReplace( );

	void mvGiveMaxSize( int n );

	int mvAddOneElem( double dTsNow );

	//得到所有的元素(从今往前)
	void mvGetAllElem( vector<int> &vecSearchNo );

	//得到从今往前几个的元素
	void mvGetPreElem( int nPreCnt, vector<int> &vecSearchNo );

	//搜索距今时间差为给定值之内的元素(要求时间戳是增加的)
	void mvSearchInPreTimeElem( double dTsNow, 
		double dPreTime, vector<int> &vecSearchNo );

	//搜索距今时间差为给定值之内的元素(要求时间戳是增加的)
	vector<int> mvSearchInPreTimeElem( double dTsNow, double dPreTime );

	//搜索在给定的两个时间戳之间的元素(要求时间戳是增加的)
	void mvSearchIn2GiveTimeElem( double dTsStart, 
		double dTsEnd, vector<int> &vecSearchNo ); 

}CycleReplace;



//扩展的直方图应用结构体
typedef struct StruExtendHistApp
{
public:
	StruExtendHistApp( );	
	StruExtendHistApp( int nBin, double dMin, double dMax );
	~StruExtendHistApp( );

	//将直方图内的值全设为0
	void mvResetHistToZero( );

	//添加一个元素
	void mvAddElem( double dV );

	//获取直方图的分布概率
	void mvGetHistDist( );

	//获取满足分布概率要求的直方图的位置
	bool mvGetSuitDistPro( double &dVSuit, double dProRequire );

private:
	bool   m_bInit;

	int    m_nBin;          //划分的数目
	double m_dMin, m_dMax;  //最小值和最大值

	double m_dStep;         //每份的取值

	int    *m_pHist;

	int    m_nSumCnt;
	int    *m_pHistDist;

	void mvInitVar( );
	void mvUninit( );

}ExtendHistApp;


//---------------------------------------
#endif 
