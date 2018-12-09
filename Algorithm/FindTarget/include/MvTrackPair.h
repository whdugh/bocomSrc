// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   15:00
	filename: 	e:\BocomProjects\find_target_lib\include\MvTrackPair.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MvTrackPair
	file ext:	h
	author:		Durong
	
	purpose:	表示一对轨迹的结构体。程序中没有使用
*********************************************************************/


#ifndef MV_TRACK_PAIR_H
#define MV_TRACK_PAIR_H

typedef struct _MvTrackPair
{
	int nIndex1;
	int nIndex2;
	float fDis;

	int nDSIndex1;
	int nDSIndex2;

public:
	_MvTrackPair(int i1, int i2, float fd, int nDSIndex1, int nDSIndex2)
	{
		nIndex1 = i1;
		nIndex2 = i2;
		fDis    = fd;
		this->nDSIndex1 = nDSIndex1;
		this->nDSIndex2 = nDSIndex2;
	}

	static bool IsPaDisLessThanPb(const _MvTrackPair& pa, const _MvTrackPair& pb);
}MvTrackPair;

#endif

