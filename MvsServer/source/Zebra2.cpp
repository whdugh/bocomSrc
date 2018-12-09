// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#include"Zebra2.h"
#include "Common.h"
#include "CommonHeader.h"
//构造函数
ZebraTwo::ZebraTwo(int nCameraType,int nPixelFormat)
{
    m_nCameraType = nCameraType;
    m_nPixelFormat = nPixelFormat;

    if(m_nCameraType==PTG_GIGE_200)
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
    else if(m_nCameraType==PTG_GIGE_500)
    {
        m_nMaxBufferSize = FRAMESIZE;
        if(0)
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
    //tcp_fd = -1;
    //udp_fd = -1;
    m_bEndCapture = false;
  //  m_bSetTable = false;
    //线程ID
    m_nThreadId = 0;
	m_nMaxBufferSize = 2;

	//***********新加的******************
	id_reg_write_req = 100;
	si_gvcp = -1;
	si_stream = -1;
	m_uSeq = 0;

}
//析构函数
ZebraTwo::~ZebraTwo()
{
    printf("ZebraTwo::~ZebraTwo\n");
}

//YUV采集线程
void* ThreadZebraTwoCapture(void* pArg)
{
    //取类指针
    ZebraTwo* zebra2 = (ZebraTwo*)pArg;
    if(zebra2 == NULL)
        return pArg;

    zebra2->CaptureFrame();
    pthread_exit((void *)0);
    return pArg;
}

void ZebraTwo::Init()
{
    for(int i=0; i< m_nMaxBufferSize; i++)
    {
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

    int nret=pthread_create(&m_nThreadId,NULL,ThreadZebraTwoCapture,this);
    printf("Zebra m_nThreadId=%lld",m_nThreadId);

    if(nret!=0)
    {
        Close();
        //失败
        LogError("创建yuv采集线程失败,无法进行采集！\r\n");
    }

    pthread_attr_destroy(&attr);
}

void ZebraTwo::CaptureFrame()
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
      //  RecvData();
		Grab_image();
    }
    LogTrace(NULL, "after RecvData\n");
}

//UDP 接收数据
int ZebraTwo::RecvData()
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
        if(si_stream!=-1)
        {
            memset(buf, 0, 9000);
            gettimeofday(&tv,NULL);

            #ifdef _LOGDEBUG
            if(count==0)
            {
                t = (double)cvGetTickCount();
            }
            #endif

            n = recv(si_stream, buf, sizeof(buf), MSG_NOSIGNAL);
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
							
                            AddFrame(1);


                        }
                    }
                    else
                    {

                       LogWarning("discard one frame index=%d,count=%d\n",index,count);
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
bool ZebraTwo::OpenFile(const char* strFileName)
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


bool ZebraTwo::Open()
{
    //读取相机参数
    m_cfg.uKind = m_nCameraType;
    LoadCameraProfile(m_cfg);
    ConvertCameraParameter(m_cfg,false,true);

    usleep(1000*100);
    m_bReadFile = false;

	if(m_strCameraIP.size() <= 8)
	{
		m_strCameraIP = "192.168.0.2";
	}

	LogNormal("ip=%s",m_strCameraIP.c_str());

	bzero(&sockaddr_cam,sizeof(sockaddr_cam));
	sockaddr_cam.sin_family=AF_INET;
	sockaddr_cam.sin_addr.s_addr= inet_addr(m_strCameraIP.c_str());
	sockaddr_cam.sin_port=htons(PORT_CAM_GVCP);
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

    printf("===========Zebra::Open,m_cfg.uSH=%d,m_cfg.uGain=%d\n",m_cfg.uSH,m_cfg.uGain);
    if(open_socket_stream()==-1)
    {
        //失败
        LogError("无法与相机建立udp连接\r\n");
        return false;
    }

    if(open_socket_gvcp()==-1)
    {
        //失败
        LogError("无法与相机建立tcp连接\r\n");
        return false;
    }


    //相机参数设置
    SetCaMeraPara();

    return true;
}

bool ZebraTwo::Close()
{
    m_bEndCapture = true;

	int  pkt_size;

   

    //SendWriteRequest(0xf0f00614, 0x00000000, &pkt_size);

	SendWriteRequest( IIDC_CONT_SHOT, 0x00000000, &pkt_size );    

    printf("===============after Command_stop\n");
	close_socket_gvcp();
	close_socket_stream();

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

float ZebraTwo::Abs_cntl_get(char *buf)
{
		printf("Abs_cntl_get buf=%s\n",buf);

	char *buff;

	buff = buf;

	unsigned int reg_add ,reg_data;

	int pkt_size;

	reg_add = strtol(buff,NULL,16);
	printf("Abs_cntl_get reg_add=%d\n",reg_add);

	SendReadRequest( reg_add, &reg_data, &pkt_size ); 

	
	float rev_x=*(float*)&reg_data;
	printf("x= %f \n",rev_x);
	//delete[]rev;
	return rev_x;
}

//手动控制
int ZebraTwo::ManualControl(CAMERA_CONFIG  cfg)
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
int ZebraTwo::Control(CAMERA_CONFIG  cfg)
{
    int  pkt_size;
	
	int nKind = cfg.nIndex;
    if (nKind == CAMERA_ASC)
    {
        if(cfg.ASC == 1)
        {
            if(SendWriteRequest(0xf0f0081c, 0x83000000, &pkt_size)==1)
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
            if(SendWriteRequest(0xf0f00820, 0x83000000, &pkt_size)==1)
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


				printf("==m_cfg.EEN_width=%d\n",m_cfg.EEN_width);
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


//gvcp 连接
int ZebraTwo::open_socket_gvcp()
{
	struct sockaddr_in sockaddr_pc;
	bzero(&sockaddr_pc,sizeof(sockaddr_pc));
	sockaddr_pc.sin_family=AF_INET;
	sockaddr_pc.sin_addr.s_addr= inet_addr(g_CameraHost.c_str());
	sockaddr_pc.sin_port=htons(PORT_PC_GVCP);

	si_gvcp = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (si_gvcp < 0)
	{
		printf("error to create socket \r\n");
		return -1;
	}

	 struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec=1;
    timeo.tv_usec=500000;//超时1.5s


    if(setsockopt(si_gvcp, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
    {
        shutdown(si_gvcp,2);
        close(si_gvcp);
        si_gvcp = -1;
        perror("setsockopt");
        return -1;
    }

    if(setsockopt(si_gvcp, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {
        shutdown(si_gvcp,2);
        close(si_gvcp);
        si_gvcp = -1;
        perror("setsockopt");
        return -1;
    }

	if (bind( si_gvcp, (sockaddr * )&sockaddr_pc, sizeof(sockaddr_pc) ) == -1)
	{
		printf("Error in bind pc socket (UDP)!\r\n");
		close_socket_gvcp();
		return -1;
	}
	else
		printf("succeed to connect pc socket (UDP) \r\n");

	return 0;
}
int ZebraTwo::close_socket_gvcp()
{
	if (si_gvcp >= 0)
	{
		shutdown(si_gvcp, 2);
		close(si_gvcp);
		si_gvcp = -1;
	}
	return 0;
}
//UDP 连接
int ZebraTwo::open_socket_stream()
{
	struct sockaddr_in sockaddr_stream;
	bzero(&sockaddr_stream,sizeof(sockaddr_stream));
	sockaddr_stream.sin_family=AF_INET;
	sockaddr_stream.sin_addr.s_addr= htonl(INADDR_ANY);
	sockaddr_stream.sin_port=htons(PORT_CAM_STREAM);
	// socket
	if ((si_stream=socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		return -1;
	}

	int nLen = 81920000;//81920000; 

	if (setsockopt(si_stream,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
	{
		shutdown(si_stream,2);
		close(si_stream);
		si_stream = -1;
		return -1;
	}

	int on = 1;
	if(setsockopt(si_stream, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
	{
		shutdown(si_stream,2);
		close(si_stream);
		si_stream = -1;
		perror("Open_socket_stream:setsockopt");
		return -1;
	}

	if(bind(si_stream, (struct sockaddr *)&sockaddr_stream, sizeof(sockaddr_stream)) == -1)
	{
		shutdown(si_stream,2);
		close(si_stream);
		si_stream = -1;
		return -1;
	}
	return 0;
}


int ZebraTwo::close_socket_stream()
{
	if( si_stream != -1 )
	{
		shutdown(si_stream,2);
		close(si_stream);
		si_stream = -1;
	}
	return 0;
}

int ZebraTwo::Start_stream()
{
	int pkt_size;

	SendWriteRequest( GVCP_CCP, 0x00000002, &pkt_size );    

	SendWriteRequest( IIDC_CONT_SHOT, 0x00000000, &pkt_size );    

	SendWriteRequest( GVCP_CONFIG, 0x00000001, &pkt_size );    
	SendWriteRequest( GVCP_SCDA0, 0xA9FE0040, &pkt_size );//network adapter ip adress A9:FE:00:40  169:254:0:64 it should is same with network adapter ip.
	SendWriteRequest( GVCP_SCP0, 0x000022B1, &pkt_size );    
	SendWriteRequest( GVCP_SCPS0, 0x00001F40, &pkt_size );    

	SendWriteRequest(IIDC_CONT_SHOT, 0x80000000, &pkt_size );    

	return 0;
}

int ZebraTwo::Stop_stream()
{
	int pkt_size;

	SendWriteRequest( IIDC_CONT_SHOT, 0x00000000, &pkt_size );    

	return 0;
}


int ZebraTwo::SendWriteRequest( unsigned int reg_add, unsigned int reg_data, int * pkt_size ) 
{
	
	
	int ret;

	memset( GVCPPacket, '\0', sizeof(GVCPPacket) );
	GVCPPacket[0] = 0x42;
	GVCPPacket[1] = 0x01;

	unsigned short read_cmd = htons(0x0082);
	memcpy( &(GVCPPacket[2]), &read_cmd, 2 );

	GVCPPacket[4] = 0x00;
	GVCPPacket[5] = 0x08;

	unsigned short req = htons( id_reg_write_req );
	id_reg_write_req++;
	memcpy( &(GVCPPacket[6]), &req, 2 );

	unsigned long write_reg_add = htonl( reg_add );
	memcpy( &(GVCPPacket[8]), &write_reg_add, 4 );

	unsigned long write_reg_data = htonl( reg_data );
	memcpy( &(GVCPPacket[12]), &write_reg_data, 4 );

	*pkt_size = 16;
	int fromlen = sizeof (sockaddr_cam);

	if ( si_gvcp >= 0 ) 
	{
		ret = sendto( si_gvcp, (char * ) GVCPPacket, *pkt_size, MSG_NOSIGNAL, 
			(sockaddr *)&sockaddr_cam, sizeof( sockaddr_cam ) );
		printf ("Write Register (0x%08X) (0x%08X), return =%d,*pkt_size=%d) \r\n", reg_add, reg_data, ret,*pkt_size);

		// Dummy read?
		ret = recvfrom (si_gvcp, (char * ) GVCPPacket, *pkt_size, MSG_NOSIGNAL, 
			(sockaddr *)&sockaddr_cam, (socklen_t*) &fromlen );

		printf ("recvfrom ret=%d\r\n", ret);
		
	}

    if(ret < 0) 
		
	return 0;

	else 

    return 1;
}
    
int ZebraTwo::SendReadRequest( unsigned int reg_add, unsigned int* reg_data, int * pkt_size ) 
{
	int res=0;
	int ret;
	int fromlen;

	memset( GVCPPacket, '\0', sizeof(GVCPPacket) );
	GVCPPacket[0] = 0x42;
	GVCPPacket[1] = 0x01;
	unsigned short read_cmd = htons(0x0080);
	memcpy( &(GVCPPacket[2]), &read_cmd, 2 );
	GVCPPacket[4] = 0x00;
	GVCPPacket[5] = 0x04;

	unsigned short req = htons( id_reg_write_req );
	id_reg_write_req++;
	memcpy( &(GVCPPacket[6]), &req, 2 );

	unsigned long read_reg_add = htonl( reg_add );
	memcpy( &(GVCPPacket[8]), &read_reg_add, 4 ); 

	*pkt_size = 12;

	if ( si_gvcp >= 0 ) 
	{
		ret = sendto( si_gvcp, (char * ) GVCPPacket, *pkt_size, MSG_NOSIGNAL, 
			(sockaddr *)&sockaddr_cam, sizeof( sockaddr_cam ) );
		if (ret == *pkt_size)
		{
			memset( GVCPPacket, '\0', sizeof(GVCPPacket) );
			*pkt_size = 256;		
			fromlen = sizeof (sockaddr_cam);
			ret = recvfrom (si_gvcp, (char * ) GVCPPacket, *pkt_size, MSG_NOSIGNAL, 
				(sockaddr *)&sockaddr_cam, (socklen_t*) &fromlen );


			unsigned int read_data;
			memcpy(&read_data, &(GVCPPacket[8]), 4);
			*reg_data = ntohl(read_data);
			printf ("Read Register (0x%08X) (0x%08X), return=%d) \r\n", reg_add, *reg_data, ret);
		}
		else
		{
			res = -1;
		}
	}

	return res;
}
int ZebraTwo::cntl_get(char *buf)
{
	char *buff;

	buff = buf;

	unsigned int reg_add ,reg_data;

	int pkt_size;

	reg_add = strtol(buff,NULL,16);

	SendReadRequest( reg_add, &reg_data, &pkt_size ); 


	int rev_x =*(int*)&reg_data;
	printf("x= %x \n",rev_x);
	//delete[]rev;
	return rev_x;
}

//设置工作方式
int ZebraTwo::cntl_mode(int nMode)
{   
	int pkt_size;

	if(nMode==0)//连续模式
	{
		SendWriteRequest( 0xf0f00830, 0x81100000, &pkt_size );
	}

	else if(nMode==1)//触发模式
	{
		SendWriteRequest( 0xf0f00830, 0x82100000, &pkt_size );

		SendWriteRequest( 0xf0f01110, 0x80000001, &pkt_size );
	}
	  return 1;
}
int ZebraTwo::get_Mode()
{   
	int pkt_size;

	unsigned int reg_data;

    SendReadRequest( 0xf0f00830, &reg_data, &pkt_size );

	int nMode;
	nMode = reg_data;

	if( (nMode & 0x2000000))
	{
		return 1;//触发模式
	}
	else
	{
		return 0;//连续模式
	}
}

int ZebraTwo::cntl_saturtion_Abs(int nSaturation)
{
	int pkt_size;

	float saturtion = nSaturation*0.01;
    /* if(saturtion  <|| saturtion >)
     {
         printf("error\n");
         return 0;
     }*/
  
    //前8位

    unsigned int reg_data;

    reg_data = 0xc2000000;

    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

	if (SendWriteRequest( 0xf0f00814, reg_data, &pkt_size )==0)
	{
		return 0;
	}

	

   
    int nn=*(int *)&saturtion;

    reg_data=nn;
	
	if (SendWriteRequest( 0xf0f00988, reg_data, &pkt_size )==0)
	{
		return 0;
	}

	

	return 1;

}

int ZebraTwo::cntl_hue_Abs(int nHue)
{
	int pkt_size;

	float hue = nHue*0.01;
    /*  if(hue <|| hue >)
      {
          printf("error\n");
          return 0;
      }*/
   
    //前8位
   unsigned int reg_data;

    reg_data = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

   if (SendWriteRequest( 0xf0f00810, reg_data, &pkt_size )==0)
   {
	   return 0;
   }
     
    

    
    int nn=*(int *)&hue;

    reg_data=nn;

    //printf("%s\n",tmp);
	if ( SendWriteRequest( 0xf0f00978, reg_data, &pkt_size )==0)
	{
		return 0;
	}

   

   return 1 ;
}

int ZebraTwo::cntl_shutter_Abs(int nShutter)
{
	int pkt_size;
	
	float shutter = nShutter*0.000001;

    printf("cntl_shutter_Abs=%d,shutter=%f\n",nShutter,shutter);
    if(shutter <0 || shutter >0.066647)
    {
        printf("error\n");
        return 0;
    }
    unsigned int reg_data;

    reg_data  = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

	if (SendWriteRequest( 0xf0f0081c, reg_data, &pkt_size )==0)
    {
		return 0;
    }
    
    int nn=*(int *)&shutter;

    reg_data=nn;

	if (SendWriteRequest( 0xf0f00918, reg_data, &pkt_size )==0)
	{
		return 0;
	}

    


	return 1;
}
int ZebraTwo::cntl_gain_Abs(int nGain)
{
	int pkt_size;

	float gain = nGain*0.01;

    if(gain <-0.2 || gain >24)
    {
        printf("error\n");
        return 0;
    }

    unsigned int reg_data;

    reg_data = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

    
	
	if (SendWriteRequest( 0xf0f00820, reg_data, &pkt_size )==0)
	{
		return 0;
	}
	


    int nn=*(int *)&gain;

    
	reg_data=nn;

	if (SendWriteRequest( 0xf0f00928, reg_data, &pkt_size )==0)
	{
		return 0;
	}

	

	return 1;

   
}

int ZebraTwo::cntl_gamma_Abs(int nGamma)
{
	int pkt_size;

	float gamma = nGamma*0.01;

    if(gamma <0.50000 || gamma >3.999023)
    {
        printf("error\n");
        return 0;
    }
  

    unsigned int reg_data;


    reg_data = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);

	 
    if ( SendWriteRequest( 0xf0f00818, reg_data, &pkt_size )==0)
    {
		return 0;
    }
   

    int nn=*(int *)&gamma;

    reg_data = nn;

    if (SendWriteRequest( 0xf0f00948, reg_data, &pkt_size )==0)
    {
		return 0;
    }
    

	return 1;
    

}

//设置帧率
int ZebraTwo::cntl_frameRate_Abs(int nFrameRate)
{
    /*if(nFrameRate <= 10) //如果设置的帧率是10，实际送给相机的帧率是15
    {
        nFrameRate = 15;
    }*/
    float fFrameRate = (float)nFrameRate;
   
	unsigned int reg_data;
    
    int pkt_size;

    reg_data = 0xc2000000;
    //32位倒序
    /*x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);
	if (SendWriteRequest( 0xf0f0083c, reg_data, &pkt_size )==0)
	{
		return 0;
	}

    

    int nn=*(int *)&fFrameRate;

    reg_data=nn;

	if (SendWriteRequest( 0xf0f00968, reg_data, &pkt_size )==0)
	{
		 return 0;
	}

    

	 return 1;

}

//
int ZebraTwo::cntl_brightness_Abs(int nBrightness)
{
    float brightness =nBrightness*0.01;
    if(brightness<0 || brightness >6.243896)
    {
        printf("error\n");
        return 0;
    }

    unsigned int reg_data;


	int pkt_size;

    reg_data = 0xc2000000;
    //32位倒序
   /* x = (x & 0x55555555) <<  1 | (x & 0xAAAAAAAA) >>  1;
    x = (x & 0x33333333) <<  2 | (x & 0xCCCCCCCC) >>  2;
    x = (x & 0x0F0F0F0F) <<  4 | (x & 0xF0F0F0F0) >>  4;
    x = (x & 0x00FF00FF) <<  8 | (x & 0xFF00FF00) >>  8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;*/

    //printf(" x = %x\n",x);
	if (SendWriteRequest( 0xf0f00800, reg_data, &pkt_size )== 0)
	{
		return 0;
	}
   
  
	

   
    int nn=*(int *)&brightness;

    reg_data=nn;
	if (SendWriteRequest( 0xf0f00938, reg_data, &pkt_size )==0)
	{
		return 0;
	}
	
	return 1;

}

int ZebraTwo::ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert)
{
	printf("============cfg.uType=%d",cfg.uType);
	cfg.nMode = m_cfg.nMode;
	//读取相机设置
	if(cfg.uType == 2)//单项
	{
		unsigned int nValue = -1;
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
			float fvalue = Abs_cntl_get("f0f00918");
			nValue = fvalue*1000000;
		}
		else if (strcmp(cfg.chCmd,"ga")==0)
		{
			float fvalue = Abs_cntl_get("f0f00928");
			nValue = fvalue;
		}
		else if (strcmp(cfg.chCmd,"gama")==0)
		{
			float fvalue = Abs_cntl_get("f0f00948");
			nValue = fvalue/0.01;
		}
		else if (strcmp(cfg.chCmd,"bn")==0)//brightness
		{
			float fvalue = Abs_cntl_get("f0f00938");
			nValue = fvalue/0.01;
		}
		else if (strcmp(cfg.chCmd,"hue")==0)//hue
		{
			float fvalue = Abs_cntl_get("f0f00978");
			nValue = fvalue/0.01;
		}
		else if (strcmp(cfg.chCmd,"sat")==0)//saturation
		{
			float fvalue = Abs_cntl_get("f0f00988");
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
				float fvalue = Abs_cntl_get("f0f00968");
				nValue = fvalue;
			}
			else if(cfg.nMode == 1 )
			{
				nValue = m_cfg.nFrequency;
			}
		}
		else if (strcmp(cfg.chCmd,"iris")==0)
		{
			nValue = cntl_get("f0f00824");
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

unsigned int ZebraTwo::get_light(char *buf)
{
	char *buff;

	buff = buf;

	unsigned int reg_add ,reg_data;

	int pkt_size;

	reg_add = strtol(buff,NULL,16);

	SendReadRequest( reg_add, &reg_data, &pkt_size ); 

    return reg_data;

}

int ZebraTwo::cntl_polarity(int p)
{
	int light = get_light("f0f01504");

	unsigned int reg_data;

	int pkt_size;

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
	light_s = light_s | 0x02000000;
	printf("light_s = %x\n",light_s);

	reg_data=light_s;

	SendWriteRequest( 0xf0f01504, reg_data, &pkt_size );

	if( !(light&0x02000000))
	{
		reg_data = reg_data&0xfdffffff;
		printf("reg_data = %x\n",reg_data);
		SendWriteRequest( 0xf0f01504, reg_data, &pkt_size );
	}
	return 1;
}

int ZebraTwo::cntl_delay(int delay)
{
	unsigned int reg_data;

	int pkt_size;
	
	if(delay <0 || delay >4096)
	{
		printf("set trouble \n");
		return 0;
	}
	
	int light = get_light("f0f01504");
	if(light == 0)
	{
		return -1;
	}
	int light_s = 0;
	delay = delay <<12;
	light = light&0xff000fff;
	light_s = light | delay;
	light_s = light_s | 0x02000000;
	printf("light_s = %x\n",light_s);
	reg_data=light_s;
	
	SendWriteRequest( 0xf0f01504, reg_data, &pkt_size );

	if( !(light&0x02000000))
	{
		reg_data = reg_data&0xfdffffff;
		printf("reg_data = %x\n",reg_data);
		SendWriteRequest( 0xf0f01504, reg_data, &pkt_size );
	}
	return 1;
}

int ZebraTwo::cntl_duration(int duration)
{
	unsigned int reg_data;

	int pkt_size;

	if(duration <0 || duration >4096)
	{
		printf("set trouble \n");
		return 0;
	}

	int light = get_light("f0f01504");
	if(light == 0)
	{
		return -1;
	}
	int light_s = 0;
	printf("light = %x,duration=%x\n",light,duration);
	light = light&0xfffff000;
	light_s = light | duration;
	light_s = light_s | 0x02000000;
	printf("light_s = %x\n",light_s);
	reg_data=light_s;

	SendWriteRequest( 0xf0f01504, reg_data, &pkt_size );

	printf("light&0x02000000 = %x\n",light&0x02000000);
	
	if( !(light&0x02000000))
	{
		reg_data = reg_data&0xfdffffff;
		printf("reg_data = %x\n",reg_data);
		SendWriteRequest( 0xf0f01504, reg_data, &pkt_size );
	}

	return 1;
}

int ZebraTwo::cntl_strobe(int s)
{
	unsigned int reg_data;

	int pkt_size;

	int light = get_light("f0f01504");
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
	reg_data=light_s;
	if(SendWriteRequest( 0xf0f01504, reg_data, &pkt_size ))
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

int ZebraTwo::get_polarity()
{
	int light = get_light("f0f01504");
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
int ZebraTwo::get_delay()
{
	int light = get_light("f0f01504");
	if(light == 0)
	{
		return -1;
	}
	light =light >>12;
	light = light &0xfff;
	return light;
}

int ZebraTwo::get_duration()
{
	int light = get_light("f0f01504");
	if(light == 0)
	{
		return -1;
	}
	light = light &0xfff;
	return light;
}
int ZebraTwo::get_strobe()
{
	int light = get_light("f0f01504");
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
int  ZebraTwo::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
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
		if(si_gvcp!=-1)
		{
			shutdown(si_gvcp,2);
			close(si_gvcp);
			si_gvcp = -1;
		}

		if(si_stream!=-1)
		{
			shutdown(si_stream,2);
			close(si_stream);
			si_stream = -1;
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
int   ZebraTwo::LightControl(bool bInit)
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

void ZebraTwo::SetCaMeraPara()
{
	int pkt_size;

	if(SendWriteRequest( GVCP_CCP, 0x00000002, &pkt_size ) <= 0)//A00
	{
		return;
	}

	SendWriteRequest( IIDC_CONT_SHOT, 0x00000000, &pkt_size );    //614
 	
	SendWriteRequest( GVCP_CONFIG, 0x00000001, &pkt_size );    //954
	//SendWriteRequest( GVCP_SCDA0, 0xA9FE0040, &pkt_size );//network adapter ip adress A9:FE:00:40  169:254:0:64 it should is same with network adapter ip.//D18
	SendWriteRequest( GVCP_SCDA0, strtoul(StringIpToHex(g_CameraHost).c_str(),NULL,16), &pkt_size );
	SendWriteRequest( GVCP_SCP0, 0x000022B1, &pkt_size );    //D00
	SendWriteRequest( GVCP_SCPS0, 0x00001F40, &pkt_size );    //D04
	
	//SendWriteRequest(IIDC_CONT_SHOT, 0x80000000, &pkt_size );    //614


	unsigned int  data, reg_add,reg_data;

	printf(">> Set Video Mode \r\n");

	//SendWriteRequest( IIDC_CONT_SHOT, 0x00000000, &pkt_size );    //614

	SendWriteRequest( IIDC_VALUE_SETTING, 0x40000000, &pkt_size );  //A7C
	SendWriteRequest( IIDC_CURRENT_VIDEO_FORMAT, 0xE0000000, &pkt_size );  	// IIDC_CURRENT_VIDEO_FORMAT 7	0xf0f00608 //608
	SendWriteRequest( IIDC_CURRENT_VIDEO_MODE, 0x00000000, &pkt_size );   	//IIDC_CURRENT_VIDEO_MODE       	0xf0f00604
	SendWriteRequest( IIDC_IMAGE_POSITION, 0x00000000, &pkt_size );  		//IIDC_IMAGE_POSITION 		0xf0f00A08
	SendWriteRequest( IIDC_COLOR_CODING_ID, 0x02000000, &pkt_size );   		//IIDC_COLOR_CODING_ID YUV422	0xf0f00A10

	//SendWriteRequest( IIDC_IMAGE_SIZE, 0x09900800, &pkt_size );		//m_iImageSize_ZEBRA2     	0xf0f00A0C  2448X2048
	SendWriteRequest( IIDC_IMAGE_SIZE, 0x096007D0, &pkt_size );  //2400X2000

	SendWriteRequest( IIDC_VALUE_SETTING, 0x40000000, &pkt_size );		//IIDC_VALUE_SETTING	      		0xf0f00A7C
	SendWriteRequest( IIDC_FRAME_RATE, 0xC20001E0, &pkt_size );			//83c 
	SendWriteRequest( IIDC_FRAME_RATE_ABS, 0x41200000, &pkt_size );  		//IIDC_FRAME_RATE 0X968 10fps

	SendWriteRequest(IIDC_CONT_SHOT, 0x80000000, &pkt_size );    

	/*do{
		SendReadRequest(IIDC_VALUE_SETTING, &data,&pkt_size);	 
	} while ((data & 0x40000000) != 0);*/

	usleep(1000*5);

	//return;

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

	//SendWriteRequest(0xf0f00824, 0x820000C0, &pkt_size );

    //usleep(1000*5);
}

   
//根据时间设置不同的增益、曝光时间初始值
void ZebraTwo::InitialGainAndSH()
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
bool ZebraTwo::ReOpen()
{
    //if(m_cfg.nMode == 0) //连续工作方式
    {
        if(!m_bReadFile)
        {
            if(si_gvcp!=-1)
            {
                shutdown(si_gvcp,2);
                close(si_gvcp);
                si_gvcp= -1;
            }

            if(si_stream!=-1)
            {
                shutdown(si_stream,2);
                close(si_stream);
                si_stream = -1;
            }

            usleep(1000*100);
            if(open_socket_stream()==-1)
            {
                //失败
                LogError("无法与相机建立udp连接\r\n");
                return false;
            }

            if(open_socket_gvcp()==-1)
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
bool ZebraTwo::ChangeMode()
{

    return true;
}


//将客户端设置的相机参数转换为相机自身能识别的参数
void ZebraTwo::ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad)
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
void ZebraTwo::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
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

    if(m_nCameraType==PTG_GIGE_500)
    cfg.nFrequency = 10;
    else
    cfg.nFrequency = 15;
}

//控制光圈
int ZebraTwo::cntl_iris(int nIris)
{
    int pkt_size;

	unsigned int  reg_data;

	if(SendWriteRequest( 0xf0f01140, 0x80040001, &pkt_size )==0)
    {
        return 0;
    }
    usleep(1000*1);

    if(SendWriteRequest( 0xf0f010A8, 0xc0003080, &pkt_size )==0)
    {
        return 0;
    }
    usleep(1000*1);

    
    unsigned int nIris_s = 0x82000000|nIris;
    reg_data=nIris_s;

    if(SendWriteRequest( 0xf0f00824, reg_data, &pkt_size )==0)//手动控制光圈，并将光圈锁定为一半
    {
        return 0;
    }

    return 1;
}
int ZebraTwo::Grab_image()
{
	struct timeval tv;
	m_iPacketSize = 8000;
	
	int nSize = m_nWidth * m_nHeight*2;
	m_iPacketsPerFrame =((nSize/(m_iPacketSize - 36))+1);   

	int rc = 0;
	unsigned int bytesRecv = 0;
	unsigned int totBytesRecv = 0;
	char* buffer = NULL;
	//	char* packet;
	char packet[10000] = {0};

	unsigned int gvsp_hdr_size = 0;
	unsigned int data_packet_size = 0;

	int iErr = 0;
	unsigned int iPacketRecv = 0;
	int rc_count = 0;

	int iFrameCount = 0;
	int iFrameGrabbed = 0;
	unsigned short iLastBlockID = 0;
	unsigned short iCurBlockID;
	unsigned int iLastPacketID = 0;
	int iPacketCount = 0;

	 while (!m_bEndCapture)
	{
		rc = recv(si_stream, packet, sizeof(packet), MSG_NOSIGNAL);
		//rc_count ++;

		if ( rc > 0 )
		{

			char* pData = packet;

			gvsp_hdr* gvsp_buffer = (gvsp_hdr*)pData;
			unsigned int packet_info = ntohl(gvsp_buffer->packet_info);
			unsigned short block_id = ntohs(gvsp_buffer->block_id);
			unsigned char packet_format = (unsigned char)(packet_info >> 24);
			unsigned int packet_id = packet_info & 0x00FFFFFF;

			gvsp_hdr_size = sizeof(gvsp_hdr) - sizeof(gvsp_buffer->data);
			data_packet_size = m_iPacketSize - NET_HEADERS - gvsp_hdr_size;

			if (packet_format == 1)
			{
				gettimeofday(&tv,NULL);

				iCurBlockID = block_id;		
				iLastPacketID = packet_id;		
				iPacketCount = 0;
				buffer = (char*)(m_pFrameBuffer+sizeof(yuv_video_buf));
			}	
			else if (packet_format == 2)
			{
				iFrameCount ++;
				if (block_id == iCurBlockID)
				{
					if (iPacketCount == m_iPacketsPerFrame)
					{
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
							AddFrame(1);
						}

						m_uSeq++;
						usleep(10*1);

						//iFrameGrabbed ++;
						printf("receive one frame m_uSeq=%d\r\n",m_uSeq);
					}
					else
						printf("=> Debug: <TAIL> iPacketCount=%d, PacketsPerFrame=%d,packet_id=%d, block_id=%d\r\n",
						iPacketCount, m_iPacketsPerFrame,packet_id, block_id);
				}
				else
					printf("=> Debug: <TAIL> frame out of sequence - current_block=%d, block_id=%d\r\n", iCurBlockID, block_id);
			}
			else if (packet_format == 3)
			{
				if (block_id == iCurBlockID)
				{
					iPacketCount++;

					if( buffer )
					{
						unsigned int payload_size = rc - gvsp_hdr_size;


						if ((packet_id >0) && (packet_id <= m_iPacketsPerFrame))
						{
							memcpy(&buffer[(packet_id-1)*data_packet_size],gvsp_buffer->data,payload_size);
						}
						else
						{
							printf ("error packed_id = %d\r\n", packet_id);
						}

						totBytesRecv += payload_size;
						iLastPacketID = packet_id;
					}

				}
				else
					printf("=> Debug: <DATA> packet out of squence - block_id=%d, iLastPacketID=%d, packet_id=%d \r\n", 
					block_id, iLastPacketID, packet_id);				

				iLastPacketID = packet_id;

			}
			else
			{
				printf("=> Debug: <WRONG> packet_format=%d, block_id = %d, packet_id=%d \r\n", packet_format, block_id, packet_id);
			}

		}
		else
		{
			//printf("=> Debug: Error - recv return %d\r\n", rc);
		}
	}

	return true;
}

//IP字符串转换成十六进制
string ZebraTwo::StringIpToHex(string strIp)
{
	char chIp[16] = {0};
	strcpy(chIp, strIp.c_str());
	int arr[4] = {0};
	char des[10] = {0};
	string stringIp = ("");
	sscanf(chIp,"%d.%d.%d.%d",&arr[0],&arr[1],&arr[2],&arr[3]);
	sprintf(des,"0x%02x%02x%02x%02x",arr[0],arr[1],arr[2],arr[3]);
	printf("StringIpToHex s => %s",chIp,des);
	stringIp = des;

	return stringIp;
}