#include "RoadDetectDHCamera.h"


CRoadDetectDHCamera::CRoadDetectDHCamera()
{
#ifdef CAMERAAUTOCTRL
	m_pMvCameraAutoCtrl =  new CMvCameraAutoCtrl();
	m_pMvCalcCamMvScale = new MvCalcCamMvScale();
	m_pOnvifCameraCtrl  = new COnvifCameraCtrl();
#endif
	m_pImgSrc = NULL;
	memset(szAddrIP,0,50);
	szAddrPort = 0;
}
CRoadDetectDHCamera::~CRoadDetectDHCamera(void)
{
	if(m_pImgSrc)
	{
		cvReleaseImageHeader(&m_pImgSrc);
	}
#ifdef CAMERAAUTOCTRL
	if(m_pMvCameraAutoCtrl)
	{
		delete m_pMvCameraAutoCtrl;
	}
	if(m_pMvCalcCamMvScale)
	{
		delete m_pMvCalcCamMvScale;
		m_pMvCalcCamMvScale = NULL;
	}

	if(m_pOnvifCameraCtrl)
	{
		delete m_pOnvifCameraCtrl;
		m_pOnvifCameraCtrl = NULL;
	}
#endif
}

void CRoadDetectDHCamera::SetCameraVar(float fCCD_w,float fCCD_h,float fMinFocus,float fMaxFocus,int nMultiPlicate )
{
#ifdef CAMERAAUTOCTRL
	m_pMvCalcCamMvScale->mvSetCameraVar(fCCD_w,fCCD_h,fMinFocus,fMaxFocus,nMultiPlicate);
#endif
}
int CRoadDetectDHCamera::InputCameraAddr(const char *pCameraHost,int pCameraPort)
{
	memset(szAddrIP,0,50);
	sprintf(szAddrIP,"%s",pCameraHost);
	szAddrPort = pCameraPort;
#ifdef CAMERAAUTOCTRL
	m_pOnvifCameraCtrl->Init(DH_CAMERA,szAddrIP,szAddrPort);
#endif
	return 0;
}
void CRoadDetectDHCamera::DetectRegionRect(int Camerawidth ,int Cameraheight, int xPos,int yPos,int width,int height)
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
	LogNormal("ptz begin,%d,%d,%d,%d \n",m_RectInput.x,m_RectInput.y,m_RectInput.width,m_RectInput.height);

	m_iBeginAdjustRect = 0;
	m_bFristMark = true;
	m_nbMoveOperate = true;
	m_bBeginDetect = false;
	m_iMarkTime = GetTimeT();
	m_nErrCount = 0;
	m_nDetectNum = 0;
	m_nCameraMoveCount = 0;
	m_fScaleRealZoom = 0.0;

}
//开始控制云台移动及拉近操作 
int CRoadDetectDHCamera::DealTrackAutoRect(string frame,int nCarId,int nSaveProcess)
{
	if( TrackAutoCtrl(nCarId,nSaveProcess) == 0)
	{
		LogNormal("nCarId=%d,nSaveProcess=%d \n",nCarId,nSaveProcess);
		LogNormal("TrackAutoCtrl Return \n");
		return 0;
	}

	if(m_nDetectNum == 0)
	{
		if ((m_nErrCount > 30)&& (( GetTimeT()- m_iMarkTime) > 10  ) )
		{
			LogNormal("Check Err,Out \n");
			return -1;
		}
	}

	SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());
	double uFrameTime = (double)sDetectHeader.uTime64/1000000.0;
	printf("[%s]: sDetectHeader.uFrameTime=%0.02f,m_iMarkTime=%d \n",__FUNCTION__,uFrameTime ,m_iMarkTime);

	if(m_iBeginAdjustRect == 0)
	{
		m_nbMoveOperate = true;
	}
	else
	{
		m_nbMoveOperate = false;
	}

	//目标已放大，4秒钟后停止检测，并返回预置位
	printf("BeginAdjustRect=%d,m_nDetectNum=%d \n",m_iBeginAdjustRect,m_nDetectNum);
	if( (m_iBeginAdjustRect == 2 ) && ( m_nDetectNum == 0))
	{
		if(abs( (int)(GetTimeT() -m_iMarkTime) ) > 10)
		{
			LogNormal("[%s]: TimeOut DetectNum=0.....\n",__FUNCTION__);
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
				return 1 ;
			}
			printf("[%s]: fScale2Init=%0.3f.....\n",__FUNCTION__,fScale2Init);
			if(abs( (int)(GetTimeT() -m_iMarkTime) ) <= 4)
			{
				m_RectInput.x = 0;
				m_RectInput.y = 0;
				m_RectInput.width = 0;
				m_RectInput.height = 0;
				return 1;
			}
#ifdef CAMERAAUTOCTRL
			CvRect rctRecommend;
			m_pMvCameraAutoCtrl->mvGetInfoWithPlateDet(rctRecommend,m_pImgSrc,rectNow);
			printf("rectRecommend.x=%d,rectRecommend.y=%d,rectRecommend.width=%d,rectRecommend.height=%d \n",rctRecommend.x,rctRecommend.y,rctRecommend.width,rctRecommend.height);
			if(rctRecommend.x >= 0)
			{
				m_RectInput.x = rctRecommend.x;
				m_RectInput.y = rctRecommend.y;
				m_RectInput.width = rctRecommend.width;
				m_RectInput.height = rctRecommend.height;
			}
#endif
			if(abs( (int)(GetTimeT() -m_iMarkTime) ) <= 7)
			{
				return 1;
			}

			if(m_RectInput.width == 0)
			{
				m_RectInput.x = rectNow.x;
				m_RectInput.y = rectNow.y;
				m_RectInput.width = rectNow.width;
				m_RectInput.height = rectNow.height;
			}

			if( ( (int)(fScale2Init*100) > 100) && bCamStop)
			{
				LogNormal("zoom 1,%d,%d,%d,%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
				m_bFristMark = true;        
				m_bBeginDetect = false;      
				m_iBeginAdjustRect = 0;  
				m_iMarkTime = GetTimeT();
				m_nDetectNum = 1;
			}
			return 1;
		}
	}
	else if( (m_iBeginAdjustRect == 2) && (m_nDetectNum == 1))
	{
		if(abs( (int)(sDetectHeader.uTimestamp -m_iMarkTime) ) > 6)
		{
			LogNormal("[%s]: TimeOut DetectNum=1.....\n",__FUNCTION__);
			return 0;
		}
		else
		{

			float fScale2Init = 1.0;
			bool bCamStop = false;
			CvRect rectNow, rectRecommend;
			cvSetData(m_pImgSrc,(char *)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_pImgSrc->widthStep);
#ifdef CAMERAAUTOCTRL
			Mv3DCamTrackRes CamMTRes;
			m_pMvCameraAutoCtrl->mvRectImageTrack(CamMTRes, m_pImgSrc, m_RectInput, uFrameTime, false,m_nbMoveOperate,1.0);
			CamMTRes.mvGetVar(rectNow, rectRecommend, fScale2Init, bCamStop);
#endif
			printf("rectNow.x=%d,rectNow.y=%d,rectNow.width=%d,rectNow.height=%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
			if ( (rectNow.x == -10000)|| (rectNow.x == -1000) )
			{
				printf("[%s]: lost goal ,goto check 2.....\n",__FUNCTION__);
				return 1;
			}
			if(abs( (int)(GetTimeT() -m_iMarkTime) ) <= 4)
			{
				return 1;
			}

			printf("Scale2 = %0.3f",fScale2Init);
			if( ( (int)(fScale2Init*100) > 100) && bCamStop)
			{
				LogNormal("zoom 2,%d,%d,%d,%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
				printf("[%s] Camera No Move..... \n",__FUNCTION__);
				return 0;
			}
			else
			{
				return 1;
			}


		}
	}

#ifdef CAMERAAUTOCTRL
	Mv3DCamTrackRes CamMTRes;
#endif

	cvSetData(m_pImgSrc,(char *)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_pImgSrc->widthStep);

	if(m_bFristMark)
	{
		printf("[%s] FristMark \n",__FUNCTION__);
#ifdef CAMERAAUTOCTRL
		m_pMvCameraAutoCtrl->mvRectImageTrack(CamMTRes, m_pImgSrc, m_RectInput, uFrameTime, true,m_nbMoveOperate,1.0);
#endif
		m_bFristMark = false;
	}
	else
	{
#ifdef CAMERAAUTOCTRL
		m_pMvCameraAutoCtrl->mvRectImageTrack(CamMTRes, m_pImgSrc, m_RectInput, uFrameTime, false,m_nbMoveOperate,1.0);
#endif
	}

	float fScale2Init = 1.0;
	bool bCamStop = false;
	CvRect rectNow, rectRecommend;
#ifdef CAMERAAUTOCTRL
	CamMTRes.mvGetVar(rectNow, rectRecommend, fScale2Init, bCamStop);
#endif

	printf("rectNow.x=%d,rectNow.y=%d,rectNow.width=%d,rectNow.height=%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
	fflush(stdout);


	//球机还在移动
	if(!bCamStop)
	{
		printf("[%s] Camera Moveing..... \n",__FUNCTION__);
		return 1;
	}

	if ( (rectNow.x == -10000)|| (rectNow.x == -1000) || (rectNow.width == 0 ) || (rectNow.height == 0 ) )
	{
		printf("[%s]: lost goal ,end Detect.....\n",__FUNCTION__);
		m_nErrCount++;
		return 1;
	}

	m_nErrCount = 0;

	if(m_iBeginAdjustRect == 0)
	{
		if((IsCentreRectResemble(rectNow)) || (m_nCameraMoveCount == 4))
		{
			m_RectInput.x = rectNow.x;
			m_RectInput.y = rectNow.y;
			m_RectInput.width = rectNow.width;
			m_RectInput.height = rectNow.height;
			//框选中心点在图片中心点附近
			printf("[%s]Rect Centre in image Centre..... \n",__FUNCTION__);
			//表示已经找到中心位置 可以开始放大了
#ifdef CAMERAAUTOCTRL
			float fRectArea;
			CalculateRectPtzZoomEX(m_RectInput,fRectArea,m_fScaleRealZoom);
			LogNormal("fRectArea=%0.3f,fZoomValue=%0.3f \n",fRectArea,m_fScaleRealZoom);
			if((int)(fRectArea*10) >= 14 )
			{
				double fRoadmaxValue = Roadmax(fRectArea-1,fRectArea/1.4);
				double fRoadminValue = Roadmin(fRectArea+1,fRectArea*1.4);
				LogNormal("min=%0.1f,max=%0.1f \n",fRoadmaxValue,fRoadminValue);
				m_pMvCameraAutoCtrl->mvSetBeforeCameraScale(sDetectHeader.uTimestamp,rectNow,fRoadmaxValue,fRoadminValue);
			}
			else
			{
				if(m_nDetectNum == 1)
				{
					LogNormal("zoom ok,%d,%d,%d,%d \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
					return 0;
				}
				else
				{
					LogNormal("zoom 1.0 \n",rectNow.x,rectNow.y,rectNow.width,rectNow.height);
					m_pMvCameraAutoCtrl->mvSetBeforeCameraScale(sDetectHeader.uTimestamp,rectNow,1.0,1.1);
				}
			}
#endif
			m_nCameraMoveCount = 0;

			{
				//开始放大
				CalculateRectAndMovePtzOrZoom(m_RectInput,nCarId,nSaveProcess,false);
				m_iBeginAdjustRect = 2;
				m_iMarkTime = GetTimeT();
			}
		}
		else
		{
			if(!m_bBeginDetect)
			{
				m_nCameraMoveCount++;
				CalculateRectAndMovePtzOrZoom(rectNow,nCarId,nSaveProcess);
				m_bBeginDetect = true;
				m_iCameraMoveTime = GetTimeT();
			}


			if (IsRectResemble(m_RectInput,rectNow) && bCamStop)
			{
				m_RectInput.x = rectNow.x;
				m_RectInput.y = rectNow.y;
				m_RectInput.width = rectNow.width;
				m_RectInput.height = rectNow.height;
				m_bBeginDetect = false;
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


		}
	}
	
	return 1;
}

int CRoadDetectDHCamera::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}

bool CRoadDetectDHCamera::IsCentreRectResemble(CvRect rectNew)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	//区域中心点
	int iwidth  =  rectNew.x + rectNew.width/2;
	int iheight =  rectNew.y + rectNew.height/2;
	//int m_nExtentX = 50;
	if( abs(iwidth - m_CameraWidth/2) < m_CameraWidth/40  )
	{
		if( abs(iheight -m_Cameraheight/2)  < m_Cameraheight/20  )
		{
			return true;
		}
	}
	return false;
}
int CRoadDetectDHCamera::CalculateRectPtzZoomEX(CvRect rectNew,float &fRectArea,float &fZoomValue)
{
	double fRealZoomValue = 0.0;
	if(   ((rectNew.width/m_CameraWidth)*1000) > ((rectNew.height/m_Cameraheight)*1000) )
	{
		double uiRectArea = m_CameraWidth/(rectNew.width*(1000.0/g_ytControlSetting.nRemotePicInterval));

		if( uiRectArea > 4.0)
		{
			uiRectArea = 4.0;
		}

		if( (int)(uiRectArea*10) <= 10)
		{
			fRectArea = 0.0;
			fZoomValue = 0.0;
			return 0;
		}

		if( (int)(uiRectArea*10) < 14)
		{
			uiRectArea = 1.4;
		}
		fRectArea = uiRectArea;
		std::string m_szPanTiltXValue;
		std::string m_szPanTiltYValue;
		std::string m_szZoomValue;
#ifdef CAMERAAUTOCTRL
		if(!m_pOnvifCameraCtrl->GetPTZStatus(m_szPanTiltXValue,m_szPanTiltYValue,m_szZoomValue))
		{
			fRectArea = 0.0;
			fZoomValue = 0.0;
			return 0;
		}

		fRealZoomValue = m_pOnvifCameraCtrl->GetCalculateCameraZoom(atof(m_szZoomValue.c_str()),uiRectArea);
#endif
		LogNormal("fRealZoomValue=%0.3f, m_szZoomValue=%s,uiRectArea=%0.3f \n",fRealZoomValue,m_szZoomValue.c_str(),uiRectArea);
		if( (int)(fRealZoomValue*1000) > (int)(atof(m_szZoomValue.c_str())*1000) )
		{
			fRealZoomValue -= atof(m_szZoomValue.c_str());
		}
		else
		{
			fRealZoomValue = atof(m_szZoomValue.c_str()) - fRealZoomValue;
		}

		if( (int)(fRealZoomValue*100) < 5)
		{
			fRealZoomValue = 0.05;
		}
		fZoomValue = fRealZoomValue;
	}
	else
	{
		double uiRectArea = m_Cameraheight/(rectNew.height*(1000.0/g_ytControlSetting.nRemotePicInterval));

		if( uiRectArea > 4.4)
		{
			uiRectArea = 4.4;
		}

		if( (int)(uiRectArea*10) < 7)
		{
			fRectArea = 0.0;
			fZoomValue = 0.0;
			return 0;
		}

		if( (int)(uiRectArea*10) < 14)
		{
			uiRectArea = 1.4;
		}
		fRectArea = uiRectArea;

		std::string m_szPanTiltXValue;
		std::string m_szPanTiltYValue;
		std::string m_szZoomValue;
#ifdef CAMERAAUTOCTRL
		if(!m_pOnvifCameraCtrl->GetPTZStatus(m_szPanTiltXValue,m_szPanTiltYValue,m_szZoomValue))
		{
			fRectArea = 0.0;
			fZoomValue = 0.0;
			return 0;
		}

		fRealZoomValue = m_pOnvifCameraCtrl->GetCalculateCameraZoom(atof(m_szZoomValue.c_str()),uiRectArea);
#endif
		LogNormal("fRealZoomValue=%0.3f, m_szZoomValue=%s,uiRectArea=%0.3f \n",fRealZoomValue,m_szZoomValue.c_str(),uiRectArea);
		if( (int)(fRealZoomValue*1000) > (int)(atof(m_szZoomValue.c_str())*1000) )
		{
			fRealZoomValue -= atof(m_szZoomValue.c_str());
		}
		else
		{
			fRealZoomValue = atof(m_szZoomValue.c_str()) - fRealZoomValue;
		}

		if( (int)(fRealZoomValue*100) < 5)
		{
			fRealZoomValue = 0.05;
		}

		fZoomValue = fRealZoomValue;
	}
	return 0;
}


bool CRoadDetectDHCamera::IsRectResemble(CvRect rectOld,CvRect rectNew)
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

bool CRoadDetectDHCamera::CalculateRectAndMovePtzOrZoom(CvRect rectNew,int m_nCarId,int m_nSaveProcess,bool bPTZMove)
{
	printf("CRoadDetectDHCamera::CalculateRectAndMovePtzOrZoom rectNow.x=%d,rectNow.y=%d,rectNow.width=%d,rectNow.height=%d \n",rectNew.x,rectNew.y,rectNew.width,rectNew.height);

	float fPanTiltX = 0.0;
	float fPanTiltY = 0.0;
	float fZoomX = 0.0;

	printf("[%d]PTZ_Relative_Move begin....... \n",GetTimeT());
	if(bPTZMove)
	{
		if(CalculateRectPtzEX(rectNew,fPanTiltX,fPanTiltY) == 0)
		{
			return 0;
		}

		if(m_nSaveProcess == 1)
		{
			char fPanTiltValue[10] = {0};
			sprintf(fPanTiltValue,"%0.3f",fPanTiltX);
			fPanTiltX = atof(fPanTiltValue);
			(int)(fPanTiltX*10000);
			memset(fPanTiltValue,0,10);
			sprintf(fPanTiltValue,"%0.3f",fPanTiltY);
			fPanTiltY = atof(fPanTiltValue);

			SaveCameraMoveTrack(m_nCarId,(int)(fPanTiltX*10000),(int)(fPanTiltY*10000),0);
		}

		printf("fPanTiltX=%0.6f,fPanTiltY=%0.6f \n",fPanTiltX,fPanTiltY);
#ifdef CAMERAAUTOCTRL
		m_pOnvifCameraCtrl->PTZRelativeMove(fPanTiltX,fPanTiltY,0.0);
#endif
	}
	else
	{
		if(m_nSaveProcess == 1)
		{
			char fPanTiltValue[10] = {0};
			sprintf(fPanTiltValue,"%0.3f",m_fScaleRealZoom);
			m_fScaleRealZoom = atof(fPanTiltValue);
			SaveCameraMoveTrack(m_nCarId,0,0,(int)(m_fScaleRealZoom*10000));
		}
#ifdef CAMERAAUTOCTRL
		if(m_fScaleRealZoom > 0.0)
		{
			m_pOnvifCameraCtrl->PTZRelativeMove(0.0,0.0,m_fScaleRealZoom);
		}
#endif
	}
	printf("[%d]PTZ_Relative_Move end....... \n",GetTimeT());
	return 0;
}

int CRoadDetectDHCamera::CalculateRectPtzEX(CvRect rectNew,float &fPanTiltX,float &fPanTiltY)
{
	std::string m_szPanTiltXValue;
	std::string m_szPanTiltYValue;
	std::string m_szZoomValue;
#ifdef CAMERAAUTOCTRL
	if(!m_pOnvifCameraCtrl->GetPTZStatus(m_szPanTiltXValue,m_szPanTiltYValue,m_szZoomValue))
	{
		return 0;
	}
#endif

	float fNowMultiPlicate = 0.0;

	if(m_nDetectNum == 0)
	{
		printf("m_nDetectNum == 0 \n");
#ifdef CAMERAAUTOCTRL
		fNowMultiPlicate = m_pOnvifCameraCtrl->GetNowCameraZoom(atof(m_szZoomValue.c_str()));
#endif
		//冗余数据 防止出错
		if( (int)(fNowMultiPlicate*100) > 5)
		{
			fNowMultiPlicate += 0.03;
		}
	}
	else
	{
		printf("m_nDetectNum == 1 \n");
#ifdef CAMERAAUTOCTRL
		fNowMultiPlicate = m_pOnvifCameraCtrl->GetNowCameraZoom(atof(m_szZoomValue.c_str()));
#endif
	}

	if( (int)(fNowMultiPlicate*100) < 5)
	{
		fNowMultiPlicate = 0.05;
	}

	if( (int)(fNowMultiPlicate*100) >= 100)
	{
		fNowMultiPlicate = 1.0;
	}


	float fOffsetRate_w    = abs(m_CameraWidth/2  - (rectNew.x + rectNew.width/2))/(m_CameraWidth*1.0);
	float fOffsetRate_h    = abs(m_Cameraheight/2 - (rectNew.y + rectNew.height/2))/(m_Cameraheight*1.0);
	printf("CRoadDetectDHCamera::CalculateRectPtzEX: fNowMultiPlicate=%0.6f,fOffsetRate_w=%0.6f,fOffsetRate_h=%0.6f \n",fNowMultiPlicate,fOffsetRate_w,fOffsetRate_h);


	float fMvArc_w = 0.0;		 //输出：相机在w方向移动的角度
	float fMvArc_h = 0.0;        //输出：相机在h方向移动的角度
#ifdef CAMERAAUTOCTRL
	m_pMvCalcCamMvScale->mvCalcCamNeedMvScale(fMvArc_w,fMvArc_h,fNowMultiPlicate,fOffsetRate_w,fOffsetRate_h);
#endif
	printf("CRoadDetectDHCamera::CalculateRectPtzEX: fMvArc_w=%0.6f,fMvArc_h=%0.6f \n",fMvArc_w,fMvArc_h);

	float fMvArcAngle_w = 0.0;
	float fMvArcAngle_h = 0.0;
	fMvArcAngle_w = (fMvArc_w/18)*0.1;		 //输出：相机在w方向移动的角度 换算后的角度
	fMvArcAngle_h = (fMvArc_h/18)*0.1;         //输出：相机在h方向移动的角度 换算后的角度
	printf("CRoadDetectDHCamera::CalculateRectPtzEX: fMvArcAngle_w=%0.6f,fMvArcAngle_h=%0.6f \n",fMvArcAngle_w,fMvArcAngle_h);


	if((m_CameraWidth/2  - (rectNew.x + rectNew.width/2))  > 0 )
	{
		fPanTiltX = 0.0 - fMvArcAngle_w;  //向左移动为负数
	}
	else
	{
		fPanTiltX = fMvArcAngle_w;
	}

	if((m_Cameraheight/2 - (rectNew.y + rectNew.height/2)) < 0)
	{
		fPanTiltY = 0.0 - fMvArcAngle_h; //向下移动为负数
	}
	else
	{
		fPanTiltY = fMvArcAngle_h;
	}
	printf("CSkpRoadDetect::CalculateRectAndMovePtzEX: fPanTiltX=%0.6f,fPanTiltY=%0.6f \n",fPanTiltX,fPanTiltY);


	return 1;
}

int CRoadDetectDHCamera::TrackAutoCtrl(int nCarId,int nSaveProcess)
{
	//清空超时的运动轨迹
// 	for(mapAutoCameraPtzTrack::iterator iter=m_AutoCameraPtzTrack.begin();iter!=m_AutoCameraPtzTrack.end();iter++)
// 	{
// 		LogNormal("show CarId=%d \n",iter->first);
// 	}

	if(nSaveProcess == 1)
	{
		SaveCameraMoveTrack(nCarId,0,0,0);
	}
	else if(nSaveProcess == 2)
	{
		if(m_AutoCameraPtzTrack.find(nCarId) != m_AutoCameraPtzTrack.end())
		{
			listPtzTrack m_listPtzTrack = m_AutoCameraPtzTrack[nCarId];
			LogNormal("Begin: size=%d \n",m_listPtzTrack.size());
			for(listPtzTrack::iterator iter=m_listPtzTrack.begin();iter!=m_listPtzTrack.end();iter++)
			{
				LogNormal("Begin Move,CarID:[%d],X=%0.03f,Y=%0.03f,Zoom=%0.03f \n",nCarId,(*iter).nPanTiltX/10000.0,(*iter).nPanTiltY/10000.0,(*iter).nPanTiltZoom/10000.0);
				if( ((*iter).nPanTiltX == 0)&&((*iter).nPanTiltY == 0)&&((*iter).nPanTiltZoom == 0))
				{
					continue;
				}
#ifdef CAMERAAUTOCTRL
				m_pOnvifCameraCtrl->PTZRelativeMove((*iter).nPanTiltX/10000.0,(*iter).nPanTiltY/10000.0,(*iter).nPanTiltZoom/10000.0);
				usleep(2000*1000); //暂停1秒
#endif
			}
			
			m_AutoCameraPtzTrack[nCarId].clear();
			m_AutoCameraPtzTrack.erase(nCarId);
			return 0;
		}
		else
		{
			LogNormal("Begin Move,CarID: NULL \n");
		}
	}
	else if(nSaveProcess == 3)
	{
		if(m_AutoCameraPtzTrack.find(nCarId) != m_AutoCameraPtzTrack.end())
		{
			m_AutoCameraPtzTrack[nCarId].clear();
			m_AutoCameraPtzTrack.erase(nCarId);
		}
		return 0;
	}

	return 1;
}

bool CRoadDetectDHCamera::GotoPreset(int uiPresetNum)
{
#ifdef CAMERAAUTOCTRL
	m_pOnvifCameraCtrl->PTZGotoPreset(uiPresetNum);
#endif
	return true;
}


int CRoadDetectDHCamera::SaveCameraMoveTrack(int nCarID,int nX,int nY,int nZ)
{
	if(m_AutoCameraPtzTrack.find(nCarID) == m_AutoCameraPtzTrack.end())
	{
		listPtzTrack m_listPtzTrack;
		AutoCameraDHPtzTrack tAutoCameraPtzTrack;
		tAutoCameraPtzTrack.nBeginTime = GetTimeT();
		tAutoCameraPtzTrack.nPanTiltX = nX;
		tAutoCameraPtzTrack.nPanTiltY = nY;
		tAutoCameraPtzTrack.nPanTiltZoom = nZ;
		m_listPtzTrack.push_back(tAutoCameraPtzTrack);

		m_AutoCameraPtzTrack.insert(make_pair(nCarID,m_listPtzTrack));
		LogNormal("Input CameraTrack[%d]: x=%d,y=%d,z=%d \n",nCarID,nX,nY,nZ);
	}
	else
	{
		if((nX == 0)&& (nY == 0) &&(nZ == 0))
		{
			return 0;
		}
		mapAutoCameraPtzTrack::iterator iter = m_AutoCameraPtzTrack.find(nCarID);
		AutoCameraDHPtzTrack tAutoCameraPtzTrack;
		tAutoCameraPtzTrack.nBeginTime   = GetTimeT();
		tAutoCameraPtzTrack.nPanTiltX    = nX;
		tAutoCameraPtzTrack.nPanTiltY    = nY;
		tAutoCameraPtzTrack.nPanTiltZoom = nZ;
		iter->second.push_back(tAutoCameraPtzTrack);
		LogNormal("Input CameraTrack[%d]:size=%d x=%d,y=%d,z=%d \n",nCarID,iter->second.size(),nX,nY,nZ);
	}
	return 0;
}





