//判断是否为同一目标的检查器
#ifndef __TRACK_CONNECTOR_H
#define __TRACK_CONNECTOR_H

#include "libHeader.h"

#include "sift_descr.h"
#include "MvSiftDescr.h"
#include "MvKeyPtExtract.h"

#include "BaseStruct.h"

#include "comHeader.h"  //放最后

#ifndef MAX_M2S_PT_CNT
	#define MAX_M2S_PT_CNT 2000  //最大的由动到静的点数
#endif

enum EnumMove2Stop
{ 
	SURE_MOVE2STOP_MODEL = 0,
	RECENT_IS_STOP_MODEL,
};

////由动到静的轨迹角点
//typedef struct StructPointMove2Stop
//{
//	double  dTsNow;
//
//	int     nPtCnt;
//	CvPoint ptAStop[100];
//
//}MvPtMove2Stop;


//由动到静的轨迹角点
typedef struct StructPointMove2Stop
{
public:
	StructPointMove2Stop( );
	void mvInitVar( );

public:
	bool   bVaild;   //是否有效 

	double dTsStartStop;   //开始的停止时间戳
	double dTsUpdateStop;  //更新的停止时间戳

	int      nSaveCnt;      //该角点保存的次数
	siftFeat pSiftFeat[3];  //该角点的SIFT特征

}MvPtMove2Stop;


typedef struct StruMove2StopTrackConnector
{
public:
	StruMove2StopTrackConnector( );
	~StruMove2StopTrackConnector( );

	void mvInitMove2StopTrackConnector( CvSize szImg );
	void mvUninitMove2StopTrackConnector( );

	//清理那些长时间没有更新的，获取得到目前为空&由动到静的点
	void mvClearAndGetEmpty( double dTsNow, double dLongTimeTh );

	//添加或更新
	bool mvAddOrUpdate( double dTsNow, EnumMove2Stop nModel,        
				  CvPoint2D32f &fpt, uchar cSift[SIFT_DIMS] );

	//显示由动到静的点
	void mvShowMove2StopPts( );


	//获取在给定点附近内所保存角点的序号
	bool mvGetIndexOfSavePts( 
			CvPoint2D32f &fpt,   //当前的角点位置
			int &nFindCnt,		 //寻找到的点个数
			int nAFindIdx[]		 //寻找到的点序号
		  );

	 //获取得到目前为空的由动到静的点
	 void mvGetMove2StopPt(
			int &nPtCnt,     //关键点的数目
			int nAPtIdx[],   //关键点的序号
			CvPoint ptALoc[] //关键点的坐标
		  );

private:
	void mvInitVar( );

	//获取得到目前为空的由动到静的点
	void mvGetEmptyMove2StopPt(	);

	//清理一个点的记录
	void mvClearOnePointRecord( MvPtMove2Stop &ptMove2Stop );


	//添加新的节点
	bool mvAddM2SPtToSaveResult( 
			double dTsNow,         //当前的时间戳
			CvPoint &ptNow,        //当前的角点位置
			uchar cSift[SIFT_DIMS] //当前角点的SIFT特征
		);


	//获取得到最好的匹配结果
	bool mvGetBestSiftMatchResult( 
			int  &nBestPtIdx,        //匹配得到的最好点序号
			int  &nBestSaveIdx,      //匹配得到的最好点的第几节点号
			uchar cSift[SIFT_DIMS],  //当前角点的SIFT特征
			int  nFindCnt,           //寻找到的点数
			int  nAFindIdx[]         //寻找到的点序号
		);

	//根据匹配结果来进行更新
	bool mvUpdateWithMatchResult( 
			double  dTsNow,          //当前的时间戳
			CvPoint ptNow,           //当前角点位置
			int  &nBestPtIdx,        //匹配得到的最好点序号
			int  &nBestSaveIdx,      //匹配得到的最好点的第几节点号
			uchar cSift[SIFT_DIMS]   //当前角点的SIFT特征
		);

private:
	bool    m_bInit;    //是否进行过初始化

	MvPtMove2Stop *m_pPtMove2Stop;      //由运动到静止的点
	IplImage      *m_pMove2StopPtIdImg;	//由动到静的点的Id图像	

	int		m_nEmptyCnt;				   //空点个数
	int		m_nAEmptyIdx[MAX_M2S_PT_CNT];  //空点序号
	int     m_nFillIdx4EmptyArray;     //当前填充的点序号(相对于空数组)

}MvM2STrConnector;

////////////////////////////////////////////
#endif