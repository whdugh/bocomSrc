/*
 * File:   CRtspSocket.h
 * Author: 
 *
 * Created on 2010年2月5日, 上午10:59
 * Modified on 2010年2月22日, 晚上21:59
 */

#ifndef _MVRTSPSOCKET_H
#define _MVRTSPSOCKET_H
#include "CSocketBase.h"

class CRtspSocket:public mvCSocketBase
{
    public:
        //构造函数
        CRtspSocket();
        //析构函数
        ~CRtspSocket();

    public:
		bool Init(string strHost,int nPort,string strStream);

		void UnInit();

        //连接rtsp server
        bool    ConnectRtspServer();  
		//关闭rtspserver
		bool   CloseRtspServer();
		//发送Option命令
		bool SendOptionCmd();
		//发送Describe命令
		bool SendDescribeCmd();
		//发送SetUp命令
		bool SendSetUpCmd();
		//发送Play命令
		bool SendPlayCmd();
		//发送TearDown命令
		bool SendTearDownCmd();

		//处理rtsp返回数据
		void DealRtspMsg(string strMsg);
		int DealRtspMsgWithMulticast(string strMsg);

		void DealDescribeMsg(string strMsg);

		//接收rtsp返回数据
		bool RecvMsg(string& strMsg);

		int GetRtpPort() {return m_nRtpPort;}

		int GetRtcpPort() {return m_nRtcpPort;}
		
		int GetReceiveRtpPort() {return m_nReceiveRtpPort;}

		int GetRandRtpPort();

		int DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen);

		//设置认证数据
		void SetAuthorizationData(string& strpasswd);

		//设置接收端口
		void SetReceivePort(int nPort){m_nReceiveRtpPort = nPort;}

    private:
		
		string m_strRtspHost;
		
		int m_nRtspPort;

		int m_nRtspSocket;

		int m_nCSeq;

		string m_strStream;

		bool m_bEndThread;

		int m_nRtpPort;
		int m_nRtcpPort;

		int m_nReceiveRtpPort;

		string m_strSessionID;

		string m_strEnpasswd;

		string m_strTrackID;
};

#endif
