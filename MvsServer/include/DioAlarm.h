#ifndef DIOALARM_H
#define DIOALARM_H

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
#include <sys/io.h>
#include "Thread.h"

class DioAlarm
{
public:
	DioAlarm();
	~DioAlarm();

	int InitDio();
	static threadfunc_t STDPREFIX BeginCheckDio(threadparam_t lpParam);
	void CheckDioInput();

private:
	int Read_GRIO();

	static int               m_exit;//程序退出标识
	bool bAlarm;
};
extern DioAlarm g_dioAlarm;

#endif