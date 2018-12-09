// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef ULTILITY_H
#define ULTILITY_H

#ifndef BYTE
	typedef unsigned char BYTE;
#endif

#ifndef _max__
	#define _max__
	template<typename T>
	inline T _max_(T a, T b) { return (a) > (b) ? (a) : (b);}
#endif

#ifndef MV_SQUARE
	#define MV_SQUARE
	template <typename T>
	inline T square(const T &x) { return x*x; };
#endif


#ifndef _min__
	#define _min__
	template<typename T>
	inline T _min_(T a, T b) { return (a) < (b) ? (a) : (b);}
#endif

enum _CMODE
{
	V_BGR = 0,
	V_RGB
};

enum __SMOOTH
{
	V_SM_ORIGINAL = 0,
	V_SM_MEDIAN,
	V_SM_CONTOURS,
	V_SM_CONTOUR_MEDIAN
};

#ifdef LINUX
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#define _chdir(a) chdir(a)
	#define _mkdir(a) mkdir(a, 0777)
#else
	#include <direct.h>
#endif


#ifdef LINUX
	//#define DEBUG_SAVE_GLOBAL_TIME
	//#define DEBUG_VIBE_GLOBAL_TIME
	//#define DEBUG_VIBE_SAVE_ILLU_FILE
#else
	//#define DEBUG_VIBE_SAVE_ILLU_FILE
	//#define DEBUG_VIBE_BGMODELIMAGES
	//#define DEBUG_VIBE_SAVE_SHADOW_IMG
	//#define DEBUG_VIBE_FILTERING_TIME
	//#define DEBUG_VIBE_ILLU_TIME
	//#define DEBUG_VIBE_UPDATEILLU_TIME
	//#define DEBUG_UPDATE_TIME
	//#define DEBUG_SAVE_GLOBAL_TIME
	//#define DEBUG_BKIMG_TIME
	//#define DEBUG_VIBE_INIT_TIME
	//#define SHOW_IMAGES
	//#define DEBUG_VIBE_GLOBAL_TIME
	//#define DEBUG_VIBE_RESULTIMAGES

#endif




#endif
