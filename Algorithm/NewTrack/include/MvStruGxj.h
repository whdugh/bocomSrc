#ifndef _MV_STRU_GXJ_H_
#define _MV_STRU_GXJ_H_

#include <time.h>
#include <vector>
#include <algorithm>
#include "libHeader.h"
#include "MvImgProUtility.h"

using namespace std;
using namespace MV_IMGPRO_UTILITY;

//检测区域结构体
typedef struct _MvStrDectectArea
{
	CvPoint pLtpt;
	CvPoint pRbpt;
}strDectectArea;

//寻找候选人头的结构体
typedef struct _MvFindHead
{
	vector <CvPoint> m_vctHeadPoint;				//头顶坐标
	vector<strDectectArea> m_vctDectectArea;		//检测区域
	vector<strDectectArea> m_vctMayExistDectectArea;//可能存在人头的检测区域

private:
	int m_nImgW, m_nImgH;	//实验图的宽/高

public:
	_MvFindHead()
	{
		if( !m_vctHeadPoint.empty() )	
		{
			m_vctHeadPoint.clear();
			m_vctHeadPoint.reserve(100);
		}
		if (!m_vctDectectArea.empty())
		{
			m_vctDectectArea.clear();
			m_vctDectectArea.reserve(100);
		}
		if (!m_vctMayExistDectectArea.empty())
		{
			m_vctMayExistDectectArea.clear();
			m_vctMayExistDectectArea.reserve(100);
		}		
		m_nImgW = 0;
		m_nImgH = 0;
	}
	//寻找头顶和检测区域方法1(较准确，但漏检较多)
	void mvFindHeadPoint(IplImage *pFore2VImg, 
			IplImage *pMaxSizeXImage, IplImage *pMaxSizeYImage);

	//寻找头顶和检测区域方法2（检出率高，但误检多）
	void mvFindHeadArea(IplImage *pErodeImg,IplImage *pFore2VImg, 
			IplImage *pMaxSizeXImage, IplImage *pMaxSizeYImage,
			IplImage *pIntFore2VImg);	

private:
	//计算两矩形的覆盖率
	double mvComputeLayover(strDectectArea DetectArea1, 
							strDectectArea DetectArea2);	
	//点是否在矩形区域内
	bool mvIsInRect(CvPoint pt, strDectectArea DetectRect);		

	//寻找四个点中中间两个点的之间的距离
	int  mvLenLap(int x1, int x2, int x3, int x4);	

	//寻找四个点中最小和最大的两个点
	void mvMinMaxValue(int x1, int x2, int x3, int x4, 
					   int &nMin, int &nMax);			
	
	//过滤一些不好的点
	void mvFiltBadPoint(IplImage *pFlagImage,
			IplImage *pDst,	IplImage *pIntFore2VImg,			
			IplImage *pMaxSizeXImage,IplImage *pMaxSizeYImage,
			vector<strDectectArea> &vctDectectArea);

	//调整检测区域
	void mvAdjustDetectArea(vector<strDectectArea> &vctDectectArea);			

}MvFindHead;


#endif