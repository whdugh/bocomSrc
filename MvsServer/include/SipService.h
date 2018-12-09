// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifdef SIP_SERVICE_OPEN

#ifndef SIPSERVICE_H
#define SIPSERVICE_H

#include "CSocketBase.h"
#include <eXosip2/eXosip.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
/******************************************************************************/
//	描述:智能交通检测系统通讯模块。
//	作者:於锋
//	日期:2011-12-5
/******************************************************************************/
 class CSipService: public mvCSocketBase
 {
  public:
	//构造
	CSipService();
	//析构
	~CSipService();
  public:
	//开始侦听服务
	bool Init();
	//释放
	bool UnInit();

    bool mvConnOrLinkTest();

    bool mvStartRecvThread();

	bool mvStartRecvUDPThread();

    bool mvSendLinkTest();

    //设置预置位
    bool mvStop();

    //设置区域
    bool mvPlay();

    //获取区域
    bool mvSetup();

	//重传记录
	bool mvSetDescribe();

	//设置时间
	bool mvSetOption();

	void mvDealMsg(const string &strMsg);

	bool mvRecvCenterServerMsg();

	bool mvRecvSocketMsg(string& strMsg);

	bool mvRebMsgAndSend(const string &strMsg,UINT32 uCmdId, UINT32 uCmdFlag = 0);

	void sipService();
	bool loadSipCfg();

	pthread_t m_uRecvMsgThreadId;

private:
	
	time_t m_lastTime;

	bool m_bCenterLink;

	int m_nCenterSocket;

	int m_nUDPSocket;

	std::string m_strLocalHostIp;
	std::string m_strSipServerIp;
	std::string m_strTelNumber;
	std::string m_strPassword;
	std::string m_strRtspServerIp;

	std::string m_strRtpClientIp;

	int m_registerId;
	int m_rtpClentPort;
	osip_message_t *m_reg;
};
//通讯服务
extern CSipService g_sipService;
#endif
#endif
