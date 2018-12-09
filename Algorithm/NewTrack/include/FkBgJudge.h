//中值背景模型建造
#ifndef _FK_BG_JUDGE_H_
#define _FK_BG_JUDGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "comHeader.h"  //放最后

using namespace MV_MATH_UTILITY;
using namespace MV_IMGPRO_UTILITY;

#ifndef AN_MAX_BGIMG_CNT   
	#define AN_MAX_BGIMG_CNT 10		//最多存的背景图像的张数
#endif
#ifndef AN_TWOBGIMG_INTERVAL
	#define AN_TWOBGIMG_INTERVAL 100.0  //两张背景图像之间的间隔(单位为s)
#endif
#ifndef AN_RESIZE_BGIMAG_AREA
	#define AN_RESIZE_BGIMAG_AREA 100000.0f  //背景图像resize后的面积
#endif

#ifndef AN_MAX_BGJUDGE_TERM_CNT   
	#define AN_MAX_BGJUDGE_TERM_CNT 100	 //最多存的背景判断特征的周期次数
#endif

#ifndef AN_MAX_FKIMG_CNT   
	#define AN_MAX_FKIMG_CNT 10			//最多存的前景图像的张数
#endif
#ifndef AN_MAX_GHOST_TERM_CNT   
	#define AN_MAX_GHOST_TERM_CNT 50	//最多存的ghost特征的周期次数
#endif

//用于判断背景是否发生变化的网格特征
#ifndef AN_FEAT_FOR_BGJUDGE  
	#define AN_FEAT_FOR_BGJUDGE	
	typedef struct _an_gridFeat4BgImg
	{
		int  nAvgGray;    //平均灰度
		int  nGradPer;    //梯度百分比

	}AnGridFeat4BgImg;

	typedef struct _an_gridFeat4BgJudge
	{
		int  nAvgGray;    //平均灰度
		int  nGradPer;    //梯度百分比

		bool bVaild2Bef;     //与前图的相差是否可用
		int  nDiff256V_Bef;  //与前图的灰度差分值
		int  nDiff2VPer_Bef; //与前图的灰度差分2值化的百分比
	
	}AnGridFeat4BgJudge;
#endif

//用于ghost检测的图像集
#ifndef AN_IMGSET_FOR_GHOST_DETECT  
	#define AN_IMGSET_FOR_GHOST_DETECT	
	typedef struct _anGhostDetImgSet
	{
		IplImage *pDiffMap;
		IplImage *pDiffMapSobel;
		IplImage *pInFrameSobel;
		IplImage *pAndImg;
		IplImage *pOrImg;
	//	IplImage *pAndInteImg;
	//	IplImage *pOrInteImg;

		_anGhostDetImgSet( );
	
		//初始化变量
		void initVar( );
		
		//开辟图像
		void createImgSet( CvSize sz );

		//释放ghost检测的图像集
		void releaseImgSet( );

	}AnGhostDetImgSet;
#endif

//用于ghost清除的网格特征
#ifndef AN_FEAT_FOR_GHOST_REMOVE  
	#define AN_FEAT_FOR_GHOST_REMOVE	
	typedef struct _an_gridFeat4GhostRemove
	{
		int  nAvgGray;    //平均灰度
		int  nDiffPtPer;  //帧差点百分比
		int  nFkPtPer;    //前景点百分比
		int  nGradPer;    //梯度百分比
	}AnGridFeat4GhostRemove;
#endif


	//用于运动和静止前景检测的集合
#ifndef AN_SET4MOVESTOPFK  
	#define AN_SET4MOVESTOPFK	
	typedef struct _anMoveStopFkSet
	{
		bool bGetStopMoveFk;    //是否获取静止和运动
		bool bExistStopFk;      //当前存在停止前景
		bool bExistMoveFk;      //当前存在运动前景

		vector<int> vecFkAreaGridIdx;
		vector<int> vecMayStopGridIdx;

		IplImage *pStopAreaFkImg;
		IplImage *pMoveAreaFkImg;

		_anMoveStopFkSet( );

		//初始化变量
		void initVar( );

		//开辟图像
		void createImgSet( CvSize sz );

		//释放图像集
		void releaseImgSet( );

	}AnMoveStopFkSet;
#endif


class CForeBackGroundJudge
{
public:
	CForeBackGroundJudge(void);
	~CForeBackGroundJudge(void);

	//----初始化----
	void mvInitFkBgJudge( const CvSize szDst );
	void mvInitFkBgJudgeVar(  );
	void mvInitMemory4Grids(  );

	void mvUninitFkBgJudge(  );	

	//获取得到文件检测配置器的指针
	void mvGetFileDetCfgPointer( CDetectConfiger *pDetConfigerByFile )
	{
		m_p_cDetConfigerByFile = pDetConfigerByFile;
	}

	//获取得到图像网格的指针
	void mvGetImgGridPointer( 
		AnImgGridsStru *p_imgGridForCar,     //对车辆的图像网格
		AnImgGridsStru *p_imgGridForPeo,     //对行人的图像网格
		AnImgGridsStru *p_imgGridForFkGhost  //对前景ghost清除的图像网格
		) 
	{
		m_p_imgGridForCar = p_imgGridForCar; 

		m_p_imgGridForPeo = p_imgGridForPeo;	

		m_p_imgGridForFkGhost = p_imgGridForFkGhost; 
	}

	//获取光照结果存储器的指针
	void mvGetIllumResSavePointer( IllumResSaveStru *_pIllumRes ) 
	{
		m_pIllumResSave = _pIllumRes;
	}

	//处理图像
	void mvProcess(	bool bUpdateBgImg,    //是否更新背景图像
					double   dTsNow,      //当前的时间戳
					IplImage *pBgImg,     //当前背景灰度
					IplImage *pFkImg,     //当前前景2值图
					IplImage *pGrayImg,     //当前灰度图
					IplImage *pIntBgGrayImg,     //背景灰度积分图
					IplImage *pIntBgGrad2VImg ); //背景梯度2值化积分图	

	//检测ghost前景
	bool mvDetectGhostAndMvStFk( double dTsNow, //当前时间戳
				IplImage *lpNowBgImg,		    //当前的背景图像
				IplImage *lpGrayImg,            //当前的灰度图像
				IplImage *lpGrayInteGrateImg,   //灰度积分图像
				IplImage *lp2VDiffInteGrateImg, //2值化帧间差积分图像
				IplImage *lp2VForeInteGrateImg,	//2值化前景积分图像
				IplImage *lp2VGradInteGrateImg  //2值化梯度积分图像
			);

	//获得当前各网格的状态结果
	AnImgGridsStru* mvGetGridsStatusResult( 
				vector<int> **pVecFkAreaGridIdx, 
				vector<int> **pVecMayStopGridIdx 
			);

	//判断给定区域是否为ghost区域
	bool mvIsGhostArea( 
				CvPoint ltPt,	//给定区域的左上点
				CvPoint rbPt,	//给定区域的右下点
				CvSize useSz,   //给定区域所在的图像的大小
				float fGhostRatioTh=0.4f, //认为是ghost时的比率要求
				bool bUseEasyMode=false  //是否采用简单模式来判断
			);

	//判断区域所对应的背景是否发生改变
	void mvIsBgChangeOfGiveAreaTime(
			double  dTsSta, double dTsEnd, 
			CvPoint ltPt, CvPoint rbPt, 
			CvSize  useSz, 
			vector<double> &vectTsChange );

	//获取给定时间内的光照情况，获取其改变的时间戳
	void mvGetIllumInfoOfGiveTime( 
			double dTsSta, double dTsEnd,  
			vector<double> &vectTsNoSureSta, 
			vector<double> &vectTsNoSureEnd,
			vector<double> &vectTsNoChangeEnd, 
			vector<double> &vectTsChangeSta ); 

private:
	#ifdef SHOW_GRID_GHOSHT
		//显示ghost判断
		void mvShowGhostDetect(	double	 dTsNow,    //当前时间戳
					IplImage *lpGrayImg,		    //当前的灰度图像
					vector<int> vecFkAreaGridIdx,   //为前景区域的网格序号
					vector<int> vecMayStopGridIdx   //可能的停止网格区域
			);
	#endif
private:
	//网格的运动检测
	bool mvGridsMotionCheck( double dTsNow,		//当前时间戳
				IplImage *lpGrayInteGrateImg,   //灰度积分图像
				IplImage *lp2VDiffInteGrateImg, //2值化帧间差积分图像
				IplImage *lp2VForeInteGrateImg,	//2值化前景积分图像
				IplImage *lp2VGradInteGrateImg, //2值化梯度积分图像
				vector<int> &vecFkAreaGridIdx,  //为前景区域的网格序号
				vector<int> &vecMayStopGridIdx  //可能为停止的网格序号
			);

	//是否为可能的停车区域,要求：
	bool mvIsMayStopGrid( int nGridIdx,	 //当前网格的索引号
					int nTermCnt,		 //网格之前所对应的周期数目
					int nATermNo[],      //网格之前所对应的周期索引号
					AnGridFeat4GhostRemove *pGhostFeat //当前的网格特征
			);

	//计算给定网格区域的边缘相似性，判断其是否为ghost
	void mvCalcEdgeSimilarity( 
				const vector<int> *pVecMayStopGridIdx, //可能的停车网格序号
				IplImage *lpGrayImg,    //当前的灰度图
				IplImage *lpNowBgImg    //当前的背景图
			);

private:
	//添加一幅前景图像
	int mvAddOneFkImg( IplImage *pFkImg,  //传入的当前背景图
					   double dTsNow );   //传入的当前时间戳

	//添加一幅背景图像
	int mvAddOneBgImg( IplImage *pBgImg,   //传入的当前背景图
					   double dTsNow,      //传入的当前时间戳
			double dAddInterval=AN_TWOBGIMG_INTERVAL );//两背景图的时间间隔

private:
	//处理前景图像
	void mvProcessFkImg( double dTsNow,       //当前的时间戳
						IplImage *pBgImg,     //当前背景灰度
						IplImage *pFkImg,     //当前前景2值图
						IplImage *pGrayImg ); //当前灰度图	

private:
	//处理背景图像
	void mvProcessBgImg( double   dTsNow,        //当前的时间戳
			 IplImage *pIntBgGrayImg  = NULL,    //背景灰度积分图
			 IplImage *pIntBgGrad2VImg = NULL ); //背景梯度2值化积分图		

	//将背景图的特征保存到各网格中去
	void mvSaveBgImgFeatToGrid(	IplImage *pIntBgGrayImg, //背景灰度积分图
						    IplImage *pIntBgGrad2VImg ); //背景梯度2值化积分图 

	//得到当前背景图是否和原来的背景图的差别
	bool mvGetDiffOfBgImgForNowBef( bool &bHaveBigDiff );


private:
	//获取得到静止或运动的前景图像
	bool mvGetStopAndMoveFkImg( 
			AnImgGridsStru	*pImgGrid, 
			vector<int> *pVecFkAreaGridIdx,
			vector<int> *pVecMayStopGridIdx,	
			IplImage *pFk2VImg,
			bool &bExistStopFk,
			IplImage *pStopFkImg, 
			bool &bExistMoveFk, 
			IplImage *pMoveFkImg );

private:	
	#ifdef SAVEIMG_FOR_FKIMGCHECK
		//保存和检查前景图像
		void mvSaveFkImgForCheck( 
				double   dTsNow,	  //当前的时间戳
				IplImage *pFkImg,     //当前前景2值图
				IplImage *pGrayImg ); //当前灰度图		 
	#endif

	#ifdef SAVEIMG_FOR_BGIMGCHECK
		//保存和检查背景图像
		void mvSaveBgImgForCheck(
				IplImage *pNowImg,			//当前背景图
				IplImage *pNowPreDiffImg ); //两背景差分图
	#endif

private:
	CDetectConfiger	 *m_p_cDetConfigerByFile; //文件检测配置器的指针 

	AnImgGridsStru	 *m_p_imgGridForCar;	 //对车辆的图像网格的指针
	AnImgGridsStru	 *m_p_imgGridForPeo;	 //对行人的图像网格的指针
	AnImgGridsStru	 *m_p_imgGridForFkGhost; //对前景ghost的图像网格的指针

	IllumResSaveStru *m_pIllumResSave;    //光照结果存储器的指针

private:
	bool     m_bResizeBgImg;      //是否采用resize背景图像
	double   m_dTsBgImgAdd;       //当前背景图像的添加时间戳
	double   m_dTsLastBgImgAdd;   //上一幅背景图像的添加时间戳

	IplImage *m_pABgImage[AN_MAX_BGIMG_CNT]; //背景图像的指针
	CycleReplace m_bgImgStoreCR;  //背景图像存储的循环覆盖体

	IplImage *m_pAFkImage[AN_MAX_FKIMG_CNT]; //前景图像的指针
	CycleReplace m_fkImgStoreCR;  //前景图像存储的循环覆盖体

	IplImage *m_pNowPreDiff2VImg;   //当前和前面的背景差分图像 

	#ifdef SAVEIMG_FOR_BGIMGCHECK
		ANImgSaveStruct m_imgSaveForBgCheck;
	#endif
	#ifdef SAVEIMG_FOR_FKIMGCHECK
		ANImgSaveStruct m_imgSaveForFkCheck;
	#endif
	
private:
	bool m_bAddOneFkImg;   //添加了一幅前景图(不一定为每帧的前景图均加入)
	bool m_bAddOneBgImg;   //添加了一幅背景图(不一定为背景图一更新就加入)
	
	IplImage *m_pNowFkImg;
	IplImage *m_pNowBgImg;

	//ghost检测所用的图像集
	AnGhostDetImgSet  m_ghostDetImgSet;
	
	//car grid
	AnGridFeat4BgImg   *m_ppGridBgImgFeat;      //保存的背景图像的特征
	AnGridFeat4BgJudge **m_ppGridBgJudgeFeat;  //保存的用于背景判断的特征
	CycleReplace   m_bgImgJudgeStoreCR;		   //背景图像判断存储的循环覆盖体

	//ghost grid
	AnGridFeat4GhostRemove **m_ppGridGhostFeat; //保存的用于ghost清除的特征
	CycleReplace   m_ghostRemoveStoreCR;		//ghost清除存储的循环覆盖体
	int			   *m_pLabelAsGhostCnt;			//将各网格标记为ghost的次数
	bool		   *m_pNowBeLabelAsGhost;		//当前被标记为ghost的网格

public:
	AnMoveStopFkSet m_moveStopFkSet;       //运动和静止前景集

};


#endif