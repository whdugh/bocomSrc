
#ifndef YIHUALUCENTER_H
#define YIHUALUCENTER_H
#define TIMER_INTERAL (5*60)

#include "structdef.h"
#include "FtpCommunication.h"
#include "BocomServerManage.h"
#include "Common.h"
#include "CSeekpaiDB.h"
#include "RoadChannelCenter.h"

class CYiHuaLuCenter:public mvCSocketBase
{
public:
	CYiHuaLuCenter();
	~CYiHuaLuCenter();

	//启动
	bool Init();

	//释放
	bool UnInit();

	/*主线程调用接口*/
	void mvConnOrLinkTest();

	//记录发送线程
	static void *ThreadSendResult(void *pArg);

   //处理一条数据
	void DealStatusResult();

	//处理数据
	bool OnResult(std::string &strMsg);
	
	//连接中心端
	bool mvConnectToCS();

	//发送记录到中心端
	bool mvSendRecordToCS(const std::string &strMsg);

	/*重组并发送消息*/
	bool mvRebMsgAndSend(int& nSocket, const std::string &strMsg);

	int getTime();


	//获取通道个数
	int GetChannelCount();

	//发送公交相机连接状态
	int SendCameraStatusOfGJMode(std::string strCameraID,CameraState state);
private:

	int m_nSendSocket;

	//监听线程ID
	pthread_t m_nThreadId;
    bool m_bEndThread;

	bool m_bCenterLink;
	
};
extern CYiHuaLuCenter g_YiHuaLuCenter;

#endif




