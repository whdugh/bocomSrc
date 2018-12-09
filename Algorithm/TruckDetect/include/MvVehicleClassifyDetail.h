/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvVehicleClassifyDetail.h
* 摘要: 简要描述本文件的内容
* 版本: V1.9
* 作者: 杨凯鹏 
* 完成日期: 2009年9月15日
*/

#ifndef MVVEHICLECLASSIFYDETAIL__H__
#define MVVEHICLECLASSIFYDETAIL__H__

#include <cv.h>
#include <highgui.h>
#include "ml.h"
#include <algorithm>
#include <vector>
#include "cvaux.h" 
#include "cxcore.h" 

#include "MvLineSegment.h"
#include "Mv_CarnumDetect.h"
#include "MvTypeUtility.h"
#include "MvTruckBusClassify.h"


//--------------控制宏的定义------------ 
#define VERSION_NUBER ("Version2014.06.21\n") //版本序号

#ifndef LINUX
//#define  MV_WIN_VER
#endif

//#ifndef MV_WIN_VER
//#include "cal_main.h"
//#endif

//#define MV_FROE_NEWVEHICLETYPE
//#define MV_NONCARD_NEWVEHICLETYPE
#define NEWVEHICLETYPE
#define MV_SAVEIMG_DEBUG
#define MV_MULTTHREAD
#define COMPRESSSVM   //svm压缩模型
//如果关掉下句，就没有任何debug记录
//#define  DEBUG_TYPE_ON


#ifdef DEBUG_TYPE_ON
	#define DEBUGSAVE
	#define  DEBUG_TRUCK
#endif

#ifndef DEBUG_TRUCK
	#define SET_TYPECODE(x)
#else
	#define SET_TYPECODE(x) m_nStatus=x
#endif

#ifdef DEBUG_TYPE_ON
	#ifdef LINUX
		#include <dirent.h>
		#include <unistd.h>
		#include <sys/stat.h>
		#include <sys/types.h>
		#define _chdir(a) chdir(a)
		#define _mkdir(a) mkdir(a, 0777)
		#include <stdio.h>
	#else
		#include <direct.h>
	#endif
#endif

#ifndef OBJECTCAR
#define OBJECTCAR
typedef struct CAR_INFO   //车辆数据结构体
{
	CvRect  plate_rect; //车牌位置,相对于原图
	CvRect  car_rect; //车辆位置,相对于原图
	int plate_color; //车牌颜色（1蓝 2黑 3黄 4白 5其他）
	int carnumrow; //单双牌（1单牌 2双牌 3其他）
	int nCarNumDirection; //车牌的方向(0前牌 1尾牌 2未知)
	char carnum[7]; //车牌号码
	CAR_INFO()
	{
		plate_rect = cvRect(0,0,0,0);
		car_rect = cvRect(0,0,0,0);
		plate_color = 5;
		carnumrow = 3;
		nCarNumDirection = 0;
		memset(carnum,0,sizeof(char)*7);
	}
} car_info;
#endif

#ifndef MV_WIN_VER
#define  USE_RGB_PIC
#endif

using namespace std;
using namespace cv;


typedef struct stVehicleRegion
{
	CvRect VehicleRect;	//整个汽车所在区域

	CvRect HeadRect;	//

	CvRect BrandRect;	//车底部，车牌附近区域
	CvRect FaceRect;	//
	CvRect WindowRect;
	CvRect BodyRect;	//车体区域
	CvRect ForeHeadRect;	//高出车窗的区域，限于卡车

	TypicalEdge tpEdge[4];
	stVehicleRegion()
	{
		VehicleRect = cvRect(0,0,0,0);
		HeadRect = cvRect(0,0,0,0);
		BrandRect = cvRect(0,0,0,0);
		FaceRect = cvRect(0,0,0,0);
		WindowRect = cvRect(0,0,0,0);
		BodyRect = cvRect(0,0,0,0);
		ForeHeadRect = cvRect(0,0,0,0);
	}
}VehicleRegion;



class MvVehicleClassifyDetail  
{
private:
	IplImage *m_pImage,*m_pGrayImage;
	IplImage *m_pIntegrateImage;	//积分图像
 	IplImage *m_pRImage,*m_pGImage,*m_pBImage;
	BLine * m_pLines;
	int m_nLines;

	float m_f_arHomography[3][3];

#ifndef  MV_WIN_VER
	//cal m_cal;
#endif

	int m_nLeft,m_nTop,m_nRight,m_nBot;	//检测区域
	int m_nBrandColor;	//车牌颜色
	int m_nSmearNum;
	CvRect m_pSmearRect[20];	//这里最多20
	
	int m_nLightLevelHight;	//车灯的高度
	int m_nLightDis;	//车灯之间的距离(包括车灯长度),只适合夜间模式
	int m_nVehicleLampType;	//车灯类型，和汽车类型相对，只有在有车牌的情况下应用

	CvMemStorage *m_pStorage;
	CvSeq * m_pHLines,*m_pVLines;	//水平线段集合,垂直线段集合
	CvSeq * m_pSortHLines,*m_pSortVLines;	//排序后的水平线段和垂直线段集合
	
	int m_nMinBusWidth;		//bus的参考宽度
	int m_nMinCarWidth,m_nMaxCarWidth;		//小车的宽度范围
	
	double m_fZXScale;
	bool m_bIsNight;		//是否晚上

	float m_fUptoLine;	//截至区域，只记录y坐标


	int m_nStat_LongLine ;
	int m_nStat_BusBot ;	//bus底线数量
	int m_nStat_WinBot ;	//车窗底线数量
	int m_nStat_WinTop ;	//车窗顶部线数量

	MvTruckBusClassify* m_pTBC;

	MvTruckBusClassify* m_pTBCBlue;//蓝牌车车型分类

	bool mvCenterAligned(stTypicalEdge e, stTypicalEdge f, int maxSizeWidth, double thres);
	void DeleteBGLine(bool bIsNight = false);
	void Orientation();
	
	static int y2cmp_func( const void* _a, const void* _b, void* userdata );
	static int bcmp_func( const void* _a, const void* _b, void* userdata );
	static int ycmp_func(const void *_a, const void *_b, void* userdata);
	static int LenCmp_func(const void *_a, const void *_b, void* userdata);

	CvSeq* GetHLines(const double thres);	//取得水平线段
	CvSeq* GetVLines(const double thres);
	bool CheckBusBottomLine(stTypicalEdge e,int nH);	//判断一条线是否是一辆bus头部的底线
	bool CheckWinBottomLine(stTypicalEdge e,int nH);	//判断是否车窗的底线
	bool CheckWinTopLine(stTypicalEdge e,int nH);	//判断是否车窗的顶部
	float grayEdgeDiff(stTypicalEdge e,int nH);
	bool CheckTopSideLine(stTypicalEdge e,stTypicalEdge f,int nW);	//
	bool CheckBusBodyLine(stTypicalEdge e,int nW);	//判断是否存在较长的车身线
	bool ExistDoubleLongBodyLine(stTypicalEdge e,int nW);  //
	bool ExistTruckBox(TypicalEdge edgeWinTop,int nW);	//是否存在集装箱车顶
	bool CheckFaceLine(stTypicalEdge e,int nW,int nMidX=0);  //检测是否存在夹着头部的长直线存在	
	bool IsVehicleWindow(CvRect rect,int thesh=70);	//判断指定区域是否为车窗
	bool IsRGBRegion(int nStartX,int nStartY,int nWidth,int nHeight);  //判断指定区域是否为大红大蓝或大绿
	bool IsBlueRegion(int nStartX,int nStartY,int nWidth,int nHeight);
	void JoinLine(TypicalEdge *HLine1,TypicalEdge *HLine2);	//链接两条水平直线
	bool CanBeJoin(TypicalEdge HLine1,TypicalEdge HLine2);
	void AjustVehicleBottomEdge(const CvRect VRect,TypicalEdge *edgeBusBot);	//调整汽车底线位置
	int FindWindowLowPos(int nVehicleLen,TypicalEdge edgeWinBot,const CvRect VRect);
	int FindCarWindowPos(TypicalEdge tpEdge[4],int nVehicleLen);
	int mvFindEdgeIntersection(TypicalEdge e,BLine f,double *intersection);

	
	int GetVehicleWidthByVLine(stTypicalEdge *e);
	int GetEstimateLen(const TypicalEdge e, const int nH,const CvRect VRect);
	bool GetLampInfo(CvRect VBrandRect);	//取得相关车灯信息
	bool FindRectStruct(stTypicalEdge e,CvPoint *rectVertex,const int nDrct=1,int nW=0);	//寻找一条边上方或者下方存在的类似矩形结构
	

	
	bool FindOtherBrandFeature(CvSeq * linesgroup,bool bCheck[4],TypicalEdge tpEdge[4]);
	int DetectVehicle(CvRect *VehicleRects,bool bVBrand=false);	//汽车检测
	int DetectVehicleNightMode(CvRect *VehicleRects); //夜间汽车检测模式
	bool BrandAtMiddle(const bool bExist[4],const stTypicalEdge tpEdge[4],const CvRect VRect,const int nVehicleLen);
	bool ExistVertex(const stTypicalEdge e,float fThresh=0.25);
	bool ExistVertexAndLongVLine(const TypicalEdge edgeWintop,const int nVehicleLen,float fTresh=0.15);
	bool FindVehicleRegion(const CvRect VRect,VehicleRegion *VRegion);
	bool ExistLongVLine(const TypicalEdge e, const TypicalEdge f,const int nW);
	bool ExistLongTruckVLine(const TypicalEdge e, const TypicalEdge f,const int nW);
	bool ExistMoreShortVLines(const TypicalEdge e,const TypicalEdge f,int minx,int maxx,const int nThresh= 5);
	int FindWinTopEdge_YellowBrand(CvSeq *linesgroup,int nVehicleLen,bool bCheck[4],TypicalEdge tpEdge[4]);

	void PadEdge(const CvRect VRect, TypicalEdge * e);
	void InitRect(CvRect &rect);
	CvPoint2D32f GetWorldCoordinate(const CvPoint2D32f &pt);	//取得图像的世界坐标
	
	bool ExistDoubleRect(const stTypicalEdge e);
	bool FindRectEdge(stTypicalEdge e,stTypicalEdge rectEdge[4]);
	
	int mvTruckDetect(const IplImage *pImage,const CvRect rect,bool IsNight=false);		//卡车判断

	bool ExistTruckVLine(bool bExist[4],TypicalEdge tpEdge[4]);
	bool ExistMoreTruckVLine(bool bExist[4],TypicalEdge tpEdge[4]);
	bool ExistDoubleTruckVLine(bool bExist[4],TypicalEdge tpEdge[4]); 
	bool ExistVLine(TypicalEdge Edge1,TypicalEdge Edge2,float fThresh=.3f);
	bool ExistBetweenLine(const TypicalEdge e,const TypicalEdge f);
	bool ExistLongBiasLine(const TypicalEdge hLine,const CvRect VRect);

	bool ExistTowTruck(bool bCheck[4],TypicalEdge tpEdge[4],const int nVehicleLen);
	bool ExistMiniBusTop(const TypicalEdge e,const int nVehicleLen);
	bool ExistSUNVTop(const TypicalEdge edgeWinTop);

	bool IsExistBlueRegion(bool bCheck[4],TypicalEdge tpEdge[4],const int nStarty,const int nVehicleLen);
	bool IsExistVLine(const int nStartX,const int nStartY,const int nEndX,const int nEndY);
	bool ExistContainerTop(TypicalEdge edgeTop,const int nVehicleLen,const float fThresh=.3f);
	
	bool IsOtherVehicleLine(TypicalEdge edge);
	bool ExistBiasVLine(TypicalEdge Edge1,TypicalEdge Edge2,float fThresh=.2f);

	int GetInitLen(const CvRect VRect);			//根据车牌颜色和车牌宽度获取汽车的初始估计宽度
	int GetSymmetryLen(const CvRect VRect, const TypicalEdge e) const;

	int m_nBDHeight,m_nBDWidth;
	bool m_bGACard;		//是否为港澳牌

	vector<TypicalEdge> m_vecHLines;
	vector<TypicalEdge> m_vecVLines;


#ifdef DEBUGSAVE
	char  m_szSaveFileName[256];
#endif
    
	int m_nStatus;			//判断状态

	bool m_bIsForCarnum;	//是否是前牌检测

	bool m_pbCheck[4];
	TypicalEdge m_tpEdge[4];

	int m_nVehicleType;
	int m_nWidth;		//汽车宽度，
	
	//====================================================================================
	//调试中所用到的程序
#ifdef DEBUGSAVE
	void SaveDebugImage(const CvRect VRect, bool bChek[4],TypicalEdge tpEdge[4]); 
#endif
	//
	int FindBottomEdge(CvSeq *lineGroups,const CvRect VRect,TypicalEdge &botEdge);
	bool AjustUpThresh(const TypicalEdge e, int &nUpThresh);
	//====================================================================================
	//前牌

	int DetectTruckByBrand(const CvRect VRect);
	
	int DetectYellowBrand(const CvRect VRect, CvSeq* lineGroups, bool bCheck[4], TypicalEdge tpEdge[4]);
	int DetectOtherBrand(const CvRect VRect, CvSeq* lineGroups, bool bCheck[4], TypicalEdge tpEdge[4]);

	int DetectTruckByBrandNightMode(const CvRect VRect);

	int ClassifyYellowBrandNightMode(bool bCheck[4],TypicalEdge tpEdge[4],const CvRect VRect,
		int nVehicleLen,VehicleRegion vRegion);
	int ClassifyOtherBrandNightMode(bool bCheck[4],TypicalEdge tpEdge[4],const CvRect VRect,
		const int nVehicleLen,VehicleRegion vRegion);
	int ClassifyOtherBrand(bool bCheck[4],TypicalEdge tpEdge[4],const CvRect VRect,VehicleRegion vRegion);
	int ClassifyYellowBrand(bool bCheck[4],TypicalEdge tpEdge[4],const CvRect VRect,VehicleRegion vRegion);

	
	//===================================================================================
	//尾牌检测
	int DetectTruckByTailBrand(const CvRect VRect);
	
	int newTruckType(IplImage *pImage, const CvRect rt);
	int newTailTruckType(IplImage *pImage, const CvRect rt);
	int newTruckTypeBlue(IplImage *pImage, const CvRect rt);
#ifdef NEWVEHICLETYPE
private:
	char buffer_path[512];
	//////////////////////
	HOGDescriptor *hog;
	int nImgNum;
	int dim;
	////样本矩阵，nImgNum：横坐标是样本数量， WIDTH * HEIGHT：样本特征向量，即图像大小  
	int PCAdi;
	CvSVM svm;
	CvMat* pMean ;
	CvMat* pEigVecs ;
#ifndef MV_MULTTHREAD
	CvMat* SVMtrainMat;
	CvMat* pResult;
	IplImage* ResizeImg;
	vector<float> descriptors;
#endif	
	//控制初始化
	int first;

#ifdef MV_FROE_NEWVEHICLETYPE
	CvSVM svm_fore;
	CvMat* pMean_fore ;
	CvMat* pEigVecs_fore ;
#endif

#ifdef MV_NONCARD_NEWVEHICLETYPE
	CvSVM svm_NonCard;
	CvMat* pMean_NonCard ;
	CvMat* pEigVecs_NonCard ;
#endif
#endif

public:
	
	MvVehicleClassifyDetail();
	virtual ~MvVehicleClassifyDetail();
	

	void SetHomograph(float homography_image_to_world[3][3]);	//设置标定参数
	void FindHomography(float *image,float *world);

	//设置TSAI标定参数
#ifndef MV_WIN_VER
	//void mvSetTSAIData(int width,int height,int nCount,float *ptImg,float *ptWorld);
#endif

	void SetNightMode(bool bMode);	//设置夜间模式
	
			
#ifdef DEBUGSAVE
	void SetSaveFileName(const char * fileName);
#endif
	int mvTruckDetect(const IplImage *pImage,const CARNUM_CONTEXT Brand, bool IsDay=true,
		bool IsForeCarnum=true,bool IsMoreDetail=false,int *nMoreDetail = NULL); //卡车判断

	int mvTruckDetect(const IplImage *pImage,const CAR_INFO *carInfo, bool IsDay=true,
		bool IsForeCarnum=true,bool IsMoreDetail=false,int *nMoreDetail = NULL); //卡车判断, 自定义车辆结构体

	/*
	mvNonCardTruckDetect：车辆类型判断
	输入：	
	pImage：需要处理的图片，由于车牌判别和卡车类型细分用的不是同一个图片，在传入该图片的同时，需要设置ROI区域；
	Brand：车牌信息
	IsDay：白天为true,夜晚为false
	IsMoreDetail：是否为细分车型（默认为否）
	nMoreDetail：细分车型返回值
	返回值
	return：第一大类
	nMoreDetail：第二子类（细分结果）
	*/
	int mvNonCardTruckDetect (const IplImage *pImage, CARNUM_CONTEXT Brand, bool IsDay=true, 
						bool IsMoreDetail=false, int *nMoreDetail = NULL);

	int mvGet_Vision( char* strPath  );//模型版本

	void Truck_Destroy();
	int Truck_Init( char* strPath = NULL );

#ifdef NEWVEHICLETYPE
	//设置模型初始化路径
	void mvSetPath(char* strPath);
#endif
	int mvMoreInfo();
};

#endif 
