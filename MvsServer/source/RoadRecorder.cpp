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

#include "RoadRecorder.h"
#include "Common.h"
#include "CommonHeader.h"
#include "cv.h"
#include "highgui.h"
#include "ximage.h"
#include "FileManage.h"
#include "CenterServerOneDotEight.h"
//h264转换成Avi录像
//#include "Avc2Avi.h"
#include "AviFile.h"

//#define DEBUG_VIDEORECORDER


//录象线程
void* ThreadRecorder(void* pArg)
{
	//取类指针
	CSkpRoadRecorder* pRecorderEntity = (CSkpRoadRecorder*)pArg;
	if(pRecorderEntity == NULL) return pArg;

    printf("======before======DealRecorder\n");
	pRecorderEntity->DealRecorder();
	printf("==========after====DealRecorder\n");

	pthread_exit((void*)0);

	return pArg;
}

//构造
CSkpRoadRecorder::CSkpRoadRecorder()
{
	//检测帧列表互斥
	pthread_mutex_init(&m_Frame_Mutex,NULL);
	//事件消息列表互斥
	pthread_mutex_init(&m_Event_Mutex,NULL);


    //线程结束标志
	m_bEndRecorder = false;
	//事件录象长度
	m_nCaptureTime = 5;
	m_nEventCount = 0;

	m_nCamType = 0;
}

//析构
CSkpRoadRecorder::~CSkpRoadRecorder()
{
	//检测帧列表互斥
	pthread_mutex_destroy(&m_Frame_Mutex);
	//事件消息列表互斥
	pthread_mutex_destroy(&m_Event_Mutex);
}
//添加一帧数据
bool CSkpRoadRecorder::AddFrame(std::string& frame)
{	
	if(g_nDetectMode == 2)
	{
		return false;
	}

    if(m_bEndRecorder)
    return false;

	//加锁
	pthread_mutex_lock(&m_Frame_Mutex);

	SRIP_DETECT_HEADER* sDetectHeader1,*sDetectHeader2;
	sDetectHeader1 = (SRIP_DETECT_HEADER*)(frame.c_str());


    if(m_FrameList.size()>0)
	{
		RecorderFrame::iterator it_b = m_FrameList.begin();
		sDetectHeader2 = (SRIP_DETECT_HEADER*)(it_b->c_str());

        //超过缓冲区最大缓冲时间则去除首帧数据
		//if( sDetectHeader1->uTimestamp >= sDetectHeader2->uTimestamp+5)//最多缓冲5秒
		if( m_FrameList.size() >= 50)
		{
		    LogTrace("AddVtsFrame.log", "==uChannelID=%d=uSeq=%d=nFrameSize=%d,uTimestamp=%lld, =m_EventList.size()=%d\r\n",sDetectHeader1->uChannelID, sDetectHeader2->uSeq, frame.size() - sizeof(SRIP_DETECT_HEADER), sDetectHeader2->uTimestamp, m_EventList.size());

			#ifdef DEBUG_VIDEORECORDER
		   printf("====uChannelID=%d===========exceed the max buffer=%d\r\n",m_FrameList.size(),sDetectHeader1->uChannelID);
			#endif
		   m_FrameList.pop_front();
		   //LogNormal("exceed frame buffer\n");
		}
	}
	//添加到图片列表
	m_FrameList.push_back(frame);
	#ifdef DEBUG_VIDEORECORDER
	printf("=========time=%d===add one video frame ok==========\r\n",sDetectHeader1->uTimestamp);
	#endif

	//解锁
	pthread_mutex_unlock(&m_Frame_Mutex);
	return true;
}

//弹出一帧图片
std::string CSkpRoadRecorder::PopFrame(unsigned int uTimestamp,int64_t uTime64)
{
	std::string response;
	//加锁
	pthread_mutex_lock(&m_Frame_Mutex);

	//根据相机类型调用
	{
		//判断是否有图片
		if(m_FrameList.size() > 0)
		{
			RecorderFrame::iterator it_b = m_FrameList.begin();
			RecorderFrame::iterator it_e = m_FrameList.end();
			while(it_b!=it_e && (!m_bEndRecorder))
			{
				SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(it_b->c_str());
				if((sDetectHeader->uTimestamp+5 >= uTimestamp))
				{
					if(sDetectHeader->uTime64 > uTime64)
					{
						//保存数据
						response = *it_b;

						//m_FrameList.pop_front(); //yuwx
						break;
					}
				}
				it_b++;
			}
		}
	}
	//解锁
	pthread_mutex_unlock(&m_Frame_Mutex);
	return response;
}

//处理录象
void CSkpRoadRecorder::DealRecorder()
{
    std::string strEvent = PopEvent();

    if(strEvent.size() > 0 && (!m_bEndRecorder))
    {
        if(g_nEncodeFormat == 0)
        {
            DealJpgToAviRecorder(strEvent);
        }
		else if(g_nEncodeFormat == 3)
        {
            DealMJpegRecorder(strEvent);
        }
        else 
        {
			//根据相机类型调用			
			if(g_nDetectMode == 2)
			{
				if(g_nEncodeFormat == 1)
				{
					DealDspH264Recorder(strEvent);//违章录像存裸h264流
				}
				else
				{
					DealAviRecorder(strEvent);//存avi格式文件
				}
			}
			else
			{
				DealH264Recorder(strEvent);
			}
        }
    }
	m_nEventCount--;
}

//录像格式为JpgAvi-bocom
void CSkpRoadRecorder::DealJpgToAviRecorder(std::string& event)
{
    SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());
	std::string strPath = g_FileManage.GetEventVideoPath(sDetectHeader1->uVideoId);

    //是否录象完毕
	bool bEndCoding = false;
    CJpgToAvi	m_JpgToAvi;
     //开始录像
    if(!m_JpgToAvi.IsEncoding())
    {
        m_JpgToAvi.OpenFile(strPath.c_str());
    }

    int64_t		 uTime64 = 0;

    while( !m_bEndRecorder && !bEndCoding)
    {
        std::string frame = PopFrame(sDetectHeader1->uTimestamp,uTime64);
        if(frame.size()>0)
		{
			//图片头
			SRIP_DETECT_HEADER* sDetectHeader2 = (SRIP_DETECT_HEADER*)(frame.c_str());
			uTime64 = sDetectHeader2->uTime64;

			if(sDetectHeader2->uTimestamp < sDetectHeader1->uTimestamp+m_nCaptureTime-5)
			{
			    //获取图片数据区
				unsigned char* chImageData = (unsigned char*)(frame.c_str());
                if(m_JpgToAvi.IsEncoding())
                {
                    m_JpgToAvi.AddFrame(chImageData,frame.size()-sizeof(SRIP_DETECT_HEADER),event);
                }
			}
			else
			{
			    if(m_JpgToAvi.IsEncoding())
                {
                    if(g_nAviHeaderEx == 1)
                    {
                        string strEvents("");
                        strEvents.append(event.c_str()+sizeof(SRIP_DETECT_HEADER), event.size()-sizeof(SRIP_DETECT_HEADER));
                        m_JpgToAvi.AddBigPics(strEvents);
                    }
                    m_JpgToAvi.CloseFile();
                    bEndCoding = true;
                }
			}
		}
        usleep(1000*1);//等待
    }
    if(!bEndCoding)
	{
        if(m_JpgToAvi.IsEncoding())
        {
            if(g_nAviHeaderEx == 1)
            {
                string strEvents("");
                strEvents.append(event.c_str()+sizeof(SRIP_DETECT_HEADER), event.size()-sizeof(SRIP_DETECT_HEADER));
                m_JpgToAvi.AddBigPics(strEvents);
            }
            m_JpgToAvi.CloseFile();
            bEndCoding = true;
        }
	}
}

//录像格式为MJpeg
void CSkpRoadRecorder::DealMJpegRecorder(std::string& event)
{
    SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());
	std::string strPath = g_FileManage.GetEventVideoPath(sDetectHeader1->uVideoId);

    //是否录象完毕
	bool bEndCoding = false;
    CMJpegAvi  m_MJpegAvi;
    if(sDetectHeader1->uWidth>2000)
    {
        m_MJpegAvi.SetWidth(sDetectHeader1->uWidth/6);
        m_MJpegAvi.SetHeight(sDetectHeader1->uHeight/6);
    }
    else if(sDetectHeader1->uWidth>1000)
    {
        m_MJpegAvi.SetWidth(sDetectHeader1->uWidth/4);
        m_MJpegAvi.SetHeight(sDetectHeader1->uHeight/4);
    }
    else
    {
        m_MJpegAvi.SetWidth(sDetectHeader1->uWidth/2);
        m_MJpegAvi.SetHeight(sDetectHeader1->uHeight/2);
    }

    //开始录像
    if(!m_MJpegAvi.IsEncoding())
    {
        m_MJpegAvi.OpenFile(strPath.c_str());
    }

    int64_t		 uTime64 = 0;

    while( !m_bEndRecorder && !bEndCoding)
    {
        std::string frame = PopFrame(sDetectHeader1->uTimestamp,uTime64);
        if(frame.size()>0)
		{
			//图片头
			SRIP_DETECT_HEADER* sDetectHeader2 = (SRIP_DETECT_HEADER*)(frame.c_str());
			uTime64 = sDetectHeader2->uTime64;

			if(sDetectHeader2->uTimestamp < sDetectHeader1->uTimestamp+m_nCaptureTime-5)
			{
                    //获取图片数据区
                    unsigned char* chImageData = (unsigned char*)(frame.c_str());
                    if(m_MJpegAvi.IsEncoding())
                    {
                        int nOffset = sizeof(SRIP_DETECT_HEADER);
                        //叠加圆圈//只叠加在一帧图片上
                        if(sDetectHeader2->uTime64 == sDetectHeader1->uTime64)
                        {
                            CxImage image;
                            image.Decode(chImageData+nOffset,frame.size()-nOffset,3);//先解码
                            IplImage* img = cvCreateImageHeader(cvSize(image.GetWidth(),image.GetHeight()),8,3);
                            cvSetData(img,image.GetBits(),img->widthStep);
                            CvScalar color = CV_RGB(0,0,255);
                            CvPoint pt;
                            int nSize =  (event.size()-nOffset)/(sizeof(RECORD_EVENT));
                            for(int i = 0; i < nSize; i ++)
                            {
                                RECORD_EVENT re_event;
                                memcpy(&re_event,event.c_str()+nOffset+i*sizeof(RECORD_EVENT),sizeof(RECORD_EVENT));
                                pt.x = re_event.uPosX;
                                pt.y = re_event.uPosY;
                                //printf("=====================void CSkpRoadRecorder::DealRecorder()==============================pt.x=%d,pt.y=%d\n",pt.x,pt.y);
                                cvCircle(img,pt,10,color,2,CV_AA,0);
                            }
                            int srcstep = 0;
                            BYTE* pJpgImage = new BYTE[image.GetWidth()*image.GetHeight()];
                            if(image.IppEncode((unsigned char*)(img->imageData),image.GetWidth(),image.GetHeight(),3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality))
                            {
                                m_MJpegAvi.AddFrame((unsigned char*)pJpgImage,srcstep);
                            }
                            if(pJpgImage)
                            {
                                delete []pJpgImage;
                            }
                            cvReleaseImageHeader(&img);
                        }
                        else
                        {
                            m_MJpegAvi.AddFrame(chImageData+nOffset,frame.size()-nOffset);
                        }
                    }
			}
			else
			{
                 if(m_MJpegAvi.IsEncoding())
                {
                    m_MJpegAvi.CloseFile();
					if(sDetectHeader1->uDetectType == SRIP_VIOLATION_VIDEO)
					{
						g_skpDB.VideoSaveUpdate(m_MJpegAvi.GetEncodingFileName(),sDetectHeader1->uChannelID,2);
					}
					else
					{
						g_skpDB.VideoSaveUpdate(m_MJpegAvi.GetEncodingFileName(),sDetectHeader1->uChannelID);
					}
                    bEndCoding = true;
                }
			}
		}
		usleep(1000*1);//等待
    }
    if(!bEndCoding)
	{
        if(m_MJpegAvi.IsEncoding())
        {
            m_MJpegAvi.CloseFile();
			if(sDetectHeader1->uDetectType == SRIP_VIOLATION_VIDEO)
			{
				g_skpDB.VideoSaveUpdate(m_MJpegAvi.GetEncodingFileName(),sDetectHeader1->uChannelID,2);
			}
			else
			{
				g_skpDB.VideoSaveUpdate(m_MJpegAvi.GetEncodingFileName(),sDetectHeader1->uChannelID);
			}
            bEndCoding = true;
        }
	}
}

//录像格式为MP4-H264
void CSkpRoadRecorder::DealH264Recorder(std::string& event)
{
    #ifdef H264_ENCODE
	//事件消息头
	SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());
	
	 int nOffset = sizeof(SRIP_DETECT_HEADER);

	RECORD_EVENT re_event;
	int nSize =  (event.size()-nOffset)/(sizeof(RECORD_EVENT));
	if(nSize > 0)
	{
		memcpy(&re_event,event.c_str()+nOffset,sizeof(RECORD_EVENT));
	}
	std::string strVideoPath(re_event.chVideoPath);

	printf("DealDspH264Recorder strVideoPath=%s\n",strVideoPath.c_str());

	std::string strVideoFileName = strVideoPath;
	String strTmpPath = "ftp://"+g_ServerHost;
	strVideoFileName.erase(0,strTmpPath.size());
	if(g_nServerType == 13 && g_nFtpServer == 1)
	{
		string strDataPath = "/home/road/dzjc";
		if (IsDataDisk() )
		{
			strDataPath = "/detectdata/dzjc";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else if(g_nServerType == 7)
	{
		string strDataPath = "/home/road/red";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/red";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else
	{
		strVideoFileName = g_strVideo + strVideoFileName;
	}
	printf("DealDspH264Recorder strVideoFileName=%s\n",strVideoFileName.c_str());

    bool bEndCoding = false;
	bool bIsWriteFrame = false;//是否通过H264编码写文件

   
    int nPicSize = sDetectHeader1->uWidth*sDetectHeader1->uHeight*3;
    
	string strPlace;
	strPlace = g_skpDB.GetPlace(sDetectHeader1->uChannelID);

	string strDirection;
	int nDirection = g_skpDB.GetDirection(sDetectHeader1->uChannelID);
	strDirection =  GetDirection(nDirection);

	//图像文本信息
    CvxText cvText;
	cvText.Init(25);
	IplImage* img = NULL;

    RoadEncode m_H264Encode;
    m_H264Encode.SetFrameRate(sDetectHeader1->dFrameRate);
    if(sDetectHeader1->uWidth > 1000)
    {
        m_H264Encode.SetVideoSize(g_nVideoWidth,g_nVideoHeight,1);
		img = cvCreateImageHeader(cvSize(g_nVideoWidth,g_nVideoHeight),8,3);
    }
    else
    {
        m_H264Encode.SetVideoSize(sDetectHeader1->uWidth,sDetectHeader1->uHeight,1);
		img = cvCreateImageHeader(cvSize(sDetectHeader1->uWidth,sDetectHeader1->uHeight),8,3);
    }
    printf("w=%d,h=%d,fr=%d\n",sDetectHeader1->uWidth,sDetectHeader1->uHeight,sDetectHeader1->dFrameRate);

    if(m_H264Encode.InitEncode((char*)strVideoFileName.c_str()) > 0)
    {
        int64_t		 uTime64 = 0;
        bool bExistEventFrame = false;//是否存在事件帧，如果不存在则将事件帧插入

        while( !m_bEndRecorder && !bEndCoding)
        {
            std::string frame = PopFrame(sDetectHeader1->uTimestamp,uTime64);
            if(frame.size()>0)
            {
                //图片头
                SRIP_DETECT_HEADER* sDetectHeader2 = (SRIP_DETECT_HEADER*)(frame.c_str());
                uTime64 = sDetectHeader2->uTime64;

                if(sDetectHeader2->uTimestamp < sDetectHeader1->uTimestamp+m_nCaptureTime-5)
                {
                    if(!bExistEventFrame)//判断是否需要插入事件帧
                    {
                        if(sDetectHeader2->uTime64 >= sDetectHeader1->uTime64)
                        {
                            bExistEventFrame = true;   
							cvSetData(img,(unsigned char*)(frame.c_str()+nOffset),img->widthStep);
                        }

                        if(bExistEventFrame)
                        {
                            CvScalar color = CV_RGB(0,0,255);
                            CvPoint pt;

                            float fscaleX = 1.0;
                            float fscaleY = 1.0;

                            if(sDetectHeader1->uWidth>2000)
                            {
                                fscaleX = g_nVideoWidth/(sDetectHeader1->uWidth/6.0);
                                fscaleY = g_nVideoHeight/(sDetectHeader1->uHeight/6.0);
                            }
                            else if(sDetectHeader1->uWidth>1000)
                            {
                                fscaleX = g_nVideoWidth/(sDetectHeader1->uWidth/4.0);
                                fscaleY = g_nVideoHeight/(sDetectHeader1->uHeight/4.0);
                            }
                            else
                            {
                                fscaleX = 2.0;
                                fscaleY = 2.0;
                            }

							if(sDetectHeader1->uDetectType == SRIP_VIOLATION_VIDEO)
							{
								nPicSize = 0;
							}
							
							int nSize =  (event.size()-nOffset-nPicSize)/(sizeof(RECORD_EVENT));
							if(nSize > 0)
							{
								for(int i = 0; i < nSize; i ++)
								{
									RECORD_EVENT re_event;
									memcpy(&re_event,event.c_str()+nOffset+i*sizeof(RECORD_EVENT),sizeof(RECORD_EVENT));

									pt.x = re_event.uPosX*fscaleX;
									pt.y = re_event.uPosY*fscaleY;

									cvCircle(img,pt,10,color,2,CV_AA,0);
								}
							}
							
							//编码前叠加文字
							PutTextOnVideo(img,sDetectHeader2,&cvText,strPlace,strDirection);
                            m_H264Encode.WriteFrame((unsigned char*)img->imageData,img->imageSize,m_pEncodeData,m_nEncodeSize);
                            bIsWriteFrame = true;
                        }

                    }

                    if(sDetectHeader2->uTime64 != sDetectHeader1->uTime64)
                    {
                        //获取图片数据区
                        unsigned char* chImageData = (unsigned char*)(frame.c_str());
						cvSetData(img,(unsigned char*)(frame.c_str()+nOffset),img->widthStep);
						PutTextOnVideo(img,sDetectHeader2,&cvText,strPlace,strDirection);
                        m_H264Encode.WriteFrame(chImageData+nOffset,frame.size()-nOffset,m_pEncodeData,m_nEncodeSize);
						bIsWriteFrame = true;
                    }
                }
                else
                {
					if (!bIsWriteFrame)//如果没有写入
					{
						string frame;//为防止H264编解码释放资源时出错
						pthread_mutex_lock(&m_Frame_Mutex);
						frame = *m_FrameList.begin();
						pthread_mutex_unlock(&m_Frame_Mutex);
						unsigned char* chImageData = (unsigned char*)(frame.c_str());
						cvSetData(img,(unsigned char*)(frame.c_str()+nOffset),img->widthStep);
						PutTextOnVideo(img,sDetectHeader2,&cvText,strPlace,strDirection);
						m_H264Encode.WriteFrame(chImageData+nOffset,frame.size()-nOffset,m_pEncodeData,m_nEncodeSize);
						bIsWriteFrame = true;
					}
                    m_H264Encode.UinitEncode();
					if(sDetectHeader1->uDetectType == SRIP_VIOLATION_VIDEO)
					{
						g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID,2);
					}
					else if (SRIP_EVENT_PLATE_VIDEO == sDetectHeader1->uDetectType)
					{
                        g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID);
						g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID,2);
					}
					else
					{
						g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID);
					}
                    bEndCoding = true;
                }
            }
            usleep(1000*1);//等待
        }
    }

    if(!bEndCoding)
	{
		if (!bIsWriteFrame)//如果没有写入
		{
			string frame;//为防止H264编解码释放资源时出错
			pthread_mutex_lock(&m_Frame_Mutex);
			frame = *m_FrameList.begin();
			pthread_mutex_unlock(&m_Frame_Mutex);
			unsigned char* chImageData = (unsigned char*)(frame.c_str());
			SRIP_DETECT_HEADER* sDetectHeader2 = (SRIP_DETECT_HEADER*)(frame.c_str());
			cvSetData(img,(unsigned char*)(frame.c_str()+nOffset),img->widthStep);
			PutTextOnVideo(img,sDetectHeader2,&cvText,strPlace,strDirection);
			m_H264Encode.WriteFrame(chImageData+nOffset,frame.size()-nOffset,m_pEncodeData,m_nEncodeSize);
		}
        m_H264Encode.UinitEncode();
		if(sDetectHeader1->uDetectType == SRIP_VIOLATION_VIDEO)
		{
			g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID,2);
		}
		else
		{
			g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID);
		}
        bEndCoding = true;
	}

	if(img != NULL)
	{
		cvReleaseImageHeader(&img);
	}
	cvText.UnInit();
    #endif
}

//启动事件录像线程
bool CSkpRoadRecorder::BeginRecorder()
{
    pthread_t Id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动事件录像线程
	int nret=pthread_create(&Id,&attr,ThreadRecorder,this);

	printf("pthread_create(&Id,&attr,ThreadRecorder,this)=nret=%d \n",nret);
	//成功
	if(nret!=0)
	{
	    pthread_attr_destroy(&attr);
		//失败
		LogError("创建事件录象线程失败,无法进行录象！\r\n");
		return false;
	}
	pthread_attr_destroy(&attr);
	m_nEventCount++;
	return true;
}

//初始化
bool CSkpRoadRecorder::Init()
{
    //线程结束标志
	m_bEndRecorder = false;
	m_nEventCount = 0;

	m_nEncodeSize = g_nVideoWidth*g_nVideoHeight*3;
    m_pEncodeData = new unsigned char[m_nEncodeSize];

	//加锁
	pthread_mutex_lock(&m_Frame_Mutex);
	//释放录象缓冲区
    m_FrameList.clear();
	//解锁
	pthread_mutex_unlock(&m_Frame_Mutex);

	//加锁
	pthread_mutex_lock(&m_Event_Mutex);
    m_EventList.clear();
	//解锁
	pthread_mutex_unlock(&m_Event_Mutex);

	return true;
}

//
bool CSkpRoadRecorder::UnInit()
{
    //线程结束标志
	m_bEndRecorder = true;

	sleep(1);

	//加锁
	pthread_mutex_lock(&m_Frame_Mutex);
	//释放录象缓冲区
    m_FrameList.clear();
	//解锁
	pthread_mutex_unlock(&m_Frame_Mutex);

	//加锁
	pthread_mutex_lock(&m_Event_Mutex);
    m_EventList.clear();
	//解锁
	pthread_mutex_unlock(&m_Event_Mutex);

	if(m_pEncodeData!=NULL)
    {
        delete []m_pEncodeData;
        m_pEncodeData = NULL;
    }

    m_nEventCount = 0;

    return true;
}

//添加事件消息
bool CSkpRoadRecorder::AddEvent(std::string& event)
{
    if(event.size() > 0 && (!m_bEndRecorder))
    {
		SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());
        //加锁
        pthread_mutex_lock(&m_Event_Mutex);
        //添加到事件消息列表
        m_EventList.push_back(event);
        //解锁
        pthread_mutex_unlock(&m_Event_Mutex);

        //启动录象线程
        return BeginRecorder();
    }
    return false;
}

//弹出一条检测结果
std::string CSkpRoadRecorder::PopEvent()
{
	std::string response;
	//加锁
	pthread_mutex_lock(&m_Event_Mutex);
	//判断是否有命令
	if(m_EventList.size() <= 0)
	{
		//解锁
		pthread_mutex_unlock(&m_Event_Mutex);
		return response;
	}
	//取最早命令
	EventMsg::iterator it = m_EventList.begin();
	//保存数据
	response = *it;
	//删除取出的命令
	m_EventList.pop_front();
	//解锁
	pthread_mutex_unlock(&m_Event_Mutex);
	return response;
}

//添加一帧h264数据
bool CSkpRoadRecorder::AddH264Frame(const char *bufFrame, 
									const unsigned int nFrameSize, 
									SRIP_DETECT_HEADER sDetectHeader)
{
	if(m_bEndRecorder)
		return false;

	std::string frame;
	frame.append((char*)&sDetectHeader,sizeof(sDetectHeader));
	frame.append(bufFrame, nFrameSize);

	//加锁
	pthread_mutex_lock(&m_Frame_Mutex);

	if(m_FrameList.size()>0)
	{
		//超过缓冲区最大缓冲时间则去除首帧数据
		if( m_FrameList.size() >= 200)
		{	
			m_FrameList.pop_front();
		}
	}	

	//添加到图片列表
	m_FrameList.push_back(frame);

	//解锁
	pthread_mutex_unlock(&m_Frame_Mutex);

	return true;
}

//录像格式为MP4-H264
void CSkpRoadRecorder::DealDspH264Recorder(std::string& event)
{
	printf("===in=DealDspH264Recorder==\n");
	
	int nOffset = sizeof(SRIP_DETECT_HEADER);
	//事件消息头
	SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());

	RECORD_EVENT re_event;
	int nSize =  (event.size()-nOffset)/(sizeof(RECORD_EVENT));
	if(nSize > 0)
	{
		memcpy(&re_event,event.c_str()+nOffset,sizeof(RECORD_EVENT));
	}
	std::string strVideoPath(re_event.chVideoPath);

	printf("DealDspH264Recorder strVideoPath=%s\n",strVideoPath.c_str());

	std::string strVideoFileName = strVideoPath;
	String strTmpPath = "ftp://"+g_ServerHost;
	strVideoFileName.erase(0,strTmpPath.size());
	if(g_nServerType == 13 && g_nFtpServer == 1)
	{
		string strDataPath = "/home/road/dzjc";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/dzjc";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else if(g_nServerType == 7)
	{
		string strDataPath = "/home/road/red";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/red";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else
	{
		strVideoFileName = g_strVideo + strVideoFileName;
	}

	printf("DealDspH264Recorder strVideoFileName=%s\n",strVideoFileName.c_str());
	
	if(access(strVideoFileName.c_str(),F_OK) == 0)
	{
		remove(strVideoFileName.c_str());
	}

	bool bEndCoding = false;

	int nFrameSize = 0;
	bool bIsWriteFrame = false;

	printf("w=%d,h=%d,fr=%d\n",sDetectHeader1->uWidth,sDetectHeader1->uHeight,sDetectHeader1->dFrameRate);

	int64_t uTime64 = 0;
	FILE *fpOut = NULL;
	fpOut = fopen(strVideoFileName.c_str(), "wb");
	SRIP_DETECT_HEADER* sDetectHeader2 = NULL;
	while( !m_bEndRecorder && !bEndCoding)
	{
		bIsWriteFrame = false;

		std::string frame = PopFrame(sDetectHeader1->uTimestamp,uTime64);
		if(frame.size() > 0)
		{
			//图片头
			sDetectHeader2 = (SRIP_DETECT_HEADER*)(frame.c_str());
			nFrameSize = frame.size() - sizeof(SRIP_DETECT_HEADER);
			uTime64 = sDetectHeader2->uTime64;		

			if(sDetectHeader2->uTimestamp < sDetectHeader1->uTimestamp + m_nCaptureTime - 5)
			{
				//if(sDetectHeader2->uTime64 != sDetectHeader1->uTime64)
				{
					if (!bIsWriteFrame)//如果没有写入
					{
						//存信息
						if(fpOut != NULL)
						{
							LogTrace("DealDspH264Recorder.log", \
								"==yyyywrite==111=1==nFrameSize=%d==\n", nFrameSize);

							fwrite(frame.c_str()+sizeof(SRIP_DETECT_HEADER), 1, nFrameSize, fpOut);
							bIsWriteFrame = true;
						}
					}
				}
			}
			else
			{
				
				if(sDetectHeader1->uDetectType == SRIP_VIOLATION_VIDEO)
				{
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID,2);
				}
				else if (SRIP_EVENT_PLATE_VIDEO == sDetectHeader1->uDetectType)
				{
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID);
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID,2);
				}
				else
				{
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID);
				}
				bEndCoding = true;
			}
		}//End of if(frame.size() > 0)

		usleep(1000*1);//等待
	}
	if (fpOut != NULL)
	{
		fclose(fpOut);
		fpOut = NULL;
	}

	printf("===out=DealDspH264Recorder==\n");
}

/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CSkpRoadRecorder::PutTextOnVideo(IplImage* pImageWord,SRIP_DETECT_HEADER* sDetectHeader,CvxText* cvText,string strPlace,string strDirection)
{
    wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    int nWidth = 10;
    int nHeight = 30;

    //时间
    std::string strTime = GetTime(sDetectHeader->uTimestamp,0);
    sprintf(chOut,"时间:%s.%03d",strTime.c_str(),(sDetectHeader->uTime64/1000)%1000);

	printf("pImageWord->width=%d,pImageWord->height=%d,chOut=%s\n",pImageWord->width,pImageWord->height,chOut);

    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    cvText->putText(pImageWord, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,0));


    //地点
    sprintf(chOut,"地点:%s",strPlace.c_str());
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    nHeight += 30;
    cvText->putText(pImageWord, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,255,0));

    //方向
    memset(wchOut,0,sizeof(wchOut));
    sprintf(chOut,"方向:%s",strDirection.c_str());
    UTF8ToUnicode(wchOut,chOut);
    nHeight += 30;
    cvText->putText(pImageWord, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,0));
}

//录像格式为H264裸流加aiv头
void CSkpRoadRecorder::DealAviRecorder(std::string& event)
{
	printf("===in=DealAviRecorder==\n");	

	int nOffset = sizeof(SRIP_DETECT_HEADER);
	//事件消息头
	SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());

	RECORD_EVENT re_event;
	int nSize =  (event.size()-nOffset)/(sizeof(RECORD_EVENT));
	if(nSize > 0)
	{
		memcpy(&re_event,event.c_str()+nOffset,sizeof(RECORD_EVENT));
	}
	std::string strVideoPath(re_event.chVideoPath);

	printf("DealDspH264Recorder strVideoPath=%s\n",strVideoPath.c_str());

	std::string strVideoFileName = strVideoPath;
	String strTmpPath = "ftp://"+g_ServerHost;
	strVideoFileName.erase(0,strTmpPath.size());
	if(g_nServerType == 13 && g_nFtpServer == 1)
	{
		string strDataPath = "/home/road/dzjc";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/dzjc";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else if(g_nServerType == 7)
	{
		string strDataPath = "/home/road/red";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/red";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else
	{
		strVideoFileName = g_strVideo + strVideoFileName;
	}

	printf("DealDspH264Recorder strVideoFileName=%s\n",strVideoFileName.c_str());

	if(access(strVideoFileName.c_str(),F_OK) == 0)
	{
		remove(strVideoFileName.c_str());
	}

	char filepath[256] = {0};
	memset(filepath, 0, 256);
	memcpy(filepath, strVideoFileName.c_str(), strVideoFileName.size());
	
	FILE* fp = NULL;
	CAviFile m_AviFile;

	int m_bAviFlag = false;//录像转换标志
	//CAvc2Avi m_avc2avi; //录像转换类
	//if(!m_bAviFlag)
	{
		//m_avc2avi.SetOutFilePath(filepath);					
		//m_avc2avi.SetFps(g_fFrameRate);//设置帧率
		//m_avc2avi.SetAviformat(g_nVideoWidth, g_nVideoHeight);
		//m_bAviFlag = m_avc2avi.Init();
	}
	
	bool bAddOneFrame = false;

	bool bEndCoding = false;

	int nFrameSize = 0;
	bool bIsWriteFrame = false;

	printf("w=%d,h=%d,fr=%d\n",sDetectHeader1->uWidth,sDetectHeader1->uHeight,sDetectHeader1->dFrameRate);

	int64_t uTime64 = 0;
	SRIP_DETECT_HEADER* sDetectHeader2 = NULL;
	while( !m_bEndRecorder && !bEndCoding)
	{
		bIsWriteFrame = false;

		std::string frame = PopFrame(sDetectHeader1->uTimestamp,uTime64);
		if(frame.size() > 0)
		{
			//图片头
			sDetectHeader2 = (SRIP_DETECT_HEADER*)(frame.c_str());
			nFrameSize = frame.size() - sizeof(SRIP_DETECT_HEADER);
			uTime64 = sDetectHeader2->uTime64;		

			if(sDetectHeader2->uTimestamp < sDetectHeader1->uTimestamp + m_nCaptureTime - 5)
			{
				if(!m_bAviFlag)
				{
					fp = fopen(filepath,"wb");
					char  fcc[5] = "H264";
					/* Init avi */
					if(fp)
					{
						m_AviFile.avi_init( &(m_AviFile.avi), fp, static_cast<float>(g_fFrameRate), fcc, sDetectHeader2->uWidth, sDetectHeader2->uHeight);
						m_AviFile.vbuf_init( &(m_AviFile.vb));
					}
					
					m_bAviFlag = true;
				}

				if (!bIsWriteFrame)//如果没有写入
				{
					if(m_bAviFlag)
					{
						nFrameSize++;

						//处理一帧h264裸流
						//bAddOneFrame = m_avc2avi.ConvertAvc2Avi2((char*)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)), nFrameSize);
						//LogNormal("Deal video frame:%d size:%d %d", m_nFrameSize, nFrameSize, bAddOneFrame);
						if(fp != NULL)
						{
							bool bKeyFrame = false;
							if ((( *(frame.c_str()+sizeof(SRIP_DETECT_HEADER)+4)&0x1f) == 0x07) || (( *(frame.c_str()+sizeof(SRIP_DETECT_HEADER)+4)&0x1f) == 0x08) || (( *(frame.c_str()+sizeof(SRIP_DETECT_HEADER)+4)&0x1f) == 0x05))
							{
								bKeyFrame = true;
							}
							m_AviFile.vbuf_add( &m_AviFile.vb, nFrameSize, (char*)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)) );
							if( m_AviFile.vb.i_data > 0 )
							{
								m_AviFile.avi_write( &m_AviFile.avi, &m_AviFile.vb, bKeyFrame ? AVIIF_KEYFRAME : 0);
								m_AviFile.vbuf_reset(&m_AviFile.vb);
							}
						}
						
						bIsWriteFrame = true;
					}				
				}
			}
			else
			{
				//LogNormal("Writ FrameSize:%d  path:%s \n", m_nFrameSize, strVideoFileName.c_str());	
				if(m_bAviFlag && (nFrameSize > 0) )
				{
					//停止一段录像
					//m_avc2avi.EndAvc2Avi();
					m_AviFile.avi.i_width  = sDetectHeader2->uWidth;
					m_AviFile.avi.i_height = sDetectHeader2->uHeight;
					m_AviFile.avi_end( &m_AviFile.avi );
					m_bAviFlag = false;		
				}

				if(sDetectHeader1->uDetectType == SRIP_VIOLATION_VIDEO)
				{
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID,2);
				}
				else if (SRIP_EVENT_PLATE_VIDEO == sDetectHeader1->uDetectType)
				{
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID);
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID,2);
				}
				else
				{
					g_skpDB.VideoSaveUpdate(strVideoFileName,sDetectHeader1->uChannelID);
				}

				bEndCoding = true;
			}			
		}//End of if(frame.size() > 0)

		usleep(1000*1);//等待
	}

	if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

	printf("===out=DealAviRecorder==\n");
}