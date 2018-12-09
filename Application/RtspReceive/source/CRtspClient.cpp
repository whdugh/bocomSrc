#include "CRtspClient.h"

FILE* m_fp = NULL;
void printf2File(const char* format,...)
{
	return;
	va_list args;
	va_start(args,format);
	//多参数
	char szBuffer[1024];
	memset(szBuffer,0,sizeof(szBuffer));
	vsprintf(szBuffer,format,args);
	va_end(args);
	//加上时间
	//std::string log = "[" + GetCurrentTime() + "]";
	//log += szBuffer;
	char buf[128];
	memset(buf, 0, sizeof(buf));
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	#ifndef WIN32
	localtime_r( &now,newTime );
	#else
	newTime = localtime( &now );
	#endif

	sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d :%s",newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday, newTime->tm_hour, newTime->tm_min, newTime->tm_sec,szBuffer);
	printf("%s\n",buf);

	std::string log = buf;
	fwrite(log.c_str(),1,log.size(),m_fp);
	fflush(m_fp);
}

#ifdef WIN32
DWORD WINAPI ThreadLoginRtspServer(LPVOID pArg)
{
	//处理一条数据
	CRtspClient* pRtspClient = (CRtspClient*)pArg;
	if(pRtspClient == NULL) return 0;

	pRtspClient->LoginRtspServer();

	ExitThread(0);
	return 0;
}
#else
void* ThreadLoginRtspServer(void* pArg)
{
	//取类指针
	CRtspClient* pRtspClient = (CRtspClient*)pArg;
	if(pRtspClient == NULL) return 0;

	pRtspClient->LoginRtspServer();

	pthread_exit((void *)0);
	return pArg;
}
#endif

//构造
CRtspClient::CRtspClient()
{
	m_strHost = "";
	#ifdef WIN32
	 m_hThread = NULL;
	 m_bThreadStopped = false;
#else
	m_nThreadId = 0;
	#endif
}

//析构
CRtspClient::~CRtspClient()
{
}

bool CRtspClient::Init(string strHost,int nPort,string strStream)
{
#ifdef WIN32
	//WSADATA wsaData;
	//::WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	
	/*if (m_fp == NULL)
	{
		m_fp = fopen("./testRtsp.log","a+");
	}*/
	m_strHost = strHost;
	m_bEndThread = false;
	m_bLogOnServer = false;
	m_RtspSocket.Init(strHost,nPort,strStream);	
	m_RtpSocket.Init(strHost);	
	m_RtcpSocket.Init(strHost);	
	
	#ifdef WIN32
	DWORD nThreadId = 0;
	m_hThread = CreateThread(NULL,0,ThreadLoginRtspServer,this,0,&nThreadId);
	m_bThreadStopped = false;
	#else
	//线程id
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//启动登陆线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadLoginRtspServer,this);
	if(nret!=0)
	{
		return false;
	}

	pthread_attr_destroy(&attr);
#endif

	return true;
}

void CRtspClient::UnInit()
{
	m_bEndThread = true;
	m_RtspSocket.UnInit();
	m_RtpSocket.UnInit();
	m_RtcpSocket.UnInit();

	#ifdef WIN32
	while(!m_bThreadStopped)
	{
		Sleep(100);
	}
	if(m_hThread != NULL)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
#else
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId, NULL);
		m_nThreadId = 0;
	}
	#endif
	if(m_fp != NULL)
	{
		fclose(m_fp);
	}
	
}

//获取一帧h264数据
void CRtspClient::PopFrame(string& strFrame)
{
	m_RtpSocket.PopFrame(strFrame);
}

bool CRtspClient::LogOnServer()
{
	//if(m_RtpSocket.ConnectRtpServerMulti())
	//{
	//	//m_RtcpSocket.SetRtpSocket(&m_RtpSocket);
	//	//m_RtcpSocket.SetPort(m_RtspSocket.GetRtcpPort());
	//	//m_RtcpSocket.SetReceivePort(m_RtspSocket.GetReceiveRtpPort()+1);
	//	printf2File("[CRtspClient::ConnectRtpServerMulti]2 GetRtpPort:%d GetReceiveRtpPort:%d\n",m_RtspSocket.GetRtpPort(),m_RtspSocket.GetReceiveRtpPort()+1);
	//	//连接rtcp控制端口
	//	//if(m_RtcpSocket.ConnectRtcpServer())
	//	{
	//		printf2File("[CRtspClient::ConnectRtpServerMulti] ConnectRtcpServer OK\n");
	//		m_bLogOnServer = true;
	//		return true;	
	//	}
	//}

	m_bLogOnServer = false;

	//连接rtsp服务器
	if(m_RtspSocket.ConnectRtspServer())
	{
		printf2File("[CRtspClient::LogOnServer] m_RtpSocket.ConnectRtspServer ok\n");
		if(m_RtspSocket.SendOptionCmd())
		{
			printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendOptionCmd ok\n");
			if(m_RtspSocket.SendDescribeCmd())
			{
				printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendDescribeCmd ok\n");
				if(m_RtspSocket.SendSetUpCmd())
				{
					printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendSetUpCmd ok\n");
					if(m_RtspSocket.SendPlayCmd())
					{
						printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendPlayCmd ok\n");

						printf2File("[CRtspClient::LogOnServer]GetRtpPort:%d GetReceiveRtpPort:%d\n",m_RtspSocket.GetRtpPort(),m_RtspSocket.GetReceiveRtpPort());
						m_RtpSocket.SetPort(m_RtspSocket.GetRtpPort());
						m_RtpSocket.SetReceivePort(m_RtspSocket.GetReceiveRtpPort());

						printf2File("[CRtspClient::LogOnServer] m_nType:%d OK\n",m_nType);
						if (m_nType == 0)
						{
							//连接rtp数据端口
							if(m_RtpSocket.ConnectRtpServer())
							{
								m_RtcpSocket.SetRtpSocket(&m_RtpSocket);
								m_RtcpSocket.SetPort(m_RtspSocket.GetRtcpPort());
								m_RtcpSocket.SetReceivePort(m_RtspSocket.GetReceiveRtpPort()+1);
								printf2File("[CRtspClient::LogOnServer]2 GetRtpPort:%d GetReceiveRtpPort:%d\n",m_RtspSocket.GetRtpPort(),m_RtspSocket.GetReceiveRtpPort()+1);
								//连接rtcp控制端口
								if(m_RtcpSocket.ConnectRtcpServer())
								{
									printf2File("[CRtspClient::LogOnServer] ConnectRtcpServer OK\n");
									m_bLogOnServer = true;
									return true;	
								}
							}
							else
							{
								printf2File("[CRtspClient::LogOnServer] m_RtpSocket.ConnectRtpServer failed\n");
							}
						}
						else if (m_nType == 1)
						{
							if(m_RtpSocket.ConnectRtpServerMulti())
							{
								//m_RtcpSocket.SetRtpSocket(&m_RtpSocket);
								//m_RtcpSocket.SetPort(m_RtspSocket.GetRtcpPort());
								//m_RtcpSocket.SetReceivePort(m_RtspSocket.GetReceiveRtpPort()+1);
								//printf2File("[CRtspClient::ConnectRtpServerMulti]2 GetRtpPort:%d GetReceiveRtpPort:%d\n",m_RtspSocket.GetRtpPort(),m_RtspSocket.GetReceiveRtpPort()+1);
								////连接rtcp控制端口
								//if(m_RtcpSocket.ConnectRtcpServer())
								{
									printf2File("[CRtspClient::ConnectRtpServerMulti] ConnectRtpServerMulti OK\n");
									m_bLogOnServer = true;
									return true;	
								}
							}
							else
							{
								printf2File("[CRtspClient::LogOnServer] ConnectRtpServerMulti failed\n");
							}
						}
					}
					else
					{
						printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendPlayCmd failed\n");
					}
				}
				else
				{
					printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendSetUpCmd failed\n");
				}
			}
			else
			{
				printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendDescribeCmd failed\n");
			}
		}
		else
		{
			printf2File("[CRtspClient::LogOnServer] m_RtpSocket.SendOptionCmd failed\n");
		}
	}
	else
	{
		printf2File("[CRtspClient::LogOnServer] m_RtpSocket.ConnectRtspServer failed\n");
	}
	return m_bLogOnServer;
}

void CRtspClient::LoginRtspServer()
{
	int nCount = 0;
	while(!m_bEndThread)
	{
		while(!m_bEndThread && !m_bLogOnServer)
		{
			m_bLogOnServer = LogOnServer();
			printf2File("[CRtspClient::LoginRtspServer] m_bLogOnServer:%d\n",m_bLogOnServer);
			#ifndef WIN32
				usleep(1000);
			#else
				Sleep(1);
			#endif
		}

		if(m_bEndThread)
		{
			break;
		}

		#ifndef WIN32
			usleep(100*1000);
		#else
			Sleep(100*1);
		#endif

		nCount++;

		if(nCount == 600)
		{
			nCount = 0;
			//如果10秒钟仍然没有收到视频则重新连接
			if(m_bLogOnServer)
			{
				unsigned int uLastVideoTime = m_RtpSocket.GetLastVideoTime();
				unsigned int uTime = time(NULL);

				if( abs(uTime - uLastVideoTime) >= 10 )
				{
					printf2File("[CRtspClient::LoginRtspServer]ReConnect\n");
					m_bLogOnServer = false;
				}
			}
		}
	}
	#ifdef WIN32
	m_bThreadStopped = true;
	#endif
	printf2File("[CRtspClient::LoginRtspServer]end LoginRtspServer\n");
}

//设置认证数据
void CRtspClient::SetAuthorizationData(string& strpasswd)
{
	m_RtspSocket.SetAuthorizationData(strpasswd);
}

//设置接收端口
void CRtspClient::SetReceivePort(int nPort)
{
	m_RtspSocket.SetReceivePort(nPort);
}
