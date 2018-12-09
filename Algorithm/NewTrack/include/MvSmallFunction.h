#ifndef _MV_SMALL_FUNCTION_H_
#define _MV_SMALL_FUNCTION_H_

#include "libHeader.h"
#include <time.h>
#include <vector>

using namespace std;

#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef PI
	#define PI ( 3.14159265 )  /* pi  */
#endif
#ifndef IPP_PI180
	#define IPP_PI180 ( 0.01745329251994329577 )  /* pi/180  */
#endif

typedef struct 
{
	uchar r, g, b; 

} rgb;

//小函数
namespace MV_SMALL_FUNCTION
{
	//读取配置时判断字段的类型
	int mvCharToInt(char flag[20 ]);

	//获得随机数
	rgb mvRandomRgb();

	//读取随机数
	int mvRand();

	// 函数功能：把公里/小时--米/秒
	double mvKMH_to_MS(double value);

	// 函数功能：把米/秒-->公里/小时 
	double mvMS_to_KMH(double value);

	// 函数功能：从rect中获得左上点和右上点 
	void mvGetLtRbPt4Rect(CvPoint &ltPt, 
		   CvPoint &rbPt, const CvRect &rct);
	void mvGetFloatLtRbPt4Rect(CvPoint2D32f &fLtPt,
		   CvPoint2D32f &fRbPt, const CvRect &rct);

	// 函数功能：获取rect的中心点 
	CvPoint mvGetCenterPt4Rect(const CvRect &rct);

	// 函数功能：从左上点和右上点获得rect
	CvRect mvGetRect4LtRbPt(const CvPoint &ltPt, const CvPoint &rbPt);
	CvRect mvGetRect4FloatLtRbPt(const CvPoint2D32f &fLtPt, 
							     const CvPoint2D32f &fRbPt);

	//得到一个区域的中心(区域顶点的均值)
	bool mvGetRegionCenter(int nPoints, CvPoint2D32f *pPoints, 
			               CvPoint &roadCenter);

	//平面相机模型的坐标变换
	CvPoint2D32f mvtransform(CvPoint2D32f src, float homography[3][3]);

	//得到某点在世界坐标中的方向
	float mvgetOriWorld(int nDirection, CvPoint2D32f pCenter, 
					    float homography_inv[3][3]);

	//从给出的点得到轮廓点集（注意需要在外面释放分配的空间）
	CvMat* mvGetCoutourPts4Pts(int nPtCnt, CvPoint2D32f pts[]);

	//从给出的轮廓获取其各轮廓点
	vector<CvPoint> mvGetPts4GiveCoutour(CvSeq *contour);

	//获取给定点所对应的道路方向
	float mvGetRoadOriOfGivePt(const CvPoint &ptNow, IplImage *pRoadOriImg);

	//判断给定区域是否为给定区域的底部
	bool mvIsBottomOfAreaRect(
			CvRect rectObjArea, CvRect rectRoadRoi, 
			IplImage *pCarWImg, IplImage *pCarHImg);

	//对给定的区域按固定大小窗口滑动来分成多个重叠窗口
	bool mvGetStrideWindows4GiveArea(
			vector<CvRect> &vectStrideWindows,  
			const CvPoint &ptLt, const CvPoint &ptRb,   
			const CvSize &szWin, const CvSize &szStride );

	//获取积分由多到少的窗口序号
	void mvGetBestWindowOfIntegralImg(
			vector<int> &vectM2LWinIdx,   //积分由多到少的窗口序号
			vector<int> &vectInteGradVal, //排序后的积分值
			IplImage *pIntImg,            //积分图
			const vector<CvRect> &vectWindows ); //给定的各窗口


	//获取点的(2n+1)*(2n+1)的邻域窗口的点位置
	CvPoint* mvGetNeighborMvPss(int &nTotalCnt, const int &nRaiuds);
}


//----MvDemarcate根据标定来计算给定点的maxsize----//
class MvDemarcate
{

public:
	MvDemarcate( );
	~MvDemarcate( );

	int mvgetMaxSizeOfPoint( int nHomographyPtNum,
		float *homography_src, float *homography_dst, 
		CvPoint2D32f pt_img );

private:
	void mvCalculateHomography(
		int nHomographyPtNum, float *homography_src,
		float *homography_dst, float homography[3][3],
		float homography_inv[3][3] );

	CvPoint2D32f mvTransform( CvPoint2D32f src,
		float homography[3][3] );
};


#endif