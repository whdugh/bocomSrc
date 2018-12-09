// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef WATCHCLIENT_H
#define WATCHCLIENT_H

#include "CSocketBase.h"

/**
    文件：WatchClient
    功能：WatchDog通讯类
    作者：於锋
    时间：2015-01-13
**/

class CWatchClient:public mvCSocketBase
{
    public:
        //构造
        CWatchClient();
        //析构
        virtual ~CWatchClient();
    public:

        //启动服务
        bool Init();

        //释放
        bool UnInit();

		//心跳检测
		void LinkTest();

    private:

        /*连接到中心端*/
        bool mvConnectToCS();

        /*发送心跳测试*/
        bool mvSendLinkTest();

     private:
        int m_nCenterSocket;
        int m_nCSLinkCount;

        bool m_bCenterLink;

        pthread_t m_nThreadId;

};
extern CWatchClient g_WatchClient;
#endif

