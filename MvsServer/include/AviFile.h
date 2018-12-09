#ifndef AVI_FILE_H
#define AVI_FILE_H

#include "inttypes.h"
#include <stdio.h>

#ifndef AVIIF_KEYFRAME
#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
#endif

typedef struct
{
	int i_data;
	int i_data_max;
	uint8_t *p_data;
} vbuf_t;

#ifndef avi_t
typedef struct
{
	FILE *f;

	float f_fps;
	char  fcc[4];

	int   i_width;
	int   i_height;

	int64_t i_movi;
	int64_t i_movi_end;
	int64_t i_riff;

	int      i_frame;
	int      i_idx_max;
	uint32_t *idx;
} avi_t;
#endif

class CAviFile
{
public:
	CAviFile();
    ~CAviFile();

public:
	//avi file info
	void vbuf_init( vbuf_t * ); //缓冲初始化
	void vbuf_add( vbuf_t *, int i_data, void *p_data ); //添加一帧需转换h264/mpeg4数据
	void vbuf_reset( vbuf_t * ); //重设缓冲区
	void avi_init( avi_t *, FILE *, float, char fcc[4] , int iWidth, int iHeight); //初始化avi文件
	void avi_write( avi_t *, vbuf_t *, int  ); //写入avi文件
	void avi_end( avi_t * ); //终止写入avi文件

private:
	void avi_write_uint16( avi_t *a, uint16_t w );
	void avi_write_uint32( avi_t *a, uint32_t dw );
	void avi_write_fourcc( avi_t *a, char fcc[4] );
	void avi_write_header( avi_t *a );
	void avi_write_idx( avi_t *a );
	void avi_set_dw( void *_p, uint32_t dw );

public:
	//avi 
	vbuf_t  vb; //buffer struct
	avi_t   avi; //avi file format
};
#endif
