// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include "mvGetBGLight.h"
#include "mvGetCameraControlPara.h"
//#include "Camerakinds.h"
//#include "DetectSmear.h"

#ifndef USE_DSP_CODE
//#define USE_DSP_CODE
#endif

#define DAY_MEAN_DIM (3)
#define NIGHT_MEAN_DIM (15)

class CameraParametter
{

public:
	CameraParametter(void);

	~CameraParametter(void);

	void cameraControl_Init(void);

	void cameraControl_Destroy(void);

	void mvGetBGLight( IplImage* image, CvRect detectRect,
					carnum_context* vehicle_Information, int nCarNumResult, int &lightTypes, int isDayByTime=C_DAY, bool bIsFaceCrt=false );

	void GetCameraControl_Para( UINT32 uTimeStamp, UINT32 m_detect_time, int interval, bool bDetectCarnum,
								CAMERA_PARA &camPara, float &fRate, float &fIncrement, int &nIris, int &isDayByLight, int &nEn );

#ifdef USE_DSP_CODE
	void mv_SetYdatetoIplImage( unsigned char *pYdate, int nWidth, int nHeight );
	IplImage *pYimage;	
#endif

protected:
	void mvComputePoliceLightMean(const int &nDim, const unsigned int &nCounts, bg_info *bgInfoHist,
									region_context &context, int &nNotZeroCounts);

	void mvCtrlLEDLight(const region_context &context, const CAMERA_PARA &camPara, const int &isDayByLight, 
		const int &isDayBytime, float &fRate, float &fIncrement, int &nEn);

private:
	BGLight m_Bglight; //类

	//DetectSmear m_SmearDetect; //类

	CameraPara controlPara;

	int m_width;

	bool m_iSInvokeBgLight;

	region_context m_context;

	region_context m_precontext;

	int m_carnum_count;

	int m_camKinds;

	bool m_bSmearNum;

	int m_IsBackOrInfront;

	bool m_bIsCloud;

	int m_colorMode; //颜色通道顺序

	bg_info *m_dayCarMeanInfo; //白天的车牌亮度直方图

	bg_info *m_nightCarMeanInfo; //晚上的车牌亮度直方图

	unsigned short m_nightInterval; //晚上调用相机控制次数

	unsigned short m_dayInterval; //白天调用相机控制次数

	float m_preRate; //前一帧的快门系数

	float m_preIncrement; //前一帧的增益系数

	int m_preDN; //前一帧的白天或者晚上

	unsigned short m_isDayCounts; //白天直方图像计数器计数

	unsigned short m_isNightCounts; //晚上直方图计数器计数

	int m_nCount; //用于统计增益大于一定值的次数

	bool m_bFirst; //第一次调用相机控制
	
	float m_preNotZeroRate; //每次相机控制循环内不是0的快门调节

	float m_preNotZeroInment; //每次相机控制循环内不是0的增益调节

	bool m_bIsFaceCrt; //是否是人脸检测相机控制
};











#endif