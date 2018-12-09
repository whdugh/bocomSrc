// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2014 上海博康智能信息技术有限公司
// Copyright 2008-2014 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _ROADTEMP_CAPTURE_H
#define _ROADTEMP_CAPTURE_H

/**
    文件：RoadTempCapture.h
    功能：缓存录像类
	说明: 1分钟缓存一个文件,循环使用,录像路径,格式:.//video/temp/00-01~59.mp4 [Hour-ChannelID]
    作者：yuwenxian
    时间：2014-2-25
**/

#include "global.h"

#define MAX_TEMP_VIDEO_SIZE 40 //最大缓存数量

class CRoadTempCapture
{
public:
	CRoadTempCapture();
	~CRoadTempCapture();

public:
	//初始化
	void Init(int nChannelId, int nWidth, int nHeight);
	//反初始化
	void Unit();

	//启动录像线程
	bool StartThread();
	//退出线程
	void CloseThread();

	//录像主循环
	void RunRecorder();
	//处理1帧录像
	void CaptureVideo();

	//添加1帧数据
	void AddFrame(unsigned char * pBuffer, Image_header_dsp &header);
	//弹出1帧数据
	int PopFrame(unsigned char ** ppBuffer);

	//弹出帧后减少帧缓存数据
	void DecreaseSize();
	//获取录像文件路径
	std::string GetEncodeFileName(){return m_strH264FileName;}

private:
	
	pthread_mutex_t m_video_Mutex;//信号互斥	
	pthread_t m_nThreadId;//线程ID
	
	int m_nChannelId;//通道编号	
	bool m_bEndCapture;//结束录象	
	int64_t m_uBeginTime;//开始时间	
	int64_t m_uEndTime;//结束时间
	
	unsigned char *m_FrameList[MAX_TEMP_VIDEO_SIZE];//缓冲区队列	
	unsigned char *m_pFrameBuffer;//帧接收缓冲区
	int m_nCurIndex;   //可存的内存块序号
	int m_nFirstIndex;//可取的内存块序号
	int m_nFrameSize ;//已经存储的内存块个数
	
	int m_uWidth;//录像宽
	int m_uHeight;//录像高
	
	std::string m_strH264FileName;//当前录像文件名称
	FILE *m_fpOut;//录像文件指针
};

#endif