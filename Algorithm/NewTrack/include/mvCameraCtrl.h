// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

// StopDetector.h: interface for the StopDetector class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _MVCAMERACTRL_H_
#define _MVCAMERACTRL_H_

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
#include <algorithm>
#include <vector>

#include "comHeader.h"  //放最后

#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAXSKIPAREANUM       //屏蔽区数目最大值
	#define MAXSKIPAREANUM 50
#endif
#ifndef MAXSKIPAREAPTNUM  //屏蔽区顶点数目最大值
	#define MAXSKIPAREAPTNUM 50
#endif

//#define DEBUG_CAMCTRL_NEWMETHOD

using namespace std;


//------------------球机和云台控制的类-------------------//
class CCamTrackCtrl
{
	//构造和析构
public:
	CCamTrackCtrl( );
	virtual ~CCamTrackCtrl( );

public:
	//初始化和释放
	void CCamTrackCtrlInit( );
	void CCamTrackCtrlUnInit( );

	//全局的信息传递(整个检测只需调一次)
	bool mvGetSkipAreaInfo(int nSkipNum, int *p_ASkipAearPtNum, CvPoint2D32f **p_ASkipAearPt);
	bool mvGetCamConfigInfo(bool bRead, int nT2C, int nB2C, int nL2C, int nR2C, int nS2, int nSMax);
	void mvGetSceneInfo(bool bFarRoad, int nSrcImgW, int nSrcImgH);

	//检测时信息传递(检测到停车事件均调用)
	void mvGetObjKeyPtInitInfo( vector<CvPoint2D32f> vctSrcKeyPt );
	void mvGetEventInitInfo(float fScale, CvPoint cetPt,CvPoint ltPt, CvPoint rbPt, CvPoint whPt);

	//接口函数
	bool mvCameraControl(IplImage *pSrcImg, double dTsNow, bool bFirst);
		
	//返回相机初次控制的参数
	void mvGetCamFirstTimeCtrlPara( int &nXMvP, int &nYMvP, int &nScaleP )
	{
		 nXMvP = m_nFirstMvXP;     
		 nYMvP = m_nFirstMvYP;      
		 nScaleP = m_nFirstMvSP;     
	};

	//临时测试函数
	void _mvSiftMatchFor2Img( );

private:
	void mvGetCamCtrlPara( int nX, int nY, float fS );

private:
	bool m_bHadInit;

private:
	int			  m_nSkipRgnNum;
	int			  *m_p_ASkipAreaPtNum;  
	CvPoint2D32f  **m_p_ASkipAreaPt;  
	CvRect   m_roadRoiRect;

private:		
	int m_nChannel_ID;            //通道号
	int m_nConfigMvXNum;    //x移动的配置
	int m_nConfigMvYNum;    //y移动的配置
	int m_nConfigScale2Num;  //拉近至2倍
	int m_nConfigScale4Num;  //拉近至4倍
	int m_nConfigScale8Num;  //拉近至8倍

private:		
	int m_nCfgT2C;         //将预置位时图像最上移动至中心的配置参数
	int m_nCfgB2C;         //将预置位时图像最下移动至中心的配置参数
	int m_nCfgL2C;         //将预置位时图像最左移动至中心的配置参数
	int m_nCfgR2C;         //将预置位时图像最右移动至中心的配置参数
	int m_nCfgS2;            //将预置拉时图像中目标放大至2倍的配置参数
	int m_nCfgSCarMax;  //将预置拉时图像中车放至最大(即小车占满画面)时的配置参数
	
	bool m_bRoadFar;
	int m_nSrcImgW;
	int m_nSrcImgH;

private:
	int m_nFirstMvXP;      //第一次左右移动的参数
	int m_nFirstMvYP;      //第一次上下移动的参数
	int m_nFirstMvSP;       //第一次放缩的参数

	vector <CvPoint> m_vctInitKeyPt;
	float        m_fScale;
	CvPoint  m_initCetPt;
	CvPoint  m_initObjLtPt;
	CvPoint  m_initObjRbPt;
	CvPoint  m_initObjMaxsizeXyPt;
};

#endif  //_MVCAMERACTRL_H_