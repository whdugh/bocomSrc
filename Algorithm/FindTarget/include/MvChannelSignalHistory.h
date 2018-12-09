#ifndef MV_CHANNEL_SIGNAL_HISTORY_H
#define MV_CHANNEL_SIGNAL_HISTORY_H


#include "MvChannelFrameSignalStatus.h"

/*
 记录一个车道各帧红灯状态的类。
*/
class MvChannelSignalHistory
{
public:

	// 构造函数。nKeepFrames表明红灯信号记忆的帧数。
	MvChannelSignalHistory(int nKeepFrames = 2500);

	// 析构函数
	~MvChannelSignalHistory();

	// 获取uFrameSeq帧车道的红灯状态。
	bool GetFrameSignalStatus(unsigned int uFrameSeq, MvChannelFrameSignalStatus &ret);

	// 记录新一帧红灯状态。
	void AddFrameSignalStatus(const MvChannelFrameSignalStatus &css);

private:
	MvChannelFrameSignalStatus *buffer;
	unsigned int m_uKeepFrames;

};



#endif
