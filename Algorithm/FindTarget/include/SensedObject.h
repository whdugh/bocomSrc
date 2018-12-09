// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef SENSEDOBJCT_H
#define SENSEDOBJCT_H

#include <vector>
#include <cxcore.h>

#include "StdAfx.h"
#include "ObjectColor.h"
//#include "MvLineSegment.h"
#include "SynCharData.h"
#include "CarInfo.h"
#include "MvChannelRegion.h"
//#include "MvGroup.h"
#include "Mode.h"



//using namespace std;
//#include<map>

struct _MvGroup;
class Car;
class VirtualLoop;
class VehicleFlowHelper;
class OpticalFlowHelper;

typedef struct _TrackIndexGrop
{
	vector<int> TrackIdex;
	CvRect  FlowRec;
	_TrackIndexGrop()
	{
		FlowRec =cvRect(0,0,1,1);
	}
}MvTrackIndexGrop;


#define MAXLIGSUM  7

typedef struct _SenesedObjectTrackData
{
	unsigned int  uTimestamp;//for yufeng
	unsigned int  frameNo;
	CvRect        pos;
	CvRect        LastByFlowPos;//相对vlp，没压缩的坐标
	int64         ts;
	//int           imgIndex;  //for yufeng
	float         dflow;

	// 颜色及颜色可信度。
	object_color  objColor;
	float         fObjColorCredit;
	
	float         fSpeedX;
	float         fSpeedY;

	CvRect        headPos;

	// 为该轨迹做贡献的，如果没有，则全0
	CvRect        flowRect;
	float         fMovLen;
	int           nTrackNum;
	CvPoint       CarPlatCen;


	std::vector<unsigned short> colorHist;


	_SenesedObjectTrackData()
	{
		//bTrustable = false;
		dflow   = 0.0f;
		fSpeedX = 0.0f;
		fSpeedY = 0.0f;
		objColor.nColor1 = 0;
		flowRect = cvRect(0, 0, 0, 0);
		LastByFlowPos = cvRect(0, 0, 0, 0);
		uTimestamp = 0;
		ts = 0;
		fMovLen =0.0;
		nTrackNum = 0;
		CarPlatCen = cvPoint(0,0);
	}


} SensedObjectTrackData;


typedef struct _BusTopInf
 {
	
	 bool TopStart;
	 double TopCol;
	 bool bAwayVlp;
	 _BusTopInf()
	 {
		 TopCol = 0;
		 TopStart = false;
		 bAwayVlp = false;
	 }
	 

}BusTopInf;

enum ObjType
{
	UNKONOWN, NONVEICHLE, CAR, BUS
};


class SensedObject
{

public:

	#ifdef DO_DONGS_CODE

		// 晚上的大车尾。 -1未计算。0不是。1是。 2不确定。
		int m_nBigCarTailN;

		//是否是大车的车尾。-1未计算。0不是。1是。 2不确定。
		int m_nBigCarTail;

		// 是否是边上的大车。-1未计算。0不是。1是。
		int m_nSideBigVeichle;

		//晚上中间大车想断被阻止的流量
		float m_fFAN;

		//中间大车剩余的流量
		float m_fFBN;

		//中间大车想断被阻止时的流量
		float m_fFA;

		//中间大车剩余的流量
		float m_fFB;

		// 边上大车想断但被阻止时刻的流量。
		float m_fSiderBigVeichleFlowA;

		// 边上大车想断但被阻止时刻计算出的想断该车还应该累计的流量。
		float m_fSideBigVeichleLeftFlow;

	#endif
	int          m_nSerZeroFlow;
	bool         m_bPark;
	bool         m_CarSpeedCut;
    bool         m_CarNearBus;

    bool         m_bReltionWithBehind;

    CvRect        m_FiStCarVlprect;

	bool          m_bCarVary;
	BusTopInf     m_BusTopinf;
	int           m_nShadowNum;
	vector <CvRect>  m_forSortRgn;
	bool       m_bPermiAssocCar;

	int m_nTrackSize;
	bool m_ArodExObj;//夜间的
	bool m_bLap;
	bool m_bNearObj;
	bool m_bNearCar;
	int  m_CutbyFlow;
	int  m_nForType;

	MvTrackIndexGrop TrackInf;

	bool m_bBus;
	int  m_nFlowNum;
	int  m_nAppearNum;


	// 目标头位置，相对于原图。
	CvRect m_rectHeadPos;

	//object_color color;//张color
	bool   m_bBelongsToPreviousObj;
	//int    m_nCenter;           //中心
	//int    m_nWidth;            //宽度
	//CvRect   m_rectMostLikelyPos; //obj在vlp上最有可能的位置。每次添加新的轨迹时更新。

	double fObjTotalFlow;//流量
	
	int    id;
	Car*   AssocCar;

	int   m_LigApearSum[MAXLIGSUM];
	int   m_nLastlig;
	int   m_nSerNoLig;
	bool  m_bicyle;
    CvRect  m_MaxWidRect;
	float   m_Angle;

	float m_fSpdX;
	float m_fSpdY;
	int  m_nObjWidth;
	int  m_nCArWidNum;

	// 根据HOG判车型。现在只用于判是不是非机。
	int      m_nVehicleTypeByHog;//-2都不是，-1未计算，0非机，1大车，2小车。


	// 白天用直线判断车型。-2都不是，-1未计算，0非机，1大车，2小车
	int      m_nVeichleTypeByLines;

	// 白天用直线判断车型。-2都不是，-1未计算，0非机，1大车，2小车
	int      m_nVehicleTypeByLinesWeekModel;

	
	// 晚间用车灯判断车型。-2判不出、-1未判、1大车、2小车。
	// 只在车牌出现时判断一次。
	int      m_nVeichleTypeByHeadLight;

	float    m_aColorMean[3];
	float    m_aColorVariance[3];

	/*
	表示该obj不能再被更新轨迹
	*/
	bool isSealed;


	// 直线判车型时得到的车辆左右位置。
	int           vt_x1;
	int           vt_x2;

    //
	// 工作模式，夜间or白天。1白天，0夜晚
	bool    ObjSenDay;

    bool          IsBusPart(); 
	void          GetObjAvgSpeed(float &vx, float &vy) const;
	
	bool          IsBelongsToPreviousObj();
	void          SetBelongsToPreviousObj();

	int           GetRoadIndex(const std::vector<ChannelRegion> &channelOri, CvRect vlpRegion) const;

	bool          IsDetectedInTimePeriod(int64 from, int64 to) const;


	std::vector<SensedObjectTrackData> track;


	// 用于过滤图像两边机动车。
	bool          m_bSideMotorized;

	int          m_Dirction;


	//用于过滤*****目标多报！
	bool          m_bFirstOutput;
	int64         m_firstTime;
	bool          m_bOverTime;  //等待后一辆车子时间是否超时！
	bool          m_bLongTimeZeroFlowSeal;  //标记*******对象是不是由长时间零流量断的，用于抑制无牌车或*******多报！
	bool		  m_bReviseCarNumSeal;  //标记*******对象是不是由ReviseCarNum断的，用于抑制无牌车多报！

	bool          m_bAssocCarHis;  //目前每个Car只能关联一个SensedObject，就可能造成多报，因此加一个判断是否关联过车子的变量，以抑制多报！

	bool          m_bAwayVlp;
	bool          m_AssocCut;
	bool          m_bMayAwayVlp;
	unsigned int  m_nBusAwayfmq;
	int           m_BicyWidSum;

   
public:


	SensedObject(VirtualLoop* vlp);
	~SensedObject();

	void          NotifyDelete() const;
	void          ClearAssocCar();
	Car*          GetAssocCar() const;

	int           GetTrackCount() const;
	void          SetAssocCar(Car* pCar);
	int           GetMosLikyLig()const;
	void          SetAssocCarHis()
	{
		m_bAssocCarHis = true;
	}

	bool          GetCarMayAwayStat()const
	{
		 return m_bMayAwayVlp;
	}

	void          SetCarMayAwayStat()
	{
		m_bMayAwayVlp = true;
	}

	bool          GetCarAssocCutSate() const
	{
		return    m_AssocCut;
	}

	void          SetCarAssocCutSate() 
	{
		         m_AssocCut = true;
	}
	bool          GetCarAwaySate() const
	{
		return    m_bAwayVlp;
	}

	void          SetCarAwaySate()
	{
		   m_bAwayVlp = true;
	}

	bool          GetAssocCarHis() const
	{
    	return m_bAssocCarHis;
	}

	object_color  GetMostLikelyColor(std::vector<unsigned short> &uColorResult) const;

	void          AppendTrack(const SensedObjectTrackData &track);

	// 用于相机同步
	void          GetSyncData(ObjSyncData &objSyncData ) const;

	
	//void MergeSensedObj();

	//根据流量确定行驶方向！
	int			  JudgeDirectionByFlow();

	// 在轨迹里找适合作为输出的记录，防止返回的帧已经不在应用程序缓存中
	bool          GetOutputTrack(unsigned int uCurFrameSeq, SensedObjectTrackData &ret) const;

	// 在轨迹里找适合作为输出的记录，防止返回的帧已经不在应用程序缓存中
	void          GetOutputTrack (SensedObjectTrackData &ret) const;
	//爆闪时取图！
	void		  GetOutputTrackForShutter(unsigned int uBrightFrmSeq, SensedObjectTrackData &ret) const;
	bool          GetReported() const;
	void          SetReported();
	bool          GetNeedReport(unsigned int nFrameSeq, VehicleFlowHelper *pFlowHelper,bool bShut =false) ;
	void          SetNeedReport();
	unsigned int  GetLastUpdateFrameSeq() const;
	int64         GetLastUpdateTimestamp() const;

	
	// 根据宽度计算目标类型。
	ObjType       GetObjType() const;

	//　综合直线、HOG、宽度、灯光、车牌、判断车型。
	ObjType       GetObjType2(Time_Mode mode) const;


	CvRect        GetFirstTrackRect() const;

	int           GetSensedObjWidth() const;

	CvRect        GetMostWideRect() const;

	//void          CombineAnother(SensedObject *pObj);


	int64         GetFirstTrackTs() const;

	int64         GetLastTrackTs() const;


	// 计算轨迹宽度均值。
	float         GetTrackWidthMean() const;

	// 计算轨迹宽度的方差。
	float         GetTrackWidthVariance(float fMean) const;

	// 用于判断目标是否有许多0流量记录使用。用于防止停车时每帧都做处理，浪费时间。这样只每隔一定帧数才判断。
	unsigned int uLastFrameSeqManyZeroFlowTrack;


	// 目标是否有许多0流量的记录。
	bool          IsObjectHaveManyFlowZeroTrack(int thresh, unsigned int uFrameSeq);

	// 取得bus当前压线的位置。
	bool          GetBusCurMostLikelyPos(CvRect &ret);
	

	//bool          TryCombineAnotherObj(SensedObject* pObjB);

	SensedObjectTrackData GetFirstTrack() const;
	SensedObjectTrackData GetLastTrack() const;
	SensedObjectTrackData GetFlowFirstTrack() const;
	SensedObjectTrackData GetVlpFlowLastTrack() const;

	// 获取最好反应sensedObject在虚拟线圈上位置的Rect
	bool          GetMostLikelyRect(CvRect &ret) const;

	CvRect        GetCodeRect() const;

	static  bool  IsTwoObjCloseByPos(const SensedObject& obj1, const SensedObject &obj2);
	
	void          SetTimeOutDrop();
	bool          GetTimeOutDrop();

	void          WriteObject(std::ofstream &ofs);


	bool          IsColorChanged(IplImage *imgCheck);

	bool          GetIsWaitColorChangeSeal() const;

	void          SetWaitColorChangeSeal();


	void          SetWaitColorChangeSealRgn(const std::vector<CvRect> &rgn, IplImage* img);

	void          DrawWaitColorChangeSealRgn( IplImage *img , CvScalar color);

	bool          GetFlowRectMeanVar(int nCount, CvRect &mean, float &var1, float &var2, float &var3, float &var4);


	bool          GetLastFlowRect(CvRect &rect) const;

	// 将每一帧的flowRect画到图上。
	void          DrawFlowRectHistory( IplImage* img, int nCount, float sx = 1.0, float sy = 1.0 );
	bool          IsGettingWide() const;

public:

	_MvGroup*     GetAssocGroup() const
	{
		return AssocGroup;
	}

	void          SetAssocGroup(_MvGroup *group)
	{
		AssocGroup = group;
	}


    bool          m_bTrackLRSim;
	int           m_nAwaCarRioNum;
	bool          m_bVlpLow;
    bool          m_DriCom;  
	int           m_nType;//-1未知，0非机动车，1机动车，2是中型车，3大车
	bool          m_bTochBtm;//判断是否接触底部
	unsigned int  m_TochBotmFram;
	bool          m_bFake;
	bool          m_bParkIng;
	int  m_nCodeIndex;
	int  m_nTempCodeIndex;
	int m_nChanel;
	int m_nChanDir;
	int           m_nLigNum;
	vector<int> m_LastTrack;
	

private:	

	CvRect        CalcMostLikelyRect() const;
	
	// 标记目标是不是处于等颜色变化就断的状态。
	bool          m_bWaitColorChangeSeal;

	std::vector<CvRect> m_vecStatColorRgn;

	//
	std::vector<float>   m_vecColorRed;

	//
	std::vector<float>   m_vecColorGreen;

	//
	std::vector<float>   m_vecColorBlue;

	std::vector<float>   m_vecColorHue;
	std::vector<float>   m_vecColorSat;
	std::vector<float>   m_vecColorLig;

	bool          m_bNeedReport;

	bool          m_bReported;

	bool          m_bTimeOutDrop;

	static int m_nIdCount;	

	VirtualLoop* m_pVlp;


	OpticalFlowHelper *m_pOpticlFlowHer;

private:
	_MvGroup *AssocGroup;
public:
		unsigned  int nBrigShutFrem; 
		int64  m_nSerStrat;
		int64  m_nSerEnd;

};

#endif
