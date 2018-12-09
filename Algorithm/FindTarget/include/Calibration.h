// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef MYCALIBRATION_H
#define MYCALIBRATION_H


//#define USE_TSAI



#include <highgui.h>
#include <cxcore.h>
#include <algorithm>

#ifdef SPEED_UP
#undef USE_TSAI
#endif

#undef USE_TSAI

#ifdef USE_TSAI
  #include "cal_main.h"
#endif

#include "StdAfx.h"







PRJ_NAMESPACE_BEGIN



/**
	类功能：相机标定类，由计算图像坐标与世界坐标的转换矩阵
	指定转换矩阵，进行图像坐标与世界坐标的互转。
*/
class MyCalibration
{
public:

	// 给定点的世界坐标，返回其图像坐标
	static CvPoint        GetImageCoordinate(const CvPoint2D64f &wordPt, const double homograhy[3][3]);

	
	
	// 给定点的图像坐标，返回其世界坐标
	static CvPoint2D64f   GetWorldCoordinate(const CvPoint &pt,const double homography[3][3]);
	static CvPoint2D64f   GetWorldCoordinate(const CvPoint2D64f &pt,const double homography[3][3]);


	// 计算图像到世界坐标的的转换举证
	static void           FindHomography(const double *image, const double *world, double homography_image_to_world[3][3]);

public:

	
	/*
	输入原始图像大小（nImageWidth， nImageHeight），在原始图像上标定点的图像坐标及其世界坐标。
	计算按dScaleX、dScaleY缩小后的图像上标定。
	*/
	MyCalibration(int nImageWidth, int nImageHeight, const double *image, const double *world, 
					int nCount, double dScaleX, double dScaleY);


	/*
	输入图像大小，在图像上标定点的位置以及其世界坐标，进行标定。
	*/
	MyCalibration(int nImageWidth, int nImageHeight, const double *image, const double *world, int nCount);


	//MyCalibration();


	// 析构函数
	~MyCalibration();


	// 给定点的图像坐标，返回其世界坐标
	CvPoint2D64f   GetWorldCoordinate(const CvPoint &pt) const;




	// 给定点在世界坐标系中的位置(x,y,z),计算点在图像中的坐标。
	CvPoint        GetImageCoordinate(const CvPoint3D64f &wordPt);


	// 给点图像上某点，并给出该点在世界坐标系中的z值，求其世界坐标。
	CvPoint3D64f   GetWorldCoordinate(const CvPoint &ptImage, float zw);


	// 给定点的世界坐标，返回其图像坐标
	CvPoint        GetImageCoordinate(const CvPoint2D64f &wordPt) const;

	// 读转换矩阵
	void           GetHomography(double homography_image_to_world[3][3]) const;



	//void SetHomography(const double homography_image_to_world[3][3]);



	// 生成一张用于测试标定是否正确的图片，存储于程序执行目录下。
	void           TestCalibration(int nWidth, int nHeight, char *strCalibName);



private:

	// 复制构造函数
	MyCalibration(const MyCalibration &calib);

	// 
	MyCalibration & operator = (const MyCalibration &calib);

private:

	//图像坐标到世界坐标的转换矩阵
	double m_homographyMatrix[3][3];

	//图像坐标到世界坐标的转换矩阵
	CvMat *m_pHomoMat;

	//世界坐标到图像坐标的转换矩阵
	CvMat *m_pHomoMatInv;


	#ifdef USE_TSAI
		cal *camTsai;
	#endif

};

PRJ_NAMESPACE_END


#endif
