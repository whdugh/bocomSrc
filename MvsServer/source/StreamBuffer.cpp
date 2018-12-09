// StreamBuffer.cpp: implementation of the CStreamBuffer class.
//
//////////////////////////////////////////////////////////////////////
#include "StreamBuffer.h"
#include <malloc.h>
#include <string.h>
#include "fastmemcpy.h"
#include <assert.h>
#include <stdio.h>
#include "constdef.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStreamBuffer::CStreamBuffer( int iBufSize )
{
	m_ubiBufSize = iBufSize;
	m_ubiTotalWrite = 0ULL;
	
	m_iPackageLen = MAX_DSP_BUF_LEN;
	m_iWritePtr = 0;

	m_pBuf = NULL;
	m_pBuf = (char *)calloc( m_ubiBufSize, 1);
	pthread_mutex_init((&datalock), NULL);
	pthread_mutex_init((&totalWriteLock), NULL);
}


CStreamBuffer::~CStreamBuffer()
{
	if (m_pBuf != NULL)
	{
		free( m_pBuf );
		m_pBuf = 0;
	}
	pthread_mutex_destroy(&datalock);
	pthread_mutex_destroy(&totalWriteLock);
}


void CStreamBuffer::SseCopy( void *pData, void *pSrc, int len )
{
#ifdef WIN32

	int iLoop = len / 8;
	if ( iLoop * 8   < len ) iLoop++;

	__asm
	{
		mov esi, [pSrc];
		mov edi, [pData];

		mov ecx, iLoop
nextcopy:
		pxor mm0, mm0
		movq mm0, [esi]
		movq [edi], mm0
		
		add esi, 8
		add edi, 8
		sub ecx, 1

		cmp ecx, 0
		jg nextcopy
		emms
	};
#else
	memcpy( pData, pSrc, len );
//	fast_sse_memcpy( pData, pSrc, len );
#endif
}


void CStreamBuffer::WriteBuffer( char *pData, int len )
{
	if ( pData == NULL || len <= 0)
	{
		return;
	}

	int iTmp =  m_iWritePtr + len + 12;//每次写入数据有12字节的附加信息，先写附加数据，再写码流数据
	if (iTmp <= m_ubiBufSize )//写入长度位置没有超过buf的大小，直接赋值
	{
		SseCopy( m_pBuf + m_iWritePtr+12, pData, len );//写入真正的码流数据
		unsigned long long* lp = (unsigned long long*)(m_pBuf + m_iWritePtr);//对应开始写入附加数据的位置
		pthread_mutex_lock(&totalWriteLock);
		*lp = m_ubiTotalWrite;//附加数据前8个字节，为已经写入数据的总数量
		pthread_mutex_unlock(&totalWriteLock);
		int *lp1 = (int*)(m_pBuf + m_iWritePtr + 8);//对应附加数据最后4个字节位置
		*lp1 = len;//附加数据最后4个字节,赋值为本次写入的数据长度

		m_iWritePtr = (m_iWritePtr + len + 12)%m_ubiBufSize;
	}
	else//写入长度位置超过了buf的大小，尾部和首部分开赋值，作为环形buf来处理
	{
		unsigned char szIndexHead[12];
		unsigned long long* lp = (unsigned long long*)(&szIndexHead[0]);
		pthread_mutex_lock(&totalWriteLock);
		*lp = m_ubiTotalWrite;//附加数据前8个字节，为已经写入数据的总数量
		pthread_mutex_unlock(&totalWriteLock);
		int *lp1 = (int*)(szIndexHead + 8);
		*lp1 = len;//附加数据最后4个字节,赋值为本次写入的数据长度
		
		int iTailSize = m_ubiBufSize - m_iWritePtr;//剩余的长度够不够12个字节的附加长度

		if(iTailSize >= 12)//足够附加数据的赋值
		{
			SseCopy( m_pBuf + m_iWritePtr, szIndexHead, 12 );//拷贝附加信息
			m_iWritePtr += 12;
			iTailSize -= 12;
			SseCopy( m_pBuf + m_iWritePtr, pData, iTailSize );//缓冲区末尾还剩iTailSize的空间，拷贝部分码流数据
			
			int iHeadSize = iTmp - m_ubiBufSize;
			m_iWritePtr = 0;
			SseCopy( m_pBuf + m_iWritePtr, pData + iTailSize, iHeadSize );//剩下的码流数据拷贝到缓冲区首部
			m_iWritePtr += iHeadSize;
		}
		else//不够附加数据的赋值，尾首环形赋值
		{
			SseCopy( m_pBuf + m_iWritePtr, szIndexHead, iTailSize );//尾部拷贝部分附加数据

			m_iWritePtr = 0;
			SseCopy( m_pBuf + m_iWritePtr, szIndexHead + iTailSize, 12 - iTailSize );//剩下的附加数据拷贝到首部
			m_iWritePtr += 12 - iTailSize;
			SseCopy( m_pBuf + m_iWritePtr, pData, len );//拷贝码流数据
			m_iWritePtr += len;
		}
	}
	pthread_mutex_lock(&totalWriteLock);
	m_ubiTotalWrite += len+12;//已经写入数据的总数量计数
	pthread_mutex_unlock(&totalWriteLock);
	assert(m_ubiTotalWrite<0xfffffffffff00000LLU);//最大值减1M
}


int CStreamBuffer::ReadBuffer( char *pData, unsigned long long &iPos, int len )
{
	unsigned long long  tmpTotalWrite = 0ULL;

	pthread_mutex_lock(&totalWriteLock);
	tmpTotalWrite = m_ubiTotalWrite; 
	pthread_mutex_unlock(&totalWriteLock);
	
	unsigned long long off = tmpTotalWrite - iPos;//写入数据总数-读取数据总数
	if(off == 0)
	{
		//printf("**************************1111111111111111\n");
        return 0;
	}

    if(off > (m_ubiBufSize-m_iPackageLen)) //写入的位置超过读取位置的长度超过了缓冲区的长度。此时丢失数据了。
	{
//		sExceptionInfos tagErroException;
//		memset(&tagErroException,0,sizeof(sExceptionInfos));
//		tagErroException.iErrCode = Err_CANT_RECEIVE_SOURCE_DATA;
//		
//		if((int)off <= m_ubiBufSize)
//		{
////			CStUnitImp::m_gpVisLog->Warning(MODULE_SRCTODEST,LOGLEVEL_ADMIN,\
//	//			"数据丢失 <= %d Bytes, usable count: %d",m_ubiBufSize,m_iPackageLen);
//			sprintf(tagErroException.sDescription,"数据丢失 <= %d Bytes, usable count: %d",m_ubiBufSize,m_iPackageLen);
//		}
//		else
//		{
////			CStUnitImp::m_gpVisLog->Warning(MODULE_SRCTODEST,LOGLEVEL_ADMIN,\
//	//			"数据丢失  %d Bytes",(int)off);
//			sprintf(tagErroException.sDescription,"数据丢失  %d Bytes",(int)off);
//		}
//
//		CStUnitImp::m_gpExceptionCBFun((sExceptionInfos*)&tagErroException,0);
		iPos = tmpTotalWrite;
		printf("**************************2222222222222\n");
		return 0;
	}
	
	//get datalen 
	int iReadPos = 0;
	int iPackageLen = 0;
	iReadPos = iPos % m_ubiBufSize;
	unsigned char szCheck[12] = {0};
	int uiTmp = iReadPos + 12;
	if(uiTmp <= m_ubiBufSize)//缓冲区剩余长度足够保存附加数据
	{
		unsigned long long ubiCheck = 0ULL;
		ubiCheck = *((unsigned long long*)(m_pBuf+iReadPos));
		iPackageLen = *((int*)(m_pBuf+iReadPos+8));//取得数据包的长度
		iReadPos += 12;
		iReadPos = iReadPos % m_ubiBufSize;//定位读取数据的开始位置
	}
	else//附加数据需要分包处理
	{
		SseCopy( szCheck, m_pBuf + iReadPos,  m_ubiBufSize - iReadPos);//附加数据拷贝到缓冲区头部
		SseCopy( szCheck + m_ubiBufSize - iReadPos, m_pBuf,  uiTmp - m_ubiBufSize);//附加数据拷贝到缓冲区尾部

		unsigned long long ubiCheck = 0ULL;
		ubiCheck = *((unsigned long long*)(szCheck));
		iPackageLen = *((int*)(szCheck+8));
		iReadPos += 12;
		iReadPos = iReadPos % m_ubiBufSize;
	}

	uiTmp = iReadPos+iPackageLen;//当前读取位置加上数据包的长度
	if ( uiTmp <= m_ubiBufSize )//不超过缓冲区的大小
	{
		SseCopy( pData, m_pBuf + iReadPos,  iPackageLen);//直接拷贝码流数据后返回
		return iPackageLen;
	}
	
    //当前读取位置加上数据包的长度超过缓冲区的大小
	int iTailSize = m_ubiBufSize - iReadPos;
	SseCopy( pData, m_pBuf + iReadPos, iTailSize );//拷贝尾部的数据

	int iHeadSize = iReadPos + iPackageLen - m_ubiBufSize;
	SseCopy( pData + iTailSize, m_pBuf, iHeadSize  );//拷贝头部的数据
	return iPackageLen;
}

unsigned long long CStreamBuffer::GetCurrentWriteTotal()
{
	unsigned long long tempTotal;
	pthread_mutex_lock(&totalWriteLock);
	tempTotal=m_ubiTotalWrite;
	pthread_mutex_unlock(&totalWriteLock);

	return tempTotal;
}
