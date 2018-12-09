// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef DSP_DATA_PROCESS_H
#define DSP_DATA_PROCESS_H

#ifndef ALGORITHM_YUV
#include "global.h"
#ifndef ALGORITHM_DL
#include "CarnumParameter.h"
#else
#include "Mv_CarnumDetect.h"
#endif
#include "MvVehicleClassify.h"
#include "MvFindTargetByVirtualLoop.h"
#include "CvxText.h"
#include "LabelRecognition.h"
#include "ColorRecognition.h"
#ifndef ALGORITHM_DL
#include "Mv_CarnumDetect.h"
#endif
#include "DspDataManage.h"
#include "structdef.h"
#include "MvFBMatch2.h"
#ifdef TWICE_DETECT
	#include "MvJudgeOnBusVioValid.h"
#endif

#define MAX_IMG_MATCH_TAG_NUM 6 //Math比对缓存大小

#ifndef NOPLATE

//最短距离-路段名对应表
typedef std::map<float,sRoadNameInfo> mapRoadName; 
//比对数据结构
typedef struct _MatchPlateData
{
	void *param;
	RECORD_PLATE_DSP_MATCH A;
	RECORD_PLATE_DSP_MATCH B;
}sMatchPlateData;
//未比对数据结构
typedef struct _NoMatchPlateData
{
	void *param;
	RECORD_PLATE_DSP_MATCH plate;
}sNoMatchPlateData;
//typedef std::list<string> listPicData;
//typedef std::map<UINT32, listPicData> mapPicData; //车辆编号-图片数据映射
//typedef std::map<UINT64, listPicData> mapSeqPicData; //图片帧号-图片数据映射
//typedef std::map<Picture_Key,listPicData> mapPKPicData;//（相机编号，图片帧号）-图片数据映射

/******************************************************************************/
//	描述:车牌检测类
//	作者:modified by wt
//	日期:2011-9-8
/******************************************************************************/
class CDspDataManage;
class CDspDataProcess
{
public:
    CDspDataProcess(CDspDataManage* pDspManage);
    ~CDspDataProcess();

public:
	//初始化
	bool Init(int nChannelId,UINT32 uWidth,UINT32 uHeight,int nWidth, int nHeight, CHANNEL_DETECT_KIND nDetectKind = DETECT_NONE);

	//结束车牌检测线程
	bool Uninit();

	//启动数据处理线程
	//bool BeginDataProcessThread();

	//重载检测区域
	inline void SetReloadROI()
	{
        m_bReloadROI = true;
	}

	//是否需要推送实时车牌
	void SetConnect(bool bConnect){m_bConnect = bConnect;}

	//设置通道地点
	void SetPlace(std::string strLocation){m_strLocation = strLocation;}
	//设置通道方向
	void SetDirection(int nDirection){m_nDirection = nDirection;}

	//设置检测类型
    void SetDetectKind(CHANNEL_DETECT_KIND nDetectKind);

    //设置需要载入的boost路径(根据不同的相机编号 1:上海;2:北京)
    void SetCameraID(int  nCameraID)    { m_nCameraID = nCameraID;}

    //设置相机类型
    void SetCameraType(int  nCameraType)    { m_nCameraType = nCameraType;}
	int GetCameraType()  { return m_nCameraType;}

    //设置白天还是晚上检测
	void SetChannelDetectTime(CHANNEL_DETECT_TIME dType);

	//添加信息
	bool AddFrame(BYTE* pBuffer);

	//添加车牌信息
	bool AddPlateFrame(BYTE* pBuffer);

	//添加jpg图像帧
	bool  AddJpgFrame(BYTE* pBuffer);

	//获取Dsp处理结果
	//bool Dsp_PopOutPut(CarInfo& cardNum);
	 //获取违章检测处理结果
    //bool PopVtsOutPut(ViolationInfo& infoViolation);

	//输出结果处理
	//void DealOutPut();
	//输出车牌遮挡前后匹配图
	void OutPutVtsMatch(RECORD_PLATE_DSP_MATCH &record_plate, RECORD_PLATE_DSP_MATCH &foundRecord, RECORD_PLATE& plate);
	//拼车牌遮挡违章图
	bool GetVtsImageMatch(IplImage* pImage1, IplImage* pImage2);

	//存储违章图像
	void SaveVtsImageMatch(
					RECORD_PLATE& plate,int nPicCount,PLATEPOSITION* TimeStamp,
					PLATEPOSITION& SignalTimeStamp,int64_t redLightStartTime);
	//电警叠加文本信息
	void PutVtsTextOnImageMatch(
		IplImage*pImage,RECORD_PLATE plate,
		int nIndex,PLATEPOSITION* pTimeStamp,
		PLATEPOSITION* pSignalTimeStamp,int64_t redLightStartTime, 
		UINT32 uViolationType);
	//存图测试
	void SaveImgTest(IplImage *pImg, char* strFileName);

	//电警叠加文本信息
	void PutVtsTextMatchForTianJin(
		IplImage*pImage,
		RECORD_PLATE plate,
		int nIndex,
		PLATEPOSITION* pTimeStamp,
		PLATEPOSITION* pSignalTimeStamp,
		int64_t redLightStartTime, 
		UINT32 uViolationType);

	//输出车牌遮挡前后匹配图-天津版本
	void OutPutVtsMatchForTianJin(
		RECORD_PLATE_DSP_MATCH &record_plate, 
		RECORD_PLATE_DSP_MATCH &foundRecord, 
		RECORD_PLATE& plate);

	//获取车身位置
	void GetCarRect(const RECORD_PLATE &plate, CvRect &rtCar);
	//获取车身位置,取图片下1/4位置.
	void GetCarRect2(const RECORD_PLATE &plate, CvRect &rtCar);

	//核查plate 是否合法
	bool CheckPlateRect(const RECORD_PLATE &plate, CvRect &rtSrc);

	void OutPutVtsMatch2x3(
		RECORD_PLATE_DSP_MATCH &record_plate, 
		RECORD_PLATE_DSP_MATCH &foundRecord, 
		RECORD_PLATE& plate);
	//输出车牌前后匹配合成图-浏阳版本-2x2格式
	void OutPutVtsMatch2x2(
		RECORD_PLATE_DSP_MATCH &record_plate, 
		RECORD_PLATE_DSP_MATCH &foundRecord, 
		RECORD_PLATE& plate);
		
	//输出过滤
	bool OutPutVtsMatchFilter(const RECORD_PLATE_DSP_MATCH &record_plate);

	//输出车牌前后匹配卡口合成图-1x3格式
	void OutPutVtsMatch1x3(
		RECORD_PLATE_DSP_MATCH &record_plate, 
		RECORD_PLATE_DSP_MATCH &foundRecord, 
		RECORD_PLATE& plate);

	//输出未找到匹配的单张卡口
	void OutPutNoMatch(RECORD_PLATE_DSP_MATCH &plate);
	//输出未找到匹配的违章4*1合图
	void OutPutVtsNoMatch(RECORD_PLATE_DSP_MATCH &plate);
	//核查记录可用状态
	bool CheckPlateMatch(const RECORD_PLATE_DSP_MATCH &plateMatch, const int nIndex);
private:
	CDspDataManage* m_pDspManage;
	BYTE* m_pBuffer;

	//输出一辆车
	void CarNumOutPut(CarInfo& cardNum);

	//输出一辆车 for gongjiao
	void CarNumOutPut(CarInfo& cardNum, std::string &strCameraCode);

	//叠加文字信息:车牌号码，日期等
	void PutTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex = 0,PLATEPOSITION* pTimeStamp = NULL);
	//文字组合叠加图片
	void PutTextOnComposeImage(IplImage* pImage,RECORD_PLATE plate,int nOrder);
	//电警叠加文本信息
    void PutVtsTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex = 0,PLATEPOSITION* pTimeStamp = NULL,PLATEPOSITION* pSignalTimeStamp = NULL,int64_t redLightStartTime = -1, UINT32 uViolationType = 0);
	//检测器-车载公交方案 获取三张图
	bool GetVtsImageByJpgKey2(ViolationInfo infoViolation,Picture_Key Pic_Key,PLATEPOSITION* pTimeStamp);


	//保存全景图像
	int SaveImage(IplImage* pImg,std::string strPicPath,int nIndex = 0);

    //输出违章检测结果
    void OutPutVTSResult(ViolationInfo& infoViolation);

	//输出违章检测结果
	void OutPutVTSResult(ViolationInfo& infoViolation,std::string &strCameraCode );
    //获取图片路径并将图片编号存储在数据库中
    int GetPicPathAndSaveDB(std::string& strPicPath);

    //获取车身位置
    CvRect GetCarPos(RECORD_PLATE plate);

	//从jpg map中寻找一个最接近的图片
	//void GetPicByKeyFromMap(Picture_Key pickeyDst, string& strPic);

	//根据帧号和相机ID组成的关键字从Jpg缓冲区查找出对应的图片
	//bool GetImageByJpgKey(Picture_Key Pic_Key,PLATEPOSITION* pTimeStamp,IplImage* pImage);

		//分开存储图像
	void SaveSplitImage(RECORD_PLATE& plate,CvRect rtPos,PLATEPOSITION* TimeStamp);

	//叠加存储图像
	void SaveComposeImage(RECORD_PLATE& plate,CvRect rtPos,PLATEPOSITION* TimeStamp);

	//存储违章图像
	void SaveVtsImage(RECORD_PLATE& plate,int nPicCount,PLATEPOSITION* TimeStamp,PLATEPOSITION& SignalTimeStamp,int64_t redLightStartTime);

	//获取图片路径
	int GetPicSavePath(RECORD_PLATE& plate,string& strPicPath);
	//获取图片路径
	int GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath);
	//获取违章图片路径
	int GetVtsPicSavePath(RECORD_PLATE& plate,int nPicCount,string& strPicPath);

	//发送车牌检测结果
	void SendResult(RECORD_PLATE& plate,unsigned int uSeq);

	//输出一条违章检测结果
	bool OutPutVTSResultElem(ViolationInfo &infoViolation, string &strEvent, SRIP_DETECT_HEADER &sHeader, bool bGetVtsImgByKey);
	bool OutPutVTSResultElem(ViolationInfo &infoViolation, string &strEvent, SRIP_DETECT_HEADER &sHeader,
		bool bGetVtsImgByKey,std::string &strCameraCode);
	//输出测试图片，模拟车牌结果
	void CarNumOutPutTest(CarInfo& cardNum);
	//叠加存储图像（测试用）
	void SaveComposeImageForTest(RECORD_PLATE& plate,CvRect rtPos,PLATEPOSITION* TimeStamp);
	//函数介绍：在图像上叠加文本信息（测试用）
	void PutTextOnImageForTest(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp);
	//找临近图片
	bool GetImgByJpgKeyNeighbor(
		const Picture_Elem &picElem,
		PLATEPOSITION* pTimeStamp,
		IplImage* pImage,
		const int iIndex,
		const bool bRet1,
		const bool bRet2,
		const bool bRet3);

	//拼图2x2
	bool ComposePic2x2(const int iIndex, const RECORD_PLATE &plate);
	//拼图1x3
	bool ComposePic1x3(const int iIndex, const RECORD_PLATE &plate);

	//获取合成图1x3,各张图位置
	bool GetRectByIndex_1x3(
		const int i,
		const IplImage* pImageSrc,
		const IplImage* pImageDst,
		CvRect &rect);

	//核查rect是否合法
	bool CheckRectInImg(const CvRect &rect, const IplImage* pImage);

	//拼违章图
	bool ComposeImg(const CvRect &rectSrc, 
		IplImage* pImageSrc,
		const CvRect &rectDst,	
		IplImage* pImageDst);

	//获取合成图,特写图区域
	bool GetComposeTeXieRect(
		const IplImage* pImageSrc,
		const IplImage* pImageDst,
		CvRect &rect);

	//获取单张图,特写图区域
	bool GetTeXieRect(
		const IplImage* pImageSrc,
		const RECORD_PLATE &plate, 
		CvRect &rtDst);

	//在1x3合成图上,叠字
	void PutVtsTextOnImage1x3(
		IplImage* pImage,
		RECORD_PLATE& plate,
		PLATEPOSITION* TimeStamp,
		PLATEPOSITION& SignalTimeStamp,
		int64_t redLightStartTime);

	//1x3单张图,电警叠加文本信息
	void PutVtsTextOnImage1x3_Elem(
		IplImage*pImage,
		RECORD_PLATE &plate,
		int nIndex,
		PLATEPOSITION* pTimeStamp,
		PLATEPOSITION* pSignalTimeStamp,
		int64_t redLightStartTime);
	//1x3单张图,电警叠加文本信息(公交)
	void PutGJVtsTextOnImage1x3_Elem(
		IplImage*pImage,
		RECORD_PLATE &plate,
		int nIndex,
		PLATEPOSITION* pTimeStamp,
		PLATEPOSITION* pSignalTimeStamp,
		int64_t redLightStartTime);

	//重设违章图片车牌区域坐标
	void ReSetVtsPlatePos(const ViolationInfo& infoViolation, RECORD_PLATE &plate);

		//合成前后车牌匹配和特写图
	bool GetVtsImageMatchForTianJin(
		RECORD_PLATE_DSP_MATCH &record_plate, 
		RECORD_PLATE_DSP_MATCH &foundRecord);

	//存储前后车牌匹配图像
	void SaveVtsImageMatchForTianJin(
		RECORD_PLATE& plate,
		int nPicCount,
		PLATEPOSITION* TimeStamp,
		PLATEPOSITION& SignalTimeStamp,
		int64_t redLightStartTime);

#ifdef TWICE_DETECT
	//初始化二次识别
	bool InitBusVioFilter();
	//将检测到的车牌位置画出来
	void DrawVioFilter(const RECORD_PLATE &plate);
	//在图像上,画矩形
	void myRectangle(IplImage *pImg, const CvRect &rt, const CvScalar &color, const int &thickness);
#endif

	//合成违章记录
	void SetVtsInfo(
		const RECORD_PLATE_DSP_MATCH &record_plate, 
		const RECORD_PLATE_DSP_MATCH &foundRecord, 
		RECORD_PLATE& plate,
		ViolationInfo &infoViolation,
		PLATEPOSITION* pTimeStamp);

	//合成未匹配违章记录
	void SetVtsInfoNoMatch(
		const RECORD_PLATE_DSP_MATCH &record_plate, 
		RECORD_PLATE& plate,
		ViolationInfo &infoViolation,
		PLATEPOSITION* pTimeStamp);

	//合成前后车牌匹配和特写图
	bool SetVtsImage2x3(
		const RECORD_PLATE_DSP_MATCH &record_plate, 
		const RECORD_PLATE_DSP_MATCH &foundRecord,
		const bool bIsReSize = false);
	//合成违章叠字	
	void SetVtsText2x3(		
		IplImage * pImage,
		RECORD_PLATE  &plate,
		const ViolationInfo &vioInfo,
		PLATEPOSITION* pTimeStamp,
		const RECORD_PLATE_DSP_MATCH &record_plate,
		const RECORD_PLATE_DSP_MATCH &foundRecord,
		const bool bIsReSize = false);
	
	//合成前后车牌匹配和特写图
	bool SetVtsImage2x2(
		const RECORD_PLATE_DSP_MATCH &record_plate, 
		const RECORD_PLATE_DSP_MATCH &foundRecord);
	
	//合成前后车牌匹配和特写图
	bool SetVtsImage1x3(
		const RECORD_PLATE_DSP_MATCH &record_plate, 
		const RECORD_PLATE_DSP_MATCH &foundRecord);

	//合成单张卡口
	bool SetKaKouImage1x2(const RECORD_PLATE_DSP_MATCH &plate);

	//合成未匹配违章2*2
	bool SetVtsImageNoMatch2x2(const RECORD_PLATE_DSP_MATCH &record_plate,const bool bIsReSize = false);
	//合成未匹配违章叠字
	void SetVtsTextNoMatch2x2(
		IplImage * pImage,
		RECORD_PLATE  &plate,
		const ViolationInfo &vioInfo,
		PLATEPOSITION* pTimeStamp,
		const RECORD_PLATE_DSP_MATCH &record_plate,
		const bool bIsReSize = false);
	//合成违章叠字
	void SetVtsText2x2(
		IplImage * pImage,
		const RECORD_PLATE  &plate,
		const ViolationInfo &vioInfo,
		PLATEPOSITION* pTimeStamp,
		const RECORD_PLATE_DSP_MATCH &record_plate,
		const RECORD_PLATE_DSP_MATCH &foundRecord);

	//合成卡口叠字
	void SetKaKouText1x3(
		IplImage * pImage,
		const RECORD_PLATE  &plate,
		const ViolationInfo &vioInfo,
		PLATEPOSITION* pTimeStamp,
		const RECORD_PLATE_DSP_MATCH &record_plate,
		const RECORD_PLATE_DSP_MATCH &foundRecord);
	//合成单张卡口叠字
	void SetKaKouText1x2(
		IplImage * pImage,
		PLATEPOSITION* pTimeStamp,
		const RECORD_PLATE_DSP_MATCH &plate);
	//获取叠字内容
	std::string GetVtsText(IplImage * pImage,
		const RECORD_PLATE  &plate, 
		const ViolationInfo &vioInfo, 
		const int nRow, 
		const int nCol,
		const int64_t ts,
		const std::string &strDeviceId,
		const std::string &strPlace,
		const bool bIsReSize = false);
	//获取叠字内容卡口1x3
	void GetKaKouText(
		IplImage * pImage,
		const RECORD_PLATE  &plate, 
		const int nRow, 
		const int nCol,
		const int64_t ts,
		const std::string &strDeviceId,
		const std::string &strPlace);
	//获取叠字位置
	void GetTextPos(int &nWidth, int &nHeight, const int nRow, const int nCol,const bool bIsReSize = false,unsigned int ExtentHeight = 0);
	
	//设置车牌记录
	void SetCarInfo(const RECORD_PLATE_DSP_MATCH &record_plate, CarInfo  &carnums);	

	//处理一张图片
	bool DealComposeImg(IplImage* pImgDeal, CvRect &rtSrc, IplImage* pImgDest, CvRect &rtDst);
	//处理特写图片
	bool DealComposeImgTeXie(IplImage* pImgDeal, CvRect &rtSrc, IplImage* pImgDest, CvRect &rtDst);
	//存储违章图像
	void SaveComposeVtsImageMatch(RECORD_PLATE& plate,IplImage* pImgSave = NULL);
	//获取车身位置
	void GetCarRectFromPlate(const RECORD_PLATE &plate, CvRect &rtCarPos);

	//济南移动公交--获取路段名
	bool GetRoadName(double dLongitude,double dLatitude,sRoadNameInfo & RoadInfo);
	//求点到路段距离
	double DistanceOfPointToLine(double dLongitude,double dLatitude,sRoadNameInfo &RoadInfo);
	//求最短距离
	double GetMinDistance(double dLongitude,double dLatitude,RoadNameInfoList &RoadInfoList);
	double DistanceOfPointAndLine(double X,double Y, double AX,double AY,double BX,double BY); 

	//存储卡口图像(3*1)
	void SaveComposeKaKouImageMatch(RECORD_PLATE& plate);
	//存储未匹配的违章图片
	void SaveComposeVtsImageNoMatch(RECORD_PLATE& plate,IplImage* pImgSave = NULL);
//#ifdef MATCH_LIU_YANG
	//解码图片
	bool SetVtsImgMatch(RECORD_PLATE_DSP_MATCH &plateMatch, bool bType);

	//解码需要比对的jpg图片
	bool CDspDataProcess::DecodeJpgMatch(IplImage **ppImg, const string &strPic);
//#endif

	//解码jpg图片
	bool DecodeJpg(IplImage *pImg, const string &strPic);
		
	//车型检测
	void DetectTruck(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context);
	//车身颜色检测
	void CarColorDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context);
	//车标检测
	void CarLabelDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context);

#ifdef REDADJUST
	//红绿灯增强
	void RedLightAdjust(int nChannel, IplImage* pImage);
	void RedLightAdjustMatch(
		RECORD_PLATE_DSP_MATCH &record_plate);
#endif	

	//更新通道记录标记
	bool UpdateImgKeyByChannelID(const int nChannelId, const UINT64 &uKey, const int &bState);
	static void *DoPopMatchData(void* lpParam);
	//处理比对线程函数
	static void * DoDealPlate(void* lpParam);
	//处理未比对线程函数
	static void * DoDealNoMatchPlate(void* lpParam);
public:
	//初始化线程
	int DoPopData();

	//获取违章代码
	bool GetVtsCode(const UINT32 uViolationType, UINT32 &uVtsCode, std::string &strViolationType);
//私有变量
private:

    //输出结果互斥
    pthread_mutex_t m_OutPutMutex;
	//jpg互斥锁
	//pthread_mutex_t m_JpgFrameMutex;


	//线程结束标志
	bool m_bEndDetect;
	//输出结果线程ID
    pthread_t m_nOutPutThreadId;

	//车牌检测是否重新读取配置
	bool m_bReloadROI;

	//检测方向
	int m_nDetectDirection;


    //目标检测帧图象
	IplImage* m_imgSnap;
	//上一帧图象
	IplImage* m_imgPreSnap;
	//存储检出图像
	IplImage* m_imgDestSnap;

	//闯红灯合成图像（3帧）
	IplImage* m_imgComposeSnap;
	//合成图像需要的单帧图片
	IplImage* m_imgComposeResult;

	//合成遮挡拼图
	IplImage* m_imgComposeMatch;

	//卡口3*1合图
	IplImage* m_imgComposeMatchKaKou;

	//单张卡口
	IplImage* m_imgComposeNoMatch;

	//未匹配的违章合图
	IplImage* m_imgComposeVtsNoMatch;
#ifdef TWICE_DETECT
	//定义对象
	MvJudgeOnBusVioValid m_OnBusVioFilter;

	//违章3张图
	IplImage* m_imgVtsElem1;//第1张卡口图
	IplImage* m_imgVtsElem2;//第2张过程图
	IplImage* m_imgVtsElem3;//第3张过程图

	vector<CvRect> m_vecRet1;
	vector<CvRect> m_vecRet2;
	vector<CvRect> m_vecRet3;
#endif

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


	//目标检测坐标列表
	vector<ChannelRegion> m_vtsObjectRegion;
	//闯红灯检测参数列表
    VTSParaMap  m_vtsObjectParaMap;

    //违章检测全局参数
    VTS_GLOBAL_PARAMETER m_vtsGlobalPara;

    //通道编号
	int m_nChannelId;
	//经过地点
	std::string m_strLocation;
	//行驶方向
	int m_nDirection;
	//相机编号
	int m_nCameraID;
	//相机型号
	int m_nCameraType;
	//检测类型
	CHANNEL_DETECT_KIND m_nDetectKind;

	//图像文本信息
	//CvxText m_cvText;
	//
	//CvxText m_cvBigText;
	//文本区域高度
	int m_nExtentHeight;


	//统计时间长度
	int m_nTrafficStatTime;
    ////////////////////////////////////////////////////

	//是否推送实时车牌
	bool m_bConnect;

	//白天还是晚上
    int m_nDayNight;
	//白天还是晚上(图像增强用，同开关灯时间)
    int m_nDayNightbyLight;
    //检测时间类别
    CHANNEL_DETECT_TIME m_nDetectTime;

    CarLabel m_carLabel;//车标
    ColorRecognisize m_carColor; //颜色

    //存图数量
    int m_nSaveImageCount;
    //是否存小图
	int m_nSmallPic;
	//文字位置
	int m_nWordPos;
	//字叠加在图片上
	int m_nWordOnPic;

	//关键字为帧号和相机编号组成的结构体的JPG大图队列(检测器为服务端)
	//map<Picture_Key,string> m_ServerJpgFrameMap;
	//dsp车牌处理
	std::multimap<int64_t,CarInfo> m_mapInPutResult;
	//关键字为帧号和相机编号组成的结构体的dsp车牌处理(检测器为服务端)//add by wantao
	std::multimap<Picture_Key,CarInfo> m_ServermapInPutResult;
	//违章检测输出结果向量
	std::vector<ViolationInfo> m_vtsResult;
	
	bool m_bEndThread;

	int m_nWidth;
	int m_nHeight;

	//防伪码
	int m_nRandCode[3];

//#ifdef MATCH_LIU_YANG
	IPL_IMAGE_TAG m_imgTagList[MAX_IMG_MATCH_TAG_NUM];

	unsigned int	m_uPrevImgag;				//环形缓冲的前一次使用索引
	unsigned int	m_uCurrImgtag;				//环形缓冲的当前使用索引
	pthread_mutex_t m_ImgTagMutex;
//#endif
	

	mapRoadName * m_pMapRoadName;
	//MvFBMatch2 *m_mvMatch2; 
};

#endif //NOPLATE
#endif
#endif
