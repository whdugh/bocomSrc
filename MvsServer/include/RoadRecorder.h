// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef SKP_ROAD_RECORDER_H
#define SKP_ROAD_RECORDER_H

#include "global.h"
#include "JpgToAvi.h"
#include "MJpegAvi.h"
#ifdef H264_ENCODE
#include "RoadEncode.h"
#endif
#include "CvxText.h"


/******************************************************************************/
//	描述:智能交通检测系统录像缓冲类
/******************************************************************************/

//帧数据列表
typedef std::list<std::string> RecorderFrame;
//事件消息列表
typedef std::list<std::string> EventMsg;

class CSkpRoadRecorder
{
public:
	//构造
	CSkpRoadRecorder();
	//析构
	~CSkpRoadRecorder();

public:
	//添加一帧数据,通道添加
	bool AddFrame(std::string& frame);

	//添加事件消息
    bool AddEvent(std::string& event);
	//启动录像
	bool BeginRecorder();

	bool Init();
	bool UnInit();

	//进行录象
	void DealRecorder();
	//录像格式为MJpeg
    void DealMJpegRecorder(std::string& event);
    //录像格式为JpgAvi-bocom
    void DealJpgToAviRecorder(std::string& event);
    //录像格式为MP4-H264
    void DealH264Recorder(std::string& event);

    //设置事件录象长度
	void SetCaptureTime( int nCaptureTime) { m_nCaptureTime = nCaptureTime;}
	//获得事件录象长度
	int GetCaptureTime() { return m_nCaptureTime;}

	//添加一帧h264数据
	bool AddH264Frame(const char *bufFrame, 
		const unsigned int nFrameSize, 
		SRIP_DETECT_HEADER sDetectHeader);
	//录像格式为MP4-H264
	void DealDspH264Recorder(std::string& event);

	//设置相机类型
	void SetCamType(int nCamType){ m_nCamType = nCamType; }
	int GetCamType() { return m_nCamType; }	

	//录像格式为H264裸流加aiv头
	void DealAviRecorder(std::string& event);

private:
	//弹出一帧图片
	std::string PopFrame(unsigned int uTimestamp,int64_t uTime64);
    //弹出一条事件
    std::string PopEvent();
	//在图像上叠加文本信息
	void PutTextOnVideo(IplImage* pImageWord,SRIP_DETECT_HEADER* sDetectHeader,CvxText* cvText,string strPlace,string strDirection);

//私有变量
private:
	//信号互斥
	pthread_mutex_t m_Frame_Mutex;
	//信号互斥
	pthread_mutex_t m_Event_Mutex;

	//帧列表
	RecorderFrame	m_FrameList;
	//消息列表
	EventMsg	m_EventList;


    //事件录象长度
	int m_nCaptureTime;
    //线程结束标志
	bool m_bEndRecorder;
	//录象线程数目
	int m_nEventCount;
	//编码结果
    unsigned char* m_pEncodeData;
    //编码大小
    int m_nEncodeSize;

	int m_nCamType;
};
#endif //SKP_ROAD_RECORDER_H
