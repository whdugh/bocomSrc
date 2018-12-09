// 明迈交通视频智能识别检测软件 V1.0
// Mimax Intelligent Transport Video Recognition & Detection Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
#ifndef _YUV_DRIVER_H
#define _YUV_DRIVER_H


#include <linux/types.h>
#include <sys/time.h>
#include <time.h>


//DEBUG输出
//#define _DEBUG


typedef struct _yuv_video_header
{
	unsigned char cSynchron[4];     //同步头
    unsigned char cType[4];     //排列类型
	unsigned short nWidth; //水平分辨率
	unsigned short nHeight; //垂直分辨率
	unsigned short nFrameRate; //帧率
	unsigned short nFieldSeq;  //场序号
	unsigned int nSize;     //码流长度
    unsigned char cReserved[8];  //预留
}yuv_video_header;

typedef struct _yuv_video_buf {
    unsigned int   width;			//宽
    unsigned int   height;			//高
    unsigned int   size;		//大小
	unsigned long  nSeq;        //帧号
	int64_t          ts;       //时间戳(从零开始的相对时间)
	unsigned int   uTimestamp;  //系统实际时间
    unsigned char  *data;		//数据
}yuv_video_buf;







#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif
//打开设备,返回socket
int yuv_open(const char* host,int port,const char* user,const char* pass);
//关闭设备
int yuv_close (int fd);

//接收数据
int yuv_nextframe(int fd,yuv_video_buf** pBuf);

//释放内存
int yuv_release_buf();
//时间戳
int64_t yuv_to_timestamp(struct timeval *tv);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	//_YUV_DRIVER_H
