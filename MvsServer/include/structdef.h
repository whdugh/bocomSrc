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

#ifndef SKP_ROAD_STRUCTDEF_H
#define SKP_ROAD_STRUCTDEF_H

#include <string>
#include <list>
#include <map>
#include "constdef.h"

#ifndef WIN32
#include "cv.h"
#endif

#ifdef _WINDOWS
#pragma warning(disable: 4800)
#endif

#define CHECKMAX 3
#define POINTLEN 1024

using namespace std;

#ifdef _WINDOWS
	#ifndef int64_t
		#define int64_t __int64
	#endif
#endif

#ifndef UINT64
#define UINT64 unsigned long long
#endif

//通讯包头[SEEKPAI ROAD INTERFACE PROTOCOL]
typedef struct _MIMAX_HEADER
{
	UINT32 uCameraID;     //相机编号
	UINT32 uCmdLen;		//命令长度
	UINT32 uCmdID;		//命令类型
	UINT32 uCmdFlag;		//命令标志

	_MIMAX_HEADER()
	{
		uCameraID = 0;
		uCmdLen = 0;
		uCmdID = 0;
		uCmdFlag = 0;
	}
}MIMAX_HEADER,*LPMIMAX_HEADER;


//扩展通讯包头[SEEKPAI ROAD INTERFACE PROTOCOL]
typedef struct _SRIP_HEADER
{
	UINT32 uMsgLen;		//消息长度
	UINT32 uMsgCommandID;	//消息命令
	UINT32 uMsgCode;		//消息代码
	UINT32 uMsgSource;	//消息源
	UINT32 uCmdFlag;		//命令标志
	_SRIP_HEADER()
	{
		uMsgLen = 0;
		uMsgCommandID = 0;
		uMsgCode = 0;
		uMsgSource = 0;
		uCmdFlag = 0;
	}
}SRIP_HEADER,*LPSRIP_HEADER;


typedef struct _RECORD_PARA
{
	UINT32 uMsgCommandID;	//消息命令
	UINT32 uMsgLen;		//消息长度(录像文件长度)
	UINT32 uMsgCode;		//消息代码
	UINT32 uID;            //录像数据ID
	UINT32 uType;		   //0:车牌查询，1:事件查询，2:全天录像
	char chReserved[32];   //扩展
	_RECORD_PARA()
	{
		uMsgCommandID = 0;
		uMsgLen = 0;
		uMsgCode = 0;
		uID = 0;
		uType = 0;
		memset(chReserved,0,32);	
	}
}RECORD_PARA,*LPRECORD_PARA;

//记录状态结构
typedef struct _RECORD_STATUS
{
	UINT32 uBeginSeq;		//开始序列号
	UINT32 uEndSeq;	    //结束序列号
	UINT32 uBeginTime;	//开始时间
	UINT32 uEndTime;		//结束时间
	UINT32 uCount;		//记录条数
	_RECORD_STATUS()
	{
		uBeginSeq = 0;
		uEndSeq = 0;
		uBeginTime = 0;
		uEndTime = 0;
		uCount = 0;
	}
}RECORD_STATUS;

//日志记录结构
typedef struct _RECORD_LOG
{
	UINT32 uSeq;					//序列号
	UINT32 uTime;					//时间
	UINT32 uCode;					//日志代码
	char chText[MAX_LOG];				//日志内容
	char chReserved[64];				//扩展
	_RECORD_LOG()
	{
		uSeq = 0;
		uTime = 0;
		uCode = 0;
		memset(chText,0,MAX_LOG);
		memset(chReserved,0,64);
	}
}RECORD_LOG;



//统计记录结构
typedef struct _RECORD_STATISTIC
{
	UINT32 uSeq;			//序列号
	UINT32 uTime;			//时间
	UINT32 uStatTimeLen;	 //统计时间长度

    UINT32 uRoadType[MAX_ROADWAY];		//车道号及车道类型
	UINT32 uFlux[MAX_ROADWAY];		//第一流量检测值
	UINT32 uSpeed[MAX_ROADWAY];		//车道平均速度
	UINT32 uQueue[MAX_ROADWAY];		//最大排队长度
	UINT32 uOccupancy[MAX_ROADWAY];   //平均占有率
	UINT32 uSpace[MAX_ROADWAY];		//平均车辆间距
	UINT32 uFluxCom[MAX_ROADWAY];		//补充流量检测值

	char chReserved[16];				//扩展

	BYTE uBigCarSpeed[MAX_ROADWAY];	//大车平均速度
	BYTE uSmallCarSpeed[MAX_ROADWAY];	//小车平均速度
	BYTE uMaxSpeed[MAX_ROADWAY];		//最大速度

	_RECORD_STATISTIC()
	{
		uSeq = 0;
		uTime = 0;
		uStatTimeLen = 300;
		memset(uRoadType,0xffffffff,MAX_ROADWAY*sizeof(UINT32));
		memset(uFlux,0xffffffff,MAX_ROADWAY*sizeof(UINT32));
		memset(uSpeed,0xffffffff,MAX_ROADWAY*sizeof(UINT32));
		memset(uQueue,0xffffffff,MAX_ROADWAY*sizeof(UINT32));
		memset(uOccupancy,0xffffffff,MAX_ROADWAY*sizeof(UINT32));
		memset(uSpace,0xffffffff,MAX_ROADWAY*sizeof(UINT32));
		memset(uFluxCom,0xffffffff,MAX_ROADWAY*sizeof(UINT32));

		memset(uBigCarSpeed,0xff,MAX_ROADWAY*sizeof(BYTE));
		memset(uSmallCarSpeed,0xff,MAX_ROADWAY*sizeof(BYTE));
		memset(uMaxSpeed,0xff,MAX_ROADWAY*sizeof(BYTE));
		memset(chReserved,0,16);
	}
}RECORD_STATISTIC;

//事件记录结构
typedef struct _RECORD_EVENT
{
	UINT32 uSeq;						//序列号
	UINT32 uVideoBeginTime;			//录象开始时间(秒)
	UINT32 uMiVideoBeginTime;			//录象开始时间(毫秒)
	UINT32 uVideoEndTime;				//录象结束时间(秒)
	UINT32 uMiVideoEndTime;			//录象结束时间(毫秒)
	UINT32 uEventBeginTime;			//事件开始时间(秒)
	UINT32 uMiEventBeginTime;			//事件开始时间(毫秒)
	UINT32 uEventEndTime;				//事件结束时间(秒)
	UINT32 uMiEventEndTime;			//事件结束时间(毫秒)

	UINT32 uCode;						//事件代码  //参考 DETECT_RESULT_TYPE
	UINT32 uRoadWayID;				//车道号

	UINT32 uPicSize;					//快照图片大小
	UINT32 uPicWidth;					//快照图片宽度
	UINT32 uPicHeight;				//快照图片高度
	UINT32 uPosX;						//事件横坐标
	UINT32 uPosY;						//事件纵坐标

	char chText[MAX_EVENT];					//事件类型描述
	char chVideoPath[MAX_VIDEO];			//录象存储位置

	UINT32 uType;						//车类型
	UINT32 uColor1;                    //车身颜色1
	UINT32 uSpeed;					//车速
	UINT32 uDirection;                //行驶方向

	char chPlace[64];				//事件地点
	char chPicPath[MAX_VIDEO];				//事件图片路径

	UINT32 uColor2;                    //车身颜色2
	UINT32 uColor3;                    //车身颜色3

	UINT32 uWeight1;                    //车身颜色权重1
	UINT32 uWeight2;                    //车身颜色权重2
	UINT32 uWeight3;                    //车身颜色权重3

	UINT32 uDetailCarType;                 //车型细分

	UINT32 Position;                   // 图象帧在文件中的开始位置（单位为字节）
	char chReserved[4];				//扩展
	UINT32 nVideoFlag;				//录像标志(0: 新建录像 1:源文件附加)
	UINT32 uDensity;                  //密度
	UINT32 uTime2;						//第二事件时间(秒)
	UINT32 uMiTime2;					//第二事件时间(毫秒)
	UINT32 uEventDefine;                //参数域值
	UINT32 uEventId;                    //事件和车牌关联时用的id
	UINT32 uStatusType;                 //记录类型
	UINT32 uChannelID;                  //通道编号

	_RECORD_EVENT()
	{
		uSeq = 0;
		uVideoBeginTime = 0;
		uMiVideoBeginTime = 0;
		uVideoEndTime = 0;
		uMiVideoEndTime = 0;
		uEventBeginTime = 0;
		uMiEventBeginTime = 0;
		uEventEndTime = 0;
		uMiEventEndTime = 0;

		uCode = 0;
		uRoadWayID = 0;

		uPicSize = 0;
		uPicWidth = 0;
		uPicHeight = 0;
		uPosX = 0;
		uPosY = 0;

		uColor1 = 11;
		uType = 0;
		uSpeed = 0;
		uDirection = 0;

		memset(chText,0,MAX_EVENT);
		memset(chVideoPath,0,MAX_VIDEO);
		memset(chPlace,0,64);
		memset(chPicPath,0,MAX_VIDEO);

		uColor2 = 11;
		uColor3 = 11;

		uWeight1 = 100;
		uWeight2 = 0;
		uWeight3 = 0;

		uEventId = 0;		
		Position = 0;
		uDetailCarType = 6;
        uStatusType = 0;
        uChannelID =0;
        uEventDefine = 0;
        uTime2 = 0;
        uMiTime2 = 0;
		uDensity  = 0;
		nVideoFlag = 0;
		memset(chReserved,0,4);
	}
}RECORD_EVENT;


//车牌记录结构
typedef struct _RECORD_PLATE
{
	UINT32 uSeq;						//序列号
	UINT32 uTime;						//识别车牌时间(秒)
	UINT32 uMiTime;					//识别车牌时间(毫秒)
	char chText[MAX_PLATE];					//车牌文本
	UINT32 uColor;					//车牌类型（颜色）

	UINT32 uCredit;					//识别可靠度
	UINT32 uRoadWayID;				//车道号

	UINT32 uType;						//车辆类型(低16位大、中、小，高16位卡车、巴士、轿车等）

	UINT32 uSmallPicSize;				//车牌小图大小
	UINT32 uSmallPicWidth;			//车牌小图宽度
	UINT32 uSmallPicHeight;			//车牌小图高度

	UINT32 uPicSize;					//车牌全景图片大小
	UINT32 uPicWidth;					//车牌全景图片宽度
	UINT32 uPicHeight;				//车牌全景图片高度

	UINT32 uPosLeft;					//车牌在全景图片中的位置左
	UINT32 uPosTop;					//车牌在全景图片中的位置上
	UINT32 uPosRight;					//车牌在全景图片中的位置右
	UINT32 uPosBottom;				//车牌在全景图片中的位置下


	UINT32 uCarColor1;				//车身颜色
	UINT32 uSpeed;					//车速
	UINT32 uDirection;				//行驶方向
	UINT32 uCarBrand;				//产商标志
	char chPlace[64];				//经过地点
	char chVideoPath[MAX_VIDEO];				//录像路径
	char chPicPath[MAX_VIDEO];				//大图片路径

	UINT32 uCarColor2;                    //车身颜色2

	UINT32 uWeight1;                    //车身颜色权重1
	UINT32 uWeight2;                    //车身颜色权重2

    UINT32 uSeqID;                      //帧序号
	UINT32 uPlateType;            //车牌结构
	UINT32 uViolationType;       //违章类型(闯红灯等)

	UINT32 uTypeDetail;       //车型细分
	UINT32 uStatusType;       //记录类型
	UINT32 Position;                   // 图象帧在文件中的开始位置（单位为字节）

	UINT32 uChannelID;                  //通道编号

	UINT32 uLongitude;//地点纬度(实际精度*1000000, 精确到小数点后六位,单位为度)
	UINT32 uLatitude; //地点经度(实际精度*1000000, 精确到小数点后六位,单位为度)
	UINT32 uTime2;						//第二车牌时间(秒)
	UINT32 uMiTime2;					//第二车牌时间(毫秒)
	UINT32 uAlarmKind;            //黑白名单报警1黑名单；2白名单
	UINT32 uSignalTime;				//红灯时间
	UINT32 uRedLightBeginTime;                    //红灯开始时间(秒)
	UINT32 uRedLightBeginMiTime;                  //红灯开始时间(毫秒)
	UINT32 uRedLightEndTime;                    //红灯结束时间(秒)
	UINT32 uRedLightEndMiTime;                  //红灯结束时间(毫秒)
	UINT32 uLimitSpeed;                    //限速值
	UINT32 uOverSpeed;                  //超速起拍值

	UINT32 uCameraId;//相机ID
	UINT32 uDetectDirection; //检测运动方向，0:前牌（从远到近）,1:后牌（从近到远）

	UINT32 uDetailCarType;       //车型细分(由uTypeDetail决定更细的车型细分结果)
	UINT32 uDetailCarBrand;				//厂商标志细分(由uCarBrand决定更细的厂商标志细分结果)

	UINT32 uCarPosLeft;//车辆在全景图片中的位置左
	UINT32 uCarPosTop;//车辆在全景图片中的位置上
	UINT32 uCarPosRight;//车辆在全景图片中的位置右
	UINT32 uCarPosBottom;//车辆在全景图片中的位置下	

	char szCameraCode[16];//相机编号(公交模式)
	char szLoctionID[16]; //地点编号(公交模式)
	char szKaKouItem[16]; //卡口编号(公交模式)
	UINT32 uBeltResult;//是否安全带
	UINT32 uPhoneResult;//是否打手机
	UINT32 uSunVisorResult;//是否有遮阳板
	char chReserved[36];//扩展
	_RECORD_PLATE()
	{
		uSeq = 0;
		uTime = 0;
		uMiTime = 0;
		memset(chText,0,MAX_PLATE);
		uColor = 1;

		uCredit = 90;
		uRoadWayID = 0;

		uType = 0;

		uSmallPicSize = 0;
		uSmallPicWidth = 0;
		uSmallPicHeight = 0;

		uPicSize = 0;
		uPicWidth = 0;
		uPicHeight = 0;

		uPosLeft = 0;
		uPosTop = 0;
		uPosRight = 0;
		uPosBottom = 0;

		uSpeed = 0;
		uCarColor1 = 11;//未知
		uDirection = 0;
#ifdef GLOBALCARLABEL
		uCarBrand = 200000;
#else
		uCarBrand = 1000;
#endif
		memset(chPlace,0,64);
		memset(chVideoPath,0,MAX_VIDEO);
		memset(chPicPath,0,MAX_VIDEO);

		uCarColor2 = 11;
		uWeight1 = 100;
		uWeight2 = 0;

        uAlarmKind = 0;
        uPlateType = 0;
        uViolationType = 0;

        uSeqID = 0;
        uTypeDetail = 0;
        uStatusType = 0;
        Position = 0;

		uLongitude = 0;
		uLatitude = 0;

        uChannelID = 0;
        uTime2 = 0;
        uMiTime2 = 0;
		uSignalTime = 0;
		uRedLightBeginTime = 0;
		uRedLightBeginMiTime = 0;
		uRedLightEndTime = 0;
		uRedLightEndMiTime = 0;
		uLimitSpeed = 0;
		uOverSpeed = 0;

		uCameraId = 0;
		uDetectDirection = 0;

		uDetailCarType = 0;
		uDetailCarBrand = 0;

		uCarPosLeft = 0;
		uCarPosTop = 0;
		uCarPosRight = 0;
		uCarPosBottom = 0;
		memset(szCameraCode, 0, 16);
		memset(szLoctionID, 0, 16);
		memset(szKaKouItem, 0, 16);
		uBeltResult = 0;
		uPhoneResult = 0;
		uSunVisorResult = 0;
		memset(chReserved, 0, 36);
	}
}RECORD_PLATE;

//与dsp相机通讯的车牌记录结构
typedef struct _RECORD_PLATE_DSP
{
    UINT64 uSeq;//帧号-UINT64
    UINT32 uTime;//识别车牌时间(秒)
    UINT32 uMiTime;//识别车牌时间(毫秒)
    char chText[MAX_PLATE];//车牌文本MAX_PLATE=16
    UINT32 uColor;//车牌类型（颜色）
    UINT32 uCredit;//识别可靠度
    UINT32 uRoadWayID;//车道号

    UINT32 uType;	//车辆类型(低16位大、中、小，高16位卡车、巴士、轿车等）
    UINT32 uSmallPicSize;	//车牌小图大小
    UINT32 uSmallPicWidth;	//车牌小图宽度
    UINT32 uSmallPicHeight;//车牌小图高度
    UINT32 uPicSize;//车牌全景图片大小
    UINT32 uPicWidth;//车牌全景图片宽度
    UINT32 uPicHeight;//车牌全景图片高度
    UINT32 uPosLeft;//车牌在全景图片中的位置左
    UINT32 uPosTop;//车牌在全景图片中的位置上
    UINT32 uPosRight;//车牌在全景图片中的位置右
    UINT32 uPosBottom;//车牌在全景图片中的位置下

    UINT32 uCarColor1;//车身颜色
    UINT32 uSpeed;//车速
    UINT32 uDirection;//行驶方向
    UINT32 uCarBrand;//产商标志

    UINT32 uCarColor2;//车身颜色2
    UINT32 uWeight1;//车身颜色权重1
    UINT32 uWeight2;//车身颜色权重2

    UINT32 uPlateType;//车牌结构（单行，双行）
    UINT32 uViolationType;//违章类型(闯红灯。。等)

    UINT32 uTypeDetail;//车型细分
    UINT64 uSeq2;//第二张图帧号-UINT64
    UINT32 uTime2;//第二车牌时间(秒)
    UINT32 uMiTime2;//第二车牌时间(毫秒)

    UINT32 uContextMean; //检测区域平均亮度（实际平均亮度取整数部分即可）
    UINT32 uContextStddev; //检测区域平均亮度方差（实际方差结果*100，精确到小数点后两位）
    UINT32 uVerticalTheta; //车牌垂直倾斜角度（默认为0）
    UINT32 uHorizontalTheta; //车牌水平倾斜角度

    UINT16 nPlateNums; //对应一帧图片里存在的车牌个数(1为基数，最大值为4)
    UINT16 nPlateOrder;//对应此条记录里车牌的序号（base 1）
	
	UINT32 uLongitude;//地点纬度(实际精度*1000000, 精确到小数点后六位,单位为度)
	UINT32 uLatitude; //地点经度(实际精度*1000000, 精确到小数点后六位,单位为度)

	UINT32 uCameraId;//相机ID

	UINT64 uSeq3;//第三张图帧号
	UINT64 uRedLightStartTime; //红灯开启时间
	UINT64 uUpperFrm; //取秒图时，判断第一张图的上限
	UINT64 uLowerFrm;  //取秒图时，判断第一张图的下限
	UINT32 uDis[3];// 在闯红灯事件中用于表示帧图片离红灯开始帧的间隔（帧数）

	UINT32 uReserve;//预留字节
	UINT64 uSeq4;//卡口记录图片帧号
	
	UINT32 uCarPosLeft;//车辆在全景图片中的位置左
	UINT32 uCarPosTop;//车辆在全景图片中的位置上
	UINT32 uCarPosRight;//车辆在全景图片中的位置右
	UINT32 uCarPosBottom;//车辆在全景图片中的位置下	
	
	char chReserved[168];//扩展，预留字节

    _RECORD_PLATE_DSP()
	{
		uSeq = 0;
		uTime = 0;
		uMiTime = 0;
		memset(chText,0,MAX_PLATE);
		uColor = 1;

		uCredit = 90;
		uRoadWayID = 0;

		uType = 0;

		uSmallPicSize = 0;
		uSmallPicWidth = 0;
		uSmallPicHeight = 0;

		uPicSize = 0;
		uPicWidth = 0;
		uPicHeight = 0;

		uPosLeft = 0;
		uPosTop = 0;
		uPosRight = 0;
		uPosBottom = 0;

		uCarColor1 = 11;//未知
		uSpeed = 0;
		uDirection = 0;
#ifdef GLOBALCARLABEL
		uCarBrand = 200000;
#else
		uCarBrand = 1000;
#endif

		uCarColor2 = 11;
		uWeight1 = 100;
		uWeight2 = 0;

        uPlateType = 0;
        uViolationType = 0;

        uTypeDetail = 0;

        uSeq2 = 0;
        uTime2 = 0;
        uMiTime2 = 0;

        uContextMean = 0;
        uContextStddev = 0;
        uVerticalTheta = 0;
        uHorizontalTheta = 0;

        nPlateNums = 0;
        nPlateOrder = 0;

		uLongitude = 0;
		uLatitude = 0;

		uCameraId = 0;
		uSeq3 = 0;
		uRedLightStartTime = 0;
		uUpperFrm = 0;
		uLowerFrm = 0;
		memset(uDis,0,12);
		uReserve = 0;

		uCarPosLeft = 0;
		uCarPosTop = 0;
		uCarPosRight = 0;
		uCarPosBottom = 0;

		uSeq4 = 0;
		memset(chReserved, 0, 168);
	}
}RECORD_PLATE_DSP;

//车牌结果列表
typedef std::list<RECORD_PLATE_DSP> DSP_PLATE_LIST;

//bocom中心端事件记录结构
typedef struct _BOCOM_RECORD_EVENT
{
	UINT32 uSeq;						//序列号
	UINT32 uVideoBeginTime;			//录象开始时间(秒)
	UINT32 uMiVideoBeginTime;			//录象开始时间(毫秒)
	UINT32 uVideoEndTime;				//录象结束时间(秒)
	UINT32 uMiVideoEndTime;			//录象结束时间(毫秒)
	UINT32 uEventBeginTime;			//事件开始时间(秒)
	UINT32 uMiEventBeginTime;			//事件开始时间(毫秒)
	UINT32 uEventEndTime;				//事件结束时间(秒)
	UINT32 uMiEventEndTime;			//事件结束时间(毫秒)

	UINT32 uCode;						//事件代码
	UINT32 uRoadWayID;				//车道号

	UINT32 uPicSize;					//快照图片大小
	UINT32 uPicWidth;					//快照图片宽度
	UINT32 uPicHeight;				//快照图片高度
	UINT32 uPosX;						//事件横坐标
	UINT32 uPosY;						//事件纵坐标

	char chText[MAX_EVENT];					//事件类型描述
	char chVideoPath[MAX_VIDEO];			//录象存储位置

	UINT32 uType;						//车类型
	UINT32 uColor1;                    //车身颜色1
	UINT32 uSpeed;					//车速
	UINT32 uDirection;                //行驶方向

	char chPlace[MAX_PLACE];				//事件地点
	char chPicPath[MAX_VIDEO];				//事件图片路径

	UINT32 uColor2;                    //车身颜色2
	UINT32 uColor3;                    //车身颜色3

	UINT32 uWeight1;                    //车身颜色权重1
	UINT32 uWeight2;                    //车身颜色权重2
	UINT32 uWeight3;                    //车身颜色权重3

	UINT32 uDetailCarType;                 //车型细分
    ////////////////////////////////////////
	char chReserved[40];				//扩展

	_BOCOM_RECORD_EVENT()
	{
		uSeq = 0;
		uVideoBeginTime = 0;
		uMiVideoBeginTime = 0;
		uVideoEndTime = 0;
		uMiVideoEndTime = 0;
		uEventBeginTime = 0;
		uMiEventBeginTime = 0;
		uEventEndTime = 0;
		uMiEventEndTime = 0;

		uCode = 0;
		uRoadWayID = 0;

		uPicSize = 0;
		uPicWidth = 0;
		uPicHeight = 0;
		uPosX = 0;
		uPosY = 0;

		uColor1 = 11;
		uType = 0;
		uSpeed = 0;
		uDirection = 0;

		memset(chText,0,MAX_EVENT);
		memset(chVideoPath,0,MAX_VIDEO);
		memset(chPlace,0,MAX_PLACE);
		memset(chPicPath,0,MAX_VIDEO);

		uColor2 = 11;
		uColor3 = 11;

		uWeight1 = 100;
		uWeight2 = 0;
		uWeight3 = 0;

		uDetailCarType = 6;

		memset(chReserved,0,40);
	}
}BOCOM_RECORD_EVENT;

//bocom中心端车牌记录结构
typedef struct _RECORD_PLATE_SERVER
{
	UINT32 uSeq;						//序列号
	UINT32 uTime;						//识别车牌时间(秒)
	UINT32 uMiTime;					//识别车牌时间(毫秒)
	char chText[MAX_PLATE];					//车牌文本
	UINT32 uColor;					//车牌类型（颜色）

	UINT32 uCredit;					//识别可靠度
	UINT32 uRoadWayID;				//车道号

	UINT32 uType;						//车辆类型(低16位大、中、小，高16位卡车、巴士、轿车等）

	UINT32 uSmallPicSize;				//车牌小图大小
	UINT32 uSmallPicWidth;			//车牌小图宽度
	UINT32 uSmallPicHeight;			//车牌小图高度

	UINT32 uPicSize;					//车牌全景图片大小
	UINT32 uPicWidth;					//车牌全景图片宽度
	UINT32 uPicHeight;				//车牌全景图片高度

	UINT32 uPosLeft;					//车牌在全景图片中的位置左
	UINT32 uPosTop;					//车牌在全景图片中的位置上
	UINT32 uPosRight;					//车牌在全景图片中的位置右
	UINT32 uPosBottom;				//车牌在全景图片中的位置下


	UINT32 uCarColor1;				//车身颜色
	UINT32 uSpeed;					//车速
	UINT32 uDirection;				//行驶方向
	UINT32 uCarBrand;				//产商标志
	char chPlace[MAX_PLACE];				//经过地点
	char chVideoPath[MAX_VIDEO];				//录像路径
	char chPicPath[MAX_VIDEO];				//大图片路径

	UINT32 uCarColor2;                    //车身颜色2

	UINT32 uWeight1;                    //车身颜色权重1
	UINT32 uWeight2;                    //车身颜色权重2

    UINT32 uDetailCarType;       //车型细分
	UINT32 uPlateType;            //车牌结构
	UINT32 uViolationType;       //违章类型(闯红灯等)
	UINT32 uSeqID;                      //帧序号

	UINT32 uTime2;                    //第二车牌时间(秒)
	UINT32 uMiTime2;                  //第二车牌时间(毫秒)
	UINT32 uRedLightBeginTime;                    //红灯开始时间(秒)
	UINT32 uRedLightBeginMiTime;                  //红灯开始时间(毫秒)
	UINT32 uRedLightEndTime;                    //红灯结束时间(秒)
	UINT32 uRedLightEndMiTime;                  //红灯结束时间(毫秒)
	UINT32 uLimitSpeed;                    //限速值
	UINT32 uOverSpeed;                  //超速起拍值
    char chReserved[4];               //扩展

	_RECORD_PLATE_SERVER()
	{
		uSeq = 0;
		uTime = 0;
		uMiTime = 0;
		memset(chText,0,MAX_PLATE);
		uColor = 1;

		uCredit = 90;
		uRoadWayID = 0;

		uType = 0;

		uSmallPicSize = 0;
		uSmallPicWidth = 0;
		uSmallPicHeight = 0;

		uPicSize = 0;
		uPicWidth = 0;
		uPicHeight = 0;

		uPosLeft = 0;
		uPosTop = 0;
		uPosRight = 0;
		uPosBottom = 0;

		uSpeed = 0;
		uCarColor1 = 11;//未知
		uDirection = 0;
#ifdef GLOBALCARLABEL
		uCarBrand = 200000;
#else
		uCarBrand = 1000;
#endif
		memset(chPlace,0,MAX_PLACE);
		memset(chVideoPath,0,MAX_VIDEO);
		memset(chPicPath,0,MAX_VIDEO);

		uCarColor2 = 11;
		uWeight1 = 100;
		uWeight2 = 0;

        uDetailCarType = 6;
        uPlateType = 0;
        uViolationType = 0;
        uSeqID = 0;

	    uTime2 = 0;
	    uMiTime2 = 0;

		uRedLightBeginTime = 0;
		uRedLightBeginMiTime = 0;
		uRedLightEndTime = 0;
		uRedLightEndMiTime = 0;
		uLimitSpeed = 0;
		uOverSpeed = 0;

		memset(chReserved,0,4);
	}
}RECORD_PLATE_SERVER;

//bocom客户端车牌记录结构
typedef struct _RECORD_PLATE_CLIENT
{
	UINT32 uSeq;						//序列号
	UINT32 uTime;						//识别车牌时间(秒)
	UINT32 uMiTime;					//识别车牌时间(毫秒)
	char chText[MAX_PLATE];					//车牌文本
	UINT32 uColor;					//车牌类型（颜色）

	UINT32 uCredit;					//识别可靠度
	UINT32 uRoadWayID;				//车道号

	UINT32 uType;						//车辆类型(低16位大、中、小，高16位卡车、巴士、轿车等）

	UINT32 uSmallPicSize;				//车牌小图大小
	UINT32 uSmallPicWidth;			//车牌小图宽度
	UINT32 uSmallPicHeight;			//车牌小图高度

	UINT32 uPicSize;					//车牌全景图片大小
	UINT32 uPicWidth;					//车牌全景图片宽度
	UINT32 uPicHeight;				//车牌全景图片高度

	UINT32 uPosLeft;					//车牌在全景图片中的位置左
	UINT32 uPosTop;					//车牌在全景图片中的位置上
	UINT32 uPosRight;					//车牌在全景图片中的位置右
	UINT32 uPosBottom;				//车牌在全景图片中的位置下


	UINT32 uCarColor1;				//车身颜色
	UINT32 uSpeed;					//车速
	UINT32 uDirection;				//行驶方向
	UINT32 uCarBrand;				//产商标志
	char chPlace[64];				//经过地点
	char chVideoPath[MAX_VIDEO];				//录像路径
	char chPicPath[MAX_VIDEO];				//大图片路径

	UINT32 uCarColor2;                    //车身颜色2

	UINT32 uWeight1;                    //车身颜色权重1
	UINT32 uWeight2;                    //车身颜色权重2

    UINT32 uDetailCarType;       //车型细分(由uTypeDetail决定更细的车型细分结果)
	UINT32 uPlateType;            //车牌结构
	UINT32 uViolationType;       //违章类型(闯红灯等)

	UINT32 uTypeDetail;       //车型细分
	UINT32 uStatusType;       //记录类型
	UINT32 Position;                   // 图象帧在文件中的开始位置（单位为字节）

	UINT32 uChannelID;                  //通道编号

	UINT32 uLongitude;//地点纬度(实际精度*1000000, 精确到小数点后六位,单位为度)
	UINT32 uLatitude; //地点经度(实际精度*1000000, 精确到小数点后六位,单位为度)
	UINT32 uTime2;						//第二车牌时间(秒)
	UINT32 uMiTime2;					//第二车牌时间(毫秒)
	UINT32 uAlarmKind;            //黑白名单报警1黑名单；2白名单
	UINT32 uDetailCarBrand;				//厂商标志细分(由uCarBrand决定更细的厂商标志细分结果)

	_RECORD_PLATE_CLIENT()
	{
		uSeq = 0;
		uTime = 0;
		uMiTime = 0;
		memset(chText,0,MAX_PLATE);
		uColor = 1;

		uCredit = 90;
		uRoadWayID = 0;

		uType = 0;

		uSmallPicSize = 0;
		uSmallPicWidth = 0;
		uSmallPicHeight = 0;

		uPicSize = 0;
		uPicWidth = 0;
		uPicHeight = 0;

		uPosLeft = 0;
		uPosTop = 0;
		uPosRight = 0;
		uPosBottom = 0;

		uSpeed = 0;
		uCarColor1 = 11;//未知
		uDirection = 0;
#ifdef GLOBALCARLABEL
		uCarBrand = 200000;
#else
		uCarBrand = 1000;
#endif
		memset(chPlace,0,64);
		memset(chVideoPath,0,MAX_VIDEO);
		memset(chPicPath,0,MAX_VIDEO);

		uCarColor2 = 11;
		uWeight1 = 100;
		uWeight2 = 0;

        uAlarmKind = 0;
        uPlateType = 0;
        uViolationType = 0;

        uDetailCarType = 0;
        uTypeDetail = 0;
        uStatusType = 0;
        Position = 0;

		uLongitude = 0;
		uLatitude = 0;

        uChannelID = 0;
        uTime2 = 0;
        uMiTime2 = 0;
		uDetailCarBrand = 0;


	}
}RECORD_PLATE_CLIENT;


//车牌特征记录结构
typedef struct _RECORD_PLATE_TEXTURE
{
	UINT32 uSeq;						//序列号
    char chText[MAX_PLATE];					//车牌文本
	char chSmallPicPath[MAX_VIDEO];		//小图片路径
	char chPicPath[MAX_VIDEO];				//大图片路径
	int nType;//车辆类型
	int nDetailedType; //车辆细分
	int uTimestamp; //时间
	int nColor;//车身颜色
	float fSpeed;//km/h
    UINT16 nTexture[DIM_FEATURE];         //特征向量400维

	_RECORD_PLATE_TEXTURE()
	{
		uSeq = 0;
        memset(chText,0,MAX_PLATE);
        memset(chSmallPicPath,0,MAX_VIDEO);
		memset(chPicPath,0,MAX_VIDEO);
        memset(nTexture, 0, DIM_FEATURE*2);

	}
}RECORD_PLATE_TEXTURE;

//检测器基本配置
typedef struct _MIMAX_CHANNEL_CONFIG
{
	UINT32 uType;			      //类型
	UINT32 uFmt;			      //视频源
	UINT32 uCameraID;			  //相机编号
	//char chDirection[MAX_DIRECTION];  //检测方向
	UINT32 uDirection; //检测方向
	char chPlace[MAX_PLACE];	      //安装地点
	UINT32 uRoadWayID;           //车道编号
	UINT32 uPannelID;			  //断面编号
	char chReserved[56];

	_MIMAX_CHANNEL_CONFIG()
	{
		uType = 0;
		uFmt = 0;
		uCameraID = 0;
		//memset(chDirection,0,MAX_DIRECTION);
		uDirection = 0;
		memset(chPlace,0,MAX_PLACE);
		uRoadWayID = 0;
		uPannelID = 0;
		memset(chReserved,0,56);
	}

}MIMAX_CHANNEL_CONFIG;

//系统配置
typedef struct _SYSTEM_CONFIG
{
	char chServerHost[SKP_MAX_HOST];  //服务地址
	char chNetMask[SKP_MAX_HOST];  //子网掩码
	char chGateWay[SKP_MAX_HOST];  //默认网关
	char chCameraHost[SKP_MAX_HOST]; //相机网卡地址
	UINT32 uNetMTU;	      //服务器网卡MTU值(eth1)

	char chVideoPath[MAX_VIDEO];	 //录象路径
	char chPicPath[MAX_VIDEO];	 //图片路径
	char chBackupPath[92]; //备份路径
	UINT32 uCameraNetMTU;	      //服务器网卡MTU值(eth0)
	char chCameraNetMask[SKP_MAX_HOST];  //子网掩码
	char chCameraGateWay[SKP_MAX_HOST];  //默认网关
	char chVersion[SKP_MAX_HOST]; //程序版本

	UINT32 uDiskDay;            //磁盘清理时间间隔
	UINT32 uTimeStamp;          //服务器时间

	char chDBHost[SKP_MAX_HOST];      //DB地址
	UINT32 uDBPort;	          //DB端口
	char chDBName[SKP_MAX_HOST];	 //DB名称
	char chDBUser[MIMAX_USERNAME];	//DB用户
	char chDBPass[MIMAX_USERPASS];  //DB密码

	char chAuthenticationHost[SKP_MAX_HOST];  //认证服务器地址
	char chSynClockHost[SKP_MAX_HOST];      //时钟服务器地址

	_SYSTEM_CONFIG()
	{
		memset(chServerHost,0,SKP_MAX_HOST);
		memset(chNetMask,0,SKP_MAX_HOST);
		memset(chGateWay,0,SKP_MAX_HOST);
		memset(chCameraHost,0,SKP_MAX_HOST);
		uNetMTU = 1500;

		memset(chVideoPath,0,MAX_VIDEO);
		memset(chPicPath,0,MAX_VIDEO);
		memset(chBackupPath,0,92);
		uCameraNetMTU = 1500;
		memset(chCameraNetMask,0,SKP_MAX_HOST);
		memset(chCameraGateWay,0,SKP_MAX_HOST);
		memset(chVersion,0,SKP_MAX_HOST);

		uDiskDay = 7;
		uTimeStamp = 0;

		memset(chDBHost,0,SKP_MAX_HOST);
		uDBPort = 3306;
		memset(chDBName,0,SKP_MAX_HOST);
		memset(chDBUser,0,MIMAX_USERNAME);
		memset(chDBPass,0,MIMAX_USERPASS);

		memset(chAuthenticationHost,0,SKP_MAX_HOST);
		memset(chSynClockHost,0,SKP_MAX_HOST);
	}

}SYSTEM_CONFIG;

//扩展系统设置
typedef struct _SYSTEM_CONFIG_EX
{
	char chControlServerHost[SKP_MAX_HOST];  //中心控制服务器地址
	UINT32 uControlServerPort;	      //中心控制服务器端口
	char chDetectorID[SKP_MAX_HOST]; //检测器编号

    UINT32 nHasCenterServer;//是否有中心端
    UINT32 nServerType;//中心端类型
    UINT32 nWorkMode;//是否全帧率输出图像
    UINT32 nSendViolationOnly;//是否只发送违章数据
    UINT32 nDoSynProcess;//是否相机同步检测
    UINT32 nWriteBack;//是否回写
    UINT32 nWriteBackTime;//回写等待时间
    UINT32 nCheckShake;//是否抖动检测
    UINT32 nModelID;//系统设置模板编号
    UINT32 nSaveImageCount;//存图数量
    UINT32 nDetectSpecialCarNum;//是否布控报警
    UINT32 nDspLog;//存储相机日志
    UINT32 nSwitchCamera;//是否需要切换相机
    UINT32 nFlashControl;//是否视频爆闪控制(判断是否打开视频爆闪串口以及进行算法预判车辆)
    UINT32 nHistoryPlayMode;//0：正常播放,1:快速播放
    UINT32 nPicMode;//卡口图片组合方式:0:叠加存放,1:分开存放
    UINT32 nNtpServer;//是否启动ntp-server,默认不启动
    UINT32 nClockMode;//校时方式 0:NTP校时,1:GPS校时
	char chLocalPlate[8];//本地车牌号码(UTF8编码)
	UINT32 nLoadBasePlateInfo;//是否需要载入违章车牌数据
	UINT32 nFtpServer;//是否ftp服务端
	UINT32 nSendImage;//是否发送图片给中心端
	UINT32 nDetectByTime;//是否按时间段检测
	UINT32 nBeginDetectTime;//开始检测时间
	UINT32 nEndDetectTime;//结束检测时间

	UINT32 nGongJiaoMode; //是否公交模式 0：否 1：是

	UINT32 nDetectMode;//检测器工作模式,0:连续模式(默认);1:触发模式;2:DSP模式
    UINT32 nVtsPicMode;//闯红灯图片组合方式:0:3x1,1:2x2
    UINT32 nHasExpoMonitor;//是否有智能控制器
    UINT32 nHasHighLight;//是否有爆闪灯(线圈爆闪以及视频爆闪中均会用到此项)
	UINT32 nSendHistoryRecord;      //是否发送历史记录

	_SYSTEM_CONFIG_EX()
	{
		memset(chControlServerHost,0,SKP_MAX_HOST);
		uControlServerPort = 0;
        memset(chDetectorID,0,SKP_MAX_HOST);
        nHasCenterServer = 1;
        nServerType = 0;
        nWorkMode = 0;
        nSendViolationOnly = 0;
        nDoSynProcess = 0;
        nWriteBack = 0;
        nWriteBackTime = 5;
        nCheckShake = 0;
        nModelID = 0;
        nSaveImageCount = 1;
        nDetectSpecialCarNum = 0;
        nDspLog = 0;
        nSwitchCamera =0;
        nFlashControl = 0;
        nHistoryPlayMode = 0;
        nPicMode = 0;
        nNtpServer = 0;
        nClockMode = 0;
		memset(chLocalPlate,0,8);
		nLoadBasePlateInfo = 0;
		nFtpServer = 0;
		nSendImage = 1;
		nDetectByTime= 0;
        nBeginDetectTime = 0;
        nEndDetectTime = 0;

        nGongJiaoMode = 0;

		nDetectMode = 0;
        nVtsPicMode = 0;
        nHasExpoMonitor = 0;
        nHasHighLight = 0;
		nSendHistoryRecord = 0;
	}

}SYSTEM_CONFIG_EX;

//FTP服务器设置
typedef struct _FTP_HOST_INFO
{
	char chFtpServerHost[SKP_MAX_HOST];//ftp服务器地址
    char chFtpUserName[SKP_MAX_HOST];//ftp用户名
    char chFtpPassword[SKP_MAX_HOST];//ftp密码
    UINT32 uFtpPort;	      //ftp端口

    _FTP_HOST_INFO()
    {
        memset(chFtpServerHost,0,SKP_MAX_HOST);
        memset(chFtpUserName,0,SKP_MAX_HOST);
        memset(chFtpPassword,0,SKP_MAX_HOST);
        uFtpPort = 0;
    }
}FTP_HOST_INFO;

//监控主机设置
typedef struct _MONITOR_HOST_INFO
{
    char chMonitorHost[SKP_MAX_HOST];  //监控主机地址
	UINT32 uMonitorPort;	      //监控主机端口
	char chUserName[MIMAX_USERNAME];  //用户名
	char chPassWord[MIMAX_USERPASS];  //密码
	char chSipServerCode[32];//sip服务器编号
	char chSipClientCode[32];//sip客户端编号
	//char chReserved[64];

    _MONITOR_HOST_INFO()
    {
        uMonitorPort = 0;
        memset(chMonitorHost,0,SKP_MAX_HOST);
        memset(chUserName,0,MIMAX_USERNAME);
        memset(chPassWord,0,MIMAX_USERPASS);
		memset(chSipServerCode,0,32);
		memset(chSipClientCode,0,32);
        //memset(chReserved,0,64);
    }
}MONITOR_HOST_INFO;

//NTP对时点设置
typedef struct _SET_NTP_TIME_INFO
{ 
	//nTime1,nTime2 中值除以100得小时，余得秒 
	int nTime1;// 第一次对时时间 753 表示7:53对时
	int nTime2;// 第二次对时时间 1953 表示19:53对时
	int nTime3;// 每一个小时对时 16 表示每小时的16分对时
	char chHost[SKP_MAX_HOST];
	char szReserved[256];

	_SET_NTP_TIME_INFO()
	{
		nTime1 = 0;
		nTime2 = 0;
		nTime3 = 0;
		memset(chHost,0,SKP_MAX_HOST);
		memset(szReserved,0,256);
	} 
}SET_NTP_TIME_INFO;

//比对服务器设置
typedef struct _MATCH_HOST_INFO
{
    char chMatchHost[SKP_MAX_HOST];  //比对服务器地址
	UINT32 uMatchPort;	      //比对服务器端口
    UINT32 uHasMatchHost;   //是否有比对服务器
	char chReserved[32];

    _MATCH_HOST_INFO()
    {
        uMatchPort = 0;
        uHasMatchHost = 0;
        memset(chMatchHost,0,SKP_MAX_HOST);
        memset(chReserved,0,32);
    }
}MATCH_HOST_INFO;


//区间测速主机设置
typedef struct _DISTANCE_HOST_INFO
{
    char chDistanceHost[SKP_MAX_HOST];  //区间测速主机地址
	UINT32 uDistancePort;	      //区间测速主机端口
	UINT32 uDistance;//区间距离，单位为米
    UINT32 bDistanceCalculate;//是否需要区间测速
	UINT32 uUsbCopyTime;          //u盘拷贝开始时间
	UINT32 bCopyVideo;          //是否拷贝录像
	char chReserved[16];

    _DISTANCE_HOST_INFO()
    {
        uDistancePort = 0;
        memset(chDistanceHost,0,SKP_MAX_HOST);
		uDistance = 0;
		bDistanceCalculate = 0;
		uUsbCopyTime = 0;
		bCopyVideo = 0;
        memset(chReserved,0,16);
    }
}DISTANCE_HOST_INFO;


//应用管理服务器设置
typedef struct _AMS_HOST_INFO
{
	char chAmsHost[SKP_MAX_HOST];  //应用管理服务器地址
	UINT32 uAmsPort;	      //应用管理服务器端口
	UINT32 uHasAmsHost;   //是否有应用管理服务器
	UINT32 uBakType; //是否备份机器 0:否 1:是
	char chReserved[28];

	_AMS_HOST_INFO()
	{
		uAmsPort = 41070;
		uHasAmsHost = 0;
		uBakType = 0;
		memset(chAmsHost,0,SKP_MAX_HOST);
		memset(chReserved,0,28);
	}
}AMS_HOST_INFO;

//智能控制器参数设置
typedef struct _EXPO_MONITOR_INFO
{
    char chExpoMonitorHost[SKP_MAX_HOST];  //智能监控主机地址
	UINT32 uExpoMonitorPort;	      //智能监控主机端口

	int nGateValue; //门当前状态 0:关门 1:开门
    int nGateStateAlarm; //门报警状态 0:关 1:开

    int nTemperatureValue;//环境温度当前状态
    int nTemperatureUp;//环境温度上限 70
    int nTemperatureDown;//环境温度下限 -10
    int nTemperatureAlarm;//环境温度报警状态 0:关 1:开

    UINT32 uSysTime; //系统时间
	char chReserved[36];

    _EXPO_MONITOR_INFO()
    {
        uExpoMonitorPort = 0;
        memset(chExpoMonitorHost,0,SKP_MAX_HOST);

        nGateValue = -1;
        nGateStateAlarm = 1;

        nTemperatureValue = -100;
        nTemperatureUp = -100;
        nTemperatureDown = -100;
        nTemperatureAlarm = 1;

        uSysTime = 0;
        memset(chReserved,0,36);
    }
}EXPO_MONITOR_INFO;

//Dsp相机服务器主机信息
typedef struct _DSP_SERVER_HOST_INFO
{
    UINT32 uDspServerHost;  //Dsp相机服务器主机地址->转成的4个字节数字p1:p2:p3:p4，从低位到高位
	UINT32 uDspServerPort;	//Dsp相机服务器主机端口

	char chReserved[52];

	_DSP_SERVER_HOST_INFO()
	{
	    uDspServerHost = 0;
	    uDspServerPort = 0;

	    memset(chReserved,0,52);
	}
}DSP_SERVER_HOST_INFO;

//图片格式信息
typedef struct _PIC_FORMAT_INFO
{
    int nWordPos;// 0：字在图下方；1：字在图上方
    int nForeColor;// 0：白色；1：黑色;2:红色;3:黄色;4:绿色;5;蓝色；6:自动
    int nBackColor;// 0：白色；1：黑色
    int nFont;// 0：楷体；1：黑体；2：宋体
    int nCarColor;// 0：不需要叠加；1：需要叠加
    int nCarType;// 0：不需要叠加；1：需要叠加
    int nCarBrand;// 0：不需要叠加；1：需要叠加
    int nCarSpeed;// 0：不需要叠加；1：需要叠加
    int nViolationType;// 0：不需要叠加；1：需要叠加
    int nCarNum;//0: 不需要叠加车牌；1：需要叠加
    int nRoadIndex;//0: 不需要叠加车道；1：需要叠加
    int nFontSize;//字体大小
    int nExtentHeight; //文字区域高度
    int nWordOnPic;//是否需要在图片上叠加文字
    int nSmallPic;//是否需要存储小图
	int nJpgQuality;//JPG压缩比例
	int nPicFlag;//是否叠加防伪码
	int nSmallViolationPic;//违章图片是否需要叠加小图
	int nOffsetX;//x方向偏移量
	int nOffsetY;//y方向偏移量
	int nSecondCarColor;//是否叠加第二颜色
	int nResizeScale;//缩放比例
	int nSpeedLimit;//是否叠加限速值
	int nSpaceRegion;//是否存在空白区域
	int nWordLine;//文字行数


    _PIC_FORMAT_INFO()
    {
       nWordPos = 0;
	   nForeColor = 0;
       nBackColor = 1;
	   nFont = 0;
	   nCarColor = 1;
       nCarType = 1;
       nCarBrand = 1;
		nCarSpeed = 1;
		nViolationType = 0;
        nCarNum = 1;
        nRoadIndex = 1;
        nFontSize = 25;
 	    nExtentHeight = 60;
        nWordOnPic = 0;
        nSmallPic = 0;
		nJpgQuality = 70;
		nPicFlag = 0;
		nSmallViolationPic = 0;
		nOffsetX = 0;
		nOffsetY = 0;
		nSecondCarColor = 1;
		nResizeScale = 100;
		nSpeedLimit = 0;
		nSpaceRegion = 0;
		nWordLine = 2;
    }
}PIC_FORMAT_INFO;


typedef struct _REGION_ROAD_CODE_INFO
{
	int nOffsetX;//x方向偏移量
	int nOffsetY;//y方向偏移量
	char nRegionRoadCode[16];//路段编号
	char chRegionName[128];//路段名称
	UINT32 nBlackFrameWidth;//黑框宽度
	UINT32 nFontSize;//字体大小
	char chReserve[64];//预留
	//char chReserve[128];//预留
	_REGION_ROAD_CODE_INFO()
	{
		nOffsetX = 0;
		nOffsetY = 0;
		memset(nRegionRoadCode,0,16);
		memset(chRegionName,0,128);
		nBlackFrameWidth = 0;
		nFontSize = 0;
		memset(chReserve,64,0);
		//memset(chReserve,128,0);
	}
}REGION_ROAD_CODE_INFO;

//录像格式信息
typedef struct _VIDEO_FORMAT_INFO
{
    int nEncodeFormat;//编码方式0:MJPG-BOCOM;1:h264;2:MJPEG4;3:MJPG
    int nFrameRate;// 帧率0:1,1:2.5,2:5,3:7.5,4:10,5:12.5,6:15,7:17.5,8:20,9:22.5,10:25
    int nResolution;// 分辨率0:400x300;1:480x270;2:600x450;3:640x360;4:800x600;5:960x540;6:1000x750;7:1280x720;8:1200x900;9:1600x900;10:1600x1200;11:1920x1080
    int nAviHeaderEx;  //扩展AVI头
    int nSendRtsp;     //是否发送RTSP实时视频
    int nTimeLength;        //录像时长(单位为分钟)
	int nSip;     //是否启动sip服务
	int nSendH264;     //是否发送H264实时视频
    char chReserved[52];

    _VIDEO_FORMAT_INFO()
    {
       nEncodeFormat = 1;//默认H264
	   nFrameRate = 2; //默认5帧
       nResolution = 4; //默认800x600
	   nAviHeaderEx = 0;
       nSendRtsp = 0;
       nTimeLength = 2; // //默认2分钟
	   nSip = 0;
	   nSendH264 = 0;
	   memset(chReserved,0,52);
    }
}VIDEO_FORMAT_INFO;

//开关灯信息
typedef struct _LIGHT_TIME_INFO
{
    UINT32 nLightTimeControl;//是否定时开光灯，0：由算法控制开关灯,1:定时开关灯
    UINT32 nSpringLightTime;//春季开关灯时间，高16位关灯时间(0630)，低16位开灯时间(1715)
    UINT32 nSummerLightTime;//夏季开关灯时间，高16位关灯时间(0630)，低16位开灯时间(1750)
    UINT32 nAutumnLightTime;//秋季开关灯时间，高16位关灯时间(0630)，低16位开灯时间(1650)
    UINT32 nWinterLightTime;//冬季开关灯时间，高16位关灯时间(0630)，低16位开灯时间(1630)
    char chReserved[20];//保留字段

    _LIGHT_TIME_INFO()
    {
        nLightTimeControl = 0;//
        nSpringLightTime = 0x027606b3;
        nSummerLightTime = 0x027606d6;
        nAutumnLightTime = 0x02760672;
        nWinterLightTime = 0x0276065e;
        memset(chReserved,0,20);
    }
}LIGHT_TIME_INFO;

//帧信息记录结构
typedef struct _FRAME_INFO
{
	UINT32 uTime;						//时间(秒)
	UINT32 uMiTime;					//时间(毫秒)
	char chReserved[64];				//扩展

	_FRAME_INFO()
	{
		uTime = 0;
		uMiTime = 0;
		memset(chReserved,0,64);
	}
}FRAME_INFO;

//客服端受限参数
typedef struct _RC
{
    float rate;      //受限权重
    int    count;    //记录次数
    _RC()
    {
        rate = 0.0;
        count = 0;
    }
}rc;


//客户端连接结构
typedef struct _SRIP_CLEINT
{
	char chHost[SKP_MAX_HOST];				//IP
	int nPort;								//端口
	int nSocket;							//socket
	int nLinker;							//心跳计数
	bool bVerify;							//通过验证
	char chUserName[MIMAX_USERNAME];		//用户名
	UINT32 uTimestamp;				//时间戳
	bool bConnect;                          //是否发送图象
    double nTime;                             //统计发送时间
	long nCount;			                   //统计发送次数

    //帧率控制
	bool nlimit;                             //是否受限
	float rate;                              //受限参数
	int   nClientKind;                        //客户端类型(0:中心端,1:客户端)
	//udp传送
	int   ClientPort;                          //客户端port
	_SRIP_CLEINT()
	{
		memset(chHost,0,sizeof(chHost));
		nPort = 0;
		nSocket = 0;
		nLinker = 0;
		bVerify = false;
		memset(chUserName,0,sizeof(chUserName));
		uTimestamp = 0;
		bConnect = 0;
		nTime    =0;
		nCount   =0;
		nlimit    =false;
		rate      =0.0;
		nClientKind =0;
		ClientPort =0;

	}
}SRIP_CLEINT;
//客户端列表
typedef std::map<int,SRIP_CLEINT> ClientMap;

//中心网管结构
typedef struct _NeClient
{
    char strHost[SKP_MAX_HOST];  //IP
    int  nPort;                 //端口
    int  nSocket;               //socket描述符
    _NeClient()
    {
        memset(strHost,0,SKP_MAX_HOST);
          nPort = 0;
          nSocket = 0;
    }
}NeClient;
typedef std::map<int, NeClient> NeMap;

//登录结构
typedef struct _LOGIN_INFO
{
	char chUserName[MIMAX_USERNAME];		//用户名
	char chPass[MIMAX_USERPASS];			//密码
	bool bCheckPriv;						//验证
	bool Priv;								//权限
	int ClientPort;                         //客户端udp端口

	_LOGIN_INFO()
	{
		memset(chUserName,0,MIMAX_USERNAME);
		memset(chPass,0,MIMAX_USERPASS);
		bCheckPriv = false;
		Priv = 0;
		ClientPort = 0;
	}
}LOGIN_INFO;

// FTP配置结构
typedef struct _Set_Ftp_Path
{
	int nUseFlag;								// 0：不可定制;1：可定制
	int nServerType;							// 中心端类型
	char strVideoPath[SRIP_LOCATION_MAXLEN];	//录像视频FTP路径
	char strWfPicPath[SRIP_LOCATION_MAXLEN];	//违法图片FTP路径
	char strWfTxtPath[SRIP_LOCATION_MAXLEN];	//违法文本FTP路径
	char strKkPicPath[SRIP_LOCATION_MAXLEN];	//卡口图片FTP路径
	char strFsTxtPath[SRIP_LOCATION_MAXLEN];	//流量统计FTP路径
	char chReserved[256*4];
	
	_Set_Ftp_Path()
	{
		nUseFlag = 0;
		nServerType = 999;
		memset(strVideoPath,0,SRIP_LOCATION_MAXLEN);
		memset(strWfPicPath,0,SRIP_LOCATION_MAXLEN);
		memset(strWfTxtPath,0,SRIP_LOCATION_MAXLEN);
		memset(strKkPicPath,0,SRIP_LOCATION_MAXLEN);
		memset(strFsTxtPath,0,SRIP_LOCATION_MAXLEN);
		memset(chReserved,0,256*4);
	}
}SET_FTP_PATH;

typedef struct _FASTINGIUM_TIME 
{ 
	UINT32 nMornFastigiumBegin; 
	UINT32 nMornFastigiumEnd; 
	UINT32 nNightFastigiumBegin; 
	UINT32 nNightFastigiumEnd; 
	UINT32 nOrdinaryMornBegin; 
	UINT32 nOrdinaryMornEnd; 
	UINT32 nOrdinaryNightBegin; 
	UINT32 nOrdinaryNightEnd; 
	char szReserved[32];//保留字段 

	_FASTINGIUM_TIME() 
	{ 
		nMornFastigiumBegin = 0; 
		nMornFastigiumEnd = 0; 
		nNightFastigiumBegin = 0; 
		nNightFastigiumEnd = 0; 
		nOrdinaryMornBegin = 0; 
		nOrdinaryMornEnd = 0; 
		nOrdinaryNightBegin = 0; 
		nOrdinaryNightEnd = 0; 
		memset(szReserved,0,32); 
	} 
}FASTINGIUM_TIME;

//3G配置结构
typedef struct _SET_3G_INFO
{
	char chUserName[MIMAX_USERNAME];		//用户名
	char chPass[MIMAX_USERPASS];			//密码
	char chApn[MIMAX_USERNAME];				//apn
	
	_SET_3G_INFO()
	{
		memset(chUserName,0,MIMAX_USERNAME);
		memset(chPass,0,MIMAX_USERPASS);
		memset(chApn,0,MIMAX_USERNAME);
	}
}SET_3G_INFO;

typedef struct _ServerKafka
{
	UINT32 uSwitchUploading; //是否需要上传
	char chItem[64];         //工业控机编号8个数字
	char chBrand[32];        //厂家
	char chVersion[32];      //版本
	char chTopic[32];        //Topic
	UINT32 uUpdateType;      //传方式 0:同时传卡口记录和图片 1:只传卡口记录，不传卡口图片
	UINT32 uCheckModal;		 //1:卡口  2:电警
	char chReserved[120];	 //扩展
	_ServerKafka()
	{
		uSwitchUploading = 0;
		memset(chItem,0,64);
		memset(chBrand,0,32);
		memset(chVersion,0,32);
		memset(chTopic,0,32);
		uUpdateType = 0;
		uCheckModal = 1;
		memset(chReserved,0,120);				//扩展
	}
}ServerKafka;

//3G配置结构扩展
typedef struct _SET_3G_INFO_EX
{
	int nExist3G ;		//是否存在3G
	char chIp[SKP_MAX_HOST];		// ip地址
	int n3GType;//0:内置，1：外置
	char chReserved[108];			

	_SET_3G_INFO_EX()
	{
		nExist3G = 0;
		memset(chIp,0,SKP_MAX_HOST);
		n3GType = 0;
		memset(chReserved,0,108);
	}
}SET_3G_INFO_EX;

// DSP码流转发
typedef struct _SET_DSP_CAMERA
{
	char chDspIp[SKP_MAX_HOST];
	int nDspPort;
	int nTcpPort;
	char chReserved[256];

	_SET_DSP_CAMERA()
	{
		memset(chDspIp,0,SKP_MAX_HOST);
		nDspPort = 0;
		nTcpPort = 0;
		memset(chReserved,0,256);
	}
}SET_DSP_CAMERA;

//系统信息结构
typedef struct _SRIP_SYSTEM_INFO
{
	float fCpu;					//CPU利用率
	float fMemory;				//内存利用率
	float fDisk;	             //磁盘使用率
	float fCpuT;                 //cpu温度
	float fSysT;                 //系统温度
	UINT32 uSysTime;             //系统时间
	_SRIP_SYSTEM_INFO()
	{
		fCpu = 0;
		fMemory = 0;
		fDisk = 0;
		fCpuT=0.0;
		fSysT=0.0;
		uSysTime=0;
	}
}SRIP_SYSTEM_INFO;

//系统硬件配置信息
typedef struct _SYSTEM_INFO_EX
{
    float fTotalMemory;//内存容量（单位：G）
    char szCpu[64];
    float fTotalDisk;//磁盘容量
    char szDetectorType[32];//检测器型号
    char szBoostType[8];//boost版本号:（如沪1.0）
	char szLabelType[24];//车标库版本号:（如沪1.0）
    _SYSTEM_INFO_EX()
    {
        fTotalMemory = 0;
        fTotalDisk = 0;
        memset(szCpu,0,64);
        memset(szDetectorType,0,32);
        memset(szBoostType,0,8);
		memset(szLabelType,0,24);
    }

}SYSTEM_INFO_EX;
//通道状态
enum CHANNEL_STATUS
{
    PAUSE_STATUS = 0,  //暂停状态
    NORMAL_STATUS, //正常状态
    UNNORMAL_STATUS //异常状态
};

//区域图像类型
enum REGION_IMAGE
{
    WHOLE_IMAGE = 1,   //全景图像
    FOCUS_REGION_IMAGE,       //对焦区域图像
    VIOLATION_REGION_IMAGE,       //违法检测区域图像
    EVENT_REGION_IMAGE,       //事件检测区域图像
    TRAFFIC_SIGNAL_REGION_IMAGE      //红灯检测区域图像
};

//检测数据结构头
typedef struct _SRIP_DETECT_HEADER
{
	UINT32 uChannelID;		//通道ID
	UINT32 uDetectType;		//0 图片 1 检测图片 2检测结果
	UINT32 uTimestamp;		//时间戳
	UINT32 uSeq;				//帧序号
	int64_t		 uTime64;			//时间戳
	UINT32 uWidth;			//宽度
    UINT32 uHeight;			//高度
	UINT32 uRealTime;			//(客户端:线圈状态；中心端：实时信息)
	UINT32 uVideoId;        //录像编号(0:jpg,1:h264)
	double dFrameRate;   //帧率
	UINT32 uImageRegionType;   //图像区域类型
	UINT16 uTrafficSignal;//控制灯状态
	_SRIP_DETECT_HEADER()
	{
		uChannelID = 0;
		uDetectType = 0;
		uTimestamp = 0;
		uTime64 = 0;
		uSeq = 0;
		uWidth = 0;
		uHeight = 0;
		uRealTime =0;
		uVideoId = 0;
		dFrameRate = 0;
		uImageRegionType = WHOLE_IMAGE;
		uTrafficSignal = 0;
	}

	_SRIP_DETECT_HEADER & operator=(const _SRIP_DETECT_HEADER& header)
	{
		uChannelID = header.uChannelID;
		uDetectType = header.uDetectType;
		uTimestamp = header.uTimestamp;
		uTime64 = header.uTime64;
		uSeq = header.uSeq;
		uWidth = header.uWidth;
		uHeight = header.uHeight;
		uRealTime =header.uRealTime;
		uVideoId = header.uVideoId;
		dFrameRate = header.dFrameRate;
		uImageRegionType = header.uImageRegionType;
		uTrafficSignal = header.uTrafficSignal;

		return *this;
	}
}SRIP_DETECT_HEADER;

//视频源格式
enum VEDIO_FORMAT
{
	VEDIO_YUV = 0,		//YUV
	VEDIO_H264 = 1,		//H264
	VEDIO_YUV_FILE = 2,  //YUV_FILE
	VEDIO_MJPEG = 3,   //Motion Jpeg
	VEDIO_PAL = 4,		//PAL
	VEDIO_AVI = 5,		//AVI
	VEDIO_PICLIB = 6,		//图库
	VEDIO_H264_FILE = 7, //H264文件流(从老到新)
	VEDIO_H264_FILE_N = 8, //H264文件流(从新到老)
	VIDEO_BAYER = 9 //BAYER
};

//相机类型
#ifndef CAMERAKINDS
#define CAMERAKINDS
enum CAMERA_TYPE
{
    JAI_CAMERA_LINK_FIELD = 1,//1920-i
    JAI_CAMERA_LINK_FRAME,
    JAI_CAMERA_LINK_FIELD_P,//1920-p
    JAI_CL_500,
    PTG_ZEBRA_200,
    PTG_ZEBRA_500,
    ANALOG_FRAME,     //模拟帧相机
    ANALOG_FIELD,     //模拟场相机
    ANALOG_FRAME_DH,     //模拟帧相机
    ANALOG_FIELD_DH,     //模拟场相机
    MONITOR_CAMERA, //监控相机
    HD_IP_CAMERA,   //discovery相机
    ROSEEK_CAMERA, //锐视相机
    SANYO_CAMERA, //三洋相机
	PTG_GIGE_200,	//ptg gige200万相机
	PTG_GIGE_500,	//ptg gige500万相机
	DSP_ROSEEK_200_310, //锐视200万dsp相机
	DSP_ROSEEK_500_335,//锐视500万dsp相机 18
	BOCOM_301_200,//bocom200万相机(301)
	BOCOM_302_500,//bocom500万相机(302)
	BOCOM_301_500,//bocom500万相机(301)
	BOCOM_302_200,//bocom200万触发相机(302)
	DSP_ROSEEK_200_380,//dsp200万触发
	DSP_ROSEEK_500_330,//dsp500万触发 24
	H3C_CAMERA,//华三相机
	VIS_CAMERA,//VIS相机
	BASLER_200,//basler相机
	BASLER_500,//basler相机
	HK_CAMERA,//海康视频源
	CF_CAMERA,//长峰视频源
	AXIS_CAMERA,//安讯士相机
	HISILICON_CAMERA,//海思相机
	CF_CAMERA_2,//长峰2期视频源
	DSP_ROSEEK_400_340,//锐视400万dsp相机
	DSP_SERVER, //MVS做为DSP相机Server 35
	DH_CAMERA,//大华相机
	DSP_500_C501K,//2592*1936分辨率相机
	DSP_200_C201K,//1920*1080分辨率相机
	NETPOSA_CAMERA,  // 东方网力
	DH_203_M, //大华200W 1600*1200, 40
	DH_523_M, //大华500W 2592*2048,
	DH_213_M,	//大华200W 1920*1080,
	DSP_ROSEEK_200_385,//1616*1232
	DSP_200_C203K,//1600*1200
	ZION_CAMERA, //凯利信相机
	DSP_ROSEEK_600_465,//锐视600万dsp相机,2752*2208
	DSP_ROSEEK,//锐视相机
	DSP_YTZ,//英泰智相机
	DSP_DH,//大华相机
	DSP_GY,//港宇相机
    OTHER_CAMERA   //其他相机
};
#endif

//通道类型
enum CHANNEL_TYPE
{
	CHANNEL_FIXUP = 0, //固定
	CHANNEL_YT		   //云台
};

//录像类型
enum CAPTURE_TYPE
{
	CAPTURE_NO = 0,		//无录像
	CAPTURE_FULL,		//全天录像
	CAPTURE_TIME		//定时录像
};

//通道检测类型
enum CHANNEL_DETECT_KIND
{
    DETECT_NONE = 0,                    //不检测
	DETECT_CARNUM = 1,              //车牌检测
	DETECT_OBJECT=2,                  //目标检测
	DETECT_VTS=4,                         //闯红灯检测
	DETECT_CARCOLOR=8,	                //车身颜色检测
	DETECT_TRUCK=16,                   //车型检测
	DETECT_TEXTURE = 32,                //车辆特征检测(车标)
	DETECT_FLUX=64,	                     //事件检测
	DETECT_EVENT_CARNUM = 128,            //事件车牌检测
	DETECT_LOOP = 256,                   //线圈测速
	DETECT_BUS_VIO_DET = 512,   //公交道违章检测
	DETECT_RADAR = 1024,   //雷达检测
	DETECT_VIOLATION = 2048, //违章检测
    DETECT_BEHAVIOR  = 4096,   //行为分析
    DETECT_CHARACTER = 8192, //特征提取
	DETECT_FACE = 16384 //人脸识别
};

//检测时间
enum CHANNEL_DETECT_TIME
{
	DETECT_DAY = 0,    //白天
	DETECT_DUSK,       //傍晚
	DETECT_NIGHT,	   //晚上
	DETECT_AUTO     //自动
};

//车身颜色
#ifndef COLORTYPE
#define COLORTYPE
enum MVCOLOR
{
	WHITE,      //白色 0 by default
	SILVER,     //银色 1
	BLACK,      //黑色 2
	RED,        //红色 3
	PURPLE,     //紫色 4
	BLUE,       //蓝色 5
	YELLOW,     //黄色 6
	GREEN,      //绿色 7
	BROWN,      //褐色 8
	PINK,       //粉红色 9
	GRAY,       //灰色 10
	UNKNOWN     //未知 11
};
#endif

//车牌颜色
#ifndef CARCARD_COLOR
#define CARCARD_COLOR
enum CARNUM_COLOR
{
	CARNUM_BLUE=1,  //蓝色 1
	CARNUM_BLACK,   //黑色 2
	CARNUM_YELLOW,  //黄色 3
	CARNUM_WHITE,   //白色 4
	CARNUM_OTHER    //其他 5
};
#endif

//车牌结构(单双行)
#ifdef WIN32
enum CARNUM_ROW
{
   LISTROW=1,
   DOUBLEROW,
   OTHERROW
};
#endif

//车辆类型
enum CAR_TYPE
{
	SMALL_CAR=1,    //小 1
	MIDDLE_CAR,     //中 2
	BIG_CAR,        //大 3
	OTHER_TYPE,      //其他(非机动车)4
	PERSON_TYPE,      //行人5
	BUS_TYPE,       //大型客车 6
	TRUCK_TYPE,     //大型货车 7
	MIDDLEBUS_TYPE, //中型客车 8
	NO_JUDGE,       //未判断 9
	WRONG_POS,      //未知太偏 10
	MINI_TRUCK,      //小型货车 11
	TAXI,           //小型客车 12
	TWO_WHEEL_TYPE,//两轮车 13
};

//车辆类型子类
enum CAR_DETAIL_TYPE
{
	DETAIL_NO_JUDGE = 0,
	DETAIL_BIG_BUS=1,    //大巴
	DETAIL_JZX,     //集装箱
	DETAIL_YGC,        //油罐车
	DETAIL_KC,      //卡车
	DETAIL_DC,      //吊车
	DETAIL_TC,       //拖车
	DETAIL_SNC,     //水泥车
	DETAIL_MBC, //面包车
	DETAIL_YWK,       //依维柯
	DETAIL_NO_CAR,      //没有发现任何汽车
};

//车道类型
enum ROAD_TYPE
{
    VEHICLE_ROAD = 1,                           //机动车道
    PERSON_ROAD,                                  //非机动车道
    VEHICLE_PERSON_ROAD,                 //机非混合车道
	OBJECT_ROAD							//行为分析车道
};

//车道方向
enum ROAD_DIRECTION
{
    EAST_TO_WEST = 1,   //从东到西 default 1
    WEST_TO_EAST,       //从西到东
    SOUTH_TO_NORTH,     //从南到北
    NORTH_TO_SOUTH,     //从北到南
    SOUTHEAST_TO_NORTHWEST, //由东南到西北
    NORTHWEST_TO_SOUTHEAST, //由西北到东南
    NORTHEAST_TO_SOUTHWEST, //由东北到西南
    SOUTHWEST_TO_NORTHEAST //由西南到东北
};

//报警类型
enum  ALARM_CODE
{
    ALARM_CODE_UNKNOWN				        = 0,		//其它
	ALARM_CODE_DETECTOR_START		        = 1,		//检测器启动
	ALARM_CODE_DETECTOR_REBOOT	            = 2,		//检测器重启
	ALARM_CODE_DETECTOR_HALT		        = 3,		//检测器停止
	ALARM_CODE_CPU_OVER_HEAT		        = 4,		//CPU温度过高
	ALARM_CODE_LOW_FAN_SPEED		        = 5,		//CPU风扇转速过低
	ALARM_CODE_CPU_OVER_LOAD		        = 6,		//CPU负载过高
	ALARM_CODE_DETECTOR_OVER_HEAT	        = 7,		//系统温度过高

	ALARM_CODE_NO_VIDEO				        = 501,		//无视频
	ALARM_CODE_CAMERA_MOVED			        = 502,		//摄像机位置移动
	ALARM_CODE_BOX_ALERT			        = 503,		//机箱报警
	ALARM_CODE_BOX_OPEN_DOOR		        = 504,		//机箱开门报警
	ALARM_CODE_BOX_LINE_CUT		            = 505,		//机箱断线报警
	ALARM_CODE_BOX_VIBRATE			        = 506,		//机箱振动报警
	ALARM_CODE_DDISK_FULL			        = 507,		//检测器磁盘满报警
	ALARM_CAMERA_BREAK                      = 508,       //相机断开 dgh
	ALARM_RADAR_BREAK                       = 509,      //雷达断开 dgh
	ALARM_CAMERA_LINK                       = 510,      //相机连接
	ALARM_VIDEO_RESUME                       = 511,      //相机视频恢复

	ALARM_CODE_ALL                          = 1000      //所有日志
};

//通道视频属性
typedef struct _SRIP_CHANNEL_ATTR
{
	UINT32 uId;			//通道ID
	UINT32 uBrightness;	//亮度
	UINT32 uContrast;	//对比度
	UINT32 uSaturation;	//饱和度
	UINT32 uHue;			//色度

	_SRIP_CHANNEL_ATTR()
	{
		uId = 0;
		uBrightness = 0;
		uContrast = 0;
		uSaturation = 0;
		uHue = 0;
	}
}SRIP_CHANNEL_ATTR;


//通道数据结构，方便两端使用
typedef struct _SRIP_CHANNEL
{
	UINT32 uId;				//通道ID
	CHANNEL_TYPE eType;				//通道类型
	VEDIO_FORMAT eVideoFmt;			//视频源格式
	CAPTURE_TYPE eCapType;			//录像类型
	UINT32 uCapBeginTime;		//录像开始时间
	UINT32 uCapEndTime;		//录像结束时间
	UINT32 uEventCaptureTime;	//事件录像时间
	UINT32 uVideoBeginTime;	//历史视频开始时间
	UINT32 uVideoEndTime;	//历史视频结束时间

	int bEventCapture;            //是否事件录象(0:无任何录像;1只有事件录像,2:只有违章录像;3:两种录像都存在)
	int nYUVFormat;              //YUV组合方式(0:VYUY;1:UYVY;2:YVYU;3:YUYV)
	int bRmShade;				//是否去阴影
	int bRmTingle;				//是否去抖动
	int bSensitive;			    //是否灵敏度

	int bRun;			    //运行状态
	int bLocal;			    //是否本机通道

	CHANNEL_DETECT_KIND uDetectKind;  //检测类型
	CHANNEL_DETECT_TIME uDetectTime; //白天还是晚上检测

	char chMonitorHost[SKP_MAX_HOST];  //监视器主机
	UINT32 uMonitorPort;			//相机端口号

	char chHost[SKP_MAX_HOST];  //主机
	UINT32 uPort;			//端口

	char  chFileName[32];	//文件名称 (64个字节)

	UINT32 uDirection;    //检测方向描述
	char chPlace[64];   //通道地点

	int nShowTime;   //停留显示时间
	int nCameraType;  //相机型号
	int nCameraId;    //相机编号
	int nPannelID;	 //断面编号
	int nCameraStatus;  //相机状态

	char chSynHost[SKP_MAX_HOST]; //相邻同步主机
	UINT32 uSynPort; //相邻同步主机端口

    int nChanWayType;//车道类型
    int nVideoIndex;//视频采集卡编号
    int nPreSet;//相机预置位
    int nMonitorId;//监视器编号
    int nWorkMode;//通道工作方式（0：连续，1：触发）

	char chCameraHost[SKP_MAX_HOST]; //相机IP
	int reserved[2];//检测保留参数 
	char chExPlace[48]; //通道地点扩充 
	char chCode[48];//注册码
	char chDeviceID[32]; //设备编号


	_SRIP_CHANNEL()
	{
		uId = 0;
		eType = CHANNEL_FIXUP;
		eVideoFmt = VEDIO_YUV_FILE;
		eCapType = CAPTURE_NO;
		uCapBeginTime = 7200;
		uCapEndTime = 7200;
		uEventCaptureTime = 5;
		uVideoBeginTime  =   0;
		uVideoEndTime = 0;
		uDetectKind = DETECT_NONE;
		uDetectTime = DETECT_AUTO;
		bEventCapture = 0;
		nYUVFormat = 0;
		bRmShade = 0;
		bRmTingle = 0;
		bSensitive = 0;
		bRun = 1;
		bLocal = 1;
		nShowTime  = 5;
		nCameraType = 1;
		nCameraId = 1;
		nPannelID = 1;
		nCameraStatus =  0;
		memset(chMonitorHost, 0, SKP_MAX_HOST);
		memset(chHost,0,SKP_MAX_HOST);
		memset(chPlace,0,64);
		//memset(chDirection,0,MAX_DIRECTION);
		uDirection = 0;
		memset(chFileName,0,32);
		memset(chSynHost, 0, SKP_MAX_HOST);
		//memset(chUserName, 0, SKP_MAX_HOST);
		//memset(chPassWord, 0, SKP_MAX_HOST);

        nChanWayType = 0;
        nVideoIndex =0;
        nPreSet = 0;
        nMonitorId = 0;
        uMonitorPort = 0;
        nWorkMode = 0;
        memset(chCameraHost,0, SKP_MAX_HOST);
		memset(reserved,0,2*sizeof(int));
		memset(chExPlace,0,48);
		memset(chCode,0,48);
		memset(chDeviceID,0,32);
	}

}SRIP_CHANNEL;

typedef std::list<SRIP_CHANNEL> CHANNEL_INFO_LIST;


//车道参数扩展配置
typedef struct _SRIP_CHANNEL_EXT
{
    UINT32 uTrafficStatTime;	//流量统计周期
	UINT32 uEventDetectDelay;	//事件检测间隔

	int nShowTime;   //停留显示时间

	int bRmShade;				//是否去阴影
	int bRmTingle;				//是否去抖动
	int bSensitive;			    //是否灵敏度

    int nHolizonMoveWeight; //一次性往左右移动步数
    int nVerticalMoveWeight; //一次性往上下移动步数
	int nZoomScale; //2倍一次性镜头拉伸次数
    int nZoomScale2; //4倍
    int nZoomScale3; //8倍

    int uId;//通道编号
    int reserved[19];//保留参数

	_SRIP_CHANNEL_EXT()
	{
	    uTrafficStatTime  =   0;
		uEventDetectDelay = 0;
		nShowTime = 0;

        bRmShade = 0;
		bRmTingle = 0;
		bSensitive = 0;

	    nHolizonMoveWeight = 1;
        nVerticalMoveWeight = 1;
        nZoomScale = 1;
        nZoomScale2 = 1;
        nZoomScale3 = 1;

        uId = 0;
        memset(reserved, 0, 19*sizeof(int));
	}
}SRIP_CHANNEL_EXT;


//检测结果类型
enum DETECT_RESULT_TYPE
{
	/* 0 为全部 */
	DETECT_RESULT_ALL		= 0,
//事件
	DETECT_RESULT_EVENT_STOP = 1,	//1 车辆停驶
	DETECT_RESULT_EVENT_GO_AGAINST,	//2 车辆逆行
	DETECT_RESULT_EVENT_DERELICT,	//3 遗弃物
	DETECT_RESULT_EVENT_PASSERBY,	//4 行人横穿
	DETECT_RESULT_EVENT_GO_SLOW,	//5 车辆慢行
	DETECT_RESULT_EVENT_GO_FAST,	//6 车辆超速
	DETECT_RESULT_EVENT_JAM,		//7 交通拥堵
	DETECT_RESULT_EVENT_GO_CHANGE,	//8 违章变道

	DETECT_RESULT_EVENT_PERSON_STOP,	//9 行人停留
	DETECT_RESULT_EVENT_WRONG_CHAN,		//10 非机动车出现(博康中心端表示机动车出现)
	DETECT_RESULT_EVENT_PERSON_AGAINST, //11 行人逆行
	DETECT_RESULT_EVENT_CROSS,		    //12 车辆横穿
	DETECT_RESULT_EVENT_PERSON_APPEAR,  //13 行人出现
	DETECT_RESULT_EVENT_APPEAR,		    //14 机动车出现(博康中心端表示非机动车出现)
	DETECT_RESULT_EVENT_CARNUM,         //15 事件车牌

	DETECT_RESULT_RED_LIGHT_VIOLATION,  //16 闯红灯
	DETECT_RESULT_PARKING_VIOLATION,    //17 违章停车11

	DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD,  //18 小车出现在禁行车道12
	DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD,    //19 大车出现在禁行车道13
	DETECT_RESULT_BIG_IN_FORBIDDEN_TIME,    //20 大车出现在禁行时间(大货禁行)

	DETECT_RESULT_EVENT_TRAFIC,               //21 交通事故15
	DETECT_RESULT_EVNET_GO_EMERGENCY_CHANNEL, //22 走应急车道16

	DETECT_RESULT_FORBID_LEFT,          //23 禁止左拐
	DETECT_RESULT_FORBID_RIGHT,         //24 禁止右拐
	DETECT_RESULT_FORBID_STRAIGHT,      //25 禁止前行
	DETECT_RESULT_RETROGRADE_MOTION,    //26 逆行1a
	DETECT_RESULT_PRESS_LINE,           //27 压黄线1b
	DETECT_RESULT_NO_PASSING,           //28 禁行所有车辆
	DETECT_RESULT_ELE_EVT_BIANDAO,      //29 变道

	DETECT_RESULT_EVENT_INSIDE,         //30 闯入
	DETECT_RESULT_EVENT_OUTSIDE,        //31 越界

	DETECT_RESULT_OBV_TAKE_UP_BUSWAY,   //32占用公交道20

	DETECT_RESULT_BLACK_PLATE,          //33 黑名单
	DETECT_RESULT_WHITE_PLATE,          //34 白名单

	DETECT_RESULT_CROWD,                //35 人群聚集
	DETECT_RESULT_PERSON_RUN,           //36 行人奔跑

	DETECT_RESULT_CYC,                  //37 柴油车出现
	DETECT_RESULT_NOCARNUM,             //38无牌车出现

	DETECT_RESULT_EVENT_HOLD_BANNERS,   //39 拉横幅
	DETECT_RESULT_EVENT_FIGHT,          //40 打架
	DETECT_RESULT_EVENT_DISTRIBUTE_LEAFLET,   //41 散传单
	DETECT_RESULT_EVENT_SMOKE,          //42 只有烟
	DETECT_RESULT_EVENT_FIRE,           //43 只有火
	DETECT_RESULT_EVENT_SMOKE_FIRE,     //44 既有烟也有火
	DETECT_RESULT_NOT_LOCAL_CAR,        //45 非本地车
	DETECT_RESULT_FACE,                 //46 人脸识别
	DETECT_RESULT_EVENT_OBJECT,         //47 行李物品目标出现
	DETECT_RESULT_EVENT_SHIELD,         //48 遮阳板出现
	DETECT_RESULT_EVENT_BEYONDSIDE,     //49 闯出
	DETECT_RESULT_YELLOW_CAR,           //50 黄标车
	DETECT_RESULT_YELLOW_CRC,           //51 黄标柴油车(国1标准汽油车禁行)
	DETECT_RESULT_PRESS_WHITELINE,      //52 压白线
	DETECT_RESULT_PRESS_LEAD_STREAM_LINE,//53压导流线
	DETECT_RESULT_EVENT_DISTANCE_FAST,	//54区间超速

	DETECT_RESULT_TAKE_UP_NONMOTORWAY,        //55 机占非
	DETECT_RESULT_NO_PUTIN,                   //56 禁止驶入
	DETECT_RESULT_NO_STOP,                    //57 禁停
	DETECT_RESULT_NO_RESORT,                  //58 路口滞留
	DETECT_RESULT_NO_TURNAROUND,              //59 禁止调头

	DETECT_RESULT_EVENT_REMOVEDITEM,		//60  物品移动
	DETECT_RESULT_EVENT_SEEPER,				//61 积水
	DETECT_RESULT_EVENT_LOITREING,			//62 徘徊
	DETECT_RESULT_EVENT_TAILGATING,			//63 尾随
	DETECT_RESULT_EVENT_IMPROPRIATE,		//64 违章占道
	DETECT_RESULT_NO_PARKING,				//65 黄网格停车
	DETECT_REAULT_BREAK_GATE,				//66 冲卡
	
	DETECT_MASK_PLATE,						//67 车牌遮挡
	DETECT_MATCH_PLATE,						//68 前后牌匹配
	DETECT_NOT_CUTRTESY_DRIVE,				//69 车辆未礼让行人
	DETECT_RESULT_RETROGRADE_MOTION_FLAG,	//70违反禁令标志 
	DETECT_NO_DIRECTION_TRAVEL,				//71不按导向车道行驶
	DETECT_RESULT_CROSSLINE_STOP,				//72越线停车
	DETECT_RESULT_PLATE_LIMIT,				//73尾号限行
	DETECT_RESULT_GASSER,				//74加塞
	DETECT_RESULT_OBV_TAKE_UP_BUSWAY_1,	//75占用公交道，分类1
	DETECT_RESULT_OBV_TAKE_UP_BUSWAY_2,	//76占用公交道，分类2
	DETECT_RESULT_OBV_TAKE_UP_BUSWAY_3,	//77占用公交道，分类3
	DETECT_RESULT_OBV_TAKE_UP_BUSWAY_4,	//78占用公交道，分类4

//统计
	DETECT_RESULT_STAT_FLUX = 100,	//车道流量100
	DETECT_RESULT_STAT_SPEED_AVG,	//平均车速
	DETECT_RESULT_STAT_ZYL,			//平均车道占有率
	DETECT_RESULT_STAT_QUEUE,		//队列长度
	DETECT_RESULT_STAT_CTJJ,		//平均车头间距
	DETECT_RESULT_STAT_CAR_TYPE,		//车辆分型
	DETECT_RESULT_STAT_SLOW_QUEUE_LEN  //车速较慢情况下的队列长度   //shan

};

typedef struct _OBJECT_COLOR_REF
{
	int  nColor1;       //颜色1
	int  nColor2;	    //颜色2
	int  nColor3;       //颜色3
	int  nWeight1;    //颜色权重1
	int  nWeight2;    //颜色权重2
	int  nWeight3;    //颜色权重3
	_OBJECT_COLOR_REF()
	{
	    nColor1 = 0;
	    nColor2 = 0;
	    nColor3 = 0;

	    nWeight1 = 0;
	    nWeight2 = 0;
	    nWeight3 = 0;
	}

}OBJECT_COLOR_REF;

#ifndef  WIN32
//车牌识别结果结构体
typedef struct _CARNUM_CONTEXT
{
	char    carnum[7];        //车牌号码
	CvRect  position;         //车牌位置,相对于原图
	int     color;            //车牌颜色
	int     vehicle_type;     //车辆类型
	float	iscarnum;         //从匹配结果看是否是车牌
	double  mean;             //车牌亮度
	double  stddev;			  //车牌方差
	char    wjcarnum[2];      //武警牌中间的两个小字
	int     carnumrow;        //单排或者双排车牌
	CvRect  smearrect[20];    //光柱所在位置
	int     smearnum;		  //光柱的数目
	int      VerticalTheta;          //垂直倾斜角
	int      HorizontalTheta;          //水平倾斜角

 	_CARNUM_CONTEXT( )
 	{
 		memset(carnum,0,7);
 		position = cvRect(0,0,0,0);
 		color = -1;
		vehicle_type = -1;
		iscarnum = -1.0f;         //从匹配结果看是否是车牌
		mean = -1.0;             //车牌亮度
	    stddev = -1.0f;			  //车牌方差
		memset(wjcarnum,0,2);
 		carnumrow = 1;
 		smearnum = 0;
 		VerticalTheta=0;
 		HorizontalTheta=0;
 	}
}CARNUM_CONTEXT_DEF;

//检测结果结构
typedef struct _SRIP_DETECT_OUT_RESULT
{
	int nChannelIndex;          //车道序号
	int nCarId;                 //车编号
	DETECT_RESULT_TYPE eRtype;		//结果类型
	UINT32 x;					//事件坐标 X
	UINT32 y;					//事件坐标 Y
	CvRect  carRect;          //车所在的区域
	double value;				//速度或除流量以外的其他统计值
	OBJECT_COLOR_REF  color;			//颜色
	int type;					//车辆类型(0车,1人，2非机动车)
	double direction;			//行驶方向
	bool bShow;                 //只是停留显示，不是真的检测事件
    long nEventId;              //事件车牌关联ID
    float  fLastImgTime;        //往前取上张图片的时间(单位秒)
    bool bHaveCarnum;       //是否存在车牌

    CARNUM_CONTEXT_DEF  carNumStruct;  //车牌信息结构体
    float  fScale;              //球机应该放大的倍数

    UINT32 uFluxAll;        //所有目标流量
    UINT32 uFluxVehicle;    //机动车流量
    UINT32 uFluxPerson;     //行人流量
    UINT32 uFluxNoneVehicle;  //非机动车流量

    double dEventTime;			//事件检测时间
	bool bSavePic; //是否存图

	UINT32 uFluxBig;    //大车流量
	UINT32 uFluxSmall;     //小车流量

	float fBigSpeed; //大车平均速度
	float fSmallSpeed; //小车平均速度
	float fMaxSpeed; //最大速度

	bool bPoss; //是否占有
	UINT32 uTimeInterval;    //累计间隔时间
	UINT32 uTimePoss;     ///累计占有时间

	int nOutPutMode;//输出模式(0:正常输出，1:只存图不输出结果，2:终止输出)
	CvRect carRectOfSrcImg;          //车所在的原始图的区域 只针对停车严管事件 add by taojun


	_SRIP_DETECT_OUT_RESULT()
	{
		nChannelIndex = 0;
		nCarId = -1;
		eRtype = DETECT_RESULT_ALL;
		x = 0;
		y = 0;
		value = 0;
		color.nColor1 = 11;//未知
		color.nColor2 = 11;//未知
		color.nColor3 = 11;//未知
		type = 0;
		direction = 0;
		bShow = false;
        nEventId = -1;
        fLastImgTime = -1;
        bHaveCarnum = false;
        fScale = 1;
        uFluxAll = 0;
        uFluxVehicle = 0;
        uFluxPerson = 0;
        uFluxNoneVehicle = 0;
        dEventTime = -1;
		bSavePic = false;
		uFluxBig = 0;
		uFluxSmall = 0;
		fBigSpeed = 0;
		fSmallSpeed = 0;
		fMaxSpeed = 0;
		bPoss = 0; //是否占有
	    uTimeInterval = 0;    //累计间隔时间
		uTimePoss = 0;     ///累计占有时间
		nOutPutMode = 0;
	}
}SRIP_DETECT_OUT_RESULT;

//检测结果链表
typedef std::list<SRIP_DETECT_OUT_RESULT> DetectResultList;
#endif

//车道结构定义
#ifndef WIN32
//非WIN32 环境下，定义CPoint 结构
typedef struct _CPoint
{
	int x;
	int y;
	_CPoint()
	{
		x = 0;
		y = 0;
	}

}CPoint;
#endif

//点列表
typedef std::list<CPoint> PointList;

//辅助标定点的图像坐标
typedef struct _CPoint32f
{
	double x;
	double y;
	double z;
	_CPoint32f()
	{
		x = 0;
		y = 0;
		z = 0;
	}

}CPoint32f;
typedef std::list<CPoint32f> Point32fList;

//区域结构
typedef struct _Region
{
	PointList vList;		//区域坐标列表
	Point32fList pt32fList; //区域坐标列表的图像坐标
	Point32fList directionListPt;    //区域方向列表
	_Region()
	{
	}
}Rgn;
//区域列表
typedef std::list<Rgn> RegionList;

//摄像机标定
typedef struct _Calibration
{
	Rgn	region;			                //标定矩形
	double		length;			        //道路长度
	double		width;			        //道路宽度
	double		cameraHeight;	        //摄像机高度
	double      cameraDistance;        //摄象机距离
	double      cameraOrientation;     //摄象机角度
	PointList   ptList;                 //标定辅助点
	Point32fList pt32fList;             //标定辅助点的世界坐标
	Point32fList pt32fList2;            //标定辅助点的图像坐标

	_Calibration()
	{
		length		= 0.0;
		width		= 0.0;
		cameraHeight= 0.0;
		cameraDistance = 0.0;
		cameraOrientation = 0.0;
	}

}Calibration;

//道路区域
typedef struct _RoadStru
{
    PointList vListRoadWay;   //道路区域
	PointList vListDirection; //两个方向点

	Point32fList vListRoadWay2;//道路区域的图像坐标
	Point32fList vListDirection2;//两个方向点的图像坐标

    RegionList vListFlowBack;   //流量背景框
    RegionList vListFlow;		//线圈流量监测线

    RegionList vListViolation;      //违章检测区域
    RegionList vListEvent;          //事件检测区域
    RegionList vListTrafficSignal;  //红灯检测区域
    RegionList vListStopLine;       //停止检测线
    RegionList vListStraightLine;   //直行检测线
    RegionList vListTurnLeft;       //左转检测线
    RegionList vListTurnRight;      //右转检测线
    RegionList vListYellowLine;    //黄线
	RegionList vListWhiteLine;    //白线

    RegionList vListStabBack;       //稳像背景区域
    RegionList vListSynchLeft;      //左同步标志点
    RegionList vListSynchRight;     //右同步标志点
}RoadStru;
//定义道路链表
typedef std::list<RoadStru> RoadList;


//车道监测区域
typedef struct _VerChanStru
{
	int				nChannel;			//通道号
	int				nRoadIndex;			//车道序号
	int				nModelIndex;        //模板编号
	std::string		strChannelName;		//车道的名称

    int      nDirection;          //车道方向(与水平方向的夹角)

	RoadList::iterator road_iter; //所在道路指针

	PointList vListChannel;     //车道区域(只有一个区域)
	Point32fList vListChannel2; //车道区域图像坐标

	RegionList vListPark;		//停车区域
	RegionList vListPerson;		//行人监测区域
	RegionList vListTrash;		//丢弃物区域

	RegionList vListRef;		//参考线

    RegionList vListTurnRoad;	//变道区域
	RegionList vListYellowLine;    //黄线区域
	RegionList vListLeadStreamLine;   //导流线

	RegionList vListCarNum;     //车牌检测区域

	RegionList vListSkip;		//屏蔽区域

	RegionList vListAmount;     //流量监测线

	RegionList vListFlowFrameget; //流量取景框

	RegionList vListCardnumber;    //车牌区域

	RegionList vListLoop;   //线圈区域

	RegionList vListBargeIn;   //闯入区域

	RegionList vListBeyondMark;   //越界区域

	RegionList vListRadar;   //雷达区域
    RegionList vListLineStop;   //单车道停止线
	RegionList vListRedLight;   //红灯区域
    RegionList vListGreenLight;   //绿灯区域
    RegionList vListLeftLight;   //左转灯区域
    RegionList vListRightLight;   //右转灯区域
    RegionList vListStraightLight;   //直行灯区域
    RegionList vListTurnAroundLight;   //禁止转向灯区域
    RegionList vListLeftRedLight;   //左转红灯区域
    RegionList vListLeftGreenLight;   //左转绿灯区域
    RegionList vListRightRedLight;   //右转红灯区域
    RegionList vListRightGreenLight;   //右转绿灯区域
    RegionList vListStraightRedLight;   //直行红灯区域
    RegionList vListStraightGreenLight;   //直行绿灯区域
    RegionList vListTurnAroundRedLight;   //禁止转向红灯区域
    RegionList vListTurnAroundGreenLight;   //禁止转向绿灯区域

    RegionList vListLineStraight;   //单车道前行线
	RegionList vListYelGrid;		//黄网格区域

#ifdef WIN32
	void ClientInit()
	{
		nChannel			= 0;
		nRoadIndex			= 0;
		nModelIndex			= 0;
		strChannelName		= "";
		nDirection          = 0;
		road_iter = NULL;
	}
#endif

	_VerChanStru()
	{
		nChannel			= 0;
		nRoadIndex			= 0;
		nModelIndex         = 0;
		strChannelName		= "";
		nDirection          = 0;
	}

}VerChanStru;
//定义车道链表
typedef std::list<VerChanStru> VerChanList;

//车道参数配置
typedef struct _VerChanParaStru
{
	int	 nChannel;			    //通道号
	int	 nRoadIndex;			//车道序号
	int  nModelIndex;           //模板编号

	bool m_bNixing;				//逆行监测
	int  m_nNixingIgn;			//逆行事件忽略

	bool m_bBianDao;			//变道监测
	int  m_nBianDaoIgn;			//变道监测忽略

	bool m_bPressLine;			//压线监测
	int  m_nPressLineIgn;			//压线监测忽略

	//排队长度统计                         //nahs
	//bool  m_bStatQueueLen;     //是否统计排队长度
	//float m_fVeloStartStat4QL;       //排队长度开始统计的速度(m/s)
	//float m_fLengthStartStat4QL;   //排队长度开始统计的长度要求(m)
	//float m_fQLStatIntervalTime;      //排队长度统计间隔时间(s)


	bool m_bStop;				//停车监测
	int  m_nStopIgn;			//停车监测忽略

	bool m_bDusai;				//堵塞监测
	int  m_nDusaiIgn;			//堵塞监测忽略

	int  m_nQueueLength[ALERT_LEVEL];			//车道队列长度
	int  m_nDusaiSpeed[ALERT_LEVEL];			//平均最小速度

	bool m_bOnlyOverSped;		//单独超速报警
	int  m_nOnlyOverSpedIgn;	//单独超速忽略
	int  m_nOnlyOverSpedMax;	//单独超速限制

	bool m_bDiuQi;				//丢弃物监测
	int  m_nDiuQiIgn;			//丢弃物监测忽略
	int  m_nDiuQiIDam;			//丢弃物的直径

	bool m_bAvgSpeed;			//平均车速监测
	int  m_nAvgSpeedIgn;		//平均车速忽略
	int  m_nAvgSpeedMax;		//平均车速最大
	int  m_nAvgSpeedMin;		//平均车速最小

	bool m_bPerson;				//行人监测
	int  m_nPersonIgn;			//行人监测忽略

	int	 m_nStatFluxTime;		//车道流量检测的时间间隔(单位:秒/s)
	bool m_bStatFlux;			//车道流量
	bool m_bStatSpeedAvg;		//平均车速
	bool m_bStatZyl;			//平均车道占有率
	bool m_bStatQueue;			//队列长度
	bool m_bStatCtjj;			//平均车头间距
	bool m_bStatCarType;		//车辆分型

	bool m_bBargeIn;            //闯入检测
	bool m_bFj;					//非机动车道

	bool m_bBeyondMark;		 //越界检测
	bool m_bRight;           //是否右车道

	int  m_nAppearIgn;			//出现检测忽略

	int  m_nAlertLevel;			//堵塞报警级别

	int m_nIgnStop;            //堵塞停车报警
	int m_nIgnStopNotJam;     //不堵塞停车报警

	int m_nDusai;            //堵塞多长时间报警

	bool m_bPersonAgainst;   //行人逆行
	bool m_bCross;          //车辆横穿
	bool m_bCarAppear;		//车辆出现
	bool m_bAppear;        //目标出现
	bool m_bHalfMaxsize;    //行人出现优先

    //删除车牌角度
    int m_nForbidType;           //车道禁行车辆类型，默认不限制 0:无限制，1：禁行小车，2：禁行大车,3:禁所有车辆
    int m_nAllowBigBeginTime;    //车道允许大车行驶的最早时间点
    int m_nAllowBigEndTime;      //车道允许大车行驶的最晚时间点


    bool m_bCrowd; //是否人群聚集检测
    int m_nPersonCount; //人数阈值
    int m_nAreaPercent; //人群区域占有率阈值（百分比）

    bool m_bPersonRun;//是否奔跑检测
    float  m_fMaxRunSpeed;//奔跑速度域值（m/s）

    int    nChannelMod;		   //车道模型:0-机动车道,1-非机车道,2-机非混合车道

    std::string		strModelName;		//模板名称

	_VerChanParaStru()
	{
		nRoadIndex			= 0;
		nChannel			= 0;
		nModelIndex			= 0;

		m_bNixing			= 0;
		m_nNixingIgn		= 10;

		m_bBianDao			= 0;
		m_nBianDaoIgn		= 10;

		m_bPressLine	= 0;			//压线监测
		m_nPressLineIgn = 10;			//压线监测忽略


		//排队长度统计                  //nash
		//m_bStatQueueLen  = false;    //是否统计排队长度
		//m_fVeloStartStat4QL = 3.0f;       //排队长度开始统计的速度(m/s)
		//m_fLengthStartStat4QL = 20.0f;   //排队长度开始统计的长度要求(m)
		//m_fQLStatIntervalTime = 5.0f;     //排队长度统计间隔时间(s)


		m_bStop				= 0;
		m_nStopIgn			= 10;

		m_bDusai			= 0;
		m_nDusaiIgn			= 10;

		m_nQueueLength[0] = 30;
		m_nDusaiSpeed[0]=1;

		m_nQueueLength[1] = 25;
		m_nDusaiSpeed[1]=2;

		m_nQueueLength[2] = 20;
		m_nDusaiSpeed[2]=3;

		m_nQueueLength[3] = 15;
		m_nDusaiSpeed[3]=4;

		m_nQueueLength[4] = 10;
		m_nDusaiSpeed[4]=15;

		m_bOnlyOverSped		= 0;
		m_nOnlyOverSpedIgn	= 10;
		m_nOnlyOverSpedMax	= 50;

		m_bDiuQi			= 0;
		m_nDiuQiIgn			= 10;
		m_nDiuQiIDam		= 10;

		m_bAvgSpeed			= 0;
		m_nAvgSpeedIgn		= 10;
		m_nAvgSpeedMax		= 50;
		m_nAvgSpeedMin		= 10;

		m_bPerson			= 0;
		m_nPersonIgn		= 10;

		m_nStatFluxTime		= 10;
		m_bStatFlux			= 1;
		m_bStatSpeedAvg		= 1;
		m_bStatZyl			= 1;
		m_bStatQueue		= 1;
		m_bStatCtjj			= 1;
		m_bStatCarType		= 1;

		m_bBargeIn = 0;
		m_bFj = 0;
		m_bBeyondMark = 0;
		m_nAppearIgn  = 10;

		m_nAlertLevel = 1;

		m_nIgnStop = 30;
		m_nIgnStopNotJam = 5;

		m_nDusai = 30;
		m_bRight = 0;

		m_bPersonAgainst = 0;   //行人逆行
		m_bCross  = 0;          //车辆横穿
		m_bCarAppear  = 0;		//车辆出现
	    m_bAppear  = 0;        //出现
		m_bHalfMaxsize  = 0;    //

		m_nForbidType = 0;
		m_nAllowBigBeginTime = 0;
		m_nAllowBigEndTime = 0;

	    m_bCrowd = 0; //是否人群聚集检测
        m_nPersonCount = 5; //人数阈值
        m_nAreaPercent = 50; //人群区域占有率阈值（百分比）

        m_bPersonRun = 0;//是否奔跑检测
        m_fMaxRunSpeed = 3;//奔跑速度域值（m/s）

        nChannelMod = 0;

		strModelName = "";
	}

}VerChanParaStru;

//定义车道链表
typedef std::list<VerChanParaStru> VerChanParaList;


enum DETECT_OBJECT_TYPE
{
	DETECT_ALL_OBJECT  = 0,          //0-所有目标

	DETECT_VEHICLE_JUST = 1,   //1-只检车辆
	DETECT_PERSON_JUST                  //2-只检行人
};

typedef struct  _VEHICLE_PARAM_FOR_EVERY_FRAME
{
	int     nChannelID;   //车道ID
	int     nChannelMod;    //车道模型:0-机动车道,1-非机车道,2-混合车道
	bool  is_person_channel;  //false:机动车道,true:非机车道,2-混合车道
	bool  is_half_maxsize;   //对应是否行人优先,对应混合车道是否缩小maxsize
	bool is_right_side;            //右侧道路，不能判逆行
	int     m_nAngle;      //角度
	int     nModelId;       //模板Id
	char  chModelName[16]; //模板名称

	bool bShow;//是否是停留显示：1表示停留显示事件；0表示真实事件

	//----------------------事件检测配置------------------------//
	//检测目标
	bool m_bObjectAppear;    //是否检测目标出现
	int    m_nAppearIgn;         //目标出现忽略
	double ts_appear_alert;     //出现
	bool m_bCarAppear;         //！！delete
	int m_nObjeceDetectMod;    //！！add

	//逆行
	bool m_bNixing;               //逆行监测
	int   m_nNixingIgn;          //逆行事件忽略
	double ts_against_alert;    //逆行
	bool m_bPersonAgainst;           //！！delete
	int m_nAgainstDetectMod;    //！！add

	//变道
	bool m_bBianDao;          //变道监测
	int  m_nBianDaoIgn;       //变道监测忽略
	double ts_change_alert;   //变道
	int m_nChangeDetectMod;   //！！add

	//压黄线
	bool m_bPressYellowLine;          //压黄线监测
	int  m_nPressYellowLineIgn;       //压黄线监测忽略
	double ts_PressYellowLine_alert;  //上次压黄线发生的时间戳

	 //压导流线
     bool m_bPressLeadStreamLine;          //压导流线监测
     int  m_nPressLeadStreamLineIgn;       //压导流线监测忽略
     double ts_PressLeadStreamLine_alert;  //上次压导流线发生的时间戳 

	 //排队长度统计
	 bool  m_bStatQueueLen;     //是否统计排队长度                    //nahs
	 float m_fVeloStartStat4QL;       //排队长度开始统计的速度(m/s)
	 float m_fLengthStartStat4QL;   //排队长度开始统计的长度要求(m)//////
	 float m_fQLStatIntervalTime;      //排队长度统计间隔时间(s)


	//停止
	bool m_bStop;                 //停车监测
	int    m_nStopIgn;           //停车监测忽略
	int m_nStopIgnJam;    //拥堵时停车多久报事件
	int m_nStopIgnAlert;  //非拥堵时停车多久报事件
	double ts_stop_alert;      //上次停车发生的时间戳
	int m_nStopDetectMod;  //！！add

	//堵塞监测
	bool m_bDusai;          //堵塞监测
	int    m_nDusaiIgn;     //堵塞监测忽略
	int m_nDusaiIgnAlert;  //拥堵多久报事件
	int    m_nWayRate[ALERT_LEVEL];       //队列长度,最多分5级报警
	float  m_nDusaiSpeed[ALERT_LEVEL];  //平均最小速度
	int   alert_level;           //需要报警的级别
	int   result_level;         //保存以停留显示
	double ts_jam;          //堵塞的时刻
	double ts_jam_alert;         //堵塞报警的时刻
	double ts_jam_update;     //最新检测出堵塞的时刻
	double ts_crowd_update; //用于报停车的5级拥堵时刻
	double ts_noncrowd_update;
	bool  is_crowd;        //是否5级拥堵

	//速度检测
	bool m_bOnlyOverSped;     //单独超速报警
	int  m_nOnlyOverSpedIgn;  //单独超速忽略
	bool m_bAvgSpeed;             //平均车速监测
	int  m_nAvgSpeedIgn;          //平均车速忽略
	float  m_nAvgSpeedMax;     //平均车速最大
	float  m_nAvgSpeedMin;      //平均车速最小
	float  m_nOnlyOverSpedMax; //单独超速限制
	double ts_fast;                           //用于平均车速快速报警
	double ts_slow;                         //用于平均车速慢速报警
	double ts_fast_alert;                  //用于单独车速报警
	double ts_slow_alert;                //用于单独车速报警

	//丢弃物
	bool m_bDiuQi;          //丢弃物监测
	int  m_nDiuQiIgn;       //丢弃物监测忽略
	int  m_nDiuQiIDam;   //丢弃物的直径
	double ts_diuqi_alert;  //丢弃物

	//横穿
	bool m_bCross;           //横穿监测
	int  m_nCrossIgn;       //横穿监测忽略
	double ts_passerby_alert;  //上次横穿发生的时间戳
	bool m_bCarCross;               //！!delete
	int m_nCrossDetectMod;   //！!add

	//禁行
	int m_nForbidType;              //车道禁行车辆类型，默认不限制 0:无限制，1：禁行小车，2：禁行大车
	int m_nAllowBigBeginTime;    //车道允许大车行驶的最早时间点
	int m_nAllowBigEndTime;       //车道允许大车行驶的最晚时间点
	double ts_forbVehicle_alert;       //上次禁行机动车的时刻
	double ts_forbBigVT_alert;         //上次大车禁时的时刻
	double ts_forbBigVR_alert;         //上次大车禁道的时刻
	double ts_forbLashup_alert;        //上次进入应急车道的时刻

	//非机监测
	int  m_nWrongChanIgn; //非机监测忽略
	double ts_hunxing_alert;//混行
	//double ts_trafficAccident_alert;   //上次交通事故的时刻

	//----------------------行为分析配置------------------------//
	//越界
	bool m_bBeyondMark;   //越界检测
	double ts_beyondMark_alert;  //上次越界的时刻

	//闯入
	bool m_bBargeIn;    //闯入检测
	double ts_bargeIn_alert;         //上次闯入的时刻
	int m_nBargeInDetectMod;   //！！add

	//是否奔跑检测
	bool m_bPersonRun;  //是否奔跑检测
	float  m_fMaxRunSpeed;  //奔跑速度域值（m/s）
	int  m_nPersonRunIgn;  //奔跑忽略时间
	double ts_personrun_alert;  //行人奔跑
	double ts_personcrowd_alert;   //行人聚集

	//聚集
	bool m_bCrowd;   //是否人群聚集检测
	int  m_nCrowdIgn;  //聚集忽略时间
	int  m_nPersonCount; //人数阈值
	int  m_nAreaPercent; //人群区域占有率阈值（百分比）

	//----------------------交通参数统计配置------------------------//
	double ts_stat;                  //统计周期,不同车道统计周期可能不同
	bool m_bStatFlux;       //车道流量
	int   m_nStatFluxTime;    //车道流量检测的时间间隔
	bool m_bStatSpeedAvg;  //平均车速
	bool m_bStatZyl;          //平均车道占有率
	bool m_bStatQueue;      //队列长度
	bool m_bStatCtjj;       //平均车头间距
	bool m_bStatCarType;  //车辆分型


	_VEHICLE_PARAM_FOR_EVERY_FRAME()
	{
		nChannelID = 0;
		nChannelMod = 0;          //车道模型:0-机动车道,1-非机车道,2-混合车道
		is_person_channel = false;  //false:机动车道,true:非机车道,2-混合车道
		is_half_maxsize = false;
		is_right_side = false;
		m_nAngle = 0;
		nModelId = 0;
		memset(chModelName, 0, 16);

		bShow = false;


		//----------------------事件检测配置------------------------//
		m_bObjectAppear = false;//false;
		m_nAppearIgn = 10;
		ts_appear_alert = -1000;//行人出现
		m_bCarAppear = false;    //！delete
		m_nObjeceDetectMod = DETECT_ALL_OBJECT;    //！！add

		m_bNixing = false;
		m_nNixingIgn = 10;
		ts_against_alert = -1000;//逆行
		m_bPersonAgainst = false;   //！delete
		m_nAgainstDetectMod = DETECT_VEHICLE_JUST;    //！！add

		m_bBianDao = false;
		m_nBianDaoIgn = 10;
		ts_change_alert = -1000;//变道
		m_nChangeDetectMod = DETECT_VEHICLE_JUST;    //！！add

		m_bPressYellowLine	= false;			//压线监测
		m_nPressYellowLineIgn = 10;			//压线监测忽略
		ts_PressYellowLine_alert = -1000;

		 m_bPressLeadStreamLine = false;         //压导流线监测
		 m_nPressLeadStreamLineIgn = 60;   //压导流线监测忽略
		 ts_PressLeadStreamLine_alert = -1000.0; //上次压导流线发生的时间戳 

		 //排队长度统计
		 m_bStatQueueLen  = false;    //是否统计排队长度
		 m_fVeloStartStat4QL = 3.0f;       //排队长度开始统计的速度(m/s)
		 m_fLengthStartStat4QL = 20.0f;   //排队长度开始统计的长度要求(m)
		 m_fQLStatIntervalTime = 5.0f;     //排队长度统计间隔时间(s)

		m_bStop  = false;
		m_nStopIgn = 10;
		m_nStopIgnJam = 30;
		m_nStopIgnAlert = 5;
		ts_stop_alert = -1000;//停车
		m_nStopDetectMod = DETECT_VEHICLE_JUST;    //！！add

		m_bDusai = false;
		m_nDusaiIgn = 10;
		m_nDusaiIgnAlert = 30;
		m_nWayRate[0] = 30;
		m_nDusaiSpeed[0]=1;
		m_nWayRate[1] = 25;
		m_nDusaiSpeed[1]=2;
		m_nWayRate[2] = 20;
		m_nDusaiSpeed[2]=3;
		m_nWayRate[3] = 15;
		m_nDusaiSpeed[3]=4;
		m_nWayRate[4] = 10;
		m_nDusaiSpeed[4]=15;
		alert_level = 1;
		result_level = -1;
		ts_jam = -1;
		ts_jam_alert = -1;
		ts_jam_update = -1;
		ts_crowd_update = -1;
		ts_noncrowd_update = -1;
		is_crowd = false;

		m_bOnlyOverSped = false;
		m_nOnlyOverSpedIgn = 10;
		m_nOnlyOverSpedMax = 50;
		m_bAvgSpeed = false;
		m_nAvgSpeedIgn = 10;
		m_nAvgSpeedMax = 50;
		m_nAvgSpeedMin = 10;
		ts_fast = -1;
		ts_slow = -1;
		ts_fast_alert = -1000;   //快行
		ts_slow_alert = -1000; //慢行

		m_bDiuQi = false;
		m_nDiuQiIgn = 10;
		m_nDiuQiIDam = 10;
		ts_diuqi_alert = -1000;//遗弃物

		m_bCross = false;
		m_nCrossIgn = 10;
		ts_passerby_alert = -1000;//横穿
		m_bCarCross = false;  //！delete
		m_nCrossDetectMod = DETECT_PERSON_JUST;    //！！add

		m_nForbidType = 0;
		m_nAllowBigBeginTime = 0;
		m_nAllowBigEndTime = 0;
		ts_forbVehicle_alert  = -1000;      //上次禁行机动车的时刻
		ts_forbBigVT_alert = -1000;         //上次大车禁时的时刻
		ts_forbBigVR_alert = -1000;         //上次大车禁道的时刻
		ts_forbLashup_alert = -1000;        //上次进入应急车道的时刻

		ts_hunxing_alert = -1000;//混行
		//ts_trafficAccident_alert = -1000;   //上次交通事故的时刻


		//----------------------行为分析配置------------------------//
		m_bBeyondMark = false;
		ts_beyondMark_alert = -1000;  //上次越界的时刻

		m_bBargeIn = false;
		ts_bargeIn_alert = -1000;    //上次闯入的时刻
		m_nBargeInDetectMod = DETECT_ALL_OBJECT;   //！！add


		m_bPersonRun = 0;           //是否奔跑检测
		m_fMaxRunSpeed = 5.0f;          //奔跑速度域值（m/s）
		m_nPersonRunIgn = 60;
		ts_personrun_alert = -1000;   //行人奔跑
		ts_personcrowd_alert = -1000;   //行人聚集

		m_bCrowd = 0;    //是否人群聚集检测
		m_nCrowdIgn = 30;
		m_nPersonCount = 5;  //人数阈值
		m_nAreaPercent = 50;  //人群区域占有率阈值（百分比）

		//----------------------交通参数统计配置------------------------//
		ts_stat = -1;
		m_bStatFlux = true;        //车道流量
		m_nStatFluxTime = 60;  //车道流量检测的时间间隔
		m_bStatSpeedAvg = true; //平均车速
		m_bStatZyl = true;            //平均车道占有率
		m_bStatQueue= true;        //队列长度
		m_bStatCtjj = true;            //平均车头间距
		m_bStatCarType = true;    //车辆分型

	}
}VEHICLE_PARAM_FOR_EVERY_FRAME;

typedef struct _ROAD_PARAM { //道路的属性，配置时作为参数传递
	int nDetectTime; //检测时间，1白天/0傍晚/-1算法自动判断
	int nShadow;//是否检测阴影,1检测/0不检测/-1算法自动判断
	int nSensitive;//高敏感检测事件
	int nSameEventIgr;//同类事件的忽略事件,交警总队为180s,其他为0
	int nShowTime; //停留显示时间
	_ROAD_PARAM()
	{
		nDetectTime = 1;//默认为白天
		nShadow = 0;//默认为不检测阴影
		nSensitive = 0;////默认为不敏感
		nSameEventIgr = 180;//默认为交警总队配置
		nShowTime = 5;
	}
}ROAD_PARAM;

//检测参数链表
typedef std::list<VEHICLE_PARAM_FOR_EVERY_FRAME> paraDetectList;
typedef std::map<int,VEHICLE_PARAM_FOR_EVERY_FRAME> paraDetectMap;


//行为分析参数结构
typedef struct _BEHAVIOR_PARAM
{
    int	 nIndex;  //车道的序号
	//检测类型
	bool bDerelict_event; //遗弃物检测
	bool bPasserby_event; //行人横穿检测
	bool bRun_event; //行人奔跑检测
	bool bJam_event; //行人聚集检测
	bool bStoop_event; //行人停留检测
	bool bAgainst_event;// 行人逆行检测
	bool bAppear_event; // 行人出现检测
	bool bInside_event; // 行人闯入检测
	bool bOutside_event; // 行人越界检测
	bool bStatFlux_event; //流量检测
	bool bContext_event;    //横幅检测
    bool bSmoke_event;      //烟检测
    bool bFire_event;	    //火检测
    bool bFight_event;      //打架检测
    bool bLeafLet_event;	 //散传单检测
	bool bDensity_event;    //密度检测
	bool bLoitering_event;  //徘徊检测
	bool bRemovedItem_event; //遗留物品检测
	bool bImpropriate_event; //违章占道
	bool bSeeper_event;    //积水检测
	bool bTailgating_event; //尾随检测


	//与检测类型相关的阈值
	float fRunThreshold;//奔跑的阈值
	int   nPeopleNumberJam;//聚集的人的数目阈值


	_BEHAVIOR_PARAM()
	{
        nIndex = 0;
        bDerelict_event = 0;
        bPasserby_event = 0;
        bRun_event = 0;
        bJam_event = 0;
        bStoop_event = 0;
        bAgainst_event = 0;
        bAppear_event = 0;
        bInside_event = 0;
        bOutside_event = 0;
        bStatFlux_event = 0;
        bContext_event = 0;
        bSmoke_event = 0;
        bFire_event = 0;
        bFight_event = 0;
        bLeafLet_event = 0;
		bDensity_event = 0;

        fRunThreshold = 0;
        nPeopleNumberJam = 0;

		bLoitering_event = 0;
		bRemovedItem_event = 0;
		bImpropriate_event = 0;
		bSeeper_event = 0;
		bTailgating_event = 0;
	}
}BEHAVIOR_PARAM;
//行为分析参数链表
typedef std::list<BEHAVIOR_PARAM> paraBehaviorList;

typedef struct _Line {
	int nKind;
	PointList vPList;			//人行道区域的范围
								//指向点队列
	int nDirPt;		  //方向点的个数
	CPoint dirPts[2]; //方向点的坐标

	_Line()
	{
		nKind=0;
		vPList.clear();
		nDirPt = 0;
	}
}Line;

//查询结构
typedef struct _SEARCH_ITEM
{
	UINT32 uBeginTime;	    //开始时间
	UINT32 uEndTime;		    //结束时间
	UINT32 uChannelId;
	UINT32 uRoadId;
	UINT32 uPage;
	UINT32 uType;
	UINT32 uLevel;
	UINT32 uSortId;
	UINT32 uSortKind;
	char chText[MAX_PLATE];	        //车牌文本
	UINT32 uConditionType; //查询附加条件

	_SEARCH_ITEM()
	{
		uBeginTime = 0;
		uEndTime = 0;
		uChannelId = 0;
		uRoadId = 0;
		uPage = 0;
		uType = 0;
		uLevel = 0;
		uSortId = 0;
		uSortKind = 0;
		memset(chText,0,MAX_PLATE);
		uConditionType = 0;
	}

}SEARCH_ITEM;

//车牌高级查询构
typedef struct _SEARCH_ITEM_CARNUM
{
	UINT32 uBeginTime;	//开始时间
	UINT32 uEndTime;		//结束时间
	UINT32 uChannelId;
	UINT32 uRoadId;
	UINT32 uPage;
	UINT32 uType;
	UINT32 uLevel;
	UINT32 uSortId;
	UINT32 uSortKind;
	char chExceptText[MAX_EXCEPT_TEXT]; //排除在外的车牌省份缩写或非警车字符串
	char chText[MAX_PLATE];  //车牌文本

    UINT32 uColor;    //车牌颜色
	UINT32 uCarColor; //车身颜色
	UINT32 uCarType;  //车辆类型
	UINT32 uDetailCarType;  //车型细分子类
	UINT32 uDetailCarBrand;  //产商细分子类
	UINT32 uDirection;//行驶方向
	UINT32 uCarBrand; //产商标志
	char chPlace[MAX_PLACE]; //经过地点
	UINT32 uConditionType; //查询附加条件

	UINT32 uTypeDetail; //车牌类型细分
	UINT32 uViolationType; //违法类型
	UINT32 uStatusType; //记录类型

#ifdef WIN32
	void SearchItemCarnumInit()
	{
		uBeginTime = 0;
		uEndTime = 0;
		uChannelId = 0;
		uRoadId = 0;
		uPage = 0;
		uType = 0;
		uLevel = 0;
		uSortId = 0;
		uSortKind = 0;
		memset(chExceptText, 0, MAX_EXCEPT_TEXT);
		memset(chText,0,MAX_PLATE);

		uColor = 0;
		uCarColor = 0;
		uCarType = 0;
		uDetailCarType = 0;
		uDetailCarBrand = 0;
		uDirection = 0;
		uCarBrand = 0;
		memset(chPlace, 0, MAX_PLACE);
		uConditionType = 0;

		uTypeDetail = 0;
		uViolationType = 0;
		uStatusType = 0;
	}
#endif

	_SEARCH_ITEM_CARNUM()
	{
		uBeginTime = 0;
		uEndTime = 0;
		uChannelId = 0;
		uRoadId = 0;
		uPage = 0;
		uType = 0;
		uLevel = 0;
		uSortId = 0;
		uSortKind = 0;
		memset(chExceptText, 0, MAX_EXCEPT_TEXT);
		memset(chText,0,MAX_PLATE);

		uColor = 0;
		uCarColor = 0;
		uCarType = 0;
		uDetailCarType = 0;
		uDetailCarBrand = 0;
		uDirection = 0;
		uCarBrand = 0;
		memset(chPlace, 0, MAX_PLACE);
		uConditionType = 0;

		uTypeDetail = 0;
		uViolationType = 0;
		uStatusType = 0;
	}

}SEARCH_ITEM_CARNUM;

typedef struct _CHARTQUERY_ITEM   //图表查询
{
	UINT32 uChannelId;
	UINT32 uRoadId;
	UINT32 uYear;
	UINT32 uMonth;
	UINT32 uDay;
	UINT32 uHour;
	UINT32 uDateType;
	UINT32 uQueryType;
	UINT32 uTypeValue;
	_CHARTQUERY_ITEM()
	{
		uChannelId = 0;
		uRoadId = 0;
		uYear = 0;
		uMonth = 0;
		uDay = 0;
		uHour = 0;
		uDateType = 0;
		uQueryType = 0;
		uTypeValue = 0;
	}

}CHARTQUERY_ITEM;

typedef struct _MJpeg_Config
{
	UINT32 uChannelId;
	UINT32 uType;
	UINT32 uShutterTime;
	UINT32 uiAutoShutterUpLimit;
	UINT32	uiBrightnessTH;
	int	 bIsManual;
	UINT32 uiWBType;
	float	 fPGAUpLimit;
	float fAfeVGAGain;
	_MJpeg_Config()
	{
		uChannelId = 0;
		uType = 0;
		uShutterTime = 20000;
		uiAutoShutterUpLimit = 20000;
		uiBrightnessTH = 90;
		bIsManual = 0;
		uiWBType = 0;
		fAfeVGAGain = 0.0f;
		fPGAUpLimit = 10.0f;
	}
}MJpeg_Config;

//相机控制结构
typedef struct _CAMERA_CONFIG
{
	int nIndex;//设置命令类型
	int uType;//获取配置还是设置

	int uKind;//相机型号

	int uSM;
	int uSH;
	int uPE;
	int uGain;

	int uPol;//频闪灯极性
	int EEN_on;//开关频闪灯
	int nMode; //工作方式(0：连续方式,1：触发方式)
	int nLightType; //灯类型(主灯,从灯)
	int nFrequency; //频率

	//新相机参数
	int EEN_delay;
	int EEN_width;
	int ASC;
	int AGC;

	int nGamma;
	double fValue;//数据

	int nMaxPE;   //机动车道极大值
	int nMaxSH;
	int nMaxGain;
	int nMaxPE2;    //非机动车道极大值
	int nMaxSH2;
	int nMaxGain2;

	char chCmd[24];	
	
	int ElcSynFlag;//是否电源同步	
	int DspRedFlag;//红灯信号来源	
	int ForceRedFlag;//是否强制闯红灯	
	int nElcValue;//同步电源相位值

	UINT32 uRGB; //RGB值（四个字节，从低到高，只用低位3个字节）

	int nKakou; //卡口图片张数
	int nVts; //闯红灯图片张数
	int nEvent; //事件图片张数

	int nReboot; //相机重启计数器

	int nRedFlag; //红灯有效值
	char szTime[8];//实时时钟时间。11年8月31日xx周11点45分50秒xx毫秒
	int nGetOnePic; //手动触发抓图标志(0:关闭（默认） 1:打开)
	int nEnhance; //图像增强级别
	UINT32 uCameraIP; //相机IP
	UINT32 uGateWay; //相机网关
	UINT32 uNetMask; //相机子网掩码
	int nOperationType;//0:按下抬起,1：按下操作，2：抬起操作
	int nFlashOn; //打开爆闪灯(1:打开,0:关闭)
	int nFlashPort;//爆闪灯端口(从0开始编号,0表示第一路,以此类推一共4路)
	int nAddress;//地址码
	int nIris;
	int nMaxGamma;  //

	UINT32 nDigital;//亮度增强
	UINT32 nMaxDigital;//亮度增强最大值
	//第1个block
	UINT32 w1_set; //宽度
	UINT32 h1_set; //高度

	//第2个block
    UINT32 x2_set; //x 坐标
	UINT32 y2_set; //y 坐标
	UINT32 w2_set; //宽度
	UINT32 h2_set; //高度

    UINT32 uCameraID; //相机ID
	UINT32 uCameraMultiIP; //相机组播地址

	float fSH; //快门时间(float类型)
	float fGain; //增益(float类型)

	//LUT设置参数
	bool  UseLUT;
	double LUT_a;  //指数函数系数
	double LUT_b;  // 指数函数指数
	int   LUT_d;  //分段点

	//char chCamVersion[64]; //相机内部版本号

	_CAMERA_CONFIG()
	{
		nIndex = 0;
		uType = 0;
		uKind = 0;

		uSM=0;
		uSH = 0;
		uPE =0;
		uGain = 0;

		uPol = 0;
		EEN_on = -1;
		nMode = 0;
		nLightType = 1;
		nFrequency = 15;

		EEN_delay = 0;
		EEN_width = 0;
		ASC = 0;
		AGC = 0;

		nGamma = 0;
		fValue = 0;

		nMaxPE = 0;
        nMaxSH = 2000;
        nMaxGain = 0;
        nMaxPE2 =0;
        nMaxSH2 =0;
        nMaxGain2 =0;
        nMaxGamma = 130;
        nIris = 0;//最大
        nAddress = 0;

		memset(chCmd,0,33);

		ElcSynFlag = 0;//是否电源同步	
		DspRedFlag = 0;//红灯信号来源	
		ForceRedFlag = 0;//是否强制闯红灯
		nElcValue = 0;//同步电源相位值

		uRGB = 0;

        nKakou = 0;
        nVts = 0;
        nEvent = 0;
		nReboot = 0;
        nRedFlag = 0;
        memset(szTime,0,8);

		nGetOnePic = 0;
		nEnhance = 1; //默认取图像增强级别1
		uCameraIP = 0;
        uGateWay = 0;
        uNetMask = 0;
		nOperationType = 0;
		nFlashOn = 0; //打开爆闪灯
	    nFlashPort = 0;//爆闪灯端口

        //第1个block
        nDigital = 0;
        nMaxDigital = 0;
        w1_set = 0;
        h1_set = 0;

        //第2个block
        x2_set = 0;
        y2_set = 0;
        w2_set = 0;
        h2_set = 0;

        //第3个block
        uCameraID = 0;
        uCameraMultiIP = 0;
        fSH = 0;
        fGain = 0;

        //LUT设置参数
        LUT_a  =1;
        LUT_b  =0.7;
        LUT_d  =80;
        UseLUT =false;

        //memset(chCamVersion, 0, 64);
	}
}CAMERA_CONFIG;

//设置参数
enum CAMERA_MESSAGE
{
	CAMERA_PE  = 0,
	CAMERA_POL = 1,//设置高低电平
	CAMERA_EEN,
	CAMERA_GAIN,
	CAMERA_MAXPE,
	CAMERA_MAXSH,
	CAMERA_MAXGAIN,
	CAMERA_AGC,
	CAMERA_ASC,
	CAMERA_SH,
	CAMERA_SM,
	CAMERA_PEI,
	CAMERA_PEW,
	CAMERA_CMD,
	CAMERA_SA,
	CAMERA_GAMMA,
	CAMERA_LUT,
	CAMERA_MODE,
	/*CAMERA_MODE1,
	CAMERA_MODE2,*/
	CAMERA_LIGHTTYPE,
	CAMERA_FREQUENCY,
	CAMERA_IRIS,
	
	//第1个block
	X1SET,//21
	Y1SET,
	W1SET,
	H1SET,
	//第2个block
	X2SET, //25
	Y2SET,
	W2SET,
	H2SET,
	//第3个block
	X3SET, //29
	Y3SET,
	W3SET,
	CAMERA_DIGITALSHIFT,
	//模拟相机控制命令
	ZOOM_FAR,//33
	ZOOM_NEAR,
	FOCUS_FAR,
	FOCUS_NEAR,
	SHUTTER_INCREASE,
	SHUTTER_DECREASE,
	IRS_INCREASE,
	IRS_DECREASE,//40
	SET_PRESET,
	CLEAR_PRESET,
	GOTO_PRESET,
	LEFT_DIRECTION,
	RIGHT_DIRECTION,
	UP_DIRECTION,//46
	DOWN_DIRECTION,//47
	SWITCH_CAMERA,
    CAMERA_FLASH, //爆闪灯控制命令
    CAMERA_RESTART, //相机重启命令
    CAMERA_H264_STRING, //相机h264码流上叠加字符
    CAMERA_SET_IP, //设置相机IP
    CAMERA_ENHANCE, //图像增强(0:NONE 1:LEVEL1 2:LEVEL2 3:LEVEL3)
    CAMERA_GET_ONE_PIC, //手动抓图标志54
    CAMERA_SET_CLOCK,   //设置时钟55
    CAMERA_GET_CLOCK,    //获取时钟
    CAMERA_SET_REDFLAG,     //设置对应红灯有效值
    CAMERA_SET_N_KAKOU, //设置卡口-抓图张数
    CAMERA_SET_N_VTS, //设置闯红灯-抓图张数
    CAMERA_SET_N_EVENT, //设置事件-抓图张数
    CAMERA_SET_XML_ROAD_SETTING, //设置XML1文件 59
    CAMERA_GET_XML_ROAD_SETTING, //获取XML1文件
    CAMERA_SET_XML_ROAD_PARAM, //设置XML2文件
    CAMERA_GET_XML_ROAD_PARAM, //获取XML2文件
    CAMERA_SET_XML_OBJ_PARAM, //设置XML3文件
    CAMERA_GET_XML_OBJ_PARAM, //获取XML3文件
    CAMERA_SET_XML_VTS_PARAM, //设置XML4文件
    CAMERA_GET_XML_VTS_PARAM, //获取XML4文件
    CAMERA_SET_CHANNEL, //设置通道信息结构
    CAMERA_GET_CHANNEL, //获取通道信息结构
    CAMERA_SET_STRC, //设置算法自定义结构
    CAMERA_GET_STRC, //获取算法自定义结构72
    //CAMERA_GET_COUNT //获取车辆计数
	CAMERA_SET_DSP_SERVER_HOST, //设置dsp服务器Ip和端口
	CAMERA_SET_ROAD_CHANNEL_AREA, //车道检测区域
	CAMERA_SET_DSP_CARNUM_RGN, //设置车道检测区域
	CAMERA_SET_DSP_REDLIGHT_RGN, //设置红灯检测区域【左，右，直】
	CAMERA_SEND_LINK, //发送心跳77
	CAMERA_SET_RGB_PARAM, //设置相机 Red值，Green值，Blue值
	CAMERA_DSP_STREAM_TYPE, //设置DSP相机输出码流分辨率
	CAMERA_SET_DSP_ELEC_SYN, //设置电源同步
	CAMERA_SET_DSP_RED_FLAG, //设置红灯信号来源
	CAMERA_GET_DSP_GET_STRC_FLAG, //获取算法自定义结构标志(0:不存在，需设置 1:存在，无需设置)
	CAMERA_CHECK_DSP_VERSION, //核查DSP版本号
	CAMERA_GET_DSP_BRAND      //获取相机厂商
};


//车牌识别参数
#ifndef CARNUMPARM
#define CARNUMPARM
typedef struct carnum_parm
{
	int img_angle;
	int isday; //1: day, 0: night, -1: auto
	int direction;
	carnum_parm()
	{
	    img_angle = 0;
	    isday = 0; //1: day, 0: night, -1: auto
		direction = 0;
	}
}carnum_parm_t;
#endif
#ifndef WIN32
typedef std::map<int,carnum_parm_t> CnpMap;//车道编号<->车牌检测参数映射
#endif
//车牌黑白名单
typedef struct _SPECIALCARD
{
	UINT32 uId;   //记录编号
    UINT32 uKind;   //黑白牌种类--1:黑名单 2:白名单
    char chDepartment[60]; //布控单位
    UINT32 uBehavior_Kind; //布控行为--base on 1
    UINT32 uBegin_Time; //布控时间
    UINT32 uEnd_Time; //布控截止日期
    char chText[MAX_PLATE]; //号牌号码
    UINT32 uCarNumType; //号牌种类--base on 1
    UINT32 uCarType; //车辆类型--base on 1
    UINT32 uColor; //号牌颜色--base on 1
#ifdef WIN32
	void SPECIALCARD_INIT()
	{
		uId = 0;
        uKind = 0;
        memset(chDepartment, 0, 60);
		uBehavior_Kind = 0;
        uBegin_Time = 0;
        uEnd_Time = 0;
		memset(chText,0,MAX_PLATE);
        uCarNumType = 0;
        uCarType = 0;
        uColor = 0;
	}
#endif
	_SPECIALCARD()
	{
		uId = 0;
        uKind = 0;
        memset(chDepartment, 0, 60);
		uBehavior_Kind = 0;
        uBegin_Time = 0;
        uEnd_Time = 0;
		memset(chText,0,MAX_PLATE);
        uCarNumType = 0;
        uCarType = 0;
        uColor = 0;
	}

}SPECIALCARD;

/////////事件录像结构
struct AVI_FrameInfo
{
	int Position;// 图象帧在文件中的开始位置（单位为字节）
	int Length;// 图象帧的存储长度（单位为字节）
	UINT32 Time;// 图象帧的时间标签（单位为秒）
	UINT32 Time2;// 图象帧的时间标签（单位为毫秒）
};

struct AVI_EventInfo
{
	int FramePos;//事件发生的帧号
	int EventCoordX;//事件在图象中的横坐标
	int EventCoordY;//事件在图象中的纵坐标
};

struct AVI_Head
{
	int Flag; //0xFFAAFFBB（原始头）
	int FrameCount;// 文件中图象帧的数量（最多15000 帧）
	int EventCount;//事件数目
	int PlateCount; // 车牌数目

	AVI_FrameInfo FInfo[25*60*10];//图片帧信息
	AVI_EventInfo EInfo[16];//事件信息
};

//扩展avi头
struct AVI_Head_EX
{
    int Flag;//FFCCFFDD（扩展头）
	char chVersion[8]; //版本号码 v1.0.0.0
	RECORD_EVENT event[600]; //事件记录
	RECORD_PLATE plate[600]; //车牌记录
};


//图片查询结构
typedef struct _PICQUERY_ITEM
{
	UINT32 uId;//在数据库中的编号
	int uKind;//车牌图片还是事件图片
	int uType;//大图还是小图

	_PICQUERY_ITEM()
	{
		uId = 0;
		uKind = 0;
		uType = 0;
	}

}PICQUERY_ITEM;

/**
 * 摄像机状态
 */
typedef	enum CameraState
{
    CM_OK = 0,
    CM_NO_VIDEO = 1,
    CM_MOVED = 2,
    CM_SHAKED = 3,
	CM_BREAK  = 4,		//相机断开
    CM_RESERVED = 99
}CameraState;


//JPG头
typedef struct _Jpg_Header
{
	BYTE cSynchron[4];     //同步头
	UINT32 nJpgInfo;  //JPEG信息
	UINT16 nWidth; //水平分辨率
	UINT16 nHeight; //垂直分辨率
	UINT32 nFrameSeq;  //帧号(低3个字节为帧号；高字节为缩放比)
	UINT32 nSize;     //码流长度
    BYTE cReserved[8];  //预留

    _Jpg_Header()
    {
        memset(cSynchron,0,4);
        nJpgInfo = 0;
        nWidth = 0;
        nHeight = 0;
        nFrameSeq = 0;
        nSize = 0;
        memset(cReserved,0,8);
    }
}Jpg_Header;


//第二代线圈检测器参数
typedef struct _Speed_Signal
{
    unsigned short SpeedSignal;
    unsigned short FastSpeed;
    int64_t   SpeedTime;
    int64_t    SystemTime;
    _Speed_Signal()
    {
        SpeedSignal   =0;
        FastSpeed = 0;
        SpeedTime     =0;
        SystemTime       =0;

    }
}Speed_Signal;
//定义线圈检测器参数链表
typedef std::list<Speed_Signal> listSpeedSignal;

// YUV头
typedef struct _yuv_video_header
{
	BYTE cSynchron[4];     //同步头
    BYTE cType[4];     //排列类型
	UINT16 nWidth; //水平分辨率
	UINT16 nHeight; //垂直分辨率
//	UINT16 nFrameRate; //帧率
//	UINT16 nFieldSeq;  //场序号
    UINT32 nSeq;  //帧号(如果是500万相机：低8位为块号，高24位为帧号；如果是200万相机则：高16位为帧率，低16位为帧号)
	UINT32 nSize;     //码流长度

    BYTE cReserved[8];     //预留段

    _yuv_video_header()
    {
        memset(cSynchron,0,4);
        memset(cType,0,4);
        nWidth = 0;
        nHeight = 0;
 //       nFrameRate = 0;
 //       nFieldSeq = 0;
        nSeq = 0;
        nSize = 0;
		memset(cReserved,0,8);
    }
}yuv_video_header;

typedef struct _yuv_video_buf {
	BYTE cSynchron[4];     //同步头
	BYTE cType[4];         //排列类型
    UINT16   width;			//宽
    UINT16   height;			//高
	UINT16 nFrameRate; //帧率
	UINT16 nFieldSeq;  //场序号
    UINT32   size;		//大小

    BYTE uGain;     //当前增益
    BYTE uMaxGain;  //最大增益
    UINT16 nSh;  //当前快门值
    UINT16 nVideoType;  //视频编码格式(0:h264,1:表示mjpeg)
    BYTE nOE;           //奇场还是偶场
    BYTE nCameraControl;    //是否控制过相机

	UINT16 uFrameType;//场图像还是帧图像
	unsigned long  nSeq;        //帧号
	int64_t          ts;       //时间戳(从零开始的相对时间)
	UINT32   uTimestamp;  //系统实际时间

	UINT16 uTrafficSignal;//信号灯状态
	Speed_Signal uSpeedSignal;//线圈状态
	Speed_Signal uRadarSignal;//雷达状态
	UINT16 uMarginX;//裁切宽度
	UINT16 uMarginY;//裁切高度
	UINT16 uFlashSignal;//爆闪灯状态
    BYTE  *data;		//数据
	UINT32 uRecvTs;	//接收数据得时间

    _yuv_video_buf()
    {
        memset(cSynchron,0,4);
        memset(cType,0,4);
        width = 0;
        height = 0;
        nFrameRate = 0;
        nFieldSeq = 0;
        size = 0;
        uGain = 0;
        uMaxGain = 20;
        nSh = 0;

        nVideoType = 0;
        nOE = 0;
        nCameraControl = 0;

        uFrameType = 0;
        nSeq = 0;
        ts = 0;
        uTimestamp = 0;

       uTrafficSignal = 0;
       uMarginX = 0;
       uMarginY = 0;
       uFlashSignal = 0;
       data = NULL;
	   uRecvTs = 0;
    }
}yuv_video_buf;


// yuv图像坐标
typedef struct _RegionCoordinate
{
    //block1(违章检测区域)
    UINT32 uViolationX;
    UINT32 uViolationY;
    UINT32 uViolationWidth;
    UINT32 uViolationHeight;

    //block2(事件检测区域)
    UINT32 uEventX;
    UINT32 uEventY;
    UINT32 uEventWidth;
    UINT32 uEventHeight;

    //block3(红灯检测区域)
    UINT32 uTrafficSignalX;
    UINT32 uTrafficSignalY;
    UINT32 uTrafficSignalWidth;
    UINT32 uTrafficSignalHeight;

    _RegionCoordinate()
    {
        //block1
        uViolationX = 0;      //x轴坐标
        uViolationY = 0;      //y轴坐标
        uViolationWidth = 0;  //图像宽度
        uViolationHeight = 0; //图像高度

        //block2
        uEventX = 0;      //x轴坐标
        uEventY = 0;      //y轴坐标
        uEventWidth = 0;  //图像宽度
        uEventHeight = 0; //图像高度

        //block3
        uTrafficSignalX = 0;      //x轴坐标
        uTrafficSignalY = 0;      //y轴坐标
        uTrafficSignalWidth = 0;  //图像宽度
        uTrafficSignalHeight = 0; //图像高度
    }
}RegionCoordinate;



enum IMAGE_HEADER_DSP_TYPE
{
    DSP_IMG_SMALL_PIC,      //0:jpg小图
    DSP_IMG_BIG_PIC,        //1:jpg大图
    DSP_IMG_PLATE_INFO,     //2:车牌记录
    DSP_IMG_GATE_PIC,       //3:卡口抓图
    DSP_IMG_VTS_PIC,        //4:闯红灯抓图
    DSP_IMG_CONVERS_PIC,    //5:逆行抓图
    DSP_IMG_LOOP_INFO,      //6:线圈触发信息
    DSP_IMG_LINK,           //7.心跳信号
    DSP_IMG_YUV_PIC,        //8:YUV源
    DSP_IMG_H264_PIC,       //9:H264源
    DSP_IMG_EVENT_INFO,     //10.事件记录
    DSP_IMG_LOOP_PIC,       //11.线圈抓拍图
    DSP_IMG_VIDEO_PIC,      //12.视频抓拍图
	DSP_IMG_BIG_PIC2,		//13.每秒两帧大图
	DSP_IMG_ALG_TEST,		//14.算法调试

	DSP_IMG_TEST_PIC = 17,	//17.图像测试用
	DSP_IMG_DSPTIME,		//18.校Dsp时间

	DSP_LOG = 20,			//20.Dsp日志
	DSP_ERROR = 99,			//99.错误数据包
	DSP_REBOOT = 100		//100.相机重启
};

//JPG图像头
typedef struct _Image_header_dsp
{
    BYTE cSynchron[4];//同步头(例：4个$)
    UINT16 nType;//排列类型
    UINT16 nWidth;//水平分辨率
    UINT16 nHeight;//垂直分辨率
    UINT16 nFrameRate;//帧率
    UINT64 nSeq;//帧号-UINT64
    UINT32 nSize;//码流长度

    int64_t ts; //时间戳
    //BYTE RTC_TIME[8]; //实时时钟时间。11年8月31日xx周11点45分50秒xx毫秒

    UINT16 nCount; //计数器（每次触发后加1，到65535后归0，范围[0，65535]）
    UINT16 nOrder; //图片序号（每次触发根据具体需要，同一辆车，会出1-3张图,范围[0,2]）
    UINT16 nRoadIndex; //车道号(从1开始)

    UINT16 nRedColor; //红灯信号值
    int64_t SpeedTime; //线圈触发时间戳，单位us
    UINT16 nSpeedSignal; //线圈信号

    UINT16 nFlashFlag; //爆闪灯标志
	UINT32 uCameraId;//相机ID
	UINT32 uRecvTs;//检测器接收到数据时间,单位s
	char radarState; //雷达状态
	
	char szCameraCode[16];//相机编号
	char chReserved;//扩展-预留
	char ledState;//LED 灯状态（见LED 灯状态值结构定义）
	
	BYTE nBglight;//当前背景亮度
	UINT32 nSh;//相机当前快门值
	UINT32 nGain;//相机当前增益	

    _Image_header_dsp()
    {
        memset(cSynchron, 0, 4);
        nType = 0;
        nWidth = 0;
        nHeight = 0;
        nFrameRate = 0;

        nSeq = 0;
        nSize = 0;

        ts = 0;

        nCount = 0;
        nOrder = 0;
        nRoadIndex = 1;

        nRedColor = 0;
        SpeedTime = 0;
        nSpeedSignal = 0;

        nFlashFlag = 0;

		uCameraId = 1;

		uRecvTs = 0;
		radarState = '#';
		memset(szCameraCode,0,16);	
		chReserved = 0;
		ledState = 0xFF;

		nBglight = 0;
		nSh = 0;
		nGain = 0;	
    }
}Image_header_dsp;

//确认数据包
typedef struct _DSP_ACK
{
	BYTE cSynchron[4];//同步头(例：4个$)
	UINT64	uSeq;//帧号
	UINT16	nType;//排列类型
	
	_DSP_ACK()
	{
		memset(cSynchron, 0, 4);
		uSeq = 0;
		nType = 0;			
	}
}DSP_ACK;

//寻找图片的关键字
typedef struct _Picture_Key
{
	UINT64	uSeq;//帧号
	UINT32  uCameraId;//相机ID
	char szCameraCode[16];//相机编号(公交模式)

	_Picture_Key()
	{
		uSeq = 0;
		uCameraId = 0;
		memset(szCameraCode,0,16);
	}
	bool operator < (const _Picture_Key& other) const
	{
        if (uSeq < other.uSeq)        //帧号按升序排序
        {
             return true;
        }
        else if (uSeq == other.uSeq)  //如果帧号相同，按比相机ID升序排序
        {
             return uCameraId < other.uCameraId;
        }
        
        return false;
	}


}Picture_Key;

//套接字结构体绑定时间计数器用于超时处理
typedef struct _Client_Fd
{
	UINT32 uCameraId; //相机编号
	int fd;
	int TimeCount;
	bool connectFlag;
	std::string strIp;
	
	pthread_mutex_t FdMutex;
	_Client_Fd()
	{
		uCameraId = 0;
		fd = -1;
		TimeCount = 0;
		connectFlag = false;
		strIp = "";
		pthread_mutex_init(&FdMutex,NULL);
	}
}Client_Fd;

//客户端列表
typedef std::map<int,Client_Fd> MapClient;

//相机，客户端列表
typedef std::map<UINT32,Client_Fd> MapIdClient;

typedef struct _InductorInfoForPC
{
    BYTE  WayNum;//车道编号
    UINT16  nCount;//当前车道的车辆编号
    BYTE  peccancyCl;//违章类别，1、红灯，2，逆行

    BYTE  Inductor1edge1Color;//进入第一线圈的红灯状态，1：红灯，0：绿灯
    BYTE  Inductor1edge2Color;//出第一线圈的红灯状态，
    BYTE  Inductor2edge1Color;//进入第二线圈的红灯状态，1：红灯，0：绿灯
    BYTE  Inductor2edge2Color;//出第二线圈的红灯状态

    BYTE  Inductor1edge1Time[8];//进入第一线圈的实时时间，精确到10ms
    BYTE  Inductor1edge2Time[8];//出第一线圈的实时时间，精确到10ms
    BYTE  Inductor2edge1Time[8];//进入第二线圈的实时时间，精确到10ms
    BYTE  Inductor2edge2Time[8];//出第二线圈的实时时间，精确到10ms

    UINT32 Inductor1edge1TickCount;//进入第一线圈的计数器值，单位us
    UINT32 Inductor1edge2TickCount;//出第一线圈的计数器值
    UINT32 Inductor2edge1TickCount;//进第二线圈的计数器值
    UINT32 Inductor2edge2TickCount;//出第二线圈的计数器值

    BYTE  Reserved[64];

    _InductorInfoForPC()
    {
        WayNum = 1;
        nCount = 0;
        peccancyCl = 0;

        Inductor1edge1Color = 0;
        Inductor1edge2Color = 0;
        Inductor2edge1Color = 0;
        Inductor2edge2Color = 0;

        Inductor1edge1TickCount = 0;
        Inductor1edge2TickCount = 0;
        Inductor2edge1TickCount = 0;
        Inductor2edge2TickCount = 0;
    }
}InductorInfoForPC;


//奇偶场结构
typedef struct _rgb_buf
{
	int type;     //是否可以拼图
	BYTE* pCurBuffer; //偶场地址
	BYTE* pNextBuffer;//奇场地址
	_rgb_buf()
	{
		type = -1;
		pCurBuffer = NULL;
		pNextBuffer = NULL;
	}
}rgb_buf;

//时间结构(用于闯红灯检测查找时间)
typedef struct _TIMESTAMP
{
    int64_t          ts;       //时间戳(从零开始的相对时间)
    UINT32   uTimestamp;  //系统实际时间
    int nDis; //红灯后秒数
    UINT32 uFieldSeq;//帧号
}TIMESTAMP;




//车牌位置结构
typedef struct _PLATEPOSITION
{
    int64_t          ts;       //时间戳(从零开始的相对时间)
    UINT32   uTimestamp;  //系统实际时间
    int nDis; //红灯后秒数
    UINT32 uFieldSeq;//场号
	UINT32 uSeq;//帧号
	int nType;//车辆类型
	int IsCarnum;//是否车牌
    int x;//   车牌位置
    int y;
    int width;
    int height;
	int nRedColor; //红灯信号值
	CvRect rtCarPos;	//车身位置
//	int nDirection;//方向
	_PLATEPOSITION()
	{
		ts = 0;
		uTimestamp = 0;
		nDis = 0;
		uFieldSeq = 0;
		uSeq = 0;
		nType = 0;
		IsCarnum = 0;
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		nRedColor = 0;
	}
}PLATEPOSITION;


//交通灯方位
enum TRAFFIC_SIGNAL_DIRECTION
{
     EAST_STRAIGHT = 0,   //东直
     EAST_LEFT,     //东左

     SOUTH_STRAIGHT,   //南直
     SOUTH_LEFT,     //南左

     WEST_STRAIGHT,     //西直
     WEST_LEFT,     //西左

     NORTH_STRAIGHT,   //北直
     NORTH_LEFT,     //北左

     EAST_RIGHT,  //东右
     SOUTH_RIGHT,  //南右
     WEST_RIGHT,     //西右
     NORTH_RIGHT    //北右
};

//违章检测全局参数
typedef struct _VTS_GLOBAL_PARAMETER
{
    int bStrongLight; //是否红绿灯增强
    int bGlemdLight; //是否红绿灯防闪
    int bCheckLightByImage; //是否通过图像检测红绿灯
	int nRedLightTime;//红灯时间（以秒为单位）
	int nStrongEnhance;//增强系数
	int  nStopTime;  //停车时间
	int nSpeedVal;//超速上传实际值
	int bStrongSignal;//是否信号增强 
	char chReserved[36];

    _VTS_GLOBAL_PARAMETER()
    {
        bStrongLight = 0;
        bGlemdLight = 0;
        bCheckLightByImage = 0;

		nRedLightTime = 30;
		nStrongEnhance = 10;
		nStopTime = 0;
		nSpeedVal =0 ;//超速上传实际值 
		bStrongSignal = 0;
		memset(chReserved,0,36);
    }
}VTS_GLOBAL_PARAMETER;

//违章检测参数
typedef struct _VTS_PARAMETER
{
	int	  nRoadIndex;			//车道序号
	int      nVerRoadIndex;    //车道逻辑序号

   TRAFFIC_SIGNAL_DIRECTION  nLeftControl;  //左转控制
   TRAFFIC_SIGNAL_DIRECTION  nStraightControl;  //直行控制
   TRAFFIC_SIGNAL_DIRECTION  nRightControl;   //右转控制

   bool bForbidLeft; //禁左
   bool bForbidRight; //禁右
   bool bForbidRun  ;//禁行
   bool bForbidStop;//禁停

   bool bAgainst; //是否逆行检测
   bool bChangeRoad; //是否变道检测
   bool bPressLine; //是否压线检测

   int nForbidType;           //车道禁行车辆类型，默认不限制 0:无限制，1：禁行小车，2：禁行大车,3:禁所有车辆，4：禁行柴油车，5:禁行大货车
   int  nForbidBeginTime;    //车道禁止行驶的最早时间点
   int  nForbidEndTime;      //车道禁止行驶的最晚时间点

   int nRoadType; //车道类型，0：机动车道，1：非机动车道 ,2:公交车道，3：机非混合车道

   int nRoadDirection; //车道行驶方向(0:直行车道、1:左转车道、2:左转+直行车道、3:右转车道、4:右转+直行车道)
    int   nRedLightTime;//该车道的红灯时间长度,以秒为单位.

   bool nFlagHoldStopReg;//是否有待转区  
   int nRetrogradeItem;//逆行的多个名称，0：逆行，1：违反禁令标志，2：不按导向车道行驶。

   char chReserved[55];

	_VTS_PARAMETER()
	{
		nRoadIndex			= 0;
		nVerRoadIndex         = 0;

		nLeftControl = EAST_LEFT;
		nStraightControl = EAST_STRAIGHT;
		nRightControl = EAST_RIGHT;

		bForbidLeft = false;
		bForbidRight = false;
		bForbidRun = false;
		bForbidStop = false;

		bAgainst = false;
		bChangeRoad = false;
		bPressLine = false;

		nForbidType = 0;
		nForbidBeginTime = 0;
        nForbidEndTime = 0;
        nRoadType = 0;
        nRoadDirection = 0;
        nFlagHoldStopReg = 0;
		nRedLightTime = 60;
		nRetrogradeItem = 0;
        memset(chReserved,0,55);
	}

}PARAMETER_VTS;
//定义闯红灯检测参数链表
typedef std::map<int,PARAMETER_VTS> VTSParaMap;


//目标检测参数
typedef struct _ObjectPara
{
	//目标检测！
	bool      bDetectNonVehicle;              //是否检测非机动车!    默认为true!
	bool      bFilterSideVehicle;             //是否过滤边上机动车!   默认为true!
	bool      bFilterSideVlpObj;              //是否过滤车牌检测区域两边的*****目标！  默认为false！
	bool      bOutPutNonAutoMobile;             //晚上是否输出信号目标，对星号目标过暗的点位进行过滤，默认为输出
	bool      bDetectNonPlate;				//是否检测无牌车
	bool	  bDetectShield;				//是否检测遮阳板
    bool      bImageEnhance;				//是否图像增强
	int       nImageEnhanceFactor;          //图像增强参数因子
	bool      bDoMotorCar;					//是否检测摩托车
	int       nRedLightViolationAllowance;    //闯红灯信号开始的延迟帧数!   典型值为5！
	int       nFramePlus;                     //线圈目标输出结果的帧号修正量!   典型值为0！
	float     fLoopVehicleLenFix;             //线圈车长修正量!    典型值为1.0m！
	float     fFarRedLightVioPic;             //闯红灯取图参数!典型值分别为1.0
	float     fNearRedLightVioPic;             //闯红灯取图参数!典型值分别为0.5
	int       nDelayFrame;						//爆闪帧延时
	float	  fPressLineScale;				//车身压线比例
	int nStrongEnhance;//红绿灯增强系数(dsp相机用)

	_ObjectPara()
	{
		bDetectNonVehicle           = true;
		bFilterSideVehicle          = true;
		bFilterSideVlpObj           = false;
		bOutPutNonAutoMobile        = true;
		bDetectNonPlate			    = true;
		bDetectShield = false;
		bImageEnhance = false;
		nImageEnhanceFactor = 15;
		bDoMotorCar = false;
		nRedLightViolationAllowance = 3;
		nFramePlus = 0;
		fLoopVehicleLenFix = 1.0;
		fFarRedLightVioPic = 1.0;
		fNearRedLightVioPic = 0.5;
		nDelayFrame = 3;
		fPressLineScale = 0.7;
		nStrongEnhance = 7;
	}
};


//线圈检测参数
typedef struct _LOOP_PARAMETER
{
	int	  nRoadIndex;		//车道序号
	int   nVerRoadIndex;    //车道逻辑序号
    int   nLoopIndex;      //线圈组号
    double   fDistance;        //线圈距离

	_LOOP_PARAMETER()
	{
		nRoadIndex			= 0;
		nVerRoadIndex         = 0;
		nLoopIndex = 0;
		fDistance = 0;
	}

}PARAMETER_LOOP;
//定义线圈检测参数链表
typedef std::map<int,PARAMETER_LOOP> LoopParaMap;


//线圈状态
typedef struct _LOOP_STATUS
{
	int	  nRoadIndex;		//车道序号
	bool  bFrontLoop;		//前线圈状态
	bool  bBackLoop;		//后线圈状态

	_LOOP_STATUS()
	{
		nRoadIndex	= 0;
		bFrontLoop = 0;
		bBackLoop = 0;
	}

}LOOP_STATUS;
//定义线圈状态链表
typedef std::map<int,LOOP_STATUS> LoopStatusMap;

//雷达检测参数
typedef struct _RADAR_PARAMETER
{
    int nRoadIndex; //车道编号
    int nCoverRoadIndex;   //覆盖车道编号(用2进制位表示车道号)
    float fSpeedFactor; //速度矫正因子，可为负数(例：12.0f,使用 [Speed源 * (1+fSpeedFactor/100.0f)]

	_RADAR_PARAMETER()
	{
	    nRoadIndex = 0;
		nCoverRoadIndex = 0;
		fSpeedFactor = 0.0;
	}
}RADAR_PARAMETER;
//定义雷达检测参数链表
typedef std::map<int,RADAR_PARAMETER> RadarParaMap;

//车牌检测参数
typedef struct _CARNUM_PARAMETER
{
	int	  nRoadIndex;			//车道序号
	int      nVerRoadIndex;    //车道逻辑序号

   int  nHorizontalAngle;  //水平旋转角度
   int  nVerticalAngle;  //垂直旋转角度

   int  nFarWidth;   //车牌远处宽度
   int  nFarHeight;   //车牌远处高度
   int  nNearWidth;   //车牌近处宽度
   int  nNearHeight;   //车牌近处高度

	_CARNUM_PARAMETER()
	{
		nRoadIndex			= 0;
		nVerRoadIndex         = 0;

		nHorizontalAngle = 0;
		nVerticalAngle = 0;

		nFarWidth = 0;
		nFarHeight = 0;
		nNearWidth = 0;
		nNearHeight = 0;
	}

}CARNUM_PARAMETER;
//车牌检测参数链表
typedef std::list<CARNUM_PARAMETER> CarNumParaList;

//显示区域结构
typedef struct _ImageRegion
{
    REGION_IMAGE nImageRegionType; //图像区域类型
	int	   x;			//左
	int      y;          //上
    int      width;    //宽
	int     height;     //高

	_ImageRegion()
	{
	    nImageRegionType = WHOLE_IMAGE;  //全景图像
		x	= 0;
		y   = 0;
		width  = 0;
		height = 0;
	}

}ImageRegion;

//停车严管矩形
typedef struct _ParkRect
{
	int left;
	int top;
	int right;
	int bottom;
	_ParkRect(int Lt = -1,int nTp = -1,int  Rt = -1,int Bm = -1)
	{
		left = Lt;
		top = nTp;
		right = Rt;
		bottom = Bm;
	}
}ParkRect;

//停车严管信息
typedef struct _RectObject
{
	int nId;      //目标的编号
	ParkRect lpRect; //目标的位置
	int bGetRoof;//是否完成取证
	int bLeave;  //是否已离开
	char chReserved[32];
	_RectObject()
	{
		nId = 0;
		//lpRect = CRect(-1,-1,-1,-1); 
		bGetRoof = 0;
		bLeave = 0;
		memset(chReserved,0,32);
	}
}RectObject;

//区域亮度信息
#ifndef REGIONCONTEXT
#define REGIONCONTEXT
typedef struct REGION_CONTEXT
{
	double     mean;               //区域平均亮度
	double     stddev;				//区域方差
	double  carnummean;             //车牌平均亮度
	double  carnumstddev;           //车牌方差

	 REGION_CONTEXT()
	{
		mean = 0;
		stddev = 0;
		carnummean = 0;
		carnumstddev = 0;
	}
}region_context;
#endif

#ifndef WIN32
typedef std::list<CvPoint> TRACK_GROUP;


//用于同步的特征数据结构
typedef struct SYN_CHARACTER_DATA
{
    UINT32     uId;    //该记录的唯一标识
    UINT32     uTimeStamp; //压入队列的时间点，精确到秒
    float       fAccuracy;  //车牌检测准确度
    int         nWidth;     //检测到的物体/车牌宽度
    int         nDistance;  //车牌中心到相对边缘的距离
    int         nType;      //0表示事件，1表示车牌
    char        szPlate[MAX_PLATE]; //车牌信息
    map<int64_t, TRACK_GROUP> mapTrack;   //轨迹map

    SYN_CHARACTER_DATA()
    {
        uId = 0;
        uTimeStamp = 0;
        fAccuracy = 0.0;
        nWidth = 0;
        nDistance = 0;
        nType = 0;
        memset(szPlate, 0, MAX_PLATE);
        mapTrack.clear();
    }

    SYN_CHARACTER_DATA(SYN_CHARACTER_DATA& data)
    {
        uId = data.uId;
        uTimeStamp = data.uTimeStamp;
        fAccuracy = data.fAccuracy;
        nWidth = data.nWidth;
        nDistance = data.nDistance;
        nType = data.nType;
        memcpy(szPlate, data.szPlate, MAX_PLATE);
        mapTrack = data.mapTrack;
    }
}SYN_CHAR_DATA;
#endif

typedef struct _COM_SETTING
{
    //控制相机串口编号
    int nCameraComport;
    int nCameraBaud;
    //控制交通灯串口编号
    int nVTSComport;
    int nVTSBaud;
    //控制车检器串口编号
    int nDHComport;
    int nDHBaud;
    //控制补光灯串口编号
    int nLightComport;
    int nLightBaud;

    _COM_SETTING()
    {
        nCameraComport = 1;
        nCameraBaud = 9600;
        nVTSComport = 2;
        nVTSBaud = 9600;
        nDHComport = 3;
        nDHBaud = 115200;
        nLightComport = 4;
        nLightBaud = 9600;
    }
}COM_SETTING;

//串口参数
typedef struct _COM_PARAMETER
{
    int nComPort;//串口编号(从1开始)
	/******************************************************************
	1:相机；        2：交通灯；   3:车检器；    4:补光灯；    5:vis;
	6:Gps；		    7:雷达；      8:爆闪灯；    9:开关门报警；10:Dio 
	11:集和诚液晶； 12:威强液晶； 13:慧昌雷达； 14:交通灯A；
	******************************************************************/
    int nComUse; //串口用途
    int nBaud;//波特率（2400，9600，19200，57600，115200）
    int nDataBits;//数据位（7，8）
    int nStopBits;//停止位（0，1）
    int nParity; //检验位（0无检验，1奇校验，2偶校验）

    _COM_PARAMETER()
    {
        nComPort = 0;
        nComUse = 1;
        nBaud = 9600;
        nDataBits = 8;
        nStopBits = 1;
        nParity = 0;
    }
}COM_PARAMETER;

typedef std::map<int,COM_PARAMETER> COM_PARAMETER_MAP;

//GPS设置
typedef struct _GPS_SET_INFO
{
    int nType; //设备类型(0:清华紫光，1：上海电警gps设备)
    char  szReserved[32];    //保留字段

	_GPS_SET_INFO()
	{
	   nType = 0;
	   memset(szReserved,0,32);
	}
}GPS_SET_INFO;

//云台控制参数
typedef struct _YUNTAI_CONTROL_PARAMETER
{
    int nNeedControl;     //是否需要进行控制(0:不需要,1:需要)
    int nControlMode; //控制方式(0:串口方式,1:网口方式)
    int nKeyBoardID;  //键盘编号
    char szVisHost[SKP_MAX_HOST];    //vis服务地址
    int nVisPort;               //vis服务端口
    int nProtocalType;       //协议类型(0:虚拟键盘码协议,1:MCIP协议,2:pelco协议)
    int nMultiPreSet;        //是否存在多个预置位
    int nHasLocalPreSet;       //是否存在近景预置位
	char szSerialHost[SKP_MAX_HOST];    //Serial服务地址
	int nSerialPort;               //Serial服务端口
	int nAddressCode;			//地址码
	int nStopInterval;//违停抓拍时间,默认60秒
	int nRemotePicInterval; //近景图片目标比例
	int nPicComPoseMode;//图片组合方式       
	int nPreSetMode;//预置位模式，0表示自动，1表示手动
	int nCameraAutoMode;//是否使用3D智能抓拍方式 0表示不适用 1表示使用
	int nCarnumToDBMode; //未识别车牌是否入库 0表示不入库 1表示入库
    int nHandCatchTime;    //手动抓拍时间间隔

   

    _YUNTAI_CONTROL_PARAMETER()
    {
        nNeedControl = 0;
        nControlMode = 0;
        nKeyBoardID = 0;
        memset(szVisHost,0,SKP_MAX_HOST);
        nVisPort = 0;
        nProtocalType = 0;
        nMultiPreSet = 0;
        nHasLocalPreSet = 0;
		memset(szSerialHost,0,SKP_MAX_HOST);
		nSerialPort = 0;
		nAddressCode = 0;
		nStopInterval = 60;
		nRemotePicInterval = 60;
		nPicComPoseMode = 0;                     
		nPreSetMode = 0;
		nCameraAutoMode = 0;
		nCarnumToDBMode = 0;
        nHandCatchTime = 0;
    }
}YUNTAI_CONTROL_PARAMETER;


//车道描述
typedef struct _ROAD_DESC
{
	char    chDirection[64];    //车道方向
	char    chDeviceDesc[64];    //车道描述

	 _ROAD_DESC()
	{
		memset(chDirection,0,64);
		memset(chDeviceDesc,0,64);
	}
}ROAD_DESC;

//道路->方向映射
typedef std::map<int, ROAD_DESC> ROAD_DIRECTION_MAP;

//车道编号映射
typedef struct _ROAD_INDEX_INFO
{
	int nVerRoadIndex; //车道逻辑编号
	int nDirection; //车道方向
	_ROAD_INDEX_INFO()
	{
	    nVerRoadIndex = 0;
		nDirection = 0;
	}
}ROAD_INDEX_INFO;

typedef std::map<int,ROAD_INDEX_INFO> RoadIndexMap;//车道编号<->车道方向映射

typedef struct MvCarNumInfo
{
	char cCarNum[7];       //车牌号码
	char wjcarnum[2];      //武警牌中间的两个小字
	int     color;            //车牌颜色
	int     vehicle_type;     //车辆类型
	int     carnumrow;        //单排或者双排车牌
	int nChannel;				//车道号（目前暂时无法给出）
	int64_t nIndex ;				//图像帧号
	int64_t ts;					//图像时间戳
	CvRect rtCarbrand;			//车牌位置
	bool bIsFilter;
	MvCarNumInfo()
	{
		memset(cCarNum, 0, 7);
       memset(wjcarnum, 0, 2);
	   color = 0;
	   vehicle_type = 0;
	   carnumrow = 0;
	   nChannel = 0;
	   ts = 0;
	   bIsFilter = false;
	}

}MvCarNumInfo;

//预置位信息
typedef struct _PreSetInfo
{
	int nPreSetID;//预置位编号(近景)
    Point32fList listRegion;//预置位位置

	_PreSetInfo()
	{
        nPreSetID = 0;
	}

}PreSetInfo;

typedef std::list<PreSetInfo> PreSetInfoList;


//特征检测数据结构头
typedef struct _FEATURE_DETECT_HEADER
{
	UINT32 uCameraID;		//相机ID
	UINT32 uCmdID;		//检测结果类型
	int64_t	uTime64;	    //时间戳
	UINT32 uWidth;			//宽度
	UINT32 uHeight;			//高度
	UINT32 uPicSize;		//图片大小
	CvRect farrect;			//远处行人框
	CvRect nearrect;		//近处行人框
	UINT32 nWorkMode;		//工作模式
	UINT32 uSeq;		//帧号

	_FEATURE_DETECT_HEADER()
	{
		uCameraID = 0;
		uCmdID = 0;
		uTime64 = 0;
		uWidth = 0;
		uHeight = 0;
		uPicSize = 0;
		nWorkMode = 0;
		farrect = cvRect(0,0,0,0);
		nearrect = cvRect(0,0,0,0);
		uSeq = 0;
	}
}FEATURE_DETECT_HEADER;

//设置H264码流文字叠加内容和格式
typedef struct _STREAM_OVERLAY_TEXT
{
    UINT32 uPoLeft; //叠加字符起始x坐标
    UINT32 uPosUp; //叠加字符起始y坐标
    UINT32 uColor; //叠加字符颜色
    UINT32 uFontSize; //叠加字符大小

    UINT32 uOsdcharactFlag; //是否显示字符 (0:否 1:是)
    UINT32 uOsdtimeFlag; //是否显示时间 (0:否 1:是)
    UINT32 uOsdweekFlag; //是否显示星期几 (0:否 1:是)
    UINT32 uOsdweeklang; //显示星期几的语言种类(0:英文 1:汉字)

    char chText[100]; //叠加字符内容
    char chReserved[76]; //保留字段

    _STREAM_OVERLAY_TEXT()
    {
        uPoLeft = 0;
        uPosUp = 0;
        uColor = 11; //未知
        uFontSize = 0;

        uOsdcharactFlag = 0;
        uOsdtimeFlag = 0;
        uOsdweekFlag = 0;
        uOsdweeklang = 0;

        memset(chText, 0, 100);
        memset(chReserved, 0, 76);
    }
}STREAM_OVERLAY_TEXT;

//北京H264视频传输控制协议头
typedef struct _BJ_CTRL_HEADER
{
	UINT32 uCommand;		//命令类型
	UINT32 uCmdLen;		//消息长度
	UINT32 uCameraID;   //相机编号
	UINT32 uCmdFlag;	//命令标志

	_BJ_CTRL_HEADER()
	{
		uCommand = 0;
		uCmdLen = 0;
		uCameraID = 0;
		uCmdFlag = 0;
	}
}BJ_CTRL_HEADER;

#define MAX_REGION_NUMS 20 //最大区域个数
#define MAX_REGION_POINTS_NUMS 8 //区域最大点数目

//区域类型定义
enum DSP_REGION_TYPE
{
	DSP_RGN_DEFAULT,		//0.默认无类型
	DSP_RGN_ROAD_WAY,		//1.道路区域
	DSP_RGN_CHANNEL_WAY,	//2.车道区域
	DSP_RGN_TRAFFIC_SIGNAL,	//3.红灯检测区域
	DSP_RGN_RED_LIGHT,		//4.红灯区域
	DSP_RGN_GREEN_LIGHT,	//5.绿灯区域
	DSP_RGN_STOP_LINE,		//6.停止线
	DSP_RGN_YELLOW_LINE		//7.双黄线
};


//区域类型，区域值，区域个数（最多20个），各区域点个数(每个区域最多8个点)，各区域点坐标
typedef struct _DSP_ROAD_SETTING
{	
	UINT32 uRgnNums; //区域个数

	UINT32 uRgnType[MAX_REGION_NUMS]; //区域类型
	UINT32 uRgnValue[MAX_REGION_NUMS]; //区域值
	UINT32 uPointNums[MAX_REGION_NUMS]; //各区域点个数
	CPoint uPointsArray[MAX_REGION_NUMS][MAX_REGION_POINTS_NUMS]; //各区域点坐标
	
	char cReserved[152]; //预留字节
	
	_DSP_ROAD_SETTING()
	{
		uRgnNums = 0;
		for(int i=0; i<MAX_REGION_NUMS; i++)
		{
			uRgnType[i] = 0;
			uRgnValue[i] = 0;
			uPointNums[i] = 0;
			for(int j=0; j<MAX_REGION_POINTS_NUMS; j++)
			{
				uPointsArray[i][j].x = 0;
				uPointsArray[i][j].y = 0;
			}
		}	
		
		memset(cReserved, 0, 152);
	}
}DSP_ROAD_SETTING;

typedef struct _RG_Rect
{
	int x0;
	int y0;
	int width;
	int height;

	_RG_Rect()
	{
		x0 = 0;
		y0 = 0;
		width = 0;
		height = 0;
	}
}RG_Rect;

typedef struct
{
	RG_Rect red_left;
	RG_Rect green_left;	
	
	RG_Rect red_straight;
	RG_Rect green_straight;

	RG_Rect red_right;
	RG_Rect green_right;	

}RG_REGION;//红绿灯检测区域

//红绿灯信号状态
typedef struct _EVENT_VTS_SIGNAL_STATUS
{
	int  nRoadIndex;	   //车道序号
	unsigned int uFrameSeq;

	bool bLeftSignal;     //左转灯状态.红灯true。绿灯false。
	bool bStraightSignal; //直行灯状态
	bool bRightSignal;    //右转灯状态

	_EVENT_VTS_SIGNAL_STATUS()
	{
		uFrameSeq       = 0;
		nRoadIndex      = -1;
		bLeftSignal     = false;
		bStraightSignal = false;
		bRightSignal    = false;
	}

}EVENT_VTS_SIGNAL_STATUS;


typedef struct _DSP_RGB_PARAM
{
	UINT32 uRed; //Red值，对应相机的Red值（float型）*100；
	UINT32 uGreen; //Green值，对应相机的Green值（float型）*100；
	UINT32 uBlue; // Blue值，对应相机的Blue值（float型）*100；

	_DSP_RGB_PARAM()
	{
		uRed = 0;
		uGreen = 0;
		uBlue = 0;
	}
} DSP_RGB_PARAM;

//强制报警结构体
typedef struct _FORCEALERT
{
	int nAlertType;
	int nX;
	int nY;
}FORCEALERT;


#ifndef struct MaxSpeedStr
typedef struct _MaxSpeedStr
{
	UINT32 nRadarAlarmVelo; //雷达预警速度
	UINT32 uAlarmBig;		//大车限速
	UINT32 uAlarmSmall;		//小车限速

	_MaxSpeedStr()
	{
		nRadarAlarmVelo = 0;
		uAlarmBig = 0;
		uAlarmSmall = 0;
	}
}MaxSpeedStr;
#endif

#ifndef mapMaxSpeed
typedef map<UINT32,MaxSpeedStr> mapMaxSpeedStr; //车道对应的限速map
#endif

#ifndef mapChanMaxSpeed
typedef map<UINT32,mapMaxSpeedStr> mapChanMaxSpeedStr; //某通道对应的限速map
#endif

//套接字结构体绑定时间计数器用于超时处理
typedef struct _DspClient_Fd
{
	char ip[16];
	int port;
	int fd;
	int TimeCount;
	bool connectFlag;
	int nRecvFlag;//接受数据状态标志 0:未接受 1:接受数据中
	int nTimeFlag;//开始接收数据时间标记
	//pthread_mutex_t FdMutex;
	_DspClient_Fd()
	{
		memset(ip, 0, 16);
		port = 0;
		fd = -1;
		TimeCount = 0;
		connectFlag = false;
		nRecvFlag = 0;
		nTimeFlag = 0;
		//pthread_mutex_init(&FdMutex,NULL);
	}
}DspClient_Fd;

typedef struct _DspSocketFd
{
	void* p;//CDspServer类型指针
	void* pProcess;//CDspDataProcess类型指针
	DspClient_Fd cfd;
	_DspSocketFd()
	{
		p = NULL;
	}

}DspSocketFd;

//信号机设置
typedef struct _SIGNAL_SET_INFO
{
	int nType; //设备类型(0:泰科信号机，1：骏马信号机)
	int nMode; //0:串口，1：网口
	int nExist;//是否存在信号机
	char  szReserved[32];    //保留字段
	_SIGNAL_SET_INFO()
	{
		nType = 0;
		nMode = 0;
		nExist  = 0;
		memset(szReserved,0,32);
	}
}SIGNAL_SET_INFO;

typedef struct _DspVideoClient_Fd
{
	char szIp[MAX_IP_LEN];
	int nPort;

	int nFd;
	int nTimeCount;
	bool bConnectFlag;

	int nConnType;
	char szUrl[MAX_URL_LEN];
	char szUser[MAX_RTSP_USER_LEN];
	char szPw[MAX_RTSP_PASSWD_LEN];

	UINT64 ubiRtspStrmId;
	UINT64 ubiTCPStrmId;

	_DspVideoClient_Fd()
	{
		memset(szIp, 0, MAX_IP_LEN);
		nPort = 0;
		nFd = -1;
		nTimeCount = 0;
		bConnectFlag = false;

		nConnType = 1;
		memset(szUrl, 0, MAX_URL_LEN);
		memset(szUser, 0, MAX_RTSP_USER_LEN);
		memset(szPw, 0, MAX_RTSP_PASSWD_LEN);
		ubiRtspStrmId = 0;
		ubiTCPStrmId = 0;
	}
}DspVideoClient_Fd;

typedef struct  _WIDHEI_H264
{
	UINT32 nWidth;
	UINT32 nHeight;
	_WIDHEI_H264() 
	{ 
		nWidth = 0; 
		nHeight = 0;
	}
}WIDHEI_H264;

typedef struct _DSP_INFO
{
	char szIp[16]; //ip地址 
	char chReserve[512]; //预留 
	_DSP_INFO()
	{
		memset(szIp, 0, 4);
		memset(chReserve, 0, 512);
	}
}DSP_INFO;

typedef std::list<DSP_INFO> DSP_LIST;

typedef struct _RECORD_PLATE_DSP_MATCH
{
	RECORD_PLATE dspRecord;
	char szDeviceID[MAX_DEVICE_CODE];//设备编号

	IplImage *pImg;//图片
	IplImage *pImgArray[3];//图片

	UINT64 uKeyArray[4];//使用图片记录全局编号
	bool bKeyStateArray[4];//使用图片记录状态 0:记录为空 1:记录存在

//#ifdef MATCH_LIU_YANG
	char * pPicArray[4];//图片
	int nSizeArray[4];//图片大小
	PLATEPOSITION pPlatePos[4];//车牌位置结构
//#endif

	bool bUse; //当前状态 false:未使用 true:使用中	

	_RECORD_PLATE_DSP_MATCH()
	{
		memset(szDeviceID, 0, MAX_DEVICE_CODE);
		pImg = NULL;

		pImgArray[0] = NULL;
		pImgArray[1] = NULL;
		pImgArray[2] = NULL;

//#ifdef MATCH_LIU_YANG
		//pPic = NULL;
		for(int i=0; i<4; i++)
		{
			uKeyArray[i] = 0;
			bKeyStateArray[i] = false;
			pPicArray[i] = NULL;
			nSizeArray[i] = 0;
		}		
//#endif

		bUse = false;		
	}

}RECORD_PLATE_DSP_MATCH;


//录像路径相关信息
typedef struct _VIDEO_PATH_INFO
{		
	UINT32 uId;
	FILE * fpOut;
	std::string strPath;
	int64_t uBeginTime;
	int64_t uEndTime;

	_VIDEO_PATH_INFO()
	{
		uId = 0;
		fpOut = NULL;
		strPath = "";
		uBeginTime = 0;
		uEndTime = 0;
	}

}VIDEO_PATH_INFO;

typedef struct _Picture_Elem{
	Picture_Key key;
	int64 ts;

	_Picture_Elem()
	{
		key.uCameraId = 0;
		key.uSeq = 0;
		ts = 0;
	}

	bool operator < (const _Picture_Elem& other) const
	{
		if (ts < other.ts)
		{
			return true;
		}
		else if (ts == other.ts)  
		{
			return key.uSeq < other.key.uSeq;
		}

		return false;
	}
}Picture_Elem;

typedef map<Picture_Elem, string> JpgMap;

#ifndef VIDEO_INFO
typedef struct _VIDEO_INFO
{	
	UINT32 uId;
	FILE * fp;
	char chPath[260];
	int64_t uBeginTime;
	int64_t uEndTime;
	int nFrameRate; //帧率0:1,1:2.5,2:5,3:7.5,4:10,5:12.5,6:15,7:17.5,8:20,9:22.5,10:25	
	bool bState;//false:未处理 true:已处理

	_VIDEO_INFO()
	{
		uId = 0;
		fp = NULL;
		memset(chPath, 0, 260);
		uBeginTime = 0;
		uEndTime = 0;
		nFrameRate = 10;
		bState = true;
	}
}VIDEO_INFO;
#endif

typedef struct _PlateLimit
{
	UINT32 nIsPlateLimit; //是否尾号限行
	char chPlateNumber[4];         //限行尾号(1位数字或字母)
	char chReserved[128];	 //扩展
	_PlateLimit()
	{
		nIsPlateLimit = 0;
		memset(chPlateNumber,0,4);
		memset(chReserved,0,128);				//扩展
	}
}PlateLimit;

//路段名经纬度信息
typedef struct _sRoadNameInfo
{
	char chKaKouItem[128];		//卡口编号
	char chPosNumber[128];		//地点编号
	char chRoadName[128];		//路段名
	char chDirection[128];		//方向
	char chStartPos[128];		//起始点
	char chEndPos[128];			//终止点
	double dStartPosX;			//起始点经度
	double dStartPosY;			//起始点纬度	
	double dEndPosX;			//终点经度
	double dEndPosY;			//终点维度

	_sRoadNameInfo()
	{
		memset(chKaKouItem,0,128);
		memset(chPosNumber,0,128);
		memset(chRoadName,0,128);
		memset(chDirection,0,128);
		memset(chStartPos,0,128);
		memset(chEndPos,0,128);
		dStartPosX = 0.0;
		dStartPosY = 0.0;
		dEndPosX   = 0.0;
		dEndPosY   = 0.0;

	}
}sRoadNameInfo;
typedef list<sRoadNameInfo> RoadNameInfoList;
//匹配结果输出
typedef struct __MatchPair
{
	RECORD_PLATE_DSP_MATCH A;
	vector<RECORD_PLATE_DSP_MATCH> vecB;

	vector<int> vecDis;
}MatchPair;

//待匹配的信息
typedef struct _ObjMatchInfo
{
	RECORD_PLATE_DSP_MATCH plate;
	int nMatchTimes;
	bool bMatchSuccess;  //是否匹配成功
	int nNoMathTimes;    //记录未比较次数
	
	_ObjMatchInfo()
	{
		nMatchTimes = 0;
		bMatchSuccess = false;
		nNoMathTimes = 0;
	}
}ObjMatchInfo;

typedef struct _IPL_IMAGE_TAG
{
	IplImage *pImg;
	bool bUse;//图片是否使用

	_IPL_IMAGE_TAG()
	{
		pImg = NULL;
		bUse = false;
	}
}IPL_IMAGE_TAG;

typedef struct _JPG_IMAGE_TAG
{
	UINT64 uKey;//全局标记,记录唯一Key

	UINT32 uTime;//入队列时间戳(单位s)
	UINT32 uLastTime;//上一次更新记录状态时间
	//RECORD_PLATE plate;//车牌信息与图片对应
	char *pImg;	
	int nUseCount;//使用记数
	bool bUse;//记录是否已使用

	_JPG_IMAGE_TAG()
	{
		uKey = 0;

		uTime = 0;
		uLastTime = 0;
		pImg = NULL;
		nUseCount = 0;
		bUse = false;
	}
}JPG_IMAGE_TAG;

#ifndef mapLoopStatus
typedef std::map<int,int> mapLoopStatus;//线圈状态
#endif

#ifndef mapChanIdLoopStatus
typedef std::map<int,mapLoopStatus> mapChanIdLoopStatus;//通道线圈状态映射

#endif

#endif
