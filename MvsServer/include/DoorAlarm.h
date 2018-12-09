#ifndef DOORALARM_H
#define DOORALARM_H

#include <stdio.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <sys/types.h> 
#include <linux/kd.h> 
#include <pthread.h>
#include "AbstractSerial.h"
#include "Common.h"


class CDoorAlarm :public AbstractSerial
{
public:
	CDoorAlarm();
	~CDoorAlarm();
public:
	//打开设备
	bool OpenDev();
	void BeginThread();
	void RecData();
};

extern CDoorAlarm g_DoorAlarm;
#endif
