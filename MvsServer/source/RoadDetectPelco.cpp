#include "RoadDetectPelco.h"


CRoadDetectPelco::CRoadDetectPelco()
{
#ifdef CAMERAAUTOCTRL
	m_pMvCameraAutoCtrl =  new CMvCameraAutoCtrl();
#endif
	m_uiTiltUpOrDown = 0;
	m_uiPanLeftOrRight = 0;
	m_pImgSrc = NULL;
	memset(szAddrIP,0,50);
	szAddrPort = 0;
}
CRoadDetectPelco::~CRoadDetectPelco(void)
{
	PTZMoveToStop();
	if(m_pImgSrc)
	{
		cvReleaseImageHeader(&m_pImgSrc);
	}
#ifdef CAMERAAUTOCTRL
	if(m_pMvCameraAutoCtrl)
	{
		delete m_pMvCameraAutoCtrl;
	}
#endif
}

int CRoadDetectPelco::InputCameraAddr(const char *pCameraHost,int pCameraPort)
{
	memset(szAddrIP,0,50);
	sprintf(szAddrIP,"%s",pCameraHost);
	szAddrPort = pCameraPort;
	return 0;
}

void CRoadDetectPelco::DetectRegionRect(int Camerawidth ,int Cameraheight, int xPos,int yPos,int width,int height)
{
	printf("%s:%d,width=%d,height=%d\n",__FUNCTION__,__LINE__,Camerawidth , Cameraheight);
	m_CameraWidth = Camerawidth;
	m_Cameraheight = Cameraheight;
	if(m_pImgSrc == NULL)
	{
		m_pImgSrc =  cvCreateImageHeader(cvSize(m_CameraWidth,m_Cameraheight),8,3);
	}

	m_RectInput.x = xPos;
	m_RectInput.y = yPos;
	m_RectInput.width =  width;
	m_RectInput.height = height;
	printf("ptz begin,%d,%d,%d,%d \n",m_RectInput.x,m_RectInput.y,m_RectInput.width,m_RectInput.height);

	m_iBeginAdjustRect = 0;
	m_bFristMark = true;
	m_nbMoveOperate = true;
	m_bBeginDetect = false;
	m_iMarkTime = GetTimeT();
	m_uiResponse = 0;
	m_nErrCount = 0;
}
//开始控制云台移动及拉近操作 
int CRoadDetectPelco::DealTrackAutoRect(string frame)
{
	if(m_nErrCount >= 40)
	{
		return -1;
	}

	SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());
	double uFrameTime = (double)sDetectHeader.uTime64/1000000.0;
	printf("[%s]: sDetectHeader.uTimestamp=%d,uFrameTime=%0.02f,m_iMarkTime=%d \n",__FUNCTION__,sDetectHeader.uTimestamp,uFrameTime ,m_iMarkTime);

	if(m_iBeginAdjustRect == 0)
		m_nbMoveOperate = true;
	else
		m_nbMoveOperate = false;

	//目标已放大，4秒钟后停止检测，并返回预置位
	if( (m_iBeginAdjustRect == 2 ) )
	{
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		if(abs( (int)(GetTimeT() -m_iMarkTime) ) > 10)
		{
			LogNormal("[%s]: TimeOut goto num 1.....\n",__FUNCTION__);
			return -1;
		}
		else
		{
			bool bCamStop = false;
			CvRect rectNow, rectRecommend;
			float fScale2Init = 1.0;
			//当前框选位置

			cvSetData(m_pImgSrc,(char *)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_pImgSrc->widthStep);
#ifdef CAMERAAUTOCTRL
			Mv3DCamTrackRes CamMTRes;
			m_pMvCameraAutoCtrl->mvRectImageTrack(CamMTRes, m_pImgSrc, m_RectInput, uFrameTime, false,m_nbMoveOperate,1.0);
			CamMTRes.mvGetVar(rectNow, rectRecommend, fScale2Init, bCamStop);
#endif
			printf("rectNow.x=%d,rectNow.y=%d,rectNow.width=%d,rectNow.height=%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
			if ( (rectNow.x == -10000)|| (rectNow.x == -1000) )
			{
				printf("[%s]: lost goal ,continue Detect 1.....\n",__FUNCTION__);
				return  1;
			}
			//3秒以前的数据不使用，防止聚焦不准
			printf("[%s]: fScale2Init=%0.3f.....\n",__FUNCTION__,fScale2Init);
			if(abs( (int)(GetTimeT() -m_iMarkTime) ) <= 3)
			{
				m_RectInput.x = 0;
				m_RectInput.y = 0;
				m_RectInput.width = 0;
				m_RectInput.height = 0;
				return 1;
			}

			//小于1/3，不做车牌检测
			if(m_uiResponse < 2)
			{
				if(abs( (int)(GetTimeT() -m_iMarkTime) ) <= 5)
				{
					return 1;
				}

				printf("%s:%d\n",__FUNCTION__,__LINE__);
				m_RectInput.x = rectNow.x;
				m_RectInput.y = rectNow.y;
				m_RectInput.width = rectNow.width;
				m_RectInput.height = rectNow.height;

				LogNormal("zoom 1,%d,%d,%d,%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
				m_bFristMark = true;        
				m_bBeginDetect = false;      
				m_iBeginAdjustRect = 0;  
				m_iMarkTime = GetTimeT();
			}
			else
			{
#ifdef CAMERAAUTOCTRL
				CvRect rctRecommend;
				m_pMvCameraAutoCtrl->mvGetInfoWithPlateDet(rctRecommend,m_pImgSrc,rectNow);
				printf("rectRecommend.x=%d,rectRecommend.y=%d,rectRecommend.width=%d,rectRecommend.height=%d \n",rctRecommend.x,rctRecommend.y,rctRecommend.width,rctRecommend.height);
				if(rctRecommend.width > 0)
				{
					m_RectInput.x = rctRecommend.x;
					m_RectInput.y = rctRecommend.y;
					m_RectInput.width = rctRecommend.width;
					m_RectInput.height = rctRecommend.height;
				}
				else
				{
					m_RectInput.x = rectNow.x;
					m_RectInput.y = rectNow.y;
					m_RectInput.width = rectNow.width;
					m_RectInput.height = rectNow.height;
				}
#endif
				if(abs( (int)(GetTimeT() -m_iMarkTime) ) <= 7)
				{
					return 1;
				}

				LogNormal("zoom 1,%d,%d,%d,%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
				m_bFristMark = true;        
				m_bBeginDetect = false;      
				m_iBeginAdjustRect = 0;  
				m_iMarkTime = GetTimeT();
			}
			return 1;
		}
	}

#ifdef CAMERAAUTOCTRL
	Mv3DCamTrackRes CamMTRes;
	cvSetData(m_pImgSrc,(char *)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_pImgSrc->widthStep);
	if(m_bFristMark)
	{
		m_pMvCameraAutoCtrl->mvRectImageTrack(CamMTRes, m_pImgSrc, m_RectInput, uFrameTime, true,m_nbMoveOperate,1.0);
		m_bFristMark = false;
	}
	else
	{
		m_pMvCameraAutoCtrl->mvRectImageTrack(CamMTRes, m_pImgSrc, m_RectInput, uFrameTime, false,m_nbMoveOperate,1.0);
	}
#endif

	float fScale2Init = 1.0;
	bool bCamStop = false;
	CvRect rectNow, rectRecommend;
#ifdef CAMERAAUTOCTRL
	CamMTRes.mvGetVar(rectNow, rectRecommend, fScale2Init, bCamStop);
#endif
	printf("rectNow.x=%d,rectNow.y=%d,rectNow.width=%d,rectNow.height=%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);

	//球机还在移动
	if(!bCamStop)
	{
		printf("[%s] Camera Moveing..... \n",__FUNCTION__);
		m_nErrCount++;
		return 1;
	}

	if ( (rectNow.x == -10000)|| (rectNow.x == -1000) || (rectNow.width == 0 ) || (rectNow.height == 0 ) )
	{
		m_nErrCount++;
		printf("[%s]: lost goal ,end Detect.....\n",__FUNCTION__);
		return 1;
	}

	m_nErrCount = 0;
	if(m_iBeginAdjustRect == 0)
	{
		if(IsCentreRectResemble(rectNow)) 
		{
			m_RectInput.x = rectNow.x;
			m_RectInput.y = rectNow.y;
			m_RectInput.width = rectNow.width;
			m_RectInput.height = rectNow.height;

			printf("[%s]Rect Centre in image Centre..... \n",__FUNCTION__);
			//表示已经找到中心位置 可以开始放大了
#ifdef CAMERAAUTOCTRL
			m_uiResponse = CalculateRectPtzZoomEX(m_RectInput);
			LogNormal("zoom 1.0 \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
			m_pMvCameraAutoCtrl->mvSetBeforeCameraScale(sDetectHeader.uTimestamp,rectNow,1.0,2.5);
#endif
			if(m_uiResponse > 0)
			{
				//开始放大
				sleep(1);
				CalculateRectAndMovePtzOrZoom(rectNow,true);
				m_iBeginAdjustRect = 2;
				m_iMarkTime = GetTimeT();
			}
			else
			{
				printf("%s:%d\n",__FUNCTION__,__LINE__);
				return 0;
			}
			return 1;
		}
		else
		{
			if(!m_bBeginDetect)
			{
				CalculateRectAndMovePtzOrZoom(rectNow);
				m_bBeginDetect = true;
				m_iCameraMoveTime = GetTimeT();
			}

			if( abs((int)GetTimeT() - m_iCameraMoveTime) >= 2 )
			{
				LogNormal("ptz,%d,%d,%d,%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
				m_RectInput.x = rectNow.x;
				m_RectInput.y = rectNow.y;
				m_RectInput.width = rectNow.width;
				m_RectInput.height = rectNow.height;
				m_bBeginDetect = false;
			}

			return 1;
		}
	}
	return 1;
}

int CRoadDetectPelco::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}

bool CRoadDetectPelco::IsCentreRectResemble(CvRect rectNew)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	//区域中心点
	int iwidth  =  rectNew.x + rectNew.width/2;
	int iheight =  rectNew.y + rectNew.height/2;
	//int m_nExtentX = 50;
	if( abs(iwidth - m_CameraWidth/2) < m_CameraWidth/10  )
	{
		if( abs(iheight -m_Cameraheight/2)  < m_Cameraheight/10  )
		{
			printf("%s:%d\n",__FUNCTION__,__LINE__);
			return true;
		}
	}
	return false;
}
int CRoadDetectPelco::CalculateRectPtzZoomEX(CvRect rectNew)
{
	
	if(rectNew.width > rectNew.height)
	{
		double uiRectArea = m_CameraWidth/(rectNew.width*(1000.0/g_ytControlSetting.nRemotePicInterval));
		if( m_CameraWidth/(rectNew.width*1.0) < 3.0)
		{
			m_uiResponse = 2;
		}
		else
		{
			m_uiResponse = 1;
		}

		if( (int)(uiRectArea*10) <= 10)
		{
			m_uiResponse = 0;
		}
	}
	else
	{
		double uiRectArea = m_Cameraheight/(rectNew.height*(1000.0/g_ytControlSetting.nRemotePicInterval));
		
		if( m_Cameraheight/(rectNew.height*1.0) < 3.0)
		{
			m_uiResponse = 2;
		}
		else
		{
			m_uiResponse = 1;
		}

		if( (int)(uiRectArea*10) <= 10)
		{
			m_uiResponse = 0;
		}
	}


	return m_uiResponse;
}


bool CRoadDetectPelco::IsRectResemble(CvRect rectOld,CvRect rectNew)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	//区域中心点
	int iwidth  =  rectNew.x + rectNew.width/2;
	int iheight =  rectNew.y + rectNew.height/2;
	if( ( abs(rectOld.x - rectNew.x) > 20 ) ||  ( abs(rectOld.y - rectNew.y) > 20 ))
	{
		//图像已经移动
		return true;
	}
	return false;
}

bool CRoadDetectPelco::CalculateRectAndMovePtzOrZoom(CvRect rectNew,bool bZoom)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	if(bZoom)
	{
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		PTZMoveToZoom();
	}
	else
	{
		printf("%s:%d\n",__FUNCTION__,__LINE__);
		//区域中心点
		int iwidth  =  rectNew.x + rectNew.width/2;
		int iheight =  rectNew.y + rectNew.height/2;
		//int m_nExtentX = 50;
		if( abs(iwidth - m_CameraWidth/2) < m_CameraWidth/10  )
		{
			m_uiPanLeftOrRight = 0;
		}
		else
		{
			m_uiPanLeftOrRight = 1;
		}

		if( abs(iheight -m_Cameraheight/2)  < m_Cameraheight/10  )
		{
			m_uiTiltUpOrDown = 0;
		}
		else
		{
			m_uiTiltUpOrDown = 1;
		}

		if(m_uiPanLeftOrRight == 1)
		{
			if(iwidth < m_CameraWidth/2)
			{
				int nCount = 1;
				if( (m_CameraWidth/2 - iwidth) >=  m_CameraWidth/4)
				{
					nCount = 2;
				}
				PTZMoveToPanTiltLeft();
				if(nCount == 2)
				{
					usleep(PELCO_PANTILT_TIME_LANG);
				}
				else
				{
					usleep(PELCO_PANTILT_TIME_SHORT);
				}
				PTZMoveToStop();
			}
			else
			{
				int nCount = 1;
				if( (iwidth - m_CameraWidth/2) >=  m_CameraWidth/4)
				{
					nCount = 2;
				}
				PTZMoveToPanTiltRight();
				if(nCount == 2)
				{
					usleep(PELCO_PANTILT_TIME_LANG);
				}
				else
				{
					usleep(PELCO_PANTILT_TIME_SHORT);
				}
				PTZMoveToStop();
			}
		}

		if(m_uiTiltUpOrDown == 1)
		{
			if(iheight  < m_Cameraheight/2)
			{
				int nCount = 1;
				if( (m_Cameraheight/2 - iheight) >=  m_Cameraheight/4)
				{
					nCount = 2;
				}
				PTZMoveToPanTiltUp();
				if(nCount == 2)
				{
					usleep(PELCO_PANTILT_TIME_LANG);
				}
				else
				{
					usleep(PELCO_PANTILT_TIME_SHORT);
				}
				PTZMoveToStop();
			}
			else
			{
				int nCount = 1;
				if( (iheight - m_Cameraheight/2) >=  m_Cameraheight/4)
				{
					nCount = 2;
				}
				PTZMoveToPanTiltDown();
				if(nCount == 2)
				{
					usleep(PELCO_PANTILT_TIME_LANG);
				}
				else
				{
					usleep(PELCO_PANTILT_TIME_SHORT);
				}
				PTZMoveToStop();
			}
		}
	}
	return true;
}

bool CRoadDetectPelco::GotoPreset(int uiPresetNum)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(1200);
	m_sockaddr.sin_addr.s_addr = inet_addr(szAddrIP);

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

	unsigned char protoclCmd[16];
	protoclCmd[0] = 0xff;
	protoclCmd[1] = 0x01;
	protoclCmd[2] = 0x00;
	protoclCmd[3] = 0x07;
	protoclCmd[4] = 0x00;
	protoclCmd[5] = uiPresetNum;
	protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
	printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);

	if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	CloseSocket(m_socket);
	return true;
}
bool CRoadDetectPelco::PTZMoveToPanTiltUp()
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(1200);
	m_sockaddr.sin_addr.s_addr = inet_addr(szAddrIP);

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

	unsigned char protoclCmd[16];
	protoclCmd[0] = 0xff;
	protoclCmd[1] = 0x01;
	protoclCmd[2] = 0x00;
	protoclCmd[3] = 0x08;
	protoclCmd[4] = 0x00;
	protoclCmd[5] = 0x08;
	protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
	printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);

	if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	CloseSocket(m_socket);
	return true;
}
bool CRoadDetectPelco::PTZMoveToPanTiltDown()
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(1200);
	m_sockaddr.sin_addr.s_addr = inet_addr(szAddrIP);

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

	unsigned char protoclCmd[16];
	protoclCmd[0] = 0xff;
	protoclCmd[1] = 0x01;
	protoclCmd[2] = 0x00;
	protoclCmd[3] = 0x10;
	protoclCmd[4] = 0x00;
	protoclCmd[5] = 0x08;
	protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
	printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);

	if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	CloseSocket(m_socket);
	return true;
}
bool CRoadDetectPelco::PTZMoveToPanTiltLeft()
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(1200);
	m_sockaddr.sin_addr.s_addr = inet_addr(szAddrIP);

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

	unsigned char protoclCmd[16];
	protoclCmd[0] = 0xff;
	protoclCmd[1] = 0x01;
	protoclCmd[2] = 0x00;
	protoclCmd[3] = 0x04;
	protoclCmd[4] = 0x08;
	protoclCmd[5] = 0x00;
	protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
	printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);

	if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	CloseSocket(m_socket);
	return true;
}
bool CRoadDetectPelco::PTZMoveToPanTiltRight()
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(1200);
	m_sockaddr.sin_addr.s_addr = inet_addr(szAddrIP);

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

	unsigned char protoclCmd[16];
	protoclCmd[0] = 0xff;
	protoclCmd[1] = 0x01;
	protoclCmd[2] = 0x00;
	protoclCmd[3] = 0x02;
	protoclCmd[4] = 0x08;
	protoclCmd[5] = 0x00;
	protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
	printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);

	if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	CloseSocket(m_socket);
	return true;
}
bool CRoadDetectPelco::PTZMoveToZoom()
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(1200);
	m_sockaddr.sin_addr.s_addr = inet_addr(szAddrIP);

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

	//放大操作
	//pelco
// 	{
// 		unsigned char protoclCmd[16];
// 		protoclCmd[0] = 0xff;
// 		protoclCmd[1] = 0x01;
// 		protoclCmd[2] = 0x00;
// 		protoclCmd[3] = 0x20;
// 		protoclCmd[4] = 0x00;
// 		protoclCmd[5] = 0x00;
// 		protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
// 		printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);
// 
// 		if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
// 		{
// 			printf("send buff failed [%s]", strerror(errno));
// 			CloseSocket(m_socket);
// 			return false;
// 		}
// 	}

	{
		unsigned char protoclCmd[16];
		protoclCmd[0] = 0x81;
		protoclCmd[1] = 0x01;
		protoclCmd[2] = 0x04;
		protoclCmd[3] = 0x07;
		protoclCmd[4] = 0x21;
		protoclCmd[5] = 0xff;
		printf("%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5]);

		if(send(m_socket,protoclCmd,6,MSG_NOSIGNAL) < 0)
		{
			printf("send buff failed [%s]", strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}
	usleep(1000*1000);

	{
		unsigned char protoclCmd[16];
		protoclCmd[0] = 0x81;
		protoclCmd[1] = 0x01;
		protoclCmd[2] = 0x04;
		protoclCmd[3] = 0x07;
		protoclCmd[4] = 0x00;
		protoclCmd[5] = 0xff;
		printf("%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5]);

		if(send(m_socket,protoclCmd,6,MSG_NOSIGNAL) < 0)
		{
			printf("send buff failed [%s]", strerror(errno));
			CloseSocket(m_socket);
			return false;
		}
	}


// 	usleep(PELCO_ZOOM_TIME);
// 	//停止一切操作
// 	{
// 		unsigned char protoclCmd[16];
// 		protoclCmd[0] = 0xff;
// 		protoclCmd[1] = 0x01;
// 		protoclCmd[2] = 0x00;
// 		protoclCmd[3] = 0x00;
// 		protoclCmd[4] = 0x00;
// 		protoclCmd[5] = 0x00;
// 		protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
// 		printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);
// 
// 		if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
// 		{
// 			printf("send buff failed [%s]", strerror(errno));
// 			CloseSocket(m_socket);
// 			return false;
// 		}
// 
// 	}

	CloseSocket(m_socket);
	return true;
}

bool CRoadDetectPelco::PTZMoveToStop()
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	int m_socket;
	m_socket = socket(AF_INET,SOCK_STREAM,0);
	if(m_socket < 0)
	{
		printf("build socket failed\n");
		return false;
	}

	sockaddr_in m_sockaddr;
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(1200);
	m_sockaddr.sin_addr.s_addr = inet_addr(szAddrIP);

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

	unsigned char protoclCmd[16];
	protoclCmd[0] = 0xff;
	protoclCmd[1] = 0x01;
	protoclCmd[2] = 0x00;
	protoclCmd[3] = 0x00;
	protoclCmd[4] = 0x00;
	protoclCmd[5] = 0x00;
	protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
	printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);

	if(send(m_socket,protoclCmd,7,MSG_NOSIGNAL) < 0)
	{
		printf("send buff failed [%s]", strerror(errno));
		CloseSocket(m_socket);
		return false;
	}

	CloseSocket(m_socket);
	return true;
}


int CRoadDetectPelco::WaitConnect(int nSocket, float fTimeout)
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
		printf("select failed [%s]...\n", strerror(errno));
		return -1;
	}

	if (!FD_ISSET(nSocket, &fdwrite))
	{
		FD_CLR(nSocket, &fdwrite);
		printf("socket is not set [%s]...\n", strerror(errno));
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
		printf("getsockopt failed [%s]...\n", strerror(errno));

		FD_CLR(nSocket, &fdwrite);
		return -1;
	}
}

int CRoadDetectPelco::CloseSocket(int& fd)
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




