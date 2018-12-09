// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef BC_SEATBELTCLASSIFY
#define BC_SEATBELTCLASSIFY

#ifdef __cplusplus 
#define EXTERN_C extern "C" 
#else 
#define EXTERN_C 
#endif 

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#if defined WIN32 || defined _WIN32 
/* win32 dll export/import directives */ 
#ifdef LIBRARY_EXPORTS 
#define BC_SEATBELTCLASSIFY_API //EXTERN_C __declspec(dllexport) 
#else 
#define BC_SEATBELTCLASSIFY_API //EXTERN_C __declspec(dllimport) 
#endif 
#else 
/* unix needs nothing */ 
#define BC_SEATBELTCLASSIFY_API EXTERN_C 
#endif 

using namespace cv;
using namespace std;

typedef long long bc_handle_s;    //算法句柄

typedef int bc_result_s;      //函数返回的错误代码类型
#define BC_OK (0)             //正常运行
#define BC_E_IMAGE (-1)       //无图片
#define BC_E_PLATE (-2)       //无车牌
#define BC_E_RECT (-3)        //无车辆
#define BC_E_HANDLE (-4)      //句柄错误
#define BC_E_FAIL (-5)        //其他错误
#define BC_E_DIRECTION (-6)   //前尾牌错误

#ifndef OBJECTFACERT
#define OBJECTFACERT
typedef struct stFaceRt
{
	cv::Rect VehicleRect;		//驾驶员区域
	double Weight;			//驾驶员可信度
	int label_type;			//安全带判断
	float fCLP;				//安全带可信度
	float fblur;			//图像模糊度
	int nPhone_type;		//手机判断
	float fPhone_CLP;		//手机可信度
	int nSunVisor_type;		//遮阳板判断
	float fSunVisor_CLP;	//遮阳板可信度
	bool bguess;			

	stFaceRt()
	{
		VehicleRect = cv::Rect(0,0,0,0);
		Weight = 0.0;
		label_type = 0;
		fCLP = 0.0;
		fblur = 0.0;
		nPhone_type = 0;
		fPhone_CLP = 0.0;
		nSunVisor_type = 0;
		fSunVisor_CLP = 0.0;
		bguess = false;
	}
}FaceRt;
#endif


#ifndef OBJECTCAR
#define OBJECTCAR
typedef struct CAR_INFO   //车辆数据结构体
{
	CvRect  plate_rect; //车牌位置,相对于原图
	CvRect  car_rect; //车辆位置,相对于原图
	int plate_color; //车牌颜色（1蓝 2黑 3黄 4白 5其他）
	int carnumrow; //单双牌（1单牌 2双牌 3其他）
	int nCarNumDirection; //车牌的方向(0前牌 1尾牌 2未知)
	char carnum[7]; //车牌号码
	CAR_INFO()
	{
		plate_rect = cvRect(0,0,0,0);
		car_rect = cvRect(0,0,0,0);
		plate_color = 5;
		carnumrow = 3;
		nCarNumDirection = 0;
		memset(carnum,0,sizeof(char)*7);
	}
} car_info;
#endif

/*安全带创建句柄并初始化
handle: 初始化的安全带实例句柄
char *strPath：设置模型初始化路径
*/
BC_SEATBELTCLASSIFY_API bc_result_s bc_Seatbelt_Init( bc_handle_s &handle, char *strPath = NULL );

/*
mvSafetyBelt：驾驶员安全带检测(IplImage *接口)
输入：
handle: 已完成初始化的安全带实例句柄
pImage：需要处理的图片；
carInfo：输入的车辆信息
isDayByTime：白天为1,夜晚为0
vecP：驾驶员数据结构体
bIsSafetyBelt：是否检测主驾驶员安全带
ncartype：车型信息
bIsAideBelt：是否检测副驾驶员安全带
bIsPhone：是否检测主驾驶员打手机
bIsSunVisor：是否检测主驾驶遮阳板
返回值
return：函数返回的错误代码类型
*/
BC_SEATBELTCLASSIFY_API bc_result_s bc_Seatbelt_Recognition(bc_handle_s handle, IplImage *Image,
	CAR_INFO *carInfo,int isDayByTime, vector<FaceRt> &vecP, bool bIsSafetyBelt,
	int ncartype, bool bIsAideBelt = false,	bool bIsPhone = false, bool bIsSunVisor = false);

/*
GetVersion：获得版本号
返回：char Version[] = { "Seatbelt Version x.x.x.x" " "  __DATE__ " " __TIME__ };
*/
BC_SEATBELTCLASSIFY_API const char* bc_Seatbelt_GetVersion( bc_handle_s handle );

/*颜色释放函数*/
BC_SEATBELTCLASSIFY_API bc_result_s bc_Seatbelt_Destroy( bc_handle_s &handle );

/*获取错误信息*/
BC_SEATBELTCLASSIFY_API const char* bc_Seatbelt_GetErrMsg(bc_result_s nErrCode);

//////////////////////////////////////////////////////////////////////////
//以下为即将废弃接口

//设置模型初始化路径
BC_SEATBELTCLASSIFY_API bc_result_s bc_Seatbelt_SetPath(bc_handle_s handle, char* strPath);

#endif
