#include "CFmfsTel.h"
#include "Common.h"

void * CFmfsTel::ThreadAccept(void *param)
{
	CFmfsTel *FmfsTel = (CFmfsTel *)param;
	if (FmfsTel)
	{
		LogTrace("fmfs.log", "CFmfsTel::open accept connect thread!");
       FmfsTel->AcceptConnects();
	}

	pthread_exit((void *)0);
}

void * CFmfsTel::ThreadFrame(void *param)
{
	CFmfsTel *FmfsTel = (CFmfsTel *)param;
	if (FmfsTel)
	{
		LogTrace("fmfs.log", "CFmfsTel::open accept video data thread!");
		FmfsTel->DealSteamData();
	}
	pthread_exit((void *)0);
}

CFmfsTel::CFmfsTel()
{
  m_strHost = "";
  m_nCtrlPort = 8999;
  m_nCtrlSocket = -1;

  m_nDataSocket = -1;
  m_nListenSocket = -1;
  m_nRecvId = 0;
  m_nListenId = 0;
  m_nDataPort = 20110;
  m_bEndRecv = false;

  m_VideoInfo.width=0;
  m_VideoInfo.height=0;
  m_VideoInfo.ts = 0;

  m_uWidth = m_uHeight = 0;
  m_lEndTime = -1;//结束时间

  pthread_mutex_init(&m_mutex, NULL);
  pthread_mutex_init(&m_Frame_Mutex, NULL);
}

CFmfsTel::~CFmfsTel()
{
  if(m_nCtrlSocket > 0)
     close(m_nCtrlSocket);

  pthread_mutex_destroy(&m_mutex);
  pthread_mutex_destroy(&m_Frame_Mutex);
}

/*
 * 与FMFS服务器建立连接
*/
bool CFmfsTel::ConnectWithFmfs(const string& strHost, int nPort/*=8999*/, int nOpt/* =SO_REUSEADDR */)
{
	m_strHost = strHost;
   m_nCtrlPort = nPort;

	if(!mvCreateSocket(m_nCtrlSocket, 1))
		return false;
	if(!mvSetSocketOpt(m_nCtrlSocket, nOpt) | !mvSetSocketOpt(m_nCtrlSocket, SO_SNDBUF))//设置套接字选项
	{
		LogTrace("fmfs.log", "reuse addr error!");
		return false;
	}
	//if(this->WaitConnect(m_nCtrlSocket, strHost, nPort, 3))
	if(mvWaitConnect(m_nCtrlSocket, strHost, nPort, 3))
	{
		LogTrace("fmfs.log", "strHost=%s, nPort=%d has connected!", strHost.c_str(), nPort);
		return true;
	}
	return false;
}
/*
 *szBuf 发送的消息
 *nLength 消息长度
*/
bool CFmfsTel::Send(int nSocket, char *szBuf, int nLength)
{
	try
	{
		int nLeft, nBytes, nDataLen;
		nLeft = nLength;
		nBytes = 0;	
		nDataLen = 0;
		while(nLeft > 0)
		{
			if(!mvWaitToWriteSocket(nSocket))
			{
				LogTrace("fmfs.log", "socket=%d can't write...", nSocket);
				return false;
			}
			if((nBytes = send(nSocket, szBuf + nDataLen, nLeft, 0)) < 0 )
				return false;
			nLeft -= nBytes;
			nDataLen += nBytes;
		}
		return (nLeft == 0);
	}
	catch(...){}
	return false;
}
/*
 * 向FMFS的控制端口发送数据
 * 成功 true
*/
bool CFmfsTel::SendToFmfs(const onDemandCommand &command, onDemandGetScope &param)
{
   bool bRet = false;
   int size = sizeof(onDemandCommand) + 1;
   char *buff = new char[size];
   string strMsg("");
   memset(buff, 0, size);
   memcpy(buff, &command, sizeof(onDemandCommand));
   if (Send(m_nCtrlSocket, buff, size))
   {
	   LogTrace("fmfs.log", "send cmd %d success!!!", (int)command.onDemandModeCode);
       switch(command.onDemandModeCode)
	   {
	   case ServerInit: 
		   {
			   usleep(2000);
             if (Recv(m_nCtrlSocket,sizeof(bool) ,strMsg))
             {
                 int value = *((int *)strMsg.c_str());
				   bRet = (value == 0)?false:true;
				   LogTrace("fmfs.log", "recv initial server value:(%d)-%s", value, strMsg.c_str());
             }
		   }break;
	   case GetVolumeScope:
		   {
             usleep(2000);
			  if (Recv(m_nCtrlSocket, sizeof(onDemandGetScope),strMsg))
			  {
               param = *((onDemandGetScope *)strMsg.c_str());
			    bRet = (param.isSuccess == 0)?false:true;
			  }
		   }break;
	   case StartOnlyForward: case SectionVideoForward:
		   {
			   usleep(2000);
			   m_lEndTime = MakeTime(command.endTime);//设置视频结束时间 
			   for(int i = 0; i < 30; i++)
			   {
				   if (Recv(m_nCtrlSocket, sizeof(onDemandStatus),strMsg))
				   {
					   onDemandStatus *status = (onDemandStatus *)strMsg.c_str();
					   LogTrace("fmfs.log", "recv video size:%d, fetchStatus:%d", strMsg.size(), status->fetchStatus);
					   if (status->fetchStatus == 1)
					   {
						   yuv_video_buf buf;
						   buf.width = m_uWidth = status->videoWidth;
						   buf.height = m_uHeight = status->videoHeight;
						   buf.nVideoType = 1;
						   buf.nFrameRate = 5;
						   SetVideoInfo(&buf);
						   LogTrace("fmfs.log", "connect with FMFS data stream, begin to recv data! image width=%d, height=%d", m_uWidth, m_uHeight);
						   StartThreads();//开启数据接收线程
						   bRet = true; 
						   break;
					   }
				   }
				   sleep(1);
			   }
				  
		   }break;
	   default:
		   bRet = true;
		   break;
	   }
   }
   else
   {
	   bRet = false;
   }
   delete[] buff;
   return bRet;
}

bool CFmfsTel::Recv(int nSocket, int nLen,string& strMsg)
{
  try
  {
 
  	   strMsg.clear();

	  pthread_mutex_lock(&m_mutex);
	  
	  int nBytes = 0;
	  int nLeft = nLen; 
	  
	  char szBuf[1024]={0};
	  while (nLeft > 0)
	  {
 
	  	  nBytes=recv(nSocket, szBuf, 1024>nLeft?nLeft:1024, MSG_NOSIGNAL);
                                                  
		  if(nBytes <= 0)
		  {
      		 break;
		  }		  
		  strMsg.append(szBuf, nBytes);
		  memset(szBuf, 0, 1024);
		  nLeft  -= nBytes; 
	  }
	  pthread_mutex_unlock(&m_mutex);
	  return nLeft == 0;
  }
  catch (...)
  { }
  pthread_mutex_unlock(&m_mutex);
  return false;
}
/*
 *请求视频服务后调用，初始化接收帧数据服务器
*/
bool CFmfsTel::OpenDataServer(int nPort)
{
	m_bEndRecv = false;
	m_nDataPort = nPort;

	//防止切换相机无法绑定端口
   if (m_nListenSocket > 0)
   {
	   shutdown(m_nListenSocket, 2);
	   close(m_nListenSocket);  
   }
   if (!mvCreateSocket(m_nListenSocket, 1))
   {
	   LogTrace("fmfs.log", "Fail to create data socket");
	   return false;
   }
   if(!mvSetSocketOpt(m_nListenSocket, SO_REUSEADDR))
   {
       LogTrace("fmfs.log", "set socket = %d parameter error!", m_nListenSocket);
	   return false;
   }
   if (!mvBindPort(m_nListenSocket, nPort))
   {
	   LogTrace("fmfs.log", "Fail to bind port %d", nPort);
	   return false;
   }
   if (!mvStartListen(m_nListenSocket))
   {
	   return false;
   }

   return true;
}
//接收数据连接
void CFmfsTel::AcceptConnects()
{
	while(!m_bEndRecv)
	{
      if (g_bEndThread)
      {
		  break;
      }
      struct sockaddr_in addr;
	   socklen_t length = sizeof(struct sockaddr_in);
	   int nSocket = -1;
	   if ((nSocket = accept(m_nListenSocket, (sockaddr *)&addr, &length)) > 0)
	   {
		   if (m_nDataSocket > 0)
		   {
			   close(m_nDataSocket);
		   }
		   m_nDataSocket = nSocket;
	   }
	  usleep(10*1000);
	}
}

void CFmfsTel::SetVideoInfo(yuv_video_buf* pVideoInfo)
{
	//if(m_pFrameBuffer == NULL)//重新获取视频流时不需要进行设置
	{
		m_VideoInfo = *pVideoInfo;

#ifdef H264_DECODE
		m_Decoder.SetVideoSize(pVideoInfo->width,pVideoInfo->height);
		m_Decoder.SetVideoCodeID(pVideoInfo->nVideoType);
		m_Decoder.InitDecode(NULL);
#endif
		LogTrace("fmfs.log", "nWidth=%d,nHeight=%d\n",pVideoInfo->width,pVideoInfo->height);
	}
}

yuv_video_buf * CFmfsTel::GetVideoInfo()
{
	return &m_VideoInfo;
}

void CFmfsTel::DealSteamData()
{
	while(!m_bEndRecv)
	{
		if (g_bEndThread)
		{
			break;
		}
       try
	   {
		   string strMsg;
		   if(Recv(m_nDataSocket,sizeof(Frame_t), strMsg))//接收帧数据头
		   {
			   Frame_t *pFrame = (Frame_t *)strMsg.c_str();
			   if (pFrame)
			   {
				   if(pFrame->flag == 1 && pFrame->length > 0)
				   {
					   LogTrace(NULL, "receive video data size:%d, Time: %s, Frame:%d", pFrame->length, GetTimeCurrent().c_str(), int(pFrame->timeStamp.frame));
					   string strFrame; 
					   if (Recv(m_nDataSocket, pFrame->length, strFrame))
					   {
						   AddFrame((unsigned char *)strFrame.c_str(), strFrame.size(), pFrame->timeStamp);
					   }
					   else//接收错误关闭流
					   {
						   close(m_nDataSocket);
						   m_nDataSocket = -1;
					   }

					   //判断是否分析结束
					   if (MakeTime(pFrame->timeStamp) >= m_lEndTime)
					   {
						   LogTrace("fmfs.log", "The history video has been analyzed! Time:%s", GetTimeCurrent().c_str());
						   close(m_nCtrlSocket);
						   m_nCtrlSocket = -1;

						   shutdown(m_nListenSocket, 2);
						   close(m_nListenSocket);
						   m_nListenSocket = -1;

						   close(m_nDataSocket);
						   m_nDataSocket = -1;
						   m_bEndRecv = true;
					   }
				   }
			   }
		   }
		   else
		   {
             close(m_nDataSocket);
			  m_nDataSocket = -1;
		   }
		  // LogTrace("fmfs.log", "%s,DealStreamData: video data size:%d,sizeof(Frame_t)=%d",GetTimeCurrent().c_str(), strMsg.size(),sizeof(Frame_t));
		   
	   }catch(...){}
		usleep(1000);
	}
}
/*
 * 关闭连接fmfs传送的视频流
*/
bool CFmfsTel::CloseVideo()
{
   //关闭与FMFS之间的连接
   CloseFmfsConnect();

	shutdown(m_nListenSocket, 2);
	close(m_nListenSocket);
	m_nListenSocket = -1;

	close(m_nDataSocket);
    m_bEndRecv = true;

//   this->mvJoinThread(m_nRecvId);
//   this->mvJoinThread(m_nListenId);
   LogTrace(NULL, "fmfs video has closed.");

#ifdef H264_DECODE
   m_Decoder.UinitDecode();
#endif
   usleep(1000*1000*5);
   return true;
}

/* 
*/
bool CFmfsTel::ReopenVideo()
{
 
	 return true;
	/*shutdown(m_nListenSocket, 2);
	close(m_nListenSocket);
	shutdown(m_nDataSocket, 2);
	close(m_nDataSocket);

	LogTrace("fmfs.log", "Reopen video, Time:%s", GetTime(time(NULL)).c_str());
	
	return OpenDataServer(m_nDataPort);*/
}

void CFmfsTel::AddFrame(unsigned char* pData,int dataLen, TimeStamp_t timestamp)
{
	//if (m_pFrameBuffer)
	{
		yuv_video_buf buf = m_VideoInfo;
		buf.ts = MakeTime(timestamp) * 1000000 + 200 * (timestamp.frame + 1)*1000;

		LogTrace(NULL, "AddFrame == nSize=%d, buf.ts=%ld\n",dataLen, buf.ts);
		string strFrame;
		strFrame.append((char *)&buf, sizeof(yuv_video_buf));
       strFrame.append((char *)pData, dataLen);
		//加锁
		pthread_mutex_lock(&m_Frame_Mutex);
         
		m_listFrame.push_back(strFrame);
		//解锁
		pthread_mutex_unlock(&m_Frame_Mutex);

		usleep(1000);
	
	}
}

int CFmfsTel::PopFrame(yuv_video_buf& header,unsigned char* response)
{
	int nSize = 0;
	string strFrame;
	//加锁
	pthread_mutex_lock(&m_Frame_Mutex);
	if(m_listFrame.size() >= 1) //只有一场
	{
		list<string>::iterator it = m_listFrame.begin();
       strFrame = *it;
		m_listFrame.pop_front();
	}
	//解锁
	pthread_mutex_unlock(&m_Frame_Mutex);

	if(strFrame.size() > 0)
	{
		header = *((yuv_video_buf*)(strFrame.c_str()));

		bool bRet = false;
#ifdef H264_DECODE
		bRet = m_Decoder.DecodeFrame((unsigned char*)(strFrame.c_str()+sizeof(yuv_video_buf)),strFrame.size()-sizeof(yuv_video_buf),response,nSize);
#endif
		/*char ch[128]={0};
		sprintf(ch, "pic%ld.jpg", time(NULL));
	
		IplImage *plImage = cvCreateImageHeader(cvSize(m_VideoInfo.width, m_VideoInfo.height), 8, 3);
		cvSetData(plImage, response, plImage->widthStep);
       cvSaveImage(ch, plImage);
	    cvReleaseImageHeader(&plImage);*/
		if(bRet&&nSize>0)
		{
			nSize = 1;
		}
	}
	return nSize;
}
//转换时间格式 得到秒数
long CFmfsTel::MakeTime(TimeStamp_t time)
{
	struct tm t;
	t.tm_year = time.year - 1900;
	t.tm_mon = time.month - 1;
	t.tm_mday = time.day;
	t.tm_hour = time.hour;
	t.tm_min = time.minute;
	t.tm_sec = time.second;

	return mktime(&t);
}

bool CFmfsTel::StartThreads()
{
	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	int nret=pthread_create(&m_nListenId,&attr,ThreadAccept,(void*)this);
	if(nret!=0)
	{
		return false;
	}

	nret=pthread_create(&m_nRecvId,&attr,ThreadFrame, (void *)this);
	if(nret!=0)
	{
		return false;
	}
  return true;
}

bool CFmfsTel::WaitConnect(int nSocket, string strHost, int nPort, int nSec)
{
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(strHost.c_str());
	servaddr.sin_port = htons(nPort);

	this->mvSetSocketBlock(nSocket, false);//设置非阻塞
	do
	{
		if ((connect(nSocket, (struct sockaddr*)&servaddr, sizeof(servaddr))) == 0) 
		{
			this->mvSetSocketBlock(nSocket, true);//设置阻塞
			return true;
		}
		nSec--;
		sleep(1);

	}while(nSec > 0);
	return false;
}

void CFmfsTel::CloseFmfsConnect()
{
	try
	{
		onDemandCommand command;
		onDemandGetScope scope;
		memset(&command, 0, sizeof(onDemandCommand));
		command.onDemandModeCode = EndofStartOnly;
		SendToFmfs(command, scope);

		usleep(10*1000);

		close(m_nCtrlSocket);
		m_nCtrlSocket = -1;
	}
	catch (...)
	{	}
}