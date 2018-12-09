#ifndef _MV_EVENT_DETECT_H_
#define _MV_EVENT_DETECT_H_

#include "libHeader.h"
#include "comHeader.h"

#include "Detector.h"

//-------------目标普遍属性-----------//
typedef struct StruObjectUsualProperty
{ 
public:
	StruObjectUsualProperty() { mvInitVar( ); }

	void mvInitVar( );

public:
	CvPoint m_ptCet;

	CvSize  m_szCar;
	CvSize  m_szPeo;

	bool	m_bBigVehicle;

	CvPoint m_ptLtVehAroundExt;
	CvPoint m_ptRbVehAroundExt;

}MvObjectUsualProperty;


//--------目标区域结构体--------
typedef struct StruObjectAreaApp
{
	//获取车辆周围的区域(包含车辆和底部阴影)
	static bool mvGetVehicleAroundArea( 
		MvObjectUsualProperty &ObjUsualProperty,
		const MyGroup &vehicle, const CvSize &szImg, 
		const IplImage *pCarWImg, const IplImage *pCarHImg,
		double dTsNow );

}MvObjectAreaApp;


//--------压线报警结构体--------
typedef struct StruPressLineAlert
{
	//获取目标上次报警的时间戳
	static bool mvGetObjectLastAlertTimeStamp( 
			double &dTsObjPressLineAlert,
			MyGroup &vehicle, int nAlertTypeMod );

	//设置目标报警的时间戳
	static bool mvSetObjectAlertTimeStamp( MyGroup &vehicle, 
			int nAlertTypeMod, double dTsNow );

	//判断是否目标和线相交
	static bool mvIsObjectIntersectWithLines( 
			vector<int> &vctIntersectLinesIdx,
			MyGroup &vehicle, int nLineCnt, 
			CvPoint **pLinePt );

}MvPressLineAlert;

#endif