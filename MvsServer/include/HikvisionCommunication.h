#ifndef _HIKVISIONCOMMUNICATION_H_
#define _HIKVISIONCOMMUNICATION_H_

#include "Common.h"
#ifdef HIKVISIONCAMERA

#include "platform.h"
//using namespace Platform;

class CHikvisionCommunication
{
public:
	CHikvisionCommunication();
	~CHikvisionCommunication();

	bool Init();
	bool UnInit();

	bool UserLogOnServer();

	void LoginHikServer();
	
	int GetLoginHandle()
	{
		return m_Userhandle;
	};
	int GetInit()
	{
		return m_nInit;
	}

private:

	int m_Userhandle;//登陆返回的句柄
	int m_nInit;//初始化返回值
	bool m_bLogOnServer;

	//线程ID
	pthread_t m_nThreadId;
};

extern CHikvisionCommunication g_HikvisionCommunication;

#endif
#endif