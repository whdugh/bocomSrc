
#ifndef _MV_DSP_MAXSIZE_
#define _MV_DSP_MAXSIZE_

#include "MvDspCalibration.h"

class MvDspMaxSize
{
public:

	//构造函数初始化
	MvDspMaxSize();

	//析构函数
	~MvDspMaxSize();

	//求取车辆中心点的世界坐标和图像中车辆的宽度和高度
	void GetWorldCoordinateAndCarSize();

public:
	
	int m_nWidth;
	int m_nHeight;

	IplImage* m_wldx_image;           //车辆中心点的X方向世界坐标图像
	IplImage* m_wldy_image;           //车辆中心点的Y方向世界坐标图像
	IplImage* m_carwidth_image;       //在图像坐标中车辆的宽度图像
	IplImage* m_carheight_image;      //在图像坐标中车辆的高度图像
	double m_image_coordinate[12];    //已知的4个点的图像坐标
	double m_world_coordinate[12];    //已知的4个点的世界坐标
	double m_homographyMatrix[3][3];  //图像坐标到世界坐标的转换矩阵
	MvDspCalibration m_pCalib;        //坐标标定对象
};

#endif