/*
 *本类功能：实现对直线的合并！
 */
#ifndef	MV_LINE_MERGE
#define MV_LINE_MERGE

#include "MvLineSegment.h"
#include "MvLine.h"
#include "Calibration.h"
#include "MvMaxSize.h"


#ifndef	PI
#define PI 3.1415926
#define HALF_PI PI/2
#endif

#define MERGE_LINE_NUM 500

typedef struct _MergeLine{         //老贺的直线数据结构！
	double x1;
	double y1;
	double x2;
	double y2;
	double ori;  //0-180
	double length;
	//double dRatio;  //相对maxsizeXY的大小
	bool   bMerge;	//合并
	bool   bBeMerge;	//被合并
	int    type;		//0-水平线，1-竖直线
	//bool   bUsed;	//已经使用
	//bool   bEdge;	//是否是边
}MergeLine;

class MvLineMerge
{
public:
	MvLineMerge(void);
	~MvLineMerge(void);

public:
	void MvLine2MergeLine(const MvLine **pSrcMvLines, int nLineNum);
	void MergeLine2MvLine();
	void Init(FindTargetElePolice::MyCalibration *pCalib, CvSize imgSize);
	void Destroy();
	void TestMvLineMerge(IplImage *img, unsigned int uFrame);
	void MyLinesMerge(BLine *lpBline, int nNonBgLineNum, MvLine **pRetMergedLine, int &nRetNum);
	
private:
	void MvSegmentUnite(BLine *lpBline, int nNoBgLineNum);
	//void QuickSort(double *data,int *index, int l, int r, bool bSmallToBig);
	void QuickSort(double *data,int *index, int l, int r);

private:
	//MergeLine *m_pSrcMergeLines;
	//int m_nSrcMergeLineNum;
	//MvLine *m_pMvLine[3];  
	MergeLine *m_pMergeLines[3];  //指向水平、竖直和倾斜三种结构的直线！
	MvLine *m_pAllMvLine;
	BLine *m_pSrcBLine;
	int m_nSrcBLineNum;
	int m_nMvLineNum[3];
	int m_nAllMvLineNum;
	FindTargetElePolice::MyCalibration *m_pCalib;

	IplImage *max_sizex;
	MvMaxSize *m_pMvMaxSize;
};

#endif