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

#include "RoadDetect.h"
#include "Common.h"
#include "CommonHeader.h"
#include <time.h>

#include "ippi.h"
#include "ippcc.h"
#include "ximage.h"
#include "XmlParaUtil.h"
#include "RoadImcData.h"

#include "Mv_CarnumDetect.h"
#ifndef NOEVENT
//#include "mvGetBGLight.h"
//#define TEMPERATURE_TEST
//#define SAVE_PREIMAGE_TXT
//#define CAMERA_CONTROL
//#define DEBUG_DETECT
#ifndef DETECT_EVENT_STOP_LOG
    //#define DETECT_EVENT_STOP_LOG
#endif
//#define TESTLOCALPRESET
#define WATER_MARK



//监控线程
void* ThreadDetectFrame(void* pArg)
{
    //取类指针
    CSkpRoadDetect* pRoadDetect = (CSkpRoadDetect*)pArg;
    if(pRoadDetect == NULL) return pArg;

    pRoadDetect->DealFrame();
    pthread_exit((void *)0);
    return pArg;
}
//构造
CSkpRoadDetect::CSkpRoadDetect()
{
    //线程ID
    m_nThreadId = 0;
    //通道ID初始
    m_nChannelID = -1;
    //检测帧列表互斥
    pthread_mutex_init(&m_Frame_Mutex,NULL);
    pthread_mutex_init(&m_preFrame_Mutex,NULL);
	pthread_mutex_init(&m_JpgFrameMutex,NULL);
	pthread_mutex_init(&m_EventPic_Mutex,NULL);
	pthread_mutex_init(&m_Preset_Mutex,NULL);
	pthread_mutex_init(&m_DetectCarParkRect_Mutex,NULL);
	
    //最多缓冲3帧
    m_nFrameSize = 3;

//	m_bNight = false;

    m_nDetectTime = DETECT_AUTO;
    //默认只检测事件
    m_nDetectKind = DETECT_FLUX;

    m_bEventCapture = false;
    m_eCapType = CAPTURE_NO;

    m_bConnect = false;

    /*m_bSensitive = 0;
    m_bRmShade = 0;
    m_nSameEventIgr  = 0;*/

    m_nTrafficStatTime = 60;
    m_nCaptureTime = 5;

    m_nSaveImageCount = 1;
    m_nSmallPic = 0;
    m_nWordPos = 0;

    m_imgSnap = NULL;
    m_imgPreSnap = NULL;
    m_imgComposeSnap = NULL;
	m_imgComposeStopSnap = NULL;
    m_pResultFrameBuffer = NULL;
    m_pJpgImage = NULL;
    m_img = NULL;
    m_imgPre = NULL;
    m_pExtentRegion = NULL;
	m_imgPreObjectSnap = NULL;
	m_imgAuxSnap = NULL;

    m_nExtent = 60;

    m_nDayNight = 1;


    m_nDeinterlace = 1;

    m_fScaleX  = 1;
    m_fScaleY  = 1;

 	m_nMonitorID = 0;
 	m_nHasLocalPreSet = 0;

	m_nDetectDirection = 0;//检测方向

	m_nBmpSize = 0;//保存单张BMP图片的大小
	m_nPreObjectSeq = 0;
	m_bTestResult = false;
	m_nFileID = -1;

	m_bDetectClientRect = false;
#ifdef CAMERAAUTOCTRL
	m_pOnvifCameraCtrl  = new COnvifCameraCtrl();
	m_pRoadDetectPelco   = new CRoadDetectPelco();
	m_pRoadDetectDHCamera = new CRoadDetectDHCamera();
	m_EventVisVideo = new CEventVisVideo();
#endif
	
#ifdef  HANDCATCHCAREVENT
	m_pHandCatchEvent = new CHandCatchEvent(g_ytControlSetting.nHandCatchTime);
#endif

	m_bDealClientRectRequest = true;
	m_nPresetNum = 1;
	m_nGotoPresetTime = (int)GetTimeT();
	m_mapAutoCameraRecord.clear();
#ifdef CAMERAAUTOCTRL
	m_CameraPtzPresetStatus.clear();
#endif
	m_bCameraAutoCtrl = false;
	m_bCameraMoveFormOp = false;
	m_nVideoNum = 10000;
	m_pMd5Ctx = new MD5_CTX();
}
//析构
CSkpRoadDetect::~CSkpRoadDetect()
{
    //检测帧列表互斥
    pthread_mutex_destroy(&m_Frame_Mutex);
    pthread_mutex_destroy(&m_preFrame_Mutex);
	pthread_mutex_destroy(&m_JpgFrameMutex);
	pthread_mutex_destroy(&m_EventPic_Mutex);
	pthread_mutex_destroy(&m_Preset_Mutex);
	pthread_mutex_destroy(&m_DetectCarParkRect_Mutex);

#ifdef CAMERAAUTOCTRL
	if(m_pOnvifCameraCtrl)
	{
		delete m_pOnvifCameraCtrl;
		m_pOnvifCameraCtrl = NULL;
	}
	if(m_pRoadDetectPelco)
	{
		delete m_pRoadDetectPelco;
	}
	if(m_pRoadDetectDHCamera)
	{
		delete m_pRoadDetectDHCamera;
	}
	if(m_EventVisVideo)
	{
		delete m_EventVisVideo;
		m_EventVisVideo = NULL;
	}
#endif

#ifdef  HANDCATCHCAREVENT
	if(m_pHandCatchEvent)
	{
		delete m_pHandCatchEvent;
		m_pHandCatchEvent = NULL;
	}
#endif

	m_bDealClientRectRequest = false;
	m_pH264Capture = NULL;

	if(m_pMd5Ctx)
	{
		delete m_pMd5Ctx;
		m_pMd5Ctx = NULL;
	}

}

//初始化检测数据，算法配置文件初始化
bool CSkpRoadDetect::Init(int nChannelID,int widthOrg,int heightOrg,int widthDst ,int heightDst)
{
	printf("CSkpRoadDetect::Init,m_iWidth=%d,m_iHeight=%d \n",m_iWidth,m_iHeight);

	//通道ID
	m_nChannelID = nChannelID;
	m_iWidth = widthOrg;
	m_iHeight = heightOrg;
	//大华球机参数 分别为 CCD 焦距 倍率
#ifdef CAMERAAUTOCTRL
	m_bDealClientRectRequest = true;
	CreateDealClientRectMsgThread();
#endif
	
	


	if( (widthOrg/widthDst) == (heightOrg/heightDst) )
	{
		m_nDeinterlace = 1; //帧图像
	}
	else
	{
		m_nDeinterlace = 2; //场图像
	}

    LogTrace(NULL, "bool CSkpRoadDetect::Init\n");
    //终止检测
    m_bTerminateThread = false;
	m_bTestResult = false;
	m_nFileID = -1;
    //完成检测
//	m_bFinishDetect = true;

    //m_nPreSeq = 0;
    //m_preTrafficStatTime = 0;
    m_uPreTimestamp = 0;
	m_nPreObjectSeq = 0;

    //文本初始化
    int nFontSize = 25;

	/*if(widthOrg < 500)
    {
        nFontSize = 15;
    }
    else if(widthOrg < 1000)
    {
        nFontSize = 15;
    }
    else*/
    {
        nFontSize = g_PicFormatInfo.nFontSize;
    }
    m_nSaveImageCount = g_nSaveImageCount;
    m_nSmallPic = g_PicFormatInfo.nSmallPic;//是否是小图
    m_nWordPos = g_PicFormatInfo.nWordPos;//文字显示位置
	//深圳北环图片格式
	if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
	{
		
		m_nExtent = 0;
		m_imgComposeSnap = cvCreateImage(cvSize(widthOrg*2,(heightOrg*2)),8,3);
	}
	else if(1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
	{
		
		m_nExtent = 0;
		m_imgComposeSnap = cvCreateImage(cvSize(widthOrg*3,heightOrg),8,3);
	}
	else
	{	
		//车牌事件保存三张图
		if((m_nDetectKind&DETECT_EVENT_CARNUM))
		{
			nFontSize = 80;
			m_nExtent = 120;
			m_imgComposeSnap = cvCreateImage(cvSize(widthOrg*3,(heightOrg*m_nDeinterlace+m_nExtent)),8,3);
			
		}
		else if(!(m_nDetectKind&DETECT_VIOLATION))
		{
			//默认左右叠加
			if(m_nSmallPic == 1)//存储小图
			{
				if(m_nSaveImageCount == 2)
				{
					nFontSize = 80;
					m_nExtent = 120;
					m_imgComposeSnap = cvCreateImage(cvSize(widthOrg*2+(heightOrg*m_nDeinterlace),(heightOrg*m_nDeinterlace+m_nExtent)),8,3);
					
				
				}
				else
				{
					
					m_nExtent = g_PicFormatInfo.nExtentHeight;
					m_imgComposeSnap = cvCreateImage(cvSize(widthOrg*2,(heightOrg*m_nDeinterlace+m_nExtent)),8,3);
				}
			}
			else
			{
				m_nExtent = g_PicFormatInfo.nExtentHeight;
				m_imgComposeSnap = cvCreateImage(cvSize(widthOrg*2,(heightOrg*m_nDeinterlace+m_nExtent)),8,3);
			}
		}
	}
#ifdef CAMERAAUTOCTRL
	m_nExtent = g_PicFormatInfo.nExtentHeight;
	nFontSize = g_PicFormatInfo.nFontSize;
#endif
	
	if (g_ytControlSetting.nPicComPoseMode == 0)// 3远1近
	{
		m_nRemotePicCount = 3;
	}
	else if (g_ytControlSetting.nPicComPoseMode == 1)// 2远2近
	{
		m_nRemotePicCount = 2;
	}
	else if (g_ytControlSetting.nPicComPoseMode == 2)// 1远2近
	{
		m_nRemotePicCount = 1;
	}
	else if (g_ytControlSetting.nPicComPoseMode == 3)// 1近3远
	{
		m_nRemotePicCount = 3;
	}

	if (g_ytControlSetting.nPicComPoseMode == 2)// 1远2近
	{
		printf("g_ytControlSetting.nPicComPoseMode == 2\n");
		m_imgComposeStopSnap = cvCreateImage(cvSize(widthOrg*3,heightOrg),8,3);
	}
	else 
	{	
		if(m_nWordPos == 0)//图片下方
		{
			m_imgComposeStopSnap = cvCreateImage(cvSize(widthOrg*2,(heightOrg*m_nDeinterlace+m_nExtent)*2),8,3);
		}
		else
		{
			m_imgComposeStopSnap = cvCreateImage(cvSize(widthOrg*2,(heightOrg*m_nDeinterlace)*2),8,3);
		}
		
	}

 
	if(!(m_nDetectKind&DETECT_VIOLATION))
	{	
		if (g_ytControlSetting.nPicComPoseMode == 2)// 1远2近
		{
			printf("g_ytControlSetting.nPicComPoseMode == 2\n");
			m_imgSnap = cvCreateImage(cvSize(widthOrg,heightOrg),8,3);
		}
		else
		{
			m_imgSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);
		}
		

		m_imgPreSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);
		//车牌事件时的第三张图
		m_imgAuxSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);
	}
	//m_imgPreObjectSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);

	if (g_ytControlSetting.nPicComPoseMode == 2)// 1远2近
	{
		printf("g_ytControlSetting.nPicComPoseMode == 2\n");
		m_img =  cvCreateImageHeader(cvSize(widthOrg, heightOrg), 8, 3);
	}
	else
	{
		m_img =  cvCreateImageHeader(cvSize(widthOrg, heightOrg+m_nExtent/m_nDeinterlace), 8, 3);
	}

    m_imgPre =  cvCreateImageHeader(cvSize(widthOrg, heightOrg+m_nExtent/m_nDeinterlace), 8, 3);

    int nSize = m_img->widthStep*m_nExtent/m_nDeinterlace*sizeof(char);

	if(m_nExtent > 0)
	{
		m_pExtentRegion  = new char[nSize];
		memset(m_pExtentRegion,0,nSize);
	}

	m_pJpgImage = new BYTE[widthOrg*heightOrg];

    //分配秒图缓冲区
	{
		m_nBmpSize = nSize = sizeof(SRIP_DETECT_HEADER)+m_img->imageSize;
		LogTrace(NULL, "=================================nSize=%d\n",nSize);
		int nPlateSize = FRAMESIZE;
		
		if(!(m_nDetectKind&DETECT_VIOLATION))
		{
			m_pResultFrameBuffer = new BYTE[nSize*nPlateSize];
			for(int i=0; i < nPlateSize; i++)
			{
				m_chResultFrameBuffer[i] = m_pResultFrameBuffer + i*nSize;
			}
		}
		m_nResultReadIndex = -1;
		m_nResultWriteIndex = -1;

		//获取xy方向缩放比
		m_fScaleX =  (double)widthOrg/widthDst;
		m_fScaleY = (double) (heightOrg*m_nDeinterlace)/heightDst;

		m_ratio_x = 1.0;
		m_ratio_y = 1.0;
	}


    //流量统计初始化
    for(int i = 0; i<MAX_ROADWAY; i++)
    {
        uFluxAll[i]=0;
        uFluxPerson[i]=0;
        uFluxNoneVehicle[i]=0;
        uFluxVehicle[i]=0;
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

    m_bReloadConfig = true;
	m_bCameraMoveFormOp = false;
	
    m_cvText.Init(nFontSize);
	//清空BMP目录
	//RemoveDir("./bmp");

    //启动检测帧数据处理线程
    if(!BeginDetectThread())
    {
        LogError("创建采集帧数据检测线程失败!\r\n");
        return false;
    }
#ifdef _DEBUG
    LogNormal("通道检测算法模块初始化完成，处理线程启动!\r\n");
#endif

    if(m_bEventCapture)
    {
        m_skpRoadRecorder.Init();
    }

    m_nHasLocalPreSet = g_ytControlSetting.nHasLocalPreSet;

    if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
    {
        if(m_nHasLocalPreSet == 0)
        {
            ;
        }
    }

	m_JpgFrameMap.clear();
	m_EventPicMap.clear();

    return true;
}

//释放检测数据，算法数据释放
bool CSkpRoadDetect::UnInit()
{
	//设置停止标志位
	m_bTerminateThread = true;
	m_bDealClientRectRequest = false;
	usleep(200*1000);


#ifdef CAMERAAUTOCTRL
	m_mapAutoCameraRecord.clear();

	pthread_mutex_lock(&m_Preset_Mutex);
	for(mapAutoCameraPtzTrack::iterator iter=m_CameraPtzPresetStatus.begin();iter!=m_CameraPtzPresetStatus.end();iter++)
	{
		delete iter->second;
	}
	m_CameraPtzPresetStatus.clear();
	pthread_mutex_unlock(&m_Preset_Mutex);

#endif

    //停止检测帧数据处理线程
    EndDetectThread();

    //停止事件录象线程
    if(m_bEventCapture)
    m_skpRoadRecorder.UnInit();

    //加锁
    pthread_mutex_lock(&m_Frame_Mutex);
    //清空检测列表
    m_ChannelFrameList.clear();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);

    //释放文本资源
    m_cvText.UnInit();

	if(g_ytControlSetting.nCameraAutoMode == 1 || (g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1))
	{
		mvcarnumdetect.carnum_quit();
	}

    m_sParamIn.clear();

    //释放检测算法模块数据
    if(m_list_DetectOut.size()>0)
        m_list_DetectOut.clear();

    if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
    {
        if(m_nHasLocalPreSet == 1)
        {
            m_PreSetControl.UnInit();
        }
        else
        {
           ;
        }
    }

    //设置过释放检测算法模块数据
#ifndef NOEVENT
    if(m_Rdetect.mvwidthIsSet())
    {
        printf("m_Rdetect.mvwidthIsSet()\r\n");
        m_Rdetect.mvuninit();
    }
#endif

#ifdef DETECT_VEHICLE
	m_MvDetector.mv_UnInit();
#endif

    if(m_imgSnap != NULL)
    {
        cvReleaseImage(&m_imgSnap);
        m_imgSnap = NULL;
    }

    if(m_imgComposeSnap != NULL)
    {
        cvReleaseImage(&m_imgComposeSnap);
        m_imgComposeSnap = NULL;
    }

	if(m_imgComposeStopSnap != NULL)
    {
        cvReleaseImage(&m_imgComposeStopSnap);
        m_imgComposeStopSnap = NULL;
    }

	if(m_imgAuxSnap != NULL)
	{
		cvReleaseImage(&m_imgAuxSnap);
		m_imgAuxSnap = NULL;
	}

    if(m_imgPreSnap != NULL)
    {
        cvReleaseImage(&m_imgPreSnap);
        m_imgPreSnap = NULL;
    }

	/*if(m_imgPreObjectSnap != NULL)
	{
		cvReleaseImage(&m_imgPreObjectSnap);
		m_imgPreObjectSnap = NULL;
	}*/

    if(m_img != NULL)
    {
        cvReleaseImageHeader(&m_img);
        m_img = NULL;
    }

    if(m_imgPre != NULL)
    {
        cvReleaseImageHeader(&m_imgPre);
        m_imgPre = NULL;
    }

    if(m_pExtentRegion != NULL)
    {
        delete []m_pExtentRegion;
        m_pExtentRegion = NULL;
    }

    if(m_pJpgImage != NULL)
    {
        delete []m_pJpgImage;
        m_pJpgImage = NULL;
    }

    //加锁
    pthread_mutex_lock(&m_preFrame_Mutex);
    if(m_pResultFrameBuffer != NULL)
    {
        delete []m_pResultFrameBuffer;
        m_pResultFrameBuffer = NULL;
    }
    //解锁
    pthread_mutex_unlock(&m_preFrame_Mutex);

    return true;
}


//启动数据处理线程
bool CSkpRoadDetect::BeginDetectThread()
{
    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    struct   sched_param   param;
    param.sched_priority   =   20;
    pthread_attr_setschedparam(&attr,   &param);
    //分离线程
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    //启动监控线程
    int nret=pthread_create(&m_nThreadId,&attr,ThreadDetectFrame,this);


    //成功
    if(nret!=0)
    {
        //失败
        LogError("创建采集帧检测处理线程失败,服务无法检测采集帧！\r\n");
        pthread_attr_destroy(&attr);
        return false;
    }
    pthread_attr_destroy(&attr);
    return true;
}

//停止处理线程
void CSkpRoadDetect::EndDetectThread()
{
    //停止线程
    if(m_nThreadId != 0)
    {
        pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
    }
}

//事件检测处理
void CSkpRoadDetect::DealFrame()
{
	#ifdef TEMPERATURE_TEST
    UINT32 uTime1 = GetTimeStamp();//开始检测的时间
    UINT32 uTime2 = 0;
    UINT32 uPreTime = uTime1;
    int ndetectframe = 0;
	int nPreDetectFrame = 0;
	#endif
    int count = 0;
	bool bDetect = true;
    while(!m_bTerminateThread)
    {
        //自动判断白天还是晚上
         if(m_nDetectTime == DETECT_AUTO)
         {
              if(count>200)
              {
                    count = 0;
              }

              if(count==0)
               {
                    m_nDayNight = DayOrNight();
                    #ifndef NOEVENT
                    m_Rdetect.mvconfige_isDayParam(m_nDayNight);
                    #endif

					//bDetect = CheckDetectTime();
               }
               count++;
         }
            //////////////////////////////////////////
		 //change by Gaoxiang
        //if(bDetect)
        {
			#ifdef TEMPERATURE_TEST
            ndetectframe++;
			#endif
            //处理一条数据
            DetectFrame();
        }
        //1毫秒
        usleep(1000*1);

		#ifdef TEMPERATURE_TEST
        uTime2 = GetTimeStamp();//每分钟统计一次
        if((uTime2%60==0)&&(uPreTime != uTime2))
        {
            std::string strTime = GetTime(uTime2);
            FILE * fp = fopen("avgrateDetect.txt","a");
            double avgrate = (ndetectframe-nPreDetectFrame*1.0)/(uTime2-uPreTime);
            fprintf(fp,"time = %s,avgrate=%f,detectframe=%d,uTime2-uTime1=%ds\n",strTime.c_str(),avgrate,ndetectframe-nPreDetectFrame,uTime2-uPreTime);//平均帧率
            fclose(fp);
            uPreTime = uTime2;
			nPreDetectFrame = ndetectframe;
        }
		#endif
    }
}

//检测数据
bool CSkpRoadDetect::DetectFrame()
{
// 	if(g_ytControlSetting.nCameraAutoMode == 1)
// 	{
// 		if(m_bDetectClientRect)
// 		{
// 			return true;
// 		}
// 		if(m_bCameraMoveFormOp)
// 		{
// 			return true;
// 		}
// 	}
    //弹出一条帧图片
    std::string result;

    if(!PopFrame(result))
	{
		return true;
	}
    //无检测帧
    if(result.size() == 0) return true;

    //检测图片，并提交检测结果
    return OnFrame(result);

}




//添加一帧数据
bool CSkpRoadDetect::AddFrame(std::string& frame)
{
    //添加到检测列表
    if(!m_bTerminateThread)
    {

        SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)frame.c_str();

        //加锁
        pthread_mutex_lock(&m_Frame_Mutex);
        if(m_ChannelFrameList.size()>=m_nFrameSize)
        {
            m_ChannelFrameList.pop_front();
            //LogError(NULL, "=======================exceed CSkpRoadDetect::AddFrame\r\n");
        }

    #ifdef DEBUG_INFO
        printf("                 AddDetect %d\r\n",sDetectHeader->uChannelID);
        //		printf("                      %d\r\n",sDetectHeader->uSeq);
    #endif

        m_ChannelFrameList.push_back(frame);
        //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);

		
        //保存秒图
    #ifdef DEBUG_INFO
        printf("====frame.size()=%d\n",frame.size());
    #endif

       //  double t = (double)cvGetTickCount();

		{
			 //保存秒图
			 if(sDetectHeader->uTime64 >= m_uPreTimestamp+300000)
			 {
				 if(m_pResultFrameBuffer != NULL)
				 {
					AddResultFrameList((BYTE*)frame.c_str(),frame.size());
					m_uPreTimestamp = sDetectHeader->uTime64;
				 }
			 }
		}


        return true;
    }
    return false;
}


//弹出一帧数据
bool CSkpRoadDetect::PopFrame(std::string& response )
{

    // ID错误或者实例为空则返回
    if(this == NULL || m_nChannelID <= 0 )
	{
		return false;
	}
        

    //加锁
    pthread_mutex_lock(&m_Frame_Mutex);

    //判断是否有采集的数据帧
    if(m_ChannelFrameList.size() == 0)
    {
        //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);
        return false;
    }

    //取最早命令
    ChannelFrame::iterator it = m_ChannelFrameList.begin();
    //保存数据
    response = *it;
    //删除取出的采集帧
    m_ChannelFrameList.pop_front();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);
    return true;

}

//添加jpg图像帧
bool  CSkpRoadDetect::AddJpgFrame(BYTE* pBuffer)
{
	if(!m_bTerminateThread)
	{
		//加锁
		pthread_mutex_lock(&m_JpgFrameMutex);

		//暂存seq->index对应关系
		if(m_JpgFrameMap.size()>20)
		{
			map<int64_t,string>::iterator it_map = m_JpgFrameMap.begin();
			m_JpgFrameMap.erase(it_map);
		}

		yuv_video_buf* pHeader = (yuv_video_buf*)pBuffer;
		string strPic;
		strPic.append((char*)pBuffer,sizeof(yuv_video_buf));
		//strPic.append((char*)(pBuffer+sizeof(yuv_video_buf)),pHeader->size);
		strPic.append((char*)(pBuffer+sizeof(yuv_video_buf)+sizeof(Image_header_dsp)),pHeader->size-sizeof(Image_header_dsp));
		m_JpgFrameMap.insert(make_pair(pHeader->nSeq,strPic));

		//解锁
		pthread_mutex_unlock(&m_JpgFrameMutex);

		return true;
	}
	return false;
}


//添加秒图
void CSkpRoadDetect::AddResultFrameList(BYTE* frame,int nSize)
{
		//加锁
		pthread_mutex_lock(&m_preFrame_Mutex);

		int nPlateSize = FRAMESIZE;
		m_nResultWriteIndex++;
		m_nResultWriteIndex %= nPlateSize;

       if(m_nWordPos == 0)//字在图下方
       {
           memcpy(m_chResultFrameBuffer[m_nResultWriteIndex],frame,nSize);
       }
       else
       {
           memcpy(m_chResultFrameBuffer[m_nResultWriteIndex],frame,sizeof(SRIP_DETECT_HEADER));
           memcpy(m_chResultFrameBuffer[m_nResultWriteIndex]+sizeof(SRIP_DETECT_HEADER)+m_img->widthStep*(m_nExtent/m_nDeinterlace),frame+sizeof(SRIP_DETECT_HEADER),nSize-sizeof(SRIP_DETECT_HEADER));
       }


    //printf("======after====m_nResultWriteIndex==%d=====================m_img->imageSize=%d,sizeof(SRIP_DETECT_HEADER)=%d\n",m_nResultWriteIndex,m_img->imageSize,sizeof(SRIP_DETECT_HEADER));
    //解锁
    pthread_mutex_unlock(&m_preFrame_Mutex);

}



//保存全景图像
int CSkpRoadDetect::SaveImage(IplImage* pImg,std::string strPicPath,int nIndex,int nRandCode1,int nRandCode2)
{
    CxImage image;
    int srcstep = 0;
    if(image.IppEncode((BYTE*)pImg->imageData,pImg->width,pImg->height,3,&srcstep,m_pJpgImage,g_PicFormatInfo.nJpgQuality))
    {
        FILE* fp = NULL;
        if(nIndex > 0)
        {
            fp = fopen(strPicPath.c_str(),"a");//追加在上一张图的后面
        }
        else
        {
            fp = fopen(strPicPath.c_str(),"wb");
        }
		if(fp)
		{
			fwrite(m_pJpgImage,srcstep,1,fp);

			if(g_nServerType == 7)
			{
				//在jpg的后面记录防伪码
				fwrite(&nRandCode1,4,1,fp);
				fwrite(&nRandCode2,4,1,fp);
			}

			//叠加数字水印
			#ifdef WATER_MARK
			std::string strWaterMark;
			GetWaterMark((char*)m_pJpgImage,srcstep,strWaterMark);		
			fwrite(strWaterMark.c_str(),strWaterMark.size(),1,fp);
			srcstep += strWaterMark.size();
			#endif

			fclose(fp);
		}
    }

    return srcstep;
}

bool CSkpRoadDetect::OnHandFrame(std::string& frame)
{
#ifdef  HANDCATCHCAREVENT
	if(m_pHandCatchEvent)
	{

		pthread_mutex_lock(&m_DetectCarParkRect_Mutex);
		if(m_mapDetectCarParkRect.size() > 0)
		{
			for(mapDetectCarParkRect::iterator iter=m_mapDetectCarParkRect.begin();iter!=m_mapDetectCarParkRect.end();iter++)
			{
				CvRect rectDraw;
				rectDraw.x = iter->second.nX;
				rectDraw.y = iter->second.nY;
				rectDraw.width = iter->second.nWidth;
				rectDraw.height = iter->second.nHeight;
				int nCarId = m_pHandCatchEvent->SetObjRect(frame,rectDraw);
				if(nCarId > 0)
				{
					if (g_ytControlSetting.nPicComPoseMode == 2)
					{
						DetectHandEventTJ(frame,nCarId,rectDraw);
					}
					else
					{
						DetectHandEvent(frame,nCarId,rectDraw,1);
					}

				}
				m_mapDetectCarParkRect.erase(iter->first);
				pthread_mutex_unlock(&m_DetectCarParkRect_Mutex);
				return true;
			}
		}
		pthread_mutex_unlock(&m_DetectCarParkRect_Mutex);
		m_pHandCatchEvent->DealFrame(frame);
	}
#endif
	return true;
}
//检测采集帧，并返回结果
bool CSkpRoadDetect::OnFrame(std::string& frame)
{
	//printf("%s:%d\n",__FUNCTION__,__LINE__);
    // 开始检测
//	m_bFinishDetect = false;

	//读取测试结果
	if(g_nFileID > 0)
	{
		//读取文件获取结果
		if(m_nFileID != g_nFileID)
		{
			m_nFileID = g_nFileID;
			m_listTestResult.clear();

			char filename[256] = {0};
			sprintf(filename,"E%d.data",g_nFileID);

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

    // 重新加载配置
    if(m_bReloadConfig)
    {
#ifdef CAMERAAUTOCTRL
		InitAutoCtrlCfg();
#endif
        //设置过释放检测算法模块数据
#ifndef NOEVENT
        if(m_Rdetect.mvwidthIsSet())
            m_Rdetect.mvuninit();
#endif

        //初始化车道配置
        if(!InitDetectConfig())
        {
            // 设置出错返回
//			m_bFinishDetect = true;
            return false;
        }

#ifndef NOEVENT
        //设置是否检测车牌
        if((m_nDetectKind&DETECT_EVENT_CARNUM)==DETECT_EVENT_CARNUM)
            m_Rdetect.SetDetectCarnum(true);
        else
            m_Rdetect.SetDetectCarnum(false);

        //设置是否检测颜色
        if((m_nDetectKind&DETECT_CARCOLOR)==DETECT_CARCOLOR)
            m_Rdetect.SetDetectColor(true);
        else
            m_Rdetect.SetDetectColor(false);

		m_passerbyColor.SetCameraType(m_nCameraType);
		m_Rdetect.m_pColorRecognisize = &m_passerbyColor;

#endif
        //设置车道检测参数
        m_sParamIn.clear();
        SRIP_CHANNEL_EXT sChannel;
        sChannel.uId = m_nChannelID;

        CXmlParaUtil xml;
        xml.LoadRoadParameter(m_sParamIn,sChannel);

        if(m_sParamIn.size()<=0)
        {
            // 设置出错返回
//			m_bFinishDetect = true;
            return false;
        }
		
		//载入通道图片格式
		xml.LoadPicFormatInfo(m_nChannelID,m_picFormatInfo);

		#ifdef DETECT_VEHICLE
			m_MvDetector.mv_Init("./SvmTrain/testT5233163854.xml");
		#endif

		if((m_nDetectKind&DETECT_VTS)==DETECT_VTS)
		{
			VTS_GLOBAL_PARAMETER vtsGlobalPara;
			CXmlParaUtil xmlvts;
			xmlvts.LoadVTSParameter(m_vtsObjectParaMap,m_nChannelID,vtsGlobalPara);
		}

        //全局参数设置
        ROAD_PARAM param_road;
        param_road.nDetectTime = m_nDayNight;
        param_road.nShadow = sChannel.bRmShade;
        param_road.nSensitive = sChannel.bSensitive;
        param_road.nSameEventIgr = sChannel.uEventDetectDelay;
        param_road.nShowTime = sChannel.nShowTime;
        m_nTrafficStatTime = sChannel.uTrafficStatTime;

        //设置两条命令之间最短需要间隔时间
        //g_VisKeyBoardControl.SetSpaceTimes(sChannel.nZoomScale);
		g_VisKeyBoardControl.SetSpaceTimes(m_nCameraId, 500); //设定为500ms

#ifndef NOEVENT
        m_Rdetect.mvconfig_param_to_detect(m_sParamIn,param_road);
			
		///////////////////////////////////////nhs
		if (g_ytControlSetting.nPicComPoseMode  == 0 )
		{
			m_Rdetect.SetRemotePicCount( m_nRemotePicCount);
		}
		else if (g_ytControlSetting.nPicComPoseMode  == 1 )                                  
		{
			m_Rdetect.SetRemoteNearCombPicCount( m_nRemotePicCount);
		}
		else if(g_ytControlSetting.nPicComPoseMode  == 2 )             
		{
			m_Rdetect.SetRemotePicCount( m_nRemotePicCount);
		}
		else if (g_ytControlSetting.nPicComPoseMode  == 3 )
		{
			m_Rdetect.SetRemotePicCount( m_nRemotePicCount);
		}
		
		if(g_ytControlSetting.nCameraAutoMode == 1 || (g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1))
		{
			std::map<int,list<unsigned char*> >::iterator iterMap = m_mapRemotePicList.begin(); 
			for(;iterMap != m_mapRemotePicList.end();iterMap++)
			{
				while(iterMap->second.size()>0)
				{
					unsigned char* pMsg =iterMap->second.front();
					iterMap->second.pop_front();
					if(pMsg != NULL)
					{
						free(pMsg);
					}
				}
			}
			m_mapRemotePicList.clear();

			mvcarnumdetect.carnum_quit();

			char buf[64] = {'\0'};
			sprintf(buf,"./BocomMv/%d",1);
			mvcarnumdetect.carnum_init(buf,homography_image_to_world,m_imgSnap->width,m_imgSnap->height);
#ifdef CAMERAAUTOCTRL
			mvcarnumdetect.set_vedio(0);//0为图像处理，1为视频处理
			mvcarnumdetect.mvSetNonplateDetect(false);
			mvcarnumdetect.mvSetDoMotorCarCarnum(true);

#endif
		}
#endif

#ifndef NOEVENT
        if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
        {
            if(m_nHasLocalPreSet == 0)
            {
                //m_RoadDetect_camCtrl.SetParaMeter(sChannel);
            }
            else
            {
               m_PreSetControl.Init(m_nChannelID);
            }
        }
#endif

		RoadIndexMap::iterator it_b = m_roadMap.begin();
		if(it_b != m_roadMap.end())
		{
           m_nDetectDirection = it_b->second.nDirection;
		}

        //重新配置完成
        m_bReloadConfig = false;
        LogNormal("通道[%d]重新加载配置\n",m_nChannelID);
#ifdef  HANDCATCHCAREVENT
		m_pHandCatchEvent->SetCatchCarEvent(CatchCarEventCB,(int)((int*)this));
		m_pHandCatchEvent->SetCatchCarEventDraw(CatchCarEventDrawCB,(int)((int*)this));
#endif
    }

#ifdef HANDCATCHCAREVENT
	OnHandFrame(frame);
#endif

    //发送数据
    //SRIP_HEADER sHeader;
    //取类型
    SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

    //////////////////////////////////////
    m_sImageFrame.width = sDetectHeader.uWidth;
    m_sImageFrame.height = sDetectHeader.uHeight;
    m_sImageFrame.ts = sDetectHeader.uTime64;
    m_sImageFrame.data = (char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER));
	//printf("sDetectHeader.uTimestamp=%d,Seq=%d\n",sDetectHeader.uTimestamp,sDetectHeader.uSeq);


    //肯定是需要检测的采集帧
    //释放前一帧的检测结果
    m_list_DetectOut.clear();

	//输出测试结果
	if(m_bTestResult)
	{
		OutPutTestResult(frame);
		return true;
	}

//   if(!m_bNight)
    {
#ifndef NOEVENT
        //检测，参数[帧数据，帧长度，检测入参，检测出参]
        //设置参数
        if (!m_Rdetect.mvwidthIsSet())
        {
            if(!m_Rdetect.mvsetWidth(m_sImageFrame.width,m_sImageFrame.height))
            {
                // 设置出错返回
//				m_bFinishDetect = true;
                return false;
            }
            else
            {
                //第一帧只用于setwidth
//				m_bFinishDetect = true;
                return true;
            }
        }

        //设置时间戳
        if (!m_Rdetect.mvtsIsSet())
        {
            if(!m_Rdetect.mvsetTs(m_sImageFrame.ts))
            {
                // 设置出错返回
//				m_bFinishDetect = true;
                return false;
            }
        }
		//使用自动定位功能
		if(g_ytControlSetting.nCameraAutoMode == 1)
		{
#ifdef HANDCATCHCAREVENT
			m_EventVisVideo->DealEventVideo();
#endif

			m_Rdetect.mvSetIsNeedCtrlCam(false);
			vector<MvCarNumInfo> carNums;
			
            //m_Rdetect.mvVehicleStopDetectEveryFrame(m_list_DetectOut,m_sImageFrame.data, m_sImageFrame.ts);
			
			if(m_Rdetect.mvDetectEveryFrame(m_sImageFrame.data,m_sImageFrame.ts, sDetectHeader.uSeq , carNums))
			{
				bool bNeedCamCtrl = false;
				m_Rdetect.mvget_result(m_list_DetectOut,bNeedCamCtrl);
			}
		}
		else
		{
			//需要根据键盘码来决定是否需要进行云台控制
			if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1 && m_nHasLocalPreSet == 1)
			{
				{
					if(!m_PreSetControl.GetObjMovingState())
					{
						#ifdef TESTLOCALPRESET
						static int count = 1;

						printf("count=%d\n",count);

						if(count > 100)
						{
							CvRect rtPos;
							rtPos.x = 100 * m_fScaleX;
							rtPos.width = 20 * m_fScaleX;
							rtPos.y = 200 * m_fScaleY;
							rtPos.height = 10 * m_fScaleY;

							 printf("rtPos.x=%d,rtPos.y=%d,rtPos.width=%d,rtPos.height=%d\n",rtPos.x,rtPos.y,rtPos.width,rtPos.height);

							//调用近景预置位
							m_PreSetControl.GoToLocalPreSet(rtPos,sDetectHeader.uTimestamp);
							count = 1;
						}
						count++;
						#else
						m_Rdetect.mvSetIsNeedCtrlCam(false);
						vector<MvCarNumInfo> carNums;
						// m_Rdetect.mvVehicleStopDetectEveryFrame(m_list_DetectOut, m_sImageFrame.data, m_sImageFrame.ts);
						if(m_Rdetect.mvDetectEveryFrame(m_sImageFrame.data,m_sImageFrame.ts, sDetectHeader.uSeq , carNums))
						{
							bool bNeedCamCtrl = false;
							//得到结果
							m_Rdetect.mvget_result(m_list_DetectOut,bNeedCamCtrl);
						}

						#endif
					}
					else
					{
						//回归远景预置位
						m_PreSetControl.GoToRemotePreSet(sDetectHeader.uTimestamp);
					}
				}
			}
			else
			{
				{
					m_Rdetect.mvSetIsNeedCtrlCam(false);
					#ifdef LogTime
					struct timeval tv;
					double t = (double)cvGetTickCount();
					gettimeofday(&tv,NULL);
					LogTrace("time-test.log","before mvdetect_every_frame==uSeq=%lld,time = %s.%03d\n",sDetectHeader.uSeq,GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
					#endif

					//检测一帧
					vector<MvCarNumInfo> carNums;

					if((m_nDetectKind&DETECT_EVENT_CARNUM))
					{
						this->PopCarNumInfo(carNums);//get datas from buff
					}
					 // LogTrace("EventDetect.log", "carNums size is:%d", carNums.size());
				   /*  IplImage* img = cvCreateImageHeader(cvSize(m_sImageFrame.width,m_sImageFrame.height),8,3);
					char buf[256] = {0};
					sprintf(buf,"%d.jpg",sDetectHeader.uSeq);
					cvSetData(img,m_sImageFrame.data,img->widthStep);
					cvSaveImage(buf,img);
					cvReleaseImageHeader(&img);*/

					 //m_Rdetect.mvVehicleStopDetectEveryFrame(m_list_DetectOut, m_sImageFrame.data, m_sImageFrame.ts);

					if(m_Rdetect.mvDetectEveryFrame(m_sImageFrame.data,m_sImageFrame.ts, sDetectHeader.uSeq , carNums))
					{
						bool bNeedCamCtrl = false;
						//得到结果
						m_Rdetect.mvget_result(m_list_DetectOut,bNeedCamCtrl);
						//m_Rdetect.mvget_result(m_list_DetectOut);
					}
				
					#ifdef LogTime
					t = (double)cvGetTickCount() - t;
					double dt = t/((double)cvGetTickFrequency()*1000.) ;
					gettimeofday(&tv,NULL);
					LogTrace("time-test.log","after mvdetect_every_frame==uSeq=%lld,time = %s.%03d,dt=%d ms\n",sDetectHeader.uSeq,GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,(int)dt);
					#endif
				}
			}
		}

  
#endif //End of #ifndef NOEVENT
    }

	bool bDetect = CheckDetectTime();

	if(bDetect)
	{
		//检测完成，处理结果
		if(m_list_DetectOut.size()>0)
		{
			if(g_ytControlSetting.nCameraAutoMode == 1)
			{
				OutPutAutoCtrlResult(frame);
			}
			else
			{
				OutPutResult(frame);
			}
			
		}
	}

	if(g_nServerType == 7)
	{
		if((m_nDetectKind&DETECT_VTS)==DETECT_VTS)
		{
			VtsStatistic(&sDetectHeader);
		}
	}

    return true;
}

//输出事件检测结果
void CSkpRoadDetect::OutPutResult(std::string& frame)
{
    //检测完成，处理结果
    if(m_list_DetectOut.size()>0)
    {
        SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

        //事件统计列表
        DetectResultList::iterator it_b = m_list_DetectOut.begin();
        DetectResultList::iterator it_e = m_list_DetectOut.end();

        bool flag = false;//判断是否真实事件
        bool bStat = false;//判断是否统计

        //bool bSave = false;//是否已经保存图片

        RECORD_STATISTIC statistic;//事件和统计分开发送
        RECORD_EVENT event;
        std::string strPicPath,strPath,strTmpPath,strEvent,strStat;
		std::string strEventPlate;
        //
        sDetectHeader.uDetectType = SRIP_DETECT_EVENT;
        //	strEvent.append((char*)sDetectHeader,sizeof(SRIP_DETECT_HEADER));

        UINT32 uRoadIndex = 0;

		{
			if(m_nExtent > 0)
			{
				//需要append文本区域
				if(m_nWordPos == 0)//文字区域叠加在图片下方
				frame.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));
				else if(m_nWordPos == 1)
				frame.insert(sizeof(SRIP_DETECT_HEADER),m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));
			}

			char* data = (char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER));
			cvSetData(m_img,data,m_img->widthStep);
		}

        //事件地点
        memcpy(event.chPlace,m_strLocation.c_str(),m_strLocation.size());

        DETECT_RESULT_TYPE eType;

        while(it_b!=it_e)
        {
            //根据车道号获取车道逻辑编号
            RoadIndexMap::iterator it_p = m_roadMap.find(it_b->nChannelIndex);
            if(it_p != m_roadMap.end())
            {
                uRoadIndex = it_p->second.nVerRoadIndex;
            }
			else
			{
				uRoadIndex = it_b->nChannelIndex;
			}

            //事件
            if(it_b->eRtype < DETECT_RESULT_STAT_FLUX)
            {

				if( ((m_nDetectKind&DETECT_VIOLATION)) )//不做违章检测才报
				{
					it_b++;
					continue;
				}
                //行驶方向
                int nDirection = 0;
                if(it_b->direction < 180)
                {
                    nDirection = 0;
                }
                else
                {
                    nDirection = 1;
                }

				if(m_nDetectDirection != nDirection)
				{
					if(m_nDirection%2==0)
					{
						event.uDirection = m_nDirection-1;
					}
					else
					{
						event.uDirection = m_nDirection+1;
					}
				}
				else
				{
					event.uDirection = m_nDirection;
				}


                event.uRoadWayID = uRoadIndex;//车道编号

                //存数据库的坐标需要乘缩放比
				{
					event.uPosX = (int)it_b->x*m_fScaleX;     //事件发生横坐标
					event.uPosY = (int)it_b->y*m_fScaleY;     //事件发生纵坐标
				}

                if(m_nWordPos == 1)
                event.uPosY += m_nExtent;
                //printf("=================uRoadIndex=%d=====m_fScaleX=%f,m_fScaleY=%f\n",uRoadIndex,m_fScaleX,m_fScaleY);

                event.uColor1 = it_b->color.nColor1;
                event.uColor2 = it_b->color.nColor2;
                event.uColor3 = it_b->color.nColor3;
                event.uWeight1 = it_b->color.nWeight1;
                event.uWeight2 = it_b->color.nWeight2;
                event.uWeight3 = it_b->color.nWeight3;

                //printf("event.uColor1=%d,event.uColor2=%d,event.uColor3=%d,event.uWeight1=%d,event.uWeight2=%d,event.uWeight3=%d,event.uPosX=%d,event.uPosY=%d\n",event.uColor1,event.uColor2,event.uColor3,event.uWeight1,event.uWeight2,event.uWeight3,event.uPosX,event.uPosY);

                event.uType = it_b->type;//判断人还是车
                event.uSpeed = it_b->value;
				//LogTrace("eventType.log", "EventType=%u, seq=%u, Time:%s", event.uType, event.uSeq, GetTimeCurrent().c_str());
                //类型
                eType = it_b->eRtype;
                //printf("eType=%d,it_b->type=%d,it_b->bShow=%d\n",eType,it_b->type,it_b->bShow);

               /* if(!GetEventType(eType, it_b->type, it_b->nChannelIndex))
                {
                    it_b++;
                    continue;
                }*/
                event.uCode = (int)eType;
            //    LogTrace("eventCode.log", "EventCode=%u, seq=%u, Time:%s", event.uCode, event.uSeq, GetTimeCurrent().c_str());
                if(g_nServerType == 3) //旅行时间
                {
                    int nWayRate = -1;
                    int nStopIgn = -1;
                    int nAvgSpeedMin = -1;

                    paraDetectList::iterator it_d_b = m_sParamIn.begin();
                    paraDetectList::iterator it_d_e = m_sParamIn.end();
                    while(it_d_b != it_d_e)
                    {
                        if(it_d_b->nChannelID == it_b->nChannelIndex) //车道实际编号，非逻辑编号
                        {
                            //取第一报警等级
                            nWayRate = it_d_b->m_nWayRate[0];
                            nStopIgn = it_d_b->m_nStopIgnAlert;
                            nAvgSpeedMin = it_d_b->m_nAvgSpeedMin;

                            printf("=****####***===CSkpRoadDetect::OnFrame===nWayRate=%d, nStopIgn=%d, nAvgSpeedMin=%d===\n",\
                                   nWayRate, nStopIgn, nAvgSpeedMin);
                        }
                        it_d_b++;
                    }

                    if(DETECT_RESULT_EVENT_JAM == eType)//A
                    {
                        event.uEventDefine = nWayRate;
                    }
                    else if(DETECT_RESULT_EVENT_STOP == eType) //B
                    {
                        event.uEventDefine = nStopIgn;
                    }
                    else if(DETECT_RESULT_EVENT_DERELICT == eType ||\
                            DETECT_RESULT_EVENT_GO_AGAINST == eType ||\
                            DETECT_RESULT_EVENT_PERSON_AGAINST == eType) //C
                    {
                        event.uEventDefine = 1;
                    }
                    else if(DETECT_RESULT_EVENT_PERSON_APPEAR == eType) //D
                    {
                        event.uEventDefine = 1;
                    }
                    else if(DETECT_RESULT_EVENT_GO_SLOW == eType) //E
                    {
                        event.uEventDefine = nAvgSpeedMin;
                    }
                    else
                    {
                        event.uEventDefine = 0;
                    }
                }

				//
				//memcpy(event.chText,"沪A28669",9);

                event.uVideoBeginTime = sDetectHeader.uTimestamp-5;//事件发生前5秒
                event.uMiVideoBeginTime = 0;
                event.uVideoEndTime = sDetectHeader.uTimestamp + m_nCaptureTime - 5;//
                event.uMiVideoEndTime = 0;
                event.uEventBeginTime = sDetectHeader.uTimestamp;
                event.uMiEventBeginTime = (sDetectHeader.uTime64/1000)%1000;
                event.uEventEndTime = sDetectHeader.uTimestamp+5;
                event.uMiEventEndTime = 0;
               //是否是车牌事件
              if (m_nDetectKind&DETECT_EVENT_CARNUM)
              {
                   OnProcessPlateEvent(frame, (*it_b), event, sDetectHeader, uRoadIndex, strPicPath, flag);
				    if (it_b->eRtype == DETECT_RESULT_ALL || \
						 (it_b->bSavePic && it_b->bHaveCarnum == false))
				     {
						it_b++;
						continue;
				     }
              }
				else
			   {

                //真实事件才写数据库
                if(!(it_b->bShow))
                {
                    SRIP_DETECT_OUT_RESULT sResult = *it_b;
                    SRIP_DETECT_HEADER sHeader = sDetectHeader;

                    bool bAppear = false;

                    if(it_b->eRtype == DETECT_RESULT_EVENT_APPEAR ||
                       it_b->eRtype == DETECT_RESULT_EVENT_PERSON_APPEAR ||
                       it_b->eRtype == DETECT_RESULT_EVENT_WRONG_CHAN)
                    {
                        bAppear = true;
                    }
                    else
                    {
                        if(it_b->eRtype == DETECT_RESULT_EVENT_STOP)
                        {
							//手动抓拍模式下不在调用算法模块即不在报违章停车数据
							if(g_nPreSetMode == 1)
							{
								it_b++;
								continue;
							}

							if (g_ytControlSetting.nPicComPoseMode == 1)                                      //  保存方式远景远近的情况   
							{
								if ( it_b->nOutPutMode == NORMOAL_OUT_MODE )                                  // 正常模式输出
								{
									//LogNormal("--正常输出_远景  id=%d, size=%d--\n",it_b->nCarId, frame.size());
									unsigned char * temp;
									temp = (unsigned char *)malloc(frame.size());
									memcpy(temp, frame.c_str(), frame.size());
									std::map<int, list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(it_b->nCarId); 
									if (iter != m_mapRemotePicList.end())
									{
										iter->second.push_back(temp);
									}
									if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)                // 调近景
									{
										if(m_nHasLocalPreSet == 1)
										{
											CvRect rtPos;
											rtPos.x = it_b->carRect.x * m_fScaleX;
											rtPos.width = it_b->carRect.width * m_fScaleX;
											rtPos.y = it_b->carRect.y * m_fScaleY;
											rtPos.height = it_b->carRect.height * m_fScaleY;
											m_PreSetControl.GoToLocalPreSet(rtPos,sDetectHeader.uTimestamp);     //调用近景预置位
											sleep(12);  // 暂停12秒，来接收第四张图片
											string fourthStr;
											PopFrame(fourthStr);
											if(m_nExtent > 0)
											fourthStr.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));   //文字区域叠加在图片下方
											if(fourthStr.size() > 0)
											{
													unsigned char * temp2;
													temp2 = (unsigned char *)malloc(fourthStr.size());
													memset(temp2, '\0', fourthStr.size());
													memcpy(temp2, fourthStr.c_str(), fourthStr.size());
													std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(it_b->nCarId); 
													if (iter2 != m_mapRemotePicList.end())
													{
														iter2->second.push_back(temp2);
													}
											}
											//LogNormal("-- 正常输出_近景 id=%d, size=%d",it_b->nCarId, fourthStr.size());
											//event.uPosX += m_imgSnap->width;                                    //事件发生横坐
											string strCarNum = CarNumDetect(fourthStr);                         // 车牌的识别并把车牌号保存到event.chText;
											memcpy(event.chText,strCarNum.c_str() , strCarNum.length());
											//LogNormal("--车牌号为：%s\n", strCarNum.c_str());
										}
										else//从当前图片中抠图
										{
											printf("2 before GetViolationPos\n");
											IplImage* imgSnap = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
											cvSetData(imgSnap,temp+sizeof(SRIP_DETECT_HEADER),imgSnap->widthStep);

											CvPoint point;//事件坐标
											point.x = event.uPosX;
											point.y = event.uPosY;
											CvRect rtPos;
											GetViolationPos(point,rtPos);
											
											if(rtPos.height > 0)
											{
												//按目标区域比例裁剪
												rtPos.width = rtPos.height * imgSnap->width/imgSnap->height;

												if(rtPos.x + rtPos.width >= imgSnap->width)
												{
													rtPos.x = imgSnap->width - rtPos.width-1;
												}
												cvSetImageROI(imgSnap,rtPos);
												cvResize(imgSnap,m_imgSnap);
												cvResetImageROI(imgSnap);
											}
											
											printf("2 after GetViolationPos\n");

											unsigned char * temp2;
											temp2 = (unsigned char *)malloc(imgSnap->imageSize+sizeof(SRIP_DETECT_HEADER));
											memcpy(temp2, temp, sizeof(SRIP_DETECT_HEADER));
											memcpy(temp2+sizeof(SRIP_DETECT_HEADER), m_imgSnap->imageData, imgSnap->imageSize);
											std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(it_b->nCarId); 
											if (iter2 != m_mapRemotePicList.end())
											{
												iter2->second.push_back(temp2);
											}

											cvReleaseImageHeader(&imgSnap);
										}
										
									}
									
								}  // end of   if ( it_b->nOutPutMode == NORMOAL_OUT_MODE )  
								else if (it_b->nOutPutMode == SAVE_REMO_NEAR_PIC_NOOUT)              //存一组远/近景组合图，不输出
								{
									//LogNormal("---存一组图片_远景 id=%d ,size=%d, ",it_b->nCarId, frame.size());
									unsigned char * temp;
									temp = (unsigned char *)malloc(frame.size());
									memcpy(temp, frame.c_str(), frame.size());
									std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(it_b->nCarId); 
									if (iter == m_mapRemotePicList.end())
									{
										list<unsigned char *> listTemp;
										listTemp.push_back(temp);
										m_mapRemotePicList.insert(make_pair(it_b->nCarId, listTemp));
									}
									else
									{
										iter->second.push_back(temp);
									}
																		
									if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)        // 调近景
									{
										
										if(m_nHasLocalPreSet == 1)
										{
											CvRect rtPos;
											rtPos.x = it_b->carRect.x * m_fScaleX;
											rtPos.width = it_b->carRect.width * m_fScaleX;
											rtPos.y = it_b->carRect.y * m_fScaleY;
											rtPos.height = it_b->carRect.height * m_fScaleY;	
										//	LogNormal("调近景rect(%d,%d,%d,%d)\n", it_b->carRect.x, it_b->carRect.width, it_b->carRect.y, it_b->carRect.height);
											//LogNormal("调近景rect(%d,%d,%d,%d)\n", rtPos.x, rtPos.y, rtPos.width, rtPos.height);
											m_PreSetControl.GoToLocalPreSet(rtPos,sDetectHeader.uTimestamp);     //调用近景预置位
											sleep(12);  // 暂停12秒，

											string nearPic;
											PopFrame(nearPic); 
											if(m_nExtent > 0)
											nearPic.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));     //文字区域叠加在图片下方
											if(nearPic.size() > 0)                // 存图
											{
											//	LogNormal("nearPic.size() > 0\n");
												unsigned char * temp;
												temp = (unsigned char *)malloc(nearPic.size());
												memset(temp, '\0', nearPic.size());
												memcpy(temp, nearPic.c_str(), nearPic.size());
												std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(it_b->nCarId); 
												if (iter2 != m_mapRemotePicList.end())
												{
													iter2->second.push_back(temp);
												}
											}
										}
										else//从当前图片中抠图
										{
											//LogNormal("1 before GetViolationPos\n");
											IplImage* imgSnap = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
											cvSetData(imgSnap,temp+sizeof(SRIP_DETECT_HEADER),imgSnap->widthStep);

											CvPoint point;//事件坐标
											point.x = (it_b->carRect.x + it_b->carRect.width/2)* m_fScaleX;
											point.y = (it_b->carRect.y + it_b->carRect.height/2)* m_fScaleY;

											CvRect rtPos;
											GetViolationPos(point,rtPos);
											
											if(rtPos.height > 0)
											{
												//按目标区域比例裁剪
												rtPos.width = rtPos.height * imgSnap->width/imgSnap->height;

												if(rtPos.x + rtPos.width >= imgSnap->width)
												{
													rtPos.x = imgSnap->width - rtPos.width-1;
												}
												cvSetImageROI(imgSnap,rtPos);
												cvResize(imgSnap,m_imgSnap);
												cvResetImageROI(imgSnap);
											}

											//LogNormal("1 after GetViolationPos\n");

											unsigned char * temp2;
											temp2 = (unsigned char *)malloc(imgSnap->imageSize+sizeof(SRIP_DETECT_HEADER));
											memcpy(temp2, temp, sizeof(SRIP_DETECT_HEADER));
											memcpy(temp2+sizeof(SRIP_DETECT_HEADER), m_imgSnap->imageData, imgSnap->imageSize);
											std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(it_b->nCarId); 
											if (iter2 != m_mapRemotePicList.end())
											{
												iter2->second.push_back(temp2);
											}

											cvReleaseImageHeader(&imgSnap);

											//LogNormal("1 after memcpy\n");
										}
									}
									
									//LogNormal("  --一存一组图片_近景  id=%d, size=%d\n", it_b->nCarId, nearPic.size());
									it_b++;
									continue;
								}  // end  if (it_b->nOutPutMode == SAVE_REMO_NEAR_PIC_NOOUT)    
							
								else if ( it_b->nOutPutMode ==  DELETE_CACHE_PIC_NOOUT)        //删除缓存的图像，不输出
								{
									//LogNormal("----删除缓存图片--id = %d--\n", it_b->nCarId);
									std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(it_b->nCarId); 
									if (iter != m_mapRemotePicList.end())
									{
										for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
										{
											unsigned char * temp = NULL;
											temp = *iter2;
											if(temp != NULL)
											{
												free(temp);
												//*iter2 = NULL;
											}
												
										}
										m_mapRemotePicList.erase(iter);
									}
									it_b++;
									continue;
								}
							}   // end of 	if (g_ytControlSetting.nPicComPoseMode == 1)          
							else if (g_ytControlSetting.nPicComPoseMode == 0)                    //表示现在的方式 2*2 格式的 三张远景一张近景
							{
								if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
								{

									//LogNormal("m_mapRemotePicList.size=%d\n",m_mapRemotePicList.size());

									if(it_b->nOutPutMode == JUST_SAVE_REMO_PIC_NOOUT)           // 1 /只存图不输出结果
									{
									//	LogNormal("nCarId=%d++++++ 存图 1",it_b->nCarId);
										unsigned char * temp;
										temp = (unsigned char *)malloc(frame.size());
										memcpy(temp, frame.c_str(), frame.size());
										std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(it_b->nCarId); 

										if (iter == m_mapRemotePicList.end())
										{
											list<unsigned char *> listTemp;
											listTemp.push_back(temp);
											m_mapRemotePicList.insert(make_pair(it_b->nCarId, listTemp));
										}
										else
										{
											iter->second.push_back(temp);
										}
										it_b++;
										continue;
									}  //end of if(it_b->nOutPutMode ==  1)

									else if (it_b->nOutPutMode == DELETE_CACHE_PIC_NOOUT)      // 2车子已经离开，需要清除之前保存的图片
									{
										//LogNormal("nCarId=%d++++++ 删图 2",it_b->nCarId);
										std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(it_b->nCarId); 

										if (iter != m_mapRemotePicList.end())
										{
											for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
											{
												unsigned char * temp = NULL;
												temp = *iter2;
												if(temp != NULL)
												{
													free(temp);
												//	*iter2 = NULL;
												}
												
											}
											m_mapRemotePicList.erase(iter);
										}
										it_b++;
										continue;
									}// end of else if (it_b->nOutPutMode == 2) 

									else if (it_b->nOutPutMode == NORMOAL_OUT_MODE)
									{
										//存第三张图片
										//LogNormal("nCarId=%d++++++ 存图 3",it_b->nCarId);
										unsigned char * temp;
										temp = (unsigned char *)malloc(frame.size());
										memcpy(temp, frame.c_str(), frame.size());
										std::map<int, list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(it_b->nCarId); 
										if (iter != m_mapRemotePicList.end())
										{
											iter->second.push_back(temp);
										}
										// 调近景
										if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
										{
											if(m_nHasLocalPreSet == 1)
											{
												CvRect rtPos;
												rtPos.x = it_b->carRect.x * m_fScaleX;
												rtPos.width = it_b->carRect.width * m_fScaleX;
												rtPos.y = it_b->carRect.y * m_fScaleY;
												rtPos.height = it_b->carRect.height * m_fScaleY;
												m_PreSetControl.GoToLocalPreSet(rtPos,sDetectHeader.uTimestamp);	    //调用近景预置位

												sleep(10);                           // 暂停10秒，来接收第四张图片
												string fourthStr;                    //取第4张图
												PopFrame(fourthStr);
												if(m_nExtent > 0)
												fourthStr.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));	//文字区域叠加在图片下方					
												if(fourthStr.size() > 0)              
												{
													unsigned char * temp2;
													temp2 = (unsigned char *)malloc(fourthStr.size());
													memset(temp2, '\0', fourthStr.size());
													memcpy(temp2, fourthStr.c_str(), fourthStr.size());
													std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(it_b->nCarId); 
													if (iter2 != m_mapRemotePicList.end())
													{
														iter2->second.push_back(temp2);
													}
												}
												event.uPosX += m_imgSnap->width;                //事件发生横坐标
																	
												string strCarNum = CarNumDetect(fourthStr);   	// 车牌的识别并把车牌号保存到event.chText;
												memcpy(event.chText,strCarNum.c_str() , strCarNum.length());	
												//LogNormal(" 车牌号码--------%s\n",strCarNum.c_str());
											}
											else//从当前图片中抠图
											{
												IplImage* imgSnap = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
												cvSetData(imgSnap,temp+sizeof(SRIP_DETECT_HEADER),imgSnap->widthStep);

												CvPoint point;//事件坐标
												point.x = event.uPosX;
												point.y = event.uPosY;
												CvRect rtPos;
												GetViolationPos(point,rtPos);
												
												if(rtPos.height > 0)
												{
													//按目标区域比例裁剪
													rtPos.width = rtPos.height * imgSnap->width/imgSnap->height;

													if(rtPos.x + rtPos.width >= imgSnap->width)
													{
														rtPos.x = imgSnap->width - rtPos.width-1;
													}
													cvSetImageROI(imgSnap,rtPos);
													cvResize(imgSnap,m_imgSnap);
													cvResetImageROI(imgSnap);
												}

												unsigned char * temp2;
												temp2 = (unsigned char *)malloc(imgSnap->imageSize+sizeof(SRIP_DETECT_HEADER));
												memcpy(temp2, temp, sizeof(SRIP_DETECT_HEADER));
												memcpy(temp2+sizeof(SRIP_DETECT_HEADER), m_imgSnap->imageData, imgSnap->imageSize);
												std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(it_b->nCarId); 
												if (iter2 != m_mapRemotePicList.end())
												{
													iter2->second.push_back(temp2);
												}

												cvReleaseImageHeader(&imgSnap);
											}
										}
										
									}
								}
							}
                        }
                    }

                    if(bAppear)
                    {
                        //车流量统计
                        if(event.uType==0)//机动车
                        {
                            uFluxVehicle[it_b->nChannelIndex-1]++;
                        }
                        else if(event.uType==1)//行人
                        {
                            uFluxPerson[it_b->nChannelIndex-1]++;
                        }
                        else if(event.uType==2)//非机动车
                        {
                            uFluxNoneVehicle[it_b->nChannelIndex-1]++;
                        }
                        uFluxAll[it_b->nChannelIndex-1]++;    //车辆及行人总数
                    }

					PLATEPOSITION  TimeStamp[2];

					{
						//保存图片
						if(m_nDeinterlace==2)
						{
							cvResize(m_img,m_imgSnap);
						}
						else
						{
							cvCopy(m_img,m_imgSnap);
						}
					}

					//区分车还是非机动车
					#ifdef DETECT_VEHICLE
					bool bVehicle = m_MvDetector.mvIsVehicle(m_imgSnap,cvPoint(event.uPosX,event.uPosY));
					if(!bVehicle)
					{
						it_b++;
						continue;
					}
					#endif

					if(m_bTestResult)
					{
						CvPoint center;
						center.x= event.uPosX;
						center.y = event.uPosY;
						int radius = 25;
						CvScalar color = cvScalar(255,0,0);
						cvCircle(m_imgSnap,center,radius,color,2);
					}
					if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1 && m_nHasLocalPreSet == 1)
					{
						 // 当是一张近景三张远景的时候
						//LogNormal("开始制作一张近景三张远景");
						ComPoseImageForYuntai(event,it_b->nCarId);
					}
					else
					{
						//LogNormal("--非停车事件\n");
						//需要判断检测结果类型，如果是违法行为需要保存两张图片
						if( (m_nSaveImageCount == 2) || (!bAppear)||((m_nSmallPic == 1)&&(m_nSaveImageCount == 1)&&(!bAppear)))
						{
							//查找5秒前的图片数据
							int  nTimeInterval = 500000;//单位为微秒
							int64_t uTimestamp = sDetectHeader.uTime64;
							if(it_b->fLastImgTime >= 0)
							{
							//	nTimeInterval = (int)(it_b->fLastImgTime*1000000);
							}
							//LogNormal("m_imgPreSnap nTimeInterval=%d\n",nTimeInterval);

							SRIP_DETECT_HEADER sPreHeader;

							{
								sPreHeader = GetImageFromResultFrameList(uTimestamp,nTimeInterval,m_imgPreSnap,sDetectHeader.uSeq);
							}

							event.uTime2 = sPreHeader.uTimestamp;
							event.uMiTime2 = ((sPreHeader.uTime64)/1000)%1000;

							//拼图
							event.uPicWidth = m_imgComposeSnap->width;  //事件快照宽度
							//事件快照高度
							event.uPicHeight = m_imgComposeSnap->height;
								
								//深圳北环格式
								if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
								{
									//获取第3张图
									SRIP_DETECT_HEADER sAuxHeader;
									
									sAuxHeader = GetImageFromResultFrameList(uTimestamp,nTimeInterval+500000,m_imgAuxSnap,sPreHeader.uSeq);

									for(int nIndex= 0; nIndex<4; nIndex++)
									{
										CvRect rect;
										rect.x = (nIndex%2)*m_imgSnap->width;
										rect.y = (nIndex/2)*m_imgSnap->height;
										rect.width = m_imgSnap->width;
										rect.height = m_imgSnap->height;

										cvSetImageROI(m_imgComposeSnap,rect);

										if(nIndex == 0)
										{
											cvCopy(m_imgAuxSnap,m_imgComposeSnap);
										}
										else if(nIndex == 1)
										{
											cvCopy(m_imgPreSnap,m_imgComposeSnap);
										}
										else if(nIndex == 2)
										{
											cvCopy(m_imgSnap,m_imgComposeSnap);
										}
										else if(nIndex == 3)
										{
											CvPoint point;//事件坐标
											point.x = event.uPosX;
											point.y = event.uPosY;
											CvRect rtPos;
											GetViolationPos(point,rtPos);
											
											if(rtPos.height > 0)
											{
												//按目标区域比例裁剪
												rtPos.width = rtPos.height * rect.width/rect.height;

												if(rtPos.x + rtPos.width >= rect.width)
												{
													rtPos.x = rect.width - rtPos.width-1;
												}
												cvSetImageROI(m_imgSnap,rtPos);
												cvResize(m_imgSnap,m_imgComposeSnap);
												cvResetImageROI(m_imgSnap);
											}

											event.uPosY +=  m_imgComposeSnap->height/2;	
										}
										cvResetImageROI(m_imgComposeSnap);
									}

									PutTextOnComposeImage(m_imgComposeSnap,event,&sAuxHeader);
								}
								//2*2
								else if(1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
								{
									{
										for(int nIndex= 0; nIndex<3; nIndex++)
										{
											CvRect rect;
											if(nIndex < 2)
											{
												rect.x = (nIndex%2)*m_imgSnap->width;
												rect.y = 0;
												rect.width = m_imgSnap->width;
												rect.height = m_imgSnap->height;
											}
											else if(nIndex == 2)
											{
												rect.x = 2*m_imgSnap->width;
												rect.y = 0;
												rect.width = m_imgComposeSnap->width - rect.x;
												rect.height = m_imgComposeSnap->height;
											}

											cvSetImageROI(m_imgComposeSnap,rect);

											if(nIndex == 0)
											{
												cvCopy(m_imgPreSnap,m_imgComposeSnap);
											}
											else if(nIndex == 1)
											{
												cvCopy(m_imgSnap,m_imgComposeSnap);
											}
											else if(nIndex == 2)
											{
												CvPoint point;//事件坐标
												point.x = event.uPosX;
												point.y = event.uPosY;
												CvRect rtPos;
												GetViolationPos(point,rtPos);
												
												if(rtPos.height > 0)
												{
													//按目标区域比例裁剪
													rtPos.width = rtPos.height * rect.width/rect.height;

													if(rtPos.x + rtPos.width >= rect.width)
													{
														rtPos.x = rect.width - rtPos.width-1;
													}

													cvSetImageROI(m_imgSnap,rtPos);
													cvResize(m_imgSnap,m_imgComposeSnap);
													cvResetImageROI(m_imgSnap);
												}
											}
											cvResetImageROI(m_imgComposeSnap);
										}
										event.uPosX += m_imgSnap->width;                //事件发生横坐标
										PutTextOnComposeImage(m_imgComposeSnap,event);
									}
								}
								//是否需要存储小图
								else if(m_nSmallPic == 1&&m_nSaveImageCount == 2)
								{
									for(int nIndex= 0; nIndex<4; nIndex++)
									{
										CvRect rect;
										if(nIndex < 2)
										{
											rect.x = (nIndex%2)*m_imgSnap->width;
											rect.y = 0;
											rect.width = m_imgSnap->width;
											rect.height = m_imgSnap->height;
										}
										else if(nIndex ==2)
										{
											rect.x = 2*m_imgSnap->width;
											if(m_nWordPos == 1)
											rect.y = m_nExtent;
											else
											rect.y = 0;
											rect.width = m_imgComposeSnap->width - rect.x;
											rect.height = m_imgComposeSnap->height - m_nExtent;
										}
										else
										{
											rect.x = 0;
											if(m_nWordPos == 1)
											rect.y = 0;
											else
											rect.y = m_imgComposeSnap->height - m_nExtent;
											rect.width = m_imgComposeSnap->width;
											rect.height = m_nExtent;
										}

										if(rect.height>0)
										cvSetImageROI(m_imgComposeSnap,rect);

										if(nIndex == 0)
										{
											cvCopy(m_imgSnap, m_imgComposeSnap);
										}
										else if(nIndex == 1)
										{
											cvCopy(m_imgPreSnap, m_imgComposeSnap);
										}
										else if(nIndex == 2)
										{
											CvPoint point;//事件坐标
											point.x = event.uPosX;
											point.y = event.uPosY;
											CvRect rtPos;
											GetViolationPos(point,rtPos,1);
											
											if(rtPos.height > 0)
											{
												//按目标区域比例裁剪
												rtPos.width = rtPos.height * rect.width/rect.height;

												if(rtPos.x + rtPos.width >= rect.width)
												{
													rtPos.x = rect.width - rtPos.width-1;
												}

												cvSetImageROI(m_imgSnap,rtPos);
												cvResize(m_imgSnap,m_imgComposeSnap);
												cvResetImageROI(m_imgSnap);
											}
										}
										else
										{
											 if(rect.height>0)
											 cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));
										}

										if(rect.height>0)
										cvResetImageROI(m_imgComposeSnap);
									}
									PutTextOnComposeImage(m_imgComposeSnap,event);
								}
								else
								{
									CvRect rect;
									rect.x = 0;
									if(m_nWordPos == 1)
									rect.y = 0;
									else
									rect.y = m_imgComposeSnap->height - m_nExtent;
									rect.width = m_imgComposeSnap->width;
									rect.height = m_nExtent;

									if(m_nExtent > 0)
									{
										cvSetImageROI(m_imgComposeSnap,rect);
										cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));
										cvResetImageROI(m_imgComposeSnap);
									}

									//叠加信息
									for(int nIndex= 0; nIndex<2; nIndex++)
									{
										rect.x = (nIndex%2)*m_imgSnap->width;
										rect.y = 0;
										rect.width = m_imgSnap->width;
										rect.height = m_imgSnap->height;
										
										if(rect.height > 0)
										cvSetImageROI(m_imgComposeSnap,rect);

										if(nIndex == 0)
										{
											cvCopy(m_imgSnap, m_imgComposeSnap);
										}
										else if(nIndex == 1)
										{
											cvCopy(m_imgPreSnap, m_imgComposeSnap);
										}

										cvResetImageROI(m_imgComposeSnap);

										PutTextOnImage(m_imgComposeSnap,event,&sPreHeader,nIndex);
									}
								}
						}
						else//卡口只需要一张图片
						{
							event.uPicWidth = m_imgSnap->width;  //事件快照宽度
							//事件快照高度
							event.uPicHeight = m_imgSnap->height;
							//叠加信息
							PutTextOnImage(m_imgSnap,event);
						}
					}
				
                    //加锁
                    pthread_mutex_lock(&g_Id_Mutex);
                    if(!flag && (!bAppear))
                    {
                        if(m_eCapType == CAPTURE_FULL)
                        {
							{  
								 //直接从全天录象中获取事件录象视频
								 strPath = g_FileManage.GetEncodeFileName();
							}

                            printf("=======222=====event==========strPath=====================%s\n",strPath.c_str());

                            strTmpPath = strPath.erase(0,g_strVideo.size());
                            strTmpPath = g_ServerHost+strTmpPath;
                            strPath = "ftp://"+strTmpPath;
                            memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径
                        }
                        else
                        {
                            if(m_bEventCapture)
                            {								
								{                                
                                        //需要判断磁盘是否已经满
                                        g_FileManage.CheckDisk(true,true);

                                        sDetectHeader.uVideoId = g_uVideoId;
                                        //获取事件录象名称
                                        strPath = g_FileManage.GetEventVideoPath(g_uVideoId);

                                        strTmpPath = strPath.erase(0,g_strVideo.size());
                                        strTmpPath = g_ServerHost+strTmpPath;
                                        strPath = "ftp://"+strTmpPath;
                                        memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径
                                        printf("============event==========strPath=====================%s\n",strPath.c_str());
                                        //删除已经存在的记录
                                       // g_skpDB.DeleteOldRecord(strPath,true,true);
                                }
                            }
                        }
                        flag = true;
                    }
					
					int nSaveRet = 0;
					if(g_nPicSaveMode == 0)
					{
						//需要判断磁盘是否已经满
						g_FileManage.CheckDisk(true,false);
						//获取快照图片名称及大小
						strPicPath = g_FileManage.GetPicPath();

						nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);

						memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size()); //事件图片路径
					}
					else
					{
						nSaveRet = GetEventPicPath(event,strPicPath);

						string strGBKPicPath = strPicPath;
						g_skpDB.GBKToUTF8(strGBKPicPath);

						memcpy(event.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());
					}
                    //解锁
                    pthread_mutex_unlock(&g_Id_Mutex);

					if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1 && m_nHasLocalPreSet == 1)
					{
						//保存2x2的图片记录
						event.uPicSize = SaveImage(m_imgComposeStopSnap,strPicPath);
					}
					else
					{
						//需要判断检测结果类型，如果是违法行为需要保存两张图片
						if( (m_nSaveImageCount == 2) || (!bAppear)||((m_nSmallPic == 1&&m_nSaveImageCount == 1)&&(!bAppear)))//违章行为
						{
							//保存图片
							event.uPicSize = SaveImage(m_imgComposeSnap,strPicPath);
						}
						else//卡口只需要一张图片
						{
							//保存图片
						//	LogNormal("---------bao cun tu pian \n");
							event.uPicSize = SaveImage(m_imgSnap,strPicPath);
						}
					}
					
                    //删除已经存在的记录
                        g_skpDB.DeleteOldRecord(strPicPath,false,false);
                    //记录检测数据,保存事件
                    if(nSaveRet>0)
					  {

                        g_skpDB.SaveTraEvent(sDetectHeader.uChannelID,event,0,0,true);
					  }
					}
                }

				//利用保留字段存储bshow和缩放比
				memcpy(event.chReserved,&it_b->bShow,sizeof(bool));

				//if(!it_b->bShow)
			//	LogNormal("it_b->bShow=%d,nChannelIndex=%d,m_list_DetectOut.size()=%d\n",it_b->bShow,it_b->nChannelIndex,m_list_DetectOut.size());


                //发往客户端的坐标需要乘缩放比
				if(g_VideoFormatInfo.nSendH264 == 0)
				{
					event.uPosX = (int)it_b->x;     //事件发生横坐标
					event.uPosY = (int)it_b->y;     //事件发生纵坐标
				}
				else
				{
					event.uPosX = (int)it_b->x*(g_nVideoWidth*m_fScaleX/m_img->width);     //事件发生横坐标
					event.uPosY = (int)it_b->y*(g_nVideoHeight*m_fScaleY/(m_img->height-m_nExtent/m_nDeinterlace));     //事件发生纵坐标
					printf("===============event.uPosX= %d,event.uPosY= %d,it_b->x=%d,it_b->y=%d\n",event.uPosX,event.uPosY,(int)it_b->x,(int)it_b->y);
				}

				strEvent.append((char*)&event,sizeof(RECORD_EVENT));

				if (m_nDetectKind&DETECT_EVENT_CARNUM)
				{
					if(!it_b->bHaveCarnum)
					{
						strEventPlate.append((char*)&event,sizeof(RECORD_EVENT));
					}
					else
					{
						sDetectHeader.uDetectType = SRIP_EVENT_PLATE_VIDEO;
					}
				}
            }
            else//统计
            {
                bStat = true;
				VehicleStatistic(uRoadIndex,it_b,statistic);
            }
            it_b++;
        }

		if(strEvent.size()>0)
		{
			strEvent.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		}

		if (strEventPlate.size()>0)
		{
			sDetectHeader.uDetectType = SRIP_DETECT_EVENT;
			strEventPlate.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		}

        //有事件才通知进行事件录象
        if(flag)
        {
            if(m_bEventCapture)
            {
                std::string strEventEx = strEvent;
                if(g_nEncodeFormat == 2 || g_nEncodeFormat == 1)
                {
                    if(m_nWordPos == 0)
                    strEventEx.append((char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER)),sDetectHeader.uWidth*sDetectHeader.uHeight*3);
                    else if(m_nWordPos == 1)
					{
						if(m_nExtent > 0)
						strEventEx.append((char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER)+m_img->widthStep*(m_nExtent/m_nDeinterlace)),sDetectHeader.uWidth*sDetectHeader.uHeight*3);
					}
                }
                //通知录象线程进行事件录象;
                if(m_eCapType != CAPTURE_FULL)
                {
				//	LogTrace("record.log", "begin to record:chVideoPath:%s, sHeader.uVideoId=%u\n",event.chVideoPath, sDetectHeader.uVideoId);
					//LogTrace("AddEvent.log", "=444===m_skpRoadRecorder.AddEvent=m_eCapType=%d=strEventEx.size()=%d\n", m_eCapType, strEventEx.size());
                    m_skpRoadRecorder.AddEvent(strEventEx);
                }
            }
        }
	
        //保存统计
        if(bStat)
        {
            statistic.uTime = sDetectHeader.uTimestamp;
            statistic.uStatTimeLen = m_nTrafficStatTime;

            //记录检测数据,保存统计
			if(g_nServerType == 7)
			{
				g_skpDB.SaveStatisticInfo(sDetectHeader.uChannelID,statistic,PERSON_ROAD);
			
			}
			else
			{	
				if((m_nDetectKind&DETECT_CARNUM)!=DETECT_CARNUM)//不做车牌才统计
					g_skpDB.SaveStatisticInfo(sDetectHeader.uChannelID,statistic,PERSON_ROAD);
			}
        }

        //printf("======================================m_bConnect=%d,strEvent.size()=%d\n",m_bConnect,strEvent.size());
        //////有连接才发送
        if(m_bConnect)
        {
			if (m_nDetectKind&DETECT_EVENT_CARNUM)
			{
				//发送事件
				if(strEventPlate.size()>0)
				 {
					g_skpChannelCenter.AddResult(strEventPlate);
				 }

			}
			else
			{
				//发送事件
				if(strEvent.size()>0)
				 {
					  g_skpChannelCenter.AddResult(strEvent);
				 }

				if(bStat)    // 车辆排队
				{   
					string strStatistic;
					sDetectHeader.uDetectType = SRIP_DETECT_STATISTIC;
					strStatistic.append((char *)&sDetectHeader,sizeof(sDetectHeader));
					strStatistic.append((char *)&statistic,sizeof(statistic));
					g_skpChannelCenter.AddResult(strStatistic);
				}
			}

        }
    }
}

void CSkpRoadDetect::OutPutAutoCtrlResult(std::string& frame)
{
	printf("CSkpRoadDetect::OutPutAutoCtrlResult,list_output.size=%d \n",(int)m_list_DetectOut.size());
    //检测完成，处理结果
    if(m_list_DetectOut.size()>0)
    {
        SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

        //事件统计列表
        DetectResultList::iterator it_b = m_list_DetectOut.begin();
        DetectResultList::iterator it_e = m_list_DetectOut.end();

        bool flag = false;//判断是否真实事件
        bool bStat = false;//判断是否统计


        RECORD_STATISTIC statistic;//事件和统计分开发送
        RECORD_EVENT event;
        std::string strPicPath,strPath,strTmpPath,strEvent,strStat;
		std::string strEventPlate;
        sDetectHeader.uDetectType = SRIP_DETECT_EVENT;

        UINT32 uRoadIndex = 0;
		if(m_nExtent > 0)
		{
			//需要append文本区域
			if(m_nWordPos == 0)//文字区域叠加在图片下方
			frame.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));
			else if(m_nWordPos == 1)
			frame.insert(sizeof(SRIP_DETECT_HEADER),m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));
		}

		char* data = (char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER));
		cvSetData(m_img,data,m_img->widthStep);

        //事件地点
        memcpy(event.chPlace,m_strLocation.c_str(),m_strLocation.size());

        DETECT_RESULT_TYPE eType;

        while(it_b!=it_e)
        {
			printf("CSkpRoadDetect::OutPutAutoCtrlResult:eRtype = %d,CarId=%d \n",(int)it_b->eRtype,it_b->nCarId);
            //根据车道号获取车道逻辑编号
            RoadIndexMap::iterator it_p = m_roadMap.find(it_b->nChannelIndex);
            if(it_p != m_roadMap.end())
            {
                uRoadIndex = it_p->second.nVerRoadIndex;
            }
			else
			{
				uRoadIndex = it_b->nChannelIndex;
			}

            //事件
            if(it_b->eRtype < DETECT_RESULT_STAT_FLUX)
            {
				if( ((m_nDetectKind&DETECT_VIOLATION)) && (it_b->eRtype == DETECT_RESULT_EVENT_GO_FAST) )//不做违章检测才报超速
				{
					it_b++;
					continue;
				}
                //行驶方向
                int nDirection = 0;
                if(it_b->direction < 180)
                {
                    nDirection = 0;
                }
                else
                {
                    nDirection = 1;
                }

				if(m_nDetectDirection != nDirection)
				{
					if(m_nDirection%2==0)
					{
						event.uDirection = m_nDirection-1;
					}
					else
					{
						event.uDirection = m_nDirection+1;
					}
				}
				else
				{
					event.uDirection = m_nDirection;
				}

				if(it_b->eRtype == DETECT_RESULT_EVENT_STOP)
				{
					event.uDirection = m_nDirection;
				}


                event.uRoadWayID = uRoadIndex;//车道编号

                //存数据库的坐标需要乘缩放比
				{
					event.uPosX = (int)it_b->x*m_fScaleX;     //事件发生横坐标
					event.uPosY = (int)it_b->y*m_fScaleY;     //事件发生纵坐标
				}

                if(m_nWordPos == 1)
					event.uPosY += m_nExtent;

                event.uColor1 = it_b->color.nColor1;
                event.uColor2 = it_b->color.nColor2;
                event.uColor3 = it_b->color.nColor3;
                event.uWeight1 = it_b->color.nWeight1;
                event.uWeight2 = it_b->color.nWeight2;
                event.uWeight3 = it_b->color.nWeight3;
                event.uType = it_b->type;//判断人还是车
                event.uSpeed = it_b->value;
                eType = it_b->eRtype; 
                event.uCode = (int)eType;
               // event.uVideoBeginTime = sDetectHeader.uTimestamp-5;//事件发生前5秒
              //  event.uMiVideoBeginTime = 0;
              //  event.uVideoEndTime = sDetectHeader.uTimestamp + m_nCaptureTime - 5;//
             //   event.uMiVideoEndTime = 0;
                event.uEventBeginTime = sDetectHeader.uTimestamp;
                event.uMiEventBeginTime = (sDetectHeader.uTime64/1000)%1000;
                event.uEventEndTime = sDetectHeader.uTimestamp+5;
                event.uMiEventEndTime = 0;

                //是否是车牌事件
				if (m_nDetectKind&DETECT_EVENT_CARNUM)
				{
					OnProcessPlateEvent(frame, (*it_b), event, sDetectHeader, uRoadIndex, strPicPath, flag);
					if (it_b->eRtype == DETECT_RESULT_ALL || (it_b->bSavePic && it_b->bHaveCarnum == false))
					{
						it_b++;
						continue;
					}
				}
				else
			    {
					//真实事件才写数据库
					if(!(it_b->bShow))
					{
						SRIP_DETECT_OUT_RESULT sResult = *it_b;
						SRIP_DETECT_HEADER sHeader = sDetectHeader;

						bool bAppear = false;

						if(it_b->eRtype == DETECT_RESULT_EVENT_APPEAR ||
							it_b->eRtype == DETECT_RESULT_EVENT_PERSON_APPEAR ||
							it_b->eRtype == DETECT_RESULT_EVENT_WRONG_CHAN)
						{
							bAppear = true;
						}
						else
						{
							if(it_b->eRtype == DETECT_RESULT_EVENT_STOP)
							{
								//手动抓拍模式下不在调用算法模块即不在报违章停车数据
								if(g_nPreSetMode == 1)
								{
									it_b++;
									continue;
								}

								if(!DetectAutoEventStop(frame,&sResult,event))
								{
									it_b++;
									continue;
								}
							}
						}

						if(bAppear)
						{
							//车流量统计
							if(event.uType==0)//机动车
							{
								uFluxVehicle[it_b->nChannelIndex-1]++;
							}
							else if(event.uType==1)//行人
							{
								uFluxPerson[it_b->nChannelIndex-1]++;
							}
							else if(event.uType==2)//非机动车
							{
								uFluxNoneVehicle[it_b->nChannelIndex-1]++;
							}
							uFluxAll[it_b->nChannelIndex-1]++;    //车辆及行人总数
						}

						cvCopy(m_img,m_imgSnap);

						if( it_b->eRtype != DETECT_RESULT_EVENT_STOP )
						{
							DetectAutoCarEvent(frame,bAppear,event);
						}
						else   
						{
							if (g_ytControlSetting.nPicComPoseMode !=2)
							{
								//LogNormal("开始制作一张近景三张远景");
								ComPoseImage(event,it_b->nCarId);
							}
							
						
							
						}
						//加锁
						pthread_mutex_lock(&g_Id_Mutex);
						if(!flag && (!bAppear))
						{
							if(m_eCapType == CAPTURE_FULL)
							{
								{  
										//直接从全天录象中获取事件录象视频
										strPath = g_FileManage.GetEncodeFileName();
								}
								
								printf("=======222=====event==========strPath=====================%s\n",strPath.c_str());

								strTmpPath = strPath.erase(0,g_strVideo.size());
								strTmpPath = g_ServerHost+strTmpPath;
								strPath = "ftp://"+strTmpPath;
								memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径
							}
							else
							{
								if(m_bEventCapture)
								{									          
									if(m_nCameraType == DH_CAMERA)
									{
										unsigned int nVideoIdTmp = 10000;
										if(m_mapAutoCameraRecord.find(it_b->nCarId) != m_mapAutoCameraRecord.end())
										{
											nVideoIdTmp = (unsigned int)m_mapAutoCameraRecord[it_b->nCarId];
											m_mapAutoCameraRecord.erase(it_b->nCarId);
											LogNormal("nVideoIdTmp=%d \n",nVideoIdTmp);
										}
										else
										{
											it_b++;
											continue;
										}

										string strVideoPath = g_strVideo;
										if(g_nServerType == 7)
										{
											strVideoPath = "/home/road/red";
											if (IsDataDisk())
											{
												strVideoPath = "/detectdata/red";
											}
											RECORD_PLATE plate;
											plate.uTime = sDetectHeader.uTimestamp;
											plate.uRoadWayID = event.uRoadWayID;
											plate.uDirection = m_nDirection;
											memcpy(plate.chText,event.chText,sizeof(event.chText));
											g_RoadImcData.GetPlatePicPath(plate,strPath,6);
										}
										else
										{
											sDetectHeader.uVideoId = g_uVideoId++;
											//获取事件录象名称
											strPath = g_FileManage.GetEventVideoPath(sDetectHeader.uVideoId);
										}
										LogNormal("path:%s \n",strPath.c_str());
										//LogNormal("strVideoPath:%s \n",strVideoPath.c_str());
										sleep(2); //防止录像文件没有close
										char szbody[200] = {0};
										sprintf(szbody,"%s/%d.mp4",g_strVideo.c_str(),nVideoIdTmp);
										//sprintf(szbody,"mv %s/%d.mp4 %s",g_strVideo.c_str(),nVideoIdTmp,strPath.c_str());
										//system(szbody);
										rename(szbody,strPath.c_str());//用rename不用system方式修改文件名称

										strTmpPath = strPath.erase(0,strVideoPath.size());
										//LogNormal("strTmpPath:%s \n",strTmpPath.c_str());
										strTmpPath = g_ServerHost+strTmpPath;
										strPath = "ftp://"+strTmpPath;
										memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径
										//LogNormal("evestrPath:%s\n",strPath.c_str());
										//删除已经存在的记录
										// g_skpDB.DeleteOldRecord(strPath,true,true);

									}
									else
									{
										//需要判断磁盘是否已经满
										g_FileManage.CheckDisk(true,true);

										sDetectHeader.uVideoId = g_uVideoId;
										//获取事件录象名称
										strPath = g_FileManage.GetEventVideoPath(g_uVideoId);

										strTmpPath = strPath.erase(0,g_strVideo.size());
										strTmpPath = g_ServerHost+strTmpPath;
										strPath = "ftp://"+strTmpPath;
										memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径
										printf("============event==========strPath=====================%s\n",strPath.c_str());
										//删除已经存在的记录
										// g_skpDB.DeleteOldRecord(strPath,true,true);
									}

									
								}
								else
								{

									{
										char szFtpPath[512] = {0};
										string strEventTime = GetTime(m_dwEventTime,2);
										sprintf(szFtpPath,"ftp://%s/%d-%d-%s.avi",g_strFtpServerHost.c_str(),atoi(g_strDetectorID.c_str()),event.uDirection,strEventTime.c_str());
										memcpy(event.chVideoPath,szFtpPath,strlen(szFtpPath));//事件录象路径
										printf("============event==========strPath=====================%s\n",szFtpPath);
									}



									//删除已经存在的记录
									// g_skpDB.DeleteOldRecord(strPath,true,true);
								}



							}
							flag = true;
						}
					
						int nSaveRet = 0;
						if(g_nPicSaveMode == 0)
						{
							printf("g_nPicSaveMode == 0\n");
							//需要判断磁盘是否已经满
							g_FileManage.CheckDisk(true,false);
							//获取快照图片名称及大小
							if(!m_bEventCapture)
							{
								if(g_nServerType == 7)
								{
									//void GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath,int nType,int nRandCode = 0,int nIndex = 1);
									if(g_ytControlSetting.nPicComPoseMode != 2)
									{
										if(m_mapVideoTime.find(it_b->nCarId) == m_mapVideoTime.end())
										{
											event.uVideoBeginTime = event.uVideoEndTime - 420;
										}
										else
										{
											event.uVideoBeginTime = m_mapVideoTime[it_b->nCarId];
											m_mapVideoTime.erase(it_b->nCarId);
										}
										event.uTime2 = m_dwEventTime;
										GetProjectEventPicPath(event,strPicPath);
									}

								}
								else
								{
									strPicPath = g_FileManage.GetPicPath();
								}
							}
							else
							{
								printf("g_FileManage.GetPicPath()\n");
								strPicPath = g_FileManage.GetPicPath();
							}
							

							nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);
							if(g_ytControlSetting.nPicComPoseMode != 2)
							{
								memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size()); //事件图片路径
							}
							
						}
						else
						{
							printf("g_nPicSaveMode != 0\n");
							nSaveRet = GetEventPicPath(event,strPicPath);

							string strGBKPicPath = strPicPath;
							g_skpDB.GBKToUTF8(strGBKPicPath);
							if(g_ytControlSetting.nPicComPoseMode != 2)
							{
								memcpy(event.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());
							}
							
						}
						//解锁
						pthread_mutex_unlock(&g_Id_Mutex);

						if(it_b->eRtype != DETECT_RESULT_EVENT_STOP)
						{
							//需要判断检测结果类型，如果是违法行为需要保存两张图片
							if( (m_nSaveImageCount == 2) || (!bAppear)||((m_nSmallPic == 1&&m_nSaveImageCount == 1)&&(!bAppear)))//违章行为
							{
								//保存图片
								event.uPicSize = SaveImage(m_imgComposeSnap,strPicPath);
							}
							else//卡口只需要一张图片
							{
								//保存图片
							//	LogNormal("---------bao cun tu pian \n");
								event.uPicSize = SaveImage(m_imgSnap,strPicPath);
							}
						}
						else
						{
							//保存2x2的图片记录
							if (g_ytControlSetting.nPicComPoseMode !=2)
							{
								event.uPicSize = SaveImage(m_imgComposeStopSnap,strPicPath);
								printf("%s:%s\n",__FUNCTION__,strPicPath.c_str());
							}
							
						}
					
						//删除已经存在的记录
						g_skpDB.DeleteOldRecord(strPicPath,false,false);
						//记录检测数据,保存事件
						if(nSaveRet>0)
						{
							
							if(m_nCameraType == DH_CAMERA)
							{
// 								printf("strPath:%s \n",strPath.c_str());
// 								g_skpDB.VideoSaveUpdate(strPath,2);
								g_skpDB.SaveTraEvent(sDetectHeader.uChannelID,event,0,0,true,1);
							}
							else
							{
								g_skpDB.SaveTraEvent(sDetectHeader.uChannelID,event,0,0,true);
							}
						}
					}
                }

				//利用保留字段存储bshow和缩放比
				memcpy(event.chReserved,&it_b->bShow,sizeof(bool));

			
                //发往客户端的坐标需要乘缩放比
				if(g_VideoFormatInfo.nSendH264 == 0)
				{
					event.uPosX = (int)it_b->x;     //事件发生横坐标
					event.uPosY = (int)it_b->y;     //事件发生纵坐标
				}
				else
				{
					event.uPosX = (int)it_b->x*(g_nVideoWidth*m_fScaleX/m_img->width);     //事件发生横坐标
					event.uPosY = (int)it_b->y*(g_nVideoHeight*m_fScaleY/(m_img->height-m_nExtent/m_nDeinterlace));     //事件发生纵坐标
					printf("===============event.uPosX= %d,event.uPosY= %d,it_b->x=%d,it_b->y=%d\n",event.uPosX,event.uPosY,(int)it_b->x,(int)it_b->y);
				}

				strEvent.append((char*)&event,sizeof(RECORD_EVENT));

				if (m_nDetectKind&DETECT_EVENT_CARNUM)
				{
					if(!it_b->bHaveCarnum)
					{
						strEventPlate.append((char*)&event,sizeof(RECORD_EVENT));
					}
					else
					{
						sDetectHeader.uDetectType = SRIP_EVENT_PLATE_VIDEO;
					}
				}
            }
            else//统计
            {
                bStat = true;
				VehicleStatistic(uRoadIndex,it_b,statistic);
            }
            it_b++;
        }

        if(strEvent.size()>0)
		{
            strEvent.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		}

		if (strEventPlate.size()>0)
		{
			sDetectHeader.uDetectType = SRIP_DETECT_EVENT;
			strEventPlate.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		}

        //有事件才通知进行事件录象
        if(flag)
        {
            if(m_bEventCapture)
            {
                std::string strEventEx = strEvent;
                if(g_nEncodeFormat == 2 || g_nEncodeFormat == 1)
                {
                    if(m_nWordPos == 0)
                    strEventEx.append((char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER)),sDetectHeader.uWidth*sDetectHeader.uHeight*3);
                    else if(m_nWordPos == 1)
					{
						if(m_nExtent > 0)
						strEventEx.append((char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER)+m_img->widthStep*(m_nExtent/m_nDeinterlace)),sDetectHeader.uWidth*sDetectHeader.uHeight*3);
					}
                }
                //通知录象线程进行事件录象;
                if(m_eCapType != CAPTURE_FULL)
                {
				//	LogTrace("record.log", "begin to record:chVideoPath:%s, sHeader.uVideoId=%u\n",event.chVideoPath, sDetectHeader.uVideoId);
					//LogTrace("AddEvent.log", "=444===m_skpRoadRecorder.AddEvent=m_eCapType=%d=strEventEx.size()=%d\n", m_eCapType, strEventEx.size());
					if(m_nCameraType != DH_CAMERA)
					{
						 m_skpRoadRecorder.AddEvent(strEventEx);
					}
                   
                }
            }
        }
	
        //保存统计
        if(bStat)
        {
            statistic.uTime = sDetectHeader.uTimestamp;
            statistic.uStatTimeLen = m_nTrafficStatTime;

            //记录检测数据,保存统计
			if(g_nServerType == 7)
			{
				g_skpDB.SaveStatisticInfo(sDetectHeader.uChannelID,statistic,PERSON_ROAD);
			
			}
			else
			{	
				if((m_nDetectKind&DETECT_CARNUM)!=DETECT_CARNUM)//不做车牌才统计
					g_skpDB.SaveStatisticInfo(sDetectHeader.uChannelID,statistic,PERSON_ROAD);
			}
        }

        //printf("======================================m_bConnect=%d,strEvent.size()=%d\n",m_bConnect,strEvent.size());
        //////有连接才发送
        if(m_bConnect)
        {
			if (m_nDetectKind&DETECT_EVENT_CARNUM)
			{
				//发送事件
				if(strEventPlate.size()>0)
				 {
					g_skpChannelCenter.AddResult(strEventPlate);
				 }

			}
			else
			{
				//发送事件
				if(strEvent.size()>0)
				 {
					  g_skpChannelCenter.AddResult(strEvent);
				 }

				if(bStat)    // 车辆排队
				{   
					string strStatistic;
					sDetectHeader.uDetectType = SRIP_DETECT_STATISTIC;
					strStatistic.append((char *)&sDetectHeader,sizeof(sDetectHeader));
					strStatistic.append((char *)&statistic,sizeof(statistic));
					g_skpChannelCenter.AddResult(strStatistic);
				}
			}

        }
    }
}



int CSkpRoadDetect::ComPoseImage(RECORD_EVENT& event, int key)
{
	std::map<int, list<unsigned char *> >::iterator iterMap = m_mapRemotePicList.find(key);

	if(iterMap != m_mapRemotePicList.end())
	{
		list<unsigned char * >::iterator iterList = iterMap->second.begin();
		unsigned char * one   = new unsigned char[7*1024*1024];
		unsigned char * two   = new unsigned char[7*1024*1024];
		unsigned char * three = new unsigned char[7*1024*1024];
		unsigned char * four  = new unsigned char[7*1024*1024];

		IplImage * cximgFist   = NULL;
		IplImage * cximgSecond = NULL;
		IplImage * cximgThird  = NULL;
		IplImage * cximgFourth = NULL;
		if(m_nWordPos == 0)
		{
			cximgFist   = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height), 8, 3);
			cximgSecond = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height), 8, 3);
			cximgThird  = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height), 8, 3);
			cximgFourth = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height), 8, 3);
		}
		else
		{
			cximgFist   = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
			cximgSecond = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
			cximgThird  = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
			cximgFourth = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
		}

		//使用前整体涂黑
		cvSet(m_imgComposeStopSnap,cvScalar(0,0,0));

		SRIP_DETECT_HEADER szHeader[4];

		int index = 0 ;
		for (; iterList != iterMap->second.end(); iterList++)
		{
			if (index == 0)
			{
				char szPicName[50] = {0};
				sprintf(szPicName,"%s",*iterList);
				LogNormal("picname1:%s \n",szPicName);
				FILE *pPicFile = fopen(szPicName,"r");
				int ii = fread(one,1,7*1024*1024,pPicFile);
				LogNormal("length = %d \n",ii);
				fclose(pPicFile);
				szHeader[0].uChannelID       = ((SRIP_DETECT_HEADER*)one)->uChannelID;
				szHeader[0].uDetectType      = ((SRIP_DETECT_HEADER*)one)->uDetectType;
				szHeader[0].uTimestamp       = ((SRIP_DETECT_HEADER*)one)->uTimestamp;
				szHeader[0].uTime64          = ((SRIP_DETECT_HEADER*)one)->uTime64;
				szHeader[0].uSeq             = ((SRIP_DETECT_HEADER*)one)->uSeq;
				szHeader[0].uWidth           = ((SRIP_DETECT_HEADER*)one)->uWidth;
				szHeader[0].uHeight          = ((SRIP_DETECT_HEADER*)one)->uHeight;
				szHeader[0].uRealTime        = ((SRIP_DETECT_HEADER*)one)->uRealTime;
				szHeader[0].uVideoId         = ((SRIP_DETECT_HEADER*)one)->uVideoId;
				szHeader[0].dFrameRate       = ((SRIP_DETECT_HEADER*)one)->dFrameRate;
				szHeader[0].uImageRegionType = ((SRIP_DETECT_HEADER*)one)->uImageRegionType;
				szHeader[0].uTrafficSignal   = ((SRIP_DETECT_HEADER*)one)->uTrafficSignal;

				if(g_ytControlSetting.nPicComPoseMode == 3)//1近3远模式下事件发生时间取第一张图时间
				{
					event.uEventBeginTime = szHeader[0].uTimestamp;
					event.uMiEventBeginTime = (szHeader[0].uTime64/1000)%1000;
				}
			}	
			else if (index == 1)
			{				
				char szPicName[50] = {0};
				sprintf(szPicName,"%s",*iterList);
				LogNormal("picname2:%s \n",szPicName);
				FILE *pPicFile = fopen(szPicName,"r");
				int ii = fread(two,1,7*1024*1024,pPicFile);
				LogNormal("length = %d \n",ii);
				fclose(pPicFile);
				szHeader[1].uChannelID       = ((SRIP_DETECT_HEADER*)two)->uChannelID;
				szHeader[1].uDetectType      = ((SRIP_DETECT_HEADER*)two)->uDetectType;
				szHeader[1].uTimestamp       = ((SRIP_DETECT_HEADER*)two)->uTimestamp;
				szHeader[1].uTime64          = ((SRIP_DETECT_HEADER*)two)->uTime64;
				szHeader[1].uSeq             = ((SRIP_DETECT_HEADER*)two)->uSeq;
				szHeader[1].uWidth           = ((SRIP_DETECT_HEADER*)two)->uWidth;
				szHeader[1].uHeight          = ((SRIP_DETECT_HEADER*)two)->uHeight;
				szHeader[1].uRealTime        = ((SRIP_DETECT_HEADER*)two)->uRealTime;
				szHeader[1].uVideoId         = ((SRIP_DETECT_HEADER*)two)->uVideoId;
				szHeader[1].dFrameRate       = ((SRIP_DETECT_HEADER*)two)->dFrameRate;
				szHeader[1].uImageRegionType = ((SRIP_DETECT_HEADER*)two)->uImageRegionType;
				szHeader[1].uTrafficSignal   = ((SRIP_DETECT_HEADER*)two)->uTrafficSignal;
			}
			else if (index == 2)
			{				
				char szPicName[50] = {0};
				sprintf(szPicName,"%s",*iterList);
				LogNormal("picname3:%s \n",szPicName);
				FILE *pPicFile = fopen(szPicName,"r");
				int ii = fread(three,1,7*1024*1024,pPicFile);
				LogNormal("length = %d \n",ii);
				fclose(pPicFile);

				szHeader[2].uChannelID       = ((SRIP_DETECT_HEADER*)three)->uChannelID;
				szHeader[2].uDetectType      = ((SRIP_DETECT_HEADER*)three)->uDetectType;
				szHeader[2].uTimestamp       = ((SRIP_DETECT_HEADER*)three)->uTimestamp;
				szHeader[2].uTime64          = ((SRIP_DETECT_HEADER*)three)->uTime64;
				szHeader[2].uSeq             = ((SRIP_DETECT_HEADER*)three)->uSeq;
				szHeader[2].uWidth           = ((SRIP_DETECT_HEADER*)three)->uWidth;
				szHeader[2].uHeight          = ((SRIP_DETECT_HEADER*)three)->uHeight;
				szHeader[2].uRealTime        = ((SRIP_DETECT_HEADER*)three)->uRealTime;
				szHeader[2].uVideoId         = ((SRIP_DETECT_HEADER*)three)->uVideoId;
				szHeader[2].dFrameRate       = ((SRIP_DETECT_HEADER*)three)->dFrameRate;
				szHeader[2].uImageRegionType = ((SRIP_DETECT_HEADER*)three)->uImageRegionType;
				szHeader[2].uTrafficSignal   = ((SRIP_DETECT_HEADER*)three)->uTrafficSignal;
			}
			else 
			{				
				char szPicName[50] = {0};
				sprintf(szPicName,"%s",*iterList);
				LogNormal("picname4:%s \n",szPicName);
				FILE *pPicFile = fopen(szPicName,"r");
				int ii = fread(four,1,7*1024*1024,pPicFile);
				LogNormal("length = %d \n",ii);
				fclose(pPicFile);

				szHeader[3].uChannelID       = ((SRIP_DETECT_HEADER*)four)->uChannelID;
				szHeader[3].uDetectType      = ((SRIP_DETECT_HEADER*)four)->uDetectType;
				szHeader[3].uTimestamp       = ((SRIP_DETECT_HEADER*)four)->uTimestamp;
				szHeader[3].uTime64          = ((SRIP_DETECT_HEADER*)four)->uTime64;
				szHeader[3].uSeq             = ((SRIP_DETECT_HEADER*)four)->uSeq;
				szHeader[3].uWidth           = ((SRIP_DETECT_HEADER*)four)->uWidth;
				szHeader[3].uHeight          = ((SRIP_DETECT_HEADER*)four)->uHeight;
				szHeader[3].uRealTime        = ((SRIP_DETECT_HEADER*)four)->uRealTime;
				szHeader[3].uVideoId         = ((SRIP_DETECT_HEADER*)four)->uVideoId;
				szHeader[3].dFrameRate       = ((SRIP_DETECT_HEADER*)four)->dFrameRate;
				szHeader[3].uImageRegionType = ((SRIP_DETECT_HEADER*)four)->uImageRegionType;
				szHeader[3].uTrafficSignal   = ((SRIP_DETECT_HEADER*)four)->uTrafficSignal;
			}
			index++;		
		}
		//LogNormal("----合成图片");
		///////////////////////////////去头
		cvSetData(cximgFist, one+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);
		cvSetData(cximgSecond, two+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);
		cvSetData(cximgThird, three+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);
		cvSetData(cximgFourth, four+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);

		for(int i=0;i<4;i++)
		{
			if(i == 0)
			{
				cvSetImageROI(cximgFist, cvRect(0, (m_imgSnap->height-m_nExtent)/2, m_imgSnap->width, ((m_imgSnap->height-m_nExtent))/2) ); 
				CvScalar val = cvAvg(cximgFist); 
				m_dFirstAvgVal = val.val[0]; 
				cvResetImageROI(cximgFist);
			}
			else if(i == 1)
			{
				cvSetImageROI(cximgSecond, cvRect(0, (m_imgSnap->height-m_nExtent)/2, m_imgSnap->width, ((m_imgSnap->height-m_nExtent))/2) ); 
				CvScalar val = cvAvg(cximgSecond); 
				m_dSecondAvgVal = val.val[0]; 
				cvResetImageROI(cximgSecond);
			}
			else if(i == 2)
			{
				cvSetImageROI(cximgThird, cvRect(0, (m_imgSnap->height-m_nExtent)/2, m_imgSnap->width, ((m_imgSnap->height-m_nExtent))/2) ); 
				CvScalar val = cvAvg(cximgThird); 
				m_dThirdAvgVal = val.val[0]; 
				cvResetImageROI(cximgThird);
			}
			else
			{
				cvSetImageROI(cximgFourth, cvRect(0, (m_imgSnap->height-m_nExtent)/2, m_imgSnap->width, ((m_imgSnap->height-m_nExtent))/2) ); 
				CvScalar val = cvAvg(cximgFourth); 
				m_dFourAvgVal = val.val[0]; 
				cvResetImageROI(cximgFourth);
			}


		}

		for(int nIndex= 0; nIndex<4; nIndex++)
		{
			CvRect rect;
			if(m_nWordPos == 0)
			{
				rect.x = (nIndex%2)*m_imgSnap->width;
				rect.y = (nIndex/2)*m_imgSnap->height;
				rect.width = m_imgSnap->width;
				rect.height = m_imgSnap->height;
			}
			else
			{
				rect.x = (nIndex%2)*m_imgSnap->width;
				rect.y = (nIndex/2)*(m_imgSnap->height-m_nExtent);
				rect.width = m_imgSnap->width;
				rect.height = m_imgSnap->height-m_nExtent;
			}


			cvSetImageROI(m_imgComposeStopSnap,rect);
			if (g_ytControlSetting.nPicComPoseMode == 1)
			{
				if(nIndex == 0)
				{
					cvCopy(cximgFist, m_imgComposeStopSnap);
				}
				else if (nIndex == 1)
				{
					cvCopy(cximgSecond, m_imgComposeStopSnap);
				}
				else if (nIndex == 2)
				{
					cvCopy(cximgThird, m_imgComposeStopSnap);
				}
				else if (nIndex == 3)
				{
					cvCopy(cximgFourth, m_imgComposeStopSnap);      
				}
			}
			else if(g_ytControlSetting.nPicComPoseMode == 0)
			{
				if(nIndex == 0)
				{
					cvCopy(cximgFist, m_imgComposeStopSnap);
				}
				else if (nIndex == 1)
				{
					cvCopy(cximgSecond, m_imgComposeStopSnap);
				}
				else if (nIndex == 2)
				{
					cvCopy(cximgThird, m_imgComposeStopSnap);
				}
				else if (nIndex == 3)
				{
					cvCopy(cximgFourth, m_imgComposeStopSnap);
				}
			}
			else if(g_ytControlSetting.nPicComPoseMode == 3)
			{
				if(nIndex == 0)
				{
					cvCopy(cximgFourth, m_imgComposeStopSnap);
				}
				else if (nIndex == 1)
				{
					cvCopy(cximgFist, m_imgComposeStopSnap);
				}
				else if (nIndex == 2)
				{
					cvCopy(cximgSecond, m_imgComposeStopSnap);
				}
				else if (nIndex == 3)
				{
					cvCopy(cximgThird, m_imgComposeStopSnap);
				}
			}
			
			cvResetImageROI(m_imgComposeStopSnap);
		}
		
		// 往图片上增加字
		ImageAddCharacter(m_imgComposeStopSnap, event,key,szHeader);


		// 释放
		if(cximgFist)
		{
			cvReleaseImageHeader(&cximgFist);
		}
		if(cximgSecond)
		{
			cvReleaseImageHeader(&cximgSecond);
		}
		if(cximgThird)
		{
			cvReleaseImageHeader(&cximgThird);
		}
		if(cximgFourth)
		{
			cvReleaseImageHeader(&cximgFourth);
		}

		delete []one;
		one = NULL;
		delete []two;
		two = NULL;
		delete []three;
		three = NULL;
		delete []four;
		four = NULL;

		
		// 释放内存  从map 删除这四张图
		
		iterList = iterMap->second.begin();
		for (; iterList != iterMap->second.end(); iterList++)
		{
			if(*iterList != NULL)
			{
				char szPicName[50] = {0};
				sprintf(szPicName,"%s",*iterList);
				LogNormal("Del picname:%s \n",szPicName);
				
				char szbody[512] = {0};
				sprintf(szbody,"rm -rf %s",szPicName);
				system(szbody);
			
				delete (*iterList);	
				*iterList = NULL;
			}
			
		}
		iterMap->second.clear();
		m_mapRemotePicList.erase(iterMap);
		
	}
	//LogNormal("-----ComPoseImage end\n");
	//LogNormal("save  buff image  ---ok  \n");
	return 1;
}


//参数设置,设置检测参数[车道信息、检测线、检测值]
bool CSkpRoadDetect::SetDetectParam(paraDetectList& sFrameIn)
{
    m_sParamIn.clear();

    m_sParamIn = sFrameIn;

    paraDetectList::iterator it_b = m_sParamIn.begin();
    paraDetectList::iterator it_e = m_sParamIn.end();
    while(it_b!=it_e)
    {
        it_b->m_nStatFluxTime = m_nTrafficStatTime;
        it_b++;
    }

    return true;
}

//添加事件录象缓冲数据
bool CSkpRoadDetect::AddVideoFrame(std::string& frame)
{
	{
       if(m_bEventCapture)
        m_skpRoadRecorder.AddFrame(frame);
    }

    return true;
}

//修改事件录像
void CSkpRoadDetect::ModifyEventCapture(bool bEventCapture)
{
    //启动事件录像线程
    if(m_bEventCapture)
    {
        m_skpRoadRecorder.UnInit();
    }

    m_bEventCapture = bEventCapture;

    if(m_bEventCapture)
    {
        m_skpRoadRecorder.Init();
    }
}

//初始化车道配置
bool CSkpRoadDetect::InitDetectConfig()
{
    // 检测算法初始化
#ifndef NOEVENT
    if(m_Rdetect.mvinit("",m_fScaleX,m_fScaleY,m_nDeinterlace))
#endif
    {
        if(!LoadRoadSettingInfo())
        {
            LogError("检测算法初始化配置失败!");
            return false;
        }
        return true;
    }
    return false;
}

//设置白天晚上还是自动判断
void CSkpRoadDetect::SetChannelDetectTime(CHANNEL_DETECT_TIME dType)
{
    if (dType == DETECT_AUTO)
    {
        m_nDetectTime = DETECT_AUTO;
#ifdef DEBUG_DETECT
        printf("===auto day and night\r\n");
#endif
    }
    else if (dType == DETECT_DAY)
    {
        m_nDetectTime = DETECT_DAY;
        m_bReloadConfig = true;
        m_nDayNight =1;
    }
    else
    {
        m_nDetectTime = DETECT_NIGHT;
        m_bReloadConfig = true;
        m_nDayNight =0;
    }
}

//获取事件类型
bool CSkpRoadDetect::GetEventType(DETECT_RESULT_TYPE& eType,int uType,int uRoadWayID)
{

    switch (eType)
    {
    case DETECT_RESULT_EVENT_STOP:
    {
        if(uType==1)
            eType = DETECT_RESULT_EVENT_PERSON_STOP;
    }
    return true;
    case DETECT_RESULT_EVENT_DERELICT:
    case DETECT_RESULT_EVENT_GO_SLOW:
    case DETECT_RESULT_EVENT_GO_FAST:
    case DETECT_RESULT_EVENT_JAM:
    case DETECT_RESULT_EVENT_GO_CHANGE:
    case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD: //小车出现在禁行车道
    case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD: //大车出现在禁行车道
    case DETECT_RESULT_BIG_IN_FORBIDDEN_TIME:
    case DETECT_RESULT_NO_PASSING:
    case DETECT_RESULT_EVENT_INSIDE:  //闯入越界事件直接由事件库报出
    case DETECT_RESULT_EVENT_OUTSIDE:
    case DETECT_RESULT_CROWD:
    case DETECT_RESULT_PERSON_RUN:
        return true;
    case DETECT_RESULT_EVENT_CROSS:
    {
        if(uType==1)
        {
            eType = DETECT_RESULT_EVENT_PASSERBY;
            return true;
        }
    }
    break;
    case DETECT_RESULT_EVENT_APPEAR://机动车
    {
        if(uType==1)
        {
            eType = DETECT_RESULT_EVENT_PERSON_APPEAR;
            return true;
        }
        else if(uType == 2)
        {
            eType = DETECT_RESULT_EVENT_WRONG_CHAN;//非机动车
            return true;
        }
    }
    break;
    case DETECT_RESULT_EVENT_GO_AGAINST:
    {
        if(uType==1)
            eType = DETECT_RESULT_EVENT_PERSON_AGAINST;
        else
            return true;
    }
    break;
    default:
        break;
    }

    paraDetectList::iterator it = m_sParamIn.begin();
    while(it!=m_sParamIn.end())
    {
        if(it->nChannelID==uRoadWayID)
        {
            if(eType==DETECT_RESULT_EVENT_CROSS) //车辆横穿需要判断,默认行人横穿 /
            {
                if(it->m_bCarCross)
                    return true;
            }
            else if(eType==DETECT_RESULT_EVENT_APPEAR)
            {
                if(it->m_bCarAppear)
                    return true;
            }
            else if(eType==DETECT_RESULT_EVENT_PERSON_AGAINST)
            {
                if(it->m_bPersonAgainst)
                    return true;
            }
            break;
        }
        it++;
    }

    return false;
}
// 函数的功能 往图片上增加字， 
void CSkpRoadDetect::ImageAddCharacter(IplImage * pImage, RECORD_EVENT event, int key,SRIP_DETECT_HEADER* pHeader)
{
	LogNormal("PicMode=%d,Extent=%d,WordPos=%d,CarNum=%d\n",g_ytControlSetting.nPicComPoseMode,m_nExtent,m_nWordPos,g_PicFormatInfo.nCarNum);
	wchar_t wchOut[255] = {'\0'};
	char chOut[255] = {'\0'};

	int nStartX = 0;    
	int nWidth = 10;
	int nHeight = 0;


	SRIP_DETECT_HEADER sDetectHeader1 = pHeader[0];
	SRIP_DETECT_HEADER sDetectHeader2 = pHeader[1];
	SRIP_DETECT_HEADER sDetectHeader3 = pHeader[2];
	SRIP_DETECT_HEADER sDetectHeader4 = pHeader[3];

#ifdef CAMERAAUTOCTRL
	if (!m_bEventCapture)
	{
		if(sDetectHeader4.uTimestamp == 0)
		{
			LogNormal("uTimestamp == 0\n");
			return;
		}
		m_dwEventTime = GetTimeT();
		m_EventVisVideo->AddEventVideoReq(sDetectHeader1.uTimestamp-60,sDetectHeader4.uTimestamp+60,g_strDetectorID.c_str(),event.uDirection,m_dwEventTime);
	}

#endif

	string strTime; 
	int nMiTime = 0;
	
	for (int nIndex = 0; nIndex <4; nIndex++)
	{
		nWidth = m_imgSnap->width*(nIndex%2);
		//nHeight =  m_imgSnap->height*(nIndex/2)+m_imgSnap->height - 17;
		char szBuf[9] = {'\0'};
		if(nIndex == 0)
		{	
			char szTmp[3] = {'\0'};
			char szAvgval[10] = {0};
			sprintf(szAvgval,"%d",(int)m_dFirstAvgVal);
			m_pMd5Ctx->MD5Update((unsigned char*)szAvgval,strlen(szAvgval));
			unsigned char szDigest[16] = {0};
			m_pMd5Ctx->MD5Final(szDigest);
			for(int i = 0;i<4;i++)
			{
				sprintf(szTmp,"%02x",szDigest[i]);
				strcat(szBuf,szTmp);
			}
			//LogNormal("Avgval[1]:%s \n",szBuf);
			
		}
		else if(nIndex == 1)
		{
			char szTmp[3] = {'\0'};
			char szAvgval[10] = {0};
			sprintf(szAvgval,"%d",(int)m_dSecondAvgVal);
			m_pMd5Ctx->MD5Update((unsigned char*)szAvgval,strlen(szAvgval));
			unsigned char szDigest[16] = {0};
			m_pMd5Ctx->MD5Final(szDigest);
			for(int i = 0;i<4;i++)
			{
				sprintf(szTmp,"%02x",szDigest[i]);
				strcat(szBuf,szTmp);
			}
			//LogNormal("Avgval[2]:%s \n",szBuf);
		}
		else if(nIndex == 2)
		{
			char szTmp[3] = {'\0'};
			char szAvgval[10] = {0};
			sprintf(szAvgval,"%d",(int)m_dThirdAvgVal);
			m_pMd5Ctx->MD5Update((unsigned char*)szAvgval,strlen(szAvgval));
			unsigned char szDigest[16] = {0};
			m_pMd5Ctx->MD5Final(szDigest);
			for(int i = 0;i<4;i++)
			{
				sprintf(szTmp,"%02x",szDigest[i]);
				strcat(szBuf,szTmp);
			}
			//LogNormal("Avgval[3]:%s \n",szBuf);
		}
		else
		{
			memset(szBuf,0,9);
			char szTmp[3] = {'\0'};
			char szAvgval[10] = {0};
			sprintf(szAvgval,"%d",(int)m_dFourAvgVal);
			m_pMd5Ctx->MD5Update((unsigned char*)szAvgval,strlen(szAvgval));
			unsigned char szDigest[16] = {0};
			m_pMd5Ctx->MD5Final(szDigest);
			for(int i = 0;i<4;i++)
			{
				sprintf(szTmp,"%02x",szDigest[i]);
				strcat(szBuf,szTmp);
			}
			//LogNormal("Avgval[4]:%s \n",szBuf);
		}
			
		if (g_ytControlSetting.nPicComPoseMode == 1)
		{
			if (nIndex == 0)
			{
				strTime = GetTime(sDetectHeader1.uTimestamp);
				nMiTime = (sDetectHeader1.uTime64/1000)%1000;
			}
			else if (nIndex == 1)
			{
				strTime = GetTime(sDetectHeader2.uTimestamp);
				nMiTime = (sDetectHeader2.uTime64/1000)%1000;
			}
			else if (nIndex == 2)
			{
				strTime = GetTime(sDetectHeader3.uTimestamp);
				nMiTime = (sDetectHeader3.uTime64/1000)%1000;
			}
			else if (nIndex == 3)
			{
				strTime = GetTime(sDetectHeader4.uTimestamp);	
				nMiTime = (sDetectHeader4.uTime64/1000)%1000;
			}
		}
		else if(g_ytControlSetting.nPicComPoseMode == 0)
		{
			if (nIndex == 0)
			{
				strTime = GetTime(sDetectHeader1.uTimestamp);
			}
			else if (nIndex == 1)
			{
				strTime = GetTime(sDetectHeader2.uTimestamp);
			}
			else if (nIndex == 2)
			{
				strTime = GetTime(sDetectHeader3.uTimestamp);

			}
			else if (nIndex == 3)
			{
				strTime = GetTime(sDetectHeader4.uTimestamp);	
			}

		}
		else if(g_ytControlSetting.nPicComPoseMode == 3)
		{
			if (nIndex == 0)
			{
				strTime = GetTime(sDetectHeader4.uTimestamp);
			}
			else if (nIndex == 1)
			{
				strTime = GetTime(sDetectHeader1.uTimestamp);
			}
			else if (nIndex == 2)
			{
				strTime = GetTime(sDetectHeader2.uTimestamp);

			}
			else if (nIndex == 3)
			{
				strTime = GetTime(sDetectHeader3.uTimestamp);	
			}
		}
			
		// 方向
		string strDirection = GetDirection(event.uDirection);

		/*sprintf(chOut,"地点:%s   车牌:%s   方向:%s   时间:%s.%03d", m_strLocation.c_str(),event.chText,strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);//
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));*/

		if (g_ytControlSetting.nPicComPoseMode == 1)
		{
			if(m_nExtent > 0)
			{
				if(m_nWordPos == 0)
				{
					if(strlen(event.chText) < 5)
					{
						sprintf(chOut, "设备编号:%s  "
							"路口名称:%s  "
							"方向:%s  "
							"车道号:%d "
							"车牌:%s  "
							"时间:%s.%03d  "
							"防伪码: %s",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str(),event.uRoadWayID,"        ",strTime.c_str(),event.uMiEventBeginTime,szBuf);
					}
					else
					{
						sprintf(chOut, "设备编号:%s  "
							"路口名称:%s  "
							"方向:%s  "
							"车道号:%d "
							"车牌:%s  "
							"时间:%s.%03d  "
							"防伪码: %s",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str(),event.uRoadWayID,event.chText,strTime.c_str(),event.uMiEventBeginTime,szBuf);
					}


					
					nHeight = m_imgSnap->height*(nIndex/2+1)-10;
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
				}
				else
				{
					nHeight = (m_imgSnap->height-m_nExtent)*(nIndex/2)+(m_nExtent/2);
					//LogNormal("---gao wei=%d,---图片高度为=%d\n", nHeight, (m_imgSnap->height-m_nExtent)*(nIndex/2));
					sprintf(chOut,"地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"方向:%s", strDirection.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					if(g_PicFormatInfo.nCarNum == 1 && nIndex == 3)
					{
						nHeight += (m_nExtent/2);
						sprintf(chOut,"车牌:%s", event.chText);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
					}
				}
			}
			else
			{
				printf("putText tianjin 2*2.........................\n");
				nWidth = m_imgSnap->width*(nIndex%2) + m_picFormatInfo.nOffsetX+40;
				nHeight = (m_imgSnap->height)*(nIndex/2)+ m_picFormatInfo.nOffsetY+40;
				sprintf(chOut, "抓拍时间:%s.%03d", strTime.c_str(),nMiTime);
				memset(wchOut,0,sizeof(wchOut))	;	
				UTF8ToUnicode(wchOut, chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

				nHeight += (50);
				sprintf(chOut,"路口名称:%s", m_strLocation.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				nHeight += (50);
				sprintf(chOut,"行驶方向:%s", strDirection.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				nHeight += (50);
				sprintf(chOut,"设备编号:%s", m_strDeviceId.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				nHeight += (50);
				sprintf(chOut,"防伪码:%s", szBuf);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
			}

		}
		else if(g_ytControlSetting.nPicComPoseMode == 0)
		{
			if(m_nExtent > 0)
			{
				if(m_nWordPos == 0)
				{
					if(g_PicFormatInfo.nCarNum == 0)
					{
							sprintf(chOut, "设备编号:%s  "
								"违章类型:%s  "
								"地点名称:%s  "
								"方向:%s  "
								"时间:%s.%03d",g_strDetectorID.c_str(),"禁止停车",m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);
					}
					else
					{
						if(strlen(event.chText) < 5)
						{
							sprintf(chOut, "设备编号:%s  "
								"违章类型:%s  "
								"地点名称:%s  "
								"车牌:%s  "
								"方向:%s  "
								"时间:%s.%03d",g_strDetectorID.c_str(),"禁止停车",m_strLocation.c_str(),"        ",strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);
						}
						else
						{
							sprintf(chOut, "设备编号:%s  "
								"违章类型:%s  "
								"地点名称:%s  "
								"车牌:%s  "
								"方向:%s  "
								"时间:%s.%03d",g_strDetectorID.c_str(),"禁止停车",m_strLocation.c_str(),event.chText,strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);
						}
					}
			
					nHeight = m_imgSnap->height*(nIndex/2+1)-10;
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
				}
				else
				{
					nHeight = (m_imgSnap->height-m_nExtent)*(nIndex/2)+(m_nExtent/2);
					//LogNormal("---gao wei=%d,---图片高度为=%d\n", nHeight, (m_imgSnap->height-m_nExtent)*(nIndex/2));
					sprintf(chOut,"地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"方向:%s", strDirection.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					if(g_PicFormatInfo.nCarNum == 1 && nIndex == 3)
					{
						nHeight += (m_nExtent/2);
						sprintf(chOut,"车牌:%s", event.chText);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
					}
				}
			}
			else
			{
				printf("putText luqiao 2*2.........................\n");
				nWidth = m_imgSnap->width*(nIndex%2) + m_picFormatInfo.nOffsetX+40;
				nHeight = (m_imgSnap->height)*(nIndex/2)+ m_picFormatInfo.nOffsetY+40;

				sprintf(chOut, "设备ID:%s", g_strDetectorID.c_str());
				memset(wchOut,0,sizeof(wchOut))	;	
				UTF8ToUnicode(wchOut, chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

				nHeight += (40);

				sprintf(chOut,"抓拍地点:%s", m_strLocation.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				nHeight += (40);
				sprintf(chOut,"抓拍时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					
			}

		}
		else if(g_ytControlSetting.nPicComPoseMode == 3)
		{
			LogNormal("ImageAddCharacter1\n");
			if(g_PicFormatInfo.nCarNum == 0)
			{
				LogNormal("ImageAddCharacter2\n");
				//在每张图片上叠加时间信息
				//if(nIndex > 0)
				{
					nWidth = (m_imgSnap->width)*(nIndex%2)+(m_imgSnap->width)/2;
					nHeight = (m_imgSnap->height)*(nIndex/2)+m_nExtent;
					LogNormal("nWidth:%d,nHeight:%d\n",nWidth,nHeight);
					sprintf(chOut,"%s",strTime.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
				}
				continue ;
			}

			if(m_nExtent > 0)
			{
				nHeight = (m_imgSnap->height-m_nExtent)*(nIndex/2)+(m_nExtent/2);
				//LogNormal("---gao wei=%d,---图片高度为=%d\n", nHeight, (m_imgSnap->height-m_nExtent)*(nIndex/2));
				sprintf(chOut,"地点:%s", m_strLocation.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

				nHeight += (m_nExtent/2);
				sprintf(chOut,"方向:%s", strDirection.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

				nHeight += (m_nExtent/2);
				sprintf(chOut,"时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

				if(g_PicFormatInfo.nCarNum == 1 && nIndex == 0)
				{
					nHeight += (m_nExtent/2);
					sprintf(chOut,"车牌:%s", event.chText);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
				}
			}
			else
			{
				printf("putText luqiao 2*2.........................\n");
				nWidth = m_imgSnap->width*(nIndex%2) + m_picFormatInfo.nOffsetX+40;
				nHeight = (m_imgSnap->height)*(nIndex/2)+ m_picFormatInfo.nOffsetY+40;

				sprintf(chOut, "设备ID:%s", g_strDetectorID.c_str());
				memset(wchOut,0,sizeof(wchOut))	;	
				UTF8ToUnicode(wchOut, chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

				nHeight += (40);

				sprintf(chOut,"抓拍地点:%s", m_strLocation.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				nHeight += (40);
				sprintf(chOut,"抓拍时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			}

		}
	}


	
}


void CSkpRoadDetect::ImageAddCharacter(IplImage * pImage, RECORD_EVENT event, int key)
{
	LogNormal("PicMode=%d,Extent=%d,WordPos=%d,CarNum=%d\n",g_ytControlSetting.nPicComPoseMode,m_nExtent,m_nWordPos,g_PicFormatInfo.nCarNum);
	//LogNormal("add character   \n");
	wchar_t wchOut[255] = {'\0'};
	char chOut[255] = {'\0'};

	int nStartX = 0;    
	int nWidth = 10;
	int nHeight = 0;

	unsigned char * firstFrame = NULL, *secondFrame = NULL, *thirdFrame = NULL, *fourthFrame = NULL;
	std::map<int, list<unsigned char *> >::iterator iterMap = m_mapRemotePicList.find(key);
	
	int index = 0;
	if(iterMap != m_mapRemotePicList.end())
	{
		std::list<unsigned char * >::iterator iterList = iterMap->second.begin();
		for (; iterList != iterMap->second.end(); iterList++)
		{
			if (index == 0)
				firstFrame = *iterList;
			else if (index == 1)
				secondFrame = *iterList;
			else if (index == 2)
				thirdFrame = *iterList;
			else if (index == 3)
				fourthFrame = *iterList;
			index++;
		}

		SRIP_DETECT_HEADER sDetectHeader1 = *((SRIP_DETECT_HEADER*)firstFrame);
		SRIP_DETECT_HEADER sDetectHeader2 = *((SRIP_DETECT_HEADER*)secondFrame);
		SRIP_DETECT_HEADER sDetectHeader3 = *((SRIP_DETECT_HEADER*)thirdFrame);
		SRIP_DETECT_HEADER sDetectHeader4 = *((SRIP_DETECT_HEADER*)fourthFrame);

		string strTime; 	
	
		for (int nIndex = 0; nIndex <4; nIndex++)
		{
			nWidth = m_imgSnap->width*(nIndex%2);
			//nHeight =  m_imgSnap->height*(nIndex/2)+m_imgSnap->height - 17;
			
			if (g_ytControlSetting.nPicComPoseMode == 1)
			{
				if (nIndex == 0)
				{
					strTime = GetTime(sDetectHeader1.uTimestamp);
				}
				else if (nIndex == 1)
				{
					strTime = GetTime(sDetectHeader2.uTimestamp);
				}
				else if (nIndex == 2)
				{
					strTime = GetTime(sDetectHeader3.uTimestamp);

				}
				else if (nIndex == 3)
				{
					strTime = GetTime(sDetectHeader4.uTimestamp);	
				}
			}
			else if(g_ytControlSetting.nPicComPoseMode == 0)
			{
				if (nIndex == 0)
				{
					strTime = GetTime(sDetectHeader1.uTimestamp);
				}
				else if (nIndex == 1)
				{
					strTime = GetTime(sDetectHeader2.uTimestamp);
				}
				else if (nIndex == 2)
				{
					strTime = GetTime(sDetectHeader3.uTimestamp);

				}
				else if (nIndex == 3)
				{
					strTime = GetTime(sDetectHeader4.uTimestamp);	
				}

			}
			else if(g_ytControlSetting.nPicComPoseMode == 3)
			{
				if (nIndex == 0)
				{
					strTime = GetTime(sDetectHeader4.uTimestamp);
				}
				else if (nIndex == 1)
				{
					strTime = GetTime(sDetectHeader1.uTimestamp);
				}
				else if (nIndex == 2)
				{
					strTime = GetTime(sDetectHeader2.uTimestamp);

				}
				else if (nIndex == 3)
				{
					strTime = GetTime(sDetectHeader3.uTimestamp);	
				}
			}
			
			// 方向
			string strDirection = GetDirection(event.uDirection);

			/*sprintf(chOut,"地点:%s   车牌:%s   方向:%s   时间:%s.%03d", m_strLocation.c_str(),event.chText,strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);//
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));*/

			if (g_ytControlSetting.nPicComPoseMode == 1)
			{
				if(m_nExtent > 0) 
				{
					nHeight = m_imgSnap->height*(nIndex/2)+m_imgSnap->height - 17 - (m_nExtent/2) ;
					//LogNormal("index = %d\n", nIndex);
					if (nIndex == 2 || nIndex == 3)
					{
						nHeight -= m_nExtent;
					}
					if(g_PicFormatInfo.nCarNum == 1 && nIndex == 0)
					{

						sprintf(chOut,"车牌:%s", event.chText);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
					}

					nHeight -= (m_nExtent/2);
					sprintf(chOut,"时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight -= (m_nExtent/2);
					sprintf(chOut,"方向:%s", strDirection.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight -= (m_nExtent/2);
					//LogNormal("---gao wei=%d,---图片高度为=%d\n", nHeight, (m_imgSnap->height-m_nExtent)*(nIndex/2));
					sprintf(chOut,"地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut))	;	
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight -= (m_nExtent/2);
					sprintf(chOut, "设备编号:%s", g_strDetectorID.c_str());
					memset(wchOut,0,sizeof(wchOut))	;	
					UTF8ToUnicode(wchOut, chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0, 0, 255));
				}
				else  //文字叠加在图片上
				{
					printf("putText luqiao 2*2.........................\n");
					nWidth = m_imgSnap->width*(nIndex%2) + m_picFormatInfo.nOffsetX+40;
					nHeight = (m_imgSnap->height)*(nIndex/2)+ m_picFormatInfo.nOffsetY+40;

					sprintf(chOut, "设备ID:%s", g_strDetectorID.c_str());
					memset(wchOut,0,sizeof(wchOut))	;	
					UTF8ToUnicode(wchOut, chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

					nHeight += (40);

					sprintf(chOut,"抓拍地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					nHeight += (40);
					sprintf(chOut,"抓拍时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

// 					nHeight += (40);
// 					sprintf(chOut,"抓拍方向:%s", strDirection.c_str());
// 					memset(wchOut,0,sizeof(wchOut));
// 					UTF8ToUnicode(wchOut,chOut);
// 					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

/*					nHeight += (40);
					sprintf(chOut,"抓拍车道:%d", event.uRoadWayID);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));*/
				}
				

			}
			else if(g_ytControlSetting.nPicComPoseMode == 0)
			{
				if(m_nExtent > 0)
				{
					nHeight = (m_imgSnap->height-m_nExtent)*(nIndex/2)+(m_nExtent/2);
					//LogNormal("---gao wei=%d,---图片高度为=%d\n", nHeight, (m_imgSnap->height-m_nExtent)*(nIndex/2));
					sprintf(chOut,"地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"方向:%s", strDirection.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					if(g_PicFormatInfo.nCarNum == 1 && nIndex == 3)
					{
						nHeight += (m_nExtent/2);
						sprintf(chOut,"车牌:%s", event.chText);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
					}
				}
				else
				{
					printf("putText luqiao 2*2.........................\n");
					nWidth = m_imgSnap->width*(nIndex%2) + m_picFormatInfo.nOffsetX+40;
					nHeight = (m_imgSnap->height)*(nIndex/2)+ m_picFormatInfo.nOffsetY+40;

					sprintf(chOut, "设备ID:%s", g_strDetectorID.c_str());
					memset(wchOut,0,sizeof(wchOut))	;	
					UTF8ToUnicode(wchOut, chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

					nHeight += (40);

					sprintf(chOut,"抓拍地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					nHeight += (40);
					sprintf(chOut,"抓拍时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					
				}

			}
			else if(g_ytControlSetting.nPicComPoseMode == 3)
			{
				if(m_nExtent > 0)
				{
					nHeight = (m_imgSnap->height-m_nExtent)*(nIndex/2)+(m_nExtent/2);
					//LogNormal("---gao wei=%d,---图片高度为=%d\n", nHeight, (m_imgSnap->height-m_nExtent)*(nIndex/2));
					sprintf(chOut,"地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"方向:%s", strDirection.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					nHeight += (m_nExtent/2);
					sprintf(chOut,"时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

					if(g_PicFormatInfo.nCarNum == 1 && nIndex == 0)
					{
						nHeight += (m_nExtent/2);
						sprintf(chOut,"车牌:%s", event.chText);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
					}
				}
				else
				{
					printf("putText luqiao 2*2.........................\n");
					nWidth = m_imgSnap->width*(nIndex%2) + m_picFormatInfo.nOffsetX+40;
					nHeight = (m_imgSnap->height)*(nIndex/2)+ m_picFormatInfo.nOffsetY+40;

					sprintf(chOut, "设备ID:%s", g_strDetectorID.c_str());
					memset(wchOut,0,sizeof(wchOut))	;	
					UTF8ToUnicode(wchOut, chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

					nHeight += (40);

					sprintf(chOut,"抓拍地点:%s", m_strLocation.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					nHeight += (40);
					sprintf(chOut,"抓拍时间:%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				}

			}
		}
	}


	
}

/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CSkpRoadDetect::PutTextOnComposeImage(IplImage* pImage,RECORD_EVENT event,SRIP_DETECT_HEADER* sAuxHeader)
{
	wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    int nStartX = 0;
    int nWidth = 10;
    int nHeight = 0;

	//深圳北环格式
	if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
	{
		std::string strTime;

		for(int nIndex = 0;nIndex<4;nIndex++)
		{
			nWidth = 400+m_imgSnap->width*(nIndex%2);
			nHeight =  g_PicFormatInfo.nFontSize+m_imgSnap->height*(nIndex/2);
			
			if(nIndex == 0)
			{
				strTime = GetTime(sAuxHeader->uTimestamp,0);
				sprintf(chOut,"%s:%03d",strTime.c_str(),(int)((sAuxHeader->uTime64)/1000)%1000);
			}
			else if(nIndex == 1)
			{
				strTime = GetTime(event.uTime2,0);
				sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiTime2);
			}
			else 
			{
				strTime = GetTime(event.uEventBeginTime,0);
				sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiEventBeginTime);
			}
			
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
		}
		
		nWidth = 40;
		nHeight = pImage->height - g_PicFormatInfo.nFontSize;
		std::string strDirection = GetDirection(event.uDirection);
		sprintf(chOut,"地点名称:%s  方向:%s  违法时间:%s:%03d",m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);//
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

		return;
	}
	else if(1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)//路桥模式
	{
		printf("putText luqiao 2*2.........................\n");
		std::string strTime;
		string strDirection = GetDirection(event.uDirection);
		{
			for(int nIndex = 0;nIndex<3;nIndex++)
			{
				nWidth = m_imgSnap->width*nIndex + m_picFormatInfo.nOffsetX+40;
				nHeight =  m_picFormatInfo.nOffsetY+40;

				sprintf(chOut, "设备ID:%s", g_strDetectorID.c_str());
				memset(wchOut,0,sizeof(wchOut))	;	
				UTF8ToUnicode(wchOut, chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

				nHeight += 40;

				sprintf(chOut,"抓拍地点:%s", m_strLocation.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

				nHeight += 40;
				if(nIndex == 0)
				{
					strTime = GetTime(event.uTime2,0);
					sprintf(chOut,"抓拍时间:%s:%03d",strTime.c_str(),event.uMiTime2);
				}
				else if(nIndex == 1)
				{
					strTime = GetTime(event.uEventBeginTime,0);
					sprintf(chOut,"抓拍时间:%s:%03d",strTime.c_str(),event.uMiEventBeginTime);
				}
				else if(nIndex == 2)
				{
					strTime = GetTime(event.uEventBeginTime,0);
					sprintf(chOut,"抓拍时间:%s:%03d",strTime.c_str(),event.uMiEventBeginTime);
				}
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

				/*nHeight += 40;
				sprintf(chOut,"抓拍方向:%s", strDirection.c_str());
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));

				nHeight += 40;
				sprintf(chOut,"抓拍车道:%d", event.uRoadWayID);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255, 255, 255));*/
			}
		}

		return;
	}

	if(m_nExtent <= 0)
	{
		return;
	}

    
    if( m_nWordPos== 0)
    {
        nHeight = m_imgSnap->height - m_nExtent;
    }

    nStartX = nWidth;


    //设备编号
    std::string strDirection = GetDirection(event.uDirection);
    std::string strTime = GetTime(event.uEventBeginTime,0);
    sprintf(chOut,"设备编号:%s    地点名称:%s    方向:%s    时间:%s.%03d",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);//
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    nHeight += 90;
    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));


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
    sprintf(chOut,"%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));


    //第二时间
    if(m_imgSnap->width > 2000)
    nWidth = 3200;
    else
    nWidth = 2000;
    std::string strTime2 = GetTime(event.uTime2,0);
    sprintf(chOut,"%s.%03d",strTime2.c_str(),event.uMiTime2);
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

    //小图时间
    if(m_imgSnap->width > 2000)
    nWidth = 5400;
    else
    nWidth = 3400;
    sprintf(chOut,"%s.%03d",strTime.c_str(),event.uMiEventBeginTime);
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

   // m_cvText.UnInit();
}

/* 函数介绍：在事件结果图像上叠加文本信息（日期、地点等）
 * 输入参数：uTimestamp-时间戳
 * 输出参数：无
 * 返回值：无
 */
void CSkpRoadDetect::PutTextOnImage(IplImage* pImg,RECORD_EVENT event,SRIP_DETECT_HEADER* sPreHeader,int nIndex)
{

	if(m_nExtent <= 0)
	{
		return;
	}
    wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    //判断卡口行为还是违章行为
    //int nType = (event.uCode != DETECT_RESULT_EVENT_APPEAR);
    int nStartX = 0;
    int nWidth = 10;
    int nHeight = 0;

    if(pImg->width > m_imgSnap->width)
    {
        nWidth += (m_imgSnap->width)*(nIndex%2);
        if(m_nWordPos == 0)
        nHeight = (m_imgSnap->height)*(nIndex/2+1) - m_nExtent;
        else
        nHeight = (m_imgSnap->height)*(nIndex/2);
    }
    else
    {
        if(m_nWordPos == 0)
        nHeight = m_imgSnap->height-m_nExtent;
        else
        nHeight = 0;
    }
    nStartX = nWidth;
    //设备编号
    std::string strDirection = GetDirection(event.uDirection);
	if(g_ytControlSetting.nNeedControl == 0)
    sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str());//
	else
	sprintf(chOut,"设备编号:%s  地点名称:%s  ",g_strDetectorID.c_str(),m_strLocation.c_str());//
    string strText(chOut);



    //车道编号
    if(g_PicFormatInfo.nRoadIndex == 1)
    {

        sprintf(chOut,"车道编号:%d",event.uRoadWayID);//
        string strTmp(chOut);
        strText += strTmp;
    }

    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,(char*)strText.c_str());
    if(pImg->width < 1000)
    {
        nHeight += 20;
    }
    else
    {
        {
            nHeight += (g_PicFormatInfo.nExtentHeight/2);
        }
    }
    m_cvText.putText(pImg, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

    //////////////////////////////////第二行
    strText.clear();
    //经过时间
    std::string strTime;

    if(nIndex == 0)
    {
        strTime = GetTime(event.uEventBeginTime,0);
        sprintf(chOut,"时间:%s.%03d  ",strTime.c_str(),event.uMiEventBeginTime);
        string strTmp(chOut);
        strText += strTmp;
    }
    else
    {
        UINT32 uPreEventTime = sPreHeader->uTimestamp;
        strTime = GetTime(uPreEventTime,0);
        sprintf(chOut,"时间:%s.%03d  ",strTime.c_str(),(int)((sPreHeader->uTime64)/1000)%1000);
        string strTmp(chOut);
        strText += strTmp;
    }

    //行驶速度
    if(g_PicFormatInfo.nCarSpeed == 1)
    {
        sprintf(chOut,"速度:%dkm/h  ",event.uSpeed);
        string strTmp(chOut);
        strText += strTmp;
    }

    //车身颜色
    if(g_PicFormatInfo.nCarColor == 1)
    {
        std::string strCarColor = GetObjectColor(event.uColor1);
        std::string strCarColor2 = GetObjectColor(event.uColor2);

        if(event.uColor2 == UNKNOWN)
        {
            sprintf(chOut,"颜色:%s  ",strCarColor.c_str());
        }
        else
        {
            sprintf(chOut,"颜色:%s,%s  ",strCarColor.c_str(),strCarColor2.c_str());
        }
        string strTmp(chOut);
        strText += strTmp;
    }


    //违章类型
    if(g_PicFormatInfo.nViolationType == 1)
    {
        //if(nType !=0 )
        {
            std::string strEvent;
            if(event.uCode == DETECT_RESULT_EVENT_STOP||
               event.uCode == DETECT_RESULT_EVENT_PERSON_STOP)
            {
                strEvent = "停止行驶";
            }
            else if( (event.uCode == DETECT_RESULT_EVENT_GO_AGAINST)||
                     (event.uCode == DETECT_RESULT_EVENT_PERSON_AGAINST))
            {
                strEvent = "逆行";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_DERELICT)
            {
                strEvent = "遗弃物";
            }
            else if( (event.uCode == DETECT_RESULT_EVENT_PASSERBY)||
                     (event.uCode == DETECT_RESULT_EVENT_CROSS))
            {
                strEvent = "横穿";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_GO_SLOW)
            {
                strEvent = "行驶缓慢";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_GO_FAST)
            {
                strEvent = "行驶超速";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_JAM)
            {
                strEvent = "交通拥堵";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_GO_CHANGE)
            {
                strEvent = "违章变道";
            }
            else if(event.uCode == DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD)
            {
                strEvent = "禁行小车";
            }
            else if(event.uCode == DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD)
            {
                strEvent = "禁行大车";
            }
            else if(event.uCode == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME)
            {
                strEvent = "大货禁行";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_INSIDE)
            {
                strEvent = "闯入";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_OUTSIDE)
            {
                strEvent = "越界";
            }
            else if(event.uCode == DETECT_RESULT_CROWD)
            {
                strEvent = "人群聚集";
            }
            else if(event.uCode == DETECT_RESULT_PERSON_RUN)
            {
                strEvent = "行人奔跑";
            }
			else if(event.uCode == DETECT_RESULT_EVENT_HOLD_BANNERS)
            {
                strEvent = "拉横幅";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_DISTRIBUTE_LEAFLET)
            {
                strEvent = "散传单";
            }
			else if(event.uCode == DETECT_RESULT_PRESS_LINE)
            {
                strEvent = "压线";
            }
            sprintf(chOut,"违章类型:%s",strEvent.c_str());
             string strTmp(chOut);
            strText += strTmp;
        }
    }

    nWidth = 10;
    if(pImg->width > m_imgSnap->width)
    {
        nWidth += (m_imgSnap->width)*(nIndex%2);
    }
    nStartX = nWidth;

	if(pImg->width < 1000)
    {
        nHeight += 20;
    }
    else
    {
      nHeight += (g_PicFormatInfo.nExtentHeight/2);
    }
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,(char*)strText.c_str());

    m_cvText.putText(pImg, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
}

//设置检测类型
void CSkpRoadDetect::SetDetectKind(CHANNEL_DETECT_KIND nDetectKind)
{
    if(m_nDetectKind != nDetectKind)
    {
        m_nDetectKind = nDetectKind;
        m_bReloadConfig = true;
    }
}


//保存车牌数据&
void CSkpRoadDetect::SavePlate(SRIP_DETECT_OUT_RESULT result,SRIP_DETECT_HEADER sDetectHeader, const RECORD_EVENT& event, const TimePlate& tp)
{
    CARNUM_CONTEXT_DEF carnum_info = result.carNumStruct;

	char chCarNum[8]={0};
	memcpy(chCarNum, carnum_info.carnum, 7);
    std::string strCarNum(chCarNum);
    CarNumConvert(strCarNum,carnum_info.wjcarnum);
	/*char temp[20]={0};
	memcpy(temp, carnum_info.carnum, 7);
	LogTrace("plateEvent.log", "EventId:%ld, orgNum: %s, carNum:%s, bHaveCarNum:%d, bShow:%d, Time:%s", result.nEventId, temp, strCarNum.c_str(), result.bHaveCarnum?1:0, result.bShow?1:0, GetTime(event.uEventBeginTime).c_str());*/

    RECORD_PLATE plate;
    //车牌号码
    memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
    //车牌颜色
    plate.uColor = carnum_info.color;
    //车辆类型
	plate.uType = carnum_info.vehicle_type;
	CarTypeConvert(plate);

    //车牌结构
    plate.uPlateType = carnum_info.carnumrow;

    //车身颜色
    plate.uCarColor1 = 11;
    plate.uCarColor2 = 11;
    plate.uWeight1 = 100;
    plate.uWeight2 = 0;

    //车道和方向
    plate.uRoadWayID = result.nChannelIndex;
    plate.uDirection = m_nDirection;
    //地点
    memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());

    //车速
    plate.uSpeed = result.value;

    //发生时间
    plate.uTime = event.uEventBeginTime;
    plate.uMiTime = event.uMiEventBeginTime;

    /*string strPicPath("");
    char szBuff[256] = {0};

    sprintf(szBuff, "select PICWIDTH,PICHEIGHT,PICSIZE,PICPATH from TRAFFIC_EVENT_INFO where EVENT_ID=%lld", result.nEventId>0?result.nEventId:0);

    MysqlQuery q = g_skpDB.execQuery(string(szBuff));

    if (!q.eof())
    {
        plate.uPicWidth = q.getIntFileds(0);
        plate.uPicHeight = q.getIntFileds(1);
        plate.uPicSize = q.getIntFileds(2);
        strPicPath = q.getStringFileds(3);
    }
    q.finalize();*/
	plate.uPicWidth = event.uPicWidth;
	plate.uPicHeight = event.uPicHeight;
	plate.uPicSize = event.uPicSize;
    memcpy(plate.chPicPath, event.chPicPath, 128);
	memcpy(plate.chVideoPath, event.chVideoPath, 128);
	
	if (m_nDetectDirection == 0)//前排
	  plate.uPosLeft = tp.nLeft + 2*m_imgSnap->width;
	else
	 plate.uPosLeft = tp.nLeft;

	if( m_nWordPos== 1)//文字区域叠加在图片上方
	{
		plate.uPosTop = tp.nTop + m_nExtent;
	}
	else
	{
		plate.uPosTop = tp.nTop;
	}
    plate.uPosRight = plate.uPosLeft + tp.nWidth;
    plate.uPosBottom = plate.uPosTop + tp.nHeight;

	//LogTrace("event.log", "left=%d, top=%d, right=%d, bottom=%d carType:%d", plate.uPosLeft, plate.uPosTop, plate.uPosRight, plate.uPosBottom, plate.uType);
	//违章类型(需要进行类型转换)
	plate.uViolationType = result.eRtype;
	if(plate.uViolationType == DETECT_RESULT_EVENT_STOP||
		plate.uViolationType == DETECT_RESULT_EVENT_PERSON_STOP)
	{
		plate.uViolationType = DETECT_RESULT_PARKING_VIOLATION;
	}
	else if(plate.uViolationType == DETECT_RESULT_EVENT_GO_AGAINST||
		plate.uViolationType == DETECT_RESULT_EVENT_PERSON_AGAINST)
	{
		plate.uViolationType = DETECT_RESULT_RETROGRADE_MOTION;
	}
	else if(plate.uViolationType == DETECT_RESULT_EVENT_GO_CHANGE)
	{
		plate.uViolationType = DETECT_RESULT_ELE_EVT_BIANDAO;
	}
	//LogTrace("plateEvent.log", "strPicPath=%s, plateNumber=%s Time:%s", event.chPicPath, plate.chText, GetTimeCurrent().c_str());

    g_skpDB.SavePlate(m_nChannelID,plate,0,NULL);

    //发往客户端
    if(m_bConnect)
    {
        //车牌检测类型
		SRIP_DETECT_HEADER sHeader;
		sHeader = sDetectHeader;
        sHeader.uDetectType = SRIP_CARD_RESULT;
		sHeader.uTimestamp = event.uEventBeginTime;

        //将车牌信息送客户端
        std::string plateResult;
        //车牌号码
        memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

		RECORD_PLATE_CLIENT plate_client;
        memcpy(&plate_client,&plate,sizeof(RECORD_PLATE_CLIENT));
        plateResult.append((char*)&plate_client,sizeof(plate_client));

        plateResult.insert(0,(char*)&sHeader,sizeof(SRIP_DETECT_HEADER));
        g_skpChannelCenter.AddResult(plateResult);
    }
}

//载入通道设置
bool CSkpRoadDetect::LoadRoadSettingInfo()
{
    LIST_CHANNEL_INFO list_channel_info;
    CXmlParaUtil xml;
    if(xml.LoadRoadSettingInfo(list_channel_info,m_nChannelID))
    {
        bool bLoadCalibration = false;
        bool bLoadSkipArea = false;
        bool bStabBackArea = false;
		bool bLoadPerson = false;
		bool bLoadYelGridArea = false;

        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator it_begin;
        Point32fList::iterator it_end;

        Point32fList::iterator it_32fb;
        Point32fList::iterator it_32fe;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        CPoint pt;
        CPoint32f pt32f;

        VerChanList listVerChan;//车道列表
        RoadList listRoad; //道路列表
        RegionList listStabBack; //稳像区域
        RegionList listSkip; //屏蔽区域
        Calibration calib;//标定

		RegionList listYelGrid; //黄网格区域

		CvRect farrect;
		CvRect nearrect;

		int nPersonRectCount = 0;

		 float image_cord[12];
        float world_cord[12];

        m_roadMap.clear();
        printf("list_channel_info.size=%d\n",(int)list_channel_info.size());
        while(it_b != it_e)
        {
            int i = 0;
            int j = 0;
            printf("m_fScaleX=%f,m_fScaleY=%f,m_nDeinterlace=%d\n",m_fScaleX,m_fScaleY,m_nDeinterlace);
            CHANNEL_INFO channel_info = *it_b;
            VerChanStru StruVerChan;
            ROAD_INDEX_INFO road_index_info;
            //车道编号
            StruVerChan.nRoadIndex = channel_info.chProp_index.value.nValue;
            road_index_info.nVerRoadIndex = channel_info.chProp_name.value.nValue;
            //车道方向
            StruVerChan.nDirection = channel_info.chProp_direction.value.nValue;
            if(StruVerChan.nDirection < 180)
            {
                road_index_info.nDirection = 0;
            }
            else
            {
                road_index_info.nDirection = 1;
            }
            m_roadMap.insert(RoadIndexMap::value_type(StruVerChan.nRoadIndex,road_index_info));

            //车道方向中心点
            printf("StruVerChan.nRoadIndex=%d,StruVerChan.nDirection=%d\n",StruVerChan.nRoadIndex,StruVerChan.nDirection);
            printf("channel_info.chProp_direction.point.x=%f,channel_info.chProp_direction.point.y=%f\n",channel_info.chProp_direction.point.x,channel_info.chProp_direction.point.y);

            //标定区域
            if(!bLoadCalibration)
            {
                calib.length = channel_info.calibration.length;
                calib.width = channel_info.calibration.width;
                calib.cameraHeight = channel_info.calibration.cameraHeight;
                printf("calib.length=%f,calib.width=%f,calib.cameraHeight=%f\n",calib.length,calib.width,calib.cameraHeight);
                i = 0;
                //矩形区域（4个点）
                it_begin = channel_info.calibration.region.listPT.begin();
                it_end = channel_info.calibration.region.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    //image cor
                    pt.x = it_begin->x/m_ratio_x;
                    pt.y = it_begin->y/m_ratio_y;
                    calib.region.vList.push_back(pt);

					image_cord[2*i] = (it_begin->x);
                    image_cord[2*i+1] = (it_begin->y);

                    //world cor
                    if(i==0)
                    {
                        pt32f.x = 0;
                        pt32f.y = 0;
						world_cord[2*i] = 0;
                        world_cord[2*i+1] = 0;
                    }
                    else if(i==1)
                    {
                        pt32f.x = calib.length;
                        pt32f.y = 0;
						 world_cord[2*i] = calib.length;
                        world_cord[2*i+1] = 0;
                    }
                    else if(i==2)
                    {
                        pt32f.x = calib.length;
                        pt32f.y = calib.width;
						 world_cord[2*i] = calib.length;
                        world_cord[2*i+1] = calib.width;
                    }
                    else if(i==3)
                    {
                        pt32f.x = 0;
                        pt32f.y = calib.width;
						 world_cord[2*i] = 0;
                        world_cord[2*i+1] = calib.width;
                    }
                    calib.region.pt32fList.push_back(pt32f);
                    printf("calib pt.x=%d,pt.y=%d,pt32f.x=%f,pt32f.y=%f\n",pt.x,pt.y,pt32f.x,pt32f.y);
                    i++;
                }

                //辅助标定点
                it_begin = channel_info.calibration.listPT.begin();
                it_end = channel_info.calibration.listPT.end();

                it_32fb = channel_info.calibration.list32fPT.begin();
                it_32fe = channel_info.calibration.list32fPT.end();
                while(it_begin!=it_end&&it_32fb!=it_32fe)
                {
                    //image cor
                    pt.x = it_begin->x/m_ratio_x;
                    pt.y = it_begin->y/m_ratio_y;
                    calib.ptList.push_back(pt);

                    //world cor
                    pt32f.x = it_32fb->x;
                    pt32f.y = it_32fb->y;
                    calib.pt32fList.push_back(pt32f);

					  //image cor
                    image_cord[2*i] = (it_begin->x);
                    image_cord[2*i+1] = (it_begin->y);

                    //world cor
                    world_cord[2*i] = it_32fb->x;
                    world_cord[2*i+1] = it_32fb->y;

                    printf("calib pt.x=%d,pt.y=%d,pt32f.x=%f,pt32f.y=%f\n",pt.x,pt.y,pt32f.x,pt32f.y);


                    it_32fb++;
                    it_begin++;
					i++;
                }
                bLoadCalibration = true;
				 mvfind_homography(image_cord,world_cord,homography_image_to_world);
            }
            //屏蔽区域
            if(!bLoadSkipArea)
            {
                it_rb = channel_info.eliminateRegion.listRegionProp.begin();
                it_re = channel_info.eliminateRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    Rgn	region;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x)/m_ratio_x;
                        pt32f.y = (item_b->y)/m_ratio_y;
                        region.pt32fList.push_back(pt32f);
                        printf("eliminateRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    }
                    listSkip.push_back(region);
                }
                bLoadSkipArea = true;
            }
            //稳像背景区域
            if(!bStabBackArea)
            {
                it_rb = channel_info.StabBackRegion.listRegionProp.begin();
                it_re = channel_info.StabBackRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    Rgn	region;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x)/m_ratio_x;
                        pt32f.y = (item_b->y)/m_ratio_y;
                        region.pt32fList.push_back(pt32f);
                        printf("StabBackRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    }
                    listStabBack.push_back(region);
                }
                bStabBackArea = true;
            }

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

					if(it_rb->listPt.size() > 0)
					{
						nPersonRectCount++;
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

					if(it_rb->listPt.size() > 0)
					{
						nPersonRectCount++;
					}
                }

                bLoadPerson = true;
            }

            //道路区域
            RoadStru StruRoad;
            int nRoadIndex = channel_info.roadRegion.chProperty.value.nValue;
            it_begin = channel_info.roadRegion.listPT.begin();
            it_end = channel_info.roadRegion.listPT.end();
            for(; it_begin != it_end; it_begin++)
            {
				it_begin->x /= m_ratio_x;
				it_begin->y /= m_ratio_y;

                pt32f.x = it_begin->x;
                pt32f.y = it_begin->y;
                StruRoad.vListRoadWay2.push_back(pt32f);
                printf("nRoadIndex=%d,roadRegion pt32f.x=%f,pt32f.y=%f\n",nRoadIndex,pt32f.x,pt32f.y);
            }
            pt32f.x = channel_info.chProp_direction.point.x/m_ratio_x;
            pt32f.y = channel_info.chProp_direction.point.y/m_ratio_y;
            StruRoad.vListDirection2.push_back(pt32f);//道路方向中心点
            //需要判断道路区域是否已经存在
            bool bFindRoad = false;
            RoadList::iterator it = listRoad.begin();
            while(it!=listRoad.end())
            {
                CPoint32f ptCenter;
                GetCenterPoint(it->vListRoadWay2,ptCenter);

                CPoint32f ptCenterCur;
                GetCenterPoint(channel_info.roadRegion.listPT,ptCenterCur);

                double distance = sqrt((ptCenter.x-ptCenterCur.x)*(ptCenter.x-ptCenterCur.x)+(ptCenter.y-ptCenterCur.y)*(ptCenter.y-ptCenterCur.y));

                if( distance <= 5)
                {
                    bFindRoad = true;
                    StruVerChan.road_iter = it;
                    break;
                }
                it++;
            }
            if(!bFindRoad)
            {
                listRoad.push_back(StruRoad);
                StruVerChan.road_iter = --listRoad.end();
            }
            printf("listRoad.size()=%d\n",(int)listRoad.size());

            //车道区域
            it_begin = channel_info.chRegion.listPT.begin();
            it_end = channel_info.chRegion.listPT.end();
            for(; it_begin != it_end; it_begin++)
            {
                pt32f.x = it_begin->x/m_ratio_x;
                pt32f.y = it_begin->y/m_ratio_y;
                StruVerChan.vListChannel2.push_back(pt32f);

                if( (pt32f.x > m_img->width) || (pt32f.y > m_img->height))
                {
                    return false;
                }
                printf("chRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
            }
            //停车检测区域
            it_rb = channel_info.stopRegion.listRegionProp.begin();
            it_re = channel_info.stopRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                Rgn	region;
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("stopRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListPark.push_back(region);
            }
            //行人检测区域
            it_rb = channel_info.personRegion.listRegionProp.begin();
            it_re = channel_info.personRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                Rgn	region;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("personRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListPerson.push_back(region);
            }
            //遗弃物检测区域
            it_rb = channel_info.dropRegion.listRegionProp.begin();
            it_re = channel_info.dropRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                Rgn	region;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("dropRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListTrash.push_back(region);
            }
             //闯入区域
             it_rb = channel_info.BargeInRegion.listRegionProp.begin();
            it_re = channel_info.BargeInRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                Rgn	region;
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("BargeInRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListBargeIn.push_back(region);
            }

            //越界区域
            it_rb = channel_info.BeyondMarkRegion.listRegionProp.begin();
            it_re = channel_info.BeyondMarkRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                Rgn	region;
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("BeyondMarkRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListBeyondMark.push_back(region);
            }

            //流量监测线
            it_rb = channel_info.AmountLine.listRegionProp.begin();
            it_re = channel_info.AmountLine.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                Rgn	region;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("AmountLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
				//方向
				item_b = it_rb->directionListPt.begin();
                item_e = it_rb->directionListPt.end();
				for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.directionListPt.push_back(pt32f);
                    printf("AmountLine directionListPt pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListAmount.push_back(region);
            }
            //移动参考线
            it_rb = channel_info.RefLine.listRegionProp.begin();
            it_re = channel_info.RefLine.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                Rgn	region;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("RefLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListRef.push_back(region);
            }
            //变道线
            it_rb = channel_info.TurnRoadLine.listRegionProp.begin();
            it_re = channel_info.TurnRoadLine.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                Rgn	region;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("TurnRoadLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
				
				//方向
				item_b = it_rb->directionListPt.begin();
                item_e = it_rb->directionListPt.end();
				for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.directionListPt.push_back(pt32f);
                    printf("TurnRoadLine directionListPt pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }

                StruVerChan.vListTurnRoad.push_back(region);
            }
            //取景框
            it_rb = channel_info.FlowFramegetRegion.listRegionProp.begin();
            it_re = channel_info.FlowFramegetRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                Rgn	region;
                for( ; item_b!=item_e; item_b++)
                {
                    pt32f.x = (item_b->x)/m_ratio_x;
                    pt32f.y = (item_b->y)/m_ratio_y;
                    region.pt32fList.push_back(pt32f);
                    printf("FlowFramegetRegion pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                }
                StruVerChan.vListFlowFrameget.push_back(region);
            }

			//黄线区域
             it_rb = channel_info.YellowLine.listRegionProp.begin();
             it_re = channel_info.YellowLine.listRegionProp.end();
              for(; it_rb != it_re; it_rb++)
              {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    Rgn	region;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x)/m_ratio_x;
                        pt32f.y = (item_b->y)/m_ratio_y;
                        region.pt32fList.push_back(pt32f);
                        printf("YellowLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    }
                    StruVerChan.vListYellowLine.push_back(region);
              }

			  //导流线区域
             it_rb = channel_info.LeadStreamLine.listRegionProp.begin();
             it_re = channel_info.LeadStreamLine.listRegionProp.end();
              for(; it_rb != it_re; it_rb++)
              {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    Rgn	region;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt32f.x = (item_b->x)/m_ratio_x;
                        pt32f.y = (item_b->y)/m_ratio_y;
                        region.pt32fList.push_back(pt32f);
                        printf("LeadStreamLine pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
                    }
                    StruVerChan.vListLeadStreamLine.push_back(region);
              }

			  //黄网格区域
			  if(!bLoadYelGridArea)
			  {
				  it_rb = channel_info.YelGridRgn.listRegionProp.begin();
				  it_re = channel_info.YelGridRgn.listRegionProp.end();
				  for(; it_rb != it_re; it_rb++)
				  {
					  item_b = it_rb->listPt.begin();
					  item_e = it_rb->listPt.end();
					  Rgn	region;
					  for( ; item_b!=item_e; item_b++)
					  {
						  pt32f.x = (item_b->x)/m_ratio_x;
						  pt32f.y = (item_b->y)/m_ratio_y;
						  region.pt32fList.push_back(pt32f);
						  printf("YelGridRgn pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);
					  }
					  listYelGrid.push_back(region);
					  StruVerChan.vListYelGrid.push_back(region);
				  }
				  bLoadYelGridArea = true;
			  }

            //保存车道信息
            listVerChan.push_back(StruVerChan);

            it_b++;
        }

        //将参数传给检测库
        //设置道路区域
        //设置车道区域
        //设置标定
        //设置屏蔽区域
        //设置稳像区域
#ifndef NOEVENT
        m_Rdetect.mvSetChannelWaySetting(listVerChan,listStabBack,listSkip,calib,listRoad);

		if(nPersonRectCount > 0)
		{
			CvRect rectA[2];
			rectA[0] = farrect;
			rectA[1] = nearrect;
			m_Rdetect.mvGetPeoRectFromClient(nPersonRectCount,rectA);
		}
#endif

        return true;
    }
    else
    {
        printf("读取车道参数失败!\r\n");
        return false;
    }
}

//获取区域中心点
void CSkpRoadDetect::GetCenterPoint(Point32fList ptList,CPoint32f& ptCenter)
{
    if(ptList.size()>0)
    {
        Point32fList::iterator it_b = ptList.begin();
        Point32fList::iterator it_e = ptList.end();

        ptCenter.x = 0;
        ptCenter.y = 0;

        while(it_b != it_e)
        {
            CPoint32f point;

            point = *it_b;
            ptCenter.x += point.x;
            ptCenter.y += point.y;

            it_b++;
        }

        ptCenter.x = ptCenter.x/ptList.size();
        ptCenter.y = ptCenter.y/ptList.size();
    }
}

//获取是否有目标移动标志
bool CSkpRoadDetect::GetObjMovingState()
{
	if(m_nHasLocalPreSet == 1)
    {
        return m_PreSetControl.GetObjMovingState();
    }
    else
    {
        return false;
    }
}

//获取违章位置
void CSkpRoadDetect::GetViolationPos(CvPoint& point,CvRect& rect,int nType)
{
    CvRect rt;
    if(m_imgSnap->width > 2000)
    {
        rt.width = 800;
        rt.height = 800;
    }
    else
    {
        rt.width = 500;
        rt.height = 500;

		if(nType == 1)
		{
			rt.width = 600;
			rt.height = 600;
		}
    }


    int x = point.x - rt.width/2;
    int y = point.y - rt.height/2;

    if(x > 0)
    {
        rt.x = x;
    }
    else
    {
        rt.x = 0;
    }

    int nExtent = 0;
    if(m_nWordPos == 1)
        nExtent = m_nExtent;

    if(y > nExtent)
    {
        rt.y = y;
    }
    else
    {
        rt.y = nExtent;
    }

    if(rt.x+rt.width>=m_imgSnap->width)
    {
        rt.x = m_imgSnap->width - rt.width-1;
    }

    if(rt.y+rt.height>=m_imgSnap->height)
    {
        rt.y = m_imgSnap->height - rt.height-1;
    }

    rect = rt;
}

//设置相机编号
void CSkpRoadDetect::SetCameraID(int nCameraID)
{
    m_nCameraId = nCameraID;
    m_PreSetControl.SetCameraID(nCameraID);
}

//设置Monitor编号
void CSkpRoadDetect::SetMonitorID(int nMonitorID)
{
    m_nMonitorID = nMonitorID;
    m_PreSetControl.SetMonitorID(nMonitorID);
}

//设置相机类型
void CSkpRoadDetect::SetCameraType(int  nCameraType)
{
	m_nCameraType = nCameraType;
    m_PreSetControl.SetCameraType(nCameraType);
}


//从Jpg缓冲区查找出对应的图片
bool CSkpRoadDetect::GetJpgImageBySeq(UINT32 nSeq,PLATEPOSITION* pTimeStamp,string& strPic)
{
	yuv_video_buf* buf = NULL;

	//加锁
	pthread_mutex_lock(&m_JpgFrameMutex);

	if(m_JpgFrameMap.size() > 0)
	{

		map<int64_t,string>::iterator it_b = m_JpgFrameMap.begin();
		map<int64_t,string>::iterator it_e = --m_JpgFrameMap.end();
		UINT32 nMinSeq = it_b->first; //最小帧号
		UINT32 nMaxSeq = it_e->first;//最大帧号

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
		}
		else
		{
			// LogNormal("帧号不在队列中，取最临近帧号nSeq=%lld",nSeq);
			GetPicFromJpgMap(nSeq,0,strPic);
		}

	}
	//解锁
	pthread_mutex_unlock(&m_JpgFrameMutex);

	if(strPic.size() > 0)
	{
		buf = (yuv_video_buf*)strPic.c_str();

		if(buf)
		{
			pTimeStamp->ts = buf->ts;
			pTimeStamp->uTimestamp = buf->uTimestamp;
			pTimeStamp->uFieldSeq = buf->nFieldSeq;
		}

		strPic.erase(0,sizeof(yuv_video_buf));
	}

	return true;
}

//从Jpg缓冲区查找出对应的图片
bool CSkpRoadDetect::GetImageByJpgSeq(UINT32 nSeq,UINT32 nPreSeq,PLATEPOSITION* pTimeStamp,IplImage* pImage)
{
	yuv_video_buf* buf = NULL;

	string strPic;
	//加锁
	pthread_mutex_lock(&m_JpgFrameMutex);

	if(m_JpgFrameMap.size() > 0)
	{

		map<int64_t,string>::iterator it_b = m_JpgFrameMap.begin();
		map<int64_t,string>::iterator it_e = --m_JpgFrameMap.end();
		UINT32 nMinSeq = it_b->first; //最小帧号
		UINT32 nMaxSeq = it_e->first;//最大帧号

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
		}
		else
		{
			// LogNormal("帧号不在队列中，取最临近帧号nSeq=%lld",nSeq);
			GetPicFromJpgMap(nSeq,nPreSeq,strPic);
		}

	}
	//解锁
	pthread_mutex_unlock(&m_JpgFrameMutex);

	if(strPic.size() > 0)
	{
		buf = (yuv_video_buf*)strPic.c_str();

		if(buf)
		{
			pTimeStamp->ts = buf->ts;
			pTimeStamp->uTimestamp = buf->uTimestamp;
			pTimeStamp->uFieldSeq = buf->nFieldSeq;

			//需要解码jpg图像
			CxImage image;
			image.Decode((BYTE*)(strPic.c_str()+sizeof(yuv_video_buf)), strPic.size() - sizeof(yuv_video_buf), 3); //解码

			cvSet(pImage, cvScalar( 0,0, 0 ));
			if(image.IsValid()&&image.GetSize()>0)
			{
				if(m_nWordPos == 0)//黑边在下方
				{
					memcpy(pImage->imageData,image.GetBits(),image.GetSrcSize());
				}
				else//黑边在上方
				{
					memcpy(pImage->imageData+m_nExtent*pImage->widthStep,image.GetBits(),image.GetSrcSize());
				}
			}
		}
	}

	return true;
}

//从jpg map中寻找一个最接近的图片
void CSkpRoadDetect::GetPicFromJpgMap(UINT32 nSeq,UINT32 nPreSeq,string& strPic)
{
	bool bFind = false;

	map<int64_t,string>::iterator it = m_JpgFrameMap.begin();
	map<int64_t,string>::iterator it_pre;

	while( (it!=m_JpgFrameMap.end())&&(m_JpgFrameMap.size()>=2))
	{
		UINT32 nSeqCur = it->first;

		if(nSeq <= nSeqCur)
		{
			it_pre = it;

			if(it != m_JpgFrameMap.begin())
			{
				it_pre--;
			}

			UINT32 nSeqPre = it_pre->first;
			if(nSeq <= (nSeqCur+nSeqPre)*0.5)
			{
				strPic = it_pre->second;
			}
			else
			{
				strPic = it->second;
			}

				if(nPreSeq == nSeqCur)
				{
					strPic = it_pre->second;
				}
				else if(nPreSeq == nSeqPre)
				{
					strPic = it->second;
				}


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
		}
	}
}

//处理车牌事件的结果
void CSkpRoadDetect::OnProcessPlateEvent(string& frame, SRIP_DETECT_OUT_RESULT& sResult, RECORD_EVENT& event, SRIP_DETECT_HEADER &sHeader, UINT32 uRoadIndex, string& strPicPath, bool &flag)
{
	string strPath, strTmpPath, strCarNum;
	bool bAppear = false;

	//尾排不保存车牌图像
	if(sResult.eRtype == DETECT_RESULT_ALL)
	{
		return;
	}

	if (sResult.bSavePic && sResult.bHaveCarnum == false)//存图
	{
			int  nTimeInterval = 1;

			UINT32 uTimestamp = int(sResult.dEventTime);

			if(sResult.fLastImgTime >= 0)
			{
				nTimeInterval = (int)(sResult.fLastImgTime);
			}

			/*char chName[100]={0};
			sprintf(chName, "./bmp/%ld.bmp", sResult.nEventId);
			if (access("./bmp", F_OK) == -1)//bmp目录不存在
			{
				mkdir("./bmp", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH);
			}*/

			if(sResult.nEventId >= 0)
			{
				SavePlateEventPic(uTimestamp, nTimeInterval, frame,sResult.nEventId);
			}
			return;
	}

		//LogTrace("s_test.log", "save Eventid=%d bShow=%d", sResult.nEventId, sResult.bShow?1:0);
	//真实事件才写数据库
	if(!(sResult.bShow))
	{
		 if(sResult.eRtype == DETECT_RESULT_EVENT_APPEAR ||
                       sResult.eRtype == DETECT_RESULT_EVENT_PERSON_APPEAR ||
                       sResult.eRtype == DETECT_RESULT_EVENT_WRONG_CHAN)
        {
            bAppear = true;
        }

		if(sResult.eRtype == DETECT_RESULT_EVENT_STOP)
		{
			if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
			{
				if(m_nHasLocalPreSet == 1)
				{
					CvRect rtPos;
					rtPos.x = sResult.carRect.x * m_fScaleX;
					rtPos.width = sResult.carRect.width * m_fScaleX;
					rtPos.y = sResult.carRect.y * m_fScaleY;
					rtPos.height = sResult.carRect.height * m_fScaleY;

					//调用近景预置位
					m_PreSetControl.GoToLocalPreSet(rtPos,sHeader.uTimestamp);
				}
			}
		}

		//获取车牌号码
		CARNUM_CONTEXT_DEF carnum_info = sResult.carNumStruct;
		char chCarNum[8]={0};
		TimePlate tp;//获取车牌信息
		memcpy(chCarNum, carnum_info.carnum, 7);
		std::string strCarNum(chCarNum);
		CarNumConvert(strCarNum,carnum_info.wjcarnum);

		//查找5秒前的图片数据
		int  nTimeInterval = 500000;
		UINT32 uTime = 0, uMiTime = 0;//记录车牌时刻
		int64_t uTimestamp = int64_t(sResult.dEventTime)*1000000;
		SRIP_DETECT_HEADER sPreHeader;
		if(sResult.fLastImgTime >= 0)
		{
		//	nTimeInterval = (int)(sResult.fLastImgTime*1000000);
		}

		//LogNormal("dEventTime=%.6f,%lld.%d",sResult.dEventTime,sHeader.uTimestamp,(sHeader.uTime64)%1000000);
		bool bEventNow = false;
		if( ((int64_t)(sResult.dEventTime*1000000) == sHeader.uTime64) || (sResult.dEventTime < 0))//事件发生时关联上车牌或者是人(事件发生时间即为当前时间)
		{
			//LogNormal("+++++++++bHaveCarnum=%d,type=%d",sResult.bHaveCarnum,sResult.type);
			{
				//事件发生时刻图片取当前帧
				if(m_nDeinterlace==2)
				{
					cvResize(m_img,m_imgSnap);
				}
				else
				{
					cvCopy(m_img,m_imgSnap);
				}

				//事件发生上一时刻图片
				//LogNormal("0 sHeader.uSeq=%lld\n",sHeader.uSeq);
				sPreHeader = GetImageFromResultFrameList(uTimestamp, nTimeInterval, m_imgPreSnap,sHeader.uSeq);
				bEventNow = true;
				
			}
		}
		else//事件发生时未关联上车牌(事件发生时间不是当前时间)
		{
			//LogNormal("---------bHaveCarnum=%d,type=%d",sResult.bHaveCarnum,sResult.type);
			SRIP_DETECT_HEADER sAuxHeader;
			sPreHeader = FindPicFromDisk(sResult.nEventId, uTime, uMiTime,sAuxHeader);
			event.uEventBeginTime = uTime;
			event.uMiEventBeginTime = uMiTime;

			uTime = sAuxHeader.uTimestamp;
			uMiTime = ((sAuxHeader.uTime64)/1000)%1000;
		}

		//删除已经保存的事件
		pthread_mutex_lock(&m_EventPic_Mutex);
		map<long,string>::iterator it = m_EventPicMap.find(sResult.nEventId);
		if(it != m_EventPicMap.end())
		{
				m_EventPicMap.erase(it);
		}
		pthread_mutex_unlock(&m_EventPic_Mutex);

	    //车牌时刻图片
		if(FindPicFromMem(chCarNum, m_imgAuxSnap, &tp))
		{
			uTime = tp.uSec;
			uMiTime = tp.uMiSec;
		}
		else
		{
			//事件发生上一时刻图片
			if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
			{
				{
					if(bEventNow)
					{
						//LogNormal("1 sHeader.uSeq=%lld\n",sPreHeader.uSeq);
						SRIP_DETECT_HEADER sAuxHeader = GetImageFromResultFrameList(uTimestamp, nTimeInterval+500000, m_imgAuxSnap,sPreHeader.uSeq);
						uTime = sAuxHeader.uTimestamp;
						uMiTime = ((sAuxHeader.uTime64)/1000)%1000;
					}
				}
			}
			else
			{
				//取当前帧
				cvCopy(m_imgSnap,m_imgAuxSnap);
				uTime = event.uEventBeginTime;
				uMiTime = event.uMiEventBeginTime;
			}
		}

		event.uTime2 = sPreHeader.uTimestamp;
		event.uMiTime2 = ((sPreHeader.uTime64)/1000)%1000;
		event.uPicWidth = m_imgComposeSnap->width;  //事件快照宽度
		//事件快照高度
		event.uPicHeight = m_imgComposeSnap->height;

		//LogTrace("event.log", "event interval:%d, event time:%d, now:%ld", (sHeader.uTimestamp - event.uEventBeginTime), sResult.dEventTime, time(NULL));
		//深圳北环格式
		if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
		{
				for(int nIndex= 0; nIndex<4; nIndex++)
				{
							CvRect rect;
							rect.x = (nIndex%2)*m_imgSnap->width;
                            rect.y = (nIndex/2)*m_imgSnap->height;
                            rect.width = m_imgSnap->width;
                            rect.height = m_imgSnap->height;

							cvSetImageROI(m_imgComposeSnap,rect);

							//LogNormal("aux=%lld,pre=%d,cur=%d",auxtime,event.uTime2*1000+event.uMiTime2,event.uEventBeginTime*1000+event.uMiEventBeginTime);

							if(nIndex == 0)
							{
								if(uTime*1000+uMiTime <= event.uTime2*1000+event.uMiTime2)
								{
									cvCopy(m_imgAuxSnap,m_imgComposeSnap);
								}
								else if(uTime*1000+uMiTime <= event.uEventBeginTime*1000+event.uMiEventBeginTime)
								{
									cvCopy(m_imgPreSnap,m_imgComposeSnap);
								}
								else 
								{
									cvCopy(m_imgPreSnap,m_imgComposeSnap);
								}
							}
							else if(nIndex == 1)
							{
								if(uTime*1000+uMiTime <= event.uTime2*1000+event.uMiTime2)
								{
									cvCopy(m_imgPreSnap,m_imgComposeSnap);
								}
								else if(uTime*1000+uMiTime <= event.uEventBeginTime*1000+event.uMiEventBeginTime)
								{
									cvCopy(m_imgAuxSnap,m_imgComposeSnap);
								}
								else
								{
									cvCopy(m_imgSnap,m_imgComposeSnap);
								}
							}
							else if(nIndex == 2)
							{
								if(uTime*1000+uMiTime <= event.uTime2*1000+event.uMiTime2)
								{
									cvCopy(m_imgSnap,m_imgComposeSnap);
								}
								else if(uTime*1000+uMiTime <= event.uEventBeginTime*1000+event.uMiEventBeginTime)
								{
									cvCopy(m_imgSnap,m_imgComposeSnap);
								}
								else
								{
									cvCopy(m_imgAuxSnap,m_imgComposeSnap);
								}
							}
							else if(nIndex == 3)
							{
								CvPoint point;//事件坐标
                                point.x = event.uPosX;
                                point.y = event.uPosY;
                                CvRect rtPos;
                                GetViolationPos(point,rtPos);
										
								if(rtPos.height > 0)
								{
									//按目标区域比例裁剪
									rtPos.width = rtPos.height * rect.width/rect.height;

									if(rtPos.x + rtPos.width >= rect.width)
									{
										rtPos.x = rect.width - rtPos.width-1;
									}

									cvSetImageROI(m_imgSnap,rtPos);
									cvResize(m_imgSnap,m_imgComposeSnap);
									cvResetImageROI(m_imgSnap);
								}
							}
							cvResetImageROI(m_imgComposeSnap);
				}
		}
		else
		{
			for(int nIndex= 0; nIndex<3; nIndex++)
			{
				CvRect rect;
				rect.x = (nIndex%3)*m_imgSnap->width;
				if(m_nWordPos == 1)
					rect.y = (nIndex/3)*m_imgSnap->height;
				else
					rect.y = 0;
				rect.width = m_imgSnap->width;
				rect.height = m_imgSnap->height;
				cvSetImageROI(m_imgComposeSnap,rect);

				if(m_nDetectDirection == 0)//前排
				{
					if(nIndex == 0)
					{
						cvCopy(m_imgPreSnap, m_imgComposeSnap);
					}
					else if(nIndex == 1)
					{
						cvCopy(m_imgSnap, m_imgComposeSnap);
					}
					else
					{
						cvCopy(m_imgAuxSnap, m_imgComposeSnap);
					}
				}
				else//尾牌
				{
					if(nIndex == 0)
					{
						cvCopy(m_imgAuxSnap, m_imgComposeSnap);
					}
					else if(nIndex == 1)
					{
						cvCopy(m_imgPreSnap, m_imgComposeSnap);
					}
					else
					{
						cvCopy(m_imgSnap, m_imgComposeSnap);
					}
				}
				cvResetImageROI(m_imgComposeSnap);
			}
		}
		PutTextOnPEComposeImage(m_imgComposeSnap,event, uTime, uMiTime, strCarNum);

	//加锁
	pthread_mutex_lock(&g_Id_Mutex);
	if(!flag && (!bAppear))
	{
		if(m_eCapType == CAPTURE_FULL)
		{						
			
			{  
				//直接从全天录象中获取事件录象视频
				strPath = g_FileManage.GetEncodeFileName();
			}
			


			//printf("=======222=====event==========strPath=====================%s\n",strPath.c_str());

			strTmpPath = strPath.erase(0,g_strVideo.size());
			strTmpPath = g_ServerHost+strTmpPath;
			strPath = "ftp://"+strTmpPath;
			memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径
		}
		else
		{
			if(m_bEventCapture)
			{				
			
				{
					//需要判断磁盘是否已经满
					g_FileManage.CheckDisk(true,true);

					sHeader.uVideoId = g_uVideoId;
					//获取事件录象名称
					strPath = g_FileManage.GetEventVideoPath(g_uVideoId);

					strTmpPath = strPath.erase(0,g_strVideo.size());
					strTmpPath = g_ServerHost+strTmpPath;
					strPath = "ftp://"+strTmpPath;
					memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径
					//LogTrace("record.log", "============event==========chVideoPath:%s, sHeader.uVideoId=%u",event.chVideoPath, sHeader.uVideoId);
					//删除已经存在的记录
					// g_skpDB.DeleteOldRecord(strPath,true,true);
				}

			}
		}
		flag = true;
	}
		//需要判断磁盘是否已经满
		g_FileManage.CheckDisk(true,false);
		//获取快照图片名称及大小
		strPicPath = g_FileManage.GetPicPath();
		memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size()); //事件图片路径
		int nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);
		//解锁
		pthread_mutex_unlock(&g_Id_Mutex);
		//保存图片
		event.uPicSize = SaveImage(m_imgComposeSnap,strPicPath);
		//删除已经存在的记录
		g_skpDB.DeleteOldRecord(strPicPath,false,false);
		//记录检测数据,保存事件
		if(nSaveRet>0)
		{
			if(sResult.bHaveCarnum)//车牌事件
			{
				sResult.nChannelIndex = uRoadIndex;
				SavePlate(sResult, sHeader, event, tp);
			}
			else
			{
				//深圳北环格式
				if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
				{
					//LogNormal("event.uPosX=%d,event.uPosY=%d\n",event.uPosX,event.uPosY);
			
					{
						if(uTime*1000+uMiTime <= event.uTime2*1000+event.uMiTime2)
						{	
							event.uPosY +=  m_imgComposeSnap->height/2;	
						}
						else if(uTime*1000+uMiTime <= event.uEventBeginTime*1000+event.uMiEventBeginTime)
						{
							event.uPosY +=  m_imgComposeSnap->height/2;	
						}
						else 
						{
							event.uPosX +=  m_imgComposeSnap->width/2;	
						}
					}
					
					//LogNormal("event.uPosX=%d,event.uPosY=%d\n",event.uPosX,event.uPosY);
				}
				else
				{
					if (m_nDetectDirection == 1)
					{
						event.uPosX += (2 * m_imgSnap->width);
					}
					else
					{
						event.uPosX += m_imgSnap->width;
					}
				}
				g_skpDB.SaveTraEvent(sHeader.uChannelID,event,0,0,true);
			}
		}

	}

}
/*
 *保存事件图片
*/
void CSkpRoadDetect::SavePlateEventPic(UINT32 uTimeStamp,int  nTimeInterval,string& frame,long nEventId)
{
	int nMinTime = INT_MAX;
	int nIndex = -1;

	int nPreMinTime = INT_MAX;
	int nPreIndex = -1;

	int nPlateSize = FRAMESIZE;
	string strPicData;

	if(nTimeInterval > FRAMESIZE)
	{
		nTimeInterval = FRAMESIZE;
	}

	{
		//加锁
		pthread_mutex_lock(&m_preFrame_Mutex);

		for(int j=0; j < nPlateSize; j++)
		{
			SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(m_chResultFrameBuffer[j]);
			if(sDetectHeader)
			{

				{
					int detTime = abs((long long)uTimeStamp - nTimeInterval- sDetectHeader->uTimestamp);
					if(nMinTime >= detTime)
					{
						nMinTime = detTime;
						nIndex = j;
					}

					int predetTime = abs((long long)uTimeStamp - nTimeInterval-1- sDetectHeader->uTimestamp);
					if(nPreMinTime >= predetTime)
					{
						nPreMinTime = predetTime;
						nPreIndex = j;
					}
				}
			}
		}

		if(nIndex >= 0 && nPreIndex >= 0)
		{
			string strPic;
			strPic.append((char*)m_chResultFrameBuffer[nIndex],sizeof(SRIP_DETECT_HEADER)+m_img->imageSize);
			strPic.append((char*)frame.c_str(),frame.size());
			strPic.append((char*)m_chResultFrameBuffer[nPreIndex],sizeof(SRIP_DETECT_HEADER)+m_img->imageSize);

			pthread_mutex_lock(&m_EventPic_Mutex);
			if(m_EventPicMap.size() > 8)//最多放8个事件
			{
				map<long,string>::iterator it_map = m_EventPicMap.begin();
				m_EventPicMap.erase(it_map);
			}
			m_EventPicMap.insert(map<long,string>::value_type(nEventId,strPic));
			pthread_mutex_unlock(&m_EventPic_Mutex);
		}

		//解锁
		pthread_mutex_unlock(&m_preFrame_Mutex);
	}
}
/*
 * 从磁盘中获取事件图片
*/
SRIP_DETECT_HEADER CSkpRoadDetect::FindPicFromDisk(long dEventId, UINT32 & uTime, UINT32 & uMiTime,SRIP_DETECT_HEADER& sAuxHeader)
{
		  SRIP_DETECT_HEADER sPreHeader;

		  pthread_mutex_lock(&m_EventPic_Mutex);
		  map<long,string>::iterator it = m_EventPicMap.find(dEventId);
		  if(it != m_EventPicMap.end())
		  {

			  if (it->second.size() > 0)
			  {
				  //前一张图
				  SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(it->second.c_str());
				  sPreHeader.uTimestamp = sDetectHeader->uTimestamp;
				  sPreHeader.uTime64 = sDetectHeader->uTime64;
				  sPreHeader.uSeq = sDetectHeader->uSeq;
				  
				  cvSet(m_imgAuxSnap, cvScalar( 0,0, 0 ));
				  cvSet(m_imgPreSnap, cvScalar( 0,0, 0 ));
				  cvSet(m_imgSnap, cvScalar( 0,0, 0 ));
					
				  {
					  if(m_nDeinterlace==2)
					  {
						  cvSetData(m_imgPre,(void *)(it->second.c_str()+sizeof(SRIP_DETECT_HEADER)),m_imgPre->widthStep);
						  cvResize(m_imgPre,m_imgPreSnap);
					  }
					  else
					  {
						  memcpy(m_imgPreSnap->imageData,it->second.c_str()+sizeof(SRIP_DETECT_HEADER),m_imgPre->imageSize);
					  }

					  //后一张图
					  SRIP_DETECT_HEADER *pHeader = (SRIP_DETECT_HEADER*)(it->second.c_str()+sizeof(SRIP_DETECT_HEADER)+m_imgPre->imageSize);
					  uTime = pHeader->uTimestamp;
					  uMiTime = (pHeader->uTime64 / 1000)%1000;
					  
					  if(m_nDeinterlace==2)
					  {
						  cvSetData(m_imgPre,(void *)(it->second.c_str()+2*sizeof(SRIP_DETECT_HEADER)+m_imgPre->imageSize),m_imgPre->widthStep);
						  cvResize(m_imgPre,m_imgSnap);
					  }
					  else
					  {
						  memcpy(m_imgSnap->imageData,it->second.c_str()+2*sizeof(SRIP_DETECT_HEADER)+m_imgPre->imageSize,m_imgPre->imageSize);
					  }

					   //最后一张图
					  pHeader = (SRIP_DETECT_HEADER*)(it->second.c_str()+2*sizeof(SRIP_DETECT_HEADER)+2*m_imgPre->imageSize);
					  sAuxHeader.uTimestamp = pHeader->uTimestamp;
					  sAuxHeader.uTime64 = pHeader->uTime64;
					  
					  if(m_nDeinterlace==2)
					  {
						  cvSetData(m_imgPre,(void *)(it->second.c_str()+3*sizeof(SRIP_DETECT_HEADER)+2*m_imgPre->imageSize),m_imgPre->widthStep);
						  cvResize(m_imgPre,m_imgAuxSnap);
					  }
					  else
					  {
						  memcpy(m_imgAuxSnap->imageData,it->second.c_str()+3*sizeof(SRIP_DETECT_HEADER)+2*m_imgPre->imageSize,m_imgPre->imageSize);
					  }
				  }
			  }
		  }
		  else
		  {
			 LogNormal("无法从内存中找到事件图片dEventId=%d,m_EventPicMap.size=%d\n",dEventId,m_EventPicMap.size());
		  }
		  pthread_mutex_unlock(&m_EventPic_Mutex);

		 return sPreHeader;
}



bool CSkpRoadDetect::GetBmpImage(const char *fileName, string& strPicData1, string& strPicData2, bool IsTwo/*=true*/)
{
    bool bRet = false;
	FILE *fp = fopen(fileName, "rb");
	char *pData = new char[m_nBmpSize];
	int nCount = -1;
	if (fp)
	{
	   int nSize = fread(pData, 1, m_nBmpSize, fp);
	   if (nSize > 0)
	   {
         strPicData1.append(pData, m_nBmpSize);
		  if (IsTwo)//两张图片
		  {
		     memset(pData, 0, m_nBmpSize);
            nSize = fread(pData, 1, m_nBmpSize, fp);
			 if (nSize > 0)
			 {
				 strPicData2.append(pData, m_nBmpSize);
			 }
		  }
		  bRet = true;
	   }
	   fclose(fp);
	}
	delete [] pData;

   return bRet;
}
/*
 * 在车牌事件的合成图片上绘制文字
 * event 事件
*/
void CSkpRoadDetect::PutTextOnPEComposeImage(IplImage* pImage, const RECORD_EVENT& event, UINT32 uTime, UINT32 uMiTime, string& strCarNum)
{
	wchar_t wchOut[512] = {'\0'};
	char chOut[512] = {'\0'};

	int nStartX = 0;
	int nWidth = 10;
	int nHeight = 0;

	//深圳北环格式
	if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
	{
		std::string strTime;

		for(int nIndex = 0;nIndex<4;nIndex++)
		{
			nWidth = 400+m_imgSnap->width*(nIndex%2);
			nHeight =  g_PicFormatInfo.nFontSize+m_imgSnap->height*(nIndex/2);
			
				if(nIndex == 0)
				{
					if(uTime*1000+uMiTime <= event.uTime2*1000+event.uMiTime2)
					{
						strTime = GetTime(uTime,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),uMiTime);
					}
					else if(uTime*1000+uMiTime <= event.uEventBeginTime*1000+event.uMiEventBeginTime)
					{
						strTime = GetTime(event.uTime2,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiTime2);
					}
					else
					{
						strTime = GetTime(event.uTime2,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiTime2);
					}
				}
				else if(nIndex == 1)
				{
					if(uTime*1000+uMiTime <= event.uTime2*1000+event.uMiTime2)
					{
						strTime = GetTime(event.uTime2,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiTime2);
					}
					else if(uTime*1000+uMiTime <= event.uEventBeginTime*1000+event.uMiEventBeginTime)
					{
						strTime = GetTime(uTime,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),uMiTime);
					}
					else
					{
						strTime = GetTime(event.uEventBeginTime,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiEventBeginTime);
					}
					
				}
				else  if(nIndex == 2)
				{
					if(uTime*1000+uMiTime <= event.uTime2*1000+event.uMiTime2)
					{
						strTime = GetTime(event.uEventBeginTime,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiEventBeginTime);
					}
					else if(uTime*1000+uMiTime <= event.uEventBeginTime*1000+event.uMiEventBeginTime)
					{
						strTime = GetTime(event.uEventBeginTime,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiEventBeginTime);
					}
					else
					{
						strTime = GetTime(uTime,0);
						sprintf(chOut,"%s:%03d",strTime.c_str(),uMiTime);
					}
				}
				else
				{
					strTime = GetTime(event.uEventBeginTime,0);
					sprintf(chOut,"%s:%03d",strTime.c_str(),event.uMiEventBeginTime);
				}

			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
			
			//叠加车牌信息
			if ((strCarNum.size() >= 7) && (strCarNum.size() <= 20))
			{
				if(nIndex == 3)
				{
					nHeight += g_PicFormatInfo.nFontSize;
					sprintf(chOut,"车牌号码:%s",strCarNum.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
				}
			}
		}
		
		nWidth = 40;
		nHeight = pImage->height - g_PicFormatInfo.nFontSize;
		std::string strDirection = GetDirection(event.uDirection);
		sprintf(chOut,"地点名称:%s  方向:%s  违法时间:%s:%03d",m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),event.uMiEventBeginTime);//
		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,chOut);
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

		return;
	}

	if(m_nExtent <= 0)
	{
		return;
	}

	if( m_nWordPos== 0)//文字区域叠加在图片下方
	{
		nHeight = m_imgSnap->height - m_nExtent;
	}

	nStartX = nWidth;


	//设备编号
	std::string strDirection = GetDirection(event.uDirection);
	std::string strTime;
	unsigned int uMili = 0;
    strTime = GetTime(event.uEventBeginTime,0);
	uMili = event.uMiEventBeginTime;

	if (strCarNum.empty() || strCarNum.size() == 0 || strCarNum.size() > 20 || strCarNum.size() < 7)
	{
		sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  时间:%s.%03d",g_strDetectorID.c_str(), m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),uMili);//
	}
	else
	{
		sprintf(chOut,"设备编号:%s  车牌号码:%s  地点名称:%s  方向:%s  时间:%s.%03d",g_strDetectorID.c_str(),strCarNum.c_str(), m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),uMili);//
	}
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	nHeight += 90;
	/*int n = 0;
	wchar_t *p = wchOut;
	while (*p != NULL)
	{
		printf("====================wchOut=%c", *((char *)p));
	   p++;
	   n++;
	}
	printf("\n wChout length:%d\n", n);
	LogTrace(NULL, "1-----chOut:%s width:%d, height:%d Image width:%d Image height:%d, length:%d", chOut, nWidth, nHeight, pImage->width, pImage->height, strlen(chOut));*/
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));


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

	if (m_nDetectDirection == 0)//前排
	{
		sprintf(chOut,"%s.%03d",GetTime(event.uTime2,0).c_str(), event.uMiTime2);
	}
	else
	{
		sprintf(chOut,"%s.%03d",GetTime(uTime,0).c_str(), uMiTime);
	}
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));


	//第二时间
	if(m_imgSnap->width > 2000)
		nWidth = 3200;
	else
		nWidth = 2000;

	if (m_nDetectDirection == 0)//前排
	{
		sprintf(chOut,"%s.%03d",GetTime(event.uEventBeginTime,0).c_str(), event.uMiEventBeginTime);
	}
	else
	{
		sprintf(chOut,"%s.%03d",GetTime(event.uTime2,0).c_str(), event.uMiTime2);
	}

	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	//LogTrace(NULL, "2-----chOut:%s", chOut);
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

	//第三时间
	if(m_imgSnap->width > 2000)
		nWidth = 5800;
	else
		nWidth = 3600;
	if (m_nDetectDirection == 0)//前排
	{
		sprintf(chOut,"%s.%03d",GetTime(uTime,0).c_str(), uMiTime);
	}
	else
	{
		sprintf(chOut,"%s.%03d",GetTime(event.uEventBeginTime,0).c_str(), event.uMiEventBeginTime);
	}

	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	//LogTrace(NULL, "3-----chOut:%s", chOut);
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
}
//获取秒图
SRIP_DETECT_HEADER CSkpRoadDetect::GetImageFromResultFrameList(int64_t uTimeStamp,int  nTimeInterval, IplImage* image,UINT32 nPreSeq)
{
	SRIP_DETECT_HEADER sPreHeader;

    int nMinTime = INT_MAX;
    int nIndex = -1;
    int nPlateSize = FRAMESIZE;

    if(nTimeInterval > FRAMESIZE*300000)
    {
        nTimeInterval = FRAMESIZE*300000;
    }

    //加锁
    pthread_mutex_lock(&m_preFrame_Mutex);
    for(int j=0; j < nPlateSize; j++)
    {
        SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(m_chResultFrameBuffer[j]);
        if(sDetectHeader)
        {
			//LogNormal("GetImageFromResultFrameList,uSeq=%lld\n",sDetectHeader->uSeq);
              int detTime = abs((long long)uTimeStamp - nTimeInterval - sDetectHeader->uTime64);
              if(nMinTime >= detTime)
              {
				  if(nPreSeq != sDetectHeader->uSeq)
				  {
					  nMinTime = detTime;
					  nIndex = j;
				  }
              }
        }
    }

    if(nIndex>=0)
    {
        SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(m_chResultFrameBuffer[nIndex]);
        sPreHeader.uTimestamp = sDetectHeader->uTimestamp;
        sPreHeader.uTime64 = sDetectHeader->uTime64;
		sPreHeader.uSeq = sDetectHeader->uSeq;
		//LogNormal("sPreHeader.uSeq=%lld\n",sPreHeader.uSeq);

		if(image != NULL)
		{
			//cvSet(image, cvScalar( 0,0, 0 ));
			if(m_nDeinterlace==2)
			{
				cvSetData(m_imgPre,m_chResultFrameBuffer[nIndex]+sizeof(SRIP_DETECT_HEADER),m_imgPre->widthStep);
				cvResize(m_imgPre,image);
			}
			else
			{
				memcpy(image->imageData,m_chResultFrameBuffer[nIndex]+sizeof(SRIP_DETECT_HEADER),m_imgPre->imageSize);
			}
			
			CvRect rect;
			rect.x = 0;
			rect.width = m_imgSnap->width;
			rect.height = m_nExtent;

			if(m_nWordPos == 0)//字在图下方
			{
				rect.y = m_imgSnap->height-m_nExtent;
			}
			else
			{
				rect.y = 0;
			}

			if(rect.height > 0)
			{
				cvSetImageROI(image,rect);
				cvSet(image, cvScalar( 0,0, 0 ));
				cvResetImageROI(image);
			}
		}
    }

     //解锁
    pthread_mutex_unlock(&m_preFrame_Mutex);

    return sPreHeader;
}

//从内存中获取车牌图像
bool CSkpRoadDetect::FindPicFromMem(char *chPlate, IplImage *pImage, TimePlate *pTimePlate)
{
	bool bRet = false;
	if (chPlate == NULL)
	{
		return bRet;
	}
   pthread_mutex_lock(&m_muxPic);
   list< pair<TimePlate, string> >::const_iterator it_b = m_Piclist.begin();
   while (it_b != m_Piclist.end())
   {
	   if(strncmp(it_b->first.chPlate, chPlate, 7) == 0)
	   {
		   cvSet(pImage, cvScalar( 0,0, 0 ));
		   bRet = true;
		   memcpy(pImage->imageData, it_b->second.c_str(), it_b->second.size());
          pTimePlate->uSec = it_b->first.uSec;
		   pTimePlate->uMiSec = it_b->first.uMiSec;
		   pTimePlate->nLeft = it_b->first.nLeft;
		   pTimePlate->nTop = it_b->first.nTop;
		   pTimePlate->nWidth = it_b->first.nWidth;
		   pTimePlate->nHeight = it_b->first.nHeight;
          break;
	   }
     it_b++;
   }
   pthread_mutex_unlock(&m_muxPic);
   return bRet;
}

//输出测试结果
void CSkpRoadDetect::OutPutTestResult(std::string& frame)
{
	if(m_listTestResult.size() > 0)
	{
		//LogNormal("m_listTestResult.size()=%lld",m_listTestResult.size());

		std::list<RECORD_PLATE>::iterator it_b = m_listTestResult.begin();
		std::list<RECORD_PLATE>::iterator it_e = m_listTestResult.end();

		while(it_b != it_e)
		{
			SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

			if( sDetectHeader.uSeq > it_b->uSeqID+20)
			{
				LogNormal("帧号过大不输出结果uSeq=%lld,uSeqID=%lld",sDetectHeader.uSeq,it_b->uSeqID);
				m_listTestResult.erase(it_b);
				break;
			}
			else if( sDetectHeader.uSeq >= it_b->uSeqID)
			{
				LogNormal("输出结果uSeq=%lld,uSeqID=%lld",sDetectHeader.uSeq,it_b->uSeqID);

				m_list_DetectOut.clear();

				SRIP_DETECT_OUT_RESULT result;

				result.nChannelIndex = it_b->uRoadWayID;//车道编号

				result.eRtype = (DETECT_RESULT_TYPE)it_b->uViolationType;//事件类型
				result.color.nColor1 = it_b->uCarColor1;//颜色

				result.x = it_b->uPosLeft/m_fScaleX;     //事件发生横坐标
				result.y = it_b->uPosTop/m_fScaleY;     //事件发生纵坐标

				result.value = it_b->uSpeed;//速度

				if(it_b->uType == PERSON_TYPE)
				{
					result.type = 1;					//车辆类型(0车,1人，2非机动车)
				}
				else if(it_b->uType < OTHER_TYPE)
				{
					result.type = 0;
				}
				else
				{
					result.type = 2;
				}

				//行驶方向
				if(m_nDetectDirection == 0)//前牌
				{
					result.direction = 90;
				}
				else //尾牌
				{
					result.direction = 270;
				}

				result.dEventTime = (double)(sDetectHeader.uTimestamp+sDetectHeader.uTime64/1000000.0);			//事件检测时间

				//result.fLastImgTime = 0;        //往前取上张图片的时间(单位秒)

				result.bShow = false;

				m_list_DetectOut.push_back(result);

				OutPutResult(frame);

				m_listTestResult.erase(it_b);
				break;
			}
			it_b++;
		}
	}
}

//读取测试结果文件
bool CSkpRoadDetect::LoadTestResult(char* filename)
{
	if(access(filename,F_OK) == 0)
	{
		m_listTestResult.clear();

		LogNormal("filename=%s",filename);
		g_skpDB.LoadResult(filename,m_listTestResult);
		if(m_listTestResult.size() > 0)
		{
			return true;
		}
	}
	return false;
}

//获取图片路径
int CSkpRoadDetect::GetEventPicPath(RECORD_EVENT& event,std::string& strPicPath)
{
	char buf[256] = {0};
	//存储大图片
	std::string strPath;

	string strTime = GetTime(event.uEventBeginTime,1);
	g_skpDB.UTF8ToGBK(strTime);

	string strViolationType = GetViolationType(event.uCode);

	strPath = g_FileManage.GetSpecialPicPath(2);
	sprintf(buf,"%s/%s_%d_%s.jpg",strPath.c_str(),strTime.c_str(),event.uRoadWayID,strViolationType.c_str());

	strPicPath = buf;

	return 1;
}

//流量统计
void CSkpRoadDetect::VehicleStatistic(int uRoadIndex,DetectResultList::iterator& it_b,RECORD_STATISTIC& statistic)
{
	UINT32 uObjectNum = 0;
	UINT32 uObjectAll = 0;

	if(g_nServerType == 7)
	{
		switch(it_b->eRtype)
		{
		case DETECT_RESULT_STAT_FLUX:  //高16位大车总数，低16位车辆总数
			//车道号和车道类型
			uRoadIndex =  uRoadIndex<<16;
			statistic.uRoadType[it_b->nChannelIndex-1] = uRoadIndex|PERSON_ROAD;
			uObjectAll = it_b->uFluxVehicle;
			uObjectNum = it_b->uFluxBig;
			uObjectNum = uObjectNum<<16;
			statistic.uFlux[it_b->nChannelIndex-1] = (uObjectNum | uObjectAll);
			statistic.uFluxCom[it_b->nChannelIndex-1] = it_b->uFluxSmall;
			break;
		case DETECT_RESULT_STAT_SPEED_AVG:
			statistic.uSpeed[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
			statistic.uMaxSpeed[it_b->nChannelIndex-1] = (UINT32)(it_b->fMaxSpeed+0.5);
			statistic.uBigCarSpeed[it_b->nChannelIndex-1] = (UINT32)(it_b->fBigSpeed+0.5);
			statistic.uSmallCarSpeed[it_b->nChannelIndex-1] = (UINT32)(it_b->fSmallSpeed+0.5);
			break;
		case DETECT_RESULT_STAT_ZYL:
			statistic.uOccupancy[it_b->nChannelIndex-1] = it_b->uTimePoss;//(UINT32)(it_b->value+0.5);
			statistic.uSpace[it_b->nChannelIndex-1] = it_b->uTimeInterval;//(UINT32)(it_b->value+0.5);
			break;
		default:
			break;
		}
	}
	else
	{
		switch(it_b->eRtype)
		{
		
		case DETECT_RESULT_STAT_FLUX:  //高16位机动车辆总数，低16位为所有物体数目
			//车道号和车道类型
			uRoadIndex =  uRoadIndex<<16;
			statistic.uRoadType[it_b->nChannelIndex-1] = uRoadIndex|PERSON_ROAD;

			if(uFluxAll[it_b->nChannelIndex-1]>0)
			{
				uObjectNum   =  uFluxVehicle[it_b->nChannelIndex-1];//由应用模块统计通过物体的数量
				uObjectAll = uFluxAll[it_b->nChannelIndex-1];
			}
			else
			{
				if(it_b->value>0)
				{
					uObjectAll = it_b->value;
					uObjectNum = it_b->value;
				}
				else
				{
					uObjectNum = it_b->uFluxVehicle;
					uObjectAll = it_b->uFluxAll;
				}
			}
			uObjectNum = uObjectNum<<16;
			statistic.uFlux[it_b->nChannelIndex-1] = (uObjectNum | uObjectAll);
			break;
		case DETECT_RESULT_STAT_SPEED_AVG:
			statistic.uSpeed[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
			break;
		case DETECT_RESULT_STAT_QUEUE:
			statistic.uQueue[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
			break;
		case DETECT_RESULT_STAT_ZYL:
			statistic.uOccupancy[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
			break;
		case DETECT_RESULT_STAT_CTJJ:
			statistic.uSpace[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
			break;
		case DETECT_RESULT_STAT_CAR_TYPE:   //高16位非机动车总数，低16位行人总数
			if(uFluxAll[it_b->nChannelIndex-1]>0)
			{
				uObjectNum = uFluxNoneVehicle[it_b->nChannelIndex-1];
				uObjectNum = uObjectNum<<16;
				statistic.uFluxCom[it_b->nChannelIndex-1] = (uObjectNum|uFluxPerson[it_b->nChannelIndex-1]);
			}
			else
			{
				uObjectNum = it_b->uFluxNoneVehicle;
				uObjectNum = uObjectNum<<16;
				statistic.uFluxCom[it_b->nChannelIndex-1] = (uObjectNum|it_b->uFluxPerson);
			}
			uFluxVehicle[it_b->nChannelIndex-1] = 0;
			uFluxAll[it_b->nChannelIndex-1] = 0;
			uFluxNoneVehicle[it_b->nChannelIndex-1] = 0;
			uFluxPerson[it_b->nChannelIndex-1] = 0;
			break;
		case DETECT_RESULT_STAT_SLOW_QUEUE_LEN:   //nahs
			uRoadIndex =  uRoadIndex<<16;
			statistic.uRoadType[it_b->nChannelIndex-1] = uRoadIndex|PERSON_ROAD;
			statistic.uQueue[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
			statistic.uFlux[it_b->nChannelIndex-1] = 0;
			break;
		default:
			break;
		}
	}
}

//流量评价统计
void CSkpRoadDetect::VtsStatistic(SRIP_DETECT_HEADER* buf)
{
	UINT16 uTrafficSignal = buf->uTrafficSignal;
	UINT32 uSeq = buf->uSeq;

	//车道编号
	int uRoadIndex = 0;
	//统计信息

	bool bStatistic = false;

	bool bLeftStatistic = false;

	VTSParaMap::iterator it_p = m_vtsObjectParaMap.begin();
	while(it_p != m_vtsObjectParaMap.end())
	{
		EVENT_VTS_SIGNAL_STATUS vtsSignalStatus;

		vtsSignalStatus.nRoadIndex =  it_p->second.nRoadIndex;

		vtsSignalStatus.uFrameSeq = uSeq;

		vtsSignalStatus.bLeftSignal = ((uTrafficSignal>>(it_p->second.nLeftControl)) & 0x1);
		vtsSignalStatus.bStraightSignal = ((uTrafficSignal>>(it_p->second.nStraightControl)) & 0x1);
		vtsSignalStatus.bRightSignal = ((uTrafficSignal>>(it_p->second.nRightControl)) & 0x1);

		//计算直行灯非红灯时间间隔
		map<int,EVENT_VTS_SIGNAL_STATUS>::iterator it = m_mapVTSStatus.find(vtsSignalStatus.nRoadIndex);

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
						m_uNonTrafficSignalBeginTime = buf->uTime64;
					}

				}

				//绿变红
				if((it->second.bStraightSignal == 0) && (vtsSignalStatus.bStraightSignal == 1))
				{
					if((m_uNonTrafficSignalEndTime == 0) &&(m_uNonTrafficSignalBeginTime != 0))
					{
						m_uNonTrafficSignalEndTime = buf->uTime64;

						m_uNonTrafficSignalTime = m_uNonTrafficSignalEndTime - m_uNonTrafficSignalBeginTime;
						LogNormal("event NonTrafficSignalTime=%d",m_uNonTrafficSignalTime);
					}
				}

				//下次绿灯期间开始统计
				if(m_uNonTrafficSignalTime > 0)
				{
					//红变绿(绿灯开始时刻)
					if((it->second.bStraightSignal == 1) && (vtsSignalStatus.bStraightSignal == 0))
					{
						m_uPreNonTrafficSignalTime = buf->uTime64;//当前统计时刻
						m_nConnectnumber = 0;
						bStatistic = true;
					}
					else if((it->second.bStraightSignal == 0) && (vtsSignalStatus.bStraightSignal == 1))
					{
						if(m_nConnectnumber == 2)
						{
							m_uPreNonTrafficSignalTime = buf->uTime64;//当前统计时刻
							m_nConnectnumber = 3;
							bStatistic = true;
						}
					}
					else if(vtsSignalStatus.bStraightSignal == 0)
					{
						if(buf->uTime64 >= m_uPreNonTrafficSignalTime + m_uNonTrafficSignalTime/3)
						{
							m_uPreNonTrafficSignalTime = buf->uTime64;//当前统计时刻
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
						m_uNonLeftTrafficSignalBeginTime = buf->uTime64;
					}

				}

				//绿变红
				if((it->second.bLeftSignal == 0) && (vtsSignalStatus.bLeftSignal == 1))
				{
					if((m_uNonLeftTrafficSignalEndTime == 0) &&(m_uNonLeftTrafficSignalBeginTime != 0))
					{
						m_uNonLeftTrafficSignalEndTime = buf->uTime64;

						m_uNonLeftTrafficSignalTime = m_uNonLeftTrafficSignalEndTime - m_uNonLeftTrafficSignalBeginTime;
						LogNormal("event NonLeftTrafficSignalTime=%d",m_uNonLeftTrafficSignalTime);
					}
				}

				//下次绿灯期间开始统计
				if(m_uNonLeftTrafficSignalTime > 0)
				{
					//红变绿(绿灯开始时刻)
					if((it->second.bLeftSignal == 1) && (vtsSignalStatus.bLeftSignal == 0))
					{
						m_uPreNonLeftTrafficSignalTime = buf->uTime64;//当前统计时刻
						m_nLeftConnectnumber = 4;
						bLeftStatistic = true;
					}
					else if((it->second.bLeftSignal == 0) && (vtsSignalStatus.bLeftSignal == 1))
					{
						if(m_nLeftConnectnumber == 6)
						{
							m_uPreNonLeftTrafficSignalTime = buf->uTime64;//当前统计时刻
							m_nLeftConnectnumber = 7;
							bLeftStatistic = true;
						}
					}
					else if(vtsSignalStatus.bLeftSignal == 0)
					{
						if(buf->uTime64 >= m_uPreNonLeftTrafficSignalTime + m_uNonLeftTrafficSignalTime/3)
						{
							m_uPreNonLeftTrafficSignalTime = buf->uTime64;//当前统计时刻
							m_nLeftConnectnumber++;
							bLeftStatistic = true;
						}
					}
				}
			}

			m_mapVTSStatus.erase(it);
		}
		m_mapVTSStatus.insert(map<int,EVENT_VTS_SIGNAL_STATUS>::value_type(vtsSignalStatus.nRoadIndex,vtsSignalStatus));

		it_p++;
	}

	if(m_uNonTrafficSignalBeginTime != m_uNonLeftTrafficSignalBeginTime)
	{
		if(bLeftStatistic)
		{
			RECORD_STATISTIC statistic2;
			DetectResultList list_LeftDetectOut;

			if( m_nLeftConnectnumber == 4 )
			{
				//LogNormal("left start buf->uTime64=%lld,m_nLeftConnectnumber=%d",buf->uTime64,m_nLeftConnectnumber);
				m_Rdetect.StartLeftStatistic(buf->uTime64);
			}
			else
			{
				//LogNormal("left end buf->uTime64=%lld,m_nLeftConnectnumber=%d",buf->uTime64,m_nLeftConnectnumber);
				m_Rdetect.EndLeftStatistic(buf->uTime64,list_LeftDetectOut);

				if(m_nLeftConnectnumber < 7)
				{
					//LogNormal("left start buf->uTime64=%lld,m_nLeftConnectnumber=%d",buf->uTime64,m_nLeftConnectnumber);
					m_Rdetect.StartLeftStatistic(buf->uTime64);
				}
			}

			if(list_LeftDetectOut.size() > 0)
			{
				//事件统计列表
				DetectResultList::iterator it_b = list_LeftDetectOut.begin();
				DetectResultList::iterator it_e = list_LeftDetectOut.end();


				while(it_b!=it_e)
				{
					//根据车道号获取车道逻辑编号
					RoadIndexMap::iterator it_p = m_roadMap.find(it_b->nChannelIndex);
					if(it_p != m_roadMap.end())
					{
						uRoadIndex = it_p->second.nVerRoadIndex;
					}
					else
					{
						uRoadIndex = it_b->nChannelIndex;
					}

					//LogNormal("11 uRoadIndex=%d,it_b->eRtype=%d",uRoadIndex,it_b->eRtype);

					if(it_b->eRtype >= DETECT_RESULT_STAT_FLUX)
					{
						switch(it_b->eRtype)
						{
						case DETECT_RESULT_STAT_FLUX:
							//车道占有率
							uRoadIndex =  uRoadIndex<<16;
							statistic2.uRoadType[it_b->nChannelIndex-1] = uRoadIndex|PERSON_ROAD;
							statistic2.uFlux[it_b->nChannelIndex-1] = 0;
							statistic2.uOccupancy[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
							statistic2.uSpace[it_b->nChannelIndex-1] = it_b->bPoss;
							break;
						default:
							break;
						}
					}
					it_b++;
				}


				std::string strStatistic;
				SRIP_DETECT_HEADER sDHeader;
				sDHeader.uChannelID = m_nCameraId;
				sDHeader.uDetectType = MIMAX_STATISTIC_REP;
				sDHeader.uRealTime = 0x00000001;//此时均设为实时统计
				strStatistic.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));

				statistic2.uTime = buf->uTimestamp;
				statistic2.uSeq = m_nLeftConnectnumber;
				statistic2.uStatTimeLen = m_uNonLeftTrafficSignalTime/3000000.0;
				strStatistic.append((char*)&statistic2,sizeof(RECORD_STATISTIC));

				g_RoadImcData.AddStatisticData(strStatistic,PERSON_ROAD,1);
			}
		}
	}
	
	if(bStatistic)
	{
		RECORD_STATISTIC statistic1;
		
		DetectResultList list_StraightDetectOut;

		if( m_nConnectnumber == 0 )
		{
			//LogNormal("straight start buf->uTime64=%lld,m_nConnectnumber=%d",buf->uTime64,m_nConnectnumber);
			m_Rdetect.StartStraightStatistic(buf->uTime64);
		}
		else
		{
			//LogNormal("straight end buf->uTime64=%lld,m_nConnectnumber=%d",buf->uTime64,m_nConnectnumber);
			m_Rdetect.EndStraightStatistic(buf->uTime64,list_StraightDetectOut);
			
			if(m_nConnectnumber < 3)
			{
				//LogNormal("straight start buf->uTime64=%lld,m_nConnectnumber=%d",buf->uTime64,m_nConnectnumber);
				m_Rdetect.StartStraightStatistic(buf->uTime64);
			}
		}

		if(list_StraightDetectOut.size() > 0)
		{
			//事件统计列表
			DetectResultList::iterator it_b = list_StraightDetectOut.begin();
			DetectResultList::iterator it_e = list_StraightDetectOut.end();
			
			
			while(it_b!=it_e)
			{
				//根据车道号获取车道逻辑编号
				RoadIndexMap::iterator it_p = m_roadMap.find(it_b->nChannelIndex);
				if(it_p != m_roadMap.end())
				{
					uRoadIndex = it_p->second.nVerRoadIndex;
				}
				else
				{
					uRoadIndex = it_b->nChannelIndex;
				}
				//LogNormal(" uRoadIndex=%d,it_b->eRtype=%d",uRoadIndex,it_b->eRtype);

				if(it_b->eRtype >= DETECT_RESULT_STAT_FLUX)
				{
					switch(it_b->eRtype)
					{
					case DETECT_RESULT_STAT_FLUX:
						//车道号和车道类型
						uRoadIndex =  uRoadIndex<<16;
						statistic1.uRoadType[it_b->nChannelIndex-1] = uRoadIndex|PERSON_ROAD;
						statistic1.uFlux[it_b->nChannelIndex-1] = 0;
						statistic1.uOccupancy[it_b->nChannelIndex-1] = (UINT32)(it_b->value+0.5);
						statistic1.uSpace[it_b->nChannelIndex-1] = it_b->bPoss;
						break;
					default:
						break;
					}
						 
				}

				it_b++;
			}

			std::string strStatistic;
			SRIP_DETECT_HEADER sDHeader;
			sDHeader.uChannelID = m_nCameraId;
			sDHeader.uDetectType = MIMAX_STATISTIC_REP;
			sDHeader.uRealTime = 0x00000001;//此时均设为实时统计
			strStatistic.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));

			statistic1.uTime = buf->uTimestamp;
			statistic1.uSeq = m_nConnectnumber;
			statistic1.uStatTimeLen = m_uNonTrafficSignalTime/3000000.0;
			strStatistic.append((char*)&statistic1,sizeof(RECORD_STATISTIC));

			g_RoadImcData.AddStatisticData(strStatistic,PERSON_ROAD,1);
		}
	}
}

//设置H264采集类指针
bool CSkpRoadDetect::SetH264Capture(CRoadH264Capture *pH264Capture)
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

//车牌识别
string CSkpRoadDetect::CarNumDetect(string& strFrame)
{
	string strCarnum("");

	carnum_parm_t cnp;
	cnp.isday = m_nDayNight;
	cnp.direction = 1;
	mvcarnumdetect.set_carnum_parm(&cnp);


	loop_parmarer m_LoopParmarer;
	char tmp[32]= {0};
	IplImage* tmpimg=NULL;
	carnum_context vehicle_result[CARNUMSIZE];
	road_context context;

	CvRect rtCarnumROI;
	rtCarnumROI.x= 0;
	rtCarnumROI.y = m_imgSnap->height/4;
	rtCarnumROI.width = m_imgSnap->width;
	rtCarnumROI.height = m_imgSnap->height*3/4;
	IplImage* img = cvCreateImageHeader(cvSize(m_imgSnap->width,m_imgSnap->height),8,3);
	cvSetData(img,(char*)(strFrame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_imgSnap->widthStep);
	mvcarnumdetect.CalibrationSet(NULL);
	int nCarNumResult = mvcarnumdetect.find_carnum(NULL,img,tmp,&tmpimg,rtCarnumROI,1,vehicle_result,&context,NULL,m_LoopParmarer);
	LogNormal("-------nCarNumResult:%d", nCarNumResult);

	if(tmpimg!=NULL)
	{
		cvReleaseImage(&tmpimg);
		tmpimg = NULL;
	}

	if(img!=NULL)
	{
		cvReleaseImageHeader(&img);
		img = NULL;
	}

	int nCarNumWidth = 0;
	for(int i = 0;i < nCarNumResult;i++)
	{
		bool bCarNum = true;

		if( (vehicle_result[i].carnum[0] == '*') || (vehicle_result[i].carnum[0] == '$') )
		{
			bCarNum = false;
		}

		if(bCarNum)
		{
			if(nCarNumWidth <= vehicle_result[i].position.width)
			{
				strCarnum.clear();
				strCarnum.append(vehicle_result[i].carnum,7);
				//车牌号码转换
				CarNumConvert(strCarnum,vehicle_result[i].wjcarnum);

				nCarNumWidth = vehicle_result[i].position.width;
			}
		}
	}

	return strCarnum;
}

string CSkpRoadDetect::CarNumDetect(string& strFrame,CvRect *pRectAtDelect)
{
	if(pRectAtDelect == NULL)
	{
		string strCarnumTmp("");
		return strCarnumTmp;
	}
	string strCarnum("");

	carnum_parm_t cnp;
	cnp.isday = m_nDayNight;
	mvcarnumdetect.set_carnum_parm(&cnp);

	
	loop_parmarer m_LoopParmarer;
	char tmp[32]= {0};
	IplImage* tmpimg=NULL;
	carnum_context vehicle_result[CARNUMSIZE];
	road_context context;

	CvRect rtCarnumROI;
	if((pRectAtDelect != NULL) && (pRectAtDelect->width > 0) && (pRectAtDelect->height > 0))
	{
		rtCarnumROI.x= pRectAtDelect->x;
		rtCarnumROI.y = pRectAtDelect->y;
		rtCarnumROI.width = pRectAtDelect->width;
		rtCarnumROI.height = pRectAtDelect->height;
	}
	else
	{
		rtCarnumROI.x= 0;
		rtCarnumROI.y = m_imgSnap->height/4;
		rtCarnumROI.width = m_imgSnap->width;
		rtCarnumROI.height = m_imgSnap->height*3/4;
	}

	LogNormal("-------Next CarnumDelect: x=%d,y=%d,width=%d,height=%d \n",pRectAtDelect->x,pRectAtDelect->y,pRectAtDelect->width,pRectAtDelect->height);
	LogNormal("-------Next CarnumDelect: width=%d,height=%d \n",m_imgSnap->width,m_imgSnap->height);

	IplImage* img = cvCreateImageHeader(cvSize(m_imgSnap->width,m_imgSnap->height),8,3);

	cvSetData(img,(char*)(strFrame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_imgSnap->widthStep);
	char szSavePicName[100] = {0};
	sprintf(szSavePicName,"%s.jpg","carnumtest");
	cvSaveImage(szSavePicName,img);
#ifdef CAMERAAUTOCTRL
	//mvcarnumdetect.set_hd(img->width, img->height);//判断是否为高清
	//mvcarnumdetect.CalibrationSet(NULL);//输入相机号
#endif 

	int nCarNumResult = mvcarnumdetect.find_carnum(NULL,img,tmp,&tmpimg,rtCarnumROI,1,vehicle_result,&context,NULL,m_LoopParmarer);

	if( (nCarNumResult != 1) && (pRectAtDelect != NULL))
	{
		rtCarnumROI.x= 0;
		rtCarnumROI.y = m_imgSnap->height/4;
		rtCarnumROI.width = m_imgSnap->width;
		rtCarnumROI.height = m_imgSnap->height*3/4;
		nCarNumResult = mvcarnumdetect.find_carnum(NULL,img,tmp,&tmpimg,rtCarnumROI,1,vehicle_result,&context,NULL,m_LoopParmarer);
	}

	LogNormal("-------nCarNumResult:%d", nCarNumResult);

	if(tmpimg!=NULL)
	{
		cvReleaseImage(&tmpimg);
		tmpimg = NULL;
	}

	if(img!=NULL)
	{
		cvReleaseImageHeader(&img);
		img = NULL;
	}
	
	int nCarNumWidth = 0;
	for(int i = 0;i < nCarNumResult;i++)
	{
		bool bCarNum = true;

		if( (vehicle_result[i].carnum[0] == '*') || (vehicle_result[i].carnum[0] == '$') )
        {
            bCarNum = false;
        }

        if(bCarNum)
		{
			if(nCarNumWidth <= vehicle_result[i].position.width)
			{
				strCarnum.clear();
				strCarnum.append(vehicle_result[i].carnum,7);
				//车牌号码转换
				CarNumConvert(strCarnum,vehicle_result[i].wjcarnum);

				nCarNumWidth = vehicle_result[i].position.width;
			}
		}
	}

	return strCarnum;
}

//获取图像坐标到世界坐标的变换矩阵
void CSkpRoadDetect::mvfind_homography(float *image, float *world, float homography_image_to_world[3][3])
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


int CSkpRoadDetect::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}

void CSkpRoadDetect::SetCameraHostAndWidthHeight(int iWidth,int iHeight, std::string strCameraHost,int nCameraPort)
{
	printf("CSkpRoadDetect::SetCameraHostAndWidthHeight: iWidth=%d,iHeight=%d,strCameraHost=%s:%d,m_nCameraType:%d \n",iWidth,iHeight,strCameraHost.c_str(),nCameraPort,m_nCameraType);
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_strCameraHost = strCameraHost;
	m_nCameraPort = nCameraPort;
#ifdef CAMERAAUTOCTRL
	m_pOnvifCameraCtrl->Init(m_nCameraType,m_strCameraHost.c_str(),m_nCameraPort);
	if(m_nCameraType == BOCOM_301_200)
	{
		m_pRoadDetectPelco->InputCameraAddr(m_strCameraHost.c_str(),m_nCameraPort);
	}
	else if(m_nCameraType == DH_CAMERA)
	{
		m_pRoadDetectDHCamera->InputCameraAddr(m_strCameraHost.c_str(),m_nCameraPort);
	}
#endif
}

void CSkpRoadDetect::DetectRegionRect(int xPos,int yPos,int width,int height,bool FromClient)
{
	printf("CSkpRoadDetect::DetectRegionRect,xPos=%d,yPos=%d,width=%d,height=%d \n",xPos,yPos,width,height);
	if(FromClient)
	{
		m_bDetectClientRect = true;
	}
	else
	{
		m_bDetectClientRect = false;
	}
#ifdef CAMERAAUTOCTRL
	if(m_nCameraType == BOCOM_301_200)
	{
		m_pRoadDetectPelco->DetectRegionRect(m_iWidth,m_iHeight,xPos,yPos,width,height);
	}
	else if(m_nCameraType == DH_CAMERA)
	{
		m_pRoadDetectDHCamera->DetectRegionRect(m_iWidth,m_iHeight,xPos,yPos,width,height);
	}
#endif
}

void CSkpRoadDetect::DetectParkObjectsRect(UINT32 uDetectCarID,UINT32 uMsgCommandID,int xPos,int yPos,int width,int height,bool FromClient)
{
	printf("CSkpRoadDetect::DetectParkObjectsRect,xPos=%d,yPos=%d,width=%d,height=%d\n",xPos,yPos,width,height);
#ifdef  HANDCATCHCAREVENT
	if(uMsgCommandID == DETECT_ADD_REGION_RECT)//增加一个目标区域
	{
		HandCatctEventInfo tParkRect;
		tParkRect.nX = xPos;
		tParkRect.nY = yPos;
		tParkRect.nWidth = width;
		tParkRect.nHeight = height;
		pthread_mutex_lock(&m_DetectCarParkRect_Mutex);
		m_mapDetectCarParkRect.insert(make_pair(GetTimeT(),tParkRect));
		pthread_mutex_unlock(&m_DetectCarParkRect_Mutex);
		return;
	}
	else if(uMsgCommandID == DETECT_DEL_REGION_RECT)//删除一个目标区域
	{
		pthread_mutex_lock(&m_DetectCarParkRect_Mutex);
		m_pHandCatchEvent->DelObjRect(uDetectCarID);
		pthread_mutex_unlock(&m_DetectCarParkRect_Mutex);
		std::string frame;
		CvRect rect;
		rect.x = 0;
		DetectHandEvent(frame,uDetectCarID+1000000,rect,3);
		return;
	}
#endif
	
}


void CSkpRoadDetect::CreateDealClientRectMsgThread()
{
	pthread_t m_DealRectId;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //设置线程分离
	if(pthread_create(&m_DealRectId, NULL, DealClientRectMsgFunc, this) != 0)
	{
		pthread_attr_destroy(&attr);
		printf("Start Thread Failed. \n");
		return ;
	}
	pthread_attr_destroy(&attr);
	return;

}
void* CSkpRoadDetect::DealClientRectMsgFunc(void * lpParam)
{
	CSkpRoadDetect *pRoadDetect = (CSkpRoadDetect *)lpParam;
	if(pRoadDetect)
	{
		pRoadDetect->DealClientRectMsg();
	}
	return NULL;
}
void* CSkpRoadDetect::DealClientRectMsg()
{
	sleep(20);
	int m_CameraMoveTime = 0;
	m_nGotoPresetTime = GetTimeT();
	int nCameraPresetCount = 0;
	int nNowPreset = g_skpDB.GetPreSet(m_nChannelID);
	int nLiveGotoPresetTime =  GetTimeT();
	while(m_bDealClientRectRequest)
	{
		if(m_nCameraType == DH_CAMERA)
		{
			if(m_bDetectClientRect)
			{
				//弹出一条帧图片
				std::string result;
				if(PopRealFrame(result))
				{
					DealDetectRect(result);
					m_bDetectClientRect = false;
				}
				else
				{
					usleep(20*1000);
				}

			}
			else
			{
				if(!m_bCameraAutoCtrl)
				{
					if (abs((GetTimeT() - nLiveGotoPresetTime)) > 600 )
					{
						nLiveGotoPresetTime = GetTimeT();
#ifdef CAMERAAUTOCTRL
						m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
#endif
					}
					if (abs((GetTimeT() - m_CameraMoveTime)) > 10 )
					{
						m_CameraMoveTime = GetTimeT();
						int nCameraStatus = CalculatePTZPosition(nNowPreset);
						if(nCameraStatus == 1)
						{
							m_bCameraMoveFormOp = true;
						}
						else if(nCameraStatus == 0)
						{
							m_bCameraMoveFormOp = false;
							m_nGotoPresetTime = GetTimeT();
							nCameraPresetCount = 0;
						}
						else if(nCameraStatus == 2)
						{
							nCameraPresetCount++;
							m_bCameraMoveFormOp = true;
						}
					}

					if(m_bCameraMoveFormOp)
					{
						if( abs((int)GetTimeT() - m_nGotoPresetTime) > 300 )
						{
							m_nGotoPresetTime = (int)GetTimeT();
#ifdef CAMERAAUTOCTRL
							m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
#endif
							sleep(2);
							m_bCameraMoveFormOp = false;
						}
				}

					if(nCameraPresetCount > 18)
					{
						LogNormal("Delete Restart,Perset No : %d..........\n",nNowPreset);
						m_bCameraMoveFormOp = false;
						nCameraPresetCount = 0;
						g_skpDB.SetPreSet(nNowPreset,m_nChannelID);
						sleep(1);
						g_skpChannelCenter.ReloadDetect(m_nChannelID);
						sleep(10);
					}

				}
		     }
		}
		else if(m_nCameraType == BOCOM_301_200)
		{
			//博康云台一体机停车严管
			if(m_bDetectClientRect)
			{
				//弹出一条帧图片
				std::string result;
				if(PopFrame(result))
				{
					//无检测帧
					if(result.size() == 0)
					{
						usleep(10*1000);
						continue;
					}
					DealDetectRect(result);
					m_bDetectClientRect = false;
				}
				else
				{
					usleep(20*1000);
				}

			}
			else
			{
				if(!m_bCameraAutoCtrl)
				{
					if (abs((GetTimeT() - m_nGotoPresetTime)) > 300 )
					{
#ifdef CAMERAAUTOCTRL
						m_pRoadDetectPelco->GotoPreset(1);
#endif
						m_nGotoPresetTime = GetTimeT();
					}
				}
			}
		}
		usleep(100*1000);
	}
	return NULL;
}



void CSkpRoadDetect::DealDetectRect(std::string& frame)
{
	printf("CSkpRoadDetect::DealDetectRect \n");
	DealTrackAutoRect(frame,-1,4);
}

int CSkpRoadDetect::DealTrackAutoRect(std::string& response,int nCarId,int nSaveProcess)
{
	if(m_nCameraType == DH_CAMERA)
	{
		//大华球机版本停车严管
		
		return DealTrackAutoRectDHCamera(response,nCarId,nSaveProcess);
	}
	else if(m_nCameraType == BOCOM_301_200)
	{
		//博康云台一体机停车严管
		return DealTrackAutoRectBocomPTZ(response,nCarId,nSaveProcess);
	}
	return 0;
}

int CSkpRoadDetect::GetPTZPresetStatus()
{
#ifdef CAMERAAUTOCTRL
	AutoCameraPtzTrack *m_pPresetInfo;
	int nPresetCount = 0;
	m_pOnvifCameraCtrl->PTZGetPreset(m_pPresetInfo,nPresetCount);
	if(nPresetCount <= 0)
	{
		return 0;
	}
	for(int i=0; i< nPresetCount; i++)
	{
		AutoCameraPtzTrack *pAutoCameraPtzTrack = new AutoCameraPtzTrack;
		pAutoCameraPtzTrack->nPanTiltX    = m_pPresetInfo[i].nPanTiltX;
		pAutoCameraPtzTrack->nPanTiltY    = m_pPresetInfo[i].nPanTiltY;
		pAutoCameraPtzTrack->nPanTiltZoom = m_pPresetInfo[i].nPanTiltZoom;
		pthread_mutex_lock(&m_Preset_Mutex);
		m_CameraPtzPresetStatus.insert(make_pair(m_pPresetInfo[i].nTaken,pAutoCameraPtzTrack));
		pthread_mutex_unlock(&m_Preset_Mutex);
		printf("PresetId=%d,nPanTiltX=%d,nPanTiltY=%d,nPanTiltZoom=%d \n",m_pPresetInfo[i].nTaken,pAutoCameraPtzTrack->nPanTiltX,pAutoCameraPtzTrack->nPanTiltY,pAutoCameraPtzTrack->nPanTiltZoom);
	}
	delete m_pPresetInfo;
#endif
	return 0;
}


int CSkpRoadDetect::CalculatePTZPosition(int &nPreset)
{
	string strPanTiltX;
	string strPanTiltY;
	string strZoomX;
	nPreset = g_skpDB.GetPreSet(m_nChannelID);
#ifdef CAMERAAUTOCTRL
	if(m_pOnvifCameraCtrl->GetPTZStatus(strPanTiltX,strPanTiltY,strZoomX))
	{
		pthread_mutex_lock(&m_Preset_Mutex);
		if(m_CameraPtzPresetStatus.size() == 0)
		{
			LogNormal("no preset..........\n");
			pthread_mutex_unlock(&m_Preset_Mutex);
			return 2;
		}
		for(mapAutoCameraPtzTrack::iterator iter=m_CameraPtzPresetStatus.begin();iter!=m_CameraPtzPresetStatus.end();iter++)
		{
			printf("PresetNum=%d,PresetId=%d    ",m_nPresetNum,iter->first);
			printf("x=%d  ",abs(iter->second->nPanTiltX - (int)(atof(strPanTiltX.c_str())*1000 )));
			printf("y=%d  ",abs(iter->second->nPanTiltY - (int)(atof(strPanTiltY.c_str())*1000 )));
			printf("z=%d  \n",abs(iter->second->nPanTiltZoom - (int)(atof(strZoomX.c_str())*1000 )));


			if( (abs(iter->second->nPanTiltX - (int)(atof(strPanTiltX.c_str())*1000 )) <= 10 ) && 
				(abs(iter->second->nPanTiltY - (int)(atof(strPanTiltY.c_str())*1000 )) <= 10 ) && 
				(abs(iter->second->nPanTiltZoom - (int)(atof(strZoomX.c_str())*1000 )) <= 10 ) )
			{
				if(iter->first == nPreset)
				{
					pthread_mutex_unlock(&m_Preset_Mutex);
					return 0;
				}
				else
				{
					nPreset = iter->first;
					pthread_mutex_unlock(&m_Preset_Mutex);
					return 2;
				}
			}
		}
		pthread_mutex_unlock(&m_Preset_Mutex);
	}
	else
	{
		return 0;
	}
#endif
	return 1;
}



bool CSkpRoadDetect::PopRealFrame(std::string& response)
{
	while(1)
	{
		if(PopFrame(response))
		{
			if(response.size() > 1024*1024)
			{
				SRIP_DETECT_HEADER szDetectHeader = *((SRIP_DETECT_HEADER*)response.c_str());
				//黑图，不做处理
				if(szDetectHeader.uTimestamp == 0)
				{
					continue;
				}
				break;
			}
		}
		else
		{
			usleep(5*1000);
		}
	}
	return true;
}

bool CSkpRoadDetect::DetectHandEvent(std::string frame,int nCarId,CvRect rect,int nPicNum)
{
	if(nPicNum == 3)
	{
		m_mapRemotePicList.erase(nCarId);
		return true;
	}

	if(nPicNum == 1)
	{
		LogNormal("nCarId:%d \n",nCarId);
		if(m_mapVideoTime.find( nCarId) == m_mapVideoTime.end())
		{
			m_mapVideoTime.insert(make_pair(nCarId,GetTimeT()));
		}
		else
		{
			m_mapVideoTime[nCarId] = GetTimeT();
		}

		printf("CSkpRoadDetect::OutPutAutoCtrlResult: SAVE_REMO_NEAR_PIC_NOOUT\n");
		if(m_nExtent > 0)
		frame.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));   

		char *szPicName = new char[100];
		{
			memset(szPicName,0,100);
			struct timeval tv;
			gettimeofday(&tv,NULL);
			sprintf(szPicName,"/detectdata/eventpic/1-%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
			LogNormal("pic1: %s \n",szPicName);
		}
		FILE *pPicFile = fopen(szPicName,"w+");
		if(pPicFile)
		{
			fwrite(frame.c_str(),1,frame.size(),pPicFile);
			fclose(pPicFile);
		}

		list<unsigned char *> listTemp;
		listTemp.push_back((unsigned char *)szPicName);
		m_mapRemotePicList.insert(make_pair(nCarId, listTemp));

		DetectRegionRect(rect.x,rect.y,rect.width,rect.height,false);
		string nearPic;
		m_bCameraAutoCtrl = true;
		if(DealTrackAutoRect(nearPic,nCarId,1) == -1)
		{
			m_bCameraAutoCtrl = false;
			m_mapRemotePicList.erase(nCarId);
			return false;
		}
		m_bCameraAutoCtrl = false;
		if(m_nExtent > 0)
		nearPic.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));     //文字区域叠加在图片下方
		if(nearPic.size() > 0)                // 存图
		{
			char *szPicName = new char[100];
			{
				memset(szPicName,0,100);
				struct timeval tv;
				gettimeofday(&tv,NULL);
				sprintf(szPicName,"/detectdata/eventpic/2-%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
				LogNormal("pic2: %s \n",szPicName);
			}
			FILE *pPicFile = fopen(szPicName,"w+");
			if(pPicFile)
			{
				fwrite(nearPic.c_str(),1,nearPic.size(),pPicFile);
				fclose(pPicFile);
			}
			std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(nCarId); 
			if (iter2 != m_mapRemotePicList.end())
			{
				iter2->second.push_back((unsigned char *)szPicName);
			}
		}
		return true;
	}
	else
	{
		if(m_mapRemotePicList.find(nCarId) == m_mapRemotePicList.end())
		{
			return false;
		}
		LogNormal("nCarId:%d \n",nCarId);
		if(m_nExtent > 0)
		frame.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));   

		char *szPicName = new char[100];
		{
			memset(szPicName,0,100);
			struct timeval tv;
			gettimeofday(&tv,NULL);
			sprintf(szPicName,"/detectdata/eventpic/3-%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
			printf("pic3: %s \n",szPicName);
		}
		FILE *pPicFile = fopen(szPicName,"w+");
		if(pPicFile)
		{
			fwrite(frame.c_str(),1,frame.size(),pPicFile);
			fclose(pPicFile);
		}
		std::map<int, list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(nCarId); 
		iter->second.push_back((unsigned char *)szPicName);


		DetectRegionRect(rect.x,rect.y,rect.width,rect.height,false);
		string fourthStr;
		m_bCameraAutoCtrl = true;
		DealTrackAutoRect(fourthStr,nCarId,2);
		m_bCameraAutoCtrl = false;
		
		if(m_nExtent > 0)
		fourthStr.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));   //文字区域叠加在图片下方

		{
			char *szPicName = new char[100];
			{
				memset(szPicName,0,100);
				struct timeval tv;
				gettimeofday(&tv,NULL);
				sprintf(szPicName,"/detectdata/eventpic/4-%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
				printf("pic4: %s \n",szPicName);
			}
			FILE *pPicFile = fopen(szPicName,"w+");
			if(pPicFile)
			{
				fwrite(fourthStr.c_str(),1,fourthStr.size(),pPicFile);
				fclose(pPicFile);
			}

			std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(nCarId); 
			if (iter2 != m_mapRemotePicList.end())
			{
				iter2->second.push_back((unsigned char *)szPicName);
			}
		}


		printf("fourthStr size:%d \n",(int)fourthStr.size());
		string strCarNum = CarNumDetect(fourthStr);                         // 车牌的识别并把车牌号保存到event.chText;
		//memcpy(event.chText,strCarNum.c_str() , strCarNum.length());
		printf("--车牌号为：%s\n", strCarNum.c_str());

		RECORD_EVENT tRecordEvent;
		tRecordEvent.uCode = DETECT_RESULT_EVENT_STOP;
		sprintf(tRecordEvent.chText,"%s",strCarNum.c_str());
		ComPoseImage(tRecordEvent,nCarId);
		//删除图片
		{
			std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(nCarId); 

			if (iter != m_mapRemotePicList.end())
			{
				for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
				{
					if(*iter2 != NULL)
					{
						char szComm[512] = {0};
						sprintf(szComm,"rm -rf %s",*iter2);
						printf("Comm: %s \n",szComm);
						system(szComm);
						delete *iter2;
						*iter2 = NULL;
					}

				}
				iter->second.clear();
				m_mapRemotePicList.erase(iter);
			}
		}
		SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)fourthStr.c_str());
		SaveEventRecord(tRecordEvent,nCarId,sDetectHeader.uChannelID);

		std::string strEventPlate;
		strEventPlate.append((char*)&tRecordEvent,sizeof(RECORD_EVENT));
		sDetectHeader.uDetectType = SRIP_DETECT_EVENT;
		strEventPlate.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));

		if(m_bConnect)
		{
			g_skpChannelCenter.AddResult(strEventPlate);
		}

	}


	return true;
}

void CSkpRoadDetect::CatchCarEventCB(int nCarId, int nContext)
{
	printf("cb,nCarId=%d, nContext=%d\n",nCarId,nContext);
	CSkpRoadDetect *pThis = (CSkpRoadDetect*)nContext;
	if(pThis)
	{
		pThis->DealCatchCarEvent(nCarId);
	}
	
	return;
}

void CSkpRoadDetect::DealCatchCarEvent(int nCarId)
{
	std::string frame;
	PopRealFrame(frame);
	CvRect rectDraw;
	rectDraw.x = 0;
	DetectHandEvent(frame,nCarId,rectDraw,2);
}

#ifdef  HANDCATCHCAREVENT
void CSkpRoadDetect::CatchCarEventDrawCB(vector<RectObject>  vectRectObj ,SRIP_DETECT_HEADER tDetectHeader, int nContext)
{
	CSkpRoadDetect *pThis = (CSkpRoadDetect*)nContext;
	if(pThis)
	{
		pThis->DealCatchCarEventDraw(vectRectObj,tDetectHeader);
	}

	return;
}

void CSkpRoadDetect::DealCatchCarEventDraw(vector<RectObject>  vectRectObj,SRIP_DETECT_HEADER tDetectHeader)
{
	printf("CSkpRoadDetect::DealCatchCarEventDraw \n");

	tDetectHeader.uDetectType = SRIP_OBJECT_STATUS;
	std::string strHandEvent;
	strHandEvent.insert(0,(char*)&tDetectHeader,sizeof(SRIP_DETECT_HEADER));

	for(vector<RectObject>::iterator iter=vectRectObj.begin();iter!=vectRectObj.end();iter++)
	{
		int nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,(char*)&(*iter).nId,sizeof(int));

		nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,(char*)&(*iter).lpRect.left,sizeof(int));


		nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,(char*)&(*iter).lpRect.top,sizeof(int));

		nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,(char*)&(*iter).lpRect.right,sizeof(int));


		nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,(char*)&(*iter).lpRect.bottom,sizeof(int));

		nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,(char*)&(*iter).bGetRoof,sizeof(int));

		nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,(char*)&(*iter).bLeave,sizeof(int));
		
		
		char szTmp[32] = {0};
		nOffsetTmp = strHandEvent.length();
		strHandEvent.insert(nOffsetTmp,szTmp,32);


	}
	printf("bodylength:%d \n",(int)strHandEvent.length());
	if(m_bConnect)
	{
		g_skpChannelCenter.AddResult(strHandEvent);
	}
}
#endif

bool CSkpRoadDetect::DetectAutoEventStop(std::string frame,SRIP_DETECT_OUT_RESULT *pResult,RECORD_EVENT &event)
{
	SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

	if (g_ytControlSetting.nPicComPoseMode == 1)                                      //  保存方式2远景2远近的情况   
	{
		if ( pResult->nOutPutMode == NORMOAL_OUT_MODE )                                  // 正常模式输出
		{
			printf("CSkpRoadDetect::OutPutAutoCtrlResult: NORMOAL_OUT_MODE\n");
			//防止算法误报
			if(m_bEventCapture)
			{
				if(m_mapAutoCameraRecord.find(pResult->nCarId) == m_mapAutoCameraRecord.end())
				{
					m_mapAutoCameraRecord.erase(pResult->nCarId);
					return false;
				}
			}
			char *szPicName = new char[50];
			{
				memset(szPicName,0,50);
				struct timeval tv;
				gettimeofday(&tv,NULL);
				sprintf(szPicName,"/detectdata/eventpic/%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
				printf("pic: %s \n",szPicName);
			}
			FILE *pPicFile = fopen(szPicName,"w+");
			if(pPicFile)
			{
				fwrite(frame.c_str(),1,frame.size(),pPicFile);
				fclose(pPicFile);
			}
			std::map<int, list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 
			if (iter != m_mapRemotePicList.end())
			{
				iter->second.push_back((unsigned char *)szPicName);
			}
			else
			{
				return false;
			}

			DetectRegionRect(pResult->carRectOfSrcImg.x,pResult->carRectOfSrcImg.y,pResult->carRectOfSrcImg.width,pResult->carRectOfSrcImg.height,false);
			string fourthStr;
			m_bCameraAutoCtrl = true;
			DealTrackAutoRect(fourthStr,pResult->nCarId,2);
			m_bCameraAutoCtrl = false;

			event.uVideoEndTime = GetTimeT()+60;
			LogNormal("event.uVideoEndTime = %d \n",event.uVideoEndTime);
			if(m_nExtent > 0)
			fourthStr.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));   //文字区域叠加在图片下方
			if(fourthStr.size() > 0)
			{
				char *szPicName = new char[50];
				{
					memset(szPicName,0,50);
					struct timeval tv;
					gettimeofday(&tv,NULL);
					sprintf(szPicName,"/detectdata/eventpic/%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
					printf("pic: %s \n",szPicName);
				}
				FILE *pPicFile = fopen(szPicName,"w+");
				if(pPicFile)
				{
					fwrite(fourthStr.c_str(),1,fourthStr.size(),pPicFile);
					fclose(pPicFile);
				}

				std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(pResult->nCarId); 
				if (iter2 != m_mapRemotePicList.end())
				{
					iter2->second.push_back((unsigned char *)szPicName);
				}
			}
			printf("fourthStr size:%d \n",(int)fourthStr.size());
			string strCarNum = CarNumDetect(fourthStr);                         // 车牌的识别并把车牌号保存到event.chText;
			
			//如果第4张图未识别出车牌则去识别第2张图
			if(strCarNum.size () <= 0)
			{
				std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 
				
				if (iter != m_mapRemotePicList.end())
				{
					int nIndex = 0;
					for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
					{
						//取第二张图片
						if(nIndex == 1)
						{
							char szPicName[50] = {0};
							sprintf(szPicName,"%s",*iter2);
							
							string strPicPath(szPicName);
							string strPic = GetImageByPath(strPicPath);
							
							if(strPic.size() > 0)
							{
								strCarNum = CarNumDetect(strPic);  
							}
							LogNormal("CarNumDetect twice: %s\n",strCarNum.c_str());

							break;
						}
						nIndex++;
					}
				}
			}

			memcpy(event.chText,strCarNum.c_str() , strCarNum.length());
			LogNormal("车牌号为：%s\n", strCarNum.c_str());


			//判断车牌是否存在，如果之前已经存在则不输出
			#ifdef CAMERAAUTOCTRL
			if(strCarNum.size () > 0)
			{
				if(!m_Rdetect.mvAddCarnumResult4StopDet(pResult->nCarId,&event.chText[2],GetTimeStamp()))
				{
					//删除图片
					{
						std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 

						if (iter != m_mapRemotePicList.end())
						{
							for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
							{
								if(*iter2 != NULL)
								{
									char szComm[512] = {0};
									sprintf(szComm,"rm -rf %s",*iter2);
									printf("Comm: %s \n",szComm);
									system(szComm);
									delete *iter2;
									*iter2 = NULL;
								}

							}
							iter->second.clear();
							m_mapRemotePicList.erase(iter);
						}
					}
					return false;
				}
			}
			#endif

			SRIP_DETECT_HEADER szDetectHeader = *((SRIP_DETECT_HEADER*)fourthStr.c_str());
			LogNormal("szDetectHeader.uTimestamp = %d \n",szDetectHeader.uTimestamp);
			//黑图，不做处理
			if(szDetectHeader.uTimestamp == 0)
			{
				printf("block pic \n");
				return false;
			}


			if(m_bEventCapture)
			{
				std::string strEventInfo;
				char pEventInfo[100] = {0};
				SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
				pEventHead->uSeq = 0; //用于录像路径的计算
				pEventHead->uChannelID = sDetectHeader.uChannelID;
				pEventHead->uTimestamp = sDetectHeader.uTimestamp;
				pEventHead->uRealTime = 2;
				pEventHead->uTrafficSignal = pResult->nCarId;
				strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
				g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
			}

		}
		else if (pResult->nOutPutMode == SAVE_REMO_NEAR_PIC_NOOUT)              //存一组远/近景组合图，不输出
		{
			//LogNormal("---存一组图片_远景 id=%d ,size=%d, ",pResult->nCarId, frame.size());
			if(m_mapVideoTime.find( pResult->nCarId) == m_mapVideoTime.end())
			{
				m_mapVideoTime.insert(make_pair(pResult->nCarId,GetTimeT()-60));
			}
			else
			{
				m_mapVideoTime[pResult->nCarId] = GetTimeT()-60;
			}


			event.uVideoBeginTime = GetTimeT()-60;
			LogNormal("event.uVideoBeginTime = %d \n",event.uVideoBeginTime);
			printf("CSkpRoadDetect::OutPutAutoCtrlResult: SAVE_REMO_NEAR_PIC_NOOUT\n");
			char *szPicName = new char[50];
			{
				memset(szPicName,0,50);
				struct timeval tv;
				gettimeofday(&tv,NULL);
				sprintf(szPicName,"/detectdata/eventpic/%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
				LogNormal("pic1: %s \n",szPicName);
			}
			FILE *pPicFile = fopen(szPicName,"w+");
			if(pPicFile)
			{
				fwrite(frame.c_str(),1,frame.size(),pPicFile);
				fclose(pPicFile);
			}

			std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 
			if (iter == m_mapRemotePicList.end())
			{
				list<unsigned char *> listTemp;
				listTemp.push_back((unsigned char *)szPicName);
				m_mapRemotePicList.insert(make_pair(pResult->nCarId, listTemp));
			}
			else
			{
				iter->second.push_back((unsigned char *)szPicName);
			}

										
			if(m_bEventCapture)
			{
				std::string strEventInfo;
				char pEventInfo[100] = {0};
				SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
				if(m_mapAutoCameraRecord.find(pResult->nCarId) == m_mapAutoCameraRecord.end())
				{
					m_mapAutoCameraRecord.insert(make_pair(pResult->nCarId,m_nVideoNum));
				}
				else
				{
					m_mapAutoCameraRecord[pResult->nCarId] = m_nVideoNum;
				}

				g_FileManage.CheckDisk(true,true);
				pEventHead->uSeq =  m_nVideoNum++; //用于录像路径的计算
				pEventHead->uChannelID = sDetectHeader.uChannelID;
				pEventHead->uTimestamp = sDetectHeader.uTimestamp;
				pEventHead->uRealTime = 0;
				pEventHead->uTrafficSignal = pResult->nCarId;
				strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
				g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
			}
										
			DetectRegionRect(pResult->carRectOfSrcImg.x,pResult->carRectOfSrcImg.y,pResult->carRectOfSrcImg.width,pResult->carRectOfSrcImg.height,false);
			string nearPic;
			m_bCameraAutoCtrl = true;
			if(DealTrackAutoRect(nearPic,pResult->nCarId,1) == -1)
			{
				m_bCameraAutoCtrl = false;

				m_mapRemotePicList.erase(pResult->nCarId);

				return false;
			}
			m_bCameraAutoCtrl = false;
			if(m_nExtent > 0)
			nearPic.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));     //文字区域叠加在图片下方
			if(nearPic.size() > 0)                // 存图
			{
				char *szPicName = new char[50];
				{
					memset(szPicName,0,50);
					struct timeval tv;
					gettimeofday(&tv,NULL);
					sprintf(szPicName,"/detectdata/eventpic/%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
					LogNormal("pic2: %s \n",szPicName);
				}
				FILE *pPicFile = fopen(szPicName,"w+");
				if(pPicFile)
				{
					fwrite(nearPic.c_str(),1,nearPic.size(),pPicFile);
					fclose(pPicFile);
				}
				std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(pResult->nCarId); 
				if (iter2 != m_mapRemotePicList.end())
				{
					iter2->second.push_back((unsigned char *)szPicName);
				}
			}
			return false;
		} 
		else if ( pResult->nOutPutMode ==  DELETE_CACHE_PIC_NOOUT)        //删除缓存的图像，不输出
		{
			//LogNormal("----删除缓存图片--id = %d--\n", pResult->nCarId);
			LogNormal("DELETE_CACHE_PIC_NOOUT:%d\n",pResult->nCarId);

			if(m_mapVideoTime.find( pResult->nCarId) != m_mapVideoTime.end())
			{
				m_mapVideoTime.erase(pResult->nCarId);
			}


			string nearPic;
			m_bCameraAutoCtrl = true;
			DealTrackAutoRect(nearPic,pResult->nCarId,3); 
			m_bCameraAutoCtrl = false;

			if(m_bEventCapture)
			{
				if(m_mapAutoCameraRecord.find(pResult->nCarId) != m_mapAutoCameraRecord.end())
				{
					m_mapAutoCameraRecord.erase(pResult->nCarId);
				}
				std::string strEventInfo;
				char pEventInfo[100] = {0};
				SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
				pEventHead->uSeq = 0;
				pEventHead->uChannelID = sDetectHeader.uChannelID;
				pEventHead->uTimestamp = sDetectHeader.uTimestamp;
				pEventHead->uRealTime = 1;
				pEventHead->uTrafficSignal = pResult->nCarId;
				strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
				g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
			}


			std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 
			if (iter != m_mapRemotePicList.end())
			{
				for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
				{
					if(*iter2 != NULL)
					{
						char szComm[512] = {0};
						sprintf(szComm,"rm -rf %s",*iter2);
						printf("Comm: %s \n",szComm);
						system(szComm);
						delete *iter2;
						*iter2 = NULL;
					}
												
				}
				iter->second.clear();
				m_mapRemotePicList.erase(iter);
			}
			return false;
		}
	}
	else if ( (g_ytControlSetting.nPicComPoseMode == 0)|| (g_ytControlSetting.nPicComPoseMode == 3) )               //表示现在的方式 2*2 格式的 三张远景一张近景
	{
		if(pResult->nOutPutMode == JUST_SAVE_REMO_PIC_NOOUT)           // 1 /只存图不输出结果
		{
			printf("CSkpRoadDetect::OutPutAutoCtrlResult: JUST_SAVE_REMO_PIC_NOOUT\n");
			if(m_bEventCapture)
			{
				std::string strEventInfo;
				char pEventInfo[100] = {0};
				SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;

				if(m_mapAutoCameraRecord.find(pResult->nCarId) == m_mapAutoCameraRecord.end())
				{
					m_mapAutoCameraRecord.insert(make_pair(pResult->nCarId,m_nVideoNum));

					g_FileManage.CheckDisk(true,true);
					pEventHead->uSeq =  m_nVideoNum++; //用于录像路径的计算
					pEventHead->uChannelID = sDetectHeader.uChannelID;
					pEventHead->uTimestamp = sDetectHeader.uTimestamp;
					pEventHead->uRealTime = 0;
					pEventHead->uTrafficSignal = pResult->nCarId;
					strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
					g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
				}
			}

			char *szPicName = new char[50];
			{
				memset(szPicName,0,50);
				struct timeval tv;
				gettimeofday(&tv,NULL);
				sprintf(szPicName,"/detectdata/eventpic/%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
				printf("pic: %s \n",szPicName);
			}
			FILE *pPicFile = fopen(szPicName,"w+");
			if(pPicFile)
			{
				fwrite(frame.c_str(),1,frame.size(),pPicFile);
				fclose(pPicFile);
			}			

			std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 

			if (iter == m_mapRemotePicList.end())
			{
				list<unsigned char *> listTemp;
				listTemp.push_back((unsigned char *)szPicName);
				m_mapRemotePicList.insert(make_pair(pResult->nCarId, listTemp));
			}
			else
			{
				iter->second.push_back((unsigned char *)szPicName);
			}
			
			return false;
		}
		else if (pResult->nOutPutMode == DELETE_CACHE_PIC_NOOUT)      // 2车子已经离开，需要清除之前保存的图片
		{
			printf("CSkpRoadDetect::OutPutAutoCtrlResult: DELETE_CACHE_PIC_NOOUT\n");
			if(m_bEventCapture)
			{
				if(m_mapAutoCameraRecord.find(pResult->nCarId) != m_mapAutoCameraRecord.end())
				{
					m_mapAutoCameraRecord.erase(pResult->nCarId);
				}

				std::string strEventInfo;
				char pEventInfo[100] = {0};
				SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
				pEventHead->uSeq = 0;
				pEventHead->uChannelID = sDetectHeader.uChannelID;
				pEventHead->uTimestamp = sDetectHeader.uTimestamp;
				pEventHead->uRealTime = 1;
				pEventHead->uTrafficSignal = pResult->nCarId;
				strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
				g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
			}

			std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 

			if (iter != m_mapRemotePicList.end())
			{
				for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
				{
					if(*iter2 != NULL)
					{
						char szComm[512] = {0};
						sprintf(szComm,"rm -rf %s",*iter2);
						printf("Comm: %s \n",szComm);
						system(szComm);
						delete *iter2;
						*iter2 = NULL;
					}
												
				}
				iter->second.clear();
				m_mapRemotePicList.erase(iter);
			}
			
			return false;
		}
		else if (pResult->nOutPutMode == NORMOAL_OUT_MODE)
		{
			//防止算法出现同一目标报两次的情况
			if(m_bEventCapture)
			{
				if(m_mapAutoCameraRecord.find(pResult->nCarId) == m_mapAutoCameraRecord.end())
				{
					return false;
				}
			}

			//存第三张图片
			printf("CSkpRoadDetect::OutPutAutoCtrlResult: NORMOAL_OUT_MODE\n");

			char *szPicName = new char[50];
			{
				memset(szPicName,0,50);
				struct timeval tv;
				gettimeofday(&tv,NULL);
				sprintf(szPicName,"/detectdata/eventpic/%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
				printf("pic: %s \n",szPicName);
			}
			FILE *pPicFile = fopen(szPicName,"w+");
			if(pPicFile)
			{
				fwrite(frame.c_str(),1,frame.size(),pPicFile);
				fclose(pPicFile);
			}			

			std::map<int, list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 
			if (iter != m_mapRemotePicList.end())
			{
				iter->second.push_back((unsigned char *)szPicName);
			}
			
			DetectRegionRect(pResult->carRectOfSrcImg.x,pResult->carRectOfSrcImg.y,pResult->carRectOfSrcImg.width,pResult->carRectOfSrcImg.height,false);
			string fourthStr;
			m_bCameraAutoCtrl = true;
			if(DealTrackAutoRect(fourthStr,pResult->nCarId) == -1)
			{
				printf("Check err \n");
				m_bCameraAutoCtrl = false;
				return false;
			}
			// 拉回远景之后判断车是否已经离开。
			if (CheckLastPic(pResult->nCarId)!=0)
			{
				m_bCameraAutoCtrl = false;
				return false;
			}

			m_bCameraAutoCtrl = false;

			printf("addr2 = %x \n",&fourthStr);

			SRIP_DETECT_HEADER szDetectHeader = *((SRIP_DETECT_HEADER*)fourthStr.c_str());
			LogNormal("szDetectHeader.uTimestamp = %d \n",szDetectHeader.uTimestamp);
			//黑图，不做处理
			if(szDetectHeader.uTimestamp == 0)
			{
				printf("block pic \n");
				return false;
			}
			if(m_nExtent > 0)
			fourthStr.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));	//文字区域叠加在图片下方					
			if(fourthStr.size() > 0)              
			{
				char *szPicName = new char[50];
				{
					memset(szPicName,0,50);
					struct timeval tv;
					gettimeofday(&tv,NULL);
					sprintf(szPicName,"/detectdata/eventpic/%d.%d",(int)tv.tv_sec,(int)tv.tv_usec);
					printf("pic: %s \n",szPicName);
				}
				FILE *pPicFile = fopen(szPicName,"w+");
				if(pPicFile)
				{
					fwrite(fourthStr.c_str(),1,fourthStr.size(),pPicFile);
					fclose(pPicFile);
				}

				std::map<int, list<unsigned char*> >::iterator iter2 = m_mapRemotePicList.find(pResult->nCarId); 
				if (iter2 != m_mapRemotePicList.end())
				{
					iter2->second.push_back((unsigned char *)szPicName );
				}
			}
			event.uPosX += m_imgSnap->width;                //事件发生横坐标					
			string strCarNum = CarNumDetect(fourthStr);   	// 车牌的识别并把车牌号保存到event.chText;
			memcpy(event.chText,strCarNum.c_str() , strCarNum.length());	
			printf(" 车牌号码--------%s\n",strCarNum.c_str());
			if(g_ytControlSetting.nCarnumToDBMode == 1)//车牌未识别或遮挡时不入库
			{
				if(strCarNum.length() < 5)
				{
					LogNormal("车牌未识别或遮挡\n");
					return false;
				}
			}


#ifdef CAMERAAUTOCTRL
			if(strCarNum.size () > 0)
			 {
				if(!m_Rdetect.mvAddCarnumResult4StopDet(pResult->nCarId,&event.chText[2],GetTimeStamp()))
				{
					//删除图片
					{
						std::map<int,list<unsigned char*> >::iterator iter = m_mapRemotePicList.find(pResult->nCarId); 

						if (iter != m_mapRemotePicList.end())
						{
							for (list<unsigned char *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
							{
								if(*iter2 != NULL)
								{
									char szComm[512] = {0};
									sprintf(szComm,"rm -rf %s",*iter2);
									printf("Comm: %s \n",szComm);
									system(szComm);
									delete *iter2;
									*iter2 = NULL;
								}

							}
							iter->second.clear();
							m_mapRemotePicList.erase(iter);
						}
					}
					return false;
				}
			}
#endif

			
			if(m_bEventCapture)
			{
				std::string strEventInfo;
				char pEventInfo[100] = {0};
				SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
				pEventHead->uSeq = 0;
				pEventHead->uChannelID = sDetectHeader.uChannelID;
				pEventHead->uTimestamp = sDetectHeader.uTimestamp;
				pEventHead->uRealTime = 2;
				pEventHead->uTrafficSignal = pResult->nCarId;
				strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
				g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
			}
		}
	}
	else if (g_ytControlSetting.nPicComPoseMode == 2)                    //表示现在的方式  格式的 一张远景2张近景
	{
		if (pResult->nOutPutMode == NORMOAL_OUT_MODE)
		{
			if(m_bEventCapture)
			{
				StartEventRecord(sDetectHeader,pResult->nCarId);
			}

			DetectRegionRect(pResult->carRectOfSrcImg.x,pResult->carRectOfSrcImg.y,pResult->carRectOfSrcImg.width,pResult->carRectOfSrcImg.height,false);
			string fourthStr,fourthStr2;
			m_bCameraAutoCtrl = true;
			//获取两张近景图片
			if(DealTrackAutoRectTJ(fourthStr,fourthStr2,pResult->nCarId) !=0)
			{
				//需要停止录像
				if(m_bEventCapture)
				{
					StopEventRecord(sDetectHeader,pResult->nCarId);
				}

				m_bCameraAutoCtrl = false;
				return false;
			}
			// 拉回远景之后判断车是否已经离开。
			if (CheckLastPic(pResult->nCarId)!=0)
			{
				//需要停止录像
				if(m_bEventCapture)
				{
					StopEventRecord(sDetectHeader,pResult->nCarId);
				}
				return false;
			}
			m_bCameraAutoCtrl = false;
			event.uPosX += m_imgSnap->width;                //事件发生横坐标				
			string strCarNum = CarNumDetect(fourthStr);   	// 车牌的识别并把车牌号保存到event.chText;
			if(strCarNum.size()<=0)//第一张没识别出识别第二张
			{
				strCarNum = CarNumDetect(fourthStr2);
			}
			if(strCarNum.size()<=0)
			{
				strCarNum = "0000000";
			}
			memcpy(event.chText,strCarNum.c_str() , strCarNum.length());	
			LogNormal(" 车牌号码%s\n",strCarNum.c_str());

			if(strCarNum.size () > 0) 
			{ 
				#ifdef CAMERAAUTOCTRL
				//判断此车短时间内是否已经有记录
				if(!m_Rdetect.mvAddCarnumResult4StopDet(pResult->nCarId,&event.chText[2],GetTimeStamp())) 
				{
					//需要停止录像
					if(m_bEventCapture)
					{
						StopEventRecord(sDetectHeader,pResult->nCarId);
					}

					m_bCameraAutoCtrl = false;
					return false;
				}
				#endif
			}



			SRIP_DETECT_HEADER szDetectHeader[3];
			szDetectHeader[0] = *((SRIP_DETECT_HEADER*)frame.c_str());
			szDetectHeader[1] = *((SRIP_DETECT_HEADER*)fourthStr.c_str());
			szDetectHeader[2] = *((SRIP_DETECT_HEADER*)fourthStr2.c_str());

			//获取图片当前时间戳
			event.uEventBeginTime = szDetectHeader[0].uTimestamp;
			event.uMiEventBeginTime = (szDetectHeader[0].uTime64/1000)%1000;
			event.uTime2 = szDetectHeader[1].uTimestamp;
			event.uMiTime2 = (szDetectHeader[1].uTime64/1000)%1000;
			event.uEventEndTime =szDetectHeader[2].uTimestamp;
			event.uMiEventEndTime =(szDetectHeader[2].uTime64/1000)%1000;

			unsigned int uiPicCount = 3;
			IplImage* cxImg[3];
			for (int i=0;i<uiPicCount;i++)
			{
				cxImg[i] =cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height), 8, 3);
				
			}
			cvSetData(cxImg[0],(void*)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)), m_imgSnap->widthStep);
			cvSetData(cxImg[1],(void*)(fourthStr.c_str()+sizeof(SRIP_DETECT_HEADER)), m_imgSnap->widthStep);
			cvSetData(cxImg[2],(void*)(fourthStr2.c_str()+sizeof(SRIP_DETECT_HEADER)), m_imgSnap->widthStep);
			int nRandCode[3]; 
			nRandCode[0] = g_RoadImcData.GetRandCode();
			nRandCode[1] = g_RoadImcData.GetRandCode();
			nRandCode[2] = g_RoadImcData.GetRandCode();
			for (int i=0;i<uiPicCount;i++)
			{
				//获取随机数
				
				//图片叠加字符
				SingleImageAddCharacterTJ(cxImg[i],event,nRandCode[i],szDetectHeader[i]);
				string strPicPath;
				//获取文件保存路径
				
				GetProjectEventPicPath(event,strPicPath,nRandCode[i],i);

				LogNormal("%s:Save strPicPath %s\n",__FUNCTION__,strPicPath.c_str());
				
				//保存图片
				SaveImage(cxImg[i],strPicPath,i,nRandCode[1],nRandCode[2]);
				//用于通知客户端图片路径
				if(i == 0)
				{
					memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.length());
				}
				
			}

			//释放内存
			for (int i=0;i<uiPicCount;i++)
			{
				if (cxImg[i] != NULL)
				{
					cvReleaseImageHeader(&cxImg[i]);
				}
				
			}

			if(m_bEventCapture)
			{
				//结束录像
				EndEventRecord(sDetectHeader,pResult->nCarId);
			}

		}
		else
		{
			return false;
		}
	}

	return true;
}

bool CSkpRoadDetect::DetectAutoCarEvent(std::string frame,bool bAppear,RECORD_EVENT &event)
{
	//LogNormal("--非停车事件\n");
	//需要判断检测结果类型，如果是违法行为需要保存两张图片
	SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());
	PLATEPOSITION  TimeStamp[2];
	if( (m_nSaveImageCount == 2) || (!bAppear)||((m_nSmallPic == 1)&&(m_nSaveImageCount == 1)&&(!bAppear)))
	{
		//查找5秒前的图片数据
		int  nTimeInterval = 500000;//单位为微秒
		int64_t uTimestamp = sDetectHeader.uTime64;

		SRIP_DETECT_HEADER sPreHeader;

		{
			sPreHeader = GetImageFromResultFrameList(uTimestamp,nTimeInterval,m_imgPreSnap,sDetectHeader.uSeq);
		}

		event.uTime2 = sPreHeader.uTimestamp;
		event.uMiTime2 = ((sPreHeader.uTime64)/1000)%1000;

		//拼图
		event.uPicWidth = m_imgComposeSnap->width;  //事件快照宽度
		//事件快照高度
		event.uPicHeight = m_imgComposeSnap->height;

		//深圳北环格式
		if(1 == g_PicFormatInfo.nSmallViolationPic && 4 == g_nVtsPicMode)
		{
			//获取第3张图
			SRIP_DETECT_HEADER sAuxHeader;

			sAuxHeader = GetImageFromResultFrameList(uTimestamp,nTimeInterval+500000,m_imgAuxSnap,sPreHeader.uSeq);

			for(int nIndex= 0; nIndex<4; nIndex++)
			{
				CvRect rect;
				rect.x = (nIndex%2)*m_imgSnap->width;
				rect.y = (nIndex/2)*m_imgSnap->height;
				rect.width = m_imgSnap->width;
				rect.height = m_imgSnap->height;

				cvSetImageROI(m_imgComposeSnap,rect);

				if(nIndex == 0)
				{
					cvCopy(m_imgAuxSnap,m_imgComposeSnap);
				}
				else if(nIndex == 1)
				{
					cvCopy(m_imgPreSnap,m_imgComposeSnap);
				}
				else if(nIndex == 2)
				{
					cvCopy(m_imgSnap,m_imgComposeSnap);
				}
				else if(nIndex == 3)
				{
					CvPoint point;//事件坐标
					point.x = event.uPosX;
					point.y = event.uPosY;
					CvRect rtPos;
					GetViolationPos(point,rtPos);

					if(rtPos.height > 0)
					{
						//按目标区域比例裁剪
						rtPos.width = rtPos.height * rect.width/rect.height;

						if(rtPos.x + rtPos.width >= rect.width)
						{
							rtPos.x = rect.width - rtPos.width-1;
						}
						cvSetImageROI(m_imgSnap,rtPos);
						cvResize(m_imgSnap,m_imgComposeSnap);
						cvResetImageROI(m_imgSnap);
					}

					event.uPosY +=  m_imgComposeSnap->height/2;	
				}
				cvResetImageROI(m_imgComposeSnap);
			}

			PutTextOnComposeImage(m_imgComposeSnap,event,&sAuxHeader);
		}
		//2*2
		else if(1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
		{
			{
				for(int nIndex= 0; nIndex<3; nIndex++)
				{
					CvRect rect;
					if(nIndex < 2)
					{
						rect.x = (nIndex%2)*m_imgSnap->width;
						rect.y = 0;
						rect.width = m_imgSnap->width;
						rect.height = m_imgSnap->height;
					}
					else if(nIndex == 2)
					{
						rect.x = 2*m_imgSnap->width;
						rect.y = 0;
						rect.width = m_imgComposeSnap->width - rect.x;
						rect.height = m_imgComposeSnap->height;
					}

					cvSetImageROI(m_imgComposeSnap,rect);

					if(nIndex == 0)
					{
						cvCopy(m_imgPreSnap,m_imgComposeSnap);
					}
					else if(nIndex == 1)
					{
						cvCopy(m_imgSnap,m_imgComposeSnap);
					}
					else if(nIndex == 2)
					{
						CvPoint point;//事件坐标
						point.x = event.uPosX;
						point.y = event.uPosY;
						CvRect rtPos;
						GetViolationPos(point,rtPos);

						if(rtPos.height > 0)
						{
							//按目标区域比例裁剪
							rtPos.width = rtPos.height * rect.width/rect.height;

							if(rtPos.x + rtPos.width >= rect.width)
							{
								rtPos.x = rect.width - rtPos.width-1;
							}

							cvSetImageROI(m_imgSnap,rtPos);
							cvResize(m_imgSnap,m_imgComposeSnap);
							cvResetImageROI(m_imgSnap);
						}
					}
					cvResetImageROI(m_imgComposeSnap);
				}
				event.uPosX += m_imgSnap->width;                //事件发生横坐标
				PutTextOnComposeImage(m_imgComposeSnap,event);
			}
		}
		//是否需要存储小图
		else if(m_nSmallPic == 1&&m_nSaveImageCount == 2)
		{
			for(int nIndex= 0; nIndex<4; nIndex++)
			{
				CvRect rect;
				if(nIndex < 2)
				{
					rect.x = (nIndex%2)*m_imgSnap->width;
					rect.y = 0;
					rect.width = m_imgSnap->width;
					rect.height = m_imgSnap->height;
				}
				else if(nIndex ==2)
				{
					rect.x = 2*m_imgSnap->width;
					if(m_nWordPos == 1)
						rect.y = m_nExtent;
					else
						rect.y = 0;
					rect.width = m_imgComposeSnap->width - rect.x;
					rect.height = m_imgComposeSnap->height - m_nExtent;
				}
				else
				{
					rect.x = 0;
					if(m_nWordPos == 1)
						rect.y = 0;
					else
						rect.y = m_imgComposeSnap->height - m_nExtent;
					rect.width = m_imgComposeSnap->width;
					rect.height = m_nExtent;
				}

				if(rect.height>0)
					cvSetImageROI(m_imgComposeSnap,rect);

				if(nIndex == 0)
				{
					cvCopy(m_imgSnap, m_imgComposeSnap);
				}
				else if(nIndex == 1)
				{
					cvCopy(m_imgPreSnap, m_imgComposeSnap);
				}
				else if(nIndex == 2)
				{
					CvPoint point;//事件坐标
					point.x = event.uPosX;
					point.y = event.uPosY;
					CvRect rtPos;
					GetViolationPos(point,rtPos,1);

					if(rtPos.height > 0)
					{
						//按目标区域比例裁剪
						rtPos.width = rtPos.height * rect.width/rect.height;

						if(rtPos.x + rtPos.width >= rect.width)
						{
							rtPos.x = rect.width - rtPos.width-1;
						}

						cvSetImageROI(m_imgSnap,rtPos);
						cvResize(m_imgSnap,m_imgComposeSnap);
						cvResetImageROI(m_imgSnap);
					}
				}
				else
				{
					if(rect.height>0)
						cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));
				}

				if(rect.height>0)
					cvResetImageROI(m_imgComposeSnap);
			}
			PutTextOnComposeImage(m_imgComposeSnap,event);
		}
		else
		{
			CvRect rect;
			rect.x = 0;
			if(m_nWordPos == 1)
				rect.y = 0;
			else
				rect.y = m_imgComposeSnap->height - m_nExtent;
			rect.width = m_imgComposeSnap->width;
			rect.height = m_nExtent;

			if(m_nExtent > 0)
			{
				cvSetImageROI(m_imgComposeSnap,rect);
				cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));
				cvResetImageROI(m_imgComposeSnap);
			}

			//叠加信息
			for(int nIndex= 0; nIndex<2; nIndex++)
			{
				rect.x = (nIndex%2)*m_imgSnap->width;
				rect.y = 0;
				rect.width = m_imgSnap->width;
				rect.height = m_imgSnap->height;

				if(rect.height > 0)
					cvSetImageROI(m_imgComposeSnap,rect);

				if(nIndex == 0)
				{
					cvCopy(m_imgSnap, m_imgComposeSnap);
				}
				else if(nIndex == 1)
				{
					cvCopy(m_imgPreSnap, m_imgComposeSnap);
				}

				cvResetImageROI(m_imgComposeSnap);

				PutTextOnImage(m_imgComposeSnap,event,&sPreHeader,nIndex);
			}
		}
	}
	else//卡口只需要一张图片
	{
		event.uPicWidth = m_imgSnap->width;  //事件快照宽度
		//事件快照高度
		event.uPicHeight = m_imgSnap->height;
		//叠加信息
		PutTextOnImage(m_imgSnap,event);
	}
	return true;
}

bool CSkpRoadDetect::InitAutoCtrlCfg()
{
	printf("CSkpRoadDetect::InitAutoCtrlCfg \n");
	LogNormal("###nPicComPoseMode:%d,nCarNum:%d###\n",g_ytControlSetting.nPicComPoseMode,g_PicFormatInfo.nCarNum);


#ifdef CAMERAAUTOCTRL
	m_EventVisVideo->SetVisInfo(g_strFtpUserName.c_str(),g_strFtpPassWord.c_str(),g_strFtpServerHost.c_str(),g_nFtpPort,g_ftpRemotePath);
	//m_EventVisVideo->AddEventVideoReq(GetTimeT()-1600,GetTimeT()-1300,GetTimeT());
#endif
	//清空临时录像文件
	char szbody[512] = {0};
	sprintf(szbody,"rm -rf %s/*.mp4 ",g_strVideo.c_str());
	system(szbody);
	{
		char szEventPic[200] = {0};
		sprintf(szEventPic,"mkdir /home/road/server/profile ");
		system(szEventPic);
	}


	{
		char szEventPic[200] = {0};
		sprintf(szEventPic,"mkdir /home/road/server/config ");
		system(szEventPic);
	}

	{
		char szEventPic[200] = {0};
		sprintf(szEventPic,"mkdir /detectdata ");
		system(szEventPic);
	}

	{
		char szEventPic[200] = {0};
		sprintf(szEventPic,"mkdir /detectdata/eventpic ");
		system(szEventPic);
	}
	{
		char szEventPic[200] = {0};
		sprintf(szEventPic,"rm -rf /detectdata/eventpic/* ");
		system(szEventPic);
	}

	m_bDetectClientRect = false;
	m_bCameraMoveFormOp = false;
	m_bCameraAutoCtrl = false;
	m_mapAutoCameraRecord.clear();
	//初始化相机预置位
	m_nPresetNum = g_skpDB.GetPreSet(m_nChannelID);
	printf("PresetNum: %d,m_nChannelID:%d \n",m_nPresetNum,m_nChannelID);
	if(m_nPresetNum < 0)
	{
		m_nPresetNum = 1;
	}

	if(m_nCameraType == BOCOM_301_200)
	{
#ifdef CAMERAAUTOCTRL
		m_pRoadDetectPelco->GotoPreset(m_nPresetNum);
#endif
		return true;
	}
	else if(m_nCameraType == DH_CAMERA)
	{
#ifdef CAMERAAUTOCTRL
		m_pRoadDetectDHCamera->SetCameraVar(4.8,3.6,4.7,94.0,20);
		pthread_mutex_lock(&m_Preset_Mutex);
		for(mapAutoCameraPtzTrack::iterator iter=m_CameraPtzPresetStatus.begin();iter!=m_CameraPtzPresetStatus.end();iter++)
		{
			delete iter->second;
		}
		m_CameraPtzPresetStatus.clear();
		pthread_mutex_unlock(&m_Preset_Mutex);

		GetPTZPresetStatus();
		m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
#endif

	}
	return true;
}

int CSkpRoadDetect::DealTrackAutoRectDHCamera(std::string& response,int nCarId,int nSaveProcess)
{
	printf("%s%d \n",__FUNCTION__,__LINE__);
	int iNowTime = GetTimeT();
	int uiResponse = 0;
	//20秒以内
	while( abs((int)GetTimeT()-iNowTime) < 30)
	{
		string frame;
		PopRealFrame(frame);
#ifdef CAMERAAUTOCTRL
		uiResponse = m_pRoadDetectDHCamera->DealTrackAutoRect(frame,nCarId,nSaveProcess);
#endif
		if(uiResponse == -1)
		{
			sleep(4);
			response = frame;
#ifdef CAMERAAUTOCTRL
			m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
			sleep(5);
#endif
			LogNormal("[%s]Lost Goal , End Detect Rect \n",__FUNCTION__);
			return -1; 
		}
		else if(uiResponse == 0)
		{
			sleep(4);
			PopRealFrame(response);
#ifdef CAMERAAUTOCTRL
			m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
			sleep(5);
#endif
			LogNormal("[%s]Find Goal , End Detect Rect \n",__FUNCTION__);
			return 0;
		}
	}

	LogNormal("[%s]Check TimeOut GotoPreset 30, End Detect Rect \n",__FUNCTION__);
#ifdef CAMERAAUTOCTRL
	m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
	sleep(5);
#endif
	return -1;
}


int CSkpRoadDetect::DealTrackAutoRectBocomPTZ(std::string& response,int nCarId,int nSaveProcess)
{
	printf("%s%d \n",__FUNCTION__,__LINE__);

	int iNowTime = GetTimeT();
	//20秒以内
	while( abs((int)GetTimeT()-iNowTime) < 300)
	{
		string frame;
		PopRealFrame(frame);
		int uiResponse = 0;
#ifdef CAMERAAUTOCTRL
		uiResponse = m_pRoadDetectPelco->DealTrackAutoRect(frame);
#endif
		if(uiResponse == -1)
		{
			sleep(4);
			response = frame;
#ifdef CAMERAAUTOCTRL
			m_pRoadDetectPelco->GotoPreset(m_nPresetNum);
#endif
			return -1;
		}
		else if(uiResponse == 0)
		{
			sleep(4);
			response = frame;
#ifdef CAMERAAUTOCTRL
			m_pRoadDetectPelco->GotoPreset(m_nPresetNum);
#endif
			return 0;
		}
	}

	LogNormal("[%s]Check TimeOut GotoPreset 3, End Detect Rect \n",__FUNCTION__);
#ifdef CAMERAAUTOCTRL
	m_pRoadDetectPelco->GotoPreset(m_nPresetNum);
#endif
	return -1;
}


int CSkpRoadDetect::ComPoseImageForYuntai(RECORD_EVENT event, int key)
{
	printf("%s%d \n",__FUNCTION__,__LINE__);
	std::map<int, list<unsigned char *> >::iterator iterMap = m_mapRemotePicList.find(key);

	if(iterMap != m_mapRemotePicList.end())
	{
		list<unsigned char * >::iterator iterList = iterMap->second.begin();
		unsigned char * one = NULL;
		unsigned char * two = NULL;
		unsigned char * three = NULL;
		unsigned char * four = NULL;

		IplImage * cximgFist = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
		IplImage * cximgSecond = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
		IplImage * cximgThird =  cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);
		IplImage * cximgFourth = cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height-m_nExtent), 8, 3);

		int index = 0 ;
		for (; iterList != iterMap->second.end(); iterList++)
		{
			if (index == 0)
				one = *iterList;
			else if (index == 1)
				two = *iterList;
			else if (index == 2)
				three = *iterList;
			else 
				four = *iterList;
			index++;		
		}
		if (one == NULL || two == NULL || three == NULL || four == NULL)
		{
			//LogNormal("---合成图片有一张为空 \n");
			return 0;
		}
		//LogNormal("----合成图片");
		///////////////////////////////去头 
		cvSetData(cximgFist, one+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);
		cvSetData(cximgSecond, two+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);
		cvSetData(cximgThird, three+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);
		cvSetData(cximgFourth, four+sizeof(SRIP_DETECT_HEADER), m_imgSnap->widthStep);


		for(int nIndex= 0; nIndex<4; nIndex++)
		{
			CvRect rect;
			rect.x = (nIndex%2)*m_imgSnap->width;
			rect.y = (nIndex/2)*(m_imgSnap->height-m_nExtent);
			rect.width = m_imgSnap->width;
			rect.height = m_imgSnap->height-m_nExtent;

			cvSetImageROI(m_imgComposeStopSnap,rect);
			if (g_ytControlSetting.nPicComPoseMode == 1)
			{
				if(nIndex == 0)
				{
					cvCopy(cximgFist, m_imgComposeStopSnap);
				}
				else if (nIndex == 1)
				{
					cvCopy(cximgSecond, m_imgComposeStopSnap);
				}
				else if (nIndex == 2)
				{
					cvCopy(cximgThird, m_imgComposeStopSnap);
				}
				else if (nIndex == 3)
				{
					cvCopy(cximgFourth, m_imgComposeStopSnap);      
				}
			}
			else if(g_ytControlSetting.nPicComPoseMode == 0)
			{
				if(nIndex == 0)
				{
					cvCopy(cximgFist, m_imgComposeStopSnap);
				}
				else if (nIndex == 1)
				{
					cvCopy(cximgSecond, m_imgComposeStopSnap);
				}
				else if (nIndex == 2)
				{
					cvCopy(cximgThird, m_imgComposeStopSnap);
				}
				else if (nIndex == 3)
				{
					cvCopy(cximgFourth, m_imgComposeStopSnap);
				}
			}
			else if(g_ytControlSetting.nPicComPoseMode == 3)
			{
				if(nIndex == 0)
				{
					cvCopy(cximgFourth, m_imgComposeStopSnap);
				}
				else if (nIndex == 1)
				{
					cvCopy(cximgFist, m_imgComposeStopSnap);
				}
				else if (nIndex == 2)
				{
					cvCopy(cximgSecond, m_imgComposeStopSnap);
				}
				else if (nIndex == 3)
				{
					cvCopy(cximgThird, m_imgComposeStopSnap);
				}
			}
			
			cvResetImageROI(m_imgComposeStopSnap);
		}
		
		// 往图片上增加字
		ImageAddCharacter(m_imgComposeStopSnap, event,key);

		// 释放
		if(cximgFist)
		{
			cvReleaseImageHeader(&cximgFist);
		}
		if(cximgSecond)
		{
			cvReleaseImageHeader(&cximgFist);
		}
		if(cximgThird)
		{
			cvReleaseImageHeader(&cximgFist);
		}
		if(cximgFourth)
		{
			cvReleaseImageHeader(&cximgFist);
		}

		
		
		// 释放内存  从map 删除这四张图
		iterList = iterMap->second.begin();
		for (; iterList != iterMap->second.end(); iterList++)
		{
			if(*iterList != NULL)
			{
				free(*iterList);	
				//*iterList = NULL;
			}
			
		}

		m_mapRemotePicList.erase(iterMap);
	}
	//LogNormal("-----ComPoseImage end\n");
	//LogNormal("save  buff image  ---ok  \n");
	return 1;
}

void CSkpRoadDetect::GetProjectEventPicPath(RECORD_EVENT RecordEvent,std::string& strPicPath,int nRandCode,int nIndex)
{
	
	RECORD_PLATE RecordPlate;
	RecordPlate.uSeq = RecordEvent.uSeq;
	RecordPlate.uTime = RecordEvent.uEventBeginTime;
	RecordPlate.uMiTime = RecordEvent.uMiEventBeginTime;
	RecordPlate.uDirection = m_nDirection;
	memset(RecordPlate.chText,0,MAX_PLATE);
	sprintf(RecordPlate.chText,"%s",RecordEvent.chText);
	//RecordPlate.uColor = RecordEvent.uColor1;//车牌颜色可通过车牌检测获取，此处不对
	RecordPlate.uViolationType = DETECT_RESULT_EVENT_STOP;
	RecordPlate.uRoadWayID = RecordEvent.uRoadWayID;
	//RecordPlate.uCarColor1 = RecordEvent.uColor2;//车身颜色未知
	RecordPlate.uSpeed = RecordEvent.uSpeed;
	RecordPlate.uLimitSpeed = 0;
	RecordPlate.uChannelID = m_nChannelID;
	//RecordPlate.uType = RecordEvent.uType;//车辆类型未知
	RecordPlate.uTime2 = RecordEvent.uEventEndTime;
	RecordPlate.uMiTime2 = RecordEvent.uMiEventEndTime;

	//RecordPlate.uRedLightBeginTime = RecordEvent.uVideoBeginTime;
	//RecordPlate.uRedLightEndTime   = RecordEvent.uVideoEndTime;
	//LogNormal("GetProjectEventPicPath,%d-%d \n",RecordPlate.uRedLightBeginTime,RecordPlate.uRedLightEndTime);
	memset(RecordPlate.chPicPath,0,MAX_VIDEO);
	sprintf(RecordPlate.chPicPath,"%s",RecordEvent.chPicPath);

	memset(RecordPlate.chVideoPath,0,MAX_VIDEO);
	sprintf(RecordPlate.chVideoPath,"%s",RecordEvent.chVideoPath);

	g_RoadImcData.GetPlatePicPath(RecordPlate,strPicPath,5,nRandCode,nIndex);
	if(nIndex == 0)
	{
		g_RoadImcData.AddStopEventViolationData(RecordPlate,strPicPath,m_strDeviceId);
	}
	return;
}


int CSkpRoadDetect::SaveEventRecord(RECORD_EVENT &tRecordEvent,int nCarId,int nChannelId)
{

	//录像路径
	{
		char szFtpPath[512] = {0};
		string strEventTime = GetTime(m_dwEventTime,2);
		sprintf(szFtpPath,"ftp://%s/%d-%d-%s.avi",g_strFtpServerHost.c_str(),atoi(g_strDetectorID.c_str()),tRecordEvent.uDirection,strEventTime.c_str());
		memcpy(tRecordEvent.chVideoPath,szFtpPath,strlen(szFtpPath));//事件录象路径
		printf("SaveEventRecord, VideoPath:%s \n",szFtpPath);
	}
	//保存图片路径
	{
		//if(g_nServerType == 7)
		{
			//获取事件开始时间
			if(m_mapVideoTime.find(nCarId) == m_mapVideoTime.end())
			{
				tRecordEvent.uEventBeginTime = tRecordEvent.uVideoEndTime - 420;
			}
			else
			{
				tRecordEvent.uEventBeginTime = m_mapVideoTime[nCarId];
				m_mapVideoTime.erase(nCarId);
			}
			tRecordEvent.uTime2 = m_dwEventTime;

			 //事件结束时间
			tRecordEvent.uEventEndTime   = GetTimeT();
			//录像开始时间
			tRecordEvent.uVideoBeginTime = tRecordEvent.uEventBeginTime - 60;
			tRecordEvent.uVideoEndTime   = GetTimeT()+60;


			std::string strPicPath;
			GetProjectEventPicPath(tRecordEvent,strPicPath);

			int nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);

			memcpy(tRecordEvent.chPicPath,strPicPath.c_str(),strPicPath.size()); //事件图片路径

			printf("SaveEventRecord, PicPath:%s \n",strPicPath.c_str());

			tRecordEvent.uPicSize = SaveImage(m_imgComposeStopSnap,strPicPath);
			printf("SaveEventRecord, PicSize:%d \n",tRecordEvent.uPicSize);

			g_skpDB.SaveTraEvent(nChannelId,tRecordEvent,0,0,true,1);

		}
	}

	return 0;
}




//抓拍过程为，检测到车辆停止行驶后，先抓拍一张远景图片，同时开始计时。然后拉近，抓拍第一张近景特写图片，镜头不回到远景预置位，一直盯着该目标，等到计时时间达到30秒后抓拍第二张近景特写图片。
int CSkpRoadDetect::DealTrackAutoRectTJ( std::string& response,std::string& response2,int nCarId )
{
	if(m_nCameraType != DH_CAMERA)
	{
		return -1;
	}
	if (g_ytControlSetting.nPicComPoseMode == 2)
	{
		return DealTrackAutoRectDHCameraTJ(response,response2,nCarId);
	}
	return -1;
}

int CSkpRoadDetect::DealTrackAutoRectDHCameraTJ( std::string& response,std::string& response2,int nCarId )
{

	printf("%s%d \n",__FUNCTION__,__LINE__);
	int iStartTime = GetTimeT();//记录流程开始时间
	int uiResponse = 0;
	int nStopInterval = g_ytControlSetting.nStopInterval;//保存截图间隔时间
	LogNormal("[%s]GetTwoPic between:%d seconds \n",__FUNCTION__,nStopInterval);
	while( abs((int)GetTimeT()-iStartTime) <nStopInterval)
	{
		string frame;
		PopRealFrame(frame);
#ifdef CAMERAAUTOCTRL
		//LogNormal("[%s]DealTrackAutoRect \n",__FUNCTION__);
		//移动到指定区域
		uiResponse = m_pRoadDetectDHCamera->DealTrackAutoRect(frame,nCarId,0);
#endif
		if(uiResponse == -1)
		{
			sleep(4);
			response = frame;
			response2 = frame;
#ifdef CAMERAAUTOCTRL
			//回到预置位
			m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
			sleep(5);
#endif
			LogNormal("[%s]Lost Goal , End Detect Rect \n",__FUNCTION__);
			return -1; 
		}
		else if(uiResponse == 0)
		{
			sleep(4);
			PopRealFrame(response);//取第一张图

			int dwTimeOut = GetTimeT() - iStartTime ;
			
			//等待时间
			if(dwTimeOut < nStopInterval)//未达到指定时间
			{
				sleep(nStopInterval-dwTimeOut);
			}
			PopRealFrame(response2);//取第二张图
#ifdef CAMERAAUTOCTRL
			LogNormal("[%s]GotoPreset \n",__FUNCTION__);
			//回到预置位
			m_pRoadDetectDHCamera->GotoPreset(m_nPresetNum);
			sleep(5);
#endif
			return 0;
		}

	}
	return -1;
}


void CSkpRoadDetect::SingleImageAddCharacterTJ( IplImage * pImage, RECORD_EVENT event, int key,SRIP_DETECT_HEADER header )
{
	//LogNormal("[%s]:PicMode=%d,Extent=%d,WordPos=%d,CarNum=%d\n",__FUNCTION__,g_ytControlSetting.nPicComPoseMode,m_nExtent,m_nWordPos,g_PicFormatInfo.nCarNum);
	wchar_t wchOut[255] = {'\0'};
	char chOut[255] = {'\0'};

	int nStartX = 0;    


	string strTime; 
	int nMiTime = 0;
	int nWidth = 10;
	int nHeight = 100;

	////防伪码
	//char szBuf[9] = {'\0'};
	//char szTmp[3] = {'\0'};
	//char szAvgval[10] = {0};
	//sprintf(szAvgval,"%d",(int)key);
	//m_pMd5Ctx->MD5Update((unsigned char*)szAvgval,strlen(szAvgval));
	//unsigned char szDigest[16] = {0};
	//m_pMd5Ctx->MD5Final(szDigest);
	//for(int i = 0;i<4;i++)
	//{
	//	sprintf(szTmp,"%02x",szDigest[i]);
	//	strcat(szBuf,szTmp);
	//}
	//时间
	strTime = GetTime(header.uTimestamp,7);
	nMiTime = (header.uTime64/1000)%1000;

	/*int nRowCount = 4;//叠加字符行数
	int nChHeight = 50;//叠加字符高度

	//计算叠加字符的区域
	CvRect dstRt;
	dstRt.x = nWidth;
	dstRt.y = nHeight;
	dstRt.height = nHeight + nRowCount*nChHeight;
	dstRt.width = m_imgSnap->width;//图片宽度

	//判断叠加字符区域的灰度值
	bool bBlack = AverageValue(pImage,dstRt);
	CvScalar color = CV_RGB(255,255,255);//默认白色

	if (!bBlack)//区域为白色
	{
		LogNormal("[%s]:character black\n",__FUNCTION__);
		color = CV_RGB(0,0,0);//黑色
	}
	else
	{
		LogNormal("[%s]:character white\n",__FUNCTION__);
	}*/
	CvScalar color = CV_RGB(0,0,0);
	
	printf("putText tianjin 1*2.........................\n");
	
	sprintf(chOut, "抓拍时间:%s.%03d", strTime.c_str(),nMiTime);
	memset(wchOut,0,sizeof(wchOut))	;	
	UTF8ToUnicode(wchOut, chOut);
	
	CvRect recSmallPic;
	recSmallPic.x = nWidth;
	recSmallPic.y = nHeight;
	recSmallPic.height = nHeight + 10*80;
	recSmallPic.width = pImage->width / 2 + 100;

	IplImage* pImageText = NULL;
	pImageText = cvCreateImage(cvSize(recSmallPic.width,recSmallPic.height),8,3);
	cvSet(pImageText, cvScalar(255,255,255) );
	m_cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);
	
	nHeight += (50);
	sprintf(chOut,"摄录地点:%s", m_strLocation.c_str());
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	m_cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);

	//nHeight += (50);
	//sprintf(chOut,"行驶方向:%s", strDirection.c_str());
	//memset(wchOut,0,sizeof(wchOut));
	//UTF8ToUnicode(wchOut,chOut);
	//m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

	nHeight += (50);
	sprintf(chOut,"设备编号:%s", m_strDeviceId.c_str());
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	m_cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight), color);

	nHeight += (50);
	sprintf(chOut,"防伪码:%08x", key);
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	m_cvText.putText(pImageText, wchOut, cvPoint(nWidth, nHeight),color);
	
	PrintColor(pImageText,recSmallPic,pImage);

	if(pImageText != NULL)
    {
		cvReleaseImageHeader(&pImageText);
		pImageText = NULL;
    }

	printf("%s:over\n",__FUNCTION__);

}

bool CSkpRoadDetect::DetectHandEventTJ( std::string frame,int nCarId,CvRect rect )
{
	LogNormal("[%s]:nCarId= %d\n",__FUNCTION__,nCarId);
	if(m_nExtent > 0)
		frame.append(m_pExtentRegion,m_img->widthStep*(m_nExtent/m_nDeinterlace));   

	DetectRegionRect(rect.x,rect.y,rect.width,rect.height,false);

	SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

	//开始录像
	if(m_bEventCapture)
	{
		StartEventRecord(sDetectHeader,nCarId);
	}

	string fourthStr,fourthStr2;
	m_bCameraAutoCtrl = true;
	//获取两张图片
	if(DealTrackAutoRectTJ(fourthStr,fourthStr2,nCarId) !=0)
	{
		m_bCameraAutoCtrl = false;
		//开始录像
		if(m_bEventCapture)
		{
			StopEventRecord(sDetectHeader,nCarId);
		}
		return false;
	}
	m_bCameraAutoCtrl = false;
	string strCarNum = CarNumDetect(fourthStr);   	// 车牌的识别并把车牌号保存到event.chText;
	if(strCarNum.size()<=0)//第一张没识别出识别第二张
	{
		strCarNum = CarNumDetect(fourthStr2);
	}
	if(strCarNum.size()<=0)
	{
		strCarNum = "0000000";
	}
	RECORD_EVENT event;
	memset(&event,0,sizeof(RECORD_EVENT));
	memcpy(event.chPlace,m_strLocation.c_str(),m_strLocation.size());
	event.uDirection = m_nDirection;
	event.uCode = DETECT_RESULT_EVENT_STOP;
	memcpy(event.chText,strCarNum.c_str() , strCarNum.length());	
	LogNormal("[%s] 车牌号码%s\n",__FUNCTION__,strCarNum.c_str());
	event.uRoadWayID = 0;
	//if(strCarNum.size () > 0) 
	//{ 
	//	//判断此车短时间内是否已经有记录
	//	if(!m_Rdetect.mvAddCarnumResult4StopDet(nCarId,&event.chText[2],GetTimeStamp())) 
	//	{
	//		m_bCameraAutoCtrl = false;
	//		LogNormal("[%s] 重复选中了此车，已经有此车的记录%s\n",__FUNCTION__,strCarNum.c_str());
	//		return false;
	//	}
	//}

	SRIP_DETECT_HEADER szDetectHeader[3];
	szDetectHeader[0] = *((SRIP_DETECT_HEADER*)frame.c_str());
	szDetectHeader[1] = *((SRIP_DETECT_HEADER*)fourthStr.c_str());
	szDetectHeader[2] = *((SRIP_DETECT_HEADER*)fourthStr2.c_str());

	//获取图片当前时间戳
	event.uEventBeginTime = szDetectHeader[0].uTimestamp;
	event.uMiEventBeginTime = (szDetectHeader[0].uTime64/1000)%1000;
	event.uTime2 = szDetectHeader[1].uTimestamp;
	event.uMiTime2 = (szDetectHeader[1].uTime64/1000)%1000;
	event.uEventEndTime =szDetectHeader[2].uTimestamp;
	event.uMiEventEndTime =(szDetectHeader[2].uTime64/1000)%1000;

	unsigned int uiPicCount = 3;
	IplImage* cxImg[3];
	for (int i=0;i<uiPicCount;i++)
	{
		cxImg[i] =cvCreateImageHeader(cvSize(m_imgSnap->width, m_imgSnap->height), 8, 3);

	}
	cvSetData(cxImg[0],(void*)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)), m_imgSnap->widthStep);
	cvSetData(cxImg[1],(void*)(fourthStr.c_str()+sizeof(SRIP_DETECT_HEADER)), m_imgSnap->widthStep);
	cvSetData(cxImg[2],(void*)(fourthStr2.c_str()+sizeof(SRIP_DETECT_HEADER)), m_imgSnap->widthStep);
	int nRandCode[3]; 
	nRandCode[0] = g_RoadImcData.GetRandCode();
	nRandCode[1] = g_RoadImcData.GetRandCode();
	nRandCode[2] = g_RoadImcData.GetRandCode();
	//保存三张图片
	for (int i=0;i<uiPicCount;i++)
	{
		//获取随机数

		//图片叠加字符
		SingleImageAddCharacterTJ(cxImg[i],event,nRandCode[i],szDetectHeader[i]);
		string strPicPath;

		//获取文件保存路径
		GetProjectEventPicPath(event,strPicPath,nRandCode[i],i);

		printf("%s:strPicPath %s\n",__FUNCTION__,strPicPath.c_str());
		//LogNormal("strPicPath=%s\n",strPicPath.c_str());
		//保存图片
		SaveImage(cxImg[i],strPicPath,i,nRandCode[1],nRandCode[2]);
		//用于通知客户端图片路径
		if(i == 0)
		{
			//删除已经存在的记录
			g_skpDB.DeleteOldRecord(strPicPath,false,false);
			memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.length());
		}

	}
	//释放内存
	for (int i=0;i<uiPicCount;i++)
	{
		if (cxImg[i] != NULL)
		{
			cvReleaseImageHeader(&cxImg[i]);
		}

	}
	
	if(m_bEventCapture)
	{
		//结束录像
		EndEventRecord(sDetectHeader,nCarId);
		//保存录像
		if (SaveEventRecordTJ(event,nCarId)!=0)
		{
			return false;
		}
	}

	int nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);

	//记录检测数据,保存事件
	if(nSaveRet>0)
	{
		g_skpDB.SaveTraEvent(sDetectHeader.uChannelID,event,0,0,true,1);
	}
	//通知客户端
	std::string strEventPlate;
	strEventPlate.append((char*)&event,sizeof(RECORD_EVENT));
	szDetectHeader[1].uDetectType = SRIP_DETECT_EVENT;
	strEventPlate.insert(0,(char*)&szDetectHeader[1],sizeof(SRIP_DETECT_HEADER));
	
	if(m_bConnect)
	{
		g_skpChannelCenter.AddResult(strEventPlate);
	}

	LogNormal("[%s] success，end\n",__FUNCTION__);
	return true;
}

int CSkpRoadDetect::CheckLastPic( int nCarId )
{
	//回到预置位之后判断车是否还存在，不存在的话,此次事件失效

	UINT32 uStartTime = 0;//开始时间
	UINT32 uEndTime = 0;//结束时间
	int flag = 0;
	LogNormal("[%s]mvCheckStopObjectIsExistAfterCameraBack  \n",__FUNCTION__);
	string frame;
	while(abs((long long)uEndTime-(long long)uStartTime)<6)//时差超过6s退出循环
	{
		PopRealFrame(frame);
		SRIP_DETECT_HEADER szDetectHeader;
		szDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());
		if (0==flag)
		{
			uStartTime = szDetectHeader.uTimestamp;//记录第一次的时间单位s
			LogNormal("[%s]uStartTime=%u \n",__FUNCTION__,uStartTime);
			flag++;
		}

		uEndTime = szDetectHeader.uTimestamp;//记录每次的时间单位s
		#ifdef CAMERAAUTOCTRL
		bool bRet =m_Rdetect.mvCheckStopObjectIsExistAfterCameraBack(nCarId,(char*)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)),szDetectHeader.uTime64);
		if(bRet)
		{

			LogNormal("[%s],success\n",__FUNCTION__);
			return 0;//有一次存在返回成功
		}
		#endif
	}
	LogNormal("[%s]uEndTime = %u \n",__FUNCTION__,uEndTime);
	LogNormal("[%s],failed \n",__FUNCTION__);
	return -1;
}

void CSkpRoadDetect::StopEventRecord( SRIP_DETECT_HEADER sDetectHeader,int nCarId )
{
	LogNormal("[%s]nCarId = %d \n",__FUNCTION__,nCarId);
	if(m_mapAutoCameraRecord.find(nCarId) != m_mapAutoCameraRecord.end())
	{
		m_mapAutoCameraRecord.erase(nCarId);
	}
	std::string strEventInfo;
	char pEventInfo[100] = {0};
	SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
	pEventHead->uSeq = 0;
	pEventHead->uChannelID = sDetectHeader.uChannelID;
	pEventHead->uTimestamp = sDetectHeader.uTimestamp;
	pEventHead->uRealTime = 1;
	pEventHead->uTrafficSignal = nCarId;
	strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
	g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
}

void CSkpRoadDetect::StartEventRecord( SRIP_DETECT_HEADER sDetectHeader,int nCarId )
{
	LogNormal("[%s]nCarId = %d \n",__FUNCTION__,nCarId);
	if(m_mapAutoCameraRecord.find(nCarId) == m_mapAutoCameraRecord.end())
	{
		m_mapAutoCameraRecord.insert(make_pair(nCarId,m_nVideoNum));
	}
	else
	{
		m_mapAutoCameraRecord[nCarId] = m_nVideoNum;
	}
	std::string strEventInfo;
	char pEventInfo[100] = {0};
	SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
	g_FileManage.CheckDisk(true,true);
	pEventHead->uSeq =  m_nVideoNum++;
	pEventHead->uChannelID = sDetectHeader.uChannelID;
	pEventHead->uTimestamp = sDetectHeader.uTimestamp;
	pEventHead->uRealTime = 0;
	pEventHead->uTrafficSignal = nCarId;
	strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
	g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
}

void CSkpRoadDetect::EndEventRecord( SRIP_DETECT_HEADER sDetectHeader,int nCarId )
{
	LogNormal("[%s]nCarId = %d \n",__FUNCTION__,nCarId);
	std::string strEventInfo;
	char pEventInfo[100] = {0};
	SRIP_DETECT_HEADER *pEventHead = (SRIP_DETECT_HEADER*)pEventInfo;
	pEventHead->uSeq = 0;
	pEventHead->uChannelID = sDetectHeader.uChannelID;
	pEventHead->uTimestamp = sDetectHeader.uTimestamp;
	pEventHead->uRealTime = 2;
	pEventHead->uTrafficSignal = nCarId;
	strEventInfo.insert(0,pEventInfo,sizeof(SRIP_DETECT_HEADER));
	g_skpChannelCenter.AddRecordEvent(m_nChannelID,strEventInfo);
}

int CSkpRoadDetect::SaveEventRecordTJ(RECORD_EVENT &event,int nCarId)
{
	LogNormal("[%s]nCarId = %d \n",__FUNCTION__,nCarId);
	unsigned int nVideoIdTmp = 1;
	if(m_mapAutoCameraRecord.find(nCarId) != m_mapAutoCameraRecord.end())
	{
		nVideoIdTmp = (unsigned int)m_mapAutoCameraRecord[nCarId];
		m_mapAutoCameraRecord.erase(nCarId);
		LogNormal("[%s]:nVideoIdTmp=%d \n",__FUNCTION__,nVideoIdTmp);
	}
	else
	{
		return -1;
	}

	string strVideoPath = g_strVideo;
	string strPath,strTmpPath;
	strVideoPath = "/home/road/red";
	if (IsDataDisk())
	{
		strVideoPath = "/detectdata/red";
	}
	RECORD_PLATE plate;
	plate.uTime = event.uEventBeginTime;
	plate.uDirection = event.uDirection;
	memcpy(plate.chText,event.chText,sizeof(event.chText));

	//录像保存路径
	g_RoadImcData.GetPlatePicPath(plate,strPath,6);
	LogNormal("[%s]path:%s \n",__FUNCTION__,strPath.c_str());
	//LogNormal("strVideoPath:%s \n",strVideoPath.c_str());
	sleep(2); //防止录像文件没有close
	char szbody[200] = {0};
	sprintf(szbody,"%s/%d.mp4",g_strVideo.c_str(),nVideoIdTmp);
	LogNormal("[%s]VideoPath:%s \n",__FUNCTION__,szbody);
	if (access(szbody,F_OK) != 0)
	{
		LogNormal("[%s]VideoPath can not assess \n",__FUNCTION__);
		return -1;
	}
	rename(szbody,strPath.c_str());//用rename不用system方式修改文件名称


	strTmpPath = strPath.erase(0,strVideoPath.size());
	//FTP路径
	strTmpPath = g_ServerHost+strTmpPath;
	strPath = "ftp://"+strTmpPath;
	memcpy(event.chVideoPath,strPath.c_str(),strPath.size());//事件录象路径

	return 0;
}

bool CSkpRoadDetect::AverageValue( IplImage *pGrayImg,CvRect rect )
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

//设定打字颜色 
void CSkpRoadDetect::PrintColor(IplImage* pSmallPic,CvRect recSmallPic, IplImage* pBigPic) 
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


#endif
