// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef OPTICAL_FLOW
#define OPTICAL_FLOW

#include <cxcore.h>

class Flow
{
public:
	
	Flow()
	{
		start  = cvPoint2D64f(0,0);
		end    = cvPoint2D64f(0,0);
		wstart = cvPoint2D64f(0,0);
		wend   = cvPoint2D64f(0,0);
	}

	Flow(const Flow &f)
	{
		start = f.start;
		end   = f.end;
		wstart= f.wstart;
		wend  = f.wend;
	}

	Flow(CvPoint2D64f s, CvPoint2D64f e)
	{
		start  = s;
		end    = e;
		wstart = cvPoint2D64f(0,0);
		wend   = cvPoint2D64f(0,0);
	}

	Flow(CvPoint2D64f is, CvPoint2D64f ie, CvPoint2D64f ws, CvPoint2D64f we)
	{
		start  = is;
		end    = ie;
		wstart = ws;
		wend   = we;
	}

	//flow 的起点和终点。图像坐标还是世界坐标未知。
	CvPoint2D64f start; 
	CvPoint2D64f end;

	CvPoint2D64f wstart;
	CvPoint2D64f wend;

	// 判断两个光流是否同向，同模。后面两个参数是阈值。
	bool static IsSameDirectionAndSize(const Flow &f1, const Flow &f2, float fDeltaAngle=10, float fDeltaLen=20, bool bUseWorldCoor = false);


};


#define DRAW_FLOW(flow, img, color) \
	cvDrawLine(img, \
	cvPoint((flow).start.x, (flow).start.y), \
	cvPoint((flow).end.x, (flow).end.y), \
	color, \
	1);


#define RESIZE_FLOW(flow, fsx, fsy) \
	(flow).start.x  *= fsx; \
	(flow).start.y  *= fsy; \
	(flow).end.x    *= fsx; \
	(flow).end.y    *= fsy; \
	(flow).wstart.x *= fsx; \
	(flow).wstart.y *= fsy; \
	(flow).wend.x   *= fsx; \
	(flow).wend.y   *= fsy;
#endif