// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


#include "cxcore.h"

// 根据统计数据判断图像左边是不是非机车道（前牌），或者右边是不是非机车道（尾牌）。
class RoadTypeDetector
{
public:
	// 前牌nDirection=0, 尾牌nDirection=1
	RoadTypeDetector(int nDirection, int nImageWidth, int nImageHeight);

	// 0不是， 1是，-1未知
	int IsRightNonMotorizedVehicleRoad();

	// 0不是， 1是，-1未知
	int IsLeftNonMotorizedVehicleRoad();


	void Update(CvRect rgn);



private:

	int m_nDirection;
	int m_nLeftCount;
	int m_nRightCount;

	int m_nImageWidth;
	int m_nImageHeight;
};