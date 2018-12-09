#ifndef RADARSERIAL_H
#define RADARSERIAL_H

#include "AbstractSerial.h"

/**
    文件：RadarSerial.h
    功能：雷达测速串口通讯类
    作者：yuwenxian
    时间：2010-9-25
**/

class CRadarSerial:public AbstractSerial
{
	public:
		CRadarSerial();
		~CRadarSerial();

    public:
        //打开设备
        bool OpenDev();
		//void Init();
		void BeginRadarSignalCapture();
		 //结束雷达采集线程
        bool EndRadarSignalCapture();
		//接收串口雷达消息
        void RecData();
        //获取速度信号函数
        int GetSpeedSignal(Speed_Signal& uSpeed);

		//接收huichang雷达串口消息
		void RecDataHuichang();

		//设置雷达类型
		void SetRadarType(int nType)
		{
			m_nRadarType = nType;
		}

		int m_nRadarType;//雷达类型 1:S3雷达 2:慧昌雷达

	private:
	    bool m_bEndRadarCapture;

	    listSpeedSignal m_listRadar;
        Speed_Signal m_sRadar;//用于记录上一次的值
        //存取互斥
        pthread_mutex_t m_speedmutex;		
};

extern CRadarSerial g_RadarSerial;
#endif
