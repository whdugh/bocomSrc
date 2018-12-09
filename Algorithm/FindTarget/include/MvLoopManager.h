#ifndef MV_LOOP_MANAGER
#define MV_LOOP_MANAGER


#include <vector>
#include <map>
#include "LoopObj.h"
#include "MvLine.h"
#include "MvPhysicLoop.h"


struct _LoopInfo;
struct _LoopObj;
class MvPhysicLoop;


// 产生、管理LoopObj的类
class MvLoopManager
{
public:

	// 
	void Init(const MvFindTargetByVirtualLoop *pFtg, const std::vector<_LoopInfo> &li, int nDelay);

	// 
	void Destroy();

	// 在时间断tf到tt内检出的目标计数。
	int CountObjDuring(int nRoadIndex, int64 tf, int64 tt, float fOverlapRatio);


	// 接收线圈信号进行处理。
	void Process(int nRoadIndex, bool b0, bool b1, int64 counter, int64 ts);


	// 删除不必要的、老的LoopObj
	void Delete(int64 ts);


	// 将线圈画到图上。
	void DrawLoops(IplImage *img);


	// 获取线圈在图像及世界坐标系里的位置
	void GetLoopPos(int nRoadIndex, MvLine &l0, MvLine &l1);


	// 获取所有LoopObj
	const std::map<int, _LoopObj*>& GetLoopObjs() const;


	// 根据车道号得到对应车道的MvPhysicLoop对象
	MvPhysicLoop* GetPhysicLoopByIndex(int nIndex);


private:

	// 线圈目标记录。目标的id为索引。记录所有LoopObj对象
	std::map<int, _LoopObj*> m_mapLoopObjs;


	// 通道号<->线圈。记录所有MvPhysicLoop对象
	std::map<int, MvPhysicLoop*>   m_mapLoops;

};



#endif
