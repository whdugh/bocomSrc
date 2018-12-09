// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "ximage.h"
#include "RoadCarnumDetect.h"
#include "Common.h"
#include "CommonHeader.h"
#include "ippi.h"
#include "ippcc.h"
#include "DioConnet.h"
#include "DioComSignalProtocol.h"
#include "DioComSignalMachine.h"

#include "XmlParaUtil.h"
#include "FtpCommunication.h"
#include "RoadImcData.h"
#ifndef ALGORITHM_YUV
#ifndef ALGORITHM_DL
	#include "MvLisenceDetect.h"
#endif
#else
	#include "MvDspX86Debug.h"
#endif
#include "CenterServerOneDotEight.h"
#include "MvsCommunication.h"
#include "mv_RedLightProcess.h"
#include "MatchCommunication.h"
//#ifdef FBMATCHPLATE
	#include "MatchPlateFortianjin.h"
//#endif
#include "CarLabel.h"

/*#ifndef WEN_LING_VTS_TEST
	#define WEN_LING_VTS_TEST
#endif*/

/*
#ifndef DEBUG_VTS
	#define DEBUG_VTS
#endif
*/

//#include "mvGetBGLight.h"
//#define DEBUG_CARNUMDETECT
//#define CAMERA_CONTROL
//#define TEMPERATURE_TEST
//#define DRAW_RESULT_RECT
//#define WRITE_BACK
//#define savetxt
//#define savevtstxt
//#define dur_savetxt
//#define DETECT_TIME
#define WATER_MARK

//#define BJGONGJIAO 1 //北京公交Dsp项目add by wantao

/*
#ifndef VTS_TEST_SEQ
	#define VTS_TEST_SEQ
#endif
*/

#include "MvImageEnhance.h"

#ifdef RED_GREEN_ENHANCE
	#include "RedGreenLightEnhance.h"
#endif

/*
//500W图片转成1600*1200输出测试
#ifndef DSP_500W_TEST
	#define DSP_500W_TEST
#endif
*/

//违章缓存输出
/*
#ifndef VTS_OUTPUT_2
	#define VTS_OUTPUT_2
#endif
	*/

/*
//违章存图测试
#ifndef VTS_OUTPUT_SAVEPIC
	#define VTS_OUTPUT_SAVEPIC
#endif
*/

/*#ifndef OUT_STATE_LOG
	#define OUT_STATE_LOG
#endif*/



#ifndef NOPLATE

	#ifdef dur_savetxt
		#include <fstream>
		std::ofstream ofs;
	#endif

	#ifdef GLOBALCARLABEL
		CarLabel  g_carLabel;
		pthread_mutex_t g_carLabelMutex;
		#include "BrandSubSection.h"
	#endif


	#ifdef GLOBALCARCOLOR
		ColorRecognisize  g_carColor;
		pthread_mutex_t g_carColorMutex;
	#endif

	#ifdef GLOBALCARCLASSIFY
		MvVehicleClassify g_vehicleClassify;
		pthread_mutex_t g_vehicleClassifyMutex;
	#endif


//输出结果线程
void* ThreadOutPut(void* pArg)
{
    CRoadCarnumDetect* pRoadCarnumDetect = (CRoadCarnumDetect*)pArg;

    if(pRoadCarnumDetect == NULL) return pArg;

    pRoadCarnumDetect->DealOutPut();

    pthread_exit((void *)0);

    return pArg;
}

//目标检测线程
void* ThreadObjectDetect(void* pArg)
{
    CRoadCarnumDetect* pRoadCarnumDetect = (CRoadCarnumDetect*)pArg;

    if(pRoadCarnumDetect == NULL) return pArg;

    pRoadCarnumDetect->DealObjectDetect();
    pthread_exit((void *)0);
    printf("=================ThreadObjectDetect end\n");
    return pArg;
}

//车牌检测线程
void* ThreadCarnumDetect(void* pArg)
{
	sleep(2);
    CRoadCarnumDetect* pRoadCarnumDetect = (CRoadCarnumDetect*)pArg;

    if(pRoadCarnumDetect == NULL) return pArg;

    pRoadCarnumDetect->DealCarnumDetect();

    pthread_exit((void *)0);
    printf("=================ThreadCarnumDetect end\n");
    return pArg;
}

//构造
CRoadCarnumDetect::CRoadCarnumDetect()
{
    //存取信号互斥
    pthread_mutex_init(&m_FrameMutex,NULL);
    //存取信号互斥
    pthread_mutex_init(&m_PlateMutex,NULL);

    pthread_mutex_init(&m_AddFrameMutex,NULL);

    pthread_mutex_init(&m_OutPutMutex,NULL);

	pthread_mutex_init(&m_JpgFrameMutex,NULL);

	pthread_mutex_init(&m_InPutMutex,NULL);

	pthread_mutex_init(&m_ImgTagMutex,NULL);

	pthread_mutex_init(&m_OutCarnumMutex,NULL);

	pthread_mutex_init(&m_imgTagMutex,NULL);

	#ifdef REDADJUST
		pthread_mutex_init(&m_RedAdjustMutex,NULL);
	#endif
	

    //线程ID
    m_nThreadId = 0;
    m_nObjectThreadId = 0;
    m_nOutPutThreadId = 0;
    m_nChannelId = -1;

    m_bConnect = false;
	m_nFrequency = 15;

	m_imgLightSnap = NULL;
    m_imgSnap = NULL;
    m_imgPreSnap = NULL;
    m_imgDestSnap = NULL;
    m_imgComposeSnap = NULL;
    m_imgFrame = NULL;
    m_img = NULL;
    m_img1 = NULL;
    m_img2 = NULL;
    m_imgWriteBack = NULL;
    m_imgResult = NULL;
	m_imgResize = NULL;

    m_pFrameBuffer = NULL;
    m_pPlateBuffer = NULL;
    m_pBuffer = NULL;

    m_nDetectKind = DETECT_CARNUM;

    m_nDetectTime = DETECT_AUTO;

    m_eCapType = CAPTURE_NO;

    m_nTrafficStatTime = 60;

    m_nExtentHeight = 60;
    m_nSaveImageCount = 1;
    m_nSmallPic = 0;
    m_nWordPos = 0;
    m_nWordOnPic = 0;
    m_nDayNight = 1;

    m_nCameraID = 0;
    m_nCameraType = 0;
    m_nDetectDirection = 0;//检测方向
	m_pDetect = NULL;

    m_pCurJpgImage = NULL;
    m_pPreJpgImage = NULL;
    m_pSmallJpgImage = NULL;
    m_pComposeJpgImage = NULL;
	m_imgComposeResult = NULL;
	m_bGetPlate = false;
	m_bTestResult = false;
	m_nFileID = -1;

	m_fScaleX = 1;
	m_fScaleY = 1;
	m_ratio_x = 1.0;
	m_ratio_y = 1.0;
	m_nDeinterlace = 1;

	m_fSpeedFactor = 0.0;

	m_bImageEnhance = true;//是否图像增强
	m_bDetectShield = false;//是否检测遮阳板
	m_nImageEnhanceFactor = 15;//图像增强因子

	m_nNotGetJpgCount = 0;

	#ifdef OBJECTFEATURESEARCH
	  m_pFeatureSearchDetect = NULL;
	  m_uFeatureSeq = 0;
	#endif

	 m_pH264Capture = NULL;
	 m_pImageFilter = NULL;
	 m_pVtsImageFilter = NULL;

	 m_nVedioFormat = 0;

	 m_BlackFrameWidth = 200;
	 m_nFontSize = 15;

	 m_imgDestSnap2 = NULL;
	 m_imgComposeSnap2 = NULL;

	 m_pDataY[0] = NULL;
	 m_pDataY[1] = NULL;
	 m_pDataY[2] = NULL;
	 m_pImageDataY = NULL;
	 m_pImageSmallDataY = NULL;	
	 
#ifdef DRAW_RECT_DEBUG	
	for(int i=0; i<MAX_RED_LIGHT_NUM; i++)
	{
		m_rectRed[i].x = 0;
		m_rectRed[i].y = 0;
		m_rectRed[i].width = 0;
		m_rectRed[i].height = 0;
	}	
#endif

	pthread_mutex_lock(&m_imgTagMutex);
	for(int i=0; i<MAX_IMG_TAG_NUM; i++)
	{
		m_imgTagList[i].uKey = 0;
		m_imgTagList[i].uTime = 0;
		m_imgTagList[i].uLastTime = 0;
		m_imgTagList[i].pImg = NULL;
		m_imgTagList[i].bUse = false;
		m_imgTagList[i].nUseCount = 0;
	}
	pthread_mutex_unlock(&m_imgTagMutex);

	m_pMachPlate = NULL;

	m_nRedNum = 0;
	m_nDealFlag = 0;
	m_strPicMatch = "";

#ifdef REDADJUST
	m_pRedLightAdjust = NULL;
	for(int i=0; i<MAX_RED_LIGHT_NUM; i++)
	{
		m_nIndexRedArray[i] = -1;
	}

	for(int i=0; i<MAX_RED_LIGHT_NUM; i++)
	{
		m_rectRedArray[i].x =  0;
		m_rectRedArray[i].y =  0;
		m_rectRedArray[i].width =  0;
		m_rectRedArray[i].height =  0;
	}
#endif

	m_nLastCheckTime = 0;
	m_pCDspDataProcess = new CDspDataProcess(NULL);
	
	m_rtCarnumROI.width = 0;
	m_rtCarnumROI.height = 0;

	//for 岳阳比武
	m_IsMultiChannel = false; //默认是单车道
	m_DetectAreaUp = 20;  
	m_DetectAreaBelow = 100;
}
//析构
CRoadCarnumDetect::~CRoadCarnumDetect()
{
    //存取信号互斥
    pthread_mutex_destroy(&m_FrameMutex);
    //存取信号互斥
    pthread_mutex_destroy(&m_PlateMutex);
    pthread_mutex_destroy(&m_AddFrameMutex);
    pthread_mutex_destroy(&m_OutPutMutex);
	pthread_mutex_destroy(&m_JpgFrameMutex);
	pthread_mutex_destroy(&m_InPutMutex);

	pthread_mutex_destroy(&m_ImgTagMutex);
	pthread_mutex_destroy(&m_OutCarnumMutex);
	pthread_mutex_destroy(&m_imgTagMutex);

	#ifdef REDADJUST
		pthread_mutex_destroy(&m_RedAdjustMutex);
	#endif

	if (m_pCDspDataProcess != NULL)
	{
		delete m_pCDspDataProcess;
		m_pCDspDataProcess = NULL;
	}
}

bool CRoadCarnumDetect::Init(int nChannelId,UINT32 uWidth,UINT32 uHeight,int nWidth, int nHeight)
{
	LogNormal("uWidth=%d,uHeight=%d,nWidth=%d,nHeight=%d\n",uWidth,uHeight,nWidth,nHeight);
	if(g_nDetectMode != 2)
	{
		if( (uWidth/nWidth) == (uHeight/nHeight) )
		{
			m_nDeinterlace = 1; //帧图像
		}
		else
		{
			m_nDeinterlace = 2; //场图像
		}
	}
	/*if(m_nCameraType == DSP_ROSEEK_200_310 || m_nCameraType == DSP_ROSEEK_200_385)
	{
		uWidth = DSP_200_BIG_WIDTH;
		uHeight = DSP_200_BIG_HEIGHT;
	}	
	else if(m_nCameraType == DSP_ROSEEK_500_335)
	{
		uWidth = DSP_500_BIG_WIDTH;
		uHeight = DSP_500_BIG_HEIGHT;
	
		LogNormal("=CRoadCarnumDetect::Init=uWidth=%d,uHeight=%d\n", uWidth, uHeight);
	}
	else if(m_nCameraType == DSP_ROSEEK_200_380)
	{
		uWidth = DSP_200_BIG_WIDTH_WIDE;
		uHeight = DSP_200_BIG_HEIGHT_WIDE;

		LogNormal("=CRoadCarnumDetect::Init=uWidth=%d,uHeight=%d\n", uWidth, uHeight);
	}
	else if(m_nCameraType == DSP_ROSEEK_500_330)
	{
		uWidth = DSP_500_BIG_HEIGHT;
		uHeight = DSP_500_BIG_WIDTH;

		LogNormal("=CRoadCarnumDetect::Init=uWidth=%d,uHeight=%d\n", uWidth, uHeight);
	}
	else if(m_nCameraType == DSP_ROSEEK_400_340)
	{
		uWidth = DSP_400_BIG_WIDTH;
		uHeight = DSP_400_BIG_HEIGHT;

		LogNormal("=CRoadCarnumDetect::Init=uWidth=%d,uHeight=%d\n", uWidth, uHeight);
	}
	else if(m_nCameraType == DSP_500_C501K)
	{
		uWidth = DSP_500_BIG_WIDTH_TWO;
		uHeight = DSP_500_BIG_HEIGHT_TWO;

	}
	else if(m_nCameraType == DSP_200_C201K)
	{
		uWidth = DSP_200_BIG_WIDTH_TWO;
		uHeight = DSP_200_BIG_HEIGHT_TWO;

	}
	else if(m_nCameraType == DH_203_M)
	{
		uWidth = DSP_200_BIG_DH_WIDTH;
		uHeight = DSP_200_BIG_DH_HEIGHT;

	}	
	else if(m_nCameraType == DH_523_M)
	{
		uWidth = DSP_500_BIG_DH_WIDTH;
		uHeight = DSP_500_BIG_DH_HEIGHT;

	}
	else if(m_nCameraType == DH_213_M)
	{
		uWidth = DSP_200_BIG_DH_213_WIDTH;
		uHeight = DSP_200_BIG_DH_213_HEIGHT;

	}
	else if(m_nCameraType == DSP_200_C203K)
	{
		uWidth = DSP_200_BIG_C203K_WIDTH;
		uHeight = DSP_200_BIG_C203K_HEIGHT;

	}
	else if(m_nCameraType == DSP_ROSEEK_600_465)
	{
		uWidth = DSP_600_BIG_465_WIDTH;
		uHeight = DSP_600_BIG_465_HEIGHT;
	}
	else
	{
		//
	}*/

    m_bReloadROI = true;
    m_bReloadObjectROI = true;
    m_bInitCarNumLib = 0;
    m_bReinitObj = false;
	m_bTestResult = false;
	m_nFileID = -1;
	m_bLoadCarColor = false;
	 m_bLoadCarLabel= false;

    m_nChannelId = nChannelId;

    //流量统计初始化
    for(int i = 0; i<MAX_ROADWAY; i++)
    {
        uFluxAll[i]=0;
        uFluxSmall[i]=0;
        uFluxMiddle[i]=0;
        uFluxBig[i]=0;
        uFluxPerson[i]=0;
        uFluxNoneVehicle[i]=0;
		uBigCarSpeed[i]=0;
		uSmallCarSpeed[i]=0;
		uAverageCarSpeed[i]=0;
		uMaxSpeed[i]=0;

		uOccupyRatio[i] = 0;
		uAvgDis[i] = 0;
		uPreCarTime[i] = 0;

		uSignalFluxAll[i]=0;       //总流量
		uSignalFluxSmall[i]=0;     //小车流量
		uSignalFluxBig[i]=0;   //大车流量
		uSignalBigCarSpeed[i]=0;	//大车平均速度
		uSignalSmallCarSpeed[i]=0;	//小车平均速度
		uSignalAverageCarSpeed[i]=0;	//平均速度
		uSignalMaxSpeed[i]=0;		//最大速度

		uLeftSignalFluxAll[i]=0;       //总流量
		uLeftSignalFluxSmall[i]=0;     //小车流量
		uLeftSignalFluxBig[i]=0;   //大车流量
		uLeftSignalBigCarSpeed[i]=0;	//大车平均速度
		uLeftSignalSmallCarSpeed[i]=0;	//小车平均速度
		uLeftSignalAverageCarSpeed[i]=0;	//平均速度
		uLeftSignalMaxSpeed[i]=0;		//最大速度
    }
	m_uNonTrafficSignalTime = 0;
	m_uPreNonTrafficSignalTime = 0;
    m_uNonTrafficSignalBeginTime = 0;
    m_uNonTrafficSignalEndTime = 0;
	m_nConnectnumber = 0;

	m_uNonLeftTrafficSignalTime = 0;
	m_uPreNonLeftTrafficSignalTime = 0;
	m_uNonLeftTrafficSignalBeginTime = 0;
	m_uNonLeftTrafficSignalEndTime = 0;
	m_nLeftConnectnumber = 4;

	m_uPreTrafficSignalTime = 0;

	for(int i = 0;i<3;i++)
	{
		m_nRandCode[i] = 0;
	}


    m_uImageSize = 0;
    m_uPreSeq = 0;
	m_nCountBufferExceed = 0;

    m_ResultFrameList.clear();

	//清空记录
    MapCarInfo.clear();
	//清空图片
	MapPicData.clear();

	m_listTestResult.clear();

    m_nExtentHeight = g_PicFormatInfo.nExtentHeight;

    m_nSaveImageCount = g_nSaveImageCount;
    m_nSmallPic = g_PicFormatInfo.nSmallPic;
    m_nWordPos = g_PicFormatInfo.nWordPos;
    m_nWordOnPic = g_PicFormatInfo.nWordOnPic;

    //文本初始化
    int nFontSize = 25;
    if(uWidth < 1000)
    {
       nFontSize = 15;
    }
    else if(uWidth < 2000)
    {
       nFontSize = g_PicFormatInfo.nFontSize;
    }
    else
    {
        if(m_nExtentHeight < 80)
        {
			if(g_nServerType == 4)//江宁电警
			{
				m_nExtentHeight = 100;
				nFontSize = 50;
			}
			else
			{
				nFontSize = g_PicFormatInfo.nFontSize;
			}
        }
        else
        {
			if(g_nServerType == 0)
			{
				nFontSize = g_PicFormatInfo.nFontSize;
			}
			else
			{
				m_nExtentHeight = 80;
				nFontSize = 40;
			}
        }
    }

	
	if(23 == g_nServerType || 26 == g_nServerType)
	{
		if(g_DistanceHostInfo.bDistanceCalculate == 1)
		{
			nFontSize = g_PicFormatInfo.nFontSize;

			g_mvsCommunication.SetFontSize(nFontSize);
			g_mvsCommunication.SetExtentHeight(m_nExtentHeight);
		}
	}
	else{}

	if(g_nServerType == 7)
	{
		if(g_PicFormatInfo.nWordOnPic == 1)
		{
			//在图片上直接叠字,强制黑边高度为0
			m_nExtentHeight = 0;
		}		
	}

	if(g_nDetectMode == 2)
	{
		nFontSize = g_PicFormatInfo.nFontSize;
	}
	//武汉格式
	if (1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode)//有违章小图 并且 大图1x2叠加
	{
		nFontSize = 15;
	}

	if(g_nDetectMode != 2)//dsp模式不需分配待检场和检出场空间 add by wantao
	{
		m_imgSnap = cvCreateImage(cvSize(uWidth,uHeight*m_nDeinterlace+m_nExtentHeight),8,3);
        m_imgPreSnap = cvCreateImage(cvSize(uWidth,uHeight*m_nDeinterlace+m_nExtentHeight),8,3);
		 //多张图片叠加存放
        if(m_nSmallPic == 1)
		{
			if(m_nSaveImageCount == 2)
			{
				m_imgDestSnap = cvCreateImage(cvSize(uWidth*m_nSaveImageCount+uHeight*m_nDeinterlace,(uHeight*m_nDeinterlace+120)),8,3);
				m_imgWriteBack = cvCreateImageHeader(cvSize(m_imgDestSnap->width*m_nSaveImageCount,m_imgDestSnap->height),8,3);
			}
			else
			{
				m_imgDestSnap = cvCreateImage(cvSize(uWidth*m_nSaveImageCount+uHeight*m_nDeinterlace,(uHeight*m_nDeinterlace+m_nExtentHeight)),8,3);
				
				if(g_nPicMode != 1)
				{
					m_imgWriteBack = cvCreateImageHeader(cvSize(m_imgSnap->width,m_imgSnap->height),8,3);
				}
				else
				{
					m_imgWriteBack = cvCreateImageHeader(cvSize(m_imgDestSnap->width,m_imgDestSnap->height),8,3);
				}
			}
		}
        else
		{
			m_imgDestSnap = cvCreateImage(cvSize(uWidth*m_nSaveImageCount,(uHeight*m_nDeinterlace+m_nExtentHeight)),8,3);
			m_imgWriteBack = cvCreateImageHeader(cvSize(m_imgSnap->width*m_nSaveImageCount,m_imgSnap->height),8,3);
		}

		m_pCurJpgImage = new BYTE[uWidth*uHeight];

        m_pPreJpgImage = new BYTE[uWidth*uHeight];

        m_pSmallJpgImage = new BYTE[uWidth*uHeight/4];

        m_pComposeJpgImage = new BYTE[uWidth*uHeight*3];

		
		//检出结果图像
		if(m_nDeinterlace == 2)
		{
			m_imgFrame = cvCreateImage(cvSize(uWidth,uHeight*m_nDeinterlace),8,3);
		}
		else
		{
			m_imgFrame = cvCreateImageHeader(cvSize(uWidth,uHeight*m_nDeinterlace),8,3);
		}
		m_imgLightSnap = cvCreateImage(cvSize(uWidth,uHeight*m_nDeinterlace+m_nExtentHeight),8,3);

		m_imgResult = cvCreateImageHeader(cvSize(uWidth,uHeight+m_nExtentHeight/m_nDeinterlace),8,3);

		m_ratio_x = 1.0;
        m_ratio_y = 1.0/m_nDeinterlace;

		m_fScaleX = m_ratio_x;
		m_fScaleY = m_ratio_y;

		int nExtentHeight = m_nExtentHeight/m_nDeinterlace;
		 //待检测图像
		m_nMaxFrameSize = FRAMESIZE;
		
		if(uWidth > 2000)
		{
			m_nMaxPlateSize = 30;//#
			if(g_sysInfo_ex.fTotalMemory > 3)//内存大于3G
			{
				m_nMaxPlateSize = 40;//#
			}
		}
		else
		{
			m_nMaxPlateSize = 40;//#
			if(g_sysInfo_ex.fTotalMemory > 3)//内存大于3G
			{
				m_nMaxPlateSize = 60;//#
			}
		}
		m_img = cvCreateImageHeader(cvSize(uWidth,uHeight),8,3);
		m_img1 = cvCreateImageHeader(cvSize(uWidth,uHeight),8,3);
		m_img2 = cvCreateImageHeader(cvSize(uWidth,uHeight),8,3);
		//分配待检场队列空间
		#ifdef ALGORITHM_YUV
		m_fScaleX = (double)uWidth/nWidth;
		m_fScaleY = (double)uHeight/nHeight;
		#endif

		#ifdef ALGORITHM_YUV_CARNUM
		int linestep = uWidth * 2;
		m_pDataY[0] = new BYTE[uWidth*uHeight];
		m_pDataY[1] = new BYTE[uWidth*uHeight/2];
		m_pDataY[2] = new BYTE[uWidth*uHeight/2];
		m_pImageDataY = cvCreateImageHeader(cvSize(uWidth,uHeight),8,1);
		m_pImageSmallDataY = cvCreateImageHeader(cvSize(nWidth,nHeight),8,1);
		#else
		int linestep = uWidth * 3 + IJL_DIB_PAD_BYTES(uWidth,3);
		#endif
		int nSize = sizeof(yuv_video_buf)+linestep*(uHeight+nExtentHeight)+sizeof(int)+CARNUMSIZE*sizeof(CarInfo);  //yuvbuf+rgb数据区+检出的车牌数+车牌信息
		m_pFrameBuffer = new BYTE[nSize*m_nMaxFrameSize];
        memset(m_pFrameBuffer,0,nSize*m_nMaxFrameSize);
		int i = 0;
		for(i=0; i < m_nMaxFrameSize; i++)
		{
			m_chFrameBuffer[i] = m_pFrameBuffer + i*nSize;
		}
		m_nReadIndex = 0;
		m_nWriteIndex = 0;
		m_FrameSize = 0;
		m_pBuffer = m_chFrameBuffer[0];

		//分配检出场队列空间
		int nPlateSize;
		nPlateSize = m_nMaxPlateSize*m_nDeinterlace;
		m_pPlateBuffer = new BYTE[nSize*nPlateSize];
		memset(m_pPlateBuffer,0,nSize*nPlateSize);
		for(i=0; i<nPlateSize; i++)
		{
			m_chPlateBuffer[i] = m_pPlateBuffer+i*nSize;
		}
		m_PlateSize = 0;
		m_nCurIndex = 0;
		m_nPlateReadIndex = 0;
		m_nPlateIndex = 0;
		memset(m_BufPlate,0,sizeof(rgb_buf)*m_nMaxPlateSize);

		m_vtsPicMap.clear();
	}
	else
	{
		m_JpgFrameMap.clear();
		m_mapInPutResult.clear();

		m_nMaxFrameSize = 50;
		m_nMaxPlateSize = 30;
	}


	if(g_nServerType != 13)
	{
		{
		  m_cvText.Init(nFontSize);
		}
	}
	if(m_nSaveImageCount == 2 || g_nVtsPicMode == 5)
	{
		m_cvBigText.Init(80);
	}
	else
	{
		{
			m_cvBigText.Init(nFontSize);
		}
	}

	if(g_nServerType == 21)
	{
		if(m_imgSnap->width > 2000)
		{
			m_cvBigText.Init(80);
		}
		else
		{
			m_cvBigText.Init(56);
		}
	}
	//LogNormal("nFontSize:%d gSize:%d ", nFontSize, g_PicFormatInfo.nFontSize);

	
	m_pMachPlate = new RECORD_PLATE_DSP_MATCH();


    //创建车牌及目标检测线程
    if(!BeginCarnumDetect())
    {
        return false;
    }
	
    if(m_bEventCapture)
    {
	   if(g_nDetectMode == 2)
	   {
			m_tempRecord.SetCamType(m_nCameraType);
			m_tempRecord.Init();

		//#ifdef FBMATCHPLATE
			g_matchPlateFortianjin.SetTempRecord(m_tempRecord);
		//#endif
	   }
	   else
	   {
			m_skpRoadRecorder.SetCamType(m_nCameraType);
			m_skpRoadRecorder.Init();
	   }
    }
	m_listBASE_PLATE_INFO_1.clear();
	m_listBASE_PLATE_INFO_2.clear();
	m_listBASE_PLATE_INFO_3.clear();

	m_vResult.clear();
	m_vtsResult.clear();

	/*//车牌检测区域初始化   for 岳阳比武
	string strPlateDetectArea = "./PlateDetectAreaSetting.xml";
	if(access(strPlateDetectArea.c_str(),F_OK) == 0)//存在
	{
		XMLNode xml,setting;
		XMLCSTR strText;
		xml = XMLNode::parseFile(strPlateDetectArea.c_str()).getChildNode("PlateDetectAreaSetting");
		setting = xml.getChildNode("IsMultiChannel");
		if(!setting.isEmpty())
		{
			unsigned int uIsMultiChannel = 0;
			strText = setting.getText();
			uIsMultiChannel = xmltoi(strText);
			if (uIsMultiChannel == 1)
			{
				m_IsMultiChannel = true;
			}
			else
			{
				m_IsMultiChannel = false;
			}
		}

		setting = xml.getChildNode("AreaUp");
		if(!setting.isEmpty())
		{
			strText = setting.getText();
			m_DetectAreaUp = xmltoi(strText);
		}
		setting = xml.getChildNode("AreaBelow");
		if(!setting.isEmpty())
		{
			strText = setting.getText();
			m_DetectAreaBelow = xmltoi(strText);
		}
	}
	
	LogNormal("--IsMultiChannel:%d,AreaUp:%d,AreaBelow:%d",m_IsMultiChannel,m_DetectAreaUp,m_DetectAreaBelow);
	printf("\r\n--IsMultiChannel:%d,AreaUp:%d,AreaBelow:%d\r\n",m_IsMultiChannel,m_DetectAreaUp,m_DetectAreaBelow);
	*/
    return true;
}

bool CRoadCarnumDetect::BeginCarnumDetect()
{
    //线程结束标志
    m_bEndDetect = false;

    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    struct   sched_param   param;

    param.sched_priority   =   20;
    pthread_attr_setschedparam(&attr,   &param);
    //分离线程
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	int nret;//add by wantao

	if(g_nDetectMode != 2)//dsp模式不需在检测器上做目标检测 add by wantao
	{
		//启动目标检测线程
		nret=pthread_create(&m_nObjectThreadId,&attr,ThreadObjectDetect,this);

		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建目标检测线程失败,服务无法检测目标！\r\n");
			return false;
		}
	}

	//启动输出结果线程
	LogNormal("启动输出结果线程\n");
    nret=pthread_create(&m_nOutPutThreadId,&attr,ThreadOutPut,this);

    //成功
    if(nret!=0)
    {
        //失败
        LogError("创建输出结果线程失败,服务无法输出结果！\r\n");
        return false;
    }

	if(g_nDetectMode != 2)//dsp模式不需在检测器上做车牌检测 add by wantao
	{
		 //启动车牌检测线程
		 nret=pthread_create(&m_nThreadId,&attr,ThreadCarnumDetect,this);

		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建车牌检测线程失败,服务无法检测车牌！\r\n");
			return false;
		}
	}

    pthread_attr_destroy(&attr);

	#ifdef dur_savetxt
		ofs.open("./dur_savetxt.txt", ios::app);
	#endif


    return true;
}

bool CRoadCarnumDetect::EndCarnumDetect()
{
	//cerr << "CRoadCarnumDetect::EndCarnumDetect() 111" << endl;

    //线程结束标志
    m_bEndDetect = true;

    if(m_nThreadId != 0)
    {
        pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
    }

    if(m_nObjectThreadId!=0)
    {
        pthread_join(m_nObjectThreadId,NULL);
        m_nObjectThreadId = 0;
    }

    if(m_nOutPutThreadId!=0)
    {
        pthread_join(m_nOutPutThreadId,NULL);
        m_nOutPutThreadId = 0;
    }


    printf("EndCarnumDetect\n");
	#ifdef ALGORITHM_YUV
	mvUnInit();
	#else
	if(g_nDetectMode != 2)
	{
		m_ftVlp.Destroy();
	}
	#endif

	#ifdef ALGORITHM_YUV_CARNUM
	MVPLR_carnum_quit();
	
	if(m_pDataY[0] != NULL)
	{
		delete []m_pDataY[0];
		m_pDataY[0] = NULL;
	}

	if(m_pDataY[1] != NULL)
	{
		delete []m_pDataY[1];
		m_pDataY[1] = NULL;
	}

	if(m_pDataY[2] != NULL)
	{
		delete []m_pDataY[2];
		m_pDataY[2] = NULL;
	}

	if(m_pImageDataY != NULL)
	{
		cvReleaseImageHeader(&m_pImageDataY);
		m_pImageDataY = NULL;
	}

	if(m_pImageSmallDataY != NULL)
	{
		cvReleaseImageHeader(&m_pImageSmallDataY);
		m_pImageSmallDataY = NULL;
	}
	#else
    //退出车牌检测程序
    if(m_bInitCarNumLib == 1)
    {
        mvcarnumdetect.carnum_quit();
        m_bInitCarNumLib = 0;
    }
	#endif
    
	m_ptsYelGrid.clear();

	//m_CRecognitionTwice.mvUninitRecognitionTwice();

    //释放文本资源
    m_cvText.UnInit();
    m_cvBigText.UnInit();
	if (g_nServerType == 13)
	{
		m_nJiaoJinCvText.UnInit();
	}
	if (g_nServerType == 15)
	{
		m_nXingTaiCvText.UnInit();
	}

	#ifndef ALGORITHM_YUV
    //释放颜色库
	#ifndef GLOBALCARCOLOR
	if(m_bLoadCarColor)
	{
		m_carColor.color_Destroy();
	}
	#endif
	#ifndef GLOBALCARLABEL
	if(m_bLoadCarLabel)
	{
		m_carLabel.carLabel_Destroy();
	}
	#endif
	#endif

	#ifdef PLATEDETECT
	m_PlateDetector.mvUnInit();
	#endif

	#ifdef DETECTSHIELD
	//m_ShieldGet.mvUninit();
	#endif
	
	#ifdef OBJECTHEADDETECT
	m_HeadDetection.uninit();
	#endif

	#ifdef OBJECTFACEDETECT
	m_FaceDetection.mv_Uninit();
	//m_FaceTrackSearch.mv_UnInitial();
	#endif

	#ifdef BELTDETECT
	m_safeType.SafetyBelt_Destroy();
	#endif
	
	#ifdef REDADJUST
	if(m_pRedLightAdjust != NULL)
	{
		delete m_pRedLightAdjust;
		m_pRedLightAdjust = NULL;
	}
	#endif

	#ifdef OBJECTFEATURESEARCH
	if(m_pFeatureSearchDetect)
	{
		m_pFeatureSearchDetect->MvKK_Destroy();
		m_pFeatureSearchDetect = NULL;
	}
	#endif


    //停止事件录象线程
    {
        if(m_bEventCapture)
		{	
			if(g_nDetectMode == 2)
			m_tempRecord.Unit();			
			else
			m_skpRoadRecorder.UnInit();
		}
    }
    m_vResult.clear();
	m_vtsResult.clear();

    if(m_pPreJpgImage)
    {
        delete []m_pPreJpgImage;
        m_pPreJpgImage = NULL;
    }

    if(m_pCurJpgImage)
    {
        delete []m_pCurJpgImage;
        m_pCurJpgImage = NULL;
    }

    if(m_pSmallJpgImage)
    {
        delete []m_pSmallJpgImage;
        m_pSmallJpgImage = NULL;
    }
    if(m_pComposeJpgImage)
    {
        delete []m_pComposeJpgImage;
        m_pComposeJpgImage = NULL;
    }

	if(m_imgDestSnap2)
	{
		cvReleaseImage(&m_imgDestSnap2);
		m_imgDestSnap2 = NULL;
	}

	if(m_imgComposeSnap2)
	{
		cvReleaseImage(&m_imgComposeSnap2);
		m_imgComposeSnap2 = NULL;
	}
    //try()
    {
          printf("=================11111\n");
          if(m_img != NULL)
        {
            cvReleaseImageHeader(&m_img);
            m_img = NULL;
        }

       printf("=================2222\n");
        if(m_imgPreSnap != NULL)
        {
            cvReleaseImage(&m_imgPreSnap);
            m_imgPreSnap = NULL;
        }
         printf("=================33333\n");
        if(m_imgDestSnap != NULL)
        {
            cvReleaseImage(&m_imgDestSnap);
            m_imgDestSnap = NULL;
        }
         printf("=================4444444\n");

        if(m_imgResult != NULL)
        {
            cvReleaseImageHeader(&m_imgResult);
            m_imgResult = NULL;
        }

        if(m_imgComposeResult != NULL)
        {
            cvReleaseImage(&m_imgComposeResult);
            m_imgComposeResult = NULL;
        }

        if(m_imgWriteBack != NULL)
        {
            cvReleaseImageHeader(&m_imgWriteBack);
            m_imgWriteBack = NULL;
        }
         printf("=================777777777777\n");
        if(m_imgComposeSnap != NULL)
        {
            cvReleaseImage(&m_imgComposeSnap);
            m_imgComposeSnap = NULL;
        }

         printf("=================88888888888888888\n");
        if(m_imgFrame != NULL)
        {
            if(m_nDeinterlace == 2)
            {
                cvReleaseImage(&m_imgFrame);
            }
            else
            {
                cvReleaseImageHeader(&m_imgFrame);
            }

            m_imgFrame = NULL;
        }
         printf("================9999999999999999,m_imgSnap=%lld\n",m_imgSnap);
        if(m_imgSnap != NULL)
        {
            cvReleaseImage(&m_imgSnap);
            m_imgSnap = NULL;
        }

		if(m_imgLightSnap != NULL)
		{
			cvReleaseImage(&m_imgLightSnap);
			m_imgLightSnap = NULL;
		}
         printf("=================1010101010101\n");
        if(m_img1 != NULL)
        {
            cvReleaseImageHeader(&m_img1);
            m_img1 = NULL;
        }
    printf("================121212121212121212\n");
        if(m_img2 != NULL)
        {
            cvReleaseImageHeader(&m_img2);
            m_img2 = NULL;
        }

		if(m_imgResize != NULL)
        {
            cvReleaseImage(&m_imgResize);
            m_imgResize = NULL;
        }
        printf("=================1313131313131333\n");
        //加锁
        pthread_mutex_lock(&m_AddFrameMutex);

        if(m_pFrameBuffer != NULL)
        {
            delete []m_pFrameBuffer;
            m_pFrameBuffer = NULL;
        }		

		printf("=================13 111\n");
        if(m_pPlateBuffer != NULL)
        {
            delete []m_pPlateBuffer;
            m_pPlateBuffer = NULL;
        }
        m_pBuffer = NULL;

		printf("=================13 333\n");
		if(m_pImageFilter != NULL)
		{
			delete []m_pImageFilter;
			m_pImageFilter = NULL;
		}
		printf("=================13 444\n");
		if(m_pVtsImageFilter != NULL)
		{
			delete []m_pVtsImageFilter;
			m_pVtsImageFilter = NULL;
		}

        //解锁
        pthread_mutex_unlock(&m_AddFrameMutex);

        printf("EndCarnumDetect 15555555\n");
        /////////
    }

	#ifdef dur_savetxt
		if (ofs.is_open())
		{
			ofs.close();
		}
	#endif

		if(m_pMachPlate)
		{
			if(m_pMachPlate->pImg)
			{
				//cvReleaseImage(&m_pMachPlate->pImg);				
				m_pMachPlate->pImg = NULL;
			}

			for(int i=0; i<3; i++)
			{
				if(m_pMachPlate->pImgArray[i])
				{
					//cvReleaseImage(&m_pMachPlate->pImgArray[i]);					
					m_pMachPlate->pImgArray[i] = NULL;
				}
			}					

			for(int i=0; i<4; i++)
			{
				if(m_pMachPlate->pPicArray[i])
				{
					//delete m_pMachPlate->pPicArray[i];
					m_pMachPlate->pPicArray[i] = NULL;
					m_pMachPlate->nSizeArray[i] = 0;
					m_pMachPlate->bKeyStateArray[i] = false;
					m_pMachPlate->uKeyArray[i] = 0;					
				}
			}
			
			delete m_pMachPlate;
			m_pMachPlate = NULL;
		}

		pthread_mutex_lock(&m_imgTagMutex);
		for(int i=0; i<MAX_IMG_TAG_NUM; i++)
		{
			if(m_imgTagList[i].pImg)
			{
				m_imgTagList[i].uKey = 0;
				m_imgTagList[i].uTime = 0;
				m_imgTagList[i].uLastTime = 0;

				delete[] m_imgTagList[i].pImg;
				m_imgTagList[i].pImg = NULL;
				m_imgTagList[i].bUse = false;
				m_imgTagList[i].nUseCount = 0;
			}			
		}
		pthread_mutex_unlock(&m_imgTagMutex);

	LogNormal("==out=EndCarnumDetect=\n");

    return true;
}

bool CRoadCarnumDetect::AddFrame(yuv_video_buf* pRGBHeader,BYTE* pBuffer)
{
	
    if(!m_bEndDetect)
    {
        //加锁
        pthread_mutex_lock(&m_AddFrameMutex);

        if(m_pBuffer)
        {
			#ifdef ALGORITHM_YUV_CARNUM
			pRGBHeader->size = pRGBHeader->width*pRGBHeader->height*2;
			yuv_video_buf* pHeader = (yuv_video_buf*)pBuffer;
			pHeader->size = pRGBHeader->size;
			#endif
            memcpy(m_pBuffer,pRGBHeader,sizeof(yuv_video_buf));

            if(m_nWordPos == 0)
			{
				 memcpy(m_pBuffer+sizeof(yuv_video_buf),pBuffer,pRGBHeader->size);
			}
            else
			{
				#ifdef ALGORITHM_YUV_CARNUM
				 memcpy(m_pBuffer+sizeof(yuv_video_buf)+pRGBHeader->width*2*m_nExtentHeight,pBuffer,pRGBHeader->size);
				#else
				 memcpy(m_pBuffer+sizeof(yuv_video_buf)+m_img->widthStep*m_nExtentHeight/m_nDeinterlace,pBuffer,pRGBHeader->size);
				#endif
			}

            //加锁
            pthread_mutex_lock(&m_FrameMutex);

            m_FrameSize++;
            if(m_FrameSize>m_nMaxFrameSize-1)//实际帧数
            {
                m_FrameSize--;
                //此时需要移位(以去除最前面的)
                int n = 1;

                if(m_nDeinterlace == 2)
                {
                    yuv_video_buf* buf1 = (yuv_video_buf*)m_chFrameBuffer[m_nReadIndex];
                    yuv_video_buf* buf2 = (yuv_video_buf*)m_chFrameBuffer[(m_nReadIndex+1)%m_nMaxFrameSize];

                    if(buf2->nFieldSeq-buf1->nFieldSeq==1)
                    {

                        bool bOE =  false;
                       
                        bOE = (buf1->nFieldSeq%2==0);
                        
                        if(bOE)
                        {
                            n = 2;
                        }
                    }
                }

                BYTE* temp = m_chFrameBuffer[(m_nReadIndex+n)%m_nMaxFrameSize];
                for(int i= 0; i<m_nMaxFrameSize-(n+1); i++)
                {
                    m_chFrameBuffer[(m_nReadIndex+n+i)%m_nMaxFrameSize] = m_chFrameBuffer[(m_nReadIndex+n+1+i)%m_nMaxFrameSize];
                }
                m_chFrameBuffer[m_nWriteIndex] = temp;
            }
            else
            {
                m_nWriteIndex++;
                m_nWriteIndex %= m_nMaxFrameSize;
            }
            m_pBuffer = m_chFrameBuffer[m_nWriteIndex];//指向下一个可以写入的位置
            //解锁
            pthread_mutex_unlock(&m_FrameMutex);
        }
        //解锁
        pthread_mutex_unlock(&m_AddFrameMutex);

        return true;
    }

    return false;
}

//添加jpg图像帧
bool  CRoadCarnumDetect::AddJpgFrame(BYTE* pBuffer)
{
	if(!m_bEndDetect)
	{
		//加锁
		pthread_mutex_lock(&m_AddFrameMutex);
		//加锁
		pthread_mutex_lock(&m_JpgFrameMutex);

		yuv_video_buf* pHeader = (yuv_video_buf*)pBuffer;

		//判断时间是否在合理范围内，不合理则丢弃
		if(pHeader != NULL)
		{
			if(abs((long long)(GetTimeStamp())-(long long)pHeader->uTimestamp) > 3600*24*90)//如果相机时间与检测器时间相差3个月以上则丢弃此数据
			{
					LogError("discard jpg %s\n",GetTime(pHeader->uTimestamp).c_str());
					//发送相机重启命令
					CAMERA_CONFIG cfg; 
					cfg.uType = 1; 
					cfg.nIndex = (int)CAMERA_RESTART; 
					g_skpChannelCenter.CameraControl(m_nChannelId,cfg); 
					LogError("相机时间不合理重启相机\n");
					//解锁
					pthread_mutex_unlock(&m_JpgFrameMutex);
					//解锁
					pthread_mutex_unlock(&m_AddFrameMutex);
					return false;
			}
		}
		
		{
			//删除一张图片

			bool bDel = DelOneJpgElem(pHeader);
			//bool bDel = DelOneJpgElemOld(pHeader);


			//暂存seq->index对应关系
			/*if(m_JpgFrameMap.size() > MAX_JPG_BUF_SIZE)
			{
				map<int64_t,string>::iterator it_map = m_JpgFrameMap.begin();

				//LogTrace("Out-JpgMap.log", "=Id[%d]==JpgMap.size()=%d===del seq=%d", \
					m_nChannelId, m_JpgFrameMap.size(), it_map->first);

				m_JpgFrameMap.erase(it_map);
			}
			*/

			//yuv_video_buf* pHeader = (yuv_video_buf*)pBuffer;//提到上面去，暂时注释add by wantao
			string strPic;

			strPic.append((char*)pBuffer,sizeof(yuv_video_buf));
			//strPic.append((char*)(pBuffer+sizeof(yuv_video_buf)),pHeader->size);
			strPic.append((char*)(pBuffer+sizeof(yuv_video_buf)+sizeof(Image_header_dsp)),pHeader->size-sizeof(Image_header_dsp));
			m_JpgFrameMap.insert(map<int64_t,string>::value_type(pHeader->nSeq,strPic));
			//LogNormal("Add jpg seq：%lld \n",pHeader->nSeq, pHeader->size);

			//LogTrace("Add-JpgMap.log", "=Id[%d]==JpgMap.size()=%d===add seq=%d", \
				m_nChannelId, m_JpgFrameMap.size(), pHeader->nSeq);
		}


		//解锁
		pthread_mutex_unlock(&m_JpgFrameMutex);

		//解锁
		pthread_mutex_unlock(&m_AddFrameMutex);
		return true;
	}
	return false;
}



//从待检队列中弹出一帧数据
int CRoadCarnumDetect::PopFrame(rgb_buf& response)
{
    yuv_video_buf* buffer1 = NULL,*buffer2 = NULL;

    int nSize = 0;

    //加锁
    pthread_mutex_lock(&m_FrameMutex);

    if(m_nDeinterlace == 2)
    {
        if(m_FrameSize >= 2) //两场以上才读
        {
            response.type = 0;
            response.pCurBuffer =  m_chFrameBuffer[m_nReadIndex];
            //先调换RB
            if(m_nWordPos == 0)
            cvSetData(m_img1,response.pCurBuffer+sizeof(yuv_video_buf),m_img1->widthStep);
            else
            cvSetData(m_img1,response.pCurBuffer+sizeof(yuv_video_buf)+m_img1->widthStep*m_nExtentHeight/m_nDeinterlace,m_img1->widthStep);
            //cvConvertImage(m_img1,m_img1,CV_CVTIMG_SWAP_RB);
            nSize = 1;

            buffer1 = (yuv_video_buf*)m_chFrameBuffer[m_nReadIndex];
            buffer2 = (yuv_video_buf*)m_chFrameBuffer[(m_nReadIndex+1)%m_nMaxFrameSize];

            if(buffer2->nFieldSeq-buffer1->nFieldSeq==1)
            {
                bool bOE =  false;
               
                bOE = (buffer1->nFieldSeq%2==0);
                

                if(bOE)
                {
                    response.type = 1;
                    response.pNextBuffer = m_chFrameBuffer[(m_nReadIndex+1)%m_nMaxFrameSize];
                    //先调换RB
                    if(m_nWordPos == 0)
                    cvSetData(m_img1,response.pNextBuffer+sizeof(yuv_video_buf),m_img1->widthStep);
                    else
                    cvSetData(m_img1,response.pNextBuffer+sizeof(yuv_video_buf)+m_img1->widthStep*m_nExtentHeight/m_nDeinterlace,m_img1->widthStep);
                    //cvConvertImage(m_img1,m_img1,CV_CVTIMG_SWAP_RB);
                    nSize = 2;
                }
            }
        }
        else if(m_FrameSize == 1) //只有一场
        {
            buffer1 = (yuv_video_buf*)m_chFrameBuffer[m_nReadIndex];

            bool bOE =  false;

            
            bOE = (buffer1->nFieldSeq%2==0);
            

            if(bOE)//该场是偶场则等待否则读出
            {
                nSize = 0;
            }
            else
            {
                response.type = 0;
                response.pCurBuffer =  m_chFrameBuffer[m_nReadIndex];//指向当前可读取的位置
                //先调换RB
                if(m_nWordPos == 0)
                cvSetData(m_img1,response.pCurBuffer+sizeof(yuv_video_buf),m_img1->widthStep);
                else
                cvSetData(m_img1,response.pCurBuffer+sizeof(yuv_video_buf)+m_img1->widthStep*m_nExtentHeight/m_nDeinterlace,m_img1->widthStep);
                //cvConvertImage(m_img1,m_img1,CV_CVTIMG_SWAP_RB);
                nSize = 1;
            }
        }
    }
    else
    {
		int nFrameSize = 1;

        if(m_FrameSize >= nFrameSize)
        {
            response.type = 0;
            response.pCurBuffer =  m_chFrameBuffer[m_nReadIndex];//指向当前可读取的位置
            //先调换RB
            if(m_nWordPos == 0)
            cvSetData(m_img1,response.pCurBuffer+sizeof(yuv_video_buf),m_img1->widthStep);
            else
            cvSetData(m_img1,response.pCurBuffer+sizeof(yuv_video_buf)+m_img1->widthStep*m_nExtentHeight,m_img1->widthStep);
            //cvConvertImage(m_img1,m_img1,CV_CVTIMG_SWAP_RB);
            nSize = 1;
        }
    }
    //解锁
    pthread_mutex_unlock(&m_FrameMutex);
    return nSize;
}

//修改待检帧队列长度
void CRoadCarnumDetect::DecreaseSize(int nSize)
{
    //加锁
    pthread_mutex_lock(&m_FrameMutex);

    /////修改可读取的编号
	if(m_FrameSize >= nSize)
	{
		m_FrameSize -= nSize;
		m_nReadIndex = (m_nReadIndex+nSize)%m_nMaxFrameSize;
	}

    //解锁
    pthread_mutex_unlock(&m_FrameMutex);
}

//目标检测处理
void CRoadCarnumDetect::DealObjectDetect()
{
	if(g_nDetectMode == 2) //dsp 模式直接return add by wantao
	{
		return;
	}

	if((access("/dev/ttyUSB2",F_OK) == 0))
	{
		//如果有3G卡存在
		return;
	}
	
	LogNormal("before DealObjectDetect SetObjectDetectPara\n");
	if(SetObjectDetectPara())
	{
		LogError("DealObjectDetect SetObjectDetectPara ok\n");
		m_bReloadObjectROI = false;
	}
	LogNormal("after DealObjectDetect SetObjectDetectPara\n");

    while(!m_bEndDetect)
    {
		{
			rgb_buf  rgbBuf;
			int nRet = PopDetectedFrame(rgbBuf);

			//printf("=PopDetectedFrame=nRet=%d\n",nRet);

			if(nRet > 0)
			{
				try
				{
					DetectObject(rgbBuf);
				}
				catch(...)
				{
					LogNormal("==Catch DetectObject=ERR==\n");
				}

				DecreaseDetectedSize();
			}
		}

        usleep(1000*1);
    }

    //printf("end of DealObjectDetect!!!!!\n");
    return;
}

//添加处理结果
void CRoadCarnumDetect::AddOutPut(std::vector<CarInfo>& vResult)
{
    //加锁
    pthread_mutex_lock(&m_OutPutMutex);

    if(m_vResult.size() > 10)
    {
        LogNormal("AddOutPut 输出记录结果过多，未能及时输出=%d",m_vResult.size());
    }

	if(m_vResult.size() > 10)
	{
		LogNormal("输出记录=%d",m_vResult.size());
	}

    std::vector<CarInfo>::iterator it = vResult.begin();
    while(it != vResult.end())
    {
        //LogNormal("AddOutPut %lld,tm =%lld,nSeq=%lld",GetTimeStamp(),it->uTimestamp,it->uSeq);

        m_vResult.push_back(*it);

        it++;
    }
     //解锁
    pthread_mutex_unlock(&m_OutPutMutex);
}

//添加违章检测处理结果
void CRoadCarnumDetect::AddVtsOutPut(std::vector<ViolationInfo>& vResult)
{
	bool bDetect = CheckDetectTime();
	if (bDetect == false)
	{
		return;
	}
    //加锁
    pthread_mutex_lock(&m_OutPutMutex);

    if(m_vtsResult.size() > 10)
    {
		LogNormal("id:%d AddVtsOutPut 输出记录结果过多，未能及时输出=%d",m_nChannelId,m_vtsResult.size());
    }

    std::vector<ViolationInfo>::iterator it = vResult.begin();
    while(it != vResult.end())
    {
			m_vtsResult.push_back(*it);
			it++;
    }
     //解锁
    pthread_mutex_unlock(&m_OutPutMutex);
}

//获取处理结果
void CRoadCarnumDetect::PopOutPut(std::vector<CarInfo>& vResult)
{

    //加锁
    pthread_mutex_lock(&m_OutPutMutex);

    if(m_vResult.size() > 0)
    {
        std::vector<CarInfo>::iterator it = m_vResult.begin();

        vResult.push_back(*it);

        m_vResult.erase(it);
    }
     //解锁
    pthread_mutex_unlock(&m_OutPutMutex);
}


//获取Dsp处理结果 add by wantao
void CRoadCarnumDetect::Dsp_PopOutPut(std::vector<CarInfo>& vResult)
{	    
	{
		if(m_mapInPutResult.size() > 0)
		{
			//核查待输出记录是否有图
			bool bCheck = CheckOutResult(vResult);
			
			if(!bCheck)
			{
				//std::multimap<int64_t,DSP_CARINFO_ELEM>::iterator it = m_mapInPutResult.begin();
				//vResult.push_back(it->second);
				//if(m_mapInPutResult.find(it->first) != it)
				//{
				//	LogError("Seq=%llu\n",it->first);
				//}
				//LogNormal("Dsp_PopOutPut bCheck False seq:%d ", it->first);
				//加锁
				//pthread_mutex_lock(&m_InPutMutex);
				//m_mapInPutResult.erase(it);
				//解锁
				//pthread_mutex_unlock(&m_InPutMutex);
			}			
		}
	}

}

//获取违章检测处理结果
void CRoadCarnumDetect::PopVtsOutPut(std::vector<ViolationInfo>& vResult)
{
    //加锁
    pthread_mutex_lock(&m_OutPutMutex);

    if(m_vtsResult.size() > 0)
    {
        std::vector<ViolationInfo>::iterator it = m_vtsResult.begin();

        vResult.push_back(*it);
		//先判断
        m_vtsResult.erase(it);
    }
     //解锁
    pthread_mutex_unlock(&m_OutPutMutex);
}


//输出结果处理
void CRoadCarnumDetect::DealOutPut()   
{
    RECORD_PLATE plate;
    PLATEPOSITION TimeStamp;
	int count = 0;

    while(!m_bEndDetect)
    {
        std::vector<CarInfo> vResult;
		vResult.clear();//add by wantao

		if(g_nDetectMode == 2)
		{
			//自动判断白天还是晚上
			if(m_nDetectTime == DETECT_AUTO)
			{
				if(count>1000)
				{
					count = 0;
				}
				if(count==0)
				{
					m_nDayNight = DayOrNight();
					m_nDayNightbyLight = DayOrNight(1);
				}
				count++;
			}
			///////////////////
			//重读检测区域
			if(m_bReloadObjectROI)
			{
				//先更新算法自定义结构文件
				CAMERA_CONFIG cfg;
				cfg.uType = 1;
				cfg.uKind = m_nCameraType;
				cfg.nIndex = (int)CAMERA_GET_STRC;	

				if(!m_bEndDetect)
				{
					g_skpChannelCenter.CameraControl(m_nChannelId, cfg);
					usleep(500 * 1000);
				}
				else
				{
					LogNormal("=Te==m_bEndDetect=%d=\n", m_bEndDetect);
				}

				//LogNormal("===after===g_skpChannelCenter.CameraControl=DealOutPut()=\n");
				//LogNormal("==TT=m_bEndDetect=%d==\n", m_bEndDetect);				
				
				bool bLoadDspSettingInfo = LoadDspSettingInfo2();
				if(!bLoadDspSettingInfo)
				{
					LogError("=m_bReloadObjectROI=GetRoadXml=error!\n");
				}
				//LogNormal("after LoadDspSettingInfo2 22 ParaMapsize:%d ", m_vtsObjectParaMap.size());

				//获取图像宽高
				int nWidth = 0;
				int nHeight = 0;
				if(GetImageWidthHeight(nWidth,nHeight))
				{
					//分配内存空间
					AlignMemory(nWidth,nHeight);

					SetObjectDetectPara();
				}
				
				m_bReloadObjectROI = false;
			}

            //Dsp的PopOutPut（vResult）

			vResult.clear();
			Dsp_PopOutPut(vResult);
			//Dsp_PopOutPutOld(vResult);

			//LogNormal("after Dsp_PopOutPut():vResult.size()=%d\n",vResult.size());//add by wantao
		}
		else
		{
		    //LogNormal("=Yu==PopOutPut==()\n");
			PopOutPut(vResult);
		}

		if(vResult.size() > 0)
		{
			printf("before ==== OutPutKakou\n");
			//输出卡口
			OutPutKakou(vResult);  
			printf("after ==== OutPutKakou\n");
		}
		else
		{
			usleep(100*1);
		}
		

		{
			std::vector<ViolationInfo> vtsResult;

			/*if(g_nDoSynProcess == 1)//前后牌比对
			{
				PopVtsOutPutMatch(vtsResult);
			}
			else*/
			{
				PopVtsOutPut(vtsResult);
			}

			if(vtsResult.size() > 0)
			{
					if( ((m_nDetectKind&DETECT_VTS)==DETECT_VTS) &&((m_nDetectKind&DETECT_LOOP)==DETECT_LOOP) )
					{
						usleep(1000*200);
					}
					
					OutPutVTSResult(vtsResult);
			}
			else
			{
				usleep(100*1);
			}		

			//流量统计
			if(g_nDetectMode == 2)
			{
				StatResultList listStatResult;
				UINT32 uTimestamp = GetTimeStamp();

				VehicleStatistic(listStatResult,uTimestamp);
			}
		}

		//强制输出队列中的内容
		#ifndef ALGORITHM_YUV
		if(g_nDetectMode != 2)//非dsp模式
		{
			if(m_nSaveImageCount == 2)//2张图
			{
				if(g_nPicMode == 1) //分开
				{
					TimeStamp.uTimestamp = GetTimeStamp();
					OutPutPreResult(plate,&TimeStamp,1);
				}
				else
				{
					if(((m_nDetectKind&DETECT_LOOP)==DETECT_LOOP))
					{
						CarInfo cardNum;
						cardNum.id = -1;
						cardNum.uSeq = 0;
						cardNum.uTimestamp = GetTimeStamp();
						RECORD_PLATE plate;
						std::vector<ObjSyncData>::iterator it_s;
						OutPutWriteBack(cardNum,plate,it_s);
					}
				}
			}
			//强制输出回写队列中的内容
			else if(m_nSaveImageCount == 1)
			{
				if(((m_nDetectKind&DETECT_LOOP)==DETECT_LOOP))
				{
					CarInfo cardNum;
					cardNum.id = -1;
					cardNum.uSeq = 0;
					cardNum.uTimestamp = GetTimeStamp();
					RECORD_PLATE plate;
					std::vector<ObjSyncData>::iterator it_s;
					OutPutWriteBack(cardNum,plate,it_s);
				}
			}
		}
		#endif
    }
}

//获取车道控制灯状态
void CRoadCarnumDetect::GetSignalStatus(yuv_video_buf* buf)
{
	UINT16 uTrafficSignal = buf->uTrafficSignal;
	UINT32 uSeq = buf->nSeq;

    m_vtsSignalList.clear();
    VTSParaMap::iterator it_p = m_vtsObjectParaMap.begin();
    while(it_p != m_vtsObjectParaMap.end())
    {
        MvChannelFrameSignalStatus vtsSignalStatus;

        vtsSignalStatus.nRoadIndex =  it_p->second.nRoadIndex;

        vtsSignalStatus.uFrameSeq = uSeq;

        vtsSignalStatus.bLeftSignal = ((uTrafficSignal>>(it_p->second.nLeftControl)) & 0x1);
        vtsSignalStatus.bStraightSignal = ((uTrafficSignal>>(it_p->second.nStraightControl)) & 0x1);
        vtsSignalStatus.bRightSignal = ((uTrafficSignal>>(it_p->second.nRightControl)) & 0x1);

		m_vtsSignalList.push_back(vtsSignalStatus);
        printf("=====nRoadIndex=%d,bLeftSignal=%d,bStraightSignal= %d,bRightSignal=%d,m_vtsSignalList.size()=%d\n",vtsSignalStatus.nRoadIndex,vtsSignalStatus.bLeftSignal,vtsSignalStatus.bStraightSignal,vtsSignalStatus.bRightSignal,m_vtsSignalList.size());
        it_p++;
    }
}

//获取车道线圈状态
void CRoadCarnumDetect::SetLoopStatus( Speed_Signal uLoopSignal)
{
	#ifndef ALGORITHM_YUV
    LoopParaMap::iterator it_p = m_LoopParaMap.begin();
    while(it_p != m_LoopParaMap.end())
    {
        int nRoadIndex = it_p->first;
        int nLoopIndex = it_p->second.nLoopIndex;
        bool bFrontLoop = ((uLoopSignal.SpeedSignal>>(nLoopIndex*2)) & 0x01);//前线圈
        bool bBackLoop = ((uLoopSignal.SpeedSignal>>(nLoopIndex*2+1)) & 0x01);//后线圈

        printf("=nRoadIndex=%d,nLoopIndex=%d,bFrontLoop=%d,bBackLoop= %d,LoopTime=%lld,SystemTime=%lld\n",nRoadIndex,nLoopIndex,bFrontLoop,bBackLoop,uLoopSignal.SpeedTime,uLoopSignal.SystemTime);
        m_ftVlp.SetPhysicLoopStatus(nRoadIndex, bFrontLoop, bBackLoop, uLoopSignal.SpeedTime,uLoopSignal.SystemTime);

        it_p++;
    }
	#endif
}

////判断是否发生线圈电警违章行为
void CRoadCarnumDetect::DoLoopVts(UINT32 nSeq,int64_t ts,UINT32 uTimestamp,Speed_Signal uLoopSignal,std::vector<ViolationInfo>& vtsResult)
{
	#ifndef ALGORITHM_YUV
	ChnFrmSignalStatusList::iterator it_b = m_vtsSignalList.begin();
	ChnFrmSignalStatusList::iterator it_e = m_vtsSignalList.end();

	while(it_b != it_e)
	{
		 MvChannelFrameSignalStatus vtsSignalStatus;

		 vtsSignalStatus = *it_b;

		if(vtsSignalStatus.bStraightSignal)
		{
			LoopStatusMap::iterator it = m_mapLoopStatus.find(vtsSignalStatus.nRoadIndex);

			LoopParaMap::iterator it_p = m_LoopParaMap.find(vtsSignalStatus.nRoadIndex);

			if( (it != m_mapLoopStatus.end()) && (it_p != m_LoopParaMap.end()) )
			{
				int nLoopIndex = it_p->second.nLoopIndex;
				bool bBackLoop = ((uLoopSignal.SpeedSignal>>(nLoopIndex*2+1)) & 0x01);//后线圈

				bool bPreBackLoop = it->second.bBackLoop;

				if( (bPreBackLoop == 0) && (bBackLoop == 1) )//发生违章检测行为
				{
					printf("bPreBackLoop=%d,bBackLoop=%d\n",bPreBackLoop,bBackLoop);
					ViolationInfo info;

					info.evtType = ELE_RED_LIGHT_VIOLATION;

					info.nPicCount = 3;

					info.nChannel = vtsSignalStatus.nRoadIndex;

					info.frameSeqs[0] = nSeq-3;
					info.frameSeqs[1] = nSeq;
					info.frameSeqs[2] = nSeq+3;

					CarInfo cardNum;
					cardNum.ix = 100;
					cardNum.iy = 100;
					cardNum.iwidth = 100;
					cardNum.iheight = 100;

					memcpy(cardNum.strCarNum,"*******",8);
					cardNum.uTimestamp = uTimestamp;

					cardNum.ts = ts;
					cardNum.uSeq = nSeq;

					info.carInfo = cardNum;

				    vtsResult.push_back(info);
				}
			}
		}

		it_b++;
	}

	//存储上次车道线圈状态
	LoopParaMap::iterator it_p = m_LoopParaMap.begin();
    while(it_p != m_LoopParaMap.end())
    {
		int nRoadIndex = it_p->first;
        int nLoopIndex = it_p->second.nLoopIndex;
        bool bFrontLoop = ((uLoopSignal.SpeedSignal>>(nLoopIndex*2)) & 0x01);//前线圈
        bool bBackLoop = ((uLoopSignal.SpeedSignal>>(nLoopIndex*2+1)) & 0x01);//后线圈


		LoopStatusMap::iterator it = m_mapLoopStatus.find(nRoadIndex);
		if(it != m_mapLoopStatus.end())
		{
			it->second.bFrontLoop = bFrontLoop;
			it->second.bBackLoop = bBackLoop;
		}

		it_p++;
	}
	#endif
}


//获取车道爆闪灯状态
void CRoadCarnumDetect::GetFlashStatus(unsigned short uFlashSignal,int64_t ts,vector<mvvideoshutterpar>& videoshutterpar)
{
	#ifndef ALGORITHM_YUV
    vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
    while(it != m_vtsObjectRegion.end())
    {
        mvvideoshutterpar vsp;
        vsp.nRoadIndex =  it->nRoadIndex;
        vsp.ts = ts;
        vsp.bFlagShutter = ((uFlashSignal >> (vsp.nRoadIndex -1)) & 0x01);

        printf("=================vsp.nRoadIndex = %d,vsp.bFlagShutter=%d,vsp.ts=%lld\n",vsp.nRoadIndex,vsp.bFlagShutter,vsp.ts);
        videoshutterpar.push_back(vsp);
        it++;
    }
	#endif
}

//添加秒图
void CRoadCarnumDetect::AddResultFrameList(BYTE* frame)
{
    {
		yuv_video_buf* buf = (yuv_video_buf*)(frame);

		int nMaxSize = 45;//
		int nDtTime = 200000; //0.2s一张
		if(m_imgSnap->width > 2000)//500万
		{
			nMaxSize = 15;
			nDtTime = 500000;
		}
		

        if(m_ResultFrameList.size() > nMaxSize)
        {
            m_ResultFrameList.pop_front();
        }

        if( m_ResultFrameList.size() >0 )
        {
            std::list<std::string>::iterator it = --m_ResultFrameList.end();

            yuv_video_buf* sHeader = (yuv_video_buf*)it->c_str();

            if(buf->ts >= sHeader->ts + nDtTime)
            {
                std::string strFrame;
				#ifndef ALGORITHM_YUV_CARNUM
                strFrame.append((char*)frame,sizeof(yuv_video_buf)+m_imgSnap->imageSize);
				#else
				strFrame.append((char*)frame,sizeof(yuv_video_buf)+m_imgSnap->imageSize*2/3);
				#endif
                m_ResultFrameList.push_back(strFrame);
            }
        }
        else
        {
            std::string strFrame;
			#ifndef ALGORITHM_YUV_CARNUM
            strFrame.append((char*)frame,sizeof(yuv_video_buf)+m_imgSnap->imageSize);
			#else
			strFrame.append((char*)frame,sizeof(yuv_video_buf)+m_imgSnap->imageSize*2/3);
			#endif
            m_ResultFrameList.push_back(strFrame);
        }
    }
}


//初始化目标检测库
bool CRoadCarnumDetect::SetObjectDetectPara()
{
	float homography[3][3];

	if(g_nDetectMode != 2)
	{
		//读取车道标定信息
		vector<mvvideostd> vListStabBack;
		CvRect farrect;
		CvRect nearrect;
		if(!LoadRoadSettingInfo(0,vListStabBack,farrect,nearrect))
		{
			LogError("SetObjectDetectPara LoadRoadSettingInfo error\n");
			return false;
		}
	
		//载入车道检测参数（读取流量统计参数）
		SRIP_CHANNEL_EXT sChannel;
		sChannel.uId = m_nChannelId;
		//paraDetectList roadParamInlist;
		m_roadParamInlist.clear();
		CXmlParaUtil xml;
		xml.LoadRoadParameter(m_roadParamInlist,sChannel);
		if(sChannel.uTrafficStatTime > 0)
			m_nTrafficStatTime = sChannel.uTrafficStatTime;

		//违章检测参数
		VTS_GLOBAL_PARAMETER vtsGlobalPara;
		CXmlParaUtil xmlvts;
		xmlvts.LoadVTSParameter(m_vtsObjectParaMap,m_nChannelId,vtsGlobalPara);
		m_vtsGlobalPara = vtsGlobalPara;

		#ifdef ALGORITHM_YUV
			//参数转换为MvDspGlobalSetting结构
			ConvertDspSetting(farrect,nearrect);
			MvDspSaveOrgSettingFile();
			int nInit = mvInit();
			printf("mvInit nInit=%d\n",nInit);
		#else
			//释放
			m_ftVlp.Destroy();
		#endif
		
		printf("-1111-m_ftVlp.Destroy()--\n");			

		//获取车道检测参数
		if((m_nDetectKind&DETECT_VTS)||(m_nDetectKind&DETECT_VIOLATION)|| (g_PicFormatInfo.nSpeedLimit == 1) || (m_nDetectKind&DETECT_TRUCK))
		{
			//需要分配内存
			if(m_imgComposeSnap == NULL)
			{
				int uWidth = m_imgSnap->width;
				int uHeight = m_imgSnap->height;

				LogNormal("m_imgSnap->width=%d,m_imgSnap->height=%d\n", \
					m_imgSnap->width, m_imgSnap->height);

				m_imgComposeResult = cvCreateImage(cvSize(uWidth,uHeight),8,3);

				if(m_nWordOnPic == 1)
				{
					uHeight -= m_nExtentHeight;
				}

				if(g_nServerType == 14)//北京公交项目特殊图片格式
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(m_imgSnap->height)*2-2*m_nExtentHeight+120),8,3);
				}
				else
				{
					//闯红灯检测三帧合成图像
					if(g_nVtsPicMode == 0) //3x1
					{
						//LogNormal("m_imgComposeSnap cvCreateImage uWidth=%d,uHeight=%d", uWidth, uHeight);
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth,(uHeight)*3),8,3);
					}
					else if(g_nVtsPicMode == 1) //2x2
					{
#ifdef WEN_LING_VTS_TEST
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*2 - m_nExtentHeight),8,3);
						//LogNormal("Init uWidth:%d,uHeight:%d m_nExtentHeight:%d", uWidth, uHeight, m_nExtentHeight);
#else
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*2),8,3);
#endif
					}
					else if(g_nVtsPicMode == 2) //3x2
					{
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*3),8,3);
					}
					else if(g_nVtsPicMode == 3)//1x2
					{
						if(1 == g_PicFormatInfo.nSmallViolationPic)//武汉倒品字形图片
						{
							if(g_nServerType == 3)
							{
								m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2+uHeight,(uHeight-m_nExtentHeight+120)),8,3);//旅行时间违章图片格式
							}
							else
							{
								m_imgComposeSnap = cvCreateImage(cvSize(800,1180),8,3);//武汉倒品字形图片
							}
						}
						else
						{
							m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)),8,3);
						}
					}
					else if(g_nVtsPicMode == 4)//1x1  //深圳项目格式
					{
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,uHeight),8,3);
					}
					else if(g_nVtsPicMode == 5)//1x3
					{
						if(1 == g_PicFormatInfo.nSmallViolationPic)
						{
							if(g_nServerType == 23 || g_nServerType == 26)//济南图片格式
							{
								m_imgComposeSnap = cvCreateImage(cvSize(uWidth*4,uHeight),8,3);
							}
							else //遵义违章格式
							{
								m_imgComposeSnap = cvCreateImage(cvSize(uWidth*4,(uHeight-m_nExtentHeight+120)),8,3);
							}
						}
						else
						{
							m_imgComposeSnap = cvCreateImage(cvSize(uWidth*3,(uHeight)),8,3);
						}
					}
				}

				printf("m_nWordOnPic=%d,m_imgComposeSnap->height=%d,uHeight=%d,m_nExtentHeight=%d\n",m_nWordOnPic,m_imgComposeSnap->height,uHeight,m_nExtentHeight);
			}

			#ifndef ALGORITHM_YUV
			vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
			while( it !=  m_vtsObjectRegion.end() )
			{
				VTSParaMap::iterator it_p = m_vtsObjectParaMap.find(it->nRoadIndex);
				if(it_p != m_vtsObjectParaMap.end())
				{
					it->bNoTurnLeft = it_p->second.bForbidLeft;
					it->bNoTurnRight =   it_p->second.bForbidRight;
					it->bNoForeward =   it_p->second.bForbidRun;
					it->bNoReverse = it_p->second.bAgainst;
					it->bNoChangeChannel = it_p->second.bChangeRoad;
					it->bNoPressLine = it_p->second.bPressLine;
					it->nChannelDriveDir = it_p->second.nRoadDirection;
					it->bFlagHoldStopReg = it_p->second.nFlagHoldStopReg;
					it->m_RedLightTime = it_p->second.nRedLightTime;		
					it->bNoPark = it_p->second.bForbidStop;
				

					//LogNormal("it->bNoTurnLeft=%d,it->bNoTurnRight=%d,it->bNoPark=%d\n",it->bNoTurnLeft,it->bNoTurnRight,it->bNoPark);

					NoPassingInfo info;
					info.nVehType = it_p->second.nForbidType;
					info.nStart = it_p->second.nForbidBeginTime;
					info.nEnd = it_p->second.nForbidEndTime;

					printf("it->nRoadIndex=%d,info.nVehType=%d,%d-%d\n",it->nRoadIndex,info.nVehType,info.nStart,info.nEnd);
					it->vecNoPassingInfo.push_back(info);
				}
				it++;
			}					
			
			//LogNormal("-1111-m_ftVlp.SetForeLine-\n");

			//设置四条检测线
			m_ftVlp.SetForeLine(m_lineStraight);

			printf("--m_lineStraight.start=[%d],[%d]-\n", m_lineStraight.start.x, m_lineStraight.end.x);

			m_ftVlp.SetLeftLine(m_lineTurnLeft);
			m_ftVlp.SetRightLine(m_lineTurnRight);
			m_ftVlp.SetStopLine(m_lineStop);
			
			printf("-1111-m_ftVlp.SetStopLine-\n");

			if((m_nDetectKind&DETECT_VTS)==DETECT_VTS)
			{
				m_ftVlp.SetVideoStdlib(vListStabBack);

				LogNormal("bStrongLight=%d,bGlemdLight=%d,bCheckLightByImage=%d\n",vtsGlobalPara.bStrongLight, vtsGlobalPara.bGlemdLight, vtsGlobalPara.bCheckLightByImage);
				m_ftVlp.SetRedAndGreenStrongGlemd(vtsGlobalPara.bStrongLight, vtsGlobalPara.bGlemdLight, vtsGlobalPara.bCheckLightByImage);
			}

			m_ftVlp.SetBianDaoXian(m_vecBianDaoLines);
			m_ftVlp.SetYellowLine(m_vecYellowLines);
			m_ftVlp.SetWhiteLine(m_vecWhiteLines);	
			#endif
		}

		#ifndef ALGORITHM_YUV
		//设置车道区域(无论哪种检测均需要给出所在的车道编号)
		if(m_vtsObjectRegion.size() > 0)
		{
			printf("-1111--m_ftVlp.SetChannels-m_vtsObjectRegion.size()=%d\n", m_vtsObjectRegion.size());
			m_ftVlp.SetChannels(m_vtsObjectRegion);
			printf("-222--m_ftVlp.SetChannels-m_vtsObjectRegion.size()=%d\n", m_vtsObjectRegion.size());			
		}
		//设置目标检测参数
		{
			_ObjectPara ObjectPara;
			CXmlParaUtil xml;
			xml.LoadObjectParameter(m_nChannelId,ObjectPara);

			_ParaConfig vlpPara;
			vlpPara.bOutPutNonAutoMobile = ObjectPara.bOutPutNonAutoMobile;
			vlpPara.bDetectNonVehicle = ObjectPara.bDetectNonVehicle;
			vlpPara.bFilterSideVehicle = ObjectPara.bFilterSideVehicle;
			vlpPara.bFilterSideVlpObj = ObjectPara.bFilterSideVlpObj;
			vlpPara.bDoNonPlateDetect = ObjectPara.bDetectNonPlate;
			vlpPara.nRedLightViolationAllowance = ObjectPara.nRedLightViolationAllowance;
			vlpPara.nFramePlus = ObjectPara.nFramePlus;
			vlpPara.fLoopVehicleLenFix = ObjectPara.fLoopVehicleLenFix;
			vlpPara.fRedLightVioPic[0] = ObjectPara.fFarRedLightVioPic;
			vlpPara.fRedLightVioPic[1] = ObjectPara.fNearRedLightVioPic;

			m_bDetectShield = ObjectPara.bDetectShield;
			m_bImageEnhance = ObjectPara.bImageEnhance;
			m_nImageEnhanceFactor = ObjectPara.nImageEnhanceFactor;


			m_ftVlp.SetParaForVlpEle(vlpPara);
			m_ftVlp.SetVideoShutterDelayFrame(ObjectPara.nDelayFrame);
			m_ftVlp.SetPressLineScale(ObjectPara.fPressLineScale);
		}

		//设置帧率
		m_ftVlp.SetFrameRate(m_nFrequency);

		//线圈测速
		if(((m_nDetectKind&DETECT_LOOP)==DETECT_LOOP))
		{
			LoopParaMap MapLoopPara;

			CXmlParaUtil xml;
			xml.LoadLoopParameter(MapLoopPara,m_nChannelId);

			m_mapLoopStatus.clear();
			m_LoopParaMap.clear();
			vector<LoopInfo>::iterator it_b = m_LoopInfo.begin();
			vector<LoopInfo>::iterator it_e = m_LoopInfo.end();
			printf("m_LoopInfo.size()=%d\n",m_LoopInfo.size());
			while(it_b!=it_e)
			{
				LoopParaMap::iterator it_p = MapLoopPara.find(it_b->nRoadIndex);
				if(it_p!= MapLoopPara.end())
				{
					it_b->dis = it_p->second.fDistance;
					printf("=====line0.start.x=%d,line0.end.y=%d,line1.start.x=%d,line1.end.y=%d==============\n",it_b->line0.start.x,it_b->line0.end.y,it_b->line1.start.x,it_b->line1.end.y);
					printf("=======it_b->dis=%f===============\n",it_b->dis);

					m_LoopParaMap.insert(LoopParaMap::value_type(it_b->nRoadIndex,it_p->second));

					LOOP_STATUS loopStatus;
					loopStatus.nRoadIndex = it_b->nRoadIndex;
					m_mapLoopStatus.insert(LoopStatusMap::value_type(it_b->nRoadIndex,loopStatus));
				}
				it_b++;
			}
			m_ftVlp.SetPhysicLoop(m_LoopInfo);

			//表明和Dsp相机做线圈爆闪
			m_ftVlp.SetDspLoopShutter(false);
		}

		//雷达测速
		if(((m_nDetectKind&DETECT_RADAR)==DETECT_RADAR))
		{
			RadarParaMap MapRadarPara;
			CXmlParaUtil xml;
			xml.LoadRadarParameter(m_nChannelId,MapRadarPara);

			int nCoverRoadIndex;
			vector<int> radarRoadIndex;
			RadarParaMap::iterator it = MapRadarPara.begin();
			while(it != MapRadarPara.end())
			{
				nCoverRoadIndex = it->second.nCoverRoadIndex;
				m_fSpeedFactor = it->second.fSpeedFactor;

				int nRoadIndex =  it->second.nRoadIndex;

				if((nCoverRoadIndex&(1<<(nRoadIndex-1))) != 0)
				{
					radarRoadIndex.push_back(nRoadIndex);
				}
				it++;
			}
			m_ftVlp.SetRadarDetection(m_rtRadarRoi,radarRoadIndex);
		}

		//设置检测类型
		m_ftVlp.SetDetectType(m_nDetectKind);
		//是否相机同步
		m_ftVlp.SetSyncSwitch(0);
		//设置检测区域
		//printf("m_rtVlpRoi.x=%d,m_rtVlpRoi.y=%d,m_rtVlpRoi.width=%d,m_rtVlpRoi.height=%d\n",m_rtVlpRoi.x,m_rtVlpRoi.y,m_rtVlpRoi.width,m_rtVlpRoi.height);
		m_ftVlp.SetCarNumDetectROI(m_rtVlpRoi);


		if((m_nDetectKind&DETECT_VIOLATION)==DETECT_VIOLATION)
		{
			//设置黄网格区域
			//if(m_ptsYelGrid.size() > 0)
			{
				MvPolygon poly(m_ptsYelGrid);
				m_ftVlp.SetYelGridRgn(poly);
				//LogNormal("-m_ftVlp.SetYelGridRgn--m_ptsYelGrid.size()-%d", m_ptsYelGrid.size());//m_ftVlp.SetVioRushTime();					
			}

			int nStopTime = vtsGlobalPara.nStopTime;
			if(nStopTime > 0)
			{
				m_ftVlp.SetVioRushTime(nStopTime);
				//LogNormal("--m_ftVlp.SetVioRushTime-nStopTime=%d \n", nStopTime);
			}
		}

		//设置检测方向(取第一个车道)
		int nDirection = 0;
		vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
		if(it != m_vtsObjectRegion.end())
		{
			nDirection = it->nDirection;
		}
		m_nDetectDirection = nDirection;
		//LogNormal("id:%d, m_nDetectDirection:%d", m_nChannelId, m_nDetectDirection);
		
		m_ftVlp.SetDirection(nDirection);

		if (g_nServerType == 13)
		{
			g_mvsCommunication.SetDetectDirection(m_nDetectDirection);
		}
		else if(23 == g_nServerType || 26 == g_nServerType)
		{
			g_mvsCommunication.SetDetectDirection(m_nDetectDirection);
		}
		else
		{}

		if (g_nServerType == 13 && g_PicFormatInfo.nSmallPic != 1)//卡口图片不存储小图
		{
			g_MyCenterServer.setDirection(m_nDetectDirection);
		}

		//设置标定
		if(m_nDeinterlace == 1)//帧图象
		{
			m_ftVlp.SetCalibCorners(m_image_cord, m_world_cord, 6);
		}
		else if(m_nDeinterlace == 2)//场图象
		{
			for(int i = 0; i<12; i++)
			{
				if(i%2==1)
				{
					m_image_cord[i] /=m_nDeinterlace;
				}
			}
			m_ftVlp.SetCalibCorners(m_image_cord, m_world_cord, 6);
		}
		//设置图像尺寸
		m_ftVlp.SetImageSize(m_img->width,m_img->height);

		//设置统计周期
		m_ftVlp.SetTrafficStatTime(m_nTrafficStatTime);

		//计算标定
		mvfind_homography(m_image_cord,m_world_cord,homography);//基于帧图象

		#ifndef GLOBALCARCOLOR
		m_ftVlp.SetColorDetector(&m_carColor);
		#else
		m_ftVlp.SetColorDetector(&g_carColor);
		#endif

		//是否使用爆闪灯
		m_ftVlp.SetLoopShutter(g_nHasHighLight);

		if(g_nFlashControl == 1)
			m_ftVlp.SetVideoShutter(true);
		else
			m_ftVlp.SetVideoShutter(false);
		
		LogNormal("before m_ftVlp.Init\n");
		//目标检测初始化
		if(m_rtVlpRoi.width > 0 && m_rtVlpRoi.height > 0)
			m_ftVlp.Init();
		LogNormal("after m_ftVlp.Init\n");

		#endif
	}
	else
	{
		#ifndef ALGORITHM_YUV_CARNUM
		#ifdef ALGORITHM_DL
		m_bSetCarnumHeight = false;
		char buf[64] = {'\0'};
		sprintf(buf,"./BocomMv/%d",4);
		LogNormal("%s",mvcarnumdetect.GetVersion());
		m_bInitCarNumLib = mvcarnumdetect.carnum_init(buf,NULL,m_imgSnap->width,m_imgSnap->height);
		m_bInitCarNumLib = 1;
		#endif
		#endif

		
		//违章检测参数
		VTSParaMap mapVTSPara;
		VTS_GLOBAL_PARAMETER vtsGlobalPara;
		CXmlParaUtil xmlvts;
		LogNormal("before LoadVTSParameter 33 ParaMapsize:%d ", m_vtsObjectParaMap.size());
		xmlvts.LoadVTSParameter(mapVTSPara,m_nChannelId,vtsGlobalPara);
		LogNormal("before LoadVTSParameter 44 ParaMapsize:%d ", m_vtsObjectParaMap.size());
		//m_vtsGlobalPara.nSpeedVal = vtsGlobalPara.nSpeedVal;
		//LogNormal("nSpeedVal:%d\n",m_vtsGlobalPara.nSpeedVal);
		
		m_vtsGlobalPara = vtsGlobalPara;

		//设置目标检测参数
		{
			_ObjectPara ObjectPara;
			CXmlParaUtil xml;
			xml.LoadObjectParameter(m_nChannelId,ObjectPara);
			
			SRIP_CHANNEL_EXT sChannel;
			sChannel.uId = m_nChannelId;
			paraDetectList roadParamInlist;
			xml.LoadRoadParameter(roadParamInlist,sChannel);
			if(sChannel.uTrafficStatTime > 0)
				m_nTrafficStatTime = sChannel.uTrafficStatTime;

			m_bDetectShield = ObjectPara.bDetectShield;
			m_bImageEnhance = ObjectPara.bImageEnhance;
			m_nImageEnhanceFactor = ObjectPara.nImageEnhanceFactor;
			m_vtsGlobalPara.nStrongEnhance = ObjectPara.nStrongEnhance;
		}
		
	}
	
	//红绿灯增强初始化
#ifdef REDADJUST
	if(m_vtsGlobalPara.bStrongSignal)
	{
		bool bInitRedAdjust = InitRedAdjust2();
		if(bInitRedAdjust)
		{
			LogNormal("id:%d 初始化红灯增强2成功!", m_nChannelId);
		}
		else
		{
			LogError("id:%d 初始化红灯增强2失败!", m_nChannelId);
		}
	}
	else
	{
		m_nRedNum = 0;
		CvRect rectRed[MAX_RED_LIGHT_NUM];//红灯的矩形区域
		vector<ChannelRegion>::iterator it_r = m_vtsObjectRegion.begin();
		while(it_r != m_vtsObjectRegion.end())
		{
				CvRect roiLeftLight;
				roiLeftLight.x =  it_r->roiLeftLight.x;
				roiLeftLight.y =  it_r->roiLeftLight.y;
				roiLeftLight.width =  it_r->roiLeftLight.width;
				roiLeftLight.height =  it_r->roiLeftLight.height;
				if(roiLeftLight.width > 0)
				{
					rectRed[m_nRedNum] = roiLeftLight;
					m_nRedNum++;							
				}

				CvRect roiMidLight;
				roiMidLight.x =  it_r->roiMidLight.x;
				roiMidLight.y =  it_r->roiMidLight.y;
				roiMidLight.width =  it_r->roiMidLight.width;
				roiMidLight.height =  it_r->roiMidLight.height;
				if(roiMidLight.width > 0)
				{
					rectRed[m_nRedNum] = roiMidLight;
					m_nRedNum++;	
				}

				CvRect roiRightLight;
				roiRightLight.x =  it_r->roiRightLight.x;
				roiRightLight.y =  it_r->roiRightLight.y;
				roiRightLight.width =  it_r->roiRightLight.width;
				roiRightLight.height =  it_r->roiRightLight.height;
				if(roiRightLight.width > 0)
				{
					rectRed[m_nRedNum] = roiRightLight;
					m_nRedNum++;	
				}	
				it_r++;
		}
#ifdef DRAW_RECT_DEBUG
		//myRectangle(pImage, rect, CV_RGB(255, 0, 255), 5);
		for(int i=0; i<m_nRedNum; i++)
		{
			m_rectRed[i].x = rectRed[i].x;
			m_rectRed[i].y = rectRed[i].y;
			m_rectRed[i].width = rectRed[i].width;
			m_rectRed[i].height = rectRed[i].height;
			LogNormal("rectRed[%d]: %d,%d,%d,%d", \
				i, rectRed[i].x, rectRed[i].y, rectRed[i].width, rectRed[i].height);
		}		
#endif

#ifdef RED_GREEN_ENHANCE
		LogNormal("RED_GREEN_ENHANCE m_nRedNum:%d ", m_nRedNum);
#endif

		for(int i=0; i<m_nRedNum; i++)
		{
			m_rectRedArray[i].x =  rectRed[i].x;
			m_rectRedArray[i].y =  rectRed[i].y;
			m_rectRedArray[i].width =  rectRed[i].width;
			m_rectRedArray[i].height =  rectRed[i].height;
		}

		if(m_pRedLightAdjust != NULL)
		{
			delete m_pRedLightAdjust;
		}
		m_pRedLightAdjust = new MvRedLightAdjust(rectRed, m_nRedNum);
		
		if(m_pRedLightAdjust)
		{	
			LogNormal("id:%d 初始化红灯增强1成功!", m_nChannelId);
		}
		else
		{
			LogError("id:%d 初始化红灯增强1失败!", m_nChannelId);
		}
	}//End of else
#endif //End of #ifdef REDADJUST


	#ifdef OBJECTHEADDETECT
	if(m_nDetectKind&DETECT_CHARACTER)
	{
		m_HeadDetection.uninit();
		m_HeadDetection.init("LBPcascade2.xml","svm_Mode.svm","LowerFaceDetector.xml");
	}
	#endif

	#ifdef OBJECTFACEDETECT
	if(m_nDetectKind&DETECT_FACE)
	{
		MV_STFACEDET fdParam;
		fdParam.m_strFFDet = "KaKou_FaceDetector.xml";
		fdParam.m_strLFDet = "KaKou_LowerFaceDetector.xml";
		fdParam.m_strLRDet = "KaKou_LeftRotDetectModel.xml";
		fdParam.m_strRRDet = "KaKou_RightRotDetectModel.xml";
		fdParam.m_strLPDet = "KaKou_LeftProfileDetectModel.xml";
		fdParam.m_strHSDet = "KaKou_HeadshoulderModel.xml";
		m_FaceDetection.mv_Uninit();
		m_FaceDetection.mv_Init(fdParam);

		//m_FaceTrackSearch.mv_UnInitial();
		mv_stFSParam param;
		param.m_strFaceDetectorFileName = "./FrontFaceDetector.xml";
		param.m_strASModelFileName = "./asmmodel";
		param.m_strFeatModelFileName = "./FaceFeatureModel";
		//m_FaceTrackSearch.mv_Initial(param);
	}
	#endif

	#ifdef OBJECTFEATURESEARCH
	if(m_nDetectKind&DETECT_CHARACTER)
	{
		mvSearchInterfaceParamer para;
		para.nApplicationType = F_KAKOU; //应用类型，做卡口目标检索
		para.m_iface_width = m_imgSnap->width; //原始图像宽度
		para.m_iface_height = m_imgSnap->height; //原始图像的高度
		if(m_pFeatureSearchDetect)
		{
			m_pFeatureSearchDetect->MvKK_Destroy();
			delete m_pFeatureSearchDetect;
			m_pFeatureSearchDetect = NULL;
		}
		m_pFeatureSearchDetect = new MvPeopleSearch();
		m_pFeatureSearchDetect->MvKK_Init(&para);
	}
	#endif
	
	#ifndef ALGORITHM_YUV_CARNUM
	//颜色库标定设置
	#ifndef GLOBALCARCOLOR
	if(m_bLoadCarColor)
	{
		m_carColor.color_Destroy(); //	
	}	
	if(m_nDetectKind&DETECT_CARCOLOR)
	{
		LogNormal("==m_carColor.color_Init\n");
		m_carColor.color_Init( homography, m_nCameraType,m_imgFrame->width,m_imgFrame->height,0 );
		LogNormal("ColorLib Version:%s",m_carColor.GetVersion());
		m_bLoadCarColor = true;
	}
	#endif
	
	#ifndef GLOBALCARLABEL
	if(m_bLoadCarLabel)
	{
		m_carLabel.carLabel_Destroy();
	}
	bool bInitCarLabel = ((m_nDetectKind&DETECT_TEXTURE)==DETECT_TEXTURE);
	if(bInitCarLabel)
	{
		LogNormal("=m_carLabel.carLabel_Init\n");
		m_carLabel.carLabel_Init( m_imgFrame->width,m_imgFrame->height,bInitCarLabel );
		LogNormal("LabelLib Version:%s",m_carLabel.GetVersion());
		m_bLoadCarLabel = true;
	}
	#endif
	#endif

	#ifdef PLATEDETECT
	m_PlateDetector.mvInit("plate.xml");
	#endif

	#ifdef BELTDETECT
	if(m_nDetectKind&DETECT_FACE)
	{
		m_safeType.SafetyBelt_Destroy();
		m_safeType.SafetyBelt_Init();
		LogNormal("%s",m_safeType.GetVersion());
	}
	#endif
#ifndef GLOBALCARCLASSIFY
	
	if (m_nDetectKind&DETECT_TRUCK)
	{
		LogNormal("TruckLib Version:%s",m_vehicleClassify.GetVersion());
	}
#endif
	//载入通道图片格式设置
	{		
		CXmlParaUtil xml;
		REGION_ROAD_CODE_INFO picFormatInfo;
		if(xml.LoadPicFormatInfo(m_nChannelId,picFormatInfo))
		{
			m_PicFormatInfo.nOffsetX = picFormatInfo.nOffsetX;
			m_PicFormatInfo.nOffsetY = picFormatInfo.nOffsetY;
			m_BlackFrameWidth = picFormatInfo.nBlackFrameWidth;
			m_nFontSize = picFormatInfo.nFontSize;
			
			if (g_nServerType == 15)
			{
				m_nXingTaiCvText.Init(m_nFontSize);
			}
			
			if (g_nServerType == 13)
			{
				g_mvsCommunication.SetOffsetXY(m_PicFormatInfo.nOffsetX,m_PicFormatInfo.nOffsetY);
			}
			else if(23 == g_nServerType || 26 == g_nServerType)
			{
				g_mvsCommunication.SetOffsetXY(m_PicFormatInfo.nOffsetX,m_PicFormatInfo.nOffsetY);
			}
			else{}
		}
		else
		{
			if (g_nServerType == 15)
			{
				m_nXingTaiCvText.Init(15);//默认
			}
			LogNormal("--Not set PicFormat!-\n");
		}
	}

	LogNormal("==after SetObjectDetectPara===ID[%d]\n", m_nChannelId);

	return true;

}

//#define DSP_TRACE printf("%s , %s, Line=%d\n",__FILE__,__FUNCTION__,__LINE__);

//检测目标
void CRoadCarnumDetect::DetectObject(rgb_buf  rgbBuf)
{
	//读取测试结果
	if(g_nFileID > 0)
	{
		if(m_nFileID != g_nFileID)
		{
			m_nFileID = g_nFileID;
			m_listTestResult.clear();

			char filename[256] = {0};
			sprintf(filename,"P%d.data",g_nFileID);

			if(LoadTestResult(filename))
			{
				m_bTestResult = true;
			}
			else
			{
				m_bTestResult = false;
			}
		}
	}
	else
	{
		m_bTestResult = false;
	}

    //重读车牌检测区域
    if(m_bReloadObjectROI)
    {
		LogNormal("before DetectObject SetObjectDetectPara\n");
		if(!SetObjectDetectPara())
		{
			LogError("DetectObject SetObjectDetectPara error\n");
			return;
		}
		LogNormal("after DetectObject SetObjectDetectPara\n");

        m_bReloadObjectROI = false;
    }

    BYTE* frame;
    yuv_video_buf* buf;

    frame = rgbBuf.pCurBuffer;
    buf = (yuv_video_buf*)(frame);

	 //获取车道控制灯状态
    if(((m_nDetectKind&DETECT_VTS)==DETECT_VTS))
    {
        GetSignalStatus(buf);
    }
	
	std::vector<CarInfo> result;            //卡口检测结果
    std::vector<ViolationInfo> vtsResult; //违章检测结果

	#ifndef ALGORITHM_YUV_CARNUM
    if(m_nWordPos == 0)
    cvSetData(m_img,frame+sizeof(yuv_video_buf),m_img->widthStep);
    else
    cvSetData(m_img,frame+sizeof(yuv_video_buf)+m_img->widthStep*m_nExtentHeight/m_nDeinterlace,m_img->widthStep);
	 //取车牌检测结果
    BYTE* pBuf = frame+sizeof(yuv_video_buf)+ (m_img->widthStep)*(m_img->height+m_nExtentHeight/m_nDeinterlace);
	int nCarNumResult =  *((int*)pBuf);         //获取检出的车牌数目
    pBuf  += sizeof(int);
    CarInfo *pCarInfo = (CarInfo*)pBuf;    //获取车牌信息
	#else
	//yuv422->y
	IppiSize srcSize= {m_img->width, m_img->height};
	int desstep[3] = {m_img->width,m_img->width/2,m_img->width/2};
    int srcstep = m_img->width * 2;
	if(m_nWordPos == 0)
	ippiCbYCr422ToYCbCr422_8u_C2P3R((BYTE*)(frame+sizeof(yuv_video_buf)),srcstep,m_pDataY,desstep,srcSize);
	else
	ippiCbYCr422ToYCbCr422_8u_C2P3R((BYTE*)(frame+sizeof(yuv_video_buf)+m_img->width*2*m_nExtentHeight),srcstep,m_pDataY,desstep,srcSize);
	cvSetData(m_pImageDataY,m_pDataY[0],m_pImageDataY->widthStep);

	#ifdef ALGORITHM_YUV
	cvSetData(m_pImageSmallDataY,g_Para.pImage,m_pImageSmallDataY->widthStep);
	cvResize(m_pImageDataY,m_pImageSmallDataY);
	#endif

	/*static int k = 0;
	if(k == 0)
	{
		cvSaveImage("ImageDataY.jpg",m_pImageDataY);
		cvSaveImage("ImageSmallDataY.jpg",m_pImageSmallDataY);
		k++;
		
	}*/
	 //取车牌检测结果
    BYTE* pBuf = frame+sizeof(yuv_video_buf)+ (m_img->width*2)*(m_img->height+m_nExtentHeight);
	int nCarNumResult =  *((int*)pBuf);         //获取检出的车牌数目
    pBuf  += sizeof(int);
    CarInfo *pCarInfo = (CarInfo*)pBuf;    //获取车牌信息
	
	//相机控制
	carnum_context vehicle_result[CARNUMSIZE];
	for(int i =0; i<nCarNumResult; i++)
    {
	   memcpy(vehicle_result[i].carnum,pCarInfo[i].strCarNum,7);
       memcpy(vehicle_result[i].wjcarnum,pCarInfo[i].wj,2);
	   vehicle_result[i].color = pCarInfo[i].color;
       vehicle_result[i].vehicle_type = pCarInfo[i].vehicle_type;
	   vehicle_result[i].position.x = pCarInfo[i].ix;
       vehicle_result[i].position.y = pCarInfo[i].iy;
       vehicle_result[i].position.width = pCarInfo[i].iwidth;
       vehicle_result[i].position.height = pCarInfo[i].iheight;
	   vehicle_result[i].bIsMotorCycle = pCarInfo[i].bIsMotorCycle;
	   vehicle_result[i].iscarnum = pCarInfo[i].iscarnum;
       vehicle_result[i].mean = pCarInfo[i].mean;
       vehicle_result[i].stddev = pCarInfo[i].stddev;
	   vehicle_result[i].VerticalTheta = pCarInfo[i].VerticalTheta;
       vehicle_result[i].HorizontalTheta = pCarInfo[i].HorizontalTheta;
	   vehicle_result[i].carnumrow = pCarInfo[i].carnumrow;//车牌结构
	   vehicle_result[i].smearnum = pCarInfo[i].smearCount;
	   memcpy(vehicle_result[i].smearrect,pCarInfo[i].smear,sizeof(CvRect)*(pCarInfo[i].smearCount));
    }
	
	m_CameraCrt.mv_SetYdatetoIplImage(m_pDataY[0],m_img->width,m_img->height);	
	IplImage *pImage;
	m_CameraCrt.mvGetBGLight(pImage,m_rtCarnumROI,vehicle_result,nCarNumResult,m_LoopParmarer.iNvise_light,m_nDayNight);
	#endif

	#ifdef ALGORITHM_YUV
	int nIndex = 0;
	g_Para.m_nIsDay = m_nDayNight;
	g_Para.nFrameIndex = buf->nSeq;
	g_Para.ts = buf->ts;
	g_Para.nCarNum = nCarNumResult;
	if(g_Para.nCarNum > 0)
	memcpy(g_Para.arrCarInfo,pCarInfo,nCarNumResult*sizeof(CarInfo));

	ChnFrmSignalStatusList::iterator it_b = m_vtsSignalList.begin();
	while(it_b != m_vtsSignalList.end())
	{
		g_Para.arrChnl[nIndex] = *it_b;
		it_b++;
		nIndex++;
	}

    printf("before mvInput\n");
	MvDspSaveOrgSizedImage();
	mvInput();
	 printf("after mvInput\n");

	//输出
	mvOutput();	
	MvDspSaveProcessImage();

	
	//编码
	if(g_Output.nCodingNum > 0)
	{
		AddYUVBuf(g_Output.nCodingNum,g_Output.pCodingIndex);
	}
	
	//删除
	if(g_Output.nDelNum > 0)
	{
		DeleteYUVBuf(g_Output.nDelNum,g_Output.pDelIndex);
	}

	printf("g_Output.nCarinfoNum=%d\n",g_Output.nCarinfoNum);
	//卡口
	if(g_Output.nCarinfoNum > 0)
	{
		std::vector<CarInfo> vResult;
		for(int i = 0;i<g_Output.nCarinfoNum;i++)
		{
			CarInfo result = g_Output.pCarInfo[i];
			vResult.push_back(result);
		}
		AddOutPut(vResult);
	}
	printf("g_Output.nViNum=%d\n",g_Output.nViNum);
	//违章
	if(g_Output.nViNum > 0)
	{
		std::vector<ViolationInfo> vtsResult;
		for(int i = 0;i<g_Output.nViNum;i++)
		{
			ViolationInfo result = g_Output.pViInfo[i];
			vtsResult.push_back(result);
		}
		AddVtsOutPut(vtsResult);
	}
	return;

	#endif
	

	#ifndef ALGORITHM_YUV
	//设置白天晚上模式
    if(m_nDayNight == 1)
    {
        m_ftVlp.SetMode(DAY);
    }
    else if(m_nDayNight == 0)
    {
        m_ftVlp.SetMode(NIGHT);
    }

	vector<CarInfo> vecCarNums;
	if(nCarNumResult > 0)
    {
		for(int i =0; i<nCarNumResult; i++)
        {
            vecCarNums.push_back(pCarInfo[i]);
        }
		LogTrace("RedLightTime.log", "===before DoAction redtime:%d ", buf->uTrafficSignal);
    }

    //线圈测速
    if(((m_nDetectKind&DETECT_LOOP)==DETECT_LOOP))
    {
        SetLoopStatus(buf->uSpeedSignal);
    }
    //雷达测速
    if(((m_nDetectKind&DETECT_RADAR)==DETECT_RADAR))
    {
        printf("==============m_nDetectKind&DETECT_RADAR\n");
        m_ftVlp.SetRadarSpeed(buf->nSeq,buf->ts,buf->uRadarSignal.SpeedSignal,buf->uRadarSignal.FastSpeed);
    }

    //设置当前增益以及模板增益
    m_ftVlp.SetCurrentAndModelGain( (uchar) (buf->uGain) , (uchar)(buf->uMaxGain) );

    //取最大seq
    unsigned int uMaxSeq = 0;
    int nPlateIndex = (m_nPlateIndex+m_nMaxPlateSize -1)%m_nMaxPlateSize;
    yuv_video_buf* bufMaxSeq = (yuv_video_buf*)(m_BufPlate[nPlateIndex].pCurBuffer);
    if(bufMaxSeq!=NULL)
    {
        uMaxSeq = bufMaxSeq->nSeq;
    }
    else
    {
        uMaxSeq = buf->nSeq;
    }
    //取最小值
	rgb_buf rgbMinBuf;
    unsigned int uMinSeq = 0;
    nPlateIndex = (m_nPlateIndex+1);
    yuv_video_buf* bufMinSeq = (yuv_video_buf*)(m_BufPlate[nPlateIndex].pCurBuffer);
    if(bufMinSeq!=NULL)
    {
        uMinSeq = bufMinSeq->nSeq;
		rgbMinBuf = m_BufPlate[nPlateIndex];
    }
    else
    {
        bufMinSeq = (yuv_video_buf*)(m_BufPlate[0].pCurBuffer);
        if(bufMinSeq!=NULL)
        uMinSeq = bufMinSeq->nSeq;

		rgbMinBuf = m_BufPlate[0];
    }

	//LogNormal("nSeq=%d,uMinSeq=%d,uMaxSeq=%d,PlateSize=%d\n",buf->nSeq,uMinSeq,uMaxSeq,m_PlateSize);

	if(g_nFlashControl == 1)
	{
	    vector<mvvideoshutterpar> videoshutterpar;
	    GetFlashStatus(buf->uFlashSignal,buf->ts,videoshutterpar);
	    m_ftVlp.SetVideoShutterPar(videoshutterpar);
	}

    //返回的同步信息（目标运动轨迹）
    vector<ObjSyncData> syncData;
    //统计接口
    StatResultList listStatResult;
    //printf("====void CRoadCarnumDetect::DetectObject() nCarNumResult=%d,buf->nSeq=%d,buf->nFieldSeq=%d,buf->size=%d,m_nPlateReadIndex=%d,uMaxSeq=%d,uMinSeq=%d,m_PlateSize=%d\r\n",nCarNumResult,buf->nSeq,buf->nFieldSeq,buf->size,m_nPlateReadIndex,uMaxSeq,uMinSeq,m_PlateSize);

    //目标检测
    m_img->ID = m_nChannelId;

	struct timeval tv1,tv2;
	if(g_nPrintfTime == 1)
	{
		gettimeofday(&tv1,NULL);
	}

    int bTrafficLightOn = -1;
    if(m_rtVlpRoi.width > 0 && m_rtVlpRoi.height > 0 && (!m_bTestResult) && (m_nVedioFormat != VEDIO_PICLIB))
	{
		
		try{
            
			#ifndef ALGORITHM_YUV_CARNUM
			m_ftVlp.DoAction(m_img, vecCarNums, m_vtsSignalList, buf->ts, buf->nSeq, \
							buf->uTimestamp, m_nPlateReadIndex,buf->nCameraControl,result,syncData, \
							vtsResult,listStatResult,bTrafficLightOn,uMaxSeq,uMinSeq);
			#else
				m_ftVlp.mv_SetYdatetoIplImage(m_pDataY[0],m_img->width,m_img->height);	
				IplImage *pImage;
				m_ftVlp.DoAction(pImage, vecCarNums, m_vtsSignalList, buf->ts, buf->nSeq, \
							buf->uTimestamp, m_nPlateReadIndex,buf->nCameraControl,result,syncData, \
							vtsResult,listStatResult,bTrafficLightOn,uMaxSeq,uMinSeq);
			#endif
		}
		catch(...)
		{
			LogNormal("==Catch ERROR=DoAction=buf->nSeq=%d,buf->ts=%lld=\n", buf->nSeq, buf->ts);
		}
	}

	if(g_nPrintfTime == 1)
	{
		gettimeofday(&tv2,NULL);
		FILE* fp = fopen("time.log","ab+");
		fprintf(fp,"DoAction==nSeq=%lld,t1=%lld,t2=%lld,dt = %lld\n",buf->nSeq,(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
		fclose(fp);
	}

	if(m_nDetectKind&DETECT_LOOP)
	{
		vector<mvloopstatus> nLoopStatus;
		m_ftVlp.GetPhysicLoopStatus(nLoopStatus);

		if(nLoopStatus.size() > 0)//此时需要给中心端报警
		{
			mapLoopStatus loopstatus;

			vector<mvloopstatus>::iterator it_b = nLoopStatus.begin();
			vector<mvloopstatus>::iterator it_e = nLoopStatus.end();
			while(it_b != it_e)
			{
				if(!it_b->status)
				{
					LogNormal("车道%d线圈异常!",it_b->nRoadIndex);

					if(g_nServerType != 2)
					{
						//线圈爆闪切换为视频爆闪
						/*g_nFlashControl = 1;
						CXmlParaUtil xml;
						xml.UpdateSystemSetting("OtherSetting","FlashControl");

						int nDetectKind = m_nDetectKind&(~DETECT_LOOP);
						g_skpDB.UpdateDetectKind(nDetectKind,m_nChannelId);

						g_bEndThread = true;
						LogNormal("线圈爆闪异常,自动切换为视频爆闪\n");
						sleep(3);

						exit(-1);*/
					}
				}
				else
				{
					//LogNormal("车道%d线圈正常!",it_b->nRoadIndex);
				}

				loopstatus.insert(mapLoopStatus::value_type(it_b->nRoadIndex,it_b->status));
				it_b++;
			}

			g_CenterServer.SetLoopStatus(loopstatus);
			if (13 == g_nServerType)
			{
				//g_MyCenterServer.SetLoopStatus(loopstatus); //向Ver1.8中心端报警
				mapChanIdLoopStatus nChanIdLoopStatus;
				nChanIdLoopStatus.insert(mapChanIdLoopStatus::value_type(m_nChannelId,loopstatus));
				g_MyCenterServer.SetChanLoopStatus(nChanIdLoopStatus);
			}
		}
	}
	
	//取消秒图缓冲区
    if((m_nDetectKind&DETECT_VIOLATION))
    {
        //处理后再存储秒图
		BYTE* pMinframe = rgbMinBuf.pCurBuffer;
		if(pMinframe != NULL)
		AddResultFrameList(pMinframe);
    }

	//线圈电警
	if( (m_nDetectKind&DETECT_VTS) &&(m_nDetectKind&DETECT_LOOP) )
	{
		if(vtsResult.size() > 0)
		{
			vtsResult.clear();
		}

		//判断是否发生电警违章行为
		DoLoopVts(buf->nSeq,buf->ts ,buf->uTimestamp,buf->uSpeedSignal,vtsResult);
	}

    //输出目标检测及车牌检测结果
	if(g_ytControlSetting.nMultiPreSet != 1)//多个预置位时不输出卡口结果
	{
		if(result.size()>0)
		{
			AddOutPut(result);
		}
	}

	//违章检测
     if(m_nDetectKind&DETECT_VIOLATION || (g_PicFormatInfo.nSpeedLimit == 1))
     {
         //判断是否出现柴油车
         if(result.size()>0)
         {
             GetVtsResult(result,vtsResult);
         }
     }

     //输出违章检测结果
     if(vtsResult.size()>0)
     {
		 bool bDetect = CheckDetectTime();

		 if(bDetect)
		 AddVtsOutPut(vtsResult);
     }

    //流量统计
    if(listStatResult.size()>0)
    {
        VehicleStatistic(listStatResult,buf->uTimestamp);
    }

	//流量评价数据统计
	if(g_nServerType == 7)
	{
		VtsStatistic(buf);
	}

    ////////////////
	//输出测试结果
	if(m_bTestResult)
	{
		OutPutTestResult(buf->nSeq,buf->ts);
	}
	
	//图库识别
	if (VEDIO_PICLIB == m_nVedioFormat)
	{
		if(vecCarNums.size() > 0)
		{
			AddOutPut(vecCarNums);
		}
	}
	#endif
}

//违章类型判断
void CRoadCarnumDetect::GetVtsResult(std::vector<CarInfo>& vResult,std::vector<ViolationInfo>& vtsResult)
{
	#ifndef ALGORITHM_YUV
    std::vector<CarInfo>::iterator it = vResult.begin();

	if(g_nLoadBasePlateInfo == 1)
	{
		if(g_bLoadXml == 0)
		{
			int nFirstId = g_skpDB.GetId("BASE_PLATE_INFO");
			g_skpDB.GetAllPlate(m_listBASE_PLATE_INFO_1,nFirstId,g_nType1);
			g_skpDB.GetAllPlate(m_listBASE_PLATE_INFO_2,(nFirstId+g_nType1),g_nType2);
			g_skpDB.GetAllPlate(m_listBASE_PLATE_INFO_3,(nFirstId+g_nType1+g_nType2),0);
			g_bLoadXml = 1;
		}
	}

	list<string>::iterator it1 = m_listBASE_PLATE_INFO_1.begin();
	list<string>::iterator it2 = m_listBASE_PLATE_INFO_2.begin();
	list<string>::iterator it3 = m_listBASE_PLATE_INFO_3.begin();

	while(it != vResult.end())
    {
            CarInfo cardNum = *it;

            {
                vector<ChannelRegion>::iterator it_o = m_vtsObjectRegion.begin();
                while( it_o !=  m_vtsObjectRegion.end() )
                {
                    if(it_o->nRoadIndex == cardNum.RoadIndex)
                    {
						std::string strCarNum;
						strCarNum = cardNum.strCarNum;
						//车牌号码转换
						CarNumConvert(strCarNum,cardNum.wj);

                        VTSParaMap::iterator it_p = m_vtsObjectParaMap.find(it_o->nRoadIndex);
                        if(it_p != m_vtsObjectParaMap.end())
                        {
								ViolationInfo info;
								info.evtType = (VIO_EVENT_TYPE)0;

							   if(it_p->second.nForbidType == 4 || g_nLoadBasePlateInfo == 1)//进行柴油车
							   {

								   list<string>::iterator it1 = m_listBASE_PLATE_INFO_1.begin();
								   list<string>::iterator it2 = m_listBASE_PLATE_INFO_2.begin();
								   list<string>::iterator it3 = m_listBASE_PLATE_INFO_3.begin();

								   //printf("=====================strCarNum=%s,it=%s\n",strCarNum.c_str(),it->c_str());
								   it1 = find(m_listBASE_PLATE_INFO_1.begin(),m_listBASE_PLATE_INFO_1.end(),strCarNum);
								   if(it1 != m_listBASE_PLATE_INFO_1.end())
								   {
									  info.evtType = EVT_YELLOW_PLATE;
								   }
								   else
								   {
									   it2 = find(m_listBASE_PLATE_INFO_2.begin(),m_listBASE_PLATE_INFO_2.end(),strCarNum);
									   if(it2 != m_listBASE_PLATE_INFO_2.end())
									   {
										   info.evtType = EVT_CYC_APPEAR;
									   }
									   else
									   {
										   it3 = find(m_listBASE_PLATE_INFO_3.begin(),m_listBASE_PLATE_INFO_3.end(),strCarNum);
										   if(it3 != m_listBASE_PLATE_INFO_3.end())
										   {
											   info.evtType = EVT_CYC_YELLOE_PLATE;
										   }
									   }
								   }

								}

							   if(it_p->second.nForbidType == 6)//非本地车
							   {
									//当输入了本地车牌，且车牌中没有*号
									string::size_type SizeTypeStrLocalPlate = strCarNum.find(g_strLocalPlate.c_str());
									string::size_type SizeTypeStar = strCarNum.find("*");
									if( (g_strLocalPlate.size() > 0)&& (SizeTypeStrLocalPlate == -1) && (SizeTypeStar == -1))
									{
										info.evtType = EVT_NON_LOCAL_PLATE;
									}
							   }
								
								if(it_p->second.nForbidType == 1 && (it->vehicle_type == SMALL))//禁行小车
								{
									unsigned int uTime = GetHmTime();

									if(uTime >= it_p->second.nForbidBeginTime && uTime <= it_p->second.nForbidEndTime)
									{
										info.evtType = (VIO_EVENT_TYPE)DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD;
									}
								}

								if(it_p->second.nForbidType == 2 && (it->vehicle_type == BIG))//禁行大车
								{
									unsigned int uTime = GetHmTime();

									if(uTime >= it_p->second.nForbidBeginTime && uTime <= it_p->second.nForbidEndTime)
									{
										info.evtType = (VIO_EVENT_TYPE)DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD;
									}
								}

								if(it_p->second.nForbidType == 3 &&(cardNum.strCarNum[0] != '*' && cardNum.strCarNum[1] != '*'))//禁行所有车辆
								{
									unsigned int uTime = GetHmTime();

									if(uTime >= it_p->second.nForbidBeginTime && uTime <= it_p->second.nForbidEndTime)
									{
										info.evtType = ELE_NO_PASSING;
									}
								}

							   if(info.evtType > 0)
							   {
									if(g_nVtsPicMode == 2)
									{
										info.nPicCount = 5;
									}
									else
									{
										info.nPicCount = 3;
									}

									unsigned int uMaxSeq = cardNum.uSeq;
									int nPlateIndex = (m_nPlateIndex+m_nMaxPlateSize -1)%m_nMaxPlateSize;
									/*yuv_video_buf* bufMaxSeq = (yuv_video_buf*)(m_BufPlate[nPlateIndex].pCurBuffer);
									if(bufMaxSeq!=NULL)
									{
										uMaxSeq = bufMaxSeq->nSeq;
									}*/

									if(cardNum.nDirection == 0)//前牌
									{
										uMaxSeq = cardNum.uSeq;
									}
									else
									{
										uMaxSeq = cardNum.uSeq + 6;
									}

									info.nChannel = cardNum.RoadIndex;
									info.frameSeqs[info.nPicCount-1] = uMaxSeq;

									int nSeq = 3;

									for(int i = info.nPicCount-1;i>0;i--)
									{
									   info.frameSeqs[i-1] = info.frameSeqs[i] - nSeq;
									}
									unsigned int uTempSeq = 0;
									if(cardNum.nDirection == 0)//前牌
									{
											uTempSeq = info.frameSeqs[0];
											info.frameSeqs[0] = info.frameSeqs[info.nPicCount-1];
											info.frameSeqs[info.nPicCount-1] = uTempSeq;
									}

									info.carInfo = cardNum;
									info.carInfo.nDirection = m_nDetectDirection;//运动方向与车道方向一致

									vtsResult.push_back(info);
							   }
                           }
                        }
						it_o++;
                    }

                }

                //如果是超速

                paraDetectList::iterator it_p = m_roadParamInlist.begin();
                while( it_p !=  m_roadParamInlist.end() )
                {
                    if(it_p->nChannelID == cardNum.RoadIndex)
                    {
                            if(it_p->m_bOnlyOverSped)//是否车辆超速报警
                           {
                                    int dSpeed =   sqrt(it->vx*it->vx+it->vy*it->vy);

									int nOnlyOverSpedMax = it_p->m_nOnlyOverSpedMax;

									if(g_PicFormatInfo.nSpeedLimit == 1)
									{
										nOnlyOverSpedMax += 10;
									}

                                    if( dSpeed > nOnlyOverSpedMax)
                                    {
                                        ViolationInfo info;
                                        if(g_nVtsPicMode == 3)
                                        {
                                            info.nPicCount = 2;
                                        }
                                        else
                                        {
                                            info.nPicCount = 3;
                                        }
                                        info.evtType = EVT_GO_FAST;
                                        info.nChannel = cardNum.RoadIndex;
                                       // info.frameSeqs[0] = cardNum.uSeq;

                                        unsigned int uMaxSeq = cardNum.uSeq;
												if(cardNum.nDirection == 0)//前牌
												{
													uMaxSeq = cardNum.uSeq;
												}
												else
												{
													uMaxSeq = cardNum.uSeq + 6;
												}
		            
												info.frameSeqs[info.nPicCount-1] = uMaxSeq;
												
												int nSeq = 3;
												for(int i = info.nPicCount-1;i>0;i--)
												{
												   info.frameSeqs[i-1] = info.frameSeqs[i] - nSeq;
												}
												unsigned int uTempSeq = 0;
												if(cardNum.nDirection == 0)//前牌
												{
													uTempSeq = info.frameSeqs[0];
													info.frameSeqs[0] = info.frameSeqs[info.nPicCount-1];
													info.frameSeqs[info.nPicCount-1] = uTempSeq;
												}


										if(g_nHasHighLight == 1)//有爆闪灯
										{
											if(cardNum.m_UseShutter)//找到亮图
											{
												cardNum.uSeq =  cardNum.m_useq;
												info.frameSeqs[0] = cardNum.uSeq;

												if(cardNum.m_EstPos)//有车牌
												{
													cardNum.ix = cardNum.m_CarnumPos.x;
													cardNum.iy = cardNum.m_CarnumPos.y;
													cardNum.iwidth = cardNum.m_CarnumPos.width;
													cardNum.iheight = cardNum.m_CarnumPos.height;
												}
											}
										}

                                        info.carInfo = cardNum;

                                        vtsResult.push_back(info);
                                    }
                           }
                    }
                    it_p++;
                }

            it++;
    }
	#endif
}

//非电警违章类型报警
void CRoadCarnumDetect::GetVtsResult(CarInfo& cardNum,RECORD_PLATE& plate)
{			
		#ifndef ALGORITHM_YUV
            //如果是大货车
            if(cardNum.subVehicleType == 2)
            {
                vector<ChannelRegion>::iterator it_o = m_vtsObjectRegion.begin();
                while( it_o !=  m_vtsObjectRegion.end() )
                {
                    if(it_o->nRoadIndex == cardNum.RoadIndex)
                    {
                        VTSParaMap::iterator it_p = m_vtsObjectParaMap.find(it_o->nRoadIndex);
                        if(it_p != m_vtsObjectParaMap.end())
                        {
                            if(it_p->second.nForbidType == 5)//大货禁行
                           {
                                    unsigned int uTime = GetHmTime();

                                    if(uTime >= it_p->second.nForbidBeginTime && uTime <= it_p->second.nForbidEndTime)
                                    {
                                        plate.uViolationType = DETECT_RESULT_BIG_IN_FORBIDDEN_TIME;

										if(g_nServerType != 3)
										{
												plate.uViolationType = 0;//注意，此时卡口图片里不用大货禁行
												ViolationInfo info;
												info.evtType = EVT_FORBID_TRUCK;
												info.nPicCount = 3;
												info.nChannel = cardNum.RoadIndex;
												info.frameSeqs[info.nPicCount-1] = cardNum.uSeq+3;
												int nSeq = 3;

												//for(int i = info.nPicCount-1;i>0;i--)
												{
												//	  info.frameSeqs[i-1] = info.frameSeqs[i] - nSeq;
												}

												unsigned int uMaxSeq = cardNum.uSeq;
												if(cardNum.nDirection == 0)//前牌
												{
													uMaxSeq = cardNum.uSeq;
												}
												else
												{
													uMaxSeq = cardNum.uSeq + 6;
												}
		            
												info.frameSeqs[info.nPicCount-1] = uMaxSeq;
		            
												for(int i = info.nPicCount-1;i>0;i--)
												{
												   info.frameSeqs[i-1] = info.frameSeqs[i] - nSeq;
												}
												unsigned int uTempSeq = 0;
												if(cardNum.nDirection == 0)//前牌
												{
													uTempSeq = info.frameSeqs[0];
													info.frameSeqs[0] = info.frameSeqs[info.nPicCount-1];
													info.frameSeqs[info.nPicCount-1] = uTempSeq;
												}

												info.carInfo = cardNum;
												info.carInfo.nDirection = m_nDetectDirection;//运动方向与车道方向一致
												std::vector<ViolationInfo> vtsResult;
												vtsResult.push_back(info);
												
												if(vtsResult.size()>0)
												{
													AddVtsOutPut(vtsResult);
												}
										}
                                    }
                           }
                        }
                    }
                    it_o++;
                }
            }
		#endif
}

//从检出队列中弹出一帧数据
int CRoadCarnumDetect::PopDetectedFrame(rgb_buf& response)
{
    int nRet = 0;
    //加锁
    pthread_mutex_lock(&m_PlateMutex);

    if(m_PlateSize >= 1) //
    {
        response = m_BufPlate[m_nPlateReadIndex];
        nRet = 1;
    }
    //解锁
    pthread_mutex_unlock(&m_PlateMutex);

    return nRet;
}

//修改检出帧队列长度
void CRoadCarnumDetect::DecreaseDetectedSize( )
{
    //加锁
    pthread_mutex_lock(&m_PlateMutex);

    /////修改可读取的编号
	if(m_PlateSize > 0)
	{
		m_PlateSize--;
		m_nPlateReadIndex = (m_nPlateReadIndex+1)%m_nMaxPlateSize;
	}

    //判断检测缓冲区是否已经满
   if(m_PlateSize >= m_nMaxPlateSize*0.8)
	//if(m_PlateSize >= 3)
    {
		if(m_nCountBufferExceed >= 1)
		{
			LogNormal("检测缓冲区满重新初始化***PlateSize=%d",m_PlateSize);

			m_PlateSize = 1;
			m_nPlateReadIndex = (m_nPlateIndex-1+m_nMaxPlateSize)%m_nMaxPlateSize;
			
			#ifndef ALGORITHM_YUV
			m_ftVlp.Destroy();
			m_ftVlp.Init();
			#endif

			m_nCountBufferExceed = 0;//每隔一次处理一次
		}
		else
		{
			LogNormal("检测缓冲区满***PlateSize=%d",m_PlateSize);

			m_PlateSize = 1;
			m_nPlateReadIndex = (m_nPlateIndex-1+m_nMaxPlateSize)%m_nMaxPlateSize;

			m_nCountBufferExceed = 1;
		}
    }

    //解锁
    pthread_mutex_unlock(&m_PlateMutex);
}

//车牌检测处理
void CRoadCarnumDetect::DealCarnumDetect()
{

	if(g_nDetectMode == 2) //dsp 模式直接return add by wantao
	{
		return;
	}

#ifdef TEMPERATURE_TEST
    UINT32 uTime1 = GetTimeStamp();//开始检测的时间
    UINT32 uTime2 = 0;
    UINT32 uPreTime = uTime1;
    m_detectframe = 0;
	int nPreDetectFrame = 0;
#endif

    int count = 0;
    while(!m_bEndDetect)
    {
        //自动判断白天还是晚上
        if(m_nDetectTime == DETECT_AUTO)
        {
            if(count>1000)
            {
                count = 0;
            }
            if(count==0)
            {
                m_nDayNight = DayOrNight();
				m_nDayNightbyLight = DayOrNight(1);
            }
            count++;
        }
        ///////////////////

        rgb_buf rgbBuf;
        int nSize = PopFrame(rgbBuf);

        if(nSize>0)
        {			
			//车牌处理
			OnDetect(rgbBuf);

            //写车牌检出队列，提供给目标检测
            //加锁
            pthread_mutex_lock(&m_PlateMutex);
		
            /////////交换指针
            BYTE* temp = m_chPlateBuffer[m_nCurIndex];
            m_chPlateBuffer[m_nCurIndex] = m_chFrameBuffer[m_nReadIndex];
            m_chFrameBuffer[m_nReadIndex] = temp;

            if(m_nDeinterlace == 2)
            {
                if(nSize==2)//可拼
                {
                    temp = m_chPlateBuffer[(m_nCurIndex+1)%(m_nMaxPlateSize*2)];
                    m_chPlateBuffer[(m_nCurIndex+1)%(m_nMaxPlateSize*2)] = m_chFrameBuffer[(m_nReadIndex+1)%m_nMaxFrameSize];
                    m_chFrameBuffer[(m_nReadIndex+1)%m_nMaxFrameSize] = temp;
                }
            }
            m_BufPlate[m_nPlateIndex] = rgbBuf;
            /////////////////////////////////////////////////////////
            m_PlateSize++;
			printf("*************m_PlateSize=%d\n",m_PlateSize);
			printf("==33333=DealCarnumDetect=\n");


            if(m_PlateSize>(m_nMaxPlateSize-1))
            {
                m_PlateSize--;
                //此时需要移位(以去除最前面的)
                if(m_nDeinterlace == 2)
                {
                    //移两场
                    BYTE* temp1 = m_chPlateBuffer[((m_nPlateReadIndex+1)*2)%(m_nMaxPlateSize*2)];
                    BYTE* temp2 = m_chPlateBuffer[((m_nPlateReadIndex+1)*2+1)%(m_nMaxPlateSize*2)];
                    rgb_buf  tmp = m_BufPlate[(m_nPlateReadIndex+1)%m_nMaxPlateSize];
                    for(int i= 0; i<(m_nMaxPlateSize-2); i++)
                    {
                        m_chPlateBuffer[((m_nPlateReadIndex+1+i)*2)%(m_nMaxPlateSize*2)] = m_chPlateBuffer[((m_nPlateReadIndex+2+i)*2)%(m_nMaxPlateSize*2)];
                        m_chPlateBuffer[((m_nPlateReadIndex+1+i)*2+1)%(m_nMaxPlateSize*2)] = m_chPlateBuffer[((m_nPlateReadIndex+2+i)*2+1)%(m_nMaxPlateSize*2)];
                        m_BufPlate[(m_nPlateReadIndex+1+i)%m_nMaxPlateSize] = m_BufPlate[(m_nPlateReadIndex+2+i)%m_nMaxPlateSize];
                    }
                    m_chPlateBuffer[m_nCurIndex] = temp1;
                    m_chPlateBuffer[(m_nCurIndex+1)%(m_nMaxPlateSize*2)] = temp2;
                    m_BufPlate[m_nPlateIndex] = tmp;
                }
                else
                {
                    //移一帧
                    temp = m_chPlateBuffer[(m_nPlateReadIndex+1)%m_nMaxPlateSize];
                    rgb_buf  tmp = m_BufPlate[(m_nPlateReadIndex+1)%m_nMaxPlateSize];
                    for(int i= 0; i<(m_nMaxPlateSize-2); i++)
                    {
                        m_chPlateBuffer[(m_nPlateReadIndex+1+i)%m_nMaxPlateSize] = m_chPlateBuffer[(m_nPlateReadIndex+2+i)%m_nMaxPlateSize];
                        m_BufPlate[(m_nPlateReadIndex+1+i)%m_nMaxPlateSize] = m_BufPlate[(m_nPlateReadIndex+2+i)%m_nMaxPlateSize];
                    }
                    m_chPlateBuffer[m_nCurIndex] = temp;
                    m_BufPlate[m_nPlateIndex] = tmp;
                }
            }
            else
            {
                m_nCurIndex +=m_nDeinterlace;
                m_nCurIndex = m_nCurIndex%(m_nMaxPlateSize*m_nDeinterlace);
                m_nPlateIndex++;
                m_nPlateIndex %= m_nMaxPlateSize;
            }
            //加锁
            pthread_mutex_unlock(&m_PlateMutex);

			printf("==44444=DealCarnumDetect=\n");

            DecreaseSize(nSize);
        }
        usleep(1000*1);
#ifdef TEMPERATURE_TEST
        uTime2 = GetTimeStamp();//每分钟统计一次
        if((uTime2%60==0)&&(uPreTime != uTime2))
        {
            std::string strTime = GetTime(uTime2);
            FILE * fp = fopen("avgrateCarnumDetect.txt","a");
            double avgrate = (m_detectframe-nPreDetectFrame*1.0)/(uTime2-uPreTime);
            fprintf(fp,"time = %s,avgrate=%f,detectframe=%d,uTime2-uTime1=%ds\n",strTime.c_str(),avgrate,m_detectframe-nPreDetectFrame,uTime2-uPreTime);//平均帧率
            fclose(fp);
            uPreTime = uTime2;
			nPreDetectFrame = m_detectframe;
        }
#endif
    }

#ifdef TEMPERATURE_TEST
    uTime2 = GetTimeStamp();//结束检测的时间
    FILE * fp = fopen("avgrateCarnumDetect.txt","a");
    double avgrate = (m_detectframe*1.0)/(uTime2-uTime1);
    fprintf(fp,"avgrate=%f,detectframe=%d,uTime2-uTime1=%ds\n",avgrate,m_detectframe,uTime2-uTime1);//平均帧率
    fclose(fp);
#endif

    printf("end of DealCarnumDetect!!!!!\n");

    return;
}

//从jpg map中寻找一个最接近的图片
void CRoadCarnumDetect::GetPicFromJpgMap(UINT32 nSeq,UINT32 nPreSeq,string& strPic)
{
	bool bFind = false;

	UINT32 nSeqCur = 0;
	UINT32 nSeqPre = 0;
	UINT32 nDisSeq = 0;
	//LogNormal("nSeq=%lld,nPreSeq=%lld\n",nSeq,nPreSeq);

	map<int64_t,string>::iterator it_pre;
	map<int64_t,string>::iterator it;


	if(m_nDetectDirection == 0)//前牌（由远及近）
	{
		it = --m_JpgFrameMap.end();
		while((it != m_JpgFrameMap.begin())&&(m_JpgFrameMap.size()>=2))
		{
			nSeqCur = it->first;

			if( (nSeq >= nSeqCur) && ((nPreSeq == 0)||(nSeqCur < nPreSeq)))
			{
				it_pre = it;

				if(it != m_JpgFrameMap.begin())
				{
					it_pre--;
				}

				nSeqPre = it_pre->first;

				if(nPreSeq == nSeqCur)
				{
					strPic = it_pre->second;
				}
				else
				{
					nDisSeq = nSeqCur - nSeqPre;//帧号差

					//前后，帧差12以内取最近一张，否则取离摄像机更远的一张
					//由远及近：帧号更小的一张（时间靠前）
					//由近及远：帧号更大的一张（时间靠后）
					if(nDisSeq <= 8)
					{
						if(nSeq <= (nSeqCur+nSeqPre)*0.5) //择半查找，取更近的那一帧
						{
							strPic = it_pre->second;
						}
						else
						{
							strPic = it->second;
						}
					}
					else
					{
						strPic = it_pre->second;
					}
				}

				//LogNormal("nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n",nSeq,nSeqPre,nSeqCur);

				bFind = true;
				break;
			}
			it--;
		}

		if(!bFind)
		{
			if(m_JpgFrameMap.size() >0)
			{
				it = m_JpgFrameMap.begin();
				strPic = it->second;
				bFind = true;
				LogNormal("begin,it->first=%lld\n",it->first);
			}
		}
	}
	else
	{
		it = m_JpgFrameMap.begin();
		while( (it!=m_JpgFrameMap.end())&&(m_JpgFrameMap.size()>=2))
		{
				nSeqCur = it->first;

				if( (nSeq <= nSeqCur) && ((nPreSeq == 0) ||(nSeqCur > nPreSeq)))//尾牌
				{
					it_pre = it;

					if(it != m_JpgFrameMap.begin())
					{
						it_pre--;
					}

					nSeqPre = it_pre->first;

					if(nPreSeq == nSeqPre)
					{
						strPic = it->second;
					}
					else
					{
						nDisSeq = nSeqCur - nSeqPre;//帧号差

						//前后，帧差12以内取最近一张，否则取离摄像机更远的一张
						//由远及近：帧号更小的一张（时间靠前）
						//由近及远：帧号更大的一张（时间靠后）
						if(nDisSeq <= 12)
						{
							if(nSeq <= (nSeqCur+nSeqPre)*0.5) //择半查找，取更近的那一帧
							{
								strPic = it_pre->second;
							}
							else
							{
								strPic = it->second;
							}
						}
						else
						{
							//尾牌（由近及远）
							strPic = it->second;
						}
					}

					//LogNormal("nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n",nSeq,nSeqPre,nSeqCur);

					bFind = true;
					break;
				}

			it++;
		}

		if(!bFind)
		{
			if(m_JpgFrameMap.size() >0)
			{
				it = --m_JpgFrameMap.end();
				strPic = it->second;
				bFind = true;
				LogNormal("end,it->first=%lld\n",it->first);
			}
		}
	}
}

//从map中寻找一个最接近的序号
int CRoadCarnumDetect::GetSeqIndexFromMap(UINT32 nSeq,UINT32 nPreSeq,SeqIndexMap& mapSeqIndex)
{
    int nIndex = 0;
    bool bFind = false;

   SeqIndexMap::iterator it = mapSeqIndex.begin();
   SeqIndexMap::iterator it_pre;

   while( (it!=mapSeqIndex.end())&&(mapSeqIndex.size()>=2))
   {
        UINT32 nSeqCur = it->first;

        if(nSeq <= nSeqCur)
        {
            it_pre = it;

            if(it != mapSeqIndex.begin())
            {
                it_pre--;
            }

            UINT32 nSeqPre = it_pre->first;
            if(nSeq <= (nSeqCur+nSeqPre)*0.5)
            {
                nIndex = it_pre->second;
            }
            else
            {
                nIndex = it->second;
            }

            if(nPreSeq == nSeqCur)
            {
			   //LogNormal("nPreSeq=nSeqCur=%lld,nSeqPre=%lld",nSeqCur,nSeqPre);
               nIndex = it_pre->second;
            }
		    else if(nPreSeq == nSeqPre)
            {
				//LogNormal("nPreSeq=nSeqPre=%lld,nSeqCur=%lld",nSeqPre,nSeqCur);
                nIndex = it->second;
            }

            bFind = true;
            break;
        }
        it++;
   }

    if(!bFind)
    {
        if(mapSeqIndex.size() >0)
        {
			//LogNormal("bFind=false");
            it = mapSeqIndex.begin();
            nIndex = it->second;

#ifdef OUT_STATE_LOG
			LogTrace("OutState.txt","--bFind=false! nSeq:%d \n",nSeq);
#endif
        }
    }

   return nIndex;
}

//获取秒图
bool CRoadCarnumDetect::GetImageFromResultFrameList(UINT32 nSeq,UINT32 nPreSeq,yuv_video_buf& bufResult)
{
        bool bFind = false;

        std::list<std::string>::iterator it = m_ResultFrameList.begin();
        std::list<std::string>::iterator it_pre;
        //////////
        while( (it!=m_ResultFrameList.end())&&(m_ResultFrameList.size()>=2))
        {
                yuv_video_buf* buf = (yuv_video_buf*)it->c_str();

                if(nPreSeq <= buf->nSeq)
                {
                    if(nSeq <= buf->nSeq)
                    {
                        it_pre = it;

                        if(it != m_ResultFrameList.begin())
                        {
                            it_pre--;
                        }

                        yuv_video_buf* buf_pre = (yuv_video_buf*)it_pre->c_str();
                        if(nSeq <= (buf_pre->nSeq+buf->nSeq)*0.5)
                        {
							#ifndef ALGORITHM_YUV_CARNUM
                            memcpy(m_imgComposeResult->imageData,(char*)(it_pre->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->imageSize);
							#else
							cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
							if(m_nWordPos == 0)
							{
								ConvertYUVtoRGBImage((BYTE*)(it_pre->c_str()+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
							}
							else
							{
								ConvertYUVtoRGBImage((BYTE*)(it_pre->c_str()+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
							}
							#endif
                            bufResult = *buf_pre;
                        }
                        else
                        {
							#ifndef ALGORITHM_YUV_CARNUM
                            memcpy(m_imgComposeResult->imageData,(char*)(it->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->imageSize);
							#else
							cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
							if(m_nWordPos == 0)
							{
								ConvertYUVtoRGBImage((BYTE*)(it->c_str()+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
							}
							else
							{
								ConvertYUVtoRGBImage((BYTE*)(it->c_str()+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
							}
							#endif
                            bufResult = *buf;
                        }

                        if(nPreSeq == buf_pre->nSeq)
                        {
							#ifndef ALGORITHM_YUV_CARNUM
                            memcpy(m_imgComposeResult->imageData,(char*)(it->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->imageSize);
							#else
							cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
							if(m_nWordPos == 0)
							{
								ConvertYUVtoRGBImage((BYTE*)(it->c_str()+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
							}
							else
							{
								ConvertYUVtoRGBImage((BYTE*)(it->c_str()+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
							}
							#endif
                            bufResult = *buf;
                        }

                        bFind = true;

#ifdef OUT_STATE_LOG
						LogTrace("OutState.txt","==111=Find GetImageFromResultFrameList! nSeq:%d, nOutSeq:%d \n", nSeq, buf_pre->nSeq);

#endif
                        break;
                    }
                }
                it++;
        }

        if(!bFind)
        {
            if(m_ResultFrameList.size() >0)
            {
                it = --m_ResultFrameList.end();
                yuv_video_buf* buf = (yuv_video_buf*)it->c_str();
			#ifdef OUT_STATE_LOG
				LogTrace("OutState.txt","GetImageFromResultFrameList! nSeq:%d, nOutSeq:%d \n", nSeq, buf->nSeq);
			#endif
				#ifndef ALGORITHM_YUV_CARNUM
                memcpy(m_imgComposeResult->imageData,(char*)(it->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->imageSize);
				#else
				cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
				if(m_nWordPos == 0)
				{
					ConvertYUVtoRGBImage((BYTE*)(it->c_str()+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
				}
				else
				{
					ConvertYUVtoRGBImage((BYTE*)(it->c_str()+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
				}
				#endif
                bufResult = *buf;
            }
        }

        return bFind;
}



//从jpg缓冲区获取闯红灯结果图像
bool CRoadCarnumDetect::GetVtsImageByJpgSeq(UINT32* frameSeqs, PLATEPOSITION* pTimeStamp, int nPicCount, \
										   UINT32 SignalframeSeq, PLATEPOSITION* pSignalTimeStamp,UINT32 uViolationType)
{
	//LogNormal("GetVtsImageByJpgSeq frameSeqs:%d,%d,%d", frameSeqs[0], frameSeqs[1], frameSeqs[2]);
	//LogNormal("--decode--222----\n");
	printf("=in=GetVtsImageByJpgSeq==\n");

	if(m_JpgFrameMap.size() < 1)
	{
		printf("=err==m_JpgFrameMap.size()=%d==\n", m_JpgFrameMap.size());
		return false;
	}

	yuv_video_buf* buf = NULL;

	string strPic1,strPic2,strPic3;

	//找违章图
	bool bGetVtsPic = GetVtsPic(frameSeqs, pTimeStamp, nPicCount, buf, strPic1,strPic2,strPic3);

	if((g_nDoSynProcess == 1) &&bGetVtsPic)//前后牌比对
	{
		//if(!m_pMachPlate->bUse)
		{
			this->ReInitMatchPlate(m_pMachPlate);//数据清空

			//添加图片到缓存队列
			m_pMachPlate->dspRecord.uChannelID = m_nChannelId;
			m_pMachPlate->dspRecord.uSeqID = frameSeqs[0];	

			if(strPic1.size() > 0)
			{
				AddJpgToMatch(strPic1, 0, m_pMachPlate);
			}

			if(strPic2.size() > 0)
			{
				AddJpgToMatch(strPic2, 1, m_pMachPlate);
			}

			if(strPic3.size() > 0)
			{
				AddJpgToMatch(strPic3, 2, m_pMachPlate);
			}

#ifdef MATCH_LIU_YANG_DEBUG
			LogNormal("m_pMachPlate vts %d,%d,%d,%d ",\
				m_pMachPlate->nSizeArray[0], m_pMachPlate->nSizeArray[1], \
				m_pMachPlate->nSizeArray[2], m_pMachPlate->nSizeArray[3]);
#endif
			//m_pMachPlate->bUse = true;
		}
		//else
		//{
		//	LogNormal("11m_pMachPlate in Use uKey:%lld,%lld,%lld !", \
		//		m_pMachPlate->uKeyArray[0], m_pMachPlate->uKeyArray[1], m_pMachPlate->uKeyArray[2]);
		//}
	}

	UINT16 uSignalColor = 0;
	unsigned long uSignalSeq = 0;
	string strPic;
	bool bExchange = false;
	for(int i = 0; i<nPicCount; i++)
	{
		if(i == 0)
		{
			strPic = strPic1;
		}
		else if(i == 1)
		{
			strPic = strPic2;
		}
		else if(i == 2)
		{
			strPic = strPic3;
		}

		if ((g_nServerType == 23 || g_nServerType == 26) && DETECT_RESULT_EVENT_GO_FAST == uViolationType)
		{
			if(pTimeStamp->ts > (pTimeStamp+1)->ts)
			{
				if (i ==0)
				{
					yuv_video_buf* buf1 = NULL;
					yuv_video_buf* buf2 = NULL;
					buf1 = (yuv_video_buf*)strPic1.c_str();
					buf2 = (yuv_video_buf*)strPic2.c_str();

					int64_t ts;
					UINT32 uTimestamp;
					int nRedColor;
					UINT32 uSeq;

					ts = buf1->ts;
					uTimestamp = buf1->uTimestamp;
					nRedColor = buf1->uTrafficSignal;
					uSeq = buf1->nSeq;

					pTimeStamp->ts = buf2->ts;
					pTimeStamp->uTimestamp = buf2->uTimestamp;
					pTimeStamp->nRedColor = buf2->uTrafficSignal;
					pTimeStamp->uSeq = buf2->nSeq;

					(pTimeStamp+1)->ts = ts;
					(pTimeStamp+1)->uTimestamp = uTimestamp;
					(pTimeStamp+1)->nRedColor = nRedColor;
					(pTimeStamp+1)->uSeq = uSeq;

					strPic = strPic2;
					bExchange = true;
				}
			}
			if (i == 1 && bExchange == true)
			{
				strPic = strPic1;
			}
		}
		if(strPic.size()>0)
		{
			//取得图片红灯信号值
			yuv_video_buf* buf = (yuv_video_buf*)(strPic.c_str());
			uSignalColor = buf->uTrafficSignal;
			uSignalSeq = buf->nSeq;

			//需要解码jpg图像
			CxImage image;
			image.Decode((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)), strPic.size() - sizeof(yuv_video_buf), 3); //解码

			printf("image.GetSize=%d,imageSize=%d,w=%d,h=%d\n",\
				image.GetSize(),m_imgComposeResult->imageSize,\
				m_imgComposeResult->width, m_imgComposeResult->height);

			if(image.IsValid()&&image.GetSize()>0)
			{
				memcpy(m_imgComposeResult->imageData,image.GetBits(),image.GetSrcSize());
			}
		}

		CvRect rect;
		CvRect rt;

		int height = m_imgComposeResult->height;
		if(m_nWordOnPic == 1)
		{
			height -= m_nExtentHeight;
		}

		if(m_imgComposeSnap->width == m_imgComposeResult->width)
		{
			rect.x = 0;
			rect.y = i*height;
		}
		else if(m_imgComposeSnap->width > 2*m_imgComposeResult->width)
		{
			rect.x = i*m_imgComposeResult->width;
			rect.y = 0;
		}
		else
		{
			if(g_nServerType == 15 || g_nServerType == 21 || ((g_nVtsPicMode == 1) &&(1 != g_PicFormatInfo.nSmallViolationPic) ))//小图在右下角
			{
				rect.x = (i%2)*m_imgComposeResult->width;
				rect.y = (i/2)*height;
			}
			else//小图在左上角
			{
				rect.x = ((i+1)%2)*m_imgComposeResult->width;
				rect.y = ((i+1)/2)*height;
			}
		}
		rect.width = m_imgComposeResult->width;
		rect.height = m_imgComposeResult->height - m_nExtentHeight;

		if(m_nWordPos == 1)
		{
			if(m_nWordOnPic == 0)
			{
				rect.y += m_nExtentHeight;
			}
		}

		rt.x = 0;
		rt.y = 0;
		rt.width = m_imgComposeResult->width;
		rt.height = m_imgComposeResult->height - m_nExtentHeight;

		if(g_nServerType == 14) //北京公交项目中心端
		{
			rect.x = (i%2)*m_imgComposeResult->width;
			rect.y = 120+(i/2)*(m_imgSnap->height - m_nExtentHeight);
			rect.height = m_imgSnap->height - m_nExtentHeight;
		}
		else if((g_nServerType == 3) && (g_nVtsPicMode == 3))//旅行时间图片格式
		{
			rect.x = i*m_imgComposeResult->width;
			rect.y = m_imgComposeSnap->height-m_imgComposeResult->height+m_nExtentHeight;;
			rect.width = m_imgComposeResult->width;
			rect.height = m_imgComposeResult->height - m_nExtentHeight;
		}
		else if(g_nServerType == 24)
		{
			rect.x = (i%2)*(m_imgSnap->width);//0:0, 1:w, 2:0
			rect.y = (i/2)*(height);//0:0, 1:0, 2:h

			rect.height = m_imgSnap->height - m_nExtentHeight;
			//LogNormal("rect:%d,%d,%d,%d", rect.x, rect.y, rect.width, rect.height);
		}	
		else
		{
			//
		}

		//LogNormal("11 rect:%d,%d,%d,%d", rect.x, rect.y, rect.width, rect.height);

		if(g_nVtsPicMode == 5)
		{
			if(g_nServerType != 23 && g_nServerType != 29)
			{
				rect.y = m_imgComposeSnap->height - m_imgComposeResult->height+ m_nExtentHeight;
			}
		}
		
		//红绿灯增强
		#ifdef REDADJUST
		RedLightAdjust(m_imgComposeResult,false,uSignalColor,uSignalSeq);
		#endif
		
		if(((g_nServerType == 13) && (1 == g_nVtsPicMode) && (DETECT_RESULT_EVENT_GO_FAST == uViolationType) && (i == 0)))//上海交警三期第一张图填充白色(点超速)
		{
			rect.height += m_nExtentHeight;
			cvSetImageROI(m_imgComposeSnap,rect);
			cvSet(m_imgComposeSnap, cvScalar( 255,255,255 ));
			cvResetImageROI(m_imgComposeSnap);
			rect.height -= m_nExtentHeight;
		}
		else if(((g_nServerType == 13) && (1 == g_nVtsPicMode) && ((DETECT_RESULT_YELLOW_CAR == uViolationType || DETECT_RESULT_YELLOW_CRC == uViolationType)) && (i == 0)))//上海交警三期第一张图填充白色(黄标车)
		{
			cvSetImageROI(m_imgComposeSnap,rect);
			cvSet(m_imgComposeSnap, cvScalar( 255,255,255 ));
			cvResetImageROI(m_imgComposeSnap);
		}
		else
		{

			//武汉格式(1x2加小图)
			if(1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode)
			{
					rt.x = 0;
					rt.y = 0;
					rt.width = m_imgComposeResult->width;
					rt.height = m_imgComposeResult->height-m_nExtentHeight;

					if(DETECT_RESULT_EVENT_GO_FAST == uViolationType)
					{
						if( i <= 1)
						{
							rect.x = 0;
						}
						else if( i == 2)
						{
							rect.x = 400;
						}	
					}
					else
					{
						if(i == 0)
						{
							rect.x = 0;
						}
						else
						{
							rect.x = 400;
						}	
					}

					rect.y = m_nExtentHeight+20;//80个像素
					rect.width = 400;
					rect.height = 300;
			}


			//LogNormal("22 rect:%d,%d,%d,%d", rect.x, rect.y, rect.width, rect.height);
			cvSetImageROI(m_imgComposeResult,rt);
			cvSetImageROI(m_imgComposeSnap,rect);
			cvResize(m_imgComposeResult, m_imgComposeSnap);
			cvResetImageROI(m_imgComposeSnap);
			cvResetImageROI(m_imgComposeResult);

			//LogNormal("rt:%d,%d,%d,%d", rt.x, rt.y, rt.width, rt.height);
			//LogNormal("m_imgComposeResult w:%d,h:%d", m_imgComposeResult->width, m_imgComposeResult->height);
			//LogNormal("m_imgComposeSnap w:%d,h:%d", m_imgComposeSnap->width, m_imgComposeSnap->height);
		}
		
		int picIndex = 0;
		if (DETECT_RESULT_RETROGRADE_MOTION == uViolationType)//逆行
		{
			if(g_nVtsPicMode != 3)
			{
				picIndex = 2;
			}
		}

		//武汉格式(1x2加小图)
		if(1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode)
		{
			if(DETECT_RESULT_EVENT_GO_FAST == uViolationType)
			{
				picIndex = 2;
			}
		}

		if (i == picIndex)
		{
			//违章图片加小图 并且 2x2叠加图
			if (1 == g_PicFormatInfo.nSmallViolationPic && (1 == g_nVtsPicMode || 2 == g_nVtsPicMode ||3 == g_nVtsPicMode || 5 == g_nVtsPicMode  ))
			{
				//cvSetImageROI(m_imgComposeSnap,rect);
				//cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));
				//cvResetImageROI(m_imgComposeSnap);

				if(g_nDetectMode == 2)//1拖N从卡口图片中抠图
				{
					if(frameSeqs[picIndex] != frameSeqs[3])//如果要扣图的帧号和第一张图不同，则从卡口图片中扣图
					{
						//LogNormal("frameSeq=%lld,frameSeqs[3]=%lld\n",frameSeqs[picIndex],frameSeqs[3]);
						UINT32 uSeq = frameSeqs[3];
						map<int64_t,string>::iterator it_map = m_JpgFrameMap.find(uSeq);
						//如果没有在缓冲区中找到
						if(it_map!=m_JpgFrameMap.end())
						{
							strPic = it_map->second;
						}
						else
						{
							GetPicVtsFromJpgMap(uSeq,0,strPic);
							//return false;
						}
						if(strPic.size()>0)
						{
							if(g_nDoSynProcess == 1)//前后牌比对
							{
								//添加图片到缓存队列
								//if(m_pMachPlate->bUse)
								{								
									AddJpgToMatch(strPic, 3, m_pMachPlate);

									//LogNormal("m_pMachPlate Vts 3 strPic:%d ", strPic.size());
								}
							}						

							buf = (yuv_video_buf*)strPic.c_str();
							(pTimeStamp+3)->ts = buf->ts;
							(pTimeStamp+3)->uTimestamp = buf->uTimestamp;

							//需要解码jpg图像
							CxImage image;
							image.Decode((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)), strPic.size() - sizeof(yuv_video_buf), 3); //解码

							if(image.IsValid()&&image.GetSize()>0)
							{
								memcpy(m_imgComposeResult->imageData,image.GetBits(),image.GetSrcSize());
							}
						}
					}
					else
					{
						(pTimeStamp+3)->ts = pTimeStamp->ts;
						(pTimeStamp+3)->uTimestamp = pTimeStamp->uTimestamp;

						if(g_nDoSynProcess == 1)//前后牌比对
						{
							m_pMachPlate->uKeyArray[0] = m_pMachPlate->uKeyArray[3];
							m_pMachPlate->bKeyStateArray[0] = m_pMachPlate->bKeyStateArray[3];
#ifdef DEBUG_LIUYANG
							LogNormal("m_pMachPlate Vts 3 same uKey:%lld useq:%lld ", \
								m_pMachPlate->uKeyArray[0], frameSeqs[3]);
#endif
						}
					}
				}

				//存小图
				RECORD_PLATE plate_r;
				plate_r.uPosLeft = pSignalTimeStamp->x;
				plate_r.uPosTop = pSignalTimeStamp->y;
				plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
				plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;
				plate_r.uType = pSignalTimeStamp->nType;
				if(!pSignalTimeStamp->IsCarnum)
				{
					plate_r.chText[0] = '*';
				}

			//	LogNormal("cvRectangle l=%d,t=%d,r=%d,b=%d\n",plate_r.uPosLeft,plate_r.uPosTop,plate_r.uPosRight,plate_r.uPosBottom);
				
				CvRect srcRt,dstRt;
				dstRt = rect;

				if (g_nServerType == 15)
				{
					dstRt.x = m_imgComposeResult->width;
					dstRt.y = rt.height;
				}
				else
				{
					dstRt.x = 0;
					dstRt.y = 0;
				}
				
				if(m_nWordPos == 1)
				{
					if(m_nWordOnPic == 0)
					{
						dstRt.y = m_nExtentHeight;
					}
				}
				
				if(g_nVtsPicMode == 3)
				{
					if (g_nServerType == 3)
					{
						dstRt.x = 2*m_imgComposeResult->width;
						dstRt.y = m_imgComposeSnap->height-m_imgComposeResult->height+m_nExtentHeight;
					}
					else
					{
						dstRt.x = 0;
						dstRt.y = 380;
						dstRt.width = 800;
						dstRt.height = 800;
					}
				}
				

				if(g_nVtsPicMode == 5)
				{
					if (g_nServerType == 23 || g_nServerType == 26)
					{
						dstRt.x = m_imgSnap->width*3;
					}
					else
					{
						dstRt.x = m_imgSnap->width*3;
						dstRt.y = m_imgComposeSnap->height - m_imgComposeResult->height+m_nExtentHeight;
						dstRt.height = m_imgComposeResult->height-m_nExtentHeight;
					}
				}

				if(g_nServerType == 21)//深圳交警格式
				{
					dstRt.x = m_imgComposeResult->width;
					dstRt.y = rt.height+m_nExtentHeight;
					dstRt.height += m_nExtentHeight;
				}


				if(g_nServerType == 14)
				{
					dstRt.x = m_imgSnap->width;
					dstRt.y = m_imgSnap->height-m_nExtentHeight+120;
					dstRt.width = m_imgSnap->width;
					dstRt.height = m_imgSnap->height-m_nExtentHeight;

					//取车身位置
				   srcRt = GetCarPos(plate_r,2);

				}
				else
				{
					//取车身位置
					if(pSignalTimeStamp->rtCarPos.width > 0)
					{
						srcRt = pSignalTimeStamp->rtCarPos;
					}
					else
					{
						srcRt = GetCarPos(plate_r,2);
					}
				}		
				
				if(g_nServerType == 13 && uViolationType == DETECT_RESULT_EVENT_GO_FAST)
				{
					dstRt.height -= m_nExtentHeight;
				}
				
				//非武汉格式
				if(!(1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode))
				{
					//按目标区域比例裁剪
					srcRt.width = srcRt.height * dstRt.width/dstRt.height;

					if(srcRt.x + srcRt.width >= dstRt.width)
					{
						srcRt.x = dstRt.width - srcRt.width-1;
					}
				}

				/*if(g_nServerType != 13 && uViolationType == DETECT_RESULT_EVENT_GO_FAST)
				{
					srcRt.x = 0;
					srcRt.y = 0;
					srcRt.width = m_imgComposeResult->width;
					srcRt.height = m_imgComposeResult->height - m_nExtentHeight;
				}*/

				if(g_nServerType == 24)
				{
					//srcRt.x += (int)((float)srcRt.width/6.0);
					//srcRt.y += (int)((float)srcRt.height/6.0);
					//srcRt.width = (int)((float)srcRt.width*2.0/3.0);
					//srcRt.height = (int)((float)srcRt.height*2.0/3.0);
					int nHeightC1 = 20;//参考固定高, eg:20(车牌高度常数) //20
					int nWidthExtC1 = 360;//参考外扩区域宽, eg:800 //120
					int nHeightExtC2 = 180;//参考外扩区域高, eg:600 //60
					int nHeightExtC3 = 90;//参考外扩区域,抠图区域高, eg:400 //30
					//取车牌Rect
					CvRect rtSrc;
					rtSrc.x = plate_r.uPosLeft;
					rtSrc.y = plate_r.uPosTop;
					rtSrc.width = plate_r.uPosRight - plate_r.uPosLeft;
					rtSrc.height = plate_r.uPosBottom - plate_r.uPosTop;

					//取图片Rect
					CvRect rtSrcImg;
					rtSrcImg.x = 0;
					rtSrcImg.y = 0;
					rtSrcImg.width = m_imgSnap->width;
					rtSrcImg.height = m_imgSnap->height;

					bool bGetCarPosRect = GetCarPosRect(nHeightC1,nWidthExtC1,nHeightExtC2,nHeightExtC3,rtSrc,rtSrcImg,srcRt);

					dstRt.x = m_imgSnap->width; 
					dstRt.y = m_imgSnap->height; 
					dstRt.width = m_imgComposeResult->width; 
					dstRt.height = m_imgComposeResult->height-m_nExtentHeight; 
					//dstRt.x = m_imgSnap->width + (m_imgSnap->width - srcRt.width)/2; 
					//dstRt.x = m_imgSnap->width;
					//dstRt.y = m_imgSnap->height;
					//dstRt.height = m_imgComposeResult->height-m_nExtentHeight;
					/*dstRt.x = m_imgSnap->width + (m_imgSnap->width - srcRt.width)/2;
					dstRt.y = m_imgSnap->height + (m_imgSnap->height - srcRt.height)/2;
					dstRt.width = srcRt.width;
					dstRt.height = srcRt.height;*/
					//LogNormal("dstRt[%d,%d,%d,%d]", dstRt.x, dstRt.y, dstRt.width, dstRt.height);
				}
				
				//LogNormal("22 srcRt:%d,%d,%d,%d", srcRt.x, srcRt.y, srcRt.width, srcRt.height);
				//LogNormal("22 dstRt:%d,%d,%d,%d", dstRt.x, dstRt.y, dstRt.width, dstRt.height);

				cvSetImageROI(m_imgComposeResult,srcRt);
				cvSetImageROI(m_imgComposeSnap,dstRt);
				cvResize(m_imgComposeResult, m_imgComposeSnap);
				cvResetImageROI(m_imgComposeSnap);
				cvResetImageROI(m_imgComposeResult);
			}
		}
	}

	//LogNormal("--yu--22222 \n");

	//获取红灯时间
	if( SignalframeSeq > 0 )
	{
		if(m_JpgFrameMap.size() > 1)
		{
			map<int64_t,string>::iterator it_b = m_JpgFrameMap.begin();
			map<int64_t,string>::iterator it_e = --m_JpgFrameMap.end();
			UINT32 nMinSeq = it_b->first; //最小帧号
			UINT32 nMaxSeq = it_e->first;//最大帧号	

			UINT32 uSeq = SignalframeSeq;
			if(uSeq > nMaxSeq)
			{
				uSeq = nMaxSeq;
			}

			if(uSeq < nMinSeq)
			{
				//LogNormal("红灯时间太晚，取最小的帧号\n");
				uSeq = nMinSeq;
			}
			//根据帧号找到对应的图像序号
			int index = 0;
			map<int64_t,string>::iterator it_map = m_JpgFrameMap.find(uSeq);

			if(it_map!=m_JpgFrameMap.end())
			{
				strPic = it_map->second;
				buf = (yuv_video_buf*)(strPic.c_str());

				if(buf)
				{
					pSignalTimeStamp->ts = buf->ts;
					pSignalTimeStamp->uTimestamp = buf->uTimestamp;
					pSignalTimeStamp->nRedColor = buf->uTrafficSignal;
				}
			}
			else
			{
				//LogNormal("未找到红灯时间\n");
				pSignalTimeStamp->ts = 0;
				pSignalTimeStamp->uTimestamp = 0;
			}
		}//End of if(m_JpgFrameMap.size() > 1)
	}

#ifdef MATCH_LIU_YANG_DEBUG
	//LogNormal("uSeqArray:%d,%d,%d,%d", \
	//	m_pMachPlate->uSeqArray[0],m_pMachPlate->uSeqArray[1],m_pMachPlate->uSeqArray[2],m_pMachPlate->uSeqArray[3]);
	LogNormal("frameSeqs:%d,%d,%d,%d", \
		frameSeqs[0],frameSeqs[1],frameSeqs[2],frameSeqs[3]);
#endif

	printf("=out=GetVtsImageByJpgSeq==\n");

	return true;
}


//获取违章结果图像
bool CRoadCarnumDetect::GetVtsImageByIndex( \
	UINT32* frameSeqs, PLATEPOSITION* pTimeStamp, int nPicCount, \
	UINT32 SignalframeSeq, PLATEPOSITION* pSignalTimeStamp,UINT32 uViolationType)
{
	cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));

	if( g_nDetectMode == 2)
	{
		printf("=frameSeqs[%d, %d, %d]===uTimestamp=%lld==nPicCount=%d=\
			   SignalframeSeq=%lld, pSignalTimeStamp=%lld==uUpperFrm=%d=\
			   uLowerFrm=%d=bCheckSeq=%d=uViolationType=%d==nDirection=%d\n", \
			frameSeqs[0], frameSeqs[1], frameSeqs[2], pTimeStamp->uTimestamp, nPicCount, \
			SignalframeSeq, pSignalTimeStamp->uTimestamp);

		bool bGet = GetVtsImageByJpgSeq(frameSeqs,pTimeStamp,nPicCount,SignalframeSeq,pSignalTimeStamp,uViolationType);
		return bGet;
	}
    //加锁
    pthread_mutex_lock(&m_PlateMutex);
    //暂存seq->index对应关系
    SeqIndexMap mapSeqIndex;
    for(int j = 0; j<m_nMaxPlateSize; j++)
    {
        yuv_video_buf* buf1 = (yuv_video_buf*)(m_BufPlate[j].pCurBuffer);

        if(buf1!=NULL)
        {
            mapSeqIndex.insert(SeqIndexMap::value_type(buf1->nSeq,j));
        }
    }

    SeqIndexMap::iterator it_b = mapSeqIndex.begin();
    SeqIndexMap::iterator it_e = --mapSeqIndex.end();
    UINT32 nMinSeq = it_b->first;
    UINT32 nMaxSeq = it_e->first;


    #ifdef savevtstxt
        FILE* fp =fopen("vtsimageIndex.txt","a");
        if(nPicCount == 3)
        fprintf(fp,"time =%s,frameSeq0=%lld,frameSeq1=%lld,frameSeq2=%lld,nMinSeq=%lld,nMaxSeq=%lld\n",GetTime(pTimeStamp->uTimestamp).c_str(),frameSeqs[0],frameSeqs[1],frameSeqs[2],nMinSeq,nMaxSeq);
        else if(nPicCount == 5)
        fprintf(fp,"time =%s,frameSeq0=%lld,frameSeq1=%lld,frameSeq2=%lld,,frameSeq3=%lld,frameSeq4=%lld,nMinSeq=%lld,nMaxSeq=%lld\n",GetTime(pTimeStamp->uTimestamp).c_str(),frameSeqs[0],frameSeqs[1],frameSeqs[2],frameSeqs[3],frameSeqs[4],nMinSeq,nMaxSeq);
    #endif

    UINT32 nPreSeq = 0;
    for(int i = 0; i<nPicCount; i++)
    {
		if(g_PicFormatInfo.nSpaceRegion == 1 && i == 2)
		{
			break;
		}
		
        UINT32 uSeq = frameSeqs[i];//frameSeqs[nPicCount-1-i];
		
		//LogNormal("%d,uSeq=%lld\n",i,uSeq);
        if(uSeq > nMaxSeq)
        {
			//LogNormal("最大值\n");
            uSeq = nMaxSeq;
        }

         bool bFind = true;
        //根据帧号找到对应的图像序号
        SeqIndexMap::iterator it_map;
        int index = 0;
        it_map = mapSeqIndex.find(uSeq);

        bool bFindImageFromResultFrameList = false;
        //如果没有在缓冲区中找到
        if(it_map!=mapSeqIndex.end())
        {
            index = it_map->second;
        }
        else
        {
			 if(uSeq < nMinSeq)
            {
                //去秒图中寻找
               bFindImageFromResultFrameList = true;
			   //LogTrace("OutState.txt", "===bFindImageFromResultFrameList=true==uSeq=%d !", uSeq);
            }
            else
            {
                index = GetSeqIndexFromMap(uSeq,nPreSeq,mapSeqIndex);
            }
        }


        if(!bFindImageFromResultFrameList)
        {
            yuv_video_buf* buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);
            (pTimeStamp+i)->ts = buf->ts;
            (pTimeStamp+i)->uTimestamp = buf->uTimestamp;
			(pTimeStamp+i)->uSeq = buf->nSeq;

            #ifdef savevtstxt
                fprintf(fp,"=======%d,find,buf->nSeq=%lld\n",i,buf->nSeq);
            #endif
			
			#ifndef ALGORITHM_YUV_CARNUM
            memcpy(m_imgComposeResult->imageData,m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf),m_imgComposeResult->imageSize);
			#else
			cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
			if(m_nWordPos == 0)
			{
				ConvertYUVtoRGBImage((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
			}
			else
			{
				ConvertYUVtoRGBImage((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
			}
			#endif
            nPreSeq = buf->nSeq;
        }
		else
        {
            yuv_video_buf bufResult;
			
			{
				GetImageFromResultFrameList(uSeq,nPreSeq,bufResult);

				#ifdef savevtstxt
					fprintf(fp,"=======%d,not find,bufResult.nSeq=%lld,nPreSeq=%lld,nMinSeq=%lld\n",i,bufResult.nSeq,nPreSeq,nMinSeq);
				#endif

				if( abs((long long)nMinSeq - (long long)uSeq) < abs((long long)bufResult.nSeq - (long long)uSeq) )//取缓冲区最后一个
				{
					it_map = mapSeqIndex.find(nMinSeq);
					index = it_map->second;

					yuv_video_buf* buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);
					(pTimeStamp+i)->ts = buf->ts;
					(pTimeStamp+i)->uTimestamp = buf->uTimestamp;
					(pTimeStamp+i)->uSeq = buf->nSeq;
					
					#ifndef ALGORITHM_YUV_CARNUM
					memcpy(m_imgComposeResult->imageData,m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf),m_imgComposeResult->imageSize);
					#else
					cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
					if(m_nWordPos == 0)
					{
						ConvertYUVtoRGBImage((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
					}
					else
					{
						ConvertYUVtoRGBImage((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
					}
					#endif
					nPreSeq = nMinSeq;
					bufResult.nSeq = nMinSeq;
				}
				else
				{
					(pTimeStamp+i)->ts = bufResult.ts;
					(pTimeStamp+i)->uTimestamp = bufResult.uTimestamp;
					nPreSeq = bufResult.nSeq;
				}
			}
        }

        CvRect rect;
        int height = m_imgComposeResult->height;
        if(m_nWordOnPic == 1)
        {
            height -= m_nExtentHeight;
        }

        if(m_imgComposeSnap->width == m_imgComposeResult->width)
        {
            rect.x = 0;
            rect.y = i*height;
        }
		else if(m_imgComposeSnap->width > 2*m_imgComposeResult->width)
		{
			rect.x = i*m_imgComposeResult->width;
			rect.y = 0;
		}
        else
        {
			rect.x = (i%2)*m_imgComposeResult->width;
			rect.y = (i/2)*height;
        }
        rect.width = m_imgComposeResult->width;
        rect.height = height;

        CvRect rt;
        rt.x = 0;
        rt.y = 0;
        rt.width = m_imgComposeResult->width;
        rt.height = height;

		if((m_nWordPos == 1)&&(m_nWordOnPic == 1))
		{
			rt.y += m_nExtentHeight;
		}

		if(g_nVtsPicMode == 5)
		{
			if (g_nServerType != 23)
			{
				rect.y = m_imgComposeSnap->height - m_imgComposeResult->height;
			}
		}

		if (1 == g_PicFormatInfo.nSmallViolationPic && (1 == g_nVtsPicMode || 5 == g_nVtsPicMode)) //违章图片加小图 并且 2x2叠加图
		{
			//逆行 压线 取第3张图,其他违章取第1张图片
			int picIndex = 0;
			if (DETECT_RESULT_RETROGRADE_MOTION == uViolationType)  //逆行
			{
				picIndex = 2;
			}

			if (i == picIndex)
			{
				//存小图
				RECORD_PLATE plate_r;
				plate_r.uPosLeft = pSignalTimeStamp->x;
				plate_r.uPosTop = pSignalTimeStamp->y;
				plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
				plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;
				plate_r.uType = pSignalTimeStamp->nType;
				if(!pSignalTimeStamp->IsCarnum)
				{
					plate_r.chText[0] = '*';
				}

				CvRect srcRt,dstRt;
				if (g_nServerType == 15 || g_nServerType == 25)//小图放在右下位置
				{
					dstRt.x = m_imgComposeResult->width;
					dstRt.y = height;
				}
				else
				{
					dstRt.x = 0;
					dstRt.y = 0;
				}
				dstRt.width = rt.width;
				dstRt.height = rt.height;

				if(m_nWordPos == 1 && m_nWordOnPic == 0) //文字在上 && 文字不叠加
				{
					dstRt.y += m_nExtentHeight;
					dstRt.height -= m_nExtentHeight;
				}
				else if(m_nWordPos == 0 && m_nWordOnPic == 0)
				{
					dstRt.height -= m_nExtentHeight;
				}

				if(g_nServerType == 14)
				{
					dstRt.x = m_imgSnap->width;
					dstRt.y = m_imgSnap->height-m_nExtentHeight+120;
					dstRt.width = m_imgSnap->width;
					dstRt.height = m_imgSnap->height-m_nExtentHeight;
				}

				//遵义图片格式
				if(g_nVtsPicMode == 5)
				{
					if (g_nServerType == 23 || g_nServerType == 26)
					{
						dstRt.x = m_imgSnap->width*3;
					}
					else
					{
						dstRt.x = m_imgSnap->width*3;
						dstRt.y = m_imgComposeSnap->height - m_imgComposeResult->height+m_nExtentHeight;
						dstRt.height = m_imgComposeResult->height-m_nExtentHeight;
					}
				}

				//温岭卡口
				if(g_PicFormatInfo.nSpaceRegion == 1)
				{
					dstRt.x = 0;
					dstRt.y = height;
				}

				//取车身位置
				srcRt = GetCarPos(plate_r,2);
				//按目标区域比例裁剪
				srcRt.width = srcRt.height * dstRt.width/dstRt.height;

				if(srcRt.x + srcRt.width >= dstRt.width)
				{
					srcRt.x = dstRt.width - srcRt.width-1;
				}

				//遵义图片格式//有违章小图 并且 大图2x2叠加时 ，中间留黑条
				if(g_nVtsPicMode != 5)
				{
					if((g_nServerType != 13) && (g_nServerType != 14) && (g_nServerType != 15) && (g_nServerType != 25))
					{
						if(g_PicFormatInfo.nSpaceRegion != 1)
						{
							dstRt.width -= 2;
						}
					}
				}

				//LogNormal("d x=%d,y=%d,w=%d,h=%d\n",dstRt.x,dstRt.y,dstRt.width,dstRt.height);
				//LogNormal("s x=%d,y=%d,w=%d,h=%d\n",srcRt.x,srcRt.y,srcRt.width,srcRt.height);

				cvSetImageROI(m_imgComposeResult,srcRt);
				cvSetImageROI(m_imgComposeSnap,dstRt);
				cvResize(m_imgComposeResult, m_imgComposeSnap);
				cvResetImageROI(m_imgComposeSnap);
				cvResetImageROI(m_imgComposeResult);
			}

			if ((g_nServerType != 15) && (g_nServerType != 25) && (g_PicFormatInfo.nSpaceRegion != 1))
			{
				//非遵义图片格式
				if(g_nVtsPicMode != 5)
				{
					if(g_nServerType != 14)
					{
						if (i%2 == 0)
						{
							rect.x += rect.width;
						}
						else
						{
							rect.x -= rect.width;
							rect.y += rect.height;
						}
					}

					if((g_nServerType != 13) && (g_nServerType != 14))
					{
						if(g_PicFormatInfo.nSpaceRegion != 1)
						{
							//有违章小图 并且 大图2x2叠加时 ，中间留黑条
							if (i%2 == 0)
							{
								rect.x += 2;
								rect.width -= 2;
							}
							else
							{
								rect.width -= 2;
							}
						}
					}
				}
			}
		
		}

		//(1x2加小图)
		if(1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode)
		{
			if (g_nServerType == 3)//旅行时间图片格式
			{
				//需要取小图
				if(i == 0)
				{
					rect.x = m_imgComposeResult->width*2;
					rect.y = m_imgComposeSnap->height-m_imgComposeResult->height+m_nExtentHeight;
					rect.width = m_imgComposeResult->width;
					rect.height = m_imgComposeResult->height-m_nExtentHeight;

					//存小图
					RECORD_PLATE plate_r;
					plate_r.uPosLeft = pSignalTimeStamp->x;
					plate_r.uPosTop = pSignalTimeStamp->y;
					plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
					plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;

					rt = GetCarPos(plate_r,2);

					cvSetImageROI(m_imgComposeResult,rt);
					cvSetImageROI(m_imgComposeSnap,rect);
					cvResize(m_imgComposeResult, m_imgComposeSnap);
					cvResetImageROI(m_imgComposeSnap);
					cvResetImageROI(m_imgComposeResult);
				}

				rt.x = 0;
				rt.y = m_nExtentHeight;
				rt.width = m_imgComposeResult->width;
				rt.height = m_imgComposeResult->height-m_nExtentHeight;

				rect.x = (i)*m_imgComposeResult->width;
				rect.y = m_imgComposeSnap->height-m_imgComposeResult->height+m_nExtentHeight;
				rect.width = m_imgComposeResult->width;
				rect.height = m_imgComposeResult->height-m_nExtentHeight;
			}
			else//武汉格式(1x2加小图)
			{
				//需要取小图
				if(i == 1)
				{
					rect.x = 0;
					rect.y = 380;
					rect.width = 800;
					rect.height = 800;

					//存小图
					RECORD_PLATE plate_r;
					plate_r.uPosLeft = pSignalTimeStamp->x;
					plate_r.uPosTop = pSignalTimeStamp->y;
					plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
					plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;

					rt = GetCarPos(plate_r,1);

					cvSetImageROI(m_imgComposeResult,rt);
					cvSetImageROI(m_imgComposeSnap,rect);
					cvResize(m_imgComposeResult, m_imgComposeSnap);
					cvResetImageROI(m_imgComposeSnap);
					cvResetImageROI(m_imgComposeResult);
				}

				rt.x = 0;
				rt.y = 0;
				rt.width = m_imgComposeResult->width;
				rt.height = m_imgComposeResult->height-m_nExtentHeight;

				rect.x = (i)*400;
				rect.y = m_nExtentHeight+20;//80个像素
				rect.width = 400;
				rect.height = 300;
			}
		}


		//深圳北环格式
		if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
		{
				//存小图
				RECORD_PLATE plate_r;
				plate_r.uPosLeft = pSignalTimeStamp->x;
				plate_r.uPosTop = pSignalTimeStamp->y;
				plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
				plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;
				plate_r.uType = pSignalTimeStamp->nType;
				if(!pSignalTimeStamp->IsCarnum)
				{
					plate_r.chText[0] = '*';
				}

				CvRect srcRt,dstRt;
				dstRt.x = m_imgSnap->width;
				dstRt.y = 0;
				dstRt.width = rt.width;
				dstRt.height = rt.height;

				if(m_nWordPos == 1 && m_nWordOnPic == 0) //文字在上 && 文字不叠加
				{
					dstRt.y += m_nExtentHeight;
					dstRt.height -= m_nExtentHeight;
				}
				else if(m_nWordPos == 0 && m_nWordOnPic == 0)
				{
					dstRt.height -= m_nExtentHeight;
				}

				//取车身位置
				srcRt = GetCarPos(plate_r,0);
				//按目标区域比例裁剪
				srcRt.width = srcRt.height * dstRt.width/dstRt.height;

				if(srcRt.x + srcRt.width >= dstRt.width)
				{
					srcRt.x = dstRt.width - srcRt.width-1;
				}

				cvSetImageROI(m_imgComposeResult,srcRt);
				cvSetImageROI(m_imgComposeSnap,dstRt);
				cvResize(m_imgComposeResult, m_imgComposeSnap);
				cvResetImageROI(m_imgComposeSnap);
				cvResetImageROI(m_imgComposeResult);
		}

		if(g_nServerType == 14) //北京公交项目中心端
		{
			rect.y = 120+(i/2)*(m_imgSnap->height - m_nExtentHeight);
			rect.height = m_imgSnap->height - m_nExtentHeight;

			if(m_nWordPos == 1)
			{
				rt.y = m_nExtentHeight;
			}
			else
			{
				rt.y = 0;
			}
			rt.height = m_imgSnap->height - m_nExtentHeight;
		}
		
		//LogNormal("width=%d,height=%d,rt.width=%d,rt.height=%d\n",m_imgComposeResult->width,m_imgComposeResult->height,m_imgComposeSnap->width,m_imgComposeSnap->height);
		//LogNormal("rt.x=%d,rt.y=%d,rt.width=%d,rt.height=%d,rect.x=%d,rect.y=%d,rect.width=%d,rect.height=%d\n",rt.x,rt.y,rt.width,rt.height,rect.x,rect.y,rect.width,rect.height);
		//红绿灯增强
		#ifdef REDADJUST
		RedLightAdjust(m_imgComposeResult);
		#endif
		
		cvSetImageROI(m_imgComposeResult,rt);
		cvSetImageROI(m_imgComposeSnap,rect);
		cvResize(m_imgComposeResult, m_imgComposeSnap);
		cvResetImageROI(m_imgComposeSnap);
		cvResetImageROI(m_imgComposeResult);

		if((g_nServerType == 7) &&(i==0) &&(DETECT_RESULT_NO_PARKING == uViolationType))//天津黄网格停车叠加车牌小图
		{
			//存小图
			RECORD_PLATE plate_r;
			plate_r.uPosLeft = pSignalTimeStamp->x;
			plate_r.uPosTop = pSignalTimeStamp->y;
			plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
			plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;
			plate_r.uType = pSignalTimeStamp->nType;
			if(!pSignalTimeStamp->IsCarnum)
			{
				plate_r.chText[0] = '*';
			}
			CvRect rtCarnum = GetCarPos(plate_r,4);

			//车牌放在左下角显示
			rect.x = 0;
			rect.y = m_imgComposeSnap->height/3-rtCarnum.height;
			rect.width = rtCarnum.width;
			rect.height = rtCarnum.height;

			cvSetImageROI(m_imgComposeResult,rtCarnum);
			cvSetImageROI(m_imgComposeSnap,rect);
			cvResize(m_imgComposeResult, m_imgComposeSnap);
			cvResetImageROI(m_imgComposeSnap);
			cvResetImageROI(m_imgComposeResult);
		}
    }


    //获取红灯时间
	if( SignalframeSeq > 0 )
	{
		UINT32 uSeq = SignalframeSeq;
        if(uSeq > nMaxSeq)
        {
            uSeq = nMaxSeq;
        }

		if(uSeq < nMinSeq)
        {
			//LogNormal("红灯时间太晚，取最小的帧号\n");
			uSeq = nMinSeq;
        }
		 //根据帧号找到对应的图像序号
        SeqIndexMap::iterator it_map;
        int index = 0;
        it_map = mapSeqIndex.find(uSeq);

        if(it_map!=mapSeqIndex.end())
        {
            index = it_map->second;
            yuv_video_buf* buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);

			if(buf)
			{
				pSignalTimeStamp->ts = buf->ts;
				pSignalTimeStamp->uTimestamp = buf->uTimestamp;
			}
        }
		else
		{
			//LogNormal("未找到红灯时间\n");
			pSignalTimeStamp->ts = 0;
			pSignalTimeStamp->uTimestamp = 0;
		}
	}

    //解锁
    pthread_mutex_unlock(&m_PlateMutex);


//#ifdef savevtstxt
//	std::list<std::string>::iterator it = m_ResultFrameList.begin();
//	while( (it!=m_ResultFrameList.end()))
//	{
//		yuv_video_buf* buf = (yuv_video_buf*)it->c_str();
//		fprintf(fp,"%lld,",buf->nSeq);
//		it++;
//	}
//	fprintf(fp,"\n");
//	fclose(fp);
//#endif

    return true;
}


//从Jpg缓冲区查找出对应的图片
bool CRoadCarnumDetect::GetImageByJpgSeq(UINT32 nSeq, UINT32 nPreSeq, PLATEPOSITION* pTimeStamp, IplImage* pImage, string &strGetPic)
{
	//printf("GetImageByJpgSeq 11");

	if(pImage == NULL)
	{
		//LogNormal("GetImageByJpgSeq pImage NULL! \n");
		return false;
	}

	//LogNormal("--decode--3333----\n");
	bool bGetJpg = true; //是否获取到图片
	yuv_video_buf* buf = NULL;

	if(m_JpgFrameMap.size() < 1)
	{
		LogNormal("=error==m_JpgFrameMap.size()=%d=nSeq=%d\n", m_JpgFrameMap.size(), nSeq);
	}
	else
	{
		if(nSeq > 0)
		{
			bool bCheckJpg = false;
			int nCheckCount = 5;
			for(int j=0; j<nCheckCount; j++)
			{
				bCheckJpg = CheckJpgInMap(nSeq);
				if(!bCheckJpg)
				{
					if(0 == m_nDealFlag)
					{
						LogNormal("## id:%d Wait DSP JPG:[%lld] !\n", m_nChannelId, nSeq);
						//LogNormal("==Wait 1s for DSP send JPG:[%d] %d !\n", iSeqTemp);
						//等待接收DSP发送大图
						sleep(1); //1s
					}				
				}
				else
				{
					break;
				}
			}
			
			if(!bCheckJpg)
			{				
				LogNormal("## 11 CheckFalse id:%d JPG:[%lld] !\n", m_nChannelId, nSeq);
				//return false;
			}
		}		
	}

	string strPic;
	UINT16 uSignalColor = 0;
	unsigned long uSignalSeq = 0;
	UINT32 iMinSeq = 0;
	UINT32 iMaxSeq = 0;
	UINT32 iSeqTemp = 0;

	if(m_JpgFrameMap.size() <= 0)
	{
		return false;
	}

	//加锁
	pthread_mutex_lock(&m_JpgFrameMutex);


	if(m_JpgFrameMap.size() > 1)
	{
		//LogNormal("=m_JpgFrameMap.size()=%d=nSeq=%d\n", m_JpgFrameMap.size(), nSeq);
		map<int64_t,string>::iterator it_b_min = m_JpgFrameMap.begin();
		iMinSeq = it_b_min->first;

		map<int64_t,string>::iterator it_e_max = --m_JpgFrameMap.end();
		iMaxSeq = it_e_max->first;//最大帧号
	}
	//解锁
	pthread_mutex_unlock(&m_JpgFrameMutex);


	if(nSeq > iMaxSeq)
	{
		//LogNormal("=Kakou=Wait DSP JPG:[%d] iMaxSeq=%d !\n", nSeq, iMaxSeq);
		//未收到数据计数
		m_nNotGetJpgCount ++;
		bGetJpg = false;

		//等待接收DSP发送大图
		usleep(1000*1000); //1s
	}

	//加锁
	pthread_mutex_lock(&m_JpgFrameMutex);

	if(m_JpgFrameMap.size() > 0)
	{

		map<int64_t,string>::iterator it_b = m_JpgFrameMap.begin();
		map<int64_t,string>::iterator it_e = --m_JpgFrameMap.end();
		UINT32 nMinSeq = it_b->first; //最小帧号
		UINT32 nMaxSeq = it_e->first;//最大帧号

		//LogNormal("nSeq=%lld,nMinSeq=%lld,nMaxSeq=%lld\n",nSeq,nMinSeq,nMaxSeq);

		if(nSeq <= nMinSeq)
		{
			nSeq = nMinSeq;
		}

		if(nSeq >= nMaxSeq)
		{
			nSeq = nMaxSeq;
		}


		map<int64_t,string>::iterator it_map = m_JpgFrameMap.find(nSeq);
		if(it_map!=m_JpgFrameMap.end())
		{
			strPic = it_map->second;
			//LogNormal("从m_JpgFrameMap中取图成功，帧号nSeq=%lld\n",nSeq);//add by wantao
		}
		else
		{
			LogNormal("帧号不在队列中，nSeq=%lld,nMinSeq=%lld,nMaxSeq=%lld m_nDealFlag:%d \n",\
				nSeq, nMinSeq, nMaxSeq, m_nDealFlag);

			//GetPicFromJpgMap(nSeq,nPreSeq,strPic);
		}

	}
	//解锁
	pthread_mutex_unlock(&m_JpgFrameMutex);

	if(strPic.size() > 0)
	{
		strGetPic = strPic;
		//LogNormal("strGetPic:%d ", strGetPic.size());

		buf = (yuv_video_buf*)strPic.c_str();
		uSignalColor = buf->uTrafficSignal;
		uSignalSeq = buf->nSeq;

		if(buf)
		{
			pTimeStamp->ts = buf->ts;
			pTimeStamp->uTimestamp = buf->uTimestamp;
			pTimeStamp->uFieldSeq = buf->nFieldSeq;
			pTimeStamp->uSeq = buf->nSeq;
			pTimeStamp->nRedColor = buf->uTrafficSignal;

			//LogNormal("=GetImageByJpgSeq=before Decode nSeq=%d", nSeq);
			//LogNormal("uTimestamp:%lld,ts:%d ,uFieldSeq=%d\n", pTimeStamp->uTimestamp, pTimeStamp->ts, pTimeStamp->uFieldSeq);

			//需要解码jpg图像
			 CxImage image;
             image.Decode((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)), strPic.size() - sizeof(yuv_video_buf), 3); //解码
			 
			 if(image.IsValid()&&image.GetSize()>0)
			 {
				cvSet(pImage, cvScalar( 0,0, 0 ));

				 //if(image.GetSize() == pImage->imageSize)
				 {
					 if(m_nWordPos == 0)//黑边在下方
					 {
						//LogNormal("image.GetSize()=%d,strPic.size()=%d,帧号nSeq=%lld\n",image.GetSrcSize(),strPic.size(),nSeq);//add by wantao
						memcpy(pImage->imageData,image.GetBits(),image.GetSrcSize());
						//memcpy(pImage->imageData,image.GetBits(),pImage->imageSize-m_nExtentHeight*pImage->widthStep);
					 }
					 else//黑边在上方
					 {
						 memcpy(pImage->imageData+m_nExtentHeight*pImage->widthStep,image.GetBits(),image.GetSrcSize());
						 //memcpy(pImage->imageData+m_nExtentHeight*pImage->widthStep,image.GetBits(),pImage->imageSize-m_nExtentHeight*pImage->widthStep);
					 }
				 }
				 //LogNormal("解码成功\n");//add by wantao
			 }
			 else //解码失败！！
			 {
				 LogNormal("解码失败\n");//add by wantao
			 }			 
		}
	}
	
#ifdef DEBUG_GAIN
	//叠字曝光，增益
	if(pImage)
	{
		int nGain = buf->uGain;
		int nSh = buf->nSh;
		
		CvxText cvTextImg;
		cvTextImg.Init(40);

		wchar_t wchOut[255] = {'\0'};
		char chOut[255] = {'\0'};

		int nWidth = pImage->width - 800;
		int nHeight = 30;

		//设备编号
		sprintf(chOut,"seq:%lld Gain:%d Sh:%d", buf->nSeq, nGain, nSh);
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight = 30;
		cvTextImg.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		cvTextImg.UnInit();
	}
#endif
	//红绿灯增强
	#ifdef REDADJUST
	RedLightAdjust(pImage, true, uSignalColor,uSignalSeq);
	#endif

	//图像增强
	if(m_bImageEnhance)
	{
		ImageEnhance(pImage);
	}
	

	if(bGetJpg)
	{
		m_nNotGetJpgCount = 0;//计数归0
	}

	if(m_nNotGetJpgCount > 100)
	{
		m_nNotGetJpgCount = 0;//计数归0

		/*
		if(g_nDetectMode == 2)
		{
			//长时间未收到对应帧图片，让相机重启
			LogNormal("==未收到对应帧图片，重启相机!=\n");

			CAMERA_CONFIG cfg;
			cfg.uType = 1;
			cfg.nIndex = (int)CAMERA_RESTART;
			g_skpChannelCenter.CameraControl(m_nChannelId,cfg);
		}
		*/
	}

	return true;
}

//根据帧号查找出对应的图片
bool CRoadCarnumDetect::GetImageByIndex(UINT32 nSeq,UINT32 nPreSeq,PLATEPOSITION* pTimeStamp,IplImage* pImage,bool bLightImage,bool bCarNum)
{
	//LogTrace("LogDsp.log", "GetImageByIndex=nSeq=%lld=nPreSeq=%lld=bCarNum=%d=\n", nSeq, nPreSeq, bCarNum);	
	if(g_nDetectMode == 2)
	{
		m_strPicMatch = "";
		bool bGetJpg = GetImageByJpgSeq(nSeq,nPreSeq,pTimeStamp,pImage, m_strPicMatch);
		
		if(!bGetJpg)
		{
			LogNormal("bGetJpg:%d nSeq:%lld \n", bGetJpg, nSeq);			
			//删除记录,不输出
		}
		return bGetJpg;
	}

    int index = 0;
    yuv_video_buf* buf = NULL;

    //加锁
    pthread_mutex_lock(&m_PlateMutex);
    //暂存seq->index对应关系
    SeqIndexMap mapSeqIndex;
    for(int j = 0;j<m_nMaxPlateSize;j++)
    {
        yuv_video_buf* pBuf = (yuv_video_buf*)(m_BufPlate[j].pCurBuffer);
        if(pBuf!=NULL)
        {
            mapSeqIndex.insert(SeqIndexMap::value_type(pBuf->nSeq,j));
        }
    }

	if(mapSeqIndex.size()<=1)
	{
		 //解锁
		pthread_mutex_unlock(&m_PlateMutex);
		return;
	}

    SeqIndexMap::iterator it_b = mapSeqIndex.begin();
    SeqIndexMap::iterator it_e = --mapSeqIndex.end();
    UINT32 nMinSeq = it_b->first; //最小帧号
    UINT32 nMaxSeq = it_e->first;//最大帧号

	//LogNormal("Seq=%lld,MinSeq=%lld,MaxSeq=%lld,PreSeq=%lld",nSeq,nMinSeq,nMaxSeq,nPreSeq);

    if(nSeq < nMinSeq)
    {
		
		if(bCarNum)
		{
			LogNormal("小于最小帧号Seq=%lld,MinSeq=%lld,MaxSeq=%lld,PreSeq=%lld",nSeq,nMinSeq,nMaxSeq,nPreSeq);
		}
        nSeq = nMinSeq;
    }

    if(nSeq > nMaxSeq)
    {
		//LogNormal("大于最大帧号bLightImage=%d,nSeq=%lld,nMaxSeq=%lld,nPreSeq=%lld",bLightImage,nSeq,nMaxSeq,nPreSeq);
        nSeq = nMaxSeq;
    }

    SeqIndexMap::iterator it_map = mapSeqIndex.find(nSeq);
    if(it_map!=mapSeqIndex.end())
    {
        index = it_map->second;
    }
    else
    {
		//LogNormal("未找到对应图片Seq=%lld\n",nSeq);
        index = GetSeqIndexFromMap(nSeq,nPreSeq,mapSeqIndex);
    }

    // LogNormal("GetImageByIndex,nSeq=%lld,nMinSeq=%lld,nMaxSeq=%lld,index=%lld",nSeq,nMinSeq,nMaxSeq,index);

    buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);
    if(buf)
    {
        #ifdef savetxt
        fprintf(fp,"index=%d,nSeq=%lld,buf->nSeq=%lld\n",index,nSeq,buf->nSeq);
        #endif

        pTimeStamp->ts = buf->ts;
        pTimeStamp->uTimestamp = buf->uTimestamp;
        pTimeStamp->uFieldSeq = buf->nFieldSeq;
		pTimeStamp->uSeq = buf->nSeq;

		#ifdef ALGORITHM_YUV_CARNUM
		cvSet(pImage, cvScalar( 0,0, 0 ));
		if(m_nWordPos == 0)
		{
			ConvertYUVtoRGBImage(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf),(BYTE*)pImage->imageData,pImage->width,pImage->height-m_nExtentHeight);
		}
		else
		{
			ConvertYUVtoRGBImage(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)+pImage->width*2*m_nExtentHeight,(BYTE*)(pImage->imageData+pImage->width*3*m_nExtentHeight),pImage->width,pImage->height-m_nExtentHeight);
		}
		#else
        cvSetData(m_imgResult,m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf),m_imgResult->widthStep);
       // LogNormal("图像时间%lld,线圈时间%lld",buf->ts,buf->uSpeedSignal.SystemTime);

        if(m_nDeinterlace == 2)
        {
            cvResize(m_imgResult,pImage);
        }
        else
        {
            cvCopy(m_imgResult,pImage);
        }
		#endif
    }
    //解锁
    pthread_mutex_unlock(&m_PlateMutex);

	//红绿灯增强
	#ifdef REDADJUST
	#ifndef ALGORITHM_YUV
	RedLightAdjust(pImage);
	#endif
	#endif

	//图像增强
	if(m_bImageEnhance)
	{
		ImageEnhance(pImage);
	}
	
	return true;
}

//求RECT总灰度值 
bool CRoadCarnumDetect::AverageValue(IplImage *pGrayImg,CvRect rect)
{ 
	int nth = 125;//设定亮度阈值 

	//rect区域的灰度平均值 
	cvSetImageROI(pGrayImg, rect); 
	CvScalar avgVal = cvAvg(pGrayImg);//计算灰度平均值 
	cvResetImageROI(pGrayImg); 

	int nAvgVal = (int) avgVal.val[0];//灰度平均值 

	//LogNormal("[%d][X:%d Y:%d W:%d H:%d",nAvgVal,rect.x,rect.y,rect.width,rect.height);

	//平均灰度值判断 
	if ( nAvgVal > nth) 
	{ 
		return false; 
	} 
	else 
	{ 
		return true; 
	} 
} 



//电警叠加文本信息
void CRoadCarnumDetect::PutVtsTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp,PLATEPOSITION* pSignalTimeStamp,int64_t redLightStartTime, UINT32 uViolationType)
{
	string strDeviceId;
	if(m_strDeviceId.size() > 0)
	{
		strDeviceId = m_strDeviceId;
	}
	else
	{
		strDeviceId = g_strDetectorID;
	}
	//LogNormal("g_nServerType:%d PutVtsTextOnImage nIndex:%d !\n", g_nServerType, nIndex);
#ifdef IMAGETEXT
		if(g_nServerType == 0)
		{
			wchar_t wchOut[255] = {'\0'};
			char chOut[255] = {'\0'};
			int nWidth = 0;
			int nHeight = 0;
			int timeIndex = nIndex;

			timeIndex = nIndex;

			int nExtentHeight = 60;
			nWidth += m_PicFormatInfo.nOffsetX+40;
			nHeight = (m_imgSnap->height)*nIndex + m_PicFormatInfo.nOffsetY+40;

			nHeight += (nExtentHeight/2);
			std::string strTime;
			strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
			sprintf(chOut,"违法时间 ：%s:%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);

			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(242, 233, 99));

			nHeight += (nExtentHeight/2);
			sprintf(chOut,"违法地点 ：%s", m_strLocation.c_str());
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(242, 233, 99));

			nHeight += (nExtentHeight/2);
			std::string strDirection = GetDirection(plate.uDirection);
			sprintf(chOut,"方   向 ：%s", strDirection.c_str());
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(242, 233, 99));

			nHeight += (nExtentHeight/2);
			std::string strViolationType = GetViolationType(plate.uViolationType,1);
			sprintf(chOut,"违法类型 ：%s", strViolationType.c_str());
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(242, 233, 99));

			nHeight += (nExtentHeight/2);
			sprintf(chOut,"设备编号 ：%s", m_strDeviceId.c_str());
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(242, 233, 99));

			nHeight += (nExtentHeight/2);
			m_nRandCode[0] = g_RoadImcData.GetRandCode();
			m_nRandCode[1] = g_RoadImcData.GetRandCode();
			sprintf(chOut,"防伪码  ：%08x%08x", m_nRandCode[0],m_nRandCode[1]);
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(242, 233, 99));
			return;
		}
#endif
    

	if (g_nServerType == 30)  //徐汇掉头图片格式
	{
		CvxText cvText;
		cvText.Init(80);

		int nWidth = 0;
		int	nHeight = 0;
		wchar_t wchOut[255] = {'\0'};
		char chOut[255] = {'\0'};

		if(nIndex < 3)
		{
			nWidth = (m_imgSnap->width)*((nIndex+1)%2) + 10;
			nHeight = (m_imgSnap->height)*((nIndex+1)/2) + m_nExtentHeight + 20;
		}
		else
		{
			nWidth = 10;
			nHeight = m_nExtentHeight + 20;
		}

		//经过时间
		std::string strTime;
		strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
		sprintf(chOut,"时间：%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		cvText.UnInit();
		return;
	}
	if (g_nServerType == 15)
	{

		/*CvxText McvText;
		McvText.Init(m_nFontSize);*/
		wchar_t wchOut[255] = {'\0'};
		char chOut[255] = {'\0'};

		int nWidth = 0;
		int nHeight = 0;
		int nDistance = m_nFontSize;

		if (nIndex == 1)
		{
			nWidth = m_imgSnap->width;
		}
		if (nIndex == 2)
		{
			nHeight = m_imgSnap->height;
			if(m_nWordOnPic == 1)
			{
				nHeight -= m_nExtentHeight;
			}
		}
		/*if (nIndex == 3)
		{
			nWidth = m_imgSnap->width;
			nHeight = m_imgSnap->height;
			if(m_nWordOnPic == 1)
			{
				nHeight -= m_nExtentHeight;
			}
		}*/
		nWidth += m_PicFormatInfo.nOffsetX;
		nHeight += m_PicFormatInfo.nOffsetY;

		CvRect dstRt;

		dstRt.x = 0;
		dstRt.y = 0;
		dstRt.x += m_PicFormatInfo.nOffsetX;
		dstRt.y += m_PicFormatInfo.nOffsetY;
		if (nIndex == 0)
		{
			if (dstRt.x > (m_imgSnap->width - m_BlackFrameWidth))
			{
				dstRt.x = 0;
				nWidth -= m_PicFormatInfo.nOffsetX;
				nHeight -= m_PicFormatInfo.nOffsetY;
			}
		}
		else if (nIndex == 1)
		{
			dstRt.x += m_imgSnap->width;
			if (dstRt.x > ((m_imgSnap->width - m_BlackFrameWidth)+ m_imgSnap->width))
			{
				dstRt.x = 0;
				nWidth -= m_PicFormatInfo.nOffsetX;
				nHeight -= m_PicFormatInfo.nOffsetY;
			}
			
		}
		else if (nIndex == 2)
		{
			dstRt.y += (m_imgSnap->height - m_nExtentHeight);
			if (dstRt.x > (m_imgSnap->width - m_BlackFrameWidth))
			{
				dstRt.x = 0;
				nWidth -= m_PicFormatInfo.nOffsetX;
				nHeight -= m_PicFormatInfo.nOffsetY;
			}
		}
		/*else if (nIndex == 3)
		{
			dstRt.x += m_imgSnap->width;
			dstRt.y += (m_imgSnap->height - m_nExtentHeight);
			if (dstRt.x > ((m_imgSnap->width - m_BlackFrameWidth)+ m_imgSnap->width))
			{
				dstRt.x = 0;
				nWidth -= m_PicFormatInfo.nOffsetX;
				nHeight -= m_PicFormatInfo.nOffsetY;
			}
		}*/
	
		if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
		{
			dstRt.height = nDistance*5;//五行字
			dstRt.width = m_BlackFrameWidth;
		}
		else
		{
			dstRt.height = nDistance*3;//三行字
			dstRt.width = m_BlackFrameWidth;
		}
		

		cvSetImageROI(pImage,dstRt);
		cvSet(pImage,cvScalar(0,0,0));
		cvResetImageROI(pImage);

		

		//设备编号
		sprintf(chOut,"设备编号:%s",strDeviceId.c_str());
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight += nDistance;
		m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		//经过地点,行驶方向
		std::string strDirection = GetDirection(plate.uDirection);
		sprintf(chOut,"地点:%s  方向:%s",m_strLocation.c_str(),strDirection.c_str());
		//sprintf(chOut,"地点:%s  方向:%s  车道:%d",m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID);

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight += nDistance;
		m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		//时间
		std::string strTime;
		/*if (nIndex == 3)
		{
			strTime = GetTime(pTimeStamp->uTimestamp,0);
			sprintf(chOut,"时间:%s.%03d",strTime.c_str(),((pTimeStamp->ts)/1000)%1000);
		}
		else*/
		{
			strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
			sprintf(chOut,"时间:%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
		}

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight += nDistance;
		m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		/*if (plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
		{
			string strRedLightBeginTime;
			string strRedLightEndTime;

			strRedLightBeginTime = GetTime(plate.uRedLightBeginTime,0);
			sprintf(chOut,"红灯开启时间: %s.%03d",strRedLightBeginTime.c_str(),plate.uRedLightBeginMiTime);
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			nHeight += nDistance;
			m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			strRedLightEndTime = GetTime(plate.uRedLightEndTime,0);//GetTime(pSignalTimeStamp->uTimestamp+m_vtsGlobalPara.nRedLightTime,0);
			sprintf(chOut,"红灯结束时间: %s.%03d",strRedLightEndTime.c_str(),plate.uRedLightEndMiTime);
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			nHeight += nDistance;
			m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
		}*/
		//McvText.UnInit();
		return;
	}

	//深圳北环格式
	if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
	{
		wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nWidth = 10;
        int nHeight = 0;

		//经过时间(第一行)
		std::string strTime;
		strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
		sprintf(chOut,"时间: %s",strTime.c_str());
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight += g_PicFormatInfo.nFontSize;
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

		 //经过地点
		std::string strViolationType = GetViolationType(plate.uViolationType,1);
		sprintf(chOut,"违法地点: %s(%s) 车道: %d车道",m_strLocation.c_str(),strViolationType.c_str(),plate.uRoadWayID);
        memset(wchOut,0,sizeof(wchOut));
        UTF8ToUnicode(wchOut,chOut);
        nHeight += (g_PicFormatInfo.nFontSize);
        m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

		return;
	}

	//
	if(5 == g_nVtsPicMode)
	{
		if(g_nServerType == 29)//南昌格式
		{
			wchar_t wchOut[255] = {'\0'};
			char chOut[255] = {'\0'};
			int nStartX = 0;
			int nWidth = 0;
			int nHeight = 0;

			nWidth = m_imgSnap->width*nIndex+10;
			nHeight += 100;
			nWidth += m_PicFormatInfo.nOffsetX;
			nHeight += m_PicFormatInfo.nOffsetY;
			sprintf(chOut,"地点:%s",m_strLocation.c_str());//
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

			nHeight += 100;
			std::string strDirection = GetDirection(plate.uDirection);
			sprintf(chOut,"方向:%s",strDirection.c_str());
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

			nHeight += 100;
			std::string strViolationType = GetViolationType(plate.uViolationType,1);
			sprintf(chOut,"违法行为:%s%d",strViolationType.c_str(),nIndex+1);
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

			nHeight += 100;
			std::string strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,7);
			sprintf(chOut,"抓拍时间:%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
			
			return;
		}

		if(nIndex == 0)
		{
				wchar_t wchOut[255] = {'\0'};
				wchar_t wchOut1[255] = {'\0'};
				wchar_t wchOut2[255] = {'\0'};
				char chOut[255] = {'\0'};
				char chOut1[255] = {'\0'};
				char chOut2[255] = {'\0'};

				int nStartX = 0;
				int nWidth = 10;
				int nHeight = 90;
			   
				nStartX = nWidth;

				//设备编号,经过地点,行驶方向,经过时间
				std::string strDirection = GetDirection(plate.uDirection);
				std::string strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);

				if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
				{
                    string strRedLightStartTime = GetTime(plate.uRedLightBeginTime,0);
					string strRedLightEndTime = GetTime(plate.uRedLightEndTime,0);
					
					if(g_nServerType == 23 || g_nServerType == 26)//济南格式
					{
						std::string strTime2 = GetTime((pTimeStamp+1)->uTimestamp,0);
						std::string strTime3 = GetTime((pTimeStamp+2)->uTimestamp,0);

						sprintf(chOut,"设备编号:%s  地点:%s  方向:%s  违法时间:%s.%03d  红灯开始时间:%s.%03d  红灯结束时间:%s.%03d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,strRedLightStartTime.c_str(),plate.uRedLightBeginMiTime,strRedLightEndTime.c_str(),plate.uRedLightEndMiTime);//
						sprintf(chOut1,"设备编号:%s  地点:%s  方向:%s  违法时间:%s.%03d  红灯开始时间:%s.%03d  红灯结束时间:%s.%03d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime2.c_str(),(((pTimeStamp+1)->ts)/1000)%1000,strRedLightStartTime.c_str(),plate.uRedLightBeginMiTime,strRedLightEndTime.c_str(),plate.uRedLightEndMiTime);//
						sprintf(chOut2,"设备编号:%s  地点:%s  方向:%s  违法时间:%s.%03d  红灯开始时间:%s.%03d  红灯结束时间:%s.%03d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime3.c_str(),(((pTimeStamp+2)->ts)/1000)%1000,strRedLightStartTime.c_str(),plate.uRedLightBeginMiTime,strRedLightEndTime.c_str(),plate.uRedLightEndMiTime);//
					}
					else
					{
					   sprintf(chOut,"设备编号:%s  地点:%s  方向:%s  违法时间:%s.%03d  红灯开始时间:%s.%03d  红灯结束时间:%s.%03d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,strRedLightStartTime.c_str(),plate.uRedLightBeginMiTime,strRedLightEndTime.c_str(),plate.uRedLightEndMiTime);//
					}
				}
				else if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
				{
					if(g_nServerType == 23 || g_nServerType == 26)//济南格式
					{
						int nOverSped = 0;
						if(plate.uLimitSpeed > 0)
						{
							nOverSped = (plate.uSpeed-plate.uLimitSpeed)*100/plate.uLimitSpeed;
						}
						//防伪码
						m_nRandCode[0] = g_RoadImcData.GetRandCode();
						m_nRandCode[1] = g_RoadImcData.GetRandCode();
						strTime = GetTime((pTimeStamp)->uTimestamp,0);
						std::string strTime2 = GetTime((pTimeStamp+1)->uTimestamp,0);

						sprintf(chOut,"设备编号:%s  违法地点:%s  %s  车道号:%d  违法时间:%s.%03d  车辆行驶速度:%dkm/h  路段行驶限速:%dkm/h  超速百分比:%d%%  防伪码: %08x",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strTime.c_str(),((pTimeStamp->ts)/1000)%1000,plate.uSpeed,plate.uLimitSpeed,nOverSped,m_nRandCode[0]);
						sprintf(chOut1,"设备编号:%s  违法地点:%s  %s  车道号:%d  违法时间:%s.%03d  车辆行驶速度:%dkm/h  路段行驶限速:%dkm/h  超速百分比:%d%%  防伪码: %08x",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strTime2.c_str(),(((pTimeStamp+1)->ts)/1000)%1000,plate.uSpeed,plate.uLimitSpeed,nOverSped,m_nRandCode[1]);
					}
					else
					{
						sprintf(chOut,"设备编号:%s  地点:%s  方向:%s  违法时间:%s.%03d  速度值:%dkm/h  限速值:%dkm/h",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,plate.uSpeed,plate.uLimitSpeed);//
					}
				}
				else
				{
					if(g_nServerType == 23 || g_nServerType == 26)//济南格式
					{
						strTime = GetTime((pTimeStamp)->uTimestamp,0);
						std::string strTime2 = GetTime((pTimeStamp+1)->uTimestamp,0);
						std::string strTime3 = GetTime((pTimeStamp+2)->uTimestamp,0);

						//防伪码
						m_nRandCode[0] = g_RoadImcData.GetRandCode();
						m_nRandCode[1] = g_RoadImcData.GetRandCode();
						m_nRandCode[2] = g_RoadImcData.GetRandCode();
						sprintf(chOut,"%s.%03d  %s%s  设备编号:%s  车道号:%d  防伪码: %08x",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),strDeviceId.c_str(),plate.uRoadWayID,m_nRandCode[0]);//
						sprintf(chOut1,"%s.%03d  %s%s  设备编号:%s  车道号:%d  防伪码: %08x",strTime2.c_str(),(((pTimeStamp+nIndex+1)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),strDeviceId.c_str(),plate.uRoadWayID,m_nRandCode[1]);//
						sprintf(chOut2,"%s.%03d  %s%s  设备编号:%s  车道号:%d  防伪码: %08x",strTime3.c_str(),(((pTimeStamp+nIndex+2)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),strDeviceId.c_str(),plate.uRoadWayID,m_nRandCode[2]);//
					}
					else
					{
						sprintf(chOut,"设备编号:%s  地点:%s  方向:%s  违法时间:%s.%03d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);//
					}
				}
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				memset(wchOut1,0,sizeof(wchOut1));
				UTF8ToUnicode(wchOut1,chOut1);
				memset(wchOut2,0,sizeof(wchOut2));
				UTF8ToUnicode(wchOut2,chOut2);

				if(g_nServerType == 23 || g_nServerType == 26)//济南格式
				{
					printf("before m_cvBigText.putText\n");
					nHeight = m_nExtentHeight-10;
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					m_cvText.putText(pImage, wchOut1, cvPoint(nWidth + m_imgSnap->width, nHeight), CV_RGB(255,255,255));
					m_cvText.putText(pImage, wchOut2, cvPoint(nWidth + 2*m_imgSnap->width, nHeight), CV_RGB(255,255,255));
					printf("after m_cvBigText.putText\n");
					 //第一时间
					nWidth = 10;
					nHeight = m_nExtentHeight+20;
					nWidth += m_PicFormatInfo.nOffsetX;
					nHeight += m_PicFormatInfo.nOffsetY;
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));


					//第二时间
					nWidth = m_imgSnap->width+10;
					nWidth += m_PicFormatInfo.nOffsetX;
					strTime = GetTime((pTimeStamp+nIndex+1)->uTimestamp,0);
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex+1)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					//第三时间
					nWidth = m_imgSnap->width*2+10;
					nWidth += m_PicFormatInfo.nOffsetX;
					strTime = GetTime((pTimeStamp+nIndex+2)->uTimestamp,0);
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex+2)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));


					//小图时间
					/*nWidth = m_imgSnap->width*3+10;
					nWidth += m_PicFormatInfo.nOffsetX;
					strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));*/
				}
				else
				{
					m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					 //第一时间
					nWidth = m_imgSnap->width/2-400;
					nHeight += 100;
					nWidth += m_PicFormatInfo.nOffsetX;
					nHeight += m_PicFormatInfo.nOffsetY;
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));


					//第二时间
					nWidth = m_imgSnap->width*3/2-400;
					nWidth += m_PicFormatInfo.nOffsetX;
					strTime = GetTime((pTimeStamp+nIndex+1)->uTimestamp,0);
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex+1)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

					//第三时间
					nWidth = m_imgSnap->width*5/2-400;
					nWidth += m_PicFormatInfo.nOffsetX;
					strTime = GetTime((pTimeStamp+nIndex+2)->uTimestamp,0);
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex+2)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));


					//小图时间
					nWidth = m_imgSnap->width*7/2-400;
					nWidth += m_PicFormatInfo.nOffsetX;
					strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
					sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
				}
		}

		return;
	}

	if((m_nExtentHeight <= 0) &&(m_nWordOnPic == 0))
	{
		return;
	}

	int timeIndex = nIndex;
	if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)//有违章小图 并且 大图2x2叠加
	{
		//增加第一张小图的时间,与第二张图的时间相等
		if (timeIndex > 0)
			timeIndex -= 1;
		//逆行的情况,第一张图片时间从第3张图片中取得
		if ( (DETECT_RESULT_RETROGRADE_MOTION == uViolationType /*||
			DETECT_RESULT_PRESS_LINE == uViolationType || //压黄线
			DETECT_RESULT_PRESS_WHITELINE == uViolationType ||
			DETECT_RESULT_ELE_EVT_BIANDAO == uViolationType */) && //变道
			0 == nIndex)
		{
			timeIndex = 2;
		}
		if ( DETECT_RESULT_BIG_IN_FORBIDDEN_TIME == uViolationType  && nIndex == 0 && plate.uDetectDirection == 0)
		{
			timeIndex = 3;
		}
		
		if(g_PicFormatInfo.nSpaceRegion == 1)
		{
			timeIndex = nIndex;

			if(nIndex == 2)
			{
				timeIndex = 0;
			}
		}
	}

    if(pTimeStamp != NULL)
	{

        wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nWidth = 10;
        int nHeight = 0;


        if(m_nWordOnPic == 1)//字直接叠加在图上
		{
            if(pImage->width > m_imgSnap->width)
            {
				if(g_nServerType == 24)//南宁格式
				{
					if(nIndex == 3)
					{
						return;
					}
					timeIndex = nIndex;

					int nExtentHeight = 60;
					nWidth += (m_imgSnap->width)*(nIndex%2) + m_PicFormatInfo.nOffsetX+40;
					nHeight = (m_imgSnap->height)*(nIndex/2)+ m_PicFormatInfo.nOffsetY+40;

					std::string strTime;
					strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
					sprintf(chOut,"捕获时间:%s %03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));
					
					//红灯时间(第一行)
					if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
					{
						nHeight += (nExtentHeight/2);
						std::string strTimeRed = GetTime(plate.uRedLightBeginTime,0);
						sprintf(chOut,"红灯开始时间: %s %03d",strTimeRed.c_str(),plate.uRedLightBeginMiTime);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));
					}	
					
					nHeight += (nExtentHeight/2);
					sprintf(chOut,"地点名称:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

					nHeight += (nExtentHeight/2);
					std::string strDirection = GetDirection(plate.uDirection);
					sprintf(chOut,"行驶方向:%s", strDirection.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

					nHeight += (nExtentHeight/2);
					sprintf(chOut,"车道编号:%d", plate.uRoadWayID);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

					nHeight += (nExtentHeight/2);
					sprintf(chOut,"防伪码:NNJJBC%08x%01x", g_RoadImcData.GetRandCode(), g_RoadImcData.GetRandCode()%10);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

					return;
				}

				if(g_nServerType == 25)//合肥格式
				{
					if(nIndex == 3)
					{
						return;
					}
					timeIndex = nIndex;

					CvxText cvText;
				    cvText.Init(15);

					nWidth += (m_imgSnap->width)*(nIndex%2)+10;
					nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex/2)+150;
					
					std::string strTime;
					if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
					{
						strTime = GetTime(plate.uRedLightBeginTime,0);
						sprintf(chOut,"红灯时间: %s:%03d",strTime.c_str(),plate.uRedLightBeginMiTime);
						memset(wchOut,0,sizeof(wchOut))	;	
						UTF8ToUnicode(wchOut, chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0, 255, 255));

						nHeight += (m_nExtentHeight/2);
					}
					
					strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
					sprintf(chOut,"违法时间:%s:%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0, 255, 255));
					
					sprintf(chOut,"%s:%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight-120), CV_RGB(0, 0, 0));

					nHeight += (m_nExtentHeight/2);

					sprintf(chOut,"违法地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0, 255, 255));

					nHeight += (m_nExtentHeight/2);
					
					std::string strDirection = GetDirection(plate.uDirection);
					sprintf(chOut,"违法方向:%s",strDirection.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0, 255, 255));

					cvText.UnInit();

					return;
				}
				
				if( (1 == g_PicFormatInfo.nSmallViolationPic) && (1 == g_nVtsPicMode) && (m_nExtentHeight > 0))
				{
					if(5 == g_PicFormatInfo.nForeColor)//临朐格式
					{
						CvScalar color = CV_RGB(255,0,0);

						nWidth += (m_imgSnap->width)*(nIndex%2) + m_PicFormatInfo.nOffsetX+40;
						nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex/2)+ m_PicFormatInfo.nOffsetY+40;

						if(nIndex == 0)
						{
							m_nRandCode[0] = g_RoadImcData.GetRandCode();
							m_nRandCode[1] = g_RoadImcData.GetRandCode();
						}

						if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
						{
							/*if(nIndex == 0)
							{
								timeIndex = 2;
							}*/
							color = CV_RGB(0,0,255);
						}
						
						std::string strTime;
						strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
						int nRandCode = g_RoadImcData.GetRandCode();
						sprintf(chOut, "抓拍时间:%s.%03d 防伪码:%08x%08x", strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000,m_nRandCode[0],m_nRandCode[1]);
						memset(wchOut,0,sizeof(wchOut))	;	
						UTF8ToUnicode(wchOut, chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

						if(nIndex == 0)
						{
							nHeight += (m_nExtentHeight/2);
							std::string strDirection = GetDirection(plate.uDirection);
							sprintf(chOut,"抓拍地点:%s %s %d车道", m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID);
							memset(wchOut,0,sizeof(wchOut));
							UTF8ToUnicode(wchOut,chOut);
							m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

							nHeight += (m_nExtentHeight/2);
							sprintf(chOut,"设备编号:%s", strDeviceId.c_str());
							memset(wchOut,0,sizeof(wchOut));
							UTF8ToUnicode(wchOut,chOut);
							m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

							if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
							{
								int nOverSped = 0;
								if(plate.uLimitSpeed > 0)
								{
									nOverSped = (plate.uSpeed-plate.uLimitSpeed)*100/plate.uLimitSpeed;
								}

								nHeight += (m_nExtentHeight/2);
								sprintf(chOut,"车速:%dkm/h 限速:%dkm/h 超速百分比:%d%%",plate.uSpeed,plate.uLimitSpeed,nOverSped);
								memset(wchOut,0,sizeof(wchOut));
								UTF8ToUnicode(wchOut,chOut);
								m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);
							}
							else
							{
								if(g_PicFormatInfo.nViolationType == 1)
								{
									std::string strViolationType = GetViolationType(plate.uViolationType,1);
									nHeight += (m_nExtentHeight/2);
									sprintf(chOut,"违章类型:%s",strViolationType.c_str());
									memset(wchOut,0,sizeof(wchOut));
									UTF8ToUnicode(wchOut,chOut);
									m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);
								}
							}
						}
					}
					else//路桥格式
					{
						nWidth += (m_imgSnap->width)*(nIndex%2) + m_PicFormatInfo.nOffsetX+40;
						nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex/2)+ m_PicFormatInfo.nOffsetY+40;

						sprintf(chOut, "设备ID:%s", strDeviceId.c_str());
						memset(wchOut,0,sizeof(wchOut))	;	
						UTF8ToUnicode(wchOut, chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

						nHeight += (m_nExtentHeight/2);

						sprintf(chOut,"抓拍地点:%s", m_strLocation.c_str());
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

						nHeight += (m_nExtentHeight/2);
						std::string strTime;
						strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
						sprintf(chOut,"抓拍时间:%s:%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

						//红灯时间(第三行)
						if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
						{
							strTime = GetTime(plate.uRedLightBeginTime,0);
							sprintf(chOut,"红灯时间: %s:%03d",strTime.c_str(),plate.uRedLightBeginMiTime);
							memset(wchOut,0,sizeof(wchOut));
							UTF8ToUnicode(wchOut,chOut);
							nHeight += (m_nExtentHeight/2);
							m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
						}
					}
				}
				else if(g_nServerType != 4 && uViolationType == DETECT_RESULT_EVENT_GO_FAST)//洛阳项目
				{
					int nExtentHeight = 60;
					nExtentHeight = g_PicFormatInfo.nFontSize*2;
					nWidth += (m_imgSnap->width)*(nIndex%2);
					nHeight = (m_imgSnap->height)*(nIndex/2);

					/*if (1 == g_PicFormatInfo.nSmallViolationPic)
					{
						if(nIndex == 0)
						{
							nIndex = 2;
						}
					}*/

					//时间
					std::string strTime;
					strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
					sprintf(chOut,"时间:%s:%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
				
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

					
					//经过地点,行驶方向
					std::string strDirection = GetDirection(plate.uDirection);
					sprintf(chOut,"地点:%s  方向:%s",m_strLocation.c_str(),strDirection.c_str());
				
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

					//限速值
					sprintf(chOut,"限速:%dkm/h  车速:%dkm/h",plate.uLimitSpeed,plate.uSpeed);

					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
				}
				else if(m_nExtentHeight > 0)//针对江宁项目
				{
					nWidth += (m_imgSnap->width)*(nIndex%2);
					nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex/2);

					//经过地点,行驶方向,车道编号
					std::string strDirection = GetDirection(plate.uDirection);
					sprintf(chOut,"%s %s 车道%d",m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (m_nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(255,255,255));


					//经过时间(第二行)
					std::string strTime;
					
					if(uViolationType == DETECT_RESULT_EVENT_GO_FAST && nIndex == 0)
					{
						//取卡口时间
						strTime = GetTime((pTimeStamp+3)->uTimestamp,0);
						sprintf(chOut,"抓拍时间: %s:%03d",strTime.c_str(),(((pTimeStamp+3)->ts)/1000)%1000);
					}
					else
					{
						strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
						sprintf(chOut,"抓拍时间: %s:%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
					}

					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (m_nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					//红灯时间(第三行)
					if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
					{
						strTime = GetTime(plate.uRedLightBeginTime,0);
						sprintf(chOut,"红灯时间: %s:%03d",strTime.c_str(),plate.uRedLightBeginMiTime);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						nHeight += (m_nExtentHeight/2);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					}
				}
				else
				{
					return;
				}
            }
            else if(7 == g_nServerType)//天津电警
			{
				int nBigHeight = 0;
				CvxText cvText;
				cvText.Init(50);
				CvRect recSmallPic;
				if(g_PicFormatInfo.nForeColor == 6)
				{
					nHeight = 0;
					nBigHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex);
				}
				else
				{
					nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex);
				}

				nHeight += 30;

				nWidth += m_PicFormatInfo.nOffsetX;
				nHeight += m_PicFormatInfo.nOffsetY;

                //经过时间(第一行)
                std::string strTime;
                strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
                sprintf(chOut,"抓拍时间: %s %03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
                memset(wchOut,0,sizeof(wchOut));
                UTF8ToUnicode(wchOut,chOut);
                nHeight += (100/2);

				CvRect dstRt;
				dstRt.x = nWidth;
				dstRt.y = nHeight;
				dstRt.height = nHeight + 7*50;
				dstRt.width = m_imgSnap->width / 2;

				IplImage* pImageText = NULL;
				if(g_PicFormatInfo.nForeColor == 6)
				{
					recSmallPic.x = nWidth;
					recSmallPic.y = nHeight + nBigHeight;
					recSmallPic.height = nHeight + 7*80;
					recSmallPic.width = m_imgSnap->width / 2 + 100;

					pImageText = cvCreateImage(cvSize(recSmallPic.width,recSmallPic.height),8,3);
					cvSet(pImageText, cvScalar(255,255,255) );
				}
				bool bBlack = AverageValue(pImage,dstRt);
				CvScalar color = CV_RGB(255,255,255);
				if(g_PicFormatInfo.nForeColor == 6)
				{
					//if (bBlack == false)//黑色字体
					{
						color = CV_RGB(0,0,0);
					}
				}
				else
				{
					if (bBlack == false)//黑色字体
					{
						color = CV_RGB(0,0,0);
					}
				}

				if(g_PicFormatInfo.nForeColor == 6)
					cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
				else
					cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

                //经过地点
				sprintf(chOut,"路口名称: %s",m_strLocation.c_str());
                memset(wchOut,0,sizeof(wchOut));
                UTF8ToUnicode(wchOut,chOut);
                nHeight += (100/2);
				if(g_PicFormatInfo.nForeColor == 6)
					cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
				else
	                cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),color);

                //行驶方向
                std::string strDirection = GetDirection(plate.uDirection);
				sprintf(chOut,"行驶方向: %s",strDirection.c_str());
                memset(wchOut,0,sizeof(wchOut));
                UTF8ToUnicode(wchOut,chOut);
                nHeight += (100/2);
				if(g_PicFormatInfo.nForeColor == 6)
					cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
				else
	                cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),color);


				//设备编号
				sprintf(chOut,"设备编号: %s",m_strDeviceId.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				nHeight += (100/2);
				
				if(g_PicFormatInfo.nForeColor == 6)
					cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
				else
					cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),color);

				if(nIndex == 2)
				{
                    //红灯时间
                    if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
                    {
                        strTime = GetTime(plate.uRedLightBeginTime,0);
                        sprintf(chOut,"红灯开启时间: %s %03d",strTime.c_str(),plate.uRedLightBeginMiTime);
                        memset(wchOut,0,sizeof(wchOut));
                        UTF8ToUnicode(wchOut,chOut);
                        nHeight += (100/2);
						if(g_PicFormatInfo.nForeColor == 6)
							cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
						else
							cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

						strTime = GetTime(plate.uRedLightEndTime,0);
                        sprintf(chOut,"红灯结束时间: %s %03d",strTime.c_str(),plate.uRedLightEndMiTime);
                        memset(wchOut,0,sizeof(wchOut));
                        UTF8ToUnicode(wchOut,chOut);
                        nHeight += (100/2);
						if(g_PicFormatInfo.nForeColor == 6)
							cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
						else
							cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),color);
                    }
                }

				//防伪码
				sprintf(chOut,"防伪码: %08x",m_nRandCode[nIndex]);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				nHeight += (100/2);
				if(g_PicFormatInfo.nForeColor == 6)
				{
					cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
					PrintColor(pImageText,recSmallPic,pImage);
				}
				else
				{
					cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),color);
				}

				/*char jpg_name[256] = {0};
				sprintf(jpg_name, "./textgcs/%s_%d.jpg", GetTimeCurrent().c_str(),nIndex);
				FILE* fp = NULL;
				fp = fopen(jpg_name, "a");
				if(fp!=NULL)
				{
					printf("********* 3\n");
					SaveImage(pImageText,jpg_name);
					printf("********* 4\n");*/
					if(g_PicFormatInfo.nForeColor == 6)
					{
						if(pImageText != NULL)
						{
							cvReleaseImageHeader(&pImageText);
							pImageText = NULL;
						}
					}
				//}

				cvText.UnInit();
            }
			else
			{}
        }
        else
        {
			//LogTrace("PutVtsText.log", "222=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d=m_nWordPos=%d", \
				g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode, m_nWordPos);

#ifdef WEN_LING_VTS_TEST
			//温岭电警测试图片格式 begin
			if(nIndex == 2)
			{
				CvxText cvText;
				cvText.Init(g_PicFormatInfo.nFontSize);

				nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex);

				nHeight += m_nExtentHeight;
				//nHeight += m_nExtentHeight;

				std::string strTime;
				strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);

				//行驶方向
				std::string strDirection2 = GetDirection2(plate.uDirection);

				sprintf(chOut,"路口名称:%s  方向:由%s  车道编号:%d  抓拍时间:%s ",\
					m_strLocation.c_str(),strDirection2.c_str(), plate.uRoadWayID, strTime.c_str());

				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(255,255,255));
				cvText.UnInit();
			}

			return;
			/////////////////////////////////////end
#endif

			//武汉格式
			if (1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode)//有违章小图 并且 大图1x2叠加
			{
				nWidth = 10+nIndex*400;
				string strText("");

				//路口名称
				sprintf(chOut,"路口名称:%s",m_strLocation.c_str());
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight = 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				//经过时间
				if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
				{
					if(nIndex == 0)
					{     
						timeIndex = 0;
					}

					if(nIndex == 1)
					{     
						timeIndex = 2;
					}
				}
				std::string strTime;
				strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,1);
				sprintf(chOut,"经过时间:%s%03d毫秒",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight += 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				//方向，限速
				std::string strDirection = GetDirection(plate.uDirection);
				sprintf(chOut,"方向:%s  车辆限速:%dkm/h",strDirection.c_str(),plate.uLimitSpeed);//
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight += 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				//速度，违法名称，超速百分比
				int nOverSped = 0;
				if(plate.uLimitSpeed > 0)
				{
					nOverSped = (plate.uSpeed-plate.uLimitSpeed)*100/plate.uLimitSpeed;
				}
				std::string strViolationType = GetViolationType(plate.uViolationType,1);
				sprintf(chOut,"速度:%dkm/h  违法名称:%s  超速百分比:%d%%",plate.uSpeed,strViolationType.c_str(),nOverSped);
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight += 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				//设备编码
				sprintf(chOut,"设备编号:%s",m_strDeviceId.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				nHeight += 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				return;
			}
			else  if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
			{
				#ifdef NO_SEND_KAKOU_DATA
				//违章停车图片格式
				{
						int nWidth = 0;
						int	nHeight = 0;
						wchar_t wchOut[255] = {'\0'};
						char chOut[255] = {'\0'};

						if(nIndex < 3)
						{
							nWidth = (m_imgSnap->width)*((nIndex+1)%2)+(m_imgSnap->width)/2;
							nHeight = (m_imgSnap->height)*((nIndex+1)/2) + m_nExtentHeight;
						}
						else
						{
							nWidth = (m_imgSnap->width)/2;
							nHeight = m_nExtentHeight;
						}

						//经过时间
						std::string strTime;
						strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
						sprintf(chOut,"%s",strTime.c_str());
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

						return;
				}
				#endif

				if(g_nServerType == 13)//上海电警特殊图像格式
				{
					if(nIndex == 0 || nIndex == 2)//叠加黑边区域中的字符
					{
						nWidth += (m_imgSnap->width)*(nIndex%2);
						nHeight = (m_imgSnap->height)*(nIndex/2+1)-5;

						string strText("");
						 //经过时间
						std::string strTime;
						strTime = GetTime(pTimeStamp->uTimestamp,0);
						//strTime = GetTime(plate.uTime,0);
						//地点，车道，时间，红灯时间
						if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
						{
							if(nIndex == 0)
							{
								nHeight -= m_nExtentHeight;
								sprintf(chOut,"%s 车道%5s%02d 速度:%dkm/h",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID,plate.uSpeed);//
								strText += chOut;
								memset(wchOut,0,sizeof(wchOut));
								UTF8ToUnicode(wchOut,(char*)strText.c_str());
								m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
								
								nHeight += m_nExtentHeight;
								strText.clear();
								sprintf(chOut,"%s",strTime.c_str());//
								strText += chOut;
								memset(wchOut,0,sizeof(wchOut));
								UTF8ToUnicode(wchOut,(char*)strText.c_str());
								m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
							}
							else if(nIndex == 2)
							{
								sprintf(chOut,"%s 车道%5s%02d %s 速度:%dkm/h",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uSpeed);//
								strText += chOut;
								memset(wchOut,0,sizeof(wchOut));
								UTF8ToUnicode(wchOut,(char*)strText.c_str());
								m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
							}
						}
						else
						{
							nHeight = (m_imgSnap->height)*(nIndex/2+1)- m_nExtentHeight/2;

							if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
							{
								sprintf(chOut,"%s 车道%5s%02d %s 红灯时间:%d秒",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uSignalTime);//
							}
							else
							{
								/*if(plate.uViolationType == DETECT_RESULT_YELLOW_CAR)
								{
									sprintf(chOut,"%s 车道%5s%02d %s (黄标车禁行)",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID,strTime.c_str());//
								}
								else if(plate.uViolationType == DETECT_RESULT_YELLOW_CRC)
								{
									sprintf(chOut,"%s 车道%5s%02d %s (国I标准汽油车禁行)",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID,strTime.c_str());//
								}
								else*/
								{
									sprintf(chOut,"%s 车道%5s%02d %s",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID,strTime.c_str());//
								}
							}
							strText += chOut;
							memset(wchOut,0,sizeof(wchOut));
							UTF8ToUnicode(wchOut,(char*)strText.c_str());

							m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

							//叠加第二行违章代码
							nHeight += m_nExtentHeight/2;
							strText = GetViolationType(plate.uViolationType,1);
							memset(wchOut,0,sizeof(wchOut));
							UTF8ToUnicode(wchOut,(char*)strText.c_str());
							m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
						}
					}

					if( ((plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST) ||(plate.uViolationType == DETECT_RESULT_YELLOW_CAR) || (plate.uViolationType == DETECT_RESULT_YELLOW_CRC)) && (nIndex == 1))//不叠字
					{
						return;
					}

					if((nIndex >= 1))//叠加图片上的字符
					{
						//CvxText cvText;
						//cvText.Init(g_PicFormatInfo.nFontSize);

						nWidth += (m_imgSnap->width)*(nIndex%2);
						nHeight = (m_imgSnap->height)*(nIndex/2) + m_nExtentHeight;

						nWidth += m_PicFormatInfo.nOffsetX;
						nHeight += m_PicFormatInfo.nOffsetY;

						string strText("");
						/*string nDirection("");//路段方向
						string nstrDirection("");//抓拍方向
						nDirection = g_mvsCommunication.GetstrDirection(plate,nstrDirection);
						if (m_nDetectDirection == 1)
						{
							nstrDirection.clear();
							nstrDirection = nDirection;
						}*/
						std::string strDirection = GetDirection(plate.uDirection);
						if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
						{
							sprintf(chOut,"雷达测速方向:%s",strDirection.c_str());//
						}
						else
						{
							sprintf(chOut,"抓拍方向:%s",strDirection.c_str());//
						}

						strText += chOut;
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

						strText.clear();

						 //经过时间
						std::string strTime;
						if (nIndex == 1)
						{
							strTime = GetTime(pTimeStamp->uTimestamp,0);
							sprintf(chOut,"%s.%03d",strTime.c_str(),((pTimeStamp->ts)/1000)%1000);
						}
						else
						{
							strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
							sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
						}
						strText += chOut;
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						nHeight += (m_nExtentHeight/2);

						m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));


						if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
						{
							strText.clear();
							//限速值
							sprintf(chOut,"限速:%dkm/h",plate.uLimitSpeed);

							strText += chOut;
							memset(wchOut,0,sizeof(wchOut));
							UTF8ToUnicode(wchOut,(char*)strText.c_str());

							nHeight += (m_nExtentHeight/2);

							m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
						}

						//cvText.UnInit();
					}
					return;
				}
				else if(g_nServerType == 14)//北京公交项目特殊图像格式
				{
					if(nIndex == 0)
					{
						CvxText cvText;
						cvText.Init(60);

						nWidth = 10;
						nHeight = 90;

						//经过时间
						std::string strTime;
						strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);

						string strText("");
						std::string strDirection = GetDirection(plate.uDirection);
						sprintf(chOut,"设备编号:%s 地点名称:%s 方向:%s 车道:%d 违法时间:%s:%03d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);//

						strText += chOut;

						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

						cvText.UnInit();
					}

					//每张图片上都要叠加时间
					nWidth = (nIndex%2)*(m_imgSnap->width)+m_imgSnap->width/2-400;
					nHeight = 120+m_nExtentHeight+(nIndex/2)*(m_imgSnap->height-m_nExtentHeight);

					if(nIndex == 3)
					{
						nIndex = 0;
					}

					//经过时间
					std::string strTime;
					strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);

					string strText("");
					sprintf(chOut,"%s:%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);//

					strText += chOut;

					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());

					m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

					return;
				}
				else if(g_nServerType == 21)//深圳交警格式
				{ 
					if(nIndex == 3)
					{
						nWidth = (m_imgSnap->width)+100;
						nHeight = (m_imgSnap->height)+100;

						string strText("");
						sprintf(chOut,"违法地点:%s 设备编号:%s",m_strLocation.c_str(),strDeviceId.c_str());//

						strText += chOut;
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

						strText.clear();
						nHeight += m_nExtentHeight*3/2;
						
						int nRoadDirection = 0;
						VTSParaMap::iterator it_p = m_vtsObjectParaMap.find(plate.uRoadWayID);
						if(it_p != m_vtsObjectParaMap.end())
						{
							nRoadDirection = it_p->second.nRoadDirection;
						}
						string strDriveDir = "直行";
						if(nRoadDirection == 1)
						{
							strDriveDir = "左转";
						}
						else if(nRoadDirection == 2)
						{
							strDriveDir = "左转+直行";
						}
						else if(nRoadDirection == 3)
						{
							strDriveDir = "右转";
						}
						else if(nRoadDirection == 4)
						{
							strDriveDir = "右转+直行";
						}

						sprintf(chOut,"车道:%d 车道类型:%s",plate.uRoadWayID,strDriveDir.c_str());//

						strText += chOut;
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

						strText.clear();
						nHeight += m_nExtentHeight*3/2;
						
						 std::string strViolationType = GetViolationType(plate.uViolationType,1);
						sprintf(chOut,"违法类型:%s",strViolationType.c_str());//

						strText += chOut;
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

						strText.clear();
						nHeight += m_nExtentHeight*3/2;

						std::string strTime;
						strTime = GetTime(plate.uTime,0);

						sprintf(chOut,"违法时间:%s.%03d",strTime.c_str(),plate.uMiTime);//

						strText += chOut;
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
					}
					else
					{
						nWidth += (m_imgSnap->width)*(nIndex%2);
						nHeight = (m_imgSnap->height)*(nIndex/2+1)-m_nExtentHeight/3;

						 //经过时间
						std::string strTime;
						strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);

						string strText("");
						sprintf(chOut,"%s 设备编号:%s 车道:%d 时间:%s.%03d 第%d幅",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID,strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000,nIndex+1);//

						strText += chOut;
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());

						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
					}
					return;
				}
			}

			int nBaseWidth = m_imgSnap->width;
			int nBaseHeight = m_imgSnap->height;
			#ifdef DSP_500W_TEST
			nBaseWidth = m_imgDestSnap2->width;
			nBaseHeight = m_imgDestSnap2->height;
			#endif


            if(pImage->width > nBaseWidth)
            {
                nWidth += (nBaseWidth)*(nIndex%2);
                if(m_nWordPos == 0)
                {
                    nHeight = (nBaseHeight)*(nIndex/2+1) - m_nExtentHeight;
                }
                else
                {
                    nHeight = (nBaseHeight)*(nIndex/2);
                }
            }
            else
            {
                if(pImage->height == nBaseHeight)
                {
                    if(m_nWordPos == 0)
                    nHeight = nBaseHeight - m_nExtentHeight;
                    else
                    nHeight = 0;
                }
                else
                {
                    if(m_nWordPos == 0)
                    nHeight = (nBaseHeight)*(nIndex+1) - m_nExtentHeight;
                    else
                    nHeight = (nBaseHeight)*(nIndex);
                }
            }

			//LogTrace("PutVtsText.log", "====nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);

			string strText("");

			
			{
				std::string strDirection = GetDirection(plate.uDirection);
				if (10 == g_nServerType)
					sprintf(chOut,"设备编号:%s  违法地点:%s  方向:%s  ",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str());//
				else if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)//有违章小图 并且 大图2x2叠加
					sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ", strDeviceId.c_str(), m_strLocation.c_str(), strDirection.c_str());
				else
					sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str());//
			}
			strText += chOut;

            //车道编号
            if(g_PicFormatInfo.nRoadIndex == 1)
            {
                sprintf(chOut,"车道编号:%d  ",plate.uRoadWayID);//
                string strTmp(chOut);
                strText += strTmp;
            }

			//车牌号码
			if(g_PicFormatInfo.nCarNum == 1)
			{
				std::string strCarNum(plate.chText);
				sprintf(chOut,"车牌号码:%s  ",strCarNum.c_str());
				string strTmp(chOut);
				strText += strTmp;
			}

			//车速
			if(g_PicFormatInfo.nCarSpeed == 1 && 1 != g_PicFormatInfo.nSmallViolationPic)
			{
				//行驶速度
				sprintf(chOut,"速度:%dkm/h  ",plate.uSpeed);
				string strTmp(chOut);
				strText += strTmp;
			}

			if ((1 != g_PicFormatInfo.nSmallViolationPic)&&(g_nServerType == 12))
			{
				//LogTrace("PutVtsText.log", "====nWidth=%d=nHeight=%d==nIndex=%d=g_PicFormatInfo.nSmallViolationPic=%d", nWidth, nHeight, nIndex, g_PicFormatInfo.nSmallViolationPic);

				//乐清宝康中心端 黑条中文字只显示1行
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				if(pImage->width < 1000)
				{
					nHeight += 20;
				}
				else
				{
					nHeight += (m_nExtentHeight/2);
				}

				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				/////////////////////////////////////////////第二行
				strText.clear();
			}

			if(g_PicFormatInfo.nSpeedLimit == 1)//温岭卡口
			{
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				if(pImage->width < 1000)
				{
					nHeight += 20;
				}
				else
				{
					nHeight += (m_nExtentHeight/2);
				}
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
				strText.clear();
			}

            //经过时间
            std::string strTime;
            strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);

			/*if(uViolationType == DETECT_RESULT_EVENT_GO_FAST)
			{
				if (1 == g_PicFormatInfo.nSmallViolationPic)
				{
					if(nIndex == 0)
					{
						timeIndex = 2;
					}
				}
			}*/

			if (10 == g_nServerType)
				sprintf(chOut,"违法时间:%s:%03d  ",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
			else
			{
				if (1 == g_PicFormatInfo.nCarBrand) //浏阳事件违章格式
				{
					nHeight = nHeight + m_nExtentHeight / 2;
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					//第二行
					//strText = "";
					strText.clear();
					memset(chOut,0,sizeof(chOut));
				}
				//else 
					sprintf(chOut,"抓拍时间:%s:%03d  ",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
			}

            {
                string strTmp(chOut);
                strText += strTmp;
            }

			if(g_PicFormatInfo.nSpeedLimit == 1)//温岭卡口
			{
				if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
				{
					int nOverSped = 0;
					if(plate.uLimitSpeed > 0)
					{
						nOverSped = (plate.uSpeed-plate.uLimitSpeed)*100/plate.uLimitSpeed;
					}

					//行驶速度
					sprintf(chOut,"速度:%dkm/h  限速:%dkm/h  超速百分比:%d%% ",plate.uSpeed,plate.uLimitSpeed,nOverSped);
					string strTmp(chOut);
					strText += strTmp;
				}
			}

            //红灯时间
            if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
            {
				if(pSignalTimeStamp != NULL && 1 != g_PicFormatInfo.nSmallViolationPic)
				{
					
					strTime = GetTime(plate.uRedLightBeginTime,0);
					{
						sprintf(chOut,"红灯开始时间:%s:%03d  ",strTime.c_str(),plate.uRedLightBeginMiTime);
						string strTmp(chOut);
						strText += strTmp;
					}

					if(g_nPicSaveMode == 1)
					{
						 strTime = GetTime(plate.uRedLightEndMiTime,0);
						 sprintf(chOut,"红灯结束时间:%s:%03d ",strTime.c_str(),plate.uRedLightEndMiTime);
						 string strTmp(chOut);
						 strText += strTmp;
					}
				}
            }

            if(g_PicFormatInfo.nViolationType == 1)
            {
#ifdef MATCH_LIU_YANG_DEBUG
				WuXiVtsText(plate, strText);
#else
               //违章行为
                if(plate.uViolationType != 0)
                {
                    std::string strViolationType = GetViolationType(plate.uViolationType,1);


					if (10 == g_nServerType)
						sprintf(chOut,"违法行为:%s",strViolationType.c_str());
					else
					{
						if (1 != g_PicFormatInfo.nCarBrand)
						{
							sprintf(chOut,"违章行为:%s",strViolationType.c_str());
						}
						else
						{
							UINT32 uVtsCode;
							std::string strViolationType = "";
							bool bGet = m_pCDspDataProcess->GetVtsCode(plate.uViolationType, uVtsCode, strViolationType);
							if(bGet)
							{
								sprintf(chOut,"违法代码:%d ", uVtsCode);
								std::string strTemp(chOut);
								strText += strTemp;
							
								memset(chOut,0,sizeof(chOut));
								sprintf(chOut,"违法行为:%s ",strViolationType.c_str());
								std::string strTemp1(chOut);
								strText += strTemp1;

								//防伪码
								strTemp = "";
								m_nRandCode[0] = g_RoadImcData.GetRandCode();
								memset(chOut,0,sizeof(chOut));
								sprintf(chOut,"防伪码：%08x ", m_nRandCode[0]);

							}
							else
							{
								strViolationType = GetViolationType(plate.uViolationType,1);
								sprintf(chOut,"违章行为:%s",strViolationType.c_str());
							}
						}						
					}
					/*}*/

					string strTmp(chOut);
					strText += strTmp;
				}                
#endif

#ifdef VTS_TEST_SEQ
					//LogNormal(" VtsSeq: [%lld] index:[%d] plate:%s ts:%u",\
						((pTimeStamp+timeIndex)->uSeq), timeIndex, plate.chText, plate.uTime);
					//记录帧号
					sprintf(chOut," VtsSeq: [%lld] index:[%d] ", \
					((pTimeStamp+timeIndex)->uSeq), timeIndex);
					string strTmpTest(chOut);
					strText += strTmpTest;			
#endif
            }

			if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)//有违章小图 并且 大图2x2叠加
			{
				//文字一行显示
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				if(pImage->width < 1000)
				{
					nHeight += 20;
				}
				else
				{
					nHeight += (m_nExtentHeight/2);
				}

				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				/////////////////////////////////////////////第二行
				strText.clear();
			}

            memset(wchOut,0,sizeof(wchOut));
            UTF8ToUnicode(wchOut,(char*)strText.c_str());
            nWidth = 10;
            if(pImage->width > nBaseWidth)
            {
                nWidth += (nBaseWidth)*(nIndex%2);
            }

            if(pImage->width < 1000)
            {
                nHeight += 20;
            }
            else
            {
                nHeight += (m_nExtentHeight/2);
            }
            m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			/*
			//针对台州黄岩电警项目
			if(g_nServerType == 0)
			{
				//图片上叠加地点
				sprintf(chOut,"%s",m_strLocation.c_str());

				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);

				int nLen = (m_strLocation.size())*( g_PicFormatInfo.nFontSize + 10) / 3;

				nWidth = (nBaseWidth * ((nIndex%2)+1)) - nLen - 10;
				nHeight = (nBaseHeight * (nIndex/2+1)) - m_nExtentHeight - 30;

				printf("vts11 nIndex:%d nWidth:%d, nHeight:%d nLen:%d ",\
					nIndex, nWidth, nHeight, nLen);

				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

				printf("vts22 nIndex:%d nWidth:%d, nHeight:%d nLen:%d ",\
					nIndex, nWidth, nHeight, nLen, strlen(chOut));
			}
			*/
        }
    }
    else //空白区域
    {
		//LogTrace("PutVtsText.log", "333=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d", \
			g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode);

        CvxText cvText;
        cvText.Init(120);

        wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nWidth = pImage->width/2+50;
        int nHeight = pImage->height/2+400;

        if(g_nVtsPicMode > 1)
        {
            nHeight = pImage->height*2/3+400;
        }

		if (10 != g_nServerType)//非鞍山交警的情况
		{
			sprintf(chOut,"路口编号: %s",strDeviceId.c_str());//
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
		}

        nHeight += 120;
        //经过地点
		if (10 == g_nServerType)
		{
			int num = 3 * 8;
			for (int i = 0; i < m_strLocation.size(); i += num)
			{
				if (i == 0)
					sprintf(chOut,"违法地点: %s", m_strLocation.substr(i, num));
				else
					sprintf(chOut,"        %s", m_strLocation.substr(i, num));

				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

				nHeight += 120;
			}
		}
		else
		{
			sprintf(chOut,"路口名称: ");
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

			nHeight += 120;
			//经过地点
			sprintf(chOut,"%s",m_strLocation.c_str());
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

			nHeight += 120;

		}

		 //行驶方向
		memset(wchOut,0,sizeof(wchOut));
		std::string strDirection = GetDirection(plate.uDirection);
		sprintf(chOut,"车道方向: %s",strDirection.c_str());
		UTF8ToUnicode(wchOut,chOut);
		cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

		nHeight += 120;

        //经过时间
        std::string strTime = GetTime(plate.uTime);
		if (10 == g_nServerType)
		{
			sprintf(chOut,"违法时间: %s",strTime.c_str());
		}
		else
		{
			sprintf(chOut,"抓拍时间: %s",strTime.c_str());
		}
        memset(wchOut,0,sizeof(wchOut));
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

        /*nHeight += 120;
        //车牌号码
        std::string strCarNum(plate.chText);
        memset(wchOut,0,sizeof(wchOut));
        sprintf(chOut,"车牌号码: %s",strCarNum.c_str());
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));*/

        cvText.UnInit();
    }
}


/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CRoadCarnumDetect::PutTextOnComposeImage(IplImage* pImage,RECORD_PLATE plate,int nOrder)
{
	string strDeviceId;
	if(m_strDeviceId.size() > 0)
	{
		strDeviceId = m_strDeviceId;
	}
	else
	{
		strDeviceId = g_strDetectorID;
	}

	if(m_nExtentHeight <= 0)
	{
		return;
	}

    wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    int nStartX = 0;
    int nWidth = 10;
    int nHeight = 0;
    if( m_nWordPos== 0)
    {
        nHeight = m_imgSnap->height - m_nExtentHeight;
    }

    nStartX = nWidth;

	string strText="";

    //设备编号,经过地点,行驶方向,经过时间
    std::string strDirection = GetDirection(plate.uDirection);
    std::string strTime = GetTime(plate.uTime,0);
	
	{
		if(g_nVtsPicMode == 5)
		{
			//遵义格式
			{
				sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  时间:%s.%03d  车道号:%d  速度值:%dkm/h",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime,plate.uRoadWayID,plate.uSpeed);//
			}
		}
		else
		{
			if(g_nServerType == 13)
			{

				nHeight = m_nExtentHeight/2;

				/*string nDirection("");//路段方向
				string nstrDirection("");//抓拍方向
				nDirection = g_mvsCommunication.GetstrDirection(plate,nstrDirection);
				if (m_nDetectDirection == 1)
				{
					nstrDirection.clear();
					nstrDirection = nDirection;
				}*/
				std::string strDirection = GetDirection(plate.uDirection);
				sprintf(chOut,"抓拍方向:%s",strDirection.c_str());//
				string strText(chOut);

				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());

				nWidth += m_PicFormatInfo.nOffsetX;
				nHeight += m_PicFormatInfo.nOffsetY;

				m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

				strText.clear();
				std::string strTime = GetTime(plate.uTime,0);
				sprintf(chOut,"%s.%03d",strTime.c_str(),plate.uMiTime);//
				strText += chOut;

				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());

				nHeight += (m_nExtentHeight/2);
				m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
			}
			else if(g_nServerType == 0)
			{			
				//LogNormal("--m_nSmallPic:%d plate.place=%s \n", m_nSmallPic, plate.chPlace);
				//LogNormal("--g_PicFormatInfo.nCarNum:%d-m_nSaveImageCount:%d-\n", g_PicFormatInfo.nCarNum, m_nSaveImageCount);

				//sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  时间:%s.%03d  车道编号:%d",\
					//g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime, plate.uRoadWayID);//
#ifdef WEN_LING_VTS_TEST
				std::string strDirection2 = GetDirection2(plate.uDirection);
				sprintf(chOut,"路口名称:%s 方向:由%s 车道编号:%d 抓拍时间:%s 速度值:%dkm/h",\
					m_strLocation.c_str(),strDirection2.c_str(), plate.uRoadWayID,strTime.c_str(),plate.uSpeed);//
#else
			//叠加安全带标志
				if(m_nDetectKind&DETECT_FACE)
				{
					if(plate.uBeltResult == 0)
					{
						sprintf(chOut,"安全带:有");
					}
					else if(plate.uBeltResult == 1)
					{
						sprintf(chOut,"安全带:无");
					}
					else if(plate.uBeltResult == 2)
					{
						sprintf(chOut,"安全带:有");
					}
					string strTmp(chOut);
					sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  时间:%s.%03d  车道编号:%d  %s",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime, plate.uRoadWayID,strTmp.c_str());//
				}
				else
				{
					//车牌号码
					if(g_PicFormatInfo.nCarNum == 1)
					{
						std::string strCarNum(plate.chText);

						if(1 != g_PicFormatInfo.nCarBrand) //不需要叠加车标
						{
							sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  时间:%s.%03d  车道编号:%d 车牌号码:%s ",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime, plate.uRoadWayID,strCarNum.c_str());//	
						}
						else
						{
							//浏阳卡口格式
							sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  时间:%s.%03d  车道编号:%d ",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime, plate.uRoadWayID);
							nHeight = nHeight + m_nExtentHeight / 2 ;
							memset(wchOut,0,sizeof(wchOut));
							UTF8ToUnicode(wchOut,chOut);
							m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

							//车辆类型
							std::string strCarType;
							if(plate.uType == SMALL_CAR)
							{
								strCarType = "小型";
							}
							else if(plate.uType == MIDDLE_CAR)
							{
								strCarType = "中型";
							}
							else if(plate.uType == BIG_CAR)
							{
								strCarType = "大型";
							}
							else
							{
								strCarType = "";
							}
							//车身颜色
							std::string strCarColor = GetObjectColor(plate.uCarColor1);
							//车牌颜色
							std::string strPlateColor = GetPlateColor(plate.uColor);
							std::string strCarBrand = "";
#ifdef GLOBALCARLABEL
							CBrandSusection BrandSub;
							UINT32 uCarLabel = plate.uCarBrand + plate.uDetailCarBrand;
							strCarBrand = BrandSub.GetCarLabelText(uCarLabel).c_str();
#else
							if(plate.uCarBrand < 131)
							{
								strCarBrand = g_strCarLabel[plate.uCarBrand];
							}
							else
							{
								LogNormal("车标类型有误！");
							}
#endif

							sprintf(chOut,"车牌号码:%s 限速:%dkm/h  车速:%dkm/h 车身颜色: %s 车牌颜色:%s 车辆类型:%s 车标：%s", 
								strCarNum.c_str(),plate.uLimitSpeed,plate.uSpeed,strCarColor.c_str(),strPlateColor.c_str(),strCarType.c_str(),strCarBrand.c_str());												
						}
					}
					else
					{
						sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  时间:%s.%03d  车道编号:%d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime, plate.uRoadWayID);
					}
				}
#endif //#ifdef WEN_LING_VTS_TEST
			}
			else
			{
				sprintf(chOut,"设备编号:%s    地点名称:%s    方向:%s    时间:%s.%03d",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime);//
			}
		}
	}
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);

	if(m_nSaveImageCount == 2)
	{
		nHeight += 90;
		m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		 //第一时间
		if(m_imgSnap->width > 2000)
		nWidth = 800;
		else
		nWidth = 400;

		nHeight += 100;
		if( m_nWordPos== 0)
		{
			nHeight = 100;
		}
		if(nOrder == 0)
		{
			sprintf(chOut,"%s.%03d",strTime.c_str(),plate.uMiTime);
		}
		else
		{
			std::string strTime2 = GetTime(plate.uTime2,0);
			sprintf(chOut,"%s.%03d",strTime2.c_str(),plate.uMiTime2);
		}
		nWidth += m_PicFormatInfo.nOffsetX;
		nHeight += m_PicFormatInfo.nOffsetY;

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));


		//第二时间
		if(m_imgSnap->width > 2000)
		nWidth = 3200;
		else
		nWidth = 2000;
		if(nOrder == 0)
		{
			std::string strTime2 = GetTime(plate.uTime2,0);
			sprintf(chOut,"%s.%03d",strTime2.c_str(),plate.uMiTime2);
		}
		else
		{
			sprintf(chOut,"%s.%03d",strTime.c_str(),plate.uMiTime);
		}
		nWidth += m_PicFormatInfo.nOffsetX;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

		if(m_nSmallPic == 1)
		{
			//小图时间
			if(m_imgSnap->width > 2000)
			nWidth = 5400;
			else
			nWidth = 3400;
			sprintf(chOut,"%s.%03d",strTime.c_str(),plate.uMiTime);
			nWidth += m_PicFormatInfo.nOffsetX;
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
		}
	}
	else
	{
		if(g_nServerType == 13)
		{
			string strText;
			sprintf(chOut,"%s 车道:%5s%02d",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID);//
			strText += chOut;

			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());

			nWidth = 10;

			nHeight = m_imgSnap->height - m_nExtentHeight/2-5;
			m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			strText.clear();
			sprintf(chOut,"%s",strTime.c_str());//
			strText += chOut;

			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());

			nHeight += (m_nExtentHeight/2)-5;
			m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
		}
		else
		{
#ifdef WEN_LING_VTS_TEST
			nHeight += (m_nExtentHeight);
#else
			nHeight += (m_nExtentHeight/2);
#endif	
			m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
		}
	}
}
//设定打字颜色 
void CRoadCarnumDetect::PrintColor(IplImage* pSmallPic,CvRect recSmallPic, IplImage* pBigPic) 
{
	assert(pSmallPic->height > 0 && pSmallPic->height < pBigPic->height);
	assert(pSmallPic->width > 0 && pSmallPic->width <= pBigPic->width);

	assert(recSmallPic.x >= 0 && recSmallPic.x + recSmallPic.width <= pBigPic->width);
	assert(recSmallPic.y >= 0 && recSmallPic.y + recSmallPic.height <= pBigPic->height);

	int nTh = 150;//设定亮度阈值

	int * nId_x = new int[pSmallPic->width];
	int * nId_y = new int[pSmallPic->height];
	int * nId_x_temp = new int[pSmallPic->width];
	int * nId_y_temp = new int[pSmallPic->height];

	for (int j = 0; j < pSmallPic->width; j++)
	{
		nId_x[j] = 0;
		nId_x_temp[j] = 0;
	}
	for (int j = 0; j < pSmallPic->height; j++)
	{
		nId_y[j] = 0;
		nId_y_temp[j] = 0;
	}

	IplImage *pSmallGrayimg = cvCreateImage(cvGetSize(pSmallPic),8,1);
	cvCvtColor(pSmallPic,pSmallGrayimg,CV_RGB2GRAY);

	//取字体纵向坐标
	int nOrder_y = 0;
	for (int j = 0; j < pSmallGrayimg->height; j++)
	{
		int ncolor = 0;
		uchar *pRect = (uchar *)pSmallGrayimg->imageData+pSmallGrayimg->widthStep*j;
		for (int i = 0; i < pSmallGrayimg->width; i++,pRect++)
		{
			if ( *pRect < 10 )//小图中黑色字体
			{
				ncolor++;
			}
		}
		if ( ncolor > 0 )
		{
			nId_y_temp[j] = j;//存每行像素变化值
		}
	}

	//存纵向字符起始点位置
	for (int j = 0; j < pSmallGrayimg->height; j++)
	{
		if ( j ==0 && nId_y_temp[j] > 0 )
		{
			nId_y[nOrder_y++] = j;//存纵向字符起点位置(边界点)
		}
		if ( j < pSmallGrayimg->height - 1 )
		{
			if ( nId_y_temp[j+1] > 0 && nId_y_temp[j] == 0 )
			{
				nId_y[nOrder_y++] = j + 1;//存纵向字符起点位置
			}
		}
		if ( j >= 1 )
		{
			if ( nId_y_temp[j-1] > 0 && nId_y_temp[j] == 0 )
			{
				nId_y[nOrder_y++] = j - 1;//存纵向字符终点位置
			}
		}
		if ( j == pSmallGrayimg->height - 1 && nId_y_temp[j] > 0 )
		{
			nId_y[nOrder_y++] = j;//存纵向字符终点位置(边界点)
		}
	}

	//取每行字符，计算横向字符位置与像素
	int nOrder_x = 0;
	for (int k = 0; k < nOrder_y && k + 1 < nOrder_y; k = k + 2)
	{
		//初始化每行横向信息
		nOrder_x = 0;
		for (int j = 0; j < pSmallPic->width; j++)
		{
			nId_x[j] = 0;
			nId_x_temp[j] = 0;
		}

		//取字体横向坐标
		for (int i = 0; i < pSmallGrayimg->width; i++)
		{
			int ncolor = 0;
			for (int j = nId_y[k]; j < nId_y[k+1]; j++)
			{
				if ( ((uchar*)(pSmallGrayimg->imageData + pSmallGrayimg->widthStep * j))[i] < 10 )
				{
					ncolor++;
				}
			}
			if ( ncolor > 0 )
			{
				nId_x_temp[i] = i;
			}
		}
		//存横向字符起始点位置
		for (int j = 0; j < pSmallGrayimg->width; j++)
		{
			if ( j == 0 && nId_x_temp[j] > 0 )
			{
				nId_x[nOrder_x++] = j;//存横向字符起点位置（边界点）
			}
			if ( j < pSmallGrayimg->width - 1 )
			{
				if ( nId_x_temp[j+1] > 0 && nId_x_temp[j] == 0 )
				{
					nId_x[nOrder_x++] = j + 1;//存横向字符起点位置
				}
			}
			if ( j >= 1 )
			{
				if ( nId_x_temp[j-1] > 0 && nId_x_temp[j] == 0 )
				{
					nId_x[nOrder_x++] = j - 1;//存横向字符终点位置
				}
			}
			if ( j == pSmallGrayimg->width - 1 && nId_x_temp[j] > 0 )
			{
				nId_x[nOrder_x++] = j;//存横向字符终点位置（边界点）
			}
		}

		//根据每个字体背景颜色赋值给原图字样
		for (int m = 0; m < nOrder_x && m + 1 < nOrder_x; m = m + 2)
		{
			//字符背景颜色比对
			int nblack = 0, nwhite = 0;

			//合并距离较小字符
			if ( m + 3 < nOrder_x )//判断后面是否有一对字符
			{
				if ( nId_x[m+2] - nId_x[m+1] < 5 )//最小字符间隔
				{
					nId_x[m+1] = nId_x[m+3];
					nId_x[m+2] = nId_x[m+1];
				}
			}

			CvRect rect;
			rect.x = nId_x[m] + recSmallPic.x;
			rect.y = nId_y[k] + recSmallPic.y;
			rect.width = nId_x[m+1] - nId_x[m];
			rect.height = nId_y[k+1] - nId_y[k];

			if (rect.height <= 0 && rect.width <= 0)
			{
				continue;
			}

			cvSetImageROI(pBigPic, rect);
			CvScalar color = cvAvg(pBigPic);//算ROI平均像素
			cvResetImageROI(pBigPic);

			double fmean = (color.val[0] + color.val[1] + color.val[2])/3;

			if ( fmean > nTh )
			{
				nblack = 1;//字符区域偏亮
			}
			else
			{
				nwhite = 1;//字符区域偏暗
			}

			//颜色赋值
			for (int j = nId_y[k]; j < nId_y[k+1]; j++)
			{
				for (int i = nId_x[m]; i < nId_x[m+1]; i++)
				{
					if ( ((uchar*)(pSmallGrayimg->imageData + pSmallGrayimg->widthStep * j))[i] < 10 )
					{
						int nidx = i + recSmallPic.x;
						int nidy = j + recSmallPic.y;

						if ( nblack > nwhite )
						{
							((uchar*)(pBigPic->imageData + pBigPic->widthStep * nidy))[nidx*3] = 0;
							((uchar*)(pBigPic->imageData + pBigPic->widthStep * nidy))[nidx*3+1] = 0;
							((uchar*)(pBigPic->imageData + pBigPic->widthStep * nidy))[nidx*3+2] = 0;
						}
						else
						{
							((uchar*)(pBigPic->imageData + pBigPic->widthStep * nidy))[nidx*3] = 255;
							((uchar*)(pBigPic->imageData + pBigPic->widthStep * nidy))[nidx*3+1] = 255;
							((uchar*)(pBigPic->imageData + pBigPic->widthStep * nidy))[nidx*3+2] = 255;
						}
					}
				}
			}
		}
	}

	if (nId_x)
	{
		delete [] nId_x;
		nId_x =NULL;
		delete [] nId_x_temp;
		nId_x_temp =NULL;
	}
	if (nId_y)
	{
		delete [] nId_y;
		nId_y =NULL;
		delete [] nId_y_temp;
		nId_y_temp =NULL;
	}
	cvReleaseImage(&pSmallGrayimg);
}


/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CRoadCarnumDetect::PutTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp)
{
	//LogNormal("m_nExtentHeight=%d",m_nExtentHeight);

	//LogNormal("PutTextOnImage Seq: [%d]", plate.uSeq);
	string strDeviceId;
	if(m_strDeviceId.size() > 0)
	{
		strDeviceId = m_strDeviceId;
	}
	else
	{
		strDeviceId = g_strDetectorID;
	}
	
	if (g_nServerType == 15)
	{
		/*CvxText McvText;
		McvText.Init(m_nFontSize);*/
		wchar_t wchOut[255] = {'\0'};
		char chOut[255] = {'\0'};

		int nWidth = 0;
		int nHeight = 0;
		int nDistance = m_nFontSize;

		nWidth += m_PicFormatInfo.nOffsetX;
		nHeight += m_PicFormatInfo.nOffsetY;

		CvRect dstRt;

		dstRt.x = 0;
		dstRt.y = 0;
		dstRt.x += m_PicFormatInfo.nOffsetX;
		dstRt.y += m_PicFormatInfo.nOffsetY;
		
		if (dstRt.x > (m_imgSnap->width - m_BlackFrameWidth))
		{
			dstRt.x = 0;
			nWidth -= m_PicFormatInfo.nOffsetX;
			nHeight -= m_PicFormatInfo.nOffsetY;
		}
		

		dstRt.height = nDistance*3;//三行字
		dstRt.width = m_BlackFrameWidth;
		


		cvSetImageROI(pImage,dstRt);
		cvSet(pImage,cvScalar(0,0,0));
		cvResetImageROI(pImage);



		//设备编号
		sprintf(chOut,"设备编号:%s",strDeviceId.c_str());
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight += nDistance;
		m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		//经过地点,行驶方向
		std::string strDirection = GetDirection(plate.uDirection);
		sprintf(chOut,"地点:%s  方向:%s",m_strLocation.c_str(),strDirection.c_str());

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight += nDistance;
		m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		//时间
		std::string strTime;
		strTime = GetTime(plate.uTime,0);
		sprintf(chOut,"时间:%s.%03d",strTime.c_str(),plate.uMiTime);

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		nHeight += nDistance;
		m_nXingTaiCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		//McvText.UnInit();
		return;

	}
	else
	{
		if((m_nExtentHeight <= 0) &&(m_nWordOnPic == 0))
		{
			return;
		}
	}
    wchar_t wchOut[1024] = {'\0'};
    char chOut[1024] = {'\0'};

    int nStartX = 0;
    int nWidth = 10;
    int nHeight = 0;
	
	int nBaseWidth = m_imgSnap->width;
	int nBaseHeight = m_imgSnap->height;
	#ifdef DSP_500W_TEST
	nBaseWidth = m_imgDestSnap2->width;
	nBaseHeight = m_imgDestSnap2->height;
	#endif


    if(pImage->width > nBaseWidth)
    {
        nWidth += (nBaseWidth)*(nIndex%2);
        if(m_nWordPos == 0)
        {
            nHeight = (nBaseHeight)*(nIndex/2+1) - m_nExtentHeight;
        }
        else
        {
            nHeight = (nBaseHeight)*(nIndex/2);
        }
    }
    else
    {
        if(pImage->height == nBaseHeight)
        {
            if(m_nWordPos == 0)
            nHeight = nBaseHeight - m_nExtentHeight;
            else
            nHeight = 0;
        }
        else
        {
            if(m_nWordPos == 0)
            nHeight = (nBaseHeight)*(nIndex+1) - m_nExtentHeight;
            else
            nHeight = (nBaseHeight)*(nIndex);
        }
    }
    nStartX = nWidth;


	if(g_nServerType == 7)
	{
		std::string strDirection = GetDirection(plate.uDirection);
		std::string strCarNum(plate.chText);
		std::string strTime;
		strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
		
		if(g_PicFormatInfo.nCarNum == 1)
		{
			int nRandCode = g_RoadImcData.GetRandCode();

			if (g_PicFormatInfo.nPicFlag == 1)
			{
				sprintf(chOut,"抓拍时间:%s %03d 路口名称:%s 行驶方向:%s 车道:%d 车牌:%s 设备编号:%s 防伪码:%08x",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strCarNum.c_str(), m_strDeviceId.c_str(),nRandCode);//
			}
			else
			{
				sprintf(chOut,"抓拍时间:%s %03d 路口名称:%s 行驶方向:%s 车道:%d 车牌:%s 设备编号:%s",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strCarNum.c_str(), m_strDeviceId.c_str());//
			}
		}
		else
		{
			sprintf(chOut,"抓拍时间:%s %03d 路口名称:%s 行驶方向:%s 车道:%d 设备编号:%s",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID, m_strDeviceId.c_str());//
		}
		string strText(chOut);

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		if(pImage->width < 1000)
		{
			nHeight += 20;
		}
		else
		{
			nHeight += (m_nExtentHeight/2);
		}
		
		//重设图片区域
		if(g_PicFormatInfo.nWordOnPic == 1)
		{			
			nHeight = 30;
			//printf("--**********--nWidth:%d, nHeight:%d \n", nWidth, nHeight);
		}

		nWidth += m_PicFormatInfo.nOffsetX;
		nHeight += m_PicFormatInfo.nOffsetY;

		CvRect dstRt;
		dstRt.x = nWidth;
		dstRt.y = nHeight;
		dstRt.height = nHeight + 50;
		dstRt.width = m_imgSnap->width;
		bool bBlack = AverageValue(pImage,dstRt);
		CvScalar color = CV_RGB(255,255,255);

		if (bBlack == false)//黑色字体
		{
			color = CV_RGB(0,0,0);
		}
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		return;
	}
	else if(g_nServerType == 10)
	{
		std::string strDirection = GetDirection(plate.uDirection);
		std::string strCarNum(plate.chText);
		std::string strTime;
		strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);

		if(g_PicFormatInfo.nCarNum == 1)
		{
			sprintf(chOut,"抓拍时间:%s %03d 路口名称:%s 行驶方向:%s 车道:%d 车牌:%s 设备编号:%s",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strCarNum.c_str(),strDeviceId.c_str());//
		}
		else
		{
			sprintf(chOut,"抓拍时间:%s %03d 路口名称:%s 行驶方向:%s 车道:%d 设备编号:%s",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strDeviceId.c_str());//
		}
		string strText(chOut);

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		if(pImage->width < 1000)
		{
			nHeight += 20;
		}
		else
		{
			nHeight += (m_nExtentHeight/2);
		}

		nWidth += m_PicFormatInfo.nOffsetX;
		nHeight += m_PicFormatInfo.nOffsetY;

		CvScalar color = CV_RGB(255,255,255);
		if(g_PicFormatInfo.nForeColor == 1)//黑色字体
		{
			color = CV_RGB(0,0,0);
		}
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		return;
	}
	else if(g_nServerType == 13)
	{
		/*CvxText cvText;
        cvText.Init(m_nExtentHeight/2);*/
		nHeight = m_nExtentHeight;

		/*string nDirection("");//路段方向
		string nstrDirection("");//抓拍方向
		nDirection = g_mvsCommunication.GetstrDirection(plate,nstrDirection);
		if (m_nDetectDirection == 1)
		{
			nstrDirection.clear();
			nstrDirection = nDirection;
		}*/
		std::string strDirection = GetDirection(plate.uDirection);
		sprintf(chOut,"抓拍方向:%s",strDirection.c_str());//
		string strText(chOut);

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());

		nWidth += m_PicFormatInfo.nOffsetX;
		nHeight += m_PicFormatInfo.nOffsetY;

		/*cvText*/m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

		strText.clear();
		std::string strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
		sprintf(chOut,"%s.%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);//
		strText += chOut;

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());

		nHeight += (m_nExtentHeight/2);
		/*cvText*/m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

		strText.clear();
		sprintf(chOut,"%s 车道:%5s%02d",m_strLocation.c_str(),strDeviceId.c_str(),plate.uRoadWayID);//
		strText += chOut;

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());

		nWidth = 10;

		nHeight = m_imgSnap->height - m_nExtentHeight/2;
		/*cvText*/m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		strText.clear();
		sprintf(chOut,"%s",strTime.c_str());//
		strText += chOut;

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());

		nHeight += (m_nExtentHeight/2);
		/*cvText*/m_nJiaoJinCvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

		//cvText.UnInit();

		return;
	}
	else if(g_nServerType == 0 && g_PicFormatInfo.nWordLine == 3)//杨浦格式
	{
		std::string strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
		sprintf(chOut,"设备编号:%s  经过时刻:%s  抓拍地点:%s",strDeviceId.c_str(),strTime.c_str(),m_strLocation.c_str());
		string strText(chOut);

		CvScalar color = CV_RGB(255,255,255);
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += ((m_nExtentHeight/3)-5);
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);
		
		std::string strDirection = GetDirection(plate.uDirection);
		std::string strCarNum(plate.chText);
		std::string strPlateColor = GetPlateColor(plate.uColor);
		std::string strCarColor = GetObjectColor(plate.uCarColor1);
		sprintf(chOut,"抓拍方向:%s  抓拍车道:%d  号牌号码:%s  号牌颜色:%s  车身颜色:%s",strDirection.c_str(),plate.uRoadWayID,strCarNum.c_str(),strPlateColor.c_str(),strCarColor.c_str());
		
		strText.clear();
		strText += chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += ((m_nExtentHeight/3)-5);
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);
		
		std::string strCarType;
		if(plate.uType == SMALL_CAR)
        {
            strCarType = "小型";
        }
        else if(plate.uType == MIDDLE_CAR)
        {
            strCarType = "中型";
        }
        else if(plate.uType == BIG_CAR)
        {
            strCarType = "大型";
        }
        else
        {
            strCarType = "";
        }
		sprintf(chOut,"车辆类型:%s  行驶速度:%dKM/H  限制速度:%dKM/H  违章信息:-",strCarType.c_str(),plate.uSpeed,plate.uLimitSpeed);

		strText.clear();
		strText += chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += ((m_nExtentHeight/3)-5);
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		return;
	}
	else if(g_nServerType == 24)//南宁格式
	{
		nWidth = 10;
		nHeight = 10;

		string strText(chOut);
		CvScalar color = CV_RGB(255,255,255);

		//抓拍时间
		std::string strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
		sprintf(chOut,"捕获时间:%s %03d",strTime.c_str(), (((pTimeStamp+nIndex)->ts)/1000)%1000);

		strText.clear();
		strText = chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += 30;
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		//地点
		sprintf(chOut,"地点名称:%s",m_strLocation.c_str());

		strText.clear();
		strText = chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += 30;
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		//行驶方向
		std::string strDirection = GetDirection(plate.uDirection);
		sprintf(chOut,"行驶方向:%s", strDirection.c_str());

		strText.clear();
		strText = chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += 30;
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		//车道编号
		sprintf(chOut,"车道编号:%d",plate.uRoadWayID);

		strText.clear();
		strText = chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += 30;
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		//防伪码
		sprintf(chOut,"防伪码:NNJJBC%8x%01x",g_RoadImcData.GetRandCode(), g_RoadImcData.GetRandCode()%10);

		strText.clear();
		strText = chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		nHeight += 30;
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

		return;
	}

	//车辆类型
    std::string strCarType = GetDetailCarType(plate.uType,plate.uTypeDetail,plate.uDetailCarType);
    
	
	//洛阳图片格式
	if((m_nExtentHeight <= 0) &&(m_nWordOnPic == 1))
	{
		int nExtentHeight = 60;
		nExtentHeight = g_PicFormatInfo.nFontSize*2;
		nHeight -= nExtentHeight;
		
		std::string strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
		std::string strDirection = GetDirection(plate.uDirection);
		std::string strCarNum(plate.chText);

		sprintf(chOut,"时间:%s.%03d  地点名称:%s  方向:%s  车牌号码:%s  车道编号:%d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000,m_strLocation.c_str(),strDirection.c_str(),strCarNum.c_str(),plate.uRoadWayID);//
		
		string strText(chOut);

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());

		nHeight += (nExtentHeight/2);

		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

		strText.clear();
		//号牌颜色
		std::string strPlateColor = GetPlateColor(plate.uColor);
		 //车身颜色
        std::string strCarColor = GetObjectColor(plate.uCarColor1);
		 //车标
		std::string strCarLabel("");
        if(plate.uCarBrand != OTHERS)
		{
#ifdef GLOBALCARLABEL
			//LogNormal("uCarBrand=%d,uDetailCarBrand=%d\n",plate.uCarBrand,plate.uDetailCarBrand);
			UINT32 uCarLabel = plate.uCarBrand + plate.uDetailCarBrand;
			CBrandSusection BrandSub;
			strCarLabel = BrandSub.GetCarLabelText(uCarLabel);
#else
			strCarLabel = g_strCarLabel[plate.uCarBrand];
#endif
		}

		sprintf(chOut,"设备编号:%s  速度:%dkm/h  车辆类型:%s  车牌颜色:%s  车身颜色:%s  车标:%s",strDeviceId.c_str(),plate.uSpeed,strCarType.c_str(),strPlateColor.c_str(),strCarColor.c_str(),strCarLabel.c_str());//
		
		strText += chOut;
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());

		nHeight += (nExtentHeight/2);

		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

		return;
	}

    //设备编号
    std::string strDirection = GetDirection(plate.uDirection);
	
	{

		{
			sprintf(chOut,"设备编号:%s 地点名称:%s 方向:%s ",strDeviceId.c_str(),m_strLocation.c_str(),strDirection.c_str());
		}

	}

	string strText(chOut);    
	string strTextTemp="";

	//车道编号
	if(g_PicFormatInfo.nRoadIndex == 1)
	{
		sprintf(chOut,"车道编号:%d ",plate.uRoadWayID);//
		string strTmp(chOut);

		strTextTemp += strTmp;
	}

	//车牌号码
	if(g_PicFormatInfo.nCarNum == 1)
	{
		std::string strCarNum(plate.chText);
		sprintf(chOut,"车牌号码:%s ",strCarNum.c_str());
		string strTmp(chOut);
		strTextTemp += strTmp;
	}

	if(g_nServerType != 23 && g_nServerType != 26)
	{
		strText += strTextTemp;
	}

	if(23 == g_nServerType || 26 == g_nServerType)
	{
		strText += strTextTemp;
	}

	//经过时间
	std::string strTime;
	strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
	sprintf(chOut,"时间:%s.%03d ",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
	{
		string strTmp(chOut);
		strText += strTmp;
	}

    if(g_PicFormatInfo.nCarSpeed == 1)
    {
        //行驶速度
        sprintf(chOut,"速度:%dkm/h ",plate.uSpeed);
        string strTmp(chOut);
        strText += strTmp;
    }

	CvScalar color = CV_RGB(255,255,255);

	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,(char*)strText.c_str());
	if(pImage->width < 1000)
	{
		nHeight += 20;
	}
	else
	{
		nHeight += (m_nExtentHeight/2-5);
	}

	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);

	/////////////////////////////////////////////第二行
	strText.clear();

	if(g_PicFormatInfo.nSpeedLimit == 1)
	{
		 //限速值
        sprintf(chOut,"限速:%dkm/h ",plate.uLimitSpeed);
        string strTmp(chOut);
        strText += strTmp;
	}

    if(g_PicFormatInfo.nCarColor == 1)
    {
			//车身颜色
			std::string strCarColor = GetObjectColor(plate.uCarColor1);
			std::string strCarColor2 = GetObjectColor(plate.uCarColor2);
			
			#ifdef OBJECTTEXTCOLOR
			if(plate.uCarColor2 == UNKNOWN)
			{
				sprintf(chOut,"目标颜色:%s  ",strCarColor.c_str());
			}
			else
			{
				if(g_PicFormatInfo.nSecondCarColor == 0)
				{
					sprintf(chOut,"目标颜色:%s  ",strCarColor.c_str());
				}
				else
				{
					sprintf(chOut,"目标颜色:%s,%s  ",strCarColor.c_str(),strCarColor2.c_str());
				}
			}
			#else
			if(plate.uCarColor2 == UNKNOWN)
			{
				sprintf(chOut,"车身颜色:%s  ",strCarColor.c_str());
			}
			else
			{
				if(g_PicFormatInfo.nSecondCarColor == 0)
				{
					sprintf(chOut,"车身颜色:%s  ",strCarColor.c_str());
				}
				else
				{
					sprintf(chOut,"车身颜色:%s,%s  ",strCarColor.c_str(),strCarColor2.c_str());
				}
			}
			#endif
			string strTmp(chOut);
			strText += strTmp;
    }

    if(g_PicFormatInfo.nCarType == 1)
    {
		//号牌颜色
		std::string strPlateColor = GetPlateColor(plate.uColor);
		sprintf(chOut,"号牌颜色:%s  ",strPlateColor.c_str());

		string strPlateTmp(chOut);
        strText += strPlateTmp;

		sprintf(chOut,"车辆类型:%s  ",strCarType.c_str());
        string strTmp(chOut);
        strText += strTmp;
    }

    if(g_PicFormatInfo.nCarBrand == 1)
    {
        //车标
      // if(plate.uCarBrand != 100)
       {
           if(plate.uCarBrand != OTHERS)
		   {
#ifdef GLOBALCARLABEL
			    CBrandSusection BrandSub;
			   UINT32 uCarLabel = plate.uCarBrand + plate.uDetailCarBrand;
			   sprintf(chOut,"车标:%s  ",BrandSub.GetCarLabelText(uCarLabel).c_str());
#else
			   sprintf(chOut,"车标:%s  ",g_strCarLabel[plate.uCarBrand]);
#endif
		   }
           else
           sprintf(chOut,"车标:  ");

           string strTmp(chOut);
           strText += strTmp;
       }
    }
	if (g_PicFormatInfo.nPicFlag == 1)
	{
		int nRandCode = g_RoadImcData.GetRandCode();
		int nRandCode1 = g_RoadImcData.GetRandCode();
		int nRandCode2 = g_RoadImcData.GetRandCode();
		sprintf(chOut,"防伪码:%08x%08x%02x ",nRandCode,nRandCode1,nRandCode2%100);
		string strTmp(chOut);
		strText += strTmp;
	}

	if(g_PicFormatInfo.nViolationType == 1)
	{
		if(plate.uViolationType > 0)
		{
			std::string strViolationType = GetViolationType(plate.uViolationType,1);
			sprintf(chOut,"违章类型:%s ",strViolationType.c_str());
			string strTmp(chOut);
			strText += strTmp;
		}
	}

	//叠加安全带标志
	if((m_nDetectKind&DETECT_FACE) && plate.uDetectDirection == 0) //前排叠加安全带
	{
		if(plate.uBeltResult == 0)
		{
			sprintf(chOut,"安全带:有  ");
		}
		else if(plate.uBeltResult == 1)
		{
			sprintf(chOut,"安全带:无  ");
		}
		else if(plate.uBeltResult == 2)
		{
			sprintf(chOut,"安全带:有  ");
		}
		string strTmp1(chOut);
		strText += strTmp1;
	}
	if (m_bDetectShield && plate.uDetectDirection == 0) //是否遮阳板检测
	{
		if(plate.uPhoneResult == 0)
		{
			sprintf(chOut,"打手机:未知  ");
		}
		else if(plate.uPhoneResult == 1)
		{
			sprintf(chOut,"打手机:否  ");
		}
		else if(plate.uPhoneResult == 2)
		{
			sprintf(chOut,"打手机:是  ");
		}

		string strTmp2(chOut);
		strText += strTmp2;

		if(plate.uSunVisorResult == 0)
		{
			sprintf(chOut,"遮阳板:未知");
		}
		else if(plate.uSunVisorResult == 1)
		{
			sprintf(chOut,"遮阳板:无");
		}
		else if(plate.uSunVisorResult == 2)
		{
			sprintf(chOut,"遮阳板:有");
		}
		string strTmp3(chOut);
		strText += strTmp3;
	}

#ifdef VTS_TEST_SEQ
	//LogNormal(" plate %s Seq: [%lld] ts:%u", plate.chText, plate.uSeq, plate.uTime);
	//记录帧号
	sprintf(chOut," plate Seq: [%lld]", plate.uSeq);
	string strTmpTest(chOut);
	strText += strTmpTest;			
#endif

    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,(char*)strText.c_str());
    nWidth = 10;
    if(pImage->width > nBaseWidth)
    {
        nWidth += (nBaseWidth)*(nIndex%2);
    }
    nStartX = nWidth;

    if(pImage->width < 1000)
    {
        nHeight += 20;
    }
    else
    {
        nHeight += (m_nExtentHeight/2-5);
    }


    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), color);


}

//图像增强
void CRoadCarnumDetect::ImageEnhance(IplImage* pImg)
{
	{		
		    if(13 == g_nServerType)//上海交警项目特殊处理
			{
				cvSmooth( pImg, pImg, CV_GAUSSIAN,5);
				return;
			}
			

			//LogNormal("m_nDayNightbyLight=%d\n",m_nDayNightbyLight);
			if(m_nDayNightbyLight == 0)
			{
				cvConvertImage(pImg,pImg,CV_CVTIMG_SWAP_RB);

				//MvImgEnhance ImgEnhance;
				MvImgEnhance_HandOver ImgEnhance;
				CvRect rt = cvRect(0,0,pImg->width,pImg->height);
				//ImgEnhance.mvWhiteBalance(pImg, rt);

				//printf("mvWhiteBalance pImg->width=%d,pImg->height=%d\n",pImg->width,pImg->height);

				//ImgEnhance.mvLightGamaAdjust(pImg, NULL);
				ImgEnhance.mvImgEnhance(pImg, rt,m_nDayNightbyLight);

				cvConvertImage(pImg,pImg,CV_CVTIMG_SWAP_RB);
			}
	}
}

//保存全景图像
int CRoadCarnumDetect::SaveImage(IplImage* pImg,std::string strPicPath,int nIndex)
{

    #ifdef LogTime
    struct timeval tv;
    double t = (double)cvGetTickCount();
    gettimeofday(&tv,NULL);
    LogTrace("time-test.log","before SaveImage==time = %s.%03d\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
    #endif

	IplImage* pSrcImg = pImg;

    BYTE* pOutImage = NULL;
    if(nIndex == 0) //第一张图
    {
        pOutImage = m_pCurJpgImage;
    }
    else if(nIndex == 1) //第二张图
    {
        pOutImage = m_pPreJpgImage;
    }
    else if(nIndex == 2) //合成图像
    {
        pOutImage = m_pComposeJpgImage;
    }

	//图像缩放
	if(g_PicFormatInfo.nResizeScale < 100 && g_PicFormatInfo.nResizeScale > 0)
	{
		if(m_imgComposeSnap != NULL)
		{
			if(g_nServerType == 23 || g_nServerType == 26)//济南
			{
				if((pImg->width > m_imgSnap->width))
				{
					if(m_imgResize != NULL)
					{
						cvReleaseImage(&m_imgResize);
						m_imgResize = NULL;
					}

					m_imgResize = cvCreateImage(cvSize(pImg->width*g_PicFormatInfo.nResizeScale/100.0,pImg->height*g_PicFormatInfo.nResizeScale/100.0),8,3);
					cvResize(pImg,m_imgResize);
					pSrcImg = m_imgResize;
				}
			}
			else
			{
				if((m_imgComposeSnap->width == pImg->width) && (m_imgComposeSnap->height == pImg->height))
				{
					if(m_imgResize == NULL)
					{
						m_imgResize = cvCreateImage(cvSize(pImg->width*g_PicFormatInfo.nResizeScale/100.0,pImg->height*g_PicFormatInfo.nResizeScale/100.0),8,3);
					}
					cvResize(pImg,m_imgResize);
					pSrcImg = m_imgResize;
				}
			}
		}
	}

#ifdef RESIZEIMAGE

	if(m_imgResize != NULL)
	{
		cvReleaseImage(&m_imgResize);
		m_imgResize = NULL;
	}

	m_imgResize = cvCreateImage(cvSize(pImg->width*2048/1920,pImg->height*1540/1440),8,3);
	cvResize(pImg,m_imgResize);
	pSrcImg = m_imgResize;

#endif

    CxImage image;
    int srcstep = 0;
	printf("IppEncode pSrcImg->width=%d,pSrcImg->height=%d\n",pSrcImg->width,pSrcImg->height);
    if(image.IppEncode((BYTE*)pSrcImg->imageData,pSrcImg->width,pSrcImg->height,3,&srcstep,pOutImage,g_PicFormatInfo.nJpgQuality))
	{

        #ifdef LogTime
        t = (double)cvGetTickCount() - t;
        double dt = t/((double)cvGetTickFrequency()*1000.) ;
        gettimeofday(&tv,NULL);
        LogTrace("time-test.log","after IppEncode==time = %s.%03d,dt=%d ms\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,(int)dt);
        t = (double)cvGetTickCount();
        #endif
        FILE* fp = NULL;
        if(nIndex == 1)
        {
           fp = fopen(strPicPath.c_str(),"a");
        }
        else
        {
           fp = fopen(strPicPath.c_str(),"wb");
        }
        if(fp!=NULL)
        {
            fwrite(pOutImage,srcstep,1,fp);

			if(g_nServerType == 7)
			{
				//在jpg的后面记录防伪码
				fwrite(&m_nRandCode[1],4,1,fp);
				fwrite(&m_nRandCode[2],4,1,fp);
			}

            //叠加数字水印
            #ifdef WATER_MARK
            std::string strWaterMark;
            GetWaterMark((char*)pOutImage,srcstep,strWaterMark);
            //printf("before write WaterMark,strWaterMark.size()=%d\n",strWaterMark.size());
            //fflush(stdout);
            fwrite(strWaterMark.c_str(),strWaterMark.size(),1,fp);
            //printf("after write WaterMark\n");
            //fflush(stdout);
            srcstep += strWaterMark.size();
            #endif


            fclose(fp);

        }
        else
        {
            printf("canot open file\n");
        }
    }
    return srcstep;
}

//保存人脸特征图像
int CRoadCarnumDetect::SaveFaceImage(IplImage* pImg,RECORD_PLATE& plate,vector<CvRect> vFaceRect)
{
	string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));

	IplImage* pSrcImg = pImg;

    BYTE* pOutImage = m_pCurJpgImage;
 
    CxImage image;
    int srcstep = 0;
    if(image.IppEncode((BYTE*)pSrcImg->imageData,pSrcImg->width,pSrcImg->height,3,&srcstep,pOutImage,g_PicFormatInfo.nJpgQuality))
    {
        FILE* fp = fopen(strPicPath.c_str(),"wb");
        if(fp!=NULL)
        {
			
            fwrite(pOutImage,srcstep,1,fp);

			//存储人脸图像

			 //先添加0xFFD8FFE0
			UINT32 nFaceFlag = 0xE0FFD8FF;
			string strFace;
			strFace.append((char*)&nFaceFlag,sizeof(UINT32));
			strFace.append("$$$$",4);
			//再添加人脸个数
			UINT32 nFaceRectSize = vFaceRect.size();
			printf("SaveFaceImage nFaceRectSize=%d\n",nFaceRectSize);


			strFace.append((char*)&nFaceRectSize,sizeof(UINT32));

			fwrite(strFace.c_str(),strFace.size(),1,fp);
			srcstep += strFace.size();

			vector<CvRect>::iterator it = vFaceRect.begin();
			while(it != vFaceRect.end())
			{
				CvRect rect = *it;
				if(rect.width%4 != 0)
				{
					rect.width += 4 - rect.width%4;
				}
				printf("SaveFaceImage rect.width=%d,rect.height=%d\n",rect.width,rect.height);

				IplImage* pFaceImg = cvCreateImage(cvSize(rect.width,rect.height+m_nExtentHeight),8,3);
				cvSet(pFaceImg, cvScalar( 0,0, 0 ));
				unsigned char* pFaceData = new unsigned char[rect.width*(rect.height+m_nExtentHeight)*3];

				CvRect rtImage;
				rtImage.x = 0;
				rtImage.y = 0;
				rtImage.width = rect.width;
				rtImage.height = rect.height;

				cvSetImageROI(pFaceImg,rtImage);
				cvSetImageROI(pImg,rect);
				cvResize(pImg, pFaceImg);
				cvResetImageROI(pImg);
				cvResetImageROI(pFaceImg);
				
				//叠加人脸小图时间
				if(pFaceImg->width > 80)
				{
					wchar_t wchOut[1024] = {'\0'};
					int nWidth = 0;
					int nHeight = rect.height+m_nExtentHeight/2;
					string strDate = GetTime(plate.uTime,11);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strDate.c_str());
					m_cvText.putText(pFaceImg, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					
					nHeight = rect.height+m_nExtentHeight-5;
					string strTime = GetTime(plate.uTime,12);

					char buf[256] = {0};
					sprintf(buf,".%03d",plate.uMiTime);
					string strMiTime(buf);
					strTime += strMiTime;
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strTime.c_str());
					m_cvText.putText(pFaceImg, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
				}

				printf("imageFace.IppEncode pFaceImg->height=%d,pFaceImg->width=%d\n",pFaceImg->width,pFaceImg->height);
				CxImage imageFace;
				int nDataSize = 0;
				if(imageFace.IppEncode((BYTE*)pFaceImg->imageData,pFaceImg->width,pFaceImg->height,3,&nDataSize,pFaceData,g_PicFormatInfo.nJpgQuality))
				{
					//再添加每张人脸图片大小
					fwrite(&nDataSize,sizeof(nDataSize),1,fp);
					srcstep += 4;
					
					//再写入人脸图片数据
					fwrite(pFaceData,nDataSize,1,fp);
					srcstep += nDataSize;
				}
				
				if(pFaceData)
				{
					delete []pFaceData;
				}
				cvReleaseImage(&pFaceImg);
				it++;
			}

			//叠加车牌位置
			int nCarnumSize = 0;
			if(plate.chText[0] != '*')
			{
				nCarnumSize = 1;
			}
			string strCarnum;
			strCarnum.append((char*)&nCarnumSize,sizeof(UINT32));
			strCarnum.append((char*)&plate.uPosLeft,sizeof(UINT32));
			strCarnum.append((char*)&plate.uPosTop,sizeof(UINT32));
			strCarnum.append((char*)&plate.uPosRight,sizeof(UINT32));
			strCarnum.append((char*)&plate.uPosBottom,sizeof(UINT32));

			fwrite(strCarnum.c_str(),strCarnum.size(),1,fp);
			srcstep += strCarnum.size();

            //叠加数字水印
            #ifdef WATER_MARK
            std::string strWaterMark;
            GetWaterMark((char*)pOutImage,srcstep,strWaterMark);
            fwrite(strWaterMark.c_str(),strWaterMark.size(),1,fp);
            srcstep += strWaterMark.size();
            #endif

            fclose(fp);

        }
        else
        {
            printf("canot open file\n");
        }
    }

    return srcstep;
}

//流量评价统计
void CRoadCarnumDetect::VtsStatistic(yuv_video_buf* buf)
{
	if(g_nDetectMode == 2)//dsp方案暂不支持流量评价数据
	{
		return;
	}

	UINT16 uTrafficSignal = buf->uTrafficSignal;
	UINT32 uSeq = buf->nSeq;

	//车道编号
	UINT32 uRoadID;
	UINT32 uVerRoadID;
	UINT32 uObjectNum;
	//统计信息

	RECORD_STATISTIC statistic1;
	RECORD_STATISTIC statistic2;

	bool bStatistic = false;

	bool bLeftStatistic = false;

	VTSParaMap::iterator it_p = m_vtsObjectParaMap.begin();
	while(it_p != m_vtsObjectParaMap.end())
	{
		MvChannelFrameSignalStatus vtsSignalStatus;

		vtsSignalStatus.nRoadIndex =  it_p->second.nRoadIndex;

		vtsSignalStatus.uFrameSeq = uSeq;

		vtsSignalStatus.bLeftSignal = ((uTrafficSignal>>(it_p->second.nLeftControl)) & 0x1);
		vtsSignalStatus.bStraightSignal = ((uTrafficSignal>>(it_p->second.nStraightControl)) & 0x1);
		vtsSignalStatus.bRightSignal = ((uTrafficSignal>>(it_p->second.nRightControl)) & 0x1);

		//计算直行灯非红灯时间间隔
		map<int,MvChannelFrameSignalStatus>::iterator it = m_mapVTSStatus.find(vtsSignalStatus.nRoadIndex);

		if(it != m_mapVTSStatus.end())
		{
			//根据第一个车道信号灯状态进行统计
			if(it_p == m_vtsObjectParaMap.begin())
			{
				////////////////////////直行灯
				//红变绿
				if((it->second.bStraightSignal == 1) && (vtsSignalStatus.bStraightSignal == 0))
				{
					if(m_uNonTrafficSignalBeginTime == 0)
					{
						m_uNonTrafficSignalBeginTime = buf->ts;
					}

				}

				//绿变红
				if((it->second.bStraightSignal == 0) && (vtsSignalStatus.bStraightSignal == 1))
				{
					if((m_uNonTrafficSignalEndTime == 0) &&(m_uNonTrafficSignalBeginTime != 0))
					{
						m_uNonTrafficSignalEndTime = buf->ts;

						m_uNonTrafficSignalTime = m_uNonTrafficSignalEndTime - m_uNonTrafficSignalBeginTime;
						LogNormal("NonTrafficSignalTime=%d",m_uNonTrafficSignalTime);
					}
				}

				//下次绿灯期间开始统计
				if(m_uNonTrafficSignalTime > 0)
				{
					//红变绿(绿灯开始时刻)
					if((it->second.bStraightSignal == 1) && (vtsSignalStatus.bStraightSignal == 0))
					{
						m_uPreNonTrafficSignalTime = buf->ts;//当前统计时刻
						m_nConnectnumber = 0;
						bStatistic = true;
					}
					else if((it->second.bStraightSignal == 0) && (vtsSignalStatus.bStraightSignal == 1))
					{
						if(m_nConnectnumber == 2)
						{
							m_uPreNonTrafficSignalTime = buf->ts;//当前统计时刻
							m_nConnectnumber = 3;
							bStatistic = true;
						}
					}
					else if(vtsSignalStatus.bStraightSignal == 0)
					{
						if(buf->ts >= m_uPreNonTrafficSignalTime + m_uNonTrafficSignalTime/3)
						{
							m_uPreNonTrafficSignalTime = buf->ts;//当前统计时刻
							m_nConnectnumber++;
							bStatistic = true;
						}
					}
				}

				////////////////////////左转灯
				//红变绿
				if((it->second.bLeftSignal == 1) && (vtsSignalStatus.bLeftSignal == 0))
				{
					if(m_uNonLeftTrafficSignalBeginTime == 0)
					{
						m_uNonLeftTrafficSignalBeginTime = buf->ts;
					}

				}

				//绿变红
				if((it->second.bLeftSignal == 0) && (vtsSignalStatus.bLeftSignal == 1))
				{
					if((m_uNonLeftTrafficSignalEndTime == 0) &&(m_uNonLeftTrafficSignalBeginTime != 0))
					{
						m_uNonLeftTrafficSignalEndTime = buf->ts;

						m_uNonLeftTrafficSignalTime = m_uNonLeftTrafficSignalEndTime - m_uNonLeftTrafficSignalBeginTime;
						LogNormal("NonLeftTrafficSignalTime=%d",m_uNonLeftTrafficSignalTime);
					}
				}

				//下次绿灯期间开始统计
				if(m_uNonLeftTrafficSignalTime > 0)
				{
					//红变绿(绿灯开始时刻)
					if((it->second.bLeftSignal == 1) && (vtsSignalStatus.bLeftSignal == 0))
					{
						m_uPreNonLeftTrafficSignalTime = buf->ts;//当前统计时刻
						m_nLeftConnectnumber = 4;
						bLeftStatistic = true;
					}
					else if((it->second.bLeftSignal == 0) && (vtsSignalStatus.bLeftSignal == 1))
					{
						if(m_nLeftConnectnumber == 6)
						{
							m_uPreNonLeftTrafficSignalTime = buf->ts;//当前统计时刻
							m_nLeftConnectnumber = 7;
							bLeftStatistic = true;
						}
					}
					else if(vtsSignalStatus.bLeftSignal == 0)
					{
						if(buf->ts >= m_uPreNonLeftTrafficSignalTime + m_uNonLeftTrafficSignalTime/3)
						{
							m_uPreNonLeftTrafficSignalTime = buf->ts;//当前统计时刻
							m_nLeftConnectnumber++;
							bLeftStatistic = true;
						}
					}
				}
			}

			m_mapVTSStatus.erase(it);
		}
		m_mapVTSStatus.insert(map<int,MvChannelFrameSignalStatus>::value_type(vtsSignalStatus.nRoadIndex,vtsSignalStatus));


		bool bDetect = false;

		if(it_p->second.bForbidLeft || it_p->second.bForbidRight || it_p->second.bForbidRun ||
			it_p->second.bForbidStop || it_p->second.bAgainst || it_p->second.bChangeRoad ||
			it_p->second.bPressLine)
		{
			bDetect = true;
		}

		if(bStatistic&&bDetect)
		{
			uRoadID = vtsSignalStatus.nRoadIndex;       //车道编号
			uVerRoadID = uRoadID<<16;
			statistic1.uRoadType[uRoadID-1] = uVerRoadID | VEHICLE_ROAD;

			//高16位大车总数，低16位车辆总数
			uObjectNum = uSignalFluxBig[uRoadID-1];
			uObjectNum = uObjectNum<<16;
			statistic1.uFlux[uRoadID-1] = uObjectNum | (uSignalFluxAll[uRoadID-1]);

			//高16位中型车总数，低16位小车总数
			statistic1.uFluxCom[uRoadID-1] = (uSignalFluxSmall[uRoadID-1]);

			//大车平均车速
			if(uSignalFluxBig[uRoadID-1] > 0)
			{
				statistic1.uBigCarSpeed[uRoadID-1] = (uSignalBigCarSpeed[uRoadID-1]*1.0/uSignalFluxBig[uRoadID-1]);
			}
			else
			{
				statistic1.uBigCarSpeed[uRoadID-1] = 0;
			}
			//LogNormal("uRoadID=%d,uBigCarSpeed=%d",uRoadID,statistic1.uBigCarSpeed[uRoadID-1]);

			//小车平均车速
			if(uSignalFluxSmall[uRoadID-1] > 0)
			{
				statistic1.uSmallCarSpeed[uRoadID-1] = (uSignalSmallCarSpeed[uRoadID-1]*1.0/uSignalFluxSmall[uRoadID-1]);
			}
			else
			{
				statistic1.uSmallCarSpeed[uRoadID-1] = 0;
			}
			//LogNormal("uRoadID=%d,uSmallCarSpeed=%d",uRoadID,statistic1.uSmallCarSpeed[uRoadID-1]);

			//平均车速
			if(uSignalFluxAll[uRoadID-1] > 0)
			{
				statistic1.uSpeed[uRoadID-1] = (uSignalAverageCarSpeed[uRoadID-1]*1.0/uSignalFluxAll[uRoadID-1]);
			}
			else
			{
				statistic1.uSpeed[uRoadID-1] = 0;
			}
			//LogNormal("uRoadID=%d,uSpeed=%d",uRoadID,statistic1.uSpeed[uRoadID-1]);


			//最大车速
			statistic1.uMaxSpeed[uRoadID-1] = (BYTE)uSignalMaxSpeed[uRoadID-1];
			//LogNormal("uRoadID=%d,uMaxSpeed=%d",uRoadID,statistic1.uMaxSpeed[uRoadID-1]);
			statistic1.uOccupancy[uRoadID-1] = 0;
			statistic1.uQueue[uRoadID-1] = 0;
			statistic1.uSpace[uRoadID-1] = 0;

		}

		if(m_uNonTrafficSignalBeginTime != m_uNonLeftTrafficSignalBeginTime)
		{
			if(bLeftStatistic&&bDetect)
			{
				uRoadID = vtsSignalStatus.nRoadIndex;       //车道编号
				uVerRoadID = uRoadID<<16;
				statistic2.uRoadType[uRoadID-1] = uVerRoadID | VEHICLE_ROAD;

				//高16位大车总数，低16位车辆总数
				uObjectNum = uLeftSignalFluxBig[uRoadID-1];
				uObjectNum = uObjectNum<<16;
				statistic2.uFlux[uRoadID-1] = uObjectNum | (uLeftSignalFluxAll[uRoadID-1]);

				//高16位中型车总数，低16位小车总数
				statistic2.uFluxCom[uRoadID-1] = (uLeftSignalFluxSmall[uRoadID-1]);

				//大车平均车速
				if(uLeftSignalFluxBig[uRoadID-1] > 0)
				{
					statistic2.uBigCarSpeed[uRoadID-1] = (uLeftSignalBigCarSpeed[uRoadID-1]*1.0/uLeftSignalFluxBig[uRoadID-1]);
				}
				else
				{
					statistic2.uBigCarSpeed[uRoadID-1] = 0;
				}
				//小车平均车速
				if(uLeftSignalFluxSmall[uRoadID-1] > 0)
				{
					statistic2.uSmallCarSpeed[uRoadID-1] = (uLeftSignalSmallCarSpeed[uRoadID-1]*1.0/uLeftSignalFluxSmall[uRoadID-1]);
				}
				else
				{
					statistic2.uSmallCarSpeed[uRoadID-1] = 0;
				}
				//平均车速
				if(uLeftSignalFluxAll[uRoadID-1] > 0)
				{
					statistic2.uSpeed[uRoadID-1] = (uLeftSignalAverageCarSpeed[uRoadID-1]*1.0/uLeftSignalFluxAll[uRoadID-1]);
				}
				else
				{
					statistic2.uSpeed[uRoadID-1] = 0;
				}
				//最大车速
				statistic2.uMaxSpeed[uRoadID-1] = (BYTE)uLeftSignalMaxSpeed[uRoadID-1];
				statistic2.uOccupancy[uRoadID-1] = 0;
				statistic2.uQueue[uRoadID-1] = 0;
				statistic2.uSpace[uRoadID-1] = 0;
			}
		}

		it_p++;
	}

	if(m_uNonTrafficSignalBeginTime != m_uNonLeftTrafficSignalBeginTime)
	{
		if(bLeftStatistic)
		{
			if(m_nLeftConnectnumber > 4)
			{
				std::string strStatistic;
				SRIP_DETECT_HEADER sDHeader;
				sDHeader.uChannelID = m_nCameraID;
				sDHeader.uDetectType = MIMAX_STATISTIC_REP;
				sDHeader.uRealTime = 0x00000001;//此时均设为实时统计
				strStatistic.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));

				statistic2.uTime = buf->uTimestamp;
				statistic2.uSeq = m_nLeftConnectnumber;
				statistic2.uStatTimeLen = m_uNonLeftTrafficSignalTime/3000000.0;
				strStatistic.append((char*)&statistic2,sizeof(RECORD_STATISTIC));

				g_RoadImcData.AddStatisticData(strStatistic,VEHICLE_ROAD,1);
			}
			//流量统计初始化
			for(int i = 0; i<MAX_ROADWAY; i++)
			{
				uLeftSignalFluxAll[i]=0;       //总流量
				uLeftSignalFluxSmall[i]=0;     //小车流量
				uLeftSignalFluxBig[i]=0;   //大车流量
				uLeftSignalBigCarSpeed[i]=0;	//大车平均速度
				uLeftSignalSmallCarSpeed[i]=0;	//小车平均速度
				uLeftSignalAverageCarSpeed[i]=0;	//平均速度
				uLeftSignalMaxSpeed[i]=0;		//最大速度
			}
		}
	}

	if(bStatistic)
	{
		if(m_nConnectnumber > 0)
		{
			std::string strStatistic;
			SRIP_DETECT_HEADER sDHeader;
			sDHeader.uChannelID = m_nCameraID;
			sDHeader.uDetectType = MIMAX_STATISTIC_REP;
			sDHeader.uRealTime = 0x00000001;//此时均设为实时统计
			strStatistic.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));

			statistic1.uTime = buf->uTimestamp;
			statistic1.uSeq = m_nConnectnumber;
			statistic1.uStatTimeLen = m_uNonTrafficSignalTime/3000000.0;
			strStatistic.append((char*)&statistic1,sizeof(RECORD_STATISTIC));

			g_RoadImcData.AddStatisticData(strStatistic,VEHICLE_ROAD,1);
		}

		//流量统计初始化
		for(int i = 0; i<MAX_ROADWAY; i++)
		{
			uSignalFluxAll[i]=0;       //总流量
			uSignalFluxSmall[i]=0;     //小车流量
			uSignalFluxBig[i]=0;   //大车流量
			uSignalBigCarSpeed[i]=0;	//大车平均速度
			uSignalSmallCarSpeed[i]=0;	//小车平均速度
			uSignalAverageCarSpeed[i]=0;	//平均速度
			uSignalMaxSpeed[i]=0;		//最大速度
		}

	}
}

//流量统计
void CRoadCarnumDetect::VehicleStatistic(StatResultList& listStatResult,UINT32 uTimestamp)
{
	//1拖N模式由检测器统计车流量
	if(g_nDetectMode == 2)
	{
		//LogNormal("VehicleStatistic t1:%lld t2:%lld uTimestamp:%lld\n", \
		//	m_uPreTrafficSignalTime, m_nTrafficStatTime, uTimestamp);
		if(uTimestamp >= m_uPreTrafficSignalTime + m_nTrafficStatTime)
		{
			//printf("CRoadCarnumDetect::VehicleStatistic-uTimestamp=%d-m_uPreTrafficSignalTime=%d-m_nTrafficStatTime=%d \n", \
			//	uTimestamp, m_uPreTrafficSignalTime, m_nTrafficStatTime);

			VehicleStatisticDsp(listStatResult,uTimestamp);
		}
		else
		{
			return;
		}
	}
	else
	{
		//车道编号
		UINT32 uRoadID;
		UINT32 uVerRoadID;
		UINT32 uObjectNum;

		//统计保存
		RECORD_STATISTIC statistic;

		StatResultList::iterator it_b = listStatResult.begin();
		StatResultList::iterator it_e = listStatResult.end();
		while(it_b != it_e)
		{
			uRoadID = it_b->nChannelIndex;       //车道编号

			vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
			while(it != m_vtsObjectRegion.end())
			{
				if(uRoadID == -1||
					uRoadID == it->nRoadIndex)
				{
					uVerRoadID = it->nVerRoadIndex;//车道逻辑编号
					uRoadID = it->nRoadIndex;
					break;
				}
				it++;
			}

			bool bDetect = false;

			if(g_nServerType == 7)
			{
				VTSParaMap::iterator it_p = m_vtsObjectParaMap.find(uRoadID);

				if(it_p != m_vtsObjectParaMap.end())
				{
					if(it_p->second.bForbidLeft || it_p->second.bForbidRight || it_p->second.bForbidRun ||
						it_p->second.bForbidStop || it_p->second.bAgainst || it_p->second.bChangeRoad ||
						it_p->second.bPressLine)
					{
						bDetect = true;
					}
				}
			}
			else
			{
				bDetect = true;
			}

			if(bDetect)
			{
				//高16位车道逻辑编号，低16位车道类型
				uVerRoadID = uVerRoadID<<16;
				statistic.uRoadType[uRoadID-1] = uVerRoadID | VEHICLE_ROAD;


				switch(it_b->sRtype)
				{
				case STAT_FLUX:
					//高16位大车总数，低16位车辆总数
					uObjectNum = uFluxBig[uRoadID-1];
					uObjectNum = uObjectNum<<16;
					//uFluxAll[uRoadID-1] = (UINT32) (it_b->value);
					statistic.uFlux[uRoadID-1] = uObjectNum | (uFluxAll[uRoadID-1]);
					break;
				case STAT_SPEED_AVG:
					//statistic.uSpeed[uRoadID-1] = (UINT32) (it_b->value);
					if(uFluxAll[uRoadID-1] > 0)
					{
						statistic.uSpeed[uRoadID-1] = uAverageCarSpeed[uRoadID-1]/uFluxAll[uRoadID-1];
					}
					else
					{
						statistic.uSpeed[uRoadID-1] = 0;
					}
					break;
				case STAT_ZYL:
					statistic.uOccupancy[uRoadID-1] = (UINT32) (it_b->value);
					break;
				case STAT_QUEUE:
					statistic.uQueue[uRoadID-1] = (UINT32) (it_b->value);
					break;
				case STAT_CTJJ:
					statistic.uSpace[uRoadID-1] = (UINT32) (it_b->value);
					break;
				}

				//高16位中型车总数，低16位小车总数
				uObjectNum = uFluxMiddle[uRoadID-1];
				uObjectNum = uObjectNum<<16;
				statistic.uFluxCom[uRoadID-1] = uObjectNum|(uFluxSmall[uRoadID-1]);
				statistic.uTime = uTimestamp;
				statistic.uStatTimeLen = m_nTrafficStatTime;

				if(uFluxBig[uRoadID-1] > 0)
				{
					statistic.uBigCarSpeed[uRoadID-1] = (uBigCarSpeed[uRoadID-1]*1.0/uFluxBig[uRoadID-1]);
				}
				else
				{
					statistic.uBigCarSpeed[uRoadID-1] = 0;
				}

				if(uFluxSmall[uRoadID-1] > 0)
				{
					statistic.uSmallCarSpeed[uRoadID-1] = (uSmallCarSpeed[uRoadID-1]*1.0/uFluxSmall[uRoadID-1]);
				}
				else
				{
					statistic.uSmallCarSpeed[uRoadID-1] = 0;
				}

				statistic.uMaxSpeed[uRoadID-1] = uMaxSpeed[uRoadID-1];
			}

			it_b++;
		}

		//记录检测数据,保存统计
		g_skpDB.SaveStatisticInfo(m_nChannelId,statistic,VEHICLE_ROAD);
	}//End of else    

    //流量统计初始化
    for(int i = 0; i<MAX_ROADWAY; i++)
    {
        uFluxAll[i]=0;
        uFluxSmall[i]=0;
        uFluxMiddle[i]=0;
        uFluxBig[i]=0;
        uFluxPerson[i]=0;
        uFluxNoneVehicle[i]=0;
		uBigCarSpeed[i]=0;
		uSmallCarSpeed[i]=0;
		uAverageCarSpeed[i]=0;
		uMaxSpeed[i]=0;

		uOccupyRatio[i] = 0;
		uAvgDis[i] = 0;
		uPreCarTime[i] = 0;
    }
}

//统计车流量
void CRoadCarnumDetect::StatisticVehicleFlux(CarInfo& cardNum,RECORD_PLATE& plate)
{	
	#ifndef ALGORITHM_YUV
			
			if(plate.uType==SMALL_CAR)
            {
				//LogNormal("cardNum.RoadIndex=%d\n",cardNum.RoadIndex);
                uFluxSmall[cardNum.RoadIndex-1]++;
                uFluxAll[cardNum.RoadIndex-1]++;

				uSmallCarSpeed[cardNum.RoadIndex-1] += plate.uSpeed;
				uAverageCarSpeed[cardNum.RoadIndex-1] += plate.uSpeed;

				uSignalFluxAll[cardNum.RoadIndex-1]++;       //总流量
				uSignalFluxSmall[cardNum.RoadIndex-1]++;     //小车流量
				uSignalSmallCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//小车平均速度
				uSignalAverageCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//平均速度

				uLeftSignalFluxAll[cardNum.RoadIndex-1]++;       //总流量
				uLeftSignalFluxSmall[cardNum.RoadIndex-1]++;     //小车流量
				uLeftSignalSmallCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//小车平均速度
				uLeftSignalAverageCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//平均速度
            }
            else if(plate.uType==MIDDLE_CAR)
            {
                uFluxMiddle[cardNum.RoadIndex-1]++;
                uFluxAll[cardNum.RoadIndex-1]++;
            }
            else if(plate.uType==BIG_CAR)
            {
                uFluxBig[cardNum.RoadIndex-1]++;
                uFluxAll[cardNum.RoadIndex-1]++;
				uBigCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;
				uAverageCarSpeed[cardNum.RoadIndex-1] += plate.uSpeed;

				uSignalFluxAll[cardNum.RoadIndex-1]++;       //总流量
				uSignalFluxBig[cardNum.RoadIndex-1]++;     //大车流量
				uSignalBigCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//大车平均速度
				uSignalAverageCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//平均速度

				uLeftSignalFluxAll[cardNum.RoadIndex-1]++;       //总流量
				uLeftSignalFluxBig[cardNum.RoadIndex-1]++;     //大车流量
				uLeftSignalBigCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//大车平均速度
				uLeftSignalAverageCarSpeed[cardNum.RoadIndex-1]+= plate.uSpeed;	//平均速度
            }
			if(plate.uType == SMALL_CAR ||
				plate.uType == BIG_CAR)
			{
				if(uMaxSpeed[cardNum.RoadIndex-1] < plate.uSpeed)
				{
					uMaxSpeed[cardNum.RoadIndex-1] = plate.uSpeed;
				}

				if(uSignalMaxSpeed[cardNum.RoadIndex-1] < plate.uSpeed)
				{
					uSignalMaxSpeed[cardNum.RoadIndex-1] = plate.uSpeed;
				}

				if(uLeftSignalMaxSpeed[cardNum.RoadIndex-1] < plate.uSpeed)
				{
					uLeftSignalMaxSpeed[cardNum.RoadIndex-1] = plate.uSpeed;
				}
			}

			if(g_nDetectMode == 2)
			{				
				if(uPreCarTime[cardNum.RoadIndex-1] > 0 && uPreCarTime[cardNum.RoadIndex-1] > 0)
				{
					uAvgDis[cardNum.RoadIndex-1] = (UINT32)((plate.uTime-uPreCarTime[cardNum.RoadIndex-1]) * plate.uSpeed * 0.27778f);//单位m

					if(uAvgDis[cardNum.RoadIndex-1] > 2000)
					{
						uAvgDis[cardNum.RoadIndex-1] = 0;
					}
				}				
				uPreCarTime[cardNum.RoadIndex-1] = plate.uTime;
			}
#endif

			//printf("--uFluxAll[cardNum.RoadIndex-1]=%d", uFluxAll[cardNum.RoadIndex-1]);
}

//输出违章检测结果
void CRoadCarnumDetect::OutPutVTSResult(std::vector<ViolationInfo>& vResult)
{
	printf("==in OutPutVTSResult==\n");

    string strEvent;
    SRIP_DETECT_HEADER sHeader;
    std::vector<ViolationInfo>::iterator it = vResult.begin();
    while(it != vResult.end())
    {
        ViolationInfo infoViolation = *it;
		
		bool bGetVtsImgByKey = false;//是否需要判断输出
        if(g_nDetectMode != 2)//非dsp模式
		{
			//违章类型
			if(infoViolation.evtType == ELE_RED_LIGHT_VIOLATION ||		   /* 0	闯红灯   */
								infoViolation.evtType == ELE_PARKING_VIOLATION ||          /* 1	违章停车 */
								infoViolation.evtType == ELE_FORBID_LEFT ||				   /* 2	禁止左拐 */
								infoViolation.evtType == ELE_FORBID_RIGHT ||               /* 3	禁止右拐 */
								infoViolation.evtType == ELE_FORBID_STRAIGHT ||            /* 4	禁止前行 */
								infoViolation.evtType == ELE_RETROGRADE_MOTION ||          /* 5	逆行	 */
								infoViolation.evtType == ELE_PRESS_LINE ||                 /* 6	压黄线	 */
								infoViolation.evtType == ELE_PRESS_WHITE_LINE ||           /* 7	压白线	 */
								infoViolation.evtType == ELE_EVT_BIANDAO ||				   /* 9	变道	 */
								infoViolation.evtType == EVT_NO_PARKING ||				   /* 34 黄网格停车 */
								infoViolation.evtType == EVT_BREAK_GATE ||				   /* 35    冲卡 */
								infoViolation.evtType == EVT_NOT_CUTRTESY_DRIVE 		   /* 36    车辆未礼让行人 */)
			{
								bGetVtsImgByKey = true;
			}
		}
		
		//dsp相机输出,核查是否立即输出记录,防止记录拥塞	
		m_nDealFlag = CheckViolationInfo(infoViolation);

		OutPutVTSResultElem(infoViolation,strEvent,sHeader,bGetVtsImgByKey);
       
        it++;
    }

    if(m_bEventCapture )
    {
        if(vResult.size() > 0 && strEvent.size() > 0)
        {
			sHeader.uChannelID = m_nChannelId;
            strEvent.insert(0,(char*)&sHeader,sizeof(SRIP_DETECT_HEADER));

            std::string strEventEx = strEvent;

            //通知录象线程进行事件录象;
            //if((m_eCapType != CAPTURE_FULL))
            {
				//LogTrace("AddEvent.log", "=aaa===m_skpRoadRecorder.AddEvent=m_eCapType=%d=g_nEncodeFormat=%d \n", m_eCapType, g_nEncodeFormat);
				if(g_nDetectMode == 2)
				m_tempRecord.AddEvent(strEventEx);
				else
                m_skpRoadRecorder.AddEvent(strEventEx);
            }
        }
    }
}

//获取录像路径
void CRoadCarnumDetect::GetVideoSavePath(RECORD_PLATE& plate,SRIP_DETECT_HEADER& sHeader)
{			
				string strVideoPath(""),strTmpPath("");
				
				/*if(m_eCapType == CAPTURE_FULL)//且存在全天录象
				{
					//直接从全天录象中获取事件录象视频
					strVideoPath = g_FileManage.GetEncodeFileName();
				}
				else*/
				{
                if(m_bEventCapture)
                {
					//需要判断磁盘是否已经满
					g_FileManage.CheckDisk(false,true);

					sHeader.uWidth = m_imgSnap->width;
					sHeader.uHeight = m_imgSnap->height-m_nExtentHeight;
					sHeader.uTimestamp = plate.uTime;
					sHeader.uTime64 =  (int64_t)plate.uTime*1000000+plate.uMiTime*1000;
					sHeader.uVideoId = g_uVideoId;
					sHeader.dFrameRate = g_fFrameRate;
					sHeader.uDetectType = SRIP_VIOLATION_VIDEO;//录像类型：违章

					if(g_nServerType == 13 && g_nFtpServer == 1)
					{
							RECORD_PLATE vPlate;
							vPlate.uTime = sHeader.uTimestamp;
							vPlate.uMiTime = (sHeader.uTime64/1000)%1000;
							vPlate.uRoadWayID = plate.uRoadWayID;
							g_MyCenterServer.GetPlatePicPath(vPlate,strVideoPath,2);
					}
					else if(g_nServerType == 7)
					{
							g_RoadImcData.GetPlatePicPath(plate,strVideoPath,3);
							printf("g_RoadImcData.GetPlatePicPath strVideoPath=%s\n",strVideoPath.c_str());
					}
					else if(g_nServerType == 29)
					{
							g_RoadImcData.GetPlatePicPath(plate,strVideoPath,7);
							printf("g_RoadImcData.GetPlatePicPath strVideoPath=%s\n",strVideoPath.c_str());
					}
					else
					{
							if(g_nDetectMode != 2)
							{
								//获取事件录象名称				
								strVideoPath = g_FileManage.GetEventVideoPath(g_uVideoId);
							}
							else
							{
								strVideoPath = g_FileManage.GetDspEventVideoPath(g_uVideoId);
							}
					}
					//LogTrace("Add-Event-Dsp.log", "=uSeq=%d=strVideoPath=%s=\n", sHeader.uSeq, strVideoPath);
				}
				}
				
				if(strVideoPath.size() > 0)
				{
					if(g_nServerType == 13 && g_nFtpServer == 1)
					{
							string strDataPath = "/home/road/dzjc";
							if (IsDataDisk())
							{
								strDataPath = "/detectdata/dzjc";
							}
							strTmpPath = strVideoPath.erase(0,strDataPath.size());
							strTmpPath = g_ServerHost+strTmpPath;
					}
					else if(g_nServerType == 7)
					{
							string strDataPath = "/home/road/red";
							if (IsDataDisk())
							{
								strDataPath = "/detectdata/red";
							}
							strTmpPath = strVideoPath.erase(0,strDataPath.size());
							strTmpPath = g_ServerHost+strTmpPath;
					}
					else
					{
							strTmpPath = strVideoPath.erase(0,g_strVideo.size());
							strTmpPath = g_ServerHost+strTmpPath;
					}

					strVideoPath = "ftp://"+strTmpPath;
					memcpy(plate.chVideoPath,strVideoPath.c_str(),strVideoPath.size());//事件录象路径
				}
}

//输出上条记录(nOutputMode == 0:上下记录关联; nOutputMode = 1:强制输出)
void CRoadCarnumDetect::OutPutPreResult(RECORD_PLATE& plate,PLATEPOSITION* pTimeStamp,int nOutputMode)
{
	//LogTrace("LogDsp.log", "=OutPutPreResult=plate.uSeq=%d=chText=%s=\n", plate.uSeq, plate.chText);
		

	//对于无牌车直接输出
    if( (g_nServerType == 3)&&(plate.uViolationType == DETECT_RESULT_NOCARNUM ||
         plate.uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME) )
    {

				//获取图片路径
				 std::string strPicPath;
				 int nSaveRet = GetPicPathAndSaveDB(strPicPath);
				 //大图存储路径
				 memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());


				//存当前记录
                plate.uPicWidth = m_imgDestSnap->width;
                plate.uPicHeight = m_imgDestSnap->height;

                CvRect rect;
                rect.x = 0;
                rect.y = 0;
                rect.width = m_imgDestSnap->width;
                rect.height = (m_imgDestSnap->height - m_imgSnap->height+m_nExtentHeight);

				if(rect.height > 0)
				{
					cvSetImageROI(m_imgDestSnap,rect);
					cvSet(m_imgDestSnap, cvScalar( 0,0, 0 ));
					cvResetImageROI(m_imgDestSnap);
				}

                //存叠加的第一张图
                rect.x = 0;
                if(m_nWordPos == 1)
                rect.y = (m_imgDestSnap->height - m_imgSnap->height);
                else
                rect.y = 0;
                rect.width = m_imgSnap->width;
                rect.height = m_imgSnap->height;
                cvSetImageROI(m_imgDestSnap,rect);
                cvCopy(m_imgSnap,m_imgDestSnap);
                cvResetImageROI(m_imgDestSnap);

                //存叠加的第二张图
                UINT32 nSeq = plate.uSeq;
                UINT32 nPreSeq = nSeq;
                int ndtSeq = 5;
                if(m_nFrequency <= 10)
                {
                    ndtSeq = 3;
                }

				if(m_nDetectDirection == 0)
				{
					nPreSeq = nSeq - ndtSeq;//前牌
				}
				else
				{
					nPreSeq = nSeq + ndtSeq;//尾牌
				}


                //LogNormal("输出当前记录,nPreSeq=%lld,nSeq=%lld",nPreSeq,nSeq);

                //存储记录的第二张图
                PLATEPOSITION  TimeStamp[2];
                bool bGetJpg = GetImageByIndex(nPreSeq,nSeq,&TimeStamp[1],m_imgPreSnap);
				if(!bGetJpg)
				{
					//LogNormal("bGetJpg 11 !");
					//return;
				}

                plate.uTime2 = TimeStamp[1].uTimestamp;
                plate.uMiTime2 = (TimeStamp[1].ts/1000)%1000;

                rect.x = m_imgSnap->width;
                if(m_nWordPos == 1)
                rect.y = (m_imgDestSnap->height - m_imgSnap->height);
                else
                rect.y = 0;
                rect.width = m_imgSnap->width;
                rect.height = m_imgSnap->height;
                cvSetImageROI(m_imgDestSnap,rect);
                cvCopy(m_imgPreSnap,m_imgDestSnap);
                cvResetImageROI(m_imgDestSnap);

                 //存储当前记录小图
                if(m_nSmallPic == 1)
                {
                    //截取小图区域
                    CvRect rtPos;

                    rtPos = GetCarPos(plate);

                    if( (rtPos.width > 0) && (rtPos.height > 0))
                    {
                        CvRect rect;
                        rect.x = m_nSaveImageCount*m_imgSnap->width;
                        if(m_nWordPos == 1)
                        rect.y = (m_imgDestSnap->height - m_imgSnap->height+m_nExtentHeight);
                        else
                        rect.y = 0;
                        rect.width = m_imgDestSnap->width - rect.x;
                        rect.height = m_imgSnap->height - m_nExtentHeight;

                        if(rect.width > 0)
                        {
                            cvSetImageROI(m_imgDestSnap,rect);
                            cvSetImageROI(m_imgSnap,rtPos);
                            cvResize(m_imgSnap,m_imgDestSnap);
                            cvResetImageROI(m_imgSnap);
                            cvResetImageROI(m_imgDestSnap);
                        }
                    }
                }

                PutTextOnComposeImage(m_imgDestSnap,plate);
                plate.uPicSize = SaveImage(m_imgDestSnap,strPicPath,2);

                //将上一条记录结果输出并写入数据库
                g_skpDB.SavePlate(m_nChannelId,plate,0,NULL);

                //将车牌信息送客户端
                if(m_bConnect)
                {
                    SendResult(plate,nSeq);
                }

                return;

	}


	if(nOutputMode == 0)//上下记录关联
    {
        plate.uPicWidth = m_imgSnap->width;
        plate.uPicHeight = m_imgSnap->height;

		//获取图片路径
        std::string strPicPath;
        int nSaveRet = GetPicPathAndSaveDB(strPicPath);
        //大图存储路径
        memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

		//存当前记录的第一张图
		//LogNormal("1 PutTextOnImage seq:%d ", plate.uSeq);
        PutTextOnImage(m_imgSnap,plate,0,pTimeStamp);

        plate.uPicSize = SaveImage(m_imgSnap,strPicPath,0);

        if(m_nDetectDirection == 0) //前牌
        {
            //存储当前记录小图
            if(m_nSmallPic == 1)
            {
                //截取小图区域
                CvRect rtPos;

                rtPos = GetCarPos(plate);

                if( (rtPos.width > 0) && (rtPos.height > 0))
                 {
                        plate.uSmallPicSize = SaveSmallImage(m_imgSnap,strPicPath,rtPos,m_pSmallJpgImage);
                 }
            }

            UINT32 nSeq = plate.uSeq;
            CarInfoMap::iterator it_c = MapCarInfo.begin();

            if(it_c != MapCarInfo.end())
            {
                RECORD_PLATE plate_pre = it_c->second;

                std::string strPicPath(plate.chPicPath);

                int ndt = 1;
                if(plate_pre.uSpeed <= 15)
                {
                    ndt = 2;
                }
                //判断当前记录与上一记录的关系
                if( (plate_pre.uTime + ndt >= plate.uTime)&&
                    (plate_pre.uTime <= plate.uTime)&& (plate_pre.uDirection == plate.uDirection))
                {
                    //取上一帧图片的第一张图以及小图
                    PicDataMap::iterator it_pd = MapPicData.begin();

                    std::string strPicData = it_pd->second;

                    //将上一条记录的第一帧作为当前记录的第二帧
                    SaveExistImage(strPicPath,(BYTE*)strPicData.c_str(),plate_pre.uPicSize);

                    plate.uTime2 = plate_pre.uTime;
                    plate.uMiTime2 = plate_pre.uMiTime;

                    //LogNormal("将上一条记录的第一帧作为当前记录的第二帧");
                }
                else
                {
                    UINT32 nPreSeq = nSeq;
                    int ndtSeq = 5;
                    if(m_nFrequency <= 10)
                    {
                        ndtSeq = 3;
                    }
					
                    nPreSeq = nSeq - ndtSeq;//前牌

                    //存储上一条记录的第二张图
                    PLATEPOSITION  TimeStamp[2];
                    bool bGetJpg = GetImageByIndex(nPreSeq,nSeq,&TimeStamp[1],m_imgPreSnap);
					if(!bGetJpg)
					{
						//LogNormal("bGetJpg 22 !");
						//return;
					}

                    plate.uTime2 = TimeStamp[1].uTimestamp;
                    plate.uMiTime2 = (TimeStamp[1].ts/1000)%1000;

					//LogNormal("2 PutTextOnImage seq:%d ", plate.uSeq);
                    PutTextOnImage(m_imgPreSnap,plate,1,TimeStamp);
                    SaveImage(m_imgPreSnap,strPicPath,1);
                }

                 //清空记录
                MapCarInfo.clear();
                //清空图片
                MapPicData.clear();
            }
            else
            {
                    //输出当前记录

                    UINT32 nPreSeq = nSeq;
                    int ndtSeq = 5;
                    if(m_nFrequency <= 10)
                    {
                        ndtSeq = 3;
                    }
					
                    nPreSeq = nSeq - ndtSeq;//前牌

                   // LogNormal("输出上一条记录,nPreSeq=%lld,nSeq=%lld",nPreSeq,nSeq);

                    //存储上一条记录的第二张图
                    PLATEPOSITION  TimeStamp[2];
                    bool bGetJpg = GetImageByIndex(nPreSeq,nSeq,&TimeStamp[1],m_imgPreSnap);
					if(!bGetJpg)
					{
						//LogNormal("bGetJpg 33 !");
						//return;
					}

                    plate.uTime2 = TimeStamp[1].uTimestamp;
                    plate.uMiTime2 = (TimeStamp[1].ts/1000)%1000;

					//LogNormal("3 PutTextOnImage seq:%d ", plate.uSeq);
                    PutTextOnImage(m_imgPreSnap,plate,1,TimeStamp);
                    SaveImage(m_imgPreSnap,strPicPath,1);
            }

            //存储小图
                if(m_nSmallPic == 1)
                {
                    SaveExistImage(strPicPath,m_pSmallJpgImage,plate.uSmallPicSize);
                }

                //将当前记录结果输出并写入数据库
                g_skpDB.SavePlate(m_nChannelId,plate,0,NULL);

                //将车牌信息送客户端
                if(m_bConnect)
                {
                    SendResult(plate,nSeq);
                }

                //需要将当前帧标记为已经输出，防止被再次输出
                plate.uSeq = 0;//plate.uSeq为0表示已经输出



            //暂存第一帧
            std::string strPicData;
            strPicData.append((char*)m_pCurJpgImage,plate.uPicSize);
            MapPicData.insert(PicDataMap::value_type(nSeq,strPicData));

            //暂存记录
            MapCarInfo.insert(CarInfoMap::value_type(nSeq,plate));
        }
        else if(m_nDetectDirection == 1) //尾牌
        {
				CarInfoMap::iterator it_c = MapCarInfo.begin();
				if(it_c != MapCarInfo.end())
				{
					UINT32 nPreSeq = it_c->first;

					RECORD_PLATE plate_pre = it_c->second;

					//获取图片路径
					std::string strPrePicPath(plate_pre.chPicPath);

					int ndt = 1;
					if(plate_pre.uSpeed <= 15)
					{
						ndt = 2;
					}

					//判断当前记录与上一记录的关系
					if( (plate_pre.uTime + ndt >= plate.uTime) &&
						 (plate_pre.uTime <= plate.uTime) && (plate_pre.uDirection == plate.uDirection))
					{
					    LogNormal("上下记录关联输出,nPreSeq=%lld,nSeq=%lld",nPreSeq,plate.uSeq);

						//将当前图片作为上一条记录的第二张图片
						SaveExistImage(strPrePicPath,m_pCurJpgImage,plate.uPicSize);

						plate_pre.uTime2 = plate.uTime;
						plate_pre.uMiTime2 = plate.uMiTime;
					}
					else //输出上一条记录
					{
						   //存储上一张的第二张图
							UINT32 nPreSecondSeq = nPreSeq;

							int ndtSeq = 5;
							if(m_nFrequency <= 10)
							{
								ndtSeq = 3;
							}
							
							nPreSecondSeq = nPreSeq + ndtSeq;//尾牌

							LogNormal("记录未能关联输出,nPreSecondSeq=%lld,nPreSeq=%lld,nSeq=%lld",nPreSecondSeq,nPreSeq,plate.uSeq);

							PLATEPOSITION  TimeStamp[2];
							bool bGetJpg = GetImageByIndex(nPreSecondSeq,nPreSeq,&TimeStamp[1],m_imgPreSnap);
							if(!bGetJpg)
							{
								//LogNormal("bGetJpg 44 !");
								//return;
							}


						   //存第二张图
							//LogNormal("4 PutTextOnImage seq:%d ", plate_pre.uSeq);
						   PutTextOnImage(m_imgPreSnap,plate_pre,1,TimeStamp);
						   SaveImage(m_imgPreSnap,strPrePicPath,1);


							plate_pre.uTime2 = TimeStamp[1].uTimestamp;
							plate_pre.uMiTime2 = (TimeStamp[1].ts/1000)%1000;
					}

					//存储上一张的小图
					if(m_nSmallPic == 1)
					{
						SaveExistImage(strPrePicPath,m_pSmallJpgImage,plate_pre.uSmallPicSize);
					}

					//将上一帧结果输出并写入数据库
					g_skpDB.SavePlate(m_nChannelId,plate_pre,0,NULL);

				   //将车牌信息送客户端
					if(m_bConnect)
					{
						SendResult(plate_pre,nPreSeq);
					}

					//清空记录
					MapCarInfo.clear();
				}


			   //是否需要存储小图
				if(m_nSmallPic == 1)
				{
					   //截取小图区域
						CvRect rtPos;
						rtPos = GetCarPos(plate);
						if( (rtPos.width > 0) && (rtPos.height > 0))
						{
							 plate.uSmallPicSize = SaveSmallImage(m_imgSnap,strPicPath,rtPos,m_pSmallJpgImage);
						}

				}

				//暂存记录
				MapCarInfo.insert(CarInfoMap::value_type(plate.uSeq,plate));
        }
    }
    else //强制输出
    {
		//LogNormal("in OutPutPreResult 强制输出\n");//add by wantao
        CarInfoMap::iterator it_c = MapCarInfo.begin();
        if(it_c != MapCarInfo.end())
        {
            RECORD_PLATE plate_pre = it_c->second;

            int ndt = 2;

            if(pTimeStamp->uTimestamp >= plate_pre.uTime + ndt)//如果2秒还未输出则输出
            {
                if(plate_pre.uSeq > 0) //如果上一帧未输出则输出
                {
                     //获取图片路径
                    std::string strPrePicPath(plate_pre.chPicPath);

                     //存储上一张的第二张图
                    UINT32 nPreSeq = it_c->first;

                    UINT32 nPreSecondSeq = nPreSeq;

                    int ndtSeq = 5;
                    if(m_nFrequency <= 10)
                    {
                        ndtSeq = 3;
                    }

                    if(m_nDetectDirection == 0)
                    {
                        nPreSecondSeq = nPreSeq - ndtSeq;//前牌
                    }
                    else
                    {
                        nPreSecondSeq = nPreSeq + ndtSeq;//尾牌
                    }

                    //LogNormal("强制输出,t1=%lld,t2=%lld",pTimeStamp->uTimestamp,plate_pre.uTime);

                    PLATEPOSITION  TimeStamp[2];
                    bool bGetJpg = GetImageByIndex(nPreSecondSeq,nPreSeq,&TimeStamp[1],m_imgPreSnap);
					if(!bGetJpg)
					{
						//LogNormal("bGetJpg 55 !");
						//return;
					}

                    plate_pre.uTime2 = TimeStamp[1].uTimestamp;
                    plate_pre.uMiTime2 = (TimeStamp[1].ts/1000)%1000;

                    //存第二张图
					//LogNormal("5 PutTextOnImage seq:%d ", plate_pre.uSeq);
                    PutTextOnImage(m_imgPreSnap,plate_pre,1,TimeStamp);
                    SaveImage(m_imgPreSnap,strPrePicPath,1);

                    //存储上一张的小图
                    if(m_nSmallPic == 1)
                    {
                        SaveExistImage(strPrePicPath,m_pSmallJpgImage,plate_pre.uSmallPicSize);
                    }

                     //将上一帧结果输出并写入数据库
                     g_skpDB.SavePlate(m_nChannelId,plate_pre,0,NULL);

                    //将车牌信息送客户端
                    if(m_bConnect)
                    {
                        SendResult(plate_pre,nPreSeq);
                    }
                }
                //清空记录
                MapCarInfo.clear();

                MapPicData.clear();
            }
        }
    }
}

//输出线圈检测结果
void CRoadCarnumDetect::OutPutLoopResult(CarInfo& cardNum,RECORD_PLATE& plate,std::vector<ObjSyncData>::iterator it_s,PLATEPOSITION*  TimeStamp,bool bLightImage,bool bLoop)
{				
			#ifndef ALGORITHM_YUV
				if(m_nSaveImageCount == 1)
                {
				    //截取小图区域
                     if(m_nSmallPic == 1)
                     {
						 //小图
						 RECORD_PLATE plate_r = plate;
						 plate_r.uPosLeft = cardNum.ix;
						 plate_r.uPosTop = cardNum.iy;
						 plate_r.uPosRight = cardNum.ix+cardNum.iwidth-1;
						 plate_r.uPosBottom = cardNum.iy+cardNum.iheight-1;
						 if(m_nWordPos == 1)
						 {
							 plate_r.uPosTop += m_nExtentHeight;
							 plate_r.uPosBottom += m_nExtentHeight;
						 }
						 CvRect rtPos;

						 if(bLoop)
						 {
							 rtPos = m_rtCarnumROI;
						 }
						 else
						 {
							rtPos = GetCarPos(plate_r);
						 }

						 if(g_nPicMode == 1)//分开存储
						 {
							 //先叠加车身图片
							 CvRect rect;
							 rect.x = 0;
                             rect.y = 0;
                             rect.width = m_imgSnap->width;
                             rect.height = m_imgSnap->height;

                             cvSetImageROI(m_imgDestSnap,rect);
							 cvCopy(m_imgSnap,m_imgDestSnap);
							 cvResetImageROI(m_imgDestSnap);

							 rect.x = m_imgSnap->width;
                             rect.y = 0;
                             rect.width = m_imgDestSnap->width - rect.x;
                             rect.height = m_imgSnap->height - m_nExtentHeight;

							 if(rect.width > 0)
                             {
                                 cvSetImageROI(m_imgDestSnap,rect);
                                 if( (rtPos.width > 0) && (rtPos.height > 0))
                                 {
                                     cvSetImageROI(m_imgSnap,rtPos);
                                     cvResize(m_imgSnap,m_imgDestSnap);
                                     cvResetImageROI(m_imgSnap);
                                 }
                                 cvResetImageROI(m_imgDestSnap);
                             }

							rtPos.x= plate_r.uPosLeft - 5;
							rtPos.y= plate_r.uPosTop - 5;
							rtPos.width= plate_r.uPosRight - plate_r.uPosLeft + 10;
							rtPos.height= plate_r.uPosBottom - plate_r.uPosTop + 20;
						 }

						 if( (rtPos.width > 0) && (rtPos.height > 0))
						 {
							  if(g_nHasHighLight == 1)
							  {

								 plate.uSmallPicSize = SaveSmallImage(m_imgPreSnap,"",rtPos,m_pSmallJpgImage);
							  }
							  else
							  {
								plate.uSmallPicSize = SaveSmallImage(m_imgSnap,"",rtPos,m_pSmallJpgImage);
							  }
						 }

						 if(g_nPicMode == 1)//分开存储
						 {
							 OutPutWriteBack(cardNum,plate,it_s,m_imgDestSnap,bLightImage);
						 }
						 else
						 {
							 OutPutWriteBack(cardNum,plate,it_s,m_imgSnap,bLightImage);
						 }
                     }
					 else
					 {
						 OutPutWriteBack(cardNum,plate,it_s,m_imgSnap);
					 }
                }
                else  if(m_nSaveImageCount == 2)
                {
                   if(g_nPicMode == 0)
                   {
                       CarInfoMap::iterator it_c = MapCarInfo.find(cardNum.id);
                       if(it_c == MapCarInfo.end())//没有找到之前的记录
                       {
                            UINT32 nPreSeq = cardNum.uSeq - 1;

							if(nPreSeq == cardNum.m_useq)
							{
								nPreSeq--;
							}

                            bool bGetJpg = GetImageByIndex(nPreSeq,cardNum.uSeq,&TimeStamp[1],m_imgPreSnap);
							if(!bGetJpg)
							{
								//LogNormal("bGetJpg 66 !");
								//return;
							}

                           for(int nIndex = 0; nIndex <m_nSaveImageCount ; nIndex++)
                           {
                                CvRect rect;
                                rect.x = m_imgSnap->width*nIndex;
                                rect.y = 0;
                                rect.width = m_imgSnap->width;
                                rect.height = m_imgSnap->height;

                                cvSetImageROI(m_imgDestSnap,rect);
                                if(nIndex == 0)
                                cvCopy(m_imgSnap,m_imgDestSnap);
                                else
                                cvCopy(m_imgPreSnap,m_imgDestSnap);
                                cvResetImageROI(m_imgDestSnap);
                            }
                            OutPutWriteBack(cardNum,plate,it_s,m_imgDestSnap);
                       }
                       else
                       {
						   if(bLightImage)
						   {
							   UINT32 nPreSeq = cardNum.uSeq - 1;

							   if(nPreSeq == cardNum.m_useq)
							   {
								  nPreSeq--;
							   }

								bool bGetJpg = GetImageByIndex(cardNum.uSeq, nPreSeq, &TimeStamp[1],m_imgPreSnap);
								if(!bGetJpg)
								{
									//LogNormal("bGetJpg 77 !");
									//return;
								}

							   for(int nIndex = 0; nIndex <m_nSaveImageCount ; nIndex++)
							   {
									CvRect rect;
									rect.x = m_imgSnap->width*nIndex;
									rect.y = 0;
									rect.width = m_imgSnap->width;
									rect.height = m_imgSnap->height;

									cvSetImageROI(m_imgDestSnap,rect);
									if(nIndex == 0)
									cvCopy(m_imgSnap,m_imgDestSnap);
									else
									cvCopy(m_imgPreSnap,m_imgDestSnap);
									cvResetImageROI(m_imgDestSnap);
								}
						   }

                            OutPutWriteBack(cardNum,plate,it_s,m_imgDestSnap,bLightImage);
                       }
                   }
                }
		#endif
}

//输出回写队列中的记录
void CRoadCarnumDetect::OutPutWriteBack(CarInfo& cardNum,RECORD_PLATE& plate,std::vector<ObjSyncData>::iterator it_s,IplImage* pImage,bool bLightImage)
{
	#ifndef ALGORITHM_YUV
    #ifdef WRITE_BACK
    FILE * fpWB = fopen("WRITE_BACK.txt","a");
    #endif
    if(cardNum.id > -1)
    {
        CarInfoMap::iterator it_c = MapCarInfo.find(cardNum.id);
        PicDataMap::iterator it_pd = MapPicData.find(cardNum.id);
        if(it_c != MapCarInfo.end()&&(it_pd != MapPicData.end()))//找到需要将前面的结果记录改写
        {
            RECORD_PLATE plate_pre = it_c->second;
            plate_pre.uSpeed = plate.uSpeed;
            plate_pre.uType = plate.uType;

            PLATEPOSITION  TimeStamp[2];
            //获取图片路径
            std::string strPicPath;
            int nSaveRet = GetPicPathAndSaveDB(strPicPath);
            //大图存储路径
            memcpy(plate_pre.chPicPath,strPicPath.c_str(),strPicPath.size());

            //如果上一次未检测到车牌，这次检测到车牌则替换掉之前的
            if( (plate_pre.chText[0]=='*')&&(plate.chText[0]!='*') )
            {
                memset(plate_pre.chText,0,sizeof(plate_pre.chText));
                memcpy(plate_pre.chText,plate.chText,sizeof(plate.chText));
                plate_pre.uPosLeft = plate.uPosLeft;
                plate_pre.uPosRight = plate.uPosRight;
                plate_pre.uPosTop = plate.uPosTop;
                plate_pre.uPosBottom = plate.uPosBottom;
                plate_pre.uPicWidth = plate.uPicWidth;
                plate_pre.uPicHeight = plate.uPicHeight;
                plate_pre.uCarColor1 = plate.uCarColor1;
                plate_pre.uCarColor2 = plate.uCarColor2;
                plate_pre.uWeight1 = plate.uWeight1;
                plate_pre.uWeight2 = plate.uWeight2;
                plate_pre.uTypeDetail = plate.uTypeDetail;
                plate_pre.uPlateType = plate.uPlateType;
                plate_pre.uColor = plate.uColor;
                plate_pre.uCredit = plate.uCredit;
                plate_pre.uCarBrand = plate.uCarBrand;
                plate_pre.uTime = plate.uTime;
                plate_pre.uMiTime = plate.uMiTime;

                TimeStamp[0].uTimestamp = plate_pre.uTime;

                if(m_nSaveImageCount == 1)
                {
					if(m_nSmallPic == 1)
					{
						if(bLightImage)
						{
							if(g_nPicMode != 1)
							{
								//LogNormal("6 PutTextOnImage seq:%d ", plate_pre.uSeq);
								PutTextOnImage(pImage,plate_pre,0,TimeStamp);
							}
							else
							{
								PutTextOnComposeImage(pImage,plate_pre);
							}
							plate_pre.uPicSize = SaveImage(pImage,strPicPath,0);
							LogNormal("上次未检测到车牌，本次检测到车牌");
						}
						else
						{
							std::string strPicData = it_pd->second;
							cvSetData(m_imgWriteBack,(unsigned char*)strPicData.c_str(),m_imgWriteBack->widthStep);
							
							if(g_nPicMode != 1)
							{
								//LogNormal("7 PutTextOnImage seq:%d ", plate_pre.uSeq);
								PutTextOnImage(m_imgWriteBack,plate_pre,0,TimeStamp);
							}
							else
							{
								PutTextOnComposeImage(m_imgWriteBack,plate_pre);
							}
							plate_pre.uPicSize = SaveImage(m_imgWriteBack,strPicPath,0);
						}

						if(plate.uSmallPicSize > 0)
						{
							plate_pre.uSmallPicSize = plate.uSmallPicSize;
							SaveExistImage(strPicPath,m_pSmallJpgImage,plate.uSmallPicSize);
						}
					}
					else
					{
						//取当前图片
						//LogNormal("8 PutTextOnImage seq:%d ", plate_pre.uSeq);
						PutTextOnImage(pImage,plate_pre,0,TimeStamp);
						plate_pre.uPicSize = SaveImage(pImage,strPicPath,0);
					}
                }
                else if(m_nSaveImageCount == 2)
                {
					 if(bLightImage)
					 {
						 //LogNormal("9 PutTextOnImage seq:%d ", plate_pre.uSeq);
						PutTextOnImage(pImage,plate_pre,0,TimeStamp);
						plate_pre.uPicSize = SaveImage(pImage,strPicPath,0);
					 }
					 else
					 {
						 std::string strPicData = it_pd->second;
						cvSetData(m_imgWriteBack,(unsigned char*)strPicData.c_str(),m_imgWriteBack->widthStep);

						//LogNormal("10 PutTextOnImage seq:%d ", plate_pre.uSeq);
						PutTextOnImage(m_imgWriteBack,plate_pre,0,TimeStamp);
						plate_pre.uPicSize = SaveImage(m_imgWriteBack,strPicPath,0);
					 }
                }
            }
            else
            {
                //取上一张图
                TimeStamp[0].uTimestamp = plate_pre.uTime;

				if( (m_nSaveImageCount == 2 || m_nSmallPic == 1) && (bLightImage))
				{
					if(g_nPicMode != 1)
					{
						//LogNormal("11 PutTextOnImage seq:%d ", plate_pre.uSeq);
						PutTextOnImage(pImage,plate_pre,0,TimeStamp);
					}
					else
					{
						PutTextOnComposeImage(pImage,plate_pre);
					}
					plate_pre.uPicSize = SaveImage(pImage,strPicPath,0);

					if(m_nSmallPic == 1)
					{
						std::string strPicData = it_pd->second;
						SaveExistImage(strPicPath,(unsigned char*)strPicData.c_str()+m_imgWriteBack->imageSize,plate_pre.uSmallPicSize);
					}

					if(bLightImage)
					{
						plate_pre.uCarColor1 = plate.uCarColor1;
						plate_pre.uCarColor2 = plate.uCarColor2;
						plate_pre.uWeight1 = plate.uWeight1;
						plate_pre.uWeight2 = plate.uWeight2;
						plate_pre.uCarBrand = plate.uCarBrand;
					}

					LogNormal("上次检测到车牌，本次未检测到车牌但有亮图");
				}
				else
				{
					std::string strPicData = it_pd->second;
					cvSetData(m_imgWriteBack,(unsigned char*)strPicData.c_str(),m_imgWriteBack->widthStep);
					
					if(g_nPicMode != 1)
					{
						//LogNormal("12 PutTextOnImage seq:%d ", plate_pre.uSeq);
						PutTextOnImage(m_imgWriteBack,plate_pre,0,TimeStamp);
					}
					else
					{
						PutTextOnComposeImage(m_imgWriteBack,plate_pre);
					}
					plate_pre.uPicSize = SaveImage(m_imgWriteBack,strPicPath,0);

					if(m_nSmallPic == 1)
					{
						if(plate_pre.uSmallPicSize > 0)
						{
							SaveExistImage(strPicPath,(unsigned char*)strPicData.c_str()+m_imgWriteBack->imageSize,plate_pre.uSmallPicSize);
						}
					}
				}
            }
            std::string strPreCarnum(plate_pre.chText);
            #ifdef WRITE_BACK
            fprintf(fpWB,"cardNum.id = %d,plate_pre.uTime=%lld, plate.uTime=%lld\n",cardNum.id,plate_pre.uTime,plate.uTime);
            #endif

            //更新同步信息
            SYN_CHAR_DATA syn_char_data;

            if(nSaveRet>0)
            g_skpDB.SavePlate(m_nChannelId,plate_pre,0,&syn_char_data);


            //将车牌信息送客户端
            if(m_bConnect)
            {
                memcpy(plate_pre.chText,strPreCarnum.c_str(),strPreCarnum.size());
                SendResult(plate_pre,cardNum.uSeq);
            }
            MapCarInfo.erase(it_c);
            MapPicData.erase(it_pd);
        }
        else
        {
            //暂存记录
            MapCarInfo.insert(CarInfoMap::value_type(cardNum.id,plate));

            std::string strPicData;
            strPicData.append((char*)pImage->imageData,pImage->imageSize);

			if(m_nSmallPic == 1)
			{
				if(plate.uSmallPicSize > 0)
				{
					strPicData.append((char*)m_pSmallJpgImage,plate.uSmallPicSize);
				}
			}

            MapPicData.insert(PicDataMap::value_type(cardNum.id,strPicData));
        }
    }
    else//强制输出
    {
        CarInfoMap::iterator it_c = MapCarInfo.begin();
        if(it_c != MapCarInfo.end())
        {
            RECORD_PLATE plate_pre = it_c->second;

            cardNum.uTimestamp += 2;
            if(cardNum.uTimestamp >= plate_pre.uTime + g_nWriteBackTime)//如果1秒还未输出则输出
            {
                //获取图片路径
                std::string strPicPath;
                int nSaveRet = GetPicPathAndSaveDB(strPicPath);
                //大图存储路径
                memcpy(plate_pre.chPicPath,strPicPath.c_str(),strPicPath.size());

                cardNum.id = it_c->first;
                PicDataMap::iterator it_pd = MapPicData.find(cardNum.id);
                if(it_pd!=MapPicData.end())
                {
                    //取上一张图
                    PLATEPOSITION  TimeStamp[2];
                    TimeStamp[0].uTimestamp = plate_pre.uTime;

                    std::string strPicData = it_pd->second;
                    cvSetData(m_imgWriteBack,(unsigned char*)strPicData.c_str(),m_imgWriteBack->widthStep);

					if(g_nPicMode != 1)
					{
						//LogNormal("13 PutTextOnImage seq:%d ", plate_pre.uSeq);
						PutTextOnImage(m_imgWriteBack,plate_pre,0,TimeStamp);
					}
					else
					{
						PutTextOnComposeImage(m_imgWriteBack,plate_pre);
					}
                    plate_pre.uPicSize = SaveImage(m_imgWriteBack,strPicPath,0);

					if(m_nSmallPic == 1)
					{
						if(plate_pre.uSmallPicSize > 0)
						SaveExistImage(strPicPath,(unsigned char*)strPicData.c_str()+m_imgWriteBack->imageSize,plate_pre.uSmallPicSize);
					}

                    MapPicData.erase(it_pd);
                }

                std::string strPreCarnum(plate_pre.chText);

                #ifdef WRITE_BACK
                fprintf(fpWB,"force cardNum.id = %d,cardNum.uTimestamp = %lld,plate_pre.uTime=%lld\n",cardNum.id,cardNum.uTimestamp,plate_pre.uTime);
                #endif

                //更新同步信息
                SYN_CHAR_DATA syn_char_data;

                if(nSaveRet > 0)
                g_skpDB.SavePlate(m_nChannelId,plate_pre,0,&syn_char_data);


                //将车牌信息送客户端
                if(m_bConnect)
                {
                    memcpy(plate_pre.chText,strPreCarnum.c_str(),strPreCarnum.size());
                    SendResult(plate_pre,cardNum.uSeq);
                }
                MapCarInfo.erase(it_c);
            }
        }
    }
    #ifdef WRITE_BACK
    fclose(fpWB);
    #endif
	#endif
}

//车型检测
void CRoadCarnumDetect::DetectTruck(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context,CvRect rtRealImage,IplImage* pImage)
{	
	 //车牌位置合理性判断
	if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= pImage->width || cardNum.iy >= pImage->height - m_nExtentHeight
		  || (cardNum.ix + cardNum.iwidth >= pImage->width) || (cardNum.iy + cardNum.iheight >= pImage->height - m_nExtentHeight))
	{
		
		LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);

		return;
    }

	#ifdef GLOBALCARLABEL
	if(!(m_nDetectDirection == 1 && plate.uColor == CARNUM_YELLOW && cardNum.strCarNum[6] != '$'))//尾牌为黄牌且不为“学“字则进入车型模块
	{
		return;
	}
	#endif

	#ifndef ALGORITHM_YUV
              int nRet = 0;
			  int nMoreDetail = 0;
              //if((cardNum.nDirection == 0) || (m_nVedioFormat == VEDIO_PICLIB))//如果是图库识别要做车辆类型检测就一定要是前排
              {

                        std::string strNumber;
                        strNumber = cardNum.strCarNum;
                        if(*(strNumber.c_str()+strNumber.size()-1)!='$')
                        {
							struct timeval tv1,tv2;
							if(g_nPrintfTime == 1)
							{
								gettimeofday(&tv1,NULL);
							}
							
                            //LogNormal("before mvTruckDetect m_imgSnap(%d,%d),m_nDayNight=%d\n",m_imgSnap->width,m_imgSnap->height,m_nDayNight);
							//由于车牌判别和卡车类型细分用的不是同一个图片，在传入该图片的同时，需要设置ROI区域；
							if((m_nDayNightbyLight == 0)&&(g_nHasHighLight == 1)&& (cardNum.m_UseShutter))
							{
								if(cardNum.m_CarnumPos.width <= 0 || cardNum.m_CarnumPos.height <= 0 || cardNum.m_CarnumPos.x <= 0 || cardNum.m_CarnumPos.y <= 0 || cardNum.m_CarnumPos.x >= m_imgSnap->width || cardNum.m_CarnumPos.y >= m_imgSnap->height - m_nExtentHeight
										  || (cardNum.m_CarnumPos.x + cardNum.m_CarnumPos.width >= m_imgSnap->width) || (cardNum.m_CarnumPos.y + cardNum.m_CarnumPos.height >= m_imgSnap->height - m_nExtentHeight))
									   {
										  
											LogError("爆闪车牌位置不合理,%d,%d,%d,%d\n",cardNum.m_CarnumPos.x,cardNum.m_CarnumPos.y,cardNum.m_CarnumPos.width,cardNum.m_CarnumPos.height);
											return;
									   }

										if(cardNum.m_CarnumPos.width > 0)
										{
											context.position.x = cardNum.m_CarnumPos.x;
											context.position.y = cardNum.m_CarnumPos.y;
											context.position.width = cardNum.m_CarnumPos.width;
											context.position.height = cardNum.m_CarnumPos.height;
										}

								pImage = m_imgLightSnap;
							}
							else
							{
								pImage = m_imgSnap;
							}
                            cvSetImageROI(pImage,rtRealImage);

							bool IsForeCarnum=true;
							if(m_nDetectDirection == 1)
							{
								IsForeCarnum = false;
							}
							//LogNormal("width=%d,height=%d,m_nDayNight=%d,IsForeCarnum=%d\n",\
							//	rtRealImage.width,rtRealImage.height,m_nDayNight,IsForeCarnum);
							#ifndef GLOBALCARCLASSIFY
							printf("m_vehicleClassify.mvTruckDetect \n");
                            nRet = m_vehicleClassify.mvTruckDetect(pImage,context,m_nDayNightbyLight,IsForeCarnum);							
							#else
							pthread_mutex_lock(&g_vehicleClassifyMutex);
							nRet = g_vehicleClassify.mvTruckDetect(pImage,context,m_nDayNightbyLight,IsForeCarnum,true,&nMoreDetail);
							pthread_mutex_unlock(&g_vehicleClassifyMutex);
							#endif
                            //LogNormal("after mvTruckDetect\n");
                            cvResetImageROI(pImage);

							if(g_nPrintfTime == 1)
							{
								gettimeofday(&tv2,NULL);
								FILE* fp = fopen("time.log","ab+");
								fprintf(fp,"mvTruckDetect==t1=%lld,t2=%lld,dt = %lld\n",(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
								fclose(fp);
							}
                        }
						cardNum.subVehicleType = nRet;
						//it->subVehicleType = nRet;
                }

               plate.uTypeDetail = GetTypeDetail(nRet);
			   plate.uDetailCarType = nMoreDetail;
	#endif
}

//车身颜色检测
void CRoadCarnumDetect::CarColorDetect(bool bCarNum,bool bLoop,CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context,CvPoint& loopPtUp,CvPoint& loopPtDowon,CvRect& rtRoi)
{	
	 //车牌位置合理性判断
	if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= m_imgSnap->width || cardNum.iy >= m_imgSnap->height - m_nExtentHeight
		  || (cardNum.ix + cardNum.iwidth >= m_imgSnap->width) || (cardNum.iy + cardNum.iheight >= m_imgSnap->height - m_nExtentHeight))
	{

		LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
			return;
    }

	#ifndef ALGORITHM_YUV
	object_color objColor;

	if(bCarNum||bLoop)
	{
				struct timeval tv1,tv2;
				if(g_nPrintfTime == 1)
				{
					gettimeofday(&tv1,NULL);
				}

				 // cvSetImageROI(m_imgSnap,rtRealImage);
				  //mvGetCarColor(m_imgSnap,context,rtRoi,&objColor,m_nDayNight,bCarNum);
				  int nExtentHeight = 0;
				  if(m_nWordPos == 1)
				  {
						nExtentHeight = m_nExtentHeight;
				   }

					if(m_nDayNightbyLight == 0)
					{
									if((g_nHasHighLight == 1)&& (cardNum.m_UseShutter) )//有爆闪灯晚上亮图未检测出车牌
									{
										if(cardNum.m_CarnumPos.width <= 0 || cardNum.m_CarnumPos.height <= 0 || cardNum.m_CarnumPos.x <= 0 || cardNum.m_CarnumPos.y <= 0 || cardNum.m_CarnumPos.x >= m_imgSnap->width || cardNum.m_CarnumPos.y >= m_imgSnap->height - m_nExtentHeight
										  || (cardNum.m_CarnumPos.x + cardNum.m_CarnumPos.width >= m_imgSnap->width) || (cardNum.m_CarnumPos.y + cardNum.m_CarnumPos.height >= m_imgSnap->height - m_nExtentHeight))
									   {
										  
											LogError("爆闪车牌位置不合理,%d,%d,%d,%d\n",cardNum.m_CarnumPos.x,cardNum.m_CarnumPos.y,cardNum.m_CarnumPos.width,cardNum.m_CarnumPos.height);
											return;
									   }

										if(cardNum.m_CarnumPos.width > 0)
										{
											context.position.x = cardNum.m_CarnumPos.x;
											context.position.y = cardNum.m_CarnumPos.y;
											context.position.width = cardNum.m_CarnumPos.width;
											context.position.height = cardNum.m_CarnumPos.height;
										}

										vector<LoopInfo>::iterator it_lb = m_LoopInfo.begin();
										vector<LoopInfo>::iterator it_le = m_LoopInfo.end();
										while(it_lb!=it_le)
										{
											if(it_lb->nRoadIndex == cardNum.RoadIndex || cardNum.RoadIndex <0)
											{
												loopPtUp.x = it_lb->line0.end.x;
												loopPtUp.y = it_lb->line0.end.y;
												loopPtDowon.x = it_lb->line1.end.x;
												loopPtDowon.y = it_lb->line1.end.y;
												break;
											}
											it_lb++;
										}
											
										#ifndef ALGORITHM_DL
										MvLisenceDetect LisenceDetect;
										LisenceDetect.mvDetectLisencePosition(m_imgLightSnap->imageData+(m_imgLightSnap->widthStep*nExtentHeight),m_imgLightSnap->width,m_imgLightSnap->height-m_nExtentHeight,loopPtUp,loopPtDowon,context);
										#endif

										#ifndef GLOBALCARCOLOR
										m_carColor.mvGetCardCarColor(m_imgLightSnap->imageData+(m_imgLightSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNightbyLight,bCarNum,true);
										#else
										pthread_mutex_lock(&g_carColorMutex);
										g_carColor.mvSetImageWidthandHeight(m_imgLightSnap->width,m_imgLightSnap->height);
										#ifdef ALGORITHM_DL
										g_carColor.mvDLCardCarColor(m_imgLightSnap,&context,&rtRoi,&objColor,m_nDayNightbyLight);
										#else
										g_carColor.mvGetCardCarColor(m_imgLightSnap->imageData+(m_imgLightSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNightbyLight,bCarNum,true);
										#endif
										pthread_mutex_unlock(&g_carColorMutex);
										#endif
									}
									else
									{
										#ifndef GLOBALCARCOLOR
										m_carColor.mvGetCardCarColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNightbyLight,bCarNum,true);
										#else
										pthread_mutex_lock(&g_carColorMutex);
										g_carColor.mvSetImageWidthandHeight(m_imgSnap->width,m_imgSnap->height);
										#ifdef ALGORITHM_DL

										printf("before mvDLCardCarColor\n");
	
										g_carColor.mvDLCardCarColor(m_imgSnap,NULL,&rtRoi,&objColor,m_nDayNightbyLight);

										printf("after mvDLCardCarColor objColor.nColor1=%d\n",objColor.nColor1);

										#else
										g_carColor.mvGetCardCarColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNightbyLight,bCarNum,true);
										#endif
										pthread_mutex_unlock(&g_carColorMutex);										
										#endif
									}
					}
					else
					{				
									#ifndef GLOBALCARCOLOR
									m_carColor.mvGetCardCarColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNightbyLight,bCarNum);
									#else
									pthread_mutex_lock(&g_carColorMutex);
									g_carColor.mvSetImageWidthandHeight(m_imgSnap->width,m_imgSnap->height);
									#ifdef ALGORITHM_DL

									//printf("before mvDLCardCarColor m_nDayNightbyLight = %d\n",m_nDayNightbyLight);

									g_carColor.mvDLCardCarColor(m_imgSnap,NULL,&rtRoi,&objColor,m_nDayNightbyLight);

									//printf("before mvDLCardCarColor m_nDayNightbyLight = %d\n",m_nDayNightbyLight);
									#else
									g_carColor.mvGetCardCarColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNightbyLight,bCarNum);
									#endif
									pthread_mutex_unlock(&g_carColorMutex);
									#endif
					}


					// printf("after mvGetCarColor\n");
				   //cvResetImageROI(m_imgSnap);
					if(g_nPrintfTime == 1)
					{
									gettimeofday(&tv2,NULL);
									FILE* fp = fopen("time.log","ab+");
									fprintf(fp,"mvGetCardCarColor==t1=%lld,t2=%lld,dt = %lld\n",(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
									fclose(fp);
					}

                    plate.uCarColor1 = objColor.nColor1;
                    plate.uWeight1 = objColor.nWeight1;
                    plate.uCarColor2 = objColor.nColor2;
                    plate.uWeight2 = objColor.nWeight2;

                   // it->objColor = objColor;
       }
        else//获取非机动车及行人颜色
       {
					if(g_nDetectMode == 2)
					{
						int nExtentHeight = 0;
						if(m_nWordPos == 1)
						{
							nExtentHeight = m_nExtentHeight;
						}
						#ifndef GLOBALCARCOLOR
						m_carColor.mvGetDSP_PasserbyColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context.position,&objColor);
						#else
						#ifndef ALGORITHM_DL
						pthread_mutex_lock(&g_carColorMutex);
						g_carColor.mvSetImageWidthandHeight(m_imgSnap->width,m_imgSnap->height);
						g_carColor.mvGetDSP_PasserbyColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context.position,&objColor);
						pthread_mutex_unlock(&g_carColorMutex);
						#endif
						#endif

						plate.uCarColor1 = objColor.nColor1;
						plate.uWeight1 = objColor.nWeight1;
						plate.uCarColor2 = objColor.nColor2;
						plate.uWeight2 = objColor.nWeight2;

					//	it->objColor = objColor;
					}
					else
					{
						plate.uCarColor1 = cardNum.objColor.nColor1;
						plate.uWeight1 = cardNum.objColor.nWeight1;
						plate.uCarColor2 = cardNum.objColor.nColor2;
						plate.uWeight2 = cardNum.objColor.nWeight2;
					}
		}
#endif
}

#ifdef BELTDETECT
//叠加人脸图片
void CRoadCarnumDetect::PutFaceOnImage(vector<FaceRt>& vecP)
{
	vector<FaceRt>::iterator it = vecP.begin();
	int nIndex = 0;
	while(it != vecP.end())
	{
		CvRect rtFace;
		rtFace.x = it->VehicleRect.x;
		rtFace.y = it->VehicleRect.y;
		rtFace.width = it->VehicleRect.width;
		rtFace.height = it->VehicleRect.height;
		printf("\r\nrtFace.x:%d,rtFace.y:%d, rtFace.width:%d,rtFace.height:%d\r\n",rtFace.x,rtFace.y,rtFace.width,rtFace.height);

		//cvDrawRect(m_imgSnap, cvPoint(rtFace.x, rtFace.y), cvPoint(rtFace.x+rtFace.width-1, rtFace.y+rtFace.height-1), CV_RGB(0,255,0), 2);

		if (rtFace.width <= 0 || rtFace.height <= 0)
		{
			it++;
			nIndex++;
			continue;
		}

		/*if (nIndex >= 1)
		{
			break;
		}
		*/
		IplImage* pImage = cvCreateImage(cvSize(rtFace.width, rtFace.height), 8, 3);

		cvSetImageROI(m_imgSnap,rtFace);
		cvResize(m_imgSnap,pImage);
		cvResetImageROI(m_imgSnap);
		
		CvRect rtDst;
		
		if(nIndex == 0)
		{
			rtDst.x = 0;
			rtDst.y = 0;
			rtDst.width = rtFace.width;
			rtDst.height = rtFace.height;
		}
		else
		{
			rtDst.x = m_imgSnap->width - rtFace.width-1;
			rtDst.y = 0;
			rtDst.width = rtFace.width;
			rtDst.height = rtFace.height;
		}

		if(m_nWordPos == 1)
		{
			rtDst.y += m_nExtentHeight;
		}

		cvSetImageROI(m_imgSnap,rtDst);
		cvResize(pImage,m_imgSnap);
		cvResetImageROI(m_imgSnap);

		cvReleaseImage(&pImage);

		it++;
		nIndex++;
	}
}

//安全带检测
void CRoadCarnumDetect::BeltDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context context,vector<FaceRt>& vecP,int& nBeltResult)
{
	 //车牌位置合理性判断
	if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= m_imgSnap->width || cardNum.iy >= m_imgSnap->height - m_nExtentHeight
		  || (cardNum.ix + cardNum.iwidth >= m_imgSnap->width) || (cardNum.iy + cardNum.iheight >= m_imgSnap->height - m_nExtentHeight))
	{

		LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
		return;
    }

	bool bIsSafetyBelt = true;
	int ncartype = 3;
	if((plate.uColor == CARNUM_YELLOW) && (cardNum.strCarNum[6] != '$'))
	{
		ncartype = 1;
	}
	if(context.position.y < (m_imgSnap->height * 1 / 3))
	{
		return;
	}	
	nBeltResult = m_safeType.mvSafetyBelt(m_imgSnap,&context,m_nDayNightbyLight,vecP,bIsSafetyBelt,ncartype,false,true,true);

	vector<FaceRt>::iterator it = vecP.begin();
	if(it != vecP.end())
	{
		plate.uBeltResult = it->label_type;
		plate.uPhoneResult = it->nPhone_type;
		plate.uSunVisorResult = it->nSunVisor_type;
	}
}
#endif

//车标检测
void CRoadCarnumDetect::CarLabelDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context,CvPoint& loopPtUp,CvPoint& loopPtDowon,CvRect& rtRoi)
{	
	 //车牌位置合理性判断
	if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= m_imgSnap->width || cardNum.iy >= m_imgSnap->height - m_nExtentHeight
		  || (cardNum.ix + cardNum.iwidth >= m_imgSnap->width) || (cardNum.iy + cardNum.iheight >= m_imgSnap->height - m_nExtentHeight))
	{

		LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
		return;
    }

	#ifndef ALGORITHM_YUV
		
		struct timeval tv1,tv2;
		if(g_nPrintfTime == 1)
		{
			gettimeofday(&tv1,NULL);
		}

       //cvSetImageROI(m_imgSnap,rtRealImage);
	   #ifdef GLOBALCARLABEL
          int uCarBrand;
	   #else
		  ushort uCarBrand;
	   #endif
	   int nTruckDetect = 0;
       //mvGetClassifyEHDTexture( m_imgSnap,context,rtRoi,uCarBrand,m_nDayNight);
       int nExtentHeight = 0;
       if(m_nWordPos == 1)
       {
            nExtentHeight = m_nExtentHeight;
       }
		//cvDrawRect(m_imgSnap, cvPoint(context.position.x, context.position.y), cvPoint(context.position.x+context.position.width-1, context.position.y+context.position.height-1), CV_RGB(255,0,0), 4);
		if(m_nDayNightbyLight == 0)
		{
						if((g_nHasHighLight == 1) && (cardNum.m_UseShutter) )//有爆闪灯晚上亮图未检测出车牌
						{
							if(loopPtUp.y == 0 && loopPtDowon.y == 0)
							{
								if(cardNum.m_CarnumPos.width <= 0 || cardNum.m_CarnumPos.height <= 0 || cardNum.m_CarnumPos.x <= 0 || cardNum.m_CarnumPos.y <= 0 || cardNum.m_CarnumPos.x >= m_imgSnap->width || cardNum.m_CarnumPos.y >= m_imgSnap->height - m_nExtentHeight
										  || (cardNum.m_CarnumPos.x + cardNum.m_CarnumPos.width >= m_imgSnap->width) || (cardNum.m_CarnumPos.y + cardNum.m_CarnumPos.height >= m_imgSnap->height - m_nExtentHeight))
								{
										  
											LogError("爆闪车牌位置不合理,%d,%d,%d,%d\n",cardNum.m_CarnumPos.x,cardNum.m_CarnumPos.y,cardNum.m_CarnumPos.width,cardNum.m_CarnumPos.height);
											return;
							     }

								if(cardNum.m_CarnumPos.width > 0)
								{
									context.position.x = cardNum.m_CarnumPos.x;
									context.position.y = cardNum.m_CarnumPos.y;
									context.position.width = cardNum.m_CarnumPos.width;
									context.position.height = cardNum.m_CarnumPos.height;
								}

								vector<LoopInfo>::iterator it_lb = m_LoopInfo.begin();
								vector<LoopInfo>::iterator it_le = m_LoopInfo.end();
								while(it_lb!=it_le)
								{
									if(it_lb->nRoadIndex == cardNum.RoadIndex || cardNum.RoadIndex <0)
									{
										loopPtUp.x = it_lb->line0.end.x;
										loopPtUp.y = it_lb->line0.end.y;
										loopPtDowon.x = it_lb->line1.end.x;
										loopPtDowon.y = it_lb->line1.end.y;
										break;
									}
									it_lb++;
								}
								
								#ifndef ALGORITHM_DL
								MvLisenceDetect LisenceDetect;
								LisenceDetect.mvDetectLisencePosition(m_imgLightSnap->imageData+(m_imgLightSnap->widthStep*nExtentHeight),m_imgLightSnap->width,m_imgLightSnap->height-m_nExtentHeight,loopPtUp,loopPtDowon,context);
								#endif
							}
							#ifndef GLOBALCARLABEL
							m_carLabel.mvGetClassifyEHDTexture( m_imgLightSnap->imageData+(m_imgLightSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNightbyLight,true );
							#else
							pthread_mutex_lock(&g_carLabelMutex);
							//g_carLabel.mvSetImageWidthandHeight(m_imgFrame->width,m_imgFrame->height);
							#ifdef ALGORITHM_DL
							object_label objLabel;
							nTruckDetect = g_carLabel.mvDLCardCarLabel(m_imgLightSnap,&context,&rtRoi,&objLabel,m_nDayNightbyLight);
							uCarBrand = objLabel.plabel;
							#else
							nTruckDetect = g_carLabel.mvGetClassifyEHDTexture( m_imgLightSnap->imageData+(m_imgLightSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNightbyLight,true );
				  		    #endif
							pthread_mutex_unlock(&g_carLabelMutex);
							#endif
						}
						else
						{
							#ifndef GLOBALCARLABEL
							m_carLabel.mvGetClassifyEHDTexture( m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNightbyLight,true );
							#else
							pthread_mutex_lock(&g_carLabelMutex);
							//g_carLabel.mvSetImageWidthandHeight(m_imgFrame->width,m_imgFrame->height);
							#ifdef ALGORITHM_DL
							object_label objLabel;
							nTruckDetect = g_carLabel.mvDLCardCarLabel(m_imgSnap,&context,&rtRoi,&objLabel,m_nDayNightbyLight);
							uCarBrand = objLabel.plabel;
							#else
							nTruckDetect = g_carLabel.mvGetClassifyEHDTexture( m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNightbyLight,true );
							#endif
							pthread_mutex_unlock(&g_carLabelMutex);
				  		    #endif
						}
		}
		else
		{				
						#ifndef GLOBALCARLABEL
						m_carLabel.mvGetClassifyEHDTexture( m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNightbyLight );
						#else
						pthread_mutex_lock(&g_carLabelMutex);
						//g_carLabel.mvSetImageWidthandHeight(m_imgFrame->width,m_imgFrame->height);
						#ifdef ALGORITHM_DL
						object_label objLabel;
						nTruckDetect = g_carLabel.mvDLCardCarLabel(m_imgSnap,&context,&rtRoi,&objLabel,m_nDayNightbyLight);
						uCarBrand = objLabel.plabel;
						#else
						nTruckDetect = g_carLabel.mvGetClassifyEHDTexture( m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNightbyLight );
						#endif
						pthread_mutex_unlock(&g_carLabelMutex);				  		
						#endif
						
	     }
         plate.uCarBrand = uCarBrand;
#ifdef GLOBALCARLABEL
		 CBrandSusection brandsub;
		#ifdef DETAIL_OLDBRAND
		plate.uCarBrand = brandsub.GetOldBrandFromDetail(uCarBrand);
		#else
		 brandsub.GetCarLabelAndChildSub(plate.uCarBrand,plate.uDetailCarBrand);
		if(plate.uCarBrand >= 200000)
		{
			plate.uCarBrand = 200000;
		}
		#endif
#endif
         //printf("after mvGetClassifyEHDTexture\n");
         //cvResetImageROI(m_imgSnap);

		if(g_nPrintfTime == 1)
		{
				gettimeofday(&tv2,NULL);
				FILE* fp = fopen("time.log","ab+");
				fprintf(fp,"mvGetClassifyEHDTexture==t1=%lld,t2=%lld,dt = %lld\n",(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
				fclose(fp);
		}
		#ifdef GLOBALCARLABEL
		if(m_nDetectKind&DETECT_TRUCK)
		{
			plate.uTypeDetail = GetTypeDetail(nTruckDetect);
		}
		#endif
#endif
}


//车牌二次检测
bool CRoadCarnumDetect::CarNumTwiceDetect(CarInfo& cardNum,carnum_context& vehicle_result)
{
	#ifndef ALGORITHM_YUV_CARNUM
					carnum_parm_t cnp;
					cnp.isday = m_nDayNight;
					mvcarnumdetect.set_carnum_parm(&cnp);
					
					if(!m_bSetCarnumHeight)
					{
						mvcarnumdetect.mv_SetCarnumHeight(m_imgSnap->width,m_imgSnap->height-m_nExtentHeight,cardNum.iheight);
						m_bSetCarnumHeight = true;
					}

				//	LogNormal("before find_carnum=x=%d,y=%d,w=%d,h=%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);

					CvRect rtCarnumROI;
					rtCarnumROI.width = 0;

					//扩充车牌区域
					rtCarnumROI.x = cardNum.ix-2*cardNum.iwidth;
					if(rtCarnumROI.x < 0)
					{
						rtCarnumROI.x = 0;
					}

					rtCarnumROI.y = cardNum.iy-3*cardNum.iwidth;
					if(rtCarnumROI.y < 0)
					{
						rtCarnumROI.y = 0;
					}

					rtCarnumROI.width = 5*cardNum.iwidth;
					if(rtCarnumROI.x + rtCarnumROI.width >= m_imgSnap->width)
					{
						rtCarnumROI.width = m_imgSnap->width - rtCarnumROI.x-1;
					}

					rtCarnumROI.height = cardNum.iheight+4*cardNum.iwidth;
					if(rtCarnumROI.y + rtCarnumROI.height >= m_imgSnap->height-m_nExtentHeight)
					{
						rtCarnumROI.height = m_imgSnap->height-m_nExtentHeight - rtCarnumROI.y-1;
					}

					if(g_nDetectMode == 2)
					{
						 /*//南昌版本
						 if(cardNum.m_CarWholeRec.width > 0)
						 {
							
							rtCarnumROI = cardNum.m_CarWholeRec;

							rtCarnumROI.y = cardNum.m_CarWholeRec.y + cardNum.m_CarWholeRec.height / 3;
							if(rtCarnumROI.y < 0)
							{
								rtCarnumROI.y = 0;
							}
							rtCarnumROI.height = cardNum.m_CarWholeRec.height * 2 / 3;
							
						 }
						 //贵阳比武版本
						rtCarnumROI.x = 0;
						rtCarnumROI.width = m_imgSnap->width; 

						rtCarnumROI.y = m_imgSnap->height * 1 / 5; 
						if(rtCarnumROI.y < 0) 
						{ 
							rtCarnumROI.y = 0; 
						} 
						rtCarnumROI.height = (m_imgSnap->height-m_nExtentHeight) * 4 / 5;
						*/
						//岳阳比武版本
						//if (m_IsMultiChannel)
						//{
							rtCarnumROI = cardNum.m_CarWholeRec;
						//}
						/*else
						{
							rtCarnumROI.x = 0;
							rtCarnumROI.width = m_imgSnap->width; 

							rtCarnumROI.y = (m_imgSnap->height - m_nExtentHeight) * (m_DetectAreaUp / 100); 
							if(rtCarnumROI.y < 0) 
							{ 
								rtCarnumROI.y = 0; 
							} 
							rtCarnumROI.height = (m_imgSnap->height - m_nExtentHeight) * (m_DetectAreaBelow / 100) - rtCarnumROI.y;
							
						}*/
					}
					
					//LogNormal("%s",cardNum.strCarNum);
					//cvDrawRect(m_imgSnap, cvPoint(rtCarnumROI.x, rtCarnumROI.y), cvPoint(rtCarnumROI.x+rtCarnumROI.width-1, rtCarnumROI.y+rtCarnumROI.height-1), CV_RGB(255,0,0), 2);

					if(rtCarnumROI.width > 0)
					{
						char tmp[32]= {0};
						//IplImage* tmpimg=NULL;
						
						#ifdef ALGORITHM_DL
						cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
						#endif

						road_context context;

						carnum_context vehicle_result1[CARNUMSIZE];
		
						//printf("before mvcarnumdetect.find_carnum\n");
						int nCarNumResult = mvcarnumdetect.find_carnum(NULL,m_imgSnap,tmp,NULL,rtCarnumROI,1,vehicle_result1,&context,NULL,m_LoopParmarer);
						//printf("after mvcarnumdetect.find_carnum\n");

						/*if(tmpimg!=NULL)
						{
							cvReleaseImage(&tmpimg);
							tmpimg = NULL;
						}*/
						#ifdef ALGORITHM_DL
						cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
						#endif


						if(nCarNumResult > 0)
						{
							printf("\n\nbefor find_carnum=CarNum=%s,x=%d,y=%d,w=%d,h=%d\n",cardNum.strCarNum,cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
							cardNum.ix      = vehicle_result1[0].position.x;
							cardNum.iy      = vehicle_result1[0].position.y;
							cardNum.iwidth  = vehicle_result1[0].position.width;
							cardNum.iheight = vehicle_result1[0].position.height;

							printf("\n\nafter find_carnum=CarNumResult=%d===%s=x=%d,y=%d,w=%d,h=%d\n",nCarNumResult,cardNum.strCarNum,cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
//#endif
							return true;
						}
						else
						{
							return false;
						}
#if 0
						else
						{
							//LogError("CarNumTwiceDetect fail\n");
                            #ifdef ALGORITHM_DL
							cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
                            #endif
							
							//车牌区域
							CvRect rtCarnumROI2;
							rtCarnumROI2.x = cardNum.ix;
							rtCarnumROI2.y = cardNum.iy;
							rtCarnumROI2.width = cardNum.iwidth;
							rtCarnumROI2.height = cardNum.iheight;	

							int nResult = mvcarnumdetect.find_carnum( m_imgSnap, rtCarnumROI2, &vehicle_result );

							//cvDrawRect(m_imgSnap, cvPoint(cardNum.ix - 20, cardNum.iy - 20), cvPoint(cardNum.ix+cardNum.iwidth + 10, cardNum.iy+cardNum.iheight + 10), CV_RGB(255,0,0), 2);
							#ifdef ALGORITHM_DL
							cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
							#endif
							if (nResult > 0)
							{
								//
								int similarNum = GetSimilarNumOfPlate(cardNum.strCarNum,vehicle_result.carnum,7);
								if (similarNum  > 4 )
								{
									/*#ifdef ALGORITHM_DL
									cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
									#endif
									char buffer[128]={0};
									sprintf(buffer, "./testOK/%s-%s-%d-%d.jpg", cardNum.strCarNum,vehicle_result.carnum,similarNum,picNumber++);
									cvSaveImage(buffer,m_imgSnap);
									#ifdef ALGORITHM_DL
									cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
									#endif*/							
									cardNum.ix      = vehicle_result.position.x;
									cardNum.iy      = vehicle_result.position.y;
									cardNum.iwidth  = vehicle_result.position.width;
									cardNum.iheight = vehicle_result.position.height;
									return true;
								}
								else
								{
									/*#ifdef ALGORITHM_DL
									cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
									#endif
									char buffer[128]={0};
									sprintf(buffer, "./testError/%s-%s-%d-%d.jpg", cardNum.strCarNum,vehicle_result.carnum,similarNum,picNumber++);
									cvSaveImage(buffer,m_imgSnap);
									#ifdef ALGORITHM_DL
									cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
									#endif	*/						
									return false;
								}
								
							}
							else
							{
                                /*#ifdef ALGORITHM_DL
								cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
                                #endif
								char buffer[128]={0};
								sprintf(buffer, "./test/%s-%d.jpg", cardNum.strCarNum,picNumber++);
								cvSaveImage(buffer,m_imgSnap);*/

								return false;
							}
							
						}
#endif
					}
	#endif
}
/*
功能://比较车牌字符相似度个数
返回值: 
*/
int CRoadCarnumDetect::GetSimilarNumOfPlate( char * strfirst,char *strSecond,int nSize)
{
	int iConfidenceCount = 0;

	for (int i = 0; i < nSize; ++i)
	{
		if (strfirst[i] == strSecond[i])
		{
			iConfidenceCount ++;
		}	

	}
	return iConfidenceCount;
}
//遮阳板检测
void CRoadCarnumDetect::CarShieldDetect(CarInfo& cardNum,RECORD_PLATE& plate,CvRect rtRealImage,CvRect rtCarnum)
{
	#ifdef DETECTSHIELD//遮阳板检测接口
	//if(m_nDayNight == 0)//晚上才做
	{
				vector<CvRect> CarShield;
				cvSetImageROI(m_imgSnap,rtRealImage);
				/*if( VEDIO_PICLIB == m_nVedioFormat)
				{
					m_ShieldGet.mvDetcCarShieldByPlat(m_imgSnap,1,rtCarnum,CarShield);
				}
				else*/
				{
					//m_ShieldGet.mvDetcCarShield(m_imgSnap,1,rtCarnum.width,rtCarnum.height,CarShield);

					bool bYelPlate = false;
					if(plate.uColor == CARNUM_YELLOW)
					{
						bYelPlate = true;
					}
					m_ShieldGet.mvDetcCarShieldByPlat(m_imgSnap,1,rtCarnum,bYelPlate,CarShield);
				}
				cvResetImageROI(m_imgSnap);
				//LogNormal("CarShield.size()=%d\n",CarShield.size());
				if(CarShield.size() > 0)
				{
					plate.uViolationType = DETECT_RESULT_EVENT_SHIELD;
				}
	}
	#endif
}

//获取事件车牌
void CRoadCarnumDetect::GetEventCarnum(CarInfo& cardNum)
{
	#ifndef ALGORITHM_YUV_CARNUM
	MvCarNumInfo info;
	string strPicData;
	TimePlate tp;

	memset(&info, 0, sizeof(MvCarNumInfo));
	tp.uSec = (cardNum.ts/1000)/1000;
	tp.uMiSec = (cardNum.ts/1000)%1000;
	strPicData.append(m_imgSnap->imageData, m_imgSnap->imageSize);
	memcpy(info.cCarNum, cardNum.strCarNum, 7);//车牌号
	memcpy(tp.chPlate, cardNum.strCarNum, 7);//车牌号
	memcpy(info.wjcarnum, cardNum.wj,2);
	info.color = cardNum.color;
	info.vehicle_type = cardNum.vehicle_type;
	info.carnumrow = cardNum.carnumrow;//车牌结构
	info.nIndex = cardNum.uSeq;
	info.ts = cardNum.ts;

	info.rtCarbrand.x = cardNum.ix;
	info.rtCarbrand.y = cardNum.iy;
	info.rtCarbrand.width = cardNum.iwidth;
	info.rtCarbrand.height = cardNum.iheight;

	tp.nLeft = info.rtCarbrand.x;
	tp.nTop = info.rtCarbrand.y;
	tp.nWidth = info.rtCarbrand.width;
	tp.nHeight = info.rtCarbrand.height;

	info.bIsFilter = true;
	m_pDetect->AddCarNumInfo(info);
	m_pDetect->AddPic(tp, strPicData);
	#endif
}

//目标特征识别
void CRoadCarnumDetect::ObjectCharacterDetect(RECORD_PLATE& plate,CvRect rtCarPos)
{		
		CvRect plateRect;
		plateRect.x = plate.uPosLeft;
		plateRect.y = plate.uPosTop;
		plateRect.width = plate.uPosRight-plate.uPosLeft;
		plateRect.height = plate.uPosBottom-plate.uPosTop;

		int vehicle = 1;
		if(plate.chText[0] != '*')
		{
			vehicle = 0;
		}

		vector<CvRect> vFacePos;

		
		
		#ifdef OBJECTHEADDETECT
		if(m_nDetectKind&DETECT_CHARACTER)
		{
			Rect rect;
			rect.x = plateRect.x;
			rect.y = plateRect.y;
			rect.width = plateRect.width;
			rect.height = plateRect.height;
			
			cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
			vector<Rect> vFaceRect = m_HeadDetection.detectMultiScale(m_imgSnap,rect,(FaceDetection::Vehicle)vehicle);
			cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
			vector<Rect>::iterator it = vFaceRect.begin();
			while(it != vFaceRect.end())
			{
				CvRect FaceRect;
				FaceRect.x = it->x;
				FaceRect.y = it->y;
				FaceRect.width = it->width;
				FaceRect.height = it->height;
				vFacePos.push_back(FaceRect);
				it++;
			}
		}
		#endif

		#ifdef OBJECTFACEDETECT
		if(m_nDetectKind&DETECT_FACE)
		{
			if(vehicle == 0)//有牌车
			{
				int nType = 0;
				if(plate.uColor == CARNUM_BLUE)
				{
					nType = 0;
				}
				else if(plate.uColor == CARNUM_YELLOW)
				{
					nType = 1;
				}
				cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
				m_FaceDetection.mv_DetectFaces(m_imgSnap,plateRect,nType,vFacePos);
				cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
			}
		}
		#endif

		plate.uPicSize = SaveFaceImage(m_imgSnap,plate,vFacePos);

		#ifdef OBJECTFACEDETECT
		/*if(m_nDetectKind&DETECT_FACE)
		{
			if(vFacePos.size() > 0)
			{
				vector<IplImage*> vImages;
				cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
				vImages.push_back(m_imgSnap);

				mv_stFaceFeature faceFeatures;
				if(m_FaceTrackSearch.mv_ExtractFeatureFromTrackImages(vImages,vFacePos,faceFeatures,vFacePos,false))
				{
					OutPutFaceFeature(faceFeatures,plate);
				}
				cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
			}
		}*/
		#endif


		#ifdef OBJECTFEATURESEARCH
		if(m_nDetectKind&DETECT_CHARACTER)
		{
			cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
			EXTRACTPARA extractPara;
			extractPara.oriImg = m_imgSnap; 
			extractPara.pMaskImg = NULL; 
			extractPara.uSeq = m_uFeatureSeq;
			OBJINFO objInfo; 
			
			vector<CvRect> smallRtVec;
			CvRect smallRt;
			smallRt.x = 0;
			smallRt.y = 0;
			smallRt.width = 0;
			smallRt.height = 0;
			_DETECT_OBJ_TYPE objType = F_PERSON;
			if(plate.chText[0] != '*')//有牌车
			{
				objType = F_VEHICLE;
				smallRt.x = plate.uPosLeft;
				smallRt.y = plate.uPosTop;
				smallRt.width = plate.uPosRight-plate.uPosLeft;
				smallRt.height = plate.uPosBottom-plate.uPosTop;
				smallRtVec.push_back(smallRt);
			}
			/*else if(plate.chText[1] == '-')//无牌车
			{
				objType = F_VEHICLE;
				smallRtVec.push_back(smallRt);
			}*/
			else//行人
			{
				vector<CvRect>::iterator it = vFacePos.begin();
				while(it != vFacePos.end())
				{
					smallRtVec.push_back(*it);
					it++;
				}
			}

			objInfo.nObjType = objType; //提取特征目标类型

			CvRect ObjRt;
			ObjRt = rtCarPos;
			//ObjRt = GetCarPos(plate,3);
			objInfo.ObjRt = ObjRt; //提取特征的目标区域
			printf("ObjRt.x=%d,ObjRt.y=%d,ObjRt.width=%d,ObjRt.height=%d\n",ObjRt.x,ObjRt.y,ObjRt.width,ObjRt.height);

			//objInfo.smallRt = smallRt; //行人目标的头部或者车辆目标的车牌位置
			objInfo.smallRtVec = smallRtVec;
			printf("smallRt.x=%d,smallRt.y=%d,smallRt.width=%d,smallRt.height=%d\n",smallRt.x,smallRt.y,smallRt.width,smallRt.height);

			extractPara.pObjInfo = &objInfo; //参数值传入
			feature *mfeature = m_pFeatureSearchDetect->MvKK_Extract(extractPara);
			cvConvertImage(m_imgSnap,m_imgSnap,CV_CVTIMG_SWAP_RB);
			if(mfeature != NULL)
			{
				OutPutFeatureResult(mfeature,plate);
			}
			m_uFeatureSeq++;
		}
		#endif

		
}

#ifdef OBJECTFEATURESEARCH
//生成特征数据
void CRoadCarnumDetect::OutPutFeatureResult(feature* mfeature,RECORD_PLATE plate)
{		
		char buf[256] = {0};
        XMLNode FeatureNode,TimeNode,PicNode,BlobNode,SurfNode,EdgeNode,ModeNode,SeqNode,RunNode,ObjectTypeNode;
        XMLCSTR strText;
		XMLNode FeatureInfoNode;//特征数据
		string strXmlResult;//特征数据

		//////////////////////////////
			

		   FeatureInfoNode = XMLNode::createXMLTopNode("FeatureInfo");


		   ModeNode = FeatureInfoNode.addChild("WorkMode");
		   sprintf(buf,"%d",0);
		   ModeNode.addText(buf);

		    RunNode = FeatureInfoNode.addChild("RunMode");
		   sprintf(buf,"%d",1);
		   RunNode.addText(buf);

		   ObjectTypeNode = FeatureInfoNode.addChild("ObjType");
		   int nObjType = 2;
			if(plate.chText[0] != '*')
			{
				nObjType = 1;
			}
			sprintf(buf,"%d",nObjType);
		   ObjectTypeNode.addText(buf);
		   

			string strPic;
			EncodeBase64(strPic,(unsigned char*)(m_pCurJpgImage),plate.uPicSize);
			PicNode = FeatureInfoNode.addChild("Pic");
			if(strPic.size() > 0)
				PicNode.addText(strPic.c_str());

			FEATURE_DETECT_HEADER header;
			header.uCameraID = m_nCameraID;
			header.uCmdID = FEATURE_INFO;

			strXmlResult.append((char*)&header,sizeof(FEATURE_DETECT_HEADER));
	

	    FeatureNode = FeatureInfoNode.addChild("Feature");

        TimeNode = FeatureNode.addChild("Time");
        string strTime = GetTime(plate.uTime);
        sprintf(buf,"%s:%03d",strTime.c_str(),plate.uMiTime);
        TimeNode.addText(buf);

		SeqNode = FeatureNode.addChild("Seq");
		sprintf(buf,"%u",m_uFeatureSeq);
		SeqNode.addText(buf);
		printf("CRoadCarnumDetect::OutPutFeatureResult----------------m_uFeatureSeq=%llu,nObjType=%d\n",m_uFeatureSeq,nObjType);

		
		int nBlobSize = mfeature->nBlobSize;
		if(nBlobSize> 0)
		{
				unsigned char* pBlobFeature = (unsigned char*)(mfeature->fBlobFeature);
				string strBlobFeature;
				EncodeBase64(strBlobFeature,pBlobFeature,sizeof(blobFeatureType)*nBlobSize);
				BlobNode = FeatureNode.addChild("BlobFeature");
				if(strBlobFeature.size() > 0)
				BlobNode.addText(strBlobFeature.c_str());
		}
		
		
		int nSurfSize = mfeature->nSurfSize;
		if(nSurfSize> 0)
		{
				unsigned char* pSurfFeature = (unsigned char*)(mfeature->fSurfFeature);
				
				string strSurfFeature;
				EncodeBase64(strSurfFeature,pSurfFeature,sizeof(featureType)*nSurfSize);
				SurfNode = FeatureNode.addChild("SurfFeature");
				if(strSurfFeature.size() > 0)
					SurfNode.addText(strSurfFeature.c_str());
		}
		
		
		int nEdgeSize = mfeature->nEdgeSize;
		if(nEdgeSize > 0)
		{
				unsigned char* pEdgeFeature = (unsigned char*)(mfeature->fEdgeFeature);

				string strEdgeFeature;
				EncodeBase64(strEdgeFeature,pEdgeFeature,sizeof(int)*nEdgeSize);
				EdgeNode = FeatureNode.addChild("EdgeFeature");
				if(strEdgeFeature.size() > 0)
				EdgeNode.addText(strEdgeFeature.c_str());
		}
		
		printf("nBlobSize=%d,nSurfSize=%d,nEdgeSize=%d\n",nBlobSize,nSurfSize,nEdgeSize);


			int nSize;
			XMLSTR strData = FeatureInfoNode.createXMLString(1, &nSize);
			if(strData)
			{
				strXmlResult.append(strData, sizeof(XMLCHAR)*nSize);
				freeXMLString(strData);
			}

			g_MatchCommunication.AddResult(strXmlResult);
}
#endif


#ifdef OBJECTFACEDETECT
//输出人脸特征结果
void CRoadCarnumDetect::OutPutFaceFeature(mv_stFaceFeature& faceFeatures,RECORD_PLATE plate)
{
		char buf[256] = {0};
        XMLNode FeatureNode,TimeNode,PicNode,PosNode,ModeNode,TempNode;
        XMLCSTR strText;

		//////////////////////////////
		XMLNode FeatureInfoNode;//特征数据
		FeatureInfoNode = XMLNode::createXMLTopNode("FaceFeatureInfo");

		ModeNode = FeatureInfoNode.addChild("WorkMode");
		sprintf(buf,"%d",0);
		ModeNode.addText(buf);

		TimeNode = FeatureInfoNode.addChild("Time");
		string strTime = GetTime(plate.uTime);
		sprintf(buf,"%s:%03d",strTime.c_str(),plate.uMiTime);
		TimeNode.addText(buf);
		printf("Time=%s\n",buf);

		string strPic;
		EncodeBase64(strPic,(unsigned char*)(m_pCurJpgImage),plate.uPicSize);
		PicNode = FeatureInfoNode.addChild("Pic");
		if(strPic.size() > 0)
			PicNode.addText(strPic.c_str());

	    FeatureNode = FeatureInfoNode.addChild("FaceFeature");
		string strFeature;
		if(faceFeatures.m_pFaceFeature != NULL)
		{
			EncodeBase64(strFeature,(unsigned char*)faceFeatures.m_pFaceFeature,faceFeatures.m_nSize);
			delete [] faceFeatures.m_pFaceFeature;
			faceFeatures.m_pFaceFeature = NULL;
		}

		if(strFeature.size() > 0)
		FeatureNode.addText(strFeature.c_str());

		FEATURE_DETECT_HEADER header;
		header.uCameraID = m_nCameraID;
		header.uCmdID = FACE_FEATURE_INFO;
			
		string strXmlResult;//特征数据
		strXmlResult.append((char*)&header,sizeof(FEATURE_DETECT_HEADER));

		int nSize;
		XMLSTR strData = FeatureInfoNode.createXMLString(1, &nSize);
		if(strData)
		{
			strXmlResult.append(strData, sizeof(XMLCHAR)*nSize);
			freeXMLString(strData);
		}
		printf("strXmlResult.size()=%d\n",strXmlResult.size());

		g_MatchCommunication.AddResult(strXmlResult);
}
#endif


//叠加存储图像
void CRoadCarnumDetect::SaveComposeImage(RECORD_PLATE& plate,CvRect rtCarPos,PLATEPOSITION* TimeStamp)
{			
	//LogNormal("SaveComposeImage seq:%d ", plate.uSeq);
	       //截取小图区域
          CvRect rtPos;
          if(m_nSmallPic == 1)
          {
				 if(g_nDetectMode == 2)
				 {
					 //LogNormal("rtCarPos:%d,%d,%d,%d ", \
					//	 rtCarPos.x, rtCarPos.y, rtCarPos.width, rtCarPos.height);
					 if(rtCarPos.width == 0)
					 {
						 rtPos = GetCarPos(plate);
					 }
					 else
					 {
						rtPos = rtCarPos;
					 }
				 }
				 else
				 {
					 rtPos = GetCarPos(plate);
				 }
          }
			std::string strPicPath;
			strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));
			//LogNormal("comPic path =%s\n",strPicPath);dgh
			//LogNormal("g_nFtpServer:%d m_bConnect:%d ", g_nFtpServer, m_bConnect);

			if (g_nServerType == 13)
			{
							if (g_DistanceHostInfo.bDistanceCalculate == 1)
							{
											plate.uChannelID = m_nChannelId;

											IplImage* nRealImage = cvCreateImage(cvSize(m_imgSnap->width,m_imgSnap->height-m_nExtentHeight),8,3);
											CvRect rtRealImage;
											rtRealImage.x = 0;
											rtRealImage.y = 0;
											rtRealImage.width = m_imgSnap->width;
											rtRealImage.height = m_imgSnap->height-m_nExtentHeight;
											cvSetImageROI(m_imgSnap,rtRealImage);
											cvCopy(m_imgSnap,nRealImage);
											cvResetImageROI(m_imgSnap);

											plate.uPicWidth =  m_imgSnap->width;
											plate.uPicHeight =  m_imgSnap->height;

											plate.uPicSize = SaveImage(nRealImage,strPicPath);
											if (nRealImage != NULL)
											{
												cvReleaseImage(&nRealImage);
												nRealImage = NULL;
											}
											//车牌号码
											//memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
											string nStrNum = plate.chText;
											int nPos = -1;
											nPos = nStrNum.find("*");
											if (nPos <= -1)
											{
												string strPlate("");
												strPlate.append((char*)&plate,sizeof(RECORD_PLATE));

												if (m_bConnect)
												{
													g_mvsCommunication.SetFlag(m_bConnect);
												}

												//LogNormal("Mvs uSeq:%lld, %s", plate.uSeq, plate.chText);
												g_mvsCommunication.OnResult(strPlate);	
											}	

											plate.uPicWidth = m_imgDestSnap->width;
											plate.uPicHeight = m_imgDestSnap->height;
							}
			}
			else if(g_nServerType == 23 || g_nServerType == 26)
			{
				if (g_DistanceHostInfo.bDistanceCalculate == 1)
				{
					plate.uChannelID = m_nChannelId;

					IplImage* nRealImage = cvCreateImage(cvSize(m_imgSnap->width,m_imgSnap->height-m_nExtentHeight),8,3);
					CvRect rtRealImage;
					rtRealImage.x = 0;
					rtRealImage.y = 0;
					rtRealImage.width = m_imgSnap->width;
					rtRealImage.height = m_imgSnap->height-m_nExtentHeight;
					cvSetImageROI(m_imgSnap,rtRealImage);
					cvCopy(m_imgSnap,nRealImage);
					cvResetImageROI(m_imgSnap);

					plate.uPicWidth =  m_imgSnap->width;
					plate.uPicHeight =  m_imgSnap->height;

					plate.uPicSize = SaveImage(nRealImage,strPicPath);
					if (nRealImage != NULL)
					{
						cvReleaseImage(&nRealImage);
						nRealImage = NULL;
					}
					//车牌号码
					//memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
					string nStrNum = plate.chText;

					plate.uOverSpeed = m_vtsGlobalPara.nSpeedVal;
					//LogNormal("22 plate.uOverSpeed:%d ", plate.uOverSpeed);

					int nPos = -1;
					nPos = nStrNum.find("*");
					if (nPos <= -1)
					{
						string strPlate("");
						strPlate.append((char*)&plate,sizeof(RECORD_PLATE));

						if (m_bConnect)
						{
							g_mvsCommunication.SetFlag(m_bConnect);
						}

						g_mvsCommunication.OnResult(strPlate);	
					}	

					plate.uPicWidth = m_imgDestSnap->width;
					plate.uPicHeight = m_imgDestSnap->height;
				}
			}
			else{}

			//是否需要存储小图
            if(m_nSmallPic == 1)
            {
				//LogNormal("11 rtPos:%d,%d,%d,%d", rtPos.x, rtPos.y, rtPos.width, rtPos.height);

                                CvRect rect;
                                rect.x = 0;
								if(m_nWordPos == 1)
								{
									rect.y = 0;
								}
								else
								{
									rect.y = m_imgDestSnap->height - m_nExtentHeight;
								}
                                rect.width = m_imgDestSnap->width;
                                rect.height = (m_imgDestSnap->height - m_imgSnap->height+m_nExtentHeight);

								if(rect.height > 0)
								{
									cvSetImageROI(m_imgDestSnap,rect);
									cvSet(m_imgDestSnap, cvScalar( 0,0, 0 ));
									cvResetImageROI(m_imgDestSnap);
								}

								PLATEPOSITION tempTimeStamp[2];
								IplImage* pImgSnap1 = NULL;
								IplImage* pImgSnap2 = NULL;
								
								int nOrder = 0;
								if(m_nSaveImageCount == 2)
								{
									if(TimeStamp[0].ts <= TimeStamp[1].ts)
									{
										pImgSnap1 = m_imgSnap;
										pImgSnap2 = m_imgPreSnap;
										tempTimeStamp[0] = TimeStamp[0];
										tempTimeStamp[1] = TimeStamp[1];
									}
									else
									{
										nOrder = 1;
										pImgSnap1 = m_imgPreSnap;
										pImgSnap2 = m_imgSnap;
										tempTimeStamp[0] = TimeStamp[1];
										tempTimeStamp[1] = TimeStamp[0];
									}
								}

                                for(int i = 0;i<m_nSaveImageCount;i++)
                                {
                                    rect.x = m_imgSnap->width*i;
                                    if(m_nWordPos == 1)
                                    rect.y = (m_imgDestSnap->height - m_imgSnap->height);
                                    else
                                    rect.y = 0;
                                    rect.width = m_imgSnap->width;
                                    rect.height = m_imgSnap->height;

                                    cvSetImageROI(m_imgDestSnap,rect);

									if(m_nSaveImageCount == 2)
									{
										if(i == 0)
										cvCopy(pImgSnap1,m_imgDestSnap);
										else
										cvCopy(pImgSnap2,m_imgDestSnap);
									}
									else
									{
										cvCopy(m_imgSnap,m_imgDestSnap);
									}

                                    cvResetImageROI(m_imgDestSnap);
                                }

                                rect.x = m_nSaveImageCount*m_imgSnap->width;
                                if(m_nWordPos == 1)
                                rect.y = (m_imgDestSnap->height - m_imgSnap->height+m_nExtentHeight);
                                else
                                rect.y = 0;
                                rect.width = m_imgDestSnap->width - rect.x;
                                rect.height = m_imgSnap->height - m_nExtentHeight;

                                if(rect.width > 0)
                                {
                                    cvSetImageROI(m_imgDestSnap,rect);
                                    if( (rtPos.width > 0) && (rtPos.height > 0))
                                    {
										//LogNormal("CardNum rtPos:%d,%d,%d,%d", \
										//	rtPos.x, rtPos.y, rtPos.width, rtPos.height);
                                        cvSetImageROI(m_imgSnap,rtPos);
                                        cvResize(m_imgSnap,m_imgDestSnap);
                                        cvResetImageROI(m_imgSnap);
                                    }
                                    cvResetImageROI(m_imgDestSnap);
                                }

#ifdef DRAW_RECT_DEBUG
								CvRect rtCar;
								rtCar.x = plate.uPosLeft - 2;
								rtCar.y = plate.uPosTop - 2;
								rtCar.width = plate.uPosRight - plate.uPosLeft + 2;
								rtCar.height = plate.uPosBottom - plate.uPosTop + 2;
								if(1 == m_nWordPos) //黑边在上方
								{
									rtCar.x -= m_nExtentHeight;
								}
								//LogNormal("1rtCar: x:%d,y:%d,w:%d,h:%d ", rtCar.x, rtCar.y, rtCar.width, rtCar.height);
								myRectangle(m_imgDestSnap, rtCar, CV_RGB(0,0,255), 2);
#endif

                               // PutTextOnComposeImage(m_imgDestSnap,plate,nOrder);
								 PutTextOnImage(m_imgDestSnap,plate,0,TimeStamp);
                                plate.uPicSize = SaveImage(m_imgDestSnap,strPicPath,2);
         }
         else
         {
                 if(m_nSaveImageCount == 2)
                 {					
									PLATEPOSITION tempTimeStamp[2];
									IplImage* pImgSnap1 = NULL;
									IplImage* pImgSnap2 = NULL;

									if(TimeStamp[0].ts <= TimeStamp[1].ts)
									{
										pImgSnap1 = m_imgSnap;
										pImgSnap2 = m_imgPreSnap;
										tempTimeStamp[0] = TimeStamp[0];
										tempTimeStamp[1] = TimeStamp[1];
									}
									else
									{
										pImgSnap1 = m_imgPreSnap;
										pImgSnap2 = m_imgSnap;
										tempTimeStamp[0] = TimeStamp[1];
										tempTimeStamp[1] = TimeStamp[0];
									}

                                    for(int nIndex = 0; nIndex <m_nSaveImageCount ; nIndex++)
                                    {
                                        CvRect rect;

										if(g_nPicMode == 2)//卡口图片上下组合（武汉特殊格式）
										{
											rect.x = 0;
											rect.y = m_imgSnap->height*nIndex;
											rect.width = m_imgSnap->width;
											rect.height = m_imgSnap->height;
										}
										else
										{
											rect.x = m_imgSnap->width*nIndex;
											rect.y = 0;
											rect.width = m_imgSnap->width;
											rect.height = m_imgSnap->height;
										}

                                        cvSetImageROI(m_imgDestSnap,rect);

										if(nIndex == 0)
										cvCopy(pImgSnap1,m_imgDestSnap);
										else
										cvCopy(pImgSnap2,m_imgDestSnap);
										
                                        cvResetImageROI(m_imgDestSnap);

										//LogNormal("14 PutTextOnImage seq:%d ", plate.uSeq);
										
                                        PutTextOnImage(m_imgDestSnap,plate,nIndex,tempTimeStamp);
                                    }
                                    plate.uPicSize = SaveImage(m_imgDestSnap,strPicPath,2);

                  }
				  else
				  {
									
#ifdef DRAW_RECT_DEBUG
					  CvRect rtCar;
					  rtCar.x = plate.uPosLeft - 2;
					  rtCar.y = plate.uPosTop - 2;
					  rtCar.width = plate.uPosRight - plate.uPosLeft + 2;
					  rtCar.height = plate.uPosBottom - plate.uPosTop + 2;
					  if(1 == m_nWordPos) //黑边在上方
					  {
						  rtCar.x -= m_nExtentHeight;
					  }
					  //LogNormal("2rtCar: x:%d,y:%d,w:%d,h:%d ", rtCar.x, rtCar.y, rtCar.width, rtCar.height);
					  myRectangle(m_imgSnap, rtCar, CV_RGB(0,0,255), 2);
#endif

									//在此处缩放
									#ifdef DSP_500W_TEST
										ReSizePic(m_imgSnap,m_imgDestSnap2);
										//LogNormal("15 PutTextOnImage seq:%d ", plate.uSeq);
										PutTextOnImage(m_imgDestSnap2,plate,0,TimeStamp);
										plate.uPicSize = SaveImage(m_imgDestSnap2,strPicPath,0);
									#else

										//LogNormal("16 PutTextOnImage seq:%d ", plate.uSeq);
										PutTextOnImage(m_imgSnap,plate,0,TimeStamp);
										//卡口特征提取人脸检测
										/*if((m_nDetectKind&DETECT_FACE) || (m_nDetectKind&DETECT_CHARACTER))
										{
											ObjectCharacterDetect(plate,rtCarPos);
										}
										else*/
										{
											plate.uPicSize = SaveImage(m_imgSnap,strPicPath,0);
										}
									#endif

									if(g_nPicSaveMode != 0)
									{
										if(plate.uViolationType == DETECT_RESULT_BLACK_PLATE)
										{
											string strViolationPicPath;
											GetPlatePicPath(plate,strViolationPicPath);
											SaveExistImage(strViolationPicPath,m_pCurJpgImage,plate.uPicSize);
										}
									}
					}
          }
}

void CRoadCarnumDetect::SaveVtsImage(
	RECORD_PLATE& plate,
	int nPicCount,
	PLATEPOSITION* TimeStamp,
	PLATEPOSITION& SignalTimeStamp,
	int64_t redLightStartTime,
	string& strPicPath2,
	string& strPicPath3)
{					
	//LogNormal("SaveVtsImage 111\n");
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));
	//大图宽高
	plate.uPicWidth = m_imgComposeSnap->width;
	plate.uPicHeight = m_imgComposeSnap->height;

					
	if((g_nServerType == 3) && (g_nVtsPicMode == 3))//旅行时间违章图片格式
	{
		plate.uTime2 = TimeStamp[1].uTimestamp;
		plate.uMiTime2 = (TimeStamp[1].ts/1000)%1000;
		//LogNormal("1=%lld,2=%lld,3=%lld\n",TimeStamp[0].ts/1000,TimeStamp[1].ts/1000,TimeStamp[2].ts/1000);
		PutTextOnComposeImage(m_imgComposeSnap,plate);
	}
	else
	{
		//在此处缩放
		#ifdef DSP_500W_TEST
		ReSizePic(m_imgComposeSnap,m_imgComposeSnap2);
		#endif
		for(int nIndex = 0; nIndex <nPicCount ; nIndex++)
		{
			#ifdef DSP_500W_TEST
			PutVtsTextOnImage(m_imgComposeSnap2,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime, plate.uViolationType);
			#else
			PutVtsTextOnImage(m_imgComposeSnap,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime, plate.uViolationType);
			#endif
		}

		//叠加额外信息
		if( (g_nVtsPicMode > 0)&& (g_nVtsPicMode < 3) && (g_PicFormatInfo.nSpaceRegion == 0))
		{
			if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
				PutVtsTextOnImage(m_imgComposeSnap,plate,nPicCount,TimeStamp,&SignalTimeStamp,redLightStartTime,plate.uViolationType);
			else 
				PutVtsTextOnImage(m_imgComposeSnap,plate);
		}
	}
	              
	if(7 == g_nServerType)
	{
		for(int i =0;i<nPicCount;i++)
		{
			//天津电警3张图分开存储
			IplImage* pImage = cvCreateImage(cvSize(m_imgComposeSnap->width,m_imgComposeSnap->height/3),8,3);
			if(i == 0)
			{
				CvRect rect;
				rect.x = 0;
				rect.y = 0;
				rect.width = pImage->width;
				rect.height = pImage->height;
				cvSetImageROI(m_imgComposeSnap,rect);
				cvCopy(m_imgComposeSnap,pImage);
				cvResetImageROI(m_imgComposeSnap);

				plate.uPicSize = SaveImage(pImage,strPicPath);
			}
			else if(i == 1)
			{
				CvRect rect;
				rect.x = 0;
				rect.y = pImage->height;
				rect.width = pImage->width;
				rect.height = pImage->height;
				cvSetImageROI(m_imgComposeSnap,rect);
				cvCopy(m_imgComposeSnap,pImage);
				cvResetImageROI(m_imgComposeSnap);

				SaveImage(pImage,strPicPath2);
			}
			else if(i == 2)
			{
				CvRect rect;
				rect.x = 0;
				rect.y = 2*pImage->height;
				rect.width = pImage->width;
				rect.height = pImage->height;
				cvSetImageROI(m_imgComposeSnap,rect);
				cvCopy(m_imgComposeSnap,pImage);
				cvResetImageROI(m_imgComposeSnap);

				SaveImage(pImage,strPicPath3);
			}

			cvReleaseImage(&pImage);

			//LogTrace("VtsPath.txt", "vts:%d path1:%s\n", plate.uViolationType, strPicPath.c_str());
			//LogTrace("VtsPath.txt", "path2:%s\n", strPicPath2.c_str());
			//LogTrace("VtsPath.txt", "path3:%s\n", strPicPath3.c_str());
		}
	}
	else if(g_nServerType == 23 || g_nServerType == 26)//济南项目
	{
		if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)//超速只要两张图
		{
				//违章图片分开存储
				IplImage* pImage = cvCreateImage(cvSize(m_imgComposeSnap->width/2,m_imgComposeSnap->height),8,3);

				CvRect rect;
				rect.x = 0;
				rect.y = 0;
				rect.width = pImage->width;
				rect.height = pImage->height;
				cvSetImageROI(m_imgComposeSnap,rect);
				cvCopy(m_imgComposeSnap,pImage);
				cvResetImageROI(m_imgComposeSnap);

				plate.uPicSize = SaveImage(pImage,strPicPath);
				cvReleaseImage(&pImage);
		}
		else
		{
			    plate.uPicSize = SaveImage(m_imgComposeSnap,strPicPath,2);
		}
	}
	else if(g_nServerType == 13 && g_nFtpServer == 1)//上海交警项目
	{
		//违章图片分开存储
		IplImage* pImage = cvCreateImageHeader(cvSize(m_imgComposeSnap->width,m_imgComposeSnap->height/2),8,3);
					
		for(int i =0;i<(nPicCount>1?2:1);i++)
		{
			cvSetData(pImage,m_imgComposeSnap->imageData+i*m_imgComposeSnap->imageSize/2,pImage->widthStep);

			if(i == 0)
			{
				{
					plate.uPicSize = SaveImage(pImage,strPicPath);
				}
			}
			else if(i == 1)
			{
				SaveImage(pImage,strPicPath2);
			}
		}

		cvReleaseImageHeader(&pImage);
	}
	else if(g_nServerType == 24)//南宁项目
	{
		//LogNormal("Nan ning...");
		plate.uPicSize = SaveImage(m_imgComposeSnap,strPicPath,2);		
	}
	else if(g_nServerType == 29 &&((plate.uViolationType ==DETECT_RESULT_PRESS_LINE)       ||   /*27 压黄线*/
								   (plate.uViolationType ==DETECT_RESULT_EVENT_GO_CHANGE)  ||   /*违章变道*/
								   (plate.uViolationType ==DETECT_RESULT_ELE_EVT_BIANDAO)  ||   /*29 变道 */
								   (plate.uViolationType ==DETECT_RESULT_PRESS_WHITELINE)  ||   /*52 压白线*/
								   (plate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION) )/*逆行*/
								   )//南昌项目  取两张图片
	{
		//违章图片分开存储
		CvRect srcRt;
		IplImage* pImage = cvCreateImage(cvSize(m_imgComposeSnap->width*2/3,m_imgComposeSnap->height),8,3);
		srcRt.x = 0;
		srcRt.y = 0;
		srcRt.width = m_imgComposeSnap->width*2/3;
		srcRt.height = m_imgComposeSnap->height;

		cvSetImageROI(m_imgComposeSnap,srcRt);
		cvResize(m_imgComposeSnap, pImage);
		cvResetImageROI(m_imgComposeSnap);

		plate.uPicSize = SaveImage(pImage,strPicPath);
		cvReleaseImage(&pImage);
	}
	else
	{
			if((g_nDetectMode == 2)&&(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)&&(g_nVtsPicMode == 1))//dsp方式超速图片格式需要特殊处理
			{
				IplImage* pImage = NULL;
				CvRect srcRt,dstRt;

				//LogNormal("Signal:%d,%d,%d,%d", \
				//	SignalTimeStamp.x, SignalTimeStamp.y, SignalTimeStamp.width, SignalTimeStamp.height);
				
				if(1 == g_PicFormatInfo.nSmallViolationPic)
				{
					pImage = cvCreateImage(cvSize(m_imgComposeSnap->width*3/2, m_imgComposeSnap->height/2), 8, 3);

					srcRt.x = 0;
					srcRt.y = m_imgComposeSnap->height/2;
					srcRt.width = m_imgComposeSnap->width;
					srcRt.height = m_imgComposeSnap->height/2;

					dstRt.x = 0;
					dstRt.y = 0;
					dstRt.width = m_imgComposeSnap->width;
					dstRt.height = m_imgComposeSnap->height/2;
					

					//取前两张
					cvSetImageROI(m_imgComposeSnap,srcRt);
					cvSetImageROI(pImage,dstRt);
					cvResize(m_imgComposeSnap, pImage);
					cvResetImageROI(m_imgComposeSnap);
					cvResetImageROI(pImage);

					//小图
					srcRt.x = 0;
					srcRt.y = 0;
					srcRt.width = m_imgComposeSnap->width/2;
					srcRt.height = m_imgComposeSnap->height/2-m_nExtentHeight;

					dstRt.x = m_imgComposeSnap->width;
					dstRt.y = 0;
					dstRt.width = m_imgComposeSnap->width/2;
					dstRt.height = m_imgComposeSnap->height/2;
					
					cvSetImageROI(m_imgComposeSnap,srcRt);
					cvSetImageROI(pImage,dstRt);
					cvResize(m_imgComposeSnap, pImage);
					cvResetImageROI(m_imgComposeSnap);
					cvResetImageROI(pImage);
				}
				else
				{
					pImage = cvCreateImage(cvSize(m_imgComposeSnap->width, m_imgComposeSnap->height/2), 8, 3);
					
					//第1张图
					srcRt.x = 0;
					srcRt.y = 0;
					srcRt.width = m_imgComposeSnap->width/2;
					srcRt.height = m_imgComposeSnap->height/2;

					dstRt.x = 0;
					dstRt.y = 0;
					dstRt.width = m_imgComposeSnap->width/2;
					dstRt.height = m_imgComposeSnap->height/2;
					
					cvSetImageROI(m_imgComposeSnap,srcRt);
					cvSetImageROI(pImage,dstRt);
					cvResize(m_imgComposeSnap, pImage);
					cvResetImageROI(m_imgComposeSnap);
					cvResetImageROI(pImage);
					
					//第2张图
					srcRt.x = 0;
					srcRt.y = m_imgComposeSnap->height/2;
					srcRt.width = m_imgComposeSnap->width/2;
					srcRt.height = m_imgComposeSnap->height/2;

					dstRt.x = m_imgComposeSnap->width/2;
					dstRt.y = 0;
					dstRt.width = m_imgComposeSnap->width/2;
					dstRt.height = m_imgComposeSnap->height/2;
					
					cvSetImageROI(m_imgComposeSnap,srcRt);
					cvSetImageROI(pImage,dstRt);
					cvResize(m_imgComposeSnap, pImage);
					cvResetImageROI(m_imgComposeSnap);
					cvResetImageROI(pImage);
				}

				plate.uPicSize = SaveImage(pImage,strPicPath,2);
				cvReleaseImage(&pImage);
			}
			else
			{
				#ifdef DSP_500W_TEST
				plate.uPicSize = SaveImage(m_imgComposeSnap2,strPicPath,2);
				#else
				plate.uPicSize = SaveImage(m_imgComposeSnap,strPicPath,2);
				#endif
			}
	}

	//LogNormal("SaveVtsImage 222\n");
}

//分开存储图像
void CRoadCarnumDetect::SaveSplitImage(RECORD_PLATE& plate,CvRect rtCarPos,PLATEPOSITION* TimeStamp,string& strPicPath2)
{
	//温岭卡口图片格式特殊处理
	if (g_nServerType == 0 && m_nSmallPic == 1 && m_nSaveImageCount == 1)
	{
		//先存第一张全景车身合成图片
		//LogNormal("11 SaveComposeImage seq:%d ", plate.uSeq);
		SaveComposeImage(plate,rtCarPos,TimeStamp);

		//再存第二张车牌小图
		CvRect rtPos;
		rtPos.x= plate.uPosLeft - 5;
		rtPos.y= plate.uPosTop - 5;
		rtPos.width= plate.uPosRight - plate.uPosLeft + 10;
		rtPos.height= plate.uPosBottom - plate.uPosTop + 20;

		std::string strPicPath;
		strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));

		if( (rtPos.width > 0) && (rtPos.height > 0))
		{
			plate.uSmallPicSize = SaveSmallImage(m_imgSnap,strPicPath,rtPos,m_pSmallJpgImage);
			SaveExistImage(strPicPath,m_pSmallJpgImage,plate.uSmallPicSize);
		}

		return;
	}

	//截取小图区域
    CvRect rtPos;
    if(m_nSmallPic == 1)
    {
		if (g_nServerType == 29)  //抠取车牌图片
		{
			rtPos.x = plate.uPosLeft;
			rtPos.y = plate.uPosTop;
			rtPos.width = plate.uPosRight - plate.uPosLeft+1;
			rtPos.height = plate.uPosBottom - plate.uPosTop+1;
		}
		else
		{
			if(rtCarPos.width > 0)
			{
				rtPos = rtCarPos;
			}
			else
			{
				rtPos = GetCarPos(plate);
			}
		}
    }

	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));

	plate.uPicWidth = m_imgSnap->width;
	plate.uPicHeight = m_imgSnap->height;

									
	if (g_nServerType == 13)
	{
		if (g_DistanceHostInfo.bDistanceCalculate == 1)
		{
				plate.uChannelID = m_nChannelId;

				IplImage* nRealImage = cvCreateImage(cvSize(m_imgSnap->width,m_imgSnap->height-m_nExtentHeight),8,3);
				CvRect rtRealImage;
				rtRealImage.x = 0;
				rtRealImage.y = 0;
				rtRealImage.width = m_imgSnap->width;
				rtRealImage.height = m_imgSnap->height-m_nExtentHeight;
				cvSetImageROI(m_imgSnap,rtRealImage);
				cvCopy(m_imgSnap,nRealImage);
				cvResetImageROI(m_imgSnap);

				plate.uPicSize = SaveImage(nRealImage,strPicPath);
				if (nRealImage != NULL)
				{
					cvReleaseImage(&nRealImage);
					nRealImage = NULL;
				}
				//车牌号码
				//memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
				string nStrNum = plate.chText;
				int nPos = -1;
				nPos = nStrNum.find("*");
				if (nPos <= -1)
				{
				        string strPlate("");
						strPlate.append((char*)&plate,sizeof(RECORD_PLATE));

						if (m_bConnect)
						{
							g_mvsCommunication.SetFlag(m_bConnect);
						}
						g_mvsCommunication.OnResult(strPlate);	
				}

		}
	}
	else if(g_nServerType == 23 || g_nServerType == 26)
	{
		if (g_DistanceHostInfo.bDistanceCalculate == 1)
		{
			plate.uChannelID = m_nChannelId;

			IplImage* nRealImage = cvCreateImage(cvSize(m_imgSnap->width,m_imgSnap->height-m_nExtentHeight),8,3);
			CvRect rtRealImage;
			rtRealImage.x = 0;
			rtRealImage.y = 0;
			rtRealImage.width = m_imgSnap->width;
			rtRealImage.height = m_imgSnap->height-m_nExtentHeight;
			cvSetImageROI(m_imgSnap,rtRealImage);
			cvCopy(m_imgSnap,nRealImage);
			cvResetImageROI(m_imgSnap);

			plate.uPicSize = SaveImage(nRealImage,strPicPath);
			if (nRealImage != NULL)
			{
				cvReleaseImage(&nRealImage);
				nRealImage = NULL;
			}
			//车牌号码
			//memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
			string nStrNum = plate.chText;

			plate.uOverSpeed = m_vtsGlobalPara.nSpeedVal;
			//LogNormal("plate.uOverSpeed:%d ", plate.uOverSpeed);

			int nPos = -1;
			nPos = nStrNum.find("*");
			if (nPos <= -1)
			{
				string strPlate("");
				strPlate.append((char*)&plate,sizeof(RECORD_PLATE));

				if (m_bConnect)
				{
					g_mvsCommunication.SetFlag(m_bConnect);
				}

				g_mvsCommunication.OnResult(strPlate);	
			}
		}
	}
	else{}

       //存第一张图
		//LogNormal("17 PutTextOnImage seq:%d ", plate.uSeq);
       PutTextOnImage(m_imgSnap,plate,0,TimeStamp);

	    if(m_nSaveImageCount == 2)
		{
			//存第二张图
			//LogNormal("18 PutTextOnImage seq:%d ", plate.uSeq);
			PutTextOnImage(m_imgPreSnap,plate,1,TimeStamp);

			if(g_nServerType == 7)
			{
				if(TimeStamp[0].ts > TimeStamp[1].ts)
				{
						string strTmp = strPicPath2;
						strPicPath2 = strPicPath;
						strPicPath = strTmp;
				}
				plate.uPicSize = SaveImage(m_imgSnap,strPicPath);
				SaveImage(m_imgPreSnap,strPicPath2);
			}
			else
			{
				plate.uPicSize = SaveImage(m_imgSnap,strPicPath);
				SaveImage(m_imgPreSnap,strPicPath,1);
			}

		}
		else//单张图
		{
			plate.uPicSize = SaveImage(m_imgSnap,strPicPath);
		}

		//存储小图
		if(m_nSmallPic == 1)
		{
				if( (rtPos.width > 0) && (rtPos.height > 0))
				{
						plate.uSmallPicSize = SaveSmallImage(m_imgSnap,strPicPath,rtPos,m_pSmallJpgImage);

						if(g_nServerType == 13 && g_nFtpServer == 1)//需要创建ftp-server
						{
							//	LogNormal("strSmallPicPath=%s\n",strSmallPicPath.c_str());
							std::string strSmallPicPath;
							g_MyCenterServer.GetPlatePicPath(plate,strSmallPicPath,1);

							SaveExistImage(strSmallPicPath,m_pSmallJpgImage,plate.uSmallPicSize);
						}
						else if(g_nServerType == 29)
						{
							SaveExistImage(strPicPath2,m_pSmallJpgImage,plate.uSmallPicSize);
						}
						else
						{
							SaveExistImage(strPicPath,m_pSmallJpgImage,plate.uSmallPicSize);
						}
				}
		}
}

//获取违章图片路径
int CRoadCarnumDetect::GetVtsPicSavePath(RECORD_PLATE& plate,int nPicCount,string& strPicPath,string& strPicPath2,string& strPicPath3)
{		
		int nSaveRet = 0;

		if(7 == g_nServerType)
		{
			for(int i =0;i<nPicCount;i++)
			{
				m_nRandCode[i] = g_RoadImcData.GetRandCode();

				if(i == 0)
				{
					g_RoadImcData.GetPlatePicPath(plate,strPicPath,1,m_nRandCode[i],i);
					LogTrace("VtsPath.txt", "plate.chPicPath:%s \n GetVtsPicSavePath: path: %s", \
						plate.chPicPath, strPicPath.c_str());
				}
				else if(i == 1)
				{
					g_RoadImcData.GetPlatePicPath(plate,strPicPath2,1,m_nRandCode[i],i);
				}
				else if(i == 2)
				{
					g_RoadImcData.GetPlatePicPath(plate,strPicPath3,1,m_nRandCode[i],i);
				}
			}

			nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);

			string strGBKPicPath = strPicPath;
			memcpy(plate.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());

			//nSaveRet = 1;
		}
		else if(g_nServerType == 13 && g_nFtpServer == 1)
		{
			g_MyCenterServer.GetPlatePicPath(plate,strPicPath,3);
			g_MyCenterServer.GetPlatePicPath(plate,strPicPath2,4);

			nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);

			memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

			//nSaveRet = 1;
		}
		else if(g_nServerType == 29)
		{
			g_RoadImcData.GetPlatePicPath(plate,strPicPath,4,0,1);
			memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

			nSaveRet = 1;
		}
		else
		{
			if(g_nPicSaveMode == 0)
			{
				//需要判断磁盘是否已经满
				g_FileManage.CheckDisk(false,false);
				//存储大图片
				strPicPath  = g_FileManage.GetPicPath();

				nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);
			}
			else
			{
				nSaveRet = GetPlatePicPath(plate,strPicPath);
			}

			//大图存储路径
			if(g_nPicSaveMode == 0)
			{
				memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());
			}
			else
			{
				string strGBKPicPath = strPicPath;
				g_skpDB.GBKToUTF8(strGBKPicPath);

				memcpy(plate.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());
			}
		}
		return nSaveRet;
}

//获取图片路径
int CRoadCarnumDetect::GetPicSavePath(RECORD_PLATE& plate,string& strPicPath,string& strPicPath2)
{					
					int nSaveRet = 0;
					if(7 != g_nServerType)
					{
						if(g_nServerType == 13 && g_nFtpServer == 1)//需要创建ftp-server
						{
							g_MyCenterServer.GetPlatePicPath(plate,strPicPath);
							memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

							//g_MyCenterServer.GetPlatePicPath(plate,strSmallPicPath,1);

							nSaveRet = 1;
						}
						else if(g_nServerType == 29)
						{
							g_RoadImcData.GetPlatePicPath(plate,strPicPath,4,0,1);
							g_RoadImcData.GetPlatePicPath(plate,strPicPath2,4,0,2);
							memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

							nSaveRet = 1;
						}
						else
						{
							if(g_nPicSaveMode == 0)
							{
								nSaveRet = GetPicPathAndSaveDB(strPicPath);
								//printf("==nSaveRet=%d\n",nSaveRet);
							}
							else
							{
								if(plate.uViolationType == DETECT_RESULT_BLACK_PLATE)
								{
									plate.uViolationType = 0;
									nSaveRet = GetPlatePicPath(plate,strPicPath);
									plate.uViolationType = DETECT_RESULT_BLACK_PLATE;
								}
								else
								{
									nSaveRet = GetPlatePicPath(plate,strPicPath);
								}
							}

							//大图存储路径
							if(g_nPicSaveMode == 0)
							{
								memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());
							}
							else
							{
								string strGBKPicPath = strPicPath;
								g_skpDB.GBKToUTF8(strGBKPicPath);

								memcpy(plate.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());
							}
						}

					}
					else
					{
						if(m_nSaveImageCount == 2)
						{
							g_RoadImcData.GetPlatePicPath(plate,strPicPath,4,0,1);
							g_RoadImcData.GetPlatePicPath(plate,strPicPath2,4,0,2);
						}
						else
						{
							g_RoadImcData.GetPlatePicPath(plate,strPicPath,0);
						}
						string strGBKPicPath = strPicPath;
						memcpy(plate.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());
						nSaveRet = 1;
					}

					return nSaveRet;
}

//输出车牌检测结果
void CRoadCarnumDetect::CarNumOutPut(std::vector<CarInfo>& vResult,std::vector<ObjSyncData> &syncData)
{
	if( g_nDetectMode == 2)
	{
		if(m_JpgFrameMap.size() < 1)
		{
				LogNormal("=CarNumOutPut=error==m_JpgFrameMap.size()=%d\n", m_JpgFrameMap.size());
				return;
		}

		if(m_imgSnap == NULL)
		{
			return;
		}
	}

    //相机同步信息
    std::vector<ObjSyncData>::iterator it_s;// = syncData.begin();


    std::vector<CarInfo>::iterator it = vResult.begin();

    while(it != vResult.end())
    {
        CarInfo cardNum = *it;
		
		if(g_nDetectMode == 2)
		{
			 //车牌位置合理性判断
			if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= m_imgSnap->width || cardNum.iy >= m_imgSnap->height - m_nExtentHeight
				  || (cardNum.ix + cardNum.iwidth >= m_imgSnap->width) || (cardNum.iy + cardNum.iheight >= m_imgSnap->height - m_nExtentHeight))
			{

				LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
				it++;
				continue;
			}
		}
        /////
        {
			if (VEDIO_PICLIB == m_nVedioFormat)
			{
				if(strncmp(cardNum.strCarNum,"$$$$$$$",7) == 0)
				{
					it++;
					continue;
				}
			}
            //////////////////////////////


            std::string strCarNum;
            strCarNum = cardNum.strCarNum;

//                printf("strCarNum=%s\r\n",strCarNum.c_str());

            //判断是否有车牌的车
            bool bCarNum = true;
            bool bLoop = false;
            if( (cardNum.strCarNum[0] == '*')&& (cardNum.strCarNum[6] == '*') )
            {
                bCarNum = false;
                if(cardNum.strCarNum[1]=='+')
                {
                    bLoop = true;
                }
            }

            if(bCarNum)
            {
                //车牌号码转换
                CarNumConvert(strCarNum,cardNum.wj);

            }
            ////////////////////////////////////////////////////
                printf("cardNum.uSeq=%d,cardNum.imgIndex=%d\n",cardNum.uSeq,cardNum.imgIndex);

            PLATEPOSITION  TimeStamp[2];
			TimeStamp[0].uTimestamp = cardNum.ts/1000000;

			//LogNormal("cardNum.uSeq=%lld,cardNum.m_useq=%lld\n",cardNum.uSeq,cardNum.m_useq);

			bool bLightImage = false;
            if(g_nHasHighLight == 1)//有爆闪灯
            {
				#ifndef ALGORITHM_YUV
                 if(cardNum.m_UseShutter)//找到亮图
                 {
					  bLightImage = true;

					 //LogNormal("cardNum.uSeq=%lld,cardNum.m_useq=%lld",cardNum.uSeq,cardNum.m_useq);

                      //if(cardNum.m_EstPos)//有车牌
                       {
                            //亮图需要在叠加信息前再获取一次
							bool bGetJpg = GetImageByIndex(cardNum.m_useq,0,TimeStamp,m_imgLightSnap,bLightImage);
					
							if(!bGetJpg)
							{						
								//LogNormal("bGetJpg 88 !");
								continue;
								//return;
							}
					   }
                  }
				#endif

				
           }
			
			//获取过滤图片
			#ifndef ALGORITHM_YUV
			{
				bool bGetJpg = GetImageByIndex(cardNum.uSeq,0,TimeStamp,m_imgSnap,bLightImage,bCarNum);
				if(!bGetJpg)
				{
					//LogNormal("CarNumOut vResult.size()=%d ", vResult.size());
					//LogNormal("bGetJpg 99 !");
					continue;
					//return;
				}
			}
			#else
				GetImageByIndex(cardNum.nFrame,0,TimeStamp,m_imgSnap,bLightImage,bCarNum);
			#endif


			#ifdef PLATEDETECT
			if( (cardNum.strCarNum[0] == '*')&& (cardNum.strCarNum[1] == '-') )
			{
				if(g_nDetectMode != 2)
				{
					cardNum.m_CarWholeRec = cardNum.m_NovehiclRec;
				}
				//LogNormal("CarWholeRec x=%d,y=%d,w=%d,h=%d\n",cardNum.m_CarWholeRec.x, cardNum.m_CarWholeRec.y,cardNum.m_CarWholeRec.width, cardNum.m_CarWholeRec.height);

				if(cardNum.m_CarWholeRec.width > 0 && cardNum.m_CarWholeRec.height > 0 )
				{
					printf("before IsCarWithPlate\n");
					if(m_PlateDetector.IsCarWithPlate(m_imgSnap,cardNum.m_CarWholeRec))
					{
						LogNormal("IsCarWithPlate\n");
						memcpy(cardNum.strCarNum,"*******",7);
						strCarNum = "*******";
					}
					printf("after IsCarWithPlate\n");

					//cvRectangle(m_imgSnap, cvPoint(cardNum.m_CarWholeRec.x, cardNum.m_CarWholeRec.y), 
					//cvPoint(cardNum.m_CarWholeRec.x+cardNum.m_CarWholeRec.width, cardNum.m_CarWholeRec.y+cardNum.m_CarWholeRec.height), CV_RGB(255,0,0), 20);
				}
			}
			#endif


			RECORD_PLATE plate;

			plate.uSeqID = cardNum.uSeq;
			plate.uDetectDirection = m_nDetectDirection;

			if (g_nServerType == 13 || g_nServerType == 17)
			{
				//经过时间(秒)
				plate.uTime = TimeStamp[0].uTimestamp;
				//毫秒
				plate.uMiTime = ((TimeStamp[0].ts)/1000)%1000;
			}
			else
			{
				//经过时间(秒)
				plate.uTime = cardNum.ts/1000000;

				if(((m_nDetectKind&DETECT_LOOP)==DETECT_LOOP))
				{
					plate.uTime += 2;
					TimeStamp[0].uTimestamp = plate.uTime;
				}
				//毫秒
				plate.uMiTime = (cardNum.ts/1000)%1000;
				
			}

			//车牌世界坐标 add by wantao
			#ifndef ALGORITHM_YUV
			plate.uLongitude = (UINT32)(cardNum.wx*10000*100);
			plate.uLatitude = (UINT32)(cardNum.wy*10000*100);
			#endif
            //地点
			if(m_strLocation.size() >= sizeof(plate.chPlace))
			{
				memcpy(plate.chPlace,m_strLocation.c_str(),sizeof(plate.chPlace));
			}
			else
			{
				memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
			}
            //行驶方向
            {
                plate.uDirection = m_nDirection;
            }
            
			
            //帧号
            plate.uSeqID = TimeStamp[0].uFieldSeq;

            //      printf("222cardNum.x=%d,y=%d,w=%d,h=%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
			//对车牌进行2次识别
			carnum_context vehicle_result;
			if( (g_nDetectMode == 2) && bCarNum)
			{
				#ifdef ALGORITHM_DL
				if( (m_nDetectKind&DETECT_TRUCK) || (m_nDetectKind&DETECT_CARCOLOR) || (m_nDetectKind&DETECT_TEXTURE))
				{
					CarNumTwiceDetect(cardNum,vehicle_result);
				}
				#endif
			}
			
			//车牌检测区域
			 CvRect rtRoi;
			 rtRoi.x = m_rtCarnumROI.x;
			 rtRoi.width = m_rtCarnumROI.width;
			 rtRoi.y = m_rtCarnumROI.y*m_nDeinterlace;
			 rtRoi.height = m_rtCarnumROI.height*m_nDeinterlace;

			 #ifdef ALGORITHM_DL
			 if( g_nDetectMode != 2)
			 {
				 if(cardNum.m_CarWholeRec.width > 0)
				 {
					rtRoi = cardNum.m_CarWholeRec;
				 }
			 }
			 #endif
			
			 printf("m_rtCarnumROI.x=%d,m_rtCarnumROI.y=%d,m_rtCarnumROI.width=%d,m_rtCarnumROI.height=%d\n",m_rtCarnumROI.x,m_rtCarnumROI.y,m_rtCarnumROI.width,m_rtCarnumROI.height);
			 printf("cardNum.m_CarWholeRec.x=%d,cardNum.m_CarWholeRec.y=%d,cardNum.m_CarWholeRec.width=%d,cardNum.m_CarWholeRec.height=%d\n",cardNum.m_CarWholeRec.x,cardNum.m_CarWholeRec.y,cardNum.m_CarWholeRec.width,cardNum.m_CarWholeRec.height);
			 printf("strCarNum=%s,rtRoi.x=%d,rtRoi.y=%d,rtRoi.width=%d,rtRoi.height=%d\n",cardNum.strCarNum,rtRoi.x,rtRoi.y,rtRoi.width,rtRoi.height);
			// cvDrawRect(m_imgSnap, cvPoint(rtRoi.x, rtRoi.y), cvPoint(rtRoi.x+rtRoi.width-1, rtRoi.y+rtRoi.height-1), CV_RGB(255,0,0), 2);	

				#ifdef ALGORITHM_DL	
			 if( (g_nDetectMode == 2) && bCarNum)
			 {
				vector<CAR_CONTEXT> vtCarContext;
				mvcarnumdetect.get_CarInfo(vtCarContext);//&&&&

				if(cardNum.m_CarWholeRec.width > 0)
				{
					rtRoi = cardNum.m_CarWholeRec;
				}
				if(vtCarContext.size() > 0)
				{
					vector<CAR_CONTEXT>::iterator it_b =  vtCarContext.begin();
					if(it_b->carnum != NULL)
					{

						if(it_b->carnum->position.y >= ((m_imgSnap->height-m_nExtentHeight) * 1 / 2))
						{
						
							printf("it->carnum=%s\n",it_b->carnum->carnum);
							rtRoi = it_b->position;

							memcpy(cardNum.strCarNum,it_b->carnum->carnum,7);
							memcpy(cardNum.wj,it_b->carnum->wjcarnum,2);
							string strTemp(cardNum.strCarNum);
							strCarNum = strTemp;
							CarNumConvert(strCarNum,cardNum.wj);
							cardNum.color      = it_b->carnum->color;
							printf("\n\nbefor find_carnum=CarNum=%s,x=%d,y=%d,w=%d,h=%d\n",cardNum.strCarNum,cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
							cardNum.ix      = it_b->carnum->position.x;
							cardNum.iy      = it_b->carnum->position.y;
							cardNum.iwidth  = it_b->carnum->position.width;
							cardNum.iheight = it_b->carnum->position.height;
							printf("\n\nafter find_carnum=CarNum=%s,x=%d,y=%d,w=%d,h=%d\n",cardNum.strCarNum,cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
							//cardNum.vehicle_type = it_b->carnum->vehicle_type;
							//LogNormal("CarNum:%s,Type:%d",it_b->carnum->carnum,it_b->carnum->vehicle_type);
							cardNum.carnumrow = it_b->carnum->carnumrow;
							bCarNum = true;
						}
					}
					else
					{
						rtRoi = it_b->position;
						printf("it->carnum====================\n");
					}

				}
				else
				{
				  
				   rtRoi = cardNum.m_CarWholeRec;
				}

				printf("CarColorDetect vtCarContext.size()=%d,rtRoi.x=%d,rtRoi.y=%d,rtRoi.width=%d,rtRoi.height=%d\n",vtCarContext.size(),rtRoi.x,rtRoi.y,rtRoi.width,rtRoi.height);
				
			 }
				#endif

			 if (strCarNum[0] != '*')
			 {
				 vector<string>::iterator iter = m_vecCachePlate.begin();
				 for (;iter != m_vecCachePlate.end();++iter)
				 {
					 if (*iter == strCarNum)
					 {				 
						 LogNormal("找到相同车牌%s\n",strCarNum.c_str());
						 break;
					 }
				 }
				 if ((iter != m_vecCachePlate.end())&&(m_vecCachePlate.size() > 0))
				 {
					 it++;
					 continue;
				 }
				 else
				 {
					 if (m_vecCachePlate.size() < 10)
					 {
						 m_vecCachePlate.push_back(strCarNum);
					 }
					 else
					 {
						 printf("erase2 %s,%d\n",*(m_vecCachePlate.begin()),m_vecCachePlate.size());
						 m_vecCachePlate.erase(m_vecCachePlate.begin());
						 m_vecCachePlate.push_back(strCarNum);
					 }
				 }
			 }
			
			 //车牌号码
			 memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

			#ifndef ALGORITHM_YUV
			//车牌结构
			 plate.uPlateType = cardNum.carnumrow;
			//车牌颜色
            plate.uColor = cardNum.color;
			#else
			plate.uColor = cardNum.nColor;
			plate.uPlateType = cardNum.uPlateType;
			printf("===plate.uPlateType=%d\n",plate.uPlateType);
			#endif
			
            if(plate.uColor <= 0)
            {
               plate.uColor =  CARNUM_OTHER;
            }

			//设置图像实际大小（不包含下面的黑色区域,）
            CvRect rtRealImage;
            rtRealImage.x = 0;
            rtRealImage.y = 0;
            rtRealImage.width = m_imgSnap->width;
            rtRealImage.height = m_imgSnap->height-m_nExtentHeight;
            if(m_nWordPos == 1)
            {
                rtRealImage.y += m_nExtentHeight;
            }

            //车牌区域
            CvRect rtCarnum;
			#ifndef ALGORITHM_YUV
            rtCarnum.x = cardNum.ix;
            rtCarnum.y = cardNum.iy*m_nDeinterlace;
            rtCarnum.width = cardNum.iwidth;
            rtCarnum.height = cardNum.iheight*m_nDeinterlace;
			#else
			rtCarnum.x = cardNum.nX;
            rtCarnum.y = cardNum.nY;
            rtCarnum.width = cardNum.nWidth;
            rtCarnum.height = cardNum.nHeight;
			#endif

			printf("rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);

            if(rtCarnum.x+rtCarnum.width>=m_imgSnap->width)
            {
                rtCarnum.width = m_imgSnap->width - rtCarnum.x-1;
            }
            if(rtCarnum.y+rtCarnum.height>=m_imgSnap->height-m_nExtentHeight)
            {
                rtCarnum.height = m_imgSnap->height-m_nExtentHeight - rtCarnum.y-1;
            }

			//printf("11111 rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);
			
			#ifndef ALGORITHM_YUV
            carnum_context context;
            context.position =  rtCarnum;
            context.vehicle_type = cardNum.vehicle_type;
            context.color = (CARNUM_COLOR)cardNum.color;
            context.mean  = cardNum.mean;
            context.stddev = cardNum.stddev;
			context.carnumrow = (CARNUM_ROW)cardNum.carnumrow;

            context.VerticalTheta  = cardNum.VerticalTheta;
            context.HorizontalTheta = cardNum.HorizontalTheta;

            memcpy(context.carnum,cardNum.strCarNum,sizeof(context.carnum));

            context.smearnum = cardNum.smearCount;
            memcpy(context.smearrect,cardNum.smear,sizeof(CvRect)*(cardNum.smearCount));

			if(g_nDetectMode == 2)
			{
				context.nCarNumDirection = (carnumdirection)m_nDetectDirection;
			}
			else
			{
				context.nCarNumDirection = (carnumdirection)cardNum.nDirection;//目标运动方向
				//图库识别
				if (VEDIO_PICLIB == m_nVedioFormat)
				{
					context.nCarNumDirection = (carnumdirection)m_nDetectDirection;
				}
			}

			//修改线圈位置
			CvPoint loopPtUp;
			CvPoint loopPtDowon;
			loopPtUp.x = 0;
			loopPtUp.y = 0;
			loopPtDowon.x = 0;
			loopPtDowon.y = 0;

			// printf("2222222 rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);


			//printf("33333 rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);

				//获取机动车颜色
				if(m_nDetectKind&DETECT_CARCOLOR)
				{
					if(rtRoi.width > 0)
					CarColorDetect(bCarNum,bLoop,cardNum,plate,context,loopPtUp,loopPtDowon,rtRoi);
				}

				//printf("444444444444 rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);

				//提取车身特征向量
				if(m_nDetectKind&DETECT_TEXTURE)
				{
					//提取车身纹理特征向量
					if(bCarNum)
					{
						//printf("before CarLabelDetect\n");
						
						if(rtRoi.width > 0)
						CarLabelDetect(cardNum,plate,context,loopPtUp,loopPtDowon,rtRoi);

						//printf("after CarLabelDetect\n");

					}
				}

				//获取车辆类型（高16位卡车、巴士、轿车等，低16位大、中、小车）
				if(m_nDetectKind&DETECT_TRUCK)
				{
					if( bCarNum)
					{
						DetectTruck(cardNum,plate,context,rtRealImage,m_imgSnap);

						if(plate.uTypeDetail > 0)
						{
							GetVtsResult(cardNum,plate);
						}
					}
				}
				
				int nBeltResult = 0;
				//安全带以及人脸检测
				if(m_nDetectKind&DETECT_FACE)
				{
					if( bCarNum)
					{
						#ifdef BELTDETECT
						vector<FaceRt> vecP;
						BeltDetect(cardNum,plate,context,vecP,nBeltResult);
						//LogNormal("vecP.size()=%d,nBeltResult=%d\n",vecP.size(),nBeltResult);

						if(vecP.size() > 0)
						{
							PutFaceOnImage(vecP);
						}
						#endif
					}
				}

			//遮阳板检测
			if(m_bDetectShield)
			{
				if(bCarNum)
				{
				  // CarShieldDetect(cardNum,plate,rtRealImage,rtCarnum);
				}
			}
			#endif
			//printf("5555555555 rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);

            //车道逻辑编号(暂时取第一个)
			#ifndef ALGORITHM_YUV
			int nRoadIndex = cardNum.RoadIndex;
			#else
			int nRoadIndex = cardNum.nChannel;
			#endif
            if(g_nDetectMode == 2)
            {
                plate.uRoadWayID = nRoadIndex;
            }
            else
            {
                vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
                while(it != m_vtsObjectRegion.end())
                {
                    if(-1 == nRoadIndex)
                    {
                        plate.uRoadWayID = it->nVerRoadIndex;
                        nRoadIndex = it->nRoadIndex;
                        break;
                    }
                    else if(it->nRoadIndex == nRoadIndex)
                    {
                        plate.uRoadWayID = it->nVerRoadIndex;
                        break;
                    }
                    it++;
                }
            }
			
			
            //LogNormal("==1111=plate.uRoadWayID=%d=cardNum.RoadIndex=%d=\n", plate.uRoadWayID,cardNum.RoadIndex);

			//车速
			#ifndef ALGORITHM_YUV
			double dSpeed =   sqrt(cardNum.vx*cardNum.vx+cardNum.vy*cardNum.vy);
			#else
			double dSpeed =   cardNum.fVelocity;
			#endif
			
			//如果做了雷达测速，做雷达测速矫正
			if( (m_nDetectKind&DETECT_RADAR) == DETECT_RADAR )
			{
				//读取雷达测速矫正
				//已经读取--m_fSpeedFeather===CRoadCarnumDetect::DetectObject(rgb_buf  rgbBuf)
				//更正速度
				float fSpeed = dSpeed * (1+m_fSpeedFactor * 0.01);
				dSpeed = fSpeed;
			}
			plate.uSpeed = (UINT32)(dSpeed+0.5);
			
			//超速
			if(g_PicFormatInfo.nSpeedLimit == 1)
			{
				int nOnlyOverSpedMax = 10;
				{
					paraDetectList::iterator it_p = m_roadParamInlist.begin();
					while( it_p !=  m_roadParamInlist.end() )
					{
						if(it_p->nChannelID == plate.uRoadWayID)
						{
							nOnlyOverSpedMax = it_p->m_nOnlyOverSpedMax;
							if(nOnlyOverSpedMax == 0)
							{
								if(plate.uType == SMALL_CAR)
								{
									nOnlyOverSpedMax = it_p->m_nAvgSpeedMin;
								}
								else if(plate.uType == BIG_CAR)
								{
									nOnlyOverSpedMax = it_p->m_nAvgSpeedMax;
								}
							}
						}
						it_p++;
					}
				}
				plate.uLimitSpeed = nOnlyOverSpedMax;
			}


            //车辆类型
			#ifndef ALGORITHM_YUV
			plate.uType = cardNum.vehicle_type;
			#else
			plate.uType = cardNum.nVehicleType;
			#endif

			if(m_nDetectKind&DETECT_TRUCK)
			{
				if(plate.uTypeDetail == MINI_TRUCK || plate.uTypeDetail == TAXI)
				{
					plate.uType = SMALL;
				}
				else if(plate.uTypeDetail == MIDDLEBUS_TYPE)
				{
					plate.uType = MIDDLE;
				}
				else if(plate.uTypeDetail == BUS_TYPE || plate.uTypeDetail == TRUCK_TYPE)
				{
					plate.uType = BIG;
				}
			}
			CarTypeConvert(plate);
			
			//流量统计
			if(bCarNum)
			{
				//LogNormal("%s-%d-%d",plate.chText,cardNum.vehicle_type,plate.uType);
				StatisticVehicleFlux(cardNum,plate);
			}

            /////////////////////////////
			//获取亮图
            if(g_nHasHighLight == 1)//有爆闪灯
            {
				#ifndef ALGORITHM_YUV
				if(m_nSmallPic == 1 && m_nSaveImageCount == 1)//暂存过滤结果
				{
					cvCopy(m_imgSnap,m_imgPreSnap);
				}

                if(cardNum.m_UseShutter)//找到亮图
                {
                    printf("====cardNum.m_EstPos=%d\n",cardNum.m_EstPos);
                    if(DayOrNight(1) == 0)//根据客户端开关灯时间判断白天还是晚上
                    {
						//LogNormal("找到亮图并且车牌不存在x=%d,y=%d,w=%d,h=%d",cardNum.m_CarnumPos.x,cardNum.m_CarnumPos.y,cardNum.m_CarnumPos.width,cardNum.m_CarnumPos.height);
                        if(cardNum.m_CarnumPos.width > 0)
						{
							rtCarnum.x = cardNum.m_CarnumPos.x;
							rtCarnum.y = cardNum.m_CarnumPos.y;
							rtCarnum.width = cardNum.m_CarnumPos.width;
							rtCarnum.height = cardNum.m_CarnumPos.height;
						}
                        //获取亮图
						cvCopy(m_imgLightSnap,m_imgSnap);
                    }
                }
				#endif
            }

			//printf("66666666 rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);

            //车牌位置
			if(m_nWordPos == 1)
            {
                rtCarnum.y += m_nExtentHeight;
            }

			printf("\n\n777777777777 rtCarnum:%d,%d,%d,%d\n",rtCarnum.x, rtCarnum.y, rtCarnum.width, rtCarnum.height);

            plate.uPosLeft = rtCarnum.x;
            plate.uPosTop = rtCarnum.y;
            plate.uPosRight = rtCarnum.x+rtCarnum.width-1;
            plate.uPosBottom = rtCarnum.y+rtCarnum.height-1;
		
			printf("\n\n-----Num:%s, platepos:%d,%d,%d,%d\n", plate.chText,plate.uPosLeft,plate.uPosTop,plate.uPosRight,plate.uPosBottom);
            //图片尺寸
            plate.uPicWidth = m_imgDestSnap->width;
            plate.uPicHeight = m_imgDestSnap->height;

			//检测是否布控报警--add by ywx
			if(g_nDetectSpecialCarNum == 1)
			{
				plate.uAlarmKind = g_skpDB.IsSpecialCard(strCarNum);
				if(plate.uAlarmKind == 1)
				{
					plate.uViolationType = DETECT_RESULT_BLACK_PLATE;
					LogNormal("黑名单内车辆[%s]出现!\r\n", strCarNum.c_str());
				}
				else if(plate.uAlarmKind == 2)
				{
					plate.uViolationType = DETECT_RESULT_WHITE_PLATE;
					LogNormal("白名单内车辆[%s]出现!\r\n", strCarNum.c_str());
				}
			}
			
			/*#ifndef ALGORITHM_YUV
			if((cardNum.strCarNum[0] == '*')&& (cardNum.strCarNum[1] == '-'))
			{
					if(g_nServerType != 26 && g_nServerType != 23)
					plate.uViolationType = DETECT_RESULT_NOCARNUM;
			}
			#endif*/

			//尾号限行
			if(g_PlateLimit.nIsPlateLimit == 1)
			{
				if(cardNum.strCarNum[6] == g_PlateLimit.chPlateNumber[0] ||\
					cardNum.strCarNum[6] == g_PlateLimit.chPlateNumber[1] ||\
					cardNum.strCarNum[6] == g_PlateLimit.chPlateNumber[2] )
				{
					plate.uViolationType = DETECT_RESULT_PLATE_LIMIT;
				}
			}

            if((m_nDetectKind&DETECT_LOOP) && (g_nDetectMode != 2))//线圈检测才回写
            {
				OutPutLoopResult(cardNum,plate,it_s,TimeStamp, bLightImage, bLoop);
			}
            else //不进行回写直接写数据库并发送给客户端
            {
				#ifndef ALGORITHM_YUV
				unsigned int uSeq = cardNum.uSeq;
				#else
				unsigned int uSeq = cardNum.nFrame;
				#endif

                if( (m_nSaveImageCount == 2) &&(g_nPicMode == 1) && (g_nDetectMode != 2)&&(g_nServerType != 7)) //两张分开
                {
                    plate.uSeq = uSeq;
                    OutPutPreResult(plate,TimeStamp,0);
                }
                else
                {
                     //获取图片路径
                    std::string strPicPath,strPicPath2;
                    int nSaveRet = GetPicSavePath(plate,strPicPath,strPicPath2);
					//获取第2张图的时间
                    if(m_nSaveImageCount == 2)
                    {
						UINT32 nPreSeq = uSeq;

                        int ndtSeq = 5;
                        if(m_nFrequency <= 10)
                        {
                            ndtSeq = 3;
                        }
						
                        if(m_nDetectDirection == 0)
                        {
                            nPreSeq = uSeq - ndtSeq;//前牌
                        }
                        else
                        {
                            nPreSeq = uSeq + ndtSeq;//尾牌
                        }
						
						#ifndef ALGORITHM_YUV
						if(g_nDetectMode == 2)
						{
							nPreSeq = cardNum.uSedImgFramseq;//第2张图帧号
						}
						#endif

                        GetImageByIndex(nPreSeq,TimeStamp[0].uSeq,&TimeStamp[1],m_imgPreSnap);
                        plate.uTime2 = TimeStamp[1].uTimestamp;
                        plate.uMiTime2 = (TimeStamp[1].ts/1000)%1000;
					}
					
					#ifndef ALGORITHM_YUV
					CvRect rtCarPos = cardNum.m_CarWholeRec;
					#else
					CvRect rtCarPos = cvRect(0,0,0,0);
					#endif
                     
					if((3 != g_nPicMode) || (plate.chText[0] == '*'))
					{
					 //叠加图片
                      if(g_nPicMode != 1)
                      {
						 // LogNormal("22 SaveComposeImage seq:%d ", plate.uSeq);
							SaveComposeImage(plate,rtCarPos,TimeStamp);
                      }
                      else   //分开
                      {
							SaveSplitImage(plate,rtCarPos,TimeStamp,strPicPath2);
                      }

					printf("\n\n SaveRet:%d, m_bConnect:%d\n",nSaveRet,m_bConnect);
                    //保存车牌记录
                    if(nSaveRet>0)
                    {
                        //LogNormal("==before save=plate.uRoadWayID=%d==\n", plate.uRoadWayID);

						 //相机同步信息
						 SYN_CHAR_DATA syn_char_data;
						printf("\n\nSaveDB----Num:%s,platepos:%d,%d,%d,%d\n", plate.chText,plate.uPosLeft,  plate.uPosTop,  plate.uPosRight,  plate.uPosBottom);
                        g_skpDB.SavePlate(m_nChannelId,plate,0,&syn_char_data);
						
						if(7 == g_nServerType)
						{							
							g_RoadImcData.AddCarNumberData(plate,strPicPath);
							LogTrace("VtsPath.txt", "plate.chPicPath:%s \n CarNumOutPut: path: %s", \
								plate.chPicPath, strPicPath.c_str());
						}
                    }

                    //将车牌信息送客户端
                    if(m_bConnect)
                    {
						//车牌号码
						memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
						printf("\n\n-SendClient----Num:%s, platepos:%d,%d,%d,%d\n",plate.chText,plate.uPosLeft,  plate.uPosTop,  plate.uPosRight,  plate.uPosBottom);
						SendResult(plate,uSeq);
                    }
					}

				}
			}

			if(g_nDoSynProcess == 1)	//前后牌比对
			{
			memcpy(plate.chText, cardNum.strCarNum, 7);
			memcpy(plate.chText + 7, cardNum.wj, 2);

			plate.uCarPosLeft = cardNum.m_CarWholeRec.x;
			plate.uCarPosTop = cardNum.m_CarWholeRec.y;
			plate.uCarPosRight = plate.uCarPosLeft + cardNum.m_CarWholeRec.width;
			plate.uCarPosBottom = plate.uCarPosTop + cardNum.m_CarWholeRec.height;

			plate.uDetectDirection = m_nDetectDirection;
			plate.uChannelID = m_nChannelId;
			//LogNormal("AddDspPlateMatchFortianjin11 dir:%d ", plate.uDirection);
			//LogNormal("uDirection:%d, cardNum:%s\n", m_nDetectDirection, plate.chText);
			LogTrace("./Log/chanId.txt","before %s,Dirid:%d,type=%d,%s, chanid=%d,seq=%d\n",
				plate.chText,
				plate.uDetectDirection,
				plate.uViolationType, 
				plate.chPlace,
				plate.uChannelID,plate.uSeq);

			if( (((3 == g_nPicMode) && (m_nDetectDirection == 1)) || (m_nDetectDirection == 0)) &&(plate.chText[0] != '*') )
			{
				//if(!m_pMachPlate->bUse)
				{
					this->ReInitMatchPlate(m_pMachPlate);
					//m_pMachPlate->bUse = true;

					if(m_strPicMatch.size() > 0)
					{
						//memcpy(m_pMachPlate->pPicArray[0],m_strPicMatch.c_str(), m_strPicMatch.size());
						//m_pMachPlate->nSizeArray[0] = m_strPicMatch.size();
						AddJpgToMatch(m_strPicMatch, 3, m_pMachPlate);
					}

					//LogNormal("m_pMachPlate 00 strJpg:%d ", m_strPicMatch.size());

					m_pMachPlate->dspRecord.uChannelID = m_nChannelId;
					m_pMachPlate->dspRecord.uSeqID = cardNum.uSeq;
					plate.uSeqID = cardNum.uSeq;
					AddDspPlateMatchVts(plate, m_pMachPlate);
					//m_pMachPlate->bUse = false;//使用完更新状态
				}
				//else
				//{
				//	LogNormal("22m_pMachPlate in Use uKey:%lld,%lld,%lld !", \
				//		m_pMachPlate->uKeyArray[0], m_pMachPlate->uKeyArray[1], m_pMachPlate->uKeyArray[2]);
				//}
			}

			}
        }
        it ++;
    }
}

//发送检测结果到客户端
void CRoadCarnumDetect::SendResult(RECORD_PLATE& plate,unsigned int uSeq)
{
		SRIP_DETECT_HEADER sDetectHeader;
        sDetectHeader.uChannelID = m_nChannelId;
        //车牌检测类型
        sDetectHeader.uDetectType = SRIP_CARD_RESULT;
        sDetectHeader.uTimestamp = plate.uTime;
        sDetectHeader.uSeq = uSeq;

        std::string result;
        
        //判断车牌位置是否需要扩充
        GetCarPostion(plate);

		RECORD_PLATE_CLIENT plate_client;
        memcpy(&plate_client,&plate,sizeof(RECORD_PLATE_CLIENT));
		plate_client.uDetailCarType =  plate.uDetailCarType;
		plate_client.uDetailCarBrand =  plate.uDetailCarBrand;

        result.append((char*)&plate_client,sizeof(plate_client));

        result.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
        g_skpChannelCenter.AddResult(result);
}

//车牌检测(使用yuv图像)
void CRoadCarnumDetect::DetectCarNumYUV(unsigned char* img,int64_t ts,unsigned long  nSeq)
{
	#ifdef ALGORITHM_YUV_CARNUM
	MVPLR_Frame  PLR_Frame;
	carnum_information vehicle_result[10];

	PLR_Frame.pImageIn = img;
	PLR_Frame.nWidth = m_img->width;
	PLR_Frame.nHeight = m_img->height;
	PLR_Frame.nDayFlag = m_nDayNight; //白天
	PLR_Frame.nDirection = m_nDetectDirection;//行驶方向
	printf("PLR_Frame.nWidth=%d,PLR_Frame.nHeight=%d,PLR_Frame.nDayFlag=%d,PLR_Frame.nDirection=%d\n",PLR_Frame.nWidth,PLR_Frame.nHeight,PLR_Frame.nDayFlag,PLR_Frame.nDirection);

	//功能函数调用
	int number = MVPLR_find_carnum(&PLR_Frame,vehicle_result );

	BYTE* pBuf = m_chFrameBuffer[m_nReadIndex]+sizeof(yuv_video_buf)+m_img->width*2*(m_img->height+m_nExtentHeight);
	memcpy(pBuf,&number,sizeof(int));//记录检出的车牌数目
    pBuf  += sizeof(int);
	printf("MVPLR_find_carnum number =%d\n",number);
	for(int i =0; i<number; i++)
    {
            CarInfo    carnums;
            memcpy(carnums.strCarNum,vehicle_result[i].carnum,7);
            memcpy(carnums.wj,vehicle_result[i].wjcarnum,2);

			#ifdef ALGORITHM_YUV
			carnums.nColor = vehicle_result[i].color;
            carnums.nVehicleType=vehicle_result[i].vehicle_type;
            carnums.nX      = vehicle_result[i].position.x;
            carnums.nY      = (vehicle_result[i].position.y);
            carnums.nWidth  = vehicle_result[i].position.width;
            carnums.nHeight = (vehicle_result[i].position.height);
			carnums.nFrame   = nSeq;
            carnums.uPlateType = vehicle_result[i].carnumrow;//车牌结构
			carnums.nCarnumMean    = vehicle_result[i].mean;
			printf("********************************carnums.strCarNum =%s,carnums.uPlateType=%d\n",carnums.strCarNum,carnums.uPlateType);
			#else
			carnums.color = vehicle_result[i].color;
            carnums.vehicle_type=vehicle_result[i].vehicle_type;
			carnums.ix      = vehicle_result[i].position.x;
            carnums.iy      = (vehicle_result[i].position.y);
            carnums.iwidth  = vehicle_result[i].position.width;
            carnums.iheight = (vehicle_result[i].position.height);
			carnums.uSeq   = nSeq;
            carnums.carnumrow = vehicle_result[i].carnumrow;//车牌结构
			carnums.mean    = vehicle_result[i].mean;
			#endif

			carnums.iscarnum= vehicle_result[i].iscarnum;

            carnums.ts      = ts;
           
            memcpy(pBuf+i*sizeof(CarInfo),&carnums,sizeof(CarInfo));//记录车牌信息
    }

	//地面亮度

	#endif
}

//车牌检测
void CRoadCarnumDetect::DetectCarNum(IplImage* img,CvRect rtROI,UINT32 uTimestamp,int64_t ts,unsigned long  nSeq,int deinterlace)
{
	#ifndef ALGORITHM_YUV_CARNUM
    int nCarNumResult=0;
    char tmp[32]= {0};
    IplImage* tmpimg=NULL;

    CvRect rect = rtROI;//检测区域

    carnum_context vehicle_result[CARNUMSIZE];
    road_context context;

	struct timeval tv1,tv2;
    if(g_nPrintfTime == 1)
	{
		gettimeofday(&tv1,NULL);
		//LogTrace("time-test.log","before find_carnum==nSeq=%lld,time = %s.%03d\n",nSeq,GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
	}

    img->ID = m_nChannelId;

    //车牌检测
    if(rect.width > 0 && rect.height > 0 && (!m_bTestResult))
	{
		#ifdef ALGORITHM_DL
		cvConvertImage(img,img,CV_CVTIMG_SWAP_RB);
		#endif

		nCarNumResult = mvcarnumdetect.find_carnum(NULL,img,tmp,&tmpimg,rect,deinterlace,vehicle_result,&context,NULL,m_LoopParmarer);

		#ifdef ALGORITHM_DL
		cvConvertImage(img,img,CV_CVTIMG_SWAP_RB);
		#endif
	}
	//nCarNumResult = 0;

   if(g_nPrintfTime == 1)
   {
       gettimeofday(&tv2,NULL);
	   FILE* fp = fopen("time.log","ab+");
       fprintf(fp,"find_carnum==nSeq=%lld,t1=%lld,t2=%lld,dt = %lld\n",nSeq,(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
	   fclose(fp);
   }

    //////////////////////将车牌检测结果写入车牌检出队列
    {
        BYTE* pBuf = NULL;

        if(m_nDeinterlace == 2)
        {
            pBuf = m_chFrameBuffer[m_nReadIndex]+sizeof(yuv_video_buf)+img->widthStep*(img->height*deinterlace/2+m_nExtentHeight/2);
        }
        else
        {
            pBuf = m_chFrameBuffer[m_nReadIndex]+sizeof(yuv_video_buf)+img->widthStep*(img->height+m_nExtentHeight);
        }
        //memcpy(pBuf,&context,sizeof(road_context));//记录车牌和区域亮度信息
        //pBuf += sizeof(road_context);
        memcpy(pBuf,&nCarNumResult,sizeof(int));//记录检出的车牌数目
        pBuf  += sizeof(int);

        for(int i =0; i<nCarNumResult; i++)
        {
            CarInfo    carnums;
            memcpy(carnums.strCarNum,vehicle_result[i].carnum,7);
            memcpy(carnums.wj,vehicle_result[i].wjcarnum,2);
            carnums.color = vehicle_result[i].color;
            carnums.vehicle_type=vehicle_result[i].vehicle_type;

            carnums.ix      = vehicle_result[i].position.x;
            carnums.iy      = (vehicle_result[i].position.y*deinterlace)/m_nDeinterlace;
            carnums.iwidth  = vehicle_result[i].position.width;
            carnums.iheight = (vehicle_result[i].position.height*deinterlace)/m_nDeinterlace;

			carnums.bIsMotorCycle = vehicle_result[i].bIsMotorCycle;

            carnums.iscarnum= vehicle_result[i].iscarnum;
            carnums.mean    = vehicle_result[i].mean;
            carnums.stddev  = vehicle_result[i].stddev;

			if(vehicle_result[i].smearnum > 20 || vehicle_result[i].smearnum < 0)
			{
				vehicle_result[i].smearnum = 0;
			}

            carnums.smearCount  = vehicle_result[i].smearnum;
            memcpy(carnums.smear,vehicle_result[i].smearrect,sizeof(CvRect)*(carnums.smearCount));

            carnums.VerticalTheta=vehicle_result[i].VerticalTheta;
            carnums.HorizontalTheta=vehicle_result[i].HorizontalTheta;

            carnums.uTimestamp = uTimestamp;
            carnums.ts      = ts;
            carnums.uSeq   = nSeq;
            carnums.carnumrow = vehicle_result[i].carnumrow;//车牌结构
			
			#ifdef ALGORITHM_DL
			vector<CAR_CONTEXT> vtCarContext;
			mvcarnumdetect.get_CarInfo(vtCarContext);
			vector<CAR_CONTEXT>::iterator it_b =  vtCarContext.begin();
			while(it_b != vtCarContext.end())
			{
				if(it_b->carnum != NULL)
				{
					if(strncmp(it_b->carnum->carnum,carnums.strCarNum,7) == 0)
					{
						carnums.m_CarWholeRec = it_b->position;

						printf("it->carnum=%s,%d-%d-%d-%d\n",it_b->carnum->carnum,carnums.m_CarWholeRec.x,carnums.m_CarWholeRec.y,carnums.m_CarWholeRec.width,carnums.m_CarWholeRec.height);
						
						break;
					}
				}
			}
			#endif
			
            memcpy(pBuf+i*sizeof(CarInfo),&carnums,sizeof(CarInfo));//记录车牌信息
            //printf("deinterlace = %d,carnums.x=%d,carnums.y=%d,carnums.mean=%g,carnums.stddev=%g\n",deinterlace,carnums.ix,carnums.iy,carnums.mean,carnums.stddev);
        }
    }

    if(tmpimg!=NULL)
    {
        cvReleaseImage(&tmpimg);
        tmpimg = NULL;
    }
    //获取地面亮度
    if(img->width>1000)
    {
        if(rect.width > 0 && rect.height > 0)
        {
			struct timeval tv1,tv2;
			if(g_nPrintfTime == 1)
			{
				gettimeofday(&tv1,NULL);
			}

			m_CameraCrt.mvGetBGLight(img,rect,vehicle_result,nCarNumResult,m_LoopParmarer.iNvise_light,m_nDayNight);

			if(g_nPrintfTime == 1)
			{
				gettimeofday(&tv2,NULL);
				FILE* fp = fopen("time.log","ab+");
				fprintf(fp,"mvGetBGLight==nSeq=%lld,t1=%lld,t2=%lld,dt = %lld\n",nSeq,(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
				fclose(fp);
			}
        }
    }
	#endif
}

bool CRoadCarnumDetect::OnDetect(rgb_buf result)
{
    //重读车牌检测区域
    if(m_bReloadROI)
    {
        //读取车道标定信息
        vector<mvvideostd> vListStabBack;
		CvRect farrect;
		CvRect nearrect;
        if(!LoadRoadSettingInfo(1,vListStabBack,farrect,nearrect))
        {
			printf("OnDetect LoadRoadSettingInfo error \n");
            return false;
        }
		
		#ifdef ALGORITHM_YUV_CARNUM

		MVPLR_AlgInitPm PLR_InitParam;
		PLR_InitParam.src_width = m_img->width;
		PLR_InitParam.src_height = m_img->height;
		PLR_InitParam.reg_x = m_rtCarnumROI.x;
		PLR_InitParam.reg_y = m_rtCarnumROI.y;
		PLR_InitParam.reg_width = m_rtCarnumROI.width;
		PLR_InitParam.reg_height = m_rtCarnumROI.height;
		printf("PLR_InitParam x=%d,y=%d,w=%d,h=%d\n",PLR_InitParam.reg_x,PLR_InitParam.reg_y,PLR_InitParam.reg_width,PLR_InitParam.reg_height);

		MVPLR_AlgInit(&PLR_InitParam);

		printf("11 PLR_InitParam x=%d,y=%d,w=%d,h=%d\n",PLR_InitParam.reg_x,PLR_InitParam.reg_y,PLR_InitParam.reg_width,PLR_InitParam.reg_height);

		 //配置函数调用
		UINT32 nCfgType = 0;
		MVPLR_Config PLR_Config;

		nCfgType = 0;
		nCfgType = (nCfgType | SET_PLR_ENABLE) 
				  | (nCfgType | SET_IMAGE_WH)
				  | (nCfgType | SET_DETECT_RECT)
				  | (nCfgType | SET_NEW_CALIBRATION);

		//算法使能
		PLR_Config.bAlgEnable = 1;  
		//不做摩托牌
		PLR_Config.bMotorEnable = 0; 
		//原图宽高
		PLR_Config.nSrcWidth = PLR_InitParam.src_width;
		PLR_Config.nSrcHeight = PLR_InitParam.src_height;

		//小车牌
		PLR_Config.sSmallRect.x = farrect.x;  //可不设置
		PLR_Config.sSmallRect.y = farrect.y;
		PLR_Config.sSmallRect.width = farrect.width;
		PLR_Config.sSmallRect.height = farrect.height;
		//大车牌
		PLR_Config.sBigRect.x = nearrect.x;  //可不设置
		PLR_Config.sBigRect.y = nearrect.y;
		PLR_Config.sBigRect.width = nearrect.width;
		PLR_Config.sBigRect.height = nearrect.height;
		//车牌检测区
		PLR_Config.sDetectRect.x = PLR_InitParam.reg_x;
		PLR_Config.sDetectRect.y = PLR_InitParam.reg_y;
		PLR_Config.sDetectRect.width = PLR_InitParam.reg_width; 
		PLR_Config.sDetectRect.height = PLR_InitParam.reg_height;

		//黄牌尾牌强制  
		PLR_Config.sCharForce.ForceNum = 0;
		PLR_Config.sCharForce.CharBuff[0] = 'n';
		PLR_Config.sCharForce.CharBuff[1] = 'Q';

		MVPLR_AlgSetting(&PLR_Config,nCfgType);

			printf(" PLR_Config x=%d,y=%d,w=%d,h=%d\n",PLR_Config.sDetectRect.x,PLR_Config.sDetectRect.y,PLR_Config.sDetectRect.width,PLR_Config.sDetectRect.height);
		m_bInitCarNumLib = 1;
		#else
        if(m_bInitCarNumLib == 1)
        {
            mvcarnumdetect.carnum_quit();
            m_bInitCarNumLib = 0;
        }

        //不同的场景载入不同的boost
        char buf[64] = {'\0'};
		#ifdef ALGORITHM_DL
		sprintf(buf,"./BocomMv/%d",4);
		#else
		sprintf(buf,"./BocomMv/%d",1);
		#endif
        //printf("***********buf=%s\n",buf);

		LogNormal("%s",mvcarnumdetect.GetVersion());

        m_bInitCarNumLib = mvcarnumdetect.carnum_init(buf,homography_image_to_world,m_imgSnap->width,m_imgSnap->height-m_nExtentHeight);
		#ifdef ALGORITHM_DL
		m_bInitCarNumLib = 1;
		#endif

		//是否检测无牌车
		_ObjectPara ObjectPara;
        CXmlParaUtil xml;
        xml.LoadObjectParameter(m_nChannelId,ObjectPara);
		mvcarnumdetect.mvSetNonplateDetect(ObjectPara.bDetectNonPlate);

		//是否检测摩托车
		mvcarnumdetect.mvSetDoMotorCarCarnum(ObjectPara.bDoMotorCar);
		#endif

        m_bReloadROI = false;
    }
    ///////////////上一帧检测太慢，则不检测下一帧
    if(m_bInitCarNumLib != 1)
    {
		LogNormal("OnDetect InitCarNumLib error \n");
        return false;
    }

    BYTE* frame;
    yuv_video_buf* buf = (yuv_video_buf*)(result.pCurBuffer);


    //设置车牌检测参数
    carnum_parm_t cnp;
    CnpMap::iterator it = m_cnpMap.begin();
    if(it != m_cnpMap.end())
    {
        cnp = it->second;
    }
    cnp.isday = m_nDayNight;
	#ifndef ALGORITHM_YUV_CARNUM
    mvcarnumdetect.set_carnum_parm(&cnp);
	#endif

    //printf("=======cnp.isday=%d,cnp.direction=%d,cnp.img_angle = %d\n",cnp.isday,cnp.direction,cnp.img_angle);

    ////////////////////

    CvRect rect = m_rtCarnumROI;

    if(m_nDeinterlace == 2)
    {
                frame = result.pCurBuffer;
                buf = (yuv_video_buf*)(frame);
                if(m_nWordPos == 0)
                cvSetData(m_img1,frame+sizeof(yuv_video_buf),m_img1->widthStep);//第一帧
                else
                cvSetData(m_img1,frame+sizeof(yuv_video_buf)+m_img1->widthStep*m_nExtentHeight/m_nDeinterlace,m_img1->widthStep);//第一帧
                //printf("*********************************buf->nSeq=%d,buf->nFieldSeq=%d\r\n",buf->nSeq,buf->nFieldSeq);
                DetectCarNum(m_img1,rect,buf->uTimestamp,buf->ts,buf->nSeq,2);

#ifdef TEMPERATURE_TEST
                m_detectframe++;
#endif
           
    }
    else
    {
        frame = result.pCurBuffer;
        buf = (yuv_video_buf*)(frame);
#ifdef ALGORITHM_YUV_CARNUM
		if(m_nWordPos == 0)
	    DetectCarNumYUV(frame+sizeof(yuv_video_buf),buf->ts,buf->nSeq);
		else
		DetectCarNumYUV(frame+sizeof(yuv_video_buf)+m_img->width*2*m_nExtentHeight,buf->ts,buf->nSeq);
#else
        if(m_nWordPos == 0)
        cvSetData(m_img1,frame+sizeof(yuv_video_buf),m_img1->widthStep);//第一帧
        else
        cvSetData(m_img1,frame+sizeof(yuv_video_buf)+m_img1->widthStep*m_nExtentHeight,m_img1->widthStep);
        //printf("====void CRoadCarnumDetect::DealCarnumDetect() buf->nSeq=%d,buf->nFieldSeq=%d\r\n",buf->nSeq,buf->nFieldSeq);
        DetectCarNum(m_img1,rect,buf->uTimestamp,buf->ts,buf->nSeq,1);
#endif
#ifdef TEMPERATURE_TEST
        m_detectframe++;
#endif
    }
    return true;
}

//获取图像坐标到世界坐标的变换矩阵
void CRoadCarnumDetect::mvfind_homography(float *image, float *world, float homography_image_to_world[3][3])
{
    //only use the first 4 points to calculate homography
    CvMat src_points = cvMat(4,2,CV_32FC1,image);
    CvMat dst_points = cvMat(4,2,CV_32FC1,world);

    CvMat *homo = cvCreateMat(3,3,CV_32FC1);
    cvFindHomography(&src_points, &dst_points, homo);//src*H=dst

    int i,j;
    for(i=0; i<3; i++) //存到数组里，transform时更快
    {
        for(j=0; j<3; j++)
        {
            homography_image_to_world[i][j] = cvmGet(homo,i,j);
            //printf("%.2f ",homography_image_to_world[i][j]);
        }
        //printf("\n");
    }
    //printf("\n");
    cvReleaseMat(&homo);
}

//载入通道设置(0:目标检测读取配置;1:车牌检测读取配置)
bool CRoadCarnumDetect::LoadRoadSettingInfo(int nType,vector<mvvideostd>& vListStabBack,CvRect& farrect,CvRect& nearrect)
{
	//LogNormal("-in-LoadRoadSettingInfo--\n");
    LIST_CHANNEL_INFO list_channel_info;
    CXmlParaUtil xml;
    if(xml.LoadRoadSettingInfo(list_channel_info,m_nChannelId))
    {
        float image_cord[12];
        float world_cord[12];
        CvPoint pt,pt1,pt2;
        CvPoint2D32f ptImage;

        if(nType == 0)
        {
            m_vtsObjectRegion.clear();
            m_LoopInfo.clear();
            m_vecYellowLines.clear();
			m_vecWhiteLines.clear();
            m_vecBianDaoLines.clear();
        }
        else
        {
            m_cnpMap.clear();
        }
        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        bool bLoadCalibration = false;
        bool bLoadCardArea = false;
        bool bLoadSkipArea = false;
        bool bLoadViolationArea = false;
        bool bLoadEventArea = false;
        bool bLoadTrafficSignalArea = false;
        bool bLoadStopLine = false;
        bool bLoadStraightLine = false;
        bool bLoadTurnLeftLine = false;
        bool bLoadTurnRightLine = false;
        bool bLoadYellowLine = false;
		bool bLoadWhiteLine = false;
        bool bLoadRadar = false;
        bool bStabBackArea = false;
		bool bLoadPerson = false;
		bool bLoadYelGridArea = false;
		bool bLoadRoad = false;
		bool bLoadViolationFirstLine = false;
		bool bLoadViolationSecondLine = false;
		bool bLoadRightFirstLine = false;
		bool bLoadForeFirstLine = false;
		bool bLoadLeftFirstLine = false;	 


        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator it_begin;
        Point32fList::iterator it_end;

        Point32fList::iterator it_32fb;
        Point32fList::iterator it_32fe;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

		#ifdef ALGORITHM_YUV
		g_Setting.nChannels = list_channel_info.size();	
		#endif
		
		int k = 0;
        while(it_b != it_e)
        {
            int i = 0;
            int j = 0;
            CHANNEL_INFO channel_info = *it_b;
            ChannelRegion  vtsRegion;
            LoopInfo loopInfo;
            carnum_parm_t cnp;
            //车道编号
            vtsRegion.nRoadIndex = channel_info.chProp_index.value.nValue;
            loopInfo.nRoadIndex = vtsRegion.nRoadIndex;
            //车道逻辑编号
            vtsRegion.nVerRoadIndex = channel_info.chProp_name.value.nValue;
            //车道方向
            int nDirection = channel_info.chProp_direction.value.nValue;
            if(nDirection < 180)
            {
                vtsRegion.nDirection = 0;
                cnp.direction = 0;
            }
            else
            {
                vtsRegion.nDirection = 1;
                cnp.direction = 1;
            }
            vtsRegion.vDirection.start.x = channel_info.chProp_direction.ptBegin.x;
            vtsRegion.vDirection.start.y = channel_info.chProp_direction.ptBegin.y;
            vtsRegion.vDirection.end.x = channel_info.chProp_direction.ptEnd.x;
            vtsRegion.vDirection.end.y = channel_info.chProp_direction.ptEnd.y;
			
			#ifdef ALGORITHM_YUV
			m_nDetectDirection = vtsRegion.nDirection;
			#endif
			
            //标定区域
            if(!bLoadCalibration)
            {
                printf("channel_info.calibration.length=%f,channel_info.calibration.width=%f\n",channel_info.calibration.length,channel_info.calibration.width);

                //矩形区域（4个点）
                it_begin = channel_info.calibration.region.listPT.begin();
                it_end = channel_info.calibration.region.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    image_cord[2*i] = (it_begin->x)*m_ratio_x;
                    image_cord[2*i+1] = (it_begin->y)*m_ratio_y*m_nDeinterlace;

                    if(i==0)
                    {
                        world_cord[2*i] = 0;
                        world_cord[2*i+1] = 0;
                    }
                    else if(i==1)
                    {
                        world_cord[2*i] = channel_info.calibration.length;
                        world_cord[2*i+1] = 0;
                    }
                    else if(i==2)
                    {
                        world_cord[2*i] = channel_info.calibration.length;
                        world_cord[2*i+1] = channel_info.calibration.width;
                    }
                    else if(i==3)
                    {
                        world_cord[2*i] = 0;
                        world_cord[2*i+1] = channel_info.calibration.width;
                    }

					#ifdef ALGORITHM_YUV
					image_cord[2*i] /= m_fScaleX;
					image_cord[2*i+1] /= m_fScaleY;
					#endif
                    printf("image_cord[2*i]=%f,image_cord[2*i+1]=%f,world_cord[2*i]=%f,world_cord[2*i+1]=%f\n",image_cord[2*i],image_cord[2*i+1],world_cord[2*i],world_cord[2*i+1]);
                    i++;
                }
                //printf("channel_info.calibration.length=%f,channel_info.calibration.width=%f\n",channel_info.calibration.length,channel_info.calibration.width);
                //辅助标定点
                it_begin = channel_info.calibration.listPT.begin();
                it_end = channel_info.calibration.listPT.end();

                it_32fb = channel_info.calibration.list32fPT.begin();
                it_32fe = channel_info.calibration.list32fPT.end();
                while(it_begin!=it_end&&it_32fb!=it_32fe)
                {
                    //image cor
                    image_cord[2*i] = (it_begin->x)*m_ratio_x;
                    image_cord[2*i+1] = (it_begin->y)*m_ratio_y*m_nDeinterlace;
					


                    //world cor
                    world_cord[2*i] = it_32fb->x;
                    world_cord[2*i+1] = it_32fb->y;

                    printf("image_cord[2*i]=%f,image_cord[2*i+1]=%f,world_cord[2*i]=%f,world_cord[2*i+1]=%f\n",image_cord[2*i],image_cord[2*i+1],world_cord[2*i],world_cord[2*i+1]);
					#ifdef ALGORITHM_YUV
					image_cord[2*i] /= m_fScaleX;
					image_cord[2*i+1] /= m_fScaleY;
					#endif

                    it_32fb++;
                    it_begin++;
                    i++;
                }


                if(nType == 0)
                {
                    memcpy(m_image_cord,image_cord,sizeof(float)*12);
                    memcpy(m_world_cord,world_cord,sizeof(float)*12);

					#ifdef ALGORITHM_YUV
					MvDspMaxSize maxsize;
					maxsize.m_nWidth  = m_img->width/m_fScaleX;
					maxsize.m_nHeight = m_img->height/m_fScaleY;
					printf("maxsize.m_nWidth=%d,maxsize.m_nHeight=%d\n",maxsize.m_nWidth,maxsize.m_nHeight);
					
					for(int t = 0;t < 4;t++)
					{
						maxsize.m_image_coordinate[2*t] = image_cord[2*t];
						maxsize.m_image_coordinate[2*t+1] = image_cord[2*t+1];
						maxsize.m_world_coordinate[2*t] = world_cord[2*t];
						maxsize.m_world_coordinate[2*t+1] = world_cord[2*t+1];

						//printf("image_cord x=%f,y=%f\n",image_cord[2*t],image_cord[2*t+1]);
						//printf("world_cord x=%f,y=%f\n",world_cord[2*t],world_cord[2*t+1]);
						printf("maxsize.m_image_coordinate x=%f,y=%f\n",maxsize.m_image_coordinate[2*t],maxsize.m_image_coordinate[2*t+1]);
						printf("maxsize.m_world_coordinate x=%f,y=%f\n",maxsize.m_world_coordinate[2*t],maxsize.m_world_coordinate[2*t+1]);
					}

					maxsize.GetWorldCoordinateAndCarSize();

					cvSaveImage("MaxSizeX.jpg", maxsize.m_carwidth_image);
					cvSaveImage("MaxSizeY.jpg", maxsize.m_carheight_image);

					for(int row = 0; row < maxsize.m_nHeight; row++)
					{
							int col =  maxsize.m_nWidth/2;
					
							g_Setting.m_pMaxSizeX[row] = 
								((unsigned short*)(maxsize.m_carwidth_image->imageData+maxsize.m_carwidth_image->widthStep*row))[col];
							g_Setting.m_pMaxSizeY[row] = 
								((unsigned short*)(maxsize.m_carheight_image->imageData+maxsize.m_carheight_image->widthStep*row))[col];
							g_Setting.m_pWorldX[row] = 
								((double*)(maxsize.m_wldx_image->imageData+maxsize.m_wldx_image->widthStep*row))[col];
							g_Setting.m_pWorldY[row] = 
								((double*)(maxsize.m_wldy_image->imageData+maxsize.m_wldy_image->widthStep*row))[col];
					}

					for(int t = 0;t < 6;t++)
					{
						g_Setting.m_pXYImage[2*t] = image_cord[2*t]*m_fScaleX;
						g_Setting.m_pXYImage[2*t+1] = image_cord[2*t+1]*m_fScaleY;
						g_Setting.m_pXYWorld[2*t] = world_cord[2*t];
						g_Setting.m_pXYWorld[2*t+1] = world_cord[2*t+1];
					}
					#endif
                }
                else if(nType == 1)
                {
					printf("before mvfind_homography\n");
                    mvfind_homography(image_cord,world_cord,homography_image_to_world);//基于帧图象
					printf("after mvfind_homography\n");
                }

                bLoadCalibration = true;
            }
            //车牌区域
            if(!bLoadCardArea)
            {
                it_rb = channel_info.carnumRegion.listRegionProp.begin();
                it_re = channel_info.carnumRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                        pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                        if(i==0)
                        {
                            pt1.x = pt.x;
                            pt1.y = pt.y;
                            pt2.x = pt.x;
                            pt2.y = pt.y;
                        }
                        else
                        {
                            if(pt1.x>pt.x)
                            {
                                pt1.x=pt.x;
                            }
                            if(pt1.y>pt.y)
                            {
                                pt1.y=pt.y;
                            }
                            if(pt2.x<pt.x)
                            {
                                pt2.x=pt.x;
                            }
                            if(pt2.y<pt.y)
                            {
                                pt2.y=pt.y;
                            }
                        }
                        i++;
                    }

					if( (pt2.x > m_imgFrame->width) || (pt2.y > m_imgFrame->height))
                    {
                        return false;
                    }

                    if(nType == 0)
                    {
                        //设置VLP检测区域
                        m_rtVlpRoi.x = pt1.x;
                        m_rtVlpRoi.y = pt1.y;
                        m_rtVlpRoi.width = pt2.x - pt1.x;
                        m_rtVlpRoi.height = pt2.y - pt1.y;
						
						#ifdef ALGORITHM_YUV
						g_Setting.m_rtCarNum.x = m_rtVlpRoi.x;
						g_Setting.m_rtCarNum.y = m_rtVlpRoi.y;
						g_Setting.m_rtCarNum.width = m_rtVlpRoi.width;
						g_Setting.m_rtCarNum.height = m_rtVlpRoi.height;
						printf("m_rtCarNum.x=%d,m_rtCarNum.y=%d,m_rtCarNum.width=%d,m_rtCarNum.height=%d\n",g_Setting.m_rtCarNum.x,g_Setting.m_rtCarNum.y,g_Setting.m_rtCarNum.width,g_Setting.m_rtCarNum.height);
						#endif
                        //printf("m_rtVlpRoi.x=%d,m_rtVlpRoi.y=%d,m_rtVlpRoi.width=%d,m_rtVlpRoi.height=%d\n",m_rtVlpRoi.x,m_rtVlpRoi.y,m_rtVlpRoi.width,m_rtVlpRoi.height);
                    }
                    else if(nType == 1)
                    {
                        //设置车牌检测区域
                        m_rtCarnumROI.x = pt1.x;
                        m_rtCarnumROI.y = pt1.y;
                        m_rtCarnumROI.width = pt2.x - pt1.x;
                        m_rtCarnumROI.height = pt2.y - pt1.y;
                        //printf("m_rtCarnumROI.x=%d,m_rtCarnumROI.y=%d,m_rtCarnumROI.width=%d,m_rtCarnumROI.height=%d\n",m_rtCarnumROI.x,m_rtCarnumROI.y,m_rtCarnumROI.width,m_rtCarnumROI.height);
                    }


                }
                bLoadCardArea = true;
            }
            //违章检测区域
            if(!bLoadViolationArea)
            {
                    it_rb = channel_info.ViolationRegion.listRegionProp.begin();
                    it_re = channel_info.ViolationRegion.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
						#ifdef ALGORITHM_YUV
						g_Setting.vioRegion.nPoints = it_rb->listPt.size();
						printf("g_Setting.vioRegion.nPoints=%d\n",g_Setting.vioRegion.nPoints);
						#endif
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            if(i==0)
                            {
                                m_rtVtsRoi.x = item_b->x;
                                m_rtVtsRoi.y = item_b->y;
                            }
                            else if(i==2)
                            {
                                m_rtVtsRoi.width = item_b->x - m_rtVtsRoi.x;
                                m_rtVtsRoi.height = item_b->y - m_rtVtsRoi.y;
                            }
							#ifdef ALGORITHM_YUV
							g_Setting.vioRegion.arrList[i].x = item_b->x;
							g_Setting.vioRegion.arrList[i].y = item_b->y;
							printf("i=%d,item_b x=%f,y=%f\n",i,item_b->x,item_b->y);
							printf("i=%d,vioRegion x=%f,y=%f\n",i,g_Setting.vioRegion.arrList[i].x,g_Setting.vioRegion.arrList[i].y);
							#endif
                            i++;
                        }
                        //设置VLP检测区域
                        m_rtVtsRoi.x *= m_ratio_x;
                        m_rtVtsRoi.y *= m_ratio_y;
                        m_rtVtsRoi.width *= m_ratio_x;
                        m_rtVtsRoi.height *= m_ratio_y;
						
                        //printf("1111m_rtVtsRoi.x=%d,m_rtVtsRoi.y=%d,m_rtVtsRoi.width=%d,m_rtVtsRoi.height=%d\n",m_rtVtsRoi.x,m_rtVtsRoi.y,m_rtVtsRoi.width,m_rtVtsRoi.height);
                    }
                    bLoadViolationArea = true;
            }
            //printf("vtsRegion.nRoadIndex=%d,vtsRegion.nVerRoadIndex=%d,vtsRegion.nDirection=%d\n",vtsRegion.nRoadIndex,vtsRegion.nVerRoadIndex,vtsRegion.nDirection);
			if(!bLoadPerson)
            {
                //远处行人框
                it_rb = channel_info.RemotePersonRegion.listRegionProp.begin();
                it_re = channel_info.RemotePersonRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        if(i==0)
                        {
                            farrect.x = (item_b->x);
                            farrect.y = item_b->y;
                        }
                        else if(i == 2)
                        {
                            farrect.width = item_b->x - farrect.x;
                            farrect.height = item_b->y - farrect.y;
                        }

                        i++;
                    }
                }
                //近处行人框
                it_rb = channel_info.LocalPersonRegion.listRegionProp.begin();
                it_re = channel_info.LocalPersonRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    i = 0;
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    for( ; item_b!=item_e; item_b++)
                    {
                         if(i==0)
                        {
                            nearrect.x = (item_b->x);
                            nearrect.y = item_b->y;
                        }
                        else if(i == 2)
                        {
                            nearrect.width = item_b->x - nearrect.x;
                            nearrect.height = item_b->y - nearrect.y;
                        }

                        i++;

                    }
                }
				
				#ifdef ALGORITHM_YUV
				if(nType == 0)
				{
					g_Setting.m_rtRemoteCarNum.x = farrect.x;
					g_Setting.m_rtRemoteCarNum.y = farrect.y;
					g_Setting.m_rtRemoteCarNum.width = farrect.width;
					g_Setting.m_rtRemoteCarNum.height = farrect.height;
					printf("m_rtRemoteCarNum x=%d,y=%d,width=%d,height=%d\n",g_Setting.m_rtRemoteCarNum.x,g_Setting.m_rtRemoteCarNum.y,g_Setting.m_rtRemoteCarNum.width,g_Setting.m_rtRemoteCarNum.height);

					g_Setting.m_rtLocalCarNum.x = nearrect.x;
					g_Setting.m_rtLocalCarNum.y = nearrect.y;
					g_Setting.m_rtLocalCarNum.width = nearrect.width;
					g_Setting.m_rtLocalCarNum.height = nearrect.height;
					printf("m_rtLocalCarNum x=%d,y=%d,width=%d,height=%d\n",g_Setting.m_rtLocalCarNum.x,g_Setting.m_rtLocalCarNum.y,g_Setting.m_rtLocalCarNum.width,g_Setting.m_rtLocalCarNum.height);
				}
				#endif

                bLoadPerson = true;
            }


            //目标检测所需参数
            if(nType == 0)
            {

				#ifdef ALGORITHM_YUV
				g_Setting.ChnlRegion[k].nRoadIndex = vtsRegion.nRoadIndex;
				g_Setting.ChnlRegion[k].nVerRoadIndex = vtsRegion.nVerRoadIndex;
				g_Setting.ChnlRegion[k].nDirection = nDirection;
				g_Setting.ChnlRegion[k].vDirection.start = vtsRegion.vDirection.start;
				g_Setting.ChnlRegion[k].vDirection.end = vtsRegion.vDirection.end;

				//道路区域(目前只支持一个道路)
				if(!bLoadRoad)
				{
					i = 0;
					g_Setting.m_nRoadCount = 1;
					g_Setting.m_pRoadRegion[0].direct = nDirection;
					g_Setting.m_pRoadRegion[0].nPoints = channel_info.roadRegion.listPT.size();
					it_begin = channel_info.roadRegion.listPT.begin();
					it_end = channel_info.roadRegion.listPT.end();
					printf("channel_info.roadRegion.listPT.size=%d\n",channel_info.roadRegion.listPT.size());
					for(; it_begin != it_end; it_begin++)
					{
						g_Setting.m_pRoadRegion[0].arrList[i].x = it_begin->x;
						g_Setting.m_pRoadRegion[0].arrList[i].y = it_begin->y;
						printf("i=%d,it_begin x=%f,y=%f\n",i,it_begin->x,it_begin->y);
						printf("i=%d,g_Setting.m_pRoadRegion x=%f,y=%f\n",i,g_Setting.m_pRoadRegion[0].arrList[i].x,g_Setting.m_pRoadRegion[0].arrList[i].y);
						i++;
					}
					bLoadRoad =true;
				}
				#endif

                //车道区域
                it_begin = channel_info.chRegion.listPT.begin();
                it_end = channel_info.chRegion.listPT.end();
				#ifdef ALGORITHM_YUV
				g_Setting.ChnlRegion[k].nChannlePointNumber = channel_info.chRegion.listPT.size();
				#endif
				j = 0;
                for(; it_begin != it_end; it_begin++)
                {
                    CvPoint   ptChanRgn;
                    ptChanRgn.x = it_begin->x*m_ratio_x;
                    ptChanRgn.y = it_begin->y*m_ratio_y;
					#ifndef ALGORITHM_YUV
                    vtsRegion.vListChannel.push_back(ptChanRgn);
					#else
					g_Setting.ChnlRegion[k].arrListChannel[j].x = it_begin->x;
					g_Setting.ChnlRegion[k].arrListChannel[j].y = it_begin->y;

					printf("k=%d,j=%d,it_begin x=%f,y=%f\n",k,j,it_begin->x,it_begin->y);
					printf("k=%d,j=%d,g_Setting.ChnlRegion x=%f,y=%f\n",k,j,g_Setting.ChnlRegion[k].arrListChannel[j].x,g_Setting.ChnlRegion[k].arrListChannel[j].y);
						
					#endif
                    //printf("ptChanRgn.x=%d,ptChanRgn.y=%d,m_imgFrame->width*m_ratio_x=%d\n",ptChanRgn.x,ptChanRgn.y,m_imgFrame->width*m_ratio_x);

					if( (ptChanRgn.x > m_imgFrame->width*m_ratio_x) || (ptChanRgn.y > m_imgFrame->height*m_ratio_y))
					{
						return false;
					}
					j++;
                }
                //屏蔽区域
                if(!bLoadSkipArea)
                {
					j = 0;
					#ifndef ALGORITHM_YUV
                    list<MvPolygon> rdlines;
					#endif
                    it_rb = channel_info.eliminateRegion.listRegionProp.begin();
                    it_re = channel_info.eliminateRegion.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
						#ifdef ALGORITHM_YUV
						g_Setting.m_pMaskRegion[j].nPoints = it_rb->listPt.size();
						#endif
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        //printf("it_rb->size()=%d,it_rb->listPt.size()=%d\n",channel_info.eliminateRegion.listRegionProp.size(),it_rb->listPt.size());
                        CvPoint* pts = new CvPoint[it_rb->listPt.size()+1];
                        i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pts[i].x = item_b->x*m_ratio_x;
                            pts[i].y = item_b->y*m_ratio_y;
                            //printf("pts[i].x=%d,pts[i].y=%d\n",pts[i].x,pts[i].y);
							#ifdef ALGORITHM_YUV
							g_Setting.m_pMaskRegion[j].arrList[i].x = pts[i].x;
							g_Setting.m_pMaskRegion[j].arrList[i].y = pts[i].y;
							#endif
                            i++;
                        }
                        pts[i].x = pts[0].x;
                        pts[i].y = pts[0].y;

						
						#ifndef ALGORITHM_YUV
                        MvPolygon poly(pts, it_rb->listPt.size());
                        rdlines.push_back(poly);
						#endif
                        delete []pts;
						j++;
                    }
                    
					#ifdef ALGORITHM_YUV
					g_Setting.m_nMaskRegionCount = channel_info.eliminateRegion.listRegionProp.size();
					#else
					//设置屏蔽区域
                    m_ftVlp.SetInvalidRgns(rdlines);
					#endif

					bLoadSkipArea = true;
                }

                //稳像背景区域
                if(!bStabBackArea)
                {
                    it_rb = channel_info.StabBackRegion.listRegionProp.begin();
                    it_re = channel_info.StabBackRegion.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        mvvideostd listStabBack;
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();

                        for( ; item_b!=item_e; item_b++)
                        {
                            ptImage.x = (item_b->x*m_ratio_x);
                            ptImage.y = (item_b->y*m_ratio_y);

                            listStabBack.vPList.push_back(ptImage);
                        }
                        vListStabBack.push_back(listStabBack);
                    }
                    bStabBackArea = true;
                }

                //停止线
                if(!bLoadStopLine)
                {
                    it_rb = channel_info.StopLine.listRegionProp.begin();
                    it_re = channel_info.StopLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                                pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                                if(i==0)
                                {
                                    m_lineStop.start.x = pt.x;
									m_lineStop.start.y = pt.y;
                                }
                                else
                                {
                                    m_lineStop.end.x = pt.x;
									m_lineStop.end.y = pt.y;
                                }
                                i++;
                            }
                            //printf("m_lineStop.start.x=%d,m_lineStop.start.y=%d,m_lineStop.end.x=%d,m_lineStop.end.y=%d\n",m_lineStop.start.x,m_lineStop.start.y,m_lineStop.end.x,m_lineStop.end.y);
                        }
                    }
                    bLoadStopLine = true;
					
					#ifdef ALGORITHM_YUV
					g_Setting.gLines.m_stopLine.start = m_lineStop.start;
					g_Setting.gLines.m_stopLine.end = m_lineStop.end;
					#endif
                }

                //直行线
                if(!bLoadStraightLine)
                {
                    it_rb = channel_info.StraightLine.listRegionProp.begin();
                    it_re = channel_info.StraightLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                                pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                                if(i==0)
                                {
                                    m_lineStraight.start.x = pt.x;
									m_lineStraight.start.y = pt.y;
                                }
                                else
                                {
                                    m_lineStraight.end.x = pt.x;
									m_lineStraight.end.y = pt.y;
                                }
                                i++;
                            }
                            //printf("m_lineStraight.start.x=%d,m_lineStraight.start.y=%d,m_lineStraight.end.x=%d,m_lineStraight.end.y=%d\n",m_lineStraight.start.x,m_lineStraight.start.y,m_lineStraight.end.x,m_lineStraight.end.y);
                        }
                    }
                    bLoadStraightLine = true;
					#ifdef ALGORITHM_YUV
					g_Setting.gLines.m_foreLine.start = m_lineStraight.start;
					g_Setting.gLines.m_foreLine.end = m_lineStraight.end;
					#endif
                }

                //左转线
                if(!bLoadTurnLeftLine)
                {
                    it_rb = channel_info.TurnLeftLine.listRegionProp.begin();
                    it_re = channel_info.TurnLeftLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                                pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                                if(i==0)
                                {
                                    m_lineTurnLeft.start.x = pt.x;
									m_lineTurnLeft.start.y = pt.y;
                                }
                                else
                                {
                                    m_lineTurnLeft.end.x = pt.x;
									m_lineTurnLeft.end.y = pt.y;
                                }
                                i++;
                            }
                            //printf("m_lineTurnLeft.start.x=%d,m_lineTurnLeft.start.y=%d,m_lineTurnLeft.end.x=%d,m_lineTurnLeft.end.y=%d\n",m_lineTurnLeft.start.x,m_lineTurnLeft.start.y,m_lineTurnLeft.end.x,m_lineTurnLeft.end.y);
                        }
                    }
                    bLoadTurnLeftLine = true;
					#ifdef ALGORITHM_YUV
					g_Setting.gLines.m_leftLine.start = m_lineTurnLeft.start;
					g_Setting.gLines.m_leftLine.end = m_lineTurnLeft.end;
					#endif
                }

                //右转线
                if(!bLoadTurnRightLine)
                {
                    it_rb = channel_info.TurnRightLine.listRegionProp.begin();
                    it_re = channel_info.TurnRightLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                                pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                                if(i==0)
                                {
                                    m_lineTurnRight.start.x = pt.x;
									m_lineTurnRight.start.y = pt.y;
                                }
                                else
                                {
                                    m_lineTurnRight.end.x = pt.x;
									m_lineTurnRight.end.y = pt.y;
                                }
                                i++;
                            }
                            //printf("m_lineTurnRight.start.x=%d,m_lineTurnRight.start.y=%d,m_lineTurnRight.end.x=%d,m_lineTurnRight.end.y=%d\n",m_lineTurnRight.start.x,m_lineTurnRight.start.y,m_lineTurnRight.end.x,m_lineTurnRight.end.y);
                        }
                    }
                    bLoadTurnRightLine = true;
					#ifdef ALGORITHM_YUV
					g_Setting.gLines.m_rightLine.start = m_lineTurnRight.start;
					g_Setting.gLines.m_rightLine.end = m_lineTurnRight.end;
					#endif
                }
					
				#ifdef ALGORITHM_YUV
				//电警第一触发线
                if(!bLoadViolationFirstLine)
                {
                    it_rb = channel_info.ViolationFirstLine.listRegionProp.begin();
                    it_re = channel_info.ViolationFirstLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    g_Setting.gLines.m_firstLine.start.x = item_b->x;
									g_Setting.gLines.m_firstLine.start.y = item_b->y;
                                }
                                else
                                {
                                    g_Setting.gLines.m_firstLine.end.x = item_b->x;
									g_Setting.gLines.m_firstLine.end.y = item_b->y;
                                }
                                i++;
                            }
                         }
                    }
                    bLoadViolationFirstLine = true;
                }
				//电警第二触发线
                if(!bLoadViolationSecondLine)
                {
                    it_rb = channel_info.ViolationSecondLine.listRegionProp.begin();
                    it_re = channel_info.ViolationSecondLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    g_Setting.gLines.m_secondLine.start.x = item_b->x;
									g_Setting.gLines.m_secondLine.start.y = item_b->y;
                                }
                                else
                                {
                                    g_Setting.gLines.m_secondLine.end.x = item_b->x;
									g_Setting.gLines.m_secondLine.end.y = item_b->y;
                                }
                                i++;
                            }
                         }
                    }
                    bLoadViolationSecondLine = true;
                }
				//禁右初始触发线
                if(!bLoadRightFirstLine)
                {
                    it_rb = channel_info.RightFirstLine.listRegionProp.begin();
                    it_re = channel_info.RightFirstLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    g_Setting.gLines.m_rightLineOri.start.x = item_b->x;
									g_Setting.gLines.m_rightLineOri.start.y = item_b->y;
                                }
                                else
                                {
                                    g_Setting.gLines.m_rightLineOri.end.x = item_b->x;
									g_Setting.gLines.m_rightLineOri.end.y = item_b->y;
                                }
                                i++;
                            }
                         }
                    }
                    bLoadRightFirstLine = true;
                }
				//禁左初始触发线
                if(!bLoadLeftFirstLine)
                {
                    it_rb = channel_info.LeftFirstLine.listRegionProp.begin();
                    it_re = channel_info.LeftFirstLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    g_Setting.gLines.m_leftLineOri.start.x = item_b->x;
									g_Setting.gLines.m_leftLineOri.start.y = item_b->y;
                                }
                                else
                                {
                                    g_Setting.gLines.m_leftLineOri.end.x = item_b->x;
									g_Setting.gLines.m_leftLineOri.end.y = item_b->y;
                                }
                                i++;
                            }
                         }
                    }
                    bLoadLeftFirstLine = true;
                }
				//禁前初始触发线
                if(!bLoadForeFirstLine)
                {
                    it_rb = channel_info.ForeFirstLine.listRegionProp.begin();
                    it_re = channel_info.ForeFirstLine.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    g_Setting.gLines.m_foreLineOri.start.x = item_b->x;
									g_Setting.gLines.m_foreLineOri.start.y = item_b->y;
                                }
                                else
                                {
                                    g_Setting.gLines.m_foreLineOri.end.x = item_b->x;
									g_Setting.gLines.m_foreLineOri.end.y = item_b->y;
                                }
                                i++;
                            }
                         }
                    }
                    bLoadForeFirstLine = true;
                }
				#endif

                //黄线
                if(!bLoadYellowLine)
                {
                    it_rb = channel_info.YellowLine.listRegionProp.begin();
                    it_re = channel_info.YellowLine.listRegionProp.end();
					#ifdef ALGORITHM_YUV
					g_Setting.gLines.m_vecYellowNumber = channel_info.YellowLine.listRegionProp.size();
					#endif
					j = 0;
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            MvLine YellowLine;
                            for( ; item_b!=item_e; item_b++)
                            {
                                pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                                pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                                if(i==0)
                                {
                                    YellowLine.start.x = pt.x;
									YellowLine.start.y = pt.y;
									#ifdef ALGORITHM_YUV
									g_Setting.gLines.m_vecYellowLine[j].start = YellowLine.start;
									#endif
                                }
                                else
                                {
                                    YellowLine.end.x = pt.x;
									YellowLine.end.y = pt.y;
									#ifdef ALGORITHM_YUV
									g_Setting.gLines.m_vecYellowLine[j].end = YellowLine.end;
									#endif
                                }
                                i++;
                            }
                            m_vecYellowLines.push_back(YellowLine);
							j++;
                         }
                    }
                    bLoadYellowLine = true;
                }

				//白线
				if(!bLoadWhiteLine)
				{
					it_rb = channel_info.WhiteLine.listRegionProp.begin();
					it_re = channel_info.WhiteLine.listRegionProp.end();
					#ifdef ALGORITHM_YUV
					g_Setting.gLines.m_vecWhiteNumber = channel_info.WhiteLine.listRegionProp.size();
					#endif
					j = 0;
					for(; it_rb != it_re; it_rb++)
					{
						item_b = it_rb->listPt.begin();
						item_e = it_rb->listPt.end();
						i = 0;
						if(it_rb->listPt.size()>1)
						{
							MvLine WhiteLine;
							for( ; item_b!=item_e; item_b++)
							{
								pt.x = (int) (item_b->x*m_ratio_x + 0.5);
								pt.y = (int) (item_b->y*m_ratio_y + 0.5);
								if(i==0)
								{
									WhiteLine.start.x = pt.x;
									WhiteLine.start.y = pt.y;
									#ifdef ALGORITHM_YUV
									g_Setting.gLines.m_vecWhiteLine[j].start = WhiteLine.start;
									#endif
								}
								else
								{
									WhiteLine.end.x = pt.x;
									WhiteLine.end.y = pt.y;
									#ifdef ALGORITHM_YUV
									g_Setting.gLines.m_vecWhiteLine[j].end = WhiteLine.end;
									#endif
								}
								i++;
							}
							m_vecWhiteLines.push_back(WhiteLine);
							j++;
						}
					}
					bLoadWhiteLine = true;
				}
                //变道线(可能分布于多个车道中)
                it_rb = channel_info.TurnRoadLine.listRegionProp.begin();
                it_re = channel_info.TurnRoadLine.listRegionProp.end();
				#ifdef ALGORITHM_YUV
				g_Setting.gLines.m_vecBianDaoXianNumber = channel_info.TurnRoadLine.listRegionProp.size();
				#endif
				j = 0;
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        MvLine TurnRoadLine;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                            if(i==0)
                            {
                                TurnRoadLine.start.x = pt.x;
								TurnRoadLine.start.y = pt.y;
								#ifdef ALGORITHM_YUV
								g_Setting.gLines.m_vecBianDaoXian[j].start = TurnRoadLine.start;
								#endif
                            }
                            else
                            {
                                TurnRoadLine.end.x = pt.x;
								TurnRoadLine.end.y = pt.y;
								#ifdef ALGORITHM_YUV
								g_Setting.gLines.m_vecBianDaoXian[j].end = TurnRoadLine.end;
								#endif
                            }
                            i++;
                        }
                        m_vecBianDaoLines.push_back(TurnRoadLine);
						j++;
                    }
                }


                 //单车道停止线
                it_rb = channel_info.LineStop.listRegionProp.begin();
                it_re = channel_info.LineStop.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        MvLine StopLine;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                            if(i==0)
                            {
                                StopLine.start.x = pt.x;
								StopLine.start.y = pt.y;
                            }
                            else
                            {
                                StopLine.end.x = pt.x;
								StopLine.end.y = pt.y;
                            }
                            i++;
                        }
                        vtsRegion.vStopLine = StopLine;
						#ifdef ALGORITHM_YUV
						g_Setting.ChnlRegion[k].vStopLine = StopLine;
						#endif
                    }
                }

                //单车道前行线
                it_rb = channel_info.LineStraight.listRegionProp.begin();
                it_re = channel_info.LineStraight.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        MvLine StraightLine;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                            if(i==0)
                            {
                                StraightLine.start.x = pt.x;
								StraightLine.start.y = pt.y;
                            }
                            else
                            {
                                StraightLine.end.x = pt.x;
								StraightLine.end.y = pt.y;
                            }
                            i++;
                        }
                        vtsRegion.vForeLine = StraightLine;
						#ifdef ALGORITHM_YUV
						g_Setting.ChnlRegion[k].vStopLine = StraightLine;
						#endif
                    }
                }

                //红灯区域
                it_rb = channel_info.RedLightRegion.listRegionProp.begin();
                it_re = channel_info.RedLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect RedLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    RedLightRegion.x = item_b->x;
                                    RedLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    RedLightRegion.width = item_b->x - RedLightRegion.x;
                                    RedLightRegion.height = item_b->y - RedLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                            vtsRegion.OnOffRed.x = RedLightRegion.x;
							vtsRegion.OnOffRed.y = RedLightRegion.y;
							vtsRegion.OnOffRed.width = RedLightRegion.width;
							vtsRegion.OnOffRed.height = RedLightRegion.height;
							 #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].OnOffRed = vtsRegion.OnOffRed;
							#endif
                        }
                }

                //绿灯区域
                it_rb = channel_info.GreenLightRegion.listRegionProp.begin();
                it_re = channel_info.GreenLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect GreenLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    GreenLightRegion.x = item_b->x;
                                    GreenLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    GreenLightRegion.width = item_b->x - GreenLightRegion.x;
                                    GreenLightRegion.height = item_b->y - GreenLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
						   vtsRegion.OnOffGreen.x = GreenLightRegion.x;
						   vtsRegion.OnOffGreen.y = GreenLightRegion.y;
						   vtsRegion.OnOffGreen.width = GreenLightRegion.width;
						   vtsRegion.OnOffGreen.height = GreenLightRegion.height;
						    #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].OnOffGreen = vtsRegion.OnOffGreen;
							#endif
                        }
                }

                //左转灯区域
                it_rb = channel_info.LeftLightRegion.listRegionProp.begin();
                it_re = channel_info.LeftLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect LeftLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    LeftLightRegion.x = item_b->x;
                                    LeftLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    LeftLightRegion.width = item_b->x - LeftLightRegion.x;
                                    LeftLightRegion.height = item_b->y - LeftLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiLeftLight.x = LeftLightRegion.x;
						   vtsRegion.roiLeftLight.y = LeftLightRegion.y;
						   vtsRegion.roiLeftLight.width = LeftLightRegion.width;
						   vtsRegion.roiLeftLight.height = LeftLightRegion.height;
						     #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiLeftLight = vtsRegion.roiLeftLight;
							#endif
                        }
                }

                //右转灯区域
                it_rb = channel_info.RightLightRegion.listRegionProp.begin();
                it_re = channel_info.RightLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect RightLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    RightLightRegion.x = item_b->x;
                                    RightLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    RightLightRegion.width = item_b->x - RightLightRegion.x;
                                    RightLightRegion.height = item_b->y - RightLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiRightLight.x = RightLightRegion.x;
						   vtsRegion.roiRightLight.y = RightLightRegion.y;
						   vtsRegion.roiRightLight.width = RightLightRegion.width;
						   vtsRegion.roiRightLight.height = RightLightRegion.height;
						     #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiRightLight = vtsRegion.roiRightLight;
							#endif

                        }
                }

                //直行灯区域
                it_rb = channel_info.StraightLightRegion.listRegionProp.begin();
                it_re = channel_info.StraightLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect StraightLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;

                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    StraightLightRegion.x = item_b->x;
                                    StraightLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    StraightLightRegion.width = item_b->x - StraightLightRegion.x;
                                    StraightLightRegion.height = item_b->y - StraightLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiMidLight.x = StraightLightRegion.x;
						   vtsRegion.roiMidLight.y = StraightLightRegion.y;
						   vtsRegion.roiMidLight.width = StraightLightRegion.width;
						   vtsRegion.roiMidLight.height = StraightLightRegion.height;
						   #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiMidLight = vtsRegion.roiMidLight;
							#endif

                        }
                }

                //禁止转向灯区域
                it_rb = channel_info.TurnAroundLightRegion.listRegionProp.begin();
                it_re = channel_info.TurnAroundLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect TurnAroundLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    TurnAroundLightRegion.x = item_b->x;
                                    TurnAroundLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    TurnAroundLightRegion.width = item_b->x - TurnAroundLightRegion.x;
                                    TurnAroundLightRegion.height = item_b->y - TurnAroundLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiTurnAroundLight.x = TurnAroundLightRegion.x;
						   vtsRegion.roiTurnAroundLight.y = TurnAroundLightRegion.y;
						   vtsRegion.roiTurnAroundLight.width = TurnAroundLightRegion.width;
						   vtsRegion.roiTurnAroundLight.height = TurnAroundLightRegion.height;
							#ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiTurnAroundLight = vtsRegion.roiTurnAroundLight;
							#endif
                        }
                }

                //左转红灯区域
                it_rb = channel_info.LeftRedLightRegion.listRegionProp.begin();
                it_re = channel_info.LeftRedLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect LeftRedLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    LeftRedLightRegion.x = item_b->x;
                                    LeftRedLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    LeftRedLightRegion.width = item_b->x - LeftRedLightRegion.x;
                                    LeftRedLightRegion.height = item_b->y - LeftRedLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiLeftLight_red.x = LeftRedLightRegion.x;
						   vtsRegion.roiLeftLight_red.y = LeftRedLightRegion.y;
						   vtsRegion.roiLeftLight_red.width = LeftRedLightRegion.width;
						   vtsRegion.roiLeftLight_red.height = LeftRedLightRegion.height;
						    #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiLeftLight_red = vtsRegion.roiLeftLight_red;
							#endif
                        }
                }

                //左转绿灯区域
                it_rb = channel_info.LeftGreenLightRegion.listRegionProp.begin();
                it_re = channel_info.LeftGreenLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect LeftGreenLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    LeftGreenLightRegion.x = item_b->x;
                                    LeftGreenLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    LeftGreenLightRegion.width = item_b->x - LeftGreenLightRegion.x;
                                    LeftGreenLightRegion.height = item_b->y - LeftGreenLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiLeftLight_green.x = LeftGreenLightRegion.x;
						   vtsRegion.roiLeftLight_green.y = LeftGreenLightRegion.y;
						   vtsRegion.roiLeftLight_green.width = LeftGreenLightRegion.width;
						   vtsRegion.roiLeftLight_green.height = LeftGreenLightRegion.height;
						    #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiLeftLight_green = vtsRegion.roiLeftLight_green;
							#endif
                        }
                }

                //右转红灯区域
                it_rb = channel_info.RightRedLightRegion.listRegionProp.begin();
                it_re = channel_info.RightRedLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect RightRedLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    RightRedLightRegion.x = item_b->x;
                                    RightRedLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    RightRedLightRegion.width = item_b->x - RightRedLightRegion.x;
                                    RightRedLightRegion.height = item_b->y - RightRedLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiRightLight_red.x = RightRedLightRegion.x;
						   vtsRegion.roiRightLight_red.y = RightRedLightRegion.y;
						   vtsRegion.roiRightLight_red.width = RightRedLightRegion.width;
						   vtsRegion.roiRightLight_red.height = RightRedLightRegion.height;
						     #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiRightLight_red = vtsRegion.roiRightLight_red;
							#endif
                        }
                }

                //右转绿灯区域
                it_rb = channel_info.RightGreenLightRegion.listRegionProp.begin();
                it_re = channel_info.RightGreenLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect RightGreenLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;

                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    RightGreenLightRegion.x = item_b->x;
                                    RightGreenLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    RightGreenLightRegion.width = item_b->x - RightGreenLightRegion.x;
                                    RightGreenLightRegion.height = item_b->y - RightGreenLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiRightLight_green.x = RightGreenLightRegion.x;
						   vtsRegion.roiRightLight_green.y = RightGreenLightRegion.y;
						   vtsRegion.roiRightLight_green.width = RightGreenLightRegion.width;
						   vtsRegion.roiRightLight_green.height = RightGreenLightRegion.height;
						     #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiRightLight_green = vtsRegion.roiRightLight_green;
							#endif
                        }
                }

                //直行红灯区域
                it_rb = channel_info.StraightRedLightRegion.listRegionProp.begin();
                it_re = channel_info.StraightRedLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect StraightRedLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    StraightRedLightRegion.x = item_b->x;
                                    StraightRedLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    StraightRedLightRegion.width = item_b->x - StraightRedLightRegion.x;
                                    StraightRedLightRegion.height = item_b->y - StraightRedLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiMidLight_red.x = StraightRedLightRegion.x;
						   vtsRegion.roiMidLight_red.y = StraightRedLightRegion.y;
						   vtsRegion.roiMidLight_red.width = StraightRedLightRegion.width;
						   vtsRegion.roiMidLight_red.height = StraightRedLightRegion.height;
						    #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiMidLight_red = vtsRegion.roiMidLight_red;
							#endif
                        }
                }

                //直行绿灯区域
                it_rb = channel_info.StraightGreenLightRegion.listRegionProp.begin();
                it_re = channel_info.StraightGreenLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect StraightGreenLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    StraightGreenLightRegion.x = item_b->x;
                                    StraightGreenLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    StraightGreenLightRegion.width = item_b->x - StraightGreenLightRegion.x;
                                    StraightGreenLightRegion.height = item_b->y - StraightGreenLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiMidLight_green.x = StraightGreenLightRegion.x;
						   vtsRegion.roiMidLight_green.y = StraightGreenLightRegion.y;
						   vtsRegion.roiMidLight_green.width = StraightGreenLightRegion.width;
						   vtsRegion.roiMidLight_green.height = StraightGreenLightRegion.height;
						    #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiMidLight_green = vtsRegion.roiMidLight_green;
							#endif
                        }
                }

                //禁止转向红灯区域
                it_rb = channel_info.TurnAroundRedLightRegion.listRegionProp.begin();
                it_re = channel_info.TurnAroundRedLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect TurnAroundRedLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    TurnAroundRedLightRegion.x = item_b->x;
                                    TurnAroundRedLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    TurnAroundRedLightRegion.width = item_b->x - TurnAroundRedLightRegion.x;
                                    TurnAroundRedLightRegion.height = item_b->y - TurnAroundRedLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
						   vtsRegion.roiTurnAroundLight_red.x = TurnAroundRedLightRegion.x;
						   vtsRegion.roiTurnAroundLight_red.y = TurnAroundRedLightRegion.y;
						   vtsRegion.roiTurnAroundLight_red.width = TurnAroundRedLightRegion.width;
						   vtsRegion.roiTurnAroundLight_red.height = TurnAroundRedLightRegion.height;
						    #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiTurnAroundLight_red = vtsRegion.roiTurnAroundLight_red;
							#endif

                        }
                }

                //禁止转向绿灯区域
                it_rb = channel_info.TurnAroundGreenLightRegion.listRegionProp.begin();
                it_re = channel_info.TurnAroundGreenLightRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect TurnAroundGreenLightRegion;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    TurnAroundGreenLightRegion.x = item_b->x;
                                    TurnAroundGreenLightRegion.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    TurnAroundGreenLightRegion.width = item_b->x - TurnAroundGreenLightRegion.x;
                                    TurnAroundGreenLightRegion.height = item_b->y - TurnAroundGreenLightRegion.y;
                                }
                                i++;
                            }
                            //设置红灯检测区域
                           vtsRegion.roiTurnAroundLight_green.x = TurnAroundGreenLightRegion.x;
						   vtsRegion.roiTurnAroundLight_green.y = TurnAroundGreenLightRegion.y;
						   vtsRegion.roiTurnAroundLight_green.width = TurnAroundGreenLightRegion.width;
						   vtsRegion.roiTurnAroundLight_green.height = TurnAroundGreenLightRegion.height;

						    #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].roiTurnAroundLight_green = vtsRegion.roiTurnAroundLight_green;
							#endif

                        }
                }

				//取图区域
                it_rb = channel_info.GetPhotoRegion.listRegionProp.begin();
                it_re = channel_info.GetPhotoRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                        CvRect rectMedianPos;

                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        if(it_rb->listPt.size()>1)
                        {
                            for( ; item_b!=item_e; item_b++)
                            {
                                if(i==0)
                                {
                                    rectMedianPos.x = item_b->x;
                                    rectMedianPos.y = item_b->y;
                                }
                                else if(i==2)
                                {
                                    rectMedianPos.width = item_b->x - rectMedianPos.x;
                                    rectMedianPos.height = item_b->y - rectMedianPos.y;
                                }
                                i++;
                            }
                            //设置取图区域
						   vtsRegion.rectMedianPos.x = rectMedianPos.x;
						   vtsRegion.rectMedianPos.y = rectMedianPos.y;
						   vtsRegion.rectMedianPos.width = rectMedianPos.width;
						   vtsRegion.rectMedianPos.height = rectMedianPos.height;

						   #ifdef ALGORITHM_YUV
							g_Setting.ChnlRegion[k].rectMedianPos = vtsRegion.rectMedianPos;
							#endif
                        }
                }

				//待转区第一前行线
				it_rb = channel_info.HoldForeLineFirst.listRegionProp.begin();
                it_re = channel_info.HoldForeLineFirst.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        MvLine StraightLine;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                            if(i==0)
                            {
                                StraightLine.start.x = pt.x;
								StraightLine.start.y = pt.y;
                            }
                            else
                            {
                                StraightLine.end.x = pt.x;
								StraightLine.end.y = pt.y;
                            }
                            i++;
                        }
                        vtsRegion.vHoldForeLineFirst = StraightLine;
						#ifdef ALGORITHM_YUV
						g_Setting.ChnlRegion[k].vHoldForeLineFirst = StraightLine;
						#endif
                    }
                }

				//待转区第二前行线
				it_rb = channel_info.HoldForeLineSecond.listRegionProp.begin();
                it_re = channel_info.HoldForeLineSecond.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        MvLine StraightLine;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                            if(i==0)
                            {
								StraightLine.start.x = pt.x;
								StraightLine.start.y = pt.y;
                            }
                            else
                            {
                               StraightLine.end.x = pt.x;
								StraightLine.end.y = pt.y;
                            }
                            i++;
                        }
                        vtsRegion.vHoldForeLineSecond = StraightLine;
						#ifdef ALGORITHM_YUV
						g_Setting.ChnlRegion[k].vHoldForeLineSecond = StraightLine;
						#endif
                    }
                }

				//待转区第一停止线
				it_rb = channel_info.HoldStopLineFirst.listRegionProp.begin();
                it_re = channel_info.HoldStopLineFirst.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        MvLine StraightLine;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                            if(i==0)
                            {
                               StraightLine.start.x = pt.x;
								StraightLine.start.y = pt.y;
                            }
                            else
                            {
								StraightLine.end.x = pt.x;
								StraightLine.end.y = pt.y;
                            }
                            i++;
                        }
                        vtsRegion.vHoldStopLineFirst = StraightLine;
						#ifdef ALGORITHM_YUV
						g_Setting.ChnlRegion[k].vHoldStopLineFirst = StraightLine;
						#endif
                    }
                }

				//待转区第二停止线
				it_rb = channel_info.HoldStopLineSecond.listRegionProp.begin();
                it_re = channel_info.HoldStopLineSecond.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        MvLine StraightLine;
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);
                            if(i==0)
                            {
                                StraightLine.start.x = pt.x;
								StraightLine.start.y = pt.y;
                            }
                            else
                            {
                                StraightLine.end.x = pt.x;
								StraightLine.end.y = pt.y;
                            }
                            i++;
                        }
                        vtsRegion.vHoldStopLineSecond = StraightLine;
						#ifdef ALGORITHM_YUV
						g_Setting.ChnlRegion[k].vHoldStopLineSecond = StraightLine;
						#endif
                    }
                }


                //雷达检测区域
                if(!bLoadRadar)
                {
                    m_rtRadarRoi.x = 0;
                    m_rtRadarRoi.y = 0;
                    m_rtRadarRoi.width = 0;
                    m_rtRadarRoi.height = 0;

                    it_rb = channel_info.RadarRegion.listRegionProp.begin();
                    it_re = channel_info.RadarRegion.listRegionProp.end();
                    for(; it_rb != it_re; it_rb++)
                    {
                        item_b = it_rb->listPt.begin();
                        item_e = it_rb->listPt.end();
                        i = 0;
                        for( ; item_b!=item_e; item_b++)
                        {
                            if(i==0)
                            {
                                m_rtRadarRoi.x = item_b->x;
                                m_rtRadarRoi.y = item_b->y;
                            }
                            else if(i==2)
                            {
                                m_rtRadarRoi.width = item_b->x - m_rtRadarRoi.x;
                                m_rtRadarRoi.height = item_b->y - m_rtRadarRoi.y;
                            }
                            i++;
                        }
                        //设置雷达检测区域
                        m_rtRadarRoi.x *= m_ratio_x;
                        m_rtRadarRoi.y *= m_ratio_y;
                        m_rtRadarRoi.width *= m_ratio_x;
                        m_rtRadarRoi.height *= m_ratio_y;
                    }
                    bLoadRadar = true;
                }

                //线圈检测参数
                it_rb = channel_info.LoopRegion.listRegionProp.begin();
                it_re = channel_info.LoopRegion.listRegionProp.end();
                j = 0;
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);

                            if(j==0)
                            {
                                if(i==0)
                                {
                                    loopInfo.line0.start.x = pt.x;
									loopInfo.line0.start.y = pt.y;
                                }
                                else
                                {
                                    loopInfo.line0.end.x = pt.x;
									loopInfo.line0.end.y = pt.y;
                                }
                            }
                            else
                            {
                                if(i==0)
                                {
                                    loopInfo.line1.start.x = pt.x;
									loopInfo.line1.start.y = pt.y;
                                }
                                else
                                {
                                    loopInfo.line1.end.x = pt.x;
									loopInfo.line1.end.y = pt.y;
                                }
                            }
                            i++;
                        }
                        //printf("m_lineTurnLeft.start.x=%d,m_lineTurnLeft.start.y=%d,m_lineTurnLeft.end.x=%d,m_lineTurnLeft.end.y=%d\n",m_lineTurnLeft.start.x,m_lineTurnLeft.start.y,m_lineTurnLeft.end.x,m_lineTurnLeft.end.y);
                    }
                    j++;
                }
                if(channel_info.LoopRegion.listRegionProp.size()> 0)
                m_LoopInfo.push_back(loopInfo);

				//黄网格区域
				if(!bLoadYelGridArea)
				{
					m_ptsYelGrid.clear();
					//LogNormal("-11-m_ptsYelGrid=%d", m_ptsYelGrid.size());
					//MvPolygon polyYelGrid;
					it_rb = channel_info.YelGridRgn.listRegionProp.begin();
					it_re = channel_info.YelGridRgn.listRegionProp.end();
					for(; it_rb != it_re; it_rb++)
					{
						item_b = it_rb->listPt.begin();
						item_e = it_rb->listPt.end();
						//printf("it_rb->size()=%d,it_rb->listPt.size()=%d\n",channel_info.YelGridRgn.listRegionProp.size(),it_rb->listPt.size());
						CvPoint* pts = new CvPoint[it_rb->listPt.size()+1];
						i = 0;
						for( ; item_b!=item_e; item_b++)
						{
							pts[i].x = (int)(item_b->x*m_ratio_x + 0.5f);
							pts[i].y = (int)(item_b->y*m_ratio_y + 0.5f);
							//printf("pts[i].x=%d,pts[i].y=%d\n",pts[i].x,pts[i].y);

							CvPoint ptTemp;
							ptTemp.x = pts[i].x;
							ptTemp.y = pts[i].y;

							m_ptsYelGrid.push_back(ptTemp);
							i++;
						}

						//MvPolygon poly(pts, it_rb->listPt.size()+1);
						//m_polyYelGrid(pts, it_rb->listPt.size()+1);
						//m_polyYelGridList.push_back(poly);

						pts[i].x = pts[0].x;
						pts[i].y = pts[0].y;						
						
						delete []pts;
					}

					//设置黄网格区域
					//m_ftVlp.SetYelGridRgn(polyYelGrid);
					//LogNormal("-m_ftVlp.SetYelGridRgn--");//m_ftVlp.SetVioRushTime();					
					bLoadYelGridArea = true;
				}
				
				printf("---111--m_vtsObjectRegion.push_back--m_vtsObjectRegion.size()=%d-\n", m_vtsObjectRegion.size());
				m_vtsObjectRegion.push_back(vtsRegion);                
			}
            else
            {
                m_LoopParmarer.pStart_point = cvPoint( 0, 0 );
                m_LoopParmarer.pEnd_point = cvPoint( 0, 0 );
                //线圈检测参数
                it_rb = channel_info.LoopRegion.listRegionProp.begin();
                it_re = channel_info.LoopRegion.listRegionProp.end();
                j = 0;
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    if(it_rb->listPt.size()>1)
                    {
                        for( ; item_b!=item_e; item_b++)
                        {
                            pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                            pt.y = (int) (item_b->y*m_ratio_y + 0.5);

                            if(j > 0)
                            {
                                if(i==0)
                                {
                                    m_LoopParmarer.pStart_point = pt;
                                }
                                else
                                {
                                    m_LoopParmarer.pEnd_point = pt;
                                }
                            }
                            i++;
                        }
                        //printf("m_lineTurnLeft.start.x=%d,m_lineTurnLeft.start.y=%d,m_lineTurnLeft.end.x=%d,m_lineTurnLeft.end.y=%d\n",m_lineTurnLeft.start.x,m_lineTurnLeft.start.y,m_lineTurnLeft.end.x,m_lineTurnLeft.end.y);
                    }
                    j++;
                }

                m_cnpMap.insert(CnpMap::value_type(vtsRegion.nRoadIndex,cnp));				
            }		

            it_b++;
			k++;
        }

		//LogNormal("-OK-OUT-LoadRoadSettingInfo--\n");
        return true;
    }
    else
    {
        printf("读取车道参数失败!\r\n");
        return false;
    }
}


//设置检测类型
void CRoadCarnumDetect::SetDetectKind(CHANNEL_DETECT_KIND nDetectKind)
{
    if(m_nDetectKind != nDetectKind)
    {
        bool bNeedReloadROI = false;
        CHANNEL_DETECT_KIND preDetectKind = m_nDetectKind;

        //////////////目标检测
        if((preDetectKind&DETECT_OBJECT)==DETECT_OBJECT)//原先需要进行目标检测
        {
            if((nDetectKind&DETECT_OBJECT)!= DETECT_OBJECT)//现在不需要进行目标检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_OBJECT)!= DETECT_OBJECT)//原先不需要进行目标检测
        {
            if((nDetectKind&DETECT_OBJECT)==DETECT_OBJECT)//现在需要进行目标检测
            {
                bNeedReloadROI = true;
            }
        }

        /////////////闯红灯检测
        if((preDetectKind&DETECT_VTS)==DETECT_VTS)//原先需要进行闯红灯检测
        {
            if((nDetectKind&DETECT_VTS)!= DETECT_VTS)//现在不需要进行闯红灯检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_VTS)!= DETECT_VTS)//原先不需要进行闯红灯检测
        {
            if((nDetectKind&DETECT_VTS)==DETECT_VTS)//现在需要进行闯红灯检测
            {
                bNeedReloadROI = true;
            }
        }

        ////////违章检测
        if((preDetectKind&DETECT_VIOLATION)==DETECT_VIOLATION)//原先需要进行违章检测
        {
            if((nDetectKind&DETECT_VIOLATION)!= DETECT_VIOLATION)//现在不需要进行违章检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_VIOLATION)!= DETECT_VIOLATION)//原先不需要进行违章检测
        {
            if((nDetectKind&DETECT_VIOLATION)==DETECT_VIOLATION)//现在需要进行违章检测
            {
                bNeedReloadROI = true;
            }
        }


        /////////////人脸检测
        if((preDetectKind&DETECT_FACE)==DETECT_FACE)//原先需要进行人脸检测
        {
            if((nDetectKind&DETECT_FACE)!= DETECT_FACE)//现在不需要进行人脸检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_FACE)!= DETECT_FACE)//原先不需要进行人脸检测
        {
            if((nDetectKind&DETECT_FACE)==DETECT_FACE)//现在需要进行人脸检测
            {
                bNeedReloadROI = true;
            }
        }

        /////////////线圈检测
        if((preDetectKind&DETECT_LOOP)==DETECT_LOOP)//原先需要进行线圈检测
        {
            if((nDetectKind&DETECT_LOOP)!= DETECT_LOOP)//现在不需要进行线圈检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_LOOP)!= DETECT_LOOP)//原先不需要进行线圈检测
        {
            if((nDetectKind&DETECT_LOOP)==DETECT_LOOP)//现在需要进行线圈检测
            {
                bNeedReloadROI = true;
            }
        }


        /////////////雷达检测
        if((preDetectKind&DETECT_RADAR)==DETECT_RADAR)//原先需要进行雷达检测
        {
            if((nDetectKind&DETECT_RADAR)!= DETECT_RADAR)//现在不需要进行雷达检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_RADAR)!= DETECT_RADAR)//原先不需要进行雷达检测
        {
            if((nDetectKind&DETECT_RADAR)==DETECT_RADAR)//现在需要进行雷达检测
            {
                bNeedReloadROI = true;
            }
        }

        //车身颜色检测
        if((preDetectKind&DETECT_CARCOLOR)==DETECT_CARCOLOR)//原先需要进行车身颜色检测
        {
            if((nDetectKind&DETECT_CARCOLOR)!= DETECT_CARCOLOR)//现在不需要进行车身颜色检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_CARCOLOR)!= DETECT_CARCOLOR)//原先不需要进行车身颜色检测
        {
            if((nDetectKind&DETECT_CARCOLOR)==DETECT_CARCOLOR)//现在需要进行车身颜色检测
            {
                bNeedReloadROI = true;
            }
        }

        //车辆特征检测
        if((preDetectKind&DETECT_TEXTURE)==DETECT_TEXTURE)//原先需要进行车辆特征检测
        {
            if((nDetectKind&DETECT_TEXTURE)!= DETECT_TEXTURE)//现在不需要进行车辆特征检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_TEXTURE)!= DETECT_TEXTURE)//原先不需要进行车辆特征检测
        {
            if((nDetectKind&DETECT_TEXTURE)==DETECT_TEXTURE)//现在需要进行车辆特征检测
            {
                bNeedReloadROI = true;
            }
        }

        //车型检测
        if((preDetectKind&DETECT_TRUCK)==DETECT_TRUCK)//原先需要进行车型检测
        {
            if((nDetectKind&DETECT_TRUCK)!= DETECT_TRUCK)//现在不需要进行车型检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_TRUCK)!= DETECT_TRUCK)//原先不需要进行车型检测
        {
            if((nDetectKind&DETECT_TRUCK)==DETECT_TRUCK)//现在需要进行车型检测
            {
                bNeedReloadROI = true;
            }
        }

		//特征提取
		if((preDetectKind&DETECT_CHARACTER)==DETECT_CHARACTER)//原先需要进行特征提取
        {
            if((nDetectKind&DETECT_CHARACTER)!= DETECT_CHARACTER)//现在不需要进行特征提取
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_CHARACTER)!= DETECT_CHARACTER)//原先不需要进行特征提取
        {
            if((nDetectKind&DETECT_CHARACTER)==DETECT_CHARACTER)//现在需要进行特征提取
            {
                bNeedReloadROI = true;
            }
        }

        m_nDetectKind = nDetectKind;
        if(bNeedReloadROI)
        {
            m_bReloadObjectROI = true;
        }
    }
}

//设置白天晚上还是自动判断
void CRoadCarnumDetect::SetChannelDetectTime(CHANNEL_DETECT_TIME dType)
{
    if (dType == DETECT_AUTO)
    {
        m_nDetectTime = DETECT_AUTO;
    }
    else if (dType == DETECT_DAY)
    {
        m_nDetectTime = DETECT_DAY;
        m_nDayNight =1;
    }
    else
    {
        m_nDetectTime = DETECT_NIGHT;
        m_nDayNight =0;
    }
}

//获取图片路径并将图片编号存储在数据库中
int CRoadCarnumDetect::GetPicPathAndSaveDB(std::string& strPicPath)
{

    pthread_mutex_lock(&g_Id_Mutex);
    ////////////////////
    //需要判断磁盘是否已经满
    g_FileManage.CheckDisk(false,false);

    //存储大图片
    strPicPath  = g_FileManage.GetPicPath();

    int nSaveRet = g_skpDB.SavePicID(g_uPicId);
    //解锁
    pthread_mutex_unlock(&g_Id_Mutex);

    //删除已经存在的记录
    g_skpDB.DeleteOldRecord(strPicPath,false,false);


    return nSaveRet;
}

//获取图片路径
int CRoadCarnumDetect::GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath)
{
	char buf[256] = {0};
	//存储大图片
	std::string strPath;

	string strTime = GetTime(plate.uTime,1);
	g_skpDB.UTF8ToGBK(strTime);

	string strCarnum(plate.chText);

	if(plate.chText[0] == '*')
	{
		if(plate.chText[1] == '-')
		{
			strCarnum = "无牌车";
		}
		else
		{
			strCarnum = "非机动车";
		}
	}
	g_skpDB.UTF8ToGBK(strCarnum);

	if( plate.uViolationType > 0)
	{
		strPath = g_FileManage.GetSpecialPicPath(1);

		string strViolationType = GetViolationType(plate.uViolationType);

		sprintf(buf,"%s/%s",strPath.c_str(),strViolationType.c_str());
		std::string strSubPicPath(buf);

		// 判断目录是否存在,不存在则建立图片目录
		if(access(strSubPicPath.c_str(),0) != 0) //目录不存在
		{
			mkdir(strSubPicPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
			sync();
		}
		sprintf(buf,"%s/%s_%s.jpg",strSubPicPath.c_str(),strTime.c_str(),strCarnum.c_str());
	}
	else
	{
		strPath = g_FileManage.GetSpecialPicPath(0);
		sprintf(buf,"%s/%s_%s.jpg",strPath.c_str(),strTime.c_str(),strCarnum.c_str());
	}

	strPicPath = buf;

	return 1;
}

//重新初始化
void CRoadCarnumDetect::ReInit()
{
    LogNormal("=CRoadCarnumDetect::ReInit()==\n");
		 
	if(g_nDetectMode == 2)
	{
		m_bReinitObj = true;

		//加锁
		pthread_mutex_lock(&m_JpgFrameMutex);
		m_JpgFrameMap.clear();
		pthread_mutex_unlock(&m_JpgFrameMutex);

		pthread_mutex_lock(&m_InPutMutex);
		m_mapInPutResult.clear();
		pthread_mutex_unlock(&m_InPutMutex);

		m_bReloadROI = true;
		m_bReloadObjectROI = true;
	}
}

//修改事件录像
void CRoadCarnumDetect::ModifyEventCapture(bool bEventCapture)
{
    //启动事件录像线程
    if(m_bEventCapture)
    {
		if(g_nDetectMode == 2)
		m_tempRecord.Unit();
		else
        m_skpRoadRecorder.UnInit();
    }

    m_bEventCapture = bEventCapture;

    if(m_bEventCapture)
    {
		if(g_nDetectMode == 2)
		{
			m_tempRecord.SetCamType(m_nCameraType);
			m_tempRecord.Init();
		}
		else
		{
			m_skpRoadRecorder.SetCamType(m_nCameraType);
			m_skpRoadRecorder.Init();
		}
    }
}

//添加事件录象缓冲数据
bool CRoadCarnumDetect::AddVideoFrame(std::string& frame)
{
    printf("CRoadCarnumDetect::AddVideoFrame \n");
    {
           if(m_bEventCapture)
           {
			   printf("m_bEventCapture CRoadCarnumDetect::AddVideoFrame \n");
			
			  if(g_nDetectMode != 2) 
			  m_skpRoadRecorder.AddFrame(frame);

           }
    }
    return true;
}

//获取车身位置
CvRect CRoadCarnumDetect::GetCarPos(RECORD_PLATE plate,int nType)
{
        int x = 0;
        int y = 0;
        int w = 0;//宽度
        int h = 0;//高度

		int nWidth = m_imgSnap->width;
		int nHeight = m_imgSnap->height;

        CvRect rtCar;

		int nExtentHeight = 0;
		if(m_nWordPos == 1)
		{
			nExtentHeight = m_nExtentHeight;
		}

		if(nType == 0 || nType == 2 || nType == 4)
		{
			if(plate.chText[0] != '*')//有牌车
			{
				w = plate.uPosRight - plate.uPosLeft;//宽度
				h = plate.uPosBottom - plate.uPosTop;//高度
				int dw = 2*w;
				if(plate.uType == SMALL_CAR)
				{
					dw = 3*w;
					rtCar.width = 7*w;
				}
				else
				{
					dw = 4*w;
					rtCar.width = 9*w;
				}

				if(nType == 4)//天津取车牌周围区域
				{
					dw = w;
					rtCar.width = 3*w;
				}
				
				rtCar.height = rtCar.width;
				
				x = plate.uPosLeft-dw;

				if(nType == 2)
				{
					if(m_nDetectDirection == 0)
					{
						y = plate.uPosTop-rtCar.height+12*h;
					}
					else
					{
						y = plate.uPosTop-rtCar.height+12*h;
					}
				}
				else
				{
					if(m_nDetectDirection == 0)
					{
						y = plate.uPosTop-rtCar.height+6*h;
					}
					else
					{
						y = plate.uPosTop-rtCar.height+8*h;
					}
				}
			}
			else
			{
				CvPoint point;
				point.x = (plate.uPosRight + plate.uPosLeft)/2.0;
				point.y = (plate.uPosBottom + plate.uPosTop)/2.0;

				if(m_imgSnap->width > 2000)
				{
					rtCar.width = 800;
					rtCar.height = 800;
				}
				else
				{
					rtCar.width = 500;
					rtCar.height = 500;
				}

				x = point.x - rtCar.width/2;
				y = point.y - rtCar.height/2;
			}
			
			

			if(nType == 2 || nType == 4)
			{
				nHeight -= m_nExtentHeight;
				nExtentHeight = 0;
			}
			else
			{
				if(m_nWordPos == 0)
				{
				  nHeight -= m_nExtentHeight;
				}
			}
		}
		else if(nType == 1)
		{
			CvPoint point;
			point.x = (plate.uPosRight + plate.uPosLeft)/2.0;
			point.y = (plate.uPosBottom + plate.uPosTop)/2.0;

			rtCar.width = 800;
			rtCar.height = 800;

			x = point.x - rtCar.width/2;
			y = point.y - rtCar.height/2;

			nHeight -= m_nExtentHeight;
		}
		else if(nType == 3)
		{
			if(plate.chText[0] != '*')//有牌车
			{
				w = plate.uPosRight - plate.uPosLeft;//宽度
				h = plate.uPosBottom - plate.uPosTop;//高度
				int dw = 2*w;
				if(plate.uType == SMALL_CAR)
				{
					dw = 2*w;
					rtCar.width = 5*w;
					rtCar.height = 3*w+6*h;
				}
				else
				{
					dw = 3*w;
					rtCar.width = 7*w;
					rtCar.height = 4*w+8*h;
				}
				
				x = plate.uPosLeft-dw;

				if(m_nDetectDirection == 0)
				{
					y = plate.uPosTop-rtCar.height;
				}
				else
				{
					y = plate.uPosTop-rtCar.height;
				}	
			}
			else
			{
				CvPoint point;
				point.x = (plate.uPosRight + plate.uPosLeft)/2.0;
				point.y = (plate.uPosBottom + plate.uPosTop)/2.0;

				if(m_imgSnap->width > 2000)
				{
					rtCar.width = 800;
					rtCar.height = 800;
				}
				else
				{
					rtCar.width = 500;
					rtCar.height = 500;
				}

				x = point.x - rtCar.width/2;
				y = point.y - rtCar.height/2;
			}

			nHeight -= m_nExtentHeight;
			nExtentHeight = 0;
		}

        if(x > 0)
        {
            rtCar.x = x;
        }
        else
        {
            rtCar.x = 0;
        }

        if(y > nExtentHeight)
        {
            rtCar.y = y;
        }
        else
        {
            rtCar.y = nExtentHeight;
        }


        if(rtCar.x+rtCar.width>=nWidth)
        {
            rtCar.x = nWidth - rtCar.width-1;
        }

        if(rtCar.y+rtCar.height>=nHeight)
        {
            rtCar.y = nHeight - rtCar.height-1;
        }

        return rtCar;
}

//添加车牌信息
bool CRoadCarnumDetect::AddPlateFrame(DSP_PLATE_LIST& listDspPlate)
{
	#ifndef ALGORITHM_YUV
    if(listDspPlate.size() > 0)
    {

		DSP_PLATE_LIST::iterator it_b = listDspPlate.begin();
		DSP_PLATE_LIST::iterator it_e = listDspPlate.end();

		while(it_b != it_e)
		{
			//RECORD_PLATE_DSP信息转换成CarInfo类型信息。
			RECORD_PLATE_DSP plate = *it_b;
			RECORD_PLATE_DSP *pPlate = &plate;

			if((pPlate->uSeq == 0) && (pPlate->uRoadWayID == 0))
			{
				LogError("相机重启清空旧缓冲区\n");

				pthread_mutex_lock(&m_InPutMutex);
				m_mapInPutResult.clear();
				pthread_mutex_unlock(&m_InPutMutex);

				LogError("清空数据缓冲区成功\n");

				//加锁
				pthread_mutex_lock(&m_JpgFrameMutex);
				m_JpgFrameMap.clear();
				//解锁
				pthread_mutex_unlock(&m_JpgFrameMutex);
				LogError("清空图片缓冲区成功\n");
				break;
			}

			CarInfo    carnums;
			//char text[8] = "1234567";
			char text[10] = "123456789";
			//memcpy(text+1, pPlate->chText+2, 6);


			if(pPlate->chText[0] != 'L') //非武警牌照
			{
			    memset(text, 0, 10);
			    memcpy(text, pPlate->chText, 8);
			    memcpy(carnums.strCarNum, text, 8);
			}
			else
			{
			    memset(text, 0, 10);
			    memcpy(text, pPlate->chText, 9);
			    memcpy(carnums.strCarNum, text, 7);
			    memcpy(carnums.wj, text+7, 2); //武警牌下面的小数字
			}

			//LogNormal("l=%d,t=%d,w=%d,h=%d\n", pPlate->uPosLeft,pPlate->uPosTop,pPlate->uPosRight,pPlate->uPosBottom);


			carnums.color = pPlate->uColor;
			carnums.vehicle_type = pPlate->uType;


			/*UINT32 uPosTop = pPlate->uPosTop;
			UINT32 uPosBottom = pPlate->uPosBottom;
			pPlate->uPosBottom = 2048 - uPosTop;
			pPlate->uPosTop = 2048 - uPosBottom;*/


			//LogNormal("uViolationType=%d,l=%d,t=%d,r=%d,b=%d\n",\
				pPlate->uViolationType,pPlate->uPosLeft,pPlate->uPosTop,pPlate->uPosRight,pPlate->uPosBottom);

			carnums.ix = (pPlate->uPosLeft);
			carnums.iy = (pPlate->uPosTop);
			carnums.iwidth = (pPlate->uPosRight - pPlate->uPosLeft);
			carnums.iheight = (pPlate->uPosBottom - pPlate->uPosTop);
			
			//#ifdef OBJECTFEATURESEARCH
			CvRect rtCarPos;
			rtCarPos.x = pPlate->uCarPosLeft;
			rtCarPos.y = pPlate->uCarPosTop;
			rtCarPos.width = pPlate->uCarPosRight - pPlate->uCarPosLeft;
			rtCarPos.height = pPlate->uCarPosBottom - pPlate->uCarPosTop;

			/*if(0 == rtCarPos.width)
			{
				LogNormal("AddPlateFrame uSeq:%lld, DspPos:%d,%d,%d,%d %s", \
					pPlate->uSeq, rtCarPos.x, rtCarPos.y,\
					rtCarPos.width, rtCarPos.height, pPlate->chText);
			}*/
			


			carnums.m_CarWholeRec = rtCarPos;
			//LogNormal("m_CarWholeRec===x=%d,y=%d,w=%d,h=%d\n",rtCarPos.x,rtCarPos.y,rtCarPos.width,rtCarPos.height);
			//#endif
			

			//carnums.iscarnum = 1;

			//carnums.mean = pPlate->uContextMean;
			//carnums.stddev = pPlate->uContextStddev / 100; //检测区域平均亮度方差（实际方差结果*100，精确到小数点后两位）

			carnums.mean = -1;
			carnums.stddev = -1;

			//carnums.smearCount = 0;
			carnums.VerticalTheta = pPlate->uVerticalTheta;
			carnums.HorizontalTheta = pPlate->uHorizontalTheta;

			carnums.uTimestamp = pPlate->uTime;
			carnums.ts = (int64_t)pPlate->uTime*1000000+pPlate->uMiTime*1000;

			//判断时间是否在合理范围内，不合理则丢弃
			if(abs((long long)(GetTimeStamp())-(long long)carnums.uTimestamp) > 3600*24*90)//如果相机时间与检测器时间相差3个月以上则丢弃此数据
			{
				LogError("discard record %s\n",GetTime(carnums.uTimestamp).c_str());
				//发送相机重启命令
				CAMERA_CONFIG cfg; 
				cfg.uType = 1; 
				cfg.nIndex = (int)CAMERA_RESTART; 
				g_skpChannelCenter.CameraControl(m_nChannelId,cfg); 
				LogError("相机时间不合理重启相机\n");
				return false;
			}

			carnums.uSedImgFramseq = pPlate->uSeq2;
			carnums.uSeq = pPlate->uSeq;
			carnums.carnumrow = pPlate->uPlateType;
			
			//LogNormal("seq1:%lld seq2:%lld car:%s, v:%d \n", \
			//	pPlate->uSeq, pPlate->uSeq2, carnums.strCarNum, pPlate->uSpeed);

			//添加车牌世界坐标 add by wantao
			//carnums.wx = (double)((pPlate->uLongitude)*1.0/(10000*100));
			//carnums.wy = (double)((pPlate->uLatitude)*1.0/(10000*100));

			carnums.wx = (double)((pPlate->uLongitude) * 0.0001);
			carnums.wy = (double)((pPlate->uLatitude) * 0.0001);

			//LogTrace("GJ_T1.log", "==seq:%d, uTime=%d,uMiTime=%d pPlate->uLongitude=%d, pPlate->uLatitude=%d==wx=%f,wy=%f", \
				pPlate->uSeq, pPlate->uTime, pPlate->uMiTime, pPlate->uLongitude, pPlate->uLatitude, carnums.wx, carnums.wy);

			//添加相机IDadd by wantao
			carnums.id = pPlate->uCameraId;

			carnums.vx = 0;
			carnums.vy = pPlate->uSpeed; //传入dsp相机线圈检测出速度
			carnums.RoadIndex = pPlate->uRoadWayID;

			//carnums.nDirection = pPlate->uDirection; //FIX ME
			//LogTrace("Direct.txt", "22==DSP_SPEED=uSeq:%d uSpeed=%d,chText=%s,uViolationType=%d,nDirection=%d\n", \
				pPlate->uSeq, pPlate->uSpeed,pPlate->chText,pPlate->uViolationType,pPlate->uDirection);


			//加锁
			pthread_mutex_lock(&m_InPutMutex);
			
			{
				if(g_nDetectMode == 2)
				{
/*
#ifdef DEBUG_PLATE_TEST
					//test
					if(pPlate->uSeq % 2 == 0)
					{
						//pPlate->uViolationType = pPlate->uSeq % (DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD+1);//(pPlate->uSeq % 3);
						pPlate->uViolationType = 13;
						pPlate->uTypeDetail = BIG_CAR;//大车
						pPlate->uSeq2 = pPlate->uSeq;
						pPlate->uSeq3 = pPlate->uSeq + 2;
						pPlate->uSeq4 = pPlate->uSeq + 5;
						//pPlate->uColor = CARNUM_YELLOW;
						//carnums.color = pPlate->uColor;

						//memcpy(pPlate->chText, "LJ0123456", 7);//"LJ0123456"
						//memcpy(carnums.strCarNum, pPlate->chText, 7);
						//memcpy(carnums.wj, pPlate->chText+7, 2);//武警牌下面的小数字
					}					
					//end test
#endif
*/
					if(pPlate->uViolationType >= 0 && pPlate->uViolationType < 99)//电警数据
					{
						/*
#ifdef DEBUG_PLATE_TEST
						memcpy(carnums.strCarNum, "*-----*", 7);
#endif
						*/
#ifdef TEST_MATCH
						int nRnd = rand();
						pPlate->uViolationType = nRnd % 10;//闯红灯
#endif
						ViolationInfo info;
						std::vector<ViolationInfo> vResult;
						memcpy(&info.carInfo,&carnums,sizeof(CarInfo));
						info.evtType = (VIO_EVENT_TYPE)pPlate->uViolationType;//违章类型
						info.nChannel = pPlate->uRoadWayID;//车道编号
						info.frameSeqs[0] = pPlate->uSeq;//取图帧号
						info.frameSeqs[1] = pPlate->uSeq2;
						info.frameSeqs[2] = pPlate->uSeq3;
						info.frameSeqs[3] = pPlate->uSeq4;//卡口图片帧号
						info.nPicCount = 3;//图片数量
						info.redLightStartTime = pPlate->uRedLightStartTime;//红灯开始时间
						info.uUpperFrm = pPlate->uUpperFrm;
						info.uLowerFrm = pPlate->uLowerFrm;
						info.dis[0]  = pPlate->uDis[0];
						info.dis[1]  = pPlate->uDis[1];
						info.dis[2]  = pPlate->uDis[2];

						//LogTrace("Vts-log.log", "=add vtsoutput frameSeqs[%d,%d,%d]===info.evtType=%d==info.redLightStartTime=%d==\n", \
							info.frameSeqs[0], info.frameSeqs[1], info.frameSeqs[2], info.evtType, (info.redLightStartTime/1000)/1000);

						vResult.push_back(info);
						AddVtsOutPut(vResult);
					}
					else//卡口数据
					{
						//黑名单比对
						if(g_nLoadBasePlateInfo == 1)
						{
							//调用比对模块比对
							int nWFLX = g_MyCenterServer.FoundPlateInfo(pPlate->chText,pPlate->uColor);
							if (nWFLX > 0)
							{
								//FILE* fp = fopen("md5.txt","a+");
								//fprintf(fp,"[%s]:FoundPlateInfo:%s pPlate->uSeq2:%d\n",GetTime(GetTimeStamp(),0).c_str(),pPlate->chText,pPlate->uSeq2);
								//fclose(fp);

								if(pPlate->uSeq2 > 0)
								{
									ViolationInfo info;
									std::vector<ViolationInfo> vResult;
									memcpy(&info.carInfo,&carnums,sizeof(CarInfo));
									if(nWFLX == 12)
									{
										info.evtType = (VIO_EVENT_TYPE)EVT_YELLOW_PLATE;//黄标车禁行
									}
									else if(nWFLX == 13)
									{
										info.evtType = (VIO_EVENT_TYPE)EVT_CYC_YELLOE_PLATE;//国1标准汽油车禁行
									}
									info.nChannel = pPlate->uRoadWayID;//车道编号
									info.frameSeqs[0] = pPlate->uSeq;//取图帧号
									if(pPlate->uSeq <= pPlate->uSeq2)
									{
										info.frameSeqs[1] = pPlate->uSeq;
										info.frameSeqs[2] = pPlate->uSeq2;
									}
									else
									{
										info.frameSeqs[1] = pPlate->uSeq2;
										info.frameSeqs[2] = pPlate->uSeq;
									}
									info.frameSeqs[3] = pPlate->uSeq;//卡口图片帧号
									info.nPicCount = 3;//图片数量
	
									vResult.push_back(info);
									AddVtsOutPut(vResult);
								}
							}
						}

						//if(m_mapInPutResult.size() > MAX_LOAD_NUMS)
						if(m_mapInPutResult.size() > 100)
						{
							//LogNormal("输入记录结果过多，未能及时输出=%d\n",m_mapInPutResult.size());
							//printf("Too much plate records!===m_vInResult.size()=%d==\n",m_mapInPutResult.size());

							std::multimap<int64_t,DSP_CARINFO_ELEM>::iterator it_map = m_mapInPutResult.begin();
							m_mapInPutResult.erase(it_map);
						}
						else
						{
							DSP_CARINFO_ELEM carInfoElem;
							
							//检测器接收相机数据时间戳,单位s.
							carInfoElem.uTimeRecv = GetTimeStamp();
							memcpy((char*)(&carInfoElem.cardNum), (char*)(&carnums), sizeof(carnums));

							m_mapInPutResult.insert(make_pair(carnums.uSeq,carInfoElem));				
							//LogNormal("添加车牌信息成功帧号:%d\n",carnums.uSeq);
						}
					}
				}
			}
			//解锁
			pthread_mutex_unlock(&m_InPutMutex);

			it_b++;
		}
		return true;
    }
	#endif
    return false;
}

//通过帧号查找车牌结果
bool CRoadCarnumDetect::GetInPutBySeq(UINT32 nSeq, std::vector<CarInfo>& vResult)
{
	bool bRet = false;
	//加锁
	pthread_mutex_lock(&m_InPutMutex);

	if(m_mapInPutResult.size() > 0)
	{
		printf("=======GetInPutBySeq()===nSeq=%d==m_mapInPutResult.size()=%d===", \
			nSeq, m_mapInPutResult.size());

		std::multimap<int64_t,DSP_CARINFO_ELEM>::iterator it_b = m_mapInPutResult.find(nSeq);
		while(it_b != m_mapInPutResult.end())
		{
			printf("==GetInPutBySeq()====it_b->uSeq=%d====\n", it_b->first);
			if(it_b->first != nSeq)
			{
				break;
			}
			else
			{
				vResult.push_back(it_b->second.cardNum);
				m_mapInPutResult.erase(it_b++);

				bRet = true;
				continue;
			}
			it_b++;
		}
	}

	//解锁
	pthread_mutex_unlock(&m_InPutMutex);


	return bRet;
}

//输出测试结果
void CRoadCarnumDetect::OutPutTestResult(UINT32 uSeq,int64_t ts)
{
	if(m_listTestResult.size() > 0)
	{
		std::list<RECORD_PLATE>::iterator it_b = m_listTestResult.begin();
		std::list<RECORD_PLATE>::iterator it_e = m_listTestResult.end();

		while(it_b != it_e)
		{
			if( uSeq >= it_b->uSeqID )
			{
				LogNormal("输出结果uSeq=%lld,uSeqID=%lld",uSeq,it_b->uSeqID);

				RECORD_PLATE plate = *it_b;

				//经过时间(秒)
				plate.uTime = ts/1000000;
				//毫秒
				plate.uMiTime = (ts/1000)%1000;

				//地点
				memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());

				//方向
				plate.uDirection = m_nDirection;

				//获取图片路径
				std::string strPicPath;
				int nSaveRet = GetPicPathAndSaveDB(strPicPath);
				//大图存储路径
				memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());


				//违章检测
				if( (plate.uViolationType > DETECT_RESULT_ALL) && ((m_nDetectKind&DETECT_VIOLATION)==DETECT_VIOLATION))
				{
						printf("plate.uViolationType=%d\n",plate.uViolationType);

						if(m_imgComposeSnap != NULL)
						{
							//图片尺寸
							plate.uPicWidth = m_imgComposeSnap->width;
							plate.uPicHeight = m_imgComposeSnap->height;

							PLATEPOSITION  TimeStamp[6];

							int nPicCount = 3;

							UINT32 frameSeqs[3] = {0};

							frameSeqs[0] = it_b->uSeqID;

							if(it_b->uTime2 > 0)
							{
								frameSeqs[1] = it_b->uTime2;
							}
							else
							{
								frameSeqs[1] = it_b->uSeqID-3;
							}

							if(it_b->uMiTime2 > 0)
							{
								frameSeqs[2] = it_b->uMiTime2;
							}
							else
							{
								frameSeqs[2] = it_b->uSeqID-6;
							}

							//printf("frameSeqs[0]=%d,frameSeqs[1]=%d,frameSeqs[2]=%d\n",frameSeqs[0],frameSeqs[1],frameSeqs[2]);
							//获取违章检测结果图像
							GetVtsImageByIndex(frameSeqs,TimeStamp,nPicCount,0,NULL,0);

							for(int nIndex = 0; nIndex <nPicCount ; nIndex++)
							{
								PutVtsTextOnImage(m_imgComposeSnap,plate,nIndex,TimeStamp);
							}
							plate.uPicSize = SaveImage(m_imgComposeSnap,strPicPath,2);
						}
				}
				else//卡口
				{
					 //图片尺寸
					plate.uPicWidth = m_imgDestSnap->width;
					plate.uPicHeight = m_imgDestSnap->height;

					PLATEPOSITION  TimeStamp[2];
					TimeStamp[0].uTimestamp = plate.uTime;

					//获取过滤图片
					bool bGetJpg = GetImageByIndex(it_b->uSeqID,0,TimeStamp,m_imgSnap);
					if(!bGetJpg)
					{
						//LogNormal("bGetJpg 1010 !");
						continue;
						//return;
					}

					if(plate.chText[0] == '*')//行人
					{
						CvPoint center;
						center.x= (plate.uPosLeft+plate.uPosRight)/2.0;
						center.y = (plate.uPosTop+plate.uPosBottom)/2.0;
						int radius = 25;
						CvScalar color = cvScalar(255,0,0);
						cvCircle(m_imgSnap,center,radius,color,2);
					}
					//LogNormal("19 PutTextOnImage seq:%d ", plate.uSeq);

					 PutTextOnImage(m_imgSnap,plate,0,TimeStamp);

					 plate.uPicSize = SaveImage(m_imgSnap,strPicPath,0);
				}

				//保存车牌记录
				if(nSaveRet>0)
			   {
					g_skpDB.SavePlate(m_nChannelId,plate,0);
				}

			    //将车牌信息送客户端
				if(m_bConnect)
				{
					SendResult(plate,it_b->uSeqID);
				}

				m_listTestResult.erase(it_b);
				break;
			}
			it_b++;
		}
	}
}

//读取测试结果文件
bool CRoadCarnumDetect::LoadTestResult(char* filename)
{
	if(access(filename,F_OK) == 0)
	{
		m_listTestResult.clear();

		LogNormal("filename=%s",filename);
		g_skpDB.LoadResult(filename,m_listTestResult);

		if(m_listTestResult.size() > 0)
		{
			LogNormal("m_listTestResult.size()=%d",m_listTestResult.size());
			return true;
		}
	}
	return false;
}

/*
* 函数介绍：通过帧号，从jpg map中寻找一个违章检测，对应的图片最接近的图片
* 输入参数：nSeq:目标帧号，nPreSeq：前一张帧号
* 输出参数：strPic:结果输数据
* 返回值 ：无
*/
void CRoadCarnumDetect::GetPicVtsFromJpgMap(UINT32 nSeq, UINT32 nPreSeq, string& strPic)
{
	printf("=in=GetPicVtsFromJpgMap==nSeq=%d,nPreSeq=%d \n", nSeq, nPreSeq);
	bool bFind = false;

	map<int64_t,string>::iterator it = m_JpgFrameMap.begin();
	map<int64_t,string>::iterator it_pre;

	UINT32 nSeqCur = 0;
	UINT32 nSeqPre = 0;
	UINT32 nDisSeq = 0;//帧号差

	//test log out put seq
	int nOutSeq = 0;

	while( (it != m_JpgFrameMap.end()) && (m_JpgFrameMap.size() >= 2) )
	{
		nSeqCur = it->first; //从map中取一帧

		if((nSeq <= nSeqCur) && ((nPreSeq == 0) ||(nSeqCur > nPreSeq)))
		{
			it_pre = it;

			if(it != m_JpgFrameMap.begin())
			{
				it_pre--;

				nSeqPre = it_pre->first; //当前帧的前一帧,序列[nSeqPre,ToFindSeq,nSeqCur]

				/*if(nPreSeq == nSeqCur)
				{
					nOutSeq = it_pre->first;
					strPic = it_pre->second;

					LogTrace("OutputSeq3.log", "#=111=nOutSeq=%d=#", nOutSeq);
				}
				else */if(nPreSeq == nSeqPre)
				{
					nOutSeq = it->first;
					strPic = it->second;
					LogTrace("OutputSeq3.log", "#=222=nOutSeq=%d=#", nOutSeq);
				}
				else
				{
					nDisSeq = nSeqCur - nSeqPre;

					//前后，帧差12以内取最近一张，否则取离摄像机更远的一张
					//由远及近：帧号更小的一张（时间靠前）
					//由近及远：帧号更大的一张（时间靠后）
					if(nDisSeq <= 12)
					{
						if(nSeq <= (nSeqCur+nSeqPre)*0.5) //择半查找，取更近的那一帧
						{
							nOutSeq = it_pre->first;
							strPic = it_pre->second;
							LogTrace("OutputSeq3.log", "#=333=nOutSeq=%d=,nDisSeq=%d #", nOutSeq, nDisSeq);
						}
						else
						{
							nOutSeq = it->first;
							strPic = it->second;
							LogTrace("OutputSeq3.log", "#=444=nOutSeq=%d=,nDisSeq=%d #", nOutSeq, nDisSeq);
						}
					}
					else
					{
						if(m_nDetectDirection == 0)//前牌（由远及近）
						{
							nOutSeq = it_pre->first;
							strPic = it_pre->second;
							LogTrace("OutputSeq3.log", "#=555=nOutSeq=%d=#,nDisSeq=%d #", nOutSeq, nDisSeq);
						}
						else //尾牌（由近及远）
						{
							nOutSeq = it->first;
							strPic = it->second;
							LogTrace("OutputSeq3.log", "#=666=nOutSeq=%d=#,nDisSeq=%d #", nOutSeq, nDisSeq);
						}
					}
				}

				//LogNormal("nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n",nSeq,nSeqPre,nSeqCur);

				LogTrace("OutputSeq3.log", "=EEE=nOutSeq=%d==nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n", \
					nOutSeq, nSeq, nSeqPre, nSeqCur);

				bFind = true;
				break;

			}//End of if(it != m_JpgFrameMap.begin())
		}//End of if(nSeq <= nSeqCur)

		if(it == --m_JpgFrameMap.end())
		{
			nOutSeq = it->first;
			strPic = it->second;
			LogTrace("OutputSeq3.log", "#=777=nOutSeq=%d #", nOutSeq);

			bFind = true;
			break;
		}

		it++;
	}

	if(!bFind)
	{
		if(m_JpgFrameMap.size() >0)
		{
			it = m_JpgFrameMap.begin();
			strPic = it->second;

			nOutSeq = it->first;
			LogTrace("OutputSeq.log", "=2222=nOutSeq=%d==nSeq=%lld==\n", nOutSeq, nSeq);
		}
	}

	//LogTrace("MAX_MIN.log", "===nOutSeq=%d==nSeq=%lld==\n", nOutSeq, nSeq);
}


//设置H264采集类指针
bool CRoadCarnumDetect::SetH264Capture(CRoadH264Capture *pH264Capture)
{
	m_pH264Capture = pH264Capture;
	if(m_pH264Capture != NULL)
	{
		//LogNormal("===m_pH264Capture is not NULL==\n");
		return  true;
	}
	else
	{
		//LogNormal("===m_pH264Capture is NULL!!!!==\n");
		return false;
	}
}

//清空输出JpgMap列表
void CRoadCarnumDetect::ClearJpgFrameMap()
{
	if(m_JpgFrameMap.size() > 0)
	{
		//加锁
		pthread_mutex_lock(&m_JpgFrameMutex);

		LogNormal("==相机重置，清空缓冲数据,通道[%d],nCount=%d!=\n", \
			m_nChannelId, m_JpgFrameMap.size());
		m_JpgFrameMap.clear();
		//解锁
		pthread_mutex_unlock(&m_JpgFrameMutex);

		pthread_mutex_lock(&m_OutPutMutex);

		LogNormal("清空车牌数据[%d],违章数据[%d]，通道[%d]!=\n", \
			m_vResult.size(), m_vtsResult.size(), m_nChannelId);

		m_vResult.clear();
		m_vtsResult.clear();

		pthread_mutex_unlock(&m_OutPutMutex);

		pthread_mutex_lock(&m_InPutMutex);
		LogNormal("=m_mapInPutResult=sizes=[%d]==\n", m_mapInPutResult.size());
		m_mapInPutResult.clear();
		pthread_mutex_unlock(&m_InPutMutex);
	}
}

//从jpg map中寻找一个最接近的违章图片
void CRoadCarnumDetect::GetPicByKeyFromMap(Picture_Key pickeyDst, string& strPic)
{
	bool bFind = false;

	UINT32 nDisSeq = 0;
	Picture_Key pickeyCur;
	Picture_Key pickeyPre;

	map<Picture_Key,string>::iterator it_pre;
	map<Picture_Key,string>::iterator it;

	if(m_ServerJpgFrameMap.size() > 0)
	{
		it = m_ServerJpgFrameMap.begin();
		while( (it!=m_ServerJpgFrameMap.end())&&(m_ServerJpgFrameMap.size()>=1))
		{
			pickeyCur = it->first;

			//if( (nSeq <= nSeqCur) && ((nPreSeq == 0) ||(nSeqCur > nPreSeq)))//尾牌
			if(pickeyDst.uSeq <= pickeyCur.uSeq)
			{
				it_pre = it;
				if(it != m_ServerJpgFrameMap.begin())
				{
					it_pre--;
				}

				pickeyPre = it_pre->first;


				nDisSeq = pickeyCur.uSeq - pickeyPre.uSeq;//帧号差

				//前后，帧差12以内取最近一张，否则取离摄像机更远的一张
				//由远及近：帧号更小的一张（时间靠前）
				//由近及远：帧号更大的一张（时间靠后）
				if(nDisSeq <= 12)
				{
					if(pickeyDst.uSeq <= (pickeyCur.uSeq + pickeyPre.uSeq) * 0.5) //择半查找，取更近的那一帧
					{
						strPic = it_pre->second;
					}
					else
					{
						strPic = it->second;
					}
				}
				else
				{
					//尾牌（由近及远）
					strPic = it->second;
				}


				LogNormal("nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n", pickeyDst.uSeq,pickeyPre.uSeq,pickeyCur.uSeq);

				bFind = true;
				break;
			}

			//LogNormal("##TT== nSeq=%d =TT##\n", pickeyCur.uSeq);
			it++;
		}

		if(!bFind)
		{
			if(m_ServerJpgFrameMap.size() >0)
			{
				it = --m_ServerJpgFrameMap.end();
				strPic = it->second;
				bFind = true;
				LogNormal("===find=pic==end,uSeq=%lld\n",(it->first).uSeq);
			}
		}
	}
}

/**
* 2D linear filter
* \param kernel: convolving matrix, in row format.
* \param Ksize: size of the kernel.
* \param Kfactor: normalization constant.
* \param Koffset: bias.
* \verbatim Example: the "soften" filter uses this kernel:
1 1 1
1 8 1
1 1 1
the function needs: kernel={1,1,1,1,8,1,1,1,1}; Ksize=3; Kfactor=16; Koffset=0; \endverbatim
* \return true if everything is ok
*/

bool CRoadCarnumDetect::Filter( unsigned char *pImageData, short nWidth, short nHeight,long *kernel, short Ksize, short Kfactor, short Koffset, unsigned char *pTempImage )
{
	if (!pImageData) return false;

	//short k2 = Ksize/2;//k2=1;
	//short kmax= Ksize-k2;//kmax=2
	int x,y,r,g,b;
	unsigned char red,green,blue;
	short xmin,xmax,ymin,ymax;
	short nWidth2;

	int kernelCenter = kernel[4];

	nWidth2 = nWidth*3;

	xmin = ymin = 0;
	xmax = nWidth;
	ymax = nHeight;	

	xmin = 2*3;
	xmax = (nWidth-2)*3;

	red = 0;
	green = 0;
	blue = 0;

	for( y = 2; y < nHeight-2; y++ )
	{			
		for(x = xmin; x < xmax; x += 3)
		{
			r = b = g = 0;
			//-1行-1，0，1列
			r -= pImageData[(y-1)*nWidth2 + x - 3];
			g -= pImageData[(y-1)*nWidth2 + x - 2];
			b -= pImageData[(y-1)*nWidth2 + x - 1];

			r -= pImageData[(y-1)*nWidth2 + x + 0];
			g -= pImageData[(y-1)*nWidth2 + x + 1];
			b -= pImageData[(y-1)*nWidth2 + x + 2];

			r -= pImageData[(y-1)*nWidth2 + x + 3];
			g -= pImageData[(y-1)*nWidth2 + x + 4];
			b -= pImageData[(y-1)*nWidth2 + x + 5];

			//0行-1，0，1列
			r -= pImageData[y * nWidth2 + x - 3];
			g -= pImageData[y * nWidth2 + x - 2];
			b -= pImageData[y * nWidth2 + x - 1];

			r += pImageData[y * nWidth2 + x + 0]*kernelCenter;
			g += pImageData[y * nWidth2 + x + 1]*kernelCenter;
			b += pImageData[y * nWidth2 + x + 2]*kernelCenter;

			r -= pImageData[y * nWidth2 + x + 3];
			g -= pImageData[y * nWidth2 + x + 4];
			b -= pImageData[y * nWidth2 + x + 5];

			//1行-1，0，1列
			r -= pImageData[(y+1)*nWidth2 + x - 3];
			g -= pImageData[(y+1)*nWidth2 + x - 2];
			b -= pImageData[(y+1)*nWidth2 + x - 1];

			r -= pImageData[(y+1)*nWidth2 + x + 0];
			g -= pImageData[(y+1)*nWidth2 + x + 1];
			b -= pImageData[(y+1)*nWidth2 + x + 2];

			r -= pImageData[(y+1)*nWidth2 + x + 3];
			g -= pImageData[(y+1)*nWidth2 + x + 4];
			b -= pImageData[(y+1)*nWidth2 + x + 5];

			red   = rgb_Kfactor9_25[(r+2040)*17+((kernelCenter-8)-1)];   //(rgb+2040)*7+(kernelCenter-8-1)
			green = rgb_Kfactor9_25[(g+2040)*17+((kernelCenter-8)-1)];   //rgb_Kfactor7[(rgb+2040)*7+(kernelCenter-8-1)];
			blue  = rgb_Kfactor9_25[(b+2040)*17+((kernelCenter-8)-1)];

			pTempImage[y * nWidth2 + x] = red;
			pTempImage[y * nWidth2 + x + 1] = green;
			pTempImage[y * nWidth2 + x + 2] = blue;
		}
	}

	memcpy(pImageData, pTempImage, nWidth * nHeight * 3 * sizeof(unsigned char));
	return true;
}

//输出一条违章检测结果
bool CRoadCarnumDetect::OutPutVTSResultElem(ViolationInfo &infoViolation, string &strEvent, SRIP_DETECT_HEADER &sHeader, bool bGetVtsImgByKey)
{
	TIMESTAMP vtsStamp[3];
	//核查违章图片是否都存在,不全则不输出
	if(g_nDetectMode == 2)
	{
		bool bCheckVts = CheckVtsPic(infoViolation, vtsStamp);
		if(!bCheckVts)
		{
			return false;
		}

		if(m_imgComposeSnap == NULL)
		{
			return false;
		}
	}

	//LogNormal("CRoadCarnumDetect frameSeqs:%d,%d,%d %d\n", \
	//	infoViolation.frameSeqs[0], infoViolation.frameSeqs[1], infoViolation.frameSeqs[2], infoViolation.frameSeqs[3]);
#ifdef PLATEDETECT
	if(infoViolation.carInfo.strCarNum[0] == '*')//当报出车牌为*---*或*****时做第二次判别,过滤掉无牌车记录
	{
		if(ELE_RED_LIGHT_VIOLATION == infoViolation.evtType ||
			ELE_FORBID_LEFT == infoViolation.evtType ||
			ELE_FORBID_RIGHT == infoViolation.evtType ||
			ELE_FORBID_STRAIGHT == infoViolation.evtType)//闯红灯、禁左、禁右、禁前 */
		{
			//LogNormal("vts:%d car:%s seq:%d", \
				infoViolation.evtType, infoViolation.carInfo.strCarNum, infoViolation.frameSeqs[3]);
			if(g_nDetectMode != 2)
			{
				infoViolation.carInfo.m_CarWholeRec = infoViolation.carInfo.m_NovehiclRec;
			}
			//LogNormal("CarWholeRec x=%d,y=%d,w=%d,h=%d\n",cardNum.m_CarWholeRec.x, cardNum.m_CarWholeRec.y,cardNum.m_CarWholeRec.width, cardNum.m_CarWholeRec.height);

			if(infoViolation.frameSeqs[3] > 0)
			{
				PLATEPOSITION TimeStampTemp;
				PLATEPOSITION* pTimeStampTemp = &TimeStampTemp;
				string strPicMatch;
				bool bGetPic = GetImageByJpgSeq(infoViolation.frameSeqs[3], 0, pTimeStampTemp, m_imgSnap, strPicMatch);
				if(bGetPic)
				{
					if(infoViolation.carInfo.m_CarWholeRec.width > 0 && infoViolation.carInfo.m_CarWholeRec.height > 0 )
					{
						if(m_PlateDetector.JudgeIsRealVehicle(m_imgSnap,infoViolation.carInfo.m_CarWholeRec))
						{
							LogNormal("JudgeIsRealVehicle %s x:%d w:%d\n", \
								infoViolation.carInfo.strCarNum, infoViolation.carInfo.m_CarWholeRec.x, infoViolation.carInfo.m_CarWholeRec.width);
							memcpy(infoViolation.carInfo.strCarNum,"*-----*",7);			
						}
						else
						{
							return false;//非车辆的目标不报违章
						}			
					}
				}		
				pTimeStampTemp = NULL;
			}			
		}	
	}
#endif

		#ifdef ALGORITHM_YUV
	    infoViolation.nPicCount = 3;
		#endif
        RECORD_PLATE plate;
		plate.uSeqID = infoViolation.carInfo.uSeq;
		
         //违章类型
		plate.uViolationType =infoViolation.evtType;
		VtsTypeConvert(plate);

        if(infoViolation.evtType == ELE_RETROGRADE_MOTION)			//5 逆行
        {
			if(g_nDetectMode != 2)//1拖N不要转换
			{
				if(infoViolation.nPicCount == 3)
				{
					UINT32 frameSeqs = infoViolation.frameSeqs[0];
					infoViolation.frameSeqs[0] = infoViolation.frameSeqs[2];
					infoViolation.frameSeqs[2] = frameSeqs;
				}
			}
        }

        std::string strCarNum = infoViolation.carInfo.strCarNum;
        //判断是否有车牌的车
        bool bCarNum = true;
        if( (infoViolation.carInfo.strCarNum[0] == '*') && (infoViolation.carInfo.strCarNum[6] == '*') )
        {
            bCarNum = false;
        }

		if(bCarNum)
		{	
			PLATEPOSITION  TimeStamp0;
			bool bGetJpg = GetImageByIndex(infoViolation.frameSeqs[3],0,&TimeStamp0, m_imgSnap);
			if(bGetJpg)
			{
				carnum_context vehicle_result;
				bool bRetValue = CarNumTwiceDetect(infoViolation.carInfo,vehicle_result);		
				/*if (!bRetValue)
				{
				LogError("vts discard CarNum=%s\n",infoViolation.carInfo.strCarNum);
				return false;
				}*/
				if(g_nDetectMode == 2 && bRetValue )
				{
					vector<CAR_CONTEXT> vtCarContext;
					mvcarnumdetect.get_CarInfo(vtCarContext);

					if(vtCarContext.size() > 0)
					{
						vector<CAR_CONTEXT>::iterator it_b =  vtCarContext.begin();
						if(it_b->carnum != NULL)
						{

							//LogNormal("vts %s-%s\n",infoViolation.carInfo.strCarNum,vehicle_result.carnum);
							//printf("it->carnum=%s\n",it_b->carnum->carnum);
							//infoViolation.carInfo.m_CarWholeRec = vehicle_result.position;

							memcpy(infoViolation.carInfo.strCarNum,it_b->carnum->carnum,7);
							memcpy(infoViolation.carInfo.wj,it_b->carnum->wjcarnum,2);
							string strTemp(infoViolation.carInfo.strCarNum);
							strCarNum = strTemp;
							//CarNumConvert(strCarNum,infoViolation.carInfo.wj);

							infoViolation.carInfo.color      = it_b->carnum->color;

							infoViolation.carInfo.ix      = it_b->carnum->position.x;
							infoViolation.carInfo.iy      = it_b->carnum->position.y;
							infoViolation.carInfo.iwidth  = it_b->carnum->position.width;
							infoViolation.carInfo.iheight = it_b->carnum->position.height;

							infoViolation.carInfo.vehicle_type = it_b->carnum->vehicle_type;
							infoViolation.carInfo.carnumrow =  it_b->carnum->carnumrow;
							bCarNum = true;
						}
						else
						{
							//infoViolation.carInfo.m_CarWholeRec = it_b->position;
							printf("it->carnum====================\n");
						}
					}


					if (strCarNum[0] != '*')
					{
						vector<sVtsPlateCache>::iterator iter = m_vecVtsCachePlate.begin();
						for (;iter != m_vecVtsCachePlate.end();++iter)
						{
							if((iter->strPlate == strCarNum) && (iter->uViolationType == plate.uViolationType))
							{				 
								LogNormal("vts找到相同车牌%s\n",strCarNum.c_str());
								break;
							}
						}
						if ((iter != m_vecVtsCachePlate.end()) &&  (m_vecVtsCachePlate.size() > 0))
						{
							return false;
						}
						else
						{
							sVtsPlateCache tempVtsCache;
							if (m_vecVtsCachePlate.size() < 10)
							{
								tempVtsCache.strPlate = strCarNum;
								tempVtsCache.uViolationType = plate.uViolationType;
								m_vecVtsCachePlate.push_back(tempVtsCache);
							}
							else
							{

								m_vecVtsCachePlate.erase(m_vecVtsCachePlate.begin());
								tempVtsCache.strPlate = strCarNum;
								tempVtsCache.uViolationType = plate.uViolationType;
								m_vecVtsCachePlate.push_back(tempVtsCache);
							}
						}
					}
				}
				
			}
			//车牌号码转换
			CarNumConvert(strCarNum,infoViolation.carInfo.wj);
		}
		//车牌号码
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

		if (g_nServerType == 13 && g_DistanceHostInfo.bDistanceCalculate == 1 && plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
		{
			return false;
		}
		
		#ifndef ALGORITHM_YUV
	    //车辆类型
        plate.uType =  infoViolation.carInfo.vehicle_type;

        //车型细分
        if(m_nDetectKind&DETECT_TRUCK)
        plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);
		#else
		plate.uType =  infoViolation.carInfo.nVehicleType;
		#endif
		
		//车辆类型转换
		CarTypeConvert(plate);

        int nPicCount = infoViolation.nPicCount;//图片数量
        if(g_nVtsPicMode == 3)
        {
           /* if(nPicCount > 2)
            nPicCount = 2;
			
			if(g_nServerType != 3)
			{
				if(1 == g_PicFormatInfo.nSmallViolationPic)
				{
					UINT32 frameSeqs = infoViolation.frameSeqs[0];
					infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
					infoViolation.frameSeqs[1] = frameSeqs;
				}
			}*/
        }
		else if(g_nVtsPicMode == 4)
		{
			nPicCount = 1;
		}

		//超速
		if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
		{
			if (g_nServerType == 23 || g_nServerType == 26)//济南项目
			{	
				UINT32 frameSeqs = infoViolation.frameSeqs[0];
				infoViolation.frameSeqs[0] = infoViolation.frameSeqs[2];
				infoViolation.frameSeqs[2] = frameSeqs;	

				frameSeqs = infoViolation.frameSeqs[0];
				infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
				infoViolation.frameSeqs[1] = frameSeqs;	
			}
			else
			{
				/*if((g_nDetectMode == 2)&&(g_nVtsPicMode == 1)&&(1 != g_PicFormatInfo.nSmallViolationPic))//2x2
				{
					UINT32 frameSeqs = infoViolation.frameSeqs[0];
					infoViolation.frameSeqs[0] = infoViolation.frameSeqs[2];
					infoViolation.frameSeqs[2] = frameSeqs;	
				}*/
			}
		}

        PLATEPOSITION  TimeStamp[6];
		PLATEPOSITION  SignalTimeStamp;

		//小图坐标转换成大图坐标
		if(g_nDetectMode != 2)//原图坐标
		{
			#ifndef ALGORITHM_YUV
			infoViolation.carInfo.ix *= m_fScaleX;
			infoViolation.carInfo.iy *= m_fScaleY;
			infoViolation.carInfo.iwidth *= m_fScaleX;
			infoViolation.carInfo.iheight *= m_fScaleY;
			#endif
		}
		
		//printf("infoViolation.carInfo.nX=%d,%d,%d,%d,rtContour=%d,%d,%d,%d\n",infoViolation.carInfo.nX,infoViolation.carInfo.nY,infoViolation.carInfo.nWidth,infoViolation.carInfo.nHeight,infoViolation.carInfo.rtContour.x,infoViolation.carInfo.rtContour.y,infoViolation.carInfo.rtContour.width,infoViolation.carInfo.rtContour.height);
		TimeStamp[0].uTimestamp = infoViolation.carInfo.ts/1000000;
		//TimeStamp[0].nDirection = infoViolation.carInfo.nDirection;
         printf("===nPicCount=%d,frameSeqs=%d,%d,%d\n",nPicCount,infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);
        if(nPicCount>=1)
        {
			UINT32 frameSeqs = infoViolation.frameSeqs[0] - infoViolation.dis[0];

			if(plate.uViolationType != DETECT_RESULT_RED_LIGHT_VIOLATION)
			{
				frameSeqs = 0;
			}
			else
			{
				printf("s1=%d,s2=%d,s3=%d,d1=%d,dis1=%d,dis2=%d,dis3=%d\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2],infoViolation.frameSeqs[0] - infoViolation.dis[0],infoViolation.dis[0],infoViolation.dis[1],infoViolation.dis[2]);
			}
			
			#ifndef ALGORITHM_YUV
			SignalTimeStamp.x = infoViolation.carInfo.ix;
			SignalTimeStamp.y = infoViolation.carInfo.iy;
			SignalTimeStamp.width = infoViolation.carInfo.iwidth;
			SignalTimeStamp.height = infoViolation.carInfo.iheight;
			SignalTimeStamp.rtCarPos = infoViolation.carInfo.m_CarWholeRec;
			//LogNormal("vts:%d,%d,%d,%d", \
			//	infoViolation.carInfo.ix, infoViolation.carInfo.iy, \
			//	infoViolation.carInfo.iwidth, infoViolation.carInfo.iheight);
			//LogNormal("wholeRec:%d,%d,%d,%d plate:%s", \
			//	infoViolation.carInfo.m_CarWholeRec.x, infoViolation.carInfo.m_CarWholeRec.y, \
			//	infoViolation.carInfo.m_CarWholeRec.width, infoViolation.carInfo.m_CarWholeRec.height,
			//	infoViolation.carInfo.strCarNum);
			#else
			SignalTimeStamp.x = infoViolation.carInfo.nX;
			SignalTimeStamp.y = infoViolation.carInfo.nY;
			SignalTimeStamp.width = infoViolation.carInfo.nWidth;
			SignalTimeStamp.height = infoViolation.carInfo.nHeight;
			#endif

			SignalTimeStamp.nType = plate.uType;
			SignalTimeStamp.IsCarnum = bCarNum;

			//LogNormal("11 infovio :%d,%d,%d,%d type:%d state:%d seq:%d,%d,%d\n", \
				infoViolation.carInfo.ix, infoViolation.carInfo.iy, \
				infoViolation.carInfo.iwidth, infoViolation.carInfo.iheight, \
				infoViolation.evtType, infoViolation.nOutState, infoViolation.frameSeqs[0], infoViolation.frameSeqs[1], infoViolation.frameSeqs[2]);


            bool bRet = false;

		//	LogNormal("f1=%lld,f2=%lld,f3=%lld\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);

			//获取违章检测结果图像
			{
				#ifndef ALGORITHM_YUV
				if(bGetVtsImgByKey)
				{
					Vts_Picture_Key vtsPicKey;
					vtsPicKey.nId = infoViolation.id;
					vtsPicKey.uCameraId = m_nCameraID;	
					if(vtsPicKey.nId >= 0)
					{
						if(infoViolation.nOutState == 0)//0->违章编码；(进缓存队列）
						{
#ifdef DEBUG_VTS
							//LogNormal("m_vtsPicMap.size()=%d\n",m_vtsPicMap.size());
							LogNormal("nOutState0 nId=%d,%d,%d,%d\n",vtsPicKey.nId,infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);
							LogNormal("0 size: %d type:%d plate:%s\n", \
								m_vtsPicMap.size(), infoViolation.evtType, infoViolation.carInfo.strCarNum);
#endif
							for(int iIndex=0; iIndex<nPicCount; iIndex++)
							{
								if(m_vtsPicMap.size() > MAX_VTS_BUF_SIZE*5)
								{
									VtsPicDataMap::iterator it_map = m_vtsPicMap.begin();
									LogNormal("Erase %d nId:%d, uSeq:%d type:%d plate:%s m_vtsPicMap!\n", \
										m_vtsPicMap.size(), it_map->first.nId, it_map->first.uSeq, infoViolation.evtType, infoViolation.carInfo.strCarNum);
									m_vtsPicMap.erase(it_map);
								}
							}
							
							UINT32 nPreSeq = 0;
							for(int iIndex=0; iIndex<nPicCount; iIndex++)
							{
								vtsPicKey.uSeq = infoViolation.frameSeqs[iIndex];
								if(vtsPicKey.uSeq > 0)
								{
									VtsPicDataMap::iterator it_map = m_vtsPicMap.find(vtsPicKey);
									if(it_map == m_vtsPicMap.end())
									{
										GetImageByIndex2(vtsPicKey,nPreSeq);
									}
								}
							}
							
							return false;
						}
						else if(infoViolation.nOutState == 1)//1->认为违章输出；（直接调用输出，并删除缓存）
						{
#ifdef DEBUG_VTS
							LogNormal("nOutState1 nId=%d,%d,%d,%d\n",vtsPicKey.nId,infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);
							LogNormal("1 size: %d type:%d plate:%s\n", \
								m_vtsPicMap.size(), infoViolation.evtType, infoViolation.carInfo.strCarNum);
#endif
							//组合违章图片
							bRet = GetVtsImagByKey(infoViolation,plate.uViolationType,frameSeqs, TimeStamp, &SignalTimeStamp);

							/*for(int iIndex=0; iIndex<nPicCount; iIndex++)
							{
								vtsPicKey.uSeq = infoViolation.frameSeqs[iIndex];
								VtsPicDataMap::iterator it_map = m_vtsPicMap.find(vtsPicKey);
								if(it_map != m_vtsPicMap.end())
								{
									//LogNormal("delete one pic\n");
									m_vtsPicMap.erase(it_map);
								}
							}*/
						}
						else if(infoViolation.nOutState == 2)//2->进行违章删除 （直接删除，不调用输出）
						{
#ifdef DEBUG_VTS
							LogNormal("nOutState2 nId=%d,%d,%d,%d\n",vtsPicKey.nId,infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);
							LogNormal("2 size: %d type:%d plate:%s\n", \
								m_vtsPicMap.size(), infoViolation.evtType, infoViolation.carInfo.strCarNum);
#endif
							for(int iIndex=0; iIndex<nPicCount; iIndex++)
							{
								vtsPicKey.uSeq = infoViolation.frameSeqs[iIndex];
								VtsPicDataMap::iterator it_map = m_vtsPicMap.find(vtsPicKey);
								if(it_map != m_vtsPicMap.end())
								{
									//LogNormal("delete one pic\n");
									m_vtsPicMap.erase(it_map);
								}
							}
							return false;
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
				else
				{
						bRet = GetVtsImageByIndex(infoViolation.frameSeqs,TimeStamp,nPicCount,frameSeqs,&SignalTimeStamp,plate.uViolationType);
				}
				#else//处理yuv方式的违章数据
						//组合违章图片
						bRet = GetVtsImagByKey(infoViolation,plate.uViolationType,frameSeqs, TimeStamp, &SignalTimeStamp);
				#endif

			}


			if(!bRet)
			{
				return false;
			}
        }

		if (g_nServerType == 13 || g_nServerType == 17)
		{
			//经过时间(秒)
			plate.uTime = TimeStamp->uTimestamp;
			//毫秒
			plate.uMiTime = ((TimeStamp->ts)/1000)%1000;
		}
		else
		{
			//经过时间(秒)
			plate.uTime = infoViolation.carInfo.ts/1000000;
			//毫秒
			plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;

			#ifdef NO_SEND_KAKOU_DATA //取第一张图片时间
			{
				//经过时间(秒)
				plate.uTime = TimeStamp->uTimestamp;
				//毫秒
				plate.uMiTime = ((TimeStamp->ts)/1000)%1000;
			}
			#endif
		}

        //地点
		if(m_strLocation.size() >= sizeof(plate.chPlace))
		{
			memcpy(plate.chPlace,m_strLocation.c_str(),sizeof(plate.chPlace));
		}
		else
		{
			memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
		}

		//LogTrace("Direct.txt", "-111-m_nDetectDirection=%d nDi=%d--plate.uDirection=%d", \
			m_nDetectDirection, infoViolation.carInfo.nDirection, plate.uDirection);

        //行驶方向
		{
			plate.uDirection = m_nDirection;
		}

		//LogTrace("Direct.txt", "-222-m_nDetectDirection=%d nDi=%d--plate.uDirection=%d", \
			m_nDetectDirection, infoViolation.carInfo.nDirection, plate.uDirection);
		
		#ifndef ALGORITHM_YUV
        //车牌结构
        plate.uPlateType = infoViolation.carInfo.carnumrow;
        //车牌颜色
        plate.uColor = infoViolation.carInfo.color;
		#else
		//车牌结构
        plate.uPlateType = infoViolation.carInfo.uPlateType;
        //车牌颜色
        plate.uColor = infoViolation.carInfo.nColor;
		#endif


        //车道编号
		if(g_nDetectMode == 2)
		{
			plate.uRoadWayID = infoViolation.nChannel;
		}
		else
		{
			vector<ChannelRegion>::iterator it_b = m_vtsObjectRegion.begin();
			while(it_b != m_vtsObjectRegion.end())
			{
				if(-1 == infoViolation.nChannel)
				{
					plate.uRoadWayID = it_b->nVerRoadIndex;
					break;
				}
				else if(it_b->nRoadIndex == infoViolation.nChannel)
				{
					plate.uRoadWayID = it_b->nVerRoadIndex;
					break;
				}
				it_b++;
			}
		}
		if(plate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION)			//逆行
		{
			if(g_nDetectMode != 2)//1拖N不要转换
			{
				VTSParaMap::iterator it_p = m_vtsObjectParaMap.find(plate.uRoadWayID);
				if(it_p != m_vtsObjectParaMap.end())
				{
					if (it_p->second.nRetrogradeItem == 1)//如果算法报逆行，同时客户端选择了违反禁令标志，我们就报违反禁令标志
					{
						plate.uViolationType = DETECT_RESULT_RETROGRADE_MOTION_FLAG;
					}
					else if (it_p->second.nRetrogradeItem == 2)//如果算法报逆行，同时客户端选择了不按导向车道行驶，我们就报不按导向车道行驶
					{
						plate.uViolationType = DETECT_NO_DIRECTION_TRAVEL;
					}
					else
					{
						plate.uViolationType = DETECT_RESULT_RETROGRADE_MOTION;
					}
				}
			}
		}
        //车速
		#ifndef ALGORITHM_YUV
        double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
		#else
		 double dSpeed =   infoViolation.carInfo.fVelocity;
		#endif
        plate.uSpeed = (UINT32)(dSpeed+0.5);

		//超速
		if(plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
		{
			
			int nOnlyOverSpedMax = 10;
			{
				paraDetectList::iterator it_p = m_roadParamInlist.begin();
				while( it_p !=  m_roadParamInlist.end() )
				{

					//LogNormal("it_p->nChannelID=%d,it_p->m_nOnlyOverSpedMax=%.2f",it_p->nChannelID,it_p->m_nOnlyOverSpedMax);
					if(it_p->nChannelID == plate.uRoadWayID)
					{
						nOnlyOverSpedMax = it_p->m_nOnlyOverSpedMax;
						if(nOnlyOverSpedMax == 0)
						{
							if(plate.uType == SMALL_CAR)
							{
								nOnlyOverSpedMax = it_p->m_nAvgSpeedMin;
							}
							else if(plate.uType == BIG_CAR)
							{
								nOnlyOverSpedMax = it_p->m_nAvgSpeedMax;
							}
						}
					}
					it_p++;
				}
			}
			plate.uLimitSpeed = nOnlyOverSpedMax;
			plate.uOverSpeed = nOnlyOverSpedMax+10;//起拍值在限速值上面加10

			if (g_nServerType == 23 || g_nServerType == 26)
			{
				if (m_vtsGlobalPara.nSpeedVal > 0)
				{
					plate.uOverSpeed = m_vtsGlobalPara.nSpeedVal;
				}
				else
				{
					plate.uOverSpeed = nOnlyOverSpedMax+5;//起拍值在限速值上面加5
				}

				if (plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
				{
					if( plate.uSpeed < plate.uOverSpeed)
					{
						//LogNormal("uSpeed=%d,uOverSpeed=%d\n",plate.uSpeed,plate.uOverSpeed);
						return false;
					}
				}
			}
		}


		
			//发生位置(在当前图片上的)
		#ifndef ALGORITHM_YUV
			plate.uPosLeft  = infoViolation.carInfo.ix;
			plate.uPosTop   = infoViolation.carInfo.iy*m_nDeinterlace;
			plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
			plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight)*m_nDeinterlace;

			unsigned int uSeq = infoViolation.carInfo.uSeq;
		#else
			plate.uPosLeft  = infoViolation.carInfo.nX;
			plate.uPosTop   = infoViolation.carInfo.nY;
			plate.uPosRight = infoViolation.carInfo.nX+infoViolation.carInfo.nWidth;
			plate.uPosBottom  = (infoViolation.carInfo.nY+infoViolation.carInfo.nHeight);

			unsigned int uSeq = infoViolation.carInfo.nFrame;
		#endif

			//LogNormal("TyD-%d-%d-%d-%d\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2],infoViolation.frameSeqs[3]);
			//济南黄网格项目,针对非大货车禁行,不输出
			if(g_nDetectMode == 2)
			{
				//LogNormal("id:%d plate.uViolationType:%d ", m_nChannelId, plate.uViolationType);
				//DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD == plate.uViolationType ||
				if((DETECT_RESULT_BIG_IN_FORBIDDEN_TIME == plate.uViolationType ||
					(DETECT_RESULT_OBV_TAKE_UP_BUSWAY == plate.uViolationType && plate.uColor == CARNUM_YELLOW))&&(m_nDetectKind&DETECT_TRUCK))//是大车禁止行则处理
				{
					bool bOutPutVts = false;//是否输出标记
					int nTypeDetail = -1; //车型细分

					carnum_context context;
					context.position.x = infoViolation.carInfo.ix;
					context.position.y = infoViolation.carInfo.iy;
					context.position.width = infoViolation.carInfo.iwidth;
					context.position.height = infoViolation.carInfo.iheight;

					context.vehicle_type = infoViolation.carInfo.vehicle_type;
					context.color = (CARNUM_COLOR)infoViolation.carInfo.color;
					context.mean  = infoViolation.carInfo.mean;
					context.stddev = infoViolation.carInfo.stddev;

					context.VerticalTheta  = infoViolation.carInfo.VerticalTheta;
					context.HorizontalTheta = infoViolation.carInfo.HorizontalTheta;
					context.carnumrow = (CARNUM_ROW)infoViolation.carInfo.carnumrow;

					memcpy(context.carnum,infoViolation.carInfo.strCarNum,sizeof(context.carnum));

					context.smearnum = infoViolation.carInfo.smearCount;
					memcpy(context.smearrect,infoViolation.carInfo.smear,sizeof(CvRect)*(infoViolation.carInfo.smearCount));

					context.nCarNumDirection = (carnumdirection)m_nDetectDirection;
				
					
					//LogNormal("before GetTypeDetailTwo \n");
					//LogNormal("VTS Pos:%d,%d,%d,%d \n", \
					//	infoViolation.carInfo.ix, infoViolation.carInfo.iy,\
					//	infoViolation.carInfo.iwidth, infoViolation.carInfo.iwidth);
					//取卡口对应帧,infoViolation.frameSeqs[3],进行找图确认二次识别车型细分

					PLATEPOSITION  TimeStampTmp[6];
					nTypeDetail = GetTypeDetailTwo(infoViolation.frameSeqs[3], infoViolation.carInfo, plate, context, TimeStampTmp);
					LogNormal("after GetTypeDetailTwo nTypeDetail:%d[%s]\n",nTypeDetail,plate.chText);
					
					//针对车型细分结果进行处理
					bOutPutVts = DealVtsTypeDetail(nTypeDetail);

					if(!bOutPutVts)
					{
						if (DETECT_RESULT_OBV_TAKE_UP_BUSWAY == plate.uViolationType)
						{
							LogError("discard invaild %s,%d\n",plate.chText,nTypeDetail);
						}
						return false;
					}
					else
					{
						if (DETECT_RESULT_OBV_TAKE_UP_BUSWAY == plate.uViolationType)
						{
							LogError("discard invaild22 %s,%d\n",plate.chText,nTypeDetail);
							return false;
						}
						plate.uViolationType = DETECT_RESULT_BIG_IN_FORBIDDEN_TIME;//大货车禁行
						plate.uTypeDetail = 0;//车型细分不报
					}
				}			
			}
			
			//获取录像以及图片路径
			 pthread_mutex_lock(&g_Id_Mutex);
			 //获取录像路径
			if(sHeader.uSeq == 0)
			{
			   sHeader.uSeq = uSeq;
			   GetVideoSavePath(plate,sHeader);
			}
			 //获取图片路径
			std::string strPicPath,strPicPath2,strPicPath3;
			////////////////////
			int nSaveRet = GetVtsPicSavePath(plate,nPicCount,strPicPath,strPicPath2,strPicPath3);

			pthread_mutex_unlock(&g_Id_Mutex);
			//删除已经存在的记录
			g_skpDB.DeleteOldRecord(strPicPath,false,false);


			//事件发生位置
			if(m_bEventCapture)
			{
				RECORD_EVENT event;
				float fscaleX = 1.0;
				float fscaleY = 1.0;
				if(m_imgSnap->width > 2000)
				{
					fscaleX = 6;
					fscaleY = 6;
				}
				else if(m_imgSnap->width > 1000)
				{
					fscaleX = 4;
					fscaleY = 4;
				}
				event.uPosX = (plate.uPosLeft+plate.uPosRight)*0.5/fscaleX;
				event.uPosY = (plate.uPosTop+plate.uPosBottom)*0.5/fscaleY;
				memcpy(event.chVideoPath,plate.chVideoPath,sizeof(plate.chVideoPath));

				//获取事件时间,通过第1帧时间和帧率计算.
				if(g_nDetectMode == 2) 
				{
					GetEventBeginTime(infoViolation, event, vtsStamp);
				}

				event.uChannelID = m_nChannelId;
				memcpy(event.chText, plate.chText, sizeof(plate.chText));
				strEvent.append((char*)&event,sizeof(RECORD_EVENT));
			}
			
			//闯红灯
			if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
			{
				UINT32 uTimestamp = (infoViolation.redLightStartTime/1000)/1000;
				if(infoViolation.redLightStartTime <= 0)
				{
					uTimestamp = SignalTimeStamp.uTimestamp;
				}
				//红灯开始时间
				plate.uRedLightBeginTime = uTimestamp;
				int nMiTime = (infoViolation.redLightStartTime/1000)%1000;
				plate.uRedLightBeginMiTime = nMiTime;

				//红灯结束时间
				plate.uRedLightEndTime = plate.uRedLightBeginTime+m_vtsGlobalPara.nRedLightTime;
				plate.uRedLightEndMiTime = nMiTime;

				uTimestamp = SignalTimeStamp.uTimestamp - uTimestamp;
				if(uTimestamp > 600)
				{
					uTimestamp = 10;
				}

				//红灯时间
				plate.uSignalTime = uTimestamp;
			}

			if(nPicCount >= 1 && (g_nDoSynProcess != 1 || m_nDetectDirection == 0))
			{		
				if(m_bImageEnhance)
				{
					ImageEnhance(m_imgComposeSnap);
				}
				//存储违章图像
				SaveVtsImage(plate,nPicCount,TimeStamp,SignalTimeStamp,infoViolation.redLightStartTime,strPicPath2,strPicPath3);
			}

			//保存闯红灯记录
			if(nSaveRet>0 && (g_nDoSynProcess != 1 || m_nDetectDirection == 0))
			{
				g_skpDB.SavePlate(m_nChannelId,plate,0);

				if(7 == g_nServerType)
				{
					UINT32 uSignalBeginTime = 0;
					UINT32 uSignalEndTime = 0;

					if(infoViolation.redLightStartTime <= 0)
					{
						uSignalBeginTime = SignalTimeStamp.uTimestamp;
					}
					else
					{
						uSignalBeginTime = (infoViolation.redLightStartTime/1000)/1000;
					}
					uSignalEndTime = uSignalBeginTime+m_vtsGlobalPara.nRedLightTime;

					int nMiTime = (infoViolation.redLightStartTime/1000)%1000;

					g_RoadImcData.AddViolationData(plate,strPicPath,uSignalBeginTime,uSignalEndTime,nMiTime);
				}
			}

			//加入结果比对队列
			if((g_nDoSynProcess == 1)&&(plate.chText[0] != '*') &&(m_nDetectDirection == 1))
			{
				//LogNormal("uDirection:%d, vioDir:%d", plate.uDirection, infoViolation.carInfo.nDirection);
				//车牌号码				
				memcpy(plate.chText, infoViolation.carInfo.strCarNum, 7);
				memcpy(plate.chText + 7, infoViolation.carInfo.wj, 2);				

				plate.uCarPosLeft = infoViolation.carInfo.m_CarWholeRec.x;
				plate.uCarPosTop = infoViolation.carInfo.m_CarWholeRec.y;
				plate.uCarPosRight = plate.uCarPosLeft + infoViolation.carInfo.m_CarWholeRec.width;
				plate.uCarPosBottom = plate.uCarPosTop + infoViolation.carInfo.m_CarWholeRec.height;

				plate.uDetectDirection = m_nDetectDirection;
				//LogNormal("AddDspPlateMatchVts11 dir:%d ", plate.uDirection);
				
				plate.uSeqID = infoViolation.carInfo.uSeq;

				//if(m_pMachPlate->bUse)
				{
					m_pMachPlate->dspRecord.uChannelID = m_nChannelId;
					AddDspPlateMatchVts(plate, m_pMachPlate);
					//m_pMachPlate->bUse = false;//使用完更新状态
				}
			}

        //将车牌信息送客户端
        if(m_bConnect && (g_nDoSynProcess != 1 || m_nDetectDirection == 0))
        {
			//车牌号码
			memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
			SendResult(plate,uSeq);
        }

	    return true;
}

//从jpg map中寻找一个最接近的图片
bool CRoadCarnumDetect::GetVtsPicByKeyFromMap(Vts_Picture_Key pickeyDst, string& strPic)
{
	bool bFind = false;

	Vts_Picture_Key pickeyCur;
	VtsPicDataMap::iterator it = m_vtsPicMap.find(pickeyDst);

	if(it != m_vtsPicMap.end())
	{		
		strPic.append((char*)it->second.c_str(),it->second.size());
		bFind = true;	

#ifdef OUT_STATE_LOG
		LogTrace("OutState.txt", "==GetVtsPicByKeyFromMap==ok=uSeq =%d ", pickeyDst.uSeq);
#endif
	}//End if

	return bFind;
}

//根据帧号查找出对应的图片
int CRoadCarnumDetect::GetImageByIndex2(Vts_Picture_Key pickey,UINT32& nPreSeq)
{
	UINT32 nSeq = pickey.uSeq;
	//printf("=GetImageByIndex2==1111111\n");
	bool bFind = false;
	int nPicSize = 0;

	int index = 0;
	yuv_video_buf* buf = NULL;

	//加锁
	pthread_mutex_lock(&m_PlateMutex);

	//暂存seq->index对应关系
	SeqIndexMap mapSeqIndex;
	for(int j = 0;j<m_nMaxPlateSize;j++)
	{
		yuv_video_buf* pBuf = (yuv_video_buf*)(m_BufPlate[j].pCurBuffer);
		if(pBuf!=NULL)
		{
			mapSeqIndex.insert(SeqIndexMap::value_type(pBuf->nSeq,j));
		}
	}

	if(mapSeqIndex.size()<=1)
	{
		pthread_mutex_unlock(&m_PlateMutex);
		return;
	}

	//printf("=GetImageByIndex2==222222222\n");
	SeqIndexMap::iterator it_b = mapSeqIndex.begin();
	SeqIndexMap::iterator it_e = --mapSeqIndex.end();
	UINT32 nMinSeq = it_b->first; //最小帧号
	UINT32 nMaxSeq = it_e->first;//最大帧号

	//LogNormal("Seq=%lld,MinSeq=%lld,MaxSeq=%lld,PreSeq=%lld",nSeq,nMinSeq,nMaxSeq,nPreSeq);

	if(nSeq > nMaxSeq)
	{
		//LogNormal("大于最大帧号bLightImage=%d,nSeq=%lld,nMaxSeq=%lld,nPreSeq=%lld",bLightImage,nSeq,nMaxSeq,nPreSeq);
		//LogTrace("OutState.txt","大于最大帧号bLightImage=%d,nSeq=%lld,nMaxSeq=%lld,nPreSeq=%lld",bLightImage,nSeq,nMaxSeq,nPreSeq);
		nSeq = nMaxSeq;
	}

	bool bFindImageFromResultFrameList = false;	

	SeqIndexMap::iterator it_map = mapSeqIndex.find(nSeq);
	if(it_map!=mapSeqIndex.end())
	{
		index = it_map->second;
		bFind = true;
	}
	else
	{
#ifdef OUT_STATE_LOG
		LogTrace("OutState.txt", "=11=未找到对应帧号图片Seq = %d\n", nSeq);
#endif

		if(nSeq < nMinSeq)
		{
			bFind = false;
			//LogTrace("OutState.txt", "===bFindImageFromResultFrameList=true==uSeq=%d !", uSeq);
			//去秒图中寻找
			bFindImageFromResultFrameList = true;
		}
		else
		{
			index = GetSeqIndexFromMap2(nSeq,nPreSeq,mapSeqIndex);
			bFind = true;
		}
	}	

	//printf("=GetImageByIndex2==44444444\n");
	if(!bFindImageFromResultFrameList)
	{
		buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);
		if(buf)
		{
			#ifndef ALGORITHM_YUV_CARNUM
			//缓存jpg图片
			CxImage image;
			int srcstep = 0;
			if(image.IppEncode((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)),m_imgComposeResult->width,m_imgComposeResult->height,3,&srcstep,m_pCurJpgImage,g_PicFormatInfo.nJpgQuality))
			{
				string strPic;
				strPic.append((char*)m_BufPlate[index].pCurBuffer,sizeof(yuv_video_buf));
				strPic.append((char*)m_pCurJpgImage,srcstep);
				m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
			}
			#else
			//缓存yuv图片
			string strPic;
			strPic.append((char*)m_BufPlate[index].pCurBuffer,sizeof(yuv_video_buf)+(m_imgResult->width*2*m_imgResult->height));
			m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
			#endif

			printf("=22222222==buf->nSeq=%d===\n", buf->nSeq);

			nPicSize = buf->size;
			nPreSeq = buf->nSeq;

			printf("=YU 111111111===nPicSize=%d==\n", nPicSize);
		}		
	}
	else
	{
		bool bFind = false;
		yuv_video_buf bufResult;

		{
			//printf("=GetImageByIndex2==55555555555555\n");
			bFind = GetImageFromResultFrameList2(pickey,nPreSeq,bufResult);

			if(bFind)
			{
				if( abs((long long)nMinSeq - (long long)nSeq) < abs((long long)bufResult.nSeq - (long long)nSeq) )//取帧图像缓冲区最后一个
				{
					it_map = mapSeqIndex.find(nMinSeq);
					index = it_map->second;

					yuv_video_buf* buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);
		
					#ifndef ALGORITHM_YUV_CARNUM
					//缓存jpg图片
					CxImage image;
					int srcstep = 0;
					if(image.IppEncode((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)),m_imgComposeResult->width,m_imgComposeResult->height,3,&srcstep,m_pCurJpgImage,g_PicFormatInfo.nJpgQuality))
					{
						string strPic;
						strPic.append((char*)m_BufPlate[index].pCurBuffer,sizeof(yuv_video_buf));
						strPic.append((char*)m_pCurJpgImage,srcstep);
						m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
					}
					#else
					//缓存yuv图片
					string strPic;
					strPic.append((char*)m_BufPlate[index].pCurBuffer,sizeof(yuv_video_buf)+(m_imgResult->width*2*m_imgResult->height));
					m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
					#endif

					nPreSeq = nMinSeq;
					bufResult.nSeq = nMinSeq;
					nPicSize = buf->size;

					printf("=YU 222222222===nPicSize=%d==", nPicSize);
				}
				else
				{
					nPreSeq = bufResult.nSeq;
					nPicSize = bufResult.size;

	#ifdef OUT_STATE_LOG
					LogTrace("OutState.txt","----4444---==nSeq:%d=bufResult.nSeq=%lld \n", nSeq, bufResult.nSeq);	
	#endif
				}	
			}//End if(bFind)
			else
			{
				LogNormal("----Not find nSeq:%d!\n", nSeq);
				nPicSize = 0;
			}					
		}
	}

	//解锁
	pthread_mutex_unlock(&m_PlateMutex);

	return nPicSize;
}

/*
* 函数说明：从vtsMap缓冲中，获取图片
* input: infoViolation: 违章信息，uViolationType：违章类型, SignalframeSeq:标志红灯信号帧
* output: pTimeStamp: 时间戳　 pSignalTimeStamp:红灯信号时间戳
* 					m_imgComposeResult: 违章输出结果图像合成，类成员变量
*					m_imgComposeSnap：违章输出结果图片转换中间变量
* 返回值：获取图片，是否成功．
*/
bool CRoadCarnumDetect::GetVtsImagByKey(ViolationInfo infoViolation, UINT32 uViolationType, UINT32 SignalframeSeq, \
				PLATEPOSITION* pTimeStamp, PLATEPOSITION* pSignalTimeStamp)
{
	bool bGetVtsImag = false;

	cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));

	
	//从vtsMap缓冲中，获取图片
	std::string strPic;
	Vts_Picture_Key vtspickey;
	 #ifndef ALGORITHM_YUV
	vtspickey.nId = infoViolation.id;
	#endif
	vtspickey.uCameraId = m_nCameraID;
	
	bool bGetOnePic = false;
	int nPicCount = infoViolation.nPicCount;
	UINT32 uSeq = 0;

	pthread_mutex_lock(&m_PlateMutex);


	//暂存seq->index对应关系
    SeqIndexMap mapSeqIndex;
    for(int j = 0; j<m_nMaxPlateSize; j++)
    {
        yuv_video_buf* buf1 = (yuv_video_buf*)(m_BufPlate[j].pCurBuffer);

        if(buf1!=NULL)
        {
            mapSeqIndex.insert(SeqIndexMap::value_type(buf1->nSeq,j));
        }
    }

    SeqIndexMap::iterator it_b = mapSeqIndex.begin();
    SeqIndexMap::iterator it_e = --mapSeqIndex.end();
    UINT32 nMinSeq = it_b->first;
    UINT32 nMaxSeq = it_e->first;

	UINT32 nPreSeq = 0;
	
	for(int i=0; i<nPicCount; i++)
	{
		strPic.clear();
		vtspickey.uSeq = infoViolation.frameSeqs[i];

		bGetOnePic = GetVtsPicByKeyFromMap(vtspickey, strPic); //Get one pic from vtspicMap
		
		UINT16 uSignalColor = 0;
		unsigned long uSignalSeq = 0;

		if(bGetOnePic && strPic.size() > 100)
		{
			#ifndef ALGORITHM_YUV_CARNUM
			//需要解码jpg图像
			 CxImage image;
             image.Decode((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)), strPic.size() - sizeof(yuv_video_buf), 3); //解码

			 if(image.IsValid()&&image.GetSize()>0)
			 {
				 printf("before ======memcpy(m_imgComposeResult->imageData,image.GetSrcSize()=%d\n",image.GetSrcSize());
				memcpy(m_imgComposeResult->imageData,image.GetBits(),image.GetSrcSize());
				printf("after ======memcpy(m_imgComposeResult->imageData,image.GetSize()=%d\n",image.GetSize());
			 }
			#else
			cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
			if(m_nWordPos == 0)
			{
				ConvertYUVtoRGBImage((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
			}
			else
			{
				ConvertYUVtoRGBImage((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
			}
			#endif
			yuv_video_buf* buf = (yuv_video_buf*)(strPic.c_str());
			(pTimeStamp+i)->ts = buf->ts;
			(pTimeStamp+i)->uTimestamp = buf->uTimestamp;
			(pTimeStamp+i)->uSeq = buf->nSeq;
			nPreSeq = buf->nSeq;
			bGetVtsImag = true;

			uSignalColor = buf->uTrafficSignal;
			uSignalSeq = buf->nSeq;
		}
		else
		{
			bGetVtsImag = false;
			 //解锁
			pthread_mutex_unlock(&m_PlateMutex);
			LogError("无法找到对应违章图片不输出\n");
			return false;
			//去帧图像缓冲区查找
			//LogError("GetVtsPicByKeyFromMap nId=%d,uSeq=%u\n",vtspickey.nId,vtspickey.uSeq);
			uSeq = vtspickey.uSeq;
			if(uSeq > nMaxSeq)
			{
				uSeq = nMaxSeq;
			}
			else if(uSeq < nMinSeq)
			{
				uSeq = nMinSeq;
			}
			//根据帧号找到对应的图像序号
			SeqIndexMap::iterator it_map;
			int index = 0;
			it_map = mapSeqIndex.find(uSeq);
			if(it_map!=mapSeqIndex.end())
			{
				index = it_map->second;
				yuv_video_buf* buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);
				(pTimeStamp+i)->ts = buf->ts;
				(pTimeStamp+i)->uTimestamp = buf->uTimestamp;
				(pTimeStamp+i)->uSeq = buf->nSeq;
				#ifndef ALGORITHM_YUV_CARNUM
				 printf("before ======memcpy(m_imgComposeResult->imageData,m_imgComposeResult->imageSize=%d\n",m_imgComposeResult->imageSize);
				memcpy(m_imgComposeResult->imageData,m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf),m_imgComposeResult->imageSize);
				 printf("after ======memcpy(m_imgComposeResult->imageData,m_imgComposeResult->imageSize=%d\n",m_imgComposeResult->imageSize);
				#else
				cvSet(m_imgComposeResult, cvScalar( 0,0, 0 ));
				if(m_nWordPos == 0)
				{
					ConvertYUVtoRGBImage((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)),(BYTE*)m_imgComposeResult->imageData,m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
				}
				else
				{
					ConvertYUVtoRGBImage((BYTE*)(m_BufPlate[index].pCurBuffer+sizeof(yuv_video_buf)+m_imgComposeResult->width*2*m_nExtentHeight),(BYTE*)(m_imgComposeResult->imageData+m_imgComposeResult->width*3*m_nExtentHeight),m_imgComposeResult->width,m_imgComposeResult->height-m_nExtentHeight);
				}
				#endif
				nPreSeq = buf->nSeq;
			}
		}

        CvRect rect;
        int height = m_imgComposeResult->height;
        if(m_nWordOnPic == 1)
        {
            height -= m_nExtentHeight;
        }

        if(m_imgComposeSnap->width == m_imgComposeResult->width)
        {
            rect.x = 0;
            rect.y = i*height;
        }
		else if(m_imgComposeSnap->width > 2*m_imgComposeResult->width)
		{
			rect.x = i*m_imgComposeResult->width;
			rect.y = 0;
		}
        else
        {
				rect.x = (i%2)*m_imgComposeResult->width;
				rect.y = (i/2)*height;
        }
        rect.width = m_imgComposeResult->width;
        rect.height = height;

        CvRect rt;
        rt.x = 0;
        rt.y = 0;
        rt.width = m_imgComposeResult->width;
        rt.height = height;

		if((m_nWordPos == 1)&&(m_nWordOnPic == 1))
		{
			rt.y += m_nExtentHeight;
		}

		if(g_nVtsPicMode == 5)
		{
			if (g_nServerType != 23)
			{
				rect.y = m_imgComposeSnap->height - m_imgComposeResult->height;
			}
		}

		if (1 == g_PicFormatInfo.nSmallViolationPic && (1 == g_nVtsPicMode || 5 == g_nVtsPicMode)) //违章图片加小图 并且 2x2叠加图
		{
			//逆行 压线 取第3张图,其他违章取第1张图片
			int picIndex = 0;
			if (DETECT_RESULT_RETROGRADE_MOTION == uViolationType) //逆行
			{
				picIndex = 2;
			}

			if (i == picIndex)
			{
				//存小图
				RECORD_PLATE plate_r;
				plate_r.uPosLeft = pSignalTimeStamp->x;
				plate_r.uPosTop = pSignalTimeStamp->y;
				plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
				plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;
				plate_r.uType = pSignalTimeStamp->nType;
				if(!pSignalTimeStamp->IsCarnum)
				{
					plate_r.chText[0] = '*';
				}

				CvRect srcRt,dstRt;
				if (g_nServerType == 15 || g_nServerType == 25)
				{
					dstRt.x = m_imgComposeResult->width;
					dstRt.y = height;
				}
				else
				{
					dstRt.x = 0;
					dstRt.y = 0;
				}
				dstRt.width = rt.width;
				dstRt.height = rt.height;

				if(m_nWordPos == 1 && m_nWordOnPic == 0) //文字在上 && 文字不叠加
				{
					dstRt.y += m_nExtentHeight;
					dstRt.height -= m_nExtentHeight;
				}
				else if(m_nWordPos == 0 && m_nWordOnPic == 0)
				{
					dstRt.height -= m_nExtentHeight;
				}

				if(g_nServerType == 14)
				{
					dstRt.x = m_imgSnap->width;
					dstRt.y = m_imgSnap->height-m_nExtentHeight+120;
					dstRt.width = m_imgSnap->width;
					dstRt.height = m_imgSnap->height-m_nExtentHeight;
				}

				//遵义图片格式
				if(g_nVtsPicMode == 5)
				{
					if (g_nServerType == 23 || g_nServerType == 26)
					{
						dstRt.x = m_imgSnap->width*3;
					}
					else
					{
						dstRt.x = m_imgSnap->width*3;
						dstRt.y = m_imgComposeSnap->height - m_imgComposeResult->height+m_nExtentHeight;
						dstRt.height = m_imgComposeResult->height-m_nExtentHeight;
					}
				}

				//取车身位置
				srcRt = GetCarPos(plate_r,2);
				//按目标区域比例裁剪
				srcRt.width = srcRt.height * dstRt.width/dstRt.height;

				if(srcRt.x + srcRt.width >= dstRt.width)
				{
					srcRt.x = dstRt.width - srcRt.width-1;
				}

				//LogNormal("d x=%d,y=%d,w=%d,h=%d\n",dstRt.x,dstRt.y,dstRt.width,dstRt.height);
				//LogNormal("s x=%d,y=%d,w=%d,h=%d\n",srcRt.x,srcRt.y,srcRt.width,srcRt.height);
				if(g_nVtsPicMode != 5)
				{
					//有违章小图 并且 大图2x2叠加时 ，中间留黑条
					if((g_nServerType != 13) && (g_nServerType != 14) && (g_nServerType != 15) && (g_nServerType != 25))
					dstRt.width -= 2;
				}

				cvSetImageROI(m_imgComposeResult,srcRt);
				cvSetImageROI(m_imgComposeSnap,dstRt);
				cvResize(m_imgComposeResult, m_imgComposeSnap);
				cvResetImageROI(m_imgComposeSnap);
				cvResetImageROI(m_imgComposeResult);
			}

			printf("=======GetVtsImagByKey==555555===\n");
			if (g_nServerType != 15 && g_nServerType != 25)
			{
				if(g_nVtsPicMode != 5)
				{
					if(g_nServerType != 14)
					{
						if (i%2 == 0)
						{
							rect.x += rect.width;
						}
						else
						{
							rect.x -= rect.width;
							rect.y += rect.height;
						}
					}

					if((g_nServerType != 13) && (g_nServerType != 14))
					{
						//有违章小图 并且 大图2x2叠加时 ，中间留黑条
						if (i%2 == 0)
						{
							rect.x += 2;
							rect.width -= 2;
						}
						else
						{
							rect.width -= 2;
						}
					}
				}
			}
		
		}

		//武汉格式(1x2加小图)
		if(1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode)
		{
			//需要取小图
			if(i == 0)
			{
				rect.x = 0;
				rect.y = 360;//60个像素
				rect.width = 800;
				rect.height = 800;

				//存小图
				RECORD_PLATE plate_r;
				plate_r.uPosLeft = pSignalTimeStamp->x;
				plate_r.uPosTop = pSignalTimeStamp->y;
				plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
				plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;

				rt = GetCarPos(plate_r,1);

				cvSetImageROI(m_imgComposeResult,rt);
				cvSetImageROI(m_imgComposeSnap,rect);
				cvResize(m_imgComposeResult, m_imgComposeSnap);
				cvResetImageROI(m_imgComposeSnap);
				cvResetImageROI(m_imgComposeResult);
			}

			rt.x = 0;
			rt.y = 0;
			rt.width = m_imgComposeResult->width;
			rt.height = m_imgComposeResult->height-m_nExtentHeight;

			rect.x = (i)*400;
			rect.y = m_nExtentHeight;//60个像素
			rect.width = 400;
			rect.height = 300;
		}


		//深圳北环格式
		if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
		{
				//存小图
				RECORD_PLATE plate_r;
				plate_r.uPosLeft = pSignalTimeStamp->x;
				plate_r.uPosTop = pSignalTimeStamp->y;
				plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
				plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;
				plate_r.uType = pSignalTimeStamp->nType;
				if(!pSignalTimeStamp->IsCarnum)
				{
					plate_r.chText[0] = '*';
				}

				CvRect srcRt,dstRt;
				dstRt.x = m_imgSnap->width;
				dstRt.y = 0;
				dstRt.width = rt.width;
				dstRt.height = rt.height;

				if(m_nWordPos == 1 && m_nWordOnPic == 0) //文字在上 && 文字不叠加
				{
					dstRt.y += m_nExtentHeight;
					dstRt.height -= m_nExtentHeight;
				}
				else if(m_nWordPos == 0 && m_nWordOnPic == 0)
				{
					dstRt.height -= m_nExtentHeight;
				}

				//取车身位置
				srcRt = GetCarPos(plate_r,0);
				//按目标区域比例裁剪
				srcRt.width = srcRt.height * dstRt.width/dstRt.height;

				if(srcRt.x + srcRt.width >= dstRt.width)
				{
					srcRt.x = dstRt.width - srcRt.width-1;
				}

				cvSetImageROI(m_imgComposeResult,srcRt);
				cvSetImageROI(m_imgComposeSnap,dstRt);
				cvResize(m_imgComposeResult, m_imgComposeSnap);
				cvResetImageROI(m_imgComposeSnap);
				cvResetImageROI(m_imgComposeResult);
		}


		if(g_nServerType == 14) //北京公交项目中心端
		{
			rect.y = 120+(i/2)*(m_imgSnap->height - m_nExtentHeight);
			rect.height = m_imgSnap->height - m_nExtentHeight;

			if(m_nWordPos == 1)
			{
				rt.y = m_nExtentHeight;
			}
			else
			{
				rt.y = 0;
			}
			rt.height = m_imgSnap->height - m_nExtentHeight;
		}
		
		printf("=======GetVtsImagByKey==666666666666===\n");
		//LogNormal("rt.y=%d,rt.height=%d,rect.y=%d,rect.height=%d\n",rt.y,rt.height,rect.y,rect.height);
		//红绿灯增强
		#ifdef REDADJUST
		#ifndef ALGORITHM_YUV
		RedLightAdjust(m_imgComposeResult, true, uSignalColor, uSignalSeq);
		#endif
		#endif

#ifdef WEN_LING_VTS_TEST
		if(i>0)
		{
			//LogNormal("i:%d, rect.y:%d,m_nExtentHeight:%d", i, rect.y, m_nExtentHeight );
			//rt.x = ;
			rect.y -= m_nExtentHeight;
		}		
#endif //#ifdef WEN_LING_VTS_TEST

		cvSetImageROI(m_imgComposeResult,rt);
		cvSetImageROI(m_imgComposeSnap,rect);
		cvResize(m_imgComposeResult, m_imgComposeSnap);
		cvResetImageROI(m_imgComposeSnap);
		cvResetImageROI(m_imgComposeResult);

		if((g_nServerType == 7) &&(i==0) &&(DETECT_RESULT_NO_PARKING == uViolationType))//天津黄网格停车叠加车牌小图
		{
			//存小图
			RECORD_PLATE plate_r;
			plate_r.uPosLeft = pSignalTimeStamp->x;
			plate_r.uPosTop = pSignalTimeStamp->y;
			plate_r.uPosRight = pSignalTimeStamp->x+pSignalTimeStamp->width-1;
			plate_r.uPosBottom = pSignalTimeStamp->y+pSignalTimeStamp->height-1;
			plate_r.uType = pSignalTimeStamp->nType;
			if(!pSignalTimeStamp->IsCarnum)
			{
				plate_r.chText[0] = '*';
			}
			CvRect rtCarnum = GetCarPos(plate_r,4);
			//LogNormal("rtCarnum %d,%d,%d,%d\n",rtCarnum.x,rtCarnum.y,rtCarnum.width,rtCarnum.height);

			//车牌放在左下角显示
			rect.x = 0;
			rect.y = m_imgComposeSnap->height/3-rtCarnum.height;
			rect.width = rtCarnum.width;
			rect.height = rtCarnum.height;

			cvSetImageROI(m_imgComposeResult,rtCarnum);
			cvSetImageROI(m_imgComposeSnap,rect);
			cvResize(m_imgComposeResult, m_imgComposeSnap);
			cvResetImageROI(m_imgComposeSnap);
			cvResetImageROI(m_imgComposeResult);
		}
    }//End for


    //获取红灯时间
	if( SignalframeSeq > 0 )
	{
		vtspickey.uSeq = SignalframeSeq;
		bool bGetOneSignalPic = GetVtsPicByKeyFromMap(vtspickey, strPic); //Get one pic from vtspicMap
		
		if(bGetOneSignalPic)
		{
			yuv_video_buf* buf = (yuv_video_buf*)(strPic.c_str());
			if(buf)
			{
				pSignalTimeStamp->ts = buf->ts;
				pSignalTimeStamp->uTimestamp = buf->uTimestamp;
			}
		}
		else
		{
			//LogNormal("22===未找到红灯时间\n");
			pSignalTimeStamp->ts = 0;
			pSignalTimeStamp->uTimestamp = 0;
		}
	}

    //解锁
    pthread_mutex_unlock(&m_PlateMutex);

    return bGetVtsImag;
}

//存图测试
void CRoadCarnumDetect::SaveImgTest(IplImage *pImg,const int nData)
{
	static int nId = 0;
	BYTE* pOutImage = NULL;
	pOutImage = new BYTE[pImg->width * pImg->height / 4];

	struct timeval tv;
	gettimeofday(&tv,NULL);

	char jpg_name[256] = {0};
	sprintf(jpg_name, "./text/TT_%d_%d_%d_TT_%d_%lld_%d-seq.jpg", \
		nId, pImg->width, pImg->height, nData, tv.tv_sec,tv.tv_usec/1000);
	CxImage image;
	int srcstep = 0;

	if(image.IppEncode((BYTE*)pImg->imageData, pImg->width, pImg->height, \
		3, &srcstep, pOutImage, g_PicFormatInfo.nJpgQuality))
	{
		FILE* fp = NULL;
		fp = fopen(jpg_name, "a+");
		if(fp!=NULL)
		{
			fwrite(pOutImage, srcstep, 1, fp);
		}
	}

	if(pOutImage != NULL)
	{
		delete [] pOutImage;
		pOutImage = NULL;
	}

	nId++;
}


//从map中寻找一个最接近的序号,方法2
int CRoadCarnumDetect::GetSeqIndexFromMap2(UINT32 nSeq,UINT32 nPreSeq, SeqIndexMap& mapSeqIndex)
{
	int nIndex = 0;
	bool bFind = false;

	SeqIndexMap::iterator it = mapSeqIndex.begin();
	SeqIndexMap::iterator it_pre;

	UINT32 nSeqCur = 0;
	UINT32 nSeqPre = 0;
	UINT32 nDisSeq = 0;//帧号差

	//test log out put seq
	int nOutSeq = 0;

	while( (it != mapSeqIndex.end()) && (mapSeqIndex.size() >= 2) )
	{
		nSeqCur = it->first; //从map中取一帧

		if((nSeq <= nSeqCur) && ((nPreSeq == 0) ||(nSeqCur > nPreSeq)))
		{
			it_pre = it;

			if(it != mapSeqIndex.begin())
			{
				it_pre--;

				nSeqPre = it_pre->first; //当前帧的前一帧,序列[nSeqPre,ToFindSeq,nSeqCur]

				if(nPreSeq == nSeqPre)
				{
					nOutSeq = it->first;
					nIndex = it->second;
#ifdef OUT_STATE_LOG
					LogTrace("OutState.txt", "#=222=nOutSeq=%d=#", nOutSeq);
#endif
				}
				else
				{
					nDisSeq = nSeqCur - nSeqPre;

					//前后，帧差12以内取最近一张，否则取离摄像机更远的一张
					//由远及近：帧号更小的一张（时间靠前）
					//由近及远：帧号更大的一张（时间靠后）
					if(nDisSeq <= 12)
					{
						if(nSeq <= (nSeqCur+nSeqPre)*0.5) //择半查找，取更近的那一帧
						{
							nOutSeq = it_pre->first;
							nIndex = it_pre->second;

						#ifdef OUT_STATE_LOG
							LogTrace("OutState.txt", "#=333=nOutSeq=%d=,nDisSeq=%d #", nOutSeq, nDisSeq);
						#endif
						}
						else
						{
							nOutSeq = it->first;
							nIndex = it->second;
						#ifdef OUT_STATE_LOG
							LogTrace("OutState.txt", "#=444=nOutSeq=%d=,nDisSeq=%d #", nOutSeq, nDisSeq);
						#endif
						}
					}
					else
					{
						if(m_nDetectDirection == 0)//前牌（由远及近）
						{
							nOutSeq = it_pre->first;
							nIndex = it_pre->second;
						#ifdef OUT_STATE_LOG
							LogTrace("OutState.txt", "#=555=nOutSeq=%d=#,nDisSeq=%d #", nOutSeq, nDisSeq);
						#endif
						}
						else //尾牌（由近及远）
						{
							nOutSeq = it->first;
							nIndex = it->second;
						#ifdef OUT_STATE_LOG
							LogTrace("OutState.txt", "#=666=nOutSeq=%d=#,nDisSeq=%d #", nOutSeq, nDisSeq);
						#endif
						}
					}
				}

				//LogNormal("nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n",nSeq,nSeqPre,nSeqCur);
#ifdef OUT_STATE_LOG
				LogTrace("OutState.txt", "=EEE=nOutSeq=%d==nSeq=%lld,,nSeqPre=%lld,nSeqCur=%lld\n", \
					nOutSeq, nSeq, nSeqPre, nSeqCur);
#endif

				bFind = true;
				break;

			}//End of if
		}//End of if(nSeq <= nSeqCur)

		if(it == --mapSeqIndex.end())
		{
			nOutSeq = it->first;
			nIndex = it->second;

		#ifdef OUT_STATE_LOG
			LogTrace("OutState.txt", "#=777=nOutSeq=%d #", nOutSeq);
		#endif

			bFind = true;
			break;
		}

		it++;
	}

	if(!bFind)
	{
		if(mapSeqIndex.size() >0)
		{
			it = mapSeqIndex.begin();
			nIndex = it->second;

			nOutSeq = it->first;

		#ifdef OUT_STATE_LOG
			LogTrace("OutState.txt", "=2222=nOutSeq=%d==nSeq=%lld==\n", nOutSeq, nSeq);
		#endif
		}
	}

	return nIndex;
}


bool CRoadCarnumDetect::GetImageFromResultFrameList2(Vts_Picture_Key pickey,UINT32 nPreSeq,yuv_video_buf& bufResult)
{
	UINT32 nSeq = pickey.uSeq;
	printf("==in=GetImageFromResultFrameList2=\n");
	bool bFind = false;

	std::list<std::string>::iterator it = m_ResultFrameList.begin();
	std::list<std::string>::iterator it_pre;
	//////////
	while( (it!=m_ResultFrameList.end())&&(m_ResultFrameList.size()>=2))
	{
		yuv_video_buf* buf = (yuv_video_buf*)it->c_str();

		if(nPreSeq <= buf->nSeq)
		{
			if(nSeq <= buf->nSeq)
			{
				it_pre = it;

				if(it != m_ResultFrameList.begin())
				{
					it_pre--;
				}

				yuv_video_buf* buf_pre = (yuv_video_buf*)it_pre->c_str();
				if(nPreSeq == buf_pre->nSeq)
				{
					//printf("=====GetImageFromResultFrameList2==111111111==\n");
					//pStrPicBuf.append((char*)(it->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
					//memcpy(pStrPicBuf, (char*)(it->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
					#ifndef ALGORITHM_YUV_CARNUM
					//缓存jpg图片
					CxImage image;
					int srcstep = 0;
					if(image.IppEncode((BYTE*)(it->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->width,m_imgComposeResult->height,3,&srcstep,m_pCurJpgImage,g_PicFormatInfo.nJpgQuality))
					{
						string strPic;
						strPic.append((char*)(it->c_str()),sizeof(yuv_video_buf));
						strPic.append((char*)m_pCurJpgImage,srcstep);
						m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
					}
					#else
					//缓存yuv图片
					string strPic;
					strPic.append((char*)it->c_str(),sizeof(yuv_video_buf)+(m_imgResult->width*2*m_imgResult->height));
					m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
					#endif
					bufResult = *buf;
				}
				else
				{
					if(nSeq <= (buf_pre->nSeq+buf->nSeq)*0.5)
					{
						//printf("=====GetImageFromResultFrameList2==2222222222==\n");
						//pStrPicBuf.append((char*)(it_pre->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
						//memcpy(pStrPicBuf, (char*)(it_pre->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
						#ifndef ALGORITHM_YUV_CARNUM
						//缓存jpg图片
						CxImage image;
						int srcstep = 0;
						if(image.IppEncode((BYTE*)(it_pre->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->width,m_imgComposeResult->height,3,&srcstep,m_pCurJpgImage,g_PicFormatInfo.nJpgQuality))
						{
							string strPic;
							strPic.append((char*)(it_pre->c_str()),sizeof(yuv_video_buf));
							strPic.append((char*)m_pCurJpgImage,srcstep);
							m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
						}
						#else
						//缓存yuv图片
						string strPic;
						strPic.append((char*)it_pre->c_str(),sizeof(yuv_video_buf)+(m_imgResult->width*2*m_imgResult->height));
						m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
						#endif
						bufResult = *buf_pre;
					}
					else
					{
						//printf("=====GetImageFromResultFrameList2==3333333333==\n");
						//pStrPicBuf.append((char*)(it->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
						//memcpy(pStrPicBuf, (char*)(it->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
						#ifndef ALGORITHM_YUV_CARNUM
						//缓存jpg图片
						CxImage image;
						int srcstep = 0;
						if(image.IppEncode((BYTE*)(it->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->width,m_imgComposeResult->height,3,&srcstep,m_pCurJpgImage,g_PicFormatInfo.nJpgQuality))
						{
							string strPic;
							strPic.append((char*)(it->c_str()),sizeof(yuv_video_buf));
							strPic.append((char*)m_pCurJpgImage,srcstep);
							m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
						}	
						#else
						//缓存yuv图片
						string strPic;
						strPic.append((char*)it->c_str(),sizeof(yuv_video_buf)+(m_imgResult->width*2*m_imgResult->height));
						m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
						#endif
						bufResult = *buf;
					}	
				}							

				bFind = true;

#ifdef OUT_STATE_LOG
				//LogTrace("OutState.txt","=222==Find GetImageFromResultFrameList! nSeq:%d, nOutSeq:%d \n", nSeq, buf_pre->nSeq);

#endif
				break;
			}
		}
		it++;
	}

	if(!bFind)
	{
		if(m_ResultFrameList.size() >0)
		{
			it = --m_ResultFrameList.end();
			yuv_video_buf* buf = (yuv_video_buf*)it->c_str();
#ifdef OUT_STATE_LOG
			LogTrace("OutState.txt","====TTT==GetImageFromResultFrameList! nSeq:%d, nOutSeq:%d \n", nSeq, buf->nSeq);
			LogNormal("==bFind false! m_ResultFrameList.size=%d !\n", m_ResultFrameList.size());
#endif
			//pStrPicBuf.append((char*)(it->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
			//memcpy(pStrPicBuf, (char*)(it->c_str()), m_imgComposeResult->imageSize + sizeof(yuv_video_buf));
			#ifndef ALGORITHM_YUV_CARNUM
			//缓存jpg图片
			CxImage image;
			int srcstep = 0;
			if(image.IppEncode((BYTE*)(it->c_str()+sizeof(yuv_video_buf)),m_imgComposeResult->width,m_imgComposeResult->height,3,&srcstep,m_pCurJpgImage,g_PicFormatInfo.nJpgQuality))
			{
				string strPic;
				strPic.append((char*)(it->c_str()),sizeof(yuv_video_buf));
				strPic.append((char*)m_pCurJpgImage,srcstep);
				m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
			}
			#else
				//缓存yuv图片
				string strPic;
				strPic.append((char*)it->c_str(),sizeof(yuv_video_buf)+(m_imgResult->width*2*m_imgResult->height));
				m_vtsPicMap.insert(VtsPicDataMap::value_type(pickey,strPic));
			#endif
			bufResult = *buf;
			bFind = true;			
		}
	}

	printf("=22===GetImageFromResultFrameList2=====\n");
	return bFind;
}


#ifdef REDADJUST
//红绿灯增强
void CRoadCarnumDetect::RedLightAdjust(IplImage* pImage,bool bCheckRect, UINT16 uColorSignal, unsigned long uSignalSeq)
{
	//LogTrace("RedSignal.log", "id:%d pImage:%x,bCheckRect:%d uColorSignal:%d m_nRedNum:%d", \
	//	m_nChannelId, pImage, bCheckRect, uColorSignal,m_nRedNum);

	if(NULL == m_pRedLightAdjust)
	{
		LogNormal("CRoadCarnumDetect::RedLightAdjust error! \n");
		return;
	}
	
	if(m_nRedNum > 0)
	{
		CvRect rect;
		rect.x = 0;
		rect.y = 0;
		if(bCheckRect)
		{
			if(m_nWordPos == 1)
			{	
				if(m_nWordOnPic == 0)
				{
					rect.y = m_nExtentHeight;
				}
			}
		}
		rect.width = pImage->width;
		rect.height = pImage->height-m_nExtentHeight;	

		printf("RedLightAdjust rect.x=%d,rect.y=%d,rect.width=%d,rect.height=%d,pImage->width=%d,pImage->height=%d\n",rect.x,rect.y,rect.width,rect.height,pImage->width,pImage->height);
		
		pthread_mutex_lock(&m_RedAdjustMutex);
		if(m_vtsGlobalPara.bStrongSignal)
		{
		//#ifdef REDADJUST_2_DEBUG
			//SaveImgTest(pImage);
		//#endif

			cvSetImageROI(pImage,rect);
			int nColorArray[MAX_RED_LIGHT_NUM];
			for(int i=0; i<MAX_RED_LIGHT_NUM; i++)
			{
				nColorArray[i] = -1;
			}
			GetRedColor(uColorSignal, nColorArray);

			//pImage:图片,
			//nColor: 灯颜色(0-红灯；1-绿灯；2-黄灯; -1-没有灯亮)
			
			int nDayNight = 1;
			time_t now = time(NULL);
			struct tm *newTime,timenow;
			newTime = &timenow;
			localtime_r( &now,newTime );

			unsigned int nDayHour1 = (g_LightTimeInfo.nSummerLightTime >> 16);
            unsigned int nNightHour1 = (g_LightTimeInfo.nSummerLightTime & 0xffff);

			unsigned int nDayHour2 = (g_LightTimeInfo.nWinterLightTime >> 16);
            unsigned int nNightHour2 = (g_LightTimeInfo.nWinterLightTime & 0xffff);

			unsigned int uTime = newTime->tm_hour*100+newTime->tm_min;
			if(  ((uTime>=nDayHour1)&&(uTime<=nNightHour1))  ||  ((uTime>=nDayHour2)&&(uTime<=nNightHour2)) )
			{
				m_pRedLightAdjust->mvRedLightAdjustNew(pImage, nColorArray);
			}

			cvResetImageROI(pImage);

		#ifdef REDADJUST_2_DEBUG
				//SaveImgTest(pImage,uColorSignal);
				//LogNormal("id:%d sig:%d nColorArray:%d,%d,%d", m_nChannelId, uColorSignal, nColorArray[0], nColorArray[1], nColorArray[2]);
				LogTrace("RedSignal.log", "id:%d sig:%d nColorArray:%d,%d,%d", m_nChannelId, uColorSignal, nColorArray[0], nColorArray[1], nColorArray[2]);
				//图片上叠加信号值 [RED][GREEN]
				PutRedSignal(pImage, nColorArray, uSignalSeq);
		#endif
		}
		else
		{
			if(m_vtsGlobalPara.bStrongLight)
			{
				cvSetImageROI(pImage,rect);
				m_pRedLightAdjust->mvRedLightAdjust(pImage);
				cvResetImageROI(pImage);
			}
		}

#ifdef RED_GREEN_ENHANCE
		//低噪度（英泰智进行，红灯发暗处理		
		{
			//LogNormal("m_nCameraType:%d m_nRedNum:%d RedGreenEnhance \n", m_nCameraType, m_nRedNum);
			//RedGreenEnhance(Uint8 *image_data,int start_x,int start_y,int width,int height,int image_width);
			for(int i=0; i<m_nRedNum; i++)
			{
				//LogNormal("rect:%d,%d,%d,%d ", m_rectRedArray[i].x, m_rectRedArray[i].y,m_rectRedArray[i].width, m_rectRedArray[i].height);
				//printf("pImage nSize:%d \n", pImage->nSize);
				if(m_rectRedArray[i].width > 0 && m_rectRedArray[i].height > 0)
				{
					RedGreenEnhance((unsigned char*)(pImage->imageData), m_rectRedArray[i].x, m_rectRedArray[i].y, \
						m_rectRedArray[i].width, m_rectRedArray[i].height, pImage->width);
				}				
			}
		}
#endif

		pthread_mutex_unlock(&m_RedAdjustMutex);
	}//End of if(m_nRedNum > 0)
}
#endif// #ifdef REDADJUST

//dsp红绿灯增强
void CRoadCarnumDetect::ProcessSinglePicRedForDsp(const int nPicCount, const PLATEPOSITION* pTimeStamp)
{
	#ifndef REDADJUST
	 if(m_imgComposeSnap != NULL)
     {
			if ( g_nVtsPicMode < 2)
			{
						if(m_vtsGlobalPara.bStrongLight > 0)
						{
							int nHeight = m_imgComposeResult->height;
							
							if(m_nWordOnPic == 1)
							nHeight -= m_nExtentHeight;

							for(int nIndex = 0; nIndex <nPicCount ; nIndex++)
							{
								CvRect rect;

								if(g_nServerType == 15)
								{
									rect.x = (m_imgComposeResult->width)*((nIndex)%2);
									rect.y = nHeight*((nIndex)/2);
									rect.width = m_imgComposeResult->width;
									rect.height = nHeight;
								}
								else
								{
									if( 1 == g_nVtsPicMode)
									{
										rect.x = (m_imgComposeResult->width)*((nIndex+1)%2);
										rect.y = nHeight*((nIndex+1)/2);
										rect.width = m_imgComposeResult->width;
										rect.height = nHeight;
									}
									else if( 0 == g_nVtsPicMode)
									{
										rect.x = 0;
										rect.y = nHeight*nIndex;
										rect.width = m_imgComposeResult->width;
										rect.height = nHeight;
									}
								}

								UINT16 uTrafficSignal = pTimeStamp[nIndex].nRedColor;

								//LogNormal("uSeq=%lld,uTrafficSignal=%x rect.x=%d,rect.y=%d\n",TimeStamp[nIndex].uSeq,uTrafficSignal,rect.x,rect.y);
									float fStrongEnhance = m_vtsGlobalPara.nStrongEnhance/10.0;
									mv_RedLightProcess RedLightProcess(fStrongEnhance);

									vector<ChannelRegion>::iterator it_r = m_vtsObjectRegion.begin();
									while(it_r != m_vtsObjectRegion.end())
									{
										VTSParaMap::iterator it_p = m_vtsObjectParaMap.find(it_r->nRoadIndex);
										if(it_p != m_vtsObjectParaMap.end())
										{
											CvRect roiLeftLight_red =  it_r->roiLeftLight_red;
											if(roiLeftLight_red.width > 0)
											{
												roiLeftLight_red.x += rect.x;
												roiLeftLight_red.y += rect.y;
											}

											CvRect roiLeftLight_green =  it_r->roiLeftLight_green;
											if(roiLeftLight_green.width > 0)
											{
												roiLeftLight_green.x += rect.x;
												roiLeftLight_green.y += rect.y;
											}

											CvRect roiMidLight_red =  it_r->roiMidLight_red;
											if(roiMidLight_red.width > 0)
											{
												roiMidLight_red.x += rect.x;
												roiMidLight_red.y += rect.y;
											}

											CvRect roiMidLight_green =  it_r->roiMidLight_green;
											if(roiMidLight_green.width > 0)
											{
												roiMidLight_green.x += rect.x;
												roiMidLight_green.y += rect.y;
											}

											CvRect roiRightLight_red =  it_r->roiRightLight_red;
											if(roiRightLight_red.width > 0)
											{
												roiRightLight_red.x += rect.x;
												roiRightLight_red.y += rect.y;
											}

											CvRect roiRightLight_green =  it_r->roiRightLight_green;
											if(roiRightLight_green.width > 0)
											{
												roiRightLight_green.x += rect.x;
												roiRightLight_green.y += rect.y;
											}

											bool bSignalType = (uTrafficSignal>>15)& 0x1;

											bool bLeftSignal = ((uTrafficSignal>>(it_p->second.nLeftControl)) & 0x1);
											bool bStraightSignal = ((uTrafficSignal>>(it_p->second.nStraightControl)) & 0x1);
											bool bRightSignal = ((uTrafficSignal>>(it_p->second.nRightControl)) & 0x1);

											if(!bSignalType)//视频检测红绿灯
											{
												bLeftSignal = (uTrafficSignal >> 14)& 0x1;
												bStraightSignal = (uTrafficSignal >> 13)& 0x1;
												bRightSignal = (uTrafficSignal >> 12)& 0x1;
											}

											//LogNormal("uTrafficSignal=%x,bSignalType=%d,l=%d,s=%d,r=%d\n",uTrafficSignal,bSignalType,bLeftSignal,bStraightSignal,bRightSignal);


											CvPoint m_affparams = cvPoint(0,0);
											if( (roiLeftLight_red.width >= 0&& roiLeftLight_red.height >= 0) && (roiLeftLight_green.width >= 0&& roiLeftLight_green.height >= 0) &&(roiMidLight_red.width >= 0&& roiMidLight_red.height >= 0)&&(roiMidLight_green.width >= 0&& roiMidLight_green.height >= 0) &&(roiRightLight_red.width >= 0&& roiRightLight_red.height >= 0)&&(roiRightLight_green.width >= 0&& roiRightLight_green.height >= 0) )
											{
												RedLightProcess.ProcessSinglePic_ForDsp(m_imgComposeSnap,bLeftSignal,bStraightSignal,bRightSignal,roiLeftLight_red,roiLeftLight_green,roiMidLight_red,roiMidLight_green,roiRightLight_red,roiRightLight_green,m_affparams);
											}
										}
										it_r++;
									}
							}
					}
			}
	}
	#endif
}

//载入检测参数
bool CRoadCarnumDetect::LoadDspSettingInfo2()
{
	#ifndef ALGORITHM_YUV
	MvDspGlobalSetting dspSettingStrc;

	CXmlParaUtil xml;
	bool bLoadDspSetFlag = xml.LoadDspSettingFile(dspSettingStrc,m_nChannelId);
	if(bLoadDspSetFlag)
	{
		 m_vtsObjectRegion.clear();
		 m_vtsObjectParaMap.clear();
		 m_roadParamInlist.clear();

		//违章检测参数
		VTS_GLOBAL_PARAMETER vtsGlobalPara;

		for(int i = 0;i<dspSettingStrc.nChannels;i++)
		{
			DspChannelRegion dspvtsRegion = dspSettingStrc.ChnlRegion[i];
			ChannelRegion  vtsRegion;

			vtsRegion.nRoadIndex = dspvtsRegion.nRoadIndex;
			vtsRegion.nVerRoadIndex = dspvtsRegion.nVerRoadIndex;

			
			//方向
			/*if(dspSettingStrc.ChnlRegion[0].nDirection < 180)
			m_nDetectDirection = 0;//前牌
			else
			m_nDetectDirection = 1;*/

			if(dspSettingStrc.ChnlRegion[0].vDirection.end.y >= dspSettingStrc.ChnlRegion[0].vDirection.start.y)
			{
				m_nDetectDirection = 0;//前牌
			}
			else
			{
				m_nDetectDirection = 1;//尾牌
			}

			LogNormal("id=%d,RoadIndex=%d,VerRoadIndex=%d,y1=%d,y2=%d,DetectDirection=%d\n",m_nChannelId,vtsRegion.nRoadIndex,vtsRegion.nVerRoadIndex,dspSettingStrc.ChnlRegion[0].vDirection.start.y,dspSettingStrc.ChnlRegion[0].vDirection.end.y,m_nDetectDirection);

			vtsRegion.OnOffRed.x = dspvtsRegion.OnOffRed.x;
			vtsRegion.OnOffRed.y = dspvtsRegion.OnOffRed.y;
			vtsRegion.OnOffRed.width = dspvtsRegion.OnOffRed.width;
			vtsRegion.OnOffRed.height = dspvtsRegion.OnOffRed.height;

			vtsRegion.OnOffGreen.x = dspvtsRegion.OnOffGreen.x;
			vtsRegion.OnOffGreen.y = dspvtsRegion.OnOffGreen.y;
			vtsRegion.OnOffGreen.width = dspvtsRegion.OnOffGreen.width;
			vtsRegion.OnOffGreen.height = dspvtsRegion.OnOffGreen.height;

			vtsRegion.roiLeftLight.x = dspvtsRegion.roiLeftLight.x;
			vtsRegion.roiLeftLight.y = dspvtsRegion.roiLeftLight.y;
			vtsRegion.roiLeftLight.width = dspvtsRegion.roiLeftLight.width;
			vtsRegion.roiLeftLight.height = dspvtsRegion.roiLeftLight.height;

			vtsRegion.roiRightLight.x = dspvtsRegion.roiRightLight.x;
			vtsRegion.roiRightLight.y = dspvtsRegion.roiRightLight.y;
			vtsRegion.roiRightLight.width = dspvtsRegion.roiRightLight.width;
			vtsRegion.roiRightLight.height = dspvtsRegion.roiRightLight.height;

			vtsRegion.roiMidLight.x = dspvtsRegion.roiMidLight.x;
			vtsRegion.roiMidLight.y = dspvtsRegion.roiMidLight.y;
			vtsRegion.roiMidLight.width = dspvtsRegion.roiMidLight.width;
			vtsRegion.roiMidLight.height = dspvtsRegion.roiMidLight.height;

			vtsRegion.roiTurnAroundLight.x = dspvtsRegion.roiTurnAroundLight.x;
			vtsRegion.roiTurnAroundLight.y = dspvtsRegion.roiTurnAroundLight.y;
			vtsRegion.roiTurnAroundLight.width = dspvtsRegion.roiTurnAroundLight.width;
			vtsRegion.roiTurnAroundLight.height = dspvtsRegion.roiTurnAroundLight.height;

			vtsRegion.roiLeftLight_red.x = dspvtsRegion.roiLeftLight_red.x;
			vtsRegion.roiLeftLight_red.y = dspvtsRegion.roiLeftLight_red.y;
			vtsRegion.roiLeftLight_red.width = dspvtsRegion.roiLeftLight_red.width;
			vtsRegion.roiLeftLight_red.height = dspvtsRegion.roiLeftLight_red.height;

			vtsRegion.roiLeftLight_green.x = dspvtsRegion.roiLeftLight_green.x;
			vtsRegion.roiLeftLight_green.y = dspvtsRegion.roiLeftLight_green.y;
			vtsRegion.roiLeftLight_green.width = dspvtsRegion.roiLeftLight_green.width;
			vtsRegion.roiLeftLight_green.height = dspvtsRegion.roiLeftLight_green.height;

			vtsRegion.roiRightLight_red.x = dspvtsRegion.roiRightLight_red.x;
			vtsRegion.roiRightLight_red.y = dspvtsRegion.roiRightLight_red.y;
			vtsRegion.roiRightLight_red.width = dspvtsRegion.roiRightLight_red.width;
			vtsRegion.roiRightLight_red.height = dspvtsRegion.roiRightLight_red.height;

			vtsRegion.roiRightLight_green.x = dspvtsRegion.roiRightLight_green.x;
			vtsRegion.roiRightLight_green.y = dspvtsRegion.roiRightLight_green.y;
			vtsRegion.roiRightLight_green.width = dspvtsRegion.roiRightLight_green.width;
			vtsRegion.roiRightLight_green.height = dspvtsRegion.roiRightLight_green.height;

			vtsRegion.roiMidLight_red.x = dspvtsRegion.roiMidLight_red.x;
			vtsRegion.roiMidLight_red.y = dspvtsRegion.roiMidLight_red.y;
			vtsRegion.roiMidLight_red.width = dspvtsRegion.roiMidLight_red.width;
			vtsRegion.roiMidLight_red.height = dspvtsRegion.roiMidLight_red.height;

			vtsRegion.roiMidLight_green.x = dspvtsRegion.roiMidLight_green.x;
			vtsRegion.roiMidLight_green.y = dspvtsRegion.roiMidLight_green.y;
			vtsRegion.roiMidLight_green.width = dspvtsRegion.roiMidLight_green.width;
			vtsRegion.roiMidLight_green.height = dspvtsRegion.roiMidLight_green.height;

			vtsRegion.roiTurnAroundLight_red.x = dspvtsRegion.roiTurnAroundLight_red.x;
			vtsRegion.roiTurnAroundLight_red.y = dspvtsRegion.roiTurnAroundLight_red.y;
			vtsRegion.roiTurnAroundLight_red.width = dspvtsRegion.roiTurnAroundLight_red.width;
			vtsRegion.roiTurnAroundLight_red.height = dspvtsRegion.roiTurnAroundLight_red.height;

			vtsRegion.roiTurnAroundLight_green.x = dspvtsRegion.roiTurnAroundLight_green.x;
			vtsRegion.roiTurnAroundLight_green.y = dspvtsRegion.roiTurnAroundLight_green.y;
			vtsRegion.roiTurnAroundLight_green.width = dspvtsRegion.roiTurnAroundLight_green.width;
			vtsRegion.roiTurnAroundLight_green.height = dspvtsRegion.roiTurnAroundLight_green.height;

			//LogNormal("ll=%d,rl=%d,ml=%d\n",vtsRegion.roiLeftLight.width,vtsRegion.roiRightLight.width,vtsRegion.roiMidLight.width);

			if(vtsRegion.roiLeftLight_red.width > 0 ||
				vtsRegion.roiLeftLight_green.width > 0 ||
				vtsRegion.roiRightLight_red.width > 0 ||
				vtsRegion.roiRightLight_green.width > 0 ||
				vtsRegion.roiMidLight_red.width > 0 ||
				vtsRegion.roiMidLight_green.width > 0 ||
				vtsRegion.roiTurnAroundLight_red.width > 0 ||
				vtsRegion.roiTurnAroundLight_green.width > 0)
			{
				//LogNormal("ll=%d,llr=%d,llg=%d\n",vtsRegion.roiLeftLight.width,vtsRegion.roiLeftLight_red.width,vtsRegion.roiLeftLight_green.width);
				vtsGlobalPara.bStrongLight = true;
			}
			vtsGlobalPara.nRedLightTime = dspvtsRegion.m_pRedLightDelayTime[1];

			m_vtsObjectRegion.push_back(vtsRegion);
			//LogNormal("--m_vtsObjectRegion.size()=%d-LoadDspSettingInfo2 \n", m_vtsObjectRegion.size());

			//m_ftVlp.SetChannels(m_vtsObjectRegion);

			PARAMETER_VTS vtsPara;
			vtsPara.nRoadIndex = dspvtsRegion.nRoadIndex;
			vtsPara.nLeftControl = (TRAFFIC_SIGNAL_DIRECTION)dspvtsRegion.nLeftControl;
			vtsPara.nStraightControl = (TRAFFIC_SIGNAL_DIRECTION)dspvtsRegion.nStraightControl;
			vtsPara.nRightControl = (TRAFFIC_SIGNAL_DIRECTION)dspvtsRegion.nRightControl;
			vtsPara.nRoadDirection = dspvtsRegion.nChannelDriveDir;
			
			//禁行时间
			if(dspvtsRegion.nNoPassingInfoNumber > 0 && dspvtsRegion.nNoPassingInfoNumber <= 24)
			{
				vtsPara.nForbidType = dspvtsRegion.vecNoPassingInfo[0].nVehType;
				vtsPara.nForbidBeginTime = dspvtsRegion.vecNoPassingInfo[0].nStart;
				vtsPara.nForbidEndTime = dspvtsRegion.vecNoPassingInfo[0].nEnd;
				//LogNormal("vtsPara.nForbidType=%d",vtsPara.nForbidType);
			}

			m_vtsObjectParaMap.insert(VTSParaMap::value_type(vtsPara.nRoadIndex,vtsPara));
			//LogNormal("m_vtsObjectParaMap RoadIndex:%d ,Map.size:%d ", vtsPara.nRoadIndex, m_vtsObjectParaMap.size());

			VEHICLE_PARAM_FOR_EVERY_FRAME roadParamIn;
			//roadParamIn.nChannelID  = dspvtsRegion.nRoadIndex;
			roadParamIn.nChannelID = dspvtsRegion.nVerRoadIndex;
			roadParamIn.m_nOnlyOverSpedMax = dspvtsRegion.nRadarAlarmVelo;
			roadParamIn.m_nAvgSpeedMax = dspvtsRegion.uAlarmBig;//大车超速阈值
			roadParamIn.m_nAvgSpeedMin = dspvtsRegion.uAlarmSmall;//小车车超速阈值
			LogNormal("nChannelID=%d,m_nOnlyOverSpedMax=%.2f",roadParamIn.nChannelID,roadParamIn.m_nOnlyOverSpedMax);
			LogNormal("nChannelID=%d,m_nAvgSpeedMax=%.2f",roadParamIn.nChannelID,roadParamIn.m_nAvgSpeedMax);
			LogNormal("nChannelID=%d,m_nAvgSpeedMin=%.2f",roadParamIn.nChannelID,roadParamIn.m_nAvgSpeedMin);
			m_roadParamInlist.push_back(roadParamIn);
		}

		m_vtsGlobalPara.nRedLightTime = vtsGlobalPara.nRedLightTime;
		m_vtsGlobalPara.bStrongLight = vtsGlobalPara.bStrongLight;

		m_rtCarnumROI.x = dspSettingStrc.m_rtCarNum.x;
		m_rtCarnumROI.y = dspSettingStrc.m_rtCarNum.y;
		m_rtCarnumROI.width = dspSettingStrc.m_rtCarNum.width;
		m_rtCarnumROI.height = dspSettingStrc.m_rtCarNum.height;

		return true;
	}
	#endif
	return false;
}

//参数转换为MvDspGlobalSetting结构
void CRoadCarnumDetect::ConvertDspSetting(CvRect farrect,CvRect nearrect)
{
	#ifdef ALGORITHM_YUV
	g_Setting.nWidth = m_img->width;           //图像的宽度（应用配置）
	g_Setting.nHeight = m_img->height;          //图像的高度（应用配置）
	g_Setting.m_fScaleX = m_fScaleX;			//m_nWidth*m_nScaleX==原图的宽-  宽度的缩放比例（应用配置）				
	g_Setting.m_fScaleY = m_fScaleY;            //高度的缩放比例（应用配置）
	g_Setting.nCheckType = (int)m_nDetectKind; //做检测的类型（应用配置）

	printf("g_Setting.nWidth=%d,g_Setting.nHeight=%d\n",g_Setting.nWidth,g_Setting.nHeight);
	printf("g_Setting.m_fScaleX=%f,g_Setting.m_fScaleY=%f\n",g_Setting.m_fScaleX,g_Setting.m_fScaleY);
	printf("g_Setting.nCheckType=%d\n",g_Setting.nCheckType);

	g_Setting.nNoPutInRegNum = 0;
	g_Setting.nNoStopRegNum = 0;
	g_Setting.nNoResortRegNum = 0;
	g_Setting.m_nDelayOutput = 0;
	g_Setting.nCarPicCount = 1;
	g_Setting.parkingRegion.nPoints = 0;

	printf("g_Setting.nChannels=%d\n",g_Setting.nChannels);

	//车道区域及参数
	vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
	VTSParaMap::iterator it_m = m_vtsObjectParaMap.begin();
	int nIndex = 0;
	while(it != m_vtsObjectRegion.end())
	{
		printf("nIndex=%d,nRoadIndex=%d,nDirection=%d\n",nIndex,g_Setting.ChnlRegion[nIndex].nRoadIndex,g_Setting.ChnlRegion[nIndex].nDirection);

		/////////////////参数设置
		g_Setting.ChnlRegion[nIndex].bNoTurnLeft = it_m->second.bForbidLeft;
		g_Setting.ChnlRegion[nIndex].bNoTurnRight = it_m->second.bForbidRight;
		g_Setting.ChnlRegion[nIndex].bNoForeward = it_m->second.bForbidRun;
		g_Setting.ChnlRegion[nIndex].bNoReverse = it_m->second.bAgainst;
		g_Setting.ChnlRegion[nIndex].bNoPressLine = it_m->second.bPressLine;
		g_Setting.ChnlRegion[nIndex].bNoChangeChannel = it_m->second.bChangeRoad;
		g_Setting.ChnlRegion[nIndex].bFlagHoldStopReg = it_m->second.nFlagHoldStopReg;
		g_Setting.ChnlRegion[nIndex].nNonMotorWay  = it_m->second.nRoadType;
		g_Setting.ChnlRegion[nIndex].nChannelDriveDir = it_m->second.nRoadDirection;

		printf("bNoReverse=%d,bNoPressLine=%d\n",g_Setting.ChnlRegion[nIndex].bNoReverse,g_Setting.ChnlRegion[nIndex].bNoPressLine);

		g_Setting.ChnlRegion[nIndex].nLeftControl = it_m->second.nLeftControl;
		g_Setting.ChnlRegion[nIndex].nStraightControl = it_m->second.nStraightControl;
		g_Setting.ChnlRegion[nIndex].nRightControl = it_m->second.nRightControl;

		g_Setting.ChnlRegion[nIndex].m_pRedLightDelayTime[1] = it_m->second.nRedLightTime;
		
		g_Setting.ChnlRegion[nIndex].nPhysicalLoop = 0;
		g_Setting.ChnlRegion[nIndex].nNoPassingInfoNumber = 0;

		it++;
		it_m++;
		nIndex++;
	}
	#endif
}

//Dsp流量统计
void CRoadCarnumDetect::VehicleStatisticDsp(StatResultList& listStatResult,UINT32 uTimestamp)
{
	//1拖N模式由检测器统计车流量
	if(uTimestamp >= m_uPreTrafficSignalTime + m_nTrafficStatTime)
	{
		listStatResult.clear();

		vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
		while(it != m_vtsObjectRegion.end())
		{
			STAT_RESULT StatResult;
			StatResult.nChannelIndex = it->nRoadIndex;
			StatResult.sRtype = STAT_FLUX;
			StatResult.value = 0;
			listStatResult.push_back(StatResult);

			StatResult.sRtype = STAT_SPEED_AVG;
			StatResult.value = 0;
			listStatResult.push_back(StatResult);

			StatResult.sRtype = STAT_ZYL;//平均占有率
			StatResult.value = 0;
			listStatResult.push_back(StatResult);

			StatResult.sRtype = STAT_QUEUE;//平均队列长度
			StatResult.value = 0;
			listStatResult.push_back(StatResult);

			StatResult.sRtype = STAT_CTJJ;//平均车头间距
			StatResult.value = 0;
			listStatResult.push_back(StatResult);

			it++;
		}

		m_uPreTrafficSignalTime = uTimestamp;
	}
	else
	{
		return;
	}


	//车道编号
	UINT32 uRoadID = 0;
	UINT32 uVerRoadID = 0;
	UINT32 uObjectNum = 0;

	//统计保存
	RECORD_STATISTIC statistic;

	StatResultList::iterator it_b = listStatResult.begin();
	StatResultList::iterator it_e = listStatResult.end();
	while(it_b != it_e)
	{
		uRoadID = it_b->nChannelIndex;       //车道编号
		//printf("---uRoadID=%d-\n", uRoadID);

		vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
		while(it != m_vtsObjectRegion.end())
		{
			if(uRoadID == -1||
				uRoadID == it->nRoadIndex)
			{
				uVerRoadID = it->nVerRoadIndex;//车道逻辑编号
				uRoadID = it->nRoadIndex;
				break;
			}
			it++;
		}

		//LogNormal("uRoadID=%d,uVerRoadID=%d\n",uRoadID,uVerRoadID);
		//LogNormal("uFluxAll[uVerRoadID-1]=%d\n",uFluxAll[uVerRoadID-1]);
		
		if(uRoadID < 1 || uVerRoadID < 1)
		{
			LogNormal("Err sRtype:%d uRoadID:%d uVerRoadID:%d sRtype:%d\n ", \
				it_b->sRtype, uRoadID,uVerRoadID, it_b->sRtype);

			if(uVerRoadID < 1)
			{
				LogNormal("车道编号有误,不统计流量! uVerRoadID:%d ", uVerRoadID);
			//	uVerRoadID = 1;//test
			}
			
			return;
		}
		
		switch(it_b->sRtype)
		{
		case STAT_FLUX:
			//高16位大车总数，低16位车辆总数
			uObjectNum = uFluxBig[uVerRoadID-1];
			uObjectNum = uObjectNum<<16;
			statistic.uFlux[uRoadID-1] = uObjectNum | (uFluxAll[uVerRoadID-1]);
			//LogNormal("statistic.uFlux[uRoadID-1]=%d\n",statistic.uFlux[uRoadID-1]);
			break;
		case STAT_SPEED_AVG:

			if(uFluxAll[uVerRoadID-1] > 0)
			{
				statistic.uSpeed[uRoadID-1] = uAverageCarSpeed[uVerRoadID-1]/uFluxAll[uVerRoadID-1];
			}
			else
			{
				statistic.uSpeed[uRoadID-1] = 0;
			}
			break;
		case STAT_ZYL:			
			if(uFluxAll[uVerRoadID-1] > 0)
			{
				statistic.uOccupancy[uRoadID-1] = (UINT32)(uFluxAll[uVerRoadID-1] * 2 * 100 / m_nTrafficStatTime);
			}
			else
			{
				statistic.uOccupancy[uRoadID-1] = 0;
			}
			break;
		case STAT_QUEUE:
			statistic.uQueue[uRoadID-1] = 0;			
			break;
		case STAT_CTJJ:
			if(uFluxAll[uVerRoadID-1] > 0)
			{
				statistic.uSpace[uRoadID-1] = uAvgDis[uVerRoadID-1]/uFluxAll[uVerRoadID-1];
			}
			else
			{
				statistic.uSpace[uRoadID-1] = 0;
			}
			break;			
		}

		//LogNormal("FLUX-[%d,%d,%d,%d,%d]-\n", \
		//	statistic.uFlux[uRoadID-1], statistic.uSpeed[uRoadID-1], statistic.uOccupancy[uRoadID-1], statistic.uQueue[uRoadID-1], statistic.uSpace[uRoadID-1]);

		//高16位中型车总数，低16位小车总数
		uObjectNum = uFluxMiddle[uVerRoadID-1];
		uObjectNum = uObjectNum<<16;
		statistic.uFluxCom[uRoadID-1] = uObjectNum|(uFluxSmall[uVerRoadID-1]);
		//LogNormal("statistic.uFluxCom[uRoadID-1]=%d\n",statistic.uFluxCom[uRoadID-1]);
		statistic.uTime = uTimestamp;
		statistic.uStatTimeLen = m_nTrafficStatTime;

		if(uFluxBig[uVerRoadID-1] > 0)
		{
			statistic.uBigCarSpeed[uRoadID-1] = (uBigCarSpeed[uVerRoadID-1]*1.0/uFluxBig[uVerRoadID-1]);
		}
		else
		{
			statistic.uBigCarSpeed[uRoadID-1] = 0;
		}

		if(uFluxSmall[uVerRoadID-1] > 0)
		{
			statistic.uSmallCarSpeed[uRoadID-1] = (uSmallCarSpeed[uVerRoadID-1]*1.0/uFluxSmall[uVerRoadID-1]);
		}
		else
		{
			statistic.uSmallCarSpeed[uRoadID-1] = 0;
		}

		statistic.uMaxSpeed[uRoadID-1] = uMaxSpeed[uVerRoadID-1];

		//高16位车道逻辑编号，低16位车道类型
		uVerRoadID = uVerRoadID<<16;
		statistic.uRoadType[uRoadID-1] = uVerRoadID | VEHICLE_ROAD;

		it_b++;
	}	


	//printf("-----g_skpDB-SaveStatisticInfo---\n");
	//记录检测数据,保存统计
	g_skpDB.SaveStatisticInfo(m_nChannelId,statistic,VEHICLE_ROAD); 
}

/*
* 函数说明：500W图像转成200W
* input: CxImage &imageIn:需要解码图像输入
* output: IplImage* pImageOut:转换后图像输出 
* 		IplImage* pImageTmp：转换中间暂存图像
* 返回值：获取图片，是否成功．
*/
void CRoadCarnumDetect::ReSizePic(IplImage* pImageIn, IplImage* pImageOut)
{
		printf("pImageIn->width,height=%d,%d--pImageOut->width,height=%d,%d--m_nExtentHeight=%d-------\n",pImageIn->width, pImageIn->height,pImageOut->width, pImageOut->height, m_nExtentHeight);

		cvSet(pImageOut, cvScalar( 0,0, 0 ));

		int nCount = pImageIn->height/m_imgSnap->height;
		
			for(int i = 0;i<nCount;i++)
			{
					//Resize pic
					CvRect rtPos;
					CvRect rtRect;

					rtPos.x = 0;
					rtPos.y = i*(m_imgSnap->height)+248;
					rtPos.width = 2400;
					rtPos.height = 1800;

					rtRect.x = 0;
					rtRect.y = i*(m_imgDestSnap2->height);
					rtRect.width = 1600;
					rtRect.height = 1200;

					cvSetImageROI(pImageIn,rtPos);

					cvSetImageROI(pImageOut,rtRect);
					cvResize(pImageIn,pImageOut);
					cvResetImageROI(pImageOut);

					cvResetImageROI(pImageIn);
			}
}

#ifdef MATCHPLATE
//添加输出结果到比对队列
bool CRoadCarnumDetect::AddDspPlateMatch(RECORD_PLATE &plate, const IplImage * pImgSnap)
{

}
#endif


//#ifdef FBMATCHPLATE
//添加输出结果到比对队列
bool CRoadCarnumDetect::AddDspPlateMatchFortianjin(RECORD_PLATE &plate,  const IplImage * pImgSnap)
{

}
//#endif

//缓冲yuv数据
void CRoadCarnumDetect::AddYUVBuf(int nCodingNum,unsigned int pCodingIndex[16])
{
	Vts_Picture_Key vtsPicKey;
	vtsPicKey.uCameraId = m_nCameraID;	

	//加锁
	pthread_mutex_lock(&m_PlateMutex);

	//暂存seq->index对应关系
	SeqIndexMap mapSeqIndex;
	for(int j = 0;j<m_nMaxPlateSize;j++)
	{
			yuv_video_buf* pBuf = (yuv_video_buf*)(m_BufPlate[j].pCurBuffer);
			if(pBuf!=NULL)
			{
				mapSeqIndex.insert(SeqIndexMap::value_type(pBuf->nSeq,j));
			}
	}

	if(mapSeqIndex.size()<=1)
	{
			pthread_mutex_unlock(&m_PlateMutex);
			return;
	}

	SeqIndexMap::iterator it_b = mapSeqIndex.begin();
	SeqIndexMap::iterator it_e = --mapSeqIndex.end();
	UINT32 nMinSeq = it_b->first; //最小帧号
	UINT32 nMaxSeq = it_e->first;//最大帧号

	for(int i = 0;i<nCodingNum;i++)
	{
		vtsPicKey.uSeq = pCodingIndex[i];

		UINT32 nSeq = vtsPicKey.uSeq;

		int index = 0;
		yuv_video_buf* buf = NULL;

		if(nSeq > nMaxSeq)
		{
			nSeq = nMaxSeq;
		}
		if(nSeq < nMinSeq)
		{
			nSeq = nMinSeq;
		}

		SeqIndexMap::iterator it_map = mapSeqIndex.find(nSeq);
		if(it_map!=mapSeqIndex.end())
		{
			index = it_map->second;
			buf = (yuv_video_buf*)(m_BufPlate[index].pCurBuffer);
			if(buf)
			{
				//缓存yuv图片
				string strPic;
				strPic.append((char*)m_BufPlate[index].pCurBuffer,sizeof(yuv_video_buf)+(m_imgResult->width*2*m_imgResult->height));
				m_vtsPicMap.insert(VtsPicDataMap::value_type(vtsPicKey,strPic));
			}	
		}
	}
	//解锁
	pthread_mutex_unlock(&m_PlateMutex);
}

//删除yuv数据
void CRoadCarnumDetect::DeleteYUVBuf(int nDelNum,unsigned int pDelIndex[16])
{
	Vts_Picture_Key vtsPicKey;
	vtsPicKey.uCameraId = m_nCameraID;	
	//加锁
	pthread_mutex_lock(&m_PlateMutex);
	for(int i=0; i<nDelNum; i++)
	{
		vtsPicKey.uSeq = pDelIndex[i];
		VtsPicDataMap::iterator it_map = m_vtsPicMap.find(vtsPicKey);
		if(it_map != m_vtsPicMap.end())
		{
			m_vtsPicMap.erase(it_map);
		}
	}
	//解锁
	pthread_mutex_unlock(&m_PlateMutex);
}


//核查图片是否已经接收进缓存
bool CRoadCarnumDetect::CheckJpgInMap(int64_t uSeq)
{
	if(0 == uSeq)
	{
		return false;
	}
	bool bRet = false;

	int64_t iMinSeq = 0;
	int64_t iMaxSeq = 0;
	
	//加锁
	pthread_mutex_lock(&m_JpgFrameMutex);
	if(m_JpgFrameMap.size() > 1)
	{
		map<int64_t,string>::iterator it_b_min = m_JpgFrameMap.begin();
		iMinSeq = it_b_min->first;

		map<int64_t,string>::iterator it_e_max = --m_JpgFrameMap.end();
		iMaxSeq = it_e_max->first;//最大帧号

		if(uSeq < iMinSeq || uSeq > iMaxSeq)
		{
			bRet = false;
		}
		else
		{
			//根据帧号找到对应的图像序号
			map<int64_t,string>::iterator it_map = m_JpgFrameMap.find(uSeq);
			if(it_map != m_JpgFrameMap.end())
			{
				bRet = true;
			}
			else
			{
				bRet = false;
			}
		}
	}//End of if(m_JpgFrameMap.size() > 1)
	else if(m_JpgFrameMap.size() == 1)
	{
		map<int64_t,string>::iterator it_b_min = m_JpgFrameMap.begin();
		if(it_b_min->first == uSeq)
		{
			bRet = true;
		}
		else
		{
			bRet = false;
		}
	}

	//解锁
	pthread_mutex_unlock(&m_JpgFrameMutex);

	//if(!bRet)
	//{
		//LogNormal("id:%d Wait DSP JPG:[%d] min:%d max:%d!\n", m_nChannelId, uSeq, iMinSeq, iMaxSeq);
	//	LogNormal("Jpg:%d plate:%d vts:%d seq:[%lld] min:%lld max:%lld", m_JpgFrameMap.size(), m_mapInPutResult.size(), m_vtsResult.size(),uSeq,iMinSeq, iMaxSeq);
	//}

	return bRet;
}


//删除一张图片
bool CRoadCarnumDetect::DelOneJpgElem(yuv_video_buf* pHeader)
{
	bool bRet = false;
	//暂存seq->index对应关系
	if(m_JpgFrameMap.size() > MAX_JPG_BUF_SIZE)
	{
		map<int64_t,string>::iterator it_map_b = m_JpgFrameMap.begin();
		map<int64_t,string>::iterator it_map_e = --m_JpgFrameMap.end();

		//LogTrace("Out-JpgMap.log", "=Id[%d]==JpgMap.size()=%d===del seq=%d", \
		m_nChannelId, m_JpgFrameMap.size(), it_map->first);

		if(pHeader != NULL)
		{
			//删除队列里面最老的图
			yuv_video_buf *pheader = (yuv_video_buf*)((it_map_b->second).c_str());						
			int nTimeNow = GetTimeStamp();
			int nDisTimePre = 0;
			int nDisTime = nTimeNow;

			int iCount = 0;
			map<int64_t,string>::iterator it_map_del = it_map_b;

			for(; it_map_b != it_map_e; it_map_b++)
			{
				pheader = (yuv_video_buf*)((it_map_b->second).c_str());
				//从接收记录到现在的时间差
				nDisTime =  nTimeNow -  pheader->uRecvTs;

				if(nDisTime > nDisTimePre)
				{
					//LogNormal("nDisTime:%d nTimeNow:%d ts:%s", \
						nDisTime, nTimeNow, GetTime(pheader->uTimestamp, 3));
					nDisTimePre = nDisTime;
					it_map_del = it_map_b;
					iCount ++;
				}
			}

			m_JpgFrameMap.erase(it_map_del);


			//yuv_video_buf *pHeaderDel = (yuv_video_buf *)((it_map_del->second).c_str());
			//LogNormal("Del seqDel:%lld tsDel:%s iCont:%d !", \
			//	pHeaderDel->nSeq, GetTime(pHeaderDel->ts/1000/1000, 3), iCount);
			//LogNormal("nDisTime:%d  seq:%lld tsCur:%s!", nDisTime, pHeader->nSeq, GetTime(pHeader->ts/1000/1000, 3));

			bRet = true;
		}
	}

	return bRet;
}

//核查违章信息输出标志
int CRoadCarnumDetect::CheckViolationInfo(ViolationInfo &vio)
{
	int nRetFlag = 0;
	UINT32 nTsNow = GetTimeStamp();
	UINT32 nTsDis = abs((int)(nTsNow - vio.carInfo.uTimestamp));

	//LogNormal("nTsDis:%d nTsNow:%d vio.uTimestamp:%d \n", nTsDis, nTsNow, vio.carInfo.uTimestamp);

	//FIX vio.carInfo.uTimestamp并非为入队列时间(暂以相机时间戳为准,需校时)!
	if(nTsDis >= 300)//5分钟之前的记录直接输出
	{
		//LogNormal("nTsDis:%d nTsNow:%d vio.uTimestamp:%d \n", nTsDis, nTsNow, vio.carInfo.uTimestamp);
		nRetFlag = 1;
	}

	return nRetFlag;
}

//设置事件录象长度
void CRoadCarnumDetect::SetCaptureTime( int nCaptureTime)
{
	m_nCaptureTime = nCaptureTime;
	if(g_nDetectMode == 2)
	m_tempRecord.SetCaptureTime(nCaptureTime);
	else
	m_skpRoadRecorder.SetCaptureTime(nCaptureTime);
}

//获取事件开始时间,通过第1帧时间和帧率计算.
void CRoadCarnumDetect::GetEventBeginTime(const ViolationInfo &infoViolation, RECORD_EVENT &event, const TIMESTAMP vtsStamp[3])
{
	int nFlag = 0;//0:取得录像开始时间,1:取得录像结束时间,2:取得录像中间时间
	UINT32 uMinSeq = infoViolation.frameSeqs[0];
	UINT32 uMaxSeq = infoViolation.frameSeqs[2];	

	int nSeqDis1 = infoViolation.frameSeqs[3] - uMinSeq;
	int nSeqDis2 = uMaxSeq - infoViolation.frameSeqs[3];

	if(nSeqDis1 < 0)
	{
		//min S4 < S1 < S2 < S3
		uMinSeq = infoViolation.frameSeqs[3];
	}
	
	if(nSeqDis2 < 0)
	{
		//S1 < S2 < S3 < S4 max
		uMaxSeq = infoViolation.frameSeqs[3];
	}

	if(uMinSeq == infoViolation.frameSeqs[3])
	{		
		nFlag = 0;//Begin
	}
	else if(uMaxSeq == infoViolation.frameSeqs[3])
	{		
		nFlag = 1;//End
	}
	else
	{
		int nSeqDisOrder = abs(nSeqDis1) - abs(nSeqDis2);//比较与头尾帧差距
		if(nSeqDisOrder > 0)//与帧小的差距更大
		{
			nFlag = 1;//End
		}
		else if(0 == nSeqDisOrder)
		{
			nFlag = 2;//Mindle
		}
		else
		{
			nFlag = 0;//Begin
		}
	}

	int nSeqDis = uMaxSeq - uMinSeq;
	int nTimeDis = (int)((nSeqDis * 1.0)/ m_nFrequency + 0.5);
	UINT32 tsKakou = (UINT32)(infoViolation.carInfo.ts / 1000 / 1000);

	//LogNormal("nTimeDis:%d, nSeqDis:%d m_nFrequency:%d %s #", nTimeDis, nSeqDis, m_nFrequency, infoViolation.carInfo.strCarNum);

	if(vtsStamp[0].uTimestamp > 0 && vtsStamp[1].uTimestamp > 0 && vtsStamp[2].uTimestamp > 0)
	{
		UINT32 tsMin = vtsStamp[0].uTimestamp;
		UINT32 tsMax = vtsStamp[2].uTimestamp;

		int nTsDis1 = tsKakou - tsMin;
		int nTsDis2 = tsMax - tsKakou;

		if(nTsDis1 < 0)
		{
			//min S4 < S1 < S2 < S3
			tsMin = tsKakou;
		}

		if(nTsDis2 < 0)
		{
			//S1 < S2 < S3 < S4 max
			tsMax = tsKakou;
		}
		
		nTimeDis = tsMax - tsMin;

		//LogNormal("tsMin:%s, tsMax:%s", GetTime(tsMin,2), GetTime(tsMax,2));
		//LogNormal("nTimeDis:%d nFlag:%d tsKa:%s ", nTimeDis, nFlag, GetTime(tsKakou,2));
	}

	if(0 == nFlag)
	{
		event.uEventEndTime = tsKakou + nTimeDis + 5;
		event.uEventBeginTime = event.uEventEndTime - m_nCaptureTime;
	}
	else if(1 == nFlag)
	{	
		event.uEventBeginTime = tsKakou - nTimeDis - 5;
		event.uEventEndTime = event.uEventBeginTime + m_nCaptureTime;					
	}
	else
	{
		event.uEventBeginTime =  tsKakou - (int)(m_nCaptureTime/2.0 + 0.5);
		event.uEventEndTime = event.uEventBeginTime + m_nCaptureTime;	
	}

	/*
	if(infoViolation.evtType == ELE_RED_LIGHT_VIOLATION)
	{
		event.uEventEndTime += 5;
		event.uEventBeginTime += 5;

		//LogNormal("vts %d %lld, %lld, %lld %lld\n", \
			m_nFrequency, infoViolation.frameSeqs[0], infoViolation.frameSeqs[1], infoViolation.frameSeqs[2], infoViolation.frameSeqs[3]);
		//LogNormal("evt:%lld, %lld %s", event.uEventBeginTime, event.uTime2, GetTime(event.uTime2, 3));		
		//LogNormal("nTDis:%d tDis:%d car:%s %s", \
		nTimeDis, tsBegin - event.uEventBeginTime, infoViolation.carInfo.strCarNum, GetTime(tsBegin,3));
	}
	*/
	//LogTrace("VideoPP.log", "55 eventB:%lld, eventE:%lld nSeqDis:%d tsVts:%lld nTimeDis:%d tDis:%d", \
		event.uEventBeginTime, event.uEventEndTime, nSeqDis, tsBegin, \
		nTimeDis, tsBegin - event.uEventBeginTime );
	//LogTrace("VideoPP.log", "66 [%s-%s] vts:%s m_nFrequency:%d", \
		GetTime(event.uEventBeginTime, 2).c_str(), GetTime(event.uEventEndTime, 2).c_str(), GetTime(tsBegin, 2).c_str(), m_nFrequency);

	//LogNormal("B:%lld,E:%lld nDis:%d tsVts:%lld ", \
	//event.uEventBeginTime, event.uEventEndTime, nSeqDis, tsKakou );
}


//核查待输出记录是否有图
bool CRoadCarnumDetect::CheckOutResult(std::vector<CarInfo>& vResult)
{
	bool bCheck = false;
	std::multimap<int64_t,DSP_CARINFO_ELEM>::iterator it = m_mapInPutResult.begin();
	for(; it != m_mapInPutResult.end(); it++)
	{
		if(it->first > 0)
		{
			bool bCheckJpg = CheckJpgInMap(it->first);		
			
			//图存在,直接输出卡口
			if(bCheckJpg)
			{
				vResult.push_back(it->second.cardNum);
				if(m_mapInPutResult.find(it->first) != it)
				{
					LogError("Seq=%llu\n",it->first);
				}

				//LogNormal("CheckOutResult bCheckJpg:%d seq:%d ", bCheckJpg, it->first);

				pthread_mutex_lock(&m_InPutMutex);
				m_mapInPutResult.erase(it);
				pthread_mutex_unlock(&m_InPutMutex);

				bCheck = true;
				break;
			}
		}		
	}

	//强制,输出5分钟以外的数据 FIX
	if(!bCheck)
	{
		it = m_mapInPutResult.begin();		
		if(it->first > 0)
		{
			//图不存在,且时间在5分钟以前的记录数据,直接删除记录
			int nTimeNow = GetTimeStamp();
			int nTimeDis = nTimeNow - it->second.uTimeRecv;
			
			if(nTimeDis > 300)
			{
				LogNormal("CheckOutResult seq:%d nTimeDis:%d", it->first, nTimeDis);
				pthread_mutex_lock(&m_InPutMutex);
				m_mapInPutResult.erase(it);
				pthread_mutex_unlock(&m_InPutMutex);
			}
			else
			{
				//LogNormal("CheckOutResult ? seq:%d nTimeDis:%d", it->first, nTimeDis);
				//vResult.push_back(it->second.cardNum);
				//pthread_mutex_lock(&m_OutPutMutex);
				//m_mapInPutResult.erase(it);
				//pthread_mutex_unlock(&m_OutPutMutex);
			}
		}		
	}

	return bCheck;
}

//把有图未输出的数据全部输出
int CRoadCarnumDetect::DealOutPutAll()
{
	int nRet = 0;

	std::vector<CarInfo> vResult;
	while((m_mapInPutResult.size() > 0) && (!m_bEndDetect))
	{
		vResult.clear();
		std::multimap<int64_t,DSP_CARINFO_ELEM>::iterator it = m_mapInPutResult.begin();

		//核查待输出记录是否有图
		bool bCheck = CheckOutResult(vResult);

		if(bCheck)
		{
			LogNormal("DealOutPutAll bCheck true seq:%d ", it->first);
			//输出
			OutPutKakou(vResult);

			nRet ++;
		}
		else
		{
			if(it->first > 0)
			{
				//图不存在,直接删除记录
				LogNormal("DealOutPutAll bCheck false seq:%d ", it->first);
				pthread_mutex_lock(&m_InPutMutex);
				m_mapInPutResult.erase(it);
				pthread_mutex_unlock(&m_InPutMutex);
			}
		}
	}

	return nRet;
}


//输出卡口
void CRoadCarnumDetect::OutPutKakou(std::vector<CarInfo>& vResult)
{
	if(m_bEndDetect)
	{
		return;
	}

	vector<ObjSyncData> syncData;

	struct timeval tv1,tv2;
	if(g_nPrintfTime == 1)
	{
		gettimeofday(&tv1,NULL);
	}

	m_nDealFlag = 0;

	//printf("==before in CarNumOutPut=vResult.size()=%d\n", vResult.size());
	pthread_mutex_lock(&m_OutCarnumMutex);
	
	CarNumOutPut(vResult,syncData);
	
	pthread_mutex_unlock(&m_OutCarnumMutex);

	if(g_nPrintfTime == 1)
	{
		gettimeofday(&tv2,NULL);
		FILE* fp = fopen("time.log","ab+");
		fprintf(fp,"CarNumOutPut==t1=%lld,t2=%lld,dt = %lld\n",(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
		fclose(fp);
	}
}

//获取违章图片
bool CRoadCarnumDetect::GetVtsPic(UINT32* frameSeqs, PLATEPOSITION* pTimeStamp, int nPicCount, yuv_video_buf* buf, std::string& strPic1, std::string& strPic2, std::string& strPic3)
{
	bool bRet = false;

	bool bGetPic1 = false;
	bool bGetPic2 = false;
	bool bGetPic3 = false;

	//加锁
	pthread_mutex_lock(&m_JpgFrameMutex);

	printf("==222=m_JpgFrameMutex==\n");

	map<int64_t,string>::iterator it_b = m_JpgFrameMap.begin();
	map<int64_t,string>::iterator it_e = --m_JpgFrameMap.end();
	UINT32 nMinSeq = it_b->first; //最小帧号
	UINT32 nMaxSeq = it_e->first;//最大帧号	

	//LogTrace("MAX_MIN.log", "=in==nMinSeq=%d=nMaxSeq=%d==frameSeqs[%d, %d, %d]\n", \
	nMinSeq, nMaxSeq, frameSeqs[0], frameSeqs[1], frameSeqs[2]);

	printf("===m_JpgFrameMap.size()=%d==\n", m_JpgFrameMap.size());

	//LogNormal("1=nSeq=%lld,nSeq=%lld,nSeq=%lld,nSeq=%lld\n",frameSeqs[0],frameSeqs[1],frameSeqs[2],frameSeqs[3]);
	UINT32 nPreSeq = 0;
	for(int i = 0; i<nPicCount; i++)
	{
		UINT32 uSeq = frameSeqs[i];//frameSeqs[nPicCount-1-i];		

		printf("i=%d,uSeq=%d,nMaxSeq=%d,nMinSeq=%d\n",i,uSeq,nMaxSeq,nMinSeq);

		if(uSeq > nMaxSeq)
		{
			//LogNormal("最大值\n");
			uSeq = nMaxSeq;
		}

		if(uSeq <= nMinSeq)
		{
			uSeq = nMinSeq;
		}

		//根据帧号找到对应的图像序号
		map<int64_t,string>::iterator it_map = m_JpgFrameMap.find(uSeq);
		//如果没有在缓冲区中找到
		if(it_map!=m_JpgFrameMap.end())
		{
			if(i == 0)
			{
				strPic1 = it_map->second;
				bGetPic1 = true;
			}
			else if(i == 1)
			{
				strPic2 = it_map->second;
				bGetPic2 = true;
			}
			else if(i == 2)
			{
				strPic3 = it_map->second;
				bGetPic3 = true;
			}
		}
		else
		{
			//LogNormal("pic not find\n");
			if(i == 0)
			{
				bGetPic1 = false;
				GetPicVtsFromJpgMap(uSeq,nPreSeq,strPic1);
			}
			else if(i == 1)
			{
				bGetPic2 = false;
				GetPicVtsFromJpgMap(uSeq,nPreSeq,strPic2);
			}
			else if(i == 2)
			{
				bGetPic3 = false;
				GetPicVtsFromJpgMap(uSeq,nPreSeq,strPic3);
			}
		}

		if(i == 0)
		{
			buf = (yuv_video_buf*)strPic1.c_str();
		}
		else if(i == 1)
		{
			buf = (yuv_video_buf*)strPic2.c_str();
		}
		else if(i == 2)
		{
			buf = (yuv_video_buf*)strPic3.c_str();
		}

		(pTimeStamp+i)->ts = buf->ts;
		(pTimeStamp+i)->uTimestamp = buf->uTimestamp;
		(pTimeStamp+i)->nRedColor = buf->uTrafficSignal;
		(pTimeStamp+i)->uSeq = buf->nSeq;

		//LogNormal("nSeq=%lld,buf->uTrafficSignal=%d\n",buf->nSeq,buf->uTrafficSignal);
		nPreSeq = buf->nSeq;
	}

	//解锁
	pthread_mutex_unlock(&m_JpgFrameMutex);

	if(1 == nPicCount)
	{
		if(bGetPic1)
		{
			bRet = true;
		}
	}
	else if(2 == nPicCount)
	{
		if(bGetPic1 && bGetPic2)
		{
			bRet = true;
		}
	}
	else if(3 == nPicCount)
	{
		if(bGetPic1 && bGetPic2 && bGetPic3)
		{
			bRet = true;
		}
	}
	else
	{}
	
	//LogNormal("seq[%lld,%lld,%lld] [%d,%d,%d] nPicCount:%d GetVtsPic! ", \
	//	frameSeqs[0], frameSeqs[1], frameSeqs[2], bGetPic1, bGetPic2, bGetPic3, nPicCount);

	return bRet;
}


//删除一张图片,4.2之前版本
bool CRoadCarnumDetect::DelOneJpgElemOld(yuv_video_buf* pHeader)
{
	bool bRet = false;
	//暂存seq->index对应关系
	if(m_JpgFrameMap.size() > MAX_JPG_BUF_SIZE)
	{
		map<int64_t,string>::iterator it_map_b = m_JpgFrameMap.begin();
		map<int64_t,string>::iterator it_map_e = --m_JpgFrameMap.end();

		//LogTrace("Out-JpgMap.log", "=Id[%d]==JpgMap.size()=%d===del seq=%d", \
		m_nChannelId, m_JpgFrameMap.size(), it_map->first);

		if(pHeader != NULL)
		{
			int iSeqDis = (it_map_e->first - pHeader->nSeq);

			if(iSeqDis < 2000)
			{
				yuv_video_buf *pHeaderEnd = (yuv_video_buf *)((it_map_e->second).c_str());
				int64_t iTimeDis = pHeader->ts - pHeaderEnd->ts;
				//LogNormal("1 iD:%lld seq:%lld,tF:%lld", iTimeDis, pHeader->nSeq, pHeader->ts);
				//LogNormal(" 1 sD:%d seq:%lld,tE:%lld!", iSeqDis, pHeaderEnd->nSeq, pHeaderEnd->ts);

				m_JpgFrameMap.erase(it_map_b);				
			}
			else
			{
				yuv_video_buf *pHeaderEnd = (yuv_video_buf *)((it_map_e->second).c_str());
				int64_t iTimeDis = pHeader->ts - pHeaderEnd->ts;

				if(iTimeDis > 0)
				{
					LogNormal("2 iD:%lld seq:%lld tF:%lld,", iTimeDis, pHeader->nSeq, pHeader->ts);
					LogNormal(" 2 sD:%d seq:%lld,tE:%lld!", iSeqDis,pHeaderEnd->nSeq, pHeaderEnd->ts);
					m_JpgFrameMap.erase(it_map_e);
				}
				else
				{
					yuv_video_buf * pHeaderBegin = NULL;
					//遍历map,删除比最新帧时间小的图片
					for(; it_map_b != it_map_e; it_map_b++)
					{
						pHeaderBegin = (yuv_video_buf *)((it_map_b->second).c_str());						
						if(pHeaderBegin->ts < pHeader->ts)
						{
							LogNormal("3 iD:%lld  seq:%lld tF:%lld!", iTimeDis, pHeader->nSeq, pHeader->ts);
							LogNormal(" 3 sD:%d seq:%lld,tB:%lld!", iSeqDis, pHeaderBegin->nSeq, pHeaderBegin->ts);
							break;
						}
					}

					m_JpgFrameMap.erase(it_map_b);
				}
			}

			bRet = true;
		}
	}

	return bRet;
}


//获取Dsp处理结果,4月2号之前版本
void CRoadCarnumDetect::Dsp_PopOutPutOld(std::vector<CarInfo>& vResult)
{
	//加锁
	pthread_mutex_lock(&m_InPutMutex);

	if(m_mapInPutResult.size() > 0)
	{
		std::multimap<int64_t,DSP_CARINFO_ELEM>::iterator it = m_mapInPutResult.begin();
		vResult.push_back(it->second.cardNum);
		if(m_mapInPutResult.find(it->first) != it)
		{
			LogError("Seq=%llu\n",it->first);
		}
		m_mapInPutResult.erase(it);
	}

	//解锁
	pthread_mutex_unlock(&m_InPutMutex);
}

//核查违章图片是否都存在,不全则不输出
bool CRoadCarnumDetect::CheckVtsPic(const ViolationInfo &infoViolation, TIMESTAMP *pStamp)
{
	bool bCheck = false;

	bool bCheckJpg[3] = {false, false, false};

	int nCount = 0;
	while((nCount < 6) && (!m_bEndDetect))
	{
		sleep(1);

		for(int i=0; i<3; i++)
		{
			if(!bCheckJpg[i])
			{
				bCheckJpg[i] = CheckJpgInMap(infoViolation.frameSeqs[i]);
			}			
		}

		if(bCheckJpg[0] && bCheckJpg[1] && bCheckJpg[2])
		{
			bCheck = true;
			break;
		}
		nCount++;
	}	

	if(!bCheck)
	{
		LogNormal("CheckVtsPic [%d,%d,%d] [%lld,%lld,%lld] n:%d \n", \
			bCheckJpg[0], bCheckJpg[1], bCheckJpg[2],\
			infoViolation.frameSeqs[0], infoViolation.frameSeqs[1], infoViolation.frameSeqs[2], nCount);
	}
	else
	{
		if(m_bEventCapture)
		{
			//取违章时间戳
			for(int i=0; i<3; i++)
			{
				GetJpgTimStampInMap(infoViolation.frameSeqs[i], *(pStamp+i));			
			}
		}		
	}

	return bCheck;
}


//获取对应帧号,取图,二次识别检出车型细分类型
int CRoadCarnumDetect::GetTypeDetailTwo(
	UINT32 uSeq,
	CarInfo &cardNum,
	RECORD_PLATE &plate, 
	carnum_context &context,
	PLATEPOSITION* pTimeStamp)
{
	int nTypeDetail = -1;

	//IplImage * imgSnap = cvCreateImage(cvSize(m_imgSnap->width,m_imgSnap->height+m_nExtentHeight),8,3);
	//if(NULL == imgSnap)
	//{
	//	return -1;
	//}

	if(g_nDetectMode == 2)
	{
		context.nCarNumDirection = (carnumdirection)m_nDetectDirection;
	}
	else
	{
		context.nCarNumDirection = (carnumdirection)cardNum.nDirection;//目标运动方向
	}

	//获取车辆类型（高16位卡车、巴士、轿车等，低16位大、中、小车）
	if(m_nDetectKind & DETECT_TRUCK)
	{
		//LogNormal("B GetImageByIndex seq:%d \n", uSeq);
		//bool bGetJpg = GetImageByIndex(uSeq,0,pTimeStamp, imgSnap);
		bool bGetJpg = GetImageByIndex(uSeq,0,pTimeStamp, m_imgSnap);
		if(bGetJpg)
		{
			//设置图像实际大小（不包含下面的黑色区域,）
			CvRect rtRealImage;
			rtRealImage.x = 0;
			rtRealImage.y = 0;
			rtRealImage.width = m_imgSnap->width;
			rtRealImage.height = m_imgSnap->height - m_nExtentHeight;

			//LogNormal("rtRealImage:%d,%d,%d,%d imgSnap:%d,%d", \
			//	rtRealImage.x, rtRealImage.y,rtRealImage.width, rtRealImage.height, \
			//	imgSnap->width, imgSnap->height);
			if(m_nWordPos == 1)
			{
				rtRealImage.y += m_nExtentHeight;
			}

			//LogNormal("B TruckDetect uTypeDetail:%d \n", plate.uTypeDetail);
			//TruckDetect(cardNum,plate,context,rtRealImage,imgSnap);
			if(m_nDetectDirection == 0)//前牌由车标模块输出车型
			{
				 CvRect rtRoi;
				 rtRoi.x = m_rtCarnumROI.x;
				 rtRoi.width = m_rtCarnumROI.width;
				 rtRoi.y = m_rtCarnumROI.y;
				 rtRoi.height = m_rtCarnumROI.height;

				CvPoint loopPtUp;
				CvPoint loopPtDowon;

				if(rtRoi.width < m_imgSnap->width && rtRoi.width > 0)
				CarLabelDetect(cardNum,plate,context,loopPtUp,loopPtDowon,rtRoi);
			}
			else
			{
				DetectTruck(cardNum,plate,context,rtRealImage,m_imgSnap);
			}
			
			//cvDrawRect(m_imgSnap, cvPoint(context.position.x, context.position.y), cvPoint(context.position.x+context.position.width-1, context.position.y+context.position.height-1), CV_RGB(255,0,0), 2);
			//cvSaveImage("test.jpg",m_imgSnap);
			nTypeDetail = plate.uTypeDetail;
			LogNormal("E TruckDetect uTypeDetail:%d-%d \n", plate.uTypeDetail,m_nDetectDirection);

			//济南黄网格的版本,dsp结果进行二次识别,大货车禁行
			//大货禁行直接在此输出
			//if(((m_nDetectKind&DETECT_VIOLATION)!=DETECT_VIOLATION))
			//{
			//	GetVtsResult(cardNum, plate);
			//}
		}
		else
		{
			LogNormal("GetImageByIndex failed! useq:%lld \n", uSeq);
		}
	}

	//if(imgSnap)
	//{
	//	cvReleaseImage(&imgSnap);
	//}

	return nTypeDetail;
}

//处理二次识别检出车型细分类型,针对不同车型细分进行处理
bool CRoadCarnumDetect::DealVtsTypeDetail(int nTypeDetail)
{
	bool bRet = false;
	
	if(nTypeDetail > -1)
	{
		//LogNormal("Big nTypeDetail:%d", nTypeDetail);
		if(TRUCK_TYPE == nTypeDetail)//大货车才输出
		{

			bRet = true;
		}
		else if(BUS_TYPE == nTypeDetail)//大客
		{
			if(g_nServerType == 13)
			{
				bRet = true;
			}
		}
		else if(WRONG_POS == nTypeDetail)//车位子太偏
		{
			LogNormal("车牌位置太偏!");
		}
		else{}
	}

	return bRet;
}

//获取图像宽高
bool CRoadCarnumDetect::GetImageWidthHeight(int& nWidth,int& nHeight)
{
	if(m_imgSnap != NULL)
	{
		return true;
	}
	
	LogNormal("m_JpgFrameMap.size()=%d\n",m_JpgFrameMap.size());
	while(!m_bEndDetect)
	{
		if(m_JpgFrameMap.size() < 1)
		{
			nWidth = 0;
			nHeight = 0;
			sleep(1);
			continue;
		}
		else
		{
			LogNormal("m_bEndDetect=%d,m_JpgFrameMap.size()=%d\n",m_bEndDetect,m_JpgFrameMap.size());
			string strPic("");
			map<int64_t,string>::iterator it_b;
			//加锁
			pthread_mutex_lock(&m_JpgFrameMutex);
			if(m_JpgFrameMap.size() > 0)
			{
				it_b = m_JpgFrameMap.begin();
				if(it_b != m_JpgFrameMap.end())
				{
					strPic = it_b->second;
				}
			}
			//解锁
			pthread_mutex_unlock(&m_JpgFrameMutex);
			LogNormal("strPic.size()=%d\n",strPic.size());
			if(strPic.size() > sizeof(yuv_video_buf))
			{
					//需要解码jpg图像
					 CxImage image;
					 image.Decode((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)), strPic.size() - sizeof(yuv_video_buf), 3); //解码
					 
					 if(image.IsValid()&&image.GetSize()>0)
					 {
						nWidth = image.GetWidth();
						nHeight = image.GetHeight();
						LogNormal("GetImageWidthHeight w=%d,h=%d\n",nWidth,nHeight);
						if(nWidth <= 1000 || nHeight <= 1000 || nWidth >= 5000 || nHeight >= 5000 )
						{
							if(nWidth < 2000)
							{
								m_nFrequency = 15;
							}
							else if(nWidth < 2700)
							{
								m_nFrequency = 10;
							}
							else 
							{
								m_nFrequency = 25;
							}
							LogNormal("jpg图片宽高错误\n");
							//加锁
							pthread_mutex_lock(&m_JpgFrameMutex);
							m_JpgFrameMap.erase(it_b);
							//解锁
							pthread_mutex_unlock(&m_JpgFrameMutex);
							continue;
						}
						break;
					 }
					 else //解码失败！！
					 {
						 LogNormal("解码失败\n");
						 //加锁
						pthread_mutex_lock(&m_JpgFrameMutex);
						m_JpgFrameMap.erase(it_b);
						//解锁
						pthread_mutex_unlock(&m_JpgFrameMutex);
						 continue;
					 }			 
			}
			else
			{
				LogNormal("jpg图片大小错误\n");
				//加锁
				pthread_mutex_lock(&m_JpgFrameMutex);
				m_JpgFrameMap.erase(it_b);
				//解锁
				pthread_mutex_unlock(&m_JpgFrameMutex);
				continue;
			}
		}
	}

	return true;
}

//分配内存
void CRoadCarnumDetect::AlignMemory(int uWidth,int uHeight)
{
	if(m_imgSnap != NULL)
	{
		return;
	}

	if (g_nServerType == 13)
	{
			//对于上海电警项目将字符高度固定
				int nFontSize = 64;
				if(uWidth < 2000)
				{
					m_nExtentHeight = 64;
					nFontSize = 64;
				}
				else if(uWidth < 2500)
				{
					m_nExtentHeight = 96;
					nFontSize = 96;
				}
				else 
				{
					m_nExtentHeight = 160;
					nFontSize = 160;
				}

				if(g_DistanceHostInfo.bDistanceCalculate == 1)
				{
					g_mvsCommunication.SetFontSize(nFontSize);
					g_mvsCommunication.SetExtentHeight(m_nExtentHeight);
				}

				if (g_PicFormatInfo.nSmallPic != 1)
				{
					g_MyCenterServer.SetExtentHeight(m_nExtentHeight);
				}
				
				g_MyCenterServer.SetRatio(uWidth*1.0/uHeight);

				m_PicFormatInfo = g_PicFormatInfo;
				m_nJiaoJinCvText.Init(m_nExtentHeight/2);
				m_cvText.Init(nFontSize);
	}

	m_pCurJpgImage = new BYTE[uWidth*uHeight];

    m_pPreJpgImage = new BYTE[uWidth*uHeight];

    m_pSmallJpgImage = new BYTE[uWidth*uHeight/4];

    m_pComposeJpgImage = new BYTE[uWidth*uHeight*3];

	#ifdef DSP_500W_TEST
		m_imgDestSnap2 = cvCreateImage(cvSize(1600*m_nSaveImageCount,(1200+m_nExtentHeight)),8,3);
		m_imgComposeSnap2 = cvCreateImage(cvSize(1600,(1200+m_nExtentHeight)*3),8,3);
	#endif

	m_imgFrame = cvCreateImageHeader(cvSize(uWidth,uHeight),8,3);

	m_imgSnap = cvCreateImage(cvSize(uWidth,uHeight+m_nExtentHeight),8,3);
    m_imgPreSnap = cvCreateImage(cvSize(uWidth,uHeight+m_nExtentHeight),8,3);

	LogNormal("m_imgSnap->width=%d,m_imgSnap->height=%d\n",m_imgSnap->width, m_imgSnap->height);

	//多张图片叠加存放
     if(m_nSmallPic == 1)
	 {
		if(m_nSaveImageCount == 2)
		{
			m_imgDestSnap = cvCreateImage(cvSize(uWidth*m_nSaveImageCount+uHeight,(uHeight+120)),8,3);
		}
		else
		{
			m_imgDestSnap = cvCreateImage(cvSize(uWidth*m_nSaveImageCount+uHeight,(uHeight+m_nExtentHeight)),8,3);
		}
	}
    else
    {
		if((m_nSaveImageCount == 2) && (g_nPicMode == 2))
		{
			m_imgDestSnap = cvCreateImage(cvSize(uWidth,(uHeight+m_nExtentHeight)*m_nSaveImageCount),8,3);
		}
		else
		{
			m_imgDestSnap = cvCreateImage(cvSize(uWidth*m_nSaveImageCount,(uHeight+m_nExtentHeight)),8,3);
		}	
	}

	
	 if(g_nDoSynProcess == 1)
	 {
		pthread_mutex_lock(&m_imgTagMutex);
		for(int i=0; i<MAX_IMG_TAG_NUM; i++)
		{
			m_imgTagList[i].uKey = 0;
			m_imgTagList[i].uTime = 0;
			m_imgTagList[i].uLastTime = 0;
			int nSize = uWidth * (uHeight+m_nExtentHeight) / 2;
			m_imgTagList[i].pImg = new char[nSize];
			m_imgTagList[i].bUse = false;
			m_imgTagList[i].nUseCount = 0;
		}
		pthread_mutex_unlock(&m_imgTagMutex);

		if(m_pMachPlate)
		{
			int nSize = uWidth * (uHeight + m_nExtentHeight) / 2;			
			for(int i=0; i<4; i++)
			{
				m_pMachPlate->pPicArray[i] = new char[nSize];
				m_pMachPlate->bKeyStateArray[i] = false;
				m_pMachPlate->uKeyArray[i] = 0;
			}
			//m_pMachPlate->bUse = false;
		}

	 }

	uWidth = m_imgSnap->width;
	uHeight = m_imgSnap->height;
	m_imgComposeResult = cvCreateImage(cvSize(uWidth,uHeight),8,3);

	if(m_nWordOnPic == 1)
	{
		uHeight -= m_nExtentHeight;
	}
	//闯红灯检测三帧合成图像
	if(g_nVtsPicMode == 0) //3x1
	{
		//LogNormal("m_imgComposeSnap cvCreateImage uWidth=%d,uHeight=%d", uWidth, uHeight);
		m_imgComposeSnap = cvCreateImage(cvSize(uWidth,(uHeight)*3),8,3);
	}
	else if(g_nVtsPicMode == 1) //2x2
	{
		m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*2),8,3);
	}
	else if(g_nVtsPicMode == 2) //3x2
	{
		m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*3),8,3);
	}
	else if(g_nVtsPicMode == 3)//1x2
	{
		if(1 == g_PicFormatInfo.nSmallViolationPic)
		{
				if(g_nServerType == 3)
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2+uHeight,(uHeight-m_nExtentHeight+120)),8,3);//旅行时间违章图片格式
				}
				else
				{
					m_imgComposeSnap = cvCreateImage(cvSize(800,1180),8,3);//武汉倒品字形图片
				}
		}
		else
		{
				m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)),8,3);
		}
	}
	else if(g_nVtsPicMode == 5)//1x3
	{
		if(1 == g_PicFormatInfo.nSmallViolationPic)
		{
				if(g_nServerType == 23 || g_nServerType == 26)//济南图片格式
				{
					    m_imgComposeSnap = cvCreateImage(cvSize(uWidth*4,uHeight),8,3);
				}
				else //遵义违章格式
				{
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth*4,(uHeight-m_nExtentHeight+120)),8,3);
				}
		}
		else
		{
				if(g_nServerType == 29)
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth*3,(uHeight-m_nExtentHeight)),8,3);
				}
				else
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth*3,(uHeight)),8,3);
				}
		}
	}

		if (g_nServerType == 13)
		{
			if(g_PicFormatInfo.nSmallPic != 1)
			{
				g_MyCenterServer.SetPicWidth(m_imgSnap->width);
				g_MyCenterServer.SetPicHeight(m_imgSnap->height);
			}
		}
}

//取违章时间戳
bool CRoadCarnumDetect::GetJpgTimStampInMap(const int64_t uSeq, TIMESTAMP &timeStamp)
{
	bool bRet = false;

	//加锁
	pthread_mutex_lock(&m_JpgFrameMutex);
	if(m_JpgFrameMap.size() > 0)
	{
		//根据帧号找到对应的图像序号
		map<int64_t,string>::iterator it_map = m_JpgFrameMap.find(uSeq);
		if(it_map != m_JpgFrameMap.end())
		{
			yuv_video_buf *pHeader= (yuv_video_buf *)((it_map->second).c_str());
			timeStamp.uTimestamp = pHeader->uTimestamp;
			bRet = true;
		}
		else
		{
			bRet = false;
		}
		
	}//End of if(m_JpgFrameMap.size() > 1)	

	//解锁
	pthread_mutex_unlock(&m_JpgFrameMutex);

	return bRet;
}

//#ifdef FBMATCHPLATE
//添加输出违章结果到比对队列
bool CRoadCarnumDetect::AddDspPlateMatchVts(const RECORD_PLATE &plate, RECORD_PLATE_DSP_MATCH * pMatchPlate)
{
	//LogTrace("./Log/DspPlateMatch.txt", \
		"AddDspPlateMatch-seq-%d,uChannel=%d-nRoad=%d---plate.uDirection=%d-chText=%s-img=/*%x*/  c1:%d c2:%d,type=%d \n", \
		plate.uSeq, plate.uChannelID, plate.uRoadWayID,plate.uDirection, plate.chText, /*pImgSnap,*/ plate.uCarColor1, plate.uCarColor2,plate.uViolationType);	
	//if(pImgList[0].pImg == NULL || pImgList[1].pImg == NULL || pImgList[2].pImg == NULL)
	if(pMatchPlate == NULL)
	{
		return false;
	}
	else
	{
		//RECORD_PLATE_DSP_MATCH plateMatch;
		memcpy((char*)(&pMatchPlate->dspRecord), (char*)(&plate), sizeof(RECORD_PLATE));
	
		if(pMatchPlate->dspRecord.uChannelID != plate.uChannelID) 
		{
			//(pMatchPlate->dspRecord.uSeq != plate.uSeqID) 
			//LogNormal("uSeqID:%d,%d, chan:%d,%d m_pMachPlate err1!", \
			//	m_pMachPlate->dspRecord.uSeqID, plate.uSeqID, m_pMachPlate->dspRecord.uChannelID, plate.uChannelID);

			return false;
		}
		
		if(0 == pMatchPlate->dspRecord.uViolationType)//卡口
		{
			if(pMatchPlate->dspRecord.uSeqID != pMatchPlate->pPlatePos[0].uSeq)
			{
				LogNormal("卡口 id:%d car:%s uPic:%d uPlate:%d dir:%d", \
						 pMatchPlate->dspRecord.uChannelID,
						 pMatchPlate->dspRecord.chText, 
						 pMatchPlate->pPlatePos[0].uSeq, 
						 pMatchPlate->dspRecord.uSeqID,
						 pMatchPlate->dspRecord.uDetectDirection);
						 
				if(0 == pMatchPlate->dspRecord.uDetectDirection)//前牌
				{
					int nSeqDis = abs(pMatchPlate->dspRecord.uSeqID - pMatchPlate->pPlatePos[0].uSeq);
					if(nSeqDis > 10)
					{
						return false;
					}					
				}
			}
		}
		else
		{
#ifdef MATCH_LIU_YANG_DEBUG
			LogNormal("car:%s uPlate:%d uPic:%d,%d,%d,%d vio:%d", \
					 pMatchPlate->dspRecord.chText, 
					 pMatchPlate->dspRecord.uSeqID,
					 pMatchPlate->pPlatePos[0].uSeq,
					 pMatchPlate->pPlatePos[1].uSeq,
					 pMatchPlate->pPlatePos[2].uSeq,
					 pMatchPlate->pPlatePos[3].uSeq, 
					 pMatchPlate->dspRecord.uViolationType);
#endif
		}
	}
	
	if(m_strDeviceId.size() > 0 && m_strDeviceId.size() < MAX_DEVICE_CODE)
	{
		memcpy(pMatchPlate->szDeviceID, m_strDeviceId.c_str(), m_strDeviceId.size());
	}
	
	if(m_strLocation.size() > 0)
	{
		memcpy(pMatchPlate->dspRecord.chPlace, m_strLocation.c_str(),m_strLocation.size());
	}
	
	//just for test
#ifdef TEST_MATCH
	if(plate.uViolationType > 0)
	{
		pMatchPlate->dspRecord.uChannelID ++;//just for test
		pMatchPlate->dspRecord.uDetectDirection = 1;//强制后牌
		std::string strDeviceId = "012345678901234567";
		memcpy(pMatchPlate->szDeviceID, strDeviceId.c_str(), strDeviceId.size());
	}
	else
	{
		pMatchPlate->dspRecord.uDetectDirection = 0;
	}
	/*
	CvRect rtPos = GetCarPos(pMatchPlate->dspRecord, 2);
	pMatchPlate->dspRecord.uCarPosLeft = rtPos.x;
	pMatchPlate->dspRecord.uCarPosTop = rtPos.y;
	pMatchPlate->dspRecord.uCarPosRight = rtPos.x + rtPos.width;
	pMatchPlate->dspRecord.uCarPosBottom = rtPos.y + rtPos.height ;
	*/

	pMatchPlate->dspRecord.uCarPosLeft = 500;
	pMatchPlate->dspRecord.uCarPosTop = 500;
	pMatchPlate->dspRecord.uCarPosRight = 1000;
	pMatchPlate->dspRecord.uCarPosBottom = 1000 ;
#endif
	
	pMatchPlate->dspRecord.uPicWidth = m_imgSnap->width;
	pMatchPlate->dspRecord.uPicHeight =  m_imgSnap->height - m_nExtentHeight;

	//LogNormal("pImgList:%d,%d,%d,%d", pImgList[0].bUse, pImgList[1].bUse, pImgList[2].bUse, pImgList[3].bUse);
	//LogNormal("pMatchPlate->bUse:%d ", pMatchPlate->bUse);

	//LogNormal("MatchVts id2:%d car:%s dir:%d vio:%d", \
		pMatchPlate->dspRecord.uChannelID, pMatchPlate->dspRecord.chText, pMatchPlate->dspRecord.uDetectDirection, pMatchPlate->dspRecord.uViolationType);
	
	//LogNormal("inPutJpg: %d,%d,%d,%d", \
		pMatchPlate->nSizeArray[0], pMatchPlate->nSizeArray[1], \
		pMatchPlate->nSizeArray[2], pMatchPlate->nSizeArray[3]);

#ifdef MATCH_LIU_YANG_DEBUG
	//LogNormal("AddDspPlateMatchVts l:%d,t:%d,r:%d,b:%d", 
	//	pMatchPlate->dspRecord.uCarPosLeft, pMatchPlate->dspRecord.uCarPosTop, 
	//	pMatchPlate->dspRecord.uCarPosRight, pMatchPlate->dspRecord.uCarPosBottom);
	//LogNormal("plate: %d,%d,%d,%d ", plate.uPosLeft, plate.uPosTop, plate.uPosRight, plate.uPosBottom);

	if((0 == plate.uCarPosLeft) && (0 == plate.uCarPosTop) && 
		(0 == plate.uCarPosRight) && (0 == plate.uCarPosBottom))
	{
		CvRect rtCar = GetCarPos(plate, 2);

		pMatchPlate->dspRecord.uCarPosLeft = rtCar.x;
		pMatchPlate->dspRecord.uCarPosTop = rtCar.y;
		pMatchPlate->dspRecord.uCarPosRight = rtCar.x + rtCar.width;
		pMatchPlate->dspRecord.uCarPosBottom = rtCar.y + rtCar.height ;
		LogNormal("AddDspPlateMatchVts2 l:%d,t:%d,r:%d,b:%d", 
			pMatchPlate->dspRecord.uCarPosLeft, pMatchPlate->dspRecord.uCarPosTop, 
			pMatchPlate->dspRecord.uCarPosRight, pMatchPlate->dspRecord.uCarPosBottom);
	}
#endif

	if(pMatchPlate->dspRecord.uChannelID == 0)
	{
		//LogNormal("NNid:%d AddDspPlateMatchVts uKey:%d,%d,%d,%d car:%s", \
		//	m_nChannelId, pMatchPlate->uKeyArray[0], pMatchPlate->uKeyArray[1], pMatchPlate->uKeyArray[2], pMatchPlate->uKeyArray[3], pMatchPlate->dspRecord.chText);
		pMatchPlate->dspRecord.uChannelID = m_nChannelId;
	}

#ifdef DEBUG_LIUYANG
	LogNormal("id:%d AddDspPlateMatchVts uKey:%d,%d,%d,%d car:%s", \
		m_nChannelId, pMatchPlate->uKeyArray[0], pMatchPlate->uKeyArray[1], pMatchPlate->uKeyArray[2], pMatchPlate->uKeyArray[3], pMatchPlate->dspRecord.chText);
	LogNormal("car:%s uPlate:%d uPic:%d,%d,%d,%d vio:%d", \
		pMatchPlate->dspRecord.chText, 
		pMatchPlate->dspRecord.uSeqID,
		pMatchPlate->pPlatePos[0].uSeq,
		pMatchPlate->pPlatePos[1].uSeq,
		pMatchPlate->pPlatePos[2].uSeq,
		pMatchPlate->pPlatePos[3].uSeq, 
		pMatchPlate->dspRecord.uViolationType);
#endif

	//just for test
	//pMatchPlate->dspRecord.uDetectDirection = 1;//强制尾牌
	//test end

	if(pMatchPlate->dspRecord.uViolationType != 0)
	{
		if(pMatchPlate->uKeyArray[0] == 0)
		{
			pMatchPlate->uKeyArray[0] = pMatchPlate->uKeyArray[3];
			pMatchPlate->bKeyStateArray[0] = pMatchPlate->bKeyStateArray[3];
		}
	}	
	
	g_matchPlateFortianjin.mvInput(*pMatchPlate);
}
//#endif

//#ifdef MATCH_LIU_YANG
//获取比对违章检测处理结果
void CRoadCarnumDetect::PopVtsOutPutMatch(std::vector<ViolationInfo>& vResult)
{
	//加锁
	pthread_mutex_lock(&m_OutPutMutex);

	if(m_vtsResult.size() > 0)
	{
		std::vector<ViolationInfo>::iterator it = m_vtsResult.begin();

		//核查违章记录是否输出
		bool bCheck = CheckVtsOutPutMatch(*it);

		if(bCheck)
		{
			vResult.push_back(*it);
			//先判断
			m_vtsResult.erase(it);
		}
	}
	//解锁
	pthread_mutex_unlock(&m_OutPutMutex);
}
//#endif

//#ifdef MATCH_LIU_YANG
//核查违章记录是否输出
bool CRoadCarnumDetect::CheckVtsOutPutMatch(const ViolationInfo & vioInfo)
{
	bool bRet = false;

	//记录与本地时间,时间差大于15秒钟,才输出
	UINT32 uTsCar = vioInfo.carInfo.uTimestamp;
	UINT32 uTsNow = GetTimeStamp();
	UINT32 uDis = abs(uTsNow - uTsCar);
	if( uDis > 15)
	{
		bRet = true;
	}

	return bRet;
}
//#endif

//#ifdef MATCH_LIU_YANG
//重设m_pMatchPlate
void CRoadCarnumDetect::ReInitMatchPlate(RECORD_PLATE_DSP_MATCH * pMatchPlate)
{
	if(pMatchPlate)
	{	
		//LogNormal("ReInitMatchPlate id:%d, uKey[%d,%d,%d,%d]", \
		//	m_nChannelId, pMatchPlate->uKeyArray[0], pMatchPlate->uKeyArray[1], pMatchPlate->uKeyArray[2], pMatchPlate->uKeyArray[3]);
		
		memset(pMatchPlate->dspRecord.chText, 0, sizeof(pMatchPlate->dspRecord.chText));
		
		for(int i=0; i<4; i++)
		{
			//pMatchPlate->uSeqArray[i] = 0;	
			pMatchPlate->pPicArray[i] = NULL;
			pMatchPlate->nSizeArray[i] = 0;
			pMatchPlate->bKeyStateArray[i] = false;
			pMatchPlate->uKeyArray[i] = 0;
		}
		
		//pMatchPlate->bUse = false;
	}
}
//#endif

//#ifdef MATCH_LIU_YANG
//添加一张图到比对列表
void CRoadCarnumDetect::AddJpgToMatch(const string &strPic, const int &nIndex, RECORD_PLATE_DSP_MATCH * pMatchPlate)
{
	std::string strJpg = strPic;
	if(strJpg.size() > 0)
	{
		yuv_video_buf * pHead = (yuv_video_buf*)(strPic.c_str());
		int64_t ts = pHead->ts;
		strJpg.erase(0,sizeof(yuv_video_buf));//去除包头

		if(nIndex < 4)
		{	
			pthread_mutex_lock(&m_imgTagMutex);
			for(int i=0; i<MAX_IMG_TAG_NUM; i++)
			{
				if(!m_imgTagList[i].bUse)
				{
					m_uCurrImgtag = i;
					break;
				}
			}

			pthread_mutex_unlock(&m_imgTagMutex);

			if(m_imgTagList[m_uCurrImgtag].bUse)
			{
				//无可用存储空间
				//LogNormal("AddJpgToMatch err! m_uCurrImgtag:%d key:%lld", \
				//	m_uCurrImgtag, m_imgTagList[m_uCurrImgtag].uKey);

				//强制删除最老的2条记录
				DelOldestImgs(2);

				pthread_mutex_lock(&m_imgTagMutex);
				for(int i=0; i<MAX_IMG_TAG_NUM; i++)
				{
					if(!m_imgTagList[i].bUse)
					{
						m_uCurrImgtag = i;
						break;
					}
				}
				pthread_mutex_unlock(&m_imgTagMutex);
			}

			pthread_mutex_lock(&m_imgTagMutex);

			if(!m_imgTagList[m_uCurrImgtag].bUse)
			{
				//更新uImgKey标记,时间戳
				UpdateImgKey(m_imgTagList[m_uCurrImgtag].uKey, m_imgTagList[m_uCurrImgtag].uTime);

				if(3 == nIndex)
				{
					pMatchPlate->pPicArray[0] = m_imgTagList[m_uCurrImgtag].pImg;
					memcpy((void *)(pMatchPlate->pPicArray[0]), strJpg.c_str(), strJpg.size());

					pMatchPlate->nSizeArray[0] = strJpg.size();
					pMatchPlate->pPlatePos[0].ts = ts;
					pMatchPlate->pPlatePos[0].uSeq = pHead->nSeq;

					pMatchPlate->uKeyArray[0] = m_imgTagList[m_uCurrImgtag].uKey;
					pMatchPlate->bKeyStateArray[0] = true;//存在uKey对应记录图片
				}
				else
				{
					pMatchPlate->pPicArray[nIndex+1] = m_imgTagList[m_uCurrImgtag].pImg;				
					memcpy((void *)(pMatchPlate->pPicArray[nIndex+1]), strJpg.c_str(), strJpg.size());

					pMatchPlate->nSizeArray[nIndex+1] = strJpg.size();
					pMatchPlate->pPlatePos[nIndex+1].ts = ts;
					pMatchPlate->pPlatePos[nIndex+1].uSeq = pHead->nSeq;

					pMatchPlate->uKeyArray[nIndex+1] = m_imgTagList[m_uCurrImgtag].uKey;
					pMatchPlate->bKeyStateArray[nIndex+1] = true;//存在uKey对应记录图片					
				}				
				m_imgTagList[m_uCurrImgtag].bUse = true;//更新记录使用状态
				m_imgTagList[m_uCurrImgtag].uLastTime = 0;

			//#ifdef DEBUG_LIUYANG
			//	LogNormal("id:%d id2:%d AddJpgToMatch m_uCurr:%d uKey:%lld seq:%lld car:%s", \
			//		m_nChannelId, pMatchPlate->dspRecord.uChannelID, m_uCurrImgtag, m_imgTagList[m_uCurrImgtag].uKey, pHead->nSeq, pMatchPlate->dspRecord.chText);
			//#endif

				//m_uCurrImgtag++;

				//if(MAX_IMG_TAG_NUM == m_uCurrImgtag)
				//{
				//	m_uCurrImgtag = 0;
				//}
				//m_imgTagList[m_uCurrImgtag].bUse = false;
			}
			
			
			pthread_mutex_unlock(&m_imgTagMutex);
		}
	}

//#ifdef DEBUG_LIUYANG
//	LogNormal("AddJpgToMatch uKey:%d,%d,%d,%d car:%s", \
//		pMatchPlate->uKeyArray[0], pMatchPlate->uKeyArray[1], pMatchPlate->uKeyArray[2], pMatchPlate->uKeyArray[3], pMatchPlate->dspRecord.chText);
//#endif
}
//#endif

#ifdef REDADJUST
//初始化,红灯增强,算法接口2
bool CRoadCarnumDetect::InitRedAdjust2()
{	
	bool bRet = false;
	
	int nRoadWayCounts = 0;
	m_nRedNum = 0;
	LightExpHeader LightExpRed[MAX_RED_LIGHT_NUM];//红灯的矩形区域
	vector<ChannelRegion>::iterator it_r = m_vtsObjectRegion.begin();
	while(it_r != m_vtsObjectRegion.end())
	{
		m_RedDirectionArray[nRoadWayCounts].uRoadWayId = it_r->nRoadIndex;

		//左边灯头
		if(it_r->roiLeftLight.width > 0 || it_r->roiLeftLight_red.width > 0 || it_r->roiLeftLight_green.width > 0)
		{
			CopyRect(it_r->roiLeftLight, LightExpRed[m_nRedNum].LightRegion);
			CopyRect(it_r->roiLeftLight_red, LightExpRed[m_nRedNum].rLightRect);//RED			
			CopyRect(it_r->roiLeftLight_green, LightExpRed[m_nRedNum].gLightRect);//GREEN
			m_RedDirectionArray[nRoadWayCounts].bLeft = true;
			m_nIndexRedArray[m_nRedNum] = m_nRedNum;
			//YELLOW,不需对黄灯进行增强
			m_nRedNum++;

			LogNormal("m_nRedNum:%d InitRedAdjust2 left road:%d", m_nRedNum, it_r->nRoadIndex);
		}
		
		//中间灯头
		if(it_r->roiMidLight.width > 0 || it_r->roiMidLight_red.width > 0 || it_r->roiMidLight_green.width > 0)
		{
			CopyRect(it_r->roiMidLight, LightExpRed[m_nRedNum].LightRegion);
			CopyRect(it_r->roiMidLight_red, LightExpRed[m_nRedNum].rLightRect);//RED			
			CopyRect(it_r->roiMidLight_green, LightExpRed[m_nRedNum].gLightRect);//GREEN			
			m_RedDirectionArray[nRoadWayCounts].bStraight = true;
			m_nIndexRedArray[m_nRedNum] = m_nRedNum;
			//YELLOW,不需对黄灯进行增强
			m_nRedNum++;

			LogNormal("m_nRedNum:%d InitRedAdjust2 mid road:%d", m_nRedNum, it_r->nRoadIndex);
		}
	
		//右边灯头
		if(it_r->roiRightLight.width > 0 || it_r->roiRightLight_red.width > 0 || it_r->roiRightLight_green.width > 0)
		{
			CopyRect(it_r->roiRightLight, LightExpRed[m_nRedNum].LightRegion);
			CopyRect(it_r->roiRightLight_red, LightExpRed[m_nRedNum].rLightRect);//RED			
			CopyRect(it_r->roiRightLight_green, LightExpRed[m_nRedNum].gLightRect);//GREEN
			m_RedDirectionArray[nRoadWayCounts].bRight = true;
			m_nIndexRedArray[m_nRedNum] = m_nRedNum;
			//YELLOW,不需对黄灯进行增强
			m_nRedNum++;

			LogNormal("m_nRedNum:%d InitRedAdjust2 right road:%d", m_nRedNum, it_r->nRoadIndex);
		}
		/*
		//转弯灯头
		if(it_r->roiTurnAroundLight.width > 0 || it_r->roiTurnAroundLight_red.width > 0 || it_r->roiTurnAroundLight_green.width > 0)
		{
			CopyRect(it_r->roiTurnAroundLight, LightExpRed[m_nRedNum].LightRegion);
			CopyRect(it_r->roiTurnAroundLight_red, LightExpRed[m_nRedNum].rLightRect);//RED			
			CopyRect(it_r->roiTurnAroundLight_green, LightExpRed[m_nRedNum].gLightRect);//GREEN
			//YELLOW,不需对黄灯进行增强
			m_nRedNum++;
		}
		*/

		nRoadWayCounts++;
		it_r++;
	}

	if(m_pRedLightAdjust != NULL)
	{
		delete m_pRedLightAdjust;
	}

	if(m_nRedNum > 0)
	{
		int nColorArray[MAX_RED_LIGHT_NUM];
		for(int i=0; i<MAX_RED_LIGHT_NUM; i++)
		{
			nColorArray[i] = -1;
		}
		//其中rLightrect []表示的是当前场景灯头的roi区域，目前设置最大为10个区域；
		//nLightnum表示实际的灯头区域个数；
		//LightInf[]表示对应灯头的灯信号
		m_pRedLightAdjust = new MvRedLightAdjust(LightExpRed, m_nRedNum, nColorArray);
		if(m_pRedLightAdjust)
		{
			bRet = true;
		}
	}
	
	return bRet;
}
#endif

bool CRoadCarnumDetect::CopyRect(const CvRect &rtSrc, CvRect &rtDest)
{
	bool bRet = false;

	if(rtSrc.width > 0 && rtSrc.height > 0)
	{
		if(rtSrc.x >= 0 && rtSrc.y >= 0)
		{
			rtDest.x = rtSrc.x;
			rtDest.y = rtSrc.y;
			rtDest.width = rtSrc.width;
			rtDest.height = rtSrc.height;
			bRet = true;
		}		
	}

	return bRet;
}

#ifdef REDADJUST
void CRoadCarnumDetect::GetRedColor(const UINT16 uTrafficSignal, int nColorArray[])
{
	//nColor灯颜色(0-红灯；1-绿灯；2-黄灯; -1-没有灯亮)
	int nColor = 0;

	if(m_vtsObjectParaMap.size() < 1)
	{
		return;
	}
	//是否视频检测
	bool bSignalType = (uTrafficSignal>>15) & 0x1;

	bool bLeftSignal = false;
	bool bStraightSignal = false;
	bool bRightSignal = false;

	//违章检测参数
	VTSParaMap::iterator it_p = m_vtsObjectParaMap.begin();	

	//int nRoadVerId = it_p->second.nVerRoadIndex;//取第一车道,当前对整个通道,红绿灯区域进行增强
	int iIndexRed = 0;
	int i = 0;
	while(it_p != m_vtsObjectParaMap.end())
	{
		for(int j=0; j<4; j++)
		{
			if(m_RedDirectionArray[j].uRoadWayId == it_p->second.nRoadIndex)
			{
				if(!bSignalType)//视频检测红绿灯
				{
					bLeftSignal = (uTrafficSignal >> 14) & 0x1;//左
					bStraightSignal = (uTrafficSignal >> 13) & 0x1;//中
					bRightSignal = (uTrafficSignal >> 12) & 0x1;//右
				}	
				else
				{
					bLeftSignal = ((uTrafficSignal>>(it_p->second.nLeftControl)) & 0x1);
					bStraightSignal = ((uTrafficSignal>>(it_p->second.nStraightControl)) & 0x1);
					bRightSignal = ((uTrafficSignal>>(it_p->second.nRightControl)) & 0x1);
				}				

				if(i < m_nRedNum)
				{
					if(m_RedDirectionArray[j].bLeft)
					{
						iIndexRed = m_nIndexRedArray[i];					
						nColorArray[iIndexRed] = (int)(!bLeftSignal);
						i++;
					}
					if(m_RedDirectionArray[j].bStraight)
					{
						iIndexRed = m_nIndexRedArray[i];					
						nColorArray[iIndexRed] = (int)(!bStraightSignal);
						i++;
					}
					if(m_RedDirectionArray[j].bRight)
					{
						iIndexRed = m_nIndexRedArray[i];					
						nColorArray[iIndexRed] = (int)(!bRightSignal);
						i++;
					}

#ifdef REDADJUST_2_DEBUG
					LogTrace("RedSignal.log", "id:%d nRoadVerId:%d dir:%d bLeftSignal:%d bStraightSignal:%d bRightSignal:%d i:%d iIndexRed:%d nColor:%d vtsParaSize:%d", \
						m_nChannelId, it_p->second.nVerRoadIndex, it_p->second.nRoadDirection, bLeftSignal, bStraightSignal, bRightSignal,i, iIndexRed, nColor, m_vtsObjectParaMap.size());
#endif					
				}
				else
				{
					break;
				}
			}//end of if(m_RedDirectionArray[j].uRoadWayId == it_p->second.nRoadIndex)
		}//End of for j
		
		it_p++;
	}
}
#endif


//图片上叠加信号值 [RED][GREEN]
void CRoadCarnumDetect::PutRedSignal(IplImage* pImage, int nColorArray[], unsigned long uSignalSeq)
{
	CvxText cvTextRed;
	cvTextRed.Init(40);

	wchar_t wchOut[255] = {'\0'};
	char chOut[255] = {'\0'};

	int nWidth = pImage->width - 400;
	int nHeight = 30;

	//设备编号
	sprintf(chOut,"seq:%lld Red:[%d, %d, %d, %d]",\
		uSignalSeq, nColorArray[0], nColorArray[1], nColorArray[2], nColorArray[3]);
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	nHeight = 30;
	cvTextRed.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

	cvTextRed.UnInit();
}

#ifdef MATCH_LIU_YANG_DEBUG
//无锡测试,违章4合一图片叠字格式
void CRoadCarnumDetect::WuXiVtsText(const RECORD_PLATE &plate, std::string &strText)
{
	UINT32 uVtsCode = 0;
	UINT32 uViolationType = plate.uViolationType;
	std::string strViolationType = "";

	//6合1图片上除了需要叠加违章类型，还需要叠加 违章号码
	//16250 - 闯红灯
	//13010 - 逆行
	//12080 - 不按导向车道行驶
	switch(uViolationType)
	{
	case DETECT_RESULT_EVENT_GO_AGAINST:     //2 车辆逆行
	case DETECT_RESULT_RETROGRADE_MOTION:    //26 逆行
		{
			uVtsCode = 13010;
			strViolationType = "逆向行驶";
			break;
		}	
	case DETECT_RESULT_FORBID_LEFT:                   //23 禁止左拐
	case DETECT_RESULT_FORBID_RIGHT:                    //24 禁止右拐
	case DETECT_RESULT_FORBID_STRAIGHT:                 //25 禁止前行
	case DETECT_RESULT_TAKE_UP_NONMOTORWAY:          //机占非
		{
			uVtsCode = 12080;
			strViolationType = "不按规定车道行驶";
			break;
		}
	case DETECT_RESULT_EVENT_GO_CHANGE:      //8 违章变道
	case DETECT_RESULT_ELE_EVT_BIANDAO:      //29 变道
		{
			uVtsCode = 12080;
			strViolationType = "不按所需行进方向驶入导向车道";
			break;
		}
	case DETECT_RESULT_RED_LIGHT_VIOLATION:   //16 闯红灯
		{
			uVtsCode = 16250;
			strViolationType = "违反信号灯规定";
			break;
		}
	default:
		{
			break;
		}
	}

	std::string strTmp = "";	
	char chOut[255] = {'\0'};

	sprintf(chOut,"违法代码:%d ", uVtsCode);
	strTmp = chOut;
	strText += strTmp;

	sprintf(chOut,"违法行为:%s ",strViolationType.c_str());
	strTmp = chOut;
	strText += strTmp;

	//防伪码
	sprintf(chOut,"防伪码: %08x ",g_RoadImcData.GetRandCode());
	strTmp = chOut;
	strText += strTmp;
}
#endif //MATCH_LIU_YANG_DEBUG

//更新uImgKey标记,时间戳
void CRoadCarnumDetect::UpdateImgKey(UINT64 &uKey, UINT32 &uTime)
{
	uKey = GetGlobalImgKey();
	uTime = GetTimeStamp();

//#ifdef DEBUG_LIUYANG
//	LogNormal("global uKey:%lld ", uKey);
//#endif
}

//通过uImgKey，核查记录状态
bool CRoadCarnumDetect::CheckImgListByKey(const UINT64 &uKey)
{
	bool bRet = false;
	pthread_mutex_lock(&m_imgTagMutex);
	for(int i=0; i<MAX_IMG_TAG_NUM; i++)
	{
//#ifdef DEBUG_LIUYANG
//		if(m_imgTagList[i].bUse)
//		{
//			LogNormal("1uKey:%d i:%d uKey:%d bUse:%d nUseCount:%d", \
//				uKey, i, m_imgTagList[i].uKey, m_imgTagList[i].bUse, m_imgTagList[i].nUseCount);
//		}
//#endif
		if(uKey == m_imgTagList[i].uKey)
		{
			bRet = m_imgTagList[i].bUse;
////#ifdef DEBUG_LIUYANG
//			if(m_imgTagList[i].bUse)
//			{
//				LogNormal("2uKey:%d i:%d uKey:%d bUse:%d nUseCount:%d", \
//					uKey, i, m_imgTagList[i].uKey, m_imgTagList[i].bUse, m_imgTagList[i].nUseCount);
//			}
////#endif
			break;
		}
	}
	pthread_mutex_unlock(&m_imgTagMutex);

	return bRet;
}

//通过uImgKey标记更新，记录状态
bool CRoadCarnumDetect::UpdateImgListByKey(const UINT64 &uKey, const bool &bState)
{
	bool bRet = false;

	UINT32 uTsNow = GetTimeStamp();
	UINT32 uTsDis = uTsNow - m_nLastCheckTime;

	//间隔5分钟，核查一次缓存
	if(uTsDis > 300)
	{
		this->CheckImgList();
		m_nLastCheckTime = GetTimeStamp();
	}

	int nIndex = 0;
	pthread_mutex_lock(&m_imgTagMutex);	
	for(int i=0; i<MAX_IMG_TAG_NUM; i++)
	{
		if(uKey == m_imgTagList[i].uKey)
		{
			nIndex = i;
			m_imgTagList[i].nUseCount++;
			m_imgTagList[i].uLastTime = uTsNow;
			bRet = true;
//#ifdef DEBUG_LIUYANG
//			LogNormal("uKey:%lld, bState:%d nCount:%d CRoadCarnumDetect::UpdateImgListByKey", \
//				uKey, bState, m_imgTagList[i].nUseCount);
//#endif
			break;
		}

		if(m_imgTagList[i].nUseCount > 0)
		{
			if(!bState)
			{
				if(m_imgTagList[i].uLastTime != 0)
				{
					uTsDis = uTsNow - m_imgTagList[i].uLastTime;
					//间隔60s钟，以上使用过的记录删除
					if(uTsDis > 60)
					{
#ifdef DEBUG_LIUYANG
						LogNormal("Del uKey:%lld,bUse:%d nCount:%d CRoadCarnumDetect uTsDis:%d", \
							m_imgTagList[i].uKey, m_imgTagList[i].bUse, m_imgTagList[i].nUseCount, uTsDis);
#endif
						m_imgTagList[i].bUse = false;
						m_imgTagList[i].nUseCount = 0;
						m_imgTagList[i].uLastTime = 0;
					}
				}				
			}
		}
	}
	pthread_mutex_unlock(&m_imgTagMutex);

	return bRet;
}

//TODO
//间隔3分钟，核查一次缓存，删除3分钟以前的数据
void CRoadCarnumDetect::CheckImgList()
{
	pthread_mutex_lock(&m_imgTagMutex);
	UINT32 uTsNow = GetTimeStamp();
	UINT32 uTsDis = 0;
	for(int i=0; i<MAX_IMG_TAG_NUM; i++)
	{
		if(m_imgTagList[i].uTime > 0)
		{
			uTsDis = uTsNow - m_imgTagList[i].uTime;
			if(uTsDis > 180)
			{
				m_imgTagList[i].bUse = false;
				m_imgTagList[i].nUseCount = 0;
				m_imgTagList[i].uLastTime = 0;
#ifdef DEBUG_LIUYANG
				LogNormal("Del2 uKey:%lld, bUse:%d CRoadCarnumDetect::CheckImgList", m_imgTagList[i].uKey, m_imgTagList[i].bUse);
#endif
				break;
			}
		}
	}
	pthread_mutex_unlock(&m_imgTagMutex);
}

//取得最老的记录下标
UINT32 CRoadCarnumDetect::GetOldestImg()
{
	UINT32 nIndex = 0;

	UINT32 uTsNow = GetTimeStamp();
	UINT32 uTsDis = 0;
	UINT32 uTsDisTemp = 0;	
	pthread_mutex_lock(&m_imgTagMutex);	
	for(int i=0; i<MAX_IMG_TAG_NUM; i++)
	{
		if(m_imgTagList[i].bUse)
		{
			uTsDis = uTsNow - m_imgTagList[i].uTime;
			if(uTsDis > uTsDisTemp)
			{
				uTsDisTemp = uTsDis;
				nIndex = i;
			}
		}		
	}
	pthread_mutex_unlock(&m_imgTagMutex);

#ifdef DEBUG_LIUYANG
	LogNormal("GetOldestImg uTsDisTemp:%d nIndex:%d ", uTsDisTemp, nIndex);
#endif

	return nIndex;
}

//删除最老的n记录
UINT32 CRoadCarnumDetect::DelOldestImgs(int n)
{
	for(int i=0; i<n; i++)
	{
		int nIndex = GetOldestImg();

		pthread_mutex_lock(&m_imgTagMutex);	
		m_imgTagList[nIndex].bUse = false;
		m_imgTagList[nIndex].nUseCount = 0;
		m_imgTagList[nIndex].uTime = 0;
		m_imgTagList[nIndex].uLastTime = 0;
		pthread_mutex_unlock(&m_imgTagMutex);

//#ifdef DEBUG_LIUYANG
//		LogNormal("DelOldestImgs nIndex:%d ", nIndex);
//#endif
	}
}



#endif

