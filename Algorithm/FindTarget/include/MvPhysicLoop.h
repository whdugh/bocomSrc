// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010/03/13
	created:	13:3:2010   22:44
	filename: 	e:\BocomProjects\find_target_lib\include\MvPhysicLoop.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MvPhysicLoop
	file ext:	h
	author:		Dur
	
	purpose:	双线圈测速类（离相机近的一个线圈具有触发功能）
*********************************************************************/

#ifndef MV_PHYSIC_LOOP
#define MV_PHYSIC_LOOP


#include <fstream>
#include "MvLine.h"
#include "LoopObj.h"

//#define DEBUG_SAVE_TS_COUNTER_PAIR

struct _LoopObj;

#ifndef  MAXINT
#define MAXINT 65536
#endif



// 无论前牌尾牌，远的线圈为编号为0，近的线圈编号为1
typedef struct _LoopInfo
{
	int nRoadIndex;        // 车道编号
	MvLine line0;          // 编号为0的线圈在图像中的位置
	MvLine line1;          // 编号为1的线圈在图像中的位置。
	float  dis;            // 线圈距离（m）
}LoopInfo;



#define EAT 40              // counter <-> ts对记录的个数。

#define NEW_METHOD          // 支持长距离线圈（5m或更多）

class MvPhysicLoop
{
public:

	//
	MvPhysicLoop(const MvFindTargetByVirtualLoop *pFtg, int nRoadIndex, MvLine line0, MvLine line1, float fLineDis, int nDefaultDir = 1);

	//
	~MvPhysicLoop();

	//
	bool Process(bool b0, bool b1, int64 counter, int64 ts, _LoopObj **vli);

	//
	void SetDelay(int nDelay);

	// 
	int GetDelay() const;


	// 获取线圈的位置
	void GetLoopPos(MvLine &l0, MvLine &l1) const;

	// 获取两个线圈的之间的距离，一般3.5m，4m，5m
	float GetDistance() const;


	void SetTsCounterPair(const std::vector<int64> &ts, const std::vector<int64> & counter);

private:

	//
	void RecordCounterTimePair(int64 counter, int64 ts);

	//
	int64 GetErrorCorrectedTime(int64 counter);
	

private:

	// 定义内嵌类型VehicleLoopInfo
	typedef struct _VehicleLoopInfo
	{
		typedef enum _VehicleStatus
		{
			UNKNOWN_STATUS, PRESS_ONE, PRESS_TWO, LEAVE_ONE, LEAVE_TWO
		} VehicleStatus;


		int64 st0;                // 开始压上0的时间。单位us
		int64 st1;                // 开始压上1的时间
		int64 et0;                // 离开0的时间
		int64 et1;                // 离开1的时间。
		VehicleStatus status;     // 
		bool  dirFlag;            // 先压0还是先压1
		float spd;                // 速度，km/h
		float length;             // 车长。m
		int  nSerZero;            //持续00状态的个数
		int  nforwardSerZero;            //线圈00->10，01前状态目标持续00状态的个数
		int64 nTcounter0;          //开始压上0的时间。晶振单位一个工作周期
		int64 nTcounter1;          //开始压上1的时间。晶振单位一个工作周期
		      




		_VehicleLoopInfo()
		{
			st0        = -1;
			st1        = -1;
			et0        = -1;
			et1        = -1;
			status     = UNKNOWN_STATUS;
			dirFlag    = true;
			spd        = 0.0;
			length     = 0.0;
			nSerZero = 0;
			nforwardSerZero = MAXINT;
			nTcounter0 = 0 ;
			nTcounter1 = 0;

		}

	}VehicleLoopInfo;


private:

	int m_nDefaultDir;

	// 线圈的通道号
	int m_nRoadIndex;

	// 两条线圈在图像中的位置。
	MvLine m_line0;
	MvLine m_line1;

	// 两条线圈之间的物理距离（m）
	float  m_fLineDis;


     int  m_nSerzero;

	// 之前两个线圈的状态。
	bool m_lb0;
	bool m_lb1;

	// 之前收到的ts，counter对。防止重复处理
	int64 m_lts;
	int64 m_lcounter;


	// 是否已经得到初始数据。第一次数据不作进一步分析，只用来初始化。
	bool m_bRecFirstData;

	// 串口延时时间(us)
	int m_nDelay;

	// 计数器的值
	int64 m_counter[EAT];

	// 时间（us）
	int64 m_ts[EAT];

	int m_nNextPos;
	int m_nCount;


	// 拟合直线参数。
	double m_k;
	double m_b;

	// 是否需要重新拟合，防止多次用同样的数据拟合浪费时间。
	bool m_bNeedReCalc;
	map<int , bool> m_nRoadIndexUpdata;


	//
	_LoopObj* m_pLoopObj;


#ifdef NEW_METHOD
	int64 m_nLastLoop0; //上一次线圈0上升沿发生时刻
	int64 m_nLastLoop1;
#endif


	//为了准确判断线圈目标的初始方向，保存上一辆车的相关信息,Note：如果是多车道多线圈可能不适用！！！
	int m_lastVehDir;  //上一辆车行驶方向！-1——尾牌，0——未知，1——前牌！
	bool m_bPressTwo;  //是否同时压上两个线圈！
	bool m_bLastVehPressTwo;  

	VehicleLoopInfo *m_pVLI1;
	VehicleLoopInfo *m_pVLI2;



	#ifdef DEBUG_SAVE_TS_COUNTER_PAIR
		std::ofstream m_ofs;
	#endif

	int m_nRoadDir;  //道路方向！
	const MvFindTargetByVirtualLoop * m_pFtg;
};



#endif
