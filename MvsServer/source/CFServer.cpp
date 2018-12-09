// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "CFServer.h"
#include "Common.h"
#include <iostream>

#ifdef CFCAMERA
#include "nvr_common.h"

CCFCommunication g_CFCommunication;

//H264采集线程
void* ThreadLoginCFServer(void* pArg)
{
	//取类指针
	CCFCommunication* pHDIPCamera = (CCFCommunication*)pArg;
	if(pHDIPCamera == NULL)
		return pArg;

	pHDIPCamera->LoginCFServer();

	pthread_exit((void *)0);
	return pArg;
}

void cbplayMsg(unsigned int playTask, void * dwUser, int iErrorCode)
{
	printf("playTask = %d,iErrorCode = %d",playTask,iErrorCode);
}



CCFCommunication::CCFCommunication()
{
	m_loginID = 0;
}

CCFCommunication::~CCFCommunication()
{

}

bool CCFCommunication::Init()
{
	//线程属性
	pthread_t m_nThreadId = 0;
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	int nret=pthread_create(&m_nThreadId,NULL,ThreadLoginCFServer,this);

	if(nret!=0)
	{
		//失败
		LogError("创建h264采集线程失败,无法进行采集！\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}
	pthread_attr_destroy(&attr);
	return true;
}

void CCFCommunication::LoginCFServer()
{
	fMsgCallback  pplayMsg = cbplayMsg;

	while(!g_bEndThread)
	{
		if(m_loginID <= 0)
		{
			string strHost(g_monitorHostInfo.chMonitorHost);
			if(strHost.size() > 8 && g_monitorHostInfo.uMonitorPort > 0)
			{
				m_loginID = AVS_login(g_monitorHostInfo.chMonitorHost,g_monitorHostInfo.uMonitorPort,g_monitorHostInfo.chUserName,g_monitorHostInfo.chPassWord,pplayMsg,this);

				if (m_loginID > 0 )
				{
					g_nLoginState = 1;
				}
			}
		}
		sleep(5);
	}
}


bool CCFCommunication::UnInit()
{
	if(m_loginID > 0)
	AVS_logout(m_loginID);

	return true;
}

#endif
