// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#ifndef __MV_SURF_H
#define __MV_SURF_H
#pragma once

#include <cv.h>
#include <vector>
#include "fasthessian.h"

using namespace std;


//-------------------------------------------------------

class MvSurf
{
public:
	MvSurf(void);
	~MvSurf(void);

	float mvGetMatchPoints(IplImage *pImg1,IplImage *pImg2,CvRect rtROI,IpPairVec &matches);
	float mvGetMatchPoints(IplImage *pImg1,IplImage *pImg2,CvRect rtROI,IpPairVec &matches,IpVec &ipts1,IpVec &ipts2);
	float mvGetROIMatchPoints(IplImage *pImg1,IplImage *pImg2,CvRect rtROI,IpPairVec &matches,IpVec &ipts1,IpVec &ipts2);
	
	float mvGetMatchPoints(IplImage *pImg1, IplImage *pImg2, vector<CvRect> vecROI, vector<IpPairVec> &matches,const float fCoff=1.0f);
	float mvGetEXMatchPoints(IplImage *pImg1, IplImage *pImg2, vector<CvRect> vecROI, vector<IpPairVec> &matches,
		const float fCoff1=1.0f,const float fCoff2=1.0f);
	float mvGetMatchPoints(IplImage *pImg1, IplImage *pImg2, vector<CvRect> vecROI, vector<IpPairVec> &matches,
		vector<CvRect>vecROIExt,vector<IpPairVec>&matchesExt, const float fCoff=1.0f);
	/*
 *参数说明：pImg1, pImg2分别为匹配图和待匹配图；
 *          vecROI1, vecROI2分别对应pImg1, pImg2两张图上的匹配区域，其坐标为相对缩放前的大图；
 *          fCoff1, fCoff2分别为pImg1, pImg2各自的缩放比例；
 *          match为返回的匹配点列，其坐标为相对缩放前的大图。
 */
float mvGetMatchPoints(IplImage *pImg1, IplImage *pImg2, vector<CvRect> vecROI1, 
	vector<CvRect> vecROI2, vector<IpPairVec> &matches, const float fCoff1 = 1.0f, const float fCoff2 = 1.0f);
	
	//找到特征点
	void mvDetDes(IplImage *img,  /* image to find Ipoints in */
		std::vector<Ipoint> &ipts, /* reference to vector of Ipoints */
		bool upright = false, /* run in rotation invariant mode? */
		int octaves = OCTAVES, /* number of octaves to calculate */
		int intervals = INTERVALS, /* number of intervals per octave */
		int init_sample = INIT_SAMPLE, /* initial sampling step */
		float thres = THRES /* blob response threshold */);
	void mvGetMatches(IpVec &ipts1, IpVec &ipts2, IpPairVec &matches);//进行匹配

private:
	void mvGetROIMatches(IpVec &ipts1, IpVec &ipts2, IpPairVec &matches,CvRect rtROI);
	void mvGetROIMatches(IpVec &ipts1, IpVec &ipts2, IpPairVec &matches,CvRect rtROI1,
		CvRect rtROI2);

	IplImage *getGray(const IplImage *img);
	IplImage *Integral(IplImage *source);

};

#endif