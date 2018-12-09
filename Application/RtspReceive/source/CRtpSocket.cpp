#include "CRtpSocket.h"
string m_strBroadIP;

#ifdef WIN32
DWORD WINAPI ThreadRecvRtpData(LPVOID pArg)
{
	//处理一条数据
	CRtpSocket* pCRtpSocket = (CRtpSocket*)pArg;
	if(pCRtpSocket == NULL) return -1;

	pCRtpSocket->RecvRtpData();

	ExitThread(0);
	return 0;
}
#else
//接收记录线程
void* ThreadRecvRtpData(void* pArg)
{
	//取类指针
	CRtpSocket* pCRtpSocket = (CRtpSocket*)pArg;
	if(pCRtpSocket == NULL) return pArg;

	pCRtpSocket->RecvRtpData();

    pthread_exit((void *)0);
	return pArg;
}
#endif
//构造
CRtpSocket::CRtpSocket()
{
	m_nRtpSocket = -1;
	m_strRtpHost = "";
	m_strBroadIP = "";
	m_nRtpPort = 0;
	m_nReceivePort = 0;
	m_bConnect = false;
	m_uLastVideoTime = 0;

	m_strData = "";

	#ifdef WIN32
	m_hThread = NULL;
	m_bThreadStopped = false;
	InitializeCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_init(&m_FrameMutex,NULL);
	m_nThreadId = 0;
#endif
}

//析构
CRtpSocket::~CRtpSocket()
{
#ifdef WIN32
	DeleteCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_destroy(&m_FrameMutex);
#endif
}

bool CRtpSocket::Init(string strHost)
{
	m_bEndThread = false;
	m_bConnect = false;

	m_strRtpHost = strHost;

	m_uSeq = 0;
	m_uSSrc = 0;

	//m_strData.clear();

	printf2File("m_strRtpHost=%s\n",m_strRtpHost.c_str());
	
	#ifdef WIN32
	DWORD nThreadId = 0;
	m_hThread = CreateThread(NULL,0,ThreadRecvRtpData,this,0,&nThreadId);
	m_bThreadStopped = false;
	#else
	//线程id
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//启动事件监控线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadRecvRtpData,this);
	if(nret!=0)
	{
		return false;
	}
	pthread_attr_destroy(&attr);
#endif

	return true;
}

void CRtpSocket::UnInit()
{
	m_bEndThread = true;
	
	m_uSeq = 0;
	m_uSSrc = 0;

	CloseRtpServer();

	//m_strData.clear();
	//m_listFrame.clear();
	m_bConnect = false;

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
}

bool CRtpSocket::ConnectRtpServerMulti()
{
	CloseRtpServer();

	m_uSeq = 0;
	m_uSSrc = 0;
	m_strData = "";
	m_uLastVideoTime = 0;
	m_bReceiveCFHeader = false;
	//加锁缓冲区
#ifdef WIN32
	EnterCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_lock(&m_FrameMutex);
#endif
	m_strData.clear();
	m_listFrame.clear();
	//解锁缓冲区
#ifdef WIN32
	LeaveCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_unlock(&m_FrameMutex);
	bzero(&m_udp_addr,sizeof(m_udp_addr));
#endif

	printf2File("[CRtpSocket::ConnectRtpServerMulti]m_strBroadIP:%s m_nRtpPort:%d\n",m_strBroadIP.c_str(),m_nRtpPort);

	struct ip_mreq mreq;

	u_int yes=1; /*** MODIFICATION TO ORIGINAL */

	/* create what looks like an ordinary UDP socket */
	if ((m_nRtpSocket=socket(AF_INET,SOCK_DGRAM,0)) < 0) 
	{
		printf2File("[CRtpSocket::ConnectRtpServerMulti]socket failed\n");
		return false;
	}


	/**** MODIFICATION TO ORIGINAL */
	/* allow multiple sockets to use the same PORT number */
#ifndef WIN32
	if (setsockopt(m_nRtpSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) 
	{
		printf2File("[CRtpSocket::ConnectRtpServerMulti]setsockopt failed\n");
		return false;
	}
	/*** END OF MODIFICATION TO ORIGINAL */

	/* set up destination address */
	memset(&m_udp_addr,0,sizeof(m_udp_addr));
	m_udp_addr.sin_family=AF_INET;
	m_udp_addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
	m_udp_addr.sin_port=htons(m_nRtpPort);

	/* bind to receive address */
	if (bind(m_nRtpSocket,(struct sockaddr *) &m_udp_addr,sizeof(m_udp_addr)) < 0)
	{
		printf2File("[CRtpSocket::ConnectRtpServerMulti]bind failed\n");
		return false;
	}

	/* use setsockopt() to request that the kernel join a multicast group */
	mreq.imr_multiaddr.s_addr=inet_addr(m_strBroadIP.c_str());
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	if (setsockopt(m_nRtpSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) 
	{
		printf2File("[CRtpSocket::ConnectRtpServerMulti]setsockopt 2 failed\n");
		return false;
	}
#endif
	m_bConnect = true;

	printf2File("[CRtpSocket::ConnectRtpServerMulti]ConnectRtpServerMulti ok [m_nRtpSocket:%d]\n",m_nRtpSocket);

	return true;
}

//连接rtpserver
bool CRtpSocket::ConnectRtpServer()
{
	CloseRtpServer();

	m_uSeq = 0;
	m_uSSrc = 0;
	m_strData = "";
	m_uLastVideoTime = 0;
	m_bReceiveCFHeader = false;

	//加锁缓冲区
#ifdef WIN32
	EnterCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_lock(&m_FrameMutex);
#endif
	m_strData.clear();
	m_listFrame.clear();
	//解锁缓冲区
    #ifdef WIN32
	LeaveCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_unlock(&m_FrameMutex);
	bzero(&m_udp_addr,sizeof(m_udp_addr));
#endif

	
	m_udp_addr.sin_family=AF_INET;
    m_udp_addr.sin_addr.s_addr= inet_addr(m_strRtpHost.c_str());
    m_udp_addr.sin_port=htons(m_nRtpPort);
	printf2File("[CRtpSocket::ConnectRtpServer]m_strRtpHost:%s m_nRtpPort:%d\n",m_strRtpHost.c_str(),m_nRtpPort);
    // socket
	struct sockaddr_in udp_addr;

	#ifndef WIN32
    bzero(&udp_addr,sizeof(udp_addr));
#endif
    udp_addr.sin_family=AF_INET;
    udp_addr.sin_addr.s_addr= htonl(INADDR_ANY);
    udp_addr.sin_port=htons(m_nReceivePort);

	if ((m_nRtpSocket=socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf2File("[CRtpSocket::ConnectRtpServer]socket failed\n");
		return false;
	}
	printf2File("[CRtpSocket::ConnectRtpServer]socket:%d ok\n",m_nRtpSocket);

	int nLen = 8192000;//8192000
	
	#ifndef WIN32
	if (setsockopt(m_nRtpSocket,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
#else
	if (setsockopt(m_nRtpSocket,SOL_SOCKET,SO_RCVBUF,(char*)&nLen,sizeof(int)) < 0)
#endif
	{
		printf2File("[CRtpSocket::ConnectRtpServer]setsockopt SO_RCVBUF failed\n");
		mvCloseSocket(m_nRtpSocket);
		return false;
	}

	int on = 1;
	if(setsockopt(m_nRtpSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
	{
		printf2File("[CRtpSocket::ConnectRtpServer]setsockopt SO_REUSEADDR failed\n");
		mvCloseSocket(m_nRtpSocket);
		return false;
	}

	struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0; //1ms
	
	#ifndef WIN32
	 if(setsockopt(m_nRtpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0)
#else
	 if(setsockopt(m_nRtpSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval)) < 0)
#endif
    
	 {
		 printf2File("[CRtpSocket::ConnectRtpServer]setsockopt SO_RCVTIMEO failed\n");
		 mvCloseSocket(m_nRtpSocket);
		return false;
	 }

	if(bind(m_nRtpSocket, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) == -1)
	{
		printf2File("[CRtpSocket::ConnectRtpServer]bind failed\n");
		mvCloseSocket(m_nRtpSocket);
		return false;
	}

	#ifndef WIN32
	usleep(500);
#else
	Sleep(10);
#endif

	m_bConnect = true;
	
	printf2File("[CRtpSocket::ConnectRtpServer]ConnectRtpServer ok\n");

	 return true;
}

//关闭rtpserver
bool CRtpSocket::CloseRtpServer()
{
	mvCloseSocket(m_nRtpSocket);
	
	m_bConnect = false;
	return true;
}

//接收rtp数据
void CRtpSocket::RecvRtpData()
{
	
	socklen_t addr_len = sizeof(m_udp_addr);

	while (!m_bEndThread)
    {
        if(m_bConnect)
        {
			printf2File("before RecvRtpMsg,m_nRtpSocket=%d,m_nRtpPort=%d,m_nReceivePort=%d\n",m_nRtpSocket,m_nRtpPort,m_nReceivePort);
			char buf[15000] = {0};
			int nBytes = 0;

			#ifndef WIN32
			nBytes = recvfrom(m_nRtpSocket,(void*)buf,sizeof(buf),MSG_NOSIGNAL,(struct sockaddr *) &m_udp_addr, &addr_len);
#else
			nBytes = recvfrom(m_nRtpSocket,buf,sizeof(buf),0,(struct sockaddr *) &m_udp_addr, &addr_len);
#endif
			printf2File("==============after RecvRtpMsg nBytes = %d,errno=%d,addr_len=%d\n",nBytes,errno,addr_len);

			if(nBytes  <= 0 )
            {
				printf2File("[CRtpSocket::RecvRtpData] nBytes=0 failed\n");
               continue;
            }

		//printf2File("======================================nBytes = %d\n",nBytes);

			string strMsg;
			strMsg.append(buf,nBytes);
			DealRtpData(strMsg);
		}
		else
		{
			#ifndef WIN32
				usleep(1000);
			#else
				Sleep(1);
			#endif
		}
	}
	#ifdef WIN32
	m_bThreadStopped = true;
	#endif
	printf2File("end RecvRtpData\n");
}


//处理rtp数据
void CRtpSocket::DealRtpData(string strMsg)
{
	try
	{
	int nBytes = strMsg.size();
	printf2File("[CRtpSocket::DealRtpData]nBytes=%d\n\n",nBytes);

	if(nBytes < 4)
	{
		return;
	}
	
	//处理长峰平台特有数据
	if(*((unsigned char*)(strMsg.c_str())) == 0xaa && *((unsigned char*)(strMsg.c_str()+1)) == 0xbb && *((unsigned char*)(strMsg.c_str()+2)) == 0xcc && *((unsigned char*)(strMsg.c_str()+3)) == 0xdd)
	{
	//	printf2File("receive aabbccdd header,nBytes=%d\n",nBytes);
		m_bReceiveCFHeader = true;

		if(m_strData.size() > 0)
		{
		//		printf2File("===========m_strData.size()=%d\n",m_strData.size());
				
				m_uLastVideoTime = time(NULL);
printf2File("[CRtpSocket::DealRtpData]AddFrame OK 1\n");
				AddFrame(m_strData);
	
			   m_strData.clear();
		}
	}
	
	if(m_bReceiveCFHeader)
	{
		printf2File("======receive===nBytes=%d\n",nBytes);
		if(strMsg.size() > 0)
		m_strData.append((char*)strMsg.c_str(),strMsg.size());
		printf2File("[CRtpSocket::DealRtpData]m_bReceiveCFHeader:%d failed\n",m_bReceiveCFHeader);
		return;
	}
	
	//处理标准的rtp数据
	if(nBytes <= 12)
	{
		printf2File("[CRtpSocket::DealRtpData]nBytes <= 12,nBytes=%d\n",nBytes);
		return;
	}

	unsigned char RtpHeader = *((unsigned char*)(strMsg.c_str()));

	if(RtpHeader != 0x80 && RtpHeader != 0xa0)
	{
		printf2File("[CRtpSocket::DealRtpData]RtpHeader != 0x80 && 0xa0,RtpHeader=%x\n",RtpHeader);
		return;
	}

	
	unsigned char RtpPayloadHeader = *((unsigned char*)(strMsg.c_str()+1));

	if(RtpPayloadHeader != 0x60 && RtpPayloadHeader != 0xe0)
	{
		printf2File("[CRtpSocket::DealRtpData]RtpPayloadHeader != 0x60 && 0xe0,RtpPayloadHeader=%x\n",RtpPayloadHeader);
		return;
	}

	if(m_uSSrc <= 0)
	{
		m_uSSrc = ntohl(*((unsigned int*)(strMsg.c_str()+8)));
	//	printf2File("=========m_uSSrc=%x\n",m_uSSrc);
	}
	
	unsigned short uSeq = ntohs(*((unsigned short*)(strMsg.c_str()+2)));
	
	/*if(uSeq > m_uSeq+1)
	{
		FILE * fp = fopen("test2.avi","ab+");
		if(fp != NULL)
		{
			char buf[256] = {0};
			sprintf(buf,"m_uSeq=%u,uSeq=%u\n",m_uSeq,uSeq);
			fwrite(buf,1,32,fp);

			fclose(fp);
		}
	}*/

	m_uSeq = uSeq;

	unsigned char RtpPadding = RtpHeader &(0x20);

	unsigned char RtpPayload = (*((unsigned char*)(strMsg.c_str()+12)));
	
	unsigned char RtpType = RtpPayload &(0x1f);

	unsigned char start_code[4] = {0x00, 0x00, 0x00, 0x01};

	printf2File("[CRtpSocket::DealRtpData]Recv RtpData %x-%x\n",RtpHeader,RtpPayloadHeader);

	//return;
	if(RtpType < 24)
	{
		printf2File("************************Recv single NAL unit uSeq = %d,RtpPayload =%x,%x-%x-%x-%x,RtpType = %d,nBytes=%d\n",uSeq,RtpPayload,(*((unsigned char*)(strMsg.c_str()+13))),(*((unsigned char*)(strMsg.c_str()+14))),(*((unsigned char*)(strMsg.c_str()+15))),(*((unsigned char*)(strMsg.c_str()+16))),RtpType,nBytes);
		
		m_strData.append((char*)start_code,4);

		//printf("11 [CRtpSocket::DealRtpData]m_strData.size()=%d,strMsg.size()=%d\n",m_strData.size(),strMsg.size());

		if(strMsg.size()-12 > 0)
		m_strData.append((char*)strMsg.c_str()+12,strMsg.size()-12);

		AddFrame(m_strData);

		m_strData.clear();
		
	}
	else if(RtpType >= 28)
	{
	printf2File("===========Recv FU-A unit uSeq = %d,RtpPadding=%d,RtpPayload=%x,%x,RtpType = %d,nBytes = %d\n",uSeq,RtpPadding,RtpPayload,(*((unsigned char*)(strMsg.c_str()+13))),RtpType,nBytes);

		unsigned char RtpFU = (*((unsigned char*)(strMsg.c_str()+13)));

		unsigned char FUStart = (RtpFU)&(0x80);
		unsigned char FUEnd = (RtpFU)&(0x40);
		if(FUStart == 0x80 &&  FUEnd == 0)
		{
			m_strData.clear();
			unsigned char FUNri = RtpPayload&0x60;
			unsigned char FUType = RtpFU&0x1f;

			unsigned char FUNalHeader = FUNri+FUType;

			printf2File("****************Begin FU-A unit uSeq = %d,RtpPayload=%x,%x-%x-%x-%x,RtpFU = %x,FUNalHeader=%x,nBytes = %d\n",uSeq,RtpPayload,(*((unsigned char*)(strMsg.c_str()+13))),(*((unsigned char*)(strMsg.c_str()+14))),(*((unsigned char*)(strMsg.c_str()+15))),(*((unsigned char*)(strMsg.c_str()+16))),RtpFU,FUNalHeader,nBytes);
			
			if(FUNalHeader > 0)
			m_strData.append((char*)start_code,4);

			m_strData.append((char*)&FUNalHeader,1);

			if(strMsg.size() > 14)
			{
				if(RtpPadding)
				{
					unsigned char nPaddingBytes = (*((unsigned char*)(strMsg.c_str()+strMsg.size()-1)));
					printf2File("[CRtpSocket::DealRtpData]nPaddingBytes=%d\n",nPaddingBytes);

					//printf("22 [CRtpSocket::DealRtpData]m_strData.size()=%d,strMsg.size()=%d,nPaddingBytes=%d\n",m_strData.size(),strMsg.size(),nPaddingBytes);
					if( (strMsg.size()-14 > nPaddingBytes) && (nPaddingBytes >= 0))
					m_strData.append((char*)strMsg.c_str()+14,strMsg.size()-14-nPaddingBytes);
				}
				else
				{
					//printf("33 [CRtpSocket::DealRtpData]m_strData.size()=%d,strMsg.size()=%d\n",m_strData.size(),strMsg.size());
					if(strMsg.size()-14 > 0)
					m_strData.append((char*)strMsg.c_str()+14,strMsg.size()-14);
				}
			}
		}
		else if(FUStart == 0 &&  FUEnd == 0)
		{
			printf2File("++++++++++++++++Middle FU-A unit uSeq = %d,RtpFU = %x,nBytes = %d\n",uSeq,RtpFU,nBytes);
			if(strMsg.size() > 14)
			{
				if(RtpPadding)
				{
					unsigned char nPaddingBytes = (*((unsigned char*)(strMsg.c_str()+strMsg.size()-1)));
					printf2File("[CRtpSocket::DealRtpData]nPaddingBytes=%d\n",nPaddingBytes);
					//printf("44 [CRtpSocket::DealRtpData]m_strData.size()=%d,strMsg.size()=%d,nPaddingBytes=%d\n",m_strData.size(),strMsg.size(),nPaddingBytes);
					if( (strMsg.size()-14 > nPaddingBytes) && (nPaddingBytes >= 0))
					m_strData.append((char*)strMsg.c_str()+14,strMsg.size()-14-nPaddingBytes);
				}
				else
				{
					//printf("55 [CRtpSocket::DealRtpData]m_strData.size()=%d,strMsg.size()=%d\n",m_strData.size(),strMsg.size());
					if(strMsg.size()-14 > 0)
					m_strData.append((char*)strMsg.c_str()+14,strMsg.size()-14);
				}
			}
		}
		else if(FUStart == 0 &&  FUEnd == 0x40)
		{
			printf2File("----------------End FU-A unit uSeq =%d,RtpPayload=%x,RtpFU = %x,nBytes = %d\n",uSeq,RtpPayload,RtpFU,nBytes);
			if(strMsg.size() > 14)
			{
				if(RtpPadding)
				{
					unsigned char nPaddingBytes = (*((unsigned char*)(strMsg.c_str()+strMsg.size()-1)));
					//printf2File("nPaddingBytes=%d\n",nPaddingBytes);
					//printf("66 [CRtpSocket::DealRtpData]m_strData.size()=%d,strMsg.size()=%d,nPaddingBytes=%d\n",m_strData.size(),strMsg.size(),nPaddingBytes);
					if( (strMsg.size()-14 > nPaddingBytes) && (nPaddingBytes >= 0))
					m_strData.append((char*)strMsg.c_str()+14,strMsg.size()-14-nPaddingBytes);
					
				}
				else
				{
					//printf("77 [CRtpSocket::DealRtpData]m_strData.size()=%d,strMsg.size()=%d\n",m_strData.size(),strMsg.size());
					if(strMsg.size()-14 > 0)
					m_strData.append((char*)strMsg.c_str()+14,strMsg.size()-14);	
				}
			}

			//printf2File("===========m_strData.size()=%d\n",m_strData.size());

			/*FILE * fp = fopen("test1.avi","ab+");

	if(fp != NULL)
		{
			fwrite(m_strData.c_str(),1,m_strData.size(),fp);
		}


	if(fp != NULL)
	{
		fclose(fp);
	}*/
			m_uLastVideoTime = time(NULL);
printf2File("[CRtpSocket::DealRtpData]AddFrame OK 2\n");
			AddFrame(m_strData);

			m_strData.clear();
		}
	}
	else
	{
		printf2File("************************Recv uSeq = %d,RtpPayload =%x,RtpType = %d\n",uSeq,RtpPayload,RtpType);
	}
	}
	catch (...)
	{

	}

}

void CRtpSocket::AddFrame(string& strFrame)
{
	//加锁缓冲区
	#ifdef WIN32
	EnterCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_lock(&m_FrameMutex);
#endif

	if(m_listFrame.size() > 100)
	{
		m_listFrame.pop_front();
	}

	m_listFrame.push_back(strFrame);
#ifdef WIN32
	LeaveCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_unlock(&m_FrameMutex);
#endif
}

void CRtpSocket::PopFrame(string& strFrame)
{
	//加锁缓冲区
	#ifdef WIN32
	EnterCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_lock(&m_FrameMutex);
#endif
	
	if(m_listFrame.size() > 0)
	{
		strFrame = *(m_listFrame.begin());

		m_listFrame.pop_front();
	}
#ifdef WIN32
	LeaveCriticalSection(&m_FrameMutex);
#else
	pthread_mutex_unlock(&m_FrameMutex);
#endif
}