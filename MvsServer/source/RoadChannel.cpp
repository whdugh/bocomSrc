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

#include "RoadChannel.h"
#include "Common.h"
#include "CommonHeader.h"
#include "ippi.h"
#include "ippcc.h"
#include <dirent.h>
#include "XmlParaUtil.h"
#ifndef ALGORITHM_YUV
#include "mvGetCameraControlPara.h"
#endif
#include "ximage.h"

//#define FASTSPEED_TEST//模拟快速场景
//#define CAMERA_CONTROL

//#define DEBUG_CHANNEL

//开启1小时未收到数据,重启程序
#ifndef REBOOT_DETECT
	#define REBOOT_DETECT
#endif

#ifdef LINUX
extern "C"
{
    void checkID();
    void checkHD();
};
#endif
//构造
CSkpChannelEntity::CSkpChannelEntity()
{
    //视频宽
    m_uWidth = 0;
    //视频高
    m_uHeight = 0;

    //初始化空
    m_chImageData = NULL;
    m_smImageData = NULL;
    m_captureImageData = NULL;
    m_JpgImageData = NULL;
    m_JpgImageData2 = NULL;

    m_imgCotext = NULL;

    //线程结束标志
    m_bEndDeal = false;

    m_nThreadId = 0;
	m_nCameraControlThreadId = 0;

    m_bOpen = false;
    m_nStatus = 0;

    m_bConnect = false;

    m_pAbstractCamera = NULL;
    m_pCameraFactory = NULL;

    m_imgCurr = NULL;

    m_tmLastCheck = 0;

    yuv_video_buf* buf = NULL;

    //pthread_mutex_init(&flash_mutex, NULL);

    flash_imgCotext = NULL;

	m_nVideoType = 0;
	m_bReloadConfig = false;

#ifdef REBOOT_DETECT
	m_nRebootFlag = 0;
#endif
	m_uTimer = 0;
    return;
}
//析构
CSkpChannelEntity::~CSkpChannelEntity()
{
    //printf("===$$$$$$$4CSkpChannelEntity\n");
    //释放内存
    if(m_chImageData)
    {
        delete[] m_chImageData;
        m_chImageData = NULL;
    }

    if(m_smImageData)
    {
        delete [] m_smImageData;
        m_smImageData = NULL;
    }

    if(m_captureImageData)
    {
        delete [] m_captureImageData;
        m_captureImageData = NULL;
    }

    if(m_JpgImageData)
    {
        delete[] m_JpgImageData;
        m_JpgImageData = NULL;
    }

    if(m_JpgImageData2)
    {
        delete[] m_JpgImageData2;
        m_JpgImageData2 = NULL;
    }

    if(m_pCameraFactory)
    {
        delete m_pCameraFactory;
        m_pCameraFactory = NULL;
    }

    if(m_imgCotext)
    {
        cvReleaseImageHeader(&m_imgCotext);
        m_imgCotext = NULL;
    }

    //pthread_mutex_destroy(&flash_mutex);

    return;
}


//采集线程
void* ThreadFrame(void* pArg)
{
    printf("==ThreadFrame==create...=\n");

    //取类指针
    CSkpChannelEntity* pChannelEntity = (CSkpChannelEntity*)pArg;
    if(pChannelEntity == NULL) return pArg;
    //处理帧
    pChannelEntity->DealFrame();
    pthread_exit((void *)0);
    return pArg;
}

//相机控制线程
void* ThreadCameraControl(void* pArg)
{
	//取类指针
	CSkpChannelEntity* pChannelEntity = (CSkpChannelEntity*)pArg;

	if(pChannelEntity == NULL) return pArg;

	pChannelEntity->CameraControl();
    pthread_exit((void *)0);
	return pArg;
}


//分配内存
bool CSkpChannelEntity::AlignMemory()
{

        if(m_chImageData)
        {
            delete [] m_chImageData;
            m_chImageData = NULL;
        }

        int lineStep = m_uWidth * 3 + IJL_DIB_PAD_BYTES(m_uWidth,3);
        //m_chImageData = new BYTE[lineStep * m_uHeight];
        m_chImageData = new BYTE[lineStep * m_uHeight ];
        //原始图像
        if(m_chImageData == NULL)
        {
            LogError("分配图像转换内存空间失败，打开设备失败!\r\n");
            return false;
        }

        if(m_JpgImageData)
        {
            delete [] m_JpgImageData;
            m_JpgImageData = NULL;
        }

        m_JpgImageData = new BYTE[m_sDetectHeader.uWidth * m_sDetectHeader.uHeight];

        if(m_JpgImageData2)
        {
            delete[] m_JpgImageData2;
            m_JpgImageData2 = NULL;
        }
		//LogNormal("jpgSize2:%d ", m_sDetectHeader.uWidth * m_sDetectHeader.uHeight * 16);

		m_JpgImageData2 = new BYTE[3000000];
        ////////////////////////
        if(m_smImageData)
        {
            delete [] m_smImageData;
            m_smImageData = NULL;
        }
        lineStep = m_sDetectHeader.uWidth * 3 + IJL_DIB_PAD_BYTES(m_sDetectHeader.uWidth,3);
        m_smImageData = new BYTE[lineStep * m_sDetectHeader.uHeight ];

        //发往客户端的全景图像（原始图像的1/4）
        if(m_smImageData == NULL)
        {
            LogError("分配图像转换内存空间失败，打开设备失败!\r\n");
            return false;
        }
      //////////////////////////
        if(m_uWidth > 1000)//如果是高清图像
        {
            if(m_captureImageData)
            {
                delete [] m_captureImageData;
                m_captureImageData = NULL;
            }
            m_captureImageData = new BYTE[g_nVideoWidth * g_nVideoHeight* 3];
            if(m_captureImageData == NULL)
            {
                LogError("分配图像转换内存空间失败，打开设备失败!\r\n");
                return false;
            }
        }
        ////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////

        //区域图像（用于计算地面亮度）
        if(m_imgCotext)
        {
            cvReleaseImageHeader(&m_imgCotext);
            m_imgCotext = NULL;
        }
        m_imgCotext = cvCreateImageHeader(cvSize(m_uWidth, m_uHeight), 8, 3);

        //用于视频震动检测
        if(g_nCheckShake == 1)
        {
            m_nMiss = 0;
            m_tmLastCheck = GetTimeStamp();
            m_nStabPolyNum = 0;
            m_nStabPolySize = NULL;
            m_fStabPolyPts = NULL;
            if(m_imgCurr!= NULL)
            {
                cvReleaseImage(&m_imgCurr);
                m_imgCurr = NULL;
            }
            m_imgCurr = cvCreateImage(cvSize(m_sDetectHeader.uWidth, m_sDetectHeader.uHeight), 8, 3);
        }

        return true;
}

//打开设备
bool CSkpChannelEntity::Open(int nNo)
{

#ifdef _DEBUG
    printf("====before======SkpChannelEntity::Open===============\n");
#endif
    m_uWidth = 0;
    m_uHeight = 0;

    m_Video_Format = GetChannelFormat();

    m_bOpen = false;

//LogTrace(NULL, "====before======checkID===============\n");



#ifdef LINUX
#ifndef NOCHECKID
	LogNormal("before checkID");
	//checkID();
	LogNormal("after checkID");
#endif
#endif

//LogTrace(NULL, "====after======checkID===============\n");
	if(g_nDetectMode == 2)//dsp方案与视频源类型无关
	{
		MJpegOpen();

		if(m_pAbstractCamera != NULL)
				m_bOpen = true;
		else
				m_bOpen = false;
	}
	else
	{
		//根据视频源格式打开对应模块
		switch(m_Video_Format)
		{
		case VEDIO_PAL:
			m_bOpen = PalOpen(nNo);
			break;
		case VEDIO_AVI:
			m_bOpen = AviOpen();
			break;
		case VEDIO_H264:
			m_bOpen = H264Open();
			break;
		case VEDIO_MJPEG:
			{
				//m_bOpen = MJpegOpen();

				MJpegOpen();

				if(m_pAbstractCamera != NULL)
					m_bOpen = true;
				else
					m_bOpen = false;

				LogNormal("=CSkpChannelEntity::Open==VEDIO_MJPEG====m_bOpen=%d===\n", m_bOpen);
				break;
			}
		case VEDIO_YUV:
		case VIDEO_BAYER:
			//m_bOpen = YuvOpen();
			{
				YuvOpen();//防止相机后上电检测器先启动时无法自动重新连接的问题

				if(m_pAbstractCamera != NULL)
				m_bOpen = true;
				else
				m_bOpen = false;
			}
			break;
		case VEDIO_YUV_FILE:
			m_bOpen = YuvFileOpen();
			printf("m_bOpen\n");
			break;
		case VEDIO_PICLIB://图库识别
			//m_bOpen = true;
			m_bOpen = PicLibOpen();
			//return m_bOpen;
			break;
		case VEDIO_H264_FILE: //H264文件流
			m_bOpen = H264FileOpen();
			break;
		default:
			LogError("未知视频源格式，无法打开设备!\r\n");
			m_bOpen = false;
			break;
		}
	}


    if(m_bOpen)
    {
      //  LogError(NULL, "ChannelID=%d open ok!!!!!\n",GetChannelID());
        g_skpDB.UpdateChannelStatus(m_sChannel.uId,NORMAL_STATUS);
        SetStatus(NORMAL_STATUS);
		g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_OK);

        //获取区域亮度统计开始时间
        m_detect_time = GetTimeStamp();

        if(m_uWidth > 0 && m_uHeight > 0)
        {

            if(!AlignMemory())
            {

                m_bOpen = false;
                return false;
            }
        }

        //打开vis控制串口
        if((g_nKeyBoardID >0 ) && g_ytControlSetting.nNeedControl == 1)
        {
            //获取预置位信息
            int nPreSet = g_skpDB.GetPreSet(m_sChannel.uId);
            if(nPreSet > 0)
            {
                //选择预置位
                CAMERA_CONFIG cfg;
                cfg.nIndex = GOTO_PRESET;
                cfg.nAddress = m_sChannel.nCameraId;
                cfg.fValue = nPreSet;
                m_sChannel.nPreSet = nPreSet;

                if(m_sChannel.nCameraType == SANYO_CAMERA)
                {
                    if(m_pAbstractCamera!= NULL)
                    m_pAbstractCamera->ManualControl(cfg);
                }
                else
                {
                    if(g_ytControlSetting.nProtocalType == 0)//虚拟键盘码协议
                    g_VisKeyBoardControl.SendKeyData(cfg, m_sChannel.nMonitorId, 1);
                    else //pelco协议
                    g_VisSerialCommunication.SendData(cfg);
                }
            }
        }
    }
    else
    {
        //无法打开设备，异常
        g_skpDB.UpdateChannelStatus(m_sChannel.uId,UNNORMAL_STATUS);
        SetStatus(UNNORMAL_STATUS);
		g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_NO_VIDEO);
    }
#ifdef _DEBUG
    printf("=======after===SkpChannelEntity::Open===============\n");
#endif

    //发送关闭消息
    if( (g_nServerType == 1)&&(g_nHasCenterServer == 1))
    {
        CHANNEL_INFO_RECORD chan_info;
        chan_info.uChannelID = m_sChannel.uId;
        chan_info.uCameraID = m_sChannel.nCameraId;
        chan_info.uWorkStatus = 1;
        if(m_sChannel.nCameraType == VEDIO_H264 ||
        m_sChannel.nCameraType == VEDIO_MJPEG)
        {
            chan_info.uRealTime = 1;
        }
        else if(m_sChannel.nCameraType == VEDIO_H264_FILE)
        {
            chan_info.uRealTime = 0;
        }

        g_AMSCommunication.mvSendChannelInfo(chan_info);
    }

    return m_bOpen;
}

//模拟信号源设备打开
bool CSkpChannelEntity::PalOpen(int nNo)
{
    printf("======CSkpChannelEntity::PalOpen()==nNo=%d=======m_sChannel.nCameraId=%d==================\n", m_sChannel.nVideoIndex, m_sChannel.nCameraId);

    bool bOpen = false;
    if(m_sChannel.nCameraType == ANALOG_FIELD ||
       m_sChannel.nCameraType == ANALOG_FIELD_DH ||
       m_sChannel.nCameraType == ANALOG_FRAME ||
       m_sChannel.nCameraType == ANALOG_FRAME_DH)
    {
        nNo = m_sChannel.nVideoIndex;

        printf("nNo=%d\r\n",nNo);

        m_uWidth = 768;
        m_uHeight = 576;

		if(m_sChannel.nCameraType == ANALOG_FRAME_DH)
		{
			m_uWidth = 640;
			m_uHeight = 480;
		}

        //宽度
        m_sDetectHeader.uWidth = m_uWidth/2;
        //高度
        m_sDetectHeader.uHeight = m_uHeight/2;

        if(m_sChannel.nCameraType == ANALOG_FIELD ||
           m_sChannel.nCameraType == ANALOG_FIELD_DH)
        {
            m_uHeight /= 2;
        }

        //打开V4L2设备
        bOpen =  m_v4lDriver.OpenVideo(nNo,m_uWidth,m_uHeight,m_sChannel.nCameraType);
    }
    return bOpen;
}

//打开avi文件
bool CSkpChannelEntity::AviOpen()
{
    //打开Avi
    String strFileName = g_skpDB.GetSrcFileByID(GetChannelID());

    bool bOpen = false;
    //判断文件是否存在
    if(access(strFileName.c_str(),F_OK) == 0)
    {
        bOpen = m_v4lDriver.OpenAvi(strFileName.c_str(),m_uWidth,m_uHeight);
        //宽度
        m_sDetectHeader.uWidth = m_uWidth/2;
        //高度
        m_sDetectHeader.uHeight = m_uHeight/2;
        printf("m_uWidth=%d,m_uHeight=%d\n",m_uWidth,m_uHeight);
    }
    return bOpen;
}

//H264设备打开
bool CSkpChannelEntity::H264Open()
{
    LogTrace(NULL, "====m_sChannel.nCameraType=%d\n",m_sChannel.nCameraType);

    if(m_sChannel.nCameraType == HD_IP_CAMERA||
       m_sChannel.nCameraType == SANYO_CAMERA||
	   m_sChannel.nCameraType == H3C_CAMERA ||
	   m_sChannel.nCameraType == HK_CAMERA ||
	   m_sChannel.nCameraType == CF_CAMERA ||
	   m_sChannel.nCameraType == CF_CAMERA_2 ||
	   m_sChannel.nCameraType == BOCOM_301_200 ||
	   m_sChannel.nCameraType == ZION_CAMERA)
    {
        printf("====CSkpChannelEntity::H264Open()=111===\n");
        m_pCameraFactory = new Factory();
        printf("====CSkpChannelEntity::H264Open()=222===\n");
        m_pAbstractCamera = m_pCameraFactory->Create(m_sChannel.nCameraType,m_sChannel.eVideoFmt);
        printf("====CSkpChannelEntity::H264Open()=333===\n");

        if(m_pAbstractCamera)
        {
			//设置相机IP
			std::string strCameraHost(m_sChannel.chCameraHost);
			m_pAbstractCamera->SetCameraIpHost(strCameraHost,m_sChannel.uMonitorPort);

			m_pAbstractCamera->SetDeviceID(m_sChannel.nCameraId);
            printf("====CSkpChannelEntity::H264Open()=444===\n");
            m_uWidth = m_pAbstractCamera->GetEffectImageWidth();
            m_uHeight = m_pAbstractCamera->GetEffectImageHeight();
			#ifndef NOEVENT
			m_skpRoadDetect.SetCameraHostAndWidthHeight(m_uWidth,m_uHeight,m_sChannel.chCameraHost);
			#endif
			//设置通道ID
			int nChannelId = m_sChannel.uId;
			m_pAbstractCamera->SetChannelID(nChannelId);

			if(m_sChannel.nCameraType == HK_CAMERA || m_sChannel.nCameraType == ZION_CAMERA)
			{
				m_sDetectHeader.uWidth = m_uWidth/4;
				m_sDetectHeader.uHeight = m_uHeight/4;
			}
			else
			{
				m_sDetectHeader.uWidth = m_pAbstractCamera->GetSmallImageWidth();
				m_sDetectHeader.uHeight = m_pAbstractCamera->GetSmallImageHeight();
			}

			//设置文件起始时间
			std::string  strFileName = g_skpDB.GetSrcFileByID(GetChannelID());
			VOD_FILE_INFO vod_info;
			memset(vod_info.chFilePath,0,256);
			memcpy(vod_info.chFilePath,strFileName.c_str(),strFileName.size());
			vod_info.uBeginTime = m_sChannel.uVideoBeginTime;
			vod_info.uEndTime = m_sChannel.uVideoEndTime;
			m_pAbstractCamera->SetVodInfo(vod_info);

			m_pAbstractCamera->SetVideoType(0);

            //打开相机
            bool bOpenCamera = m_pAbstractCamera->Open();

            return bOpenCamera;
        }
        else
        {
            if(m_pCameraFactory)
            {
                delete m_pCameraFactory;
                m_pCameraFactory = NULL;
            }
        }
        return false;
    }
    else if(m_sChannel.nCameraType == MONITOR_CAMERA)
    {
        int nVideoType = 0;//实时视频
        std::string  strFileName = g_skpDB.GetSrcFileByID(GetChannelID());
        LogTrace(NULL, "H264Open strFileName=%s,strFileName.size()=%d\n",strFileName.c_str(),strFileName.size());
        if(strFileName.size() > 0)
        {
			g_skpDB.UTF8ToGBK(strFileName);
            if(access(strFileName.c_str(),F_OK) == 0)
            {
                nVideoType = 2;//本地历史视频
            }
        }

        VOD_FILE_INFO vod_info;
        memset(vod_info.chFilePath,0,256);
        memcpy(vod_info.chFilePath,strFileName.c_str(),strFileName.size());
		vod_info.uBeginTime = m_sChannel.uVideoBeginTime;
        vod_info.uEndTime = m_sChannel.uVideoEndTime;

        bool bOpen = m_MonitorH264.OpenVideo(m_sChannel.nCameraId,vod_info,nVideoType);

        if(bOpen)
        {
            yuv_video_buf* pVideoInfo = m_MonitorH264.GetVideoInfo();
            m_uWidth = pVideoInfo->width;
            m_uHeight = pVideoInfo->height;

            if(m_uWidth > 1000)
            {
                //宽度
                m_sDetectHeader.uWidth = m_uWidth/4;
            }
            else if(m_uWidth > 500)
            {
                //宽度
                m_sDetectHeader.uWidth = m_uWidth/2;
            }
            else
            {
                //宽度
                m_sDetectHeader.uWidth = m_uWidth;
            }


			if(m_uHeight > 1000)
            {
                //高度
                m_sDetectHeader.uHeight = m_uHeight/4;
            }
            else if(m_uHeight > 500)
            {
                //高度
				if(m_uWidth >= 1000)
				{
					 m_sDetectHeader.uHeight = m_uHeight/4;
				}
				else
				{
					 m_sDetectHeader.uHeight = m_uHeight/2;
				}
            }
            else
            {
                //高度
                m_sDetectHeader.uHeight = m_uHeight;
            }
            LogTrace(NULL, "m_uWidth=%d,m_uHeight=%d\n",m_uWidth,m_uHeight);

        }
        return bOpen;
    }
	else if( m_sChannel.nCameraType == DH_CAMERA)
	{
		m_pCameraFactory = new Factory();
		m_pAbstractCamera = m_pCameraFactory->Create(m_sChannel.nCameraType,m_sChannel.eVideoFmt);
		if(m_pAbstractCamera)
		{
			//设置相机IP
			std::string strCameraHost(m_sChannel.chCameraHost);
			m_pAbstractCamera->SetCameraIpHost(strCameraHost,m_sChannel.uMonitorPort);
			m_pAbstractCamera->SetDeviceID(m_sChannel.nCameraId);
			m_uWidth = m_pAbstractCamera->GetEffectImageWidth();
			m_uHeight = m_pAbstractCamera->GetEffectImageHeight();
			
			 #ifndef NOEVENT
			m_skpRoadDetect.SetCameraHostAndWidthHeight(m_uWidth,m_uHeight,m_sChannel.chCameraHost);
			#endif
			//设置通道ID
			int nChannelId = m_sChannel.uId;
			m_pAbstractCamera->SetChannelID(nChannelId);

			m_sDetectHeader.uWidth = m_pAbstractCamera->GetSmallImageWidth();
			m_sDetectHeader.uHeight = m_pAbstractCamera->GetSmallImageHeight();

			//设置文件起始时间
			std::string  strFileName = g_skpDB.GetSrcFileByID(GetChannelID());
			VOD_FILE_INFO vod_info;
			memset(vod_info.chFilePath,0,256);
			memcpy(vod_info.chFilePath,strFileName.c_str(),strFileName.size());
			vod_info.uBeginTime = m_sChannel.uVideoBeginTime;
			vod_info.uEndTime = m_sChannel.uVideoEndTime;
			m_pAbstractCamera->SetVodInfo(vod_info);
			m_pAbstractCamera->SetVideoType(0);

			//打开相机
			return m_pAbstractCamera->Open();
		}
		else
		{
			if(m_pCameraFactory)
			{
				delete m_pCameraFactory;
				m_pCameraFactory = NULL;
			}
		}
		return false;
	}
    return false;
}

//H264文件流打开
bool CSkpChannelEntity::H264FileOpen()
{
    int nVideoType = 2;//本地历史视频
    std::string  strFileName = g_skpDB.GetSrcFileByID(GetChannelID());
    LogTrace(NULL, "H264Open strFileName=%s\n",strFileName.c_str());
    if(strFileName.size() > 0)
    {
        if(access(strFileName.c_str(),F_OK) != 0)
        {
            nVideoType = 1;//远程历史视频
        }
    }
    else
    {
        nVideoType = 1;//远程历史视频
    }
    LogTrace(NULL, "H264Open strFileName=%d,nVideoType=%d\n",strFileName.size(),nVideoType);

    VOD_FILE_INFO vod_info;
    //历史视频文件名称
    {
        vod_info.uBeginTime = m_sChannel.uVideoBeginTime;
        vod_info.uEndTime = m_sChannel.uVideoEndTime;
        memcpy(vod_info.chFilePath,strFileName.c_str(),strFileName.size());
    }

	bool bOpen = false;

	if(m_sChannel.nCameraType == MONITOR_CAMERA)
	{
		bOpen = m_MonitorH264.OpenVideo(m_sChannel.nCameraId,vod_info,nVideoType);
		if(bOpen)
		{
			yuv_video_buf* pVideoInfo = m_MonitorH264.GetVideoInfo();
			m_uWidth = pVideoInfo->width;
			m_uHeight = pVideoInfo->height;

			//宽度
			m_sDetectHeader.uWidth = m_uWidth/4;
			//高度
			m_sDetectHeader.uHeight = m_uHeight/4;

			LogTrace(NULL, "m_uWidth=%d,m_uHeight=%d\n",m_uWidth,m_uHeight);

		}
	}
	else
	{
			m_pCameraFactory = new Factory();
			m_pAbstractCamera = m_pCameraFactory->Create(m_sChannel.nCameraType,m_sChannel.eVideoFmt);
			 if(m_pAbstractCamera)
			{
				//设置相机IP
				std::string strCameraHost(m_sChannel.chCameraHost);
				m_pAbstractCamera->SetCameraIpHost(strCameraHost,m_sChannel.uMonitorPort);

				m_pAbstractCamera->SetDeviceID(m_sChannel.nCameraId);
				printf("====CSkpChannelEntity::H264Open()=444===\n");
				m_uWidth = m_pAbstractCamera->GetEffectImageWidth();
				m_uHeight = m_pAbstractCamera->GetEffectImageHeight();

				//设置通道ID
				int nChannelId = m_sChannel.uId;
				m_pAbstractCamera->SetChannelID(nChannelId);

				if(m_sChannel.nCameraType == HK_CAMERA || m_sChannel.nCameraType == ZION_CAMERA)
				{
					m_sDetectHeader.uWidth = m_uWidth/4;
					m_sDetectHeader.uHeight = m_uHeight/4;
				}
				else
				{
					m_sDetectHeader.uWidth = m_pAbstractCamera->GetSmallImageWidth();
					m_sDetectHeader.uHeight = m_pAbstractCamera->GetSmallImageHeight();
				}

				m_pAbstractCamera->SetVodInfo(vod_info);

				m_pAbstractCamera->SetVideoType(1);

				//打开相机
				bOpen = m_pAbstractCamera->Open();

				return bOpen;
			}
			else
			{
				if(m_pCameraFactory)
				{
					delete m_pCameraFactory;
					m_pCameraFactory = NULL;
				}
			}
	}

    return bOpen;
}

//YUV流打开
bool CSkpChannelEntity::YuvOpen()
{
    m_pCameraFactory = new Factory();
    m_pAbstractCamera = m_pCameraFactory->Create(m_sChannel.nCameraType,m_sChannel.eVideoFmt);


    if(m_pAbstractCamera)
    {

        m_uWidth = m_pAbstractCamera->GetEffectImageWidth();
        m_uHeight = m_pAbstractCamera->GetEffectImageHeight();
        m_sDetectHeader.uWidth = m_pAbstractCamera->GetSmallImageWidth();
        m_sDetectHeader.uHeight = m_pAbstractCamera->GetSmallImageHeight();

		//设置相机工作方式
		m_pAbstractCamera->SetCameraMode(m_sChannel.nWorkMode);

		//设置相机ID
		int nCamID = g_skpDB.GetCameraSelfID(m_sChannel.uId);
		if(nCamID < 0)//表示获取不到该字段
			nCamID = 0;
		m_pAbstractCamera->SetCamID(nCamID);

		//设置相机IP
        std::string strCameraHost(m_sChannel.chCameraHost);
        m_pAbstractCamera->SetCameraIpHost(strCameraHost,m_sChannel.uMonitorPort);

        //打开相机
        bool bOpenCamera = m_pAbstractCamera->Open();

        //设置是否采集红灯信号
        bool bDetectRedSignal = ((m_sChannel.uDetectKind&DETECT_VTS)==DETECT_VTS);
        m_pAbstractCamera->SetDetectRedSignal(bDetectRedSignal);
        if(bDetectRedSignal)
        {
            r_SerialComm.BeginRedSignalCapture();
        }

        //设置是否采集线圈信号
        bool bDetectLoopSignal = ((m_sChannel.uDetectKind&DETECT_LOOP)==DETECT_LOOP);
        m_pAbstractCamera->SetDetectLoopSignal(bDetectLoopSignal);

        if(bDetectLoopSignal)
        {
            d_SerialComm.BeginLoopSignalCapture();
        }

        //设置是否采集雷达信号
		LogNormal("--SetDetectRadarSignal--YUV-\n");
        bool bDetectRadarSignal = ((m_sChannel.uDetectKind&DETECT_RADAR)==DETECT_RADAR);
        m_pAbstractCamera->SetDetectRadarSignal(bDetectRadarSignal);
        if(bDetectRadarSignal)
        {
			if(g_RadarComSetting.nComUse == 7 || g_RadarComSetting.nComUse == 13) //S3雷达,慧昌雷达
			{					
				//打开串口							
				LogNormal("Open Dev g_RadarSerial %d,%d,%d,%d,%d", \
					g_RadarComSetting.nComPort, g_RadarComSetting.nBaud, g_RadarComSetting.nDataBits, g_RadarComSetting.nStopBits, g_RadarComSetting.nParity);
				if(g_RadarSerial.IsOpen())
				{
					g_RadarSerial.Close();
				}				
				g_RadarSerial.OpenDev();							
			}

			if(g_RadarComSetting.nComUse == 7)
			{
				g_RadarSerial.SetRadarType(1);
			}
			else if(g_RadarComSetting.nComUse == 13)
			{
				LogNormal("nComUse 13 SetRadarType 11!\n");
				g_RadarSerial.SetRadarType(2);
			}
			
            g_RadarSerial.BeginRadarSignalCapture();
        }

        return bOpenCamera;
    }
    else
    {
        if(m_pCameraFactory)
        {
            delete m_pCameraFactory;
            m_pCameraFactory = NULL;
        }
    }

    return false;
}
//YUV文件流打开
bool CSkpChannelEntity::YuvFileOpen()
{
    String strFileName = g_skpDB.GetSrcFileByID(GetChannelID());

    //	#ifdef DEBUG_CHANNEL
    LogNormal("YuvFileOpen:YUV文件流打开--%s\r\n",strFileName.c_str());
    //	#endif

    //判断文件是否存在
    if(access(strFileName.c_str(),F_OK) == 0)
    {
        //目前只支持场图像读图像
        if(m_sChannel.nCameraType == JAI_CAMERA_LINK_FIELD||
           m_sChannel.nCameraType == JAI_CAMERA_LINK_FRAME||
           m_sChannel.nCameraType == PTG_ZEBRA_200||
           m_sChannel.nCameraType == PTG_ZEBRA_500)
        {
            m_pCameraFactory = new Factory();
            m_pAbstractCamera = m_pCameraFactory->Create(m_sChannel.nCameraType);

            if(m_pAbstractCamera)
            {
				bool bOpenFile = m_pAbstractCamera->OpenFile(strFileName.c_str());

                m_uWidth = m_pAbstractCamera->GetEffectImageWidth();
                m_uHeight = m_pAbstractCamera->GetEffectImageHeight();

                m_sDetectHeader.uWidth = m_pAbstractCamera->GetSmallImageWidth();
                m_sDetectHeader.uHeight = m_pAbstractCamera->GetSmallImageHeight();

/*
				//设置是否采集雷达信号
				LogNormal("--SetDetectRadarSignal--YUV-FILE-\n");

				bool bDetectRadarSignal = ((m_sChannel.uDetectKind&DETECT_RADAR)==DETECT_RADAR);
				m_pAbstractCamera->SetDetectRadarSignal(bDetectRadarSignal);
				if(bDetectRadarSignal)
				{
					if(g_RadarComSetting.nComUse == 7 || g_RadarComSetting.nComUse == 13) //S3雷达,慧昌雷达
					{					
						//打开串口							
						LogNormal("Open Dev F g_RadarSerial %d,%d,%d,%d,%d", \
							g_RadarComSetting.nComPort, g_RadarComSetting.nBaud, g_RadarComSetting.nDataBits, g_RadarComSetting.nStopBits, g_RadarComSetting.nParity);
						if(g_RadarSerial.IsOpen())
						{
							g_RadarSerial.Close();
						}				
						g_RadarSerial.OpenDev();							
					}

					g_RadarSerial.SetRadarType(2);//huichang
					g_RadarSerial.BeginRadarSignalCapture();
				}
*/
                return bOpenFile;
            }
            else
            {
                if(m_pCameraFactory)
                {
                    delete m_pCameraFactory;
                    m_pCameraFactory = NULL;
                }
            }
        }
    }
	else
	{
		LogNormal("文件不存在%s",strFileName.c_str());
	}
    return false;
}

//打开MJpeg流
bool CSkpChannelEntity::MJpegOpen()
{
    //LogNormal("====CSkpChannelEntity::MJpegOpen()====\n");
    printf("====m_sChannel.nCameraType=%d\n",m_sChannel.nCameraType);

	bool bInitCam = false;
	int nCamType = GetCameraType();
	if(DSP_SERVER == nCamType)
	{
		if(m_pAbstractCamera)
		{
			//LogNormal("DD=CSkpChannelEntity::MJpegOpen()==BB!\n");
			m_pAbstractCamera->Close();
			bInitCam = true;
		}
	}
	else
	{
		if(m_pAbstractCamera)
		{
			//LogNormal("DD=CSkpChannelEntity::MJpegOpen()==BB!\n");
			m_pAbstractCamera->Close();
			m_pAbstractCamera = NULL;
		}
	}
	
	if(g_nDetectMode == 2 || m_sChannel.nCameraType == AXIS_CAMERA)
    {
		if(!bInitCam)
		{
			printf("====CSkpChannelEntity::MJpegOpen()=1111===\n");
			m_pCameraFactory = new Factory();
			printf("====CSkpChannelEntity::MJpegOpen()==2222==\n");
			m_pAbstractCamera = m_pCameraFactory->Create(m_sChannel.nCameraType);
			printf("====CSkpChannelEntity::MJpegOpen()==3333==\n");
		}

        if(m_pAbstractCamera)
        {
            //设置相机IP
            std::string strCameraHost(m_sChannel.chCameraHost);
            printf("==m_sChannel.chCameraHost=%s======\n", m_sChannel.chCameraHost);
            m_pAbstractCamera->SetCameraIpHost(strCameraHost,m_sChannel.uMonitorPort);
            printf("====SetCameraIpHost====strCameraHost=%s===\n", (m_pAbstractCamera->GetCameraIP()).c_str());

            //有效区域的宽高
            m_uWidth = 400;//m_pAbstractCamera->GetEffectImageWidth();
            m_uHeight = 300;//m_pAbstractCamera->GetEffectImageHeight();

            printf("####=MJpegOpen()====m_uWidth=%d, m_uHeight=%d===\n", m_uWidth, m_uHeight);

            //实际检验区域的小图的宽高         
			{
				m_sDetectHeader.uWidth = m_uWidth;
				m_sDetectHeader.uHeight = m_uHeight;
			}

            printf("####=MJpegOpen():==m_sDetectHeader.uWidth=%d=m_sDetectHeader.uHeight=%d==\n", \
					m_sDetectHeader.uWidth, m_sDetectHeader.uHeight);

			//设置相机ID
			int nCamID = m_sChannel.nCameraId;
			m_pAbstractCamera->SetCamID(nCamID);
			LogNormal("MJpegOpen=nCamID=%d=\n", nCamID);

			//设置通道ID
			int nChannelId = m_sChannel.uId;
			m_pAbstractCamera->SetChannelID(nChannelId);
			//LogNormal("MJpegOpen=nChannelId=%d=\n", nChannelId);


			if(DSP_SERVER == m_sChannel.nCameraType)
			{
				//设置检测类型
				m_pAbstractCamera->SetDetectKind(m_sChannel.uDetectKind);
			}			

            //打开相机
            bool bOpenCamera = m_pAbstractCamera->Open();

			if(bOpenCamera && (DSP_SERVER == nCamType))
			{
				InitRecordCapture();
				
				if(g_nDetectMode == 2)
				{
					InitTempCapture();
				}
			}

            return bOpenCamera;
        }
        else
        {
            printf("=m_pAbstractCamera=Create false m_sChannel.nCameraType=%d!!\n", \
				m_sChannel.nCameraType);
            if(m_pCameraFactory)
            {
                delete m_pCameraFactory;
                m_pCameraFactory = NULL;
            }
        }
        return false;
    }

    return false;
}

//关闭
bool CSkpChannelEntity::Close()
{
    bool bRet = false;

    if (m_imgCurr)
    {
        cvReleaseImage(&m_imgCurr);
        m_imgCurr = NULL;
    }
	
	if(g_nDetectMode == 2)//dsp方案与视频源类型无关
	{
		bRet = MJpegClose();
	}
	else
	{
		//根据视频源格式关闭对应模块
		switch(m_Video_Format)
		{
		case VEDIO_PAL:
		case VEDIO_AVI:
			bRet = PalClose();
			break;
		case VEDIO_H264:
		case VEDIO_H264_FILE:
			bRet = H264Close();
			break;
		case VEDIO_MJPEG:
			bRet = MJpegClose();
			break;
		case VEDIO_YUV:
		case VEDIO_YUV_FILE:
		case VIDEO_BAYER:
			bRet = YuvClose();
			break;
		case VEDIO_PICLIB:
			//bRet = true;
			bRet = PicLibClose();
			break;
		default:
			LogError("未知视频源格式，无法关闭设备!\r\n");
			return false;
		}
	}

    if(bRet)
        m_bOpen = false;

    //发送关闭消息
    if( (g_nServerType == 1)&&(g_nHasCenterServer == 1))
    {
        CHANNEL_INFO_RECORD chan_info;
        chan_info.uChannelID = m_sChannel.uId;
        chan_info.uCameraID = m_sChannel.nCameraId;
        chan_info.uWorkStatus = 0;
        if(m_sChannel.nCameraType == VEDIO_H264 ||
        m_sChannel.nCameraType == VEDIO_MJPEG)
        chan_info.uRealTime = 1;
        else if(m_sChannel.nCameraType == VEDIO_H264_FILE)
        chan_info.uRealTime = 0;
        g_AMSCommunication.mvSendChannelInfo(chan_info);
    }

    return bRet;
}

//模拟信号源设备关闭
bool CSkpChannelEntity::PalClose()
{
    //打开V4L2设备
    return m_v4lDriver.CloseVideo();
}

//H264设备关闭
bool CSkpChannelEntity::H264Close()
{
    if(m_pAbstractCamera)
    {
        m_pAbstractCamera->Close();
        m_pAbstractCamera = NULL;
    }

    if(m_pCameraFactory)
    {
        delete m_pCameraFactory;
        m_pCameraFactory = NULL;

        return true;
    }

    return m_MonitorH264.CloseVideo();
}

//YUV流关闭
bool CSkpChannelEntity::YuvClose()
{
    if(m_pAbstractCamera)
    {
        m_pAbstractCamera->Close();
        m_pAbstractCamera = NULL;
    }

    if(m_pCameraFactory)
    {
        delete m_pCameraFactory;
        m_pCameraFactory = NULL;
    }
    printf("YuvClose\n");
    return true;
}

//close zebra
bool CSkpChannelEntity::MJpegClose()
{
	//return m_MJpegDriver.MJpegClose();

	if(GetCameraType() == DSP_SERVER)
	{
		LogNormal("==MJpegClose()==DSP_SERVER==\n");

		if(m_pAbstractCamera)
		{
			m_pAbstractCamera->Close();
		}
	}
	else
	{
		if(m_pAbstractCamera)
		{
			m_pAbstractCamera->Close();
			m_pAbstractCamera = NULL;
		}

		if(m_pCameraFactory)
		{
			delete m_pCameraFactory;
			m_pCameraFactory = NULL;

			return true;
		}
	}
	return true;
}

bool CSkpChannelEntity::PicLibClose()
{
	if(m_pAbstractCamera)
	{
		m_pAbstractCamera->Close();
		m_pAbstractCamera = NULL;
	}

	if(m_pCameraFactory)
	{
		delete m_pCameraFactory;
		m_pCameraFactory = NULL;
	}
	printf("PicLibClose\n");
	return true;
}


//开始检测
bool CSkpChannelEntity::BeginDetect()
{
    if(m_Video_Format == VEDIO_PAL)
    {
        SRIP_CHANNEL_ATTR sAttr;
        g_skpDB.GetAdjustPara(sAttr);

        //设置视频参数
        SetChannelParams(sAttr);
    }

    #ifndef NOEVENT
    //设置事件录象长度
    m_skpRoadDetect.SetCaptureTime(m_sChannel.uEventCaptureTime);
    //是否事件录象
    if(m_sChannel.bEventCapture == 1 || m_sChannel.bEventCapture == 3)
    m_skpRoadDetect.SetEventCapture(true);
    else
    m_skpRoadDetect.SetEventCapture(false);
    #endif

	m_bReloadConfig = true;

    #ifndef NOOBJECTDETECT
    //设置事件录象长度
    m_ObjectBehaviorDetect.SetCaptureTime(m_sChannel.uEventCaptureTime);
    //是否事件录象
    if(m_sChannel.bEventCapture == 1 || m_sChannel.bEventCapture == 3)
    m_ObjectBehaviorDetect.SetEventCapture(true);
    else
    m_ObjectBehaviorDetect.SetEventCapture(false);
    #endif

    #ifndef NOPLATE
    m_RoadCarnumDetect.SetCaptureTime(m_sChannel.uEventCaptureTime);

	#ifndef NOEVENT
	m_RoadCarnumDetect.SetEventDetect(&m_skpRoadDetect);
	#endif

    if(m_sChannel.bEventCapture == 2 || m_sChannel.bEventCapture == 3)
	{
		m_RoadCarnumDetect.SetEventCapture(true);

		if(m_pAbstractCamera)
		{
			m_pAbstractCamera->SetHaveSkpRecorder(true);
/*
			CSkpRoadRecorder *pSkpRecorder = m_RoadCarnumDetect.GetSkpRecorder();
			m_pAbstractCamera->SetSkpRecorder(*pSkpRecorder);
*/
		}
	}
    else
	{
		m_RoadCarnumDetect.SetEventCapture(false);
		if(m_pAbstractCamera)
		{
			m_pAbstractCamera->SetHaveSkpRecorder(false);
		}
	}

    if(m_pAbstractCamera)
    {
        CAMERA_CONFIG cfg = m_pAbstractCamera->GetCameraConfig();
        m_RoadCarnumDetect.SetFrequency(cfg.nFrequency);
    }
    #endif

	LogTrace("rc_feature.log", "m_sChannel.uDetectKind&DETECT_FACE,m_uWidth=%d,m_uHeight=%d\n",m_uWidth,m_uHeight);

    if(m_uWidth > 0 && m_uHeight>0)
    {

        {
            //事件检测
            ////////////////////////////////////////////////////////
            #ifndef NOEVENT
            if((m_sChannel.uDetectKind&DETECT_FLUX)==DETECT_FLUX)
            {
                //初始化检测类
                int nWidth = m_sDetectHeader.uWidth;
                int nHeight = m_sDetectHeader.uHeight;

                printf("==CSkpChannelEntity::BeginDetect()==m_uWidth=%d,m_uHeight=%d,nWidth=%d,nHeight=%d\n",m_uWidth,m_uHeight,nWidth,nHeight);
                if(!m_skpRoadDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
                //if(!m_skpRoadDetect.Init(GetChannelID(),m_uWidth*4,m_uHeight*4,nWidth,nHeight))
                {
                    LogError("事件检测模块初始化失败，无法检测！\r\n");
                    return false;
                }
            }
            #endif

            #ifndef NOOBJECTDETECT
            if((m_sChannel.uDetectKind&DETECT_BEHAVIOR)==DETECT_BEHAVIOR)
            {
                //初始化检测类
                int nWidth = m_sDetectHeader.uWidth;
                int nHeight = m_sDetectHeader.uHeight;

                if(!m_ObjectBehaviorDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
                {
                    LogError("行为分析检测模块初始化失败，无法检测！\r\n");
                    return false;
                }
            }
            #endif

            #ifndef NOFEATURESEARCH
            if((m_sChannel.uDetectKind&DETECT_CHARACTER)==DETECT_CHARACTER)
            {
                //初始化检测类
                int nWidth = m_sDetectHeader.uWidth;
                int nHeight = m_sDetectHeader.uHeight;
                LogTrace("rc_feature.log", "begin to init FeatureSearchDetect....");
                if(!m_FeatureSearchDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
                {
                    LogError("特征提取模块初始化失败，无法检测！\r\n");
                    return false;
                }
            }
            #endif

			printf("11m_sChannel.uDetectKind&DETECT_FACE\n");
			#ifndef NOFACEDETECT
			 printf("22m_sChannel.uDetectKind&DETECT_FACE\n");
            if((m_sChannel.uDetectKind&DETECT_FACE)==DETECT_FACE)
            {
                //初始化检测类
                int nWidth = m_sDetectHeader.uWidth;
                int nHeight = m_sDetectHeader.uHeight;

                if(!m_FaceDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
                {
                    LogError("特征提取模块初始化失败，无法检测！\r\n");
                    return false;
                }
            }
            #endif


            //printf("11 DETECT_CARNUM=%d,m_sChannel.uDetectKind=%d,m_sChannel.uDetectKind&DETECT_CARNUM=%d\n",DETECT_CARNUM,m_sChannel.uDetectKind,m_sChannel.uDetectKind&DETECT_CARNUM);
            #ifndef NOPLATE
            //printf("22 DETECT_CARNUM=%d,m_sChannel.uDetectKind=%d,m_sChannel.uDetectKind&DETECT_CARNUM=%d\n",DETECT_CARNUM,m_sChannel.uDetectKind,m_sChannel.uDetectKind&DETECT_CARNUM);
			            
			if((m_sChannel.uDetectKind&DETECT_CARNUM)|| (g_nDetectMode == 2))
            {
                printf("m_RoadCarnumDetect.Init====\n");
                //车牌检测
				if(!m_RoadCarnumDetect.Init(GetChannelID(),m_uWidth,m_uHeight,m_sDetectHeader.uWidth ,m_sDetectHeader.uHeight))
				{
					LogError("车牌检测模块启动失败，无法检测！\r\n");
					return false;
				}
            }
            #endif

            #ifdef DSPDATAPROCESS
			/* if((g_nGongJiaoMode == 1) && (m_sChannel.nCameraType == DSP_ROSEEK_500_335))
			{
				if((m_sChannel.uDetectKind&DETECT_CARNUM)==DETECT_CARNUM)
				{
					LogNormal("in RoadChannel.cpp line 1100 车牌检测模块\n");


					//初始化检测类
                    int nWidth = m_sDetectHeader.uWidth;
                    int nHeight = m_sDetectHeader.uHeight;

					if(nWidth < 1000)
                    {
                        nWidth = 4 * m_sDetectHeader.uWidth;
                        nHeight = 4 * m_sDetectHeader.uHeight;
                    }
					if(!m_DspDataProcess.Init(GetChannelID(),nWidth,nHeight,nWidth ,nHeight))
					{
						LogError("车牌检测模块启动失败，无法检测！\r\n");
						return false;
					}
					else
					{
						LogNormal("in RoadChannel.cpp line 1065 车牌检测模块\n");
					}
				}
			}*/
            #endif


			if(g_nCheckShake == 1)
			{
				InitShakeDetect();
			}

		}
				
        InitRecordCapture();
		
		if(g_nDetectMode == 2)
		{
			InitTempCapture();
		}
    }
	
	if(g_nDetectMode == 2)//dsp方案与视频源类型无关
	{
		return MJpegBegin();
	}
	else
	{
		//根据视频源格式启动对应模块
		switch(m_Video_Format)
		{
		case VEDIO_PAL:
		case VEDIO_AVI:
			return PalBegin();
		case VEDIO_H264:
		case VEDIO_H264_FILE:
			return H264Begin();
		case VEDIO_MJPEG:
			{
				LogNormal("=before=MJpegBegin===\n");
				return MJpegBegin();
			}
		case VEDIO_YUV:
		case VEDIO_YUV_FILE:
		case VIDEO_BAYER:
			return YuvBegin();
		case VEDIO_PICLIB:
			return PicLibBegin();
		default:
			LogError("未知视频源格式，通道无法采集并检测图片!\r\n");
			return false;
		}
	}
    return true;
}

//模拟信号源设备开始采集
bool CSkpChannelEntity::PalBegin()
{
    //开始采集
    if(m_v4lDriver.StartCapture())
    {
        /********************************************************************/
//说明：未把线程放在对的模块中处理，省去中间数据传递消耗
//		同时能够共用通道同一份数据.
//		启动线程，将this传递到线程，线程直接控制设备的数据流.
        /********************************************************************/
        //启动采集线程
        BeginFrameThread();
        return true;
    }
    return false;
}
//H264设备开始采集
bool CSkpChannelEntity::H264Begin()
{
    if(m_pAbstractCamera)
    {
        //启动采集线程
        BeginFrameThread();
        return true;
    }
    else
    {
        //启动采集线程
		if(m_MonitorH264.StartCapture())
		{
			BeginFrameThread();
			return true;
		}
    }
    return false;
}
//YUV流开始采集
bool CSkpChannelEntity::YuvBegin()
{
    //启动采集线程
    BeginFrameThread();
    return true;
}

//MJpeg流开始采集
bool CSkpChannelEntity::MJpegBegin()
{
    LogNormal("==CSkpChannelEntity::=MJpegBegin=111=\n");
    //启动采集线程
    //BeginFrameThread();
    if(m_pAbstractCamera)
    {
        //启动采集线程
        BeginFrameThread();
        return true;
    }
    else
    {
        LogError("启动MJpeg流采集线程失败!\n");
        return false;
    }
    return true;
}

//图库识别
bool CSkpChannelEntity::PicLibBegin()
{
    //启动图库识别
    BeginFrameThread();
    return true;
}

//停止检测
bool CSkpChannelEntity::EndDetect()
{
	LogNormal("=CSkpChannelEntity=EndDetect==\n");
    if(m_nThreadId==0)
    {
        return false;
    }

    bool bRet = false;
	if(g_nDetectMode == 2)//dsp方案与视频源类型无关
	{
		bRet = MJpegEnd();
	}
	else
	{
    //停止对应模块
    switch(m_Video_Format)
    {
		case VEDIO_PAL:
		case VEDIO_AVI:
			bRet = PalEnd();
			break;
		case VEDIO_H264:
		case VEDIO_H264_FILE:
			bRet = H264End();
			break;
		case VEDIO_MJPEG:
			bRet = MJpegEnd();
			break;
		case VEDIO_YUV:
		case VEDIO_YUV_FILE:
		case VIDEO_BAYER:
			bRet = YuvEnd();
			break;
		case VEDIO_PICLIB:
			bRet = PicLibEnd();
			break;
		default:
			LogError("未知视频源格式，停止通道采集检测图片失败!\r\n");
			bRet = false;
			break;
		}
	}

    //停止录像
    if(m_JpgToAvi.IsEncoding())
    {
        m_uEndTime = GetTimeStamp();
        //添加车牌记录和图片
        if(g_nAviHeaderEx == 1)
        m_JpgToAvi.AddPlatePics(m_sChannel.uId,m_uBeginTime,m_uEndTime);
        //记录录像结束时间
        m_JpgToAvi.CloseFile();
        g_skpDB.SaveVideo(m_sChannel.uId,m_uBeginTime,m_uEndTime,m_JpgToAvi.GetEncodingFileName(),0);
    }

    if(g_nCheckShake == 1)
    {
        UnInitShakeDetect();
    }

    if(g_nEncodeFormat == 1||g_nEncodeFormat == 2 || g_nEncodeFormat == 4)
    {
        m_H264Capture.UnInit();
    }
	
	if(g_nDetectMode == 2)
	{
		m_TempCapture.Unit();
	}

    if(1 == g_nFlashControl)
    {
        #ifdef VIDEOSHUTTER
        //pthread_mutex_lock(&flash_mutex);
        if(flash_imgCotext)
        {
            cvReleaseImageHeader(&flash_imgCotext);
            flash_imgCotext = NULL;

            m_shut.mv_UnInit();
        }
        //pthread_mutex_unlock(&flash_mutex);
        #endif
    }
    printf("bool CSkpChannelEntity::EndDetect\n");
    return bRet;
}



//模拟信号源设备停止采集
bool CSkpChannelEntity::PalEnd()
{
    //停止采集线程
    EndFrameThread();
    //停止采集
    return m_v4lDriver.StopCapture();
}

//H264设备停止采集
bool CSkpChannelEntity::H264End()
{
    //停止采集线程
    EndFrameThread();

    if(m_pAbstractCamera)
    {
       return true;
    }

    return m_MonitorH264.StopCapture();
}
//YUV流停止采集
bool CSkpChannelEntity::YuvEnd()
{
    //停止采集线程
    EndFrameThread();
    return true;
}

//MJpeg流停止采集
bool CSkpChannelEntity::MJpegEnd()
{
    //停止采集线程
    EndFrameThread();
    //this->Close();
    //return m_MJpegDriver.EndVideoMJpeg();
    return true;
}

//停止图库识别
bool CSkpChannelEntity::PicLibEnd()
{
cerr << "PicLibEnd" << endl;
    //停止采集线程
    EndFrameThread();
    return true;
}

//启动采集线程
bool CSkpChannelEntity::BeginFrameThread()
{
	LogNormal("==BeginFrameThread()==\n");
    //线程结束标志
    m_bEndDeal = false;
    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    struct   sched_param   param;
//	printf("before param.sched_priority =%d\n",param.sched_priority );
    pthread_attr_getschedparam(&attr,   &param);
//	printf("after param.sched_priority =%d\n",param.sched_priority );
    param.sched_priority   =   20;
    pthread_attr_setschedparam(&attr,   &param);
    LogTrace(NULL, "after set param.sched_priority =%d\n",param.sched_priority );

    //分离线程
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    //启动监控线程
    int nret=pthread_create(&m_nThreadId,&attr,ThreadFrame,this);
    LogTrace(NULL, "pthread_create ThreadFrame\r\n");
    //fflush(stdout);
    LogTrace(NULL, "CSkpChannelEntity m_nThreadId=%lld",m_nThreadId);
    //成功
    if(nret!=0)
    {
        //失败
        LogError("创建通道数据采集线程失败,无法处理通道数据[%d]！\r\n",GetChannelID());
        return false;
    }

	if(g_nDetectMode != 2)
	{
		nret=pthread_create(&m_nCameraControlThreadId,&attr,ThreadCameraControl,this);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建相机控制线程失败,无法进行相机控制[%d]！\r\n",GetChannelID());
			return false;
		}
	}

    pthread_attr_destroy(&attr);
    return true;
}

//停止采集线程
void CSkpChannelEntity::EndFrameThread()
{
	LogNormal("=111=End FrameThread()=m_bEndDeal=%d=g_bEndThread=%d==\n", m_bEndDeal, g_bEndThread);

    //线程结束标志
    m_bEndDeal = true;

    //停止采集线程
    if(m_nThreadId != 0)
    {
        pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
    }

	if(m_nCameraControlThreadId != 0)
	{
		pthread_join(m_nCameraControlThreadId,NULL);
        m_nCameraControlThreadId = 0;
	}

   // if(m_Video_Format != VEDIO_PICLIB)
    {
        #ifndef NOEVENT
        if((m_sChannel.uDetectKind&DETECT_FLUX)==DETECT_FLUX)
        {
            //释放检测算法模块
            m_skpRoadDetect.UnInit();
        }
        #endif
        #ifndef NOOBJECTDETECT
        if((m_sChannel.uDetectKind&DETECT_BEHAVIOR)==DETECT_BEHAVIOR)
        {
            //释放检测算法模块
            m_ObjectBehaviorDetect.UnInit();
        }
        #endif
         #ifndef NOFEATURESEARCH
        if(((m_sChannel.uDetectKind&DETECT_CHARACTER)==DETECT_CHARACTER))
        {
            //释放检测算法模块
            m_FeatureSearchDetect.UnInit();
        }
        #endif
		 #ifndef NOFACEDETECT
        if((m_sChannel.uDetectKind&DETECT_FACE)==DETECT_FACE)
        {
            //释放检测算法模块
            m_FaceDetect.UnInit();
        }
        #endif
cerr << "EndFrameThread 2" << endl;


#ifndef NOPLATE

		if( (m_sChannel.uDetectKind&DETECT_CARNUM)|| ((g_nDetectMode == 2)))
        {
            printf("=====EndFrameThread EndCarnumDetect\n");
            //停止车牌检测
			m_RoadCarnumDetect.EndCarnumDetect();

			#ifdef DSPDATAPROCESS
			/*if(m_sChannel.nCameraType == DSP_ROSEEK_500_335)
			{
				m_DspDataProcess.Uninit();
			}*/
			#endif

        }
#endif
    }

cerr << "EndFrameThread 3" << endl;
    if((m_sChannel.uDetectKind&DETECT_VTS)==DETECT_VTS)
    {
        //停止红灯采集
        r_SerialComm.EndRedSignalCapture();
    }

    if((m_sChannel.uDetectKind&DETECT_LOOP)==DETECT_LOOP)
    {
        //停止线圈采集
        d_SerialComm.EndLoopSignalCapture();
    }

    if((m_sChannel.uDetectKind&DETECT_RADAR)==DETECT_RADAR)
    {
        //停止雷达采集
        g_RadarSerial.EndRadarSignalCapture();
    }
cerr << "EndFrameThread 4" << endl;
    return;
}

//帧数据处理
void CSkpChannelEntity::DealFrame()
{
    LogNormal("=======DealFrame====\n");
	if(g_nDetectMode == 2)//dsp方案与视频源类型无关
	{
		MJpegDealFrame();
	}
	else
	{
		//对应模块帧数据处理
		switch(m_Video_Format)
		{
		case VEDIO_PAL:
			PalDealFrame();
			break;
		case VEDIO_AVI:
			AviDealFrame();
			break;
		case VEDIO_H264:
		case VEDIO_H264_FILE:
			H264DealFrame();
			break;
		case VEDIO_MJPEG:
			MJpegDealFrame();
			break;
		case VEDIO_YUV:
		case VEDIO_YUV_FILE:
		case VIDEO_BAYER:
			YuvDealFrame();
			break;
		case VEDIO_PICLIB:
			DealPicLib();
			break;
		default:
			LogError("未知视频源格式，无法启动帧数据处理!\r\n");
			break;
		}
	}
    return;
}
//模拟信号源帧采集
void CSkpChannelEntity::PalDealFrame()
{
    //通道ID
    m_sDetectHeader.uChannelID = GetChannelID();

    UINT32 uSwitch = 0;
    UINT32 nPreSeq = 0;
    int nVideoFrameRate = 0;

    //PAL数据
    while(!g_bEndThread&&!m_bEndDeal)
    {
        struct v4l2_video_buf* buf = m_v4lDriver.NextFrame();

        //是否需要检测,根据设定的规则处理采集的数据
        //1.采集图片帧
        //2.普通图片帧直接push到通道处理中心，中心统一发送通道的数据
        //3.符和规则的图片，如第10帧或者第5帧，push到算法检测模块进行自检检测

        if(buf != NULL && (buf->size == m_uWidth*m_uHeight*2))
        {
            //判断是否需要跳帧
            if(uSwitch%4==0)
            {
                nPreSeq = uSwitch;

                //帧号
                m_sDetectHeader.uSeq = uSwitch;
                //时间戳
                m_sDetectHeader.uTimestamp = GetTimeStamp();
                //时间戳
                m_sDetectHeader.uTime64 = buf->info.ts;
                //帧率
                m_sDetectHeader.dFrameRate = 25;

                nVideoFrameRate = m_sDetectHeader.dFrameRate/12;

                //printf("=====buf->info.ts=%lld,buf->size=%d\n",buf->info.ts,buf->size);
                //将yuv转换为RGB
                IppiSize srcSize= {m_uWidth, m_uHeight};
                int desstep =  m_uWidth * 3 + IJL_DIB_PAD_BYTES(m_uWidth,3);
                int srcstep =  m_uWidth * 2 + IJL_DIB_PAD_BYTES(m_uWidth,2);

                //转换为RGB
                ippiYUV422ToRGB_8u_C2C3R(buf->data,srcstep,m_chImageData,desstep,srcSize);


                //车牌检测每帧都要送检测
                #ifndef NOPLATE
                if( (m_sChannel.uDetectKind&DETECT_CARNUM)==DETECT_CARNUM)
                {
                    yuv_video_buf RGBHeader;
                    RGBHeader.nSeq = m_sDetectHeader.uSeq;
                    RGBHeader.uTimestamp = m_sDetectHeader.uTimestamp;
                    RGBHeader.ts = m_sDetectHeader.uTime64;
                    RGBHeader.nFieldSeq = m_sDetectHeader.uSeq;

                    RGBHeader.width = m_uWidth;
                    RGBHeader.height = m_uHeight;
                    RGBHeader.size = m_uWidth*m_uHeight*3;
                    printf("========pal carnum\n");
                    //直接送rgb图象
                    m_RoadCarnumDetect.AddFrame(&RGBHeader,m_chImageData);
                }
                #endif

                bool bDealVideo = false;
                if(uSwitch%12==0)
                bDealVideo = true;
                DealNormalPic(true,nVideoFrameRate,bDealVideo);	//保存普通未检测图片
            }

            //释放数据
            m_v4lDriver.ReleaseVideoBuffer(buf);

            uSwitch ++;

            //采集到数据，计时器即清0
            m_uTimer = 0;

            //定时选择预置位(防止被他人移动相机)
            if((g_nKeyBoardID >0 ) && g_ytControlSetting.nNeedControl == 1)
            {
                if(((m_sChannel.uDetectKind&DETECT_FLUX) == DETECT_FLUX))
                GotoPreSet(uSwitch);
            }
        }
        else//没有采集到数据，计时器加1(ms)
        {
            m_uTimer++;
            if (m_uTimer > 5000)//5s之内没有收到视频数据,则认为视频源断开了
            {
                LogWarning("视频源断开！\n");
                m_uTimer = 0;
                g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_NO_VIDEO);

            }
        }

        //1毫秒
        usleep(1000*1);
    }

    return;
}

//读avi文件
void CSkpChannelEntity::AviDealFrame()
{
    //通道ID
    m_sDetectHeader.uChannelID = GetChannelID();

    UINT32 uSwitch = 0;
    UINT32 nSeq = 0;
    int nVideoFrameRate = 0;

    //PAL数据
    while(!g_bEndThread&&!m_bEndDeal)
    {
        IplImage* buf = NULL;

        buf = m_v4lDriver.NextAviFrame(uSwitch);

        if(buf != NULL)
        {
            //判断是否需要跳帧
            if(nSeq%4==0)
            {
                //保存数据
                memcpy(m_chImageData,buf->imageData,buf->imageSize);

                //帧号
                m_sDetectHeader.uSeq = nSeq;
                //时间戳
                m_sDetectHeader.uTimestamp = GetTimeStamp();
                //时间戳
                m_sDetectHeader.uTime64 = nSeq*((int64_t)40000);
                //帧率
                m_sDetectHeader.dFrameRate = 25;
                nVideoFrameRate = m_sDetectHeader.dFrameRate/4;

                printf("uChannelID=%d,m_uWidth=%d,m_uHeight=%d\r\n",m_sDetectHeader.uChannelID,m_uWidth,m_uHeight);
                fflush(stdout);

                DealNormalPic(true,nVideoFrameRate);	//保存普通未检测图片
            }

            //采集到数据，计时器即清0
            m_uTimer = 0;
            nSeq++;
        }
        else//没有采集到数据，计时器加1(ms)
        {
            m_uTimer++;
            if (m_uTimer > 5000)//5s之内没有收到视频数据,则认为视频源断开了
            {
                LogWarning("视频源断开！\n");
                m_uTimer = 0;
                g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_NO_VIDEO);

            }
        }
        //1毫秒
        usleep(1000*1);
    }
    return;
}

//H264采集
void CSkpChannelEntity::H264DealFrame()
{
    LogTrace(NULL, "=========H264DealFrame===\r\n");
    int nFileCount = 0;
    int nFileIdx = 0;
    int64_t tsEndTime = 0;
	int64_t tsPreTime = 0;
    int nVideoType = -1;

    if(m_pAbstractCamera == NULL ||
		m_sChannel.nCameraType == HK_CAMERA||
		m_sChannel.nCameraType == CF_CAMERA_2 ||
		m_sChannel.nCameraType == CF_CAMERA ||
		m_sChannel.nCameraType == ZION_CAMERA)
    {
        //需要等待登陆返回(本地视频源不需要等待)
		if(m_pAbstractCamera == NULL)
		{
			nVideoType  = m_MonitorH264.GetVideoType();
		}
		else if(m_sChannel.nCameraType == HK_CAMERA || m_sChannel.nCameraType == ZION_CAMERA)
		{
			nVideoType = 0;
		}
		else if(m_sChannel.nCameraType == CF_CAMERA || m_sChannel.nCameraType == CF_CAMERA_2)
		{
			nVideoType = m_pAbstractCamera->GetVideoType();
		}

        if(nVideoType <= 1)
        {
			while(g_nLoginState != 1 && !m_bEndDeal)
		    {
			    LogTrace(NULL, "====waiting for LoginStateCallBackFunc g_nLoginState=%d,&g_nLoginState=%lld\n",g_nLoginState,&g_nLoginState);

			    sleep(1);
		    }

		    if(g_nLoginState == 1)//登陆成功
            {
				sleep(5);
				int nTime = 0;

				if(m_pAbstractCamera == NULL)
				{
					if(nVideoType == 1)//
					{
	#ifndef FMFS_MONITOR
						m_MonitorH264.GetDeviceId();
	#endif
						//先获取文件列表
						nFileCount = m_MonitorH264.GetFileCount();

						if(nFileCount <= 0)
						{
							//如果文件都处理过则直接退出
						   LogNormal("所有文件处理完毕\n");
						   return;
						}
					}
					 else
					 {
						m_MonitorH264.GetDeviceId();
					 }
					tsEndTime = m_MonitorH264.PullStream(0);
					LogTrace(NULL, "======tsEndTime=%lld\n",tsEndTime);
				}
				else
				{
					m_pAbstractCamera->PullStream();
				}

                //重新分配内存
                while(!m_bEndDeal)
                {
                    sleep(1);

					{
						int nWidth = 	0;
						int nHeight = 	0;
						
						if(m_pAbstractCamera)
						{
							nWidth = 	m_pAbstractCamera->GetEffectImageWidth();
							nHeight = 	m_pAbstractCamera->GetEffectImageHeight();
						}
						else
						{
							yuv_video_buf* pVideoInfo = m_MonitorH264.GetVideoInfo();

							nWidth = pVideoInfo->width;
							nHeight = pVideoInfo->height;
							LogTrace("fmfs.log", "RoadChannelEntity:width:%d, height:%d", pVideoInfo->width, pVideoInfo->height);
						}

						if( (nWidth > 0) &&(nHeight > 0))
						{
							if(m_uWidth != nWidth)
							{
								m_uWidth = nWidth;
								m_uHeight = nHeight;
								
								if(m_uWidth < 500)
								{
									//宽度
									m_sDetectHeader.uWidth = m_uWidth;
									//高度
									m_sDetectHeader.uHeight = m_uHeight;
								}
								else if(m_uWidth < 1000)
								{
									//宽度
									m_sDetectHeader.uWidth = m_uWidth/2;
									//高度
									m_sDetectHeader.uHeight = m_uHeight/2;
								}
								else
								{
									//宽度
									m_sDetectHeader.uWidth = m_uWidth/4;
									//高度
									m_sDetectHeader.uHeight = m_uHeight/4;
								}

								if(AlignMemory())
								{
									#ifndef NOEVENT
									if((m_sChannel.uDetectKind&DETECT_FLUX)==DETECT_FLUX)
									{
										if(!m_skpRoadDetect.Init(GetChannelID(),m_uWidth,m_uHeight,m_sDetectHeader.uWidth ,m_sDetectHeader.uHeight))
									   {
											LogError("事件检测模块初始化失败，无法检测！\r\n");
										}
									}
									#endif

									#ifndef NOOBJECTDETECT
									if((m_sChannel.uDetectKind&DETECT_BEHAVIOR)==DETECT_BEHAVIOR)
									{
										if(!m_ObjectBehaviorDetect.Init(GetChannelID(),m_uWidth,m_uHeight,m_sDetectHeader.uWidth ,m_sDetectHeader.uHeight))
										{
											LogError("行为分析检测模块初始化失败，无法检测！\r\n");
										}
									}
									#endif

								#ifndef NOPLATE
								if((m_sChannel.uDetectKind&DETECT_CARNUM)==DETECT_CARNUM)
									{
										//车牌检测
										if(!m_RoadCarnumDetect.Init(GetChannelID(),m_uWidth,m_uHeight,m_sDetectHeader.uWidth ,m_sDetectHeader.uHeight))
										{
											LogError("车牌检测模块启动失败，无法检测！\r\n");
										}
									}
									#endif
								}
							}
							break;
						}
						else
						{
							LogError("无法获取宽高信息！\r\n");
							nTime++;
							if(nTime >= 5)//如果5秒钟仍然不返回继续请求视频流
							{
								if(m_pAbstractCamera)
								{
									m_pAbstractCamera->PullStream();//有可能不会返回
								}
								else
								{
								 tsEndTime = m_MonitorH264.PullStream(0);
								}
								LogError("视频请求失败继续请求视频");
								nTime = 0;
							}
						}
					}
					/*else
					{
						int nFrameSize = m_pAbstractCamera->GetFrameSize();
						if(nFrameSize > 0)
						{
							LogError("获取视频成功！\r\n");
							break;
						}
						else
						{
							LogError("获取视频失败！\r\n");
							nTime++;
							if(nTime >= 10)//如果5秒钟仍然不返回继续请求视频流
							{
								m_pAbstractCamera->PullStream();
								LogError("视频请求失败继续请求视频");
								nTime = 0;
							}
						}
					}*/
                }//end while
            }

			m_nVideoType = 0;
        }
		else
		{
			m_nVideoType = 1;
		}
    }

	 #ifndef NOFEATURESEARCH
	    //设置视频类型
		m_FeatureSearchDetect.SetVideoType(m_nVideoType);
	 #endif

    //通道ID
    m_sDetectHeader.uChannelID = GetChannelID();

    UINT32 nSeq = 0;
    int nVideoFrameRate = 0;
    BYTE* pBuffer = NULL;
    yuv_video_buf header;

    //H264数据
    while(!g_bEndThread&&!m_bEndDeal)
    {
            int nSize = 0;


            if(m_pAbstractCamera)
            {
                nSize = m_pAbstractCamera->PopFrame(&pBuffer);

                if(nSize > 0)
                {
                    header = *((yuv_video_buf*)pBuffer);
                    memcpy(m_chImageData,pBuffer+sizeof(yuv_video_buf),m_uWidth*m_uHeight*3);
                    m_pAbstractCamera->DecreaseSize();
                }
            }
            else
            {
                nSize = m_MonitorH264.PopFrame(header,m_chImageData);

                //根据分辨率去除上面的字
                if(m_uWidth < 500)
                {
                  //  memset(m_chImageData,0,m_uWidth*3*40);
                }
            }
          //  LogTrace(NULL, "===========h264 deal frame================ nSize=%d", nSize);
            if(nSize>0)
            {
                //采集到数据，计时器即清0
                m_uTimer = 0;
                if(m_nStatus != NORMAL_STATUS)//将相机状态置为正常
                {
                    LogNormal("视频恢复\r\n");
                    m_sChannel.nCameraStatus = CM_OK;
                    m_sChannel.bRun = NORMAL_STATUS;

                    //视频恢复
                    g_skpDB.UpdateChannelStatus(m_sChannel.uId,NORMAL_STATUS);
                    SetStatus(NORMAL_STATUS);
                    g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_OK);
                }

               // printf("nFrameRate=%d,nowTime=%lld,nPicHeight=%d,nPicWidth=%d\r\n",header.nFrameRate,header.ts,header.height,header.width);
					if(g_nForceRedLlight > 0)//如果强制闯红灯
					{
						if((m_sChannel.uDetectKind&DETECT_VTS) == DETECT_VTS)
						header.uTrafficSignal = g_uTrafficSignal;
					}

                    {
                        //帧号
                        m_sDetectHeader.uSeq = nSeq;
                        //时间戳
                        m_sDetectHeader.uTimestamp = (header.ts/1000)/1000;
                        //时间戳
                        m_sDetectHeader.uTime64 = header.ts;
						//红灯状态
						m_sDetectHeader.uTrafficSignal = header.uTrafficSignal;

						if(nVideoType == 2)
						{
							if(tsPreTime > header.ts)
							{
								m_sChannel.uVideoEndTime = m_sChannel.uVideoBeginTime;
								LogNormal("历史视频分析完成,id=%d,b=%lld,e=%lld",m_sChannel.nCameraId,m_sChannel.uVideoBeginTime,m_sChannel.uVideoEndTime);
							}
							tsPreTime = header.ts;//上次时间
						}

                        int nSkip = 1;
                        if(header.nFrameRate >= 20)
                        {
                            nSkip = 5;
                        }
						else if(header.nFrameRate >= 15)
                        {
                            nSkip = 3;
                        }
                        else if(header.nFrameRate >= 10)
                        {
                            nSkip = 2;
                        }
                        else
                        {
                            nSkip = 1;
                        }

						if(!(m_sChannel.uDetectKind&DETECT_FLUX))//事件检测跳帧
						{
							if(m_uWidth <= 1500)
							{
								nSkip = 1;
							}
						}

                        #ifndef NOPLATE
                        //if(nSeq%2 == 0)
                        {
                                if( (m_sChannel.uDetectKind&DETECT_CARNUM)==DETECT_CARNUM)
                                {
                                    yuv_video_buf RGBHeader;
                                    RGBHeader.nSeq = m_sDetectHeader.uSeq;
                                    RGBHeader.uTimestamp = m_sDetectHeader.uTimestamp;
                                    RGBHeader.ts = m_sDetectHeader.uTime64;
                                    RGBHeader.nFieldSeq = m_sDetectHeader.uSeq;
									RGBHeader.uTrafficSignal=header.uTrafficSignal;

                                    RGBHeader.width = m_uWidth;
                                    RGBHeader.height = m_uHeight;
                                    RGBHeader.size = m_uWidth*m_uHeight*3;

                                    //直接送rgb图象
                                    m_RoadCarnumDetect.AddFrame(&RGBHeader,m_chImageData);
                                }
                        }
                        #endif
						
						//if(g_VideoFormatInfo.nSendH264 == 1)
						{
							nSkip = 1;
						}

                        bool bSkip = ((nSeq%nSkip) == 0);//是否跳帧
						 bool bDealVideo = bSkip;//录像是否跳帧

						 if(g_nWorkMode == 1)
						{
							bDealVideo = true;
						}

						// LogNormal("nFrameRate=%d,nSkip=%d,bSkip=%d\n",header.nFrameRate,nSkip,bSkip);

                        if(bSkip || bDealVideo)
                        {
                            //帧率
                            m_sDetectHeader.dFrameRate = header.nFrameRate;
                            nVideoFrameRate = m_sDetectHeader.dFrameRate/nSkip;

                           // printf("m_sDetectHeader.uTimestamp=%lld,nSeq=%d,%s\r\n",m_sDetectHeader.uTimestamp,nSeq,GetTime(m_sDetectHeader.uTimestamp));
                            //fflush(stdout);

                            ////////////////////////缩放
                            DealNormalPic(bSkip,nVideoFrameRate,bDealVideo);	//保存普通未检测图片
                        }

                        nSeq++;

                        //定时选择预置位(防止被他人移动相机)
                        if(g_nKeyBoardID >0 && g_ytControlSetting.nNeedControl == 1 || (m_sChannel.nCameraType == HK_CAMERA))
                        {
                            if(((m_sChannel.uDetectKind&DETECT_FLUX) == DETECT_FLUX) ||
                               ((m_sChannel.uDetectKind&DETECT_BEHAVIOR) == DETECT_BEHAVIOR) )
                           {
                                GotoPreSet(nSeq);
                           }
                        }
                    }
            }
            else//没有采集到数据，计时器加1(ms)
            {
                m_uTimer++;
                if (m_uTimer > 5000)//5s之内没有收到视频数据,则认为视频源断开了
                {
                    m_uTimer = 0;

                    if(m_nStatus != UNNORMAL_STATUS)
                    {
                        g_skpRoadLog.WriteLog("无视频\n",ALARM_CODE_NO_VIDEO,true);
                        m_sChannel.nCameraStatus = ALARM_CODE_NO_VIDEO;
                        m_sChannel.bRun = UNNORMAL_STATUS;

                        //无视频，异常
                        g_skpDB.UpdateChannelStatus(m_sChannel.uId,UNNORMAL_STATUS);
                        SetStatus(UNNORMAL_STATUS);
                        g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_NO_VIDEO);
                    }

                    //断流重拉
                    if(m_pAbstractCamera == NULL)
                    {
                        m_MonitorH264.ReOpen();
                    }
                    else
                    {
						//LogTrace("sanyo.log", "RoadChannel::Reopen camera. m_bEndDeal=%d", m_bEndDeal);
                        m_pAbstractCamera->ReOpen();
                    }
                }
            }

            //判断某段文件是否分析完成,如果完成则切换到下一个文件
            if(m_pAbstractCamera == NULL)
            {
                if(nVideoType == 1)
                {
                    if((m_sDetectHeader.uTime64 + 200000 >= tsEndTime) && (tsEndTime > 0))
                    {
                        m_MonitorH264.StopCapture(true);

                        nFileIdx++;
                        if(nFileIdx < nFileCount)
                        {
                            tsEndTime = m_MonitorH264.PullStream(nFileIdx);
                            printf("======tsEndTime=%lld\n",tsEndTime);
                            LogNormal("某段文件分析完成,切换到下一个文件\n");
                        }
                        else
                        {
                            //停止分析
                            LogNormal("所有文件处理完毕,停止分析\n");
                            break;
                        }
                    }
                }
            }
        //1毫秒
        usleep(1000*1);
    }

    return;
}

//MJpeg处理
void CSkpChannelEntity::MJpegDealFrame()
{
    //LogNormal("===in======CSkpChannelEntity::MJpegDealFrame===\r\n");

    bool bGetOnCapture = false; //是否取一张截图

    //通道ID
    m_sDetectHeader.uChannelID = GetChannelID();

    UINT32 nSeq = 1;
	//just for test
	UINT32 nSeqPre = 0;

    int nVideoFrameRate = 0;

    //Jpeg数据
    while(!g_bEndThread && !m_bEndDeal)
    {
            int nSize1 = 0;
            int nSize2 = 0;
            int nSize3 = 0;
            BYTE* pBuffer1 = NULL;
            BYTE* pBuffer2 = NULL;
            //车牌相关信息处理
            DSP_PLATE_LIST dspPlateList;

			//printf("==1111==CSkpChannelEntity::MJpegDealFrame()===\n");

            if(m_pAbstractCamera)
            {
                //printf("=======1111111111=========\n");
				nSize1 = 0;
				nSize2 = 0;
				nSize3 = 0;
				if(dspPlateList.size() > 0)
				{
					dspPlateList.clear();
				}

				if(m_sChannel.nCameraType == AXIS_CAMERA)
				{
					nSize1 = m_pAbstractCamera->PopFrame(&pBuffer1);

					if(nSize1 > 0)
					{
						yuv_video_buf header1;
						header1 = *((yuv_video_buf*)pBuffer1);

                    #ifdef _DSP_DEBUG
						printf("===111===nSize1=%d==1111111==22222222===header1.size=%d===\n", \
							nSize1, header1.size);
                    #endif

						memcpy(m_JpgImageData, pBuffer1, header1.size + sizeof(yuv_video_buf));


						//LogTrace("Mem_TT.log", "===111===nSize1=%d==1111111=====header1.size=%d===\n", \
							nSize1, header1.size);

						m_pAbstractCamera->DecreaseSize();
					}
				}
				else
				{
                /////////////1.小图///////////////////
                    //BYTE* pBuffer1 = NULL;
                    nSize1 = m_pAbstractCamera->PopJpgFrame(&pBuffer1);

                    if(nSize1 > 0)
                    {
                        yuv_video_buf header1;
                        header1 = *((yuv_video_buf*)pBuffer1);

                    #ifdef _DSP_DEBUG
                        printf("===111===nSize1=%d==1111111====22222222=====header1.size=%d===\n", \
							nSize1, header1.size);
                    #endif

                        memcpy(m_JpgImageData, pBuffer1, header1.size + sizeof(yuv_video_buf));

						//LogTrace("Mem_TT.log", "===111===nSize1=%d=====22222222=====header1.size=%d===\n", \
							nSize1, header1.size);

                        m_pAbstractCamera->DecreaseJpgSize();
                    }
                /////////////2.大图///////////////////
                    //BYTE* pBuffer2 = NULL;
                    nSize2 = m_pAbstractCamera->PopJpgFrame(&pBuffer2, 2);

                    if(nSize2 > 0)
                    {
                        yuv_video_buf header2;
                        header2 = *((yuv_video_buf*)pBuffer2);

                    #ifdef _DSP_DEBUG
                        printf("===222===nSize2=%d==1111111====22222222===header2.size=%d===\n", \
							nSize2, header2.size);
                    #endif
						                        
                        memcpy(m_JpgImageData2, pBuffer2, header2.size + sizeof(yuv_video_buf));
						//LogTrace("Mem_TT.log", "===111===nSize2=%d=====3333=====header2.size=%d===\n", \
							nSize2, header2.size);

                        m_pAbstractCamera->DecreaseJpgSize(2);

#ifdef DEBUG_PLATE_TEST
						//just for test 
						TestAddPlate(header2);//添加模拟测试车牌
						//End test
#endif

                    }
                    //bDealPic = false;

                /////////////3.车牌///////////////////
                    nSize3 = m_pAbstractCamera->PopPlateList(dspPlateList);
                    //printf("=====dspPlateList.size()=%d===\n", dspPlateList.size());

                    #ifdef _DSP_DEBUG
                    if(nSize3 > 0)
                    {
                        //LogTrace("LogDsp.log", "=====dspPlateList.size()=%d==nSize123=%d,%d,%d=\n", \
							dspPlateList.size(), nSize1, nSize2, nSize3);
                    }
                    #endif

				}
            }

#ifdef REBOOT_DETECT
			if(nSize1 > 0 || nSize2 > 0 || nSize3 > 0)
			{
				m_nRebootFlag = 0;//重启计数归0
			}
#endif

            if(nSize1>0)
            {
                //判断是否需要送车牌检测
                bool bDetectCarnum = ((m_sChannel.uDetectKind & DETECT_CARNUM)==DETECT_CARNUM);

                yuv_video_buf header1;
                header1 = *((yuv_video_buf*)m_JpgImageData);
                //采集到数据，计时器即清0
                m_uTimer = 0;
                if(m_nStatus != NORMAL_STATUS)//将相机状态置为正常
                {
                    LogNormal("视频恢复\r\n");
                    m_sChannel.nCameraStatus = CM_OK;
                    m_sChannel.bRun = NORMAL_STATUS;

                    //视频恢复
                    g_skpDB.UpdateChannelStatus(m_sChannel.uId, NORMAL_STATUS);
                    SetStatus(NORMAL_STATUS);
                    g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_OK);

					 //往客户端发送通道状态
                    std::string response;
                    response.insert(0,(char*)&m_sChannel,sizeof(SRIP_CHANNEL));
                    SRIP_DETECT_HEADER sDetectHeader;
                    sDetectHeader.uDetectType = SRIP_CHANNEL_STATUS;
                    response.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
                    g_skpChannelCenter.AddResult(response);


                    #ifndef NOPLATE
                    if( (m_sChannel.uDetectKind&DETECT_CARNUM)==DETECT_CARNUM)
                    {
                        //重新初始化目标检测
                        m_RoadCarnumDetect.ReInit();
                    }
                    #endif
                }

                //printf("nFrameRate=%d,nowTime=%lld,nPicHeight=%d,nPicWidth=%d\r\n",header.nFrameRate,header.ts,header.height,header.width);
                #ifdef _DSP_DEBUG
                printf("=======22222222222222=========\n");
                #endif

                //if(bDealPic && header1.size > 0)
                if(header1.size > 0 && g_nDetectMode != 2)
                {
                #ifdef _DSP_DEBUG
                    printf("=====in==jpg->rgb=========\n");
                    printf("====m_sDetectHeader.uWidth=%d==m_sDetectHeader.uHeight=%d==\n", m_sDetectHeader.uWidth, m_sDetectHeader.uHeight);
                #endif

                    int nRgbSize = 0;
                    //解码jpg->rgb解码小图
                    DecodeJpgToRgb(m_JpgImageData, m_sDetectHeader.uWidth, m_sDetectHeader.uHeight, nRgbSize, m_chImageData);

                    #ifdef _DSP_DEBUG
                    printf("=====out==jpg->rgb=========\n");
                    #endif


                        nSeq = header1.nSeq;

                        #ifdef _DSP_DEBUG
                        printf("==MJpegDealFrame===nSeq=%d===\n", nSeq);
                        #endif
                        //帧号
                        m_sDetectHeader.uSeq = nSeq;
                        //时间戳
                        m_sDetectHeader.uTimestamp = (header1.ts/1000)/1000;
                        //时间戳
                        m_sDetectHeader.uTime64 = header1.ts;


                        //红灯状态
						if(g_nForceRedLlight > 0)//如果强制闯红灯
						{
							if((m_sChannel.uDetectKind&DETECT_VTS) == DETECT_VTS)
								m_sDetectHeader.uTrafficSignal = g_uTrafficSignal;
						}
						else
						{
							m_sDetectHeader.uTrafficSignal = header1.uTrafficSignal;
						}


                        //线圈和雷达状态（高16雷达数据，低16位线圈数据）
                        //header1.uRadarSignal.SpeedSignal = 20;
                        m_sDetectHeader.uRealTime = (header1.uRadarSignal.SpeedSignal<<16) | (header1.uSpeedSignal.SpeedSignal);


                        int nSkip = 1;
                        if(header1.nFrameRate >= 20)
                        {
                            nSkip = 5;
                        }
                        else if(header1.nFrameRate >= 10)
                        {
                            nSkip = 2;
                        }
                        else
                        {
                            nSkip = 1;
                        }

                        #ifndef NOPLATE
                        //if(nSeq%nSkip == 0)
                        {

                                if( (m_sChannel.uDetectKind&DETECT_CARNUM)==DETECT_CARNUM)
                                {
                                    yuv_video_buf RGBHeader;
                                    RGBHeader.nSeq = m_sDetectHeader.uSeq;
                                    RGBHeader.uTimestamp = m_sDetectHeader.uTimestamp;
                                    //RGBHeader.ts = m_sDetectHeader.uTime64;
                                    RGBHeader.nFieldSeq = m_sDetectHeader.uSeq;

                                    RGBHeader.width = m_uWidth;
                                    RGBHeader.height = m_uHeight;
                                    RGBHeader.size = m_uWidth*m_uHeight*3;

                                    //add by yuwx
                                    RGBHeader.uSpeedSignal.SpeedSignal = header1.uSpeedSignal.SpeedSignal;
                                    RGBHeader.uSpeedSignal.SystemTime = header1.uSpeedSignal.SystemTime;
                                    RGBHeader.uSpeedSignal.SpeedTime = header1.uSpeedSignal.SpeedTime;
                                    RGBHeader.uFlashSignal = header1.uFlashSignal;

                                    RGBHeader.ts = header1.ts;
									RGBHeader.uTrafficSignal = m_sDetectHeader.uTrafficSignal;


                                    //LogNormal("==m_sDetectHeader.uTime64=%lld,header1.ts=%lld\n", m_sDetectHeader.uTime64, header1.ts);
                                    //直接送rgb图象
                                    m_RoadCarnumDetect.AddFrame(&RGBHeader,m_chImageData);

                                }

                        }
                        #endif

#ifdef _DSP_DEBUG
                        printf("=333333333333===================\n");
#endif
                        bool bSkip = ((nSeq%nSkip) == 0);//是否跳帧
                        if(bSkip)
                        {
                            //帧率
                            m_sDetectHeader.dFrameRate = header1.nFrameRate;
                            nVideoFrameRate = m_sDetectHeader.dFrameRate/nSkip;

                            //printf("m_sDetectHeader.uTime64=%lld,m_sDetectHeader.uTimestamp=%lld,nSeq=%d,%s\r\n",m_sDetectHeader.uTime64,m_sDetectHeader.uTimestamp,nSeq,GetTime(m_sDetectHeader.uTimestamp));
                            //fflush(stdout);

                            ////////////////////////缩放
                            //DealNormalMJpgPic(true, nVideoFrameRate, true);	//保存普通未检测图片
                        } //End of if(bSkip)

                        //nSeq++;
                        //相机自动控制
                        if(g_nCameraControl==1)
                        {
                            //定时选择预置位(防止被他人移动相机)
                            if(g_nKeyBoardID >0 && g_ytControlSetting.nNeedControl == 1)
                            {
                                if(((m_sChannel.uDetectKind&DETECT_FLUX) == DETECT_FLUX) ||
                               ((m_sChannel.uDetectKind&DETECT_BEHAVIOR) == DETECT_BEHAVIOR) )
                               {
                                    GotoPreSet(nSeq);
                               }
                            }

							
                            //只有事件检测和车牌检测时才需要控制相机
                            if(((m_sChannel.uDetectKind&DETECT_FLUX) == DETECT_FLUX) ||bDetectCarnum ||
                               ((m_sChannel.uDetectKind&DETECT_BEHAVIOR) == DETECT_BEHAVIOR) )
                            {
								 //计算地面亮度
								if(!bDetectCarnum)
								{
									CalculateRoadContext();
								}
                                //CameraAutoControl(m_sDetectHeader.uTimestamp);
                                //LogNormal("==CameraAutoControl==\n");
                            }
                        }
                    }//End of if(header1.size > 0)
            } //End of if(nSize1>0)

            if(nSize2 > 0)
            {
				m_uTimer = 0;
				//printf("=11==m_RoadCarnumDetect.AddJpgFrame==");
                 //添加大图处理
                 m_RoadCarnumDetect.AddJpgFrame(m_JpgImageData2);
                 // m_skpRoadDetect.AddJpgFrame(m_JpgImageData2);

            }//End of if(nSize2 > 0)
			else
			{
				//1拖N模式如果长时间未收到大图自动重连(暂定5分钟)
				if(g_nDetectMode == 2 && g_nGongJiaoMode != 1)
				{
					if(m_pAbstractCamera)
					{
						 int nRecvNothingFlag = m_pAbstractCamera->GetRecvNothingFlag();

						 if(nRecvNothingFlag > 30000) //5分钟
						 {
							LogNormal("长时间未收到大图自动重连5分钟");
							//bool bOpenCamera = m_pAbstractCamera->ReOpen();
							//this->ReOpenDsp();
							g_skpRoadLog.WriteLog("相机异常\n",ALARM_CAMERA_BREAK ,true);
							m_pAbstractCamera->DisConnectCamera();
						 }
					}

					m_uTimer++;
					if (m_uTimer > 600000)//20分钟
					{
						m_uTimer = 0;
						
#ifdef REBOOT_DETECT
						/*m_nRebootFlag ++;
						LogNormal("ID:%d m_nRebootFlag:%d ", m_sChannel.uId, m_nRebootFlag);

						if(m_nRebootFlag > 3) //60分钟未收到数据,检测器程序进行重启,保证能重连上相机
						{
							bool bConnect = m_pAbstractCamera->GetCameraState();
							if(bConnect)
							{
								m_nRebootFlag = 0;
								g_bEndThread = true;
								LogNormal("长时间未收到数据,软件复位! \n");
								sleep(3);
								exit(-1);
							}							
						}*/
#endif

						//断流重拉
						if(m_pAbstractCamera)
						{
							LogNormal("长时间未收到大图自动重连20分钟!");
							g_skpRoadLog.WriteLog("相机断开\n",ALARM_CAMERA_BREAK ,true,m_sChannel.nCameraId);							
							//this->ReOpenDsp();			
							m_pAbstractCamera->DisConnectCamera();
							g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_BREAK);
						}
					}
				}
			}

            if(nSize3 > 0)
            {
				 m_uTimer = 0;
                #ifdef _DSP_DEBUG
                printf("===nSize3=%d==\n", nSize3);
                #endif
                //传输DSP输出的车牌信息队列
                m_RoadCarnumDetect.AddPlateFrame(dspPlateList);
            }
            /*
            /////////////////////////
            else//模拟检出车牌
            {
				yuv_video_buf header3;
				header3 = *((yuv_video_buf*)m_JpgImageData);
				nSeq = header3.nSeq;

				if((nSeq % 50 == 0) && nSeqPre != nSeq)
				{
					printf("==nSeq=%d, nSeqPre=%d\n", nSeq, nSeqPre);
					nSeqPre = nSeq;
					//帧号
					m_sDetectHeader.uSeq = nSeq;
					//时间戳
					m_sDetectHeader.uTimestamp = (header3.ts/1000)/1000;
					//时间戳
					m_sDetectHeader.uTime64 = header3.ts;

					BYTE *buf;
					buf = new BYTE[sizeof(RECORD_PLATE_DSP)];
					RECORD_PLATE_DSP *pPlate = (RECORD_PLATE_DSP*)(buf);

					DSP_PLATE_LIST plateList;

					pPlate->uSeq = nSeq;
					pPlate->uTime = GetTimeStamp();

					//memcpy(pPlate->chText ,"ABC1234", 7);
					char bufTT[8] = {0};
					sprintf(bufTT, "ABC1%03d", ((nSeq / 100) % 100) );
					memcpy(pPlate->chText, bufTT, 7);

					pPlate->uColor = 3;
					pPlate->uCredit = 80;
					pPlate->uRoadWayID = 1;
					pPlate->uType = 0x00010001;
					pPlate->uSmallPicSize = 20;
					pPlate->uSmallPicWidth = 10;
					pPlate->uSmallPicHeight = 5;
					pPlate->uPicSize = 1000;
					pPlate->uPicWidth = 612;
					pPlate->uPicHeight = 512;
					pPlate->uPosLeft = 80;
					pPlate->uPosTop = 80;
					pPlate->uPosRight = 120;
					pPlate->uPosBottom = 180;
					pPlate->uCarColor1 = 3;
					pPlate->uSpeed = 20;
					pPlate->uDirection = 3;
					pPlate->uCarBrand = 15;
					pPlate->uCarColor2 = 5;
					pPlate->uWeight1 = 20;
					pPlate->uWeight2 = 30;
					pPlate->uPlateType = 1;
					pPlate->uViolationType = 0;
					pPlate->uTypeDetail = 3;
					pPlate->uSeq2 = nSeq + 4;
					pPlate->uSeq3 = nSeq + 8;
					pPlate->uTime2 = 80;
					pPlate->uMiTime2 = 800;
					pPlate->uContextMean = 80;
					pPlate->uContextStddev = 34;
					pPlate->uVerticalTheta = 0;
					pPlate->uHorizontalTheta = 0;

					plateList.push_back((*pPlate));

					//m_RoadCarnumDetect.AddPlateFrame((BYTE*)pPlate);
					m_RoadCarnumDetect.AddPlateFrame(plateList);
				}//End of if
                nSeq++;
            }
            /////////////////////////
*/
            //没有采集到数据，计时器加1(ms)
            if( nSize1 <= 0 && g_nDetectMode != 2)
            {
                m_uTimer++;
                if (m_uTimer > 5000)//10s之内没有收到视频数据,则认为视频源断开了
                {
                    m_uTimer = 0;
                    
                    if(m_nStatus != UNNORMAL_STATUS)
                    {
                        g_skpRoadLog.WriteLog("无视频\n",ALARM_CODE_NO_VIDEO,true);
                        m_sChannel.nCameraStatus = ALARM_CODE_NO_VIDEO;
                        m_sChannel.bRun = UNNORMAL_STATUS;

                        //无视频，异常
                        g_skpDB.UpdateChannelStatus(m_sChannel.uId,UNNORMAL_STATUS);
                        SetStatus(UNNORMAL_STATUS);
                        g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_NO_VIDEO);
                    }

                    //g_skpRoadLog.WriteLog("无视频\n",ALARM_CODE_NO_VIDEO,true);

                    //往客户端发送通道状态
                    std::string response;
                    response.insert(0,(char*)&m_sChannel,sizeof(SRIP_CHANNEL));
                    SRIP_DETECT_HEADER sDetectHeader;
                    sDetectHeader.uDetectType = SRIP_CHANNEL_STATUS;
                    //printf("sDetectHeader.uDetectType=%x\r\n",sDetectHeader.uDetectType);
                    response.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
                    g_skpChannelCenter.AddResult(response);

					if (g_nGongJiaoMode == 1)
					{
						//1毫秒
						usleep(1000*2);
						continue;
					}

					//断流重拉
					if(m_pAbstractCamera)
					{
						//LogNormal("RoadChannel::Reopen camera. m_bEndDeal=%d", m_bEndDeal);
						m_pAbstractCamera->ReOpen();
					}

					//LogNormal("=MJpegDealFrame=没有采集到数据=等待处理=\n");

					//sleep(5);//等待5秒
                }
            }

			//printf("==ttt22222=====CSkpChannelEntity::MJpegDealFrame===\r\n");

        usleep(1000*2);
    }


	//LogNormal("==out=====CSkpChannelEntity::MJpegDealFrame===\r\n");


    return;
}

//打开文件，加载图片信息
bool CSkpChannelEntity::PicLibOpen()
{
	m_pCameraFactory = new Factory();
	m_pAbstractCamera = m_pCameraFactory->Create(m_sChannel.nCameraType,m_sChannel.eVideoFmt);

	if(m_pAbstractCamera)
	{
		//打开图库
		String strFileName = g_skpDB.GetSrcFileByID(GetChannelID());
		bool bOpenFile = m_pAbstractCamera->OpenFile(strFileName.c_str());

		m_uWidth = m_pAbstractCamera->GetEffectImageWidth();
		m_uHeight = m_pAbstractCamera->GetEffectImageHeight();

		m_sDetectHeader.uWidth = m_uWidth/4;
		m_sDetectHeader.uHeight = m_uHeight/4;
		m_RoadCarnumDetect.SetVedioFormat(VEDIO_PICLIB);
		return bOpenFile;
	}
	else
	{
		if(m_pCameraFactory)
		{
			delete m_pCameraFactory;
			m_pCameraFactory = NULL;
		}
	}
	return false;
}


//图库识别
void CSkpChannelEntity::DealPicLib()
{
	int nSize = 0;
	BYTE* pBuffer = NULL;
	yuv_video_buf buf;
	while(!g_bEndThread && !m_bEndDeal)
	{
		if(m_pAbstractCamera)
		{
			pBuffer = NULL;
			nSize = m_pAbstractCamera->PopFrame(&pBuffer);
		}

		if(nSize > 0)
		{
			buf = *((yuv_video_buf*)(pBuffer));
			memcpy(m_chImageData,pBuffer+sizeof(yuv_video_buf),buf.size);
			bool bDetectCarnum = ((m_sChannel.uDetectKind & DETECT_CARNUM)==DETECT_CARNUM);
			//用完后减
			if(m_pAbstractCamera)
				m_pAbstractCamera->DecreaseSize();
			//车牌检测每帧都要送检测
			#ifndef NOPLATE
			if(bDetectCarnum)
			{
				yuv_video_buf RGBHeader = (buf);

				//帧号
                m_sDetectHeader.uSeq = buf.nSeq;
                //时间戳
                m_sDetectHeader.uTimestamp = buf.uTimestamp;
                //时间戳
                m_sDetectHeader.uTime64 = buf.ts;

				RGBHeader.width = m_uWidth;
				RGBHeader.height = m_uHeight;
				RGBHeader.size = m_uWidth*m_uHeight*3;
				printf("====RGBHeader.width=%d,RGBHeader.height=%d\n",RGBHeader.width,RGBHeader.height);

				m_RoadCarnumDetect.AddFrame(&RGBHeader,m_chImageData);
			}
			#endif
		}
		usleep(1000*10);
	}
    return;
}

//yuv->rgb
void CSkpChannelEntity::ConvertYUVtoRGB(BYTE* pSrc,BYTE* pDest)
{
    yuv_video_buf* pCurBuffer = (yuv_video_buf*)(pSrc);

    int n = 0;
    if(m_sChannel.nYUVFormat == 3 && m_sChannel.nCameraType != H3C_CAMERA)
    {
        n = 2;
    }
    pCurBuffer->data = (BYTE*)(pSrc + sizeof(yuv_video_buf)) - n;


    int nWidth  = m_uWidth;
    int nHeight = m_uHeight;
    IppiSize srcSize= {nWidth, nHeight};
    int desstep = nWidth * 3 + IJL_DIB_PAD_BYTES(nWidth,3);
    int srcstep = pCurBuffer->width * 2 + IJL_DIB_PAD_BYTES(pCurBuffer->width,2);

    int nOffSetX = pCurBuffer->uMarginX*2;
    int nOffSetY = pCurBuffer->uMarginY*srcstep;

    //yuv->rgb
	if(m_sChannel.nCameraType == H3C_CAMERA)
	{
		//华三YUV420视频流转成RGB
		unsigned char *pic[3];
		pic[0] = pCurBuffer->data;
		pic[1] = (unsigned char*)pCurBuffer->data + nWidth*nHeight;
		pic[2] = (unsigned char*)pCurBuffer->data + nWidth*nHeight + nWidth*nHeight/4;

		ippiYUV420ToRGB_8u_P3C3((const Ipp8u**)pic,
			pDest,
			srcSize);
		return;
	}
    if(m_sChannel.nYUVFormat == 0)//"VYUY" ptg
    {
        ippiCbYCr422ToRGB_8u_C2C3R(pCurBuffer->data+nOffSetX+nOffSetY,srcstep,pDest,desstep,srcSize);
    }
    else if(m_sChannel.nYUVFormat == 1)//"UYVY"
    {
        ippiYCrCb422ToRGB_8u_C2C3R(pCurBuffer->data+nOffSetX+nOffSetY,srcstep,pDest,desstep,srcSize);
    }
    else if(m_sChannel.nYUVFormat == 2)//"YVYU"
    {
        ippiYCrCb422ToRGB_8u_C2C3R(pCurBuffer->data+nOffSetX+nOffSetY,srcstep,pDest,desstep,srcSize);
    }
    else if(m_sChannel.nYUVFormat == 3)//"YUYV"
    {
        ippiCbYCr422ToRGB_8u_C2C3R(pCurBuffer->data+nOffSetX+nOffSetY,srcstep,pDest,desstep,srcSize);
    }
}

//bayer->rgb
void CSkpChannelEntity::ConvertBAYERtoRGB(BYTE* pSrc,BYTE* pDest)
{
    yuv_video_buf* pCurBuffer = (yuv_video_buf*)(pSrc);

    pCurBuffer->data = (BYTE*)(pSrc + sizeof(yuv_video_buf));

    IplImage* imgSrc = cvCreateImageHeader(cvSize(m_uWidth,m_uHeight),8,1);
    cvSetData(imgSrc,pCurBuffer->data,imgSrc->widthStep);

    IplImage* imgDest = cvCreateImageHeader(cvSize(m_uWidth,m_uHeight),8,3);
    cvSetData(imgDest,pDest,imgDest->widthStep);


    //struct timeval tv;
    //struct timezone tz;

    //gettimeofday(&tv, &tz);
    //printf("=1111===tv.tv_sec=%d==tv.tv_usec=%d==\n", tv.tv_sec, tv.tv_usec);
	if(m_sChannel.nCameraType == PTG_GIGE_500||
	   m_sChannel.nCameraType == PTG_ZEBRA_500||
	   m_sChannel.nCameraType == JAI_CL_500)
	{
		cvCvtColor(imgSrc,imgDest,CV_BayerGR2RGB);
	}
	else
	{
		cvCvtColor(imgSrc,imgDest,CV_BayerBG2RGB);
	}

	//CV_BayerBG2RGB, CV_BayerGB2RGB, CV_BayerRG2RGB, CV_BayerGR2RGB

    //gettimeofday(&tv, &tz);
    //printf("=222===tv.tv_sec=%d==tv.tv_usec=%d==\n", tv.tv_sec, tv.tv_usec);

    cvReleaseImageHeader(&imgSrc);
    cvReleaseImageHeader(&imgDest);
}

//ResizeImage
void CSkpChannelEntity::ResizeBigImagetoSmall(BYTE* pSrc,BYTE* pDest,bool bHalfSize)
{
    //printf("==============CSkpChannelEntity::ResizeBigImagetoSmall======\n");
    //RGB视频流resize
    IppiSize roi;
    roi.width  = m_uWidth;
    roi.height = m_uHeight;

    IppiSize   dstRoi;

    if(bHalfSize)
    {
        dstRoi.width = g_nVideoWidth;
        dstRoi.height = g_nVideoHeight;
    }
    else
    {
        dstRoi.width = m_sDetectHeader.uWidth;
        dstRoi.height = m_sDetectHeader.uHeight;
    }
    int lineStep   = roi.width*3 + IJL_DIB_PAD_BYTES(roi.width,3);
    int dstImgStep = dstRoi.width*3 + IJL_DIB_PAD_BYTES(dstRoi.width,3);
    double   ratioX = (double)dstRoi.width / (double)roi.width ;
    double   ratioY = (double)dstRoi.height / (double)roi.height ;
    IppiRect srcroi= {0,0,roi.width,roi.height};

	struct timeval tv1,tv2;
	if(g_nPrintfTime == 1)
	{
		gettimeofday(&tv1,NULL);
	}
	
	#ifndef ALGORITHM_DL
    ippiResize_8u_C3R(pSrc, roi, lineStep, srcroi, pDest, dstImgStep, dstRoi,ratioX, ratioY,IPPI_INTER_NN);
	#else
	//IppiPoint dstOffset={0,0};
	//ippiResizeLinear_8u_C3R(pSrc,lineStep,pDest, dstImgStep,dstOffset,dstRoi,ippBorderInMem,NULL,NULL, NULL);

	IplImage* pOrgImg = cvCreateImageHeader(cvSize(roi.width, roi.height), 8, 3);
	IplImage* pDestImg = cvCreateImageHeader(cvSize(dstRoi.width, dstRoi.height), 8, 3);
	cvSetData(pOrgImg,pSrc,pOrgImg->widthStep);
	cvSetData(pDestImg,pDest,pDestImg->widthStep);
	cvResize(pOrgImg,pDestImg);
	cvReleaseImageHeader(&pOrgImg);
	cvReleaseImageHeader(&pDestImg);
	#endif

	if(g_nPrintfTime == 1)
	{
		gettimeofday(&tv2,NULL);
		FILE* fp = fopen("time.log","ab+");
		fprintf(fp,"ippiResize_8u_C3R==nSeq=%lld,t1=%lld,t2=%lld,dt = %lld\n",m_sDetectHeader.uSeq,(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
		fclose(fp);
	}
}

//定时选择预置位
void CSkpChannelEntity::GotoPreSet(int64_t uSwitch)
{
    //自动回归预置位
	if(g_nPreSetMode == 0)
    {
            //定时选择预置位(防止被他人移动相机)
            if(uSwitch % 3000 == 0) //每2分钟判断一次
            {
                #ifndef NOEVENT
                bool bObjMovingState = false;

                if((m_sChannel.uDetectKind&DETECT_FLUX) == DETECT_FLUX)
                {
                    bObjMovingState = m_skpRoadDetect.GetObjMovingState();
                }
                else if((m_sChannel.uDetectKind&DETECT_BEHAVIOR) == DETECT_BEHAVIOR)
                {
                    #ifndef NOOBJECTDETECT
                    bObjMovingState = m_ObjectBehaviorDetect.GetObjMovingState();
                    #endif
                }


                if(!bObjMovingState) //若发现目标没有移动
                {
                    printf("=============GOTO_PRESET m_sChannel.nPreSet=%d,m_sChannel.uId=%d\n",m_sChannel.nPreSet,m_sChannel.uId);
                    if(m_sChannel.nPreSet >0)
                    {
                        //选择预置位
                        CAMERA_CONFIG cfg;
                        cfg.nIndex = GOTO_PRESET;
                        cfg.nAddress = m_sChannel.nCameraId;
                        cfg.fValue = m_sChannel.nPreSet;

                        if(m_sChannel.nCameraType == SANYO_CAMERA)
                        {
                            if(m_pAbstractCamera!= NULL)
                            m_pAbstractCamera->ManualControl(cfg);
                        }
                        else
                        {
                            if(g_ytControlSetting.nProtocalType == 0)//虚拟键盘码协议
                            g_VisKeyBoardControl.SendKeyData(cfg, m_sChannel.nMonitorId, 1);
                            else //pelco协议
                            g_VisSerialCommunication.SendData(cfg);
                        }

                        LogNormal("相机%d自动回归预置位%d\r\n",m_sChannel.nCameraId,m_sChannel.nPreSet);
                    }
                }
                #endif
            }
    }
}

//YUV流采集
void CSkpChannelEntity::YuvDealFrame()
{
    //通道ID
    m_sDetectHeader.uChannelID = GetChannelID();
    SRIP_DETECT_HEADER sDetectHeader;
    //通道号
    sDetectHeader.uChannelID = m_sChannel.uId;

    BYTE* pBuffer = NULL;
    yuv_video_buf buf;
    int nSize = 0;

    int64_t uSwitch = 0;
    UINT32 nPreSeq = 0;
    int nVideoFrameRate = 0;


#ifdef LINUX
    //checkHD();
#endif

    //////////////
    while(!g_bEndThread&&!m_bEndDeal)
    {
        //////////////////////////////////////////获取yuv图象
            //读缓冲数据(此时仍然是yuv数据)
           // printf("=====PopFrame=m_sChannel.uId=%d\n",m_sChannel.uId);
            if(m_pAbstractCamera)
            nSize = m_pAbstractCamera->PopFrame(&pBuffer);

            if(nSize > 0)
            {
                //判断是否需要送车牌检测
                bool bDetectCarnum = ((m_sChannel.uDetectKind & DETECT_CARNUM)==DETECT_CARNUM);

                /*if(m_uTimer > 200)
                {
                    LogError("长时间未获取到视频后恢复,m_uTimer=%d\n",m_uTimer);
                }*/
                //采集到数据，计时器即清0
                m_uTimer = 0;

                if(m_nStatus != NORMAL_STATUS)//将相机状态置为正常
                {
                    LogNormal("视频恢复\r\n");
                    m_sChannel.nCameraStatus = CM_OK;
                    m_sChannel.bRun = NORMAL_STATUS;

                    //视频恢复
                    g_skpDB.UpdateChannelStatus(m_sChannel.uId,NORMAL_STATUS);
                    SetStatus(NORMAL_STATUS);
                    g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_OK);

                    //往客户端发送通道状态
                    std::string response;
                    response.insert(0,(char*)&m_sChannel,sizeof(SRIP_CHANNEL));
                    SRIP_DETECT_HEADER sDetectHeader;
                    sDetectHeader.uDetectType = SRIP_CHANNEL_STATUS;
                    response.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
                    g_skpChannelCenter.AddResult(response);

                    //重新初始化目标检测
                    #ifndef NOPLATE
                    if(bDetectCarnum)
                    {
                        m_RoadCarnumDetect.ReInit();
                    }
                    #endif
                    uSwitch = 0;
                }
                //////////////////////////////////
                buf = *((yuv_video_buf*)(pBuffer));
                if(buf.nSeq<nPreSeq)
                {
                    uSwitch = 0;
                }
                nPreSeq = buf.nSeq;
                int nSkip = (buf.nFrameRate/5);//图片跳帧数目
				if(nSkip < 1)
				{
					nSkip = 1;
				}
                bool bSkip = ((uSwitch%nSkip) == 0);//图片是否跳帧
                bool bDealPic = bSkip; //图片是否送事件检测和客户端

                int nSkipVideo = (buf.nFrameRate/g_fFrameRate);//录像跳帧数目
				if(nSkipVideo < 1)
				{
					nSkipVideo = 1;
				}
                bool bDealVideo = ((uSwitch%nSkipVideo) == 0);//录像是否跳帧
                nVideoFrameRate = g_fFrameRate;//录像帧率

				#ifdef _LOGSHUTTER
				struct timeval tv1,tv2,tv3,tv4,tv5;
				gettimeofday(&tv1,NULL);
				#endif

				struct timeval tv1,tv2;
				if(g_nPrintfTime == 1)
				{
					gettimeofday(&tv1,NULL);
				}

                //yuv->rgb
               if(m_Video_Format == VEDIO_YUV ||
                   m_Video_Format == VEDIO_YUV_FILE)
                {
                    //double t = (double)cvGetTickCount();
					if(m_uWidth >= DSP_500_BIG_WIDTH && m_Video_Format == VEDIO_YUV_FILE)
					{
						ConvertBAYERtoRGB(pBuffer,m_chImageData);
					}
					else
					{
						ConvertYUVtoRGB(pBuffer,m_chImageData);
					}
                   /*t = (double)cvGetTickCount() - t;
                   double dt = t/((double)cvGetTickFrequency()*1000.) ;
                   printf( "============CSkpChannelEntity::ConvertYUVtoRGB dt=%d ms\n",(int)dt);*/
                }
                else if(m_Video_Format == VIDEO_BAYER)//bayer->rgb
                {
                   // double t = (double)cvGetTickCount();
                   ConvertBAYERtoRGB(pBuffer,m_chImageData);
                   /*printf( "============CSkpChannelEntity::ConvertYUVtoRGB dt=%d ms\n",(int)dt);*/
                }

			   if(g_nPrintfTime == 1)
			   {
				   gettimeofday(&tv2,NULL);
				   FILE* fp = fopen("time.log","ab+");
				   fprintf(fp,"ConvertYUVtoRGB==nSeq=%lld,t1=%lld,t2=%lld,dt = %lld\n",buf.nSeq,(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
				   fclose(fp);
			   }


                //用完后减
                if(m_pAbstractCamera)
                m_pAbstractCamera->DecreaseSize();

                if(g_nForceRedLlight > 0)//如果强制闯红灯
                {
                    if((m_sChannel.uDetectKind&DETECT_VTS) == DETECT_VTS)
                    buf.uTrafficSignal = g_uTrafficSignal;
                }

#ifdef _LOGSHUTTER
				gettimeofday(&tv2,NULL);
				LogNormal("yuv->rgb time dt=%dms",((tv2.tv_sec*1000000+tv2.tv_usec) - (tv1.tv_sec*1000000+tv1.tv_usec))/1000);
#endif

                //车牌检测每帧都要送检测
                #ifndef NOPLATE
                if(bDetectCarnum)
                {
                    yuv_video_buf RGBHeader = (buf);
                    RGBHeader.width = m_uWidth;
                    RGBHeader.height = m_uHeight;
                    RGBHeader.size = m_uWidth*m_uHeight*3;
                    //调用算法接口判断是否需要打开爆闪灯
                    if(g_nFlashControl == 1)
                    {
                       // pthread_mutex_lock(&flash_mutex);
                        AutoFlashControl(&buf);
                       // pthread_mutex_unlock(&flash_mutex);
                       RGBHeader.uFlashSignal = buf.uFlashSignal;
                    }

#ifdef _LOGSHUTTER
					gettimeofday(&tv3,NULL);
					LogNormal("DetectEveryFrame time dt=%dms",((tv3.tv_sec*1000000+tv3.tv_usec) - (tv2.tv_sec*1000000+tv2.tv_usec))/1000);
#endif
					#ifdef ALGORITHM_YUV_CARNUM

					//传yuv数据
					m_RoadCarnumDetect.AddFrame(&RGBHeader,pBuffer+sizeof(yuv_video_buf));
					#else
                    //直接送rgb图象
					m_RoadCarnumDetect.AddFrame(&RGBHeader,m_chImageData);
					#endif
                }
                #endif

                if(g_nWorkMode == 1)
                {
                    bSkip = true;
                    bDealPic = true;
                }

                if(bSkip || bDealVideo)
                {
                    //帧号
                    m_sDetectHeader.uSeq = buf.nSeq;
                    //时间戳
                    m_sDetectHeader.uTimestamp = buf.uTimestamp;
                    //时间戳
                    m_sDetectHeader.uTime64 = buf.ts;
                    //帧率
                    if(buf.nFrameRate==12)
                        m_sDetectHeader.dFrameRate = 12.5;
                    else
                        m_sDetectHeader.dFrameRate = buf.nFrameRate;
                    //红灯状态
                    m_sDetectHeader.uTrafficSignal = buf.uTrafficSignal;
                    //线圈和雷达状态（高16雷达数据，低16位线圈数据）
                    //buf.uRadarSignal.SpeedSignal = 20;
                    m_sDetectHeader.uRealTime = (buf.uRadarSignal.SpeedSignal<<16) | (buf.uSpeedSignal.SpeedSignal);

                    //保存普通未检测图片
                    DealNormalPic(bDealPic,nVideoFrameRate,bDealVideo);
                }
                uSwitch ++;


				#ifdef _LOGSHUTTER
				gettimeofday(&tv4,NULL);
				LogNormal("DealNormalPic time dt=%dms",((tv4.tv_sec*1000000+tv4.tv_usec) - (tv3.tv_sec*1000000+tv3.tv_usec))/1000);
				#endif

                //相机自动控制
                if(g_nCameraControl==1)
                {
                    //定时选择预置位(防止被他人移动相机)
                    if(g_nKeyBoardID >0 && g_ytControlSetting.nNeedControl == 1)
                    {
                        if(((m_sChannel.uDetectKind&DETECT_FLUX) == DETECT_FLUX)||bDetectCarnum ||
                       ((m_sChannel.uDetectKind&DETECT_BEHAVIOR) == DETECT_BEHAVIOR) )
                       {
                            GotoPreSet(uSwitch);
                       }
                    }

                    //只有事件检测和车牌检测时才需要控制相机
                    if(((m_sChannel.uDetectKind&DETECT_FLUX) == DETECT_FLUX) ||bDetectCarnum ||
                       ((m_sChannel.uDetectKind&DETECT_BEHAVIOR) == DETECT_BEHAVIOR) )
                    {
						//计算地面亮度
						if(!bDetectCarnum)
						{
							CalculateRoadContext();
						}
                        //CameraAutoControl(m_sDetectHeader.uTimestamp);
                    }
                }

				#ifdef _LOGSHUTTER
				gettimeofday(&tv5,NULL);
				LogNormal("total time dt=%dms",((tv5.tv_sec*1000000+tv5.tv_usec) - (tv1.tv_sec*1000000+tv1.tv_usec))/1000);
				#endif
            }

            //没有采集到数据，计时器加1(ms)
            if( (nSize <= 0))
            {
                m_uTimer++;
                if (m_uTimer > 5000)//5s之内没有收到视频数据,则认为视频源断开了
                {
                    m_uTimer = 0;
                    if(m_nStatus != UNNORMAL_STATUS)
                    {
                        g_skpRoadLog.WriteLog("无视频\n",ALARM_CODE_NO_VIDEO,true);
                        m_sChannel.nCameraStatus = ALARM_CODE_NO_VIDEO;
                        m_sChannel.bRun = UNNORMAL_STATUS;

                        //无视频，异常
                        g_skpDB.UpdateChannelStatus(m_sChannel.uId,UNNORMAL_STATUS);
                        SetStatus(UNNORMAL_STATUS);
                        g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_NO_VIDEO);
                    }

                    //g_skpRoadLog.WriteLog("无视频\n",ALARM_CODE_NO_VIDEO,true);

                    //往客户端发送通道状态
                    std::string response;
                    response.insert(0,(char*)&m_sChannel,sizeof(SRIP_CHANNEL));
                    SRIP_DETECT_HEADER sDetectHeader;
                    sDetectHeader.uDetectType = SRIP_CHANNEL_STATUS;
                    printf("sDetectHeader.uDetectType=%x\r\n",sDetectHeader.uDetectType);
                    response.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
                    g_skpChannelCenter.AddResult(response);


                    {
                        //ptg相机需要重连
                        if(m_pAbstractCamera)
                        m_pAbstractCamera->ReOpen();
                    }
                }
            }

            usleep(1000*1);
        }

    return;
}

//重启通道
bool CSkpChannelEntity::Restart(SRIP_CHANNEL& sChannel)
{
cerr << "Restart" << endl;
    m_uTimer = 0;

    if(m_bOpen)
    {
cerr << "Restart 1" << endl;
        //停止处理
        EndDetect();
cerr << "Restart 2" << endl;
        //释放空间
        Close();
    }
cerr << "Restart 3" << endl;
    /////////change
    int nCameraType = sChannel.nCameraType;
    SetCameraType(nCameraType);
    int eVideoFmt = sChannel.eVideoFmt;
    SetChannelFormat((VEDIO_FORMAT)eVideoFmt);
    std::string strFileName(sChannel.chFileName);
cerr << "strFileName = " << strFileName << endl;
    SetChannelFileName(strFileName);
    int nVideoIndex = sChannel.nVideoIndex;
    SetVideoID(nVideoIndex);
    int nCameraId = sChannel.nCameraId;
    SetCameraID(nCameraId);

	std::string strDeviceId(sChannel.chDeviceID);
	SetDeviceID(strDeviceId);

    int nYUVFormat = sChannel.nYUVFormat;
    SetYUVFormat(nYUVFormat);
    UINT32 uVideoBeginTime = sChannel.uVideoBeginTime;
    SetVideoBeginTime(uVideoBeginTime);
    UINT32 uVideoEndTime = sChannel.uVideoEndTime;
    SetVideoEndTime(uVideoEndTime);
	int nWorkMode = sChannel.nWorkMode;
	SetChannelWorkMode(nWorkMode);

	SetChannelDetectKind(sChannel.uDetectKind);
    /////////
cerr << "Restart 4" << endl;

    printf("I'm restart!!!!!!!!!!!\r\n");
    printf("strFileName=%s,m_sChannel.chFileName=%s\r\n",strFileName.c_str(),m_sChannel.chFileName);

    if(m_nStatus != PAUSE_STATUS )
    {
        //开始处理
        if(Open())
        {
            //开始处理
            BeginDetect();

            LogNormal("重启通道成功!\r\n");
            //返回成功
            return true;
        }
        else
        {
            LogError("重启通道失败!\r\n");
        }
    }
    return false;
}

//设置视频参数
bool CSkpChannelEntity::SetChannelParams(SRIP_CHANNEL_ATTR& sAttr)
{

//对应模块帧数据处理
    switch(m_Video_Format)
    {
    case VEDIO_PAL:
        return m_v4lDriver.SetVideoParams(sAttr);
        break;
    case VEDIO_AVI:
    case VEDIO_H264:
    case VEDIO_H264_FILE:
    case VEDIO_YUV:
    case VEDIO_YUV_FILE:
    case VEDIO_MJPEG:
    case VEDIO_PICLIB:
        break;
    default:

        break;
    }
    return true;
}


//处理普通采集图像
bool CSkpChannelEntity::DealNormalPic(bool bDealPic,int nVideoFrameRate,bool bDealVideo)
{

    if(bDealPic)
    {
        //判断是否需要送检测
        if( ((m_sChannel.uDetectKind&DETECT_FLUX)==DETECT_FLUX) ||
            ((m_sChannel.uDetectKind&DETECT_BEHAVIOR)==DETECT_BEHAVIOR) ||
            ((m_sChannel.uDetectKind&DETECT_CHARACTER)==DETECT_CHARACTER)||
			((m_sChannel.uDetectKind&DETECT_FACE)==DETECT_FACE))
        {
            //送事件检测程序
            DealDetectPic(nVideoFrameRate);
        }
    }

    //判断是否需要送客户端或送录像
    if( (m_sChannel.bEventCapture > 0)|| (m_bConnect&&bDealPic) || (m_sChannel.eCapType != CAPTURE_NO)||(g_nCheckShake == 1)||(g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))
    {
        //printf("=====m_uWidth=%d====m_sDetectHeader.uWidth=%d=,m_uHeight=%d=m_sDetectHeader.uHeight=%d==\n", m_uWidth, m_sDetectHeader.uWidth, m_uHeight, m_sDetectHeader.uHeight);


        //缩放图象
        if((m_uWidth!=m_sDetectHeader.uWidth)||(m_uHeight!=m_sDetectHeader.uHeight))
        {
            ////////////////////////缩放
            //RGB视频流resize
            ResizeBigImagetoSmall(m_chImageData,m_smImageData);
        }
        else
        {
            memcpy(m_smImageData,m_chImageData,m_uWidth*m_uHeight*3);
        }


        if(g_nCheckShake == 1)
        {
            CheckShakedFrame(m_sDetectHeader.uTimestamp);
        }

        int srcstep = 0;
        //printf("=====m_sChannel.bEventCapture=%d,m_sChannel.eCapType=%d\n",m_sChannel.bEventCapture,m_sChannel.eCapType);
        if((m_sChannel.bEventCapture > 0) || (m_sChannel.eCapType != CAPTURE_NO)||(g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))//需要进行事件录像
        {
            //printf("===-1-1===1111111 m_JpgToAvi.Encode\n");
            if((m_sChannel.bEventCapture > 0) || (m_sChannel.eCapType != CAPTURE_NO))
            {
                //printf("===00000===1111111 m_JpgToAvi.Encode\n");
                if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
                {
                   // printf("======1111111 m_JpgToAvi.Encode\n");
                    m_JpgToAvi.Encode(m_smImageData,m_sDetectHeader.uWidth,m_sDetectHeader.uHeight,&srcstep,m_JpgImageData);
                }
            }

            //将jpg编码后的图片进行录象
            if(m_sChannel.eCapType != CAPTURE_NO ||(g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))
            {
                //处理定时或全天录像功能
                DealTimeCapture(srcstep);
            }

            if(m_sChannel.bEventCapture > 0)
            {
                if(m_sChannel.eCapType != CAPTURE_FULL)
                {
                    std::string video_pic;
                    SRIP_DETECT_HEADER sDetectHeader = m_sDetectHeader;
                    sDetectHeader.dFrameRate = nVideoFrameRate;
                    //printf("====nVideoFrameRate=%d\n",nVideoFrameRate);
                    //添加到事件录象缓冲区
                    if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
                    {
                        video_pic.append((char*)&sDetectHeader,sizeof(sDetectHeader));
                        video_pic.append((char*)m_JpgImageData,srcstep);
                        if(m_sChannel.bEventCapture == 1)
                        {
                            #ifndef NOEVENT
                            m_skpRoadDetect.AddVideoFrame(video_pic);
                            #endif

                            #ifndef NOOBJECTDETECT
                            m_ObjectBehaviorDetect.AddVideoFrame(video_pic);
                            #endif
                        }
                        else if(m_sChannel.bEventCapture == 2)
                        {
                            #ifndef NOPLATE
                            m_RoadCarnumDetect.AddVideoFrame(video_pic);
                            #endif
                        }
						else  if(m_sChannel.bEventCapture == 3)
						{
							 #ifndef NOEVENT
                            m_skpRoadDetect.AddVideoFrame(video_pic);
                            #endif

                            #ifndef NOOBJECTDETECT
                            m_ObjectBehaviorDetect.AddVideoFrame(video_pic);
                            #endif

							#ifndef NOPLATE
                            m_RoadCarnumDetect.AddVideoFrame(video_pic);
                            #endif
						}
                    }
                    else if(g_nEncodeFormat == 2 || g_nEncodeFormat == 1)//对于g_nEncodeFormat=1的情形直接从全天录象中获取
                    {
                        if(bDealVideo)
                        {
                            ///////////h264编码
                            if(m_uWidth > 1000)
                            {
                                ResizeBigImagetoSmall(m_chImageData,m_captureImageData,true);
                                sDetectHeader.uWidth = g_nVideoWidth;
                                sDetectHeader.uHeight = g_nVideoHeight;
                                video_pic.append((char*)&sDetectHeader,sizeof(sDetectHeader));
                                video_pic.append((char*)m_captureImageData,g_nVideoWidth*g_nVideoHeight*3);
                            }
                            else
                            {
                                sDetectHeader.uWidth = m_uWidth;
                                sDetectHeader.uHeight = m_uHeight;
                                video_pic.append((char*)&sDetectHeader,sizeof(sDetectHeader));
                                video_pic.append((char*)m_chImageData,m_uWidth*m_uHeight*3);
                            }

                            if(m_sChannel.bEventCapture == 1)
                            {
                                #ifndef NOEVENT
                                m_skpRoadDetect.AddVideoFrame(video_pic);
                                #endif

                                #ifndef NOOBJECTDETECT
                                m_ObjectBehaviorDetect.AddVideoFrame(video_pic);
                                #endif
                            }
                            else if(m_sChannel.bEventCapture == 2)
                            {
                                #ifndef NOPLATE
                                m_RoadCarnumDetect.AddVideoFrame(video_pic);
                                #endif
                            }
							else  if(m_sChannel.bEventCapture == 3)
							{
								#ifndef NOEVENT
                                m_skpRoadDetect.AddVideoFrame(video_pic);
                                #endif

                                #ifndef NOOBJECTDETECT
                                m_ObjectBehaviorDetect.AddVideoFrame(video_pic);
                                #endif

								#ifndef NOPLATE
                                m_RoadCarnumDetect.AddVideoFrame(video_pic);
                                #endif
							}
                        }
                    }
                }
            }
        }

        if(m_bConnect&&bDealPic&&(!(g_VideoFormatInfo.nSendH264 == 1)))//需要发送视频
		{
            ///////////////////////////////////////
            std::string pic;
            //普通图片,未经过检测
            m_sDetectHeader.uDetectType = SRIP_NORMAL_PIC;

            if( m_imageRegion.nImageRegionType == WHOLE_IMAGE)//全景图像
            {
				if(g_VideoFormatInfo.nSendH264 == 0)
                {

                    if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
                    {
                        if( (m_sChannel.bEventCapture == 0)&&(m_sChannel.eCapType == CAPTURE_NO))
                        {
                            //printf("======22222222222 m_JpgToAvi.Encode\n");
							//是否传送jpg图象给客户端
							srcstep = 0;
							m_JpgToAvi.Encode(m_smImageData,m_sDetectHeader.uWidth,m_sDetectHeader.uHeight,&srcstep,m_JpgImageData);
                        }
                    }
                    else if(g_nEncodeFormat == 1 || g_nEncodeFormat == 2)
                    {

                        m_JpgToAvi.Encode(m_smImageData,m_sDetectHeader.uWidth,m_sDetectHeader.uHeight,&srcstep,m_JpgImageData);
                    }



					////////////////////////////////////////////////////
					/*static int nIndex = 0;
					char chpath[256] ={0};
					// 判断目录是否存在,不存在则建立图片目录
					if(access("./jpgpic",0) != 0) //目录不存在
					{
						mkdir("./jpgpic",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
					}

					sprintf(chpath,"./jpgpic/%04d.jpg",nIndex);
					FILE* fp = fopen(chpath,"wb");
					fwrite(m_JpgImageData,1,srcstep,fp);
					fclose(fp);
					nIndex++;*/
					///////////////////////////////////////////////////

                    //图片数据头
                    pic.append((char*)&m_sDetectHeader,sizeof(m_sDetectHeader));
                    //图片数据
                    pic.append((char*)m_JpgImageData,srcstep);
                    //添加到处理列表
                    g_skpChannelCenter.AddResult(pic);
                }
            }
            else//对焦区域等
            {
                //图片数据头
                SRIP_DETECT_HEADER sDetectHeader = m_sDetectHeader;
                sDetectHeader.uImageRegionType = m_imageRegion.nImageRegionType;
                srcstep = m_uWidth*3 + IJL_DIB_PAD_BYTES(m_uWidth,3);

                float floatX = m_uWidth/m_sDetectHeader.uWidth;
                float floatY = m_uHeight/m_sDetectHeader.uHeight;

                int nOffsetX = 0;
                int nOffsetY = 0;

                if(m_imageRegion.nImageRegionType == FOCUS_REGION_IMAGE)
                {
                    {
                        nOffsetX = m_imageRegion.x*floatX*3;
                        nOffsetY = srcstep*m_imageRegion.y*floatY;
                        sDetectHeader.uWidth = m_imageRegion.width*floatX;
                        sDetectHeader.uHeight = m_imageRegion.height*floatY;
                    }

                }
                else if(m_imageRegion.nImageRegionType == VIOLATION_REGION_IMAGE)
                {

                    {
                        nOffsetX = m_cropOrd.uViolationX*floatX*3;
                        nOffsetY = srcstep*m_cropOrd.uViolationY*floatY;
                        sDetectHeader.uWidth = m_cropOrd.uViolationWidth*floatX;
                        sDetectHeader.uHeight = m_cropOrd.uViolationHeight*floatY;
                    }
                }
                else if(m_imageRegion.nImageRegionType == EVENT_REGION_IMAGE)
                {
                    nOffsetX = m_cropOrd.uEventX*floatX*3;
                    nOffsetY = srcstep*m_cropOrd.uEventY*floatY;
                    sDetectHeader.uWidth = m_cropOrd.uEventWidth*floatX;
                    sDetectHeader.uHeight = m_cropOrd.uEventHeight*floatY;
                }
                else if(m_imageRegion.nImageRegionType == TRAFFIC_SIGNAL_REGION_IMAGE)
                {
                    nOffsetX = m_cropOrd.uTrafficSignalX*floatX*3;
                    nOffsetY = srcstep*m_cropOrd.uTrafficSignalY*floatY;
                    sDetectHeader.uWidth = m_cropOrd.uTrafficSignalWidth*floatX;
                    sDetectHeader.uHeight = m_cropOrd.uTrafficSignalHeight*floatY;
                }

                //rgb->jpg
                m_JpgToAvi.Encode(m_chImageData+nOffsetX+nOffsetY,sDetectHeader.uWidth,sDetectHeader.uHeight,&srcstep,m_JpgImageData);

                //printf("m_uWidth=%d,m_uHeight=%d,m_imageRegion.x=%d,m_imageRegion.y =%d,m_imageRegion.width=%d,m_imageRegion.height=%d,bRet=%d,srcstep=%d\n",m_uWidth,m_uHeight,m_imageRegion.x,m_imageRegion.y,m_imageRegion.width,m_imageRegion.height,bRet,srcstep);

                //图片数据头
                pic.append((char*)&sDetectHeader,sizeof(sDetectHeader));
                //图片数据
                pic.append((char*)m_JpgImageData,srcstep);

                //添加到处理列表
                g_skpChannelCenter.AddResult(pic);
                //printf("==VIOLATION_REGION_IMAGE===m_uWidth=%d==m_uHeight=%d\r\n",m_uWidth,m_uHeight);
            }
        }
    }
    return true;
}

//处理普通采集JPEG图像
bool CSkpChannelEntity::DealNormalMJpgPic(bool bDealPic, int nVideoFrameRate, bool bDealVideo)
{
    #ifdef _DSP_DEBUG
    printf("==============CSkpChannelEntity::DealNormalMJpgPic=====\n");
    printf("====bDealPic=%d====nVideoFrameRate=%d===bDealVideo=%d====\n", bDealPic, nVideoFrameRate, bDealVideo);
    #endif

    if(bDealPic)
    {
        //判断是否需要送检测
        if( ((m_sChannel.uDetectKind&DETECT_FLUX)==DETECT_FLUX) ||
            ((m_sChannel.uDetectKind&DETECT_BEHAVIOR)==DETECT_BEHAVIOR) ||
            ((m_sChannel.uDetectKind&DETECT_CHARACTER)==DETECT_CHARACTER) )
        {
            //送事件检测程序
            DealDetectPic(nVideoFrameRate);
        }
    }

#ifdef _DSP_DEBUG
    printf("==m_sChannel.bEventCapture=%d=m_bConnect=%d=bDealPic=%d=m_sChannel.eCapType=%d=g_nCheckShake=%d=g_nSendRTSP=%d==\n",\
           m_sChannel.bEventCapture, m_bConnect, bDealPic, m_sChannel.eCapType, g_nCheckShake, g_nSendRTSP);
#endif
    if(!m_bConnect)
    {
        m_bConnect = true;

        //m_sDetectHeader.uWidth *= 4;
        //m_sDetectHeader.uHeight *= 4;
    }

    //判断是否需要送客户端或送录像
    if(g_nDetectMode != 2)
    {
        if( (m_sChannel.bEventCapture > 0)|| (m_bConnect&&bDealPic) || (m_sChannel.eCapType != CAPTURE_NO)||(g_nCheckShake == 1)||(g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))
        {
#ifdef _DSP_DEBUG
            printf("==DealNormalMJpgPic==m_uWidth=%d====m_sDetectHeader.uWidth=%d=,m_uHeight=%d=m_sDetectHeader.uHeight=%d==\n",\
                    m_uWidth, m_sDetectHeader.uWidth, m_uHeight, m_sDetectHeader.uHeight);
#endif
            //缩放图象
            if((m_uWidth!=m_sDetectHeader.uWidth)||(m_uHeight!=m_sDetectHeader.uHeight))
            {
                ////////////////////////缩放
                //RGB视频流resize
                ResizeBigImagetoSmall(m_chImageData,m_smImageData);
            }
            else
            {
                memcpy(m_smImageData,m_chImageData,m_uWidth*m_uHeight*3);
            }


            if(g_nCheckShake == 1)
            {
                CheckShakedFrame(m_sDetectHeader.uTimestamp);
            }

#ifdef _DSP_DEBUG
            printf("===========DealNormalMJpgPic=====333333==========\n");
#endif

            int srcstep = 0;
            //printf("=====m_sChannel.bEventCapture=%d,m_sChannel.eCapType=%d\n",m_sChannel.bEventCapture,m_sChannel.eCapType);
            if((m_sChannel.bEventCapture > 0) || (m_sChannel.eCapType != CAPTURE_NO) || (g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))//需要进行事件录像
            {
                #ifdef _DSP_DEBUG
                printf("===========DealNormalMJpgPic=====4444==========\n");
                #endif
                yuv_video_buf* header = (yuv_video_buf*)m_JpgImageData;
                //不需要进行JPG编码
                srcstep = header->size;

                //将jpg编码后的图片进行录象
                if(m_sChannel.eCapType != CAPTURE_NO ||(g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))
                {
                    //处理定时或全天录像功能
                  //  DealTimeCapture(srcstep);
                }

                if(m_sChannel.bEventCapture > 0)
                {
                    if(m_sChannel.eCapType != CAPTURE_FULL)
                    {
                        std::string video_pic;
                        SRIP_DETECT_HEADER sDetectHeader = m_sDetectHeader;
                        sDetectHeader.dFrameRate = nVideoFrameRate;
                        //printf("====nVideoFrameRate=%d\n",nVideoFrameRate);
                        //添加到事件录象缓冲区
                        if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
                        {
                            srcstep = m_uWidth*3 + IJL_DIB_PAD_BYTES(m_uWidth,3);
                            int nPicSize = srcstep * m_uHeight;
                            video_pic.append((char*)&sDetectHeader,sizeof(sDetectHeader));
                            video_pic.append((char*)m_chImageData, nPicSize);

                            if(m_sChannel.bEventCapture == 1 || m_sChannel.bEventCapture == 3)
                            {
                                #ifndef NOEVENT
                                //m_skpRoadDetect.AddVideoFrame(video_pic);
                                #endif

                                #ifndef NOOBJECTDETECT
                                m_ObjectBehaviorDetect.AddVideoFrame(video_pic);
                                #endif
                            }
                            else if(m_sChannel.bEventCapture == 2 || m_sChannel.bEventCapture == 3)
                            {
                                #ifndef NOPLATE
                                m_RoadCarnumDetect.AddVideoFrame(video_pic);
                                #endif
                            }
                        }
                        else if(g_nEncodeFormat == 2 || g_nEncodeFormat == 1)//对于g_nEncodeFormat=1的情形直接从全天录象中获取
                        {
                            if(bDealVideo)
                            {
                                ///////////h264编码
                                if(m_uWidth > 1000)
                                {
                                    ResizeBigImagetoSmall(m_chImageData,m_captureImageData,true);
                                    sDetectHeader.uWidth = g_nVideoWidth;
                                    sDetectHeader.uHeight = g_nVideoHeight;
                                    video_pic.append((char*)&sDetectHeader,sizeof(sDetectHeader));
                                    video_pic.append((char*)m_captureImageData,g_nVideoWidth*g_nVideoHeight*3);
                                }
                                else
                                {
                                    sDetectHeader.uWidth = m_uWidth;
                                    sDetectHeader.uHeight = m_uHeight;
                                    video_pic.append((char*)&sDetectHeader,sizeof(sDetectHeader));
                                    video_pic.append((char*)m_chImageData,m_uWidth*m_uHeight*3);
                                }

                                if(m_sChannel.bEventCapture == 1 || m_sChannel.bEventCapture == 3)
                                {
                                    #ifndef NOEVENT
                                    //m_skpRoadDetect.AddVideoFrame(video_pic);
                                    #endif

                                    #ifndef NOOBJECTDETECT
                                    m_ObjectBehaviorDetect.AddVideoFrame(video_pic);
                                    #endif
                                }
                                else if(m_sChannel.bEventCapture == 2 || m_sChannel.bEventCapture == 3)
                                {
                                    #ifndef NOPLATE
                                    m_RoadCarnumDetect.AddVideoFrame(video_pic);
                                    #endif
                                }

                            }

                        }
                    }
                }
            }
        }//End of //判断是否需要送客户端或送录像
    }//End of if

     if( m_imageRegion.nImageRegionType == WHOLE_IMAGE)//全景图像
    {
        #ifdef _DSP_DEBUG
        printf("===========DealNormalMJpgPic=====55555==========\n");
        #endif

        /*
        //存图
        sprintf(jpg_name, "./jpg/%d_nSeq.jpg",header.nSeq);
        fpOut = fopen(jpg_name,"wb");
        //fwrite(pBuffer + sizeof(yuv_video_buf) + sizeof(Image_header_dsp), 1, header.size - sizeof(Image_header_dsp), fpOut);
        fwrite(m_JpgImageData, 1, header.size , fpOut);
        fclose(fpOut);
        */  
        {
			if(g_nDetectMode != 2)
			{
				if(m_JpgImageData == NULL)
				{
					return false;
				}
				yuv_video_buf* header = (yuv_video_buf*)m_JpgImageData;

				m_sDetectHeader.uDetectType = SRIP_NORMAL_PIC;
				m_sDetectHeader.uWidth = header->width;
				m_sDetectHeader.uHeight = header->height;

				std::string  pic;

				//图片数据头
				pic.append((char*)&m_sDetectHeader, sizeof(SRIP_DETECT_HEADER));
				//图片数据
				pic.append((char*)(m_JpgImageData + sizeof(yuv_video_buf)), header->size);

#ifdef _DSP_DEBUG
				printf("==========pic.size=%d===\n", pic.size());
#endif
				//添加到处理列表
				g_skpChannelCenter.AddResult(pic);
			}
			else
			{
				if(m_JpgImageData2 == NULL)
				{
					//LogTrace("LogDsp.log", "m_JpgImageData2 is empty!\n");
					return false;
				}

				std::string  pic;

				yuv_video_buf *header = (yuv_video_buf*)m_JpgImageData2;

				LogTrace("OutNormal.log", "==header->nSeq=%d==header->size=%d \n", \
					header->nSeq, header->size);


				Image_header_dsp *pDspHeader = (Image_header_dsp*)(m_JpgImageData2+sizeof(yuv_video_buf));

				m_sDetectHeader.uDetectType = SRIP_NORMAL_PIC;
				m_sDetectHeader.uWidth = header->width;
				m_sDetectHeader.uHeight = header->height;

				//LogTrace("LogDsp.log", "==header->nSeq=%d=m_sDetectHeader=uWidth=%d, uHeight=%d==\n", \
					header->nSeq, m_sDetectHeader.uWidth, m_sDetectHeader.uHeight);
				m_sDetectHeader.dFrameRate = 2;

				m_sDetectHeader.uSeq = header->nSeq;//帧号
				m_sDetectHeader.uTimestamp = (header->ts/1000)/1000; //时间戳
				m_sDetectHeader.uTime64 = header->ts;

				printf("=#####1111=23==pic.size()=%d=header->size=%d=\n", (int)pic.size(), header->size);

				if(header->size < m_sDetectHeader.uWidth * m_sDetectHeader.uHeight)
				{
					pic.reserve(m_sDetectHeader.uWidth * m_sDetectHeader.uHeight);
					printf("==#########==pic.capacity = %d===\n", (int)pic.capacity() );

					//图片数据头
					pic.append((char*)&m_sDetectHeader, sizeof(m_sDetectHeader));
					//图片数据
					pic.append((char*)( m_JpgImageData2 + sizeof(yuv_video_buf) + sizeof(Image_header_dsp) ),header->size - sizeof(Image_header_dsp) );

					printf("=#####2222=23==pic.size()=%d===\n", (int)pic.size());
					//添加到处理列表
					g_skpChannelCenter.AddResult(pic);
				}
			}

            return true;
        }    

        if(m_JpgImageData == NULL)
        {
            return false;
        }
        yuv_video_buf* header = (yuv_video_buf*)m_JpgImageData;

        m_sDetectHeader.uDetectType = SRIP_NORMAL_PIC;
        m_sDetectHeader.uWidth = header->width;
        m_sDetectHeader.uHeight = header->height;

        std::string  pic;

        //图片数据头
        pic.append((char*)&m_sDetectHeader, sizeof(SRIP_DETECT_HEADER));
        //图片数据
        pic.append((char*)(m_JpgImageData + sizeof(yuv_video_buf)), header->size);

        printf("==========pic.size=%d===\n", (int)pic.size());

        //添加到处理列表
        g_skpChannelCenter.AddResult(pic);
    }
    else//对焦区域等
    {
        if(m_JpgImageData2 == NULL)
        {
            printf("m_JpgImageData2 is empty!\n");
            return false;
        }

        yuv_video_buf* header = (yuv_video_buf*)m_JpgImageData2;

        unsigned int uWidth;
        unsigned int uHeight;

        {
            uWidth = m_uWidth;
            uHeight = m_uHeight;
        }
        //m_sDetectHeader.uWidth = header->width;
        //m_sDetectHeader.uHeight = header->height;
        if(header->width != uWidth || header->height != uHeight)
        {
            printf("m_JpgImageData2 not set!\n");
            return false;
        }

        //图片数据头
        SRIP_DETECT_HEADER sDetectHeader = m_sDetectHeader;
        sDetectHeader.uWidth = m_uWidth;
        sDetectHeader.uHeight = m_uHeight;

        sDetectHeader.uImageRegionType = m_imageRegion.nImageRegionType;
        int srcstep = uWidth*3 + IJL_DIB_PAD_BYTES(uWidth,3);

        float floatX =  (double)uWidth / sDetectHeader.uWidth;
        float floatY =  (double)uHeight / sDetectHeader.uHeight;
		if(m_sChannel.nCameraType == DSP_ROSEEK_500_335)
		{
			floatY = 6;
		}

        printf("===sDetectHeader.uWidth=%d, sDetectHeader.uHeight =%d, m_uWidth = %d, m_uHeight =%d==\n",sDetectHeader.uWidth, sDetectHeader.uHeight, m_uWidth, m_uHeight);

        int nOffsetX = 0;
        int nOffsetY = 0;

        if(m_imageRegion.nImageRegionType == FOCUS_REGION_IMAGE)
        {
            nOffsetX = m_imageRegion.x*floatX*3;
            nOffsetY = srcstep*m_imageRegion.y*floatY;
            sDetectHeader.uWidth = m_imageRegion.width*floatX;
            sDetectHeader.uHeight = m_imageRegion.height*floatY;

			printf("===m_imageRegion.x=%d, m_imageRegion.y =%d, m_imageRegion.width = %d, m_imageRegion.height =%d==floatX=%f,floatY=%f,nOffsetX=%d,nOffsetY=%d,sDetectHeader.uWidth=%d,sDetectHeader.uHeight=%d\n",m_imageRegion.x,m_imageRegion.y,m_imageRegion.width,m_imageRegion.height,floatX,floatY,nOffsetX,nOffsetY,sDetectHeader.uWidth,sDetectHeader.uHeight);

        }
        else if(m_imageRegion.nImageRegionType == VIOLATION_REGION_IMAGE)
        {
            nOffsetX = m_cropOrd.uViolationX*floatX*3;
            nOffsetY = srcstep*m_cropOrd.uViolationY*floatY;
            sDetectHeader.uWidth = m_cropOrd.uViolationWidth*floatX;
            sDetectHeader.uHeight = m_cropOrd.uViolationHeight*floatY;
        }
        else if(m_imageRegion.nImageRegionType == EVENT_REGION_IMAGE)
        {
            nOffsetX = m_cropOrd.uEventX*floatX*3;
            nOffsetY = srcstep*m_cropOrd.uEventY*floatY;
            sDetectHeader.uWidth = m_cropOrd.uEventWidth*floatX;
            sDetectHeader.uHeight = m_cropOrd.uEventHeight*floatY;
        }
        else if(m_imageRegion.nImageRegionType == TRAFFIC_SIGNAL_REGION_IMAGE)
        {
            nOffsetX = m_cropOrd.uTrafficSignalX*floatX*3;
            nOffsetY = srcstep*m_cropOrd.uTrafficSignalY*floatY;
            sDetectHeader.uWidth = m_cropOrd.uTrafficSignalWidth*floatX;
            sDetectHeader.uHeight = m_cropOrd.uTrafficSignalHeight*floatY;
        }

        printf("m_uWidth=%d,m_uHeight=%d,m_imageRegion.x=%d,m_imageRegion.y =%d,m_imageRegion.width=%d,m_imageRegion.height=%d,srcstep=%d\n",\
               m_uWidth,m_uHeight,m_imageRegion.x,m_imageRegion.y,m_imageRegion.width,m_imageRegion.height,srcstep);

		if(g_nDetectMode == 2)
         {
            //rgb->jpg
            CxImage image;
            //先解码
            image.Decode(m_JpgImageData2 + sizeof(yuv_video_buf) + sizeof(Image_header_dsp), header->size - sizeof(Image_header_dsp), 3); //解码-大图

            //BYTE *pBuf = new BYTE[uWidth * uHeight * 3];
            bool bRet = image.IppEncode(image.GetBits()+nOffsetX+nOffsetY,sDetectHeader.uWidth,sDetectHeader.uHeight,3,&srcstep,m_JpgImageData2);
            //bool bRet = image.IppEncode(image.GetBits()+nOffsetX+nOffsetY,sDetectHeader.uWidth,sDetectHeader.uHeight,3,&srcstep,pBuf);

            printf("m_uWidth=%d,m_uHeight=%d,m_imageRegion.x=%d,m_imageRegion.y =%d,m_imageRegion.width=%d,m_imageRegion.height=%d,srcstep=%d\n",m_uWidth,m_uHeight,m_imageRegion.x,m_imageRegion.y,m_imageRegion.width,m_imageRegion.height,srcstep);

            std::string  pic;
            //图片数据头
            pic.append((char*)&sDetectHeader, sizeof(sDetectHeader));
            //图片数据
            pic.append((char*)(m_JpgImageData2),srcstep); //未带yuv_video_buf头
            //pic.append((char*)(pBuf),srcstep); //未带yuv_video_buf头

            //添加到处理列表
            g_skpChannelCenter.AddResult(pic);
         }
         else
         {
             //rgb->jpg
            CxImage image;
            //先解码
            image.Decode(m_JpgImageData2 + sizeof(yuv_video_buf), header->size, 3); //解码-大图

            //BYTE *pBuf = new BYTE[uWidth * uHeight * 3];
            bool bRet = image.IppEncode(image.GetBits()+nOffsetX+nOffsetY,sDetectHeader.uWidth,sDetectHeader.uHeight,3,&srcstep,m_JpgImageData2);
            //bool bRet = image.IppEncode(image.GetBits()+nOffsetX+nOffsetY,sDetectHeader.uWidth,sDetectHeader.uHeight,3,&srcstep,pBuf);

            printf("m_uWidth=%d,m_uHeight=%d,m_imageRegion.x=%d,m_imageRegion.y =%d,m_imageRegion.width=%d,m_imageRegion.height=%d,srcstep=%d\n",m_uWidth,m_uHeight,m_imageRegion.x,m_imageRegion.y,m_imageRegion.width,m_imageRegion.height,srcstep);

            std::string  pic;
            //图片数据头
            pic.append((char*)&sDetectHeader, sizeof(sDetectHeader));
            //图片数据
            pic.append((char*)(m_JpgImageData2),srcstep); //未带yuv_video_buf头
            //pic.append((char*)(pBuf),srcstep); //未带yuv_video_buf头

            //添加到处理列表
            g_skpChannelCenter.AddResult(pic);

         }
        //printf("==VIOLATION_REGION_IMAGE===m_uWidth=%d==m_uHeight=%d\r\n",m_uWidth,m_uHeight);
    } //End of 对焦区域等

    return true;
}

//处理事件检测采集图片
bool CSkpChannelEntity::DealDetectPic(int nVideoFrameRate)
{
     int lineStep = m_uWidth*3 + IJL_DIB_PAD_BYTES(m_uWidth,3);
    int nPicLen = lineStep * m_uHeight;
    //printf("nPicLen=%d,\r\n",nPicLen);
    //待检测的图片,轮换为对应的编码格式(BGR)
    //图片数据头
    //检测数据头
    SRIP_DETECT_HEADER sDetectHeader = m_sDetectHeader;

    //原图大小
    sDetectHeader.uWidth = m_uWidth;
    sDetectHeader.uHeight = m_uHeight;
    sDetectHeader.dFrameRate = nVideoFrameRate;

    //保存采集帧
    #ifndef NOEVENT
    if((m_sChannel.uDetectKind&DETECT_FLUX)==DETECT_FLUX)
	{
		 std::string pic("");
		 //检测图片,支持传输算法需要的格式
		pic.append((char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		//图片数据
		pic.append((char*)m_chImageData,nPicLen);
		m_skpRoadDetect.AddFrame(pic);
	}
    #endif

	#ifndef NOFACEDETECT
    if((m_sChannel.uDetectKind&DETECT_FACE))
    {
		std::string picFace("");
		picFace.append((char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		picFace.append((char*)m_chImageData,nPicLen);
        m_FaceDetect.AddFrame(picFace);
    }
    #endif

    #ifndef NOFEATURESEARCH
    if(((m_sChannel.uDetectKind&DETECT_CHARACTER))&&((m_sChannel.uDetectKind&DETECT_OBJECT)!=DETECT_OBJECT))
    {
		std::string picFeature("");
		FeatureSearchHeader header;
		header.sMode = 0;
		picFeature.append((char*)&header,sizeof(FeatureSearchHeader));
		picFeature.append((char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		picFeature.append((char*)m_chImageData,nPicLen);

		if(m_nVideoType == 1&& g_nHistoryPlayMode == 2)//历史
		{
			if(m_sChannel.uVideoBeginTime != m_sChannel.uVideoEndTime)
			{
				m_FeatureSearchDetect.AddFrame(picFeature);
			}
		}
		else//实时
		{
			m_FeatureSearchDetect.AddFrame(picFeature);
		}
    }
    #endif

	#ifndef NOOBJECTDETECT
    if((m_sChannel.uDetectKind&DETECT_BEHAVIOR)==DETECT_BEHAVIOR)
	{
		std::string picObject("");
		picObject.append((char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
		picObject.append((char*)m_chImageData,nPicLen);
		m_ObjectBehaviorDetect.AddFrame(picObject);
	}
    #endif

	//printf("CSkpChannelEntity::DealDetectPic\n");
	
    return true;
}

//定时录像处理
bool CSkpChannelEntity::DealTimeCapture(int srcstep)
{
	//LogTrace("Capture.log", "===DealTimeCapture==g_nSendRTSP=%d=g_VideoFormatInfo.nSendH264=%d", \
		g_nSendRTSP, g_VideoFormatInfo.nSendH264);

    //取时分,精度控制在分以内
    if(m_sChannel.eCapType == CAPTURE_TIME)
    {
        if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
        {
            UINT32 uTime = GetHmTime();
            //开始录像
            if( (uTime>= m_sChannel.uCapBeginTime)&&(uTime <= m_sChannel.uCapEndTime))
            {

                if(!m_JpgToAvi.IsEncoding())
                {
                    //g_skpDB.check_disk();

                    m_uBeginTime = GetTimeStamp();
                    m_JpgToAvi.OpenFile(g_FileManage.GetVideoPath().c_str());
					m_uEndTime = m_uBeginTime + m_sChannel.uCapEndTime;
                    g_skpDB.SaveVideo(m_sChannel.uId,m_uBeginTime,m_uEndTime,m_JpgToAvi.GetEncodingFileName(),1);
                    //输出
                    LogNormal("开始通道录像!\r\n");
                    return true;
                }
            }

            //停止录像
            else if( uTime >= m_sChannel.uCapEndTime)
            {
                if(m_JpgToAvi.IsEncoding())
                {
                    m_uEndTime = GetTimeStamp();
                    //记录录像结束时间
                    m_JpgToAvi.CloseFile();
                    //输出
                    LogNormal("停止通道录像!\r\n");
                    return true;
                }
            }
        }
    }
    else if(m_sChannel.eCapType == CAPTURE_FULL)
    {
        if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
        {
            if(!m_JpgToAvi.IsEncoding())
            {
                //g_skpDB.check_disk();
                //记录开始时间
                m_uBeginTime = GetTimeStamp();
                m_JpgToAvi.OpenFile(g_FileManage.GetVideoPath().c_str());
				m_uEndTime = m_uBeginTime + g_VideoFormatInfo.nTimeLength*60;
                g_skpDB.SaveVideo(m_sChannel.uId,m_uBeginTime,m_uEndTime,m_JpgToAvi.GetEncodingFileName(),0);
            }
            else
            {
                UINT32 uTime =  GetTimeStamp();
                if(uTime >= m_uBeginTime + g_VideoFormatInfo.nTimeLength*60)//
                {
                        m_uEndTime = GetTimeStamp();
                        //添加车牌记录和图片
                        if(g_nAviHeaderEx == 1)
                        m_JpgToAvi.AddPlatePics(m_sChannel.uId,m_uBeginTime,m_uEndTime);
                        //记录录像结束时间
                        m_JpgToAvi.CloseFile();
                }
            }
        }
    }

    if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
    {
        if(m_sChannel.eCapType != CAPTURE_NO)
        {
            if(m_JpgToAvi.IsEncoding())
            {
                m_JpgToAvi.AddOneFrame(m_JpgImageData,srcstep,&m_sDetectHeader);
            }
        }
    }
    else if(g_nEncodeFormat == 1||g_nEncodeFormat == 2)
    {
        ///////////h264编码
		if(m_sChannel.eCapType == CAPTURE_FULL || (g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))
		{
			if(g_nDetectMode != 2)
			{
				m_H264Capture.AddFrame(m_chImageData, m_sDetectHeader);
			}
		}
    }
    return true;
}


//重启录象
bool CSkpChannelEntity::RestartCapture(CAPTURE_TYPE cType)
{
	if(m_nStatus == PAUSE_STATUS)//对于暂停状态的通道直接返回
	{
		return true;
	}

    CAPTURE_TYPE  eCapType = m_sChannel.eCapType;

    if(g_nEncodeFormat == 0 || g_nEncodeFormat == 3)
    {
        //如果在录象则停止录象
        if(m_JpgToAvi.IsEncoding())
        {
            m_uEndTime = GetTimeStamp();
            //记录录像结束时间
            m_JpgToAvi.CloseFile();
            g_skpDB.SaveVideo(m_sChannel.uId,m_uBeginTime,m_uEndTime,m_JpgToAvi.GetEncodingFileName(),0);
        }
        SetChannelCaptureType(cType);
    }
    else
	{
		if(g_nEncodeFormat == 1 || g_nEncodeFormat == 2 || g_nEncodeFormat == 4)
		{
			if( (eCapType != CAPTURE_NO) || (g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))
			{
				m_H264Capture.UnInit();
			}

			SetChannelCaptureType(cType);
			InitRecordCapture();
			
			if(g_nDetectMode == 2)
			{
				m_TempCapture.Unit();
				InitTempCapture();
			}
		}
	}

    return true;
}

//重启检测
bool CSkpChannelEntity::RestartDetect(CHANNEL_DETECT_KIND dType)
{
    if(m_Video_Format == VEDIO_PICLIB)
    {
        return true;
    }

    if(m_nStatus == PAUSE_STATUS)//对于暂停状态的通道直接返回
	{
		return true;
	}

    //正在进行的检测类型
    CHANNEL_DETECT_KIND uDetectKind = m_sChannel.uDetectKind;

    printf("===============uDetectKind=%d,dType=%d\n",uDetectKind,dType);

    #ifndef NOEVENT
    if((uDetectKind&DETECT_FLUX)==DETECT_FLUX)//原先需要进行事件检测
    {
        if((dType&DETECT_FLUX)!= DETECT_FLUX)//现在不需要进行事件检测
        {
            printf("=====RestartDetect m_skpRoadDetect.UnInit\n");
            //释放检测算法模块
            m_skpRoadDetect.UnInit();
        }
    }
    #endif
    #ifndef NOPLATE
    if((uDetectKind&DETECT_CARNUM)==DETECT_CARNUM)//原先需要进行车牌检测
    {
        if((dType&DETECT_CARNUM)!= DETECT_CARNUM)//现在不需要进行车牌检测
        {
            printf("=====RestartDetect m_RoadCarnumDetect.EndCarnumDetect\n");
            //停止车牌检测
			m_RoadCarnumDetect.EndCarnumDetect();
        }
    }
    #endif

    #ifndef NOOBJECTDETECT
    if((uDetectKind&DETECT_BEHAVIOR)==DETECT_BEHAVIOR)//原先需要进行行为分析
    {
        if((dType&DETECT_BEHAVIOR)!= DETECT_BEHAVIOR)//现在不需要进行行为分析
        {
            printf("=====RestartDetect m_ObjectBehaviorDetect.UnInit\n");
            //释放检测算法模块
            m_ObjectBehaviorDetect.UnInit();
        }
    }
    #endif

	#ifndef NOFACEDETECT
    if((uDetectKind&DETECT_FACE)==DETECT_FACE)//原先需要进行人脸识别
    {
        if((dType&DETECT_FACE)!= DETECT_FACE)//现在不需要进行人脸识别
        {
            printf("=====RestartDetect m_FaceDetect.UnInit\n");
            //释放检测算法模块
            m_FaceDetect.UnInit();
        }
    }
    #endif

    #ifndef NOFEATURESEARCH
    if((uDetectKind&DETECT_CHARACTER)==DETECT_CHARACTER)//原先需要进行行为分析
    {
        if((dType&DETECT_CHARACTER)!= DETECT_CHARACTER)//现在不需要进行行为分析
        {
            printf("=====RestartDetect m_FeatureSearchDetect.UnInit\n");
            //释放检测算法模块
            m_FeatureSearchDetect.UnInit();
        }
    }
    #endif

    //设置是否采集红灯信号
    if((uDetectKind&DETECT_VTS)==DETECT_VTS)//原先需要采集红灯信号
    {
        if((dType&DETECT_VTS)!=DETECT_VTS)//现在不需要采集红灯信号
        {
            printf("=====RestartDetect r_SerialComm.EndRedSignalCapture\n");
            r_SerialComm.EndRedSignalCapture();
            if(m_pAbstractCamera)
            m_pAbstractCamera->SetDetectRedSignal(false);
        }
    }

    //设置是否采集线圈信号
    if((uDetectKind&DETECT_LOOP)==DETECT_LOOP)//原先需要采集线圈信号
    {
        if((dType&DETECT_LOOP)!=DETECT_LOOP)//现在不需要采集线圈信号
        {
            printf("=====RestartDetect d_SerialComm.EndLoopSignalCapture\n");
            d_SerialComm.EndLoopSignalCapture();

			if(0==m_sChannel.nWorkMode)
			{
				if(m_pAbstractCamera)
				m_pAbstractCamera->SetDetectLoopSignal(false);
			}
        }
    }

    //设置是否采集雷达信号
    if((uDetectKind&DETECT_RADAR)==DETECT_RADAR)//原先需要采集雷达信号
    {
        if((dType&DETECT_RADAR)!=DETECT_RADAR)//现在不需要采集雷达信号
        {
            printf("=====RestartDetect g_RadarSerial.EndRadarSignalCapture\n");
            g_RadarSerial.EndRadarSignalCapture();
            if(m_pAbstractCamera)
            m_pAbstractCamera->SetDetectRadarSignal(false);
        }
    }
    //设置通道检测类型
    SetChannelDetectKind(dType);
    #ifndef NOEVENT
    if((dType&DETECT_FLUX)==DETECT_FLUX)//现在需要进行事件检测
    {
        if((uDetectKind&DETECT_FLUX)!=DETECT_FLUX) //原先不需要进行事件检测
        {
            printf("=====RestartDetect m_skpRoadDetect.Init\n");
            //初始化检测类
            int nWidth = m_sDetectHeader.uWidth;
            int nHeight = m_sDetectHeader.uHeight;
            //初始化检测类
            if(!m_skpRoadDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
            //if(!m_skpRoadDetect.Init(GetChannelID(),m_uWidth*4,m_uHeight*4,nWidth,nHeight))
            {
                LogError("检测算法模块初始化失败，无法检测！\r\n");
                return false;
            }
        }
    }
    #endif
    #ifndef NOPLATE
    if((dType&DETECT_CARNUM)==DETECT_CARNUM)//现在需要进行车牌检测
    {
        if((uDetectKind&DETECT_CARNUM)!=DETECT_CARNUM)//原先不需要进行车牌检测
        {
            printf("=====RestartDetect m_RoadCarnumDetect.Init\n");
			if(!m_RoadCarnumDetect.Init(GetChannelID(),m_uWidth,m_uHeight,m_sDetectHeader.uWidth ,m_sDetectHeader.uHeight))
			{
				LogError("车牌检测模块启动失败，无法检测！\r\n");
				return false;
			}
        }
    }
    #endif

    #ifndef NOOBJECTDETECT
    if((dType&DETECT_BEHAVIOR)==DETECT_BEHAVIOR)//原先不需要进行行为分析
    {
        if((uDetectKind&DETECT_BEHAVIOR)!=DETECT_BEHAVIOR) //现在需要进行行为分析
        {
            printf("=====RestartDetect m_ObjectBehaviorDetect.Init\n");
            //初始化检测类
            int nWidth = m_sDetectHeader.uWidth;
            int nHeight = m_sDetectHeader.uHeight;
            //初始化检测类
            if(!m_ObjectBehaviorDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
            {
                LogError("检测算法模块初始化失败，无法检测！\r\n");
                return false;
            }
        }
    }
    #endif

	#ifndef NOFACEDETECT
	if((dType&DETECT_FACE)== DETECT_FACE)//原先需要进行人脸识别
    {
        if((uDetectKind&DETECT_FACE)!=DETECT_FACE)//现在不需要进行人脸识别
        {
            printf("=====RestartDetect m_FaceDetect.Init\n");
           //初始化检测类
            int nWidth = m_sDetectHeader.uWidth;
            int nHeight = m_sDetectHeader.uHeight;
            //初始化检测类
            if(!m_FaceDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
            {
                LogError("检测算法模块初始化失败，无法检测！\r\n");
                return false;
            }
        }
    }
    #endif


    #ifndef NOFEATURESEARCH
    if((dType&DETECT_CHARACTER)==DETECT_CHARACTER)//原先不需要进行特征提取
    {
        if((uDetectKind&DETECT_CHARACTER)!=DETECT_CHARACTER) //现在需要进行特征提取
        {
            printf("=====RestartDetect m_FeatureSearchDetect.Init\n");
            //初始化检测类
            int nWidth = m_sDetectHeader.uWidth;
            int nHeight = m_sDetectHeader.uHeight;
            //初始化检测类
            if(!m_FeatureSearchDetect.Init(GetChannelID(),m_uWidth,m_uHeight,nWidth,nHeight))
            {
                LogError("检测算法模块初始化失败，无法检测！\r\n");
                return false;
            }
        }
    }
    #endif

    //设置是否采集红灯信号
    if((dType&DETECT_VTS)==DETECT_VTS)//现在需要采集红灯信号
    {
        if((uDetectKind&DETECT_VTS)!=DETECT_VTS)//原先不需要采集红灯信号
        {
            printf("=====RestartDetect r_SerialComm.BeginRedSignalCapture\n");

			if(g_nDetectMode != 2)
			{
				r_SerialComm.BeginRedSignalCapture();
				if(m_pAbstractCamera)
				m_pAbstractCamera->SetDetectRedSignal(true);
			}
        }
    }

    //设置是否采集线圈信号
    if((dType&DETECT_LOOP)==DETECT_LOOP)//现在需要采集线圈信号
    {
        if((uDetectKind&DETECT_LOOP)!=DETECT_LOOP)//原先不需要采集线圈信号
        {
			if(g_nDetectMode != 2)
			{
				printf("=====RestartDetect d_SerialComm.BeginLoopSignalCapture\n");
				d_SerialComm.BeginLoopSignalCapture();

				if(0==m_sChannel.nWorkMode)
				{
					if(m_pAbstractCamera)
					m_pAbstractCamera->SetDetectLoopSignal(true);
				}
			}
        }
    }

    //设置是否采集雷达信号
    if((dType&DETECT_RADAR)==DETECT_RADAR)//现在需要采集雷达信号
    {
        if((uDetectKind&DETECT_RADAR)!=DETECT_RADAR)//原先不需要采集雷达信号
        {
            printf("=====RestartDetect g_RadarSerial.BeginRadarSignalCapture\n");
			
			if(g_RadarComSetting.nComUse == 7)
			{
				g_RadarSerial.SetRadarType(1);//S3
			}
			else if(g_RadarComSetting.nComUse == 13)
			{
				LogNormal("nComUse 13 SetRadarType 22!\n");
				g_RadarSerial.SetRadarType(2);//huichang
			}

            g_RadarSerial.BeginRadarSignalCapture();
            if(m_pAbstractCamera)
            m_pAbstractCamera->SetDetectRadarSignal(true);
        }
    }

    if(uDetectKind != dType)
    LogNormal("重启检测成功!\n");
    return true;
}

//设置通道检测类型
void CSkpChannelEntity::SetChannelDetectTime(CHANNEL_DETECT_TIME dType)
{
    if(dType != m_sChannel.uDetectTime)
    {
        #ifndef NOEVENT
        m_skpRoadDetect.SetChannelDetectTime(dType);
        #endif
        #ifndef NOOBJECTDETECT
        m_ObjectBehaviorDetect.SetChannelDetectTime(dType);
        #endif
		#ifndef NOFACEDETECT
        m_FaceDetect.SetChannelDetectTime(dType);
        #endif
        #ifndef NOPLATE
        m_RoadCarnumDetect.SetChannelDetectTime(dType);
        #endif
        m_sChannel.uDetectTime = dType;
    }
}

//重置配置
void CSkpChannelEntity::SetReloadConfig(bool bReloadParam)
{
    #ifndef NOEVENT
    m_skpRoadDetect.SetReloadConfig();
    #endif

    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetReloadConfig();
    #endif

	#ifndef NOFACEDETECT
    m_FaceDetect.SetReloadConfig();
    #endif

    #ifndef NOFEATURESEARCH
    m_FeatureSearchDetect.SetReloadConfig();
    #endif

    #ifndef NOPLATE
	m_RoadCarnumDetect.SetReloadROI();
    #endif

	m_bReloadConfig = true;
    //发送开启消息
    if( (g_nServerType == 1)&&(g_nHasCenterServer == 1))
    {
        CHANNEL_INFO_RECORD chan_info;
        GetChannelInfo(chan_info);
        g_AMSCommunication.mvSendChannelInfo(chan_info);
    }
}

//是否事件录像
void CSkpChannelEntity::SetEventCapture(int bEventCapture)
{
    #ifndef NOEVENT
    if(bEventCapture == 1 || bEventCapture == 3)
    m_skpRoadDetect.SetEventCapture(true);
    else
    m_skpRoadDetect.SetEventCapture(false);
    #endif

    #ifndef NOOBJECTDETECT
    if(bEventCapture == 1 || bEventCapture == 3)
    m_ObjectBehaviorDetect.SetEventCapture(true);
    else
    m_ObjectBehaviorDetect.SetEventCapture(false);
    #endif

    #ifndef NOPLATE
    if(bEventCapture == 2 || bEventCapture == 3)
	{
		m_RoadCarnumDetect.SetEventCapture(true);

		if(m_pAbstractCamera)
		{
			m_pAbstractCamera->SetHaveSkpRecorder(true);

/*
			CSkpRoadRecorder *pSkpRecorder = m_RoadCarnumDetect.GetSkpRecorder();
			m_pAbstractCamera->SetSkpRecorder(*pSkpRecorder);
*/
		}
	}
    else
	{
		m_RoadCarnumDetect.SetEventCapture(false);
		if(m_pAbstractCamera)
		{
			m_pAbstractCamera->SetHaveSkpRecorder(false);
		}
	}
    #endif

    m_sChannel.bEventCapture = bEventCapture;
}

//修改事件录像
void CSkpChannelEntity::ModifyEventCapture(int bEventCapture)
{
    if(m_sChannel.bEventCapture != bEventCapture)
    {
        #ifndef NOEVENT
        if(bEventCapture == 1 || bEventCapture == 3)
        m_skpRoadDetect.ModifyEventCapture(true);
        else
        m_skpRoadDetect.ModifyEventCapture(false);
        #endif

        #ifndef NOOBJECTDETECT
        if(bEventCapture == 1 || bEventCapture == 3)
        m_ObjectBehaviorDetect.ModifyEventCapture(true);
        else
        m_ObjectBehaviorDetect.ModifyEventCapture(false);
        #endif

        #ifndef NOPLATE
        if(bEventCapture == 2 || bEventCapture == 3)
		{
			m_RoadCarnumDetect.ModifyEventCapture(true);

			if(m_pAbstractCamera)
			{
				m_pAbstractCamera->SetHaveSkpRecorder(true);

/*
				CSkpRoadRecorder *pSkpRecorder = m_RoadCarnumDetect.GetSkpRecorder();
				m_pAbstractCamera->SetSkpRecorder(*pSkpRecorder);
*/
			}
		}
        else
		{
			m_RoadCarnumDetect.ModifyEventCapture(false);
			if(m_pAbstractCamera)
			{
				m_pAbstractCamera->SetHaveSkpRecorder(false);
			}
		}
        #endif
        m_sChannel.bEventCapture = bEventCapture;
    }
}

//设置事件录像时间
void CSkpChannelEntity::SetChannelEventCaptureTime(const int nEventCaptureTime)
{
    if(m_sChannel.uEventCaptureTime != nEventCaptureTime)
    {
        m_sChannel.uEventCaptureTime = nEventCaptureTime;
        //设置事件录象长度
        #ifndef NOEVENT
        m_skpRoadDetect.SetCaptureTime(nEventCaptureTime);
        #endif

        #ifndef NOOBJECTDETECT
        m_ObjectBehaviorDetect.SetCaptureTime(nEventCaptureTime);
        #endif

        #ifndef NOPLATE
        m_RoadCarnumDetect.SetCaptureTime(nEventCaptureTime);
        #endif
    }
}


//是否连接通道（推送实时视频）
void CSkpChannelEntity::SetConnect(bool bConnect)
{
    printf("CSkpChannelEntity::SetConnect=%d\r\n",bConnect);
    m_bConnect = bConnect;
    #ifndef NOEVENT
    m_skpRoadDetect.SetConnect(bConnect);
    #endif

    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetConnect(bConnect);
    #endif

	#ifndef NOFACEDETECT
    m_FaceDetect.SetConnect(bConnect);
    #endif

    #ifndef NOPLATE
	//两种都要设置
	m_RoadCarnumDetect.SetConnect(bConnect);
    #endif

	#ifdef DSPDATAPROCESS
	//m_DspDataProcess.SetConnect(bConnect);
    #endif
}

//设置通道地点
void CSkpChannelEntity::SetChannelLocation(std::string location)
{
	location.clear();
	location = g_skpDB.GetPlace(m_sChannel.uId);

	m_strLocation = location;
    #ifndef NOPLATE
	m_RoadCarnumDetect.SetPlace(location);
    #endif
    #ifndef NOEVENT
    m_skpRoadDetect.SetPlace(location);
    #endif

    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetPlace(location);
    #endif

	#ifndef NOFACEDETECT
    m_FaceDetect.SetPlace(location);
    #endif

	#ifndef NOFEATURESEARCH
	m_FeatureSearchDetect.SetPlace(location);
	#endif

	#ifdef DSPDATAPROCESS
	//m_DspDataProcess.SetPlace(location);
    #endif
}

//设置通道方向
void CSkpChannelEntity::SetChannelDirection(int nDirection)
{
    m_sChannel.uDirection = nDirection;
    #ifndef NOPLATE
	m_RoadCarnumDetect.SetDirection(nDirection);
    #endif
    #ifndef NOEVENT
    m_skpRoadDetect.SetDirection(nDirection);
    #endif
    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetDirection(nDirection);
    #endif

	 #ifndef NOFACEDETECT
    m_FaceDetect.SetDirection(nDirection);
    #endif

	#ifndef NOFEATURESEARCH
	m_FeatureSearchDetect.SetDirection(nDirection);
	#endif

	#ifdef DSPDATAPROCESS
	//m_DspDataProcess.SetDirection(nDirection);
    #endif
}

//设置相机编号
void CSkpChannelEntity::SetCameraID(int nCameraID)
{
    m_sChannel.nCameraId = nCameraID;
    #ifndef NOPLATE
    m_RoadCarnumDetect.SetCameraID(nCameraID);
    #endif
    #ifndef NOEVENT
    m_skpRoadDetect.SetCameraID(nCameraID);
    #endif
    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetCameraID(nCameraID);
    #endif
	#ifndef NOFACEDETECT
    m_FaceDetect.SetCameraID(nCameraID);
    #endif
    #ifndef NOFEATURESEARCH
    m_FeatureSearchDetect.SetCameraID(nCameraID);
    #endif
}

//设置设备ID
void CSkpChannelEntity::SetDeviceID(std::string strDeviceId)
{
	memset(m_sChannel.chDeviceID, 0, sizeof(m_sChannel.chDeviceID));
	memcpy(m_sChannel.chDeviceID, strDeviceId.c_str(), strDeviceId.size());

#ifndef NOPLATE
	m_RoadCarnumDetect.SetDeviceID(strDeviceId);
#endif
	
	
#ifndef NOEVENT
    m_skpRoadDetect.SetDeviceID(strDeviceId);
#endif
    /*#ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetCameraID(nCameraID);
    #endif
	#ifndef NOFACEDETECT
    m_FaceDetect.SetCameraID(nCameraID);
    #endif
    #ifndef NOFEATURESEARCH
    m_FeatureSearchDetect.SetCameraID(nCameraID);
    #endif
	*/
}

//设置相机型号
void CSkpChannelEntity::SetCameraType(int nCameraType)
{
    m_sChannel.nCameraType = nCameraType;
    #ifndef NOPLATE
    m_RoadCarnumDetect.SetCameraType(nCameraType);
    #endif
    #ifndef NOEVENT
    m_skpRoadDetect.SetCameraType(nCameraType);
    #endif
    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetCameraType(nCameraType);
    #endif
	#ifndef NOFACEDETECT
    m_FaceDetect.SetCameraType(nCameraType);
    #endif
}

//设置通道检测类型
void CSkpChannelEntity::SetChannelDetectKind(CHANNEL_DETECT_KIND dType)
{
    m_sChannel.uDetectKind = dType;
    #ifndef NOPLATE
	//两个都要设置，不然在切换模式时会有一方没有被设置检测类型
	m_RoadCarnumDetect.SetDetectKind(dType);
    #endif
    #ifndef NOEVENT
    m_skpRoadDetect.SetDetectKind(dType);
    #endif
    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetDetectKind(dType);
    #endif
	#ifndef NOFACEDETECT
    m_FaceDetect.SetDetectKind(dType);
    #endif

	#ifdef DSPDATAPROCESS
	//m_DspDataProcess.SetDetectKind(dType);
    #endif
}

//设置监视器编号
void CSkpChannelEntity::SetMonitorID(int nMonitorID)
{
    m_sChannel.nMonitorId = nMonitorID;
    #ifndef NOEVENT
    m_skpRoadDetect.SetMonitorID(nMonitorID);
    #endif
    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetMonitorID(nMonitorID);
    #endif
}

//截取图片(需要根据不同区域类型)
void CSkpChannelEntity::CaptureOneFrame(std::string& result,ImageRegion imgRegion)
{
        if(m_chImageData!=NULL)
        {
			 if(g_nDetectMode == 2)
			 {

				if (m_pAbstractCamera)
				{				    
					//if(g_nDetectMode == 2)
				    {
				        //若为触发模式，先发一条取图命令
                        CAMERA_CONFIG cfg;
                        /*cfg = m_pAbstractCamera->GetCameraConfig();
                        if(cfg.nMode <= 0)
                        {
                            printf("====CSkpChannelEntity::CaptureOneFrame=cfg.nMode=%d==========\n", cfg.nMode);
							 cerr << "==CSkpChannelEntity::CaptureOneFrame=cfg.nMode=%d==========\n" << endl;
                            cfg.nMode = 1;
                        }

                        if(cfg.nMode == 1) //(0：连续方式,1：触发方式)*/
                        {
                           // printf("===CSkpChannelEntity::CaptureOneFrame====CameraControl==getonepic=\n");
							 cerr << "==CSkpChannelEntity::CaptureOneFrame====CameraControl==getonepic=\n" << endl;
                            cfg.uType = 2;
                            cfg.nIndex = CAMERA_GET_ONE_PIC;
                            cfg.nGetOnePic = 1;
                            this->CameraControl(cfg);

							//sleep(1);
                        }

				    }//End of if

					{
						if(m_JpgImageData2 != NULL)
						{
							Image_header_dsp *pDsp_header = (Image_header_dsp *)(m_JpgImageData2+sizeof(yuv_video_buf));

							LogNormal("=22=CaptureOneFrame==pDsp_header->nSize=%d=\n", pDsp_header->nSize);
							if((pDsp_header->nSize > 0)&& (pDsp_header->nSize <= m_sDetectHeader.uWidth * m_sDetectHeader.uHeight * 16-sizeof(Image_header_dsp)-sizeof(yuv_video_buf)))
							{
#ifndef DEBUG_PLATE_TEST
								result.append((char*)(m_JpgImageData2+sizeof(Image_header_dsp)+sizeof(yuv_video_buf)), pDsp_header->nSize);
#endif
							}
						}
					}					

				}
				return;
			 }


			    BYTE* pRgbImage = NULL;
				BYTE* pJpgImageData = NULL;
				BYTE* pImage = NULL;

                int width = m_uWidth;
                int height = m_uHeight;
                int srcstep = 0;

                BYTE* pImageData = new BYTE[width*height*3];
				memcpy(pImageData,m_chImageData,width*height*3);

				int nDeinterlace = 1;
				if( (width/m_sDetectHeader.uWidth) == (height/m_sDetectHeader.uHeight) )
				{
					nDeinterlace = 1; //帧图像
				}
				else
				{
					nDeinterlace = 2; //场图像
				}

                pJpgImageData = new BYTE[m_uWidth*m_uHeight];

				PutTextOnImage(pImageData);

                if(nDeinterlace==2)//场图像
                {
                    pImage  = new BYTE[width*height*nDeinterlace*3];
                    //RGB视频流resize
                    IppiSize roi;
                    roi.width  = m_uWidth;
                    roi.height = m_uHeight;

                    IppiSize   dstRoi;
                    dstRoi.width = m_uWidth;
                    dstRoi.height = m_uHeight*nDeinterlace;

                    int lineStep   = roi.width*3 + IJL_DIB_PAD_BYTES(roi.width,3);
                    int dstImgStep = dstRoi.width*3 + IJL_DIB_PAD_BYTES(dstRoi.width,3);
                    double   ratioX = (double)dstRoi.width / (double)roi.width ;
                    double   ratioY = (double)dstRoi.height / (double)roi.height ;
                    IppiRect srcroi= {0,0,roi.width,roi.height};

                    

					#ifndef ALGORITHM_DL
					ippiResize_8u_C3R(pImageData, roi, lineStep, srcroi, pImage, dstImgStep, dstRoi,ratioX, ratioY,IPPI_INTER_LINEAR);
					#else
					//IppiPoint dstOffset={0,0};
					//ippiResizeLinear_8u_C3R(pImageData,lineStep,pImage, dstImgStep,dstOffset,dstRoi,ippBorderInMem,NULL,NULL, NULL);
					#endif


				    pRgbImage = pImage;
				}
				else//帧图像
				{
					pRgbImage = pImageData;
				}

                    int nOffsetX = 0;
                    int nOffsetY = 0;
                    int nWidth = width;
                    int nHeight = height*nDeinterlace;
                    if(imgRegion.nImageRegionType == WHOLE_IMAGE)
                    {
                        srcstep = 0;
                    }
                    else
                    {
                        srcstep = m_uWidth * 3 + IJL_DIB_PAD_BYTES(m_uWidth,3);
                        float floatX = m_uWidth/m_sDetectHeader.uWidth;
                        float floatY = (m_uHeight/m_sDetectHeader.uHeight)*nDeinterlace;

                        if(imgRegion.nImageRegionType == FOCUS_REGION_IMAGE)
                        {

                                nWidth = imgRegion.width*floatX;
                                nHeight = imgRegion.height*floatY;
                                nOffsetX = imgRegion.x*floatX*3;
                                nOffsetY = srcstep*imgRegion.y*floatY;

                        }
                        else if(imgRegion.nImageRegionType == VIOLATION_REGION_IMAGE)
                        {

                                nWidth = m_cropOrd.uViolationWidth*floatX;
                                nHeight = m_cropOrd.uViolationHeight*floatY;
                                nOffsetX = m_cropOrd.uViolationX*floatX*3;
                                nOffsetY = srcstep*m_cropOrd.uViolationY*floatY;

                        }
                        else if(imgRegion.nImageRegionType == EVENT_REGION_IMAGE)
                        {
                            nWidth = m_cropOrd.uEventWidth*floatX;
                            nHeight = m_cropOrd.uEventHeight*floatY;
                            nOffsetX = m_cropOrd.uEventX*floatX*3;
                            nOffsetY = srcstep*m_cropOrd.uEventY*floatY;
                        }
                        else if(imgRegion.nImageRegionType == TRAFFIC_SIGNAL_REGION_IMAGE)
                        {
                            nWidth = m_cropOrd.uTrafficSignalWidth*floatX;
                            nHeight = m_cropOrd.uTrafficSignalHeight*floatY;
                            nOffsetX = m_cropOrd.uTrafficSignalX*floatX*3;
                            nOffsetY = srcstep*m_cropOrd.uTrafficSignalY*floatY;
                        }
                    }
                    printf("nOffsetX=%d,nOffsetY=%d,nWidth=%d,nHeight=%d\n",nOffsetX,nOffsetY,nWidth,nHeight);
                    if(m_JpgToAvi.Encode(pRgbImage+nOffsetX+nOffsetY,nWidth,nHeight,&srcstep,pJpgImageData))
                    {
                        result.append((char*)pJpgImageData,srcstep);
                    }


                if(pImageData)
				{
                    delete []pImageData;
				}

                if(pImage)
				{
                    delete []pImage;
				}

				if(pJpgImageData)
				{
					delete []pJpgImageData;
				}

		}
}

/* 函数介绍：相机控制(for gige)
* 输入参数：cfg-相机控制参数
* 输出参数：无
* 返回值：无
*/
void CSkpChannelEntity::CameraControl(CAMERA_CONFIG& cfg)
{
	if ((strcmp(cfg.chCmd,"epl")==0) && (cfg.nIndex == (int)CAMERA_CMD))
	{
		LogNormal("使用cmd修改了相机的极性 epl=%d",(int)cfg.fValue);
	}
	if(cfg.nIndex == (int)CAMERA_POL)
	{
		LogNormal("使用pol修改了相机的极性 epl=%d",(int)cfg.fValue);
	}

    LogTrace(NULL, "====CSkpChannelEntity::CameraControl()==cfg.nIndex=%d===Type=%d\n", cfg.nIndex, cfg.uType);
    if(cfg.nIndex >= ZOOM_FAR && cfg.nIndex <= SWITCH_CAMERA)//模拟相机PTZ控制
    {
            if(cfg.nIndex == GOTO_PRESET)//选择预置位
            {
                //更新数据库(下次启动时用)
                int nPreSet =(int)cfg.fValue;
                if(g_skpDB.SetPreSet(nPreSet,m_sChannel.uId))
                {
                    m_sChannel.nPreSet = nPreSet;
                }
            }
            else if(cfg.nIndex == SET_PRESET)
            {
                int nPreSet =(int)cfg.fValue;
                LogNormal("用户设置预置位%d",nPreSet);
            }
            else if(cfg.nIndex == CLEAR_PRESET)
            {
                int nPreSet =(int)cfg.fValue;
                LogNormal("用户清除预置位%d",nPreSet);
            }


            cfg.nAddress = m_sChannel.nCameraId;


            //相当于键盘控制Vis的切换
            {
                int nMoveWeight = 1;
                CAMERA_MESSAGE nMsg = (CAMERA_MESSAGE)cfg.nIndex;

                if(m_sChannel.nCameraType == SANYO_CAMERA) //对于三洋相机，
                {
					if(cfg.uType==1)//表示检测器或客户端发的命令
                    {
						 if(nMsg == ZOOM_NEAR ||
						   nMsg == ZOOM_FAR)
						  {
								LogTrace(NULL, "=============cfg.nIndex=%d, Type:%d\n",cfg.nIndex, cfg.uType);
								m_pAbstractCamera->ManualControl(cfg);
						  }
						 else
						 {

							if(g_ytControlSetting.nProtocalType == 0)//虚拟键盘码协议
							{
								if(g_nKeyBoardID >0 && g_ytControlSetting.nNeedControl == 1)
								{
									g_VisKeyBoardControl.SendKeyData(cfg, m_sChannel.nMonitorId, nMoveWeight,false);
								}
							}
							else //pelco协议
							{
								g_VisSerialCommunication.SendData(cfg);
							}
						 }
                    }
					else if(cfg.uType==0)//表示vis发来的命令
					{
						if(nMsg == ZOOM_NEAR ||
						   nMsg == ZOOM_FAR  ||
						   nMsg == SWITCH_CAMERA||
						   nMsg == SET_PRESET ||
						   nMsg == GOTO_PRESET)
						  {
								LogTrace(NULL, "=============cfg.nIndex=%d, Type:%d\n",cfg.nIndex, cfg.uType);
								m_pAbstractCamera->ManualControl(cfg);
						  }
					}
                }
				else if(m_sChannel.nCameraType == DH_CAMERA)
				{
					m_pAbstractCamera->ManualControl(cfg);
				}
				else
				{

					//相当于键盘控制Vis的切换

                    printf("===============cfg.nOperationType=%d\n",cfg.nOperationType);
                    if(g_ytControlSetting.nProtocalType == 0)//虚拟键盘码协议
					{
						if(g_nKeyBoardID >0 && g_ytControlSetting.nNeedControl == 1)
						{
							g_VisKeyBoardControl.SendKeyData(cfg, m_sChannel.nMonitorId, nMoveWeight,false);
						}
					}
                    else //pelco协议
					{
						g_VisSerialCommunication.SendData(cfg);
					}
				}
             }
    }
    else if(cfg.nIndex == CAMERA_FLASH)//视频爆闪 控制
    {
        if(1 == cfg.nFlashOn)
        {
            FlashSerial *flashSerial = FlashSerial::CreateStance();
            flashSerial->InputCmd(cfg.nFlashPort,2);
        }
    }
    else if(cfg.nIndex < ZOOM_FAR)//相机控制
    {
        if(m_pAbstractCamera)
        {
            if(cfg.uType==1)//设置
            {
                printf("=============cfg.nIndex=%d\n",cfg.nIndex);
                m_pAbstractCamera->ManualControl(cfg);
            }
            else if(cfg.uType==2)//读取单条记录
            {
                printf("======CSkpChannelEntity::CameraControl=======cfg.chCmd=%s\n",cfg.chCmd);
                m_pAbstractCamera->ReadCameraSetting(cfg);
            }
            else if(cfg.uType == 0) //读取所有记录
            {
                printf("=======CSkpChannelEntity::CameraControl======cfg.uType=%d\n",cfg.uType);
                m_pAbstractCamera->ReadCameraSetting(cfg);
            }
            else if(cfg.uType == 3) //载入默认模板
            {
                printf("=======CSkpChannelEntity::CameraControl======cfg.uType=%d\n",cfg.uType);

                m_pAbstractCamera->GetDefaultCameraModel(cfg);
            }
        }
    }
    else
    {
        printf("======22222=======cfg.nIndex=%d\n",cfg.nIndex);
        m_pAbstractCamera->ManualControl(cfg);
    }
}


//发送区域图像
void CSkpChannelEntity::SendRegionImage(ImageRegion imgRegion)
{
    m_imageRegion = imgRegion;
}

//计算地面亮度
void CSkpChannelEntity::CalculateRoadContext()
{
#ifndef NOPLATE
    if(m_imgCotext != NULL)
    {
        if(m_imgCotext->width>800)
        {
            cvSetData(m_imgCotext,m_chImageData,m_imgCotext->widthStep);
           // printf("m_imgCotext->width=%d,m_imgCotext->height=%d\n",m_imgCotext->width,m_imgCotext->height);
            //cvConvertImage(m_imgCotext, m_imgCotext, CV_CVTIMG_SWAP_RB);

            //获取地面亮度
            CvRect rect;
            rect.x = 0;
            rect.y = m_imgCotext->height*2/3;
            rect.width = m_imgCotext->width;
            rect.height = m_imgCotext->height/3;


            //需要判断白天还是晚上
            static int count = 0;

            if(count>1000)
            {
                count = 0;
            }
            if(count==0)
            {
                m_nDayNight = DayOrNight();
            }
            count++;

            //printf("======before mvGetBGLight nDayNight=%d\n",m_nDayNight);
            int lightTypes;
			#ifndef ALGORITHM_YUV
            m_CameraCrt.mvGetBGLight(m_imgCotext,rect,NULL,0,lightTypes,m_nDayNight);
			#endif
            //printf("after mvGetBGLight\n");
        }
    }

#endif
}

//相机自动控制
void CSkpChannelEntity::CameraAutoControl(UINT32 uTimeStamp)
{
	#ifndef ALGORITHM_YUV
	int interval = 20;

	if( uTimeStamp >= m_detect_time + interval)
    {
		//根据车牌程序返回的亮度和方差进行修正
		float fRate = 1;
		float fIncrement = 0;
		int nIris = 0;//最大值

		bool bDetectCarnum = ((m_sChannel.uDetectKind&DETECT_CARNUM)==DETECT_CARNUM);

        #ifndef NOPLATE
        CAMERA_CONFIG cfg;
        if(m_pAbstractCamera)
        {
            cfg = m_pAbstractCamera->GetCameraConfig();

			//LogNormal("1 CameraAutoControl convert uGain=%.2f,uSH=%.2f\n",cfg.fGain,cfg.fSH);
            m_pAbstractCamera->ConvertCameraParameter(cfg,true);//转换为db和us传送给相机控制
			//LogNormal("2 CameraAutoControl convert uGain=%.2f,uSH=%.2f\n",cfg.fGain,cfg.fSH);
        }
        CAMERA_PARA para;
        int nMaxPE = 0;
        //极大值
        if(bDetectCarnum)
        {
            para.nMaxGain = cfg.nMaxGain;
            para.nMaxSH = cfg.nMaxSH;
            nMaxPE = cfg.nMaxPE;
        }
        else
        {
            para.nMaxGain = cfg.nMaxGain2;
            para.nMaxSH = cfg.nMaxSH2;
            nMaxPE = cfg.nMaxPE2;
        }

        //当前值
        para.nGain = cfg.uGain;
        para.nSH = cfg.uSH;
        para.nIris = cfg.nIris;
        if((m_sChannel.nCameraType < PTG_ZEBRA_200) || (m_sChannel.nCameraType == BASLER_200))//
        {
            para.nSH = cfg.uPE;
			para.nMaxSH = nMaxPE;
        }
		else if((m_sChannel.nCameraType == BOCOM_301_200) ||(m_sChannel.nCameraType == BOCOM_302_200)
			    || (m_sChannel.nCameraType == BOCOM_301_500) || (m_sChannel.nCameraType == BOCOM_302_500) )
		{
			para.nGain = cfg.fGain;
			para.nSH = cfg.fSH;
			para.nMaxSH = nMaxPE;
		}
        para.nEn = cfg.EEN_on;

        CameraParametter* pCameraCrt = NULL;
        if(bDetectCarnum)
        {
            pCameraCrt = m_RoadCarnumDetect.GetCameraCrt();
        }
        else
        {
            pCameraCrt = &m_CameraCrt;
        }

		//LogNormal("CameraAutoControl uGain=%d,uSH=%d,nMaxGain=%d,nMaxSH=%d\n",para.nGain,para.nSH,para.nMaxGain,para.nMaxSH);

        int isDayByLight = 1;//1:day;0:night
        int nEn = 0;//0关灯；1：开灯
        if(pCameraCrt)
        {
            pCameraCrt->GetCameraControl_Para(uTimeStamp,m_detect_time,interval,bDetectCarnum,para,fRate,fIncrement,nIris,isDayByLight,nEn);
        }
        #endif

        //LogNormal("after GetCameraControl_Para,before AutoControl\n");
        //增益、曝光时间调整
        if(m_pAbstractCamera)
		{
			m_pAbstractCamera->AutoControl(uTimeStamp,fRate,fIncrement,bDetectCarnum,nIris,nEn);
		}
        m_detect_time = uTimeStamp;

        //LogNormal("after AutoControl\n");
    }
	#endif
}


//初始化视频抖动检测数据
void CSkpChannelEntity::InitShakeDetect()
{
    //UnInitShakeDetect();
    //获取稳像区域
    CXmlParaUtil xml;
    RegionList listStabBack; //稳像区域
    xml.GetStabBackRegion(m_sChannel.uId,listStabBack);
    //对坐标进行缩放处理
    RegionList::iterator it_rb = listStabBack.begin();
    RegionList::iterator it_re = listStabBack.end();
    Point32fList::iterator it_32fb;
    Point32fList::iterator it_32fe;

    float floatX = m_uWidth/m_sDetectHeader.uWidth;
    float floatY = m_uHeight/m_sDetectHeader.uHeight;

    m_nStabPolyNum = listStabBack.size();
    if(m_nStabPolyNum >0)
    {
        m_nStabPolySize = new int [ m_nStabPolyNum ];
        m_fStabPolyPts = new CvPoint2D32f* [m_nStabPolyNum];

        int i =0;
        int j =0;
        CvPoint2D32f pt32f;
        while(it_rb!=it_re)
        {
            it_32fb = it_rb->pt32fList.begin();
            it_32fe = it_rb->pt32fList.end();

            m_nStabPolySize[i] = it_rb->pt32fList.size();
            m_fStabPolyPts[i]  = new CvPoint2D32f[ m_nStabPolySize[i] ];
            j = 0;
            while (it_32fb!=it_32fe)
            {
                pt32f.x = it_32fb->x/floatX;
                pt32f.y = it_32fb->y/floatY;
                printf("m_fStabPolyPts pt32f.x=%f,pt32f.y=%f\n",pt32f.x,pt32f.y);

                m_fStabPolyPts[i][j] = pt32f;

                it_32fb++;
                j++;
            }
            it_rb++;
            i++;
        }
    }
}

//释放视频抖动检测数据
void CSkpChannelEntity::UnInitShakeDetect()
{
    for(int i=0; i<m_nStabPolyNum; i++ )
    {
		 delete [] m_fStabPolyPts[i];
	}
	if(m_nStabPolySize)
	{
        delete [] m_nStabPolySize;
        m_nStabPolySize = NULL;
	}
	if(m_fStabPolyPts)
	{
        delete [] m_fStabPolyPts;
        m_fStabPolyPts = NULL;
	}
	m_nStabPolyNum = 0;
}

//检测视频是否抖动
void CSkpChannelEntity::CheckShakedFrame(UINT32 uTimeStamp)
{
     if(m_nStabPolyNum <=0)
     {
        return;
     }

     if(m_imgCurr==NULL)
     {
         return;
     }

     if (uTimeStamp > m_tmLastCheck + 2)//2s检测一次
     {
         printf("============CheckShakedFrame,uTimeStamp=%u,m_tmLastCheck=%u\n",uTimeStamp,(UINT32)m_tmLastCheck);
         //保存当前帧，用于视频震动检测
         memcpy(m_imgCurr->imageData,m_smImageData,m_imgCurr->imageSize);

         int offsetX = 0, offsetY = 0;

         AffParams  offSetParams;
         int nMaxStbRadius = 12;
         float fCornerQuality = 0.05;
         int nRes = m_CStab.mvGetImgOffSet4Client(m_imgCurr, m_nStabPolyNum, m_nStabPolySize, (const CvPoint2D32f**)m_fStabPolyPts, TRANSLATION_MODEL, offSetParams, nMaxStbRadius, fCornerQuality);

         if (0 == nRes) //匹配上，可给出偏移位移
         {
             this->m_nMiss = 0;
             offsetX = offSetParams.dx;
             offsetY = offSetParams.dy;
             //x方向抖动幅度超过4像素，或y方向抖动幅度超过3像素，则认为出现抖动
             if ( abs(offsetX) > 4 || abs(offsetY) > 3)
             {
                 g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_SHAKED);

                 //LogWarning("视频画面有抖动现象！\n");
                 //printf("============视频画面有抖动现象\n");
                 g_skpRoadLog.WriteLog("摄像机位置移动\n",ALARM_CODE_CAMERA_MOVED,true);

                 //往客户端发送通道状态
                 std::string response;
                 SRIP_CHANNEL sChannel = m_sChannel;
                 sChannel.nCameraStatus = CM_SHAKED;
                 response.insert(0,(char*)&sChannel,sizeof(SRIP_CHANNEL));
                 SRIP_DETECT_HEADER sDetectHeader;
                 sDetectHeader.uDetectType = SRIP_CHANNEL_STATUS;
                 response.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
                 g_skpChannelCenter.AddResult(response);

                 m_CStab.mvReInitialBkGrd( m_imgCurr, fCornerQuality, m_nStabPolySize, (const CvPoint2D32f**)m_fStabPolyPts );
             }
             printf("======offsetX=%d,offsetY=%d\n",offsetX,offsetY);
             m_tmLastCheck = uTimeStamp;
#ifdef SNMP_AGENT
			 g_SnmpAgent.m_bCameraMoved = false;
#endif

             return;//抖动幅度较小，则认为没有抖动
         }
         else if (-1 == nRes) //获取角点失败
         {
             //LogError("采集角点失败！");
             return;
         }
         else if (1 == nRes) //匹配不上
         {
             //LogWarning("视频画面有明显移位现象！\n");
             //printf("============视频画面有明显移位现象\n");
             g_skpRoadLog.WriteLog("摄像机位置移动\n",ALARM_CODE_CAMERA_MOVED,true);
#ifdef SNMP_AGENT
			 g_SnmpAgent.m_bCameraMoved = true;
#endif
         }
         else
         {
             return;
         }

         //找不到匹配的块，则认为是移动，连续5次找不到匹配的则认为是无法修复的人为移动
         if (++m_nMiss >= 5)
         {
             m_nMiss = 0;
             m_CStab.mvReInitialBkGrd( m_imgCurr, fCornerQuality, m_nStabPolySize, (const CvPoint2D32f**)m_fStabPolyPts );
             g_skpDB.SetCameraStateToDBByChannelID(m_sChannel.uId, CM_MOVED);

             //LogError("相机被永久移动！\n");
             g_skpRoadLog.WriteLog("摄像机位置移动\n",ALARM_CODE_CAMERA_MOVED,true);

             //往客户端发送通道状态
             std::string response;
             SRIP_CHANNEL sChannel = m_sChannel;
             sChannel.nCameraStatus = CM_MOVED;
             response.insert(0,(char*)&sChannel,sizeof(SRIP_CHANNEL));
             SRIP_DETECT_HEADER sDetectHeader;
             sDetectHeader.uDetectType = SRIP_CHANNEL_STATUS;
             response.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
             g_skpChannelCenter.AddResult(response);
         }

         m_tmLastCheck = uTimeStamp;
     }
     return;
}

//发送相机状态
void CSkpChannelEntity::SendCameraState(CameraState state)
{
//    assert(nChannel>=0 && state>CM_OK);

    //与中心网管通讯协议头
    sNeMsgHead msgHeader;
    memset(&msgHeader, 0, sizeof(msgHeader));
    msgHeader.m_szSyncHeader[0] = '$';
    msgHeader.m_szSyncHeader[1] = 'h';
    msgHeader.m_szSyncHeader[2] = 'e';
    msgHeader.m_szSyncHeader[3] = 'a';
    msgHeader.m_szSyncHeader[4] = 'd';
    msgHeader.m_szSyncHeader[5] = '$';
    msgHeader.m_szVersion[0] = (char)0x01;
    msgHeader.m_szVersion[1] = (char)0x01;
    msgHeader.m_szVersion[2] = (char)0x01;
    msgHeader.m_szVersion[3] = (char)0x02;
    msgHeader.m_uiMsgType = NE_NOTE_IDE_DEVICE_ALARM;

    timeval tv;
    gettimeofday(&tv, NULL);
    msgHeader.m_ubiWorkflowId = tv.tv_sec ;
    msgHeader.m_ubiWorkflowId = msgHeader.m_ubiWorkflowId << 32;
    msgHeader.m_ubiWorkflowId += tv.tv_usec;

    //设备警告信息
    sNeIdeNoteDeviceAlarm alarmInfo;
    alarmInfo.uiTimeStamp = (UINT32)time(NULL);
    memset(&alarmInfo, 0, sizeof(alarmInfo));
    alarmInfo.sDevice.uiDeviceId = m_sChannel.nCameraId;
    snprintf(alarmInfo.sDevice.szDeviceName, sizeof(alarmInfo.sDevice.szDeviceName),
             "Digital video camera: %d:%d", m_sChannel.nCameraType, m_sChannel.nCameraId);
    alarmInfo.sDevice.uiSideId = m_sChannel.nPannelID;
    strncpy(alarmInfo.sDevice.szInstallPlace, m_strLocation.c_str(),
            sizeof(alarmInfo.sDevice.szInstallPlace));

    if (state == CM_NO_VIDEO)
    {
        alarmInfo.uiAlarmLevel = NE_ALERMLEVEL_MAJOR;
        alarmInfo.uiAlarmCategory = NE_ALARMCATEGORY_DEVICE;
        alarmInfo.uiAlarmType = NE_ALARMTYPE_CAMERA_VIDEOLOST;
        strcpy(alarmInfo.szDescription, "视频源丢失");
    }
    else if (state == CM_MOVED)
    {
        alarmInfo.uiAlarmLevel = NE_ALERMLEVEL_MAJOR;
        alarmInfo.uiAlarmCategory = NE_ALARMCATEGORY_ENVIRONMENT;
        alarmInfo.uiAlarmType = NE_ALARMTYPE_CAMERA_VIDEOLOST;
        strcpy(alarmInfo.szDescription, "摄像机被永久移动");
    }
    else if (state == CM_SHAKED)
    {
        alarmInfo.uiAlarmLevel = NE_ALERMLEVEL_MINOR;
        alarmInfo.uiAlarmCategory = NE_ALARMCATEGORY_QUALITY;
        alarmInfo.uiAlarmType = NE_ALARMTYPE_CAMERA_POSITIONMOVE;
        strcpy(alarmInfo.szDescription, "视频画面出现抖动现象");
    }
    else
    {
        alarmInfo.uiAlarmLevel = NE_ALARMLEVEL_WARNING;
        alarmInfo.uiAlarmCategory = NE_ALARMCATEGORY_QUALITY;
        alarmInfo.uiAlarmType = NE_ALARMTYPE_TYPE_UNKNOWN;
    }

    msgHeader.uiPacketBodyLen = sizeof(alarmInfo);

//printf("Before Send alarm info==================================\n");

    std::string strMsg;
    strMsg.append((char*)&msgHeader,sizeof(sNeMsgHead));
    strMsg.append((char*)&alarmInfo,sizeof(sNeIdeNoteDeviceAlarm));

    g_skpNeServer.SendAlarmInfo(strMsg);

//printf("After Send alarm info==================================\n");
}

//获取检测类型
void CSkpChannelEntity::GetDetectParam(UINT32& DetectorType)
{
    paraDetectList sParamIn;

    SRIP_CHANNEL_EXT sChannel;
    sChannel.uId = m_sChannel.uId;

    CXmlParaUtil xml;
    xml.LoadRoadParameter(sParamIn,sChannel);

    DetectorType = 0;
    paraDetectList::iterator it_b = sParamIn.begin();
    paraDetectList::iterator it_e = sParamIn.end();
    while(it_b!=it_e)
    {
        if(it_b->m_bStop)
        {
            DetectorType |= 0x01;
        }

        if(it_b->m_bNixing)
        {
            DetectorType |= 0x02;
        }

        if(it_b->m_bCross)
        {
            DetectorType |= 0x04;
        }

        if(it_b->m_bDusai)
        {
            DetectorType |= 0x08;
        }

        if(it_b->m_bBianDao)
        {
            DetectorType |= 0x10;
        }

        if(it_b->m_bDiuQi)
        {
            DetectorType |= 0x20;
        }

        if(it_b->m_bOnlyOverSped)
        {
            DetectorType |= 0x40;
        }

        if(it_b->m_bObjectAppear)
        {
            DetectorType |= 0x80;
        }

        it_b++;
    }
}

//获取通道情况
void CSkpChannelEntity::GetChannelInfo(CHANNEL_INFO_RECORD& chan_info)
{
    chan_info.uChannelID = m_sChannel.uId;
    chan_info.uCameraID = m_sChannel.nCameraId;

	if(m_nStatus == NORMAL_STATUS)
    chan_info.uWorkStatus = 1;
	else
    chan_info.uWorkStatus = 0;

    if(m_sChannel.nCameraType == VEDIO_H264 ||
       m_sChannel.nCameraType == VEDIO_MJPEG ||
	   m_sChannel.nCameraType == VEDIO_YUV)
    chan_info.uRealTime = 1;
    else if(m_sChannel.nCameraType == VEDIO_H264_FILE||
		m_sChannel.nCameraType == VEDIO_YUV_FILE)
    chan_info.uRealTime = 0;
    GetDetectParam(chan_info.DetectorType);
}

//获取通道图像分辨率
void CSkpChannelEntity::GetImageSize(int& nWidth,int& nheight)
{
    nWidth = m_uWidth;
    nheight = m_uHeight;
}

//设置录像类型
void CSkpChannelEntity::SetChannelCaptureType(CAPTURE_TYPE eType)
{
    m_sChannel.eCapType = eType;
    #ifndef NOPLATE
    m_RoadCarnumDetect.SetCaptureType(eType);
    #endif
    #ifndef NOEVENT
    m_skpRoadDetect.SetCaptureType(eType);
    #endif
    #ifndef NOOBJECTDETECT
    m_ObjectBehaviorDetect.SetCaptureType(eType);
    #endif
    m_H264Capture.SetCaptureType(eType);
}

//自动爆闪灯控制
void CSkpChannelEntity::AutoFlashControl(yuv_video_buf* buf)
{

    #ifdef VIDEOSHUTTER

	if(m_bReloadConfig)
	{
			    m_shut.mv_UnInit();

				//取世界坐标和图像坐标
				float image_cord[12];
				float world_cord[12];
				CXmlParaUtil get_parameter;
				int channelID = GetChannelID();
				get_parameter.GetCalibration(image_cord, world_cord, channelID);

				UINT32 flash_uWidth = m_uWidth;
				UINT32 flash_uHeight = m_uHeight;
				if(flash_imgCotext == NULL)
				{
					flash_imgCotext = cvCreateImageHeader(cvSize(flash_uWidth, flash_uHeight), 8, 3);
				}
				vector<mvvideorgnstru> mChannelReg;
				CvRect VideoRect,rtCarnumber;

				get_parameter.GetVirtualLoopRegion(channelID,mChannelReg,VideoRect,rtCarnumber);

				//LogNormal("rw=%d,rh=%d,vw=%d,vh=%d",rtCarnumber.width,rtCarnumber.height,VideoRect.width,VideoRect.height);
				if((rtCarnumber.width > 0) && (rtCarnumber.height > 0) && (VideoRect.width>0) &&(VideoRect.height > 0))
					m_shut.mv_Init( VideoRect, rtCarnumber.width, rtCarnumber.height, mChannelReg, flash_uWidth, flash_uHeight, 6, image_cord, world_cord);

				m_bReloadConfig = false;
	}

	if(flash_imgCotext)
	{
        int channel = -1;
        BYTE* flash_chImageData = m_chImageData;
        cvSetData(flash_imgCotext,flash_chImageData,flash_imgCotext->widthStep);
        FlashSerial* flashSerial=FlashSerial::CreateStance();
        //vector<skip_video_out_result> vResult;
		m_vShutResult.clear();

        printf("before m_shut.mv_DetectEveryFrame===\n");
        m_nDayNight = DayOrNight(1);
		printf("###==m_nDayNight=%d\n", m_nDayNight);
        m_shut.mv_DetectEveryFrame( flash_imgCotext, ((buf->ts)/1000),m_nDayNight );
        printf("after m_shut.mv_DetectEveryFrame===\n");
        m_shut.mv_GetVideoResult( m_vShutResult );

		/*struct timeval tv;
		gettimeofday(&tv,NULL);
		int64_t ts = tv.tv_sec*1000000+tv.tv_usec;
		FILE *fp = fopen("video.txt", "a");
		fprintf(fp,"deal one frame seq = %u,ts = %lld,m_vShutResult.size()=%d\n", buf->nSeq,ts,m_vShutResult.size());
		fclose(fp);*/

		if(m_vShutResult.size() > 0)
		{
			vector<skip_video_out_result>::iterator it_b = m_vShutResult.begin();
			vector<skip_video_out_result>::iterator it_e = m_vShutResult.end();

			while(it_b != it_e)
			{
				channel = it_b->nChannel-1;

				LogNormal("videoshutter open channel=%d",channel);

				if(channel >= 0 && channel <= 3)
				{
					flashSerial->InputCmd(channel,2);
				}

				buf->uFlashSignal |= (0x1<<channel);

				it_b++;
			}


			/*struct timeval tv;
			gettimeofday(&tv,NULL);
			int64_t ts = tv.tv_sec*1000000+tv.tv_usec;
			FILE *fp = fopen("video.txt", "a");
			fprintf(fp,"send serialsignal seq = %u,ts = %lld\n", buf->nSeq,ts);
			fclose(fp);*/

		}
    }
    #endif
}

//叠加文本信息
void CSkpChannelEntity::PutTextOnImage(unsigned char* pBuffer)
{
	IplImage* pImage = cvCreateImageHeader(cvSize(m_uWidth,m_uHeight),8,3);
	cvSetData(pImage,pBuffer,pImage->widthStep);

	//图像文本信息
	CvxText cvText;
	cvText.Init(40);

	wchar_t wchOut[255] = {'\0'};
	char chOut[255] = {'\0'};

	int nWidth = 10;
	int nHeight = 30;

	//经过时间
	std::string strTime;
	strTime = GetTime(m_sDetectHeader.uTimestamp,0);
	sprintf(chOut,"时间: %s:%03d-%u",strTime.c_str(),(int)(((m_sDetectHeader.uTime64)/1000)%1000),m_sDetectHeader.uSeq);
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	nHeight += 30;
	cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

	//经过地点
	sprintf(chOut,"地点: %s",m_strLocation.c_str());
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	nHeight += 40;
	cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

	cvText.UnInit();

	PutLogoOnImage(pImage);

	if(pImage != NULL)
	{
		cvReleaseImageHeader(&pImage);
		pImage = NULL;
	}
}

//叠加LOGO信息
void CSkpChannelEntity::PutLogoOnImage(IplImage* pImage)
{
		IplImage* pLogoImage = NULL;
		std::string strPath = "./logo.bmp";
		if(access(strPath.c_str(),F_OK) == 0)
		{
			pLogoImage = cvLoadImage(strPath.c_str(),-1);
			if(pLogoImage != NULL)
			{
				cvConvertImage(pLogoImage,pLogoImage,CV_CVTIMG_SWAP_RB);
			}
		}

		if(pLogoImage != NULL && pImage != NULL)
		{
			int nWidth = pLogoImage->width;
			int nHeight = pLogoImage->height;

			CvRect rect;
            rect.x = pImage->width - nWidth - 10;
            rect.y = 10;
            rect.width = nWidth;
            rect.height = nHeight;

			if(nWidth > 0 && nHeight > 0)
			{
				cvSetImageROI(pImage,rect);
				cvCopy(pLogoImage,pImage);
				cvResetImageROI(pImage);
			}
		}

		if(pLogoImage != NULL)
		{
			cvReleaseImage( &pLogoImage );
			pLogoImage = NULL;
		}
}

//解码jpg->rgb
bool CSkpChannelEntity::DecodeJpgToRgb(BYTE *pBuf, const int nWidth, const int nHeight, int &srcstep, BYTE *pBufOut)
{
    //jpg->rgb解码
    if(pBuf != NULL)
    {
        CxImage image;

        int lineStep = nWidth*3 + IJL_DIB_PAD_BYTES(nWidth,3);
        srcstep = lineStep * nHeight;

        yuv_video_buf* header = (yuv_video_buf*)pBuf;

#ifdef _DSP_DEBUG
        printf("=1111=CSkpChannelEntity::DecodeJpgToRgb==nBufSize=%d=image.GetSize()=%d===srcstep=%d==\n", \
			header->size, image.GetSize(), srcstep);
#endif
        //先解码jpg->rgb
        image.Decode(pBuf + sizeof(yuv_video_buf), header->size, 3); //解码
        memcpy(pBufOut, image.GetBits(), srcstep);

        //srcstep = image.GetSize();
#ifdef _DSP_DEBUG
        printf("2222==CSkpChannelEntity::DecodeJpgToRgb==nBufSize=%d=image.GetSize()=%d===srcstep=%d==\n", \
			header->size, image.GetSize(), srcstep);
#endif
    }
    else
    {
        return false;
    }


    return true;
}


//清空输出JpgMap列表
void CSkpChannelEntity::ClearJpgMap()
{
	m_RoadCarnumDetect.ClearJpgFrameMap();
}

//重连dsp相机
bool CSkpChannelEntity::ReOpenDsp()
{
	LogNormal("=ReOpenDsp=\n");
	if(m_pAbstractCamera == NULL)
	{
		return false;
	}
	bool bOpenCamera = m_pAbstractCamera->ReOpen();	

	return bOpenCamera;
}

//设置相机IP
void CSkpChannelEntity::SetCameraHost(std::string strCameraHost)
{
	printf("==CSkpChannelEntity::SetCameraHost==\n");

	if(m_pAbstractCamera)
	{
		m_pAbstractCamera->SetCameraIpHost(strCameraHost, m_sChannel.uMonitorPort);
	}
	LogNormal("Old IP:[%s]=New IP:[%s]=\n", m_sChannel.chCameraHost, strCameraHost.c_str());

	//需要添加判断strCameraHost是否合法 FIX!

	printf("#####==strCameraHost to set is:%s \n", strCameraHost.c_str());
	printf("#####=SetCameraHost before memcpy =m_sChannel.chCameraHost=%s=\n", m_sChannel.chCameraHost);
	memset(m_sChannel.chCameraHost, 0, sizeof(m_sChannel.chCameraHost));
	memcpy( m_sChannel.chCameraHost, strCameraHost.c_str(), strCameraHost.size() );
	printf("#####=SetCameraHost after memcpy =m_sChannel.chCameraHost=%s=\n", m_sChannel.chCameraHost);
}

bool CSkpChannelEntity::AddForceAlert(FORCEALERT *pAlert)
{
	#ifndef NOOBJECTDETECT
	return m_ObjectBehaviorDetect.AddForceAlert(pAlert);
	#else
	return true;
	#endif
}


void CSkpChannelEntity::DetectRegionRectImage(ImageRegion imgRegion)
{
	//printf("CSkpChannelEntity::DetectRegionRectImage: %d,%d \n",m_uWidth,m_uHeight);
	//VIS发送过来的
	if(imgRegion.nImageRegionType == TRAFFIC_SIGNAL_REGION_IMAGE)
	{
		printf("CSkpChannelEntity::DetectRegionRectImage: x=%d,y=%d,width=%d,height=%d\n",imgRegion.x,imgRegion.y,imgRegion.width,imgRegion.height);
		int nWidth =  imgRegion.width * (m_uWidth/1000.0);
		int nHeight=  imgRegion.height* (m_uHeight/1000.0);

		int nRect_x =  imgRegion.x* (m_uWidth/1000.0);
		int nRect_y =  imgRegion.y* (m_uHeight/1000.0);

		printf("Real Rect: x=%d,y=%d,width=%d,height=%d \n",nRect_x,nRect_y,nWidth,nHeight);

		if(nRect_x > 0)
		{
			nRect_x += m_uWidth/2;
			nRect_x -= nWidth/2;
		}
		else
		{
			nRect_x = abs(nRect_x);
			printf("nRect_x=%d \n",nRect_x);
			nRect_x = m_uWidth/2-nRect_x;
			printf("nRect_x=%d \n",nRect_x);
			nRect_x -= nWidth/2;
			printf("nRect_x=%d \n",nRect_x);
		}


		if(nRect_y > 0)
		{
			nRect_y = m_uHeight/2 - nRect_y;
			nRect_y -= nHeight/2;
		}
		else
		{
			nRect_y = abs(nRect_y);
			nRect_y += m_uHeight/2;
			nRect_y -= nHeight/2;
		}
		 #ifndef NOEVENT
		printf("Real Rect: x=%d,y=%d,width=%d,height=%d \n",nRect_x,nRect_y,nWidth,nHeight);
		m_skpRoadDetect.DetectRegionRect(nRect_x,nRect_y,nWidth,nHeight,true);
		#endif
	}
	else
	{
		 #ifndef NOEVENT
		printf("CSkpChannelEntity::DetectRegionRectImage: x=%d,y=%d,width=%d,height=%d\n",imgRegion.x,imgRegion.y,imgRegion.width,imgRegion.height);
		m_skpRoadDetect.DetectRegionRect(imgRegion.x*4,imgRegion.y*4,imgRegion.width*4,imgRegion.height*4,true);
		#endif
	}

	return;
}

void CSkpChannelEntity::DetectParkObjectsRect(UINT32 uMsgCommandID,RectObject &ObjectRect)
{
	printf("CSkpChannelEntity::DetectParkObjectsRect \n");
	ParkRect lpRect = ObjectRect.lpRect; //目标的位置
	int nWidth = lpRect.right - lpRect.left;
	int nHigth = lpRect.bottom - lpRect.top;

	int xPos = lpRect.left*4;
	int yPos = lpRect.top*4;
	int width = nWidth*4;
	int height = nHigth*4;
	 #ifndef NOEVENT
	m_skpRoadDetect.DetectParkObjectsRect(ObjectRect.nId,uMsgCommandID,xPos,yPos,width,height,true);
	#endif
}

//录像初始化
bool CSkpChannelEntity::InitRecordCapture()
{
	printf("--------in CSkpChannelEntity::InitRecordCapture()----\n");
	if(g_nEncodeFormat == 1 || g_nEncodeFormat == 2 || g_nEncodeFormat == 4)
	{
		 #ifndef NOEVENT
		m_skpRoadDetect.SetH264Capture(&m_H264Capture);
		#endif
		m_RoadCarnumDetect.SetH264Capture(&m_H264Capture);
				
		if(g_nDetectMode == 2)
		{
			if(m_sChannel.eCapType == CAPTURE_NO)
			{
				LogNormal("====SetHaveH264Capture===false=!!!\n");
				m_pAbstractCamera->SetHaveH264Capture(false);
			}
			else
			{
				m_pAbstractCamera->SetHaveH264Capture(true);
				bool bSet = m_pAbstractCamera->SetH264Capture(m_H264Capture);
				//LogTrace("TT1.log","====SetH264Capture==bSet=%d=\n", bSet);

				m_H264Capture.SetCameraType(m_sChannel.nCameraType);

				if(bSet)
				{
					if(m_sChannel.eCapType == CAPTURE_FULL)
					{
						m_H264Capture.Init(m_sChannel.uId, m_uWidth, m_uHeight);
						LogNormal("==DSP=m_H264Capture.Init=====\n");
					}
				}
			}
		}
		else
		{
			if( (m_sChannel.eCapType != CAPTURE_NO) || (g_nSendRTSP == 1)|| (g_VideoFormatInfo.nSendH264 == 1))
			{
				m_H264Capture.Init(m_sChannel.uId,m_uWidth,m_uHeight);
				//LogNormal("====m_H264Capture.Init=====\n");
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

//缓存录像初始化
bool CSkpChannelEntity::InitTempCapture()
{
	printf("--------in CSkpChannelEntity::InitRecordCapture()----\n");

	if(g_nDetectMode == 2)
	{
		if(m_sChannel.bEventCapture == 2 || m_sChannel.bEventCapture == 3)//开启违章录像,才进行缓存
		{			
			bool bSet = m_pAbstractCamera->SetTempCapture(m_TempCapture);
			if(bSet)
			{
				m_TempCapture.Init(m_sChannel.uId, m_uWidth, m_uHeight);//启动缓存录像线程				
				LogNormal("==DSP=m_TempCapture.Init=====\n");
			}			
		}
	}
	else
	{
		//非dsp相机暂时不做缓存
	}

	return true;
}

int CSkpChannelEntity::AddRecordEvent(int nChannel,std::string result)
{
	printf("CSkpChannelEntity::AddRecordEvent \n");
	if(m_pAbstractCamera)
	{
		m_pAbstractCamera->AddRecordEvent(nChannel,result);
	}
	return 0;
}

#ifdef DEBUG_PLATE_TEST
//添加模拟测试车牌
void CSkpChannelEntity::TestAddPlate(const yuv_video_buf & header)
{
	//模拟检出车牌
	if(header.nSeq > 0)
	{
		int nSeq = header.nSeq;
		//if(nSeq % 3 == 0)
		{
			//printf("==nSeq=%d, nSeqPre=%d\n", nSeq, nSeqPre);
			//nSeqPre = nSeq;
			//帧号
			//m_sDetectHeader.uSeq = nSeq;
			//时间戳
			//m_sDetectHeader.uTimestamp = (header.ts/1000)/1000;
			//时间戳
			//m_sDetectHeader.uTime64 = header.ts;

			BYTE *buf;
			buf = new BYTE[sizeof(RECORD_PLATE_DSP)];
			RECORD_PLATE_DSP *pPlate = (RECORD_PLATE_DSP*)(buf);

			DSP_PLATE_LIST plateList;

			pPlate->uSeq = nSeq;
			pPlate->uTime = GetTimeStamp();

			//memcpy(pPlate->chText ,"ABC1234", 7);
			char bufTT[8] = {0};
			sprintf(bufTT, "ABC1%03d", ((nSeq / 100) % 100) );
			memcpy(pPlate->chText, bufTT, 7);

			pPlate->uColor = 3;
			pPlate->uCredit = 80;
			pPlate->uRoadWayID = 1;
			pPlate->uType = 0x00010001;
			pPlate->uSmallPicSize = 20;
			pPlate->uSmallPicWidth = 10;
			pPlate->uSmallPicHeight = 5;
			pPlate->uPicSize = 1000;
			pPlate->uPicWidth = 612;
			pPlate->uPicHeight = 512;
			pPlate->uPosLeft = 80;
			pPlate->uPosTop = 80;
			pPlate->uPosRight = 120;
			pPlate->uPosBottom = 180;
			pPlate->uCarColor1 = 3;
			pPlate->uSpeed = 20;
			pPlate->uDirection = 3;
			pPlate->uCarBrand = 15;
			pPlate->uCarColor2 = 5;
			pPlate->uWeight1 = 20;
			pPlate->uWeight2 = 30;
			pPlate->uPlateType = 1;
			pPlate->uViolationType = 99;//卡口数据
			pPlate->uTypeDetail = 3;
			pPlate->uSeq2 = nSeq + 4;
			pPlate->uSeq3 = nSeq + 8;
			pPlate->uTime2 = 80;
			pPlate->uMiTime2 = 800;
			pPlate->uContextMean = 80;
			pPlate->uContextStddev = 34;
			pPlate->uVerticalTheta = 0;
			pPlate->uHorizontalTheta = 0;

			plateList.push_back((*pPlate));
			m_RoadCarnumDetect.AddPlateFrame(plateList);

			if(buf)
			{
				delete buf;
				buf = NULL;
			}
		}//End of if
		nSeq++;
	}	
}
#endif

//把有图未输出的数据全部输出
bool CSkpChannelEntity::OutPutResultAll()
{
	bool bRet = false;
	int nSize = 0;

	nSize = m_RoadCarnumDetect.DealOutPutAll();
	if(nSize > 0)
	{
		LogNormal("OutPutResultAll nSize:%d ", nSize);
		bRet = true;
	}

	return bRet;
}

//相机控制
void CSkpChannelEntity::CameraControl()
{
    while(!g_bEndThread&&!m_bEndDeal)
    {
		if((m_sChannel.uDetectKind&DETECT_FLUX) || (m_sChannel.uDetectKind&DETECT_CARNUM) ||(m_sChannel.uDetectKind&DETECT_BEHAVIOR))
		{
			UINT32 uTimeStamp = GetTimeStamp();

			CameraAutoControl(uTimeStamp);
		}

        sleep(5);
	}
}

#ifdef REDADJUST
//红绿灯增强
bool CSkpChannelEntity::RedLightAdjust(IplImage *pImage)
{
#ifndef NOPLATE
	m_RoadCarnumDetect.RedLightAdjust(pImage);
#endif

	return true;
}
#endif

//通过uImgKey，核查记录状态
bool CSkpChannelEntity::CheckImgKeyState(const UINT64 &uKey)
{
	bool bRet = false;
#ifndef NOPLATE
	bRet = m_RoadCarnumDetect.CheckImgListByKey(uKey);
#endif
	return bRet;
}

//更新通道记录标记
bool CSkpChannelEntity::UpdateImgListByKey(const UINT64 &uKey, const int &bState)
{
	bool bRet = false;
#ifndef NOPLATE
	bRet = m_RoadCarnumDetect.UpdateImgListByKey(uKey, bState);
#endif
	return bRet;
}
