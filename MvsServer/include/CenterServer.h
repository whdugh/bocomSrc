/*
 * File:   CenterServer.h
 * Author: zhangyaoyao
 *
 * Created on 2010年2月23日, 上午8:59
 */

#ifndef _CENTERSERVER_H
#define _CENTERSERVER_H

#include "CSocketBase.h"

//#define CS_FILE_LOG

extern bool g_bCenterLink;
extern string g_strCSMsgVer;
typedef multimap<string, string> CS_MSG_QUEUE;

//通道数据列表
typedef std::list<std::string> CSResultMsg;
//线圈状态
typedef std::map<int,int> mapLoopStatus;


class mvCSocketBase;

class mvCCenterServer:public mvCSocketBase
{
    public:
        /*构造函数*/
        mvCCenterServer();
        /*析构函数*/
        ~mvCCenterServer();

    public:
        //初始化
        bool Init();
        //释放
        bool UnInit();
        /*主线程调用接口*/
        void mvConnOrLinkTest();

        //添加一条数据,普通未检测图片
        bool AddResult(std::string& strMsg);

        //处理数据
        void DealResult();

    public:
        /*压入一条消息*/
        void mvPushOneMsg(string strMsg);
        /*弹出一条消息*/
        bool mvPopOneMsg(string &strCode, string &strMsg);
        /*设置检测器ID*/
        void mvSetDetectorId(const char *pDetectorId);

    public:
        /*处理一条消息*/
        bool mvOnDealOneMsg(const char *pCode, const string &strMsg);
        /*重组并发送消息*/
        bool mvRebMsgAndSend(int& nSocket,const char *pCode, const string &strMsg);

    public:
        /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg, bool bRealTime = true);
        /*结束工作*/
        bool mvOnEndWork();
        /*检查是否空闲，空闲则处理历史记录*/
        void mvCheckFreeAndDealHistoryRecord();
        //接收中心端消息
        bool mvRecvCenterServerMsg();

        /*接收消息*/
        bool mvRecvSocketMsg(int nSocket, string& strMsg);

		//设置线圈状态
		void SetLoopStatus(mapLoopStatus& vLoopStatus);

		/*检查设备状态*/
        void mvCheckDeviceStatus(int nDeviceStatusKind);

		//获取线圈状态
		mapLoopStatus GetLoopStatus();

    private:
        /*请求并连接中心端*/
        bool mvRequestAndConnCS();
        /*连接中心端并开始接收消息*/
        bool mvConnCSAndRecvMsg();
        /*连接到中心端*/
        bool mvConnectToCS();
        /*启动接收消息线程*/
        bool mvStartRecvThread();

        /*取出一条历史记录*/
        bool mvGetOneHistoryRecord(string &strMsg);

        //处理检测结果
        bool OnResult(std::string& result);

    private:
        /*发送路由请求并等待回复*/
        bool mvSendRouterReqAndRecv();
        /*发送路由请求*/
        bool mvSendConnReqToRouter();
        /*接收路由回复*/
        bool mvRecvIpPortFromRouter();

    private:
        /*开始工作*/
        bool mvOnDetectRestart(); //(const string &strMsg);
        /*标记未发送，强制重传*/
        bool mvOnMarkUnsend(const string &strMsg);
        /*记录查询*/
        bool mvOnRecordQuery(const string &strMsg);
        /*获取设备状态*/
        bool mvOnDeviceStatus(const string &strMsg);
        /*收到结束工作回复*/
        bool mvOnEndWorkRep(); //(const string &strMsg);
        /*实时记录回复*/
        bool mvOnRealTimeRecordRep(); //(const string &strMsg);
        /*历史记录回复*/
        bool mvOnHistoryRecordRep(); //(const string &strMsg);
        /*时钟设置*/
        bool mvOnSysTimeSetup(const string &strMsg);

    private:
        /*发送心跳测试*/
        bool mvSendLinkTest();
        /*发送重启应答*/
        bool mvSendRestartRep();
        /*发送标记未发送应答*/
        bool mvSendMarkUnsendRep(const string &strMsg);
        /*发送记录查询应答*/
        bool mvSendRecordQueryRep(const string &strMsg);
        /*发送设备状态查询应答*/
        bool mvSendDeviceStatusRep(const string &strMsg);
        /*发送设备报警*/
        bool mvSendDeviceAlarm(const string &strMsg);
        /*发送结束工作报告*/
        bool mvSendEndWork();

    private:
        /*CRC编码*/
        void mvCRCEncode(const string &strSrc, string &strRes);
        /*获取日期*/
        void mvGetDateTimeFromSec(const long &uSec, string &strDate, int nType = 0);


    public:
#ifdef CS_FILE_LOG
        FILE *m_pCSRecvLog;
        FILE *m_pCSSendLog;
        FILE *m_pCSConnLog;
#endif

    private:
        int m_nControlSocket;
        int m_nCenterSocket;
        int m_nCSLinkCount;
        int m_nConnectTime;//统计连接次数
        int m_nAllDisk;
        bool m_bHistoryRep;
        string m_strHeader;
        string m_strEnd;
        string m_strDetectorId; //including ' ' as valid character.
        unsigned int m_uHistoryRepTime; //历史记录回复时刻
        unsigned int m_uCSLinkTime;//心跳发送时间

    private:
        CS_MSG_QUEUE m_mapCSMsg;
        pthread_mutex_t m_mutexMsg;

        //线程ID
        pthread_t m_nThreadId;
        //检测结果信号互斥
        pthread_mutex_t m_Result_Mutex;
        //检测结果消息列表
        CSResultMsg	m_ChannelResultList;

		//线圈状态
		mapLoopStatus m_vLoopStatus;
};

extern mvCCenterServer g_CenterServer;

#endif // _CENTERSERVER_H
