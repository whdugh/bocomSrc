#ifndef __TRACKS_APP_H
#define __TRACKS_APP_H

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include "libHeader.h"

#include "comHeader.h"  //放最后

using namespace std;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_AN_ANLGORITHM;
using namespace MV_SMALL_FUNCTION;


//获取在区域内的轨迹的应用
typedef struct StructTracksInAreaApp
{
public:
	//获取在在给定的多边形区域内的轨迹序号
	static bool mvGetInPolygonTracks(
			vector<int> &vecInAreaTrIdx,  //在区域内的轨迹序号
			int nGiveTrNum,				  //当前非空轨迹数目
			int *nAGiveTrIdx,			  //当前非空轨迹序号
			MyTrackElem **pATrPointer,	  //所有轨迹的地址 
			int nPolygonPtCnt,            //多边形的顶点数
			CvPoint2D32f *pPolygonPts,    //多边形的的各顶点
			int nMinTrLenThresh			  //轨迹的最小记录次数要求
		);

	//获取在给定的长方形区域内的轨迹序号
	static bool mvGetInRectangleTracks(
			vector<int> &vectInAreaTrIdx,	   //在区域内的轨迹序号
			CvPoint &ptTracksLt,				   //在区域内的轨迹形成的左上点
			CvPoint &ptTracksRb,				   //在区域内的轨迹形成的右下点
			const vector<int> &vectGiveTrIdx,  //给定的轨迹序号
			MyTrackElem **pATrPointer,		   //所有轨迹的地址 
			const CvPoint &ptLt,		       //多边形的顶点数
			const CvPoint &ptRb,		       //多边形的的各顶点
			int nMinTrLenThreshold		       //轨迹的最小记录次数要求
		);

}AnTrsInAreaApp;


//轨迹区域的应用
typedef struct StructTracksAreaApp
{
public:
	//获取给定的轨迹所形成的左上点和右下点(图像和世界坐标)
	static bool mvGetLtRbPt4Tracks(
			CvPoint &ptTracksLtImg,		//在区域内的轨迹形成的图像坐标左上点
			CvPoint &ptTracksRbImg,		//在区域内的轨迹形成的图像坐标右下点
			CvPoint2D32f &ptTracksLtWor,  //在区域内的轨迹形成的世界坐标左上点
			CvPoint2D32f &ptTracksRbWor,  //在区域内的轨迹形成的世界坐标右下点
			const vector<int> &vectGiveTrIdx,  //给定的轨迹序号
			MyTrackElem **pATrPointer,	//所有轨迹的地址 
			IplImage *pXWorCImg,		//X世界坐标值的图像
			IplImage *pYWorCImg			//Y世界坐标值的图像
		);

	//显示给定轨迹
	static void mvShowGiveTracks(
			const vector<int> &vectGiveTrIdx,  //给定的轨迹序号
			MyTrackElem **pATrPointer,		   //所有轨迹的地址 
			IplImage *pShowRgbImg,             //显示的rgb图像
			CvRect rectShowRoi,         //显示在的ROI区域
			CvScalar ColorVal,          //颜色       
			int nRadius,                //半径
			int nThickness              //厚度
		);

}AnTrsAreaApp;

//轨迹的速度应用
typedef struct StructTracksVelocityApp
{
public:
	//计算出给定的轨迹的x,y世界坐标的速度
	static bool mvCalcTracksWorVeloXY(	
			vector<CvPoint2D32f> &vectWorVeloXYVal,  //计算出来的XY世界速度值(输出)
			const vector<int> &vecGiveTrIdx,  //给定的轨迹序号
			MyTrackElem **pATrPointer,		  //所有轨迹的地址 
			int nMaxCalcTrLen,				  //计算的轨迹长度
			IplImage *pXWorCImg,			  //X世界坐标值的图像
			IplImage *pYWorCImg				  //Y世界坐标值的图像
		);

	//计算出给定的轨迹的x,y世界坐标的按直方图分布的绝对速度
	static bool mvCalcTracksAbsWorVeloXY4Distribute(	
			vector<CvPoint2D32f> &vectWorVeloXYVal,  //计算出来的XY世界速度值(输出)
			const vector<double> &vectGiveDistVal,   //给定的需计算的分布概率值
			const vector<int> &vecGiveTrIdx,  //给定的轨迹序号
			MyTrackElem **pATrPointer,		  //所有轨迹的地址 
			int nMaxCalcTrLen,				  //计算的轨迹长度
			IplImage *pXWorCImg,			  //X世界坐标值的图像
			IplImage *pYWorCImg				  //Y世界坐标值的图像
		);

	//获取得到速度较慢的轨迹(排除掉地面轨迹)
	static bool mvGetSlowTracks(	
			vector<int> &vectSlowTrIdx,        //获取到的速度较慢的轨迹序号
			const vector<int> &vectGiveTrIdx,  //给定的轨迹序号
			MyTrackElem **pATrPointer,		   //所有轨迹的地址 
			IplImage *pXWorCImg,               //图像点所对应的X世界坐标
			IplImage *pYWorCImg,               //图像点所对应的Y世界坐标 
			float fVeloTh4Slow,                //认为缓慢的速度阈值
			IplImage *m_pInteMvStCntImg        //存在运动现停止的点的积分图
		);

}AnTrsVelocityApp;


//end-----------

#endif