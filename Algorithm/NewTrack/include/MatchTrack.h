#ifndef __MATCHTRACK_H
#define __MATCHTRACK_H

#include "TrackProperty.h"
#include "TrackConnector.h"

using namespace std;
using namespace MV_IMGPRO_UTILITY;
using namespace MV_AN_ANLGORITHM;
using namespace MV_SMALL_FUNCTION;



//点匹配的中间轨迹结构
typedef struct StrutPtMatchMidLevelInfo
{
	bool m_bVaild;   //该节点是否有效

	CvPoint m_ptNow;        //轨迹的当前点
	uchar   m_ucASiftDsc[SIFT_DIMS];   //轨迹的sift特征

	CvPoint m_ptMinLt;    //轨迹运动的最左上点
	CvPoint m_ptMaxRb;    //轨迹运动的最右下点	

	double m_dTsAdd;      //轨迹的更新时间戳
	double m_dTsUpdate;   //轨迹的添加时间戳

	int m_nUpdatTime; //该节点共更新多少次

	StrutPtMatchMidLevelInfo( )
	{
		m_bVaild = false;
	}
}PtMatchMidLevelInfo;


//点匹配的轨迹匹配中间层
//保证长轨迹(位移长或历史长)基本不会匹配不上
typedef struct StrutPtMatchMidLevel
{
public:
	StrutPtMatchMidLevel( );
	~StrutPtMatchMidLevel( );

	void mvInitVar( );

	void mvInitPtMatchMidLevel( const CvSize &szImg );
	void mvUninitPtMatchMidLevel( );

	//对点进行匹配和存储
	void mvPtMatchAndSave(	
			double dTsNow,              //当前的时间戳
			int nPtCnt,					//给定的特征点个数
			CvPoint2D32f *pPts,			//给定的特征点位置
			uchar **ucSift,				//给定的特征点的sift特征值
			int nNoNullTrNum,				//非空轨迹数目
			int *nANoNullTrIdx,			//非空轨迹所对应的序号
			MyTrackElem **pATrPointer     //轨迹对应的地址(指针)
		);

public:
	IplImage *m_pHMvNStImg;  //历史上存在运动/现在静止的轨迹图像


private:
	//添加或更新一个元素
	int mvAddOrUpdateOneElem(	
			double dTsNow,   //当前的时间戳
			CvPoint ptNow,	 //给定的特征点位置
			CvPoint ptTrMvLt, //给定的特征点轨迹运动的最左上点
			CvPoint ptTrMvRb, //给定的特征点轨迹运动的最右下点
			uchar ucASift[],  //给定的特征点的sift特征值
			int nMaxMatchPtCnt,  //匹配点的最大值
			const CvPoint *ptAMatchMv, //匹配位置(相对给定点)
			bool *bAMatched,     //该点位置是否匹配上
			int nTrAddMinMvDistTh = 5,   //轨迹添加时最小的位移距离要求
			int nMaxSaveCnt4OnePt = 25   //最多只对周围多少个点进行影响
		);

	//删除长时间未更新的元素
	int mvDeleteLongTimeNoUpdateElem(	
			double dTsNow, 
			double dUpdateTimeTh 
		);

	//获取历史上存在运动/现在静止的轨迹图像
	IplImage* mvGetHistoryMoveNowStopTrImg( );

private:
	IplImage *m_pPtLoc2TrIdxImg;  //该点位置所对应轨迹序号图

	vector<int> m_vectNullIdx;
	vector<int> m_vectUsedIdx;
	PtMatchMidLevelInfo *m_pPtMatchMLInfo;

}PtMatchMidLevel;

//点匹配跟踪器结构体
typedef struct AN_POINT_MATCH_TRACK
{
public:
	AN_POINT_MATCH_TRACK()
	{
		initVar();
	}	

	//初始化变量
	void initVar( );

	//初始化点匹配跟踪器
	void mvInitPointMatchTracker( AnGlobalSet *pGlobalSet );
	
	//释放点匹配跟踪器
	void mvUninitPointMatchTracker( );

	//点的匹配跟踪
	void mvPointMatchTrack(	int nPtCnt,  //角点数目
			CvPoint2D32f *pPts, //角点位置
		    uchar **ucSiftDesc, //角点sift描述符
		    double dTsNow,		//当前时间戳
			AreaPtTrInfo &struAreaPtTrInfo, //区域点轨迹信息
			StruPtTrProperty &struPtTrProperty,  //点轨迹属性判断
			PtsNear2FarSort &struTrPtsN2FResult, //轨迹点由近到远排序后的结果
			bool bEasyMatchMode	 //IN:是否采用简单匹配模式
		);

	//获取得到点匹配跟踪器结构体中轨迹集的指针
	CvSet * mvGetTrackSetPointer( ) { 
			return m_pPtTrackSet;
		}

	//利用kalman来对轨迹进行点预测
	bool mvPredictPtWithKalman(
			CvPoint2D32f &fptPred,	//得到的预测点
			const MyTrack &tr,		//给定轨迹
			int nPredNextCnt,		//预测下几个点
			IplImage *pShowImg=NULL	//显示图像
		);

	//将断裂的轨迹连接起来
	void mvConnectTrack( MvM2STrConnector &M2STrConnector );

	//设置轨迹当前的前景标识值
	void mvSetTrackFkLabel( IplImage *pFkLabelImg );

public:
	int m_nNoNullTrNum;    //非空轨迹数目
	int *m_nANoNullTrIdx;  //非空轨迹所对应的序号

	MyTrackElem **m_pATrPointer;   //轨迹对应的地址(指针)

	vector<int> m_vectNoMatchPtIdx;    //未匹配上的点序号
	vector<int> m_vectLongTrackPtIdx;  //长轨迹上的点序号

	PtMatchMidLevel m_ptMatchMidLevel;  //点匹配中间层

#ifdef TEST_SIFT_THROD
	Matchtrackpoint matchPoint;    //匹配上的点的属性
	vector<Matchtrackpoint> matchPointSet;  //匹配上点得集合
#endif

private:
	//轨迹匹配前的预先处理
	void mvTrackMatchPreProcess( bool bFirst );

	//获取得到轨迹集合中的非空轨迹
	void mvGetNonullTrack( bool bAll = true );

	//初始化轨迹内信息
	void mvNewTrackInit( MyTrackElem &trElem, double dTsNow );

	//对轨迹的当前帧信息进行初始化
	void mvInitTrackCurFrameInfo( );

	//对轨迹上的节点进行移动
	void mvShiftTrackNodes( );

	//把角点存为轨迹
	void mvSavePtsAsTrack( int nPtCnt,  //角点数目
			CvPoint2D32f *pPts, //角点位置
			uchar **ucSiftDesc, //角点sift描述符
			double dTsNow       //当前时间戳
		);

	//将轨迹加入到轨迹集合中
	int mvAddTrackToTrackSet( MyTrackElem &trElem );

	//估计当前帧的轨迹坐标及匹配窗口
	void mvEstimateTrImgPtLocAndMatchWin(
			CvPoint2D32f *ptAPredict,  //预测点位置
			CvRect *rectAMatch,		//匹配框
			bool *bAHadGoodMatchWin //是否存在好的匹配窗口
		);

	//根据最小二乘法预测轨迹的图像坐标
	bool mvGetPredictPtOfTrack(
			MyTrack& track,		//当前轨迹 
			double tsdiff,		//当前帧与上一帧的时间差
			CvPoint2D32f &ptPredict //预测点			
		);

	//获取得到预测点在本帧的匹配区域
	CvRect mvGetMatchRect(
			bool bCanPredict,        //是否能预测
			CvPoint2D32f ptPredict,  //预测点
			double tsdiff			 //上帧和本帧的时间间隔
		);

	//预先进行轨迹匹配
	void mvPtSiftMatch( 
			vector<trMatchStru> &vctMatchResult, //OUT:匹配结果
			bool *bHadGoodMatchWin,   //IN:是否存在较好的匹配窗口
			CvRect *rectAPtMatch,     //IN:点的匹配窗口
			bool bMatchAllPt,		  //IN:是否对所有点进行匹配
			bool bEasyMatchMode=false //IN:是否采用简单匹配模式
		);

	//每帧对轨迹和角点进行sift匹配,并判断轨迹是否删除
	void mvFiltrateMatchResult(
			vector<trMatchStru> &vctMatchResult, //匹配结果
			int nPtCnt, //角点数目
			int* nAPtIdx2TrIdx //角点所对应的轨迹序号
		);

	//将角点加入到轨迹中
	void mvAddKeyPtsToTracks(
			int nPtCnt,			  //角点数目
			CvPoint2D32f *ptALoc, //角点坐标
			uchar **pSiftDesc,	  //角点sift特征描述符
			int* nAPtIdx2TrIdx	  //角点所对应的轨迹序号
		);

	//获取本帧角点的匹配情况(即那些匹配上了,那些和较长轨迹匹配上了)
	void mvGetKeyPtsMatchedInfo(
			int nPtCnt,			  //角点数目
			CvPoint2D32f *ptALoc, //角点坐标
			int* nAPtIdx2TrIdx,   //角点所对应的轨迹序号
			int nLongTrackThres,  //长轨迹的历史长度要求阈值
			vector<int> &vectNoMatchPtIdx,   //未匹配上的点序号
			vector<int> &vectLongTrackPtIdx  //长轨迹上的点序号
		);

	//对轨迹和角点匹配后,对轨迹进行处理
	void mvProcessTracksAfterPtMatch( 
			CvPoint2D32f *ptAPreidct //轨迹本帧的预测点
		);
	
	//判断和删除较差的轨迹(较多帧或较长时间匹配不上)
	int mvJudgeDeleteBadTracks( 
			int nTrIdx,				 //轨迹序号
			MyTrackElem *pTrackElem, //轨迹元素指针
			IplImage *pRoadMask = NULL  //道路mask
		);

	//从轨迹集合中删除轨迹
	void mvDeleteTrack(	int nTrIdx,	MyTrackElem *pTrackElem );
	

	//获取点，匹配和轨迹信息存入到AreaPtTrInfo结构中
	void mvGetPointMatchTrackInfo(
			AreaPtTrInfo &struAreaPtTrInfo,  //区域内的点轨迹信息结构
			vector<int> &vectNoMatchPtIdx,   //未匹配上轨迹的点序号
			vector<int> &vectLongTrackPtIdx, //匹配到长轨迹上的点序号
			IplImage *pHMvNStImg  //历史上存在运动/现在静止的轨迹图像
		);

	//显示轨迹
	void mvShowTracks(char *cWinName, int nShowMod, 
		  int nKeyPtCnt, CvPoint2D32f  *ptKeyptLoc);

	//显示点的匹配情况
	void mvShowPtsMatchInfo(char *cWinName,
		  int nKeyPtCnt, CvPoint2D32f  *ptKeyptLoc, //当前的角点
		  vector<int> &vectNoMatchPtIdx,   //未匹配上轨迹的点序号
		  vector<int> &vectLongTrackPtIdx  //匹配到长轨迹上的点序号
		);
private:
	bool  m_bFrist;        //是否为第一帧
	CvSet *m_pPtTrackSet;  //轨迹集合
	CvMemStorage *m_pMemStorage;  //开辟的内存

	AnGlobalSet  *m_pGlobalSet;  //指向全局集

	int m_nTrackId;   //轨迹的Id号

private:
	//角点信息(由外部传入)
	int    m_nKeyPtCnt;
	CvPoint2D32f  *m_p_ptKeyptLoc;
	uchar  **m_pp_chSiftDesc;
	//-----------------------

	double m_dTsNow;  //当前的时间戳
	double m_dTsLast; //上一帧的时间戳

}StruPtMatchTracker;


//点的简单SIFT匹配应用
typedef struct StruPtSimpleSIFTMatchApp
{
	//一对多个点进行SIFT匹配,返回是否匹配成功
	static bool mvSiftMatchOne2Much( 
			const CvPoint &ptGive,      //给定的点
			uchar *giveSiftFeat,		//给定的sift特征
			const int nComPtNum,        //进行比较的点数目
			siftFeat *pComPtSiftFeat,	//进行比较的所有点sift特征
			const CvPoint &ptMatchWH,	//匹配的空间距离要求
			const int nSiftMatchDistTh  //设定的SIFT匹配阈值
		);

	//一对多个点进行SIFT匹配,返回是否匹配成功
	static bool mvSiftMatchOne2Much( 
			siftFeat &givePtSiftFeat,   //给定的点sift特征
			const int nComPtNum,        //进行比较的点数目
			siftFeat *pComPtSiftFeat,   //进行比较的所有点sift特征
			const CvPoint &ptMatchWH,	//匹配的空间距离要求
			const int nSiftMatchDistTh  //设定的SIFT匹配阈值
		);
}PtSimSIFTMatchApp;





#endif