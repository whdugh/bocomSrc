#ifndef _C_USB_KEY_IMP_H
#define _C_USB_KEY_IMP_H

#include <string>

class UsbKeyLockWrite
{
public:
	UsbKeyLockWrite();
	~UsbKeyLockWrite();

	//打开usbkey设备
	bool OpenUsb(int port = 0);
	//关闭usbkey设备
	bool CloseUsb();
	//写数据到usbkey设备
	bool WriteUsb();

private:
	//修改管理员用户和普通用户密码,retries为输入密码错误时最多尝试的次数：-1，表示无尝试，（1~15）次,默认3次
	int SetPwd(unsigned char * adminPwd, unsigned char * userPwd, int retries = 6);

private:
#if defined _X64_ || defined __x86_64__
	 int m_inHandle;
#else
	 int m_inHandle;
#endif
};

#endif