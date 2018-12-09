// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifdef SIP_SERVICE_OPEN

#include "Common.h"
#include "SipService.h"
#include "XmlParser.h"
#include <iostream>

#define BUFSIZE 4096

const char *g_AllowFun = "INVITE, BYE, CANCEL, ACK";

//通讯服务
CSipService g_sipService;

/*
* 函数介绍：接收消息线程入口
* 输入参数：pArg-线程入口参数，接收套接字
* 输出参数：无
* 返回值 ：无
*/
void *RecvMsgThread(void *pArg)
{
    while (!g_bEndThread)
    {
        g_sipService.mvRecvCenterServerMsg();

        usleep(100);
    }

    g_sipService.m_uRecvMsgThreadId = (pthread_t)0;
    pthread_exit((void *)0);
}


//构造
CSipService::CSipService()
{
    m_nCenterSocket = 0;
	m_nUDPSocket = 0;
	m_uRecvMsgThreadId = 0;
	m_bCenterLink = false;
	m_strRtspServerIp = "127.0.0.1";
}

//析构
CSipService::~CSipService()
{
}

//启动侦听服务
bool CSipService::Init()
{
	cerr<<"CSipService::Init "<<endl;
	if(loadSipCfg() == false)
	{
		LogError("载入SIP服务配置文件失败!\n");
		cerr<<"Sip: LoadCfg error!"<<endl;
		return false;
	}
	// 初始化 sip
	int ret = eXosip_init ();
	if (ret != 0)
	{
		cerr << "\n\t--> Can't initialize eXosip!\n"; 
		return false;
	}
	else
	{
		cerr << "\n\t--> eXosip_init successfully!\n"; 
	}

	//监听sip
	ret = eXosip_listen_addr (IPPROTO_UDP, NULL, 5060, AF_INET, 0); 
	if (ret != 0) 
	{
		eXosip_quit ();
		cerr << "\n\t--> eXosip_listen_addr error! Couldn't initialize transport layer!\n";
		return false;
	}

cerr<<"user="<<m_strTelNumber<<endl;
cerr<<"passwd="<<m_strPassword<<endl;
	string localSip = "sip:" + m_strTelNumber + "@" + g_ServerHost + ":5060";
	string server = "sip:" + m_strSipServerIp;

	eXosip_lock ();
	//向sip服务器注册
	m_registerId = eXosip_register_build_initial_register(localSip.c_str(), server.c_str(), NULL, 1800, &m_reg);
	if (m_registerId < 0)
	{
		cerr<<"Build register error!"<<endl;
		eXosip_unlock ();
		return false;
	}
//testcode
cerr<<"clear password()="<<eXosip_clear_authentication_info()<<endl;
cerr<<"add password()="<<eXosip_add_authentication_info(m_strTelNumber.c_str(), m_strTelNumber.c_str(), m_strPassword.c_str(), NULL, NULL)<<endl;

	//osip_message_set_allow(m_reg, g_AllowFun);
	ret = eXosip_register_send_register (m_registerId, m_reg);
	eXosip_unlock ();
	
	if (ret < 0)
	{
		cerr<<"send register error!"<<endl;
		return false;
	}

	time(&m_lastTime);
	return true;
}

//释放
bool CSipService::UnInit()
{
	if(m_nCenterSocket > 0)
	{
		shutdown(m_nCenterSocket,2);
		close(m_nCenterSocket);
		m_nCenterSocket = -1;
	}

	if(m_nUDPSocket > 0)
	{
		shutdown(m_nUDPSocket,2);
		close(m_nUDPSocket);
		m_nUDPSocket = -1;
	}

	//注销SIP服务
	eXosip_lock ();
	if (eXosip_register_build_register(m_registerId, 0, &m_reg) < 0)
	{
		cerr<<"Build detele register error!"<<endl;
	}
	if (eXosip_register_send_register (m_registerId, m_reg) < 0)
	{
		cerr<<"Send detele register error!"<<endl;
	}
	cerr<<mvStop()<<" mvStop()"<<endl;//停止播放
	eXosip_quit ();
	eXosip_unlock ();
	return true;
}

//接收中心端消息
bool CSipService::mvRecvCenterServerMsg()
{
    string strMsg("");
    //receive msg and push it into the msg queue.
    if (mvRecvSocketMsg(strMsg))
    {
        mvDealMsg(strMsg);
        return true;
    }
    else
    {
        return false;
    }
}

void CSipService::mvDealMsg(const string &strMsg)
{
	char chSeq = *((strMsg.c_str()+23));

	printf("%c,%c\n",*(strMsg.c_str()),chSeq);

	if(chSeq == '1')
	{
		cerr<<mvSetDescribe()<<" mvSetDescribe()"<<endl;
	}
	else if(chSeq == '2')
	{
		cerr<<mvSetup()<<" mvSetup()"<<endl;
	}
	else if(chSeq == '3')
	{
		cerr<<mvPlay()<<" mvPlay()"<<endl;
	}
}

bool CSipService::mvRecvSocketMsg(string& strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }
    if (m_nCenterSocket <= 0)
    {
        return false;
    }

    MIMAX_HEADER mHeader;

    char chBuffer[SRIP_MAX_BUFFER];

    int nBytes = 0;
    int nLeft = SRIP_MAX_BUFFER; //数据包体长度
    if(nLeft >  0)
    {
		cerr<<"wait recv"<<endl;
        nBytes = recv(m_nCenterSocket, chBuffer, SRIP_MAX_BUFFER, MSG_NOSIGNAL);
		cerr<<"Recv !"<<endl;

        if ( nBytes <= 0)
        {
            m_bCenterLink = false;

            mvCloseSocket(m_nCenterSocket);
            return false;
        }
        //保存数据
        if(nBytes > 0)
        {
            strMsg.append(chBuffer,nBytes);
            nLeft -= nBytes;
        }
    }

	printf("strMsg.size=%d,strMsg=%s",strMsg.size(),strMsg.c_str());

    return (!strMsg.empty());
}

bool CSipService::mvRebMsgAndSend(const string &strMsg,UINT32 uCmdId, UINT32 uCmdFlag)
{
    if (!mvSendMsgToSocket(m_nCenterSocket, strMsg))
    {
        m_bCenterLink = false;

        mvCloseSocket(m_nCenterSocket);
        return false;
    }
    return true;
}

bool CSipService::loadSipCfg()
{
	cerr<<"loadSipCfg() in"<<endl;

	XMLNode xml, setting, subSetting, temp;
	string xmlFile = "./SipServiceCfg.xml";

	//判断xml文件是否存在
	if(access(xmlFile.c_str(),F_OK) != 0)//不存在
	{
		cerr<<10<<endl;
		xml = XMLNode::createXMLTopNode("Setting");
		setting = xml.addChild("SipServerSeting");
		temp = setting.addChild("SipServerIp");
		temp.addText("");
		temp = setting.addChild("TelNumber");
		temp.addText("");
		temp = setting.addChild("TelPassword");
		temp.addText("");
		
		if(!xml.writeToFile(xmlFile.c_str()))
		{
			LogError("生成配置文件模板失败!\r\n");
		}
		else
		{
			LogError("没有配置文件! 生成默认配置文件./SipServiceCfg.xml\n");
		}
		cerr<<12<<endl;
		return false;
	}
	else
	{
		cerr<<20<<endl;
		xml = XMLNode::parseFile(xmlFile.c_str());
		setting = xml.getChildNode("Setting");
		if (!setting.isEmpty())
		{
			cerr<<21<<endl;
			subSetting = setting.getChildNode("SipServerSeting");
			if (!subSetting.isEmpty())
			{
				cerr<<22<<endl;
				temp = subSetting.getChildNode("SipServerIp");
				if (!temp.isEmpty())
				{
					m_strSipServerIp = temp.getText();
				}
				else
				{
					return false;
				}
				temp = subSetting.getChildNode("TelNumber");
				if (!temp.isEmpty())
				{
					m_strTelNumber = temp.getText();
				}
				else
				{
					return false;
				}
				temp = subSetting.getChildNode("TelPassword");
				if (!temp.isEmpty() && temp.getText()!=NULL)
				{
					m_strPassword = temp.getText();
				}
				else
				{
					m_strPassword = "";
				}
				cerr<<23<<endl;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	cerr<<"loadSipCfg() out"<<endl;
	return true;
}


void CSipService::sipService()
{
	cerr<<"sipService() in"<<endl;
	if (m_registerId <= 0)
	{
		cerr<<"m_registerId <= 0"<<endl;
		return;
	}
    eXosip_event_t *je = NULL; 
    osip_message_t *invite = NULL; 
    osip_message_t *answer = NULL; 
    sdp_message_t *remote_sdp = NULL; 
	
	char buf[BUFSIZE] = {0}; 

	//非阻塞侦听是否有消息到来 
	je = eXosip_event_wait (0, 50);

	// 协议栈带有此语句,具体作用未知 
	eXosip_lock (); 
	eXosip_default_action (je); 
	eXosip_automatic_refresh (); 
	eXosip_unlock ();

	//取得当前时间
	time_t now;
	time(&now);

	int ret;
	eXosip_lock ();
	if (now - m_lastTime > 30) //30秒做一次心跳保活
	{
		if (eXosip_register_build_register(m_registerId, 1800, &m_reg) < 0)
		{
			cerr<<"Build register update error!"<<endl;
			return;
		}
		ret = eXosip_register_send_register (m_registerId, m_reg);			
		if (ret < 0)
		{
			cerr<<"Send register update error!"<<endl;
			return;
		}
		m_lastTime = now;
	}
	eXosip_unlock ();

	if (je == NULL) // 没有接收到消息，继续 
	{ 
		return; 
	} 

	switch (je->type) 
	{
	case EXOSIP_REGISTRATION_SUCCESS:
		cerr<<"\n\t-->Sip service registe success!"<<endl;
		break;
	case EXOSIP_REGISTRATION_FAILURE:
		cerr<<"\n\t-->Sip service registe failed! je->response->status_code="<<je->response->status_code<<endl;
/*
		eXosip_lock ();
		cerr<<"clear password()="<<eXosip_clear_authentication_info()<<endl;
		cerr<<"add password()="<<eXosip_add_authentication_info(m_strTelNumber.c_str(), m_strTelNumber.c_str(), m_strPassword.c_str(), NULL, NULL)<<endl;

		ret = eXosip_register_build_register(je->rid, 1800, &m_reg);
		cerr<<"\n\t-->build_register return="<<ret<<endl;

		ret = eXosip_register_send_register (m_registerId, m_reg);	
		eXosip_unlock ();
		cerr<<"\n\t-->send_register return="<<ret<<endl;
*/
		break;

	case EXOSIP_CALL_INVITE: // INVITE 请求消息 
		// 得到消息体,认为该消息就是 SDP 格式
		remote_sdp = eXosip_get_remote_sdp (je->did); 
		
		eXosip_lock (); 
		//发送180 Trying回复
		//eXosip_call_send_answer (je->tid, 180, NULL); 

		//建立200 OK回复
		ret = eXosip_call_build_answer (je->tid, 200, &answer); 
		if (ret != 0) 
		{ 
			cerr << "\n\t--> This request msg is invalid! Cann't response!\n" << endl; 
			eXosip_call_send_answer (je->tid, 400, NULL); 
		} 
		else 
		{
			cerr<<"SDP INFO:"<<endl
				<<"o_addrtype="<<remote_sdp->o_addrtype<<endl
				<<"o_addr="<<remote_sdp->o_addr<<endl;

			//保存client的ip、端口
			m_strRtpClientIp = remote_sdp->o_addr;

			sdp_media_t* media = eXosip_get_video_media(remote_sdp);
			if (media != NULL)
			{
				cerr<<"m_port="<<media->m_port<<endl;
				m_rtpClentPort = atoi(string(media->m_port).c_str());
			}
			else
			{
				media = eXosip_get_audio_media(remote_sdp);
				cerr<<"m_port="<<media->m_port<<endl;
				m_rtpClentPort = atoi(string(media->m_port).c_str());
			}

			//如果不是IP4类型
			if(strcmp(remote_sdp->o_addrtype, "IP4") != 0)
			{
				cerr<<"IP type is not 'IP4'"<<endl;
				eXosip_call_send_answer (je->tid, 400, answer);
				eXosip_unlock ();
				break;
			}

			snprintf (buf, BUFSIZE,
				"v=0\n"
				"o=mediasip 123456 0 IN IP4 %s\n"
				"s=mediasip\n"
				"c=IN IP4 %s\n"
				"t=0 0\n"
				"m=video 18888 RTP/AVP 96\n"
				"a=rtpmap:96 H264/90000\n"
				"a=fmtp:96 packetization-mode=1;profile-level-id=000042;sprop-parameter-sets=Z0LAHqtAZAm/LCAAAAMAIAAAAwFR4sXU,aM48gA==\n", g_ServerHost.c_str(), g_ServerHost.c_str()); 

			cerr<<buf<<endl;

			if (osip_message_set_body (answer, buf, strlen(buf)) < 0) 
			{
				cerr<<"osip_message_set_body error!"<<endl;
				eXosip_unlock ();
				break;
			}
			if (osip_message_set_content_type (answer, "application/sdp") < 0)
			{
				cerr<<"osip_message_set_content_type error!"<<endl;
				eXosip_unlock ();
				break;
			}
			if (eXosip_call_send_answer (je->tid, 200, answer) < 0)
			{
				cerr<<"eXosip_call_send_answer error!"<<endl;
				eXosip_unlock ();
				break;
			}
			cerr << "\n\t--> send 200 over!" << endl; 
		}
		eXosip_unlock (); 

		//向RTP服务器发送视频请求
		cerr<<mvSetOption()<<" mvSetOption()"<<endl;
		break; 
	case EXOSIP_CALL_ACK: 
		cerr << "\n\t--> ACK recieved!\n" << endl; 
		break;
	case EXOSIP_CALL_CLOSED: 
		cerr << "\n\t--> the remote hold the session!\n" << endl; 
		eXosip_lock ();
		ret = eXosip_call_build_answer (je->tid, 200, &answer); 
		if (ret < 0) 
		{ 
			printf ("This request msg is invalid! Send 400 response!\n"); 
			cerr<<"This request msg is invalid! Send 400 response!"<<endl;
			eXosip_call_send_answer (je->tid, 400, NULL); 
		} 
		else 
		{
			eXosip_call_send_answer (je->tid, 200, answer); 

			cerr<<mvStop()<<" mvStop()"<<endl;//停止播放
			cerr << "\n\t--> bye send 200 over!\n"; 
		} 
		eXosip_unlock ();
		
		break; 

	default: 
		cerr << "\n\t--> Received unknown msg! Code=" <<je->type<< endl; 
	}

	cerr<<"sipService() out"<<endl;
}

/*
* 函数介绍：开启SIP服务,收到SIP请求后 要求RTSP服务器向SIP对端发送视频数据
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CSipService::mvConnOrLinkTest()
{
    if (!g_bEndThread)
    {
        if (!m_bCenterLink)
        {
            if (!mvPrepareSocket(m_nCenterSocket))
            {
                printf("\n准备连接RTP视频服务器套接字失败!\n");
				cerr<<"Sip: mvPrepareSocket() error!"<<endl;
                return false;
            }
            printf("======m_strHost=%s\n",m_strRtspServerIp.c_str());
			cerr<<"======m_strHost="<<m_strRtspServerIp.c_str()<<endl;
            if (!mvWaitConnect(m_nCenterSocket, m_strRtspServerIp, 8554,2))
            {
                printf("\n尝试连接RTP视频服务器失败!\n");
				cerr<<"Sip: mvWaitConnect() error!"<<endl;
                return false;
            }

            if (!mvStartRecvThread())
            {
				cerr<<"Sip: mvStartRecvThread() error!"<<endl;
                return false;
            }
            m_bCenterLink = true;
        }

		sipService();
    }

    return true;
}

/*
* 函数介绍：接收消息线程入口
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CSipService::mvStartRecvThread()
{
    pthread_attr_t   attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    mvJoinThread(m_uRecvMsgThreadId);
    if (pthread_create(&m_uRecvMsgThreadId, &attr, RecvMsgThread, NULL) != 0)
    {
		cerr<<"Sip: pthread_create() error!"<<endl;
        pthread_attr_destroy(&attr);
        return false;
    }

    pthread_attr_destroy(&attr);
    return true;
}

//
bool CSipService::mvStop()
{
    string strMsg = "TEARDOWN rtsp://" + m_strRtspServerIp + ":8554/avStream/ RTSP/1.0\r\nCSeq: 5\r\nSession: 1\r\nUser-Agent:\r\n\r\n";
	cerr<<strMsg<<endl;
    return mvRebMsgAndSend(strMsg,0);
}

//
bool CSipService::mvPlay()
{
    string strMsg = "PLAY rtsp://" + m_strRtspServerIp + ":8554/avStream/ RTSP/1.0\r\nCSeq: 4\r\nSession: 1\r\nRange: npt=0.000-\r\nUser-Agent:\r\n\r\n";

	cerr<<strMsg<<endl;
    return mvRebMsgAndSend(strMsg,0);
}

//
bool CSipService::mvSetup()
{
	char rtpPort[10] = {0};
	sprintf(rtpPort, "%d", m_rtpClentPort);

	char rtcpPort[10] = {0};
	sprintf(rtcpPort, "%d", m_rtpClentPort+1);

    string strMsg = "SETUP rtsp://" + m_strRtspServerIp + ":8554/avStream/track1 RTSP/1.0\r\nCSeq: 3\r\nTransport: RTP/AVP;unicast;destination=" +m_strRtpClientIp+";client_port=" +rtpPort+ "-" +rtcpPort+ "\r\nUser-Agent:\r\n\r\n";

	cerr<<strMsg<<endl;
    return mvRebMsgAndSend(strMsg,0);
}


//
bool CSipService::mvSetDescribe()
{
	//LibVLC/1.1.0 (LIVE555 Streaming Media v2010.03.16)
    string strMsg = "DESCRIBE rtsp://" + m_strRtspServerIp + ":8554/avStream RTSP/1.0\r\nCSeq: 2\r\nAccept: application/sdp\r\nUser-Agent:\r\n\r\n";
    cerr<<strMsg<<endl;
	return mvRebMsgAndSend(strMsg,0);
}

//
bool CSipService::mvSetOption()
{
	string strMsg = "OPTIONS rtsp://" + m_strRtspServerIp + ":8554/avStream RTSP/1.0\r\nCSeq: 1\r\nUser-Agent:\r\n\r\n";
    
	cerr<<strMsg<<endl;
    return mvRebMsgAndSend(strMsg,0);
}
#endif
