#ifndef _VMUX_SOCKET_H_
#define _VMUX_SOCKET_H_

#define VMUX_PROC

#include "Common.h"
#include "CSocketBase.h"

#pragma pack(1)
//视频数据协议头
typedef struct tagVissHead
{
	char m_Heads[4];		//标志头 "0x3E3E3E3E"
	char m_ChipType;		//芯片类型
	char m_PackType;		//封装格式,A/V,Version
	char m_FrameRate;		//帧率
	char m_CompressType;	//压缩格式，帧类型，P/N
	short m_DataRate;		//码率
	short m_SizeH;			//分辨率H
	short m_SizeV;			//分辨率V
	short m_PackSize;		//数据包长度,包含40字节的头的长度
	short m_GOP;			//GOP信息
	short m_PackCount;		//包数量
	short m_PackID;			//包序号
	int m_FrameID;			//帧ID（L）
	short m_Reserve1;		//保留1
	short m_Reserve2;		//保留2
	short m_CheckSum;		//校验
	unsigned int m_TimeStampSec;	//时间戳（秒）
	unsigned int m_TimeStampuSec;	//时间戳（微秒）
	short m_MultiP;//倍率
	char m_InforChar[128];//其他字符信息
}VissHead;
//控制协议头
typedef struct tagCtlHead
{
  unsigned char m_Heads[2];//标志头0xBFEC
  unsigned char m_version;//版本号0x1b
  unsigned char m_catalog;//ACK DIR CATALOG
  unsigned char m_cmd;//命令类型
  unsigned char m_lenH;
  unsigned char m_lenV;
  unsigned char m_CheckSum;
  tagCtlHead()
  {
	  m_Heads[0] = 0xBF;
	  m_Heads[1] = 0xEC;
	  m_version = 0x1B;
	  m_catalog = 0;
	  m_cmd = 0;
	  m_lenH = 0;
	  m_lenV = 0;
	  m_CheckSum = 0;
  }
}CtlHead;
#pragma pack()

typedef struct _ClientInfo
{
	char chHost[16];
	int nPort;
	short status;//0 关闭 1 开启
	_ClientInfo()
	{
		memset(chHost, 0, 16);
		nPort = 4501;
		status = 0;
	}

}ClientInfo;


#pragma pack(1)
//消息体的前四个字节
typedef struct _BODY_SYNC
{
	short slot;
	unsigned char channelId;
	unsigned char streamId;
	_BODY_SYNC()
	{
		slot = 0;
		channelId = 0;
		streamId = 0;
	}
}BodySync;

//ACK返回
typedef struct _RET
{
	short slot;
	short channelId;
	int ack;//0 成功，小于0代表错误
	_RET()
	{
	  slot = 0;
	  channelId = 0;
	  ack = -5;
	}
}AckRet;
#pragma pack()

#ifdef VMUX_PROC

class CVmuxSocket : public mvCSocketBase
{
public:
	~CVmuxSocket();
	static CVmuxSocket * GetInstance()
	{
		return _vmux;
	}
	bool Init();
	//处理连接
	void OnAccept();
	//发送数据
	void SendMsg(string& msg);
   //处理接收数据
	void OnReceive(int nSocket);
	//处理数据
	void OnDeal();
	//处理视频数据连接
	void OnDataAccept();
	//释放资源
	void Unit();
private:
	CVmuxSocket();
	//接收连接
   static void * ThreadAccept(void * param);
   //接收数据
   static void * ThreadRecv(void * param);
   //处理数据
   static void * ThreadDeal(void *param);
   //向某个连接发送消息
   bool Send(int nSocket, const char *szBuf, int nLength);
   //从连接队列中清除一个连接
   bool DelOneClient(int nSocket);
   //处理消息
   bool OnMessage(int nSocket, string& msg);
   int CheckSum(const CtlHead* head);

   /////////////////////////
   //接收视频数据连接
   static void * ThreadDataAccept(void * param);
   //开启端口建立连接
   bool CreateSocket(int& nSocket, int nPort);

	static CVmuxSocket* _vmux;

	pthread_mutex_t m_thread_mutex;
	pthread_mutex_t m_DataMutex;
	pthread_mutex_t m_Mutex;

	//侦听套接字
	int m_nAcceptSocket;
	//侦听端口
	int m_nPort;

    multimap<int, string> m_msgMap;//消息映射

	std::vector< pair<int, ClientInfo> > m_vecSocket;//所有连接的SOCKET


	int m_nDataSocket;//视频数据连接
	int m_nDataPort;//视频数据端口

	vector<int> m_vecDataSocket;//存放所有数据连接的socket

	bool m_bEndStream;//是否中止码流
	bool m_bEndThread;//是否中止线程
};
#endif

#endif