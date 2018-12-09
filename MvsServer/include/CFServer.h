// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _CFSERVER_H_
#define _CFSERVER_H_


#ifdef CFCAMERA


class CCFCommunication
{
public:
	CCFCommunication();
	~CCFCommunication();

	bool Init();
	bool UnInit();
	
	void LoginCFServer();

	unsigned int GetLoginId() {return m_loginID;}

private:

	unsigned int m_loginID;
};
extern CCFCommunication g_CFCommunication;
#endif
#endif
