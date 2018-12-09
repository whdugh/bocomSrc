//对前景的应用
#ifndef _AN_FORE_IMAGE_APP_H_
#define _AN_FORE_IMAGE_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "comHeader.h"  //放最后

using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_SMALL_FUNCTION;

enum ENUM_FIND_FKCONTOUR_PEO_MODE
{ 
	FIND_FKCONTOUR_PEOHEAD = 0,
	FIND_FKCONTOUR_LIKEPEO,
	FIND_FKCONTOUR_PEOHEAD_LIKEPEO
};

typedef struct MvStruFkImgApp
{
public:
	//获得得到前景对应的countour
	static void mvFindContours(
			IplImage *pTempFkImg,  //临时的前景图,会被改写	
			int nContourAreaTh,    //轮廓面积阈值要求
			CvSeq *pContour        //前景轮廓序列
		);


	//给定区域的前景是否小于车辆宽度
	static bool mvIsFkImgSmallThanCarWidth( 
			IplImage *pFkIntImg,		//整个图像的2值积分图像
			CvPoint ltPt, CvPoint rbPt, //给定区域的左上和右下点
		    CvSize  szCarWH,            //给定区域标定的标准车辆宽高
			CvSize  szPeoWH,		    //给定区域标定的标准行人宽高
			IplImage *pDrawFkImg=NULL   //绘制判断过程和结果
		);   

	//获取腐蚀后的前景图像的轮廓结果
	static bool mvGetErodeForeImgContoursResult( 
			const IplImage *pSrcFkImg, //原始前景
			IplImage **ppFkImg,        //输出:腐蚀后的前景
			vector<fkContourInfo> &vcetForeContours, //输出:获取的前景轮廓
			bool bShowResult //输出:显示轮廓获取的结果
		);

	//从给的的轮廓点来获取行人的头部
	static bool mvGetPeoHeadFromContoursPts(
			int total,		    //轮廓点的个数
			CvPoint point[],    //轮廓点的坐标
			IplImage *pFore2VImg,	 //2值前景图
			IplImage *pInteFk2VImg,  //2值前景积分图
			IplImage *pPeoWImg, //行人宽的图像
			IplImage *pPeoHImg, //行人高的图像
			vector<CvPoint> &vectHeadPt, //检测出的人头点
			float fCloseXRateTh, //与原有人头点x过近的阈值(相对行人宽)
			float fCloseYRateTh  //与原有人头点y过近的阈值(相对行人高)
		);


	//利用前景及轮廓来获取得到行人区域
	static bool mvFindPeoRectWithFkContours( 
			vector<CvRect> &vectCandiatePeoRects,   //找到的行人区域
			IplImage *pFkImg, IplImage *pInteFkImg, //前景及前景积分图
			vector<fkContourInfo> &vcetForeContours, //前景轮廓
			IplImage *pPeoWImg, IplImage *pPeoHImg, //行人宽度和高度图
			CvSize dstUstSz, //最终所使用的图像大小(将行人区域至于该图中)
			int nFindMode=FIND_FKCONTOUR_PEOHEAD_LIKEPEO //寻找模式
		);

	//对给定区域分为n份来对的前景分布概率进行计算
	//(并对各划分的区域求取中心点，左上点和右下点)。
	static bool mvCalFkImgPercentOfArea( 
			IplImage *pInte2VFkImg,		//2值化前景积分图
			CvPoint ltPt, CvPoint rbPt, //给定区域的左上/右下点
			int   nBin,					//将给定区域划分为多少份
			float *fAPercent,			//各份的前景分布概率
			CvPoint *ptABinAreaCet,     //各份的中心点				
			CvPoint *ptABinAreaLt,		//各份的左上点
			CvPoint *ptABinAreaRb,      //各份的右下点
			bool  bVMod,				//是否采用竖直划分来求取
			bool  bSumPercentMod		//求取的概率是否相对于整个区域
		);

	//获取像行人的前景contours的序号
	static bool mvGetLikePeopleFkContours(
			vector<int> &vctLikePeoContoursIdx, //像行人的contours序号
			vector<fkContourInfo> &vctForeContours, //前景轮廓
			IplImage *pFore2VImg,	 //2值前景
			IplImage *pInteFk2VImg,  //2值前景积分图
			IplImage *pPeoWImg, //行人宽的图像
			IplImage *pPeoHImg, //行人高的图像
			bool bFiltrateBorder  //过滤掉边界的
		);


	//将给定的区域限制在前景区域内
	static bool mvRestrictAreaInFkArea(		
			CvPoint ptGiveLt, CvPoint ptGiveRb,  //给定的区域左上和右下角
			CvSize szGive,				   //给定的区域所对应的图像的大小
			IplImage *pFkImg,IplImage *pFkIntImg, //给定的2值前景及积分图像
			CvPoint &ptResLt, CvPoint &ptResRb,   //限制后的区域左上和右下角
			float fMinHoriPrec=0.2f, 
			float fMinVertPrec=0.2f //要求的水平和竖直前景百分比
		);


	//获取较为孤立的前景(可能对应速度快的目标)contours的序号
	static bool mvGetIsolatedContours(
			vector<CvRect> &vectIsolatedoRects,  //孤立前景rect
			vector<fkContourInfo> &vctForeContours,  //前景轮廓			
			IplImage *pFore2VImg,	 //2值前景
			IplImage *pInteFk2VImg,  //2值前景积分图
			IplImage *pPeoWImg,		 //行人宽的图像
			IplImage *pPeoHImg,      //行人高的图像
			CvSize dstUstSz, //最终所使用的图像大小(将行人区域至于该图中)
			bool bFiltrateBorder=true //过滤掉边界的
		);


	//在前景块较少的图像中获取像人或车的前景rect
	static bool mvGetLikePeoCarRects4FkContours(
			vector<CvRect> &vectPeoCartRects,  
			vector<fkContourInfo> &vctForeContours, 		
			IplImage *pFore2VImg,	 //2值前景
			IplImage *pInteFk2VImg,  //2值前景积分图
			IplImage *pPeoWImg,   //行人宽的图像
			IplImage *pPeoHImg,   //行人高的图像
			IplImage *pCarWImg,   //车辆宽的图像
			IplImage *pCarHImg,   //车辆高的图像
			CvSize dstUstSz,      //最终所使用的图像大小(将行人区域至于该图中)
			bool bFiltrateBorder  //过滤掉边界的
		);


	//对前景图像中的大车扩展区域按行隔列来进行扫瞄，判断是否为大车
	static bool mvScanFkImg2JudgeIsBigVehHead( 
			const int nH, //在目标处的车高
			const IplImage *pFkIntImg, const IplImage *pMaskImg,
			const CvRect rectBigVehArea, const int nStopAreaCnt,
			const CvPoint ptAStopLt[], const CvPoint ptAStopRb[],		
			const float fSz2Fk_y, const float fSz2Fk_x, 
			const IplImage *pCarWImg,const IplImage *pCarHImg,
			IplImage *pFkDrawImg 
		);

	//判断是否为较大车的车顶部分落入到停止区域。假设：在停车区域内的
	//超过0.8车宽的前景高度Ht,在停车区域下的超过0.8车宽的前景高度Hb。
	//则认为同时满足以下几个条件的为中型车：
	//  (1). Ht+Hb>车高;  (2). 2*Ht<Hb  (3). Ht<1.2车高
	static bool mvScanFkImg2JudgeIsMidVehHead( 
			const bool bDay, int nFkImgH,  //是否为白天/前景图像的高
			const int nCarH, const int nCarW, //在目标处的车高和车宽
			const IplImage *pFkIntImg,  
			const CvRect  rectVehArea, const int nStopAreaCnt,
			const CvPoint ptAStopLt[], const CvPoint ptAStopRb[],		
			const float fSz2Fk_y, const float fSz2Fk_x, 
			const IplImage *pCarWImg, const IplImage *pCarHImg,
			IplImage *pDrawImg 
		); 


	//对前景图像中的大车扩展区域按行隔列来进行扫瞄，判断是否为大车
	static bool mvScanFkImg2JudgeIsBigMidVehicle(
			bool bBigV,
			const int nPtCarH,        //在目标处的车高
			const int nPtCarW,        //在目标处的车宽
			const float fSz2Fk_y, 
			const float fSz2Fk_x, 
			const IplImage *pFkIntImg, 
			const IplImage *pMaskImg,
			const CvRect rectFkBVJ,   //在前景区域的大车判断区域
			CvRect &rectGetBV,		  //检测得到的大车区域
			IplImage **pFkBVJudgeDrawImg
		);


	//获取前景图像满足为大车宽的点
	static IplImage* mvGetSuitBigVehWidthPtsImg( 
			const CvRect rectBigVehArea, //目标roi区域 (前景图中)
			int  nFkBVWidthTh,           //在前景图像中的认为的大车车宽阈值
			bool bLeft2Right,            //是否从左到右扫瞄
			const IplImage *pMaskImg,    //目标roi区域的前景轮廓图像	
			const IplImage *pFkIntImg,   //对目标roi区域前景进行积分的图像
			CvPoint &ptStepXY            //生成结果图像中的每点表示的步长
		);

	//对大车和中型车判断中，获取满足条件的最好的x坐标和及对应的行数
	static bool mvGetBestXAndSuitYCnt4BMVJudge( 
			const IplImage *pL2RSuitImg, //从左到右满足宽度要求的扫瞄点图像
			const IplImage *pR2LSuitImg, //从右到左满足宽度要求的扫瞄点图像
			const IplImage *pL2RSuitIntImg, //从左到右满足宽度要求的扫瞄点图像积分图
			const IplImage *pR2LSuitIntImg, //从右到左满足宽度要求的扫瞄点图像积分图
			int nDiffW,	          //从左到右 与 从右到左 的列的序号相差要求
			int &nBestXL2R,       //最好的从左到右的扫瞄点的X值
			int &nBestXR2L,       //最好的从右到左的扫瞄点的X值
			int &nMaxSuitRowCnt,    //满足为大车的行数
			int *nABestSuitRowFlag  //各行满足的标识
		);

}struFkImgApp;

#endif