#ifndef GPSSERIAL_H
#define GPSSERIAL_H

#include "AbstractSerial.h"

/**
    文件：GpsSerial.h
    功能：GPS校时串口通讯类
    作者：yuwenxian
    时间：2010-9-15
**/

class CGpsSerial:public AbstractSerial
{
	public:
		CGpsSerial();
		~CGpsSerial();

    public:
        //打开设备
        bool OpenDev();
		//void Init();
		void BeginThread();
		//接收串口GPS消息
        void RecData();

	private:
};

extern CGpsSerial g_GpsSerial;
#endif
