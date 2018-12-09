// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _MY_DETECT_H
#define _MY_DETECT_H

#include "roadDetHeader.h"	 

#include "MvMathUtility.h"
#include "MvImgProUtility.h"
#include "MvSmallFunction.h"
#include "MvAnAlgorithm.h"

using namespace std;
using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_AN_ANLGORITHM;
using namespace MV_SMALL_FUNCTION;

#ifndef LINUX
	//#define WIN_SIMULATE_CAMCTRL4STOPDET  //模拟在停车检测过程中的相机控制
#endif

//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
	void on_mouse_forMaxsize( int event, int x, int y, int flags, void* param );
	void on_trackbar( int h );
#endif

	


//类声明
class Mvroad_detect
{
	//////////////////////////////////////////////////////////////////
	//////////--------初始化所用函数及变量------------////////////////
	//////////////////////////////////////////////////////////////////
public:
	bool m_bGivenRGBImg;
	int m_nChannel; 

	#ifdef WIN32
		IplImage *m_pWndShowImg ;
		char m_wndName[256] ;
		char m_tbarname[256] ;
		int  m_tbarThresh ;
	#endif

private:

	bool m_bUseNewKeypt;
	bool is_pudong;      //是否为浦东
	bool is_gaoqing;     //是否为高清
	int  m_isDayParam;	 //-1自动，0指定为傍晚，1指定为白天
	bool isDay;			 //0傍晚，1白天
	int  m_isShadowParam; //-1自动，0指定不检测阴影，1指定检测阴影
	bool isShadow;		 //0不检测阴影，1检测阴影
	int  m_nSensitive;	    //事件检测敏感度,主要针对逆行和横穿
	int  m_nSameEventIgn;    //同类事件的忽略时间,对交警总队为180s,其他为0s
	int  m_nStatFluxTime;  //统计周期
	//道路区域的数目,晚上检测需要
	int  m_nRoadRegNum;
	
	RgnList  *m_pRoadRgn;
	VEHICLE_PARAM *m_pParamChan;
	VEHICLE_PARAM_FOR_EVERY_FRAME *m_pParamDetect;
	GLOBAL_SETING_PARAM *m_pParamGloabal;  

	int		*m_pChnlToRoad;	    //车道所对应的道路


	int               m_nStabPolyNum;
	int	              *m_nStabPolySize;  
	CvPoint2D32f      **m_fStabPolyPts;
	bool              m_bNoDetectWhenStabFail;	
	CVideoStabilizer  m_CVideoStabilizer;


#ifndef	USE_NEW_MATCH_TRACK
	//区域内的点轨迹信息
	StruAreaPointTrackInfo m_struAreaPtTrInfo;
#endif

	double ts_last_frame,ts_last_frame2;	  //上一帧的时间戳
	double ts_isDayDetection;	//检测白天还是傍晚的时间间隔
	double ts_isShadowDetection;//检测是否有阴影的时间间隔
	double m_tsLastCalBgImgLight;  //上次计算背景亮度的时刻
	int        m_nBgImgAvgLight;       //计算出的背景亮度

	bool m_bRoadFar;  //场景是否由远及近，根据maxsize的变化确定
	CvRect m_roadRoi;

	int	  m_nHomographyPtNum;
	float m_fASrcHomegraphy[20], m_fADstHomegraphy[20];   //进行标定的坐标
	float m_homography[3][3],m_homography_inv[3][3];
	float homography_carnum[3][3],homography_carnum_inv[3][3];

public:
	int    m_nListImgNo, m_nPreListImgNo;
	int    m_nImgFrameNo;  //图像序号
	int    m_nTrOriMapH, m_nTrOriMapW;
	int    ***m_pTrOriHist, ***m_pTrOriFST;
	ParameterConfigStr m_p_str;  //参数设置的结构体
private:
	int   m_nEventMvTrNum;
	int  *m_nAEventMvTrId;
	//设置ROI图像
	float m_fROILeft, m_fROITop, m_fROIRight, 
		  m_fROIBottom;  //在原始图中的ROI坐标
	float m_fROICoff,    //使用图和原始图ROI的比率
		  m_fSrcImg2ServerUseX, m_fSrcImg2ServerUseY;

	bool  m_bHaveHalfMaxsizeChannel;     //是否存在maxsize减半的车道
	int    m_nRoadMod;					 //0-全机动,1-全非机,2-机非混合,3行为分析区域
	double m_src2use_w, m_src2use_h;
	int    image_width, image_height;	 //实验图像宽度和高度
	double	t_now, t_last;               //当前帧,上一帧的时间戳，以秒为单位
	double  m_tsReStart;
	double  m_ts_start_frame,  m_ts_end_frame;   //本帧运算所耗时间，以秒为单位

	int64  m_nIntellAnalysisCnt;

	MvStruTotalDebug  m_myDebug;  //调试结构体

public:
	//maxsize的map图像
	IplImage	*max_size;		//表示2.5米等于多少像素(由homography决定)
	IplImage	*max_sizex;        //在图像上某点正常方向小车的宽(由Tsai决定)
	IplImage	*max_sizey;        //在图像上某点正常方向小车的高
	IplImage	*max_sizex45;    //在图像上某点沿正常方向旋转45度小车的宽
	IplImage	*max_sizey45;    //在图像上某点沿正常方向旋转45度小车的高
	IplImage	*m_pCarMaxSizeXImg; //在图像上某点正常方向车的宽
	IplImage	*m_pCarMaxSizeYImg; //在图像上某点正常方向车的高
	IplImage	*m_pPeoMaxSizeXImg; //在图像上某点正常方向人的宽
	IplImage	*m_pPeoMaxSizeYImg; //在图像上某点正常方向人的高

private:
	//所用到的图像初始化
	IplImage   *m_initRgbImage;    //原始图
	IplImage   *m_oriRgbImage;     //目前所用的原始图(只是指针,未开辟内存)
	IplImage   *m_useRgbImage;     //目前所用的处理图

	IplImage	*rgbImage;
	IplImage    *grayImage, *m_befAdjGrayImage;
	IplImage	*lastImage, *last2Image;
	IplImage    *diff256Image, *diffImage;
	IplImage    *m_pRgbBgImage, *m_pGrayBgImage;
	IplImage	*m_pFore2VImg, *m_pForeMaskImg;	
	IplImage	*m_pFkSobelImg;
	IplImage    *m_pMaskImage,*m_pRoadMaskImage; 
	IplImage	*oriImageEx, *oriImage; 
	IplImage    *skipimage; 	
	IplImage	*m_pChannelIDImg;           //表示像素点对应的通道
	IplImage    *m_pIsPeopleRoadImg;

	IplImage	*m_pIntGrayImg;
	IplImage	*m_pIntBgGrayImg;
	IplImage	*m_pIntDiff2VImg;
	IplImage	*m_pIntFore2VImg;
	IplImage	*m_pIntGrad2VImg;
	IplImage	*m_pIntBgGrad2VImg; 	
	IplImage	*m_pIntGrad2VDiffImg;
	IplImage	*m_pIntHSobelDiffImg;
	IplImage	*m_pIntVSobelDiffImg;

	//隔段灰度图的信息
	double     m_tsImgNow,  m_tsImgPre;
	int        m_nInterValStrogeTime;

	IplImage    *m_pVaildMvTrMapImg;
	IplImage    *m_pIntegrateVaildMvTrMapImg;

	IplImage	*corners1,*corners2;	
	IplImage    *m_pExtKeyPtMapImg;

	//照射光的区域
	float m_fStatTopLeftRate;
	float m_fStatTopRightRate; 
	float m_fStatBottomLeftRate;
	float m_fStatBottomRightRate;//统计阴影信息

#ifdef  DEBUG_ON 
	IplImage  *result_track;
#endif

#ifdef  MVDEBUGON
	int          m_nDebugLineIndex;
	int          *m_pnDebugMod;
	char       **m_ppcDebugInfo;

	//类型判断
	bool      m_bNeedSaveObjTypeJudgeInfo;
	int          m_nLastObjTypeJudgeLineIndex;
	int          m_nNowObjTypeJudgeLineIndex;

	//判断停车
	int          m_nSaveStopJudgeNo;
	bool      m_bNeedSaveStopAddInfo;
	double   m_tsLastTsStopJudgeSave;
	int          m_nLastStopJudgeLineIndex;
	int          m_nNowStopJudgeLineIndex;

	//判断变道
	int         m_nSaveChangeJudgeNo;
	bool      m_bNeedSaveChangeAddInfo;
	double   m_tsLastTsChangeJudgeSave;
	int          m_nLastChangeJudgeLineIndex;
	int          m_nNowChangeJudgeLineIndex;

#endif
#ifndef LINUX
public:		
#endif
	
private:

#ifndef JUSTUSEBASICCODE
	IplImage	*m_pPeoTop2BotHImg; //在图像上以某点为头顶从头到脚的长度
#endif

	float *m_fASqrtVal;  
	float *m_fAATan2Val;

	int num_fast, *cars_fast;  
	int num_slow, *cars_slow;  
	int num_against, *cars_against; 
	int num_dusai;   
	CvPoint2D32f* center_dusai;	
	int num_stop_all, *cars_stop_ID_all;  
	int num_passerby, *passerby;    
	int num_newAppearObj, *newAppearObj;     
	int num_derelict_all, *derelict_ID_all;  
	int num_change, *cars_change;  

	//事件判断辅助信息
	eventAlertInfo *m_AEventAlertInfo;

	//track,group等数据结构的初始化
	GroupingResult	*m_AGroupingResult;

	//各种工具的初始化
	//区域划分器
	int        m_nsubAreaNum;	
	SubAreaStr *m_subAreaStrA, m_tempSubAreaStr; 
	    
	// 短期背景线段检测器
	CBgLineDetector  m_SbgLineDetector; 

	// 长期背景线段检测器
	CBgLineDetector  m_LbgLineDetector; 

	MvBgTrackMap *m_pTrackMap;

	//目标ID和序号缓冲器
	int     m_nCompBuffTime;
	unsigned long    *m_nACompBuffId;   //目标id
	int     m_nBuffObjSetNum;
	int    *m_nABuffObjSetId;           //目标序号

	//事件缓冲器
	CBufferMechanism  m_cBuffObjExist;				//目标存在判断缓冲器
	CBufferMechanism  m_cBuffAppear;				//目标出现检测缓冲器
	CBufferMechanism  m_cBuffBargeIn;				//闯入检测缓冲器
	CBufferMechanism  m_cBuffCarnum;				//车牌检测出现缓冲器
	CBufferMechanism  m_cBuffStop;					//停车检测缓冲器
	CBufferMechanism  m_cBuffCross;					//横穿检测缓冲器
	CBufferMechanism  m_cBuffAgainst;				//逆行检测缓冲器
	CBufferMechanism  m_cBuffChange;				//变道检测缓冲器
	CBufferMechanism  m_cBuffBeyondMark;			//越界检测缓冲器
	CBufferMechanism  m_cBuffSlow;					//慢行检测缓冲器
	CBufferMechanism  m_cBuffFast;					//超速检测缓冲器	
	CBufferMechanism  m_cBuffPersonRun;				//行人奔跑检测缓冲器
//	CBufferMechanism  m_cBuffPersonCrowd;			//行人聚集检测缓冲器
	CMayBigVBuffer    m_cBuffMayBigV;				//候选大车缓冲器


	CTrafficStat  m_cTrafficStat;   //交通统计器

	AnEventDSCfgParaReader m_DetStatCfgParaReader;  //检测和统计配置参数读入器

	//动态结构中添加和提取元素的临时变量
	CvMemStorage	*m_memStorage;
#ifndef LINUX
public:
#endif
	CvSet *track_set, *vehicle_result_set, *vehicle_history_set;  //动态结构
	MyTrackElem	  *m_pATrackAddress[SETMAXTRNUM];   //轨迹集合中轨迹对应的地址
	MyVehicleElem *m_pAGroupAddress[MAX_OBJECT_NUM]; //group集合中group对应的地址
private:
	RoadAreaStruct m_RoadAreaInfo;   //道路区域

	unsigned long   m_nTrackId, m_nVehicleId;
	int  m_nNoNullTrNum;
	int  *m_nNoNullTrNo;
	vector<mvTrStruct> m_vctMvTr;
	vector<mvTrStruct> m_vctFKTr;
	vector<fkContourInfo> m_vctForeContours;  

	vector<int> m_vctNoNullVehicle;
	bool *m_bNoAlertAppear;
	int  m_nSortEventObjNum;
	int  *m_nASortEventObjNo;


	float **m_fAHoGSvmDetector;          //较为容易判断为行人的HoG分类器的各矢量值
	float *m_fADescriptors;
	float *m_fAWindowHogDesc;
	MvHOGDescriptor  m_hogDescriptor;	//较为容易判断为行人(默认)


	int     *m_p_nA_hSortLNo, *m_p_nA_vSortLNo, *m_p_nA_oSortLNo;
	double  *m_p_dA_hSortLData, *m_p_dA_vSortLData,  *m_p_dA_oSortLData;

public:	
	MVVEHICLETIMEVOL *vehicle_infor;
	float image_word_angle[4];	
	int m_nShowTime; //停留显示时间(必须小于忽略时间)

public:
	MySIFT	sift;
#ifndef NOPLATE_COLOR
	PasserbyRoadColor *m_pColorRecognisize;
#endif

private:
	int  n_ref, n_ref1;
	int  m_nMaxTrNum, m_nMaxKeyPt, m_nMaxTrCntInRect;
	int  m_nMaxExtrackKeyPtNum;

	int	  keypt_count, lastKeypt_count;
	CvPoint2D32f *cornersPt, *lastCornersPt;
	uchar **m_siftDesc;

	int m_nExtPtCnt;	
	CvPoint2D32f *m_ptAExtPt;
	IplImage *m_opticalFlowPyr;
	IplImage *m_opticalFlowPyrLast;
	int      **m_opticalFlowPtFlag;


	short	**m_trImgMap;

	CvPoint2D32f *point_kalman;
	CvRect		 *match_rect;	

	int		effective_vehicle_count;
	int		vehicle_count,group_count,track_count;  //CvSet中数目，包括空位置
	int		m_nMinTrackLen;                //min track length for grouping

#ifndef UNIQUE_FLOWLINE
	float  *velocity_avg;               //平均车速
	int*   num_pass;                    //存放过线车辆的数目
	float **m_p_f_AvgVelocity;  //记录平均车速，记录一个统计周期内的
#else
	float *m_p_f_AvgVelocity;	
#endif
	bool    m_bExistJamChannel, m_bExistSlowChannel;
	bool    *m_bAChannelIsJam, *m_bAChannelIsSlow;
	double  velocity_max, velocity_min;   //最大平均车速,最小平均车速

	/***Below for traffic flow stat calculation*****/
	double   m_dAChanImgArea[20];	       //车道图像坐标体系区域
	bool     m_bHadCalChannelForeImgNum;   //是否以计算过车道中前景点数 
	int      *m_nAChannelForeImgNum;       //在车道中的前景点数
	double	 area_proportion;              //车道占有率

	long int index; 
	bool m_bQuickBuildBg;
	bool m_bGetInitBgImg;
	bool m_bBankPeoAppDet;          //是否为银行行人出现检测
	int  m_nStartAlertThres;
	long int m_nOutIndex;			//实际帧号，由外部传入
	vector<MvCarNumInfo> *m_pGetVecCarNum;	//指向传递进来的车牌信息
	vector<MvCarNumInfo> m_vecCarNumInfo;	//记录车牌信息
	
	cal	  cam; //for camera calibration
	double cam_height;//像机高度

	//for Burns bus detection
	IppiSize gray_roi;                  //the size of road region ROI
	int      gray_tlx, gray_tly;    //top-left corner of road region ROI
	CvPoint m_ptARoadCenter[ROAD_REGION_LIMIT];

	CvPoint2D32f* center_velocity_max;//最大平均车速的中心位置
	CvPoint2D32f* center_velocity_min;//最小平均车速的中心位置

	MvImage<ushort> m_MaxsizeX;
	MvImage<ushort> m_MaxsizeY;

	CvSize  m_szMinPeo, m_szMaxPeo;   //行人大小

	double  max_size_max, max_size_min, max_min_dist_inv;//表示maxsize最大值与最小值，用以调整合理性判断时的阈值
	double  m_maxSizeCar_x_max, m_maxSizeCar_x_min, m_maxSizeCar_y_max, m_maxSizeCar_y_min;
	
	//统计累计通过流量线的车辆数
#ifdef SHOW_MAINSTEP_RESULT
	int   *arrive_line_total;
	float *arrive_line_velo;
	int   *arrive_line_vid;
#endif
		
	CvSeq *bg_line;
	//线段提取
	int    prenum_lines;
	LSDRect *preburnsLines;		
	//非背景线段和背景线段
	int    m_noBgLineNum, m_BgLineNum, m_noBgShortLineNum, m_BgShortLineNum;   
	BLine  *m_noBgLine, *m_BgLine, *m_noBgShortLine, *m_BgShortLine;    
	//hvs线段
	int    m_nMergeHLineNum, m_nMergeVLineNum, m_nMergeOLineNum;
	MegerLine *m_MergeHLine, *m_MergeVLine, *m_MergeOLine;
	//线段组合
	int    m_nLine3Com;
	Line3Com  *m_lpListLine3Com;


	int   m_nGroupingTrCnt;//统计进入grouping的轨迹数，用于分配顶点和边的内存
	int   *m_pAGroupingTrackNo;

	CvPoint *m_p_trNowPt;
	int     m_nSortTrNum;
	int     *m_p_nSortXTrNo, *m_p_nSortYTrNo;
	double  *m_p_dSortXVal, *m_p_dSortYVal;


#ifdef USE_SOBEL_BGIMG
	IplImage *m_pSobelBgImg;
	IplImage *m_pLTermBgSobelImg;
#endif

	int		m_shaow_ori, m_shaow_pos;
	double  m_ts_pre_shadow, m_ts_now_shadow;
#ifdef DEBUG_SHADOW_DIR
	FILE   *fp_shadow_dir ;
#endif
	int    m_nShadowLineNum;
	int    *m_nShadowLineOriSet, *m_nShadowLinePosSet;
	double *m_dShadowLineTimeSet;
#ifdef MVDEBUGON
	long   m_nEventImgNum;
#endif

	float   m_min_img_x, m_min_img_y, m_max_img_x, m_max_img_y;
	float   m_min_world_x, m_min_world_y, m_max_world_x, m_max_world_y;	
	float   m_d_img_x, m_d_img_y, m_d_world_x, m_d_world_y;


#ifdef MVDEBUGON
	#ifdef   DEBUG_GROUPING
		IplImage *m_lpGroupImage; 		FILE   *fp_group;
	#endif
	#ifdef   DEBUG_TR_GROUPING
		IplImage *m_lpTrGroupImage;
	#endif
	#ifdef  DEBUG_ERROR_ALERT
		int       m_nOffX,  m_nOffY;
		IplImage  *m_errorAlertImg;
	#endif	
	#ifdef  DEBUG_3DMODEL
		IplImage  *m_lp3DModelAreaImg;
	#endif
	#ifdef DEBUG_VEHICLE_TYPE
		IplImage *m_vehicleTypeImg;
	#endif
	#ifdef DEBUG_LINEMERGER_CONDITION
		IplImage *m_lpLineMergerImg;
	#endif
	#ifdef 	DEBUG_SHADOW_DIR
		IplImage *m_shadowDirImg;
	#endif
	#ifdef 	DEBUG_MERGER_BIG_VEHICLE
		IplImage *m_bigVehicleMergerImg;
	#endif
	#ifdef	DEBUG_TRACK_NOJOIN_VEHICLE
		FILE     *m_joinGroupfp;
		int      m_jgIndex[200];		double	 m_jgData[200];
		int      m_joinGroupTrNum,  m_joinGroup_group_idx;
		int      m_joinGroupTrId[2000], m_TrType[2000];
		IplImage *m_joinGroupImg;
	#endif
	#ifdef   DEBUG_TIME						//各阶段耗时的文本保存
		FILE *m_fp_debugTime;
	#endif
	#ifdef   DEBUG_FRAMETIME		//各阶段耗时的文本保存
		FILE *m_fp_debugFrameTime;
	#endif
	#ifdef  DEBUG_TIME_HOG			//HOG检测耗时的文本保存
		FILE *m_fp_debugHogTime;
	#endif
	#ifdef 	DEBUG_TIME_FORE
		FILE *m_fp_debugForeTime;		//前景检测耗时的文本保存
	#endif
	#ifdef  DEBUG_TIME_TYPE			//类型判断耗时的文本保存
		FILE *m_fp_debugTypeTime;
	#endif
	#ifdef 	DEBUG_TIME_ADJU        //目标调整耗时的文本保存
		FILE *m_fp_debugAdjustTime;
	#endif
	#ifdef 	DEBUG_EVENT_ALERT			  //事件报警的文本保存
		FILE *m_fp_debugEvent;
	#endif
	#ifdef 	DEBUG_OBJAPPEAR_ALERT     //目标出现报警的文本保存
		FILE *m_fp_debugObjAppear;
	#endif
	#ifdef DEBUG_TYPE_WIN32
		FILE *m_fpDebugObjType;                   //类型判断过程的文本保存
	#endif
	#ifdef  DEBGU_TYPE
		IplImage *m_typeImg;
	#endif
	#ifdef  DEBUG_TYPE_DEFALUT
		IplImage *m_defTypeImg;
	#endif
	#ifdef  DEBUG_TYPE1
		IplImage  *m_type1Img;
	#endif
	#ifdef  DEBUG_TYPE2
		IplImage  *m_type2Img;
	#endif
	#ifdef  DEBUG_TYPE3
		IplImage  *m_type3Img;
	#endif
	#ifdef  DEBUG_TYPE4
		IplImage  *m_type4Img;
	#endif
	#ifdef DEBGU_TYPE_NIGHT
		IplImage *m_nightTypeImg;
	#endif
	#ifdef DEBUG_TYPE_CLOSE
		IplImage *m_typecloseImg;
	#endif

#endif
		

public:
	//----------系统初始化与释放与服务端的接口---------------//
	Mvroad_detect();
	~Mvroad_detect();
	bool mvSetChannelWaySetting( VerChanList& listVerChan, 
			RegionList& listStabBack, RegionList& listSkip,
			Calibration& calib, RoadList& listRoad ); 


	bool mvinit(char* config, double config2real_x=1.0,
		   double config2real_y=1.0, int nDeinterlace=1);
	bool mvuninit();
	bool mvwidthIsSet();	
	bool mvsetWidthRepeat(int64_t ts);
	bool mvtsIsSet();
	bool mvsetTs(int64_t ts);
	int  m_nSetWidthTime;
	CvSize m_szSrcImg;
	bool mvsetWidth(int width, int height);  


	//将配置读入器中的地址传递给分析类的配置读入器指针
	void mvGetConfigReaderInfoPointer( CConfigInfoReader *pCCfgReader );

	bool mvconfig_param_to_detect(paraDetectList& m_paramin, 
					ROAD_PARAM& param_road);

	//设置是否为白天
	void mvconfige_isDayParam(int nDetectTime);

	//设置是否需要检测颜色
	void SetDetectColor(bool  bDetectColor= true);      

	//设置是否检测车牌
	void SetDetectCarnum(bool  bDetectCarnum = false);   

	//传递得到远近处的行人框
	int    m_nRectPeo;
	CvRect m_rectPeoA[5];
	bool mvGetPeoRectFromClient( int nCnt, CvRect rectA[5] );

	//设置远景预置位存图数量
	void SetRemotePicCount(int nCount = 3);
	
	//设置远景预置位存图时间间隔
	void SetRemotePicInterval(int nTimeInterval = 60);	

	//设置远景近景预置位组合存图数量
	void SetRemoteNearCombPicCount( int nCount );

	//设置远景近景预置位组合存图时间间隔
	void SetRemoteNearCombPicInterval( int nTimeInterval );


#ifdef USE_STAB
	void mvGetStableAreaLineInitInfo( int nChannelId, int nPreSetId, 
			int nPolyCnt, int nPolySize[4], CvPoint2D32f **ppPolyPt,
			IplImage *srcImg, int &nLineCnt, CvPoint2D32f pALinePt1[10],
			CvPoint2D32f pALinePt2[10], int nALineMod[10] );
#endif

		
private:
	//设置图像所使用的宽度和高度
	bool mvSetImgUseWidthHight( int width, int height );

	//得到事件类所用的道路,车道，检测参数
	bool mvGetDetParam4Cfg( );

	//得到事件类所用的全局参数
	bool mvGetGlobalDetParam4Cfg( );

	//得到事件类所用的道路,车道，检测参数
	bool mvGetRoadChanDetParam( );

	//事件所用的灰度图像集合结构体
	EventUseGrayImgSet m_eventUseGrayImgSet; 

	//事件所用的maxisze图像集合结构体
	EventMaxsizeImgSet m_eventMaxsizeImgSet;

	//初始化map的值以用于快速计算
	void mvInitMapVal4QuickCalc( );

	//创建用于调试的图像
	void mvCreateDebugImage( );

	//事件所用的cvSet和cvSeq结构
	EventUseCvSetSeq  m_eventUseCvSetSeq;

public:
	bool m_bStartStat, m_bStat;
	TraffFluVelStatStru *m_pAllFluVelStat;
	TraffFluVelStatStru *m_pStraightFluVelStat;
	TraffFluVelStatStru *m_pLeftFluVelStat;


	//天津对面车道流量统计接口(含指示灯控制)
	//开始直行绿灯统计
	void StartStraightStatistic( int64_t ts );
	//结束直行绿灯统计
	void EndStraightStatistic( int64_t ts, DetectResultList& list_DetectOut );

	//开始左转绿灯统计 
	void StartLeftStatistic( int64_t ts );
	//结束左转绿灯统计
	void EndLeftStatistic( int64_t ts, DetectResultList& list_DetectOut );	

	//获取直行绿灯统计结果，此接口在EndStraightStatistic后调用
	void GetStraightStatistic( DetectResultList& list_DetectOut );

	//获取左转绿灯统计结果，此接口在EndLeftStatistic后调用
	void GetLeftStatistic( DetectResultList& list_DetectOut );	

	//目标是否确定在给定车道
	bool mvCarInChannel( MyGroup *pObj, int nChannelIdx );
	
	//将统计值赋值给输出结果
	void mvGetFluVelStaResOut( SRIP_DETECT_OUT_RESULT &outRes, 
				TraffFluVelStatStru* pFluVelSta );

	//获取进行TLD跟踪的目标区域,
	bool mvGetObjAreaToTLDTrack(vector<CvRect> &vectWantTrackRects);
	
	//判断目标区域是否为大车和中型车顶
	bool mvIsBigVehicleTopOfObjArea( );

	//目标跟踪结果的显示
	void mvShowAllVaildObjects( );
private:
	
	//setWidth内函数
	float mvQuickSqrt( float fTempV );
	float mvQuickAtan2( float fDy, float fDx );	

	//读取阴影分布信息
	//void mvReadShadowDistriMsg();
	//稳像初始化
	void mvInitStablization( );

	//标定的初始化
	void mvInitCalibration( );

	//初始化使用的道路区域及其相关信息
	void mvInitUseRoadAreasAndInfo( );

	//初始化maxsize及其相关信息
	void mvInitMaxsizeAndInfo( );
	
	//对道路和车道信息等进行扩展信息的获取
	void mvGetExtInofOfRoadChannels( );


	//初始化HoG的应用
	void mvInitHoGApplication( );


#ifdef  MVDEBUGON	
	//创建调试目录
	void mvCreateDebugDirectory( );
#endif

	//创建耗时情况保存文件
	void mvCreateTimeConsumeFiles( );

#ifdef USE_FANTATRACK
	//初始化奇异轨迹
	void mvInitFeantaTracks( );
#endif


#ifdef DEBUG_EVENTDETECT_CONFIG
	//存下调试和配置
	void mvInitEventDetectConfig( );
#endif


	//通过读文件来决定采用的使用模式
	void mvReadFileToDecideUseMod( );

	//通过读文件来决定是否调试
	bool m_bDebug;
	bool m_bDebugObjTypeJudge;
	bool m_bDebugSaveFkImgResult;
	bool m_bDebugSaveObjectResult;
	bool m_bDebugSaveObjAppResult;
	bool m_bDebugSaveEventResult;
	bool m_bDebugStab;
	bool m_bDebugVehicleStop;
	bool m_bDebugChange;   
	bool m_bDebugSaveHoGResult;  


	void	mvReadFileToDebug( const char cFileName[104] );
#ifdef MVDEBUGON
	void  mvReadFileToDebugTemp( const char cFileName[104] );
#endif
	void mvinit_globalVariable();

	void mvGetHomographyPt( float* homography_src, float *homography_dst );
	bool mvTransformCalibrationPt( float *fSrcImgX, float *fSrcImgY, 
			float *fSrcWorX, float *fSrcWorY, float *fDstImgX, float *fDstImgY, 
			float *fDstWorX, float *fDstWorY, int nMod );
	void mvfind_homography();
	float mvcreate_oriImage(CvPoint2D32f *border_point,int nPolySize,
			int nDirection,CvPoint2D32f *pCenter);

	//maxsize求取
	void   mvGetMaxMinCalibration( VEHICLE_PARAM paramChan, int len );
	void   mvsetMaxSize(/*bool isEx*/);
	void   mvsetSmallCarMaxSizeXY( );
	void   mvsetStandPeoMaxSizeXY( );
#ifndef JUSTUSEBASICCODE
	//根据头部位置得到标准行人脚部位置
	void Mvroad_detect::mvgetPeoHightFromTop2Bot( );
#endif

	CvPoint mvGetBottomPtFromTopPt( int nRoughObjH, float fObjHight, int nTopX, int nTopY );
	void mvgetMinMaxValXOf3Array(int n1, CvPoint2D32f pt[], int n2, CvPoint2D32f p2lt[], int n3, CvPoint2D32f ptlt[], 
		float &tempmin, float &tempmax);
	void mvgetMinMaxValYOf3Array(int n1, CvPoint2D32f pt[], int n2, CvPoint2D32f p2lt[], int n3, CvPoint2D32f ptlt[],
		float &tempmin, float &tempmax);
	void	mvgetMinMaxValXOf2Array(int n1, CvPoint2D32f pt[], int n2, CvPoint2D32f p2lt[], float &tempmin, float &tempmax);
	void	mvgetMinMaxValYOf2Array(int n1, CvPoint2D32f pt[], int n2, CvPoint2D32f p2lt[], float &tempmin, float &tempmax);
	CvPoint2D32f  mvgetCarMaxSizeXY( int i,int j,double mvAngle, float halfCarWidVal=1.0f,
		     float halfCarLenVal=2.4f, float ratioVal=0.9f, float midHeiVal=0.7f,float topHeiVal=1.4f );
	bool mvJudgePointMaxsizeIsHalf( double x, double y );
	
	bool mvGetMaxsizeXY( CvPoint c_pt, CvPoint &maxsize_pt, 
		               int nMod, float fObjImgMvAngle=-1000 );
	double mvgetMaxsizeWidth(int x, int y);
	double mvgetMaxsizeHeight(int x, int y);
	
	bool mvGetImgDirectionOfPt( CvPoint c_pt, float &fAngle, bool bReturnArc );

#ifndef JUSTUSEBASICCODE
	void   mvSetPeoHightFromTop2Bot(  );
#endif
	void mvcreate_road_mask(IplImage *maskImage, IplImage *pTempimage,CvPoint2D32f *border_point,
		                                     int nPolySize,int nTpye,CvRect &roadRoi);
	void mvextend_bus_mask(CvPoint2D32f *border_point,int nPolySize,int nDirection,
		                                     CvPoint2D32f *pCenter,CvPoint2D32f *border_point_s);
	void mvCreateChannelIDMask();
	void mvAdjustGrayRoi(CvRect &gray_roi);
	void mvsetPersonMaxSize(bool bAllPeoChannel, IplImage *pFlagImg);	
#ifdef USE_SAVEINFOMATION	
	bool mvSaveObjSizeImage( int nSrcW, int nSrcH, 
			   int nDstW, int nDstH, int nDrawMod ); 
	bool mvsaveConfigInfomation( );
#endif

	//在检测再次启动时，对一些信息需要进行清理
	bool m_bStartupAgain;
	void mvClearWhenStartupAgain( );


	//在屏幕上显示当前的时间戳
	void mvShowTimeStampOnScreen( const double & dTsNow );

#ifdef DEBGU_EVENT_SAVE
	//拥堵停车确认
	CvPoint2D32f m_ptAWaitMatch[256];  
#endif

	/////////////////////////////////////////////////////////////////////////
	///////////------------与其他部分接口函数及变量------------//////////////
	/////////////////////////////////////////////////////////////////////////
public:
	bool    m_bNeedCtrlCam;          //是否需要球机控制
	bool    m_bDetectColor;          //是否需要检测颜色
	bool    m_bDetectCarnum;         //是否需要检测车牌
	long    m_nEventId;              //事件的Id号
	bool    m_bInterleavedImg;       //传递给事件的图像是否为隔行扫瞄
	bool    m_bGetIfIsInterleavedImg;  //是否获取已 传递给事件的图像是否为隔行扫瞄 的信息

	int    m_nChannelId;  //通道ID
	int    m_nPreSetId;   //预置号

public:	
	void mvSetIsNeedCtrlCam( bool bNeedCtrlCam );  //是否需要球机控制
	
	void mvSetIsInterleavedImg( bool bInterleavedImg ); //传递给事件的图像是否为隔行扫瞄图

	void mvGetChannelAndPreset( int nChannelId, int nPreSetId ); //传递通道号和预置位号

	void mvCamGotoPreLoc( int64_t ts );   //球机回预置位
	bool mvNeedGotoProLoc(  );      //球机是否需回预置位


	//得到给定点的行人框大小
	bool mvGetPeoRectWithGiveVal( CvPoint pt, int nRefCnt,
					CvRect rectRefA[5], CvPoint &ptWt );


	void mvsetListImgNo(int nListImgNo);
	void mvsetImgFrameNo(int nImgFrameNo);

	int mvgetnChannel( )
	{
		return m_nChannel;
	}

	bool mvdetect_every_frame(char* image_data, int64_t ts);  //与应用端的调用接口1
	bool mvDetectEveryFrame(char *image_data,int64_t ts,
			const long int _nIndex,vector<MvCarNumInfo> &vecCarNum);
	
	bool mvget_result(DetectResultList&, bool &bNeedCamCtrl);  //与应用端的调用接口2

	//设置交通统计
	void mvSetTrafficStatistics(bool &bNeedStatistic);

	//先对车牌进行处理
	bool mvPreProcessCarnum( );
	
	bool mvget_initInfo_toScaleObj(SRIP_DETECT_OUT_RESULT &detect_out_result, 
									CMoveTracker &objMvTracker);

private:
	//球机控制相关
	void mvGetTracksOfMove2Stop( CvPoint ltPt, CvPoint rbPt, vector<int> &vctTrNo );		
	
private:
	//对给定目标进行排序
	void mvSortObjects( int **pSortObjIdx, int nObjCnt,
		                int *nAObjIdx, int nSortMode );

private:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////------------图像处理所用函数及变量------------////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//对图像进行转换
	void mvImageConvertForEventDetect( );

	//对图像进行分析
	void mvImageAnalysisForEventDetect( );

	//稳像函数的调用接口
	CvPoint2D32f m_ptSrcImgOffset;   //源图的偏移值
	CvPoint2D32f m_ptUseImgOffset;   //实验图的偏移值
#ifdef USE_STAB
	bool mvGetImgOffsetAndRecorrect(IplImage *pShakeSrcImg, IplImage **pStabSrcImg);
#endif	
		
	
	//根据平均亮度确定是白天或晚上,  默认为白天
	void mvjudge_is_day(IplImage *grayImg);
	//判断是否需要检测阴影(通过判断图像中是否有阴影存在)
	void mvjudge_is_shadow( IplImage *grayImg ); 

	
	//从分析类中获取得到关键点及其sift特征
	void mvGetKeyptsFromAnalysis( );

	//线段提取
	bool  mvLineExtract( int nIndex, int nStep, bool bGaoQing, bool bDay ); 
	//线段过滤
	void  mvBurnsLinesOrientation(BLine* burnsLines,	int num_lines);
	void  mvLineFiltrate(  );		
	void  mvSegmentUnite( IplImage *pShowImg, 
				 int nLineNum, BLine *pLines, 
				 int &h_lineNum, MegerLine *h_lineArray, 
		         int &v_lineNum, MegerLine *v_lineArray,
				 int &o_lineNum, MegerLine *o_lineArray );
	//存下水平的较长线段
	void  mvsave_vaildHoriLines(  );		
	//提取并剔除掉背景线段
	void  mvExtackAndDeleteBgLines( double dSUpTimeTh, double dLUpTimeTh );  
	bool  mvis_line_matched(BLine &line1, BLine &line2);
	void  mvdelete_bg_line(BLine *noBgShortLine, int &noBgShortLineNum, BLine *bgLines, int &bgLinesNum);
	bool  mvLineInShortBgLine(MegerLine line);
	//对线段排序
	void  mvsort_threeKindLines( int nHL,MegerLine AHL[], int nVL,MegerLine AVL[], int nOL,MegerLine AOL[] );  
	//对线段提取过滤合并排序等
	void  mvBurnsLineExtractionAndMerge( );
	bool m_bVaildLine;
	//线段检测
	bool mvdetect_LineOrBus( );

	//图像处理部分的主函数
	bool mvProcessImage_every_frame( );

//#ifndef JUSTUSEBASICCODE
	//对图像进行分割
	//#ifdef DEBUG_GET_FORIMG
		void mvdebug_ImageSegment(  );
		inline float mvget_maxRatio( float fA1[3], float fA2[3] );
		void mvget_foreRgbImg_segment( IplImage *lpSrcRgbImg, float fJoinThres, int minSizeThres, IplImage *lpMaskImg, int *pAFlag,vector<int> &vctCompSize );  //分割
		void mvshow_segment_result( IplImage *lpSrcRgbImg, IplImage *lpMaskImg, int *pAFlag, int nCompSize );
	//#endif
//#endif

	void  mvSiftMatch( int nRefPtCnt, KeyPtSiftFeat *pRefPtsFeat, 
				int nCurPtCnt, KeyPtSiftFeat *pCurPtsFeat,
				float fR, bool *bAMatch );

	/////////////////////////////////////////////////////////////////////////////
	////////////------------轨迹层所用函数及变量------------/////////////////////
	/////////////////////////////////////////////////////////////////////////////
	
	//距离定义
	float mvDTWDiff(const MyTrack tr1, const MyTrack tr2) const;
	float mvXYCosDiff(const MyTrack tr1, const MyTrack tr2) const;
	float mvCosDistance(const MyTrack tr1, const MyTrack tr2) const;
	float mvXYCosDiffW(const MyTrack tr1, const MyTrack tr2) ;
	float mvCosDiff(const MyTrack tr,const MyGroup obj)const;
	float mvCosDiff(const MyGroup obj1,const MyGroup obj2) const;

	//判断两轨迹是否为相似运动，返回true要很确定
	bool mvare_tracks_similar_motion(MyTrack &tr1, MyTrack &tr2);

	//获取非空轨迹		
	void mvget_noNullTrack_Of_trackSet( bool bAll = true );

	//通过历史速度判断该轨迹是否为误匹配
	CvSeq *pVeloHis[MAX_VEL_HIS];
	bool mvis_mismatch_by_velo_history(MyTrack &tr);

	//每帧匹配前，每条track的每帧数据依次前移
	void mvshift_track_current_frame();
	 //对当前帧做赋初值、清空等
	void mvclear_track_current_frame();
	//开始时将全部track新增到动态结构中
	void mvadd_track_first_frame();
	
	//根据最小二乘法预测轨迹的图像坐标
	CvPoint2D32f	mvtrack_get_predict(MyTrack& track, double tsdiff);
#ifndef JUSTUSEBASICCODE
	//获得轨迹的预测点的运动方向
	bool    mvget_moveAngle_for_predictPt(CvPoint2D32f predPt, MyTrack track, double &mvAngle);  
#endif

	//获得匹配框
#ifndef JUSTUSEBASICCODE
	bool	mvis_track_sure_lastFrame( MyTrack tr );
	void    mvget_lastFrame_sureTrack( int &sureTrNum, int *sureTrId );
#endif
	
	CvRect  mvcreate_match_rect(bool bCanPredict, CvPoint2D32f point, double tsdiff);
	CvRect	mvcreate_match_rect(CvPoint2D32f& point, bool flag, double tsdiff);

	//每帧匹配前，估计轨迹当前帧的坐标及匹配窗口
	void mvestimate_track_current_frame();
	
	//判断轨迹的优先级别
	bool mvis_Track_suit_level1_condition(MyTrack tr);
	bool mvis_Track_suit_level3_condition(MyTrack tr);
	void mvget_diffLevel_track_for_siftmatch(int nLevelCnt[3], int **nLevelTrId);
	//记录轨迹的当前帧位置的邻域内的角点,用于匹配
	void	mvselect_point_in_map(short** keypt_map,CvRect &rect,MyPoint* point_in_rect,int* point_count_in_rect);
	//记录轨迹的当前帧位置的邻域内的SIFT
	void	mvselect_descriptor(uchar** descriptor, MyPoint*ptA, int nKeyptCnt, MyDescriptor* );
	//新轨迹的初始化
	void mvNewTrackInit(MyTrackElem	&track_elem);
	//在sift匹配后，判断该轨迹是否该删除(较多帧或较长时间匹配不上)
	bool mvis_deleteTrack_afterSiftmatch( int nTrNo, MyTrackElem *p_track_elem );
	//处理当前的匹配结果
	void mvprocess_trackMatchResult( int nKeyPtCnt, int nFlags[], CvPoint2D32f keyPts[],
					int nExtKeyPtCnt, int nExtFlagBiaohaos[], CvPoint2D32f extKeyPts[], 	
		            uchar **descriptor, CvPoint2D32f prediectPts[] );
	
	//删除不满足给定方向的轨迹
	void mvdelete_noSuitGiveOriTrack( float fGiveAngle, float fOffsetThres );

	//显示轨迹的差异
	void mvshow_track_diff( );
	//删除多次未能匹配上的轨迹
	void mvshow_track_delete( IplImage *pTempImg, int nMod );

	//预先进行轨迹匹配,并进行分类
	void mvPreTrackMatch( vector<trMatchStru> &vctMatchTr);
	void mvPartitionPreMatchTr( vector<trMatchStru> &vctMatchTr, vector<matchTrClassStru> &vctClass );
	void mvGetPreMatchTrMainLabel( int nLabelClassCnt, vector<trMatchStru> &vctMatchTr, vector<int> &vctMainLabel, vector<int> &vctMainLabelCnt );
	
	//每帧对轨迹和角点进行sift匹配,并判断轨迹是否删除
	void mvmatch_track_withPreMatchInfo( vector<int> &vctMainLabel, vector<int> &vctMainLabelCnt, vector<matchTrClassStru> vctMatchTrClass );
	void mvmatch_track_current_frame( );

	//对轨迹当前点为观察前一点为估计的节点坐标进行修正
	void mvmodify_estimateNodesCoordinate_ofTrack( MyTrack& tr );

	//判断轨迹近期是否为停车的
	bool mvIsTrackRecStop( MyTrack &tr );
	//获取得到轨迹中一些消失点的信息
	bool mvget_track_disappearPtInfo( MyTrack &tr );
	//判断轨迹是否由动到静
	bool mvIsTrRecMove2Stop( MyTrackElem *pTrackElem );
	//获取得到停止的车上的轨迹
	bool mvGetStopCarTracks( vector<CvPoint> vctStopPt, double tsNow );  
	
	//对指定的轨迹删除及删除判断，若和背景满足自相关则删除
	bool mvdelete_likeBgTrack_withCNM( int nTrNo, MyTrackElem* pTrackElem, IplImage *pImg=NULL);
	//得到轨迹的一些属性值,放在一起是因为取轨迹较为耗时
	void mvget_someProperty_ofTracks( int nIndex, bool bDelTr=true);	

	//利用已有的由运动到静止的轨迹来得到由运动到静止目标的静止轨迹
	void mvGetRecStop4M2STr( int nMv2StopTrNum, IplImage *pMapImg );

	//判断track的运动方向是否一致
	bool mvis_track_motion_consistency(MyTrack& tr);
	//判断track的运动是否有跳变
	bool mvis_track_motion_jump( MyTrack& tr, bool &bM2S, bool &bS2M, bool &bJ2S, bool &bS2J );
	//判断拥堵的track的跳变的可疑性
	void mvis_track_determined(MyTrack& tr);
	//判断track的拥堵和确定性
	void mvjudge_track_jam_current_frame();

	//标记真的跳变轨迹
	void mvSignTrueJumpTrack( vector<int> vctJumpDTrNo,
			vector<int> vctJumpMod, vector<bool> &vctTrueJump );

	//寻找出背景轨迹
	void mvFindBgTrack();		

	//轨迹形成部分的主函数
	bool mvFormTrack_every_frame( );
	bool mvGetTracks4PtsMatchTracker( );

	//传递轨迹信息给网格方法
	void mvGetPointMatchTrInfo(StruAreaPointTrackInfo &struAreaPtTrInfo);

#ifdef	DEBUG_MATCHTR
	IplImage *m_pTrMatchShowImg;
#endif
#ifdef  DEBUG_TR
	IplImage *m_pTrShowImg;
#endif

	simpleTrInfo *m_pASimpleTrInfo;  //track简单信息结构数组
	//保存track的一些简单信息
	void mvsave_trackSimpleInfo( const MyTrack &tr, simpleTrInfo &vctASimTrs );
	//保存tracks的一些简单信息
	void mvsave_tracksSimpleInfo( int nTrNum, int nATrNo[], simpleTrInfo ASimTrs[]  );

	//标记为删除或背景的track,从车辆删除;标记为jam的强制保留
	void mvdelete_track_from_object( simpleTrInfo ASimTrs[] );

	//每帧grouping后，首先处理目标中过快或过慢的轨迹
	void mvremove_fastSlowTr_from_object( simpleTrInfo ASimTrs[] );

#ifndef JUSTUSEBASICCODE
	#ifdef USE_FANTATRACK
		fantTrUseInfo m_fantTrUseInfo;
		float mvCalTrack_recentImgOri( MyTrack tr );
		int  mvAccumulateTrackOriHist( int y, int x, float fImgOri, int nYToH, int nXToW, int ***pOriHist, int ***pOriFST );
		bool mvIsFantasticOfTrack( float fImgOri, int nY, int nX, int nBinNo, int ***pOriHist, int ***pOriFST, int nFantMod );
		void mvget_fantTracks( int nTrSetElemNum,MyTrackElem* pATrackElem[], int nATrOriBinNo[], float fATrImgOri[] );
		void mvget_fantTracksIndex( int nTrSetElemNum, MyTrackElem* pATrackElem[], int nXBlock, int nYBlock, 
			                                         int nMaxTrNum, int *nATrNum, int **nAATrNo, int nMod );
	#endif
#endif
	
	bool	mvdelete_track( int track_index, MyTrackElem *p_track_elem );
	void	mvdelete_track( simpleTrInfo ASimTrs[] );
	bool mvis_track_alone_in_neighbor( int t_idx, MyTrack &tr, int win );
	void mvDeleteMismatchTrackCurrentFrame( );
	bool mvget_overImgBottom_track( MyTrackElem* pATrackElem[], int &trNum, int *trId );
	bool mvget_overImgBottom_track( int &trNum, int *trId );
	bool mvis_track_consistency_by_history(simpleTrInfo tr1, simpleTrInfo tr2,int min_common_length);	    
	bool mvjudge_track_rationality_size(simpleTrInfo tr1, simpleTrInfo tr2, int nObjType);
	bool mvis_differentOf2Track(MyTrack tr1, MyTrack tr2, float fMaxXRatioThres, float fMaxYRatioThres);	

	bool  mvget_mvTrack_for_bigVehMerger( int &nMvTrNum, int *nMvTrId, int nTrNumThres );	
	bool  mvget_moveTrack_for_bigVehMerger( int &nMvTrNum, int *nMvTrId, int nTrLenThres, int nTrNumThres );
	bool  mvget_stopTrack_for_bigVehMerger( int &nStopTrNum, int *nStopTrId, int nTrLenThres, int nStopLenThres, int nTrNumThres );

	bool  mvget_track_in_rectArea(int &nTrNum, int *nTrId, CvRect rect, int &nInRectTrNum, int *nInRectTrId);
	float mvcal_trackLineOri( MyTrack tr ,int nLen = 0);
	bool  mvfilter_tracks_noInRect(CvPoint pt_lt, CvPoint pt_rb, int &nTrNum, int *nATrId);
	bool  mvfilter_tracks_noInRect(MyTrackElem* pATrackElem[], CvPoint pt_lt, CvPoint pt_rb, int &nTrNum, int *nATrId)	;	

	void mvget_mainOri_of_track_inarea(CvPoint2D32f lt_pt, CvPoint2D32f rb_pt, float &mainHistImgOri, float &mainHistWrdOri);
	bool  mvJudge2TrackInObjOnHistory(simpleTrInfo ASimTr[], int nTrNo1, int nTrNo2, MyGroup obj);
	bool  mvJudgeTwoTrackInVehicleOnHistory(MyTrack tr1, MyTrack tr2, MyGroup vehicle);
	
#ifdef MVDEBUGON
	FILE *fp_error;
	void mvjudge_track_jam_current_frame_debug();
	void mvdebug_track_property(char cClassName[50]);
#endif

	void mvdelete_doubt_against_track();
#ifndef JUSTUSEBASICCODE
	int  m_nDiffNo;
	void mvjudge_different_of_tracks( );
	void mvset_lineId_to_tracks(int &nTrNum, int *pATrLineNo);
#endif

	bool  mvget_rectPt_of_tracks(int nTrNum, int *nATrId, CvPoint &ltPt, CvPoint &rbPt);
	bool  mvget_rectPtOfTracks(MyTrackElem *pATrackElem[], int nTrNum, int nATrNo[], CvPoint &ltPt, CvPoint &rbPt);

	bool  mvget_sameOriTr_inRect(int nMvTrNum, int *nAMvTrId, float fObjTrOri, CvPoint pt_lt, CvPoint pt_rb, 
		int &nSameOriTrNum, int *nASameOriTrId, int &nSameOriSureiTrNum, int *nASameOriSureTrNo);	
	bool mvGetSameOriTrInRect(int nMvTrNum, int *nAMvTrId, const MyGroup obj, CvPoint pt_lt, CvPoint pt_rb, 
		int &nSameOriTrNum, int *nASameOriTrId, int &nSameOriSureiTrNum, int *nASameOriSureTrNo);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////------------group形成层所用函数及变量------------///////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//每帧grouping前的初始化
	void mvgroup_initialize_current_frame( simpleTrInfo ASimTrs[] );
	//判断轨迹所对应的角点是否为前景
	void   mvis_bgCornerOfTrack( simpleTrInfo ASimTrs[] );
	//判断轨迹是否在一个目标的附近，且与该目标的运动基本一致
	bool mvis_trackAroundAndConsObj( CvPoint trNowPt, MyTrack tr, 
			int &nObjNum, int pAObjNo[], 	
			CvPoint pAObjltPt[], CvPoint pAObjRbPt[], 
			float pAObjVelo1[], float pAObjOri[] );
	//更新map图,用于grouping时找邻域
	bool   bLetNoMvTrJoinGrouping;
	void   mvLetNoMvTrJoinGrouping( int &nGroupingTrCnt, int pACanGroupingTrId[ ] );
	void   mvcreate_map_grouping_new( simpleTrInfo ASimTrs[] );
	void   mvshow_grouping( );

	//将cvSet中的一些轨迹存到数组中
	void	  mvsave_tracksInSetToArray( int nTrNum, int nTrNo[], MyTrack **pATracks );
	//获取在区域范围内的点的序号
	bool  **m_p_bAUse;
	bool mvget_inRangePtNo( int nNum,	int nAIndexYNo[], double dAIndexYVal[], int nAIndexXNo[], double dAIndexXVal[], CvPoint ptA[],
		    int nAInYRangeTrNo[], bool **p_bMap, double ly,double hy, double lx,double hx, int &nInNum, int nAInNo[] );
	//判断两轨迹的速度区别的函数，用于轨迹的聚类分析
#ifndef JUSTUSEBASICCODE
	float	 mvdiff_velocity(MyTrack& track1, MyTrack& track2);
	float  mvdiffValue_ofTwoVelocity(float velo1, float v_x1, float v_y1, float velo2, float v_x2, float v_y2);
#endif
	inline float mvget_diffValOf_twoTrackVelocity( MyTrack tr1,  MyTrack tr2);

	//判断两轨迹之间是否存在水平线段或竖直线段或斜线段
	bool    mvis_existLine_bettween_TwoPoint(CvPoint2D32f pt1, CvPoint2D32f pt2, bool bAExistLine[3]);
	//求取两轨迹的差异值
	float  mvget_diffVal_for2Track( 
			MyTrackElem* pATr1, MyTrackElem* pATr2, 
			float fXThres=1.0f, float fYThres=1.0f);	
	float  mvGetDiffVal4PeopleTrack(
			MyTrackElem* pATr1, MyTrackElem* pATr2, 
			float fXThres=1.0f, float fYThres=1.0f);

	//两两计算轨迹之间的差异值
	void  mvcal_diffValBetweenTracks( int nTrNum, int nATrNo[], int &nVerticeNum, point4D *coors, int &nEdgeNum,edge *edges );
	void  mvshow_cal_diffValBetweenTracks( int nTrNum, int nATrNo[], int &nVerticeNum, point4D *coors, int &nEdgeNum, edge *edges );

	//利用图对轨迹进行聚类
	universe  *mvsegment_graph(int num_vertices,int num_edges,edge *edges,point4D *coors,float thres) const;
	IplImage  *m_pTrDiffWightImg;
	universe  *mvsegment_graph_strict(int num_vertices,int num_edges,edge *edges,point4D *coors,float thres) ;
	universe  *mvsegment_graph_new(int num_vertices,int num_edges,edge *edges,point4D *coors,float thres) const;

	//对轨迹作grouping聚类
	void mvsegment_image(float thres);

	//判断grouping后的group是否为临时group
	bool   mvis_tempGroup(MyGroup group, simpleTrInfo ASimTrs[]);
	bool   mvis_tempGroup(GroupingResult group, simpleTrInfo ASimTrs[]);
	bool   mvis_wrong_group_with_trOri(MyGroup group, int nTrSetElemNum,MyTrackElem* pATrackElem[]);
	bool   mvis_wrong_group_with_trDis(MyGroup group, int nTrSetElemNum,MyTrackElem* pATrackElem[]);

	//对grouping后的group进行新处理
	void   mvGetGroupInfoBeforeProcess( int &nGroupNum, int **p_nAGroupId, double **p_dAGroupSize );
	void   mvprocess_group_result_new( simpleTrInfo ASimTrs[], IplImage* pTrDiffWightImg );


#ifndef JUSTUSEBASICCODE
	#ifdef	DEBUG_TRACK_NOJOIN_VEHICLE
		int mvget_flagOfTrNoJoinGrouping( MyTrack tr, simpleTrInfo simTr );
	#endif
#endif
#ifdef MVDEBUGON	
	void mvDegbugTrMatch( IplImage *pShowImg, int nStep );
	void mvDegbugGrouping( IplImage *pShowImg, int nStep );
#endif

	//grouping的主函数
	bool mvFormGroup_every_frame( simpleTrInfo ASimTrs[] );
	
	//银行行人跟踪
	bool mvGroupingHumanTracks( simpleTrInfo ASimTrs[] );
	universe * mvsegment_track_graph( int num_vertices,
		   int num_edges, edge *edges, point4D *coors, 
		   float thres ) const;
	void mvProcessGroupingResult( int &nGroupNum, 
		  int **pp_nAGroupIdx, double **pp_dAGroupSize );

	//对跟踪的区域进行HoG行人检测
	bool mvHumanDetectAndTrack( );

	//对跟踪的区域进行HoG行人检测
	void mvGetHoGDet4TrackArea( 
		   bool bTrackFirst   //是否采用跟踪优先(默认为检测优先)
		);

	//获取得到TLD跟踪区域的当前信息
	void mvGetTLDRectsCurInfo( );

	//获取最好的HoG行人检测区域
	void mvGetBestHumanHoGDetArea( vector<CvRect> &vRctHoGDet, 
			int nGroupNum, int *nAGroupIdx,	double *dAGroupSize );


	//将HoG检测得到的区域转换为跟踪的区域
	CvRect mvConvertHoGDetAreaToTrack( const CvRect &rctHoGDet );
	

	//针对银行检测行人出现的报警函数
	IplImage *m_pTempCopyRgbImg;
	void mvAlertPeopleAppearOfBank( DetectResultList &detOutList );


	//对目标越过警戒线进行报警
	vector<int>    m_vIdAlertObject;  //报警的目标ID
	vector<double> m_vTsAlertObject;  //报警目标的时间戳
	bool mvBankHumanAppearAlert(	
			vector<bool> &vIsShow,       //OUT:
			vector<CvPoint> &vPtPeoCet,  //OUT:
			int  nIndex,				 //IN:
			bool bDrawResult = false     //IN:是否绘制结果
		);

	//该函数用于--判断行人是否越过了警戒线 
	bool mvIsPersonOverWarningLine( 
			const OneTLDTrackResult &TLDRes,   //行人目标
			CvPoint *linePts,		  //警戒线
			bool bDrawResult = false  //是否绘制结果
		);

	//该函数用于--判断行人是否出现 
	bool mvIsPersonAppear( 
			const OneTLDTrackResult &TLDRes,   //行人目标
			bool bDrawResult = false  //是否绘制结果
		);

	//对目标越过警戒线进行报警
	bool mvAlertObjectCrossLines( int nIndex );

	////////////////////////////////////////////////////////////////////////
	/////////------------目标形成层所用函数及变量------------///////////////
	////////////////////////////////////////////////////////////////////////

	void mvget_noNullVehicle_Of_groupSet( bool bAll = true );
	int	mvget_matched_size_of_vehicle(MyGroup &vehicle);
	int	mvget_matched_size_of_vehicle(simpleTrInfo ASimTrs[], MyGroup &obj);
	int mvGetObjectValidTrCnt(const MyGroup &vehicle);
	int	mvGetObjectValidTrCnt(simpleTrInfo ASimTrs[], MyGroup &obj);
	void mvmax_watchTime_of_ObjTr( MyGroup &vehicle,
			int &nMaxWatchTime, int &nCorTrTime );
	float mvmax_vehicle_displace( const MyGroup &vehicle, short eff_len, 
			float *mean_dist=NULL,float *max_dist3 =NULL, int todo3 = 0 );
	float mvmax_object_displace( simpleTrInfo ASimTrs[], const MyGroup &obj, 
			short eff_len, float *mean_dist=NULL,float *max_dist3 =NULL, int todo3=0 );	
	float	mvcalculate_vehicle_area(MyGroup& group1);
	float	mvcalculate_vehicle_overlap(MyGroup& group1, MyGroup& group2);

	//预测本帧的目标
	void mvpredict_personArea_withHoG( );     //利用HoG检测来预测行人区域

	void mvpredict_vehicleArea_withFkInfo( );  //利用前景信息来预测车辆区域

	//删除本帧中所有应当删除的目标
	void mvdelete_vehicle();
	void mvdelete_vehicle(MyVehicleElem *pObjElem, int nObjNo, 
						  bool is_delete_track);
	void mvdelete_vehicle(simpleTrInfo ASimTrs[], MyVehicleElem *pObjElem, 
						  int nObjNo, bool is_delete_track);
	
	//检验拥堵车辆的区域是否有运动的轨迹
	void mvcheck_vehicle_jam( simpleTrInfo ASimTrs[] );

	//对目标轨迹按group进行分类和排序
	void	mvsort_in_vehicle(int *split_group_id, int *split_group_count,
							  int *nCnt, int *nUnknownCnt);
	
	//目标更新
	void	mvupdate_objects( simpleTrInfo ASimTrs[] );
	void    mvupdate_objects_20110813( simpleTrInfo ASimTrs[] );


	void	mvupdate_vehicle(MyGroup &group);
	void	mvupdate_vehicle( simpleTrInfo ASimTrs[], MyGroup &group );
	void	mvinit_vehicle_status(MyGroup &vehicle,bool is_bus);
	void	mvupdate_group_size(MyGroup &group);
	void    mvupdate_group_size(simpleTrInfo ASimTrs[], MyGroup &group);
	void	mvupdate_vehicle_status(MyGroup &group);
	void    mvupdate_vehicle_status(simpleTrInfo ASimTrs[], MyGroup &group);

	//目标合理性判断
	int		m_nNoRation;
	bool	mvis_vehicle_large( MyGroup &group, uchar ori );	
	bool	mvjudge_vehicle_rationality_border(const MyGroup &group);
	bool	mvjudge_vehicle_rationality_size(MyGroup &group, 
				bool use_estimate = false, short type = 1);
	bool	mvjudge_vehicle_rationality_velocity(MyGroup &group);
	bool	mvjudge_objectRationality_velocity(simpleTrInfo ASimTrs[], MyGroup &obj);
	bool	mvjudge_vehicle_rationality_by_vehicle_history(const MyGroup &vehicle);
	bool	mvjudge_objRationality_by_objHistory(simpleTrInfo ASimTrs[], const MyGroup &obj);
	bool	mvjudge_vehicle_rationality_by_track_history(const MyGroup &group);

	bool	mvjudge_vehicle_rationality(MyGroup &group,
				bool use_estimate = false, short type=1);
	bool	mvjudge_object_rationality(simpleTrInfo ASimTrs[], MyGroup &obj,
				bool use_estimate = false, short type=1);

	//将不合理的车目标分开
	void mvsplit_vehicle1_new( simpleTrInfo ASimTrs[], 
			int vehicle_index, int first_id);
	void mvsplit_vehicle2_new( simpleTrInfo ASimTrs[], 
			int vehicle_index, int first_id);	
	void mvsplit_objects( simpleTrInfo ASimTrs[] ) ;

	//添加轨迹到目标中	
	void mvget_centerPoint_for_Group(MyGroup group, CvPoint &ct_pt);
	bool mvAddOneTrack_to_vehicle(MyVehicleElem *p_elem, int v_idx, int tr_idx);
	bool mvAddSomeTracks_to_oneVehicle( simpleTrInfo ASimTrs[], 
			MyVehicleElem *p_elem, int v_idx, float scale_x, float scale_y,
			int trNum, int *tr_idxA, int &noAddTrNum, int *noAddTr_idxA=NULL);
	
	//从目标中删除轨迹	
	void mvdelete_track_from_vehicle(int track_index, MyTrack& track);
	void mvdelete_track_from_vehicle(simpleTrInfo ASimTrs[], 
			int track_index, MyTrack& track);

	void  mvget_unKownTrack( simpleTrInfo [], MyGroup , 
			int , int &, int [], int &, int [], int [] );
	//将group新增到vehicle集合中
	bool mvadd_group_to_vehicle_prejude(MyGroup& group);
	void mvadd_group_to_vehicle(MyGroup& group, bool is_new = true);
	void mvadd_group_to_vehicle(simpleTrInfo ASimTrs[], MyGroup& group, bool is_new = true);

	//目标继承
	void mvinherit_vehicle_attrib_real(MyGroup& vehicle_dst, 
			MyGroup& vehicle_reserve, MyGroup& vehicle_delete);
	bool mvinherit_vehicle_attrib(MyGroup& vehicle_dst, 
			MyGroup& vehicle_reserve, MyGroup& vehicle_delete);
	void mvinherit_vehicle_attrib(MyGroup& vehicle_dst, 
			MyGroup& vehicle_src);

	//将group合并到目标中去
	void mvcreate_temp_vehicle(MyGroup &vehicle_i, 
		MyGroup &vehicle_j, MyGroup &group_tmp);
	void mvSetVehicleIndexOfTrack(MyGroup &vehicle_i, 
		MyGroup &vehicle_j);
	void mvSetVehicleIndexOfTrack(simpleTrInfo ASimTrs[], 
		MyGroup &vehicle_i, MyGroup &vehicle_j);
	bool mvmerge_grouping_to_vehicle(MyVehicleElem *p_elem,
		MyGroup &group, int v_idx);
	bool mvmerge_grouping_to_vehicle(simpleTrInfo ASimTrs[], 
		MyVehicleElem *p_elem, MyGroup &group, int v_idx);

	//两目标合为一个
	bool mvExistBlankBettwenTwoVehicle(MyGroup &obj_i,MyGroup &obj_j, 
		MyGroup &obj_temp, float &x1,float &y1, float &x2,float &y2);
	void mvCalculateLineAreaOfVehicle( MyGroup &group, 
		int &h_lineNum, int &v_lineNum, float rectA[4]);
	bool mvIfNeedCheckHorLineAreaForMerger(MyGroup vehicle_i,
		MyGroup vehicle_j);
	bool mvmerge_vehicle_small( simpleTrInfo ASimTrs[],
		int i, int j, MyVehicleElem *&, MyVehicleElem *& );

	//根据group结果来调整目标
	int  mvGetGroupingTrCntOfObj( MyGroup obj, bool bAGrouping[] );
	bool mvAdjustGroupToObject( simpleTrInfo ASimTrs[], MyGroup& group );
	bool mvAdjustGroupToObject_20110809( simpleTrInfo ASimTrs[], 
		bool bAGrouping[], MyGroup& group );
	void mvadjust_group_to_objects( simpleTrInfo ASimTrs[] );  
	void mvadjust_group_to_objects_20110809( simpleTrInfo ASimTrs[] );

	//调整大小不合理的目标
	bool mvis_distanceOf2Pt_biggerMaxsizeXY( CvPoint2D32f pt1,
		CvPoint2D32f pt2, int nObjType );
	void mvcal_track_nonRatTime_with_distance(MyGroup vehicle, int *count);
	void mvcal_track_nonRatTime_with_distance(simpleTrInfo ASimTrs[], 
		MyGroup vehicle, int *count);
	void mvsplit_vehicle_by_velocity(simpleTrInfo ASimTrs[], 
		MyGroup& vehicle, MyGroup& group_tmp, int v_idx);
	void mvadjust_non_rational_vehicle(simpleTrInfo ASimTrs[],
		MyGroup& vehicle, int v_idx);

	//目标合并
	int  m_nPreNoRation;
	bool mvIsSuitMergeSize( MyGroup& obj_i, MyGroup& obj_j, bool &bNeedCheckSize );
	bool mvIsWantMerObjSuitSizeRationality( 
		MyGroup& vehicle_i, MyGroup& vehicle_j, bool &bCheckSize );
	bool mvcompare_vehicle_by_history(int v_his_idx1, int v_his_idx2);
	bool mvmerge_vehicle_prejudge(int i, int j, MyGroup& vehicle_i, MyGroup& vehicle_j);
	bool mvmerge_vehicle_prejudge_20110817(int i, int j, MyGroup& vehicle_i, MyGroup& vehicle_j);

	bool mvmerge_smallObj(simpleTrInfo ASimTrs[],int i, int j,
		MyVehicleElem *&pElem_i, MyVehicleElem *&pElem_j, MyGroup& tmp);
	bool mvmerge_largeObj(simpleTrInfo ASimTrs[],int i, int j, 
		MyVehicleElem *&pElem_i, MyVehicleElem *&pElem_j, MyGroup& tmp);
	void mvmerge_objects( simpleTrInfo ASimTrs[] );
    void mvmerge_objects_20110816( simpleTrInfo ASimTrs[] );

	//删除一些目标
	void mvremove_vehicle_on_bgline();
	void mvremove_small_vehicle_near_large();
	void mvremove_isolated_motion_jump();
	
	//保存目标历史
	void	mvsave_vehicle_history();
	void mvsave_velo_history();
	void mvget_vehicle_result_from_history();
	bool mvget_vehicle_result_from_history( MyGroup vehicle );

	//目标形成的主函数
	void	mvdetect_objects( simpleTrInfo ASimTrs[] );

	//目标确定性判断
	void	mvjudge_vehicle_determined(MyGroup &vehicle);
	void	mvjudge_new_vehicle_determined(MyGroup &vehicle);
	void	mvjudge_new_vehicle_determined(simpleTrInfo ASimTrs[], MyGroup &vehicle);
	void	mvjudge_vehicle_determined_remove(MyGroup &vehicle);
	void	mvjudge_vehicle_determined_remove(simpleTrInfo ASimTrs[], MyGroup &obj);


	//预测目标是否存在分碎的部分
	void   mvIsObjectHaveSplitPart( );

	//////////////////////////////////////////////////////////////////////////////
	/////////////--------------目标调整所用函数及变量--------------///////////////
	//////////////////////////////////////////////////////////////////////////////	
	//目标调整判断的主函数
	void mvadjust_objects( simpleTrInfo *pASimpleTrInfo, vector<fkContourInfo> vctForeContours );

	//获取目标调整前的初始信息
	void mvinit_objectAdjustInfo( object_adjust_info &objAdjustInfo );
	bool mvget_vaildImgAngle_withObjBest3Tr( int nABestTrNo[3], simpleTrInfo ASimTrs[], float &objMvImgAngle );
	bool mvget_mvImgAngle_bestTrPts_ofObj( MyGroup obj, float &objMvImgAngle, int nALongTrNo[3]  );
	void mvget_nowObj_matchLtRbPt( MyGroup obj, simpleTrInfo ASimTrs[], int &nLen, CvPoint ltPtA[], CvPoint rbPtA[] );	
	//合并时的要求
	bool mvis_suitSizeRequire_forMerger( object_adjust_info , object_adjust_info , int , int  );
	bool mvis_suitVeloRequire_forMerger( object_adjust_info , object_adjust_info , float , float );
	bool mvare_twoObj_suitHistoryVeloRequire( MyGroup , MyGroup , float , float , float , int &, bool bASuit[] );
	bool mvare_twoObj_suitHistoryWHRequire( MyGroup , MyGroup , 	float , float , int &, bool bASuit[] );
	bool mvis_distanceTooFar_of2Tr(MyTrack tr1, MyTrack tr2, float fMaxXRatioThres, float fMaxYRatioThres);
	bool mvis_distanceTooFar_of2ObjLtRbPts( int ,CvPoint [],CvPoint [], int ,CvPoint [],CvPoint [],  float , float  );
	bool mvis_existDifferent_of2TrDiffImgPt(simpleTrInfo tr1, simpleTrInfo tr2, int nComLen);
	bool mvare_existDifferent_ofTrsDiffImgPt( simpleTrInfo ASimTr[], int nTrNum, int nATrNo[], int nComTrNum, int nAComTrNo[], int nComLen );
	bool mvis_existDifferent_of2TrDiffVelo(simpleTrInfo tr1, simpleTrInfo tr2, int nComLen, float fXThres, float fYThres, float fVThres);
	bool mvare_existDifferent_ofTrsVelcocity( simpleTrInfo ASimTr[], int nTrNum, int nATrNo[], int nComTrNum, int nAComTrNo[], 
		                                                              int nComLen, float fXThres, float fYThres, float fVThres );
	bool mvare_twoObj_suitVeloRequire( MyGroup vehicle_i, MyGroup vehicle_j, float v_scale, float v_x_scale, float v_y_scale );

	void mv_add_oneTrack_toVehicle(int nTrNo, MyTrack &tr, int nWantAddVehicleNo);
	void mvadd_oneTrackToObject( int nTrNo, MyTrack &tr, int nWantAddVehicleNo, int &nUpdateRemObjNo, int &nUpdateAddObjNo );

	//轨迹相似性判断
	bool mvis_similar_trackAndObjTracks(MyTrack tr, MyGroup obj, float fXRatioThres, float fYRatioThres, float fSmilarThres);
	bool mvis_similar_trackAndObjTracks(MyTrack tr, int nObjTrNum, MyTrack *pAObjTracks, 
		                                                       float fXRatioThres, float fYRatioThres, float fSmilarThres);
	bool	mvis_similar_trackAndTracks(MyTrack tr, int nTrNum, MyTrackElem* pATrackElem[], 
		                                                  float fXRatioThres, float fYRatioThres, bool bHard );
	bool mvare_twoTracks_motion_similar( MyTrack &tr1, MyTrack &tr2 );	
	bool mvare_tracks_motion_similar( int nTrNum, int nATrNo[], int nComTrNum, int nAComTrNo[] );		
	bool mvare_twoObj_suitTrack_condition(MyTrackElem* pATrackElem[], MyGroup obj, MyGroup splObj);
	bool mvare_twoObj_suitTrack_condition2(MyTrackElem* pATrackElem[], float fObjOri, MyGroup obj, MyGroup splObj);
	bool mvis_track_diffWith_object_withDisplace(MyTrackElem* pATrackElem[], int nTrId, MyTrack tr, MyGroup obj, float fThres=2.0f );
	bool mvis_track_diffWith_object_with_xDist(MyTrackElem* pATrackElem[], int nDir, int nTrId, MyTrack tr, MyGroup obj );

	CvPoint mvget_twoObject_edgeDist(MyGroup obj1, MyGroup obj2);
	float mvget_foreImgPercent_of_hSegement(IplImage *foreImg, int sx, int ex, int y);
	float mvget_foreImgPercent_of_vSegement(IplImage *foreImg, int sy, int ey, int x);
	bool mvare_twoObj_suitForeImg_condition(IplImage *foreImg, CvPoint carMaxsizeXy_pt, MyGroup obj, MyGroup splObj);

	void mvget_2rectFor_2waitMergerObj(object_adjust_info obj1, object_adjust_info obj2, CvRect &rect1, CvRect &rect2);
	bool mvare_2objRect_suitLineRequire(object_adjust_info adjObj1, object_adjust_info adjObj2);

	void mvget_objectAdjustBasicInfo( MyGroup obj, object_adjust_info &objAdjustInfo, simpleTrInfo ASimTrs[] );
	void mvget_objectCanMergerObjNo( int nMod, int nObjNo, int nComObjNum, int *nAComObjNo, object_adjust_info *pAObjectsAdjustInfo );

	float mvget_object_trackOri( MyGroup obj );
	float mvget_object_trackOri( MyGroup obj, MyTrackElem* pATrackElem[] );
	float mvget_object_trackOri_new( MyGroup obj, int nVaildTrNumThres );


	//获得调整前的基本信息的主函数
	void mvget_objectsAdjustBasicInfo_befAdj( object_adjust_info *pAObjectsAdjustInfo, simpleTrInfo ASimTrs[] );
	void mvget_objectsNo_forNeedMergerJudge( object_adjust_info *pAObjectsAdjustInfo, 
		int &nAllObjNum, int *nAAllObjNo, 	int &nObjNum, int *nAIndex, double *dASize, int nMod );
	void mvmerger_twoObject( int nObjNo1, MyVehicleElem *pObjElem1, int nObjNo2, MyVehicleElem *pObjElem2, bool &bAfterOneBeMerger );

	//小车合并调整的主函数
	void mvadjust_objects_forSmlVehicle( object_adjust_info *pAObjectsAdjustInfo, simpleTrInfo ASimTrs[] );
	//行人合并调整的主函数
	void mvadjust_objects_forPeople( object_adjust_info *pAObjectsAdjustInfo, simpleTrInfo ASimTrs[] );
	//大车合并调整的主函数
	bool mvis_foreImgDistributeSuitBigVehicle( fkContourInfo foreContour, IplImage *pIntegrateForeImg );
	bool mvis_existBigVehicle_withForeImg( vector<fkContourInfo> vctForeContours, IplImage *pIntegrateForeImg,
		vector<int> &vctExistNo,	vector<int> &vctHLNo, vector<int> &vctVLNo, vector<int> &vctOLNo,	vector<int> &vctSuitContoursNo );
	bool mvis_existBigVehicle( vector<fkContourInfo> vctForeContours, IplImage *pIntegrateForeImg );
	void mvadjust_objects_forBigVehicle( object_adjust_info *pAObjectsAdjustInfo, simpleTrInfo ASimTrs[] );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////------------线段组合进行大车检测和调整所用函数及变量------------/////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void  mvfree_lineComResult_memory(lineCom_result &lineComResult);

	//按区域划分结果获取得到各划分区域内的线段
	void mvget_subArea_lines( SubAreaStr &subAreaStr, float fInstersectThres);
	void mvget_subArea_longLines( SubAreaStr &subAreaStr, float fHLThres, float fVLThres, float fOLThres );
	void mvget_subArea_suitRequire_linesNum( SubAreaStr subAreaStr, float fHLThres, float fVLThres, float fOLThres,
		                                                                int &nHLineNum, int &nVLineNum, int &nOLineNum );
	bool  mvget_cluster_lines_withLen(SubAreaStr *lpSubArea, int nAHLineNum[], int nAVLineNum[], int nAOLineNum[]);
	void mvSortWithLines(SubAreaStr *lpSubArea, int nAhLNo[],double dAhLD[], int nAvLNo[],double dAvLD[], int nAoLNo[],double dAoLD[]);
	//区域线段进行3边组合
	double mvget_minDist_Of_PtAndLineEndPt(MegerLine line, double pt_x, double pt_y );
	bool  mvadd_Line3Com_toList(int nType, int nL1No,int nL2No,int nL3No, CvPoint pt, Line3Com &tempLine3com);
	bool  mvget_Line3Com(SubAreaStr *lpSubArea, int nHLineNum[], int nVLineNum[], int nAhLNo[],double dAhLData[],
		                                 int nAvLNo[],double dAvLData[], int nAoLNo[],double dAoLData[]);

	bool  mvget_1LH2V_lineCom(SubAreaStr subArea, int &nComNum, rectangle_1LH2V *ARectangle1LH2V, int nMaxComNum);
	bool  mvget_xRange_with_1LH2V(SubAreaStr subArea, int &minX, int &maxX);
	bool  mvget_1LV2H_lineCom(SubAreaStr subArea, int &nComNum, rectangle_1LV2H *ARectangle1LV2H, int nMaxComNum);
	bool  mvget_yRange_with_1LV2H(SubAreaStr subArea, int &minY, int &maxY);
	bool  mvget_1H1V1O_lineCom(SubAreaStr subArea, int &nComNum, switch3_1H1V1O *ASwitch3, int nMaxComNum);

	void  mvget_minmaxXY_withLine(MegerLine line, CvPoint2D32f &pt_lt, CvPoint2D32f &pt_rb);
	bool  mvis_line_inRect(MegerLine line, CvPoint2D32f &pt_lt, CvPoint2D32f &pt_rb);
	bool  mvget_1H1V1X_lineCom(SubAreaStr subArea, float fHLThres, float fVLThres, float fOLThres,  
		                 float fDistXLThres, float fDistYLThres, int &nComNum, switch3_1H1V1X *ASwitch3);
	bool  mvget_2ParaO_lineCom(SubAreaStr subArea, int &nComNum, parallel_OLine *AParaO, int nMaxComNum);

	//区域线段3边组合体进行确认
	bool  mvget_lines_of_Line3Com(Line3Com tempLine3Com, MegerLine megerL[3]);
	bool  mvget_thePointNo_of_switch3(Line3Com tempLine3Com, int nLineType[3], int nLineNo[3], bool bPositiveXYO[3]);
	bool  mvis_switch3_suitLenRequire(Line3Com tempLine3Com, MegerLine megerLine[3]);
	void  mvget_rect_of_lines(int nLineNum, MegerLine *pMegerLines, CvPoint &lt_pt, CvPoint &rb_pt);
	void  mvget_extRect_of_hvoSwitch3(CvPoint &ltPt,CvPoint &rbPt, CvPoint intersectPt, MegerLine hLine,MegerLine vLine,MegerLine oLine);
	bool  mvis_existLine_onEndPt_of_hvoSwitch3(Line3Com hvoSwitch3, SubAreaStr *lpSubArea,				  
		int nAhLNo[],double dAhLData[],	int nAvLNo[],double dAvLData[], int nAoLNo[],double dAoLData[],
		int &nLineNum,  int *pSwitch3EndPtId, int *pLineType, int *pLineNo);
	bool  mvexist_longLine_in_rect(Line3Com vert, SubAreaStr *lpSubArea, int nAhLNo[],double dAhLData[],	
		int nAvLNo[],double dAvLData[], int nAoLNo[],double dAoLData[], int &nLineN, int *pLineType, int *pLineNo);
	bool  mvis_trueParal3_ofRect(Line3Com parall3, CvPoint lt_pt, CvPoint rb_pt);
	//对线段组合体内的目标和轨迹进行处理
	void  mvget_vaildObject_andSortWithSize( int &nVaildObjNum, int nAVIdx[], double dASize[] );
	void  mvget_noNullTracks_andAddress( int nTrNum, int nATrNo[], bool bATrIsNull[],  bool bAHadAdd[], MyTrackElem* pATrackElem[] );
	void  mvget_best3TrOfObj( MyGroup obj, int &nBestTrNum, int nABestTrNo[3] );
	bool  mvis_trackVehicle_suitRequire(MyTrack tr, MyGroup vehicle, float fThres=0.5f);
	//线段组合
	bool  mvjudge_Line3Com_inBigV( );
	void  mvfilter_overlap_Line3Com( );
#ifndef JUSTUSEBASICCODE
	void  mvfilter_nosuit_trackAndForeImg_Line3Com( );
#endif
	//线段组合进行大车检测的主要步骤
	bool  mvLine3ComDetect_with_lines( );
	void  mvfilter_some_line3Com(int &nLine3ComNum, int *lpLine3ComIndex);
	bool  mvadd_trackTo_object_of_Line3Com(int nLine3ComNum, int* lpLine3ComIndex);
	
	//线段组合进行大车检测的主函数
	bool  mvdetect_bigVeh_withLineCom( );


	/////////////////////////////////////////////////////////////////////////////
	/////////---------大车上分碎目标合并的所用函数及变量-------//////////////////
	/////////////////////////////////////////////////////////////////////////////
	bool  mvget_joinBigVJudge_ObjNo(int &nVidxNum1, int &nVidxNum, int *nAVIdx, double *dASize, IplImage *pImg=NULL);
	void  mvinit_bigVMegerTr( bigVMegerTrInfo &tr );
	void  mvGetMoveTrAndStopTr( MyTrackElem* pATrackElem[], bigVMegerTrInfo ABigVMegerTr[], int nTrNum, int nATrNo[], 
		int &nMvTrNum, int &nRecStTrNum, int nAMvTrNo[ ], int nARecStTrNo[] );
	float  mvget_tracksAvgOri( MyTrackElem* pATrackElem[], int nBestTrNum, int nABestTrNo[], bool &bRecentStop );
	bool  mvjudge_objIsBigVehicle(MyTrackElem* pATrackElem[], bigVMegerTrInfo ABigVMegerTr[], 
		int nVidxNum, int nAVIdx[], int nHadJudgeNum, 
		int &nExistNum, mayBigSplVeh_info AMayBigSplVeh[]);
	CvPoint mvGetCarMaxsizeXyWithOri( CvPoint cetPt, float fObjTrOri );
	bool  mvis_objTracks_similarWithTracks(MyTrackElem* pATrackElem[], int nComTrNum, int nAComTrNo[], 
		MyGroup smlObj, float fXRatioThres, float fYRatioThres, float fSmilarThres);
	
	//判断目标是否为大车上的分碎目标
	bool mvis_bigSplObj_withLineCross(CvPoint lt_bigO_pt, CvPoint rb_bigO_pt, CvPoint lt_splO_pt, CvPoint rb_splO_pt);
	bool mvis_partOf_bigObj_withLine(MyTrackElem* pATrackElem[], mayBigSplVeh_info bigSplVeh, int splVeh_idx, lineCom_result lineComResult);
	float mvcal_overlap_percent(CvPoint lt_pt1, CvPoint rb_pt1, CvPoint lt_pt2, CvPoint rb_pt2);
	bool  mvis_partOf_bigObj_withOverlap(MyTrackElem* pATrackElem[], mayBigSplVeh_info bigSplVeh, int splVeh_idx, lineCom_result lineComResult);
#ifndef JUSTUSEBASICCODE
	bool mvis_partOf_bigObj_withOri( MyGroup big_veh, MyGroup spl_veh, mayBigSplVeh_info  mayBigVeh );
	bool mvis_partOf_bigObj_withTrack( MyGroup big_veh, MyGroup spl_veh, mayBigSplVeh_info mayBigVeh, int nMvTrNum, int *nMvTrId );
	bool mvis_partOf_bigObj_withDiff( mayBigSplVeh_info  mayBigVeh, MyGroup big_veh, MyGroup spl_veh );
#endif
		
	//大车上分碎目标合并的主要步骤
	bool mvis_exist_bigVAndSplO(MyTrackElem* pATrackElem[], bigVMegerTrInfo ABigVMegerTr[], 
		                                             int &nBigVehNum, mayBigSplVeh_info *AMayBigSplVeh);
	bool mvGetVehicleAreaWithLine(CvPoint &lt_pt, CvPoint &rb_pt);
	bool mvget_lineComResult_of_subArea( SubAreaStr subArea, lineCom_result &lineComResult );
	void mvadd_trackInRect_toBigVehicle( MyTrackElem* pATrackElem[], int nBigVNo, int nBestTrNum,MyTrackElem* pABest3TrackElem[],
		                                                        CvPoint lt_pt,CvPoint rb_pt, int nTrNum,int ATrNo[], bool bAHadAdd[] );
	bool mvjudge_SplVeh_IsPartOf_bigVeh( MyTrackElem* pATrackElem[], mayBigSplVeh_info bigSplVeh,
		                                                           int splVeh_idx, lineCom_result lineComResult );
	void mvupdate_mergerVehicle(int nSaveVidx,int nDeleteVIdx, MyVehicleElem *pBigVElem,MyVehicleElem *&pSplVElem);

	//大车上分碎目标合并的主函数
	bool mvDetectMegerBigSmlVehicle(  );

	//将标记为大车轨迹的轨迹加入到大车中
	void mvadd_bigVTrack_toBigV( );

	int  m_bigSplVehNum;
	mayBigSplVeh_info  *m_bigSplVeh;  

	bool mvObjectIsBig( MyGroup &vehicle, float k_x, float k_y );
	bool mvis_bigSplObj_withLine(MyGroup big_veh, MyGroup spl_veh, SubAreaStr subAreaStrA, mayBigSplVeh_info mayBigVeh);	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////-----------3D模型进行大车检测和调整所用函数及变量------------////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef JUSTUSEBASICCODE
	#ifdef USE_3DMODEL
		int      m_3DModelBigVNum;
		double   *m_p_tsGetModelBigV;
		int      *m_p_3DModelBigVPtNum;
		CvPoint2D32f  **m_p_3DModelBigVPts;
		CvPoint2D32f  **m_p_3DModelExpectPts;
		bool mvget_bigVehicle_with3DModel(int &nPolygonNum, int nPolygonPtsNum[5], CvPoint nPolygonPts[5][8]);	
		bool mvgetAndExpect_positionOf_3DModelBigV(int &nPolygonNum, int nPolygonPtsNum[5], CvPoint nPolygonPts[5][8]);
		void mvgive_switch3_to_3DModel(int &nSwitch3Num, bigVehicle_switch3*lpSwitch3);
		void mvgive_bigVArea_to_3DModel(int &nBigVAreaNum, bigVehicle_bigVArea *lpBigVArea);
		void mvgive_camParam_and_modelSize(strcutBusInfo &busInfo, float fW,float fL,float fH);
		void mvgive_linesinfo_to3DModel(strcutBusInfo &busInfo, structSortedLineInfo &sortedLineInfo);
		void mvfree_memory_businfoStruct(strcutBusInfo &busInfo, structSortedLineInfo &sortedLineInfo);


		long m_n3DModelId;
		void mvget_theBusNumAndPostion(int &nBusNum, int nABusPtNum[5], CvPoint ptABusPts[5][8]);
		void mvsign_theTrack_For3DModelBus(int nBusNum, int nABusPtNum[5], CvPoint ptABusPts[5][8]);
		void mvadd_3DModelTrack_toBigV( );
		void mvsignAndAdd_track_ofBigVehicle2( );

		void mvget_polygon_from_pts(int npts, CvPoint2D32f *pts, int &nPolygon, CvPoint *ptsPolygon);

		bool  mvget_3DModel_withSwitch3( Line3Com line3Com,  bigVehicle_switch3 &switch3, 
			double dHig, double xyz_w[4][3], double xy_i[4][2] );

	#endif
#endif
				
		///////////////////////////////////////////////////////////////////////////////////////////////////
		bool mvget_rectCetPt_forePerc_withIntegrateImg(IplImage *IntegrateForeImg, CvPoint cetPt,
			     CvPoint lt_pt,CvPoint rb_pt,  float &fPercentX,float &fPercentY);
		bool  mvget_edge_with_foreImgPerc(int nCetNum, int nPercNum, float *fPerc, int &nSma, int &nBig);
		bool  mvget_noBgRectArea(IplImage *integrateForeImg, CvPoint cetPt, CvPoint &lt_pt, CvPoint &rb_pt);
		bool  mvis_bigVehicle_with_foreImg(IplImage *integrateForeImg, CvPoint lt_pt, CvPoint rb_pt, float fThres);
		bool  mvget_track_findArea(MyGroup obj, CvPoint pt_wh, float fObjTrOri, CvPoint &pt_lt, CvPoint &pt_rb);
		bool mvget_similarTr_inRect( MyTrackElem* pATrackElem[], CvPoint pt_lt, CvPoint pt_rb, int nBestTrNum, int ABestTrNo[],
			     bigVMegerTrInfo ABigVMegerTr[], int nTrNum, int nATrNo[],
			     int &nSimilarTrNum, int nASimilarTrNo[], int &nSureSimilarTrNum, int nASureSimilarTrNo[] );	 
		bool  mvis_ptRect_overlap_with_rects(CvPoint pt_rb, CvPoint pt_lt, int nRectNum, CvRect *rectA, float fThres=0.75f);
		bool  mvis_area_suit_bigV_lineReq(SubAreaStr &subArea);	
		bool  mvget_mayBigV_info(mayBigSplVeh_info mayBigSplVeh, mayBigVStrcuct &bigVehicle);
		bool  mvjudge_bigVehicle(int nVidxNum, int *nAVIdx, int nHadJudgeNum, 
			int &nExistNum, mayBigSplVeh_info *AMayBigSplVeh, IplImage *lpDrawImg=NULL);
		bool mvget_splObjOfBigVeh( MyTrackElem* pATrackElem[], int nBigVehNum, mayBigSplVeh_info AMayBigSplVeh[] );
		bool mvis_exist_bigVAndSplO(int &nBigVehNum, mayBigSplVeh_info *AMayBigSplVeh );
		bool mvis_objectVaildTrack_inSet(MyGroup obj, int nTrNum, int *nATrId, float fThres);
		bool mvsuit_objectTrackInSet(MyTrackElem* pATrackElem[], MyGroup obj, bool bAHad[], float fThres);
		void mvget_twoTypeVeh_area( CvPoint cet_pt, mayBigSplVeh_info &mayBigSplVeh );
		bool mvget_hLine_hHist( int nHLNum, int *nAHLId, int nOneHistLen, int nHistNum, int *nHistLineX );
		bool mvget_vLine_hHist( int nVLNum, int *nAVLId, int nOneHistLen, int nPerCnt, int nHistNum, int *nHistCnt );
		bool mvget_vLine_vHist( int nVLNum, int *nVHLId, int nOneHistLen, int nHistNum, int *nHistCnt );
		bool mvget_bigVehicleHead( mayBigSplVeh_info &mayBigSplVeh, SubAreaStr  subArea );
		bool mvlocate_bigVehicle( mayBigSplVeh_info &mayBigSplVeh );
		bool mvget_mayBigSplVeh_info( int  nBigVId, mayBigSplVeh_info  &mayBigVeh);
#ifdef DEBUG_MERGER_BIG_VEHICLE
		IplImage  *m_tempGrayImg; 
#endif
		double  *m_tsBVMvToBt;	
#ifdef	DEBUG_EVENT_CROSS
		IplImage *m_lpEventDebugImg;
#endif		

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////------------类型判断所用函数及变量------------/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	int  m_nNowPointObjTypeInfo;
	int  m_nAObjTypeInfoNum[SAVETYPEINFOFRAMENUM];
	object_typeJudge_info* m_pAAObjTypeInfo[SAVETYPEINFOFRAMENUM];

	//(1)获取本帧目标的类型判断的基本信息
	void mvget_typeJudgeBasicInfo_everyframe( object_typeJudge_info *pAObjectsTypeInfo, IplImage *pIntegrateFatForeImg, 
			IplImage *pIntegrateStrongVSobelForeImg, IplImage *pIntegrateStrongHSobelForeImg,
			IplImage *pIntegrateWeakVSobelForeImg, IplImage *pIntegrateWeakHSobelForeImg);

	//判断前景是否满足为行人宽度
	bool mvIsFkImgSuitPeoWidth( IplImage *pFkImg, object_typeJudge_info &objTypeInfo );

	//获取本帧满足前景要求的线段
	void mvGetSuitFkRequireLines( IplImage *pInteFkImg, int nALinesCnt[3], int nALineNo[3][1000] );

	//(1.0)对目标的类型判断的基本信息进行初始化
	void mvinit_object_typeInfo( object_typeJudge_info &objTypeInfo );
	
	//(1.2)获取得到候选行人区域	
	bool mvGetScanBlockLocation( CvPoint areaLtPt, CvPoint areaRbPt, int nBlockW, int nBlockH,
		    int nYBin, int nXBin, int nAY1[], int nAY2[], int nAX1[], int nAX2[] );      //获取扫瞄时所分块的位置	
	bool mvIsSuitSinglePeopleShape( float fAHPercent[10], float fAVPercent[8] ); //获得修正后的单个人的中心
	void mvGetCandidatePeopleArea( CvPoint cetPt, CvPoint peoMaxsizeXyPt, IplImage *pInteeFatFkImg,
		    IplImage *pInteWeakVSobelImg, IplImage *pInteWeakHSobelImg, 
		    vector<int> &vctPeoCetMod, vector<CvPoint> &vctPeoCetPt, IplImage *pImg=NULL );
	//(1.3)获得目标的sobel信息
#ifdef	USE_SOBEL
	void mvget_hvSobleDiffInfo_ofObj( object_typeJudge_info &objTypeJudgeInfo, bool bPeopelArea );	
#endif
	//(1.4)获得目标的线段信息
	//获得目标的水平线段信息
	bool mvget_hLineInfo_ofObj(  MyGroup obj, object_typeJudge_info &objTypeJudgeInfo, CvPoint &lt_pt, CvPoint &rb_pt );
	//获取得到目标区域内的竖直线段信息
	bool mvget_vLineInfo_ofObj( MyGroup obj, CvPoint lt_pt, CvPoint rb_pt, object_typeJudge_info &objTypeJudgeInfo);
	//获取得到目标区域内的斜线段信息
	bool mvget_oLineInfo_ofObj( MyGroup obj, CvPoint lt_pt, CvPoint rb_pt, object_typeJudge_info &objTypeJudgeInfo);

	//获得目标的组合线段信息
	bool mvget_comLineInfo_ofObj( MyGroup obj, CvPoint lt_pt, CvPoint rb_pt, 
		 object_typeJudge_info &objTypeJudgeInfo, int nALineCnt[3], int nALineNo[3][1000], bool bAMod[3] );

	//(2)利用本帧目标的类型判断的基本信息来判断目标的类型
	void mvget_sureType_thisFrame( object_typeJudge_info *pAObjectsTypeInfo );
	//(2.1)利用本帧目标的类型判断的基本信息获取较为肯定的目标类型
	void mvget_sureType_withBasicInfo_everyframe( object_typeJudge_info *pAObjectsTypeInfo );
	//(2.1.2)判断本帧该目标是否肯定为车
	void mvis_sureVehicleType( object_typeJudge_info &objTypeInfo );
	
	//获取存在车灯对的目标区域
	bool mvGetExistPairLampObjArea( 
			vector<int> &vectLampObjIdx, //车灯所对应的目标序号
			int nAreaCnt, const int *nAObjIdx, //给定的目标数目及其序号
			const CvPoint *ptAObjLt, const CvPoint *ptAObjRb //目标区域左上点/右下点
		);

	//---------对所有的目标进行是否为灯光的判断-------//
	bool mvJudgeIsLampLight4AllObjs(  );

	//------------事件检测和车辆阴影方向检测的接口-------------//
#ifdef USE_SHADOW_DETECT
	void mvDetectVehicleShadowDirection(  );
#endif
	void mvGetShowderDirectionDetect(object_typeJudge_info *pAObjectsTypeInfo);
	bool mvis_distribute_uniform( int nBin, float fARate[] );   //判断所给数组中元素的值是否分布较均匀
	bool mvis_distribute_focusOnLocal( int nBin, float fARate[] ); 	//判断所给数组中元素的值是否集中在局部	
	//(2.1.3)判断本帧该目标是否肯定为人
	void mvis_surePeopleType( object_typeJudge_info &objTypeInfo, vector<int> vctLikePeoContoursNo, 
		vector<fkContourInfo> vctForeContours, IplImage *lpIntegrateMapImg, IplImage *pImg=NULL );
	bool mvIsAreaFkImgSuitPeoWidth( IplImage *pFkImg, CvPoint maxsize_pt, CvPoint areaLtPt, CvPoint areaRbPt );
	bool mvis_surePeople_withForeImg( object_typeJudge_info objTypeInfo );
	bool mvget_likePeopleContours( vector<fkContourInfo> , vector<int> & );
	bool mvis_surePeople_withForeImgAndContours( object_typeJudge_info , vector<int> , vector<fkContourInfo>  );

	//(2.2)使用简单HOG来判断目标的类型
	void mvjudge_objectsType_withSimpleHog( object_typeJudge_info *pAObjsTypeInfo, vector<int> vctNeedJudgeTypeObjNo );
	//(2.2.1)获取得到需要进行HOG检测的目标序号
	void mvget_objectsNo_waitfor_hogDetect( IplImage *pInteFkImg, object_typeJudge_info *pAObjsTypeInfo, vector<int> , vector<int>& );	
	//(2.2.2)获取Hog检测时所需要的信息
	void mvget_information_forHogDetect( IplImage *pInteFkImg, object_typeJudge_info&objTypeInfo,
		    float fExtX, float fExtY, CvPoint &cetPt, CvPoint &maxsize_pt, CvPoint &areaLtPt, CvPoint &areaRbPt );
	bool mvget_peopleArea_forHog( IplImage *pIntegrateForeImg, object_typeJudge_info objectTypeInfo, 
		    CvPoint &judgeLtPt, CvPoint &judgeRbPt );
	//(2.2.3)简单HOG检测行人
	int     m_nStaticHogCalNum;
	//判断是否存在行人,并返回其个数和位置
	bool  mvisExistPeopleWithSimpleHoG( CvPoint maxsizePt, CvPoint areaLtPt, CvPoint areaRbPt, 
		                           vector<float>& vctPredictVal, vector<CvRect>& vctRectPeople );

	//计算行人HoG特征得分值
	float  mvGetPredictValueOfHoGFeature( float fHogSvmDetectot[], float fADescriptors[] );
	//利用简单HoG判断图像某区域是否为行人
	bool  mvGetPeoWinWithSimplexHoG(float *fPeoDetector,IplImage *&pHogImg, CvSize winStride,
		  float fThres, int &nWindow, int &nPeoWindow, CvRect APeoRect[], float fAPredictScore[] );
	//保存简单HOG的检测结果
	void mvSaveHoGResultToObjeTypeInfo( vector<float> vctVal, vector<CvRect> vctRect, object_typeJudge_info &ObjTypeInfo );


	//(3)利用本帧和多帧的目标类型信息来判断目标类型
	void mvjudge_objectsType_withFrames( object_typeJudge_info **ppAObjTypeInfo, int nNow, vector<int> &vctTypeObjNo );
	//(3.1)修改本帧目标的类型信息
	void mvmodify_objectTypeProperty( MyGroup &obj, int nMod );
	//(3.2)根据道路区域,车道类型和本帧类型判断结果,判断目标本帧确定目标类型
	void mvDetermine_SureType( object_typeJudge_info &objTypeInfo, int nRoadMod );
	//(3.3)利用多帧信息来对目标进行类型判断和确认
	bool mvIsSureVehicleWithMulFrame( MyGroup obj, int nNum, object_typeJudge_info* pATypeInfo[], int nRoadMod );
	bool mvIsSurePeopleWithMulFrame( MyGroup obj, int nNum, object_typeJudge_info* pATypeInfo[], int nRoadMod );
	int    mvGetObjTypeWithMultiFrameInfo( MyGroup &obj, int nNum, object_typeJudge_info* pATypeInfo[], int nRoadMod );
	//(3.4)利用位置信息来判定目标类型
	int mvget_objDefType_with_postion(MyGroup vehicle, int nRoadMod);

	int mvis_workman_or_noMotorVehicle(MyGroup obj);  //行人和非机动车类型判断

	//(4)利用本帧和多帧的目标类型信息来判断目标类型
	void mvget_peopleArea_forHog( IplImage *pIntegrateForeImg, 
				object_typeJudge_info *pAObjectsTypeInfo, 
		        vector<int> vctObjNo, vector<bool> &vctModifyArea ); 
	bool mvis_existPeople( CvPoint maxsize_pt, CvPoint areaLtPt, CvPoint areaRbPt, 
		         vector<CvRect>& vctRectPeople, vector<int>& vctHogSureMod );
	int mvjudge_objIsPeople_withHog( IplImage *pIntegrateForeImg,  MyGroup &obj,
			 object_typeJudge_info &objTypeInfo, vector<CvRect>& vctRectPeople,
			 MvHOGDescriptor hog, float fHitThres=1.0f, 
			 int nGroupNumThres=2, float fScale=1.1f );
	void mvjudge_objectsType_withHog( object_typeJudge_info *pAObjectsTypeInfo, 
			vector<int> vctNeedJudgeTypeObjNo );

#ifndef JUSTUSEBASICCODE
	void mvjudge_object_type_output( object_typeJudge_info  *pAObjTypeInfo );
	void mvfprint_objectType_judgeInfo( object_typeJudge_info  *pAObjTypeInfo );
	void mvdebug_objTypeJudgeWindow( object_typeJudge_info  *pAObjTypeInfo, bool bSave, char *winNamStr );
#endif
	bool mvIsSureVehicleWhenDetectEvent( const MyGroup &obj, int nNum,
			object_typeJudge_info* pATypeInfo[], int nRoadMod );
	bool mvIsSurePeopleWhenDetectEvent( const MyGroup &obj, int nNum,
			object_typeJudge_info* pATypeInfo[], int nRoadMod );
	int  mvjudge_objType_onDetectedEvent_new(const MyGroup &obj, int nNum,
			object_typeJudge_info* pATypeInfo[], int nRoadMod );

	//类型判断的主函数
	void mvjudge_object_type(  );   

	//获取目标的信息
	MvMultiFrameObjInfo  m_multiFrameObjInfo;
	void mvGetObjectInfo(  );

	//清理之前的目标信息
	void mvClearBeforeObjInfo( double dTimeThresh );
	//获取目标的基本信息
	void mvGetObjectBasicInfo( MvObjBasicInfo *pObjBasicInfo );
	//获取目标的其他信息
	void mvGetObjectOtherInfo( MvObjBasicInfo *pObjBasicInfo );

	//判断目标是否为行人
	void mvJudgeObjectIsPerson(		
			vector<float> &vctPeoScore,	   //HoG行人的得分
			vector<CvRect> &vctRectPeoDst, //在实验图中位置
			MyGroup &obj,				   //给定的目标
			MvObjMaxsizeBasicInfo *pBI4ObjMaxSz, //目标的MaxSz信息
			MvObjAreaBasicInfo *pBI4ObjArea,     //目标的区域信息
			const CvRect &rectBestPeoArea,       //最好的行人候选区域
			const CvSize &szImg,		   //实验图像大小
			IplImage *pShowHoGDetImg= NULL //HoG检测绘制图像
		);

	//获得认为是人的最好区域
	CvRect mvGetBestPeopleArea4Obj(	MyGroup &obj,
				MvObjMaxsizeBasicInfo *pBI4ObjMaxSz, 
				MvObjAreaBasicInfo *pBI4ObjArea,
				IplImage *pFkGrad8UImg,
				IplImage *pFkGrad32SImg, 
				IplImage *pPeoWeight32SImg,
				IplImage *pHoGDetImg = NULL 
		);


	//获取得到与该目标为同一目标的以往类型信息的地址
	void mvget_typeInfo_ofSameObj( int nNowIndex, int nObjNo,
			int objId, int nFrameNum, int &nNum, 
			object_typeJudge_info* pATypeInfo[] );
	void mvjudge_objType_onDetectedEvent( MyGroup &obj, int nNum, 
			object_typeJudge_info* pATypeInfo[], int nRoadMod );
	bool mvIsWrongPersonByNightLamp( object_typeJudge_info* pTypeInfo );
	bool mvConfirmIsPerson( int nObjNo, const MyGroup &obj, int nNum, 
			object_typeJudge_info* pATypeInfo[], int nObj2PerRate );
	int  mvConfirmObjTypeOnDetectedEvent(const MyGroup &obj,
			int nObjNo, int nRoadMod, int nPointer, 
			int nObj2PerRate=2, int nObj2VehRate=2);
	void mvGetRecentSureType( MyGroup &obj, int nObjNo, int nPointer, 
			bool &bSureVehicle, bool &bSurePerson );

	void mvGetVaildTrack( int nTrLenThres, vector<mvTrStruct> &vctFkTr, 
			float fTrDistThres, vector<mvTrStruct> &vctMvTr );
	void mvSetMapAndInteImgOfVaildMvTr( vector<mvTrStruct> &vctMvTr,
			int nXStep, int nYStep,	IplImage *pMapImg,
			IplImage *pIntegrateMapImg );

	void mvGetHvDiffSobleRateOfRect(CvPoint ltPt1, CvPoint rbPt1, 
		IplImage *pIntegVSobelDiffImg, IplImage *pIntegHSobelDiffImg, 
		int nBin, float fAColAreaVSobelRate[], float fARowAreaVSobelRate[], 
		float fAColAreaHSobelRate[], float fARowAreaHSobelRate[]);

	//计算各行列的运动轨迹数目
	bool mvGetExCarAreas( int nDivX, int nDivY, int n, CvPoint cetPtA[], CvPoint maxsizeXyPtA[], CvPoint ltPtA[], CvPoint rbPtA[] );
	bool mvGetPeoAreas( int nDivX, int nDivY, int n, CvPoint cetPtA[], CvPoint maxsizeXyPtA[], CvPoint ltPtA[], CvPoint rbPtA[] );
	bool mvCalMvTrNumOfRowColBin( CvPoint ltPt, CvPoint rbPt, IplImage *lpIntegrateMapImg,
		int nBin, int nAColMvTrNum[ ], int nARowMvTrNum[ ] );
	//判断sobel是否满足行人形状
#ifdef	USE_SOBEL
	int    mvSobelSuitPeoShapeMod( int nCarAreaMvTrSum, CvPoint carMaxsizeXyPt, CvPoint carLtPt, CvPoint carRbPt,
		CvPoint peoMaxsizeXyPt, CvPoint peoLtPt, CvPoint peoRbPt );
#endif
	bool mvIsMvTrDistributeSuitPeoShape( int nPeoAreaMvTrSum, int nCarAreaMvTrSum,
		int nACarAreaColMvTr[], int nCarAreaRowMvTr[] );
	bool mvis_surePeople_withVaildMvTrs( object_typeJudge_info objTypeInfo, IplImage *lpIntegrateMapImg, bool bASurePeo[3] );

	bool mvGetHoGPersonArea( MyGroup obj, int nObjNo, int nPointer, CvPoint &peoLtPt, CvPoint &peoRbPt );
	bool mvGetPersonArea( MyGroup obj, int nObjNo, CvPoint &peoLtPt, CvPoint &peoRbPt );
	//获取得到当前帧的行人区域
	bool mvGetAndPredictPersonArea( MyGroup obj, int nObjNo, int nPointer, 
		CvPoint ptAPeoLt[2], CvPoint ptAPeoRb[2],  double dTsNow, double dTsNext );

	bool mvGetFkVehicleArea( MyGroup obj, int nObjNo, int nPointer, CvPoint &vehicleLtPt, CvPoint &vehicleRbPt );
	bool mvGetVehicleArea( MyGroup obj, CvPoint &vehicleLtPt, CvPoint &vehicleRbPt );

	//判断目标是否为机动车,根据该目标的以往历史确定地判为车的次数
	bool mvis_object_motor( int nObjNo, MyGroup obj, bool bHard, bool &bDetectMultiTimeMotor, int &nMod );
	//得到进行统计的目标区域
	void mvGetCandidateObjArea(vector<CvRect> &vctCanShowderArea);
	void mvGetKeyPtInShadowImg(IplImage *pRgbImage, IplImage *pShadowImg,int nKeyPtCnt, CvPoint2D32f keyPts[],
		CvSet *vehicle_result_set,int vehicle_count);
	void mvFindBoderPoint(IplImage *pTempShadowImg,
		int nKeyPtCnt,
		CvPoint2D32f keyPts[],
		float fRateW, 
		float fRateH,
		vector <CvPoint> &vctBoderPoint,
		vector <CvPoint> &vctNoBoderPoint
		);
	bool mvis_vehicle_motor(MyGroup &vehicle);//车辆是否为机动车
	bool mvis_vehicle_motor1(MyGroup &vehicle); //车辆是否为机动车
	bool mvis_vehicle_motor2(MyGroup &vehicle);//车辆是否为机动车

	bool mvis_vehicle_motor_withHoriLine(MyGroup vehicle, int &nInterHoriLineNum, int *nInterHoriLineIDA,
		int &nInterAndSuitHoriLineNum, int *nInterAndSuitHoriLineIDA );
	bool mvis_vehicle_motor_withHoriLine(MyGroup vehicle);
	bool mvis_truePeople_withLongLine(MyGroup &obj);

	bool mvdetectPeople_withHog(IplImage *detectImg, MvHOGDescriptor &hog, int &nPeopleNum, CvRect *rectPeople);
	bool mvdetectPeople_withHog( IplImage  *detectImg, MvHOGDescriptor &hog, vector<CvRect> &vctRectPeople, bool  bASureIsPeople[3],
		float fDetectedThres = 0.9f, CvSize sizeStepXy = cvSize(8,8), CvSize sizePaddingXy = cvSize(2,2),
		float fScale = 1.1f, int nDetectdPeopleNumThres = 2, float fMergerThres = 0.2f );
	bool mvjudge_object_ispeople(MyGroup obj, int &nPeopleNum, CvRect rectPeople[10]);


	double mvget_intervalTime_forHogDetect(double dTsNow, MyGroup obj);


#ifndef JUSTUSEBASICCODE
	//找人头
	void mvgetHeadTopRect( IplImage *pForeImg, IplImage *pInteForeImg, IplImage *pInteSobelImg, IplImage *pFlagImg,
				CvSeq *counters, CvSeq *pOutPutRect, CvMemStorage *childstorage );	
	void mvgroupRect( CvSeq *pSeqFk, CvMemStorage *childstorage );
	bool mvcalObject( IplImage *pForeImg, IplImage *pInteForeImg, IplImage *pInteSobelImg );

#endif


//#ifndef JUSTUSEBASICCODE
	CvPoint mvget_topBootomPt_ofContourPts( vector<CvPoint>pAPts, int nTopPtNo );
	CvPoint mvget_leftRightPt_ofContourPts( vector<CvPoint>pAPts, int nTopPtNo );

	void mvfind_leak_human( IplImage *pFore2VImg, IplImage *pIntegrateFore2VImg );
	void mvfind_human( IplImage *pFore2VImg, IplImage *pIntegrateFore2VImg );
	bool mvfind_human( IplImage *pIntegrateFore2VImg, CvPoint ltPt, CvPoint rbPt, CvPoint peoSizePt,
		int nStepY, int nStepX, float fPrecentThres, int nFindThres );

	bool mvGetCandiHuman( IplImage *pFore2VImg, IplImage *pIntegrateFore2VImg, vector<fkContourInfo> vctFkContours );
//#endif
	bool mvis_obj_closeto_roadAreaedge(MyGroup obj, CvPoint maxSize_pt, float fThres1, float fThres2);
	bool mvget_carRect(MyGroup vehicle, float veh_angle, CvPoint &lt_pt, CvPoint &rb_pt, bool bSide[4], bool &bCloseToRoadArea);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////------------事件报警前进行操作的函数及变量------------////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//得到事件检测和行为分析前轨迹和目标的信息
	bool mvget_ObjInfo_befGetResult( int nTrSetElemNum,MyTrackElem* pATrackElem[], MyGroup obj, objInfo_befResult &objInfo ); 	
	void mvget_trObjInfo_befGetResult( int nTrSetElemNum,MyTrackElem* pATrackElem[], int nObjSetElemNum,
		                                                     MyVehicleElem* pAObjectElem[],objInfo_befResult *pAObjInfo );	
	//计算轨迹的准确方向,用于判断逆行和横穿
	void mvget_realDirectionOfObjTrack( int nTrSetElemN,MyTrackElem* pATrackElem[], int nObjSetElemN,MyVehicleElem* pAObjectElem[] );
	//获取得到轨迹的新的属性
	void mvGetNewPropertyOfTrack(  );
	//更新得到轨迹的平均速度
	void mvgetTrackAvgVelocity( int nTrSetElemNum,MyTrackElem* pATrackElem[] );  
	//计算轨万迹的确定帧数内的最大位移和最小位移
	void mvget_maxAndMinDist_ofTracks(int nCalTrLen,int nTrSetElemNum,MyTrackElem* pATrackElem[]);
	//获取到目标位于哪个车道内
	void mvlane_judgement( );	
	void mvget_objChannel( int nTrSetElemNum,MyTrackElem* pATrackElem[], int nObjSetElemNum,MyVehicleElem* pAObjectElem[] );
	//获取图像中用于事件报警时所有的有运动轨迹
	bool mvget_mvTrack_for_eventAlert(int &nMvTrNum, int *nMvTrId, int nTrLenThres, float fTrDistThres);
	bool	mvget_mvTrack_for_eventAlert(int nTrSetElemNum, MyTrackElem* pATrackElem[], 
		                int &nMvTrNum, int *nAMvTrId, int nTrLenThres, float fTrDistThres);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////------------事件报警前对目标过滤的函数及变量------------/////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//对一些目标设置本帧不进行事件的报警
	void mvset_vehicle_nojoin_event( int nTrSetElemNum, MyTrackElem* pATrackElem[],
		    int nObjSetElemNum,MyVehicleElem* pAObjectElem[],objInfo_befResult *pAObjInfo );  
	void	mvset_alikeBgObj_nojoin_event( );

	void mvset_badProperty_vehicle_nojoin_event( int nObjSetElemNum,MyVehicleElem* pAObjectElem[],objInfo_befResult *pAObjInfo );
	bool mvJudge_TwoObj_IfCanCom(MyGroup vehicle_i, MyGroup vehicle_j);

	void mvset_beoverlap_vehicle_nojoin_event( int nObjSetElemNum,MyVehicleElem* pAObjectElem[],objInfo_befResult *pAObjInfo );
	
	void mvset_shadow_vehicle_nojoin_event( );  
	bool mvis_pixel_suit_shadow_color(int nH, int nW);
	void mvget_shadow_area( IplImage *lpMaybeShadowImg );
	bool mvis_objSuit_shadowCharacter(MyGroup shadowObj, MyGroup shadowMakeObj);

	void mvset_mismatch_vehicle_nojoin_event( int nTrSetElemNum,MyTrackElem* pATrackElem[],
		    int nObjSetElemNum,MyVehicleElem* pAObjectElem[],objInfo_befResult *pAObjInfo );  
	bool mvis_wrong_group_with_aroundTr( MyGroup vehicle, int nTrSetElemNum,MyTrackElem* pATrackElem[] );
	bool mvis_wrong_group_with_estPtAndTurnTr( MyGroup object, int nTrSetElemNum,MyTrackElem* pATrackElem[] );

	void mvset_allTimeIn_skipAreaEdge_nojoin_event( int nTrSetElemNum,MyTrackElem* pATrackElem[],
		    int nObjSetElemNum,MyVehicleElem* pAObjectElem[],objInfo_befResult *pAObjInfo );  	
	
	void mvset_temp_vehicle_nojoin_event( );
	
	void mvset_light_vehicle_nojoin_event( );
	void mvset_light_vehicle_nojoin_event2( );
	bool mv_is_track_suit_light_RGB_color( IplImage *RGBImage, MyTrack tr );
	bool mvis_vehicle_suit_light_color_require( MyGroup &vehicle );
	void mvget_light_area( IplImage *lpLightMaskImg );

	void mvset_suitBgObj_onNight_nojoin_event( );

	//将一些目标设置本帧不参入目标出现报警
	void mvset_objects_noAlertAppear( int nTrSetElemN,MyTrackElem* pATrackElem[], 
		    int nObjSetElemN,MyVehicleElem* pAObjectElem[], objInfo_befResult *pAObjInfo );
	bool mvNoAlert_smallObj_around_bigObj(int nObjNum, int nAObjId[], int nTrSetElemNum,MyTrackElem* pATrackElem[], 
		int nObjSetElemNum,MyVehicleElem* pAObjectElem[], objInfo_befResult *pAObjInfo, bool bANoAlert[]);
	bool mvis_foreMvVehicle_wantDisappear(float ori, MyGroup vehicle);

	//获取得到可用于事件检测的目标
	void mvget_allSortObject_for_eventDetect( int nObjSetElemNum,MyVehicleElem* pAObjectElem[] );
		
	bool mvis_track_closeTo_polygonEdge( MyTrack &tr,
		   int n, CvPoint2D32f *ASkipRgnPt, float fThres);	

	/////////////////////////////////////////////////////对阴影进行处理/////////////////////////////////////////////////////////////
	//获取得到候选的阴影及阴影制造者
	bool mvget_candiate_shadowAndMaker_for_size( int &nMyabeShadowNum,int *nMyabeShadowIDA, double *dMyabeShadowSizeA, 
		int &nMyabeCauseShadowNum, int *nMyabeCauseShadowIDA, double *dMyabeCauseShadowSizeA );
	bool mvJude_twoObj_suit_shadowandMaker_velo_and_dist( int v_idx, int v_com_idx );
	bool mvJudege_twoObj_suit_distConst( MyGroup vehicle_i, MyGroup vehicle_j, float thres );
	bool mvget_candiate_shadow_for_velo_and_dist( int nMyabeShadowNum, int *nMyabeShadowIDA, double *dMyabeShadowSizeA, 
		int nMyabeCauseShadowNum, int *nMyabeCauseShadowIDA, double *dMyabeCauseShadowSizeA, 
		int &nShadowNum, int *nShadowIDA );
	bool mvget_candiate_shadowmaker_for_velo_and_dist( int nMyabeShadowNum, int *nMyabeShadowIDA, double *dMyabeShadowSizeA, 
		int nMyabeCauseShadowNum, int *nMyabeCauseShadowIDA, double *dMyabeCauseShadowSizeA, 
		int &nCauseShadowNum, int *nCauseShadowIDA );
	bool mvget_candiate_shadowAndMaker( int &nShadowNum,int *nShadowIDA, int &nCauseShadowNum, int *nCauseShadowIDA );
	//判断候选阴影是否满足颜色要求（线段颜色或轨迹颜色中一个满足即可）
	bool mvIsObjTrackSuitShadowColor( MyGroup &vehicle );	
	void mvget_line_inster_with_window( MyGroup vehicle, float h_ratio, float w_ratio, int nLineNum, BLine *blLine, int &nInsterLineNum, int *nInsterLineID );
	void mvget_midline_of_two_line( BLine line1, BLine line2, BLine &midLine );
	bool mvIsObjLineSuitShadowColor( MyGroup vehicle, int nShadowLineOri );
	//判断候选阴影和阴影制造者是否满足阴影线条关系
	void mvGetMaybeShadowLine( MyGroup &vehicle, int &nMaybeShadowLineNum, int *nMaybeShadowLineIDA );
	bool mvIsObjSuitPeopleShadowLine( MyGroup &vehicle, MyGroup &vehicle_make_shadow, int lineOri, int linePos, int &lineNum, int *lineIDA );
	//设置阴影不参入判断
	IplImage 	*m_shadowImage; 
	void mvUpdata_shadow_track( double timeThres );
	void mvDelete_shadow_track_from_vehicle( int nObjNo, MyVehicleElem *p_v_elem );
	int  mvget_ratio_value_of_pixel( int nH, int nW, int nRadius, IplImage *Img );
	void mvget_coners_inRect(CvRect rect, int srcPtCnt,CvPoint2D32f *srcPtA, int &ptCnt,CvPoint2D32f *ptA);
	bool mvAre_object_noshortbg_corner_too_less(MyGroup obj, float r_scale);
	//阴影线段
	void  mvGetObjForShadowCal( int &nObjNum, int *nObjIDA );
	void  mvFilterLineForShadowCal(int inLineNum,  BLine *inLineA, int &validLineNum, int  *validLineId);
	void  mvGetLineForShadowCal( int nObjNum, int *nObjIDA, int validLineNum, int *validLineId,	
		                                              int &nLineNum, int *nLineIDA, int *nLineVehicleIDA );
	bool  mvIsLineCauesByPeopleShadow( int l_idx, int l_v_idx );
	void  mvCalShadowLineDirection( MyGroup vehicle, BLine line, int &linePos, int &lineOri );
	void  mvStatisShadowDirAndPos( int m_nShadowLineNum, int *m_nShadowLineOriSet, 
		                                                 int *m_nShadowLinePosSet, int *nOriStaA, int *nPosStaA );	
	void  mvGetShadowDirection( );

	bool mv_is_track_suit_shadow_RGB_color( IplImage *RGBImage, MyTrack tr, bool bAreaSuit[4] );
	bool mv_is_track_suit_shadow_HSV_color( IplImage *HImage, IplImage *SImage, IplImage *VImage, MyTrack tr, bool bAreaSuit[4] );
	void mv_make_sign_for_shadow_track( );
	void mvAdjustShadowTrack( );

	//////////////////////////对灯光进行处理////////////////////////////
	void mvget_light_judge_area( MyGroup vehicle, float &xlt, float &ylt, float &xrb, float &yrb );
	void mvUpdate_light_track( double timeThres );	
	void mvget_brightDarkArea(IplImage *srcImg, float &fAvgV, CvPoint lt_pt, CvPoint rb_pt, IplImage *brightMaskImg, IplImage *darkMaskImg);
	

	////////////////////////////////////////////////////////////////////
	////////----------交通参数统计和事件报警的函数及变量----------//////
	////////////////////////////////////////////////////////////////////		
	double mvget_halfTime_trTs( MyGroup obj, double dLastFrameMaxTime );		
	bool mvget_bufferBigVehicleTr_with_existTr( int nTrNum, int *nTrIDA, 
			int &nVaildTrNum, int *nVaildTrNo );
	bool mvjudge_channelIsJam( int nObjSetElemNum,
			MyVehicleElem* pAObjectElem[], bool bAChannelIsJam[] );
	bool mvjudge_channelIsSlow( int nObjSetElemNum,
			MyVehicleElem* pAObjectElem[], bool bAChannelIsJam[] );

#ifndef NOPLATE_COLOR
	bool mvGetVehicleColorInfo(MyGroup &vhicle, vhicle_color_info *colo_vhivle, bool bStop=false);
	void mvGetVehicleColorImages( ImageSet *pImageSet );
#endif

	//检测到事件后,设置该事件的ID
	bool    mvset_eventId_after_detectedEvent(  MyGroup &vehicle, SRIP_DETECT_OUT_RESULT& detect_out_result);  
	bool    mvSetEventIdAfterDetectedChannelEvent( SRIP_DETECT_OUT_RESULT& detect_out_result );
	bool    mvis_twoTrackSet_closeAllTime(int nTrNum1, int *nATrId1, int nTrNum2, int *nATrId2, float factorX, float factorY);
	bool    mvGetObjectOfBufferAtNow(int nCompN, unsigned long *nACompId, int &nObjN, int *nObjId); 
	bool    mvIsTwoObjCloseAllTime( const MyGroup &vehicle_com, 
				const MyGroup &vehicle, float factor, 
				bool bPeopleExt=true, bool bPeopleRed=false );
	
	//根据目标类型判断是否为需要报警的事件类型
	bool	mvIsNeedAlertEventWithObjType(
			const VEHICLE_PARAM_FOR_EVERY_FRAME &chanPara,
			const DETECT_RESULT_TYPE& eType, int uObjType);

	//(1)交通参数统计
	void mvget_statistic_result( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool _mvcars_arrive_line(int nIndex);	//上面函数的替代函数
	bool mvis_car_arrive(MyGroup& group1,Line line1);
	void mvsubtract_arrive_line(MyGroup &vehicle);
	void mvchannel_zyl(int nIndex, bool flag = false );//计算车道的占有率
	double mvqueue_calculation(int nIndex);//计算队列长度
	double mvctjj_judgement(int nIndex);//平均车头长度
	float mvgetbuspercent( int nIndex );

	void mvmark_vehicle_track_arrive(MyGroup &vehicle);
#ifndef UNIQUE_FLOWLINE
	bool mvcars_arrive_line(int nIndex);//no use 
#endif		
	void mvmodify_param(int nStatFluxTime);//修改统计周期
	
	void mvget_event_detect_result( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );

	//(2)事件检测报警
	//保存事件报警结果
	bool mvSaveEventAlertResult(	
			int nChannelIdx,	 //车道序号
			int nObjIdx,		 //目标序号
			int nAlertMode,		 //报警事件模式
			DetectResultList &detOutList ); //报警事件列表

	//目标在报警显示时间范围内
	bool mvObjInAlertShowTime( const MyGroup &obj,
			double dTsNow, int nAlertMode );
	
	//为防止目标报警过多:到缓冲器中寻找f秒以内的发生事件的目标,
	//      若该目标在当前帧还存在且其与欲报目标较为接近则不报.
	bool mvIsObjAlertInBuffer(const MyGroup &obj, double dTsNow,
			int nAlertMode, bool bAddToBuff, float fBufferTimeTh,
			float fDisTh);

	//判断是否为误报的目标类型(如报警车辆逆行，却误报了行人)
	bool mvIsWrongWantAlertObjType(const MyGroup &obj, 
			int nObjIdx, int nChannelIdx, int nAlertMode);

	//目标逆行判断报警
	bool mvAreObjAgainst(int nIndex, vector<int> &vectObjIdx);

	//车辆逆行判断
	bool mvIsAgainstObject(MyGroup &obj, float fCentroidOriKind,
			int nSensitivity=0, bool bJustDetVehAgainst=false);

	//获取和当前方向接近的长轨迹信息
	bool mvGetLongTrsInfoAlongNowDir( int nAgainstTrCnt, int *nAAgainstTrIdx,
			float fDistThres, int &nLongAgainstTrCnt, int *nALongAgainstTrIdx,
			float *fALongAgainstTrDist, CvPoint2D32f *fSrartPt, int nAgainstCross=-1 );

	//在世界坐标系中距离是否满足要求
	bool mvTrIsLongInWorldCoordinate(
		int nLongTrCnt,				//长轨迹的个数
		int *nALongTrIdx,			//长轨迹索引
		CvPoint2D32f *fSrartPt,		//与长轨迹相同方向的起始点
		float fDistThInWorld		//世界坐标中的距离阈值
		);

	//找出在分析区域内的存在方向和逆行的轨迹
	bool mvGetImgOriAndAgainstTrIdx( CvPoint ptExpLt, CvPoint ptExpRb,			
			vector<int> &vectImgOriTrIdx, vector<int> *pVectAgainstTrIdx=NULL, 
			vector<int> *pVectNoAgainstTrIdx=NULL );

	//获取逆行轨迹往下扫瞄得到"在相反方向的道路中"的y坐标
	bool mvGetYCoordInInvRoadOfAgainstTr(
			IplImage *oriImage,			//道路的图像角度图
			int nAgainstTrCnt, int *nAAgainstTrIdx, //逆行轨迹数及其序号
			int *nADownYInInvOriRoad,	//往下的Y在相反方向的道路中
			int *nADownDistInInvOriRoad //往下多少在相反方向的道路中
		);

	//判断目标的底部所在点与逆行轨迹点所对应的道路方向是否相反	
	bool mvIsInvBetweenObjBottomAndAgainstTr(
			MyGroup &obj,				//目标
			int nMinBotWidthTh,         //目标底部的最小宽度要求
			int nMinObjHightTh,         //目标高度要求
			int nAgainstTrCnt,			//逆行轨迹数目
			float *fAAgainstTrRoadKind, //逆行轨迹当前点的道路方向角度
			IplImage *pRoadOriImg		//道路的图像角度图
		);

	//将点往下走，看其需要往下落多远进入相反道路区域(负值无效)
	int mvGetDownDistInInvRoadOfGivePt(
			IplImage *oriImage,	CvPoint ptNow, int nCarH );

	//消除因相反道路区域所引起来的逆行误报
	bool mvIsWrongAgainstCauseByInvRoad( MyGroup &obj,
			int nAgainstTrCnt, int *nAAgainstTrIdx, //逆行轨迹数及其序号
			float *fAAgainstTrRoadKind  //逆行轨迹的道路方向角度
		);

	//利用已有的轨迹差异计算结果 来 判断两轨迹群是否相似
	bool mvIsSimilarTwoTrackGroup( 
			int nTrCnt1, int *nATrIdx1,  //轨迹群1的数目及序号
			int nTrCnt2, int *nATrIdx2,  //轨迹群2的数目及序号
			bool bUsePtLocDist=true     //使用点的位置距离做为约束
		);

	//获取得到目标的颜色
	void mvGetObjectColor( MyGroup &obj, SRIP_DETECT_OUT_RESULT &detOutRes );

	//给应用端所报的事件目标类型
	bool mvGetAlertResultType(int nAlertMode, SRIP_DETECT_OUT_RESULT &detOutRes);

	//强制输出检测出事件的目标类型
	int mvForceDetectedEventObjType( int objType );


	//get alert result and push to server 
	void mvGetAlertResult2Server( int nAlertCnt,
			int nAObjIdx[], bool bAShow[],	
			DETECT_RESULT_TYPE nAlertTypeMod,
			DetectResultList& detOutList,
			SRIP_DETECT_OUT_RESULT& detOutResult );

	//detect the car if drive on the lane lines( the interface )
	void mvAreCarsPressOnLine( 
			DETECT_RESULT_TYPE nAlertTypeMod,
			DetectResultList& detOutList, 
			SRIP_DETECT_OUT_RESULT& detOutResult );


	//(2.1)目标出现报警
	void mvAreObjectAppearNew( int i, DetectResultList& detOutList, 
							   SRIP_DETECT_OUT_RESULT& detOutRes);

	void mvAlertPeopleAppearInBank( int i, DetectResultList &detOutList,
							   SRIP_DETECT_OUT_RESULT &detOutRes );
	bool mvAlertObjectAppear( int nIndex, 
			 vector<MvObjEventAlert> &vectObjEventAlert );


	//判断目标是否满足目标出现的条件
	bool mvIsObjSuitAppearCondition( int nObjIdx, MyGroup &obj );

	//从多帧目标信息中获取得到目标所对应的序号
	bool mvGetObjIdxInMultiFrameObjInfo(vector<int> &vectSaveFrameIdx, 
				    vector<int> &vectObjIdx, int nObjIdx, long lObjId);

	//判断目标是否满足目标出现的目标轨迹要求
	bool mvIsSuitAppearRequire4ObjTracks( MyGroup &obj,
			 vector<int> &vectFrameIdx, vector<int> &vectObjIdx );

	//判断目标是否满足目标出现的目标区域大小和轨迹数目要求
	bool mvIsSuitAppearRequire4ObjAreaTrack( MyGroup &obj,
			vector<int> &vectFrameIdx, vector<int> &vectObjIdx );

	//判断目标是否满足目标出现的目标区域积分要求
	bool mvIsSuitAppearRequire4ObjAreaInte( MyGroup &obj,
			vector<int> &vectFrameIdx, vector<int> &vectObjIdx );

	//判断目标是否和车辆较靠近且运动较一致
	bool mvIsCloseAndSmilarWithVehicle( MyGroup &obj, 
			vector<int> &vectFrameIdx, vector<int> &vectObjIdx );

	//根据目标类型判断结果来过滤目标出现结果（只针对银行这样改）
	bool mvIsSuitAppearRequire4ObjTypeJudge( MyGroup &obj, int nObjIdx,
			vector<int> &vectSaveFrameIdx, vector<int> &vectSaveObjIdx );

	//判断目标是否在目标出现buffer中出现过了
	bool mvIsObjectAlertInAppearBuffer( MyGroup &obj, float fBuffComTime );

	bool mvare_objectAppear( int nIndex );
	bool mvIsSuitAppearInNomobleRoad( int nObjIdx, MyGroup& vehicle );
	bool mvIsWrongObjAppOnNight( MyGroup &vehicle, bool bSuitDiff );

#ifdef ALERTOBJ_ON_GIVELOCATION
	bool m_bNeedSave;
	FILE *m_appAlertFp;
	IplImage *m_pAppAlertImg;
	char m_cAppAlertDir[104];
#endif
#ifdef DEBUG_OBJECT_TYPE
	long m_nDetectedPeoCnt;
	char m_cObjTypeDir[104];
#endif
	//获取单个目标的HOG判断时行人区域
	bool mvGetBestPeopleArea( IplImage *pIntFkImg, 
				CvPoint peoCetPt, CvPoint peoWhPt,
		        CvPoint &bestLtPt, CvPoint &bestRbPt  );
	//获取得到目标所对应的行人检测区域的中心
	bool mvGetCetPtOfPeoDetArea( CvPoint2D32f objCetPt, int nChannelNo, 
		      int &nInPerNo, CvRect &peoAreaRect, CvPoint &areaCetPt );

	//获取得到目标所车道的第一条流量检测线
	bool mvGetAmountLineOfChannel( int nChannelNo, CvPoint &areaCetPt, 
		      CvPoint &linePt1, CvPoint &linePt2 );

	//得到人的宽高
	bool mvGetPeoMaxsizeXy(  CvPoint cetPt, CvPoint &cetPtWh );
	//判断是否为报目标出现的最好的位置
	bool mvIsBestLocForAlertApp( MyGroup obj, int nInPerNo, CvRect peoDetectAreaRect,
		     CvPoint bestCetPt, CvPoint linePt1, CvPoint linePt2, int &nAlertMod );
	bool mvHaveTrueAppearObject( int objNum, int *objArray );


	bool mvAppObj_IsTBPart_Of_Vehicle (MyGroup vehilce, MyTrackElem* pATrackElem[] );
	bool mvAppObj_IsTBPart_Of_Vehicle(MyGroup &vehilce );

	
	bool _mvJudgeFalseAlarm(const CvRect rtROI);	 
	bool mvis_exist_object(MyGroup &object);

	bool mvGetValidVehicleAroundObject( int nObjIdx, MyGroup &vehicle, 
			    int &nValidNum, int *nValidA, int &nAppearObjNum, 
				int *nAppearObjA, float x_ratio, float y_ratio );
	void mvmarkAppearObjectTrack( MyGroup &Vehicle );
	bool mvJudegIsNewObjectByTrack( MyGroup &Vehicle );
	bool mvIsNewObjectInNomobleRoad( int v_idx, MyGroup& vehicle, int objNum, int *objArray );	

	//(2.2)车辆禁行报警
	void mvAreForbiddenDrive( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	int  mvget_mainChannelNo_ofHLine(int lx, int rx, int y);
	bool _mvCheckReported(const int nChannel,const CvRect rtRect,const bool bUp=false);		

	bool mvis_needDetectForbid( );
	void mvget_appearObjeOnChan_forForb( int *nAValid, int *nAAppearObj, int &nNewAppearObjNum, int *nANewAppearObj );
	void mvare_objectAppear_onChannel( int nTrSetElemNum,MyTrackElem* pATrackElem[],
		int nObjSetElemNum,MyVehicleElem* pAObjectElem[],objInfo_befResult *pAObjInfo );


	void mvget_anglesOfOnePoint( CvPoint ptNow, int ori_kind, float homography[3][3], float homography_inv[3][3],									 
		float &against_angle1, float &against_angle2, float &move_angle1, float &move_angle2, 
		float &angle_cross1_s , float &angle_cross1_e, float &angle_cross2_s , float &angle_cross2_e );
	void mvget_rectTracks_direction_new( int kind, CvPoint cetPt, bool flagAgainst, 
		int nTrNum, int *nTrIDA, int &nVaildTrNum, int *nVaildTrIDA );
	bool mvreal_vehicle_direction_new( int kind, MyGroup& vehicle, bool flagAgainst, int &nTrNum, int *nTrIDA );
	//(2.3)横穿报警
	void mvAreObjectsCrossMove( int nChannelIdx, DetectResultList& detOutList,
								SRIP_DETECT_OUT_RESULT& detOutRes );
	void mvAre_cross_new( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
		
	bool  mvAreObjCross( int nChannelIdx, vector<int> &vectObjIdx );
	bool  mvare_cross_new( int nIndex );

	bool  mvIsCrossObject( MyGroup &obj, int nSensitivity=0 );
	bool  mvIsTrackCrossInWordCoord( int nTrNum, int *nTrIDA, int nType=1 );

	//(2.4)逆行报警
	void mvAreObjectsAgainstMove( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool mvcars_go_against_new(int nIndex);
	bool mvGetTrackInAround( float scaleX, float scaleY, int nTrNum, int *nTrIDA, int &nAroundTrNum, int *nAroundTrIDA );
	bool mvget_track_with_dirAndVelo( int nTrNum, int *nTrIDA, int nAroundTrNum, int *nAroundTrIDA, int &ndvTrNum, int *ndvTrIDA );
	bool mvGetCandidateExTrack( float scaleX, float scaleY, int nTrNum, int *nTrIDA, int &nCanTrNum, int *nCanTrIDA );
	bool mvIsTrackAgainstInWordCoord( int nTrNum, int *nTrIDA );
	int  mvget_mainRoadId_ofTrack( int nTrNum, int *nTrIDA );
	int  mvget_mainRoadId_ofHLine( int lx, int rx, int y );
	void mvget_oriAndCanTracks_mainRoadKind( int nTrNum, int *nTrIDA, int nCanTrNum, int *nCanTrIDA,
		   int &oriTrRTrNum, int &oriTrRKind, int &canTrInOriTrRTrNum, int &canTrRTrNum, int &canTrRKind );
	bool mvis_trueAgainst_withTrack( int v_idx, int nTrNum, int *nTrIDA );   //主要是高度影响
	bool mvis_trueAgainst_withVehicle( int v_idx, int nTrNum, int *nTrIDA, bool bDetectPeople=false ); //主要是宽度影响
	bool mvis_trueAgainst_with_sameDirTr( int v_idx, int nTrNum, int *nTrIDA, IplImage *lpDrawImg=NULL );
	bool  mvget_bigVArea_bottomTrack_aroundObj( MyGroup obj, int &nTrNum, int *nATrNo);		
	bool mvdetect_shadow(MyGroup &vehicle, bool against = false);


	//过滤掉错误的逆行目标
	int mvFiltrate_wrongAgainstObj( MyGroup obj, bool bJustAlertVehicle, 
			char cRefuseInfo[256], int &nInfoCnt, char cInfo[20][104] );
	int mvFiltrateWrongAgainstObjOnNight( MyGroup &obj,
			CvPoint ltPt, CvPoint rbPt, bool bJustAlertVehicle );

#ifndef LINUX
	bool mvFilterObject( );
#endif
	//存下逆行误报过滤的判断挑选信息
	void mvSaveAgainstDebugResult( int nSaveMod, int nWrongMod, MyGroup vehicle, int nInfoCnt, char cInfo[20][104] );

	//清除区域内的阴影
	bool mvGetNoShadowPixel( CvRect roiRect, IplImage* pNoCandShadowImg );

	//判断是否为阴影
	bool mvJudgeIsShadow( CvRect roiRect, IplImage* pCandShadowImg );

	bool mvget_vaildBufferBigVehicleNo( int &nVaildBigVNum,  int *nAVaildBigVNo );
	bool mvis_trueAgainst_with_bigVehicle( int v_idx, int nTrNum, int *nTrIDA );	

	//(2.5)变道报警
	void mvAreObjectsChangeChannel(int nIndex,
		      DetectResultList &detOutList, 
		      SRIP_DETECT_OUT_RESULT &detOutRes);	
	bool mvare_cars_change(int nIndex);

	//检测变道的目标
	bool mvDetChangeChannelsObjs( int nIndex, vector<int> &vecDetObjIdx );
	bool mvIsObjectTrackGoChange(MyGroup& group, int &changleTrNum, 
		                      int *changleTrIdA, Line &line_change);
	//判断轨迹方向是否发生改变
	bool mvJugeTrDirIsChange(MyTrackElem  *pTrElem);
	bool mvIsChangeTrackSuitDistRequire( const MyGroup &obj, 
	       int changleTrNum, int *changleTrIdA, Line &line_change );


	//判断是否为允许的变道（即方向与变道方向相反）
	bool mvIsSuitDirectionChange( int nChangleTrNum, 
			int *nAChangleTrId, Line &line_change );

	//判断变道轨迹是否满足距离要求
	bool mvIsChangeTrackSuitDistRequir(	int nChangleTrNum, 
			int *nAChangleTrId, Line &line_change );

	bool mvIsItTrueChangeWithHoriLine(MyGroup &vehicle);
	bool mvIsItTrueChangeWithCentroid(MyGroup &vehicle);
	bool mvIsItTrueChangeWithBottomPt(MyGroup &vehicle);
	bool mvObjIsLisgt(MyGroup &obj);

	//(2.5+)压线报警
	bool mvFiltrateBgLines( int &nLineCnt, CvPoint2D32f *pALinePt1, CvPoint2D32f *pALinePt2, IplImage *pRSFkImg );
	bool mvGetCandidateVehicleBottomLine( const MyGroup &vehicle, float fR2URatio, 
		   int &nLineCnt, CvPoint2D32f pALineRSPt1[], CvPoint2D32f pALineRSPt2[], 
		   int &nCanLineCnt, CvPoint2D32f pACanLineRSPt1[], CvPoint2D32f pACanLineRSPt2[],
		   IplImage *pRSShdowImg, IplImage *pRSGrayImg, IplImage *pRSBgImg,
		   IplImage *pRSFkImg, IplImage*pDrawImg=NULL );
	void mvProjectShadowToYAxis( IplImage *pRSShdowImg, int *nAProjectY );
	vector<int> mvGetShadowLocalPeak( const MyGroup &vehicle, CvSize sz, 
		   int *nAProjectY, float fR2URatio, IplImage *pDrawImg=NULL );
	bool mvGetCloseToPeakLines( vector<int> vctPeakY, 
		   const MyGroup &vehicle, float fR2URatio, int nLineCnt,
		   CvPoint2D32f pALineRSPt1[], CvPoint2D32f pALineRSPt2[],
		   vector<candBottomLine> &vctCandBottomLine,
		   IplImage *pDrawImg=NULL );

	//get the candidate bottom lines of vehicle
	vector<candBottomLine> mvGetCandidateVehicleBottomLines( 
		const MyGroup &vehicle, float fR2URatio, int nLineCnt, 
		CvPoint2D32f *pALineSRPt1, CvPoint2D32f *pALineSRPt2,
		int nShadowMinYInRsImg, IplImage *pRoiScaleGrayImg,
		IplImage *pRoiScaleBgImg, IplImage *pRoiScaleFkImg );

	//detect the car if drive on the lane lines( the algorithm)
	bool mvGetPressLineCars(
				DETECT_RESULT_TYPE nAlertTypeMod,
				vector<int> vecPtCnt, 
			    vector<CvPoint *> vecPtPointer,
				vector<int> &vecAlertObjIdx );

	//车辆的底部的线段是否和禁止压的线相交
	bool mvIsVehicleBottomLineIntersectWithForbLine( 
			const MyGroup &vehicle,
			CvPoint cetPt, CvPoint carWhPt,
			int nForbidLineCnt, CvPoint **pPtAddress );

	//detect the car if drive on give lines
	bool mvExtractLineOfGiveArea( 
				CvPoint ptUseLt, CvPoint ptUseRb,
				vector<CvPoint2D32f> &vctUseLinePt1, 
				vector<CvPoint2D32f> &vctUseLinePt2 );


	//get vehicle bottom lines of give area
	vector<candBottomLine> mvGetVehicleBottomLineOfGiveArea( 
				const MyGroup &vehicle,
				IplImage *pRoiScaleGrayImg, 
				CvPoint ptLt, CvPoint ptRb,
				CvPoint rsLtPt, CvPoint rsRbPt,
				float fR2URatio, int nLineCnt,
				CvPoint2D32f pALineSRPt1[],
				CvPoint2D32f pALineSRPt2[] );


	float mvGetScaleResizeArea( bool bBigVehicle, CvPoint ptLt,CvPoint ptRb, CvPoint &oriLtPt,CvPoint &oriRbPt, CvPoint &rsLtPt,CvPoint &rsRbPt);
	float mvExtractLinesInObjArea( float fRes2UseRatio, CvPoint carWhPt, CvPoint oriLtPt, CvPoint oriRbPt, CvPoint rsLtPt, CvPoint rsRbPt,
		                           IplImage **ppGrayImg, int &nLineCnt, CvPoint2D32f pASRLinePt1[], CvPoint2D32f pASRLinePt2[] );

	void mvLineExtract( IplImage *srcImg, CvSize dstSize, int &nLineCnt, CvPoint2D32f pALinePt1[], 
		                CvPoint2D32f pALinePt2[], IplImage *pGrayImg, IplImage *pHSobImg=NULL );

	
	bool mvIsIntersect( bool bStrictMod, MyGroup vehicle, float fD2SRatio, CvPoint2D32f forbidLinePt1, CvPoint2D32f forbidLinePt2,
		   int &nLineCnt, CvPoint2D32f pALinePt1[], CvPoint2D32f pALinePt2[], CvPoint2D32f pALineRSPt1[], CvPoint2D32f pALineRSPt2[], 
		   IplImage*pRSGrayImg, IplImage*pRSBgImg, IplImage*pRSFkImg, IplImage*pDrawImg );
	bool mvIsLineCloseToShadowOrBackground( CvPoint2D32f linePt1, CvPoint2D32f linePt2, int nFindLen,
		   IplImage*pRSGrayImg, IplImage*pRSBgImg, IplImage*pRSFkImg, int nALineMod[3] );
	//get the candidate road surface result
	void mvGetCandBgImg( IplImage*pRSGrayImg, IplImage*pRSBgImg, IplImage*pRoadSurfaceImg );

	//(2.6)单独速度监测
	void mvIs_ObjSpeed_tooFOS( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool mvis_car_fast(int nIndex, bool bJustDetectCar=true);
	bool mvIsObjectFast(MyGroup &vehicle,
			double thres_fast, int &nTrNum, int *nTrIdA); 
	bool mvIsItTrueCarFast(int v_idx , double thres_fast, int nTrNum, int *nTrIdA); 
	bool mvis_car_slow(int nIndex);
	bool mvIsObjectSlow(MyGroup &vehicle, 
			double thres_slow, int &nTrNum, int *nTrIdA); 
	bool mvIsItTrueCarSlow(int v_idx, double thres_slow);
	bool mvIsGetFastTr(CvRect rectVicleBottom, double thres_fast);
	bool  mvis_slowObjBelongToSlowChannel( MyGroup &slowObj );
	//(2.7)停车检测
	bool m_bUseObjStop;  //使用目标检测来进行停车检测
	vector<StruObjStopAreaInfo> m_vObjStopAreaInfo;
	bool mvGetUnhinderedStopCars(  //获取得到顺畅停车目标信息
			vector<StruObjStopAreaInfo> &vectObjStopAreInfo ); 

	//判断目标当前是否为停下来
	bool mvIsObjectStopNow( int nObjNo, MyGroup &obj, float fVeloTh, float fDistTh );

	bool mvIsObjExistMove2StopStatus( MyGroup &obj, float thres_velo, float thres_dist );

	bool mvget_minAndMaxDist_OfTrack( int nTrNum, short *nATrNo, int &minDist, int &maxDist,
		    int &nSumMinTrNum, int &nSumMaxTrNum ); //计算得到轨迹中指定长度的最长和最短的位移
	

	//(2.8)道路堵塞报警
	void mvIs_road_jam( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	int  mvcar_dusai(int nIndex);			
	void mvcalc_jam_queue_stop(int nIndex, double &length, double &velo);
	void mvcalc_jam_queue(int nIndex, double &length, double &velo);
	//(2.9)其他事件检测
	void mvOtherEventAlarm( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
#if 0	
	bool mvAreDerelicts(int nIndex);
#endif
	bool mvAreDerelicts_old(int nIndex);
	bool mvJudgeDesert(const CvRect rtRect);
	bool mvJudgeTrackBias(const CvPoint ptCenter,double ts=-1) const;
	void  mvPush_dropObj_ToList( DropObjStruct dropObj, int &num,  DropObjStruct *dropObjList, double tsMaxExist );
	int     m_dropObjNum;
	DropObjStruct  *m_dropObjList;
	IplImage **m_ppDropRegion;

	//(2.10)平均车速报警
	void mvAre_AvgObjSpeed_tooFOS(int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool mvavg_speed_fast(int nIndex); //判断平均车速是否快速
	bool mvavg_speed_slow(int nIndex); //判断平均车速是否慢速
	

	//得到在停止点矩形区域内的所有点
	void mvGetInStopRectPt( int nPtCnt, CvPoint2D32f ptA[], vector<CvPoint> vctSuitStopPt, vector<CvPoint> &vctInStopRectPt );
	//得到在扩展区域内的点
	void mvGetInExtendRectPt( vector<CvPoint> vctPt, CvPoint ltPt, CvPoint rbPt, vector<CvPoint> &vctInAreaPt );


	//(3)行为分析报警
#ifdef USE_BEGAVIOR_ANALYSE	
	//(3.1)行人奔跑检测
	void mvIs_person_run(int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool mvis_people_run(int nIndex);
	void mvget_vaildFastTracks_ofArea( CvPoint areaLtPt, CvPoint areaRbPt, float fFastThres, float fTrDistThres,
		    int &nVaildTrNum, int nAVaildTrNo[], int &nFastTrNum, int nAFastTrNo[], IplImage *pIntegrateDiff );
	void mvget_vaildFastTracks_ofObject( MyGroup object, float fFastThres, float fTrDistThres,
		int &nVaildTrNum, int nAVaildTrNo[], int &nFastTrNum, int nAFastTrNo[], IplImage *pIntegrateDiff );
	void mvget_peopleArea_ofTracks( int nTrNum, int nATrNo[], CvPoint &areaLtPt, CvPoint &areaRbPt );
	bool mvis_trMoveFast_withDiffImg( CvPoint pt, IplImage *pIntegrateDiff );
	//(3.2)人群聚集检测
	void mvIs_person_crowd(int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool mvis_peopleCrowd(int nIndex); 
	void mvstat_pixels_ofEveryChannel( IplImage *pWaitStatImg, int nPixVMaxThres, int nChannelNum, int nAStartSum[ ] );
	//(3.3)闯入检测
	void mvare_bargeIn( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool mvare_bargeIn(int nIndex);
	bool mvHaveTrueBargeIn( int objNum, int *objArray );
	//(3.4)越界检测
	void mvare_beyondMark( int i, DetectResultList& , SRIP_DETECT_OUT_RESULT& );
	bool mvare_beyondMark(int nIndex);
	bool mvis_object_track_overLine(MyGroup& group, int &overTrNum, int *overTrIdA, Line &line_mark);
#endif
		

	/////////////////////////////////////////////////////////////////////////
	////////----调试过程中图像绘制及文本输出等信息的函数及变量-------////////
	/////////////////////////////////////////////////////////////////////////	

	//调试机制
#ifdef MVDEBUGON
	//存下目标类型判断的调试信息
	void mvSaveDebugInfoBefObjTypeJudge( );
	void mvSaveDebugInfoOfObjTypeJudge( );

	//存下事件检测前的调试信息
	void mvSaveDebugInfoBefEventAlert( );
	
	//存下停车检测中的调试信息
	void mvSaveDebugInfoOfVehicleStopAlert( );
	//存下变道检测中的调试信息
	void mvSaveDebugInfoOfChangeAlert( );

	//存下事件检测中的调试信息
	void mvSaveDebugInfoOfEventAlert( );

#endif

 #ifdef USE_SIM_HOG 	
	//对给定一些目标进行HoG检测,并保存结果到目标中
	void mvSimHoGDet4GiveObjects( 
			object_typeJudge_info *pAObjsTypeInfo,
			vector<int> &vctWaitObjNo );

	//对给定的一个区域,判断是否存在行人,并返回其个数和位置
	bool mvGetDetPeoWithSimHoG(	
		  CvPoint ptLtDst, CvPoint ptRbDst, //在实验图上的待检测区域
		  CvPoint ptPeoWHDst,     //给定的点在实验图上的行人标定宽高
		  vector<float> &vctPeoScore,	    //检测到的最好行人得分
		  vector<CvRect> &vctRectPeoDst );  //检测到的最好行人rect		

	void mvScanImageForHoGDetect( ); //扫瞄图像进行HoG检测
	void mvTestScanImageForHoGDetect( //测试图像进行HoG检测
			IplImage *pErodeImg, IplImage *pTempImg ); 
#endif
	

	void mvSaveImage(IplImage *pImage,int nNum);
	void mvSetROIAndSaveImg(IplImage *pImage, CvPoint LtPt,int nLength, 
							int nOffSetX, int nOffsetY,int nPeoNum);
	

#ifdef MVDEBUGON
	void mvDrawSomeLineOnResultPic(IplImage *pImage); 	//画车道、变道线、流量线	
	void mvsave_vehicle_track_to_file(char *file_name);
#endif
#ifdef	DEBUG_TRACK_NOJOIN_VEHICLE
	void mvshow_track_nojoin_vehicle( );
#endif
#ifdef DEBUG_ON
	void mvget_drawAndSave_detect_result( );
	void mvDrawOverLineResult(MyGroup& group);
	void mvDrawStaticsResult();		//画出交通统计结果
	bool mvdraw_track(IplImage *result_track);
	bool mvdrawVehicleResult(IplImage *result_vehicle, bool draw_less=true);
	void mvDrawVehicleResultStep( char *winNamStr, bool bShow, 
			char *saveNameStr, bool bSave );
	void mvDrawDebugWindow( bool *bShowA, int iNum, bool bSave, 
			char *winNamStr=NULL );
#endif
	void mvgetResultImage(IplImage *result3, IplImage *rgbImage, 
		   SRIP_DETECT_OUT_RESULT detect_out_result, int nIndex);
	bool mvdraw_maxsize_maxsize45(IplImage* img);

#ifdef DEBUG_SAVE
	void mvsave_vehicle_track_to_file(char *file_name);
#endif


#ifdef MVDEBUGON
	void mvsave_info_for_cars_stop_judge(MyGroup vehicle);
	void mvjudge_obj_type_show1( );
	void mvjudge_obj_type_show2( );

#ifdef DEBUG_TIME
	double m_dSaveEventAppearImgTime;
#endif
	//保存事件结果图像 
	void   mvSaveEventResultImg(  IplImage *testImage, float fPreTime, int nEventObjNo, MyGroup obj );
	//保存检测到事件时的目标信息 
	void   mvSaveObjInfoWhenDetectEvent( char cFileName[], float fPreTime, int nEventObjNo, MyGroup obj );
	//保存调试信息文本 
	void   mvSaveDebugInfo( FILE *fp_debug, char cType[5] );
	void   mvSaveEventAppearInfo( int nObj, MyGroup &Vehicle,
				char cType[5], float fPreTime=4.0f );
	bool   mvSaveAlertEventInfo( int nAlertMode, int nObjIdx,
				MyGroup &obj, SRIP_DETECT_OUT_RESULT &detOutRes );
	bool   mvSavePreDebugInfo( FILE *fp_debug, int nPreFrameNum, int nMod );
	bool   mvSaveGivenNoDebugInfo( FILE *fp_debug, int nStartNo, int nEndNo, int nMod );
	void   mvSaveTrackNowPt( );
#endif
#ifdef WIN32	
	#ifdef SHOW_MAINSTEP_RESULT
		public:
			bool mvdisplay_result();
		private:
	#endif
#endif

#ifdef DEBUG_MERGER_BIG_VEHICLE
	void  mvdebug_isExist_big_vehicle(int nExistNum, mayBigSplVeh_info *AMayBigSplVeh);
	void mv_bigVehicle_merger_show0( );
	void mv_bigVehicle_merger_show1( );
#endif	

#ifdef MVDEBUGON
	void mvDebugDraw1(IplImage  *lpDrawImg, SubAreaStr subArea);
	void  mvBigVehicleDetect_debug1(SubAreaStr *lpSubArea, IplImage *lpDrawImg, IplImage *lpDrawImg2,
		IplImage *lpDrawImg3, IplImage *lpDrawImg4);
	void  mvBigVehicleDetect_debug2(MegerLine megerL[3], IplImage *lpDrawImg2, bool bSuitLenRequire, bool bExistLineOnEndPt);
	void  mvBigVehicleDetect_debug_showAndSave(bool bDetectBigV, IplImage *lpDrawImg, IplImage *lpDrawImg2, 
		IplImage *lpDrawImg3, IplImage *lpDrawImg4);
#endif
#ifdef	DEBUG_CLIENT_SHOW
	bool   mvGetClientShowMod( int &nShow, int &nMod );
	void   mvShowAllObjOnClient( DetectResultList& objOutList, SRIP_DETECT_OUT_RESULT& objOutResult );
	void   mvClientShow( DetectResultList& OutList, SRIP_DETECT_OUT_RESULT& OutResult );
#endif
#ifdef DEBUG_BGMODEL_FOREIMG  //背景模型和前景图
	void mvSaveAndShow_bgAndFore( );
#endif
#ifdef DEBUG_NEWLSD
	void mvTestNewLSD();
#endif

#ifdef MVDEBUGON
	void mvdebug_useTimeOfGetElemFromSet(  );
#endif
	void mvopen_timeTestFile( );

#ifdef DEBUG_FANTATRACK
	void mvshow_fantasticTrack( int nMod, bool bSave=false );
	void mvshow_fantasticArea( int nYBlock, int nXBlock, int nATrNum[] );
	void mvshow_roadOri( bool bSave=false );
	void mvshow_roadMSOri_ofOriHist( bool bSave=false );
#endif
#ifdef	DEBUG_EVENT_CROSS
	void mvCreateCrossDebugImg( IplImage *pDebugImg );
	void mvshow_crossDebugImg( IplImage *pDebugImg, bool bSave );
#endif
#ifdef DEBUG_EVENT_ALERT
	void mvDraw_EventDetectInfo( char *cWinName, bool bSave );
#endif
#ifdef MVDEBUGON
	void mvDraw_maxsizeXy( int mod, char *cWinName );
	void mvshow_trackDisappearInfo( int nMod, bool bSave );
#endif
#ifndef LINUX	
	bool   bShowWindow[40];
#endif
#ifdef MVDEBUGON
#ifndef LINUX
			public:		
#endif					
				void	mvprint_vehicle_info();
			private:
#ifdef DEBUG_PRINT_VEHICLE
				FILE    *fp_vehicle_debug;
				void    mvprint_vehicle_info_for_debug( int step );
#endif
#endif
#ifdef DEBUG_TYPE_TXT
		FILE *fp_type_txt;
		void mvopen_debugFile(  );
#endif


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////------------老杨所写代码------------//////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
private:
	//track ori map
	void _mvUpdateTrackOriMap();
	int _mvExistLongHLine(const CvRect rtRect,const int nWidth,const float fThresh);
	int _mvExistLongVLine(const CvRect rtRect,const int nHeight,const float fThresh);
	int _mvExistLongOLine(const CvRect rtRect,const int nHeight,const int nWidth,const float fThresh);
	bool m_bExistLongV;	//当前帧是否存在大车
	bool _mvExistLongVehicle();

	//activity analysis
	bool m_bNotice;
	vector< vector<int> > m_vecJDFB;
	CvPoint m_ptCrowdCenter;
	int m_nDetectFrames;
	vector<int> m_vctRunFast;		//存储快速奔跑的行人
	vector<int>m_vctBargeIn;		//闯入
	vector<int>m_vecBeyond;			//越界
	bool mvStatTrack();
	bool mvFlowEventDetect(vector< vector<int> > vecJDFB);			//判断是否有群体事件发生
	bool mvPeopleRunFast(vector< vector<int> > vecJDFB);			//
	//bool mvPeopleRunAgainst(vector< vector<int> > vecJDFB);		//
	bool mvPeopleBargeIn(vector< vector<int> > vecJDFB);
	bool mvPeopleBeyondMark(vector< vector<int> > vecJDFB);		//判断是否存在轨迹越界
	void mvStatTrackDirection(vector< vector<int> >&vecJDFB,int nThreshold,float fDistThresh=.867f);		//根据轨迹的运动方向进行分类

	//行为事件检测
	void mvCrowdAppear(int i, DetectResultList& detect_out_list, SRIP_DETECT_OUT_RESULT& detect_out_result);			//人群聚集
	void mvRunFast(int i, DetectResultList& detect_out_list, SRIP_DETECT_OUT_RESULT& detect_out_result);
	
	int m_nStatAgainstTime;
	int m_nAAgainstTrCnt[20][20];
	void mvAgainst(int i, DetectResultList& detect_out_list, SRIP_DETECT_OUT_RESULT& detect_out_result);
	void mvBargeIn(int i, DetectResultList& detect_out_list, SRIP_DETECT_OUT_RESULT& detect_out_result);		//闯入警戒区
	void mvBeyondMark(int i, DetectResultList& detect_out_list, SRIP_DETECT_OUT_RESULT& detect_out_result);		//	越界
	void mvHover(int i, DetectResultList& detect_out_list, SRIP_DETECT_OUT_RESULT& detect_out_result);					//徘徊
	void mvCrowdDisappear();		//人群分散

	void mvObjectAppear(int nChannelIndex, DetectResultList& detect_out_list, SRIP_DETECT_OUT_RESULT& detect_out_result);		//目标出现

	//tools
	IplImage * mvConvertImage(MvImage<uchar>*pImage);

	//activity event
	CvRect mvFindRectsCenter(vector<CvRect> rtRects);
	bool mvisPeopleCrowd(int nIndex);

	//方案I
	float mvTrackDistanceI(int nTr1Index,int nTr2Index);
	float mvTrackDistanceI_1(const int nTr1Index, const int nTr2Index);
	void mvSegmentImageI(vector<  vector<int> > vecJDFB,float fThresh);

	bool mvIsBeyondMark(const MyTrack tr);
	bool mvIsObjBeyondMark(const int nVIdx);

	void mvAjustEventObject();
	void mvPridictVehilce();		//
	map<int,MvObject>  m_mapObjectRect;	//目标估计区域

	map<int,MvEventObject> m_mapEventObject;			//事件目标区域
	
	bool m_bCarnum2Object;

	void mvRecordObj();						//记录目标运动等相关信息
	void mvCarnum2Vehicle();				//事件车牌关联
	map<string,MvCarnumTrack> m_mapCarnumTrack; 
	map<int, MvEventObject> m_mapEvent2Carnum;		//需要关联的目标及事件
	map<int, MvObjInfo> m_mapObjInfo;							//记录目标运动、车牌等相关信息

	void mvGetCarnumEventResult(int nObjID, SRIP_DETECT_OUT_RESULT& detect_out_result);

	void mvReportEvent(DetectResultList& detect_out_list);	//报出进入缓冲的事件
	void mvClearMap();				//清理缓冲

	int mvChooseMatchBest(vector<int> vecCandidate,MvCarnumTrack carnumTrack) ;


	int m_nMinRegSize;			//自动求出最小的minsize

	float mvCosDist(const MyTrack &tr,const MvObject &obj) const;
	bool mvIsSame(const MyGroup &group, const MvObject &obj) const;
#ifdef USE_OPTICALFLOW
	vector< MvCoTrack> m_vecCoTrack;		//辅助轨迹
	int m_nOpticalFlowFlag;
	IplImage *m_pPrymid,*m_pPrevPry,*m_pSwapTemp;			//用于光流法跟踪的金字塔缓存
	void mvOpticalFlow();
#endif

	//静态特征检测
	IplImage * m_pDiffFG;	
	IplImage * m_pForeSobel;
	IplImage * m_pIntegrateForeSobel;
	void mvStaticFeature();
	vector<MvForeRect> m_vecForeRect;

	//辅助函数
	void mvFilterImage(IplImage *pForeImage);
	bool mvCalObject(IplImage *pFkImage, vector<CvRect> &vecObj);
	void mvGetheadtoprect( IplImage *pCopyIntegrayImage, IplImage *pSobelIntegray, 
		CvSeq *counters, CvSeq *pOutputrect, CvMemStorage *childstorage, IplImage *imageedge );
	void mvGroupRect(vector<CvRect> &vecObj, CvMemStorage *childstorage);

	//人群密度估计
	bool  m_bHadCalHead;
	int m_nStatHeadCntTime;
	int m_nAChannelHeadCnt[20][20];
	vector<CvPoint> m_vecCandidateHead;
	int mvCountCrowdPeople( vector<CvPoint> &vecCandidateHead );
	void mvFindCandidateHead(IplImage * pFore, int nMinD, vector<CvPoint> &vecHead);	//找出备选的头部
	
	//结合静态特征及轨迹信息对目标进行聚类
	vector<MvGroupRect> m_vecGroup;		//存储聚类结果
	void mvGenGroupRect(vector<CvPoint> vecHead);		//结合静态特征及轨迹对目标进行聚类

	vector<CvRect> m_vecVibeForeRects;
	vector<DropRect> m_vecDropRgn;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////------------其他的函数及变量------------///////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CvPoint* mvpointListToPoint(PointList vPlist);
	inline float mvmy_velocity(CvPoint2D32f &pt1,CvPoint2D32f &pt2)
	{ 
		return sqrt( (pt2.x-pt1.x)*(pt2.x-pt1.x) + (pt2.y-pt1.y)*(pt2.y-pt1.y) ); 
	}
	inline float mvmy_velocity2(CvPoint2D32f pt1,CvPoint2D32f pt2)
	{ 
		return sqrt( (pt2.x-pt1.x)*(pt2.x-pt1.x) + (pt2.y-pt1.y)*(pt2.y-pt1.y) ); 
	}
	double  mvAngleDiff(double angle, double orientation);
	double  mvAngleDiff2(double angle, double orientation); //angle and orientation in the -pi/2 and pi/2
	double  mvAngleDiff3(double angle, double orientation);
	bool mvare_vectors_similar(CvPoint2D32f &pt1S, CvPoint2D32f &pt1E, CvPoint2D32f &pt2S, CvPoint2D32f &pt2E);
	
	template <typename T>
	void mvlimit_point_inside_image(T &pt);
	void mvlimit_2D32fPoint_inside_image( CvPoint2D32f &pt );	

	template<typename T>
	bool mvget_rect_of_points(int nPtNum, T* pAPt, CvRect &rect );

	//以下函数判断点及线段与矩形的包围关系
	bool	mvisLineIntersectingLineExtend(double x0, double y0, double x1, double y1,double x2, double y2, double x3, double y3);
	
	//以下函数用于ROI坐标转换
	void UpdateEventXY(int *nInX,int *nInY,int nX,int nY);	//更新事件发生坐标位置
	float mvGetOriX(const float fX);		//取得在原始图中的位置
	float mvGetOriY(const float fY);
	int mvGetEventX(const float fX);		//返回server需要的位置
	int mvGetEventY(const float fY);
	CvPoint2D32f mvGetOffsetPtOfEvent( const CvPoint2D32f offsetPt );
	int mvGetOri2EventX(const int nX);	//由原始图的位置得到事件检测需要的位置
	int mvGetOri2EventY(const int nY);
		
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////------------杂乱的函数及变量------------///////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool mvsort_group_track( const MyGroup &group, double *fSortTrackValA, int  *nSortTrackA, int &sortTrackNum, int mod);
	bool mvsort_group_track( simpleTrInfo ASimTrs[], const MyGroup &group, 
		         double *fSortTrackValA, int *nSortTrackA, int &sortTrackNum, int mod );	
	double mvgetAngleForTwoDirLine( double a1, double a2 );
	bool mvfilter_nomatch_group_track_byAngle( const MyGroup &group, bool  *bCompareTrackA);	
	int    mvget_realImgOri_size_of_vehicle(MyGroup &vehicle);	
	float mvMeanVehicleDisplace(const MyGroup &vehicle, short eff_len);
	bool mvjudgestdv( IplImage *image, CvPoint point1, CvPoint point2 );	
	bool mvgetvehiclestatue( float angel, CvPoint2D32f point, bool against );	

	void mvget_rect_for_vehicle(MyGroup vehicle_i, MyGroup vehicle_j, CvRect &rect_i, CvRect &rect_j);
	bool mvis_theheight_oflines_tooLow(int nInterHoriLNum, int *nAInterHoriLNo, float fmaxsizeY);
	bool mvis_lines_overlap_tooLittle(int nInterHoriLNum, int *nAInterHoriLNo, CvRect rect);
	bool mvis_intersectLines_tooLittle(MyGroup vehicle_i, MyGroup vehicle_j, int nInterHoriLNum, int *nAInterHoriLNo);
	bool mvare_vehicle_rect_suitLineRequire( MyGroup vehicle_i, MyGroup vehicle_j);
	bool mvis_tracksSimilar_between_twoObject(MyGroup smlObj, MyGroup bigObj, float fXRatioThres, float fYRatioThres, float fSmilarThres);	
	void mvInheritVehicleAttribForNomobelAppear( MyGroup& vehicle_dst, MyGroup& vehicle_reserve, MyGroup& vehicle_delete );
	bool mvis_theAfterOne_beMerger( MyGroup &vehicle_i, MyGroup &vehicle_j );	

	bool mvObjectIsSmall( MyGroup &vehicle, float k_x, float k_y );
	bool mvExistSimilarObject( int vehicle_idx, float k_x, float k_y, int objNum, int *objIndexArray, int &simiarNum, int *similarArray );
	bool mvisObjTrackSimilarWithOtherObj(MyGroup &vehicle, MyGroup &vehicle_com);	
	bool mvIs_vehicle_moveFore( MyGroup vehicle, MyTrackElem* pATrackElem[] );
	bool mvIs_vehicle_moveFore( MyGroup vehicle );
	bool mvAreTrBelongSameObject(int nVehForeTrNum, int *nVehForeTrId, int nOverTrNum, int *nOverTrId );
	
	//预测目标下次的位移
	bool mvPredictObjMvPt( MyGroup obj, CvPoint2D32f &objMvPt );
	bool mvGetGroupValidMoveAngle( const MyGroup &obj, float &objMoveImgAngle );
	bool mvGetGroupValidMoveAngle( int nTrSetElemNum,MyTrackElem* pATrackElem[], MyGroup obj, float &objMoveImgAngle );
	void mvGetObjectRealCentroid( MyGroup &vehicle );

	//找到车辆底部的位置
	void mvGetVicleBottomArea(MyGroup vehicle,int nTrNum,
		int *nTrIdA,CvRect &rectVicleBottom);

	//粗略判断是不是大车
	bool mvIsBigCar(CvRect rectObj, CvPoint ptObjMaxCar);

	//判断过流量线的是否为同一目标
	bool mvIsSameObject(MyGroup& group, MyGroup& pregroup);

	//判断目标是否为过小的
	bool mvIsSmallObj(MyGroup& group);

	//得到区域中的平均速度
	bool mvGetAvgVelOfBotArea(CvPoint ptBotLt, CvPoint ptBotRb, 
		float fObjAvgVel, float fObjOri, float &fVicleBottomVel);

	bool mvgetdiffimagearea(  CvRect rect ); //计算目标出现区域像素值为255的面积比
	bool mvGetBottomRectForTracks( int nTrNum, int *nTrIdA, CvRect &rect );
	int  mvget_suitNoShadow_HoriLine(int line_num, int *nInterAndSuitHoriLineIDA, int temp_max_size_y);
	bool mvare_twoVehicle_trackSimilar( MyGroup vehicle_i, MyGroup vehicle_j );
	bool mvget_objMoveDirection(MyGroup obj, int disThres, bool bSide[4], float &mvAngle);
	int  m_vehicle_idx, m_vehicle2_idx;
	int  m_nTrIDA[1000];  //保存一个目标中满足方向要求的轨迹的序号的数组，1000条足够了
	

	double mvcalc_roi_avg(IplImage *pIntegrateDiff,CvPoint2D32f pt, int win);	
	bool   mvis_point_out_mask(CvPoint2D32f pt);
	
#ifndef LINUX
		public:	
#else
		private:
#endif
	void mvGetTopPt( CvPoint bottomPt, double dHight, CvPoint &topPt );
	void	 mvprint_vehicle_info(int v_idx);
	CvRect m_rect;

#ifdef GUIZHOUYANSHI
	typedef struct __MvDropRect
	{
		double ts;
		vector<CvRect> vecRect;
		int nTime;
		__MvDropRect()
		{
			ts = -1;
			nTime = 0;
		}
	}MvDropObjRgn;

	vector<MvDropObjRgn> m_vecDropObj;

	void UpdateDropRgn();

#endif
	
public:
	CConfigInfoReader   m_cCfgInfoReader;  //配置读入器	
	CfgInterface4Server m_interface4Server;  //与服务器的一些接口

private:
	CIntellAnalyst      m_cMyIntellAnalyst;  	
	CConfigInfoReader   *m_pCCfgReader;

	
private:
	//interface with m_cMyIntellAnalyst

	//-----------------------------------------//
	//获取"用于目标确认的带时间戳的图像集"的指针
	MvObjConfStampImgSet* mvGetObjConfirmStampImgSetPointer( ) {
		return m_cMyIntellAnalyst.mvGetObjConfirmStampImgSetPointer( );
	}

private: 	
	//main

	//-------------------------------------------//
	//设置时间戳
	void mvSetTimeStamp( double dTs_Now );

	//将数据设置为图像
	IplImage* mvSetDataToImg( char* image_data );

	//视频稳像
	bool mvVideoStabilization ( IplImage *pSrcImg );

#ifdef WIN_SIMULATE_CAMCTRL4STOPDET
	//模拟在停车检测过程中的相机(球机或云台)控制
	bool mvSimulateCameraCtrl4StopDetect(IplImage *pSrcImg);
#endif

	//对每帧均进行智能分析
	void mvIntellAnalystEveryFrame( int64_t ts );

	//设置检测参数
	void mvSetParmValue( GetParamSet &ParamSet, int64_t tsInt64 );

	//对中断进行处理
	void mvProcessForInterrupt( );

	//形成点匹配的轨迹
	void mvFormPointsTracks( );

	//-------------------------------------------//
	//获取轨迹和目标的信息
	void mvGetTrackObjectInfo(
			int nTrSetElemNum, 
			int nObjSetElemNum );

	//获取得到事件报警和交通统计的结果
	void mvGetEventAlertTrafficStatResult(
			DetectResultList &detOutList, 
			bool bNeedStatistic );

	//调试事件报警和交通统计的结果
	void mvDebugEventAlertTrafficStatResult(
			DetectResultList &detOutList );

	//显示总共使用时间的结果
	void mvShowTotalTimeUse( );

	//-----------------test code----------------//
	//对获取更好的区域进行测试
	void mvTestBetterResultArea( );
};

#endif
