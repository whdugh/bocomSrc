// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#if !defined(AFX_STDAFX_H__1C7A72A5_FA7D_4E8B_979A_4D0AE3C54735__INCLUDED_)
#define AFX_STDAFX_H__1C7A72A5_FA7D_4E8B_979A_4D0AE3C54735__INCLUDED_

#pragma warning(disable: 4786)

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers




#include <stdio.h>
#include "cxcore.h"

// TODO: reference additional headers your program requires here

#define DO_COL_DET//颜色检测
#define  SHUT_REC//针对车牌爆闪位置修正
#define SPEED_UP  //加速，还关闭了采标定;关闭了线段检测，对getimg做了处理，

#ifdef SPEED_UP //
//#define USE_DSP_CODE_FIND //启动Y通道加速！
//#define CORNER_UP //角点加速
//#undef  SHUT_REC//取消车牌爆闪位置修正
//#undef DO_COL_DET //关闭颜色检测
#endif

#define TRACK_GROUP_LGW //轨迹跟踪开关 ---不用vlp

//#define TURNON_EXCEDSPEED //温岭限速用/


//#define  VIO_RANK_OUT//违章等级控制--天津--有2处地方定义了？


//比武测试可以关闭，平时启动
//#define  CONTRAL_SINGLE_CAR //比武测试------可关闭:使车牌检测到一次的也进行跟踪

//#define RESEVE_DIR_0 //温岭中环医院卡口的逆行---针对卡口的逆行通用

#define DO_BURNS_VEICHLE_TYPE             // 是否开启使用直线判断车型
#define DO_ABS_BG                         // 是否启用绝对背景。
//#define DEBUG_FORCE_NIGHT_MODE          // 强制夜晚模式
//#define DEBUG_FORCE_DAY_MODE            // 强制白天模式
//#define DO_DONGS_CODE
//#define PREVENT_PLATE_MISS              // 防止车牌在短时间内多次经过情形时的漏报。打开此开关在车堵时可能导致多报。
 //#define DEBUG_FTG_OPTIMIZATION //记录各模块耗时----------------------------
//#define DEBUG_ERROR                     //开启
#define SAVE_CODE    //本地调试使用
//编码调试

#ifdef LINUX
 #define int64 long long
#endif

#ifdef LINUX

	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#define _chdir(a) chdir(a)
	#define _mkdir(a) mkdir(a, 0777)

	#define DEBUG_SILENT                      //打开它不调试(无窗口、无存图)。关闭它调试(有窗口或存图)。
	#define DEBUG_ERROR                     //开启
    #undef DEBUG_FTG_OPTIMIZATION //close time Recoard
    #undef SAVE_CODE

#else
	#include <direct.h>
	#define SHOW_WINDOW
	#define READ_CALIBRATION_FILE             //Linux下必须 关掉，Windows下必须打开
//	#define DEBUG_SILENT
    //#define DEBUG_ERROR                     //开启


#endif


#ifdef  SPEED_UP  //加速，还关闭了采标定;关闭了线段检测，对getimg做了处理，
#define DEBUG_SILENT 
#endif

#define STORE_FRAMES     60  
#define CHANGE_FOLDER_FRAMES     1000     //每隔这么多帧换个文件夹存图。
//#define DEBUG_VIRTUAL_LOOP_IMAGE          //是否调试虚拟线圈图像，显示、存图。
//#define SAVE_VLP_IMAGE                    //保存虚拟线圈图像
//#define DEBUG_BACKGROUND                  //是否调试虚拟线圈背景
//#define SAVE_BACKGROUND                   //保存背景图像
//#define DEBUG_LONG_TERM_GBM
//#define DEBUG_LONG_TRACK

//#define DEBUG_ABS_BG
#define LSD_LINE





//#define DEBUG_ERODE_DILATE                //是否显示虚拟线圈前景、以及其膨胀腐蚀结果
//#define DEBUG_VLP_CONTOUR                 //是否显示虚拟线圈图像的轮廓

//#define DEBUG_VIRTUAL_LOOP_RECTS          //是否显示虚拟线圈感知到的Rects （BG,FLOW) //*********

//#define SAVE_FLOW_FORE
//#define SAVE_BG_FORE                      //保存背景模型提取的前景
//#define SAVE_RECTSIMAGE                   //保存rectsimg
//#define DEBUG_EDGE_CONTOUR_GROUP          //是否显示边缘组合得到的Rects

              
//#define DEBUG_CHECK_IMAGE                 //调试check图
//#define SAVE_CHECK_IMAGE                  //保存check图
//#define DEBUG_CHECK_IMG_GROUP             //调试checkimg的group
//#define DEBUG_NON_UPDATE_REGION 

//#define DEBUG_OUTPUT_WAIKEY
//#define DEBUG_CLEAR_ROAD_LINE
//#define DEBUG_OPTICAL_FLOW_SIFT
//#define DEBUG_WRITE_RECORD                //是否每一帧记录所有objs信息
//#define DEBUG_HAVE_MOVED_POINT_IN_REGION
//#define DEBUG_OBJECT_RECT              //调试晚间模式中预测的目标位置。
//#define SAVE_OBJECT_RECT_IMAGE
//#define DEBUG_POS_FRAME_ASSOC          //记录根据车牌位置关联
//#define DEBUG_CAR_NUM_LEAKING            //记录漏车到CarNumRecord.txt
//#define LOG_RUN_FRAMES                  //记录程序接受到的每一帧，帧号，时间戳
//#define DEBUG_BEST_ASSIGNMENT           //打印最佳对应信息
//#define DEBUG_EXIT_WHEN_REINIT
//#define TIMER								//打印和保存运行的时间信息
//#define DEBUG_MANY_ZERO_FLOW_TRACK_SEAL
//#define DEBUG_VEICHLE_TYPE_LINES         //查看判车型时的直线提取情况
//#define DEBUG_VEICHLE_TYPE_BY_LINES        //调试判车型效果
//#define SAVE_VEICHLE_TYPE_BY_LINES_IMG     //判出的车型存图。
//#define DEBUG_CAR_NUM_REVISE               //车牌出现是清理的记录，存文件。
//#define DEBUG_SPLIT_BY_OBJS                //调试分裂情况
//#define DEBUG_CORNER_BACKGROUND
//#define DEBUG_VEICHLE_TYPE_BY_HEAD_LIGHT   //调试用车灯判断车型
//#define SAVE_VEICHLE_TYPE_BY_HEAD_LIGHT_IMG
#define DEBUG_CHANNELS_LINES                  // 调试车道、停止线等信息。

//#define DEBUG_BG_CHANGED_DETECT

//#define DEBUG_WAIT_COLOR_CHANGE_SEAL
//#define SAVE_WAIT_COLOR_CHANGE_SEAL
// #define DEBUG_RIGHT_CAR
// #define DEBUG_BIG_CAR
// #define SAVE_SIDE_BIG_CAR_IMAGE0
// #define SAVE_BIG_CAR
// #define DEBUG_HISTOGRAM
// #define DEBUG_THRESHOLD
// #define DEBUG_HISTOGRAM_PROJECT
 //#define DEBUG_GROUPS
// #define SAVE_BIG_CAR_NIGHT
//#define DEBUG_WAITKEY


//#define DEBUG_FTG_CORNERS
#define DEBUG_FTG_TRACKS
#define DEBUG_FTG_TACK_SET

//#define DEBUG_FIX_BY_HISTORY
//#define SAVE_FIX_BY_HISTORY


//#define DEBUG_ROAD_STATISTIC
//#define DEBUG_BG_LINE
//#define DEBUG_REVISE_BY_CARNUM
//#define DEBUG_PHYSIC_LOOP1
//#define DEBUG_MOTION_DIFF_CUT`
//#define DEBUG_PHYSIC_LOOP2
//#define DEBUG_ERROR_CORRECT_TIME

//#define DEBUG_SAVE_StarVlpBigVeh_Removal   //调试抑制*****大车目标多报功能！




//police
#define FORCE_STRAIGHT_RED_LIGHT
//#define SAVE_CORNERS_IMAGE
#define SAVE_TRACKS_AND_GROUPS


#define DEBUG_GROUPS_UPDATE 

//#define DEBUG_TRACKS_UPDATE

//#define DEBUG_CORNERS

#define DEBUG_TRACKS_AND_GROUPS 



//#define DEBUG_LOOP_SELECT_PIC

//#define PRINTF_INFO
//#define TIMER_INFO

//#define DEBUG_MERGE_GROUPS
//#define DEBUG_ROAD_TYPE_DETECTOR            // 调试道路类型判断功能。判断图像两侧边是机动车道还是非机动车道。

//#define LGW_GOAL_IMG
//#define LGW_TIME_PRINTF


//#define   FUNCTION_TIME_TEST
//#define   TEST_SWITCH
//#define LGW_TEST_SHOW














#ifdef DEBUG_SILENT

#ifndef SHOW_WINDOW
#undef SAVE_CODE
#endif

#undef TEST_SWITCH
#undef FUNCTION_TIME_TEST
#undef LGW_TIME_PRINTF
#undef LGW_GOAL_IMG
#undef LGW_TEST_SHOW
#undef DEBUG_BACKGROUND
#undef SAVE_BACKGROUND
#undef DEBUG_LONG_TERM_GBM
#undef DEBUG_LONG_TRACK
#undef DEBUG_ABS_BG

#undef LSD_LINE
//#undef DEBUG_FTG_OPTIMIZATION //打印时间 //////-------------在liuxu启动打印时间


#undef DEBUG_VIRTUAL_LOOP_RECTS
#undef DEBUG_CHECK_IMAGE


#undef DEBUG_VIRTUAL_LOOP_IMAGE
#undef SAVE_VLP_IMAGE
#undef DEBUG_ERODE_DILATE
#undef DEBUG_VLP_CONTOUR

#undef DEBUG_EDGE_CONTOUR_GROUP
#undef DEBUG_VIDEO
#undef DEBUG_DIFF_IMAGE

#undef DEBUG_CHECK_IMG_GROUP
#undef DEBUG_NON_UPDATE_REGION
#undef DEBUG_SOBEL_IMAGE
#undef DEBUG_OUTPUT_WAIKEY
#undef DEBUG_LINES
#undef DEBUG_CLEAR_ROAD_LINE
#undef DEBUG_OPTICAL_FLOW_SIFT
#undef DEBUG_WRITE_RECORD
#undef DEBUG_HAVE_MOVED_POINT_IN_REGION

#undef DEBUG_OBJECT_RECT

#undef DEBUG_POS_FRAME_ASSOC
#undef DEBUG_CAR_NUM_LEAKING
#undef LOG_RUN_FRAMES
#undef DEBUG_BEST_ASSIGNMENT
#undef DEBUG_EXIT_WHEN_REINIT
#undef SAVE_BIG_CAR_NIGHT
#undef TIMER
#undef DEBUG_RIGHT_CAR
#undef DEBUG_BIG_CAR
#undef DEBUG_HISTOGRAM
#undef DEBUG_THRESHOLD
#undef DEBUG_HISTOGRAM_PROJECT

#undef DEBUG_WAITKEY

#undef DEBUG_MANY_ZERO_FLOW_TRACK_SEAL
#undef DEBUG_VEICHLE_TYPE_LINES
#undef DEBUG_VEICHLE_TYPE_BY_LINES
#undef SAVE_VEICHLE_TYPE_BY_LINES_IMG
#undef DEBUG_CAR_NUM_REVISE
#undef DEBUG_SPLIT_BY_OBJS
#undef DEBUG_CORNER_BACKGROUND
#undef DEBUG_VEICHLE_TYPE_BY_HEAD_LIGHT
#undef DEBUG_BG_CHANGED_DETECT


#undef DEBUG_FTG_CORNERS
#undef DEBUG_FTG_TRACKS
#undef DEBUG_FTG_TACK_SET
#undef DEBUG_FIX_BY_HISTORY

#undef DEBUG_WAIT_COLOR_CHANGE_SEAL

#undef DEBUG_ROAD_STATISTIC
#undef DEBUG_BG_LINE
#undef DEBUG_REVISE_BY_CARNUM
#undef DEBUG_PHYSIC_LOOP1
#undef DEBUG_MOTION_DIFF_CUT
#undef DEBUG_PHYSIC_LOOP2
#undef DEBUG_ERROR_CORRECT_TIME


//police

#undef FORCE_STRAIGHT_RED_LIGHT
#undef SAVE_CORNERS_IMAGE
#undef SAVE_TRACKS_AND_GROUPS


#undef DEBUG_GROUPS_UPDATE 

#undef DEBUG_TRACKS_UPDATE

#undef DEBUG_CORNERS

#undef DEBUG_TRACKS_AND_GROUPS 

#undef DEBUG_MAX_SIZE

#undef PRINTF_INFO
#undef TIMER_INFO

#undef DEBUG_MERGE_GROUPS

#undef DEBUG_ROAD_TYPE_DETECTOR

#undef DEBUG_LOOP_SELECT_PIC

#endif



//#define DEBUG_VIRTUAL_LOOP_RECTS






#ifndef MAX_OF_TWO
#define MAX_OF_TWO(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN_OF_TWO
#define MIN_OF_TWO(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#ifndef MEAN_OF_TWO
#define MEAN_OF_TWO(a, b)          (((a) + (b)) / 2)
#endif



#ifndef MAX_UINT
#define MAX_UINT     ((unsigned int)~((unsigned int)0))
#endif


#ifndef MAX_INT
#define MAX_INT      ((int)(MAX_UINT >> 1))
#endif

#ifndef MIN_INT
#define MIN_INT      ((int)~MAX_INT)
#endif


#ifndef PRJ_NAMESPACE_NAME
#define PRJ_NAMESPACE_NAME FindTargetElePolice
#endif

#ifndef PRJ_NAMESPACE_BEGIN
#define PRJ_NAMESPACE_BEGIN namespace FindTargetElePolice {
#endif

#ifndef PRJ_NAMESPACE_END
#define PRJ_NAMESPACE_END }
#endif




#define DRAW_RECT(img, rect, color) \
	cvDrawRect( \
	(img), \
	cvPoint(((rect).x), ((rect).y)), \
	cvPoint(((rect).x) + ((rect).width -1), ((rect).y) + ((rect).height - 1)), \
	(color), \
	2 \
	)




#define RESIZE_RECT(rect, fx, fy) \
	cvRect((rect.x) * (fx), (rect.y) * (fy), (rect.width) * (fx), (rect.height) * (fy));





#ifdef LINUX
	#define LOCALTIME(ptime, ptm) localtime_r((ptime), (ptm))
#else
	#define LOCALTIME(ptime, ptm) localtime_s((ptm), (ptime))
#endif



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__1C7A72A5_FA7D_4E8B_979A_4D0AE3C54735__INCLUDED_)




