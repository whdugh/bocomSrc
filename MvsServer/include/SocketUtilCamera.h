// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _SOCKETUTILCAMERA_H_
#define _SOCKETUTILCAMERA_H_

#ifndef _SOCKET_ROSEEK
    typedef int SOCKET;
    #define SOCKET_ERROR            (-1)
    #define INVALID_SOCKET  (SOCKET)(~0)
#endif

#ifndef _BOOL_ROSEEK
    #define BOOL bool
#endif

BOOL SafeSend(SOCKET sock, const char* pBuf, int nLen);
BOOL SafeRecv(SOCKET sock, char*pBuf, int nLen);
BOOL ConnectHasTimeout(SOCKET sock, struct sockaddr_in *pAddr, int nTimeOut);
BOOL WaitForSockCanRead(SOCKET sock, int nTimeOut = 1000);

#endif



