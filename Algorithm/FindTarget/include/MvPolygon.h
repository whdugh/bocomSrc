// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#if !defined(AFX_POLYGON_H__07674F13_FDD1_4483_A554_504BE8732732__INCLUDED_)
#define AFX_POLYGON_H__07674F13_FDD1_4483_A554_504BE8732732__INCLUDED_




#include <vector>
#include <cxcore.h>
#include <list>

class MvPolygon  
{
public:

	// 多边形的顶点
	std::vector<CvPoint> points;
	
	// 构造函数
	MvPolygon() {};
	MvPolygon(CvPoint pts[], int ptCount);
	MvPolygon(const std::vector<CvPoint> &pts);
	MvPolygon(const std::list<CvPoint> &pts);

	// 判断点在不在多边形里面。
	int IsPointInPoly(const CvPoint &pt) const;
	
	//获取主方向角度
	float  GetMainAngle();
	CvRect GetBoudingRect();

	void   DrawPoly(IplImage *img, CvScalar color)const;

	// 获取多边形的顶点。第一个和最后一个不是同一点。外部释放
	CvPoint *GetPointPointer(int &nCount);

};

#endif
