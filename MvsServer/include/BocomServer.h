
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef BOCOM_SOCKET_H
#define BOCOM_SOCKET_H

#include "CSocketBase.h"

#include "kafkaManage.h"
/******************************************************************************/
//	描述:BOCOM中心端通讯模块
//	作者:於锋
//	日期:2011-3-10
/******************************************************************************/

//通道数据列表
typedef std::list<std::string> MsgResult;
typedef std::multimap<int, std::string> ListMsgMap;
typedef std::list<std::string> ListVideoName;

 class CBocomServer:public mvCSocketBase
 {
  public:
	//构造
	CBocomServer();
	//析构
	~CBocomServer();
  public:
	//开始侦听服务
	bool Init(int nPort, int nId);
	//释放
	bool UnInit();
	
	//传送数据到kafka服务端
	bool SendTokafka(const string& strData);
    //发送到ftp服务器
    bool SendToFtpServer(std::string& strMsg);
	//传送数据到服务端
    bool SendToServer(const string& strData);
	//发送数据
	bool SendMsg(const int nSocket,const string& strData);
    //添加一条命令
	bool AddMsg(const int nSocket,const UINT32 uCommand,const std::string request);
	//删除命令
    bool DelMsg(int nSocket);
    //消息处理
    bool OnMsg(int nSocket,std::string response);

    //弹出消息
	bool PopMsg(int& nSocket,std::string& response);

    //处理一条命令
    bool mvOnDealOneMsg();

	//添加客户端
	bool AddClient(SRIP_CLEINT sClient);
	//删除客户端
	bool DelClient();

	//心跳检测
	void LinkTest();

    //处理历史数据
    void mvDealHistoryRecord();

    //处理数据
	void DealResult();

	//添加一条数据,普通未检测图片
	bool AddResult(std::string& strResult);

    //接收中心端消息
    void mvRecvCenterServerMsg();

	//发送录像
	bool SendDataByFtp(string strLocalFilePath,int nCameraID);
	bool SendFlvDataByFtp(string strLocalFilePath,int nCameraID);

	bool SendVideoForReSend();

	//返回侦听套接字
	int GetAcceptSocket() { return m_nAcceptSocket; }

	//判断socket状态
	bool CheckSocketStatus();
#ifdef KAFKA_SERVER
	//初始化kafkaClient服务
	bool InitKafkaClient();
#endif

private:
    //处理检测结果
	bool OnResult(std::string& result);

    //登录处理
    bool OnLogin(const int nSocket,const UINT32 uCommand,std::string request);

    //心跳处理
    bool OnLink(const int nSocket,std::string request);

    //是否推送实时记录(推送实时记录需要确认)
    bool OnRealTime(const int nSocket,std::string request);

	//系统时间设置
	bool OnSysTime(const int nSocket,std::string request);

    //设置停车检测区域
    bool OnSetStopRegionInfo(const int nSocket, std::string request);

    //设置预置位信息
    bool OnSetPresetInfo(const int nSocket, std::string request);
	//设置预置位模式
	bool OnSetPresetMode(const int nSocket, std::string request);
    //获取记录状态
    bool OnRecordStatus(const int nSocket,std::string request);

    //日志查询
    bool OnSearchLog(const int nSocket,std::string request);
    //统计查询
    bool OnSearchStatistic(const int nSocket,std::string request);
    //事件查询
    bool OnSearchEvent(const int nSocket,std::string request);
    //车牌查询
    bool OnSearchPlate(const int nSocket,std::string request);
    //获取车牌检测区域
    bool OnPlateRegion(const int nSocket,std::string request);

    //截取一帧图象
	bool OnCaptureOneFrame(const int nSocket,std::string request);

	string ConvertPlate(RECORD_PLATE &plate);

private:

    //线程ID
	pthread_t m_nThreadId;
    //历史线程ID
	pthread_t m_nHistoryThreadId;
	pthread_t m_nResendThreadId;
	//检测结果信号互斥
	pthread_mutex_t m_Result_Mutex;
	//检测结果消息列表
	MsgResult	m_ChannelResultList;

	//侦听套接字连接
    int m_nAcceptSocket;
    //侦听端口
    int m_nPort;
    //中心端连接套接字
    int m_nCenterSocket;
    //心跳计数
    int m_nLinker;
    //是否认证成功
    bool m_bVerify;
    //接收的命令队列映射列表
    ListMsgMap m_mapMsgList;
	//存放要发送的录像的名字
	ListVideoName m_VideoName;
	//存放录像类型
	int m_nVideoType;
	//信号互斥
	pthread_mutex_t m_Msg_mutex;
	//发送录像互斥锁
	pthread_mutex_t m_Video_Mutex;
	//信号互斥
	pthread_mutex_t m_thread_mutex;

	int m_nId;//编号

#ifdef _DEBUG
	int m_nMsgCount;
#endif

#ifdef KAFKA_SERVER
	CKafakaManage* m_kafkaManage;
#endif
};


#endif
