//目标检测
#ifndef _MV_DETECTOR_H_
#define _MV_DETECTOR_H_

#include "libHeader.h"
#include "comHeader.h"  

#include "MatchTrack.h"
#include "ConstructGroup.h"

//class CDetector
//{
//public:
//	CDetector(void);
//	~CDetector(void);
//
//	//获取得到目标的大小图像
//	void mvGetObjSzImg( IplImage *pHImg, IplImage *pWImg )
//	{
//		m_pObjHImg = pHImg,	m_pObjWImg = pWImg;
//	};
//
//	////获取得到当前的信息
//	//void mvGetNowInfo( int dTs, 
//	//		map<int, AnPtTrack> *p_mapTrack, 
//	//		vector<AnPtTrack> *p_vecTrack, 
//	//		vector<MvGroup> *p_vecGroup, 
//	//		vector<MvSimpleObject> *p_vecObject )
//	//{
//	//	m_tsNow = dTs;
//	//	m_p_mapTrack = p_mapTrack; 
//	//	m_p_vecTrack = p_vecTrack;
//
//	//	m_p_vecGroup = p_vecGroup;
//
//	//	m_p_vecObject = p_vecObject;
//
//	//	m_bGetInfo = true;
//	//};
//
//public:
//	bool m_bGetInfo;
//
//	int m_tsNow;
//
//	//map<int, AnPtTrack> *m_p_mapTrack;   //trackID 和track对应关系
//	//vector<AnPtTrack> *m_p_vecTrack;	   //存放track
//
//	//vector<MvGroup> *m_p_vecGroup;	   //存放group
//
//	//vector<MvSimpleObject> *m_p_vecObject;   //存放object
//	
//	IplImage *m_pObjHImg, *m_pObjWImg;
//};


//----------------------StruTrackUtility-------------------------//

typedef struct StruTrackUtility
{
	//计算轨迹的图像运动角度
	static bool mvCalTrValidMvImgAngle( float &fTrMvImgAngle,
			MyTrack &tr, double dTsNow, float fTrMvDistThres );

	//计算轨迹的图像运动角度
	static bool mvCalTrValidMvImgAngle( float &fTrMvImgAngle,
			const simpleTrInfo &simTr, float fTrMvDistThres );

}MvTrackUtility;


//----------------------StruObjectUtility-------------------------//

typedef struct StruObjectUtility
{
	//获取目标中有效的轨迹(目前指观测)的序号
	static int mvGetVaildTrsOfObj( int *nAVaildTrIdx,
				 MyGroup &obj, simpleTrInfo ASimTrs[] );

	//计算目标轨迹的最/次大位移
	static void mvGetObjectTracksDist( 
		MyGroup &obj, simpleTrInfo ASimTrs[], 
		float &fMaxDist, float &fSecMaxDist );

	//计算最近nCalcLen帧内,目标轨迹的最/次大位移及平均位移
	static bool mvGetObjectTracksDist4FixTrLen( 
		MyGroup &obj, simpleTrInfo ASimTrs[], int nCalcLen,
		float &fMaxDist, float &fMaxDist2, float &fMeanDist );

	//目标中轨迹的观察次数的最大次数，及所对应的轨迹的点数
	static void mvGetMaxWatchTimeOfObjTrack( 
		MyGroup &obj, simpleTrInfo ASimTrs[], 
		int &nMaxWatchTime, int &nCorTrPtCnt );

	//获取目标中最好的3条轨迹(也可能少于3条)的序号
	static void mvGetBest3TrsOfObj( 
		int &nBestTrNum, int nABestTrIdx[3], 
		MyGroup &obj, simpleTrInfo ASimTrs[] );	

	//利用给定的轨迹来获取目标的有效图像运动方向
	static bool mvGetVaildImgMvAngleWithGiveTrs( 
		float &fObjMvImgAngle, int nTrCnt, int *nATrIdx,
		simpleTrInfo *pSimTrs, float fDistThres );

}MvObjectUtility;


//---------------------------------------------------------------//
//目标的位置基本信息
typedef struct StruObjLocationBasicInfo
{
	StruObjLocationBasicInfo( ) { mvInitVar( ); }

	double   m_dTs;

	CvPoint	 m_ptObjCet;
	CvPoint  m_ptObjLtPt;
	CvPoint  m_ptObjRbPt;

	void mvInitVar( );

	//获取目标的位置基本信息
	void mvGetInfo( MyGroup &obj, double dTsNow );

}MvObjLocationBasicInfo;


//目标关于maxsize的基本信息
typedef struct StruObjMaxsizeBasicInfo
{
	StruObjMaxsizeBasicInfo( ) { mvInitVar( ); }

	double   m_dTs;

	CvSize  m_szCar;
	CvSize  m_szPeo;

	CvSize  m_szCar4MvAngle;

	void mvInitVar( );

}MvObjMaxsizeBasicInfo;


//目标的轨迹基本信息(点轨迹和质心轨迹)
#define OBJINFO_TRDIST_CALCLEN 5   //目标信息中轨迹位移计算的帧数
#define OBJ_BEST_TRACK_CNT 3	   //目标中最好的几条轨迹
typedef struct StruObjTrackBasicInfo
{
	StruObjTrackBasicInfo( ) { mvInitVar( ); }

	double   m_dTs;

	double 	 m_dTsAdd;
	int      m_nHistoryLen;
	
	int		 m_nTracksCnt;
	int		 m_nVaildTracksCnt;
	
	float    m_fTrMaxDist;
	float    m_fTrSecMaxDist;

	int		 m_nTrCalcLen;
	float    m_fMaxDist4CalcLen;
	float    m_fSecMaxDist4CalcLen;
	float    m_fMeanDist4CalcLen;

	int		 m_nTrMaxWatchTime, m_nCorTrPtCnt;

	int      m_nBestTrCnt;
	int      m_nABestTrIdx[OBJ_BEST_TRACK_CNT];

	void  mvInitVar( );

	void  mvGetInfo(MyGroup &obj, simpleTrInfo ASimTrs[], double dTsNow);

}MvObjTrackBasicInfo;


//目标的角度基本信息
typedef struct StruObjAngleBasicInfo
{
	StruObjAngleBasicInfo( ) { mvInitVar( );	}

	double   m_dTs;

	float m_fImgMvAngle;	 //运动角度(度)
	float m_fImgArc4RoadOri; //所对应的道路弧度(弧度)

	void mvInitVar( );

}MvObjAngleBasicInfo;


//目标的速度基本信息
typedef struct StruObjVelocityBasicInfo
{
	StruObjVelocityBasicInfo( ) { mvInitVar( );	}

	double   m_dTs;

	CvPoint2D32f m_fptWroVelocity;  //世界速度

	void mvInitVar( );

}MvObjVelocityBasicInfo;


//区域的积分结果
typedef struct StruAreaInteResult
{	
	float fGrayVal;		//区域当前灰度值
	float fBgGrayVal;	//区域背景灰度值
	float fDiffPtRate;  //区域差分点比率值
	float fForePtRate;  //区域前景点比率值
	float fHSobDiffPtRate;  //区域水平sobel点(减掉背景)比率值
	float fVSobDiffPtRate;  //区域竖直sobel点(减掉背景)比率值

}MvAreaInteResult;

//区域的多尺度结构体
typedef struct StruMultiScaleArea
{
	int    m_nLevelCnt;  //尺度数(不能多于10个)
	float  m_fRate;      //相邻尺度比率所对应的系数
	
	float  m_fAScale[10];   //各尺度的值

	double   m_dTsCalcAreaLoc;	//计算的区域位置的时间戳
	double   m_dTsCalcAreaInte;	//计算的区域积分的时间戳

	CvPoint  m_ptAAreaLt[10];  //区域左上点
	CvPoint  m_ptAAreaRb[10];  //区域右下点

	MvAreaInteResult m_areaInteResA[10];

	StruMultiScaleArea( );

	void mvInitVar( );

	//设置区域的多个尺度空间
	void mvSetScaleValue( );
	void mvSetScaleValue( int nLevelCnt, 
		    float fRate, float fScale0 );

	//获取与给定尺度最靠近的指标
	int mvGetClosestScaleIdx( float fScale );

	//获取第一个比与给定尺度大的指标
	int mvGetSmallestIdxBigGiveScale( float fScale );

	//设置各个区域的所对应的多个尺度区域
	bool mvSetObjMultiScaleAreaValue(
		    double dTsNow,           //当前的时间戳
			const MyGroup &obj,		 //给定的目标
			const CvSize &szStd,	 //标准大小，即对应尺度为1
			const CvSize &szImg,	 //图像的大小
			bool bCalcObjLargeArea   //是否为计算目标的大区域 
		);

	//设置各个区域的所对应的多个尺度区域
	bool mvSetObjMultiScaleAreaValue(   
			double dTsNow,            //当前的时间戳
			CvPoint2D32f &fptAreaLt,  //给定的区域的左上点
			CvPoint2D32f &fptAreaRb,  //给定的区域的右下点
			const CvSize &szStd,	  //标准大小，即对应尺度为1
			const CvSize &szImg,	  //图像的大小
			bool bCalcObjLargeArea    //是否为计算目标的大区域 
		);

	//计算多个尺度区域的积分值
	void mvGetInteImgValueOfAreaMultiScale( double dTsNow,
			IplImage *pIntGrayImg, IplImage *pIntBgGrayImg, 
			IplImage *pIntDiff2VImg, IplImage *pIntFore2VImg,
			IplImage *pIntHSobDiffImg, IplImage *pIntVSobDiffImg
		);

}MvMultiScaleArea;


//目标关于区域的基本信息
typedef struct StruObjAreaBasicInfo
{
	StruObjAreaBasicInfo( ) { mvInitVar( ); }

	double   m_dTs;

	MvMultiScaleArea  m_area4MSPeo;  //多尺度的行人区域
	MvMultiScaleArea  m_area4MSCar;  //多尺度的车辆区域

	MvMultiScaleArea  m_area4MSLargePeo;  //多尺度的大行人区域
	MvMultiScaleArea  m_area4MSLargeCar;  //多尺度的大车辆区域


	bool     m_bVaildImgA;
	CvPoint  m_ptImgACarLt, m_ptImgACarRb; //带角度方向的车辆区域
	CvPoint  m_ptLargeImgACarLt, m_ptLargeImgACarRb; 
	

	CvPoint  m_ptVehBotHalfLt, m_ptVehBotHalfRb; //车下半部

	void mvInitVar( );

	//目标关于区域的位置基本信息
	void mvGetObjAreaLocInfo( const MyGroup &obj,
			const CvSize &szPeo, const CvSize &szCar,
			const CvSize &szImgACar, const CvSize &szImg,
			double dTsNow );

	//目标关于区域的积分图基本信息
	void mvGetObjAreaInteInfo( const MyGroup &obj, double dTsNow,
			IplImage *pIntGrayImg, IplImage *pIntBgGrayImg, 
			IplImage *pIntDiff2VImg, IplImage *pIntFore2VImg,
			IplImage *pIntHSobDiffImg, IplImage *pIntVSobDiffImg );

}MvObjAreaBasicInfo;
	

//HoG行人判断的结果
typedef struct StruHoGPeoDetRes
{
	int  m_nHoGMode;    //所采用的HoG模型的模式

	CvRect  m_rectBestPeoDetArea;  //进行HoG行人检测的区域

	int     m_nDetectedHoGPeoNum;  //检测到的HoG行人个数
	float   m_fAHoGPeoScore[10];   //检测到的HoG行人的得分
	CvRect  m_rectAHoGPeo[10];     //检测到的HoG行人的rect

	void mvInitVar( ); 

	void mvSaveDetRes( CvRect rectBestPeoArea,			
				   vector<float> &vctPeoScore,   
				   vector<CvRect> &vctRectPeoDst );    
	
}MvHoGPeoDetRes;


//目标关于类型判断的基本信息
typedef struct StruObjTypeJudgeBasicInfo
{
	StruObjTypeJudgeBasicInfo( ) { mvInitVar( ); }
	
	MvHoGPeoDetRes  m_HoGPeoDetRes;

	void mvInitVar( )
	{ 
		m_HoGPeoDetRes.mvInitVar( );
	}

}MvObjTypeJudgeBasicInfo;

//目标的基本信息
typedef struct StruObjBasicInfo
{
	double   m_dTs;

	StruObjLocationBasicInfo m_BI4ObjLocation;
	StruObjMaxsizeBasicInfo  m_BI4ObjMaxsize;
	StruObjTrackBasicInfo    m_BI4ObjTrack;
	StruObjAngleBasicInfo    m_BI4ObjAngle;
	StruObjVelocityBasicInfo m_BI4ObjVelocity;
	StruObjAreaBasicInfo	 m_BI4ObjArea;

	//目标类型判断信息
	MvObjTypeJudgeBasicInfo  m_BI4ObjTypeJudge;

	void mvInitVar( );

}MvObjBasicInfo;


//单帧所对应的目标信息
typedef struct StruSingleFrameObjInfo
{
	bool    m_bEmpty;       //是否为空

	double  m_dTs;			//该帧对应的时间戳
	int     m_nMaxObjIdx;   //该帧目标序号的最大值

	MvIdx2IdMap    *m_pObjIdx2IdMap;  //目标序号到ID的映射关系
	MvObjBasicInfo *m_pObjBasicInfo;  //目标的基本信息

	StruSingleFrameObjInfo( )
	{
		m_bEmpty = true;		//是否为空

		m_dTs = -10000.0;		//该帧对应的时间戳
		m_nMaxObjIdx = -10000;	//该帧目标序号的最大值

		m_pObjIdx2IdMap = NULL;  //目标序号到ID的映射关系
		m_pObjBasicInfo = NULL;  //目标的基本信息
	}

	void mvSetValue( double dTsNow, int nMaxObjIdx,
					 MvIdx2IdMap   *pObjIdx2IdMap, 
					 MvObjBasicInfo *pObjBasicInfo )
	{
		m_bEmpty = false;			//是否为空

		m_dTs = dTsNow;				//该帧对应的时间戳
		m_nMaxObjIdx = nMaxObjIdx;	//该帧目标序号的最大值

		m_pObjIdx2IdMap = pObjIdx2IdMap;  //目标序号到ID的映射关系
		m_pObjBasicInfo = pObjBasicInfo;  //目标的基本信息
	}

}MvSingleFrameObjInfo;


//多帧所对应的目标信息
#define  OBJINFO_MAX_MULTI_FRAME 100  //目标信息最多的帧数
typedef struct StruMultiFrameObjInfo
{
	vector<int> m_vectVaildIdx;      //目标信息从近到远的帧

	CycleReplace m_nFObjInfoStoreCR; //多帧目标信息的循环覆盖体

	MvSingleFrameObjInfo m_ASFrameObjInfo[OBJINFO_MAX_MULTI_FRAME];

	StruMultiFrameObjInfo( )
	{ 
		m_vectVaildIdx.clear( );
		m_nFObjInfoStoreCR.mvGiveMaxSize(OBJINFO_MAX_MULTI_FRAME);
	}
}MvMultiFrameObjInfo;


//目标事件报警的结构体
typedef struct StruObjEventAlert
{
	int  m_nObjIdx;  //目标序号
	bool m_bShow;    //是否为显示

	StruObjEventAlert( int nObjIdx, bool bShow )
	{
		m_nObjIdx = nObjIdx;  //目标序号
		m_bShow  = bShow;     //是否为显示
	}

	void mvSetValue( int nObjIdx, bool bShow )
	{
		m_nObjIdx = nObjIdx;  //目标序号
		m_bShow  = bShow;     //是否为显示
	}
}MvObjEventAlert;

#endif