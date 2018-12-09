#include "DioComSignalMachine.h"

DioComSignalMachine  g_DioComSignalProtocol2;

DioComSignalMachine::DioComSignalMachine()
{
	pthread_mutex_init(&m_ComPoseWriteLock, NULL);
}
DioComSignalMachine::~DioComSignalMachine()
{
	pthread_mutex_destroy(&m_ComPoseWriteLock);
}


int DioComSignalMachine::OpenDev()
{
	fd_com = open_port(g_DioComSetting.nComPort, g_DioComSetting.nBaud,\
		g_DioComSetting.nDataBits, g_DioComSetting.nStopBits, 0);

	if (fd_com == -1)
	{
		return -1;
	}
	
	return 0;
}


void * SendSignal(void * pAg)
{

	int nRoadWayId = *(int *)pAg;

	char szSignalMsg[6] = {0};

	sprintf(szSignalMsg, "{%d/1}", nRoadWayId);
	string strMsg1(szSignalMsg);
	g_DioComSignalProtocol2.WriteSignal(strMsg1);
	
	usleep(400 * 1000);


	sprintf(szSignalMsg, "{%d/0}", nRoadWayId);
	string strMsg2(szSignalMsg);
	g_DioComSignalProtocol2.WriteSignal(strMsg2);
	
}

int DioComSignalMachine::HandelSignal(int nRoadWayId)
{

	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t pthredId1;

	if ( pthread_create(&pthredId1, &attr1, SendSignal, (void *)&nRoadWayId) != 0)
	{
		pthread_attr_destroy(&attr1);
		return -1;
	}
	pthread_attr_destroy(&attr1);
	usleep(20 * 1000);

	return 0;
}

// ·¢ËÍÐÅºÅ
int DioComSignalMachine::WriteSignal(string strMsg)
{ 
	if (fd_com == -1)
		return -1;
	
	pthread_mutex_lock(&m_ComPoseWriteLock); 

	LogNormal("WriteSignal strMsg=%s,strMsg.size=%d",strMsg.c_str(),strMsg.size());
	write(fd_com, strMsg.c_str(), strMsg.size());

	pthread_mutex_unlock(&m_ComPoseWriteLock); 

	return 0;

}





