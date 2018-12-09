//group形成和调整
#ifndef _MV_CONSTRUCT_GROUP_H_
#define _MV_CONSTRUCT_GROUP_H_

#include <vector>
#include <stdio.h>
#include <stdlib.h>

//#include <map>

#include "libHeader.h"

#include <algorithm>

#include "MvMathUtility.h" 

#include "MatchTrack.h"
#include "MvDistGraph.h"

#include "comHeader.h"  //放最后

using namespace std;

using namespace MV_MATH_UTILITY;
using namespace MV_AN_DISTGRAPH;

#ifndef PTTR_SEGMENT_MAP_INIT_VAL
	#define PTTR_SEGMENT_MAP_INIT_VAL -1  //点轨迹进行匹配的map初始值
#endif

#ifndef SEGMENT_THRESHOLD
	#define SEGMENT_THRESHOLD(size, c) (c/size)
#endif

//轨迹差异模型的显示
#ifndef SHOW_TRACKS_DIFFERENT_MOD
	enum{
		SHOW_MOVE_UNCONS = 1,	  //段轨迹间的运动不一致性
		SHOW_PTDIST_UNCONS,       //节点间距的不一致性
		SHOW_DIFF_VECTOR_VAL	  //轨迹差异向量的值 
	};
#endif



//轨迹中各小段的运动信息
typedef struct TRACK_LITTLESEG_MVINFO
{
	//x/y方向的运动是否有效 
	bool  bVaild; 

	//该轨迹x/y方向的绝对位移总和
	float fSum;

	//该段轨迹x/y方向的绝对位移
	float fADiff[N_REF];

	//该段轨迹x/y方向的绝对位移积分
	float fAInteDiff[N_REF];

	//该段轨迹x/y方向的绝对位移占总长度的比率
	float fARate[N_REF];

	//该段轨迹x/y方向的绝对位移比率的分布概率
	float fAPDF[N_REF];


	TRACK_LITTLESEG_MVINFO( )
	{
		bVaild = false;  	
		fSum = 0.0f;
	}

}StruTrLitSegMvInfo;

//点轨迹聚类结构器
typedef struct MV_PTTR_SEGMENT 
{
public:
	MV_PTTR_SEGMENT( )
	{
		initVar( );
	};

	//初始化变量
	void initVar( );
	
	//初始化点轨迹聚类结构器
	void mvInitPtTracksSegment( AnGlobalSet *pGlobalSet );

	//对点轨迹聚类器进行释放
	void mvUninitPtTracksSegment( );

	//对点轨迹进行聚类
	void mvPointTracksSegmente( 	
			int nNoNullTrNum,	  //非空轨迹数目
			int *nANoNullTrIdx,   //非空轨迹所对应的序号
			MyTrackElem **pATrPointer //轨迹对应的地址(指针)
		);

private:
	//对点轨迹进行聚类之前进行处理
	void mvProcessBeforeSegmente( 
			int nNoNullTrNum,	  //非空轨迹数目
			int *nANoNullTrIdx,   //非空轨迹所对应的序号
			MyTrackElem **pATrPointer, //轨迹对应的地址(指针)
			vector<int> &vectWaitSegmentTr //等待聚类分割的轨迹
		);

	//获取点轨迹上的节点运动信息
	void mvGetMoveInfoNodes( const MyTrack& tr,
			StruTrLitSegMvInfo &StruTrLSMvInfo
		);


	//计算两轨迹向量的差异值
	float mvCalcVectorDiffVal( 
			const MyTrack &tr1,  //轨迹1
			const MyTrack &tr2	 //轨迹2
		);

	//计算两轨迹节点距离不一致性
	float mvCalUnconsistencyOfNodeDist( 
			const MyTrack &tr1,  //轨迹1
			const MyTrack &tr2,	 //轨迹2
			IplImage *pCarW,  //车宽图像
			IplImage *pCarH,  //车高图像
			float &fRatio_x,  //当前帧两点x间距与车宽比率
			float &fRatio_y   //当前帧两点y间距与车高比率 
		);


	//计算两轨迹节点距离不一致性
	float mvCalMoveUnconsistency( 
			int nTr1, int nTr2, int nMinComLen, 
			StruTrLitSegMvInfo *pStruTrLSMvInfo
		);

	//对点轨迹进行聚类之后进行处理
	void mvProcessAfterSegmente( 
			int nNoNullTrNum,	  //非空轨迹数目
			int *nANoNullTrIdx,   //非空轨迹所对应的序号
			MyTrackElem **pATrPointer, //轨迹对应的地址(指针)
			const vector<int> &vectWaitSegmentTr //等待聚类分割的轨迹
		);

	//显示点轨迹由近到远的轨迹序号的结果
	void mvShowSimilarityResult( 
			int nTrCnt,		//获取的轨迹数目
			int *nACorTrIdx,   //轨迹对应的序号
			MyTrackElem **m_pATrPointer,  //轨迹对应的地址(指针)
			int nVaildEdgeCnt, //有效边数
			MvSegmEdge *pEdges,  //两两轨迹组合形成的边
			int nShowMod		 //显示模式
		);

	//对图进行分割
	CSegmUniverse * mvSegmentGraph( 
			int num_vertices, 
			int num_edges, 
			MvSegmPoint4D *coors,
			MvSegmEdge *edges,	
			MvSegmDiff *pDiff,
			float c );

private:
	//指向全局信息集的指针
	AnGlobalSet *m_pGlobalSet;
	
	bool m_bInit;	//是否成功初始化

private:
	IplImage *m_pSegmenteTrMap;	//参加聚类的轨迹map

}StruPtTrSegment;

/*
typedef struct MV_GROUP 
{
	int  nGroupId;			//编号
	bool bHaveCarnum;		//是否关联车牌		
	int  nStatus;			//状态,默认为0，

	CvRect rtContour;		//目标轮廓
	MvTrack tr;			    //目标中心运行轨迹,没有关联车牌前，以最长轨迹为主

	int nType;

	vector<int> vecTrackID;		//存储所有在该集合中的trackID
	
	MV_GROUP( )
	{
		nGroupId = -1;
		nStatus = 0;
		bHaveCarnum = false;
		nType = -1;
	}
}MvGroup;

typedef struct _mvSimpleObject 
{
	int     nGroupId;	//对应于group编号
	int     nGroupNo;	//对应在group容器中的序号

	int     nObjType;    //目标的类型

	CvPoint ptCentroid;  //目标的质心

	float   fImgDirection; //目标的图像方向

	int64  ts_appear;  //目标出现报警的时刻,ms为单位

	_mvSimpleObject( )
	{
		nGroupId = -1;
		nGroupNo = -1;

		nObjType = DEFAULT_TYPE;
		ptCentroid  = cvPoint(-1000, -1000); 
		fImgDirection = -1000.0f; 

		ts_appear = -1000000;
	}
}MvSimpleObject;


class CConstructGroup
{
public:

	//获取得到目标的大小图像
	void mvGetObjSzImg( IplImage *pHImg, IplImage *pWImg )
	{
		m_pObjHImg = pHImg,	m_pObjWImg = pWImg;
	};

	//获取得到当前的参数信息
	void mvGetNowParaInfo( int dTs )
	{
		m_tsNow = dTs;
	};

	//由轨迹聚类为group
	int mvConstructGroup( vector<MvTrack> vecTrack ); 

	//建立轨迹ID和轨迹之间的映射关系
	void mvCreateIdTrackMap( vector<MvTrack> vecTrack );

	//从track产生group并进行继承
	void mvTrackGroup( vector<MvTrack> vecTrack );

	//对group结果进行调整
	void AjustGroup( vector<MvTrack> &vecTrack );

	//对group结果保存为目标
	void SaveGroupAsObject( );

	//显示group结果
	void mvShowGroupResult( IplImage *pShowImg, char *p_chWinName=NULL );

private:
	
	//增加group的ID
	int _GenGroupId( );

	//由轨迹产生group
	int GenerateGroup( vector<MvTrack> &vecTrack ); 

	//根据某一点调整左上角点和右下角点 
	void AjustPoint(CvPoint pt, CvPoint &ptLT, CvPoint &ptRB);

	void AjustGroupContour( vector<MvTrack> &vecTrack, MvGroup &gr );

	bool IsNormalGroup( MvGroup &gr );

	int  GetSimTrNumInGroup( MvTrack &tr, MvGroup &gr, map<int,MvTrack> &mapTrack, float fTresh );

	bool IsNear( CvPoint pt, CvRect rt, float fThresh );

	bool AddTrackToGroup( MvTrack &tr, MvGroup &gr, map<int,MvTrack> &mapTrack, float fTresh );

	void MergeSplit(vector<MvTrack> &vecTrack, MvGroup &gr1, MvGroup &gr2);

	void MergeGroup(vector<MvTrack> &vecTrack, MvGroup &gr1, MvGroup &gr2);

public:
	map<int, MvTrack> m_mapTrack;	//trackID 和track对应关系

	int m_nGroupIndex;             //group的ID
	vector<MvGroup> m_vecGroup;	   //存放所有的车辆

	vector<MvSimpleObject> m_vecObject;  //存放目标信息	
private:
	IplImage *m_pObjHImg;
	IplImage *m_pObjWImg;

	int m_tsNow;
private:

};
*/

#endif