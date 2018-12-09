#ifndef __AN_BASE_H
#define __AN_BASE_H

#include <vector>
#include <stdio.h>
#include "DebugMacroDefine.h"

//-----------------------MIN/MAX-----------------------//
#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef AN_SQUARE
	#define AN_SQUARE(a,b)      ( (a*a + b*b) )
#endif


//--------------------------PI--------------------------//
#ifndef PIx2
	#define PIx2 6.28318530717958647692
#endif

#ifndef PI
	#define PI 3.14159265358979323846
#endif

#ifndef HALF_PI
	#define HALF_PI 1.57079632679489661923
#endif

#ifndef PI_DIVIDE_2
	#define PI_DIVIDE_2  1.57079632679  //pi/2
#endif

#ifndef PI_DIVIDE_3
	#define PI_DIVIDE_3  1.047197551197 //pi/3
#endif

#ifndef PI_DIVIDE_4
	#define PI_DIVIDE_4  0.785398163397 //pi/4
#endif

#ifndef PI_DIVIDE_5
	#define PI_DIVIDE_5  0.628318530718 //pi/5
#endif

#ifndef PI_DIVIDE_6
	#define PI_DIVIDE_6  0.523598775598 //pi/6
#endif

#ifndef PI_DIVIDE_8
	#define PI_DIVIDE_8  0.392699075000 //pi/8
#endif

#ifndef PI_DIVIDE_9
	#define PI_DIVIDE_9  0.349065844444 //pi/9
#endif

#ifndef PI_DIVIDE_10
	#define PI_DIVIDE_10  0.314159265359 //pi/10
#endif

#ifndef PI_DIVIDE_12
	#define PI_DIVIDE_12  0.26179938799 //pi/12
#endif

#ifndef PI_DIVIDE_20
	#define PI_DIVIDE_20  0.157079632679 //*.05
#endif

#ifndef PI_DIVIDE_30
	#define PI_DIVIDE_30  0.104719755119 //6°
#endif

#ifndef PI15
	#define PI15 0.471238898025//*.15
#endif

#ifndef ONEDIVPI
	#define ONEDIVPI 0.318309886  //1/PI
#endif

#ifndef PI_ONE_DEGREE
	#define  PI_ONE_DEGREE  0.0174533
#endif

//------------------------------------------------------//

//--for image size--
#ifndef IMAGE_VIBE_BGMOD_SIZE  //vibe背景模型所用的大小
	#define IMAGE_VIBE_BGMOD_SIZE    30000.0f
#endif
#ifndef IMAGE_BGMOD_SIZE  //背景模型所用的大小
	#define IMAGE_BGMOD_SIZE    60000.0f  //60000.0
#endif
#ifndef MV_USE_ROI_SIZE_BEHAVIOR  //行为分析对ROI所用的大小
	#define  MV_USE_ROI_SIZE_BEHAVIOR 180000
#endif
#ifndef MV_USE_ROI_SIZE_EVENT  //事件检测对ROI分析所用的大小
	#define  MV_USE_ROI_SIZE_EVENT    120000   //90000
#endif

//--for image process--
#ifndef MAX_SIFT_CORNERS
	#define MAX_SIFT_CORNERS 1000
#endif

#ifndef COS45
	#define COS45 0.70710678119
#endif

#ifndef SIN45
	#define SIN45 0.70710678119
#endif

#ifndef CLOCK_DIVISION
	#define CLOCK_DIVISION 1000000.0
#endif


//------------------------SIFT---------------------------//
#ifndef SIFT_WIN_SIZE
	#define SIFT_WIN_SIZE    4
#endif
#ifndef SIFT_BINS
	#define SIFT_BINS        8
#endif
#ifndef SIFT_DIMS
	#define SIFT_DIMS  (SIFT_WIN_SIZE*SIFT_WIN_SIZE*SIFT_BINS) //128 
#endif


//----------------for track matching-------------------//
#ifndef DONT_MATCH_FAIL
	#define DONT_MATCH_FAIL  -10       //未进行过匹配
#endif
#ifndef SIFT_MATCH_FAIL
	#define SIFT_MATCH_FAIL  -1        //SIFT匹配失败
#endif
#ifndef OPTI_FLOW_MATCH_FAIL
	#define OPTI_FLOW_MATCH_FAIL  -2   //光流匹配失败
#endif


//-------------------结构体定义-------------------
#ifndef CHANNEL_TYPE_MOD
	#define CHANNEL_TYPE_MOD  //车道类型模式
	enum ChannelType
	{	
		MOTORDIRVEWAY = 0,  	//机动车道
		NOMOTORDIRVEWAY,		//非机动车道
		MiXRDIRVEWAY,			//混行车道
		BEHAVIORANALYZEAREA     //行为分析区域
	};
#endif

#ifndef IMAGE_ENHANCE_MOD
	#define IMAGE_ENHANCE_MOD
	enum enhance_enum
	{
		DEFAULT_LINEA_ENHANCE = 0,
		LINEA_ENHANCE_1,
		LINEA_ENHANCE_2,
		LINEA_ENHANCE_3
	};  
#endif

#ifndef OBJ_TYPE_FOR_ANALYSIS
	#define OBJ_TYPE_FOR_ANALYSIS
	enum object_type_enum
	{
		DEFAULT_TYPE = 0,

		HUMAN = 100,
		WALK_PERSON = 101,
		BIKE_PERSON,
		MOTOR_PERSON,

		VEHICLE = 200,
		SMALL_VEHICLE = 201,
		MIDDLE_VEHICLE,
		BIG_VEHICLE,

		SHIP = 300,
		SMALL_SHIP = 301,
		MIDDLE_SHIP,
		BIG_SHIP,

		OTHER_OBJECTS = 900,
		LOSS_PAPER
	}; 
#endif

#ifndef ROAD_TYPE_MOD
	#define ROAD_TYPE_MOD
	//车道类型
	enum RoadType
	{	
		DEFAULTROAD = -1,  	//默认道路(机非混合)
		MOTORROAD = 0,  	//机动道路
		NOMOTORROAD,		//非机动道路
		BEHAVIORROAD = 3	//行为分析道路
	};
#endif

#ifndef USE_TIME_FOR_OUTPUT
	#define USE_TIME_FOR_OUTPUT
	//耗时输出类型
	enum useTimeForOutPut
	{
		ONLY_PRINTF = 10,
		ONLY_WRITE = 20,
		PRINTF_WRITE,
	};
#endif

#ifndef OBJ_TYPE_FOR_OUTPUT
	#define OBJ_TYPE_FOR_OUTPUT
	enum ObjTypeForOutput
	{
		OTHER = 1,     // 未知
		PERSON,        // 行人5
		TWO_WHEEL,     // 两轮车
		SMALL,         // 小 1
		MIDDLE,        // 中 2
		BIG           // 大 3
	};
#endif


#ifndef CAR_TYPE_JUDGE
	#define CAR_TYPE_JUDGE
	enum CarTypeJudge
	{
		ISCAR_WITH_LINES = 1,   // 线段
		ISCAR_WITH_SOBEL = 2,   // sobel
		ISCAR_WITH_COLOR = 4,   // 水平sobel
		ISNOEXIST_PEOPEL_WITH_HOG = 8,	    // 不满足HoG行人检测
	};
#endif

#ifndef DIFFIMG_WITH_BG_MOD
	#define DIFFIMG_WITH_BG_MOD
	enum DiffImgWithBgMod
	{
		DIFF_WITH_OR = 0,
		DIFF_WITH_AND = 1,
		DIFF_WITH_OTHER
	};
#endif

#ifndef SOBEL_EXTRACT_MOD
	#define SOBEL_EXTRACT_MOD
	enum sobel_extract_mod_enum
	{
		EXTRACT_H_SOBEL = 0,
		EXTRACT_V_SOBEL,
		EXTRACT_M_SOBEL
	};  
#endif


#ifndef DIFF_FOR_ROADWAY_MOD
	#define DIFF_FOR_ROADWAY_MOD
	enum DiffRoadMode
	{	
		DIFF_DEFAULTROAD = -1,  //默认道路
		DIFF_MOTORROAD = 0,  	//机动道路
		DIFF_NOMOTORROAD,		//非机动道路
		DIFF_BEHAVIORROAD   	//行为分析道路
	};
#endif

//停车判断中被拒的序号
#ifndef REFUSE_MOD_FOR_GRIDSTOP
	#define REFUSE_MOD_FOR_GRIDSTOP
		enum RefuseGridStopMode
		{
			NOSUIT_RECFKIMG = 1,    //近期前景数目要求
			NOSUIT_RECDIFFIMG,      //近期差分数目要求
			NOSUIT_RECHFKEDGE,      //近期前景边缘要求
			NOSUIT_RECGRADPERC,     //近期梯度边缘数目 
			NOSUIT_RECGRAYSAME,     //近期灰度值接近
			NOSUIT_RECGRADSAME,     //近期梯度边缘数接近 //6
			NOSUIT_INSTOPFKIMG,     //停车时间内长期前景数目要求
			NOSUIT_ONSTOPSAME,      //停车期间与近期相似的要求
			NOSUIT_OUTSTOPNOSAME,    //停车前与近期相似的要求//9

			NOSUIT_SIMILAR_REQUIRE = 20  //不满足相似性要求
		};
#endif


//////////////////////--网格法判断停车--//////////////////////
#ifndef GS_MAX_SIFT_FEAT_CNT
	#define   GS_MAX_SIFT_FEAT_CNT 800   //sift特征点的最多个数
#endif

#ifndef AN_SHORTLONG_TERM_MOD
	#define AN_SHORTLONG_TERM_MOD		
	enum _an_slTermMode	//长短周期模式
	{
	   AN_SHORT_TERM_MOD =  1,	
	   AN_LONG_TERM_MOD	
	};
#endif

#ifndef AN_GRID_LRTB
	#define AN_GRID_LRTB		
	enum _an_grid_lrtb  //左右上下
	{
		AN_LEFT =  0,	
		AN_RIGHT,
		AN_TOP,
		AN_BOTTOM
	};
#endif

#ifndef EXISTSIMITIME
	#define  EXISTSIMITIME  3.0        //存在相似的时间 
#endif
#ifndef EXISTSTOPTIME
	#define  EXISTSTOPTIME   10.0      //存在静止的时间
#endif

#ifndef RECENTCNT
	#define   RECENTCNT    5 		//最近几帧的数目
#endif
#ifndef SHORTTERMCNT
	#define   SHORTTERMCNT    40 	//短周期的数目
#endif
#ifndef LONGTERMCNT
	#define   LONGTERMCNT     90	//长周期的数目
#endif

#ifndef FORBGTERMCNT
	#define   FORBGTERMCNT    30	//背景周期的数目
#endif
#ifndef BGJUDGETERMTIME
	#define   BGJUDGETERMTIME 30.0	//背景周期更新的时间
#endif

#ifndef BGIMGTERMCNT
	#define   BGIMGTERMCNT    6		//背景图像的周期数目
#endif
#ifndef BGIMGTERMTIME
	#define   BGIMGTERMTIME 300.0	//背景图像周期更新的时间
#endif


//------------------------存图------------------------
#ifndef AN_RESIZE_IMAG4CHECK
	#define AN_RESIZE_IMAG4CHECK 40000.0f  //为check而resize后的图像面积
#endif


#endif  //#ifndef __ANALYSIS_BASE_H