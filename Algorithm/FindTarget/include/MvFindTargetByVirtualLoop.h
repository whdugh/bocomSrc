// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   11:56
	filename: 	e:\BocomProjects\find_target_lib\include\MvFindTargetByVirtualLoop.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MvFindTargetByVirtualLoop
	file ext:	h
	author:		Durong
	
	purpose:	与应用端的接口已经算法的入口
*********************************************************************/
#ifndef MV_FIND_TARGET_BY_VIRTUAL_LOOP_H
#define MV_FIND_TARGET_BY_VIRTUAL_LOOP_H

#include <vector>
#include <deque>
#include <errno.h>
#include <fstream>
#include <cxcore.h>

#include "VibeModel.h"
#include "Calibration.h"
#include "IBackgroundModel.h"
#include "VirtualLoop.h"
#include "CarInfo.h"
//#include "BackgroundStatistic.h"
#include "MvPolygon.h"
#include "OpticalFlow.h"
#include "CarManager.h"
#include "OpticalFlowSift.h"
#include "DetectType.h"
#include "Mode.h"
#include "MvCornerLayer.h"
#include "MvLightStatusMem.h"
#include "MvTrackLayer.h"
#include "MvGroupLayer.h"
#include "ViolationInfo.h"
#include "MvMaxSize.h"
#include "MvLine.h"
#include "MvChannelRegion.h"
#include "SynCharData.h"
#include "MvRectTracker.h"
#include "VehicleFlowHelper.h"
//#include "BgStatistic.h"
#include "RoadStatistic.h"
#include "MvBgLine.h"
#include "MvLoopManager.h"
#include "MvUnionManager.h"
#include "CornerBackgroundModel.h"
#include "MvLightTwinklingPreventer.h"
#include "ColorRecognition.h"
#include "MvLineStorage.h"
#include "RoadTypeDetector.h"
#include "MvLineMerge.h"
//#include "OnBusVioDet.h"
#include "MvFixRedLight.h"
#include "MvRadarSpeed.h"
#include "Mv_LoopshutterLight.h"//选择爆闪图片加入的头文件
#include "MvSurf.h"
#include "MyVideoStabilizer4Client.h"
#include "Cshadow.h"

#include "OpticalFlowHelper.h"
#define MAP_TS_TO_FRAMESEQ_SIZE 8000




#ifndef LINUX
  #define Local_Test //本地进行测试
#else
   #undef Local_Test
   #define int64 long long
#endif

using namespace PRJ_NAMESPACE_NAME;



class MvUnionManager;
struct _LoopInfo;
//class MvViolationDetecter;
//class OnBusVioDet;
class MvPhysicLoop;
class MvRadarSpeed;
class Mv_LoopShutterLight;
class OpticalFlowHelper;


typedef struct _RedLightTwinkRgn
{
	int    nRoadIndex;
	int    nDriection;//-1左转，0前行，1右转
	CvRect redRgn;
	CvRect greenRgn;

	_RedLightTwinkRgn()
	{
		nRoadIndex = -2;
		redRgn.width = 0;
		greenRgn.width = 0;
	}
}RedLightTwinkRgn;

typedef struct _MVVIDEOSTD {	
	vector<CvPoint2D32f> vPList;	
	_MVVIDEOSTD()
	{		
		vPList.clear();
	}
}mvvideostd;

typedef struct _MVLOOPSTATUS{

	int nRoadIndex;
	bool status;
	_MVLOOPSTATUS()
	{
		nRoadIndex = -1;
		status = false;//false:异常，ture：正常
	}
}mvloopstatus;

typedef struct _MVOBJPAR{
		int  ParkId;
		CvRect paRec;
		_MVOBJPAR()
		{
			ParkId = -1;
			paRec  = cvRect(0,0,1,1);
		}

}mvobjpar;

//将目标检测&电子警察里的控制参数改由客户端配置！
typedef struct _ParaConfig
{
	//目标检测！
	bool      bDetectNonVehicle;              //是否检测非机动车!    默认为true!
	bool      bFilterSideVehicle;             //是否过滤边上机动车!   默认为true!
	bool      bFilterSideVlpObj;              //是否过滤车牌检测区域两边的*****目标！  默认为false！
	bool      bUseSurf;                       //是否使用Surf进行视频测速！  默认为false!
	bool      bUseExternalCtrl;               //视频测速时，是否外部控制车牌距地面的像素高度!  默认为false!
	int       nCarPlateHeightOffGround;       //视频测速时，车牌距地面的图像投影像素大小！   典型值为70！
	int       nFramePlus;                     //线圈目标输出结果的帧号修正量!   典型值为0！
	float     fLoopVehicleLenFix;             //线圈车长修正量!    典型值为1.0m！
	bool      bOutPutNonAutoMobile;           //晚上是否输出信号目标，对星号目标过暗的点位进行过滤，默认为输出
	bool      bDoNonPlateDetect;              //做无牌车检测，默认检测，为true！

	//电子警察！
	bool      bUseChongHongDeng;              //是否使用冲红灯功能！默认为false!
	bool      bUseJudgeTurning;               //是否使用转向车辆的直行闯红灯抑制功能！  默认为false!
	bool      bUseMask;                       //是否使用Mask加速计算！  默认为false!
	int       nRedLightViolationAllowance;    //闯红灯信号开始的延迟秒数!   典型值为3！
	float     fRedLightVioPic[2];             //闯红灯取图参数!   far = fRedLightVioPic[0], near = fRedLightVioPic[1];  far、near的典型值分别为1.0、0.5，注意顺序不要搞反！


	_ParaConfig()
	{
		bDetectNonVehicle           = true;
		bFilterSideVehicle          = true;
		bFilterSideVlpObj           = false;
		bUseSurf                    = false;
		bUseExternalCtrl            = false;
		bOutPutNonAutoMobile        = true;
		bDoNonPlateDetect           = true;
		nCarPlateHeightOffGround    = 70;
		nFramePlus                  = 0;
		fLoopVehicleLenFix          = 1.0;

		bUseChongHongDeng           = false;
		bUseJudgeTurning            = false;
		bUseMask                    = false;
		nRedLightViolationAllowance = 3;
		fRedLightVioPic[0]          = 1.0;
		fRedLightVioPic[1]          = 0.5;
	}
};

class MvFindTargetByVirtualLoop
{
public:

	// 构造函数
	MvFindTargetByVirtualLoop();

	~MvFindTargetByVirtualLoop();

	CvRect GetCarRec( CarInfo *pCarIf)const;

	IplImage **   GetSurfImage()
	{
		return m_PreviousImg;
	}
	int64 *       GetCorrectionTime()
	{
		return m_lastTimeStamp;
	}

	MvViolationDetecter* GetViolationDetctor()
	{
		return m_pViolationDetecter;
	}

#ifdef USE_DSP_CODE_FIND
	void mv_SetYdatetoIplImage( unsigned char *pYdate, int nWidth, int nHeight );
	IplImage *pYimage;	
#endif

	// -1未知，0绿灯，1红灯/黄灯。
	int LightColorDet(IplImage *img, IplImage *imgCmp, unsigned int uSeq);


	//dsp会传进来此张图片是否是爆闪灯的图片，在调用DoAction之前set此标志，为后续的取图带来帮助
	//亮图是true，暗图是false
	void SetShutterPictureFlag( bool ShutterFlag = false );

	//表明和Dsp相机相连做线圈爆闪
	//做dsp线圈爆闪是true，做pc上的线圈爆闪是false
	void SetDspLoopShutter( bool bFlag = false );


	void SetRedLightStrenthForSingle( IplImage* img0, ChnFrmSignalStatusList cfss, int nShiftX, int nShiftY );

	void SetPressLineScale( float fscale = 0.2 );

	CvRect TransCheckShrinkToRio(CvRect Rec);//将角点压缩图像下的坐标转化成原始图像



	// 
	void DoAction(IplImage *img0, const std::vector<CarInfo> &vecCarNums, 
		ChnFrmSignalStatusList &sigStatusList, int64 ts, 
		unsigned int frameSeq, unsigned int uTimestamp, int imgIndex, 
		bool bCtrlCamera, std::vector<CarInfo> &ret, std::vector<ObjSyncData> &syncData, 
		std::vector<ViolationInfo> &vecElePoliceRet, StatResultList& listStatResult,
		int &bTrafficLightOn,
		unsigned int uMaxFrameSeq = 0, unsigned int uMinFrameSeq = 0); 

   void mvShowOutVio(const ViolationInfo &OutVio,char Imgpath[125],bool bSrcImg =true);
	

	// 获取程序收到的图像的宽、高（像素）
	int       GetImageWidth() const;
	int       GetImageHeight() const;

	


	//通过客户端配置目标检测&电警需要的参数！
	void      SetParaForVlpEle(const _ParaConfig &vlpPara);

	// 设置模式，白天、晚上。
	void      SetMode(Time_Mode mode);

	// 帧率。
	void      SetFrameRate(float fRate) const;
	

	// 设置屏蔽区域
	void      SetInvalidRgns(std::list<MvPolygon> &rgns);

	void SetYelGridRgn(MvPolygon &YelGridrgn); 

	void SetVioRushTime(unsigned int uRushTime); 
	

	std::list<MvPolygon> GetInvalidRgns() const;


	void      SetCurrentAndModelGain( uchar uCurrentGain, uchar uModelGain );

	void      SetVideoStdlib( vector<mvvideostd> pVideoStd );



	// 设置smear
	void      SetSmear(CvRect *pRects, int count);


	void      SetRedGreenScale( float fScale );


	


	// 设置检测类型。
	// DO_CAR_NUM_DETECT车牌检测、DO_FIND_TARGET卡口目标检测、DO_ELE_POLICE电子警察。
	// 例：SetDetectType(DO_CAR_NUM_DETECT | DO_FIND_TARGET);
	void      SetDetectType(unsigned int detType);

	//定义是否使用爆闪灯
	void SetLoopShutter( bool UseShutter );//true表示使用，默认不使用

	void SetVideoShutter( bool UseShutter );//true表示使用，默认不使用

	//void SetDropNonAutoMobile( bool bDrop );


	
	unsigned int GetDetectType() const;


	void      SetTrafficStatTime(const int nTrafficStatTime);

	// 设置标定点
	// nCount是corner个数。每个角点image,world都是2维。c1.x, c1.y, c2.x, c2.y.......
	void      SetCalibCorners(float *image, float *world, int nCount);


	// 设置图像尺寸
	void      SetImageSize(int nWidth, int nHeight);

	// 设置车牌检测区ROI。程序根据这个roi设定自己的roi
	void      SetCarNumDetectROI(CvRect rectROI);
	
	// 设置方向
	// direction: 0由远到近。1由近到远
	void      SetDirection(int direction=0);

	int       GetDirection() const
	{
		return m_nDirection;
	}
	

	void      SetColorDetector(ColorRecognisize *pColorDetector);




	//设置是否需要红灯增强和防闪功能
	void      SetRedAndGreenStrongGlemd( bool bStrong, bool bGlemd, bool b_CheckLightByImage );

	//设置视频触发爆闪灯方案
	void      SetVideoShutterPar( vector<mvvideoshutterpar> videoshutterpar );

	// 设置车道
	void      SetChannels(const std::vector<ChannelRegion> &cs);

	// 设置停止线
	void      SetStopLine(const MvLine &l);

	// 设置前红灯线
	void      SetForeLine(const MvLine &l);

	// 设置右红灯线
	void      SetRightLine(const MvLine &l);

	// 设置左红灯线
	void      SetLeftLine(const MvLine &l);

	// 压线违章线。
	void      SetYellowLine(const std::vector<MvLine> &vecYellowLines);

	// 压白色违章线。
	void      SetWhiteLine(const std::vector<MvLine> &vecYellowLines);

	// 设置红灯区域，对于非频闪红灯不需要设置。
	void      SetRedLightRgn(RedLightTwinkRgn rgn);

	void      SetVideoShutterDelayFrame( int nDelayFrame = 3 );

	//获取违章检测设置
	bool      GetViolationDetection()
	{
		return m_bViolationDetection;
	}

	// 设置线圈信息。在初始化之前调用。如系统中无线圈，可不调用。
	// nDelay表示串口传输延时，暂时设0
	void      SetPhysicLoop(const std::vector<_LoopInfo> &li, int nDelay = -92234);//-352000

	//设置雷达检测！
	void      SetRadarDetection(CvRect rectROI, std::vector<int> radarRoadIndex);

	//传递雷达速度！
	void      SetRadarSpeed(unsigned int frameSeq, long long timeStamp, double speed, double fastSpeed);
	// 每帧告之线圈状态。在调用DoAction之前调用。如系统中无线圈，可不调用。
	// 如果有多个车道，需要多次调用。
	// counter为计数器值
	// ts为系统时间。区别于DoAction的ts
	// b0，b1分别为编号为0，1的线圈的状态。true表示高电平
	void      SetPhysicLoopStatus(int nRoadIndex, bool b0, bool b1, long long counter, long long ts/*, unsigned int uFrameSeq*/);

	void      GetPhysicLoopStatus( vector<mvloopstatus> &nLoopStatus );

	void      _ScaleParas();
	// 初始化
	void      Init();

	// destroy all the member objects.
	// invoke it after all operations have been done to release memory.
	void      Destroy();
	
	
	unsigned int GetMaxSeq() const
	{
		return m_uMaxSeq;
	}

	// 从原图得到check 的区域
	CvRect    GetCheckRoi() const;


	CvRect    GetVlpRoi() const;
	
	
	Time_Mode      GetMode() const;


	CvRect    GetCarNmRoi() const;

	void          GetCarNumRoiUpDownLines(MvLine &lnUp, MvLine &lnDown);

	MvPolygon  GetYelGridRgn()const; 

	unsigned int GetVioRushTime()const; 


	const std::vector<ChannelRegion>& GetChannels() const;


	void  mvGetSensObjChanel(CvRect Rgn ,int &nChanlIdex ,int &nChannelDir);


	


	//MvPhysicLoopManager* GetPhysicLoopManager() const;


	// 
	//void      TestOpticalFlowLK(IplImage *pCurFrame, IplImage *pLastFrame);
	
	
	// 计算帧差图像,输入当前帧图像frameImage。
	// 该方法用当前帧与上一帧相减，并将结果填充到frameDiffImage图像。
	void          GetFrameDiffImage(IplImage *frameImage, IplImage *frameDiffImage) const;


	// 获取图像缩小比例。缩小是为了计算更快。
	float         GetVlpScaleX() const;
	float         GetVlpScaleY() const;
	float         GetCheckScaleX() const;
	float         GetCheckScaleY() const;

	
	// 是否检测非机动车。
	bool          GetDetectNonVehicle() const;
	void          SetDetectNonVehicle(bool bDetOrNot);

	
	// 获取相对于传入图像的标定对象。
	MyCalibration* GetCalibrationObject() const;


	MyCalibration* GetCheckImgCalibObj() const;


	VehicleFlowHelper* GetVehicleFlowHelper() const;


	MvPhysicLoop* GetPhysicLoopByRoadIndex(int nRoadIndex);


	MvBgLine* GetCheckImgLineBGM() const;


	ColorRecognisize* GetColorDetector() const;


	MvLineStorage *GetLineStorageObj() const;

	// 根据ts找帧号。返回0表示找到，返回-1表示给的ts小于记录的最小的时间，返回1表示ts大于记录中的最大的时间。
	// tips：返回-1表示可能需要扩大记录容量。返回1意味着过一会再取可以取到。
	int          FindFrameSeq(int64 ts, unsigned int &seq, bool bBigOrSmall);

	int			 FindTimeByFrameSeq( unsigned int seq, int64 &ts, bool bBigOrSmall ) const;

	float GetEleScaleX() const
	{
		return m_fEleScaleX;
	}


	unsigned int GetDetype()const
	{
		return m_uDetectType;
	}
	float GetEleScaleY() const
	{
		return m_fEleScaleY;
	}

	int   GetCarPlateHeightOffGround() const
	{
		return m_nCarPlateHeightOFFGround;
	}
	// 将imgForeground图像中的影子去除，并且记录影子区域。
	// imgCurFrame 是当前帧图像。imgBackground是背景图像。imgForeground是当前帧与背景相减并二值化的结果。
	// 该方法将imgForeground图像中阴影处像素值设为0。并记录阴影区域。
	void          RemoveAndRecordShadow(IplImage *imgCurFrame, IplImage *imgBackground, 
									IplImage *imgForeground, std::vector<CvRect> &vecShadowRgn);


	// Rgb差分。当前帧图像，与背景比较，将有显著差异的区域用imgFore返回，将明显没有变化的区域用imgBack返回。
	void          RgbDiff(const IplImage *imgCurFrame,const IplImage *imgBackGround, IplImage **imgFore, int nThreshold = 30, bool bMaxDiff = true, bool binv=false);


#ifdef   LGW_GOAL_IMG
	 static IplImage *gFramImg ;//在group中测试使用
	 static CvRect    gcheckRegion;
	 // vlp 在帧图像中的位置（相对于原图）
	 static CvRect    gvlpRegion;
	 static float Xsize;
	 static float Ysize;
#endif


	public:
		int64  m_Time;
		MvCorner **m_pCorPointerFm;//没有分配空间

		//为无锡测试，线圈测速有跳变用，防止线圈跳变引起误差
		float fLastSpeed;
		unsigned int m_nMinSeq;
	




private:

#ifdef SHOW_WINDOW
		void mvDescirGroupTrack(const IplImage* imgCornerRgb,const unsigned int frameSeq,
			const vector<MvTrack*> &vecTracks,const list<CvRect> &lstRectForDebug,
		const list<MvPolygon> &lstPolyForDebug,const vector <CvRect> &CarGroupParkRect,const vector<CarInfo> &vecCarNums);
#endif
   

	void mvDetCartype( IplImage*imgCornerRgb,  MyCalibration*pCalibCommon,const unsigned int frameSeq);

	void mvLineSegModel(IplImage* imgCorner,const unsigned int frameSeq);

	void mvGetTrackByDervieConer(vector<MvTrack*> &vecTracks,
		vector<MvTrack*> &vecEleTracks,
		MvCorner **vCorPointer,int &nCornerCount, 
		IplImage *imgGrayFrm, const unsigned int uFrameSeq,const int64 ts, const unsigned int uTimestamp,
		const  MyCalibration *pCalib, const MvMaxSize* pMaxSize,   const bool bVio_Target,
		const std::vector<CvRect> &CarGroupPark);

	//获取电警违章中车辆停靠的信息
	void GetVioPark(vector <CvRect> &CarGroupParkRect);

	//设置最大角点个数
	void SetMaxCorner(int &nMaxCornerCount,const bool bViolation_Target);

	//获取对应图像数据
	void GetImg(IplImage *img0,IplImage * &vlpImage,IplImage * &imgCorner,IplImage * &imgCornerRgb);

	//记录帧号和时间信息
	void RecordTimeAndSeq(const unsigned int frameSeq,const int64 ts);

	void VideoShutRecod(IplImage *img0,const unsigned int frameSeq,const int64 ts);

	//根据Xml记录信息
	void SaveDatabyXml( IplImage *img0,const vector<CarInfo> &vecTrueCarNums
		,const ChnFrmSignalStatusList &sigStatusList,const unsigned int frameSeq,const unsigned int uTimestamp
		,const int64 ts,const bool bCtrlCamera) ; 

	//获取无损牌信息
	void GetNoPlate(vector<CarInfo> &vecTrueCarNums, vector<CvRect> &vecDirtyPlates,const vector<CarInfo> &vecCarNums
		,unsigned int frameSeq,unsigned int uTimestamp);

	void      SetMaxMinSeq( unsigned int &uMaxFrameSeq, unsigned int uMinFrameSeq,unsigned int frameSeq);

	
	void mvGetParkTrackData(int &nParkingShortTrackAroundLongTracks,int &nLongTrackCount,
		int &nParkingLongTrackCount, const vector<MvTrack*> &vecTracks,unsigned int frameSeq,
		const bool bDoTarDet,const IplImage *imgCornerRgb);



	bool IsCameraTuned(IplImage *imgFrame);




	// 将地感线圈获得信息和车牌及SensedObj关联。
	void AssocPhyLoopInfoToCarSensedObj();

	void PhysicLoopOutput(unsigned int uFrameSeq, int64 ts, std::vector<CarInfo> &ret, std::vector<ObjSyncData> &syncData);


	//判断同一辆车是否在不同车道既检出车牌，又检出*+++*的情况，是则返回true，否则false！
	bool IsSameCarOnTwoLoops(const CarInfo &car, const vector<Car*> &cars, const vector<ChannelRegion> &cr, _MvUnion **pUnion, Car **pRetCar);


	//车辆是否靠近道路中间行驶！
	bool IsCarOnBetweenRoads(const Car *pCar, int nCurRoad, int nLoopRoad, const vector<ChannelRegion> &cr);

	void          DoTargetDetect( IplImage *vlpImage, IplImage* pCheck, IplImage* pRgbCheck, 
								std::vector<MvTrack*> vecTracks, MvCorner* vCors[], int nCorCount, 
								int nLongTrackCount, int nParkingLongTrackCount, int nParkingShortTrackAroundLongTracks,
								unsigned int uFrameSeq,
								unsigned int uMaxFrameSeq, int64 ts, 
								unsigned int uTimestamp, int nImageIndex, bool bCtrlCamera, 
								std::vector<CarInfo> &ret, std::vector<ObjSyncData> &syncData );


	void          DoElePolice();

	
	// 在图上画电子警察的四条线和车道。一般用于调试。
	void          DrawLinesAndChannelRegion(IplImage *img);


	//
	void          DrawChannelRegions(IplImage *img);

	







	// 计算sobel边缘图像
	//void          GetSobelImage(IplImage *frameImage, IplImage **sobelImage);


	// 截取虚拟线圈部位图像
	IplImage*     GetVlpImage(IplImage *frame,CvPoint TranPoint= cvPoint(0,0));


	// 截取check部位图像
	void          GetCheckImage(IplImage *img0Resized, IplImage **imgCheckGray, IplImage **imgCheckRgb
		,CvPoint TranPoint= cvPoint(0,0));

	// 擦除图像rect以内的像素。使变为0
	//void          ClearRects(IplImage *img, const vector<CvRect> &rects);

	// 设置vlp在原图中的位置
	//void          SetVlpRegion(const CvRect &vlpRegion, const CvRect &sobelRegion);

	

	// 对于不在任何车道里的车辆，修正一下车道信息。选择一个离它近的车道。
	void          FixRoadIndex(CarInfo &ci);

	void          FixSpeed(CarInfo &ci);


	// sobel image OR diff image
	//void          CombineSobelAndFrameDiffImage(IplImage* sobelImage, IplImage *frameDiffImage, IplImage **destImage);


	// check foreground rects using checking rects
	//void          CombineForegroundRectsAndCheckRects(vector<CvRect> foregroundRects, vector<CvRect> checkRects, vector<CvRect> &result);

	// map the rects in check image to vlp image
	void          MapCheckRects(std::vector<CvRect> &rects);

	// find lines in img using hough transform
	//void          GetLines(IplImage *img, vector<CvPoint> &ret);

	// clear lines in image
	//void          ClearAlongLines(IplImage *img, vector<CvPoint> lines); 

	// 用于在服务器上跑视频，如果帧号变小则重新初始化，但是这个过程又有别于Init，所以定义ReInit
	void          ReInit();

	// 在传入图像的缩小图（原图按照checkscale缩小）上将屏蔽区域涂黑。
	void          ZeroUsingInvalidRgn(IplImage *img,CvPoint TranPoint= cvPoint(0,0));

	//为闯红灯提取角点准备Mask图像，以提高效率！
	bool          _GetMaskForCornersExtraction(IplImage *maskImg);

	//void          GetCarNumRoiUpDownLines(MvLine &lnUp, MvLine &lnDown);

	//判断候选无牌车是不是由于污损车牌导致的！
	bool          GetLikelyDirtyPlate(const SensedObject *pOutputObj, CvRect objPos, map<int, vector<CvRect> > mapDirtyPlates);

	
	public:

	    IplImage *m_pInVaildMaskImg;

		MvMaxSize*     GetMaxSize()
		{
			return m_pMaxSizeCommon;
		}
			CvRect m_ParkRec ;
			int m_nVlpCarWidth;
			bool m_bNOParBacUpd;


			// 在红灯亮起之后的m_nRedLightViolationAllowance帧内这一段时间闯红灯的不算。
			int                 m_nRedLightViolationAllowance;
		OpticalFlowHelper *      m_pOpticalFlowHeler;
		inline bool mvGetDoEleVio()
		{
			return m_bVioRedLightWithViolation;
		}
		// 电警违章检测器。
		MvViolationDetecter  * mvGetEleObject()
		{
			return m_pViolationDetecter;
		}

#ifdef TRACK_GROUP_LGW
		void mvGroupUpdatCar(const unsigned int frameSeq);
		void mvGroupAssoCar(const unsigned int frameSeq,vector<MvTrack*> &vecTracks,vector<OpticalFlowHelper::FlowSetInfo>&vecFlowSets);
#endif

private:
	
	IplImage           *m_preditImg;
	//int              m_nLastCID;
	CvRect             m_VlpAradRec;

	int                m_nDelayFrame;

	bool               bDspLoopFlag;//dsp爆闪标志
	bool               bShutterFlag;//为dsp传进来的图是否是爆闪灯的图标记。dsp专用
	
	ColorRecognisize * m_pColorDetector;

	//为Surf匹配缓冲两帧图像，能不能利用m_pLastFrame?用缩小图？
	IplImage         *m_PreviousImg[2];  //当前图像的前二帧图像，灰度图，暂时用原图！
	int64			  m_lastTimeStamp[2];  //时间！
	unsigned int      m_uLastFrm[2];   //两帧帧号！

	//////////////////////////////////////////////////////////////////////////
	//                         工作模式无关的
	//////////////////////////////////////////////////////////////////////////



	//
	unsigned int     m_uCornerBgmLastUpdateSeq;


	bool             m_UseShutter;
	bool             m_UseVideoShutter;

	uchar			 m_ModelGain;
	uchar			 m_CurrentGain;


	int64           m_FramCout;

	float           m_fScale; 

	//
	//int              m_nLastOutputId;
	bool             m_bRoadStat;


	// 输入原图的宽度
	int              m_nImageWidth;


	// 输入原图的高度
	int              m_nImageHeight;



	// 标定角点坐标。相对于传入图像。
	double           m_calibCornerImage[20];
	double           m_calibCornerWorld[20];
	int              m_nCalibCornerCount;


	// 於峰传递给我的车牌检测区
	CvRect           m_roiCarNum;


	// 於峰传递给我的车牌检测区
	MvPolygon           m_YelGridRgn;
    unsigned int     m_VioRushTime;        //秒为单位


	//雷达测速的检测区域，由应用端传入！
	CvRect           m_rectOverlap;
	std::vector<int> m_vecRadarDetectionRoadIndex;  //雷达覆盖的检测车道！
	// 工作模式，夜间or白天。0白天，1夜晚
	Time_Mode             m_mode;

	//Time_Mode             m_prevMode;


	// 方向，前牌（从远到近）为0，后面（从近到远）为1
	int              m_nDirection;


	

	// 屏蔽区域。
	std::list<MvPolygon>  m_lstInvalidRgns;


	// 屏蔽区域为0，非屏蔽区域为1.
	//IplImage*        m_pValidMask;


	//mask，用于提取角点，暂时只用在闯红灯情况，由前行线、左转线、右转线构造出！
	IplImage         *m_maskGrayImg;
	bool              m_bUseMaskImg;
	bool              m_bGotMaskImg;
	// 工作方式.参加DetectType.h
	unsigned int     m_uDetectType;

	RoadStatistic*   m_pRoadStat;


	int              m_nTrafficStatTime;

	int              m_nLastOutputTimestamp;




	//
	//MvPhysicLoopManager* m_pPhysicLoopManager;
	MvLoopManager*   m_pLoopManager;
	bool             m_bDoPhysicLoop;
	std::vector<_LoopInfo> m_vecLoopInfo;
	int              m_nLoopDelay;
	bool             m_bMultiChannels;  //是否为多车道，用于解决同一辆车在不同车道既检出车牌，又检出*+++*的问题！
	//雷达相关
	MvRadarSpeed *   m_pRadarSpeed;  //速度map!
	bool m_bDoRadarSpeed;  //雷达测速！
	bool m_bRadarParaValid;  //雷达参数设置是否有效，无效则不做雷达检测！
	

	//// ts->uFrameSeq
	//std::map<int64, unsigned int> m_mapTsToSeq;
	std::deque<int64> *m_ts;
	std::deque<unsigned int> *m_seq;

	//MvDeque<int> *m_queue;


	MvUnionManager *         m_pUnionManager;


	std::map<int, int> m_mapUnionIdToObjId;
	int m_nNextObjId;

	// 已经输出了的车辆。
	std::map<int, _CarInfo> m_mapOutputedVehicle;

	//记录每帧传入的污损车牌，用于判断无牌车是否可信！
	std::map<int, vector<CvRect> > m_mapDirtyPlates;

	// 记录应用传进来的数据的文件流。
	std::ofstream*          m_ofsRecordParas;
	std::ofstream          *m_ofsDirtyPlates;  //记录污损车牌信息，用于判定无牌车！


	// 角点、轨迹层。
	MvCornerLayer*          m_pCornerLayerCommon;
	MvTrackLayer*           m_pTrackLayerCommon;
	MyCalibration*          m_pCalibCommon;
	MvMaxSize*              m_pMaxSizeCommon;

	VibeModel  m_VibelModel;
	IplImage   *m_VlpBakImg;
	IplImage   *m_vlpForImg;



	


	



	// 检测角点的图像（缩小的）上的直线背景模型。
	MvBgLine*         m_pLineBGM;
	


	MvLineMerge*      m_pLineMerge;
	

	
	bool              m_bFilterSideVlpObj;  //是否抑制边边角角的*****目标！

	
	//////////////////////////////////////////////////////////////////////////
	//                       电子警察的
	//////////////////////////////////////////////////////////////////////////

	// 电子警察的辅助线（相对于电警缩小图）
	MvLine           m_stopLine;
	MvLine           m_foreLine;
	MvLine           m_rightLine;
	MvLine           m_leftLine;


	MvLine           m_stopLineOri;
	MvLine           m_foreLineOri;
	MvLine           m_rightLineOri;
	MvLine           m_leftLineOri;

	std::vector<MvLine> m_vecYellowLine;

	std::vector<MvLine> m_vecWhiteLine;


	std::vector<MvLine> m_vecBianDaoXian;


	// 电警违章检测器。
	MvViolationDetecter  *m_pViolationDetecter;

	// 电警图上的maxsize
	MvMaxSize*       m_pMaxSize;

	// 电子警察角点、提取图的缩放比。
	float            m_fEleScaleX;
	float            m_fEleScaleY;

	// 电子警察的标定对象。
	//MyCalibration*   m_pEleCalib;



	// 电子警察
	//MvCornerLayer       *m_pCornerLayer;

	// 电子警察
	//MvTrackLayer        *m_pTrackLayer;

	// 电子警察
	MvGroupLayer        *m_pGroupLayer;

	// 电子警察
	MvLightStatusMem    *m_pLightStatusMem;


	CMyVideoStabilizer4Client m_videostabilizer;

	//CVideoStabilizer  m_cStab;

	// 
	//MvCarPlateTracker   *m_pCarPlateTracker;


	//有没有做违章检测！
	bool                m_bViolationDetection;
	bool                m_bVioRedLightWithViolation;
	bool                m_bElepoliceParaValid;  //违章线有没有设置好等！
	//////////////////////////////////////////////////////////////////////////
	//                       卡口相关的
	//////////////////////////////////////////////////////////////////////////
	
	RoadTypeDetector* m_pRoadTypeDet;



	// 背景模型状态，1期，2期，3期。
	int               m_nBgmStatus;
	unsigned int      m_uLastCameraControlSeq;

	
	// 目标检测用的角点层与轨迹层。
	// 如果好用，可能将其提出来为电子警察和卡口共用。暂时单独放。
	//MvCornerLayer*    m_pCornerLayerFtg;


	//MvTrackLayer*     m_pTrackLayerFtg;

	// check图像的标定（小图）。
	//MyCalibration*    m_pCalibCheck;


	// stores smear position.暂时没有使用。
	//std::vector<CvRect>    m_vecSmear;


	//unsigned int      m_uLastBgmUpdateSeq;

	

	//Surf匹配修正视频速度！
	MvSurf			*m_pSurf;
	int             m_nCarPlateHeightOFFGround;  //车牌至地面的图像高度!

      

	
	// 标定对象, 相对于传入原图。
	MyCalibration*    m_pCalibOri;


	// check图上的maxsize。
	MvMaxSize*       m_pMaxSizeFtg;

	// 背景模型对象
	IBackgroundModel* m_bgm;

	unsigned int      m_uBgmLastUpdatedSeq;


	float m_fScalWidht;//车身压线上的百分比


	//tmp
	//bool m_bBgmValid;


	IBackgroundModel* m_pLongTermBgm;
	unsigned int      m_uLastUpdateLongTermBgmSeq;


	// 虚拟线圈对象
	VirtualLoop*      m_vlp;

	//int               m_nBgUpdateRate;


    
	// check 区域（相对于原图），提角点的区域，通常是原图的下2/3
	CvRect            m_checkRegion;
	
	// vlp 在帧图像中的位置（相对于原图）
	CvRect            m_vlpRegion;
	
	// 上一帧图像，用于帧间差分
	IplImage*         m_pLastFrame;

	// 差分图像Buffer
	IplImage*         m_diffImage;

	// checkImage缩放比例x
	double            m_dCheckImageScaleX;
	
	// checkImage缩放比例y
	double            m_dCheckImageScaleY;

	// vlp image放大系数
	double            m_dVlpImageScaleX;
	
	// vlp image放大系数
	double            m_dVlpImageScaleY;


	// 卡口中是否检测非机动车。如果为false不输出非机。
	bool              m_bDetectNonVehicle;

	//是否检测无牌车！
	bool              m_bDoNonPlateDetect;

	// 
	VehicleFlowHelper* m_pVehicleFlowHelper;


	// 用以辅助判断背景是否突变（失效）
	//BgStatistic*      m_pBgStatistic;

	

	//提取直线的复用！
	MvLineStorage    *m_pLineStorage;

	// 直线背景模型是否调用过更新函数。
	bool              m_bCheckImgLineBgmUpdated;

	unsigned int      m_uLastCheckImgLineBgmUpdatedSeq;


	CornerBackgroundModel<CvPoint2D32f> *m_pCornerBGM;


	//线圈异常计数器
	int m_nLoopCounters[ROADINDEX_BUFFER_SIZE][4];//最多支持5个车道,每个车道线圈有四个状态0：正常 1：线圈0异常 2：线圈1异常 3：线圈全部异常


	Cshadow m_shadow;


	//////////////////////////////////////////////////////////////////////////
	// 防止红灯闪功能。
	//////////////////////////////////////////////////////////////////////////

	//MvLightTwinklingPreventer* m_pLightTwinklingPreventer;
	//CvRect                     m_TwinklingRgn;
	MvLightTwinklingPreventer* m_redLightHelper;
	MvLightTwinklingPreventer* m_greenLightHelper;


	// 是否开启防止灯闪功能。
	//bool              m_bPreventLightFlash;

	// 为添加项目暂时添加，需要完善。读通道，多方向，多种红灯行驶。
	//CvRect            m_RedLightRegion;


	// 红灯区域灰度。
	//std::vector<float>  m_vecLightRgnGrayRecord;

	//int               m_lightThreshold;

	// 帧号->灯是否亮（不管红绿）
	//std::map<unsigned int, bool>  m_mapLightOn;

public:

		CarManager*       m_pCarManager; 
		vector<ViolationInfo> m_vecarViocod;//用来记录车牌没有关联Group，删除该车牌的编码
	  // 车道,电警用。缩小的。
	  std::vector<ChannelRegion> m_vecChannels;

		//查找爆闪灯图片
		Mv_LoopShutterLight*  m_pLoopShutterLight;

		bool m_bStrong;
		bool m_bGlemd;
		bool m_bCheckLightByImage;

		//稳像相关的
		int m_RegNum;
		int *pPointNumber;
		CvPoint2D32f **pPointStorage;
		std::ofstream*          m_ofsSpeedParas;
		MvGroupLayer    *GetGroupLayer()
		{
			return	m_pGroupLayer;
		}


public :
	vector<CvRect> m_GroupArea;
	// 用于重新初始化
	unsigned int     m_uLastFrameSeq;
	mvobjpar   m_ParObj;
	bool       m_bInitBacSuc;
	bool       m_vlPark;
	unsigned int  m_LastUpDataParkTime;
	bool         m_bFulDark;
	bool      m_bigPass;
	bool GetMostCloseLightOnFrame(unsigned int uFrameSeq, unsigned int &uLightOnSeq);
	// 车道,没缩小。
	std::vector<ChannelRegion> m_vecChannelsOri;
	IplImage *m_pinValidImg;
	bool  m_bTailMov;

private:

    CvRect TransHeadPsToNoVelPos(CvRect Headpos,bool bForPlat);
	void SaveSettings();

	void mvGetX86Version(char *ver);

	// pCalib相对于传入大图的标定。
	void _InitCheckRgnAndVlpRgn(MyCalibration* pCalib);

	void _InitSaveCalibCorners();




	// 同时检测卡口与电警时，为了共用轨迹，卡口的检测区域被自动扩大。
	// 这时候容易造成侧边并没有经过车牌检测区域的多报。
	// 该函数判断点是否在这个“多报区域”。
	// tips：坐标相对缩小的图。（同时检测时，缩小比例相同）
	bool IsPointInExRgn(int x, int y);

	// 清除虚拟线圈上的“多报区域”。
	// vlpImage是缩小后的。
	void ClearVlpExRgn(IplImage* vlpImage);

	

public:
	void SetBianDaoXian(const std::vector<MvLine> &vecBianDaoXian);

private:

	// 相机同步功能是不是打开了
	bool m_bSyncOn;

	// 是否进行图像两边上目标过滤。
	// 方法：将可以根据直线很确定的判定是小车、大巴、货车的过滤掉，不输出。
	bool m_bFilterOutSideMotorizedVehicle;


public:

	// 告知相机同步有没有打开。False，没打开。
	void SetSyncSwitch(bool bOnOrOff = false);

	// 返回m_bFilterOutSideMotorizedVehicle变量
	bool GetFilterOutSide();

	void ClearObjPar();
	unsigned int mvGetReasonFram(unsigned int uFram)const;


	CvRect GetCarNumRoi() const
	{
		return m_roiCarNum;
	}
private:

	


	unsigned int m_uMaxSeq;

	float        m_fFixedVeilcheLength;


	//OnBusVioDet* m_pOnBusVioDetector;


	bool      m_bOutPutNonAutoMobile;

private:

	void mvJudeOutPut(SensedObject* pOutputObj,vector<CarInfo> &ret, vector<ObjSyncData> &syncData,unsigned int frameSeq);
	void mvSenseOutPut(SensedObject* pOutputObj,vector<CarInfo> &ret, vector<ObjSyncData> &syncData,unsigned int frameSeq);

	bool DetermineWhetherOutputForFtgObj(SensedObject* pObj, MyCalibration* pCalib);
	
	//根据Tsai标定获得车牌离地面的图像高度！
	void _CalcCarPlateHeightOffGround();

	void mvSetCorossLineByCarRio();

private:
	RedLightTwinkRgn m_RPTR;  //没用！

//	std::vector<RedLightFixer*> m_pRLF;
	_ParaConfig m_InputParas;


public:
	RedLightFixer** m_pRLF; 
	//存储SensedObject目标的速度！
	FILE *m_pFILESaveSpeed;
	map <int,int64>m_PassChanlTime;
	
public:
	MvLine m_CrossSedLin;//车牌区域中心线-卡口压第二线 --针对前牌
	MvLine m_CrossFriLin;//车牌区域中心线--卡口压第一线---针对前牌

	MvLine m_EleCrossSedLin;//车牌区域中心线-电警第二线 --针对尾牌
	MvLine m_EleCrossFriLin;//车牌区域中心线--电警压第一线---针对尾牌


	CvRect m_CheckCarNumRio;
	CvMemStorage* m_pstorage; 
	CvSeq* m_point_seq; 
	float m_fline[2];
	bool  m_bLoopLearnSucces;
	IplImage *m_pChanlImg;
	IplImage *m_pLasCarGrayImg;
	int   m_CarRioY;//车牌区域最上线在cheak图上的
	vector<int> m_vecplatWid;//统计车牌的大小;
	int m_CarMidWidth;
	unsigned int m_presFram[STORE_FRAMES];
	int m_Stroepos;

	
	

};

#endif



















