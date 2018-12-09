
//线段提取的应用
#ifndef _AN_LINES_EXT_APP_H_
#define _AN_LINES_EXT_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "lsd.h"
#include "comHeader.h"  //放最后

using namespace MV_IMGPRO_UTILITY;


#ifndef SIMLINEAPPMOD
	#define SIMLINEAPPMOD
	enum SimLineAppMod
	{
		SIM_LINE_APP_NORMAL = 0, //正常线段
		SIM_LINE_APP_NOIN_MASK,  //不在mask区域
		SIM_LINE_APP_MAYBE_BG    //可能为背景线 
	};
#endif

//简单的线段应用结构
typedef struct _SimLineAppStr
{
	CvPoint2D32f pt1;
	CvPoint2D32f pt2;
	int nLineMod;

	_SimLineAppStr( )
	{
		nLineMod = 0;
	}
}SimLineAppStr;


//--------class CLinesExtractApplication------//
class CLinesExtractApplication
{
public:
	CLinesExtractApplication(void);
	~CLinesExtractApplication(void);

	void mvInitLinesExtApp( );
	void mvUninitLinesExtApp(  );

public:
	//在给定区域内进行LSD线段提取(在给定ROI内对图像进行了Resize，
	//    给出的结果是在给定区域Resize的结果)。
	bool mvExtractLines(
			IplImage *srcImg,					  //给定图像
			CvRect srcLineRect,					  //给定提取区域
			vector<CvPoint2D32f> &vctLinePt1,	  //线段端点1
			vector<CvPoint2D32f> &vctLinePt2,	  //线段端点2
			vector<CvPoint2D32f> &vctSrcLinePt1,  //在原图的端点1 
			vector<CvPoint2D32f> &vctSrcLinePt2,  //在原图的端点2
			double &dExt2Src,       //提取的线段图和给定图的比率
			float fResizeArea = -1  //用来提线段的面积
		);


	//提取得到有效的线段(排除背景线段,选取mask图中的线段)
	bool mvExtractVaildLines(
			IplImage *pSrcImg,				 //当前图像	
			CvRect   rectExt,				 //给定的线段提取区域
			CvSize   szRectImg,				 //提取区域所对应的图大小
			vector<SimLineAppStr> &vecExtLine,  //提取出的线段
			float    fExtArea = 10000.0f,	 //线段提取的最终面积
			IplImage *pSobelBg2VImg = NULL,  //sobel背景二值图
			IplImage *pGrayBgImg = NULL,     //当前的灰度背景图		
			IplImage *pSaveMaskImg  = NULL   //可保留的mask图像
		);
	
public:
	//利用前景图像来过滤掉背景线段
	static bool mvFiltrateBgLinesWithFkImg( int &nLineCnt, 
		 CvPoint2D32f *pALinePt1, CvPoint2D32f *pALinePt2,
		 IplImage *pRSFkImg );
};


#endif