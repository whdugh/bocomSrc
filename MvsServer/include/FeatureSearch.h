// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef FEATURE_SEARCH_DETECT_H
#define FEATURE_SEARCH_DETECT_H

#define NOFEATURESEARCH
#ifndef NOFEATURESEARCH

#include "global.h"
#include "CommonSearch.h"
#include "Cpeoplesearch.h"
#include "CvxText.h"
#include "XmlParaUtil.h"

/******************************************************************************/
//	描述:特征提取实体类.
//	作者:於锋
//	日期:2011-5-26
/******************************************************************************/
typedef struct _FEATURE_SEARCH_HEADER
{
  short sMode;//提取方式，0代表使用标定提出，1代表使用目标框提取
  UINT32 uLeft;
  UINT32 uTop;
  UINT32 uWidth;
  UINT32 uHeight;
  _FEATURE_SEARCH_HEADER()
  {
	  sMode = 0;
	  uLeft = uTop = uHeight = uWidth = 0;
  }
}FeatureSearchHeader;

class CFeatureSearchDetect
{
public:
	//构造
	CFeatureSearchDetect();
	//析构
	~CFeatureSearchDetect();

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

	//设置是否重新配置
	inline void SetReloadConfig(){ m_bReloadConfig = true;}

	//设置相机编号
	void SetCameraID(int nCameraID);

	//设置通道地点
	void SetPlace(std::string location){m_strLocation = location;}

	//设置通道方向
	void SetDirection(int nDirection){m_nDirection = nDirection;}

	//设置视频类型(0:实时，1：历史)
	void SetVideoType(int nVideoType){ m_nVideoType = nVideoType;}
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

     //输出特征结果
	//void OutPutResult(feature* mfeature,SRIP_DETECT_HEADER* sDetectHeader, bool bIsSendPic=true);
	void OutPutResult(feature* mfeature,SRIP_DETECT_HEADER* sDetectHeader);

	//输出特征特征标定信息
	//void OutPutCalibration(CvRect& farrect,CvRect& nearrect,SRIP_DETECT_HEADER* sDetectHeader);

	//保存全景图像
    int EncodeImage(IplImage *pImage);

    //载入通道设置
    bool LoadRoadSettingInfo(vector<CvPoint>& v_iface_poly,CvRect& farrect,CvRect& nearrect,vector<CvPoint> &ptDirection);

	string GetHMS(const string& strTime,int nType);

	string GetDate(const string& strTime);

	//获取图片文件名称
	string GetPicFileName(const string& strTime);

	//叠加文本信息
	void PutTextOnImage(IplImage*pImage,SRIP_DETECT_HEADER* sDetectHeader);

	//叠加LOGO信息
	void PutLogoOnImage();

//私有变量
private:
	//信号互斥
	pthread_mutex_t m_Frame_Mutex;

	//线程ID
	pthread_t m_nThreadId;

	//待检测的帧列表
	std::list<std::string>	m_FrameList;
	//待检测的帧列表长度
	int m_nFrameSize;

	//经过地点
	std::string m_strLocation;

	//行驶方向
	int m_nDirection;

	IplImage *m_pImage;//当前处理图像
	IplImage *m_pImageFrame;//当前处理图像
	IplImage *m_pLogoImage;//LOGO图像

	//图像文本信息
	CvxText m_cvText;

	//x方向缩放比
    double m_fScaleX;
	//y方向缩放比
	double m_fScaleY;

	//场图像还是帧图像
	int m_nDeinterlace;


	//通道号
	int m_nChannelID;

	//相机编号
	int m_nCameraId;

	//是否停止线程
	bool m_bTerminateThread;

	//是否重读配置文件
	bool m_bReloadConfig;

	 //白天还是晚上
	 int m_nDayNight;

	 //JPG图像
     BYTE* m_pJpgImage;

	 //远处行人框
	 CvRect m_farrect;
	 //近处行人框
	 CvRect m_nearrect;

	 //视频模式（0：实时，1：历史）
	 int m_nVideoType;

     //特征提取接口
     MvPeopleSearch   *m_searchinterface;

	 int m_nInterval;//间隔保存图片

	 int m_lCount;//特征计数

	  XMLNode m_FeatureInfoNode;//特征数据

	  string m_strXmlResult;//特征数据

	  UINT32 m_uSeq;//计数器
};
#endif

#endif



