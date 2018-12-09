//停车检测
#ifndef _MV_GRIDSTOP_DETECTOR_H_
#define _MV_GRIDSTOP_DETECTOR_H_

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "libHeader.h"

#include "sift_descr.h"
#include "MvSiftDescr.h"

#include "MvKeyPtExtract.h"
#include "MatchTrack.h"

#include "Interface4Server.h"  //与服务端的接口

#include "struRoadDet.h" //智能交通的结构体-用于代码管理和简化
#include "MvReadDebugCfg.h"  //调试配置器指针

#include "comHeader.h"   //放最后

using namespace std;
using namespace MV_MATH_UTILITY;
using namespace MV_AN_ANLGORITHM;
using namespace MV_IMGPRO_UTILITY;

#ifndef AN_SIFT_MINSEC_RATIO_FOR_STOP
	#define AN_SIFT_MINSEC_RATIO_FOR_STOP 0.64
#endif
#ifndef AN_SIFT_DISTANCE_FOR_STOP
	#define AN_SIFT_DISTANCE_FOR_STOP 1.5e5
#endif

#ifndef MAX_SIFT_PTNUM_FOR_STOPAREA
	#define MAX_SIFT_PTNUM_FOR_STOPAREA 100
#endif

//短期周期的图像存储
typedef struct _sTermImgCache
{
	int		 nNowPointer;

	bool	 bAExist[SHORTTERMCNT];
	double	 dATsIn[SHORTTERMCNT];  	//存图的时间戳(s为单位)
	IplImage *pAImg[SHORTTERMCNT];	

	_sTermImgCache( )
	{
		nNowPointer = -1;

		int  i;
		for( i=0; i<SHORTTERMCNT; i++ )
		{
			bAExist[i] = false;
			dATsIn[i]  = -100000.0;
			pAImg[i] = NULL;
		}
	};
}STermImgCache;

//长期周期的图像存储
typedef struct _lTermImgCache
{
	int		 nNowPointer;

	bool	 bAExist[LONGTERMCNT];
	double	 dATsIn[LONGTERMCNT];  	//存图的时间戳(s为单位)
	IplImage *pAImg[LONGTERMCNT];	
	
	_lTermImgCache( )
	{
		nNowPointer = -1;

		int  i;
		for( i=0; i<LONGTERMCNT; i++ )
		{
			bAExist[i] = false;
			dATsIn[i]  = -100000.0;
			pAImg[i] = NULL;
		}
	};
}LTermImgCache;


//停止的信息结构
typedef struct _stopAreaStru
{
	int nStopEventId;

	double   tsAlertStop;
	double   tsUpdateStopPt;

	//ltPt,rbPt对停止点区域进行了扩展，以得到
	//       更多的sift特征来排除同一目标多报
	CvPoint  ltPt;  //区域左上角点位置
	CvPoint  rbPt;  //区域右上角点位置

	//resLtPt,resRbPt为对ltPt,rbPt进行前景约束得到区域
	CvPoint  resLtPt;  //区域左上角点位置
	CvPoint  resRbPt;  //区域右上角点位置

	int      nPtNum;
	siftFeat *pSiftFeat;

	_stopAreaStru( )
	{
		nStopEventId = -1;

		tsAlertStop = -100000.0;
		tsUpdateStopPt = -100000.0;
		ltPt = cvPoint(  10000,  10000 );
		rbPt = cvPoint( -10000, -10000 );
		resLtPt = cvPoint(  10000,  10000 );
		resRbPt = cvPoint( -10000, -10000 );
		nPtNum = 0;
	};
}StopAreaStru;

//停止区域是否为车辆结构
typedef struct _stopAreaIsCarStru
{
	double   tsJudge;   //判断的时间戳
	CvPoint  ltPt;      //区域左上角点位置
	CvPoint  rbPt;      //区域右上角点位置

	bool  bCar;
	_stopAreaIsCarStru( )
	{
		tsJudge = -100000.0;
		ltPt = cvPoint(  10000,  10000 );
		rbPt = cvPoint( -10000, -10000 );
		bCar = false;
	};
}StopAreaIsCarStru;

//候选停止的信息结构
typedef struct _candidateStopAreaStru
{
	int      nStopEventId;  //停止事件的ID

	CvPoint  ltPt;          //区域左上角点位置
	CvPoint  rbPt;          //区域右上角点位置

	int      nPtNum;        //角点个数
	int      nAPtIdx[MAX_SIFT_PTNUM_FOR_STOPAREA];   //角点序号
	siftFeat siftFeatA[MAX_SIFT_PTNUM_FOR_STOPAREA]; //角点特征

	_candidateStopAreaStru( )
	{
		nStopEventId = -1;

		ltPt = cvPoint(  10000,  10000 );
		rbPt = cvPoint( -10000, -10000 );
		nPtNum = 0;
	};
}CandidateStopArea;


//---------------------------------//
class CGridStopDetector
{
public:
	CGridStopDetector(void);
	~CGridStopDetector(void);

	void mvInitGridStopDetector(
			CvRect roadRect,
			IplImage *pRoadMask,
			IplImage *pSkipImg,
			IplImage *pCarWImg, 
			IplImage *pCarHImg,
			float fCameraHeight,
			bool  bRoadFar,
			KeyptsStore *pStruKeyptsStore );

	//设置点的匹配范围
	void mvSetPtMatchRange( 
		   const CvPoint &ptMatchRange );

	void mvUninitGridStopDetector( );

	//获取得到文件检测配置器的指针
	void mvGetFileDetCfgPointer(
		   CDetectConfiger *pDetConfigerByFile )
	{
		m_p_cDetConfigerByFile = pDetConfigerByFile;
	}

	//初始化停车检测区域
	void mvGetStopArea(
			int nStopAreaCnt, 
			int nAStopRgnPtCnt[20], 
			CvPoint2D32f ptAStopRgnPt[20][20] );

	//得到各停车检测区域的rect
	void mvGetStopAreaRect( vector<CvRect> &vecStopRect );

	//初始化一些网格停车法的变量
	void mvInitGridStopVar( 
			CvSize szImg,
			bool bQuickBuildBg, 
			int  nChanCnt, 
			bool bAStop[], 
			double dAStopIgnJam[],
			double dAStopIgnAlert[],
			CfgInterface4Server *pInterface4Server,
			CvPoint ptMatchRange = cvPoint(-1,-1) );

	//获取当前的一些状态变量值
	void mvGetNowStatus(
			int nListImgNo,
			int64 nIndex,
			double tsNow, 
			bool bDay, 
			bool bDebug,
			bool bBgVaild );

	//获取一些额外的附加信息
	void mvGetAdditionalInfo( 
			IllumResSaveStru *_pIllumRes,     //光照结果存储器的指针
			MvDebugInfoList *_pDebugInfoList, //调试信息存储器的指针 
			MvDebugCfgRead *_pDebugCfgReader  //调试配置器的指针
		  ) 
	{
		m_pIllumRes = _pIllumRes;
		m_pDebugInfoList = _pDebugInfoList;  	
		m_p_DebugCfgRead = _pDebugCfgReader;		
	}

	//获取停止时间要求
	double mvGetStopTimeRequire( )
	{
		return m_dStopTimeRequire;
	}
		
	
	//保存各网格区域的特征
	void mvSaveGridAreaFeatForSTerm( 
			int nPtCnt,
			CvPoint2D32f *ptA,
			uchar **cSiftVal,
			IplImage *lpGrayImg, 
			IplImage *lpGrayInteGrateImg,
			IplImage *lpDiffInteGrateImg, 
			IplImage *lpFkInteGrateImg,
			IplImage *lpInteGrateGradImg,
			IplImage *lpInteGrateBgGradImg,
			IplImage *lpInteDiffGradImg, 
			IplImage *lpHDiffSobelInteGrateImg );

	void mvSaveGridAreaFeatForLTerm( 
			int nPtCnt,
			CvPoint2D32f *ptA, 
			uchar **cSiftVal,
			IplImage *lpGrayImg,
			IplImage *lpGrayInteGrateImg, 
			IplImage *lpDiffInteGrateImg,
			IplImage *lpFkInteGrateImg,
			IplImage *lpInteGrateGradImg, 
			IplImage *lpInteGrateBgGradImg,
			IplImage *lpInteDiffGradImg,			
			IplImage *lpHDiffSobelInteGrateImg );

	//计算各网格当前与以往周期相似的次数
	bool mvCalcSimiTimeWithBeforeOfGrids( );

	//保存调试信息
	bool  mvSaveDebugInfo( char *cInfo, IplImage *pImg=NULL );

	//获取得到可能为停止的网格和角点
	bool mvGetMayStopGridPoint(
			vector<int> &vecMayStopGridNo, //OUT：最近可能为停止的网格的序号	
			vector<CvPoint> &vctStopPt,    //OUT：认为是停止的点的坐标
			vector<int> &vctStopPtNoInST,  //OUT：短周期中停止点的序号
			bool bDay = true               //是否为白天
		);
			
	//设置目标的停车时间图像
	void  mvSetStopTsImgOfObjTrack(	
			const vector<StruObjStopAreaInfo> &vOSAreaInfo //停止目标信息
		);

	//获取给定区域的的停车时间图像信息
	void  mvGetStopTsImgInfo(	
			vector<double> &vProb,		//out:出现的概率
			vector<double> &vPassTime,  //out:距今已过时间
			const CvRect &rectArea,     //in:停止目标区域
			double dTsNow               //in:当前时间戳
		);

	//获取得到可能为停止的网格和角点(在不需存图时)
	bool mvGetMayStopGridPointWithoutSaveImg(	
			vector<int> &vctInSTStopPtNo,	//短周期中停止点的序号
			vector<CvPoint> &vctStopPtLoc,  //认为是停止的点的坐标
			const vector< vector<int> > &vctSTMatchPtNo, //匹配上的角点序号
			int nMatchRadius                //匹配半径
		);

	//获取得到可能为停止的网格和角点(需存多张远景图时)
	bool mvGetMayStopGridPointWithSaveRemoteImg(	
			vector<int> &vctInSTStopPtNo,	//OUT:短周期中停止点的序号
			vector<CvPoint> &vctStopPtLoc,  //OUT:认为是停止的点的坐标
			const vector< vector<int> > &vctSTMatchPtNo, //IN:匹配上的角点序号
			int nMatchRadius   //IN:匹配半径
		);

	//获取得到可能为停止的网格和角点(需存多组远/近景组合图时)
	bool mvGetMayStopGridPointWithSaveRemoNearCombImg(	
			vector<int> &vctInSTStopPtNo,	//短周期中停止点的序号
			vector<CvPoint> &vctStopPtLoc,  //认为是停止的点的坐标
			const vector< vector<int> > &vctSTMatchPtNo, //匹配上的角点序号
			int nMatchRadius   //匹配半径
		);

	//利用网格的方法来检测停车
	void mvDetectNoAlertStopArea( 			
			vector<CandidateStopArea> &vecCandSA,  //输出：候选停止区域
			vector<int> &vctStopCarPtNo,	   //输入：停止点
			bool bSensitive,				//输入：是否采用敏感模式
			bool bJustDetUnnormalStop,      //输入：是否只检测异常停车
			bool  =true,    //输入：是否存在停止时间的要求
			double dGiveComparePreTime=-1000.0 //输入：与之前的多久进行比较
		);

	//利用给定的可能停止目标获取得到未报警的停车区域
	bool mvGetNoAlertStopAreaWithGiveObj(	
			vector<StruObjStopAreaInfo> &vectObjStopAreInfo, //停止目标信息
			vector<CandidateStopArea> &vecCandSA );  //输出：候选停止区域

	//获取采用网格停车法的检测结果
	bool mvGetStopCarDetectResult( 
			double dTsNow, 
			double dShowTime, 
			vector<GenSimDetectOut> &vectOut );

	//保存下停止区域的判断是否为车的结果信息到存储器中
	void mvAddIsCarRes2Cache( 
			CvPoint ptResLt,
			CvPoint ptResRb,
			bool bTrueCar );

	//根据多次的结果来判断给定区域是否为车
	bool mvIsCarWithMultiTimeResult(
			double  dPreComTime,		  //往前比较的时间
			CvPoint ptLt, CvPoint ptRb ); //给定区域的位置
	

	//保存下停止区域的信息到存储器中(便于后面比较)
	void mvAddStopAreaInfoToCache( 
			const CandidateStopArea &candSA,
			CvPoint ltResPt, CvPoint rbResPt );

	//判断该区域当前是否和给定时间内较为相似
	bool mvIsSimilarAreaWithBefore( 
			CandidateStopArea candSA, 
			CvSize szCar,
			double dTsStart, 
			double dTsEnd, 
			CvRect &rtROI, 
			IplImage *pNowGrayImg );


	//判断块是否相似
	bool mvIsBlockSimilarAreaWithBefore(
			IplImage *pNowGrayImg,
			double dTsStart,
			double dTsEnd );

	//获取各种周期的间隔
	void mvGetTermInterval(	double &dSTermTime,
			double &dLTermTime );


	//判断给定候选目标是否满足停车时间要求
	bool mvIsSuitStopTimeRequir( const CandidateStopArea &tempCandSA );

private:
	//初始化变量
	void InitGridStopDetectorVar(  );

	//创建目标的网格
	void  mvCreateObjectGrid(
				CvRect roadRect, 
				IplImage *pRoadMask,
				IplImage *pObjWImg,
				IplImage *pObjHImg,
				CvPoint  *ptAObjWh,
				int &nGridArea, 
				gridArea **ppGridArea,
				float fGridW2ObjW=0.6f, 
				float fGridH2ObjH=0.6f, 
				float fGridHStep2ObjH=0.25f, 
				float fGridWStep2ObjW=0.25f 
			);


	//获取得到最近没有变化的网格区域
	bool mvGetRecExistSimiGridArea( 
			vector<int> &vectRecSTNo1,
			vector<int> &vctNo );
	bool mvGetRecExistStopGridArea(
			vector<int> &vectRecSTNo2,
			vector<int> &vctNo );

	//获取得到在短周期中认为是最近的序号
	bool mvGetRecTermIdxOfST( 
			vector<int> &vectRecSTNo1,
			vector<int> &vectRecSTNo2 );

	//获取得到在长周期中认为是最近的序号
	bool mvGetOutOnTermIdxOfLT( 
		int  &nRecSTCnt1, int nARecSTNo1[],	
		int  &nRecSTCnt2, int nARecSTNo2[], 
		double dOutStopTimeReq = -1000.0, 
		double dOnStopTimeReq = -1000.0 );

	//获取得到在短周期中最近可能为停止的网格
	bool mvGetRecMayStopGrids( 
			vector<int> &vectRecSTNo1, 
			vector<int> &vectRecSTNo2,
			vector<int> &vecMayStopGridNo );

	//获取各网格在短周期中最近可能为停止的次数
	int mvGetRecentStopTime( 
			vector<int> &vGridIdx,			   //OUT:考察过的网格序号
			vector<int> &vecSuitModel1TermCnt, //OUT:满足模式1的次数
			vector<int> &vecSuitModel2TermCnt, //OUT:满足模式2的次数
			double dTimeRequire,         //IN:考察的时间
			vector<int> *pVectInGridIdx  //IN:输入的待考察网格序号
		);   
	
	//对网格的重要性进行排序
	bool mvSortGridsImportant( 
			vector<int> &vSortIdx,	
			vector<int> &vInIdx );

	//为减少计算量,先对符合条件的网格区域进行聚类
	bool mvGroupingGridArea(
			const vector<int> &vctIdx, 
			vector<CvPoint> &vctG_LtPt, 
			vector<CvPoint> &vctG_RbPt );

	//对给定时间和区域的短期sift特征来进行SIFT匹配
	void mvSiftMatchWithGiveTimeAreaInST( 
			vector<int>  &vectTermNo,	  //IN:对应的短周期序号
			vector<CvPoint> vctGroupLtPt, //IN:聚类后的群的左上点
			vector<CvPoint> vctGroupRbPt, //IN:聚类后的群的右下点
			vector<int> &vctInGroupPtCnt, //OUT:在该类中的角点数目
			vector<int> &vctInGroupPtNo,  //OUT:在该类中的角点序号
			vector< vector<int> > &vctMatchPtNo, //OUT:该类匹配上的角点序号
			vector<bool> &vctGroupMatchSucc,     //OUT:聚类群是否匹配成功
			int nMatchRadius = 2          //IN:匹配半径
		);

	//获取得到在指定区域内的点的序号
	bool mvGetInRecPtNo(
			int nPtCnt, 
			CvPoint ptA[], 
			CvPoint ltPt,
			CvPoint rbPt, 
			int &nInRectPtCnt,
			int nAInRectPtNo[] );

	//获取给定时间和区域的长期sift特征匹配失败的结果
	void mvGetLongTermSiftMatchFailResult(
			int nTermCnt, int nATermNo[],   //对应的长周期序号
			int nNowSTNo,			        //当前的短周期的指针序号
			vector< vector<int> > vctPtNo,  //各类的角点序号
			vector< vector<int> > &vctMatchFailPtNo, //各类匹配失败的角点序号
			int nMatchRadius = 3 ); 

	//获取得到非重复的所有点
	bool mvGetAllNoRepeatedPt( int nNowSTermNo, 
			const vector< vector<int> > &vctPtNo,
			vector<int> &vctInSTPtNo,
			vector<CvPoint> &vctPtLoc );

	//获取给定时间和区域的长期sift特征匹配失败的结果
	void mvGetSiftMatchSuccResult( 
			int nNowSTermNo, 		 //当前短期周期半径
			int nTermCnt, int nATermNo[],  //长期周期
			vector<int> &vctMatchPtNo,   //匹配上的点序号
			vector<CvPoint> &vctMatchPt, //匹配上的点位置
			int nMatchRadius = 2	     //匹配半径
		);

	//得到在停车区域内的停止点和满足停车时间要求的停止点
	bool mvGetSuitCarStopAlertPt(
			vector<int> &vctStopPtNo_InSA,    //OUT:停车区域内的停止的角点序号
			vector<CvPoint> &vecGroupExtLtPt, //OUT:满足要求的聚类后的rect左上点
			vector<CvPoint> &vecGroupExtRbPt, //OUT:满足要求的聚类后的rect右下点
			vector< vector<int> > &vctStopPtNo_InRect, //OUT:对应于rect内的停止点
			vector< vector<int> > &vctPtNo_InExtRect,  //OUT:对应于扩展rect内的角点
			vector<int> &vctStopPtNo,			//IN:停止点       
			bool bHaveStopTimeRequire = true,   //IN:是否存在停止时间的要求
			double dGiveComparePreTime=-1000.0  //IN:与之前的多久进行比较
		);

	//获取给定时间和区域的长期sift特征匹配成功的点
	void mvIsMatchWithGiveLongTerm( 
			bool bAMatch[],					//各点是否匹配上，输出
			int  nPtCnt, siftFeat siftFeatPtA[], //各欲匹配点的sift特征
			int  nTermCnt, int nATermIdx[],	//给定的各长期周期序号
			int  nMatchR					//匹配半径
		);

	//得到给定角点在停车区域内的那部分角点的序号
	bool mvGetPtIdx_InStopArea(
		   vector<int> &vctInStopAreaPtNo,  //输出：在停车区域内的那部分角点的序号
		   int nSTermIdx = -1,              //输入：要得到的角点所对应的短周期序号
		   vector<int> *p_vctStopPtNo = NULL );  //输入：给定的角点序号

	//获取给定时间和区域的长期sift特征匹配成功的点
	//要求：1.与停车时间范围附近的长周期相比较(匹配上一次即可)	
	//      2.与停车时间内的长周期相比较(匹配上比率要达到一定的比率)
	void mvGetMatchPtInCloseInStopTime( 
			int nPtCnt, siftFeat siftFeatPtA[],	    //sift点
			int nCloseTermCnt, int nACloseTermNo[], //停车时间范围附近
			int nInTermCnt, int nAInTermNo[],       //停车时间内
			bool bAMatch[] );                       //匹配结果	

	//得到满足停车时间要求的停止点
	bool mvGetSuitTimeRequrePoint(
			vector<int> &vctSuitStopPtNo,   //满足时间要求的点的序号
			vector<int> &vctStopPtNo        //停止点的序号
		 );	

	//获取给定时间和区域的长期sift特征匹配成功的点
	bool mvGetMatchPoint4GiveTimeInLongTerm(
			vector<int> &vctSuitStopPtNo,	//输出
			vector<int> &vctStopPtNo, 		//输入
			double dTsNow,                  //当前时间戳
			double dGivePreTime,            //与多长时间前的比较
			int  nMatchRadius   
		);


	//判断是给定目标区域所在的车道中否存在很多的可能为停车目标
	bool mvExistManyStopObject( 
			CvPoint ptLt, CvPoint ptRb,  //给定目标区域
			float fWThres = 1.6f,    //为多辆车宽的阈值
			float fHThres = 1.6f );  //为多辆车高的阈值  

	//判断是否存在较多的可能停车
	bool mvExistManyMayStopGrids(
			int nStopAreaIdx,   //停车区域的index
			vector<int> *p_vecMayStopGridNo,  //可能为停车的网格的index
			vector<CvPoint> *p_vctStopPtLoc,  //可能为停止的点的坐标
			float fWThres = 1.6f,    //为多辆车宽的阈值
			float fHThres = 1.6f );  //为多辆车高的阈值

	//将停车点按目标大小先按车大小聚类得到rect,判断其rect是否满足停车
	bool mvGetSuitStopRequireAfterGrouping(
			vector<int> &vctStopPtNo_InSA,    //在停车区域内的停止点的角点序号
			vector<int> &vctPtNo_InSA,		  //在停车区域内的角点序号
			vector<CvPoint> &vecGroupExtLtPt, //满足要求的聚类后的rect左上点
			vector<CvPoint> &vecGroupExtRbPt, //满足要求的聚类后的rect右下点
			vector< vector<int> > &vctStopPtNo_InRect, //对应于rect内的停止点
			vector< vector<int> > &vctPtNo_InExtRect );//对应于扩展rect内的角点

	//对候选的停车角点进行聚类
	//no more usefulness, need delete
	bool mvGroupingStopPt(
			vector<CvPoint> vctPt, 
			vector<int> &vctPtGroupNo, 
			vector<CvPoint> &vctGroupingLtPt,
			vector<CvPoint> &vctGroupingRbPt );

	//获取给定索引和序号的点
	void mvGetPtOfGiveIdx( 
			bool bShortTerm, 
			int nTermIndex, 
			vector<int> vctPtNo, 
			vector<CvPoint> &vctPt );

	//获取给定索引和序号点的sift特征值
	void mvGetPtSiftFeatOfGiveIdx( 
			bool bShortTerm, 
			int nTermIndex, 
			vector<int> vctPtNo, 
			uchar **pSift );

	//得到在扩展矩形区域内的所有点
	void mvGetInExtendRectPt(
			vector<CvPoint> vctPt, 
			CvPoint ltPt, 
			CvPoint rbPt, 
			vector<CvPoint> &vctInAreaPt );

	//利用给定候选停车目标来获取得到可能为停止的网格和角点
	void  mvGetMayStopGridPtsWithGiveObjs(	
			vector<StruObjStopAreaInfo> &vectObjStopAreInfo, //停止目标信息
			vector<CvRect> &vctStopRects,	    //各停止目标所对应的group区域
			vector< vector<int> > &vectNowPtIdx //各类匹配成功的角点序号
		);

	//将给定区域与报警过为停止的区域进行匹配，更新停止的区域
	bool mvMatchUpateStopArea(			
			const double dSameObjAlertTime, //同一目标的报警间隔
			const CvPoint &stopAreaLtPt,    //给定停止区域的左上点
			const CvPoint &stopAreaRbPt,    //给定停止区域的右下点
			const vector<CvPoint> &vctPt,   //该区域的停止角点位置
			uchar **pSift 		    //该区域的停止角点SIFT特征
		);

	//判断该区域是否为报警过为停止的区域
	bool mvIsHadAlertStopArea( 
			const double dSameObjAlertTime, //同一目标的报警间隔
			const CvPoint &stopAreaLtPt,    //给定停止区域的左上点
			const CvPoint &stopAreaRbPt,    //给定停止区域的右下点
			const vector<CvPoint> &vctPt,   //该区域的停止角点位置
			uchar **pSift 		    //该区域的停止角点SIFT特征
		);

	//设置存图变量
	void mvSetPictureSaveVar(
			double dMinStopTime, double dMaxStopTime,
			StruSaveRemotePic	 &cfgSaveRemotePic,
			StruRemoteNearCombPic &cfgSaveRemoNearCombPic );

//#ifdef DEBUG_GRID
	//显示停车点检测
	void  mvShowStopPtJudge(
			vector< vector<int> > vctSMatchSPtNo, 
			vector< vector<int> > vctLMatchFPtNo, 
			vector<CvPoint> vctStopPt );

	void  mvShowStopPtJudge(
		vector<int> vctStopPtNo_InSA, 
		vector<int> vctPtNo_SuitStopTR, 
		vector< vector<int> > vctStopPtNo_InRect );
//#endif

private:
	//判断给定的区域和可能的停止网格是否重叠
	bool  mvIsOverlapWithMayStopGrids(
			vector<StruObjStopAreaInfo> &vectObjStopAreInfo, //停止目标信息
			vector<int> &vecMayStopGridNo,  //最近可能为停止的网格的序号
			vector<CvPoint> &vctGroupLtPt, 
			vector<CvPoint> &vctGroupRbPt );

private:
	//将停车检测器的设置回初始状态
	void mvSetStopDetectorToInitStatus( );



	//获取停车前和停车期间及停车时间内长期网络对应的序号
	void mvGetLTermNoForStop(
			int &nInStopLTCnt,
			int nAInStopLTNo[],
			int &nOnStopLTCnt,
			int nAOnStopLTNo[], 
			int &nOutStopLTCnt,
			int nAOutStopLTNo[],
			double dRecThres = -1.0 );

	//显示指定网格区域的特征
	void mvShowGridAreaFeatResult(
			IplImage *lpGrayImg, 
			int nShowNo );

	//保存停车检测的判断过程图像
	void  mvSaveStopJudgeShowImg( 
			IplImage *pShowImg,
			bool bSaveLTerm=false );

	//判断欲报的目标区域是否和停车前相同
	bool mvIsSameWithBefStopImg( CvRect objRect );

	//显示停车检测的判断过程
	void  mvShowStopJudgeArea(	int nStep, 
			vector<int> *p_vctGridNoSuit1 = NULL, 
			vector<CvPoint> *p_vctGroupLtPt = NULL,
			vector<CvPoint> *p_vctGroupRbPt = NULL,
			vector<int> *p_vctSTermMatchSGroup = NULL,
			vector<int> *p_vctLTermMatchFGroup = NULL );

public:
	CycleReplace    *m_pSTImgCR;    //短周期的循环覆盖存储体
	STermImgCache   m_cacheSTImg;   //短周期图像存储器

	CycleReplace    *m_pLTImgCR;    //长周期的循环覆盖存储体
	LTermImgCache   m_cacheLTImg;   //长周期图像存储器

	CycleReplace    m_stopAlertCR;  //停车报警存储的循环覆盖存储体
	StopAreaStru    *m_pStopArea;   //已报过停车报警的区域信息存储器

	CycleReplace      m_stopIsCarCR;  //停止区域是否为车的的循环覆盖存储体
	StopAreaIsCarStru *m_pStopIsCar;  //停止区域是否为车的信息存储器	

	vector<int>		m_vectMayStopGridNo;   //可能的停止网格
	vector<CvPoint> m_vectStopCarPt;      //可能的停止点坐标

private:
	IllumResSaveStru *m_pIllumRes;       //光照结果存储器的指针
	MvDebugInfoList  *m_pDebugInfoList;  //调试信息存储器的指针 

private:
	CDetectConfiger *m_p_cDetConfigerByFile;

	int     m_nImgListNo;
	int64   m_nIndex;
	double  m_tsNow;  //s为单位
	bool    m_bDay;
	bool    m_bSensitive;
	CvSize  m_imgSize;
	bool    m_bDebug;
	bool    m_bBgVaild;
	int     m_nSrcImgMod;
	bool    m_bQuickBuildBg;

	CvPoint m_ptMatchRange;

	float   m_fCameraHeight;
	bool    m_bRoadFar;
	
	CvRect  m_roadRect;
	int		m_nStopAreaCnt;
	int		m_nAStopRgnPtCnt[20];
	CvPoint2D32f m_ptAStopRgnPt[20][20];

private:
	IplImage *m_pCarWImg, *m_pCarHImg;
	IplImage *m_pRoadMask, *m_pSkipImg;

	IplImage  *m_pTemp1, *m_pTemp2;  //for sift match

	IplImage  *m_pStopTsImg;  //停止时间戳图像的

private:
	double m_dStopTimeRequire; 	//停车报警时间
	double m_dMaxStopTime;   	//停车检测的时间要求
	double m_dSTermInterval;  	//短周期的间隔
	double m_dLTermInterval;  	//长周期的间隔
	int    m_nSTermIndex;
	int    m_nLTermIndex;

	int	   m_nSaveRemotePicCnt;   //存多少张停车远景图
	int    m_nSaveRemoNearPicCnt; //存多少组停车远近景组合图

	double m_dAInStopTime[10];    //在停车时间内
	double m_dAOutStopTime[10];   //在停车时间外

private:
	PtExtAddFlag  *m_pPtExtAddFlag;  //角点添加标识
	bool	m_bNeedComThisFrame;

private:
	int		  m_nGridArea;
	gridArea  *m_pGridArea;
	vector<CvPoint>  m_vectPtsOfCarWh;

private:
	//SIFT特征
	int       *m_nASTSiftFeatCnt; //[SHORTTERMCNT];
	siftFeat  **m_pSTSiftFeat;

	int       *m_nALongTermSiftFeatCnt; //[LONGTERMCNT];
	siftFeat  **m_pLongTermSiftFeat;

	CycleReplace  *m_pBgTImgCR;   //背景周期的循环覆盖存储体
	int       *m_nAForBgTermSiftFeatCnt; //[FORBGTERMCNT];
	siftFeat  **m_pForBgTermSiftFeat;

	IplImage *m_pGrayImg;

//////#ifdef DEBUG_GRID
	double m_dRecShortThres;	
public:
	IplImage *m_pShowImg; 
	MvDebugCfgRead *m_p_DebugCfgRead;
//////#endif	
};


#endif
