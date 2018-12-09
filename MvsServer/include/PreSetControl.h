// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

//	描述:预置位控制类
//	作者:於锋
//	日期:2011-5-5

#ifndef PRESET_CONTROL_H
#define PRESET_CONTROL_H

#include "global.h"

class CPreSetControl
{
public:
	//构造
	CPreSetControl();
	//析构
	~CPreSetControl();

public:
    //
    //初始化
	bool Init(int nChannelID);
	//释放
	bool UnInit();

	//设置相机编号
	void SetCameraID(int nCameraID){m_nCameraId = nCameraID;}

	//设置Monitor编号
    void SetMonitorID(int nMonitorID) { m_nMonitorID = nMonitorID; }

    //设置相机类型
    void SetCameraType(int  nCameraType) {m_nCameraType = nCameraType;}

    //调用远景预置位
    void GoToRemotePreSet(UINT32 uCurrentTime);

    //调用近景预置位
    void GoToLocalPreSet(CvRect rtPos,UINT32 uLocalPreSetTime);

    //是否有目标移动标志
    bool GetObjMovingState() { return m_bGetScaleObj; };

private:
    //根据事件发生位置判断目标属于哪个近景预置位
    int GetLocalPreSet(CvRect rtPos);

    float CalOverlapPercent(CvPoint lt_pt1, CvPoint rb_pt1, CvPoint lt_pt2, CvPoint rb_pt2);

private:
    int m_nChannelID;//通道号

    int m_nPreSetID;//当前远景预置位编号

    int m_nMonitorID; //监视器编号

	int m_nCameraId;//相机编号

	int m_nCameraType;//相机型号

	PreSetInfoList m_listPreSetInfo; //近景预置位信息

    //调用近景预置位时刻
    UINT32 m_uLocalPreSetTime;

	bool m_bGetScaleObj;//是否正在调用预置位
};
#endif
