#ifndef VIOLATIONINFO_H
#define VIOLATIONINFO_H

#include "CarInfo.h"

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
	EVT_FORBID_BIG,					//19	进行大车

	EVT_NO_PUTIN,                   //20	禁止驶入
	EVT_NO_STOP,                    //21	禁停
	EVT_NO_RESORT,                  //22	路口滞留
	EVT_NO_TURNAROUND,               //23	禁止调头

	DSP_EVT_JAM,					//24    交通拥堵，对应事件DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_JAM
	DSP_EVT_PASSBY,					//25	行人横穿, DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_PASSERBY,			
	DSP_EVT_SLOW,					//26	车辆慢行, DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_GO_SLOW,		
	DSP_EVT_PERSON_STOP,			//27	DETECT_RESULT_EVENT_PERSON_STOP,	//9 行人停留
	DSP_EVT_WRONG_CHAN,				//28	DETECT_RESULT_EVENT_WRONG_CHAN,		//10 非机动车出现

	DSP_EVT_PERSON_AGAINST,			//DETECT_RESULT_EVENT_PERSON_AGAINST, //11 行人逆行
	DSP_EVT_CROSS,					//30	DETECT_RESULT_EVENT_CROSS,		    //12 车辆横穿
	DSP_EVT_PERSON_APPEAR,			//31	DETECT_RESULT_EVENT_PERSON_APPEAR,  //13 行人出现
	DSP_EVT_APPEAR,				    //32	DETECT_RESULT_EVENT_APPEAR,		    //14 机动车出现
	DSP_EVT_DECILIT,				 //33	遗弃物
	EVT_NO_PARKING,                  //34    黄网格停靠
	EVT_BREAK_GATE,					//35    冲卡
	EVT_NOT_CUTRTESY_DRIVE,			//36	没有礼让行人

	EVT_CROSSLINE_STOP,				//37	越线停车
	EVT_GASSER,						//38    加塞
	ELE_ARROW_RED_VIO,              //39   箭头闯红灯
	ELE_CIRCLE_RED_VIO,              //40	圆形闯红灯
	ELE_ONE_WAY_STREET_VIO,          //41	单行道闯禁令
	EVT_TAKE_UP_EMERGENCYWAY    //42  占用应急车道
};

enum VIO_EVENT_RECORD
{
	EVT_FORE_FIRST_RECORD,			//0 压第一个待转前行线记录
	EVT_STOP_SECOND_RECORD			//1 压第二个待转停止行线记录
};

typedef struct _ViolationInfo
{
	int          id;
	CarInfo carInfo;             // 闯红灯的车辆信息。主要是：车牌号码、颜色、速度。如果没有车牌则按******，*++++++*显示。
	unsigned int frameSeqs[20];  // 标明其闯红灯的三帧图像的帧号。
	unsigned int dis[20];        // 在闯红灯事件中用于表示3帧图片离红灯开始帧的间隔（帧数）
	unsigned int uUpperFrm, uLowerFrm;  //取秒图时，判断第一张图的上下限！
	int64        redLightStartTime;   //红灯开始时间！
	int          index[20];
	int          nPicCount;      // 图像数

	int          nChannel;       // 车道号，-1表未知
	VIO_EVENT_TYPE evtType;      // 事件类型

	int          nChannelTurnDir;  //所属车道行驶方向属性！-1,左转车道，左转+直行车道；1，右转车道，右转+直行车道！

	bool         bFlagHoldReg;    //是否为待转区闯红灯，主要是用于第三张延时取图用 //true表示非待转区
	MvLine*       pLineTouch;      //看看是碰到左直右哪根现报的闯红灯 
	unsigned int uFirstRedLightFrame;// = 0;
	int          nRedLightTime;
	int          nRedLightViolationAllowance;
	int          nOutState; //；0：存储；1输出；2:删除
	_ViolationInfo()
	{
		nChannelTurnDir = -2;  //未知！
		nChannel  = -1;
		nPicCount = 3;
		uUpperFrm = 0; uLowerFrm = 0;
		redLightStartTime = -1;
		memset(dis, 0, 3*sizeof(unsigned int));
		bFlagHoldReg = false;
		uFirstRedLightFrame = 0;
		nRedLightTime = 0;
		nRedLightViolationAllowance = 0;
		nOutState = 1;		
	}
} ViolationInfo;

typedef struct _VioPressRecode
{
	unsigned int frameSeqsFore;  // 标明其闯红灯压第一待转前行线的帧号。
	unsigned int frameSeqsStop;  // 标明其闯红灯压第二待转停止线的帧号。
	VIO_EVENT_RECORD evtType;      // 事件类型
	_VioPressRecode()
	{
		frameSeqsFore = 0;
		frameSeqsStop = 0;
	}
} VioPressRecode;

#endif
