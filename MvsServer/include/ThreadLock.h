#ifndef __THREADLOCK_H
#define __THREADLOCK_H

#ifdef linux
#	include <stdint.h>
#	include <signal.h>
#	include <unistd.h> 
#	include <fcntl.h>
#	include <dirent.h>
#	include <pthread.h>
#	include <sys/time.h>
#	include <sys/file.h>
#	include <sys/socket.h>
#	include <sys/ipc.h>
#	include <sys/shm.h>
#	include <sys/wait.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <sys/param.h>
extern int h_errno;
#endif // linux

#ifdef WIN32
#   define FD_SETSIZE 1024
#   include <winsock2.h>
#	include <sys/locking.h>
#	include <io.h>
#	include <rpc.h>
#endif //WIN32

namespace BASE
{
	class ThreadLock
	{
	public:
		/*
		形式：ThreadLock(void)
		描述：ThreadLock操作是线程锁类的构造函数。
		输入：
			无
		算法或处理：
			初始化锁变量
		输出：无
		返回：
			无
		*/
		ThreadLock(void)
		{
#ifdef WIN32
			InitializeCriticalSection(&criCounter);
#endif
#ifdef linux
			pthread_mutex_init(&fastmutex, NULL);
#endif
		};
		/*
		形式：~ThreadLock(void)
		描述：~ThreadLock是将类的析构函数。
		输入：
			无
		算法或处理：
		    释放申请的临界段变量。
		输出：无
		返回：
			无
		*/
		~ThreadLock(void)
		{
#ifdef WIN32
			DeleteCriticalSection(&criCounter);
#endif
		};

		// 进入锁和退出锁
		/*
		形式：bool Lock()
		描述：Lock是线程进行锁操作的函数。
		输入：
			无
		算法或处理：
		    调用相关系统函数锁定临界区
		输出：无
		返回：
			成功返回：true
			失败返回：false
		*/
		bool Lock()
		{
#ifdef WIN32
			EnterCriticalSection(&criCounter);
			return true;
#endif
#ifdef linux
			return pthread_mutex_lock( &fastmutex ) == 0;
#endif
		};
		/*
		形式：bool UnLock()
		描述：UnLock方法是将线程进行解锁的操作。
		输入：
			无
		算法或处理：
		    调用相关系统函数退出临界区
		输出：无
		返回：
			成功返回：true
			失败返回：false
		*/
		bool UnLock()
		{
#ifdef WIN32
			LeaveCriticalSection(&criCounter);
			return true;
#endif
#ifdef linux
			return pthread_mutex_unlock( &fastmutex ) == 0;
#endif
		};


	private:
#ifdef WIN32
		CRITICAL_SECTION criCounter;	//windows环境下的临界段变量
#endif
#ifdef linux
		pthread_mutex_t fastmutex;		//linux环境下的posix互斥量
#endif
	};

};
#endif //__THREADLOCK_H

