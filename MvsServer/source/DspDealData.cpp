#include "DspDealData.h"
#include "dspUnit.h"
#include "RoSeekCameraDsp.h"


CDspDealData::CDspDealData()
{
	m_pDspClient = NULL;
	m_iReadTotal = 0ULL;
	pthread_mutex_init((&m_mutexLock), NULL);
}

CDspDealData::~CDspDealData()
{
	pthread_mutex_destroy(&m_mutexLock);
}

void CDspDealData::Start()
{
	m_iReadTotal = m_pDspClient->m_pStreamBuffer->GetCurrentWriteTotal();
	CDspUnit::m_gpThreadPool->Start(this, 3);
}

//停止接受数据
int CDspDealData::Stop()
{
	printf("EEEEEEEEEEEEEEEEEEEEEEEEEE  Stop[%d]\n",m_iReadTotal);
	//TOD 关闭Socket
	CDspUnit::m_gpThreadPool->Stop(this, 3);
	return 0;
}

void CDspDealData::SyncResLock(bool bLock)
{
	if(bLock)
	{
		pthread_mutex_lock(&m_mutexLock);
	}
	else
	{
		pthread_mutex_unlock(&m_mutexLock);
	}
	return;
}

void* CDspDealData::Working(templateThreadTask<CDspDealData>* pThis)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL );
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL );

	char *buf = new char[MAX_DSP_BUF_LEN];

	while ( pThis->m_iCount > 0 && (pThis->m_iThreadExitFlag == 0))
	{
		//printf("************************[%d][%d]\n",pThis->m_iCount,pThis->m_iThreadExitFlag);
		bool bDealflag = false;
		for ( int i = 0; i < pThis->m_iTaskMaxCount; i++ )
		{
			if ( ((pThis->m_pTaskerFlag + i)->m_iaddtaskFg == 1) && ((pThis->m_pTaskerFlag + i)->m_istartaskFg == 1))
			{
				CDspDealData *pDspDeal = (CDspDealData*)((pThis->m_pTaskerFlag + i)->m_TaskersPtr);
				if(pDspDeal != NULL)
				{
					int len = 0;
					pDspDeal->SyncResLock(true);
					if(pDspDeal->m_pDspClient != NULL)
					{
						len = 0;
						len = pDspDeal->m_pDspClient->m_pStreamBuffer->ReadBuffer(buf, pDspDeal->m_iReadTotal, len);
						//printf("GGGGGGGGGGGGGGGGGG  [%d][%d]\n",pDspDeal->m_iReadTotal,len);
						if (len > 0)
						{
							printf("GGGGGGGGGGGGGGGGGG  [%d][%d]\n",pDspDeal->m_iReadTotal,len);
						//	//return NULL;
						}
					}
					else
					{
						printf("TTTTTTTTTTTTTTTTTTTTTT ERROR *******************\n");
					}
					pDspDeal->SyncResLock(false);

					if(len > 0)
					{
						bDealflag = true;
						pDspDeal->m_iReadTotal += (UINT64)len+12ULL;
						printf("##############AddFrame Start\n");
						#ifndef ALGORITHM_YUV
						((CDspDataManage*)(pDspDeal->m_pDspClient->m_dspSocket.pProcess))->AddFrame(buf);
						#endif
						printf("##############AddFrame End\n");
					}
				}
				else
				{
					printf("****************************\n");
				}
			}
			if((pThis->m_pTaskerFlag + i)->m_iremovetaskFg == 1)
			{
				pThis->TaskCountSyncLck(true);
				CDspDealData *pDspDeal = 0;
				pDspDeal = (pThis->m_pTaskerFlag + i)->m_TaskersPtr;
				if(pDspDeal != NULL)
				{
					delete pDspDeal;
					pDspDeal = 0;
					(pThis->m_pTaskerFlag + i)->m_TaskersPtr = 0;
				}
				(pThis->m_pTaskerFlag + i)->m_istartaskFg = 0;
				(pThis->m_pTaskerFlag + i)->m_iaddtaskFg = 0;
				pThis->m_iCount--;
				pThis->TaskCountSyncLck(false);
			}
		}
		if ( bDealflag != true)
		{
			struct timeval tv;
			tv.tv_sec = 1 / 1000 / 1000;
			tv.tv_usec = 1 % (1000 * 1000);
			select(0,NULL, NULL, NULL, &tv);
		}
	}
	printf("test  ************************[%d][%d]\n",pThis->m_iCount,pThis->m_iThreadExitFlag);

	if(buf)
	{
		delete []buf;
		buf = NULL;
	}

	//退出while循环，清除资源
	for ( int i = 0; i < pThis->m_iTaskMaxCount; i++ )
	{
		{
			if((pThis->m_pTaskerFlag + i)->m_TaskersPtr != NULL)
			{
				delete (CDspDealData*)(pThis->m_pTaskerFlag + i)->m_TaskersPtr;
				(pThis->m_pTaskerFlag + i)->m_TaskersPtr = NULL;
			}
		}
	}
	if (CDspUnit::m_gpThreadPool != NULL)
	{
		CDspUnit::m_gpThreadPool->NotifyThreadExit(pThis);
	}
	printf("************************************* Exit\n");
}

void CDspDealData::setDspClientHandler(CDspClient* pDspClient)
{
	m_pDspClient = pDspClient;
}