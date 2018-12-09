#ifndef MV_CHANNEL_REGION_H
#define MV_CHANNEL_REGION_H

#include <vector>
#include "StdAfx.h"
#include "MvPolygon.h"
#include "MvPointList.h"
#include "NoPassingInfo.h"
#include "MvLine.h"

using namespace std;

enum ChannelDriveDir
{
	StraightForwardChannel = 0,         //直行车道！
	TurnLeftChannel,                    //左转车道！
	TurnLeftStraightForwardChannel,     //直行+左转车道！
	TurnRightChannel,                   //右转车道！
	TurnRightStraightForwardChannel,    //直行+右转车道！
	Other = -1                          //其他！
};

/*
 车道结构体。
*/
typedef struct _ChannelRegion
{
	
	int      nRoadIndex;	//车道序号
	int      nVerRoadIndex; //车道逻辑序号，暂时没有用处。
	int      nDirection;    //车道方向
	MvLine   vDirection;    //
	MvPointList vListChannel;     //车道区域
	bool     bNoTurnLeft;   //禁左
	bool     bNoTurnRight;  //禁右
	bool     bNoForeward;   //禁止前行
	
	bool     bNoReverse;    // 禁止逆行
	bool     bNoPressLine;  //禁止压线！
	bool     bNoChangeChannel;   //禁止变道！
	bool     bNoPark;           //禁止停车

	//车道行驶属性:直行车道、左转车道、右转车道、左转+直行车道、右转+直行车道,等！
	int      nChannelDriveDir;
	MvLine   vForeLine;  //每车道一个前行线！相对于原图坐标！但是传给MvViolationDetecter的停止线和vDirection方向线都为缩小图坐标，这一点要注意！

	//每个车道一个停止线
	MvLine   vStopLine;


	bool     bHoldSet;//设置待转状态；
	bool     bFlagHoldStopReg;//待转等候状态；
	MvLine   vHoldForeLineFirst;
	MvLine   vHoldForeLineSecond;
	MvLine   vHoldStopLineFirst;
	MvLine   vHoldStopLineSecond;

	CvRect OnOffRed;
	CvRect OnOffGreen;
	
	//防闪用
	CvRect roiLeftLight;
	CvRect roiMidLight;
	CvRect roiRightLight;
	CvRect roiTurnAroundLight;

	//红灯增强用的
	CvRect roiLeftLight_red, roiLeftLight_green;
	CvRect roiMidLight_red, roiMidLight_green;
	CvRect roiRightLight_red, roiRightLight_green;
	CvRect roiTurnAroundLight_red, roiTurnAroundLight_green; 
	
	CvRect rectMedianPos;  //闯红灯或电警时，由客户端指定的第二张图的车辆位置。不可太靠上，否则左转或右转电警就可能取不到！禁行的取图不用吧？

	std::vector<NoPassingInfo>   vecNoPassingInfo; //禁止通过时间段

	int    m_RedLightTime;//该车道的红灯时间长度,以秒为单位.

	_ChannelRegion()
	{
		nRoadIndex		= 0;
		nVerRoadIndex   = 0;
		nDirection      = 0;
		bHoldSet = false;
		bNoTurnLeft     = false;
		bNoTurnRight    = false;
		bNoForeward     = false;
		bNoReverse      = false;
		bFlagHoldStopReg = false;
		bNoPark = false;
		nChannelDriveDir = 0;  //默认为直行车道！
		vForeLine.start = cvPoint(0, 0);
		vForeLine.end   = cvPoint(0, 0);

		//初始化待转停靠线
		vHoldForeLineFirst.start = cvPoint( 0, 0 );
		vHoldForeLineFirst.end = cvPoint( 0, 0 );

		vHoldForeLineSecond.start = cvPoint( 0, 0 );
		vHoldForeLineSecond.end = cvPoint( 0, 0 );

		vHoldStopLineFirst.start = cvPoint( 0, 0 );
		vHoldStopLineFirst.end = cvPoint( 0, 0 );

		vHoldStopLineSecond.start = cvPoint( 0, 0 );
		vHoldStopLineSecond.end = cvPoint( 0, 0 );




		OnOffRed   = cvRect(0, 0, 0, 0);
		OnOffGreen = cvRect(0, 0, 0, 0);

		roiLeftLight = cvRect(0, 0, 0, 0);
		roiMidLight = cvRect(0, 0, 0, 0);
		roiRightLight = cvRect(0, 0, 0, 0);
		roiTurnAroundLight = cvRect(0, 0, 0, 0);

		roiLeftLight_red = cvRect( 0,0,0,0 );
		roiLeftLight_green = cvRect( 0,0,0,0 );

		roiMidLight_red = cvRect( 0,0,0,0 );
		roiMidLight_green = cvRect( 0,0,0,0 );

		roiRightLight_red = cvRect( 0,0,0,0 );
		roiRightLight_green = cvRect( 0,0,0,0 );

		roiTurnAroundLight_red = cvRect( 0,0,0,0 );
		roiTurnAroundLight_green = cvRect( 0,0,0,0 );

		rectMedianPos = cvRect(0, 0, 0, 0);

		m_RedLightTime = 300;//秒
	}


	static _ChannelRegion GetChannelInfoByIndex(const std::vector<_ChannelRegion> &lstChns, int Index)
	{
		std::vector<_ChannelRegion>::const_iterator it;
		for (it=lstChns.begin(); it!=lstChns.end(); it++)
		{
			if (it->nRoadIndex == Index)
			{
				return *it;
			}
		}
		
		assert(false);
		return _ChannelRegion();
	}

	//根据位置获得车道编号！原图像坐标！
	static int GetChannelIndexByPos(const std::vector<_ChannelRegion> &lstChns, CvPoint pt)
	{
		int ret = -1;

		std::vector<_ChannelRegion>::const_iterator it;
		
		for (it=lstChns.begin(); it!=lstChns.end(); it++)
		{
			if (it->IsPointInRegion(pt))
			{
				ret = it->nRoadIndex;
				break;
			}
		}

		return ret;
	}
	
	//根据车道编号获得车道方向！
	static int GetChannelDirByIndex(const std::vector<_ChannelRegion> &lstChns, int Index)
	{
		assert(Index > 0);
		
		int ret = -1;

		std::vector<_ChannelRegion>::const_iterator it;
		for (it=lstChns.begin(); it!=lstChns.end(); it++)
		{
			if (it->nRoadIndex == Index)
			{
				ret = it->nDirection;
				break;
			}
		}

		return ret;
	}

	// 判断点在不在车道区域以内
	bool IsPointInRegion(CvPoint pt) const
	{
		MvPointList pts = vListChannel;
		pts.push_back(vListChannel.front());
		MvPolygon poly(pts);
		return poly.IsPointInPoly(pt)==1;
		
	}

	void DrawChannel(IplImage *img)
	{
		MvPointList::iterator it1, it2;
		for (it1=vListChannel.begin(); it1!=vListChannel.end(); it1++)
		{
			it2=it1;
			it2++;
			if (it2==vListChannel.end())
			{
				it2=vListChannel.begin();
			}

			cvLine(img, cvPoint(it1->x, it1->y), 
				cvPoint(it2->x, it2->y), 
				CV_RGB(255, 255, 255));
		}
		
		it1=vListChannel.begin();
		char buffer[300];
		
		//sprintf(buffer, "nRoadIndex=%d,nVerRoadIndex=%d,nDirection=%d,bNoTurnLeft=%d,bNoTurnRight=%d,bNoForeward=%d,bNoReverse=%d,dirstart.x=%d", 
		//	nRoadIndex,nVerRoadIndex,nDirection,bNoTurnLeft,bNoTurnRight,bNoForeward,bNoReverse, vDirection.start.x);
		
		for (std::vector<NoPassingInfo>::iterator itt = vecNoPassingInfo.begin(); itt!=vecNoPassingInfo.end(); itt++)
		{
			//sprintf(buffer, "%s,nType=%d,nStart=%d,nEnd=%d", buffer, itt->nVehType, itt->nStart, itt->nEnd);
		}

		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.7f, 1.0f);
		DRAW_MV_LINE(img, &vDirection);
		cvPutText(img, "start", vDirection.start, &font, CV_RGB(255, 255, 255));
		cvPutText(img, "end", vDirection.end, &font, CV_RGB(255, 255, 255));

		
		cvPutText(img, buffer, cvPoint(0, it1->y), &font, CV_RGB(255, 255, 255));
	}


	
	// 返回车道有没有禁行限制
	// timestamp当前时间离本日0时0分0秒经过的秒数。
	bool HasNoPassingConstrain(int timestamp) const
	{
		std::vector<NoPassingInfo>::const_iterator nit;
		for (nit=vecNoPassingInfo.begin(); nit!=vecNoPassingInfo.end(); nit++)
		{
			if (timestamp > nit->nStart && timestamp < nit->nEnd)
			{
				return true;
			}
		}

		return false;
	}

	// 判断在一天中的某个时间，某个车道有没有禁行限制。
	// int timestamp 时间。
	static bool HaveNoPassingConstrain(const std::vector<_ChannelRegion> &vecChannelRgns, 
											int timestamp, int nRoadIndex)
	{
		std::vector<_ChannelRegion>::const_iterator cit;
		for (cit=vecChannelRgns.begin(); cit!=vecChannelRgns.end(); cit++)
		{
			if (cit->nRoadIndex == nRoadIndex)
			{
				if (cit->HasNoPassingConstrain(timestamp))
					return true;
			}
		}

		return false;
	}

	//取得最大、最小的车道编号！
	static bool GetMaxMinChannelIndex(const vector<_ChannelRegion> &vecChns, int &nMaxRoadIndex, int &nMinRoadIndex)
	{
		if (vecChns.size() <= 0)
		{
			return false;
		}

		int nMaxIndex = -1, nMinIndex = MAX_INT;
		vector<_ChannelRegion>::const_iterator citCR;
		for (citCR = vecChns.begin(); citCR != vecChns.end(); citCR++)
		{
			int nTmpRoadIndex = citCR->nRoadIndex;
			if (nMinIndex > nTmpRoadIndex)
			{
				nMinIndex = nTmpRoadIndex;
			}
			if (nMaxIndex < nTmpRoadIndex)
			{
				nMaxIndex = nTmpRoadIndex;
			}
		}
		
		nMaxRoadIndex = nMaxIndex;
		nMinRoadIndex = nMinIndex;

		return true;
	}

}ChannelRegion;

#endif