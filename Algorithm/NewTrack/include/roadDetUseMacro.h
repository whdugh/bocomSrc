#ifndef __ROAD_DET_USEMACRO_H
	#define __ROAD_DET_USEMACRO_H	

	#define USE_NEW_CLIENT  //使用新的客户端(默认要检测流量，车道要做事件)

	#define USE_SOBEL              //使用SOBEL

	#define  JUSTUSEBASICCODE      //只使用基本代码

	#define  USE_BIGVEHICLE_DETECT   //使用3边大车检测 

	
	#define  USE_HOG_TYPE	   //使用hog行人检测得到区域以判断类型
	#define  USE_HOG_COLOR	   //使用hog行人检测得到区域以判断颜色	
	//#define USE_LINEHOG      //使用线段hog行人检测
	
	#define USE_CONFIRM_OBJTYPE   //使用在报出事件时再确认下目标类型

	//#define USE_HEAD_DETECT	  //使用人头检测

	#define USE_SAVEINFOMATION     //保存标定和配置信息

	#define USE_BEGAVIOR_ANALYSE   //需进行行为分析

	//#define USE_RAPID_VER		   //速度优先

	//#define USE_FANTATRACK       //使用奇异轨迹

	#define MODIFY_CHANNEL_CONFIG3   //使用下拉列表框的3个车道检测参数配置

	#define ALERTOBJ_ON_GIVELOCATION    //将目标出现报警于给定的位置
	//#define ALERTOBJ_FOR_INDOORDEMO   //室内报警演示

	#ifdef MVDEBUGTEST
		#define NOPLATE_COLOR    
	#endif
	#ifndef LINUX
		#define NOPLATE_COLOR
	#endif

	//#define  GUIZHOUYANSHI      //贵州演示

	//////////////////////////////////////////////////////////////////////////
	#define  LINE_EXTRACT_INTERVAL 1   //线段提取的间隔

	#define MAXDISTANCE     10  //7 //求真实方向，用到的两点之间图像中的最小距离

#ifndef	SNAP_HISTORY
	#define SNAP_HISTORY	20
#endif
	#define MIN_TRACK_LEN	2		//进入车辆的最小轨迹长度


	#define VEHICLE_FRONT   1
	#define VEHICLE_BACK    0
	//#define ALERT_LEVEL 5 //报警分级,1级最严格
	#define UNIQUE_FLOWLINE

	#ifndef MAXLINENUM 
		#define MAXLINENUM 1000
	#endif

	#ifndef MAX3DMODELBIGVNUM
		#define MAX3DMODELBIGVNUM 500
	#endif

	#ifndef SOBELWEAKTHRES          //提取出sobel边缘的阈值(弱)
		#define SOBELWEAKTHRES 80
	#endif
	#ifndef SOBELSTRONGTHRES      //提取出sobel边缘的阈值(强)
		#define SOBELSTRONGTHRES 130
	#endif

	#define WRATEIMG2MVTRMAP 2   //图像相对MvTr的积分图的宽的比率
	#define HRATEIMG2MVTRMAP  2   //图像相对MvTr的积分图的高的比率

	#ifndef SAVETYPEINFOFRAMENUM   //保存多少帧类型信息
		#define SAVETYPEINFOFRAMENUM 50
	#endif
	#ifndef MULTIFRAMETYPEJUDGETHRES   //利用多少帧来判断类型
		#define MULTIFRAMETYPEJUDGETHRES 30
	#endif

	#define  TRACKORIHISTBIN 12   //轨迹角度直方图bin(即将360度分为12份)
	#define  IMGW2ORIHIST 5   //图像与轨迹角度直方图映射图的宽度比
	#define  IMGH2ORIHIST 5     //图像与轨迹角度直方图映射图的高度比

	#define  TRDISAPPEARPTNUM 20     //保留轨迹消失点的数目(每点50帧)

	#ifdef DEBUG_TIME
		#define MVTRACE_TIME(ARG1,ARG2) ts_s_e[ts_time][0]= (double) cvGetTickCount();\
					sprintf( putInfo[ts_time],ARG1);\
					ARG2; \
					ts_s_e[ts_time++][1]= (double) cvGetTickCount();
	#else
		#define MVTRACE_TIME(ARG1,ARG2) ARG2
	#endif

	#ifdef DEBUG_TIME
		#define MVTRACE_TIME_2(ARG1,ARG2,ARG3,ARG4) ARG1[ARG2][0]= (double) cvGetTickCount();\
					sprintf( putInfo[ARG2],ARG3);\
					ARG4; \
					ARG1[ARG2++][1]= (double) cvGetTickCount();
	#else
		#define MVTRACE_TIME_2(ARG1,ARG2,AG3,ARG4)  ARG4
	#endif

	#ifndef LINUX 
		#define int64_t __int64
	#endif

#if 1
	#define N_REF 50            //每条轨迹的记录的帧数for CvSet
	#define N_REF_SUB1 49
	#define N_REF_SUB2 48
	#define N_REF_SUB3 47
#else
	#define N_REF 500            //为绘制结果临时改的
	#define N_REF_SUB1 499
	#define N_REF_SUB2 498
	#define N_REF_SUB3 497
#endif

	#define ROAD_REGION_LIMIT 20 //最多支持这么多个ROAD REGION （算roadCenter时的限制）

	#define VELO_THRES_STOP		0.15
	#define VELO_THRES_SLOW		0.3 //1.25  0.3
	#define VELO_THRES_NOT_SLOW	0.6 //2.5  
	#define VELO_THRES_MOVE		0.96 //4.0
	#define VELO_THRES_FAST		1.8 //7.5
	#define VELO_THRES_VFAST	3.0

	#define STOPRGNMASK		1
	#define PASSBYRGNMASK	2
	#define DERELICTRGNMASK 4

	#define SCALE   0.3
	#define MAXIMAGESTROGENUM   15 
	#define INTERVALSTROGETIME     2
	#define JAMSTOPSTARTMATCHVAL 0.7f

	#define LINENUMTHRES 7

	#define SAVEOBJECTAPPEAR		1
	#define SAVEBIANDAO				2
	#define SAVEAGAINST				3
	#define SAVECROSS				4
	#define SAVEAPPEARONMOBEIL      5
	#define SAVESTOP				6
	#define SAVECROWSTOP            7
	#define SAVESLOW				8
	#define SAVEFAST				9
	#define SAVEBARGEIN				10
	#define SAVEBEYANDMARK			11

	#define  PUDONGGAOQING    1
	#define  SHANGHAIJIAOJIN  2

	#define  MAXAVTNUM 1000

	#ifndef  SETMAXTRNUM
		#define  SETMAXTRNUM  5000   //集合中的轨迹数目最大值
	#endif
	#ifndef  MAXMVTRNUM
		#define  MAXMVTRNUM  3500     //最多的运动轨迹数目
	#endif

	#define  PERSONTOCARWIDTH  0.5   //0.4
	#define  CARTOPERSONWIDTH  2.0   //2.5
	#define  PERSONTOCARHEIGHT 1.0   //1.2     //1.5
	#define  CARTOPERSONHEIGHT 1.0   //0.833   //0.66

	// #define  GRAYINTVELTOBG 4
	// #define  BLOCKHEIGHTTOBG 1
	// #define  BLOCKWIDTHTOBG 1

	#define  MAXSUBAREANUM  2
	#define  MAXSUBAREALINENUM  500

	#define  MAXDETECTEDCARNUM  50
	#define  MAXDROPOBJNUM 256

	#ifndef MAX_BIGVEH_NUM
		#define  MAX_BIGVEH_NUM  10
	#endif

	#ifndef  MAX_OBJECT_NUM
		#define  MAX_OBJECT_NUM  200
	#endif

	#ifndef MIN_VEL
		#define MIN_VEL 1.25//0.05 //if velocity less than MIN_VEL, then set orientation to 0
	#endif

	#ifndef CAMTHRES
		#define CAMTHRES 15 // use this to decide whether to use high or low models
	#endif

	#define STOPJUDGETRLEN 10  //用于停止判断时轨迹的长度

	#define BGKEYPTNUM 500       //背景角点数目

	#define MAXSTOPCARNUM 50       //最多的停车数

	#define LABEL_STEP_X 12
	#define LABEL_STEP_Y 12

	//提取车身颜色
	#ifndef CAR_COLOR_NUMBER
		#define CAR_COLOR_NUMBER 12
	#endif

	#define  SIFT_POINT_NUM   10

	#define MAX_VEHICLE_FRAME 200
	#define MAX_VEL_HIS 5

	#define VEHICLE_FRONT   1
	#define VEHICLE_BACK    0

	#define MAX_OBJID_NUM 10000  

#endif  //#ifndef __ROAD_DET_USEMACRO_H