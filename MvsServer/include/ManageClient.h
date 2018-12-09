#ifndef ManageClient_H
#define ManageClient_H

#include "ThreadLock.h"
#include "dspUnit.h"
#include "structdef.h"


class CDspUnit;
using namespace BASE;

class CManageClient{
public:
	CManageClient();
	~CManageClient();

	// 增加一个DSP连接
	bool AddDspClient(const DspSocketFd dspSocket);

	// 删除一个Dsp连接
	bool MoveDspClient(const DspSocketFd dspSocket);
	

	// 打印当前Dsp连接信息
	int BeginRealRunInfoThread();
	static threadfunc_t STDPREFIX BeginInfoFunc(threadparam_t lpParam);
	void InfoFunc();
	bool CleanMap();

	//断开指定socket连接
	bool DisConnectClient(const DspSocketFd &dspSocket );

	//关闭套接字
	bool CloseDspUnitFd(CDspUnit* lp);

	bool GetDspList(DSP_LIST &dsplist);

private:
	CDspUnit* IsExistDspClient(char* szAddress);
	void PrintDspClientInfo();

private:
	ThreadLock m_DspClientMapLock;

	map<string, CDspUnit*> m_DspClientMap;
	static int               m_exit;//程序退出标识

	bool m_bPrintThStart;
};

//全局调用
//extern CManageClient g_ManageClients;

#endif