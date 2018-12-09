#ifndef MV_PHYSIC_LOOP_MANAGER
#define MV_PHYSIC_LOOP_MANAGER

#include <map>
#include <vector>
#include "MvLine.h"
#include "MvPhysicLoop.h"
#include "Calibration.h"

//using namespace std;
using namespace PRJ_NAMESPACE_NAME;



// 无论前牌尾牌，远的线圈为编号为0，近的线圈编号为1
typedef struct _LoopInfo
{
	int nRoadIndex;        // 车道编号
	MvLine line0;          // 编号为0的线圈在图像中的位置
	MvLine line1;          // 编号为1的线圈在图像中的位置。
	float  dis;            // 线圈距离（m）
}LoopInfo;



class MvPhysicLoopManager
{
public:

	MvPhysicLoopManager();

	~MvPhysicLoopManager();

	void SetLoopInfo(const std::vector<LoopInfo> &li, int nDelay);


	void Update(int nRoadIndex, bool b0, bool b1, int64 counter, int64 ts);

	// 计算2个时间，如果算不出则返回-1.
	// t1是车牌刚能检出的时间。t2是车牌最后一次能检出的时间。
	// lnUp, lnDn分别为车牌检测区域的上线和下线。
	// 时间单位都为us。
	bool ComputerPlateDetectTime(const VehicleLoopInfo &vli, MvLine lnUp, MvLine lnDn, int64 &t1, int64 &t2);


	// 计算车辆第一次压虚拟线圈的时间和最后一次压虚拟线圈的时间。
	// vlpLine 为vlp线在图像中的位置。
	bool ComputerPressVlpTime(const VehicleLoopInfo &vli, MvLine lnUp, int64 &t1, int64 &t2);


	MvLine GetLoopImagePos(int nRoadIndex, bool bNo0OrNo1);


	void* GetAssocObject(int nVliId) const;


	int   GetAssocType(int nVlidId) const;


	void  Delete();


	void  SetLoopWorldCoordinate(MyCalibration *pCalib);

	// 将线圈画到图上。
	void DrawLoops(IplImage *img);


public:

	
	// 车辆压线圈信息。
	std::map<int, VehicleLoopInfo> m_mapVehicles;

	//
	std::map<int, int>             m_mapStatus; //1：参与配对，>=2：等待删除

	// int：vli->id。 void*：与vli关联的对象。
	std::map<int, void*>           m_mapVliAssoc;

	// 关联类型。与车牌关联取1， 与目标关联取2，没有关联取0
	std::map<int, int>             m_mapAssocType;


	std::map<void*, int>           m_mapVliAssocV;

	

	// 在有速度状态下和车牌或目标匹配上，之后车牌或目标立即删除了。
	// 如果立即从删除，之后如检出长度则可能导致多报。所以在这种情况下
	// 暂时不从m_mapVehicles将信息删除，而在m_vecWaitDelete中做一个记录。
	// 在长度信息出现后，或者别的车辆压线圈信息出现后，将他们从m_mapVehicles
	// 中删除，因为这时他们不会再更新了。
	// vector<int>               m_vecWaitDelete;

private:

	// 通道号<->线圈。
	std::map<int, MvPhysicLoop*>   m_mapLoops;

	// 通道号
	std::map<int, LoopInfo>        m_mapLoopInfo;


	
};


#endif