// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef SKP_ROAD_DETECT_H
#define SKP_ROAD_DETECT_H

#ifndef NOEVENT

#include "global.h"
#include "road_detect.h"
//#include "StopVehicleDetect.h"
#include "RoadRecorder.h"
#include "CvxText.h"
#include "CameraControl.h"
#include "Mv_CarnumDetect.h"
#include "PRoadColor.h"


#include "EventDetect.h"
#include "PreSetControl.h"
#include "RoadH264Capture.h"
#include "md5.h"
//#include "EventVisVideo.h"

#ifdef  HANDCATCHCAREVENT
#include "HandCatchEvent.h"
#endif

#ifdef CAMERAAUTOCTRL
#include "CameraAutoCtrl.h"
#include "CamCtrlComCalc.h"
#include "Markup.h"
#include "OnvifCameraCtrl.h"
#include "RoadDetectPelco.h"
#include "RoadDetectDHCamera.h"
#endif

#ifdef DETECT_VEHICLE
#include "MvDetector.h"
#endif

#ifdef CAMERAAUTOCTRL
struct AutoCameraPtzTrack
{
	int nTaken;
	int nPanTiltX;
	int nPanTiltY;
	int nPanTiltZoom;
	int nBeginTime;
};
#endif


struct HandCatctEventInfo
{
	int nX;
	int nY;
	int nWidth;
	int nHeight;
	int nCarID;
};


#ifndef Roadmax
#define Roadmax(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef Roadmin
#define Roadmin(a,b)            (((a) < (b)) ? (a) : (b))
#endif


/******************************************************************************/
//	描述:智能交通检测系统通道图片采集检测实体类.
//	作者:徐永丰
//	日期:2008-5-12
/******************************************************************************/
//帧数据列表
typedef std::list<std::string> ChannelFrame;

class CSkpRoadDetect : public CEventDetect
{
public:
	//构造
	CSkpRoadDetect();
	//析构
	~CSkpRoadDetect();

public:
	//初始化检测数据，算法配置文件初始化
	bool Init(int nChannelID,int widthOrg,int heightOrg,int widthDst ,int heightDst);
	//释放检测数据，算法数据释放
	bool UnInit();

	// 最后合成图图片 2*2 格式的  nhs
	int ComPoseImage(RECORD_EVENT& event, int key);

	

	int ComPoseImageForYuntai(RECORD_EVENT event, int key);

	// 图片上加字， pImage key 是m_mapRemotePicList的键
	void ImageAddCharacter(IplImage * pImage, RECORD_EVENT event, int key,SRIP_DETECT_HEADER* pHeader);

	void ImageAddCharacter(IplImage * pImage, RECORD_EVENT event, int key);

//处理
    //事件检测处理
    void DealFrame();

	//添加一帧数据
	bool AddFrame(std::string& frame);

	//添加jpg图像帧
	bool  AddJpgFrame(BYTE* pBuffer);


//参数设置,设置检测参数[车道信息、检测线、检测值]
	bool SetDetectParam(paraDetectList& sFrameIn);

	//设置事件录象长度
    void SetCaptureTime( int nCaptureTime)
	{
		m_nCaptureTime = nCaptureTime;
		m_skpRoadRecorder.SetCaptureTime(nCaptureTime);
	}

	//添加事件录象缓冲数据
	bool AddVideoFrame(std::string& frame);

	//设置白天还是晚上检测
	void SetChannelDetectTime(CHANNEL_DETECT_TIME dType);

	//控制是否事件录象
	void SetEventCapture(bool bEventCapture) {m_bEventCapture = bEventCapture;}
	//修改事件录像
	void ModifyEventCapture(bool bEventCapture);

	//设置是否重新配置
	inline void SetReloadConfig(){ m_bReloadConfig = true;}

	//设置交通量检测周期
//	void SetChannelTrafficStatTime(const int nTrafficStatTime){m_nTrafficStatTime = nTrafficStatTime;}
	//设置是否推送实时事件
	void SetConnect(bool bConnect){m_bConnect = bConnect;}

	//设置是否去阴影
	/*void SetRmShade(bool bRmShade){ m_bRmShade=bRmShade;m_bReloadConfig = true;}
	//设置是否去抖动
	void SetRmTingle(bool bRmTingle){ m_bRmTingle= bRmTingle;m_bReloadConfig = true;}
	//灵敏度设置
	void SetSensitive(bool bSensitive){ m_bSensitive= bSensitive;m_bReloadConfig = true;}
	//设置事件检测间隔
	void SetChannelEventDetectDelay (int nEventDetectDelay){ m_nSameEventIgr = nEventDetectDelay ;m_bReloadConfig = true;}
	//设置停留显示时间
	void SetShowTime(int nShowTime){ m_nShowTime = nShowTime;m_bReloadConfig = true;}*/
	//设置设备ID
	void SetDeviceID(string strDeviceId) { m_strDeviceId = strDeviceId; }

	//设置通道地点
	void SetPlace(std::string location){m_strLocation = location;}
	//设置通道方向
	void SetDirection(int nDirection){m_nDirection = nDirection;}
	//设置相机编号
	void SetCameraID(int nCameraID);
	//设置相机类型
    void SetCameraType(int  nCameraType);
    //设置检测类型
    void SetDetectKind(CHANNEL_DETECT_KIND nDetectKind);
    //设置Monitor编号
    void SetMonitorID(int nMonitorID);
    //设置录像类型
    void SetCaptureType(CAPTURE_TYPE eType) {m_eCapType = eType;}

    //获取是否有目标移动标志
    bool GetObjMovingState();

	//流量统计
	void VehicleStatistic(int uRoadIndex,DetectResultList::iterator& it_b,RECORD_STATISTIC& statistic);

	//流量评价统计
	void VtsStatistic(SRIP_DETECT_HEADER* buf);

	//设置H264采集类指针
	bool SetH264Capture(CRoadH264Capture *pH264Capture);

	void DetectRegionRect(int xPos,int yPos,int width,int height,bool FromClient=true);
	//发送停车严管增加或删除目标区域
	void DetectParkObjectsRect(UINT32 uDetectCarID,UINT32 uMsgCommandID,int xPos,int yPos,int width,int height,bool FromClient=true);
	void DealDetectRect(std::string& frame);
	void SetCameraHostAndWidthHeight(int iWidth,int iHeight, std::string strCameraHost,int nCameraPort = 80);
	//创建处理来自客户端的3D定位请求
	void CreateDealClientRectMsgThread();
	static void * DealClientRectMsgFunc(void * lpParam);
	void * DealClientRectMsg();
	bool InitAutoCtrlCfg();

	//输出的一帧数据位镜头拉近后的数据 //bSaveProcess 此字段表示是否保存此次PTZ过程
	int DealTrackAutoRect(std::string& response,int nCarId,int nSaveProcess=0);

	int DealTrackAutoRectDHCamera(std::string& response,int nCarId,int nSaveProcess=0);
	
	int DealTrackAutoRectBocomPTZ(std::string& response,int nCarId,int nSaveProcess=0);
	bool PopRealFrame(std::string& response);
	void GetProjectEventPicPath(RECORD_EVENT RecordEvent,std::string& strPicPath,int nRandCode=0,int nIndex=0);

	//天津项目定制,一次获取两张图片
	int DealTrackAutoRectTJ(std::string& response,std::string& response2,int nCarId);
	int DealTrackAutoRectDHCameraTJ(std::string& response,std::string& response2,int nCarId);
	int SavePicToLocal(std::string strPic,char *pSzFileName);
	//天津手动抓拍处理
	bool DetectHandEventTJ(std::string frame,int nCarId,CvRect rect);
	//天津项目单个图片加水印
	void SingleImageAddCharacterTJ(IplImage * pImage, RECORD_EVENT event, int key,SRIP_DETECT_HEADER header);
	//图片抓拍完毕判断车是否还在
	int CheckLastPic(int nCarId);
	//停止事件录像
	void StopEventRecord(SRIP_DETECT_HEADER sDetectHeader,int nCarId);
	//开始事件录像
	void StartEventRecord(SRIP_DETECT_HEADER sDetectHeader,int nCarId);
	//结束事件录像
	void EndEventRecord(SRIP_DETECT_HEADER sDetectHeader,int nCarId);
	int SaveEventRecordTJ(RECORD_EVENT &event,int nCarId);
private:
	//启动帧检测线程
	bool BeginDetectThread();

	//停止帧检测线程
	void EndDetectThread();

	//检测采集帧
	bool DetectFrame();

	//弹出一帧数据
	bool PopFrame(std::string& response);

	//检测一帧
	bool OnFrame(std::string& frame);

	bool OnHandFrame(std::string& frame);

	//初始化检测算法配置
	bool InitDetectConfig();
	//载入通道设置
    bool LoadRoadSettingInfo();

	//获取事件类型
	bool  GetEventType(DETECT_RESULT_TYPE& eType,int uType,int uRoadWayID);

	//保存图像
	int SaveImage(IplImage* pImg,std::string strPicPath,int nIndex = 0,int nRandCode1 = 0,int nRandCode2 = 0);

	//在事件图像上叠加文本信息
	void PutTextOnImage(IplImage* pImg,RECORD_EVENT event,SRIP_DETECT_HEADER* sPreHeader = NULL,int nIndex = 0);

	void PutTextOnComposeImage(IplImage* pImage,RECORD_EVENT event,SRIP_DETECT_HEADER* sAuxHeader = NULL);

    //保存车牌数据
    void SavePlate(SRIP_DETECT_OUT_RESULT result,SRIP_DETECT_HEADER sDetectHeader, const RECORD_EVENT& event, const TimePlate& tp);
	
	//获取图片路径
	int GetEventPicPath(RECORD_EVENT& event,std::string& strPicPath);
    //zhangyaoyao:保存事件记录到数据库
    //void mvSaveEvent(SRIP_DETECT_OUT_RESULT result, SRIP_DETECT_HEADER& sDetectHeader, RECORD_EVENT& event);
    //计算地面亮度
    void CalculateRoadContext();

    //添加秒图
    void AddResultFrameList(BYTE* frame,int nSize);

	//获取秒图
	SRIP_DETECT_HEADER GetImageFromResultFrameList(int64_t uTimeStamp,int  nTimeInterval, IplImage* image,UINT32 nPreSeq = 0);

    //获取区域中心点
    void GetCenterPoint(Point32fList ptList,CPoint32f& ptCenter);

    //获取违章位置
    void GetViolationPos(CvPoint& point,CvRect& rect,int nType = 0);

    //输出事件检测结果
    void OutPutResult(std::string& frame);

	//输出事件检测结果
	void OutPutAutoCtrlResult(std::string& frame);

	

	//从Jpg缓冲区查找出对应的RGB图片
	bool GetImageByJpgSeq(UINT32 nSeq,UINT32 nPreSeq,PLATEPOSITION* pTimeStamp,IplImage* pImage);

	//从Jpg缓冲区查找出对应的jpg图片
	bool GetJpgImageBySeq(UINT32 nSeq,PLATEPOSITION* pTimeStamp,string& strPic);

	//从jpg map中寻找一个最接近的图片
	void GetPicFromJpgMap(UINT32 nSeq,UINT32 nPreSeq,string& strPic);
	//处理车牌事件
    void OnProcessPlateEvent(string& frame, SRIP_DETECT_OUT_RESULT& sResult, RECORD_EVENT& event, SRIP_DETECT_HEADER &sHeader, UINT32 uRoadIndex, string& strPicPath, bool &flag);
	//保存车牌事件图片
	void SavePlateEventPic(UINT32 uTimeStamp,int  nTimeInterval,string& frame,long nEventId);
	//从磁盘中获取事件图片
   SRIP_DETECT_HEADER FindPicFromDisk(long dEventId, UINT32 & uTime, UINT32 & uMiTime,SRIP_DETECT_HEADER& sAuxHeader);
   //获取BMP图像
   bool GetBmpImage(const char *fileName, string& strPicData1, string& strPicData2, bool IsTwo=true);
   //在车牌事件合成图上写文字
   void PutTextOnPEComposeImage(IplImage* pImage,const RECORD_EVENT& event, UINT32 uTime, UINT32 uMiTime, string& strCarNum);
   //从内存中获取车牌图像
   bool FindPicFromMem(char *chPlate, IplImage *pImage, TimePlate *pTimePlate);

   //读取测试结果文件
	bool LoadTestResult(char* filename);

	//输出测试结果
	void OutPutTestResult(std::string& frame);

	//车牌识别
	string CarNumDetect(string& strFrame);
	string CarNumDetect(string& strFrame,CvRect *pRectAtDelect);

	//获取图像坐标到世界坐标的变换矩阵
	void mvfind_homography(float *image, float *world, float homography_image_to_world[3][3]);

	int GetTimeT();
	int GetPTZPresetStatus();
	//0表示没有移动，1 表示移动，2表示切换到了远景预置位
	int CalculatePTZPosition(int &nPreset); 

	bool DetectAutoEventStop(std::string frame,SRIP_DETECT_OUT_RESULT *pResult,RECORD_EVENT &event);
	bool DetectAutoCarEvent(std::string frame,bool bAppear,RECORD_EVENT &event);
	/*
	功能描述： 手动抓拍提示保存图片
	参数：	frame   帧数据
			nCarId  车辆ID 	
			rect    车辆坐标
			nPicNum 图片顺序 1 为第一次 2为第二次 3表示车辆离开
	*/
	bool DetectHandEvent(std::string frame,int nCarId,CvRect rect,int nPicNum);

	static void CatchCarEventCB(int nCarId, int nContext);

	void DealCatchCarEvent(int nCarId);
	
	#ifdef  HANDCATCHCAREVENT
	static void CatchCarEventDrawCB(vector<RectObject>  vectRectObj ,SRIP_DETECT_HEADER tDetectHeader, int nContext);

	void DealCatchCarEventDraw(vector<RectObject>  vectRectObj,SRIP_DETECT_HEADER tDetectHeader);
	#endif


	int SaveEventRecord(RECORD_EVENT &tRecordEvent,int nCarId,int nChannelId);

	bool AverageValue( IplImage *pGrayImg,CvRect rect );

	//设定打字颜色 
	void PrintColor(IplImage* pSmallPic,CvRect recSmallPic, IplImage* pBigPic);
//私有变量
public:
	//信号互斥
	pthread_mutex_t m_Frame_Mutex;
	//信号互斥
	pthread_mutex_t m_JpgFrameMutex;
	//信号互斥
	pthread_mutex_t m_preFrame_Mutex;//秒图
	//事件结果图片互斥
	pthread_mutex_t m_EventPic_Mutex;
	//线程ID
	pthread_t m_nThreadId;

	//待检测的帧列表
	ChannelFrame	m_ChannelFrameList;

	//待检测的帧列表长度
	int m_nFrameSize;

	//JPG大图队列
	map<int64_t,string> m_JpgFrameMap;

	//事件结果图片映射
	map<long,string> m_EventPicMap;

	//经过地点
	std::string m_strLocation;
	//行驶方向
	int m_nDirection;

	//相机型号
	int m_nCameraType;

	//车道编号映射
	RoadIndexMap m_roadMap;

    IplImage* m_img;//当前场图像
    IplImage* m_imgPre;//上一场图像
	IplImage* m_imgSnap;//当前帧图象
	IplImage* m_imgPreSnap;//上一帧图像
    IplImage* m_imgComposeSnap;//合成图像（2帧）
	//车牌事件中需存图
    IplImage* m_imgAuxSnap;//车牌事件时保存的第三张图
	IplImage* m_imgPreObjectSnap;//目录识别上一帧图像
	IplImage* m_imgComposeStopSnap;//停车严管结果图片


    //叠加区域高度
    int m_nExtent;

    //前一张图片与当前图片的时间间隔
    int m_nTimeInterval;

	//x方向缩放比
    double m_fScaleX;
	//y方向缩放比
	double m_fScaleY;

	//x缩放比
	double m_ratio_x;
	//y缩放比
	double m_ratio_y;

	//闯红灯检测参数列表
	VTSParaMap  m_vtsObjectParaMap;
	//控制灯状态列表
	map<int,EVENT_VTS_SIGNAL_STATUS> m_mapVTSStatus;
	//检测控制参数
	paraDetectList m_sParamIn;
	//检测数据
	IMG_FRAME	m_sImageFrame;

    //事件录象类
    CSkpRoadRecorder m_skpRoadRecorder;


	//检测类型
	CHANNEL_DETECT_KIND m_nDetectKind;

    //夜晚道路检测
    //	Mv_Light_VehicleDetect m_NightRoadDetect;
    //流量统计
    UINT32 uFluxAll[MAX_ROADWAY];       //总流量
    UINT32 uFluxPerson[MAX_ROADWAY];     //行人流量
    UINT32 uFluxNoneVehicle[MAX_ROADWAY];  //非机动车流量
    UINT32 uFluxVehicle[MAX_ROADWAY];   //机动车流量

	//图像文本信息
	CvxText m_cvText;

	//文本区域
	char* m_pExtentRegion;

	//场图像还是帧图像
	int m_nDeinterlace;

	 //秒图循环队列
    BYTE* m_chResultFrameBuffer[FRAMESIZE];
    //秒图循环队列首地址
    BYTE* m_pResultFrameBuffer;
    //秒图循环队列当前读位置
    int m_nResultReadIndex;
    //秒图循环队列当前写位置
    int m_nResultWriteIndex;
    //保存的每张BMP图片的大小
	int m_nBmpSize;

	float homography_image_to_world[3][3];
// 公有变量
public:

	//通道号
	int m_nChannelID;

	int m_nMonitorID; //监视器编号

	//相机编号
	int m_nCameraId;
	//设备ID
	string m_strDeviceId;

	//是否停止线程
	bool m_bTerminateThread;

	//存图数量
	int m_nSaveImageCount;
    //是否存小图
	int m_nSmallPic;
	//文字位置
	int m_nWordPos;
    //是否重读配置文件
	bool m_bReloadConfig;


	 //检测时间类别
	 CHANNEL_DETECT_TIME m_nDetectTime;

	 //是否事件录象
	 bool m_bEventCapture;
	  //是否推送事件
	 bool m_bConnect;

	//统计时间长度
	int m_nTrafficStatTime;
	//上次统计时间
	//UINT32 m_preTrafficStatTime;
	//事件录象长度
	int m_nCaptureTime;

	//存放上一次秒图存储时间
    int64_t   m_uPreTimestamp;

    //JPG图像
    BYTE* m_pJpgImage;

	 //白天还是晚上
	 int m_nDayNight;
	 //录像类型
	 CAPTURE_TYPE m_eCapType;

	 //检测的结果
	DetectResultList m_list_DetectOut;


	 PasserbyRoadColor m_passerbyColor; //颜色


    	//事件检测
#ifndef NOEVENT
	Mvroad_detect m_Rdetect;
	 //MvStopVehDetect m_Rdetect;
#endif
	
	//车牌检测
	Mv_CarnumDetect mvcarnumdetect;

	 //-------------------球机控制部分变量-----------------------------------
public:

#ifdef CAMERAAUTOCTRL
	 COnvifCameraCtrl  *m_pOnvifCameraCtrl;
	 CRoadDetectPelco  *m_pRoadDetectPelco;
	 CRoadDetectDHCamera  *m_pRoadDetectDHCamera;
	 CEventVisVideo*  m_EventVisVideo; 
#endif

#ifdef  HANDCATCHCAREVENT
	  CHandCatchEvent*  m_pHandCatchEvent; 
#endif

	 bool m_bDealClientRectRequest;
	 bool m_bDetectClientRect;
	 bool m_bCameraAutoCtrl; //当前球机是否在进行3D智能定位
	 bool m_bCameraMoveFormOp;

	 int m_iWidth;                //图像宽度
	 int m_iHeight;               //图像高度
	 std::string m_strCameraHost; //相机地址
	 int m_nCameraPort;           //相机端口
	 pthread_mutex_t m_Preset_Mutex;
	 #ifdef CAMERAAUTOCTRL
	 typedef map<int,AutoCameraPtzTrack*> mapAutoCameraPtzTrack;
	 mapAutoCameraPtzTrack m_CameraPtzPresetStatus; //复用此map 用于存储每个预置位的PTZ状态  x轴值 y轴值 zoom值
	#endif
	 typedef map<int,int> mapAutoCameraRecord;
	 mapAutoCameraRecord m_mapAutoCameraRecord;
	 mapAutoCameraRecord m_mapVideoTime; //录像时间
	 //MD5加密算法
	 MD5_CTX* m_pMd5Ctx; 

	 int m_dwEventTime;

	  pthread_mutex_t m_DetectCarParkRect_Mutex; //手动抓拍
	  typedef map<int,HandCatctEventInfo> mapDetectCarParkRect;
	  mapDetectCarParkRect m_mapDetectCarParkRect;

	 //四张图片的灰度值
	 double m_dFirstAvgVal;
	 double m_dSecondAvgVal;
	 double m_dThirdAvgVal;
	 double m_dFourAvgVal;

	 double *m_dPicsAvgVal;//图片灰度值数组
	 //当前预置位编号 默认为1
	 int m_nPresetNum; //当前预置位编号
	 int m_nGotoPresetTime;   //回归预置位时间
	
	 int  m_nVideoNum; //录像临时编号

     //预置位控制
    CPreSetControl m_PreSetControl;

    //是否存在近景预置位
    int m_nHasLocalPreSet;

	//检测方向
	int m_nDetectDirection;
	
	//目标出现上张图帧号
	UINT32 m_nPreObjectSeq;
	//目标出现上张图时间
	int64_t m_tsPreObject;

	std::list<RECORD_PLATE> m_listTestResult;
	bool m_bTestResult;
	int m_nFileID;

	//直行
	UINT32 m_uNonTrafficSignalTime; //非红灯时间
	int64_t m_uNonTrafficSignalBeginTime; //非红灯开始时刻
	int64_t m_uNonTrafficSignalEndTime; //非红灯结束时刻
	int64_t m_uPreNonTrafficSignalTime; //上次统计时刻
	int m_nConnectnumber;//采集序号

	//左转
	UINT32 m_uNonLeftTrafficSignalTime; //非红灯时间
	int64_t m_uNonLeftTrafficSignalBeginTime; //非红灯开始时刻
	int64_t m_uNonLeftTrafficSignalEndTime; //非红灯结束时刻
	int64_t m_uPreNonLeftTrafficSignalTime; //上次统计时刻
	int m_nLeftConnectnumber;//采集序号

	CRoadH264Capture *m_pH264Capture;

	map<int,list<unsigned char*> > m_mapRemotePicList;//远景图片列表映射

	REGION_ROAD_CODE_INFO m_picFormatInfo;//通道图片格式

	int m_nRemotePicCount;//远景图片数量

	#ifdef DETECT_VEHICLE
	MvDetector m_MvDetector;
	#endif
};
#endif
#endif
