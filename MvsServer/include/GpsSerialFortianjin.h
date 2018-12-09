#ifndef   GPS_SERIAL_FOR_TIANJIN_H
#define   GPS_SERIAL_FOR_TIANJIN_H

/*
	文件 CGpsSerialFortianjin.h
	功能：GPS校时串口通讯类
	作者：
	时间： 20131112
*/

#include "AbstractSerial.h"
#include "Common.h"

class CGpsSerialFortianjin:public AbstractSerial
{
public:
	CGpsSerialFortianjin();
	~CGpsSerialFortianjin();

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

extern CGpsSerialFortianjin g_GpsSerialFortianjin;

#endif 
