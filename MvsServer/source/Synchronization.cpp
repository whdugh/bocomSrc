// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

/**
    文件：Synchronization.cpp
    功能：相机同步功能具体实现
    作者：张要要
    时间：2009-11-27
**/

#include "Common.h"
#include "CommonHeader.h"
#include "cv.h"
#include "highgui.h"
#include "XmlParaUtil.h"
#include "Synchronization.h"

pthread_t     g_uDMThreadId = 0;
pthread_t     g_uRLMThreadId = 0;
pthread_t     g_uRRMThreadId = 0;
pthread_t     g_uACThreadId = 0;

int     g_nCameraType = 0;
int     g_nSynTimeGap = 1200;
float   g_fMatchAreaPercent = 0.4;
float   g_fSynGoodPercent = 0.5;
bool    g_bLeftLink = false;
bool    g_bRightLink = false;
mvCSynchronization g_SynProcess;

#define MAX_TIME_IN_QUEUE 5 //记录在队列中存放的最长时间
#define MAC_ADDR_LENGTH 12 //MAC地址的长度
//#define OUR_TIME_GAP 1200 //匹配目标允许的时间差
//#define MATCH_AREA_PERCENT 0.5 //重合面积占原面积的比
//#define OUR_GOOD_PERCENT 0.7 //匹配率
#define SYN_BUFFER_SIZE 4096 //4k

#define SEND_MAC_ADDRESS     0x40000009  //发送mac地址
#define RE_SEND_MAC_ADDRESS     0x80000009  //重新发送mac地址
#define SEND_MARKER_GROUP     0x4000000A  //发送标定点群
#define RE_SEND_MARKER_GROUP     0x8000000A //重发标定点群
#define DEL_REPEATED_RECORD     0x4000000B  //删除重复记录
#define SEND_CHARACTER_DATA     0x4000000C  //发送特征数据

//#define PRINT_SYN_PROCESS_LOG //记录文件打印信息开关
#define PRINT_SYN_DEBUG_MSG //printf打印信息开关
#ifdef PRINT_SYN_PROCESS_LOG
FILE* fpLeftPushLog = NULL;
FILE* fpRightPushLog = NULL;
FILE* fpLeftPopLog = NULL;
FILE* fpRightPopLog = NULL;
FILE* fpLeftCmpLog = NULL;
FILE* fpRightCmpLog = NULL;
FILE* fpCmpDisLog = NULL;
#endif

#ifndef PRINT_SYN_DEBUG_MSG
int printf(const char* format, ...)
{
    return 0;
}
#endif

//#define SYN_STUPID_LOG

#ifdef SYN_STUPID_LOG
void SSLog(const char *pLog)
{
    FILE *pSSLog = fopen("syn_stupid.log", "a");
    if (pSSLog != NULL)
    {
        fprintf(pSSLog, pLog);
        fflush(pSSLog);
        fclose(pSSLog);
    }
}
#endif  //SYN_STUPID_LOG


/*
* 函数介绍：信号捕捉函数
* 输入参数：nSig-要捕捉的信号
* 输出参数：无
* 返回值 ：无
*/
void MySigHandle(int nSig)
{
#ifdef SYN_STUPID_LOG
    SSLog("In MySigHandle()\n");
#endif
    if (SIGPIPE == nSig)
    {
        LogError("\n####相机同步：接收到SIGPIPE信号\n");
        g_SynProcess.mvUnInit();
    }
}

/*
* 函数介绍：接收左侧套接字消息
* 输入参数：strRecvMsg-存储接收到的消息变量
* 输出参数：strRecvMsg-成功时存储接收到的消息
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSynchronization::mvRecvLeftSocketMsg(string& strRecvMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvRecvLeftSocketMsg()\n");
#endif
    if (0 == m_nPassiveSocket)
    {
        printf("接收过程中，左连接套接字为0!\n");
        return false;
    }
    if (!g_bLeftLink)
    {
        printf("接收过程中，左连接断开!\n");
        return false;
    }
    if (!strRecvMsg.empty())
        strRecvMsg.clear();

    int nBytes = 0;
    SRIP_HEADER sHeader;
	char szBuffer[SYN_BUFFER_SIZE] = {0};

    if (m_nPassiveSocket != 0 && g_bLeftLink) //&& mvIsFdCanRead(m_nPassiveSocket))
    {
        nBytes = recv(m_nPassiveSocket, (void*)&sHeader, sizeof(SRIP_HEADER), MSG_NOSIGNAL);
        if (nBytes < 0 || nBytes != sizeof(SRIP_HEADER))
        {
            printf("\n####相机同步：接收左边消息头出错!Socket=%d,nBytes=%d\n", m_nPassiveSocket, nBytes);
            return false;
        }
        if (sHeader.uMsgLen < sizeof(SRIP_HEADER))
        {
            printf("\n####相机同步：接收左边消息头结构错误!Socket=%d,nBytes=%d\n", m_nPassiveSocket, nBytes);
            return false;
        }
        //sHeader.uMsgSource = m_nPassiveSocket;
        strRecvMsg.append((char*)&sHeader, sizeof(SRIP_HEADER));

        //printf("before recv msg body, sHeader.uMsgLen = %u, sizeof(sHeader)=%d\n", sHeader.uMsgLen, sizeof(sHeader));

        int nLeft = sHeader.uMsgLen - sizeof(SRIP_HEADER);
        while (nLeft > 0 && g_bLeftLink)
        {
            /*
            if (!mvIsFdCanRead(m_nPassiveSocket))
            {
                printf("接收过程中，左连接套接字不可读!\n");
                return false;
            }
            */
            if (0 == m_nPassiveSocket || !g_bLeftLink)
            {
                printf("接收过程中，左连接套接字为0，或左连接断开!\n");
                return false;
            }
            memset(szBuffer, 0, SYN_BUFFER_SIZE);
            if((nBytes = recv(m_nPassiveSocket, szBuffer, SYN_BUFFER_SIZE<nLeft?SYN_BUFFER_SIZE:nLeft, MSG_WAITALL)) < 0)
            {
                printf("\n####相机同步：接收左边后续消息出错!%s\n\n", strerror(errno));
                return false;
            }
            strRecvMsg.append(szBuffer, nBytes);
            nLeft -= nBytes;
        }
        //printf("after recv msg body, strRecvMsg.size()=%d\n", strRecvMsg.size());
        return (0 == nLeft);
    }
    printf("接收消息时，左连接套接字为0，或左连接断开，或左套接字不可读!\n");
    return false;
}

/*
* 函数介绍：接收右侧套接字消息
* 输入参数：strRecvMsg-存储接收到的消息变量
* 输出参数：strRecvMsg-成功时存储接收到的消息
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSynchronization::mvRecvRightSocketMsg(string& strRecvMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvRecvRightSocketMsg()\n");
#endif
    if (0 == m_nActiveSocket)
    {
        printf("接收过程中，右连接套接字为0!\n");
        return false;
    }
    if (!g_bRightLink)
    {
        printf("接收过程中，右连接断开!\n");
        return false;
    }
    if (!strRecvMsg.empty())
        strRecvMsg.clear();

    int nBytes = 0;
    SRIP_HEADER sHeader;
	char szBuffer[SYN_BUFFER_SIZE] = {0};

    if (m_nActiveSocket != 0 && g_bRightLink) //&& mvIsFdCanRead(m_nActiveSocket))
    {
        nBytes = recv(m_nActiveSocket, (void*)&sHeader, sizeof(SRIP_HEADER), MSG_NOSIGNAL);
        if (nBytes < 0 || nBytes != sizeof(SRIP_HEADER))
        {
            printf("\n####相机同步：接收右边消息头出错!Socket=%d,nBytes=%d\n", m_nActiveSocket, nBytes);
            return false;
        }
        if (sHeader.uMsgLen < sizeof(SRIP_HEADER))
        {
            printf("\n####相机同步：接收右边消息头结构错误!Socket=%d,nBytes=%d\n", m_nActiveSocket, nBytes);
            return false;
        }
        //sHeader.uMsgSource = m_nActiveSocket;
        strRecvMsg.append((char*)&sHeader, sizeof(SRIP_HEADER));

        //printf("before recv msg body, sHeader.uMsgLen = %u, sizeof(sHeader)=%d\n", sHeader.uMsgLen, sizeof(sHeader));

        int nLeft = sHeader.uMsgLen - sizeof(SRIP_HEADER);
        while (nLeft > 0 && g_bRightLink)
        {
            /*
            if (!mvIsFdCanRead(m_nActiveSocket))
            {
                printf("接收过程中，右连接套接字不可读!\n");
                return false;
            }
            */
            if (0 == m_nActiveSocket || !g_bRightLink)
            {
                printf("接收过程中，右连接套接字为0，或右连接断开!\n");
                return false;
            }
            memset(szBuffer, 0, SYN_BUFFER_SIZE);
            if((nBytes = recv(m_nActiveSocket, szBuffer, SYN_BUFFER_SIZE<nLeft?SYN_BUFFER_SIZE:nLeft, MSG_WAITALL)) < 0)
            {
                printf("\n####相机同步：接收右边后续消息出错!%s\n\n", strerror(errno));
                return false;
            }
            strRecvMsg.append(szBuffer, nBytes);
            nLeft -= nBytes;
        }
        //printf("after recv msg body, strRecvMsg.size()=%d\n", strRecvMsg.size());
        return (0 == nLeft);
    }
    printf("接收消息时，右连接套接字为0，或右连接断开，或右套接字不可读!\n");
    return false;
}

/*
* 函数介绍：接收左侧消息线程
* 输入参数：pArg-线程入口参数
* 输出参数：无
* 返回值 ：无
*/
void* RecvLeftMsgThread(void* pArg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In RecvLeftMsgThread()\n");
#endif
    printf("接收左边消息线程 g_uRLMThreadId=%u started...\n", (UINT_32)pthread_self());
    /*
	if (NULL == pArg)
	{
	    printf("\n####相机同步：线程接收参数为空，异常退出!\n\n");
	    g_uRLMThreadId = (pthread_t)0;
		pthread_exit((void*)-1);
	}
	mvCSynchronization *pSyn = (mvCSynchronization *)pArg;
    */
    string strRecvMsg("");
	while (!g_bEndThread)
	{
	    if (!g_bLeftLink)
	    {
	        printf("左边连接已断开，接收左消息线程即将退出!\n");
	        break;
	    }
	    //if (g_SynProcess.mvIsFdCanRead(g_SynProcess.m_nPassiveSocket))
	    //{	    }
        strRecvMsg.clear();
        if (!g_SynProcess.mvRecvLeftSocketMsg(strRecvMsg))
        {
            printf("\n####相机同步：接收左边消息出错，关闭左连接，且接收左消息线程退出!\n\n");
            //g_bLeftLink = false;
            g_SynProcess.mvMyClosePassiveSocket();
            g_uRLMThreadId = (pthread_t)0;
            pthread_exit((void*)0);
        }
        if (!strRecvMsg.empty())
        {
            //printf("before push, strRecvMsg.size() = %d\n", strRecvMsg.size());
            if (!g_SynProcess.mvPushOneLeftSynMsg(strRecvMsg))
            {
                printf("\n####相机同步：push左边消息入列出错!\n\n");
            }
        }
        usleep(50);
	}
	printf("\n####相机同步：接收左边消息线程退出!\n\n");
	g_uRLMThreadId = (pthread_t)0;
	pthread_exit((void*)0);
}

/*
* 函数介绍：接收右侧消息线程
* 输入参数：pArg-线程入口参数
* 输出参数：无
* 返回值 ：无
*/
void* RecvRightMsgThread(void* pArg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In RecvRightMsgThread()\n");
#endif
    printf("接收右边消息线程 g_uRRMThreadId=%u started...\n", (UINT_32)pthread_self());
    /*
	if (NULL == pArg)
	{
	    printf("\n####相机同步：线程接收参数为空，异常退出!\n\n");
	    g_uRRMThreadId = (pthread_t)0;
		pthread_exit((void*)-1);
	}
	mvCSynchronization *pSyn = (mvCSynchronization *)pArg;
    */
    string strRecvMsg("");
	while (!g_bEndThread)
	{
	    if (!g_bRightLink)
	    {
	        printf("右边连接已断开，接收右消息线程即将退出!\n");
	        break;
	    }
        //if (g_SynProcess.mvIsFdCanRead(g_SynProcess.m_nActiveSocket))
        //{        }
        strRecvMsg.clear();
        if (!g_SynProcess.mvRecvRightSocketMsg(strRecvMsg))
        {
            printf("\n####相机同步：接收右边消息出错，关闭右连接，且接收右消息线程退出!\n\n");
            //g_bRightLink = false;
            g_SynProcess.mvMyCloseActiveSocket();
            g_uRRMThreadId = (pthread_t)0;
            pthread_exit((void*)0);
        }
        if (!strRecvMsg.empty())
        {
            //printf("before push, strRecvMsg.size() = %d\n", strRecvMsg.size());
            if (!g_SynProcess.mvPushOneRightSynMsg(strRecvMsg))
            {
                printf("\n####相机同步：push右边消息入列出错!\n\n");
            }
        }
        usleep(50);
	}
	printf("\n####相机同步：接收右边消息线程退出!\n\n");
	g_uRRMThreadId = (pthread_t)0;
	pthread_exit((void*)0);
}

/*
* 函数介绍：处理消息线程
* 输入参数：pArg-线程入口参数
* 输出参数：无
* 返回值 ：无
*/
void* DealMsgThread(void* pArg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In DealMsgThread()\n");
#endif
    printf("处理数据线程 g_uDMThreadId=%u started...\n", (UINT_32)pthread_self());
    /*
	if (NULL == pArg)
	{
	    printf("\n####相机同步：线程接收参数为空，异常退出!\n\n");
	    g_uDMThreadId = (pthread_t)0;
		pthread_exit((void*)-1);
	}
	mvCSynchronization *pSyn = (mvCSynchronization *)pArg;
    */
    string strLeftPopMsg("");
    string strRightPopMsg("");
    string strLeftRec("");
    string strRightRec("");
	while (!g_bEndThread)
	{
	    if (g_SynProcess.m_bRecvLeftMac && g_SynProcess.m_bRecvLeftMarker && g_SynProcess.m_bSendMarkerToLeft)
	    {
            if (!strLeftPopMsg.empty())
            {
                strLeftPopMsg.clear();
            }
            g_SynProcess.mvPopOneLeftSynMsg(strLeftPopMsg);
            if (!strLeftPopMsg.empty())
            {
                g_SynProcess.mvDealOneLeftSynMsg(strLeftPopMsg);
            }
/*
            if (!strLeftRec.empty())
            {
                strLeftRec.clear();
            }
            g_SynProcess.mvGetTheFirstLeftCharRecord(strLeftRec);
            if (!strLeftRec.empty())
            {
                g_SynProcess.mvSendOneCharacterData(g_SynProcess.m_nPassiveSocket, strLeftRec);
            }
*/
        }

        if (g_SynProcess.m_bSendMacToRight && g_SynProcess.m_bSendMarkerToRight && g_SynProcess.m_bRecvRightMarker)
        {
            if (!strRightPopMsg.empty())
            {
                strRightPopMsg.clear();
            }
            g_SynProcess.mvPopOneRightSynMsg(strRightPopMsg);
            if (!strRightPopMsg.empty())
            {
                g_SynProcess.mvDealOneRightSynMsg(strRightPopMsg);
            }
/*
            if (!strRightRec.empty())
            {
                strRightRec.clear();
            }
            g_SynProcess.mvGetTheFirstRightCharRecord(strRightRec);
            if (!strLeftRec.empty())
            {
                g_SynProcess.mvSendOneCharacterData(g_SynProcess.m_nActiveSocket, strRightRec);
            }
*/
        }

        usleep(50);
    }
	printf("\n####相机同步：处理消息线程退出!\n\n");
	g_uDMThreadId = (pthread_t)0;
	pthread_exit((void*)0);
}

/*
* 函数介绍：接受连接线程
* 输入参数：pArg-线程入口参数
* 输出参数：无
* 返回值 ：无
*/
void* AcceptConnectThread(void* pArg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In AcceptConnectThread()\n");
#endif
    printf("侦听连接线程 g_uACThreadId=%u started...\n", (UINT_32)pthread_self());
    /*
	if (NULL == pArg)
	{
	    printf("\n####相机同步：线程接收参数为空，异常退出!\n\n");
	    g_uACThreadId = (pthread_t)0;
		pthread_exit((void*)-1);
	}
	mvCSynchronization *pSyn = (mvCSynchronization *)pArg;
    */
    struct sockaddr_in sLeftHostAddr;
	socklen_t nSize = sizeof(struct sockaddr_in);

	while (!g_bEndThread)
	{
	    if (!g_bLeftLink)
	    {
	        /* //如果拔网线后侦听套接字无效，则重新建立侦听套接字
            if (!mvPrepareSocket())
            {
                printf("Create or set socket option error!\n");
                continue;
            }
            */
            //printf("\n####相机同步：等待被动TCP套接字连接，m_nSocket=%d, m_nSynPort=%d...\n\n", g_SynProcess.m_nAcceptSocket, g_SynProcess.m_nSynPort);
	        //if (g_SynProcess.mvIsFdCanRead(g_SynProcess.m_nSocket))
            //{}
            g_SynProcess.m_nPassiveSocket = accept(g_SynProcess.m_nAcceptSocket, (struct sockaddr*)&sLeftHostAddr, &nSize);
            if (g_SynProcess.m_nPassiveSocket != -1)   //接收成功
            {
                g_bLeftLink = true;
                g_SynProcess.m_nLeftLinkCount = 0;
                printf("\n左边连接建立成功：g_SynProcess.m_nPassiveSocket=%d\n", g_SynProcess.m_nPassiveSocket);

                if (!g_SynProcess.mvMySetSocketOpt(g_SynProcess.m_nPassiveSocket, SO_REUSEADDR))
                {
                    printf("\n####相机同步：设置左连接TCP套接字重用失败，关闭并重新侦听左连接!\n\n");
                    g_SynProcess.mvMyClosePassiveSocket();
                    continue;
                }

                if (g_bEndThread)
                    break;

                if (!g_SynProcess.m_bSendMarkerToLeft)
                {
                    if (!g_SynProcess.mvSendMarkerToLeft())
                    {
                        printf("\n####相机同步：往左边发送标定失败，关闭并重新侦听左连接!\n\n");
                        //g_bLeftLink = false;
                        g_SynProcess.mvMyClosePassiveSocket();
                        continue;
                    }
                    else
                    {
                        g_SynProcess.m_bSendMarkerToLeft = true;
                    }
                }

                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

                g_SynProcess.mvJoinThread(g_uRLMThreadId);
                if (pthread_create(&g_uRLMThreadId, &attr, RecvLeftMsgThread, NULL) != 0)
                {
                    printf("\n####相机同步：创建接收左边消息线程失败，关闭并重新侦听左连接!\n\n");
                    pthread_attr_destroy(&attr);
                    g_SynProcess.mvMyClosePassiveSocket();
                    continue;
                }

                pthread_attr_destroy(&attr);
            }
        }
        sleep(2);
    }
	printf("\n####相机同步：侦听左连接线程退出!\n\n");
	g_uACThreadId = (pthread_t)0;
	pthread_exit((void*)0);
}

/*
* 函数介绍：构造函数
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCSynchronization::mvCSynchronization()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvCSynchronization()\n");
#endif
    m_pLeftMatrix = cvCreateMat(3, 3, CV_32FC1);
	m_pRightMatrix = cvCreateMat(3, 3, CV_32FC1);
    /*
    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
    {
        m_fLeftMatrix[i][j] = 0.0;
        m_fRightMatrix[i][j] = 0.0;
    }
*/
    m_strMyMacAddress.clear();
    m_strLeftMacAddress.clear();
    m_strSynHost.clear();
    m_bSynInitSuccess = false;
    m_bSendMacToRight = false;
    m_bSendMarkerToRight = false;
    m_bSendMarkerToLeft = false;
    m_bRecvLeftMac = false;
    m_bRecvLeftMarker = false;
    m_bRecvRightMarker = false;
    m_nSynPort = 41030;
    m_nActiveSocket = 0;
    m_nPassiveSocket = 0;
    m_nLeftLinkCount = 0;
    m_nSynTimeCount = 0;
    m_nAreaY[0] = 0;
    m_nAreaY[1] = 0;
    m_nLeftAreaTop = 0;
    m_nLeftAreaBottom = 0;
    m_nRightAreaTop = 0;
    m_nRightAreaBottom = 0;

    m_nActiveSocket = 0;
    m_nAcceptSocket = 0;
    //m_fpLeftMatrix = 0;
    //m_fpRightMatrix = 0;
    pthread_mutex_init(&m_LeftQueueMutex, NULL);
    pthread_mutex_init(&m_RightQueueMutex, NULL);
    pthread_mutex_init(&m_LeftSynMsgMutex, NULL);
    pthread_mutex_init(&m_RightSynMsgMutex, NULL);
}

/*
* 函数介绍：析构函数
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCSynchronization::~mvCSynchronization()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::~mvCSynchronization()\n");
#endif
    cvReleaseMat(&m_pLeftMatrix);
    cvReleaseMat(&m_pRightMatrix);
    pthread_mutex_destroy(&m_LeftSynMsgMutex);
    pthread_mutex_destroy(&m_RightSynMsgMutex);
    pthread_mutex_destroy(&m_LeftQueueMutex);
    pthread_mutex_destroy(&m_RightQueueMutex);
    m_listLeftSynMsg.clear();
    m_listRightSynMsg.clear();
    m_mapLeftRepeatQueue.clear();
    m_mapRightRepeatQueue.clear();
}

/*
* 函数介绍：准备工作
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSynchronization::mvInit()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvInit()\n");
#endif
    m_bUnInited = false;
    signal(SIGPIPE, MySigHandle);

    //mvGetCameraType();

    if (!mvGetLeftMacAddress())
    {
        printf("Get left MAC error!\n");
        return false;
    }

    if (!mvGetMyMacAddress())
    {
        printf("Get my own MAC error!\n");
        return false;
    }

    if (!mvGetRightHostIp())
    {
        printf("Get right host IP error!\n");
        return false;
    }

    if (!mvGetSynMarkerGroup())
    {
        printf("Get syn marker group error!\n");
        //return false;
    }

    if (!mvPrepareSocket())
    {
        printf("Create or set socket option error!\n");
        return false;
    }

    if (m_sLeftTrack.size() != 4)
    {
        printf("自身左标定未读成功!\n");
        //return false;
    }

    if (m_sRightTrack.size() != 4)
    {
        printf("自身右标定未读成功!\n");
        //return false;
    }

#ifdef PRINT_SYN_PROCESS_LOG
    mvOpenLogFile();
#endif

	return mvStartThreads();
}

#ifdef PRINT_SYN_PROCESS_LOG
void mvCSynchronization::mvOpenLogFile()
{
    fpLeftPushLog = NULL;
    fpLeftPushLog = fopen("LeftPush.log", "wb");
    if(NULL == fpLeftPushLog)
    {
        printf("\nOpen left push log file error!\n");
    }

    fpRightPushLog = NULL;
    fpRightPushLog = fopen("RightPush.log", "wb");
    if(NULL == fpRightPushLog)
    {
        printf("\nOpen right push log file error!\n");
    }

    fpLeftPopLog = NULL;
    fpLeftPopLog = fopen("LeftPop.log", "wb");
    if(NULL == fpLeftPopLog)
    {
        printf("\nOpen left pop log file error!\n");
    }

    fpRightPopLog = NULL;
    fpRightPopLog = fopen("RightPop.log", "wb");
    if(NULL == fpRightPopLog)
    {
        printf("\nOpen right pop log file error!\n");
    }

    fpLeftCmpLog = NULL;
    fpLeftCmpLog = fopen("LeftCmp.log", "wb");
    if(NULL == fpLeftCmpLog)
    {
        printf("\nOpen left cmp log file error!\n");
    }

    fpRightCmpLog = NULL;
    fpRightCmpLog = fopen("RightCmp.log", "wb");
    if(NULL == fpRightCmpLog)
    {
        printf("\nOpen right cmp log file error!\n");
    }

    fpCmpDisLog = NULL;
    fpCmpDisLog = fopen("CmpDis.log", "wb");
    if(NULL == fpCmpDisLog)
    {
        printf("\nOpen cmp dis log file error!\n");
    }
}
#endif

#ifdef PRINT_SYN_PROCESS_LOG
void mvCSynchronization::mvCloseLogFile()
{
    if (fpLeftPushLog != NULL)
        fclose(fpLeftPushLog);

    if (fpRightPushLog != NULL)
        fclose(fpRightPushLog);

    if (fpLeftPopLog != NULL)
        fclose(fpLeftPopLog);

    if (fpRightPopLog != NULL)
        fclose(fpRightPopLog);

    if (fpLeftCmpLog != NULL)
        fclose(fpLeftCmpLog);

    if (fpRightCmpLog != NULL)
        fclose(fpRightCmpLog);

    if (fpCmpDisLog != NULL)
        fclose(fpCmpDisLog);
}
#endif

/*
* 函数介绍：结束清理工作
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCSynchronization::mvUnInit()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvUnInit()\n");
#endif

#ifdef PRINT_SYN_PROCESS_LOG
    mvCloseLogFile();
#endif

    if (!m_bUnInited)
    {
        mvMyClosePassiveSocket();
        mvMyCloseActiveSocket();

        //mvJoinThread(g_uRLMThreadId);
        //mvJoinThread(g_uRRMThreadId);
        //mvJoinThread(g_uACThreadId);
        mvJoinThread(g_uDMThreadId);

        m_bUnInited = true;
        printf("\n####相机同步：服务退出完成!\n\n");
    }
    else
    {
        printf("\n####相机同步：服务已经提前退出!\n\n");
        return;
    }
}

/*
* 函数介绍：从数据库查询左侧主机MAC地址
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSynchronization::mvGetLeftMacAddress()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetLeftMacAddress()\n");
#endif
    char szGetSynMac[128] = {0};
    sprintf(szGetSynMac, "select SYN_MAC from CHAN_INFO where CAMERA_ID=%d;", g_nCameraID);

    //printf("\nSelect SYN_MAC SQL: %s\n", szGetSynMac);
    MysqlQuery q = g_skpDB.execQuery(string(szGetSynMac));

    if (!q.eof())
        m_strLeftMacAddress = q.getStringFileds(0);

    printf("\n数据库中左MAC为:%s\n", m_strLeftMacAddress.c_str());

    q.finalize();
    return true;
}

/*
* 函数介绍：更新数据库中左侧主机MAC地址
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSynchronization::mvStoreLeftMacAddress()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvStoreLeftMacAddress()\n");
#endif
    if (m_strLeftMacAddress.empty())
    {
        printf("要存储的左MAC为空!\n");
    }

    char szUpdSynMac[128] = {0};
    sprintf(szUpdSynMac, "update CHAN_INFO set SYN_MAC=\"%s\" where CAMERA_ID=%d;", m_strLeftMacAddress.c_str(), g_nCameraID);

    //printf("\nUpdate SYN_MAC SQL: %s\n", szUpdSynMac);
    printf("\n存储数据库，左MAC为:%s\n", m_strLeftMacAddress.c_str());

    if (g_skpDB.execSQL(szUpdSynMac) == 0)
    {
        m_bRecvLeftMac = true;
        return true;
    }
    return false;
}

/*
* 函数介绍：获得本机MAC地址
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSynchronization::mvGetMyMacAddress()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetMyMacAddress()\n");
#endif
    u_char szBuff[6] = {0};
    if (mac_addr_sys(szBuff) == 0)
    {
        for (int i = 0; i < 6; i++)
        {
            u_char szBuff2[2] = {0};
            sprintf((char*)szBuff2, "%.2x", szBuff[i]);
            m_strMyMacAddress.append((char*)szBuff2, 2*sizeof(u_char));
        }
        printf("\nMy MAC-ADDR is: %s\n", m_strMyMacAddress.c_str());
        return true;
    }
    printf("获取自身MAC-ADDR出错!\n");
    return false;
}

/*
* 函数介绍：查询数据库中存储的右侧主机IP
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSynchronization::mvGetRightHostIp()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetRightHostIp()\n");
#endif
    char szGetRightHost[128] = {0};
    sprintf(szGetRightHost, "select SYN_HOST from CHAN_INFO where CAMERA_ID=%d;", g_nCameraID);

    //printf("\nselect SYN_HOST SQL: %s\n", szGetRightHost);
    MysqlQuery q = g_skpDB.execQuery(string(szGetRightHost));

    if (!q.eof())
    {
        m_strSynHost = q.getStringFileds(0);
    }

    printf("\n数据库中右主机为:%s\n", m_strSynHost.c_str());

    q.finalize();
    return true;
}

/*
* 函数介绍：返回车牌检测区域中心位置
* 输入参数：无
* 输出参数：无
* 返回值 ：车牌检测区域中心位置x坐标
*/
int mvCSynchronization::mvGetDetectAreaCenter() const
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetDetectAreaCenter()\n");
#endif
    return m_nDetectAreaCenter;
}

/*
bool mvCSynchronization::mvCountDetectAreaCenter()
{
    char szGetCenter[128] = {0};
    string strArea("");

    sprintf(szGetCenter, "select CardArea from ROAD_SETTING_INFO as t_s,CHAN_INFO as t_c where CAMERA_ID=%d and t_c.CHAN_ID=t_s.Channel;", g_nCameraID);
    //printf("\nselect SYN_MARKER SQL: %s\n", szGetSynMarker);

    MysqlQuery q = g_skpDB.execQuery(string(szGetCenter));
    if (!q.eof())
    {
        strArea = q.getStringFileds(0);
    }
    q.finalize();

    if (strArea.empty())
    {
        printf("\n数据库中检测区域为空!\n");
        return false;
    }

    printf("\n数据库中检测区域为%s\n", strArea.c_str());

    stringstream stream("");
    string sub_str("");
    char szBuffer[128] = {0};
    stream.str(strArea);

    int i = 0;
    float sum = 0.0;
    while(getline(stream, sub_str, ')'))		// Get a segment
    {
        sub_str.erase(0,1);		// Get rid of the first character '('
        memset(szBuffer, 0, sizeof(szBuffer));

        stringstream streamTemp(sub_str);
        string subTemp;

        while(getline(streamTemp, subTemp, '|'))
        {
            float x, y;
            strcpy(szBuffer, subTemp.c_str());
            sscanf(szBuffer, "%f,%f", &x, &y);

            printf("====CardArea====X[%d]=%f\n", i, x);
            sum += x;
            i++;
        }
    }
    m_nDetectAreaCenter = sum/i;
    return true;
}
*/

//bool mvCSynchronization::mvGetSynMarkerGroup()
//{
//    char szGetSynMarker[128] = {0};
//    string strLeftMarker("");
//    string strRightMarker("");
//    string strArea("");
//
//    sprintf(szGetSynMarker, "select SynchLeftArea,SynchRightArea,CardArea from ROAD_SETTING_INFO as t_s,CHAN_INFO as t_c where CAMERA_ID=%d and t_c.CHAN_ID=t_s.Channel;", g_nCameraID);
//    //printf("\nselect SYN_MARKER SQL: %s\n", szGetSynMarker);
//
//    MysqlQuery q = g_skpDB.execQuery(string(szGetSynMarker));
//    if (!q.eof())
//    {
//        strLeftMarker = q.getStringFileds(0);
//        strRightMarker = q.getStringFileds(1);
//        strArea = q.getStringFileds(2);
//    }
//    q.finalize();
//
//    if (strLeftMarker.empty() && strRightMarker.empty())
//    {
//        printf("\n数据库中左右同步标定均为空!\n");
//        return false;
//    }
//    if (strArea.empty())
//    {
//        printf("\n数据库中检测区域为空!\n");
//        return false;
//    }
//
//    printf("\n数据库中左同步标定为:%s，右同步标定为%s\n", strLeftMarker.c_str(), strRightMarker.c_str());
//    printf("\n数据库中检测区域为%s\n", strArea.c_str());
//
//    stringstream stream("");
//    string sub_str("");
//    char szBuffer[128] = {0};
//    stream.str(strLeftMarker);
//
//    m_sLeftTrack.clear();
//    while(getline(stream, sub_str, ')'))		/* Get a segment */
//    {
//        sub_str.erase(0,1);		/* Get rid of the first character '(' */
//        memset(szBuffer, 0, sizeof(szBuffer));
//
//        stringstream streamTemp(sub_str);
//        string subTemp;
//        int i = 0;
//        while(getline(streamTemp, subTemp, '|'))
//        {
//            float x, y;
//            strcpy(szBuffer, subTemp.c_str());
//            sscanf(szBuffer, "%f,%f", &x, &y);
//            CvPoint point;
//            point.x = x;
//            if (JAI_CAMERA_LINK_FIELD == g_nCameraType)
//            {
//                point.y = y/2;
//            }
//            else
//            {
//                point.y = y;
//            }
//            m_sLeftTrack.push_back(point);
//            printf("m_sLeftTrack[%d].x=%d, m_sLeftTrack[%d].y=%d\n", i, point.x, i, point.y);
//            i++;
//        }
//    }
//
//    stream.clear();
//    sub_str.clear();
//    stream.str(strRightMarker);
//    m_sRightTrack.clear();
//    while(getline(stream, sub_str, ')'))		/* Get a segment */
//    {
//        sub_str.erase(0,1);		/* Get rid of the first character '(' */
//        memset(szBuffer, 0, sizeof(szBuffer));
//
//        stringstream streamTemp(sub_str);
//        string subTemp;
//        int i = 0;
//        while(getline(streamTemp, subTemp, '|'))
//        {
//            float x, y;
//            strcpy(szBuffer, subTemp.c_str());
//            sscanf(szBuffer, "%f,%f", &x, &y);
//            CvPoint point;
//            point.x = x;
//            if (JAI_CAMERA_LINK_FIELD == g_nCameraType)
//            {
//                point.y = y/2;
//            }
//            else
//            {
//                point.y = y;
//            }
//            m_sRightTrack.push_back(point);
//            printf("m_sRightTrack[%d].x=%d, m_sRightTrack[%d].y=%d\n", i, point.x, i, point.y);
//            i++;
//        }
//    }
//
//    stream.clear();
//    sub_str.clear();
//    stream.str(strArea);
//    int i = 0;
//    float sum = 0.0;
//    while(getline(stream, sub_str, ')'))		/* Get a segment */
//    {
//        sub_str.erase(0,1);		/* Get rid of the first character '(' */
//        memset(szBuffer, 0, sizeof(szBuffer));
//
//        stringstream streamTemp(sub_str);
//        string subTemp;
//
//        while(getline(streamTemp, subTemp, '|'))
//        {
//            float x, y;
//            strcpy(szBuffer, subTemp.c_str());
//            sscanf(szBuffer, "%f,%f", &x, &y);
//
//            printf("====CardArea====X[%d]=%f\n", i, x);
//            sum += x;
//            i++;
//        }
//    }
//    m_nDetectAreaCenter = sum/i;
//    printf("======m_nDetectAreaCenter=%d\n", m_nDetectAreaCenter);
//    return true;
//}

/*
* 函数介绍：读取同步所需标定点
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvGetSynMarkerGroup()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetSynMarkerGroup()\n");
#endif
    CvRect rectArea;
    CXmlParaUtil xmlParam;
    xmlParam.GetSynchRegion(m_sLeftTrack, m_sRightTrack, rectArea); //m_nDetectAreaCenter);

    if (m_sLeftTrack.size() != 4 && m_sRightTrack.size() != 4)
    {
        g_nHasSynTrack = 0;
        printf("\n获取左右同步标定点均失败!\n");
        return false;
    }
    else
    {
        g_nHasSynTrack = 1;
    }

    m_nDetectAreaCenter = rectArea.x + rectArea.width/2;

    if (m_nDetectAreaCenter <= 0)
    {
        printf("\n检测区域中心位置读取失败!\n");
        return false;
    }

    TRACK_GROUP::iterator iter = m_sLeftTrack.begin();
    int i = 0;
    printf("\n左同步标定点为:\n");
    while (iter != m_sLeftTrack.end())
    {
        printf("x[%d]=%d, y[%d]=%d\n", i, iter->x, i, iter->y);
        i++;
        iter++;
    }

    iter = m_sRightTrack.begin();
    i = 0;
    printf("\n右同步标定点为:\n");
    while (iter != m_sRightTrack.end())
    {
        printf("x[%d]=%d, y[%d]=%d\n", i, iter->x, i, iter->y);
        i++;
        iter++;
    }

    m_nAreaY[0] = rectArea.y;
    m_nAreaY[1] = rectArea.y + rectArea.height;
    printf("\n检测区域中心位置m_nDetectAreaCenter=%d,m_nAreaY[0]=%d,m_nAreaY[1]=%d\n",
           m_nDetectAreaCenter, m_nAreaY[0], m_nAreaY[1]);
/*
    CvPoint pointTop, pointBottom;
    pointTop.x = m_nDetectAreaCenter;
    pointTop.y = rectArea.y;
    pointBottom.x = m_nDetectAreaCenter;
    pointBottom.y = rectArea.y + rectArea.height;
    m_sAreaChar.push_back(pointTop);
    m_sAreaChar.push_back(pointBottom);
*/
    return true;
}

/*
* 函数介绍：获取相机类型
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCSynchronization::mvGetCameraType()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetCameraType()\n");
#endif
    char szGetCameraType[128] = {0};
    sprintf(szGetCameraType, "select CAMERA_TYPE from CHAN_INFO where CAMERA_ID=%d;", g_nCameraID);

    MysqlQuery q = g_skpDB.execQuery(string(szGetCameraType));

    if (!q.eof())
    {
        g_nCameraType = q.getIntFileds(0);
    }

    q.finalize();
}

/*
* 函数介绍：准备被动连接套接字
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPrepareSocket()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPrepareSocket()\n");
#endif
    mvCloseSocket(m_nAcceptSocket);

    if(mvCreateSocket(m_nAcceptSocket,1)==false)
	{
	    printf("\n####相机同步：创建被动连接TCP套接字失败!\n\n");
		return false;
	}
	//printf("\n####相机同步：创建被动连接TCP套接字成功!\n\n");

	if (!mvMySetSocketOpt(m_nAcceptSocket, SO_REUSEADDR))
    {
        printf("\n####相机同步：设置被动连接TCP套接字重用失败!\n\n");
        return false;
    }

	if(mvBindPort(m_nAcceptSocket,m_nSynPort)==false)
	{
	    printf("\n####相机同步：绑定被动连接TCP套接字端口失败!\n\n");
		return false;
	}
	//printf("\n####相机同步：绑定被动连接TCP套接字端口成功!\n\n");

    if (mvStartListen(m_nAcceptSocket) == false)
    {
        printf("\n####相机同步：被动连接TCP套接字开始侦听失败!\n\n");
        return false;
    }
/*
    if (!mvPrepareActiveSocket())
	{
	   return false;
	}
*/
	return true;
}

/*
* 函数介绍：准备主动连接套接字
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPrepareActiveSocket()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPrepareActiveSocket()\n");
#endif
    mvCloseSocket(m_nActiveSocket);

    if(mvCreateSocket(m_nActiveSocket,1)==false)
	{
	    printf("\n####相机同步：创建主动连接TCP套接字失败!\n\n");
		return false;
	}
	//printf("\n####相机同步：创建主动连接TCP套接字成功!\n\n");
/*
	if (!mvMySetSocketOpt(m_nActiveSocket, SO_SNDTIMEO))
    {
        printf("\n####相机同步：设置主动连接TCP套接字发送超时失败!\n\n");
        return false;
    }

	if (!mvMySetSocketOpt(m_nActiveSocket, SO_SNDBUF))
    {
        printf("\n####相机同步：设置主动连接TCP套接字发送缓冲区大小失败!\n\n");
        return false;
    }

	if (!mvMySetSocketOpt(m_nActiveSocket, SO_RCVBUF))
    {
        printf("\n####相机同步：设置主动连接TCP套接字接收缓冲区大小失败!\n\n");
        return false;
    }
*/
	return true;
}

/*
* 函数介绍：重新设置选项
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvReSetOption()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvReSetOption()\n");
#endif
    m_bSendMarkerToLeft = false;
    m_bSendMacToRight = false;
    m_bSendMarkerToRight = false;
    m_bRecvLeftMac = false;
    m_bRecvLeftMarker = false;
    m_bRecvRightMarker = false;

    //mvGetCameraType();

    if (!mvGetLeftMacAddress())
    {
        printf("\nGet left MAC error!\n");
        //return false;
    }

    if (!mvGetSynMarkerGroup())
    {
        printf("Get syn marker group error!\n");
        //return false;
    }

    string strOldRightIp = m_strSynHost;
    if (!mvGetRightHostIp())
    {
        printf("\nGet right host IP error!\n");
        //return false;
    }
    if (strOldRightIp != m_strSynHost)
    {
        mvMyCloseActiveSocket();
        mvConnOrLink();
    }
    else
    {
        if (g_bRightLink)
        {
            if (!mvSendMacToRight())
            {
                printf("\nSend mac to right error!\n");
                mvMyCloseActiveSocket();
                //return false;
            }
            else
            {
                m_bSendMacToRight = true;
            }
        }

        if (g_bRightLink) //may be changed by mvMyCloseActiveSocket.
        {
            if (!mvSendMarkerToRight())
            {
                printf("\nSend marker to right error!\n");
                mvMyCloseActiveSocket();
                //return false;
            }
            else
            {
                m_bSendMarkerToRight = true;
            }
        }
    }

    if (g_bLeftLink)
    {
        if (!mvSendMarkerToLeft())
        {
            printf("\nSend marker to left error!\n");
            mvMyClosePassiveSocket();
            //return false;
        }
        else
        {
            m_bSendMarkerToLeft = true;
        }
    }

    return true;
}

/*
* 函数介绍：主线程的调用接口，断开时重连或连接正常时测试心跳
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCSynchronization::mvConnOrLink()
{
    if (!g_bEndThread && m_bSynInitSuccess)
    {
        if (!g_bRightLink)
        {
            if (m_strSynHost.empty() || m_nSynPort != 41030 || m_strSynHost == g_ServerHost || m_strSynHost == "0.0.0.0" || m_strSynHost == "127.0.0.1")
            {
                if (m_nSynTimeCount++ == 12)
                {
                    m_nSynTimeCount = 0;
                    LogError("相机同步：右边主机参数异常：host=%s,port=%d", m_strSynHost.c_str(), m_nSynPort);
                }
                printf("\n####相机同步：右边主机参数异常，稍后重试：host=%s\tport=%d\n", m_strSynHost.c_str(), m_nSynPort);
                return;
            }
            else
            {
                if (!mvPrepareActiveSocket())
                {
                   return;
                }
                printf("\n####相机同步：尝试连接右边相邻主机，m_nActiveSocket=%d, m_strSynHost=%s, m_nSynPort=%d ....\n\n", m_nActiveSocket, m_strSynHost.c_str(), m_nSynPort);
                if (!mvWaitConnect(m_nActiveSocket, m_strSynHost, m_nSynPort,5))
                {
                    if (m_nSynTimeCount++ == 12)
                    {
                        m_nSynTimeCount = 0;
                        LogError("相机同步：主动连接右边相邻主机失败,Host=%s,Port=%d", m_strSynHost.c_str(), m_nSynPort);
                    }
                    printf("\n####相机同步：主动连接右边相邻主机失败，稍后重试!\n\n");
                }
                else
                {
                    g_bRightLink = true;
                    printf("\n####相机同步：主动连接右边相邻主机成功，m_nActiveSocket=%d\n\n", m_nActiveSocket);

                    if (!mvSendMacToRight())
                    {
                        printf("\n####相机同步：往右边发送MAC失败，稍后重试!\n\n");
                        //g_bRightLink = false;
                        mvMyCloseActiveSocket();
                        return;
                    }
                    else
                    {
                        m_bSendMacToRight = true;
                    }

                    if (!mvSendMarkerToRight())
                    {
                        printf("\n####相机同步：往右边发送标定失败，稍后重试!\n\n");
                        //g_bRightLink = false;
                        mvMyCloseActiveSocket();
                        return;
                    }
                    else
                    {
                        m_bSendMarkerToRight = true;
                    }

                    pthread_attr_t   attr;
                    pthread_attr_init(&attr);
                    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

                    mvJoinThread(g_uRRMThreadId);
                    if (pthread_create(&g_uRRMThreadId, &attr, RecvRightMsgThread, NULL) != 0)
                    {
                        printf("\n####相机同步：创建接收右边消息线程失败，关闭右连接!\n\n");
                        pthread_attr_destroy(&attr);
                        //g_bRightLink = false;
                        mvMyCloseActiveSocket();
                        m_bSendMacToRight = false;
                        m_bSendMarkerToRight = false;
                        return;
                    }
                    else
                    {
                        pthread_attr_destroy(&attr);
                        //g_bRightLink = true;
                        return;
                    }
                }
            }
        }
        else
        {
            printf("\n右边连接正常: m_nActiveSocket=%d\n", m_nActiveSocket);

            if (!m_bSendMacToRight && g_bRightLink)
            {
                if (!mvSendMacToRight())
                {
                    printf("\n####相机同步：往右边发送MAC失败，稍后重试!\n\n");
                    //g_bRightLink = false;
                    mvMyCloseActiveSocket();
                    return;
                }
                else
                {
                    m_bSendMacToRight = true;
                }
            }

            if (!m_bSendMarkerToRight && g_bRightLink)
            {
                if (!mvSendMarkerToRight())
                {
                    printf("\n####相机同步：往右边发送标定失败，稍后重试!\n\n");
                    //g_bRightLink = false;
                    mvMyCloseActiveSocket();
                    return;
                }
                else
                {
                    m_bSendMarkerToRight = true;
                }
            }

            if (!m_bRecvRightMarker && g_bRightLink)
            {
                mvSend(m_nActiveSocket, RE_SEND_MARKER_GROUP);
            }

            if (g_bRightLink)
            {
                if (!mvSend(m_nActiveSocket, EVENT_LINK))
                {
                    printf("\n####相机同步：发送右连接心跳失败，关闭右连接!\n\n");
                    //g_bRightLink = false;
                    mvMyCloseActiveSocket();
                    return;
                }
            }
        }
        if (g_bLeftLink)
        {
            if (m_nLeftLinkCount++ > SRIP_LINK_MAX)
            {
                printf("\n####相机同步：连续多次未收到左连接心跳，关闭左连接!\n\n");
                //g_bLeftLink = false;
                mvMyClosePassiveSocket();
                return;
            }
        }
    }
}

/*
* 函数介绍：向左同步队列压入一条记录
* 输入参数：strRec-记录数据；charData-轨迹特征
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPushOneLeftRecord(const string& strRec, const SYN_CHAR_DATA& charData)//SYN_CHAR_DATA charData)
{
    if (m_bRecvLeftMac && m_bRecvLeftMarker && m_bSendMarkerToLeft && m_mapLeftRepeatQueue.size()<=20)
    {
        /*
        SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)strRec.c_str();
        if (NULL == detHeader)
        {
            return false;
        }
*/
        CHAR_VAR charVar;
        TRACK_MAP mapTrack = charData.mapTrack;

        charVar.fAccuracy = charData.fAccuracy;
        charVar.nWidth = charData.nWidth;
        charVar.nDistance = charData.nDistance;
        charVar.uId = charData.uId;
        charVar.uTimeStamp = charData.uTimeStamp;
        charVar.nType = charData.nType;
        memcpy(charVar.szPlate, charData.szPlate, MAX_PLATE);
        //charVar.nSend = 1;

        string strTrack("");
        if (!mvTrackMapToString(mapTrack, strTrack, 1))
        {
            printf("\n转换左轨迹到string错误!\n");
            return false;
        }
        /*
        if (strTrack.empty())
        {
            printf("\n转换左轨迹string为空!\n");
            return false;
        }
        */
        string strCharData("");
        strCharData.append((char*)&charVar, sizeof(CHAR_VAR));
        strCharData += strTrack;

        string strFullRec("");
        strFullRec += strRec;
        strFullRec += strCharData;

        pthread_mutex_lock(&m_LeftQueueMutex);
        //REPEAT_QUEUE::iterator iter = m_mapLeftRepeatQueue.begin();   //it's unuseful.
        m_mapLeftRepeatQueue.insert(make_pair(charVar.uId, strFullRec));
        pthread_mutex_unlock(&m_LeftQueueMutex);

    #ifdef PRINT_SYN_PROCESS_LOG
        //fprintf(fpLeftPushLog, "push one left record, type = 0x%08x, id = %u, time = %u, plate = %s, m_mapLeftRepeatQueue.size()=%d\n", detHeader->uDetectType, charVar.uId, (UINT_32)time(NULL), charVar.szPlate, (int)m_mapLeftRepeatQueue.size());
        fprintf(fpLeftPushLog, "push one left record, id = %u, time = %u, plate = %s, m_mapLeftRepeatQueue.size()=%d\n", charVar.uId, (UINT_32)time(NULL), charVar.szPlate, (int)m_mapLeftRepeatQueue.size());
        fflush(fpLeftPushLog);
    #endif
        mvSend(m_nPassiveSocket, SEND_CHARACTER_DATA, strCharData);
        return true;
    }
    printf("m_bRecvLeftMac=%d && m_bRecvLeftMarker=%d && m_bSendMarkerToLeft=%d && m_mapLeftRepeatQueue.size()=%d", m_bRecvLeftMac, m_bRecvLeftMarker, m_bSendMarkerToLeft, m_mapLeftRepeatQueue.size());
    printf("\n压入左数据的前提条件尚未满足，无法处理数据，不能压入，否则map将可能耗尽内存!\n");
    return false;
}

/*
* 函数介绍：向右同步队列压入一条记录
* 输入参数：strRec-记录数据；charData-轨迹特征
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPushOneRightRecord(const string& strRec, const SYN_CHAR_DATA& charData)//SYN_CHAR_DATA charData)
{
    if (m_bSendMacToRight && m_bSendMarkerToRight && m_bRecvRightMarker && m_mapRightRepeatQueue.size()<=20)
    {
        /*
        SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)strRec.c_str();
        if (NULL == detHeader)
        {
            return false;
        }
*/
        CHAR_VAR charVar;
        TRACK_MAP mapTrack = charData.mapTrack;

        charVar.fAccuracy = charData.fAccuracy;
        charVar.nWidth = charData.nWidth;
        charVar.nDistance = charData.nDistance;
        charVar.uId = charData.uId;
        charVar.uTimeStamp = charData.uTimeStamp;
        charVar.nType = charData.nType;
        memcpy(charVar.szPlate, charData.szPlate, MAX_PLATE);
        //charVar.nSend = 1;

        string strTrack("");
        if (!mvTrackMapToString(mapTrack, strTrack, 2))
        {
            printf("\n转换右轨迹到string错误!\n");
            return false;
        }
        /*
        if (strTrack.empty())
        {
            printf("\n转换右轨迹string为空!\n");
            return false;
        }
        */

        string strCharData("");
        strCharData.append((char*)&charVar, sizeof(CHAR_VAR));
        strCharData += strTrack;

        string strFullRec("");
        strFullRec += strRec;
        strFullRec += strCharData;

        pthread_mutex_lock(&m_RightQueueMutex);
        //REPEAT_QUEUE::iterator iter = m_mapRightRepeatQueue.begin();
        m_mapRightRepeatQueue.insert(make_pair(charVar.uId, strFullRec));
        pthread_mutex_unlock(&m_RightQueueMutex);

    #ifdef PRINT_SYN_PROCESS_LOG
        //fprintf(fpRightPushLog, "push one right record, type = 0x%08x, id = %u, time = %u, plate = %s, m_mapRightRepeatQueue.size()=%d\n", detHeader->uDetectType, charVar.uId, (UINT_32)time(NULL), charVar.szPlate, m_mapRightRepeatQueue.size());
        fprintf(fpRightPushLog, "push one right record, id = %u, time = %u, plate = %s, m_mapRightRepeatQueue.size()=%d\n", charVar.uId, (UINT_32)time(NULL), charVar.szPlate, m_mapRightRepeatQueue.size());
        fflush(fpRightPushLog);
    #endif
        mvSend(m_nActiveSocket, SEND_CHARACTER_DATA, strCharData);
        return true;
    }
    printf("m_bSendMacToRight=%d && m_bSendMarkerToRight=%d && m_bRecvRightMarker=%d && m_mapRightRepeatQueue.size()=%d", m_bSendMacToRight, m_bSendMarkerToRight, m_bRecvRightMarker, m_mapRightRepeatQueue.size());
    printf("\n压入右数据的前提条件尚未满足，无法处理数据，不能压入，否则map将可能耗尽内存!\n");
    return false;
}

/*
* 函数介绍：从左侧队列取一条经过同步处理的记录
* 输入参数：strRec-记录数据参数
* 输出参数：strRec-经过同步处理的一条记录数据
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvGetOneLeftRecord(string& strRec)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetOneLeftRecord()\n");
#endif
    if (!strRec.empty())
    {
        strRec.clear();
    }

    if (!mvGetTheFirstLeftRecord(strRec))
    {
        return false;
    }

    if (strRec.empty())
    {
        return false;
    }

    if (mvCanBeSend(strRec)) //经过比对，或已超时
    {
        return mvPopTheFirstLeftRecord();
    }
    else
    {
        strRec.clear();
        return false;
    }
}

/*
* 函数介绍：从右侧队列取一条经过同步处理的记录
* 输入参数：strRec-记录数据参数
* 输出参数：strRec-经过同步处理的一条记录数据
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvGetOneRightRecord(string& strRec)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetOneRightRecord()\n");
#endif
    if (!strRec.empty())
    {
        strRec.clear();
    }

    if (!mvGetTheFirstRightRecord(strRec))
    {
        return false;
    }

    if (strRec.empty())
    {
        return false;
    }

    if (mvCanBeSend(strRec)) //经过比对，或已超时
    {
        return mvPopTheFirstRightRecord();
    }
    else
    {
        strRec.clear();
        return false;
    }
}

/*
* 函数介绍：设置左同步标定点
* 输入参数：tGroup-左同步标定点组
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvSetLeftTrack(TRACK_GROUP& tGroup)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvSetLeftTrack()\n");
#endif
    if (tGroup.size() != 4)
    {
        printf("mvSetLeftTrack error!\n");
        return false;
    }

    printf("before set left track...\n");

    m_sLeftTrack.clear();
    m_sLeftTrack = tGroup;
    /*
    TRACK_GROUP::iterator iter = tGroup.begin();
    for (int i = 0; i < 4; i++)
    {
        if (iter == tGroup.end())
        {
            return false;
        }
        CvPoint point;
        point.x = iter->x;
        point.y = iter->y;
        printf("m_sLeftTrack[%d].x=%d, m_sLeftTrack[%d].y=%d\n", i, point.x, i, point.y);
        m_sLeftTrack.push_back(point);
        tGroup.erase(iter++);
    }
    */
    printf("set left track ok\n");
    return true;
}

/*
* 函数介绍：设置右同步标定点
* 输入参数：tGroup-右同步标定点组
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvSetRightTrack(TRACK_GROUP& tGroup)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvSetRightTrack()\n");
#endif
    if (tGroup.size() != 4)
    {
        printf("mvSetRightTrack error!\n");
        return false;
    }

    printf("before set right track...\n");

    m_sRightTrack.clear();
    m_sRightTrack = tGroup;
    /*
    TRACK_GROUP::iterator iter = tGroup.begin();
    for (int i = 0; i < 4; i++)
    {
        if (iter == tGroup.end())
        {
            return false;
        }
        CvPoint point;
        point.x = iter->x;
        point.y = iter->y;
        printf("m_sRightTrack[%d].x=%d, m_sRightTrack[%d].y=%d\n", i, point.x, i, point.y);
        m_sRightTrack.push_back(point);
        tGroup.erase(iter++);
    }
    */
    printf("set right track ok\n");
    return true;
}

/*
* 函数介绍：开启相应线程
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvStartThreads()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvStartThreads()\n");
#endif
	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    mvJoinThread(g_uACThreadId);
	if (pthread_create(&g_uACThreadId, &attr, AcceptConnectThread, NULL) != 0)
	{
		printf("\n####相机同步：创建侦听连接线程失败!\n\n");
		pthread_attr_destroy(&attr);
		return false;
	}
	else
	{
        printf("\n####相机同步：创建侦听连接线程成功!\n\n");
	}

	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    mvJoinThread(g_uDMThreadId);
    if (pthread_create(&g_uDMThreadId, &attr, DealMsgThread, NULL) != 0)
    {
        printf("\n####相机同步：创建消息处理线程失败!\n\n");
        pthread_attr_destroy(&attr);
        return false;
    }
    else
    {
        printf("\n####相机同步：创建消息处理线程成功!\n\n");
    }
    pthread_attr_destroy(&attr);
    m_bSynInitSuccess = true;
    return true;
}

/*
* 函数介绍：设置套接字选项
* 输入参数：nSocket-套接字；nOpt-设置类型
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvMySetSocketOpt(int nSocket, int nOpt)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvMySetSocketOpt()\n");
#endif
     if (SO_REUSEADDR == nOpt)
     {
         int on = 1;
         return (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == 0);
     }
     else if (SO_SNDTIMEO == nOpt)
     {
         struct timeval timeout;
         timeout.tv_sec = 0;
         timeout.tv_usec = 5000; //1ms

         return (setsockopt(nSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval)) == 0);
     }
     else if(SO_SNDBUF == nOpt)
     {
        int nLen = 102400;  //100k
        return (setsockopt(nSocket, SOL_SOCKET, SO_SNDBUF, &nLen, sizeof(int)) == 0);
     }
     else if(SO_RCVBUF == nOpt)
     {
        int nLen = 102400;  //100k
        return (setsockopt(nSocket, SOL_SOCKET, SO_RCVBUF, &nLen, sizeof(int)) == 0);
     }
     else
        return false;
}

/* 暂不使用，发送特征数据在push时完成
bool mvCSynchronization::mvGetTheFirstLeftCharRecord(string& strRec)
{
    if (!strRec.empty())
    {
        strRec.clear();
    }
    pthread_mutex_lock(&m_LeftQueueMutex);
    if (m_mapLeftRepeatQueue.size() == 0)
    {
        pthread_mutex_unlock(&m_LeftQueueMutex);
        return false;
    }
    REPEAT_QUEUE::iterator iter = m_mapLeftRepeatQueue.begin();
    for (; iter != m_mapLeftRepeatQueue.end(); iter++)
    {
        SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)(iter->second.c_str());
        if (NULL == detHeader)
        {
            pthread_mutex_unlock(&m_LeftQueueMutex);
            return false;
        }
        int nHeaderLength = 0;

        if (MIMAX_EVENT_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_EVENT);
        }
        else if (MIMAX_PLATE_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_PLATE);
        }
        else
        {
            printf("\nLeft unknown type!\n");
            //pthread_mutex_unlock(&m_LeftQueueMutex);
            //return false;
            continue;
        }

        if (*((int*)(iter->second.c_str()+nHeaderLength)) == 0)
        {
            *((int*)((char*)(iter->second.c_str()+nHeaderLength))) = 1;
            strRec = iter->second;
            break;
        }
    }

    pthread_mutex_unlock(&m_LeftQueueMutex);
    return true;
}

bool mvCSynchronization::mvGetTheFirstRightCharRecord(string& strRec)
{
    if (!strRec.empty())
    {
        strRec.clear();
    }
    pthread_mutex_lock(&m_RightQueueMutex);
    if (m_mapRightRepeatQueue.size() == 0)
    {
        pthread_mutex_unlock(&m_RightQueueMutex);
        return false;
    }
    REPEAT_QUEUE::iterator iter = m_mapRightRepeatQueue.begin();
    for (; iter != m_mapRightRepeatQueue.end(); iter++)
    {
        SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)(iter->second.c_str());
        if (NULL == detHeader)
        {
            pthread_mutex_unlock(&m_RightQueueMutex);
            return false;
        }
        int nHeaderLength = 0;

        if (MIMAX_EVENT_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_EVENT);
        }
        else if (MIMAX_PLATE_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_PLATE);
        }
        else
        {
            printf("\nRight unknown type!\n");
            //pthread_mutex_unlock(&m_RightQueueMutex);
            //return false;
            continue;
        }

        if (*((int*)(iter->second.c_str()+nHeaderLength)) == 0)
        {
            *((int*)((char*)(iter->second.c_str()+nHeaderLength))) = 1;
            strRec = iter->second;
            break;
        }
    }

    pthread_mutex_unlock(&m_RightQueueMutex);
    return true;
}
*/

/*
* 函数介绍：取出左队列中第一条记录
* 输入参数：strRec-存放记录的变量
* 输出参数：strRec-存放取出的记录
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvGetTheFirstLeftRecord(string& strRec)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetTheFirstLeftRecord()\n");
#endif
    if (!strRec.empty())
    {
        strRec.clear();
    }

    pthread_mutex_lock(&m_LeftQueueMutex);
    if (m_mapLeftRepeatQueue.size() == 0)
    {
        pthread_mutex_unlock(&m_LeftQueueMutex);
        return false;
    }

    REPEAT_QUEUE::iterator iter = m_mapLeftRepeatQueue.begin(); //map is sorted
    strRec = iter->second;
    pthread_mutex_unlock(&m_LeftQueueMutex);
    return true;
}

/*
* 函数介绍：取出右队列中第一条记录
* 输入参数：strRec-存放记录的变量
* 输出参数：strRec-存放取出的记录
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvGetTheFirstRightRecord(string& strRec)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetTheFirstRightRecord()\n");
#endif
    if (!strRec.empty())
    {
        strRec.clear();
    }

    pthread_mutex_lock(&m_RightQueueMutex);
    if (m_mapRightRepeatQueue.size() == 0)
    {
        pthread_mutex_unlock(&m_RightQueueMutex);
        return false;
    }

    REPEAT_QUEUE::iterator iter = m_mapRightRepeatQueue.begin();
    strRec = iter->second;
    pthread_mutex_unlock(&m_RightQueueMutex);
    return true;
}

/*
* 函数介绍：弹出左队列中第一条记录
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPopTheFirstLeftRecord()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPopTheFirstLeftRecord()\n");
#endif
    pthread_mutex_lock(&m_LeftQueueMutex);
    if (m_mapLeftRepeatQueue.size() != 0)
    {
        REPEAT_QUEUE::iterator iter = m_mapLeftRepeatQueue.begin();
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpLeftPopLog, "Send one left record, id = %u, time = %u\n", (UINT_32)iter->first, (UINT_32)time(NULL));
        fflush(fpLeftPopLog);
    #endif
        m_mapLeftRepeatQueue.erase(iter);
        pthread_mutex_unlock(&m_LeftQueueMutex);

        return true;
    }
    pthread_mutex_unlock(&m_LeftQueueMutex);
    return true;
}

/*
* 函数介绍：弹出右队列中第一条记录
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPopTheFirstRightRecord()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPopTheFirstRightRecord()\n");
#endif
    pthread_mutex_lock(&m_RightQueueMutex);
    if (m_mapRightRepeatQueue.size() != 0)
    {
        REPEAT_QUEUE::iterator iter = m_mapRightRepeatQueue.begin();
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpRightPopLog, "Send one right record, id = %u, time = %u\n", (UINT_32)iter->first, (UINT_32)time(NULL));
        fflush(fpRightPopLog);
    #endif
        m_mapRightRepeatQueue.erase(iter);
        pthread_mutex_unlock(&m_RightQueueMutex);

        return true;
    }
    pthread_mutex_unlock(&m_RightQueueMutex);
    return true;
}

/*
* 函数介绍：向左消息队列插入一条同步消息
* 输入参数：strMsg-要插入的消息
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPushOneLeftSynMsg(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPushOneLeftSynMsg()\n");
#endif
    m_nLeftLinkCount = 0; //when recv msg (no matter what kind) we should reset it zero

    SRIP_HEADER* pHeader = (SRIP_HEADER*)strMsg.c_str();
    if (NULL == pHeader)
    {
        return false;
    }
    UINT_32 uCmdId = pHeader->uMsgCommandID;

    if (EVENT_LINK == uCmdId)
    {
        printf("\nReceive left's EVENT_LINK, ok ############## !\n");
        m_nLeftLinkCount = 0;   //Reset count zero
        return true;
    }
    else if (SEND_MAC_ADDRESS == uCmdId)
    {
        printf("\nRecv left msg SEND_MAC_ADDRESS.... strMsg.size() = %d\n", (int)strMsg.size());
        return mvDealLeftMacAddress(strMsg);
    }
    else if (SEND_MARKER_GROUP == uCmdId)
    {
        printf("\nRecv left msg SEND_MARKER_GROUP.... strMsg.size() = %d\n", (int)strMsg.size());
        return mvDealLeftMarkerGroup(strMsg);
    }
    else if (RE_SEND_MARKER_GROUP == uCmdId)
    {
        printf("\nRecv left msg RE_SEND_MARKER_GROUP.... strMsg.size() = %d\n", (int)strMsg.size());
        if (mvSendMarkerToLeft())
        {
            m_bSendMarkerToLeft = true;
            return true;
        }
        return false;
    }
    else if (DEL_REPEATED_RECORD == uCmdId)
    {
        string strBuff;
        memcpy((char*)strBuff.c_str(), strMsg.c_str()+sizeof(SRIP_HEADER), strMsg.size()-sizeof(SRIP_HEADER));
        UINT_32 uId;
        sscanf(strBuff.c_str(), "%u", &uId);
        printf("\nRecv left msg DEL_REPEATED_RECORD, uId=%u, strMsg.size() = %d\n", uId, (int)strMsg.size());
        if (uId <= 0)
        {
            return false;
        }
        return mvOnDelOneLeftRecord(uId);
    }
    else
    {
        printf("\nRecv left msg OTHER MSG, type = 0x%08x\n", uCmdId);

        if (!m_bRecvLeftMac)
        {
            return mvSend(m_nPassiveSocket, RE_SEND_MAC_ADDRESS);
        }
        if (!m_bRecvLeftMarker)
        {
            return mvSend(m_nPassiveSocket, RE_SEND_MARKER_GROUP);
        }
        if (!m_bSendMarkerToLeft)
        {
            if (mvSendMarkerToLeft())
            {
                m_bSendMarkerToLeft = true;
                return true;
            }
            return false;
        }

        pthread_mutex_lock(&m_LeftSynMsgMutex);
        m_listLeftSynMsg.push_back(strMsg);
        pthread_mutex_unlock(&m_LeftSynMsgMutex);
        return true;
    }
}

/*
* 函数介绍：向右消息队列插入一条同步消息
* 输入参数：strMsg-要插入的消息
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPushOneRightSynMsg(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPushOneRightSynMsg()\n");
#endif
    SRIP_HEADER* pHeader = (SRIP_HEADER*)strMsg.c_str();
    if (NULL == pHeader)
    {
        return false;
    }
    UINT_32 uCmdId = pHeader->uMsgCommandID;

    if (EVENT_LINK == uCmdId)
    {
        printf("Receive right's EVENT_LINK, what's wrong?????\n"); //采用单心跳，不会发生
        return true;
    }
    else if (SEND_MARKER_GROUP == uCmdId)
    {
        printf("\nRecv right msg SEND_MARKER_GROUP.... strMsg.size() = %d\n", (int)strMsg.size());
        return mvDealRightMarkerGroup(strMsg);
    }
    else if (RE_SEND_MARKER_GROUP == uCmdId)
    {
        printf("\nRecv right msg RE_SEND_MARKER_GROUP.... strMsg.size() = %d\n", (int)strMsg.size());
        if (mvSendMarkerToRight())
        {
            m_bSendMarkerToRight = true;
            return true;
        }
        return false;
    }
    else if (RE_SEND_MAC_ADDRESS == uCmdId)
    {
        printf("\nRecv right msg RE_SEND_MAC_ADDRESS.... strMsg.size() = %d\n", (int)strMsg.size());
        if (mvSendMacToRight())
        {
            m_bSendMacToRight = true;
            return true;
        }
        return false;
    }
    else if (DEL_REPEATED_RECORD == uCmdId)
    {
        string strBuff;
        memcpy((char*)strBuff.c_str(), strMsg.c_str()+sizeof(SRIP_HEADER), strMsg.size()-sizeof(SRIP_HEADER));
        UINT_32 uId;
        sscanf(strBuff.c_str(), "%u", &uId);
        printf("\nRecv right msg DEL_REPEATED_RECORD, uId=%u, strMsg.size() = %d\n", uId, (int)strMsg.size());
        if (uId <= 0)
        {
            return false;
        }
        return mvOnDelOneRightRecord(uId);
    }
    else
    {
        printf("\nRecv right msg OTHER MSG, type = 0x%08x\n", uCmdId);
        if (!m_bSendMacToRight)
        {
            if (mvSendMacToRight())
            {
                m_bSendMacToRight = true;
                return true;
            }
            return false;
        }
        if (!m_bSendMarkerToRight)
        {
            if (mvSendMarkerToRight())
            {
                m_bSendMarkerToRight = true;
                return true;
            }
            return false;
        }
        if (!m_bRecvRightMarker)
        {
            return mvSend(m_nActiveSocket, RE_SEND_MARKER_GROUP);
        }

        pthread_mutex_lock(&m_RightSynMsgMutex);
        m_listRightSynMsg.push_back(strMsg);
        pthread_mutex_unlock(&m_RightSynMsgMutex);
        return true;
    }
}

/*
* 函数介绍：从左消息队列弹出一条同步消息
* 输入参数：strPopMsg-存储弹出消息的变量
* 输出参数：strPopMsg-弹出的消息内容
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPopOneLeftSynMsg(string& strPopMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPopOneLeftSynMsg()\n");
#endif
    if (!strPopMsg.empty())
    {
        strPopMsg.clear();
    }

    pthread_mutex_lock(&m_LeftSynMsgMutex);
    if (m_listLeftSynMsg.size() != 0)
    {
        SYN_MSG_LIST::iterator iter = m_listLeftSynMsg.begin();
        strPopMsg = *iter;
        m_listLeftSynMsg.erase(iter);
        //m_listSynMsgQueue.pop_front();
        pthread_mutex_unlock(&m_LeftSynMsgMutex);
        return true;
    }
    pthread_mutex_unlock(&m_LeftSynMsgMutex);
    return false;
}

/*
* 函数介绍：从右消息队列弹出一条同步消息
* 输入参数：strPopMsg-存储弹出消息的变量
* 输出参数：strPopMsg-弹出的消息内容
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvPopOneRightSynMsg(string& strPopMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvPopOneRightSynMsg()\n");
#endif
    if (!strPopMsg.empty())
    {
        strPopMsg.clear();
    }

    pthread_mutex_lock(&m_RightSynMsgMutex);
    if (m_listRightSynMsg.size() != 0)
    {
        SYN_MSG_LIST::iterator iter = m_listRightSynMsg.begin();
        strPopMsg = *iter;
        m_listRightSynMsg.erase(iter);
        //m_listSynMsgQueue.pop_front();
        pthread_mutex_unlock(&m_RightSynMsgMutex);
        return true;
    }
    pthread_mutex_unlock(&m_RightSynMsgMutex);
    return false;
}

/*
* 函数介绍：处理一条左同步消息
* 输入参数：strMsg-要处理的消息
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvDealOneLeftSynMsg(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvDealOneLeftSynMsg()\n");
#endif
    if (strMsg.empty())
    {
        return false;
    }

    SRIP_HEADER* pHeader = (SRIP_HEADER*)strMsg.c_str();
    if (NULL == pHeader)
    {
        return false;
    }

    UINT_32 uCmdId = pHeader->uMsgCommandID;

    switch(uCmdId)
    {
        case DEL_REPEATED_RECORD:
        {
            UINT_32 uId;
            memcpy(&uId, strMsg.c_str()+sizeof(SRIP_HEADER), strMsg.size()-sizeof(SRIP_HEADER));
            return mvOnDelOneLeftRecord(uId);
        }

        case SEND_CHARACTER_DATA:
            return mvOnCmpLeftRecord(strMsg);

        default:
            return false;
    }
}

/*
* 函数介绍：处理一条右同步消息
* 输入参数：strMsg-要处理的消息
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvDealOneRightSynMsg(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvDealOneRightSynMsg()\n");
#endif
    if (strMsg.empty())
    {
        return false;
    }

    SRIP_HEADER* pHeader = (SRIP_HEADER*)strMsg.c_str();
    if (NULL == pHeader)
    {
        return false;
    }

    UINT_32 uCmdId = pHeader->uMsgCommandID;
    switch(uCmdId)
    {
        case DEL_REPEATED_RECORD:
        {
            UINT_32 uId;
            memcpy(&uId, strMsg.c_str()+sizeof(SRIP_HEADER), strMsg.size()-sizeof(SRIP_HEADER));
            return mvOnDelOneRightRecord(uId);
        }

        case SEND_CHARACTER_DATA:
            return mvOnCmpRightRecord(strMsg);

        default:
            return false;
    }
}

/*
* 函数介绍：组装并发送一条消息
* 输入参数：nSocket-要发送到的套接字；uCmdType-消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvSend(int nSocket, UINT_32 uCmdType, string strMsg)
{
    //if (nSocket <= 0)
    //    return false;
/*
#ifdef PRINT_SYN_PROCESS_LOG
    struct timeval sTimeNow;
    gettimeofday(&sTimeNow, NULL);
    UINT_32 uTime1 = sTimeNow.tv_sec*1000000 + sTimeNow.tv_usec;
#endif
*/
	SRIP_HEADER sHeader;
	sHeader.uMsgCommandID = uCmdType;
	sHeader.uMsgLen = sizeof(SRIP_HEADER) + strMsg.size();
	//sHeader.uMsgSource = nSocket;
	//printf("before send, sHeader.uMsgLen = %d, sizeof(SRIP_HEADER) =%d, strMsg.size()=%d\n", sHeader.uMsgLen, sizeof(SRIP_HEADER), strMsg.size());
	string strSend("");
	strSend.append((char*)&sHeader,sizeof(SRIP_HEADER));
	strSend += strMsg;

	int nLeft = strSend.size();
	int nBytes = 0;
	int nDataLen = 0;

	if(!mvSendMsgToSocket(nSocket,strSend))
	{
        if (nSocket == m_nActiveSocket)
        {
            mvMyCloseActiveSocket();
        }
        else if (nSocket == m_nPassiveSocket)
        {
            mvMyClosePassiveSocket();
        }
        return false;
	}

	return true;

/*	pthread_mutex_lock(&m_SynSendMutex);
	while (nLeft > 0)
	{
     //   if (nSocket <= 0)
     //       return false;

        if (!mvIsFdCanWrite(nSocket))
        {
            if (nSocket == m_nActiveSocket)
            {
                mvMyCloseActiveSocket();
            }
            else if (nSocket == m_nPassiveSocket)
            {
                mvMyClosePassiveSocket();
            }
            pthread_mutex_unlock(&m_SynSendMutex);
            return false;
        }

		if ((nBytes = send(nSocket, strSend.c_str()+nDataLen, nLeft, MSG_NOSIGNAL)) <= 0)
		{
			printf("\n####相机同步：发送数据出错：%s\n", strerror(errno));
			if (nSocket == m_nActiveSocket)
			{
			    mvMyCloseActiveSocket();
			}
			else if (nSocket == m_nPassiveSocket)
			{
			    mvMyClosePassiveSocket();
			}
			pthread_mutex_unlock(&m_SynSendMutex);
			return false;
		}
		nDataLen += nBytes;
		nLeft -= nBytes;
	}
	pthread_mutex_unlock(&m_SynSendMutex);
	printf("\n发送消息成功!消息类型为uCmdType=0x%08X\n", uCmdType);*/
/*
#ifdef PRINT_SYN_PROCESS_LOG
    gettimeofday(&sTimeNow, NULL);
    UINT_32 uTime2 = sTimeNow.tv_sec*1000000 + sTimeNow.tv_usec;
    fprintf(fpCmpDisLog, "############send out msg###########one send cost us time(us) = %u\n", uTime2-uTime1);
    fflush(fpCmpDisLog);
#endif

	return true;*/
}

/*
* 函数介绍：把轨迹map转换为xml
* 输入参数：mapTrack-轨迹map；strXml-存储转换后xml的变量
* 输出参数：strXml-存储转换后的xml
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvTrackMapToString(TRACK_MAP mapTrack, string& strXml, int nLorR)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvTrackMapToString()\n");
#endif
    char szBuff[8] = {0};
    CSkpRoadXmlValue xmlTrack;
/*
*没有轨迹的目标我们也放进来进行比对
    if (mapTrack.size() == 0)
    {
        printf("\nTrack map is NULL, error!\n");
        return false;
    }
*/
    TRACK_MAP::iterator iter = mapTrack.begin();
    for (int i = 0; iter != mapTrack.end(); iter++,i++)
    {
        memset(szBuff, 0, 8);
        sprintf(szBuff, "%lld", iter->first);
        xmlTrack[i]["Time"] = szBuff;

        TRACK_GROUP trackGrp = iter->second;

        TRACK_GROUP::iterator iter2 = trackGrp.begin();
        int nMinY = iter2->y;
        iter2++;iter2++;
        int nMaxY = iter2->y;

        if (1 == nLorR) //left
        {
            if (nMinY > m_nLeftAreaBottom || nMaxY < m_nLeftAreaTop)
            {
                i--;
                continue;
            }
        }
        else //right
        {
            if (nMinY > m_nRightAreaBottom || nMaxY < m_nRightAreaTop)
            {
                i--;
                continue;
            }
        }

        iter2 = trackGrp.begin();
        if (1 == nLorR)
        {
            for (int j = 0; iter2 != trackGrp.end(); iter2++,j++)
            {
                if (j < 2)
                {
                    xmlTrack[i]["Point"][j]["y"] = max(iter2->y, m_nLeftAreaTop);
                }
                else
                {

                    xmlTrack[i]["Point"][j]["y"] = min(iter2->y, m_nLeftAreaBottom);
                }
                xmlTrack[i]["Point"][j]["x"] = iter2->x;
            }
        }
        else
        {
            for (int j = 0; iter2 != trackGrp.end(); iter2++,j++)
            {
                if (j < 2)
                {
                    xmlTrack[i]["Point"][j]["y"] = max(iter2->y, m_nRightAreaTop);
                }
                else
                {

                    xmlTrack[i]["Point"][j]["y"] = min(iter2->y, m_nRightAreaBottom);
                }
                xmlTrack[i]["Point"][j]["x"] = iter2->x;
            }
        }
    }

    strXml = xmlTrack.toXml();
    xmlTrack.clear();
    return true;
}

/*
* 函数介绍：把xml转换为轨迹map
* 输入参数：strXml-要转换的xml；mapTrack-存储转换后map的变量；
* 输出参数：mapTrack-存储转换后的map
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvStringToTrackMap(const string& strXml, TRACK_MAP& mapTrack)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvStringToTrackMap()\n");
#endif
    int nOffset = 0;
    CSkpRoadXmlValue xmlTrack(strXml, &nOffset);
/*
*轨迹为空也比较
    if (xmlTrack.size() == 0)
    {
        printf("\nTrack xml is NULL, error!\n");
        return false;
    }
*/
    //printf("xmlTrack.size()=%d\n", (int)xmlTrack.size());
    for (int i = 0; i < xmlTrack.size(); i++)
    {
        string strTime = (string)xmlTrack[i]["Time"];
        int64_t llTime;// = (int64_t)atol(strTime.c_str());
        sscanf(strTime.c_str(), "%lld", &llTime);
        TRACK_GROUP trackGrp;

        //printf("xmlTrack[%d][\"Point\"].size()=%d\n", i, (int)xmlTrack[i]["Point"].size());

        for (int j = 0; j < xmlTrack[i]["Point"].size(); j++)
        {
            CvPoint point;
            point.x = (int)xmlTrack[i]["Point"][j]["x"];
            point.y = (int)xmlTrack[i]["Point"][j]["y"];
            trackGrp.push_back(point);
        }
        mapTrack.insert(make_pair(llTime, trackGrp));
    }

    xmlTrack.clear();
    return true;
}

/* 暂放在push接口中发送
bool mvCSynchronization::mvSendOneCharacterData(int nSocket, const string& strRec)
{
    SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)strRec.c_str();
    if (NULL == detHeader)
    {
        return false;
    }

    string strCharData("");
    int nHeaderLength = 0;

    if (MIMAX_EVENT_REP == detHeader->uDetectType)
    {
        nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_EVENT);
    }
    else if (MIMAX_PLATE_REP == detHeader->uDetectType)
    {
        nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_PLATE);
    }
    else
    {
        printf("\nPush character record type error!\n");
        return false;
    }

    strCharData.append(strRec.c_str()+nHeaderLength, strRec.size()-nHeaderLength);

    if (strCharData.empty())
    {
        printf("\nCharacter data is NULL, error!\n");
        return false;
    }
    return mvSend(nSocket, SEND_CHARACTER_DATA, strCharData);
}
*/

/*
* 函数介绍：关闭左侧套接字
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCSynchronization::mvMyClosePassiveSocket()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvMyClosePassiveSocket()\n");
#endif
    /*
    if (mvIsFdCanRead(m_nPassiveSocket))
    {
        shutdown(m_nPassiveSocket, 0);
    }
    if (mvIsFdCanWrite(m_nPassiveSocket))
    {
        shutdown(m_nPassiveSocket, 1);
    }
    */
    if (m_nPassiveSocket > 0)
    {
        shutdown(m_nPassiveSocket, 2);
        close(m_nPassiveSocket);
        m_nPassiveSocket = 0;
    }
    g_bLeftLink = false;
}

/*
* 函数介绍：关闭右侧套接字
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCSynchronization::mvMyCloseActiveSocket()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvMyCloseActiveSocket()\n");
#endif
    /*
    if (mvIsFdCanRead(m_nActiveSocket))
    {
        shutdown(m_nActiveSocket, 0);
    }
    if (mvIsFdCanWrite(m_nActiveSocket))
    {
        shutdown(m_nActiveSocket, 1);
    }
    */
    if (m_nActiveSocket > 0)
    {
        shutdown(m_nActiveSocket, 2);
        close(m_nActiveSocket);
        m_nActiveSocket = 0;
    }
    g_bRightLink = false;
}

/*
* 函数介绍：从左同步队列删除一条记录
* 输入参数：uId-记录的ID
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvOnDelOneLeftRecord(UINT_32 uId)//(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnDelOneLeftRecord()\n");
#endif
    if (uId <= 0)
    {
        return false;
    }

    pthread_mutex_lock(&m_LeftQueueMutex);
    REPEAT_QUEUE::iterator iter = m_mapLeftRepeatQueue.find(uId);
    if (iter != m_mapLeftRepeatQueue.end())
    {
        string strRec = iter->second;
        m_mapLeftRepeatQueue.erase(iter);
        pthread_mutex_unlock(&m_LeftQueueMutex);

        SRIP_DETECT_HEADER* sDetect = (SRIP_DETECT_HEADER*)(strRec.c_str());
        if (NULL == sDetect)
        {
            return false;
        }

        UINT_32 uType = sDetect->uDetectType; //需要在删除记录的时候修改数据库中的状态，根据type确定表名
        if (MIMAX_EVENT_REP == uType)
        {
            if ((sDetect->uRealTime & 0x00010000) == 0x00010000) //是目标检测（***记录）
            {
                uType = MIMAX_PLATE_REP;
            }
        }

        return mvOnUpdateRecStatusInDB(uType, uId, 2); //2表示下次开机可以不发，是我们同步剔除掉的记录
    }
    pthread_mutex_unlock(&m_LeftQueueMutex);
    return true;
}

/*
* 函数介绍：从右同步队列删除一条记录
* 输入参数：uId-记录的ID
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvOnDelOneRightRecord(UINT_32 uId)//(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnDelOneRightRecord()\n");
#endif
    if (uId <= 0)
    {
        return false;
    }

    pthread_mutex_lock(&m_RightQueueMutex);
    REPEAT_QUEUE::iterator iter = m_mapRightRepeatQueue.find(uId);
    if (iter != m_mapRightRepeatQueue.end())
    {
        string strRec = iter->second;
        m_mapRightRepeatQueue.erase(iter);
        pthread_mutex_unlock(&m_RightQueueMutex);

        SRIP_DETECT_HEADER* sDetect = (SRIP_DETECT_HEADER*)(strRec.c_str());
        if (NULL == sDetect)
        {
            return false;
        }

        UINT_32 uType = sDetect->uDetectType;//需要在删除记录的时候修改数据库中的状态，根据type确定表名
        if (MIMAX_EVENT_REP == uType)
        {
            if ((sDetect->uRealTime & 0x00010000) == 0x00010000) //是目标检测（***记录）
            {
                uType = MIMAX_PLATE_REP;
            }
        }

        return mvOnUpdateRecStatusInDB(uType, uId, 2); //2表示下次开机可以不发，是我们同步剔除掉的记录
    }
    pthread_mutex_unlock(&m_RightQueueMutex);
    return true;
}

/*
* 函数介绍：更新数据库中记录的状态
* 输入参数：uType-记录类型；uId-记录ID；nStatus-要修改成的状态
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvOnUpdateRecStatusInDB(UINT_32 uType, UINT_32 uId, int nStatus)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnUpdateRecStatusInDB()\n");
#endif
    char szBuff[128] = {0};
    string strTableName("");

    if(MIMAX_EVENT_REP == uType)
    {
        strTableName = "TRAFFIC_EVENT_INFO";
    }
    else if(MIMAX_PLATE_REP == uType)
    {
        strTableName = "NUMBER_PLATE_INFO";
    }
    else
        return false;

    //strTableName = "NUMBER_PLATE_INFO";    //由于***事件被放在车牌表里
    sprintf(szBuff,"update %s set STATUS=%d where ID=%u;", strTableName.c_str(), nStatus, uId);
    return (g_skpDB.execSQL(szBuff) == 0);
}

/*
* 函数介绍：左同步处理
* 输入参数：strRecord-待同步处理的记录
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvOnCmpLeftRecord(const string& strRecord)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnCmpLeftRecord()\n");
#endif
    //收到一条记录，进行比对，根据比对结果
    //1.删除自己一条记录
    //2.发送一条删除命令
    //3.不处理
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpLeftCmpLog, "left cmp print 1\n");
    fflush(fpLeftCmpLog);
#endif
*/
    if (!m_bRecvLeftMac || !m_bRecvLeftMarker)
    {
        printf("\n没有接收到左MAC或左标定，不能进行去重处理，属异常情况，关闭并重新建立左连接!\n");
        //g_bLeftLink = false;
        mvMyClosePassiveSocket();
        return false;
    }
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpLeftCmpLog, "left cmp print 2\n");
    fflush(fpLeftCmpLog);
#endif
*/
/*
    string strDel("");
    SRIP_HEADER sDelHeader;
    sDelHeader.uMsgCommandID = DEL_REPEATED_RECORD;
    strDel.append((const char*)&sDelHeader, sizeof(SRIP_HEADER));
*/
    string strRecvCharData("");
    strRecvCharData.append(strRecord.c_str()+sizeof(SRIP_HEADER), strRecord.size()-sizeof(SRIP_HEADER));
    if (strRecvCharData.empty())
    {
        printf("\n接收到的左特征数据为空!\n");
        return false;
    }
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpLeftCmpLog, "left cmp print 3\n");
    fflush(fpLeftCmpLog);
#endif
*/
    CHAR_VAR* varRecvChar = (CHAR_VAR*)strRecvCharData.c_str();
    if (NULL == varRecvChar)
    {
        return false;
    }
    UINT_32 uRecvId = varRecvChar->uId;
/*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpLeftCmpLog, "left cmp wait for m_LeftQueueMutex...\n");
    fflush(fpLeftCmpLog);
#endif
*/
    pthread_mutex_lock(&m_LeftQueueMutex);
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpLeftCmpLog, "left cmp got m_LeftQueueMutex, m_mapLeftRepeatQueue.size()=%d\n", (int)m_mapLeftRepeatQueue.size());
    fflush(fpLeftCmpLog);
#endif
*/
    if (m_mapLeftRepeatQueue.size() == 0)
    {
        printf("\n自身左队列尚无数据压入，丢弃处理!\n");
        pthread_mutex_unlock(&m_LeftQueueMutex);
        return true;
    }
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpLeftCmpLog, "left cmp print 4\n");
    fflush(fpLeftCmpLog);
#endif
*/
    REPEAT_QUEUE::iterator iter = m_mapLeftRepeatQueue.begin();
    for (; iter != m_mapLeftRepeatQueue.end(); iter++)
    {
        /*
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpLeftCmpLog, "left cmp print 5\n");
        fflush(fpLeftCmpLog);
    #endif
    */
        string strMyRec = iter->second;
        SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)strMyRec.c_str();
        if (NULL == detHeader)
        {
            pthread_mutex_unlock(&m_LeftQueueMutex);
            return false;
        }

        int nHeaderLength = 0;

        if (MIMAX_EVENT_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_EVENT);
        }
        else if (MIMAX_PLATE_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_PLATE);
        }
        else
        {
            /*
            printf("\nI've got unknown character record, error!\n");
        #ifdef PRINT_SYN_PROCESS_LOG
            fprintf(fpLeftCmpLog, "left cmp got unknown character record\n");
            fflush(fpLeftCmpLog);
        #endif
        */
            //pthread_mutex_unlock(&m_LeftQueueMutex);
            //return false;
            continue;
        }
        /*
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpLeftCmpLog, "left cmp print 6, nHeaderLength=%d, strMyRec.size()=%d\n", nHeaderLength, (int)strMyRec.size());
        fflush(fpLeftCmpLog);
    #endif
    */
        string strMyCharData("");
        strMyCharData.append(strMyRec.c_str()+nHeaderLength, strMyRec.size()-nHeaderLength);

        CHAR_VAR* varMyChar = (CHAR_VAR*)strMyCharData.c_str();
        if (NULL == varRecvChar)
        {
            pthread_mutex_unlock(&m_LeftQueueMutex);
            return false;
        }
        UINT_32 uMyId = varMyChar->uId;

        int nRes = mvHowToDeal(strRecvCharData, strMyCharData, 1);  //末尾一个参数，1表示左比对（使用左矩阵），2表示右比对
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpLeftCmpLog, "left cmp result = %d\n", nRes);
        fflush(fpLeftCmpLog);
    #endif
        if (0 == nRes) //通知对方删除
        {
            *((int*)(iter->second.c_str()+nHeaderLength)) = 1; //+sizeof(int))) = 1;
            pthread_mutex_unlock(&m_LeftQueueMutex);
            //strDel.append((const char*)&uRecvId, sizeof(UINT_32));
            char szBuff[4] = {0};
            sprintf(szBuff, "%u", uRecvId);
            return mvSend(m_nPassiveSocket, DEL_REPEATED_RECORD, string(szBuff));
        }
        else if (1 == nRes) //自己删除
        {
            pthread_mutex_unlock(&m_LeftQueueMutex);
            //strDel.append((const char*)&uMyId, sizeof(UINT_32));
            return mvOnDelOneLeftRecord(uMyId);//(strDel);
        }
    }
    pthread_mutex_unlock(&m_LeftQueueMutex);
    return true;
}

/*
* 函数介绍：右同步处理
* 输入参数：strRecord-待同步处理的记录
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvOnCmpRightRecord(const string& strRecord)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnCmpRightRecord()\n");
#endif
    //收到一条记录，进行比对，根据比对结果
    //1.删除自己一条记录
    //2.发送一条删除命令
    //3.不处理
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpRightCmpLog, "right cmp print 1\n");
    fflush(fpRightCmpLog);
#endif
*/
    if (!m_bRecvRightMarker)
    {
        printf("没有接收到右MAC或右标定，不能进行去重处理，属异常情况，关闭并重新建立右连接!");
        //g_bRightLink = false;
        mvMyCloseActiveSocket();
        return false;
    }
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpRightCmpLog, "right cmp print 2\n");
    fflush(fpRightCmpLog);
#endif
*/
/*
    string strDel("");
    SRIP_HEADER sDelHeader;
    sDelHeader.uMsgCommandID = DEL_REPEATED_RECORD;
    strDel.append((const char*)&sDelHeader, sizeof(SRIP_HEADER));
*/
    string strRecvCharData("");
    strRecvCharData.append(strRecord.c_str()+sizeof(SRIP_HEADER), strRecord.size()-sizeof(SRIP_HEADER));
    if (strRecvCharData.empty())
    {
        printf("\n接收到的右特征数据为空!\n");
        return false;
    }
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpRightCmpLog, "right cmp print 3\n");
    fflush(fpRightCmpLog);
#endif
*/
    CHAR_VAR* varRecvChar = (CHAR_VAR*)strRecvCharData.c_str();
    if (NULL == varRecvChar)
    {
        return false;
    }

    UINT_32 uRecvId = varRecvChar->uId;
/*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpRightCmpLog, "right cmp wait for m_RightQueueMutex...\n");
    fflush(fpRightCmpLog);
#endif
*/
    pthread_mutex_lock(&m_RightQueueMutex);
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpRightCmpLog, "right cmp got m_RightQueueMutex, m_mapRightRepeatQueue.size()=%d\n", (int)m_mapRightRepeatQueue.size());
    fflush(fpRightCmpLog);
#endif
*/
    if (m_mapRightRepeatQueue.size() == 0)
    {
        printf("\n自身右队列尚无数据压入，丢弃处理!\n");
        pthread_mutex_unlock(&m_RightQueueMutex);
        return true;
    }
    /*
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpRightCmpLog, "right cmp print 4\n");
    fflush(fpRightCmpLog);
#endif
*/
    REPEAT_QUEUE::iterator iter = m_mapRightRepeatQueue.begin();
    for (; iter != m_mapRightRepeatQueue.end(); iter++)
    {
        /*
        #ifdef PRINT_SYN_PROCESS_LOG
            fprintf(fpRightCmpLog, "right cmp print 5\n");
            fflush(fpRightCmpLog);
        #endif
        */
        string strMyRec = iter->second;
        SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)strMyRec.c_str();
        if (NULL == detHeader)
        {
            pthread_mutex_unlock(&m_RightQueueMutex);
            return false;
        }

        int nHeaderLength = 0;

        if (MIMAX_EVENT_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_EVENT);
        }
        else if (MIMAX_PLATE_REP == detHeader->uDetectType)
        {
            nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_PLATE);
        }
        else
        {/*
            printf("\nI've got unknown character record, error!\n");
            #ifdef PRINT_SYN_PROCESS_LOG
                fprintf(fpRightCmpLog, "right cmp got unknown character record\n");
                fflush(fpRightCmpLog);
            #endif
            */
            //pthread_mutex_unlock(&m_RightQueueMutex);
            //return false;
            continue;
        }
        /*
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpRightCmpLog, "right cmp print 6\n");
        fflush(fpRightCmpLog);
    #endif
    */
        //printf("right cmp print 6\n");
        string strMyCharData("");
        strMyCharData.append(strMyRec.c_str()+nHeaderLength, strMyRec.size()-nHeaderLength);

        CHAR_VAR* varMyChar = (CHAR_VAR*)strMyCharData.c_str();
        if (NULL == varMyChar)
        {
            pthread_mutex_unlock(&m_RightQueueMutex);
            return false;
        }

        UINT_32 uMyId = varMyChar->uId;

        int nRes = mvHowToDeal(strRecvCharData, strMyCharData, 2);
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpRightCmpLog, "right cmp result = %d\n", nRes);
        fflush(fpRightCmpLog);
    #endif
        if (0 == nRes) //通知对方删除
        {
            *((int*)(iter->second.c_str()+nHeaderLength)) = 1; //+sizeof(int))) = 1;
            pthread_mutex_unlock(&m_RightQueueMutex);
            //strDel.append((const char*)&uRecvId, sizeof(UINT_32));
            char szBuff[4] = {0};
            sprintf(szBuff, "%u", uRecvId);
            return mvSend(m_nActiveSocket, DEL_REPEATED_RECORD, string(szBuff));
        }
        else if (1 == nRes) //自己删除
        {
            pthread_mutex_unlock(&m_RightQueueMutex);
            //strDel.append((const char*)&uMyId, sizeof(UINT_32));
            return mvOnDelOneRightRecord(uMyId);//(strDel);
        }
    }
    pthread_mutex_unlock(&m_RightQueueMutex);
    return true;
}

/*
* 函数介绍：同步处理
* 输入参数：strRecvChar-接收到的特征数据；strMyChar-本机的特征数据；nLorR-左同步或右同步
* 输出参数：无
* 返回值 ：返回1，自己删除；返回0，通知对方删除；返回-1，不处理
*/
int mvCSynchronization::mvHowToDeal(const string& strRecvChar, const string& strMyChar, int nLorR)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvHowToDeal()\n");
#endif
    CHAR_VAR* varRecv = (CHAR_VAR*)strRecvChar.c_str();
    CHAR_VAR* varMine = (CHAR_VAR*)strMyChar.c_str();

    if (NULL == varRecv || NULL == varMine)
    {
        return -1;
    }

    int nRecvType = varRecv->nType;
    int nMyType = varMine->nType;
    UINT_32 uRecvTimeStamp = varRecv->uTimeStamp;
    UINT_32 uMyTimeStamp = varMine->uTimeStamp;
    float fRecvAccur = varRecv->fAccuracy;
    float fMyAccur = varMine->fAccuracy;
    int nRecvWidth = varRecv->nWidth;
    int nMyWidth = varMine->nWidth;
    int nRecvDis = varRecv->nDistance;
    int nMyDis = varMine->nDistance;

    if (abs((long)uRecvTimeStamp - (long)uMyTimeStamp) >= MAX_TIME_IN_QUEUE)  //超过了有效时限，不比对
    {
        return -1;
    }

    if ((1 == nRecvType) && (1 == nMyType)) //都是车牌信息
    {
    #ifdef PRINT_SYN_PROCESS_LOG
        if (1 == nLorR)
        {
            fprintf(fpLeftCmpLog, "left cmp according to plate! varRecv->szPlate=%s, varMine->szPlate=%s\n",varRecv->szPlate, varMine->szPlate);
            fflush(fpLeftCmpLog);
        }
        else if (2 == nLorR)
        {
            fprintf(fpRightCmpLog, "right cmp according to plate! varRecv->szPlate=%s, varMine->szPlate=%s\n",varRecv->szPlate, varMine->szPlate);
            fflush(fpRightCmpLog);
        }
    #endif
        if (memcmp(varRecv->szPlate, varMine->szPlate, MAX_PLATE) == 0)
        {
            if (nRecvDis > nMyDis)
            {
                return 1;
            }
            else if (nRecvDis < nMyDis)
            {
                return 0;
            }
            else
            {
                if (nRecvWidth > nMyWidth)
                {
                    return 1;
                }
                else if (nRecvWidth < nMyWidth)
                {
                    return 0;
                }
                else
                {
                    if (fRecvAccur < fMyAccur)
                    {
                        return 0;
                    }
                    else
                    {
                        return 1;
                    }
                }
            }
        }
        return -1;
    }

    string strRecvTrack("");
    string strMyTrack("");

    strRecvTrack.append(strRecvChar.c_str()+sizeof(CHAR_VAR), strRecvChar.size()-sizeof(CHAR_VAR));
    strMyTrack.append(strMyChar.c_str()+sizeof(CHAR_VAR), strMyChar.size()-sizeof(CHAR_VAR));

    TRACK_MAP mapRecvTrack;
    TRACK_MAP mapMyTrack;

    if (!mvStringToTrackMap(strRecvTrack, mapRecvTrack))
    {
        return -1;
    }
    if (!mvStringToTrackMap(strMyTrack, mapMyTrack))
    {
        return -1;
    }

    if (mapRecvTrack.size() == 0 || mapMyTrack.size() == 0)
    {
        return -1;
    }

    bool bMatch = false;
    if (1 == nLorR)
    {
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpLeftCmpLog, "left cmp according to track group!\n");
        fflush(fpLeftCmpLog);
    #endif
        bMatch = mvMatchCount(mapRecvTrack, mapMyTrack, *m_pLeftMatrix, nLorR);
    }
    else if (2 == nLorR)
    {
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpRightCmpLog, "right cmp according to track group!\n");
        fflush(fpRightCmpLog);
    #endif
        bMatch = mvMatchCount(mapRecvTrack, mapMyTrack, *m_pRightMatrix, nLorR);
    }
    else //will never happen.
    {
        printf("\nCmp with which???\n");
        return -1;
    }

    if (!bMatch)
    {
        return -1;
    }

    if ((1 == nRecvType) && (0 == nMyType))
    {
        return 1;
    }
    else if ((0 == nRecvType) && (1 == nMyType))
    {
        return 0;
    }

    if (nRecvDis > nMyDis)
    {
        return 1;
    }
    else if (nRecvDis < nMyDis)
    {
        return 0;
    }
    else
    {
        if (nRecvWidth > nMyWidth)
        {
            return 1;
        }
        else if (nRecvWidth < nMyWidth)
        {
            return 0;
        }
        else
        {
            if (fRecvAccur < fMyAccur)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }
}

/*
* 函数介绍：判断两个轨迹重合度是否达到最低重合标准
* 输入参数：map1-轨迹1；map2-轨迹2；matrix-转换矩阵；nLorR-左同步或右同步
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvMatchCount(TRACK_MAP map1, TRACK_MAP map2, const CvMat& matrix, int nLorR)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvMatchCount()\n");
#endif
    if (map1.size() == 0 || map2.size() == 0)
    {
        return -1;
    }

    int nSize = map1.size();
    int nSize2 = map2.size();
    int nCount = 0;

    //int i = 0;
    TRACK_MAP::iterator iter = map1.begin();
    while (iter != map1.end())
    {
        //int j = 0;
        TRACK_GROUP tGrp1 = iter->second;
        if (tGrp1.size() == 0)
        {
            iter++;
            continue;
        }
        TRACK_MAP::iterator iter2 = map2.begin();
        while (iter2 != map2.end())
        {
            TRACK_GROUP tGrp2 = iter2->second;
            if (tGrp2.size() == 0)
            {
                iter2++;
                continue;
            }
            int nGap = abs(int(iter->first-iter2->first)/1000);

            if (nGap <= g_nSynTimeGap)
            {
            #ifdef PRINT_SYN_PROCESS_LOG
                fprintf(fpCmpDisLog, "g_nSynTimeGap=%d, nTimeGap=%d\n", g_nSynTimeGap, nGap);
                fflush(fpCmpDisLog);
            #endif
/*
            #ifdef PRINT_SYN_PROCESS_LOG
                struct timeval sTimeNow;
                gettimeofday(&sTimeNow, NULL);
                UINT_32 uTime1 = sTimeNow.tv_sec*1000000 + sTimeNow.tv_usec;
            #endif
*/
                float fMatchRes = mvGetMatchPercent(tGrp1, tGrp2, matrix, nLorR);
            #ifdef PRINT_SYN_PROCESS_LOG
                fprintf(fpCmpDisLog, "fMatchRes = %f\n\n", fMatchRes);
                fflush(fpCmpDisLog);
            #endif
/*
            #ifdef PRINT_SYN_PROCESS_LOG
                gettimeofday(&sTimeNow, NULL);
                UINT_32 uTime2 = sTimeNow.tv_sec*1000000 + sTimeNow.tv_usec;
                fprintf(fpCmpDisLog, "####one cmp cost us time(us) = %u\n", uTime2-uTime1);
                fflush(fpCmpDisLog);
            #endif
*/
                if (fMatchRes >= g_fMatchAreaPercent)
                {
                    nCount++;
                    break;
                }
            }
            /*
            if (j > nSize2 && 0 == nCount)  //该轮前一半完全没没匹配，则执行下一轮
                break;
            j++;
            */
            iter2++;
            if (iter2 != map2.end() && nSize2 >= 13)    //没有到尾部，且总数大于12，则跳
            {
                iter2++;
            }
        }
        /*
        if (i > nSize/2 && 0 == nCount) //前一半完全没匹配，则不再比对
            break;
        if (i > nSize/2 && nCount == i)
        {
            nCount = nSize; //前一半完全匹配，则认为匹配
            break;
        }
        i++;
        */
        iter++;
        if (iter != map1.end() && nSize >= 13)    //没有到尾部，且总数大于12，则跳
        {
            iter++;
        }
    }
    if (nSize >= 13)
    {
        nSize = nSize/2 + nSize%2;
    }

    float fPercent = 0;
    fPercent = (float)nCount/(float)nSize;
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpCmpDisLog, "############nCount=%d########nSize=%d######fPercent=%f#######g_fSynGoodPercent=%f\n", nCount, nSize, fPercent, g_fSynGoodPercent);
    fflush(fpCmpDisLog);
#endif
    return (fPercent >= g_fSynGoodPercent);
}

/*
* 函数介绍：计算两个rect的重合比例
* 输入参数：r1-rect1；r2-rect2
* 输出参数：无
* 返回值 ：重合面积占rect2面积的比例
*/
float mvCSynchronization::mvRectMatchPercent(const CvRect &r1, const CvRect &r2)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvRectMatchPercent()\n");
#endif
/*	CvRect ru;

    int lx = max(r1.x, r2.x);
	int rx = min(r1.x+r1.width-1, r2.x+r2.width-1);
	int ty = max(r1.y, r2.y);
	int by = min(r1.y+r1.height-1, r2.y+r2.height-1);

	if (rx <= lx || ty >= by)
	{
		return 0;
	}

	ru.x = lx;
	ru.y = ty;
	ru.width = rx - lx + 1;
	ru.height = by - ty + 1;

	float fCount = 0;
	fCount = (float)(ru.width*ru.height)/(float)(r2.width*r2.height);
*/
    float x = max(r1.x, r2.x);
	float y = max(r1.y, r2.y);
	float width = min(r1.x+r1.width-x, r2.x+r2.width-x);
	float height = min(r1.y+r1.height-y, r2.y+r2.height-y);

	if (width <= 0.00001 || height <= 0.00001)
	{
		return 0;
	}

	float fCount = (float)(width*height)/(float)(r2.width*r2.height);

#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpCmpDisLog, "fCount = %f\n\n", fCount);
    fflush(fpCmpDisLog);
#endif
	return fCount;
}

/*
* 函数介绍：计算两组轨迹重合率
* 输入参数：track1-轨迹组1；track2-轨迹组2；matrix-转换矩阵；nLorR-左同步或右同步
* 输出参数：无
* 返回值 ：重合率
*/
float mvCSynchronization::mvGetMatchPercent(TRACK_GROUP track1, TRACK_GROUP track2, const CvMat& matrix, int nLorR)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvGetMatchPercent()\n");
#endif
	CvMat* arr1 = cvCreateMat(4, 3, CV_32FC1);
	CvMat* arr2 = cvCreateMat(4, 3, CV_32FC1);

	if (NULL == arr1 || NULL == arr2)
	{
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpCmpDisLog, "?????????????????NULL == arr1 || NULL == arr2????????????\n");
        fflush(fpCmpDisLog);
    #endif
	    return 0;
	}

	CvRect rect1, rect2;
	float fMinX, fMinY, fMaxX, fMaxY;

    int i = 0;
	TRACK_GROUP::iterator it = track1.begin();
	for(; it != track1.end(); it++,i++)
	{
		CvPoint pt = (CvPoint)*it;
		arr1->data.fl[i*arr1->width] = (float)pt.x;
		arr1->data.fl[i*arr1->width+1] = (float)pt.y;
		arr1->data.fl[i*arr1->width+2] = (float)1.f;
	}

	cvMatMul(arr1, &matrix, arr2);

    i = 0;
    fMinX = arr2->data.fl[i*arr2->width];
    fMinY = arr2->data.fl[i*arr2->width+1];
    fMaxX = arr2->data.fl[i*arr2->width];
    fMaxY = arr2->data.fl[i*arr2->width+1];
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpCmpDisLog, "===================Receive track after convert, %f\t%f\n", fMinX, fMinY);
    fflush(fpCmpDisLog);
#endif

    for (i = 1; i < 4; i++)
    {
        float fX = arr2->data.fl[i*arr2->width];
        float fY = arr2->data.fl[i*arr2->width+1];
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpCmpDisLog, "===================Receive track after convert, %f\t%f\n", fX, fY);
        fflush(fpCmpDisLog);
    #endif

        fMinX = min(fMinX, fX);
        fMinY = min(fMinY, fY);
        fMaxX = max(fMaxX, fX);
        fMaxY = max(fMaxY, fY);
    }
    if (1 == nLorR)
    {
        fMaxX += 0.2*fMaxX;
    }
    else if (2 == nLorR)
    {
        fMinX -= 0.2*fMinX;
    }
	rect1.x = fMinX;
	rect1.y = fMinY;
	rect1.width = fMaxX-fMinX+1;
	rect1.height = fMaxY-fMinY+1;

	it = track2.begin();
	fMinX = (float)(it->x);
	fMinY = (float)(it->y);
	fMaxX = (float)(it->x);
	fMaxY = (float)(it->y);
	it++;
#ifdef PRINT_SYN_PROCESS_LOG
    fprintf(fpCmpDisLog, "===================My own track, %f\t%f\n", fMinX, fMinY);
    fflush(fpCmpDisLog);
#endif

	for(; it != track2.end(); it++)
	{
	    float fX = (float)(it->x);
	    float fY = (float)(it->y);
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpCmpDisLog, "===================My own track, %f\t%f\n", fX, fY);
        fflush(fpCmpDisLog);
    #endif

        fMinX = min(fMinX, fX);
        fMinY = min(fMinY, fY);
        fMaxX = max(fMaxX, fX);
        fMaxY = max(fMaxY, fY);
	}
    if (1 == nLorR)
    {
        fMinX -= 0.2*fMinX;
    }
    else if (2 == nLorR)
    {
        fMaxX += 0.2*fMaxX;
    }
	rect2.x = fMinX;
	rect2.y = fMinY;
	rect2.width = fMaxX-fMinX+1;
	rect2.height = fMaxY-fMinY+1;
    if (rect2.width <= 0.00001 || rect2.height <= 0.00001)
	{
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpCmpDisLog, "?????????????????rect2.width <= 0.00001 || rect2.height <= 0.00001????????????\n");
        fflush(fpCmpDisLog);
    #endif
	    return 0;
	}

    float x = max(rect1.x, rect2.x);
	float y = max(rect1.y, rect2.y);
	float width = min(rect1.x+rect1.width-x, rect2.x+rect2.width-x);
	float height = min(rect1.y+rect1.height-y, rect2.y+rect2.height-y);

	if (width <= 0.00001 || height <= 0.00001)
	{
    #ifdef PRINT_SYN_PROCESS_LOG
        fprintf(fpCmpDisLog, "?????????width = %f, height = %f????????????\n", width, height);
        fflush(fpCmpDisLog);
    #endif
	    return 0;
	}

	cvReleaseMat(&arr1);
	cvReleaseMat(&arr2);

    float fAreaPer = (float)(width*height)/(float)(rect2.width*rect2.height);
	return fAreaPer;
}

/*
* 函数介绍：判断记录是否满足发送要求
* 输入参数：strRec-待发送的记录
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvCanBeSend(string& strRec)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvCanBeSend()\n");
#endif
    SRIP_DETECT_HEADER* detHeader = (SRIP_DETECT_HEADER*)strRec.c_str();
    if (NULL == detHeader)
        return false;

    string strCharData("");
    int nHeaderLength = 0;

    if (MIMAX_EVENT_REP == detHeader->uDetectType)
        nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_EVENT);
    else if (MIMAX_PLATE_REP == detHeader->uDetectType)
        nHeaderLength = sizeof(SRIP_DETECT_HEADER) + sizeof(RECORD_PLATE);
    else //will never happen.
    {
        printf("\nPush character record type error! type = 0x%08x\n", detHeader->uDetectType);
        return false;
    }

    CHAR_VAR varChar;
    memcpy(&varChar, strRec.c_str()+nHeaderLength, sizeof(CHAR_VAR));

    if (1 == varChar.nDone)   //经过比对处理
    {
        strRec.erase((size_t)nHeaderLength, strRec.size()-nHeaderLength);
        return true;
    }

    struct timeval sTimeNow;
    gettimeofday(&sTimeNow, NULL);
    if ((sTimeNow.tv_sec - varChar.uTimeStamp) >= MAX_TIME_IN_QUEUE) //超时
    {
        strRec.erase((size_t)nHeaderLength, strRec.size()-nHeaderLength);
        return true;
    }
    else
        return false;
}

/*
* 函数介绍：计算转换矩阵
* 输入参数：tGroupA-标定点1；tGroupB-标定点2；matrix-存储转换矩阵的变量
* 输出参数：matrix-计算出的转换矩阵
* 返回值 ：无
*/
void mvCSynchronization::mvOnComputeMatrix(TRACK_GROUP& tGroupA, TRACK_GROUP& tGroupB, CvMat& matrix)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnComputeMatrix()\n");
#endif
    if (tGroupA.size() != 4 || tGroupB.size() != 4)
    {
        printf("计算矩阵所需标定输入有误!!\n");
        return;
    }

	CvMat *arr1 = cvCreateMat(4, 3, CV_32FC1);
	CvMat *arr2 = cvCreateMat(4, 3, CV_32FC1);

    TRACK_GROUP::iterator it = tGroupA.begin();
	for (int i = 0; it != tGroupA.end(); it++,i++)
	{
		CvPoint pt = (CvPoint)*it;
// 		((float *)(arr1->data.ptr+arr1->step*i))[0] = (float)pt.x;
// 		((float *)(arr1->data.ptr+arr1->step*i))[1] = (float)pt.x;
// 		((float *)(arr1->data.ptr+arr1->step*i))[2] = 1.f;
		arr1->data.fl[i*arr1->width] = (float)pt.x;
		arr1->data.fl[i*arr1->width+1] = (float)pt.y;
		arr1->data.fl[i*arr1->width+2] = 1.f;
	}

    it = tGroupB.begin();
	for (int i = 0; it != tGroupB.end(); it++,i++)
	{
		CvPoint pt = (CvPoint)*it;
// 		((float *)(arr2->data.ptr+arr2->step*i))[0] = (float)pt.x;
// 		((float *)(arr2->data.ptr+arr2->step*i))[1] = (float)pt.x;
// 		((float *)(arr2->data.ptr+arr2->step*i))[2] = 1.f;
		arr2->data.fl[i*arr2->width] = (float)pt.x;
		arr2->data.fl[i*arr2->width+1] = (float)pt.y;
		arr2->data.fl[i*arr2->width+2] = 1.f;
	}

    //printf("开始矩阵计算...\n");
    cvSolve(arr1, arr2, &matrix, CV_SVD);

#ifdef PRINT_SYN_DEBUG_MSG
    printf("求得矩阵为：\n");
    for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			printf("%f\t", matrix.data.fl[i*matrix.width+j]);
		}
		printf("\n");
	}
    printf("\n");
#endif

    cvReleaseMat(&arr1);
	cvReleaseMat(&arr2);
}

/*
* 函数介绍：处理接收到的左边主机MAC
* 输入参数：strMsg-包含左边主机MAC的整个消息
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvDealLeftMacAddress(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvDealLeftMacAddress()\n");
#endif
    //SRIP_HEADER* sHeader = (SRIP_HEADER*)strMsg.c_str();
    //printf("sHeader->uMsgCommandID=%08x\nsHeader->uMsgLen=%u\nsHeader->uMsgSource=%u\n", sHeader->uMsgCommandID, sHeader->uMsgLen, sHeader->uMsgSource);

    string strRecvMac("");
    //printf("strMsg.size()-sizeof(SRIP_HEADER)=%d\n", strMsg.size()-sizeof(SRIP_HEADER));
    strRecvMac.append(strMsg.c_str()+sizeof(SRIP_HEADER), strMsg.size()-sizeof(SRIP_HEADER));
    printf("Recv left mac-addr, strRecvMac=%s\n", strRecvMac.c_str());

    if (strRecvMac.size() != 12)
    {
        printf("接收到左边的MAC地址长度有误，要求重发!\n");
        return mvSend(m_nPassiveSocket, RE_SEND_MAC_ADDRESS);
    }
    if (m_strLeftMacAddress.empty())
    {
        m_strLeftMacAddress = strRecvMac;
        return mvStoreLeftMacAddress();
    }
    if (strncmp(m_strLeftMacAddress.c_str(), strRecvMac.c_str(), MAC_ADDR_LENGTH) != 0)
    {
        printf("接受到的MAC和存储MAC不匹配，关闭左连接！m_strLeftMacAddress=%s\tstrRecvMac=%s\n", m_strLeftMacAddress.c_str(), strRecvMac.c_str());
        //g_bLeftLink = false;
        mvMyClosePassiveSocket();
        return false;
    }

    m_bRecvLeftMac = true;
    return true;
}

/*
* 函数介绍：处理左边主机发送过来的标定点
* 输入参数：strMsg-包含左边主机标定点的整个消息
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvDealLeftMarkerGroup(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvDealLeftMarkerGroup()\n");
#endif
    if (strMsg.empty())
    {
        printf("\n接收到的左标定消息为空，要求重发!\n");
        return mvSend(m_nPassiveSocket, RE_SEND_MARKER_GROUP);
        //g_bLeftLink = false;
        //mvMyClosePassiveSocket();
        //return false;
    }

    if (m_sLeftTrack.size() != 4)
    {
        printf("\n自身左标定未读成功，不能进行左同步，关闭左连接! m_sLeftTrack.size()=%d\n\n", (int)m_sLeftTrack.size());
        //g_bLeftLink = false;
        mvMyClosePassiveSocket();
        return false;
    }
/*
    string strRecvLeftMarker("");
    strRecvLeftMarker.append(strMsg.c_str()+sizeof(SRIP_HEADER), strMsg.size()-sizeof(SRIP_HEADER));

    if (strRecvLeftMarker.empty())
    {
        printf("\n接收到的左标定数据为空，要求重发!\n");
        return mvSend(m_nPassiveSocket, RE_SEND_MARKER_GROUP);
    }
*/
	int nOffset = sizeof(SRIP_HEADER);
	int nOffset2 = strMsg.size() - (sizeof(int)*2+1);
	string strLeftAreaY(""), strLeftMaker("");
	strLeftAreaY.append(strMsg.c_str()+nOffset2+1, (sizeof(int)*2+1));
	strLeftMaker.append(strMsg.c_str(), nOffset2);

    CSkpRoadXmlValue xmlLeftMarker(strLeftMaker, &nOffset);

    if (xmlLeftMarker.size() == 0)
    {
        printf("\n接收到的左标定数据为空，要求重发!\n");
        return mvSend(m_nPassiveSocket, RE_SEND_MARKER_GROUP);
    }

    printf("\n接收到的左标定点个数为%d\n", xmlLeftMarker.size());
    TRACK_GROUP recvLeftTrack;
    for (int i = 0; i < 4; i++)
    {
        CvPoint point;
        point.x = xmlLeftMarker[i]["Point"][0]["x"];
        point.y = xmlLeftMarker[i]["Point"][0]["y"];
        recvLeftTrack.push_back(point);
    }
    xmlLeftMarker.clear();

#ifdef PRINT_SYN_DEBUG_MSG
    TRACK_GROUP::iterator iterRecvLeft = recvLeftTrack.begin();
    TRACK_GROUP::iterator iterMyLeft = m_sLeftTrack.begin();
    for (int i = 0; i < 4; i++)
    {
        printf("recvLeftTrack[%d].x=%d,recvLeftTrack[%d].y=%d\n", i, iterRecvLeft->x, i, iterRecvLeft->y);
        printf("m_sLeftTrack[%d].x=%d,m_sLeftTrack[%d].y=%d\n", i, iterMyLeft->x, i, iterMyLeft->y);
        iterRecvLeft++;
        iterMyLeft++;
    }
    printf("\n计算左矩阵...\n");
#endif

    mvOnComputeMatrix(recvLeftTrack, m_sLeftTrack, *m_pLeftMatrix);

    if (m_pLeftMatrix != NULL)
    {
        int nLeftAreaTop, nLeftAreaBottom;
        if (strLeftAreaY.size() == (sizeof(int)*2+1))
        {
            printf("strLeftAreaY.c_str()=%s\n", strLeftAreaY.c_str());
            sscanf(strLeftAreaY.c_str(), "%d,%d", &nLeftAreaTop, &nLeftAreaBottom);
            printf("nLeftAreaTop=%d,nLeftAreaBottom=%d\n", nLeftAreaTop, nLeftAreaBottom);
        }
        mvOnGetLeftAreaY(nLeftAreaTop, nLeftAreaBottom);
        printf("m_nLeftAreaTop=%d,m_nLeftAreaBottom=%d\n", m_nLeftAreaTop, m_nLeftAreaBottom);

        m_bRecvLeftMarker = true;
        return true;
    }
    printf("\n计算得到的左矩阵为空!\n");
    return false;
}

/*
* 函数介绍：处理右边主机发送过来的标定点
* 输入参数：strMsg-包含右边主机标定点的整个消息
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvDealRightMarkerGroup(const string& strMsg)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvDealRightMarkerGroup()\n");
#endif
    if (strMsg.empty())
    {
        printf("\n接收到的右标定消息为空，要求重发!\n");
        return mvSend(m_nActiveSocket, RE_SEND_MARKER_GROUP);
        //g_bRightLink = false;
        //mvMyCloseActiveSocket();
        //return false;
    }

    if (m_sRightTrack.size() != 4)
    {
        printf("\n自身右标定未读成功，不能进行右同步，关闭右连接!\n");
        //g_bRightLink = false;
        mvMyCloseActiveSocket();
        return false;
    }

	int nOffset = sizeof(SRIP_HEADER);
	int nOffset2 = strMsg.size() - (sizeof(int)*2+1);
	string strRightAreaY(""), strRightMaker("");
	strRightAreaY.append(strMsg.c_str()+nOffset2+1, (sizeof(int)*2+1));
	strRightMaker.append(strMsg.c_str(), nOffset2);

    CSkpRoadXmlValue xmlRightMarker(strRightMaker, &nOffset);

    if (xmlRightMarker.size() == 0)
    {
        printf("\n接收到的左标定数据为空，要求重发!\n");
        return mvSend(m_nPassiveSocket, RE_SEND_MARKER_GROUP);
    }

    printf("\n接收到的右标定点个数为%d\n", xmlRightMarker.size());
    TRACK_GROUP recvRightTrack;

    for (int i = 0; i < 4; i++)
    {
        CvPoint point;
        point.x = xmlRightMarker[i]["Point"][0]["x"];
        point.y = xmlRightMarker[i]["Point"][0]["y"];
        recvRightTrack.push_back(point);
    }
    xmlRightMarker.clear();

#ifdef PRINT_SYN_DEBUG_MSG
    //打印表定点
    TRACK_GROUP::iterator iterRecvRight = recvRightTrack.begin();
    TRACK_GROUP::iterator iterMyRight = m_sRightTrack.begin();
    for (int i = 0; i < 4; i++)
    {
        printf("recvRightTrack[%d].x=%d,recvRightTrack[%d].y=%d\n", i, iterRecvRight->x, i, iterRecvRight->y);
        printf("m_sRightTrack[%d].x=%d,m_sRightTrack[%d].y=%d\n", i, iterMyRight->x, i, iterMyRight->y);
        iterRecvRight++;
        iterMyRight++;
    }
    printf("\n计算右矩阵...\n");
#endif

    mvOnComputeMatrix(recvRightTrack, m_sRightTrack, *m_pRightMatrix);

    if (m_pRightMatrix != NULL)
    {
        int nRightAreaTop, nRightAreaBottom;
        if (strRightAreaY.size() == (sizeof(int)*2+1))
        {
            printf("strRightAreaY.c_str()=%s\n", strRightAreaY.c_str());
            sscanf(strRightAreaY.c_str(), "%d,%d", &nRightAreaTop, &nRightAreaBottom);
            printf("nRightAreaTop=%d,nRightAreaBottom=%d\n", nRightAreaTop, nRightAreaBottom);
        }
        mvOnGetRightAreaY(nRightAreaTop, nRightAreaBottom);
        printf("m_nRightAreaTop=%d,m_nRightAreaBottom=%d\n", m_nRightAreaTop, m_nRightAreaBottom);

        m_bRecvRightMarker = true;
        return true;
    }
    printf("\n计算得到的右矩阵为空!\n");
    return false;
}

/*
* 函数介绍：把本机MAC地址发往右边主机
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvSendMacToRight()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvSendMacToRight()\n");
#endif
    printf("\n把自己的MAC地址发往右边主机，m_strMyMacAddress=%s\n", m_strMyMacAddress.c_str());
    return mvSend(m_nActiveSocket, SEND_MAC_ADDRESS, m_strMyMacAddress);
    /*
    string strTest("");
    for (int i = 0; i < 6; i++)
    {
        printf("%.2x", m_szMyMacAddr[i]);
        u_char* pBuff = (u_char*)malloc(2*sizeof(u_char));
        sprintf((char*)pBuff, "%.2x", m_szMyMacAddr[i]);
        strTest.append((char*)pBuff, 2*sizeof(u_char));
        free(pBuff);
    }
    printf("My MAC-ADDR is: %s\n", strTest.c_str());
    string strMyMac("");
    strMyMac.append((char*)m_szMyMacAddr, 6*sizeof(u_char));
    //return mvSend(m_nActiveSocket, SEND_MAC_ADDRESS, strMyMac);
    return mvSend(m_nActiveSocket, SEND_MAC_ADDRESS, strTest);
    */
}

/*
* 函数介绍：把本机左标定点发往左侧主机
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvSendMarkerToLeft()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvSendMarkerToLeft()\n");
#endif
    if (m_sLeftTrack.size() != 4)
    {
        printf("\n自身左标定为有误!m_sLeftTrack.size()=%d\n", (int)m_sLeftTrack.size());
        return false;
    }
    string strLeftMarker("");
    CSkpRoadXmlValue leftMarkerXml;
    TRACK_GROUP::iterator iter = m_sLeftTrack.begin();
    for (int i = 0; iter != m_sLeftTrack.end(); iter++,i++)
    {
        leftMarkerXml[i]["Point"][0]["x"] = iter->x;
        leftMarkerXml[i]["Point"][0]["y"] = iter->y;
    }

    strLeftMarker = leftMarkerXml.toXml();
    leftMarkerXml.clear();
    if (strLeftMarker.empty())
        return false;

    char szAreaY[sizeof(int)*2 + 1] = {0};
    sprintf(szAreaY, "%d,%d", m_nAreaY[0], m_nAreaY[1]);
    strLeftMarker += szAreaY;
    return mvSend(m_nPassiveSocket, SEND_MARKER_GROUP, strLeftMarker);
}

/*
* 函数介绍：把本机右标定点发往右侧主机
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，失败返回false
*/
bool mvCSynchronization::mvSendMarkerToRight()
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvSendMarkerToRight()\n");
#endif
    if (m_sRightTrack.size() != 4)
    {
        printf("\n自身右标定为有误!m_sRightTrack.size()=%d\n", (int)m_sRightTrack.size());
        return false;
    }
    string strRightMarker("");
    CSkpRoadXmlValue rightMarkerXml;
    TRACK_GROUP::iterator iter = m_sRightTrack.begin();
    for (int i = 0; iter != m_sRightTrack.end(); iter++,i++)
    {
        rightMarkerXml[i]["Point"][0]["x"] = iter->x;
        rightMarkerXml[i]["Point"][0]["y"] = iter->y;
    }

    strRightMarker = rightMarkerXml.toXml();
    rightMarkerXml.clear();
    if (strRightMarker.empty())
        return false;

    char szAreaY[sizeof(int)*2 + 1] = {0};
    sprintf(szAreaY, "%d,%d", m_nAreaY[0], m_nAreaY[1]);
    strRightMarker += szAreaY;
    return mvSend(m_nActiveSocket, SEND_MARKER_GROUP, strRightMarker);
}

/*
* 函数介绍：计算左边检测区域最高点和最低点Y坐标映射
* 输入参数：左侧最高点Y坐标原值，左侧最低点Y坐标原值，
* 输出参数：无
* 返回值 ：无
*/
void mvCSynchronization::mvOnGetLeftAreaY(int nLeftY1, int nLeftY2)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnGetLeftAreaY()\n");
#endif

	CvMat* arr1 = cvCreateMat(2, 3, CV_32FC1);
	CvMat* arr2 = cvCreateMat(2, 3, CV_32FC1);

	if (NULL == arr1 || NULL == arr2)
	{
	    printf("cvCreateMat error!\n");
	    return;
	}

    int i = 0;
    for (; i < 2; i++)
    {
        arr1->data.fl[i*arr1->width] = 0.f;
        if (0 == i)
        {
            arr1->data.fl[i*arr1->width+1] = (float)nLeftY1;
        }
        else
        {
            arr1->data.fl[i*arr1->width+1] = (float)nLeftY2;
        }
        arr1->data.fl[i*arr1->width+2] = 1.f;
    }

    printf("before cvMatMul(arr1, &m_pLeftMatrix, arr2);\n");
	cvMatMul(arr1, m_pLeftMatrix, arr2);
    printf("after cvMatMul(arr1, &m_pLeftMatrix, arr2);\n");

    m_nLeftAreaTop = arr2->data.fl[1];
    m_nLeftAreaBottom = arr2->data.fl[arr2->width+1];

	cvReleaseMat(&arr1);
	cvReleaseMat(&arr2);
}

/*
* 函数介绍：计算右边检测区域最高点和最低点Y坐标映射
* 输入参数：右侧最高点Y坐标原值，右侧最低点Y坐标原值，
* 输出参数：无
* 返回值 ：无
*/
void mvCSynchronization::mvOnGetRightAreaY(int nRightY1, int nRightY2)
{
#ifdef SYN_STUPID_LOG
    SSLog("In mvCSynchronization::mvOnGetRightAreaY()\n");
#endif

	CvMat* arr1 = cvCreateMat(2, 3, CV_32FC1);
	CvMat* arr2 = cvCreateMat(2, 3, CV_32FC1);

	if (NULL == arr1 || NULL == arr2)
	{
	    printf("cvCreateMat error!\n");
	    return;
	}

    int i = 0;
    for (; i < 2; i++)
    {
        arr1->data.fl[i*arr1->width] = 0.f;
        if (0 == i)
        {
            arr1->data.fl[i*arr1->width+1] = (float)nRightY1;
        }
        else
        {
            arr1->data.fl[i*arr1->width+1] = (float)nRightY2;
        }
        arr1->data.fl[i*arr1->width+2] = 1.f;
    }

    printf("before cvMatMul(arr1, &m_pRightMatrix, arr2);\n");
	cvMatMul(arr1, m_pRightMatrix, arr2);
    printf("after cvMatMul(arr1, &m_pRightMatrix, arr2);\n");

    m_nRightAreaTop = arr2->data.fl[1];
    m_nRightAreaBottom = arr2->data.fl[arr2->width+1];

	cvReleaseMat(&arr1);
	cvReleaseMat(&arr2);
}
