// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

/**
    文件：FTPCommunication.cpp
    功能：
    作者：於锋
    时间：2011-2-22
**/

#include "Common.h"
#include "FtpCommunication.h"

CFtpCommunication g_FtpCommunication;


CFtpCommunication::CFtpCommunication()
{
    m_nListenSocket = INVALID_SOCKET;
    m_nControlSocket= INVALID_SOCKET;
    m_nDataSocket=    INVALID_SOCKET;
    m_bConnected     = 0;
    m_bLogin = 0;
    m_nSendPort = 1;
    m_nMode = BINARY;

    pthread_mutex_init(&m_SendMutex, NULL);
}

void CFtpCommunication::SetConfig(FTP_HOST_INFO *pFtp)
{
	if (NULL == pFtp)
	{
		m_strFtpHost = g_strFtpServerHost;
		m_strFtpUser = g_strFtpUserName;
		m_strFtpPasswd = g_strFtpPassWord;
		m_nFtpPort = g_nFtpPort;
	}
	else
	{
		m_strFtpHost = pFtp->chFtpServerHost;
		m_strFtpUser = pFtp->chFtpUserName;
		m_strFtpPasswd = pFtp->chFtpPassword;
		m_nFtpPort = pFtp->uFtpPort;
	}
}

CFtpCommunication::~CFtpCommunication()
{
    pthread_mutex_destroy(&m_SendMutex);
}

int CFtpCommunication::ConnectToServer(char *name, int port)
{
  int s;
  unsigned int portnum;


  struct sockaddr_in server;

  portnum = port;

  bzero((char *) &server, sizeof(server));


   server.sin_family      = AF_INET;

   server.sin_addr.s_addr = inet_addr(name);

   server.sin_port        = htons(portnum);



  /* create socket */

  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 1)
  {
	  perror("socket");
      return INVALID_SOCKET;
  }

  struct timeval timeo;
  socklen_t len = sizeof(timeo);
  timeo.tv_sec=5;
  timeo.tv_usec=0;//超时5s
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeo, len);
  setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);


  if(connect(s,(struct sockaddr *)&server, sizeof(server))< 0)
  {
      shutdown(s, 2);
      close(s);
	  perror("connect");
      return INVALID_SOCKET;
  }

  setsockopt(s,SOL_SOCKET,SO_LINGER,0,0);

  setsockopt(s,SOL_SOCKET,SO_REUSEADDR,0,0);

  //setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,0,0);

  //m_nDataSocket = s;

  return s;
}

int CFtpCommunication::SendControlMsg(char *szBuffer, int len)
{
   if( send(m_nControlSocket,szBuffer,len,MSG_NOSIGNAL) <= 0)
       return 0;

   return 1;
}

int CFtpCommunication::ReadControlMsg( char *szBuffer, int len)
{
   int ret = 0;

   ret=recv(m_nControlSocket,szBuffer,len,MSG_NOSIGNAL);

   return ret;
}

int CFtpCommunication::CheckControlMsg( char *szPtr, int len)
{
    return recv(m_nControlSocket,szPtr,len,MSG_PEEK|MSG_NOSIGNAL);
}

int CFtpCommunication::ReadDataMsg( char *szBuffer, int len)
{
   int ret;
   if( (ret=recv(m_nDataSocket,szBuffer,len,MSG_NOSIGNAL)) <= 0)
       return 0;

   return ret;
}

int CFtpCommunication::SendDataMsg( char *szBuffer, int len)
{
   int nBytes = 0;

   nBytes = send(m_nDataSocket,szBuffer,len,MSG_NOSIGNAL);

   return nBytes;
}

void CFtpCommunication::CloseDataConnection( void )
{
      shutdown(m_nDataSocket,2);
      close(m_nDataSocket);
      m_nDataSocket = INVALID_SOCKET;
}

void CFtpCommunication::CloseControlConnection( void )
{
      shutdown(m_nControlSocket,2);
      close(m_nControlSocket);
      m_nControlSocket = INVALID_SOCKET;

      m_bConnected = 0;
	  m_bLogin = false;
}

void CFtpCommunication::CloseListenSocket( void )
{
      shutdown(m_nListenSocket,2);
      close(m_nListenSocket);
      m_nListenSocket = INVALID_SOCKET;
}

int CFtpCommunication::AcceptConnection()
{
    struct sockaddr_in cli_addr;

    socklen_t clilen = sizeof(cli_addr);

    int sockfd;

    sockfd = accept(m_nListenSocket, (struct sockaddr *) &cli_addr,

			   &clilen);

    if (sockfd < 0) {

        perror("accept");

	return INVALID_SOCKET;

    }

    m_nDataSocket = sockfd;

    CloseListenSocket();

    return sockfd;
}

int CFtpCommunication::analysis_ipaddr(char *str, char *re_addr, int *re_port)
{
	//static char addr[16];
	printf("str=====%s\n",str);
	int ip[4];
	int port[2];
	int m;
	int i = 0;
	sscanf(str, "%d,%d,%d,%d,%d,%d", &ip[0],  &ip[1], &ip[2], &ip[3], &port[0], &port[1]);
	while(i < 4) {
		if((ip[i] > 255) || (ip[i] < 0)) {
			return -1;
		}
		i++;
	}
	m = port[0]*256 + port[1];
	if(m > 65535) {
		return -1;
	}
	snprintf(re_addr, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	*re_port = m;
	return 0;
}

int CFtpCommunication::GetPassivePort(string& strHost,int& nPort)
{
    if(m_bConnected ==0 )
    {
        return 0;
    }

    char szBuffer[1024] = {0};

    if(!SendControlMsg("PASV\r\n",6))
    {
        return 0;
    }

   int done = 0, iRetCode = 0;
   string strRetCode;
   memset(szBuffer,0,1024);

   while(!done )
   {

    //printf("****************before GetLine\n");

        iRetCode = GetLine(szBuffer);

         printf("*****************GetLine,iRetCode=%d,szBuffer=%s\n",iRetCode,szBuffer);

         if(iRetCode <= 0)
            break;

        (void)strtok(szBuffer,"\r\n");

        strRetCode.append(szBuffer);

        if( szBuffer[3] != '-' && iRetCode > 0 )

            done = 1;

        memset(szBuffer,0,1024);
   }

   if(iRetCode == 227)
   {
       char addr[16] = {0};
        //解析ip和端口号
        analysis_ipaddr((char*)(strRetCode.c_str()+27),addr,&nPort);

        strHost.append(addr);
   }
   else
   {
       LogError("strRetCode=%s,strerror=%s\n",strRetCode.c_str(),strerror(errno));
   }
   //LogNormal("strRetCode=%s\n",strRetCode.c_str());
   return iRetCode;
}

int CFtpCommunication::GetListenSocket()
{
    int sockfd, flag=1;
    socklen_t len;

    struct sockaddr_in  serv_addr, TempAddr;

    char *port,*ipaddr;

    char szBuffer[64]={0};

    /*

     * Open a TCP socket (an Internet stream socket).

     */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

	perror("socket");

	return INVALID_SOCKET;

    }

    /*

     * Fill in structure fields for binding

     */

   if( m_nSendPort ) {

       bzero((char *) &serv_addr, sizeof(serv_addr));

       serv_addr.sin_family      = AF_INET;

       serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

       serv_addr.sin_port        = htons(0); /* let system choose */

   }


    /*

     * bind the address to the socket

     */

    if (bind(sockfd,(struct sockaddr *)&serv_addr,

	     sizeof(serv_addr)) < 0) {

	perror("bind");

	close(sockfd);

	return INVALID_SOCKET;

    }

    len = sizeof(serv_addr);

    if(getsockname(sockfd,

		   (struct sockaddr *)&serv_addr,

		   &len)<0) {

       perror("getsockname");

	   close(sockfd);

       return INVALID_SOCKET;

    }

    len = sizeof(TempAddr);

    if(getsockname(m_nControlSocket,

		   (struct sockaddr *)&TempAddr,

		   &len)<0) {

       perror("getsockname");

	   close(sockfd);

       return INVALID_SOCKET;

    }

    ipaddr = (char *)&TempAddr.sin_addr;

    port  = (char *)&serv_addr.sin_port;



#define  UC(b)  (((int)b)&0xff)


    sprintf(szBuffer,"PORT %d,%d,%d,%d,%d,%d\r\n",

          UC(ipaddr[0]), UC(ipaddr[1]), UC(ipaddr[2]), UC(ipaddr[3]),

          UC(port[0]), UC(port[1]));

    /*

     * allow ftp server to connect

     * allow only one server

     */

    if( listen(sockfd, 1) < 0) {

	perror("listen");

	close(sockfd);

	return INVALID_SOCKET;

    }

    SendControlMsg(szBuffer,strlen(szBuffer));

    int iRetCode = GetReply();
    if(iRetCode != 200)//发送port命令不成功
    {
        close(sockfd);
        return INVALID_SOCKET;
    }

    m_nListenSocket = sockfd;

    return sockfd;

}
int CFtpCommunication::GetReply(char* szData)
{
	int done = 0, iRetCode = 0;
	char szBuffer[1024] = {0};
	//memset(szBuffer,0,1024);

	while(!done ) {
		memset(szBuffer,0,1024);
		iRetCode = GetLine(szBuffer);

		printf("*****************GetLine,iRetCode=%d,szBuffer=%s\n",iRetCode,szBuffer);

		if(iRetCode <= 0)
			break;

		(void)strtok(szBuffer,"\r\n");

		char* szCmdData = strtok(szBuffer,"\r\n");
		char* szTmpData = strtok(szCmdData," ");
		szTmpData = strtok(NULL," ");
		strcpy(szData, szTmpData);

		if( (szBuffer[3] != '-') && (iRetCode > 0) )
			done = 1;
	}
	//LogNormal("strRetCode= %d,%s",strRetCode.size(),strRetCode.c_str());

	return iRetCode;
}

int CFtpCommunication::GetReply()
{
   int done = 0, iRetCode = 0;
   string strRetCode;
   char szBuffer[1024] = {0};
   //memset(szBuffer,0,1024);

   while(!done ) {

    //printf("****************before GetLine\n");
    memset(szBuffer,0,1024);

    iRetCode = GetLine(szBuffer);

     printf("*****************GetLine,iRetCode=%d,szBuffer=%s\n",iRetCode,szBuffer);

    if(iRetCode <= 0)
        break;

	(void)strtok(szBuffer,"\r\n");

	strRetCode.append(szBuffer);

	if( (szBuffer[3] != '-') && (iRetCode > 0) )

	    done = 1;
   }
   //LogNormal("strRetCode= %d,%s",strRetCode.size(),strRetCode.c_str());

   return iRetCode;
}

int CFtpCommunication::GetLine(char* szBuffer)
{

   int done=0, iRetCode =0, iLen, iBuffLen=0;

   char *szPtr = szBuffer, nCode[4]={0},ch=0;



   while( (iBuffLen < 1024) &&
	  (CheckControlMsg(&ch,1)  > 0) )
   {
        iLen = ReadControlMsg(&ch,1);

        if(iLen > 0)
        {
            iBuffLen += iLen;

            *szPtr = ch;

            szPtr += iLen;

            if( ch == '\n' )

                break;    // we have a line: return
        }
        else
        {
            return 0;
        }
   }

   *(szPtr+1) = '\0';

   strncpy(nCode, szBuffer, 3);

   return (atoi(nCode));
}

int  CFtpCommunication::CheckFds( )
{
    if(m_bConnected ==0 )
    {
        //LogError("CheckFds not connected!\r\n");
        return 0;
    }
    else
    {
        return 1;
    }

 int rval, i;
 fd_set readfds, writefds, exceptfds;
 struct timeval timeout;

  char szBuffer[1024] = {0};
  memset(szBuffer,0,1024);
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);

/*#if (!defined(WIN32) || !defined(_WIN32) )
  FD_CLR (fileno(stdin),&readfds);
#endif*/

  if( m_nControlSocket > 0)
      FD_CLR (m_nControlSocket,&readfds);
  timeout.tv_sec = 0 ;                /* 1-second timeout. */
  timeout.tv_usec = 0 ;               /* 0 microseconds.  */

/*#if (!defined( _WIN32 ) || !defined( WIN32 ) )
  FD_SET(fileno(stdin),&readfds);
#endif*/

  if( m_nControlSocket > 0)
      FD_SET(m_nControlSocket,&readfds);

  i=select ((m_nControlSocket > 0) ? (m_nControlSocket+1) : 1,
	         &readfds,
			 &writefds,
			 &exceptfds,
			 &timeout);
 /* SELECT interrupted by signal - try again.  */
if (errno == EINTR && i ==-1)  {
   /*memset(command,0,1024);*/
   //printf("errno == EINTR && i ==-1\r\n");
   LogError("SELECT interrupted by signal\r\n");
   CloseControlConnection();
   CloseDataConnection();
   //CloseListenSocket();
   m_bConnected = 0;
   m_bLogin = false;
   return 0;
}
printf(" select======================i=%d======\r\n",i);

if( (m_nControlSocket > 0) && FD_ISSET(m_nControlSocket, &readfds) )
{
    printf(" FD_ISSET=============================\r\n");
  if ( ( rval = ReadControlMsg(szBuffer,1024))  > 0)
	  {
	  //printf(szBuffer);
	  LogError("FD_ISSET==%s",szBuffer);

      CloseControlConnection();
      CloseDataConnection();
      //CloseListenSocket();
	  m_bConnected = 0;
	  m_bLogin = false;
	  return 0;
	  }
  else {
	 LogError("Connection closed by server\r\n");

	 CloseControlConnection();
	 CloseDataConnection();
	 //CloseListenSocket();
	 m_bConnected = 0;
	 m_bLogin = false;
	 return 0;
  }

 }

 //printf("after CheckFds=============================\r\n");
 return 1;
}

//PutFile升级版
bool CFtpCommunication::VideoPutFile(char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite)
{
   //char szBuffer[4096] = {0};
	const unsigned int uiBufLen = 4096;
	char szBuffer[uiBufLen] = {0};
	FILE *fp=NULL;
	int fd, nTotal=0, nBytesRead=0, retval, aborted=0;
	char *abortstr = "ABOR\r\n", ch;

  /* void (*OldHandler)(int); */

   if( (chLocalFilePath == NULL) && (strData.size()<= 0 ))
   {
      printf("No file specified.\n");
      return false;
   }

   if(chLocalFilePath != NULL)
   {
       if(! (fp=fopen(chLocalFilePath,"rb")))
       {
          perror("file open");
          return false;
       }
   }

   //pasv方式
   string strHost;
   int nPort;
   if( GetPassivePort(strHost,nPort) != 227)
   {
       if(fp)
       fclose(fp);

       CloseControlConnection();

       LogError("PASV error 2\n");
       return false;
   }

   //connect to server
    m_nDataSocket = ConnectToServer((char*)strHost.c_str(),nPort);
    if(m_nDataSocket <= 0)
    {
         if(fp)
         fclose(fp);

         CloseControlConnection();

         LogError("data connect error \n");
         return false;
    }
   /*
    * send command to server & read reply
    */
	int nRetCode=0;
	bool bSuccess = false; // FTP协议设置成功失败标识

	// 获取文件大小，通过返回值判断文件是否已存在，以便于判断是否需要断点续传
	memset(szBuffer, 0, uiBufLen);
	sprintf(szBuffer,"SIZE %s\r\n",chRemoteFilePath);

	if(SendControlMsg(szBuffer,strlen(szBuffer)))
	{
		char szFileLen[100] = {0};
		
		if(((nRetCode = GetReply(szFileLen)) == 213)&&(fp != NULL))
		{ // 返回成功，则执行断点续传
			//LogNormal("ftp resume start!nRetCode = %d\n",nRetCode);
//			LogWarning("FTP Resume file:%s, len:%s \n", chRemoteFilePath, szFileLen);
			unsigned int uiLen = atoi(szFileLen);
			printf("SIZE GetReply uiLen:%u\n", uiLen);
			// 重新定位文件，从指定位置开始上传文件
			fseek(fp, uiLen, SEEK_SET);
			memset(szBuffer, 0, uiBufLen);
			sprintf(szBuffer,"REST %u\r\n",uiLen);
			if(SendControlMsg(szBuffer,strlen(szBuffer)))
			{
				if((nRetCode = GetReply()) < 400)
				{ // 返回正确
					printf("REST GetReply ret:%d \n", nRetCode);
					memset(szBuffer, 0, uiBufLen);
					sprintf(szBuffer,"APPE %s\r\n",chRemoteFilePath);
					if(SendControlMsg(szBuffer,strlen(szBuffer)))
					{
						if((nRetCode = GetReply()) < 300)
						{ // 返回正确
							printf("APPE GetReply ret:%d \n", nRetCode);
							bSuccess = true; // 设置成功
							//LogNormal("ftp resume success!\n");
						}
					}
					else {
						LogError("APPE error nRetCode=%d\n",nRetCode);
					}
				}
				else {
					LogError("REST error nRetCode=%d\n",nRetCode);
				}
			}
		}
		else 
		{ // 不需要断点续传，直接上传
			memset(szBuffer, 0, uiBufLen);
			sprintf(szBuffer,"STOR %s\r\n",chRemoteFilePath);
			if(SendControlMsg(szBuffer,strlen(szBuffer)))
			{
				nRetCode = GetReply();
				if(nRetCode = 150 || nRetCode == 125) {
					bSuccess = true; // 设置成功
				}else {
					LogError("STOR error nRetCode=%d\n",nRetCode);
				}
			}
		}
	}

	if (!bSuccess)
	{ // 设置失败，释放资源
		if(fp)
			fclose(fp);

		LogNormal("%s",chRemoteFilePath);
		CloseDataConnection();
		CloseControlConnection();
		return false;
	}	
	//sprintf(szBuffer,"STOR %s\r\n",chRemoteFilePath);
 //  if(!SendControlMsg(szBuffer,strlen(szBuffer)))
 //  {
 //     if(fp)
 //     fclose(fp);

 //    // CloseListenSocket();
 //    CloseDataConnection();
 //    CloseControlConnection();
 //     return false;
 //  }
 //  int nRetCode = GetReply();

 //  if((nRetCode != 150) && (nRetCode != 125))
 //  {
 //     if(fp)
 //     fclose(fp);

 //     LogError("STOR error nRetCode=%d\n",nRetCode);
 //     CloseDataConnection();
 //     CloseControlConnection();

 //     return false;
 //  }

   /*
    * now send file
    */

  // printf("Type q and hit return to abort\r\n");
   bool bRet = true;

   if(fp)
   {
       int nBytes = 0;
       while( (nBytesRead=fread(szBuffer,1,4096,fp)) > 0)
       {
          // printf("===nBytesRead=%d\r\n",nBytesRead);
          nBytes = SendDataMsg(szBuffer,nBytesRead);

          if(nBytes <= 0)
          {
              bRet = false;
              break;
           }

          nTotal+=nBytes;
          //printf("%s : %d sent\r\n",chLocalFilePath,nTotal);
       }
       printf("===nBytesRead=%d\r\n",nBytesRead);
   }
   else
   {
       //LogNormal("%s\n",chRemoteFilePath);
	   if(strData.size() > 0)
       {
           int nBytes = 0;
           int nLeft = strData.size();
           //printf("strData.c_str()=%s\r",strData.c_str());
           while (nLeft > 0)
           {
              nBytes = SendDataMsg((char*)(strData.c_str()+nTotal),nLeft);

              if(nBytes <= 0)
              {
                 bRet = false;
                 break;
              }

              nTotal+= nBytes;
              nLeft -= nBytes;
              printf("left=%d nTotal=%d sent\r",nLeft,nTotal);
           }
       }
	   
   }

   /*(void)signal(SIGINT,OldHandler); */

   /*
    * close data connection
    */


   CloseDataConnection();
   int iRetCode = GetReply();

   if(iRetCode != 226)//226 transfer complete
   {
       bRet = false;
   }
   else
   {
	   printf("strData.size()=%d nTotal=%d sent\r",strData.size(),nTotal);
	   if (nTotal == strData.size())
	   {
		   string strPathTmp = chRemoteFilePath;

	   }
   }


   if(!bRet)
   {
      LogNormal("===send error\n");
      CloseControlConnection();
   }

   if(fp)
   fclose(fp);

   return bRet;
}

/*
 * DoOpen
 * this function is called when the o,open command is issued.
 * it connects to the requested host, and logs the user in
 *
 */
bool CFtpCommunication::DoOpen( char *szHost,int nPort,char *User,char* szPass)
{
    //CloseControlConnection();
    //CloseDataConnection();
   /*
    * do not do anything if we are already connected.
    */
   if( m_bConnected ) {
       printf("Already connected.  Close connection first.\n");
       fflush(stdout);
       return true;
   }

   //printf("Connecting to %s\n",szHost);
   m_nControlSocket = ConnectToServer(szHost,nPort);

    usleep(1000*500);

   if( m_nControlSocket > 0)
   {
     printf("Connected to %s,%d\n",szHost,nPort);

     int iRetCode = GetReply();            /* get reply (welcome message) from server */

     if(iRetCode == 220) //登录成功
     {
         m_bConnected = true;         /* we ar now connected */

          if(DoLogin(User,szPass)) /* prompt for username and password */
          {
            if(DoBinary() == 200)            /* default binary mode */
            {
                return true;
            }
          }
     }
     else
     {
         CloseControlConnection();
     }
   }

    LogError("Open Failed!\r\n");

    m_bConnected = false;
    m_bLogin = false;
    return false;
}

/*
 * DoLogin
 * this function logs the user into the remote host.
 * prompts for username and password
 */
bool CFtpCommunication::DoLogin( char *User,char* szPass)
{
    char szBuffer[1024] = {0};
   if( m_bConnected )  {
     /*
      * send user name & password to server  & get reply message
      */
     sprintf(szBuffer,"USER %s\r\n",User);
     if(!SendControlMsg(szBuffer,strlen(szBuffer)))
     {
         LogError("send USER strerror(errno) = %s\r\n",strerror(errno));
         CloseControlConnection();
         return false;
     }
     int iRetCode = GetReply();
     if(iRetCode != 331)
     {
         CloseControlConnection();
         return false;
     }

     sprintf(szBuffer,"PASS %s\r\n",szPass);
     if(!SendControlMsg(szBuffer,strlen(szBuffer)))
     {
         CloseControlConnection();
         return false;
     }
     iRetCode = GetReply();

     if(iRetCode == 230)
     {
         m_bLogin = true;
         return true;
     }
     else
     {
        LogError("Login Failed iRetCode=%d!\r\n",iRetCode);
        CloseControlConnection();
        m_bConnected = false;
        m_bLogin = false;
        return false;
     }
   }
   else
   {
       m_bLogin = false;
       printf("Not Connected.\n");
       return false;
   }
}

/*
 * DoClose
 * closes connection to the ftp server
 */
void CFtpCommunication::DoClose( void )
{
   if( !m_bConnected  ) {
     printf("Not Connected.\n");

   }
   else {
	   SendControlMsg("quit\r\n",6);
	   GetReply();
	   CloseControlConnection();
	   m_bConnected = 0;
	   m_bLogin = false;
   }
}

/*
 * DoBinary
 * set file transfer mode to binary
 */
int CFtpCommunication::DoBinary()
{
  char szBuffer[1024] = {0};
  if( !m_bConnected ) {
      printf("Not Connected.\n");
      return 0;
  }
   sprintf(szBuffer, "TYPE I\r\n");
   if(!SendControlMsg(szBuffer,strlen(szBuffer)))
   {
       CloseControlConnection();
        return 0;
   }
   int iRetCode = GetReply();
   if(iRetCode != 200)
   {
       CloseControlConnection();
        return 0;
   }
   printf("File transfer modes set to binary.\n");
   m_nMode = BINARY;

   return iRetCode;
}

//DoPut升级版
bool CFtpCommunication::VideoDoPut( char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite,bool bPutFile,char* chRemoteDirPath,bool bMkdir)
{
  bool bRet = false;

  pthread_mutex_lock(&m_SendMutex);

  int nCount = 0;
  while( !(m_bConnected&&m_bLogin) )//一直等到登录成功为止
  {
     if(DoOpen((char*)m_strFtpHost.c_str(),m_nFtpPort,(char*)m_strFtpUser.c_str(),(char*)m_strFtpPasswd.c_str()))
     {
        break;
     }
     //LogError("第%d次尝试连接ftp服务器失败!\n",nCount+1);
     nCount++;

     if(nCount >=3 ) //尝试3次
     {
		LogError("连接ftp服务器失败!\n");
        pthread_mutex_unlock(&m_SendMutex);
        return false;
     }
     sleep(1);
  }
  //LogNormal("FTP 【IP：%s:%d】USER:%s::%s\n",m_strFtpHost.c_str(),m_nFtpPort,m_strFtpUser.c_str(),m_strFtpPasswd.c_str());

  printf("chLocalFilePath=%s,chRemoteFilePath=%s,MSG_NOSIGNAL=%d\n",chLocalFilePath,chRemoteFilePath,MSG_NOSIGNAL);
 // LogNormal("[%s]:chLocalFilePath = %s\n",__FUNCTION__,chLocalFilePath);
 // LogNormal("[%s]:chRemoteFilePath = %s\n",__FUNCTION__,chRemoteFilePath);
  if(bMkdir)
   {
	   int nRetCode = MKDir(chRemoteDirPath);
       if((nRetCode != 257) && (nRetCode != 550))
       {
            pthread_mutex_unlock(&m_SendMutex);
            return false;
       }
   }


  if(bPutFile)
  bRet =  VideoPutFile(chLocalFilePath,chRemoteFilePath,strData,bDosite);
  else
  bRet = true;

  pthread_mutex_unlock(&m_SendMutex);

  return bRet;
}

/*noop command*/
int CFtpCommunication::DoNoop()
{
    if(!SendControlMsg("NOOP\r\n",6))
    {
        CloseDataConnection();
        CloseControlConnection();

        return 0;
    }
    else
    {
        int iRetCode = GetReply();

        if(iRetCode != 200)
        {
            CloseDataConnection();
            CloseControlConnection();

            return 0;
        }

        return iRetCode;
    }
}


/*MKD command*/
int CFtpCommunication::DoMKD(char *chRemoteDirPath)
{
    LogNormal("[%s]:chRemoteDirPath = %s\n",__FUNCTION__,chRemoteDirPath);
	char szBuffer[1024] = {0};
    sprintf(szBuffer,"MKD %s\r\n",chRemoteDirPath);
    if(!SendControlMsg(szBuffer,strlen(szBuffer)))
    {
        CloseDataConnection();
        CloseControlConnection();

        return 0;
    }
    else
    {
        int iRetCode = GetReply();

        if((iRetCode != 257) && (iRetCode != 550))
        {
            CloseDataConnection();
            CloseControlConnection();

            return 0;
        }

        return iRetCode;
    }
}

/*
 * site command
 */
int CFtpCommunication::DoSite(char *command,string& strRetCode)
{
  char szBuffer[1024] = {0};
  if( !m_bConnected ) {
      printf("Not Connected.\n");
      return 0;
  }
  sprintf(szBuffer, "SITE %s\r\n",command);
  printf("%s",szBuffer);
  if(!SendControlMsg(szBuffer,strlen(szBuffer)))
  {
     return 0;
  }
  //int iRetCode = GetReply();
   int done = 0, iRetCode = 0;

   memset(szBuffer,0,1024);

   while(!done ) {

    //printf("****************before GetLine\n");

        iRetCode = GetLine(szBuffer);

     printf("*****************GetLine,iRetCode=%d,szBuffer=%s\n",iRetCode,szBuffer);

     if(iRetCode <= 0)
        break;

	(void)strtok(szBuffer,"\r\n");

	strRetCode.append(szBuffer);

	if( szBuffer[3] != '-' && iRetCode > 0 )

	    done = 1;

	memset(szBuffer,0,1024);
   }
   //LogNormal("DoSite=%s",strRetCode.c_str());
   return iRetCode;
}






//MKD command 创建多级目录
int CFtpCommunication::MKDir(char *chRemoteDirPath)
{
	int iRetCode = 0;
    char *temp_chRemoteDirPath = chRemoteDirPath;
	string strRemoteDirPath = "";
    while(*temp_chRemoteDirPath != 0)
    {
        char szBuffer[1024] = {0};
        int i = 0;
        while(*temp_chRemoteDirPath != '/' && (*temp_chRemoteDirPath != 0))
        {
                szBuffer[i] = *temp_chRemoteDirPath;
                temp_chRemoteDirPath++;
                i++;
        }
		if(*temp_chRemoteDirPath == 0)
		{
			break;
		}

		string strPath(szBuffer);
		
		if(szBuffer[0] != '.')
		{
			strRemoteDirPath += "/";
		}
		strRemoteDirPath += strPath;

        char chPath[1024] = {0};
		
        sprintf(chPath,"MKD %s\r\n",strRemoteDirPath.c_str());
		printf("chPath=%s\n",chPath);
		
		if(!SendControlMsg(chPath,strlen(chPath)))
		{
			CloseDataConnection();
			CloseControlConnection();
			return 0;
		}
		else
		{
			iRetCode = GetReply();
			if((iRetCode != 257) && (iRetCode != 550))
			{
				CloseDataConnection();
				CloseControlConnection();

				return 0;
			}
		}

        temp_chRemoteDirPath++;
    }
	return iRetCode;
}



/*
 * DoCD
 * chang to another directory on the remote system
 */
void CFtpCommunication::DoCD( char *command)
{
char szBuffer[1025]={0};
   char *dir=&command[0];

   if( !m_bConnected ) {
       printf("Not Connected.\n");
       return;
   }

   /*
    * ignore leading whitespace
    */
   while( *dir && (*dir == ' ' || *dir == '\t') ) 
       dir++;

   /*
    * if dir is not specified, read it in
    */
   if( ! (*dir) ) {
      printf("Remote directory:");
      fgets(szBuffer,1024,stdin);
      (void)strtok(szBuffer,"\n");
      dir = (char *)strdup(szBuffer);
      while( *dir && (*dir) == ' ') 
       dir++;
      if( !(*dir) ) {
	printf("Usage: cd remote-directory\n");
	return;
      }
   }
   
   /*
    * send command to server and read response
    */
   sprintf(szBuffer, "CWD %s\r\n",dir);
   SendControlMsg(szBuffer,strlen(szBuffer));
   int ret = GetReply();
}



/*
 * PutFile
 * called to transfer a file to the remote host using the current
 * file transfer mode.  it's just like GetFile.
 *
 * i have commented out lines that would have helped with trapping
 * ctrl-c because longjmp did not work :((
 * if you figure it out, then uncomment them
 */

bool CFtpCommunication::PutFile(char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite)
{
	LogNormal("[%s]:chLocalFilePath = %s,chRemoteFilePath = %s\n",chLocalFilePath,chRemoteFilePath);
	LogNormal("[%s]:strData.size = %d\n",strData);
	const unsigned int uiBufLen = 4096;
   char szBuffer[uiBufLen] = {0};
   FILE *fp=NULL;
   int fd, nTotal=0, nBytesRead=0, retval, aborted=0;
   char *abortstr = "ABOR\r\n", ch;

  /* void (*OldHandler)(int); */

   if( (chLocalFilePath == NULL) && (strData.size()<= 0 ))
   {
      printf("No file specified.\n");
      return false;
   }

   if(chLocalFilePath != NULL)
   {
       if(! (fp=fopen(chLocalFilePath,"rb")))
       {
          perror("file open");
          return false;
       }
   }

   //pasv方式
   string strHost;
   int nPort;
   if( GetPassivePort(strHost,nPort) != 227)
   {
       if(fp)
       fclose(fp);

       CloseControlConnection();

       LogError("PASV error 3\n");
       return false;
   }

   //connect to server
    m_nDataSocket = ConnectToServer((char*)strHost.c_str(),nPort);
    if(m_nDataSocket <= 0)
    {
         if(fp)
         fclose(fp);

         CloseControlConnection();

         LogError("data connect error \n");
         return false;
    }

	int nRetCode=0;
	bool bSuccess = false; // FTP协议设置成功失败标识

	// 获取文件大小，通过返回值判断文件是否已存在，以便于判断是否需要断点续传
	memset(szBuffer, 0, uiBufLen);
	sprintf(szBuffer,"SIZE %s\r\n",chRemoteFilePath);
	if(SendControlMsg(szBuffer,strlen(szBuffer)))
	{
		char szFileLen[100] = {0};
		if((nRetCode = GetReply(szFileLen)) == 213)
		{ // 返回成功，则执行断点续传
//			LogWarning("FTP Resume file:%s, len:%s \n", chRemoteFilePath, szFileLen);
			unsigned int uiLen = atoi(szFileLen);
			printf("SIZE GetReply uiLen:%u\n", uiLen);
			// 重新定位文件，从指定位置开始上传文件
			fseek(fp, uiLen, SEEK_SET);
			memset(szBuffer, 0, uiBufLen);
			sprintf(szBuffer,"REST %u\r\n",uiLen);
			if(SendControlMsg(szBuffer,strlen(szBuffer)))
			{
				if((nRetCode = GetReply()) < 400)
				{ // 返回正确
					printf("REST GetReply ret:%d \n", nRetCode);
					memset(szBuffer, 0, uiBufLen);
					sprintf(szBuffer,"APPE %s\r\n",chRemoteFilePath);
					if(SendControlMsg(szBuffer,strlen(szBuffer)))
					{
						if((nRetCode = GetReply()) < 300)
						{ // 返回正确
							printf("APPE GetReply ret:%d \n", nRetCode);
							bSuccess = true; // 设置成功
						}
					}
					else {
						LogError("APPE error nRetCode=%d\n",nRetCode);
					}
				}
				else {
					LogError("REST error nRetCode=%d\n",nRetCode);
				}
			}
		}
		else 
		{ // 不需要断点续传，直接上传
			memset(szBuffer, 0, uiBufLen);
			sprintf(szBuffer,"STOR %s\r\n",chRemoteFilePath);
			if(SendControlMsg(szBuffer,strlen(szBuffer)))
			{
				nRetCode = GetReply();
				if(nRetCode = 150 || nRetCode == 125) {
					bSuccess = true; // 设置成功
				}else {
					LogError("STOR error nRetCode=%d\n",nRetCode);
				}
			}
		}
	}

	if (!bSuccess)
	{ // 设置失败，释放资源
		if(fp)
			fclose(fp);
		
		LogNormal("%s",chRemoteFilePath);
		CloseDataConnection();
		CloseControlConnection();
		return false;
	}	

   /*
    * now send file
    */

  // printf("Type q and hit return to abort\r\n");
   bool bRet = true;

   if(fp)
   {
       int nBytes = 0;
       while( (nBytesRead=fread(szBuffer,1,4096,fp)) > 0)
       {
           //printf("===nBytesRead=%d\r\n",nBytesRead);
          nBytes = SendDataMsg(szBuffer,nBytesRead);

          if(nBytes <= 0)
          {
              bRet = false;
              break;
           }

          nTotal+=nBytes;
          //printf("%s : %d sent\r\n",chLocalFilePath,nTotal);
       }
       printf("===nBytesRead=%d\r\n",nBytesRead);
   }
   else
   {
       if(strData.size() > 0)
       {
           int nBytes = 0;
           int nLeft = strData.size();
           //printf("strData.c_str()=%s\r",strData.c_str());
           while (nLeft > 0)
           {
              nBytes = SendDataMsg((char*)(strData.c_str()+nTotal),nLeft);

              if(nBytes <= 0)
              {
                 bRet = false;
                 break;
              }

              nTotal+= nBytes;
              nLeft -= nBytes;
              printf("left=%d nTotal=%d sent\r",nLeft,nTotal);
           }
       }
   }

   /*(void)signal(SIGINT,OldHandler); */

   /*
    * close data connection
    */
   CloseDataConnection();
   int iRetCode = GetReply();

   if(iRetCode != 226)//226 transfer complete
   {
       bRet = false;
   }


   if(bRet)
   {
      if(bDosite)
      {
          string strRetCode;
          int iRetCode = DoSite("UPLOADNUM 1 1",strRetCode);
          if(iRetCode != 200)
          {
            LogNormal("===DoSite error,strRetCode.size() = %d\n",strRetCode.size());

            if(strncmp((char*)strRetCode.c_str(),"501 SITE option not supported.",30) != 0)
            {
                CloseControlConnection();
            }
          }
      }
   }
   else
   {
      LogNormal("===send error\n");
      CloseControlConnection();
   }

   if(fp)
   fclose(fp);

   return bRet;
}





/*
 * DoPut
 * send a file to the remote host.  calls PutFile(..)
 */
bool CFtpCommunication::DoPut( char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite,bool bPutFile,char* chRemoteDirPath,bool bMkdir)
{
  bool bRet = false;

  pthread_mutex_lock(&m_SendMutex);

  int nCount = 0;
  while( !(m_bConnected&&m_bLogin) )//一直等到登录成功为止
  {
     if(DoOpen((char*)m_strFtpHost.c_str(),m_nFtpPort,(char*)m_strFtpUser.c_str(),(char*)m_strFtpPasswd.c_str()))
     {
        break;
     }
     LogError("第%d次尝试连接ftp服务器失败!\n",nCount+1);
     nCount++;

     if(nCount >=3 ) //尝试3次
     {
		LogError("连接ftp服务器失败!\n");
        pthread_mutex_unlock(&m_SendMutex);
        return false;
     }
     sleep(1);
  }

  printf("chLocalFilePath=%s,chRemoteFilePath=%s,MSG_NOSIGNAL=%d\n",chLocalFilePath,chRemoteFilePath,MSG_NOSIGNAL);
  LogNormal("[%s]:chLocalFilePath=%s\n",__FUNCTION__,chLocalFilePath);
  LogNormal("[%s]:chRemoteFilePath=%s\n",__FUNCTION__,chRemoteFilePath);
  if(bMkdir)
   {
       int nRetCode = DoMKD(chRemoteDirPath);

       if((nRetCode != 257) && (nRetCode != 550))
       {
            pthread_mutex_unlock(&m_SendMutex);
            return false;
       }
   }

  if(bPutFile)
  bRet =  PutFile(chLocalFilePath,chRemoteFilePath,strData,bDosite);
  else
  bRet = true;

  pthread_mutex_unlock(&m_SendMutex);

  return bRet;
}




/*
 * DoGet
 * retrieve a file from the remote host.  calls GetFile(..)
 */
bool CFtpCommunication::DoGet(char *chLocalFilePath,char *chRemoteFilePath)
{
  pthread_mutex_lock(&m_SendMutex);
  bool bRet = true;
  int nCount = 0;
  while( !(m_bConnected&&m_bLogin) )//一直等到登录成功为止
  {
     if(DoOpen((char*)m_strFtpHost.c_str(),m_nFtpPort,(char*)m_strFtpUser.c_str(),(char*)m_strFtpPasswd.c_str()))
     {
        break;
     }
     LogError("第%d次尝试连接ftp服务器失败!\n",nCount+1);
     nCount++;

     if(nCount >=3 ) //尝试3次
     {
		LogError("连接ftp服务器失败!\n");
        pthread_mutex_unlock(&m_SendMutex);
        return false;
     }
     sleep(1);
  }

  printf("chLocalFilePath=%s,chRemoteFilePath=%s,MSG_NOSIGNAL=%d\n",chLocalFilePath,chRemoteFilePath,MSG_NOSIGNAL);

  bRet = GetFile(chLocalFilePath,chRemoteFilePath);


  pthread_mutex_unlock(&m_SendMutex);

  return bRet;
}

bool CFtpCommunication::DoList(char *chRemoteDirPath,string& strFileNameList)
{
	pthread_mutex_lock(&m_SendMutex);
	  bool bRet = true;
	  int nCount = 0;
	  while( !(m_bConnected&&m_bLogin) )//一直等到登录成功为止
	  {
		 if(DoOpen((char*)m_strFtpHost.c_str(),m_nFtpPort,(char*)m_strFtpUser.c_str(),(char*)m_strFtpPasswd.c_str()))
		 {
			break;
		 }
		 LogError("第%d次尝试连接ftp服务器失败!\n",nCount+1);
		 nCount++;

		 if(nCount >=3 ) //尝试3次
		 {
			LogError("连接ftp服务器失败!\n");
			pthread_mutex_unlock(&m_SendMutex);
			return false;
		 }
		 sleep(1);
	  }

	  printf("chRemoteDirPath=%s,MSG_NOSIGNAL=%d\n",chRemoteDirPath,MSG_NOSIGNAL);
	  bRet = GetList(chRemoteDirPath,strFileNameList);

	  pthread_mutex_unlock(&m_SendMutex);

	  return bRet;
}

bool CFtpCommunication::GetList(char *chRemoteDirPath,string& strFileNameList)
{
	char szBuffer[4096] = {0};
	int nTotal=0, nBytesRead=0;

	//pasv方式
   string strHost;
   int nPort;
   if( GetPassivePort(strHost,nPort) != 227)
   {
       CloseControlConnection();

       LogError("PASV error 1\n");
       return false;
   }

   //connect to server
    m_nDataSocket = ConnectToServer((char*)strHost.c_str(),nPort);
    if(m_nDataSocket <= 0)
    {
         CloseControlConnection();

         LogError("data connect error \n");
         return false;
    }

	sprintf(szBuffer,"NLST %s\r\n",chRemoteDirPath);
    if(!SendControlMsg(szBuffer,strlen(szBuffer)))
    {
        CloseDataConnection();
        CloseControlConnection();

        return false;
    }
   
      int nRetCode = GetReply();

      if((nRetCode != 150) && (nRetCode != 125))
      {
           CloseDataConnection();
           CloseControlConnection();
		   LogError("NLST error nRetCode=%d\n",nRetCode);
           return false;
      }

	   while( (nBytesRead=ReadDataMsg(szBuffer,1024)) > 0)
       {
           //printf("===nBytesRead=%d\r\n",nBytesRead);
		  strFileNameList.append(szBuffer,nBytesRead);
          //printf("%s : %d recv\r\n",chLocalFilePath,nTotal);
       }
	
//	   printf("strFileNameList=%s\n",strFileNameList.c_str()); 
//	  printf("strFileNameList.size=%d\n",strFileNameList.size());

	  bool bRet = true;
   /*
    * close data connection
    */
   CloseDataConnection();
   int iRetCode = GetReply();

   if(iRetCode != 226)//226 Directory send OK.
   {
       bRet = false;
   }

   if(!bRet)
   {
      LogNormal("===send error\n");
      CloseControlConnection();
   }
   return bRet;
}


bool CFtpCommunication::GetFile(char *chLocalFilePath,char *chRemoteFilePath)
{
   //LogNormal("chLocalFilePath = %s,chRemoteFilePath =%s\n",chLocalFilePath,chRemoteFilePath);
	char szBuffer[4096] = {0};
   FILE *fp=NULL;
   int fd, nTotal=0, nBytesRead=0, retval, aborted=0;
   char *abortstr = "ABOR\r\n", ch;

  /* void (*OldHandler)(int); */

   if( (chLocalFilePath == NULL))
   {
      printf("No file specified.\n");
      return false;
   }

   if(chLocalFilePath != NULL)
   {
       if(! (fp=fopen(chLocalFilePath,"wb")))
       {
          perror("file open");
          return false;
       }
   }

   //pasv方式
   string strHost;
   int nPort;
   if( GetPassivePort(strHost,nPort) != 227)
   {
       if(fp)
       fclose(fp);

       CloseControlConnection();

       LogError("PASV error 1\n");
       return false;
   }

   //connect to server
    m_nDataSocket = ConnectToServer((char*)strHost.c_str(),nPort);
    if(m_nDataSocket <= 0)
    {
         if(fp)
         fclose(fp);

         CloseControlConnection();

         LogError("data connect error \n");
         return false;
    }
   /*
    * send command to server & read reply
    */

   sprintf(szBuffer,"RETR %s\r\n",chRemoteFilePath);
   if(!SendControlMsg(szBuffer,strlen(szBuffer)))
   {
      if(fp)
      fclose(fp);

     // CloseListenSocket();
     CloseDataConnection();
     CloseControlConnection();
      return false;
   }
   int nRetCode = GetReply();
   if((nRetCode != 150) && (nRetCode != 125))
   {
      if(fp)
      fclose(fp);

      LogError("RETR error nRetCode=%d\n",nRetCode);
	  LogError("FTP目录下没有%s文件\n",chRemoteFilePath);
      CloseDataConnection();
      CloseControlConnection();

      return false;
   }

   /*
    * now send file
    */

  // printf("Type q and hit return to abort\r\n");
   bool bRet = true;

   if(fp)
   {
       int nBytes = 0;
	   while( (nBytesRead=ReadDataMsg(szBuffer,1024)) > 0)
       {
           //printf("===nBytesRead=%d\r\n",nBytesRead);
		   nBytes = 0;
		   nBytes = fwrite(szBuffer,1,nBytesRead,fp);
		  //printf("===nBytes=%d\r\n",nBytes);

          if(nBytes <= 0)
          {
              bRet = false;
              break;
           }

          nTotal+=nBytes;
		  memset(szBuffer, 0, sizeof(szBuffer));
          //printf("%s : %d recv\r\n",chLocalFilePath,nTotal);
       }
       printf("===nBytesRead=%d\r\n",nBytesRead);
   }

   /*(void)signal(SIGINT,OldHandler); */

   /*
    * close data connection
    */
   CloseDataConnection();
   int iRetCode = GetReply();

   if(iRetCode != 226)//226 transfer complete
   {
       bRet = false;
   }


   if(!bRet)
   {
      LogNormal("===send error\n");
      CloseControlConnection();
   }

   if(fp)
   fclose(fp);

   return bRet;
}


