// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef FACE_DETECT_H
#define FACE_DETECT_H

#define NOFACEDETECT

#ifndef NOFACEDETECT

#include "global.h"

#include "RoadRecorder.h"
#include "CvxText.h"

#include "MvFaceDetection.h"
#include "MvFaceTrackSearch.h"

/******************************************************************************/
//	描述:人脸识别实体类.
//	作者:於锋
//	日期:2011-7-14
/******************************************************************************/

//帧数据列表
typedef std::list<std::string> ListFrame;

//人脸特征
typedef struct stFaceFeatureResult
{
	int64_t		 uTime64;			//时间戳
	IplImage*	pRepImage;			//每个轨迹一张全图
	CvRect		repImageRect;			//轨迹人脸在全图中的位置
	mv_stFaceFeature stFaceFeature;

} mv_stFaceFeatureResult;


class CFaceDetect
{
public:
	//构造
	CFaceDetect();
	//析构
	~CFaceDetect();

public:
	//初始化检测数据，算法配置文件初始化
	bool Init(int nChannelID,int widthOrg,int heightOrg,int widthDst ,int heightDst);
	//释放检测数据，算法数据释放
	bool UnInit();

//处理
    //事件检测处理
    void DealFrame();

	//添加一帧数据
	bool AddFrame(std::string& frame);

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
	inline void SetReloadConfig()
	{ 
		m_bReloadConfig = true;
	    m_bReloadConfigFeature = true;
	}

	//设置是否推送实时事件
	void SetConnect(bool bConnect){m_bConnect = bConnect;}

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

	bool DealFaceResult();
private:
	//启动帧检测线程
	bool BeginDetectThread();

	//停止帧检测线程
	void EndDetectThread();

	//检测采集帧
	bool DetectFrame();

	//弹出一帧数据
	std::string PopFrame();

	//检测一帧
	bool OnFrame(std::string& frame);

	//载入通道设置
    bool LoadRoadSettingInfo(vector<CvPoint>& vImagePoint, vector<CvPoint3D32f>& vWorldPoint,vector<CvPoint>& PoinStore,CvRect& farrect,CvRect& nearrect);


	//保存图像
	int SaveImage(IplImage* pImg,std::string strPicPath,int nIndex = 0);

	//在事件图像上叠加文本信息
	void PutTextOnImage(IplImage* pImg,RECORD_EVENT event,SRIP_DETECT_HEADER* sPreHeader = NULL,int nIndex = 0);

     //查找5秒前的图片数据
    SRIP_DETECT_HEADER GetPreImage(UINT32 uTimeStamp,DETECT_RESULT_TYPE  dType);

    //获取区域中心点
    void GetCenterPoint(Point32fList ptList,CPoint32f& ptCenter);

	//人脸特征提取
	bool DealFeatureExtract(mv_stFaceTrack& stFaceTrackResult);

	//添加人脸检测结果数据
	bool AddFaceResult(vector<mv_stFaceTrack>& vFaceTracks);

	//弹出人脸检测结果数据
	bool PopFaceResult(mv_stFaceTrack& stFaceTrackResult);
	
	//输出人脸特征结果
	void OutPutFaceFeature(mv_stFaceFeatureResult& faceFeatures);

	//删除人脸检测结果数据
	bool DeleteFaceResult();

//私有变量
public:
	//信号互斥
	pthread_mutex_t m_Frame_Mutex;

	//线程ID
	pthread_t m_nThreadId;

	//待检测的帧列表
	ListFrame	m_ChannelFrameList;
	//待检测的帧列表长度
	int m_nFrameSize;
	
	//信号互斥
	pthread_mutex_t m_FaceResult_Mutex;
	//人脸特诊提取线程
	pthread_t m_nFeatureThreadId;


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

    IplImage *m_pImage;//当前处理图像
	IplImage *m_pImageFrame;//

    //叠加区域高度
    int m_nExtent;

    //前一张图片与当前图片的时间间隔
    int m_nTimeInterval;

	//x方向缩放比
    double m_fScaleX;
	//y方向缩放比
	double m_fScaleY;

    //事件录象类
    CSkpRoadRecorder m_skpRoadRecorder;


	//检测类型
	CHANNEL_DETECT_KIND m_nDetectKind;

    //流量统计
    UINT32 uFluxAll[MAX_ROADWAY];       //总流量
    UINT32 uFluxPerson[MAX_ROADWAY];     //行人流量
    UINT32 uFluxNoneVehicle[MAX_ROADWAY];  //非机动车流量
    UINT32 uFluxVehicle[MAX_ROADWAY];   //机动车流量

	//图像文本信息
	CvxText m_cvText;

	//文本区域
	char* m_pExtentRegion;
	//文字位置
	int m_nWordPos;

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



// 公有变量
public:

	//通道号
	int m_nChannelID;

	int m_nMonitorID; //监视器编号

	//相机编号
	int m_nCameraId;

	//是否停止线程
	bool m_bTerminateThread;


	//是否重新初始化人脸检测线程
	bool m_bReloadConfig;

	//是否重新初始化人脸特征提取线程
	bool m_bReloadConfigFeature;


	 //检测时间类别
	 CHANNEL_DETECT_TIME m_nDetectTime;

	 //是否事件录象
	 bool m_bEventCapture;
	  //是否推送事件
	 bool m_bConnect;

	//统计时间长度
	int m_nTrafficStatTime;

	//事件录象长度
	int m_nCaptureTime;

	 //白天还是晚上
	 int m_nDayNight;
	 //录像类型
	 CAPTURE_TYPE m_eCapType;

	 //JPG图像
    BYTE* m_pJpgImage;

	//人脸检测区域
	CvRect m_vDetectRoi;

	//人脸检测区域
	vector<CvPoint> m_DetectPointList;
	
	//人脸检测
	MvFaceDetection m_FaceDetection;
	//人脸搜索
	MvFaceTrackSearch m_FaceTrackSearch;
	
	//人脸检测结果
	vector<mv_stFaceTrack> m_vFaceTracks;
	
};
#endif

#endif


