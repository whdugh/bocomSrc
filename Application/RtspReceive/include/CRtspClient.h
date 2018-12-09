/*
 * File:   CRtpSocket.h
 * Author: 
 *
 * Created on 2010年2月5日, 上午10:59
 * Modified on 2010年2月22日, 晚上21:59
 */

#ifndef _MVRTSPCLIENT_H
#define _MVRTSPCLIENT_H

#include "CRtspSocket.h"
#include "CRtpSocket.h"
#include "CRtcpSocket.h"

class CRtspClient
{
    public:
        //构造函数
        CRtspClient();
        //析构函数
        ~CRtspClient();

    public:
		
		bool Init(string strHost,int nPort,string strStream);

		void UnInit();

		void PopFrame(string& strFrame);

		void LoginRtspServer();

		bool LogOnServer();

		//设置认证数据
		void SetAuthorizationData(string& strpasswd);
		//设置接收端口
		void SetReceivePort(int nPort);


    private:
		
	CRtspSocket m_RtspSocket;
	CRtpSocket m_RtpSocket;
	CRtcpSocket m_RtcpSocket;

	bool m_bLogOnServer;

	bool m_bEndThread;	

	string  m_strHost;

	//存取互斥
		#ifdef WIN32
			HANDLE  m_hThread;
			bool m_bThreadStopped;
			#else
			pthread_t m_nThreadId;
		#endif
};

#endif
