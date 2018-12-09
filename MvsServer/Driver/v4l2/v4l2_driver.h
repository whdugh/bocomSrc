// 明迈交通视频智能识别检测软件 V1.0
// Mimax Intelligent Transport Video Recognition & Detection Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
#ifndef _V4L2_DRIVER_H
#define _V4L2_DRIVER_H

#include "v4l2_structdef.h"

//DEBUG输出
//#define _DEBUG

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif
//打开设备
void *v4l2_open (char *device);
//关闭设备
int v4l2_close (void *handle);

//捕捉视频
//设备视频参数
int v4l2_setformat (void *handle, struct v4l2_video_fmt *fmt);
//开始捕捉
int v4l2_startvideo (void *handle, int fps, unsigned int buffers);
//停止捕捉
void v4l2_stopvideo (void *handle);
//捕捉下一帧
struct v4l2_video_buf *v4l2_nextframe (void *handle);
//释放使用的数据
int v4l2_release_video_buf(struct v4l2_video_buf *buf);

//设置视频参数
//设置亮度
int v4l2_setbrightness(void *handle,unsigned int uBrightness);
//设置对比度
int v4l2_setcontrast(void *handle,unsigned int uContrast);
//设置饱和度
int v4l2_setsaturation(void *handle,unsigned int uSaturation);	
//设置色调
int v4l2_sethue(void *handle,unsigned int uHue);					
//设置视频源
int v4l2_setsource(void *handle,unsigned int uSource);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	//_V4L2_DRIVER_H
