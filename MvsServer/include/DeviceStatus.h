#ifndef DEVICESTATUS_H
#define DEVICESTATUS_H

#include "AbstractSerial.h"
#include "Common.h"
#include "CSeekpaiDB.h"
#include <fcntl.h> 
#include <stdlib.h> 
#include <string> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <sys/types.h> 
#include <linux/kd.h> 
#include <stdio.h>
#include <sys/vfs.h>

using namespace std;

/*
	文件  Devicestatus.h
	功能：检查设备运行的状态，并且把设备的状态保存到  DEVICE_STATUS 表中
	作者：牛河山
	时间：20121115
*/


class DeviceStatus
{
public:

	DeviceStatus();                            // 构造函数

	~DeviceStatus();

	void Init();    // 启动线程 

	int GetMessageAndSave();                  //得到一些数据 然后把这些数据保存到 DEVICE_STATUS这个表中

	int ProgramStart();                       //程序刚刚启动的时候 ，会更新一次

	int OneHourCheckDisk();                   // 当一小时的时候会检测下磁盘剩余状态

	int GetDiskFree();                        // 得到硬盘的剩余空间

	int DeleMessage();                        // 只保存一个月的数据    
	
	string  Dispose(string &str );            // 格式化ip地址为制定的格式
		
		
	static int m_saitci_ch_ok;                // 这是个静态的变量 0 ntp 校时失败， 1 ntp 校时成功。

	string m_idAddr;       // ip地址是否发生变化
	int m_changeIsok;      // ntp 校时是成功没有 
	int m_diskSurplus;       // 磁盘空间剩余


protected:

private:

};

void *  ThreadCheckupDeviceStatus();

extern DeviceStatus g_CheckupDeviceStatus;

#endif