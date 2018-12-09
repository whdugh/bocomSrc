// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef VISNETCOMMUNICATION_H
#define VISNETCOMMUNICATION_H

#include "CSocketBase.h"

/**
    文件：VisNetCommunication.h
    功能：VIS 网口通讯类
    作者：於锋
    时间：2010-9-4
**/

class CVisNetCommunication:public mvCSocketBase
{
    public:
        //构造
        CVisNetCommunication();
        //析构
        virtual ~CVisNetCommunication();
    public:
        /*主线程调用接口*/
        void mvConnOrLinkTest();
    public:
        /*压入一条消息*/
        void mvPushOneMsg(string strMsg);
        /*弹出一条消息*/
        bool mvPopOneMsg(string &strMsg);
    public:
        /*处理一条消息*/
        bool mvOnDealOneMsg(const string &strMsg);
        /*重组并发送消息*/
        bool mvRebMsgAndSend(int& nSocket,CAMERA_CONFIG& cfg, int nMonitorID);
    public:
        /*发送记录到中心端*/
        bool mvSendMsgToVIS(CAMERA_CONFIG& cfg, int nMonitorID);
        //发送命令到vis
        bool WriteCmdToVis(std::string& sCmdMsg);
        //接收中心端消息
        bool mvRecvCenterServerMsg();

    private:
        /*连接中心端并开始接收消息*/
        bool mvConnCSAndRecvMsg();
        /*连接到中心端*/
        bool mvConnectToCS();
        /*启动接收消息线程*/
        bool mvStartRecvThread();
        //接收消息
        bool mvRecvMsg(int nSocket, string& strMsg);

        //获取控制字符串码
        std::string GetControlCode(CAMERA_CONFIG& cfg,int nMonitorID);

	public:
		//命令发送时间
		UINT32 m_uSendCmdTime;

     private:
        int m_nCenterSocket;
        int m_nCSLinkCount;

        bool m_bCenterLink;
        pthread_t m_uRecvCSMsgThreadId;

    private:

        pthread_mutex_t m_mutexMsg;
};
extern CVisNetCommunication g_VisNetCommunication;
#endif

