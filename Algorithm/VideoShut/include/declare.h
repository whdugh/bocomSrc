
#ifndef _DECLARE_H_
#define _DECLARE_H_


#define SIFT_DESC_DIMENTION 128
#define MAX_CORNERS_PER_FRAME 100       //每帧中检测出的最大的角点数量
#define MAX_CORNERS_OF_NONMAX 1024

#define MAX_TRACKS_PER_FRAME  64      //每帧中最大的轨迹数量		
#define MAX_CORNERS_PER_TRACK 40       //每条轨迹最大的角点数量	

#define MAX_GROUPS_PER_FRAME 128        //每帧中Group数量的最大值
#define MAX_TRACKS_PER_GROUP 16        //每个Group中包含的Track数量的最大值

//#define MAX_IMG_HEIGHT 10000  //（算法内部计算）
//#define MAX_IMG_WIDTH  10000 //（算法内部计算）


#include "stdio.h"
#include "cv.h"
#include "highgui.h"



typedef struct   _Mv_INPUT_DATA
{
	unsigned char *sqrtData;
	unsigned char *stdData;
} MvInputData;


#endif
