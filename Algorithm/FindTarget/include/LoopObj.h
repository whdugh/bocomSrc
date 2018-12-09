#ifndef LOOP_OBJ
#define LOOP_OBJ

#include "cxcore.h"
#include "MvUnion.h"
#include "MvLine.h"
#include "CarInfo.h"
#include "MvFindTargetByVirtualLoop.h"


//struct _MvUnion;
//class MvFindTargetByVirtualLoop;

// 线圈检出的目标
typedef struct _LoopObj
{
public:
	int64 st;          // 压上触发线圈的时间，us
	int64 et;          // 离开触发线圈的时间，us
	float spd;         // km/h 小于0表示未知。速度方向有dir标明。
	int   dir;         // 大于0表示从远到近，小于0表示从近到远。等于0表示不知道。
	int   id;          // 编号
	int   nRoadIndex;  // 道路编号。

	bool  loop0_triggered;  //触发了线圈0！
	bool  loop1_triggered;  //触发了线圈1！
	bool  haveVeto; //是否已经有一次碰线圈0不算！

public:

	_LoopObj(int nRoadIndex);
	

	// 设置关联的Union
	void SetAssocUnion(_MvUnion *pUnion)
	{
		AssocUnion = pUnion;
	}

	// 获取关联的Union
	_MvUnion* GetAssocUnion() const
	{
		return AssocUnion;
	}

	// 设置刚开始压到触发线圈的时间
	void SetEnterTime(int64 st)
	{
		if (this->st != st)
		{
			this->st = st;
			OnDataChanged();
		}
	}

	// 设置离开触发线圈的时间
	void SetLeaveTime(int64 et)
	{
		if (this->et != et)
		{
			this->et = et;
			OnDataChanged();
		}
	}

	// 设置速度
	void SetSpeed(float fSpd)
	{
		if (this->spd != fSpd)
		{
			this->spd = fSpd;
			OnDataChanged();
		}
	}

	// 方向
	void SetDir(int nDir)
	{
		if (this->dir != nDir)
		{
			this->dir = nDir;
			OnDataChanged();
		}
	}


	// 标记已经进入了Union，防止重入导致多报。
	void SetEnterUnionList()
	{
		bEnterUnionList = true;
	}

	// 获取是否进入过Union
	bool GetEnterUnionList() const
	{
		return bEnterUnionList;
	}

	// 判断车牌能不能检出
	//bool IsPlateCanBeDetected(int64 ts, MvLine l0, MvLine l1, CvRect roiCarNum);

	// 计算车牌
	bool GetPlateDetectdTimePeriod(MvLine l0, MvLine l1, MvLine lnUp, MvLine lnDn, /*CvRect roiCarNum, */int64 &tf, int64 &tt);

	// fLen 车长（m）
	bool GetVehicleLength(float &fLen);


	// 判断是否可以删除
	bool CanDelete() const;


	// 获取输出信息。存储与ci里面
	void GetOutput(MvLine l0, MvLine l1, MvFindTargetByVirtualLoop *pFtg, CarInfo &ci) const;


	// 获取本对象所关联的Union关联的car对象
	std::vector<Car*> GetAssocCars() const;


private:

	_MvUnion *AssocUnion;// 关联的结合体


	// 是否进入过union流
	bool bEnterUnionList;

	void OnDataChanged();



	
private:

	static int nIdGen;





} LoopObj;




#endif
