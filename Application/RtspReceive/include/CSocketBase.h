#ifndef _MVCSOCKETBASE_H
#define _MVCSOCKETBASE_H

#include "global.h"

class mvCSocketBase
{
    public:
        //构造函数
        mvCSocketBase();
        //析构函数
        ~mvCSocketBase();

    public:
        //创建套接字
        bool    mvCreateSocket(int &nSocket, int nType = 1);
        //为套接字绑定端口
        bool    mvBindPort(int nSocket, int nPort);
        //设置套接字属性
        bool    mvSetSocketOpt(int nSocket, int nOpt);
        //开始监听
        bool    mvStartListen(int nSocket);
        //主动连接
        bool    mvWaitConnect(int nSocket, const string& strHost, int nPort, int nSec = 0);
        //发送消息
        bool    mvSendMsgToSocket(int nSocket, const string& strMsg,bool bBlock = false);
        //关闭套接字
        void    mvCloseSocket(int &nSocket);

		#ifndef WIN32
        //回收线程
        void    mvJoinThread(pthread_t& uThreadId);
		#endif

        //设置套接字是否阻塞
        void    mvSetSocketBlock(int &nSocket, bool bBlock = true);
        //准备套接字
        bool    mvPrepareSocket(int& nSocket);

    protected:
        //等待套接字可写
        bool    mvWaitToWriteSocket(int nSocket);

    private:
  
		#ifdef WIN32
			CRITICAL_SECTION m_SendMutex;
		#else
			pthread_mutex_t m_SendMutex;
		#endif
};

#endif