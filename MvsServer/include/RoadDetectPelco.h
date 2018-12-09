#ifndef ROAD_DETECT_PELCO_H
#define ROAD_DETECT_PELCO_H
/*************************
pelco版本停车严管检测及控制部分
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
#include "cv.h"
#include "cxcore.h"
#include "Common.h"
#include <string>
using namespace std;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (unsigned int)(~0)
#endif

#ifndef PELCO_ZOOM_TIME
#define PELCO_ZOOM_TIME 250000
#endif

#ifndef PELCO_PANTILT_TIME_LANG
#define PELCO_PANTILT_TIME_LANG 3000000
#endif 

#ifndef PELCO_PANTILT_TIME_SHORT
#define PELCO_PANTILT_TIME_SHORT 1000000
#endif 


#ifdef CAMERAAUTOCTRL
#include "CameraAutoCtrl.h"
#endif

class CRoadDetectPelco
{
public:
	CRoadDetectPelco();
	~CRoadDetectPelco(void);
	//设置检测车辆位置坐标
	void DetectRegionRect(int Camerawidth,int Cameraheight,int xPos,int yPos,int width,int height);
	//开始控制云台移动及拉近操作
	int DealTrackAutoRect(string frame);
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
	int CalculateRectPtzZoomEX(CvRect rectNew);
	/*
	输入参数：当前目标位置 false表示当前为水平移动，true表示当前为放大操作
	当目前动作为水平移动时，判断当前目标的中心点距离画面的中心点大小，
	当小于1/20时，不做动作，当距离大于画面的1/4时，移动长时；小于1/4时，移动小时间
	*/
	bool CalculateRectAndMovePtzOrZoom(CvRect rectNew,bool bZoom = false);
	/*使用pelco协议向上移动云台*/
	bool PTZMoveToPanTiltUp();
	/*使用pelco协议向下移动云台*/
	bool PTZMoveToPanTiltDown();
	/*使用pelco协议向左移动云台*/
	bool PTZMoveToPanTiltLeft();
	/*使用pelco协议向右移动云台*/
	bool PTZMoveToPanTiltRight();
	/*使用pelco协议控制相机倍率放大操作*/
	bool PTZMoveToZoom();
	/*使用pelco协议停止所有操作*/
	bool PTZMoveToStop();

	int WaitConnect(int nSocket, float fTimeout);
	int CloseSocket(int& fd);//可重复调用

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
#endif
	int m_uiResponse;
	int m_iCameraMoveTime;

	// 0表示不需要移动 1表示需要
	int m_uiTiltUpOrDown;
	int m_uiPanLeftOrRight;

	int m_nErrCount; //目标丢失数，超过40个人为此次定位失败
	char szAddrIP[50];
	int  szAddrPort;


};
#endif
