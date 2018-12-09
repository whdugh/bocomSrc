#ifndef MVSTOSERIALSERVER_H
#define MVSTOSERIALSERVER_H

#include "CSocketBase.h"
#include <pthread.h>

class CMvsToSerialServer:public mvCSocketBase
{
public:
	CMvsToSerialServer();
	~CMvsToSerialServer();

	bool compareMsgTime();

	bool compareTime();

	void mvConnOrLinkTest();

	void mvRecvSerialMsg();

	bool testConnectThread();   // 开启检测连接的问题

	void testConnetServerGetStatus();  //隔一定的时间去连接服务器，得到连接的状态
	void testCatGetMessage();         // 判断得到信息的标志是否改变
	void setSockFd(int sockfd);                 // 设置socke描述符
	int getSokeFd();                 //得到socke描述符
	int m_testThread;
	int m_testConnSerialSock;
	pthread_t m_testURecvSSMsgThreadId;
	pthread_t m_testCanFlagThreadId;	
	static int m_catGet;
	static int m_getMsgTime;
	pthread_mutex_t m_sockefdLock;
private:

	bool mvConnSerialAndRecvMsg();

	bool mvConnToSerial();

	bool theThreadStart();
	

	UINT32 m_uRecvserialmsgtime;

	int m_SerialSocket;
	pthread_t m_uRecvSSMsgThreadId;
	

	pthread_mutex_t m_mvtossMsg;

	string m_strSerialHost;
	int m_nSerialPort;

	vector<int> m_vRemotePreSet;
};
extern CMvsToSerialServer g_MVSToSerialServer;

#endif
