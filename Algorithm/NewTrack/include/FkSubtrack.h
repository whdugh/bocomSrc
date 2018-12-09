//前景抠取
#ifndef _AN_FK_SUBTRACK_H_
#define _AN_FK_SUBTRACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "comHeader.h"  //放最后

using namespace MV_IMGPRO_UTILITY;

class CFkSubtrack
{
public:
	CFkSubtrack(void);
	~CFkSubtrack(void);

	int  **m_nRateMap;   //0-255的比率值(值为50相当于1.0)

	bool m_bBankPeoDet;  //是否为银行行人检测
public:		
	//--利用彩色背景模型来获取当前图的前景
	void  mvGetFkImgWithRGBBgModel( 
					bool bEasyExtModel,	    //是否采用前景易提取模式
					int  nNowIllum,			//当前图像的光照度
					int  nBgIllum,			//背景图像的光照度
					IplImage *pVibeFkImg,	//vibe前景图
					IplImage *pRImg,		//当前的R通道的图像
					IplImage *pGImg,		//当前的G通道的图像
					IplImage *pBImg,		//当前的B通道的图像
					IplImage *pRBgM,		//背景的R通道的图像
					IplImage *pGBgM,		//背景的G通道的图像
					IplImage *pBBgM,		//背景的B通道的图像
					IplImage *pFkImg,		//输出:获取的前景图像
					int nAvgR = -100,		//R通道的平均亮度
					int nAvgG = -100,		//G通道的平均亮度
					int nAvgB = -100		//B通道的平均亮度
				) ;

	//--获取两彩色图各通道中像素点的比率
	void  mvGetTwoRGBImgRate( 
					IplImage *pRImg, 
					IplImage *pGImg, 
					IplImage *pBImg,
					IplImage *pRBgM, 
					IplImage *pGBgM,
					IplImage *pBBgM, 
					IplImage *pRRateI,
					IplImage *pGRateI, 
					IplImage *pBRateI,
					IplImage *pMaskImg
				);

private:
	//--获取当前图和背景图(彩色图)的较为敏感的差异点
	void  mvGetSensDiffImgWithBgImg_RGB( 
				    IplImage *pVibeFkImg, //vibe前景图
					IplImage *pRImg,
					IplImage *pGImg,
					IplImage *pBImg,
					IplImage *pRBgM, 
					IplImage *pGBgM, 
					IplImage *pBBgM, 
					IplImage *pSenDiffImg,
					float fTh = 0.1f, 
					int   nTh = 5 
				) ;

	//--获取当前图和背景的差异较大处(彩色图)
	void  mvGetDiffImgWithBgImg_RGB( 
					IplImage *pDiffImg,   //差异图(输出)
					IplImage *pVibeFkImg, //vibe前景图
					IplImage *pRImg, 
					IplImage *pGImg,
					IplImage *pBImg,
					IplImage *pRBgM, 
					IplImage *pGBgM,
					IplImage *pBBgM, 
					float fBg2NowR,
					float fBg2NowG,
					float fBg2NowB,
					float fAbsDiffRateTh = -10.0f, 
					int nAbsDiffValTh = -100
				) ;

	//--获取当前图和背景的差异处
	void  mvGetDiffImgWithBgImg(
					bool bDay, 
					IplImage *pGrayImg, 
					IplImage *pBgImg,
					IplImage *pTempBgImg, 
					IplImage *pTempImg1, 
					IplImage *pTempImg2, 
					IplImage *pDiffImg 
				) ;	
};

#endif