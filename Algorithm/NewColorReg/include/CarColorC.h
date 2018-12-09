// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef BC_COLORRECOGNITION_C
#define BC_COLORRECOGNITION_C

#ifdef __cplusplus 
#define EXTERN_C extern "C" 
#else 
#define EXTERN_C 
#endif 

#if defined WIN32 || defined _WIN32 
/* win32 dll export/import directives */ 
#ifdef LIBRARY_EXPORTS 
#define BC_COLOR_API //EXTERN_C __declspec(dllexport) 
#else 
#define BC_COLOR_API //EXTERN_C __declspec(dllimport) 
#endif 
#else 
/* unix needs nothing */ 
#define BC_COLOR_API EXTERN_C 
#endif 

typedef long long bc_handle_c;    //算法句柄
typedef int bc_result_c;      //函数返回的错误代码类型
#define BC_OK (0)             //正常运行
#define BC_E_IMAGE (-1)       //无图片
#define BC_E_PLATE (-2)       //无车牌
#define BC_E_RECT (-3)        //无车辆
#define BC_E_HANDLE (-4)      //句柄错误
#define BC_E_FAIL (-5)        //其他错误

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifndef OBJECTCOLOR
#define OBJECTCOLOR
typedef struct OBJECT_COLOR   //颜色数据结构体
{
	int  nColor1;       //颜色1
	int  nColor2;	    //颜色2
	int  nColor3;       //颜色3

	int  nWeight1;    //颜色权重1
	int  nWeight2;    //颜色权重2
	int  nWeight3;    //颜色权重3

	float fCLP1;       //颜色置信度
	float fCLP2;       //颜色置信度
	float fCLP3;       //颜色置信度

	OBJECT_COLOR()
	{
		nColor1 = 11;
		nColor2 = 11;
		nColor3 = 11;
		nWeight1 = 0;
		nWeight2 = 0;
		nWeight3 = 0;
		fCLP1 = 0.0;
		fCLP2 = 0.0;
		fCLP3 = 0.0;
	}
} object_color;
#endif

#ifndef BCRECT
#define BCRECT
//图像当中矩形区域
struct BCRect
{
	int x;				//左上x坐标
	int y;				//左上y坐标
	int width;			//矩形框宽度
	int height;			//矩形框高度 
};
inline BCRect bcRect( int x, int y, int width, int height )
{
	BCRect r;
	r.x = x;
	r.y = y;
	r.width = width;
	r.height = height;
	return r;
}
#endif

#ifndef OBJECTCAR
#define OBJECTCAR
typedef struct CAR_INFO   //车辆数据结构体
{
	BCRect  plate_rect; //车牌位置,相对于原图
	BCRect  car_rect; //车辆位置,相对于原图
	int plate_color; //车牌颜色（1蓝 2黑 3黄 4白 5其他）
	int carnumrow; //单双牌（1单牌 2双牌 3其他）
	int nCarNumDirection; //车牌的方向(0前牌 1尾牌 2未知)
	char carnum[7]; //车牌号码
	CAR_INFO()
	{
		plate_rect = bcRect(0,0,0,0);
		car_rect = bcRect(0,0,0,0);
		plate_color = 5;
		carnumrow = 3;
		nCarNumDirection = 0;
		memset(carnum,0,sizeof(char)*7);
	}
} car_info;
#endif

/*颜色创建句柄并初始化
handle: 初始化的车身颜色实例句柄
strPath：设置模型初始化路径
*/
BC_COLOR_API bc_result_c bc_Color_Init_C( bc_handle_c &handle, char *strPath = NULL );

/*
GetVersion：获得版本号
返回：char Version[] = { "ColorRecog Version x.x.x.x" " "  __DATE__ " " __TIME__ };
*/
BC_COLOR_API const char* bc_Color_GetVersion_C( bc_handle_c handle );

/*颜色释放函数*/
BC_COLOR_API bc_result_c bc_Color_Destroy_C( bc_handle_c &handle );

/*颜色识别主函数（char*接口）
handle: 已完成初始化的车身颜色实例句柄
Image：输入的图像数据
nWidth：待检测图像的宽
nHeight：待检测图像的高
nChannels：彩色图像通道
carInfo：输入的车辆信息
objColor：输出的颜色类别
isDayBytime：使用默认值1（白天）
*/
BC_COLOR_API bc_result_c bc_Color_Recognition_C( bc_handle_c handle, const unsigned char *imageData, int nWidth, int nHeight, 
	int nChannels, CAR_INFO *carInfo, OBJECT_COLOR *objColor, int isDayByTime = 1 );

/*获取错误信息*/
BC_COLOR_API const char* bc_Color_GetErrMsg_C(bc_result_c nErrCode);

//////////////////////////////////////////////////////////////////////////
//以下为即将废弃接口

/*设置模型初始化路径*/
BC_COLOR_API bc_result_c bc_Color_SetPath_C( bc_handle_c handle, char* strPath );

#endif
