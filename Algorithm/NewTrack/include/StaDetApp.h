#ifndef __STA_DET_H
#define __STA_DET_H

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
typedef struct StructQueueLenStaApp
{
public:
	//对进行排队长度计算的网格进行过滤
	static void mvGetFiltrateQueueLenghtGrids(	
			vector<int> &vectOutQLGridIdx,
			AnImgGridsStru *pImgGrid, 
			vector<int> *pVectGiveGridIdx,
			double dTsNow,
			bool bFiltrateNoExistMove,
			double dPreTh,
			bool bFiltrateIsolate, 
			vector<int> *pVecInAreaGridIdx,
			IplImage *pShowRgbImg
		);

	//获取在在给定的多边形区域内的轨迹序号
	static void mvGetGridsSegmentResult(	
			vector< vector<int> > &vectGridsIdxGroups,  //聚类后的结果(输出)
			vector<CvRect> &vectRectImg,  //聚类后的图像rect结果(输出)
			vector<CvRect> &vectRectWor,  //聚类后的世界rect结果(输出)
			AnImgGridsStru *pImgGrid,     //给定的网格
			int nChanDirMod,              //车道方向模式
			const vector<int> &vectGiveGridIdx,  //给定的网格序号
			float fMinGroupDist2GridSz,   //类间的距离与网格大小的最小比率
			IplImage *pXWorCImg,     //X世界坐标值的图像
			IplImage *pYWorCImg      //Y世界坐标值的图像
		);

}AnQueueLenStaApp;



//end-----------

#endif