// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "MvFindTargetByVirtualLoop.h"

class Parameters
{

public:
	static int RED_LIGHT_MEM_SIZE;      // 红灯状态记录帧数
	static int CAR_OBJECT_KEEP_TIME;    // Car对象在CarManager里面保留时间，单位秒。



	//////////////////////////////////////////////////////////////////////////
	// 分白天晚上的值
	//////////////////////////////////////////////////////////////////////////
public:
	




	static int OPTICAL_FLOW_FORM_RECT_SIZE_THRESH;//       = DAY_OPTICAL_FLOW_FORM_RECT_SIZE_THRESH;       //用流量组合前景rect要求最低包含的流量线个数。
	static int OPTICAL_FLOW_FORM_RECT_LENGTH_THRESH;//     = DAY_OPTICAL_FLOW_FORM_RECT_LENGTH_THRESH;     //用流量组合前景rect要求的最低流量线长度
	//static float CAR_FORCE_SEAL_FLOW;//                      = DAY_CAR_FORCE_SEAL_FLOW ;                     //小车强断流量
	//static float BUS_FORCE_SEAL_FLOW;//                      = DAY_BUS_FORCE_SEAL_FLOW;                      //bus强断流量
	//static float NOVEICHLE_FORCE_SEAL_FLOW;//                = DAY_NOVEICHLE_FORCE_SEAL_FLOW;                //非机强断流量
	//static float UNKONOWN_FORCE_SEAL_FLOW ;//                = DAY_UNKONOWN_FORCE_SEAL_FLOW;
	//static float CAR_MAX_FLOW;//                             = DAY_CAR_MAX_FLOW;                             //小车最大流量
	//static float BUS_MAX_FLOW    ;//                         = DAY_BUS_MAX_FLOW;
	//static float NONVEICHLE_MAX_FLOW ;//                     = DAY_NONVEICHLE_MAX_FLOW;
	//static float UNKNOWN_MAX_FLOW    ;//                     = DAY_UNKNOWN_MAX_FLOW;
	static float CAR_MIN_FLOW        ;//                     = DAY_CAR_MIN_FLOW;                             //小车最小流量
	static float BUS_MIN_FLOW       ;//                      = DAY_BUS_MIN_FLOW;
	static float NONVEICHLE_MIN_FLOW  ;//                    = DAY_NONVEICHLE_MIN_FLOW;
	static float UNKNOWN_MIN_FLOW     ;//                    = DAY_UNKNOWN_MIN_FLOW;
	static int DISAPPERA_ALLOWRANCE_FRAMES;//              = DAY_DISAPPERA_ALLOWRANCE_FRAMES;                 //多少帧没被更新就自然断
	static int SEAL_RESONABLE_FRAME_DIS;//                 = 5;                                               //断合理性判断所允许的间隔帧数
	
	
	



	static float NONVEICHLE_WIDTH;
	static float CAR_WIDTH;
	static float BUS_WIDTH;

	static float NONVEICHLE_MIN_WIDTH;
	static float CAR_MIN_WIDTH;
	static float BUS_MIN_WIDTH;

	static float NONVEICHLE_MAX_WIDTH;
	static float CAR_MAX_WIDTH;
	static float BUS_MAX_WIDTH;


	static int CORNER_DET_MIN_DIS;
	static float CORNER_DET_QUALITY;
	
	static int BG_GRADIENT;  //背景及目标直线梯度阈值！
	static int FG_GRADIENT;


	//
	static float RED_LIGHT_VIO_PIC_FAR;
	static float RED_LIGHT_VIO_PIC_NEAR;


	// 防止红灯闪烁的阈值。越大越容易将不亮判成亮。如果检测出来有红灯不亮的，则将该值调小。
	static float PREVENT_RED_LIGHT_FLASH_THRESHOLD;

	// 线圈检测车长的修正量。线圈检测出的车长-修正量
	static float LOOP_VEHICLE_LEN_FIX;

	// 线圈输出目标帧号修正量。结果=算出量+修正量
	static int   LOOP_OUTPUT_SEQ_PLUS;


	static bool USE_SURF;           //使不使用Surf匹配计算视频速度！
	static bool USE_EXTERNAL_CTRL;  //是否外部控制车牌距地面的像素高度!
	static int  PROJECTION_PIXEL;   //视频测速时，车牌距地面的像素高度!

	static bool USE_CHONGHONGDENG;  //是否使用冲红灯功能！
	static bool USE_JUDGE_TURNING;  //是否使用转向车辆的直行闯红灯抑制功能！
	static int  MIN_TURN_ANGLE;   //认为车子转向的最小角度！

	static void SelectParameterGroup(Time_Mode mode);
	
private:
	static int DAY_NIGHT_DISAPPERA_ALLOWRANCE_FRAMES[2];//            = 4;
	static int DAY_NIGHT_OPTICAL_FLOW_FORM_RECT_SIZE_THRESH[2];//     = 6;                
	static int DAY_NIGHT_OPTICAL_FLOW_FORM_RECT_LENGTH_THRESH[2];//   = 20;              
	//static float DAY_NIGHT_CAR_FORCE_SEAL_FLOW[2];//                    = 350;
	//static float DAY_NIGHT_BUS_FORCE_SEAL_FLOW[2] ;//                   = 800;
	//static float DAY_NIGHT_NOVEICHLE_FORCE_SEAL_FLOW[2]  ;//            = 220;
	//static float DAY_NIGHT_UNKONOWN_FORCE_SEAL_FLOW[2]  ;//             = 120;
	//static float DAY_NIGHT_CAR_MAX_FLOW[2]         ;//                  = 500;
	//static float DAY_NIGHT_BUS_MAX_FLOW[2]         ;//                  = 1050;
	//static float DAY_NIGHT_NONVEICHLE_MAX_FLOW[2]   ;//                 = 350;
	//static float DAY_NIGHT_UNKNOWN_MAX_FLOW[2]   ;//                    = 300;
	static float DAY_NIGHT_CAR_MIN_FLOW[2]       ;//                    = 200;
	static float DAY_NIGHT_BUS_MIN_FLOW[2]       ;//                    = 800;
	static float DAY_NIGHT_NONVEICHLE_MIN_FLOW[2]  ;//                  = 170;
	static float DAY_NIGHT_UNKNOWN_MIN_FLOW[2]       ;//                = 400;
	static int DAY_NIGHT_SEAL_RESONABLE_FRAME_DIS[2];

	static float DAY_NIGHT_NONVEICHLE_WIDTH[2];
	static float DAY_NIGHT_CAR_WIDTH[2];
	static float DAY_NIGHT_BUS_WIDTH[2];
	
	static float DAY_NIGHT_NONVEICHLE_MIN_WIDTH[2];
	static float DAY_NIGHT_CAR_MIN_WIDTH[2];
	static float DAY_NIGHT_BUS_MIN_WIDTH[2];

	static float DAY_NIGHT_NONVEICHLE_MAX_WIDTH[2];
	static float DAY_NIGHT_CAR_MAX_WIDTH[2];
	static float DAY_NIGHT_BUS_MAX_WIDTH[2];

	static int DAY_NIGHT_CORNER_DET_MIN_DIS[2];
	static float DAY_NIGHT_CORNER_DET_QUALITY[2];

	static int DAY_NIGHT_BG_GRADIENT[2];   
	static int DAY_NIGHT_FG_GRADIENT[2];
};


#endif