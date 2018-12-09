// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifdef HK_SIP_SERVICE_OPEN

#ifndef HK_SIPSERVICE_H
#define HK_SIPSERVICE_H

#include "CSocketBase.h"
#include <eXosip2/eXosip.h> 
#include <osip2/osip.h>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

typedef struct _FileList
{
	string FileName;
	string FileBeginTime;
	string FileEndTime;
	string FileSize;
	_FileList()
	{
		FileName = "";
		FileBeginTime = "";
		FileEndTime = "";
		FileSize = "";
	}	
}FileList;


typedef struct _SipID
{
	int cid;
	int did;
	string VideoServerIp;
	string Transport;
	string Port;

	_SipID()
	{
		cid = 0;
		did = 0;
		VideoServerIp = "";
		Transport = "";
		Port = "";
	}
}SipID;

typedef map<string,SipID> mapSipID;

typedef struct _VideoServerIpAndPort
{
	string VideoServerIp;
	string Transport;
	string Port;
	_VideoServerIpAndPort()
	{
		VideoServerIp = "";
		Transport = "";
		Port = "";
	}

}VideoServerIpAndPort;

typedef map<string,VideoServerIpAndPort> mapIpAndPortID; 

#ifndef mapCamerCodeAndListFile
typedef map<string,std::list<FileList> > mapCamerCodeAndListFile;
#endif

#ifndef mapCameraCodeAndHisURL
typedef map<string,string> mapCameraCodeAndHisURL;
#endif


class CHKSipService: public mvCSocketBase
{
public:
	//构造
	CHKSipService();
	//析构
	~CHKSipService();
public:
	//开始侦听服务
	bool Init();
	//释放
	bool UnInit();

	//bool mvConnOrLinkTest();

	void sipService();
	bool loadSipCfg();

	void CreateInviteToXml( string& xmlstr);

	bool CreateInviteCall(string strDestTelNumber,int dport,unsigned int uRandSSrc);
	//向目的联网单元发送INVITE请求历史视频
	bool CreateHistoryInviteCall(string strDestTelNumber,int dport,UINT32 uBeginTime,UINT32 uEndTime);

	bool CreateKeepAliveInviteCall();

	bool CreateListenMessageThread();

	//void CreateQueryDoMsg(int did,osip_message_t *mdo);
	void CreateQueryDoMsg(string nStrDestTelNumber,long HisBeginTime,long HisEndTime);

	//void CreateGetUrlDoMsg(int did,osip_message_t *mdo);
	void CreateGetUrlDoMsg(string nStrDestTelNumber,long HisBeginTime,long HisEndTime,string fileName);

	//void GetRtspIPandPort(char *CameraAddressCode,osip_body_t *body);
	void GetRtspIPandPort(char *CameraAddressCode,sdp_message_t *sdp);

	void CreateKeepAliveDoMsg(int did,osip_message_t *mdo);

	bool UserLogOnServer();

	void LoginHKServer();

	int GetSPort();
	string GetHost();


	int GetSPortBasedOnCameraCode(string CameraCode);
	string GetHostBaseOnCameraCode(string CameraCode);


	void CreateDORequest();

	void ChangTimeFormat(string& mTime,long ltime);

	void GetFileList(string nStrCameraCode,osip_body_t *body);
	void SaveFileToMemory(char* tmp,FileList* tmpFile,int i,int& j,std::list<FileList>& nListFile);

	void GetHisVideoUrl(string nStrCamerCode,osip_body_t *body);

	void TerminateCall();

	void TerminateOneCall(string CameraAddressCode);

	int CallTerminate(int cid, int did);

	int AddSipIDtoMap(char* CameraAddressCode,int cid,int did);

	bool CreateInviteProcThread(eXosip_event_t *je);

	void ProcForInvite200OK(eXosip_event_t *je);

	//pthread_t m_uRecvMsgThreadId;
	bool m_bregFlag;//向Sip服务器注册成功与否标记

	bool m_EndThread;

	bool m_bLogOnServer;//是否登陆成功


private:

	time_t m_lastTime;
	
	std::string m_strSipServerIp;
	std::string m_sourcTelNumber;
	std::string m_strPassword;

	

	int m_registerId;
	osip_message_t *m_reg;

	string m_sourceCall;
	string m_destCall;
	string m_server;
	string m_destIP;
	string m_destTelNumber;
	
	string m_sipserverTelNumber;
	string m_serverCall;

	int m_InviteCount;
	
	//int m_failReqCount;

	int m_cid;
	int m_did;

	string m_destRtspIP;//对方Rtsp服务器的IP
	string m_transport; //解析出来的协议
	string m_destRtspPort;//解析出来的端口 

	mapCamerCodeAndListFile m_CamerCodeAndListFileMap;
	pthread_mutex_t m_CamerCodeAndListFileMap_mutex;

	mapCameraCodeAndHisURL m_CameraCodeAndHisUrlMap;
	pthread_mutex_t m_CameraCodeAndHisUrlMap_mutex;

	mapSipID m_mapSipID;

	mapIpAndPortID m_mapIpPortID;

	pthread_mutex_t m_map_mutex;
	pthread_mutex_t m_mapIpPortID_mutex;

};
//通讯服务
extern CHKSipService g_hk_sipService;
#endif
#endif
