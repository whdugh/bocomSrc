// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "Common.h"
#include "CommonHeader.h"
#include "RoadSocket.h"

//通讯服务
CSkpRoadSocket g_skpRoadServer;

//监控线程
void* ThreadAccept(void* pArg)
{
	int nSocket =*(int*)pArg;
	//客户端连接
	struct sockaddr_in clientaddr;
	//长度
	socklen_t sin_size = sizeof(struct   sockaddr_in);
	int nClient = 0;
	while(!g_bEndThread)
	{				
		//close(0);//test..
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
		LogNormal("客户端连接[IP:%s][nClient = %d,端口:%d]!\r\n", \
			inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));

		SRIP_CLEINT sClient;
		sClient.nSocket = nClient;
		sClient.nPort = ntohs(clientaddr.sin_port);

		sClient.uTimestamp = GetTimeStamp();
		strcpy(sClient.chHost,inet_ntoa(clientaddr.sin_addr));

		//保存客户端,并启动接收数据线程
		g_skpRoadServer.AddClient(sClient);
		LogNormal("增加客户端连接完成[IP:%s][nClient = %d,端口:%d]!\r\n", \
			inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));

		//10毫秒
		usleep(1000*10);
	}
	LogNormal("41020客户端连接服务关闭\n");
	pthread_exit((void *)0);
	return pArg;
}

//心跳检测线程
void* ThreadLink(void* pArg)
{
	CSkpRoadSocket* pSocket = (CSkpRoadSocket*)pArg;
	if(pArg == NULL) return pArg;
	while(!g_bEndThread)
	{
		//检测状态一次
		pSocket->LinkTest();

		//5秒检测一次
		sleep(5);
	}
	pthread_exit((void *)0);
	return pArg;
}

//客户端接收线程
void* ThreadRecv(void* pArg)
{
	int client =*(int*)pArg;
	int nBytes = 0;

	char chBuffer[SRIP_MAX_BUFFER];
	while(!g_bEndThread)
	{
	    MIMAX_HEADER mHeader;
		std::string response;
		//接收HTTP 头，一次性接收
		if((nBytes = recv(client,(void*)&mHeader,sizeof(mHeader),MSG_NOSIGNAL)) < 0)
		{
			//断开连接
			if(g_bEndThread)
			{
			    pthread_exit((void *)0);
				return pArg;
			}

			LogError("接收协议头出错，连接断开! socket = %d,%s\r\n", client, strerror(errno));
			g_skpRoadServer.DelClient(client);
			pthread_exit((void *)0);
			return pArg;
		}

		/*if(nBytes == 0 && errno == 0)
		{
            continue;
		}*/

		//判断结构中的数据长度，小于本身结构的长度，错误
		if(mHeader.uCmdLen < sizeof(mHeader))
		{
			LogError("接收协议头结构错误，连接断开!,%s,nBytes=%d,mHeader.uCmdLen=%d,errno=%d\r\n",strerror(errno),nBytes,mHeader.uCmdLen,errno);
			g_skpRoadServer.DelClient(client);
			pthread_exit((void *)0);
			return pArg;
		}

		///////////将博康接口转换
		SRIP_HEADER sHeader;
		sHeader.uMsgCommandID = mHeader.uCmdID;
		sHeader.uCmdFlag = mHeader.uCmdFlag;
		sHeader.uMsgSource = mHeader.uCameraID;

		//保存头
		response.append((char*)&sHeader,sizeof(sHeader));

		//接收剩下的数据
		int nLeft = mHeader.uCmdLen - sizeof(mHeader);

		while(nLeft >  0)
		{
			//接收后面的数据
			if((nBytes = recv(client,chBuffer,sizeof(chBuffer)<nLeft?sizeof(chBuffer):nLeft,MSG_NOSIGNAL)) < 0)
			{
				//断开连接
				if(g_bEndThread)
				{
				    pthread_exit((void *)0);
					return pArg;
				}
				LogError("接收后续消息出错，连接断开!,%s\r\n",strerror(errno));

				g_skpRoadServer.DelClient(client);
				pthread_exit((void *)0);
				return pArg;
			}
			//保存数据
			response.append(chBuffer,nBytes);
			nLeft -= nBytes;
		}

		//将命令传送到处理模块,继续处理下一个命令
		g_skpRoadMsgCenter.AddMsg(client,(sHeader.uMsgCommandID),response);

		//1毫秒
		usleep(1000);

	}
	pthread_exit((void *)0);
	return pArg;
}

//构造
CSkpRoadSocket::CSkpRoadSocket()
{
	pthread_mutex_init(&m_thread_mutex,NULL);
	//发送信号

	m_nAcceptSocket = 0;
	m_nUdpSocket = 0;

	return;
}
	//析构
CSkpRoadSocket::~CSkpRoadSocket()
{
	//发送信号
	//
	pthread_mutex_destroy(&m_thread_mutex);


	//关闭连接
	mvCloseSocket(m_nAcceptSocket);

	return;
}

//启动侦听服务
bool CSkpRoadSocket::Init()
{
    //创建套接字
	if(mvCreateSocket(m_nAcceptSocket,1)==false)
	{
	    LogError("创建套接字失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//重复使用套接字
	if(mvSetSocketOpt(m_nAcceptSocket,SO_REUSEADDR)==false)
	{
	    LogError("设置重复使用套接字失败,服务无法启动!,%s\r\n",strerror(errno));
	    g_bEndThread = true;
		return false;
	}

	//////////////////////////
	m_nPort = g_nEPort;
	//绑定服务端口
	if(mvBindPort(m_nAcceptSocket,m_nPort)==false)
	{
        LogError("绑定到 %d 端口失败,服务无法启动!\r\n",m_nPort);
        printf("%s\n",strerror(errno));
	    g_bEndThread = true;
		return false;
	}

	//开始监听
	if (mvStartListen(m_nAcceptSocket) == false)
	{
		LogError("监听连接失败，服务无法启动!\r\n");
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
	int nret=pthread_create(&id,&attr,ThreadAccept,(void*)&m_nAcceptSocket);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建事件接收线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动心跳检测线程
	nret=pthread_create(&id,&attr,ThreadLink,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建连接心跳检测线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);

	// 创建发送视频用UDP套接字
	if (mvCreateSocket(m_nUdpSocket, 2) == false)
	{
		LogError("发送视频用套接字创建失败!服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	if (mvSetSocketOpt(m_nUdpSocket, SO_REUSEADDR) == false)
	{
		LogError("设置重复使用套接字失败,服务无法启动!,%s\r\n",strerror(errno));
		g_bEndThread = true;
		return false;
	}
	return true;
}

//释放
bool CSkpRoadSocket::UnInit()
{
    //需要关闭和删除掉所有的客户端连接
    DelClient(0,true);
    //关闭连接
    mvCloseSocket(m_nAcceptSocket);
	mvCloseSocket(m_nUdpSocket);

	return true;
}


//传送数据到客户端
bool CSkpRoadSocket::SendToClient(std::string& strData)
{
	//服务端的发送数据为向所有的连接发送数据
	bool bRet = true;
	//取所有连接，发送数据
    pthread_mutex_lock(&m_thread_mutex);
	if(m_ClientMap.size() > 0)
	{
		//查询列表
		ClientMap::iterator it = m_ClientMap.begin();
//REV:
		MIMAX_HEADER* mHeader = (MIMAX_HEADER*)strData.c_str();
		while(it != m_ClientMap.end())
		{
                //需要判断连接类型,是中心端连接（bClient=false）还是客户端连接（bClient=true）
                /////////////////////////////////////
                bool bClient = true;
                //printf("bClient = %d,it->second.bVerify = %d\n",bClient,it->second.bVerify);
                //客户端连接
                if(bClient &&(it->second.bVerify))
                {
                         if(mHeader->uCmdID==MIMAX_FRAME
                        ||mHeader->uCmdID==SRIP_FRAME_RESULT
                        ||mHeader->uCmdID==SRIP_CARD_RESULT)
                        {
                            if(it->second.bConnect==0)//不发送
                            {
                                it++;
                                continue;
                            }
                        }

					if (mHeader->uCmdID == MIMAX_FRAME && it->second.ClientPort) //如果ClientPort存在, 用udp发送图片帧
					{
						string strMsg(strData);
						//数据加密
						MIMAX_HEADER* mHeader = (MIMAX_HEADER*)strMsg.c_str();
						if(mHeader->uCmdLen  > sizeof(MIMAX_HEADER))
						{
							//补充四个字节用于存放crc循环校验码
							UINT32 crc32 = 0;
							strMsg.append( (char *)&crc32, sizeof(UINT32) );
							EncodeBuff( (char*)strMsg.c_str(), 0);
						}
						//加入同步字符串
						strMsg.insert(0, "$$$$", strlen("$$$$"));

						struct sockaddr_in pin;
						bzero(&pin, sizeof(pin));
						pin.sin_family = AF_INET;
						inet_pton(AF_INET, it->second.chHost, &pin.sin_addr);
						pin.sin_port = htons(it->second.ClientPort);
						int ret = sendto(m_nUdpSocket, strMsg.c_str(), strMsg.size()+1, MSG_NOSIGNAL, (struct sockaddr *)&pin, sizeof(pin));
						if (-1 == ret)
						{
							LogError("发送图片帧错误!ClientIP=%s,strMsg.size()=%d\n", it->second.chHost,strMsg.size());
						}
						it ++;
						continue;
					}
				
                   //printf("=========before send\n");
                   int nSocket = it->first;
                   //tcp发送视频数据和检测信息
                    if(!SendMsg(nSocket,strData,bClient))
                    {
                        LogError("ip = %s, socket = %d, 发送出错：%s\r\n",it->second.chHost, nSocket, strerror(errno));
                        bRet = false;

                        //关闭连接
                        mvCloseSocket(nSocket);

                        //删除命令列表中的数据
                        g_skpRoadMsgCenter.DelMsg(nSocket);

                        m_ClientMap.erase(it++);
                        continue;
                    }
                }
                //printf("=========after send\n");
               it ++;
               usleep(10);
                /////////////////////////////////////
		}
	}
	pthread_mutex_unlock(&m_thread_mutex);

	return bRet;
}

//发送数据
bool CSkpRoadSocket::SendMsg(const int nSocket,std::string strData,bool bClient)
 {
    if(nSocket <= 0)
    {
      return false;
    }
     //数据加密
    //中心端和客户端均需要加密
    MIMAX_HEADER* mHeader = (MIMAX_HEADER*)strData.c_str();
    if(mHeader->uCmdLen  > sizeof(MIMAX_HEADER))
    {
        //补充四个字节用于存放crc循环校验码
        //if(!bClient)
        {
            UINT32 crc32 = 0;
            strData.append( (char *)&crc32, sizeof(UINT32) );
            EncodeBuff( (char*)strData.c_str(), 0);
        }
    }

    if(bClient)//如果是发送客户端
    {
        strData.insert(0, "$$$$", strlen("$$$$"));
    }
    int nLeft = strData.size();
	int nDataLen = 0;
	int nBytes = 0;


    if(!mvSendMsgToSocket(nSocket,strData))
    {
        return false;
    }
    return true;
}

//添加客户端
bool CSkpRoadSocket::AddClient(SRIP_CLEINT sClient)
{
    int nClientSocket = sClient.nSocket;
    //必须先添加到列表
    pthread_mutex_lock(&m_thread_mutex);

    ClientMap::iterator it = m_ClientMap.find(nClientSocket);
    if (it != m_ClientMap.end())
    {
        //关闭连接
        //mvCloseSocket(nClientSocket);

        g_skpRoadMsgCenter.DelMsg(nClientSocket);
        m_ClientMap.erase(it);

        LogError("same Client nClientSocket=%d,sClient.chHost=%s\r\n",nClientSocket,sClient.chHost);
    }
    m_ClientMap.insert(ClientMap::value_type(nClientSocket, sClient));
    pthread_mutex_unlock(&m_thread_mutex);

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
	int nret=pthread_create(&id,&attr,ThreadRecv,(void*)&nClientSocket);

#ifdef _THREAD_DEBUG
	printf("添加客户端线程启动!(CSkpRoadSocket::Init:ThreadRecv)\r\n");
#endif
	//成功
	if(nret!=0)
	{
		DelClient(nClientSocket);
		//失败
		LogError("创建客户端接收数据线程失败,连接断开!\r\n");
		return false;
	}
    pthread_attr_destroy(&attr);
	usleep(100);
	return true;
}

//删除客户端
bool CSkpRoadSocket::DelClient(int nSocket,bool bAll)
{
	//接收数据错误
#ifdef _DEBUG
	LogError("接收数据错误,客户端被断开! %d\r\n",nSocket);
#endif
	//获取连接的客户端数目
	int nConnectClientCount =  GetConnectClientCount();
	//查询列表
	pthread_mutex_lock(&m_thread_mutex);

	if(bAll)
	{
		ClientMap::iterator it = m_ClientMap.begin();
		while(it != m_ClientMap.end())
		{
			int nSocket = it->first;

			//关闭连接
			mvCloseSocket(nSocket);

			//删除命令列表中的数据
			g_skpRoadMsgCenter.DelMsg(nSocket);

			it++;
		}
		m_ClientMap.clear();
		pthread_mutex_unlock(&m_thread_mutex);
		//关闭所有通道
		g_skpChannelCenter.SetChannelConnect(false);

		//删除通道中心列表中的数据
        g_skpChannelCenter.Clear();

		return true;
	}
	else
	{
		ClientMap::iterator it = m_ClientMap.find(nSocket);
		if(it != m_ClientMap.end())
		{
		    printf("DelClient  nSocket=%d\r\n",nSocket);

            {
                if((nConnectClientCount == 1) && (it->second.bConnect))
                {
                    //关闭所有通道
                    g_skpChannelCenter.SetChannelConnect(false);
                    //删除通道中心列表中的数据
                    g_skpChannelCenter.Clear();
                }
            }
			//关闭连接
			mvCloseSocket(nSocket);

            //删除命令列表中的数据
			g_skpRoadMsgCenter.DelMsg(nSocket);
			//删除连接
			m_ClientMap.erase(it);

			pthread_mutex_unlock(&m_thread_mutex);
			return true;
		}
	}

	//关闭连接
	mvCloseSocket(nSocket);

	pthread_mutex_unlock(&m_thread_mutex);
	return false;
}

//心跳检测
void CSkpRoadSocket::LinkTest()
{
    //zhangyaoyao:该函数对一把锁进行了4次获取和释放，感觉结构有些不合理，暂时按优先级把发送心跳提前处理
	/////////////////////////////发送心跳信号
	//协议头
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (PLATE_LINK);
	//消息长度
	mHeader.uCmdLen = sizeof(mHeader);
	std::string request;
	//数据
	request.append((char*)&mHeader,sizeof(mHeader));
	//printf("*************************linktest\n");

	int nClientCount = GetConnectClientCount();

	pthread_mutex_lock(&m_thread_mutex);
	//查询列表
	ClientMap::iterator it = m_ClientMap.begin();
	while(it != m_ClientMap.end())
	{
	    bool bClient = true;

        int nSocket = it->first;
	    //发送心跳信号//心跳值大于设定的阀值,断开连接
	    if( (!SendMsg(nSocket,request,bClient)) /*|| (it->second.nLinker > SRIP_LINK_MAX)*/)
	    {
            if(bClient)
            {
                if ((nClientCount == 1) && (it->second.bConnect))
                {
                    g_skpChannelCenter.SetChannelConnect(false);
                    g_skpChannelCenter.Clear();
                }
            }
            //关闭连接
            mvCloseSocket(nSocket);

            //删除命令列表中的数据
            g_skpRoadMsgCenter.DelMsg(nSocket);
            m_ClientMap.erase(it++);

            LogError("LinkTest disconnect IP = %s\n", it->second.chHost);
            continue;
	    }
        //计数自加
        it->second.nLinker ++;
        it ++;
	}
	pthread_mutex_unlock(&m_thread_mutex);

	return;
}

//重置心跳检测值
bool CSkpRoadSocket::ResetLinker(const int nSocket)
{
	pthread_mutex_lock(&m_thread_mutex);
	//查询列表
	ClientMap::iterator it = m_ClientMap.find(nSocket);
	if(it != m_ClientMap.end())
	{
		//重置
		it->second.nLinker = 0;
        //LogNormal("reset linker, IP = %s, socket = %d", it->second.chHost, nSocket);
		//解锁
		pthread_mutex_unlock(&m_thread_mutex);
		return true;
	}
	//解锁
	pthread_mutex_unlock(&m_thread_mutex);

	return false;
}

//保存登录用户
bool CSkpRoadSocket::SaveUser(const int nSocket,const std::string user,const int port,int nClientKind)
{
	pthread_mutex_lock(&m_thread_mutex);
	//查询列表
	ClientMap::iterator it = m_ClientMap.find(nSocket);
	if(it != m_ClientMap.end())
	{
		it->second.bVerify = true;
		//保存用户名
		strncpy(it->second.chUserName,user.c_str(),sizeof(it->second.chUserName) -1);

		it->second.ClientPort = port;
		it->second.nClientKind = nClientKind;
		it->second.nLinker = 0;
		//printf("777777777777777777777it->second.ClientPort=%d\n",it->second.ClientPort);
		//解锁
		pthread_mutex_unlock(&m_thread_mutex);
		//返回
		return true;
	}
	//解锁
	pthread_mutex_unlock(&m_thread_mutex);
	return false;
}

//获取连接数目
int CSkpRoadSocket::GetConnectClientCount()
{
    int nConnectClientCount = 0;
    pthread_mutex_lock(&m_thread_mutex);
    ClientMap::iterator it = m_ClientMap.begin();
    while(it!=m_ClientMap.end())
    {
        if(it->second.bConnect)
        {
            nConnectClientCount++;
        }
        it++;
    }
    pthread_mutex_unlock(&m_thread_mutex);
	return nConnectClientCount;
}

//设置是否发送数据
void CSkpRoadSocket::SetConnect(const int nSocket,bool bConnect)
{
	pthread_mutex_lock(&m_thread_mutex);

	ClientMap::iterator it = m_ClientMap.find(nSocket);
	if(it != m_ClientMap.end())
	{
	    printf("SetConnect=%d\n",bConnect);
		it->second.bConnect = bConnect;
	}
	//解锁
	pthread_mutex_unlock(&m_thread_mutex);
}

// zhangyaoyao: get user's username through socket
string CSkpRoadSocket::GetClientName(int nSocket)
{
    string strUsrName("");
    ClientMap::iterator it = m_ClientMap.find(nSocket);
    if (it != m_ClientMap.end())
        strUsrName = (it->second).chUserName;
    if (!strUsrName.empty())
        ToLowerCase(strUsrName);
    return strUsrName;
}
