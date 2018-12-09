#ifndef _MV_KEYPT_EXTRACT_H_
	#define _MV_KEYPT_EXTRACT_H_

	#include <algorithm> //for std::sort
	#include <vector>

	#include "libHeader.h"
	#include "MvImgProUtility.h"
	#include "SmallVaildStruct.h"
	#include "MvSiftDescr.h"

	using namespace std;
	using namespace MV_IMGPRO_UTILITY;

	#define USE_OLD_SIFT_CALC   //使用旧的SIFT描述符计算方法

	//sift特征点信息结构
	#ifndef SIFT_FEAT_STRUCT
		#define SIFT_FEAT_STRUCT
		typedef struct _siftFeat
		{
			CvPoint pt;				 //点位置
			uchar cSift[SIFT_DIMS]; //特征值
		} siftFeat;
	#endif

	//块划分的结构体
	typedef struct _MV_DividBlock
	{
		CvPoint ltPt, rbPt;
		CvPoint fkLtPt, fkRbPt;
		int     nFkPercent;   //前景的百分比

		_MV_DividBlock( )
		{
			ltPt = rbPt = cvPoint( -1000, -1000 );
			fkLtPt = fkRbPt = cvPoint( -1000, -1000 );

			nFkPercent = -100;
		};
	}MvDividBlock;

	//角点提取的结构体
	typedef struct MvStruKeyptExtract
	{
	public:
		MvStruKeyptExtract( );
		void mvInitVar( );
		void mvInitKeyptExtract( CvSize sz,
				int nYBlockCnt = 10,
				int nXBlockCnt = 10 );
		void mvUninitKeyptExtract(  );


		//利用前景分块来进行角点提取
		bool mvExtractKeyPtsWithFkBlocks( 
				 IplImage* grayImg, IplImage* srcRgbImg,
				 ImgRoiResizeStru *pSrcDstRoiResStru,
			     IplImage* fkImg, IplImage *pSkipImg,
				 IplImage *pCarWImg, IplImage *pCarHImg,
				 bool bEasyExtractMode=false );

		//利用分块来进行角点提取
		bool mvExtractKeyPtsWithBlocks( 
				IplImage* grayImg, IplImage* srcRgbImg, 
				ImgRoiResizeStru *pSrcDstRoiResStru,
				int nMaxBlockExtPtCnt, IplImage *pSkipImg,  
				IplImage *pCarWImg, IplImage *pCarHImg,
				bool bEasyExtractMode=false );


		//利用全图来进行角点提取
		bool mvExtractKeyPtsWithWholeImg(
				IplImage* grayImg, IplImage* pSkipImg,
				bool bEasyExtractMode=false );

	private:
		//分为多块，每块的前景是否满足存在目标的条件
		bool mvInitDivdeBlock( CvSize useSz,
				int nYBlockCnt, int nXBlockCnt );

		//获取得到膨胀的前景
		void mvGetDilateFkMask( IplImage *pFk2VImg  );


		//分为多块，每块的前景是否满足存在目标的条件
		bool mvGetDivdeBlockInfo( IplImage* fkImg, 
								IplImage* intFkImg );

		//获取需进行提取的块的序号
		vector<CvPoint> mvGetExtrackBlockIdx( int nFkPercTh = -100 );


		//设置提取预置
		void mvSetExtractThreshold( 
				float &fMinEigRequire,  //角点提取的最小能量要求
				int &nMaxBlockExtPtCnt, //最多能提取出的角点数目 
				IplImage *grayImg,		 //给定的灰度图像
				const CvRect &rct		 //给定的提取区域
			);

		//确定各块的角点提取数目
		void mvSetBlockExtractPointNumber( 
				int &nConstPtCnt,		//每块得到最终角点数目的最大值
				int &nMaxMallocPtCnt,	//每块可进行角点提取的最多数目
				int nMaxBlockExtPtCnt,	//每块给定的提取角点数目
				int nExtBlockCnt		//进行提取的总块数
			);
		
		//确定角点提取的最小能量要求
		double mvSetMinEnergyOfPointExtract( IplImage *srcRgbImg );

		//对block块进行角点提取
		bool mvExtractOneBlockKeyPoint(
				CvRect rct, IplImage* grayImg, 
				CvRect rctSrc, IplImage* srcRgbImg, 
				int &nPtCnt, CvPoint2D32f *pRegPts,
				float *pEig, uchar **ppSiftDes,
				IplImage *pCarWImg,	IplImage *pCarHImg,
				double fMinEigThres = 1.0e-8,
				bool bEasyExtractMode = false );

		//利用OpenCV来提取角点
		void mvExtrackKeyptWithCV( 
				int &nPtCnt, 
				CvPoint2D32f *pRegPts,
				IplImage* pGrayImg, 
				bool bEasyExtractMode );

		//利用自己的方法来提取角点
		void mvExtrackKeyptWithMySelf( 
				int &nPtCnt,
				CvPoint2D32f *pRegPts,
				int &nMaxExtPtCnt,
				CvPoint2D32f *pExtRegPts,
				double dMinEigThres, 
				IplImage* pGrayImg, 
				bool bEasyExtractMode );

		//计算角点的SIFT特征值
		void mvCalcSiftDesc(
				uchar **ppSiftDes,								 	
				int nPtCnt, 
				CvPoint2D32f *pRegPts, 
				IplImage* grayImg );

		//OpenCV中的代码的优化(为提速)
		void myGoodFeaturesToTrack( IplImage *image, 
				IplImage *eig, IplImage *tmp, 
				vector<CvPoint2D32f>& corners, int maxCorners, 
				double qualityLevel, double minDistance,
				IplImage *mask, int blockSize, 
				bool useHarrisDetector, double harrisK );

		void mvGoodFeaturesToTrack( IplImage* _image,
				IplImage *eig, IplImage *tmp,
				CvPoint2D32f* _corners, int *_corner_count,	
				double quality_level, double min_distance,
				IplImage* _maskImage, int block_size,
				int use_harris, double harris_k );

		void goodFeaturesToTrackMy2( IplImage *image,
				IplImage *eig, IplImage *tmp,
				vector<CvPoint2D32f>& corners, int maxCorners,
				vector<CvPoint2D32f>& extCorners, int maxExtCorners,
				double dMinEigThres, //角点所需的最小能量要求
				double qualityLevel, double minDistance,
				IplImage *mask, int blockSize,
				bool useHarrisDetector, double harrisK );

		void mvGoodFeaturesToTrack2( IplImage* _image,
				IplImage *eig, IplImage *tmp,
				CvPoint2D32f* _corners, int *_corner_count,
				CvPoint2D32f* _extCorners, int *_extCorner_count,
				double dMinEigThres, //角点所需的最小能量要求
				double quality_level, double min_distance, 
				IplImage* _maskImage, int block_size,
				int use_harris, double harris_k );

	public:
		IplImage *m_pFkMaskImg;  //前景mask图
		
		//角点
		int   m_nKeyPtCnt;         //角点数目
		CvPoint2D32f *m_pKeyPts;   //角点位置
		float *m_pEigOfPt;         //角点的能量
		uchar **m_pp_chSiftDesc;   //sift特征 

		//简单的角点SIFT特征描述符
		SimSiftDescr m_simSiftDescr;  

	private:
		IplImage *m_pFk2VImg;		
		IplImage *m_pPtMapImg; 
		IplImage *m_pEigImg; 
		IplImage *m_pTempImg;

		//将全图分成nY*nX块
		int  m_nYBlockCnt, m_nXBlockCnt;
		MvDividBlock  **m_ppDividBlock;	

	}StruKeyptExtract;


	typedef struct struAddFlag
	{
	public:
		bool  bAddRec; 
		bool  bAddST;
		bool  bAddLT;
		bool  bAddBgT;		
		bool  bAddBgImgT;

		bool  bSTermVaild;
		bool  bLTermVaild;
		bool  bBgTermVaild;
		bool  bBgImgTermVaild;

		struAddFlag( )
		{
			bSTermVaild = false;
			bLTermVaild = false;
			bBgTermVaild = false;
			bBgImgTermVaild = false;

			initVar( );
		}

		void initVar( )
		{
			bAddRec = false;
			bAddST = false;
			bAddLT = false;
			bAddBgT = false;
			bAddBgImgT = false;
		}
	}PtExtAddFlag;


	//角点存储的结构体
	typedef struct StruKeyptStore
	{ 
	public:
		StruKeyptStore( );
		void mvInitVar();

		//设置背景判断过程中点匹配范围
		void mvSetPtMatchRange( CvPoint ptMatchRange4Bg );
	
		void mvInitStruBgKeyptJudge( CvSize sz );
		void mvUninitStruBgKeyptJudge( );

		//添加最近几帧角点信息
		void mvAddRecentInfo( double dTsNow, int nSiftPtCnt, 
				CvPoint2D32f *pPts, uchar **ppSiftDesc );

		//添加短周期角点信息
		void mvAddShortTermInfo( double dTsNow, int nSiftPtCnt, 
				CvPoint2D32f *pPts, uchar **ppSiftDesc );

		//添加长周期角点信息
		void mvAddLongTermInfo( double dTsNow, int nSiftPtCnt, 
				CvPoint2D32f *pPts, uchar **ppSiftDesc );

		//添加背景判断周期角点信息
		void mvAddBgTermInfo( double dTsNow, int nSiftPtCnt, 
				CvPoint2D32f *pPts, uchar **ppSiftDesc );

		//添加背景图像角点信息
		void mvAddBgImgTermInfo( double dTsNow, int nSiftPtCnt, 
				CvPoint2D32f *pPts, uchar **ppSiftDesc );


		//背景角点存储是否可用来判断是为背景角点
		bool mvIsBgKeyptsStoreCanUse( double dTsNow );

		//判断给定的角点是否为背景角点
		bool mvIsBgKeyptsWithBgTerm( double dTsNow,
				int nSiftPtCnt, CvPoint2D32f *pPts,
				uchar **ppSiftDesc, bool *pIsBgPt );

		//利用背景图像周期角点来判断给定的角点是否为背景角点
		bool mvIsBgKeyptsWithBgImgTerm( double dTsNow,
				int nSiftPtCnt, CvPoint2D32f *pPts,
				uchar **ppSiftDesc, bool *pIsBgPt );

	private:
		//将角点和背景角点集合匹配，看是否为背景角点
		bool mvPtMatchWithBgKeypts( CvPoint pt, uchar *pk1, 
				int nBgPtCnt, siftFeat *pBgSiftFeat, 
				CvPoint ptMatch = cvPoint(2,2) );

	public:
		PtExtAddFlag  m_ptExtAddFlag;  //角点添加标识

		//最近几帧的角点特征		
		CycleReplace  m_recentCR;  //循环覆盖存储体
		int		  m_nARecentSiftFeatCnt[RECENTCNT];
		siftFeat  **m_pRecentSiftFeat;	

		//短周期角点特征		
		CycleReplace  m_sTermCR;  //短周期循环覆盖存储体
		int		  m_nASTermSiftFeatCnt[SHORTTERMCNT];
		siftFeat  **m_pSTermSiftFeat;	

		//长周期角点特征		
		CycleReplace  m_lTermCR;  //长周期循环覆盖存储体
		int		  m_nALTermSiftFeatCnt[LONGTERMCNT];
		siftFeat  **m_pLTermSiftFeat;	

		//背景周期角点特征
		CycleReplace  m_bgTermCR;  //背景周期循环覆盖存储体
		int		  m_nABgTermSiftFeatCnt[FORBGTERMCNT];
		siftFeat  **m_pBgTermSiftFeat;	

		
		//背景图像所对应的角点特征
		CycleReplace  m_bgImgTermCR;  //背景周期循环覆盖存储体
		int		  m_nABgImgTermSiftFeatCnt[BGIMGTERMCNT];
		siftFeat  **m_pBgImgTermSiftFeat;	

	private:
		CvSize   m_szUse;
		int      m_nStatTime;

		IplImage *m_pCurFlagImg;  //当前标识图
		int		 **m_ppAppTime;   //角点出现次数数组

		CvPoint  m_ptMatchRange4Bg;  //背景判断过程中点匹配范围

	}KeyptsStore;

	/////////////////////////

#endif