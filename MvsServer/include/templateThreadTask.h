// templateThreadTask.h: interface for the templateThreadTask class.
#include "Thread.h"

//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEMPLATETHREADTASK_H__DC9D4EB1_1956_46FE_BA43_4603F5E4CE7A__INCLUDED_)
#define AFX_TEMPLATETHREADTASK_H__DC9D4EB1_1956_46FE_BA43_4603F5E4CE7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/************************************************************************/
/* 
线程工具类
templateThreadTask继承Thread基类

线程分配任务的管理

*/
/************************************************************************/

template<class T> class templateThreadTask  : public Thread 
{
public:
	templateThreadTask(int iTaskMaxCount = 3);
	virtual ~templateThreadTask();
public:
	void Run();
	bool AddTask(T *pTasker );
	bool RemoveTask(T *pTasker);
	bool StartTask(T *pTasker );
	bool PauseTask(T *pTasker);
	bool IsFull();
 	void CancelThread();
	void TaskCountSyncLck(bool block);
private:
	friend void* T::Working(templateThreadTask<T>* pThis);
	//线程退出标志
	int m_iThreadExitFlag;

	struct TaskFlag
	{
		//标志业务开始运行
		int m_istartaskFg;
		//标志业务已经添加
		int m_iaddtaskFg;
		//标志业务已经删除
		int m_iremovetaskFg;
		//obj数组
		T * m_TaskersPtr;
	};
	struct TaskFlag * m_pTaskerFlag;
	int m_iTaskMaxCount;
	//任务计数
	int m_iCount;
	pthread_mutex_t TaskCountLock;
};

template<class T>
templateThreadTask<T>::templateThreadTask(int iTaskMaxCount): Thread(true)
{
	m_iCount = 0;
	m_iThreadExitFlag = 0;
	m_iTaskMaxCount = iTaskMaxCount;
	m_pTaskerFlag = NULL;
	m_pTaskerFlag = new struct TaskFlag[m_iTaskMaxCount];
	for ( int i = 0; i < m_iTaskMaxCount; i++ )
	{
		(m_pTaskerFlag+i)->m_iaddtaskFg = 0;
		(m_pTaskerFlag+i)->m_istartaskFg = 0;
		(m_pTaskerFlag+i)->m_TaskersPtr = NULL;
		(m_pTaskerFlag+i)->m_iremovetaskFg = 0;
	}
	pthread_mutex_init(&TaskCountLock, NULL);
}
template<class T>
templateThreadTask<T>::~templateThreadTask()
{
	for(int i=0;i<m_iTaskMaxCount;i++)
	{
		if((m_pTaskerFlag+i)->m_TaskersPtr != NULL)
		{
			delete (m_pTaskerFlag+i)->m_TaskersPtr;
			(m_pTaskerFlag+i)->m_TaskersPtr = 0;
		}	
	}
	if(m_pTaskerFlag != NULL)
	{
		delete []m_pTaskerFlag;
		m_pTaskerFlag = NULL;
	}
	pthread_mutex_destroy(&TaskCountLock);	
}

template<class T>
void templateThreadTask<T>::Run()
{
	T::Working(this);
}

template<class T>
bool templateThreadTask<T>::RemoveTask(T *pTasker)
{
	if (pTasker == NULL) return false;
	bool bret = false;

	for (int i = 0;  i < m_iTaskMaxCount; i++)
	{
		if ((m_pTaskerFlag+i)->m_TaskersPtr == pTasker)
		{
			(m_pTaskerFlag+i)->m_iremovetaskFg = 1;
			bret = true;
			break;
		}
	}
	return bret;
}

template<class T>
bool templateThreadTask<T>::AddTask(T *pTasker )
{
	if ( m_iCount >= m_iTaskMaxCount ) return false;
	bool bret = false;

	for ( int i = 0; i < m_iTaskMaxCount; i++ )
	{
		if ((m_pTaskerFlag+i)->m_iaddtaskFg == 0 && ((m_pTaskerFlag+i)->m_istartaskFg == 0))
		{
			(m_pTaskerFlag+i)->m_TaskersPtr = pTasker;
			(m_pTaskerFlag+i)->m_iaddtaskFg = 1;
			(m_pTaskerFlag+i)->m_iremovetaskFg = 0;
			m_iCount++;
			bret = true;
			break;
		}
	}
	return bret;
}

template<class T>
bool templateThreadTask<T>::StartTask(T *pTasker )
{
	if ( pTasker == NULL ) return false;
	bool bret = false;
	for ( int i = 0; i < m_iTaskMaxCount; i++ )
	{
		if ( (m_pTaskerFlag+i)->m_TaskersPtr == pTasker )
		{
			(m_pTaskerFlag+i)->m_istartaskFg = 1;
			bret = true;
			break;
		}
	}
	return bret;
}

template<class T>
bool templateThreadTask<T>::PauseTask(T *pTasker)
{
	if ( pTasker == NULL ) return false; 
	bool bret = false;
	for ( int i = 0;  i < m_iTaskMaxCount; i++ )
	{
		if ((m_pTaskerFlag+i)->m_TaskersPtr == pTasker)
		{
			(m_pTaskerFlag+i)->m_istartaskFg = 0;
			bret = true;
			break;
		}
	}
	return bret;
}


template<class T>
bool templateThreadTask<T>::IsFull()
{
	return (m_iCount >= m_iTaskMaxCount || (m_iCount == 0));
}

template<class T>
void templateThreadTask<T>::CancelThread()
{
	m_iThreadExitFlag = 1;
	//StopThread();
}

template<class T>
void templateThreadTask<T>::TaskCountSyncLck(bool block)
{
	if(block)
	{
		pthread_mutex_lock(&TaskCountLock);
	}
	else
	{
		pthread_mutex_unlock(&TaskCountLock);
	}
}

#endif // !defined(AFX_TEMPLATETHREADTASK_H__DC9D4EB1_1956_46FE_BA43_4603F5E4CE7A__INCLUDED_)
