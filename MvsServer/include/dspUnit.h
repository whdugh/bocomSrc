#ifndef DSP_UNIT_H
#define DSP_UNIT_H
#include "SendThreadPool.h"
#include "DspClient.h"
#include "DspDealData.h"

typedef enum _EnumDspType
{
	tpTcpServer = 1,
	tpTcpClient,
	tpDealData
}EnumDspType;


class CSendThreadPool;
class CDspClient;


class CDspUnit{
public:
	CDspUnit();
	virtual ~CDspUnit();

	// 开始从Dsp相机获取data
	int StartGetDataFromDsp(DspSocketFd dspSocket, EnumDspType emType = tpTcpClient);

	// 增加一个Dsp数据处理
	bool AddDspDealData();

public:
	static CSendThreadPool*  m_gpThreadPool;

	CDspClient* m_pDspClient;

private:
	
	list<CDspDealData*>* m_pDspDealList;		//一个Dsp对应的处理
};
#endif
