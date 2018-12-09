#ifndef __ROAD_DET_HEADER_H
	#define __ROAD_DET_HEADER_H	

	#include "mvEventDebug.h"	 //调试宏的头文件
	
	#ifdef LINUX		
		#include <sys/time.h>
		#include <sys/timeb.h>
		#include <sys/vfs.h>
		#include <sys/statvfs.h>
	#else
		#include  <io.h>
		#include "XmlParser.h"
	#endif

	#include <stdio.h>
	#include <stdlib.h>
	#include <time.h>
	#include <algorithm> //for std::sort
	#include <vector>
	#include <fstream>
	#include <iostream>
	#include <iomanip>
	#include <list>
	#include <string>
	#include <math.h>
	#include <map>
	#include "mvImage.h"

	#include "ipp.h"

	//#include "cvwimage.h"
	#include "ml.h"
	#include "cxmisc.h"
	#include "sift_descr.h"
	#include "disjoint-set.h"
	#include "cal_main.h"

	#include "comHeader.h"  //放最后
	//#define  USE_SURF
	//#ifdef USE_SURF
	//	#include "MvSurf.h"
	//#endif

	#include "MvBgTrackMap.h"

	//#define  USE_3DMODEL			//使用3D模型 
	#ifdef USE_3DMODEL
		#include "Vehicle3D.h"
	#endif

	#ifdef LINUX
		#define  USE_STAB	//使用稳像
	#else	
	//	#define  USE_STAB	//本地不使用稳像
	#endif
	#include "VideoStab.h"
	

	#include "MvTypeDetect.h"

	#ifndef LINUX
		#include "VehicleConfig.h"
	#endif

	//图像显示和调试
	#include "StopDetector.h"    //停车检测器（包含背景模型和背景线段及停车检测等）
	#include "bufferMechanism.h" //缓冲机制
	#include "mvCameraCtrl.h"    //相机跟踪控制器
	
	#ifndef LINUX
		#define NOPLATE_COLOR
	#else
		#ifdef MVDEBUGTEST
			#define NOPLATE_COLOR    
		#endif
	#endif
	#ifndef NOPLATE_COLOR
		#include "PRoadColor.h"       //颜色识别
	#endif

	#include "carmerTrack.h"      //相机偏移
	#include "MvSurf.h"			  //Surf匹配
	#include "mvCoFeature.h"

	#include "analysis.h"        //分析类
	#include "trafficStat.h"     //交通统计

	#include "MvEventDetect.h"
#endif  //#ifndef __COM_HEADER_H