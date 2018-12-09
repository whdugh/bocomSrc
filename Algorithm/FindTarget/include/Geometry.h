// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cxcore.h>
#include <cv.h>
#include <vector>
#include "StdAfx.h"

//using namespace std;
typedef struct _LineInfoInRect
{
	float lineLenInRect;  //直线在Rect内的部分的长度！
	float rate; //直线在Rect内部分占整个线段长度的比例！
	CvPoint pt1, pt2;  //直线在Rect内的两点！
}LineInfoInRect;

class Geometry
{
public:
	static bool    IsTwoRectInteracting(CvRect r1, CvRect r2);
	static double  IsTwoRectPosMatch(const CvRect &r1, const CvRect &r2);
	static void    IsTwoRectPosMatch(const CvRect &r1, const CvRect &r2, double &ret1, double &ret2);
	static double  IsTwoRectPosMatchXY(const CvRect &r1, const CvRect &r2, double &ret1, double &ret2);
	static double  IsTwoRectLeftMatch(const CvRect &r1, const CvRect &r2);
	static double  IsTwoRectRightMatch(const CvRect &r1, const CvRect &r2);
	static CvRect  GetBoundingRect(const CvRect &r1, const CvRect &r2);
	static double  GetTwoRectsCenterDis(const CvRect &r1, const CvRect &r2);
	static void    GetTwoRectsBoudaryMinDis(const CvRect &r1, const CvRect &r2, double &xdis, double &ydis);
	static bool    GetTwoLinesUnion(int l1x1, int l1x2, int l2x1, int l2x2, float &fU1, float &fU2);  //两个一维水平线的相交情况！
	static CvRect  GetCenRect(const CvRect &r1, CvPoint CenPoin);
	// 计算两个Rect的交集Rect             
	inline static bool    GetUnionRect(CvRect r1, CvRect r2, CvRect &ret)
	{
		int lx = MAX_OF_TWO(r1.x, r2.x);
		int rx = MIN_OF_TWO(r1.x+r1.width-1, r2.x+r2.width-1);
		int ty = MAX_OF_TWO(r1.y, r2.y);
		int by = MIN_OF_TWO(r1.y+r1.height-1, r2.y+r2.height-1);

		if (rx <= lx || ty >= by)
		{
			return false;
		}

		ret.x = lx;
		ret.y = ty;
		ret.width = rx-lx + 1;
		ret.height= by-ty + 1;
		return true;
	}
	
	inline static void    GetBoundRect(CvRect r1, CvRect r2, CvRect &ret)
	{
		int lx = MIN_OF_TWO(r1.x, r2.x);
		int rx = MAX_OF_TWO(r1.x+r1.width-1, r2.x+r2.width-1);
		int ty = MIN_OF_TWO(r1.y, r2.y);
		int by = MAX_OF_TWO(r1.y+r1.height-1, r2.y+r2.height-1);

		ret.x = lx;
		ret.y = ty;
		ret.width = rx-lx + 1;
		ret.height= by-ty + 1;

	}

	// 计算两个向量的夹角。如果有一个向量是0向量，抛出异常。
	static double   GetAngleBetweenTwoVector(double vx1, double vy1, double vx2, double vy2);
	static bool    IsRectAContainsB(const CvRect &a, const CvRect &b);
	static bool    IsPointInRect(CvPoint2D32f p, const CvRect &r);
	static bool    IsPointInRect(CvPoint p, const CvRect &r);
	static bool    IsLinePassRect(const CvPoint &p1, const CvPoint &p2, const CvRect &rect);

//	template <class T>
	static bool    IsTwoLineSegCross(const CvPoint &line1start, const CvPoint &line1end, const CvPoint &line2s, const CvPoint &line2e);
	static bool    IsTwoLineSegCross(const CvPoint2D64f &line1start, const CvPoint2D64f &line1end, const CvPoint2D64f &line2s, const CvPoint2D64f &line2e);
	//判断线段与矩形间的几何关系！返回：0，线段在矩形外；1，线段在矩形内；2.线段与矩形相交！
	static int     RelationBtwLineAndRect(CvPoint line_start, CvPoint line_end, CvRect rect, LineInfoInRect &lineInRect, int &nIntersectionNum);
	//取得直线在Rect中的部分，包括端点、长度及其占线段总长度的比例！
	static void    GetLineInRect(CvPoint line_start, CvPoint line_end, CvRect rect, LineInfoInRect &lineInRect);
	static float   CrossMultiply(CvPoint p1, CvPoint p2, CvPoint p0);
	static float   CrossMultiply(CvPoint2D64f p1, CvPoint2D64f p2, CvPoint2D64f p0);
	static int    PointtoRectDis(CvPoint p, const CvRect &r);

	/**
	* Compute the intersection between two line segments, or two lines
	* of infinite length.
	* 
	* @param  x0              X coordinate first end point first line segment.
	* @param  y0              Y coordinate first end point first line segment.
	* @param  x1              X coordinate second end point first line segment.
	* @param  y1              Y coordinate second end point first line segment.
	* @param  x2              X coordinate first end point second line segment.
	* @param  y2              Y coordinate first end point second line segment.
	* @param  x3              X coordinate second end point second line segment.
	* @param  y3              Y coordinate second end point second line segment.
	* @param  intersection[2] Preallocated by caller to double[2]
	* @return -1 if lines are parallel (x,y unset),
	*         -2 if lines are parallel and overlapping (x, y center)
	*          0 if intersection outside segments (x,y set)
	*         +1 if segments intersect (x,y set)
	*/
	static int     GetLineSegIntersection(double x0, double y0, double x1, double y1,
						double x2, double y2, double x3, double y3, double* intersection);


	// -1，rect在线的左边/下边。0相交。1上边/右边
	static int     GetPosRelationBetweenLineAndRect(CvPoint pt1, CvPoint pt2, CvRect rect, bool bFlag = false, float fScalWidht = 0.2 );


	// r1 = r1-r2;类似于集合相减。
	// 只考虑两个rect高度与y相同，且两个rect必须相交
	static CvRect  SubtractRect(CvRect &r1, const CvRect &r2);

	static int JudgeXUnit(CvRect Src, CvRect Dst, float fXRioTrd=0.5 ,bool IsGetRio = true,bool ComparSrc = false );

	static bool JudgeYUnit(CvRect Src, CvRect Dst,float fYRioTrd);

	// 
	static void  SubtractRect(CvRect r1, CvRect r2, std::vector<CvRect> &ret);


	static void  SubtractRect(std::vector<CvRect> v1, std::vector<CvRect> v2, std::vector<CvRect> &ret);


	// 将一个rect分为两个.如果切的位置和rect不相交则返回false。
	// 否则返回true，并且dest存储两个切出来的rect。
	static bool    SplitRect(CvRect src, int splitPos, CvRect *dest);
	
	// 将一组rect切开。rects里面存着互不重叠的一些rect。
	static void    SplitRects(std::vector<CvRect> &rects, int nSplitPos);
	
	// 尝试将一组互不重叠的rects切一下，返回值标记有没有切中。
	// 如果切中aSplitedRes中存储被切中后分裂开的两个rect
	static bool    TrySplitRect(const std::vector<CvRect> &rects, int nSplitPos, CvRect aSplitedRes[2]);


	static bool    IsTwoPointClose(const CvPoint &p1, const CvPoint &p2, float dis);


	// 缩放rect。
	// rect.x *= fScaleX;
	// rect.w *= fScaleX;
	// rect.y *= fScaleY;
	// rect.h *= fScaleY;
	static void    ResizeRect(CvRect &rect, float fScaleX, float fScaleY);

	//计算点到直线的距离
	static float   CalculateDistanceBetweenPoint2Line(const CvPoint &pt,
												   const CvPoint &pt1OfLine,const CvPoint &pt2OfLine);

	static float   CalculateDistanceBetweenPoint2Line(const CvPoint2D64f &pt,
												   const CvPoint2D64f &pt1OfLine,const CvPoint2D64f &pt2OfLine);
	//计算两点之间的距离
	static float	   CalculateDistanceBetween2Points(const float &x1, const float &y1, const float &x2, const float &y2);


	static CvRect      GetBoudingRect(const std::vector<CvPoint> &pts);

	static CvRect      GetBoudingRect(CvRect r, CvPoint pt);

	static bool IsXDir(const CvRect &r1, const CvRect &r2);

	static int PoinInLine(const CvPoint &p1, const CvPoint &p2,const CvPoint &point);




	///////////////////////////////////////////////////////////////////////
	//  参数:
	//     double P1X,P1Y             -线段的起点坐标
	//	   double P2X,P2Y             -线段的终点坐标
	//     int    nHeight,nWidth      -图像的高和宽
	//     double w                   -线段的宽度
	//     int    &nPixelNum          -线段上的像素点数
	//	   int    *nPixelXY           -线段上的像素位置
	//  说明:
	//     该函数实现获取到在由两点所确定的线段上的所有像素点(对图像)      \
	//   by hyp
	/////////////////////////////////////////////////////////////////////////
	static bool        GetPixelsOnLine(double P1X, double P1Y, double P2X, double P2Y, 
		int nHeight, int nWidth, double w, int &nPixelNum, int *nPixelYX);


	// r1和r2是否在x方向是比较靠近。
	static bool        IsTwoRectCloseX(const CvRect &r1, const CvRect &r2, int dis);

	// r1和r2是否在y方向是比较靠近。
	static bool        IsTwoRectCloseY(const CvRect &r1, const CvRect &r2, int dis);

	static CvPoint   GetRectCentre(const CvRect &Rec);

	static CvRect  ExpandRec (const CvRect &Rec,float ExpRio);
};


#endif
