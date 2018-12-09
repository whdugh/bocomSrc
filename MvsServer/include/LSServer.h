// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _LSSERVER_H
#define _LSSERVER_H

#include "CSocketBase.h"
#include "LSConsdef.h"
/**
    文件：LSServer.h
    功能：丽水通讯类
    作者：yufeng
    时间：2010-8-31
**/
typedef std::list<std::string> LSResultMsg;

class mvCLSServer:public mvCSocketBase
{
    public:
        //构造
        mvCLSServer();
        //析构
        virtual ~mvCLSServer();

    public:
        //启动侦听服务
        bool Init();
        //释放
        bool UnInit();
        /*发送心跳测试*/
        bool mvSendLinkTest();
        /*处理一条消息*/
        bool mvOnDealOneMsg(const string &strMsg);

         //添加一条数据
        bool AddResult(std::string& strResult);

        //处理检测结果
        bool OnResult(std::string& result);

        //处理实时数据
        void DealResult();

         //获取一条历史记录
        void mvDealHistoryRecord();

		void mvTestRecord();

    public:
        /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg);
        //接收中心端消息
        bool mvRecvCenterServerMsg(int nSocket);
        //接收消息
        bool mvRecvMsg(int nSocket, string& strMsg);

    private:
        //重组消息并发送
        bool mvRebMsgAndSend(UINT32 uCode, const string &strMsg);

     private:
        //侦听套接字
        int m_nAcceptSocket;
        //侦听端口
        int m_nPort;
        //TCP连接套接字
        int m_nCenterSocket;

        //线程ID
        pthread_t m_nThreadId;
        //历史线程ID
        pthread_t m_nHistoryThreadId;
        //检测结果信号互斥
        pthread_mutex_t m_Result_Mutex;
        //检测结果消息列表
        LSResultMsg m_ResultList;
		//认证状态
		bool m_bAuthStatus;

};

extern mvCLSServer g_LSServer;

#endif // _TRAVELSERVER_H

