#include "DioAlarm.h"
#include "Common.h"
#include "CommonHeader.h"

using namespace std;

DioAlarm g_dioAlarm;

int DioAlarm::m_exit = 0;

DioAlarm::DioAlarm()
{
	bAlarm = false;
}

DioAlarm::~DioAlarm()
{
	DioAlarm::m_exit = 1;
}

int DioAlarm::InitDio()
{
	pthread_t m_hId = 0;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	if(pthread_create(&m_hId, &attr, BeginCheckDio, this) != 0)
	{
		printf("创建线程失败。");
		return -1;
	}
	pthread_attr_destroy(&attr);
	return 0;
}

threadfunc_t STDPREFIX DioAlarm::BeginCheckDio(threadparam_t lpParam)
{
	DioAlarm *pThis = (DioAlarm *)lpParam;
	pThis->CheckDioInput();
	return NULL;
}

void DioAlarm::CheckDioInput()
{
	time_t startTime;
	time_t nowTime;
	time(&startTime);
	while(DioAlarm::m_exit == 0)
	{
		time(&nowTime);
		int offTime = (nowTime-startTime);
		if(offTime >= 1)
		{
			Read_GRIO();
			startTime = nowTime;
		}
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
	pthread_exit((void*)0);
}

int DioAlarm::Read_GRIO()
{
	int ret=iopl(3);
	if(ret<0)
	{
		return -1;
	}
	
	int result = -1;
	result=inb(0xa40);//读取值的低4位对应gpio1-4口
	int bit1 = (result>>1)&0x01;

	if (bit1 == 0 && bAlarm == false)// First Alarm 开门报警
	{
		g_ExpoMonitorInfo.nGateValue = 1;
		g_MyCenterServer.mvCheckDeviceStatus(2);
		LogNormal("DIO Bit1:开门报警\n");
		bAlarm = true;
	}
	else if (bit1 == 1 && bAlarm == true) // clean Alarm  关门报警
	{
		g_ExpoMonitorInfo.nGateValue = 0;
		g_MyCenterServer.mvCheckDeviceStatus(2);
		LogNormal("DIO Bit1:关门报警\n");
		bAlarm = false;
	}

	ret=iopl(0);
	if(ret<0)
	{
		return -1;
	}
	return 0;
}

