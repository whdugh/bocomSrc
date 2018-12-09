#ifndef MONITORINGANDALARM_H
#define MONITORINGANDALARM_H

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
#include "XmlParaUtil.h"
#include "AbstractSerial.h"
#include "Common.h"




extern int isChoiceAlarm;

class MonitoringAndAlarm :public AbstractSerial
{
public:
	int m_freq;			/* 我们期望输入的频率Hz */ 
	int m_length;		    /* 发生的长度，以微妙为单位*/ 
	int m_reps;           /* 重复的次数*/ 
	int m_delay;          /* 两次发声的间隔，微妙为单位u*/ 

	//int fd_com;
	int m_fd_alarm;

	pthread_t m_nThreadId ;

	MonitoringAndAlarm(int freq = 440, int length = 200, int reps = 1, int delay = 100 );
	~MonitoringAndAlarm();

	int OpenDev();    // 初始化报警设备

	int beginAlarm();  // 开始报警

	int endAlarm();    // 结束报警

	int sendPack();    // 发送包

	char *pack(char buffer[]);     // 进行对发送命令的打包

	char checkSum(char *sData);    // 对发送命令的校验和进行计算

	char strHex(char temp);        // 

	bool iniAlarmThread();          // 启动报警的线程


	//接收串口消息
	void RecData();



protected:

private:

};

void * ThreadMonitoringAndAlarm(void * arg);

extern MonitoringAndAlarm g_AlarmSerial;
#endif
