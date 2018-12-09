// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
 *   文件：BJServer.h
 *   功能：北京图侦通讯类
 *   作者：YuFeng
 *   时间：2012-01-30
**/

#include "Common.h"
#include "CommonHeader.h"
#include "BJServer.h"
#include "RoadImcData.h"
#include "XmlParaUtil.h"

CBJServer g_BJServer;

//监听线程
void* ThreadBJAccept(void* pArg)
{
	int nSocket = *(int*)pArg;
	//中心端连接
	struct sockaddr_in clientaddr;
	//长度
	socklen_t sin_size = sizeof(struct sockaddr_in);
	int nClient;
	while(!g_bEndThread)
	{
		//接受连接
		if((nClient = accept(nSocket,(struct sockaddr*)&clientaddr,&sin_size)) ==  -1)
		{
			//断开连接
			if(g_bEndThread)
			{
                pthread_exit((void *)0);
				return pArg;
			}
			continue;
		}

		//输出用户连接
		LogNormal("中心端连接[IP:%s][nClient = %d,端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));
cerr<<"中心端连接IP="<<inet_ntoa(clientaddr.sin_addr)<<endl;

		SRIP_CLEINT sClient;
		sClient.nSocket = nClient;
		sClient.nPort = ntohs(clientaddr.sin_port);

		strcpy(sClient.chHost,inet_ntoa(clientaddr.sin_addr));

		//保存客户端,并启动接收数据线程
		g_BJServer.AddClient(sClient);

		//10毫秒
		usleep(1000*10);
	}
	pthread_exit((void *)0);
}

//接收线程
void* ThreadBJRecv(void* pArg)
{
    g_BJServer.mvRecvCenterServerMsg();

    LogError("接收中心端消息线程退出\r\n");
    pthread_exit((void *)0);
}

//心跳检测线程
void* ThreadBJLink(void* pArg)
{
	while(!g_bEndThread)
	{
		//检测状态一次
		g_BJServer.LinkTest();

		//5秒检测一次
		sleep(5);
	}
	pthread_exit((void *)0);
}

////H264发送线程
//void* ThreadSendH264(void* pArg)
//{
//	g_BJServer.SendH264Frame();
//	pthread_exit((void *)0);
//}

//Event发送线程
void* ThreadSendEvent(void* pArg)
{
	g_BJServer.SendEvent();
	pthread_exit((void *)0);
}

//构造
CBJServer::CBJServer()
{
	pthread_mutex_init(&m_Event_Mutex, NULL);
	pthread_mutex_init(&m_thread_mutex, NULL);

	m_nEventThreadId = 0;
	m_nAcceptSocket = 0;
	m_nCenterSocket = 0;
	m_bVerify = false;
}

//析构
CBJServer::~CBJServer()
{
	pthread_mutex_destroy(&m_Event_Mutex);
	pthread_mutex_destroy(&m_thread_mutex);
}

//启动侦听服务
bool CBJServer::Init()
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

	//绑定服务端口
	if(mvBindPort(m_nAcceptSocket,BJSERVER_PORT)==false)
	{
        LogError("绑定到 %d 端口失败,服务无法启动!\r\n",BJSERVER_PORT);
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
	pthread_attr_t attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动监听线程
	int nret=pthread_create(&id,&attr,ThreadBJAccept,(void*)&m_nAcceptSocket);
	if(nret!=0)
	{
		//失败
		LogError("创建监听线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动接收消息线程
	nret=pthread_create(&id,&attr,ThreadBJRecv,this);
	if(nret!=0)
	{
		//失败
		LogError("创建接收消息线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动心跳检测线程
	nret=pthread_create(&id,&attr,ThreadBJLink,NULL);
	if(nret!=0)
	{
		//失败
		LogError("创建心跳检测失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	////启动H264视频流发送线程
	//nret=pthread_create(&m_nH264ThreadId,&attr,ThreadSendH264,NULL);
	//if(nret!=0)
	//{
	//	//失败
	//	LogError("创建H264视频流发送线程失败,服务无法启动!\r\n");
	//    g_bEndThread = true;
	//	return false;
	//}

	//启动检测结果发送线程
	nret=pthread_create(&m_nEventThreadId,&attr,ThreadSendEvent,NULL);
	if(nret!=0)
	{
		//失败
		LogError("创建Event发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);
	return true;
}

//心跳检测
void CBJServer::LinkTest()
{
	if(m_bVerify == false)
		return;

	if(m_nCenterSocket > 0)
	{
        //协议头
        MIMAX_HEADER header;
        header.uCmdID = LINK_HEART;
        //消息长度
        header.uCmdLen = sizeof(header);
        std::string msg;
        //数据
        msg.append((char*)&header,sizeof(header));

        //发送心跳信号
		pthread_mutex_lock(&m_thread_mutex);
		if(mvSendMsgToSocket(m_nCenterSocket, msg) == false)
		{
			//关闭连接
			DelClient();
			LogError("发送心跳失败\n");
			cerr<<"发送心跳失败"<<endl;
		}
		pthread_mutex_unlock(&m_thread_mutex);
	}
}

//接收中心端消息 
void CBJServer::mvRecvCenterServerMsg()
{
    while (!g_bEndThread)
    {
        if(m_nCenterSocket > 0)
        {
            int nBytes = 0;

            char chBuffer[SRIP_MAX_BUFFER];

            MIMAX_HEADER header;
            //接收HTTP 头，一次性接收
			cerr<<"wait recv!"<<endl;
            if((nBytes = recv(m_nCenterSocket,(void*)&header,sizeof(header),MSG_NOSIGNAL)) < 0)
            {
				cerr<<"接收协议头出错，连接断开!"<<endl;
                LogError("接收协议头出错，连接断开! socket = %d,%s\r\n", m_nCenterSocket, strerror(errno));
                DelClient();
                continue;
            }
			cerr<<"00; nBytes="<<nBytes<<endl;

            if((nBytes == 0) && (errno == 0))
            {
				cerr<<"Socket 关闭"<<endl;
				DelClient();
                continue;
            }
			cerr<<"11"<<endl;

            //判断结构中的数据长度，小于本身结构的长度，错误
            if(header.uCmdLen < sizeof(header))
            {
                LogError("接收协议头结构错误，连接断开!,%s,nBytes=%d,mHeader.uCmdLen=%d,errno=%d\r\n",strerror(errno),nBytes,header.uCmdLen,errno);
                DelClient();
                continue;
            }
			cerr<<"22"<<endl;

			//保存报头
			string request;
			request.append((char*)&header, sizeof(header));

            //接收剩下的数据
            int nLeft = header.uCmdLen - sizeof(header);
			
            while(nLeft >  0)
            {
                //接收后面的数据
				cerr<<"wait recv2"<<endl;
                if((nBytes = recv(m_nCenterSocket,chBuffer,sizeof(chBuffer)<nLeft?sizeof(chBuffer):nLeft,MSG_NOSIGNAL)) < 0)
                {
					cerr<<"wait recv2 Error!"<<endl;
                    LogError("接收后续消息出错，连接断开!,%s\r\n",strerror(errno));
                    DelClient();
                    break;
                }
				if((nBytes == 0) && (errno == 0))
				{
					cerr<<"Socket2 关闭"<<endl;
					DelClient();
					break;
				}

                //保存数据
                request.append(chBuffer,nBytes);
                nLeft -= nBytes;
            }
			cerr<<"33"<<endl;

            //将命令传送到处理模块,继续处理下一个命令
            if(request.size() == header.uCmdLen)
				OnMsg(m_nCenterSocket, request);
        }
        usleep(1000);
    }
}

//处理一条命令
void CBJServer::OnMsg(const int nSocket, string request)
{
	
}

/*
//发送H264视频
void CBJServer::SendH264Frame()
{
	while(!g_bEndThread)
	{
		//取出msg
		string msg;
		PopH264Frame(msg);
		int cameraID = *((int*)msg.c_str());
		msg.erase(0, sizeof(cameraID));

		//向ClientMap中的匹配主机发送数据
		pthread_mutex_lock(&m_ClientMap_mutex);
		pair< iterator, iterator > pos;
		pos = m_h264ClientMap.equal_range(cameraID);
		for ( ; pos.first != pos.second; pos.first++ )
		{
			int socket = pos.first->second.nSocket;
			if(mvSendMsgToSocket(socket, msg) == false)
			{
				//发送失败如何操作??
				LogError("向%s发送H264视频流失败\n", pos.first->second.chHost);
			}
		}
		pthread_mutex_unlock(&m_ClientMap_mutex);
		usleep(10);
	}
}
*/

//发送Event
void CBJServer::SendEvent()
{
	while(!g_bEndThread)
	{
		//取出数据
		pthread_mutex_lock(&m_Event_Mutex);

		list<string>::iterator it = m_eventBuf.begin();
		if(it == m_eventBuf.end())
		{
			pthread_mutex_unlock(&m_Event_Mutex);
			usleep(1000);
			continue;
		}
		string msg = *it;
		m_eventBuf.pop_front();
		pthread_mutex_unlock(&m_Event_Mutex);

		//封装Event成xml格式
		string xmlstr;
		int cameraID = *( (int*)msg.c_str() );
		RECORD_EVENT* sEvent = (RECORD_EVENT*)(msg.c_str()+sizeof(int));
		CXmlParaUtil xmlUtil;
		xmlUtil.CreateEventToXml(cameraID, sEvent, xmlstr);

if(xmlstr.size()>300)
	cerr<<"SendXML="<<string(xmlstr.c_str(), 300)<<endl;
else
	cerr<<"SendXML="<<xmlstr.c_str()<<endl;
		//加协议头
		MIMAX_HEADER header;
		header.uCmdID = EVENT_ALARM;
		header.uCmdLen = sizeof(MIMAX_HEADER) + xmlstr.size();
		header.uCameraID = cameraID;
		header.uCmdFlag = 0;

		xmlstr.insert(0, (char*)&header, sizeof(MIMAX_HEADER));

		pthread_mutex_lock(&m_thread_mutex);
		if(mvSendMsgToSocket(m_nCenterSocket, xmlstr) == false)
		{
			cerr<<"Send Failed"<<endl;
			DelClient();
			LogError("向中心端发送Event失败\n");
		}
		pthread_mutex_unlock(&m_thread_mutex);
		usleep(1000);
	cerr<<"Send End"<<endl;
	}
}

//释放
bool CBJServer::UnInit()
{
    //停止Event发送线程
	if(m_nEventThreadId != 0)
	{
		pthread_join(m_nEventThreadId,NULL);
		m_nEventThreadId = 0;
	}

    //需要关闭和删除掉所有的客户端连接
    DelClient();
    //关闭连接
    mvCloseSocket(m_nAcceptSocket);

	return true;
}


//添加客户端
void CBJServer::AddClient(SRIP_CLEINT sClient)
{
    //加锁
	cerr<<"AddClient() in"<<endl;
    pthread_mutex_lock(&m_thread_mutex);

    DelClient();
    m_nCenterSocket = sClient.nSocket;

    //解锁
    pthread_mutex_unlock(&m_thread_mutex);
	cerr<<"AddClient() out"<<endl;
}

//删除客户端
void CBJServer::DelClient()
{
    if(m_nCenterSocket > 0)
    {
        //关闭连接
		m_bVerify = false;
        
		mvCloseSocket(m_nCenterSocket);
		cerr<<"DelClient:CloseSocket()"<<endl;
        m_nCenterSocket = 0;

		//清空数据
		pthread_mutex_lock(&m_Event_Mutex);
		m_eventBuf.clear();
		pthread_mutex_unlock(&m_Event_Mutex);
    }
}

/*
//向h264视频缓冲区加入数据
void CBJServer::AddH264Frame(string& frame, int cameraID)
{
	//匹配相机ID
	pthread_mutex_lock(&m_ClientMap_mutex);

	H264Client_Map::iterator iter = m_h264ClientMap.find(cameraID);
	if(iter == m_h264ClientMap.end())
	{
		pthread_mutex_unlock(&m_ClientMap_mutex);
		return;
	}
	pthread_mutex_unlock(&m_ClientMap_mutex);

	pthread_mutex_lock(&m_Frame_Mutex);
	//缓存区超过10帧就丢弃
	if(m_h264FrameBuf.size() > 10)
	{
		LogError("H264缓存区溢出\n");
		pthread_mutex_unlock(&m_Frame_Mutex);
		return;
	}
	string newFrame((char*)cameraID, sizeof(int));
	newFrame.append(frame);
	m_h264FrameBuf.push_back(newFrame);

	pthread_mutex_unlock(&m_Frame_Mutex);
}

//取出一帧图像
void CBJServer::PopH264Frame(string &frame)
{
	pthread_mutex_lock(&m_Frame_Mutex);
	list<string>::iterator it = m_h264FrameBuf.begin();
	frame = *it;
	m_h264FrameBuf.pop_front();
	pthread_mutex_unlock(&m_Frame_Mutex);
}
*/

//向Event缓冲区加入数据
void CBJServer::AddEvent(RECORD_EVENT& event, int cameraID)
{
/*
	//匹配相机ID
	pthread_mutex_lock(&m_EventVec_mutex);
	vector<int>::iterator it = m_eventCameraIdVec.find(cameraID);
	if(it == m_eventCameraIdVec.end())
	{
		pthread_mutex_unlock(&m_EventVec_mutex);
		return;
	}
	pthread_mutex_unlock(&m_EventVec_mutex);
*/
	if(m_bVerify == false)
		return;

	//保存Evnet到缓冲队列
	string eventStr((char*)&cameraID, sizeof(cameraID));
	eventStr.append((char*)&event, sizeof(RECORD_EVENT));

	pthread_mutex_lock(&m_Event_Mutex);
	m_eventBuf.push_front(eventStr);
	pthread_mutex_unlock(&m_Event_Mutex);
	LogNormal("BJ AddEvent success!\n");
}
