/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：StabilizeImage.h
* 摘要: 基于局部区域SIFT匹配的稳像
* 版本: V1.1
* 作者: 鲍政
* 完成日期: 2010年01月19日
*/

#ifndef __VIDEOSTABILIZER_H
#define __VIDEOSTABILIZER_H

#ifdef LINUX
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#define _chdir(a) chdir(a)
	#define _mkdir(a) mkdir(a, 0777)
#else
	#include  <io.h>
	#include <direct.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
//#include <ctype.h>
#include <time.h>
//#include <sys/utime.h>
#include <algorithm> //for std::sort

#ifndef __CV__H__
	#include "cv.h"
	#include "highgui.h"
	#include "cxcore.h"
#endif

#ifndef __SIFT__H__
	#include "sift_descr.h"
#endif


// 仿射模型的类型
#define TRANSLATION_MODEL      0
#define AFFINE_MODEL                   1
#define SIMPLE_AFFINE_MODEL    2

// for sift matching
#ifndef SIFT_DIMS
	#define SIFT_DIMS     128 
#endif
#ifndef SIFT_WIN_SIZE
	#define SIFT_WIN_SIZE    4
#endif
#ifndef SIFT_BINS
	#define SIFT_BINS     8
#endif

#ifndef MAX_CORNERS                       //最多角点数目
	#define MAX_CORNERS 400  
#endif
#ifndef MAX_REGIONS
	#define MAX_REGIONS   10              //最大画图区域个数
#endif


#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

typedef struct AffParams
{
	float theta; // 旋转角度
	float scale; // 缩放尺度
	float dx;    // x方向平移量
    float dy;    // y方向平移量
}AffParams;

typedef struct Corners 
{
	CvPoint2D32f pos;      // 位置坐标
	uchar *siftVal;             // SIFT描述符
	int isMatched;              // SIFT匹配是否成功
	int bkgrdTimes;           // 作为背景角点匹配成功次数，在统计、更新背景点时使用
    
	Corners()
	{
		pos        = cvPoint2D32f(0, 0);
		siftVal    = NULL;
		isMatched  = 0;
		bkgrdTimes = 0;
	}

}Corners;


class CVideoStabilizer4Client
{
	
public:
	CVideoStabilizer4Client();
	~CVideoStabilizer4Client(); 

	//初始化，包括块匹配区域生成，利用n帧图像初始背景点统计，返回多边形外接矩形、属于多边形内的角点位置、SIFT值及角点个数
	bool mvInitial( IplImage *pFrame, int nAreaNum, const int *nPolySize, const CvPoint2D32f **fPolyPts, 
								 int nRadius, uchar AffModel, int nFrameNum, float fCornerQuality, int nCornerMinDist );

	// 背景更新，新出现的角点是否添加入参考角点中
	bool mvUpdateBkCorners( void );

	// 参考背景点SIFT更新
	bool mvUpdateSIFT( void );

	// SIFT特征匹配包括sift特征更新
	void mvSIFTMatch( void );
//	int mvNewSIFTMatch( void );

	// 获取图像角点及SIFT值
    void mvGetRefCornersSIFT( IplImage *pFrame, float fCornerQuality, int nCornerMinDist,
										const int *nPolySize, const CvPoint2D32f **fPolyPts );

	// 获取图像角点及SIFT值
    void mvGetCurCornersSIFT( IplImage *pFrame, float fCornerQuality, int nCornerMinDist,
		                                const int *nPolySize, const CvPoint2D32f **fPolyPts );

	// 获得块匹配矩形区域,用于在外部程序中显示结果
	CvRect mvGetMaskROI(void); 

	// Ransac二次匹配
	AffParams mvRANSAC(
				   int nSelectNum, // 每次随机选几个数
				   int nNum, // ransac样本点数
				   float fThrd, // 阈值
				   int nMinMatchNum, // 最小匹配数
				   uchar uAffModel   // 仿射模型类型
				   );
    // 根据整型的偏移量纠正原图像，获得平移后的图像
	void mvTranslation( const IplImage *src, IplImage *dst, int dx, int dy );
	// 根据运动量纠正图像,用于在外部程序中显示结果
	void mvStabilized( const IplImage *pFrame, IplImage *pStbFrame );

    
    //判断点是否在多边形内, 第二个参数设为const与原函数不同
	bool mvIsPointInPolygon(int nPolySize, const CvPoint2D32f *pts, CvPoint2D32f pt);
    
	// 获取计算出的仿射参数
	AffParams mvGetAffParams( void );

	// 获取计算出的参考角点位置
	Corners* mvGetRefCorners( void );

	// 获取计算出的当前角点位置
	Corners* mvGetCurCorners( void );

	// 保存结果，存图函数
	void mvSaveTestImage( char cTopDir[50], char cType[10], char *infoStr, long &nImgCount );

	// 按照ROI矩形截图
	void mvCutImage( const IplImage* src, IplImage* dst, CvRect rect );

	// 计算2幅图像的峰值信噪比PSNR
	float mvPSNR( const IplImage* pTestImg, const IplImage* pSrcImg );
	float mvMSE( const IplImage* pTestImg, const IplImage* pSrcImg );

	// Function radnLocs() generates unique numbers.
    void mvRandLocs( int nSelectNum, int nRange, int *pLocs );

	// 对相应的点进行仿射变换
    void mvAffTform( const CvPoint2D32f* src, int numSrc, CvPoint2D32f* dst, AffParams affParams, uchar mode );

	// Function DLT calculates the transformation matrix that best describes
	// the geometric transformation between the points.
	// Note: numPts seems the same as selecNum, maybe one of those params can be removed 
	AffParams mvXform( const CvPoint2D32f* Pts1, // current frame
		               const CvPoint2D32f* Pts2, // reference frame 
	   	               int           numPts,
			           uchar         mode );

//  接口函数
//  IplImage *imgSrc  	    	-- 源图像
//  int nAreaNum   	         	-- 图像中选定用作稳像区域的个数
//  int *nPolySize              -- 用于稳像的区域的顶点数目
//  CvPoint2D32f **fPolyPts     -- 用于稳像的区域的顶点坐标
//  uchar uAffModel             -- 仿射模型的类型：
//	                                   0 -- 仅平移模型
//	                                   1 -- 简化仿射模型
//	                                   2 -- 标准仿射模型
//  AffParams &params           -- 仿射参数
//  int nMaxStbRadius           -- 最大稳像半径（user define），默认7
    int mvGetImgOffSet4Client( const IplImage     *pFrame, 
		                 int                nAreaNum, 
						 const int          *nPolySize,
						 const CvPoint2D32f **fPolyPts, 
						 uchar              uAffModel, 
						 AffParams          &params, 
		                 int                nMaxStbRadius  = 12,
						 float              fCornerQuality = 0.05f );

    // 重新初始化背景点
    bool mvReInitialBkGrd( IplImage *pFrame, float fCornerQuality,
										const int *nPolySize, const CvPoint2D32f **fPolyPts );

private:

	// SIFT局部匹配半径，与相机抖动幅度，即最大偏移量相关
	int m_nRadius;
	bool m_bUpdateCorners; // 更新背景点?

	// sift类对象，用以sift特征提取及匹配
    MySIFT m_SIFT;
		
	CvRect m_nRectMask[MAX_REGIONS]; // 人工指定多边形区域的外接矩形（mask区域，即为块匹配区域），用于生成子图

#ifdef DEBUG_CORNERMAP
	IplImage *m_pNewCorMap;// 新点出现次数统计图
#endif

	Corners *m_pRefCorners, *m_pCurCorners, *m_pNewCorners;// [2*MAX_CORNERS]

	long m_nRefCorNum, m_nCurCorNum, m_nSIFTMatchNum, m_nRANSACMatchNum , 
		 m_nBeforeImgCount, m_nAfterImgCount, m_nFrameCout, m_nAreaNum, m_nUpdateNum, m_nNewCorNum;

	CvPoint2D32f m_fCurSiftMatchPts[MAX_CORNERS], 
						  m_fRefSiftMatchPts[MAX_CORNERS],
						  m_fCurRANSACMatchPts[MAX_CORNERS], 
		                  m_fRefRANSACMatchPts[MAX_CORNERS] ;

	CvSize m_nImgSize;

	AffParams m_fAppParams;

#ifdef DEBUG_PSNR
	IplImage *m_pStbImg;
    bool m_bIsStbImgSaved;
#endif

	uchar m_uAffModel;
};

#endif