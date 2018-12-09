#pragma  one

#include "Common.h"
#include "DioComSignalProtocol.h"



class  DioComSignalMachine : public AbstractSerial
{
public:

	DioComSignalMachine();
	~DioComSignalMachine();

	int		OpenDev();

	int		HandelSignal(int nRoadWayId);
	// ·¢ËÍÐÅºÅ
	int WriteSignal(string strMsg);


protected:
private:

	pthread_mutex_t		m_ComPoseWriteLock;
};

extern DioComSignalMachine  g_DioComSignalProtocol2;