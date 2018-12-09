// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
*   文件：VisNetCommunication.cpp
*   功能：VIS 网口通讯类
*   作者：於锋
*   时间：2010-9-4
**/
#include "Common.h"
#include "VisNetCommunication.h"
//#include "MVSToSerialServer.h"

CVisNetCommunication g_VisNetCommunication;
/*
* 函数介绍：接收消息线程入口
* 输入参数：pArg-线程入口参数，接收套接字
* 输出参数：无
* 返回值 ：无
*/

void *RecvVISMCIPMsgThread(void *pArg)
{
    while (!g_bEndThread)
    {
        g_VisNetCommunication.mvRecvCenterServerMsg();

        usleep(100);
    }

    LogError("接收VIS消息线程退出\r\n");

    pthread_exit((void *)0);
}


CVisNetCommunication::CVisNetCommunication()
{
    m_uRecvCSMsgThreadId = 0;
    m_bCenterLink = false;
    m_nCenterSocket = 0;
    m_nCSLinkCount = 0;
	m_uSendCmdTime = 0;
    pthread_mutex_init(&m_mutexMsg, NULL);
}


CVisNetCommunication::~CVisNetCommunication()
{
    pthread_mutex_destroy(&m_mutexMsg);
}
/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void CVisNetCommunication::mvConnOrLinkTest()
{
    if (!g_bEndThread)
    {
        if (!m_bCenterLink)
        {
                if (mvConnCSAndRecvMsg())
                {
                    LogNormal("连接VIS服务器成功!\n");
                    m_bCenterLink = true;
                    m_nCSLinkCount = 0;
                }
                else
                {
                    printf("连接VIS服务器失败!\r\n");
                }
        }
    }
}

/*
* 函数介绍：连接到中心并开启接收消息线程
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CVisNetCommunication::mvConnCSAndRecvMsg()
{
    //connect to center server;
    if (!mvConnectToCS())
    {
        return false;
    }

    //start to receive cs's msgs;
    if (!mvStartRecvThread())
    {
        return false;
    }

    return true;
}

/*
* 函数介绍：连接到中心
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CVisNetCommunication::mvConnectToCS()
{
    //connect to center server and set socket's option.
    if (g_strVisHost.empty() || g_strVisHost == "0.0.0.0" || g_nVisPort <= 0)
    {
        //printf("\n中心数据服务器连接参数异常:host=%s,port=%d\n", g_strVisHost.c_str(), g_nVisPort);
        return false;
    }

    if (!mvPrepareSocket(m_nCenterSocket))
    {
        //printf("\n准备连接中心数据服务器套接字失败!\n");
        return false;
    }

    if (!mvWaitConnect(m_nCenterSocket, g_strVisHost, g_nVisPort,2))
    {
        //printf("\n尝试连接中心数据服务器失败!\n");
        return false;
    }

    return true;
}

/*
* 函数介绍：接收消息线程入口
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CVisNetCommunication::mvStartRecvThread()
{
    pthread_attr_t   attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    mvJoinThread(m_uRecvCSMsgThreadId);
    if (pthread_create(&m_uRecvCSMsgThreadId, &attr, RecvVISMCIPMsgThread, NULL) != 0)
    {
        pthread_attr_destroy(&attr);
        return false;
    }

    pthread_attr_destroy(&attr);
    return true;
}

//接收中心端消息
bool CVisNetCommunication::mvRecvCenterServerMsg()
{
        string strMsg("");
        //receive msg and push it into the msg queue.
        if (mvRecvMsg(m_nCenterSocket, strMsg))
        {
            mvPushOneMsg(strMsg);
            return true;
        }
        else
        {
            return false;
        }
}

/*
* 函数介绍：压入一条消息
* 输入参数：strMsg-要压入的消息
* 输出参数：无
* 返回值 ：无
*/
void CVisNetCommunication::mvPushOneMsg(string strMsg)
{
    if (strMsg.size()<0)
    {
        return ;
    }
}

/*
* 函数介绍：弹出一条消息
* 输入参数：strCode-要弹出的消息类型变量；strMsg-要弹出的消息变量
* 输出参数：strCode-弹出的消息类型；strMsg-弹出的消息
* 返回值 ：成功返回true，否则返回false
*/
bool CVisNetCommunication::mvPopOneMsg(string &strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }

    return (!strMsg.empty());
}

/*
* 函数介绍：处理一条消息
* 输入参数：pCode-要处理的消息类型；strMsg-要处理的消息
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CVisNetCommunication::mvOnDealOneMsg(const string &strMsg)
{
    //deal one message.
    return true;
}

/*
* 函数介绍：接收消息
* 输入参数：nSocket-要接收消息的套接字；strMsg-将接收到的消息内容
* 输出参数：strMsg-接收到的消息内容
* 返回值 ：成功返回true，否则false
*/
bool CVisNetCommunication::mvRecvMsg(int nSocket, string& strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }
    if (nSocket <= 0)
    {
        return false;
    }

    char chBuffer[SRIP_MAX_BUFFER]={0};

    int nBytes = 0;
    int nLeft = SRIP_MAX_BUFFER;

    //printf("before nBytes=%d,receiving strMsg=%s,strMsg.size()=%d\n",nBytes,strMsg.c_str(),strMsg.size());
    nBytes = recv(nSocket, chBuffer, nLeft, MSG_NOSIGNAL);
    //printf("after nBytes=%d,receiving strMsg=%s,strMsg.size()=%d\n",nBytes,strMsg.c_str(),strMsg.size());
    if(nBytes < 0)
    {
        return false;
    }

    if(nBytes > 0)
    strMsg.append(chBuffer,nBytes);
   // printf("CVisNetCommunication::mvRecvMsg nBytes=%d,receiving strMsg.size()=%d\n",nBytes,strMsg.size());

    /*FILE *pEvent = fopen("RecvCmdfromVis.txt", "a+");
            //需要应用端添加相机放大的代码
            if (pEvent != NULL)
            {
                fprintf(pEvent, "=============strMsg=%s \n\n",strMsg.c_str());
                fflush(pEvent);
            }
            fclose(pEvent);*/

    return (!strMsg.empty());
}

//发送命令到vis
bool CVisNetCommunication::WriteCmdToVis(std::string& sCmdMsg)
{
    if(!m_bCenterLink)
    {
        return false;
    }


    if (!mvSendMsgToSocket(m_nCenterSocket, sCmdMsg))
    {
        mvCloseSocket(m_nCenterSocket);
        m_bCenterLink = false;
        LogError("发送消息失败，连接断开\n");
        return false;
    }

	m_uSendCmdTime = GetTimeStamp();
      /*      FILE *pEvent = fopen("Net_WriteCmdToVis.txt", "a+");
            //需要应用端添加相机放大的代码
            if (pEvent != NULL)
            {
                fprintf(pEvent, "=============sCmdMsg=%s \n\n",sCmdMsg.c_str());
                fflush(pEvent);
            }
            fclose(pEvent);*/

    return true;
}

/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CVisNetCommunication::mvSendMsgToVIS(CAMERA_CONFIG& cfg, int nMonitorID)
{
    if(!m_bCenterLink)
    {
        return false;
    }

    return mvRebMsgAndSend(m_nCenterSocket,cfg,nMonitorID);
}

/*
* 函数介绍：重组消息并发送
* 输入参数：pCode-要发送的消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CVisNetCommunication::mvRebMsgAndSend(int& nSocket,CAMERA_CONFIG& cfg, int nMonitorID)
{
    //rebuild the message according to the msg-code and then send it to the center server.
    string strFullMsg("");

    strFullMsg = GetControlCode(cfg,nMonitorID);

    //printf("GetControlCode=%s\n",strFullMsg.c_str());
    if (!mvSendMsgToSocket(nSocket, strFullMsg))
    {
        mvCloseSocket(nSocket);
        m_bCenterLink = false;
        LogError("发送消息失败，连接断开\n");
        return false;
    }
    else
    {
       LogNormal("发送消息成功＝%s\n",strFullMsg.c_str());
    }

    return true;
}

//获取控制字符串码
std::string CVisNetCommunication::GetControlCode(CAMERA_CONFIG& cfg,int nMonitorID)
{
    printf(" CVisNetCommunication::GetControlCode\n");
    char chPTZ ='\0';

    if(cfg.nIndex == LEFT_DIRECTION)
    {
        chPTZ = 'L';
    }
    else if(cfg.nIndex == RIGHT_DIRECTION)
    {
        chPTZ = 'R';
    }
    else if(cfg.nIndex == UP_DIRECTION)
    {
        chPTZ = 'U';
    }
    else if(cfg.nIndex == DOWN_DIRECTION)
    {
        chPTZ = 'D';
    }
    else if(cfg.nIndex == FOCUS_NEAR)
    {
        chPTZ = 'N';
    }
    else if(cfg.nIndex == FOCUS_FAR)
    {
        chPTZ = 'F';
    }
    else if(cfg.nIndex == ZOOM_NEAR)
    {
        chPTZ = 'T';
    }
    else if(cfg.nIndex == ZOOM_FAR)
    {
        chPTZ = 'W';
    }
    else if(cfg.nIndex == SET_PRESET)
    {
        chPTZ = 'S';
    }
    else if(cfg.nIndex == GOTO_PRESET)
    {
        chPTZ = 'V';
    }
    else if(cfg.nIndex == CLEAR_PRESET)
    {
        chPTZ = '~';
    }

    char buf[128]={0};
    //如果只是切换相机
    if(cfg.nIndex == SWITCH_CAMERA)
    {
        sprintf(buf,"#M==%d,C==%d\r",nMonitorID,cfg.nAddress);
    }
    else
    {
        printf("nMonitorID=%d,cfg.nAddress=%d,cfg.nAddress=%d,chPTZ=%c,(int)cfg.fValue=%d\n",nMonitorID,cfg.nAddress,cfg.nAddress,chPTZ,(int)cfg.fValue);
        //sprintf(buf,"#M==%d,C==%d\r#C==%d,%c==%d\r",nMonitorID,cfg.nAddress,cfg.nAddress,chPTZ,(int)cfg.fValue);
        sprintf(buf,"#C==%d,%c==5\r",cfg.nAddress,chPTZ);
    }

    string strControlCode(buf);

    //printf("buf=%s",buf);

    return strControlCode;
}
