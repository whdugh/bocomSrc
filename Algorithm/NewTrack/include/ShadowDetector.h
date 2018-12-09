//阴影检测
#ifndef __MV_SHADOW_DETECTOR_H
#define __MV_SHADOW_DETECTOR_H

#include "libHeader.h"
#include "comHeader.h"  
#include "ObjExistConfirm.h"

//HSV图像集合
typedef struct StruHSVImgSet  
{
public:
	IplImage *m_pHImg;
	IplImage *m_pSImg;
	IplImage *m_pVImg;

public:
	StruHSVImgSet( void );
	~StruHSVImgSet( void );

	void mvCreateImgs( const CvSize &sz );	
	void mvReleaseImgs( );
	void mvInitVar( );

}AnHSVImgSet;


//阴影检测结构
typedef struct StruImgSetToSmlFk4ShadowDet
{ 
public:
	StruImgSetToSmlFk4ShadowDet( IplImage *pSml2VFkImg,	
		IplImage *pModelRgbImg, IplImage *pModelRgbBgImg );

public:
	IplImage *m_pSml2VFkImg;	
	IplImage *m_pModelRgbImg;	
	IplImage *m_pModelRgbBgImg;	

}MvSDImgSetToSmlFk;


typedef struct StruImgSetToBigFk4ShadowDet
{ 
public:
	StruImgSetToBigFk4ShadowDet( IplImage *pBgGrayImg, 
		 IplImage *pBig2VFkImg, IplImage *pCalCarWImg,
		 IplImage *pCalCarHImg );

public:
	IplImage *m_pBgGrayImg;	
	IplImage *m_pBig2VFkImg;	
	IplImage *m_pCalCarWImg;	
	IplImage *m_pCalCarHImg;	

}MvSDImgSetToBigFk;


//阴影检测结构
typedef struct StruShadowDetector  
{
public:
	bool m_bInitShadowLightDet;  //是否初始化

private:
	CvSize  m_szRbgBgImg;	     //彩色背景模型图像的大小
	CvSize  m_szGrayImg;	     //灰度图像的大小

	IplImage *m_pSmoothRgbImg;	    //平滑后的当前Rgb图像
	IplImage *m_pRgbColorShadowImg;	  //采用彩色颜色所判断的阴影图像
	IplImage *m_pVehBotShadowRgbImg;  //采用彩色颜色所判断的车底阴影图像

	AnHSVImgSet m_hsvImgSet;     //当前彩色图的hsv图像
	AnHSVImgSet m_bgHsvImgSet;   //背景彩色图的hsv图像


	//对候选阴影轮廓轮廓确认器
	StruObjConfirmByCtus m_confirm4CShadowCtus; 
	IplImage  *m_pShadowFillImg;
	double    m_dTsShadowFill;

	//候选车底阴影图像
	IplImage  *m_pVehBotShadowImg;
	double    m_dTsVehBotShadow;

public:
	StruShadowDetector( void );
	~StruShadowDetector( void );

	//初始化和释放
	bool mvInitShadowLightDetector( 
			const CvSize &szRbgBgImg,
			const CvSize &szGrayImg );
	bool mvUninitShadowLightDetector( );

	void mvInitShadowLightDetectorVar( );

	//进行阴影检测(主接口)
	void mvDetectShadow(
			double dTsNow, IplImage *pGrayImg,   //当前的时间戳/灰度图像
			MvSDImgSetToBigFk &SDImgSetToBigFk,  //对应大前景的阴影检测图像集
			MvSDImgSetToSmlFk &SDImgSetToSmlFk,	 //对应小前景的阴影检测图像集
			IllumRgbVarSet illumRgbVarSet,       //rgb颜色光照度变量集
			bool bDetectObjectShadow = true,	 //是否需要检测目标阴影
			bool bDetectVehBotShadow = true		 //是否需要检测车底阴影
		);

	//获取"阴影部分所对应图"的指针及其对应的时间戳
	IplImage* mvGetShadow2VImgPointerAndStamp( 
			double  &dTsCalc   //其计算的时间戳（输出）
		);

	//获取"车底阴影部分所对应图"的指针及其对应的时间戳
	IplImage* mvGetVehBotShadow2VImgPointerAndStamp( 
			double  &dTsCalc   //其计算的时间戳（输出）
		);

private:
	//通过轮廓来检测阴影
	void mvDetectShadowByCountours( 
			IplImage *pShadowFillImg,
			CvScalar &colorFill,
			IplImage *pSmoothGrayImg,
			IplImage *pBgGrayImg,
			IplImage *pCandShadowImg,
			IplImage *pCarWImg, 
			IplImage *pCarHImg
		);

public:
	//根据光照和RGB情况，获取得到背景图像的各灰度级转换的map
	static void mvGetRgbBgIntensityCvtMap( 
			uchar unA_RBgMap[256],  //R
			uchar unA_GBgMap[256],  //G
			uchar unA_BBgMap[256],  //B
			IllumRgbVarSet *pIllumRgbVarSet //光照和RGB
		);

	//根据光照强度map来修改彩色背景图像
	static void mvModifyRgbBgImgWithIntensityMap( 
			IplImage *pModifyRgbBgImg,  //修改后的Rgb背景图
			IplImage *pRgbBgImg,		//原始的Rgb背景图
			uchar unARBgMap[256],		//R-map
			uchar unAGBgMap[256],		//G-map
			uchar unABBgMap[256]		//B-map
		);

	//利用颜色和光照信息来检测阴影。阴影像素和其覆盖的背景像素相比,
	//  具有如下特征: 相似的色度和饱和度，但亮度较低的特征来检测阴影。
	static void mvGetObjectShadowByColorIllum( 
			IplImage *pModifyRgbBgImg,  //rgb背景图
			IplImage *pRgbImg,			//rgb当前图
			IplImage *pFk2VImg,			//前景2值图
			IplImage *hBgImg,			//背景图的h通道 
			IplImage *sBgImg,			//背景图的s通道	
			IplImage *hImg,				//当前图的h通道
			IplImage *sImg,				//当前图的s通道
			IplImage *pShadowImg		//阴影结果图
		);

	//利用颜色和光照信息来检测车辆底部的阴影。
	static void mvGetVehBotShadowByColorIllum( 
			IplImage *pRgbBgImg,		//rgb背景图
			IplImage *pRgbImg,			//rgb当前图
			IplImage *pFk2VImg,			//前景2值图
			IplImage *hBgImg,			//背景图的h通道 
			IplImage *sBgImg,			//背景图的s通道	
			IplImage *hImg,				//当前图的h通道
			IplImage *sImg,				//当前图的s通道
			IplImage *pShadowImg		//阴影结果图
		);

	//获取得到车辆底部的阴影。底部阴影存在的特征：
	//    颜色比背景深得多，与上下的颜色相差得较多.
	static void mvGetVehBotShadowByGrayImg( 
			IplImage *pDrawImg, 
			const CvPoint &ptLt, 
			const CvPoint &ptRb,
			IplImage *pGrayImg, 
			IplImage *pBGraygImg
		);

	//利用颜色和光照信息来检测阴影。阴影像素和其覆盖的背景像素相比,具
	//    有如下特征: 相似的色度和饱和度，但亮度较低的特征来检测阴影。
	static void mvDetectShadowByColorIllum( 
			IplImage *pRgbBgImg,			 //rgb背景图
			IplImage *pRgbImg,				 //rgb当前图
			IplImage *pFk2VImg,				 //前景2值图
			IplImage *hBgImg,				 //背景图的h通道 
			IplImage *sBgImg,				 //背景图的s通道
			IplImage *vBgImg,				 //背景图的v通道
			IplImage *hImg,					 //当前图的h通道
			IplImage *sImg,					 //当前图的s通道
			IplImage *vImg,					 //当前图的v通道
			IllumRgbVarSet *pIllumRgbVarSet, //光照和RGB
			IplImage *pObjectShadowImg=NULL, //目标阴影结果图
			IplImage *pVehBotShadowImg=NULL  //车底阴影结果图
		);

	//判断点是否为阴影判断
	static bool mvIsShadowPoint4TNPt( 
			const PtTNLineInfo &ctuPtTNLineInfo, //轮廓点的线信息结构
			const IplImage *pShadowImg,	    //当前候选阴影图
			const IplImage *pBackGrayImg,	//当前灰度背景图
			const IplImage *pSmoothGrayImg, //当前灰度平滑图
			const float &fStart = 2.0f,     //计算的起始点位
			const float &fEnd = 10.0f,      //计算的终止点位
			const float &fStep = 1.0f,      //计算的点步长
			IplImage *pRgbShowImg = NULL    //显示图
		);

	//判断点是否为阴影,依据：
	static void mvShadowPtJudge4Ctus( 
		    vector<int> &vectShadowPartCtuIdx,
			MvCvPoint2DVector &vector2DOfShadowCtusPart,
			vector<StruPtTNLineInfoVector> &vctCtusPtTNLineInfo, //线信息结构的轮廓
			const vector<int> &vectCtuIdx,   //需考察的轮廓序号
			const IplImage *pShadowImg,	     //当前阴影图
			const IplImage *pBgGrayImg,	     //前景背景灰度图
			const IplImage *pSmoothGrayImg,  //当前灰度平滑图
			const IplImage *pCarHImg,	     //当前车辆高度图
			IplImage *pRgbShowImg            //彩色显示图
		);

private:
	//获取得到平滑图像和HSV图像
	void mvGetSmoothAndHsvImgs( 
			const IplImage *pRgbBgImg, //彩色背景图像
			const IplImage *pRgbImg    //当前的彩色图像
		);

}AnShadowDetector;

#endif