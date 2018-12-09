// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   14:44
	filename: 	e:\BocomProjects\find_target_lib\include\SynCharData.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	SynCharData
	file ext:	h
	author:		Durong
	
	purpose:	用于左右相机之间同步用的同步数据。
	记录了物体在一些时间点上的位置信息。
*********************************************************************/


#ifndef SYN_CHAR_DATA_H
#define SYN_CHAR_DATA_H


#include <list>
#include <map>
#include "MvPointList.h"



typedef std::map<int64, MvPointList> ObjSyncData;//描述物体某一时间所处位置的点群



#endif