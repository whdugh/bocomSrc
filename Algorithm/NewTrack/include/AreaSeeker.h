
//查找区域
#ifndef _AN_AREA_SEEKER_H_
#define _AN_AREA_SEEKER_H_

#include "libHeader.h"

#include "comHeader.h"  //放最后
#include "MvCalCoord.h"
#include "MvKeyPtExtract.h"

typedef struct MvSymArea
{
	CvRect rectLtArea;
	CvRect rectRbArea;
}symArea;

//进行搜索所用的宽高图像
typedef struct StruWHImgs4Seeker
{
public:
	StruWHImgs4Seeker( );
	~StruWHImgs4Seeker( );
	void mvInitVar( );

	//获取车辆宽高图像
	void mvGetCarWHImg( IplImage *pCarWImg, IplImage *pCarHImg );
	
	//获取行人宽高图像
	void mvGetPeoWHImg( IplImage *pPeoWImg, IplImage *pPeoHImg );

public:
	bool m_bInitCarI;
	bool m_bInitPeoI;

	//车辆宽/高
	IplImage *m_pCarWImg;
	IplImage *m_pCarHImg;

	//行人宽/高
	IplImage *m_pPeoWImg;
	IplImage *m_pPeoHImg;

}AnWHImgs4Seeker;


//搜索得到最好的小车区域
typedef struct StruBestCarAreaSeeker
{
public:
	StruBestCarAreaSeeker( ){  }

	~StruBestCarAreaSeeker( ){	}
	
	void mvInit( IplImage *pCarWImg, IplImage *pCarHImg );

	//获取最好的小车区域
	CvRect mvSeekBestCarArea(
			IplImage *pCarWImg, //对应于使用图的车宽图像
			IplImage *pCarHImg, //对应于使用图的车高图像
			IplImage *pFk2VImg, //对应的前景2值图像
			const CvRect &rectUseGive, //给定的在使用图像中区域
			Pt2D32fCoordConvert pt2dSrc2UseCoordCvt, //原始/使用图坐标转换器
			IplImage *pSrcRgbImg //原始彩色图像
		);

private:
	//获取最好的小车区域
	bool mvGeekBestCarArea4GiveImg(
			CvRect &rectBestCar,     //寻找到的最好的小车区域
			IplImage *pGrayImg,		 //ROI区域所对应的灰度图像
			IplImage *pIntGradImg,   //ROI区域所形成的梯度积分图像
			const CvSize &szStdCar   //给定图所对应的车的标准宽高
		);

	//调整最好车辆区域
	void mvAdjustRect(
			IplImage *pIntGradImg,	//ROI区域所形成的梯度积分图像
			CvPoint ptLt,			//ROI区域的左上角点
			CvPoint ptRb,			//ROI区域的右下角点
			CvRect &rectBestCar		//ROI区域中调整后的最好车辆区域
		);

	//得到对称区域
	void mvGetSymArea(
			CvRect rectBest,			//区域
			int nMidX,					//区域的中间分界线
			float fSymAreaH,			//对称区域的高度
			float fStep,				//步长
			vector<symArea> &vctsymArea	//得到的对称区域
		);

private:
	//进行搜索所用的宽高图像
	AnWHImgs4Seeker m_WHImgs4Seeker;

}AnBestCarAreaSeeker;



//搜索得到最好的小车底部阴影区域
typedef struct StruBestVehBotShadowAreaSeeker
{
public:
	StruBestVehBotShadowAreaSeeker( ){ 	}

	~StruBestVehBotShadowAreaSeeker( ){	}

	void mvInit( IplImage *pCarWImg, IplImage *pCarHImg );

	//获取最好的车辆底部阴影区域
	bool mvSeekVehBotShadowArea(
			CvRect &rectBestArea, //搜索到的最好区域
			IplImage *pCarWImg,  //对应于使用图的车宽图像
			IplImage *pCarHImg,  //对应于使用图的车高图像
			IplImage *pIntegImg, //对应的积分计算图像
			const CvRect &rectUseGiveSeek  //给定的使用图中搜索区
		);

private:
	//进行搜索所用的宽高图像
	AnWHImgs4Seeker m_WHImgs4Seeker;

}MvBestVehBotShadowAreaSeeker;




//搜索得到最好的小车底部水平边缘区域
typedef struct StruBestVehBotHoriEdgeAreaSeeker
{
public:
	StruBestVehBotHoriEdgeAreaSeeker( ){   }

	~StruBestVehBotHoriEdgeAreaSeeker( ){	}

	void mvInit( IplImage *pCarWImg, IplImage *pCarHImg );

	//获取最好的车辆底部水平边缘区域
	bool mvSeekVehBotHoriEdgeAreaArea(
			CvRect &rectBestArea, //搜索到的最好区域
			IplImage *pCarWImg,  //对应于使用图的车宽图像
			IplImage *pCarHImg,  //对应于使用图的车高图像
			IplImage *pIntegImg, //对应的积分计算图像
			const CvRect &rectUseGiveSeek  //给定的使用图中搜索区
		);

private:
	//进行搜索所用的宽高图像
	AnWHImgs4Seeker m_WHImgs4Seeker;

}MvBestVehBotHoriEdgeAreaSeeker;


//对称中心
typedef struct StruSymmetryCet
{ 
	CvPoint ptCet;   //中心点
	float   fArc;    //轴的弧度

	float   fAvgDiff;  //平均的差异

	CvPoint ptMin;
	CvPoint ptMax;
}MvSymmetryCet;


typedef struct StruCoordDist
{ 
	CvPoint ptObjCet;

	int  nYCnt;
	int  nStrideHei;
	int  nACetY[101];

	int  nXCnt;
	int  nStrideWid;
	int  nACetX[101];
	
}MvCoordDist;

typedef struct StruSymmetryX
{ 
	int   nYIdx;
	int   nXIdx;

	int   nYCoord;
	int   nBestSymmertyX;

	float fUnSymmertyVal;

}MvSymmetryX;


typedef struct StruXSymmetryResult
{ 
	MvCoordDist m_CoordDist;
	
	vector<MvSymmetryX> m_vectSymmetryX;

}MvXSymmetryResult;


//变化点
typedef struct StruChangeXCoord
{ 
	CvPoint ptObjCet;

	int   nYCoord;

	int   nBestChangeX1;
	int   nBestChangeX2;

	float fChangeVal;

}MvChangeXCoord;


//变化点
typedef struct StruEdgePtsOfRow
{ 
	int   nYCoord;
	vector<int>  vectXCoord;

}MvEdgePtsOfRow;

//搜索得到最好的对称轴
typedef struct StruBestSymmetryAxisSeeker
{
public:
	StruBestSymmetryAxisSeeker( ){ }

	~StruBestSymmetryAxisSeeker( ){	}

	void mvInit( IplImage *pCarWImg, IplImage *pCarHImg );

	//获取y方向最好的对称轴
	bool mvSeekBestSymmetryAxisOfY(
			MvSymmetryCet &SymmetryCet,	//搜索到的最好的对称中心
			IplImage *pCarWImg,     //对应于使用图的车宽图像
			IplImage *pCarHImg,     //对应于使用图的车高图像
			IplImage *pIntegImg,    //对应的积分计算图像
			IplImage *pIntFkImg,    //对应的前景积分图像
			const float fInitArc,   //给定的最初对称轴的弧度
			const CvRect &rectUseGiveSeek  //给定的使用图中搜索区
		);


	//获取最好对称轴所对应的x坐标值
	bool mvSeekSymmetryOfXCoord(
			MvXSymmetryResult &XSymmetryRes,	//搜索到对称点
			IplImage *pCarWImg,			//对应于使用图的车宽图像
			IplImage *pCarHImg,			//对应于使用图的车高图像
			IplImage *pInteg256Img,		//对应的256值的积分图像
			IplImage *pIntFk2VImg,      //对应的2值前景的积分图像
			const CvRect &rectUseGiveSeek //给定的使用图中搜索区
		);


	//获取最好对称轴所对应的x坐标值
	bool mvSeekBestSymmetryOfXCoord(
			vector<CvPoint> &vectSymmPts, //搜索到对称点
			IplImage *pCarWImg,			  //对应于使用图的车宽图像
			IplImage *pCarHImg,			  //对应于使用图的车高图像
			IplImage *pIntHSob256Img,	  //对应的水平sobel256值的积分图
			IplImage *pIntVSob256Img,	  //对应的竖直sobel256值的积分图
			IplImage *pIntGray256Img,     //对应的灰度256值的积分图
			IplImage *pIntFk2VImg,        //对应的2值前景的积分图像
			const CvRect &rectUseGiveSeek //给定的使用图中搜索区
		);
	
	//搜索得到小车左右边界
	bool mvSeekLefRigBorderOfSmlVeh(
			vector<MvEdgePtsOfRow> &vectEdgePtsOfRow, //搜索到边界点
			IplImage *pCarWImg,			//对应于使用图的车宽图像
			IplImage *pCarHImg,			//对应于使用图的车高图像
			IplImage *pIntFore2VImg,	//前景2值化的256值的积分图
			IplImage *pIntSobH256Img,	//水平sobel的256值的积分图
			IplImage *pIntSobV256Img,	//竖直sobel的256值的积分图
			const CvRect &rectUseGiveSeek, //给定的使用图中搜索区
			IplImage *pDrawImg = NULL
		);

private:
	//进行搜索所用的宽高图像
	AnWHImgs4Seeker m_WHImgs4Seeker;

}MvBestSymmetryAxisSeeker;



//搜索得到最好的对称轴
typedef struct StruLeftRightBorderSeeker
{
public:
	//搜索得到小车左右边界
	bool mvSeekLefRigBorderOfSmlVeh(
			vector<MvEdgePtsOfRow> &vectEdgePtsOfRow, //搜索到边界点
			IplImage *pCarWImg,			//对应于使用图的车宽图像
			IplImage *pCarHImg,			//对应于使用图的车高图像
			IplImage *pIntFore2VImg,	//前景2值化的256值的积分图
			IplImage *pIntSobH256Img,	//水平sobel的256值的积分图
			IplImage *pIntSobV256Img,	//竖直sobel的256值的积分图
			const CvRect &rectUseGiveSeek, //给定的使用图中搜索区
			IplImage *pDrawImg = NULL
		);

private:
	//搜索得到小车左右边界
	void mvInitSeeker( 
		   IplImage *pCarWImg,	 //对应于使用图的车宽图像
		   IplImage *pCarHImg,	 //对应于使用图的车高图像
		   const CvSize &sz,
		   const CvRect &rectUseGiveSeek
		 );

private:
	CvPoint ptLt, ptRb; 

	int  nInitCetX, nInitCetY;	
	int  nStdCarWid, nStdCarHei;	
	int  nHalfWinW, nHalfWinH;

	int  nStrideHei, nYBin, nYCnt;
	int  nACetY[100];

	int  nXL, nXR;	
	int  nHalfStdCarWid, nVertWinHalfWid;  
	int  nBigVertWinHalfWid, nCalcCnt;
	int  nAMidX[100], nALefX[100], nARigX[100];
	int  nABigLefX[100], nABigRigX[100];

}MvLeftRightBorderSeeker;


//////////////////////////////

#endif
