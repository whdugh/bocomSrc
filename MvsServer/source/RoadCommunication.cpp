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
#include "RoadCommunication.h"


CRoadCommunication g_skpRoadCommunication;

//
void* ThreadRoadCommunication(void* pArg)
{
    CRoadCommunication* pRoadCommunication = (CRoadCommunication*)pArg;

    if(pRoadCommunication == NULL) return pArg;

	pRoadCommunication->RunRoadCommunication();
    pthread_exit((void *)0);
	return pArg;
}

CRoadCommunication::CRoadCommunication()
{
    m_nRoadCommunicationPort = AUTHEN_PORT;
    //发送信号
	pthread_mutex_init(&m_send_mutex,NULL);

	m_bExistUsb = false;

	#ifdef USBKEY
	m_pUsbKeyLockRead = NULL;
	#endif

	m_nCenterSocket = 0;
}

CRoadCommunication::~CRoadCommunication()
{
    //发送信号
	pthread_mutex_destroy(&m_send_mutex);
}


bool CRoadCommunication::RoadCommunication()
{
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
	int nret=pthread_create(&id,&attr,ThreadRoadCommunication,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("create ThreadRoadCommunication error!\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}
	printf("create ThreadRoadCommunication ok!\r\n");
	pthread_attr_destroy(&attr);
	return true;
}

bool CRoadCommunication::RunRoadCommunication()
{
	#ifdef USBKEY
	if(m_pUsbKeyLockRead != NULL)
	{
		bool ret = false;
		ret = m_pUsbKeyLockRead->CompareLocalInfo();
		if (!ret)
		{
			LogError("Communication error!\r\n");
			//认证不成功检测器停止工作(暂停通道检测)
			g_skpChannelCenter.PauseDetect();
			return false;
		}
		LogNormal("Communication==success\r\n");
		g_skpChannelCenter.RestartDetect();
		return true;
	}
	#endif

	if((access("/dev/ttyUSB2",F_OK) == 0))
	{
		//如果有3G卡存在
		return true;
	}
	
}

//打开虚拟串口
bool CRoadCommunication::OpenSession()
{
	#ifdef USBKEY
	FILE* fp = fopen("testusbkey.txt","wb");
	m_pUsbKeyLockRead = new CUsbKeyLockRead;
	bool ret = false;
	ret = m_pUsbKeyLockRead->OpenUsb();
	if (!ret)
	{
		if (m_pUsbKeyLockRead)
		{
			delete m_pUsbKeyLockRead;
			m_pUsbKeyLockRead = NULL;
		}

		if((access("/dev/ttyUSB2",F_OK) == 0))
		{
			fprintf(fp,"test ok\r\n");
			fclose(fp);
			//如果有3G卡存在
			return true;
		}
		
		fprintf(fp,"test error\r\n");
		fclose(fp);
		
		return false;
	}
	if(RunRoadCommunication())
	{
		fprintf(fp,"test ok\r\n");
		fclose(fp);
		return true;
	}
	else
	{
		fprintf(fp,"test error\r\n");
		fclose(fp);
		return false;
	}
	#endif
	
	return true;
}

//关闭虚拟串口
bool CRoadCommunication::CloseSession()
{
	#ifdef USBKEY
	if(m_pUsbKeyLockRead != NULL)
	{
		bool ret = m_pUsbKeyLockRead->CloseUsb();
		if (!ret)
		{
			return false;
		}
		if (m_pUsbKeyLockRead)
		{
			delete m_pUsbKeyLockRead;
			m_pUsbKeyLockRead = NULL;
		}
		return true;
	}
	#endif

	if((access("/dev/ttyUSB2",F_OK) == 0))
	{
		//如果有3G卡存在
		return true;
	}
	return false;
}