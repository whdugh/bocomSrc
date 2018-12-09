#ifndef __MV_SHADOW_LIGHT_DETECTOR_H
#define __MV_SHADOW_LIGHT_DETECTOR_H

#include "comHeader.h"  //放最后


#define MAX_SHADOW_INFO_CNT 1000
#define MAX_SAVE_DIRECTION_CNT 100
#define BA 254 //定义前景边缘海拔值

#ifndef LINUX
	#define STAT_CNT_IN_TIME 50		//一段时间内统计的次数
#else
	#define STAT_CNT_IN_TIME 200
#endif

#ifndef LINUX
	#define SAVE_TIME 1		//相隔一段时间保存阴影信息
#else
	#define SAVE_TIME 30
#endif

//阴影点的模式
#ifndef SHADOW_PT_MODEL 
	#define SHADOW_PT_MODEL
	enum shadow_pt_model
	{
		SHADOW_PT_COLOR = 64, //利用颜色判断为阴影
		SHADOW_PT_OTHER = 255
	};
#endif

typedef struct _MvObjShadowInfo
{
	float fARate[4];  //上下左右四个象限区域出现的概率
	CvPoint ptObjCenter;//目标的中心点
	double ts;			//时间戳
	_MvObjShadowInfo()
	{
		for (int i=0;i<4;i++)
			fARate[i] = 0.0;
		ptObjCenter = cvPoint(0,0);
		ts = 0.0;
	}
}ObjShadowInfo;

typedef struct _MvDirectionReslut
{
	int nYear;		//年
	int nMonth;
	int nDay;
	int nHour;
	int nMin;
	float fStatTopLeftRate;
	float fStatTopRightRate;
	float fStatBottomLeftRate;
	float fStatBottomRightRate;
	_MvDirectionReslut()
	{
		nYear = 0;
		nMonth = 0;
		nDay = 0;
		nHour = 0;
		nMin = 0;
		fStatTopLeftRate = -1.0;
		fStatTopRightRate = -1.0;
		fStatBottomLeftRate = -1.0;
		fStatBottomRightRate = -1.0;
	}
}DirectionReslut;

//阴影区域判断结果
#ifndef SHADOW_JUDEGE_RESULT 
	#define SHADOW_JUDEGE_RESULT
	typedef struct _shadowJudgeResultStru
	{
	private:
		int m_nSize;  //阴影区域判断的最大数目
	
		CvRect m_rctAArea[MAX_CYCLE_SIZE];   //阴影区域
		CvSize m_szAUse[MAX_CYCLE_SIZE];     //所使用的图像大小

		int m_nAFkPtCnt[MAX_CYCLE_SIZE];     //前景点
		int m_nAShadowPtCnt[MAX_CYCLE_SIZE]; //阴影点 

		CycleReplace m_shadowJResCR;

	public:
	
		//-------------------阴影判断结果保存结构体-------------------//
		void mvInitShadowJRSStru( int _nSize )
		{
			if( _nSize<0 || _nSize>MAX_CYCLE_SIZE )
			{
				printf("error!input size(%d) should in [1,%d]\n",
					_nSize, MAX_CYCLE_SIZE);
				assert(false);
			}	

			m_nSize = _nSize;
		
			m_shadowJResCR.mvGiveMaxSize(m_nSize);
		}

		//添加一个元素
		int mvAddOneDataElem( double dTsNow, CvRect rctArea,
				CvSize sz, int nFkPtCnt, int nShadowPtCnt ) 
		{
			int nNow = m_shadowJResCR.mvAddOneElem(dTsNow);

			m_rctAArea[nNow] = rctArea;
			m_szAUse[nNow] = sz;

			m_nAFkPtCnt[nNow] = nFkPtCnt;
			m_nAShadowPtCnt[nNow] = nShadowPtCnt;

			return nNow;
		}		
	}ShadowJudgeResultStru;	
#endif

typedef struct _MvDeleteShadower
{
	int m_nArraySize;
	CvPoint *m_ForePointSet;		//前景点集
	CvPoint *m_BoudaryPointSet;	//边界点集
	CvPoint *m_NowAlPointSet;		//当前等势线点集
	CvPoint *m_NextAlPointSet;	//下一级等势线点集
	CvPoint *m_SeedPointSet;		//种子点集
	CvPoint *m_nowPtSet;			//当前点集
	CvPoint *m_neiPtSet;			//8领域点集
	IplImage *m_pAltitudeImage;	//等势图
	bool **m_bCompute;			 //点是否被处理过

public:
	//阴影消除主接口函数
	void mvShadowDelete(IplImage *pFkImage, IplImage *pMaxSizeImgW, IplImage *pMaxSizeImgH);

	//初始化阴影消除器
	void mvInit(CvSize fkSize);

	//释放阴影消除器
	void mvUninit();

private:
	//得到前景、边界的点集和初始等势图
	void mvGetFirstAltImg(IplImage *pFk2VImg, int &nForeNum, int &nBoundptNum);

	//得到等势图
	void mvGetAltitudeImg(int nBoundptNum);

	//判断点是否为边界点
	bool mvIsBoundaryPoint(IplImage *pFk2VImg, CvPoint pt);

	//得到8领域点
	void mvGet8NeiAreaPoint(CvPoint pt, CvPoint AreaPoint[]); 

	//得到种子点
	void mvGetSeedPoints(IplImage *pFkImage, IplImage *pMaxSizeImgW,
		IplImage *pMaxSizeImgH, int nForeNum, int &nSeedNum);

	//在8领域中是否为最小点
	bool mvIsMinPoint(CvPoint pt);

	//计算点所在等势线的长度
	int mvComputeAltLen(CvPoint pt);

	//得到前景、边界的点集和初始等势图
	void mvGetFirstAltImg2(IplImage *pFkImage, CvSeq *contour, int &nForeNum,
		CvPoint *m_ForePointSet, int &nBoundptNum, CvPoint *m_BoudaryPointSet);

	//得到等势图
	void mvGetAltitudeImg2(IplImage *pRgbAltitudeImage, int nBoundptNum,
		CvPoint *m_BoudaryPointSet);
}DeleteShadower;


//阴影和灯光检测器
class CShadowDetector  
{
public:
	CShadowDetector( void );
	~CShadowDetector( void );

	//初始化和释放
	bool mvInitShadowLightDetector( CvSize fkSz );
	bool mvUninitShadowLightDetector( );

	void mvInitShadowLightDetectorVar( );

	//在像素层来进行阴影检测
	void mvDetectShadowOnPixelLevel(
			double dTsNow,
			IplImage *pGrayImg, 
			IplImage *pBgImg, 
			IplImage *pRgbBgImg, 
			IplImage *pRgbImg, 
			IplImage *pFk2VImg, 
			IplImage *pFkBefProImg,
			const IplImage *pStopAFkImg,
			const IplImage *pMoveAFkImg,
			IllumRgbVarSet illumRgbVarSet
		);

	//判断给定区域是否为阴影
	bool mvIsStopShadowArea(
			CvSize useSz,
			IplImage *pStopAreaFkImg,
			CvPoint ltPt,
			CvPoint rbPt,
			double dTsNow,
			int nModel 
		);
	//得到当前帧的阴影信息
	void mvGetCurrentShowderMsg(
			IplImage *pRgbImage,
			CvRect objRect,
			IplImage *pCarMaxSizeXImg, 
			IplImage *pCarMaxSizeYImg,
			IplImage *ShadowHardImg,
			float &fTopRate,
			float &fRightRate,
			float &fBottomRate,
			float &fLeftRate,
			CvPoint &pCenter,
			double t_now
		);
	////将阴影信息存入循环结构体
	void mvStoreInfoInCR(
			float fTopRate,
			float fRightRate,
			float fBottomRate,
			float fLeftRate,
			CvPoint pCenter, 
			double t_now
		);
	//得到统计阴影信息
	bool mvGetStatShadowMsg(
			double t_now,
			float &fTopLeftRate,
			float &fTopRightRate,
			float &fBottomLeftRate,
			float &fBottomRightRate
		);
	//将统计阴影信息存入到文本和内存中
	void mvSaveStatShadowInfo(
			double t_now, 
			float fStatTopLeftRate,
			float fStatTopRightRate,
			float fStatBottomLeftRate,
			float fStatBottomRightRate
		);

	//在候选阴影中进一步检测阴影
	void mvDetectInCandiShadow(
			CvSet *vehicle_result_set,
			int vehicle_count,
			IplImage *pRgbImage,
			IplImage *pShadowHardImg,
			IplImage *pCarMaxSizeXImg,
			IplImage *pCarMaxSizeYImg,
			float fStatTopLeftRate,
			float fStatTopRightRate,
			float fStatBottomLeftRate,
			float fStatBottomRightRate
		);

private:
	//利用颜色来检测阴影
	void mvDetectCandiateShadow( 
			IplImage *pRgbBgImg,
			IplImage *pRgbImg,
			IplImage *pFk2VImg, 
			IllumRgbVarSet *pIllumRgbVarSet, 
			IplImage *pShadowImg 
		);


	//利用《融合颜色和梯度特征的运动阴影消除方法》来检测阴影
	void mvMSECG4Shadow( 
			IplImage *pGrayImg,
			IplImage *pBgImg,   
			IplImage *pRgbBgImg,
			IplImage *pRgbImg,
			IplImage *pSDNMImg,
			IplImage *pFk2VImg,
			IplImage *pMegerFkImg
		);	

	//对前景模板和背景模板求梯度差来检测阴影
	void mvSEBG4Shadow( 
			IplImage *pGrayImg,
			IplImage *pBgImg, 
			IplImage *pFk2VImg, 
			IplImage* pDiffGrad
		);

	//对SDNM, SEBG进行融合来得到阴影结果
	void mvMSECG(
			IplImage *pSDNMImg,
			IplImage *pSEBGImg, 
			IplImage *pTempImg, 
			IplImage* pFkImg
		);

	//去除外轮廓边缘点
	void mvRemoveExtContoursEdges( 
			IplImage *pGrayImg,
			IplImage *pFk2VImg, //灰度图和前景图
			IplImage* pInnerSobelImg //去除外轮廓边缘点后的边缘点图
		);

	//计算NCC来获取阴影
	void mvCalNCC(
			IplImage *pFk2VImg, 
			IplImage *pBgGrayImg, 
			IplImage *pGrayImg 
		);

	//统计得到阴影的概率
	void mvGetStatShadowRate(
		vector<int> vecSearchNo,
		ObjShadowInfo mObjShadowInfo[],
		float &fStatTopLeftRate,
		float &fStatTopRightRate,
		float &fStatBottomLeftRate,
		float &fStatBottomRightRate
	);
	//得到统计概率
	float mvGetStatRate(
			vector<int> vecSearchNo,
			ObjShadowInfo mObjShadowInfo[], 
			int nNum, 
			int no
		);
	//保存阴影信息
	void mvSaveShadowInfo(
			DirectionReslut myTempReslut,
			double t_now
		);
	
public:
	bool m_bInitShadowLightDet;  //是否初始化
	IplImage *m_pShadowEasyImg;	 //阴影(容易模式)图像
	IplImage *m_pShadowHardImg;	 //阴影(困难模式)图像
	ObjShadowInfo  m_ObjShadowInfo[MAX_SHADOW_INFO_CNT];//保存的前1000个阴影信息
	CycleReplace   m_ShadowInfoStoreCR;//保存阴影信息循环结构体
	DirectionReslut m_directionReslut[MAX_SAVE_DIRECTION_CNT];//内存中保存的阴影方向
	CycleReplace   m_SaveDirectionReslutCR;//统计阴影方向的循环结构体
	AnDateStruct m_myNowDT;	//当前时间结构体
	DirectionReslut m_myTempReslut;//临时（上一次）时间结构体

private:
	CvSize  m_szFk;			     //前景图的大小
	IplImage *m_pFkImg;			 //前景图像
	IplImage *m_pFkBefPImg;      //图像处理前的前景图
	IplImage *m_pInteShadowImg;	 //阴影积分图像
	IplImage *m_pStopAreaShadowImg;  //停止区域的阴影
	IplImage *m_pHBgImg, *m_pSBgImg, *m_pVBgImg; //背景图的h,s,v分量
	IplImage *m_pHImg, *m_pSImg, *m_pVImg; //当前图的h,s,v分量
	ShadowJudgeResultStru m_shadowJRS;  //阴影判断结果存储器
};


#endif
