// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>
#include <string>
#include <cxcore.h>
#include "OpticalFlow.h"

#ifndef PI
#define PI 3.1415926
#endif


typedef struct _rgbQuad {
	uchar rgbBlue;
	uchar rgbGreen;
	uchar rgbRed;
	uchar rgbReserved;
} RgbQuad;


typedef struct _hslQuad {
	uchar h;
	uchar s;
	uchar l;
	uchar reserved;
} HslQuad;


char* i2str(int val, char* buf);


//using namespace std;
class Utils
{
public:
	static void     FormRectsFromContours(CvSeq *contour, std::vector<CvRect> &rects,bool bSizeFil = false);
	static void     GroupRects(std::vector<CvRect> &srcRects, std::vector<CvRect> &dstRects, bool (*IsTwoRectsClose)(const CvRect &r1, const CvRect &r2));


	static void     FilterOutSmallRects(std::vector<CvRect> &rects, int width, int height,  bool orAnd = true);
	static void     FilterOutBigRects(std::vector<CvRect> &rects, int width, int height, bool orAnd = true);
	
	static void     RemoveDuplicatedRects(std::vector<CvRect> &srcRect);


	static void     GroupRectsInRects(std::vector<CvRect> &rects, std::vector<CvRect> bigRects);

	static void     GroupContourRects(CvSeq *contour, std::vector<CvRect> &bigRectContainer);
	static void     FilterOutSmallSideRects(std::vector<CvRect> &rects, int widthThreshold, int minDistance, CvRect visibleRect);
	static bool     IsLeftThan(const CvRect &r1, const CvRect &r2);
	static void     SortRects(std::vector<CvRect> &rects);
	static float    Median(std::vector<float> arr);
	static float    Median(std::vector<double> arr);
	//static void     PredictRectByFlow(CvRect &rect, Flow flow);

	static void     PrintfRect(CvRect r, char *name);

	static void     DrawRectOnImage(IplImage *img, CvRect rect, CvScalar color=CV_RGB(255, 255, 255), int nThickness = 1);

	// 垂直投影单通道图（灰度或者二值图）。
	static void     ProjectImageVert(IplImage *img, CvRect roi, int *prj);

	// 计算均值与方差
	static void		calcMean(IplImage *src, CvRect roi, double &mean, double &dev);


	static void     MeanVar(float *pData, int nSize, float &mean, float &var);


	static void     MeanVar(const std::vector<float> &vecData, float &mean, float &var);



	static bool     IsFloatAThanB(float a, float b);


	static CvRect   RectsMedian(const std::vector<CvRect> &rects);


	static void     DrawRect(IplImage *img, CvRect r, CvScalar color);

	static void     MedianFilter(int *pData, int nSize, int nWinSize);
	//自动创建单个文件夹！
	static void     AutoCreateDefinedFolder(const char *pCreateDir);
	// 将src中所有与rect相交的放入dest。
	static void     GetInteractingRects(const std::vector<CvRect> &src, CvRect rect, std::vector<CvRect> &dest);


	inline bool static IsFloatEqual(double a, double b, double limit=1e-5)
	{
		return fabs (a - b) < limit;
	}

	//交换两个点！
	inline static void   SwapTwoPoints(CvPoint &pt1, CvPoint &pt2)
	{
		CvPoint tmp = pt1;
		pt1 = pt2;
		pt2 = tmp;
	};


	static std::string GetDateTimeString();


	static std::string GetDateTimeString(unsigned int timestamp);

	static std::string GetDateTimeString( int64 ts );

	// 将时间转为时间戳，输入格式为2010-09-01 08:07:06
	// 如果格式不对，返回0
	static time_t GetTimestamp(std::string timestr);

	/* RGB to HSL transforms*/
	static void RgbToHsl(RgbQuad *rgb, HslQuad *hsl);

	// 外部释放
	static void RgbToHsl(IplImage* imgSrc, IplImage** imgDest);

	//对于图像img给定一ROI，求其平均H值！
	static bool MeanHSLValue(IplImage *img, CvRect rectRoi, HslQuad &meanHvalue);
	static bool GetRegionHValue(IplImage *img, CvRect rectRoi, int &majorHvalue);

	static inline bool ExpandRect(CvRect &rect, float fx, float fy)
	{
		CvRect ret = cvRect(rect.x - (int)(rect.width*fx), rect.y-(int)(rect.height*fy),(int)( rect.width*(1+2*fx)), (int)(rect.height*(1+2*fy)));
		if (ret.width <= 0 || ret.height <=0)
		{
			return false;
		}

		rect = ret;
		return true;
	}

	static float GetEnergy(IplImage* integray, CvRect rgn);


	static CvRect SetImageROI(IplImage* img, CvRect roi);


	static CvRect ResetImageROI(IplImage* img);

	//用图像处理判断红绿灯，暂时用于北京安防展演示！
	static int GetSignalStatusByImage(IplImage *imgRGBSrc, CvRect redlightRgn, CvRect greenlightRgn);
	static int GetSignal(IplImage *imgRGBSrc, CvRect redlightRgn, CvRect greenlightRgn, uchar &meanRed, uchar &meanGreen);

};

#endif
