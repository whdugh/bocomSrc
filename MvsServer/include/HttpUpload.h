// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary


//南昌项目Http传输专用
#ifndef SKP_HTTPUPLOAD_H
#define SKP_HTTPUPLOAD_H

#include "CSocketBase.h"
#include "CenterServer.h"
#include<map>

typedef struct ChannelInitInfo
{
	char chDeviceID[32];   //卡口编号
	int nCameraId;    //车道号编号 每个相机监控一个车道
	int nChanWayType;//车道类型
	char chCode[48];//注册码
	int initState;  //初始化状态
	int nChannelID;//通道号
	ChannelInitInfo()
	{
		memset(chDeviceID,0,32);  
		nCameraId = 1;
		nChanWayType = 0;
		memset(chCode,0,48);
		initState = 0;
		nChannelID=0;
	}
}ChannelInitInfo;

typedef struct InitDownChannel
{
	char chDeviceID[32];   //卡口编号
	int nChannelID;//通道号
	InitDownChannel()
	{
		memset(chDeviceID,0,32);  
		nChannelID=0;
	}
}InitDownChannel;

typedef std::map<int,int> IntMapDir;

class CHttpUpload:public mvCSocketBase
{
public:
		CHttpUpload();
		virtual ~CHttpUpload();

		//启动服务
        bool Init();
		void InitHttpTrans(char chKakouID[32],int nChannelId,int nChanWayType,char chKey[48],int & initstate);
		void InitRoadWay();
        //释放
        bool UnInit();

		//获取通道信息
		void GetChannelInfo();

		//添加一条数据
        bool AddResult(std::string& strResult);
		//获取行驶方向的首字母大写
		char GetCapitalOfDirection(unsigned int uDirection);
		//获取违法行为编码
		string GetViolationType(unsigned int uViolationType);
        //处理检测结果
        bool OnResult(std::string& result);
		//获取请求消息                        /* 卡口编号或地点编号  远程图片路径            全景图片名称         特征图片名称*/
		string GetRequest(const string& strMsg,char *chDeviceID,char * sServerPath,char *sFileName ,char *tzFileName = NULL);
        //处理实时数据
        void DealResult();
		
        //处理历史数据
        void DealHistoryResult();
		//磁盘管理
		void DiskManage();

		 /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg);

		//接受返回消息  xmlRet保存返回消息的大小
		int mvRecvMsg(string& strMsg,unsigned int & xmlRet);
		 /*连接到中心端*/
		bool mvConnectToCS();
		//将接收到响应消息存到文件
		void SaveMessageToFile(string strMsg);

		//删除时间目录
		void DelDir();
		//递归删除目录
		bool RemoveDir( const char* dirname,int nType);
 private:
		int m_nCenterSocket;
		pthread_t m_nInitThreadId;
        pthread_t m_nThreadId;
        pthread_t m_nHistoryThreadId;
		pthread_t m_nDiskManageThreadId;

		std::list<string> m_ResultList;

        pthread_mutex_t m_Result_Mutex;
		pthread_mutex_t m_Send_Mutex;
	
		pthread_mutex_t m_Init_mutex;
	    //pthread_mutex_t m_test_mutex;
	    //pthread_mutex_t m_file_mutex;
		list<ChannelInitInfo> m_lchannelInitInfo; //通道初始化信息及状态列表
		list<InitDownChannel> m_lInitDownChannel; //初始化成功的通道编号
		map<int,struct tm> m_initStartWaitTime;
		int            m_sendErLog; //失败日志记录
		struct timeval m_TheTimeLog;//日志记录时间
};

extern CHttpUpload g_HttpUpload;

#endif
