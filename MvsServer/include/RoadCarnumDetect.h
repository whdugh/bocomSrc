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
#ifndef SKP_ROAD_CARNUM_H
#define SKP_ROAD_CARNUM_H

#include "global.h"

#include "RoadRecordTemp.h"

#ifndef ALGORITHM_YUV
#ifndef ALGORITHM_DL
#include "CarnumParameter.h"
#else
#include "Mv_CarnumDetect.h"
#endif
#include "MvVehicleClassify.h"
#include "MvFindTargetByVirtualLoop.h"
#endif
#include "CvxText.h"

#ifndef ALGORITHM_YUV
#include "LabelRecognition.h"
#include "ColorRecognition.h"
#include "CameraControl.h"
#ifndef ALGORITHM_DL
#include "Mv_CarnumDetect.h"
#endif
#endif
#include "RoadRecorder.h"
#include "AbstractDetect.h"
#ifdef OBJECTFEATURESEARCH
#include "CommonSearch.h" 
#include "Cpeoplesearch.h"
#endif
#include "RoadH264Capture.h"
//#include "RecognitionTwice.h"
#include "mvdspapi.h"
#include "DspDataProcess.h"
//#define DETECTSHIELD

#ifdef DETECTSHIELD
#include "MvFrontWindPos.h"
#endif

#ifdef OBJECTHEADDETECT
#include "FaceDetection.h"
#endif

#ifdef OBJECTFACEDETECT
#include "MvFaceDet.h"
#include "MvFaceTrackSearch.h"
#endif

#ifdef REDADJUST
#include "mvRedAdjust.h"
#endif

#ifdef BELTDETECT
#include "MvSafetyBeltClassify.h"
#endif

#ifdef ALGORITHM_YUV_CARNUM
#include "MVPLR_Api.h"
#endif

#ifdef ALGORITHM_YUV
#include "MvDspAPI.h"
#include "MvDspMaxSize.h"

extern "C" MvDspGlobalSetting g_Setting;
extern "C" MvDspGlobalPara g_Para;
extern "C" MvDspOutPut g_Output;

typedef std::list<CvPoint> MvPointList;
typedef std::map<int64, MvPointList> ObjSyncData;//描述物体某一时间所处位置的点群

typedef struct _MVVIDEOSTD {	
	vector<CvPoint2D32f> vPList;	
	_MVVIDEOSTD()
	{		
		vPList.clear();
	}
}mvvideostd;

typedef struct ROAD_CONTEXT
{
	double     mean;               //区域平均亮度

	double     stddev;				//区域方差

	double  carnummean;             //车牌平均亮度

	double  carnumstddev;           //车牌方差

	CvRect    smearrect[20];         //光柱所在位置

	int       smearnum;				//光柱的数目
	
}road_context;

// 统计数据类型
enum STAT_TYPE
{
	STAT_FLUX,	       //车道流量
	STAT_SPEED_AVG,	   //平均车速
	STAT_ZYL,		   //平均车道占有率
	STAT_QUEUE,		   //队列长度
	STAT_CTJJ 	       //平均车头间距
};

//统计结果结构
typedef struct _STAT_RESULT
{
	int nChannelIndex;          //车道序号
	STAT_TYPE sRtype;		    //结果类型
	double value;				//统计值（如果没有则用负数表示）
	_STAT_RESULT ()
	{
		nChannelIndex = 0;
		value = -1;
	}
}STAT_RESULT;


//统计结果链表
typedef std::list< STAT_RESULT> StatResultList;

typedef struct	_MVVIDEOSHUTTERPAR{
	int64 ts;   //时间戳
	bool  bFlagShutter;//是否触发爆闪灯
	int   nRoadIndex;//所在的车道号
    bool  bHavReprted;
	_MVVIDEOSHUTTERPAR()
	{
		ts = -1;
		bFlagShutter = false;
		nRoadIndex = -1;
		bHavReprted = false;
	}
}mvvideoshutterpar;

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

//车道控制灯状态链表
typedef std::list<MvChannelFrameSignalStatus> ChnFrmSignalStatusList;

// 无论前牌尾牌，远的线圈为编号为0，近的线圈编号为1
typedef struct _LoopInfo
{
	int nRoadIndex;        // 车道编号
	MvLine line0;          // 编号为0的线圈在图像中的位置
	MvLine line1;          // 编号为1的线圈在图像中的位置。
	float  dis;            // 线圈距离（m）
}LoopInfo;

#ifndef REALLOOP
#define REALLOOP
typedef struct LOOP_PARMARER
{
	CvPoint pStart_point;
	CvPoint pEnd_point;
	int     iNvise_light;
	LOOP_PARMARER()
	{
		pStart_point = cvPoint( 0, 0 );
		pEnd_point = cvPoint( 0, 0 );
		iNvise_light = -1;
	}

}loop_parmarer;
#endif
#endif

#ifdef REDADJUST
//每个车道对应信号灯方向[左,中,右,转弯]
typedef struct _RED_DIRECTION
{
	UINT32 uRoadWayId;
	bool bLeft;
	bool bStraight;
	bool bRight;
	bool bTurnAround;

	_RED_DIRECTION()
	{
		uRoadWayId = 0;
		bLeft = 0;
		bStraight = 0;
		bRight = 0;
		bTurnAround = 0;
	}
}RED_DIRECTION;
#endif

typedef struct _Vts_Plate_Cache
{
	string strPlate;
	UINT32 uViolationType;

	_Vts_Plate_Cache()
	{
		uViolationType = 0;
	}
}sVtsPlateCache;
#define MAX_RED_LIGHT_NUM 10
/******************************************************************************/
//	描述:车牌检测类
//	作者:於锋
//	日期:2008-9-8
/******************************************************************************/

#ifndef NOPLATE
typedef std::map<int,int> SeqIndexMap; //seq<->index映射

typedef std::map<int,RECORD_PLATE> CarInfoMap;    //车辆信息映射
typedef std::map<int,ObjSyncData> SyncDataMap; //同步标记信息映射
typedef std::map<int,std::string> PicDataMap; //图片映射

//寻找图片的关键字
typedef struct _Vts_Picture_Key
{
	int	nId;//id号
	UINT32	uCameraId; //相机ID
	UINT64	uSeq;//帧号
	_Vts_Picture_Key()
	{
		nId = 0;
		uCameraId = 0;
		uSeq = 0;
	}
	bool operator < (const _Vts_Picture_Key& other) const
	{
		if (nId < other.nId)        //id按升序排序
		{
			return true;
		}
		else if (nId == other.nId)  //如果id相同，按比帧号升序排序
		{
			return uSeq < other.uSeq;
		}
		else if (uSeq == other.uSeq)  //如果帧号相同，按比相机ID升序排序
		{
			return uCameraId < other.uCameraId;
		}

		return false;
	}
}Vts_Picture_Key;

typedef struct _VTS_PIC_INDEX
{
	int iFlag; //是否可用,0: 未使用　1:已使用
	int i;//存储图片数组，下标

	_VTS_PIC_INDEX()
	{
		iFlag = 0;
		i = 0;
	}
}VTS_PIC_INDEX;
//typedef std::multimap<Vts_Picture_Key, string> VtsPicDataMultiMap; //违章缓存图片映射
//typedef std::map<Vts_Picture_Key, std::string> VtsPicDataMultiMap; //违章缓存图片映射
typedef std::map<Vts_Picture_Key, VTS_PIC_INDEX> VtsPicDataIndexMap; //违章缓存图片下标映射
typedef std::map<Vts_Picture_Key, std::string> VtsPicDataMap; //违章缓存图片映射

#define MAX_VTS_BUF_SIZE 15

//上次输出图片信息
typedef struct _OUTPUT_PIC_INFO
{
    int64_t ts; //上次输出图片时间（单位:微秒）
    UINT32 nSeq;//帧号
    int nPosX;
    int nPosY;
    int nImageSize[3]; //JPG图像大小(第1，2张大图加小图)
    string strPicPath;
    _OUTPUT_PIC_INFO()
    {
        ts = 0;
        nSeq = 0;
        nPosX = 0;
        nPosY = 0;
        memset(nImageSize,0,3*sizeof(int));
        strPicPath = "";
    }
}OUTPUT_PIC_INFO;

//#ifdef MATCH_LIU_YANG
	#define MAX_IMG_TAG_NUM 100 //Math比对缓存大小
//#else
//	#define MAX_IMG_TAG_NUM 20 //Math比对缓存大小
//#endif

#define MAX_JPG_BUF_SIZE 100 //jpg图片缓存大小

//车牌记录结构
typedef struct _DSP_CARINFO_ELEM
{
	UINT32 uTimeRecv;
	CarInfo cardNum;
	
	_DSP_CARINFO_ELEM()
	{
		uTimeRecv = 0;
	}
}DSP_CARINFO_ELEM;

#ifdef GLOBALCARLABEL
extern CarLabel  g_carLabel;
extern pthread_mutex_t g_carLabelMutex;
#endif

#ifdef GLOBALCARCOLOR
extern ColorRecognisize  g_carColor;
extern pthread_mutex_t g_carColorMutex;
#endif

#ifdef GLOBALCARCLASSIFY
extern MvVehicleClassify g_vehicleClassify;
extern pthread_mutex_t g_vehicleClassifyMutex;
#endif

#ifdef PLATEDETECT
#include "MvPlateDetector.h"
#endif
class CRoadCarnumDetect
{
public:
	//构造
	CRoadCarnumDetect();
	//析构
	~CRoadCarnumDetect();

public:

	//初始化
	bool Init(int nChannelId,UINT32 uWidth,UINT32 uHeight,int nWidth, int nHeight);

	//启动车牌检测线程
    bool BeginCarnumDetect();
	//结束车牌检测线程
	bool EndCarnumDetect();
	//车牌检测处理
	void DealCarnumDetect();
	//目标检测处理
	void DealObjectDetect();

	bool AverageValue(IplImage *pGrayImg,CvRect rect);
	void PrintColor(IplImage* pSmallPic,CvRect recSmallPic,IplImage* pBigPic);

	//添加图象帧
    bool AddFrame(yuv_video_buf* pRGBHeader,BYTE* pBuffer);

	//添加jpg图像帧
	bool  AddJpgFrame(BYTE* pBuffer);

	//添加车牌信息
	bool AddPlateFrame(DSP_PLATE_LIST& listDspPlate);

	//重载检测区域
	inline void SetReloadROI()
	{
        m_bReloadROI = true;
        m_bReloadObjectROI=true;
	}

	//是否需要推送实时车牌
	void SetConnect(bool bConnect){m_bConnect = bConnect;}

	//设置通道地点
	void SetPlace(std::string strLocation){m_strLocation = strLocation;}
	//设置通道方向
	void SetDirection(int nDirection){m_nDirection = nDirection;}

	//设置交通量检测周期
//	void SetChannelTrafficStatTime(const int nTrafficStatTime){ m_nTrafficStatTime = nTrafficStatTime;}

	//设置检测类型
    void SetDetectKind(CHANNEL_DETECT_KIND nDetectKind);

    //设置需要载入的boost路径(根据不同的相机编号 1:上海;2:北京)
    void SetCameraID(int  nCameraID)    { m_nCameraID = nCameraID;}
	//设置设备ID
	void SetDeviceID(string strDeviceId) { m_strDeviceId = strDeviceId; }

    //设置相机类型
    void SetCameraType(int  nCameraType)    { m_nCameraType = nCameraType;}

	//设置视频源格式
	void SetVedioFormat(int  nVedioFormat)    { m_nVedioFormat = nVedioFormat;}

    //设置白天还是晚上检测
	void SetChannelDetectTime(CHANNEL_DETECT_TIME dType);

	//
	#ifndef ALGORITHM_YUV
	CameraParametter* GetCameraCrt() { return &m_CameraCrt;}
	#endif
	//重新初始化
	void ReInit();

	//控制是否事件录象
	void SetEventCapture(bool bEventCapture) {m_bEventCapture = bEventCapture;}
	//修改事件录像
	void ModifyEventCapture(bool bEventCapture);

	//设置事件录象长度
    void SetCaptureTime( int nCaptureTime);

	//添加事件录象缓冲数据
	bool AddVideoFrame(std::string& frame);

	  //设置录像类型
    void SetCaptureType(CAPTURE_TYPE eType) {m_eCapType = eType;}
	//设置事件检测指针
	void SetEventDetect(CAbstractDetect *pDetect)
	{
		m_pDetect = pDetect;
	}

	//输出结果处理
    void DealOutPut();

     //设置帧率
    void SetFrequency(int nFrequency){m_nFrequency = nFrequency;}


	CRoadRecordTemp * GetRecordTemp() { return &m_tempRecord; }

	CSkpRoadRecorder * GetSkpRecorder() { return &m_skpRoadRecorder; }

	 //设置H264采集类指针
	 bool SetH264Capture(CRoadH264Capture *pH264Capture);

	 //清空输出JpgMap列表
	 void ClearJpgFrameMap();
	
	 //缓冲yuv数据
	void AddYUVBuf(int nCodingNum,unsigned int pCodingIndex[16]);
	 //删除yuv数据
	void DeleteYUVBuf(int nDelNum,unsigned int pDelIndex[16]);

	//把有图未输出的数据全部输出
	int DealOutPutAll();

	
#ifdef REDADJUST
	//红绿灯增强
	void RedLightAdjust(IplImage* pImage,bool bCheckRect = true, UINT16 nColorSignal = 0, unsigned long uSignalSeq = 0);
#endif

	//更新全局uKey标记,时间戳
	void UpdateImgKey(UINT64 &uKey, UINT32 &uTime);
	//通过uImgKey标记更新，核查记录状态
	bool CheckImgListByKey(const UINT64 &uKey);
	//通过uImgKey标记更新，记录状态
	bool UpdateImgListByKey(const UINT64 &uKey, const bool &bState);
	//间隔5分钟，核查一次缓存，删除8分钟以前的数据
	void CheckImgList();

	//取得最老的记录下标
	UINT32 GetOldestImg();
	//删除最老的n记录
	UINT32 DelOldestImgs(int n);

private:
	//车牌检测
    bool OnDetect(rgb_buf result);
	void DetectCarNum(IplImage* img,CvRect rtROI,UINT32 uTimestamp,int64_t ts,unsigned long  nSeq,int deinterlace);
	//修改待检帧队列长度
	void DecreaseSize(int nSize);
	//取出一帧数据
    int PopFrame(rgb_buf& response);

    //目标检测
    void DetectObject(rgb_buf  rgbBuf);
    //修改检出帧队列长度
    void DecreaseDetectedSize();
    //从检出队列中弹出一帧数据
    int PopDetectedFrame(rgb_buf& response);

	//通过帧号查找车牌结果
	bool GetInPutBySeq(UINT32 nSeq, std::vector<CarInfo>& vResult);

	//输出一辆车
	void CarNumOutPut(std::vector<CarInfo>& vResult,std::vector<ObjSyncData> &syncData);
	void CarNumOutPutForMatch(std::vector<CarInfo>& vResult,std::vector<ObjSyncData> &synData);

	//输出线圈检测结果
	void OutPutLoopResult(CarInfo& cardNum,RECORD_PLATE& plate,std::vector<ObjSyncData>::iterator it_s,PLATEPOSITION* TimeStamp,bool bLightImage,bool bLoop);
	//输出回写队列中的记录
	void OutPutWriteBack(CarInfo& cardNum,RECORD_PLATE& plate,std::vector<ObjSyncData>::iterator it_s,IplImage* pImage = NULL,bool bLightImage = false);
	//输出上一条记录
	void OutPutPreResult(RECORD_PLATE& plate,PLATEPOSITION* pTimeStamp,int nOutputMode);
	//根据帧号查找出对应的图片
	bool GetImageByIndex(UINT32 nSeq,UINT32 nPreSeq,PLATEPOSITION* pTimeStamp,IplImage* pImage,bool bLightImage = false,bool bCarNum = true);

	//根据帧号查找出对应的图片2
	int GetImageByIndex2(Vts_Picture_Key pickey,UINT32& nPreSeq);

	//从Jpg缓冲区查找出对应的图片
	bool GetImageByJpgSeq(UINT32 nSeq,UINT32 nPreSeq,PLATEPOSITION* pTimeStamp,IplImage* pImage, string &strGetPic);
	//从jpg缓冲区获取闯红灯结果图像
	bool GetVtsImageByJpgSeq(UINT32* frameSeqs, PLATEPOSITION* pTimeStamp, int nPicCount,UINT32 SignalframeSeq, PLATEPOSITION* pSignalTimeStamp,UINT32 uViolationType);
	//200万相机获取闯红灯结果图像
    bool GetVtsImageByIndex(UINT32* frameSeqs,PLATEPOSITION* pTimeStamp,int nPicCount,UINT32 SignalframeSeq,PLATEPOSITION* pSignalTimeStamp,UINT32 uViolationType);
	//叠加文字信息:车牌号码，日期等
	void PutTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex = 0,PLATEPOSITION* pTimeStamp = NULL);
	//电警叠加文本信息
    void PutVtsTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex = 0,PLATEPOSITION* pTimeStamp = NULL,PLATEPOSITION* pSignalTimeStamp = NULL,int64_t redLightStartTime = -1, UINT32 uViolationType = 0);
    //文字组合叠加图片
    void PutTextOnComposeImage(IplImage* pImage,RECORD_PLATE plate,int nOrder = 0);
	//保存全景图像
	int SaveImage(IplImage* pImg,std::string strPicPath,int nIndex = 0);
	//保存人脸特征图像
	int SaveFaceImage(IplImage* pImg,RECORD_PLATE& plate,vector<CvRect> vFaceRect);

	//获取图象坐标世界坐标转换矩阵
	void mvfind_homography(float *image, float *world, float homography_image_to_world[3][3]);

	//读取车道标定信息
	bool InitDetectConfig(int nType = 0);

	bool LoadRoadSettingInfo(int nType,vector<mvvideostd>& vListStabBack,CvRect& farrect,CvRect& nearrect);

	//相机控制
	void CaMeraControl(UINT32 uTimeStamp,bool bCarnumControl,road_context context);

	//流量统计
    void VehicleStatistic(StatResultList& listStatResult,UINT32 uTimestamp);
	//流量评价统计
	void VtsStatistic(yuv_video_buf* buf);

	//统计车流量
	void StatisticVehicleFlux(CarInfo& cardNum,RECORD_PLATE& plate);

	void DetectCarNumYUV(unsigned char* img,int64_t ts,unsigned long  nSeq);

	void ConvertDspSetting(CvRect farrect,CvRect nearrect);

	//获取图像宽高
	bool GetImageWidthHeight(int& nWidth,int& nHeight);

    //获取车道控制灯状态
    void GetSignalStatus(yuv_video_buf* buf);
    //获取车道线圈状态
    void SetLoopStatus( Speed_Signal uLoopSignal);
    //获取车道爆闪灯状态
    void GetFlashStatus(unsigned short uFlashSignal,int64_t ts,vector<mvvideoshutterpar>& videoshutterpar);

    //输出闯红灯检测结果
    void OutPutVTSResult(std::vector<ViolationInfo>& vResult);

	//输出一条违章检测结果
	bool OutPutVTSResultElem(ViolationInfo &infoViolation, string &strEvent, SRIP_DETECT_HEADER &sHeader, bool bGetVtsImgByKey);
	
	//从jpg map中寻找一个最接近的图片
	void GetPicFromJpgMap(UINT32 nSeq,UINT32 nPreSeq,string& strPic);
    //从map中寻找一个最接近的序号
    int GetSeqIndexFromMap(UINT32 nSeq,UINT32 nPreSeq,SeqIndexMap& mapSeqIndex);
    //获取秒图
    bool GetImageFromResultFrameList(UINT32 nSeq,UINT32 nPreSeq,yuv_video_buf& bufResult);

    //添加秒图
    void AddResultFrameList(BYTE* frame);
    //获取图片路径并将图片编号存储在数据库中
    int GetPicPathAndSaveDB(std::string& strPicPath);
	//获取图片路径
	int GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath);

    //判断是否出现柴油车
    void GetVtsResult(std::vector<CarInfo>& vResult,std::vector<ViolationInfo>& vtsResult);

    //非电警违章类型报警
    void GetVtsResult(CarInfo& cardNum,RECORD_PLATE& plate);

    //获取车身位置
    CvRect GetCarPos(RECORD_PLATE plate,int nType = 0);

    //添加处理结果
    void AddOutPut(std::vector<CarInfo>& vResult);

    //获取处理结果
    void PopOutPut(std::vector<CarInfo>& vResult);

	//添加违章检测处理结果
	void AddVtsOutPut(std::vector<ViolationInfo>& vResult);

	 //获取违章检测处理结果
    void PopVtsOutPut(std::vector<ViolationInfo>& vResult);

	//获取Dsp处理结果
	void Dsp_PopOutPut(std::vector<CarInfo>& vResult);

	//获取Dsp处理结果,4月2号之前版本
	void Dsp_PopOutPutOld(std::vector<CarInfo>& vResult);

	////判断是否发生线圈电警违章行为
	void DoLoopVts(UINT32 nSeq,int64_t ts,UINT32 uTimestamp,Speed_Signal uLoopSignal,std::vector<ViolationInfo>& vtsResult);
	
	//输出测试结果
	void OutPutTestResult(UINT32 uSeq,int64_t ts);

	//读取测试结果文件
	bool LoadTestResult(char* filename);

	//初始化目标检测库
	bool SetObjectDetectPara();
	
	//通过帧号，从jpg map中寻找一个违章检测，对应的图片最接近的图片
	void GetPicVtsFromJpgMap(UINT32 nSeq, UINT32 nPreSeq, string& strPic);
	//分配内存
	void AlignMemory(int uWidth,int uHeight);

	//从jpg map中寻找一个最接近的图片
	void GetPicByKeyFromMap(Picture_Key pickeyDst, string& strPic);

	//从jpg map中寻找一个最接近的违章图片
	bool GetVtsPicByKeyFromMap(Vts_Picture_Key pickeyDst, string& strPic);

	/*
	* 函数说明：从m_vtsPicMap缓冲中，获取图片
	*/
	bool GetVtsImagByKey(ViolationInfo infoViolation, UINT32 uViolationType, UINT32 SignalframeSeq, \
		PLATEPOSITION* pTimeStamp, PLATEPOSITION* pSignalTimeStamp);
	
	//滤波
	bool Filter( unsigned char *pImageData, short nWidth, short nHeight,long *kernel, short Ksize, short Kfactor, short Koffset, unsigned char *pTempImage );

	//存图测试
	void SaveImgTest(IplImage *pImg,const int nData = 0);

	//从map中寻找一个最接近的序号,方法2
	int GetSeqIndexFromMap2(UINT32 nSeq,UINT32 nPreSeq, SeqIndexMap& mapSeqIndex);

	//获取秒图
	bool GetImageFromResultFrameList2(Vts_Picture_Key pickey,UINT32 nPreSeq,yuv_video_buf& bufResult);

	//dsp1拖2版本红绿灯增强
	void ProcessSinglePicRedForDsp(const int nPicCount, const PLATEPOSITION* pTimeStamp);

	//载入检测参数
	bool LoadDspSettingInfo2();

	//车型检测
	void DetectTruck(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context,CvRect rtRealImage,IplImage* pImage);

	//车身颜色检测
	void CarColorDetect(bool bCarNum,bool bLoop,CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context,CvPoint& loopPtUp,CvPoint& loopPtDowon,CvRect& rtRoi);

	//车标检测
	void CarLabelDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context,CvPoint& loopPtUp,CvPoint& loopPtDowon,CvRect& rtRoi);

	#ifdef BELTDETECT
	//安全带检测
	void BeltDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context context,vector<FaceRt>& vecP,int& nBeltResult);
	
	//叠加人脸图片
	void PutFaceOnImage(vector<FaceRt>& vecP);
	#endif

	//车标细分
#ifdef GLOBALCARLABEL
	void GetCarLabelAndChildSub(UINT32 &uCarBrand,UINT32 &uDetailCarBrand);
	UINT32 GetCorrectBasePoint(UINT32 &uCarBrand);
	UINT32 GetCarBasePoint(UINT32 uMin,UINT32 uMax,UINT32 uCarBrand);
#endif

	//车牌二次检测
	bool CarNumTwiceDetect(CarInfo& cardNum,carnum_context& vehicle_result);

	//比较车牌相似度
	int GetSimilarNumOfPlate( char * strfirst,char *strSecond,int nSize);

	//遮阳板检测
	void CarShieldDetect(CarInfo& cardNum,RECORD_PLATE& plate,CvRect rtRealImage,CvRect rtCarnum);

	//目标特征识别
	void ObjectCharacterDetect(RECORD_PLATE& plate,CvRect rtCarPos);

	//获取事件车牌
	void GetEventCarnum(CarInfo& cardNum);

	//图像增强
	void ImageEnhance(IplImage* pImg);

	//发送车牌检测结果
	void SendResult(RECORD_PLATE& plate,unsigned int uSeq);

	//分开存储图像
	void SaveSplitImage(RECORD_PLATE& plate,CvRect rtCarPos,PLATEPOSITION* TimeStamp,string& strPicPath2);

	//叠加存储图像
	void SaveComposeImage(RECORD_PLATE& plate,CvRect rtCarPos,PLATEPOSITION* TimeStamp);

	//存储违章图像
	void SaveVtsImage(RECORD_PLATE& plate,int nPicCount,PLATEPOSITION* TimeStamp,PLATEPOSITION& SignalTimeStamp,int64_t redLightStartTime,string& strPicPath2,string& strPicPath3);

	//获取图片路径
	int GetPicSavePath(RECORD_PLATE& plate,string& strPicPath,string& strPicPath2);

	//获取违章图片路径
	int GetVtsPicSavePath(RECORD_PLATE& plate,int nPicCount,string& strPicPath,string& strPicPath2,string& strPicPath3);

	//获取录像路径
	void GetVideoSavePath(RECORD_PLATE& plate,SRIP_DETECT_HEADER& sHeader);

	//Dsp流量统计
	void VehicleStatisticDsp(StatResultList& listStatResult,UINT32 uTimestamp);
	//500W图像转成200W
	void ReSizePic(IplImage* pImageIn, IplImage* pImageOut);
	
	#ifdef OBJECTFEATURESEARCH
	//生成特征数据
	void OutPutFeatureResult(feature* mfeature,RECORD_PLATE plate);
	#endif

	#ifdef OBJECTFACEDETECT
	//生成特征数据
	void OutPutFaceFeature(mv_stFaceFeature& faceFeatures,RECORD_PLATE plate);
	#endif

#ifdef MATCHPLATE
	//添加输出结果到比对队列
	bool AddDspPlateMatch(RECORD_PLATE &plate,  const IplImage * pImgSnap);
#endif
//#ifdef FBMATCHPLATE
	//添加输出结果到比对队列
	bool AddDspPlateMatchFortianjin(RECORD_PLATE &plate,  const IplImage * pImgSnap);
//#endif

	//核查图片是否已经接收进缓存
	bool CheckJpgInMap(int64_t nSeq);

	//删除一张图片
	bool DelOneJpgElem(yuv_video_buf* pHeader);

	//删除一张图片,4.2之前版本
	bool DelOneJpgElemOld(yuv_video_buf* pHeader);


	//核查违章信息输出标志
	int CheckViolationInfo(ViolationInfo &vio);

	//获取事件开始时间,通过第1帧时间和帧率计算.
	void GetEventBeginTime(const ViolationInfo &infoViolation, RECORD_EVENT &event, const TIMESTAMP vtsStamp[3]);

	//核查待输出记录是否有图
	bool CheckOutResult(std::vector<CarInfo>& vResult);
	
	//输出卡口
	void OutPutKakou(std::vector<CarInfo>& vResult);

	//获取违章图片
	bool GetVtsPic(UINT32* frameSeqs, PLATEPOSITION* pTimeStamp, int nPicCount, yuv_video_buf* buf, std::string& strPic1, std::string& strPic2, std::string& strPic3);


	//核查违章图片是否都存在,不全则不输出
	bool CheckVtsPic(const ViolationInfo &infoViolation, TIMESTAMP *pStamp);


	//获取对应帧号,取图,二次识别检出车型细分类型
	int GetTypeDetailTwo(
		UINT32 uSeq,
		CarInfo &cardNum,
		RECORD_PLATE &plate, 
		carnum_context &context,
		PLATEPOSITION* pTimeStamp);

	//处理二次识别检出车型细分类型,针对不同车型细分进行处理
	bool DealVtsTypeDetail(int nTypeDetail);
	//取违章时间戳
	bool GetJpgTimStampInMap(const int64_t uSeq, TIMESTAMP &timeStamp);

//#ifdef FBMATCHPLATE
	//添加图片到缓存队列
	void AddImgToSrcList(const int &nIndex, const string &strPic, RECORD_PLATE_DSP_MATCH * pMatchPlate);
	//添加输出违章结果到比对队列
	bool AddDspPlateMatchVts(const RECORD_PLATE &plate, RECORD_PLATE_DSP_MATCH * pMatchPlate);
//#endif

//#ifdef MATCH_LIU_YANG
	//获取比对违章检测处理结果
	void PopVtsOutPutMatch(std::vector<ViolationInfo>& vResult);
	//核查违章记录是否输出
	bool CheckVtsOutPutMatch(const ViolationInfo & vioInfo);
	//重设m_pMatchPlate
	void ReInitMatchPlate(RECORD_PLATE_DSP_MATCH * pMatchPlate);
	//添加一张图到比对列表
	void AddJpgToMatch(const string &strPic, const int &nIndex, RECORD_PLATE_DSP_MATCH * pMatchPlate);
//#endif

#ifdef REDADJUST
	//初始化,红灯增强,算法接口2
	bool InitRedAdjust2();
	void GetRedColor(const UINT16 uTrafficSignal, int nColorArray[]);
#endif

	bool CopyRect(const CvRect &rtSrc, CvRect &rtDest);	

	//图片上叠加信号值 [RED][GREEN]
	void PutRedSignal(IplImage* pImage, int nColorArray[], unsigned long uSignalSeq = 0);

#ifdef MATCH_LIU_YANG_DEBUG
	//无锡测试,违章4合一图片叠字格式
	void WuXiVtsText(const RECORD_PLATE &plate, std::string &strText);
#endif

//私有变量
private:
	//读写信号互斥
    pthread_mutex_t m_FrameMutex;
    //检出场读写信号互斥
    pthread_mutex_t m_PlateMutex;
    //添加帧信号互斥
    pthread_mutex_t m_AddFrameMutex;
    //输出结果互斥
    pthread_mutex_t m_OutPutMutex;
	//jpg互斥锁
	pthread_mutex_t m_JpgFrameMutex;
	//输入结果互斥
	pthread_mutex_t m_InPutMutex;
	pthread_mutex_t m_ImgTagMutex;
	//卡口输出互斥
	pthread_mutex_t m_OutCarnumMutex;

	//线程结束标志
	bool m_bEndDetect;
	//车牌检测线程ID
	pthread_t m_nThreadId;
	//目标检测线程ID
	pthread_t m_nObjectThreadId;
	//输出结果线程ID
    pthread_t m_nOutPutThreadId;
	////////////////////////
	//待检场循环队列
	BYTE* m_chFrameBuffer[50];
	//待检场循环队列首地址
	BYTE* m_pFrameBuffer;
	//待检场循环队列当前可写地址
	BYTE* m_pBuffer;
	//待检场循环队列实际长度
	int m_FrameSize;
	//待检场循环队列当前读取位置
	int m_nReadIndex;
	//待检场循环队列当前写位置
	int m_nWriteIndex;
	//检出队列最大长度
	int m_nMaxFrameSize;

    ///////////////////////////////////////////////////////////////////////////////////

	//检出场循环队列
	BYTE* m_chPlateBuffer[200];
	//检出场循环队列首地址
	BYTE* m_pPlateBuffer;
	//检出场循环队列当前可写位置
	int m_nCurIndex;

	//检出帧地址循环队列
	rgb_buf m_BufPlate[100];
	 //检出帧循环队列实际长度
    int m_PlateSize;
	//检出帧地址循环队列当前可写位置
	int m_nPlateIndex;
	//检出帧循环队列当前可读位置
    int m_nPlateReadIndex;
    //检出队列最大长度
    int m_nMaxPlateSize;

    //seq->index映射
    //SeqIndexMap  m_mapSeqIndex;
    //上一帧帧号
    UINT32 m_uPreSeq;


	//车牌检测是否重新读取配置
	bool m_bReloadROI;
	//目标检测是否重新读取配置
	bool m_bReloadObjectROI;
	//重新初始化目标检测
	bool m_bReinitObj;
	//是否初始化车牌库
	int m_bInitCarNumLib;
	//车牌检测参数(每个车道一个)
	CnpMap m_cnpMap;
	//检测方向
	int m_nDetectDirection;

	//车牌检测帧图象
	IplImage* m_imgFrame;
	//车牌检测第一场图象
	IplImage* m_img1;
	//车牌检测第二场图象
	IplImage* m_img2;
    //目标检测帧图象
	IplImage* m_imgSnap;
	//上一帧图象
	IplImage* m_imgPreSnap;
	//存储检出图像
	IplImage* m_imgDestSnap;
	//目标检测场图象
	IplImage* m_img;
	//闯红灯合成图像（3帧）
	IplImage* m_imgComposeSnap;
	//合成图像需要的单帧图片
	IplImage* m_imgComposeResult;
	//回写图象
	IplImage* m_imgWriteBack;
	//结果单帧图片指针
	IplImage* m_imgResult;
	//结果单帧图片指针(不包含黑边)
	IplImage* m_imgSingleResult;
	//亮图
	IplImage* m_imgLightSnap;
	//缩放图像
	IplImage* m_imgResize;

	//存储检出图像中间结果
	IplImage* m_imgDestSnap2;
	//存储检出图像中间结果
	IplImage* m_imgComposeSnap2;

//#ifdef MATCH_LIU_YANG
	//缓存违章源图队列
	//IPL_IMAGE_TAG m_imgSrcFrames[4];
	RECORD_PLATE_DSP_MATCH *m_pMachPlate;//比对车牌结构
//#endif

	//组合JPG图片
    BYTE* m_pComposeJpgImage;
    //当前JPG图像
    BYTE* m_pCurJpgImage;
    //上一帧JPG图像
    BYTE* m_pPreJpgImage;
    //JPG小图
    BYTE* m_pSmallJpgImage;

    //VLP检测区域
    CvRect m_rtVlpRoi;
    //车牌检测区域
	CvRect m_rtCarnumROI;
    //VTS检测区域
    CvRect m_rtVtsRoi;
    //事件检测区域
    CvRect m_rtEventRoi;
    //红灯检测区域
    CvRect m_rtTrafficSignalRoi;
    //雷达检测区域
    CvRect m_rtRadarRoi;
    //交通灯检测区域
    RedLightTwinkRgn m_rgnTrafficSignal;
    //停止线
    MvLine  m_lineStop;
    //直行检测线
    MvLine  m_lineStraight;
    //左转检测线
    MvLine  m_lineTurnLeft;
    //右转检测线
    MvLine  m_lineTurnRight;
    //压线
    vector<MvLine> m_vecYellowLines;
	//压白线
	vector<MvLine> m_vecWhiteLines;
    //变道线
    vector<MvLine> m_vecBianDaoLines;
    //图象坐标和世界坐标变换矩阵
	float homography_image_to_world[3][3];
    //图象坐标和世界坐标点
    //for 目标检测线程
	float m_image_cord[12];
	float m_world_cord[12];
    //for 车牌检测线程
	float m_image_cord2[12];
	float m_world_cord2[12];
	//目标检测坐标列表
	vector<ChannelRegion> m_vtsObjectRegion;
	//闯红灯检测参数列表
    VTSParaMap  m_vtsObjectParaMap;
    //车道控制灯状态
    ChnFrmSignalStatusList  m_vtsSignalList;
	//控制灯状态列表
	map<int,MvChannelFrameSignalStatus> m_mapVTSStatus;
    //违章检测全局参数
    VTS_GLOBAL_PARAMETER m_vtsGlobalPara;
	//线圈状态列表
	LoopStatusMap m_mapLoopStatus;

    //线圈检测参数列表
    LoopParaMap m_LoopParaMap;

    //线圈位置及检测参数
    vector<LoopInfo> m_LoopInfo;

    //车牌线程线圈参数
    loop_parmarer m_LoopParmarer;
    //车道检测参数
    paraDetectList m_roadParamInlist;

	//车道编号速度映射表
	map<int,int> m_mapSpeed;

    //x缩放比
	double m_ratio_x;
	//y缩放比
	double m_ratio_y;

	//x缩放比
	double m_fScaleX;
	//y缩放比
	double m_fScaleY;

    //通道编号
	int m_nChannelId;
	//经过地点
	std::string m_strLocation;
	//行驶方向
	int m_nDirection;
	//相机编号
	int m_nCameraID;
	//设备ID
	string m_strDeviceId;

	//相机型号
	int m_nCameraType;
	//录像类型
	int m_nVedioFormat;
	//检测类型
	CHANNEL_DETECT_KIND m_nDetectKind;

	//图片编号
	UINT32 m_uPicId;
	//图像文本信息
	CvxText m_cvText;
	//
	CvxText m_cvBigText;
	//文本区域高度
	int m_nExtentHeight;

	//场图像还是帧图像
	int m_nDeinterlace;

	//防伪码
	int m_nRandCode[3];

	/******************机动车道流量统计*************/
	UINT32 uFluxAll[MAX_ROADWAY];       //总流量
    UINT32 uFluxSmall[MAX_ROADWAY];     //小车流量
    UINT32 uFluxMiddle[MAX_ROADWAY];  //中型车流量
    UINT32 uFluxBig[MAX_ROADWAY];   //大车流量
	UINT32 uFluxPerson[MAX_ROADWAY];  //行人
	UINT32 uFluxNoneVehicle[MAX_ROADWAY];//非机动车
	UINT32 uBigCarSpeed[MAX_ROADWAY];	//大车平均速度
	UINT32 uSmallCarSpeed[MAX_ROADWAY];	//小车平均速度
	UINT32 uAverageCarSpeed[MAX_ROADWAY];	//平均速度
	UINT32 uMaxSpeed[MAX_ROADWAY];		//最大速度

	UINT32 uOccupyRatio[MAX_ROADWAY];//车道占有率
	UINT32 uAvgDis[MAX_ROADWAY];	//车头间距	
	UINT32 uPreCarTime[MAX_ROADWAY]; //前一辆车时刻

	//统计时间长度
	int m_nTrafficStatTime;
	unsigned int m_uPreTrafficSignalTime; //上次统计时刻
	
	/******************机动车道流量评价统计*************/
	//直行
	UINT32 m_uNonTrafficSignalTime; //非红灯时间
	int64_t m_uNonTrafficSignalBeginTime; //非红灯开始时刻
	int64_t m_uNonTrafficSignalEndTime; //非红灯结束时刻
	int64_t m_uPreNonTrafficSignalTime; //上次统计时刻
	int m_nConnectnumber;//采集序号
	UINT32 uSignalFluxAll[MAX_ROADWAY];       //总流量
	UINT32 uSignalFluxSmall[MAX_ROADWAY];     //小车流量
	UINT32 uSignalFluxBig[MAX_ROADWAY];   //大车流量
	UINT32 uSignalBigCarSpeed[MAX_ROADWAY];	//大车平均速度
	UINT32 uSignalSmallCarSpeed[MAX_ROADWAY];	//小车平均速度
	UINT32 uSignalAverageCarSpeed[MAX_ROADWAY];	//平均速度
	UINT32 uSignalMaxSpeed[MAX_ROADWAY];		//最大速度

	//左转
	UINT32 m_uNonLeftTrafficSignalTime; //非红灯时间
	int64_t m_uNonLeftTrafficSignalBeginTime; //非红灯开始时刻
	int64_t m_uNonLeftTrafficSignalEndTime; //非红灯结束时刻
	int64_t m_uPreNonLeftTrafficSignalTime; //上次统计时刻
	int m_nLeftConnectnumber;//采集序号
	UINT32 uLeftSignalFluxAll[MAX_ROADWAY];       //总流量
	UINT32 uLeftSignalFluxSmall[MAX_ROADWAY];     //小车流量
	UINT32 uLeftSignalFluxBig[MAX_ROADWAY];   //大车流量
	UINT32 uLeftSignalBigCarSpeed[MAX_ROADWAY];	//大车平均速度
	UINT32 uLeftSignalSmallCarSpeed[MAX_ROADWAY];	//小车平均速度
	UINT32 uLeftSignalAverageCarSpeed[MAX_ROADWAY];	//平均速度
	UINT32 uLeftSignalMaxSpeed[MAX_ROADWAY];		//最大速度

    ////////////////////////////////////////////////////
	//检测帧数
	int m_detectframe;
	//是否推送实时车牌
	bool m_bConnect;

	//带宽测试
	UINT32 m_uImageSize;
	//白天还是晚上(算法用，固定时间)
    int m_nDayNight;
	//白天还是晚上(图像增强用，同开关灯时间)
    int m_nDayNightbyLight;
    //检测时间类别
    CHANNEL_DETECT_TIME m_nDetectTime;
	
	#ifndef ALGORITHM_YUV
    //目标检测
	MvFindTargetByVirtualLoop m_ftVlp;
	#ifndef GLOBALCARCLASSIFY
	//车辆类型检测
	MvVehicleClassify m_vehicleClassify;
	#endif
	#endif

	//缓冲15秒的图像数据，每1秒保存一次
	std::list<std::string> m_ResultFrameList;

	//JPG大图队列
	map<int64_t,string> m_JpgFrameMap;
	//关键字为帧号和相机编号组成的结构体的JPG大图队列(检测器为服务端)
	map<Picture_Key,string> m_ServerJpgFrameMap;

    //用于实现回写
	//车辆信息映射
	CarInfoMap MapCarInfo;
	//同步标记信息映射
    SyncDataMap MapSyncData;
    //图片信息映射
    PicDataMap MapPicData;
	
	#ifndef ALGORITHM_YUV
	#ifndef GLOBALCARLABEL
    CarLabel m_carLabel;//车标
	#endif
	#ifndef GLOBALCARCOLOR
    ColorRecognisize m_carColor; //颜色
	#endif
    CameraParametter m_CameraCrt; //相机控制

    Mv_CarnumDetect mvcarnumdetect;//车牌检测
	#endif
	
	#ifdef REDADJUST
	MvRedLightAdjust* m_pRedLightAdjust;//红绿灯增强
	#endif
	
	#ifdef BELTDETECT
	MvSafetyBeltClassify m_safeType;
	#endif

	//通过缓存录违章录像
	CRoadRecordTemp m_tempRecord;
    //事件录象类
    CSkpRoadRecorder m_skpRoadRecorder;

    //是否事件录象
    bool m_bEventCapture;
    //事件录象长度
	int m_nCaptureTime;
	//录像类型
    CAPTURE_TYPE m_eCapType;
    //存图数量
    int m_nSaveImageCount;
    //是否存小图
	int m_nSmallPic;
	//文字位置
	int m_nWordPos;
	//字叠加在图片上
	int m_nWordOnPic;
	//事件检测指针
	CAbstractDetect *m_pDetect;
    //输出结果向量
	std::vector<CarInfo> m_vResult;
	//违章检测输出结果向量
	std::vector<ViolationInfo> m_vtsResult;
	//上次输出图片信息
    OUTPUT_PIC_INFO m_ExistPicInfo;
     //帧率
    int m_nFrequency;
	list<string> m_listBASE_PLATE_INFO_1;//存储BASE_PLATE_INFO中的车牌(存储车牌类型1的)
	list<string> m_listBASE_PLATE_INFO_2;//存储BASE_PLATE_INFO中的车牌(存储车牌类型2的)
	list<string> m_listBASE_PLATE_INFO_3;//存储BASE_PLATE_INFO中的车牌(存储车牌类型3的)
	bool m_bGetPlate;//判断strBASE_PLATE_INFO是否已经加载了车牌

	//dsp车牌处理
	//std::multimap<int64_t,CarInfo> m_mapInPutResult;
	std::multimap<int64_t,DSP_CARINFO_ELEM> m_mapInPutResult;
	//关键字为帧号和相机编号组成的结构体的dsp车牌处理(检测器为服务端)//add by wantao
	std::multimap<Picture_Key,CarInfo> m_ServermapInPutResult;

	#ifdef DETECTSHIELD
	MvFrontWindPos m_ShieldGet;
	#endif

	float m_fSpeedFactor; //雷达测速矫正因子

	bool m_bImageEnhance;//是否图像增强
	bool m_bDetectShield;//是否检测遮阳板
	int  m_nImageEnhanceFactor;//图像增强因子
	int m_nSinglePicNonPlate;//未识别出车牌是否单张图
	
	#ifdef OBJECTFEATURESEARCH
	 MvPeopleSearch * m_pFeatureSearchDetect;
	 unsigned int m_uFeatureSeq;
	#endif

	 #ifdef OBJECTHEADDETECT
	 FaceDetection  m_HeadDetection;
	#endif

	  #ifdef OBJECTFACEDETECT
	 MV_FaceDet  m_FaceDetection;
	 //人脸搜索
	MvFaceTrackSearch m_FaceTrackSearch;
	#endif
	
	 //读结果文件
	 std::list<RECORD_PLATE> m_listTestResult;
	 bool m_bTestResult;
	 int m_nFileID;

	 //记录缓冲区满的次数
	 int m_nCountBufferExceed;

	 int m_nNotGetJpgCount; //未获取到图片计数

	 CRoadH264Capture* m_pH264Capture;

	 //无牌情况下目标类型二次识别
	// MvClassRecognitionTwice m_CRecognitionTwice;

	 //图片格式信息
	PIC_FORMAT_INFO m_PicFormatInfo;

	//是否是多车道
	bool          m_IsMultiChannel;
	unsigned int  m_DetectAreaUp;     //车牌检测区域上边界
	unsigned int  m_DetectAreaBelow;  //车牌检测区域下边界

	//检测器-车载公交方案需按实际图像宽高来分配内存(单张图片)
	IplImage* m_imgGJSnap;
	//检测器-车载公交方案需按实际图像宽高来分配内存(合成图片（2*2）)
	IplImage* m_imgGJComposeSnap;
	//检测器-车载公交方案合成图像需要的单帧图片
	IplImage* m_imgGJComposeResult;
	//检测器-车载公交方案组合JPG图片
	BYTE* m_pGJComposeJpgImage;
	//检测器-车载公交方案当前JPG图像
	BYTE* m_pGJCurJpgImage;
	//判断是否设置车牌高度
	bool m_bSetCarnumHeight;
	//滤波图像
	BYTE* m_pImageFilter;
	//滤波图像
	BYTE* m_pVtsImageFilter;

	VtsPicDataMap m_vtsPicMap;//违章输出缓存

	int m_BlackFrameWidth;
	UINT32 m_nFontSize;

	CvxText m_nJiaoJinCvText;
	CvxText m_nXingTaiCvText;

	list<CvPoint> m_ptsYelGrid;

	BYTE* m_pDataY[3];

	IplImage* m_pImageDataY;//yuv大图
	IplImage* m_pImageSmallDataY;//yuv小图

	//map<string,int> m_mapCachePlate;
	vector<string> m_vecCachePlate;
	vector<sVtsPlateCache> m_vecVtsCachePlate;
#ifdef MATCHPLATE
	IPL_IMAGE_TAG m_imgTagList[MAX_IMG_TAG_NUM];
	unsigned int	m_uCurrImgtag;				//环形缓冲的当前使用索引
#endif //End of #ifdef MATCHPLATE
//#else ifdef  FBMATCHPLATE

	pthread_mutex_t m_imgTagMutex;//jpg记录互斥锁
	JPG_IMAGE_TAG m_imgTagList[MAX_IMG_TAG_NUM];
	

	unsigned int	m_uCurrImgtag;				//环形缓冲的当前使用索引	
//#endif //End of #ifdef MATCHPLATE
    
	int m_nRedNum;//红绿灯区域个数
	int m_nDealFlag;//是否立即输出记录标记 -1:不输出 0:正常输出 1:立即输出

#ifdef PLATEDETECT
	MvPlateDetector m_PlateDetector;
#endif

	bool m_bLoadCarColor;
	bool m_bLoadCarLabel;

	string m_strPicMatch;

#ifdef REDADJUST
	int m_nIndexRedArray[MAX_RED_LIGHT_NUM];//红灯信号对应灯头下标
	RED_DIRECTION m_RedDirectionArray[4];//最多4车道,每个车道对应信号灯方向[左,中,右,转弯]
#endif

	int m_nLastCheckTime;//记录上一次核查记录时间

	CDspDataProcess *m_pCDspDataProcess;

#ifdef REDADJUST
	//红灯增强互斥
	pthread_mutex_t m_RedAdjustMutex;

	CvRect m_rectRedArray[MAX_RED_LIGHT_NUM];
#endif
};
#endif
#endif //SKP_ROAD_CARNUM_H
