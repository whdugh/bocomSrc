// 明迈交通视频智能识别检测软件 V1.0
// Mimax Intelligent Transport Video Recognition & Detection Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
#ifndef _V4L2_STRUCTDEF_H
#define _V4L2_STRUCTDEF_H

#include <stdio.h>
#include <sys/time.h>
#include <linux/types.h>
#include <pthread.h>
#include <inttypes.h>
//vedio for linux 2
#include <linux/videodev2.h>
#include <string.h>


//设备编号最大值
#define	LINUX_VIDEO_MAX	16
//缓冲区数据集
#define LINUX_V4L2_BUFFERS 32
//最大采集数目
#define LINUX_V4L2_MAX_INPUT   16

//#define MAX_NORM    16
#define LINUX_V4L2_MAX_FORMAT  32	//格式

#define LINUX_V4L2_MAX_CTRL    32	//控制属性

#define LINUX_V4L2_UNSET		-1	//无效值

#define VIDEO_NONE           0
#define VIDEO_RGB08          1  /* bt848 dithered */
#define VIDEO_GRAY           2
#define VIDEO_RGB15_LE       3  /* 15 bpp little endian */
#define VIDEO_RGB16_LE       4  /* 16 bpp little endian */
#define VIDEO_RGB15_BE       5  /* 15 bpp big endian */
#define VIDEO_RGB16_BE       6  /* 16 bpp big endian */
#define VIDEO_BGR24          7  /* bgrbgrbgrbgr (LE) */
#define VIDEO_BGR32          8  /* bgr-bgr-bgr- (LE) */
#define VIDEO_RGB24          9  /* rgbrgbrgbrgb (BE) */
#define VIDEO_RGB32         10  /* -rgb-rgb-rgb (BE) */
#define VIDEO_LUT2          11  /* lookup-table 2 byte depth */
#define VIDEO_LUT4          12  /* lookup-table 4 byte depth */
#define VIDEO_YUYV			13  /* 4:2:2 */
#define VIDEO_YUV422P       14  /* YUV 4:2:2 (planar) */
#define VIDEO_YUV420P	    15  /* YUV 4:2:0 (planar) */
#define VIDEO_MJPEG	    16  /* MJPEG (AVI) */
#define VIDEO_JPEG	    17  /* JPEG (JFIF) */
#define VIDEO_UYVY	    18  /* 4:2:2 */
#define VIDEO_FMT_COUNT	    19



struct v4l2_video_fmt {
    unsigned int   fmtid;         /* VIDEO_* */
    unsigned int   width;
    unsigned int   height;
    unsigned int   bytesperline;  /* zero for compressed formats */
};


struct v4l2_video_buf {
    struct v4l2_video_fmt  fmt;
    size_t               size;
    unsigned char        *data;

	/* meta info for frame */
    struct {
	int64_t          ts;      /* time stamp */
	int              seq;
	int              twice;
    } info;

	//锁定标识
	int	refcount;
    pthread_mutex_t	lock;
    pthread_cond_t	cond;
	void                 (*release)(struct v4l2_video_buf *buf);
};




//驱动数据结构
struct v4l2_handle {

    int	fd;	//设备句柄
    
	//设备相关描述信息
    struct v4l2_capability	cap;

    //stream 依靠参数，干啥用的？
	struct v4l2_streamparm	streamparm;
    //设备INPUTS，干啥用的？
	int	ninputs;
	struct v4l2_input		inp[LINUX_V4L2_MAX_INPUT];
    //	struct v4l2_standard      	std[MAX_NORM];
	//设备格式描述
	int nfmts;
    struct v4l2_fmtdesc		fmt[LINUX_V4L2_MAX_FORMAT];
	//控制属性[亮度、对比度、饱和度......]，V4L2标准，不使用私有属性
	struct v4l2_queryctrl	ctl[LINUX_V4L2_MAX_CTRL];

    //属性
    //int                         nattr;
    //struct ng_attribute         *attr;

    //捕捉数据
    int	fps;	//帧率
	int	first;
    //long long                      start;
    //v4l2格式
	struct v4l2_format             fmt_v4l2;
	//内部使用格式,记录视频参数
	struct v4l2_video_fmt		   fmt_me;
	//缓冲数据
    struct v4l2_requestbuffers     reqbufs;
	//缓冲区数据
    struct v4l2_buffer             buf_v4l2[LINUX_V4L2_BUFFERS];
    //缓冲区数据
	struct v4l2_video_buf          buf_me[LINUX_V4L2_BUFFERS];

    unsigned int	queue;		//队列
	unsigned int	waiton;		//取得数据计数

    /* overlay */
    //struct v4l2_framebuffer        ov_fb;
    //struct v4l2_format             ov_win;
    //struct v4l2_clip               ov_clips[256];
    //int                            ov_error;
    //int                            ov_enabled;
    //int                            ov_on;
};



#endif//_V4L2_STRUCTDEF_H
