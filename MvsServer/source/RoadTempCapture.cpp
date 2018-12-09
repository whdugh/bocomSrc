// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2014 上海博康智能信息技术有限公司
// Copyright 2008-2014 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "RoadTempCapture.h"
#include "Common.h"
#include "CommonHeader.h"


//采集线程
void* ThreadTempCapture(void * pArg)
{
	CRoadTempCapture * pRoadTempCapture = (CRoadTempCapture*)pArg;
	if(pRoadTempCapture != NULL)
	{
		pRoadTempCapture->RunRecorder();
	}
	pthread_exit((void *)0);
	return pArg;
}

CRoadTempCapture::CRoadTempCapture()
{
	m_nChannelId = 0;
	m_bEndCapture = false;
	m_uBeginTime = 0;
	m_uEndTime = 0;
	m_pFrameBuffer = NULL;

	for(int i=0; i<MAX_TEMP_VIDEO_SIZE; i++)
	{
		m_FrameList[i] = NULL;
	}
	m_nFrameSize = 0;
	m_nFirstIndex = 0;
	m_nCurIndex = 0;

	pthread_mutex_init(&m_video_Mutex,NULL);
	m_strH264FileName = "";
	m_fpOut = NULL;
	m_nThreadId = 0;
}

CRoadTempCapture::~CRoadTempCapture()
{
	LogNormal("CRoadTempCapture::~CRoadTempCapture 11");
	this->Unit();

	pthread_mutex_destroy(&m_video_Mutex);

	if(m_fpOut != NULL)
	{
		fclose(m_fpOut);
		m_fpOut = NULL;
	}
	LogNormal("CRoadTempCapture::~CRoadTempCapture 22");
}


//初始化
void CRoadTempCapture::Init(int nChannelId, int nWidth, int nHeight)
{
	m_nChannelId = nChannelId;
	m_bEndCapture = false;
	m_uBeginTime = 0;
	m_uEndTime = 0;
	m_uWidth = nWidth;
	m_uHeight = nHeight;

	LogNormal("CRoadTempCapture::Init m_uWidth=%d,m_uHeight=%d\n",m_uWidth,m_uHeight);

	for(int i=0; i<MAX_TEMP_VIDEO_SIZE; i++)
	{
		//FIX h264的一帧小于m_uWidth * m_uHeight?
		m_FrameList[i] = (unsigned char *)calloc(sizeof(Image_header_dsp) + m_uWidth * m_uHeight*4, sizeof(unsigned char));
	}

	m_nCurIndex = 0;   //可存的内存块序号
	m_nFirstIndex = 0;//可取的内存块序号
	m_nFrameSize = 0;//已经存储的内存块个数
	m_pFrameBuffer = m_FrameList[0];

	//启动录像线程
	StartThread();
}

//反初始化
void CRoadTempCapture::Unit()
{
	LogNormal("CRoadTempCapture::Unit 11");
	m_bEndCapture = true;

	//停止录像线程
	CloseThread();

	if(m_fpOut != NULL)
	{
		fclose(m_fpOut);
		m_fpOut = NULL;
	}

	LogNormal("CRoadTempCapture::Unit 11 aaa");

	//加锁缓冲区
	pthread_mutex_lock(&m_video_Mutex);

	//释放内存
	for(int i=0; i<MAX_TEMP_VIDEO_SIZE; i++)
	{
		if(m_FrameList[i]!=NULL)
		{
			//LogNormal("free(m_FrameList[%d])\n",i);
			free(m_FrameList[i]);
			m_FrameList[i] = NULL;
		}
	}
	m_nFrameSize = 0;
	m_pFrameBuffer = NULL;

	//解锁
	pthread_mutex_unlock(&m_video_Mutex);

	LogNormal("CRoadTempCapture::Unit 22");
}

//启动录像线程
bool CRoadTempCapture::StartThread()
{
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//启动事件录像线程
	int nret = pthread_create(&m_nThreadId,&attr,ThreadTempCapture,this);
	if(nret != 0)
	{
		CloseThread();
		LogError("创建缓存录像线程失败!");
	}
	LogNormal("==CRoadTempCapture=start=ThreadTempCapture=m_nThreadId=%x==\n", m_nThreadId);

	pthread_attr_destroy(&attr);
}

//退出线程
void CRoadTempCapture::CloseThread()
{
	if(m_nThreadId != 0)
	{
		//pthread_cancel(m_nThreadId);
		pthread_join(m_nThreadId, NULL);
		m_nThreadId = 0;
		LogNormal("CRoadTempCapture::CloseThread ");
	}
}

//录像主循环
void CRoadTempCapture::RunRecorder()
{
	while(!m_bEndCapture)
	{
		CaptureVideo();

		//等1毫秒响应下一个录象请求
		usleep(1000);
	}
}

//处理1帧录像
void CRoadTempCapture::CaptureVideo()
{
	unsigned char* pBuffer = NULL;
	int nSize = this->PopFrame(&pBuffer);

	struct timeval tv;
	int nFrameSize = 0;

	int nSkip = 0;
	UINT32 uSwitch = 0;
	int iWrite = 0;

	if(nSize>0 && !m_bEndCapture)
	{
		Image_header_dsp* header = (Image_header_dsp*)(pBuffer);
		nFrameSize = header->nSize;

		//LogTrace("CaptureVideo-Tmp-264.log", "==CRoadH264Capture=seq:%d ==nFrameSize=%d===\n", \
			header->nSeq, nFrameSize);		

		if(nFrameSize > 0)
		{
			if(m_uBeginTime == 0)
			{
				m_uBeginTime = header->ts; //单位 us
				m_uEndTime = m_uBeginTime + 60 * 1000 * 1000;//缓存1分钟			
				m_strH264FileName = g_FileManage.GetDspVideoTempPath(m_nChannelId);
				//不存数据库,直接写到对应temp文件夹下
				//清空老的文件,循环存储
				if(access(m_strH264FileName.c_str(),F_OK) == 0)
				{
					FILE* fp = fopen(m_strH264FileName.c_str(),"wb");
					if(fp)
					{
						fwrite("",0,1,fp);
						fclose(fp);
					}
				}

				m_fpOut = fopen(m_strH264FileName.c_str(), "ab+");
				if(NULL == m_fpOut)
				{
					LogNormal("Open file err! path=%s. \n", m_strH264FileName.c_str());
				}
			}
			
			//存信息
			if(m_fpOut != NULL)
			{
				//LogTrace("VideoTempCap.log", "ts:%s size:%d", GetTime(header->ts/1000/1000,2).c_str(), header->nSize);
				//bool bWrite = SafeWrite(m_fpOut, (char*)(pBuffer+sizeof(Image_header_dsp)), nFrameSize);
				bool bWrite = SafeWrite(m_fpOut, (char*)(pBuffer), nFrameSize + sizeof(Image_header_dsp));
				if(!bWrite)
				{
					LogNormal("Write h264Video file error!");
					if(m_fpOut != NULL)
					{
						fclose(m_fpOut);
						m_fpOut = NULL;

						m_fpOut = fopen(m_strH264FileName.c_str(), "ab+");
						if(NULL == m_fpOut)
						{
							LogNormal("Open file err! path=%s. \n", m_strH264FileName.c_str());
						}
					}					
				}		
			}

			if(header->ts >= m_uEndTime)
			{
				if(m_fpOut != NULL)
				{
					fclose(m_fpOut);
					m_fpOut = NULL;
				}				

				//无需通知事件录象完成  //g_skpDB.VideoSaveUpdate(m_strH264FileName,1);
				m_uBeginTime = 0;
			}
		}//End of if(nFrameSize > 0)

		DecreaseSize();
	}//End of if(nSize>0)
}

//添加1帧数据
void CRoadTempCapture::AddFrame(unsigned char * pBuffer, Image_header_dsp &header)
{
	if(pBuffer == NULL)
	{
		return;
	}
		
	if(header.nSize > 8*m_uWidth * m_uHeight)
	{
		LogNormal("AddFrame error! nSize:%d-%d-%d \n ", header.nSize,m_uWidth * m_uHeight);
		return;
	}
	
	if(m_pFrameBuffer)
	{	
		memcpy(m_pFrameBuffer, &header, sizeof(Image_header_dsp));
		memcpy(m_pFrameBuffer+sizeof(Image_header_dsp), pBuffer, header.nSize);
	}

	//加锁缓冲区
	pthread_mutex_lock(&m_video_Mutex);

	m_nFrameSize++;
	if(m_nFrameSize > MAX_TEMP_VIDEO_SIZE-1)//实际帧数
	{
		LogError("通道[%d],temp录像缓冲区满,m_nFrameSize=%d,uSeq=%d",m_nChannelId, m_nFrameSize, header.nSeq);
		m_nFrameSize--;
	}
	else
	{
		m_nCurIndex++;
		m_nCurIndex %= MAX_TEMP_VIDEO_SIZE;
	}
	m_pFrameBuffer = m_FrameList[m_nCurIndex];


	//解锁缓冲区
	pthread_mutex_unlock(&m_video_Mutex);
}
//弹出1帧数据
int CRoadTempCapture::PopFrame(unsigned char ** ppBuffer)
{
	int nSize = 0;
	//加锁
	pthread_mutex_lock(&m_video_Mutex);
	if(m_nFrameSize >= 1) //只有一场
	{
		nSize = 1;
		if(m_FrameList[m_nFirstIndex] != NULL)
		{
			*ppBuffer = m_FrameList[m_nFirstIndex];//指向当前可读取的位置
		}
	}
	//解锁
	pthread_mutex_unlock(&m_video_Mutex);

	return nSize;
}

//弹出帧后减少帧缓存数据
void CRoadTempCapture::DecreaseSize()
{
	//加锁
	pthread_mutex_lock(&m_video_Mutex);

	m_nFrameSize --;
	m_nFirstIndex = (m_nFirstIndex+1) % MAX_TEMP_VIDEO_SIZE;

	//解锁
	pthread_mutex_unlock(&m_video_Mutex);
}
