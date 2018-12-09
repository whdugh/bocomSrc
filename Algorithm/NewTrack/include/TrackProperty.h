#ifndef __TRACK_PROPERTY_H
#define __TRACK_PROPERTY_H

#include "libHeader.h"

#include "sift_descr.h"
#include "MvSiftDescr.h"
#include "MvKeyPtExtract.h"

#include "BaseStruct.h"

#include "comHeader.h"  //放最后

using namespace std;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_AN_ANLGORITHM;
using namespace MV_SMALL_FUNCTION;

#define PT_SQUARES(a,b) ((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y))

//认为为停止轨迹的阈值集
typedef struct StructStopTrackThresholdSet
{
	int    nTrCalcLenTh;    //轨迹的最小记录次数要求
	double dPassTimeTh;     //轨迹的最小时间跨度要求 
	int    nImgMoveDistTh;  //轨迹的最大图像位移要求 

	int  nMatchRadius4ML;  //在中间层时匹配的半径
	int  nSaveRadius4ML;   //在中间层时保存的半径

	StructStopTrackThresholdSet()
	{
		nTrCalcLenTh = 10;
		dPassTimeTh = 3.0;
		nImgMoveDistTh = 5;

		nMatchRadius4ML = 3; 
		nSaveRadius4ML = 2;   
	}
}AnStTrThSet;


//角点加入到网格的模式
enum ENUM_KEYPT_ADD2GRID_MODE
{
	ADD_ALL_PTS_TOGRID = 0,  //所有的角点
	ADD_NOMATCH_PTS_TOGRID,  //未匹配上的角点
	ADD_MATCH_LTR_PTS_TOGRID //匹配到长轨迹上的角点
};

//涉及到当前帧信息
typedef struct _AN_FRAME_INFO
{
	int nIndex;			//帧号
	int ts;				//时间戳(以毫秒为单位)
	int bDay;           //是否为白天

	_AN_FRAME_INFO()
	{
		initVar( );
	}

	void initVar( )
	{ 
		nIndex	= -1;
		ts		= -1;	
		bDay = true;
	}
}AnFrameInfo;


//全局的参数
typedef struct _AN_TR_GLOBAL_PARA
{
	AnFrameInfo m_FrameInfo;

	int m_nPointsCount;
	CvPoint2D32f m_pPoints[MAX_SIFT_CORNERS];
	uchar m_pFeature[MAX_SIFT_CORNERS][SIFT_DIMS];	//和上述角点一一对应

	_AN_TR_GLOBAL_PARA()
	{
		m_nPointsCount	= 0;
	}

}AnTrGlobalPara;


typedef struct _AN_TRACK_POINT
{
	CvPoint corner;

	int ts;			//时间戳，精确到毫秒
	int nStatus;	//0是估计，1是确切

	_AN_TRACK_POINT()
	{
		ts = -1;
		nStatus = 0;
	}

}AnTrackPoint;

typedef struct _AN_POINT_TRACK
{
	int nTrackId;	//轨迹编号，唯一
	uchar feature[SIFT_DIMS];

	vector<AnTrackPoint> vecTrackPoint;

	CvPoint ptPredict;

	int nEstTimes;

	_AN_POINT_TRACK( )
	{
		nTrackId = -1;
		nEstTimes = 0;
	}

}AnPtTrack;

typedef struct _MV_OPTIFLOW_PT
{
	int     nTrackNo;	//对应的轨迹序号
	CvPoint ptTrPt;

	int     nPtNo;      //对应的角点序号
	CvPoint ptMatchPt;

	_MV_OPTIFLOW_PT( )
	{

	}

	_MV_OPTIFLOW_PT( int n1,CvPoint pt1, int n2, CvPoint pt2 )
	{
		nTrackNo = n1;	
		ptTrPt = pt1;

		nPtNo = n2;    
		ptMatchPt = pt2;
	}	

}MvOptiFlowPt;


//-----------获取点由近到远的点序号的结构体------------//
typedef struct StruPointsNear2FarSort
{
public:
	bool m_bVaildNow;            //当前是否有效

	int m_nCnt;					//获取的点数目
	int *m_nACorIdx;			//对应的点序号
	int *m_nACorSortCnt;		//与给定点按距离排序的点数
	IplImage *m_pCorSortIdxImg; //与给定点按距离排序的点序号图像

	int m_nMaxPtCnt;		    //最多的点数
	int *m_nAAllCor2Sort;		//所有的 与 进行排序 对应关系

public:
	StruPointsNear2FarSort( );

	void mvInitVar( );   
	void mvUninit( );

	void mvInitPointsNear2FarSort(int nMaxPtCnt);
	void mvUninitPointsNear2FarSort( );

	//获取当前的排序结果
	void mvGetNowSortResult( int _nCnt,	//获取的点数目
			int *_nACorIdx,				//对应的点序号
			int *_nACorSortCnt,			//与给定点按距离排序的点数
			IplImage *_pCorSortIdxImg   //与给定点按距离排序的点序号图像
		);

	//得到给定序号所对应的排序结果
	bool mvGetN2FResultOfGiveIdx( int nIdx,
			int &nCnt, int **ppCorIdx );

}PtsNear2FarSort;


//-----------区域内的点轨迹信息获取和存储的结构体------------//
//网格图像与点轨迹图像的宽高比率
#ifndef AN_GRID_PTTR_RATE
	#define AN_GRID_PTTR_RATE 2
#endif
typedef struct _StruAreaPointTrackInfo
{
public:
	bool m_bInit;          //是否完成初始化
	bool m_bAddThisFrame;  //本帧是否加入信息

	int  m_nGrid2PtTrRate; //网格和所用到的点轨迹图像的宽高之比
	CvSize  m_szImg;	   //所用图像大小(为减少计算量，为实验图的n分之一)

	//运动速度
	IplImage *m_pInteTrImgVeloX;  //轨迹的图像X速度积分图
	IplImage *m_pInteTrImgVeloY;  //轨迹的图像Y速度积分图

	IplImage *m_pInteTrCntImg;    //轨迹数目积分图

	//角点匹配
	IplImage *m_pInteAllKeyptCntImg;      //所有角点map图像积分图
	IplImage *m_pInteUnmatchKeyptCntImg;  //未匹配上的角点map图像积分图
	IplImage *m_pInteMatchLTrKeyptCntImg; //匹配到长轨迹的角点map图像积分图

	//传进来的图
	IplImage *m_pTrImgMaxMvD;  //轨迹最多位移的图像(只是个指针，不开辟)

public:
	_StruAreaPointTrackInfo( )
	{
		initVar( );
	}

	void initVar( );

	//初始化区域点轨迹信息结构体
	void mvInitAreaPointTrackInfo( int nGrid2PtTrRate, CvSize sz );

	//释放区域点轨迹信息结构体
	void mvUninitAreaPointTrackInfo( );

	//清除结果图像
	void mvClearResultImage( );

	//产生结果图像的积分图
	void mvBuildIntegalImage( );

	//加入一条轨迹信息
	void mvAddOnePointTrackInfo( 
			const CvPoint &ptTr,            //轨迹当前点的图像位置
			const CvPoint2D32f &fptImgVelo, //轨迹当前的图像速度
			const CvPoint2D32f &fptLtMv,    //轨迹的总体运动矩形左上点
			const CvPoint2D32f &fptRbMv     //轨迹的总体运动矩形右下点
		 );

	//加入一个角点及其匹配信息
	void mvAddOnePointMatchInfo( CvPoint ptTr,	int nPtAddToGridMode );

private:
	//运动速度
	IplImage *m_pTrImgVeloX;  //轨迹的图像X速度
	IplImage *m_pTrImgVeloY;  //轨迹的图像Y速度

	IplImage *m_pTrCntImg;    //轨迹数目

	//角点匹配
	IplImage *m_pAllKeyptCntImg;      //所有角点map图像
	IplImage *m_pUnmatchKeyptCntImg;  //未匹配上的角点map图像
	IplImage *m_pMatchLTrKeyptCntImg; //匹配到长轨迹的角点map图像

}StruAreaPointTrackInfo, AreaPtTrInfo;


//点轨迹属性结构体
typedef struct AN_POINT_TRACK_PROPERTY
{
public:
	AN_POINT_TRACK_PROPERTY( )
	{
		initVar( );
	}
	
	//初始化变量
	void initVar( );
		
	//初始化点轨迹属性结构体
	void mvInitPointTrackProperty( );

	//释放点轨迹属性结构体
	void mvUnnitPointTrackProperty( );

	//获取点轨迹属性结构体
	void mvGetPtTrBasicProperty( 
			double dTsNow,      //当前的时间戳
			int nNoNullTrNum,	  //非空轨迹数目
			int *nANoNullTrIdx,   //非空轨迹所对应的序号
			MyTrackElem **pATrPointer, //轨迹对应的地址(指针)
			int nImgW, int nImgH,  //图像的宽度和高度
			IplImage *pCarWImg,	   //车辆宽度maxsize图像
			IplImage *pCarHImg,	   //车辆高度maxsize图像
			IplImage *pGrayImg,	   //当前灰度图
			PtsNear2FarSort &struTrPtsN2FResult  //轨迹点由近到远排序结果
		);

	//获取点轨迹属性结构体
	void mvGetPointTrackProperty(
			float fAhomography_w2i[3][3],  //世界到图像坐标的Homography   
			float fAhomography_i2w[3][3],  //图像到世界坐标的Homography
			IplImage *pCalOriImage,  //所给图像坐标方向图像
			IplImage *pVaildOriImg,        //所给图像点的坐标方向是否有效
			PtsNear2FarSort &trPtsN2FResult  //轨迹点由近到远的排序结果图
		);
public:
	float *m_fARealWorldOri;  //轨迹的世界方向(弧度)
	float *m_fARealImageOri;  //轨迹的图像方向(弧度)
	int   *m_nATrAgainst;     //轨迹是否为逆行
	int   *m_nATrCross;       //轨迹是否为横穿
	
	//提供的一些对外的静态函数(暂时为静态，后面需要将其改为私有)
public:

	//获取点轨迹的运动状态属性
	static bool mvGetPtTrMoveStatus( MyTrack &tr, 
			int nCalcLenTh,      //轨迹参入计算的长度阈值
			double dCalcTimeTh,  //轨迹参入计算的时间阈值
			CvPoint2D32f &fptImgVelo  //轨迹的图像速度
		);

private:
	bool   m_bInit;        //是否初始化过

	double m_dTsNow;         //当前时间戳
	double m_dTsLast1Frame;  //上一帧的时间戳
	double m_dTsLast2Frame;  //上两帧的时间戳

	int  m_nNoNullTrNum;    //非空轨迹数目
	int  *m_nANoNullTrIdx;  //非空轨迹所对应的序号
	MyTrackElem **m_pATrPointer;   //轨迹对应的地址(指针)

	int   m_nImgWidth;     //图像宽度
	int   m_nImgHeight;	   //图像高度
	IplImage *m_pCarWImg;  //图像宽度maxsize图像
	IplImage *m_pCarHImg;  //图像高度maxsize图像
	IplImage *m_pGrayImg;  //当前的灰度图像

private:

	//------获取点轨迹由近到远的轨迹序号
	bool mvGetNear2FarTracks( 
			int &nTrCnt,		//获取的轨迹数目
			int **nACorTrIdx,   //轨迹对应的序号
			int **nACorSortCnt,	//与给定轨迹按距离排序的轨迹数
			IplImage **pCorSortIdxImg   //与给定轨迹按距离排序的轨迹序号图像
		);

	//显示点轨迹由近到远的轨迹序号的结果
	void mvShowNear2FarResult( 
			int nTrCnt,		//获取的轨迹数目
			int *nACorTrIdx,   //轨迹对应的序号
			int *nACorSortCnt,    //与给定轨迹按距离排序的轨迹数
			IplImage *pCorSortIdxImg  //与给定轨迹按距离排序的轨迹序号图像
		);


	//-------判断轨迹是否为特殊(与周围轨迹情况不一致)
	void mvIsDifferentTrack( 
			int nTrCnt,		   //获取的轨迹数目
			int *nACorTrIdx,   //轨迹对应的序号
			int *nACorSortCnt,    //与给定轨迹按距离排序的轨迹数
			IplImage *pCorSortIdxImg  //与给定轨迹按距离排序的轨迹序号图像
		);
		
	//-------轨迹运动一致性判断
	bool mvGetMotionConsistency( MyTrack& tr );

	//-------获取得到方向为异常的轨迹
	void mvGetDirUnusualTracks(
			float fAhomography_w2i[3][3],  //世界到图像坐标的Homography   
			float fAhomography_i2w[3][3],  //图像到世界坐标的Homography
			IplImage *pCalOriImage,		   //所给图像坐标方向图像
			IplImage *pVaildOriImg         //所给图像点的坐标方向是否有效
		);

	//轨迹运动方向性判断
#ifdef DEBUG_SHOW_AGAINST_CROSS_TR
	float m_fRoadImgOri; //道路方向弧度
	float m_fMoveAngle1, m_fMoveAngle2; //顺行弧度
	float m_fAgainstAngle1, m_fAgainstAngle2; //逆行弧度	

	//显示非正常轨迹
	void mvShowUnusualTracks( 
			const CvPoint2D32f &fpt, float fRealImgOri,
			bool bGet, bool bAgainst, bool bCross,
			IplImage *pOriImg, IplImage *pRgbImg    
		);
#endif

	//判断轨迹运动方向是否为正常
	bool mvIsTrackDirUnusual( CvPoint2D32f point,  //轨迹的当前点坐标
			float fRealImgOri,       //轨迹的当前运动图像方向
			float fAhomography_w2i[3][3],  //世界到图像坐标的Homography   
			float fAhomography_i2w[3][3],  //图像到世界坐标的Homography   
			IplImage *oriImage,      //所给图像坐标方向图像
			IplImage *pVaildOriImg,  //所给图像方向点是否有效
			bool &bAgainst,			 //逆行
			bool &bCross			 //横穿
		);


	//-------获取得到轨迹的总的运动区域
	void mvGetMoveAreaOfTracks( );

	//-------对估计点的图像坐标进行修正
	void mvRecorrectCoordOfEstPt(
			float fAhomography_i2w[3][3]  //图像到世界坐标的Homography
		);

	//-------对点的世界坐标进行平滑
	void mvSmoothWorldCoord(
			float fAhomography_i2w[3][3]  //图像到世界坐标的Homography
		);

	//-------对轨迹计算其世界速度
	void mvCalcWorldVelocity( );

	//-------计算轨迹的方向
	void mvCalcOrientation( );

	//计算轨迹的当前图像方向角度(弧度)
	bool mvCalcRecentImgDir( MyTrack &tr, int nRecTrLen );

	//计算轨迹的准确图像和世界方向角度(弧度)
	bool mvGetRealTrackWorImgDir(		
		float &fRealWorldori,  //计算出的世界坐标角度
		float &fRealImageOri,  //计算出的图像坐标角度
		MyTrack &tr,     //轨迹
		double dTsNow,    //当前的时间戳
		double dEstshift, //往前追溯的时间
		float fCalDistTh  //距离计算的阈值
		);


	//-------获取得到轨迹中一些消失点的信息
	void mvGetDisappearPtInfo( );

	//-------获取得到由动到静的轨迹
	void mvGetMove2StopTracks(  );

	
	//利用已有的由动到静的轨迹来得到由动到静目标的静止轨迹
	void mvGetRecStop4M2STr( 	
			int nMv2StopTrNum,   //由动到静的轨迹数目
			IplImage *pMapImg,	 //由动到静的轨迹map图
			int nGray2MapRate,   //灰度图与map图的大小比
			int nAroundM2SPtTh,  //周围的由动到静的点个数
			IplImage *pShowImg   //结果显示图
		);

	//判断轨迹最近是否为由动到静
	void mvIsTrackRecentMove2Stop(
			vector<int> &vectTrIdx,
			int nRecLenThresh = 10,
			IplImage *pShowImg = NULL 
		);

}StruPtTrProperty;


#ifdef TEST_SIFT_THROD
    //SIFT匹配上的点的属性结构体
	typedef struct _MATCH_TRACK_POINT
	{
		CvPoint2D32f pointtracked;  //匹配上点得坐标
		double dMinDist;		    //匹配上点的最小距离
		float fMin2SecRate;			//匹配上点最小距离与次小距离的比率
		_MATCH_TRACK_POINT()
		{
			pointtracked.x = 0.0f;
			pointtracked.y = 0.0f;
			dMinDist = 1.2e7;
			fMin2SecRate = 1.0f;
		}
	}Matchtrackpoint;
#endif


#endif