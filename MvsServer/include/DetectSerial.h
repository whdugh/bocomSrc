#include "AbstractSerial.h"
#ifndef DETECTSERIAL2_H
#define DETECTSERIAL2_H
#define MaxCount 16777216
#define MAXREAD  300
class CDetectSerial:public AbstractSerial
{
    public:
        CDetectSerial();
        ~CDetectSerial();
         //打开设备
        bool OpenDev();
        //捕获速度信号
        void CapureSpeedSignal();
        //获取速度信号函数
        int GetSpeedSignal(Speed_Signal& uSpeed);

        //启动线圈采集线程
        bool BeginLoopSignalCapture();
        //结束线圈采集线程
        bool EndLoopSignalCapture();

    protected:
    private:
    //车速信号保存变量
        //unsigned short m_uTrafficSpeed;
        listSpeedSignal m_listLoop;
        Speed_Signal m_sLoop;//用于记录上一次的值
        //存取互斥
        pthread_mutex_t m_speedmutex;

        bool m_bEndDlCapture;
};
extern CDetectSerial d_SerialComm;
#endif // DETECTSERIAL2_H
