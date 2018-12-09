// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef AMSCOMMUNICATION_H
#define AMSCOMMUNICATION_H

#include "CSocketBase.h"
#include "AMSConstdef.h"
#include "FtpCommunication.h"
#include "XmlParaUtil.h"


/**
    文件：AMSCommunication.h
    功能：应用管理服务器通讯类
    作者：於锋
    时间：2010-8-4
**/

typedef multimap<UINT32, string> CS_MSG_MAP;
typedef std::list<std::string> AMS_Result;
typedef map<UINT32, VOD_FILE_INFO> VOD_FILE_MAP;

class CAMSCommunication:public mvCSocketBase
{
    public:
        //构造
        CAMSCommunication();
        //析构
        virtual ~CAMSCommunication();
    public:
        /*主线程调用接口*/
        void mvConnOrLinkTest(bool isInit = false);

        //获取历史视频文件名称
        bool GetRemoteFile(UINT32 uCameraID,VOD_FILE_INFO& strRemoteFile);

        //设置历史视频文件名称
        void SetRemoteFile(UINT32 uCameraID,VOD_FILE_INFO strRemoteFile);

        //启动服务
        bool Init();

        //释放
        bool UnInit();

    public:
        /*压入一条消息*/
        void mvPushOneMsg(string strMsg);
        /*弹出一条消息*/
        bool mvPopOneMsg(UINT32& uCmdID, string &strMsg);
    public:
        /*处理一条消息*/
        bool mvOnDealOneMsg(UINT32 uCmdID, const string &strMsg);
        /*重组并发送消息*/
        bool mvRebMsgAndSend(int& nSocket,UINT32 uCameraID,UINT32 uCmdID, const string &strMsg, UINT32 uFlag=0);

        //添加一条数据
        bool AddResult(std::string& strResult);

        //处理检测结果
        bool OnResult(std::string& result);

        //处理实时数据
        void DealResult();

        //处理历史数据
        void DealHistoryResult();
    public:
        /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg);
        //接收中心端消息
        bool mvRecvCenterServerMsg();
        /*发送通道情况报告*/
        bool mvSendChannelInfo(CHANNEL_INFO_RECORD& chan_info);
        //发送通道情况
        void mvSendChannelInfo();
        //发送检测参数到AMS服务器
        bool SendSettingsToCS(int nCameraID,int nPreSet,int nType);
        //从ftp服务器获取检测参数
        void GetSettingsFromCS(int nCameraID,int nPreSet);
        /*历史视频分析完成报告*/
        bool mvSendVodFinish(UINT32 uCameraID,const string strFilePath);
        /*历史视频分析请求回复*/
        bool mvVodRequestRep(const string &strMsg);
        /*历史视频分析请求*/
        bool mvSendVodRequest(UINT32 uCameraID);
		//从AMS控制服务器取得检测配置文件的请求
		bool mvSendLoadSettingReq();

	#ifdef MVSBAK
		//组包: MVS通道列表xml
		bool mvPachChannelListToXml(string &strMsg);
		//拆包: MVS通道列表xml
		bool mvDePachXmlToChannelList(string &strMsg, CHANNEL_INFO_LIST &chan_info_list);
		//发送MVS通道列表到AMS服务器
		bool mvSendChannelListXml();
		//AMS通知备份MVS接管DSP
		bool mvStartBakDsp(CHANNEL_INFO_LIST &chan_info_list);
		//AMS通知备份MVS停止接管DSP
		bool mvStopBakDsp(CHANNEL_INFO_LIST &chan_info_list);
	#endif

    private:
        /*连接中心端并开始接收消息*/
        bool mvConnCSAndRecvMsg();
        /*连接到中心端*/
        bool mvConnectToCS();
        /*启动接收消息线程*/
        bool mvStartRecvThread();
        //接收消息
        bool mvRecvMsg(int nSocket, string& strMsg);
        //连接ftp服务器
        bool ConnectFtpServer();
        //通过ftp发送数据
        bool SendDataByFtp(const string& strMsg,string& strRemotePath,int nType = 0);

    private:
        /*发送心跳测试*/
        bool mvSendLinkTest();
        /*发送历史视频分析时间回复*/
        bool mvSendVideoTimeRep(const string &strMsg);
        //获取一条车牌或事件历史记录
		 bool mvGetPlateAndEventHistoryRecord(string &strMsg, short sType);
     private:
        int m_nCenterSocket;
        int m_nCSLinkCount;

        bool m_bCenterLink;
        pthread_t m_uRecvCSMsgThreadId;

        pthread_t m_nThreadId;
        pthread_t m_nHistoryThreadId;

    private:
        //消息列表
        CS_MSG_MAP m_mapCSMsg;
        pthread_mutex_t m_mutexMsg;

        AMS_Result m_ResultList;
        pthread_mutex_t m_Result_Mutex;

        //文件列表
        VOD_FILE_MAP m_mapVodFile;
        pthread_mutex_t m_mutexVodFile;
};
extern CAMSCommunication g_AMSCommunication;
#endif
