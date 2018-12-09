// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include"Factory.h"
#include"AbstractCamera.h"
#include"Zebra.h"
#include "RoadVideoYuv.h"
#include "HDIPCamera.h"
#include "RoSeekCamera.h"
#include "SanYoCamera.h"
#include "RoSeekCameraDsp.h"
#include "Zebra2.h"
#include "H3CCamera.h"
#include "PicLib.h"
#include "BaslerCamera.h"
#include "Axis.h"
#include "HKCamera.h"
#include "CFCamera.h"
#include "Hikvision.h"
#include "DspServer.h"
#include "DHCamera.h"
#ifdef ZIONCAMERA
#include "ZionCamera.h"
#endif



Factory::Factory()
{
    printf("=Factory::Factory()=\n");
    m_pAbstractCamera = NULL;
}

Factory::~Factory()
{
    LogTrace(NULL, "before Factory::~Destory\n");
    Destory();
    LogTrace(NULL, "after Factory::~Destory\n");
}

AbstractCamera* Factory::Create(int nCameraType,int nPixelFormat)
{
    printf("=Factory::Create=nPixelFormat=%d,nCameraType=%d\n", nPixelFormat,nCameraType);
    m_pAbstractCamera = NULL;

	if(g_nDetectMode == 2)
	{
		 if(nCameraType == DSP_SERVER)
		{
			LogNormal("==Create DSP_SERVER CAM!!=\n");
			#ifndef ALGORITHM_YUV
			m_pAbstractCamera = new CDspServer(nCameraType);
			#endif
		}
		else//dsp方案与相机类型无关
		{
			m_pAbstractCamera = new CRoSeekCameraDsp(nCameraType);
		}
	}
	else
	{
		if(nPixelFormat == VEDIO_PICLIB)//无视相机类型，所以必须放在最上面
		{
			m_pAbstractCamera = new PicLib(nCameraType,nPixelFormat);
		}
		else if (nCameraType == PTG_ZEBRA_200|| nCameraType == PTG_ZEBRA_500)
		{
			m_pAbstractCamera = new Zebra(nCameraType,nPixelFormat);
		}
		else if(nCameraType == JAI_CAMERA_LINK_FIELD ||
				nCameraType == JAI_CAMERA_LINK_FRAME||
				nCameraType == JAI_CAMERA_LINK_FIELD_P||
				nCameraType == BOCOM_301_200||
				nCameraType == BOCOM_302_500||
				nCameraType == BOCOM_301_500||
				nCameraType == JAI_CL_500||
				nCameraType == BOCOM_302_200)
		{
			
				m_pAbstractCamera = new CSkpRoadVideoYuv(nCameraType,nPixelFormat);
		
		}
		else if(nCameraType == ANALOG_FRAME||
				nCameraType == ANALOG_FIELD)
		{

		}
		else if(nCameraType == HD_IP_CAMERA)
		{
			m_pAbstractCamera = new CHDIPCamera(nCameraType);
		}
		else if(nCameraType == ROSEEK_CAMERA)
		{
			m_pAbstractCamera = new CRoSeekCamera(nCameraType);
		}
		else if(nCameraType == SANYO_CAMERA)
		{
			m_pAbstractCamera = new CSanYoCamera(nCameraType);
		}
		else if((nCameraType == PTG_GIGE_200) || (nCameraType == PTG_GIGE_500))
		{
			m_pAbstractCamera = new ZebraTwo(nCameraType,nPixelFormat);
		}	
		else if(nCameraType == DSP_SERVER)
		{
			LogNormal("==Create DSP_SERVER CAM!!=\n");
			#ifndef ALGORITHM_YUV
			m_pAbstractCamera = new CDspServer(nCameraType);
			#endif
		}
		else if (nCameraType == H3C_CAMERA) //H3C视频流
		{
			#ifndef NOH3CCAMERA
			m_pAbstractCamera = new H3CCamera(nCameraType);
			#endif
		}
		else if (nCameraType == BASLER_200 || nCameraType == BASLER_500) //BASLER相机
		{
	#ifdef BASLER_CAMERA
			m_pAbstractCamera = new BaslerCamera(nCameraType);
	#endif
		}
		else if (nCameraType == AXIS_CAMERA) //安讯士相机
		{
			m_pAbstractCamera = new Axis(nCameraType,nPixelFormat);
		}
		else if (nCameraType == HK_CAMERA) //HK相机
		{
	#ifdef HKCAMERA
			m_pAbstractCamera = new CHKCamera(nCameraType);
	#endif

	#ifdef HIKVISIONCAMERA
			m_pAbstractCamera = new CHikvision(nCameraType);
	#endif

		}
		else if (nCameraType == CF_CAMERA || nCameraType == CF_CAMERA_2) //CF相机
		{
	#ifdef CFCAMERA
			m_pAbstractCamera = new CCFCamera(nCameraType);
	#endif
		}
		else if (nCameraType == DH_CAMERA ) //CF相机
		{
	//#ifdef DIO_RTSP
			m_pAbstractCamera = new CDHCamera(nCameraType);
			printf("Create CDHCamera");
	//#endif
		}
		else if(nCameraType == ZION_CAMERA) //凯利信
		{
	#ifdef ZIONCAMERA
			m_pAbstractCamera = new CZionCamera(nCameraType);
	#endif
		}
	}

    return m_pAbstractCamera;
}

void Factory::Destory()
{
    if(m_pAbstractCamera)
    {
        delete m_pAbstractCamera;
        m_pAbstractCamera = NULL;
    }
}
