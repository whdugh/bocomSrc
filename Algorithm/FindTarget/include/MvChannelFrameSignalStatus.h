#ifndef MV_CHANNEL_FRAME_SIGNAL_STATUS
#define MV_CHANNEL_FRAME_SIGNAL_STATUS

#include <list>


/*
 一个通道在一帧的红灯状态。
*/
typedef struct _VTS_SIGNAL_STATUS
{
     int  nRoadIndex;	   //车道序号
	 unsigned int uFrameSeq;

     bool bLeftSignal;     //左转灯状态.红灯true。绿灯false。
     bool bStraightSignal; //直行灯状态
     bool bRightSignal;    //右转灯状态

     _VTS_SIGNAL_STATUS()
     {
		 uFrameSeq       = 0;
         nRoadIndex      = -1;
         bLeftSignal     = false;
         bStraightSignal = false;
         bRightSignal    = false;
     }

}VTS_SIGNAL_STATUS, MvChannelFrameSignalStatus;

//车道控制灯状态链表
typedef std::list<MvChannelFrameSignalStatus> ChnFrmSignalStatusList;
typedef std::list<MvChannelFrameSignalStatus>::iterator ChnFrmSignalStatusListIt;
typedef std::list<MvChannelFrameSignalStatus>::const_iterator ChnFrmSignalStatusListConIt;

#endif