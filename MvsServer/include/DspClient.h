#ifndef DSP_CLIENT_H
#define DSP_CLIENT_H
#include "templateThreadTask.h"
#include "Common.h"
#include "global.h"
#include "StreamBuffer.h"
//#include "DspDealData.h"

#include "DspServer.h"

//class CRoSeekCameraDsp;
class CDspServer;
class CDspDealData;

class CDspClient
{
public:
	CDspClient(DspSocketFd dspSocket);
	~CDspClient();

	void Start();
	void Stop();
	void Close();

	static void* Working(templateThreadTask<CDspClient>* pThis);

	void  setDspDealList(void* plist);
public:
	DspSocketFd m_dspSocket;
	static int	m_nDspClientId;

	//»·ÐÎ»º³åÇø
	CStreamBuffer* m_pStreamBuffer;

	list<CDspDealData*> *m_pDspDealList;
};

#endif

