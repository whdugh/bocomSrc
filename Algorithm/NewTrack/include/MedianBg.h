//中值背景模型建造
#ifndef _MEDIAN_BG_H_
#define _MEDIAN_BG_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "MvImgProUtility.h"
#include "AnalysisStruct.h"

using namespace MV_IMGPRO_UTILITY;

#ifndef AN_MEDIAN_COUNT
	#define AN_MEDIAN_COUNT 15 
#endif

#ifdef SHOW_MEDIAN_BGIMG_RESULT
	enum enum_show_median_bg
	{
		SHOW_MEDIAN_LONG_SHORTBG = 0,
		SHOW_MEDIAN_LONGBG,		
		SHOW_MEDIAN_SHORTBG
	};
#endif

typedef struct _medianBgBasicImgSet
{
	IplImage *grayImg;  //读入的灰度图像
	IplImage *pMaskImg; //读入的mask图像

	IplImage *pSBgImg;  //短期背景图像
	IplImage *pLBgImg;  //长期背景图像

	IplImage *pFkImg;   //前景图像

	_medianBgBasicImgSet( )
	{
		grayImg  = NULL;
		pMaskImg = NULL;
		pSBgImg  = NULL;
		pLBgImg  = NULL;
		pFkImg   = NULL;
	};
}MedianBgBasicImgSet; 


typedef struct _medianBgHistSet
{
	uchar  ***p_sBHisList;  //短期
	uchar  ***p_lBHisList;  //长期
//	uchar  ***p_tBHisList;  //临时

	_medianBgHistSet( )
	{
		p_sBHisList = NULL;
		p_lBHisList = NULL;
//		p_tBHisList = NULL;
	};
}MedianBgHistSet; 


typedef struct _medianBgTsSet
{
	double dTsNow;     //当前的时间戳
	double dTsUpdate;  //背景更新的时间戳

	_medianBgTsSet( )
	{
		dTsNow    = -100000.0;
		dTsUpdate = -100000.0;
	};
}MedianBgTsSet; 


typedef struct _medianBgUpdateVarSet
{
	double   dTsUpdateSBThresh;       //短期背景的更新间隔
	double   dTsQuickUpdateSBThresh;  //短期背景的快速更新间隔(用于刚启动)
	int      nQuickUpdateSBgCnt;      //快速更新短期背景的次数

	int      nSBg2LBgUpdateRatio;     //短期背景和长期背景的更新比率

	int      nQuickUpdateLBgCnt;      //快速更新长期背景的次数
	int      nQuickUpdateLB_S2LRatio; //快速更新长期背景时,短期和长期的更新比率

	int      nSBgUpdateCnt;           //短期背景的更新次数
	int      nLBgUpdateCnt;           //长期背景的更新次数

	_medianBgUpdateVarSet( )
	{
		initVar( );
	}


	void initVar( )
	{
		dTsUpdateSBThresh      = 3.0;
		dTsQuickUpdateSBThresh = 1.0;  //1.0

		nQuickUpdateSBgCnt	   = 200;

		nSBg2LBgUpdateRatio    = 10;   //更新10次短期背景后更新1次长期背景

		nQuickUpdateLBgCnt     = 40; 
		nQuickUpdateLB_S2LRatio = 5;

		nSBgUpdateCnt = 0;
		nLBgUpdateCnt = 0;
	}

	//设置更新参数
	void setMedianBgUpdateParam(
			double _dTsUpdateSBThresh,
			double _dTsQuickUpdateSBThresh,
			int _nQuickUpdateSBgCnt,
			int _nSBg2LBgUpdateRatio,
			int _nQuickUpdateLBgCnt,
			int _nQuickUpdateLB_S2LRatio,
			int _nSBgUpdateCnt,
			int _nLBgUpdateCnt )
	{
		dTsUpdateSBThresh = _dTsUpdateSBThresh;      
		dTsQuickUpdateSBThresh = _dTsQuickUpdateSBThresh; 
		nQuickUpdateSBgCnt = _nQuickUpdateSBgCnt;     

		nSBg2LBgUpdateRatio = _nSBg2LBgUpdateRatio;    

		nQuickUpdateLBgCnt = _nQuickUpdateLBgCnt;       
		nQuickUpdateLB_S2LRatio = _nQuickUpdateLB_S2LRatio;  

		nSBgUpdateCnt = _nSBgUpdateCnt;          
		nLBgUpdateCnt = _nLBgUpdateCnt;           
	}
}MedianBgUpdateVarSet; 

typedef struct _medianBgOutParamSet
{
	int      m_nListNo;     //图像序列号

	bool     m_bRead_SBg;    //是否读入了短期背景
	bool     m_bRead_LBg;    //是否读入了长期背景

	bool	 m_bVaild_SBg;   //短期背景是否可用
	bool	 m_bVaild_LBg;   //长期背景是否可用

	bool     m_bUpdate_SBg;  //当前帧是否更新了短期背景
	bool     m_bUpdate_LBg;  //当前帧是否更新了长期背景

	_medianBgOutParamSet( )
	{
		initVar( );  	
	}

	void initVar( )
	{
		m_nListNo   = 0;

		m_bRead_SBg = false;    
		m_bRead_LBg = false;    

		m_bVaild_SBg = false;  
		m_bVaild_LBg = false;   

		m_bUpdate_SBg = false; 
		m_bUpdate_LBg = false;  
	}

}MedianBgOutParamSet; 

class CMedianBgBuilder
{
public:
	CMedianBgBuilder(void);
	~CMedianBgBuilder(void);

	void mvInitMedianBgBuilder( const CvSize sz, IplImage *pMask,
			bool bLongUpdate = false, int nBiListInitVal = -1 );
	void mvUninitMedianBgBuilder(  );

	//获取灰度背景和背景信息
	void mvUpdateBackground( double dTsNow, IplImage *pGrayImg );

	//前景获取
	void mvGetFkImgAndInfo( int nIndex, double t_now, bool bDay, 
		   IplImage *pFDiffImg,  IplImage *pRoadMaskImage, 
		   vector<fkContourInfo> &vctForeContours, 
		   IplImage *pForeImg, bool &m_bUseBg );

#ifdef SHOW_MEDIAN_BGIMG_RESULT
	//函数功能：显示背景图结果
	void mvShowBgimgResult( int nShowMod=SHOW_MEDIAN_LONGBG );
#endif


public:
	MedianBgTsSet        m_tsSet;
	MedianBgBasicImgSet  m_basicImgSet;
	MedianBgOutParamSet  m_bgOutParamSet;

	MedianBgUpdateVarSet m_updateVarSet;

	MedianBgHistSet      m_bgHistSet;
	

private:
	void mvUpDateMedianBg(  );

	//对中值背景缓存进行更新的快速算法
	void mvQuickUpdateMedianBg( const IplImage *pGrayImg,
		   IplImage *pMedianBg,uchar ***pList,IplImage *pMaskImg );

private:
	int	m_nHeight, m_nWidth; 
};


#endif