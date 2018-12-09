/* 版权所有 2013 上海博康智能信息技术有限公司
* Copyright (c) 2013，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvImgProUtility.h
* 摘要: 公用的数学处理函数的整理
* 版本: V1.0
* 作者: 贺岳平
* 完成日期: 2013年12月
*/ 
//////////////////////////////////////////////////////////////////////


#ifndef _MV_MATH_UTILITY_H_
#define _MV_MATH_UTILITY_H_

#include "BaseComLibHeader.h"


namespace MV_MATH_UTILITY
{
	//-------------part0(rect)-----------------//			

	//返回c是否在 a 和 b之间
	bool mvIsBetween(double a, double b, double c);	

	//返回给定端点线段的长度
	double mvlengthLine(double x0, double y0, double x1, double y1);

	//线段的长度模值的平方
	float mvSqLineMagnitude(float x1, float y1, float x2, float y2);

	//两点的欧氏距离
	float mvGetEuclidDist(const CvPoint pt1, const CvPoint pt2);	

	//得到从原始点的目标点
	CvPoint2D32f mvGetSrc2DstPoint( CvPoint2D32f fSrcPt,
			  CvPoint2D32f fSrcOffset, float fDst2Src_x,
			  float fDst2Src_y );

	//根据两坐标系的进行点转换
	CvPoint2D32f mvConvertCoordinate( 
			CvPoint2D32f ptLtSrc, CvPoint2D32f ptRbSrc,	
			CvPoint2D32f ptLtDst, CvPoint2D32f ptRbDst, 
			CvPoint2D32f ptSrc );

	//-------------part1(rect)-----------------//			
	// 对两个相交但是尺寸相差较大的矩形进行分离 
	void mvRectSeperate(CvRect &rt1, CvRect &rt2);

	//点是否在rect内
	bool mvIsPointInRect( const CvPoint pt, const CvRect rt );

	//Check if a specified point is inside a specified rectangle.
	bool mvIsPointInsideRectangle( double x0, double y0,
			  double x1, double y1, double x, double y );

	//两rect是否相交
	bool mvIntersectRect( const CvRect rt1, const CvRect rt2 );

	//判断点是否在多边形内
	bool mvIsPointInPolygon( int nPolySize, 
			CvPoint2D32f *pts, CvPoint2D32f pt
		);

	//扩展一个rect区域
	void mvExtendRect( 
			const CvPoint ltPt,   //给定的原始区域左上点
			const CvPoint rbPt,   //给定的原始区域右下点
			const CvSize imgSz,   //区域所限制于的图像尺寸大小
			const CvSize expSz,   //将区域扩展为多大尺寸的区域
			CvPoint &extLtPt,	  //扩展后的区域的左上点
			CvPoint &extRbPt      //扩展后的区域的右下点
		);

	//根据需求来往下扩展一个rect区域
	void mvExtendRectDown( 
			const CvPoint ltPt,   //给定的原始区域左上点
			const CvPoint rbPt,   //给定的原始区域右下点
			const CvSize imgSz,   //区域所限制于的图像尺寸大小
			const CvSize expSz,  //将区域扩展为多大尺寸的区域
			CvPoint &extLtPt,	  //扩展后的区域的左上点
			CvPoint &extRbPt      //扩展后的区域的右下点
		);

	//计算两个矩形框的边缘的最小距离
	void mvCalMinEdgeDistOf2Rectangle(
			CvPoint lt_pt1, CvPoint rb_pt1, 
			CvPoint lt_pt2, CvPoint rb_pt2,
			CvPoint &ptEdgeMinDst );

	//计算两个矩形框的边缘的最小距离
	void mvCalMinEdgeDistOf2Rect(
			CvRect rect1, CvRect rect2,
			CvPoint &ptEdgeMinDst );

	//计算两个矩形框的重叠比率
	float mvCalOverlapPercentOf2Rectangle(
			CvPoint lt_pt1, CvPoint rb_pt1, 
			CvPoint lt_pt2, CvPoint rb_pt2 );
	
	//计算两个rect的重叠比率
	float mvCalOverlapPercentOf2Rect(
			CvRect rect1, CvRect rect2 );

	//获取两点线段和rect的位置关系
	//返回值： -1--rect在线的左边/下边。0--相交。1--上边/右边
	int mvGetPosRelationBetweenLineAndRect(
			CvPoint pt1, CvPoint pt2, CvRect rect, 
			float fWRatio, float fHRatio );



	//-------------part2(line)-----------------//	
	//计算2点的直线方程 a*x + b*y + c = 0
	bool mvGetLineEquationOf2Point(
		float fx1, float fy1, float fx2, float fy2, 
		float &fa, float &fb, float &fc );
	
	//得到在直线:a*x + b*y + c = 0上的给定点附件的点
	CvPoint2D32f* mvGetPtOfLineEquation( 
		float fa, float fb, float fc,  //直线方程系数
		CvPoint2D32f fPtGive,  //直线上的给定点的坐标
		int &nPtCnt );  //要获取多少个点,以返回值为准
		
	//等间隔地获取所有在两端点线段上的点坐标
	bool mvGetEqualIntervalPtsOfLine(CvPoint2D32f line1Pt1,
			  CvPoint2D32f line1Pt2, int nPtCnt,
			  CvPoint2D32f ptA[] );

	//-----------part3(line and other)--------//
	//两变量是否相等
	bool mvequals(double a, double b, double limit);	
	bool mvequals(double a, double b);

    //寻找两线段的交点
	int mvfindLineSegmentIntersection( 
			double x0, double y0,  double x1, double y1,
			double x2, double y2,  double x3, double y3, 
			double* intersection );
	
	
   //检查两点是否在给定端点的线段的同一边
	int mvsameSide(double x0, double y0, double x1, double y1,
			  double px0, double py0, double px1, double py1);

	//检查两给定端点线段是否相交
	bool mvisLineIntersectingLine(
			double x0, double y0, double x1, double y1,
			double x2, double y2, double x3, double y3);

	//获取给定的一点和给定端点的线段的距离
	double mvdistanceBetweenPointLine(
			  double x0, double y0, double x1, double y1,
			  double x, double y);
	double mvDistanceOfPointAndLine(double X,double Y,
			  double AX,double AY,double BX,double BY);

	float mvGetDisToLineSegment( float x1, float y1,
			  float x2, float y2, float px, float py );

	//获取给定端点的两条线段的最小距离
	double mvmindistanceBetweenLines(
		double x0, double y0, double x1, double y1, 
		double x2, double y2, double x3, double y3);

	//获取给定端点的两条线段的最大距离
	double mvmaxDistanceBetweenLines(
		double x0, double y0, double x1, double y1,
		double x2, double y2, double x3, double y3);

	//判断给定端点的线段是否和给定矩形相交
	bool mvisLineIntersectingRectangle(
		double lx0, double ly0, double lx1, double ly1,
		double x0, double y0, double x1, double y1);


	//return PLS distance between a point and a line segment
	double mvgetPointLinePLSDist(CvPoint2D32f pts,
			CvPoint2D32f pt0, CvPoint2D32f pt1);

	//获得给定线段和点的垂直交点
	bool mvintersectPointLine(
			double x0, double y0, double x1, double y1, 
			double x, double y, float*pt);

	//计算拟合直线的斜率
	float mvLineSlope(int ncount,CvPoint* points, float fVxVyX0Y0[4]);

	//最小二乘来对点进行拟合，得到 y = kx+b 的直线方程
	bool mvFitDistL2(double fAkXb[2], int num, CvPoint2D32f * points);

	//-------part3-----------
	//按逆时钟方向(以原点为中心)旋转一个点
	CvPoint2D32f mvRotatePtByInvClockwise(const CvPoint2D32f &pt, float fArc);
	void mvRotatePtByInvClockwise( CvPoint2D32f *pt, float fArc );

	//计算给定的多边形的面积
	float mvCalcAreaOfPolygon( int nPtCnt, CvPoint2D32f *fpts );

	//计算图像中某区域中非零点的个数
	int mvComputeNoZeroNum(IplImage *pImage, CvPoint ptLt, CvPoint ptRb);


	//-------part4-----------

}
#endif
