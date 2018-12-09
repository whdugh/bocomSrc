#include <unistd.h>
#include <pthread.h>
#include<sys/types.h>
#include<dirent.h>
#include<unistd.h>
#include<stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "dspUnit.h"

CSendThreadPool SendThreadPoolObj;
CSendThreadPool* CDspUnit::m_gpThreadPool    =  &SendThreadPoolObj;

CDspUnit::CDspUnit()
{
	m_pDspDealList = new list<CDspDealData*>();
}

CDspUnit::~CDspUnit()
{
	if (m_pDspClient)
	{
		m_pDspClient->Close();
	}
	if (m_pDspDealList != NULL)
	{
		m_pDspDealList->clear();
		delete m_pDspDealList;
		m_pDspDealList = NULL;
	}
}

int CDspUnit::StartGetDataFromDsp(DspSocketFd dspSocket, EnumDspType emType)
{
	int nRetId = 0;
	int nDspDealNum = 1;//一个Dsp连接，有1个处理

	m_pDspClient = new CDspClient(dspSocket);
	if (m_pDspClient)
	{
		if (m_pDspDealList != NULL)
		{
			m_pDspDealList->clear();
		}

		for(int nLoop = 0; nLoop < nDspDealNum; nLoop++)
		{
			AddDspDealData();
			LogNormal("Start Dsp Deal \n");
		}

		printf("11111111111");

		m_pDspClient->setDspDealList(m_pDspDealList);
		if(m_gpThreadPool->AddTask(m_pDspClient, emType))
		{
			printf("22222");
			LogNormal("Start Dsp Client\n");
			m_pDspClient->Start();
		}
		
		printf("333333");

		nRetId = m_pDspClient->m_nDspClientId;
	}
	return nRetId;
}

// 增加一个Dsp数据处理
bool CDspUnit::AddDspDealData()
{
	CDspDealData* pDspDealData = NULL;
	pDspDealData = new CDspDealData();
	pDspDealData->setDspClientHandler(m_pDspClient);
	if(m_gpThreadPool->AddTask(pDspDealData, tpDealData))
	{
		pDspDealData->Start();
	}
	//		m_gpVisLog->Notice(MODULE_SRCTODEST,LOGLEVEL_RD1,"Start New Dest...");
	m_pDspDealList->push_front(pDspDealData);
}