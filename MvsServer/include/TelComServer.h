// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef TELCOMSERVER_H
#define TELCOMSERVER_H

#include "CSocketBase.h"
#include "TelConstdef.h"
#include "XmlParser.h"
#include "md5.h"

/**
    文件：TelComServer.h
    功能：电信全球眼通讯类
    作者：於锋
    时间：2011-7-13
**/

class CTelComServer:public mvCSocketBase
{
    public:
        //构造
        CTelComServer();
        //析构
        virtual ~CTelComServer();

        /*主线程调用接口*/
        void mvConnOrLinkTest();

        //启动服务
        bool Init();

        //释放
        bool UnInit();

        //添加一条数据
        bool AddResult(std::string& strResult);

        //处理检测结果
        void OnResult(std::string& result);

        //处理实时数据
        void DealResult();

		//接收IVM消息
        bool mvRecvIVMMsg();
		//从IVM收到的消息加入List
        void doIVMMsg(string strMsg);
		//从socket中接收消息
        bool mvRecvMsg(int socket, string& strMsg);
		//从CMS接收消息
        bool mvRecvCMSRep(int socket);

		//登陆状态
		LOGIN_STATE m_bLoginIVM;

		int m_nResultUpSocket;
		int m_nPicUpSocket;

    private:
        /*连接到中心端*/
        bool mvConnectToIVM();

        /*发送心跳测试*/
        bool mvSendLinkTest();
		//发送消息到IVM
        bool SendMsgToIVM(BYTE cmd, string strData, unsigned short seq = 0);
		//发送事件及文件
		bool SendMsgToCMS(int& socket, string strMsg, int type);
		
		//生成XML数据报文
        string CreateLoginXml();
        string CreateLinkXml();
        string CreateDetectStartRepXml(string result);
		string CreateDetectStopXml(string result);
		string CreateExNotifyXml(const vector<string>& vec);
		string CreateAlgAbilityXml();
		string CreateSearchXml(string algCode);

		//解析XML数据报文
		void AnalyseLoginResXml(string strXml, string& result);
		void AnalyseDetectStartXml(string strXml, MONITOR_HOST_INFO& info, string& result);
        void AnalyseDetectStopXml(string strXml, string& result);
		void AnalyseAlgAbilityXml(string strXml, string& result);
		void AnalyseSearchXml(string strXml, string& result);
		void AnalyseExRepXml(string strXml, string& result);
        
		//处理CMS响应消息
        void doRepResult(string strMsg);
		//生成MD5验证信息
        void GetStringMd5(string str, unsigned char* digest);
		//读入配置文件
		void LoadTelComConfig(map<string,string>& map);
        //连接到CMS
        bool connectCMS(int& socket, int type);
		//向IVM发送异常信息
		bool TelIVMException(const vector<string>& vec);

     private:
        int m_nIvmSocket;
        int m_nIVMLinkCount;

        bool m_bIVMLink;
        bool m_bCMSResultLink;
        bool m_bCMSPicLink;

        pthread_t m_nTelCMSThreadId;
        pthread_t m_nResultRepThreadId;
		pthread_t m_nPicRepThreadId;
        pthread_t m_nRecvIVMMsgThreadId;

		string m_sTaskSession; //任务会话

        string m_sIvmServerHost;//IVM ip地址
        int m_nIvmServerPort;// IVM 端口号

        string m_sResultUpHost;//上传分析结果ip地址
        int m_nResultUpPort;// 上传分析结果端口号

		string m_sDataPicUpHost;//上传图片ip地址
		int m_nDataPicUpPort;// 上传图片端口号

        UINT16 m_nSEQCount;//流水号

        std::list<std::string> m_ResultList;	//分析结果List
        pthread_mutex_t m_Result_Mutex;
		std::map<UINT16, std::string> m_ImageMap;  //图片Map
		pthread_mutex_t m_Image_Mutex;
};
extern CTelComServer g_TelComServer;
#endif

