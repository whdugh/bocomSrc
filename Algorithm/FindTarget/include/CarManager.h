// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef CARMANAGER_H
#define CARMANAGER_H

#include <map>
#include <vector>

#include "Car.h"
#include "Calibration.h"
#include "SynCharData.h"
#include "MvFindTargetByVirtualLoop.h"
#include "MvTrack.h"
#include "MvSurf.h"

//class Car;
class MvFindTargetByVirtualLoop;

class CarManager
{
public:

	CarManager(MvFindTargetByVirtualLoop *pFtg, MvSurf *pSurf);

	~CarManager();
	
	std::vector<Car*> lastFrameCars;

	// 按时间顺序，先检测出来的车牌放在前面。后检测的放在后面。
	std::vector<Car*> cars;


	void UpdateCar(const std::vector<CarInfo>& vecCarNums, unsigned int frameSeq, FindTargetElePolice::MyCalibration *pCalib, const std::vector<MvTrack*> &tracks, IplImage *previousImage[], int64 *lastTimeStamp, unsigned int *lastFrm);

	


	void Delete(int64 timestamp);

	//void Output(int64 timestamp, unsigned int frameSeq, std::vector<CarInfo> &output, std::vector<ObjSyncData> &vecObjSyncData);

	void SetDirection(int nDirection=0);//direction: 0由远到近。1由近到远 

	Car* GetCarById(unsigned int id) const;

	// 将pSrc合并到pDest，删除pSrc
	//void MergeCars(Car *pDest, Car *pSrc);

	//void ClearAll();
	void WriteCarsToFile(unsigned int uFrameSeq);

	// 在特定的区域，特定的范围内搜车牌记录个数。
	int CountCars(std::vector<Car*>::const_iterator from, int nLeftPos, int nRightPos);

	// 删除不合理的车牌，如果车牌检出次数很少，并且有另外一个车牌将他夹在中间，则不合理。
	void RmUnresonableCars(int64 ts);


	//bool HasBigVehAfter(int64 ts);

	//得到距某一帧最近的车子！
	std::vector<Car *> GetCarsNearFrame(unsigned int uFrame);

private:
	Car* AddCar(CarInfo cn, MyCalibration *pCali);

	// 卡口，电警用
	void _UpdateCar(const std::vector<CarInfo>& vecCarNums, unsigned int frameSeq, FindTargetElePolice::MyCalibration *pCalib, const std::vector<MvTrack*> &tracks, IplImage *previousImg[], int64 *lastTimeStamp, unsigned int *lastFrm);

	// 车载违章检测用
	void _UpdateCarOnBusVioDet(const std::vector<CarInfo>& vecCarNums, unsigned int frameSeq, FindTargetElePolice::MyCalibration *pCalib, const std::vector<MvTrack*> &tracks);


	// CarId到pCar的map
	std::map<unsigned int, Car*> mapCarIdToPointer;
	
	//direction: 0由远到近。1由近到远 
	int m_nDirection;

	MvFindTargetByVirtualLoop* m_pFtg;
	
	MvSurf *m_pSurf;


#ifdef DES_ROAD_STAT
public:
	void mvDesCarRoadState();
#endif
};

#endif
