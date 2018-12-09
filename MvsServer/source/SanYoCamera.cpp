// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "SanYoCamera.h"
#include "Common.h"
#include "VmuxSocket.h"
#include "CSeekpaiDB.h" 

//图像采集线程
void* ThreadSanYoCapture(void* pArg)
{
    //取类指针
    CSanYoCamera* pSanYoCamera = (CSanYoCamera*)pArg;
    if(pSanYoCamera == NULL)
        return pArg;

    pSanYoCamera->CaptureFrame();
	printf("=====ThreadSanYoCapture=pthread_exit=\n");
    pthread_exit((void *)0);
    return pArg;
}

CSanYoCamera::CSanYoCamera(int nCameraType)
{
    m_nCameraType = nCameraType;

    m_nWidth = 1920;
    m_nHeight = 1080;

	memset(m_channelPlace, '\0', sizeof(m_channelPlace));

    m_nMarginX = 0;
    m_nMarginY = 0;

    m_pFrameBuffer = NULL;
    m_nTcpFd = -1;
    m_nUdpFd = -1;
    m_bEndCapture = false;
    //线程ID
    m_nThreadId = 0;

    m_strHost = "192.168.0.2";

    m_nTcpPort = 80;
    m_nUdpPort = 3939;

	m_uGop = 0;
	m_strUserName = "admin";
	m_strPassWord = "admin";
}

CSanYoCamera::~CSanYoCamera()
{

}

void CSanYoCamera::Init()
{

    for(int i=0; i< MAX_SIZE; i++)
    {
        m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3,sizeof(unsigned char));
    }

    m_uSeq = 0;
    m_nPreFieldSeq  = 0;
    m_nCurIndex = 0;
    m_nFirstIndex = 0;
    m_nFrameSize=0;
    m_pFrameBuffer = m_FrameList[0];


    m_bEndCapture = false;
    //线程ID
    m_nThreadId = 0;

     #ifdef H264_DECODE
    m_Decoder.SetVideoSize(m_nWidth,m_nHeight);
    m_Decoder.SetVideoCodeID(0);
    m_Decoder.InitDecode(NULL);
    #endif

    m_strHost = g_monitorHostInfo.chMonitorHost;
    m_nUdpPort = g_monitorHostInfo.uMonitorPort;
    m_strUserName = g_monitorHostInfo.chUserName;
    m_strPassWord = g_monitorHostInfo.chPassWord;

#ifdef VMUX_PROC
	//开启与VIS通信的服务器
	if (!CVmuxSocket::GetInstance()->Init())
	{
		LogNormal("启动与VIS通信的服务器失败! \r\n");
	}
	LogTrace("vmux.log","start mux service success...");
#endif 

    LogNormal("ip=%s,port=%d,user=%s,pass=%s\n",m_strHost.c_str(),m_nUdpPort,m_strUserName.c_str(),m_strPassWord.c_str());

    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    int nret=pthread_create(&m_nThreadId,NULL,ThreadSanYoCapture,this);

    if(nret!=0)
    {
        Close();
        //失败
        LogError("创建SanYoCamera采集线程失败,无法进行采集！\r\n");
    }

    pthread_attr_destroy(&attr);
}

//打开相机
bool CSanYoCamera::Open()
{
     //读取相机参数
    m_cfg.uKind = m_nCameraType;
    LoadCameraProfile(m_cfg);
    m_cfg.nFrequency = 25;
  //  ConvertCameraParameter(m_cfg,false,true);

    Init();

    //http控制
    if(!HttpControl())
    {
        LogError("http控制出错，无法与相机通讯\r\n");
        //return false;
    }

    if(!connect_udp())
    {
        //失败
        LogError("无法与相机建立udp连接\r\n");
        //return false;
    }

    //发送数据请求
    SYO_COMMAND cmd;
    cmd.uLength = htons(0x4);
    cmd.uCommand = htons(0x7001);
    if(!UdpControl(cmd))
    {
        LogError("udp控制出错，无法与相机通讯\r\n");
        //return false;
    }


    //相机参数设置
    //SetCaMeraPara();

    return true;
}

void CSanYoCamera::close_tcp()
{
    if(m_nTcpFd!=-1)
    {
        shutdown(m_nTcpFd,2);
        close(m_nTcpFd);
        m_nTcpFd = -1;
    }
}

void CSanYoCamera::close_udp()
{
    if(m_nUdpFd!=-1)
    {
        shutdown(m_nUdpFd,2);
        close(m_nUdpFd);
        m_nUdpFd = -1;
    }
}

bool CSanYoCamera::Close()
{
    m_bEndCapture = true;

    //通知结束发送
    SYO_COMMAND cmd;
    cmd.uLength = htons(0x4);
    cmd.uCommand = htons(0x7002);
    UdpControl(cmd);

    close_udp();

    close_tcp();

    if (m_nThreadId != 0)
    {
        //pthread_cancel(m_nThreadId);
        pthread_join(m_nThreadId, NULL);
        m_nThreadId = 0;
    }

    #ifdef H264_DECODE
    m_Decoder.UinitDecode();
    #endif

#ifdef VMUX_PROC
	CVmuxSocket::GetInstance()->Unit();
#endif

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

    return true;
}


//TCP 连接
bool CSanYoCamera::connect_tcp()
{
    struct sockaddr_in tcp_addr;
    // socket
    if ((m_nTcpFd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        return false;
    }

    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec=1;
    timeo.tv_usec=500000;//超时1.5s


    if(setsockopt(m_nTcpFd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
    {
        close_tcp();
        perror("setsockopt");
        return false;
    }

    if(setsockopt(m_nTcpFd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {
        close_tcp();
        perror("setsockopt");
        return false;
    }

    bzero(&tcp_addr,sizeof(tcp_addr));
    tcp_addr.sin_family=AF_INET;
    tcp_addr.sin_addr.s_addr=inet_addr(m_strHost.c_str());
    tcp_addr.sin_port=htons(m_nTcpPort);

    int nCount = 0;
    while(nCount < 3)//连接不上最多重试三次
    {
        if(connect(m_nTcpFd,(struct sockaddr *)&tcp_addr,sizeof(struct sockaddr))==-1)
        {
            if(nCount == 2)
            {
                perror("connect");
          //      LogError("connect:%s\n",strerror(errno));
                if(m_nTcpFd!=-1)
                {
                   close_tcp();
                }
                return false;
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
    printf("connect_tcp ok\n");
    return true;
}

//UDP 连接
bool CSanYoCamera::connect_udp()
{
    /* connect */
    struct sockaddr_in udp_addr;
    bzero(&udp_addr,sizeof(udp_addr));
    udp_addr.sin_family=AF_INET;
    udp_addr.sin_addr.s_addr= htonl(INADDR_ANY);
    udp_addr.sin_port=htons(12450);

    // socket
    if ((m_nUdpFd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        return false;

    }

    int nLen = 81920000; //接收缓冲区大小(8M)
//设置接收缓冲区大小
    if (setsockopt(m_nUdpFd,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
    {
        close_udp();
        return false;
    }

    int on = 1;
    if(setsockopt(m_nUdpFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
    {
        close_udp();
        perror("setsockopt");
        return false;
    }
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0; //1ms
	if(setsockopt(m_nUdpFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
	{
		close_udp();
		perror("setsockopt");
		return false;
	}

    if(bind(m_nUdpFd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) == -1)
    {
        close_udp();
      //  LogError("YUV视频流接收地址无法绑定!\r\n");
        return false;
    }
	//LogTrace("sanyo.log", "connect_udp ok Time:%s\n", GetTimeCurrent().c_str());
    return true;
}

//手动控制
int CSanYoCamera::ManualControl(CAMERA_CONFIG  cfg)
{
    if(cfg.nIndex == ZOOM_FAR)
    {
        ZoomControl(cfg);
    }

    if(cfg.nIndex == ZOOM_NEAR)
    {
        ZoomControl(cfg);
    }

    if(cfg.nIndex == SET_PRESET)
    {
        int ndata = (int)cfg.fValue;
        SetPreSet(ndata);
    }

    if(cfg.nIndex == GOTO_PRESET)
    {
        int ndata = (int)cfg.fValue;
        GotoPreSet(ndata);
    }
	if(cfg.nIndex == SWITCH_CAMERA)
	{
		ZoomControl(cfg);
	}
    return 1;
}

//自动控制
int CSanYoCamera::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
{
    return true;
}

//切换工作方式
bool CSanYoCamera::ChangeMode()
{
    return true;
}

//将客户端设置的相机参数转换为相机自身能识别的参数
void CSanYoCamera::ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad)
{
}

//获取相机默认模板
void CSanYoCamera::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{
}

//相机控制
bool CSanYoCamera::Control(CAMERA_CONFIG cfg)
{
    return true;
}

//镜头远近缩放
bool CSanYoCamera::ZoomControl(CAMERA_CONFIG  cfg)
{
	char buffer[4096]={0};
    if(!connect_tcp())
    {
        LogError("无法与相机建立tcp连接");
        return false;
    }

    sprintf(buffer,"GET /cgi-bin/cam_set.cgi?status=1 HTTP/1.1\r\nUser-Agent: SY_CGISession2\r\nHost: %s\r\nConnection: Keep-Alive\r\nAuthorization: Basic %s\r\n\r\n",m_strHost.c_str(),m_strBase64.c_str());
    //printf("buffer=%s",buffer);

    if(!SendTcpData(buffer))
    {
        LogError("发送http请求2失败");
        close_tcp();
        return false;
    }

    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http请求2响应失败");
        close_tcp();
        return false;
    }
    //printf("buffer=%s\n",buffer);
    //获取cookies
    std::string response(buffer);
    int nPos = response.find("NOBSESS=");
    nPos += 8;

    m_strCookie = response.substr(nPos,32);

    int cmd = 0;

    if(cfg.nIndex == ZOOM_NEAR)
    {
        cmd = 9;
    }
    else if(cfg.nIndex == ZOOM_FAR)
    {
        cmd = 10;
    }

    if(cfg.nOperationType == 2)
    {
        cmd = 11;
    }

    if (cfg.nIndex == ZOOM_NEAR || cfg.nIndex == ZOOM_FAR)
    {
		if (cfg.fValue > 4.0 || cfg.fValue < 1.0)
		{
			cfg.fValue = 1.0;
		}
    }
	
   
	if(cfg.nIndex == ZOOM_FAR || cfg.nIndex == ZOOM_NEAR)
     sprintf(buffer,"GET /cgi-bin/opecmd.cgi?ope=51&cmd=%d&z_speed=%d HTTP/1.1\r\nAuthorization: Basic %s\r\nUser-Agent: SY_CGISession2\r\nHost: %s\r\nCookie: NOBSESS=%s\r\n\r\n",cmd,(int)cfg.fValue,m_strBase64.c_str(),m_strHost.c_str(),m_strCookie.c_str());
	else if(cfg.nIndex == SWITCH_CAMERA)
	 sprintf(buffer,"GET /cgi-bin/opecmd.cgi?ope=52&num=%d HTTP/1.1\r\nConnection: Keep-Alive\r\nCookie: NOBSESS=%s\r\nAuthorization: Basic %s\r\n\r\n", (int)cfg.fValue,m_strCookie.c_str(),m_strBase64.c_str());

	 LogTrace("vmux.log","cgi cmd=%s\n",buffer);
    if(!SendTcpData(buffer))
    {
        LogError("发送http控制失败");
        close_tcp();
        return false;
    }
    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http控制响应失败");
        close_tcp();
        return false;
    }
    printf("buffer=%s\n",buffer);
    close_tcp();

    return true;
}


//保存预置位(范围1-8)
bool CSanYoCamera::SetPreSet(int cmd)
{
    char buffer[4096];
    memset(buffer,0,sizeof(buffer));
    if(!connect_tcp())
    {
        LogError("无法与相机建立tcp连接");
        return false;
    }

    sprintf(buffer,"GET /cgi-bin/cam_set.cgi?status=1 HTTP/1.1\r\nUser-Agent: SY_CGISession2\r\nHost: %s\r\nConnection: Keep-Alive\r\nAuthorization: Basic %s\r\n\r\n",m_strHost.c_str(),m_strBase64.c_str());
    //printf("buffer=%s",buffer);

    if(!SendTcpData(buffer))
    {
        LogError("发送http请求2失败");
        close_tcp();
        return false;
    }

    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http请求2响应失败");
        close_tcp();
        return false;
    }
    //printf("buffer=%s\n",buffer);
    //获取cookies
    std::string response(buffer);
    int nPos = response.find("NOBSESS=");
    nPos += 8;

    m_strCookie = response.substr(nPos,32);

    sprintf(buffer,"GET /cgi-bin/opecmd.cgi?ope=8&registration=%d HTTP/1.1\r\nAuthorization: Basic %s\r\nUser-Agent: SY_CGISession2\r\nHost: %s\r\nCookie: NOBSESS=%s\r\n\r\n",cmd,m_strBase64.c_str(),m_strHost.c_str(),m_strCookie.c_str());

    if(!SendTcpData(buffer))
    {
        LogError("发送http控制失败");
        close_tcp();
        return false;
    }
    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http控制响应失败");
        close_tcp();
        return false;
    }
    close_tcp();
    return true;
}

//读取预置位(范围1-8)
bool CSanYoCamera::GotoPreSet(int cmd)
{
    char buffer[4096];
    memset(buffer,0,sizeof(buffer));
    if(!connect_tcp())
    {
        LogError("无法与相机建立tcp连接");
        return false;
    }

    sprintf(buffer,"GET /cgi-bin/cam_set.cgi?status=1 HTTP/1.1\r\nUser-Agent: SY_CGISession2\r\nHost: %s\r\nConnection: Keep-Alive\r\nAuthorization: Basic %s\r\n\r\n",m_strHost.c_str(),m_strBase64.c_str());
    //printf("buffer=%s",buffer);

    if(!SendTcpData(buffer))
    {
        LogError("发送http请求2失败");
        close_tcp();
        return false;
    }

    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http请求2响应失败");
        close_tcp();
        return false;
    }
    //printf("buffer=%s\n",buffer);
    //获取cookies
    std::string response(buffer);
    int nPos = response.find("NOBSESS=");
    nPos += 8;

    m_strCookie = response.substr(nPos,32);

    sprintf(buffer,"GET /cgi-bin/opecmd.cgi?ope=11&execution=%d HTTP/1.1\r\nAuthorization: Basic %s\r\nUser-Agent: SY_CGISession2\r\nHost: %s\r\nCookie: NOBSESS=%s\r\n\r\n",cmd,m_strBase64.c_str(),m_strHost.c_str(),m_strCookie.c_str());

    if(!SendTcpData(buffer))
    {
        LogError("发送http控制失败");
        close_tcp();
        return false;
    }
    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http控制响应失败");
        close_tcp();
        return false;
    }
    close_tcp();
    return true;
}


//发送tcp数据
bool CSanYoCamera::SendTcpData(char *buff)
{
    if(m_nTcpFd < 0)
    {
        return false;
    }

    int res;

    res=send(m_nTcpFd,buff,strlen(buff),MSG_NOSIGNAL);

    if(res<=0)
    {
       close_tcp();

       return false;
    }

    return true;
}


//接收tcp数据
bool CSanYoCamera::RecvTcpData(char *buff)
{
    if(m_nTcpFd < 0)
    {
        return false;
    }

    int res;

    res=recv(m_nTcpFd,buff,4096,MSG_NOSIGNAL);

    if(res<=0)
    {
       close_tcp();

       return false;
    }

    return true;
}


//reconnect the camera
bool CSanYoCamera::ReOpen()
{
    close_tcp();
    //http控制
    if(!HttpControl())
    {
        LogError("http控制出错，无法与相机通讯\r\n");
        return false;
    }

    close_udp();
    if(!connect_udp())
    {
        //失败
        LogError("无法与相机建立udp连接\r\n");
        return false;
    }

    //发送数据请求
    SYO_COMMAND cmd;
    cmd.uLength = htons(0x4);
    cmd.uCommand = htons(0x7001);
    if(!UdpControl(cmd))
    {
        LogError("udp控制出错，无法与相机通讯\r\n");
        return false;
    }
    LogNormal("相机自动重新连接");
    return true;
}

//HTTP 控制
bool CSanYoCamera::HttpControl()
{
    char buffer[4096] = {0};

    //http请求2
    if(!connect_tcp())
    {
        LogError("无法与相机建立tcp连接");
        return false;
    }

    memset(buffer,0,sizeof(buffer));

    sprintf(buffer,"%s:%s",m_strUserName.c_str(),m_strPassWord.c_str());
    std::string request(buffer);

    m_strBase64.clear();
    EncodeBase64(m_strBase64,(unsigned char*)request.c_str(),request.size());

    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"GET /cgi-bin/cam_set.cgi?status=1 HTTP/1.1\r\nUser-Agent: SY_CGISession1\r\nHost: %s\r\nConnection: Keep-Alive\r\nAuthorization: Basic %s\r\n\r\n",m_strHost.c_str(),m_strBase64.c_str());
    printf("buffer=%s",buffer);

    if(!SendTcpData(buffer))
    {
        LogError("发送http请求2失败");
        close_tcp();
        return false;
    }

    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http请求2响应失败");
        close_tcp();
        return false;
    }
    //close_tcp();
    printf("buffer=%s\n",buffer);
    //获取cookies
    std::string response(buffer);
    int nPos = response.find("NOBSESS=");
    nPos += 8;

    m_strCookie = response.substr(nPos,32);

    //http请求3
   /* if(!connect_tcp())
    {
        LogError("无法与相机建立tcp连接");
        return false;
    }*/

    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"GET /cgi-bin/codec_stream.cgi?status=1 HTTP/1.1\r\nUser-Agent: SY_CGISession1\r\nHost: %s\r\nCookie: NOBSESS=%s\r\nAuthorization: Basic %s\r\n\r\n",m_strHost.c_str(),m_strCookie.c_str(),m_strBase64.c_str());


    if(!SendTcpData(buffer))
    {
        LogError("发送http请求3失败");
        close_tcp();
        return false;
    }

    memset(buffer,0,sizeof(buffer));
    if(!RecvTcpData(buffer))
    {
        LogError("接收http请求3响应失败");
        close_tcp();
        return false;
    }
    close_tcp();
    return true;
}

//UDP 控制
bool CSanYoCamera::UdpControl(SYO_COMMAND cmd)
{
    if(m_nUdpFd < 0)
    {
        return false;
    }
 
    /* 设置对方地址和端口信息 */
    struct sockaddr_in s_addr;
    int addr_len = sizeof(s_addr);

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(m_nUdpPort);
    s_addr.sin_addr.s_addr = inet_addr(m_strHost.c_str());

    int nLen = sendto(m_nUdpFd, &cmd, sizeof(SYO_COMMAND), MSG_NOSIGNAL,
    (struct sockaddr *) &s_addr, addr_len);
    if (nLen < 0)
    {
        LogTrace("sanyo.log", "\n\rsend error.\n\r");
        return false;
    }
    if(cmd.uCommand == htons(0x7001))
    {
        /* 接收数据HDNC */
        socklen_t slen;

        char buffer[4096];
        memset(buffer,0,sizeof(buffer));
        nLen = recvfrom(m_nUdpFd, buffer, sizeof(buffer) - 1, MSG_NOSIGNAL, \
        (struct sockaddr *) &s_addr, &slen);
        if (nLen < 0)
        {
          perror("recvfrom");

           return false;
        }

        short uFrameRate = 0;
        short uImageWidth = 0;
        short uImageHeight = 0;
        ID_HEADER header = *((ID_HEADER*)buffer);
        if(ntohs(header.uDataID) == 0x7100)
        {
            uFrameRate = *((short*)(buffer+24));
            uFrameRate = ntohs(uFrameRate);

            uImageWidth = *((short*)(buffer+28));
            uImageWidth = ntohs(uImageWidth);

            uImageHeight = *((short*)(buffer+30));
            uImageHeight = ntohs(uImageHeight);

			 m_uGop = *((unsigned short *)(buffer + 22));//GOP信息
			 m_uGop = ntohs(m_uGop);
        }

       // LogTrace("sanyo.log","nLen=%d,uFrameRate=%d,uImageWidth=%d,uImageHeight=%d\n",nLen,uFrameRate,uImageWidth,uImageHeight);
    }

    return true;
}

void CSanYoCamera::CaptureFrame()
{
    char buffer[9000] = {0};
    unsigned char* pData = new unsigned char[m_nWidth*m_nHeight/4];

    int nLen = 0;

    struct timeval tv;

    short uPakDataNum = 0;
    unsigned char nSeqNum = 0;
    unsigned char nSplit = 0;
	unsigned char uFrameType=0;//0 I帧 其它：P帧 
    unsigned char uZoomPosition = 0;//焦距位置
    short uZoomInfo=0;
    int nFrameSize = 0;

    /* 设置对方地址和端口信息 */
    struct sockaddr_in s_addr;
    socklen_t addr_len;

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(m_nUdpPort);
    s_addr.sin_addr.s_addr = inet_addr(m_strHost.c_str());

printf("====before===CSanYoCamera::CaptureFrame()=m_nUdpFd=%d=m_bEndCapture=%d\n",m_nUdpFd,m_bEndCapture);
    while (!m_bEndCapture)
    {
        if(m_nUdpFd!=-1)
        {
            memset(buffer, 0, 9000);

            nLen = recvfrom(m_nUdpFd, buffer, sizeof(buffer), MSG_NOSIGNAL,(struct sockaddr *) &s_addr, &addr_len);

            //printf("==============n =  %d\n",n);
            if (nLen >0 && !m_bEndCapture)
            {
                ID_HEADER header = *((ID_HEADER*)buffer);

                if(strncmp(header.chHNCV,"HNCV",4) == 0)
                {
                    if(ntohs(header.uDataID) == 0x7101)//首个数据包
                    {
                        nSplit = *((unsigned char*)(buffer+16));//是否某个帧的最后一个数据包1:has next
                        nSeqNum = *((unsigned char*)(buffer+36));//帧号（0－255）

                        nFrameSize = 0;
                        gettimeofday(&tv,NULL);//获取接收帧时间

                        uPakDataNum = *((short*)(buffer+38));
                        uPakDataNum = ntohs(uPakDataNum);

							uZoomPosition = *((unsigned char *)(buffer + 21));
							uZoomInfo = *((short *)(buffer + 26));
							uZoomInfo = ntohs(uZoomInfo);


							int n = (header.uLength - 48)/8;
							if(n > 0)
							{
								uFrameType = *((unsigned char *)(buffer + 48 + n*4));
							}
							// LogTrace(NULL, "Frame Type is:%d, Zoom Position %d, Zoom Info: %d", (int)uFrameType, (int)uZoomPosition, (int)uZoomInfo);
                    }
                    else if(ntohs(header.uDataID) == 0x7102)
                    {
                        nSplit = *((unsigned char*)(buffer+8)); //是否某个帧的最后一个数据包，0:last
                        nSeqNum = *((unsigned char*)(buffer+12));//帧号（0－255）

                        uPakDataNum = *((short*)(buffer+14));
                        uPakDataNum = ntohs(uPakDataNum);

                    }

                    if (m_pFrameBuffer != NULL && !m_bEndCapture)
                    {
                        memcpy(pData+nFrameSize,buffer+ntohs(header.uLength),uPakDataNum);
                        nFrameSize += uPakDataNum;

                        //printf("nSeqNum=%d,nSplit=%d,uPakDataNum=%d,nFrameSize=%d\n",nSeqNum,nSplit,uPakDataNum,nFrameSize);

                        if(nSplit == 0)
                        {
                            //此处需要解码
                            int nSize = 0;

                            bool bRet = false;

                            #ifdef H264_DECODE
                            bRet = m_Decoder.DecodeFrame(pData,nFrameSize,m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
                            #endif

                            //printf("bRet=%d,nSize=%d,nFrameSize=%d\n",bRet,nSize,nFrameSize);
                            if(bRet&& nSize > 0)
                            {
                                yuv_video_buf header;
                                header.ts = tv.tv_sec*1000000+tv.tv_usec;//(int64_t)pts*1000*1000;
                                header.nFrameRate = 25;
                                header.width = m_nWidth;
                                header.height = m_nHeight;
								header.nSeq = m_uSeq;

                                memcpy(m_pFrameBuffer,&header,sizeof(yuv_video_buf));

                                AddFrame(1);
								//向VIS发送数据
#ifdef VMUX_PROC
								VissHead visHead;
								string msg;
								memset(&visHead, 0, sizeof(VissHead));
								memcpy(visHead.m_Heads, ">>>>", 4);
                                visHead.m_ChipType = 3;
								visHead.m_PackType = 0x12;
								visHead.m_FrameRate = 25;
								visHead.m_CompressType = (uFrameType << 4) + 6;
								visHead.m_DataRate = 25;
								visHead.m_SizeH = m_nWidth;
								visHead.m_SizeV = m_nHeight;
								visHead.m_PackSize = sizeof(VissHead) + nFrameSize;
								visHead.m_GOP = m_uGop;
								visHead.m_PackCount = 1;
								visHead.m_PackID = 0;
                                visHead.m_FrameID = m_uSeq;
								visHead.m_CheckSum = CheckSum(&visHead);
								visHead.m_TimeStampSec = time(NULL);
                                visHead.m_TimeStampuSec = tv.tv_usec;
								int Frame = sizeof(VissHead) + nFrameSize;
								visHead.m_Reserve1 = (Frame & 0xFFFF);//帧长度低位
								visHead.m_Reserve2 = (Frame >> 16);//帧长度高位
								visHead.m_MultiP = uZoomInfo;//倍率
								printf("++++++++++++++++++++++++++++++++++++++++++++++++=m_channelPlace%s\n",m_channelPlace);
								if(strlen(m_channelPlace) == 0)
								{
									GetChannelPlace();
								}
								strcpy(visHead.m_InforChar, m_channelPlace);//通道地址
								printf("+++++++++++++++++++++++visHead.m_InforChar=%s=%s\n",visHead.m_InforChar,m_channelPlace);
								msg.append((char *)&visHead, sizeof(VissHead));
								msg.append((char *)pData, nFrameSize);

								CVmuxSocket::GetInstance()->SendMsg(msg);
#endif
                            }

                            if(m_uSeq%100 == 0)//每隔一定的时间通知一次
                            {
                                //通知继续发送
                                SYO_COMMAND cmd;
                                cmd.uLength = htons(0x4);
                                cmd.uCommand = htons(0x7003);
                                UdpControl(cmd);
                                //LogNormal("发送继续获取视频命令");
                            }

                            m_uSeq++;

                           usleep(1000*1);
                        }
                    }
                }
            }
        }
		else
		{
			usleep(1000*100);
		}
    }
printf("====after===CSanYoCamera::CaptureFrame()=m_nUdpFd=%d=m_bEndCapture=%d\n",m_nUdpFd,m_bEndCapture);
    if(pData)
    {
        delete []pData;
        pData = NULL;
    }
}

//获取三洋相机通道地址
void CSanYoCamera::GetChannelPlace(void)
{
	memset(m_channelPlace, '\0', sizeof(m_channelPlace));
	string tmpStr = g_skpDB.GetChannelList();
	printf("=======CSanYoCamera::GetChannelPlace()=tmpStr=%s\n",tmpStr);
	SRIP_CHANNEL* sChannel = (SRIP_CHANNEL*)tmpStr.c_str();
	printf("=======CSanYoCamera::GetChannelPlace()=sChannel->chPlace=%s\n",sChannel->chPlace);
	memcpy(m_channelPlace,sChannel->chPlace,strlen(sChannel->chPlace));
	printf("=======CSanYoCamera::GetChannelPlace()=m_channelPlace=%s\n",m_channelPlace);
}

#ifdef VMUX_PROC
//与VIS通信时取校验
int CSanYoCamera::CheckSum(const VissHead *visHead)
{
	int sum = 0;
    unsigned char * head = (unsigned char *)visHead;
	for (int i = 0; i<30; i++)
	{
		sum += (int)(head[i]);
	}

	return sum & 0xff;
}

#endif