// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _BJSERVER_H
#define _BJSERVER_H

#include "CSocketBase.h"

/**
 *   文件：BJServer.h
 *   功能：北京图侦通讯类
 *   作者：YuFeng
 *   时间：2012-01-30
**/

//typedef std::multimap<int, SRIP_CLEINT> H264Client_Map;

//#define START_H264_STREAM	0x00000001	//开启H264视频流
//#define STOP_H264_STREAM	0x00000002	//关闭H264视频流

#define LOGIN				0x00000001	//中心端登陆
#define LOGIN_REP			0x10000001	//中心端登陆返回
#define LINK_HEART			0x10000002	//心跳
#define EVENT_ALARM			0x10000003	//事件报警
#define SWITCH_CAMERA_BJ	0x00000004	//切换视频
#define SWITCH_CAMERA_REP_BJ	0x10000004	//切换视频返回

#define BJSERVER_PORT 61000

class CBJServer:public mvCSocketBase
{
public:
    //构造
    CBJServer();
    //析构
    virtual ~CBJServer();

    //启动侦听服务
    bool Init();
    //释放
    bool UnInit();

	//向h264视频缓冲区加入数据
	//void AddH264Frame(string& frame, int cameraID);
	
	//向Event缓冲区加入数据
	void AddEvent(RECORD_EVENT& event, int cameraID);
	//添加客户端
	void AddClient(SRIP_CLEINT sClient);
	//删除客户端
	void DelClient();
	//接收中心端消息
	void mvRecvCenterServerMsg();

	//发送H264视频
	//void SendH264Frame();

	//发送Event
	void SendEvent();

	//发送心跳
	void LinkTest();

private:
	//处理收到的消息
	void OnMsg(const int nSocket, string request);

	//void PopH264Frame(string &frame);
	//H264Client_Map m_h264ClientMap;
	//vector<int> m_eventCameraIdVec;
	//list<string> m_h264FrameBuf;
	//pthread_t m_nH264ThreadId;
	//pthread_mutex_t m_ClientMap_mutex;
	//pthread_mutex_t m_EventVec_mutex;
	//pthread_mutex_t m_Frame_Mutex;

	//是否通过验证
	bool m_bVerify;

	list<string> m_eventBuf;

	pthread_t m_nEventThreadId;

	int m_nAcceptSocket;
	int m_nCenterSocket;

	pthread_mutex_t m_Event_Mutex;
	pthread_mutex_t m_thread_mutex;
};

extern CBJServer g_BJServer;

#endif
