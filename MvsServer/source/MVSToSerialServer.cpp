#include "Common.h"
#include "MVSToSerialServer.h"
#include "VisNetCommunication.h"
#include <math.h>
#include "RoadChannelCenter.h"
#include "CSeekpaiDB.h"
#include "XmlParaUtil.h"
#include <time.h>

CMvsToSerialServer g_MVSToSerialServer;

int CMvsToSerialServer::m_catGet = 0;
int CMvsToSerialServer::m_getMsgTime = 0;


void *RecvSerialServerMsgThread(void *pArg)
{
	//while (!g_bEndThread)
	{
		g_MVSToSerialServer.mvRecvSerialMsg();

		//usleep(100);
	}

	LogError("接收串口服务器消息线程退出\r\n");

	pthread_exit((void *)0);
}

//得到socke描述符
int CMvsToSerialServer::getSokeFd()                 
{
	return m_SerialSocket;
}

// 设置socke描述符
void CMvsToSerialServer::setSockFd(int socketFd)
{
	m_SerialSocket = socketFd;
}

//  这个函数是调用 CMvsToSerialServer检测连接线程
void * TestRecvSerialServerMsgThread(void * PAg)
{
	g_MVSToSerialServer.testConnetServerGetStatus();
}



 // 开启检测连接的问题 
bool CMvsToSerialServer::testConnectThread()
{
	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	//mvJoinThread(m_testURecvSSMsgThreadId);
	if (pthread_create(&m_testURecvSSMsgThreadId, &attr,TestRecvSerialServerMsgThread, NULL) != 0)
	{
		pthread_attr_destroy(&attr);
		return false;
	}

	pthread_attr_destroy(&attr);
	
	return true;
}

void CMvsToSerialServer::testConnetServerGetStatus()  //隔一定的时间去连接服务器，得到连接的状态
{
	while(m_testThread)
	{
		
		sleep(6*60);                                  //每隔5分钟检测一次
		int persentTime = time((time_t*)NULL);
		if (persentTime - m_getMsgTime > 6*30)          //6分钟后还没有收到回码都重连
		{
			m_getMsgTime = time((time_t *)NULL);
			g_MVSToSerialServer.mvCloseSocket(m_SerialSocket);
			g_MVSToSerialServer.setSockFd(-1);
			while(g_MVSToSerialServer.getSokeFd() <= 0)
			{
				//LogNormal("------vis 重连 ing \n");
				g_MVSToSerialServer.mvConnOrLinkTest();      // 重连
				if (g_MVSToSerialServer.getSokeFd() > 0)     // 重连成功，跳出循环， 否则都一直去连接
				{
					break;
				} 
				sleep(3);
			}
		}
	}


}
CMvsToSerialServer::CMvsToSerialServer()
{
	m_testURecvSSMsgThreadId = 0;
	m_testCanFlagThreadId = 0;
	m_uRecvSSMsgThreadId= 0;
	//m_bCenterLink = false;
	m_SerialSocket= -1;
	//m_nCSLinkCount = 0;
	m_strSerialHost = "";
	m_nSerialPort = 0;
	m_uRecvserialmsgtime = 0;
	m_testThread = 1;
	pthread_mutex_init(&m_mvtossMsg, NULL);

	//mvConnSerialAndRecvMsg();
}



CMvsToSerialServer::~CMvsToSerialServer()
{
   pthread_mutex_destroy(&m_mvtossMsg);
}	

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void CMvsToSerialServer::mvConnOrLinkTest()
{
	if (!g_bEndThread)
	{
		if (m_SerialSocket <= 0)
		{
			if (mvConnSerialAndRecvMsg())
			{
				LogNormal("连接串口服务器成功!\n");

				CXmlParaUtil xml;
				xml.LoadRemotePreSet(1,m_vRemotePreSet);
			}
			else
			{
				printf("连接串口服务器失败!\r\n");
			}
		}

		/*if(compareTime())
		{
			g_nPreSetMode = 0;
			g_skpChannelCenter.RestartDetect();
			LogNormal("外部不在控制，启动检测!");
		}*/
	}

}

bool CMvsToSerialServer::mvConnSerialAndRecvMsg()
{
	if (!mvConnToSerial())
	{
		return false;
	}

	if (!theThreadStart())
	{
		return false;
	}

	return true;
}

/*
* 函数介绍：连接到串口服务器
*/
bool CMvsToSerialServer::mvConnToSerial()
{
	m_strSerialHost = g_ytControlSetting.szSerialHost;
	m_nSerialPort = g_ytControlSetting.nSerialPort;

	//LogNormal("m_strSerialHost.c_str()=%s,m_nSerialPort=%d",m_strSerialHost.c_str(),m_nSerialPort);

	//connect to center server and set socket's option.
	if (m_strSerialHost.empty() || m_strSerialHost == "0.0.0.0" || m_nSerialPort <= 0)
	{
		//printf("\n中心数据服务器连接参数异常:host=%s,port=%d\n", g_strVisHost.c_str(), g_nVisPort);
		return false;
	}

	if (!mvPrepareSocket(m_SerialSocket))
	{
		//printf("\n准备连接中心数据服务器套接字失败!\n");
		mvCloseSocket(m_SerialSocket);
		return false;
	}

	if (!mvWaitConnect(m_SerialSocket, m_strSerialHost, m_nSerialPort,2))
	{
		//printf("\n尝试连接中心数据服务器失败!\n");
		mvCloseSocket(m_SerialSocket);
		return false;
	}


/*	int sizeofdata=0;
	int senddata=0;
	unsigned char a,ntemp[7];
	int nBPos = -1;
	int nEPos = -1;
	unsigned int temp1=0;
	struct timeval timeo;
	socklen_t len = sizeof(timeo);
	timeo.tv_sec=1;
	timeo.tv_usec=500000;//超时1.5s

	std::string strMessage("");
	std::string strtOrder("");
	std::string buf("");
	struct sockaddr_in tcp_addr;

	if ((m_SerialSocket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		return false;
	}

	if(setsockopt(m_SerialSocket, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
	{
		shutdown(m_SerialSocket,2);
		close(m_SerialSocket);
		m_SerialSocket = -1;
		perror("setsockopt");
		return false;
	}


	//printf("chMonitorHost=%s,uMonitorPort=%d\n",g_monitorHostInfo.chMonitorHost,g_monitorHostInfo.uMonitorPort);

	bzero(&tcp_addr,sizeof(tcp_addr));
	tcp_addr.sin_family=AF_INET;
	tcp_addr.sin_addr.s_addr=inet_addr(192.168.60.160);
	tcp_addr.sin_port=htons(10015);

	int nCount = 0;
	while(nCount < 3)
	{
		if(connect(m_SerialSocket,(struct sockaddr *)&tcp_addr,sizeof(struct sockaddr))==-1)
		{
			if(nCount == 2)
			{
				perror("connect error");
				LogError("connect:%s\n",strerror(errno));
				if(m_SerialSocket!=-1)
				{
					shutdown(m_SerialSocket,2);
					close(m_SerialSocket);
					m_SerialSocket = false;
				}
				return false;
			}
		}
		else
		{
			break;
		}
		nCount++;
		usleep(1000*10);
	}


	unsigned char* pData = new unsigned char[200];
	sizeofdata=recv(m_SerialSocket,pData,100,0);

	if(sizeofdata==-1)
	{
		printf("recv socket error");
		if(pData)
		{
			delete []pData;
		}

		return false;
	}
	strMessage.append((char*)pData,100);
	nEPos=strMessage.find("welcome");

	if (nEPos<0)
	{
		return false;
	}

	std::string strHeader=strMessage.substr(nEPos,7);

	strMessage.erase(0,nEPos+7);*/

	return true;
}

/*
* 函数介绍：接收消息线程入口
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CMvsToSerialServer::theThreadStart()
{
	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	///////////////////////////////////
	//mvJoinThread(m_uRecvSSMsgThreadId);
	if (pthread_create(&m_uRecvSSMsgThreadId, &attr,RecvSerialServerMsgThread, NULL) != 0)
	{
		pthread_attr_destroy(&attr);
		return false;
	}

	pthread_attr_destroy(&attr);

	return true;
}


//接收串口服务器消息
void CMvsToSerialServer::mvRecvSerialMsg()
{
	/*string strMsg("");
	//receive msg and push it into the msg queue.
	if (mvRecvMsg(m_nCenterSocket, strMsg))
	{
		mvPushOneMsg(strMsg);
		return true;
	}
	else
	{
		return false;
	}*/
	sleep(3);
	int sizeofdata=0;
	int senddata=0;
	unsigned char a,ntemp[7];
	int nBPos = -1;
	int nEPos = -1;
	unsigned int temp1=0;
	time_t timep;
	char *recvmsgtime;


	std::string strMessage("");
	std::string strtOrder("");
	std::string buf("");

	unsigned char chData[200]={0};

	//printf("%s\n",strHeader.c_str());

	//printf("begin\n");
	while(!g_bEndThread)
	{
		temp1=0;
		nBPos=-1;
		sizeofdata=recv(m_SerialSocket,chData,100,MSG_NOSIGNAL);
		if(sizeofdata<=0)
		{

			//m_catGet = 0;               // 接受失败
			printf("recv socket error");
			//continue;
			shutdown(m_SerialSocket,2);
			close(m_SerialSocket);
			m_SerialSocket = -1;
			break;
		}
		m_getMsgTime = time((time_t *)NULL);        // 得到数据的时间
	
		printf("sizeofdata=%d,strMessage.size=%d\n",sizeofdata,strMessage.size());

		strMessage.append((char *)chData,sizeofdata);

		while(strMessage.size() > 0)
		{
			nBPos=strMessage.find(255);

			printf("nBPos=%d,strMessage.size=%d\n",nBPos,strMessage.size());
			if (nBPos>=0)
			{
				if(nBPos > 0)
				strMessage.erase(0,nBPos);

				if (strMessage.size()>=7)
				{
					strtOrder=strMessage.substr(nBPos,7);

					for (int i=0;i<7;i++)
					{
						ntemp[i]=strtOrder[i];
						printf("%X\n",ntemp[i]);
					}
					
					temp1 = 0;
					for (int i=1;i<6;i++)
					{
						temp1=temp1+ntemp[i];
						temp1=temp1&0xFF;
					}
					if (temp1==ntemp[6])
					{
						
						if(g_ytControlSetting.nAddressCode == ntemp[1])
						{
							LogNormal("naddress code=%d,camera address=%d\n",g_ytControlSetting.nAddressCode,ntemp[1]);

						    LogNormal("data = %d,%d,%d,%d\n",ntemp[2],ntemp[3],ntemp[4],ntemp[5]);
						//	if(compareMsgTime())
							{
								int nPreSet = 0;//预置位
								if((ntemp[2] == 0) && (ntemp[3] == 7) && (ntemp[4] == 0) && (ntemp[5] <= 0x20))//表示调用预置位
								{
									int nCurPreSet = g_skpDB.GetPreSet(1);

									nPreSet = ntemp[5];

									LogNormal("go to PreSet=%d,nCurPreSet=%d\n",ntemp[5],nCurPreSet);

									if(nPreSet != nCurPreSet)//切换到其他预置位
									{
										bool bPreSetInList = false;
										vector<int>::iterator it = m_vRemotePreSet.begin();
										while(it != m_vRemotePreSet.end())
										{
											if(nPreSet == *it)
											{
												bPreSetInList = true;
											}
											it++;
										}

										if(bPreSetInList)
										{
											LogNormal("调用列表中的预置位%d，启动检测!",nPreSet);
											
											CAMERA_CONFIG cfg;
											cfg.uType = 1;
											cfg.fValue = nPreSet;
											cfg.nIndex = GOTO_PRESET;

											g_skpChannelCenter.CameraControl(1,cfg);

											g_nPreSetMode = 0;
											g_skpChannelCenter.RestartDetect();
											g_skpChannelCenter.ReloadDetect(1);
											
										}
										else
										{
											CXmlParaUtil xml;
											PreSetInfoList ListPreSetInfo;
											xml.LoadPreSetInfo(1,nCurPreSet,ListPreSetInfo);

											bool bPreSetInLocalList = false;
											PreSetInfoList::iterator it_loc = ListPreSetInfo.begin();
											while(it_loc != ListPreSetInfo.end())
											{
												if(nPreSet == it_loc->nPreSetID)
												{
													bPreSetInLocalList = true;
												}
												it_loc++;
											}

											if(bPreSetInLocalList)
											{
												LogNormal("切换到远景预置位%d下的近景预置位%d，不处理!\n",nCurPreSet,nPreSet);
											}
											else
											{
												LogNormal("切换到其他远景预置位%d，停止检测!\n",nPreSet);
												g_nPreSetMode = 1;
												g_skpChannelCenter.PauseDetect(0);
											}
										}
									}
									else
									{
										LogNormal("回归预置位%d，启动检测!",nPreSet);
										g_nPreSetMode = 0;
										g_skpChannelCenter.RestartDetect();
									}
								}
								else  if((ntemp[2] == 0) && (ntemp[3] == 0) && (ntemp[4] == 0) && (ntemp[5] <= 0))//表示停止命令
								{
									LogNormal("停止命令，不处理!");
								}
								else if((ntemp[2] == 0) &&(ntemp[3] == 0x02))//
								{
									LogNormal("往右命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0) &&(ntemp[3] == 0x04))//
								{
									LogNormal("往左命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0) &&(ntemp[3] == 0x08))//
								{
									LogNormal("往下命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0) &&(ntemp[3] == 0x10))//
								{
									LogNormal("往上命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0) &&(ntemp[3] == 0x20))//
								{
									LogNormal("镜头拉近命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0) &&(ntemp[3] == 0x40))//
								{
									LogNormal("镜头拉远命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0) &&(ntemp[3] == 0x80))//
								{
									LogNormal("聚焦近命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0x10) &&(ntemp[3] == 0x0))//
								{
									LogNormal("聚焦远命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0x02) &&(ntemp[3] == 0))//
								{
									LogNormal("光圈减小命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0x04) &&(ntemp[3] == 0))//
								{
									LogNormal("光圈增大命令，停止检测!");
									g_nPreSetMode = 1;
									g_skpChannelCenter.PauseDetect(0);
								}
								else if((ntemp[2] == 0) && (ntemp[3] == 0x03))//
								{
									LogNormal("设置预置位命令，不处理!");
								}
								else if((ntemp[2] == 0) && (ntemp[3] == 0x05))//
								{
									LogNormal("清除预置位命令，不处理!");
								}
								else//其他命令
								{
									LogNormal("外部其他控制，不处理!");
								}
							}
						}
					}
					strMessage.erase(nBPos,7);
				}
			}
			else
			{
				strMessage.clear();
			}
		}

	}
}

//比较从串口服务器接收到的信号时间和检测器发送命令的时间差
bool CMvsToSerialServer::compareMsgTime()
{
	UINT32 uTime = GetTimeStamp();
	LogNormal("RecvSerialMsg uTime=%lld,SendCmdTime=%lld\n",uTime,g_VisNetCommunication.m_uSendCmdTime);
	if ( (fabs(uTime - g_VisNetCommunication.m_uSendCmdTime)>=5))
	{
		m_uRecvserialmsgtime = uTime;
		return true;
	}

	return false;
}

//比较从串口服务器接收到的信号时间和当前时间差
bool CMvsToSerialServer::compareTime()
{
	UINT32 uTime = GetTimeStamp();
	if ( (fabs(uTime-m_uRecvserialmsgtime)>=300)&&(m_uRecvserialmsgtime > 0) )
	{
		m_uRecvserialmsgtime = 0;
		return true;
	}
	return false;
}
