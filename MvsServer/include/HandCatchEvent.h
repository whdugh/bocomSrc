#ifndef HAND_CATCH_EVENT_H
#define HAND_CATCH_EVENT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <map>
#include <string>
using namespace std;

#include "cv.h"
#include "cxcore.h"
#include "Common.h"
#ifdef  HANDCATCHCAREVENT
#include "MvObjectStatus.h"
#endif

#ifndef ROADDETECT_OK
#define ROADDETECT_OK 0
#endif

typedef void (*TCatchCarEventCB)(int nCarId, int nContext);

typedef void (*TCatchCarEventDrawCB)(vector<RectObject>  vectRectObj, SRIP_DETECT_HEADER tDetectHeader , int nContext);

class CHandCatchEvent
{
public:
	CHandCatchEvent(int nCheckTime);
	~CHandCatchEvent();
	int SetCatchCarEvent(TCatchCarEventCB tEventCB,int nContext);
	int SetCatchCarEventDraw(TCatchCarEventDrawCB tDrawCB,int nContext);
	int DealFrame(std::string& frame);
	int SetObjRect(std::string& frame,CvRect rectDraw);
	int DelObjRect(int nCarId);
private:
	int GetTimeT();
private:

	int m_detectTime; //检测时间间隔 
#ifdef  HANDCATCHCAREVENT
	MvStopObjectStatusDetector m_CStopObjStatusDet;
#endif
	IplImage *m_pImgSrc;     //用于停车算法中的图片

	int m_detectTestTime;    //测试检测时间间隔 
	bool m_bDetectTestPic;
	TCatchCarEventCB m_tCatchCarEventCB;
	int m_nContext;


	TCatchCarEventDrawCB m_tDrawCB; //回显的回调
};



#endif