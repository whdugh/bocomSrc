// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef __BASESTRUCT__H
#define __BASESTRUCT__H

#ifndef N_REF
	#if 1
		#define N_REF 50  //每条轨迹的记录的帧数for CvSet
	#else
		#define N_REF 500  //为绘制结果临时改的
	#endif
#endif

#ifndef TRDISAPPEARPTNUM
	#define  TRDISAPPEARPTNUM 20     //保留轨迹消失点的数目
#endif



#ifndef SIFT_WIN_SIZE
	#define SIFT_WIN_SIZE    4
#endif
#ifndef SIFT_BINS
	#define SIFT_BINS        8
#endif
#ifndef SIFT_DIMS
	#define SIFT_DIMS (SIFT_WIN_SIZE*SIFT_WIN_SIZE*SIFT_BINS) //128 
#endif



#define MAXDISTANCE     10  //7 //求真实方向，用到的两点之间图像中的最小距离
#define  SIFT_POINT_NUM   10
#define  MAX_VALIDTRACKS_IN_VEHICLE  120

#ifndef MAXSPLOBJNUM
#define  MAXSPLOBJNUM  100   //一大车最多对应多少个小目标
#endif

#ifndef MAX_TRACKNUM_OFBIGV
#define MAX_TRACKNUM_OFBIGV  1000
#endif

#ifndef MAX_HISTORY_NUM
#define MAX_HISTORY_NUM 50
#endif

#ifndef MAX_HISTORY_NUM_BACKGROUND
#define MAX_HISTORY_NUM_BACKGROUND   50
#endif

#ifndef MAX_TRACKS_IN_VEHICLE
	#define MAX_TRACKS_IN_VEHICLE	160  // 80 每辆车包含最多轨迹数目(原为80，由贺岳平改为120，后再改为160)
#endif


#ifndef MAX_LINE3COM_NUM
	#define  MAX_LINE3COM_NUM 10000
#endif

#ifndef FOREBINNUM_OFTYPE
	#define  FOREBINNUM_OFTYPE 9		//在车类型判断过程中前景bin数目
#endif
#ifndef MAX_LINENUM_OFTYPE
	#define  MAX_LINENUM_OFTYPE 10   //在车类型判断过程中线段数目限制
#endif
#ifndef SOBELBINNUM_OFTYPE
	#define  SOBELBINNUM_OFTYPE 9		//在车类型判断过程中sobell的bin数目
#endif
#ifndef MVTRBINNUM_OFTYPE
	#define  MVTRBINNUM_OFTYPE 9		//在人类型判断过程中运动轨迹的bin数目
#endif

#ifndef	MAXNUM_SPLPEOPLE
	#define  MAXNUM_SPLPEOPLE 10		//人最多可合并的数目
#endif
#ifndef MAXNUM_SPLSMLVEH
	#define  MAXNUM_SPLSMLVEH  20		//小车最多可合并的数目
#endif
#ifndef MAXNUM_SPLBIGVEH
	#define  MAXNUM_SPLBIGVEH 40			//大车最多可合并的数目
#endif

#include "ConfigStruct.h"
#include <vector>

//结构体在作为类的成员时，最好能在程序初始化的时候重新设置初值
typedef struct edge
{
	float w;
	int   a, b;
	edge( ) 
	{
		w=100;
	}
} edge;


typedef struct MyTrackPoint
{
	CvPoint2D32f	point;
	CvPoint2D32f	world_coord;
	CvPoint2D32f	world_coord_s;
//	uchar			descriptor[SIFT_DIMS];
	short			is_match;  //1匹配上,0为角点，-1未匹配上
	bool			is_estimate;
	double		    ts;        //时间戳，秒
}MyTrackPoint;



#define RECFRAMENUM 2
#define RECFRAMEDESC 1
#define RECFRAMEDESC1 0
typedef struct MyTrack
{
	MyTrackPoint	track_point[N_REF+1];      //for CvSet
	uchar			recDescriptor[RECFRAMENUM][SIFT_DIMS];  //debug
	int				length;	                  //从0开始计
	bool			is_delete;                //是否被删除的标记
	int				is_use;	
	CvPoint2D32f	velocity;         //近期内的平均速度
	float			velo1;                     //近期速度的绝对值
	float			orientation;             //图像坐标中总体方向
	float			ori_image;              //图像坐标中的方向(较短轨迹不进行计算)
	int				group_index;          //group后所属组
	int				vehicle_index;         //最终所属组
	float           fBgNcc;                  //与背景的自相关系数值
	bool            is_bg;                     //是否作为背景点,qiansen
	int				is_jam;                   //move_counter可以删了
	int				stop_to_move;       //由静止到运动,针对拥堵而言
	int				determined;            //轨迹的可疑性//2:default; 3:doubtful;
	bool			is_arrive_line;         //是否已经过了流量线

	bool         is_shadow;           //是否为阴影所造成的
	double       ts_isShadow;         //判断为阴影时的时间

	bool         is_light;           //是否为灯光所造成的
	double       ts_isLight;         //判断为灯光时的时间

	CvPoint2D32f  pt_arrive;      //已过的流量线的中心
		
	float       realworldori;    //世界坐标的角度
	float       realimageori;       //图像坐标的角度
	
	int    app_num;                    //轨迹出现的次数
	bool   bCornerIsBg;  

	int    nRoadID;                   //所处道路区域的索引号
	float  avgVelo1;                  //平均速度
	unsigned long   nTrId;  

	double          ts_bigV;   //上次判断为大车轨迹的时刻
	unsigned long   nbigVId;   //所属的大车ID号
	CvPoint2D32f    xyRatio;   //轨迹点到质心相对于maxsizeXy的比率

	double         ts_3DModel;  //上次判断为3D模型轨迹的时刻
	unsigned long  n3DModelId;  //所属的3D模型ID号

	CvPoint2D32f  ltPt;        //轨迹所到的最左和最上位置
	CvPoint2D32f  rbPt;		   //轨迹所到的最右和最下位置

	int         nMaxImgDistOfCalLen;   //在指定帧数内该轨迹的最长位移
	int         nMinImgDistOfCalLen;   //在指定帧数内该轨迹的最短位移
	int         nMaxWorDistOfCalLen;   //在指定帧数内该轨迹的最长世界位移
	int         nMinWorDistOfCalLen;   //在指定帧数内该轨迹的最短世界位移
#ifdef USE_FANTATRACK
	double      ts_isFantastic;        //认为是奇异轨迹时的时间戳
	double      ts_isFantCross;        //认为是奇异横穿轨迹的时间戳
#endif

	CvPoint2D32f	ptADisappear[TRDISAPPEARPTNUM];  //消失点的坐标
	double		    tsADisappear[TRDISAPPEARPTNUM];  //消失点的时间戳
	bool            bMove2Stop;       //由动到静
	bool            bRecStop4M2S;     //最近静止但对应由动到静
	bool            bStopCarPt;       //为静止车上轨迹对应的点

	int             nFkLabel;         //当前的前景label值

	void initMyTrack( )
	{
		is_shadow = false;
		is_light = false;
		ts_isShadow = -1000;
		ts_isLight = -1000;

		is_use = 1;
		length = 0;
		is_bg = false;
		is_delete = false;//第一次进入,标记为不能删除

		vehicle_index = -1;
		is_jam = false;
		stop_to_move = false;
		determined = 2;
		velo1 = 0;
		velocity.x = 0;
		velocity.y = 0;

		is_arrive_line = false;
		pt_arrive.x = -1000;
		pt_arrive.y = -1000;

		realworldori = -1.0f;
		realimageori = -1.0f;

		//轨迹运动的边界矩形
		ltPt.x =  10000.0f, ltPt.y =  10000.0f;
		rbPt.x = -10000.0f, rbPt.y = -10000.0f;		 

		nMaxImgDistOfCalLen = -10000;
		nMinImgDistOfCalLen = 10000;
		nMaxWorDistOfCalLen = -10000;
		nMinWorDistOfCalLen = 10000;
			
		bMove2Stop = false;
		bRecStop4M2S = false;
		bStopCarPt = false;
		fBgNcc = -1.0f;    

		nFkLabel = -1;
	}
}MyTrack;   //把每个点的轨迹做成一个结构体


typedef struct _CentroidHistory
{
	int					length;                        //从0开始计
	CvPoint2D32f		centroid[N_REF+1];	  //质心坐标
	float				velo[N_REF+1];        //目标速度
	float				ori[N_REF+1];
	double         		ts[N_REF+1];            //时间戳
	bool				is_jam[N_REF+1];
	
	CvPoint2D32f		left_top[N_REF+1];				//质心坐标
	CvPoint2D32f		right_bottom[N_REF+1];		//质心坐标
	int					size[N_REF+1];					//车的轨迹数目
	CvPoint2D32f		veloPt[N_REF+1];				//目标速度矢量
	
	_CentroidHistory( )
	{
		length = 0;
	};
}CentroidHistory;

#ifndef OBJECTCOLOR
	#define OBJECTCOLOR
	typedef struct OBJECT_COLOR
	{
		int  nColor1;       //颜色1
		int  nColor2;	    //颜色2
		int  nColor3;       //颜色3

		int  nWeight1;    //颜色权重1
		int  nWeight2;    //颜色权重2
		int  nWeight3;    //颜色权重3	
	} object_color;
#endif


typedef struct _GroupingResult
{
	int		size;         
	short	track_index[MAX_TRACKS_IN_VEHICLE];
	CvPoint2D32f   left_top;
	CvPoint2D32f   right_bottom;
	int     nBestTrNo;
	int     nSecTrNo;
	_GroupingResult( )
	{
		size = 0;	
		left_top = cvPoint2D32f( 0.0f, 0.0f );
		right_bottom = cvPoint2D32f( 0.0f, 0.0f );
		nBestTrNo = -1;
		nSecTrNo = -1;
	}

}GroupingResult;

typedef struct _MyGroup
{	
	int		size;                          //从1开始计
	short   track_index[MAX_TRACKS_IN_VEHICLE];//限定每个group最多包含80条轨迹

	CentroidHistory  centroid_history;

	CvPoint2D32f	centroid, jam_centroid;                                 
	CvPoint2D32f	left_top, right_bottom;
	CvPoint2D32f	left_top_all, right_bottom_all;
	CvPoint2D32f	left_top_all45, right_bottom_all45;
	float			width;
	float			height;
	float			width45;                       //45度旋转方向的width
	float			height45; 
	
	CvPoint2D32f	w_centroid;                   //对应世界坐标
	CvPoint2D32f	w_left_top,w_right_bottom;
	CvPoint2D32f	w_left_top_all,w_right_bottom_all;
	float			w_width;
	float			w_height;

	CvPoint2D32f	velocity;                   //近期速度
	float			velo1;                           //近期速度的绝对值
	float			orientation;                      //图像坐标方向
	float			ori_image;                       //图像坐标中的方向(较短轨迹不进行计算)
	double		force_reserve_ts;
	int				determined;                    //0:not; 1:is; 2:default; 3:doubtful;
	int				pass_line;	                     //flag to indicate whether the vehicle has been counted as passing a detection line
								                         //we support a maximum of 4 traffic flow lines per channel
	bool			is_estimate;                   //整辆车是否全为估计点
	bool			is_jam;                          //车辆是否为拥堵
	bool			stop_to_move;              //由静止到运动,针对拥堵而言
	int				vehicle_history_index;
	bool			is_large;
	object_color    color;
	int				type;                             //车辆类型：车=0 人=1

	bool			is_line_jam;                   //一直静止的车根据线段判断出的拥堵
	int				line_num;                      //判静止车时的线段计数

	int             nChannel;                     //存放所在车道序号
	int	        	nChannel_queue;           //属于某车道的车，包含可疑的车

	//事件报警所用时间戳
	double ts_stop;     
	double ts_stopconfirm;     
	double ts_alert_stop;
	double ts_line_num;             //for stopped vehicle detection by burns line
	double ts_line_num_pre;      //for stopped vehicle detection by burns line

	double ts_against;
	double ts_drop;
	double ts_passerby;
    double ts_fast;              //单独车速
	double ts_slow;
    double ts_change;
	double ts_pressLine;			 //压黄线
	double ts_pressLeadstreamLine;   //压导流线

	double ts_beyondMark;        //越界
	double ts_wrong_chan;        //非机动车道
	double ts_objAppear;
	double ts_person_appear;
	double ts_bargeIn;            //闯入
	double ts_personRun;          //行人奔跑

	 //事件确认标志，需检出两次才能报
	 bool   bConfirmPasserby;		
	 bool   bConfirmAgainst;
	 bool   bConfirmFast;
	 bool   bConfirmSlow;
	 bool   bConfirmPersonRun;   //确认行人奔跑
	 
	double ts_last_split;               //add by hyp
	double ts_last_merge;            //add by hyp
	double ts_addToSet;             //add by hyp

	double  ts_merge_split_group;       //add by hyp on 20090811
	double  ts_smallVAppear;
	double  ts_bigVAppear;
	double  ts_isBigVehicle;

	double dHogDetectAgainThres;
	double ts_hogDetect;
	double ts_sure_type;     
	double ts_good_history;  
	double ts_no_light;     
	double ts_detectCarNum;
	double ts_maySplObjOfBigV;


	int    m_nSizeTotal;               //add by hyp
	int    m_nAppearTime;

	bool   bShow;	                    //是否是停留显示：1表示停留显示事件；0表示真实事件
	CvPoint2D32f pt_arrive;				//上次通过的流量线的中点

	bool   history_appear;           //行人出现的次数
	int    nAppearTime;
	int    noMobAppearTime;

	bool   bNeedCheckSizeRationality;   //对group是否需要进行大小的合理性判断
	int    nIsMotorScore;                        //为机动车的得分   
	int    nIsPeopleScore;                       //为行人的得分
	bool   bHogPeople;
	bool   bRecSureVehicle;                  //近期是否判断过为确定车辆
	bool   bRecSurePerson;                   //近期是否判断过为确定行人

	int    nNoJoinEvent;                        //不参入事件，每帧均要判断一下
	
	float  centroid_X;   
	float  centroid_Y;   

    long   nEventId;       //事件的Id
	bool   bHaveCarnum;    //是否存在车牌
    CARNUM_CONTEXT_DEF  carNumStruct;  //车牌信息结构体

    unsigned long   nVehicleId;     //目标的ID

	int    nAppAlertType;           //目标出现报警的类型 0-为真，1-为需再次考虑，2-为需慎重考虑
	
	int m_nKeepTimes;

	_MyGroup()
	{
		 size = 0;                  
	//	short   track_index[MAX_TRACKS_IN_VEHICLE];//限定每个group最多包含80条轨迹
	//	CentroidHistory  centroid_history;

		centroid = cvPoint2D32f( 0.0f, 0.0f );
		jam_centroid = cvPoint2D32f( 0.0f, 0.0f );
		left_top = cvPoint2D32f( 0.0f, 0.0f );
		right_bottom = cvPoint2D32f( 0.0f, 0.0f );
		left_top_all = cvPoint2D32f( 0.0f, 0.0f );
		right_bottom_all = cvPoint2D32f( 0.0f, 0.0f );
		left_top_all45 = cvPoint2D32f( 0.0f, 0.0f );
		right_bottom_all45 = cvPoint2D32f( 0.0f, 0.0f );
		width = 0.0f;
		height = 0.0f;
		width45 = 0.0f;
		height45 = 0.0f;

		w_centroid = cvPoint2D32f( 0.0f, 0.0f );                   
		w_left_top= cvPoint2D32f( 0.0f, 0.0f );
		w_right_bottom= cvPoint2D32f( 0.0f, 0.0f );
		w_left_top_all= cvPoint2D32f( 0.0f, 0.0f );
		w_right_bottom_all= cvPoint2D32f( 0.0f, 0.0f );
		w_width = 0.0f;
		w_height = 0.0f;

		velocity = cvPoint2D32f( 0.0f, 0.0f );
		velo1 = 0.0f;
		orientation = -1.0f;
		ori_image = -1.0f;

		force_reserve_ts = -1;
		determined = 2;

		pass_line = -1;	

		is_estimate = false;
		is_jam = false;
		stop_to_move = false;
	
		vehicle_history_index = -1;
		is_large = false;

		color.nColor1 = -1,	 color.nColor2 = -1,  color.nColor3 = -1;
		color.nWeight1 = -1, color.nWeight2 = -1, color.nWeight3 = -1;

		type = 0;	

		is_line_jam = false;
		line_num = 0;

		nChannel = -1;                 //存放所在车道序号
		nChannel_queue = -1;           //属于某车道的车，包含可疑的车

		//事件报警所用时间戳
		ts_stop = -1000.0;     
		ts_stopconfirm = -1000.0;     
		ts_alert_stop = -1000.0;
		ts_line_num = -1000.0;             //for stopped vehicle detection by burns line
		ts_line_num_pre = -1000.0;      //for stopped vehicle detection by burns line

		ts_against = -1000.0;
		ts_drop = -1000.0;
		ts_passerby = -1000.0;
		ts_fast = -1000.0;               //单独车速
		ts_slow = -1000.0;
		ts_pressLine = -1000.0;
		ts_pressLeadstreamLine = -1000.0;
		ts_change = -1000.0;
		ts_beyondMark = -1000.0;      //越界
		ts_wrong_chan = -1000.0;        //非机动车道
		ts_objAppear = -1000.0;
		ts_person_appear = -1000.0;
		ts_bargeIn = -1000.0;            //闯入
		ts_personRun = -1000.0;          //行人奔跑

		//事件确认标志，需检出两次才能报
		bConfirmPasserby = false;		
		bConfirmAgainst = false;
		bConfirmFast = false;
		bConfirmSlow = false;
		bConfirmPersonRun = false;   //确认行人奔跑

		ts_last_split = -1000.0;        //add by hyp
		ts_last_merge= -1000.0;         //add by hyp
		ts_addToSet= -1000.0;           //add by hyp

		ts_merge_split_group = -1000.0;      //add by hyp on 20090811
		ts_smallVAppear = -1000.0;
		ts_bigVAppear = -1000.0;
		ts_isBigVehicle = -1000.0;

		dHogDetectAgainThres = 1000.0;
		ts_hogDetect = -1000.0;
		ts_sure_type = -1000.0;    
		ts_good_history = -1000.0; 
		ts_no_light = -1000.0;   
		ts_detectCarNum = -1000.0;
		ts_maySplObjOfBigV = -1000.0;


		m_nSizeTotal = 0;               //add by hyp
		m_nAppearTime = 0;

		bShow = false;	            //是否是停留显示：1表示停留显示事件；0表示真实事件
		pt_arrive = cvPoint2D32f(-1000.0f, -1000.0f);		//上次通过的流量线的中点

		history_appear = false;           //行人出现的次数
		nAppearTime = 0;
		noMobAppearTime = 0;

		bNeedCheckSizeRationality = true;   //对group是否需要进行大小的合理性判断

		nIsMotorScore = 0;                        //为机动车的得分   
		nIsPeopleScore = 0;                       //为行人的得分

		bHogPeople = false;
		bRecSureVehicle = false;                  //近期是否判断过为确定车辆
		bRecSurePerson = false;                   //近期是否判断过为确定行人

		nNoJoinEvent = 0;                        //不参入事件，每帧均要判断一下

		centroid_X = 1.0f;   
		centroid_Y = 1.0f;   

		nEventId = -1;       //事件的Id
		bHaveCarnum = false; //是否存在车牌
		carNumStruct;        //车牌信息结构体

		nVehicleId = 0;     //目标的ID

		nAppAlertType = 0;  //目标出现报警的类型 0-为真，1-为需再次考虑，2-为需慎重考虑
		m_nKeepTimes = 0;
	}

	CvRect mvGetRect( )
	{
		return cvRect( (int) left_top.x, (int) left_top.y,
					   (int) width, (int) height );
	}

}MyGroup;

//动态结构
struct MyTrackElem : CvSetElem
{
	MyTrack track;
};

struct MyVehicleElem : CvSetElem
{
	MyGroup group;
};


typedef struct MegerLine{
       float	x1;
	   float	y1;
	   float	x2;
	   float	y2;
	   float	ori;					//0-180
	   float	length;
	   float	dRatio;				//相对maxsizeXY的大小
	   bool		bMerge;			//合并
	   bool		bBeMerge;		//被合并
	   int			type;					//0-水平线，1-竖直线，2-斜线
	   bool		bUsed;				//已经使用
	   bool		bEdge;				//是否是边
	   bool		bStop;				//是否为静止的线
	
	   float      fOri;					//线段的角度
	   CvPoint2D32f  ptVelo;   //线段的速度

}MegerLine;


typedef struct SubAreaStr{
		
		int    v_idx;  //区域所对应的vehicle的ID
		float  fObjImgMvOri;

		//区域的左上角和右下角的坐标
		int ltx;
		int lty;
		int rbx;
		int rby;

		//区域所对应的vehicle的左上角和右下角的坐标
		int rect_x1;
		int rect_y1;
		int rect_x2;
		int rect_y2;

		//水平线段，垂直线段和其他角度的线段（条数及序号）
		int nHLineNum;
		int nVLineNum;
		int nOLineNum;
		int *h_LineID;
		int *v_LineID;
		int *o_LineID;

    	//长的水平线段，垂直线段和其他角度的线段（条数及序号）
		int nLongHLineNum;
		int nLongVLineNum;
		int nLongOLineNum;	
		int *h_longLineID;
		int *v_longLineID;
		int *o_longLineID;

		//水平线段，垂直线段和其他角度线段的长度和
		float fHLineSumLen;
		float fVLineSumLen;
		float fOLineSumLen;

		//其他信息
		int exist_obj_type;      //0-不存在车,1-存在小车,2-存在大车
		int nExist_vehicle_num;  //存在的vehicle的数目
		int hLine_score[3][3];   //水平线数目的投票
		int diff_percent[3][3];  //差分图白点的百分比

}SubAreaStr;

typedef struct StopObjStr{
	int             vehicle_index;
	double          ts_stop;
	CvPoint2D32f	left_top;
	CvPoint2D32f    right_bottom;	
	int             cornerNum;
	CvPoint2D32f    *cornerPt;
	uchar           **descriptor;
	int             nStop[50];   //最多存50次匹配上的记录
}StopObjStr;



typedef struct _ParameterConfigStr{
		double  thres_minDis_calTrDir;    //求轨迹的世界和图像方向角度时,两点的最小长度
        double  thres_cross_x;				   //横穿时要求最长的轨迹的x方向世界坐标的最小位移
		double  thres_cross_modify;		   //横穿时轨迹的x方向世界坐标的最小位移
		double  thres_against_y;			   //逆行时要求最长的轨迹的y方向世界坐标的最小位移
		double  thres_changle_scale1;
	    double  thres_changle_scale2;
		_ParameterConfigStr( )
		{
			thres_minDis_calTrDir = 10;
			thres_cross_x = 1.5;    //1.5
			thres_cross_modify = 1.0;
			thres_against_y = 2.0;  //1.5
			thres_changle_scale1 = 0.1;  
	        thres_changle_scale2 = 0.05;
		}
}ParameterConfigStr;

typedef struct _RoadAreaStruct{

	int    nRoadNum;      //道路区域的数目

	int    *nRoadPtNumA;  //道路区域的顶点数目

	CvPoint2D32f **pRoadPtA;   //道路区域的顶点

	int    *nRoadKind;  //道路的方向

}RoadAreaStruct;


typedef struct _KeyPtSiftFeat 
{
	CvPoint2D32f  ptPos;               // 位置坐标
	uchar cSiftVal[SIFT_DIMS];    // SIFT描述符
	_KeyPtSiftFeat( )
	{
		ptPos  = cvPoint2D32f(0.0f, 0.0f);
	}

}KeyPtSiftFeat;

//丢弃物结构
typedef struct _DropObjStruct
{
	int           nCannelId;
	double        ts_add;
	double        ts_drop;
	long          lAddFramNo;

	CvPoint2D32f  cet_pt;
	CvPoint2D32f  lt_pt;
	CvPoint2D32f  rb_pt;

	double dNCC1;
	double dNCC2;
	bool          bTrue;
	_DropObjStruct( )
	{
		nCannelId = -1;
		ts_add   = -1000;
		lAddFramNo = -10000;
		ts_drop  = -1000;
		cet_pt.x = -1000;
		cet_pt.y = -1000;
		lt_pt.x  = -1000;
		lt_pt.y  = -1000;
		rb_pt.x  = -1000;
		rb_pt.y  = -1000;
		dNCC1    = -1;
		dNCC2	 = -1;
		bTrue  = false;
	}
}DropObjStruct;

typedef struct _DropRect
{
	CvRect rt;
	bool bHit;
	int nTimes;
	_DropRect()
	{
		bHit = false;
		nTimes = 0;
	}
}DropRect;

//可能的大车和分碎目标的信息
typedef struct _mayBigSplVeh_InfoStruct 
{
	int      nBigVehId;    //所对应的大车目标的Id
	float    fImgMvOri;	   //所对应的大车目标的图像运动方向 	
	CvPoint  trLt_pt;
	CvPoint  trRb_pt;
	CvPoint  bus_lt_pt;
	CvPoint  bus_rb_pt;
	int      nSameOriTrNum;
	int     *nASameOriTrId;

	CvPoint  bigV_line_lt_pt;
	CvPoint  bigV_line_rb_pt;

	bool     bExistSplObj;                
	int      nSplObjNum;
	int      nASplObjId[MAXSPLOBJNUM];

	bool     bExistSpl2Obj;
	int      nSpl2ObjNum;
	int      nASpl2ObjId[MAXSPLOBJNUM];
		
	CvPoint  car_maxsize_pt;  //小车的maxsize
	CvPoint  car_lt_pt;       //小车的左上角点
	CvPoint  car_rb_pt;       //小车的右下角点
	
	CvPoint  bigV_maxsize_pt;  //大车的maxsize
	CvPoint  bigV_lt_pt;       //大车的左上角点
	CvPoint  bigV_rb_pt;       //大车的右下角点
	
	_mayBigSplVeh_InfoStruct( )
	{
		nBigVehId = -1;
		fImgMvOri = -1;			
		trLt_pt = cvPoint( 0, 0 );
		trRb_pt = cvPoint( 0, 0 );
		bus_lt_pt = cvPoint( 0, 0 );
		bus_rb_pt = cvPoint( 0, 0 );
		nSameOriTrNum = 0;
		nASameOriTrId = NULL;
		nASameOriTrId = new int[MAX_TRACKNUM_OFBIGV];

		bigV_line_lt_pt = cvPoint( 0, 0 );
	    bigV_line_rb_pt = cvPoint( 0, 0 );

		bExistSplObj = false;
		nSplObjNum = 0;

		bExistSpl2Obj = false;
		nSpl2ObjNum = 0;

		car_maxsize_pt = cvPoint( 0, 0 );
		car_lt_pt = cvPoint( 0, 0 );
		car_rb_pt = cvPoint( 0, 0 );

		bigV_maxsize_pt = cvPoint( 0, 0 );
		bigV_lt_pt = cvPoint( 0, 0 );
		bigV_rb_pt = cvPoint( 0, 0 );	
	}

}mayBigSplVeh_info;


typedef struct _rectangle_1LH2V 
{
	int nlongHLId;
	int nVLId1;
	int nVLId2;
}rectangle_1LH2V;


typedef struct _rectangle_1LV2H 
{
	int nlongVLId;
	int nHLId1;
	int nHLId2;
}rectangle_1LV2H;

typedef struct _switch3_1H1V1O 
{
	int nHLId;
	int nVLId;
	int nOLId;
}switch3_1H1V1O;

typedef struct _switch3_1H1V1X 
{
	int nHLId;
	int nVLId;
	int nXLType;  //第三条线的类别(1斜线 2竖线)
	int nXLId;    //第三条线的序号
}switch3_1H1V1X;


typedef struct _parallel_OLine
{
	int nOLId1;
	int nOLId2;
}parallel_OLine;


typedef struct _lineCom_result 
{
	int  nRect1LH2VNum;
	rectangle_1LH2V *ARect1LH2V;

	int  nRect1LV2HNum;
	rectangle_1LV2H *ARect1LV2H;

	int  nSwitch3HVONum;
	switch3_1H1V1O  *ASwitch3HVO;

	int  nParallONum;
	parallel_OLine  *AParallO;

	int  nLineLeftX;  
	int  nLineRightX;
	  
	int  nLineTopY;
	int  nLineBottomY;

	_lineCom_result( )
	{
		nRect1LH2VNum = 0;
	    ARect1LH2V = NULL;

	    nRect1LV2HNum = 0;
	    ARect1LV2H = NULL;

	    nSwitch3HVONum = 0;
	    ASwitch3HVO = NULL;

		nParallONum = 0;
	    AParallO = NULL;

		nLineLeftX  =  10000;
		nLineRightX = -10000;
	  
		nLineTopY    =  10000;
		nLineBottomY = -10000;
	}

}lineCom_result;


//检测到的3边组合结构体类型
enum LINE3COMTYPE
{	
	SWITCH_HVO = 0,  	
	SWITCH_VHO,  
	SWITCH_OHV,

	SWITCH_HVV,  //3
	SWITCH_HVH,

	VERTI_1H2V,   //5
	VERTI_1V2H,

	VERTI_1H2O,  //7
	VERTI_1V2O,
	VERTI_1O2H,
	VERTI_1O2V,
	
	PARAL_3H,   //11
	PARAL_3V,
	PARAL_3O
};


//线段组合的三元结构
typedef struct _Line3Com 
{	
	int		nType;			//三叉体的类型(0:hvo,1:hvv,2:hvh)   //线段组合的垂直结构
	int		nL1No;			//第1边的序号
	int		nL2No;			//第2边的序号
	int		nL3No;			//第3边的序号
	CvPoint intersectPt;	//三叉体相交点
	bool    bNeedMeger;     //是否应合并组合体附近的轨迹
	bool    bTrueInBigV;	//三元结构是否在大车上
	CvPoint lt_pt;
	CvPoint rb_pt;

	_Line3Com( )
	{
		nType = -1;  
		nL1No = -1;
		nL2No = -1;
		nL3No = -1;
		intersectPt  = cvPoint( -10000,  -10000);
		bNeedMeger = false;
		bTrueInBigV = false;   	
		lt_pt = cvPoint( 10000,  10000);
		rb_pt = cvPoint(-10000, -10000);
	}
}Line3Com;


typedef struct MvTrStruct 
{
	int				nTrNo;     //对应的轨迹序号
	CvPoint    nowPt;     //该轨迹的当前点
}mvTrStruct;


//对目标类型属性修改的方式
enum MODIFY_OBJTYPEPROPERTY_MOD
{
	SURE_VEHICLE_TYPE_MOD	= 0,          //确定的车
	SURE_PEOPLE_TYPE_MOD,					//确定的人
	VEHICLE_TYPE_MOD,                             //车
	PEOPLE_TYPE_MOD,					            //人
	REDUCE_TYPE_SCORE_MOD,				//减少目标的得分
	SURETYPE_AFFECTTIME_MOD,            //在肯定类型的影响时间范围
	OTHER_TYPE_MOD						
};


//确定为车辆的判断方法
enum SUREVEHICLEMETHOD
{
	VEH_HORILINE = 0,				//水平线段
	VEH_VERTLINE,				    //竖直线段
	VEH_ORILINE,				    //斜线段

	VEH_COMLINE,				    //组合线段

	VEH_SOBELDIS1,					//Sobel分布1                  
	VEH_SOBELDIS2,					//Sobel分布2
	VEH_SOBELDIS3,					//Sobel分布3

	VEH_LAMPDETECT,					//车灯检测
	VEH_OTHERMETHOD                 //其他方法
};

//确定为行人的判断方法
enum SUREPEOPLEMETHOD
{
	PEO_FOREIMG	= 0,						//前景分布
	PEO_FORECONTOURS,					    //前景及contour                  
	PEO_VAILDMVTR1,							//运动轨迹分布1
	PEO_VAILDMVTR2,							//运动轨迹分布2
	PEO_VAILDMVTR3,							//运动轨迹分布3
	PEO_HOGDETECT,                          //HOG检测
	PEO_OTHERMETHOD                         //其他方法
};

//获取需进行调整合并的对象的模式
enum GET_NEEDADJUST_OBJECTS_MOD
{
	ADJUST_PEOPLE	= 0,			//分碎的行人
	ADJUST_SMLVEH,					//分碎的小车        
	ADJUST_BIGVEH					//分碎的大车
};

//获取需进行调整合并的对象的模式
enum CANDIPEOCETMOD
{
	CANDIPEOISALONE	= 1,			//单个行人
	CANDIPEOISMULTI					//多个行人
};

//行人区域信息
typedef struct _candPeoAreaInfo 
{
	CvPoint    peoExLtPt;				//人区域的左上角
	CvPoint    peoExRbPt;				//人区域的右下角
	CvPoint    peo2ExLtPt;				//两并排人区域的左上角
	CvPoint    peo2ExRbPt;			//两并排人区域的右下角
}candPeoAreaInfo;  

//目标类型判断的信息
typedef struct _object_typeJudge_info 
{
	bool       bVaild;                                      //该信息是否有效
	int        nObjId;										//目标的Id
	double     tsStamp;                                     //时间戳
	float      fImgMvAngle;									//图像运动角度
	CvPoint    centerPt;                                    //目标中心点
	
	CvPoint    peoMaxsizeXyPt;								//人maxsizeXy大小
	CvPoint    carMaxsizeXyPt;								//车maxsizeXy大小
	CvPoint    carAngleMaxsizeXyPt;							//车角度maxsizeXy大小

	CvPoint    peoLtPt;										//人区域的左上角
	CvPoint    peoRbPt;										//人区域的右下角
	int        nPeoAvgGray;                                 //人区域的平均亮度

	CvPoint    carLtPt;										//车区域的左上角
	CvPoint    carRbPt;										//车区域的右下角
	int        nCarAvgGray;                                 //车区域的平均亮度

	int        nCandPeoCetNum;                              //候选的行人中心的数目
	int        candPeoCetMod[10];                           //候选的行人中心的模式
	CvPoint    candPeoCetPt[10];                            //候选的行人中心

	CvPoint    peo2W1HExLtPt;								//两人宽1人高的扩展区域的左上角
	CvPoint    peo2W1HExRbPt;								//两人宽1人高的扩展区域的右下角

	bool		bVaildVPeoExAreaForeRate;					//行人扩展区域按垂直方向前景比率是否有效
	float		fAVPeoExAreaForeRate[FOREBINNUM_OFTYPE];    //行人扩展区域按垂直方向前景比率

	bool		bVaildHPeoExAreaForeRate;					//行人扩展区域按水平方向前景比率是否有效
	float       fAHPeoExAreaForeRate[FOREBINNUM_OFTYPE];    //行人扩展区域按水平方向前景比率

	int         nPeoAreaVSobelSum;							 //人区域竖直sobel的和
	float		fAColPeoAreaVSobelRate[SOBELBINNUM_OFTYPE];	 //行人扩展区域按垂直方向的竖直sobel比率
	float		fARowPeoAreaVSobelRate[SOBELBINNUM_OFTYPE];	 //行人扩展区域按水平方向的竖直sobel比率
	int         nPeoAreaHSobelSum;							 //人区域水平sobel的和
	float		fAColPeoAreaHSobelRate[SOBELBINNUM_OFTYPE];	 //行人扩展区域按水平方向的水平sobel比率
	float		fARowPeoAreaHSobelRate[SOBELBINNUM_OFTYPE];	 //行人扩展区域按水平方向的水平sobel比率
	int         nCarAreaVSobelSum;							 //车区域竖直sobel的和
	float		fAColCarAreaVSobelRate[SOBELBINNUM_OFTYPE];	 //车区域按垂直方向的竖直sobel比率
	float		fARowCarAreaVSobelRate[SOBELBINNUM_OFTYPE];	 //车区域按水平方向的竖直sobel比率
	int         nCarAreaHSobelSum;							 //车区域水平sobel的和
	float		fAColCarAreaHSobelRate[SOBELBINNUM_OFTYPE];	 //车区域按水平方向的水平sobel比率
	float		fARowCarAreaHSobelRate[SOBELBINNUM_OFTYPE];	 //车区域按水平方向的水平sobel比率

	int			nHLineNum;									//水平线段条数
	int			nAHLineNo[MAX_LINENUM_OFTYPE];				//水平线段序号
	int			nVLineNum;									//竖直线段条数
	int			nAVLineNo[MAX_LINENUM_OFTYPE];				//竖直线段序号
	int			nOLineNum;									//斜线段条数
	int			nAOLineNo[MAX_LINENUM_OFTYPE];				//斜线段序号

	int			nAComLineCnt[3][2][6];		 //1:平行/垂直/相交,2:max/sum,3：HH/HV/HO/VV/VO/OO

	int			nPeoAreaMvTrSum;							//人区域运动轨迹数目和
	int			nCarAreaMvTrSum;							//车区域运动轨迹数目和
	int			nCarAreaColBinMvTrNum[MVTRBINNUM_OFTYPE];	//车区域各列bin区域内的运动轨迹数
	int			nCarAreaRowBinMvTrNum[MVTRBINNUM_OFTYPE];	//车区域各行bin区域内的运动轨迹数

	int         nSureType;									//确定的类型
	int         nDefaultType;                               //默认的类型  

	bool        bASureVehicle[10];
	bool        bASurePeople[10];
	
	bool        bFkImgSuitPeoWitdh;                         //前景满足为人的宽度


	CvPoint     hogAreaLtPt;								//hog行人检测区域的左上点
	CvPoint     hogAreaRbPt;								//hog行人检测区域的右下点
	int         nHogPeoNum;                                 //检测到的HoG行人窗口个数
	float       fAHogPeoVal[10];                            //检测到的HoG行人的得分
	CvPoint     AHogPeoLtPt[10];                            //检测到的HoG行人的左上点
	CvPoint     AHogPeoRbPt[10];                            //检测到的HoG行人的右下点

	int         nDetectedHogLevel[2];                       //Hog检测的可信层次
	CvRect      detectedHogPeoRect[2];						//Hog检测到的行人区域

	_object_typeJudge_info( ) { 
		mvInitVar( );
	}

	void mvInitVar( )
	{
		bVaild = false;
		nObjId = -1;
		tsStamp = -1000.0;

		int  n, i, j; 
		for( n=0; n<3; n++ ){
			for( i=0; i<2; i++ ){
				for( j=0; j<6; j++ ){
					nAComLineCnt[n][i][j] = 0;
				}
			}
		}

		for( n=0; n<10; n++ ){
			bASureVehicle[n] = false;
		}

		for( n=0; n<10; n++ ){
			bASurePeople[n] = false;
		}

		bFkImgSuitPeoWitdh = false;

		nHogPeoNum = 0;
	}

}object_typeJudge_info;  


//事件检测前目标的信息
typedef struct _objInfo_befResult 
{
	bool         bVaild;                                        //该信息是否有效
	int          nObjId;										//目标的Id

	float      fImgMvAngle;											//图像运动角度
	CvPoint    centerPt;                                            //目标中心点

	CvPoint    peoMaxsizeXyPt;										//人maxsizeXy大小
	CvPoint    carMaxsizeXyPt;										//车maxsizeXy大小
	CvPoint    carAngleMaxsizeXyPt;									//车角度maxsizeXy大小

	CvPoint    peoLtPt;												//人区域的左上角
	CvPoint    peoRbPt;												//人区域的右下角
	CvPoint    carLtPt;												//车区域的左上角
	CvPoint    carRbPt;												//车区域的右下角

	CvPoint    peo2W1HExLtPt;										//两人宽一人高的扩展区域的左上角
	CvPoint    peo2W1HExRbPt;										//两人宽一人高的扩展区域的右下角

	int        nVaildSize;                                          //有效轨迹数目
	float      fAvgObjTrLineOri;                                    //目标的轨迹直线角度的平均值
	int		   nHLineNum;											//水平线段条数
	int		   nAHLineNo[MAX_LINENUM_OFTYPE];						//水平线段序号

	_objInfo_befResult( )
	{
		bVaild = false;
		nObjId = -1;
	}
}objInfo_befResult;  

//简单的轨迹信息
typedef struct _simpleTrInfo 
{
	bool  bVaild;					//是否可用

	unsigned long   nTrId;          //轨迹的Id

	//轨迹的属性
	int			nTrLen;             //轨迹的长度
	int			nObjNo;             //所对应的目标的序号
	bool		bCornerIsBg;		//所对应的角点为背景

	int			nIsUse;				//轨迹上匹配上的点的个数
		
	bool	   bDelete;             //是否有删除标记
	bool	   bBg;					//是否有背景标记			
	int		   nJam;				//jam属性值
	int        nDetermined;         //确定性属性

	int        nStopToMove;         //从停止到运动的属性

	bool     bMove2Stop;			//较长时间范围内从运动到停止的属性
	bool     bRecStop4M2S;			//最近时间范围内从运动到停止的属性
	bool     bStopCarPt;
	float    fBgNcc;                //与背景的自相关系数

	float			fVelo1;         //速率
	float			fOrientation;   //轨迹的角度
	float			fOriImage;      //轨迹的图像角度
	CvPoint2D32f	ptVelocity;     //轨迹的速度

	//轨迹上各点的属性
	double		   dATimeStamp[N_REF+1];    //时间戳             
	bool		   bAIsEstimate[N_REF+1];	//是否估计
	CvPoint		   ptATrImg[N_REF+1];		//图像坐标点
	CvPoint2D32f   ptATrWor[N_REF+1];		//世界坐标点

	int       nGroupNo;						//对应grouping中的group序号
	
}simpleTrInfo;  

//目标调整时的信息
typedef struct _object_adjust_info 
{
	bool         bVaild;				//该目标是否有效
	bool         bHadAdjust;            //该目标是否已调整过

	int			 nHistoryLen;			//目标历史长度
	double       dAddToSetStamp;        //加入到集合的时间戳

	int				nTrSize;                        //轨迹数目
	CvPoint			centerPt;                       //目标中心点
	CvPoint			objLtPt;						//目标左上角点
	CvPoint			objRbPt;						//目标右下角点
	CvPoint2D32f	veloPt;							//目标的速度        

	int            nLongTrNum;                      //最长的轨迹数目
	int				nALongTrNo[3];					//最长的3轨迹的序号
	float         fImgMvAngle;						//图像运动角度

	CvPoint    peoMaxsizeXyPt;						//人maxsizeXy大小
	CvPoint    carMaxsizeXyPt;						//车maxsizeXy大小
	CvPoint    carAngleMaxsizeXyPt;					//车角度maxsizeXy大小

	int    nMegerPeopleObjNum;												//该目标需要考虑的人的合并数目
	int		nAMegerPeopleObjNo[MAXNUM_SPLPEOPLE];	//该目标需要考虑的人的合并对象
	int    nMegerSmlVehObjNum;						//该目标需要考虑的小车的合并数目
	int		nAMegerSmlVehObjNo[MAXNUM_SPLSMLVEH];	//该目标需要考虑的小车上的合并对象
	int    nMegerBigVehObjNum;						//该目标需要考虑的大车的合并数目
	int		nAMegerBigVehObjNo[MAXNUM_SPLBIGVEH];	//该目标需要考虑的大车上的合并对象

	_object_adjust_info( )
	{
		bVaild = false;
		bHadAdjust = false;
	}

}object_adjust_info;  



//大车合并时的轨迹信息
typedef struct _bigVMegerTrInfo 
{
	float			 fTrDist;		//轨迹的总位移长度
	CvPoint2D32f	 distPt;		//轨迹的总位移(分x,y方向)
	bool             bTrMove;		//轨迹是否具有运动
	bool             bTrRecStop;    //轨迹最近是否为静止 
	float            fTrLineOri;    //轨迹的直线角度
}bigVMegerTrInfo;  


//轨迹的模式
enum TRACKTYPE
{
	DEFAULTTRID	= 0,		//默认
	CROSSTRID,				//横穿        
	AGAINSTTRID				//逆行
};


//对奇异轨迹使用的综合体
typedef struct _fantTrUseInfo 
{
	float *fATrImgOri; 
	int    *nATrOriBinNo; 
	int		nYBlock;
	int    nXBlock;
	int    nAreaNum;
	int    nAreaMaxFantTrNum;

	int   *nAAreaFantTrNum;
	int   **nAAAreaFantTrNo;

	int   *nAAreaCrossTrNum;
	int   **nAAAreaCrossTrNo;
	_fantTrUseInfo( )
	{
		fATrImgOri = NULL; 
		nATrOriBinNo = NULL; 
		nYBlock = 6;
		nXBlock = 6;
		nAreaNum = 36;
		nAreaMaxFantTrNum = 500;

		nAAreaFantTrNum = NULL;
		nAAAreaFantTrNo = NULL;

		nAAreaCrossTrNum = NULL;
		nAAAreaCrossTrNo = NULL;
	}
}fantTrUseInfo;


//事件报警所用到的辅助信息
typedef struct _eventAlertInfo 
{
	bool       bVaild;              //该信息是否有效
	//int      nObjId;				//目标的Id

	double     tsMove2Stop;         //满足从动到静的时间戳

	_eventAlertInfo( )
	{
		bVaild = false;
		//nObjId = -1;

		tsMove2Stop = -1000.0;
	}
}eventAlertInfo;

//maxsize类型
enum MAXSIZEMOD
{	
	MAXSIZE = 0,  	
	MAXSIZEX,  	
	MAXSIZEY,  	
	CARMAXSIZEX,		//3
	CARMAXSIZEY,
	PEOMAXSIZEX,		//5
	PEOMAXSIZEY,
	PEOTOP2BOTH		//7
};


//maxsize点的类型
enum MaxSizePtMode
{	
	CARMAXSIZEPT = 0,  	//车
	PEOMAXSIZEPT, 		//人
	IMGA_CARMAXSIZEPT 	//带图像角度的车
};



//事件调试类型
enum EVENTDEBUGMOD
{	
	DEBUG_ALL = 0,  
	DEBUG_TYPE = 10,     //类型判断调试

	EVENTDEBUG_ALL = 100,  
	EVENTDEBUG_CARSTOP = 110,     //车辆停止调试
	EVENTDEBUG_CHANGE = 120,       //变道调试

	EVENTDEBUG_OBJAPPEAR =300   //目标出现调试
};

//sift特征点信息结构
#ifndef SIFT_FEAT_STRUCT
	#define SIFT_FEAT_STRUCT
	typedef struct _siftFeat
	{
		CvPoint pt;				 //点位置
		uchar cSift[SIFT_DIMS]; //特征值
	} siftFeat;
#endif

//停止的车的信息结构
typedef struct _stopCarStru
{
	double   tsAlertStop;
	double   tsUpdateStopPt;
	CvPoint ltPt;       //点位置
	CvPoint rbPt;      //点位置
	int          nPtNum;
	siftFeat  ASiftFeat[100];
}stopCarStru;


//轨迹与点匹配的结构
typedef struct _trMatchStru
{
	int nTrNo;        //轨迹序号	
	int nMatchPtNo;   //匹配上的点位置
	int nSiftDist;    //sift距离

	CvPoint diffPt;   //点位置
	int nXLabelNo;     //分类后的X序号
	int nYLabelNo;     //分类后的Y序号
	int nLabelNo;      //分类后的序号

	int nMainLabelNo;   //轨迹分类后周围的主序号
	int nMainLabelCnt;  //轨迹分类后周围为该主序号的数目
	
	_trMatchStru()
	{
		nTrNo = -1;
		nMatchPtNo = -1;
		nSiftDist = 10000000;

		diffPt = cvPoint(-10000, -10000);
		nXLabelNo = -10000;
		nYLabelNo = -10000;
		nLabelNo = -1;

		nMainLabelNo = -1;
		nMainLabelCnt = -1;
	}
}trMatchStru;



//匹配轨迹的分类结构
typedef struct _matchTrClassStru
{
	int nLabelNo;      //分类后的序号
	int nXLabelNo;     //分类后的X序号
	int nYLabelNo;     //分类后的Y序号
	int nElemCnt;      //该类的元素数目
	_matchTrClassStru()
	{
		nLabelNo = -1;
		nXLabelNo = -10000;
		nYLabelNo = -10000;
		nElemCnt = -1;
	}
}matchTrClassStru;


typedef struct _myGrid
{
	CvPoint  ltPt;
	CvPoint  rbPt;
	CvPoint  whPt;
}myGridArea;




typedef struct _candBottomLine
{
	CvPoint ptStart;
	CvPoint ptEnd;
	int     nPeakNo;
	_candBottomLine( )
	{
		ptStart = cvPoint( -1000, -1000 );
		ptEnd = cvPoint( -1000, -1000 );
		nPeakNo = -1000;
	}
}candBottomLine;



///////////////////////////////////////////////////////////////


//记录队列长度及其平均速度
typedef struct mvQUEUE 
{
	float queue_length;
	float mean_velo;
	int	sum;
} mvQUEUE;

typedef struct
{
	float x; 
	float y;
} MvPoint;

CV_INLINE  MvPoint  mvPoint( float x, float y )
{
	MvPoint p;

	p.x = x;
	p.y = y;

	return p;
}

typedef struct _IMG_FRAME
{
	char*		data;
	int			width;
	int			height;
	int         useWidth;
	int         useHeight;
	int64_t		ts;	

	_IMG_FRAME()
	{
		data	=	NULL;
		width	=	0;	
		height	=	0;
		useWidth =  0;
		useHeight = 0;
		ts		=	0;
	}
}IMG_FRAME;


//记录点的轨迹
typedef struct  APPEAR_VEHICLE_TRACK_INFO
{
	int    num;            //出现的次数
	double ts_last;        //最后一次出现的时间
	APPEAR_VEHICLE_TRACK_INFO()
	{
		num=0;
		ts_last=-1.0;
	}
}appear_vhicle_track_info;


typedef struct _VeloHistory
{
	float left,top,right,bottom;
	float velo,ori;
	_VeloHistory()
	{
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
		velo = 0.0f;
		ori = 0.0f;
	}
}VeloHistory;


#define THRESHOLD(size, c) (c/size)



typedef struct MVVEHICLETIMEVOL 
{
	double cur_vol;
	double cur_time;
	double pre_vol;
	double pre_time;
	bool   flag;
	float  betwin_car_distance;
	float  vehicle_length;
	int    vehicle_num;
	int    busnum;
	//long   int    lasttimes;
	MVVEHICLETIMEVOL()
	{
		cur_vol = 0.0;
		cur_time = 0.0;
		pre_time = 0.0;
		pre_vol = 0.0;
		betwin_car_distance = 0.0f;
		flag = false;
		vehicle_length = 0.0f;
		vehicle_num = 0;
		busnum = 0;
		//lasttimes = 0;
	}
} mvvehicletimevol;


#ifndef	SNAP_HISTORY
	#define SNAP_HISTORY	20
#endif

struct MyVehicleHistoryElem : CvSetElem
{
	int		length;
	double	ts[SNAP_HISTORY+1];
	MyGroup	*group;//太大了，需new
};

typedef struct  ROAD_ATTRIBUTE
{
	IplImage* image;
	bool  need_to_broad;
	int		 vehicle_direct;
	ROAD_ATTRIBUTE()
	{
		image =NULL;
		need_to_broad = false;
	}
} road_attribute;


typedef struct strMvBusElem
{
	MyVehicleElem *p_bus_elem;
	int  v_idx;
	bool is_matched;

}MvBusElem;

typedef struct  MvBusContourElem{
	CvPoint2D32f ptcenter;
	CvPoint2D32f pts[8];
	CvPoint2D32f ptsPolygon[8];
	short        nPolygon;
	CvPoint2D32f ptsTopPolygon[8];
	short        nTopPolygon;
	bool         is_matched;
}MvBusContourElem;

//事件车牌检测结构体
typedef  struct _EVENT_CARNUM_DETECT_STRUCT
{ 
	CvRect detectArea;  //用于事件车牌检测的区域
	int    nDirection;  //方向,前牌0 尾牌1
	_EVENT_CARNUM_DETECT_STRUCT( )
	{
		detectArea = cvRect(0, 0, 0, 0);  
		nDirection = -1;  
	}
}EVENT_CARNUM_DETECT_STRUCT;

//对检测到的车牌区域结构体
typedef struct _DETECTED_CARNUM_AREA_STRUCT
{
	long     index;
	double   ts_detected;
	CvPoint  lt_pt;
	CvPoint  rb_pt;
	int      nPoint;
	CvPoint  *tr_pt;

	_DETECTED_CARNUM_AREA_STRUCT( )
	{
		index = -100;
		ts_detected = -100.0;
		lt_pt = cvPoint( 0, 0);
		rb_pt = cvPoint( 0, 0);
		nPoint = 0;
		tr_pt = NULL;
	}
}DETECTED_CARNUM_AREA_STRUCT;


struct MvCarNumElem:CvSetElem
{
	MvCarNumInfo stCarnum;
};




//涉及到全局设置参数
typedef struct _AN_GLOBAL_SET
{
	//所使用的图像大小
	int nWidth;    
	int nHeight;


	//当前的彩色和灰度图像
	IplImage * m_pRgbImg;   //彩色 
	IplImage * m_pGrayImg;  //灰度

	//车辆maxsize
	IplImage * m_pCarMaxSizeX;  //ushort 
	IplImage * m_pCarMaxSizeY;  //ushort

	//行人maxsize
	IplImage * m_pPeoMaxSizeX;  //ushort
	IplImage * m_pPeoMaxSizeY;  //ushort

	//道路区域mask
	IplImage * m_pRoadMaskImg;  //uchar

	//图像中点对应于世界坐标系中的x坐标
	IplImage * m_pWorldX;  //32f 		
	IplImage * m_pWorldY;  //32f

	_AN_GLOBAL_SET( )
	{
		initVar( );
	}

	void initVar( )
	{
		nWidth = nHeight = 0;

		//当前的彩色和灰度图像
		m_pRgbImg = m_pGrayImg = NULL; 

		//车辆maxsize
		m_pCarMaxSizeX = m_pCarMaxSizeY = NULL;

		//行人maxsize
		m_pPeoMaxSizeX = m_pPeoMaxSizeY = NULL;

		//道路区域mask
		m_pRoadMaskImg = NULL;

		//图像中点对应于世界坐标系中的x坐标
		m_pWorldX = m_pWorldY = NULL;
	}

}AnGlobalSet;

#endif
