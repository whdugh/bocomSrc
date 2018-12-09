// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary 


// bufferMechanism.h: interface for the StopDetector class.
//
//////////////////////////////////////////////////////////////////////
#ifndef  _BUFFERMECHANISM_H
#define  _BUFFERMECHANISM_H

#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include "ipp.h"

#include "comHeader.h"  //放最后

#ifndef MAX
	#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef  MAXBUFFERNUM
	#define  MAXBUFFERNUM  100
#endif

//#define  DEBUG_BUFFER  

//-------------------------类CBufferMechanism-------------------------//
typedef  struct _eventBufferStrcuct
{
	unsigned long    nVehicleId;   //vehicle的id号
	_eventBufferStrcuct( )
	{
		nVehicleId = 0;	
	}
}eventBufferStrcuct;

class CBufferMechanism 
{
public:
	CBufferMechanism();
	virtual ~CBufferMechanism();
	
	//接口函数
	bool bufferMechanismInit();
	bool bufferMechanismUninit();

	//添加目标
	bool mvAddObject(const MyGroup &group, double ts_now, double dBuffTimeThres); 

	//比较目标
	bool mvCompareObject(const MyGroup &group, int mod, double ts_now,
						double dBuffTimeThres, int &nVaildCompareTime,
						unsigned long *nAVaildCompareId); 
private:
	bool     m_bEmpty;  //是否为空
	int      m_startN;
	int      m_endN;
	eventBufferStrcuct  *m_pBuffEvent;	  //可将其缩小
	double   *m_tsAddToBuff;

};


//-------------------------类CMayBigVBuffer-------------------------//

#ifndef  MAXTRNUMOFBIGV
	#define  MAXTRNUMOFBIGV 400
#endif

typedef  struct _mayBigVStrcuct{

	unsigned long    nVidx;   //vehicle的id号
	double   fImgOri;         //vehicle的图像运动方向
	double   ts_add;          //加入到vehicle的最新时刻

	CvPoint  trLt_pt;         //轨迹外包矩形的左上角
	CvPoint  trRb_pt;         //轨迹外包矩形的右下角

	int      nTrNum;
	int      nATrNo[MAXTRNUMOFBIGV];         //轨迹在轨迹集合中的序号
	unsigned long  nATrId[MAXTRNUMOFBIGV];  //轨迹真实ID号

	_mayBigVStrcuct( )
	{
		nVidx = 0;
		fImgOri = -1.0f;
		ts_add = -100000.0f;   //不加会出异常,从buffer中找大车时有问题
		trLt_pt = cvPoint( 10000, 10000 ); 
		trRb_pt = cvPoint( 0, 0 );   
		nTrNum = 0;
	}

}mayBigVStrcuct;


class CMayBigVBuffer 
{
public:
	CMayBigVBuffer();
	virtual ~CMayBigVBuffer();
	
	//接口函数
	bool mayBigVBufferInit( );
	bool mayBigVBufferUninit( );

	bool mvget_mayBigV_inBuffer( 
			double ts_now, 
			double dBuffTimeThres,
			int &nVaildCompTime, 
			int *nAVaildCompId );
	bool mvadd_mayBigV_toBuffer(
			mayBigVStrcuct  bigVehicle, 
			double dBuffTimeThres ); 

private:
	bool   m_bEmpty;  //是否为空
	int      m_startN;
	int      m_endN;

public:
	mayBigVStrcuct  *m_pBuffBigV;	

};

#endif
