// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#ifndef __MVUTILITY_H
#define __MVUTILITY_H

#include <vector>
#include <list>

#include "road_detect.h"
#include "comHeader.h"  //放最后

using namespace std;
using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;


typedef struct mvCircle
{
	CvPoint ptCenter;
	float fRadius; 
}mvCircle;



class MvUtility  
{
public:
	MvUtility();
	virtual ~MvUtility();

public:
	//获取得到Linux下当前的系统时间
	static void mvget_nowDateTimeOnLinuX(char *cDateTimeStr);

	//自动创建文件目录和文件名称
	static void  mvget_wantSaveFileDirAndName( long &nUseTime,
		   char cTopDir[50], char cType[5], char infoStr[100], 
		   long &nYear, long &nMon, long &nDay, long &nHour, 
		   long &nMin, long &nSec, long index );

	inline static float mvmy_velocity(CvPoint2D32f &pt1,CvPoint2D32f &pt2)
	{ 
		return sqrt( (pt2.x-pt1.x)*(pt2.x-pt1.x) + (pt2.y-pt1.y)*(pt2.y-pt1.y) ); 
	} 
	
	//以下函数判断点及线段与矩形的包围关系
	static double mvLenLap(double x1, double x2, double x3, double x4);	//寻找四个点中中间两个点的之间的距离

	static double  mvgetAngleForTwoDirLine( double a1, double a2 );

	static float mvcalculate_polygons_overlap_ratio(short npts1, CvPoint2D32f *pts1, short npts2, CvPoint2D32f *pts2);
	//
	static float mvGetLocalGrayEntropy(const IplImage *pGrayImg,const CvRect rtROI);

	static bool mvIsLongShape(CvPoint2D32f * pRegion,int nPointsNum,  float fWHThres);
	
	static bool mvcalc_histogram(IplImage *src, CvPoint ltPt, CvPoint rbPt, int bins, float *hist, float &fAvgGray );
	static bool mvIs2HistAlike( int nBin, int nMaxShift, float fAHist1[], float fAHist2[] );

	//获取到图与背景的差异积分  
	static void mvIntegralOf2ImgDiff( IplImage *pImg, IplImage *p2VBgImg, int nThres, IplImage *pIntegrateDiffImg );

	//利用点的距离将点进行分类
	static int mvClassPoints(int nPtNum, CvPoint *p_APt, CvPoint distPt, int *p_nAClassNo);

	//从分类结果中选择出数量最多的一类
	static void mvChooseMaxPtNumClass(int nPtNum, int *p_nAClassNo, int &nMaxClassNo, int &nMaxClassPtNum);

#ifdef LINUX
	static	void mvget_time_onLinux(int &nDay, int &nHour, int &nMin, int &nSec);
#endif

	//判断两图像差异变量是否相差较大
	static bool mvis_different_twoImgdiffPtVar(float fMin, float fMax);

	//判断两速度变量是否相差较大
	static bool mvis_different_twoVeloVar(float fMin, float fMax, float fRateThres );

	static int mvGetMinSize(const int nXSize,const int nYSize);

};


//图像处理公用类
class MvUtilityImgP  
{
public:
	MvUtilityImgP();
	virtual ~MvUtilityImgP();

public:
	//计算图像的平均亮度
	static bool mvCalImgAvgLight(IplImage *grayImg, 
		IplImage *maskImg, int &nAvgLight );
	
	//计算图像中亮度大于预值的点的个数
	static int mvGetImgNumOfLightThanThres(IplImage *grayImg, 
		IplImage *maskImg, int nThres );
	
	//获取当前灰度图的水平sobel结果
	static bool mvGetHSobel( IplImage *srcImg, IplImage *dstImg );

	//获取当前灰度图的水平sobel结果
	static bool mvGetVSobel( IplImage *srcImg, IplImage *dstImg );

	//获取当前灰度图的sobel结果
	static bool mvGetSobelResult( IplImage *grayImg, 
		IplImage *pVSobeImg, IplImage *pHSobeImg );

	//获取当前灰度图的sobel结果和细化结果
	static bool mvGetThinHSobel( IplImage *grayImg, 
		IplImage *pHSobeImg, IplImage *pThinHSobeImg );

	//获取当前灰度图的ipp的sobel结果
	static bool mvGetIppSobelResult( IplImage *pSrcImg, 
		IplImage *pVSobeImg, IplImage *pHSobeImg );

	//获取得到3种sobel结果的sobel结果
	static void mvGet3SobelResult(IplImage *pSrcImg, 
		IplImage *pSobelImg, int nThres1, IplImage *p2VSobelImg1,
		int nThres2, IplImage *p2VSobelImg2);

	//calculate the horizontal gradient image of given image
	static void mvGetCandiateHorizontalGradient( IplImage* img,
					IplImage *pHoriGradImg ) ;

	//判断两图的边缘是否相同
	static bool mvIsSameOf2ImgEdge( int nGradThres, IplImage *pImg, 
		            IplImage *pRefImg, IplImage *pSameEdgeImg ) ;
	//判断两边缘图是否相同
	static bool mvIsSameOf2EdgeImg( IplImage *pGradImg,IplImage *pGradRefImg );

	//线段Hough变换
	static void mvLineHoughTransform( IplImage *pImg, int nMod );

	//judge if exist road surface under given line
	static bool mvIsBkImgUpOrDownGivenLine( IplImage* pFkImg, 
				  CvPoint2D32f linePt1, CvPoint2D32f linePt2, 
		          int nPtCnt, int nFindLen, int nFkCntThres,
				  int nContinuousThres, bool bDownMod );

	//judge if exist road surface under given line
	static bool mvIsShdowNearGivenLine( IplImage* pFkImg,
					IplImage* pBkImg, IplImage* pGrayImg,
					CvPoint2D32f linePt1, CvPoint2D32f linePt2,
					int nPtCnt, int nFindLen, int &nShadowLineMod,
				    IplImage *pDownDrawImg=NULL,
					IplImage *pUpDrawImg=NULL );

};

#endif 
