/* 版权所有 2013 上海博康智能信息技术有限公司
* Copyright (c) 2013，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvAnAlgorithm.h
* 摘要: 公用的通用算法的整理归类
* 版本: V1.0
* 作者: 贺岳平
* 完成日期: 2013年12月
*/ 
//////////////////////////////////////////////////////////////////////

#ifndef _MV_AN_ALGORITHM_H_
#define _MV_AN_ALGORITHM_H_

#include "BaseComLibHeader.h"

#ifndef MAX_UNSHORT_VALUE
	#define	MAX_UNSHORT_VALUE 10000
#endif


namespace MV_AN_ANLGORITHM
{
	//--------------part1.排序--------------//
	//快速排序的算法源程序：从小到大 或 从大到小 排序
	void mvQuickSort( double *data,int *index, 
		   int l, int r, bool bSmallToBig=true );

	int bcmp_func( const void* _a, const void* _b, 
			void* userdata );  //从小到大排序操作
	
	int scmp_func( const void* _a, const void* _b, 
			void* userdata );  //从大到小排序操作

	//快速排序的OpenCV改写算法：从小到大 或 从大到小 排序
	//好像效果有问题
	void quickSort_opencv( double *data,int *index, 
		     int l, int r, bool bSmallToBig=true );

	//对数组array1进行升序排序，n为数组中非空的数目 选择法
	void mvBubbleSort( double* array1,int n );

	//--------------part2.查找--------------//
	//对有序数组(从小到大)进行折半查找在某一范围内的数据序号
	bool  mvBinSearch( double r[], int low, int high, 
			double lowkey, double highkey, 
			int &nLowKeyNo, int &nHighKeyNo );


	//------------part3----------------//

	//对float数组的获取得到其中的局部峰值	
	bool mvGetLocalPeakFromFloatArray( 
		   int &nLocalPeakCnt, int nALocalPeakIdx[],
		   int nCnt, float fAVal[], int nRadiusComCnt );

	//从一维投影数组中寻找到局部峰值所对应的下标
	vector<int> mvGetLocalPeakFromProjectArray(
		   int nCnt, int *nAProjectVal,
		   int nLocalMaxvalTh = -1000, 
		   int nPeakDistance = 1000000 );

	//从一维投影vector中寻找到局部峰值所对应的下标
	vector<int> mvGetLocalPeakFromProjectVector(
			vector<int> &vecProjectVal,
			int nLocalMaxvalTh = -1000, 
			int nPeakDistance = 1000000 );


	//--------------part4--------------//
	//对图像中的点获取得到其从近到远的点序号
	void mvGetNear2FarPtIdxOfImgPt(	
			CvSize szImg,     //图像的大小
			int nPtCnt,       //点的个数
			const CvPoint2D32f *fptA, //点的坐标 
			float fMaxRateX,    //可计算的点间最大x距离（相对图像宽）
			float fMaxRateY,    //可计算的点间最大y距离（相对图像高）
			const float *fAPtMaxCalRateX,   //各点可计算的点间最大x距离（相对图像宽）
			const float *fAPtMaxCalRateY,   //各点可计算的点间最大y距离（相对图像高）
			int  *nASortCnt,		//各点的排序点数
			IplImage *pSortIdxImg,  //各点的排序结果的图像
			int nOneMult        //一个距离可对应几个(相当于链表)	
		);

	//---------------part5--------------//

	//采用密度的方法来将点进行聚类
	bool mvGroupingForPtsWithDensity(
			vector<int> &vPtGroupIdx,		 //OUT:点聚类后的group序号 
			vector<CvPoint> &vGroupingLtPt,  //OUT:点聚类后的group序号
			vector<CvPoint> &vGroupingRbPt,  //OUT:点聚类后的group序号
			vector<CvPoint> &vctPt,			    //IN:给定的点坐标
			const CvSize szImgOfPts,		    //IN:点集所对应的图像大小
			vector<CvPoint> &vSzStdCarWithYCoord  //IN:y坐标所对应的小车大小
		);

	//将重叠较多的group聚在一起
	void mvMegerOverlapGroup(
			vector<CvPoint> vctGroupLtPt, //OUT：聚类后的多个group左上点
			vector<CvPoint> vctGroupRbPt, //OUT：聚类后的多个group右下点
			float fOverlapThres,          //IN：重叠比率阈值
			vector<CvPoint> &vctG_ltPt,   //IN：原先的多个group左上点
			vector<CvPoint> &vctG_rbPt    //IN：原先的多个group左上点
		);
}
#endif