#ifndef MV_UNION
#define MV_UNION

#include <vector>
#include "SensedObject.h"
#include "Car.h"
#include "MvFindTargetByVirtualLoop.h"
#include "MvLine.h"
#include <iomanip>
//#include "MvLoopManager.h"

struct _LoopObj;
class MvLoopManager;
class MvRadarSpeed;
class Mv_LoopShutterLight;
//class MvFindTargetByVirtualLoop;

//class Car;
//class SensedObject;

typedef struct _MvUnion
{
public:

	// union关联的car对象集合
	std::vector<Car*> cars;

	// 没有使用
	std::vector<SensedObject*> vlpObjs;

	// union关联的loopObj
	_LoopObj* loopObj;	


	// union的id
	int id;




	int64 tsOutput; // 最后一次输出时间。初始-1

	unsigned int uLastOutputSeq;//最后一次输出时帧号。

	int nOutputTimes;  //输出次数。

	// 标记结合体在上一次输出之后又有新数据。即发生了变化
	bool fresh;

	bool bDroppedForShutter;  //浦东卡口，若尾牌爆闪灯取不到亮图或者看不到车牌，则不输出！

	_MvUnion()
	{
		loopObj        = NULL;
		fresh          = true;
		tsOutput       = -1;
		uLastOutputSeq = 0;
		nOutputTimes   = 0;
		bDroppedForShutter = false;	

		_MvUnion::nIdGen += 1;
		id = _MvUnion::nIdGen;
		bUnionStat = false;
	}


	// 取得往应用输出的数据
	bool GetOutput(int nDirection, MvLoopManager *pLoopManager, MvFindTargetByVirtualLoop* pFtg, MvRadarSpeed *pRadar, FILE *pFile, Mv_LoopShutterLight *p_mLoopShutter,bool bFlagDsp,/*是否是dsp线圈爆闪*/ unsigned int uFrameSeq, Time_Mode mode, CarInfo &ci);


	void SetFresh()
	{
		fresh = true;
	}


	//设置已经union已经找到亮图
	void SetUnionStat( bool bStat);
	//返回是否已经对应亮图
	bool GetUnionStat();

	Car* GetAssocCar();

private:

	static int nIdGen;
	bool   bUnionStat;

}MvUnion;

#endif
