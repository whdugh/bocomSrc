/* 版权所有 2013 上海博康智能信息技术有限公司
* Copyright (c) 2013，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvImgProUtility.h
* 摘要: 公用的通用图像处理函数的整理
* 版本: V1.0
* 作者: 贺岳平
* 完成日期: 2013年12月
*/ 
//////////////////////////////////////////////////////////////////////

#ifndef _MV_IMGPRO_UTILITY_H_
#define _MV_IMGPRO_UTILITY_H_

#include "ipp.h"
#include "BaseComLibHeader.h"

#ifdef LINUX
	#include "../../common/sse_def.h" 
#else 
	//#define USE_SSE_AUTO_CORRELATION
	#ifdef USE_SSE_AUTO_CORRELATION
		#include "basic_sse.h"
	#endif
#endif

#define GET_FLOAT_ELE(A, B) (*((float*)&(A) + (B)))

#ifndef INT_MAX
	#define INT_MAX   2147483647    /* maximum (signed) int value */
#endif

#ifndef RGB_HSV_CONST
	#define RGB_HSV_CONST 0.023529411765
#endif

//图像转换
typedef struct mvRGBQUAD
{
	uchar rgbBlue;
	uchar rgbGreen;
	uchar rgbRed;
	uchar rgbReserved;
} mvRGBQUAD;

//前景轮廓信息结构
#ifndef FORE_CONTOUR_STRUCT
	#define FORE_CONTOUR_STRUCT
	typedef struct _fkContourInfo
	{
		double			   dArea;		 //对应的面积
		double			   dArcLen;      //对应的弧长
		CvRect			   rect;		 //对应的rect
		int                nTopPtNo;	 //最上面的点的序号
		int                nBottomPtNo;  //最下面的点的序号
		int                nLeftPtNo;	 //最左面的点的序号
		int                nRightPtNo;	 //最右面的点的序号
		vector<CvPoint>    pts;			 //对应的点组
	}fkContourInfo;
#endif

//图像增强
#ifndef IMAGE_ENHANCE_MOD
	#define IMAGE_ENHANCE_MOD
	enum enhance_mod_enum
	{
		DEFAULT_LINEA_ENHANCE = 0,
		LINEA_ENHANCE_1,
		LINEA_ENHANCE_2,
		LINEA_ENHANCE_3
	};  
#endif

//光流块匹配
#ifndef CALC_OPTICAL_FLOWBM
	#define CALC_OPTICAL_FLOWBM
	enum
	{
		MV_SMALL_DIFF = 2,
		MV_BIG_DIFF = 32,  //128
		MV_OUTOFMEM_ERR = -3,
		MV_NULLPTR_ERR = -2
	};  
#endif

//图像大小格式
#ifndef IMAGE_SIZE_FORMAT
	#define IMAGE_SIZE_FORMAT
	enum
	{
		MV_SZ_HIGH_CLARITY = 1,  //高清  
		MV_SZ_STANDARD_FROM = 2, //标清
		MV_SZ_SMALL_FORMAT = 3   //SIF格式
	};  
#endif

//sobel提取的格式
#ifndef SOBEL_EXTRACT_MOD
	#define SOBEL_EXTRACT_MOD
	enum sobel_extract_mod_enum
	{
		EXTRACT_H_SOBEL = 0,
		EXTRACT_V_SOBEL,
		EXTRACT_M_SOBEL
	};  
#endif
	
//当前图与背景差的模式
#ifndef DIFFIMG_WITH_BG_MOD
	#define DIFFIMG_WITH_BG_MOD
	enum DiffImgWithBgMod
	{
		DIFF_WITH_OR = 0,
		DIFF_WITH_AND = 1,
		DIFF_WITH_OTHER
	};
#endif

//图像质量诊断结构体
typedef struct StruImgQualityCheck
{
public:
	StruImgQualityCheck( )
	{
		initVar( );
	}

	void initVar( );

public:
	//获取图像的格式
	static int mvGetImgFormat( const IplImage *pSrcImg );
	static int mvGetImgFormat( CvSize szSrc );

private:

}ImgQualityChecker;

namespace MV_IMGPRO_UTILITY
{
	//part 0
	//只在window下显示窗口，不在linux下显示
	void mvWinShowImg( char *p_chWinName, const IplImage *pImg );

	//获取得到和参考图像大小统一的图
	IplImage* mvGetSameSizeImg8U1C( IplImage *pImg, const CvSize &szRef );
	
	//获取得到和参考图像大小统一且平滑的图
	IplImage* mvGetSameSizeSmoothImg8U1C( IplImage *pImg, const CvSize &szRef );

	//修补场图，减少锯齿
	void mvRepairInterleavedImg( IplImage *pSrcImg, bool bInterleavedImg );

	//part 1
	//光流法进行块匹配
	void myCalcOpticalFlowBM( const void* srcarrA, const void* srcarrB,
		CvSize blockSize, CvSize shiftSize,	CvSize maxRange, int usePrevious,
		void* velarrx, void* velarry );

	int myCalcOpticalFlowBM_8u32fR( uchar * imgA, uchar * imgB,
		int imgStep, CvSize imgSize, CvSize blockSize, CvSize shiftSize,
		CvSize maxRange, int usePrev, float *velocityX, float *velocityY,
		int velStep );

	//myCalcOpticalFlowBM_8u32fR函数中用到的几个小函数
	int myAlign( int size, int align );

	void myCopyBM_8u_C1R( const uchar* src, int src_step,uchar* dst, 
						 int dst_step, CvSize size );

	int myCmpBlocksL1_8u_C1( const uchar * vec1, const uchar * vec2, int len );


	//part 2
	//前两个函数要求积分图为32S的(一般情况下，请将积分图设为32s格式)
	//获取到积分图像中一点的值 
	int mvGetValueOfIntegrateImg( const IplImage *lpInteGrateImg, CvPoint pt );

	//获取到积分图像区域的值(需将原图中的点位置加1)      
	int mvGetValueOfIntegrateImgRect( const IplImage *lpInteGrateImg,
									  CvPoint lt_pt, CvPoint rb_pt );

	//获取到积分图像区域的值(给定在原图中的rect即可)
	int mvGetValueOfIntegrateImgRect( const IplImage *lpInteGrateImg, CvRect rct );

	//以下这两个函数要求积分图为64f的(请将积分图设为64f格式)
	//获取到积分图像中一点的值 
	double mvGet64fValueOfIntegrateImg( const IplImage *lpInteGrateImg, CvPoint pt );

	//获取到积分图像区域的值      
	double mvGet64fValueOfIntegrateImgRect( const IplImage *lpInteGrateImg,
										    CvPoint lt_pt, CvPoint rb_pt );

	//计算图像的梯度
	void mvGetGradient(const IplImage* img, IplImage *pGradImg, 
			IplImage *pAngleImg, float fGradScale=40.0f, 
			float fAngleScale=28.64788f );

	//计算图像的梯度
	void mvGetGradientImg(const IplImage* img, IplImage *pGradImg,
		    IplImage *pAngleImg, float fGradScale=40.0f,
		    float fAngleScale=28.64788f );

	//计算图像的梯度
	void mvGetGridentBy4Operator( IplImage *img, 
			IplImage *pMaskImg, IplImage *pGradImg );

	//提取sobel结果
	void mvExtractSobel( IplImage* grayImg, int nMod, 
			IplImage *pSImg, IplImage *pMaskImg=NULL );

	//自适应的二值化(可用直方图来替换掉)
	IplImage* mvAdaptThreshold( IplImage *pGImg, int nInitThreshold=60,
					float fProMinVal = 0.12f, float fProBigVal = 0.15f,
					int nMaxIteraction = 6 );

	//part 3
	//对图像作对比度线性拉伸
	void mvImageAdjust_liner( IplImage* src, IplImage* dst,
							  int low, int high );
	void mvImageAdjust_liner( IplImage* src, IplImage* dst, 
						int val_1, int val_2, int val_3 );
	bool mvImageAdjust( IplImage* src, IplImage* dst, 
			  double low, double high, double bottom,
			  double top,	double gamma );

	//对图像进行线性增强 enhance the image with linear method
	bool mvEnhanceImage( IplImage* srcGrayImg,
			IplImage* dstGrayImg, int nMod=0 );


	//part4
	//获取得到与背景不同的像素点所组成的图像,其中pImg图像中道路区域之外像素值为0
	void mvGetDiffImgWithBg( const IplImage *pImg, const IplImage *bgImg,
			   int nDiffThres, int nMinDiffThres, float fThres, int nMod,
			   IplImage *pDiff2VImg, IplImage *pDiff256Img=NULL );
	//获取得到干净的2值图像
	void mvGetClean2VImg( const IplImage *pImg, IplImage *pCleanImg );
	//清理小的碎块
	void mvClearSmallContours( IplImage *p2VImg, IplImage *pCopy2VImg, 
							   int nPtThres );
	//填充小的空洞
	void mvFillSmallContours( IplImage *p2VImg, IplImage *pCopy2VImg, 
							  int nPtThres );

	//保存轮廓的信息
	void mvSaveContoursInfo( fkContourInfo &MyCtu, 
			  CvSeq *contour, double pointnum=-100.0 );

	//得到图像的外轮廓，填充内轮廓
	void mvGetAndFillContours( 
			IplImage *pFkImg, //OUT: 
			IplImage *p8ULabelImg, //OUT: 
			IplImage *pCopyFkImg, 
		    int nPtClearThres, 
			vector<fkContourInfo> &vctFkContours,
			bool bSaveAllPts, 
			int nSaveExternalContourPtTh = 100,
			float fFillRateTh=0.04f );

	//得到图像的外轮廓,根据maxsize来清理小外轮廓，填充内轮廓
	void mvGetAndFillContoursWithCarSz( 
			IplImage *pFkImg,      //OUT:
			IplImage *p8ULabelImg, //OUT: 
			IplImage *pCopyFkImg, 
			int nPtClearTh, 
			float fFillTh,	 			
			IplImage *pCarWImg,
			IplImage *pCarHImg,	
			float  fF2C_x ,	
			float  fF2C_y ,
			vector<fkContourInfo> &vctFkContours,
			double dSaveExternalContourPtTh=10000000.0 );

	//获得得到给定目标区域所对应的countour前景mask图
	IplImage* mvGetContoursMaskOfGiveRect(
			const CvRect rectObjFkArea,	//给定的目标区域
			const IplImage *pFkAllImg,	//全图区域内的前景
			const IplImage *pFkAllInteImg,	//全图区域内的前景积分
			CvRect &rectBest4Obj,       //得到的最好的目标rect
			int nContourAreaTh,			//要求的轮廓面积阈值
			IplImage *pDrawImg );       //结果绘制图
		
	//part5
	//获取两帧之间的帧间差分(256和2值)
	void mvGetAbsDiffImg(IplImage *pImgNow, IplImage *pImgLast, 
			IplImage *pMaskImg,	bool bUseHardDiffMode,
			IplImage *pDiff256Img, IplImage *pDiff2VImg);


	//part6
	//计算两图像的自相关系数
	double mvCorrNormMatch( const IplImage *pImg1, const IplImage *pImg2,
		                    const CvRect rtROI );
	double mvCorrNormMatch( const IplImage *pImg1, const IplImage *pImg1Ingr,
		                    const IplImage *pImg2, const IplImage *pImg2Ingr,
			                const CvRect rtROI );   //存在积分图来加速
#ifdef USE_SSE_AUTO_CORRELATION	
	double mvCorrNormMatch_sse( const IplImage *pImg1, const IplImage *pImg2, 
		                        const CvRect rtROI );
	double mvCorrNormMatch_sse( const IplImage *pImg1, const IplImage *pImg1Ingr,
		                        const IplImage *pImg2, const IplImage *pImg2Ingr,
						        const CvRect rtROI );  //SSE存在积分图来加速
#endif

	//part7
	//计算图像照度
	CvScalar mvCalIllumination( IplImage *pGrayImg, double dExpo=0.5,
								IplImage *pBgMaskImg=NULL );

	//part8
	//计算图像点周围的最长长度(
	void mvGetLengthOfImgPoint( IplImage *p2VImg, IplImage *pLenImg );
	IplImage* mvGetLengthOfImgPoint2( IplImage *p2VImg );

	
	//part9
	//获取到在由两点所确定的线段上的所有像素点(对图像)
	bool mvGetPixelOfTwoPoint( double P1X, double P1Y, 
			double P2X, double P2Y, int nHeight, int nWidth,
			double w, int &nPixelNum, int *nPixelYX,
			CvPoint *ptAStaEndIdx = NULL, int *pNum=NULL );

	//part10
	//利用积分图像来将一些零碎的点删除
	bool mvErode( IplImage *src2VImg, IplImage *dst2VImg, 
		   int nStepX, int nStepY, float fErodeRateThres );
	//利用积分图像来将零碎的块扩张
	bool mvDilate( IplImage *src2VImg, IplImage *dst2VImg, 
		   int nStepX, int nStepY, float fErodeRateThres );

	//对前景进行腐蚀
	void mvGetErodeFkImg( const IplImage *pFkAllImg,  //全图前景
			IplImage *pErodeFkImg,  //腐蚀后的前景
			int nErodeTime          //腐蚀次数
		);

	//part11
	//RGB到HSV颜色空间的转换
	bool mvRGB_To_HSV(int red, int green, int blue, int *h, int *s, int *v);

	//颜色空间变换:RGB到HSV
	void mvRGB2HSV(mvRGBQUAD *c, mvRGBQUAD *cc);

	//将8U1C的图像转换为32S1C的图像
	IplImage * mvGet32S1CImgFrom8U1C( IplImage *p8U1CImg );
	bool mvGet32S1CImgFrom8U1C( IplImage *p32S1CImg, IplImage *p8U1CImg );

	//part12
	//对灰度图像进行自适应阈值化
	int mvImgOstuThreshold(IplImage *pSrcImg);

	//获取图像中的smear
	bool mvGetSmear(IplImage* pGrayImg, bool *bASmear);

	//提取区域内的Hough线段
	int mvExtHoughLine( IplImage *pGrayImg, CvRect roiRect, 
			vector<CvPoint> &vctPt1, vector<CvPoint> &vctPt2 );

	//细化模式
	enum EnumThinMode {
		THIN_VERT_DIR_MODE = 0,  //对竖直方向进行细化
		THIN_HORI_DIR_MODE,		 //对水平方向进行细化
		THIN_H_V_DIR_MODE	     //竖直/水平方向均细化
	};


	//对灰度图像的给定区域获取得到其所对应的变化尖锐处的灰度图像
	IplImage * mvGetSharpChangeGrayImg( IplImage *pGrayImg,
			CvRect roiRect, int nDirMod, uchar nGrayTh=20,			
			float fChangeRateTh=0.2f, uchar nChangeTh=20 );	

	//对灰度图像的给定区域获取得到其所对应的局部最大值的灰度图像
	IplImage * mvGetLocalMaxGrayImg( IplImage *pGrayImg,
			CvRect roiRect, int nThinMod, uchar nThres=20 );

	//获取得到水平边缘图像的主方向
	float mvGetMainOriOfHoriEdgeImg( IplImage *img,
			 CvRect rctRoi, float rho, float theta );

	//提取给定2值图像的标准Hough线段
	CvMemStorage* mvExtSHTHoughLine( IplImage *p2VImg,  
			double rho, double theta, int threshold );

	//part13----获取侧向点	

	//点在那一侧的模式
	enum EnumSideMode {
		IN_SIDE_MODE = 0,  //内侧
		OUT_SIDE_MODE,     //外侧
		IN_OUT_SIDE_MODE   //内/外侧
	};

	//带侧向的点结构
	typedef struct StruSidePoint{
		CvPoint pt;          //点的坐标
		float   fDist;       //距离
		int     nSideFlag;   //点的侧向
	}MvSidePoint;

	//获取在法线方向的侧向点
	int mvGetSidePtsInNormalLine( 		
			StruSidePoint *ptAOfSide,    //获取到的侧向点(输出)
			const int &nMaxCalcPtCnt,    //最多能获取的侧向点数
			const float &fStart,         //计算的起始点位--2.0f
			const float &fEnd,           //计算的终止点位--10.0f 
			const float &fStep,          //计算的点步长--1.0f
			const EnumSideMode &nSideMode,	//需获取的点侧向模式
			const CvPoint &ptNow,        //当前点的坐标
			const float &fNormalK,       //当前点的法线斜率
			const float &fInSide,		 //内侧所对应的系数
			const CvSize &szImg,         //图像尺寸
			IplImage *pRgbShowImg = NULL //用于显示的Rgb图像	
		);

}
#endif