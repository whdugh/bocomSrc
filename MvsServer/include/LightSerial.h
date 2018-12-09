#include "AbstractSerial.h"
#ifndef LIGHTSERIAL_H_INCLUDED
#define LIGHTSERIAL_H_INCLUDED
class LightSerial:public AbstractSerial
{
    public:
        LightSerial();
        ~LightSerial();
        	 //打开设备
        bool OpenDev();
            //脉冲间隔
        bool SetPulsInter(int nSpace);
        //脉冲宽度
        bool SetPulsWidth(int nWidth);
            //设置开关灯
        bool SetOpenAndHost(bool bOpen, bool bHost);
            //设置频率
        bool SetFrequency(int nFren);
           //调节脉宽
        bool AdjustPuls(int nWidth);

    private:
            //调节频率
        bool AdjustFrequency(int nFren);
            //调节分频计数器
        bool AdjustCount(int nFrenCount);

    private:
        int  m_nCount;
};
extern LightSerial L_SerialComm;


#endif // LIGHTSERIAL2_H_INCLUDED
