// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef MATCHCOMMUNICATION_H
#define MATCHCOMMUNICATION_H

#include "CSocketBase.h"
#include "MatchConstdef.h"
#include "CvxText.h"

/**
    文件：MatchCommunication.h
    功能：比对服务器通讯类
    作者：於锋
    时间：2011-5-26
**/

typedef std::list<std::string> Match_Result;

class CMatchCommunication:public mvCSocketBase
{
    public:
        //构造
        CMatchCommunication();
        //析构
        virtual ~CMatchCommunication();
    public:
        /*主线程调用接口*/
        void mvConnOrLinkTest();

        //启动服务
        bool Init();

        //释放
        bool UnInit();
    public:

        /*重组并发送消息*/
        bool mvRebMsgAndSend(int& nSocket, const string &strMsg);

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

		//输出特征特征标定信息
		void OutPutCalibration(CvRect& farrect,CvRect& nearrect,SRIP_DETECT_HEADER* sDetectHeader);

    private:
        /*连接中心端并开始接收消息*/
        bool mvConnCSAndRecvMsg();
        /*连接到中心端*/
        bool mvConnectToCS();

    private:
        /*发送心跳测试*/
        bool mvSendLinkTest();

        //生成特征数据
        void CreateXmlMsg(string& strXmlResult,const string& strMsg);

     private:
        int m_nCenterSocket;
        int m_nCSLinkCount;

        bool m_bCenterLink;

        pthread_t m_nThreadId;
        pthread_t m_nHistoryThreadId;

    private:
        //消息列表

        Match_Result m_ResultList;
        pthread_mutex_t m_Result_Mutex;
};
extern CMatchCommunication g_MatchCommunication;
#endif

