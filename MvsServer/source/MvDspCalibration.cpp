
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "MvDspCalibration.h"

/************************************************************************/
/*                  构造函数初始化                                      */
/************************************************************************/
MvDspCalibration::MvDspCalibration()
{

	memset( m_homographyMatrix, 0, sizeof(double)*3*3 );
}

/************************************************************************/
/*                    析构函数                                          */
/************************************************************************/
MvDspCalibration::~MvDspCalibration()
{
}

/************************************************************************/
/*             计算图像坐标到世界坐标的的转换矩阵                       */
/************************************************************************/
void MvDspCalibration::FindHomography()
{
	CvMat src_points = cvMat( 4, 2, CV_64FC1, (void*)m_image_coordinate );
	CvMat dst_points = cvMat( 4, 2, CV_64FC1, (void*)m_world_coordinate );
	CvMat *homo = cvCreateMat( 3, 3, CV_64FC1 );
	
	cvFindHomography( &src_points, &dst_points, homo );
	//S*H*src=dst(S为比例系数,H为透析矩阵：是由相机内部参数和相机的位置（平移旋转有关）)

	int i, j;
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			m_homographyMatrix[i][j] = cvmGet( homo, i, j );
		}
	}
	cvReleaseMat( &homo );
}

/************************************************************************/
/*             给定点的图像坐标，返回其世界坐标                         */
/************************************************************************/
CvPoint2D64f MvDspCalibration::GetWorldCoordinate( const CvPoint &pt )
{
	double x, y, sum;

	x   = m_homographyMatrix[0][0] * pt.x + m_homographyMatrix[0][1] * pt.y + m_homographyMatrix[0][2];
	y   = m_homographyMatrix[1][0] * pt.x + m_homographyMatrix[1][1] * pt.y + m_homographyMatrix[1][2];
	sum = m_homographyMatrix[2][0] * pt.x + m_homographyMatrix[2][1] * pt.y + m_homographyMatrix[2][2];

	CvPoint2D64f world_point;
	world_point.x = x / sum;
	world_point.y = y / sum;

	return world_point;
}

/************************************************************************/
/*              给定点的世界坐标，返回其图像坐标                        */
/************************************************************************/
CvPoint MvDspCalibration::GetImageCoordinate( const CvPoint2D64f& wordPt )
{
	CvMat hmat;
	CvMat *ihmat = cvCreateMat( 3, 3, CV_64FC1 );
	double hdata[] = { m_homographyMatrix[0][0], m_homographyMatrix[0][1], m_homographyMatrix[0][2],
		               m_homographyMatrix[1][0], m_homographyMatrix[1][1], m_homographyMatrix[1][2],
		               m_homographyMatrix[2][0], m_homographyMatrix[2][1], m_homographyMatrix[2][2] };
	cvInitMatHeader( &hmat, 3, 3, CV_64FC1, hdata );
	cvInvert( &hmat, ihmat );

	double u   = cvmGet(ihmat, 0, 0) * wordPt.x + cvmGet(ihmat, 0, 1) * wordPt.y + cvmGet(ihmat, 0, 2);
	double v   = cvmGet(ihmat, 1, 0) * wordPt.x + cvmGet(ihmat, 1, 1) * wordPt.y + cvmGet(ihmat, 1, 2);
	double sum = cvmGet(ihmat, 2, 0) * wordPt.x + cvmGet(ihmat, 2, 1) * wordPt.y + cvmGet(ihmat, 2, 2);

	CvPoint ret;
	ret.x = (int)(u/sum);
	ret.y = (int)(v/sum);

	cvReleaseMat( &ihmat );
	return ret;
}