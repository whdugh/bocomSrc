// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

/**
    文件：SobeyCommunication.h
    功能：
    作者：於锋
    时间：2010-1-7
**/

#ifndef SOBEYCOMMUNICATION_H
#define SOBEYCOMMUNICATION_H

#ifdef MONITOR
#include "libmonitor.h"

extern LibMonitor g_monitor;

class CSobeyCommunication
{
 public:
    CSobeyCommunication();
    ~CSobeyCommunication();

	bool Init();
	bool UnInit();

	bool UserLogOnServer();

	void LoginSobeyServer();
		
	void SetLogOnStatus(bool bLogOnServer){ m_bLogOnServer = bLogOnServer;}

 private:
	bool m_bLogOnServer;
};

extern CSobeyCommunication g_SobeyCommunication;

#endif
#endif
