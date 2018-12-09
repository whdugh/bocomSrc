#ifndef _MV_TYPE_DETECT_H_
#define _MV_TYPE_DETECT_H_

#include "libHeader.h"
#include "comHeader.h"

#include <algorithm> //for std::sort

#include "MvImgProUtility.h"
#include "SmallVaildStruct.h"

#include "hog.h"
#include "MvHoG.h"

#include "ForeImgApp.h"
#include "LinesExtApp.h"
#include "ObjExistConfirm.h"
#include "OpenConfigParameter.h"

using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_SMALL_FUNCTION;


//------------定义使用HoG来检测人的结构体------------//

typedef struct AnStruSimHoGPeoDetApp/*:MvBaseSimHoGPeoDetApp*/
{
public:
	//在给定实验图和原始图上进行HoG检测
	bool mvDetWithSimHoGUseDstSrcImg( 
			vector<float> &vctPeoScore,	      //检测到的最好行人得分
			vector<CvRect> &vctRectPeoDst,    //检测到的最好行人rect(实验图)
			CvPoint ptLtDst,			      //实验图上待检测区域左上点
			CvPoint ptRbDst,			      //实验图上待检测区域右下点
			CvPoint ptDstPeoWH,			      //实验图上检测区的行人大小	
			enumUseHoGSvmMode nUseHoGMode,    //所使用的HoG模型		
			IplImage *pDstRgbImg,		      //实验图像
			IplImage *pSrcRgbImg,			  //原始图像
			ImgRoiResizeStru *pStruS2DRoiRes, //原始和实验图像坐标转换器
			double dTsNow				      //当前的时间戳(单位s,用于存图)
		);

public:
	//初始化
	void mvInitSimHoGApplication( ) { 
		m_BaseSimHoGPeoDetApp.mvInitSimHoGApplication( );
	}
	
	//释放
	void mvUnInitSimHoGApplication( ) {
		m_BaseSimHoGPeoDetApp.mvUnInitSimHoGApplication( );
	}

	//在给定图上进行HoG检测
	bool mvDetWithSimHoG( 
			vector<float> &vctBestPeoScore,	 //检测到的最好行人得分
			vector<CvRect> &vctrectBestPeo,  //检测到的最好行人rect
			IplImage *pSrcRgbImg,			 //给定的彩色图像
			CvPoint ptLtSrc,				 //给定图上待检测区域左上点
			CvPoint ptRbSrc,				 //给定图上待检测区域右下点
			CvPoint ptPeoWhSrc,				 //给定图检测区上行人的标准宽高
			enumUseHoGSvmMode nUseHoGMode,   //所使用的HoG模型
			double dTsNow,					 //当前的时间戳(单位s,用于存图)
			int    nResCnt = 3,          //对行人模板缩放的次数--3
			float  fMaxResScale = 1.2f,  //最大缩放比率--1.2
			float  fResizeRate  = 1.2f,	 //每次的缩放的比率--1.2
			float  fPeoScoreTh  = 0.0f,  //行人判断的阈值--0.0
			int    nPeoCntTh = 1         //行人判断的个数阈值--1
		)
	{
		bool bDet = m_BaseSimHoGPeoDetApp.mvDetWithSimHoG(
					    vctBestPeoScore, vctrectBestPeo, pSrcRgbImg,
						ptLtSrc, ptRbSrc, ptPeoWhSrc, nUseHoGMode, 
						dTsNow, nResCnt, fMaxResScale, fResizeRate,
						fPeoScoreTh, nPeoCntTh);

		return bDet;
	}

private:
	MvBaseSimHoGPeoDetApp  m_BaseSimHoGPeoDetApp;

}StruSimHoGPeoDetApp;


//------------在停止检测中判断是否为车辆的结构体------------//
typedef struct StruIsVehicleOfStopDet
{
public:
	//使用前景的宽度来判断是否为车辆
	static bool MvIsVehicleWithFkWidth( 
			const CvPoint &ltPt, const CvPoint &rbPt, 
			const CvSize &szCar, const CvSize &szPeo,
			IplImage *pUseFk2VImg, IplImage *pUseFk2VInteImg
		);

	//判断指定区域是否存在目标
	static bool MvIsExistObject( 
			const CvRect &rectGiveArea, 
			IplImage *pUseGrayImg,
			IplImage *pSmoothBgGradImg,
			IplImage *pShowRgbImg 
		);

	//获取检测参数
	static void MvGetConfigParameter( 
		    bool &bStrictObjType, bool &bConfirmObjExist,
		    StructDetStaParaConfiger *pParaConfiger 
		);

	//获取指定点的标准目标大小
	static CvSize MvGetStdObjSz( 
			CvPoint ptCet, IplImage *pWImg,	IplImage *pHImg 
		);

	//获取在指定点为中心的提取区域
	static void mvGetExtractArea( 
			CvRect &srcLineRect,	  //OUT:在原始图像中的线段提取区域
			IplImage *pSrcImg,		  //IN:原始图像
			CvPoint2D32f fSrcCetPt,	  //IN:给定的区域中心点（原始图像） 
			CvPoint2D32f fSrcCarSzPt, //IN:中心点对应的车辆大小（原始图像）
			float fExpandW = 1.5f,    //IN:在宽方向扩展的比率（相对车辆大小）
			float fExpandH = 1.5f     //IN:在高方向扩展的比率（相对车辆大小）
		);

	//获取在指定点区域的线段提取结果
	static double mvExtactLinesOnClick( 		
			vector<CvPoint2D32f> &vctLinePt1, //OUT:线段起点（原始图像）
			vector<CvPoint2D32f> &vctLinePt2, //OUT:线段终点（原始图像）
			IplImage *pSrcImg,			   //IN:原始图像
			const CvRect &srcLineRect      //IN:在原始图像中的线段提取区域
		);

}MvIsVehicleOfStopDet;


#endif
