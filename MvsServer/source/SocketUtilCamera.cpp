// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "SocketUtilCamera.h"
#include "Common.h"
#include "CommonHeader.h"

BOOL SafeSend(SOCKET sock, const char* pBuf, int nLen)
{
	int nRet = 0;
	int nSended = 0;
	while (nSended < nLen)
	{
		nRet = send(sock, pBuf + nSended, nLen - nSended, MSG_NOSIGNAL);
		if (nRet <= 0)
		{
			LogError("send strerror(errno)=%s #\r\n",strerror(errno));
			break;
		}
		nSended += nRet;
	}
	if (nSended < nLen)
	{
		LogNormal("=##===SafeSend==nSended=%d==\n", nSended);
		return FALSE;
	}
	return TRUE;
}

BOOL SafeRecv(SOCKET sock, char*pBuf, int nLen)
{
	int nRet = 0;
	int nRecved = 0;
	while (nRecved < nLen)
	{
		nRet = recv(sock, pBuf + nRecved, nLen - nRecved, MSG_NOSIGNAL);
		if (nRet <= 0)
		{
			break;
		}
		nRecved += nRet;
	}
	if (nRecved < nLen)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL ConnectHasTimeout(SOCKET sock, struct sockaddr_in *pAddr, int nTimeOut)
{
    printf("====ConnectHasTimeout====sock=%d====!!!=\n", sock);
	BOOL bResult = FALSE;
	do
	{
		//unsigned long ulVal = 1;
		/*
		if (0 != ioctlsocket(sock, FIONBIO, &ulVal))
		{
			break;
		}

		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&ulVal, sizeof(unsigned long)))
		{
		    break;
		}
		*/		

		fd_set fsVal;
		FD_ZERO(&fsVal);
		FD_SET(sock, &fsVal);
		struct timeval tvTimeOut, tvTimeOut2;
		tvTimeOut.tv_sec = nTimeOut/1000;
		tvTimeOut.tv_usec = (nTimeOut%1000)*1000;
		//int nRet = select(0, 0, &fsVal, 0, &tvTimeOut);

		if(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tvTimeOut, sizeof(timeval)) == -1)
        {
            printf("==ConnectHasTimeout==setsockopt==1=ERROR!!!");
            break;
        }

		tvTimeOut2.tv_sec = tvTimeOut.tv_sec + 3;
		tvTimeOut2.tv_usec = tvTimeOut.tv_usec;

        if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tvTimeOut2, sizeof(timeval)) == -1)
        {
            printf("==ConnectHasTimeout==setsockopt==2=ERROR!!!");
            break;
        }

		unsigned long ulVal = 1;	
		if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&ulVal, sizeof(unsigned long)))
		{
			break;
		}

		if(connect(sock, (struct sockaddr*)pAddr, sizeof(*pAddr)) == -1)
		{
			return false;
		}

        /*
		if (0 == nRet || SOCKET_ERROR == nRet)
		{
			break;
		}
		*/
/*
		ulVal = 0;
		if (0 != ioctlsocket(sock, FIONBIO, &ulVal))
		{
			break;
		}

        ulVal = 0;
        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&ulVal, sizeof(unsigned long)))
        {
            break;
        }
*/
		bResult = TRUE;
	} while(0);

	return bResult;
}

BOOL WaitForSockCanRead(SOCKET sock, int nTimeOut /* = 1000 */)
{
	fd_set fsVal;
	FD_ZERO(&fsVal);
	FD_SET(sock, &fsVal);
	struct timeval tvTimeOut;
	tvTimeOut.tv_sec = nTimeOut/1000;
	tvTimeOut.tv_usec = (nTimeOut%1000)*1000;
	int nRet = select(0, &fsVal, 0, 0, &tvTimeOut);
	if (nRet > 0 && FD_ISSET(sock, &fsVal))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


