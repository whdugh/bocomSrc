#include "DspClient.h"
#include "dspUnit.h"
#include "constdef.h"

int CDspClient::m_nDspClientId = 1000;

CDspClient::CDspClient(DspSocketFd dspSocket)
{
	m_nDspClientId++;
	m_dspSocket = dspSocket;

	struct timeval timeo;
	socklen_t slen = sizeof(timeo);
	timeo.tv_sec = 1;
	timeo.tv_usec = 0;//超

	if(setsockopt(m_dspSocket.cfd.fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, slen) == -1)
	{
		LogNormal("CDspClient setsockopt err!");
	}

	m_pStreamBuffer = NULL;
	m_pStreamBuffer = new CStreamBuffer(10*1024*1024);

	m_pDspDealList = NULL;
}

CDspClient::~CDspClient()
{
	if(m_pStreamBuffer != NULL)
	{
		delete m_pStreamBuffer;
		m_pStreamBuffer = 0;
	}
}

void CDspClient::Start()
{
	CDspUnit::m_gpThreadPool->Start(this, tpTcpClient);
}

void CDspClient::Stop()
{
	if(m_pDspDealList != NULL) 
	{
		list <CDspDealData *>::const_iterator dealIter;
		for ( dealIter = m_pDspDealList->begin(); dealIter != m_pDspDealList->end(); dealIter++ )
		{ 
			CDspDealData *pDeal =  (CDspDealData*)(*dealIter);
			if ( pDeal != NULL )
			{
				pDeal->Stop();
				pDeal->SyncResLock(true);
				if(pDeal != NULL)
				{
					pDeal->setDspClientHandler((CDspClient*)0);
				}
				pDeal->SyncResLock(false);
			}
		}
	}
	CDspUnit::m_gpThreadPool->Pause(this, tpTcpClient);

	return;
}

void CDspClient::Close()
{
	if(m_pDspDealList != NULL) 
	{
		list <CDspDealData *>::const_iterator dealIter;
		for ( dealIter = m_pDspDealList->begin(); dealIter != m_pDspDealList->end(); dealIter++ )
		{ 
			CDspDealData *pDeal =  (CDspDealData*)(*dealIter);
			if ( pDeal != NULL )
			{
				pDeal->SyncResLock(true);
				if(pDeal != NULL)
				{
					pDeal->setDspClientHandler((CDspClient*)0);
				}
				pDeal->SyncResLock(false);

				pDeal->Stop();
			}
		}
	}
	CDspUnit::m_gpThreadPool->Stop(this, tpTcpClient);
}

void* CDspClient::Working(templateThreadTask<CDspClient>* pThis)
{
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
	pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );

	bool bRet = false;
	int bytes = 0;
	int nDspHeaderSize = sizeof(Image_header_dsp);
	unsigned long long nCnt = 0;
	char *buf = new char[MAX_DSP_BUF_LEN];

	while ( pThis->m_iCount > 0 && (pThis->m_iThreadExitFlag == 0))
	{
		for(int i = 0; i < pThis->m_iTaskMaxCount; i++)
		{
			int iLen = 0;
			bool isSet=true;
			if(((pThis->m_pTaskerFlag + i)->m_iaddtaskFg == 1) \
				&& ((pThis->m_pTaskerFlag + i)->m_istartaskFg == 1) && isSet )
			{
				bytes = 0;
				bRet = false;

				DspSocketFd * pDspSocket = NULL;
				pDspSocket = &((pThis->m_pTaskerFlag + i)->m_TaskersPtr->m_dspSocket);

				//接受数据
				if(0 == pDspSocket->cfd.nRecvFlag)
				{
					pDspSocket->cfd.nRecvFlag = 1;

					list<CDspDealData*> *pDspDealList = (pThis->m_pTaskerFlag + i)->m_TaskersPtr->m_pDspDealList;

					int nTs1 = GetTimeStamp();
					int nTs2 = 0;
					int nTsDis = 0;

					LogTrace("RecvSize.txt", "tick 111-i:%d--t1:%lld fd:%d ip:%s bytes:%d nCnt:%d ", \
						i, nTs1, pDspSocket->cfd.fd, pDspSocket->cfd.ip, bytes, nCnt);

					#ifndef ALGORITHM_YUV
					pDspSocket->cfd.nTimeFlag = nTs1;
					bRet = ((CDspServer*)(pDspSocket->p))->RecvDspDataFd(*pDspSocket, buf, bytes);
					#endif

					nTs2 = GetTimeStamp();
					nTsDis = nTs2 - nTs1;
					LogTrace("RecvSize.txt", "tick 222-i:%d--t1:%lld t2:%lld nTsDis:%d fd:%d ip:%s bytes:%d bRet:%d nCnt:%d ", \
						i, nTs1, nTs2, nTsDis, pDspSocket->cfd.fd, pDspSocket->cfd.ip, bytes, bRet, nCnt);

					nCnt++;

					printf("*************************Get Header[%d]\n",nCnt);

					pDspSocket->cfd.nRecvFlag = 0;					
				}//end of if(1 == nRecvFlag)

				if (!bRet)
				{
					printf("Not Get Data From Dsp!\n");
				}
				else
				{
					if(bytes > 96 && bytes < MAX_DSP_BUF_LEN && buf != NULL)
					{
						printf("************11**WriteBuffer*************bytes=%d****\n", bytes);
						(pThis->m_pTaskerFlag + i)->m_TaskersPtr->m_pStreamBuffer->WriteBuffer(buf, bytes);
						printf("************22**WriteBuffer*****************\n");
					}
					else
					{
						LogNormal("err! to Write: %d too Big!! buf:%x ", bytes, buf);
					}
					//	printf("****************************************************[%d][%d]\n",dspSocket.cfd.fd,bytes);
				}
			}
			
			if((pThis->m_pTaskerFlag + i)->m_iremovetaskFg == 1)
			{
				pThis->TaskCountSyncLck(true);
				if((pThis->m_pTaskerFlag + i)->m_TaskersPtr != NULL)
				{
					delete (pThis->m_pTaskerFlag + i)->m_TaskersPtr;
					(pThis->m_pTaskerFlag + i)->m_TaskersPtr = 0;
				}
				(pThis->m_pTaskerFlag + i)->m_istartaskFg = 0;
				(pThis->m_pTaskerFlag + i)->m_iaddtaskFg = 0;	
				pThis->m_iCount--;
				pThis->TaskCountSyncLck(false);
			}
		}//End of for
	}//End of while

	if(buf)
	{
		delete []buf;
		buf = NULL;
	}
	for(int i = 0; i < pThis->m_iTaskMaxCount; i++)
	{
		if((pThis->m_pTaskerFlag + i)->m_TaskersPtr != NULL)
		{
			delete (pThis->m_pTaskerFlag + i)->m_TaskersPtr;
			(pThis->m_pTaskerFlag + i)->m_TaskersPtr = 0;
		}
	}
	if (CDspUnit::m_gpThreadPool != NULL)
	{
		CDspUnit::m_gpThreadPool->NotifyThreadExit(pThis);
	}
	
}

void  CDspClient::setDspDealList(void* plist)
{
	m_pDspDealList = (list<CDspDealData*>*)plist;
}
