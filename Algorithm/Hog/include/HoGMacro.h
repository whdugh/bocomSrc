#ifndef __MV_HOG_MACRO_H
#define __MV_HOG_MACRO_H

#ifdef  OPENCV245
	#include "opencv2/opencv.hpp"
	#include "opencv2/objdetect/objdetect.hpp" 
	#include "opencv2/features2d/features2d.hpp" 
	#include "opencv2/highgui/highgui.hpp" 
	#include "opencv2/calib3d/calib3d.hpp" 
	#include "opencv2/imgproc/imgproc_c.h" 
	#include "opencv2/imgproc/imgproc.hpp"   
	#include "opencv2/nonfree/features2d.hpp"
	#include "opencv2/legacy/legacy.hpp"
	#include "opencv2/legacy/compat.hpp"

	#include <wtypes.h>
	using namespace cv; 
#else
	#include "cv.h"
	#include "highgui.h"
	#include "cxcore.h"
#endif

#include <errno.h>
#include <string.h>

#ifdef LINUX
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#ifndef CHDIR
		#define CHDIR
		#define _chdir(a) chdir(a)
	#endif
	#ifndef MKDIR
		#define MKDIR
		#define _mkdir(a) mkdir(a, 0777)
	#endif
	#ifndef ACCESS
		#define ACCESS
		#define _access(a, b) access(a, b)
	#endif
#else
	#include <direct.h>
	//#define DEBUG_HOG_CALGRADIENT   //显示新的梯度计算方法的计算结果
#endif

#ifndef MAX
	#define MAX(a,b)   (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define MV_IPL_BORDER_CONSTANT   0
#define MV_IPL_BORDER_REPLICATE  1
#define MV_IPL_BORDER_REFLECT    2
#define MV_IPL_BORDER_WRAP       3
#define MV_IPL_BORDER_REFLECT_101    4
#define MV_IPL_BORDER_TRANSPARENT    5
enum{  
	MV_BORDER_REPLICATE = MV_IPL_BORDER_REPLICATE, 
	MV_BORDER_CONSTANT = MV_IPL_BORDER_CONSTANT,
	MV_BORDER_REFLECT = MV_IPL_BORDER_REFLECT, 
	MV_BORDER_REFLECT_101 = MV_IPL_BORDER_REFLECT_101,
	MV_BORDER_REFLECT101 = MV_BORDER_REFLECT_101,
	MV_BORDER_WRAP = MV_IPL_BORDER_WRAP,
	MV_BORDER_TRANSPARENT,
	MV_BORDER_DEFAULT = MV_BORDER_REFLECT_101, 
	MV_BORDER_ISOLATED =16 
};


//HoG模型的类型
enum{ HOG_PEO_1372 = 0, HOG_HEAD_1372 };

#define USE_NEWGRADIENT_COMPUTE  //使用新的计算梯度的方法

//-----原来HoG的宏定义-------
#define WIN_SIZE_X  32      //32 //检测窗的宽
#define WIN_SIZE_Y  64      //64 //检测窗的高
#define BLOCK_STRIDEX 8     //块每次X滑动的距离
#define BLOCK_STRIDEY 8     //块每次Y滑动的距离
#define BLOCK_SIZE  16      //块的大小
#define CELL_SIZE	8       //cell大小
#define FDIM		840     //840HoG特征维数
#define LINEFDIM	840	    //840线段HoG特征	
#define MAXHOG_WINNUM  200  //HoG最多的窗口


//-------新HoG行人的宏定义-------
#define SH_WIN_SIZE_X  32     //检测窗的宽
#define SH_WIN_SIZE_Y  64     //检测窗的高	
#define CELL_SIZE_X   4    //块中每个单元的宽
#define CELL_SIZE_Y   8    //块中每个单元的高
#define GRADDIR_HIST_BIN_CNT 6		//梯度方向对应的bin数
#define ADD_GRADDIR_HIST_BIN_CNT 1	//添加的梯度方向bin数

#define SH_BLOCK_STRIDEX  CELL_SIZE_X  //块每次X滑动的距离
#define SH_BLOCK_STRIDEY  CELL_SIZE_Y  //块每次Y滑动的距离

#define BLOCK_SIZE_X  (2*CELL_SIZE_X) //块的宽
#define BLOCK_SIZE_Y  (2*CELL_SIZE_Y) //块的高

#define X_BLOCK_CNT  ( 1+(SH_WIN_SIZE_X-BLOCK_SIZE_X)/SH_BLOCK_STRIDEX ) //x方向的块数
#define Y_BLOCK_CNT  ( 1+(SH_WIN_SIZE_Y-BLOCK_SIZE_Y)/SH_BLOCK_STRIDEY ) //y方向的块数
#define BLOCK_CNT	 ( X_BLOCK_CNT*Y_BLOCK_CNT )   //窗口中的block数目

#define MV_HIST_BIN_CNT (GRADDIR_HIST_BIN_CNT+ADD_GRADDIR_HIST_BIN_CNT)	//bin数目

#define BLOCK2CELL_NUM ((BLOCK_SIZE_X/CELL_SIZE_X)*(BLOCK_SIZE_Y/CELL_SIZE_Y)) //block中cell数

#define BLOCK_FEAT_DIM  (BLOCK2CELL_NUM*MV_HIST_BIN_CNT)   //块的特征维数

#define SIMHOG_FDIM    (BLOCK_CNT*BLOCK_FEAT_DIM)   //HoG特征维数


//-------新HoG头肩的宏定义-----------
#define HEAD_WIN_SIZE_X  48     //检测窗的宽
#define HEAD_WIN_SIZE_Y  48     //检测窗的高	
#define HEAD_CELL_SIZE_X   6    //块中每个单元的宽
#define HEAD_CELL_SIZE_Y   6    //块中每个单元的高
#define HEAD_GRADDIR_HIST_BIN_CNT 6		//梯度方向对应的bin数
#define HEAD_ADD_GRADDIR_HIST_BIN_CNT 1	//添加的梯度方向bin数

#define HEAD_BLOCK_STRIDEX  HEAD_CELL_SIZE_X  //块每次X滑动的距离
#define HEAD_BLOCK_STRIDEY  HEAD_CELL_SIZE_Y  //块每次Y滑动的距离

#define HEAD_BLOCK_SIZE_X  (2*HEAD_CELL_SIZE_X) //块的宽
#define HEAD_BLOCK_SIZE_Y  (2*HEAD_CELL_SIZE_Y) //块的高

#define HEAD_X_BLOCK_CNT  ( 1+(SH_WIN_SIZE_X-BLOCK_SIZE_X)/SH_BLOCK_STRIDEX ) //x方向的块数
#define HEAD_Y_BLOCK_CNT  ( 1+(SH_WIN_SIZE_Y-BLOCK_SIZE_Y)/SH_BLOCK_STRIDEY ) //y方向的块数
#define HEAD_BLOCK_CNT	 ( HEAD_X_BLOCK_CNT*HEAD_Y_BLOCK_CNT )   //窗口中的block数目

#define HEAD_HIST_BIN_CNT (HEAD_GRADDIR_HIST_BIN_CNT+HEAD_ADD_GRADDIR_HIST_BIN_CNT)	//bin数目

//block中cell数
#define HEAD_BLOCK2CELL_NUM ((HEAD_BLOCK_SIZE_X/HEAD_CELL_SIZE_X)*(HEAD_BLOCK_SIZE_Y/HEAD_CELL_SIZE_Y)) 

#define HEAD_BLOCK_FEAT_DIM  (HEAD_BLOCK2CELL_NUM*HEAD_HIST_BIN_CNT)   //块的特征维数

#define HEAD_SIMHOG_FDIM    (HEAD_BLOCK_CNT*HEAD_BLOCK_FEAT_DIM)   //HoG特征维数


//#define DEBUG_HOGTIME
//#define DEBUG_SAVE_HOG_TIME
//#define DEBUG_SAVE_HOG_RESULTS

#ifndef LINUX
//	#define SHOW_HOGCALC_PROCESS  //显示HoG计算的过程
#endif


#endif
