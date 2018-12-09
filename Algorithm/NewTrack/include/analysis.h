#ifndef __ANALYSIS_H
#define __ANALYSIS_H


#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <list>

#include "libHeader.h"

#include "ConfigInfoReader.h"

#include "DetectConfiger.h"  //检测配置类

#include "MedianBg.h"
#include "MvSimVibeBg.h"

#include "FkSubtrack.h"
#include "FkBgJudge.h"
#include "ForeImgApp.h"

#include "MvStruGxj.h"

#include "ShadowDetector.h"
//#define  USE_SHADOW_DETECT  //阴影和灯光判断
#ifdef USE_SHADOW_DETECT
	#include "ShadowLightDetector.h"
#endif

#include "ImgPro.h"
#include "MvKeyPtExtract.h"

#include "MvStruDebug.h"   //调试结构体

#include "GridStopDetector.h"
#include "MvCalCoord.h"
#include "Mv3DModel.h"
#include "LinesExtApp.h"

#include "ObjTypeDetector.h"

//跟踪
#include "LTDRectTrack.h"
#include "MatchTrack.h"

//聚类
#include "ConstructGroup.h"	

#include "Detector.h"

#include "GlobalDetRes.h"
#include "LightLampDetector.h"
#include "VehicleTopJudge.h"

#include "struRoadDet.h"  //智能交通的结构体-用于代码管理和简化

#include "Interface4Server.h"  //与服务端的接口
#include "SameObjectCheck.h"   //是否为同一目标的检查器

#include "AreaSeeker.h"   //区域搜索器

#include "TracksApp.h"   //轨迹公用的应用函数包
#include "StaDetApp.h"   //统计和检测公用的应用函数包

#include "OpenConfigParameter.h"  //开放的配置参数

#include "MvCamshift.h"  //Camshift

#include "comHeader.h"    //放最后

using namespace std;

#define USE_NEW_MATCH_TRACK  //使用新的点跟踪方法
#define USE_SIM_HOG          //使用新的简单HoG

//注意：所有的结构体在作为内的成员时，需要重新赋初值
typedef struct objStandSizeConfig  
{
	bool  bVaild;
	CvPoint2D32f ptLocPt1;
	CvPoint2D32f ptLocPt2;
	CvPoint2D32f ptCarSz1;
	CvPoint2D32f ptCarSz2;

	objStandSizeConfig( );

	objStandSizeConfig( 
		CvPoint2D32f pt1, CvPoint2D32f pt2,
		CvPoint2D32f sz1, CvPoint2D32f sz2 );

}ObjStdSzConf;  //标定结构


typedef struct _imgSzSet
{
	CvSize m_szCfgImg;   //用于车道绘制的图像大小
	CvSize m_szSrcImg;   //原始传递过来的图像大小
	CvSize m_szUseImg;   //用于分析的图像大小

	_imgSzSet( );

}ImgSzSet; //图像大小集合


typedef struct _bgModelImageSet
{
	IplImage *pMedianRgbImg; //中值背景模型所用的彩色图像
	IplImage *pMedian_RImg;  //中值背景模型所用的R图像
	IplImage *pMedian_GImg;  //中值背景模型所用的G图像
	IplImage *pMedian_BImg;  //中值背景模型所用的B图像

	IplImage *pMedian_RgbBgImg;	 //中值背景模型的rgb背景图像

	IplImage *pMedian_FkImg4RgbBg;  //rgb中值背景模型的前景图像(道路区域)
	IplImage *pMedian_FkImgBefPro;  //rgb中值背景模型的进行处理前的前景图像


	IplImage *pVibeRgbImg;
	IplImage *pVibeFkImg;

	_bgModelImageSet( );

	void initVar( );

	void releaseImgSet( );

}BgImgSet; //背景模型图像集合


typedef struct _analyImageSet
{
	IplImage *pObjWSzImg;  //目标宽图像
	IplImage *pObjHSzImg;  //目标高图像
	
	bool     bExistSkipArea;  //是否存在忽略区域
	IplImage *pSkipImg;  //忽略区域图像
	IplImage *pMaskImg;  //道路mask图像

	IplImage *pCarWSzImg;  //车宽图像
	IplImage *pCarHSzImg;  //车高图像

	_analyImageSet( );  	

	void initVar( );  

	void releaseImgSet( );

}AnalyImgSet; //用于分析的图像


typedef struct _gridStopImgSet
{
	IplImage *pMaskImg;
	
	IplImage *pSkipImg;
	
	IplImage *pCarWSzImg;
	IplImage *pCarHSzImg;

	_gridStopImgSet();	

	void initVar( );

	void releaseImgSet( );

}GridStopImgSet; //用于网格停车的图像


typedef struct _An_TsStampSet  //时间戳集合
{
	double dTsStartCalIllum;	
	double dTsLastCalIllum;	

	_An_TsStampSet( );

	void initVar( );

}AnTsStampSet;  //标定结构

//背景模型和前景提取的模式
enum{
	UPDATE_MEDIAN_BGMODEL = 0, //更新中值背景模型	

	UPDATE_GETFK_VIBE_MODEL,   //更新Vibe模型和获取前景--delete
	GETFK_FROM_VIBE_BGMODEL,   //通过Vibe背景模型来获取前景	

	GETFK_FROM_MEDIAN_BGMODEL, //通过中值背景模型来获取前景

	UPDATE_VIBE_BGSAMPLES      //更新Vibe背景模型中的背景样本
};

//------------------CIntelAnalyst------------------//
class CIntellAnalyst
{
public:
	CIntellAnalyst( void );
	~CIntellAnalyst( void );

	bool mvInitIntellAnalyst(CvSize szSrcImg, IplImage *pOriRgbImg=NULL);
	bool mvUninitIntellAnalyst( );

	bool mvGetConfigInfo( ObjStdSzConf objSzCfg ); //获取配置信息

	int  m_nVideoNo, m_nFrameNo;
	void mvGetVideoFrameIdx( int nVideoNo, int nFrameNo )
	{
		m_nVideoNo = nVideoNo, m_nFrameNo = nFrameNo;
	}

	//智能分析主接口
	int   m_nRunCnt;  //运行的次数
	bool  m_bHadIntellAnalysisThisFrame;  //当前帧进行过了图像智能分析
	bool  mvIntellAnalysis( IplImage *pOriRgbImg,
			CfgInterface4Server *pInterface4Server );

	//针对事件检测进行智能分析的接口
	bool mvIntellAnalysis4EventDet( 
			vector<StruObjStopAreaInfo> &vectObjStopAreInfo //停止目标信息
		);

	//得到结果主接口
	bool mvGetAnalysisResult( DetectResultList &detOutList );

public:

	//得到各种网格的间隔的信息
	IplImage  *m_pInteMvStCntImg;
	void mvGetGridsInfo( StruAreaPointTrackInfo &areaPtTrInfo );

#ifdef  SHOW_ANALYSIS_OBJDETECT
	char *m_p_chFileName;
	void mvGetReadImgName(char *pChFileName) {	m_p_chFileName=pChFileName; }
#endif	

	//得到更好的结果区域
	bool mvGetBetterResultAreas(vector<CvRect> &vectBetterRect,
							const vector<CvRect> &vectGiveRect);
	bool mvGetBetterResultArea(CvRect &rctBetter, const CvRect &rctGive);

	//获取得到前景的256梯度图像（32S）
	IplImage * mvGetFkGrad32SImg( IplImage **ppFkGrad8UImg );

	//判断目标区域是否为大车或中车的车顶在给定区域内
	bool mvIsTopOfBigMidVehicleInGiveArea(const CvRect &rectObjArea, 
			    CvRect &rectGetBMVArea, bool bDay, double dTsNow);

	//获取"用于目标确认的带时间戳的图像集"的指针
	MvObjConfStampImgSet* mvGetObjConfirmStampImgSetPointer( ){
			return (&m_ObjConfStampImgSet);
	}
  
	//定位小车的底部
	bool mvLocateSmallVehicleBottom(	
			vector<CvRect> &vectRectSmlVehBot,
			const vector<int> &vectObjIdx,
			const vector<CvRect> &vectRectGiveSeekVehBot,			
			const vector<MvObjBasicInfo *> &vectObjBIPointer );

public:
	GetParamSet   m_getParam;   //根据配置和其他，得到一些参数

private:

	//获取车底部阴影比率(背景/当前灰度)的积分图 
	IplImage * mvGetRateInteImgOfVehBotShadow( 
			MvTimeStampImg &TsImg4VBShadow,
			IplImage *pGrayImg, 
			IplImage *pGrayBgImg, 
			double dTsNow );

	//获取最好的车辆底部阴影区域
	void mvLocateSmlVehBotWithShadow(
			vector<CvRect> &vectBestSeekVehBot,	
			const vector<int> &vectObjIdx,
			const vector<CvRect> &vectRect4GiveSeekVehBot, 		
			IplImage *pGrayImg, IplImage *pGrayBgImg,
			IplImage *pDrawImg = NULL );


	//获取背景梯度图像的反值图
	IplImage * mvGetInvBgGrad2VImg( );

	//获取细化的水平边缘积分图
	void  mvGetThinHoriEdgeInteImg(
			IplImage **ppThinHoriEdgeImg,
			IplImage *pDiffHSobel256Img );

	//获取最好的车辆底部水平边缘区域
	void mvLocateSmlVehBotWithHoriEdge( 
			vector<CvRect> &vectBestSeekVehBot,
			const vector<int> &vectObjIdx,
			const vector<CvRect> &vectRect4GiveSeekVehBot, 		
			IplImage *pHoriEdgeIntImg, 
			IplImage *pDrawImg = NULL );

	//获取y方向最好的对称轴
	bool mvSeekBestSymmetryAxis(	
			const vector<MvObjBasicInfo *> &vectObjBIPointer,
			IplImage *pIntHSob256Img, IplImage *pIntVSob256Img, 
			IplImage *pIntGray256Img, IplImage *pIntFk2VImg,
			IplImage *pDrawImg=NULL );

public:
	CConfigInfoReader  *m_p_cCfgInfoReader;  //配置读入器指针
	//--(地址为road_detect类中的m_cCfgInfoReader)

	StructDetStaParaConfiger *m_p_DetStaParaConfiger; //检测和统计配置参数读入器指针

	CfgInterface4Server *m_pInterface4Server;  //与server端的接口配置
	//--(地址为road_detect类中的m_interface4Server)

	MvStruTotalDebug  *m_p_myDebug;   //调试结构体的指针
	MvDebugCfgReader  *m_p_debugCfg;  //调试配置读入器的指针

	CCalCoord    m_cCalCoord; //标定坐标器

	CDetectConfiger	   m_cDetConfigerByFile; //文件检测配置器 

	//得到角点和一些其他的图像工具
	MvStruKeyptExtract  m_struKeyptsExtract;  //角点提取结构体
	KeyptsStore		    m_struKeyptsStore;    //角点存储结构体

#ifdef USE_SIM_HOG
	AnStruSimHoGPeoDetApp m_struHoGPeoDetApp; //HoG行人检测应用器
#endif

	GlobalDetRes  m_gobalDetRes;  //全局的检测结果

	StructLightAreaJudge  m_lightAreaJudge;  //灯光区域判断器

	//分析类中事件所用的图像集合结构体
	Event4AnalysisUseImgSet m_event4AnalyImgSet; 


	//获取程序的启动初始时间，作为一个全局参数
	char g_chInitDateTime[48];
	
	//调试所在的目录
	char g_chDebugSaveDir[48];   //调试结果所存的目录
	char g_chDebugCfgDir[48];    //调试配置所在的目录

	//角点跟踪聚类
#ifdef USE_NEW_MATCH_TRACK
	AnGlobalSet m_pGlobalSet;
	StruPtMatchTracker m_ptMatchTracker;  //角点匹配跟踪器
	StruPtTrProperty m_struPtTrProperty;  //点轨迹属性判断
	PtsNear2FarSort  m_struTrPtsN2FResult; //轨迹点进行由近到远排序后的结果
	//StruPtTrSegment  m_ptTrSegmentor;   //角点轨迹聚类器
	StruAreaPointTrackInfo m_struAreaPtTrInfo;  //区域内的点轨迹信息
#endif

	MvM2STrConnector  m_Move2StopTrConnector;  //由运动到静止的轨迹连接器

	//目标相关
	MvObjConfStampImgSet m_ObjConfStampImgSet; //目标确认的带时间戳的图像集

private:   //初始化
	//初始化分析类的调试目录
	void mvInitDebugDirectory( );

	//初始化分析类的变量
	void mvInitIntellAnalystVar( );

	//判断是否只检测长时间停车
	bool mvIsJustDetectLongVehicleStop(float fLongTh);

	//初始化分析类所用的图像尺寸且保留其坐标关系
	void mvInitIntellAnalystUseSize( CvSize szSrcImg );

	//是否成功获取将初始车道绘制信息进行转换后的配置信息
	bool mvGetChanRoadCfgInfo( );

	//初始化分析类的相机标定
	bool mvInitIntellAnalystCamCalib( );

	//修改配置中的标定点
	bool mvModifyCalibrationPoint( );

	//生成标定
	void mvCamreaCalibration( IplImage *pUseRgbImg );

	//设置多个图的值(这些图在今后不改变)
	bool mvSetImagesConstValue( );

	//初始化事件检测类所用的图像
	void mvInitEventUseImgs( CvSize &szRoiRes );

	//初始化分析类所用的背景模型
	void mvInitBackgroundModel( CvSize szRoiRes );

	//初始化分析类所用的Roi和resize背景模型
	void mvInitRoiResBackgroundModel( bool bDetLongStop, CvSize szRoiRes );
	
	//初始化分析类所用的各种网格
	void mvInitImgGrids( );

	//释放分析类所用的各种网格
	void mvUninitImgGrids( );

	//初始化前景背景判断类
	void mvInitFkBgJudge( );

private:  //背景模型

	//背景模型和前景提取
	void mvBgModelFkExtrack( int nMod, double dTsNow,
			IplImage *pUseGrayImg, IplImage *pUseRgbImg,
			IplImage *pFkMaskImg=NULL );

	//Vibe背景模型的使用
	void mvGetVibeBgFk( IplImage *pFkMaskImg, int nMod );


	//获取得到中值背景模型的前景图像
	void mvGetFkImg( IplImage *pFrameDiff2VImg = NULL,
					 IplImage *pGrad2VImg = NULL,
					 IplImage *pVibeFk2VImg = NULL,
					 IplImage *pGrad2VDiffBgImg = NULL,
					 IplImage *pRoadMaskImg = NULL );

	//获取得到当前图和背景图的比率结果图像
	void mvGetNowBgRateImg( );

	//获取用于事件检测所用的前景和背景图
	void mvGetFkBgImg4RoiRes( IplImage *pAnalysisFkImg );

	//处理得到前景图像，得到更好的前景(干净，充实)
	void mvProcessFkImg( IplImage *pSrcFkImg, 
					     IplImage *pDstFkImg,
						 bool bEasyFkImgExtModel = false,
						 IplImage *pCarWImg = NULL,
						 IplImage *pCarHImg = NULL );

	//中值背景模型的更新
	void mvUpdateMedianBgModel( double dTsNow,
			IplImage *pGrayImg, IplImage *pRgbImg );

	//光照计算和应用
	void mvIllumCalAndUse( IplImage *pUseGrayImg,
			IplImage *pUseFk2VImg, IplImage *pRImg = NULL,
			IplImage *pGImg = NULL,	IplImage *pBImg = NULL, 
			bool bVaildBgRgbImg = false, IplImage *pRBgImg = NULL, 
			IplImage *pGBgImg = NULL, IplImage *pBBgImg = NULL );

private:  //阴影/车灯/灯光

	//阴影/车灯/灯光等检测 
	void mvShadowLampLightDetect(
			bool bDay, int nBgIllum,      //是否为白天/背景光照度
			const IplImage *pStopAFkImg,  //静止的前景图像
			const IplImage *pMoveAFkImg,  //运动的前景图像
			const IllumRgbVarSet &illumRgbVarSet  //光照变量集
		);

	bool mvDetectBrightAreaPairLampLight( 
			vector<int> &vectLampObjIdx,  //车灯所对应的目标序号
			int nAreaCnt,		     //给定的目标数目
			const int *nAObjIdx,     //给定的目标序号
			const CvPoint *ptAObjLt, //目标区域左上点
			const CvPoint *ptAObjRb, //目标区域右下点
			const vector<fkContourInfo> &vcetForeContours  //前景轮廓
		);

	//--------begin----TLD--------
public:
	vector<CvRect> m_vectNowWantTrackRects;  //当前欲跟踪rects
	vector<OneTLDTrackResult> m_vectTLDReulst;  //TLD跟踪结果

public:  //原为private,为了在event_detect中使用，改为public
	void mvRectsTrackWithTLD(
			vector<CvRect> &vectNowFindCandRects,  //当前发现的候选跟踪区域
			bool bTrackFirst = false  //是否采用跟踪优先(默认为检测优先)
		);

	//对之前帧和当前帧进行CamShift跟踪 
	bool mvRectsTrackWithCamShift(	
			vector<int> &vRectId,      //OUT:
			vector<CvRect> &vTrackRes, //OUT:
			vector<CvRect> &vRectSel,  //OUT:
			vector<CvRect> *vectGive,  //IN:
			vector<int>	  &vIdOfGive,  //IN:
			vector<CvRect> *vectPred   //IN:
		);

private: 
	//对给定的rect进行TLD跟踪 	
	int	  m_nLastFindRectIdxs;  //上帧寻找到的区域index


	//--------end----TLD--------

private:
	//前景背景图像的的应用
	void mvBackForeImgApplication( );

	//图像处理
	void mvProcessImage( IplImage *pOriRgbImg );

	//处理背景和前景图像 
	void mvProcessBackForeImg(double dTsNow, bool bUpdateBgImg);

private:

	//得到角点和一些其他的图像工具
	void mvGetKeyptsOtherTools( IplImage *pOriRgbImg );

	//得到和保存角点
	void mvGetAndStoreKeypts( IplImage *pOriRgbImg, double dTsNow, 
				 double dSTermTime, double dLTermTime,
				 double dBgTermTime, double dBgImgTermTime,
				 bool bDetBankPeopelApp = false  //是否银行行人检测
			);

	//背景角点判断
	void mvJudgeBackgroundKeypts( double dTsNow, int &nCnt, 
			     CvPoint2D32f *pKeyPts, float *pEigOfPt,
				 uchar **ppSiftDesc, bool bDetBgPt=false );

#ifdef USE_OBJ_DETECT
	//轨迹层
	void mvTrackLayer( int nFrameNo, vector<AnPtTrack> &vctVaildTrs );

	//group层
	void mvGroupLayer( int tsNow, vector<AnPtTrack> &vctVaildTrs );

	//事件报警层
	void mvEventAlert( int tsNow, vector<AnPtTrack> &vctVaildTrs );
#endif


private:  //网格拥堵和排队	
	//利用网格停车法检测出的拥堵
	bool mvGetDetChanJamResultWithGrid(
		vector<SRIP_DETECT_OUT_RESULT> &vectDetOut);

	//利用网格法得到目前认为是拥堵的车道序号
	vector<int> m_vectJamChannelIdx;
	vector<CvPoint> m_vectChannelJamCet;
	void mvGetJamAlertWithGrids( vector<int> &vectJamChannelIdx, 
		vector<CvPoint> &vectChannelJamCet );

	//采用网格停车法进行停车检测
	void mvGetTopCetBottomCarSzOfArea(
			CvSize szATCB[3],     //OUT:上中下三个区域的车大小
			const CvPoint &ltPt,  //IN:区域的左上点
			const CvPoint &rbPt,  //IN:区域的右下点
			IplImage *pCarWImg,   //IN:车辆的宽度图
			IplImage *pCarHImg,    //IN:车辆的高度图
			int nPtCnt,					//IN:区域的点数
			CvPoint2D32f *ptAChanRgnPt  //IN:区域的顶点坐标
		);

	//利用网格法得到轨迹匹配较差的区域
	void mvGetKeyptMatchBadAreaWithGrids( );

	
	//利用网格法统计车道的队列长度
	bool mvGetStatChanQLResultWithGrid( 
		vector<SRIP_DETECT_OUT_RESULT> &vectDetOut );

	//利用网格法得到各车道的排队长度
	vector<int> m_vectQueueChanIdx;    
	vector<float> m_vectQueueChanLen; 
	double m_dATsQueueLenStat[20];
	double m_dAValQueueLenStat[20];
	AnDetStaResStore m_StaResStore4QueueLen[20];
	void mvGetQueueLenghtWithGrids(	vector<int> &vectChanIdx,    
			   vector<float> &vectQueueLen, int nNoNullTrNum,
			   int *nANoNullTrIdx, MyTrackElem **pATrPointer );

	void mvSetQueueLenResult(SRIP_DETECT_OUT_RESULT &detOutResult,
			        int nChanIdx, double dQueueLen, double dTsNow);

	void mvSaveQueueLenResult(double dTsNow, int nResChanCnt, 
			        int *nAChanIdx, double *dAQueueLen);

private:  //网格停车	
	//对网格法停车进行初始化
	void mvGridStopDetector( CvSize srcSz, CvSize useSz, CvSize stopSz, 
		       CvRect useRoi, IplImage *m_pCarXImg, IplImage *m_pCarYIm,
			   KeyptsStore *pStruKeyptsStore );
	
	//采用网格停车法进行停车检测
	bool mvDetectStopCarWithGrid( 
			vector<StruObjStopAreaInfo> &vectObjStopAreInfo //停止目标信息
		);

	//得到网格停车法检测出的停车
	bool mvGetDetStopCarResultWithGrid(
			vector<SRIP_DETECT_OUT_RESULT> &vectDetOut);

	//利用网格停车法检测出的缓存停车
	bool mvGetDetStopCarCacheResultWithGrid( 
			vector<SRIP_DETECT_OUT_RESULT> &vectDetOut);

	//形成停车区域，确认停止区域
	bool mvConfirmStopDetect( 
			vector<StruObjStopAreaInfo> &vectObjStopAreInfo, //IN:停止目标信息
			double dTsNow, bool bDay,     //IN:时间戳和是否为白天
			vector<int> &vectStopCarPtNo  //IN:检测到的停止点
		);


	//利用停止角点形成区域，并对检测到停止区域进行确认
	void mvConfirmStopDetect4RemotePicCache( 
			vector<CandidateStopArea> &vectCandSA, //OUT:检测到的停止点
			vector<int> &vectStopCarPtNo,		  //IN:检测到的停止点
			StruSaveRemotePic &cfgSaveRemotePic,  //IN:存远景图的配置    
			double dTsNow, bool bSensitive,		  //IN:当前时间戳和是否敏感
			bool bJustDetUnnormalStop			  //IN:是否只检测非正常停车
		);

	//利用停止角点形成区域，并对检测到停止区域进行确认
	void mvConfirmStopDetect4RemoNearComCache( 
			vector<CandidateStopArea> &vectCandSA,
			vector<int> &vectStopCarPtNo,	
			StruRemoteNearCombPic &cfgSaveRemoNearCombPic, 
			double dTsNow, double dGiveComparePreTime, 
			bool bSensitive, bool bJustDetUnnormalStop     
		);


	//判断给定区域在给定时间内是否为误报的目标
	bool mvIsWrongObjOfGiveTime( CvPoint ltPt, CvPoint rbPt,
			CvSize useSz, double dTsSta, double dTsEnd );

	//判断采用网格停车法得到的停止区域是否为车
	bool mvGridStopAreaIsCar( CandidateStopArea vecCandSA, 
			double dTsNow, bool bDay,
			CvPoint &ptResLt, CvPoint &ptResRb );

	//判断点击处是否为车
	bool mvIsCarOnClick( uchar &uchCarMod,
			bool bDay, IplImage *pSrcImg, IplImage *pFkImg, 
			CvPoint2D32f fSrcPt, CvPoint2D32f fSrcCarSzPt, 
			CvPoint2D32f fSrcPeoSzPt, CvPoint2D32f fSrcOriPt, 
			float fSrc2Fk_x, float fSrc2Fk_y, 
			bool bShadowArea = false );

	//判断是否为大车的车顶落入到停止区域而造成的误报
	bool mvIsErrorAlert4BigVehTopInStop(
			CandidateStopArea candSA, bool bDay 
		);

	//判断给定区域是否为大车的车顶
	bool mvIsTopOfBigVehicle( bool bDay, //是否为白天
			CvRect rectObjArea,			 //给定的目标区域
			IplImage *pCarWImg,			 //计算的车辆宽结果图
			IplImage *pCarHImg,			 //计算的车辆高结果图
			IplImage *pFkAllImg,		 //全图区域内的前景
			vector<CvRect> vecStopArea,  //所有的停车区域
			CvRect &rectExBigVehInFk,    //前景图中的大车扩展区域
			IplImage *pFkDrawImg = NULL,
			IplImage *pDrawMaskImg = NULL
		);

	//利用线段来判断是否为大车
	bool mvBigVehUseLine( IplImage *pSrcImg, //当前图
			IplImage *pDrawGrayImg,   //当前灰度copy图
			IplImage *pGrayBgImg, //灰度背景图
			CvRect rect,		  //给定的判断区域
			CvSize rectSz         //判断区域所对应的图大小
		);

	//判断给定的目标区域是否在skip区域内
	bool mvIsGiveAreaInSkipArea( IplImage *pSkipImg,	
			CvPoint ltPt, CvPoint rbPt );

	bool mvIfExistPeopleOnClick( CvPoint2D32f fSrcPt,
			CvPoint2D32f fSrcCarSzPt, CvPoint2D32f fSrcPeoSzPt,		
			IplImage *pSrcImg, bool bHard ) ;

	//判断最近10分钟（背景模型10分钟可改过来）光线是否发生了较大的变化
	bool mvIsIllumChangeNowToBef10Min( double dTsNow );

private:
	void mvGetShowImgSize( CvSize &sz )
	{ 
		m_bInitIntellAnalyst ? sz=m_bgSize : sz=cvSize(400,300); 
	}

	CvPoint2D32f mvGetCarSize( CvPoint2D32f pt,
			 CvPoint2D32f pt1, CvPoint2D32f pt2,
			 CvPoint2D32f ptCarSz1, CvPoint2D32f ptCarSz2 );

	bool mvGetImgCarSizeWith2Pt( CvPoint2D32f pt1, CvPoint2D32f pt2,
			 CvPoint2D32f ptCarSz1, CvPoint2D32f ptCarSz2 );


private:
	//创建忽略图像
	void mvCreateSkipImage(  );

	//创建mask图像
	void mvCreateMask( IplImage *pMaskImg, 
			CvPoint2D32f *borderPt,
			int nPolySize, CvRect &roadRoi );

	//创建道路mask图像和ROI
	void mvCreateRoadMask_Roi( IplImage *pMaskImg,
			 CvSize useSz, CvRect &rctRoadRoi );

private:
	bool m_bInitIntellAnalyst;   //分析类是否初始化了
	bool m_bGetConfigInfo;       //是否得到了配置信息
	bool m_bJustUseGridStopDet;  //是否只采用网格停车检测

private:  //各功能元件
	PrintfStruct m_printfIO; //进出打印器

	ObjStdSzConf m_objSzCfg; //标定配置(画框)


private:
	Pt2D32fCoordConvert m_pt2dSrc2UseCoordCvt;  //坐标转换器

	MvSimVibeModel   m_cVibeBGModel;    //vibe背景模型构造器
	CMedianBgBuilder m_cMedianBgModel;  //中值背景模型构造器
	CMedianBgBuilder m_cSmoothGradMedianBgModel; //中值梯度背景模型（平滑后）构造器

	CMedianBgBuilder m_cRMedianBgModel; //R通道的中值背景模型构造器
	CMedianBgBuilder m_cGMedianBgModel; //G通道的中值背景模型构造器
	CMedianBgBuilder m_cBMedianBgModel; //B通道的中值背景模型构造器

	CFkSubtrack		 m_cFkSubtrack;	   //前景抠取器

	CForeBackGroundJudge m_cFkBgJudge; //前景和背景判断器
	
	StruObjConfirmByCtus m_objConfirmByFkCtus; //对前景轮廓进行目标判断

	StruShadowDetector m_cShadowDet;   //阴影检测器

	
#ifdef USE_SHADOW_DETECT
public:	
	CShadowDetector m_cShadowDetector;  //阴影和灯光检测器
	DeleteShadower	m_deleteShadow;		  //阴影删除器
#endif


private:
	CImgPro          m_cImgProcessor;  //图像处理器

	IllumResSaveStru m_illumResSaver;    //光照结果储蓄器(原始灰度图)
	IllumResSaveStru m_bgIllumResSaver;  //光照结果储蓄器(背景灰度图)

	AnImgGridsStru	 m_imgGridForCar;	  //对车辆的图像网格
	AnImgGridsStru	 m_imgGridForPeo;	  //对行人的图像网格
	AnImgGridsStru	 m_imgGridForFkGhost; //对前景ghost的图像网格

#ifdef USE_OBJ_DETECT
	CMatchTrack      m_cMatchTracker;  //匹配跟踪器

    CConstructGroup  m_cGroupConstructor;  //匹配跟踪器

	CObjectDetector  m_cObjectDetector;   //目标检测器

	MvDspGlobalSetting *m_pGlobalSettin;  //全局参数
#endif
	
	CGridStopDetector  m_cGridStopDetector;  //网格停车检测器
	AnSameStopAreaCheck m_sameStopAreaCheck; //相同停车区域检查器

	TLDRectTracker  m_tldRectTracker;     //ltd区域跟踪器
	MvClassCamShift  m_CCamShift;         //camShift辅助跟踪器

	BVTopAdjudicator m_bvTopAdjudicator;     //车顶判断器

private:  //图像大小集合
	CvSize  m_srcSize;
	CvSize  m_srcRoadSize;
	CvSize  m_dstSize;

	CvSize  m_bgSize;
	CvSize  m_szVibeBg;

	CvSize  m_gridStopCarSize;

private:  //图像集
	BgImgSet       m_bgMImgSet;   //背景模型图像集
	AnalyImgSet    m_analyImgSet;
	GridStopImgSet m_gridStopImgSet;
	AnTsStampSet   m_tsAnalysisSet;
	
	IllumRgbVarSet m_illumRgbVarSet;

	BgImgSet       m_roiRes_bgMImgSet;  //设置roi并resize的背景模型图像集


	//调试信息存储
	char	 m_chTempDebugInfo[104];
	MvDebugInfoList m_DebugInfoList; 


	//----------------配置信息----------------//
private:
//	ConfigParamSet m_configParam;        //参数配置集合

private:
	CvRect        m_rctRoadRoi;         //道路区域的ROI

};



#endif
