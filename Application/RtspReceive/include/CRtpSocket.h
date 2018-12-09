/*
 * File:   CRtpSocket.h
 * Author: 
 *
 * Created on 2010年2月5日, 上午10:59
 * Modified on 2010年2月22日, 晚上21:59
 */

#ifndef _MVRTPSOCKET_H
#define _MVRTPSOCKET_H
#include "CSocketBase.h"

class CRtpSocket:public mvCSocketBase
{
    public:
        //构造函数
        CRtpSocket();
        //析构函数
        ~CRtpSocket();

    public:
		bool Init(string strHost);

		void UnInit();

		//连接rtpserver
		bool ConnectRtpServer();
		bool ConnectRtpServerMulti();

		//关闭rtpserver
		bool CloseRtpServer();

		//接收rtp数据
		void RecvRtpData();

		//处理rtp数据
		void DealRtpData(string strMsg);

		unsigned short GetSeq() {return m_uSeq;}

		void AddFrame(string& strFrame);

		void PopFrame(string& strFrame);
		
		//设置发送端口
		void SetPort(int nPort){ m_nRtpPort = nPort;}

		//获取SSRC
		unsigned int GetSSRC() {return m_uSSrc;}
		
		//设置接收端口
		void SetReceivePort(int nPort){ m_nReceivePort = nPort;}

		//获取上一次收到视频的时间
		unsigned int GetLastVideoTime() {return m_uLastVideoTime;}

    private:
		
		string m_strRtpHost;
		
		int m_nRtpPort;

		int m_nReceivePort;

		int m_nRtpSocket;

		bool m_bEndThread;	

		string m_strData;

		unsigned short m_uSeq;

		list<string> m_listFrame;

		//存取互斥
		#ifdef WIN32
			CRITICAL_SECTION m_FrameMutex;
			HANDLE  m_hThread;
			bool m_bThreadStopped;
		#else
			pthread_mutex_t m_FrameMutex;
			pthread_t m_nThreadId;
		#endif

		unsigned int m_uSSrc;

		struct sockaddr_in m_udp_addr;

		bool m_bConnect;

		unsigned int m_uLastVideoTime;

		bool m_bReceiveCFHeader;
};

#endif
