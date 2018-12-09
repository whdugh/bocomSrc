// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef COLORRECOGNITION_H
#define COLORRECOGNITION_H
#include <vector>
#include "cv.h"
#include "cxcore.h"
#include "ml.h"

using namespace cv;
using namespace std;

class Carcolor;
class MvMulObjDetect;

struct Target;
struct CARNUM_CONTEXT;

#define MV_SAVEIMG_DEBUG
//#define MV_CARDETECTION //车辆检测

#ifndef OBJECTCOLOR
#define OBJECTCOLOR
typedef struct OBJECT_COLOR
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

////车身颜色
//#ifndef __COLORTYPE
//#define __COLORTYPE
//enum MVCOLOR
//{
//	WHITE, //= 0 by default
//	SILVER,
//	BLACK,
//	RED, //3
//	PURPLE,
//	BLUE,
//	YELLOW,
//	GREEN,
//	BROWN, //褐色
//	PINK, //粉红色
//	GRAY,  //10
//	UNKNOWN
//};
//#endif

class ColorRecognisize
{

public:
	ColorRecognisize();

	~ColorRecognisize();
	
	/*颜色初始化*/
	int color_Init( char *strPath = NULL );	

	/*颜色释放函数*/
	void color_Destroy();

	/*设置图像宽和高	
	mvSetImageWidthandHeight：设置图像宽和高，仅在mvGetClassifyEHDTexture输入参数imageData为char*时使用
	int nWidth：待检测图像的宽
	int nHeight：待检测图像的高
	*/
	void mvSetImageWidthandHeight( int nWidth, int nHeight );

	/*DL颜色识别主函数（二次识别调用接口）
	IplImage *img：输入的图像数据
	CARNUM_CONTEXT *vehicle_Info：输入的车牌信息（若无则输入NULL）
	CvRect *rtRoi：输入的车检区域（若无则输入NULL）
	object_color *objColor：输出的颜色类别
	int isDayBytime：使用默认值1（白天）
	*/
	int mvDLCardCarColor( IplImage *img, CARNUM_CONTEXT *vehicle_Info, CvRect *rtRoi, 
		object_color *objColor, int isDayByTime=1);
	
	/*DL颜色识别主函数（於峰的x86系统用）
	IplImage *img：输入的图像数据
	CARNUM_CONTEXT *vehicle_Info：输入的车牌信息（若无则输入NULL）
	CvRect *rtRoi：备用无牌车区域输入接口（若无则输入NULL）
	vector<Target> vTarget：输入的车检区域
	object_color *objColor：输出的颜色类别
	int isDayBytime：使用默认值1（白天）
	*/
	int mvDLCardCarColor( IplImage *img, CARNUM_CONTEXT *vehicle_Info, CvRect *rtRoi, 
		vector<Target> vTarget, object_color *objColor, int isDayByTime=1);

	/*DL颜色识别主函数（二次识别调用接口,自定义车辆结构体）
	IplImage *img：输入的图像数据
	CAR_INFO *car_Info：输入的车辆信息
	object_color *objColor：输出的颜色类别
	int isDayBytime：使用默认值1（白天）
	*/
	int mvDLCardCarColor( IplImage *img, CAR_INFO *carInfo, object_color *objColor, int isDayByTime=1);

	/*DL颜色识别主函数（於峰的x86系统用,自定义车辆结构体）
	IplImage *img：输入的图像数据
	CAR_INFO *car_Info：输入的车辆信息
	vector<Target> vTarget：输入的车检区域
	object_color *objColor：输出的颜色类别
	int isDayBytime：使用默认值1（白天）
	*/
	int mvDLCardCarColor( IplImage *img, CAR_INFO *carInfo, 
		vector<Target> vTarget, object_color *objColor, int isDayByTime=1);

	vector<Target> mvCarDetection(IplImage *img);

	CvRect mvCarLocation(IplImage *img, vector<Target> CarIn, CvRect vPlate);

	bool mvIntersect(const CvRect rt1, const CvRect rt2, float *fThresh = NULL);

	/*
	GetVersion：获得版本号	
	返回：static char Version[] = { "color Recognition Version x.x.x.x" " "  __DATE__ " " __TIME__ };
	*/
	static char* GetVersion();

	//设置模型初始化路径
	void mvSetPath(char* strPath);

	//显示模型版本
	int mvGet_Vision( char* strPath  );

	IplImage* mvCreateROI(IplImage *src, CvRect Brand, CvRect &m_RectROI, float fsize = 200);

	/*存图（按照不同种类和数量上限）
	输入：
	IplImage *img：原始图像
	IplImage *psubimage：局部图像（车辆）
	char path[512]：存图路径
	int nsort：存图类别号
	CvRect rtV：局部图像在原始图像中的位置（车辆）
	int nTh：数量上限（每种1000）
	*/
	void mvSaveImage( IplImage *img, IplImage *psubimage, char path[512], object_color *objColor, CvRect rtV, int nTh = 1000 );

protected:	


private:
	char buffer_path[512];

			
private:

	//控制初始化
	int first;
	int m_nWidht;
	int m_nHeight;
	IplImage *m_oriImg;

//DLL
	Carcolor *net;
#ifdef MV_CARDETECTION
	MvMulObjDetect *carD;
#endif
};

#endif
