// 博康智能
// Bocom Intelligent 

//本模块实现图像增强算法

#pragma once


#include "math.h"
#include <deque>
//opencv函数库的头文件
#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "cvaux.h"
#include "ml.h"
#include <vector>
#include "ipp.h"
#include <iostream>
#include <fstream>

using std::deque;
using std::vector;
using namespace std;
//定义常量
#define PI 3.1415926f //pi值
#define mvMin(x,y) x<=y? x : y
#define mvMax(x,y) x>=y? x : y

//函数定义
float ThresholdOtsu(IplImage* grayim, CvRect rect, IplImage *mask); //ostu阈值分割

//图像增强类别
class MvImgEnhance_HandOver
{
public:	
	MvImgEnhance_HandOver();
	~MvImgEnhance_HandOver();

	//图像增强的交付内容
	//接口
	void mvImgEnhance(IplImage *pImage, CvRect rectImg, bool bDay);
	
	//图像特征
	void mvWeibullParam(IplImage *image, double *gama, double *beta, IplImage *imageMask);
	void mvImgCastF();//颜色特征
	void mvImgMoment(IplImage *Pimage, CvRect *rect, CvPoint2D32f *centerNow);
	void mvLightF();//光照特征
	void mvImgColorF();//彩度特征
	void Process1(IplImage * src, IplImage* dst);//去雾
	void LocalFilter(IplImage*dst, IplImage *src,int r);
	//图像增强
	void mvImgEnhanceProcess(IplImage *pImage, CvRect rectImg);
	
	//algorithm test
	void mvImgFogRemove(IplImage *src);//DCP
	
private:
	IplImage *pImgResize;
	IplImage *Lab;//也可做hsl
	
	IplImage *pImgResizeGray;
	IplImage *pA;//可做s
	IplImage *pB;
	IplImage *pResizeMask;
	
	IplImage *pImgResizeGray32;
	
	IplImage *pImgGW32;
	//图像基本特征
	CvSize sizeOri;//图像原始大小
	CvSize sizeRe;//缩放后的大小
	//计算图像特征
	double gama;
	double beta;
	float fTh;//ostu分割阈值
	bool bCastYN;//是否偏色
	int nMethod;//白平衡方法
	CvPoint2D32f abMove;//lab平移向量
	CvScalar sGWvector;//gray world的系数
	float fGammaLUT[256];//gamma校正的LUT
	float fColorLUT[256];//彩度校正的LUT
	float fLight;
	float fFog;
	

};