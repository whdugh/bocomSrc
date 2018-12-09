#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <string>
#include <iostream>
#include <termios.h>

//#include "matrix_wdt.h"
//#include "conio.h"
//#include "pciioctl.h"
#include "DioConnet.h"
#include "Common.h"///////////////////////////

using namespace std;

volatile int roadIndex1 = 0;
volatile int roadIndex2 = 0;
volatile int roadIndex3 = 0;
volatile int roadIndex4 = 0; 

DioConnect g_dioConnect;

void * SendDioSignal(void * pAg)
{
	int nRoadWayId = *(int *)pAg;
	

	if (nRoadWayId == 1)
	{
		roadIndex1 = 1;
	}
	else if (nRoadWayId == 2)
	{
		roadIndex2 = 1;
	}
	else if (nRoadWayId == 3)
	{
		roadIndex3 = 1;
	}
	else if (nRoadWayId == 4)
	{
		roadIndex4 = 1;
	}
	
	int nVal = 15;
	if(roadIndex1 == 1)
	{
		nVal -= 1;
	}
	if(roadIndex2 == 1)
	{
		nVal -= 2;
	}
	if(roadIndex3 == 1)
	{
		nVal -= 4;
	}
	if(roadIndex4 == 1)
	{
		nVal -= 8;
	}
	//char szSignalMsg[5] = {0};

	//sprintf(szSignalMsg, "{%d/1}", nRoadWayId);
	g_dioConnect.GPO_Write(&nVal);
	//cerr<<szSignalMsg<<"\n";
	//g_dioConnect.GPO_Write(szSignalMsg);
	
	
	usleep(400 * 1000);
	

	if (nRoadWayId == 1)
	{
		roadIndex1 = 0;
	}
	else if (nRoadWayId == 2)
	{
		roadIndex2 = 0;
	}
	else if (nRoadWayId == 3)
	{
		roadIndex3 = 0;
	}
	else if (nRoadWayId == 4)
	{
		roadIndex4 = 0;
	}

	nVal = 0;
	if(roadIndex1 == 0)
	{
		nVal += 1;
	}
	if(roadIndex2 == 0)
	{
		nVal += 2;
	}
	if(roadIndex3 == 0)
	{
		nVal += 4;
	}
	if(roadIndex4 == 0)
	{
		nVal += 8;
	}
	
	//char szSingMs2[5] = {0};	
	//sprintf(szSingMs2, "{%d/0}", nRoadWayId);
	//cerr<<szSingMs2<<"\n";
	g_dioConnect.GPO_Write(&nVal);
	//g_dioConnect.GPO_Write(szSingMs2);
	
	
	
}

DioConnect::DioConnect()
{
	m_initFlag = 0;
	m_DevFd = -1;

	pthread_mutex_init(&m_ComPoseWriteLock, NULL);
}

DioConnect::~DioConnect()
{
	pthread_mutex_destroy(&m_ComPoseWriteLock);
}

// 打开设备！
short DioConnect::GPIO_Init(void)
{
	int fd;
	char driverStr[256];
	unsigned short dev_number = 0;

	if(!m_initFlag) 
	{
		sprintf(driverStr,"/dev/watchdog");
		fd = open( driverStr, O_RDWR );
		if( fd < 0 ) 
		{
			//LogNormal("Dio intit 打开失败");
			return -1;
		}
		m_DevFd=fd;
		m_initFlag=1;
	}
	else
	{
		fd = m_DevFd;
	}
	int nVal = 0x0F;
	GPO_Write(&nVal);
	//LogNormal("Dio 初始化成功");
	return 0;
}

// 关闭设备！
void DioConnect::GPIO_unInit(void)
{
	if(m_DevFd >= 0)
	{
		close(m_DevFd);
		m_DevFd = -1;
	}
}

// 根据车道号 
int DioConnect::CreatethreadForLeanNo(int RoadIndex)
{
	

	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t pthredId1;
	if (pthread_create(&pthredId1, NULL, SendDioSignal, (void *)&RoadIndex) != 0)
	{
		pthread_attr_destroy(&attr1);
		return -1;
	}
	pthread_attr_destroy(&attr1);
	usleep(1000 * 1000);
	
	return 0;
}

// 发送信号
short DioConnect::GPO_Write(void* pState)
{
	int fd;    

	if(!m_initFlag) 
		return -1;
	
	pthread_mutex_lock(&m_ComPoseWriteLock); 
	fd = m_DevFd;
	LogNormal("GPO_Write pState=%s",pState);
	cerr<<pState<<"\n";
	if ( ioctl(fd, DAS_IOC_SET_GPO, pState) < 0 )
	{
	
		pthread_mutex_unlock(&m_ComPoseWriteLock); 
		printf("DAS_IOC_SET_GPO Error\n");
		return -1;
	}	
	pthread_mutex_unlock(&m_ComPoseWriteLock); 

	return 0;

}

