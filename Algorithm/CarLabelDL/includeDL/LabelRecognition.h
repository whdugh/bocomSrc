// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef BRANDSUBSECTION_H
#define BRANDSUBSECTION_H

#include <vector>
#include "cv.h"
#include "cxcore.h"
#include "ml.h"

using namespace cv;
using namespace std;

#define MV_SAVEIMG_DEBUG //样本存储宏开关

#define LABELNUM 1828  //车标可检测的子类数

class Carlogo;

struct CARNUM_CONTEXT;

//车型输出枚举
enum TRUCK_LABEL
{
	LARGEBUS = 1,                //大型客车     1
	LARGEVAN,                    //大型货车     2
	MEDIUMBUS,                   //中型客车     3
	SMALLCAR,					//小型客车		4
	UNKNOW,						//未知			5
	MISS,						//未知			6
	SMALLVAN,                   //小型货车		7
	SUV,                        //suv           8
	MBC,                        //面包车         9
	YVK,                        //依维柯        10
	MPV,                        //MPV          11
	SMALLTRUCK,                //皮卡           12

};

#ifndef OBJECTLABEL
#define OBJECTLABEL
typedef struct OBJ_LABEL
{
	int plabel; //车标子类别标号

	int plabel_G; //车标大类别标号

	int plabel_Y;  //车标年份标号

	float fCLP; //置信度

	float fCLP_G; //置信度

	OBJ_LABEL()
	{
		plabel = 500000;

		plabel_G = 500000;

		plabel_Y = 500000;

		fCLP = 0.f;

		fCLP_G = 0.f;
	}

}object_label;
#endif

#ifndef OBJECTCAR 
#define OBJECTCAR 
typedef struct CAR_INFO //车辆数据结构体 
{ 
	CvRect plate_rect; //车牌位置,相对于原图 
	CvRect car_rect; //车辆位置,相对于原图 
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
	} 
} car_info; 
#endif

class CarLabel
{

public:
	CarLabel();

	~CarLabel();

	/************************************************************************/
	/*车标初始化*/
	/************************************************************************/
	int label_Init(char* strPath = NULL);	
	
	/************************************************************************/
	//车标子类查询函数接口
	/************************************************************************/
	int label_subclass(int carnum,string &subclassstr);

	/************************************************************************/
	/*车标释放函数*/
	/************************************************************************/
	int label_Destroy();

	/************************************************************************
	DL车标识别主函数（二次识别调用接口）
	IplImage *img：输入的图像数据
	carnum_context *vehicle_Info：输入的车牌信息（若无则输入NULL）
	CvRect *rtRoi：输入的车检区域（若无则输入NULL）
	object_label *objLabel：输出的车标类别
	int isDayBytime：使用默认值1（白天）
	/************************************************************************/
	int mvDLCardCarLabel( IplImage *img, CARNUM_CONTEXT *vehicle_Info, CvRect *rtRoi, 
		object_label *objLabel, int isDayByTime=1);
	

	/************************************************************************
	GetVersion：获得版本号	
	返回：static char Version[] = { "color Recognition Version x.x.x.x" " "  __DATE__ " " __TIME__ };
	/************************************************************************/
	static char* GetVersion();

	/************************************************************************/
	//设置模型初始化路径
	/************************************************************************/
	void mvSetPath(char* strPath);

	/************************************************************************
	存图（按照不同种类和数量上限）
	输入：
	IplImage *img：原始图像
	IplImage *psubimage：局部图像（车辆）
	carnum_context *vehicle_Info：输入的车牌信息（若无则输入NULL）
	char path[512]：存图路径
	int nsort：存图类别号
	CvRect rtV：局部图像在原始图像中的位置（车辆）
	int nTh：数量上限（每种1000）
	/************************************************************************/
	void mvSaveImage( IplImage *img, IplImage *psubimage,CARNUM_CONTEXT *vehicle_Info, char path[512], int nsort, CvRect rtV, int nTh = 10000 );


	/************************************************************************/
	//车型判别输出函数
	/************************************************************************/
	void TruckOutStyleput( int classfication, CARNUM_CONTEXT *vehicle_Info, int &nTruckDetect);
	


protected:	


private:
	char buffer_path[512]; 
	int label[LABELNUM];            //前牌子类输出映射表
	vector<string>vSubclass;     //车标子类存放容器
private:

	//控制初始化
	int first;
	int m_nWidht;
	int m_nHeight;
	IplImage *m_oriImg;
	
	Carlogo *net; //网络初始化
	
};

#endif