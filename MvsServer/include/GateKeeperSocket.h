// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef GATEKEEPERSOCKET_H
#define GATEKEEPERSOCKET_H

#include "CSocketBase.h"
#include "GateKeeperConsdef.h"


#define MAX_GATE_ARRAY_SIZE 8
/**
    文件：GateKeeperSocket.h
    功能：远程监控看门设备通讯类
    作者：yuwenxian
    时间：2010-10-13
**/


class CGateKeeperSocket:public mvCSocketBase
{
    public:
        //构造
        CGateKeeperSocket();
        //析构
        virtual ~CGateKeeperSocket();

    public:
        //启动侦听服务
        bool Init();
        //释放
        bool UnInit();
        /*发送心跳测试*/
        bool mvSendLinkTest();
        /*处理一条消息*/
        bool mvOnDealOneMsg(const string &strMsg);

    public:
        //接收中心端消息
        bool mvRecvCenterServerMsg(int nSocket);
        //接收消息
        bool mvRecvMsg(int nSocket, string& strMsg);

        //设定连接套件字
        void mvSetCenterSocket(int nSocket) { m_nCenterSocket = nSocket; }

        //解析数据包
        //bool mvParseCmdString(const std::string &strMsg, BYTE &nCodeType,  BYTE &nCodeId, BYTE &nValueType, BYTE &nValue);
        bool mvParseCmdString(const std::string &strMsg, EXPO_MONITOR_INFO & ExpoMonitorInfo, bool bFlag);


        //获取环境监控状态信息
        bool mvGetGateKeeperInfo();

        //启动环境监控数据轮询线程
        bool mvStartGateKeeperAsk(int nSocket);

    private:
        //重组消息并发送
        bool mvRebMsgAndSend(int nSocket, UINT32 uCode, const string &strMsg);

        //设置请求和响应数据报号
        bool mvSetDataOrder(GATEKEEPER_HEADER &m_GateHeader);

        //设置响应数据报号
        void mvSetRpkn(int nRpkn) { m_nRpkn = nRpkn; }
        //设置请求数据报号
        void mvSetSpkn(int nSpkn) { m_nSpkn = nSpkn; }

     private:
        //侦听套接字
        int m_nAcceptSocket;
        //侦听端口
        int m_nPort;
        //TCP连接套接字
        int m_nCenterSocket;

        unsigned short m_nSpkn; //请求数据报号
        unsigned short m_nRpkn; //响应数据报号
};

extern CGateKeeperSocket g_GateKeeperServer;

#endif // GATEKEEPERSOCKET_H

