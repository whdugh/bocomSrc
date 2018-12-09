/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：NESocket.h
* 摘要: 与中心网管通信模块
* 版本: V1.0
* 作者:
* 完成日期: 2009年9月5日
* 版本：V2.0
* 作者：於锋
* 修改日期：2009年11月10日
*/
#ifndef NESOCKET_H
#define NESOCKET_H

#include "CSocketBase.h"
#include "necommhead.h"


class NESocket: public mvCSocketBase
{
public:
    NESocket();
    virtual ~NESocket();

    //开始侦听服务
    bool Init();
    //释放
	bool UnInit();

    /**
     * 添加中心网管连接到连接队列中
     * client:  中心网管连接
     */
    void AddNeClient(const NeClient &client);
    //删除中心网管连接
	bool DelNeClient(int nSocket=0);

    /**
     * 向中心网管的所有连接发送数据
     * msg:     消息缓冲，由使用者维护，缓冲长度由strlen函数计算，所以msg[strlen(msg)]必须是'\0';
     */
    void SendAlarmInfo(const std::string& strMsg);

     /**
     * 与中心网管通讯的心跳检测
     * 说明：智能检测端从不主动发送心跳检测请求，只会等待网管心跳检测请求，然后返回心跳响应包
     */
    void SendHeartBitPackage(int nClient, UINT64 workflowId);
        /**
     * 与中心网管通信的相机状态查询响应
     * nClient:     网管连接socket
     * workflowId:  本次会话编号
     * nCameraID:   请求查询的相机编号
     */
    void SendCameraStatus(int nClient, UINT64 workflowId, int nCameraID);
    /**
     * 与中心网管通讯的协议解析
     * nClient：     中心网管连接socket
     * msgHeader：   要解析的协议包
     */
    void ParseNeMsg(int nClient, const sNeMsgHead& msgHeader);

    //zhangyaoyao:告诉网管业务程序的重启信息
    void SendStartInfoToNe(int nNeSocket);
    //zhangyaoyao:判断程序的重启是否是因为程序更新
    bool IsStartForUpdate(const char* pFilePath);

private:

    /**
     * 中心网管连接
     */
    NeMap m_NeMapClients;
    //NeMap信号互斥
	pthread_mutex_t m_thread_mutex;

    //侦听套接字
    int m_nAcceptSocket;
    //侦听端口
    int m_nPort;
};

extern NESocket g_skpNeServer;

#endif//NESOCKET_H
