// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#ifndef _VIDEO_SHUTTER_H_
#define _VIDEO_SHUTTER_H_
#include <vector>
using  namespace std;

#define MAX_OF_TWO max
#define MIN_OF_TWO min

#define  BUFFERSIZE   20

#include <algorithm>

#include <string>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>

#ifdef LINUX
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/vfs.h>
#include <sys/statvfs.h>
#else
#include <io.h>
#include <direct.h>
#endif


typedef struct CHANLL_FIL
{		
	int  nChanIndex;     //车道索引号	 
	int64 nFramSeq;   //帧号	
}ChanlFil;


typedef struct CAR_FIL_AREA 
{		
	CvRect nFilCarPal;     //在shringk压缩图上的坐标	 
	int64 nFramSeq;   //最后一次检测到的帧号
	int   nAvgMovDis;
	int   nDisAppearFram; //预测车多少帧后要消失
	bool  bCalDisappearTrd;
	CAR_FIL_AREA()
	{
		nFilCarPal =cvRect(0,0,1,1);
		nFramSeq = 0;
		nAvgMovDis = 0;
		nDisAppearFram = 20;
		bCalDisappearTrd = false;

	}
}car_filarea;



typedef struct ENERGY_m_SumPixel 
{		
	int nFlag;     //是否已经计算过能量	 
	double nEnergy;   //能量	
}energy_m_SumPixel;


//检测结果结构体
typedef struct _SKIP_VIDEO_OUT_RESULT
{	    
	int nChannel;//车道的序号

	_SKIP_VIDEO_OUT_RESULT()
	{	
		nChannel = -1;		
	}
}skip_video_out_result;


typedef struct _MVVIDEORGNSTRU {	
	vector<CvPoint2D32f> pPoints;	
	int nChannel;
	int nChanlIndex;
	_MVVIDEORGNSTRU()
	{	nChannel  = -1;	
	    nChanlIndex = -1;
		pPoints.clear();		
	}
}mvvideorgnstru;


typedef struct _MVPARK {	
	CvRect PaRec;
	int64  nFramSeq;
	bool  bShut;
	bool bPark;
	int  nBehdpakSum;
	int64 nParkFram;
	_MVPARK()
	{
		PaRec = cvRect(0,0,1,1);
		nFramSeq = 0;
		bShut = false;
		bPark = false;
		nBehdpakSum = 0;
		nParkFram =0;

	}
}mvPark;


//车牌结果结构体
typedef struct _CHANL_DATA
{
	int nChanlIndex;//车道索引 -从0开始
	int nChanlNum;//车道号-对应实际路面的车道

}ChanlData;
//车牌结果结构体
typedef struct _SKIP_CARNUM_RESULT : CvSetElem
{	
	CvRect  carnumpos[BUFFERSIZE];//相对对原始虚拟检测的大小
	int     number;
	int64   ts[BUFFERSIZE];
	int64   FramSeq[BUFFERSIZE];
	bool    IsCausedByLig[BUFFERSIZE];
	int  nuPSimPlat ;
	bool bPark;
	ChanlData nChanl;

	CvRect FistcarRect;
	int nForCarAvg;
	int nFirstAvg; //0;
	bool IsNoFirSet;
	bool IsShut ;
	bool  bNearCar;
	bool  bLigAndCar;
	bool  bLitConer;
	
	
	_SKIP_CARNUM_RESULT()
	{		
		//carnumpos = cvRect( 0, 0, 0, 0 );
		FistcarRect =cvRect( 0, 0, 1, 1 );
		nForCarAvg = 0;
		nFirstAvg = 0;
		IsNoFirSet = true;
		IsShut = false;
		for (int i =0; i<BUFFERSIZE;i++)
		{
			carnumpos[i] = cvRect( 0, 0, 0, 0 );
			ts[i] = 0;
			FramSeq[i] = 0;
			IsCausedByLig[i] = false;
			bPark = false;
		}
		number = 0;
		nuPSimPlat = 0;
		nChanl.nChanlIndex = 0;
		nChanl.nChanlNum = 0;
		bNearCar = false;
		bLigAndCar = false;
		bLitConer = false;
		
		
	}
}skip_carnum_result;

typedef struct CAR_CURRENT_INF
{
	CvRect CarRec;
	int64  ts;
	int64  FramSeq;
	bool   IsCasdLig;
	bool    CasLigAddPlat;
	CAR_CURRENT_INF()
	{
		IsCasdLig = false;
		CasLigAddPlat = false;
	}

}car_current_inf;

typedef struct CIRCLE_ATTRIBUTION
{
	 int center_x;
	 int center_y;
	 int radius;
	 CvRect LigRec;
}circle_attribution;

typedef struct LIGWORLD_INF
{
	 double CirlWoldPosX;
	 double CirlWoldPosY;
	 int nLabNum;

}ligworld_inf;

typedef struct LIGMATCH_INF
{
	int nNum;
	int nMatchNum;

}ligmatch_inf;

typedef struct RECT_LIG
{
	CvRect  CaRect;
	bool    ISCasByLig;
	bool    CasLigAndPlat;
	RECT_LIG()
	{
		ISCasByLig = false;
		CasLigAndPlat = false;
	}
         
}rect_lig;

typedef struct LINE_INF
{
	CvPoint  StartPoi;
	CvPoint  EndPoi;
    int      nLeng;
	LINE_INF()
	{
		nLeng = 1;
		StartPoi = cvPoint(0,0);
		EndPoi  = cvPoint(0,0);

	}


}Line_inf;

typedef struct _ChANLVEHIC
{

	int64 nPassFram;
	_ChANLVEHIC()
	{
		
		nPassFram = 0;
		
	}

}ChanlVeh;
typedef struct _PEEKINF 
{		
	CvPoint Pos;     //是否已经计算过能量	 
	int  nHig;   //能量
	CvPoint MovePoint;
	_PEEKINF()
	{
		Pos = cvPoint(0,0);
		MovePoint = cvPoint(0,0);
		nHig = 0;
	}
}Peekinf;


typedef struct _MOVE_OBJ
{
	CvRect ObjRec;
	int64  nFramSeq;

	bool   bCauByCar;

	bool   bCauByLig;
	CvRect RecByLig;

	_MOVE_OBJ()
	{
		ObjRec = cvRect(0,0,1,1);
		nFramSeq = 0;
		bCauByCar = false;
		RecByLig = cvRect(0,0,1,1);
		bCauByLig =false;
	}

}MovObj;

typedef struct _CAR_HIT
{
	CvRect ShutRec;
	int nHitSerNum;
	_CAR_HIT()
	{
        ShutRec = cvRect(-10,-10,1,1);
		nHitSerNum = 0;
	}
}CarHit;

//运动前景目标结构体
typedef struct _SKIP_MOVOBJ_RESULT : CvSetElem
{	
	CvRect  MovPos[BUFFERSIZE]; //相对压缩虚拟检测区域的图像坐标
	int     number;
	int64   FramSeq[BUFFERSIZE];
	CvRect    RecCausedByLig[BUFFERSIZE];
	int       nLigLapNum;
	ChanlData nChanl;
	bool IsShut;
	bool   bSerHig;
	bool   bSerYAdd;
	int   nTrackNum;
	int   nShutTracNum;
	bool  IsMeetSHut;
	uchar   nGrayVal;




	_SKIP_MOVOBJ_RESULT()
	{		

		for (int i =0; i<BUFFERSIZE;i++)
		{
			MovPos[i] = cvRect( 0, 0, 1, 1 );
			FramSeq[i] = 0;
			RecCausedByLig[i] = cvRect( 0, 0, 1, 1 );


		}
		number = 0;
		nChanl.nChanlIndex = 0;
		nChanl.nChanlNum = 0;
		nLigLapNum = -1;
		IsShut = false;
		bSerHig = false;
		bSerYAdd = false;
		nTrackNum = 0;
		nShutTracNum = 0;
		IsMeetSHut = false;
		nGrayVal = 255;

	}
}skip_movobj_result;




#endif
