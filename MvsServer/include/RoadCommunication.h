// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

/**
    文件：RoadCommunication.h
    功能：
    作者：於锋
    时间：2010-1-7
**/

#ifndef ROADCOMMUNICATION_H
#define ROADCOMMUNICATION_H

#include "CSocketBase.h"
#include "global.h"

#ifdef USBKEY
#include "CUsbKeyLockRead.h"
#endif

class CRoadCommunication:public mvCSocketBase
{
 public:
    CRoadCommunication();
    ~CRoadCommunication();

 public:
    bool RoadCommunication();
    bool RunRoadCommunication();

    //打开
    bool OpenSession();
    //关闭
    bool CloseSession();

 private:

    int m_nCenterSocket;
    int m_nRoadCommunicationPort;

    //发送信号互斥
	pthread_mutex_t m_send_mutex;

	#ifdef USBKEY
	CUsbKeyLockRead* m_pUsbKeyLockRead;
	#endif

	bool m_bExistUsb;
};

extern CRoadCommunication g_skpRoadCommunication;

#endif
