#include "EventVisVideo.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (~0)
#endif


CEventVisVideo::CEventVisVideo()
{
	m_ClientSocket=INVALID_SOCKET;
	m_dwVisPort = 0;
	m_dwCameraID = 0;
	m_dwHeartBeatTime = 0;
}
CEventVisVideo::~CEventVisVideo()
{
	CloseSocket(m_ClientSocket);
	m_listEvent.clear();
}

/*************************************
功  能：设置VIS服务器
参  数：szVisIp    -- 服务器地址
		dwVisPort  -- 服务器端口
		dwCameraID -- 摄像机编号
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::SetVisInfo(const char* szFtpUserName,const char* szFtpPwd,const char *szVisIp,int dwVisPort,char* szCameraID)
{
	memset(m_szFtpUserName,0,50);
	sprintf(m_szFtpUserName,"%s",szFtpUserName);
	memset(m_szFtpPwd,0,50);
	sprintf(m_szFtpPwd,"%s",szFtpPwd);


	memset(m_szVisIp,0,50);
	sprintf(m_szVisIp,"%s",szVisIp);
	m_dwVisPort = 4503;
	LogNormal("CameraID=%s \n",szCameraID);
	m_dwCameraID = roadatoull(szCameraID);//13882412423395158526;//dwCameraID;
	LogNormal("VisIP: %s:%d,CameraID=%llu \n",szVisIp,dwVisPort,m_dwCameraID);
	return 0;
}

/*************************************
功  能：添加请求视频信息
参  数：dwEventBeginTime -- 开始时间
		dwEventEndTime   -- 结束时间
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::AddEventVideoReq(int dwEventBeginTime,int dwEventEndTime,const char* szDeviceId,int dwDir,int dwEventTime)
{
	LogNormal("AddEventVideoReq:BeginTime=%d[%s] \n",dwEventBeginTime,GetTime(dwEventBeginTime).c_str());
	LogNormal("AddEventVideoReq:EndTime=%d[%s] \n",dwEventEndTime,GetTime(dwEventEndTime).c_str());
	strEventVisVidoe strReq;
	strReq.dwBeginTime = dwEventBeginTime;
	strReq.dwEntTime   = dwEventEndTime;
	memset(strReq.szDeviceId,0,30);
	sprintf(strReq.szDeviceId,"%s",szDeviceId);

	strReq.dwDir = dwDir;

	string strTime = GetTime(dwEventTime,2);	
	memset(strReq.szEventTime,0,30);
	sprintf(strReq.szEventTime,"%s",strTime.c_str());
	strReq.dwResult    = 0;
	strReq.dwSendTime  = GetTimeT();

	m_listEvent.push_back(strReq);


	return 0;
}
/*************************************
功  能：添加请求视频业务 比如短线重连，删除冗余请求
参  数：
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::DealEventVideo()
{
	if(m_ClientSocket == INVALID_SOCKET)
	{
		ConnectToVis(m_szVisIp,m_dwVisPort);
	}

	RecvMsg();

	if( (GetTimeT() - m_dwHeartBeatTime) > 30 )
	{
		m_dwHeartBeatTime = GetTimeT();
		if(SentHeartBeat() < 0 )
		{
			CloseSocket(m_ClientSocket);
			return 0;
		}
	}

	if(DealEvent() < 0)
	{
		CloseSocket(m_ClientSocket);
		return 0;
	}
	return 0;
}

int CEventVisVideo::DealEvent()
{
	if(m_listEvent.size() == 0)
	{
		return 0;
	}
	strEventVisVidoe strEvent = m_listEvent.front();
	if(( GetTimeT()-strEvent.dwSendTime) < 180 )
	{
		return 0;
	}
	m_listEvent.pop_front();

	char szBuf[131] = {'\0'};

	strVisMsgHead strHead;
	strHead.bHead1 = 0xDF;
	strHead.bHead2 = 0xDF;
	strHead.bVersion = 0x01;
	strHead.bMsgType = ROADEVENT_UPLOAD_FTP;
	strHead.bLength  = 124;
	strHead.bChecksum = (strHead.bHead1 + strHead.bHead2 + strHead.bVersion + strHead.bMsgType + strHead.bLength) & 0xFF;

	memcpy(szBuf,&strHead,sizeof(strVisMsgHead));

	REQUPLOADVIDEO tReqUpLoadVideo;
	tReqUpLoadVideo.ubiSessionId = GetTimeT();
	tReqUpLoadVideo.ubiCameraId = m_dwCameraID;
	tReqUpLoadVideo.uiStartTime = strEvent.dwBeginTime;
	tReqUpLoadVideo.uiEndTime   = strEvent.dwEntTime;
	memset(tReqUpLoadVideo.szFtpUserName,0,20);
	sprintf(tReqUpLoadVideo.szFtpUserName,"%s",m_szFtpUserName);
	memset(tReqUpLoadVideo.szFtpPassword,0,20);
	sprintf(tReqUpLoadVideo.szFtpPassword,"%s",m_szFtpPwd);
	memset(tReqUpLoadVideo.szDeviceCode,0,20);
	sprintf(tReqUpLoadVideo.szDeviceCode,"%s",strEvent.szDeviceId);
	memset(tReqUpLoadVideo.szDriveDir,0,10);
	sprintf(tReqUpLoadVideo.szDriveDir,"%d",strEvent.dwDir);
	memset(tReqUpLoadVideo.szLaneFlag,0,10);
	memset(tReqUpLoadVideo.szDataTime,0,20);
	sprintf(tReqUpLoadVideo.szDataTime,"%s",strEvent.szEventTime);

	LogNormal("[%s:%s]: BeginTime:%d,EndTime:%d,CameraID:%d \n",tReqUpLoadVideo.szFtpUserName,tReqUpLoadVideo.szFtpPassword,tReqUpLoadVideo.uiStartTime,tReqUpLoadVideo.uiEndTime,tReqUpLoadVideo.ubiCameraId);
	
	memcpy(szBuf+sizeof(strVisMsgHead),&tReqUpLoadVideo,sizeof(REQUPLOADVIDEO));

	int nCount = SendMsg(szBuf,sizeof(strVisMsgHead)+sizeof(REQUPLOADVIDEO));
	LogNormal("SendMsgLength:%d \n",nCount);
	return nCount;
}

int CEventVisVideo::SentHeartBeat()
{
	//LogNormal("SentHeartBeat\n");
	strVisMsgHead strHead;
	strHead.bHead1 = 0xDF;
	strHead.bHead2 = 0xDF;
	strHead.bVersion = 0x01;
	strHead.bMsgType = ROADEVENT_HEART_BEAT;
	strHead.bLength  = 0;
	strHead.bChecksum = (strHead.bHead1 + strHead.bHead2 + strHead.bVersion + strHead.bMsgType + strHead.bLength) & 0xFF;

	char szBuf[7] = {'\0'};
	memcpy(szBuf,&strHead,sizeof(strVisMsgHead));
	return SendMsg(szBuf,sizeof(strVisMsgHead));
}

int CEventVisVideo::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}

/*************************************
功  能：连接VIS服务器
参  数：szVisIp    -- 服务器地址
		dwVisPort  -- 服务器端口
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::ConnectToVis(char *szVisIp,int dwVisPort)
{
	if (m_ClientSocket !=0)
	{
		CloseSocket(m_ClientSocket);
	}
	sockaddr_in tmpsockaddr;
	tmpsockaddr.sin_family = AF_INET;
	tmpsockaddr.sin_port = htons(dwVisPort);
	tmpsockaddr.sin_addr.s_addr = inet_addr(szVisIp);

	int iRet,iSocket;
	iSocket = socket(AF_INET, SOCK_STREAM, 0);	//建立socket

	if (iSocket < 0)
	{
		printf("create socket failed [%s],Ip = %s,Port = %d\n",strerror(errno),szVisIp,dwVisPort);
		return -1;
	}
	int nCtlMod = 0x01;
	ioctl(iSocket,FIONBIO,&nCtlMod);
	//设置SOCKET缓冲区大小
	//int rcvbufsize = MAXSIZE;
	//setsockopt(iSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&rcvbufsize, sizeof(rcvbufsize));
	if((iRet = connect(iSocket,(sockaddr*)&tmpsockaddr,sizeof(tmpsockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(iSocket);
			return -1;
		}
	}
	m_ClientSocket = iSocket;
	printf("%s success [%d]\n",__FUNCTION__,iSocket);
	return 0;
}
/*************************************
功  能：发送数据
参  数：szMsg    -- 发送数据
		uiMsgLen -- 发送数据长度
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::SendMsg(char* szMsg,int uiMsgLen)
{	
	if (m_ClientSocket == INVALID_SOCKET)
	{
		printf("m_ClientSocket is %d !\n",__FUNCTION__,m_ClientSocket);
		return -1;
	}
	int sendbufsize = send(m_ClientSocket, szMsg,uiMsgLen, MSG_NOSIGNAL);//发送数据
	//int sendbufsize = write(m_ClientSocket, szMsg,uiMsgLen);//发送数据
	printf("sendbufsize:%d \n",sendbufsize);
	if (sendbufsize <= 0)
	{
		printf("%s failed !\n",__FUNCTION__);
		return -1;
	}

	return sendbufsize;
}
/*************************************
功  能：发送数据
参  数：
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::RecvMsg()
{	
	if (m_ClientSocket < 0 )
	{
		printf("RecvMsg: m_ClientSocket err \n");
		return -1;
	}
	if (WaitRecv(m_ClientSocket,0.05) == 0)
	{
		char szRecvBuff[512] = {0};
		int recvbufsize = recv(m_ClientSocket,szRecvBuff,512,0);//接收数据
		if (recvbufsize >0 )
		{
			printf("RecvLength: %d \n",recvbufsize);
			return 1;
		}
		
	}
	return 1;
}
/*************************************
功  能：发送数据
参  数：dwSocketFd socket句柄
		fTimeout   超时时间
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::WaitRecv(int dwSocketFd, float fTimeout)
{
	if((dwSocketFd <= 0) || (dwSocketFd > 1024) )
	{
		return -1;
	}
	int nSecond = int(fTimeout);
	int nUSecond = (int)((fTimeout -  nSecond) * 1000000);
	timeval tv;
	tv.tv_sec = nSecond;
	tv.tv_usec = nUSecond;
	fd_set fdread;
	FD_ZERO(&fdread);
	FD_SET(dwSocketFd, &fdread);

	if (select(dwSocketFd + 1, &fdread, NULL, NULL, &tv) <= 0)
	{
		FD_CLR(dwSocketFd, &fdread);
		return -1;
	}

	if (!FD_ISSET(dwSocketFd, &fdread))
	{
		FD_CLR(dwSocketFd, &fdread);
		return -1;
	}

	FD_CLR(dwSocketFd, &fdread);

	return 0;
}
/*************************************
功  能：发送数据
参  数：dwSocketFd socket句柄
返回值：成功返回0，失败返回错误码
*****************************************/
int CEventVisVideo::CloseSocket(int &dwSocketFd)
{
	if (dwSocketFd >0)
	{
		close(dwSocketFd);
	}
	dwSocketFd = INVALID_SOCKET;
	return 0;
}

unsigned long long  CEventVisVideo::roadatoull(const char* szID) 
{ 
	if( szID == NULL ) 
		return -1; 
	unsigned long long id = 0; 
	int ilen = strlen(szID); 

	if( ilen > 20 ) 
		return -1; 
	char cid[2] = {0}; 
	for(int i=0; i<ilen; i++) 
	{ 
		id *= 10; 
		memcpy(cid, &szID[i], sizeof(char)*1); 
		cid[1] = '\0'; 
		id += atoi(cid); 
	} 

	return id; 
}
