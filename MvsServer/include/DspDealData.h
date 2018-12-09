#ifndef DSP_DEAL_DATA_H
#define DSP_DEAL_DATA_H
#include "templateThreadTask.h"
#include "Common.h"
#include "global.h"
#include "StreamBuffer.h"
#include "DspClient.h"

class CDspClient;

class CDspDealData
{
public:
	CDspDealData();
	~CDspDealData();

	void Start();
	int Stop();

	static void* Working(templateThreadTask<CDspDealData>* pThis);

	void SyncResLock(bool bLock);
	void setDspClientHandler(CDspClient* pDspClient);

public:
	DspSocketFd m_dspSocket;
	static int	m_nDspClientId;

	//»·ÐÎ»º³åÇø
	CStreamBuffer* m_pStreamBuffer;
	unsigned long long m_iReadTotal;

	CDspClient*		m_pDspClient;

	pthread_mutex_t	m_mutexLock;
};

#endif

