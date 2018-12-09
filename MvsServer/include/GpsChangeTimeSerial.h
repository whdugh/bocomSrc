#ifndef   GPSCHANGETIMESERIAL_H
#define   GPSCHANGETIMESERIAL_H

/*
	文件 GpsChangeTimeSerial.h
	功能：GPS校时串口通讯类
	作者：
	时间： 20120925
*/

#include "AbstractSerial.h"
#include "Common.h"

class GpsChangeTimeSerial:public AbstractSerial
{
public:
	GpsChangeTimeSerial();
	~GpsChangeTimeSerial();

public:
	//打开设备
	bool OpenDev();
	//void Init();
	void BeginThread();
	//接收串口GPS消息
	void RecData();

	long gettimefromString(char  str[], int l);

private:
	string m_nHour;
};

extern GpsChangeTimeSerial g_GpsChangeTime;

#endif 
