// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include"PicLib.h"
#include "Common.h"
#include "CommonHeader.h"

//YUV采集线程
void* ThreadPicCapture(void* pArg)
{
	//取类指针
	PicLib* picLib = (PicLib*)pArg;

	if(picLib == NULL) return pArg;

	picLib->DealPicFrame();
	//pthread_exit((void *)0);
	return pArg;
}





//构造函数
PicLib::PicLib(int nCameraType,int nPixelFormat)
{
	m_nCameraType = nCameraType;
	m_nPixelFormat = nPixelFormat;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nMarginX = 0;
	m_nMarginY = 0;
	m_pFrameBuffer = NULL;
}

PicLib::~PicLib()
{
}

void PicLib::Init()
{
	for(int i=0; i< MAX_SIZE; i++)//分配图像存储内存
	{
		m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3,sizeof(unsigned char));
	}

	//printf("sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3 = %d\n", (sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3));
	m_uSeq = 0;
	m_nPreFieldSeq  = 0;
	m_nCurIndex = 0;
	m_nFirstIndex = 0;
	m_nFrameSize=0;
	m_pFrameBuffer = m_FrameList[0];

	m_bCameraControl = false;
	m_nCountGain = 0;
	m_bEndCapture = false;

	//线程ID
	m_nThreadId = 0;

	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);

	struct   sched_param   param;

	param.sched_priority   =   20;
	pthread_attr_setschedparam(&attr,&param);

	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	int nret=pthread_create(&m_nThreadId,NULL,ThreadPicCapture,this);

	if(nret!=0)
	{
		Close();
		//失败
		LogError("创建图像采集线程失败,无法进行采集！\r\n");
	}
	pthread_attr_destroy(&attr);
}

//打开文件
bool PicLib::OpenFile(const char* strFileName)
{
	std::string strFilePathName(strFileName);
	m_strFilePathName = strFilePathName;
	DIR* dir=NULL;
	struct dirent* ptr=NULL;
	dir = opendir(strFilePathName.c_str());
	if(dir)
	{
		while( ((ptr=readdir(dir))!=NULL)&&(!g_bEndThread))
		{
			if(ptr)
			{
				if((strcmp(".",ptr->d_name)!=0)&&
					(strcmp("..",ptr->d_name)!=0))
				{
					char buf[128] = {'\0'};
					sprintf(buf,"%s/%s",strFilePathName.c_str(),ptr->d_name);
					std::string strPicPath(buf);
					IplImage* image = cvLoadImage(strPicPath.c_str(),-1);
					if(image)
					{
						m_nWidth = image->width;
						m_nHeight = image->height;
						cvReleaseImage( &image );
						break;//只需要获取一张图的信息
					}
					else
					{
						printf("=======%s,error!\n",strPicPath.c_str());
					}

				}
			}
			usleep(1000*1);
		}
		closedir(dir);
		if ((m_nWidth == 0) || (m_nHeight == 0))
		{
			LogError("图像长宽为零!\r\n");
			return false;
		}
		else
		{
			Init();
			return true;
		}
	}
	else
	{
		return false;
	}
}





//处理YUV采集
void PicLib::DealPicFrame()
{

	int nSleep = 0;	//sleep(5);
	//while(!m_bEndCapture)
	{
		int nRet = -1;

		//获取一帧YUV流
		//if(m_bReadFile)
		{
			struct timeval tv;
			DIR* dir=NULL;
			struct dirent* ptr=NULL;
			dir = opendir(m_strFilePathName.c_str());
			if(dir)
			{
				bool bInit = false;
				BYTE *chImageData = NULL;

				while( ((ptr=readdir(dir))!=NULL)&&(!m_bEndCapture))
				{
					if(ptr)
					{
						if((strcmp(".",ptr->d_name)!=0)&&
							(strcmp("..",ptr->d_name)!=0))
						{
							
							char buf[128] = {'\0'};
							sprintf(buf,"%s/%s",m_strFilePathName.c_str(),ptr->d_name);
							std::string strPicPath(buf);

							struct timeval tv1,tv2;
							if(g_nPrintfTime == 1)
							{
								gettimeofday(&tv1,NULL);
							}

							IplImage* image = cvLoadImage(strPicPath.c_str(),-1);

							if(g_nPrintfTime == 1)
							{
								gettimeofday(&tv2,NULL);
								FILE* fp = fopen("time.log","ab+");
								fprintf(fp,"cvLoadImage==t1=%lld,t2=%lld,dt = %lld\n",(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
								fclose(fp);
							}

							if(image)
							{
								if((m_nWidth != image->width) && (m_nHeight != image->height))
								{
									continue;
								}
								cvConvertImage(image,image,CV_CVTIMG_SWAP_RB);
								yuv_video_buf* buffer = (yuv_video_buf*)m_pFrameBuffer;
								buffer->size = image->width*image->height*3;
								memcpy(m_pFrameBuffer+sizeof(yuv_video_buf),image->imageData,buffer->size);
								m_uSeq++;
								//当前帧号
								buffer->nSeq = m_uSeq;

								buffer->width = image->width;
								buffer->height = image->height;
								buffer->uMarginX = 0;
								buffer->uMarginY = 0;
								buffer->uFrameType = 2;
								//时间戳
								//buffer->ts = (buffer->nSeq)*((int64_t)40000)*(buffer->uFrameType);
								gettimeofday(&tv,NULL);
								buffer->ts = tv.tv_sec*1000000+tv.tv_usec;
								buffer->uTimestamp = tv.tv_sec;
								printf("=======%s,w=%d,h=%d\n",strPicPath.c_str(),image->width,image->height);
								AddFrame();
								cvReleaseImage( &image );
							}
							usleep(1000*2000);
							if (nSleep < 5)
							{
								sleep(1);
								nSleep++;
							}
						}
					}
				}
				sleep(1);

					closedir(dir);
			}
			//usleep(1000*1);
			sleep(1);
		}
	}



}

bool PicLib::Close()
{
	m_bEndCapture = true;
	if (m_nThreadId != 0)//先停止线程
	{
		pthread_join(m_nThreadId, NULL);
		m_nThreadId = 0;
	}
	for (int i=0; i<MAX_SIZE; i++)//再回收线程资源
	{
		if(m_FrameList[i] != NULL)
		{
			free(m_FrameList[i]);
			m_FrameList[i] = NULL;
		}
		m_pFrameBuffer = NULL;
	}
}
