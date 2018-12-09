#include "Common.h"
#include "CSocketBase.h"

//#define SOCKET_BASE_LOG

#ifdef SOCKET_BASE_LOG
void SBLog(const char *pLog)
{
    FILE *pSBLog = fopen("socket_base.log", "a");
    if (pSBLog != NULL)
    {
        fprintf(pSBLog, pLog);
        fflush(pSBLog);
        fclose(pSBLog);
    }
}
#endif  //SOCKET_BASE_LOG

/*
* 函数介绍：构造函数，初始化线程锁
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCSocketBase::mvCSocketBase()
{
    pthread_mutex_init(&m_SendMutex, NULL);
}

/*
* 函数介绍：析构函数，销毁线程锁
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCSocketBase::~mvCSocketBase()
{
    pthread_mutex_destroy(&m_SendMutex);
}

/*
* 函数介绍：创建套接字
* 输入参数：nSocket-要创建的套接字变量；nType-要创建的套接字类型
* 输出参数：nSocket-创建的套接字值
* 返回值 ：成功返回true，否则false
*/
bool mvCSocketBase::mvCreateSocket(int& nSocket, int nType) //1:tcp; 2:udp
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvCreateSocket()\n");
#endif
    if (1 == nType)
    {
        nSocket = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (2 == nType)
    {
        nSocket = socket(AF_INET, SOCK_DGRAM, 0);
    }

    return (nSocket > 0);
}

/*
* 函数介绍：准备套接字，包括创建、绑定端口、设置选项等
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSocketBase::mvPrepareSocket(int& nSocket)
{
    //先关闭套接字
    mvCloseSocket(nSocket);
    //在创建新套接字
    if (!mvCreateSocket(nSocket, 1))
    {
        return false;
    }
    //设置套接字可重复使用
    if (!mvSetSocketOpt(nSocket, SO_REUSEADDR))
    {
        return false;
    }

    return true;
}

/*
* 函数介绍：为套接字绑定端口
* 输入参数：nSocket-要绑定端口的套接字；nPort-端口
* 输出参数：无
* 返回值 ：成功返回true，否则false
*/
bool mvCSocketBase::mvBindPort(int nSocket, int nPort)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvBindPort()\n");
#endif
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;

    servaddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(servaddr.sin_zero), 8);
    servaddr.sin_port = htons(nPort);

    if (bind(nSocket, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1)
    {
        return false;
    }

    return true;
}

/*
* 函数介绍：设置套接字选项
* 输入参数：nSocket-要设置的套接字；nOpt-选项
* 输出参数：无
* 返回值 ：成功返回true，否则false
*/
bool mvCSocketBase::mvSetSocketOpt(int nSocket, int nOpt)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvSetSocketOpt()\n");
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
	else if (SO_RCVTIMEO == nOpt)
    {
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        return (setsockopt(nSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval)) == 0);
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
    {
        return false;
    }
}

/*
* 函数介绍：开始侦听
* 输入参数：nSocket-已绑定侦听端口的套接字
* 输出参数：无
* 返回值 ：成功返回true，否则false
*/
bool mvCSocketBase::mvStartListen(int nSocket)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvStartListen()\n");
#endif
    if (listen(nSocket, 10) == -1)
    {
        return false;
    }

    return true;
}

/*
* 函数介绍：主动连接
* 输入参数：nSocket-创建并设置好选项的套接字；strHost-要连接的IP；nPort-要连接的端口；nSec-超时时间
* 输出参数：无
* 返回值 ：成功返回true，否则false
*/
bool mvCSocketBase::mvWaitConnect(int nSocket, const string& strHost, int nPort, int nSec)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvWaitConnect()\n");
#endif
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(strHost.c_str());
    servaddr.sin_port = htons(nPort);

    if(nSec > 0)
    {
        struct timeval timeo;
        socklen_t len = sizeof(timeo);
        timeo.tv_sec= nSec;
        timeo.tv_usec=0;//设置超时

        //设置连接超时
        if(setsockopt(nSocket, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
        {
            perror("setsockopt");
            return false;
        }
    }

    if ((connect(nSocket, (struct sockaddr*)&servaddr, sizeof(servaddr))) == 0) //block
    {
        return true;
    }
    return false;
}

/*
* 函数介绍：发送消息
* 输入参数：nSocket-要发送消息的套接字；strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则false
*/
bool mvCSocketBase::mvSendMsgToSocket(int nSocket, const string& strMsg,bool bBlock)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvSendMsgToSocket()\n");
#endif
    if (nSocket <= 0)
    {
        return false;
    }

    int nLeft = strMsg.size();
    int nBytes = 0;
    int nDataLen = 0;

    pthread_mutex_lock(&m_SendMutex);
    while (nLeft > 0)
    {
        if(!bBlock)
        {
            if (!mvWaitToWriteSocket(nSocket))
            {
                    pthread_mutex_unlock(&m_SendMutex);
                    LogError("mvWaitToWriteSocket==nLeft=%d,nDataLen=%d,%s\n",nLeft,nDataLen,strerror(errno));
                    return false;
            }
        }

        if ((nBytes = send(nSocket, strMsg.c_str()+nDataLen, nLeft>16384?16384:nLeft, MSG_NOSIGNAL)) < 0)
        {
            pthread_mutex_unlock(&m_SendMutex);
           // LogError("send error===nSocket=%d==nLeft=%d,nDataLen=%d,%s\n",nSocket, nLeft,nDataLen,strerror(errno));
            return false;
        }
        nDataLen += nBytes;
        nLeft -= nBytes;
    }
    pthread_mutex_unlock(&m_SendMutex);

    return true;
}


/*
* 函数介绍：关闭套接字
* 输入参数：nSocket-要关闭的套接字
* 输出参数：无
* 返回值 ：无
*/
void mvCSocketBase::mvCloseSocket(int &nSocket)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvCloseSocket()\n");
#endif
    if (nSocket > 0)
    {
        shutdown(nSocket, 2);
        close(nSocket);
        nSocket = 0;
    }
}

/*
* 函数介绍：回收线程
* 输入参数：uThreadId-要回收的线程ID
* 输出参数：uThreadId-回收完成，把要回收的线程ID设为0
* 返回值 ：无
*/
void mvCSocketBase::mvJoinThread(pthread_t& uThreadId)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvJoinThread()\n");
#endif
    if (pthread_equal(uThreadId, (pthread_t)0) != 0)    //return non-zero value if equal
    {
        return;
    }

    int nRes = pthread_kill(uThreadId, 0);
    if (ESRCH == nRes)  //已经退出
    {
        uThreadId = (pthread_t)0;
        return;
    }
    else if (EINVAL == nRes)
    {
        uThreadId = (pthread_t)0;
        return;
    }
    else
    {
        pthread_cancel(uThreadId);
        pthread_join(uThreadId, NULL);
        uThreadId = (pthread_t)0;
    }
}

/*
* 函数介绍：等待套接字可写
* 输入参数：nSocket-等待的套接字
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCSocketBase::mvWaitToWriteSocket(int nSocket)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvWaitToWriteSocket()\n");
#endif
    if (nSocket <= 0)
    {
        return false;
    }

    int nCount = 0;
    while (true)
    {
        if (nCount >= 5)
        {
            return false;
        }

        fd_set fds;
        int nMaxfd;

        FD_ZERO(&fds);
        FD_SET(nSocket, &fds);
        nMaxfd = nSocket + 1;

        struct timeval tvTimeOut;
        tvTimeOut.tv_sec = 1;
        tvTimeOut.tv_usec = 0;

   //     FD_ZERO(&fds);
   //     FD_SET(nSocket, &fds);

        //LogError("before select nSocket=%d\n",nSocket);
        int nRes = select(nMaxfd, NULL, &fds, NULL, &tvTimeOut);

        if (nRes > 0)
        {
            if (FD_ISSET(nSocket, &fds))
            {
                return true;
            }
            else
            {
                LogError("FD_ISSET false\n");
            }
        }
        else if (nRes == 0)//发送超时
        {
            LogError("发送超时nSocket=%d,error =%s\n",nSocket,strerror(errno));
        }
        else //发送失败
        {
            LogError("select nRes=%d,nSocket=%d,error =%s\n",nRes,nSocket,strerror(errno));
        }

        usleep(100);
        nCount++;
    }
}

/*
* 函数介绍：设置套接字是否阻塞
* 输入参数：nSocket-要设置的套接字；bBlock-是否阻塞
* 输出参数：无
* 返回值 ：无
*/
void mvCSocketBase::mvSetSocketBlock(int &nSocket, bool bBlock)
{
#ifdef SOCKET_BASE_LOG
    SBLog("In mvCSocketBase::mvSetSocketBlock()\n");
#endif
    int nValue;

    if ((nValue = fcntl(nSocket, F_GETFL, 0)) < 0)
    {
        return ;
    }

    if (!bBlock)
    {
        nValue |= O_NONBLOCK;   // set non-block.
    }
    else
    {
        nValue &= ~O_NONBLOCK;  // set block.
    }

    if (fcntl(nSocket, F_SETFL, nValue) < 0)
    {
        return ;
    }
}
