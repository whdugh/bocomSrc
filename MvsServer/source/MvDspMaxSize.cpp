
#include "MvDspMaxSize.h"

/************************************************************************/
/*                构造函数初始化                                        */
/************************************************************************/
MvDspMaxSize::MvDspMaxSize()
{
	m_wldx_image = NULL;
	m_wldy_image = NULL;
	m_carwidth_image  = NULL;
	m_carheight_image = NULL;
	memset( m_homographyMatrix, 0, sizeof(double)*3*3 );
}

/************************************************************************/
/*                析构函数释放内存                                      */
/************************************************************************/
MvDspMaxSize::~MvDspMaxSize()
{
	if( m_wldx_image )
	{
		cvReleaseImage( &m_wldx_image );
	}
	if( m_wldy_image )
	{
		cvReleaseImage( &m_wldy_image );
	}
	if( m_carwidth_image )
	{
		cvReleaseImage( &m_carwidth_image );
	}
	if( m_carheight_image )
	{
		cvReleaseImage( &m_carheight_image );
	}
}

/************************************************************************/
/*           求取车辆中心点的世界坐标和图像中车辆的宽度和高度           */
/************************************************************************/
void MvDspMaxSize::GetWorldCoordinateAndCarSize()
{
	CvSize imgSize = cvSize( m_nWidth, m_nHeight );

	m_wldx_image      = cvCreateImage( imgSize, IPL_DEPTH_64F, 1 );
	m_wldy_image      = cvCreateImage( imgSize, IPL_DEPTH_64F, 1 );
	m_carwidth_image  = cvCreateImage( imgSize, IPL_DEPTH_16U, 1 );
	m_carheight_image = cvCreateImage( imgSize, IPL_DEPTH_16U, 1 );

	memcpy( m_pCalib.m_image_coordinate, m_image_coordinate, sizeof(double)*2*4 );  //图像坐标有4个点
	memcpy( m_pCalib.m_world_coordinate, m_world_coordinate, sizeof(double)*2*4 );  //世界坐标有4个点
	m_pCalib.FindHomography();  //计算从图像坐标到世界坐标的转换矩阵

	CvPoint image_center;  //图像坐标中车辆的中心点
	CvPoint image_left;    //图像坐标中中心点的左点
	CvPoint image_right;   //图像坐标中中心点的右点
	CvPoint image_up;      //图像坐标中中心点的上点
	CvPoint image_down;    //图像坐标中中心点的下点
	CvPoint2D64f world_center;  //世界坐标中车辆的中心点
	CvPoint2D64f world_left;    //世界坐标中中心点的左点
	CvPoint2D64f world_right;   //世界坐标中中心点的右点
	CvPoint2D64f world_up;      //世界坐标中中心点的上点
	CvPoint2D64f world_down;    //世界坐标中中心点的下点
	double car_width_world = 1.9;   //世界坐标系中车辆的宽度
	double car_height_world = 6.0;
	//double car_height_world = 4.3;  //世界坐标系中车辆的高度
	int car_width_image, car_height_image;  //图像坐标中车辆的宽度和高度

	int row, col;
	for( row = 0; row < m_nHeight; row++ )
	{
		for( col = 0; col < m_nWidth; col++ )
		{
			image_center = cvPoint( col, row );
			world_center = m_pCalib.GetWorldCoordinate( image_center );  //求取图像中心点的世界坐标

			//求取世界坐标系中车辆的左点、右点、上点、下点
			world_left  = cvPoint2D64f( world_center.x - car_width_world/2, world_center.y );
			world_right = cvPoint2D64f( world_center.x + car_width_world/2, world_center.y );
			world_up    = cvPoint2D64f( world_center.x, world_center.y - car_height_world/2 );
			world_down  = cvPoint2D64f( world_center.x, world_center.y + car_height_world/2 );

			//根据世界坐标系中车辆的左点、右点、上点、下点求取图像坐标系中车辆的左点、右点、上点、下点
			image_left  = m_pCalib.GetImageCoordinate( world_left );
			image_right = m_pCalib.GetImageCoordinate( world_right );
			image_up    = m_pCalib.GetImageCoordinate( world_up );
			image_down  = m_pCalib.GetImageCoordinate( world_down );

			//计算图像坐标系中车辆的宽度和高度
			car_width_image  = cvFloor( sqrt( (double)(image_right.x - image_left.x) * (image_right.x - image_left.x) 
				                            + (double)(image_right.y - image_left.y) * (image_right.y - image_left.y) ) );
			car_height_image = cvFloor( sqrt( (double)(image_down.x - image_up.x) * (image_down.x - image_up.x)
				                            + (double)(image_down.y - image_up.y) * (image_down.y - image_up.y) ) );

			((double*)(m_wldx_image->imageData + m_wldx_image->widthStep * row))[col] = world_center.x;
			((double*)(m_wldy_image->imageData + m_wldy_image->widthStep * row))[col] = world_center.y;
			((ushort*)(m_carwidth_image->imageData + m_carwidth_image->widthStep * row))[col] = car_width_image;
			((ushort*)(m_carheight_image->imageData + m_carheight_image->widthStep * row))[col] = car_height_image;
		}
	}
}