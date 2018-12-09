#include "VmuxSocket.h"

#ifdef VMUX_PROC

#include "RoadChannelCenter.h"

CVmuxSocket* CVmuxSocket::_vmux = new CVmuxSocket;

CVmuxSocket::CVmuxSocket()
{
   m_nAcceptSocket = -1;
   m_nPort = 4501;//接收控制信息的端口

   m_nDataSocket = -1;
   m_nDataPort = 8888;

   m_bEndStream = false;

   pthread_mutex_init(&m_thread_mutex,NULL);
   pthread_mutex_init(&m_DataMutex,NULL);
    pthread_mutex_init(&m_Mutex,NULL);
}

CVmuxSocket::~CVmuxSocket()
{
   pthread_mutex_destroy(&m_thread_mutex);
   pthread_mutex_destroy(&m_DataMutex);
   pthread_mutex_destroy(&m_Mutex);
}

void * CVmuxSocket::ThreadAccept(void * param)
{
   int nRet;
   CVmuxSocket *pSocket = (CVmuxSocket *)param;
   if (pSocket)
   {
	   pSocket->OnAccept();
   }

   pthread_exit(&nRet);
}

void * CVmuxSocket::ThreadDataAccept(void * param)
{
	int nRet;
	CVmuxSocket *pSocket = (CVmuxSocket *)param;
	if (pSocket)
	{
		pSocket->OnDataAccept();
	}

	pthread_exit(&nRet);
}

void * CVmuxSocket::ThreadRecv(void * param)
{
   int nRet;
   int nSocket = *((int *)param);
   if (nSocket > 0)
   {
	   CVmuxSocket *pSocket = GetInstance();
	   if (pSocket)
	   {
		   pSocket->OnReceive(nSocket);
	   }
   }
   pthread_exit(&nRet);
}

void * CVmuxSocket::ThreadDeal(void *param)
{
	int nRet;
	CVmuxSocket *pSocket = (CVmuxSocket *)param;
	if (pSocket)
	{
		pSocket->OnDeal();
	}
	pthread_exit(&nRet);
}

bool CVmuxSocket::Init()
{
   
   m_bEndThread = false;

   if (!CreateSocket(m_nAcceptSocket, m_nPort))
   {
	   LogTrace("vmux.log", "create control socket error.");
	   return false;
   }
	//启动接收连接线程
	pthread_t id;           //线程id
	pthread_attr_t   attr;  //线程属性


	pthread_attr_init(&attr);//初始化
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//分离线程

	//启动线程
	if(pthread_create(&id,&attr, ThreadAccept, this)!=0)
	{
		pthread_attr_destroy(&attr);
		LogError("创建VMUX接收线程失败,服务无法启动!\r\n");
		return false;
	}

	//启动线程
	if(pthread_create(&id,&attr, ThreadDeal, this)!=0)
	{
		pthread_attr_destroy(&attr);
		LogError("创建VMUX处理数据线程失败,服务无法启动!\r\n");
		return false;
	}
	

	m_bEndStream = false;

	//建立数据连接
	if(!CreateSocket(m_nDataSocket, m_nDataPort))
	{
		LogTrace("vmux.log", "create data socket error!");
		return false;
	}
	//启动线程
	if(pthread_create(&id,&attr, ThreadDataAccept, this)!=0)
	{
		pthread_attr_destroy(&attr);
		LogError("创建VMUX接收数据线程失败,服务无法启动!\r\n");
		return false;
	}
    pthread_attr_destroy(&attr);

	return true;
}

void CVmuxSocket::OnAccept()
{
	struct sockaddr_in clientaddr;
	socklen_t sin_size = sizeof(struct sockaddr_in);
	int nClient;

	while(!g_bEndThread)
	{
		if (m_bEndThread)
		{
			break;
		}

		//接受连接
		if((nClient = accept(m_nAcceptSocket,(struct sockaddr*)&clientaddr,&sin_size)) ==  -1)
		{
			//断开连接
			if(g_bEndThread || m_bEndThread)
			{
				return;
			}
			continue;
		}
	//	LogTrace("vmux.log", "Server %s has connected! socket=%d,port:%d", inet_ntoa(clientaddr.sin_addr), nClient, ntohs(clientaddr.sin_port));
		
		ClientInfo info;
		memcpy(info.chHost, inet_ntoa(clientaddr.sin_addr), 16);
		info.nPort = ntohs(clientaddr.sin_port);
       info.status = 1;//已开启

		pthread_mutex_lock(&m_thread_mutex);
		m_vecSocket.push_back(make_pair(nClient, info));
		pthread_mutex_unlock(&m_thread_mutex);


		pthread_t id;           //线程id
		pthread_attr_t   attr;  //线程属性

		pthread_attr_init(&attr);//初始化
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);//分离线程

		//启动线程
		if(pthread_create(&id,&attr, ThreadRecv, &nClient)!=0)
		{
			pthread_attr_destroy(&attr);
			LogTrace("vmux.log", "create thread ThreadRecv error! socket=%d", nClient);
			return;
		}
		pthread_attr_destroy(&attr);

		usleep(1000*10);
	}
}

void CVmuxSocket::SendMsg(string& msg)
{
	pthread_mutex_lock(&m_Mutex);
	vector<int>::iterator it_b = m_vecDataSocket.begin();
	while (it_b != m_vecDataSocket.end())
	{
		int nClient = *it_b;
		
		if (!Send(nClient, msg.c_str(), msg.size()))
		{
			it_b = m_vecDataSocket.erase(it_b);
			continue;
		}
		//LogTrace(NULL, "send msg size:%d success.", msg.size());
		
		it_b++;
	}
	pthread_mutex_unlock(&m_Mutex);
}

bool CVmuxSocket::Send(int nSocket, const char *szBuf, int nLength)
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
				close(nSocket);
				return false;
			}
			if((nBytes = send(nSocket, szBuf + nDataLen, nLeft, MSG_NOSIGNAL)) <= 0 )
			{
				close(nSocket);
				return false;
			}
			nLeft -= nBytes;
			nDataLen += nBytes;
		}
		return (nLeft == 0);
	}
	catch(...){}
	return false;
}

/*
 * 接收数据并放入消息映射中 
*/
void CVmuxSocket::OnReceive(int nSocket)
{
   while (!g_bEndThread)
   {
	   if (m_bEndThread)
	   {
		   break;
	   }

	   CtlHead header;
	   int nBytes = 0;
	   std::string response;
	   //接收HTTP 头，一次性接收
	   nBytes = recv(nSocket,(void*)&header,sizeof(header),MSG_NOSIGNAL);
	   if(nBytes <= 0)
	   {
		   LogTrace("vmux.log", "接收协议头出错，连接断开! socket = %d,%s\r\n", nSocket, strerror(errno));
		   DelOneClient(nSocket);
		   break;
	   }
	   
      int nLeft = header.m_lenH;
	  char buffer[1024]={0};

	  if (header.m_Heads[0] != 0xBF || header.m_Heads[1] != 0xEC)
	  {
		  continue;
	  }

	  unsigned short uCmd = header.m_catalog & 0xf; 
	  uCmd = (uCmd << 8) | header.m_cmd;

	  nLeft = (nLeft << 8) | header.m_lenV;
	  response.append((char *)&header, sizeof(CtlHead));
	 // LogTrace("vmux.log", "OnReceive: recv cmd %x, len:%d, CheckSume:%d, check:%d, ack=%d", uCmd, nLeft, header.m_CheckSum, CheckSum(&header), header.m_catalog >> 5);

	  while (nLeft > 0)
	  {
		  nBytes = recv(nSocket, buffer, 1024<nLeft?nLeft:1024, MSG_NOSIGNAL);
		  if (nBytes <= 0)
		  {
			  LogTrace("vmux.log", "接收后续消息出错，连接断开! socket = %d,%s\r\n", nSocket, strerror(errno));
			  DelOneClient(nSocket);
			  return;
		  }

		  response.append(buffer, nBytes);
		  nLeft -= nBytes;
	  }
	 //把消息加入消息队列 
	  pthread_mutex_lock(&m_DataMutex);
      m_msgMap.insert(make_pair(nSocket, response));
	  pthread_mutex_unlock(&m_DataMutex);
	  
	   usleep(2000);
   }
}

bool CVmuxSocket::DelOneClient(int nSocket)
{
	pthread_mutex_lock(&m_thread_mutex);
	vector< pair<int, ClientInfo> >::iterator it_b = m_vecSocket.begin();
	while (it_b != m_vecSocket.end())
	{
		int nClient = it_b->first;
		if (nSocket == nClient)
		{
			it_b = m_vecSocket.erase(it_b);
			continue;
		}
		it_b++;
	}
	pthread_mutex_unlock(&m_thread_mutex);
	return true;
}

void CVmuxSocket::OnDeal()
{
	while(!g_bEndThread)
	{
		if (m_bEndThread)
		{
			break;
		}

		string msg;
		int nSocket;
		pthread_mutex_lock(&m_DataMutex);
		multimap<int, string>::iterator it = m_msgMap.begin();
		if (it != m_msgMap.end())
		{
			msg = it->second;
			nSocket = it->first;

           m_msgMap.erase(it);
		}
		pthread_mutex_unlock(&m_DataMutex);

		if (msg.size() > 0)
		{
          OnMessage(nSocket, msg);//处理消息
		}

		usleep(1000);
	}
}

//处理消息
bool CVmuxSocket::OnMessage(int nSocket, string& msg)
{
   CtlHead* pHead = (CtlHead *)msg.c_str();
   unsigned short uCmd = pHead->m_catalog & 0xf; 
   uCmd = (uCmd << 8) | pHead->m_cmd;
  

   CtlHead header = *pHead;
   string response;
   unsigned short len = 0;
   header.m_catalog = header.m_catalog | 0x10;
   //LogTrace("vmux.log", "recv cmd=%x from socket %d", uCmd, nSocket);
   switch(uCmd)
   {
   case 0x0100://获取IP
	   {
         struct tagStruct
		 {
           int flag;
		    int type;
			unsigned int ip;
           unsigned int mask;
		    unsigned int gateway;
			unsigned int dns1;
			unsigned int dns2;
		 }temp;
        
		 len = sizeof(tagStruct);
		 header.m_lenH = len >> 8;
		 header.m_lenV = len & 0xff;
		 header.m_CheckSum = (unsigned char)CheckSum(&header);
		 response.append((char *)&header, sizeof(CtlHead));

        memset(&temp, 0, sizeof(tagStruct));
		 temp.ip = inet_addr(g_ServerHost.c_str());
		 temp.gateway = inet_addr(g_strGateWay.c_str());
		 temp.mask = inet_addr(g_strNetMask.c_str());
		 response.append((char *)&temp, sizeof(tagStruct));
	   }break;
   case 0x104://获取时间
	   {
		   struct TimeStruct
		   {
			   int flag;
			   unsigned char year;
			   unsigned char month;
			   unsigned char day;
			   unsigned char wDay;//星期
			   unsigned char hour;
			   unsigned char min;
			   unsigned char sec;
			   unsigned char reserve;
		   }tmp;

		   len = sizeof(TimeStruct);
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);
		   response.append((char *)&header, sizeof(CtlHead));

		   struct tm *local, newTime;
		   long lTimestamp = time(NULL);
		   local = &newTime;
		   localtime_r((time_t *)&lTimestamp, local);
		   memset(&tmp, 0, sizeof(tmp));
          tmp.year = local->tm_year + 1900;
		   tmp.month = local->tm_mon + 1;
		   tmp.day = local->tm_mday;
		   tmp.wDay = local->tm_wday + 1;
		   tmp.hour = local->tm_hour;
		   tmp.min = local->tm_min;
		   tmp.sec = local->tm_sec;
		   response.append((char *)&tmp, sizeof(TimeStruct));
	   }break;
   case 0x105://设置时间
	   {
		   struct TimeStruct
		   {
			   unsigned short slot;
			   unsigned short channelId;
			   unsigned char year;
			   unsigned char month;
			   unsigned char day;
			   unsigned char wDay;//星期
			   unsigned char hour;
			   unsigned char min;
			   unsigned char sec;
			   unsigned char reserve;
		   }VisTime, *pVisTime;

	    AckRet sRet;
        pVisTime =(TimeStruct *)(msg.c_str() + sizeof(CtlHead));
        if (pVisTime)
        {
           sRet.slot = pVisTime->slot;
		    sRet.channelId = pVisTime->channelId;
			sRet.ack = 0;
        }
		 header.m_catalog = header.m_catalog | 0x30;
		 len = sizeof(AckRet);
		 header.m_lenH = len >> 8;
		 header.m_lenV = len & 0xff;
		 header.m_CheckSum = (unsigned char)CheckSum(&header);

		 response.append((char *)&header, sizeof(CtlHead));
		 response.append((char *)&sRet, sizeof(AckRet));
	   }break;
   case 0x0200://获取视频参数
	   {
		   char buf[12]={0};
		   len = 12;
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);
		   response.append((char *)&header, sizeof(CtlHead));
		   response.append(buf, 12);
	   }break;
   case 0x0201:  case 0x203:  case 0x205: case 0x207:  case 0x405://视频参数设置
	   {
		   AckRet sRet;
		   BodySync *pBody = (BodySync *)(msg.c_str() + sizeof(CtlHead));
		   if (pBody)
		   {
			   sRet.slot = pBody->slot;
			   sRet.channelId = pBody->channelId;
			   sRet.ack = 0;
		   }
		   header.m_catalog = header.m_catalog | 0x30;
		   len = sizeof(AckRet);
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);

		   response.append((char *)&header, sizeof(CtlHead));
		   response.append((char *)&sRet, sizeof(AckRet));
	   }break;
   case 0x202://编码器参数
	   {
		   char buf[52]={0};
		   len = 52;
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);
		   response.append((char *)&header, sizeof(CtlHead));
		   response.append(buf, 52);
	   }break;
   case 0x204://解码器参数
	   {
		   char buf[20]={0};
		   len = 20;
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);
		   response.append((char *)&header, sizeof(CtlHead));
		   response.append(buf, 20);
	   }break;
   case 0x206://OSD参数
	   {
		   char buf[92]={0};
		   len = 92;
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);
		   response.append((char *)&header, sizeof(CtlHead));
		   response.append(buf, 92);
	   }break;
   case 0x400://获取码流发送信息
	   {
         struct FrameInfo
		 {
			 short slot;
			 char channel;
			 char streamId;
			 int headFlag;
			 int protocal;
			 unsigned int ip;
			 int port;
		 }info;
		 memcpy(&info, msg.c_str()+sizeof(CtlHead), 4);
		 len = sizeof(FrameInfo);
		 header.m_lenH = len >> 8;
		 header.m_lenV = len & 0xff;
		 header.m_CheckSum = (unsigned char)CheckSum(&header);

		 memset(&info, 0, sizeof(FrameInfo));
		 info.headFlag = 0;
		 info.protocal = 1;
        info.ip = inet_addr(g_ServerHost.c_str());
		 info.port = htonl(8888);
		 response.append((char *)&header, sizeof(CtlHead));
		 response.append((char *)&info, sizeof(FrameInfo)); 
	   }break;
   case 0x401://启动码流发送
	   {
		   struct FrameInfo
		   {
			   short slot;
			   char channel;
			   char streamId;
			   int headFlag;
			   int protocal;
			   unsigned int ip;
			   int port;
		   }info, *pInfo;

		  AckRet sRet; 
		  pInfo = (FrameInfo *)(msg.c_str() + sizeof(CtlHead));
		  if (pInfo)
		  {
			  sRet.slot = pInfo->slot;
			  sRet.channelId = pInfo->channel;
             sRet.ack = 0;
		  }
		  header.m_catalog = header.m_catalog | 0x30;
		  len = sizeof(AckRet);
		  header.m_lenH = len >> 8;
		  header.m_lenV = len & 0xff;
		  header.m_CheckSum = (unsigned char)CheckSum(&header);

		  response.append((char *)&header, sizeof(CtlHead));
		  response.append((char *)&sRet, sizeof(AckRet));
		  if (m_nDataSocket <= 0)//码流没有开启
		  {
			  LogNormal("启动码流发送");
			   pthread_t id;
			   m_bEndStream = false;
			   //建立数据连接
			   if(!CreateSocket(m_nDataSocket, m_nDataPort))
			   {
				   LogTrace("vmux.log", "create data socket error!");
				   return false;
			   }
			   //启动线程
			   if(pthread_create(&id, NULL, ThreadDataAccept, this)!=0)
			   {
				   LogError("创建VMUX接收数据线程失败,服务无法启动!\r\n");
				   return false;
			   }
		  }
	   }break;
   case 0x403://关闭码流
	   {
		   struct FrameInfo
		   {
			   short slot;
			   char channel;
			   char streamId;
			   int headFlag;
			   int protocal;
			   unsigned int ip;
			   int port;
		   }info, *pInfo;

		   AckRet sRet; 
		   pInfo = (FrameInfo *)(msg.c_str() + sizeof(CtlHead));
		   if (pInfo)
		   {
			   sRet.slot = pInfo->slot;
			   sRet.channelId = pInfo->channel;
			   sRet.ack = 0;
		   }
		   header.m_catalog = header.m_catalog | 0x30;//设置ACK返回值为1
		   len = sizeof(AckRet);
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);

		   response.append((char *)&header, sizeof(CtlHead));
		   response.append((char *)&sRet, sizeof(AckRet));

		   if (m_nDataSocket > 0)//码流开启
		   {
			    LogNormal("关闭码流");
			   m_bEndStream = true;
			   shutdown(m_nDataSocket, 2);
              close(m_nDataSocket);
			   m_nDataSocket = -1;
			   LogTrace("vmux.log", "end sending stream!");
		   }
	   }break;
   case 0x404://码流结构
	   {
		   char buf[8]={0};
		   len = 8;
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);
		   response.append((char *)&header, sizeof(CtlHead));
		   response.append(buf, 8);
	   }break;
   case 0x501://云台控制命令
	   {
          struct PTZ
		  {
            unsigned short slot;
			 unsigned short com;//串口编号
			 unsigned char yunTai;
			 unsigned char preSet;
			 unsigned char speed;
			 unsigned char reserve;
		  }ptz, *pPtz;

		  AckRet sRet;
         pPtz = (PTZ *)(msg.c_str() + sizeof(CtlHead));
		  if (pPtz)
		  {
			  CAMERA_CONFIG cfg;
			  LogTrace("vmux.log", "ptz: yunTai:%d, preSet:%d,speed=%d", (int)pPtz->yunTai, (int)pPtz->preSet, (int)pPtz->speed);
             cfg.uType = 0;//表示是vis发过来的命令
			  if (pPtz->yunTai == 11)//zoom far
			  {
				  cfg.nIndex = ZOOM_FAR;
				  cfg.fValue = pPtz->speed;
			  }
			  else if (9 == pPtz->yunTai)
			  {
                cfg.nIndex = ZOOM_NEAR;
				  cfg.fValue = pPtz->speed;
			  }
			  else if (23 == pPtz->yunTai)
			  {
				  cfg.nIndex = SET_PRESET;
				  cfg.fValue = pPtz->preSet;
			  }
			  else if (24 == pPtz->yunTai)
			  {
				  cfg.nIndex = GOTO_PRESET;
				  cfg.fValue = pPtz->preSet;
			  }
			  else if (10 == pPtz->yunTai || 12 == pPtz->yunTai)//停止镜头的接近和拉远
			  {
                cfg.nIndex = ZOOM_FAR;
			     cfg.nOperationType = 2;
			  }
			  else if (32 == pPtz->yunTai)//转换镜头焦距
			  {
				  cfg.nIndex = SWITCH_CAMERA;
				  cfg.fValue = pPtz->speed;
				  LogTrace("vmux.log", "cfg.nIndex=%d, cfg.fValue=%f", cfg.nIndex, cfg.fValue);
			  }
			  else
			  {
				  cfg.nIndex = 0;
			  }
             

			 g_skpChannelCenter.CameraControl(cfg);//对相机进行控制

			 sRet.slot = pPtz->slot;
			 sRet.ack = 0;
		  }

		  header.m_catalog = header.m_catalog | 0x30;//设置ACK返回值为1
		  len = sizeof(AckRet);
		  header.m_lenH = len >> 8;
		  header.m_lenV = len & 0xff;
		  header.m_CheckSum = (unsigned char)CheckSum(&header);

		  response.append((char *)&header, sizeof(CtlHead));
		  response.append((char *)&sRet, sizeof(AckRet));
	   }break;
   case 0x510://op框选位置
	   {
		   struct DetectRect
		   {
			   short int nSlot;
			   short int nChannel;//串口编号
			   unsigned char ptzAction;
			   short int iFrmWidth; // （ 选框宽度 / 分辨率宽度 ）* 1000
			   short int iFrmHeight; // （ 选框高度 / 分辨率高度 ）* 1000
			   short int iCenterHOffset; // （ 选框中心点与画面中心点水平偏离值 / 分辨率宽度 ）* 1000
			   short int iCenterVOffset; // （ 选框中心点与画面中心点垂直偏离值 / 分辨率高度 ）* 1000
		   }detectrect, *pDetectRect;

		   printf("111111111111111111111\n");
		   AckRet sRet;
		   pDetectRect = (DetectRect *)(msg.c_str() + sizeof(CtlHead));
		   if (pDetectRect)
		   {
			   ImageRegion szImageRegion;
			   szImageRegion.x = pDetectRect->iCenterHOffset;
			   szImageRegion.y = pDetectRect->iCenterVOffset;
			   szImageRegion.width = pDetectRect->iFrmWidth;
			   szImageRegion.height = pDetectRect->iFrmHeight;
			   szImageRegion.nImageRegionType = TRAFFIC_SIGNAL_REGION_IMAGE; //给VIS使用的字段
			   g_skpChannelCenter.DetectRegionRectImage(1,szImageRegion);//对相机进行控制
			   sRet.slot = pDetectRect->nSlot;
			   sRet.ack = 0;
		   }

		   header.m_catalog = header.m_catalog | 0x30;//设置ACK返回值为1
		   len = sizeof(AckRet);
		   header.m_lenH = len >> 8;
		   header.m_lenV = len & 0xff;
		   header.m_CheckSum = (unsigned char)CheckSum(&header);

		   response.append((char *)&header, sizeof(CtlHead));
		   response.append((char *)&sRet, sizeof(AckRet));
	   }break;
   default:
	   {
		   if (msg.size() - sizeof(CtlHead) > 0)
		   {
			   response.append(msg.c_str()+sizeof(CtlHead), msg.size()-sizeof(CtlHead));
		   }
	   }
   }
  
   return Send(nSocket, response.c_str(), response.size());
}

int CVmuxSocket::CheckSum(const CtlHead* head)
{
	int sum=0;
	unsigned char *ch = (unsigned char *)head;
	for (int i = 0; i < 7; i++)
	{
       sum += ch[i];
	}
	return (sum & 0xff);
}

bool CVmuxSocket::CreateSocket(int& nSocket, int nPort)
{
	//创建套接字
	if(mvCreateSocket(nSocket,1)==false)
	{
		LogError("创建套接字失败,服务无法启动!\r\n");
		return false;
	}

	//重复使用套接字
	if(mvSetSocketOpt(nSocket,SO_REUSEADDR)==false || !mvSetSocketOpt(nSocket, SO_SNDBUF))
	{
		LogError("设置重复使用套接字失败,服务无法启动!\r\n");
		return false;
	}

	//绑定服务端口
	if(mvBindPort(nSocket, nPort)==false)
	{
		LogError("绑定到 %d 端口失败,服务无法启动!\r\n", nPort);
		return false;
	}

	//开始监听
	if (mvStartListen(nSocket) == false)
	{
		LogError("监听连接失败，服务无法启动!\r\n");
		return false;
	}
	return true;
}
//接收数据连接
void CVmuxSocket::OnDataAccept()
{
	struct sockaddr_in clientaddr;
	socklen_t sin_size = sizeof(struct sockaddr_in);
	int nClient;

	while(!m_bEndStream)
	{
		if (g_bEndThread || m_bEndThread)
		{
			return;
		}
		//接受连接
		if((nClient = accept(m_nDataSocket, (struct sockaddr*)&clientaddr,&sin_size)) < 0)
		{
			//断开连接
			if(g_bEndThread || m_bEndThread)
			{
				return;
			}
			continue;
		}
		//LogTrace("vmux.log", "DataAccept: Server %s has connected! socket=%d,port:%d", inet_ntoa(clientaddr.sin_addr), nClient, ntohs(clientaddr.sin_port));
		LogNormal("接收vis数据连接m_vecDataSocket.size()=%d",m_vecDataSocket.size());
		pthread_mutex_lock(&m_Mutex);
		
		if(m_vecDataSocket.size() > 0)
		{
			vector<int>::iterator it = m_vecDataSocket.begin();
			int nSocket = *it;
			close(nSocket);
			shutdown(nSocket, 2);
			m_vecDataSocket.erase(it);
		}

	    m_vecDataSocket.push_back(nClient);
		pthread_mutex_unlock(&m_Mutex);

		usleep(1000*10);
	}
}

void CVmuxSocket::Unit()
{
  if (m_nDataSocket > 0)
  {
	  close(m_nDataSocket);
	  shutdown(m_nDataSocket, 2);
  }
  if (m_nAcceptSocket > 0)
  {
	  close(m_nAcceptSocket);
	  shutdown(m_nAcceptSocket, 2);
  }
  m_bEndThread = true;
  m_bEndStream = true;
  usleep(1000*100);
  LogTrace("vmux.log", "close vmux service!");
}

#endif