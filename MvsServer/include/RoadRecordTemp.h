// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2014 上海博康智能信息技术有限公司
// Copyright 2008-2014 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _ROAD_RECORD_TEMP_H
#define _ROAD_RECORD_TEMP_H

/**
    文件：RoadRecordTemp.h
    功能：通过缓存录像类
	说明: 通过缓存录像,切分每帧,再解析,得到需要的时间段录像
    作者：yuwenxian
    时间：2014-2-28
**/

#include "global.h"
//#include "VideoCut.h"

#ifndef MAX_PATH_LEN
	#define MAX_PATH_LEN 260
#endif

class CVideoCut;
class CRoadRecordTemp
{
public:
	CRoadRecordTemp();
	~CRoadRecordTemp();

public:
	//初始化
	void Init();
	//反初始化
	void Unit();

	//添加事件消息
	bool AddEvent(std::string& event);
	//弹出一条事件
	std::string PopEvent();

	//启动录像主线程
	bool StartThread();
	//停止录像线程
	void CloseThread();

	//主循环
	void Run();
	//进行录象,缓存录像处理
	void DealTempVideo();

	//设置事件录象长度
	void SetCaptureTime( int nCaptureTime) { m_nCaptureTime = nCaptureTime;}
	//获得事件录象长度
	int GetCaptureTime() { return m_nCaptureTime;}

	//设置相机类型
	void SetCamType(int nCamType){ m_nCamType = nCamType; }
	int GetCamType() { return m_nCamType; }

	//获取单个待处理事件
	int GetEventElem(const std::string &event, RECORD_EVENT &re_event);

	//FIX 此函数单独用一个CVideoDeal来处理.
	//处理违章录像,对缓存录像进行切分
	//通过时间戳,获取录像时间,分钟区间
	bool GetVideoMinDis(int &nMinStart, int &nMinDis);
	//由时间戳获到小时,分钟,秒
	void GetVideoTime(const UINT32 uTime, UINT32 &uHour, UINT32 &uMin, UINT32 &uSec);
	//获取原始录像文件列表
	void GetSrcPathList();
	//录像格式为MP4-H264
	bool DealDspH264Recorder(std::string& event);

private:
	//线程ID
	pthread_t m_nThreadId;
	//信号互斥
	pthread_mutex_t m_Event_Mutex;
	//消息列表
	std::list<std::string>	m_EventList;

	//事件录象长度
	int m_nCaptureTime;
	//线程结束标志
	bool m_bEndRecorder;
	//录象线程数目
	int m_nEventCount;

	//相机类型
	int m_nCamType;	
	//是否正在录像
	bool m_bEndCoding; 

	//录像输出路径
	string m_strVideoPath;	

	//事件转换为待处理VIDEO_INFO记录
	CVideoCut * m_pVideoCut;

	//记录上一次事件，时间戳 单位s
	//UINT32 m_uPrevTimestamp;
	//CAvc2Avi m_avc2avi; //录像转换类
	//int m_bAviFlag;//录像转换标志

	//缓存处理文件路径列表
	std::list<std::string>	m_pathSrcList;
	//通道Id
	UINT32 m_uChannelId;
	//切分开始时间
	UINT32 m_uBeginTime;
	//切分结束时间
	UINT32 m_uEndTime; 
};

//extern CRoadRecordTemp g_roadRecordTemp;
#endif