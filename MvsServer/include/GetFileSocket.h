#ifndef _FILE_SOCKET
#define _FILE_SOCKET

#include "CSocketBase.h"

//#define _TEST_CODE

class CGetFileClientSocket:public mvCSocketBase
{
public:
	CGetFileClientSocket();
	virtual ~CGetFileClientSocket();
private:
	int m_nSocket;
	int m_nPort;
	UINT32 uTimestamp;				//时间戳
	string m_strData;
	//是否发送数据，如果在发送之前即接收数据 超过10次没有收完数据断开连接
	bool m_bSendBuf;
	int m_nRecvNum;

	
	int RecvMsg(std::string &response,int nSize);
	bool SendMsg(RECORD_PARA para);
	/*参数二：录像数据ID
	参数三：0:车牌查询，1:事件查询，2:全天录像*/
	int FindLocalPic(string &strData,RECORD_PARA para);
	//从ftp路径解析到正确的本地存储路径
	void GetCorrectLocalDir(string &strLocalPath,string strFtpPath,int nVideoType = 0);
#ifdef _TEST_CODE
	bool ReadLocalPic(char* szFile,std::string &strBuffer);
#endif
public:
	char m_chHost[16];
	bool m_bEndThread;
	
	bool Init(int nSocket,char *pchar,int nPort);
	void UnInit();
	void SetRecvBuf(string strData){m_strData = strData;}
	string GetRecvBuf(){return m_strData;}
	bool CheckClientSendState();

	int GetSocket(){return m_nSocket;}
	int DealMsg();
};

//客户端列表
typedef std::map<int,CGetFileClientSocket*> GetFileClientMap;
class CGetFileServerSocket:public mvCSocketBase
{
public:
	CGetFileServerSocket();
	virtual ~CGetFileServerSocket();
private:
	//侦听端口
	int m_nPort;
	bool m_bEndThread;
	//ClientMap信号互斥
	pthread_mutex_t m_thread_mutex;
	//客户端连接映射列表
	GetFileClientMap m_ClientMap;
	//允许连接的套接字
	int m_nPermitSocket;
	//static DWORD WINAPI ThreadAccept(LPVOID lpParam);
	//static DWORD WINAPI ThreadOnLine(LPVOID lpParam);
public:
	//侦听套接字连接
	int m_nAcceptSocket;
	//开始侦听服务
	bool Init();
	//释放
	bool UnInit();
	//添加客户端
	//bool AddClient(SRIP_CLEINT sClient);
	//删除客户端
	//bool DelClient(int nSocket,bool bAll=false);
	//添加客户端
	bool AddClient(int nClientSocket,char *pchHost,int nPort);//CMsgSocket* sMsgSocket);
	//删除客户端
	bool DelClient(int nSocket);
	bool DelAllClients();
	//检测超时
	bool CheckClientTimeOut();
};
#ifdef _TEST_CODE
extern BOOL g_bEndThread;
#endif
//通讯服务
//extern CGetFileServerSocket g_skpFileServer;
#endif