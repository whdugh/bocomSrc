// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#ifndef VISKEYBOARDCONTROL_H
#define VISKEYBOARDCONTROL_H

#include "global.h"
/**
    文件：VisKeyBoardControl.h
    功能：Vis虚拟键盘控制类
    作者：於锋
    时间：2010-9-14
**/

typedef std::list<std::string> SerialCmdMsg;

typedef struct _CmdMsgAndTime //设备命令和等待时间
{
    int nIndex; //设备编号
    std::string cmdMsg; //命令
    int nTimesWait; //等待时间

    _CmdMsgAndTime()
    {
        nIndex = 0;
        cmdMsg = "";
        nTimesWait = 0;
    }
}CmdMsgAndTime;


typedef std::list<CmdMsgAndTime> CmdMsgAndTimeList; //存放某个相机对应的 命令和等待时间 列表
//typedef std::list<CmdMsgAndTimeList> AllCmdMsgList; //存放所有相机对 的 命令列表和等待时间列表
//typedef std::map<int, AllCmdMsgList::iterator> AllCmdMsgMap; //设备->消息列表和等待时间列表映射

typedef std::map<int, CmdMsgAndTimeList> CmdMsgMap; //设备->消息列表映射

typedef std::map<int, struct timeval> TimeMap; //设备->前一条命令发送时间映射

typedef std::map<int, int> SpaceTimeMap; //两条命令之间最少要间隔的时间映射（单位毫秒）--设备->间隔时间
typedef std::map<int, int> SleepTimeMap; //按下，抬起命令时间间隔（单位毫秒）--设备->时间间隔

class CVisKeyBoardControl
{
public:
        CVisKeyBoardControl();
        ~CVisKeyBoardControl();

        //发送镜头控制命令(pelco protocol)
        bool SendKeyData(CAMERA_CONFIG& cfg, int nMonitorID, int nCount,bool bTimeWait = true);

        //设置等待时间
        void SetSleepTimes(int nIndex, int nTimes);
        //获取等待时间
        int GetSleepTimes(int nIndex);

        //设置按格数控制，两条命令之间最少要间隔的时间-单位毫秒
        void SetSpaceTimes(int nIndex, int nTimes);
        //获取按格数控制，两条命令之间最少要间隔的时间-单位毫秒
        int GetSpaceTimes(int nIndex);

        //压入串口控制命令
        void AddCmdMsg(std::string& sCmdMsg);
        //取出串口控制命令
        int PopCmdMsg(std::string& sCmdMsg);

        //处理串口控制命令
        void DealCmdMsg();
        //开启串口控制命令发送线程
        void BeginThread();

        //设置是否等待时间
        void SetIsWaitTime(bool bWait) { m_bTimeWait = bWait; }

        //设置是否球机控制
        void SetBallControl(bool bBall) { m_bBallCamera = bBall; }
        //获取是否球机控制
        bool GetBallControl() { return m_bBallCamera; }


        //压入串口控制命令和需要间隔时间
        void AddCmdMsgAndWait(std::string& sCmdMsg, int nTimesWait, int nIndex);
        //取出串口控制命令 和 需要间隔时间
        int PopCmdMsgAndWait(CmdMsgAndTime& cmdMsg);

        //取得所有设备待发命令列表中找出最先发送命令的 设备编号，设备命令和等待时间
        bool GetCmdMsgAndWait(CmdMsgAndTime& cmdMsg);

        //处理串口控制命令和需要间隔时间
        void DealCmdMsgAndWait();

        //获取设备Id,对应前一条命令发出时间
        bool GetTimesPre(struct timeval* pTimePretv, int nIndex);

        //获取指定设备编号，上一条命令与现在时间差-单位毫秒
        int GetTimesWaitById(int nIndex);

        //获取设备Id,对应前一条命令发出时间
        bool GetTimesPreById(struct timeval* pTimePretv, int nIndex);

        //删除消息链表中发出去的消息
        bool DelCmdMsgById(int nIndex);

        //记录设备nIndex,发送记录时的瞬时时刻
        bool SetTimeMap(int nIndex);

        //添加新设备控制消息列表
        bool AddDev(int nIndex);

private:

    SpaceTimeMap m_SpaceTimesMap; //两条命令之间最少要间隔的时间映射（单位毫秒）--设备->间隔时间
    SleepTimeMap m_SleepTimesMap; //按下，抬起命令时间间隔（单位毫秒）--设备->时间间隔

    SerialCmdMsg m_cmdMsgList;
    pthread_mutex_t m_mutexMsg;
    bool m_bTimeWait; //是否发码完等待一定时间

    CmdMsgAndTimeList m_cmdMsgSendList; //发送消息列表

    bool m_bBallCamera; //是否球机控制
    struct timeval m_tvTimesPre; //前一条命令发出时间

    CmdMsgMap m_CmdMsgMap; //相机->消息列表映射
    TimeMap m_timeMapPre; //设备->前一条命令发送时间映射
};

extern CVisKeyBoardControl g_VisKeyBoardControl;;

#endif //
