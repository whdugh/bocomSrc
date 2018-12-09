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

#ifndef SKP_ROAD_VIDEO_V4L2_H
#define SKP_ROAD_VIDEO_V4L2_H


#include "global.h"
#include "v4l2_driver.h"

#include "cv.h"
#include "highgui.h"

#include "capture.h"
#define RESETCOLOR 32768



/******************************************************************************/
//	描述:智能交通检测系统v4l2类
/******************************************************************************/
typedef struct _avi_video_header
{
	unsigned short uWidth;  //水平分辨率
	unsigned short uHeight; //垂直分辨率
	unsigned int   uSeq;    //帧号
	unsigned int   uSize;     //码流长度
}avi_video_header;

class CSkpRoadV4l2
{
public:
	//构造
	CSkpRoadV4l2();
	//析构
	~CSkpRoadV4l2();

public:
	//打开设备,设备编号
	bool OpenVideo(int nNo,unsigned int uWidth,unsigned int uHeight,int nCameraType = ANALOG_FRAME);
	//关闭设备
	bool CloseVideo();

	//打开avi文件流
	bool OpenAvi(const char* strFileName,unsigned int& uWidth,unsigned int& uHeight);
	//读取一帧avi图象
	IplImage* NextAviFrame(unsigned int& nSeq);

	//开始采集数据
	bool StartCapture();
	//停止采集数据
	bool StopCapture();

	//采集数据
	struct v4l2_video_buf* NextFrame();
	//释放数据
	bool ReleaseVideoBuffer(struct v4l2_video_buf* buf);

	//设置视频参数[亮度+对比度+饱和度+色调]
	bool SetVideoParams(SRIP_CHANNEL_ATTR& sAttr);

private:
	//v4l2结构
	struct v4l2_handle *m_pV4l2Handle;

	//v4l2结构
	struct v4l2_video_handle *m_pVideoHandle;

	//视频参数[亮度+对比度+饱和度+色调]
	SRIP_CHANNEL_ATTR m_sChannelAttr;

	//avi文件指针
	FILE* m_AviFp;
	CvCapture* m_AviCapture;
	//是否读avi文件
	bool m_bReadFile;
	//当前帧号
	unsigned int m_nSeq;
	//首场的时间戳
	int64_t m_uTs;
	//avi文件名称
	std::string m_strFileName;
	//数据指针
	unsigned char* m_pBuffer;
	//模拟相机类型
	int m_nCameraType;
    //
	v4l2_video_buf m_video_buf;
};

#endif
