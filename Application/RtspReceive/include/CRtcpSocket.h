/*
 * File:   CRtpSocket.h
 * Author: 
 *
 * Created on 2010年2月5日, 上午10:59
 * Modified on 2010年2月22日, 晚上21:59
 */

#ifndef _MVRTCPSOCKET_H
#define _MVRTCPSOCKET_H
#include "CSocketBase.h"

#include "CRtpSocket.h"

class CRtcpSocket:public mvCSocketBase
{
    public:
        //构造函数
        CRtcpSocket();
        //析构函数
        ~CRtcpSocket();

    public:
		bool Init(string strHost);

		void UnInit();

		//连接rtpserver
		bool ConnectRtcpServer();

		//关闭rtpserver
		bool CloseRtcpServer();

		//接收rtcp数据
		void RecvRtcpData();

		//发送rtcp数据
		void SendRtcpData();

		//获取Receive report and source Description
		string GetRRSD();

		//处理rtp数据
		void DealRtcpData(string strMsg);

		//
		void SetRtpSocket(CRtpSocket* pRtpSocket){m_pRtpSocket = pRtpSocket;}

		//获取SSRC
		unsigned int GetRandSSRC();

		void SetPort(int nPort){ m_nRtcpPort = nPort;}

		void SetReceivePort(int nPort){ m_nReceivePort = nPort;}

    private:
		
		string m_strRtcpHost;

		//rtcp server 发送端的端口号
		int m_nRtcpPort;
		//rtcp client 接收端的端口号
		int m_nReceivePort;
		//s->c rtcp
		int m_nRtcpSocket;

		unsigned int m_uRandSSrc;

		unsigned int m_uLatSRT;

		struct timeval m_tvLatSRT;


		bool m_bEndThread;	

		CRtpSocket* m_pRtpSocket;

		struct sockaddr_in m_udp_addr;

		bool m_bConnect;

		#ifdef WIN32
		HANDLE  m_hSendThread;
		HANDLE  m_hRecvThread;
		bool m_bSendThreadStopped;
		bool m_bRecvThreadStopped;
#else
		pthread_t m_nSendThreadId;
		pthread_t m_nRecvThreadId;
		#endif
};

#endif
