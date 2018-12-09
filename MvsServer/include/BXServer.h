/************************************************************************/
/* 
	宝信平台通讯类
	addby-mawei 20150911
*/
/************************************************************************/
#ifndef _BXSERVER_H_
#define _BXSERVER_H_


#include <iostream>
#include <list>
#include <pthread.h>
#include <string>

extern "C"
{
	#include "BaoXinSdk.h"

	//初始化
	extern BX_Int32 BX_Init();
	//清理
	extern BX_Int32 BX_UnInit();
	//设置获取状态回调，用于获取设备状态信息
	extern BX_Int32 BX_Reg_CB_FUNS( BX_CB_FUNS cb_funs );

	//发送图片信息
	extern BX_Int32 BX_SendImage( const BX_HeadData *pbxHead,const BX_Uint8 *JpegData, BX_Uint32 picLen);
};
using namespace std;


class CBXServer
{
public:
	CBXServer();
	~CBXServer();

	int Init();

	int UnInit();

	int AddResult(string &result);

private:
	static BX_Int32 GetDeviceTime( BX_Uint64 *pU64msSecond );

	static BX_Int32 SetDeviceTime( BX_Uint64 U64msSecond );
	static BX_Int32 GetDeviceWH(BX_Uint32 *pWidth,BX_Uint32 *pHeight);
	//实时数据发送
	static void* CreateSendRealTimeResult( void *arg);

	void DealRealTimeResult();

	//历史数据发送
	static void* CreateSendHistoryResult( void *arg);

	void DealHistoryResult();
	//dataType0-实时数据，1历史数据
	int sendDataToBX(string &result,int dataType);

private:

	int m_picWidth;
	int m_picHeight;
	
	pthread_t m_SendRealTimeThrd;//实时数据发送线程id
	pthread_mutex_t m_ResultLock;//
	list<string> m_ResultList;//实时数据缓冲队列

	int m_stopSend;//线程停止标示 0-继续，1-停止


	pthread_t m_SendHistoryThrd;//历史数据发送线程id
};

extern CBXServer g_BXServer;





#endif //_BXSERVER_H_