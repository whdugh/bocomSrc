// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   15:08
	filename: 	e:\BocomProjects\find_target_lib\include\VehicleFlowHelper.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	VehicleFlowHelper
	file ext:	h
	author:		Durong
	
	purpose:	用于目标检测，根据标定计算各类型车经过时流量的阈值。
*********************************************************************/


#ifndef VEHICLE_FLOW_HELPER_H
#define VEHICLE_FLOW_HELPER_H

#include "cxcore.h"
#include "Calibration.h"

using namespace PRJ_NAMESPACE_NAME;

class VehicleFlowHelper
{
public:
	VehicleFlowHelper();

	~VehicleFlowHelper();

	void Init(CvSize imgSize, CvRect vlpRoi, MyCalibration*pCalib);

	// 计算小车的正常断流量阈值
	float GetCarSealFlow(int nVlpPos) const;

	// 计算小车的最大阈值
	float GetCarMaxFlow() const;

	// 计算小车的最小阈值
	float GetCarMinFlow() const;

	// 计算非机动车正常应该具有的阈值。
	float GetNonvSealFlow(int nVlpPos) const;


	// 按照车辆在图像中占的大小，计算其流量。
	static float CalcFlowByPos(CvRect r, MyCalibration *pCalib);

	// 给定车辆的长宽高，以及中心点在图像中的位置，算出车辆在图像中的位置。
	static CvRect GetVehicleImagePos(CvPoint pt, MyCalibration* pCalib, int nVehType);


	// 生成一个小车的模型
	static void GetCarModel(float modelCarBottom[12], float modelCarMid[12], float modelCarTop[12]);

	// 生成一个非机动车的模型
	static void GetNonVModel(float modelBottom[12], float modelMid[12], float modelTop[12]);


private:

	// 存储了在vlp上从左到右小车应该具有的流量值（考虑高度）
	float *m_pCarFlow;
	
	// 
	float *m_pNonvFlow;


	int m_nVlpWidth;

};



#endif