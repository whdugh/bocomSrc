#ifndef __MVBGTRACKMAP__H__
#define __MVBGTRACKMAP__H__

#include <ipp.h>
#include <vector>
#ifndef LINUX
	#include <algorithm>
#endif

#include "comHeader.h"  //放最后

using namespace MV_IMGPRO_UTILITY;

using namespace std;

#ifndef LINUX
//	#define DEBUG_DERELICT
#endif

class MvBgTrackMap  
{
private:
	int m_nWidth;
	int m_nHeight;

	bool m_bIsDay;

	IplImage * m_pMaxxImg, *m_pMaxyImg;
	int m_nStandW,m_nStandH;

	IplImage *m_pTrackOriMap;				//当前帧轨迹角度分布
	IplImage *m_pHisTrackOriMap;			//近期轨迹角度分布
	IplImage *m_pHisTrackOriMap_LongTerm;	//长期轨迹角度分布


public:
	MvBgTrackMap(const int nWidth,const int nHeight,const IplImage *pMaxXImg, const IplImage *pMaxYImg);
	virtual ~MvBgTrackMap();
	
public:
	int m_nUpRectCount,m_nDownRectCount;
	CvRect *m_pUpRects;
	CvRect *m_pDownRects;

	bool *m_pbUpExistLV;	//上行大车存在标志
	bool *m_pbDownExistLV;	//下行大车存在标志

	IplImage *m_pCornersMap;	
	IplImage *m_pShortCornersMap; //短期角点背景图

public:		//大车区域输出
	int m_nUp_LongV_count;		//上行大车数量
	int m_nDown_longV_count;	//下行大车数量
	CvRect *m_pUp_LongV_Rects;		//上行大车区域
	CvRect *m_pDown_LongV_Rects;	//下行大车区域

private:
	void FormTrackingRegion(IplImage *pBgImg=NULL,IplImage *pFgImg=NULL);
	bool MergeRects(const CvRect rtSrc1,const CvRect rtSrc2,CvRect &rtDst);
	static int rectCompare(const CvRect rt1,const CvRect rt2);
	int MakeupTrackingRegions(int nRectCount,CvRect *rects,CvRect *outRects);
	float MergeOri(const float fJd1,const float fJd2,float fAlpha);

	void ComputeInfluenceRegion(const CvPoint2D32f pt,CvRect &outRect);

public:
	void UpdateMap(const int nTrackCount,const CvSet * pTrackSet,double dAlpha,bool bIsDay=true,IplImage *pBgImg=NULL,IplImage *pFgImg=NULL);
	void AjustTrackingRegion();
	void AjustTrackingRegion(const int nHineCount,const MegerLine *pMergeHLine,IplImage *pBgImg=NULL,IplImage *pFgImg=NULL);
	void AjustTrackingRegionByVLine(const int nVlineCount,const MegerLine *pMergeVLine);

	float mvTrackChangeRatio(const CvRect rtROI,int nThresh);

	void mvUpdateCornerMap(const int nConerCount,const CvPoint2D32f *pCornersPt);

public:	//功能区


/*	void mvGetUpDownInfo(int &nUpCount,CvRect *pUpRects,int &nDownCount,CvRect *pDownRects);*/
};

#endif 
