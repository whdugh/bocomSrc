#include "SendThreadPool.h"


CSendThreadPool::CSendThreadPool()
{
	m_iDspClientThreadCount = 0;
	for ( int i = 0;  i < MAXTHREADCOUNTER; i++ )
	{
		m_iDspClientThreadFlag[i] = 0;
		m_pDspClientThreads[i] = NULL;
	}

	m_iDspDealDataThreadCount = 0;
	for ( int i = 0;  i < MAXTHREADCOUNTER; i++ )
	{
		m_iDspDealDataThreadFlag[i] = 0;
		m_pDspDealDataThreads[i] = NULL;
	}
}

CSendThreadPool::~CSendThreadPool()
{
	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if(m_pDspClientThreads[i] != NULL)
		{
			if(m_pDspClientThreads[i]->IsRunning())
			{
				m_pDspClientThreads[i]->CancelThread();
			}
			m_pDspClientThreads[i]->LockThreadExit();
			if(m_pDspClientThreads[i] != NULL)
			{
				m_iDspClientThreadFlag[i] = 0;
				delete m_pDspClientThreads[i];
				m_pDspClientThreads[i] = NULL;
			}
			m_pDspClientThreads[i]->UnLockThreadExit();
			m_iDspClientThreadCount--;
		}
	}

	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if(m_pDspDealDataThreads[i] != NULL)
		{
			if(m_pDspDealDataThreads[i]->IsRunning())
			{
				m_pDspDealDataThreads[i]->CancelThread();
			}
			m_pDspDealDataThreads[i]->LockThreadExit();
			if(m_pDspDealDataThreads[i] != NULL)
			{
				m_iDspDealDataThreadFlag[i] = 0;
				delete m_pDspDealDataThreads[i];
				m_pDspDealDataThreads[i] = NULL;
			}
			m_pDspDealDataThreads[i]->UnLockThreadExit();
			m_iDspDealDataThreadCount--;
		}
	}
}

bool CSendThreadPool::AddTask(void* pTask,int type)
{
	bool ret = false;
	switch(type)
	{
	case 2://Client
		{
			CDspClient* pDspClient = (CDspClient*)pTask;
			if(Add(pDspClient, 1))
			{
				ret = true;
			}
			break;
		}
	case 3: // DealData
		{
			CDspDealData* pDspDeal = (CDspDealData*)pTask;
			if(Add(pDspDeal, 1))
			{
				ret = true;
			}
			break;
		}
	default:
		break;
	}

	unsigned int mstime = 10;
	struct timeval tv;
	tv.tv_sec = mstime/1000;
	tv.tv_usec = mstime%1000 * 1000;
	select(0,NULL, NULL, NULL, &tv);

	return ret;
}

bool CSendThreadPool::Add(CDspClient* pDspClient,int iTaskCount)
{
	if ( pDspClient == NULL ) return false;

	if ( m_iDspClientThreadCount > 0 )
	{
		for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
		{
			if ( m_iDspClientThreadFlag[i] == 1 )
			{
				templateThreadTask<CDspClient>* pDspThread = m_pDspClientThreads[i];
				if ( pDspThread != NULL )
				{
					pDspThread->TaskCountSyncLck(true);
					if ( !pDspThread->IsFull() )
					{
						if(pDspThread->AddTask(pDspClient))
						{
							pDspThread->TaskCountSyncLck(false);
							return true;
						}
						else
						{
							pDspThread->TaskCountSyncLck(false);
							return false;
						}
					}
					pDspThread->TaskCountSyncLck(false);
				}
			}
		}
	}

	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if ( m_iDspClientThreadFlag[i] == 0 )
		{
			templateThreadTask<CDspClient>*pSrcThread = 0;
			pSrcThread = new templateThreadTask<CDspClient>(iTaskCount);
			m_pDspClientThreads[i] = pSrcThread;
			m_iDspClientThreadFlag[i] = 1;
			m_iDspClientThreadCount++;

			if ( pSrcThread != NULL )
			{
				if(pSrcThread->AddTask(pDspClient))
				{	
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}
	return false;
}

void CSendThreadPool::Start(void* pTask, int type)
{
	switch(type)
	{
	case 2://tpTcpClient:
		{
			CDspClient* pSrc =(CDspClient*)pTask;
			for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
			{
				if(m_pDspClientThreads[i] != NULL)
				{
					if(m_pDspClientThreads[i]->StartTask(pSrc))
					{
						break;
					}
				}
			}
			break;
		}
	case 3:// DealData
		{
			CDspDealData* pDealData =(CDspDealData*)pTask;
			for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
			{
				if(m_pDspDealDataThreads[i] != NULL)
				{
					if(m_pDspDealDataThreads[i]->StartTask(pDealData))
					{
						break;
					}
				}
			}
			break;
		}
	default:
		break;
	}
	return;
}

void CSendThreadPool::Stop(void* pTask, int type)
{
	switch(type)
	{
	case 2://tpTcpClient:
		{
			CDspClient* pDsp =(CDspClient*)pTask;
			RemoveTask(pDsp);
			break;
		}
	case 3:// DealData
		{
			CDspDealData* pDeal =(CDspDealData*)pTask;
			RemoveTask(pDeal);
			break;
		}
	default:
		break;
	}
	return;
}

void CSendThreadPool::Pause(void* pTask,int type)
{
	switch(type)
	{
	case 2://tpTcpClient:
		{
			CDspClient* pDsp =(CDspClient*)pTask;
			PauseTask(pDsp);
			break;
		}
	case 3:// DealData
		{
			CDspDealData* pDeal =(CDspDealData*)pTask;
			PauseTask(pDeal);
			break;
		}
	default:
		break;
	}
	return;
}

bool CSendThreadPool::Add(CDspDealData* pDspDealData,int iTaskCount)
{
	if ( pDspDealData == NULL ) return false;

	if ( m_iDspDealDataThreadCount > 0 )
	{
		for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
		{
			if ( m_iDspDealDataThreadFlag[i] == 1 )
			{

				templateThreadTask<CDspDealData>* pDealDataThread = m_pDspDealDataThreads[i];
				if ( pDealDataThread != NULL )
				{
					pDealDataThread->TaskCountSyncLck(true);
					if ( !pDealDataThread->IsFull() )
					{
						if(pDealDataThread->AddTask(pDspDealData))
						{
							pDealDataThread->TaskCountSyncLck(false);
							return true;
						}
						else
						{
							pDealDataThread->TaskCountSyncLck(false);
							return false;
						}
					}
					pDealDataThread->TaskCountSyncLck(false);
				}
			}
		}
	}

	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if ( m_iDspDealDataThreadFlag[i] == 0 )
		{
			templateThreadTask<CDspDealData>*pDealDataThread = 0;
			pDealDataThread = new templateThreadTask<CDspDealData>(iTaskCount);
			m_pDspDealDataThreads[i] = pDealDataThread;
			m_iDspDealDataThreadFlag[i] = 1;
			m_iDspDealDataThreadCount++;

			if ( pDealDataThread != NULL )
			{
				if(pDealDataThread->AddTask(pDspDealData))
				{	
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}
	return false;
}

void CSendThreadPool::RemoveTask(CDspClient* pDspClient)
{
	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if ( m_iDspClientThreadFlag[i] == 1 )
		{
			templateThreadTask<CDspClient> *pDspClientThread = m_pDspClientThreads[i];
			if ( pDspClientThread != NULL )
			{
				if(pDspClientThread->RemoveTask(pDspClient))
				{
					break;
				}
			}
		}
	}
}

void CSendThreadPool::RemoveTask(CDspDealData* pDeal)
{
	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if ( m_iDspDealDataThreadFlag[i] == 1 )
		{
			templateThreadTask<CDspDealData> *pDealThread = m_pDspDealDataThreads[i];
			if ( pDealThread != NULL )
			{
				if(pDealThread->RemoveTask(pDeal))
				{
					break;
				}
			}
		}
	}
}

void CSendThreadPool::PauseTask(CDspClient* pDspClient)
{
	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if ( m_iDspClientThreadFlag[i] == 1 )
		{
			templateThreadTask<CDspClient> *pDspClientThread = m_pDspClientThreads[i];
			if ( pDspClientThread != NULL )
			{
				if(pDspClientThread->PauseTask(pDspClient))
				{
					break;
				}
			}
		}
	}
}

void CSendThreadPool::PauseTask(CDspDealData* pDeal)
{
	for ( int  i = 0; i < MAXTHREADCOUNTER; i++ )
	{
		if ( m_iDspDealDataThreadFlag[i] == 1 )
		{
			templateThreadTask<CDspDealData> *pDealThread = m_pDspDealDataThreads[i];
			if ( pDealThread != NULL )
			{
				if(pDealThread->PauseTask(pDeal))
				{
					break;
				}
			}
		}
	}
}


void CSendThreadPool::NotifyThreadExit(templateThreadTask<CDspClient>* pDspClient)
{
	if (pDspClient == NULL) return;

	for (int  i = 0; i < MAXTHREADCOUNTER; i++)
	{
		if(m_iDspClientThreadFlag[i] == 1)
		{
			if (((templateThreadTask<CDspClient>*)m_pDspClientThreads[i]) == pDspClient)
			{
				delete pDspClient;
				pDspClient = NULL;
				m_pDspClientThreads[i] = 0;
				m_iDspClientThreadCount--;
				m_iDspClientThreadFlag[i] = 0;
				break;
			}
		}
	}
}

void CSendThreadPool::NotifyThreadExit(templateThreadTask<CDspDealData>* pDeal)
{
	if (pDeal == NULL) return;

	for (int  i = 0; i < MAXTHREADCOUNTER; i++)
	{
		if(m_iDspDealDataThreadFlag[i] == 1)
		{
			if (((templateThreadTask<CDspDealData>*)m_pDspDealDataThreads[i]) == pDeal)
			{
				delete pDeal;
				pDeal = NULL;
				m_pDspDealDataThreads[i] = 0;
				m_iDspDealDataThreadCount--;
				m_iDspDealDataThreadFlag[i] = 0;
				break;
			}
		}
	}
}

