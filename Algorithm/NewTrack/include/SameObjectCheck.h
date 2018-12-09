//判断是否为同一目标的检查器
#ifndef __SAME_OBJECT_CHECK_H
#define __SAME_OBJECT_CHECK_H

#include <vector>
#include "libHeader.h"
#include "comHeader.h"

using namespace std;

#define MAX_SAVE_REMOTE_PICNUM 4   //最多所保存的远景图像数目
#define MAX_SAVE_STOPAREA_NUM 100  //最多所保存的停止区域数目
#define MAX_PTNUM_OF_STOPAREA 100  //停止区域最多的角点数目

enum enumCheckMode  //检查是否为同一类事件的目标
{
	CHECK_SAME_STOP_OBJ = 0,
	CHECK_SAME_OTHER_OBJ	
};

//Id分配器
typedef struct StructIdAssign
{
public:
	int m_nId;

public:
	StructIdAssign( );

	void mvInitVar( );  //初始化变量
	void mvIdAdd( );    //对ID进行增加
 
}AnIdAssign;


//事件ID分配器
typedef struct StructEventIdAssign
{
public:
	AnIdAssign m_stopEventIdAssign;  //停止事件ID分配器

public:
	StructEventIdAssign( );

	void mvInitVar( );               //初始化变量
	bool mvIdAdd( int nCheckMode );  //对ID进行增加
	int  mvGetId( int nCheckMode );  //获取当前要查找事件的ID
		 
}AnEventIdAssign;


//停止区域节点结构
typedef struct StructStopAreaNode
{
public:
	double dTsAdd;         //节点的加入的时间戳
	double dTsUpdate;      //节点的加入的更新戳

	CvPoint ptLtStopArea;  //节点所对应的停车区域左上点
	CvPoint ptRbStopArea;  //节点所对应的停车区域右下点

	int		 nPtCnt;	   //角点数目 
	siftFeat pSiftFeat[MAX_PTNUM_OF_STOPAREA];  //各角点的特征
public:
	StructStopAreaNode( );

}AnStopAreaNode;


//停止区域存储结构
typedef struct StructStopObjStore
{
public:
	double dTsAdd;		  //该区域的加入时间戳
	int    nStopObjId;	  //该区域所对应的停止目标的ID

	CvRect rct4CamCtrl;   //相机控制时的rect区域

	int  nSaveNodeCnt;    //该区域已保存的节点数目
	int  nHadSavePicCnt;  //该区域已保存的图片数目
	AnStopAreaNode stopAreaChain[MAX_SAVE_REMOTE_PICNUM]; //停止区域的存储链

public:
	StructStopObjStore( );

}AnStopObjNode;


//针对"需保存多张远景预置位图"而检查"是否为相同停止区域"
typedef struct StruSameStopAreaCheck
{
public:
	StruSameStopAreaCheck( );

	void mvInitVar( );

	//初始化
	void mvInitSameStopAreaCheck( );

	//设置点的匹配范围
	void mvSetPtMatchRange( const CvPoint &ptMatchRange );
	
	//释放
	void mvUninitSameStopAreaCheck( );

	//检查给定的目标是否在停止区域存储中
	int mvCheckAndSaveGiveObj(
			double dTsNow,           //当前的时间戳
			double dMinInterval,	 //当前与区域节点的最小间隔 
			const CvPoint &ptLtArea, //给定的停止区域左上点
			const CvPoint &ptRbArea, //给定的停止区域左上点
			const int nPtCnt,       //给定的停止角点数目
			siftFeat *pSiftFeat     //给定的停止角点特征(位置和sift值)
		);

	//将当前结果作为节点加入到停止区域存储中
	int mvAddGiveObjAsStopStoreNode(
			double dTsNow,         //当前的时间戳
			const CvPoint &ptLtA,  //当前区域左上点
			const CvPoint &ptRbA,  //当前区域右下点
			const int nPtCnt,          //当前区域的角点数目
			const siftFeat *pSiftFeat, //当前区域的各角点特征
			const int nMatchStoreIdx = -1 //匹配上存储节点ID
		);

	//从停止存储中获取给定ID的停止区域(返回该区域存的节点数目)
	int mvGetGiveId4StopStore(
			const int nGiveStopEventId  //给定的停止事件的ID
		);

	//对停止存储中给定ID的停止区域设置其"拉相机的rect"
	bool mvSetCamCtrlRect4GiveId( 
		    const CvRect &rctCamCtrl,  //相机拉动前对应的rect
			const int nGiveStopEventId  //给定的停止事件的ID
		);

	//从停止存储中获取给定ID的停止区域所对应"拉相机的rect"
	bool mvGetCamCtrlRect4GiveId(
		    CvRect &rctCamCtrl,  //相机拉动前对应的rect
			const int nGiveStopEventId  //给定的停止事件的ID
		);


	//从停止存储中删除没再检测到的停止区域(返回删除节点的事件ID)
	vector<int> mvDelNomoreDet4StopStore(
			double dTsNow,       //当前的时间戳
			double dMaxSaveTime  //最多保留的时间
		);

	//从停止存储中节点获取已经保存了节点，但还没存图像的停止区域
	vector<int> mvGetNoSavePic4StopStore( 
			vector<CvRect> &vectObjArea, //得到的停止目标区域
			const int nSaveNodeCntTh     //保存节点的数目要求
		);

	//对还没存图像的停止区域的存图数自增
	void mvAddSavePicCnt4StopStore( 
			const vector<int> &vectGiveStopEventId  //给定的停止事件的ID
		);

public:
	CvPoint  m_ptMatchRange;  //点匹配范围

	AnEventIdAssign  m_eventIdAssign;  //事件ID分配器
	vector<AnStopObjNode> m_vectStopCache;  //停止缓存

}AnSameStopAreaCheck;


////////////////////////////////////////////
#endif