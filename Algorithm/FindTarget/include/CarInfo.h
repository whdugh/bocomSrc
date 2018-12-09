// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef CAR_INFO_H
#define CAR_INFO_H

#include <string>
#include "cxcore.h"

#include "ObjectColor.h"
#include "ObjTypeForOutput.h"

using namespace std;

#include <vector>

#ifndef CARNUMROW
#define CARNUMROW
enum CARNUM_ROW{ listrow=1, doublerow, other };
#endif

#ifndef SUB_VEHICLE_TYPE
#define SUB_VEHICLE_TYPE
//车辆类型细分；
enum SubVehicleTypeForOutput
{
	SVT_Default = 0,     //0    未计算
	SVT_Bus,             //1	大巴
	SVT_Truck,           //2	卡车
	SVT_MiniBus,       //3	中巴
	SVT_Car,             //4	轿车
	SVT_UnKnown,         //5    未知
	SVT_WrongPos,        //6	车牌位置太偏
	SVT_MiniVan         //7	小型货车
};
#endif

typedef struct _NovehiclInfo
{
	 CvRect   ObjRec;
	unsigned int     uSeq;                  //帧号
	_NovehiclInfo()
	{
		ObjRec = cvRect(0,0,1,1);
		uSeq = 0;

	}

 }NovehiclInfo;

typedef struct _CarInfo
{
    
	std::vector < _NovehiclInfo >  CharSerInf;
	int              id;                    //id，用于回写
	char             strCarNum[8];          //车牌号码

	char             wj[2];                 //武警车牌小字

	int64            ts;                    //时间戳(微秒！)
	int              ix;                    //车牌图像坐标
	int              iy;
	int              iwidth;                //车牌宽度(pixels)
	int              iheight;               //车牌高度
	unsigned int     uSeq;                  //帧号
	double           wx;                    //车牌世界坐标
	double           wy;
	double           vx;                    //车牌速度（世界坐标）m/s
	double           vy;
	double           ax;                    //加速度
	double           ay;
	int              updateTimes;           //更新次数

	unsigned int     uTimestamp;            //时间戳(秒)for yufeng
	int              color;                 //车牌颜色      for yufeng
	int              vehicle_type;          //车辆类型粗分！	   for yufeng   按照ObjTypeForOutput定义
											//小于5.5小，5.5到9.5中， 大于9.5大
	int              subVehicleType;        //用于车型细分，勿与vehicle_type混淆！
	float	         iscarnum;              //模板匹配重合度  //车牌置信度 for yufeng
	double           mean;                  //车牌亮度      for yufeng
	double           stddev;			    //车牌方差      for yufeng
	int              imgIndex;              //图像编号      for yufeng
	unsigned int     uSedImgFramseq;         //第二张图像的帧号 for yufeng
	object_color     objColor;	
	int              carnumrow;             //单排或者双排车牌

	CvRect           smear[20];              //
	int              smearCount;

	int              VerticalTheta;          //垂直倾斜角,以角度为单位
	int              HorizontalTheta;        //水平倾斜角,以角度为单位      
	int              RoadIndex;              //车道编号

	//非机动车位置信息
	CvRect          m_NovehiclRec;

	// TODO: 赋值。
	int              nDirection;             //运动方向，前牌（从远到近）为0，后面（从近到远）为1。-1未知。
	
#ifdef OBJ_CORRESPONDENCE
	unsigned short   features[1000];
	int              nFeatureSize;
#endif

	bool             bIsMotorCycle;          //是否为摩托车！
	//是否可以找到最亮图
	bool             m_UseShutter;
	//最亮图中是的车牌位置是否来自估计还是精确定位
	bool             m_EstPos;
	//车牌在最亮图中的位置
	CvRect           m_CarnumPos;
	//最亮图的帧号
	unsigned int     m_useq;
	//对与浦东卡口线圈触发的尾牌是否输出
	bool             m_BackCarnum;
	//最亮图的亮度值
	double           m_Mean;
	//车牌完整区域输出
	CvRect           m_CarWholeRec;

	//该车牌对应的抓拍线圈起始时刻和终止时刻。为监测线圈状态用
	int64 st;

	int64 et;

	int nNoCarNum;//0:表示有牌车(默认值);1:表示无牌车;2.其他（如非机和两轮车）  //线圈目标暂时都认为是有牌车！

	_CarInfo()
	{
		
		uSedImgFramseq = 0;
		id = -1;
	    uTimestamp = 0;
	    ix = 0;
	    iy = 0;
	    iwidth = 0;
	    iheight = 0;
        uSeq = 0;
		wx = 0;
		wy = 0;
		vx = 0;
		vy = 0;
		ax = 0;
		ay = 0;
		updateTimes = 0;
		m_CarWholeRec = cvRect(0,0,0,0);

		ts = 0;
		color = 0;
		vehicle_type = OTHER;
		subVehicleType = SVT_Default;
		iscarnum = -1;
		mean = -1.0f;
		stddev = -1;
		imgIndex = 0;
		strCarNum[7] = '\0';
		wj[0] = '\0';
		wj[1] = '\0';
		carnumrow = 0;

		smearCount = 0;
		VerticalTheta = 0;
		HorizontalTheta = 0;
		RoadIndex       = -1;
		nDirection      = -1;

#ifdef OBJ_CORRESPONDENCE
		nFeatureSize    = 0;
#endif
		bIsMotorCycle = false;

		m_UseShutter = false;    //是否找到亮图
		m_EstPos     = false;	//目标在亮图中的位置是估计的还是正确的
		m_CarnumPos = cvRect(0,0,0,0);//目标在亮图中的位置
		m_NovehiclRec = cvRect(0,0,0,0);
		m_useq = 0; //亮图的位置
		m_BackCarnum = false;
		m_Mean = 0.0;
		st = -1;
		et = -1;

		nNoCarNum = 0;
	}

	// 判断值是否一致
	bool IsDifferent(const _CarInfo &ci)
	{
		// 如果车牌不一样，不同
		if (0 != strncmp(ci.strCarNum, this->strCarNum, 7*sizeof(char)) ||
			(ci.strCarNum[0] == 'L' && 0 != strncmp(ci.wj, wj, 2*sizeof(char))))
		{
			return true;
		}

		if (((int)sqrt(ci.vx * ci.vx + ci.vy * ci.vy)) != (int)(sqrt(vx * vx + vy * vy)))
		{
			return true;
		}

		if (vehicle_type != ci.vehicle_type)
		{
			return true;
		}

		return false;
	}
	
} CarInfo; 

#endif
