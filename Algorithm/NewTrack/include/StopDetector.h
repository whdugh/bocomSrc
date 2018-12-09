// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary 

// StopDetector.h: interface for the StopDetector class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _STOPDETECTOR_H_
#define _STOPDETECTOR_H_

#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include "ipp.h"

#include "libHeader.h"

#ifndef MAXLINENUMFORCALSHADOW
	#define  MAXLINENUMFORCALSHADOW 200
#endif

#include "BaseStruct.h"
#include "MvLineSegment.h"
#ifndef __SIFT__H__
	#include "sift_descr.h"
#endif
#ifndef _MY_DETECT_H
	#include "road_detect.h"
#endif

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#define  MAX_TIME_NUM  100   //最多存的时间段数目
#define  BLOCKWIDTHTOBG  1
#define  BLOCKHEIGHTTOBG 1
#define  GRAYINTVELTOBG  5
#include "sift_descr.h"
#ifndef OBJMAXSIZETRNUM
	#define OBJMAXSIZETRNUM  200
#endif 
#ifndef TRACKMAXLEN
	#define TRACKMAXLEN  50
#endif 

#ifndef SIFT_WIN_SIZE 
	#define SIFT_WIN_SIZE 4
#endif
#ifndef SIFT_BINS
	#define SIFT_BINS 8
#endif
#ifndef SIFT_DIMS
	#define SIFT_DIMS 128
#endif

// disjoint-set forests using union-by-rank and path compression (sort of).
typedef struct {
	int rank;
	int p;
	int size;
} mv_uni_elt;

class MvUniverse {
public:
	MvUniverse(int elements);
	~MvUniverse();
	int find(int x);  
	void join(int x, int y);
	int size(int x) const { return elts[x].size; }
	int num_sets() const { return num; }

private:
	mv_uni_elt *elts;
	int num;
};


//------------------------类CBgLineDetector-------------------------//
class CBgLineDetector  
{
public:
	CBgLineDetector();
	virtual ~CBgLineDetector();

	int    m_nListImgNo;
	void mvsetListImgNo(int nListImgNo)
	{
		m_nListImgNo = nListImgNo;
	};

	//接口函数
	void mvInit(int height,int width);
	void mvUnInit( );

	//判断线段是否为背景线段，对其进行标识
	void mvGetLineIsBgLabel(double dNow_ts, double dUpTimeTh,
		   int nAllLineNum, BLine *blAllLine, bool *bIsBgLine);

	void mvGetNoBgLineWithGiveLabel(int nAllLineNum, BLine *blAllLine,
			bool *bIsBgLine, int &nNoBgLineNum,BLine *blNoBgLine, 
			int &nBgLineNum, BLine *blBgLine);
private:
	void mvInitVar( );  //初始化变量

	void mvUpdataBgAppCount(bool bUpdate, int nAllLineNum,
		                    BLine *blAllLine);
	bool mv_judge_line_is_BgLine(BLine line, ushort *nLinePixelMapV, 
								 int nThres, float fThres);


private:
	int    m_height, m_width;    //图像高宽

	ushort *m_bgLinePixelV;      //前0-2M帧的结果(M帧更新一次)
	ushort *m_bgLinePixelV1;     //前0-M帧的结果(每帧均更新)
	ushort *m_bgLinePixelV2;     //前M-2M帧的结果(M帧更新一次)

	int *m_nPixelYX;             //线上点的Y,X坐标值

	double m_dTsUpdate;			 //更新的时间戳
	bool   m_bNeedUpdate;		//当前帧背景是否需要更新

	int m_nTimes;			 //在当前这段时间内经过的帧数
	int m_nUpFrameNum;		 //在当前这段时间内更新的帧数
	int m_nLastUpFrameNum;	 //在上一段时间内更新的帧数

	int m_nPixHaveTimesTh;   //每一像素出现次数的阈值
	CvPoint *m_ptAStaEndIdx;  //每个像素对应附近几个点的索引值
};

#endif //_STOPDETECTOR_H_
