//确认是否存在目标
#ifndef __OBJ_EXIST_CONFIRM_H
#define __OBJ_EXIST_CONFIRM_H

#include "libHeader.h"
#include "comHeader.h"  

using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_SMALL_FUNCTION;

enum ENUM_COUNTOUR_MODE
{ 
	DEFAULT_CTU = 0,
	HEI_WID_ALL_SMALL_CTU = 1,
	HEI_SMALL_WID_BIG_CTU = 2,
	HEI_BIG_WID_SMALL_CTU = 4,
	NARROW_PART_OF_CTU = 8,
	SHADOW_PART_OF_CTU = 32
};

//点的线信息结构
typedef struct StruPtTNLineInfo
{
	CvPoint pt;        //点坐标
	int     nPtIdx;    //点的序号

	float   fTangentK;  //切线斜率
	float   fNormaleK;  //法线斜率

	float   fInside;	   //轮廓内侧方向
	float   fNormalConLen; //轮廓法线连通长度

	CvPoint ptEnd4Connect; //连通终止点

	StruPtTNLineInfo( )
	{
		nPtIdx = -1;
	}
}PtTNLineInfo;

struct StruPtTNLineInfoVector
{
	int nContoutsIdx;  //所对应的轮廓序号
	vector<PtTNLineInfo> m_vectPtTNLineInfo; //点的线信息结构
};

//轮廓应用结构
typedef struct StruContoursApplication
{
public:
	StruContoursApplication( );
	~StruContoursApplication( );
	
	//获取轮廓上的点并计算其属性值
	void mvGetCalcContoursPts( 	
			double dTsNow,             //当前时间戳
			IplImage *pSrc2VGrayImg,   //给定的2值灰度图像
			int  nPtClearThres = 25,   //清除面积<n的外轮廓
			int  nSaveExtCtuPtTh = 25, //只保存面积>n的外轮廓
			float fFillRateTh = 0.0004f //填充面积比率>f的内轮廓
		);

	//获取 检测到的轮廓信息 的指针
	vector<fkContourInfo> * mvGetCtusInfoPointer( )
	{
		return (&m_vctCtusInfo);
	}

	//获取 检测到的轮廓上的点的切线和法线信息 的指针
	vector<StruPtTNLineInfoVector> * mvGetCtusPtTNLineInfoPointer( )
	{	
		return (& m_vctCtusPtTNLineInfo);
	}

	//获取得到给定rect所对应的size阈值
	static CvSize mvGetSizeThreshOfGiveRect(
			const CvRect &rct, 
			IplImage *pCarWImg,	IplImage *pCarHImg,		
			float fXSmlRate, float fYSmlRate );

	//获取得到给定点所对应的size阈值
	static CvSize mvGetSizeThreshOfGivePt(
			const CvPoint &ptCet, 
			IplImage *pCarWImg, IplImage *pCarHImg,		
			float fXSmlRate, float fYSmlRate );

	//获取得到所有的小轮廓
	static vector<int> mvGetAllSmallContours(
			vector<fkContourInfo> &vctCtusInfo, //给定的2值前景图像
			IplImage *pCarWImg,		//车辆宽map图像
			IplImage *pCarHImg,		//车辆高map图像
			float fXSmlRate = 0.2f, 
			float fYSmlRate = 0.2f 
		);

	//获取得到所有的宽度很大高度很小的轮廓
	static vector<int> mvGetAllBigWSmlHCtus(
			vector<fkContourInfo> &vctCtusInfo,  //给定轮廓
			IplImage *pCarWImg,		//车辆宽map图像
			IplImage *pCarHImg,		//车辆高map图像
			float fXSmlRate,   //与车辆宽的比值要求
			float fYSmlRate    //与车辆高的比值要求
		);

	//获取得到所有的宽度很小高度很大的轮廓
	static vector<int> mvGetAllSmlWBigHCtus(
			vector<fkContourInfo> &vctCtusInfo,  //给定轮廓
			IplImage *pCarWImg,		//车辆宽map图像
			IplImage *pCarHImg,		//车辆高map图像
			float fXSmlRate,   //与车辆宽的比值要求
			float fYSmlRate    //与车辆高的比值要求
		);

	//对给定序号的进行轮廓模式设置
	static void mvSetCtuModeToArrayOfGiveIndex(
			int *nACtuMode,
			const vector<int> &vectGiveIdx,	
			int nSetMode
		);

	//寻找到给定轮廓中的小轮廓(较矮瘦||较高瘦||较矮胖)
	static bool mvGetSmlCtus( 
			vector<int> &vectCtusMode,   //轮廓的模式
			vector<fkContourInfo> &vctCtusInfo, //给定轮廓
			IplImage *pCarWImg,			//车辆宽map图像
			IplImage *pCarHImg,			//车辆高map图像
			IplImage *pShowRgbImg		//用于显示的彩色图
		);

	//获取得到轮廓的窄轮廓部分
	static bool mvGetNarrowPartOfCtu(
			vector<CvPoint> &vectStaEndIdx4Part, //符合部分的始终序号
			StruPtTNLineInfoVector &ctuPtTNLineInfo, //轮廓点线信息
			IplImage *pCarWImg,		//车辆宽map图像
			IplImage *pCarHImg,		//车辆高map图像
			float fXThresholdRate,	//与车辆宽的比值要求
			float fYThresholdRate,	//与车辆高的比值要求
			bool bGetSmallThanThreshold=true //是否获取小于阈值的轮廓部分
		);

	//获取目标所形成的窄阴影轮廓部分
	static bool mvGetNarrowPartOfGiveCtus( 
			vector<int> &vectCtusIdxOfNarrowPart,  //窄轮廓所对应的轮廓序号(输出)
			MvCvPoint2DVector &My2DVector4CvPoint, //窄轮廓的起始终止点序号(输出)
			const vector<int> &vectIsSmlCtu,	   //各给定轮廓是否被认为为小轮廓
			vector<StruPtTNLineInfoVector> &vectCtusPtTNLineInfo, //各轮廓的点信息
			IplImage *pCarWImg,		//车辆宽map图像
			IplImage *pCarHImg,		//车辆高map图像
			float fXSmlRate,		//x方向的比率要求
			float fYSmlRate,        //y方向的比率要求
			IplImage *pShowRgbImg   //用于显示的彩色图
		);


	//显示给定的轮廓
	void static mvShowGiveContours( 
			IplImage *pShowRgbImg, 
			const vector<fkContourInfo> &vctCtusInfo,
			const vector<int> &vectShowCtusIdx,
			CvScalar colorV, int nRadius, int nThick
		);

	//显示给定的轮廓点线信息的部分
	void static mvShowPartOfGiveContoursPtTNLine( 
			IplImage *pShowRgbImg, 
			StruPtTNLineInfoVector &ctuPtTNLineInfo, 
			const vector<CvPoint> *pVectStaEndIdx4Part,
			CvScalar colorV, int nRadius, int nThick
		);

	//填充轮廓片断所对应的图像
	bool static mvFillImg4CtusParts( 
			IplImage *pWaitFillImg, //用于填充的图像 
			CvScalar &ScalarFill,   //用于填充的颜色
			vector<StruPtTNLineInfoVector> &vectCtusPtTNLineInfo, //点线轮廓
			vector<int> &vectCtusIdxOfNarrowPart,  //轮廓所对应的轮廓序号
			MvCvPoint2DVector &My2DVector4CvPoint, //轮廓片断所对应的起始终止点序号
			vector<fkContourInfo> &vectCtusInfo    //点轮廓
		);

private:
	//计算轮廓上的点的线信息
	void mvCalcCtuPtsLineInfo( 	
			IplImage *pSrc2VGrayImg,  //给定的2值灰度图像	
			const int nCtuPtCalcRadius = 4,  //对轮廓点计算的半径
			const int nMaxCalcCtuPtCnt = 200 //对一个轮廓最多的轮廓点数目
		);

private:
	double m_tsCalcCtus;  //轮廓计算的时间戳
	
	vector<fkContourInfo> m_vctCtusInfo; //检测到的轮廓信息

	//检测到的轮廓上的点的切线和法线信息
	vector<StruPtTNLineInfoVector> m_vctCtusPtTNLineInfo;
	
	IplImage *m_pRgbShowImg;

}StruCtusApp;


//-----------------------------------//

typedef struct StruObjConfStampImgSet
{
	MvTimeStampImg m_narrowFkImg;  //较为狭窄的前景二值图
	MvTimeStampImg m_shadowImg;	   //阴影图像

	MvTimeStampImg m_vehBotShadowImg;	//车底阴影图
	
	void mvShow( IplImage *pGrayImg, double dTsNow );

}MvObjConfStampImgSet;



//轮廓应用于目标确认的结构体
typedef struct StruObjConfirmByCtus
{
public:
	StruObjConfirmByCtus( );
	~StruObjConfirmByCtus( );

	void mvInitObjConfirmByCtus( );
	void mvUninitObjConfirmByCtus( );
	
	//获取得到2值图像的轮廓信息(主接口)
	bool mvGetContoursInfoOf2VImg(	
			double   dTsNow,    //当前的时间戳
			IplImage *pFk2VImg, //给定的2值前景图像
			IplImage *pCarWImg, //车辆宽map图像
			IplImage *pCarHImg  //车辆高map图像
		);

	//获取"轮廓模式"的指针
	vector<int> * mvGetCtusModePointer( );

	//获取"检测到的轮廓信息"的指针
	vector<fkContourInfo> * mvGetCtusInfoPointer( );

	//获取"轮廓上的点的切线和法线信息"的指针
	vector<StruPtTNLineInfoVector> * mvGetCtusPtTNLineInfoPointer( );
	
	//获取"狭窄部分所对应图"的指针及其对应的时间戳
	IplImage* mvGetNarrow2VImgPointerAndStamp(
			double  &dTsCalc   //其计算的时间戳（输出）
		) ; 

private:
	void mvFree( );	
	void mvInitVar( );	

private:
	double      m_dTsCalc;
	StruCtusApp m_ctusApp;     //轮廓应用器
	vector<int> m_vectCtuMode; //轮廓所对应的模式
	IplImage    *m_pNarrow2VImg; //获取的较狭窄部分所对应的图像

}ObjConfirmByFkCtus;


//目标确认结构体
typedef struct StructObjConfirm
{
public:
	//在候选区域中是否有目标
	static bool mvIsHaveObjectInArea(
		IplImage *pShowRgbImg,		//显示的RGB图像
		IplImage *pSmoothGradImg,	//平滑后的梯度图 
		IplImage *pSmoothBgGradImg,	//平滑后的背景梯度图
		CvRect rectGiveArea			//候选区域
		);

}AnObjConfirm;

#endif