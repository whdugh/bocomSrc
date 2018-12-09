
//车辆车顶判断
#ifndef _AN_VEHICLE_TOP_JUDGE_H_
#define _AN_VEHICLE_TOP_JUDGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "ForeImgApp.h"

#include "comHeader.h"  //放最后

using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_SMALL_FUNCTION;



//-------------------------------------------------------------//
//用于判断给定区域是否为大车顶部的结构体
typedef struct StruBigVehicleTopJudge
{
public:
	StruBigVehicleTopJudge( );
	~StruBigVehicleTopJudge( );
	void mvInitVar( );

	//初始化大车顶部判断器
	void mvInitBVTopAdjudicator(IplImage *pCarWImg, 
			IplImage *pCarHImg, IplImage *pGrayImg,
			IplImage *pFkRoadImg);

	//释放
	void mvUninitBVTopAdjudicator( );


	//判断给定区域是否对应为大车或较大车的车顶
	bool MvIsTopOfBigMidVehicle(const CvRect &rectObjArea, 
			CvRect &rectBMVArea, double dTsNow, bool bBigV=true);


private:
	//获得得到给定目标区域所对应的countour前景mask图
	bool mvGetContours4ErodeFkImg( 
			double dTsNow  //当前的时间戳
		);


	//获得得到给定目标区域所对应的countour前景mask图
	IplImage* mvGetContoursMaskOfGiveRect( 
			double dTsNow,				    //当前的时间戳
			const CvRect rectObjFkArea,  	//给定的目标区域
			const IplImage *pFkAllInteImg,	//全图区域内的前景积分
			CvRect &rectBest4Obj,           //得到的最好的目标rect
			int nContourAreaTh			    //要求的轮廓面积阈值
		);

private:
	IplImage *m_pCarWImg;
	IplImage *m_pCarHImg;
	IplImage *m_pGrayImg;
	IplImage *m_pFkRoadImg;

	double   m_dTsInteFk;
	IplImage *m_pInteFkRoadImg;

	double   m_dTsContour;
	CvSeq    *m_pContourSeq;
	CvMemStorage *m_pMemStorage;	

#ifdef DEBUG_BIGVEHICLE_TOP
	IplImage *m_pShowErodeFkContoursImg;
#endif

}BVTopAdjudicator;



#endif