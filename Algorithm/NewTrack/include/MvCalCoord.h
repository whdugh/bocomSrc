#ifndef __MVCALCOORD_H
#define __MVCALCOORD_H

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#ifndef LINUX
	#include <io.h>
#endif
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

#include "libHeader.h"

#include "cal_main.h"  //tsai

#include "comHeader.h"  //放最后

using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;

#ifndef PI
	#define PI 3.14159265 
#endif
#ifndef DEGREE_PI
	#define DEGREE_PI 0.0174529   
#endif

#ifndef MINMAX_X
	#define MINMAX_X 0
#endif
#ifndef MINMAX_Y
	#define MINMAX_Y 1
#endif

enum OBJECT_3DMODEL_MODE
{
	SMALL_VEHICLE_3DMODEL = 0,  //标准小车
	GENERAL_PEOPLE_3DMODEL,	    //正常行人 
};

#ifndef POINT_COORD_CONVERT
	#define POINT_COORD_CONVERT
	//2D32f点的坐标系转换结构体
	typedef struct _point2D32fCoordConvert
	{
		CvPoint2D32f ptSrcOffSet;
		float fS2D_x, fS2D_y;
		
		_point2D32fCoordConvert( )
		{
			ptSrcOffSet = cvPoint2D32f( 0.0f, 0.0f );
			fS2D_x = 1.0f;
			fS2D_y = 1.0f;
		};

		//给定初始的两坐标系的关系参数
		void giveInitParam( CvPoint2D32f _ptSrcOffSet, 
				float _fS2D_x, float _fS2D_y )
		{
			ptSrcOffSet = _ptSrcOffSet;
			fS2D_x = _fS2D_x;
			fS2D_y = _fS2D_y;
		};

		//给定在源坐标系里的点位置，得到在目标坐标系的点位置
		CvPoint2D32f getDstCoord4SrcCoord( const CvPoint2D32f ptSrc )
		{
			CvPoint2D32f ptDiff = cvPoint2D32f( ptSrc.x - ptSrcOffSet.x,
												ptSrc.y - ptSrcOffSet.y );
			CvPoint2D32f ptDst = cvPoint2D32f( ptDiff.x / fS2D_x,
											   ptDiff.y / fS2D_y );
			return ptDst;
		};


		//给定在目标坐标系里的点位置，得到在源坐标系的点位置
		CvPoint2D32f getSrcCoord4DstCoord( const CvPoint2D32f ptDst )
		{
			CvPoint2D32f ptDiff = cvPoint2D32f( ptDst.x * fS2D_x,
										    	ptDst.y * fS2D_y );
			CvPoint2D32f ptSrc = cvPoint2D32f( ptDiff.x + ptSrcOffSet.x,
											   ptDiff.y + ptSrcOffSet.y );			
			return ptSrc;
		};

		//给定在源坐标系里的rect，得到在目标坐标系的rect位置
		CvRect getDstRect4SrcRect( const CvRect rectSrc )
		{
			CvPoint2D32f ptSrc1, ptSrc2, ptDst1, ptDst2;
			ptSrc1 = cvPoint2D32f( rectSrc.x, rectSrc.y );
			ptSrc2 = cvPoint2D32f( rectSrc.x + rectSrc.width - 1,
								   rectSrc.y + rectSrc.height - 1 );
			ptDst1 = getDstCoord4SrcCoord( ptSrc1 );
			ptDst2 = getDstCoord4SrcCoord( ptSrc2 );

			CvRect rct = cvRect( (int) ptDst1.x, (int) ptDst1.y,
								 (int) (ptDst2.x - ptDst1.x + 1), 
								 (int) (ptDst2.y - ptDst1.y + 1) );
			return rct;
		};


		//给定在目标坐标系里的rect，得到在源坐标系的rect位置
		CvRect getSrcRect4DstRect( const CvRect rectDst )
		{
			CvPoint2D32f  ptDst1, ptDst2, ptSrc1, ptSrc2;
			ptDst1 = cvPoint2D32f( rectDst.x, rectDst.y );
			ptDst2 = cvPoint2D32f( rectDst.x + rectDst.width - 1,
								   rectDst.y + rectDst.height - 1 );
			ptSrc1 = getSrcCoord4DstCoord( ptDst1 );
			ptSrc2 = getSrcCoord4DstCoord( ptDst2 );

			CvRect rct = cvRect( (int) ptSrc1.x, (int) ptSrc1.y,
							 	 (int) (ptSrc2.x - ptSrc1.x + 1), 
				                 (int) (ptSrc2.y - ptSrc1.y + 1) );
			return rct;
		};

	}Pt2D32fCoordConvert;
#endif


//Tsai标定的应用
typedef struct struTsaiCaliApp
{
public:
	struTsaiCaliApp( void )
	{
		mvInitTsaiCaliAppVar( );
	}

	//修正标定点(第5，6点)
	bool mvModifyCalibratePoint(
			CvSize szCam,			//相机的光点个数(列，行)
			int &nHomographyPtNum,   //给定的标定点个数
			float fAImgHomegraphy[], //给定的标定点的图像坐标
			float fAWrdHomegraphy[], //给定的标定点的世界坐标
			char *pFN1, char *pFN2   //存的文件名称
		);

private:
	//对Tsai标定应用进行初始化
	bool mvInitTsaiCaliApp( CvSize szCam, int nHomographyPtNum,
			float fAImgHomegraphy[], float fAWrdHomegraphy[] );


	//得到Tsai标定的结果
	bool mvGetTsaiCalibrateResult(
			CvSize szCam,			//相机的光点个数(列，行)
			int nHomographyPtNum,   //给定的标定点个数
			float fAImgHomegraphy[], //给定的标定点的图像坐标
			float fAWrdHomegraphy[], //给定的标定点的世界坐标
			char *pChSaveFileName //保存的图像名称
		);

	//重新修正Tsai标定的结果
	bool mvRemodifyTsaiCalibrate(
			CvSize szCam,			//相机的光点个数(列，行)
			int &nHomographyPtNum,   //给定的标定点个数
			float fAImgHomegraphy[], //给定的标定点的图像坐标
			float fAWrdHomegraphy[]  //给定的标定点的世界坐标
		);


	//对标定应用的变量进行初始化
	void mvInitTsaiCaliAppVar( );

	//得到各世界点在图像坐标系中的坐标,计算两标定方法计算出来的误差
	float mvCalcDiffValOf2Calbration( cal *pCamCalib, 
			 float fMinX_w, float fMinY_w,
			 float fMaxX_w, float fMaxY_w );


	//Tsai标定的图像/世界坐标结果
	bool mvShowImgWorTsaiResult( CvSize szCam );

private:
	cal	  m_cCamCalib; 
	float m_homography_i2w[3][3];
	float m_homography_w2i[3][3];

	bool m_bInitTsaiCaliApp;

	CvSize m_szCamera;
	float  m_fInitSumDiff;

}MvStruTsaiCaliApp;


class CCalCoord  //标定及坐标转换
{
public:
	CCalCoord( void );
	~CCalCoord( void );
	void initVar( );

	//获取标定信息进行初始化
	bool mvInitCalCoord( IplImage *pUseGrayImg, int nHomographyPtNum,
		                 float fAImgHomegraphy[], float fAWrdHomegraphy[],
			             int nRoadCnt = 0, CvPoint2D32f *pPtCet = NULL,
					     int *pKinds = NULL, int *pPtCnt = NULL, 
					     CvPoint2D32f **ppPts = NULL );
	bool mvUninitCalCoord( );

	//得到给定点的运动方向
	float mvGetWorldOriOfGivePoint( CvPoint pt_cet_i, float fImgMoveAlpah );


	//图像和世界坐标的转换
	static CvPoint2D32f mvCoordTransform( CvPoint2D32f src,
									float homography[3][3] );

	//根据给点地面图像点，得到其对应高度与其的差值点
	void mvGetImgDiffPtWithBotPt_High( CvPoint2D32f ptBot_i, 
						float fHight, CvPoint2D32f &diffPt );

	//由旋转角度得到旋转矩阵
	void mvGetRotationMat( float fRotArc, float fRotationMat[3][3] );

	//void mvGet2DProjectOf3DModel( CvPoint pt_cet_i );
public:
	cal	  m_cCamCalibration; 
	float m_homography_i2w[3][3];
	float m_homography_w2i[3][3];

	//计算的车辆宽高图像
	IplImage *m_pCalCarWImg, *m_pCalCarHImg;
	
	//计算的行人宽高图像
	IplImage *m_pCalPeoWImg, *m_pCalPeoHImg;

	//计算出的世界坐标图像
	IplImage *m_pXWorCoordImg, *m_pYWorCoordImg;

	 //计算出的方向图像（角度）
	IplImage *m_pVaildOriImg;  //标记各点的道路方向是否有效
	IplImage *m_pCalOriImg;    //各点的道路方向图像
private:
	//初始化OpenCV的homegraphy
	bool mvInitOpenCVHomegraphy( int nHomographyPtNum,
		   float fAImgHomegraphy[], float fAWrdHomegraphy[] );

#ifdef SHOW_CALIBRATE_CONGIG 
	bool mvShowCalibrationConfig( IplImage *pUseGrayImg,
		   int nHomographyPtNum, float fAImgHomegraphy[], 
		   float fAWrdHomegraphy[] );
#endif

private:
	//根据标定点来生成整个场景各点的图像方向图
	void mvCreateOriImageWithCalibPoint( IplImage *pUseGrayImg,
										 float fAImgHomegraphy[] );

	//根据道路区域生成整个场景各点的图像方向图(根据)
	void mvCreateOriImageWithMultiRoad( IplImage *pUseGrayImg,
			int nRoadCnt, CvPoint2D32f *pPtCet,
			int *pKinds, int *pPtCnt, CvPoint2D32f **ppPts );

#ifdef SHOW_ORI_CALC_RESULT 
	//显示计算出来的图像方向结果
	void CCalCoord::mvShowOriCalResult( IplImage *pUseGrayImg );
#endif

	//根据平面模型计算代表每点车辆尺寸的边长图
	void mvSetGiveObjSzImgByPlaneModel( CvSize szImg,
							IplImage *pObjSzRadiusImg );

	//根据目标三维模型生成代表每点目标尺寸的map图
	void mvSetGiveObjSzImgBy3DModel( IplImage *oriImage, 
		       IplImage *max_sizex, IplImage *max_sizey,
			   int nObjMod = SMALL_VEHICLE_3DMODEL );

#ifdef SHOW_CARSZ_CALC_RESULT 
	//显示计算出来的目标大小
	void mvShowCalObjSz(IplImage *max_sizex, IplImage *max_sizey,
						char *pWinName=NULL);
#endif

	//获得3数组中最小和最大值
	void mvGetMinMaxValOf3Array( int n1, CvPoint2D32f pt[], 
		int n2, CvPoint2D32f p2lt[], int n3, CvPoint2D32f ptlt[],
		int nMod, float &tempmin, float &tempmax );

private:
	bool m_bInitCalCoord;
		
};


#endif
