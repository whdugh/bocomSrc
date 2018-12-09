
#ifndef _MV_DSP_CALIBRATION_
#define _MV_DSP_CALIBRATION_

#include "cxcore.h"
#include "highgui.h"
#include "cv.h"

class MvDspCalibration
{
public:
	//构造函数初始化
	MvDspCalibration();
	
	//析构函数
	~MvDspCalibration();
	
	//计算图像坐标到世界坐标的的转换矩阵
    void FindHomography();

	//给定点的图像坐标，返回其世界坐标
	CvPoint2D64f GetWorldCoordinate( const CvPoint& pt );

	//给定点的世界坐标，返回其图像坐标
	CvPoint GetImageCoordinate( const CvPoint2D64f& wordPt );

public:
	double m_image_coordinate[12];       //给定点的图像坐标
	double m_world_coordinate[12];       //给定点的世界坐标
	double m_homographyMatrix[3][3];  //图像坐标到世界坐标的转换矩阵
};

#endif