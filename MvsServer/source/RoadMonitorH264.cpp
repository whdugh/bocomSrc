//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "RoadMonitorH264.h"
#include "Common.h"
#include "XmlParser.h"
#include "CSeekpaiDB.h"
#ifdef MONITOR
#include "libmonitor.h"
#endif
#include "SobeyCommunication.h"
#include "ximage.h"
#include "ippi.h"
#include "ippcc.h"

//#define HKCAMERA
#ifdef HKCAMERA
#include "AnalyzeDataNewInterface.h"
#define  BUFFER_SIZE (20*1024)
#endif

int CRoadMonitorH264::m_nFmfsPort = 20110;

//#define MONITORLOG
#ifdef MONITORLOG
        FILE* g_pMonitorLog;
#endif

//#define MONITORVIDEO
#ifdef MONITORVIDEO
        FILE* g_pMonitorVideo;
#endif
//采集线程
void* ThreadMonitorCapture(void* pArg)
{
    CRoadMonitorH264* pLibMonitor = (CRoadMonitorH264*)pArg;

    if(pLibMonitor)
    {
        pLibMonitor->DealMonitorCapture();
    }
	LogNormal("end DealMonitorCapture");
    pthread_exit((void *)0);
	return pArg;
}

#ifdef MONITOR

int AVDataCallBackFunc(unsigned long dwCookie,unsigned int extendType,unsigned char* pData,int dataLen
										,long long pts,FRAMETYPE nFrameFlag,unsigned char* pPrivateData,int nPriLen)
{
	printf("dwCookie[%ld],pts=[%lld] datalen=[%d],nFrameFlag[%d],nPriLen[%d],extendType[%d]\n",dwCookie,pts,dataLen,nFrameFlag,nPriLen,extendType);

    #ifdef MONITORVIDEO
    if(g_pMonitorVideo)
    {
			fwrite(pData,dataLen,1,g_pMonitorVideo);
			fflush(g_pMonitorVideo);
    }
    #else
	CRoadMonitorH264* pRoadMonitorH264 = (CRoadMonitorH264*)dwCookie;

    if(pRoadMonitorH264)
    {
        bool bKeyFrame = false;

        if(nFrameFlag == 1)
        {
            bKeyFrame = true;
        }

        if(dataLen > 0)
        pRoadMonitorH264->AddFrame(pData,dataLen,pts,bKeyFrame);
    }
    #endif
	return 1;
}

int GotVideoInfoCallBackFunc(void* hHandle,ST_VideoInfo * pVideoInfo,unsigned long cookie)
{
	printf("Recv stream info\n");
	CRoadMonitorH264* pRoadMonitorH264 = (CRoadMonitorH264*)cookie;

	//实时为H264格式(25fps)，历史视频为MJPEG格式(5fps),图像大小均为1920x1080

	if( pRoadMonitorH264 )
	{
		printf("videoCodecId=%d,width=%d,height=%d,fFramePS=%f,fBitRate=%f,totalTime=%lld,nowTime=%lld\n",pVideoInfo->videoCodecId,pVideoInfo->nPicWidth,pVideoInfo->nPicHeight,pVideoInfo->fFramePS,pVideoInfo->fBitRate,pVideoInfo->totalTime,pVideoInfo->nowTime);

        #ifdef MONITORLOG
        fprintf(g_pMonitorLog,"videoCodecId=%d,width=%d,height=%d,fFramePS=%f,fBitRate=%f,totalTime=%lld,nowTime=%lld\n",pVideoInfo->videoCodecId,pVideoInfo->nPicWidth,pVideoInfo->nPicHeight,pVideoInfo->fFramePS,pVideoInfo->fBitRate,pVideoInfo->totalTime,pVideoInfo->nowTime);
        fflush(g_pMonitorLog);
        #endif

        if(pVideoInfo)
        {
            if(pVideoInfo->nPicWidth == 1600 ||
               pVideoInfo->nPicWidth == 1920)
            {
                yuv_video_buf info;
                info.width = pVideoInfo->nPicWidth;
                info.height = pVideoInfo->nPicHeight;
                info.nFrameRate = pVideoInfo->fFramePS;
                info.nVideoType = pVideoInfo->videoCodecId;
                pRoadMonitorH264->SetVideoInfo(&info);
            }
			else
			{
				LogError("pVideoInfo->nPicWidth=%d\n",pVideoInfo->nPicWidth);
			}
        }
	}

	return 1;
}

int StartRealStreamStateFunc(void* hHandle,StartStreamState state,string strDes)
{
	if( state == START_SUCCEEDED)
	{
		printf("start real stream successed\n");
        #ifdef MONITORLOG
        fprintf(g_pMonitorLog,"start real stream successed\n");
        fflush(g_pMonitorLog);
        #endif
	}
    else if( state == START_FAILED)
	{
		LogError("start real stream failed\n");
		#ifdef MONITORLOG
        fprintf(g_pMonitorLog,"start real stream failed\n");
        fflush(g_pMonitorLog);
        #endif
	}

	return 1;
}

//历史视频回调函数
int OnGotVodFileInfo(VodFileInfo *pVodFileInfo,unsigned long lCookie)
{
	printf("Recv vod file info\r\n");
	#ifdef MONITORLOG
    fprintf(g_pMonitorLog,"Recv vod file info\r\n");
    fflush(g_pMonitorLog);
    #endif
	return 0;
}
#endif

CRoadMonitorH264::CRoadMonitorH264()
{
    m_bEndCapture = false;
    m_nDeviceId = -1;
    m_nUnitDeviceId = 0;
    m_handle = NULL;

    m_nThreadId = 0;
    m_strFileName = "";
    m_nVideoType = 0;


    m_VideoInfo.width=0;
    m_VideoInfo.height=0;
    m_VideoInfo.ts = 0;

    #ifdef MONITORLOG
    g_pMonitorLog = fopen("MONITORLOG.log","wb");
    printf("=====g_pMonitorLog=%d\n",g_pMonitorLog);
    #endif

    pthread_mutex_init(&m_Frame_Mutex,NULL);
}

CRoadMonitorH264::~CRoadMonitorH264()
{
    #ifdef MONITORLOG
    if(g_pMonitorLog != NULL)
    {
        fclose(g_pMonitorLog);
        g_pMonitorLog = NULL;
    }
    #endif

    #ifdef MONITORVIDEO
    if(g_pMonitorVideo != NULL)
    {
        fclose(g_pMonitorVideo);
        g_pMonitorVideo = NULL;
    }
    #endif

    pthread_mutex_destroy(&m_Frame_Mutex);
}
//打开设备,设备编号
bool CRoadMonitorH264::OpenVideo(int nNo,VOD_FILE_INFO& vod_info,int nVideoType)
{
    std::string strFileName(vod_info.chFilePath);

    m_nVideoType = nVideoType;//0 实时 1 历史 2 本地

    m_nUnitDeviceId = nNo;

    m_vod_info = vod_info;

    m_bEndCapture = false;

    m_strFileName = "";

    m_pts = 0;

	if(access("UseTest.cfg",F_OK) == 0)
	{
		g_nFileID = nNo;
	}
	LogNormal("g_nFileID=%d\n",g_nFileID);

#ifdef MONITORVIDEO
	char fileName[128]={0};
	sprintf(fileName, "new_video%d.avi", nNo);
	g_pMonitorVideo = fopen(fileName,"wb");
#endif
	
	
    LogNormal("m_strFileName.c_str()=%s\n",m_strFileName.c_str());
	LogNormal("nVideoType=%d\n",nVideoType);

    if(nVideoType == 2)/////本地历史视频
    {
        m_nDeviceId = nNo;
        m_strFileName = strFileName;


        //判断文件名称是否已经存在
        if(access(m_strFileName.c_str(),F_OK) != 0)
        {
			LogNormal("文件不存在\n");
            return false;
        }

        #ifdef H264_DECODE
		
		int nRet = m_strFileName.rfind(".264");
		if (nRet > 0)
		{
			yuv_video_buf info;
			info.width = 1920;
			info.height = 1072;
			info.nFrameRate = 25;
			info.nVideoType = 0;
			SetVideoInfo(&info);
			return true;
		}
		else
		{
			nRet = m_strFileName.rfind(".hk");
			if (nRet > 0)
			{
				yuv_video_buf info;
				info.width = 1280;
				info.height = 720;
				info.nFrameRate = 25;
				info.nVideoType = 0;
				SetVideoInfo(&info);
				return true;
			}
			else
			{
				RoadDecode myDecode;
				if(myDecode.InitDecode((char*)m_strFileName.c_str())>0)
				{
					int nWidth,nHeight;
					myDecode.GetVideoSize(nWidth,nHeight);
					int nVideoCodeID;
					myDecode.GetVideoCodeID(nVideoCodeID);
					int nFrameRate;
					myDecode.GetVideoFrameRate(nFrameRate);
					LogNormal("Video FrameRate=%d\n",nFrameRate);
					yuv_video_buf info;
					info.width = nWidth;
					info.height = nHeight;
					info.nFrameRate = nFrameRate;
					info.nVideoType = nVideoCodeID;
					m_nPixelFormat = myDecode.GetPixelFormat();
					//GotVideoInfoCallBackFunc(NULL,&info,(unsigned long)this);
						
					int uFileHeaderSize;
					unsigned char* pFileHeaderInfo = NULL;
					myDecode.GetFileHeaderInfo(&pFileHeaderInfo,uFileHeaderSize);

					printf("pFileHeaderInfo=%lld,uFileHeaderSize=%d\n",pFileHeaderInfo,uFileHeaderSize);

					if(uFileHeaderSize > 0)
					{
						m_Decoder.SetFileHeaderInfo(pFileHeaderInfo,uFileHeaderSize);
					}

					SetVideoInfo(&info);
					
					printf("before myDecode.UinitDecode==============\n");
					myDecode.UinitDecode();
					printf("after myDecode.UinitDecode==============\n");

					return true;
				}
				else
				{
					printf("false myDecode.InitDecode==============\n");
				}
			}
		}
        #endif

        return false;
    }
    else if(nVideoType == 1)//实时视频或远程历史视频
    {
       // m_bEndCapture = false;
//#ifdef FMFS_MONITOR
//		XMLNode nodeFile = XMLNode::parseFile("fmfs.xml");
//		if (nodeFile.isEmpty())
//		{
//			LogTrace("fmfs.log", "file format error!");
//			return;
//		}
//		const char *host = nodeFile.getChildNode("host").getText();
//		const char *chPort = nodeFile.getChildNode("port").getText();
//		int port = atoi(chPort);
//		LogTrace("fmfs.log", "host%s, port=%d", host, port);
//		string strHost=host;
//		if(m_fmfsTel.ConnectWithFmfs(strHost, port))
//		{
//			onDemandCommand cmd;
//			onDemandGetScope scope;
//			cmd.onDemandModeCode = ServerInit;
//			memset(cmd.arrayIP, 0, 20);
//			memcpy(cmd.arrayIP, strHost.c_str(), strHost.size());
//			cmd.recBytes = sizeof(int);
//			LogTrace("fmfs.log", "send msg of initialization server!");
//			return m_fmfsTel.SendToFmfs(cmd, scope);
//		}
//		return false;
//#else
//		return true;
//#endif
		return true;
    }
	else if (nVideoType == 0)//实时视频
	{
		return true;
	}
}
//重新获取视频流
void CRoadMonitorH264::ReOpen()
{
    if(m_nVideoType <= 1)
	{
#ifdef MONITOR
        if(m_handle != NULL)
        {
            if(m_nVideoType == 0)
            {
                LogNormal("重新获取视频流\n");
                int nRet = g_monitor.DoRePullStream(m_handle,0);

				if(nRet <= 0)
				{
					LogError("断流重拉失败\n");
					g_SobeyCommunication.SetLogOnStatus(false);
				}
            }
            else if(m_nVideoType == 1)
			{
#ifdef FMFS_MONITOR
				m_fmfsTel.ReopenVideo();
#else
                if(m_pts > 0)
                {
                   g_monitor.StopVodStream(m_handle);

                   m_handle = g_monitor.StartVodStream(m_strFileName.c_str(),NULL,false,StartRealStreamStateFunc);

                   if(m_handle != NULL)
                   {
                       if(g_monitor.SetGotVodVideoInfoCallBackFunction(m_handle,GotVideoInfoCallBackFunc,(unsigned long)this)>0)
                       {
                            g_monitor.SetVodMediaCallbackFunction(m_handle,AVDataCallBackFunc,(unsigned long)this);

                            g_monitor.SeekToVodTs(m_handle,m_pts,true);
                       }
                   }
                }
#endif
            }
        }
#endif
    }
}

//获取视频编号
void CRoadMonitorH264::GetDeviceId()
{
    #ifdef MONITOR
    char chDeviceCode[256] = {0};
    sprintf(chDeviceCode,"%u",m_nUnitDeviceId);

    const DeviceItem *pDev 	= NULL;
    pDev = g_monitor.GetDeviceByUnitDeviceCode(chDeviceCode);
    if(pDev)
    m_nDeviceId = pDev->lDeviceID;

    printf("====m_nUnitDeviceId=%u,m_nDeviceId=%d\n",m_nUnitDeviceId,m_nDeviceId);
    #ifdef MONITORLOG
    fprintf(g_pMonitorLog,"====m_nUnitDeviceId=%u,m_nDeviceId=%d\n",m_nUnitDeviceId,m_nDeviceId);
    fflush(g_pMonitorLog);
    #endif
    #endif
}

//获取指定相机指定时间段内的文件列表
int CRoadMonitorH264::GetFileCount()
{
#ifdef FMFS_MONITOR
  return 1;
#else
	if(m_nDeviceId < 0 )
		return 0;
    m_mapFileIndex.clear();
    int nFileCount = 0;

    string strBeginTime = GetTime(m_vod_info.uBeginTime);
    string strEndTime = GetTime(m_vod_info.uEndTime);

    char szDevList[256] = {0};
    sprintf(szDevList,"%d",m_nDeviceId);

    #ifdef MONITOR
    g_monitor.GetVodFileList(szDevList,OnGotVodFileInfo,0,strBeginTime.c_str(),strEndTime.c_str());
    sleep(5);
    int nCount = g_monitor.GetVodFileCount();

    for(int nIndex = 0; nIndex< nCount;nIndex++)
    {
        const VodFileInfo * pVodFile = NULL;
        pVodFile = g_monitor.GetVodFileByIndex(nIndex);

        if(pVodFile != NULL)
        {
            String strPath = pVodFile->strFilePath;

            //先判断历史视频文件是否已经被检测过
            //if(!(g_skpDB.IsHistoryVideoDealed(strPath)))
            {
                char buf[64] = {0};

                //获取本段历史视频起始时间
                sprintf(buf,"%4d-%02d-%02d %02d:%02d:%02d",pVodFile->stStartTime.wYear,pVodFile->stStartTime.wMonth,pVodFile->stStartTime.wDay,pVodFile->stStartTime.wHour,pVodFile->stStartTime.wMinute,pVodFile->stStartTime.wSecond);
                std::string strBeginTime(buf);
                int uBeginMiTime = pVodFile->stStartTime.wMilliseconds;

                //获取本段历史视频结束时间
                sprintf(buf,"%4d-%02d-%02d %02d:%02d:%02d",pVodFile->stEndTime.wYear,pVodFile->stEndTime.wMonth,pVodFile->stEndTime.wDay,pVodFile->stEndTime.wHour,pVodFile->stEndTime.wMinute,pVodFile->stEndTime.wSecond);
                std::string strEndTime(buf);
                int uEndMiTime = pVodFile->stEndTime.wMilliseconds;

                g_skpDB.SaveHistoryVideoInfo(m_nUnitDeviceId,strPath,strBeginTime,uBeginMiTime,strEndTime,uEndMiTime,0);

                m_mapFileIndex.insert(FileIndexMap::value_type(nFileCount,nIndex));
                nFileCount++;
            }
        }
    }
    printf("strBeginTime=%s,strEndTime=%s,nCount=%d,nFileCount=%d\n",strBeginTime.c_str(),strEndTime.c_str(),nCount,nFileCount);
    #ifdef MONITORLOG
    fprintf(g_pMonitorLog,"strBeginTime=%s,strEndTime=%s,nCount=%d,nFileCount=%d\n",strBeginTime.c_str(),strEndTime.c_str(),nCount,nFileCount);
    fflush(g_pMonitorLog);
    #endif
    #endif

    return nFileCount;
#endif
}

//启动视频流，如果是远程历史视频则返回当前历史文件的结束时间
int64_t CRoadMonitorH264::PullStream(int nIndex)
{
    int64_t tsEndTime = 0;
    #ifdef MONITOR
        {
            if(m_nVideoType == 0)
            {
                    m_handle = g_monitor.StartRealStream(m_nDeviceId,NULL,StartRealStreamStateFunc);

                    if(m_handle != NULL)
                    {
                        printf("====StartRealStream ok,m_nDeviceId=%d\n",m_nDeviceId);
                        #ifdef MONITORLOG
                        fprintf(g_pMonitorLog,"====StartRealStream ok\n");
                        fflush(g_pMonitorLog);
                        #endif
                        if(g_monitor.SetGotVideoInfoCallBackFunction(m_handle,GotVideoInfoCallBackFunc,(unsigned long)this)>0)
                        {
                            printf("====SetGotVideoInfoCallBackFunction ok\n");
                            g_monitor.SetRealMediaCallbackFunction(m_handle,AVDataCallBackFunc,(unsigned long)this);
                            #ifdef MONITORLOG
                            fprintf(g_pMonitorLog,"====SetGotVideoInfoCallBackFunction ok\n");
                            fflush(g_pMonitorLog);
                            #endif
                        }
                        else
                        {
                            LogError("====SetGotVideoInfoCallBackFunction fail\n");
                            #ifdef MONITORLOG
                            fprintf(g_pMonitorLog,"====SetGotVideoInfoCallBackFunction fail\n");
                            fflush(g_pMonitorLog);
                            #endif
                        }
                    }
                    else
                    {
                        LogError("====StartRealStream fail,m_nDeviceId=%d\n",m_nDeviceId);
                         #ifdef MONITORLOG
                        fprintf(g_pMonitorLog,"====StartRealStream fail\n");
                        fflush(g_pMonitorLog);
                        #endif
						g_SobeyCommunication.SetLogOnStatus(false);
                    }
            }
            else if(m_nVideoType == 1)//历史视频
            {
#ifdef FMFS_MONITOR
				int port = g_ExpoMonitorInfo.uExpoMonitorPort;//借用智能控制器的IP和端口
				string strHost = g_ExpoMonitorInfo.chExpoMonitorHost;
				int nDataPort = GetFmfsPort();
				LogTrace("fmfs.log", "host=%s, port=%d recv data port:%d", strHost.c_str(), port, nDataPort);
				m_fmfsTel.OpenDataServer(nDataPort);//打开数据连接
				
				for(int i = 0; i<10; i++)
				{
					if(m_fmfsTel.ConnectWithFmfs(strHost, port))
					{
						LogTrace(NULL, "connect success!");
						onDemandCommand cmd;
						onDemandGetScope scope;
						memset(&cmd, 0, sizeof(onDemandCommand));
						cmd.onDemandModeCode = SectionVideoForward;
						memcpy(cmd.answerIP, g_ServerHost.c_str(), g_ServerHost.size());
						memcpy(cmd.arrayIP, g_ServerHost.c_str(), g_ServerHost.size());
						cmd.answerPort = nDataPort;
						cmd.camID = m_nUnitDeviceId;
						//cmd.volumeID = 1;
						cmd.startTime = MakeTimeStamp(m_vod_info.uBeginTime);//开始时间
						cmd.endTime = MakeTimeStamp(m_vod_info.uEndTime);//结束时间
						cmd.recBytes = sizeof(onDemandStatus);
						LogTrace("fmfs.log", "send recv video request host=%s, cameraID=%d, st=%s, et=%s", cmd.answerIP, cmd.camID, GetTime(m_vod_info.uBeginTime).c_str(), GetTime(m_vod_info.uEndTime).c_str());

						if(m_fmfsTel.SendToFmfs(cmd, scope))
						{
							yuv_video_buf info;
							info.width = m_fmfsTel.GetWidth();
							info.height = m_fmfsTel.GetHeight();
							info.nVideoType = 1;
							m_VideoInfo = info;
							LogTrace("fmfs.log", "RoadMonitorH264::width=%d, height=%d", m_VideoInfo.width, m_VideoInfo.height);
							//m_fmfsTel.SetVideoInfo(&info);
							break;
						}
						else
						{
							m_fmfsTel.CloseFmfsConnect();
						}
				
					}
					sleep(1);
				}
#else
                FileIndexMap::iterator it  = m_mapFileIndex.find(nIndex);

                int nFileIdx = 0;

                if(it != m_mapFileIndex.end())
                {
                    nFileIdx = it->second;
                }

                const VodFileInfo * pVodFile = NULL;
                pVodFile = g_monitor.GetVodFileByIndex(nFileIdx);

                m_strFileName = "";
                if(pVodFile != NULL)
                {
                    m_handle = g_monitor.StartVodStream(pVodFile->strFilePath.c_str(),NULL,false, );

                    if(m_handle != NULL)
                    {
                        printf("====StartVodStream ok\n");
                        #ifdef MONITORLOG
                        fprintf(g_pMonitorLog,"====StartVodStream ok,strFilePath=%s\n",pVodFile->strFilePath.c_str());
                        fflush(g_pMonitorLog);
                        #endif
                        if(g_monitor.SetGotVodVideoInfoCallBackFunction(m_handle,GotVideoInfoCallBackFunc,(unsigned long)this)>0)
                        {
                            char buf[64] = {0};

                            //获取本段历史视频起始时间
                            sprintf(buf,"%4d-%02d-%02d %02d:%02d:%02d",pVodFile->stStartTime.wYear,pVodFile->stStartTime.wMonth,pVodFile->stStartTime.wDay,pVodFile->stStartTime.wHour,pVodFile->stStartTime.wMinute,pVodFile->stStartTime.wSecond);
                            std::string strBeginTime(buf);
                            unsigned long uBeginTime = MakeTime(strBeginTime);
                            m_VideoInfo.ts = (int64_t)uBeginTime*1000000+pVodFile->stStartTime.wMilliseconds*1000;

                            //获取本段历史视频结束时间
                            sprintf(buf,"%4d-%02d-%02d %02d:%02d:%02d",pVodFile->stEndTime.wYear,pVodFile->stEndTime.wMonth,pVodFile->stEndTime.wDay,pVodFile->stEndTime.wHour,pVodFile->stEndTime.wMinute,pVodFile->stEndTime.wSecond);
                            std::string strEndTime(buf);
                            unsigned long uEndTime = MakeTime(strEndTime);
                            tsEndTime = (int64_t)uEndTime*1000000+pVodFile->stEndTime.wMilliseconds*1000;

                            m_strFileName = pVodFile->strFilePath;

                            printf("====SetGotVodVideoInfoCallBackFunction ok,m_strFileName=%s\n",m_strFileName.c_str());
                            g_monitor.SetVodMediaCallbackFunction(m_handle,AVDataCallBackFunc,(unsigned long)this);
                             #ifdef MONITORLOG
                            fprintf(g_pMonitorLog,"====SetGotVodVideoInfoCallBackFunction ok,strFilePath=%s\n",pVodFile->strFilePath.c_str());
                            fflush(g_pMonitorLog);
                            #endif
                        }
                        else
                        {
                            LogError("====SetGotVodVideoInfoCallBackFunction fail\n");
                             #ifdef MONITORLOG
                            fprintf(g_pMonitorLog,"====SetGotVodVideoInfoCallBackFunction fail,strFilePath=%s\n",pVodFile->strFilePath.c_str());
                            fflush(g_pMonitorLog);
                            #endif
                        }
                    }
                    else
                    {
                        LogError("====StartVodStream fail\n");
                        #ifdef MONITORLOG
                        fprintf(g_pMonitorLog,"====StartVodStream fail,strFilePath=%s\n",pVodFile->strFilePath.c_str());
                        fflush(g_pMonitorLog);
                        #endif
                    }
                }
                else
                {
                   LogError("====GetVodFileInfo fail\n");
                   #ifdef MONITORLOG
                    fprintf(g_pMonitorLog,"====GetVodFileInfo fail\n");
                    fflush(g_pMonitorLog);
                    #endif
                }
#endif
            }
        }

    #endif
    return tsEndTime;
}

//关闭设备
bool CRoadMonitorH264::CloseVideo()
{
    m_bEndCapture = true;
	LogNormal("HCloseVideo 1");
    if(m_nVideoType == 2)
    {
        if(m_nThreadId != 0)
        {
            pthread_join(m_nThreadId,NULL);
            m_nThreadId = 0;
        }
    }
	LogNormal("HCloseVideo 2");
	 //加锁
    pthread_mutex_lock(&m_Frame_Mutex);

	m_listFrame.clear();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);

	LogNormal("HCloseVideo 3");
    #ifdef H264_DECODE
    m_Decoder.UinitDecode();
    #endif
   
	LogNormal("HCloseVideo 4");
	m_VideoInfo.ts = 0;
	m_VideoInfo.width=0;
	m_VideoInfo.height=0;

    m_nDeviceId = -1;
    m_nUnitDeviceId = 0;
    m_handle = NULL;
    m_strFileName = "";
    m_nVideoType = 0;

#ifdef FMFS_MONITOR
	 m_fmfsTel.CloseVideo();
#endif
    return true;
}

//开始采集数据
bool CRoadMonitorH264::StartCapture()
{
    m_bEndCapture = false;

    if(m_nVideoType == 2)
    {
        //线程属性
        pthread_attr_t   attr;
        //初始化
        pthread_attr_init(&attr);
        //分离线程
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
        //启动事件录像线程
        int nRet = pthread_create(&m_nThreadId,&attr,ThreadMonitorCapture,this);

        pthread_attr_destroy(&attr);

        if(nRet!=0)
        return false;
        else
        return true;
    }
    else
    {
        if(m_nUnitDeviceId > 0)
        {
            //sobey返回值有问题
            return true;
        }
        return false;
    }
}

//停止采集数据
bool CRoadMonitorH264::StopCapture(bool bSaveVideoInfo)
{
    m_bEndCapture = true;
    #ifdef MONITOR
    if( m_handle!= NULL)
    {
        if(m_nVideoType == 0)
        {
            if(g_monitor.StopRealStream(m_handle)>0)
            {
                m_handle = NULL;
                return true;
            }

        }
        else if(m_nVideoType == 1)
        {
            if(bSaveVideoInfo)
            g_skpDB.SaveHistoryVideoInfo(m_nUnitDeviceId,m_strFileName,"",0,"",0,1);

			if(g_monitor.StopVodStream(m_handle)>0)
            {
                m_handle = NULL;
                return true;
            }
        }
    }
    #endif
    return false;
}

//添加一帧
void CRoadMonitorH264::AddFrame(unsigned char* pData,int dataLen,long long pts,bool bKeyFrame)
{
    if((pData != NULL) && (dataLen > 0))
    {
        yuv_video_buf buf = m_VideoInfo;

        if(m_nVideoType == 0)//实时视频
        {
            struct timeval tv;
            gettimeofday(&tv,NULL);
            buf.ts = tv.tv_sec*1000000+tv.tv_usec;

            if(pts - m_pts > 40000)
            {
                #ifdef MONITORLOG
                fprintf(g_pMonitorLog,"this=%lld,pts=[%lld],m_pts=%lld,datalen=[%d]\n",this,pts,m_pts,dataLen);
                fflush(g_pMonitorLog);
                #endif
            }

            m_pts = pts;


           /* static int nCount = 0;
            if(bKeyFrame)
            {
                nCount = 0;
            }
            else
            {
                nCount++;
            }

            printf("===+++time=%lld==buf.ts=%lld,dataLen=%d,bKeyFrame=%d,nCount=%d\n",time(NULL),buf.ts,dataLen,bKeyFrame,nCount);

            if(nCount > 12)
            {
                return;
            }*/
        }
        else if(m_nVideoType == 1)//远程历史视频(需要取实际历史时间)
        {
          m_pts = pts;
           //需要对索贝时间戳进行转换
          int64_t absPts = pts;
          absPts 		= absPts/1000;
          int nMilSec 	= absPts%1000; //毫秒
          absPts 		= absPts/1000; //秒

          struct tm *pTm,timenow;
          pTm = &timenow;
          localtime_r((const time_t*)&absPts,pTm);
          pTm->tm_year -= 1900;

          int64_t uTime=  mktime(pTm);

           buf.ts = uTime*1000000+nMilSec*1000;

           printf("===+++==buf.ts=%lld\n",buf.ts);
        }
        else if(m_nVideoType == 2)//本地视频
        {
            buf.ts = pts;
        }

        string strFrame;
        strFrame.append((char*)&buf,sizeof(yuv_video_buf));
        strFrame.append((char*)pData,dataLen);

        //加锁
        pthread_mutex_lock(&m_Frame_Mutex);

        m_listFrame.push_back(strFrame);

         //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);

        usleep(1000*5);

        return;
    }
}

//获取一帧数据
int CRoadMonitorH264::PopFrame(yuv_video_buf& header,unsigned char* response)
{
    int nSize = 0;
	if (m_nVideoType == 1)//历史视频
	{
#ifdef FMFS_MONITOR
		nSize = m_fmfsTel.PopFrame(header, response);
#else
		goto LABEL_POP; 
#endif
	}
	else
	{
LABEL_POP:	string strFrame;

		//加锁
		pthread_mutex_lock(&m_Frame_Mutex);

		if(m_listFrame.size() > 0)
		{
			 //printf("PopFrame m_listFrame.size()=%d\n",m_listFrame.size());
			 #ifdef MONITORLOG
			if(m_listFrame.size()>1000)
			{
				fprintf(g_pMonitorLog,"this=%lld,m_listFrame.size()=%d\n",this,m_listFrame.size());
				fflush(g_pMonitorLog);
			}
			 #endif

			 std::list<std::string>::iterator it = m_listFrame.begin();

			 strFrame = *it;

			 m_listFrame.pop_front();
		}

		//解锁
		pthread_mutex_unlock(&m_Frame_Mutex);

		if(strFrame.size() > 0 &&(!m_bEndCapture))
		{
			 header = *((yuv_video_buf*)(strFrame.c_str()));

			 bool bRet = false;
			 #ifdef H264_DECODE
			bRet = m_Decoder.DecodeFrame((unsigned char*)(strFrame.c_str()+sizeof(yuv_video_buf)),strFrame.size()-sizeof(yuv_video_buf),response,nSize);
			 #endif

			 if(bRet&&nSize>0)
			 {
				 if(m_VideoInfo.nVideoType == 3)//rawvideo
				 {
					 if(m_nPixelFormat == PIX_FMT_YUV420P)
					 {
						 IppiSize srcSize= {m_VideoInfo.width,m_VideoInfo.height};

						 unsigned char *pData = (unsigned char*)(strFrame.c_str()+sizeof(yuv_video_buf));
						 unsigned char *pic[3];
						 pic[0] = pData;
						 pic[1] = (unsigned char*)pData + m_VideoInfo.width*m_VideoInfo.height;
						 pic[2] = (unsigned char*)pData + m_VideoInfo.width*m_VideoInfo.height + m_VideoInfo.width*m_VideoInfo.height/4;

						 ippiYUV420ToRGB_8u_P3C3((const Ipp8u**)pic,
							 response,
							 srcSize);

						 //swap rgb
						 BYTE temp;
						 for (int i=0;i<nSize*2;i+=3)
						 {
							 temp = response[i]; response[i] = response[i+2]; response[i+2] = temp;
						 }
					 }
					 else
					 {
						 CxImage image;
						 image.CreateFromArray(response,m_VideoInfo.width,m_VideoInfo.height,24,m_VideoInfo.width*3,true);
						 memcpy(response,image.GetBits(),nSize);
						 image.Destroy();

						 //swap rgb
						 BYTE temp;
						 for (int i=0;i<nSize;i+=3)
						 {
							 temp = response[i]; response[i] = response[i+2]; response[i+2] = temp;
						 }
					 }
				 }
				 nSize = 1;
			 }
		}
	}
    return nSize;
}


//设置视频信息
void CRoadMonitorH264::SetVideoInfo(yuv_video_buf* pVideoInfo)
{
    if(pVideoInfo != NULL)//重新获取视频流时不需要进行设置
    {
        m_VideoInfo = *pVideoInfo;

        #ifdef H264_DECODE
        m_Decoder.SetVideoSize(pVideoInfo->width,pVideoInfo->height);
        m_Decoder.SetVideoCodeID(pVideoInfo->nVideoType);
        m_Decoder.InitDecode(NULL);
        #endif

       // m_pImageData = new unsigned char[pVideoInfo->nPicWidth*pVideoInfo->nPicHeight*3];
        LogTrace(NULL, "nWidth=%d,nHeight=%d\n",pVideoInfo->width,pVideoInfo->height);
    }
}

//获取视频信息
yuv_video_buf* CRoadMonitorH264::GetVideoInfo()
{
    return &m_VideoInfo;
}

void CRoadMonitorH264::DealMonitorCapture()
{
    uint8_t* pBuffer = new uint8_t[1024*1024*8];
    int nLength = 1024;
    struct timeval tv;
    long long pts_t,pts;
    #ifdef H264_DECODE
    RoadDecode myDecode;
    unsigned int nIndex = 0;
	int dt = 0;
    while((!m_bEndCapture))
	{
		int nRet = m_strFileName.rfind(".264");
		if (nRet > 0)
		{
			unsigned char buf[4] = {0};

			int nReadByte = 0;
			nIndex = 0;

			FILE* fp = fopen64(m_strFileName.c_str(),"rb");

			while((feof(fp)==0) &&(!m_bEndCapture))
			{
				//读数据头
				nReadByte = fread(buf,1,4,fp);
				
				if(nReadByte > 0)
				{
					if((buf[0] == 0xaa) && (buf[1] == 0xbb) &&(buf[2] == 0xcc) &&(buf[3] == 0xdd))
					{
						nReadByte = fread(buf,1,4,fp);
						
						if(nReadByte > 0)
						{
							nLength = *(int*)(buf);

							if(nLength > 0)
							{
								//判断开始字节是否0x0,0x0,0x0,0x1
								nReadByte = fread(buf,1,4,fp);

								if(nReadByte > 0)
								{
									if(buf[0] == 0x0 && buf[1] == 0x0 &&buf[2] == 0x0 &&buf[3] == 0x01)
									{
										fseek(fp,-4,SEEK_CUR);
									}

									nReadByte = fread(pBuffer,1,nLength,fp);
									
									if(nReadByte > 0)
									{
										if(g_nHistoryPlayMode == 0)
										{
											gettimeofday(&tv,NULL);
											pts_t = tv.tv_sec*1000000+tv.tv_usec;
											dt = 1000.0/m_VideoInfo.nFrameRate;
											pts = pts_t ;
										}
										else
										{
											//**************************历史视频时间
											if(nIndex == 0)
											{
												pts_t = (long long)m_vod_info.uBeginTime*1000000;
											}
											else
											{
												pts_t = pts;
											}
											dt = 1000.0/m_VideoInfo.nFrameRate;
											pts = pts_t + dt*1000;
											nIndex++;
											//****************************
											//printf("****************************pts=%lld\n",pts);
										}

										AddFrame(pBuffer,nLength,pts);
										usleep(1000*dt-5);
									}
								}
							}
						}
					}
				}
			}

			if(fp)
			{
				fclose(fp);
			}
		}
		else
		{
			
			nRet = m_strFileName.rfind(".hk");
			if (nRet > 0)
			{
				#ifdef HKCAMERA
				int hr                    = 0;
				int bSuc                  = 0;	
				int lHandle               = -1;
				unsigned int dwBytes      = 0;	
				unsigned int dwVideoFrame = 0;
				unsigned int dwAudioFrame = 0;
				unsigned long dwBufSize   = 0;
				void* hAnalyze            = NULL;
				FILE* hFile               = NULL;
				unsigned char* pDataBuf   = NULL;
				unsigned char pBuf[40]    = {0};
				PACKET_INFO_EX stInfoEx   = {0};

				if (NULL == pDataBuf)
				{
					pDataBuf = (unsigned char*)malloc(BUFFER_SIZE);
				}

				hFile = fopen64(m_strFileName.c_str(),"rb");
				if(hFile)
				{
					dwBytes = fread(pBuf, 1, 40, hFile);

					if (dwBytes == 40)
					{
						fseek(hFile, 0, SEEK_SET);

						hAnalyze = HIKANA_CreateStreamEx(dwBufSize, pBuf);

						if (hAnalyze != NULL)
						{
							while((feof(hFile)==0) &&(!m_bEndCapture))
							{
								dwBytes = fread(pDataBuf, 1, BUFFER_SIZE, hFile);

								if (dwBytes == 0) 
								{
									break;
								}

								bSuc = HIKANA_InputData(hAnalyze, pDataBuf, dwBytes);
								if (1 != bSuc)
								{
									break;
								}

								while (!m_bEndCapture)
								{
									hr = HIKANA_GetOnePacketEx(hAnalyze, &stInfoEx);
									if (0 == hr)
									{
										if ((VIDEO_I_FRAME == stInfoEx.nPacketType) || 
											(VIDEO_P_FRAME == stInfoEx.nPacketType) || 
											(VIDEO_B_FRAME == stInfoEx.nPacketType))
										{
											//get video packet
											dwVideoFrame++;
											if (VIDEO_I_FRAME == stInfoEx.nPacketType)
											{
												//get video resolution
												int width  = stInfoEx.uWidth;
												int heigth = stInfoEx.uHeight;
											}
											//printf("nPacketType=%d,uWidth=%d,dwPacketSize=%d\n",stInfoEx.nPacketType,stInfoEx.uWidth,stInfoEx.dwPacketSize);

											if(g_nHistoryPlayMode == 0)
											{
												gettimeofday(&tv,NULL);
												pts_t = tv.tv_sec*1000000+tv.tv_usec;
												dt = 1000.0/m_VideoInfo.nFrameRate;
												pts = pts_t ;
											}
											else
											{
												//**************************历史视频时间
												if(nIndex == 0)
												{
													pts_t = (long long)m_vod_info.uBeginTime*1000000;
												}
												else
												{
													pts_t = pts;
												}
												dt = 1000.0/m_VideoInfo.nFrameRate;
												pts = pts_t + dt*1000;
												nIndex++;
												//****************************
												//printf("****************************pts=%lld\n",pts);
											}

											AddFrame(stInfoEx.pPacketBuffer,stInfoEx.dwPacketSize,pts);
											usleep(1000*dt-5);
										}
										else if (AUDIO_PACKET == stInfoEx.nPacketType)
										{
											// get audio packet
											dwAudioFrame++;
										}
										else if (FILE_HEAD == stInfoEx.nPacketType)
										{
											// get media head
											printf("get media head nPacketType=%d\n",stInfoEx.nPacketType);
										}
									}
									else
									{
										break;
									}
								}

							}
						}
					}
				}

				if (NULL == hAnalyze)
				{
					HIKANA_Destroy(hAnalyze);
					hAnalyze = NULL;
				}

				if (NULL != hFile)
				{
					fclose(hFile);
					hFile = NULL;
				}
				if (NULL != pDataBuf)
				{
					free(pDataBuf);
					pDataBuf = NULL;
				}
				#endif
			}
			else
			{
				if(myDecode.InitDecode((char*)m_strFileName.c_str())>0)
				{
					nIndex = 0;
					printf("StartDecode true\n");
					while(myDecode.GetNextFrame(pBuffer, nLength)&&(!m_bEndCapture))
					{
						//printf("%x-%x-%x-%x-%x\n",pBuffer[0],pBuffer[1],pBuffer[2],pBuffer[3],pBuffer[4]);
						////////////////////////正常播放
						if(g_nHistoryPlayMode == 0)
						{
							gettimeofday(&tv,NULL);
							pts_t = tv.tv_sec*1000000+tv.tv_usec;

							dt = 1000.0/m_VideoInfo.nFrameRate;
							pts = pts_t ;

							AddFrame(pBuffer,nLength,pts);
							usleep(1000*dt-5);
							nIndex++;
						}
						///////////////////快速播放历史视频文件
						else if(g_nHistoryPlayMode == 1)
						{
							if(nIndex == 0)
							{
								gettimeofday(&tv,NULL);
								pts_t = tv.tv_sec*1000000+tv.tv_usec;
							}
							pts = pts_t + nIndex*(1000000.0/m_VideoInfo.nFrameRate);

							AddFrame(pBuffer,nLength,pts);
							usleep(1000*30);
							nIndex++;
						}
						else if(g_nHistoryPlayMode == 2)//历史播放模式
						{
							if(nIndex == 0)
							{
								pts_t = (long long)m_vod_info.uBeginTime*1000000;
							}
							else
							{
								pts_t = pts;
							}
							dt = 1000.0/m_VideoInfo.nFrameRate;
							pts = pts_t + dt*1000;
							nIndex++;
							AddFrame(pBuffer,nLength,pts);
							usleep(1000*dt-5);
						}
						/////////////////////////////
					}
				}
				myDecode.UinitDecode();
			}
		}
		 //加锁
		/*pthread_mutex_lock(&m_Frame_Mutex);

		m_listFrame.clear();
		//解锁
		pthread_mutex_unlock(&m_Frame_Mutex);
		break;*/
    }
    #endif

    LogNormal("end StartDecode\n");

    delete []pBuffer;
}

//转换为FMFS协议中的日期
TimeStamp_t CRoadMonitorH264::MakeTimeStamp(unsigned int uTime)
{
	struct tm *local, newTime;
	TimeStamp_t t;
	long lTimestamp = uTime;
	local = &newTime;
	localtime_r((time_t *)&lTimestamp, local);
   t.year = local->tm_year + 1900;
   t.month = local->tm_mon + 1;
   t.day = local->tm_mday;
   t.hour = local->tm_hour;
   t.minute = local->tm_min;
   t.second = local->tm_sec;
   return t;
}
