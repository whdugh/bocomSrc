// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef CAR_H
#define CAR_H

#include <fstream>
#include <string>
#include <list>
#include "Calibration.h"
#include "CarInfo.h"
#include "MvChannelRegion.h"
#include "Mode.h"
#include "MvSurf.h"

using namespace PRJ_NAMESPACE_NAME;

class SensedObject;

struct _LoopObj;
struct _MvGroup;
struct _MvUnion;

class Mv_LoopShutterLight;
class MvFindTargetByVirtualLoop;

// 车辆类型的新规范。
#define VEHICLE_TYPE_WJS    0x00000000 // 未计算
#define VEHICLE_TYPE_UN     0x10000000 // 未知

#define VEHICLE_TYPE_LARGE  0x00000001 // 大
#define VEHICLE_TYPE_MIDDLE 0x00000002 // 中
#define VEHICLE_TYPE_SMALL  0x00000004 // 小
#define VEHICLE_TYPE_OTHER  0x00000008 // 其他

#define VEHICLE_TYPE_XR     0x00000010 // 行人
#define VEHICLE_TYPE_LL     0x00000020 // 两轮
#define VEHICLE_TYPE_SL     0x00000040 // 三轮
#define VEHICLE_TYPE_HC     0x00000080 // 货车
#define VEHICLE_TYPE_KC     0x00000100 // 客车

#define VEHICLE_TYPE_JZ     0x00010000 // 集装箱货车


typedef std::vector<std::pair<CvPoint, CvPoint> > vecCvPointPair;
typedef std::vector<std::pair<float, float> > vecFloatPair;
typedef std::vector<std::pair<CvPoint2D64f, CvPoint2D64f> > vecCvPoint2D64fPair;


class Car
{
public:
	SensedObject * AssocSensedObj;

	std::list<CarInfo> trackRecord;

    //存储目标标记
	std::vector<int> ObjHis;
	

	// 变成真车牌的帧号。
	unsigned int m_uGettingRealCarFrameSeq;

	//用于视频测速的两张缓冲图片帧号,如果都为-1表示该车的视频速度不是通过Surf匹配获得！
	unsigned int m_uLastFrm[2];

	int m_nHitBusNum;
	

public:

	Car(MvFindTargetByVirtualLoop *pFtg);

	MvFindTargetByVirtualLoop * m_pFtg;


	void SetVehicleType(int nVT)
	{
		m_nVehicleType = nVT;
	}

	int GetVehicleType() const
	{
		return m_nVehicleType;
	}

	//根据最亮图的帧号，去搜索是否已经在车牌的list中，相应的返回出位置

	bool Mvgetshuttercarnumpos(MvFindTargetByVirtualLoop* pFtg, unsigned int uSeq, Time_Mode mode, CarInfo &ci );

	bool Mv_Getmorelihgetbycarnum( int64 st, int64 et, Mv_LoopShutterLight *p_mLoopShutter, CarInfo &ci );

	bool mvModfiyCarInfo( CarInfo &ci );

	bool mvModfiyCarInfo_ForEle( CarInfo &ci, unsigned int uFreq );



	// 判断车牌是否可以输出。
	// 如果车牌像误检返回false，
	// 如果车牌可能还会检出,并且检出总数小于15，返回false。
	// 否则返回true
	bool CanOutput(unsigned int uFrameSeq) const;

	// 车载项目上的 
	bool CanOutputOnBus(unsigned int uFrameSeq) const;

	// 与Union进行关联
	void SetAssocUnion(_MvUnion *pUnion)
	{
		m_pAssocUnion = pUnion;
	}

	// 获取所关联的Union
	_MvUnion* GetAssocUnion() const
	{
		return m_pAssocUnion;
	}

	// 标记Car对象有没有进入Union列表。
	// 当CanOutput返回true时，Car对象就会进入列表，标明Car已经输出过了。
	void SetEnterUnionList()
	{
		m_bEnterUnionList = true;
	}

	// 返回是否进入过UnionList，防止错误的重入
	bool GetEnterUnionList() const
	{
		return m_bEnterUnionList;
	}

	// 清除进入过UnionList的记录，如果发现是误进入的，则调用该函数以允许本对象后续重新
	// 进入UnionList
	void ClearEnterUnionList()
	{
		m_bEnterUnionList = false;
	}

	// 判断车牌序列能不能根据AABB模式分成两个车牌。
	int GetSplitPosOtsu() const;

	// 将车牌序列截断，尾巴存在t里返回。
	void Split(int nPos, std::list<_CarInfo> &t);

	//bool GetPlatePosBySeq(unsigned int uSeq, CvRect &rect) const;

	
	// 获取对象的ID
	unsigned int  GetCarId() const;


	// be 大于0前牌，小于0尾牌，等于0不知道
	std::string        GetMostLikelyCarNum(int be, int count) const;
	
	// 过滤武警车牌小字。
	std::string        GetMostLikelyWjNo() const;
	
	bool GetPlatePosBySeq(unsigned int uSeq, CvRect &rect) const;

	// 根据车牌carnum判断它和当前实例是不是同一辆车.默认不使用相似字符。要求大于等于6位相同。
	bool          IsSameCar(std::string carnum, bool bUseSimilar = false, int nCount = 6) const;

	// 错位比较车牌相似性。可以防止车牌定位的时候定位偏掉造成的误检输出。
	bool          IsSmaeCarWithOffset(std::string carnum, int nOffset, bool bUseSimilar = false, int nCount = 6);

	// 根据位置判断是否为同一辆车的车牌
	bool          IsSameCarByPos(CvRect carnumPos, unsigned int frameSeq, int direction, MyCalibration *pCalib) const;

	bool          IsSameCarByPosOnBus(CvRect carnumPos, MyCalibration *pCalib);

	//某辆车在某段时间内是否出现过！
	bool          IsPresent(int64 startTime, int64 endTime);

	//void          SetNeedReport();

	// 设定已经输出，防止重新输出
	void          SetReported(unsigned int frameNo);

	bool          GetReported() const;

	// 返回是否需要输出，对于检出次数较少，并且置信度较低的认为是误检，返回false。
	bool          GetNeedReport() const;

	// 如果有关联的Group、Union不能删除，如果能近UnionList，但是还没进的也不删。
	bool          CanDelete(int64 ts) const;

	// 根据多次车牌检测的位置估计车牌运动方向。
	// 返回<0表示未知。0由远到近（前牌）。1由近到远（尾牌） 
	int           EstimateDirUsingPlateTrack() const;

	// 如果车牌在第n及n+k帧检出，则可以估出（n,n+k)区间内某帧车牌位置。
	bool          EstimatePlatePos(unsigned int uSeq, CvRect &ret) const;

	// 返回车牌运动方向矢量
	void          GetMotionDirection(float &dx, float &dy) const;

	// 用新的车牌位置更新本对象相关数据
	void          UpdateCar(CarInfo cn, FindTargetElePolice::MyCalibration *pCali);

	//根据Surf匹配得到的点对，考虑高度因素，应用中值滤波的方法，修正车子视频速度！
	float		  CorrectVideoSpdBySurf(IpPairVec vecPair, int64 *lastTimeStamp, bool bUseRemoveDisturbing, vecCvPointPair &img_coor, vecFloatPair &vecDeltaY, vecCvPointPair &corrected_img_coor, vecCvPoint2D64fPair &world_coor, std::vector<float> &corrected_spd);

	// 获取Car对象中车牌检出次数
	int           GetTrackCount() const;

	// 找置信度最好的一个值。（越小越好）
	float         GetBestIsCarNum() const;
	
	// 根据车牌号码记录的数量，以及相似度和置信度判断当前实例是不是真的是车牌，而不是地面之类的其他东西。
	bool          IsRealCar() const;

	// 获取最后加入的车牌信息CarInfo
	CarInfo       GetLastTrack() const;

	// 获取输出信息。
	bool          GetOutput(int direction, unsigned int uFrameSeq, unsigned int uMaxSeq, _LoopObj *pLoopObj, CarInfo &ret);

	// 设置关联的OBJ，卡口用。
	void          SetAssocSensedObject(SensedObject *pSensedObj);


	SensedObject* GetAssocSensedObj() const;
	
	
	void          ClearAssocSensedObj();


	// 设置关联Group。
	void          SetAssocGroup(_MvGroup *pGroup);


	// 读取关联Groupd。
	_MvGroup*     GetAssocGroup() const;


	// 通知sensedobj车牌对象将删除
	void          NotifyDelete() const;

	

	// 估计车牌在timestamp时刻在图像中的位置   
	//CvPoint       EstimatePosition(int64 timestamp, FindTargetElePolice::MyCalibration *cal) const;

	// 获取车牌最后一次的位置。图像坐标。
	CvRect        GetLastPosition() const;


	CvRect        GetFirstPosition() const;

	// 获取第一次出现的时间ts。
	int64		  GetFirstTime() const;

	// 获取最后一次更新的时间ts。
	int64         GetLastUpdateTime() const;

	// 获取第一次的帧号！
	unsigned int GetFirstFrameSeq() const;

	// 获取最后一次更新时的帧号。
	unsigned int  GetLastUpdateFrameSeq() const;

	//获取输出过滤时关联得到的爆闪灯帧号！
	unsigned int  GetShutterFrame() const
	{
		return m_uShutterFrame;
	}
	//设置输出过滤时关联得到的爆闪灯帧号，无则为初始值0！
	void          SetShutterFrame(unsigned int uFrame)
	{
		m_uShutterFrame = uFrame;
	}

	// 获取该车牌输出的帧号。
	//unsigned int  GetOutputFrameNo() const;


	void          WriteCarToFile(std::ofstream &fs) const;

	// 
	void          MergeCar(Car* pCar, int nDirection);


	//void          IncRefCount();


	//void          DecRefCount();


	float         GetSpeed() const;

	float         GetCorrectedSpd() const
	{
		return    sqrtf(m_fSpdX_Correction*m_fSpdX_Correction + m_fSpdY_Correction*m_fSpdY_Correction);
	}

	// 判断车牌属于哪个车道。如果返回-1，表示不属于任何车道。
	int           GetRoadIndex(const std::vector<ChannelRegion> &rd) const;

	// 判断在from到to时间内，车牌有没有检出。
	bool          IsDetectedInTimePeriod(int64 from, int64 to) const;


	//判断是否摩托车！
	bool          IsMotorCycle() const;

	// 根据车牌颜色判断是否为大车。黄色非学为大车。
	bool          IsBigVehicle() const;


	// 根据车牌颜色判断是否为大车
	static bool    IsBigVehicle(const CarInfo &ci);
	
	//判断是否为真的大车，可能第一次被误检成蓝牌了！
	bool		   GetRealBigVehicle() const
	{
		return m_bRealBigVehicle;
	}
	void		   SetRealBigVehicle()
	{
		m_bRealBigVehicle = true;
	}
	int			   GetCheckBigVehicleNum() const
	{
		return m_nCheckBigVehicleNum;
	}
	void           UpdateCheckBigVehicleNum()
	{
		m_nCheckBigVehicleNum++;
	}
	//static bool    IsNotYellowPlate(const CarInfo &ci);

	CvRect           GetCarVlpRec()const
	{
		 return m_CarVLpRec;
	}

	void   SetCarVlpRec(CvRect rect)
	{
		m_CarVLpRec = rect;

	}

	void SetCarAwayStat()
	{
       m_CarDirveAway = true;
	}

	bool GetCarWayStat()
	{
		return m_CarDirveAway;
	}

	void SetCarInStat()
	{
		m_CarDirveAway = false;
	}

	void SetCarCutStat()
	{
		m_CarCut = true;
	}

	bool GetCarCutStat()
	{
		return m_CarCut;
	}

	// 计算两个车牌7位中相似/同的位数    
	int static    CalcCarNumDis(const std::string &num1, const std::string &num2, bool useSimilarChar = false);

private:
	bool    m_CarDirveAway;


	bool   m_CarCut;

	CvRect  m_CarVLpRec;

	// 过滤车牌第一位（汉字）。
	char          GetMostLikelyChnCharacter(int be, int count) const;


	// 过滤车牌后六位。
	std::string   GetMostLikelyCharacter(int be, int count) const;

	// 是否已经输出。
	bool          m_bReported;

	// 车牌输出帧号
	unsigned int  m_uOutputFrameNo;


	// 与这辆车相关联的group的编号。每个group有一个编号。电子警察用。
	_MvGroup*     m_pAssocGroup;

	// 每个car对象有一个唯一的编号
	unsigned int  m_uCarId;


	// 车牌检出时判断车辆类型。暂时只用于电警货车禁行
	int           m_nVehicleType;


	static unsigned int uCarIdGenerater;



	// 速度，km/h。每次检出时更新。
	float     m_fSpdX;
	float     m_fSpdY;
	// 考虑高度后的校正视频速度。
	float     m_fSpdX_Correction;
	float     m_fSpdY_Correction;

	int       m_nImageHeight;
	FindTargetElePolice::MyCalibration *m_pCalib;
	//判断是否是真的黄牌车及判断次数，前三次只要有一次即可，用于尾牌车型！
	bool      m_bRealBigVehicle;
	int       m_nCheckBigVehicleNum;

public:
	bool    m_Bus;
	unsigned int uShutBriFrm;
	int64     m_SerStr;
	int64     m_SerEnd;	
	bool     m_bStoreCar;
	bool     m_bViolation;//状态很重要，在编码的时候如果关联违章了，就由违章来删除
	
	
private:
	_MvUnion* m_pAssocUnion;

	bool      m_bEnterUnionList;

	int       m_nCarPlateHeightOffGround;

	unsigned int m_uShutterFrame;  //输出时过滤线圈对象时设置的爆闪灯帧号！
private:
	char     static GetMostLikelyChar(int index, const std::list<CarInfo> &lst);
	char     static GetMostLikelyChar(int index, const std::list<std::string> &lst);
	bool     static IsSimilar(const char &c1, const char &c2);

	// 计算从nStart开始的共nLen个字符中有几个一样的（相似）。
	int      static GetSameCount(char* str1, char* str2, int nStart, int nLen, bool bUseSimilar);


	
};

#endif
