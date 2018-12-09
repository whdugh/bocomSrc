/* 版权所有 2013 上海博康智能信息技术有限公司
* Copyright (c) 2013，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvImageGrids.h
* 摘要: 网格结构体的使用
* 版本: V1.0
* 作者: 贺岳平
* 完成日期: 2013年12月
*/ 
//////////////////////////////////////////////////////////////////////

#ifndef _MV_AN_IMAGE_GRIDS_H_
#define _MV_AN_IMAGE_GRIDS_H_

#include "BaseComLibHeader.h"

#include "MvMathUtility.h"
using namespace MV_MATH_UTILITY;

#ifndef SHORTTERMCNT
	#define   SHORTTERMCNT    40 	//短周期的数目
#endif
#ifndef LONGTERMCNT
	#define   LONGTERMCNT     90	//长周期的数目
#endif

#ifndef AN_GRID_LRTB
	#define AN_GRID_LRTB		
	enum _an_grid_lrtb  //左右上下
	{
		AN_LEFT =  0, AN_RIGHT, AN_TOP, AN_BOTTOM
	};
#endif

#ifndef AN_SHORTLONG_TERM_MOD
	#define AN_SHORTLONG_TERM_MOD		
	enum _an_slTermMode	//长短周期模式
	{
		AN_SHORT_TERM_MOD =  1,	
		AN_LONG_TERM_MOD	
	};
#endif

//车道方向
enum ENUM_CHANNEL_DIRECTION
{
	CHAN_ALONG_X = 0,   //车道沿x方向
	CHAN_ALONG_Y        //车道沿y方向
};

//网格显示模式
enum ENUM_GRIDSHOW_MOD
{
	SHOW_GRID_CET_PT = 0,     //网格的中心点位置
	SHOW_GRID_IMG_VELO,       //网格的图像速度
	SHOW_GRID_CETPT_IMGVELO,  //网格的中心点位置和图像速度
	SHOW_GRID_RECTANGLE       //网格的rectanle
};



//-------------------------AN_GRID_STRUCT---------------------//
//结构体在作为类的成员时，最好能在程序初始化的时候重新设置初值
//网格区域的结构
#ifndef AN_GRID_STRUCT
	#define AN_GRID_STRUCT

	//网格区域的历史基本信息结构
	typedef struct _gridAreaBasicInfo
	{
		int  nAvgGray;        //平均灰度
		int  nDiffPtPercent;  //帧差点百分比
		int  nFkPtPercent;    //前景点百分比
		int  nGradPercent;    //梯度百分比
		int  nBgGradPercent;  //背景梯度百分比
		int  nDiffGradPercent;  //前景梯度百分比

		int  nHSobelPercent;  //水平sobel百分比
	}gridAreaBasicInfo;


	//网格区域的近期相似的基本信息结构
	typedef struct _gridAreaRecSimilarInfo
	{
		int  nMinAvgGray;       //平均灰度最小值
		int  nMaxAvgGray;       //平均灰度最大值

		int  nMinFkPtPercent;   //前景百分比最小值
		int  nMaxFkPtPercent;   //前景百分比最大值

		int  nMinGradPercent;   //梯度百分比最小值
		int  nMaxGradPercent;   //梯度百分比最大值

		//int  nMinHSobelPercent;  //水平sobel百分比最小值
		//int  nMaxHSobelPercent;  //水平sobel百分比最大值

		//int  nMinVSobelPercent;  //竖直sobel百分比最小值
		//int  nMaxVSobelPercent;  //竖直sobel百分比最大值
	}gridAreaRecSimilarInfo;


	//网格区域的运动信息结构
	typedef struct _gridAreaMoveInfo
	{
		bool   bVaild;			//是否有效

		int	   nTrCnt;			//该网格区域的轨迹数目
		float  fTrAvgImgVeloX;  //该网格区域的轨迹X方向的平均速度
		float  fTrAvgImgVeloY;  //该网格区域的轨迹Y方向的平均速度	

		bool   bJam;         //是否为拥堵网格
	}gridAreaMoveInfo;


	//网格区域的点匹配信息结构
	typedef struct _gridAreaPtMatchInfo
	{
		bool   bVaild;			  //是否有效

		int	   nKeyptCnt;		  //该网格区域的角点数目
		int    nUnmatchKeyptCnt;  //该网格区域的未匹配上角点数目
		int    nMatchLTrKeyptCnt; //该网格区域的匹配上长轨迹的角点数目

		int    nHisMoveNowStopCnt;  //历史上运动现在静止的中间层点数

	}gridAreaPtMatchInfo;

	//当前网格是否为静止的判断
	typedef struct _gridStopProperty
	{
		int  nNoSuitMod1;
		int  nNoSuitMod2;

		int  nSimiWithPreCnt;   //与前面的块相似的次数

		_gridStopProperty( )
		{
			nNoSuitMod1 = 0;
			nNoSuitMod2 = 0;

			nSimiWithPreCnt = 0;
		}
	}MvGridStopProperty;

	//网格区域的结构
	typedef struct _gridArea
	{
		CvPoint  ltPt;  //左上点
		CvPoint  rbPt;  //右下点

		CvPoint  whPt;	//宽高

		int      nALRTB[4];  //左右上下4个邻近网格的序号

		MvGridStopProperty stopProperty[SHORTTERMCNT];  //当前网格是否为静止的判断

		gridAreaBasicInfo  sTermInfo[SHORTTERMCNT]; //保留最近较短时间的结果
		gridAreaBasicInfo  lTermInfo[LONGTERMCNT];  //保留最近较长时间的结果
	}gridArea;

#endif


//-------------------------AN_GRID_STOP_STRUCT---------------------//
//网格停车所用的结构
#ifndef AN_GRID_STOP_STRUCT
	#define AN_GRID_STOP_STRUCT
	//网格区域的用于停车的时间戳结构
	typedef struct _an_gridAreaStopTimeStamp
	{
		double  dTsStaStop;		//判出刚停止的时间戳
		double  dTsEndStop;		//迄今为止能判出停止的时间戳
		bool    bLightChange;   //光线是否发生变化
		double  dTsStaStopAfterLC;	//在光线发生变化后的判出刚停止的时间戳
		double  dTsEndStopAfterLC;	//在光线发生变化后的迄今为止能判出停止的时间戳

		_an_gridAreaStopTimeStamp( )
		{
			dTsStaStop = -10000;
			dTsEndStop = -10000;
			bLightChange = false;   
			dTsStaStopAfterLC = -10000;
			dTsEndStopAfterLC = -10000;
		}		
	}AnGridAreaStopTimeStamp;
#endif



//----------------网格所用的结构体-------------------//
//一个网格结构定义
typedef struct an_grid_stru
{
	CvPoint  ltPt;   //该网格的左上点
	CvPoint  rbPt;   //该网格的右下点
	CvPoint  whPt;   //该网格中心所对应的目标宽高
	int  nALRTB[4];  //左右上下4个邻近网格的序号

	//获取得到网格的大小
	CvSize mvGetGridSize( )
	{ 
		return cvSize(rbPt.x-ltPt.x+1, rbPt.y-ltPt.y+1);
	}
}AnGridStru;

enum ENUM_RECENT_JAM_STATUS  //最近拥堵的状态
{
	 MV_REC_IS_JAM = 0,     //最近为拥堵
	 MV_REC_F_G_NOSUIT,     //最近前景/梯度不满足拥堵
	 MV_REC_TR_VELO_NOSUIT  //最近轨迹/前景不满足拥堵
};

//图像网格结构定义
typedef struct an_img_grids_stru
{
public:
	//网格
	int m_nGridCnt;
	AnGridStru *m_pImgGrid;

	//网格特征
	gridAreaBasicInfo **m_pST_GridBasicFeat;  //短期
	gridAreaBasicInfo **m_pLT_GridBasicFeat;  //长期
	gridAreaMoveInfo  **m_pST_GridMoveFeat;  //短期
	gridAreaMoveInfo  **m_pLT_GridMoveFeat;  //长期
	gridAreaPtMatchInfo  **m_pST_GridPtMatchFeat;  //短期
	gridAreaPtMatchInfo  **m_pLT_GridPtMatchFeat;  //长期

	//长短周期
	CycleReplace  m_sTermCR;    //短周期的循环覆盖存储体
	CycleReplace  m_lTermCR;    //长周期的循环覆盖存储体

	CvSize  m_imgSize;

public:
	an_img_grids_stru( );

	~an_img_grids_stru( );

	void  mvInitImgGridVar( );

	void  mvInitImgGrid( CvRect roadRect, 
			IplImage *pRoadMask,
			IplImage *pSkipImg,	
			IplImage *pCarWImg, 
			IplImage *pCarHImg,
			IplImage *pXWorCoordImg, 
			IplImage *pYWorCoordImg,
			bool bSaveFeat,
			float fGridW2ObjW=0.6f, 
			float fGridH2ObjH=0.6f,
			float fGridHStep2ObjH=0.25f,
			float fGridWStep2ObjW=0.25f );

	void  mvUninitImgGrid(  );

	//---------------------------------------//

	//得到给定区域所对应的网格序号
	void  mvGetGridIdxOfGiveArea(vector<int> &vecGridIdx,
				CvPoint ltPt, CvPoint rbPt,	CvSize useSz, 
				vector<int> *pVecGiveGridIdx = NULL );

	//保存各网格区域的普通特征
	static void  mvSaveGridAreaGenFeat( 
		gridArea *pGridArea,		//网格区域
		int      nGridCnt,          //网格的总数目
		int      nSLTermMod,        //长短周期模式
		int      nTermNo,           //周期的序号
		IplImage *lpGrayInteImg,	//灰度的积分图
		IplImage *lpDiffInteImg,    //差分的积分图
		IplImage *lpFkInteImg,		//2值前景的积分图
		IplImage *lpGradInteImg,	//梯度的积分图
		IplImage *lpBgGradInteImg,	//背景梯度的积分图
		IplImage *lpDiffGradInteImg,//与背景差梯度的积分图
		IplImage *lpHDiffSobelInteImg ); //与背景差水平梯度的积分图


	//保存各网格区域的普通特征
	void  mvSaveGridAreaGenFeat( 
		AnGridStru *pGridArea,		//网格区域
		int      nGridCnt,          //网格的总数目
		gridAreaBasicInfo **ppGridBasicFeat, //网格普通特征
		int      nTermNo,           //周期的序号
		IplImage *lpGrayInteImg,	//灰度的积分图
		IplImage *lpDiffInteImg,    //差分的积分图
		IplImage *lpFkInteImg,		//2值前景的积分图
		IplImage *lpGradInteImg,	//梯度的积分图
		IplImage *lpBgGradInteImg,	//背景梯度的积分图
		IplImage *lpDiffGradInteImg,//与背景差梯度的积分图
		IplImage *lpHDiffSobelInteImg ); //与背景差水平梯度的积分图


	//保存各网格区域的从点，匹配和轨迹或获取的信息
	void  mvSaveGridAreaInfo4PointMatchTrack( 
		AnGridStru *pGridArea,		//网格区域
		int      nGridCnt,          //网格的总数目	
		int      nTermNo,           //周期的序号
		int	     nGrid2PtTrRate,	//给定积分图与网格图的大小比率
		bool	 bAddVaild,         //本帧的添加是否有效
		gridAreaMoveInfo  **ppGridMoveFeat,  //网格运动特征
		IplImage *pInteTrCntImg,    //轨迹数目积分图
		IplImage *pInteTrImgVeloX,  //轨迹的图像X速度积分图
		IplImage *pInteTrImgVeloY,  //轨迹的图像Y速度积分图		
		gridAreaPtMatchInfo  **ppGridPtMatchFeat,  //网格点匹配特征	
		IplImage *pInteAllKeyptCntImg,     //所有角点map图像积分图
		IplImage *pInteUnmatchKeyptCntImg, //未匹配上的角点map图像积分图
		IplImage *pInteMatchLTrKeyptCntImg, //匹配到长轨迹的角点map图像积分图
		IplImage *pInteHisMoveNowStopCntImg //历史运动现在静止的中间层点数积分图
	);

	//获取在给定区域内的网格序号(从给定的网格中找)
	void mvGetInAreaGrids( 
		vector<int> &vecInAreaGridIdx, //输出
		const vector<int> &vecGridIdx, //输入:给定网格序号
		int nPtCnt, CvPoint2D32f *pPts ); //输入:给定的区域
	
	//获取在给定区域内的网格序号(从所有的网格中找)
	void mvGetInAreaGrids( 
		vector<int> &vecInAreaGridIdx,    //输出
		int nPtCnt, CvPoint2D32f *pPts ); //输入:给定的区域
		
	//获取给定点的世界坐标的位置
	CvPoint2D32f mvGetWorldCoordiate( const CvPoint &pt )
	{
		CvPoint2D32f fptW = cvPoint2D32f(
		  CV_IMAGE_ELEM(m_pXWorCoordImg, float, pt.y, pt.x),
		  CV_IMAGE_ELEM(m_pYWorCoordImg, float, pt.y, pt.x) );
		return fptW;
	}

private:
	CvRect  m_roadRect;

	IplImage *m_pStandObjWImg;
	IplImage *m_pStandObjHImg;

	IplImage *m_pXWorCoordImg; 
	IplImage *m_pYWorCoordImg;

	IplImage *m_pRoadMask;
	IplImage *m_pSkipImg;

	double  m_tsNow;  //当前时间戳,单位为s
	bool    m_bDay;   //当前是否为白天

public:
	//获取得到在Y方向的标准小车大小
	static void  mvGetStdCarSzInYCoordinate( 
		vector<CvPoint> &vectPtsOfObjWh,
		CvRect roadRect, 	IplImage *pRoadMask, 
		IplImage *pObjWImg,	IplImage *pObjHImg );

	//创建目标网格
	static void  mvCreateObjectGrid( 
		CvRect roadRect, 	   IplImage *pRoadMask, 
		IplImage *pObjWImg,	   IplImage *pObjHImg,	
		int &nGridArea,		   AnGridStru **ppGrids,
		float fGridW2ObjW,	   float fGridH2ObjH,
		float fGridHStep2ObjH, float fGridWStep2ObjW );	

	//得到目标网格中各网格的上下左右四个网格的序号
	static void  mvGet4NeighbourGrid( int nGridArea, AnGridStru *pGrids );

public:
	//获取当前认为是匹配较差的目标区域
	void mvGetRecMatchBadObjAeraGrids(
			vector<int> &vecFkMvGridIdx, //运动的前景网格序号
			vector<int> &vecMBOAGridIdx, //匹配较差的网格序号 
			vector<int> *pVecGiveGridIdx = NULL );

	//获取当前认为是拥堵的网格区域序号
	void mvGetRecentJamGrids( vector<int> &vecMayJamGridIdx, 
			vector<int> *pVecGiveGridIdx = NULL,
			float fWorVeloTh = -1.0f );

	//获取当前认为是速度较慢的网格区域序号
	void mvGetRecentSlowGrids( vector<int> &vecMaySlowGridIdx, 
			double dTsNow, double dPreTime,
			vector<int> *pVecGiveGridIdx = NULL,			
			int nTrCntTh = 1,
			float fWorVeloTh = 1.0f,
			float fImgVeloTh = 10.0f );


	//获取存在过历史上运动现在静止的网格区域序号
	void mvGetExistMoveGrids( 
			vector<int> &vecExistMoveGridIdx, 
			double dTsNow, double dPreTime, 
			vector<int> *pVectGiveGridIdx );

	//获取当前认为是拥堵的网格区域序号
	void mvGetJamGrids( const vector<int> &vecMayRecJamGridIdx, 
			double dTsNow, double dJamTime, 
			vector<int> &vecJamGridIdx );

	//获取当前认为是拥堵的网格区域序号
	void mvShowGiveGrids( IplImage *pShowRgbImg,
			const vector<int> &vecGridIdx, int nShowMod,
			CvScalar color=cvScalarAll(-100),
			 int nThickness = 2 );

	//对给定的网格进行聚类
	void mvGetGridsSegment( 
			vector< vector<int> > &vectGridsIdxGroups, //网格聚类后的结果
			const vector<int> &vecGridIdx,			   //给定的网格 
			int nMod,						   //进行聚类的走向模式 
			int nMaxLen,					   //网格中心点所能达到的最大值
			float fMinGroupDist2GridSz= -1.0f  //类间的距离与网格大小的最小比率
		);

	//获取所有网格对应的区域序号
	void mvGetAreaIdxOfAllGrids( 
			vector<int> &vectGridAreaIdx, 
			int nAreaCnt,int nAAreaPtCnt[],
			CvPoint2D32f **ppAreaRgnPt );

private:
	//对给定网格判断其是否当前(短周期内)为拥堵
	bool mvJudgeGridIsRecentJam( int nGiveGridIdx, 
		   int nTermCnt, int *nATermNo, float fWorVeloTh,
		   int nTrThres = 2, float fImgVeloXTh = 3.0f,
		   float fImgVeloYTh = 3.0f, bool bEasyModel = false);

}AnImgGridsStru;



//与图像网格结构定义有关的应用函数集合
typedef struct StruImgGridAppFuncSet
{
	//判断给定网格所对应区域的图像方向-横\竖
	static bool mvGetDirectOfGrids(	 
			int &nDirAngle,       //获取得到的区域角度
			int &nAreaDirMod,     //区域的走向
			AnImgGridsStru *pImgGrid,     //图像网格
			vector<int> &vectGiveGridIdx, //给定的图像网格序号
			IplImage *pCalOriImg  //图像角度图像
		);

	//得到所给网格的按所给分布要求的网格尺寸大小
	static bool mvGetGridsSize4GiveHist(
			vector<CvSize> &vectSzGrid,       //计算出的比率的网格
			int nCalCnt, double dACalRate[],  //需计算比率的个数
			AnImgGridsStru *pImgGrid,         //图像网格指针
			vector<int> &vectGiveGridIdx      //给定的图像网格序号
		);

}AnImgGridAppFuncSet;

////////////////////////////////
#endif