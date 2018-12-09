#ifndef __AN_SMALL_VAILD_STRUCT_H

#define __AN_SMALL_VAILD_STRUCT_H

#include "libHeader.h"

#include "BaseMacro.h"
#include "DebugMacroDefine.h"

#include "DetectConfiger.h"

using namespace std;

#ifndef USE_TIME_FOR_OUTPUT
	#define USE_TIME_FOR_OUTPUT
	//耗时输出类型
	enum useTimeForOutPut
	{
		ONLY_PRINTF = 10,
		ONLY_WRITE = 20,
		PRINTF_WRITE,
	};
#endif

#ifndef AN_ILLUM_CHANGR_MODE
	#define AN_ILLUM_CHANGR_MODE
	enum _anIllumChangeMode
	{	
		AN_ILLUM_CHANGE = 0,     //光照变化了
		AN_ILLUM_NO_CHANGE ,     //光照未变化		
		AN_ILLUM_NOTSURE_CHANGE  //无法确定光照是否变化
	};
#endif


//排序所用的结构体
typedef struct _sortStruct 
{
	double  dData;
	int     nIndex;
}SortStruct;

//序号到ID的映射关系
typedef struct StruIdx2IdMap
{
	int  nIdx;  //序号
	long lId;   //ID
	StruIdx2IdMap( ) { lId = -1; }
}MvIdx2IdMap;


//结构体在作为类的成员时，最好能在程序初始化的时候重新设置初值
//---------------------时耗计算结构体---------------------//
typedef struct _AN_ComputeTime
{	
	double dFreq;
	int64  nStart;
	double time;

	_AN_ComputeTime( );	

	void initComputeTime( );	

	void setStartTime( );	

	double getUseTime( );	

	void printUseTime( char chPrintInfo[] );	//打印耗时到屏幕	

	void printAndWirte( char chDir[], 
		  char chTxtName[], char chPrintInfo[],
		  int nMod, const char chWriteMod[] );	//打印和输出到文本文件
	
}ComputeTime;

//---------------------打印信息的结构体---------------------//
typedef struct _AN_PrintfStruct
{	
	int   nFrameNo;

	_AN_PrintfStruct( );

	void setFrameNo( int _nFrameNo ); //设置帧号
	
	void myPrintf( char chPrintInfo[] ); //打印信息

}PrintfStruct;

//---------------------日期的结构体---------------------//
typedef struct _AN_DateStruct
{
public:
	int nNowYear;
	int nNowMon;
	int nNowDay;
	int nNowHour;
	int nNowMin;
	int nNowSec;

	_AN_DateStruct( );	

	void getNowDateTime( );

}AnDateStruct;


//----------------左上右下点&Rect的结构体----------------//
typedef struct StructRectLtRbPt
{
public:
	CvPoint ptLt, ptRb;
	CvRect  rect;

	StructRectLtRbPt( const CvRect &_rect );
	StructRectLtRbPt( const CvPoint &_ptLt, const CvPoint &_ptRb );	

	CvPoint		 mvGetCetPoint( );
	CvPoint2D32f mvGetCetFloatPoint( );

}MvRectLtRbPt;


#ifndef LINUX
	//-----------------读写视频序号的结构体-----------------//
	typedef struct an_video_idx_Struct
	{
	public:
		int nVideoIdx;

		an_video_idx_Struct( );	

		void initVar( );
		
		void writeVideoIdx( int _nVideoIdx );

		void readVideoIdx( );

	}AnVideoIdxStru;
#endif

//-------------------图像存储的结构体-------------------//
typedef struct _AN_ImgSaveStruct
{
private:
	double dTsLastSaveImg;
	double dTsLastSaveTxt;
	int    nSaveImgFrameNo;
	int    nSaveTxtFrameNo;
	FILE   *fp_w;

#ifndef LINUX
	int   nVideoIdx;   //视频的序号
#endif

	char  chTxtFileName[104];
public:
	_AN_ImgSaveStruct( );

	~_AN_ImgSaveStruct( );

	void initImgSaveStruct( );


	//得到距上次的保存时间
	double getTimeForLastSave( 
				double dTsNow, 
				bool bImg=true );

	//打开txt文件
	void openTxt( char *chDirName );

	//存图(返回所存图的)
	int saveImg( IplImage *pImg,     //要存的图像
				double dTsNow,		  //当前时间戳
				double dSaveInterval, //保存间隔 
				char *chCfgFileName,  //配置文件
				char *chDirName );     //存图目录

	//存文件
	int saveTxt( int nLineCnt,		  //要存的文本行数
				char **pChInfo,       //要存的文本内容
				double dTsNow,		  //当前时间戳
				double dSaveInterval, //保存间隔 
				char *chCfgFileName,  //配置文件
				char *chDirName );    //存文件目录

	//简单的存图
	static void mvEasySaveImg( 
				int &nSaveIdx,		//保存图的序号
				IplImage *pImg,     //要存的图像
				char *chDirName,    //存图目录
				int nVideoNo,       //视频序号
				int nFrameNo );     //图像序号

private:
	//根据配置文件判断：当前时间是否可以进行保存
	bool canSaveNow( char *chCfgFileName );  

}ANImgSaveStruct;


//--------------------循环覆盖存储 结构体--------------------//
#ifndef MAX_CYCLE_SIZE
	#define MAX_CYCLE_SIZE  1000
#endif
typedef struct _cycleReplace
{
	int        m_nMaxSize;
	bool       m_bFull; 
	int        m_nNow;
	int64      m_nTotalCnt;
	double     m_dTsFristAdd; //第一个元素加入的时间
	double     m_dTsAdd[MAX_CYCLE_SIZE];

	_cycleReplace( );

	void mvGiveMaxSize( int n );

	int mvAddOneElem( double dTsNow );

	//得到所有的元素(从今往前)
	void mvGetAllElem( vector<int> &vecSearchNo );

	//得到从今往前几个的元素
	void mvGetPreElem( int nPreCnt, vector<int> &vecSearchNo );

	//搜索距今时间差为给定值之内的元素(要求时间戳是增加的)
	void mvSearchInPreTimeElem( double dTsNow, 
		double dPreTime, vector<int> &vecSearchNo );

	//搜索距今时间差为给定值之内的元素(要求时间戳是增加的)
	vector<int> mvSearchInPreTimeElem( double dTsNow, double dPreTime );

	//搜索在给定的两个时间戳之间的元素(要求时间戳是增加的)
	void mvSearchIn2GiveTimeElem( double dTsStart, 
		double dTsEnd, vector<int> &vecSearchNo ); 

}CycleReplace;


//----------------光照和R,G,B的变量集结构体----------------//
typedef struct _IllumRgbVarSet
{

public:
	bool bVaild;
	//bool bBePtVaild;

	double  dIllum;    //当前所有点的亮度
	double  dBgIllum;  //背景图的亮度

	double  dR, dG, dB;				 //当前所有点的RGB值
	//double  dBgPtR, dBgPtG, dBgPtB;  //当前背景点的RGB值

	double  dBgImgR, dBgImgG, dBgImgB; //背景图像

	_IllumRgbVarSet( );

	void initVar( );

	void setVar( double _dIllum, double _dBgIllum,
		   double _dR,   double _dG,   double _dB,
		   double _dBgR, double _dBgG, double _dBgB );

	//void setVar2( double _dBgPtR, 
	//double _dBgPtG, double _dBgPtB );
	
}IllumRgbVarSet;


//--------对图像进行ROI设置并进行resize的结构体--------//
typedef struct _ImgRoiResizeStru
{
public:
	bool   m_bGet;          //是否成功获取
	CvRect m_rectRoiSrc;    //对原始图的ROI	

	float  m_fROICoff;      //roi的resize比率

	float  m_fXResizeScale; //x方向的resize比率
	float  m_fYResizeScale; //y方向的resize比率

	int	   m_nDstWidth;     //目标图的宽度
	int	   m_nDstHeight;    //目标图的高度

	_ImgRoiResizeStru( );

	void initVar( );

	//在原始图上设置ROI,并按指定大小进行resize
	void setRoiResize2SrcImg( CvRect _rectRoiSrc, 
					 int nDstImgArea = -100000 );

	//在原始图上设置ROI,并按指定放大比率进行resize
	void setRoiResize2SrcImg( CvRect _rectRoiSrc, 
							  float fROICoff );

	//根据原始图上的点坐标 得到 在目标图上的点坐标
	bool getDstPtFromSrcPt( 
			CvPoint2D32f ptScr, CvPoint2D32f &ptDst,
			bool bCtrlInImg=true //是否控制结果点在图像范围内
		);

	//根据目标图上的点坐标 得到 在原始图上的点坐标
	bool getSrcPtFromDstPt( 
			CvPoint2D32f ptDst,CvPoint2D32f &ptScr,
			bool bCtrlInImg=true //是否控制结果点在图像范围内
		);

}ImgRoiResizeStru;


//-------------------光照度结果保存结构体-------------------//
enum EnumIllumChangeMod
{ 
	ILLUM_CHANGE_NOWIMG = 0,  //用当前图像来判断
	ILLUM_CHANGE_BGPTS		  //用图像的背景点来判断
};

typedef struct _illumResultSaveStru
{
public:
	int   m_nSize;
	int   m_nAIllum[MAX_CYCLE_SIZE];   //整图的光照度
	int   m_nABgIllum[MAX_CYCLE_SIZE]; //背景图像的光照度

	bool  m_bUseBg;
	int   m_nABgPtIllum[MAX_CYCLE_SIZE];   //认为是背景点的光照度

    //注：m_nABgPtIllum中可能存在负数，表示非背景点过少，该计算无效

	int   m_nBgPtPer[MAX_CYCLE_SIZE];  //认为是背景点的百分比

	int   m_nAvgR[MAX_CYCLE_SIZE];   //R通道的平均亮度
	int   m_nAvgG[MAX_CYCLE_SIZE];   //G通道的平均亮度
	int   m_nAvgB[MAX_CYCLE_SIZE];   //B通道的平均亮度

	int   m_nAvgBgR[MAX_CYCLE_SIZE];  //背景图像R通道的平均亮度
	int   m_nAvgBgG[MAX_CYCLE_SIZE];  //背景图像G通道的平均亮度
	int   m_nAvgBgB[MAX_CYCLE_SIZE];  //背景图像B通道的平均亮度

	int   m_nAChange[MAX_CYCLE_SIZE];   //判断光照度是否变化的模式

	//光照发生变化的(未变时和刚变)时间戳
	vector<double> m_vecTsNoChangeEnd; 
	vector<double> m_vecTsTsChangeSta;

	//不能确认是否变的(开始和结束)时间戳
	vector<double> m_vecTsNoSureSta; 
	vector<double> m_vecTsNoSureEnd;

	CycleReplace m_illumResCR;

public:
	void mvInitIllumResultStru( int _nSize );

	//添加一个数据元素
	int mvAddOneDataElem( int nIllum,	//整图的光照度
					   double dTsNow ); //时间戳
	
	//添加一个数据元素
	int mvAddOneDataElem( int nIllum,    //整图的光照度
						int nBgPtIllum,  //背景点的光照度
						int nBgPercent,  //背景点的百分比
						int nAvgR,       //R通道的平均亮度
						int nAvgG,       //G通道的平均亮度
						int nAvgB,       //B通道的平均亮度
						int nAvgBgR,	 //背景图的R通道的平均亮度
						int nAvgBgG,     //背景图的G通道的平均亮度
						int nAvgBgB,     //背景图的B通道的平均亮度
						double dTsNow ); //时间戳

	//添加一个结果元素(光照度是否变化和变化的最靠近的比较时间)
	void mvAddOneResultElem( int nPointer,  //元素的下标序号
						  int nChangeMod ); //光照度变化模式
							

	//判断给定的光照是否和给定时间内的光照发生变化(返回值：
	//    0：未变化,1：变化,-1：无法确定)
	int mvIsIllumChangeWithGiveTime( 
			double dTsSta1, double dTsEnd1,   //时间段1
			double dTsSta2, double dTsEnd2 ); //时间段2

	//得到光照发生变化的时间戳
	bool mvGetIllumChangeTimeStamp(	int nPointer,			 
							 double dCheckTime ) ;    

	//得到光照是否发生变化不确定的时间戳
	bool mvGetIllumNoSureTimeStamp( int nPointer,
							 double dCheckTime ) ;

	//获取当前(距今最近)的光照度等信息
	bool mvGetNowIllumInfo( int &nIllum, //整图的光照度
						int &nBgPtIllum, //背景点的光照度
						int &nBgPtPer,   //背景点的百分比
						int &nAvgR,		 //R通道的平均亮度
						int &nAvgG,		 //R通道的平均亮度
						int &nAvgB,	     //R通道的平均亮度			
						int &nAvgBgR,    //背景图的R通道的平均亮度	
						int &nAvgBgG,    //背景图的G通道的平均亮度	
						int &nAvgBgB     //背景图的B通道的平均亮度	
						);	 

	//获取当前(距今最近)的光照度
	int mvGetNowIllumVal( )
	{	
		int nNow = m_illumResCR.m_nNow;
		if( nNow>=0 )		
		{
			return m_nAIllum[nNow];  
		}	
		return -10000;
	}

private:
	//判断给定一组光照度的值是否较为恒定
	//返回值：-1--无法判断,0--不稳定,1--稳定
	int mvIsIllumSteady( int nCnt, int nAIllum[], 
						 int &nMedianVal );

	//判断给定时间内的光照是否较为恒定
	//返回值：-1--无法判断,0--不稳定,1--稳定
	int mvIsIllumSteadyWithGiveTime(
			double dTsSta, double dTsEnd, int &nMedianVal,
			EnumIllumChangeMod nMod = ILLUM_CHANGE_NOWIMG );

}IllumResSaveStru;


//-------------------计算期望图像的结构体-------------------//
typedef struct _getExpectedImgStru
{
private:
	float fE_lum;     //期望的亮度
	float fE_stdDev;  //期望的标准差

	//给定期望的亮度和标准差
	void giveExpectLuminanceStdDev( float _fLum, float _fStdDev );

	//fN_lum-当前点的亮度, fN_W_lum-当前窗口的平均亮度, fN_W_stdDev-当前窗口的标准差 
	float calLuminance( float fN_lum, float fN_W_lum, float fN_W_stdDev );

public:
	_getExpectedImgStru( );

	//根据原始图和预期图像，求取转换后的目标图像
	void getExpectImg( IplImage *pSrcImg, IplImage *pExpImg, 
		IplImage *pDstImg, CvSize szBlock );

}GetExpectedImg;


//-------利用亮度和标准差插值的图像增强的结构体----------//
typedef struct _enhanceBlockVertPtStru
{
	CvPoint ptLoc;
	bool    bCalc;
	float   fW_MeanLum;
	float   fW_StdDev;

	_enhanceBlockVertPtStru()
	{
		bCalc = false;
	};
}EnhanceBlockVertPtStru;

typedef struct _EnhanceStru
{
public:
	_EnhanceStru( );

private:
	bool    bInit;
	CvSize  szImg;

	int     nBlockW, nBlockH;
	float   *pBlockCx, *pBlockCy;

	int     nRowBlock, nColBlock;
	EnhanceBlockVertPtStru **ppPtBlock;

	//-------------part1-----------//
	//初始化系数块大小和系数值,计算块的左上顶点值(坐标及窗口亮度和标准差)
	void init( IplImage *pSrcGImg, CvSize _szBlock );
	void uninit( );

	//计算块的双线性插值的系数值
	void mvGetBlockBICoeff(CvPoint ltPt,CvPoint rbPt, 
						  float fACx[],float fACy[]);

	//-------------part2-----------//
	//four point:0..1
	//           2..3
	void mvGetBlockBIValue( CvPoint ltPt, CvPoint rbPt, 
				  CvPoint2D32f fptABlockLumStdDev[4],
				  float fACx[], float fACy[],
				  CvPoint2D32f** pBlockBIV ) ;

	//获取标准差的修正系数
	float mvGetStdDevCoeffice( float fStadDev );

	//获取增强后的亮度值
	float mvGetEnhanceLum( float fNowLum, 
		float fWinAvgLum, float fWinStadDev );

public:
	void mvGetEnhanceImg( IplImage *pSrcGImg, 
		CvSize szBlock, IplImage *pDstGImg );	

//	//图像增强的调用示例
//#ifdef SHOW_IMAGE_ENHANCE
//	EnhanceStru enhanceOperator;
//	IplImage *pEnImg = cvCreateImage( cvGetSize(lpGrayImg), 8, 1 );
//	enhanceOperator.mvGetEnhanceImg( lpGrayImg, cvSize(48,36), pEnImg );
//	cvNamedWindow("enh1");	cvShowImage("enh1",pEnImg);
//	cvReleaseImage(&pEnImg);
//#endif

}EnhanceStru;




//事件所用的图像集合结构体(从分析类所获取)
//(这些图像的大小均一致)
typedef struct _Event4AnalysisUseImgSet
{
public:
	CvSize sz;  //图像的大小

	//这些图像为指针，不直接开劈
	IplImage *pRgbImage;	 //彩色图像
	IplImage *pGrayImg;		 //灰度图像
	IplImage *pLastGrayImg;	 //上一帧灰度图像
	IplImage *pLast2GrayImg; //上两帧灰度图像
	IplImage *pDiff256Img;   //256级灰度差分图像
	IplImage *pDiff2VImg;    //2值灰度差分图像
	IplImage *pForeMaskImg;  //扩展的前景mask图像

	IplImage *pFkLabelImg;   //前景label图像

	//这些积分图像为指针，不直接开辟
	IplImage *pIntGrayImg;			//灰度积分图
	IplImage *pIntBgGrayImg;		//灰度背景积分图
	IplImage *pIntDiff2VImg;		//差分积分图
	IplImage *pIntFore2VImg;		//2值前景积分图
	IplImage *pIntGrad2VImg;		//2值梯度积分图
	IplImage *pIntBgGrad2VImg;		//2值背景梯度积分图
	IplImage *pIntGrad2VDiffBgImg;  //2值梯度差积分图
	IplImage *pIntHSobelDiffBgImg;  //2值水平sobel差图
	IplImage *pIntVSobelDiffBgImg;  //2值竖直sobel差图

//	IplImage *pIntHSobel2VDiffBgImg;


	//这些为开劈的图像
	IplImage *pRgbBgImage;	 //彩色背景图像
	IplImage *pGrayBgImage;	 //灰度背景图像
	IplImage *pFore2VImg;    //2值前景图像

public:
	_Event4AnalysisUseImgSet( );	

	void initVar( );

	void mvSetImgWH( int nW, int nH );

	bool createImages( );

	void releaseImgSet( );

}Event4AnalysisUseImgSet;


//事件所用的灰度图像集合结构体
//(这些图像的大小均一致)
typedef struct _EventUseGrayImgSet
{
public:
	CvSize sz;  //图像的大小

	IplImage *grayImage;       //灰度图像
	IplImage *m_befAdjGrayImage;   //未进行拉伸前的灰度图像

	IplImage *lastImage;       //上一帧的灰度图像
	IplImage *last2Image;      //上两帧的灰度图像

	IplImage *m_pFkSobelImg;   //前景sobel图像

//	IplImage *diff256Image;     //256级灰度差分图像
//	IplImage *diffImage;        //2值灰度差分图像

	IplImage *m_pMaskImage;		//mask图像
	IplImage *m_pRoadMaskImage;	//道路区域mask图像

	IplImage *oriImage;         //方向图像
	IplImage *oriImageEx;		//方向扩展图像

	IplImage *skipimage;		//忽略区域mask图像
	IplImage *m_pChannelIDImg;  //车道Id图像

	IplImage *m_pIsPeopleRoadImg; //是否为行人道路图像
public:
	_EventUseGrayImgSet( );	

	void initVar( );

	void mvSetImgWH( int nW, int nH );

	bool createImages( );

	void releaseImgSet( );

}EventUseGrayImgSet;


//扩展的直方图应用结构体
typedef struct StruExtendHistApp
{
public:
	StruExtendHistApp( );	
	StruExtendHistApp( int nBin, double dMin, double dMax );
	~StruExtendHistApp( );

	//将直方图内的值全设为0
	void mvResetHistToZero( );

	//添加一个元素
	void mvAddElem( double dV );

	//获取直方图的分布概率
	void mvGetHistDist( );
	
	//获取满足分布概率要求的直方图的位置
	bool mvGetSuitDistPro( double &dVSuit, double dProRequire );
	
private:
	bool   m_bInit;

	int    m_nBin;          //划分的数目
	double m_dMin, m_dMax;  //最小值和最大值
	
	double m_dStep;         //每份的取值

	int    *m_pHist;

	int    m_nSumCnt;
	int    *m_pHistDist;

	void mvInitVar( );
	void mvUninit( );

}ExtendHistApp;


//图像的灰度直方图(主要思想是对一幅灰度图像进行遍历，将各级灰度存入
//   不同级别图像中，再对各级别图像进行积分，这样对给定区域，取得该
//   区域的各灰度级别的积分图像所对应的积分值，进行归一化就得到得到
//   其该区域的直方图)
#define MAX_HIST_IMG_LEVEL_CNT 32
typedef struct StruImageHistogram
{
public:
	void mvInitVar( )
	{
		m_nLevelCnt = 0;
		for( int n=0; n<MAX_HIST_IMG_LEVEL_CNT; n++ )
		{
			m_pALevelImgs[n] = NULL;
			m_pAInteLevelImgs[n] = NULL;
		}
	}

private:
	int		  m_nLevelCnt;
	IplImage* m_pALevelImgs[MAX_HIST_IMG_LEVEL_CNT];
	IplImage* m_pAInteLevelImgs[MAX_HIST_IMG_LEVEL_CNT];

}MvImageHistogram;

//将彩色像素点转换为灰度像素点的结构体
typedef struct StruRgb2GrayPixels
{
public:
	StruRgb2GrayPixels( );

	//获取彩色点的灰度值
	uchar mvGetGrayValueOfColorPt( uchar ucVal[3], bool bBGR );

private:	
	int  m_tab[256][3];
	
}PtsRgb2Gray;


//获取环型点的积分图及其应用
typedef struct StruRingPtsIntegAndApp
{
public:
	~StruRingPtsIntegAndApp( );
	StruRingPtsIntegAndApp( );
	
	//获取环型点的积分图(必须先执行,后面才能获取)
	void mvIntegral( int nPtCnt, int *nAPtVal );

	//获取环型点半径范围内所对应的点
	int mvGetSumValue4Radius( const int nPtIdx, const int nRadius ) const;

	//获取环型点半径范围内所对应的点
	int mvGetSumValue4Range( const int nPtIdxSta, const int nPtIdxEnd ) const;

private:
	void mvInitVar( );
	void mvFreeMalloc( );

private:
	int m_nPtCnt;
	int m_nHalfCnt;
	int m_nIntegralCnt;
	
	int *m_nAPtsExtend;
	int *m_nAPtsIntegral;

}RingPtsIntegApp;


//投影的应用
typedef struct StruProjectApp
{
	//获取得到投影数组的往前/后扩展的端点
	static bool mvGetExtendPt( int &nExtStart, int &nExtEnd,
				  int *nAProjectPtNum, int nStart, int nEnd,
				  int nMaxLen, int nCntThres, int nLongThres );

	//获取局部峰值所对应的位置
	vector<int> mvGetLocalPeakLocation( int *nAProjectPtNum, 
		  int nStart, int nEnd, int nRadius, int nLocakPeakTh );

	//通过位置关系来对局部峰值进行过滤 
	vector<int> mvFiltrateLocalPeaks( const vector<int> &vectPeak,
		  int *nAProjectPtNum, int nLocationDistTh );


}MvProjectApp;


//kalman预测器
typedef struct StruKalmanApp
{ 
public:
	StruKalmanApp( )
	{
		m_bInit = false; 
	}

	void mvInitKalman( );
	void mvUninitKalman( );

	bool mvKalmanTrack4GivePts(
			vector<CvPoint2D32f> &vectPtsPredict,
			int nPts, CvPoint2D32f ptA[], double dATs[] );

	bool mvKalmanPredict4GivePts( 
			vector<CvPoint2D32f> &vectPtsPredict,
			int nPts, CvPoint2D32f ptA[],
			int nPredictFrameCnt );

private:
	bool m_bInit;

	int m_stateNum;
	int m_measureNum;

	CvKalman* m_cvkalman;

	CvMat* m_process_noise;
	CvMat* m_measurement;

}MvKalmanApp;


//最小二乘法的多项式拟合应用
typedef struct StruMultinomialFitApp
{ 
public:
	StruMultinomialFitApp( ){ };

	void  mvEMatrix(double *dx, double*dy, int n, 
					int ex, double coefficient[]);

	//根据计算得到的多项式的系数，来预测给定的自变量的值
	float mvPredict( float &fVal, double ts, 
			int nMultinomial, double dACoef4Ts[] );

private:
	double mvSum(double * dNumarry,int n);
	double mvMutilSum(double* dX,double *dY,int n);
	double mvRelatePow(double *dx,int n,int ex);
	double mvRelateMutiXY(double *dx,double*dy,int n,int ex);
	void   mvCalEquation(int exp,double coefficient[] );
	double mvF(double c[],int l,int m);

	double m_Em[10][10];

}MvMultinomialFitApp;


//---------------------------------------
#endif  //#ifndef __AN_SMALL_VAILD_STRUCT_H
