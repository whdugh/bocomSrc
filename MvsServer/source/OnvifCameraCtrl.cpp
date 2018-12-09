// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#include "OnvifCameraCtrl.h"


COnvifCameraCtrl::COnvifCameraCtrl()
{
	m_iPort = 0;
	m_nCameraType = 0;
	m_fPanTilta =0.0;
	m_fPanTiltb =0.0;
	m_fPanTiltc =0.0;
	m_fPanTiltd =0.0;
	m_nPanTiltZoom = 0;
	m_nRectZoomCount =0;
	pRectZoomInfo = NULL;
	m_nAbsoluteZoomCount = 0;
	pAbsoluteZoomInfo = NULL;
}

COnvifCameraCtrl::~COnvifCameraCtrl()
{
	if(pRectZoomInfo)
	{
		delete pRectZoomInfo;
		pRectZoomInfo = NULL;
	}
	if(pAbsoluteZoomInfo)
	{
		delete pAbsoluteZoomInfo;
		pAbsoluteZoomInfo = NULL;
	}
}

bool COnvifCameraCtrl::Init(int nCameraType,const char *szIp,int iPort)
{
	m_nCameraType = nCameraType;
	m_CameraIp.clear();
	m_CameraIp.append(szIp);
	m_iPort = iPort;
	printf("CameraIP:[%s:%d] \n",m_CameraIp.c_str(),m_iPort);
	LoadConfig("CameraZoomConfig.xml");
	return true;
}

//相对移动
bool COnvifCameraCtrl::PTZRelativeMove(double PanTiltX,double PanTiltY,double ZoomX)
{
	//DH_CAMERA
	if(m_nCameraType == 36)
	{
		return PTZRelativeMoveDH(PanTiltX,PanTiltY,ZoomX);
	}
	else if(m_nCameraType == 14) //SANYO_CAMERA
	{
		return PTZRelativeMoveSanYo(PanTiltX,PanTiltY,ZoomX);
	}
	else
	{
		return false;
	}

}
bool COnvifCameraCtrl::PTZRelativeMoveDH(double PanTiltX,double PanTiltY,double ZoomX)
{
	if(m_PTZServiceAddr.size() == 0)
	{
		if(!GetPTZServiceAddr())
		{
			return false;
		}
	}

	if(m_ProfileToken.size() == 0)
	{
		if(!GetPTZProfileToken())
		{
			return false;
		}
	}


	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}


	char phttp_body[1024] = {0};


	sprintf(phttp_body,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">"
		"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
		"<RelativeMove xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
		"<ProfileToken>%s</ProfileToken>"
		"<Translation>"
		"<PanTilt "
		"x=\"%0.3f\" "
		"y=\"%0.3f\" "
		"space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace\" xmlns=\"http://www.onvif.org/ver10/schema\" />"
		"<Zoom x=\"%0.3f\""
		" space=\"http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace\" xmlns=\"http://www.onvif.org/ver10/schema\" />"
		"</Translation>"
		"</RelativeMove>"
		"</s:Body>"
		"</s:Envelope>",m_ProfileToken.c_str(), PanTiltX, PanTiltY, ZoomX);

	string str_http_body;
	str_http_body.append(phttp_body);


	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/ptz_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());

	string str_http_header;
	str_http_header.append(phttp_header);


	string strsoap;
	strsoap = str_http_header + str_http_body;

	printf("%s \n",strsoap.c_str());

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,2.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char *phttp_buf = new char[1024*1024];
		memset(phttp_buf,0,1024*1024);
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{
			printf("%s\n",phttp_buf);
			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);
			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				CloseSocket(m_socket);
				delete phttp_buf;
				phttp_buf = NULL;
				return false;
			}
			else
			{
				CloseSocket(m_socket);
				delete phttp_buf;
				phttp_buf = NULL;
				return true;
			}

		}
		delete phttp_buf;
		phttp_buf = NULL;
	}

	CloseSocket(m_socket);
	return false;
}

bool COnvifCameraCtrl::PTZRelativeMoveSanYo(double PanTiltX,double PanTiltY,double ZoomX)
{
	return true;
}
//调用预置位
bool COnvifCameraCtrl::PTZGotoPreset(int iPresetToken)
{
	printf("[COnvifCameraCtrl::PTZGotoPreset] %d \n",GetTimeT());
	//DH_CAMERA
	if(m_nCameraType == 36)
	{
		return PTZGotoPresetDH(iPresetToken);
	}
	else if(m_nCameraType == 14) //SANYO_CAMERA
	{
		return PTZGotoPresetSanYo(iPresetToken);
	}
	else
	{
		return false;
	}
}
bool COnvifCameraCtrl::PTZGotoPresetDH(int iPresetToken)
{
	if(m_PTZServiceAddr.size() == 0)
	{
		if(!GetPTZServiceAddr())
		{
			return false;
		}
	}

	if(m_ProfileToken.size() == 0)
	{
		if(!GetPTZProfileToken())
		{
			return false;
		}
	}


	int m_socket;

	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	string str_http_header;

	char phttp_body[1024] = {0};
	sprintf(phttp_body,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\"><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
		"<GotoPreset xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
		"<ProfileToken>%s</ProfileToken>"
		"<PresetToken>%d</PresetToken>"
		"<Speed>"
		"<PanTilt x=\"1\" y=\"1\" xmlns=\"http://www.onvif.org/ver10/schema\"/><Zoom x=\"1\" xmlns=\"http://www.onvif.org/ver10/schema\"/>"
		"</Speed>"
		"</GotoPreset>"
		"</s:Body>"
		"</s:Envelope>",m_ProfileToken.c_str(),iPresetToken);

	string str_http_body;
	str_http_body.append(phttp_body);

	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/ptz_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());


	str_http_header.append(phttp_header);

	string strsoap;
	strsoap = str_http_header + str_http_body;

	printf("%s",strsoap.c_str());
	printf("\r\n");

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,2.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char *phttp_buf = new char[1024*1024];
		memset(phttp_buf,0,1024*1024);
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{
			printf("%s\n",phttp_buf);
			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);
			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				CloseSocket(m_socket);
				delete phttp_buf;
				phttp_buf = NULL;
				return false;
			}
			else
			{
				CloseSocket(m_socket);
				delete phttp_buf;
				phttp_buf = NULL;
				return true;
			}
		}
		delete phttp_buf;
		phttp_buf = NULL;
	}

	CloseSocket(m_socket);
	return false;
}
bool COnvifCameraCtrl::PTZGotoPresetSanYo(int iPresetToken)
{
	return false;
}

//设置预置位
bool COnvifCameraCtrl::PTZSetPreset(int iPresetToken)
{
	if(m_PTZServiceAddr.size() == 0)
	{
		if(!GetPTZServiceAddr())
		{
			return false;
		}
	}

	if(m_ProfileToken.size() == 0)
	{
		if(!GetPTZProfileToken())
		{
			return false;
		}
	}

	int m_socket;

	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	string str_http_header;

	char phttp_body[1024] = {0};
	sprintf(phttp_body,"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
		"<s:Body>"
		"<tptz:SetPreset>"
		"<tptz:ProfileToken>%s</tptz:ProfileToken>"
		"<tptz:PresetName>Preset%02d</tptz:PresetName>"
		"<tptz:PresetToken>%d</tptz:PresetToken>"
		"</tptz:SetPreset>"
		"</s:Body>"
		"</s:Envelope>",m_ProfileToken.c_str(),iPresetToken,iPresetToken);
	string str_http_body;
	str_http_body.append(phttp_body);

	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/ptz_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());


	str_http_header.append(phttp_header);

	string strsoap;
	strsoap = str_http_header + str_http_body;

	printf("%s",strsoap.c_str());
	printf("\r\n");

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,1.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char phttp_buf[1024*1024] = {0};
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{
			printf("%s\n",phttp_buf);
			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);

			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				printf("Request Failed\n");
				CloseSocket(m_socket);
				return false;
			}
			CloseSocket(m_socket);
			return true;
		}
	}

	CloseSocket(m_socket);
	return false;
}
//获取预置位
bool COnvifCameraCtrl::PTZGetPreset(AutoCameraPtzTrack *&m_pPresetInfo,int &m_nPresetCount)
{
	if(m_PTZServiceAddr.size() == 0)
	{
		if(!GetPTZServiceAddr())
		{
			return false;
		}
	}

	if(m_ProfileToken.size() == 0)
	{
		if(!GetPTZProfileToken())
		{
			return false;
		}
	}

	int m_socket;

	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	string str_http_header;


	string str_http_body;
	str_http_body.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">"
		"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
		"<GetPresets xmlns=\"http://www.onvif.org/ver20/ptz/wsdl\">"
		"<ProfileToken>000</ProfileToken>"
		"</GetPresets>"
		"</s:Body>"
		"</s:Envelope>");


	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/ptz_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());


	str_http_header.append(phttp_header);

	string strsoap;
	strsoap = str_http_header + str_http_body;

	printf("%s",strsoap.c_str());
	printf("\r\n");

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,1.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char phttp_buf[1024*1024] = {0};
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{
			printf("%s\n",phttp_buf);
			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);

			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				printf("Request Failed\n");
				CloseSocket(m_socket);
				return false;
			}

			int nPresetCount = 0;

			CMarkup xmlParser(phttp_buf);
			if (xmlParser.FindChildElem("env:Body") || xmlParser.FindChildElem("SOAP-ENV:Body"))
			{
				xmlParser.IntoElem();
				if(xmlParser.FindChildElem("tptz:GetPresetsResponse"))
				{
					xmlParser.IntoElem();
					while(xmlParser.FindChildElem("tptz:Preset"))
					{
						string strProfileToken = xmlParser.GetChildAttrib("token");
						printf("PresetsToken:%s \n",strProfileToken.c_str());
						xmlParser.IntoElem();
						if(xmlParser.FindChildElem("tt:Name"))
						{
							nPresetCount++;
							string strTokenName = xmlParser.GetChildData();
							printf("PresetsName:%s \n",strTokenName.c_str());
						}
						xmlParser.OutOfElem();
					}
					xmlParser.OutOfElem();
				}
				xmlParser.OutOfElem();
			}
			CloseSocket(m_socket);

			printf("nPresetCount=%d\n",nPresetCount);
			if(nPresetCount == 0)
			{
				return true;
			}
			else
			{
				m_nPresetCount = nPresetCount;
				m_pPresetInfo = new AutoCameraPtzTrack[nPresetCount];
				int nPresetCountTmp = 0;

				printf("%s:%d\n",__FUNCTION__,__LINE__);
				CMarkup xmlParser(phttp_buf);
				if (xmlParser.FindChildElem("env:Body") || xmlParser.FindChildElem("SOAP-ENV:Body"))
				{
					printf("%s:%d\n",__FUNCTION__,__LINE__);
					xmlParser.IntoElem();
					if(xmlParser.FindChildElem("tptz:GetPresetsResponse"))
					{
						xmlParser.IntoElem();
						while(xmlParser.FindChildElem("tptz:Preset"))
						{
							string strProfileToken = xmlParser.GetChildAttrib("token");
							printf("PresetsToken:%s \n",strProfileToken.c_str());
							xmlParser.IntoElem();
							if(xmlParser.FindChildElem("tt:Name"))
							{
								string strTokenName = xmlParser.GetChildData();
								printf("PresetsName:%s \n",strTokenName.c_str());
							}

							if(xmlParser.FindChildElem("tt:PTZPosition"))
							{
								xmlParser.IntoElem();

								xmlParser.FindChildElem("tt:PanTilt");
								string strPanTiltX = xmlParser.GetChildAttrib("x");
								printf("strPanTiltX:%s ",strPanTiltX.c_str());

								xmlParser.FindChildElem("tt:PanTilt");
								string strPanTiltY = xmlParser.GetChildAttrib("y");
								printf("strPanTiltY:%s ",strPanTiltY.c_str());

								xmlParser.FindChildElem("tt:Zoom");
								string strZoomX = xmlParser.GetChildAttrib("x");
								printf("ZoomValue:%s \n",strZoomX.c_str());

								m_pPresetInfo[nPresetCountTmp].nTaken = atoi(strProfileToken.c_str());
								m_pPresetInfo[nPresetCountTmp].nPanTiltX = (int)(atof(strPanTiltX.c_str())*1000);
								m_pPresetInfo[nPresetCountTmp].nPanTiltY = (int)(atof(strPanTiltY.c_str())*1000);
								m_pPresetInfo[nPresetCountTmp].nPanTiltZoom = (int)(atof(strZoomX.c_str())*1000);
								nPresetCountTmp++;
								xmlParser.OutOfElem();
							}

							xmlParser.OutOfElem();
						}
						xmlParser.OutOfElem();
					}
					xmlParser.OutOfElem();
				}
			}
			return true;
		}
	}

	CloseSocket(m_socket);
	return false;

}
bool COnvifCameraCtrl::PTZRemovePreset(int iPresetToken)
{
	if(m_PTZServiceAddr.size() == 0)
	{
		if(!GetPTZServiceAddr())
		{
			return false;
		}
	}

	if(m_ProfileToken.size() == 0)
	{
		if(!GetPTZProfileToken())
		{
			return false;
		}
	}

	int m_socket;

	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	string str_http_header;

	char phttp_body[1024] = {0};
	sprintf(phttp_body,"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
		"<s:Body>"
		"<tptz:RemovePreset>"
		"<tptz:ProfileToken>%s</tptz:ProfileToken>"
		"<tptz:PresetToken>%d</tptz:PresetToken>"
		"</tptz:RemovePreset>"
		"</s:Body>"
		"</s:Envelope>",m_ProfileToken.c_str(),iPresetToken);
	string str_http_body;
	str_http_body.append(phttp_body);

	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/ptz_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());


	str_http_header.append(phttp_header);

	string strsoap;
	strsoap = str_http_header + str_http_body;

	printf("%s",strsoap.c_str());
	printf("\r\n");

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,1.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char phttp_buf[1024*1024] = {0};
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{
			printf("%s\n",phttp_buf);
			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);

			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				printf("Request Failed\n");
				CloseSocket(m_socket);
				return false;
			}
			CloseSocket(m_socket);
			return true;
		}
	}

	CloseSocket(m_socket);
	return false;
}
//获取当前云台信息，
bool COnvifCameraCtrl::GetPTZStatus(string &strPanTiltX,string &strPanTiltY,string &strZoomX)
{
	if(m_PTZServiceAddr.size() == 0)
	{
		if(!GetPTZServiceAddr())
		{
			return false;
		}
	}

	if(m_ProfileToken.size() == 0)
	{
		if(!GetPTZProfileToken())
		{
			return false;
		}
	}


	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}


	char phttp_body[1024] = {0};

	sprintf(phttp_body,"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
		"<s:Body>"
		"<tptz:GetStatus>"
		"<tptz:ProfileToken>%s</tptz:ProfileToken>"
		"</tptz:GetStatus>"
		"</s:Body>"
		"</s:Envelope>",m_ProfileToken.c_str());

	string str_http_body;
	str_http_body.append(phttp_body);


	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/ptz_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());

	string str_http_header;
	str_http_header.append(phttp_header);


	string strsoap;
	strsoap = str_http_header + str_http_body;
#ifdef ONVIFPRINTF
	printf("%s \n",strsoap.c_str());
#endif

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,2.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 200 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char *phttp_buf = new char[1024*1024];
		memset(phttp_buf,0,1024*1024);
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{
#ifdef ONVIFPRINTF
			printf("%s\n",phttp_buf);
#endif
			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);
			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				CloseSocket(m_socket);
				delete phttp_buf;
				phttp_buf = NULL;
				return false;
			}


			CMarkup xmlParser(phttp_buf);
			if (xmlParser.FindChildElem("env:Body") || xmlParser.FindChildElem("SOAP-ENV:Body"))
			{
				xmlParser.IntoElem();
				if(xmlParser.FindChildElem("tptz:GetStatusResponse"))
				{
					xmlParser.IntoElem();
					if(xmlParser.FindChildElem("tptz:PTZStatus"))
					{
						xmlParser.IntoElem();
						if(xmlParser.FindChildElem("tt:Position"))
						{
							xmlParser.IntoElem();

							xmlParser.FindChildElem("tt:PanTilt");
							strPanTiltX = xmlParser.GetChildAttrib("x");
							printf("strPanTiltX:%s ",strPanTiltX.c_str());

							xmlParser.FindChildElem("tt:PanTilt");
							strPanTiltY = xmlParser.GetChildAttrib("y");
							printf("strPanTiltY:%s ",strPanTiltY.c_str());

							xmlParser.FindChildElem("tt:Zoom");
							strZoomX = xmlParser.GetChildAttrib("x");
							printf("ZoomValue:%s \n",strZoomX.c_str());

							CloseSocket(m_socket);
							delete phttp_buf;
							phttp_buf = NULL;
							return true;
						}
					}
				}
			}

		}
		delete phttp_buf;
		phttp_buf = NULL;
	}

	CloseSocket(m_socket);
	return false;
}

int COnvifCameraCtrl::WaitConnect(int nSocket, float fTimeout)
{
	int nSecond = int(fTimeout);
	int nUSecond = (int)((fTimeout -  nSecond) * 1000000);
	timeval tv;
	tv.tv_sec = nSecond;
	tv.tv_usec = nUSecond;
	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	FD_SET(nSocket, &fdwrite);
	if (select(nSocket + 1, NULL, &fdwrite, NULL, &tv) <= 0)
	{
		FD_CLR(nSocket, &fdwrite);
		printf("select failed [%s]。\n", strerror(errno));
		return -1;
	}

	if (!FD_ISSET(nSocket, &fdwrite))
	{
		FD_CLR(nSocket, &fdwrite);
		printf("socket is not set [%s]。\n", strerror(errno));
		return -1;
	}

	int error=-1, len;
	len = sizeof(int);
	//select检查的是它是否可写,用getsockopt函数来获取套接口目前的一些信息来判断是否真的是连接上了
	getsockopt(nSocket, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
	if (error == 0)
	{//connect success
		FD_CLR(nSocket, &fdwrite);
		return 0;
	}
	else
	{//connect failed
		printf("getsockopt failed [%s]。\n", strerror(errno));

		FD_CLR(nSocket, &fdwrite);
		return -1;
	}
}
int COnvifCameraCtrl::WaitRecv(int nSocket, float fTimeout)
{
	int nSecond = int(fTimeout);
	int nUSecond = (int)((fTimeout -  nSecond) * 1000000);
	timeval tv;
	tv.tv_sec = nSecond;
	tv.tv_usec = nUSecond;
	fd_set fdread;
	FD_ZERO(&fdread);
	FD_SET(nSocket, &fdread);

	if (select(nSocket + 1, &fdread, NULL, NULL, &tv) <= 0)
	{
		FD_CLR(nSocket, &fdread);
		//pLog->Debug("select socket failed [%s]", strerror(errno));
		return -1;
	}

	if (!FD_ISSET(nSocket, &fdread))
	{
		FD_CLR(nSocket, &fdread);
		printf("socket not set [%s]", strerror(errno));
		return -1;
	}

	FD_CLR(nSocket, &fdread);

	return 0;
}
int COnvifCameraCtrl::CloseSocket(int& fd)
{
	printf("CloseSocket,[%d]!\n",fd);
	if (fd>0)//防止多次close，
	{
		if(shutdown(fd,2) != 0)
		{
			printf("close socket error,[%d:%s]!",errno,strerror(errno));
		}
		close(fd);
	}
	fd=INVALID_SOCKET;
	return 0;
}

bool COnvifCameraCtrl::GetPTZServiceAddr()
{
	int m_socket;

	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	string str_http_header;

	string str_http_body;
	str_http_body.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">"
		"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
		"<GetCapabilities xmlns=\"http://www.onvif.org/ver10/device/wsdl\">"
		"<Category>PTZ</Category>"
		"</GetCapabilities>"
		"</s:Body>"
		"</s:Envelope>");

	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/device_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());

	str_http_header.append(phttp_header);

	string strsoap;
	strsoap = str_http_header + str_http_body;

	printf("%s",strsoap.c_str());
	printf("\r\n");

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,3.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char phttp_buf[1024*1024] = {0};
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{

			printf("%s\n",phttp_buf);

			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);

			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				printf("Request Failed\n");
				CloseSocket(m_socket);
				return false;
			}


			CMarkup xmlParser(phttp_buf);
			if (xmlParser.FindChildElem("env:Body") || xmlParser.FindChildElem("SOAP-ENV:Body"))
			{
				xmlParser.IntoElem();
				if(xmlParser.FindChildElem("tds:GetCapabilitiesResponse"))
				{
					xmlParser.IntoElem();
					if(xmlParser.FindChildElem("tds:Capabilities"))
					{
						xmlParser.IntoElem();
						if(xmlParser.FindChildElem("tt:PTZ"))
						{
							xmlParser.IntoElem();
							xmlParser.FindChildElem("tt:XAddr");
							m_PTZServiceAddr = xmlParser.GetChildData();
							printf("xaddr:%s \n",m_PTZServiceAddr.c_str());
							CloseSocket(m_socket);
							return true;
						}
					}
				}
			}

		}
	}

	CloseSocket(m_socket);
	return false;
}
bool COnvifCameraCtrl::GetPTZProfileToken()
{
	int m_socket;

	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(m_iPort);
	m_sockaddr.sin_addr.s_addr = inet_addr(m_CameraIp.c_str());

	int nCtlMod = 0x01;
	ioctl(m_socket,FIONBIO,&nCtlMod);

	int nRet;
	if((nRet = connect(m_socket,(sockaddr*)&m_sockaddr,sizeof(m_sockaddr))) < 0)
	{
		if(errno != EINPROGRESS)
		{
			printf("connect failed [%s]\n",strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}

	if (WaitConnect(m_socket, 1.0) != 0)
	{
		printf("wait connect failed [%s]\n", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	string str_http_header;

	string str_http_body;
	str_http_body.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">"
		"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
		"<GetProfiles xmlns=\"http://www.onvif.org/ver10/media/wsdl\" />"
		"</s:Body>"
		"</s:Envelope>");

	char phttp_header[200] = {0};
	sprintf(phttp_header,"POST /onvif/ptz_service HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: application/soap+xml;charset=utf-8\r\n"
		"Content-Length: %d\r\n\r\n",m_CameraIp.c_str(),(int)str_http_body.size());

	str_http_header.append(phttp_header);

	string strsoap;
	strsoap = str_http_header + str_http_body;

	printf("%s",strsoap.c_str());
	printf("\r\n");

	if(send(m_socket,strsoap.c_str(),strsoap.size(),MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	if(WaitRecv(m_socket,1.0) == 0)
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 200 * 1000;
		select(0, NULL, NULL, NULL, &tv);

		char phttp_buf[1024*1024] = {0};
		if(recv(m_socket,phttp_buf,1024*1024,0) > 0)
		{

			printf("%s\n",phttp_buf);

			char pResult[30] = {0};
			memcpy(pResult,phttp_buf,30);

			char *pOK = strstr(pResult,"200");
			if(pOK == NULL)
			{
				printf("Request Failed\n");
				CloseSocket(m_socket);
				return false;
			}



			CMarkup xmlParser(phttp_buf);
			if (xmlParser.FindChildElem("env:Body") || xmlParser.FindChildElem("SOAP-ENV:Body"))
			{
				xmlParser.IntoElem();
				if(xmlParser.FindChildElem("trt:GetProfilesResponse"))
				{
					xmlParser.IntoElem();
					if(xmlParser.FindChildElem("trt:Profiles"))
					{
						m_ProfileToken = xmlParser.GetChildAttrib("token");
						printf("ProfileToken:%s \n",m_ProfileToken.c_str());
						xmlParser.IntoElem();
						if(xmlParser.FindChildElem("tt:PTZConfiguration"))
						{
							string strConfigurationToken = xmlParser.GetChildAttrib("token");
							printf("ConfigurationToken:%s \n",strConfigurationToken.c_str());\
							CloseSocket(m_socket);
							return true;
						}
					}
				}
			}
		}
	}

	CloseSocket(m_socket);
	return false;
}

double COnvifCameraCtrl::GetCalculateCameraZoom(double nNowZoom,double nZoom)
{
	//DH CAMERA
	if(m_nCameraType == 36)
	{
		double m_fNowRectZoom = 0.0;
		int m_nNowZoom = (int)(nNowZoom*1000);
		if(m_nNowZoom < 50 )
		{
			m_nNowZoom = 50;
		}

		if( (m_nNowZoom >= 50)&& (m_nNowZoom < 190))
		{
			m_fNowRectZoom = 1.0*nZoom;
		}
		else if( (m_nNowZoom >= 190)&& (m_nNowZoom < 350))
		{
			m_fNowRectZoom = 1.4*nZoom;
		}
		else if( (m_nNowZoom >= 350)&& (m_nNowZoom < 470))
		{
			m_fNowRectZoom = 1.96*nZoom;
		}
		else if( (m_nNowZoom >= 470)&& (m_nNowZoom < 600))
		{
			m_fNowRectZoom = 2.744*nZoom;
		}
		else if( (m_nNowZoom >= 600)&& (m_nNowZoom < 700))
		{
			m_fNowRectZoom = 3.84*nZoom;
		}
		else if( (m_nNowZoom >= 700)&& (m_nNowZoom < 800))
		{
			m_fNowRectZoom = 5.38*nZoom;
		}
		else if( (m_nNowZoom >= 800)&& (m_nNowZoom < 900))
		{
			m_fNowRectZoom = 7.53*nZoom;
		}
		else if( (m_nNowZoom >= 900)&& (m_nNowZoom < 970))
		{
			m_fNowRectZoom = 10.54*nZoom;
		}
		else if( (m_nNowZoom >= 970)&& (m_nNowZoom < 1000))
		{
			m_fNowRectZoom = 14.75*nZoom;
		}
		else
		{
			m_fNowRectZoom = 20.66*nZoom;
		}


		int nCalculateValue = (int)(log(m_fNowRectZoom)/log(1.4));
		printf("COnvifCameraCtrl::GetCalculateCameraZoom: nCalculateValue=%d \n",nCalculateValue);

		double fZoomAbsoluteValue = 0.0;
		
		if(nCalculateValue == 0)
		{
			fZoomAbsoluteValue = 0.05;
		}
		else if(nCalculateValue == 1)
		{
			fZoomAbsoluteValue = 0.211;
		}
		else if(nCalculateValue == 2)
		{
			fZoomAbsoluteValue = 0.391;
		}
		else if(nCalculateValue == 3)
		{
			fZoomAbsoluteValue = 0.5;
		}
		else if(nCalculateValue == 4)
		{
			fZoomAbsoluteValue = 0.633;
		}
		else if(nCalculateValue == 5)
		{
			fZoomAbsoluteValue = 0.75;
		}
		else if(nCalculateValue == 6)
		{
			fZoomAbsoluteValue = 0.844;
		}
		else if(nCalculateValue == 7)
		{
			fZoomAbsoluteValue = 0.93;
		}
		else if(nCalculateValue == 8)
		{
			fZoomAbsoluteValue = 0.984;
		}
		else
		{
			fZoomAbsoluteValue = 1.0;
		}

		return fZoomAbsoluteValue;
	}
	return 0;
}


double COnvifCameraCtrl::GetNowCameraZoom(double nNowZoom)
{
	//DH CAMERA
	if(m_nCameraType == 36)
	{
		double m_fNowRectZoom = 0.0;
		int m_nNowZoom = (int)(nNowZoom*1000);

		if( m_nNowZoom < 190)
		{
			m_fNowRectZoom = 1.0/20;
		}
		else if( (m_nNowZoom >= 190)&& (m_nNowZoom < 350))
		{
			m_fNowRectZoom = 1.4/20;
		}
		else if( (m_nNowZoom >= 350)&& (m_nNowZoom < 470))
		{
			m_fNowRectZoom = 1.96/20;
		}
		else if( (m_nNowZoom >= 470)&& (m_nNowZoom < 600))
		{
			m_fNowRectZoom = 2.744/20;
		}
		else if( (m_nNowZoom >= 600)&& (m_nNowZoom < 700))
		{
			m_fNowRectZoom = 3.84/20;
		}
		else if( (m_nNowZoom >= 700)&& (m_nNowZoom < 800))
		{
			m_fNowRectZoom = 5.38/20;
		}
		else if( (m_nNowZoom >= 800)&& (m_nNowZoom < 900))
		{
			m_fNowRectZoom = 7.53/20;
		}
		else if( (m_nNowZoom >= 900)&& (m_nNowZoom < 970))
		{
			m_fNowRectZoom = 10.54/20;
		}
		else if( (m_nNowZoom >= 970)&& (m_nNowZoom < 1000))
		{
			m_fNowRectZoom = 14.75/20;
		}
		else
		{
			m_fNowRectZoom = 20.66/20;
		}
		return m_fNowRectZoom;

	}
	return 0;
}


int COnvifCameraCtrl::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (int)tv.tv_sec;
}

bool COnvifCameraCtrl::LoadConfig(char *pConfig)
{
	m_fPanTilta =0.0;
	m_fPanTiltb =0.0;
	m_fPanTiltc =0.0;
	m_fPanTiltd =0.0;
	m_nPanTiltZoom = 0;
	m_nRectZoomCount =0;
	m_nAbsoluteZoomCount = 0;

	if(pRectZoomInfo)
	{
		delete pRectZoomInfo;
		pRectZoomInfo = NULL;
	}
	if(pAbsoluteZoomInfo)
	{
		delete pAbsoluteZoomInfo;
		pAbsoluteZoomInfo = NULL;
	}

	CMarkup xmlParser;
	xmlParser.Load(pConfig);
	if (xmlParser.FindChildElem("SOAP-ENV:Body"))
	{
		xmlParser.IntoElem();
		if(xmlParser.FindChildElem("tt:Zoom"))
		{
			xmlParser.IntoElem();
			if(xmlParser.FindChildElem("PanTilt"))
			{
				string strProfileToken = xmlParser.GetChildAttrib("a");
				m_fPanTilta = atof(strProfileToken.c_str());
				strProfileToken = xmlParser.GetChildAttrib("b");
				m_fPanTiltb = atof(strProfileToken.c_str());
				strProfileToken = xmlParser.GetChildAttrib("c");
				m_fPanTiltc = atof(strProfileToken.c_str());
				strProfileToken = xmlParser.GetChildAttrib("d");
				m_fPanTiltd = atof(strProfileToken.c_str()); 
				strProfileToken = xmlParser.GetChildAttrib("x");
				m_nPanTiltZoom = atoi(strProfileToken.c_str()); 
				printf("CameraZoom:%f,%f,%f,%f,%d \n",m_fPanTilta,m_fPanTiltb,m_fPanTiltc,m_fPanTiltd,m_nPanTiltZoom);

			}
			xmlParser.OutOfElem();
		}

		if(xmlParser.FindChildElem("tt:RectZoom"))
		{
			xmlParser.IntoElem();
			if(xmlParser.FindChildElem("tt:Position"))
			{
				string strProfileToken = xmlParser.GetChildAttrib("token");
				m_nRectZoomCount = atoi(strProfileToken.c_str());
				printf("RectZoom token:%d \n",m_nRectZoomCount);
				if(m_nRectZoomCount > 0)
				{
					pRectZoomInfo = new OnvifZoomInfo[m_nRectZoomCount];
				}
				xmlParser.IntoElem();
				for(int i=0;i<m_nRectZoomCount;i++)
				{
					if(xmlParser.FindChildElem("PanTilt"))
					{
						string strProfileToken = xmlParser.GetChildAttrib("x");
						pRectZoomInfo[i].nPanTiltx = atoi(strProfileToken.c_str());
						strProfileToken = xmlParser.GetChildAttrib("y");
						pRectZoomInfo[i].nPanTilty = atoi(strProfileToken.c_str());
						strProfileToken = xmlParser.GetChildAttrib("z");
						pRectZoomInfo[i].fPanTiltz = atof(strProfileToken.c_str());
						printf("x:%d,y:%d,z:%f \n",pRectZoomInfo[i].nPanTiltx,pRectZoomInfo[i].nPanTilty,pRectZoomInfo[i].fPanTiltz);
					}
				}
				xmlParser.OutOfElem();
			}
			xmlParser.OutOfElem();
		}

		if(xmlParser.FindChildElem("tt:AbsoluteZoom"))
		{
			xmlParser.IntoElem();
			if(xmlParser.FindChildElem("tt:Position"))
			{
				string strProfileToken = xmlParser.GetChildAttrib("token");
				m_nAbsoluteZoomCount = atoi(strProfileToken.c_str());
				printf("AbsoluteZoom token:%d \n",m_nAbsoluteZoomCount);
				if(m_nAbsoluteZoomCount > 0)
				{
					pAbsoluteZoomInfo = new OnvifZoomInfo[m_nAbsoluteZoomCount];
				}
				xmlParser.IntoElem();
				for(int i=0;i<m_nAbsoluteZoomCount;i++)
				{
					if(xmlParser.FindChildElem("PanTilt"))
					{
						string strProfileToken = xmlParser.GetChildAttrib("x");
						pAbsoluteZoomInfo[i].nPanTiltx = atoi(strProfileToken.c_str());
						strProfileToken = xmlParser.GetChildAttrib("y");
						pAbsoluteZoomInfo[i].fPanTiltz = atof(strProfileToken.c_str());
						printf("x:%d,y:%f \n",pAbsoluteZoomInfo[i].nPanTiltx,pAbsoluteZoomInfo[i].fPanTiltz);
					}
				}
				xmlParser.OutOfElem();
			}
			xmlParser.OutOfElem();
		}
	}
	return false;
}


float COnvifCameraCtrl::GetPanTilta()
{
	return m_fPanTilta;
}
float COnvifCameraCtrl::GetPanTiltb()
{
	return m_fPanTiltb;
}
float COnvifCameraCtrl::GetPanTiltc()
{
	return m_fPanTiltc;
}
float COnvifCameraCtrl::GetPanTiltd()
{
	return m_fPanTiltd;
}
int   COnvifCameraCtrl::GetPanTiltZoom()
{
	return m_nPanTiltZoom;
}



