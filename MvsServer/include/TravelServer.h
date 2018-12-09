// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _TRAVELSERVER_H
#define _TRAVELSERVER_H

#include "CSocketBase.h"
#include "TravelConsdef.h"

/**
    文件：TravelServer.h
    功能：旅行时间通讯类
    作者：yuwenxian
    时间：2010-8-4
**/

typedef multimap<BYTE, string> TRA_MSG_MAP;
//通道数据列表
typedef std::list<std::string> TravelResultMsg;

class mvCTravelServer:public mvCSocketBase
{
    public:
        //构造
        mvCTravelServer();
        //析构
        virtual ~mvCTravelServer();

    public:
        //启动侦听服务
        bool Init();
        //释放
        bool UnInit();
        /*主线程调用接口*/
        void mvConnOrLinkTest();
        //ftp心跳处理线程
        void DealFTPLinkTest();
        /*处理一条消息*/
        bool mvOnDealOneMsg(int nSocket,const string &strMsg);
        /*重组并发送消息*/
        bool mvRebMsgAndSend(int& nSocket, BYTE uCmdID, const string &strMsg);
        //历史视频发送处理
        void mvDealHistoryVideo();

        //创建录像记录发送线程
        bool CreateVideoSendThread(string& strVideoPath);

        //事件录像发送处理
        void mvDealEventVideo();
        //发送设备类日志
        bool mvSendLogData();

    public:
        /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg);
        //接收中心端消息
        bool mvRecvCenterServerMsg(int nSocket);
        /*连接到中心端*/
        bool mvConnectToCS();
        //接收消息
        bool mvRecvMsg(int nSocket, string& strMsg);

        bool mvDealHistoryRecord();

        //记录点播
        bool mvOrdOneRecord(string strTime,string& strRecord);

        //获取连接套接字
        int mvGetSocket(){return m_nCenterSocket;}

        //处理数据
        void DealResult();
        //处理历史记录结果
        void DealHistoryResult();

        //添加一条数据,普通未检测图片
        bool AddResult(std::string& strMsg);

    private:
        /*发送心跳测试*/
        bool mvSendLinkTest();
        //发送业务控制参数回复
        bool mvSendOprationControlRep(bool &bFlag);
        //处理实时视频点播消息
        bool mvOnRealTimeVideo(int nSocket,const string &strMsg);
        //处理历史视频消息
        bool mvOnHistoryVideo(int nSocket,const string &strMsg);
        //发送认证请求
        void mvAuthentication();
        //响应认证请求
        bool mvOnAuthenticationRep(const string &strMsg);
        //实时过车文本数据回复
        bool mvOnRealTimeDataRep(const string &strMsg);
        //点播业务响应
        bool mvOnOrderOpration(int nSocket,const string &strMsg);
        //处理业务控制消息
        bool mvOnOprationControl(int nSocket,const string &strMsg);
        //检查设备状态
        bool mvOnDeviceStatus(int nSocket,const string &strMsg);
        //接收消息线程入口
        bool mvStartRecvThread();
        //处理检测结果
        bool OnResult(std::string& result);
        /////////////////////////////////
        //获取ftp登录信息
        bool GetFtpLoginInfo();
        //时钟同步
        bool SynClock();

        //通过ftp发送数据(socket方式)
        bool SendDataByFtp(string strLocalPath,string strRemotePath,string& strMsg,int nMode = 0);

     private:
        //侦听套接字
        int m_nAcceptSocket;
        //侦听端口
        int m_nPort;
        //TCP连接套接字
        int m_nCenterSocket;

        bool m_bCenterLink;
        pthread_t m_uRecvCSMsgThreadId;
        int m_nCSLinkCount;
        unsigned int m_uCSLinkTime;//心跳发送时间

        TRA_MSG_MAP m_mapCSMsg;
        pthread_mutex_t m_mutexMsg;

        //线程ID
        pthread_t m_nThreadId;
        //检测结果信号互斥
        pthread_mutex_t m_Result_Mutex;
        //检测结果消息列表
        TravelResultMsg	m_ChannelResultList;
        //历史线程ID
        pthread_t m_nHistoryThreadId;

        //int m_nCommunicationMode;//0：主通讯方式，1：备用通讯方式
        bool m_bAuthenticationOK;//是否认证成功

        TRAVEL_CONTROL_INFO m_travelControlInfo;

        //历史视频开始时间
        string m_strVideoBeginTime;
        //历史视频结束时间
        string m_strVideoEndTime;

        //事件录像名称
        string m_strEventVideoName;

        //记录ftp密码获取间隔
        int m_nGapTime;
        //是否可以ping通ftp
        bool m_bFtpLink;
};

extern mvCTravelServer g_TravelServer;

#endif // _TRAVELSERVER_H
