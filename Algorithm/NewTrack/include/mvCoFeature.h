// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef __MV_COFEATURE__H
#define __MV_COFEATURE__H

#include <vector>
#include <utility>

#include "libHeader.h"

typedef struct _MvBaseObject
{
	vector<CvPoint2D32f> m_vecCentroid;
	vector<double> m_vecTs;
	CvPoint m_ptLT;
	CvPoint m_ptRB;
	CvPoint m_ptPredict;				//预测点（len>=3 时才有效）
#ifndef LINUX
	CvScalar debugColor;				//用于跟踪时显示用
#endif
	_MvBaseObject()
	{
#ifndef LINUX
		debugColor = CV_RGB((uchar)rand(), (uchar)rand(), (uchar)rand());
#endif
	}
}MvBaseObject;

//带有状态说明的obj
//说明：nType			 0			不具有参考意义，不能用此位置做任何判断
//									 1			通过静态特征检测出(可能误检)
//									 2			通过动态特征判断出
//									 4		    根据上帧位置推测出的位置
//									 8          确认的位置
typedef struct _MvExtBaseObject
{
	int nType;
	MvBaseObject m_obj;
	_MvExtBaseObject()
	{
		nType = 2;
	}
}MvExtBaseObject;

//行为分析
typedef struct _MvObject:MvBaseObject
{

	double ts_person_appear;		//行人出现判定
	double ts_person_against;		//行人逆行判定
	double ts_beyondMark;			//越界
	double ts_fast;					//奔跑

	_MvObject()
	{
		ts_person_appear = -1;
		ts_person_against = -1;
		ts_beyondMark = -1;
		ts_fast = -1;
		m_ptPredict.x = -1;
		m_ptPredict.y = -1;

	}

}MvObject;

//事件关联
typedef struct _MvEventObject:MvBaseObject
{
	int nVehicleID;					//
	double ts_carnum_detect;		//车牌检测时间
	double ts_event;				//事件发生时间
	_MvEventObject()
	{
		ts_carnum_detect = -1;
		ts_event = -1;
	}
}MvEventObject;

//事件
typedef struct _MvEventType
{
	int nEventType;				//类型
	CvPoint2D32f ptLocation;	//地点
	double ts;					//发生事件
	int nObjType;				//目标类型
	double dValue1;				//近期速度
	double dDirection;			//方向
	_MvEventType()
	{
		ts = -1;
	}
}MvEventType;

//目标在某一时刻在图像中的位置
typedef struct _MvObjRect
{
	double ts;
	CvPoint ptLT;
	CvPoint ptRB;
}MvObjectRect;

typedef struct _mvCarnumTrack
{
	vector<MvObjectRect> vecCarnumTrack;
	bool bAssociate;
	char cCarNum[7];
	char wjcarnum[2];      //武警牌中间的两个小字
	int     color;          //车牌颜色
	int     vehicle_type;     //车辆类型
	int     carnumrow;        //单排或者双排车牌
	int nChannel;
	int nSavePicID;			//尾牌存图时用到
	CvRect rtPosition;    
	bool bSave;					//存图
	bool bSure;					//确认后结果
	_mvCarnumTrack()
	{
		bAssociate = false;
		memset(cCarNum,0,7);
		memset(wjcarnum,0,2);
		rtPosition = cvRect(0,0,0,0);
		nSavePicID = -1;
		bSave = false;
		bSure = false;
	}
}MvCarnumTrack;

//typedef vector<MvCarnumRect> MvVecRtTrack;

typedef struct _MvObjInfo
{
	int nObjID;
	vector<SRIP_DETECT_OUT_RESULT> vecEvent;
	vector<MvObjectRect> vecObjRect;
	bool bCarnum ;
	MvCarNumInfo carnum;
	bool bTail;
	int nEventID;    //车牌为尾牌时用到，bTail = ture;
	_MvObjInfo()
	{
		bCarnum = false;
		memset(&carnum,0 ,sizeof(MvCarNumInfo));
		bTail = false;
		nEventID = -1;
	}
}MvObjInfo;

typedef struct _MvForeRect
{
	CvRect rtForeRT;			//前景区域
	vector<CvPoint> vecHead;	//候选头部
}MvForeRect;

//说明：nType			 0			不具有参考意义，不能用此位置做任何判断
//									 1			通过静态特征检测出(可能误检)
//									 2			通过动态特征判断出
//									 4		    根据上帧位置推测出的位置
//									 8         确认的位置
typedef struct _MvCoTrackPoint
{
	int nType;		//0
	CvPoint ptPos;
	_MvCoTrackPoint()
	{
		nType = 0;			//
		ptPos.x = 0;
		ptPos.y = 0;
	}
}MvCoTrackPoint;

//结合动态和静态特征的group
typedef struct _MvGroupRect
{
	vector<MvCoTrackPoint> vecHeadPos;		//头部位置
											//车牌位置
	MvBaseObject m_obj;						//目标位置信息
	


}MvGroupRect;

#endif
