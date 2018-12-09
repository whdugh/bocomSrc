#ifndef DIOCOMSIGNALPROTOCOL
#define DIOCOMSIGNALPROTOCOL
#include "AbstractSerial.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <memory.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/ipc.h> 
#include <unistd.h>
#include <errno.h>
#include <termios.h>

using namespace  std;
/*
 * 文件名：DioComSignalPotocol.h
 * 功能：  当对应的车道有车经过时， 会向ut5510 发送一个信号，
 * 控制ut5510 从而来给泰科的设备进行通讯， 最终目的的 计数车俩流量数目
 * 作者：niuheshan
 * 完成日期：2013 04 11
*/


class DioComSignalProtocol: public AbstractSerial
{

public:


	DioComSignalProtocol();
	~DioComSignalProtocol();

	int		OpenDev();
	int		CreatethreadForLeanNo(int RoadIndex);
	int		sendMessage(string strAnser);
	int		SetCommSpeed(int fd, int nSpeed);
	char*	pack(char buffer[]);
	char		checkSum(char *sData);
	char		strHex(char temp);
	
	static void * RoadIndex1 (void * arg);
	static void * RoadIndex2 (void * arg);
	static void * RoadIndex3 (void * arg);
	static void * RoadIndex4 (void * arg);

	void SetComPoseWriteLock(){ pthread_mutex_lock(&m_ComPoseWriteLock); }
	void SetComPoseWriteUnlock(){ pthread_mutex_unlock(&m_ComPoseWriteLock); }

	void 	ComPoseMsgAndSendMsg(int type);
	

	static int			m_fd;

	static volatile int	m_road1IsOpen;
	static volatile int	m_road2IsOpen;
	static volatile int	m_road3IsOpen;
	static volatile int	m_road4IsOpen;

	pthread_mutex_t		m_ComPoseWriteLock;
	pthread_mutex_t		m_writeMutex;
};

extern DioComSignalProtocol g_DioComSignalProtocol;

#endif 




