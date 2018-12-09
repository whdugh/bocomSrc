// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include <assert.h>
#include "Common.h"
#include "CommonHeader.h"
#include "NESocket.h"


/*************************************************************/
/** 与中心服务端通信类                                       **/
/*************************************************************/

NESocket g_skpNeServer;
bool g_bSendStartInfo = false; //zhangyaoyao 记录是否发送启动状态，避免断开连接重连时误发
#define OUR_PROGRAM_RUN_DIR "/home/road/server/RoadDetect.out"

/**
 * 中心网管连接接收线程
 */
 void* NEThreadRecv(void* pArg)
{
    int nClient = *(int *)pArg;
    sNeMsgHead header;
    int nRecv = 0;
    while (!g_bEndThread)
    {
        nRecv = recv(nClient, (void *)&header, sizeof(sNeMsgHead), MSG_NOSIGNAL);
        if (nRecv < 0)
        {
            LogError("接收中心网管连接[%d]协议头出错，断开连接！\n\r", nClient);
            if(!g_bEndThread)
            g_skpNeServer.DelNeClient(nClient);

            return pArg;
        }
        else if (nRecv == 0)
        {
            if (errno == EBADFD)//连接可能已经断开
            {
                LogNormal("中心网管连接[%d]主动断开", nClient);
                return pArg;
            }
            usleep(10000);
            continue;
        }
        else if (nRecv != sizeof(sNeMsgHead))
        {
            LogWarning("接收中心网管连接[%d]协议头数据不完整，丢弃！", nClient);
            usleep(10000);
            continue;
        }
        g_skpNeServer.ParseNeMsg(nClient, header);
    }

    return pArg;
}


/**
 * 中心端连接请求接收线程
 */
 void* ThreadNEAccept(void *pArg)
{
    int nSocket =*(int*)pArg;
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
				return pArg;
			}
			//自动重启
			continue;
		}

		g_skpNeServer.SendStartInfoToNe(nClient);

		//输出用户连接
		LogNormal("中心端连接[IP:%s][端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));

		NeClient client;
		client.nSocket = nClient;
		client.nPort = ntohs(clientaddr.sin_port);
		strcpy(client.strHost, inet_ntoa(clientaddr.sin_addr));

		//保存客户端,并启动接收数据线程
		g_skpNeServer.AddNeClient(client);

		usleep(1000*10);//等待10毫秒
	}
	return pArg;
}

//构造函数/析构函数/初始化函数
NESocket::NESocket()
{
    m_nAcceptSocket = 0;

    m_nPort = NE_PROGRAM_SERVER_PORT;

    pthread_mutex_init(&m_thread_mutex,NULL);
}

NESocket::~NESocket()
{
	//
	pthread_mutex_destroy(&m_thread_mutex);

}

/**
 * 初始化与网管的通信类
 */
bool NESocket::Init()
{
    //创建套接字
	if(mvCreateSocket(m_nAcceptSocket,1)==false)
	{
	    LogError("创建套接字失败,服务无法启动!\r\n");
		return false;
	}

    //重复使用套接字
	if(mvSetSocketOpt(m_nAcceptSocket,SO_REUSEADDR)==false)
	{
	    LogError("设置重复使用套接字失败,服务无法启动!\r\n");
		return false;
	}

   	//绑定服务端口
	if(mvBindPort(m_nAcceptSocket,m_nPort)==false)
	{
        LogError("绑定到 %d 端口失败,服务无法启动!\r\n",m_nPort);
		return false;
	}

	//开始监听
	if (mvStartListen(m_nAcceptSocket) == false)
	{
		LogError("监听连接失败，服务无法启动!\r\n");
		return false;
	}

	//启动接收连接线程
	pthread_t id;           //线程id
	pthread_attr_t   attr;  //线程属性


	pthread_attr_init(&attr);//初始化
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);//分离线程

	//启动线程
	if(pthread_create(&id,&attr, ThreadNEAccept, (void*)&m_nAcceptSocket)!=0)
	{
		LogError("创建网管连接接收线程失败,服务无法启动!\r\n");
		return false;
	}
	pthread_attr_destroy(&attr);

	return true;
}

//释放
bool NESocket::UnInit()
{
    DelNeClient(0);
    //关闭连接
	mvCloseSocket(m_nAcceptSocket);

	return true;
}

/**
 * 添加一中心端连接
 */
void NESocket::AddNeClient(const NeClient &client)
{
    pthread_mutex_lock(&m_thread_mutex);
    m_NeMapClients.insert(NeMap::value_type(client.nSocket, client));
    pthread_mutex_unlock(&m_thread_mutex);

    pthread_t tid;          //接收线程号
    pthread_attr_t tattr;   //接收线程属性
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&tid, &tattr, NEThreadRecv, (void *)&client.nSocket) < 0)
    {
            LogWarning("为中心网管连接[%d]创建连接接收线程失败！\r\n", client.nSocket);
    }
    pthread_attr_destroy(&tattr);
}

//删除中心网管连接
bool NESocket::DelNeClient(int nSocket)
{
    pthread_mutex_lock(&m_thread_mutex);
    if(nSocket == 0)
    {
        NeMap::iterator it = m_NeMapClients.begin();
		while(it != m_NeMapClients.end())
		{
			int nSocket = it->first;

			//关闭连接
			mvCloseSocket(nSocket);

			it++;
		}
		m_NeMapClients.clear();
    }
    else
    {
        NeMap::iterator it = m_NeMapClients.find(nSocket);
        if(it != m_NeMapClients.end())
		{
		    //关闭连接
			mvCloseSocket(nSocket);

			//删除连接
			m_NeMapClients.erase(it);
		}
    }
    pthread_mutex_unlock(&m_thread_mutex);
    return true;
}

//向中心网管的所有连接发送数据
void NESocket::SendAlarmInfo(const std::string& strMsg)
{
        int nSent,nLeft,nTotal,nClient;

        std::string strMsgHeader;
        strMsgHeader.append(strMsg.c_str(),sizeof(sNeMsgHead));

        std::string strAlarmInfo;
        strAlarmInfo.append(strMsg.c_str()+sizeof(sNeMsgHead),sizeof(sNeIdeNoteDeviceAlarm));

        pthread_mutex_lock(&m_thread_mutex);
        NeMap::iterator it = m_NeMapClients.begin();
		while(it != m_NeMapClients.end())
		{
		    nClient = it->first;

		    mvSendMsgToSocket(nClient,strMsgHeader);

		    mvSendMsgToSocket(nClient,strAlarmInfo);


            it++;
		}
		pthread_mutex_unlock(&m_thread_mutex);
}

//zhangyaoyao:告诉网管业务程序的重启信息
void NESocket::SendStartInfoToNe(int nNeSocket)
{
    if (g_bSendStartInfo == false)
    {
        sNeMsgHead msgHeader;
        memset(&msgHeader, 0, sizeof(msgHeader));
        msgHeader.m_szSyncHeader[0] = '$';
        msgHeader.m_szSyncHeader[1] = 'h';
        msgHeader.m_szSyncHeader[2] = 'e';
        msgHeader.m_szSyncHeader[3] = 'a';
        msgHeader.m_szSyncHeader[4] = 'd';
        msgHeader.m_szSyncHeader[5] = '$';
        msgHeader.m_szVersion[0] = (char)0x01;
        msgHeader.m_szVersion[1] = (char)0x01;
        msgHeader.m_szVersion[2] = (char)0x01;
        msgHeader.m_szVersion[3] = (char)0x02;
        msgHeader.m_uiMsgType = NE_NOTE_COMMON_PROGRAM_STATUS;
        msgHeader.uiPacketBodyLen = sizeof(sNeNoteProgramStatus);
        sNeNoteProgramStatus prgStat;
        strncpy(prgStat.szServerName, "RoadDetect.out", sizeof("RoadDetect.out"));
        strncpy(prgStat.szProgramName, "RoadDetect.out", sizeof("RoadDetect.out"));
        prgStat.uiBusinessType = NE_SVR_MODE_IDE;

        if (IsStartForUpdate(OUR_PROGRAM_RUN_DIR))
        {
            prgStat.uiProgramStatus = NE_PROGRAM_RESTART;
            prgStat.uiRestartReason = NE_RESTART_PROGRAM_UPDATE;
            //prgStat.bSelfRestart = 1;
        }
        else
        {
            prgStat.uiProgramStatus = NE_PROGRAM_START;
            prgStat.uiRestartReason = NE_RESTART_RUN_ERROR;
            //prgStat.bSelfRestart = 1;
        }

        prgStat.bSelfRestart = 1;
        prgStat.uiRunTime = 0;
        prgStat.uiTimeStamp = (UINT32)time(NULL);

        string strStartInfo("");
        strStartInfo.append((char*)&msgHeader, sizeof(sNeMsgHead));
        strStartInfo.append((char*)&prgStat, sizeof(sNeNoteProgramStatus));

        if (mvSendMsgToSocket(nNeSocket, strStartInfo))
        {
            g_bSendStartInfo = true;
        }
    }
}

//zhangyaoyao:判断业务程序重启是否是因为更新
bool NESocket::IsStartForUpdate(const char* pFilePath)
{
    struct stat sBuff;
    if (stat(pFilePath, &sBuff) < 0)
    {
        printf("获取文件状态信息错误!\n");
        return false;
    }
    if (time(NULL)-sBuff.st_mtime <= 60)
        return true;
    return false;
}


/**
 * 与中心网管通讯的心跳检测
 * 说明：智能检测端从不主动发送心跳检测请求，只会等待网管心跳检测请求，然后返回心跳响应包
 */
void NESocket::SendHeartBitPackage(int nClient, UINT64 workflowId)
{
    assert(workflowId);

    sNeMsgHead msgHeader;
 //   int nSent = 0, nLeft = sizeof(sNeMsgHead), nTotal = 0;

    memset(&msgHeader, 0, sizeof(msgHeader));
    msgHeader.m_szSyncHeader[0] = '$';
    msgHeader.m_szSyncHeader[1] = 'h';
    msgHeader.m_szSyncHeader[2] = 'e';
    msgHeader.m_szSyncHeader[3] = 'a';
    msgHeader.m_szSyncHeader[4] = 'd';
    msgHeader.m_szSyncHeader[5] = '$';
    msgHeader.m_szVersion[0] = (char)0x01;
    msgHeader.m_szVersion[1] = (char)0x01;
    msgHeader.m_szVersion[2] = (char)0x01;
    msgHeader.m_szVersion[3] = (char)0x02;
    msgHeader.m_uiMsgType = NE_RET_NOTE_HEARTBIT;
    msgHeader.m_ubiWorkflowId = workflowId;

    printf("Before send heart bit info=========================================\n");
    std::string strMsg;
    strMsg.append((char*)&msgHeader,sizeof(sNeMsgHead));

    mvSendMsgToSocket(nClient,strMsg);
 /*   while (nLeft > 0)
    {
        nSent = send(nClient, &msgHeader, sizeof(sNeMsgHead), 0);
        if (nSent < 0)
        {
            LogError("向网管连接[%d]发送心跳检测失败，自动断开连接!\n", nClient);
            close(nClient);
            return;
        }
        nTotal += nSent;
        nLeft -= nSent;
    }*/
    printf("After send heart bit info===========================================\n");
}

/**
 * 与中心网管通信的相机状态查询响应
 * nClient:     网管连接socket
 * workflowId:  本次会话编号
 * nCameraID:   请求查询的相机编号
 */
void NESocket::SendCameraStatus(int nClient, UINT64 workflowId, int nCameraID)
{

    printf("SendCamerStatus: \n");
    printf("Client: %d, Session: %Lu, CameraID: %d\n", nClient, workflowId, nCameraID);


    //与中心网管通讯协议头
    sNeMsgHead msgHeader;
    memset(&msgHeader, 0, sizeof(msgHeader));
    msgHeader.m_szSyncHeader[0] = '$';
    msgHeader.m_szSyncHeader[1] = 'h';
    msgHeader.m_szSyncHeader[2] = 'e';
    msgHeader.m_szSyncHeader[3] = 'a';
    msgHeader.m_szSyncHeader[4] = 'd';
    msgHeader.m_szSyncHeader[5] = '$';
    msgHeader.m_szVersion[0] = (char)0x01;
    msgHeader.m_szVersion[1] = (char)0x01;
    msgHeader.m_szVersion[2] = (char)0x01;
    msgHeader.m_szVersion[3] = (char)0x02;
    msgHeader.m_uiMsgType = NE_RET_IDE_CAMERA_STATUS;
    msgHeader.m_ubiWorkflowId = workflowId;

    //相机状态查询返回
    sNeIdeRetCameraStatus retCmStatus;
    retCmStatus.uiCount = 1;
    retCmStatus.sRetBase.iRetValue = 0;
    strcpy(retCmStatus.sRetBase.sDescription, "相机状态查询返回");
    memset(retCmStatus.sRetBase.sReserved, 0, sizeof(retCmStatus.sRetBase.sReserved));

    //获取通道信息
    SRIP_CHANNEL sChannel;//通道信息(相机状态)
    g_skpDB.GetChannelInfoFromDB(nCameraID, sChannel);
    CameraState state = (CameraState)sChannel.nCameraStatus;

    printf("Read camera[%d] state[%d] from db\n", nCameraID, (int)state);

    //摄像机状态信息
    sNeIdeCameraStatus cmStatus;
    memset(&cmStatus, 0, sizeof(cmStatus));
    cmStatus.uiCameraId = sChannel.nCameraId;
    snprintf(cmStatus.szCameraName, sizeof(cmStatus.szCameraName), "Digital video camera: %d:%d",
        sChannel.nCameraType, sChannel.nCameraId);
    cmStatus.uiSideId = sChannel.nPannelID;
    strncpy(cmStatus.szInstallPlace, sChannel.chPlace, sizeof(cmStatus.szInstallPlace));
    if (CHANNEL_FIXUP == sChannel.eType)
        cmStatus.uiInstallType = NE_CAMERA_INSTALL_FIXED;
    else
        cmStatus.uiInstallType = NE_CAMERA_INSTALL_ACTIVITY;
    switch (sChannel.uDirection)
    {
        case EAST_TO_WEST:
            cmStatus.uiInstallDirect = NE_CAMERA_DIRECT_EAST;
            break;
        case WEST_TO_EAST:
            cmStatus.uiInstallDirect = NE_CAMERA_DIRECT_WEST;
            break;
        case SOUTH_TO_NORTH:
            cmStatus.uiInstallDirect = NE_CAMERA_DIRECT_SOUTH;
            break;
        case NORTH_TO_SOUTH:
            cmStatus.uiInstallDirect = NE_CAMERA_DIRECT_NORTH;
            break;
        default:
            cmStatus.uiInstallDirect = NE_CAMERA_DIRECT_NORTH;
    }

    if (state == CM_OK || state == CM_SHAKED)//TODO:如果相机状态OK或者视频有震动，则认为相机可用
        cmStatus.uiEnable = 1;
    else
        cmStatus.uiEnable = 0;
    cmStatus.uiCtrlable = 1;            //TODO:当前实现相机不可由中心端控制，但可由客户端控制

    if (sChannel.bRun)
        cmStatus.uiStatus = NE_CAMERASTATUS_LIVE;
    else
        cmStatus.uiStatus = NE_CAMERASTATUS_PTZ;
    if (sChannel.bEventCapture)
        cmStatus.uiStatus = (eNeCameraStatus)(cmStatus.uiStatus | NE_CAMERASTATUS_RECORD);
    strcpy(cmStatus.szUserName, "system");//TODO::get operating user name?

    msgHeader.uiPacketBodyLen = sizeof(retCmStatus) + sizeof(cmStatus);

    printf("Before Send State ===================================================\n");
    //需要分开发送
    std::string strMsgHeader;
    strMsgHeader.append((char*)&msgHeader,sizeof(sNeMsgHead));
    mvSendMsgToSocket(nClient,strMsgHeader);

    std::string strMsgRetCmStatus;
    strMsgRetCmStatus.append((char*)&retCmStatus,sizeof(sNeIdeRetCameraStatus));
    mvSendMsgToSocket(nClient,strMsgRetCmStatus);

    std::string strMsgCmStatus;
    strMsgCmStatus.append((char*)&cmStatus,sizeof(sNeIdeCameraStatus));
    mvSendMsgToSocket(nClient,strMsgCmStatus);

/*  send(nClient, &msgHeader, sizeof(msgHeader), 0);
    send(nClient, &retCmStatus, sizeof(retCmStatus), 0);
    send(nClient, &cmStatus, sizeof(cmStatus), 0);*/
    printf("After Send State ====================================================\n");
}

/**
 * 与中心网管通讯的协议解析
 * nClient：     中心网管连接socket
 * msgHeader：   要解析的协议包
 */
void NESocket::ParseNeMsg(int nClient, const sNeMsgHead& msgHeader)
{
    assert(nClient);
    switch(msgHeader.m_uiMsgType)
    {
        case NE_REQ_NOTE_HEARTBIT://心跳测试
             g_skpNeServer.SendHeartBitPackage(nClient, msgHeader.m_ubiWorkflowId);
            return;
        case NE_REQ_IDE_CAMERA_STATUS://相机状态查询
            sNeIdeReqCameraStatus reqCmStatus;
            if (recv(nClient, &reqCmStatus, sizeof(sNeIdeReqCameraStatus), MSG_NOSIGNAL)
                != sizeof(sNeIdeReqCameraStatus))
            {
                LogError("从中心网管连接[%d]读取相机查询请求信息失败，主动断开连接！", nClient);
                DelNeClient(nClient);
                return;
            }
             g_skpNeServer.SendCameraStatus(nClient, msgHeader.m_ubiWorkflowId, reqCmStatus.szCameraId);
            return;
        default://其它类型消息，智能检测不作处理
            return;
    }
}
