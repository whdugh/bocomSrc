#include "CRtcpSocket.h"

#ifdef WIN32
DWORD WINAPI ThreadRecvRtcpData(LPVOID pArg)
{
	//处理一条数据
	CRtcpSocket* pCRtcpSocket = (CRtcpSocket*)pArg;
	if(pCRtcpSocket == NULL) return -1;

	pCRtcpSocket->RecvRtcpData();

	ExitThread(0);
	return 0;
}

DWORD WINAPI ThreadSendRtcpData(LPVOID pArg)
{
	//处理一条数据
	CRtcpSocket* pCRtcpSocket = (CRtcpSocket*)pArg;
	if(pCRtcpSocket == NULL) return -1;

	pCRtcpSocket->SendRtcpData();

	ExitThread(0);
	return 0;
}

#else
//接收记录线程
void* ThreadRecvRtcpData(void* pArg)
{
	//取类指针
	CRtcpSocket* pCRtcpSocket = (CRtcpSocket*)pArg;
	if(pCRtcpSocket == NULL) return pArg;

	pCRtcpSocket->RecvRtcpData();

    pthread_exit((void *)0);
	return pArg;
}

//发送记录线程
void* ThreadSendRtcpData(void* pArg)
{
	//取类指针
	CRtcpSocket* pCRtcpSocket = (CRtcpSocket*)pArg;
	if(pCRtcpSocket == NULL) return pArg;

	pCRtcpSocket->SendRtcpData();

    pthread_exit((void *)0);
	return pArg;
}
#endif

//构造
CRtcpSocket::CRtcpSocket()
{
	m_nRtcpSocket = -1;
	m_strRtcpHost = "";
	m_nRtcpPort = 0;
	m_nReceivePort = 0;
	m_pRtpSocket = NULL;
	#ifdef WIN32
	m_hRecvThread = NULL;
	m_hSendThread = NULL;
	m_bSendThreadStopped = false;
	m_bRecvThreadStopped = false;
#else
	m_nSendThreadId = 0;
	m_nRecvThreadId = 0;
	#endif
}

//析构
CRtcpSocket::~CRtcpSocket()
{
}

bool CRtcpSocket::Init(string strHost)
{
	m_bEndThread = false;
	m_bConnect = false;

	m_strRtcpHost = strHost;

	m_uLatSRT = 0;
	m_uRandSSrc = 0;

	printf2File("m_strRtcpHost=%s\n",m_strRtcpHost.c_str());
	
	#ifdef WIN32
	DWORD nThreadId = 0;
	m_hRecvThread = CreateThread(NULL,0,ThreadRecvRtcpData,this,0,&nThreadId);
	m_hSendThread = CreateThread(NULL,0,ThreadSendRtcpData,this,0,&nThreadId);
	m_bSendThreadStopped = false;
	m_bRecvThreadStopped = false;
	#else
	//线程id
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//启动事件监控线程
	int nret=pthread_create(&m_nRecvThreadId,&attr,ThreadRecvRtcpData,this);
	if(nret!=0)
	{
		return false;
	}

	nret=pthread_create(&m_nSendThreadId,&attr,ThreadSendRtcpData,this);
	if(nret!=0)
	{
		return false;
	}

	pthread_attr_destroy(&attr);
	#endif
	return true;
}

void CRtcpSocket::UnInit()
{
	m_bEndThread = true;

	m_uLatSRT = 0;
	m_uRandSSrc = 0;

	CloseRtcpServer();

	m_bConnect = false;

	#ifdef WIN32
	while(!m_bSendThreadStopped)
	{
		Sleep(100);
	}
	if(m_hSendThread != NULL)
	{
		CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}

	while(!m_bRecvThreadStopped)
	{
		Sleep(100);
	}
	if(m_hRecvThread != NULL)
	{
		CloseHandle(m_hRecvThread);
		m_hRecvThread = NULL;
	}
#else
	if(m_nSendThreadId != 0)
	{
		pthread_join(m_nSendThreadId, NULL);
		m_nSendThreadId = 0;
	}
	if(m_nRecvThreadId != 0)
	{
		pthread_join(m_nRecvThreadId, NULL);
		m_nRecvThreadId = 0;
	}
	#endif
}


//连接rtpserver
bool CRtcpSocket::ConnectRtcpServer()
{
	CloseRtcpServer();

	m_uLatSRT = 0;
	m_uRandSSrc = GetRandSSRC();
	
	#ifndef WIN32
	bzero(&m_udp_addr,sizeof(m_udp_addr));
#endif
	m_udp_addr.sin_family=AF_INET;
    m_udp_addr.sin_addr.s_addr= inet_addr(m_strRtcpHost.c_str());
    m_udp_addr.sin_port=htons(m_nRtcpPort);

	printf2File("[CRtcpSocket::ConnectRtcpServer]m_strRtcpHost:%s m_nRtcpPort:%d\n",m_strRtcpHost.c_str(),m_nRtcpPort);
    // socket
	struct sockaddr_in udp_addr;
	#ifndef WIN32
    bzero(&udp_addr,sizeof(udp_addr));
#endif
    udp_addr.sin_family=AF_INET;
    udp_addr.sin_addr.s_addr= htonl(INADDR_ANY);
    udp_addr.sin_port=htons(m_nReceivePort);

	if ((m_nRtcpSocket=socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf2File("[CRtcpSocket::ConnectRtcpServer]socket failed\n");
		return false;
	}

	int nLen = 8192000;//8192000 
	
	#ifndef WIN32
	if (setsockopt(m_nRtcpSocket,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
#else
	if (setsockopt(m_nRtcpSocket,SOL_SOCKET,SO_RCVBUF,(char*)&nLen,sizeof(int)) < 0)
#endif
	{
		printf2File("[CRtcpSocket::ConnectRtcpServer]setsockopt SO_RCVBUF failed\n");
		mvCloseSocket(m_nRtcpSocket);
		return false;
	}

	int on = 1;
	if(setsockopt(m_nRtcpSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
	{
		printf2File("[CRtcpSocket::ConnectRtcpServer]setsockopt SO_REUSEADDR failed\n");
		mvCloseSocket(m_nRtcpSocket);
		return false;
	}

	struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0; //1ms

	#ifndef WIN32
     if(setsockopt(m_nRtcpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval)) < 0)
#else
	 if(setsockopt(m_nRtcpSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval)) < 0)
#endif
	 {
		 printf2File("[CRtcpSocket::ConnectRtcpServer]setsockopt SO_SNDTIMEO failed\n");
		 mvCloseSocket(m_nRtcpSocket);
		return false;
	 }	

#ifndef WIN32
     if(setsockopt(m_nRtcpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0)
#else
	 if(setsockopt(m_nRtcpSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval)) < 0)
#endif
	 {
		 printf2File("[CRtcpSocket::ConnectRtcpServer]setsockopt SO_RCVTIMEO failed\n");
		 mvCloseSocket(m_nRtcpSocket);
		return false;
	 }	

	if(bind(m_nRtcpSocket, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) == -1)
	{
		printf2File("[CRtcpSocket::ConnectRtcpServer]bind failed\n");
		mvCloseSocket(m_nRtcpSocket);
		return false;
	}
	
	#ifndef WIN32
	usleep(500);
#else
	Sleep(10);
#endif

	m_bConnect = true;

	 return true;
}

//关闭rtpserver
bool CRtcpSocket::CloseRtcpServer()
{
	mvCloseSocket(m_nRtcpSocket);

	m_bConnect = false;

	return true;
}

//接收rtp数据
void CRtcpSocket::RecvRtcpData()
{
	socklen_t addr_len = sizeof(m_udp_addr);

	while (!m_bEndThread)
    {
        if(m_bConnect)
        {
		//	printf2File("before RecvRtcpMsg,m_nRtcpPort = %d,m_nReceivePort = %d\n",m_nRtcpPort,m_nReceivePort);

			char buf[4096] = {0};
			int nBytes = 0;

			#ifndef WIN32
			nBytes = recvfrom(m_nRtcpSocket,(void*)buf,sizeof(buf),MSG_NOSIGNAL,(struct sockaddr *) &m_udp_addr, &addr_len);
#else
			nBytes = recvfrom(m_nRtcpSocket,buf,sizeof(buf),0,(struct sockaddr *) &m_udp_addr, &addr_len);
#endif
			
		//	
			
			if(nBytes  <= 0 )
            {
               continue;
            }

			printf2File("*************************RecvRtcpMsg nBytes = %d\n",nBytes);

			string strMsg;
			strMsg.append(buf,nBytes);
			DealRtcpData(strMsg);
			
		}
		
		#ifndef WIN32
	usleep(100*1000);
#else
	Sleep(100);
#endif
	}

	#ifdef WIN32
	m_bRecvThreadStopped = true;
	#endif
	printf2File("end RecvRtcpData\n");
}


//发送rtcp数据
void CRtcpSocket::SendRtcpData()
{
	int nCount = 0;
	while (!m_bEndThread)
    {
       //printf2File("1111111111111111111*************************** SendRtcpData\n");
	#ifndef WIN32
		usleep(100*1000);
	#else
		Sleep(100*1);
	#endif

		nCount++;

		if(nCount == 40)
		{
			nCount = 0;

			 if(m_bConnect)
			{
				char buf[4096] = {0};
				int nBytes = 0;

				//发送数据
				string strRRSDMsg = GetRRSD();
				nBytes = 0;

				#ifndef WIN32
				nBytes = sendto(m_nRtcpSocket,(void*)strRRSDMsg.c_str(),strRRSDMsg.size(),MSG_NOSIGNAL,(struct sockaddr *) &m_udp_addr,sizeof(m_udp_addr));
				#else
							nBytes = sendto(m_nRtcpSocket,(char*)strRRSDMsg.c_str(),strRRSDMsg.size(),0,(struct sockaddr *) &m_udp_addr,sizeof(m_udp_addr));
				#endif

				if(nBytes  <= 0 )
				{
				   continue;
				}

				printf2File("++++++++++++++++++++++++sendto SendRtcpData=%d\n",nBytes);
				
			}
		}

		//printf2File("2222222222222*************************** SendRtcpData\n");
	}
	
	#ifdef WIN32
	m_bSendThreadStopped = true;
	#endif

	printf2File("end SendRtcpData\n");
}

//处理rtp数据
void CRtcpSocket::DealRtcpData(string strMsg)
{
	m_uLatSRT = ntohl(*((unsigned int*)(strMsg.c_str()+10)));
	//printf2File("=========m_uLatSRT=%x\n",m_uLatSRT);
}

//获取Receive report and source Description
string CRtcpSocket::GetRRSD()
{
	string strMsg;

	char buf[52] = {0};

	////////////////////以下是Receive report
	buf[0] = 0x81;
	buf[1] = 0xc9;
	buf[2] = 0x00;
	buf[3] = 0x07;
	//ssrc(每次绘话都需要改变)
	memcpy(&buf[4],&m_uRandSSrc,4);

	//sender ssrc(需要从发送端中获取)
	if(m_pRtpSocket != NULL)
	{
		unsigned int uSSrc = htonl(m_pRtpSocket->GetSSRC());
		memcpy(&buf[8],&uSSrc,4);
		//printf2File("======buf[8]=%x,buf[9]=%x,buf[10]=%x,buf[11]=%x\n",buf[8],buf[9],buf[10],buf[11]);
	}
	//fraction lost
	buf[12] = 0x0;
	//number of packets lost
	buf[13] = 0x0;
	buf[14] = 0x0;
	buf[15] = 0x0;

	// 接收到的扩展的最高序列号
	buf[16] = 0x00;
	buf[17] = 0x00;
	
	if(m_pRtpSocket != NULL)
	{
		unsigned short uSeq = m_pRtpSocket->GetSeq();
		//printf2File("======uSeq=%d\n",uSeq);
		uSeq = htons(uSeq);
		memcpy(&buf[18],&uSeq,2);
	}

	//到达间隔抖动
	buf[20] = 0x00;
	buf[21] = 0x00;
	buf[22] = 0x00;
	buf[23] = 0x00;

	//接收到的来自源SSRC_n的最新RTCP发送者报告(SR)的64位NTP时间标志的中间32位。若还没有接收到SR，该域值为零
	unsigned int uLatSRT = htonl(m_uLatSRT);
	memcpy(&buf[24],&uLatSRT,4);
	//printf2File("======buf[24]=%x,buf[25]=%x,buf[26]=%x,buf[27]=%x\n",buf[24],buf[25],buf[26],buf[27]);

	//从收到来自SSRC_n的SR包到发送此接收报告块之间的延时，以1/65536秒为单位。若还未收到来自SSRC_n的SR包，该域值为零
	unsigned int uDLSR = htonl(6554);
	if(m_uLatSRT <= 0)
	{
		uDLSR = 0;
	}
	memcpy(&buf[28],&uDLSR,4);
	
	////////////////////以下是source Description
	buf[32] = 0x81;
	buf[33] = 0xca;
	buf[34] = 0x00;
	buf[35] = 0x04;

	//ssrc(每次绘话都需要改变)
	memcpy(&buf[36],&m_uRandSSrc,4);

	//sdes items
	buf[40] = 0x1;
	buf[41] = 0x5;
	buf[42] = 'b';
	buf[43] = 'o';
	buf[44] = 'c';
	buf[45] = 'o';
	buf[46] = 'm';

	strMsg.append(buf,52);

	return strMsg;
}

//获取SSRC
unsigned int CRtcpSocket::GetRandSSRC()
{
	srand( (unsigned)time( NULL ) );
#ifndef WIN32
	sleep(1);
#else
	Sleep(1000);
#endif
	//getpid();
	int nRandCode = 1800 + (int)(1000.0*rand()/(RAND_MAX+1.0));
	return nRandCode;
}