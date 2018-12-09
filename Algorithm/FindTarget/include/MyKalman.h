// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   15:05
	filename: 	e:\BocomProjects\find_target_lib\include\MyKalman.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MyKalman
	file ext:	h
	author:		Durong
	
	purpose:	没有使用
*********************************************************************/


#ifndef MV_MY_KALMAN_H

#define MV_MY_KALMAN_H



#include "cxcore.h"
#include "cv.h"
#include "highgui.h"

class MyKalman
{
private:

	CvRandState rng;
	CvKalman* kalman;
	CvMat* x_k;//状态
	CvMat* w_k;//过程噪声
	CvMat* z_k;//观测值

public:
	
	MyKalman();
	~MyKalman();

	CvPoint2D32f Predict(float dt);

	void Correct(float dt, CvPoint2D32f measur);

};



#endif