/**
 **	File ......... Thread.cpp
 **	Published ....  2004-10-30
 **	Author ....... grymse@alhem.net
**/
/*
Copyright (C) 2004  Anders Hedstrom

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <stdio.h>
#ifdef _WIN32
#include "socket_include.h"
#else
#include <unistd.h>
#endif

#include "Thread.h"
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

Thread::Thread(bool release)
:m_thread(0)
,m_running(true)
,m_release(false)
{
	pthread_mutex_init(&m_SyncExitLock, NULL);
#ifdef _WIN32
	m_thread = ::CreateThread(NULL, 0, StartThread, (void * )this, 0, &m_dwThreadId);
#else
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int ret = pthread_attr_setstacksize(&attr,1024*1024*4);
        if(ret != 0)
        {
//                CStUtilImp::m_gpVisLog->Warning(MODULE_SRCTODEST,LOGLEVEL_ADMIN,"Creat Thread faild!");
        }

	if (pthread_create(&m_thread,&attr,StartThread,this) == -1)
	{
//		CStUtilImp::m_gpVisLog->Warning(MODULE_SRCTODEST,LOGLEVEL_ADMIN,"Creat Thread faild!");
		SetRunning(false);
	}
	pthread_attr_destroy(&attr);
#endif
//	ThNum++;
//	printf("new th num = %d\n",ThNum);
	m_release = release;
}


Thread::~Thread()
{
	printf("*********************Thread\n");
//	while (m_running || m_thread)
	if (m_running)
	{
		SetRunning(false);
		SetRelease(true);

#ifdef _WIN32
		myMsleep(100);
		::CloseHandle(m_thread);
#else
		//mySsleep(1);
#endif
	}
	pthread_mutex_destroy(&m_SyncExitLock);
}

void Thread::StopThread()
{
	printf("*********************StopThread\n");
	pthread_cancel( m_thread );
}

threadfunc_t STDPREFIX Thread::StartThread(threadparam_t zz)
{
	Thread *pclThread = (Thread *)zz;

	if ( pclThread == NULL ) return NULL;
	while (pclThread -> m_running && !pclThread -> m_release)
	{
#ifdef _WIN32
		myMsleep(100);
#else
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
#endif
	}
	if (pclThread -> m_running)
	{
		pclThread ->LockThreadExit();
		pclThread -> Run();
		pclThread ->UnLockThreadExit();
	}
	pclThread -> SetRunning(false); // if return
	return (threadfunc_t)zz;
}


bool Thread::IsRunning() 
{
 	return m_running; 
}


void Thread::SetRunning(bool x) 
{
	printf("*********************SetRunning\n");
 	m_running = x; 
}


bool Thread::IsReleased() 
{
	printf("*********************IsReleased\n");
 	return m_release; 
}


void Thread::SetRelease(bool x) 
{
 	m_release = x; 
}

void Thread::LockThreadExit()
{
	pthread_mutex_lock(&m_SyncExitLock);
}

void Thread::UnLockThreadExit()
{
	pthread_mutex_unlock(&m_SyncExitLock);
}







