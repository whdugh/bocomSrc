
//车辆底部定位
#ifndef _AN_VEHICLE_BOTTOM_LOCATE_H_
#define _AN_VEHICLE_BOTTOM_LOCATE_H_

#include "libHeader.h"

#include "ForeImgApp.h"

#include "comHeader.h"  //放最后




//-------------------------------------------------------------//
//用于对给定区域进行车辆底部定位的结构体
typedef struct StruVehicleBottomLocate
{
public:
	StruVehicleBottomLocate( );
	~StruVehicleBottomLocate( );

	void mvInitVar( );

public:
	static bool mvGetCandiateShadowOfVehicleBottom( 
		IplImage *pCandiatShadowImg, const CvRect &rctJudge,
		IplImage *pGrayImg, IplImage *pBgImg, IplImage *pFkImg );

private:
	IplImage *m_pCarWImg, *m_pCarHImg;

}MvVehBotLocate;



#endif