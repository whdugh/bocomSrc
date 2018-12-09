// ImgPro.h: interface for the CImgPro class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AN_IMGPEO_H_
#define _AN_IMGPEO_H_

#include "libHeader.h"
#include <stdio.h>

#include "lsd.h"

#include "sift_descr.h"

#include "comHeader.h"  //放最后

using namespace MV_IMGPRO_UTILITY;


typedef struct _imgProImgSet
{
	IplImage *pDiff256VImg;      //帧差图像
	IplImage *pDiff2VImg;        //帧差2值图像
	IplImage *pLastDiff2VImg;    //上一帧的帧差2值图像
	IplImage *pMontion2VImg;	 //当前帧的运动2值图像
	IplImage *pLastMontion2VImg; //上一帧的运动2值图像

	bool	 bExistFk2VImg;      //是否存在两值化前景
	IplImage *pFk2VImg;          //前景图像
	IplImage *pFk2VImg4Last;	 //上帧图像的2值前景图

	IplImage *pFkLabelImg;		 //前景块label值图

	IplImage *pGrad256Img;       //梯度图像
	IplImage *pGrad256Img4Last;  //上帧图像的梯度能量图像
	IplImage *pGrad2VImg;        //2值梯度图像

	IplImage *pHSobel256Img;     //水平sobel图像
	IplImage *pHSobel2VImg;      //2值水平sobel图像
	
	IplImage *pVSobel256Img;     //竖直sobel图像
	IplImage *pVSobel2VImg;      //2值竖直sobel图像

	bool     bExistBgImg;		 //是否存在背景
	IplImage *pBgImg;            //背景图像

	IplImage *pBgGrad256Img;     //背景256值梯度图像
	IplImage *pBgGrad2VImg;      //背景的2值梯度图像
	IplImage *pGrad2VDiffBgImg;  //与背景之差后的2值梯度图像

	IplImage *pBgHSobel256Img;     //背景的256值水平sobel图像
	IplImage *pBgHSobel2VImg;      //背景的2值水平sobel图像
	IplImage *pHSobel2VDiffBgImg;  //与背景之差后的2值的水平sobel图像

	IplImage *pBgVSobel256Img;     //背景的256值竖直sobel图像  
	IplImage *pBgVSobel2VImg;      //背景的2值竖直sobel图像       
	IplImage *pVSobel2VDiffBgImg;  //与背景之差后的2值的竖直sobel图像

	_imgProImgSet( )
	{
		initVar( );
	}

	void initVar( )
	{
		pDiff256VImg = NULL;
		pDiff2VImg   = NULL;
		pLastDiff2VImg = NULL;

		pMontion2VImg = NULL;
		pLastMontion2VImg = NULL;

		bExistFk2VImg = false;
		pFk2VImg = NULL;
		pFk2VImg4Last = NULL;

		pFkLabelImg = NULL;

		pGrad256Img = NULL;
		pGrad256Img4Last = NULL;
		pGrad2VImg = NULL;

		pHSobel256Img = NULL;
		pHSobel2VImg = NULL;

		pVSobel256Img = NULL;
		pVSobel2VImg = NULL;

		bExistBgImg = false;
		pBgImg = NULL;

		pBgGrad256Img = NULL;
		pBgGrad2VImg = NULL;
		pGrad2VDiffBgImg = NULL;

		pBgHSobel256Img = NULL;
		pBgHSobel2VImg = NULL;      
		pHSobel2VDiffBgImg = NULL;  		

		pBgVSobel256Img = NULL;
		pBgVSobel2VImg = NULL;      
		pVSobel2VDiffBgImg = NULL;  	
	}

	void createImages( CvSize sz )
	{
		pDiff256VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		cvSetZero( pDiff256VImg );
		pDiff2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		cvSetZero( pDiff2VImg );
		pLastDiff2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		cvSetZero( pLastDiff2VImg );

		pMontion2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		cvSetZero( pMontion2VImg );
		pLastMontion2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		cvSetZero( pLastMontion2VImg );
		
		bExistFk2VImg = false;
		pFk2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		pFk2VImg4Last = cvCreateImage( sz, IPL_DEPTH_8U, 1 );

		pFkLabelImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );

		pGrad256Img = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		pGrad256Img4Last = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		pGrad2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );

		pHSobel256Img = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		pHSobel2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );

		pVSobel256Img = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		pVSobel2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );

		bExistBgImg = false;
		pBgImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );

		pBgGrad256Img = cvCreateImage( sz, IPL_DEPTH_8U, 1 );	
		pBgGrad2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );	
		pGrad2VDiffBgImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		cvSetZero(pGrad2VDiffBgImg);

		pBgHSobel256Img = cvCreateImage( sz, IPL_DEPTH_8U, 1 );	 
		pBgHSobel2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );	 
		pHSobel2VDiffBgImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );
		cvSetZero(pHSobel2VDiffBgImg);

		pBgVSobel256Img = cvCreateImage( sz, IPL_DEPTH_8U, 1 );	 
		pBgVSobel2VImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );	 
		pVSobel2VDiffBgImg = cvCreateImage( sz, IPL_DEPTH_8U, 1 );	
		cvSetZero(pVSobel2VDiffBgImg);
	}

	void releaseImgSet( )
	{
		if( NULL != pDiff256VImg )
		{
			cvReleaseImage(&pDiff256VImg);
			pDiff256VImg = NULL;
		}
		if( NULL != pDiff2VImg )
		{
			cvReleaseImage(&pDiff2VImg);
			pDiff2VImg   = NULL;
		}
		if( NULL != pLastDiff2VImg )
		{
			cvReleaseImage(&pLastDiff2VImg);
			pLastDiff2VImg = NULL;
		}

		if( NULL != pMontion2VImg )
		{
			cvReleaseImage(&pMontion2VImg);
			pMontion2VImg = NULL;
		}
		if( NULL != pLastMontion2VImg )
		{
			cvReleaseImage(&pLastMontion2VImg);
			pLastMontion2VImg = NULL;
		}

		if( NULL != pFk2VImg )
		{
			cvReleaseImage(&pFk2VImg);
			pFk2VImg = NULL;
		}
		if( NULL != pFk2VImg4Last )
		{
			cvReleaseImage(&pFk2VImg4Last);
			pFk2VImg4Last = NULL;
		}

		if( NULL != pFkLabelImg )
		{
			cvReleaseImage(&pFkLabelImg);
			pFkLabelImg = NULL;
		}

		if( NULL != pGrad256Img )
		{
			cvReleaseImage(&pGrad256Img);
			pGrad256Img = NULL;
		}
		if( NULL != pGrad256Img4Last )
		{
			cvReleaseImage(&pGrad256Img4Last);
			pGrad256Img4Last = NULL;
		}
		if( NULL != pGrad2VImg )
		{
			cvReleaseImage(&pGrad2VImg);
			pGrad2VImg = NULL;
		}

		if( NULL != pHSobel256Img )
		{
			cvReleaseImage(&pHSobel256Img);
			pHSobel256Img = NULL;
		}
		if( NULL != pHSobel2VImg )
		{
			cvReleaseImage(&pHSobel2VImg);
			pHSobel2VImg = NULL;
		}

		if( NULL != pVSobel256Img )
		{
			cvReleaseImage(&pVSobel256Img);
			pVSobel256Img = NULL;
		}
		if( NULL != pVSobel2VImg )
		{
			cvReleaseImage(&pVSobel2VImg);
			pVSobel2VImg = NULL;
		}

		if( NULL != pBgImg )
		{
			cvReleaseImage(&pBgImg);
			pBgImg = NULL;
		}

		if( NULL != pBgGrad256Img )
		{
			cvReleaseImage(&pBgGrad256Img);
			pBgGrad256Img = NULL;
		}
		if( NULL != pBgGrad2VImg )
		{
			cvReleaseImage(&pBgGrad2VImg);
			pBgGrad2VImg = NULL;
		}
		if( NULL != pGrad2VDiffBgImg )
		{
			cvReleaseImage(&pGrad2VDiffBgImg);
			pGrad2VDiffBgImg = NULL;
		}

		if( NULL != pBgHSobel256Img )
		{
			cvReleaseImage(&pBgHSobel256Img);		
			pBgHSobel256Img = NULL;  
		}
		if( NULL != pBgHSobel2VImg )
		{
			cvReleaseImage(&pBgHSobel2VImg);		
			pBgHSobel2VImg = NULL;  
		}
		if( NULL != pHSobel2VDiffBgImg )
		{
			cvReleaseImage(&pHSobel2VDiffBgImg);
			pHSobel2VDiffBgImg = NULL;  
		}

		if( NULL != pBgVSobel256Img )
		{
			cvReleaseImage(&pBgVSobel256Img);		
			pBgVSobel256Img = NULL;  
		}
		if( NULL != pBgVSobel2VImg )
		{
			cvReleaseImage(&pBgVSobel2VImg);		
			pBgVSobel2VImg = NULL;  
		}
		if( NULL != pVSobel2VDiffBgImg )
		{
			cvReleaseImage(&pVSobel2VDiffBgImg);
			pVSobel2VDiffBgImg = NULL;  
		}
	}

}ImgProImgSet;


typedef struct _imgProIntegrateImgSet
{
	IplImage *pIntGrayImg; 
	IplImage *pIntBgGrayImg;

	IplImage *pIntDiff2VImg; 
	IplImage *pIntFore2VImg;

	IplImage *pIntGrad2VImg;
	IplImage *pIntBgGrad2VImg;
	IplImage *pIntGrad2VDiffBgImg;  

	IplImage *pIntHSobel2VDiffBgImg;
	IplImage *pIntVSobel2VDiffBgImg;
	_imgProIntegrateImgSet( )
	{
		initVar( );
	};

	void initVar( )
	{
		pIntGrayImg = NULL;
		pIntBgGrayImg = NULL;

		pIntDiff2VImg = NULL;
		pIntFore2VImg = NULL;

		pIntGrad2VImg = NULL;
		pIntBgGrad2VImg = NULL;
		pIntGrad2VDiffBgImg = NULL;  

		pIntHSobel2VDiffBgImg = NULL;
		pIntVSobel2VDiffBgImg = NULL;
	}

	void createImages( CvSize szInte )
	{
		pIntGrayImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );
		pIntBgGrayImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );

		pIntDiff2VImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );
		pIntFore2VImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );

		pIntGrad2VImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );
		pIntBgGrad2VImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );
		pIntGrad2VDiffBgImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );

		pIntHSobel2VDiffBgImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );	
		pIntVSobel2VDiffBgImg = cvCreateImage( szInte, IPL_DEPTH_32S, 1 );	
	}

	void releaseImgSet( )
	{
		if( NULL!= pIntGrayImg )
		{
			cvReleaseImage(&pIntGrayImg);
			pIntGrayImg = NULL;
		}
		if( NULL!= pIntBgGrayImg )
		{
			cvReleaseImage(&pIntBgGrayImg);
			pIntBgGrayImg = NULL;
		}

		if( NULL!= pIntDiff2VImg )
		{
			cvReleaseImage(&pIntDiff2VImg);
			pIntDiff2VImg = NULL;
		}
		if( NULL!= pIntFore2VImg )
		{
			cvReleaseImage(&pIntFore2VImg );
			pIntFore2VImg  = NULL;
		}	

		if( NULL!= pIntGrad2VImg )
		{
			cvReleaseImage(&pIntGrad2VImg );
			pIntGrad2VImg  = NULL;
		}	
		if( NULL!= pIntBgGrad2VImg )
		{
			cvReleaseImage(&pIntBgGrad2VImg );
			pIntBgGrad2VImg = NULL;
		}	
		if( NULL!= pIntGrad2VDiffBgImg )
		{
			cvReleaseImage(&pIntGrad2VDiffBgImg );
			pIntGrad2VDiffBgImg = NULL;
		}	

		if( NULL!= pIntHSobel2VDiffBgImg )
		{
			cvReleaseImage(&pIntHSobel2VDiffBgImg );
			pIntHSobel2VDiffBgImg = NULL;
		}	
		if( NULL != pIntVSobel2VDiffBgImg )
		{
			cvReleaseImage(&pIntVSobel2VDiffBgImg);
			pIntVSobel2VDiffBgImg = NULL;
		}
	}
}ImgProIntegrateImgSet;


class CImgPro  
{
public:
	bool   m_bInitImgPro;
	CvSize m_useSize;

	//线段
	unsigned int  m_nLineCnt;
	CvPoint *m_pLinePt1;
	CvPoint *m_pLinePt2;
	int m_nFrameNo;
public:
	bool mvImgInit( CvSize useSize );
	bool mvImgUnInit(  );

	CImgPro( );
	virtual ~CImgPro( );

	bool mvInitResizeCvtColor( IplImage* srcRgbImg, const CvRect roiRect );
	
	bool mvGetBgImgInfo( IplImage* bgImg ); //多帧调用一次

	//处理不需要前景和背景信息的图像(每帧都调用)
	bool mvProcessImgWithoutBgFkImg(bool bUseHardMode=false);

	//处理需要背景信息的图像(每帧都调用)
	bool mvProcessImgWithBgImg( IplImage* bgImg, bool bGetBeImg );

	//处理需要前景信息的图像(每帧都调用)
	bool mvProcessImgWithFkImg( IplImage* fk2vImg );


	//处理图像(每帧都调用)
	bool mvProcessImgforAnalysis( IplImage* fk2vImg ); 

private:
	void mvImgInitVar( );

	//光流法进行块匹配
	bool mvBlockMatchByOptiFlow( IplImage* imgPre, IplImage* imgCur );  
	bool mvBlockMatchByOptiFlow2( IplImage* imgPre, IplImage* imgCur );

	//获取得到分块信息
	bool mvGetDivdeBlockInfo( IplImage* fkImg=NULL );  

	//利用FloodFill方法进行填充
	void mvFloodFillToSegment( IplImage* imgRgbImg ); 

#ifdef	SHOW_OPTIFLOWBM
	void mvShowBlockMatchByOptiFlow( IplImage* imgPre,
			     IplImage* imgCur, int n_YB, int n_XB, 
			     CvSize block_size, IplImage *velx, 
			     IplImage *vely );
#endif

public:
	IplImage* m_pSrcImg;
	IplImage* m_pUseRgbImg;     //使用的彩色图像
	IplImage* m_pUseGrayImg;
	IplImage* m_pUseLastGrayImg;
	IplImage* m_pUseLast2GrayImg;


	ImgProImgSet          m_imgSet;
	ImgProIntegrateImgSet m_inteImgSet;	
	
private:
	//提取线段
	void mvExtractLine( IplImage* grayImg );

};

#endif //_AN_IMGPEO_H_
