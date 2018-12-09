// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary


#ifndef CARDETECT_H
#define CARDETECT_H

#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include "ipp.h"
#include <string>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <map>
#include "VideoShutter.h"
#include "MvLine.h"
#include "MvUtility.h"
#include "cal_main.h"

#define  TRACK


#ifdef TRACK
#include "CSiftTrack.h"
#include "Corner.h"

//#define TRACK_SHOW
//#define IMGSUB              //观察目标前景运动-针对非机动车
//#define TRACK_TIME 

#endif





using namespace std;

#ifdef TRACK
using namespace Shut_Corner;
using namespace Shut_Track;
#endif

#define  MAXCHALNUM 10



         

class mvvideoshutter
{

public:

	mvvideoshutter();

	~mvvideoshutter();

	//VideoRect:虚拟检测区域
	//nWidth:车牌在虚拟检测区域的宽
	//nHeight:车牌在虚拟检测区域的高
	//vector<mvvideorgnstru> mChannelReg 车道区域
	bool mv_Init( CvRect VideoRect, int nWidth, int nHeight, vector<mvvideorgnstru> &mChannelReg,
		int nImgWidth,int nImgHight, int ncount,float *ptImage_cord, float *ptWorld_cord );

	//pOriImage:原始图像,为BGR格式
	//ts:图片时间戳 单位：ms

	//车牌检测的总体函数（main）
	bool mv_DetectEveryFrame( IplImage *pImage,long long FramTime,bool bIsDay = false);

	//vResult：输出结果
	bool mv_GetVideoResult( vector<skip_video_out_result> &vResult);

	//类释放函数
	bool mv_UnInit();

private:

	//寻找车道线与检测区域的相交线
	void  MvInTerSerctLine();

	//判断点是否在多边形区域内
	bool mv_is_point_in_polygon(const mvvideorgnstru &Chanlpos,const CvPoint2D32f pt);

	//otsu阈值
	unsigned int mvgetotsuthershold(const IplImage *simg, const unsigned int nWidSample,const unsigned int nHigSample );

	//迭代阈值
	int mvInteratonThershold( const IplImage *pImg, int nWidSample,int nHigSample);


	//直方图平滑化
	void mvHistogSmooth( int *Grayhist, int histNum,int teringNum);


	//截取区域灰度化
	void mvDetcAreaImgGray(IplImage *pImage);

	void mvGrayImgMorTop(const IplImage *pGrayImage,IplImage *pHatImage);

	//过滤函数
	bool mvCarRectTell ( IplImage* GrayImage,const  CvRect CarRect ,bool bKmean = false);

	//灰度梯度函数
	bool mvGraySub (const IplImage* pGrayImg,int &nHigEnd);

	//sobel函数
	bool mvGetSobelImage( const IplImage *pImage,int &ImgCarHigStart,int &ImgCarHigEnd);

	//寻找波峰波谷函数
	bool mvSeekPeakVallegPos(const int *Grayhist, int histNum,int *PeekPos, int *valPos);

	//对过滤后的邻居车牌区域进行标记
	void mvCombineCarRect(vector <CvRect> CarRec, int *pCarLab,const int pCarInital);

	//提取最终车牌
	void mvFinalCarDerive(const vector <rect_lig> &Rec , vector <rect_lig> &CarRec);

	//控制同一时刻某车道内只允许有一辆车牌
	void mvControlOneCarInChal(vector <rect_lig> &DstCarRec);

	//控制车牌间距存在一定间距
	void mvControlCarDis(vector <rect_lig> &DstCarRec);

	//添加当前车牌的时间信息存储到m_CarnowInf中
	void mvAddCarTime(const vector <rect_lig> &Rec);

	//把当前检测到的车牌信息载入到链条中
     void InserCarToChain();

    //针对车牌情况,给任意时刻车牌节点赋上车牌更新时间信息
	int mvCalUpdaTime(const skip_carnum_result *pElem ,int &nForCarAvg,int nFrstAvg, CvRect FirstCarRect,int CarSpeed);
    
	//判断是否为夜幕（图像除车灯、车牌外都很黑）
	void mvGetImgDark(const IplImage* pGrayImg);

	//前后车牌区域亮度比较的过滤
	void mvCarLigComp(const IplImage* pGrayImg,vector <rect_lig> &DstCarRec);

	//计算车速
	void mvGetCarSpeed(const skip_carnum_result *pElem,int &Speed);

    //灰度梯度阈值过滤
	bool mvGrayThred(const IplImage* pGrayImg,int &nHigEnd);

	bool MvBocPix(const IplImage* pGrayImg);

	//车灯检测
	bool mvGetLight(const IplImage *pGrayImage);

	//前景提取运动目标*******************************************
	void mvGetObjForSub(const IplImage *pForImage, const vector <rect_lig> &CarPlat,bool bChange = false);


	//获取运动目标
	void MvGetMovObj(const vector<Peekinf> &Peek,int *nProjX,vector <MovObj> &Obj);

	//对区域重叠的进行何必
	void MvFilObjByPlate(vector <MovObj> &Obj);

	//对区域重叠的进行何必
	void MvComBinObj(vector <MovObj> &Obj);

	//通过车灯，车牌增加目标状态
	void  MvSetObjSta(vector <MovObj> &Obj,const vector <rect_lig> &CarPlat);

	//更新车牌过滤信息
	void  MvCarFilUpdat(const vector <rect_lig> &CarPlat);

	//更新前景
	void  MvUpdatMovObj(const vector <MovObj> &Obj);


	 void MvUpdatMovObjChain(const MovObj *Obj);

	 //
	 void  MvReslByMove(vector<skip_video_out_result> &vResult);

	//圆的拟合检测
	void MvLightDetct(IplImage *image ,CvSeq* result_seq,CvSeq *CriclArr,vector<CvRect> BriArea,
		const vector<circle_attribution> &CirByBig,int nXZoom = 1, int nYZoom =1);


	//灯的图像坐标增加标签
	void mvChagLigToWorldPos(CvSeq *CriclArr,vector<ligworld_inf>&CirlWoldpos);

	//找出车灯匹配序号
	void mvFindLigMatchNum(vector<ligworld_inf>CirlWoldpos,vector<ligmatch_inf> &Ligmatch);

	//匹配灯的灰度均差
	int mvLigGraySub(const IplImage*pGrayImage,CvRect Ligrect, CvRect MatLigRec);

	//匹配灯的前方灰度均值
	int mvLigUpGray(const IplImage*pGrayImage,CvRect Ligrect);

	//非机动车判定
	bool mvJueSigLig( IplImage*pGrayImage,CvRect Ligrect);

	//判断车道是否有一辆车灯
	void mvDeriveSigLigInChal(CvSeq *CriclArr,uchar *nMachLig,vector <int>&nSigLig);
    
	//控制爆闪
	bool mvShutContol( int &nFirstAvg,bool &IsFirstSet,int i,bool IsCausedByLig, skip_carnum_result *pElem,int Graysub,vector<skip_video_out_result> &vResult);


	//车牌检测的总体函数（main）
	void mv_LocatPlat( IplImage *pImage,const CvRect Carplat, vector <CvPoint> &CarLineOut);

	//
	bool mvKmean(const IplImage *pImage,int nHigEnd);

	bool mvPlateByKmean(IplImage *pImage);

	bool mvGetSlope(const IplImage *TrdImg);
	
	//判读车牌的车道信息
	void mvJudgeChanl(skip_carnum_result *pElem);

	//CarRect坐标：像对原始---检测区域---坐标
	ChanlData mvJudgeChanlByRec(CvRect CarRect);

	//获得---可能---右车灯造成的路面前景区域
	void   mvGetChanlCasByRoad(vector<CvRect> &RecByRoad) ;

	void   MvFilObjByRoadChanlLig(vector <MovObj> &Obj);

#ifdef TRACK
	//获取轨迹提供的运动
	void  MvAddShurObjByTrack(const vector <MovObj> &Obj);
#endif
	

	//通过亮度框寻找车道
	void MvLightByRec( vector<CvRect> &BigRec,vector<circle_attribution> &CirByBig,int nRidusTrd);

	static bool IsYHig(const CvRect &r1, const CvRect &r2);
	
	void mvFilCarLin(vector<CvPoint> &CarOuLine,const CvRect &Carplat);

    void mvSegRectUnit(vector <CvRect> &SegRect,int nDisTrd);
	
	bool mvRemoPlaByRoadLig(CvRect CarRec);

	bool MeetNOVehicShut(int nMovDis);
      



private:

#ifdef TRACK
      CsiftTrack m_SifTrack;
	  Corner     m_Corner;
#endif

	vector<Line_inf> m_IntesecLine; //对应到压缩的m_ShrinkGrayimage的坐标上去；
	vector<car_filarea> m_CarFile;
	bool m_bSaveImg;
	bool m_bSavPtintTime;
	bool m_bShutOut;
	IplImage *m_Grayimage;
	IplImage *m_ShrinkGrayimage;
	IplImage *m_SwapImg;
	IplImage *m_LastImg;
 
	// 标定相关
	//MyCalibration_New m_calibration;
	cal  m_calibration;
	vector <int> m_LigColum;

	CvRect m_VideoRect;
	int m_nWidth;
	int m_nHeight;
	int m_detareYmov;
	int m_carintegralHig;
	int m_detareXmov;
	int m_carintegralWid;
	
	vector<mvvideorgnstru> m_ChannelReg;
	//vector<int> m_LigColum;
	int64 m_nFramSeq;

	IplImage *m_pMaskImg;
	IplConvKernel *m_kernel5x5;	
	IplImage *m_tempimage;
	IplImage *m_pForImg;

	


	CvMemStorage* m_storage;
	CvMemStorage* m_Movstorage;
    CvSet   *m_MovResu;

	CvSet   *m_CarNumResu;
	vector<car_current_inf> m_CarnowInf;
	int64 m_FramTime;
	int m_IsWhite;
	bool m_Dark; //车牌是否暗

	bool m__IsInitSucs;
	
	bool m_IsImgDark; //图像是否处于黑夜
	vector<rect_lig> m_LigResRect;
	vector<CvRect>  m_LigByRoad;
	CvRect m_LigRect;
	int64 m_HiShut[MAXCHALNUM];
	unsigned int *m_BusPass;
	int *m_HiShutIndex;
    mvPark m_ChanlPark[MAXCHALNUM];
	int m_CarAvg;
	int m_nLigRiX;
	int m_nLigRiY;
	bool m_bNoObj;
	uchar m_DetAreAvg;
	vector <ChanlFil> m_FilChanl;
	ChanlVeh m_ChanVel[MAXCHALNUM];
	CarHit m_CarHit[MAXCHALNUM];
	public:
	vector<skip_video_out_result> m_AddResult;
	


protected:

};

#endif

