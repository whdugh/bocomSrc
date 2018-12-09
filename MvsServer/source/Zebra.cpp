// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include"Zebra.h"
#include "Common.h"
#include "CommonHeader.h"
//构造函数
Zebra::Zebra(int nCameraType,int nPixelFormat)
{
    m_nCameraType = nCameraType;
    m_nPixelFormat = nPixelFormat;

    if(m_nCameraType==PTG_ZEBRA_200)
    {
        if(0)
        {
            m_nWidth = 1624;
            m_nHeight = 1224;
        }
        else
        {
            m_nWidth = 1600;
            m_nHeight = 1200;
        }
    }
    else if(m_nCameraType==PTG_ZEBRA_500)
    {
        m_nMaxBufferSize = FRAMESIZE;
        if(m_nPixelFormat == VIDEO_BAYER)
        {
            m_nWidth = 2448;
            m_nHeight = 2048;
        }
        else
        {
            m_nWidth = 2400;
            m_nHeight = 2000;
        }
    }
    m_nMarginX = 0;
    m_nMarginY = 0;
    m_pFrameBuffer = NULL;
    tcp_fd = -1;
    udp_fd = -1;
    m_bEndCapture = false;
    m_bSetTable = false;
    //线程ID
    m_nThreadId = 0;
	m_nMaxBufferSize = 2;

}
//析构函数
Zebra::~Zebra()
{
    printf("Zebra::~Zebra\n");
}

//YUV采集线程
void* ThreadZebraCapture(void* pArg)
{
    //取类指针
    Zebra* zebra = (Zebra*)pArg;
    if(zebra == NULL)
        return pArg;

    zebra->CaptureFrame();
    pthread_exit((void *)0);
    return pArg;
}

void Zebra::Init()
{
    for(int i=0; i< m_nMaxBufferSize; i++)
    {
        if(m_nPixelFormat == VIDEO_BAYER)
        m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight,sizeof(unsigned char));
        else
        m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*2,sizeof(unsigned char));
    }

    m_uSeq = 0;
    m_nPreFieldSeq  = 0;
    m_nCurIndex = 0;
    m_nFirstIndex = 0;
    m_nFrameSize=0;
    m_pFrameBuffer = m_FrameList[0];

    m_bCameraControl = false;
    m_nCountGain = 0;
    m_bEndCapture = false;
    m_bSetTable = false;
    //线程ID
    m_nThreadId = 0;


    /*if(m_nCameraType==PTG_ZEBRA_500)
    {
        if(strncmp(g_sysInfo_ex.szDetectorType,"MVS-1000-LVS-151Y",17)!=0 &&
           strncmp(g_sysInfo_ex.szDetectorType,"MVS-1000-LVS-152Y",17)!=0)
           {
               LogError("相机类型与检测器类型不匹配%s\r\n",g_sysInfo_ex.szDetectorType);
                return;
           }
    }*/

    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    int nret=pthread_create(&m_nThreadId,NULL,ThreadZebraCapture,this);
    printf("Zebra m_nThreadId=%lld",m_nThreadId);

    if(nret!=0)
    {
        Close();
        //失败
        LogError("创建yuv采集线程失败,无法进行采集！\r\n");
    }

    pthread_attr_destroy(&attr);
}

void Zebra::CaptureFrame()
{
    if(m_bReadFile)
    {
        while(!m_bEndCapture)
        {
            int nRet = -1;
            nRet = GetNextFileframe();

            if(nRet!=-1)
		    {
			   //采集成功
			  AddFrame(1);
			}
			usleep(1000*1);
        }

    }
    else
    {
        RecvData();
    }
    LogTrace(NULL, "after RecvData\n");
}

//UDP 接收数据
int Zebra::RecvData()
{
    //接收参数socket
    struct sockaddr_in recv_addr;
	socklen_t addrlen = sizeof(recv_addr);

    unsigned char buf[9000] = {0};
    int n = 0;
    int count = 0;
    unsigned short index = 0;
    struct timeval tv;
    int nSize =  m_nHeight*m_nWidth*2;
    if(m_nPixelFormat == VIDEO_BAYER)
    {
        nSize /=  2;
    }
    int m_count = ((nSize/8000)-1);
    bool bFirstFrame = true;
    double t = 0;
	/*struct timeval pretv;
	pretv.tv_sec = 0;
	pretv.tv_usec = 0;
	int nFrameCount = 0;*/
    while (!m_bEndCapture)
    {
        if(udp_fd!=-1)
        {
            memset(buf, 0, 9000);
            gettimeofday(&tv,NULL);

            #ifdef _LOGDEBUG
            if(count==0)
            {
                t = (double)cvGetTickCount();
            }
            #endif

            n = recv(udp_fd, buf, sizeof(buf), MSG_NOSIGNAL);
            //n = recvfrom(udp_fd, buf, sizeof(buf), MSG_NOSIGNAL,(struct sockaddr *) &recv_addr, &addrlen);

            //printf("==============n =  %d,%s,%d\n",n,inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port));
            if (n >=2 && !m_bEndCapture)
            {
                memcpy(&index,buf,2);
                index = ntohs(index);

                if (index == 0)
                {
					//if(count>0)
					//LogWarning("discard one frame index=%d,count=%d\n",index,count);
                    count = 0;
                }
                else
                {
                    count++;
                }

                if (m_pFrameBuffer != NULL && !m_bEndCapture)
                {
                    memcpy(m_pFrameBuffer+sizeof(yuv_video_buf)+count*8000,buf+2,n-2);
                }

                if (count == m_count)
                {
                    if (index == count)
                    {
                        //printf("===================================Have rev %d\n",m_uSeq);

                        yuv_video_buf camera_header;
                        memcpy(camera_header.cType,"VYUY",4);
                        camera_header.height = m_nHeight;
                        camera_header.width = m_nWidth;
                        camera_header.size = nSize;

                        camera_header.nFrameRate = m_cfg.nFrequency;

                        camera_header.uFrameType = 2;
                        //                       camera_header.nFieldSeq = m_uSeq;
                        camera_header.nSeq = m_uSeq;
                        camera_header.uTimestamp = tv.tv_sec;
                        camera_header.ts = tv.tv_sec*1000000+tv.tv_usec;

                        camera_header.uGain = m_cfg.uGain/100.0;
                        camera_header.uMaxGain = m_cfg.nMaxGain/100.0;

                       // printf("=================================camera_header.uTimestamp =%lld\n",camera_header.uTimestamp);

                        if(m_bCameraControl)//控制过相机
                        {
                            camera_header.nCameraControl = 1;
                            m_bCameraControl = false;
                        }
                        else
                        {
                            camera_header.nCameraControl = 0;
                        }

                        if(m_pFrameBuffer != NULL && !m_bEndCapture)
                        {
                            camera_header.data = m_pFrameBuffer+sizeof(yuv_video_buf);
                            memcpy(m_pFrameBuffer,&camera_header,sizeof(yuv_video_buf));
                        }
                        if (!m_bEndCapture)
                        {
							/*if(pretv.tv_sec == 0)
							{
								pretv.tv_sec = tv.tv_sec;
								pretv.tv_usec = tv.tv_usec;
							}

							if(tv.tv_sec*1000000+tv.tv_usec >= pretv.tv_sec*1000000+pretv.tv_usec+1000000)
							{
								if(nFrameCount < camera_header.nFrameRate)
								{
									LogNormal("m_uSeq=%lld,dt=%lld,nFrameCount=%d\n",m_uSeq,(tv.tv_sec*1000000+tv.tv_usec)-(pretv.tv_sec*1000000+pretv.tv_usec),nFrameCount);
								}

								nFrameCount = 0;
								pretv.tv_sec = tv.tv_sec;
								pretv.tv_usec = tv.tv_usec;
							}
							nFrameCount++;*/

							/*FILE *fp = fopen("video.txt", "a");
							fprintf(fp,"receive one frame seq = %u,ts = %lld\n", m_uSeq,camera_header.ts);
							fclose(fp);*/

                            AddFrame(1);


                        }
                    }
                    else
                    {

                       LogWarning("discard one frame index=%d,count=%d\n",index,count);
					   	//LogNormal("网络带宽不足");
                    }

                    #ifdef _LOGDEBUG
                    double dt = ((double)cvGetTickCount() - t)/((double)cvGetTickFrequency()*1000.) ;
                    gettimeofday(&tv,NULL);
                    LogTrace("time-test.log","RecvImage==m_uSeq=%lld,time = %s.%03d,index=%d,count=%d,dt=%d ms\n",m_uSeq,GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,index,count,(int)dt);
                    #endif

                    count = 0;
                    m_uSeq++;
                    usleep(100*1);
                }
            }
        }
		else
		{
			usleep(1000*1);
		}	
    }
    return 1;
}

//打开文件
bool Zebra::OpenFile(const char* strFileName)
{
    std::string strFilePathName(strFileName);
	m_yuvFp = NULL;
	m_yuvFp = fopen64(strFilePathName.c_str(),"rb");
	if(m_yuvFp)
	{
	   //判断文件格式是否有效
	    if(IsValidFormat())
	    {
	        rewind(m_yuvFp);
            m_bReadFile = true;
            m_bFirstFrame = true;

            Init();
            return true;
	    }
	    else
	    {
	        fclose(m_yuvFp);
	        m_yuvFp = NULL;
	    }
	}

    LogError("YUV文件打开失败!\r\n");
    return false;
}


bool Zebra::Open()
{
    //读取相机参数
    m_cfg.uKind = m_nCameraType;
    LoadCameraProfile(m_cfg);
    ConvertCameraParameter(m_cfg,false,true);

    usleep(1000*100);
    m_bReadFile = false;

    //打开并设置串口

   /* if(m_cfg.nLightType == 0)
    {
        if(L_SerialComm.IsOpen())
        {
            L_SerialComm.Close();
        }
        L_SerialComm.OpenDev();
    }*/

    //启动采集线程并分配空间
    Init();

    //printf("===========Zebra::Open,m_cfg.uSH=%d,m_cfg.uGain=%d\n",m_cfg.uSH,m_cfg.uGain);
    if(connect_udp()==-1)
    {
        //失败
        LogError("无法与相机建立udp连接\r\n");
        return false;
    }

    if(connect_tcp()==-1)
    {
        //失败
        LogError("无法与相机建立tcp连接\r\n");
        return false;
    }
    //相机参数设置
    SetCaMeraPara();

    return true;
}

bool Zebra::Close()
{
    m_bEndCapture = true;

    cntl_set(Command_stop);
    printf("===============after Command_stop\n");
    if(tcp_fd!=-1)
    {
        shutdown(tcp_fd,2);
        close(tcp_fd);
        tcp_fd = -1;
    }

    if(udp_fd!=-1)
    {
        shutdown(udp_fd,2);
        close(udp_fd);
        udp_fd = -1;
    }

    if (m_nThreadId != 0)
    {
        //pthread_cancel(m_nThreadId);
        pthread_join(m_nThreadId, NULL);
        m_nThreadId = 0;
    }


    if(m_bReadFile)
	{
		if(m_yuvFp!=NULL)
		{
		  fclose(m_yuvFp);
		  m_yuvFp=NULL;
		}
	}

    //printf("before free\n");
    //加锁
    pthread_mutex_lock(&m_FrameMutex);
    //释放内存
    for (int i=0; i<MAX_SIZE; i++)
    {
        if(m_FrameList[i] != NULL)
        {
            free(m_FrameList[i]);
            m_FrameList[i] = NULL;
        }
        m_nFrameSize = 0;
        m_pFrameBuffer = NULL;
    }
    //解锁
    pthread_mutex_unlock(&m_FrameMutex);
    //printf("after free\n");

    //关闭串口
    //if(g_nCameraControl==1)
    {
  //      L_SerialComm.Close();
    }

    return true;
}

float Zebra::Abs_cntl_get(char *buf)
{
    if(tcp_fd < 0)
    {
		if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }
    char *buff;
    buff = buf;
    int res=send(tcp_fd,buff,strlen(buff),MSG_NOSIGNAL);

    if(res<=0)
    {
        LogError("send Zebra::Abs_cntl_get\n");
        if(tcp_fd!=-1)
        {
            shutdown(tcp_fd,2);
            close(tcp_fd);
            tcp_fd = -1;
        }

        if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }
    char buffer[512];
    memset(buffer,0,sizeof(buffer));
	int nRet = 0;
    
	for(int i = 0;i<3;i++)
	{
		nRet = recv(tcp_fd,buffer,sizeof(buffer),MSG_NOSIGNAL);

		if(nRet >= 16)
		{
			break;
		}
		usleep(1000);
	}

    if(nRet<16)
    {
		 LogError("recv Zebra::Abs_cntl_get\n");

		if(tcp_fd!=-1)
		{
			shutdown(tcp_fd,2);
			close(tcp_fd);
			tcp_fd = -1;
		}

		if(udp_fd!=-1)
		{
			shutdown(udp_fd,2);
			close(udp_fd);
			udp_fd = -1;
		}
        return 0;
    }
    printf("cntl_get is %s\n",buf);
    printf("get_rev is %s,nRet=%d\n",buffer,nRet);
    char rev[9]= {'\0'};
    memcpy(rev,buffer+7,8);
    printf("rev =%s \n",rev);
    int rev_n = strtol(rev,NULL,16);
    printf("rev_n=%x\n",rev_n);
    float rev_x=*(float*)&rev_n;
    printf("x= %f \n",rev_x);
    //delete[]rev;
    return rev_x;
}

//手动控制
int Zebra::ManualControl(CAMERA_CONFIG  cfg)
{
    if(cfg.nIndex != (int)CAMERA_MODE)
        cfg.nMode = m_cfg.nMode;

    if(cfg.nIndex != (int)CAMERA_LIGHTTYPE)
        cfg.nLightType = m_cfg.nLightType;

    if(cfg.nIndex == (int)CAMERA_CMD)
    {

        if (strcmp(cfg.chCmd,"asc")==0)
        {
            cfg.nIndex = (int)CAMERA_ASC;
            cfg.ASC = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"agc")==0)
        {
            cfg.nIndex = (int)CAMERA_AGC;
            cfg.AGC = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"sh")==0)
        {
            cfg.nIndex = (int)CAMERA_SH;
            cfg.uSH = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            cfg.nIndex = (int)CAMERA_GAIN;
            cfg.uGain = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"gama")==0)
        {
            cfg.nIndex = (int)CAMERA_GAMMA;
            cfg.nGamma = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pe")==0)
        {
            cfg.nIndex = (int)CAMERA_PE;
            cfg.uPE = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"epl")==0)
        {
            cfg.nIndex = (int)CAMERA_POL;
            cfg.uPol = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pen")==0)
        {
            cfg.nIndex = (int)CAMERA_EEN;
            cfg.EEN_on = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pei")==0)
        {
            cfg.nIndex = (int)CAMERA_PEI;
            cfg.EEN_delay = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pew")==0)
        {
            cfg.nIndex = (int)CAMERA_PEW;
            cfg.EEN_width = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"bn")==0)//brightness
        {
            if(cntl_brightness_Abs((int)cfg.fValue)==1)
            {
                return 1;
            }
        }
        else if (strcmp(cfg.chCmd,"hue")==0)//hue
        {
            if(cntl_hue_Abs((int)cfg.fValue)==1)
            {
                return 1;
            }
        }
        else if (strcmp(cfg.chCmd,"sat")==0)//saturation
        {
            if(cntl_saturtion_Abs((int)cfg.fValue)==1)
            {
                return 1;
            }
        }
        else if (strcmp(cfg.chCmd,"luta")==0)//luta
        {
            m_cfg.LUT_a = cfg.fValue;
            printf("===============m_cfg.LUT_a=%f\n",m_cfg.LUT_a);
            return 1;
        }
        else if (strcmp(cfg.chCmd,"lutb")==0)//lutb
        {
            m_cfg.LUT_b = cfg.fValue;
            printf("===============m_cfg.LUT_b=%f\n",m_cfg.LUT_b);
            return 1;
        }
        else if (strcmp(cfg.chCmd,"lutd")==0)//lutd
        {
            m_cfg.LUT_d = (int)cfg.fValue;
            printf("===============m_cfg.LUT_d=%d\n",m_cfg.LUT_d);
            return 1;
        }
    }

    if((cfg.nIndex == (int)CAMERA_GAIN)
            ||(cfg.nIndex == (int)CAMERA_PEI)
            ||(cfg.nIndex == (int)CAMERA_PEW)
            ||(cfg.nIndex == (int)CAMERA_MAXGAIN)
            ||(cfg.nIndex == (int)CAMERA_MAXPE)
            ||(cfg.nIndex == (int)CAMERA_MAXSH))
    {
        ConvertCameraParameter(cfg,false);
    }
    else if(cfg.nIndex == (int)CAMERA_LUT)
    {
        m_cfg.UseLUT = cfg.UseLUT;
        return UpdateLUT();
    }

    if(cfg.nIndex == CAMERA_SH)
    {
        cfg.uPE = cfg.uSH;
        cfg.uGain = m_cfg.uGain;
        WriteCameraIni(cfg);
    }
    else if(cfg.nIndex == CAMERA_GAIN)
    {
        cfg.uPE = m_cfg.uSH;
        WriteCameraIni(cfg);
    }

    return Control(cfg);
}

//相机控制
int Zebra::Control(CAMERA_CONFIG  cfg)
{
    int nKind = cfg.nIndex;
    if (nKind == CAMERA_ASC)
    {
        if(cfg.ASC == 1)
        {
            if(cntl_set(Command_shutter)==1)
            {
                m_cfg.ASC = 1;
                return 1;
            }
        }
        else if(cfg.ASC == 0)
        {
            if(cntl_shutter_Abs(m_cfg.uSH)==1)
            {
                m_cfg.ASC = 0;
                return 1;
            }
        }
    }
    else if (nKind == CAMERA_AGC)
    {
        if(cfg.AGC == 1)
        {
            if(cntl_set(Command_gain)==1)
            {
                m_cfg.AGC = 1;
                return 1;
            }
        }
        else if(cfg.AGC == 0)
        {
            if(cntl_gain_Abs(m_cfg.uGain)==1)
            {
                m_cfg.AGC = 0;
                return 1;
            }
        }
    }
    else if(nKind == CAMERA_PEI)
    {
        if(cntl_delay(cfg.EEN_delay)==1)
        {
            m_cfg.EEN_delay = cfg.EEN_delay;
            return 1;
        }
    }
    else if(nKind == CAMERA_PEW)//设置脉宽
    {
        if(cfg.nMode ==0)//连续方式
        {
            if(cntl_duration(cfg.EEN_width)==1)
            {
                m_cfg.EEN_width = cfg.EEN_width;
                return 1;
            }
        }
        else if(cfg.nMode ==1)//触发方式
        {
            /*if(cfg.nLightType==0)//主灯
            {
                L_SerialComm.AdjustPuls(cfg.EEN_width);
            }*/
			if(cntl_duration(cfg.EEN_width)==1)
			{
				m_cfg.EEN_width = cfg.EEN_width;
				return 1;
			}
        }
    }
    else if(nKind == CAMERA_POL)//极性
    {
        if(cntl_polarity(cfg.uPol)==1)
        {
            m_cfg.uPol = cfg.uPol;
            return 1;
        }
    }
    else if(nKind == CAMERA_EEN)//开关灯
    {
        printf("==cfg.nMode=%d\n",cfg.nMode);
        if(cfg.nMode ==0)//连续方式
        {
            if(cntl_strobe(cfg.EEN_on)==1)
            {
                m_cfg.EEN_on = cfg.EEN_on;
                return 1;
            }
        }
        else if(cfg.nMode ==1)//触发方式
        {
     //       if(L_SerialComm.SetOpenAndHost(cfg.EEN_on,cfg.EEN_on))
			if(cntl_strobe(cfg.EEN_on)==1)
            {
                m_cfg.EEN_on = cfg.EEN_on;
                return 1;
            }
        }
    }
    else if(nKind == CAMERA_SH)
    {
		//LogError("cntl cfg.uSH = %d\n",cfg.uSH);
        if(cntl_shutter_Abs(cfg.uSH)==1)
        {
            m_cfg.uSH = cfg.uSH;
            m_cfg.ASC = 0;
            return 1;
        }
    }
    else if(nKind == CAMERA_GAIN)
    {
		//LogError("cntl cfg.uGain = %d\n",cfg.uGain);
        if(cntl_gain_Abs(cfg.uGain)==1)
        {
            m_cfg.uGain = cfg.uGain;
            m_cfg.AGC = 0;
            return 1;
        }
    }
    else if(nKind == CAMERA_GAMMA)
    {
        if(cntl_gamma_Abs(cfg.nGamma)==1)
        {
            m_cfg.nGamma = cfg.nGamma;
            return 1;
        }
    }
    else if(nKind == CAMERA_MODE)//设置工作方式
    {
        if(cntl_mode(cfg.nMode)==1)
        {
            m_cfg.nMode = cfg.nMode;

            //设置帧率
            if(cfg.nMode ==0)
                cntl_frameRate_Abs(m_cfg.nFrequency);
            return 1;
        }
    }
    else if(nKind == CAMERA_LIGHTTYPE)//设置灯类型
    {

        LogNormal("设置主从灯类型=%d\n",cfg.nLightType);
     //   L_SerialComm.SetOpenAndHost(true,(cfg.nLightType==0));

        m_cfg.nLightType = cfg.nLightType;
        return 1;
    }
    else if(nKind == CAMERA_FREQUENCY) //设置触发频率
    {
        if(cfg.nFrequency < 10 || cfg.nFrequency > 25)
            cfg.nFrequency = 15;


        //设置帧率
        if(cntl_frameRate_Abs(cfg.nFrequency))
        {
            m_cfg.nFrequency = cfg.nFrequency;
        }

        if(cfg.nMode ==1)//触发方式
        {
            if(cfg.nLightType==0)//主灯
            {
    //            L_SerialComm.SetFrequency(cfg.nFrequency);
            }
        }
        return 1;
    }
    else if( nKind == CAMERA_IRIS)//光圈控制
    {
        if(cntl_iris(cfg.nIris)==1)
        {
            m_cfg.nIris = cfg.nIris;
            return 1;
        }
    }
    return 0;
}


//TCP 连接
int Zebra::connect_tcp()
{
    struct sockaddr_in tcp_addr;
    // socket
    if ((tcp_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        return -1;
    }

    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec=1;
    timeo.tv_usec=500000;//超时1.5s


    if(setsockopt(tcp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
    {
        shutdown(tcp_fd,2);
        close(tcp_fd);
        tcp_fd = -1;
        perror("setsockopt");
        return -1;
    }

    if(setsockopt(tcp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {
        shutdown(tcp_fd,2);
        close(tcp_fd);
        tcp_fd = -1;
        perror("setsockopt");
        return -1;
    }
	
	if(m_strCameraIP.size() <= 8)
	{
		m_strCameraIP = "192.168.0.2";
	}

    bzero(&tcp_addr,sizeof(tcp_addr));
    tcp_addr.sin_family=AF_INET;
    tcp_addr.sin_addr.s_addr=inet_addr(m_strCameraIP.c_str());
    tcp_addr.sin_port=htons(ZEBRA_TLE_PORT);

    int nCount = 0;
    while(nCount < 3)//连接不上最多重试三次
    {
        if(connect(tcp_fd,(struct sockaddr *)&tcp_addr,sizeof(struct sockaddr))==-1)
        {
            if(nCount == 2)
            {
                perror("connect");
                LogError("connect:%s\n",strerror(errno));
                if(tcp_fd!=-1)
                {
                    shutdown(tcp_fd,2);
                    close(tcp_fd);
                    tcp_fd = -1;
                }
                return -1;
            }
        }
        else
        {
            break;
        }
        //LogError("nCount=%d",nCount);
        nCount++;
        usleep(1000*10);
    }

    //接收初始值；//set_rev is Type '?' and return for help
    char buffer[512];
    memset(buffer,0,sizeof(buffer));
    recv(tcp_fd,buffer,sizeof(buffer),MSG_NOSIGNAL);
    printf("connect_tcp buffer=%s\n",buffer);

    return 1;
}

//UDP 连接
int Zebra::connect_udp()
{
    /* connect */
    struct sockaddr_in udp_addr;
    bzero(&udp_addr,sizeof(udp_addr));
    udp_addr.sin_family=AF_INET;
    udp_addr.sin_addr.s_addr= htonl(INADDR_ANY);
    udp_addr.sin_port=htons(ZEBRA_UDP_PORT);

    // socket
    if ((udp_fd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        return -1;

    }

    int nLen = 81920000; //接收缓冲区大小(8M)
//设置接收缓冲区大小
    if (setsockopt(udp_fd,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
    {
        shutdown(udp_fd,2);
        close(udp_fd);
        udp_fd = -1;
        return -1;
    }

    int on = 1;
    if(setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
    {
        shutdown(udp_fd,2);
        close(udp_fd);
        udp_fd = -1;
        perror("setsockopt");
        return -1;
    }

    if(bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) == -1)
    {
        shutdown(udp_fd,2);
        close(udp_fd);
        udp_fd = -1;
        LogError("YUV视频流接收地址无法绑定!\r\n");
        return -1;
    }
    printf("star receive date\n");
    return 1;
}

//TCP 控制set
int Zebra::cntl_set(char *buf)
{
    printf("tcp_fd = %d\n",tcp_fd);
    if(tcp_fd < 0)
    {
		if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }

    char *buffer;
    buffer=buf;
    int res;
    printf("send buffer = %s\n",buffer);
    res=send(tcp_fd,buffer,strlen(buffer),MSG_NOSIGNAL);
    printf("send res = %d\n",res);
    if(res<=0)
    {
       //perror("send");
        LogError("设置相机参数失败,主动关闭连接\n");
        usleep(1000*100);

        if(tcp_fd!=-1)
        {
            shutdown(tcp_fd,2);
            close(tcp_fd);
            tcp_fd = -1;
        }

        if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }

    char buff[512];
    memset(buff,0,sizeof(buff));
    printf("11recv buffer = %s\n",buffer);
	int nRet = 0;
    
	for(int i = 0;i<3;i++)
	{
		nRet = recv(tcp_fd,buff,sizeof(buff),MSG_NOSIGNAL);

		if(nRet >= 16)
		{
			break;
		}
		usleep(1000);
	}

    if(nRet<=0)
    {
		LogError("recv cntl_set error=%d\n",nRet);

        if(tcp_fd!=-1)
        {
            shutdown(tcp_fd,2);
            close(tcp_fd);
            tcp_fd = -1;
        }

        if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }

    return 1;
}

//TCP 控制get
int Zebra::cntl_get(char *buf)
{
    if(tcp_fd < 0)
    {
		if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }

    char *buff;
    buff = buf;
    int res=send(tcp_fd,buff,strlen(buff),MSG_NOSIGNAL);

    if(res<=0)
    {
        LogError("send cntl_get error\n");
        if(tcp_fd!=-1)
        {
            shutdown(tcp_fd,2);
            close(tcp_fd);
            tcp_fd = -1;
        }

        if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }
    char buffer[512];
    memset(buffer,0,sizeof(buffer));

	int nRet = 0;
    
	for(int i = 0;i<3;i++)
	{
		nRet = recv(tcp_fd,buffer,sizeof(buffer),MSG_NOSIGNAL);

		if(nRet >= 16)
		{
			break;
		}
		usleep(1000);
	}

    if(nRet<16)
    {
		LogError("recv cntl_get error\n");
        if(tcp_fd!=-1)
        {
            shutdown(tcp_fd,2);
            close(tcp_fd);
            tcp_fd = -1;
        }

        if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }

    printf("cntl_get is %s\n",buffer);
    char rev[9]= {'\0'};
    memcpy(rev,buffer+7,8);
    printf("rev= %s \n",rev);
    int rev_x = strtol(rev,NULL,16);
    printf("rev_x= %x \n",rev_x);
    return rev_x;
}

//设置工作方式
int Zebra::cntl_mode(int nMode)
{
    if(nMode==0)//连续模式
    {
        return cntl_set("set 830 81100000 \r\n");
    }
    else if(nMode==1)//触发模式
    {
        if(cntl_set("set 830 82100000 \r\n")==0)
        {
            return 0;
        }

        if(cntl_set("set 1110 80000001 \r\n")==0)
        {
            return 0;
        }
    }
    return 1;
}

//获取工作方式
int Zebra::get_Mode()
{
    int nMode = cntl_get("get 830 \r\n");
    if( (nMode & 0x2000000))
    {
        return 1;//触发模式
    }
    else
    {
        return 0;//连续模式
    }
}

int Zebra::cntl_saturtion_Abs(int nSaturation)
{
    float saturtion = nSaturation*0.01;
    /* if(saturtion  <|| saturtion >)
     {
         printf("error\n");
         return 0;
     }*/
    char tmp[50];
    //前8位
    int  b_16 = 0x43;

    int x = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

    sprintf(tmp,"set 814 %x \r\n",x);

    if(cntl_set(tmp))
    {
        printf("set manual is ok \n");
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
    int nn=*(int *)&saturtion;
    sprintf(tmp,"set 988 %x \r\n",nn);

    printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
}
int Zebra::cntl_hue_Abs(int nHue)
{
    float hue = nHue*0.01;
    /*  if(hue <|| hue >)
      {
          printf("error\n");
          return 0;
      }*/
    char tmp[50];
    //前8位
    int  b_16 = 0x43;

    int x = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

    sprintf(tmp,"set 810 %x \r\n",x);

    if(cntl_set(tmp))
    {
        printf("set manual is ok \n");
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
    int nn=*(int *)&hue;
    sprintf(tmp,"set 978 %x \r\n",nn);

    //printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
}

int Zebra::cntl_shutter_Abs(int nShutter)
{

    float shutter = nShutter*0.000001;

    printf("cntl_shutter_Abs=%d,shutter=%f\n",nShutter,shutter);
    if(shutter <0 || shutter >0.066647)
    {
        printf("error\n");
        return 0;
    }
    char tmp[50];
    //前8位
    int  b_16 = 0x43;

    int x = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    printf(" x = %x\n",x);

    sprintf(tmp,"set 81c %x \r\n",x);

    if(cntl_set(tmp))
    {
        printf("set manual is ok \n");
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }

    int nn=*(int *)&shutter;
    sprintf(tmp,"set 918 %x \r\n",nn);

    printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }

}

int Zebra::cntl_gain_Abs(int nGain)
{
    float gain = nGain*0.01;
    if(gain <-0.2 || gain >24)
    {
        printf("error\n");
        return 0;
    }

    char tmp[50];
    //前8位
    int  b_16 = 0x43;

    int x = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

    sprintf(tmp,"set 820 %x \r\n",x);

    if(cntl_set(tmp))
    {
        printf("set manual is ok \n");
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
    int nn=*(int *)&gain;
    sprintf(tmp,"set 928 %x \r\n",nn);

    printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }

}
int Zebra::cntl_gamma_Abs(int nGamma)
{
    float gamma = nGamma*0.01;
    if(gamma <0.50000 || gamma >3.999023)
    {
        printf("error\n");
        return 0;
    }

    char tmp[50];
    //前8位
    int  b_16 = 0x43;

    int x = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

    sprintf(tmp,"set 818 %x \r\n",x);

    if(cntl_set(tmp))
    {
        printf("set manual is ok \n");
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
    int nn=*(int *)&gamma;
    sprintf(tmp,"set 948 %x \r\n",nn);

    printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }

}

//设置帧率
int Zebra::cntl_frameRate_Abs(int nFrameRate)
{
    /*if(nFrameRate <= 10) //如果设置的帧率是10，实际送给相机的帧率是15
    {
        nFrameRate = 15;
    }*/
    float fFrameRate = (float)nFrameRate;
    char tmp[50];
    //前8位
    int  b_16 = 0x43;

    int x = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

    sprintf(tmp,"set 83c %x \r\n",x);
    printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        printf("set manual is ok \n");
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
    int nn=*(int *)&fFrameRate;
    sprintf(tmp,"set 968 %x \r\n",nn);

    printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }

}

//
int Zebra::cntl_brightness_Abs(int nBrightness)
{
    float brightness =nBrightness*0.01;
    if(brightness<0 || brightness >6.243896)
    {
        printf("error\n");
        return 0;
    }

    char tmp[50];
    //前8位
    int  b_16 = 0x43;

    int x = 0xc2000000;
    //32位倒序
   /* x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

    sprintf(tmp,"set 800 %x \r\n",x);

    if(cntl_set(tmp))
    {
        printf("set manual is ok \n");
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
    int nn=*(int *)&brightness;
    sprintf(tmp,"set 938 %x \r\n",nn);

    //printf("%s\n",tmp);

    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
}

//读取
int Zebra::ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert)
{
    printf("============cfg.uType=%d",cfg.uType);
    cfg.nMode = m_cfg.nMode;
    //读取相机设置
    if(cfg.uType == 2)//单项
    {
        int nValue = -1;
        if (strcmp(cfg.chCmd,"asc")==0)
        {
            nValue = m_cfg.ASC;
        }
        else if (strcmp(cfg.chCmd,"agc")==0)
        {
            nValue = m_cfg.AGC;
        }
        else if (strcmp(cfg.chCmd,"sh")==0)
        {
            float fvalue = Abs_cntl_get("get 918 \r\n");
            nValue = fvalue*1000000;
        }
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            float fvalue = Abs_cntl_get("get 928 \r\n");
            nValue = fvalue;
        }
        else if (strcmp(cfg.chCmd,"gama")==0)
        {
            float fvalue = Abs_cntl_get("get 948 \r\n");
            nValue = fvalue/0.01;
        }
        else if (strcmp(cfg.chCmd,"bn")==0)//brightness
        {
            float fvalue = Abs_cntl_get("get 938 \r\n");
            nValue = fvalue/0.01;
        }
        else if (strcmp(cfg.chCmd,"hue")==0)//hue
        {
            float fvalue = Abs_cntl_get("get 978 \r\n");
            nValue = fvalue/0.01;
        }
        else if (strcmp(cfg.chCmd,"sat")==0)//saturation
        {
            float fvalue = Abs_cntl_get("get 988 \r\n");
            nValue = fvalue/0.01;
        }
        else if (strcmp(cfg.chCmd,"pe")==0)
        {
            nValue = cntl_get("get 804 \r\n");
            nValue = nValue&0xfff;
        }
        else if (strcmp(cfg.chCmd,"epl")==0)
        {
            if(cfg.nMode == 0 )
                nValue =  get_polarity();
        }
        else if (strcmp(cfg.chCmd,"pen")==0 ||
                 strcmp(cfg.chCmd,"PEN")==0)
        {
            if(cfg.nMode == 0 )
            {
                nValue = get_strobe();
                cfg.EEN_on = nValue;
            }
            else if(cfg.nMode == 1 )
            {
                //nValue = m_cfg.EEN_on;
				nValue = get_strobe();
                cfg.EEN_on = nValue;
            }
        }
        else if (strcmp(cfg.chCmd,"pei")==0)
        {
            nValue = get_delay();
            nValue *= 2;
        }
        else if (strcmp(cfg.chCmd,"pew")==0)
        {
            if(cfg.nMode == 0 )
            {
                nValue = get_duration();
                nValue *= 2;
            }
            else if(cfg.nMode == 1 )
            {
               // nValue = m_cfg.EEN_width;
				nValue = get_duration();
				nValue *= 2;
            }
        }
        else if (strcmp(cfg.chCmd,"luta")==0)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%f",m_cfg.LUT_a);
            return 1;
        }
        else if (strcmp(cfg.chCmd,"lutb")==0)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%f",m_cfg.LUT_b);
            return 1;
        }
        else if (strcmp(cfg.chCmd,"lutd")==0)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%d",m_cfg.LUT_d);
            return 1;
        }
        else if (strcmp(cfg.chCmd,"fr")==0)
        {
            if(cfg.nMode == 0 )
            {
                float fvalue = Abs_cntl_get("get 968 \r\n");
                nValue = fvalue;
            }
            else if(cfg.nMode == 1 )
            {
                nValue = m_cfg.nFrequency;
            }
        }
        else if (strcmp(cfg.chCmd,"iris")==0)
        {
            nValue = cntl_get("get 824 \r\n");
            float fvalue = nValue&0xfff;
            nValue = (fvalue/0xfff)*100;
        }

        if(nValue != -1)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%d",nValue);
        }
    }
    else if(cfg.uType == 0)//多项
    {
        cfg = m_cfg;
        printf("============cfg.uGain = %d,cfg.uSH=%d",cfg.uGain,cfg.uSH);
        ConvertCameraParameter(cfg,true);
        printf("===after=====ConvertCameraParameter\n");
    }


    return 1;
}

int Zebra::get_light(char *buf)
{
    if(tcp_fd < 0)
    {
		if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }

    char *buff;
    buff = buf;
    int res=send(tcp_fd,buff,strlen(buff),MSG_NOSIGNAL);

    if(res<=0)
    {
        LogError("send Zebra::get_light\n");
        if(tcp_fd!=-1)
        {
            shutdown(tcp_fd,2);
            close(tcp_fd);
            tcp_fd = -1;
        }

        if(udp_fd!=-1)
        {
            shutdown(udp_fd,2);
            close(udp_fd);
            udp_fd = -1;
        }
        return 0;
    }
    char buffer[512];
    memset(buffer,0,sizeof(buffer));
	int nRet = 0;
    
	for(int i = 0;i<3;i++)
	{
		nRet = recv(tcp_fd,buffer,sizeof(buffer),MSG_NOSIGNAL);

		if(nRet >= 16)
		{
			break;
		}
		usleep(1000);
	}

    if(nRet<16)
    {
		LogError("recv Zebra::get_light,nRet=%d\n",nRet);
		if(tcp_fd!=-1)
		{
			shutdown(tcp_fd,2);
			close(tcp_fd);
			tcp_fd = -1;
		}

		if(udp_fd!=-1)
		{
			shutdown(udp_fd,2);
			close(udp_fd);
			udp_fd = -1;
		}
        return 0;
    }
	else
	{
		if(strncmp(buffer,"1504 =>",7) != 0)
		{
			LogError("get_light error\n");
			if(tcp_fd!=-1)
			{
				shutdown(tcp_fd,2);
				close(tcp_fd);
				tcp_fd = -1;
			}

			if(udp_fd!=-1)
			{
				shutdown(udp_fd,2);
				close(udp_fd);
				udp_fd = -1;
			}
			return 0;
		}
	}

    printf("get_light is %s\n",buffer);
    char rev[9]= {'\0'};
    memcpy(rev,buffer+8,8);
    //printf("rev =%s \n",rev);
    int rev_x = strtol(rev,NULL,16);
	//LogNormal("rev_x=%d\n",rev_x);
    return rev_x;
}

int Zebra::cntl_polarity(int p)
{
    char tmp[50];
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    int light_s = 0;
    //printf("light == %x\n",light);
    int polarity_sb = 0x1000000;
    int polarity_cb = 0xfeffffff;
    if(p==1)
    {
        light_s = light|polarity_sb;
    }
    else if(p==0)
    {
        light_s = light & polarity_cb;
    }
    else
    {
        printf("wU~~you set wrong number\n");
        return 0;
    }
    printf("light_s = %x\n",light_s);
    sprintf(tmp,"set 1504 %x \r\n",light_s);
    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
}

int Zebra::cntl_delay(int delay)
{
    if(delay <0 || delay >4096)
    {
        printf("set trouble \n");
        return 0;
    }
    char tmp[50];
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    int light_s = 0;
    delay = delay <<12;
    light = light&0xff000fff;
    light_s = light | delay;

    printf("light_s = %x\n",light_s);
    sprintf(tmp,"set 1504 %x \r\n",light_s);
    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }

}

int Zebra::cntl_duration(int duration)
{
    if(duration <0 || duration >4096)
    {
        printf("set trouble \n");
        return 0;
    }
    char tmp[50];
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    int light_s = 0;
    printf("light = %x,duration=%x\n",light,duration);
    light = light&0xfffff000;
    light_s = light | duration;
    printf("light_s = %x\n",light_s);
    sprintf(tmp,"set 1504 %x \r\n",light_s);
    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
}

int Zebra::cntl_strobe(int s)
{
    char tmp[50];
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    int light_s = 0;

    if(s==1)
        light_s = light |(0x2000000);
    else if(s==0)
        light_s = light&0xfdffffff;

    printf("light_s = %x\n",light_s);
    sprintf(tmp,"set 1504 %x \r\n",light_s);
    if(cntl_set(tmp))
    {
        //printf("set is ok \n");
        return 1;
    }
    else
    {
        printf("set trouble \n");
        return 0;
    }
}

int Zebra::get_polarity()
{
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    int polarity_sb = 0x1000000;
    if((light & polarity_sb))
        return 1;
    else
        return 0;
}

int Zebra::get_delay()
{
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    light =light >>12;
    light = light &0xfff;
    return light;
}
int Zebra::get_duration()
{
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    light = light &0xfff;
    return light;
}
int Zebra::get_strobe()
{
    int light = get_light("get 1504 \r\n");
	if(light == 0)
	{
		return -1;
	}
    int strobe_sb = 0x2000000;
    if((light & strobe_sb))
        return 1;
    else
        return 0;

}

//自动控制
int  Zebra::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
{
    fIncrement *= 100;

    CAMERA_CONFIG cfg = m_cfg;//获取当前相机配置

    //LogNormal("AutoControl fRate=%0.2f,fIncrement=%0.2f,uGain=%d,uSH=%d\n",fRate,fIncrement,cfg.uGain,cfg.uSH);

    int nMaxGain = m_cfg.nMaxGain;//2000;
    int nMaxSH = m_cfg.nMaxSH;//2000;
    int nMinGain = 0;
    int nMinSH = 50;
    if(!bDetectCarnum)
    {
        nMaxGain = m_cfg.nMaxGain2;//非机动车道
        nMaxSH = m_cfg.nMaxSH2;
    }

    if( (cfg.uSH == nMaxSH)&& (fRate > 1)) //SH不能再大
    {
        fRate = 1;
    }
    if( (cfg.uGain == nMaxGain)&& (fIncrement > 0)) //GAIN不能再大
    {
        fIncrement = 0;
    }


    if( (cfg.uSH == nMinSH)&& (fRate < 1))  //SH不能再小
    {
        fRate = 1;
    }
    if( (cfg.uGain == nMinGain)&& (fIncrement < 0)) //GAIN不能再小
    {
        fIncrement = 0;
    }

    if(fRate>1)
        cfg.uSH= cfg.uSH*fRate+0.5;
    else if(fRate<1)
        cfg.uSH = cfg.uSH*fRate;

    if( fIncrement >0  || fIncrement < 0)
        cfg.uGain += fIncrement;


    if(cfg.uGain >=nMaxGain)//增益
        cfg.uGain = nMaxGain;
    else if(cfg.uGain<=nMinGain)
        cfg.uGain = nMinGain;

    if(cfg.uSH >=nMaxSH)//快门
        cfg.uSH = nMaxSH;
    else if(cfg.uSH<=nMinSH)
        cfg.uSH = nMinSH;

    if(cfg.ASC == 0)
    {
        if( fRate > 1 || fRate < 1)
        {
            cfg.nIndex = CAMERA_SH;
            Control(cfg);
            //LogNormal("AutoControl uSH=%d\n",cfg.uSH);
        }
    }

    if(cfg.AGC == 0)
    {
        if( fIncrement >0  || fIncrement < 0)
        {
            cfg.nIndex = CAMERA_GAIN;
            Control(cfg);
            //LogNormal("AutoControl uGain=%d\n",cfg.uGain);
        }
    }

    if(nIris != m_cfg.nIris)//光圈控制
    {
        cfg.nIris = nIris;
        cfg.nIndex = CAMERA_IRIS;
        //Control(cfg);
    }

    if(fRate > 1 || fRate < 1 ||
            fIncrement >0  || fIncrement < 0)
    {
        m_bCameraControl = true;
        //写配置文件
        cfg.uPE = cfg.uSH;
        WriteCameraIni(cfg);
        LogNormal("AutoControl uGain=%d,uSH=%d,nEn=%d\n",cfg.uGain,cfg.uSH,nEn);
    }

    //判断是否开灯
    cfg.EEN_on = LightControl();

    if(cfg.EEN_on > -1)
    {
        m_cfg.EEN_on = cfg.EEN_on;
    }
	else
	{
		LogError("读取状态出错链接断开\n");
		if(tcp_fd!=-1)
		{
			shutdown(tcp_fd,2);
			close(tcp_fd);
			tcp_fd = -1;
		}

		if(udp_fd!=-1)
		{
			shutdown(udp_fd,2);
			close(udp_fd);
			udp_fd = -1;
		}
		return 0;
	}

    if(g_nHasHighLight == 0)
    {
        if(g_LightTimeInfo.nLightTimeControl == 0)
        {
            //如果增益和曝光时间很大仍然未开灯则开灯
            if((nEn == 1))//进入晚间模式
            {
                if(cfg.EEN_on != 1)//根据亮度去开灯
                {
                    LogNormal("AutoControl OpenLight,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);
                    cfg.EEN_on = 1;//打开
                    cfg.nIndex = CAMERA_EEN;
                    Control(cfg);
                    usleep(1000*5);

                    //设置gama
                    cfg.nIndex = CAMERA_GAMMA;
                    cfg.nGamma = cfg.nMaxGamma;
                    Control(cfg);
                    usleep(1000*1);
                }

            }
            else if((nEn == 0))
            {

                    if( (cfg.EEN_on != 0))//根据亮度去关灯
                    {
                        LogNormal("AutoControl CloseLight,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);

                        //关灯
                        cfg.EEN_on = 0;//关闭
                        cfg.nIndex = CAMERA_EEN;
                        Control(cfg);

                        m_nCountGain = 0;
                        usleep(1000*1);
                        //设置gama
                        cfg.nIndex = CAMERA_GAMMA;
                        cfg.nGamma = 100;
                        Control(cfg);
                    }
            }
        }
        else if(g_LightTimeInfo.nLightTimeControl == 1)
        {
            int nDayNight = DayOrNight(1);
            if(nDayNight == 0)
            {
                if(cfg.EEN_on != 1)//根据亮度去开灯
                {
                    LogNormal("强制开灯,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);
                    cfg.EEN_on = 1;//打开
                    cfg.nIndex = CAMERA_EEN;
                    Control(cfg);
                    usleep(1000*5);

                    //设置gama
                    cfg.nIndex = CAMERA_GAMMA;
                    cfg.nGamma = cfg.nMaxGamma;
                    Control(cfg);
                    usleep(1000*1);
                }

            }
            else if(nDayNight == 1)
            {
                if( (cfg.EEN_on != 0))//根据亮度去关灯
                {
                        LogNormal("强制关灯,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);

                        //关灯
                        cfg.EEN_on = 0;//关闭
                        cfg.nIndex = CAMERA_EEN;
                        Control(cfg);

                        m_nCountGain = 0;
                        usleep(1000*1);
                        //设置gama
                        cfg.nIndex = CAMERA_GAMMA;
                        cfg.nGamma = 100;
                        Control(cfg);
                }
            }
        }
    }
    else if(g_nHasHighLight == 1)//有爆闪灯则不管白天晚上都需要打开闪光灯
    {
        //如果未开灯强制开灯
        if(cfg.EEN_on != 1)
        {
            cfg.EEN_on = 1;//打开
            cfg.nIndex = CAMERA_EEN;
            Control(cfg);
        }
    }

    return 1;
}

//灯控制
int   Zebra::LightControl(bool bInit)
{
    CAMERA_CONFIG cfg = m_cfg;

    if(!bInit)
    {
        //判断是否开灯
        cfg.uType = 2;
        memset(cfg.chCmd,0,sizeof(cfg.chCmd));
        memcpy(cfg.chCmd,"PEN",3);
        ReadCameraSetting(cfg);
    }
    else
    {
        ///////////////////////////////闪光灯控制
        int nDayNight = DayOrNight(1);
        if(nDayNight==0)//晚间
        {
            LogNormal("OpenLight\n");

            cfg.EEN_on = 1;//打开
            cfg.nIndex = CAMERA_EEN;
            Control(cfg);

            //设置gama
            cfg.nIndex = CAMERA_GAMMA;
            cfg.nGamma = cfg.nMaxGamma;
            Control(cfg);
            usleep(1000*5);

            //if(g_nWorkMode == 0)
            {
                if(cfg.nMode == 1)//触发方式
                {
                    //设置相机为外触发
                    cntl_mode(1);
                    LogNormal("相机进入触发工作模式\n");
                }
            }
        }
        else if(nDayNight==1)//白天
        {
            LogNormal("CloseLight\n");

            //if(g_nWorkMode == 0)
            {
                if(cfg.nMode == 1)//触发方式
                {
                    //设置相机为触发方式
                    cntl_mode(1);
                    LogNormal("相机进入触发工作模式\n");
                }
            }

            if(g_nHasHighLight == 0)
            {
                cfg.EEN_on = 0;//关闭
                cfg.nIndex = CAMERA_EEN;
                Control(cfg);
            }
            else if(g_nHasHighLight == 1)
            {
                cfg.EEN_on = 1;//打开
                cfg.nIndex = CAMERA_EEN;
                Control(cfg);
            }

            //设置gama
            cfg.nIndex = CAMERA_GAMMA;
            cfg.nGamma = 100;
            Control(cfg);
        }
    }
    return cfg.EEN_on;
}

//相机参数设置
void Zebra::SetCaMeraPara()
{
    //相机控制初始化
    cntl_set(Command_rev ); //000
    usleep(1000*5);
    cntl_set(Command_power);//610
    usleep(1000*5);

    if(m_nCameraType==PTG_ZEBRA_200)
    {
        if((m_nWidth == 1624)&&(m_nHeight == 1224))
        {
            cntl_set("set 608 e0000000 \r\n");
            cntl_set("set 604 00000000 \r\n");
            cntl_set("set A08 00000000 \r\n");
            cntl_set("set A0C 065804C8 \r\n");
            cntl_set("set A10 02000000 \r\n");
            cntl_set("set A7C 40000000 \r\n");
            cntl_set("set A44 3AC03AC0 \r\n");
        }
        else
        {
            cntl_set(Command_mode);
            usleep(1000*5);
            cntl_set(Command_format);
            usleep(1000*5);
        }
    }
    else if(m_nCameraType==PTG_ZEBRA_500)
    {
        cntl_set("set 608 e0000000 \r\n");
        cntl_set("set 604 00000000 \r\n");
        cntl_set("set A08 00000000 \r\n");

        //2448x2048
        if(m_nPixelFormat == VIDEO_BAYER)
        {
            cntl_set("set A0C 09900800 \r\n");
            cntl_set("set A10 09000000 \r\n");
            cntl_set("set A7C 40000000 \r\n");
            cntl_set("set A44 24B424B4 \r\n");
        }
        else
        {
            cntl_set("set A0C 096007D0 \r\n");
            cntl_set("set A10 02000000 \r\n");
            cntl_set("set A7C 40000000 \r\n");
            cntl_set("set A44 49500000 \r\n");
        }

    }
    cntl_set(Command_compress);
    usleep(1000*5);

    //相机参数设置
    CAMERA_CONFIG  cfg = m_cfg;
    //printf("===g_nCameraControl=%d\n",g_nCameraControl);

    //设置主从灯
    cfg.nIndex = CAMERA_LIGHTTYPE;
    Control(cfg);
    usleep(1000*5);

    //设置触发频率
    cfg.nIndex = CAMERA_FREQUENCY;
    Control(cfg);
    usleep(1000*5);

    //必须开灯情况下才能设置
    cfg.nIndex = CAMERA_POL;
    Control(cfg);
    usleep(1000*5);

    cfg.nIndex = CAMERA_PEW;
    Control(cfg);
    usleep(1000*5);


    if(g_uTime >= g_uLastTime+1200)
    {
        InitialGainAndSH();
    }
    else
    {
        if(cfg.ASC == 0)
        {
            cfg.nIndex = CAMERA_SH;
            Control(cfg);
            usleep(1000*5);
        }

        if(cfg.AGC == 0)
        {
            cfg.nIndex = CAMERA_GAIN;
            Control(cfg);
            usleep(1000*5);
        }
    }
    LogNormal("SetCaMeraPara cfg.uSH=%d,cfg.uGain=%d\n",cfg.uSH,cfg.uGain);

    //开关灯控制
    LightControl(true);

   // if(m_cfg.UseLUT)
   //     SetTable();

    //光圈控制
    //cfg.nIndex = CAMERA_IRIS;
    //Control(cfg);
    cntl_set(Command_IRS);
    usleep(1000*5);

    cntl_set(Command_start);

    usleep(1000*100);
    /*int nDayNight = DayOrNight(1);
    if(nDayNight==0)//晚间
        UpdateLUT();*/

    //关闭晚上效果
    cntl_set("set 19A0 80000000 \r\n");

    //锐化
    //cntl_set("set 808 82000800 \r\n");
}

//根据时间设置不同的增益、曝光时间初始值
void Zebra::InitialGainAndSH()
{

    //获取当前时间
    time_t now;
    struct tm *newTime,timenow;
    newTime = &timenow;
    time( &now );
    localtime_r( &now,newTime);

    unsigned int uTime =  newTime->tm_hour*100+newTime->tm_min;

    CAMERA_CONFIG cfg;

    if((uTime>=0)&&(uTime<=500))//0->5//后半夜
    {
        cfg.uGain =2000;//固定增益
        cfg.uSH = 2000;
    }
    else if((uTime>=500)&&(uTime<=830))//5->8.30//早晨
    {
        cfg.uGain =1000;//固定增益
        cfg.uSH = 1500;
    }
    else if((uTime>=830)&&(uTime<=1030))//8.30->10.30//上午
    {
        cfg.uGain = 500;
        cfg.uSH = 1000;
    }
    else if((uTime>=1030)&&(uTime<=1430))//10.30->14.30//中午
    {
        cfg.uGain = 100;
        cfg.uSH = 500;
    }
    else if((uTime>=1430)&&(uTime<=1630))//14.30->16.30//下午
    {
        cfg.uGain = 500;
        cfg.uSH = 1000;
    }
    else if((uTime>=1630)&&(uTime<=1800))//16.30->18//黄昏
    {
        cfg.uGain = 1000;
        cfg.uSH = 1500;
    }
    else if((uTime>=1800)&&(uTime<=2400))//18->24//前半夜
    {
        cfg.uGain =2000;//固定增益
        cfg.uSH = 2000;
    }

    if(m_cfg.ASC == 0)
    {
        cfg.nIndex = CAMERA_SH;
        Control(cfg);
    }

    if(m_cfg.AGC == 0)
    {
        cfg.nIndex = CAMERA_GAIN;
        Control(cfg);
    }
}

//reconnect the camera
bool Zebra::ReOpen()
{
    //if(m_cfg.nMode == 0) //连续工作方式
    {
        if(!m_bReadFile)
        {
            if(tcp_fd!=-1)
            {
                shutdown(tcp_fd,2);
                close(tcp_fd);
                tcp_fd = -1;
            }

            if(udp_fd!=-1)
            {
                shutdown(udp_fd,2);
                close(udp_fd);
                udp_fd = -1;
            }

            usleep(1000*100);
            if(connect_udp()==-1)
            {
                //失败
                LogError("无法与相机建立udp连接\r\n");
                return false;
            }

            if(connect_tcp()==-1)
            {
                //失败
                LogError("无法与相机建立tcp连接\r\n");
                return false;
            }

            g_uTime = g_uLastTime;
            //相机参数设置
            SetCaMeraPara();

            LogNormal("相机自动重新连接\r\n");
        }
    }
    return true;
}

//切换工作方式
bool Zebra::ChangeMode()
{
  /*  if(m_cfg.nMode == 1)//触发方式才需要切换
    {
        //设置相机为连续方式(此时需要关闭补光灯)
        cntl_mode(0);

        LogNormal("相机自动切换为连续工作模式\n");
        usleep(1000*5);

        //设置帧率
        cntl_frameRate_Abs(m_cfg.nFrequency);
        usleep(1000*5);

        CAMERA_CONFIG cfg;
        //关灯
        cfg.EEN_on = 0;//关闭
        cfg.nIndex = CAMERA_EEN;
        Control(cfg);
        m_nCountGain = 0;

        //关闭快表
        CloseTable();
    }*/
    return true;
}

int Zebra::SetTable()
{

// 以读文件的形式设置look up table

    FILE *fp;
    int   i = 0;
    int   len = 0;
    int   InitAdress = 0xF80000;
    char  tmp[50];
    char  buf[16];

    fp = fopen("LUT_HEX.hex","r");
    if(fp == NULL)
    {
        perror("fail to open");
        return -1;
    }

    while(fgets(buf,16,fp) != NULL)
    {
        len = strlen(buf);
        buf[len-1]='\0';
        sprintf(tmp,"set %x %s \r\n",InitAdress,buf);
        if(cntl_set(tmp))
        {
            //设置成功置零
            ;
        }
        else
        {
            printf("set is fail\n");
            return -1;
        }

        InitAdress=InitAdress+4;
        i++;
    }
    //printf("count=%d\n",i);
    fclose(fp);

    m_bSetTable = true;
    return 0;
}

int Zebra::table(double a,double g,int d)
{
    //当g=0时，为两段直线
    //循环控制变量
    int i=0;
    //直线斜率
    float k=0;
    //两段直线
    if(g == 0.0)
    {
        for(i=0; i<=d; i++)
        {
            tansfer[i] = a*i;
        }
        k = (511.433-(1.5*d))/(2047-d);
        for(i=d+1; i<2048; i++)
        {
            tansfer[i]=(k*(i-2047)+511.433);
        }
    }
    //前端指数，后端直线
    else
    {
        //指数部分的变化
        for(i=0; i<=d; i++)
        {
            tansfer[i]=(a*(pow((((float)i)/2047),1/g))*511);

        }
        //直线斜率
        k=(511-(a*(pow((((float)d)/2047),1/g))*511))/(2047-d);
        //线性部分变化
        for(i=d+1; i<2048; i++)
        {
            tansfer[i]=(k*(i-2047)+511);
        }
    }

    //构造成功返回0
    return 0;

}

int Zebra::UseTable()
{
    if(m_bSetTable)
    {
        //临时字符变量
        char tmp[50];
        //LUT设置值
        int LUT=0x8C090800;
        LUT=LUT | (1<<25);
        //启动表
        sprintf(tmp,"set 1a40 %x \r\n",LUT);
        if(cntl_set(tmp))
        {
            printf("set is ok\n");
        }
        else
        {
            return -1;
        }
    }
    return 0;


}
int Zebra::CloseTable()
{
    //临时字符变量
    char tmp[50];
    //LUT设置值
    int LUT=0x8C090800;
    //关闭表
    sprintf(tmp,"set 1a40 %x \r\n",LUT);
    if(cntl_set(tmp))
    {
        printf("set is ok\n");
    }
    else
    {
        return -1;
    }
    return 0;

}
int Zebra::UpdateLUT()
{
    //更新快表
    if(m_cfg.UseLUT)
    {
        UseTable();
    }
    else
    {
        CloseTable();
    }
    return 0;

}


int Zebra::ReadTable()
{
    FILE *fp;
    int   i = 0;
    int   len = 0;
    int   InitAdress = 0xF80000;
    char  tmp[50];
    char buffer[50];

    fp = fopen("READ_LUT.hex","wb");
    if(fp == NULL)
    {
        perror("fail to open");
        return -1;
    }

    for(i = 0; i<4096; i++)
    {
        sprintf(tmp,"get %x \r\n",InitAdress);

        int res=send(tcp_fd,tmp,strlen(tmp),MSG_NOSIGNAL);
        if(res<=0)
        {
            return -1;
        }

        memset(buffer,0,sizeof(buffer));

        if(recv(tcp_fd,buffer,sizeof(buffer),MSG_NOSIGNAL)<=0)
        {
            return -1;
        }

//
//    char rev[18]={'\0'};
//     memcpy(rev,buffer,18);
//   printf("rev= %s \n",rev);
//   int rev_x = strtol(rev,NULL,16);
//   printf("rev_x= %x \n",rev_x);
//   int x = rev_x & 0xfff;
//   printf("x= %d \n",x);
        fprintf(fp,buffer);
        InitAdress=InitAdress+4;
    }
    fclose(fp);
    return 0;

}

//将客户端设置的相机参数转换为相机自身能识别的参数
void Zebra::ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad)
{
    if(!bReverseConvert)//客户端转相机
    {
        if(cfg.nMode == 0)//连续方式
        {
            cfg.EEN_delay = cfg.EEN_delay /2.0;
            cfg.EEN_width = cfg.EEN_width /2.0;
        }
        cfg.nMaxGain = cfg.nMaxGain * 100;
        cfg.nMaxGain2 = cfg.nMaxGain2 * 100;

        if(!bFirstLoad)
            cfg.uGain = cfg.uGain*100.0;

        if( (cfg.nIndex == (int)CAMERA_MAXGAIN)
                ||(cfg.nIndex == (int)CAMERA_MAXPE)
                ||(cfg.nIndex == (int)CAMERA_MAXSH))
        {
            m_cfg.nMaxPE = cfg.nMaxPE;
            m_cfg.nMaxSH = cfg.nMaxSH;
            m_cfg.nMaxGain = cfg.nMaxGain;
            m_cfg.nMaxPE2 = cfg.nMaxPE2;
            m_cfg.nMaxSH2 = cfg.nMaxSH2;
            m_cfg.nMaxGain2 = cfg.nMaxGain2;
            m_cfg.nMaxGamma = cfg.nMaxGamma;
            printf("==========m_cfg.nMaxSH2=%d\n",m_cfg.nMaxSH2);
        }
    }
    else//相机转客户端
    {
        if(cfg.nMode == 0)//连续方式
        {
            cfg.EEN_delay = cfg.EEN_delay*2.0;
            cfg.EEN_width = cfg.EEN_width*2.0;
        }
        cfg.nMaxGain = cfg.nMaxGain/100.0;
        cfg.nMaxGain2 = cfg.nMaxGain2/100.0;

        cfg.uGain = cfg.uGain/100.0;
    }

    if(m_nCameraType==PTG_ZEBRA_500)
    {
       cfg.nFrequency = 10;
    }
    else
    {
        if(cfg.nFrequency < 10 || cfg.nFrequency > 25)
        cfg.nFrequency = 15;
    }
}

//获取相机默认模板
void Zebra::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{
    cfg.AGC = 0;
    cfg.ASC = 0;
    cfg.EEN_delay = 0;
    cfg.EEN_width = 1000;
    cfg.uSM = 0;
    cfg.uPol = 0;
    cfg.nMaxSH = 2000;
    cfg.nMaxSH2 = 6000;
    cfg.nMaxGain = 20;
    cfg.nMaxGain2 = 20;
    cfg.nGamma = 130;
    cfg.nMaxGamma = 130;
    cfg.nMode = 0;
    cfg.nLightType = 1;
    cfg.UseLUT = 0;

    if(m_nCameraType==PTG_ZEBRA_500)
    cfg.nFrequency = 10;
    else
    cfg.nFrequency = 15;
}

//控制光圈
int Zebra::cntl_iris(int nIris)
{
    if(cntl_set("set 1140 80040001 \r\n")==0)
    {
        return 0;
    }
    usleep(1000*1);

    if(cntl_set("set 10A8 c0003080 \r\n")==0)
    {
        return 0;
    }
    usleep(1000*1);

    char tmp[50];
    int nIris_s = 0x82000000|nIris;
    sprintf(tmp,"set 824 %x \r\n",nIris_s);

    if(cntl_set(tmp)==0)//手动控制光圈，并将光圈锁定为一半
    {
        return 0;
    }

    return 1;
}
