#include "Common.h"
#include "CommonHeader.h"
#include "GetFileSocket.h"


#define SRIP_MAX_BUFFER 1024
#define MAX_WAIT_RECV_TIME 1000

#ifdef _TEST_CODE
//程序主线程中止标识
BOOL g_bEndThread = FALSE;
#endif

//通讯服务
//CGetFileServerSocket g_skpFileServer;

CGetFileClientSocket::CGetFileClientSocket()
{
	m_nSocket = 0;
	memset(m_chHost,0,16);
	m_nPort = 0;
	m_bEndThread = false;
	m_bSendBuf = false;
	m_nRecvNum = 0;
}

CGetFileClientSocket::~CGetFileClientSocket()
{
	UnInit();
}

//客户端接收发送线程
void* ThreadDeal(void* pArg)
{
	CGetFileClientSocket* pSocket = (CGetFileClientSocket*)pArg;
	if(pArg == NULL) return 0;
	pSocket->DealMsg();
	pSocket->UnInit();

	pthread_exit((void *)0);
	return pArg;
}

bool CGetFileClientSocket::Init( int nSocket,char *pchar,int nPort )
{
	m_nSocket = nSocket;
	memcpy(m_chHost,pchar,16);
	m_nPort = nPort;

	//启动接收线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//启动监控线程
	int nret=pthread_create(&id,&attr,ThreadDeal,this);

	//成功
	if(nret!=0)
	{
		//DelClient(nClientSocket);
		//失败
		LogError("创建客户端接收数据线程失败,连接断开!\r\n");
		return false;
	}
	pthread_attr_destroy(&attr);
	usleep(100);
	
	return true;
}

void CGetFileClientSocket::UnInit()
{
	m_bEndThread = true;
	m_bSendBuf = false;
	m_nRecvNum = 0;
	if(m_nSocket != 0)
	{
		mvCloseSocket(m_nSocket);
		m_nSocket = 0;
	}
}

int CGetFileClientSocket::DealMsg()
{
	string strData;
	int nPara = sizeof(RECORD_PARA);
	if(RecvMsg(strData,nPara) == -1) return -1;

	m_bSendBuf = true;
	RECORD_PARA para;
	memcpy(&para,strData.c_str(),nPara);
	if(para.uMsgCommandID != GET_VIDEO_REQ)
	{
		LogError("下载端口：收到错误的请求数据\r\n");
		return -1;
	}
	if(SendMsg(para) == false)
	{
		LogError("下载端口：发送正确的数据失败\r\n");
		return -1;
	}
	//LogError("下载端口：已经发送正确的数据\r\n");
	//printf("数据正确处理完毕\n");
	//m_bEndThread = true;
	return 1;
}

int CGetFileClientSocket::RecvMsg(std::string &response,int nSize)
{
	if(nSize < 0) return -1;
	int nBytes = 0;
	bool bDisConnect = false;
	//接收数据设置
	char chBuffer[92];
	
	int nLeft = nSize;
	while(nLeft>0)
	{
		if (m_bEndThread == true) 
		{
			bDisConnect = true;
			break;
		}
		//接收后面的数据
		memset(chBuffer,0, 92);
		if((nBytes = recv(m_nSocket,chBuffer,nLeft,0)) <= 0)
		{
			//断开连接
			if(g_bEndThread)
			{
				bDisConnect = true;
				break;
			}

			LogError("接收协议头出错，连接断开! socket = %d,%s\r\n", m_nSocket, strerror(errno));
			bDisConnect = true;

		}
		else
		{
			//保存数据
			response.append(chBuffer,nBytes);
			nLeft -= nBytes;
		}
		//1毫秒
		usleep(1000);
	}
	if(bDisConnect == true || m_bEndThread == true)
		return -1;
	return 0;
}


bool CGetFileClientSocket::SendMsg(RECORD_PARA para)
{
	string strLocalPic;
	int nCode = FindLocalPic(strLocalPic,para);

	para.uMsgCode = nCode;
	para.uMsgLen = strLocalPic.size();
	para.uID = GET_VIDEO_REP;
	strLocalPic.insert(0,(char *)&para,sizeof(RECORD_PARA));
	//LogError("下载端口：已经发送长度%d\r\n",para.uMsgLen);
	if(!mvSendMsgToSocket(m_nSocket,strLocalPic))
		return false;
	return true;
}

//接口取录像文件
int CGetFileClientSocket::FindLocalPic( string &strData,RECORD_PARA para)
{
	string strFtpPath;
	g_skpDB.GetVideoSaveString(strFtpPath,para.uID,para.uType);
	
	//LogError("下载端口：录像未完成UID:%d---Type%d\r\n",para.uID,para.uType);
	if(strFtpPath.size()<= 0)
		return SRIP_ERROR_UNFINISH ;
	
	if(para.uType != 2)
	{
		string strLocalPath;
		GetCorrectLocalDir(strLocalPath,strFtpPath,para.uType);
		strData = GetImageByPath(strLocalPath);
		if(strData.size()<=0)
			return SRIP_ERROR_NOEXIST;
		//LogError("下载端口：地址---%s\r\n",strLocalPath);
	}
	else
		strData = GetImageByPath(strFtpPath);
	if(strData.size()<=0)
		return SRIP_ERROR_NOEXIST;

	return SRIP_OK;


#ifdef _TEST_CODE
	UINT32 uID = para.uID;
	UINT32 uType = para.uType;
	char chfile[260] = {0};
	sprintf(chfile,".\\%d.avi",uID);
	ReadLocalPic(chfile,strData);
	if(strData.size()<1)
		return 0x00000061;
#else
	//
#endif
	return 0;
}

//录象结束通知(nVideoType ＝ 0：事件录像，1：全天录像，2：违章录像)
void CGetFileClientSocket::GetCorrectLocalDir(string &strLocalPath,string strFtpPath,int nVideoType)
{
	strLocalPath = strFtpPath;
	
	//除掉人为增加的ftp
	String strPath = "ftp://"+g_ServerHost;
	strLocalPath.erase(0,strPath.size());

	String strTmpPath;;
	if(g_nServerType == 13 && g_nFtpServer == 1)
	{
		strTmpPath = "/home/road/dzjc";
		if (IsDataDisk())
		{
			strTmpPath = "/detectdata/dzjc";
		}
	}
	else if(g_nServerType == 7)
	{
		strTmpPath = "/home/road/video";
		if(nVideoType == 1)
		{
			if (IsDataDisk())
			{
				strTmpPath = "/detectdata/video";
			}
		}
		else
		{
			strTmpPath = "/home/road/red";
			if (IsDataDisk())
			{
				strTmpPath = "/detectdata/red";
			}
		}

	}
	else
	{
		strTmpPath.append((char*)g_strVideo.c_str(),g_strVideo.size());
	}

	strLocalPath = strTmpPath + strLocalPath;
}

#ifdef _TEST_CODE
bool CGetFileClientSocket::ReadLocalPic( char* szFile,std::string &strBuffer )
{

	std::string request,response;
	const int MAX_BUF_SIZE = SRIP_MAX_BUFFER*10;
	unsigned char bufArry[MAX_BUF_SIZE];//初始化包
	FILE *fp;
	if((fp=fopen(szFile,"rb"))==NULL)
		return FALSE;
	fseek(fp,0,SEEK_END);
	int nBufSize = ftell(fp); 
	fseek(fp,0,SEEK_SET);

	//10k一次读文件并一包发送
	int nDisSend = nBufSize;
	while (nDisSend > MAX_BUF_SIZE)
	{
		memset(bufArry, 0, MAX_BUF_SIZE);
		int i = fread(bufArry, MAX_BUF_SIZE/10, 10, fp);
		strBuffer.append((char*)bufArry, MAX_BUF_SIZE);
		nDisSend -= MAX_BUF_SIZE;
	}
	memset(bufArry,0,MAX_BUF_SIZE);
	int i = fread(bufArry, 1, nDisSend,fp);
	strBuffer.append((char*)bufArry,nDisSend);
	fclose(fp);

	return true;
}
#endif

//true:没有检测到异常。false：异常。
bool CGetFileClientSocket::CheckClientSendState()
{
	if(m_bSendBuf == false)
	{
		if(m_nRecvNum < 10)
			m_nRecvNum++;
		else
		{
			m_nRecvNum = 0;
			return false;
		}
	}
	return true;
}



//*********服务器监听****************

CGetFileServerSocket::CGetFileServerSocket()
{
	m_bEndThread = TRUE;
	m_nAcceptSocket = 0;
	m_nPermitSocket = -1;
	pthread_mutex_init(&m_thread_mutex,NULL);
}

CGetFileServerSocket::~CGetFileServerSocket()
{
	pthread_mutex_destroy(&m_thread_mutex);
	UnInit();
}

//监控线程
void* ThreadAcceptGetFile(void* pArg)
{
	CGetFileServerSocket* pSocket = (CGetFileServerSocket*)pArg;
	int nSocket = pSocket->m_nAcceptSocket;
	
	struct sockaddr_in clientaddr;
	//长度
	socklen_t sin_size = sizeof(struct   sockaddr_in);
	int nClient = 0;
	while(!g_bEndThread)
	{				
		nClient = accept(nSocket,(struct sockaddr*)&clientaddr,&sin_size);

		//接受连接
		if(nClient < 0)
		{
			//断开连接
			if(g_bEndThread)
			{
				//		    LogNormal("11 accept exit\r\n");
				pthread_exit((void *)0);
				return pArg;
			}
			//		LogNormal("accept nClient = %d\r\n",nClient);

			//printf("=accept  nClient = %d\r\n", nClient);

			//自动重启
			continue;
		}
		else if(nClient == 0)
		{
			close(0);
			int fd=open("/dev/tty",O_RDONLY);
			int flags = fcntl(fd, F_GETFD);
			flags |= FD_CLOEXEC;
			fcntl(fd, F_SETFD, flags);
			LogNormal("=TT=fd=%d=\n", fd);

			if(fd != 0)
			{
				close(fd);
			}			

			continue;
		}
		else
		{}


		//输出用户连接
		/*LogNormal("下载文件:来自IP:%s--nClient = %d,端口:%d连接!\r\n", \
			inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));*/

		pSocket->AddClient(nClient,inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));

		//10毫秒
		usleep(1000*10);
	}
	LogNormal("41050客户端连接服务关闭\n");
	pthread_exit((void *)0);
	return pArg;
}

//心跳检测线程
void* ThreadOnLine(void* pArg)
{
	CGetFileServerSocket* pSocket = (CGetFileServerSocket*)pArg;
	if(pArg == NULL) return pArg;
	while(!g_bEndThread)
	{
		//100秒检测一次
		sleep(100);
		//检测状态一次
		pSocket->CheckClientTimeOut();
	}
	pthread_exit((void *)0);
	return pArg;
}


bool CGetFileServerSocket::Init()
{
	m_bEndThread = FALSE;
	//创建套接字
	if(mvCreateSocket(m_nAcceptSocket,1)==false)
	{
		LogError("下载文件:创建套接字失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//重复使用套接字
	if(mvSetSocketOpt(m_nAcceptSocket,SO_REUSEADDR)==false)
	{
		LogError("下载文件:设置重复使用套接字失败,服务无法启动!,%s\r\n",strerror(errno));
		g_bEndThread = true;
		return false;
	}

	//绑定服务端口
	m_nPort = VIDEO_PORT;
	if(mvBindPort(m_nAcceptSocket,m_nPort)==false)
	{
		LogError("下载文件:绑定到 %d 端口失败,服务无法启动!\r\n",m_nPort);
		//printf("%s\n",strerror(errno));
		g_bEndThread = true;
		return false;
	}

	//开始监听
	if (mvStartListen(m_nAcceptSocket) == false)
	{
		LogError("下载文件:监听连接失败，服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动事件监控线程
	int nret=pthread_create(&id,&attr,ThreadAcceptGetFile,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("下载文件:创建事件接收线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//启动心跳检测线程
	nret=pthread_create(&id,&attr,ThreadOnLine,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("下载文件:创建连接心跳检测线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}
    //LogError("启动下载文件端口!\r\n");
	pthread_attr_destroy(&attr);

	return true;
}

bool CGetFileServerSocket::UnInit()
{
	m_bEndThread = TRUE;
	DelAllClients();
	//关闭连接
	if(m_nAcceptSocket>0)
		mvCloseSocket(m_nAcceptSocket);
	m_nAcceptSocket = 0;
	m_nPermitSocket = -1;
	return true;
}

//添加客户端
bool CGetFileServerSocket::AddClient(int nClientSocket,char *pchHost,int nPort)
{
	//必须先添加到列表
	pthread_mutex_lock(&m_thread_mutex);

	GetFileClientMap::iterator it = m_ClientMap.find(nClientSocket);
	if (it != m_ClientMap.end())
	{
		//关闭连接
		delete it->second;
		m_ClientMap.erase(it);

		
	}

	CGetFileClientSocket *pMsgSocket = new CGetFileClientSocket; 
	pMsgSocket->Init(nClientSocket,pchHost,nPort);
	m_ClientMap.insert(GetFileClientMap::value_type(nClientSocket, pMsgSocket));
	pthread_mutex_unlock(&m_thread_mutex);

	return true;
}

bool CGetFileServerSocket::DelClient( int nSocket )
{
	GetFileClientMap::iterator it = m_ClientMap.find(nSocket);
	if(it != m_ClientMap.end())
	{
		//删除连接
		delete it->second;
		m_ClientMap.erase(it);
		return true;
	}
	return false;
}

bool CGetFileServerSocket::DelAllClients()
{
	pthread_mutex_lock(&m_thread_mutex);
	GetFileClientMap::iterator it = m_ClientMap.begin();
	while(it != m_ClientMap.end())
	{
		int nSocket = it->first;

		//删除命令列表中的数据
		delete it->second;

		it++;
	}
	m_ClientMap.clear();
	pthread_mutex_unlock(&m_thread_mutex);
	return true;
}

bool CGetFileServerSocket::CheckClientTimeOut()
{
	pthread_mutex_lock(&m_thread_mutex);
	bool bDelItem = false;
	GetFileClientMap::iterator it = m_ClientMap.begin();
	while(it != m_ClientMap.end())
	{
		CGetFileClientSocket *pMsgSocket = it->second;
		//检测数据是否处理
		if(pMsgSocket->m_bEndThread == true)
		{
			//LogError("下载端口：数据已经处理完成，断开来自IP.chHost=%s\r\n",pMsgSocket->m_chHost);
			bDelItem = true;
			DelClient(it->first);
			break;
		}

		//检测是否长时间没有收到来自客户端的数据
		if(pMsgSocket->CheckClientSendState() == false)
		{
			//LogError("下载端口：长时间没有收到客户端数据，断开来自IP.chHost=%s\r\n",pMsgSocket->m_chHost);
			bDelItem = true;
			DelClient(it->first);
			break;
		}

		it++;
	}
    pthread_mutex_unlock(&m_thread_mutex);

	if(bDelItem == true)
		CheckClientTimeOut();

	return true;
}