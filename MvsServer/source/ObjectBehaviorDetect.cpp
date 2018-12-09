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

#include "Common.h"
#include "CommonHeader.h"
#include "ObjectBehaviorDetect.h"
#include <time.h>

#include "ippi.h"
#include "ippcc.h"
#include "ximage.h"
#include "XmlParaUtil.h"


#ifndef NOOBJECTDETECT


//监控线程
void* ThreadDetectObjectBehavior(void* pArg)
{
    //取类指针
    CObjectBehaviorDetect* pRoadDetect = (CObjectBehaviorDetect*)pArg;
    if(pRoadDetect == NULL) return pArg;

    pRoadDetect->DealFrame();
    pthread_exit((void *)0);
    return pArg;
}
//构造
CObjectBehaviorDetect::CObjectBehaviorDetect()
{
    //线程ID
    m_nThreadId = 0;
    //通道ID初始
    m_nChannelID = -1;
    //检测帧列表互斥
    pthread_mutex_init(&m_Frame_Mutex,NULL);
    pthread_mutex_init(&m_preFrame_Mutex,NULL);
	pthread_mutex_init(&m_ForceAlert_Mutex,NULL);
    //最多缓冲3帧
    m_nFrameSize = 3;

//	m_bNight = false;

    m_nDetectTime = DETECT_AUTO;
    //默认只检测事件
    m_nDetectKind = DETECT_BEHAVIOR;

    m_bEventCapture = false;
    m_eCapType = CAPTURE_NO;

    m_bConnect = false;

   m_pJpgImage = NULL;

    m_nTrafficStatTime = 60;
    m_nCaptureTime = 5;

//	m_smImageData = NULL;

    m_imgSnap = NULL;
    m_imgPreSnap = NULL;
    m_imgComposeSnap = NULL;

    m_pResultFrameBuffer = NULL;

    m_img = NULL;
    m_imgPre = NULL;
    m_pExtentRegion = NULL;

    m_pImage = NULL;
	m_pImageFrame = NULL;
	m_pLogoImage = NULL;

    m_nExtent = 60;
    m_nWordPos = 0;
    m_nDayNight = 1;


    m_nDeinterlace = 1;

    m_fScaleX  = 1;
    m_fScaleY  = 1;

 	m_nMonitorID = 0;
    m_nHasLocalPreSet = 0;
	m_bTestResult = false;
	m_nFileID = -1;
    return;
}
//析构
CObjectBehaviorDetect::~CObjectBehaviorDetect()
{
    //检测帧列表互斥
    pthread_mutex_destroy(&m_Frame_Mutex);
    pthread_mutex_destroy(&m_preFrame_Mutex);
	pthread_mutex_destroy(&m_ForceAlert_Mutex);
    return;
}

//初始化检测数据，算法配置文件初始化
bool CObjectBehaviorDetect::Init(int nChannelID,int widthOrg,int heightOrg,int widthDst ,int heightDst)
{
    printf("bool CObjectBehaviorDetect::Init\n");
    //终止检测
    m_bTerminateThread = false;
	m_bTestResult = false;
	m_nFileID = -1;
    //完成检测
//	m_bFinishDetect = true;
    //通道ID
    m_nChannelID = nChannelID;

    //m_nPreSeq = 0;
    //m_preTrafficStatTime = 0;

    if( (widthOrg/widthDst) == (heightOrg/heightDst) )
    {
        m_nDeinterlace = 1; //帧图像
    }
    else
    {
        m_nDeinterlace = 2; //场图像
    }

    m_imgSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);
    m_imgPreSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);
    m_imgComposeSnap = cvCreateImage(cvSize(widthOrg*2,(heightOrg*m_nDeinterlace+m_nExtent)),8,3);

    m_img =  cvCreateImageHeader(cvSize(widthOrg, heightOrg+m_nExtent/m_nDeinterlace), 8, 3);
    m_imgPre =  cvCreateImageHeader(cvSize(widthOrg, heightOrg+m_nExtent/m_nDeinterlace), 8, 3);

    m_pImage = cvCreateImageHeader(cvSize(widthOrg, heightOrg), 8, 3);
	m_pImageFrame = cvCreateImage(cvSize(widthOrg, heightOrg*m_nDeinterlace), 8, 3);

    int nSize = m_img->widthStep*m_nExtent/m_nDeinterlace*sizeof(char);

	if(m_nExtent > 0)
	{
		m_pExtentRegion  = new char[nSize];
		memset(m_pExtentRegion,0,nSize);
	}

    m_pJpgImage = new BYTE[widthOrg*heightOrg];

	std::string strPath = "./logo.bmp";
	if(access(strPath.c_str(),F_OK) == 0)
	{
		m_pLogoImage = cvLoadImage(strPath.c_str(),-1);
		if(m_pLogoImage != NULL)
		{
			cvConvertImage(m_pLogoImage,m_pLogoImage,CV_CVTIMG_SWAP_RB);
		}
	}

    //分配秒图缓冲区
    nSize = sizeof(SRIP_DETECT_HEADER)+m_img->imageSize;
    printf("=================================nSize=%d\n",nSize);
    int nPlateSize = FRAMESIZE;
    m_pResultFrameBuffer = new BYTE[nSize*nPlateSize];
    for(int i=0; i < nPlateSize; i++)
    {
        m_chResultFrameBuffer[i] = m_pResultFrameBuffer + i*nSize;
    }
    m_nResultReadIndex = -1;
    m_nResultWriteIndex = -1;

    //获取xy方向缩放比
    m_fScaleX =  (double)widthOrg/widthDst;
    m_fScaleY = (double) (heightOrg*m_nDeinterlace)/heightDst;

    //流量统计初始化
    for(int i = 0; i<MAX_ROADWAY; i++)
    {
        uFluxAll[i]=0;
        uFluxPerson[i]=0;
        uFluxNoneVehicle[i]=0;
        uFluxVehicle[i]=0;
    }


    m_bReloadConfig = true;

    //文本初始化
    int nFontSize = 25;

    if(widthOrg < 1000)
    {
        nFontSize = 15;
    }
    else
    {
        nFontSize = 25;
    }
    m_cvText.Init(nFontSize);
    m_nWordPos = g_PicFormatInfo.nWordPos;//文字显示位置

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

    return true;
}

//释放检测数据，算法数据释放
bool CObjectBehaviorDetect::UnInit()
{
    //设置停止标志位
    m_bTerminateThread = true;

    //停止检测帧数据处理线程
    EndDetectThread();

    //停止事件录象线程
    if(m_bEventCapture)
    m_skpRoadRecorder.UnInit();

    //释放检测类
    m_objectdetect.mv_UnInit();

    //加锁
    pthread_mutex_lock(&m_Frame_Mutex);
    //清空检测列表
    m_ChannelFrameList.clear();
    //m_ResultFrameList.clear();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);

    //释放文本资源
    m_cvText.UnInit();

    if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
    {
        m_PreSetControl.UnInit();
    }

    if(m_imgSnap)
    {
        cvReleaseImage(&m_imgSnap);
        m_imgSnap = NULL;
    }

    if(m_imgComposeSnap)
    {
        cvReleaseImage(&m_imgComposeSnap);
        m_imgComposeSnap = NULL;
    }

    if(m_imgPreSnap)
    {
        cvReleaseImage(&m_imgPreSnap);
        m_imgPreSnap = NULL;
    }

    if(m_img)
    {
        cvReleaseImageHeader(&m_img);
        m_img = NULL;
    }

    if(m_imgPre)
    {
        cvReleaseImageHeader(&m_imgPre);
        m_imgPre = NULL;
    }

    if(m_pImage)
    {
        cvReleaseImageHeader(&m_pImage);
        m_pImage = NULL;
    }

	if(m_pImageFrame)
	{
		cvReleaseImage(&m_pImageFrame);
		m_pImageFrame = NULL;
	}

    if(m_pExtentRegion)
    {
        delete []m_pExtentRegion;
        m_pExtentRegion = NULL;
    }

    if(m_pJpgImage != NULL)
    {
        delete []m_pJpgImage;
        m_pJpgImage = NULL;
    }

	if(m_pLogoImage != NULL)
	{
		cvReleaseImage( &m_pLogoImage );
		m_pLogoImage = NULL;
	}

    //加锁
    pthread_mutex_lock(&m_preFrame_Mutex);
    if(m_pResultFrameBuffer)
    {
        delete []m_pResultFrameBuffer;
        m_pResultFrameBuffer = NULL;
    }
    //解锁
    pthread_mutex_unlock(&m_preFrame_Mutex);

    return true;
}


//启动数据处理线程
bool CObjectBehaviorDetect::BeginDetectThread()
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
    int nret=pthread_create(&m_nThreadId,&attr,ThreadDetectObjectBehavior,this);


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
void CObjectBehaviorDetect::EndDetectThread()
{
    //停止线程
    if(m_nThreadId != 0)
    {
        pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
    }
}

//事件检测处理
void CObjectBehaviorDetect::DealFrame()
{
    printf("m_bTerminateThread=%d\n",m_bTerminateThread);
    int count = 0;
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
            }
            count++;
        }
        //////////////////////////////////////////

        //处理一条数据
        DetectFrame();

        //1毫秒
        usleep(1000*1);
    }
}

//检测数据
bool CObjectBehaviorDetect::DetectFrame()
{
    //弹出一条帧图片
    std::string result = PopFrame();
    //无检测帧
    if(result.size() == 0) return true;

    //检测图片，并提交检测结果
    return OnFrame(result);

}

//添加一帧数据
bool CObjectBehaviorDetect::AddFrame(std::string& frame)
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
            printf("=======================exceed CObjectBehaviorDetect::AddFrame\r\n");
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

        AddResultFrameList((BYTE*)frame.c_str(),frame.size());
        return true;
    }
    return false;
}


//弹出一帧数据
std::string CObjectBehaviorDetect::PopFrame( )
{
    std::string response;

    // ID错误或者实例为空则返回
    if(this == NULL || m_nChannelID <= 0 )
        return response;

    //加锁
    pthread_mutex_lock(&m_Frame_Mutex);

    //判断是否有采集的数据帧
    if(m_ChannelFrameList.size() == 0)
    {
        //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);
        return response;
    }

    //取最早命令
    ListFrame::iterator it = m_ChannelFrameList.begin();
    //保存数据
    response = *it;
    //删除取出的采集帧
    m_ChannelFrameList.pop_front();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);
    return response;

}

//添加秒图
void CObjectBehaviorDetect::AddResultFrameList(BYTE* frame,int nSize)
{
    bool bAddPreFrame = false;
    //加锁
    pthread_mutex_lock(&m_preFrame_Mutex);
    if(m_nResultWriteIndex>-1)
    {
        SRIP_DETECT_HEADER* sHeader = (SRIP_DETECT_HEADER*)(frame);
        SRIP_DETECT_HEADER* sPreHeader = (SRIP_DETECT_HEADER*)(m_chResultFrameBuffer[m_nResultWriteIndex]);
        if(sHeader->uTimestamp >= sPreHeader->uTimestamp + 1)
        {
            int nPlateSize = FRAMESIZE;
            m_nResultWriteIndex++;
            m_nResultWriteIndex %= nPlateSize;

            bAddPreFrame = true;
        }
    }
    else//第一帧直接写入
    {
        m_nResultWriteIndex++;
        bAddPreFrame = true;
    }
    //printf("====bAddPreFrame======m_nResultWriteIndex==%d=====================m_img->imageSize=%d,sizeof(SRIP_DETECT_HEADER)=%d\n",m_nResultWriteIndex,m_img->imageSize,sizeof(SRIP_DETECT_HEADER));

    if(bAddPreFrame)
    {
       if(m_nWordPos == 0)
       {
           memcpy(m_chResultFrameBuffer[m_nResultWriteIndex],frame,nSize);
       }
       else
       {
           memcpy(m_chResultFrameBuffer[m_nResultWriteIndex],frame,sizeof(SRIP_DETECT_HEADER));
           memcpy(m_chResultFrameBuffer[m_nResultWriteIndex]+sizeof(SRIP_DETECT_HEADER)+m_img->widthStep*(m_nExtent/m_nDeinterlace),frame+sizeof(SRIP_DETECT_HEADER),nSize-sizeof(SRIP_DETECT_HEADER));
       }
    }


    //printf("======after====m_nResultWriteIndex==%d=====================m_img->imageSize=%d,sizeof(SRIP_DETECT_HEADER)=%d\n",m_nResultWriteIndex,m_img->imageSize,sizeof(SRIP_DETECT_HEADER));

    //解锁
    pthread_mutex_unlock(&m_preFrame_Mutex);
}

//获取秒图
SRIP_DETECT_HEADER CObjectBehaviorDetect::GetImageFromResultFrameList(UINT32 uTimeStamp,int  nTimeInterval)
{
    SRIP_DETECT_HEADER sPreHeader;

    int nMinTime = INT_MAX;
    int nIndex = -1;
    int nPlateSize = FRAMESIZE;

    if(nTimeInterval > FRAMESIZE)
    {
        nTimeInterval = FRAMESIZE;
    }

    //加锁
    pthread_mutex_lock(&m_preFrame_Mutex);
    for(int j=0; j < nPlateSize; j++)
    {
        SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(m_chResultFrameBuffer[j]);
        if(sDetectHeader)
        {
            if(uTimeStamp >= sDetectHeader->uTimestamp + nTimeInterval )
            {
                int detTime = uTimeStamp - sDetectHeader->uTimestamp;
                if(nMinTime >= detTime)
                {
                    nMinTime = detTime;
                    nIndex = j;
                }
            }
        }
    }

    if(nIndex < 0)
    {
        nIndex = (m_nResultWriteIndex+1)%nPlateSize;;
    }

    if(nIndex>=0)
    {
        SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(m_chResultFrameBuffer[nIndex]);
        sPreHeader.uTimestamp = sDetectHeader->uTimestamp;
        sPreHeader.uTime64 = sDetectHeader->uTime64;

        //cvSet(m_imgPreSnap, cvScalar( 0,0, 0 ));
        if(m_nDeinterlace==2)
        {
            cvSetData(m_imgPre,m_chResultFrameBuffer[nIndex]+sizeof(SRIP_DETECT_HEADER),m_imgPre->widthStep);
            cvResize(m_imgPre,m_imgPreSnap);
        }
        else
        {
            memcpy(m_imgPreSnap->imageData,m_chResultFrameBuffer[nIndex]+sizeof(SRIP_DETECT_HEADER),m_imgPre->imageSize);
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
		cvSetImageROI(m_imgPreSnap,rect);
		cvSet(m_imgPreSnap, cvScalar( 0,0, 0 ));
		cvResetImageROI(m_imgPreSnap);
    }

     //解锁
    pthread_mutex_unlock(&m_preFrame_Mutex);

    return sPreHeader;
}

//保存全景图像
int CObjectBehaviorDetect::SaveImage(IplImage* pImg,std::string strPicPath,int nIndex)
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

			//叠加数字水印
			#ifdef WATER_MARK
			std::string strWaterMark;
			GetWaterMark(m_pJpgImage,srcstep,strWaterMark);
			fwrite(strWaterMark.c_str(),strWaterMark.size(),1,fp);
			srcstep += strWaterMark.size();
			#endif

			fclose(fp);
		}
    }

    return srcstep;
}

//检测采集帧，并返回结果
bool CObjectBehaviorDetect::OnFrame(std::string& frame)
{
	//读取测试结果
	if(g_nFileID > 0)
	{
		//读取文件获取结果
		if(m_nFileID != g_nFileID)
		{
			m_nFileID = g_nFileID;
			m_listTestResult.clear();

			char filename[256] = {0};
			sprintf(filename,"B%d.data",g_nFileID);

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

		//输出测试结果
	if(m_bTestResult)
	{
		OutPutTestResult(frame);

		return true;
	}

    //取类型
    SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

    // 重新加载配置
    if(m_bReloadConfig)
    {
        vector<CvPoint> vImagePoint;
        vector<CvPoint3D32f> vWorldPoint;
        vector<event_param> DectorRegn;
        vector<CvPoint> pointstore;

        CvRect farrect = cvRect(0,0,100,100);
        CvRect nearrect = cvRect(0,0,200,200);

        //初始化车道配置
        if(!LoadRoadSettingInfo(vImagePoint,vWorldPoint,DectorRegn,pointstore,farrect,nearrect))
        {
            return false;
        }

        //设置车道检测参数
        //检测控制参数
        //需要参数结构转换
        vector<person_param_for_every_frame> DectorType;


        SRIP_CHANNEL_EXT sChannel;
        sChannel.uId = m_nChannelID;

        CXmlParaUtil xml;
        /*paraDetectList sParamIn;
        xml.LoadRoadParameter(sParamIn,sChannel);
        if(sParamIn.size()<=0)
        {
            return false;
        }
        paraDetectList::iterator it_d_b = sParamIn.begin();
        paraDetectList::iterator it_d_e = sParamIn.end();*/
        paraBehaviorList listParaBehavior;
        xml.LoadBehaviorParameter(listParaBehavior,sChannel);
        if(listParaBehavior.size()<=0)
        {
            return false;
        }
        paraBehaviorList::iterator it_d_b = listParaBehavior.begin();
        paraBehaviorList::iterator it_d_e = listParaBehavior.end();

        while(it_d_b != it_d_e)
        {
           // VEHICLE_PARAM_FOR_EVERY_FRAME para = *it_d_b;
           BEHAVIOR_PARAM ParaBehavior = *it_d_b;

            person_param_for_every_frame paraPerson;

            /*paraPerson.m_Derelict_event = para.m_bDiuQi;
            paraPerson.m_Passerby_event = para.m_bCross;
            paraPerson.m_Run_event = para.m_bPersonRun;
            paraPerson.m_Jam_event = para.m_bCrowd;
            paraPerson.m_Stoop_event = para.m_bStop;
            paraPerson.m_Against_event = para.m_bNixing;
            paraPerson.m_Appear_event = para.m_bObjectAppear;
            paraPerson.m_Inside_event = para.m_bBargeIn;
            paraPerson.m_Outside_event = para.m_bBeyondMark;
            paraPerson.m_StatFlux_event = false;
            paraPerson.fRunThreshold = para.m_fMaxRunSpeed;
            paraPerson.nPeopleNumberJam = para.m_nPersonCount;*/

            paraPerson.m_Derelict_event = ParaBehavior.bDerelict_event;
            paraPerson.m_Passerby_event = ParaBehavior.bPasserby_event;
            paraPerson.m_Run_event = ParaBehavior.bRun_event;
            paraPerson.m_Jam_event = ParaBehavior.bJam_event;
            paraPerson.m_Stoop_event = ParaBehavior.bStoop_event;
            paraPerson.m_Against_event = ParaBehavior.bAgainst_event;
            paraPerson.m_Appear_event = ParaBehavior.bAppear_event;
            paraPerson.m_Inside_event = ParaBehavior.bInside_event;
            paraPerson.m_Outside_event = ParaBehavior.bOutside_event;
            paraPerson.m_StatFlux_event = ParaBehavior.bStatFlux_event;
            paraPerson.fRunThreshold = ParaBehavior.fRunThreshold;
            paraPerson.nPeopleNumberJam = ParaBehavior.nPeopleNumberJam;

            paraPerson.m_Context_event = ParaBehavior.bContext_event;
            paraPerson.m_Smoke_event = ParaBehavior.bSmoke_event;
            paraPerson.m_Fire_event = ParaBehavior.bFire_event;
            paraPerson.m_Fight_event = ParaBehavior.bFight_event;
            paraPerson.m_HandBill_enent = ParaBehavior.bLeafLet_event;
			paraPerson.m_Density_event = ParaBehavior.bDensity_event;
			paraPerson.m_StatDensity_event = ParaBehavior.bDensity_event;

			paraPerson.m_Loitering_event = ParaBehavior.bLoitering_event;
			paraPerson.m_RemovedItem_event = ParaBehavior.bRemovedItem_event;
			paraPerson.m_Impropriate_event = ParaBehavior.bImpropriate_event;
			paraPerson.m_Seeper_event = ParaBehavior.bSeeper_event;
			paraPerson.m_Tailgating_event = ParaBehavior.bTailgating_event;

            printf("============paraPerson.m_Run_event=%d,paraPerson.m_Against_event=%d\n",paraPerson.m_Run_event,paraPerson.m_Against_event);

            DectorType.push_back(paraPerson);
            it_d_b++;
        }

        //释放检测类
        m_objectdetect.mv_UnInit();

        //设置图像宽高以及检测区域
        m_objectdetect.mv_Init( sDetectHeader.uWidth, sDetectHeader.uHeight*m_nDeinterlace, pointstore );

        //设置人在图中的宽高
       /* int nPwidth = 60;
        int nPheght = 160;
        m_objectdetect.mv_setpersonwidthandheight(nPwidth,nPheght);*/
        //
        m_objectdetect.mv_InitMaxSize(farrect,nearrect);
        //设置标定信息
        m_objectdetect.mv_SetCalibration(vImagePoint,vWorldPoint);

        //设置检测区域以及检测类型
        m_objectdetect.mv_SetRegAndType(DectorRegn,DectorType);

        //全局参数设置
        globe_paramer paramer;
        paramer.m_ShowTime = sChannel.nShowTime*1000;
        paramer.m_TimeStat = sChannel.uTrafficStatTime*1000;
        m_nTrafficStatTime = sChannel.uTrafficStatTime;
        m_objectdetect.mv_SetGlobeParamer(paramer);

		printf("CObjectBehaviorDetect m_nTrafficStatTime=%d\n",m_nTrafficStatTime);

        //保存相关配置
        m_objectdetect.mv_SaveSettingConfig();

        if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
        {
            m_PreSetControl.Init(m_nChannelID);
        }

        //重新配置完成
        m_bReloadConfig = false;
        LogNormal("通道[%d]重新加载配置\r\n",m_nChannelID);
    }
    ///////////////////////////////////

    printf("before mv_Detecteveryframe\n");
    //检测分析
    char* data = (char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER));
    cvSetData(m_pImage,data,m_pImage->widthStep);
    cvConvertImage(m_pImage,m_pImage,CV_CVTIMG_SWAP_RB);
	
	if(m_nDeinterlace == 2)
	{
		cvResize(m_pImage,m_pImageFrame);
	}

    if(g_nKeyBoardID > 0 && g_ytControlSetting.nNeedControl == 1)
    {
        if(!m_PreSetControl.GetObjMovingState())
        {
			if(m_nDeinterlace == 2)
			{
				m_objectdetect.mv_Detecteveryframe(m_pImageFrame, sDetectHeader.uTime64/1000,sDetectHeader.uSeq, sDetectHeader.uTimestamp );
			}
			else
			{
				m_objectdetect.mv_Detecteveryframe(m_pImage, sDetectHeader.uTime64/1000,sDetectHeader.uSeq, sDetectHeader.uTimestamp );
			}  
        }
        else
        {
            //回归远景预置位
            m_PreSetControl.GoToRemotePreSet(sDetectHeader.uTimestamp);
        }
    }
    else
    {
		if(m_nDeinterlace == 2)
		{
			m_objectdetect.mv_Detecteveryframe(m_pImageFrame, sDetectHeader.uTime64/1000,sDetectHeader.uSeq, sDetectHeader.uTimestamp );
		}
		else
		{
			m_objectdetect.mv_Detecteveryframe(m_pImage, sDetectHeader.uTime64/1000,sDetectHeader.uSeq, sDetectHeader.uTimestamp );
		}
    }
    cvConvertImage(m_pImage,m_pImage,CV_CVTIMG_SWAP_RB);


    printf("after mv_Detecteveryframe\n");
    //获取检测结果
    vector<skip_event_out_result> list_DetectOut;
    m_objectdetect.mv_GetEventResult(list_DetectOut);

	FORCEALERT *pAlert = GetForceAlert();

	if (pAlert)
	{
		skip_event_out_result sor;
		sor.nChannel = 1;
		sor.eRtype =  static_cast<EVENT_RESULT_TYPE>(pAlert->nAlertType);
		sor.x = pAlert->nX;
		sor.y = pAlert->nY;
		sor.bShow = false;

		list_DetectOut.push_back(sor);

		delete pAlert;
	}

	bool bDetect = CheckDetectTime();

	if(bDetect)
	{
		//检测完成，处理结果
		if(list_DetectOut.size()>0)
		{
			OutPutResult(frame,list_DetectOut);
		}
	}

    return true;
}

//输出流量统计结果
void CObjectBehaviorDetect::OutPutStatisticResult(skip_event_out_result& result,UINT32 uTimestamp)
{
	UINT32 uRoadIndex = 0;
	UINT32 uObjectNum = 0;
	UINT32 uObjectAll = 0;

	//根据车道号获取车道逻辑编号
     RoadIndexMap::iterator it_p = m_roadMap.find(result.nChannel);
     if(it_p != m_roadMap.end())
     {
        uRoadIndex = it_p->second.nVerRoadIndex;
     }

	RECORD_STATISTIC statistic;//统计结果

	statistic.uTime = uTimestamp;
    statistic.uStatTimeLen = m_nTrafficStatTime;

	uRoadIndex =  uRoadIndex<<16;
    statistic.uRoadType[result.nChannel-1] = uRoadIndex|OBJECT_ROAD;

	vector<amountcounters>::iterator it_b = result.m_amount.begin();
	vector<amountcounters>::iterator it_e = result.m_amount.end();
	int i = 0;
	while(it_b != it_e)
	{
		amountcounters acs = *(it_b);

		//高16位正向流量总数，低16位为所有物体数目

		uObjectAll = acs.nTotalCounters;
		uObjectNum = acs.nSDCounters;
		
		if(i == 0)//暂时一个车道只支持2条流量线
		{
			statistic.uFlux[result.nChannel-1] = (uObjectNum | uObjectAll);
		}
		else if(i == 1)
		{
			statistic.uFluxCom[result.nChannel-1] = (uObjectNum | uObjectAll);
		}
		else
		{
			break;
		}
		
		if(acs.fDensity > 0)//密度统计
		{
			if(i == 0)//暂时一个车道只支持2条流量线
			{
				statistic.uOccupancy[result.nChannel-1] = acs.fDensity;
			}
			else
			{
				break;
			}
		}

		it_b++;
		i++;
	}

	g_skpDB.SaveStatisticInfo(m_nChannelID,statistic,OBJECT_ROAD);
}


//添加事件录象缓冲数据
bool CObjectBehaviorDetect::AddVideoFrame(std::string& frame)
{
    {
       if(m_bEventCapture)
        m_skpRoadRecorder.AddFrame(frame);
    }

    return true;
}

//修改事件录像
void CObjectBehaviorDetect::ModifyEventCapture(bool bEventCapture)
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

//设置白天晚上还是自动判断
void CObjectBehaviorDetect::SetChannelDetectTime(CHANNEL_DETECT_TIME dType)
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
void CObjectBehaviorDetect::GetEventType(EVENT_RESULT_TYPE& eType,DETECT_RESULT_TYPE& rType)
{
    switch (eType)
    {
        case EVENT_RESULT_EVENT_DERELICT:
        {
            rType = DETECT_RESULT_EVENT_DERELICT;
            break;
        }
        case EVENT_RESULT_EVENT_PASSERBY:
        {
            rType = DETECT_RESULT_EVENT_PASSERBY;
            break;
        }
        case EVENT_RESULT_EVENT_PERSON_RUN:
        {
            rType = DETECT_RESULT_PERSON_RUN;
            break;
        }
        case EVENT_RESULT_EVENT_PERSON_JAM:
        {
            rType = DETECT_RESULT_CROWD;
            break;
        }
        case EVENT_RESULT_EVENT_PERSON_STOP:
        {
            rType = DETECT_RESULT_EVENT_PERSON_STOP;
            break;
        }
        case EVENT_RESULT_EVENT_PERSON_AGAINST:
        {
            rType = DETECT_RESULT_EVENT_PERSON_AGAINST;
            break;
        }
         case EVENT_RESULT_EVENT_PERSON_APPEAR:
        {
            rType = DETECT_RESULT_EVENT_PERSON_APPEAR;
            break;
        }
        case EVENT_RESULT_EVENT_PERSON_INSIDE://行人闯入
        {
            rType = DETECT_RESULT_EVENT_INSIDE;
            break;
        }
		case EVENT_RESULT_EVENT_PERSON_BEYONDSIDE://行人闯出
        {
            rType = DETECT_RESULT_EVENT_BEYONDSIDE;
            break;
        }
        case EVENT_RESULT_EVENT_PERSON_OUTSIDE:
        {
            rType = DETECT_RESULT_EVENT_OUTSIDE;
            break;
        }
        case EVENT_RESULT_EVENT_CONTEXT_DETECT:
        {
            rType = DETECT_RESULT_EVENT_HOLD_BANNERS;
            break;
        }
        case EVENT_RESULT_EVENT_FIRE:
        {
            rType = DETECT_RESULT_EVENT_FIRE;
            break;
        }
        case EVENT_RESULT_EVENT_SMOKE:
        {
            rType = DETECT_RESULT_EVENT_SMOKE;
            break;
        }
        case EVENT_RESULT_EVENT_FIGHT:
        {
            rType = DETECT_RESULT_EVENT_FIGHT;
            break;
        }
        case EVENT_RESULT_EVENT_HANDBILL:
        {
            rType = DETECT_RESULT_EVENT_DISTRIBUTE_LEAFLET;
            break;
        }
		case EVENT_RESULT_STAT_FLUX:
		{
			rType = DETECT_RESULT_STAT_FLUX;
			break;
		}
        default:
        {
            rType = DETECT_RESULT_ALL;
            break;
        }
    }
}

/* 函数介绍：在事件结果图像上叠加文本信息（日期、地点等）
 * 输入参数：uTimestamp-时间戳
 * 输出参数：无
 * 返回值：无
 */
void CObjectBehaviorDetect::PutTextOnImage(IplImage* pImg,RECORD_EVENT event,SRIP_DETECT_HEADER* sPreHeader,int nIndex)
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
    sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str());//
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
        sprintf(chOut,"时间:%s.%03d  ",strTime.c_str(),((sPreHeader->uTime64)/1000)%1000);
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
            if(event.uCode == DETECT_RESULT_EVENT_PERSON_STOP)
            {
                strEvent = "行人停留";
            }
            else if( (event.uCode == DETECT_RESULT_EVENT_PERSON_AGAINST))
            {
                strEvent = "行人逆行";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_DERELICT)
            {
                strEvent = "遗弃物";
            }
            else if( (event.uCode == DETECT_RESULT_EVENT_PASSERBY))
            {
                strEvent = "行人横穿";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_HOLD_BANNERS)
            {
                strEvent = "拉横幅";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_DISTRIBUTE_LEAFLET)
            {
                strEvent = "散传单";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_FIGHT)
            {
                strEvent = "打架";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_INSIDE)
            {
                strEvent = "闯入";
            }
			else if(event.uCode == DETECT_RESULT_EVENT_BEYONDSIDE)
            {
                strEvent = "闯出";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_OUTSIDE)
            {
                strEvent = "越界";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_SMOKE)
            {
                strEvent = "烟";
            }
            else if(event.uCode == DETECT_RESULT_EVENT_FIRE)
            {
                strEvent = "火";
            }
            else if(event.uCode == DETECT_RESULT_CROWD)
            {
                strEvent = "人群聚集";
            }
            else if(event.uCode == DETECT_RESULT_PERSON_RUN)
            {
                strEvent = "行人奔跑";
            }
            sprintf(chOut,"行为类型:%s",strEvent.c_str());
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
	
	if(pImg->width == m_imgSnap->width)
	{
		PutLogoOnImage(pImg);
	}
	else if(pImg->width > m_imgSnap->width)
	{
		if(nIndex > 0)
		{
			PutLogoOnImage(pImg);
		}
	}
}

//查找5秒前的图片数据
SRIP_DETECT_HEADER CObjectBehaviorDetect::GetPreImage(UINT32 uTimeStamp,DETECT_RESULT_TYPE  dType)
{
    SRIP_DETECT_HEADER sPreHeader;

    return sPreHeader;
}

//设置检测类型
void CObjectBehaviorDetect::SetDetectKind(CHANNEL_DETECT_KIND nDetectKind)
{
    if(m_nDetectKind != nDetectKind)
    {
        m_nDetectKind = nDetectKind;
        m_bReloadConfig = true;
    }
}

//载入通道设置
bool CObjectBehaviorDetect::LoadRoadSettingInfo(vector<CvPoint>& vImagePoint, vector<CvPoint3D32f>& vWorldPoint,vector<event_param>& DectorRegn,vector<CvPoint>& PoinStore,CvRect& farrect,CvRect& nearrect)
{
    LIST_CHANNEL_INFO list_channel_info;
    CXmlParaUtil xml;
    if(xml.LoadRoadSettingInfo(list_channel_info,m_nChannelID))
    {
        bool bLoadCalibration = false;
        bool bLoadRoad = false;
        bool bLoadPerson = false;

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

        CvPoint ptImage;
        CvPoint3D32f ptWorld;

        CvPoint2D32f ptEvent;


        m_roadMap.clear();
        printf("list_channel_info.size=%d\n",list_channel_info.size());
        while(it_b != it_e)
        {
            mvrgnstru rgnStru;
            mvline  lineStru;
            event_param paramEvent;

            int i = 0;
            int j = 0;
            printf("m_fScaleX=%f,m_fScaleY=%f,m_nDeinterlace=%d\n",m_fScaleX,m_fScaleY,m_nDeinterlace);
            CHANNEL_INFO channel_info = *it_b;

            ROAD_INDEX_INFO road_index_info;
            //车道编号
            paramEvent.nIndex = channel_info.chProp_index.value.nValue;
            road_index_info.nVerRoadIndex = channel_info.chProp_name.value.nValue;
            //车道方向
            int nDirection = channel_info.chProp_direction.value.nValue;
            if(nDirection < 180)
            {
                road_index_info.nDirection = 0;
            }
            else
            {
                road_index_info.nDirection = 1;
            }
            m_roadMap.insert(RoadIndexMap::value_type(paramEvent.nIndex,road_index_info));

            //车道方向中心点
            printf("channel_info.chProp_direction.point.x=%f,channel_info.chProp_direction.point.y=%f\n",channel_info.chProp_direction.point.x,channel_info.chProp_direction.point.y);

            //标定区域
            if(!bLoadCalibration)
            {
                i = 0;
                //矩形区域（4个点）
                it_begin = channel_info.calibration.region.listPT.begin();
                it_end = channel_info.calibration.region.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    //image cor
                    ptImage.x = it_begin->x;
                    ptImage.y = it_begin->y;
                    vImagePoint.push_back(ptImage);

                    //world cor
                    if(i==0)
                    {
                        ptWorld.x = 0;
                        ptWorld.y = 0;
                    }
                    else if(i==1)
                    {
                        ptWorld.x = channel_info.calibration.length;
                        ptWorld.y = 0;
                    }
                    else if(i==2)
                    {
                        ptWorld.x = channel_info.calibration.length;
                        ptWorld.y = channel_info.calibration.width;
                    }
                    else if(i==3)
                    {
                        ptWorld.x = 0;
                        ptWorld.y = channel_info.calibration.width;
                    }
                    ptWorld.z = 0;
                    vWorldPoint.push_back(ptWorld);
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
                    ptImage.x = it_begin->x;
                    ptImage.y = it_begin->y;
                    vImagePoint.push_back(ptImage);

                    //world cor
                    ptWorld.x = it_32fb->x;
                    ptWorld.y = it_32fb->y;
                    ptWorld.z = 0;
                    vWorldPoint.push_back(ptWorld);

                    it_32fb++;
                    it_begin++;
                }
                bLoadCalibration = true;
            }

            //道路区域
            if(!bLoadRoad)
            {
                int nRoadIndex = channel_info.roadRegion.chProperty.value.nValue;
                it_begin = channel_info.roadRegion.listPT.begin();
                it_end = channel_info.roadRegion.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    ptImage.x = it_begin->x;
                    ptImage.y = it_begin->y;

                    PoinStore.push_back(ptImage);
                }
                bLoadRoad = true;
            }

            //车道方向
            lineStru.vPList.clear();
            ptImage.x = channel_info.chProp_direction.ptBegin.x;
            ptImage.y = channel_info.chProp_direction.ptBegin.y;
            lineStru.vPList.push_back(ptImage);
            ptImage.x = channel_info.chProp_direction.ptEnd.x;
            ptImage.y = channel_info.chProp_direction.ptEnd.y;
            lineStru.vPList.push_back(ptImage);
            paramEvent.oriRgn.push_back(lineStru);


            //车道区域
            rgnStru.pPoints.clear();
            it_begin = channel_info.chRegion.listPT.begin();
            it_end = channel_info.chRegion.listPT.end();
            for(; it_begin != it_end; it_begin++)
            {
                ptEvent.x = it_begin->x;
                ptEvent.y = it_begin->y;

                rgnStru.pPoints.push_back(ptEvent);
            }
            paramEvent.chanRgn.push_back(rgnStru);

            //遗弃物检测区域
            it_rb = channel_info.dropRegion.listRegionProp.begin();
            it_re = channel_info.dropRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                rgnStru.pPoints.clear();
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();

                for( ; item_b!=item_e; item_b++)
                {
                    ptEvent.x = (item_b->x);
                    ptEvent.y = (item_b->y);

                    rgnStru.pPoints.push_back(ptEvent);
                }

                paramEvent.dropRgn.push_back(rgnStru);
            }
             //闯入区域
             it_rb = channel_info.BargeInRegion.listRegionProp.begin();
            it_re = channel_info.BargeInRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                rgnStru.pPoints.clear();
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                for( ; item_b!=item_e; item_b++)
                {
                    ptEvent.x = (item_b->x);
                    ptEvent.y = (item_b->y);

                    rgnStru.pPoints.push_back(ptEvent);
                }

				//闯入方向
				if(it_rb->directionListPt.size() >= 2)
				{
					rgnStru.nAttribution = SINGLE;
				}
				else
				{
					rgnStru.nAttribution = ALLDERICTION;
				}
				
				item_b = it_rb->directionListPt.begin();
                item_e = it_rb->directionListPt.end();
				i = 0;
				for( ; item_b!=item_e; item_b++)
				{
					ptImage.x = (item_b->x);
                    ptImage.y = (item_b->y);
					
					if(i == 0)
					{
						rgnStru.ptStart = ptImage;
					}
					else
					{
						rgnStru.ptEnd = ptImage;
					}

					i++;
				}

                paramEvent.InsideRgn.push_back(rgnStru);
            }

            //越界区域
            it_rb = channel_info.BeyondMarkRegion.listRegionProp.begin();
            it_re = channel_info.BeyondMarkRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                lineStru.vPList.clear();
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                for( ; item_b!=item_e; item_b++)
                {
                    ptImage.x = (item_b->x);
                    ptImage.y = (item_b->y);
                    lineStru.vPList.push_back(ptImage);
                }
                paramEvent.beyondMarkLine.push_back(lineStru);
            }

            //流量监测线
            it_rb = channel_info.AmountLine.listRegionProp.begin();
            it_re = channel_info.AmountLine.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                lineStru.vPList.clear();
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                for( ; item_b!=item_e; item_b++)
                {
                    ptImage.x = (item_b->x);
                    ptImage.y = (item_b->y);
                    lineStru.vPList.push_back(ptImage);
                }

				//流量方向
				if(it_rb->directionListPt.size() >= 2)
				{
					lineStru.nAttribution = SINGLE;
				}
				else
				{
					lineStru.nAttribution = ALLDERICTION;
				}
				
				item_b = it_rb->directionListPt.begin();
                item_e = it_rb->directionListPt.end();
				i = 0;
				for( ; item_b!=item_e; item_b++)
				{
					ptImage.x = (item_b->x);
                    ptImage.y = (item_b->y);
					
					if(i == 0)
					{
						lineStru.ptStart = ptImage;
					}
					else
					{
						lineStru.ptEnd = ptImage;
					}
					i++;
				}

                paramEvent.amountLine.push_back(lineStru);
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

                bLoadPerson = true;
            }

			//密度区域
            it_rb = channel_info.DensityRegion.listRegionProp.begin();
            it_re = channel_info.DensityRegion.listRegionProp.end();
            for(; it_rb != it_re; it_rb++)
            {
                rgnStru.pPoints.clear();
                item_b = it_rb->listPt.begin();
                item_e = it_rb->listPt.end();
                for( ; item_b!=item_e; item_b++)
                {
                    ptEvent.x = (item_b->x);
                    ptEvent.y = (item_b->y);

                    rgnStru.pPoints.push_back(ptEvent);
                }
                paramEvent.dDensity.push_back(rgnStru);
            }


            DectorRegn.push_back(paramEvent);

            it_b++;
        }

        return true;
    }
    else
    {
        printf("读取车道参数失败!\r\n");
        return false;
    }
}

//获取区域中心点
void CObjectBehaviorDetect::GetCenterPoint(Point32fList ptList,CPoint32f& ptCenter)
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
bool CObjectBehaviorDetect::GetObjMovingState()
{
    return m_PreSetControl.GetObjMovingState();
}

//设置相机编号
void CObjectBehaviorDetect::SetCameraID(int nCameraID)
{
    m_nCameraId = nCameraID;
    m_PreSetControl.SetCameraID(nCameraID);
}

//设置Monitor编号
void CObjectBehaviorDetect::SetMonitorID(int nMonitorID)
{
    m_nMonitorID = nMonitorID;
    m_PreSetControl.SetMonitorID(nMonitorID);
}

//设置相机类型
void CObjectBehaviorDetect::SetCameraType(int  nCameraType)
{
    m_nCameraType = nCameraType;
    m_PreSetControl.SetCameraType(nCameraType);
}

//输出目标检测结果
void CObjectBehaviorDetect::OutPutResult(std::string& frame,vector<skip_event_out_result>& list_DetectOut)
{
	 SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());
	 //事件统计列表
        vector<skip_event_out_result>::iterator it_b = list_DetectOut.begin();
        vector<skip_event_out_result>::iterator it_e = list_DetectOut.end();

        bool flag = false;//判断是否真实事件
        bool bStat = false;//判断是否统计
      
        RECORD_EVENT event;
        std::string strPicPath,strPath,strTmpPath,strEvent,strStat;
        //
        sDetectHeader.uDetectType = SRIP_DETECT_EVENT;
        //	strEvent.append((char*)sDetectHeader,sizeof(SRIP_DETECT_HEADER));

        UINT32 uObjectNum = 0;
        UINT32 uObjectAll = 0;
        UINT32 uRoadIndex = 0;
		
		if(m_nExtent > 0)
		{
			//需要append文本区域
			if(m_nWordPos == 0)
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
            //根据车道号获取车道逻辑编号
            RoadIndexMap::iterator it_p = m_roadMap.find(it_b->nChannel);
            if(it_p != m_roadMap.end())
            {
                uRoadIndex = it_p->second.nVerRoadIndex;
            }
			else
			{
				uRoadIndex = it_b->nChannel;
			}

            //事件
            if(it_b->eRtype < EVENT_RESULT_STAT_FLUX)
            {
                //行驶方向
                int nDirection = 0;

				if(it_p != m_roadMap.end())
				{
					if(it_p->second.nDirection != nDirection)
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
				}
				else
				{
					event.uDirection = m_nDirection;
				}
                event.uRoadWayID = uRoadIndex;//车道编号

                //存数据库的坐标需要乘缩放比
                event.uPosX = (int)it_b->x;     //事件发生横坐标
                event.uPosY = (int)it_b->y;     //事件发生纵坐标
                if(m_nWordPos == 1)
                event.uPosY += m_nExtent;
                //printf("=================uRoadIndex=%d=====m_fScaleX=%f,m_fScaleY=%f\n",uRoadIndex,m_fScaleX,m_fScaleY);

                /*event.uColor1 = it_b->color.nColor1;
                event.uColor2 = it_b->color.nColor2;
                event.uColor3 = it_b->color.nColor3;
                event.uWeight1 = it_b->color.nWeight1;
                event.uWeight2 = it_b->color.nWeight2;
                event.uWeight3 = it_b->color.nWeight3;*/

                event.uSpeed = it_b->dVelocity;
				event.uDensity = it_b->dDensity;
				event.uType = 1;

                //类型
                GetEventType(it_b->eRtype,eType);
                event.uCode = (int)eType;

                event.uVideoBeginTime = sDetectHeader.uTimestamp-5;//事件发生前5秒
                event.uMiVideoBeginTime = 0;
                event.uVideoEndTime = sDetectHeader.uTimestamp + m_nCaptureTime - 5;//
                event.uMiVideoEndTime = 0;
                event.uEventBeginTime = sDetectHeader.uTimestamp;
                event.uMiEventBeginTime = (sDetectHeader.uTime64/1000)%1000;
                event.uEventEndTime = sDetectHeader.uTimestamp+5;
                event.uMiEventEndTime = 0;

                //真实事件才写数据库
                if(!(it_b->bShow))
                {
					bool bAppear = false;

					if(it_b->eRtype == DETECT_RESULT_EVENT_PERSON_APPEAR)
					{
						bAppear = true;
					}

                    //保存图片
                    if(m_nDeinterlace==2)
                    {
                        cvResize(m_img,m_imgSnap);
                    }
                    else
                    {
                        //memcpy(m_imgSnap->imageData,m_img->imageData,m_img->imageSize);
                        cvCopy(m_img,m_imgSnap);
                    }

					if(m_bTestResult)
					{
						CvPoint center;
						center.x= event.uPosX;
						center.y = event.uPosY;
						int radius = 25;
						CvScalar color = cvScalar(255,0,0);
						cvCircle(m_imgSnap,center,radius,color,2);
					}

                    //需要判断检测结果类型，如果是违法行为需要保存两张图片
                    if( (g_nSaveImageCount == 2) && (!bAppear))//违章行为
                    {
                        //查找5秒前的图片数据
                        int  nTimeInterval = 1;
                        UINT32 uTimestamp = sDetectHeader.uTimestamp;

                        SRIP_DETECT_HEADER sPreHeader = GetImageFromResultFrameList(uTimestamp,nTimeInterval);

						event.uTime2 = sPreHeader.uTimestamp;
                        event.uMiTime2 = ((sPreHeader.uTime64)/1000)%1000;

						 //拼图
                        event.uPicWidth = m_imgComposeSnap->width;  //事件快照宽度
                        //事件快照高度
                        event.uPicHeight = m_imgComposeSnap->height;

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
                    else//卡口只需要一张图片
                    {
						event.uPicWidth = m_imgSnap->width;  //事件快照宽度
                        //事件快照高度
                        event.uPicHeight = m_imgSnap->height;
                        //叠加信息
                        PutTextOnImage(m_imgSnap,event);
                    }

                    //加锁
                    pthread_mutex_lock(&g_Id_Mutex);
                    if(!flag && (it_b->eRtype != EVENT_RESULT_EVENT_PERSON_APPEAR))
                    {
                        if(m_eCapType == CAPTURE_FULL)
                        {
                             //直接从全天录象中获取事件录象视频
                             strPath = g_FileManage.GetEncodeFileName();

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
                                        //g_skpDB.DeleteOldRecord(strPath,true,true);
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

                    //需要判断检测结果类型，如果是违法行为需要保存两张图片
                    if( (g_nSaveImageCount == 2) && (!bAppear))//违章行为
                    {
                        if(g_nPicMode ==0 )
                        {
                            //保存图片
                            event.uPicSize = SaveImage(m_imgComposeSnap,strPicPath);
                        }
                        else
                        {
                            event.uPicSize = SaveImage(m_imgSnap,strPicPath);//数据库中保存主识别图片的大小
                            SaveImage(m_imgPreSnap,strPicPath,1);//辅助识别图片由文件大小减去主识别图片大小即可获得
                        }
                    }
                    else//卡口只需要一张图片
                    {
                        //保存图片
                        event.uPicSize = SaveImage(m_imgSnap,strPicPath);
                    }
                    //删除已经存在的记录
                        g_skpDB.DeleteOldRecord(strPicPath,false,false);
                    //记录检测数据,保存事件
                    if(nSaveRet>0)
					{
						bool bSendEventToCenter = true;
						
						#ifdef SENSITIVE
						if(it_b->nSensitivity == 1)//高灵敏度不主动发送给中心端
						{
							bSendEventToCenter = false;
						}
						#endif
                        g_skpDB.SaveTraEvent(sDetectHeader.uChannelID,event,0,0,bSendEventToCenter);
					}
                }
                //发往客户端的坐标需要乘缩放比
				if(g_VideoFormatInfo.nSendH264 == 0)
				{
					event.uPosX = (int)it_b->x/m_fScaleX;     //事件发生横坐标
					event.uPosY = (int)it_b->y/m_fScaleY;     //事件发生纵坐标
				}
				else
				{
					event.uPosX = (int)it_b->x*(g_nVideoWidth/m_img->width);     //事件发生横坐标
					event.uPosY = (int)it_b->y*(g_nVideoHeight/(m_img->height-m_nExtent/m_nDeinterlace));     //事件发生纵坐标
				}
                //利用保留字段存储bshow和缩放比
                memcpy(event.chReserved,&it_b->bShow,sizeof(bool));
                strEvent.append((char*)&event,sizeof(RECORD_EVENT));

				//发送检测结果给中心端
				if(11 == g_nServerType)
				{
					//g_BJServer.AddEvent(event,m_nCameraId);
				}	
            }
			else if(it_b->eRtype == EVENT_RESULT_STAT_FLUX||//流量统计
				    it_b->eRtype == EVENT_RESULT_STAT_DENSITY)//密度统计
			{
				//LogNormal("it_b->eRtype=%d,FLUX=%d,DENSITY=%d\n",it_b->eRtype,EVENT_RESULT_STAT_FLUX,EVENT_RESULT_STAT_DENSITY);
				//暂时一个车道只支持2条流量线
                //OutPutStatisticResult(*it_b,sDetectHeader.uTimestamp);
			}
            
            it_b++;
        }

        if(strEvent.size()>0)
            strEvent.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));

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
                    strEventEx.append((char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER)+m_img->widthStep*(m_nExtent/m_nDeinterlace)),sDetectHeader.uWidth*sDetectHeader.uHeight*3);
                }
                //通知录象线程进行事件录象;
                if(m_eCapType != CAPTURE_FULL)
                {
                    printf("strEvent.size()=%d,m_skpRoadRecorder.AddEvent(strEventEx)=%d\n",strEvent.size(),strEventEx.size());
					//LogTrace("AddEvent.log", "=222===m_skpRoadRecorder.AddEvent=m_eCapType=%d=strEventEx.size()=%d\n", m_eCapType, strEventEx.size());

                    m_skpRoadRecorder.AddEvent(strEventEx);
                }
            }
        }

        //printf("======================================m_bConnect=%d,strEvent.size()=%d\n",m_bConnect,strEvent.size());
        //////有连接才发送
        if(m_bConnect)
        {
            //发送事件
            if(strEvent.size()>0)
                g_skpChannelCenter.AddResult(strEvent);
        }
}

//输出测试结果
void CObjectBehaviorDetect::OutPutTestResult(std::string& frame)
{
	if(m_listTestResult.size() > 0)
	{
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

				vector<skip_event_out_result> list_DetectOut;

				skip_event_out_result result;

				result.nChannel = it_b->uRoadWayID;//车道编号

				//事件类型
				if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_DERELICT)
				{
					result.eRtype = EVENT_RESULT_EVENT_DERELICT;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_PASSERBY)
				{
					result.eRtype = EVENT_RESULT_EVENT_PASSERBY;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_PERSON_RUN)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_RUN;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_CROWD)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_JAM;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_PERSON_STOP)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_STOP;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_PERSON_AGAINST)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_AGAINST;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_INSIDE)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_INSIDE;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_BEYONDSIDE)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_BEYONDSIDE;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_OUTSIDE)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_OUTSIDE;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_FIRE)
				{
					result.eRtype = EVENT_RESULT_EVENT_FIRE;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_SMOKE)
				{
					result.eRtype = EVENT_RESULT_EVENT_SMOKE;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_FIGHT)
				{
					result.eRtype = EVENT_RESULT_EVENT_FIGHT;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_HOLD_BANNERS)
				{
					result.eRtype = EVENT_RESULT_EVENT_CONTEXT_DETECT;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_DISTRIBUTE_LEAFLET)
				{
					result.eRtype = EVENT_RESULT_EVENT_HANDBILL;
				}
				else if(it_b->uViolationType == (DETECT_RESULT_TYPE)DETECT_RESULT_EVENT_PERSON_APPEAR)
				{
					result.eRtype = EVENT_RESULT_EVENT_PERSON_APPEAR;
				}
				

				result.x = it_b->uPosLeft;     //事件发生横坐标
				result.y = it_b->uPosTop;     //事件发生纵坐标

				result.dVelocity = it_b->uSpeed;//速度		

				result.bShow = false;

				list_DetectOut.push_back(result);

				OutPutResult(frame,list_DetectOut);

				m_listTestResult.erase(it_b);
				break;
			}
			it_b++;
		}
	}
}

//读取测试结果文件
bool CObjectBehaviorDetect::LoadTestResult(char* filename)
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

//叠加LOGO信息
void CObjectBehaviorDetect::PutLogoOnImage(IplImage* pImg)
{
		if(m_pLogoImage != NULL && pImg != NULL)
		{
			int nWidth = m_pLogoImage->width;
			int nHeight = m_pLogoImage->height;

			CvRect rect;
            rect.x = pImg->width - nWidth - 10;
            rect.y = 10;
            rect.width = nWidth;
            rect.height = nHeight;
			
			if(nWidth > 0 && nHeight > 0)
			{
				cvSetImageROI(pImg,rect);
				cvCopy(m_pLogoImage,pImg);
				cvResetImageROI(pImg);
			}
		}
}

bool CObjectBehaviorDetect::AddForceAlert(FORCEALERT *pAlert)
{
	if (!pAlert)
	{
		return false;
	}

	pthread_mutex_lock(&m_ForceAlert_Mutex);

	m_deqAlert.push_back(pAlert);

	pthread_mutex_unlock(&m_ForceAlert_Mutex);

	return true;
}

FORCEALERT *CObjectBehaviorDetect::GetForceAlert()
{
	FORCEALERT *pAlert = NULL;

	pthread_mutex_lock(&m_ForceAlert_Mutex);

	if (m_deqAlert.begin() != m_deqAlert.end())
	{
		pAlert = m_deqAlert.front();

		m_deqAlert.pop_front();
	}

	pthread_mutex_unlock(&m_ForceAlert_Mutex);

	return pAlert;
}

#endif


