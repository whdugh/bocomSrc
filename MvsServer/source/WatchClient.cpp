// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "Common.h"
#include "CommonHeader.h"
#include "WatchClient.h"

CWatchClient g_WatchClient;

//记录发送线程
void* ThreadWatchLink(void* pArg)
{
	//处理一条数据
	g_WatchClient.LinkTest();

    pthread_exit((void *)0);
	return pArg;
}


CWatchClient::CWatchClient()
{
    m_bCenterLink = false;
    m_nCenterSocket = 0;
    m_nCSLinkCount = 0;
    m_nThreadId = 0;
}


CWatchClient::~CWatchClient()
{
}

//启动侦听服务
bool CWatchClient::Init()
{
	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    //启动检测结果发送线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadWatchLink,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}
	pthread_attr_destroy(&attr);

	return true;
}

//释放
bool CWatchClient::UnInit()
{
    //停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}
	return true;
}

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void CWatchClient::LinkTest()
{
    while (!g_bEndThread)
    {
        if (!m_bCenterLink)
        {
                if (mvConnectToCS())
                {
                    LogNormal("连接WatchDog成功!\n");
                    m_bCenterLink = true;
                    m_nCSLinkCount = 0;
                }
        }

		mvSendLinkTest();

		//60秒检测一次
		sleep(60);
    }
}


/*
* 函数介绍：连接到中心
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CWatchClient::mvConnectToCS()
{
    //connect to center server and set socket's option.

    if (!mvPrepareSocket(m_nCenterSocket))
    {
        //printf("\n准备连接中心数据服务器套接字失败!\n");
        return false;
    }

    if (!mvWaitConnect(m_nCenterSocket, "127.0.0.1", 12300,2))
    {
        //printf("\n尝试连接中心数据服务器失败!\n");
        return false;
    }

    return true;
}


/*
* 函数介绍：发送心跳测试
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CWatchClient::mvSendLinkTest()
{
	if(!m_bCenterLink)
	{
		return false;
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER);

	string strFullMsg;
	strFullMsg.append((char*)&mHeader,sizeof(MIMAX_HEADER));

	if(!mvSendMsgToSocket(m_nCenterSocket,strFullMsg,true))
	{
		LogNormal("send WatchDog LinkTest error!\n");
		m_bCenterLink = false;
		mvCloseSocket(m_nCenterSocket);
		return false;
	}

	static int countLinkTest = 0;

	if(countLinkTest == 0)
	LogNormal("send WatchDog LinkTest ok!\n");

	 countLinkTest++;
    if(countLinkTest>2)
        countLinkTest = 0;

    return true;
}
