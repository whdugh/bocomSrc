// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include "AbstractCamera.h"
#include "Common.h"
#include "CommonHeader.h"

AbstractCamera::AbstractCamera()
{
    pthread_mutex_init(&m_FrameMutex,NULL);
    pthread_mutex_init(&m_FrameMutex2,NULL);
	pthread_mutex_init(&m_PlateFrameMutex,NULL);

    m_nThreadId = 0;

    m_nWidth = 0;
    m_nHeight = 0;

    m_nMarginX = 0;
    m_nMarginY = 0;
    m_ratioX = 1;
    m_ratioY = 1;
    m_bDetectRedSignal = false;
    m_bDetectLoopSignal = false;
    m_bDetectRadarSignal = false;
    m_bCameraControl = false;

    m_nFrameSize = 0;
    m_nFirstIndex = 0;
    m_nCurIndex = 0;

    m_nFrameSize2 = 0;
    m_nFirstIndex2 = 0;
    m_nCurIndex2 = 0;

    m_nMaxBufferSize = MAX_SIZE;
    m_bReadFile = false;
	m_bFirstFrame = true;
	m_yuvFp=NULL;

    for(int i=0;i<MAX_SIZE;i++)
    {
        m_FrameList[i] = NULL;
        m_FrameList2[i] = NULL;
    }

    m_pH264Capture = NULL;
    m_bH264Capture = false;
	m_nPixelFormat = VEDIO_YUV;
	m_nCameraPort = 0;
	m_nCamID = 0;
	m_mode = 0;
	m_bUseTestResult = false;
	m_nVideoType = 0;
	m_nChannelId = 1;
	
	m_bSkpRecorder = false;
	m_nRecvNothingFlag = 0;

	m_pTempCapture = NULL;
	m_pSkpRecorder = NULL;
	m_bCamReboot = false;
	m_bCameraState = false;
}

AbstractCamera::~AbstractCamera()
{
     pthread_mutex_destroy(&m_FrameMutex);
     pthread_mutex_destroy(&m_FrameMutex2);
     pthread_mutex_destroy(&m_PlateFrameMutex);

     printf("AbstractCamera::~AbstractCamera\n");
}

//写YUV视频流缓冲队列
bool AbstractCamera::AddFrame(int nBlock)
{
//加锁缓冲区
	pthread_mutex_lock(&m_FrameMutex);

	if(nBlock == 1)//添加第一块
	{
		/*取红灯信号*///需要判断是否需要取
		if(m_bDetectRedSignal)
		{
			yuv_video_buf* pCamera_header = (yuv_video_buf*)m_FrameList[m_nCurIndex];
			r_SerialComm.GetRedSignal(pCamera_header->uTrafficSignal);
			printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++camera_header.uTrafficSignal=%x\n",pCamera_header->uTrafficSignal);
		}

		/*取线圈信号*///需要判断是否需要取
		if(m_bDetectLoopSignal)
		{
			yuv_video_buf* pCamera_header = (yuv_video_buf*)m_FrameList[m_nCurIndex];
			d_SerialComm.GetSpeedSignal(pCamera_header->uSpeedSignal);
			printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++camera_header.uSpeedSignal=%x\n",pCamera_header->uSpeedSignal.SpeedSignal);
		}

		/*取雷达信号*///需要判断是否需要取
		if(m_bDetectRadarSignal)
		{
			yuv_video_buf* pCamera_header = (yuv_video_buf*)m_FrameList[m_nCurIndex];
			g_RadarSerial.GetSpeedSignal(pCamera_header->uRadarSignal);
		}

        m_nFrameSize++;
        if(m_nFrameSize>m_nMaxBufferSize-1)//实际帧数
        {
            m_nFrameSize--;
        }
        else
        {
            m_nCurIndex++;
            m_nCurIndex %= m_nMaxBufferSize;
        }
        m_pFrameBuffer = m_FrameList[m_nCurIndex];
	}

	//解锁缓冲区
    pthread_mutex_unlock(&m_FrameMutex);

	return true;
}

int AbstractCamera::PopFrame(unsigned char** response,int nBlock)
{
    int nSize = 0;

	//加锁
	pthread_mutex_lock(&m_FrameMutex);

    if(nBlock == 1)
    {
        if(m_nFrameSize >= 1) //只有一场
        {
            nSize = 1;
            if(m_FrameList[m_nFirstIndex]!= NULL)
            *response = m_FrameList[m_nFirstIndex];//指向当前可读取的位置
        }
    }

	//解锁
	pthread_mutex_unlock(&m_FrameMutex);
	return nSize;
}

void AbstractCamera::DecreaseSize(int nBlock)
{
     //加锁
	pthread_mutex_lock(&m_FrameMutex);

	if(nBlock == 1)
	{
        m_nFrameSize --;
        m_nFirstIndex = (m_nFirstIndex+1)%m_nMaxBufferSize;
	}

	//解锁
	pthread_mutex_unlock(&m_FrameMutex);

}

//写JPG视频流缓冲队列
bool AbstractCamera::AddJpgFrame(int nBlock)
{
//printf("======AbstractCamera::AddJpgFrame()=====\n");

    if(nBlock == 1)
    {
        //加锁缓冲区
        pthread_mutex_lock(&m_FrameMutex);

        m_nFrameSize++;
        if(m_nFrameSize>m_nMaxBufferSize-1)//实际帧数
        {
            m_nFrameSize--;
        }
        else
        {
            m_nCurIndex++;
            m_nCurIndex %= m_nMaxBufferSize;
        }
        m_pFrameBuffer = m_FrameList[m_nCurIndex];

        //解锁缓冲区
        pthread_mutex_unlock(&m_FrameMutex);
    }
    else
    {
        //加锁缓冲区
        pthread_mutex_lock(&m_FrameMutex2);

        m_nFrameSize2++;
        if(m_nFrameSize2 > m_nMaxBufferSize-1)//实际帧数
        {
            m_nFrameSize2--;
        }
        else
        {
            m_nCurIndex2++;
            m_nCurIndex2 %= m_nMaxBufferSize;
        }
        m_pFrameBuffer2 = m_FrameList2[m_nCurIndex2];

        //解锁缓冲区
        pthread_mutex_unlock(&m_FrameMutex2);
    }

	return true;
}

 //读JPG视频流缓冲队列
int AbstractCamera::PopJpgFrame(unsigned char** response, int nBlock)
{
    //printf(" ==============AbstractCamera::PopJpgFrame===========\n");
    int nSize = 0;

    if(nBlock == 1)
    {
        //加锁
        pthread_mutex_lock(&m_FrameMutex);


        if(m_nFrameSize >= 1) //只有一场
        {
            nSize = 1;
            if(m_FrameList[m_nFirstIndex]!= NULL)
            *response = m_FrameList[m_nFirstIndex];//指向当前可读取的位置
        }

        //解锁
        pthread_mutex_unlock(&m_FrameMutex);
    }
    else
    {
        //加锁
        pthread_mutex_lock(&m_FrameMutex2);

        if(m_nFrameSize2 >= 1) //只有一场
        {
            nSize = 1;
            if(m_FrameList2[m_nFirstIndex2] != NULL)
            *response = m_FrameList2[m_nFirstIndex2];//指向当前可读取的位置
        }

        //解锁
        pthread_mutex_unlock(&m_FrameMutex2);
    }



	return nSize;
}

//修改JPG缓冲队列长度
void AbstractCamera::DecreaseJpgSize(int nBlock)
{
    if(nBlock == 1)
    {
        //加锁
        pthread_mutex_lock(&m_FrameMutex);

        m_nFrameSize --;
        m_nFirstIndex = (m_nFirstIndex + 1) % m_nMaxBufferSize;

        //解锁
        pthread_mutex_unlock(&m_FrameMutex);
    }
    else
    {
        //加锁
        pthread_mutex_lock(&m_FrameMutex2);

        m_nFrameSize2 --;
        m_nFirstIndex2 = (m_nFirstIndex2 + 1) % m_nMaxBufferSize;

        //解锁
        pthread_mutex_unlock(&m_FrameMutex2);
    }
}

//增加一条车牌记录进列表
bool AbstractCamera::AddPlateFrame(RECORD_PLATE_DSP &plate, int nBlock)
{
    printf("======AbstractCamera::AddPlateFrame()=====\n");

    if(nBlock == 1)
    {
        //加锁缓冲区
        pthread_mutex_lock(&m_PlateFrameMutex);

        m_dspPlatelist.push_back(plate);

        //解锁缓冲区
        pthread_mutex_unlock(&m_PlateFrameMutex);
    }

	return true;
}

//弹出车牌列表
int AbstractCamera::PopPlateList(DSP_PLATE_LIST &dspPlateList, int nBlock)
{
    //printf(" ==============AbstractCamera::PopTextFrame===========\n");
    int nSize = 0;

    if(nBlock == 1)
    {
        //加锁
        pthread_mutex_lock(&m_PlateFrameMutex);

        if(m_dspPlatelist.size() >= 1)
        {
            nSize = 1;
            //dspPlateList = m_dspPlatelist; //深度拷贝

            int nPlateNums = m_dspPlatelist.size();
            DSP_PLATE_LIST::iterator it_b = m_dspPlatelist.begin();
            for(int i=0; i<nPlateNums; i++)
            {
                dspPlateList.push_back((*it_b));
                it_b++;

                printf("=PopPlateList===dspPlateList.size()=%d=\n", dspPlateList.size());
            }


            m_dspPlatelist.clear(); //清空
            printf("=PopPlateList===dspPlateList.size()=%d=m_dspPlatelist.size()=%d==\n", dspPlateList.size(), m_dspPlatelist.size());

        }

        //解锁
        pthread_mutex_unlock(&m_PlateFrameMutex);
    }

	return nSize;
}


//获取一帧大图
bool AbstractCamera::GetOneFrame(unsigned char* pBuffer, int nBlock)
{
        bool bRet = false;

        //加锁
        pthread_mutex_lock(&m_FrameMutex);

        if(nBlock == 1)
        {
            if(m_FrameList[(m_nCurIndex-1+m_nMaxBufferSize)%m_nMaxBufferSize])
            {
                memcpy(pBuffer,m_FrameList[(m_nCurIndex-1+m_nMaxBufferSize)%m_nMaxBufferSize],sizeof(yuv_video_buf)+m_nWidth*m_nHeight*2);
                bRet = true;
            }
        }

        //解锁
        pthread_mutex_unlock(&m_FrameMutex);


	return bRet;
}

 //捕获一场JPG图象
bool AbstractCamera::GetOneJpgFrame(unsigned char* pBuffer, int nBlock)
{
    bool bRet = false;

    if(nBlock == 1)
    {
       //加锁
        pthread_mutex_lock(&m_FrameMutex);

        if(m_FrameList[(m_nCurIndex-1+m_nMaxBufferSize)%m_nMaxBufferSize])
        {
            yuv_video_buf header = *((yuv_video_buf*)m_FrameList[(m_nCurIndex-1+m_nMaxBufferSize)%m_nMaxBufferSize]);
            memcpy(pBuffer, m_FrameList[(m_nCurIndex-1+m_nMaxBufferSize)%m_nMaxBufferSize], sizeof(yuv_video_buf) + header.size);
            bRet = true;
        }

        //解锁
        pthread_mutex_unlock(&m_FrameMutex);
    }
    else
    {
        //加锁
        pthread_mutex_lock(&m_FrameMutex2);

        if(m_FrameList2[(m_nCurIndex2-1+m_nMaxBufferSize)%m_nMaxBufferSize])
        {
            yuv_video_buf header = *((yuv_video_buf*)m_FrameList2[(m_nCurIndex2-1+m_nMaxBufferSize)%m_nMaxBufferSize]);
			printf("header.size=%d\n",header.size);
            memcpy(pBuffer, m_FrameList2[(m_nCurIndex2-1+m_nMaxBufferSize)%m_nMaxBufferSize], sizeof(yuv_video_buf) + header.size);
            bRet = true;
        }
        //解锁
        pthread_mutex_unlock(&m_FrameMutex2);
    }

    return bRet;
}

//刷新JPG队列内容
bool AbstractCamera::RefreshJpgFrame(int nBlock)
{
    bool bRet = false;

    if(nBlock == 1)
    {
       //加锁
        pthread_mutex_lock(&m_FrameMutex);

        while(m_nFrameSize > 1)
        {
            m_nFirstIndex = (m_nFirstIndex + 1) % m_nMaxBufferSize;
            m_nFrameSize--;
            //DecreaseJpgSize(1);
        }
        bRet = true;

        //解锁
        pthread_mutex_unlock(&m_FrameMutex);


    }
    else
    {
        //加锁
        pthread_mutex_lock(&m_FrameMutex2);

		if(m_nFrameSize2 > 0)
		{
			while(m_nFrameSize2 > 1)
			{
				m_nFirstIndex2 = (m_nFirstIndex2 + 1) % m_nMaxBufferSize;
				m_nFrameSize2--;
				//DecreaseJpgSize(2);
			}
			bRet = true;
		}
        //解锁
        pthread_mutex_unlock(&m_FrameMutex2);

    }
    return bRet;
}

 //获取yuv图像宽高
int AbstractCamera::GetImageWidth()
{
    return m_nWidth;
}

int AbstractCamera::GetImageHeight()
{
    return m_nHeight;
}

 //获取需要剪切的黑边大小
int AbstractCamera::GetMarginX()
{
    return m_nMarginX;
}

int AbstractCamera::GetMarginY()
{
    return m_nMarginY;
}

int AbstractCamera::GetEffectImageWidth()
{
    return (m_nWidth - m_nMarginX*2);
}

int AbstractCamera::GetEffectImageHeight()
{
    return (m_nHeight - m_nMarginY*2);
}

int AbstractCamera::GetSmallImageWidth()
{
	int nImageWidth = GetEffectImageWidth();
	if( nImageWidth >= 2000)
	{
		nImageWidth /= 6;
	}
	else if(nImageWidth >= 1000)
	{
		nImageWidth /= 4;
	}
	else if(nImageWidth >= 500)
	{
		nImageWidth /=2;
	}

	return nImageWidth;
}

int AbstractCamera::GetSmallImageHeight()
{
	int nImageHeight = GetEffectImageHeight();
	if(nImageHeight >= 2000)
	{
		nImageHeight /= 6;
	}
	if(nImageHeight >= 1000)
	{
		nImageHeight /=4;
	}
	else if(nImageHeight >= 500)
	{
		int nImageWidth = GetEffectImageWidth();
		if(nImageWidth >= 1000)
		{
			nImageHeight /= 4;
		}
		else
		{
		    nImageHeight /= 2;
		}
	}
	return nImageHeight;
}

//jpg图像与yuv图像之间的缩放比
double AbstractCamera::GetRatioX()
{
    return m_ratioX;
}

double AbstractCamera::GetRatioY()
{
    return m_ratioY;
}

//打开
bool AbstractCamera::Open()
{
    return false;
}

//关闭
bool AbstractCamera::Close()
{
    return false;
}

//打开文件
bool AbstractCamera::OpenFile(const char* strFileName)
{
    return false;
}

//控制
int AbstractCamera::ManualControl(CAMERA_CONFIG cfg)
{
    return 0;
}

//自动控制
int  AbstractCamera::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
{
    return 0;
}

//读取
int AbstractCamera::ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert)
{
    return 0;
}

void AbstractCamera::SetVodInfo(VOD_FILE_INFO& vod_info)
{
	m_vod_info = vod_info;
}

bool AbstractCamera::ReOpen()
{
    return true;
}

//获取相机参数设置
CAMERA_CONFIG AbstractCamera::GetCameraConfig()
{
    return m_cfg;
}

//判断文件格式是否有效
bool AbstractCamera::IsValidFormat()
{
        //读yuv头
        yuv_video_header header;
	    int nSize = -1;
		nSize = fread(&header,sizeof(char),sizeof(yuv_video_header),m_yuvFp);
		if(nSize!=sizeof(yuv_video_header))
		{
			return false;
		}
		m_nWidth = header.nWidth;//只需要读取宽度

		printf("header.nWidth=%d,header.nHeight=%d,header.nSize=%d\r\n",header.nWidth,header.nHeight,header.nSize);

		if(m_nWidth == 1600)
		{
			if(m_nHeight >=2000)
			{
				return false;
			}
            m_nMarginX = 0;
            m_nMarginY = 0;
            m_nHeight = header.nHeight;
		}

		if(m_nWidth == DSP_500_BIG_WIDTH)
		{
			m_nHeight = DSP_500_BIG_HEIGHT;
			nSize = m_nWidth*m_nHeight;
		}
		else
		{
			nSize = m_nWidth*m_nHeight*2;
		}


		//判断宽高大小是否正常
		if(header.nWidth!=m_nWidth||header.nSize!=nSize)
		{
		    return false;
		}
        return true;
}

//读yuv文件数据
int AbstractCamera::GetNextFileframe()
{
    if (NULL == m_yuvFp)
    {
        return -1;
    }

	//如果到达文件尾则将文件指针指向文件头
	if(feof(m_yuvFp)!=0)
	{
		rewind(m_yuvFp);
		m_uSeq = 0;
		m_nPreFieldSeq  = 0;
		m_bFirstFrame = true;
	}

	yuv_video_header header;
	unsigned int nFieldSeq = 0;
	unsigned short nFrameRate = 0;
	struct timeval tv;
	int nSize = -1;

	while(1)
	{
		//读yuv头
		nSize = fread(&header,sizeof(char),sizeof(yuv_video_header),m_yuvFp);
		if(nSize!=sizeof(yuv_video_header))
		{
			break;
		}

		if(m_nPixelFormat == VIDEO_BAYER)
		{
			nSize = m_nWidth*m_nHeight;
		}
		else
		{
			nSize = m_nWidth*m_nHeight*2;
		}

		//判断宽高大小是否正常
		if(header.nWidth!=m_nWidth||header.nSize!=nSize)
		{
		    printf("header.nWidth=%d,header.nSize=%d\r\n",header.nWidth,header.nSize);
			break;
		}

		//**********************************测试使用
		if(m_bUseTestResult)
		{
			if(g_nFileID != header.cReserved[0])
			{
				g_nFileID = header.cReserved[0];//文件ID
				LogNormal("g_nFileID=%d,%c%c%c%c-%c%c%c%c",g_nFileID,header.cSynchron[0],header.cSynchron[1],header.cSynchron[2],header.cSynchron[3],header.cType[0],header.cType[1],header.cType[2],header.cType[3]);
			}
		}
		//********************************************

		 nFrameRate =  (header.nSeq) & 0xffff;
         nFieldSeq =   (header.nSeq)>>16;
        //printf("======m_nWidth=%d,m_nHeight=%d,header.nSize=%d,header.cType=%s\n",m_nWidth,m_nHeight,header.nSize,header.cType);
        // printf("======header.nSeq=%x,nFrameRate=%d,nFieldSeq=%d\n",header.nSeq,nFrameRate,nFieldSeq);

		int64_t ts;
		struct timeb tp;
		ftime(&tp);
		ts = tp.time*1000+tp.millitm;

		if(m_bFirstFrame==true)//记住首场
		{
			m_uTs = ts;
			m_uFieldSeq = nFieldSeq;
			m_bFirstFrame = false;
		}

		int nSeq;
		if(	m_nPreFieldSeq>nFieldSeq)
		{
			nSeq  = m_uSeq + nFieldSeq+1;
		}
		else
		{
			nSeq  = m_uSeq + nFieldSeq - m_nPreFieldSeq;
		}

		double low,up,step;


		if(m_nHeight==1080)
		{
                nFrameRate /= 2;
                step = (1000.0/nFrameRate);
		}
		else  if(m_nHeight==540)
		{
		        //nFrameRate = 5;
                step = 1000/nFrameRate;
		}
		else
		{
                step = 1000/nFrameRate;
		}

        low = (nSeq-m_uFieldSeq)*step-5;//step/2;
        up = (nSeq-m_uFieldSeq)*step+5;//step/2;

		double dx = (double)ts-m_uTs;
		{
			if((dx>low)&&(dx<up))
			{
				yuv_video_buf* buffer = (yuv_video_buf*)m_pFrameBuffer;
				buffer->size = header.nSize;
				buffer->data = m_pFrameBuffer+sizeof(yuv_video_buf);//必须要有
				//读yuv数据
				fread(buffer->data,sizeof(char),buffer->size,m_yuvFp);

                m_uSeq  = nSeq ;
				//当前帧号
				buffer->nSeq = m_uSeq;

                buffer->width = header.nWidth;
                buffer->height = m_nHeight;
                buffer->uMarginX = m_nMarginX;
                buffer->uMarginY = m_nMarginY;

				if(m_nHeight==540)//读场
				{
                    buffer->uFrameType = 1;
				}
				else //读帧
				{
                    buffer->uFrameType = 2;
				}
                //时间戳
                //buffer->ts = (buffer->nSeq)*((int64_t)40000)*(buffer->uFrameType);
                gettimeofday(&tv,NULL);
                buffer->ts = tv.tv_sec*1000000+tv.tv_usec;
				buffer->uTimestamp = time(NULL);

				memcpy(buffer->cType,header.cType,4);
				buffer->nFieldSeq = nFieldSeq;
				buffer->nFrameRate = nFrameRate;

                if(m_bDetectRedSignal)
                {
                    buffer->uTrafficSignal = 0xffff;
                   // printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++buffer->uTrafficSignal=%d\n",buffer->uTrafficSignal);
                }

				#ifdef DEBUG_VIDEOYUV
				printf(" read 1 m_nSeq=%d,header.nFieldSeq=%d,header.nSize=%d,m_nPreFieldSeq=%d,nWidth=%d,nHeight=%d\r\n",m_uSeq,nFieldSeq,header.nSize,m_nPreFieldSeq,header.nWidth,header.nHeight);
				#endif
				m_nPreFieldSeq = nFieldSeq ;
				return 1;

			}
			else if(dx>=up)
			{
				//读yuv数据
				fseek(m_yuvFp,header.nSize,SEEK_CUR);

				m_uSeq  = nSeq ;

				#ifdef DEBUG_VIDEOYUV
				printf("skip 1 m_nSeq=%d,header.nFieldSeq=%d,header.nSize=%d,m_nPreFieldSeq=%d,nWidth=%d,nHeight=%d\r\n",m_uSeq,nFieldSeq,header.nSize,m_nPreFieldSeq,header.nWidth,header.nHeight);
				#endif
				m_nPreFieldSeq = nFieldSeq;
				continue;
			}
			else
			{
				#ifdef DEBUG_VIDEOYUV
				printf(" rewind 1 ts=%lld,m_uTs=%lld,nSeq=%d,m_uFieldSeq=%d\r\n",ts,m_uTs,nSeq,m_uFieldSeq);
                #endif
				fseek(m_yuvFp,sizeof(yuv_video_header)*(-1),SEEK_CUR);
				break;
			}
		}
	}

	return -1;
}

//设置H264采集类指针
bool AbstractCamera::SetH264Capture(CRoadH264Capture &H264Capture)
{
    m_pH264Capture = &H264Capture;
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

//设置设备编号
void AbstractCamera::SetDeviceID(int nDeviceID)
{
	m_nDeviceId = nDeviceID;
}

//切换工作方式
bool AbstractCamera::ChangeMode()
{

}

//获取相机默认模板
void AbstractCamera::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{

}

//获取相机默认模板
void AbstractCamera::ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad)
{

}

//获取视频流
void AbstractCamera::PullStream()
{
}

//设置采集违章事件H264码流类指针
bool AbstractCamera::SetSkpRecorder(CSkpRoadRecorder &SkpRecorder)
{
	m_pSkpRecorder = &SkpRecorder;
	if(m_pSkpRecorder != NULL)
	{
		//LogNormal("===m_pSkpRecorder is not NULL==\n");
		return  true;
	}
	else
	{
		//LogNormal("===m_pSkpRecorder is NULL!!!!==\n");
		return false;
	}
}

//获取缓冲区大小
int AbstractCamera::GetFrameSize()
{
	if(g_nDetectMode == 2)
	{
		return m_nFrameSize2;
	}
	else
	{
		return m_nFrameSize;
	}
}

int AbstractCamera::AddRecordEvent(int nChannel,std::string result)
{
	m_nChannelId = nChannel;
	return 0;
}

//设置缓存采集类指针
bool AbstractCamera::SetTempCapture(CRoadTempCapture &TempCapture)
{
	m_pTempCapture = &TempCapture;
	if(m_pTempCapture != NULL)
	{
		return  true;
	}
	else
	{
		return false;
	}
}
