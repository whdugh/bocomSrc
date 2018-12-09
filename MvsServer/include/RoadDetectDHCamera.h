#ifndef ROAD_DETECT_DHCAMERA_H
#define ROAD_DETECT_DHCAMERA_H
/*************************
大华高速球版本停车严管
*************************/

#include <sys/time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "cv.h"
#include "cxcore.h"
#include "Common.h"
#include <string>
using namespace std;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (unsigned int)(~0)
#endif

#ifndef Roadmax
#define Roadmax(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef Roadmin
#define Roadmin(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef CAMERAAUTOCTRL
#include "CameraAutoCtrl.h"
#include "CamCtrlComCalc.h"
#include "OnvifCameraCtrl.h"
#endif

struct AutoCameraDHPtzTrack
{
	int nTaken;
	int nPanTiltX;
	int nPanTiltY;
	int nPanTiltZoom;
	int nBeginTime;
};

class CRoadDetectDHCamera
{
public:
	CRoadDetectDHCamera();
	~CRoadDetectDHCamera(void);
	//设置相机的一些技术参数（初始化时进行一次）
	void SetCameraVar(
		float fCCD_w,        //输入：CCD的宽度 
		float fCCD_h,        //输入：CCD的高度
		float fMinFocus,     //输入：最小焦距
		float fMaxFocus,     //输入：最大焦距
		int   nMultiPlicate  //输入：相机的倍率
		);
	//设置检测车辆位置坐标
	void DetectRegionRect(int Camerawidth,int Cameraheight,int xPos,int yPos,int width,int height);
	//开始控制云台移动及拉近操作
	int DealTrackAutoRect(string frame,int nCarId,int nSaveProcess);
	/*调用预置位操作*/
	bool GotoPreset(int uiPresetNum);
	int InputCameraAddr(const char *pCameraHost,int pCameraPort);

private:
	/*获取当前时间*/
	int GetTimeT();
	/*判断当前目标是否在整个画面中心，误差控制在1/20返回之内*/
	bool IsCentreRectResemble(CvRect rectNew);
	/*判断当前画面是否移动*/
	bool IsRectResemble(CvRect rectOld,CvRect rectNew);
	// 返回值 0表示无需放大 1表示可以继续放大 2表示为可以继续放大并加入车牌检测功能
	int CalculateRectPtzZoomEX(CvRect rectNew,float &fRectArea,float &fZoomValue);
	/*
	输入参数：当前目标位置 false表示当前为水平移动，true表示当前为放大操作
	当目前动作为水平移动时，判断当前目标的中心点距离画面的中心点大小，
	当小于1/20时，不做动作，当距离大于画面的1/4时，移动长时；小于1/4时，移动小时间
	*/
	bool CalculateRectAndMovePtzOrZoom(CvRect rectNew,int m_nCarId,int m_nSaveProcess,bool bPTZMove = true);

	int CalculateRectPtzEX(CvRect rectNew,float &fPanTiltX,float &fPanTiltY);

	int TrackAutoCtrl(int nCarId,int nSaveProcess);
	//保存相机移动轨迹
	int SaveCameraMoveTrack(int nCarID,int nX,int nY,int nZ);

private:
	CvRect m_RectInput;
	int m_CameraWidth;
	int m_Cameraheight;
	int m_iBeginAdjustRect; //标明目前是水平移动还是放大缩小操作
	bool m_nbMoveOperate; //标明目前是水平移动还是放大缩小操作
	int m_iMarkTime;
	bool m_bBeginDetect;
	bool m_bFristMark;
	IplImage *m_pImgSrc;     //用于停车算法中的图片
#ifdef CAMERAAUTOCTRL
	CMvCameraAutoCtrl *m_pMvCameraAutoCtrl;
	MvCalcCamMvScale  *m_pMvCalcCamMvScale;
	COnvifCameraCtrl  *m_pOnvifCameraCtrl;
#endif
	int m_iCameraMoveTime;

	int m_nErrCount; //目标丢失数，超过40个人为此次定位失败
	int m_nDetectNum; //0表示为第一次放大 1表示为第二次
	int  m_nCameraMoveCount;//云台移动次数 限制为4次
	float m_fScaleRealZoom;  //需要变倍的大小

	typedef list<AutoCameraDHPtzTrack> listPtzTrack;
	typedef map<int,listPtzTrack> mapAutoCameraPtzTrack;
	mapAutoCameraPtzTrack m_AutoCameraPtzTrack;
	char szAddrIP[50];
	int  szAddrPort;

};
#endif
