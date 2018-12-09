// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
// author: wyj!
// date: 2010—10—13

// MvRadarSpeed类功能:通过Radar测速的方式测量机动车速度，无法实现精确定位，
// 对于行人和非机动车则用视频速度！

#ifndef MV_RADAR_SPEED
#define MV_RADAR_SPEED

#include "Mode.h"
#include "SensedObject.h"
#include "CarManager.h"
#include "Car.h"
#include "cxcore.h"
#include <map>

class CarManager;
class Car;

class MvRadarSpeed
{
public:

	MvRadarSpeed();

	~MvRadarSpeed();

	void Init(MvFindTargetByVirtualLoop *ftgVlp, CarManager *carManager, VirtualLoop *vlp, CvRect detectionRgn, std::vector<int> vecRoadIndex, Time_Mode mode, int delay = 0);

	void Destroy();
	
	//保存当前雷达速度！
	void SetRadarSpeed(unsigned int frameSeq, int64 timeStamp, double speed, double fastSpeed);

	//接收雷达速度加入speed map！
	void UpdateRadarSpeedList();

	//根据speed map计算car速度！
	bool CalcSpeed(void *pObj, int objType, double &retSpd);

	//判断当前雷达速度是不是这辆车子的速度，还是旁边车道车子的干扰！
	bool IsRadarSpdForThisCar(Car *pCar,  FILE *pFile, double videoSpd, double radarSpd);

	//测试在检测区域内找车牌！后来添加*****机动车目标没有同步更新，所以以后测试需要注意！
	void TestIsHaveCarInDetectRgn(const IplImage *imgSrc, unsigned int frameSeq);

private:

	bool _CalcCarSpeed(const Car *pCar, double &retSpd); 
	bool _CalcSensedObjSpeed(SensedObject *pSensedObj, double &retSpd);

	//确定车牌检测区域是否有Car对象，若有，则接收雷达传来的速度更新速度map！
	bool _IsHaveCarInDetectionRgn();
	bool _IsHaveVlpCarInDetectionRgn();

	//坐标转换，原图->Check图!
	void _OriCoordinate2CheckCoordinate(const CvRect rectSrc, CvRect &rectDst);
	void _Vlp2Original(CvRect &rectDst);

private:

	unsigned int m_frameSeq;
	int64 m_timeStamp;
	double m_dSpeed;
	std::map<unsigned int, double> m_mapRadarSpeed;  //speed map!
	CarManager *m_pCarManager;
	MvFindTargetByVirtualLoop * m_pFtgVlp;
	VirtualLoop *m_pVlp;
	
	std::vector<ChannelRegion> m_vecChannelRegion;
	std::vector<int> m_vecChannelIndex;  //雷达覆盖的车道索引！


	CvRect m_rectPlateDetection;  //车牌检测区域！
	CvRect m_rectOverlap;  //车牌检测区域和雷达感应重合区域！
	int m_nDelay;
	Time_Mode m_nMode;

	FILE *m_pRadarDataFILE;
	FILE *m_pFILERadarSpdmapValue;
};
#endif