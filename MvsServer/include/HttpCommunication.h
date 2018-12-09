// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary


//湖北省卡口项目Http传输专用
#ifndef SKP_HTTPCOMMUNICATION_H
#define SKP_HTTPCOMMUNICATION_H

#include "CSocketBase.h"
#include "CenterServer.h"


class CHttpCommunication:public mvCSocketBase
{
public:
		CHttpCommunication();
		virtual ~CHttpCommunication();

		//启动服务
        bool Init();

        //释放
        bool UnInit();

		/*主线程调用接口*/
        void mvConnOrLinkTest();

		//添加一条数据
        bool AddResult(std::string& strResult);

        //处理检测结果
        bool OnResult(std::string& result);

        //处理实时数据
        void DealResult();

        //处理历史数据
        void DealHistoryResult();

		 /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg);

		 /*连接到中心端*/
        bool mvConnectToCS();

		//获取请求消息
		string GetRequest(const string& strMsg);
		string GetRegister(UINT32 uCameraID);
		string GetLinkTest(UINT32 uCameraID);
		bool mvRecvMsg(string& strMsg);
		bool& GetbCenterLink(){return m_bCenterLink;}
		int& GetCenterSocket(){return m_nCenterSocket;}
		int GetRandTime();
		bool SendRegister(UINT32 uCameraID = 0);
		string GetBoundary();


		//将接收到响应消息存到文件
		void SaveMessageToFile(string strTmp);
		//根据取到的线圈状态对车检器Detector的Value值做界定
		int GetDetectorValue();
		//根据时间字符串(YYYYMMDDHHMMSS)还原成秒数
		unsigned long MyMakeTime(std::string& strTime);
		//设置与卡口中心时间同步
		bool OnSystemTime(string timeTmp);
	
 private:
        int m_nCenterSocket;
        bool m_bCenterLink;

        pthread_t m_nThreadId;
        pthread_t m_nHistoryThreadId;
		pthread_t m_nLinkThreadId;

		std::list<string> m_ResultList;
        pthread_mutex_t m_Result_Mutex;
		pthread_mutex_t m_file_mutex;
};

extern CHttpCommunication g_HttpCommunication;
#endif
