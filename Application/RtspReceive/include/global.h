// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef RTSP_GLOBAL_H
#define RTSP_GLOBAL_H




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <sys/time.h>
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <iconv.h>
#else
#pragma once
#include <time.h>
#include <Winsock.h>
#pragma comment(lib,"WSock32.Lib")
#endif

#include <sys/types.h>

#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <sys/timeb.h>


#include <string>
#include <map>
#include <list>
#include <vector>
#include <ostream>
#include <fstream>
#include <ios>
#include <sstream>

using namespace std;
extern FILE* m_fp;
extern void printf2File(const char* format,...);
extern int m_nType;
extern string m_strBroadIP;

#ifdef WIN32
#define socklen_t int
#endif

#endif //SKP_ROAD_GLOBAL_H
