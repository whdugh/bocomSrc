#include "CRtspSocket.h"

const char DeBase64Tab[] =
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                62,        // '+'
                0, 0, 0,
                63,        // '/'
                52, 53, 54, 55, 56, 57, 58, 59, 60, 61,        // '0'-'9'
                0, 0, 0, 0, 0, 0, 0,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,        //'A'-'Z'
                0, 0, 0, 0, 0, 0,
                26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,        //'a'-'z'
            };

//base64编码
const char EnBase64Tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int m_nType;

void EnBase64(std::string& result,unsigned char* pSrc,int nSrcLen)
{

    unsigned char c1, c2, c3;    // 输入缓冲区读出3个字节
    char pDst[4]={0};          // 输出缓冲区4个字节

    int nDiv = nSrcLen / 3;      // 输入数据长度除以3得到的倍数
    int nMod = nSrcLen % 3;      // 输入数据长度除以3得到的余数

    printf2File("EncodeBase64=%d,nDiv=%d,nMod=%d\n",nSrcLen,nDiv,nMod);

    // 每次取3个字节，编码成4个字符
    for (int i = 0; i < nDiv; i ++)
    {
        // 取3个字节
        c1 = *pSrc++;
        c2 = *pSrc++;
        c3 = *pSrc++;

        // 编码成4个字符
        pDst[0] = EnBase64Tab[c1 >> 2];
        pDst[1] = EnBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3f];
        pDst[2] = EnBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3f];
        pDst[3] = EnBase64Tab[c3 & 0x3f];
        //printf2File("c1=%d,c2=%d,c3=%d,pDst[0]=%d,pDst[1]=%d,pDst[2]=%d,pDst[3]=%d\n",c1,c2,c3,pDst[0],pDst[1],pDst[2],pDst[3]);

        result.append(pDst,4);
    }

    // 编码余下的字节
    if (nMod == 1)
    {
        c1 = *pSrc++;
        pDst[0] = EnBase64Tab[(c1 & 0xfc) >> 2];
        pDst[1] = EnBase64Tab[((c1 & 0x03) << 4)];
        pDst[2] = '=';
        pDst[3] = '=';
        result.append(pDst,4);
    }
    else if (nMod == 2)
    {
        c1 = *pSrc++;
        c2 = *pSrc++;
        pDst[0] = EnBase64Tab[(c1 & 0xfc) >> 2];
        pDst[1] = EnBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xf0) >>4)];
        pDst[2] = EnBase64Tab[((c2 & 0x0f) << 2)];
        pDst[3] = '=';
        result.append(pDst,4);
    }
}

//构造
CRtspSocket::CRtspSocket()
{
	m_nRtspSocket = -1;
	m_strRtspHost = "";
	m_nRtspPort = 0;
	m_nCSeq = 1;
	m_strStream = "avStream";
	m_nRtpPort = 0;
	m_nRtcpPort = 0;
	m_nReceiveRtpPort = GetRandRtpPort();
	m_strSessionID = "1";
	m_strEnpasswd = "";
	m_strTrackID = "track1";
}

//析构
CRtspSocket::~CRtspSocket()
{
}

bool CRtspSocket::Init(string strHost,int nPort,string strStream)
{
	m_bEndThread = false;
	m_nType = 0;

	m_strRtspHost = strHost;
	m_nRtspPort = nPort;
	m_strStream = strStream;
	//printf2File("=================================m_strStream=%s\n",m_strStream);
	//m_nReceiveRtpPort = GetRandRtpPort();

	return true;
}

void CRtspSocket::UnInit()
{
	m_bEndThread = true;

	SendTearDownCmd();

	CloseRtspServer();
}


//连接rtspserver
bool CRtspSocket::ConnectRtspServer()
{
	if(m_nRtspSocket > 0)
	{
		bool bRet = SendTearDownCmd();
		printf2File("[CRtspSocket::ConnectRtspServer]:SendTearDownCmd:%d\n",bRet);
	}

	 if (!mvPrepareSocket(m_nRtspSocket))
     {
		 mvCloseSocket(m_nRtspSocket);
		 printf2File("[CRtspSocket::ConnectRtspServer]:mvPrepareSocket\n");
		return false;
	 }

	 int on = 1;
	if(setsockopt(m_nRtspSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
	{
		printf2File("[CRtspSocket::ConnectRtspServer]:SO_REUSEADDR\n");
		mvCloseSocket(m_nRtspSocket);
		return false;
	}

	struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0; //1ms	

	 #ifndef WIN32
     if(setsockopt(m_nRtspSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) < 0)
#else
	 if(setsockopt(m_nRtspSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval)) < 0)
#endif
	 {
		 printf2File("[CRtspSocket::ConnectRtspServer]:setsockopt\n");
		 mvCloseSocket(m_nRtspSocket);
		return false;
	 }	

	 if (!mvWaitConnect(m_nRtspSocket, m_strRtspHost, m_nRtspPort,2))
     {
		  printf2File("[CRtspSocket::ConnectRtspServer]:mvWaitConnect\n");
		 mvCloseSocket(m_nRtspSocket);
		 return false;
	 }
	printf2File("[CRtspSocket::ConnectRtspServer]:OK\n");

	 return true;
}

//关闭rtspserver
bool CRtspSocket::CloseRtspServer()
{
	mvCloseSocket(m_nRtspSocket);

	return true;
}

//设置认证数据
void CRtspSocket::SetAuthorizationData(string& strpasswd)
{
	EnBase64(m_strEnpasswd,(unsigned char*)strpasswd.c_str(),strpasswd.size());
}

//发送Option命令
bool CRtspSocket::SendOptionCmd()
{
	char buf[256] = {0};

	if(m_strEnpasswd.size() <= 0)
	{
		sprintf(buf,"OPTIONS rtsp://%s:%d/%s RTSP/1.0\r\nCSeq: %d\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq);
	}
	else
	{
		sprintf(buf,"OPTIONS rtsp://%s:%d/%s RTSP/1.0\r\nCSeq: %d\r\nAuthorization: Basic %s\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_strEnpasswd.c_str());
	}
	printf2File("[CRtspSocket::SendOptionCmd]SendOptionCmd = %s",buf);

	string strMsg(buf);

	if(!mvSendMsgToSocket(m_nRtspSocket,strMsg,true))
	{
		printf2File("[CRtspSocket::SendOptionCmd]SendOptionCmd:mvSendMsgToSocket failed\n");
		return false;
	}
	m_nCSeq++;

	string strRecvMsg;
	if(RecvMsg(strRecvMsg))
	{
		printf2File("[CRtspSocket::SendOptionCmd]:RecvMsg OK\n");
		return true;
	}
	else
	{
		printf2File("[CRtspSocket::SendOptionCmd]:RecvMsg failed\n");
		return false;
	}
}

//发送Describe命令
bool CRtspSocket::SendDescribeCmd()
{
	char buf[256] = {0};

	if(m_strEnpasswd.size() <= 0)
	{
		sprintf(buf,"DESCRIBE rtsp://%s:%d/%s RTSP/1.0\r\nCSeq: %d\r\nAccept: application/sdp\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq);
	}
	else
	{
		sprintf(buf,"DESCRIBE rtsp://%s:%d/%s RTSP/1.0\r\nCSeq: %d\r\nAccept: application/sdp\r\nAuthorization: Basic %s\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_strEnpasswd.c_str());
	}
	printf2File("[CRtspSocket::SendDescribeCmd]SendDescribeCmd = %s",buf);

	string strMsg(buf);

	if(!mvSendMsgToSocket(m_nRtspSocket,strMsg,true))
	{
		printf2File("[CRtspSocket::SendDescribeCmd]mvSendMsgToSocket failed\n");
		return false;
	}
	m_nCSeq++;

	string strRecvMsg;
	if(RecvMsg(strRecvMsg))
	{
		printf2File("[CRtspSocket::SendDescribeCmd]RecvMsg OK\n");
		DealDescribeMsg(strRecvMsg);
		return true;
	}
	else
	{
		printf2File("[CRtspSocket::SendDescribeCmd]RecvMsg failed\n");
		return false;
	}
}

//发送SetUp命令
bool CRtspSocket::SendSetUpCmd()
{
	char buf[256] = {0};
	
	if(strncmp(m_strStream.c_str(),"1",1) == 0)
	{
		sprintf(buf,"SETUP rtsp://%s:%d/%s/trackID=2 RTSP/1.0\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_nReceiveRtpPort,m_nReceiveRtpPort+1,m_strRtspHost.c_str(),m_nRtspPort);
	}
	else if(strncmp(m_strStream.c_str(),"0",1) == 0)
	{
		sprintf(buf,"SETUP rtsp://%s:%d/%s/trackID=0 RTSP/1.0\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_nReceiveRtpPort,m_nReceiveRtpPort+1);
	}
	else if(strncmp(m_strStream.c_str(),"cam",3) == 0)
	{
		sprintf(buf,"SETUP rtsp://%s:%d/%s/trackID=0 RTSP/1.0\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_nReceiveRtpPort,m_nReceiveRtpPort+1);
	}
	else
	{
		if(m_strEnpasswd.size() <= 0)
		{
			if (m_nType == 0)
			{
				sprintf(buf,"SETUP rtsp://%s:%d/%s/%s RTSP/1.0\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_strTrackID.c_str(),m_nCSeq,m_nReceiveRtpPort,m_nReceiveRtpPort+1);
			}
			else
			{
				sprintf(buf,"SETUP rtsp://%s/%s/track1 RTSP/1.0\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\n\r\n",m_strRtspHost.c_str(),m_strStream.c_str(),m_nCSeq,m_nReceiveRtpPort,m_nReceiveRtpPort+1);
			}
		}
		else
		{
		  sprintf(buf,"SETUP rtsp://%s:%d/%s/trackID=1 RTSP/1.0\r\nCSeq: %d\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\nAuthorization: Basic %s\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_nReceiveRtpPort,m_nReceiveRtpPort+1,m_strEnpasswd.c_str());
		}
	}
	printf2File("[CRtspSocket::SendSetUpCmd]SendSetUpCmd = %s",buf);

	string strMsg(buf);

	int nRet = -1;
	bool bRet = false;
	m_nCSeq++;
	int nCount = 0;

	while(nRet == -1)
	{
		nCount++;
		if (nCount > 10)
		{
			printf2File("CRtspSocket::SendSetUpCmd]:timeout count:%d\n",nCount);
			return false;
		}

		if(!mvSendMsgToSocket(m_nRtspSocket,strMsg,true))
		{
			printf2File("[CRtspSocket::SendSetUpCmd]:mvSendMsgToSocket\n");
			return false;
		}

		string strRecvMsg;
		if(RecvMsg(strRecvMsg))
		{
			printf2File("[CRtspSocket::SendSetUpCmd]:RecvMsg OK\n");
			int nPos = -1;
			nPos = strRecvMsg.find("multicast");
			if (nPos > 0)
			{
				m_nType = 1;
				nRet = DealRtspMsgWithMulticast(strRecvMsg);
				printf2File("CRtspSocket::SendSetUpCmd]:timeout nRet:%d count:%d\n",nRet,nCount);
			}
			else
			{
				m_nType = 0;
				nRet = 0;
				DealRtspMsg(strRecvMsg);
			}
			bRet = true;
		}
		else
		{
			printf2File("[CRtspSocket::SendSetUpCmd]:RecvMsg failed\n");
			bRet = false;
		}
	}
	return bRet;
}

//发送Play命令
bool CRtspSocket::SendPlayCmd()
{
	char buf[256] = {0};
	if(m_strEnpasswd.size() <= 0)
	{
		if (m_nType == 0)
		{
			sprintf(buf,"PLAY rtsp://%s:%d/%s/ RTSP/1.0\r\nCSeq: %d\r\nSession: %s\r\nRange: npt=0.000-\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_strSessionID.c_str());
		}
		else
		{
			sprintf(buf,"PLAY rtsp://%s/%s/ RTSP/1.0\r\nCSeq: %d\r\nSession: %s\r\nRange: npt=0.000-\r\n\r\n",m_strRtspHost.c_str(),m_strStream.c_str(),m_nCSeq,m_strSessionID.c_str());
		}
	}
	else
	{
		sprintf(buf,"PLAY rtsp://%s:%d/%s/ RTSP/1.0\r\nCSeq: %d\r\nSession: %s\r\nRange: npt=0.000-\r\nAuthorization: Basic %s\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_strSessionID.c_str(),m_strEnpasswd.c_str());
	}
	printf2File("[CRtspSocket::SendPlayCmd]SendPlayCmd = %s",buf);

	string strMsg(buf);

	if(!mvSendMsgToSocket(m_nRtspSocket,strMsg,true))
	{
		printf2File("[CRtspSocket::SendPlayCmd]:mvSendMsgToSocket failed\n");
		return false;
	}
	m_nCSeq++;

	string strRecvMsg;
	if(RecvMsg(strRecvMsg))
	{
		printf2File("[CRtspSocket::SendPlayCmd]:RecvMsg OK\n");
		return true;
	}
	else
	{
		printf2File("[CRtspSocket::SendPlayCmd]:RecvMsg failed\n");
		return false;
	}
	
}

//发送TearDown命令
bool CRtspSocket::SendTearDownCmd()
{
	char buf[256] = {0};

	if(m_strEnpasswd.size() <= 0)
	{
		sprintf(buf,"TEARDOWN rtsp://%s:%d/%s/ RTSP/1.0\r\nCSeq: %d\r\nSession: %s\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_strSessionID.c_str());
	}
	else
	{
		sprintf(buf,"TEARDOWN rtsp://%s:%d/%s/ RTSP/1.0\r\nCSeq: %d\r\nSession: %s\r\nAuthorization: Basic %s\r\n\r\n",m_strRtspHost.c_str(),m_nRtspPort,m_strStream.c_str(),m_nCSeq,m_strSessionID.c_str(),m_strEnpasswd.c_str());
	}
	printf2File("SendTearDownCmd = %s",buf);

	string strMsg(buf);

	if(!mvSendMsgToSocket(m_nRtspSocket,strMsg,true))
	{
		printf2File("[CRtspSocket::SendTearDownCmd]:mvSendMsgToSocket failed\n");
		return false;
	}
	m_nCSeq++;

	//m_nSessionID++;
	
	string strRecvMsg;
	if(RecvMsg(strRecvMsg))
	{
		printf2File("[CRtspSocket::SendTearDownCmd]:RecvMsg ok\n");
		return true;
	}
	else
	{
		printf2File("[CRtspSocket::SendTearDownCmd]:RecvMsg failed\n");
		return false;
	}
}


//接收rtsp返回数据
bool CRtspSocket::RecvMsg(string& strMsg)
{
	 if(m_nRtspSocket > 0)
      {
			printf2File("before RecvMsg\n");
			char buf[1024] = {0};
			int nBytes = 0;

#ifndef WIN32
			nBytes = recv(m_nRtspSocket,(void*)buf,sizeof(buf),MSG_NOSIGNAL);
#else
			nBytes = recv(m_nRtspSocket,buf,sizeof(buf),0);
#endif

			printf2File("after RecvMsg nBytes = %d\n",nBytes);

			if(nBytes > 0 )
            {
				printf2File("[CRtspSocket::RecvMsg]RecvMsg=%s\n",buf);
				strMsg = buf;
				return true;
            }
			else
			{
				printf2File("[CRtspSocket::RecvMsg]RecvMsg failed\n");
			}

	}
	 else
	 {
		printf2File("[CRtspSocket::RecvMsg]RecvMsg failed m_nRtspSocket=%d\n",m_nRtspSocket);
	 }
	 return false;
}

int CRtspSocket::DealRtspMsgWithMulticast(string strMsg)
{
	int nRet = 0;
	int nPos = -1;
	nPos = strMsg.find("destination");
	if(nPos >= 0)
	{
		nPos +=1;
		std::string strSubMsg;
		strSubMsg.append(strMsg.c_str()+nPos,strMsg.size() - nPos);
		int nPos1 = -1;
		nPos1 = strSubMsg.find("=");
		int nPos2 = -1;
		nPos2 =	strSubMsg.find(";");
		std::string strSubMsg1;
		if(nPos1 >= 0 && nPos2 > nPos1)
		{
			strSubMsg1.append(strSubMsg.c_str()+nPos1+1,nPos2 - nPos1-1);
			m_strBroadIP = strSubMsg1;
			printf2File("[CRtspSocket::DealRtspMsg]IP=%s\n",m_strBroadIP.c_str());
		}
	}

	nPos = -1;
	nPos = strMsg.find(";port");
	if(nPos >= 0)
	{
		nPos +=1;
		std::string strSubMsg;
		strSubMsg.append(strMsg.c_str()+nPos,strMsg.size() - nPos);
		int nPos1 = -1;
		nPos1 = strSubMsg.find("=");
		int nPos2 = -1;
		nPos2 = strSubMsg.find("-");
		int nPos3 = -1;
		nPos3 =	strSubMsg.find(";");

		std::string strSubMsg1;
		if(nPos1 >= 0 && nPos2 > nPos1)
		{
			strSubMsg1.append(strSubMsg.c_str()+nPos1+1,nPos2 - nPos1-1);
			m_nRtpPort = atoi(strSubMsg1.c_str());
			printf2File("[CRtspSocket::DealRtspMsg]m_nRtpPort=%d\n",m_nRtpPort);
		}

		std::string strSubMsg2;
		if(nPos2 >= 0 && nPos3 > nPos2)
		{
			strSubMsg2.append(strSubMsg.c_str()+nPos2+1,nPos3 - nPos2-1);
			m_nRtcpPort = atoi(strSubMsg2.c_str());
			printf2File("[CRtspSocket::DealRtspMsg]m_nRtcpPort=%d\n",m_nRtcpPort);
		}

	}
	/*nPos = -1;
	nPos = strMsg.find("timeout");

	if(nPos >= 0)
	{
		return -1;
	}*/
	//获取SessionID
	nPos = -1;
	nPos = strMsg.find("Session");

	if(nPos >= 0)
	{
		std::string strSubMsg;
		strSubMsg.append(strMsg.c_str()+nPos+9,strMsg.size() - nPos-9);

		int nPos1 = -1;
		nPos1 = strSubMsg.find("timeout");
		if(nPos1 >= 0)
		{
			nPos1 =	strSubMsg.find(";");

			std::string strSubMsg1("");
			if(nPos1 >= 0)
			{
				printf2File("nPos1=%d\n",nPos1);
				strSubMsg1.append(strSubMsg.c_str(),nPos1);
				printf2File("[CRtspSocket::DealRtspMsg]strSubMsg1=%s\n",strSubMsg1.c_str());
				m_strSessionID = strSubMsg1;
				printf2File("[CRtspSocket::DealRtspMsg]m_nSessionID=%s\n",m_strSessionID.c_str());
			}
		}
		else
		{
			printf2File("11 [CRtspSocket::DealRtspMsg]strSubMsg=%s\n",strSubMsg.c_str());
			m_strSessionID = strSubMsg;
		}
	}
	printf2File("[CRtspSocket::DealRtspMsg]BroadIP:%s m_nRtpPort:%d m_nRtcpPort:%d m_strSessionID:%s\n",m_strBroadIP.c_str(),m_nRtpPort,m_nRtcpPort,m_strSessionID.c_str());
	return nRet;
}

//处理rtsp返回数据
void CRtspSocket::DealRtspMsg(string strMsg)
{
	int nPos = -1;
	nPos = strMsg.find("server_port");
	if(nPos >= 0)
	{
		string strSubMsg;
		strSubMsg.append(strMsg.c_str()+nPos,strMsg.size() - nPos);

		int nPos1 = -1;
		nPos1 = strSubMsg.find("=");
		int nPos2 = -1;
		nPos2 = strSubMsg.find("-");
		int nPos3 = -1;
		nPos3 =	strSubMsg.find("\r\n");

		string strSubMsg1;
		if(nPos1 >= 0 && nPos2 > nPos1)
		{
			strSubMsg1.append(strSubMsg.c_str()+nPos1+1,nPos2 - nPos1-1);
			m_nRtpPort = atoi(strSubMsg1.c_str());
			printf2File("[CRtspSocket::DealRtspMsg]m_nRtpPort=%d\n",m_nRtpPort);
		}

		string strSubMsg2;
		if(nPos2 >= 0 && nPos3 > nPos2)
		{
			strSubMsg2.append(strSubMsg.c_str()+nPos2+1,nPos3 - nPos2-1);
			m_nRtcpPort = atoi(strSubMsg2.c_str());
			printf2File("[CRtspSocket::DealRtspMsg]m_nRtcpPort=%d\n",m_nRtcpPort);
		}

	}
	
	//获取SessionID
	nPos = -1;
	nPos = strMsg.find("Session");
	if(nPos >= 0)
	{
		string strSubMsg;
		strSubMsg.append(strMsg.c_str()+nPos+9,strMsg.size() - nPos-9);
		printf2File("[CRtspSocket::DealRtspMsg]strSubMsg=%s\n",strSubMsg.c_str());
		
		int nPos1 = -1;
		if(strSubMsg.find("timeout") >= 0)
		{
			nPos1 =	strSubMsg.find(";");
		}
		else
		{
			nPos1 =	strSubMsg.find("\r\n");
		}
		if (nPos1 == -1)
		{
			nPos1 =	strSubMsg.find("\n");
		}
		string strSubMsg1("");
		if(nPos1 >= 0)
		{
			printf2File("nPos1=%d\n",nPos1);
			strSubMsg1.append(strSubMsg.c_str(),nPos1);
			printf2File("[CRtspSocket::DealRtspMsg]strSubMsg1=%s\n",strSubMsg1.c_str());
			m_strSessionID = strSubMsg1;
			printf2File("[CRtspSocket::DealRtspMsg]m_nSessionID=%s\n",m_strSessionID.c_str());
		}
	}
	printf2File("[CRtspSocket::DealRtspMsg]m_nRtpPort:%d m_nRtcpPort:%d m_strSessionID:%s\n",m_nRtpPort,m_nRtcpPort,m_strSessionID.c_str());
}

void CRtspSocket::DealDescribeMsg(string strMsg)
{
	int nPos = -1;
	nPos = strMsg.find("track");
	if(nPos >= 0)
	{
		printf("nPos=%d\n",nPos);

		string strSubMsg;
		strSubMsg.append(strMsg.c_str()+nPos,strMsg.size() - nPos);

		printf("strSubMsg=%s\n",strSubMsg.c_str());

		int nPos1 = -1;
		nPos1 =	strSubMsg.find("\r\n");
		string strSubMsg1("");
		if(nPos1 >= 0)
		{
			strSubMsg1.append(strSubMsg.c_str(),nPos1);
			m_strTrackID = strSubMsg1;
			printf("m_strTrackID=%s\n",m_strTrackID.c_str());
		}
	}
}

//获取RandRtpPort
int CRtspSocket::GetRandRtpPort()
{
	srand( (unsigned)time( NULL ) );
#ifndef WIN32
	sleep(1);
#else
	Sleep(1000);
#endif
	//getpid();
	int nRandCode = 1888 + (int)(1000.0*rand()/(RAND_MAX+1.0));
	return nRandCode;
}

int CRtspSocket::DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen)
{
                int nDstLen;            // 输出的字符计数
                int nValue;             // 解码用到的长整数
                int i;

                i = 0;
                nDstLen = 0;

                // 取4个字符，解码到一个长整数，再经过移位得到3个字节
                while (i < nSrcLen)
                {
                        nValue = DeBase64Tab[*pSrc++] << 18;
                        nValue += DeBase64Tab[*pSrc++] << 12;
                        *pDst++ = (nValue & 0x00ff0000) >> 16;
                        nDstLen++;

                        if (*pSrc != '=')
                        {
                            nValue += DeBase64Tab[*pSrc++] << 6;
                            *pDst++ = (nValue & 0x0000ff00) >> 8;
                            nDstLen++;

                            if (*pSrc != '=')
                            {
                                nValue += DeBase64Tab[*pSrc++];
                                *pDst++ =nValue & 0x000000ff;
                                nDstLen++;
                            }
                        }
                        i += 4;
                 }

                return nDstLen;
}
