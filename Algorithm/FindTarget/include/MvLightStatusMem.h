#ifndef MV_LIGHT_STATUS_H
#define MV_LIGHT_STATUS_H

#include <list>
#include <map>
#include "MvChannelSignalHistory.h"

//using namespace std;

/*
 记住每一帧，每个通道向前、向左、向右的红灯信号的一个类。
*/
class MvLightStatusMem
{
public:

	// lstChannelsIndex存储了系统中所有通道的索引号。
	MvLightStatusMem(int nMemFrames, const std::list<int> &lstChannelsIndex);

	// 析构函数
	~MvLightStatusMem();

	// 获取某帧某通道的红灯状态
	bool GetLightStatus(unsigned int uFrameSeq, int nChannelIndex, MvChannelFrameSignalStatus &ret);

	// 增加某帧各通道的红灯状态。
	void AddLightStatus(const std::list<MvChannelFrameSignalStatus> &ls);

	// 根据cfss里面信息标记的车道c、帧号f、方向d。假如f帧是c车道d方向为红灯，找出该红灯一次亮起的帧号。
	bool FindFirstRedLightFrameSeq(const MvChannelFrameSignalStatus &cfss, unsigned int &uRet);

	bool FindFirstRedLightFrameSeq(int nRoadIndex, unsigned int uFrameSeq, int nDir, unsigned int &uRet);

private:

	// 通道红灯信号记忆的帧数。
	// 超过这个帧数就会被删除。
	int m_nMemFrames;


	// 通道编号->通道红绿状态信息记录的映射表
 	std::map<int, MvChannelSignalHistory*> mapChnIndex2ChnHistory;

};


#endif
