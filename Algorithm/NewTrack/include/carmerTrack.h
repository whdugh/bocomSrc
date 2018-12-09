// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

// StopDetector.h: interface for the StopDetector class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _CARMERTRACK_H_
#define _CARMERTRACK_H_

//#define MVDEBUGON_CAMERA
#ifdef MVDEBUGON_CAMERA
	#ifndef DEBUG_SCALE
		#define  DEBUG_SCALE						//相机移动拉伸的调试
	#endif
	#ifndef DEBUG_CAMCTRL_FLAW		//相机的控制流程调试
		#define  DEBUG_CAMCTRL_FLAW  //相机移动拉伸的调试
	#endif
#endif

#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

#ifdef LINUX
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#ifndef CHDIR
		#define CHDIR
		#define _chdir(a) chdir(a)
	#endif
	#ifndef MKDIR
		#define MKDIR
		#define _mkdir(a) mkdir(a, 0777)
	#endif
	#ifndef ACCESS
		#define ACCESS
		#define _access(a, b) access(a, b)
	#endif
#else
	#include <direct.h>
#endif


#include "ipp.h"

#include "sift_descr.h"
#include "MvLineSegment.h"
#include <algorithm>
#include <vector>
#include "fasthessian.h"

#define  USE_MEANSHIFT
#ifdef USE_MEANSHIFT
	#include "CTrack.h"             //meanShift跟踪
#endif

#include "comHeader.h"  //放最后

#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef OBJMAXSIZETRNUM
	#define OBJMAXSIZETRNUM  200
#endif
#ifndef TRACKMAXLEN
	#define TRACKMAXLEN  50
#endif

#ifndef SIFT_WIN_SIZE
	#define SIFT_WIN_SIZE 4
#endif
#ifndef SIFT_BINS
	#define SIFT_BINS 8
#endif
#ifndef SIFT_DIMS
	#define SIFT_DIMS 128
#endif

#ifndef MAXSKIPAREANUM
	#define MAXSKIPAREANUM 50
#endif

#ifndef MAXSKIPAREAPTNUM
	#define MAXSKIPAREAPTNUM 50
#endif

#ifndef MAXCARMERKEYPTNUM
	#define MAXCARMERKEYPTNUM 1000
#endif

#ifndef DEFAULT_SCALE_ONETIME
	#define DEFAULT_SCALE_ONETIME 2.0  //相机拉近时一次想拉近多少倍
#endif

#ifndef MAX_NOMATCH_TIME
	#define MAX_NOMATCH_TIME  2  //最多可多少次连续匹配不上
#endif

using namespace std;
using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_AN_ANLGORITHM;
using namespace MV_SMALL_FUNCTION;

///////////////////////////////////////////////////类CMoveTracker//////////////////////////////////////////////////
typedef struct ObjTrackPoint
{
	int             nMatchTime;     //该点的匹配次数
	CvPoint2D32f    pt;
	uchar		descriptor[SIFT_DIMS];
	
	ObjTrackPoint( )
	{
		nMatchTime = 0;
	}
}ObjTrackPoint;  //目标的轨迹点


typedef struct ObjTrack
{
	int             nLen;						//目标的轨迹的长度
	int             nNoMatchTime;       //目标的轨迹连续未匹配上的次数
	ObjTrackPoint	trPt[TRACKMAXLEN];  //目标的轨迹的轨迹点
	bool            bEventInitTr;       //是否为事件的初始轨迹

	ObjTrack( )
	{
		nNoMatchTime = 0;
		bEventInitTr = false;
	}
}ObjTrack;      //目标的轨迹


typedef struct ObjectInfo
{
	bool         bNeedMv;   //需要进行移动
	CvPoint   cetPt;     //目标的轨迹中心
	CvPoint	ltPt;      //目标的左上点
	CvPoint	rbPt;      //目标的右下点
	int            nTrNum;                    //目标的轨迹数目
	short        objTrNo[OBJMAXSIZETRNUM];  //目标的轨迹序号

	int            nADiffLevelTrNum[4];   //目标所对应的轨迹

	int             nTotalMatchTime;				//总共匹配的次数
	CvPoint2D32f    ptAvgMvOfMatch[TRACKMAXLEN];    //每次匹配的平均位移   

	bool            bEstimateObjRect;               //目标的顶点是否为估计的

	ObjectInfo( )
	{
		cetPt = cvPoint(0, 0);
		bNeedMv = false;
		nADiffLevelTrNum[0] = 0;
		nADiffLevelTrNum[1] = 0;
		nADiffLevelTrNum[2] = 0;
		nADiffLevelTrNum[3] = 0;

		nTotalMatchTime = 0;

		bEstimateObjRect = false;
	}

}ObjectInfo;


typedef struct FeatureStruct
{
	int  nKeyPtNum;
	CvPoint2D32f *pKeyPt;
	uchar **ppDescriptor;

}featureStruct;


typedef struct ImgWHStruct
{
	int nWidth;
	int nHeight;
	int nWidthStep;

}ImgWHStruct;


typedef struct DebugInfoStruct
{
	char  cDirectoryName[108];
	char  cDateTime[108];
	int		 nFrameNo;
}DebugInfoStruct;


class CMoveTracker
{
public:
	bool  m_bHadInit;

#ifdef USE_MEANSHIFT
public:
	CTrack  *m_pMeanShiftTracker;
#endif

public:
	DebugInfoStruct debugInfo;

	int  m_nTrackNo;
	int  m_nImgWidth;
	int  m_nImgHeight;
	CvPoint   m_pt_carMaxsizeXy;

	IplImage *m_lpBeforeScaleImg;   //相机放大前的图像
	IpVec m_vecSurfPts;				//存放SURF特征点
public:
	int			  m_nSkipRgnNum;
	int			  *m_p_ASkipAreaPtNum;  
	CvPoint2D32f  **m_p_ASkipAreaPt;  

private:
	bool m_bDay;

//	IplImage      *m_maskImg;
	CvRect        m_roadRoiRect;
	int           m_keypt_cnt;
	CvPoint2D32f  *m_p_AKeyPt;

	int		  m_nMaxExtrackKeyptNum;
	bool      m_bRoadFar;

	MySIFT    m_classSift;

	int m_nChannel_ID;    //通道号
	int m_nConfigMvXNum;    //x移动的配置
	int m_nConfigMvYNum;    //y移动的配置
	int m_nConfigScale2Num;  //拉近至2倍
	int m_nConfigScale4Num;  //拉近至4倍
	int m_nConfigScale8Num;  //拉近至8倍

	int m_nMatchRectScaleTime;  //匹配框需要放大的倍数


#ifdef DEBUG_SCALE
	IplImage *m_lpInitImg;         //事件发生时的图像
	IplImage *m_lpSimMvScaleImg;   //模拟的相机移动和拉伸时的图像
	IplImage *m_lpLastImg;         //上次的相机移动时的图像
	IplImage *m_lpLastMvCameraImg; //上次相机移动的图像
#endif


	bool m_bLastTimeCarmerMvX;
	bool m_bLastTimeCarmerMvY;

	int  nKeyPtLevelNum[3];  //分层次角点数目

	int      m_nBurnsLineNum;
	LSDRect *m_pBurnsLines;

public:
	int m_nCameraMvXNum;  //上次相机x方向移动多少格
	int m_nCameraMvYNum;  //上次相机Y方向移动多少格

	int m_nCamMvXPara;  //上次相机x方向的移动参数
	int m_nCamMvYPara;  //上次相机Y方向的移动参数

	int m_nTotalCamMvXPara;  //相机x方向的移动总参数
	int m_nTotalCamMvYPara;  //相机Y方向的移动总参数

	CvPoint2D32f m_sumMvPt;        //目标轨迹平均移动的像素
	CvPoint2D32f m_avgMvToParaPt;  //每一参数所对应的平均移动像素

	int   m_nJoinMatchTrNum;
	int   m_nVaildMatchTrNum;

private:
	bool m_bMoveX_small;   //是否对相机x方向做小幅度的移动
	bool m_bMoveY_small;   //是否对相机y方向做小幅度的移动

	int  m_nScaleToHalfNum;  //将近处目标放大到画面一半时的格数
	int  m_nScaleToAllNum;   //将近处目标放大到画面大小时的格数

	IplImage * m_pBefStaticImg;		//相机邻近静止时的图像

private:
	featureStruct m_lastKeyPt_Feature;
	featureStruct m_lastMvCamer_keyPt_feature;

	ImgWHStruct   m_imgDataWhInfo;
	bool        m_bAfterScaleFirstJudge;  //是否为相机拉伸后的第一次是否稳定的判断
	int           m_nFinishScaleJudgeTime;  //判断相机是否拉伸完并稳定的次数
	CvRect   m_lastObjRect;            //目标上次的rect区域大小
	int           m_nLastScaleNum;          //上次拉伸的格数
	float        m_fLastScaleTime;         //上次拉伸的倍数
	int           m_nTotalScaleTime;        //总共拉伸的次数

private:
	CvPoint    m_obj_initLtPt;             //目标初始的左上点
	CvPoint    m_obj_initRbPt;			  //目标初始的右下点
	CvRect     m_befScaleObjRect;          //目标在拉近前的rect区域
	CvRect     m_wantScaleObjRect;         //拉近后欲得到的目标rect区域
	bool			m_bMoveInScale;             //在拉伸过程中是否移动了
	float			m_fRatioObjWidthToLineLen;  //在线段长度和车宽之比

private:
	int				m_nMvCameraTime ;                                     //移动相机的次数
	CvPoint	m_pAMvCameraParam[TRACKMAXLEN];   //相机怎么移动的

private:
	//对图像来提取用于匹配的角点
	bool mvfind_match_harris(IplImage *grayImage, IplImage *maskImage, CvRect roadRoi, int &keypt_cnt, CvPoint2D32f *p_AKeyPt);

	//利用移动方向来确定匹配框的大小
	bool mvget_matchRect(CvPoint &lt_mvPt, CvPoint &rb_mvPt);

	//对目标轨迹和角点进行匹配, 判断相机是否移动过	
	bool mvis_carmer_move(uchar **descriptor);
	bool mvis_carmer_moveX(uchar **descriptor);
	bool mvis_carmer_moveY(uchar **descriptor);

	//将本次的角点及特征进行保存以进行跟踪
	void mvgive_feature_toTracker(int nKeyPtNum, CvPoint2D32f *pKeyPt, uchar **descriptor);

	//对一灰度图进行特征提取及保存
	void mvget_feature_of_grayImg( IplImage *grayImg, featureStruct &imgFeature );

	//判断本次目标的特征是否和上次的特征在小范围内是一致的
	bool mvis_sameFeature_ofTwoTime(int nKeyPtNum,    CvPoint2D32f *pKeypt,    uchar **ppDescriptor, 
								    int nComKeyPtNum, CvPoint2D32f *pComKeypt, uchar **ppComDescriptor,
									IplImage *lpNowGrayImg=NULL);

	//判断本次目标的特征是否和上次的特征在小范围内是不同的
	bool mvis_diffFeature_ofTwoTime(int nKeyPtNum,    CvPoint2D32f *pKeypt,    uchar **ppDescriptor, 
								    int nComKeyPtNum, CvPoint2D32f *pComKeypt, uchar **ppComDescriptor,
								    IplImage *lpDrawImg);

	#ifdef DEBUG_SCALE	
		//保存相机是否移动完的判断图像
		void mvsave_image_of_cameraMvFinishJudege(IplImage *lpDrawImg, bool bFirstTime);
	#endif
	
	//对目标轨迹和角点进行匹配, 以得到匹配框应该放大的倍数
	int mvget_matchRect_scaleTime(uchar **descriptor, CvPoint lt_mvPt, CvPoint rb_mvPt);

	//利用相机移动方向和匹配总情况，将本帧匹配上的角点进行筛选,以减少误匹配
	bool mvget_vaildMatch_keypt(int *pAkeyPtMatchTrNo, int &nVaildNum, int *pAVaildNo, IplImage *lpDrawImg=NULL, FILE *fp_match=NULL);

	//利用前面信息来计算到目前为止，每个参数所对应的平均位移
	bool mvcal_avgDist_everyPara_withPreKown(int nLastSumMvXPara, int nLastSumMvYPara,
											 CvPoint2D32f &mvPt, CvPoint2D32f &avgPerPt, FILE *fp=NULL);
	bool mvcal_avgDist_everyPara_withPreKown_new(int nLastSumMvXPara, int nLastSumMvYPara,
									         CvPoint2D32f &mvPt, CvPoint2D32f &avgPerPt, FILE *fp=NULL);

	//估计目前还需要移动的参数
	void mvestimate_need_moveParameter( int &nXNeedMvPara, int &nYNeedMvPara, FILE *fp_match=NULL);


	//得到按指定倍数来拉伸相机放大后目标rect区域
	CvRect mvget_objectRect_afterScale(CvRect srcRect, float fScale);
	CvRect mvget_objectRect_afterScale(CvRect srcRect, float fScale, CvPoint &lt_pt, CvPoint &rb_pt);

#ifndef LINUX
	//在本地模拟目标相机移动后放大后的图像，并修改了rgbImg和grayImg图像
	void mvget_objectRect_debug1( IplImage *rgbImg, IplImage *grayImg, int nMvX, int nMvY, float fScale );

#endif

	//获取得到和目标宽度较为一致的线段
	bool mvget_objectWidthLine( int nLineNum, int *p_nALineNo, int *p_nAIndex, double *p_dAData,
		                        int nWantObjWidth, int nWantObjHeight, int &nObjectEstWidth,
							    IplImage *lpDrawImg=NULL );

	//获取得到图像某区域内的线段
	void mvget_horiLines(IplImage *grayImage, bool isDay, CvRect extRect);

	//得到图像区域内的长线段
	bool mvget_longLines(int &nLongLineNum, int *p_nALongLineNo);

	//判断长线段是否过长
	bool mvis_longLines_tooLong(int nLongLineNum, int *p_nALongLineNo, IplImage *grayImage);

	//保存是否需进一步放大的判断图像
	void mvsave_isNeedScaleMore_judge(IplImage *grayImage); 

	//获得目标第一次需要拉伸的格数
	int mvget_initScaleNum(CvPoint pt_maxsizeXy);

	//根据上次的结果来获得本次需要拉的格数
	int mvget_wantScaleNum(int nLastScaleNum, float fLastScaleTime, float fEstWidth);

	//获取目标的rect区域
	bool mvget_objectRect(IplImage *lpInPutImg, bool bDay, CvRect lastObjRect, float fWantScale,
		                  CvPoint2D32f &linesCetPt, float &fRatioObjWidthToLineLen, CvRect &objEstRect); 

	//在相机拉近过程中,若发现目标线段质心较偏,则需要转动相机
	bool mvGetMvNum_whenCameraScale(CvPoint2D32f &linesCetPt, int &nMvXNum, int &nMvYNum);

public:
	ObjectInfo		m_obj;
	int						m_nSetTrNum;   //集合中的轨迹数目
	ObjTrack			*m_p_setTr;       //集合中的轨迹

	int                     m_nObjInitPtNum;
	ObjTrackPoint	*m_pObjInitFeature;

public:
	int   m_nMvX;
	int   m_nMvY;
	float m_fScaleTime;

public:
	CMoveTracker();
	virtual ~CMoveTracker();

public:
	void CMoveTrackerInit();

    void CMoveTrackerUnInit();

	//得到一些初始信息
	bool mvget_initInfo(int w, int h,
		                bool bDay, CvRect roadRoi, int nMaxExtrackKeyPtNum, bool bRoadFar,
						CvPoint pt_carMaxsizeXy,
						int nSkipNum, int *p_ASkipAearPtNum, CvPoint2D32f **p_ASkipAearPt);

	//得到事件发生时的灰度图像
	void mvget_initGrayImg(IplImage *srcImg);

	//得到模拟的相机转动和拉近时的灰度图像
	bool mvget_simMvScaleGrayImg(char *imgData, int nMvX, int nMvY, float fScale);

	//设置相机上次的运动状态
	void mvset_carmer_lastTime_mvStatus(bool bMvX, bool bMvY);

	//判断相机运动方向是否一致
	bool mvis_camera_mvConsistency(int &nMvXDir, int &nMvYDir);

	//得到相机的运动方向
	bool mvget_carmer_xyMvDircetion(int &nMvXDir, int &nMvYDir,bool bJudgeConsistency=true, bool bJudgeDecrease=true);

	//利用目标的中心点位置来得到相机的运动方向
	bool mvget_camera_xyMvDircetion(CvPoint cetPt, int &nMvXDir, int &nMvYDir);

	//得到相机移动的方向，以指导下次的区域判断
	void mvget_cameraXyMvDir(int nXMvNum, int nYMvNum);

	//得到相机移动的参数，以指导下次的区域判断
	void mvget_cameraXyMvParameter(int nXMvParameter, int nYMvParameter);

	//移动图像(用于模拟相机移动)
	bool mvmove_image(IplImage *lpInitRgbImg, IplImage *lpMvRgbImg, bool bNeedJudeg=true, bool bFirstTime=false);
	
	//判断相机是否已经移动完了
	bool mvis_finish_carmer_move(IplImage *lpInitRgbImg, bool bFirstTime);
	bool __mvis_finish_carmer_move(IplImage *lpInitRgbImg, bool bFirstTime);

	//判断目标是否已经匹配上了,在匹配不上时给出x,y方向移动的参数
	bool mvmatch_object(IplImage *lpInitRgbImg, int &nMvXPara, int &nMvYPara);

	//设置通道编号
	void SetChannelID(int nChannel)	{ m_nChannel_ID = nChannel;	}
	//设置x方向移动幅度
	void SetMvXNum(int nX)	{ m_nConfigMvXNum = nX;	}
	//设置y方向移动幅度
	void SetMvYNum(int nY)	{ m_nConfigMvYNum = nY;	}
	//设置拉近一倍移动幅度
	void SetScaleNum(int nScaleTo2, int nScaleTo4, int nScaleTo8)
	{		
		m_nConfigScale2Num = nScaleTo2; 
		m_nConfigScale4Num = nScaleTo4; 
		m_nConfigScale8Num = nScaleTo8; 
#if 0
		FILE *fp_print = fopen("scaleConf_debug.txt", "a");
		fprintf(fp_print,"get---nConfigScale248=%d,%d,%d\n", m_nConfigScale2Num, m_nConfigScale4Num, m_nConfigScale8Num);
		fclose(fp_print);
#endif
	}


	//保存事件发生后的相机移动过程图
	void mvsave_serialImage(IplImage *lpInitRgbImg, int nFrameNo); 

	//获得当前的日期和时刻(md_hms)
	void mvget_nowDateTime(char *cDateTimeStr);

	//获得初始图像数据的宽高等信息
	bool mvget_initImageDataInfo(ImgWHStruct imgWhInfo);

	//判断是否需要进一步进行缩放
	bool mvis_needScaleCameraMore(uchar *imgData, bool isDay);
	bool mvis_needScaleCameraMore(IplImage *lpInPutImg, bool isDay);

	//对相机进行转动和拉伸后的图像模拟
	void mvget_moveScale_Carmer_image(IplImage *lpInputImg, IplImage *scaleImg);

	//利用点的距离将点进行分类
	int mvClassPoints(int nPtNum, CvPoint *p_APt, CvPoint distPt, int *p_nAClassNo);

	//从分类结果中选择出数量最多的一类
	void mvChooseMaxPtNumClass(int nPtNum, int *p_nAClassNo, int &nMaxClassNo, int &nMaxClassPtNum);

	//得到目标的初始左上和右下点
	void mvget_objectInitLtRbPt(CvPoint ltPt, CvPoint rbPt)
	{
		m_obj_initLtPt = ltPt;
		m_obj_initRbPt = rbPt;
	}
	
	//得到目前的图像（将其大小resize，并转换为灰度图）的特征来作为相机移动前的特征
	void mvget_srcImg_feature_toBeMvCamera(IplImage *lpInitRgbImg, bool bNeedCvt);

	//得到目标中原始的轨迹在目前的rect区域，并将新轨迹加入到集合和车中
	bool mvget_nowRect_ofInitTrack(char *imgData, CvPoint &nowLtPt, CvPoint &nowRbPt); 

	//画目标rect区域
	void mvdraw_objectRect(char *imgData, CvRect objRect);

	//保存对相机拉伸进行控制前的结果图
	void mvsave_ImageBeforeCameraControl(char *imgData, int nWidth, int nHeight);

	//对相机放大进行控制
	bool mvControl_cameraScale(char *imgData, int nScaleFrame, bool &bTerminate, int &nScaleNum, int &nMvXNum, int &nMvYNum);

	//获取放大所需要对应的拉伸参数
	void mvget_wantScaleTimeAndNum(bool bSimpleMod, float fMaxScale, float fScaleRatTo248[3], int nScaleNumTo248[3],
															float &fWantScale, int &nScaleNum);

	//获取移动所需要对应的移动方向
	void mvget_wantMoveDir(bool bMoveLR[2], bool bMoveUD[2], CvPoint scaleLtPt, CvPoint scaleRbPt);

	//获取放大后移动画面1/3所需要对应的移动参数
	void mvget_mvPara_of_wantMoveThird(float fWantScale, int &nMvXThird, int &nMvYThird);
	
	//依靠配置3个参数来对相机放大进行控制
	void mvControl_cameraScale_with3Parameter(char *imgData, int &nControlTime, int nAScaleNum[5], int nAMvXNum[5], int nAMvYNum[5]);

};

#endif  //_CARMERTRACK_H_
