#ifndef _Mv_CarnumDetect_H_
#define _Mv_CarnumDetect_H_

#pragma once

#include "cv.h"

#ifndef CARNUMPARM
#define CARNUMPARM
//车牌识别参数
typedef struct carnum_parm
{	
	int img_angle;//图像旋转角度，默认设置0度
	int isday; //1: 白天, 0: 晚上，设置检测的图片或者视频是白天还是晚上。
	int direction; //检测的是前排还是尾牌，如果应用程序无法判定，就设置成尾牌模式。//0表示前牌,1 表示后牌.
}carnum_parm_t;
#endif

//#define ER_CI_SHI_BIE //二次识别开关

#ifndef _CARNUMPARAMETER_H_
#define _CARNUMPARAMETER_H_


//#define CARNUMPARAMETER_DEBUG_PRINTF_PLATELOCATION
#ifndef MAXPLATENUMBERPERPICTURE
#define MAXPLATENUMBERPERPICTURE
#define MAX_PLATE_NUMBER_PERPICTURE 30
#endif


class Mv_Vehicle;

enum carnumdirection{ frontcarnum, backcarnum, unknow };

//于外面定义的车牌颜色
#ifndef CARCARD_COLOR
#define CARCARD_COLOR
enum CARNUM_COLOR
{
	CARNUM_BLUE=1,
	CARNUM_BLACK,
	CARNUM_YELLOW,
	CARNUM_WHITE,
	CARNUM_OTHER
};
#endif

#ifndef CARNUMROW
#define CARNUMROW
enum CARNUM_ROW{ listrow=1, doublerow, other };
#endif

#ifndef CARNUMCONTEXT
#define CARNUMCONTEXT
typedef struct CARNUM_CONTEXT
{
	char    carnum[7];             //车牌号码

	double	confidence[7];		   //位置信度

	CvRect  position;              //车牌位置,相对于原图

	CARNUM_COLOR   color;          //车牌颜色

	int     vehicle_type;          //车辆类型	

	float	iscarnum;              //从匹配结果看是否是车牌

	double     mean;               //车牌亮度

	double     stddev;			   //车牌方差	

	char    wjcarnum[2];           //武警牌中间的两个小字

	CARNUM_ROW   carnumrow;        //单排或者双排车牌

	CvRect    smearrect[20];        //光柱所在位置

	int       smearnum;				//对应的车辆编号

	int      VerticalTheta;         //垂直倾斜角,以角度为单位

	int      HorizontalTheta;       //水平倾斜角,以角度为单位

	carnumdirection     nCarNumDirection;    //车牌的方向，通知安发是前牌还是尾牌

	bool     bIsMotorCycle;          //是否为摩托车！


}carnum_context;
#endif

typedef struct ROAD_CONTEXT
{
	double     mean;               //区域平均亮度

	double     stddev;				//区域方差

	double  carnummean;             //车牌平均亮度

	double  carnumstddev;           //车牌方差

	CvRect    smearrect[20];         //光柱所在位置

	int       smearnum;				//光柱的数目

}road_context;

#ifndef CARNUMPARM
#define CARNUMPARM
//车牌识别参数
typedef struct carnum_parm
{	
	int img_angle;//图像旋转角度
	int isday; //1: day, 0: night
	int direction; //道路方向
}carnum_parm_t;
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
	BIG,           // 大 3
};

#endif

//张安发
enum mvCARCOLOR_NUMBER { YELLOW_N, WHITE_N, RED_N, BLACK_N, BLUE_N};

#ifndef NVISE_LIGHT
#define NVISE_LIGHT
enum mvNvise_light{ NORMAL, STRONGNVISE, LIGHTNVISE, LIGHTTOWARDS };//正常,强逆,弱逆,顺光;
#endif

#ifndef REALLOOP
#define REALLOOP
typedef struct LOOP_PARMARER
{
	CvPoint pStart_point;
	CvPoint pEnd_point;
	int     iNvise_light;
	LOOP_PARMARER()
	{
		pStart_point = cvPoint( 0, 0 );
		pEnd_point = cvPoint( 0, 0 );
		iNvise_light = -1;
	}

}loop_parmarer;
#endif

//struct carnum_context;
#ifndef CARCONTEXT
#define CARCONTEXT
typedef struct CAR_CONTEXT
{
	CvRect position;
	carnum_context* carnum;
	carnumdirection direction;
	double reliability;
	bool haveplate;
	bool isTaxiCar;

}car_context;
#endif

//struct object_context;
#ifndef OBJECTCONTEXT
#define OBJECTCONTEXT
#define object_Info char
enum OBJECT_TYPE
{	VEHICLE_TYPE = 0,       /* = 0 机动车     */
    TAXI_VEHICLE,           /* = 1 出租车     */
    MOTOR_VEHICLE,          /* = 3 摩托车     */
	AGRICULTURAL_VEHICLE,   /* = 2 农用车     */
	TRICYCLE,               /* = 4 三轮车     */
	PEDESTRIANER_TYPE,      /* = 5 行人       */	
	OTHRE_0,                /* = 6 其他0      */
	OTHRE_1,                /* = 7 其他1      */
	ERROR_TYPE              /* = 8 错误       */};
typedef struct OBJECT_CONTEXT
{
	CvRect position;
	object_Info* objInfo;
	carnumdirection direction;
	double reliability;
	OBJECT_TYPE objecttype;

}object_context;
#endif

#ifndef  CARNUMCONFIG
#define CARNUMCONFIG
typedef  struct  CARNUM_CONFIG
{
	int  pAlgEnable; 		   //算法使能, 0 不做车牌, 1 做车牌
	int  pMotorEnable;  	   //0不处理摩托牌, 1 处理
	int  pTaxiEnable;          //出租车判断使能
	int  pTricycleEnable;      //三轮车检测使能
	int  pPedestrianerEnable;  //行人检测使能
	int  pAgriculturalVehiclePlateEnable;  //农用车车牌识别使能

	char sCharForce;        //字符强制
	int  bBackLight;  	//逆光状态 1-逆光

	int  sSmallRect;  	//标定小车牌
	int  sBigRect;    	//标定大车牌

	CARNUM_CONFIG()
	{
		pAlgEnable = 0;
		pMotorEnable = 0;
		pTaxiEnable = 0;  
		pTricycleEnable = 0;  
		pPedestrianerEnable = 0;  
		pAgriculturalVehiclePlateEnable = 0;  

		sCharForce = '0';        //字符强制
		bBackLight = 0;  

		sSmallRect = 0;  
		sBigRect = 0;  
	}
} CARNUM_Config;

#define SET_PLR_ENABLE	  		    0x001           // 设置算法使能
#define SET_MOTOR_ENABLE			0x002           // 设置摩托牌使能 
#define SET_TAXI_ENABLE				0x004			// 设置TAXI检测使能
#define SET_NEW_CALIBRATION			0x008           // 重新设置标定
#define SET_CHAR_FORCE				0x010           // 设置强制字符
#define SET_BACKLIGHTING_OP			0x020			// 是否开启逆光模式
#endif


#endif


#ifndef REALLOOP
#define REALLOOP
typedef struct LOOP_PARMARER
{
	CvPoint pStart_point;
	CvPoint pEnd_point;
	int     iNvise_light;
	LOOP_PARMARER()
	{
		pStart_point = cvPoint( 0, 0 );
		pEnd_point = cvPoint( 0, 0 );
		iNvise_light = -1;
	}

}loop_parmarer;
#endif

#ifndef NVISE_LIGHT
#define NVISE_LIGHT
enum mvNvise_light{ NORMAL, STRONGNVISE, LIGHTNVISE, LIGHTTOWARDS };//正常,强逆,弱逆,顺光;
#endif

using namespace std;

class Mv_PLR_New;
class Mv_NoPlateVehicleDetector;
class Mv_VehicleDetector;

//class _declspec(dllexport) Mv_CarnumDetect
class Mv_CarnumDetect
{
public:
	Mv_CarnumDetect(void);
	~Mv_CarnumDetect(void);

	/*
	mvGet_BoostVision：获得识别字库的版本号及省份
	输入：	
	char* strPath:输入识别字库所在的路径
	输出
	cProvince：返回的字库省份，为字符，需要字符对照表翻译成相应的省份
	fVision：版本号。
	返回值
	1:表示不是新版本,是旧的字库，需要更新字库
	2:表示输入的路径字库不存在
	0:表示成功，可以取值进行字库判断
	*/
	int mvGet_BoostVision(char* strPath, char &cProvince, float &fVision);

	/*
	carnum_init：车牌识别模块初始化
	输入：	
	char* strPath:输入识别字库所在的路径
	float homo[3][3]：标定信息，图侦机版本不使用，给随机值就可以。
	nWidth：待处理图像的宽
	nHeight：带处理图像的高
	返回值
	暂无定义
	*/
	int carnum_init(char* strPath, float homo[3][3], int nWidth, int nHeight );

	/*
	mv_SetCarnumHeight：图像的宽和高及要检测的车牌的实际高
	输入：	
	nWidth：待处理图像的宽
	nHeight：带处理图像的高
	nCarnumHeight：待检测车牌的高	
	返回值
	暂无定义
	*/
	int mv_SetCarnumHeight( int nWidth, int nHeight, int nCarnumHeight );

	/*
	mv_SetPredectCarnumWidth：仅在二次识别种有效，特殊比武测试中，预知估计车牌宽
	输入：	
	small_width：车牌宽下限
	big_width：车牌宽上限
	返回值
	0 正确
	1 参数不合法（上下限大于70，且范围不大于100 ）
	*/
	int mv_SetPredectCarnumWidth( const int small_width, const int big_width );

	/*
	set_carnum_parm：设置检测参数
	输入：	
	carnum_parm_t：见该结构体说明
	*/
	void set_carnum_parm(carnum_parm_t *p);

	void set_carnum_parm(unsigned int m_Config);


	/*
	set_vedio：设置检测的源是单张的图片，还是连续的视频流
	输入：	
	input_type：//1为视频，2为图库
	*/
	void set_vedio(int input_type);

	/*
	GetSeqandTs：做视频检测时输出的帧号和时间戳
	输入：	
	seq_input：//输入的帧号
	ts_input：输入的时间戳
	number：每帧的车牌色数量
	输出：
	seq：输出的帧号
	ts：输出的时间戳
	*/
	void GetSeqandTs(unsigned int *seq,int64 *ts, unsigned int seq_input,int64 ts_input, int number );

	/*
	mvSetNonplateDetect：设置是否检测无牌车，默认不检测
	输入：	
	bFlag：//ture检测，false不检测
	*/
	void mvSetNonplateDetect( bool bFlag = false );


	/*
	mvSetDoMotorCarCarnum：设置是否检测摩托车牌，默认设置成不检测
	输入：	
	bFlagDo：//ture检测，false不检测
	*/
	void mvSetDoMotorCarCarnum( bool bFlagDo = false );


	/*
	CalibrationSet：设置相机的ID,在做图库检测中，如果知道这些图片来自于那个相机ID，则把该相机的ID也输入，减少初始化的次数。
	输入：	
	ID：//相机的ID号
	*/
	void CalibrationSet(unsigned int ID);


	/*
	find_carnum：车牌识别模块主函数入口
	输入：		
	char *filename：无意义，设置为NULL。
	IplImage *imgsrc：待识别的图像。
	char *ref：未使用，设置为NULL。
	IplImage **wb：无意义，设置为NULL。
	CvRect valid_rect：车牌检测区域，即相对于imgsrc的哪个位置进行车牌检测。如果cvrcet=（0,0,0,0），则用默认的检测区域，标清图片：整个图像的下80%区域。视频：整个图像的下35%区域；高清图片：整个图片的50%区域
	int indeinterlace：图像帧场标志，帧图像为1，场图像为2
	carnum_context* vehicle_result：车牌识别结果，结构见头文件说明。
	road_context* road_result：路面的均值方差等结果，结构见头文件说明。
	CvSeq *Indeterminobject：无意义，设置为NULL。
	LOOP_PARMARER loopparemar：线圈的出事起始位置，图侦机系统中，不使用，定义变量传入即可
	返回值：
	表示识别出的车牌个数，例如 0表示未识别出车牌，1表示识别出1张，以此类推。
	*/
	//输入原图，检测区域，需要做车检和车牌检测
	int find_carnum(char *filename, IplImage *imgsrc, char *ref, IplImage **wb, CvRect valid_rect, int indeinterlace, 
		carnum_context* vehicle_result, road_context* road_result, CvSeq *Indeterminobject, LOOP_PARMARER loopparemar);


	//给违停用的接口,参数参考上面
	//输入原图，车牌区域，无需车检和车牌检测
	int find_carnum( IplImage *imgsrc, CvRect valid_rect, carnum_context* vehicle_result );

	//给检测器二次识别定义的接口，只在给定区域内做车牌检测和识别
	//输入原图，和车牌检测区域
	int find_carnum( IplImage *imgsrc, CvRect valid_rect, carnum_context& vehicle_result );


	//给图侦、电警卡口用，不做车检，有应用端传入
	//输入原图，检测区域 以及 车检信息（VV），只需做车牌检测
	int find_carnum( IplImage *imgsrc, CvRect valid_rect, carnum_context* vehicle_result, vector<Mv_Vehicle>& VV );


	/* 张文俊 2014.8.11
	adjust_detectarea： 通过交集调整检测区域
	输入：
	car_rect： 车头位置
	detectarea： 老的检测区域
	New_detectarea： 调整后的检测区域
	返回值：
	0：没有交集
	1：有交集
	*/
	int adjust_detectarea( CvRect car_rect, CvRect detectarea, CvRect &New_detectarea );


	/* 张文俊 2014.8.9
	get_NonplateCar： 获得无牌车序列
	输入：
	vector<CAR_CONTEXT>&：车头容器的引用
	*/
	int get_NonplateCar( vector<CAR_CONTEXT>& );

	/* 张文俊 2014.8.9
	get_NonplateCar： 获得有牌车序列
	输入：
	vector<CAR_CONTEXT>&：车头容器的引用
	*/
	int get_HaveplateCar( vector<CAR_CONTEXT>& );

	/* 张文俊 2015.3.6
	get_NonplateCar： 获得有牌车序列
	输入：
	vector<CAR_CONTEXT>&：车头容器的引用
	*/
	int get_CarInfo( vector<CAR_CONTEXT>& );

	int get_ObjInfo ( vector<OBJECT_CONTEXT>& );

	/* 张文俊 2015.3.11
	get_VelocpedeInfo： 获得两轮车序列
	输入：
	vector<CAR_CONTEXT>&：车头容器的引用
	*/
	int get_VelocpedeInfo( vector<CAR_CONTEXT>& );

	/* 张文俊 2015.3.11
	Enable_Velocpede： 开启两轮车
	输入：

	*/
	int Enable_Velocpede();

	/* 张文俊 2015.3.11
	Diaable_Velocpede： 关闭两轮车
	输入：

	*/
	int Diaable_Velocpede();

	/*
	carnum_quit：车牌识别模块的释放函数	
	*/
	int carnum_quit();

	
	
	/*
	GetVersion：获得库版本号	
	返回：static char Version[] = { "Carnum Recognition Version x.x.x.x" " "  __DATE__ " " __TIME__ };
	*/
	static char* GetVersion();


	

private:
	Mv_PLR_New *m_pMv_PLR_New;
	Mv_NoPlateVehicleDetector *m_pVehivleDetect;
	Mv_VehicleDetector *m_pVelocpedeDetect;

	int m_iSmall_Width;
	float m_Rate_Resize;
	bool m_bIsPrePlateWidth;
	bool nFlagNonPlate;
	bool m_bEnableVelocpede;
	CARNUM_CONFIG m_Config;
	vector<CAR_CONTEXT> m_vVehicles;
	vector<CAR_CONTEXT> m_vVelocpede;
	vector<OBJECT_CONTEXT> m_oObjects;

};

#endif