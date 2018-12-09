#ifndef SEND_THREAD_POOL_H
#define SEND_THREAD_POOL_H
#include "templateThreadTask.h"
#include "dspUnit.h"
#include <semaphore.h>
#include <assert.h>

#define MAXTHREADCOUNTER 100

class CDspClient;
class CDspDealData;

class CSendThreadPool
{
public:
	CSendThreadPool();
	~CSendThreadPool();

	bool AddTask(void* pTask,int type);
	void Start(void* pTask,int type);
	void Stop(void* pTask, int type);
	void Pause(void* pTask,int type);
	void NotifyThreadExit(templateThreadTask<CDspClient>* pDspClient);
	void NotifyThreadExit(templateThreadTask<CDspDealData>* pDeal);

public:
	sem_t  m_fwrite_sem;

private:
	bool Add(CDspClient* pDspClient,int iTaskCount);
	bool Add(CDspDealData* pDspDealData,int iTaskCount);
	void RemoveTask(CDspClient* pDspClient);
	void RemoveTask(CDspDealData* pDeal);
	void PauseTask(CDspClient* pDspClient);
	void PauseTask(CDspDealData* pDeal);
	
private:
	templateThreadTask<CDspClient> *m_pDspClientThreads[MAXTHREADCOUNTER];
	int m_iDspClientThreadCount;
	int m_iDspClientThreadFlag[MAXTHREADCOUNTER];

	templateThreadTask<CDspDealData> *m_pDspDealDataThreads[MAXTHREADCOUNTER];
	int m_iDspDealDataThreadCount;
	int m_iDspDealDataThreadFlag[MAXTHREADCOUNTER];
};
#endif
