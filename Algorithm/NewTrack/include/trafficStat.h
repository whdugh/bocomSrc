#ifndef _TRAFFIC_STAT_H_
#define _TRAFFIC_STAT_H_

#include "libHeader.h"

#include "BaseStruct.h"
#include "structdef.h"


//流量和速度
typedef struct _TraffFluVelStatStru
{	
public:
	_TraffFluVelStatStru( );

	void initVar( );

	//设置开始进行统计
	void SetBeginStat( double tsNow );  //单位为s	

	//设置结束统计
	void SetEndStat( double tsNow );	//单位为s

	//累加到统计结果中去
	void traffStatAdd( MyGroup *pObj );	

	//计算统计结果
	void traffStatCalc(  );

	//清理
	void traffClear( );

public:
	bool bStat;   //当前在统计

	//车道的统计周期(各车道可能不同)
	double ts_start;  //开始统计的时间戳
	double ts_end;    //结束统计的时间戳

	//针对天津电子警察所做流量和速度统计
	UINT32 uFluxAll;            //所有目标流量
	UINT32 uFluxVehicle;        //机动车流量
	UINT32 uFluxPerson;         //行人流量
	UINT32 uFluxNoneVehicle;    //非机动车流量

	UINT32 uFluxBigVeh;			//大车流量
	UINT32 uFluxSmlVeh;			//小车流量

	float  fAllVehSpeed;        //所有车的平均速度
	float  fBigVehSpeed;		//大车平均速度
	float  fSmlVehSpeed;		//小车平均速度
	float  fMaxSpeed;			//最大速度

	float  fTotalInterval;      //累计间隔时间
	float  fTotalPossessTime;   //累计占有时间

	bool   bPossess;            //车道是否被占有
	float  fPossessRate;        //车道占有率

}TraffFluVelStatStru;


typedef struct _TrafficStatStru
{	
public:
	_TrafficStatStru( );

public:
	//以下是统计配置
	bool m_bStatFlux;			//车道流量
	int  m_nStatFluxTime;		//车道流量检测间隔
	bool m_bStatSpeedAvg;		//平均车速
	bool m_bStatZyl;			//平均车道占有率
	bool m_bStatQueue;			//队列长度
	bool m_bStatCtjj;			//平均车头间距
	bool m_bStatCarType;		//车辆分型

}TrafficStatStru;




class CTrafficStat
{
public:
	CTrafficStat(void);
	~CTrafficStat(void);
	
	void mvInitTrafficStat( int nChannelNum,
			VEHICLE_PARAM_FOR_EVERY_FRAME *pParamDet );
	void mvUninitTrafficStat(  );

	//得到交通统计结果接口
	void mvGetTrafficStatResult(
			CvSet *track_set, CvSet *group_set,
			MyTrackElem*  pATrackAddress[],
			MyVehicleElem* pAGroupAddress[],
		    DetectResultList& detOutList );

private:
	//初始化交通统计变量
	void mvInitTrafficStatVar( );

	//得到交通统计的参数
	void mvGetTrafficStatParam( int nChannelNum,
			VEHICLE_PARAM_FOR_EVERY_FRAME *pParamDet,
			TrafficStatStru** ppTrafficStat );

public:
	int  m_nChannelNum;
	TrafficStatStru *m_pTrafficStat; //交通统计
	
};


#endif