#include "AbstractSerial.h"
#ifndef REDLIGHTSERIAL_H
#define REDLIGHTSERIAL_H
class RedLightSerial:public AbstractSerial
{
	public:
		RedLightSerial();
		~RedLightSerial();
		 //打开设备
         bool OpenDev();

        //捕获红灯信号
        void CapureRedSignal();
		void CapureRedSignal1();
        //获取红灯信号函数
        int GetRedSignal(unsigned short& uTrafficSignal);

        //启动红灯线程
        bool BeginRedSignalCapture();
        //结束红灯线程
        bool EndRedSignalCapture();

	protected:

	private:
        //红灯信号保存变量
        unsigned short m_uTrafficSignal;
        //存取互斥
        pthread_mutex_t m_redmutex;

        bool m_bEndTsCapture;

};
extern RedLightSerial r_SerialComm;
#endif
