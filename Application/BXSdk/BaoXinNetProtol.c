#include "BXTypes.h"

#include "BaoXinNetProtol.h"

#define BX_CTRL_PORT 35000 //宝信控制命令端口
#define BX_DATA_PORT 45000 //宝信数据端口
#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 1024
#define OUT_MS 200 //超时时间
extern BX_CB_FUNS *g_bx_cb_funs;

typedef struct
{
	time_t lastRecordTime;//上次记录时间
	int minRecords;//记录个数

}sRecordInfo;

sRecordInfo g_RecordInfo;

typedef struct
{
	pthread_t pid;
	int shutdown;

}sThread_info;

typedef struct _sClientInfo
{
	int i32ClientSockId;	//socket id
	time_t connecttime;//连接时间
	int serverPort;
	int i32ServerSockId;
	struct sockaddr_in client_addr;//网络信息
}sClientInfo;


sThread_info g_CtrlThrdInfo;

sThread_info g_DataThrdInfo;
sClientInfo *g_p45000Client = NULL;

int g_CtrlPort = BX_CTRL_PORT;
int g_DataPort = BX_DATA_PORT;
int create_tcp_server( unsigned int port );
int get_new_connect( int i32ServerFd,int i32TimeOut,sClientInfo *pClientInfo );
int monitor_client(sClientInfo *pClientInfo,int i32TimeOut);
int deal35000msg( sClientInfo *pClientInfo,char *msg , int len );

void* CreateServer( void*arg );

int BX_CreateServer()
{
	g_CtrlThrdInfo.pid = 0;
	g_CtrlThrdInfo.shutdown = 0;
	g_DataThrdInfo.pid = 0;
	g_DataThrdInfo.shutdown = 0;

	int ret = pthread_create(&g_CtrlThrdInfo.pid, NULL, (void *)CreateServer, (void*)&g_CtrlPort);

	if (ret != 0)
	{
		printf("BX_CreateServer failed exit,%d!\n",g_CtrlPort);
		return BX_SERVER1_FAILED;
	}
	
	ret = pthread_create(&g_DataThrdInfo.pid, NULL, (void *)CreateServer, (void*)&g_DataPort);
	if (ret != 0)
	{
		g_CtrlThrdInfo.shutdown = 1;
		pthread_join( g_CtrlThrdInfo.pid ,NULL);
		printf("BX_CreateServer failed exit,%d!\n",g_DataPort);
		return BX_SERVER2_FAILED;
	}


	return BX_OK;
}

int BX_CloseServer()
{
	g_CtrlThrdInfo.shutdown = 1;
	g_DataThrdInfo.shutdown = 1;

	if ( g_CtrlThrdInfo.pid != 0 )
	{
		pthread_join( g_CtrlThrdInfo.pid , NULL );
		g_CtrlThrdInfo.pid = 0;
	}

	if ( g_DataThrdInfo.pid != 0 )
	{
		pthread_join( g_DataThrdInfo.pid , NULL );
		g_DataThrdInfo.pid = 0;
	}


	return BX_OK;
}




void* CreateServer( void* arg )
{
	if (arg == NULL)
	{
		return 0;
	}
	int port = *(int*)arg;

	sClientInfo clientInfo;//client连接信息
	memset(&clientInfo,0,sizeof(clientInfo));
	int i32Ret = -1;
	//只支持单连接
	int listenfd = create_tcp_server( port );

	if (listenfd < 0 )
	{
		printf("%d端口创建连接失败\n",port);
		return NULL;	
	}
	printf("%d端口创建连接成功\n",port);
	//记录客户端连接的服务端信息
	clientInfo.serverPort = port;
	clientInfo.i32ServerSockId = listenfd;

	while(g_CtrlThrdInfo.shutdown == 0)
	{
		if(clientInfo.connecttime == 0)
		{
			//printf("%d端口等待连接\n",port);
			i32Ret = get_new_connect( listenfd, OUT_MS, &clientInfo);
			if (i32Ret != 0)
			{
				usleep(100000);
				continue;
			}
			if (port == BX_DATA_PORT)
			{
				g_p45000Client = &clientInfo;//记录45000数据端口的信息
			}
		}
		i32Ret = monitor_client(&clientInfo,OUT_MS);
		//连接断开
		if (i32Ret == -1)
		{
			clientInfo.connecttime = 0;//连接时间清0

			close(clientInfo.i32ClientSockId);
			clientInfo.i32ClientSockId = -1;

		}
		
	}
}



int create_tcp_server(unsigned int port)
{
	struct sockaddr_in server;
	int server_sock_fd = -1;    
  
	int on = 1;    
	int keepalive = 1; // 开启keepalive属性 
	int keepidle = 60; // 如该连接没有任何数据往来,则进行探测 
	int keepinterval = 2; // 探测时发包的时间间隔为2 秒 
	int keepcount = 2; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发. 
	struct timeval timeo = {1,10000}; 
 
	server_sock_fd = socket(AF_INET, SOCK_STREAM, 0); //socket(AF_INET,SOCK_DGRAM,0);    
	server.sin_family = AF_INET;    
	server.sin_addr.s_addr = INADDR_ANY;    
	server.sin_port = htons(port);    
	setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on));  
	setsockopt(server_sock_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive )); 
	setsockopt(server_sock_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle )); 
	setsockopt(server_sock_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval )); 
	setsockopt(server_sock_fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount )); 
	setsockopt(server_sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)); 
	setsockopt(server_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo)); 
	if (bind(server_sock_fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) 
	{        
		close(server_sock_fd);//应该退出        
		return -1;   
	}    

	if ( listen( server_sock_fd, 5) == 1)
	{
		close( server_sock_fd );
		return -1;
	}

	return server_sock_fd;
}

int get_new_connect( int i32ServerFd,int i32TimeOut,sClientInfo *pClientInfo )
{

	int i32Ret = -1;
	int addresslen;
	struct pollfd   fds[1];//监控socket

	//校验参数
	if(pClientInfo == NULL)
	{
		return i32Ret;
	}

	addresslen = sizeof(pClientInfo->client_addr);

	fds[0].fd = i32ServerFd;
	fds[0].events=POLLIN;//注册可读事件

	i32Ret = poll(fds,1,i32TimeOut);
	//poll的返回值-1表示失败，0表示无事件，1表示有一个事件
	if ( i32Ret == 1 && fds[0].revents & POLLIN)
	{
		//等待网络连接     
		pClientInfo->i32ClientSockId = accept(i32ServerFd, (struct sockaddr *)&pClientInfo->client_addr, (socklen_t *)&addresslen);    

		//接收连接失败
		if(pClientInfo->i32ClientSockId < 1)
		{
			pClientInfo->i32ClientSockId = -1;
			i32Ret = -1;
		}
		else
		{
			i32Ret = 0;
			pClientInfo->connecttime = time(NULL);//记录当前连接时间
			printf("%d端口新连接，%s:%d\n",pClientInfo->serverPort,inet_ntoa(pClientInfo->client_addr.sin_addr),htons(pClientInfo->client_addr.sin_port));
		}

	}
	else
	{
		i32Ret = -1;
	}
	return i32Ret;
}
int monitor_client( sClientInfo *pClientInfo,int i32TimeOut )
{
	int i32Ret = -1;
	char recvbuf[6] = {0};
	struct    pollfd   fds[1];

	if (pClientInfo == NULL)
	{
		return i32Ret;
	}

	fds[0].fd = pClientInfo->i32ClientSockId;
	fds[0].events=POLLIN ;//注册可读事件

	i32Ret = poll(fds,1,i32TimeOut);
	//poll的返回值-1表示失败，0表示无事件，1表示有一个事件
	if ( i32Ret == 1 && fds[0].revents & POLLIN)
	{
		
		i32Ret = recv( fds[0].fd,recvbuf, sizeof( recvbuf ), 0);

		if (i32Ret == 0)//对方关闭了
		{
			printf("%d端口连接被断开,%s:%d\n",pClientInfo->serverPort,inet_ntoa(pClientInfo->client_addr.sin_addr),htons(pClientInfo->client_addr.sin_port));
			
			
			i32Ret = -1;
		}
		else if (i32Ret == -1 && errno == ETIMEDOUT)//keepalive断开
		{
			printf("%d端口网络异常%s:%d\n",pClientInfo->serverPort,inet_ntoa(pClientInfo->client_addr.sin_addr),htons(pClientInfo->client_addr.sin_port));

			i32Ret = -1;
		}
		else
		{
			if ( pClientInfo->serverPort == BX_CTRL_PORT )
			{
				deal35000msg( pClientInfo,recvbuf,sizeof(recvbuf) );
			}
			i32Ret = 0;
		}
	}
	else
	{
		i32Ret = 1;
	}
	return i32Ret;

}

int deal35000msg( sClientInfo *pClientInfo,char *msg , int len )
{
	if (pClientInfo == NULL || msg == NULL || len == 0)
	{
		return -1;
	}

	BX_Int32 i32Ret = 0;
	char retmsg[128] = {0};
	int retLen = 0;
	char recvbody[8] = {0};
	int recvBodylen = 0;
	memcpy(&recvBodylen,msg+2,sizeof(recvBodylen));
	printf("接收bodylen=%d\n",recvBodylen);

	switch( msg[0])
	{
	case 0x70://获取状态
		{

			retmsg[0] = 0x18;//返回24个字节
			int statusType = msg[5];//状态类型
			if (statusType == 0x00)//设备状态
			{
				retmsg[16] = 0x00;
			}
			else if (statusType == 0x01)//1分钟内流量
			{
				retmsg[16] = 0x01;
				//if (time(NULL) - g_RecordInfo.lastRecordTime <= 60)
				//{
				//	retmsg[17] = g_RecordInfo.minRecords;
				//}
				//else
				//{
				//	g_RecordInfo.minRecords = 0;//上次记录时间超时归0
				//}
				retmsg[17] = g_RecordInfo.minRecords;
				g_RecordInfo.minRecords = 0;//查询后记录时间归0
			}
			else if (statusType == 0x04)//时间
			{
				BX_Uint64 nowTime;
				i32Ret = g_bx_cb_funs->GetDeviceTime(&nowTime);

				printf("nowTime:%llu\n",nowTime);
				if (i32Ret != 0)
				{
					return -1;
				}
				//转化为宝信时间
				char bxtime[8] = {0};

				struct tm *nowtime;
				time_t thisSecond = nowTime/1000;
				nowtime = localtime( &thisSecond );
				bxtime[0] = nowtime->tm_year+1900-2000;//年
				bxtime[1] = nowtime->tm_mon+1;
				bxtime[2] = nowtime->tm_mday;
				bxtime[3] = nowtime->tm_wday;
				bxtime[4] = nowtime->tm_hour;
				bxtime[5] = nowtime->tm_min;
				bxtime[6] = nowtime->tm_sec;
				bxtime[7] = nowTime%1000;//取毫秒？？？毫秒最大255？有疑问
				printf("获取时间%d年-%d月-%d日-周%d %d:%d:%d-%d\n",\
					bxtime[0],bxtime[1],bxtime[2],bxtime[3],bxtime[4],bxtime[5],bxtime[6],bxtime[7]);
				//返回信息填充
				
				retmsg[16] = 0x04;//时间
				memcpy(retmsg+20,bxtime,sizeof(bxtime));
			}

			break;
		}
		
	case 0x18://设置时间
		{
			
			int len = recv(pClientInfo->i32ClientSockId,recvbody,recvBodylen,0); //msg[2]回复字节数
			if( len != recvBodylen)
			{
				printf("设置时间接收失败\n");
				return -1;
			}
			printf("设置时间%d年-%d月-%d日-%d周 %d:%d:%d-%d\n",\
				recvbody[0],recvbody[1],recvbody[2],recvbody[3],recvbody[4],recvbody[5],recvbody[6],recvbody[7]);

			struct tm nowtime;
			nowtime.tm_year = recvbody[0]+2000-1900;
			nowtime.tm_mon = recvbody[1]-1;
			nowtime.tm_mday = recvbody[2];
			nowtime.tm_wday = recvbody[3]-1;
			nowtime.tm_hour = recvbody[4];
			nowtime.tm_min = recvbody[5];
			nowtime.tm_sec = recvbody[6];
			
			time_t thisSecond = mktime(&nowtime);
	
			BX_Uint64 nowTime = ((BX_Uint64)thisSecond)*1000 + recvbody[7];
			i32Ret = g_bx_cb_funs->SetDeviceTime( nowTime );
			if( i32Ret != 0)
			{
				printf("设置时间失败\n");
				return -1;
			}

			break;
		}
		
	case 0x31://获取分辨率
		{
			
			int len = recv(pClientInfo->i32ClientSockId,recvbody,recvBodylen,0);
			if(len != recvBodylen)
			{
				printf("获取分辨率接收失败\n");
				return -1;
			}
			BX_Uint32 videoW=0,videoH=0;
			i32Ret = g_bx_cb_funs->GetDeviceWH(&videoW,&videoH);
			printf("获取分辨率%d*%d\n",videoW,videoH);
			if (i32Ret != 0)
			{
				return -1;
			}

			//返回信息填充
			retmsg[0] = 0x0c;//返回12个字节
			
			memcpy(retmsg+4,&videoW,sizeof(videoW));
			memcpy(retmsg+4+sizeof(videoW),&videoH,sizeof(videoH));
			
		}
		break;
	default:
		printf("未知类型 0x%x\n",msg[0]);
		break;
	}
	//需要回复
	if (msg[1] == 1)
	{
		retLen = 4+retmsg[0];//4个位长度加上实际返回的长度
		i32Ret = send(pClientInfo->i32ClientSockId,retmsg,retLen,0);
	}
	
	return i32Ret;
}

int BX_45000Send(const unsigned char *sendData,int len,int state)
{
	int iRet = -1;
	int sendlen = 0;

	if (state == 0 )//实时数据
	{
		//流量统计
		g_RecordInfo.minRecords++;
		g_RecordInfo.lastRecordTime = time(NULL);//记录当前时间
	}

	
	if (g_p45000Client != NULL && g_p45000Client->connecttime != 0)
	{
		
		while(sendlen < len)
		{
			iRet = send(g_p45000Client->i32ClientSockId,sendData+sendlen,len-sendlen,MSG_NOSIGNAL);
			if(iRet <= 0)
			{
				printf("-----------sendlen =%d  iRet =%d  len=%d \n",sendlen,iRet,len);
				perror("send error !!");
				if (errno == EAGAIN )
				{
					usleep(1000);
					continue;
				}
				break;
			}
			sendlen += iRet;
			printf("sendlen =%d  iRet =%d  len=%d \n",sendlen,iRet,len);
		}
		if (iRet >0 )
		{
			iRet = BX_OK;
		}
		else
		{
			iRet = BX_SEND_ERROR;
		}
	}
	else
	{
		iRet = BX_NO_CONNECT;
	}

	printf("sendlen =%d\n",sendlen);

	

	return iRet;
}


