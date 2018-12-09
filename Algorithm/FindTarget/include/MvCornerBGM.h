// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   11:53
	filename: 	e:\BocomProjects\find_target_lib\include\MvCornerBGM.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MvCornerBGM
	file ext:	h
	author:		Durong
	
	purpose:	原本想实现一个用sift特征的角点背景模型。没有完全实现。
	目前不起任何作用。
*********************************************************************/


#ifndef MV_CORNER_BGM_H
#define MV_CORNER_BGM_H


#include "MvCorner.h"


// 背景角点
typedef struct _MvBGCorner
{
	MvCorner   corner;
	int          nTimes;
	unsigned int uLastAppearFrameSeq;
	struct _MvBGCorner   *pNext;

	_MvBGCorner()
	{
		nTimes = 0;
		uLastAppearFrameSeq = 0;
		pNext = NULL;
	}
} MvBGCorner;




// 角点背景模型
class MvCornerBGM
{
private:

	// 背景角点
	MvBGCorner *pBGCornerHead;
	

public:
	MvCornerBGM();
	~MvCornerBGM();

	void Input(MvCorner *pCorners, int nCount);
	void GetForeCorners(MvCorner *pCorners, int &nCount);
	void Update();


}; 


#endif