// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   11:46
	filename: 	e:\BocomProjects\find_target_lib\include\MvBgLine.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MvBgLine
	file ext:	h
	author:		Durong
	
	purpose:	直线背景模型，类似于角点背景模型。
*********************************************************************/



#ifndef MV_BG_LINE_H
#define MV_BG_LINE_H


#include "MvLineSegment.h"

class MvBgLine
{

public:
	// 初始化，告之图像宽度与高度。
	// nInterval为统计的次数。统计数据每达到nInterval次，就生成新背景，并重新开始统计。
	void Init( int width, int height, int nInterval );

	void Destroy();

	// 添加统计数据
	void Update(BLine *pLines, int nCount);

	// 将输入直线pAllLines，分为背景直线与非背景直线。
	void SepBgLines(BLine *pAllLines, int nAllCount, BLine *pNonBgLine, int &nNonBgLineCount, BLine *pBgLine, int &nBgLineCount);

	// 存背景直线图，用于观察直线背景效果如何。
	void SaveBgImage() const;

private:

	// 认为一个像素是背景的概率要求。
	// 直线的所有像素点中背景像素点占整个线段的像素点的比例超过p2，则认为线段是背景。
	bool IsBgLine(BLine *pLine, float p1, float p2);

private:
	int m_nWidth;
	int m_nHeight;
	int m_nCount;
	int m_nInterval;

	int *m_pMapStat;
	int *m_pMapBg;


	int *m_pTemp;
	int m_nTmpSize;

};

#endif