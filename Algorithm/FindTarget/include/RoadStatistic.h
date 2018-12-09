#ifndef ROAD_STATISTIC_H
#define ROAD_STATISTIC_H

#include <vector>
#include <map>
#include <list>
#include "StdAfx.h"
#include "CarInfo.h"
#include "MvChannelRegion.h"

//using namespace std;

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



class RoadStatistic
{
public:
	
	RoadStatistic(const std::vector<ChannelRegion> &vecRoadChannels);


	void GetStatisticData(int nRoadIndex, float &fAvgSpd, float &fVehicleDis, float &fRoadOccPer, float &fQueueLength, float &fFlux);

	void GetStatisticData(StatResultList &ret);


	void OnCarPlateDetected(const CarInfo *pCarInfo);


	void Reset();


private:

	// 车道
	std::vector<ChannelRegion> m_vecRoadChannels;


	std::map< int, std::vector<CarInfo> > m_mapPlateRecord;


	
};

#endif