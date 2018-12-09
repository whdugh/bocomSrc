//目标检测
#ifndef _MV_OBJECT_DETECTOR_H_
#define _MV_OBJECT_DETECTOR_H_

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <map>

#include "cv.h"
#include "highgui.h"
#include "cxcore.h"

#include "MvMathUtility.h"

#include "MatchTrack.h"
#include "ConstructGroup.h"

#include "ConfigStruct.h"

#include "Detector.h"

#include "comHeader.h"  //放最后

using namespace std;
using namespace MV_MATH_UTILITY;

typedef struct  _CHANNEL_OBJDETECT_PARAM
{
	int     nChannelID;         //车道ID
	int     nChannelMod;		//车道模型:0-机动车道,1-非机车道,2-混合车道
	int     m_nAngle;           //角度

	bool    bShow;              //是否为停留显示：1-停留显示事件,0-真实事件

	//----------------------事件检测配置------------------------//
	//检测目标	
	bool   bObjectAppear;        //是否检测目标出现
	int64  nShowTime;            //同一目标出现报警的显示时间(ms)
	int64  nSameObjAppearIgn;    //同一目标出现报警的忽略时间(ms)	
	int64  nSameEventAlertIgn;   //车道同类事件(目标出现)报警的忽略时间(ms)	

	int64 ts_appear_alert;        //该车道上次出现报警的时刻

	int  nObjeceDetectMod;    

	_CHANNEL_OBJDETECT_PARAM( )
	{
		nChannelID = 0;
		nChannelMod = 0;		   	
		m_nAngle = 0;
		bShow = false;            //真实事件

		//----------------------事件检测配置------------------------//
		bObjectAppear = false;   
		nShowTime = (int64) 30e3;             //同一目标报警显示15s
		nSameObjAppearIgn = (int64) 300e3;    //同一目标5分钟内只报一次
		nSameEventAlertIgn = 0;	        //该类事件一出现就报，不忽略

		ts_appear_alert = -1000000;       //该车道上次出现报警的时刻

		nObjeceDetectMod = DETECT_ALL_OBJECT;   
	}
}MVChannelObjDetParam;


class CObjectDetector /*: public CDetector*/
{

public:
	CObjectDetector(void);
	~CObjectDetector(void);

	//船检测
	bool mvDetectShips( DetectResultList& outList, 
			int nPtCnt, CvPoint2D32f *ptAObjDetPloy, IplImage *pShowImg=NULL );

private:
	//显示检测结果
	void mvShowDetectResult( int nPtCnt, CvPoint2D32f *ptAObjDetPloy, 
				DetectResultList objDectOutList, IplImage *pShowImg );

	void mvShowEllipseFiting( int nRandPt, CvPoint2D32f *p_ptRand,
							  CvSize imgSz=cvSize(200,150) );

private:
	MVChannelObjDetParam m_chanelObjDetParam;

};


#endif