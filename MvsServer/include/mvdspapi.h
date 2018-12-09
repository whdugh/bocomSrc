/* 版权所有 2013 上海博康智能信息技术有限公司
* Copyright (c) 2013，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvDspAPI.h
* 摘要: 
* 版本: SVN 1611
* 作者: 
* 完成日期: 2013年1月11日
* 修改说明：1、PC应用需要增加一个宏开关  #define MV_DSP_USE_SLIM_STRU
*			2、2013-2-27 版本1184 增加道路区域和相机高度
*			3、2013-3-14 版本1224 增加预留GlobeLine预留字段	
*			4、2013-3-19 版本1234 为了兼容性考虑，去除GlobeLine预留字段
*			5、2013-3-26 版本1274 增加相机版本号
*			6、2013-3-28 版本1289 增加对事件的支持
*			7、2013-4-1  版本1294 MvDspGlobalSetting    增加卡口图片输出张数
*								  busReserved[4096];    公交预留字段
*								  结构CarInfo增加字段 unsigned int	pExtFrame[4];	
*			8、2013-4-18 版本1359 增加车道属性按照大小车分别来判超速
*			9、2013-4-18 版本1362 增加车道属性遗弃物判断
*			10、2013-05-13 svn1443 车道属性增加车辆行驶速度低于设定的报警速度
*			11、2013-05-20 svn1465 支持黄网格自动判罚
*			12、2013-07-01 svn1669 卡口输出增加目标轮廓大小 MvRect rtContour;	//车牌目标输出帧的轮廓大小，初始化应为(0,0,0,0)
*           13、2014-02-24 svn141  禁行结构体中设置的类型做扩展—增加超速（按时间段报）
*			14、2014-05-04 svn220  1.增加车牌方向 2.增加统计信息
*			15、2014-06-16 svn317  1.增加流量检测功能 2、无牌车违章检测 3、越线停车检测
*/
#ifndef ALGORITHM_YUV

#ifndef MV_DSP_API_H__
#define MV_DSP_API_H__
#include "declare.h"

#define MAX_IMG_WIDTH		432
#define MAX_IMG_HEIGHT		360

#define L_MAX_IMG_WIDTH		480
#define L_MAX_IMG_HEIGHT	400

#define MON 0x01//周一
#define TUE 0x02//周二
#define WEN 0x04//周三
#define THU 0x08//周四
#define FRI 0x10//周五
#define SAT 0x20//周六
#define SUN 0x40//周日
#define WEK 0x7F//全周
#define WED 0x60//周六、周日
#define WKD 0x1F//周一到周五，工作日

#define DO_CAR_NUM_DETECT			0x001           // 是否进行车牌过滤。程序内部默认打开。
#define DO_FIND_TARGET				0x002           // 是否进行卡口目标检测
#define DO_ELE_POLICE				0x004           // 电子警察
#define DO_COLOR_DETECT				0x008           // 检测颜色
#define DO_VEHICLE_TYPE				0x010           // 车型检测
#define DO_LOOP_SHUTTER				0x020			// 线圈爆闪
#define DO_DSP_EVENT				0x040			// 事件检测
#define DO_NOPLATE					0x080			// 无牌车违章检测
#define DO_LOOP_DETECT				0x100           // 线圈检测
#define DO_ON_BUS_VIO_DET			0x200           // 车载检测
#define DO_RADAR_DETECT				0x400           // 雷达检测
#define DO_VIOLATION_DETECT			0x800           // 违章检测
#define DO_DETECT_TRAFFIC_LIGHT		0x1000			// 视频检测红绿灯
#define DO_LASER_DETECT				0x2000			// 激光检测
#define DO_VIDEO_SHUTTER			0x8000			// 视频爆闪

#ifdef MV_DSP_ENV
#define int64  long long
//typedef int  bool;
//const int true = 1;
//const int false = 0;
#endif

/*
enum ObjTypeForOutput
{
	OTHER = 1,     // 未知
	PERSON,        // 行人5
	TWO_WHEEL,     // 两轮车
	SMALL,         // 小 1
	MIDDLE,        // 中 2
	BIG,           // 大 3
};*/

typedef struct _MvRect
{
	int x;
	int y;
	int width;
	int height;
}MvRect;

 /*MvRect  mvRect( int x, int y, int width, int height )
{
	MvRect r;

	r.x = x;
	r.y = y;
	r.width = width;
	r.height = height;

	return r;
}*/

typedef struct DspMvPoint
{
	int x;
	int y;
}DspMvPoint;
/*
static  DspMvPoint  mvPoint( int x, int y )
{
	DspMvPoint p;

	p.x = x;
	p.y = y;

	return p;
}
*/
//红灯信号
typedef struct _DSP_VTS_SIGNAL_STATUS
{
	int  nRoadIndex;	   //车道序号
	unsigned int uFrameSeq;

	int bLeftSignal;     //左转灯状态.红灯true。绿灯false。
	int bStraightSignal; //直行灯状态
	int bRightSignal;    //右转灯状态

	/*
	_VTS_SIGNAL_STATUS()
	{
		uFrameSeq       = 0;
		nRoadIndex      = -1;
		bLeftSignal     = 0;
		bStraightSignal = 0;
		bRightSignal    = 0;
	}
	*/

}Dsp_VTS_SIGNAL_STATUS, DspMvChannelFrameSignalStatus;

typedef struct MvPoint2D64f
{
	double x;
	double y;
}
MvPoint2D64f;


static  MvPoint2D64f  mvPoint2D64f( double x, double y )
{
	MvPoint2D64f p;

	p.x = x;
	p.y = y;

	return p;
}


typedef struct _DspMvLine
{
	// 图像坐标。
	DspMvPoint start;  //(应用配置)
	DspMvPoint end;    //（应用配置）

}DspMvLine;

typedef struct _MV_DSP_CAR_INFO
{
	char   strCarNum[8];  //车牌号码
	char   wj[2];         //武警车牌小字

	int64				ts;					//时间戳(微秒)
	unsigned int		nFrame;				//帧号
	unsigned int		nX;					//车牌图像坐标
	unsigned int		nY;
	unsigned int		nWidth;				//车牌宽度(pixels)
	unsigned int		nHeight;			//车牌高度

	unsigned int		fVelocity;			//车辆速度（km/H）

	unsigned int		nCarnumMean;		//车牌亮度
	unsigned int		nColor;				//颜色
	unsigned int		uPlateType;			//车牌类型 
	unsigned int		nVehicleType;		//车辆类型粗分！小于5.5小，5.5到9.5中， 大于9.5大

	unsigned int		nChannel;			//车道号

	float  iscarnum;      //模板匹配重合度,置信度范围【0,1】，越大越不可靠

	unsigned int		pExtFrame[4];		//
	MvRect				rtContour;			//车牌目标输出帧的轮廓大小，初始化应为(0,0,0,0)
} MvDspCarInfo, DspCarInfo;

// 禁止通行信息。
// 包含禁止通行的时间段，以及禁止通行的车辆类型。
typedef struct _DspNoPassingInfo
{
	unsigned char ucWeekDay; //禁止通行的周天。可以按位或。 (应用配置)

	int nStart;   // 禁止通行时间断开始。从0时0分0秒到禁止通行时间的秒数。  (应用配置)
	int nEnd;     // 比如从1点到两点是禁行时间，则nStart = 1*60*60, nEnd = 2*60*60     (应用配置)

	int nVehType; // //0表示不禁行；1表示禁行小车；2表示禁行大车；3表示禁行所有车辆   (应用配置)
	               ////还要扩充定义4表示禁左、5表示禁右、6表示禁前、7表示超速、8表示禁行大货车
    /*
	_NoPassingInfo()
	{
		ucWeekDay = WEK;
		nStart    = -1;
		nEnd      = -1;
		nVehType  = 0;
	}
	*/
} DspNoPassingInfo;


typedef struct _MV_DSP_REGION
{
	DspMvPoint arrList[16];
	int nPoints;
}MvDspRegion;


/*
车道结构体。
*/
typedef struct _DspChannelRegion
{
	int			nRoadIndex;	//车道序号（应用配置）
	int			nVerRoadIndex; //车道逻辑序号，暂时没有用处。（应用配置）
	int			nDirection;    //车道方向（应用配置）（坐标轴右半轴，以水平方向为起点，顺时针转动所得角度）
	DspMvLine		vDirection;    //车道方向线（应用配置）

	DspMvPoint		arrListChannel[16];     //车道区域（应用配置）
	
	int			nPhysicalLoop;			//物理线圈个数（应用配置）
	float		fLoopDist;				//物理线圈之间的距离（应用配置，默认为5m）
	
	int			nChannlePointNumber;  //构成每个车道区域的顶点个数（应用配置）
	int			bNoTurnLeft;		//禁左（应用配置）
	int			bNoTurnRight;		//禁右（应用配置）
	int			bNoForeward;		//禁止前行（应用配置）

	int     bNoReverse;				// 禁止逆行（应用配置）
	int     bNoPressLine;			//禁止压线！（应用配置）
	int     bNoChangeChannel;		//禁止变道！（应用配置）

	int     nNoTurnAround;          //禁止调头 ——应用配置
    int     nNonMotorWay;           // 是否为非机动车道标志符，0表示机动车道，1表示非机动车道
	                                // 2表示公交车道，3表示机非混行车道，4表示人行道——应用配置

	//车道行驶属性:直行车道0、左转车道1、左转+直行车道2、右转车道3、右转+直行车道4,等！
	int      nChannelDriveDir; //（应用配置）
	DspMvLine   vForeLine;  //每车道一个前行线！（应用配置）相对于原图坐标！但是传给MvViolationDetecter的停止线和vDirection方向线都为缩小图坐标，这一点要注意！

	//每个车道一个停止线
	DspMvLine   vStopLine;  //（应用配置）


	//待转区
	int     bFlagHoldStopReg;  //是否存在待转区（应用配置）
	DspMvLine   vHoldForeLineFirst;  //待转区第一前行线（应用配置）
	DspMvLine   vHoldForeLineSecond;  //待转区第二前行线（应用配置）
	DspMvLine   vHoldStopLineFirst;    //待转区第一停止线（应用配置）
	DspMvLine   vHoldStopLineSecond;    //待转区第二停止线（应用配置）
 
	MvRect OnOffRed;      //红灯区域（应用配置）
    MvRect OnOffGreen;    //绿灯区域（应用配置）

	//防闪用
	MvRect roiLeftLight;   //左边灯区域（应用配置）
	MvRect roiMidLight;    //中间灯区域（应用配置）
	MvRect roiRightLight;  //右边灯区域（应用配置）
	MvRect roiTurnAroundLight; //拐弯灯区域（应用配置）

	//红灯增强用的
	MvRect roiLeftLight_red, roiLeftLight_green;  //左边红、绿灯区域（应用配置）
	MvRect roiMidLight_red, roiMidLight_green;    //中间红、绿灯区域（应用配置）
	MvRect roiRightLight_red, roiRightLight_green; //右边红、绿灯区域（应用配置）
	MvRect roiTurnAroundLight_red, roiTurnAroundLight_green; //拐弯红、绿灯区域（应用配置）

	MvRect rectMedianPos;  //闯红灯或电警时，由客户端指定的第二张图的车辆位置。(应用配置)不可太靠上，否则左转或右转电警就可能取不到！禁行的取图不用吧？

	DspNoPassingInfo   vecNoPassingInfo[24]; //禁止通过时间段属性（应用配置）
	int             nNoPassingInfoNumber;  //禁止通过的时间段个数（应用配置）

	int nRadarAlarmVelo;				//雷达预警速度
	
	int64          m_pRedLightDelayTime[2];    //红灯延迟时间（秒），每个车道两个时间段，[0]信号延迟接收时间，[1]红灯的持续时间

	int  nLeftControl;  //左转控制 (取值范围0-11)
	int  nStraightControl;  //直行控制(取值范围0-11)
	int nRightControl;   //右转控制(取值范围0-11)
	
	//其中0：东直，1：东左，2：南直，3：南左，4：西直，5：西左，6：北直，7：北左，8：东右，9：南右，10：西右，11：北右

	int  nJamp;               //是否判断交通拥堵，1：是， 0：否
    int  nPassagerCross;      //是否判断行人横穿，1：是， 0：否
	int  nCarSlow;            //是否判断车辆慢行，1：是， 0：否
	int  nPerson_Stop;        //是否判断行人停留，1：是， 0：否
	int  nNonMotorAppear;     //是否判断非机动车出现， 1：是， 0：否
	int  nPerson_Against;     //是否判断行人逆行，1：是， 0：否
	int  nCarCross;           //是否判断车辆横穿， 1：是，0：否
	int  nPersonAppear;       //是否判断行人出现， 1：是，0：否
	int  nCarAppear;          //是否判断机动车出现，1:是，0：否
	int  nDelicit;				//遗弃物判断， 1：是， 0： 否

	//说明：如果下面两个速度为0，则参照nRadarAlarmVelo定义的值
	unsigned int uAlarmBig;			//大车限速
	unsigned int uAlarmSmall;		//小车限速

	int nCarStop;				//是否判断停车事件
	int nStopTime;				//停车多久报出(单位秒)
	
	unsigned int uLowThresh;		//车辆行驶速度低于设定的报警速度；

	int bCrossLineStop;				//越线停车检测 默认为0 

	char pReserved[2048-1028];			//预留字段
 
}DspChannelRegion;


typedef struct _GlobeLine
{
	// 电子警察的辅助线（相对于电警缩小图）
	DspMvLine           m_stopLine;   //道路停止线（应用配置）
	DspMvLine           m_foreLine;   //道路前行线（应用配置）
	DspMvLine           m_rightLine;  //禁右线（应用配置）
	DspMvLine           m_leftLine;   //禁左线（应用配置）


	DspMvLine           m_firstLine;		//电警第一触发线（应用配置）
	DspMvLine           m_secondLine;  //	//电警第二触发线（应用配置）
	DspMvLine           m_rightLineOri;	//禁右初始触发线（应用配置）
	DspMvLine           m_leftLineOri;		//禁左初始触发线（应用配置）
	DspMvLine           m_foreLineOri;     //禁前初始触发线（应用配置）

	//可以有多条
	DspMvLine m_vecYellowLine[8];    //黄线（应用配置）
	DspMvLine m_vecWhiteLine[8];     //白线（应用配置）
	DspMvLine m_vecBianDaoXian[8];   //变道线（应用配置）
	int    m_vecYellowNumber;     //黄线的条数（应用配置）
	int    m_vecWhiteNumber;      //白线的条数（应用配置）
	int    m_vecBianDaoXianNumber;  //变道线的条数（应用配置）

    DspMvLine  m_NoTurnAroundLine;     //禁止调头线 ——应用配置
    DspMvLine  m_NoTurnAroundLineOri;     //禁止调头初始触发线 ——应用配置

	//unsigned char pReserved[1024-572];


}GlobeLine;

//道路区域定义
typedef struct _MV_DSP_ROAD_REGIN
{
	int direct;	//方向
	DspMvPoint arrList[16];
	int nPoints;
}MvDspRoadRegion;

//涉及的数据结构
//涉及到全局参数
typedef struct _MV_DSP_GLOBAL_SETTING
{
	int nWidth;           //图像的宽度（应用配置）
	int nHeight;          //图像的高度（应用配置）

	float m_fScaleX;			//m_nWidth*m_nScaleX==原图的宽-  宽度的缩放比例（应用配置）				
	float m_fScaleY;            //高度的缩放比例（应用配置）
 
    int nCheckType;			//做检测的类型（应用配置）
	
	MvRect m_rtCarNum;		//车牌检测区,相对于小图中位置（应用配置）

	int nChannels;					//车道数目（应用配置）
	DspChannelRegion ChnlRegion[8];    //车道属性（应用配置）
	//
	GlobeLine gLines;			//全局用到的线（应用配置）
	
	unsigned short m_pMaxSizeX[MAX_IMG_HEIGHT];	//
	unsigned short m_pMaxSizeY[MAX_IMG_HEIGHT];
	float m_pWorldX[MAX_IMG_HEIGHT];
	float m_pWorldY[MAX_IMG_HEIGHT];

    MvRect m_rtRemoteCarNum;				//远处车牌大小
	MvRect m_rtLocalCarNum;					//近处车牌大小

	int				m_nMaskRegionCount;		//屏蔽区域数目
	MvDspRegion		m_pMaskRegion[8];		//屏蔽区	
	
	int             nNoPutInRegNum;			//禁止驶入区数目——应用配置 
	MvDspRegion     NoPutInReg[8];			//禁止驶入区 ——应用配置

	int             nNoStopRegNum;			//禁停区数目——应用配置
    MvDspRegion     NoStopReg[8];			//禁停区 ——应用配置

	int             nNoResortRegNum;		//禁止滞留区数目——应用配置
    MvDspRegion     NoResortReg[8];			//禁止路口滞留区 ——应用配置

	int			m_nDelayOutput;				//卡口延迟输出时间(公交用，单位是秒)

	//标定6个点对应的图像坐标和世界坐标
	float m_pXYImage[12];					//在原图位置
	float m_pXYWorld[12];					//原图对应位置

	int m_nRoadCount;						//道路数量,最多允许四个
	MvDspRoadRegion m_pRoadRegion[4];		//道路区域
	
	MvDspRegion vioRegion;					//违章检测区域
	
	float		cameraHeight;				//摄像机高度

	int nVersion;							//版本号

	int				nCarPicCount;			//卡口输出张数，默认为1

	MvDspRegion parkingRegion;				//黄网格区域

	int nMode;								//0:普通模式，16为调试模式，默认为0

	int nIsRankVio;                         //是否进行违章分级，0：否，1：是
	int pVioTypeRank[64];                   //结构enum VIO_EVENT_TYPE中每个违章类型对应的等级，
	                                        //默认0不分级。分9个级别，9级别最高，1最低

	int nBreakGate;							//是否冲卡

	int nNotCoutesyDrive;					//是否礼让行人

	int bStrictMode;						//是否为严格模式

	int nFrameRate;

	int nFluxDetect;						//是否做流量统计，默认为0

	unsigned char reserved[1024*32-26796];

	unsigned short m_pSizeX[512];	//
	unsigned short m_pSizeY[512];
	float m_pWY[512];

	unsigned char busReserved[4096];		//公交预留字段


}MvDspGlobalSetting;



//运行时每帧需要传入的内容
typedef struct _MV_DSP_GLOBAL_PARA
{
	int m_nIsDay;			//1 白天, 0 夜晚， -1自动判断
	
	unsigned int nFrameIndex;	//帧号
	int64 ts;					//时间戳(以微秒为单位)

	int nCarNum;			//车牌数量
	DspCarInfo arrCarInfo[16];	//车牌信息

	DspMvChannelFrameSignalStatus arrChnl[8];		//车道信号状态，要和车道对应

	int arPhysicalLoopStatus[8][2];			//物理线圈状态
	int64 arPhyLoopTs[8];			//物理线圈采集信号对应时间戳
	
	unsigned char arrImageStatus[8];		//亮图状态，1-亮图， 0-非亮图

	unsigned char *pImage;			//图像信息

	unsigned int m_pRadar[8];  //记录每个车道的雷达测速值

}MvDspGlobalPara;

/*
enum VIO_EVENT_TYPE
{
	ELE_RED_LIGHT_VIOLATION,        // 0	闯红灯
	ELE_PARKING_VIOLATION,          // 1	违章停车
	ELE_FORBID_LEFT,                // 2	禁止左拐
	ELE_FORBID_RIGHT,               // 3	禁止右拐
	ELE_FORBID_STRAIGHT,            // 4	禁止前行
	ELE_RETROGRADE_MOTION,          // 5	逆行
	ELE_PRESS_LINE,                 // 6	压黄线
	ELE_PRESS_WHITE_LINE,           // 7	压白线
	ELE_NO_PASSING,                 // 8	禁行
	ELE_EVT_BIANDAO,                // 9	变道
	OBV_TAKE_UP_BUSWAY,             // 10	占用公交道

	EVT_CYC_APPEAR, 				//11	柴油车出现
	EVT_FORBID_TRUCK,				//12	大货禁行
	EVT_GO_FAST,                    //13	车辆超速
	EVT_NON_LOCAL_PLATE,            //14	非本地车！
	EVT_YELLOW_PLATE,               //15	黄牌车！
	EVT_CYC_YELLOE_PLATE,           //16	黄牌柴油车！
	EVT_TAKE_UP_NONMOTORWAY,        //17	机占非

	EVT_FORBID_SMALL,				//18	禁行小车
	EVT_FORBID_BIG,					//19	禁行大车
	
	EVT_NO_PUTIN,                   //20	禁止驶入
	EVT_NO_STOP,                    //21	禁停
	EVT_NO_RESORT,                  //22	路口滞留
	EVT_NO_TURNAROUND,               //23	禁止调头

	DSP_EVT_JAM,					//24    交通拥堵，对应事件DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_JAM
	DSP_EVT_PASSBY,					//25	行人横穿, DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_PASSERBY,			
	DSP_EVT_SLOW,					//26	车辆慢行, DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_GO_SLOW,		
	DSP_EVT_PERSON_STOP,			//27	DETECT_RESULT_EVENT_PERSON_STOP,	//9 行人停留
	DSP_EVT_WRONG_CHAN,				//28	DETECT_RESULT_EVENT_WRONG_CHAN,		//10 非机动车出现
	DSP_EVT_PERSON_AGAINST,			//29	DETECT_RESULT_EVENT_PERSON_AGAINST, //11 行人逆行
	DSP_EVT_CROSS,					//30	DETECT_RESULT_EVENT_CROSS,		    //12 车辆横穿
	DSP_EVT_PERSON_APPEAR,			//31	DETECT_RESULT_EVENT_PERSON_APPEAR,  //13 行人出现
	DSP_EVT_APPEAR,					//32	DETECT_RESULT_EVENT_APPEAR,		    //14 机动车出现
	DSP_EVT_DECILIT,				//33	遗弃物
	
	EVT_NO_PARKING,					//34	非停车区域等候(黄网格)
	EVT_BREAK_GATE,					//35    冲卡

	EVT_NOT_CUTRTESY_DRIVE,			//36	没有礼让行人

	EVT_CROSSLINE_STOP				//37	越线停车
	EVT_GASSER						//38    加塞

	OBV_TAKE_UP_BUSWAY_1 //39.公交车道内的条件 一.在左线的右侧1或在右线的左侧2、满足算法其他过滤条件。 
	OBV_TAKE_UP_BUSWAY_2 //40.非公交车道内，且有车道线 二.在左线的左侧3或在右线的右侧4。 
	OBV_TAKE_UP_BUSWAY_3 //41.算法内部过滤条件 三.比如算法1/3、停在路口、右侧有车、转弯掉头。 
	OBV_TAKE_UP_BUSWAY_4 //42.未检测出车道线及其他 四.未检测出车道线的。 
};
*/


typedef struct _DspViolationInfo
{
	DspCarInfo carInfo;             // 闯红灯的车辆信息。主要是：车牌号码、颜色、速度。如果没有车牌则按******，*++++++*显示。
	unsigned int frameSeqs[16];  // 标明其闯红灯的三帧图像的帧号。
	unsigned int dis[16];        // 在闯红灯事件中用于表示3帧图片离红灯开始帧的间隔（帧数）
	unsigned int uUpperFrm, uLowerFrm;  //取秒图时，判断第一张图的上下限！
	int64        redLightStartTime;   //红灯开始时间！
	//int          index[20];
	int          nPicCount;      // 图像数

	int          nChannel;       // 车道号，-1表未知
	int          evtType;        // 事件类型

	int          nChannelTurnDir;  //所属车道行驶方向属性！-1,左转车道，左转+直行车道；1，右转车道，右转+直行车道！
	/*
	_ViolationInfo()
	{
		nChannelTurnDir = -2;  //未知！
		nChannel  = -1;
		nPicCount = 3;
		uUpperFrm = 0; uLowerFrm = 0;
		redLightStartTime = -1;
		memset(dis, 0, 3*sizeof(unsigned int));
	}
	*/
} DspViolationInfo;

typedef struct _MV_DSP_OUTPUT
{
	int nCarinfoNum;		//车牌或者目标数量
	DspCarInfo pCarInfo[16];

	int nViNum;				//违章数量
	DspViolationInfo pViInfo[16];

	int nCodingNum;
	unsigned int pCodingIndex[16];

	int nDelNum;
	unsigned int pDelIndex[16];

	unsigned char uChalShutState[8];    //车道爆闪状态

	int nStatus;					//记录程序执行的状态
	char pMsg[256];		

}MvDspOutPut;


typedef struct _MV_DSP_STATINFO
{
	int nChnl;					// 车道编号
	int nFlux;					// 实时交通流量
	unsigned int nAvgVelo;		// 平均车速,单位：公里/小时
	unsigned int nOccupyRatio;	// 平均占有率，单位：%
	unsigned int nAvgDis;		// 平均车头间距；单位：米
}MvDspStatInfo;


//==========================================================
//	接口函数
//**********************************************************
//1、MvInit()
//2、MvUnInit()
//3、MvInput()
//4、MvOutPut()
//**********************************************************

/************************************************************************/
/* 函数说明：初始化调用
   参数说明：pSetting	设置信息，传入地址	
			 pPara		运行信息，传入地址
			 pOutPut	每帧返回的结果
*/
/************************************************************************/
int mvInit();

int mvUnInit();


int mvInput();		//传入内容到pPara

int mvOutput();		//读取pOutPut内容

int mvInquireStaticInfo(MvDspStatInfo pStatInfo[8]);		//获取流量等统计信息

int mvGetVersion(char * ver);	//获取版本号

#endif

#endif
