// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

/**
    文件：RoadCommunication.h
    功能：
    作者：於锋
    时间：2011-2-22
**/

#ifndef FTPCOMMUNICATION_H
#define FTPCOMMUNICATION_H

#include "global.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (~0)
#endif


enum {

 LS = 0,

 BINARY,

 ASCII,

 PWD,

 CD,

 OPEN,

 CLOSE,

 QUIT,

 LCD,

 LLS,

 LDIR,

 USER,

 SHELL,

 IGNORE_COMMAND,

 GET,

 PUT,

 HELP,

 RHELP,

 SITE,

};


class CFtpCommunication
{
  public:
    CFtpCommunication();
    ~CFtpCommunication();

  public:

    bool DoOpen( char *szHost,int nPort,char *User,char* szPass);
    bool DoLogin( char *User,char* szPass);
    int DoBinary();
    int DoSite(char *command,string& strRetCode);
    int DoNoop();
    int DoMKD(char *chRemoteDirPath);
    bool DoPut( char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite = true,bool bPutFile = true,char* chRemoteDirPath = NULL,bool bMkdir = false);
	bool VideoDoPut( char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite = true,bool bPutFile = true,char* chRemoteDirPath = NULL,bool bMkdir = false);
    bool DoGet(char *chLocalFilePath,char *chRemoteFilePath);
	void DoClose( void );
	bool DoList(char *chRemoteDirPath,string& strFileNameList);
    int  CheckFds();

	void SetConfig(FTP_HOST_INFO *pFtp = NULL);

	string m_strFtpHost;
	string m_strFtpUser;
	string m_strFtpPasswd;
	int m_nFtpPort;

  private:
    int AcceptConnection();
    int GetListenSocket();
    int GetPassivePort(string& strHost,int& nPort);
    int analysis_ipaddr(char *str, char *re_addr, int *re_port);
    void CloseListenSocket( void );

    int ConnectToServer(char *name, int port);
    int CheckControlMsg( char *, int);
    int ReadControlMsg( char *, int);
    int SendControlMsg(char *, int);
    void CloseControlConnection( void );

    int ReadDataMsg( char *szBuffer, int len);
    int SendDataMsg( char *szBuffer, int len);
    void CloseDataConnection( void );

	int GetReply();
	int GetReply(char* szData);
    int GetLine(char* szBuffer);
    bool PutFile(char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite = true);
	bool VideoPutFile(char *chLocalFilePath,char *chRemoteFilePath,string& strData,bool bDosite = true);
	bool GetFile(char *chLocalFilePath,char *chRemoteFilePath);
	int MKDir(char *chRemoteDirPath);
	void DoCD( char *command);
	int DoCWD(char *chPath);
	bool GetList(char *chRemoteDirPath,string& strFileNameList);

  private:

    int  m_nControlSocket;

    int  m_nListenSocket;
    int  m_nDataSocket;

     bool m_bConnected;

     bool m_bLogin;

    int  m_nSendPort;

    int m_nMode;

    pthread_mutex_t m_SendMutex;
};

extern CFtpCommunication g_FtpCommunication;
#endif
