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

#ifndef SKP_ROAD_CHANNEL_H
#define SKP_ROAD_CHANNEL_H


#include "global.h"

#include "RoadVideoV4l2.h"
#include "Factory.h"
#include "FeatureSearch.h"
#include "RoadDetect.h"
#include "RoadCarnumDetect.h"
#include "ObjectBehaviorDetect.h"
#include "VideoStabilizer4Client.h"
#include "FaceDetect.h"

#define REBOOT_DETECT

#ifndef ALGORITHM_YUV
#define VIDEOSHUTTER
#endif

#ifdef VIDEOSHUTTER
//#include "locate.h"
#include "CarDetect.h"
#endif

#include "necommhead.h"
#include "NESocket.h"
#ifndef ALGORITHM_YUV
#include "CameraControl.h"
#endif
#include "RoadH264Capture.h"
#include "RoadMonitorH264.h"
#include "RoadTempCapture.h"

//#include "DspDataProcess.h"
/******************************************************************************/
//	描述:智能交通检测系统通道对象
//	作者:徐永丰
//	日期:2008-4-22
/******************************************************************************/

class CSkpChannelEntity
{
public:
	//构造
	CSkpChannelEntity();
	//析构
	~CSkpChannelEntity();

public:
	//打开设备
	bool Open(int nNo = 0);
	//关闭设备
	bool Close();
	//设备是否成功打开
	bool IsOpen(){return m_bOpen;}

	//获取相机状态
	int GetCameraStatus(){return m_sChannel.nCameraStatus;}

	//设置获取通道状态
	void SetStatus(int nStatus){m_nStatus = nStatus;}
	int GetStatus(){return m_nStatus;}

	//连接断开通道
	void SetConnect(bool bConnect);
	bool GetConnect(){return m_bConnect;}

	//开始检测
	bool BeginDetect();
	//停止检测
	bool EndDetect();

	//重启通道
	bool Restart(SRIP_CHANNEL& sChannel);

	//重启录象
	bool RestartCapture(CAPTURE_TYPE cType);

	//重启检测
	bool RestartDetect(CHANNEL_DETECT_KIND dType);

	//返回通道ID
	int GetChannelID() { return m_sChannel.uId;}
	//设置通道ID
	void SetChannelID(const int nID) { m_sChannel.uId = nID;}

	//返回通道类型
	CHANNEL_TYPE GetChannelType() { return m_sChannel.eType;}
	//设置通道类型
	void SetChannelType(const CHANNEL_TYPE cType){m_sChannel.eType = cType;}

	//返回视频源
	VEDIO_FORMAT GetChannelFormat() { return m_sChannel.eVideoFmt;}
	//设置视频源
	void SetChannelFormat(const VEDIO_FORMAT vFormat) { m_sChannel.eVideoFmt = vFormat;}

	//设置通道地点
	void SetChannelLocation(std::string location);
	std::string GetChannelLocation()
	{
		return m_strLocation;
	}
	//设置通道方向
	void SetChannelDirection(int nDirection);
	int GetChannelDirection(){return m_sChannel.uDirection;}

	//返回事件录像时间
	int GetChannelEventCaptureTime() { return m_sChannel.uEventCaptureTime;}
	//设置事件录像时间
	void SetChannelEventCaptureTime(const int nEventCaptureTime);

	//设置历史视频起止时间
	void SetVideoBeginTime(const UINT32 uVideoBeginTime) {m_sChannel.uVideoBeginTime = uVideoBeginTime;}
	void SetVideoEndTime(const UINT32 uVideoEndTime) {m_sChannel.uVideoEndTime = uVideoEndTime;}

	//获取历史视频起止时间
    UINT32 GetVideoBeginTime() {return m_sChannel.uVideoBeginTime;}
    UINT32 GetVideoEndTime() {return m_sChannel.uVideoEndTime;}

	//视频参数
	//返回视频参数-亮度
	int GetChannelBrightness() { return 0;/*m_sChannel.sAttr.uBrightness;*/}
	//设置视频参数-亮度
	void SetChannelBrightness(int uBrightness) { /*m_sChannel.sAttr.uBrightness = uBrightness;*/}

	//返回视频参数-对比度
	int GetChannelContrast() { return 0;/*m_sChannel.sAttr.uContrast;*/}
	//设置视频参数-对比度
	void SetChannelContrast(int uContrast) { /*m_sChannel.sAttr.uContrast = uContrast;*/}

	//返回视频参数-饱和度
	int GetChannelSaturation() { return 0;/*m_sChannel.sAttr.uSaturation;*/}
	//设置视频参数-饱和度
	void SetChannelSaturation(int uSaturation) { /*m_sChannel.sAttr.uSaturation = uSaturation;*/}

	//返回视频参数-色调
	int GetChannelHue() { return 0;/*m_sChannel.sAttr.uHue;*/}
	//设置视频参数-色调
	void SetChannelHue(int uHue) { /*m_sChannel.sAttr.uHue = uHue;*/}

	//返回通道检测类型
	CHANNEL_DETECT_KIND GetChannelDetectKind() { return m_sChannel.uDetectKind;}
	//设置通道检测类型
	void SetChannelDetectKind(CHANNEL_DETECT_KIND dType);

	//设置白天还是晚上检测
	void SetChannelDetectTime(CHANNEL_DETECT_TIME dType);

	//是否事件录象
	void SetEventCapture(int bEventCapture);
	//修改事件录像
	void ModifyEventCapture(int bEventCapture);

	//yuv格式
	void SetYUVFormat(int nYUVFormat){ m_sChannel.nYUVFormat = nYUVFormat;}
	//获取yuv格式
	int GetYUVFormat() {return m_sChannel.nYUVFormat;}


	//相机型号设置
	void SetCameraType(int nCameraType);
	int GetCameraType(){ return m_sChannel.nCameraType; }
	//相机编号设置
	void SetCameraID(int nCameraID);
	int GetCameraID(){ return m_sChannel.nCameraId; }

	//设备ID设置
	void SetDeviceID(std::string strDeviceId);
	//返回设备ID
	std::string GetDeviceID()
	{
		std::string strDeviceId(m_sChannel.chDeviceID);
		return strDeviceId;
	}


	//断面编号设置
	void SetPannelID(int nPannelID){ m_sChannel.nPannelID = nPannelID;}
	int GetPannelID(){ return m_sChannel.nPannelID;}

	//设置监视器编号
    void SetMonitorID(int nMonitorID);
	int GetMonitorID(){ return m_sChannel.nMonitorId;}

	//设置视频编号
    void SetVideoID(int nVideoID){ m_sChannel.nVideoIndex = nVideoID;}
	int GetVideoID(){ return m_sChannel.nVideoIndex;}

	//返回录像类型
	CAPTURE_TYPE GetChannelCaptureType() { return m_sChannel.eCapType;}
	//设置录像类型
	void SetChannelCaptureType(CAPTURE_TYPE eType);

	//返回录像开始时间
	UINT32 GetChannelCapBeginTime() { return m_sChannel.uCapBeginTime;}
	//设置录像开始时间
	void SetChannelCapBeginTime(UINT32 uTime) { m_sChannel.uCapBeginTime = uTime;}

	//返回录像结束时间
	UINT32 GetChannelCapEndTime() { return m_sChannel.uCapEndTime;}
	//设置录像结束时间
	void SetChannelCapEndTime(UINT32 uTime) { m_sChannel.uCapEndTime = uTime;}

	//YUV流连接参数
	//返回连接主机
	std::string GetChannelYuvHost()
	{
		std::string strMonitorHost(m_sChannel.chMonitorHost);
		return strMonitorHost;
	}
	//设置连接主机
	void SetChannelYuvHost(std::string strMonitorHost)
	{
		memcpy(m_sChannel.chMonitorHost,strMonitorHost.c_str(),strMonitorHost.size());
	}

	//返回连接端口
	UINT32 GetChannelYuvPort() { return m_sChannel.uMonitorPort;}
	//设置连接端口
	void SetChannelYuvPort(UINT32 uMonitorPort) { m_sChannel.uMonitorPort = uMonitorPort;}

	////////////////////////////////////////
	//设置相邻同步主机
	void SetChannelSynHost(std::string strHost)
	{
		memcpy(m_sChannel.chSynHost,strHost.c_str(),strHost.size());
	}
	//返回相邻同步主机
	std::string GetChannelSynHost()
	{
		std::string strSynHost(m_sChannel.chSynHost);
		return strSynHost;
	}


	//设置相邻同步主机连接端口
	void SetChannelSynPort(UINT32 uPort) { m_sChannel.uSynPort = uPort;}
	//返回相邻同步主机连接端口
	UINT32 GetChannelSynPort() { return m_sChannel.uSynPort;}

	//设置相机IP
	void SetCameraHost(std::string strCameraHost);

	//获取相机IP
	std::string GetCameraHost()
	{
	    std:string strHost(m_sChannel.chCameraHost);
        return strHost;
    }
	///////////////////////////////////////////
	//设置通道工作方式
	void SetChannelWorkMode(const int nWorkMode){m_sChannel.nWorkMode = nWorkMode;}
	//获取通道工作方式
	int GetChannelWorkMode(){return m_sChannel.nWorkMode;}

		//返回文件名称
	std::string GetChannelFileName()
	{
		std::string strFileName(m_sChannel.chFileName);
		return strFileName;
	}
	//设置文件名称
	void SetChannelFileName(std::string strFileName)
	{
	    memset(m_sChannel.chFileName,0,sizeof(m_sChannel.chFileName));
		memcpy(m_sChannel.chFileName,strFileName.c_str(),strFileName.size());
		printf("strFileName=%s,m_sChannel.chFileName=%s\r\n",strFileName.c_str(),m_sChannel.chFileName);
	}


	//设置视频参数
	bool SetChannelParams(SRIP_CHANNEL_ATTR& sAttr);

	//调整镜头
	//bool SerialControl( SERIAL_CONTROL_MESSAGE nKind);

	//重置配置
	void SetReloadConfig(bool bReloadParam = true);

	//帧数据处理
	void DealFrame();

	//截取图片
	void CaptureOneFrame(std::string& result,ImageRegion imgRegion);

	//相机控制
	void CameraControl(CAMERA_CONFIG& cfg);

	//相机自动控制
    void CameraAutoControl(UINT32 uTimeStamp);

	//发送区域图像
    void SendRegionImage(ImageRegion imgRegion);

    //获取预置位
    int GetPreSet(){return m_sChannel.nPreSet;}

    //获取通道情况
    void GetChannelInfo(CHANNEL_INFO_RECORD& chan_info);

    //定时选择预置位
    void GotoPreSet(int64_t uSwitch);

    //获取通道图像分辨率
    void GetImageSize(int& nWidth,int& nheight);

	//清空输出JpgMap列表
	void ClearJpgMap();

	//重连dsp相机
	bool ReOpenDsp();

	bool AddForceAlert(FORCEALERT *pAlert);


	//客户端发送过来的停车框选区域
	void DetectRegionRectImage(ImageRegion imgRegion);
	//发送停车严管增加或删除目标区域
	void DetectParkObjectsRect(UINT32 uMsgCommandID,RectObject &ObjectRect);

	int AddRecordEvent(int nChannel,std::string result);

	//录像初始化
	bool InitRecordCapture();

	//缓存录像初始化
	bool InitTempCapture();

	//把有图未输出的数据全部输出
	bool OutPutResultAll();

	//相机控制
	void CameraControl();
	
#ifdef REDADJUST
//红绿灯增强
	bool RedLightAdjust(IplImage *pImage);
#endif

	//通过uImgKey，核查记录状态
	bool CheckImgKeyState(const UINT64 &uKey);
	//更新通道记录标记
	bool UpdateImgListByKey(const UINT64 &uKey, const int &bState);

//私有函数
private:

	//模拟信号设备打开
	bool PalOpen(int nNo);
	//H264设备打开
	bool H264Open();
	bool H264FileOpen();
	//YUV流打开
	bool YuvOpen();
	//YUV文件流打开
	bool YuvFileOpen();
	bool MJpegOpen();
	//AVI文件流打开
	bool AviOpen();

	//模拟信号设备关闭
	bool PalClose();
	//H264设备关闭
	bool H264Close();
	//YUV流关闭
	bool YuvClose();
	bool MJpegClose();
	bool PicLibClose();

	//模拟信号源采集启动
	bool PalBegin();
	//H264源采集启动
	bool H264Begin();
	//YUV流采集启动
	bool YuvBegin();
	bool MJpegBegin();
	//图库识别启动
	bool PicLibBegin();

	//模拟信号源采集停止
	bool PalEnd();
	//H264源采集停止
	bool H264End();
	//YUV流采集停止
	bool YuvEnd();
	bool MJpegEnd();
	//图库识别停止
	bool PicLibEnd();


	//启动采集线程
	bool BeginFrameThread();
	//停止采集线程
	void EndFrameThread();

	//模拟信号源帧采集
	void PalDealFrame();
	//H264采集
	void H264DealFrame();
	//YUV流采集
	void YuvDealFrame();
    //MJPEG流采集
	void MJpegDealFrame();
	//Avi文件流采集
	void AviDealFrame();
    //图库识别
    void DealPicLib();
	
	bool PicLibOpen();

	//处理普通采集图像
	bool DealNormalPic(bool bDealPic,int nVideoFrameRate,bool bDealVideo = true);
	//处理检测采集图片
	bool DealDetectPic(int nVideoFrameRate);

	//处理普通采集JPEG图像
	bool DealNormalMJpgPic(bool bDealPic,int nVideoFrameRate,bool bDealVideo);

	//定时录像处理
	bool DealTimeCapture(int srcstep);

    //发送相机状态
    void SendCameraState(CameraState state);
    //初始化视频抖动检测数据
    void InitShakeDetect();
    //释放视频抖动检测数据
    void UnInitShakeDetect();
    //检测视频是否抖动
    void CheckShakedFrame(UINT32 uTimeStamp);

    //yuv->rgb
    void ConvertYUVtoRGB(BYTE* pSrc,BYTE* pDest);

    //bayer->rgb
    void ConvertBAYERtoRGB(BYTE* pSrc,BYTE* pDest);
    //ResizeImage
    void ResizeBigImagetoSmall(BYTE* pSrc,BYTE* pDest,bool bHalfSize = false);
    //计算地面亮度
    void CalculateRoadContext();

    //获取检测类型
    void GetDetectParam(UINT32& DetectorType);

    //自动爆闪灯控制
    void AutoFlashControl(yuv_video_buf*);

    //分配内存
    bool AlignMemory();

	//叠加文本信息
	void PutTextOnImage(unsigned char* pBuffer);
	//叠加LOGO信息
	void PutLogoOnImage(IplImage* pImage);
    //解码jpg->rgb
    bool DecodeJpgToRgb(BYTE *pBuf, const int nWidth, const int nHeight, int &srcstep, BYTE *pBufOut);
	
#ifdef DEBUG_PLATE_TEST
	void TestAddPlate(const yuv_video_buf & header);//添加模拟测试车牌
#endif

//私有变量
private:
    //暴闪灯用
    IplImage* flash_imgCotext;
    #ifdef VIDEOSHUTTER
    mvvideoshutter m_shut;
	vector<skip_video_out_result> m_vShutResult;
    #endif
    //通道数据
	SRIP_CHANNEL m_sChannel;

    //视频采集计时器，单位为毫秒(ms), 用于判断视频源是否断开
    //默认计时器时间超过5s时，认为视频源断开
    UINT32 m_uTimer;

    //上次视频格式
    VEDIO_FORMAT m_Video_Format;

	//视频宽
	UINT32 m_uWidth;
	//视频高
	UINT32 m_uHeight;

	//RGB大图数据
	BYTE* m_chImageData;
	//RGB小图数据
	BYTE* m_smImageData;
	//RGB录象数据
	BYTE* m_captureImageData;

	//JPG图像数据1
    BYTE* m_JpgImageData;
    //JPG图像数据2
    BYTE* m_JpgImageData2;

    //车牌相关信息
    BYTE* m_chPlateData;

	//检测头
	SRIP_DETECT_HEADER m_sDetectHeader;

	//线程ID
	pthread_t m_nThreadId;

	//线程ID
	pthread_t m_nCameraControlThreadId;

	//采集类,根据视频源调用对应的驱动采集
	//v4l2 处理类-PAL
	CSkpRoadV4l2 m_v4lDriver;
	//Monitor 处理类-H264
	CRoadMonitorH264 m_MonitorH264;

    //相机工厂类
	Factory* m_pCameraFactory;
	//抽象相机类指针（用于接收zebra相机数据）
	AbstractCamera* m_pAbstractCamera;

	//线程结束标志
	bool m_bEndDeal;

	//事件检测类
	#ifndef NOEVENT
	CSkpRoadDetect m_skpRoadDetect;
	#endif

	//车牌、目标、闯红灯检测类
	#ifndef NOPLATE
    CRoadCarnumDetect m_RoadCarnumDetect;
    #endif

    //行为分析检测类
    #ifndef NOOBJECTDETECT
    CObjectBehaviorDetect m_ObjectBehaviorDetect;
    #endif

    //特征提取
    #ifndef NOFEATURESEARCH
    CFeatureSearchDetect m_FeatureSearchDetect;
    #endif

	//人脸识别类
    #ifndef NOFACEDETECT
    CFaceDetect m_FaceDetect;
    #endif

    //jpg->avi
	CJpgToAvi	m_JpgToAvi;

	 //h264录象
    CRoadH264Capture m_H264Capture;

	//录像缓存类
	CRoadTempCapture m_TempCapture;

	//录像开始时间
    int64_t m_uBeginTime;
	//录像结束时间
	int64_t m_uEndTime;

	//通道是否打开
	bool m_bOpen;
	//通道状态
	int m_nStatus;
	//通道是否连接
	bool m_bConnect;

	//发送图像区域类型
	ImageRegion m_imageRegion;

	 //裁剪区域坐标
	RegionCoordinate m_cropOrd;

    //上次车牌个数统计时间
	UINT32 m_detect_time;

    //*************************视频抖动检测
    CVideoStabilizer4Client m_CStab; //图像震动检测类
	IplImage* m_imgCurr;//当前帧图像
	int m_nMiss;        //匹配丢失次数
	time_t m_tmLastCheck;//上次震动检测时间
	int m_nStabPolyNum;
	int* m_nStabPolySize;
    CvPoint2D32f** m_fStabPolyPts;

	 #ifndef NOPLATE
	#ifndef ALGORITHM_YUV
	 CameraParametter m_CameraCrt; //相机控制
	#endif
	 #endif
	 //区域图像（用于计算地面亮度）
	 IplImage* m_imgCotext;
	 //白天晚上判断标志
	 int m_nDayNight;

	 //视频模式（0：实时，1：历史）
	 int m_nVideoType;

	 #ifdef DSPDATAPROCESS
	 //CDspDataProcess m_DspDataProcess;
	 #endif

	 //是否需要重新载入检测参数
	 bool m_bReloadConfig;

	 //经过地点
	 std::string m_strLocation;

#ifdef REBOOT_DETECT
	 int m_nRebootFlag;//重启标记
#endif
};

#endif
