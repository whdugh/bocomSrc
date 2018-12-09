// StreamBuffer.h: interface for the CStreamBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENCODERBUFFER_H__2E192A08_4E32_40DA_89C8_A881515802D7__INCLUDED_)
#define AFX_ENCODERBUFFER_H__2E192A08_4E32_40DA_89C8_A881515802D7__INCLUDED_
/* 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/
#include <pthread.h>
#define ROUNDARRYLEN 100
extern char* GetNowTime(char*);

/************************************************************************/
/* 
本类为数据缓冲区工具，为环形缓冲区
缓冲区的大小由数据的大小决定
目前缓存5S的数据
*/
/************************************************************************/
class CStreamBuffer
{
public:
	CStreamBuffer( int iBufSize );
	virtual ~CStreamBuffer();

public:
    //向缓冲区写入数据
	void WriteBuffer( char *pData, int len );
	//从缓冲区读出数据
    int ReadBuffer( char *pData, unsigned long long &iPos, int len );
	//获取写入的数据总数量
    unsigned long long GetCurrentWriteTotal();
    //拷贝数据
	void SseCopy( void *pData, void *pSrc, int len );
private:
    //缓冲区大小
	unsigned long long m_ubiBufSize;
    //写入的数据总数量
	unsigned long long m_ubiTotalWrite;
    //读写数据总数量互斥锁
	pthread_mutex_t totalWriteLock;
	//缓冲区buf指针
    char *m_pBuf;
    //写数据时对应的buf地址
	int m_iWritePtr;
    //单次接收数据的最大长度
	int m_iPackageLen;
    //读写缓冲区数据互斥锁
	pthread_mutex_t datalock;	
};

#endif // !defined(AFX_ENCODERBUFFER_H__2E192A08_4E32_40DA_89C8_A881515802D7__INCLUDED_)
