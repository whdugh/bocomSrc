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

#ifndef SKP_ROAD_SOCKET_H
#define SKP_ROAD_SOCKET_H

#include "CSocketBase.h"

/******************************************************************************/
//	描述:智能交通检测系统通讯模块。
//	作者:徐永丰
//	日期:2008-4-1
/******************************************************************************/

//通道数据列表
typedef std::list<std::string> ResultMsg;

 class CSkpRoadSocket:public mvCSocketBase
 {
  public:
	//构造
	CSkpRoadSocket();
	//析构
	~CSkpRoadSocket();
  public:
	//开始侦听服务
	bool Init();
	//释放
	bool UnInit();

	//传送数据到客户端
	bool SendToClient(std::string& strData);
	//发送数据
	bool SendMsg(const int nSocket,std::string strData,bool bClient=true);

	//添加客户端
	bool AddClient(SRIP_CLEINT sClient);
	//删除客户端
	bool DelClient(int nSocket,bool bAll=false);

	//心跳检测
	void LinkTest();
	//重置心跳检测值
	bool ResetLinker(const int nSocket);

	//保存登录用户
	bool SaveUser(const int nSocket,const std::string user,const int port,int nClientKind);

	//获取连接数
	int GetConnectClientCount();

	//是否需要推送实时视频
	void SetConnect(const int nSocket,bool bConnect);


    // zhangyaoyao: get user's username through socket
    string GetClientName(int nSocket);


private:


private:

	//客户端连接映射列表
	ClientMap m_ClientMap;
	//ClientMap信号互斥
	pthread_mutex_t m_thread_mutex;

	//侦听套接字连接
    int m_nAcceptSocket;
    //侦听端口
    int m_nPort;

	//udp方式向客户端发送视频的socket
	int m_nUdpSocket;
};
//通讯服务
extern CSkpRoadSocket g_skpRoadServer;

#endif
