/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：StabilizeImage.h
* 摘要: 基于局部区域SIFT匹配的稳像
* 版本: V1.1
* 作者: 贺岳平
* 完成日期: 2010年01月19日
*/

#ifndef __VIDEOSTAB_H
#define __VIDEOSTAB_H

#ifdef LINUX
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#ifndef CHDIR
		#define CHDIR
		#define _chdir(a) chdir(a)
	#endif
	#ifndef MKDIR
		#define MKDIR
		#define _mkdir(a) mkdir(a, 0777)
	#endif
	#ifndef ACCESS
		#define ACCESS
		#define _access(a, b) access(a, b)
	#endif
#else
	#include <io.h>
	#include <direct.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <algorithm> 
#include <vector>

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
	using namespace cv; 
#else
	#include "cv.h"
	#include "highgui.h"
	#include "cxcore.h"
#endif


#include "sift_descr.h"
#include "lsd.h"


using namespace std;


//#define  DEBUG_VIDEO_STAB
#ifdef DEBUG_VIDEO_STAB
	#define SHOW_STAB_AREA    //稳像区域
	#define  DEBUG_STABMATCH        //稳像匹配
	#ifdef DEBUG_STABMATCH
		#define  DEBUG_SAVE_STABMATCH  //存稳像匹配结果
	#endif
	#define  DEBUG_STABLINE_MATCH     //稳像线匹配
	#define  DEBUG_STABLINE_MODIFY    //稳像线匹配
#endif

//#define DEBUG_LIUX_TXT      //读文本中的线段信息


#ifndef SIFT_WIN_SIZE
	#define SIFT_WIN_SIZE    4
#endif
#ifndef SIFT_BINS
	#define SIFT_BINS        8
#endif
#ifndef SIFT_DIMS
	#define SIFT_DIMS  (SIFT_WIN_SIZE*SIFT_WIN_SIZE*SIFT_BINS) //128 
#endif


#define MAX_REGIONS   10                 //最大画图区域个数
#define REFREGION_MAX_KEYPTCNT    40     //稳像区域内最多的角点数目
#define REGION_MAX_KEYPTCNT      120     //匹配区域内最多的角点数目

#define MAX_FEAT_FILE_NUM        100     //最多的特征文件数目
#define MAX_LONGFEAT_FILE_TNUM	 10      //保存的次初始参考帧的数目

#define MAX_STABLINE_NUM         10      //最多的稳像线段数目
#define MAX_EXTRACT_LINE_NUM     1000    //最多的提取线段数目

#define MIN_MATCHCNT			 8		 //最小匹配上的数目
#define MAX_ACCUMUL_ERROR        4       //允许的最大累计误差

#ifdef LINUX
	#define SAVE_FEAT_TIME         900.0       //隔多长时间保存一次特征
	#define UPDATE_FEAT_TIME       3600.0      //隔多长时间更新一次特征
	#define UPDATE_LONGFEAT_TIME   (3600.0*12) //隔多长时间更新一次长期特征
#else
	#define SAVE_FEAT_TIME         1.0       //10 隔多少帧保存一次特征
	#define UPDATE_FEAT_TIME       5.0       //60 隔多长时间更新一次特征
	#define UPDATE_LONGFEAT_TIME   (5.0*12)  //240 隔多长时间更新一次长期特征
#endif

//#define SAVE_FEAT_TIME         18.0       //隔多长时间保存一次特征
//#define UPDATE_FEAT_TIME       72.0      //隔多长时间更新一次特征
//#define UPDATE_LONGFEAT_TIME   (72.0*12) //隔多长时间更新一次长期特征



#define  LANE_WIDTH_MIN_RATE  0.0025f   //标志线的宽度占图像比率的最小值
#define  LEFT_BRIGHT_RIGHT   -1         //左边比右边亮
#define  RIGHT_BRIGHT_LEFT    1         //右边比左边亮
#ifndef PI_ONE_DEGREE
	#define  PI_ONE_DEGREE  0.0174533
#endif


#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif


#define stab_eps 1e-8
#define zero(x) (((x)>0?(x):-(x))<stab_eps)



//--------------------------CMyImgPro类--------------------------
typedef struct myMegerLine
{
	CvPoint2D32f	pt1;
	CvPoint2D32f	pt2;
	double	dOri;			//0-PI
	double	dLen;         
	bool	bMerge;			//合并
	bool	bBeMerge;		//被合并
	bool	bUsed;			//已经使用
}MyMegerLine;

class CMyImgPro
{
public:
	CMyImgPro();
	~CMyImgPro();

	//得到两点之间的像素点
	static bool mvGetPixelOfTwoPoint( 
					double P1X, double P1Y, double P2X, double P2Y, 
					int nHeight, int nWidth, double w,
					int &nPixelNum, int *nAPixelYX );

	//对给定线段附近提取最好的线段结果
	static bool mvGetBestLineOnGiveLine( 
					CvPoint2D32f srcPt1, CvPoint2D32f srcPt2, 
					IplImage *srcImg, CvSize lineSz, 
					CvPoint2D32f &ptLinePt1, CvPoint2D32f &ptLinePt2 );

	static bool mvGetModifyStabLine( 
					IplImage *srcImg, int &nStabLineCnt,
					CvPoint2D32f pAStabLinePt1[],
					CvPoint2D32f pAStabLinePt2[], 
					int nChannel=-1, int nPresetId=-1 );

	//calculate the gradient image of given image
	static void mvGetMaxGradientImg( 
					IplImage* img, IplImage *pGradImg, 
					IplImage* pMaskImg, bool bIsVLine ); 

	double  mvLinesDist(
					CvPoint2D32f pLinePt1, CvPoint2D32f pLinePt2, 
					CvPoint2D32f pAStabPt1,CvPoint2D32f pAStabPt2);

	//get the color distribution on the line
	void   getColorModelOfLines( int nLineCnt, 
				CvPoint2D32f ptALEndPt1[], CvPoint2D32f ptALEndPt2[], 
				int nAMod[], IplImage *srcImg, 
				int nChannel=-1, int nPresetId=-1  );

	void getColorModelOfStabLines( int nLineCnt,
			CvPoint2D32f ptALEndPt1[], CvPoint2D32f ptALEndPt2[],
			int nAMod[], IplImage *pShowImg);
	int  getColorDistOfLine(
			CvPoint2D32f endPt1, CvPoint2D32f endPt2,
			IplImage *pGrayImg, IplImage *pShowImg=NULL );

	//unite the short break lines together
	static void mvUniteGiveAngleLines( int nLineNum, 
				CvPoint2D32f *pALinePt1, CvPoint2D32f *pALinePt2,
				CvPoint2D32f givePt1, CvPoint2D32f givePt2, 
				double dDiffArcTh, double dDistWithGiveLineTh,
				double dPtLineTh, double dPt2PtTh, int &nMeglineNum,
				CvPoint2D32f *pAMegLinePt1, CvPoint2D32f *pAMegLinePt2, 
				IplImage *pShowLineImg=NULL );

	//获取给定线段附近和其相似的线段
	static int mvFiltrateWantMegerLines( int nLineNum,
				CvPoint2D32f *pALinePt1, CvPoint2D32f *pALinePt2, 
				CvPoint2D32f givePt1, CvPoint2D32f givePt2, 
				double dDiffArcTh, double dDistWithGiveLineTh,
				MyMegerLine *pLineArray );

	//对给定角度附近值的线段进行合并
	static void mvUniteLinesFirst( int nLineNum, 
				MyMegerLine *pLineArray, 
				double dPt2PtTh, double dPtLineTh );
	static void mvUniteLinesLoose( int nLineNum,
				MyMegerLine *pLineArray );

	//保存线段合并后的结果
	static void mvSaveLineResultAfterMeger( int nLineNum, 
				MyMegerLine *pLineArray,
				int &nAfterMLineNum, 
				MyMegerLine *pAfterMLineArray );
};

//--------------------------CMyMath类--------------------------
typedef struct 
{
	double x,y;
}dPoint;

class CMyMath
{
public:
	CMyMath();
	~CMyMath(); 

	// 判断点是否在多边形内
	static bool mvIsPointInPolygon( int nPolySize, 
						  const CvPoint2D32f *pts, 
						  CvPoint2D32f pt );

	//the interface function of calculate lines intersection
	bool getTwoLineIntersection( 
			CvPoint2D32f line1Pt1, CvPoint2D32f line1Pt2,
			CvPoint2D32f line2Pt1, CvPoint2D32f line2Pt2,
			CvPoint2D32f &intersectPt );


	double lengthLine(double x0, double y0, double x1, double y1);

	double distBetweenPtLine( double x0,double y0, 
			  double x1,double y1, double x,double y );

	//calculate the point's position 
	bool getPointsInLine( 
			CvPoint2D32f line1Pt1, CvPoint2D32f line1Pt2,
			int nPtCnt, CvPoint2D32f ptA[] );

	static void mvQuickSort(double *data,int *index,
					int l, int r, bool bSmallToBig=true);

private:
	//计算交叉乘积(P1-P0)x(P2-P0)
	double xmult(dPoint p1,dPoint p2,dPoint p0);

	//判点是否在线段上,包括端点
	int dot_online_in(dPoint p,dPoint l1,dPoint l2);	

	//判两点在线段同侧,点在线段上返回0
	int same_side(dPoint p1,dPoint p2,dPoint l1,dPoint l2);

	//判两直线平行
	int parallel(dPoint u1,dPoint u2,dPoint v1,dPoint v2);	

	//判三点共线
	int dots_inline(dPoint p1,dPoint p2,dPoint p3);

	//判两线段相交,包括端点和部分重合
	int intersect_in(dPoint u1,dPoint u2,dPoint v1,dPoint v2);

	//计算两线段交点,请判线段是否相交(同时还是要判断是否平行!)
	dPoint intersection(dPoint u1,dPoint u2,dPoint v1,dPoint v2);
};


typedef struct _KeyPtFeat 
{
	CvPoint2D32f  ptPos;          // 位置坐标
	uchar cSiftVal[SIFT_DIMS];    // SIFT描述符
	_KeyPtFeat( )
	{
		ptPos  = cvPoint2D32f(0.0f, 0.0f);
	}

}KeyPtFeat;


typedef struct _StabTsSet 
{
public:
	double dNowTs;
	double dLastTs;
	double dLastSavePtRef;
	double dLastUpdate;	
	double dLastSaveInit2Feat;

	_StabTsSet( )
	{
		initVar( );
	}

	void initVar( )
	{
		dNowTs = -100000.0;
		dLastTs = -100000.0;
		dLastSavePtRef = -1000.0;
		dLastUpdate = -1000.0;	

		dLastSaveInit2Feat = 0;//-10000000.0;
	}

}StabTsSet;


typedef struct _StabLineSet 
{
public:
	///提取的线
	unsigned int nLineCnt;  //提取的线的数目
	int	    *pBelongNo;     //属于哪条稳像线
	CvPoint *pLinePt1;
	CvPoint *pLinePt2;	    //当前提取的线的起点,终点

	//稳像线数
	int     nStabLineNum;    //稳像线数目
	CvPoint *pStabLinePt1;
	CvPoint *pStabLinePt2;   //稳像绘制线的起点,终点
	int		*pAStabLineMod;  //稳像线颜色的模式

	_StabLineSet( )
	{
		initVar( );
	}

	void initVar( )
	{
		nLineCnt = 0;                
		pBelongNo = NULL;
		pLinePt1 = NULL;
		pLinePt2 = NULL; 

		nStabLineNum = 0;
		pStabLinePt1 = NULL;
		pStabLinePt2 = NULL;
		pAStabLineMod = NULL;
	}

	void initStabLineSet( )
	{
		initVar( );              
		pBelongNo = new int [MAX_EXTRACT_LINE_NUM];
		pLinePt1 = new CvPoint [MAX_EXTRACT_LINE_NUM];
		pLinePt2 = new CvPoint [MAX_EXTRACT_LINE_NUM];  

		pStabLinePt1 = new CvPoint [MAX_EXTRACT_LINE_NUM];
		pStabLinePt2 = new CvPoint [MAX_EXTRACT_LINE_NUM];
		pAStabLineMod = new int[MAX_EXTRACT_LINE_NUM];
	}

	void uninitStabLineSet( )
	{
		if( NULL != pBelongNo )
		{
			delete [] pBelongNo;
		}
		if( NULL != pLinePt1 )
		{
			delete []  pLinePt1;
		}
		if(  NULL != pLinePt2 )
		{
			delete [] pLinePt2;  
		}

		if( NULL != pStabLinePt1 )
		{
			delete [] pStabLinePt1;
		}
		if( NULL != pStabLinePt2 )
		{
			delete [] pStabLinePt2;
		}
		if( NULL!= pAStabLineMod )
		{
			delete [] pAStabLineMod;
		}
	}
}StabLineSet;


typedef struct _StabRefFeat
{
public:
	int  m_nStabAreaNum;
	char m_chFeatDir[104]; 
	char m_chChanPreDirName[104];
	char m_chStabAreaFN[104];

	//最开始的参考特征
	bool   bInitRef;
	char   m_chInitRefFeatFN[104];
	int    m_nAInitRefKeyptCnt[MAX_REGIONS];
	KeyPtFeat** m_ppInitRefKeyptsFeat;
	float  m_fAInitRefImgMean[MAX_REGIONS];
	float  m_fAInitRefImgSdvDev[MAX_REGIONS];

	//隔段时间保留一次的参考特征
	int    m_nRefIdx;
	bool   bARef[MAX_FEAT_FILE_NUM];
	char   m_chRefFeatDir[104];
	int    m_nARefKeyptCnt[MAX_FEAT_FILE_NUM][MAX_REGIONS];
	KeyPtFeat*** m_ppRefKeyptsFeat;
	float  m_fARefImgMean[MAX_FEAT_FILE_NUM][MAX_REGIONS];
	float  m_fARefImgSdvDev[MAX_FEAT_FILE_NUM][MAX_REGIONS];

	//次参考特征
	int    m_nLongRefIdx;
	bool   bALongRef[MAX_LONGFEAT_FILE_TNUM];
    char   m_chRefLongFeatDir[104];
	int    m_nALongRefKeyptCnt[MAX_LONGFEAT_FILE_TNUM][MAX_REGIONS];
	KeyPtFeat*** m_ppLongRefKeyptsFeat;
	float  m_fALongRefImgMean[MAX_LONGFEAT_FILE_TNUM][MAX_REGIONS];
	float  m_fALongRefImgSdvDev[MAX_LONGFEAT_FILE_TNUM][MAX_REGIONS];

public:
	_StabRefFeat( );
	
	void initVar( );

	void setDirAndFileName( int nChannelId, int nPreSetId );

	void setStabAreaFN(  );

	void setInitRefFeatFN(  );
	
	void setRefFeatDirName(  );

	void setLongRefFeatDirName(  );

	//若稳像区域信息文本不存在，则创建并将稳像区域信息存入；否则，
	//  判断其和当前信息是否一致。若一致，不需操作；否则，需要将
	//  原来的覆盖，并将参考角点信息删除。
	void mvSaveStabAreaInfo( float fExtX, float fExtY,
						int nAreaNum, int *nAPolySize, 
						CvPoint2D32f **ppPolyPts );

	void writeStabArea( char chFileNam[104], float fExtX, float fExtY, 
			  int nAreaNum, int nAPolySize[], CvPoint2D32f **ppPoltpts );


	bool readStabArea2JudegSame( FILE *fpStabArea_r,
			float fExtX, float fExtY, int nAreaNum, 
			int nAPolySize[], CvPoint2D32f **ppPoltpts );

	void initStabMatchFeat( int nStabAreaNum );
	
	void uninitStabMatchFeat( );

	//读入文本作为角点特征
	bool mvReadKeyptFromTxt( char cFileName[], int &nStabAreaNum, 
		   int nAKeyptCnt[], float fAImgMean[], float fAImgSdvDev[],
		   KeyPtFeat** ppKeyptsFeat );

	//将角点特征存为文本
	void mvSaveKeyptToTxt( char cFileName[], int nStabAreaNum, 
		int nAKeyptCnt[], float fAImgMean[], float fAImgSdvDev[],
		KeyPtFeat** ppKeyptsFeat );

	//将参考角点存为文本
	void mvSaveRefKeypt( char cKeyptFeatFN[], int nKeyptCnt, 
						 float fImgMean, float fImgSdvDev, 
						 KeyPtFeat *pKeyptFeat );

	//读入文本中参考角点
	bool mvReadRefKeypt( char cKeyptFeatFN[], int &nRefKeyptCnt, 
						 float &fImgMean, float &fImgSdvDev, 
						 KeyPtFeat *pRefKeyptsFeat );
	
	//将其他的一些特征点存为参考点
	void mvSetRefKeyptFromOtherKeypt( int nAreaNum, 
							int   nAKeyptCnt[], 
							float fAMean[], 
							float fASdvDev[], 
							KeyPtFeat **pKeyptsFeat, 
							int   nARefKeyptCnt[], 
							float fARefMean[], 
							float fARefSdvDev[], 
							KeyPtFeat **pRefKeyptsFeat );

}StabRefFeat;

	
//对初始参考角点和读入所存的稳像后参考角点，与当前帧角点进行匹配
typedef struct _StabMatchFeat
{
public:
	int    nBestMatchResultNo[REGION_MAX_KEYPTCNT];
	int    nBestMatchResultVal[REGION_MAX_KEYPTCNT];
		
	int	   pMatchNo[MAX_REGIONS][REGION_MAX_KEYPTCNT];	
	int    pTrueMatchNo[MAX_REGIONS][REGION_MAX_KEYPTCNT];	

	int    nAMatchPair[MAX_REGIONS]; 
	int    nAStabMatchPair[MAX_REGIONS];

	CvPoint2D32f AOffsetPt[MAX_REGIONS];

public:
	_StabMatchFeat( )
	{

	}

	//稳像区域的参考点和当前角点进行匹配
	void mvFeatsMatch( int nAreaNum, 
		int nACnt1[], KeyPtFeat** pFeat1,
		int nACnt2[], KeyPtFeat** pFeat2, 
		float fAMatchR[], CvPoint2D32f AOffsetPt[], 
		int &nMaxNo, int &nMaxVal );

	//参考点和当前角点进行匹配
	int mvMatchRefCurFeats( int nAreaNum, 
			int nACnt1[], KeyPtFeat** pFeat1,
			int nACnt2[], KeyPtFeat** pFeat2, 
			float fAMatchR[], float fAScale[],
			CvPoint2D32f AOffsetPt[], 
			CvPoint2D32f &ptOffsetWithRef,
			bool bAMatchSucc[] );

	//角点群1和角点群2进行匹配
	void mvMatch2SetFeats( int nCnt1,   KeyPtFeat* pFeat1,
						   int nCnt2,   KeyPtFeat* pFeat2,
						   float fMatchR,   int nAMatchNo[],
						   int &nMatchPair, int &nStabMatchPair,
						   CvPoint2D32f &ptAvgOffset );


	//SIFT特征匹配
	int mvSiftMatch( int nRefPtCnt, KeyPtFeat *pRefPtsFeat, 
					 int nCurPtCnt, KeyPtFeat *pCurPtsFeat,
					 float fR, int *pMatchNo );
	
	//计算得到匹配后的所认为的图像位移距离
	int mvGetOffsetPtForMatch( int nRefPtCnt,
				KeyPtFeat *pRefPtsFeat,
				KeyPtFeat *pCurPtsFeat,
				int *pMatchNo,
				CvPoint2D32f& offsetPt );
	
	//对各区域内的匹配点对判断是否为正确的匹配
	int mvGetTrueSiftMatch( int *pMatchPtNo, float fScale,
				 int nRefPtCnt, KeyPtFeat *pRefPtsFeat, 
				 int nCurPtCnt, KeyPtFeat *pCurPtsFeat, 
				 CvPoint2D32f matchOffserPt, float fMaxOffR, 
				 int *pTrueMatchNo );

}StabMatchFeat;

//--------------------------CVideoStabilizer类--------------------------
class CVideoStabilizer
{

public:
	CVideoStabilizer();
	~CVideoStabilizer(); 

	//初始化，包括块匹配区域生成，利用n帧图像初始背景点统计，返回多边形外接矩形
	int  m_nChannelId, m_nPreSetId;
	bool mvInitial( int nChannelId, int nPreSetId,
			int nAreaNum,int *nPolySize, CvPoint2D32f **fPolyPts,
			int nImgW, int nImgH, bool bMoveCamera = false );

	//稳像区域设置 和 稳像线段设置
	void mvSetStabAreaAndLine( int &nPolyCnt,
			int nPolySize[4], CvPoint2D32f **ppPolyPt,
		    int &nStabLineCnt, CvPoint2D32f pAStabLinePt1[10],
			CvPoint2D32f pAStabLinePt2[10] );

	//设置稳像线段
	bool mvSetStabLine( int nChannelId, int nPreSetId,
				int &nLineNum, CvPoint2D32f *pLinePt1, 
				CvPoint2D32f *pLinePt2, int *pLineMod );

	//卸载
	void mvUnInitial( );	

	//是否进行稳像调试模式
	void mvSetIsDebug( bool bDebugStab )
	{
		 m_bDebugStab = bDebugStab;
	 };

	//得到与初始图的偏移值的调用接口
	bool mvGetImgOffSetWithInit( bool bDay, IplImage *pSrcImg,
				   long nIndex, double ts, CvPoint &offsetPt );

	
	//新稳像函数的调用接口
	bool mvUseNewVideoStab( bool bDay, long nIdx, double tsNow,
			IplImage *pShakeSrcImg, CvPoint2D32f &ptSrcImgOffset,
			bool &bNeedRecorrect );

private:
	StabMatchFeat m_stabMatch;
	bool     m_bNeedStab;
	bool     m_bTen;
	bool     m_bFirst;
	bool     m_bMoveCamera;
	int      m_nStabIndex;
	bool     m_bStabSuccess[50];
	double   m_dUpdateSuccessStabValTime;
	double   m_tsLastUpdateSuccessStab;
	int      m_nSuccessStabIndex;
	CvPoint  m_ptASuccessStabOffset[50];
	int      m_nRefUpdate;//参考帧是否更新
	int      m_nIdx; 
	int		 m_nStabLinesCnt;
	int      m_nTimes;	//使用线段匹配的次数
	int           m_nStabLineNum;      //稳像线数目
	CvPoint2D32f  m_AStabLinePts1[MAX_STABLINE_NUM]; //稳像线点对的起点
	CvPoint2D32f  m_AStabLinePts2[MAX_STABLINE_NUM]; //稳像线点对的终点
	int           m_nAStabLineMod[MAX_STABLINE_NUM]; //稳像线颜色的模式

	CvPoint2D32f  m_AAddOffsetStabLinePts1[MAX_STABLINE_NUM]; //加上偏移值后的稳像线点对的起点
	CvPoint2D32f  m_AAddOffsetStabLinePts2[MAX_STABLINE_NUM]; //加上偏移值后的稳像线点对的终点

public:
	bool     m_bNoDetectWhenStabFail;

private:
	long     m_lFrameCnt;

	CvPoint2D32f  m_ptShake;
#ifndef LINUX
public:
	bool          m_wBStabSucc;
	CvPoint2D32f  m_wPtOffset;
#endif

private:
	void mvUseLinesStab( IplImage *pShakeSrcImg, 
			int nStabLineCnt, CvPoint2D32f pAStabLinePt1[10],
			CvPoint2D32f pAStabLinePt2[10],
			CvPoint &ptOffsetWithLines );

	bool mvGetStabBestLine(int nTimes,int nCnt, IplImage *pImg,
				CvPoint2D32f pALinePt1, CvPoint2D32f pALinePt2,
				CvPoint2D32f &ptLinePt1, CvPoint2D32f &ptLinePt2);

	bool mvPtNoOverLap(CvPoint2D32f pBestLinePt1, CvPoint2D32f pBestLinePt2,
					   CvPoint2D32f pStabLinePt1, CvPoint2D32f pStabLinePt2);

	//根据两直线计算偏移量
	void mvGetXYWithLine(CvPoint2D32f pBestPt1,CvPoint2D32f pBestPt2, 
						 CvPoint2D32f pStabPt1,CvPoint2D32f pStabPt2,
						 double dDst, double &dX, double &dY );

	//遍历法两直线计算偏移量
	double mvGetOffsetXY(bool pAGetBestStabLine[], int nStabLineCnt, 
				CvPoint2D32f pABestPt1[],CvPoint2D32f pABestPt2[], 
				CvPoint2D32f pAStabPt1[], CvPoint2D32f pAStabPt2[],
				CvPoint &OffsetPtWithLine);

	double mGetLenX(CvPoint2D32f pBestLinePt1, CvPoint2D32f pBestLinePt2, 
				    CvPoint2D32f pStabLinePt1, CvPoint2D32f pStabLinePt2);

	double mGetLenY(CvPoint2D32f pBestLinePt1, CvPoint2D32f pBestLinePt2, 
					CvPoint2D32f pStabLinePt1, CvPoint2D32f pStabLinePt2);

	//点到线之间的距离
	double mvDotToLineDst(float x, float y, CvPoint2D32f pt1, CvPoint2D32f pt2);
 
	//判断是不是同一条直线
	bool mvIsDifferrentLine(CvPoint2D32f pt1[],	CvPoint2D32f pt2[],
							int nNum, double dDstTh); 

	//修改参考点的坐标
	void mvModifyRefPointPostion( CvPoint2D32f offsetPt );
	void mvExtractLineFromStab(IplImage* grayImg, int &nLineCnt,
						   CvPoint *pLinePt1, CvPoint *pLinePt2);

	void mvSaveTenBestMatchLines( float fX, float fY,int nStabLineCnt,
		CvPoint2D32f pABestStabLinePt1[],CvPoint2D32f pABestStabLinePt2[],
		CvPoint2D32f pAStabLinePt1[], CvPoint2D32f pAStabLinePt2[],
		double dASaveMaxDstAfterOffset[10][10],
		bool bIndeedOffset,bool bAGetBestStabLine[]);

	bool mvJudgeIsIndeed(float fX, float fY,int nStabLineCnt,
		CvPoint2D32f pABestStabLinePt1[],CvPoint2D32f pABestStabLinePt2[],
		CvPoint2D32f pAStabLinePt1[], CvPoint2D32f pAStabLinePt2[],
		double dASaveMaxDstAfterOffset[10][10],bool bAGetBestStabLine[]);

public:
	//得到稳像后的图像指针
	IplImage* mvGetPointerOfStabImg( )
	{
		return m_pStabSrcImg;
	};

	//是否需要回预置位
	bool mvNeedGotoProLoc( );

private:
	//读入稳像检测的配置文件
	void mvReadStabDetConfig( 
		bool &bNoDetWhenStabFail, 
		float &fExtX, float &fExtY );

	//对稳像区域的大小位置进行初始化
	void mvInitStabArea( int nAreaNum, 
		int *nPolySize, CvPoint2D32f **fPolyPts,
		CvPoint2D32f exPt, int nImgW, int nImgH );

	//将稳像区域进行缩放,得到固定大小稳像区域
	void mvResizeStabArea(  );

public:
	CvRect   m_AStabRect[MAX_REGIONS];      //初始的匹配区域


private:	
	IplImage * m_pSrcImg;  //稳像前的原始图像
	IplImage * m_pStabSrcImg;  //稳像后的原始图像

	bool     m_bDebugStab;

	int           m_nStabPolyNum;
	int	          *m_nAStabPolySize;  
	CvPoint2D32f  **m_AARefPolyPts;  //初始的稳像区域点(初始化后转到图像resize后以匹配区域为参考的坐标系)
	CvPoint2D32f  **m_AAStabPolyPts; //初始的匹配区域点(初始化后转到图像resize后以匹配区域为参考的坐标系)
	
	CvRect      m_ARefRect[MAX_REGIONS];           //初始的稳像区域
	CvRect      m_AStabScaleRect[MAX_REGIONS];     //对图像resize后的匹配区域
	CvRect      m_ARef2StabScaleRect[MAX_REGIONS]; //对图像resize后以匹配区域为参考的稳像区域

	float       m_fAScale[MAX_REGIONS];
	float		m_fAMatchR[MAX_REGIONS];

	IplImage   *m_pStabAreaImg[MAX_REGIONS];
	IplImage   *m_pInitStabAreaImg[MAX_REGIONS];

	IplImage   *m_pTemImg[MAX_REGIONS];
	IplImage   *m_pTem1Img[MAX_REGIONS];
	IplImage   *m_pTem2Img[MAX_REGIONS]; 

	CvPoint2D32f  m_AKeypts[REGION_MAX_KEYPTCNT];
	uchar  *m_AASiftDes[REGION_MAX_KEYPTCNT];

	StabRefFeat m_stabRefFeat;  //稳像参考特征

	int    m_nACurKeyptCnt[MAX_REGIONS];
	float  m_fATempImgMean[MAX_REGIONS];
	float  m_fATempImgSdvDev[MAX_REGIONS];
	KeyPtFeat** m_ppRefKeyptsFeat;

	int    m_nARefKeyptCnt[MAX_REGIONS];
	float  m_fARefImgMean[MAX_REGIONS];
	float  m_fARefImgSdvDev[MAX_REGIONS];
	KeyPtFeat** m_ppCurKeyptsFeat;

	int    m_nATempKeyptCnt[MAX_REGIONS];
	float  m_curfAImgMean[MAX_REGIONS];
	float  m_curfAImgSdvDev[MAX_REGIONS];
	KeyPtFeat** m_ppTempKeyptsFeat;

	int    m_nANoUpdateTime[MAX_REGIONS];  //该稳像区域连续多次未能更新

	bool    m_bGetBkPts;

	StabTsSet    m_stabTsSet;
	StabLineSet  m_stabLineSet;

	int     m_nContFailTime;

	int          m_nOffsetWithRefCnt;
	CvPoint2D32f m_AOffSetWithRef[100];

private:
	//初始化公共变量
	void mvInitialPublicVar( );

	//计算匹配半径
	void mvCalMatchRadius( int nAreaNum );

	//保存各种时间戳
	void mvSetTimeStamp( double dTsNow );

	//对多边形进行扩张
	void mvExtendPolygon(CvPoint2D32f exPt, int nPolySize, 
						 const CvPoint2D32f *srcPts, CvPoint2D32f *dstPts );

	//角点提取和sift特征计算
	void mvCalImgKeyptsFeat( IplImage *pGrayImg, IplImage *tem1Img, IplImage *tem2Img,
		                     int &nCnt, CvPoint2D32f *APts, uchar **ppSiftDes );

	//参考帧和当前帧图像的角点提取及特征计算
	bool mvGetRefImgKeyptsFeat( IplImage *pSrcImg );
	bool mvGetCurImgKeyptsFeat( IplImage *pSrcImg );

	//当前帧图像中原始区域的角点提取和特征计算
	bool mvGetCurSrcImgKeyptsFeat( IplImage *pSrcImg );

	//将参考角点作为初始角点
	void mvSaveOrReadInitRefKeypt( );


	//将初始角点作为参考角点
	void mvReadSaveKeyptAsRef(  );

	void mvMatchAndUpdateKeyptAsRef( int *nACurPtCnt, 
						KeyPtFeat** ppCurKeyptsFeat, 
						CvPoint2D32f *ptAOffSetPt,	 
						float *fAImgMean, 
						float *fAImgSdvDev );

	//从所有的参考文本中，获取得到最好的匹配
	void mvGetBestMatchWithCur4AllRef( int &nMaxVal0, 
					int &nMaxNo1, int &nMaxVal1, 
					int &nMaxNo2, int &nMaxVal2 );

	void mvSaveCurAsSetLongRefFeat( int *nACurPtCnt,
					KeyPtFeat **ppCurKeyptsFeat, 
					CvPoint2D32f *ptAOffSetPt,
					float *fAImgMean, 
					float *fAImgSdvDev );

    void mvSetGiveAsRef( int* TempKeyptCnt, KeyPtFeat** TempKeyptsFeat, 
			float* TempImgMean, float* TempImgSdvDev );  //将给定的设为参考

	bool mvIsInitMatchBetterRef( );

	//角点群1和角点群2进行匹配
	void mvMatch2SetFeats( int nCnt1, KeyPtFeat* pFeat1,
			int nCnt2, KeyPtFeat* pFeat2,
			float fMatchR, int pMatchNo[],
			int &nMatchPair, int &nStabMatchPair,
			CvPoint2D32f &ptAvgOffset );

	//计算得到匹配后的所认为的图像位移距离
	int mvGetOffsetPtForMatch( int nRefPtCnt, KeyPtFeat *pRefPtsFeat, 
				KeyPtFeat *pCurPtsFeat, int *pMatchNo, CvPoint2D32f& offsetPt );

	//更新和更新参考角点(添加一些新角点来替换不好的参考角点)
	bool mvSaveUpdateRefKey( bool *bAMatchSucc, int *nACurPtCnt, 
				KeyPtFeat** ppCurKeyptsFeat, CvPoint2D32f *ptAOffSetPt );

	//选择当前的角点结果存为参考角点
	void mvSaveCurAsSetRefFeat( int *nACurPtCnt, KeyPtFeat** ppCurKeyptsFeat, 
			 CvPoint2D32f *ptAOffSetPt, float *fAImgMean, float *fAImgSdvDev );

	//选择当前的角点更新为参考角点
	bool mvUpdateCurAsRef( int *nACurPtCnt,	KeyPtFeat** ppCurKeyptsFeat,
						   CvPoint2D32f *ptAOffSetPt, float *fAImgMean,
						   float *fAImgSdvDev );

	//得到与初始图的偏移值
	int mvGetImgOffSet( IplImage *pSrcImg, CvPoint &offsetPt, 
						long nIndex, double ts, bool &bUpdate );

	//保存稳像结果
	bool mvSaveStatResult( double t_now, int nOffsetMod, CvPoint offsetPt );

public:
	//纠正偏移图像
	static void mvRecorrectOffsetImage(
		IplImage *pSrcImg, IplImage *pDstImg,
		int nAvgOffSetX, int nAvgOffSetY );

private:

#ifdef DEBUG_STABMATCH
	void mvShowStableMatch( int nIndex, 
		int pMatchNo[MAX_REGIONS][REGION_MAX_KEYPTCNT],
		int pTrueMatchNo[MAX_REGIONS][REGION_MAX_KEYPTCNT] );
#endif

	//选择当前的角点结果存为参考角点
	void mvChooseCurPtAsRefTxt( int nAreaNum, 
			int *nACurPtCnt, KeyPtFeat** ppCurPtFeat, 
			CvPoint2D32f *ptAOffSetPt, 
			int *nARefPtCnt, KeyPtFeat** ppRefPtFeat, 
			float *fARefMean, float *fARefSdvDev );

	//将当前的结果存为文本
	void mvSaveCurResultAsRefTxt( char *cFeatFN, int nAreaNUm,
				int *nACurPtCnt, KeyPtFeat** ppCurKeyptsFeat, 
				CvPoint2D32f *ptAOffSetPt, float fAImgMean[], 
				float fAImgSdvDev[] );

};


#endif
