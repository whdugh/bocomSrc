// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef	VIBEMODEL_H
#define VIBEMODEL_H

#ifdef  OPENCV245
	#include "opencv2/opencv.hpp"
	#include "opencv2/objdetect/objdetect.hpp" 
	#include "opencv2/features2d/features2d.hpp" 
	#include "opencv2/highgui/highgui.hpp" 
	#include "opencv2/calib3d/calib3d.hpp" 
	#include "opencv2/imgproc/imgproc_c.h" 
	#include "opencv2/imgproc/imgproc.hpp"   
	#include "opencv2/nonfree/features2d.hpp"
	#include "opencv2/legacy/legacy.hpp"
	#include "opencv2/legacy/compat.hpp"

	#include <wtypes.h>
	using namespace cv; 
#else
	#include "cv.h"
	#include "highgui.h"
	#include "cxcore.h"
#endif

#include "Ultility.h"

#ifndef PI
	#define PI (3.1415926)
#endif
#define ILLUM_DIM (60)
#define	FORIMAGE_NUM (3)
#define PI_HALF (PI*0.5f)
#define BK_SHORT_DIM (8)
#define BK_LONG_DIM (12)
#define INTERVAL_BK (180)


class VibeModel
{
public:
	VibeModel(void);

	~VibeModel(void);

	void libVibeModel_Release_8u(void);
	
	bool libVibeModel_Init_8u(IplImage *img, IplImage *grayImg, const int &dim=3, const int &R=8, 
								const int &seta=16, const int &min=1, const int &N=5, 
								const int &colorMode=V_BGR, const bool bIsResize=false, 
								const int &wScale=1, const int&hScale=1, const int &nNerbor=2);

	void libVibeModel_Update_8u(IplImage *img, IplImage *grayImg, IplImage *bgForeImg,
									IplImage *bkImg=NULL, int nSmoothTypes=V_SM_ORIGINAL);
	

private:
	void libVibe_AllocImages(void);

	void libVibe_AllocVariable(const int &w, const int &h, const int &dim, const int &R,
				const int &seta, const int &min, const int &N, const int &colorMode,
				const bool bIsResize, int wScale, int hScale, int nNerbor);

	void libVibe_AllocBkImages(void);

	bool libVibe_ModelInit(IplImage *img, IplImage *grayImg, bool bIsChange=false);

	void vibeModelUpdate_8u_C1R( IplImage *oriGrayImg, IplImage *foreImg, bool bIsChanged=false );

	void vibeModelUpdate_8u_C3R(IplImage *oriImg, IplImage *foreImg, bool bIsChanged=false);

	inline int mvGetRandomNumber(const int &min, const int &max) const;

	inline unsigned short mvEuclidDist_GRAY(const BYTE &a, const BYTE &b) const;

	inline int mvEuclidDist_RGB( BYTE *a, BYTE *b );

	inline unsigned short mvEuclidDist_L_Inf(const BYTE *pSrcB, const BYTE *pSrcG, const BYTE *pSrcR, const BYTE *pOutB,
									const BYTE *POutG, const BYTE *pOutR) const;

	IplImage **mvEuclidDist_L_Inf(IplImage *bgrImg) const;

	inline void mvGetRandomNeighbrXCoordinate(const int &x, int &nb_x) const;

	inline void mvGetRandomNeighbrYCoordinate(const int &y, int &nb_y) const;

	void mvFittering(IplImage *bgForeImg, const int nSmoothTypes, const int nSize=36) const;

	bool mvIsSuddenIlluminance(IplImage *grayImg, IplImage *foreImg);

	void mvGetStopForeground(IplImage *bgForeImg);

	void mvUpdateIlluForeground(IplImage *lstGrayImg, IplImage *foreLstImg, IplImage *grayImg, 
		IplImage *bgForeImg, const bool &bIsChange);

	int mvComputeIIlluminan(IplImage *grayImg, float scale_W=0.25f, float scale_H=0.25f, double dExpoIndex=0.5);

	void mvMoveShadowsByHSV(IplImage *bgrImg, IplImage *bgrBgImg, IplImage *bgForeImg) const;

	void mvMoveShadowsByConstancy(IplImage *foreLstImg, IplImage *bgrImg, IplImage *bgrBgImg, IplImage *bgForeImg);

	void mvMoveShadowsByTexture(IplImage *bgrImg, IplImage *bgrBgImg, IplImage *grayImg, IplImage *bgForeImg);

	void mvMoveShadowsByRatio(IplImage *bgrImg, IplImage *bgrBgImg, IplImage *grayImg, IplImage *bgForeImg);

	void mvComputeNoiseDeviation(IplImage *oriImg, CvScalar &noiseDv) const;

	void mvComputeRatio(IplImage *bgrImg, IplImage *bgrBgImg, IplImage *shadowImg, IplImage *foreUpdtImg,
							IplImage *shdwUpdtImg, CvScalar &noiseDv, const int &W_R, const float &fLamda, const float &TR);
	
	void mvGetIntensityMskImg(IplImage *bgrImg, IplImage *bgrBgImg, IplImage *bgForeImg, IplImage *cmpMskImg) const;

	bool mvRemoveShadowByHist(IplImage *roiImg, CvRect &rect) const;

	IplImage *mvCreateIntegrayImg( IplImage *binaryImg ) const;

	inline void mvErodeByNbr(IplImage *binaryImg, int wx_R, int wy_R) const;

	void mvUpdateMedianBk(const IplImage *img, IplImage *pMedianBg, BYTE ***pList, int nDim, int nChnnls);

	void mvGetBkImg(IplImage *img, IplImage *longBkImg);

	void mvSmoothByContours(IplImage *bgForeImg, const int nSmoothTypes, int nSize) const;

	void mvSmoothByMedian(IplImage *bgForeImg) const;

	void mvSobelHorizontal(IplImage *grayImg, IplImage *sobelImg, const int MIN_THRESHOLD_SOBEL) const;

	inline long	mvGetIppSumInRect(IplImage* integrayImg, CvRect rect) const;

	void mvRemovalSmear(IplImage *grayImg) const;

	IplImage *mvFilterExposurePixels(IplImage *grayImg) const;

	void mvGetSobel(IplImage *grayImg, IplImage *sobelImg) const;

	void mvGetVSobel(IplImage *srcImg, IplImage *dstImg) const;

	void mvGetHSobel(IplImage *srcImg, IplImage *dstImg) const;

	void mvGetBkDiffImg(const IplImage *grayImage, const IplImage *bgImage, 
							IplImage *diffImg, int nDiffThres, int nMinDiffThres, float fThres);

	void mvVibeSobel( IplImage *src, IplImage *dst, const int xorder, const int yorder, const int aperture_size ) const;

private:
	unsigned short m_skpFram;

	unsigned short m_NG_C; //2 对应于4连通域 3对应于8连通域

	short m_BOUND_UP; //邻域随机数的开上界

	short m_BOND_DOWN; //邻域随机数的闭下界

	const unsigned short THRESHOLD;


private:
	IplImage **m_samImgs; //背景模型图像

	IplImage *m_foreLstImg; //背景前景map图像

	IplImage *m_bgrImg; //当前帧彩色图像

	IplImage *m_bgrLstImg; //前一帧彩色图像

	IplImage *m_grayImg; //当前帧灰度图像

	IplImage *m_lastImg; //灰度前帧图像

	IplImage **m_foreImgSet; //前几帧前景图像集合

	IplImage *m_upForImg; //多帧保存的前景图像

	IplImage *m_sitaThImg; // 前景与阴影比值图像

	IplImage *m_rndNbrImg; //随机数图像

	BYTE ***m_pLongBkSet; //长期背景图像模型

	BYTE ***m_pBkSet; //短期背景图像模型

	IplImage *m_bkImg; //短期背景图像

	IplImage *m_bkLongImg; //长期背景图像

	int m_width; //背景建模图像宽度

	int m_height; //图像高度

	CvSize m_imgSize;

	int m_nN; //抽样个数

	int m_nSubN; //背景更新的时间间隔

	int m_nMinN; //最小的基数

	int m_nR; //球半径R

	int m_chnnls; //颜色通道模式 支持单通道与三通道

	int m_nInitCount; //初始化帧数的统计

	int m_nLChangHist[ILLUM_DIM]; //统计多帧亮度

	int m_nLastIllu; //前一帧的照度

	float m_fRatio; //照度比值

	bool m_bIsChanged; //判断是否照度改变

	bool m_bIsUpdate; //前景是否要更新

	int m_colorMode; //bgr 与 rgb颜色模式

	CvRNG m_rnd_state; //初始化随机数生成器

	unsigned int m_frmCount; //帧号统计
	
	bool m_bIsLongBk; //是否存在长期背景

	bool m_bIsFirtTime; //是否是第一次运行本lib库

	bool m_bIsFirstUpdate; //是否是第一次调用更新程序

	float m_resizeRatio; //缩放比例

	bool m_bIsResize; //是否需要缩放

	float m_lstNotZeroRatio; //前一帧前景的比例

	bool m_bIsAllFore; //是否是全部前景

	int m_seta; //传入的抽样参数阈值

	int m_countSud;

	#ifndef LINUX
		char *m_buffer;
	#endif

};

#endif