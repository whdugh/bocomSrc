#ifndef __LTD_RECT_TRACK_H
#define __LTD_RECT_TRACK_H

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include "libHeader.h"

#include "comHeader.h"  //放最后

using namespace std;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_AN_ANLGORITHM;


enum ENUM_TLD_TRACK
{
	MV_TLD_COUNT=1, //!< the maximum number of iterations or elements to compute
	MV_TLD_MAX_ITER = MV_TLD_COUNT, //!< ditto
	MV_TLD_EPS = 2  //!< the desired accuracy or change in parameters
	                //       at which the iterative algorithm stops
};


//区域进行TLD跟踪的结果
#ifndef MAX_TLD_TRACK_LEN
	#define MAX_TLD_TRACK_LEN 100
#endif
typedef struct StruOneTLDTrackResult
{ 
	int  nTldRectId;  //该Tld区域的id
	int  nCurLenIdx;  //当前长度的指示

	CvRect rectA[MAX_TLD_TRACK_LEN];			//跟踪得到的区域
	CvPoint2D32f fptAOffset[MAX_TLD_TRACK_LEN]; //与上帧的点集偏移中值

	//附加的区域信息
	float  fAHoGPeoScore[MAX_TLD_TRACK_LEN];  //HoG行人的得分
	float  fAFkPtsRate[MAX_TLD_TRACK_LEN];    //区域前景点的比率
	float  fADiffPtsRate[MAX_TLD_TRACK_LEN];  //区域差分点的比率

public:
	StruOneTLDTrackResult( );

	//移动一个节点，但不填充
	void mvMoveOneNode( );

}OneTLDTrackResult;



//利用LTD进行rect区域跟踪的结构体
typedef struct StruTLDRectTrack
{
public:	
	StruTLDRectTrack( );

	//初始化变量
	void initVar( );
	
	//初始化rect区域匹配跟踪器	
	void mvInitTLDRectTrack( int nW, int nH );	
	
	//释放rect区域匹配跟踪器
	void mvUninitTLDRectTrack( );	

	//对给定rect区域进行匹配跟踪
	bool mvTLDTrack(
			IplImage *img1,  //图像1
			IplImage *img2,  //图像2
			IplImage *p2VFkImg4Give, //图1所对应的前景
			IplImage *p2VFkImg4Pred, //图2所对应的前景
			IplImage *pEigImg4Give,  //图1所对应的能量
			const CvRect &rectGive,  //图1中要跟踪的rect区域
			vector<CvPoint2D32f> &points1,  //图1欲匹配的点
			vector<CvPoint2D32f> &points2,  //图2匹配上的点
			CvRect &rectPredict,     //图像2的对图像1中rect的跟踪结果
			CvPoint2D32f &fptOffset  //点集匹配后的中值位移
		);  

	//处理当前的TLD跟踪结果
	static void mvProcessTLDRectResultEveryFrame( 
			vector<StruOneTLDTrackResult> &vectTLDReulst,
			vector<CvRect> &vectLastFindRects,
			vector<int> &vectLastFindRectIdxs,
			vector<CvPoint2D32f> &vFptOffset, //区域点匹配的偏移值中值
			int  nRecNoPeoCntDelTh = -1000,  //最近不为行人的删除阈值
			bool bDelLongTimeOverLap = true //是否删除长时间重叠的目标
		);

	//删除重叠的目标
	static void mvDeleteOverlap(
			vector<StruOneTLDTrackResult> &vectTLDReulst //IN+OUT
		);

	//获取要删除重叠的目标序号
	static void mvGetWantDeleteOverlapIdx( int &nDelIdx,  //OUT:
			int i, const OneTLDTrackResult &TLDR_i, //IN:
			int j, const OneTLDTrackResult &TLDR_j  //IN:
		);

public:
	bool m_bTValid;   //跟踪的结果是否有效
	bool m_bTracked;  //TLD方法是否跟踪上

private:
	//获取得到给定rect内的像素点
	void mvGetPoints4GiveRect(
			vector<CvPoint2D32f> &points, //OUT:点集合
			const CvSize &szImg,		  //IN:图像大小
			const CvRect &rectGive,		  //IN:给定的点提取区域
			IplImage *p2VFkImg4Give=NULL, //IN:前景2值图像
			IplImage *pEigImg4Give=NULL   //IN:梯度能量图像
		);


	//对两帧图像进行跟踪
	bool mvTrackFrame2Frame(
			IplImage *img1, IplImage *img2,
			vector<CvPoint2D32f> &points1, 
			vector<CvPoint2D32f> &points2, 
			IplImage *p2VFkImg2 = NULL  //前景2值图像
		);

	//计算两帧匹配点的相关性
	void mvCalcNormCrossCorrelation(
			IplImage *img1, IplImage *img2, 
			int nPtcnt,	char *chMatchStatus, 
			CvPoint2D32f *pts1, CvPoint2D32f *pts2,
			float *fSimilarity 
		);

	//对点匹配的结果进行过滤
	bool mvFilterMatchPts( int &nPariCnt,
			CvPoint2D32f *pts1, CvPoint2D32f *pts2,
			char *FBStatus, float *fFBError, 
			float *fSimilarity 
		);

	//过滤背景点
	void mvFilterBgPts(	
			vector<CvPoint2D32f> &pts1,
			vector<CvPoint2D32f> &pts2,
			IplImage *p2VFkImg2 = NULL  //前景2值图像
		);

	//预测rect
	void mvPredictRect(
			float &dx,	  //OUT: 点集的x位移中值
			float &dy,	  //OUT: 点集的y位移中值
			CvRect &bb2,  //OUT: 预测后的目标框
			const vector<CvPoint2D32f>& points1,
			const vector<CvPoint2D32f>& points2,
			const CvRect& bb1
		);

	//获取数组的中值
	float mvMedian(vector<float> v);

	//获取数组排序后的前百分比的值
	float mvGetSortValue(vector<float> v, float f);

	//返回给定点的范数
	float mvPointNorm(CvPoint2D32f v);

private:
	CvTermCriteria m_term_criteria;

	CvSize m_window_size;
	int   m_level;
	float m_lambda;

	IplImage *m_opticalFlowPyr;			
	IplImage *m_opticalFlowPyrLast;	

}TLDRectTracker;


#endif