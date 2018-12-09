// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef CAMERACONTROL_H
#define CAMERACONTROL_H

#include <string.h>
#include <iostream>


#define CARDETECTINTERVAL (45) //15分钟

#ifndef UINT32
	typedef unsigned int UINT32;
#endif

#ifndef REGIONCONTEXT
	#define REGIONCONTEXT
	//区域亮度信息
	typedef struct REGION_CONTEXT
	{
		double     mean;              //区域平均亮度
		double     stddev;				//区域方差
		double  carnummean;           //车牌平均亮度
		double  carnumstddev;         //车牌方差

		REGION_CONTEXT()
		{
			mean = 0;
			stddev = 0;
			carnummean = 0;
			carnumstddev = 0;
		}
	}region_context;
#endif

typedef struct _CAMERA_PARA
{
	int nSH;
	int nGain;
	int nEn;
	int nMaxSH;
	int nMaxGain;
	int nIris; //0(最小)-100(最大)
	_CAMERA_PARA()
	{
		nSH = 0;
		nGain = 0;
		nEn = 0;
		nMaxSH = 0;
		nMaxGain = 0;
		nIris = 0;
	}
}CAMERA_PARA;

enum EEN_STATE
{
	E_CLOSE = 0,
	E_ON
};

class CameraPara
{
	
public:
	CameraPara();

	void cameraPara_Init(int *sideTowards, int &isDayByLight, int &isDayByTime, int &nBgCardDiff,
								 int nCamKinds, int &imgWidth, int &backOrinfront, bool bIsFaceCrt=false );
	
	void mvGetCamera_Para(const bool bDetectCarnum, const region_context &context, const region_context &precontext,
							const bool &bIsSmear, CAMERA_PARA &camPara, float &fRate, float &fIncrement, 
								int &nIris, bool &bIsAdjust);
	void mvPolyfitCamera_Para(const region_context &context,float &fRate, float &fIncrement);

	
private:
	void mvGetEventCameraPara(region_context precontext, bool &bDetectCarnum, 
									  bool &bSmearNum, float &fRate, float &fIncrement);

	void mvGetInvLightCameraPara(float &diffLight, float &fRate, float &fIncrement, int nNoCardCount, 
									const float &stdThLowest, const float &stdThLow, const float &stdThMedian, 
										const float &stdThHigh, const int &minCL, const int &nCLThMedian, const int &nCLThLow);

	void mvGetTowardsLightCameraPara(float &diffLight, float &fRate, float &fIncrement, int nNoCardCount);

	void mvIrisControlCarNum(bool &bDetectCarnum, bool &bSmearNum, float &fRate, float &fIncrement, int &nIris);

	void mvIrisControlEvent(bool &bDetectCarnum, bool &bSmearNum, float &fRate, float &fIncrement, int &nIris);

	void mvRestrinctShGain(bool bDetectCarnum, float &fRate, float &fIncrement);

	bool mvAutoMinifyIris(const bool &bDetectCarnum, const bool &bSmearNum, float &fRate, float &fIncrement, int &nIris, int &nIal);

	void mvNormalLightUpdateShGain(float &fRate, float &fIncrement, float &diffLight, int nNoCardCount);

	void mvEventCameraControl( const region_context &precontext, bool bDetectCarnum, 
									bool bSmearNum, float &fRate, float &fIncrement, int &nIris);

	void mvCardDetectCameraControl(const region_context &precontext,  bool bDetectCarnum, bool bSmearNum, float &fRate, 
									float &fIncrement, int &nIris, bool &bIsAdjust);

	void mvBGLightControl(float &fRate, float &fIncrement);

	void mvCardLightControl(float &fRate, float &fIncrement);

	void mvZeroCardLight(float &diffLight,float &fRate, float &fIncrement);

	void mvGetRoadLightDiffCrt( const float &diffLight, float &fRate, float &fIncrement ) const;
	
private:
	int m_nIsDayByLight;//0为晚上，1为白天
	
	int m_nIsDayByTime;//0为晚上，1为白天
	
	int m_sideTowards[2];

	int m_bgCardDiff;

	int m_nTowardsOrInvLight;

	int m_camKinds;

	int m_imgWidth;

	int m_nBackOrInfront;

	int m_maxRoadLight;
	
	int m_minCardLight;

	int m_maxCardLight;

	region_context m_context;

	CAMERA_PARA m_camPara;

	int m_currentIris;//当前的光圈值 

	int m_bIsControlIris;

	int m_noCardCount;

	//bool m_led_on; //灯的状态

	bool m_bFirstRun;

	int m_nZeroCount;

	double m_dPreRoadLight;

	int m_lstSH;
	int m_lstGain;

	bool m_bIsAdjust;

	bool m_bFOpenFile;
	int m_nCLMinTh;
	int m_nInverseMinCL;
	int m_nMaxCL;
	int m_nCLThMedian;
	int m_nCLThLow; 
	float m_fStdThLowest;
	float m_fStdThLow;
	float m_fStdThMedian;
	float m_fStdThHigh;
	int m_nNightMinTh;
	int m_nMaxBgLight;
	int m_nMaxCLLight;

	bool m_bIsFaceCrt; //是否是人脸检测相机控制


private:
	enum{m_SH_TH = 1000, m_s_IRISTH_LOW = 80, m_s_IRISTH_HIGH = 100}; //快门中间阈值，光圈低阈值，光圈高阈值

	
};


#endif
