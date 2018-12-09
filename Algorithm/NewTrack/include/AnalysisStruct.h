#ifndef __AN_ANALYSIS_STRUCT_H

#define __AN_ANALYSIS_STRUCT_H

#include <stdio.h>
#include "libHeader.h"
#include "BaseMacro.h"

using namespace std;

//检测结果输出模式
enum EnumDetResOutPutMode
{ 
	NORMOAL_OUT_MODE = 0,	   //正常模式输出
	JUST_SAVE_REMO_PIC_NOOUT,  //只存一张远景图，不输出
	SAVE_REMO_NEAR_PIC_NOOUT,  //存一组远/近景组合图，不输出
	DELETE_CACHE_PIC_NOOUT     //删除缓存的图像，不输出
};

//sift特征点信息结构
#ifndef SIFT_FEAT_STRUCT
	#define SIFT_FEAT_STRUCT
	typedef struct _siftFeat
	{
		CvPoint pt;                //点位置
		uchar   cSift[SIFT_DIMS];  //特征值
	} siftFeat;
#endif


//前景轮廓信息结构
#ifndef FORE_CONTOUR_STRUCT
	#define FORE_CONTOUR_STRUCT
	typedef struct _fkContourInfo
	{
		double			   dArea;		 //对应的面积
		double			   dArcLen;      //对应的弧长
		CvRect			   rect;		 //对应的rect
		int                nTopPtNo;	 //最上面的点的序号
		int                nBottomPtNo;  //最下面的点的序号
		int                nLeftPtNo;	 //最左面的点的序号
		int                nRightPtNo;	 //最右面的点的序号
		vector<CvPoint>    pts;			 //对应的点组
	}fkContourInfo;
#endif


#ifndef IMG_FRAME_STRUCT
	#define IMG_FRAME_STRUCT
	//图像帧结构
	typedef struct _MyIMG_FRAME
	{
		char*		chData;

		bool        bVaild;
		int			nCfgWidth;
		int			nCfgHeight;
		int			nSrcWidth;
		int			nSrcHeight;
		int         nUseWidth;
		int         nUseHeight;

		float		fCfg2use_x;
		float		fCfg2use_y;

		int64		nTs;	

		_MyIMG_FRAME( )
		{
			initVar( );
		}

		void initVar( )
		{
			chData  = NULL;

			bVaild = false;
			nCfgWidth  = 0;
			nCfgHeight = 0;
			nSrcWidth  = 0;
			nSrcHeight = 0;
			nUseWidth  = 0;
			nUseHeight = 0;

			fCfg2use_x = 1.0f;
			fCfg2use_y = 1.0f;

			nTs		   = 0;
		}

	}MyIMGFRAME;
#endif


#ifndef AN_CONFING_STRUCT
	#define AN_CONFING_STRUCT
	//系统设置的配置参数
	typedef struct _configParamSet
	{
		int nDayNightP;      //-1,0,1
		int nShadowP;        //-1,0,1
		int nSensitiveP;
		int nSameEventIgnP;  //同类事件的忽略时间
		int nShowTimeP;      //报警的停留显示时间
	}ConfigParamSet;  

	//获取并翻译得到的配置参数
	typedef struct _GetParamSet
	{
		int64   nFrameNo;       //当前帧号

		int64   nTsNow;         //毫秒为单位
		double  dTsNow;         //秒为单位

		int		nDayNight;      //-1,0,1--自己判断，晚上，白天
		bool    bDay;           //是否为白天

		int		nShadow;        //-1,0,1--自己判断，不检阴影，检阴影

		int		nSensitive;     //是否敏感

		bool	bRoadFar;       //是否存在明显的远近区别   

		int		nSameEventIgn;  //同类事件的忽略时间
		int		nShowTime;      //报警的停留显示时间

		bool    bBankPeoAppDet;  //是否检测银行行人出现

		_GetParamSet( )
		{
			nSensitive = 0;
			bRoadFar = true;

			bBankPeoAppDet = false;
		};
	}GetParamSet;  
#endif

#ifndef AN_GENSIM_DETECTOUT_STRUCT
	#define AN_GENSIM_DETECTOUT_STRUCT
	//较为通用的简单的检测结果结构
	typedef struct _genSimDetectOut
	{
		bool    bVaild;
		int     nEventId;           //事件的ID

		CvPoint ptEvent;			//事件发生的点
		CvRect  rectCar;            //车所在的区域
		int     nObjType;			//目标类型
		double  dDirection;			//行驶方向
		bool    bShow;              //是否为停留显示
		float   fLastImgTime;       //往前取上张图片的时间(单位秒)

		_genSimDetectOut( )
		{
			bVaild = false;
			nEventId = -1;

			ptEvent = cvPoint( -100, -100 );
			rectCar = cvRect( -100, -100, -100, -100 );
			nObjType = -1;
			dDirection = -1000;
			bShow = true;
			fLastImgTime = -10000;
		}
	}GenSimDetectOut;
#endif

#ifndef AN_TIME_STAMP_IMG
	#define AN_TIME_STAMP_IMG
	typedef struct StruTimeStampImg
	{
		double   m_dTs;
		IplImage *m_pImg;

	}MvTimeStampImg;
#endif


#endif  //#ifndef __ANALYSIS_BASE_H