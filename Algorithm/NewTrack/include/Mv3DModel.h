#ifndef __MV3DMODEL_H
#define __MV3DMODEL_H

#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "libHeader.h"

#include "MvCalCoord.h" //tsai

#include "comHeader.h"  //放最后

using namespace MV_IMGPRO_UTILITY;

#define  MAX_LEVEL_NUM 10
#define  MAX_PTNUM_PER_LEVEL 10

enum ObjType3DModel
{
	STANDARD_BUS_3DMODEL = 0,
	GIVE_BUS_3DMODEL,
	BIG_TRUCK_3DMODEL = 10,
	OTHER_DEFAULT_3DMODEL
};

//定义一个较为广泛的3D模型
typedef struct _MyWorld3DModel
{
	int nMod;
	int nLevel;   //模型分几层
	int nALevelPtCnt[MAX_LEVEL_NUM]; //模型每层的顶点数
	CvPoint3D32f w_ptA2LevelVert[MAX_LEVEL_NUM][MAX_PTNUM_PER_LEVEL]; //世界坐标各顶点的坐标
	CvPoint2D32f i_ptA2LevelVert[MAX_LEVEL_NUM][MAX_PTNUM_PER_LEVEL]; //图像坐标各顶点的坐标
	bool bA2Visable[MAX_LEVEL_NUM][MAX_PTNUM_PER_LEVEL]; //各顶点的是否可见

	_MyWorld3DModel( )
	{
		nMod = 0;
		nLevel = 0;
	};

	_MyWorld3DModel( int n3DModel )
	{
		switch( n3DModel )
		{
		case STANDARD_BUS_3DMODEL:
			{
				nMod = STANDARD_BUS_3DMODEL;
				nLevel = 2; //标准Bus分两层:底部和顶部
				nALevelPtCnt[0] = nALevelPtCnt[1] = 4; //每层4个顶点
				float fHalfWidth = 1.25f;
				float fHalfLength = 5.5f;
				float fHight = 2.5f;
				//标准Bus的世界坐标顶点顺序是这样定义的: 0...1
				//                                        点O
				//                                       3...2
				w_ptA2LevelVert[0][0] = cvPoint3D32f( -fHalfWidth, -fHalfLength, 0.0f );
				w_ptA2LevelVert[0][1] = cvPoint3D32f(  fHalfWidth, -fHalfLength, 0.0f );
				w_ptA2LevelVert[0][2] = cvPoint3D32f(  fHalfWidth,  fHalfLength, 0.0f );
				w_ptA2LevelVert[0][3] = cvPoint3D32f( -fHalfWidth,  fHalfLength, 0.0f );

				w_ptA2LevelVert[1][0] = cvPoint3D32f( -fHalfWidth, -fHalfLength, fHight );
				w_ptA2LevelVert[1][1] = cvPoint3D32f(  fHalfWidth, -fHalfLength, fHight );
				w_ptA2LevelVert[1][2] = cvPoint3D32f(  fHalfWidth,  fHalfLength, fHight );
				w_ptA2LevelVert[1][3] = cvPoint3D32f( -fHalfWidth,  fHalfLength, fHight );


				//默认标准Bus的各点均可见
				bA2Visable[0][0] = true;
				bA2Visable[0][1] = true;
				bA2Visable[0][2] = true;
				bA2Visable[0][3] = true;

				bA2Visable[1][0] = true;
				bA2Visable[1][1] = true;
				bA2Visable[1][2] = true;
				bA2Visable[1][3] = true;
				break;
			}
		case BIG_TRUCK_3DMODEL:
			{
				break;
			}
		case OTHER_DEFAULT_3DMODEL:
			{
				break;
			}
		default:
			{
				nLevel = 0;
				break;
			}
		}
	};

	_MyWorld3DModel( int _nLevel, int *_p_nLevelPtCnt, 
	   CvPoint3D32f **_p_w_ptLevelVert, bool **_p_bVisable )
	{
		nLevel = MIN( MAX_LEVEL_NUM, _nLevel );

		int i, j;
		for( i=0; i<nLevel; i++ )
		{
			nALevelPtCnt[i] = MIN( MAX_PTNUM_PER_LEVEL, _p_nLevelPtCnt[i] );
		}

		for( i=0; i<nLevel; i++ )
		{
			for( j=0; j<nALevelPtCnt[i]; j++ )
			{
				w_ptA2LevelVert[i][j] = _p_w_ptLevelVert[i][j];
				bA2Visable[i][j] = _p_bVisable[i][j];
			}
		}
	};


	//得到一个Bus模型(在世界坐标)
	void getBusModel( float fHalfWidth, float fHalfLength, float fHight )
	{
		nMod = GIVE_BUS_3DMODEL;
		nLevel = 2; //Bus分两层:底部和顶部
		nALevelPtCnt[0] = nALevelPtCnt[1] = 4; //每层4个顶点

		//标准Bus的世界坐标顶点顺序是这样定义的: 0...1
		//                                        点O
		//                                       3...2
		w_ptA2LevelVert[0][0] = cvPoint3D32f( -fHalfWidth, -fHalfLength, 0.0f );
		w_ptA2LevelVert[0][1] = cvPoint3D32f(  fHalfWidth, -fHalfLength, 0.0f );
		w_ptA2LevelVert[0][2] = cvPoint3D32f(  fHalfWidth,  fHalfLength, 0.0f );
		w_ptA2LevelVert[0][3] = cvPoint3D32f( -fHalfWidth,  fHalfLength, 0.0f );

		w_ptA2LevelVert[1][0] = cvPoint3D32f( -fHalfWidth, -fHalfLength, fHight );
		w_ptA2LevelVert[1][1] = cvPoint3D32f(  fHalfWidth, -fHalfLength, fHight );
		w_ptA2LevelVert[1][2] = cvPoint3D32f(  fHalfWidth,  fHalfLength, fHight );
		w_ptA2LevelVert[1][3] = cvPoint3D32f( -fHalfWidth,  fHalfLength, fHight );

		//默认标准Bus的各点均可见
		bA2Visable[0][0] = true;
		bA2Visable[0][1] = true;
		bA2Visable[0][2] = true;
		bA2Visable[0][3] = true;

		bA2Visable[1][0] = true;
		bA2Visable[1][1] = true;
		bA2Visable[1][2] = true;
		bA2Visable[1][3] = true;
	};


	//将模型旋转(在世界坐标)
	void rotation( float fRotArc )
	{
		float fSin = sin( fRotArc );
		float fCos = cos( fRotArc );
		float x, y, rx, ry;
		int   i, j;
		for( i=0; i<nLevel; i++ )
		{
			for( j=0; j<nALevelPtCnt[i]; j++ )
			{
				x = w_ptA2LevelVert[i][j].x;
				y = w_ptA2LevelVert[i][j].y;
			
				rx = x*fCos - y*fSin;
				ry = x*fSin + y*fCos;

				w_ptA2LevelVert[i][j].x = rx;
				w_ptA2LevelVert[i][j].y = ry;
			}
		}	
	};

	//将模型平移(在世界坐标)
	void moveXY( float fMvX, float fMvY )
	{
		float x, y, mx, my;
		int   i, j;
		for( i=0; i<nLevel; i++ )
		{
			for( j=0; j<nALevelPtCnt[i]; j++ )
			{
				x = w_ptA2LevelVert[i][j].x;
				y = w_ptA2LevelVert[i][j].y;

				mx = x + fMvX;
				my = y + fMvY;

				w_ptA2LevelVert[i][j].x = mx;
				w_ptA2LevelVert[i][j].y = my;
			}
		}		
	};


	//将模型的顶点坐标保存(在世界坐标)
	void saveModelPtCoord( char cFileName[] )
	{
		FILE *fp_save = NULL;
		fp_save = fopen( cFileName, "w" );
		if( NULL != fp_save )
		{
			int  i, j;
			for( i=0; i<nLevel; i++ )
			{
				fprintf( fp_save, "level=%d,ptCnt=%d\n", i, nALevelPtCnt[i]);
				for( j=0; j<nALevelPtCnt[i]; j++ )
				{
					fprintf( fp_save, "%.2f,%.2f,%.2f\n", w_ptA2LevelVert[i][j].x,
					         w_ptA2LevelVert[i][j].y, w_ptA2LevelVert[i][j].z );
				}
			}	
			fclose( fp_save );
		}
	};


	//将模型的顶点坐标保存(在世界坐标)
	void saveModelPtCoordForMatlab( char cFileName[] )
	{
		FILE *fp_save = NULL;
		fp_save = fopen( cFileName, "w" );
		if( NULL != fp_save )
		{
			int  i, j;
			for( i=0; i<nLevel; i++ )
			{
				fprintf( fp_save, "x%d=[", i );
				for( j=0; j<nALevelPtCnt[i]; j++ )
				{
					fprintf( fp_save, "%.2f,", w_ptA2LevelVert[i][j].x );
				}
				fprintf( fp_save, "%.2f];\n", w_ptA2LevelVert[i][0].x );

				fprintf( fp_save, "y%d=[", i );
				for( j=0; j<nALevelPtCnt[i]; j++ )
				{
					fprintf( fp_save, "%.2f,", w_ptA2LevelVert[i][j].y );
				}
				fprintf( fp_save, "%.2f];\n", w_ptA2LevelVert[i][0].y );

				fprintf( fp_save, "z%d=[", i );
				for( j=0; j<nALevelPtCnt[i]; j++ )
				{
					fprintf( fp_save, "%.2f,", w_ptA2LevelVert[i][j].z );
				}
				fprintf( fp_save, "%.2f];\n", w_ptA2LevelVert[i][0].z );
			}	
			if( STANDARD_BUS_3DMODEL == nMod )
			{				
				for( j=0; j<nALevelPtCnt[0]; j++ )
				{
					fprintf( fp_save, "x_l%d=[", j );					
					fprintf( fp_save, "%.2f,%.2f];\n", w_ptA2LevelVert[0][j].x, w_ptA2LevelVert[1][j].x);
					fprintf( fp_save, "y_l%d=[", j );					
					fprintf( fp_save, "%.2f,%.2f];\n", w_ptA2LevelVert[0][j].y, w_ptA2LevelVert[1][j].y);
					fprintf( fp_save, "z_l%d=[", j );					
					fprintf( fp_save, "%.2f,%.2f];\n", w_ptA2LevelVert[0][j].z, w_ptA2LevelVert[1][j].z);
				}					
			}
			fclose( fp_save );
		}
	};

}General3DModel;



class CMy3DModel
{
public:
	CMy3DModel( void );
	~CMy3DModel( void );

	bool mvInit3DModel(  );
	void mvInit3DModelVar(  );

	bool mvUninit3DModel( );

	//获取标定信息
	bool mvGetCalibrationInfo( CvSize sz_srcImg, int nHomographyPtNum,
		             float fAImgHomegraphy[], float fAWrdHomegraphy[] );
	
	//主接口
	bool mvGet3DModel( CCalCoord *pCCalCoord, IplImage *pUseRgbImg );


public:
	//车辆定位
	bool mvVehcileLocalisation(	
			vector<CvRect> *pVecRect, //当前rect区域
			IplImage *pGrayImg,		  //当前灰度图
			IplImage *pGradXImg=NULL, //x梯度图
			IplImage *pGradYImg=NULL  //y梯度图
			);

private:
	//获取透视投影矩阵
	bool  m_bGetPerspectTM;
	float m_fMatrix[4][4];
	bool mvGetPerspectiveTransformMatrix( );

	//获取兴趣区域的梯度数据
	bool mvGetGradientDataOfROI( 
			vector<CvRect> *pVecRect, //当前rect区域
			IplImage *pGrayImg,		  //当前灰度图	
			IplImage *pGradXImg,	 //x梯度图
			IplImage *pGradYImg  	 //y梯度图
			);

	//确定车辆的角度
	bool mvDeterminateOrientation(
			vector<CvRect> *pVecRect,		 //当前rect区域
			IplImage *pGradXImg,			 //x梯度图
			IplImage *pGradYImg,  			 //y梯度图
			vector< vector<float> >  &vecOri //目标角度
			);

	//确定车辆的位置
	bool mvDeterminateLocation(
			vector<CvRect> *pVecRect,		 //当前rect区域
			IplImage *pGradXImg,			 //x梯度图
			IplImage *pGradYImg,  			 //y梯度图
			vector< vector<CvPoint> >  &vecCetPt //目标中心
			);


private:
	//计算Bus模型的得分
	float mvGetScoreOfBus( General3DModel &bus, CvSize sz,
		      IplImage *pInt_VSobImg, IplImage *pInt_HSobImg );

	//显示Bus模型
	void mvShowBusModel( General3DModel &bus, IplImage *pUseRgbImg );


public:
	CCalCoord *m_p_cCalCoord;  

private:
	bool m_bInit3DModel;

};


#endif