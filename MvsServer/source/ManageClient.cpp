#include "ManageClient.h"
#include "Common.h"
#include "global.h"
#include "RoadMsgCenter.h"

//#define DSP_SERVER_TEST //本地1拖N测试,ip,port映射

int CManageClient::m_exit = 0;

CManageClient::CManageClient()
{
	m_bPrintThStart = false;
}

CManageClient::~CManageClient()
{
	CleanMap();
}

int CManageClient::BeginRealRunInfoThread()
{
	pthread_t m_hId = 0;
	//此线程为专门统计信息
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	if(pthread_create(&m_hId, &attr, BeginInfoFunc, this) != 0)
	{
		printf("创建线程失败。");
		return -1;
	}
	pthread_attr_destroy(&attr);
	return 0;
}

threadfunc_t STDPREFIX CManageClient::BeginInfoFunc(threadparam_t lpParam)
{
	CManageClient *pThis = (CManageClient *)lpParam;
	pThis->InfoFunc();
	return NULL;
}

void CManageClient::InfoFunc()
{
	time_t startTime;
	time_t nowTime;
	time(&startTime);
	while(CManageClient::m_exit == 0)
	{
		time(&nowTime);
		int offTime = (nowTime-startTime);
		if(offTime >= 30)
		{
			PrintDspClientInfo();
			startTime = nowTime;
		}
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
	pthread_exit((void*)0);
}

void CManageClient::PrintDspClientInfo()
{
	int nCount = 1;

	LogNormal("Dsp Client connect Num:[%d]\n",m_DspClientMap.size());

	//传递指针
	g_skpRoadMsgCenter.SetManageClient(this);
}

// 增加一个DSP连接
bool CManageClient::AddDspClient(const DspSocketFd dspSocket)
{
	bool nRet = true;

	LogNormal("Add Dsp Client.SOCKET:%d [%s:%d]\n", dspSocket.cfd.fd, dspSocket.cfd.ip, dspSocket.cfd.port);
	LogTrace("RecvSize.txt", "Add Dsp Client.SOCKET:%d [%s:%d]\n", dspSocket.cfd.fd, dspSocket.cfd.ip, dspSocket.cfd.port);

	char szAddress[64] = {0};

#ifdef DSP_SERVER_TEST
	sprintf(szAddress, "%s:%d", dspSocket.cfd.ip, dspSocket.cfd.port);//test
#else
	sprintf(szAddress, "%s", dspSocket.cfd.ip);
#endif

	if (m_bPrintThStart == false)
	{
		BeginRealRunInfoThread();
		m_bPrintThStart = true;
	}

	CDspUnit* pUnit = NULL;
	pUnit = IsExistDspClient(szAddress);
	if (pUnit)
	{
		LogNormal("Dsp Client[%s] is Exist! Del old connect!\n",szAddress);
		//MoveDspClient(dspSocket, strIp, nPort);
		MoveDspClient(dspSocket);
		usleep(5*1000);
//		return false;
	}

	if ((CDspUnit::m_gpThreadPool != NULL))
	{
		pUnit = new CDspUnit();
	}
	if (pUnit == NULL)
	{
		LogNormal("Add Dsp Client is failed!\n");
		return false;
	}

	int nRetDspClientId = pUnit->StartGetDataFromDsp(dspSocket,tpTcpClient);
	m_DspClientMapLock.Lock();
	if (nRetDspClientId > 0)
	{
		LogNormal("Dsp Client [%s] is added\n",szAddress);
		m_DspClientMap.insert(make_pair(szAddress, pUnit));
	}
	m_DspClientMapLock.UnLock();

	return nRet;
}

// 删除一个Dsp连接
bool CManageClient::MoveDspClient(const DspSocketFd dspSocket)
{
	LogTrace("RecvSize.txt", \
		"**********************CManageClient::MoveDspClient fd:%d ip:%s---m_DspClientMap.size()=%d", \
		dspSocket.cfd.fd, dspSocket.cfd.ip, m_DspClientMap.size());

	char szAddress[64] = {0};

#ifdef DSP_SERVER_TEST
	sprintf(szAddress, "%s:%d", dspSocket.cfd.ip, dspSocket.cfd.port);
#else
	sprintf(szAddress, "%s", dspSocket.cfd.ip);
#endif
	
	m_DspClientMapLock.Lock();

	map<string, CDspUnit*>::iterator iter;
	if(m_DspClientMap.size() > 0)
	{
		iter = m_DspClientMap.find(szAddress);
		if(iter != m_DspClientMap.end())
		{
			pair<string, CDspUnit*> p = *iter;
			p.first.erase();

			CDspUnit* lp = p.second;
			if (lp)
			{
				LogNormal("Stop Dsp Client[IP:%s:%d]\n", dspSocket.cfd.ip, dspSocket.cfd.port);				

				#ifndef ALGORITHM_YUV
				CDspServer* pDspServer = (CDspServer*)(lp->m_pDspClient->m_dspSocket.p);
				pDspServer->CloseFd(lp->m_pDspClient->m_dspSocket.cfd.fd);

				LogTrace("RecvSize.txt", "Stop Dsp Client[IP:%s:%d] fd_old:%d fd_new:%d \n", \
					dspSocket.cfd.ip, dspSocket.cfd.port, lp->m_pDspClient->m_dspSocket.cfd.fd, dspSocket.cfd.fd);

				#endif

				delete lp;
				lp = NULL;
			}
			m_DspClientMap.erase(iter);
		}
	}
	
	m_DspClientMapLock.UnLock();
}

// 清除所有的Dsp客户端
bool CManageClient::CleanMap()
{
	m_DspClientMapLock.Lock();
	map<string, CDspUnit*>::iterator iterDsp;
	for(iterDsp = m_DspClientMap.begin(); iterDsp != m_DspClientMap.end(); iterDsp++)
	{
		pair<string, CDspUnit*> p = *iterDsp;
		p.first.erase();

		CDspUnit* lp = p.second;
		if (lp)
		{
			delete lp;
			lp = NULL;
		}
		m_DspClientMap.erase(iterDsp);
	}
	m_DspClientMapLock.UnLock();
}

// 通过IP和Port查找Dsp是否存在
CDspUnit* CManageClient::IsExistDspClient(char* szAddress)
{
	m_DspClientMapLock.Lock();
	map<string, CDspUnit*>::iterator iter;
	iter = m_DspClientMap.find(szAddress);
	if(iter != m_DspClientMap.end())
	{
		m_DspClientMapLock.UnLock();
		return iter->second;
	}
	m_DspClientMapLock.UnLock();
	return NULL;
}

//断开指定socket连接
bool CManageClient::DisConnectClient(const DspSocketFd &dspSocket)
{
	bool bRemove = false;
	
	m_DspClientMapLock.Lock();
	
	char szAddress[64] = {0};
	
#ifdef DSP_SERVER_TEST
	sprintf(szAddress, "%s:%d", dspSocket.cfd.ip, dspSocket.cfd.port);//test
#else
	sprintf(szAddress, "%s", dspSocket.cfd.ip);
#endif

	map<string, CDspUnit*>::iterator iter;
	if(m_DspClientMap.size() > 0)
	{
		iter = m_DspClientMap.find(szAddress);
		if(iter != m_DspClientMap.end())
		{
			pair<string, CDspUnit*> p = *iter;
			CDspUnit* lp = p.second;
			if(lp->m_pDspClient->m_dspSocket.cfd.fd == dspSocket.cfd.fd)
			{
				lp->m_pDspClient->m_dspSocket.cfd.connectFlag = false;//断开标志
				bRemove = true;				
			}
			else
			{
				if (lp)
				{
					LogNormal("-need--CloseFd client-sock:%d \n", dspSocket.cfd.fd);
				}

				//LogNormal("--DisConnectClient--fd:%d ! \n", dspSocket.cfd.fd);
			}
		}
	}
	m_DspClientMapLock.UnLock();

	if(bRemove)
	{
		MoveDspClient(dspSocket);
	}
	
	return true;
}

bool CManageClient::GetDspList(DSP_LIST &dsplist)
{
	dsplist.clear();
	if(m_DspClientMap.size() > 0)
	{
		DSP_INFO dspInfo;
		m_DspClientMapLock.Lock();
		map<string, CDspUnit*>::iterator iterDsp;
		for(iterDsp = m_DspClientMap.begin(); iterDsp != m_DspClientMap.end(); iterDsp++)
		{
			memcpy(dspInfo.szIp, (iterDsp->first).c_str(), (iterDsp->first).size());
			//other dspinfo...
			dsplist.push_back(dspInfo);
		}
		m_DspClientMapLock.UnLock();

	}
}