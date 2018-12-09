//智能交通的结构体-用于代码管理和简化
#ifndef _STRU_ROAD_DET_H_
#define _STRU_ROAD_DET_H_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "libHeader.h"

#include "comHeader.h"

#include "BaseStruct.h"

#include "MvLineSegment.h"

enum ENUM_AN_EVENT_ALERT_TYPE
{
	AN_OBJAPPEAR_ALERT = 0,    //目标出现
	AN_AGAINST_ALERT,          //逆行 
	AN_CROSS_ALERT,            //横穿
	AN_CHANNEL_CHANGE_ALERT,   //变道
	AN_STOP_ALERT              //停止
};

//线段结构体
typedef struct BGLineElem 
{
	BLine   line;
	bool    is_bg;
	double	ts_first;//时间戳，秒
	double	ts_now;//时间戳，秒
	int		occur_times;
}BGLineElem;


//事件所用的maxisze图像集合结构体
//(这些图像的大小均一致)
typedef struct _EventMaxsizeImgSet
{
public:
	CvSize sz;  //图像的大小

	IplImage *max_size;    //maxsize直径图

	IplImage *max_sizex;   //车的maxsize的x直径图
	IplImage *max_sizey;   //车的maxsize的y直径图

	IplImage *max_sizex45;    //车的maxsize的x直径旋转45°图
	IplImage *max_sizey45;    //车的maxsize的y直径旋转45°图

	IplImage *m_pCarMaxSizeXImg;   //车的宽
	IplImage *m_pCarMaxSizeYImg;   //车的高

	IplImage *m_pPeoMaxSizeXImg;   //行人的宽
	IplImage *m_pPeoMaxSizeYImg;   //行人的高

public:
	_EventMaxsizeImgSet( );	

	void initVar( );

	void mvSetImgWH( int nW, int nH );

	bool createImages( );

	void releaseImgSet( );

}EventMaxsizeImgSet;



//事件所用的cvSet集合
typedef struct _EventUseCvSetSeq
{
public:
	CvMemStorage *pMemStorage;
		
	CvSet  *pTrackSet;
	CvSet  *pObjResultSet;
	CvSet  *pObjHistorySet; 
	CvSeq  *pBgLineSeq;

public:
	void initVar( );	

	void createSet( );	

	void relesaeSet( );
	
}EventUseCvSetSeq;



//事件所用的cvSet集合
typedef struct _EventUseCvSetSeq2
{
public:
	CvMemStorage *pMemStorage;

	CvSet  *pTrackSet;
	CvSet  *pObjResultSet;
	CvSet  *pObjHistorySet; 
	CvSet  *pCarNumSet;
	CvSeq  *pBgLineSeq;

public:
	void initVar( );	

	void createSet( );	

	void relesaeSet( );

}EventUseCvSetSeq2;


//调试控制结构体


#ifndef EVENT_COFING_LINE
	#define EVENT_COFING_LINE
	enum
	{
		EVENTCFG_YELLOW_LINE = 0,
		EVENTCFG_LEADSTREAM_LINE
	};
#endif

//EventStruConfigLine:config line struct for event detect
typedef struct EventStruConfigLine
{
public:

	//get the give mode config line
	bool mvGetConfigLinesOfChannel( 
			int nChannelCnt,   //number of channels
			VEHICLE_PARAM *pParamChan, //pointer of channels
			int nGetLineMod,   //need get line's mode
			vector<int> &vecPtCnt,  //point number of line
			vector<CvPoint *> &vecPtPointer //point address of line
		); 

private:
	//将链表中的点转为数组中的点，便于使用
	CvPoint* mvPtListToPoint( PointList vPlist, int &nPtCnt );

}struConfigLine;



//时间报警处理结构体
typedef struct EventAlertProcess
{
public:
	EventAlertProcess( )
	{
		initVar( );
	}

	void initVar( )
	{
		m_nShowTime = 3;	  //同一事件的显示时间
		m_nSameEventIgn = 0;  //同类事件忽略检测的时间
	}
	
	//处理已经报警过的目标
	bool mvProcAlertedObj( MyGroup &obj, int nAlertMod, float fDuration );

private:
	int m_nShowTime;	  //同一事件的显示时间
	int m_nSameEventIgn;  //同类事件忽略检测的时间

}StruEventAlertProc;


//目标检测所得到的停止区域信息
typedef struct _StruObjStopAreaInfo
{
	bool   bChannelIsJam;   //目标所在的车道是否拥堵

	int    nObjType;        //目标类型
	int    nTrCnt;			//轨迹数目(包含估计的)

	CvPoint2D32f fptObjLt;  //目标的左上点
	CvPoint2D32f fptObjRb;  //目标的右下点

	CvPoint2D32f fptStopAreaLt; //停止区域的左上点
	CvPoint2D32f fptStopAreaRb; //停止区域的右下点

	double	 dTsStop;		//发现停下来的时间戳
	double	 dAlertTime;    //报警时间要求

	_StruObjStopAreaInfo( )
	{
		bChannelIsJam = false;

		nObjType = 0;
		nTrCnt = 0;

		dTsStop = -10000.0;
		dAlertTime = -10000.0;
	}
}StruObjStopAreaInfo;


//前段时间内的多次检测/统计结果的存储
typedef struct StruDetStaResStore
{
public:
	StruDetStaResStore( );
	~StruDetStaResStore( );

	//初始化
	void mvInitDetStaResStore( 
			int nMaxSaveTime,    //最多存储的结果次数
			int nOneTimeSaveCnt  //一次存储多少个数据
		);	

	//释放
	void mvUninitDetStaResStore( );

	//添加一元素
	bool mvAddOneElement(
			double dTsNow,  //当前时间戳
			int nDataDim,   //要加入数据的维数
			double *dAData  //要加入的数据
		);	

	//搜索距今时间差为给定值之内的所保存的元素值
	double** mvSearchInPreTimeElement( 
			int &nSearchCnt,		//搜索到的数据记录条数
			int &nDim,				//搜索到的数据的维数
			double dTsNow,			//当前时间戳
			double dPreTime,		//往前搜索的时间	
			bool bReturnData = true //是否需返回数据
		);

private:
	//初始化变量
	void mvInitVar( );

private:
	bool m_bInit;          //是否进行过初始化

	int	m_nMaxSaveTime;    //最多存储的结果次数
	int m_nOneTimeSaveCnt; //一次存储多少个数据

	CycleReplace m_CR;     //循环覆盖体
	double **m_ppData;     //存储的数据
	
}AnDetStaResStore;




#endif