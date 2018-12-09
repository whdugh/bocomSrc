#ifndef ROADENCODE_H
#define ROADENCODE_H

#include <stdio.h>
#include <gpac/isomedia.h>
#include "common/common.h"
#include "x264.h"
#include "ippi.h"
#include "ippcc.h"

typedef struct
{
	GF_ISOFile *p_file;
	GF_AVCConfig *p_config;
	GF_ISOSample *p_sample;
	int i_track;
	uint32_t i_descidx;
	int i_time_inc;
	int i_time_res;
	int i_numframe;
	int i_init_delay;
	uint8_t b_sps;
	uint8_t b_pps;
} mp4_t;

class RoadEncode
{
public:
	int InitEncode(char* fileName_mp4);
	int WriteFrame(uint8_t* buffer,int size,uint8_t* pEncodedBuffer,int &nOutSize,int* nalSize = NULL,int* nalCount = NULL);
	void UinitEncode();
	/*编码一帧数据，并同时写入文件和通过参数传出*/
	int WriteFrame(uint8_t* buffer,int size,bool bCount = true);
	void SetFrameRate(int frame);
	void SetVideoScale(float scale);
	long GetFrameCount() const;
	int64_t GetCodeSize() const;
	void SetVideoSize(int width, int height, float scale=1.0);

	RoadEncode();
private:
	int  Encode( x264_t *h,x264_picture_t *pic );
	bool OpenOutputFile(char* fileName);
	int  WriteOneFrame(uint8_t* buffer,int size);
	void CloseOutputFile();
	void SetEop(x264_picture_t *p_picture);
	bool SetOutputParam();
private:
	uint8_t* temBuffer;
	int buffer_size;
	mp4_t* m_mp4;
	int dSize[3];
	uint8_t* ptb[3];
	IppiSize srcSize;
	int i_flag;
	int m_rgbH;
	int m_rgbW;
	int i_rate;
	float f_scale;
	x264_t* m_h;          //X264 主要结构，贯穿整个程序
	x264_param_t m_param; //x264参数结构
	x264_picture_t m_pic; //x264帧结构
	int     i_frame;      //当前压缩帧
	int64_t i_file;       //压缩码流的大小
	int     i_frame_size; //某一帧的压缩码流的大小
	int     i_next_frame;
	bool    m_ifStart;
};

void *my_malloc( int i_size );
void my_free( void *p );
#endif // ROADENCODE_H
