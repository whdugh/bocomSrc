#ifndef MVVEHICLECLASSIFY__H__
#define MVVEHICLECLASSIFY__H__

#include <cv.h>
#include <highgui.h>
#include "ml.h"
#include <algorithm>
#include <vector>

using namespace std;
using namespace cv;

struct CARNUM_CONTEXT;
class MvVehicleClassifyDetail;

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

class MvVehicleClassify  
{

public:

	MvVehicleClassify();
	~MvVehicleClassify();
		
	/*
	mvTruckDetect：车辆类型判断
	输入：	
	pImage：需要处理的图片，由于车牌判别和卡车类型细分用的不是同一个图片，在传入该图片的同时，需要设置ROI区域；
	Brand：车牌信息
	IsDay：白天为true,夜晚为false
	IsForeCarnum：是否为前牌（默认为前牌）
	IsMoreDetail：是否为细分车型（默认为否）
	nMoreDetail：细分车型返回值
	返回值
	return：第一大类
	nMoreDetail：第二子类（细分结果）
	*/
	int mvTruckDetect(const IplImage *pImage,const CARNUM_CONTEXT Brand,bool IsDay=true,
		bool IsForeCarnum=true,bool IsMoreDetail=false,int *nMoreDetail = NULL);

	/*
	mvTruckDetect：车辆类型判断(自定义车辆结构体)
	输入：	
	pImage：需要处理的图片，由于车牌判别和卡车类型细分用的不是同一个图片，在传入该图片的同时，需要设置ROI区域；
	Brand：车牌信息
	IsDay：白天为true,夜晚为false
	IsForeCarnum：是否为前牌（默认为前牌）
	IsMoreDetail：是否为细分车型（默认为否）
	nMoreDetail：细分车型返回值
	返回值
	return：第一大类
	nMoreDetail：第二子类（细分结果）
	*/
	int mvTruckDetect(const IplImage *pImage,const CAR_INFO *carInfo, bool IsDay=true,
		bool IsForeCarnum=true,bool IsMoreDetail=false,int *nMoreDetail = NULL);

	/*
	mvNonCardTruckDetect：车辆类型判断
	输入：	
	pImage：需要处理的图片，由于车牌判别和卡车类型细分用的不是同一个图片，在传入该图片的同时，需要设置ROI区域；
	Brand：车牌信息
	IsDay：白天为true,夜晚为false
	IsForeCarnum：是否为前牌（默认为前牌）
	IsMoreDetail：是否为细分车型（默认为否）
	nMoreDetail：细分车型返回值
	返回值
	return：第一大类
	nMoreDetail：第二子类（细分结果）
	*/
	int mvNonCardTruckDetect (const IplImage *pImage, CARNUM_CONTEXT Brand, bool IsDay=true, 
		bool IsMoreDetail=false, int *nMoreDetail = NULL);

	static char* GetVersion();
	
	//设置模型初始化路径
	void mvSetPath(char* strPath);

	void Truck_Destroy();

	int Truck_Init(char* strPath = NULL);

private:
	MvVehicleClassifyDetail *mvclsVType;
	
};

#endif
