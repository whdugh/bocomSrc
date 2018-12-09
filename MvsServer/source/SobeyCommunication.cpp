// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

/**
    文件：RoadCommunication.cpp
    功能：
    作者：於锋
    时间：2010-1-7
**/

#include "Common.h"
#include "CommonHeader.h"
#include "SobeyCommunication.h"

#ifdef MONITOR

LibMonitor g_monitor;

CSobeyCommunication g_SobeyCommunication;

//
void* ThreadSobeyCommunication(void* pArg)
{
    CSobeyCommunication* pRoadCommunication = (CSobeyCommunication*)pArg;

    if(pRoadCommunication == NULL) return pArg;

	pRoadCommunication->LoginSobeyServer();
    pthread_exit((void *)0);
	return pArg;
}

int LoginStateCallBackFunc(LoginState enState,string strDes)
{
	if(enState == LOGIN_SUCCEEDED )
	{
		g_nLoginState = 1;
		printf("$$$$$$$ Login success $$$$$$$$[%s],g_nLoginState=%d,&g_nLoginState=%lld\n",strDes.c_str(),g_nLoginState,&g_nLoginState);
#ifdef MONITORLOG
		fprintf(g_pMonitorLog,"$$$$$$$ Login success $$$$$$$$[%s],g_nLoginState=%d,&g_nLoginState=%lld\n",strDes.c_str(),g_nLoginState,&g_nLoginState);
		fflush(g_pMonitorLog);
#endif
	}
	else if(enState == LOGIN_FAILED)
	{
		LogError("Login failed\n");
		g_nLoginState = 2;
		printf("$$$$$$$ Login failed $$$$$$$$[%s],g_nLoginState=%d,&g_nLoginState=%lld\n",strDes.c_str(),g_nLoginState,&g_nLoginState);
#ifdef MONITORLOG
		printf("=================g_nLoginState=%d,g_pMonitorLog=%d,&g_pMonitorLog=%lld\n",g_nLoginState,g_pMonitorLog,&g_pMonitorLog);
		fprintf(g_pMonitorLog,"$$$$$$$ Login failed $$$$$$$$[%s],g_nLoginState=%d,&g_nLoginState=%lld\n",strDes.c_str(),g_nLoginState,&g_nLoginState);
		fflush(g_pMonitorLog);
#endif
	}

	return 1;
}

CSobeyCommunication::CSobeyCommunication()
{
   
}

CSobeyCommunication::~CSobeyCommunication()
{
}


bool CSobeyCommunication::Init()
{
	m_bLogOnServer = false;
    //启动认证线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动事件监控线程
	int nret=pthread_create(&id,&attr,ThreadSobeyCommunication,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("create ThreadSobeyCommunication error!\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}
	printf("create ThreadSobeyCommunication ok!\r\n");
	pthread_attr_destroy(&attr);
	return true;
}

bool CSobeyCommunication::UnInit()
{
	m_bLogOnServer = false;
	g_monitor.UserLogOut();
	g_nLoginState = 0;

	return true;
}

bool CSobeyCommunication::UserLogOnServer()
{
	 int nLogOnServer = g_monitor.UserLogOnServer(g_monitorHostInfo.chMonitorHost,g_monitorHostInfo.uMonitorPort,g_monitorHostInfo.chUserName,g_monitorHostInfo.chPassWord,LoginStateCallBackFunc);
     
	 if(nLogOnServer <= 0)
	 {
		 LogError("UserLogOnServer error");
		 return false;
	 }
	 else
	 {
		 LogNormal("UserLogOnServer ok");
		 return true;
	 }
}

void CSobeyCommunication::LoginSobeyServer()
{
	while(!g_bEndThread)
	{
		while(!m_bLogOnServer)
		{
			m_bLogOnServer = UserLogOnServer();

			sleep(5);
		}
		sleep(5);
	}
}
#endif

