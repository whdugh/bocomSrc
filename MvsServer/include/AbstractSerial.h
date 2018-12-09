#ifndef ABSTRACTSERIAL_H
#define ABSTRACTSERIAL_H
#include "global.h"

class AbstractSerial
{
	public:
		AbstractSerial();
		~AbstractSerial();
        //打开设备
        bool OpenDev();

        bool IsOpen();

        //关闭设备
        bool Close();

	protected:
        //打开串口
       int open_port(int comport,int nSpeed,int nDataBit=8,int nStopBit=1,int nParity=0, bool bVTime=false);

        //设置串口波特率
       void set_speed(int fd,int nSpeed);
        //设置奇偶校验位
        int set_Parity(int fd,int databits,int stopbits,int parity);
        int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop,bool bVTime=false);
        //打开文件
        int fd_com;

	private:
};

#endif
