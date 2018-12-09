
//全局的检测结果
#ifndef _AN_GLOBAL_DETECT_RESULET_H_
#define _AN_GLOBAL_DETECT_RESULET_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "comHeader.h"  //放最后

using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_SMALL_FUNCTION;

//smear检测结果保存结构
#ifndef MAX_SMEAR_COL
	#define MAX_SMEAR_COL  2000   //smear最多的列数
#endif
typedef struct StruSmearResult
{
public:
	bool   m_bDetSmear;   //是否检测到smear
	int	   m_nImgColNum;  //图像的列数

	double m_dDetectTs;   //检测的时间戳
	bool   m_bASmear[MAX_SMEAR_COL];   //该列是否为smear

public:
	StruSmearResult( );

	void mvInitVar( );

	//保存smear检测结果
	void mvSaveDetResult(bool bGet, int nImgW, double dTsNow, bool *bASmear);
}SmearResult;


//轨迹间的差异结果
#ifndef MAX_CALC_DIFFVAL_TRCNT
	#define MAX_CALC_DIFFVAL_TRCNT  2000  //计算差异的最多轨迹数目
#endif
typedef struct StruTracksDifferentResult
{
public:
	double m_dDetectTs;					//检测的时间戳
	IplImage *m_pTracksDiffValImg;      //轨迹间差异值图像(对应为*10的uchar)

	int    m_nCalcDiffTrCnt;   //当前计算差异的轨迹数目
	int    *m_nATrIdx2Img;     //轨迹序号对应于图像中行序号

public:
	StruTracksDifferentResult( );

	void mvInitVar( );

	//保存smear检测结果
	void mvSaveDetResult(bool bGet, int nImgW, double dTsNow, bool *bASmear);

	//对每帧均进行轨迹间差异结果保存的初始化
	bool mvInitTracksDiffResultEveryFrame(int nTrCnt,
						 int *nATrIdx, double dTsNow);

	//添加一对轨迹的结果
	bool mvAddOnePairTracksResult(int nTrIdx1, int nTrIdx2,
							 float fDiffVal, double dTsNow);

	//获取一对给定序号轨迹的差异值(返回值为非负数有意义)
	float mvGetPairTracksDiffVal(int nTrIdx1, int nTrIdx2, double dTsNow);

	//释放
	void mvUninitTracksDiffResult( );

}TracksDiffRes;


//简单的轮廓信息结果
typedef struct StructSimContourResult
{
public:
	double m_dDetectTs;			  //检测的时间戳
	bool   m_bAddFristThisFrame;  //是否为本帧第一次添加
	vector<fkContourInfo> m_vectContourRes;  //检测到的轮廓结果

public:
	StructSimContourResult( );
	~StructSimContourResult( );

	void mvInitVar( );

	//添加一条结果数据
	void mvAddOneResult(double dTsNow, const fkContourInfo &fkContour);

	//获取当前的轮廓检测结果数据指针
	vector<fkContourInfo> * mvGetResult(double dTsNow);

}SimContourRes;


//晚间寻找到的亮区结果
#ifndef MAX_BRIGHT_AREA_CONTOUR_NUM
	#define MAX_BRIGHT_AREA_CONTOUR_NUM  100  //晚间亮区数目
#endif


//亮区检测结果的结构体
typedef struct StructBrightAreaResult
{
	CvRect	rectCoutour;    //轮廓的外接rect
	bool	bIsBigArea;     //是否为大的区域
	vector<CvPoint> vectContourPts;  //轮廓的轮廓点

}AnBrightAreaRes;

typedef struct StruBrightAreaOnNightResult
{
public:
	double m_dDetectTs;					//检测的时间戳
	
	bool   m_bAddFristThisFrame;        //是否为本帧第一次添加

	vector<AnBrightAreaRes>  m_vectBrightArea;
	
public:
	StruBrightAreaOnNightResult( );
	~StruBrightAreaOnNightResult( );

	void mvInitVar( );

	//添加一条结果数据
	void mvAddOneResult(double dTsNow, const CvRect &rct, 
		   bool bBigArea, const vector<CvPoint> &vectPts);

	//获取当前的亮区检测结果数据指针
	vector<AnBrightAreaRes> * mvGetResult(double dTsNow);

}BrightAreaOnNightRes;


//---------车灯对的检测结果----------//

//车灯对检测结果的结构体
typedef struct StruLampPairDetResult
{	
	int nObjIdx;  //车灯所对应的目标序号

	CvRect	rectLefLamp;  //左灯rect
	CvRect	rectRigLamp;  //右灯rect
	CvPoint ptLTPairLamp; //两灯的左上点
	CvPoint ptRBPairLamp; //两灯的右下点

}AnLampPairDetRes;

typedef struct StruPairLampResult
{
public:
	double m_dDetectTs;					//检测的时间戳
	bool   m_bAddFristThisFrame;        //是否为本帧第一次添加
	vector<AnLampPairDetRes> m_vectLampPairRes;  //车灯对检测结果

public:
	StruPairLampResult( );
	~StruPairLampResult( );

	void mvInitVar( );

	//添加一条结果数据
	void mvAddOneResult(double dTsNow, const AnLampPairDetRes &lampPair);

	//获取当前的车灯对检测结果数据指针
	vector<AnLampPairDetRes> * mvGetResult(double dTsNow);

}AnPairLampResult;


//---------车灯所造成的灯光的检测结果----------//
//衰减线的信息结构体
typedef struct _MvShineLightMsg
{
	CvPoint	pts;	 //起点
	CvPoint	pte;	 //终点
	float   fThick;  //代表的线宽度
	float	fkTheory;
	float	fkSlope;
	_MvShineLightMsg()
	{
		pts = cvPoint(0,0);
		pte = cvPoint(0,0);
		fThick = 1.0f;
		fkTheory = 0.0f;
		fkSlope = 0.0f;
	}
}ShineLightMsg;


//车灯所造成的灯光的检测结果的结构体
typedef struct StruLampLightResult
{
public:
	double m_dDetectTs;					//检测的时间戳
	bool   m_bAddFristThisFrame;        //是否为本帧第一次添加
	vector<ShineLightMsg> m_vectLampLightRes;  //车灯对检测结果

public:
	StruLampLightResult( );
	~StruLampLightResult( );

	void mvInitVar( );

	//添加一条结果数据
	void mvAddOneResult(double dTsNow, const ShineLightMsg &lampLight);

	//获取当前的灯光检测结果数据指针
	vector<ShineLightMsg> * mvGetResult(double dTsNow);

}AnLampLightResult;


//全局的结果保存结构体
typedef struct StruGlobalDetectResult
{
public:
	SmearResult   m_smearResult;    //smear检测结果

	TracksDiffRes m_tracksDiffRes;  //轨迹间差异值结果

	SimContourRes m_bigFkContourRes;  //较大图像的前景轮廓结果

	BrightAreaOnNightRes m_brightAreaRes;  //晚间亮区检测结果

	AnPairLampResult m_pairLampRes;     //车灯对检测结果

	AnLampLightResult m_lampLightRes;   //车灯光检测结果

public:
	void mvInit( );    //初始化
	void mvUninit( );  //释放

}GlobalDetRes;



//////////////////////////////

#endif