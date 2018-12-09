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

#include "RoadVideoYuv.h"
#include "Common.h"
#include "CommonHeader.h"
#include <math.h>

//#define DEBUG_VIDEOYUV
//#define TEMPERATURE_TEST
//#define CAMERA_CONTROL
#define BOCOM_GIGE_MAX_PE 40000//BOCOM_GIGE相机自身最大曝光值
#define BOCOM_GIGE_MAX_GAIN 35//BOCOM_GIGE相机自身最大增益值
#define QBOX_HEADER_SIZE 28
#define BE32(a) ((((a)>>24)&0xFF)|(((a)>>8)&0xFF00)| (((a)<<8)&0xFF0000)|((((a)<<24))&0xFF000000))

//构造
CSkpRoadVideoYuv::CSkpRoadVideoYuv(int nCameraType,int nPixelFormat)
{   
	m_nPixelFormat = nPixelFormat;
	m_nCameraType =  nCameraType;

	if (m_nPixelFormat == VEDIO_H264)
	{
		m_strCameraIP =  "192.168.60.98";              ;
		m_nCameraPort = 4501;

        m_nCamID =1;
		

		m_nTcpCameraPort = 8888;
		
	}
	
   else
   {
	    m_nPixelFormat = nPixelFormat;
        m_nCameraType =  nCameraType;
	    m_strCameraIP = "";
	    m_nCameraPort = 0;
	     m_bComRet = true;
		    m_nCamID = 0;
	    m_strCameraMultiIP = "";
	  
   }
   
	m_bControlOpen = true;
	

	if(nCameraType == JAI_CAMERA_LINK_FIELD)
    {
        m_nWidth = 1920;
        m_nHeight = 540;
        m_nMarginX = 160;
        m_nMarginY = 0;
    }
    else if(nCameraType == JAI_CAMERA_LINK_FIELD_P)
    {
        m_nWidth = 1920;
        m_nHeight = 1080;
        m_nMarginX = 160;
        m_nMarginY = 0;
    }
	else if(nCameraType == BOCOM_301_200)
	{
		if (m_nPixelFormat ==VEDIO_H264)
		{
			m_nWidth = 1920;
			m_nHeight = 1080;

			m_nMarginX = 0;
			m_nMarginY = 0;
		}
		else
		{	
		  if(1)
		 {
			m_nWidth = 1600;
			m_nHeight = 1200;
			m_nMarginX = 0;
			m_nMarginY = 0;
		 }
		 else
		 {
			m_nWidth = 1620;
			m_nHeight = 1236;
			m_nMarginX = 0;
			m_nMarginY = 0;
		 }
		}
	}
	else if(nCameraType == BOCOM_302_500)
	{
		m_nWidth = 2400;
		m_nHeight = 2000;
		m_nMarginX = 0;
       m_nMarginY = 0;
	}
	else if(nCameraType == BOCOM_301_500)
	{
		m_nWidth = 2400;
		m_nHeight = 2000;
		m_nMarginX = 0;
        m_nMarginY = 0;
	}
    else if((nCameraType==JAI_CAMERA_LINK_FRAME))
    {
         m_nWidth = 1620;
         m_nHeight = 1236;
         m_nMarginX = 10;
         m_nMarginY = 18;
    }
	else if(nCameraType==JAI_CL_500)
    {
		m_nWidth = 2448;
        m_nHeight = 2048;
        m_nMarginX = 0;
        m_nMarginY = 0;
	}
	else if(nCameraType == BOCOM_302_200)
	{
		m_nWidth = 1920;
		m_nHeight = 1080;
		m_nMarginX = 0;
		m_nMarginY = 0;
	}
	
	


    m_pFrameBuffer = NULL;
//    crdL=malloc(sizeof(RegionCoordinate));

	//线程结束标志
	m_bEndCapture = false;
	//线程ID
	m_nThreadId = 0;
	m_nYuvFd = -1;
    m_uShutterModeTime = 0;
	//m_nMaxBufferSize = MAX_SIZE;
	m_strControlIP ="";

	if (m_nPixelFormat == VEDIO_H264)
	{
		m_nMaxBufferSize = MAX_SIZE ;     
	} 
	else
	{
		m_nMaxBufferSize = 2 ;
	}
	m_bUnicast = false;
#ifdef DIO_RTSP
	m_bisOk = false;
#endif
}
//析构
CSkpRoadVideoYuv::~CSkpRoadVideoYuv()
{

}


void* ThreadVideoH264Capture(void* pArg)
{
	//取类指针
	CSkpRoadVideoYuv*  pRoadVideoYuv = (CSkpRoadVideoYuv*)pArg;
	if(pRoadVideoYuv == NULL)
		return pArg;

	pRoadVideoYuv->GetNextH264frame( );
	pthread_exit((void *)0);
	return pArg;
}

//YUV采集线程
void* ThreadVideoYuv(void* pArg)
{
	//取类指针
	CSkpRoadVideoYuv* pRoadVideoYuv = (CSkpRoadVideoYuv*)pArg;

	if(pRoadVideoYuv == NULL) return pArg;

	pRoadVideoYuv->DealYuvFrame();
    pthread_exit((void *)0);
	return pArg;
}

void CSkpRoadVideoYuv::Init()
{   
	
	if (m_nPixelFormat == VEDIO_H264)
	{
		int nret = -1;
		int i;
		m_bEndCapture = false;
		//m_bFirstFrame = true;

		for(i=0;i<m_nMaxBufferSize;i++)
		{
			m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3,sizeof(unsigned char));
		}
		m_pFrameBuffer = m_FrameList[0];
		m_uSeq = 0;
		m_nPreFieldSeq  = 0;
		m_nCurIndex = 0;
		m_nFirstIndex = 0;
		m_nFrameSize=0;

#ifdef H264_DECODE
		m_Decoder.SetVideoSize(m_nWidth,m_nHeight);
		m_Decoder.SetVideoCodeID(0);
		m_Decoder.InitDecode(NULL);
#endif

		m_nThreadId = 0;
		pthread_attr_t   attr;
		//初始化
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

		nret=pthread_create(&m_nThreadId,NULL, ThreadVideoH264Capture,this);
		printf("Bocom301 m_nThreadId=%lld",m_nThreadId);

		if(nret!=0)
		{
			Close();
			//失败
			LogError("创建yuv采集线程失败,无法进行采集！\r\n");
		}

		pthread_attr_destroy(&attr);
	}

	else
   {
    int i=0;

    int nret = -1;
    m_bFirstFrame = true;
	//线程结束标志
	m_bEndCapture = false;

	m_bUseTestResult = false;

	if(access("UseTest.cfg",F_OK) == 0)
	{
		g_nFileID = -1;
		m_bUseTestResult = true;
	}

	//线程ID
	m_nThreadId = 0;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);

	struct   sched_param   param;
	pthread_attr_getschedparam(&attr,   &param);
	param.sched_priority   =   25;
	pthread_attr_setschedparam(&attr,   &param);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	{
            //开辟MAX_SIZE个内存块，其指针存储在m_FrameList数组中
            for(i=0;i<m_nMaxBufferSize;i++)
            {
				if(m_nPixelFormat == VIDEO_BAYER)
				m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight,sizeof(unsigned char));
				else
                m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*2,sizeof(unsigned char));
            }

            m_uSeq = 0; //帧号
            m_nPreFieldSeq  = 0;
            m_nCurIndex = 0;   //可存的内存块序号
            m_nFirstIndex = 0;//可取的内存块序号
            m_nFrameSize = 0;//已经存储的内存块个数
            m_pFrameBuffer = m_FrameList[0];

            //启动YUV采集线程
            nret=pthread_create(&m_nThreadId,&attr,ThreadVideoYuv,this);

            if(nret!=0)
            {
                //YUV流关闭
                Close();
                //失败
                LogError("创建yuv采集线程失败,无法进行采集！\r\n");
            }
	}
	pthread_attr_destroy(&attr);
	}
}

//处理YUV采集
void CSkpRoadVideoYuv::DealYuvFrame()
{
	#ifdef TEMPERATURE_TEST
	UINT32 uTime1 = GetTimeStamp();//开始检测的时间
    UINT32 uTime2 = 0;
    UINT32 uPreTime = uTime1;
	UINT32 nframe = 0;
	#endif


    while(!m_bEndCapture)
	{
		int nRet = -1;

	   //获取一帧YUV流
	    if(m_bReadFile)
		{
		   nRet = GetNextFileframe();
		   if(nRet!=-1)
		   {
			   //采集成功
			  AddFrame( );
			}
			usleep(1000*1);
		}
		else
		{
		    if(m_nYuvFd > 0)
		    {
                nRet = GetNextframe();
                if(nRet ==1 )//只保存第一块
                {
                   //采集成功
                   AddFrame(nRet);
				    #ifdef TEMPERATURE_TEST
					nframe++;
					#endif

				   usleep(1000*1);
                }

				#ifdef TEMPERATURE_TEST
				uTime2 = GetTimeStamp();//每分钟统计一次
				if((uTime2%60==0)&&(uPreTime != uTime2))
				{
					std::string strTime = GetTime(uTime2);
					FILE * fp = fopen("avgrate.txt","a");
					double avgrate = (nframe*1.0)/(uTime2-uPreTime);
					fprintf(fp,"time = %s,avgrate=%f,nframe=%d,uTime2-uTime1=%ds\n",strTime.c_str(),avgrate,nframe,uTime2-uPreTime);//平均帧率
					fclose(fp);
					uPreTime = uTime2;
					nframe = 0;
				}
			 #endif

		    }
		}
	}
}



//打开文件
bool CSkpRoadVideoYuv::OpenFile(const char* strFileName)
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

bool  CSkpRoadVideoYuv::conect_set_tcp()
{
	struct sockaddr_in tcp_set_addr;
	// socket
	if ((tcp_set_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		return false;
	}

	struct timeval timeo;
	socklen_t len = sizeof(timeo);
	timeo.tv_sec=1;
	timeo.tv_usec=500000;//超时1.5s
	printf("tcp_set_fd=%d\n",tcp_set_fd);

	if(setsockopt(tcp_set_fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
	{
		shutdown(tcp_set_fd,2);
		close(tcp_set_fd);
		tcp_set_fd = -1;
		perror("setsockopt");
		return false;
	}

	if(setsockopt(tcp_set_fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
	{
		shutdown(tcp_set_fd,2);
		close(tcp_set_fd);
		tcp_set_fd = -1;
		perror("setsockopt");
		return false;
	}

	bzero(&tcp_set_addr,sizeof(tcp_set_addr));
	tcp_set_addr.sin_family=AF_INET;
	tcp_set_addr.sin_addr.s_addr=inet_addr(m_strCameraIP.c_str());

	tcp_set_addr.sin_port=htons(4501);

	//printf("#################m_strCameraIP = %s\n",inet_ntoa(tcp_set_addr.sin_addr.s_addr.c_str()));
	printf("#################m_nCameraPort = %d\n",ntohl(tcp_set_addr.sin_port));

	int nCount = 0;
	while(nCount < 3)//连接不上最多重试三次
	{
		if(connect(tcp_set_fd,(struct sockaddr *)&tcp_set_addr,sizeof(struct sockaddr))==-1)
		{
			if(nCount == 2)
			{
				perror("connect");
				//LogError("connect:%s\n",strerror(errno));
				if(tcp_set_fd!=-1)
				{
					shutdown(tcp_set_fd,2);
					close(tcp_set_fd);
					tcp_set_fd = -1;
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




	//接收初始值；//set_rev is Type '?' and return for help

	return true;
}

bool CSkpRoadVideoYuv::close_set_tcp()
{
	if(tcp_set_fd!=-1)
	{
		shutdown(tcp_set_fd,2);
		close(tcp_set_fd);
		tcp_set_fd = -1;
	}

	return true;
}

bool CSkpRoadVideoYuv::conect_data_tcp()
{
	struct sockaddr_in tcp_data_addr;
	// socket
	if ((tcp_data_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		return false;
	}

	struct timeval timeo;
	socklen_t len = sizeof(timeo);
	timeo.tv_sec=1;
	timeo.tv_usec=500000;//超时1.5s


	if(setsockopt(tcp_data_fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
	{
		shutdown(tcp_data_fd,2);
		close(tcp_data_fd);
		tcp_data_fd = -1;
		perror("setsockopt");
		return false;
	}

	if(setsockopt(tcp_data_fd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
	{
		shutdown(tcp_data_fd,2);
		close(tcp_data_fd);
		tcp_data_fd = -1;
		perror("setsockopt");
		return false;
	}

	bzero(&tcp_data_addr,sizeof(tcp_data_addr));
	tcp_data_addr.sin_family=AF_INET;
	tcp_data_addr.sin_addr.s_addr=inet_addr(m_strCameraIP.c_str());
	tcp_data_addr.sin_port=htons(m_nTcpCameraPort);

	int nCount = 0;
	while(nCount < 3)//连接不上最多重试三次
	{
		if(connect(tcp_data_fd,(struct sockaddr *)&tcp_data_addr,sizeof(struct sockaddr))==-1)
		{
			if(nCount == 2)
			{
				perror("connect");
				//LogError("connect:%s\n",strerror(errno));
				if(tcp_data_fd!=-1)
				{
					shutdown(tcp_data_fd,2);
					close(tcp_data_fd);
					tcp_data_fd = -1;
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
	printf("tcp_data_fd= %d\n",tcp_data_fd);
	//接收初始值；//set_rev is Type '?' and return for help

	return true;		
}

bool CSkpRoadVideoYuv::close_data_tcp()
{
	if(tcp_data_fd!=-1)
	{
		shutdown(tcp_data_fd,2);
		close(tcp_data_fd);
		tcp_data_fd = -1;
	}

	return true;
}

int CSkpRoadVideoYuv::StartSendStream()
{
	VMUX6KHEAD head;
	//BuildHead(head);
	head.bHead1=0xBF;
	head.bHead2=0xEC;
	head.bVersion=0x01;
	head.bFlag = 0x04;
	head.bMsgType = 0x1;
	head.bLength_h = 0x00;
	head.bLength_l = sizeof(SetStreamParam);
	head.bChecksum = ((head.bHead1 + head.bHead2 + head.bVersion + head.bFlag \
		+ head.bMsgType + head.bLength_h + head.bLength_l) & 0xff);

	unsigned char szBuf[sizeof(VMUX6KHEAD) + sizeof(SetStreamParam)] = {0};
    
	

	memcpy(szBuf, &head, sizeof(head));


	SetStreamParam Param;

	SetStreamParam* pStreamSendParam;
	pStreamSendParam = &Param;

	pStreamSendParam->iSlot=htons(0x1);
	pStreamSendParam->bChannel=0x1;
	pStreamSendParam->bStreamNo=0x1;
	pStreamSendParam->iProtocol=htonl(0x1);
	pStreamSendParam->iHeadFlag=htonl(0x1);
	pStreamSendParam->iPeerIP=inet_addr(g_ServerHost.c_str());
	pStreamSendParam->iPeerPort=htonl(8888);

	memcpy(szBuf + sizeof(head), pStreamSendParam, sizeof(SetStreamParam));

     /////////////////////发送控制命令////////////////////////////

	int iRet = SendData(szBuf, sizeof(szBuf));/////////////要改

	

	printf("#######kaishimaliu########htonl(0x1)=%d,ntohs(0x1)=%d,g_ServerHost.c_str()=%s\n",htonl(0x1),ntohs(0x1),g_ServerHost.c_str());

	if (iRet != 1)
	{
		printf("BCM_OpenStream StartSendStream BC_SEND_FAILED\r\n");
		return 0;
	}


	///////////////////////接受返回命令///////////////////////////////////
	unsigned char szCmdRet[sizeof(VMUX6KHEAD) + sizeof(CMDRESPONSE)] ={0};

	iRet = Recvdata( szCmdRet, sizeof(szCmdRet) );////////////要改
     
     printf("接收到 %d 个字节\n",iRet);

	if (iRet != sizeof(szCmdRet))
	{
		printf("BCM_OpenStream StartSendStream BC_RECV_FAILED\r\n");
		return 0;
	}

	/*if (CheckHead(*(VMUX6KHEAD*)(szCmdRet)) != 1)////////////要改
	{
		printf("BCM_OpenStream StartSendStream BC_CHECK_RECV_HEAD_FAILED\r\n");
		return 0;
	}*/

	int nRet = ntohl(*((unsigned int*)(szCmdRet + sizeof(VMUX6KHEAD) + 4)));
	if (nRet != 0)
	{
		printf("BCM_OpenStream StartSendStream ACK return %d  is not zero\r\n",nRet);
		return 0;
	}

    return 1;
}
int CSkpRoadVideoYuv::EndSendStream()
{
	VMUX6KHEAD head;

	head.bHead1=0xBF;
	head.bHead2=0xEC;
	head.bVersion=0x01;
	head.bFlag = 0x04;
	head.bMsgType = 0x3;
	head.bLength_h = 0x00;
	head.bLength_l = sizeof(SetStreamParam);
	head.bChecksum = ((head.bHead1 + head.bHead2 + head.bVersion + head.bFlag \
		+ head.bMsgType + head.bLength_h + head.bLength_l) & 0xff);

	unsigned char szBuf[sizeof(VMUX6KHEAD) + sizeof(SetStreamParam)] = {0};

	memcpy(szBuf, &head, sizeof(head));

	SetStreamParam Param;

	SetStreamParam* pStreamSendParam;
	pStreamSendParam = &Param;

	pStreamSendParam->iSlot=ntohs(0x1);
	pStreamSendParam->bChannel=0x1;
	pStreamSendParam->bStreamNo=0x1;
	pStreamSendParam->iProtocol=ntohl(0x1);
	pStreamSendParam->iHeadFlag=ntohl(0x1);
	pStreamSendParam->iPeerIP=inet_addr(g_ServerHost.c_str());
	pStreamSendParam->iPeerPort=ntohl(8888);

	memcpy(szBuf + sizeof(head), pStreamSendParam, sizeof(SetStreamParam));

	/////////////////////发送控制命令////////////////////////////

	int iRet = SendData(szBuf, sizeof(szBuf));/////////////要改


	if (iRet != 1)
	{
		printf("BCM_OpenStream StartSendStream BC_SEND_FAILED\r\n");
		return 0;
	}

	///////////////////////接受返回命令///////////////////////////////////
	unsigned char szCmdRet[sizeof(VMUX6KHEAD) + sizeof(CMDRESPONSE)] ={0};

	iRet = Recvdata( szCmdRet, sizeof(szCmdRet) );////////////要改

	printf("接收到 %d 个字节\n",iRet);

	if (iRet != sizeof(szCmdRet))
	{
		printf("BCM_OpenStream StartSendStream BC_RECV_FAILED\r\n");
		return 0;
	}
	return 1;

}
int CSkpRoadVideoYuv::SendData(unsigned char *buf, int len)
{
	//printf("tcp_set_fd = %d\n",tcp_set_fd);
	if(tcp_set_fd < 0)
	{
		if(tcp_set_fd!=-1)
		{
			shutdown(tcp_set_fd,2);
			close(tcp_set_fd);
			tcp_set_fd = -1;
		}
		return 0;
	}
	int res;
	//for(int i=0;i<28;i++)
	//printf("第 %d 字节 = %X\n",i,*(buf++));
	//printf("buf = %X\n",*buf);
	printf("#########fasongshuju########\n");
	res=send(tcp_set_fd,buf,len,MSG_NOSIGNAL);
	printf("send res = %d\n",res);
	if(res<=0)
	{
		//perror("send");
		printf("Camera set fail ,close connect\n");
		usleep(1000*100);
		if(tcp_set_fd!=-1)
		{
			shutdown(tcp_set_fd,2);
			close(tcp_set_fd);
			tcp_set_fd = -1;
		}
		if(tcp_data_fd!=-1)
		{
			shutdown(tcp_data_fd,2);
			close(tcp_data_fd);
			tcp_data_fd = -1;
		}
		return 0;
	}
	return 1;
}

int CSkpRoadVideoYuv::Recvdata(unsigned char *buf,int len)
{
	int nRet = 0;
	nRet = recv(tcp_set_fd,buf,len,MSG_NOSIGNAL);

	printf("nRet = %d ",nRet)  ;
	for(int i=0;i<16;i++)
		printf(" %02X",*(buf++));
	printf("\n");


	VMUX6KHEAD header = *(VMUX6KHEAD*)buf;
	unsigned char buffer[sizeof(CMDRESPONSE)]={0};

	memcpy(buffer, buf+sizeof(header), sizeof(CMDRESPONSE));

	CMDRESPONSE CMD = *(CMDRESPONSE*)&buffer;

	printf("cmd.iSlot=%X,cmd.iChannel=%X,cmd.iRet=%X \n",CMD.iSlot,CMD.iChannel,CMD.iRet);

	if (nRet<=0)
	{
		return -1;
	}
	else
		return nRet;
}

int CSkpRoadVideoYuv::GetNextH264frame( )
{
	//std::ofstream B("FRAMERATE.txt",std::ios::app);

	//printf("GetNextframe:::::::::::::::::::::::::::::::::\n");
	//接收标志
	int nFlag = -1;

	//每次接收的字节
	int bytes = 0;
	//接收数据位置
	
	//剩余接收的字节
   
	//h264大小
	int nSize = 0;

	int index1=0;
	nSize = MAX_LEN;

	int left = nSize;

	char buf[MAX_LEN]={'\0'};

	//采集时间(以微妙为单位)
	struct timeval tv;
	int64_t nSystemTime;
	unsigned int uTimeStamp;

	//printf("========before recvfrom======m_nYuvFd=%d,m_strHost=%s,m_nPort=%d\r\n",m_nYuvFd,m_strHost.c_str(),m_nPort);

	int fd;

	char bit=0x07;

// 	unsigned char *pH264 = (unsigned char*)malloc(1024*1024);

	//fd = open("./video80",O_CREAT|O_RDWR|O_APPEND,0770);
	//接收码流头
	//printf("############jieshoumaliu\n");
	while(!m_bEndCapture)
	{

		bytes = recv(tcp_data_fd, buf, MAX_LEN, MSG_NOSIGNAL);
#ifdef DIO_RTSP
		if(bytes > 0)
		{
			m_StreamParser.InputData((unsigned char *)buf,bytes);
		}

		if(!m_bisOk)
		{
			//获取码流的宽高
			sStrmInfo si;
			if(m_StreamParser.StrmInfo(si) == 0)
			{
				printf("*********si.uWidth=%d,si.uHeight=%d,si.nFrameRate=%d  **********\n",si.uWidth,si.uHeight,(int)si.fFrameRate);
				m_bisOk = true;
			}
			else
			{
				usleep(10*1000);
				continue;
			}
		}

		int iErr = m_StreamParser.NextFrame(&pMs);
		if(BM_ERR_OK == iErr && pMs)
		{
			//printf("***************uFrameIndex:%d  ***********\n",pMs->uFrameIndex);
			//解码
			bool bRet = m_Decoder.DecodeFrame(pMs->pData,pMs->iDataLen,m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
			yuv_video_buf  camera_header;
			if(bRet&& nSize > 0)
			{
				//printf("***************nSize:%d  ***********\n",nSize);
				//YUV头
				yuv_video_buf header;
				struct timeval tv;
				gettimeofday(&tv,NULL);
				camera_header.ts = tv.tv_sec*1000000+tv.tv_usec;
				camera_header.uTimestamp = tv.tv_sec;
				camera_header.nFrameRate = 10;
				camera_header.width = m_nWidth;
				camera_header.height = m_nHeight;
				camera_header.nSeq = m_uSeq;
				camera_header.size = nSize;
				memcpy(m_pFrameBuffer,&camera_header,sizeof(yuv_video_buf));
				AddFrame(1);
				m_uSeq++;
			}
			//printf("get frame: %d, Len: %d, type: %d\n", ++iFrames, pMs->iDataLen, pMs->eType);

			//释放资源
			m_StreamParser.ReleaseMediaSample(pMs);
		}
		else
		{
			usleep(10*1000);
		}
#endif

	}
	return nFlag;
}


//UDP 连接(单播方式)
bool CSkpRoadVideoYuv::connect_udp_unicast()
{
	if(m_strCameraIP.size() <= 8)
	{
		m_strCameraIP = "192.168.58.179";
	}

	LogNormal("ip=%s",m_strCameraIP.c_str());

	/* connect */
	struct sockaddr_in udp_addr;
	bzero(&udp_addr,sizeof(udp_addr));
	udp_addr.sin_family=AF_INET;
	udp_addr.sin_addr.s_addr= htonl(INADDR_ANY);
	udp_addr.sin_port=htons(9001);

	// socket
	if ((m_nYuvFd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		return false;

	}

	int nLen = 81920000; //接收缓冲区大小(8M)
	//设置接收缓冲区大小
	if (setsockopt(m_nYuvFd,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
	{
		shutdown(m_nYuvFd,2);
		close(m_nYuvFd);
		m_nYuvFd = -1;
		return false;
	}

	int on = 1;
	if(setsockopt(m_nYuvFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
	{
		shutdown(m_nYuvFd,2);
		close(m_nYuvFd);
		m_nYuvFd = -1;
		perror("setsockopt");
		return false;
	}

	if(bind(m_nYuvFd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) == -1)
	{
		shutdown(m_nYuvFd,2);
		close(m_nYuvFd);
		m_nYuvFd = -1;
		LogError("YUV视频流接收地址无法绑定!\r\n");
		return false;
	}
	printf("star receive date\n");
	return true;
}

//连接相机(组播方式)
bool CSkpRoadVideoYuv::Connect_udp()
{
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	{
		m_strCameraIP = m_strCameraMultiIP;
	}

	if(m_strCameraIP.size() <= 8)
	{
		m_strCameraIP = "224.168.0.1";
	}

	if(m_nCameraPort <= 10)
	{
		m_nCameraPort = 9001;
	}
	LogNormal("ip=%s,port=%d",m_strCameraIP.c_str(),m_nCameraPort);
    //YUV流打开
	 struct sockaddr_in addr;
     struct ip_mreq mreq;
	 int nLen = 8192000; //接收缓冲区大小(8M)

     /* create what looks like an ordinary UDP socket */
     if ((m_nYuvFd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
	 {
	   return false;
     }

     /* set up destination address */
     memset(&addr,0,sizeof(addr));
     addr.sin_family=AF_INET;
     addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
     addr.sin_port=htons(m_nCameraPort);

     int on = 1;
    if(setsockopt(m_nYuvFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
    {
        shutdown(m_nYuvFd,2);
        close(m_nYuvFd);
        m_nYuvFd = -1;
        perror("setsockopt");
        return false;
    }

    //设置接收缓冲区大小
	 if (setsockopt(m_nYuvFd,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
	 {
	     shutdown(m_nYuvFd,2);
        close(m_nYuvFd);
        m_nYuvFd = -1;
	   return false;
     }

    /* use setsockopt() to request that the kernel join a multicast group */
     mreq.imr_multiaddr.s_addr=inet_addr(m_strCameraIP.c_str());
     mreq.imr_interface.s_addr=htonl(INADDR_ANY);
     if (setsockopt(m_nYuvFd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
	 {
	     shutdown(m_nYuvFd,2);
         close(m_nYuvFd);
         m_nYuvFd = -1;
	     LogError("加入组播地址失败!\r\n");
	    return false;
     }

     /* bind to receive address */
     if (bind(m_nYuvFd,(struct sockaddr *)&addr,sizeof(addr)) < 0)
	 {
	     shutdown(m_nYuvFd,2);
         close(m_nYuvFd);
         m_nYuvFd = -1;
		 LogError("YUV视频流接收地址无法绑定!\r\n");
	     return false;
     }

     return true;
}

//打开相机
bool CSkpRoadVideoYuv::Open()
{
     m_cfg.uKind = m_nCameraType;

	 //采集视频流而不是读文件
	m_bReadFile = false;
	m_bCameraControl = false;
	m_nCountGain = 0;

	////////////////////////
	if(access("Unicast.cfg",F_OK) == 0)
	{
		m_bUnicast = true;
		g_bShortConnect = true;
	}

	 //以下3种相机一开始是不需要转换单位
	 if((m_nCameraType != BOCOM_301_200) && (m_nCameraType != BOCOM_302_500) && (m_nCameraType != BOCOM_301_500) && (m_nCameraType != BOCOM_302_200))
	 {
		 LoadCameraProfile(m_cfg);

		 if(m_cfg.uSM <= 0)
		{
			m_cfg.uSM = 1;
		}

		ConvertCameraParameter(m_cfg,false,true);
		m_bComRet = g_CameraSerialComm.OpenDev();

		if(m_bComRet)
		m_bControlOpen = true;
	 }
	 else
	 {
		 LoadCameraFloatProfile(m_cfg, m_nCamID);
		//进行极性转换
		if(m_cfg.uPol == 0)
		{
			m_cfg.uPol = 1;
		}
		else if (m_cfg.uPol == 1)
		{
			m_cfg.uPol = 0;
		}
		 if(m_cfg.uSM <= 0)
		{
			m_cfg.uSM = 1;
		}
		if(m_nCameraType == BOCOM_302_200)
		{
			m_cfg.EEN_width = m_cfg.EEN_width / 34.725;
			m_cfg.EEN_delay = m_cfg.EEN_delay / 34.725;
		}
		else if(m_nCameraType == BOCOM_301_200)
		{
			m_cfg.EEN_width = m_cfg.EEN_width / 32;
			m_cfg.EEN_delay = m_cfg.EEN_delay / 32;
		}
		else if(m_nCameraType == BOCOM_302_500)
		{
			m_cfg.EEN_width = m_cfg.EEN_width / 36.95;
			m_cfg.EEN_delay = m_cfg.EEN_delay / 36.95;
		}
		else if(m_nCameraType == BOCOM_301_500)
		{
			m_cfg.EEN_width = m_cfg.EEN_width / 32.067;
			m_cfg.EEN_delay = m_cfg.EEN_delay / 32.067;
		}
		  //以下三个参数不在LoadCameraProfile中可以加载
		string strCameraIP, strCameraMultiIP;
		strCameraIP = "";
		strCameraMultiIP = "";
		
		if (m_nPixelFormat == VEDIO_H264)
		{
			g_skpDB.GetCamIpByChannelID(strCameraIP, strCameraMultiIP,m_nChannelId);

			LogNormal("m_nChannelId=%d\n",m_nChannelId);
		}
		else
		{
			g_skpDB.GetCamIpByID(strCameraIP, strCameraMultiIP,m_nCamID);
		}
		
		LogNormal("strCameraIP=%s,m_nCamID=%d",strCameraIP.c_str(),m_nCamID);
		if (strCameraIP.size() <= 8)
		{
			strCameraIP = "192.168.58.179";//如果相机IP没有或者非法，默认赋值"192.168.58.179"
		}
		m_cfg.uCameraIP = ConvertStringToIP(strCameraIP);
		m_cfg.uCameraMultiIP = ConvertStringToIP(strCameraMultiIP);

		//m_nCamID = m_cfg.uCameraID;

		if(strCameraMultiIP != "")
		{
			m_strCameraMultiIP = strCameraMultiIP;
		}

		//m_strCameraIP = strCameraIP;
		
		//BOCOM_301_200等相机均采用网口控制
		m_bComRet = false;
		//打开串口
		if(g_CameraComSetting.nComPort == 0)
		{
			m_bComRet = false;
		}
		else
		{
			m_bComRet = g_CameraSerialComm.OpenDev();
		}

		if(!m_bComRet)
		{   
		  if ( m_nPixelFormat == VEDIO_H264)
		  {
			  UINT32 nTempIP = m_cfg.uCameraIP;//相机控制IP等于相机视频IP加一
			  m_strControlIP = ConvertIPToString(nTempIP);
			  LogNormal("strControlIP=%s",m_strControlIP.c_str());
			  //strControlIP = "192.168.58.1";
			  if(g_bShortConnect)
				  g_CameraSerialComm.SetControlIP(m_strControlIP);
			  else
			  {
				  if(g_CameraSerialComm.OpenDev(m_strControlIP, 3600))
				  {
					  m_bControlOpen = true;
				  }
				  else
				  {
					  m_bControlOpen = false;
				  }
			  }
		  }
		   else
		   {
				UINT32 nTempIP = m_cfg.uCameraIP + (1<<24);//相机控制IP等于相机视频IP加一
				m_strControlIP = ConvertIPToString(nTempIP);
				LogNormal("strControlIP=%s",m_strControlIP.c_str());
				//strControlIP = "192.168.58.1";
				if(g_bShortConnect)
				{
					g_CameraSerialComm.SetControlIP(m_strControlIP);
				}
				else
				{
					if(g_CameraSerialComm.OpenDev(m_strControlIP, 3600))
					  {
						  m_bControlOpen = true;
					  }
					  else
					  {
						  m_bControlOpen = false;
					  }
				}
		   }
		}
		
	 }

     if (m_nPixelFormat == VEDIO_H264)
     {
		 m_strData.clear();

		 if (!conect_set_tcp())
		 {
			 LogError("无法与相机建立控制TCP连接\r\n");
			// return false;
		 }

		 StartSendStream();

		 if (!conect_data_tcp())
		 {
			 LogError("无法与相机建立控制TCP连接\r\n");
		//	 return false;
		 }
     }

	 else
	 {
		 if(m_bUnicast)
		 {
			 if(!connect_udp_unicast())
			 {
				 LogError("连接相机失败!\r\n");
				 //return false;//为了相机自动重新连接此处不返回
			 }
		 }
		 else
		 {
			 //YUV流打开
			if(!Connect_udp())
			{
				LogError("连接相机失败!\r\n");
				//return false;//为了相机自动重新连接此处不返回
			}
		 }
	 }
    //相机参数设置
    {
        //m_bFirstAutoControlCamera = true;
        SetCaMeraPara();
    }

/*	if( m_nYuvFd > 0 )
	{
	    ////////////////////////
        //自适应分辨率
        if(m_nCameraType == JAI_CAMERA_LINK_FRAME)
        {
            GetFrameInfo();
        }

        LogNormal("GetFrameInfo ok");
        //printf("========OpenYuv ok\n");
	}*/

	Init();

	return true;
}

//YUV流关闭
bool CSkpRoadVideoYuv::Close()
{   

	if (m_nPixelFormat == VEDIO_H264)
	{
		m_bEndCapture = true;

		EndSendStream();

		close_set_tcp();

		close_data_tcp();

		if (m_nThreadId != 0)
		{
			//pthread_cancel(m_nThreadId);
			pthread_join(m_nThreadId, NULL);
			m_nThreadId = 0;
		}

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
	}
	else
	{
    int i;
    //线程结束标志
	m_bEndCapture = true;

    //关闭
    if(m_nYuvFd > 0)
    {
        shutdown(m_nYuvFd,2);
        close(m_nYuvFd);
        m_nYuvFd = -1;
    }
    printf("I'm shutdown ================\n");

	//停止YUV采集线程,等待线程结束
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
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

	//加锁
	pthread_mutex_lock(&m_FrameMutex);

	//释放内存
	if((m_nWidth!=0)&&(m_nHeight!=0))
	{
        for( i=0;i<m_nMaxBufferSize;i++)
        {
            if(m_FrameList[i]!=NULL)
            {
                free(m_FrameList[i]);
                m_FrameList[i] = NULL;
            }
        }
        m_nFrameSize = 0;
        m_pFrameBuffer = NULL;
	}


	//解锁
	pthread_mutex_unlock(&m_FrameMutex);

	}

    //关闭串口
	//if(g_nCameraControl==1)
	{
        g_CameraSerialComm.Close();
        //L_SerialComm.Close();
	}

	return true;
}


//相机控制
int CSkpRoadVideoYuv::ManualControl(CAMERA_CONFIG cfg)
{
	if(cfg.fValue < 0)//所输入的相机参数值不能小于0
	{
		return 0;
	}
	if((m_nCameraType == BOCOM_301_200) ||(m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500)||(m_nCameraType == BOCOM_302_200))
	{
		if (strcmp(cfg.chCmd,"pe")==0)
		{
			if(cfg.fValue > BOCOM_GIGE_MAX_PE)
				cfg.fValue = BOCOM_GIGE_MAX_PE;
		}
		else if (strcmp(cfg.chCmd,"ga")==0)
		{
			if(cfg.fValue > BOCOM_GIGE_MAX_GAIN)
				cfg.fValue = BOCOM_GIGE_MAX_GAIN;
		}
	}
    if(cfg.nIndex != (int)CAMERA_MODE)
    cfg.nMode = m_cfg.nMode;

    if(cfg.nIndex != (int)CAMERA_LIGHTTYPE)
    cfg.nLightType = m_cfg.nLightType;

    if(cfg.nIndex == (int)CAMERA_CMD)
    {
		if (strcmp(cfg.chCmd,"tr")==0)
        {
            cfg.nIndex = (int)CAMERA_MODE;
            cfg.nMode = (int)cfg.fValue;
        }
		else if (strcmp(cfg.chCmd,"LIS")==0||
			strcmp(cfg.chCmd,"lis")==0)
		{
			cfg.nIndex = (int)CAMERA_IRIS;
			cfg.nIris = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"asc")==0)
		{
			cfg.nIndex = (int)CAMERA_ASC;
			cfg.ASC = (int)cfg.fValue;
		}
		else if (strcmp(cfg.chCmd,"agc")==0)
		{
			cfg.nIndex = (int)CAMERA_AGC;
			cfg.AGC = (int)cfg.fValue;
		}
        else if (strcmp(cfg.chCmd,"pe")==0)
        {
            cfg.nIndex = (int)CAMERA_PE;
            if((m_nCameraType == BOCOM_301_200) ||(m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
			{
				cfg.fSH = (int)cfg.fValue;
			}
			else
			{				
				cfg.uPE = (int)cfg.fValue;
			}
        }
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            cfg.nIndex = (int)CAMERA_GAIN;
            if((m_nCameraType == BOCOM_301_200) ||(m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
			{
				cfg.fGain = (int)cfg.fValue;
			}
			else
			{				
				cfg.uGain = (int)cfg.fValue;
			}     
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
        else if (strcmp(cfg.chCmd,"pen")==0)
        {
            cfg.nIndex = (int)CAMERA_EEN;
            cfg.EEN_on = (int)cfg.fValue;
        }
		else if (strcmp(cfg.chCmd,"epl")==0)
        {
            cfg.nIndex = (int)CAMERA_POL;
            cfg.uPol = (int)cfg.fValue;
        }
    }
	else if(cfg.nIndex == (int)CAMERA_SET_IP)//设置相机视频IP，组播IP，控制IP
	{
		int nRet = 0;
		string strCamIP = ConvertIPToString(cfg.uCameraIP);//转化成字符串格式
		nRet = g_CameraSerialComm.SetIPAddress(strCamIP,m_nCamID);
		usleep(5000);
		if(nRet)//如果相机IP设置失败，后面的设置就不执行
		{
			g_CameraSerialComm.SaveCamPara(m_nCamID);
			usleep(5000);
			
			string CamMultiIP = ConvertIPToString(cfg.uCameraMultiIP);
			nRet = 0;
			nRet = g_CameraSerialComm.SetAddress(CamMultiIP,m_nCamID);
			if(nRet)//之有当2个IP都设置成功了才都保存到数据库
			{
				usleep(5000);
				g_CameraSerialComm.SaveCamPara(m_nCamID);
				g_skpDB.SaveCamIpByID(strCamIP, CamMultiIP, cfg.uCameraID, m_nCamID);
				m_cfg.uCameraIP = cfg.uCameraIP;
				m_cfg.uCameraMultiIP = cfg.uCameraMultiIP;
			}
			else//当只有相机IP设置成功，组播地址没有成功
			{
				string strCameraIP, strCameraMultiIP;
				strCameraIP = "";
				strCameraMultiIP = "";
				g_skpDB.GetCamIpByID(strCameraIP, strCameraMultiIP, m_nCamID);//读出原来的组播地址
				g_skpDB.SaveCamIpByID(strCamIP, strCameraMultiIP, cfg.uCameraID,m_nCamID);//存入新的相机IP和原先的组播地址
				m_cfg.uCameraIP = cfg.uCameraIP;
			}

			

		}
		m_nCamID = cfg.uCameraID;
	}

    if( (cfg.nIndex == (int)CAMERA_GAIN)
		||(cfg.nIndex == (int)CAMERA_POL)
        ||(cfg.nIndex == (int)CAMERA_PE)
        ||(cfg.nIndex == (int)CAMERA_PEI)
        ||(cfg.nIndex == (int)CAMERA_PEW)
        ||(cfg.nIndex == (int)CAMERA_EEN)
        ||(cfg.nIndex == (int)CAMERA_MAXGAIN)
        ||(cfg.nIndex == (int)CAMERA_MAXPE)
        ||(cfg.nIndex == (int)CAMERA_MAXSH))
    {
		if((m_nCameraType == BOCOM_301_200) ||(m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500)||(m_nCameraType == BOCOM_302_200))
		{
			if(cfg.nIndex == (int)CAMERA_PE)
			{
				if(cfg.fSH > BOCOM_GIGE_MAX_PE)
					cfg.fSH = BOCOM_GIGE_MAX_PE;
				else if(cfg.fSH < 0)
						return 0;
				
			}
			else if(cfg.nIndex == (int)CAMERA_GAIN)
			{
				if(cfg.fGain > BOCOM_GIGE_MAX_GAIN)
						cfg.fGain = BOCOM_GIGE_MAX_GAIN;
					else if(cfg.fGain < 0)
						return 0;
			}
		}
		ConvertCameraParameter(cfg,false);
		//ConvertCameraParameter(m_cfg,false);
		
        if(cfg.nIndex == (int)CAMERA_PE)
        {
			if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
			{
				cfg.fGain = m_cfg.fGain;
				WriteCameraFloatIni(cfg,m_nCamID);
			}
			else
			{
				cfg.uGain = m_cfg.uGain;
				WriteCameraIni(cfg);
			}
        }
        else if(cfg.nIndex == (int)CAMERA_GAIN)
        {
			if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
			{
				cfg.fSH = m_cfg.fSH;
				WriteCameraFloatIni(cfg,m_nCamID);
			}
			else
			{
				cfg.uPE = m_cfg.uPE;
				WriteCameraIni(cfg);
			}
        }
    }


    if(cfg.nIndex == (int)CAMERA_LIGHTTYPE)//设置灯类型(不需要告诉灯)
    {
        m_cfg.nLightType = cfg.nLightType;
        return 1;
    }
    else if(cfg.nIndex == (int)CAMERA_FREQUENCY) //设置触发频率
    {
       // if(cfg.nLightType==0)//主灯
      //  L_SerialComm.AdjustFrequency(cfg.nFrequency);

        m_cfg.nFrequency = cfg.nFrequency;
        return 1;
    }
    else if(cfg.nIndex == (int)CAMERA_PEW)
    {
        if(cfg.nMode ==1)//触发方式
        {
           // if(cfg.nLightType==0)//主灯
           // L_SerialComm.AdjustPuls(cfg.EEN_width);

            m_cfg.EEN_width = cfg.EEN_width;
            return 1;
        }
    }
    else if(cfg.nIndex == (int)CAMERA_EEN)
    {
        if(cfg.nMode ==1)//触发方式
        {
            if(cfg.EEN_on==0)//开灯
            {
           //     if(cfg.nLightType==0)//主灯
          //      L_SerialComm.SetHost(1);

                m_cfg.EEN_on = cfg.EEN_on;
                return 1;
            }
            else if(cfg.EEN_on==1)//关灯
            {
            //    if(cfg.nLightType==0)//主灯
            //    L_SerialComm.SetHost(0);

                m_cfg.EEN_on = cfg.EEN_on;
                return 1;
            }
        }
    }
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500)||(m_nCameraType == BOCOM_302_200))
	{
		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,0,m_nCamID);
	}
	else
	{
		g_CameraSerialComm.SendMessage(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex);
	}
}

//读取
int CSkpRoadVideoYuv::ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert)
{
	
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500)||(m_nCameraType == BOCOM_302_200))
	{
		int nValue = -1;
		cfg.nMode = m_cfg.nMode;
		//读取相机设置
		if(cfg.uType == 2)//单项
		{
			if (strcmp(cfg.chCmd,"asc")==0)
			{
				nValue = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_ASC,1,m_nCamID);
				//nValue = m_cfg.ASC;
			}
			else if (strcmp(cfg.chCmd,"LIS")==0||
				strcmp(cfg.chCmd,"lis")==0)
			{
				nValue = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_IRIS,1,m_nCamID);
			}
			else if (strcmp(cfg.chCmd,"agc")==0)
			{
				//nValue = m_cfg.AGC;
				nValue = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_AGC,1,m_nCamID);
			}
			/*else if (strcmp(cfg.chCmd,"sh")==0)
			{
				g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,1);
			}*/
			else if (strcmp(cfg.chCmd,"ga")==0)
			{
				cfg.fGain = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_GAIN,1,m_nCamID);
				m_cfg.fGain = cfg.fGain;
				if(bConvert)
				{
					ConvertCameraParameter(cfg,true);
				}
				//ConvertCameraParameter(m_cfg,true);
				nValue = cfg.fGain;
			}
			else if (strcmp(cfg.chCmd,"gama")==0)
			{
				//nValue = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,1);
			}
			else if (strcmp(cfg.chCmd,"bn")==0)//brightness
			{
				//nValue = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,1);
				g_CameraSerialComm.UpdateCam(1,1);
			}
			else if (strcmp(cfg.chCmd,"hue")==0)//hue
			{
				//g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,1);
			}
			else if (strcmp(cfg.chCmd,"sat")==0)//saturation
			{
				//g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,1);

				g_CameraSerialComm.GetVersion();
				printf("GetVersion out\n");
			}
			else if (strcmp(cfg.chCmd,"pe")==0)
			{
				cfg.fSH = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PE,1,m_nCamID);	
				m_cfg.fSH = cfg.fSH;
				if(bConvert)
				{
					ConvertCameraParameter(cfg,true);
				}
				nValue = cfg.fSH;
			}
			else if (strcmp(cfg.chCmd,"TR")==0||
				   strcmp(cfg.chCmd,"tr")==0)
			{
				cfg.nMode = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_MODE,1,m_nCamID);
				m_cfg.nMode = cfg.nMode;
				nValue = cfg.nMode;
			}
			else if (strcmp(cfg.chCmd,"epl")==0)
			{
				cfg.uPol = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_POL,1,m_nCamID);
				m_cfg.uPol = cfg.uPol;
				ConvertCameraParameter(cfg,true);
				nValue = cfg.uPol;
			}
			else if (strcmp(cfg.chCmd,"pen")==0 ||
					 strcmp(cfg.chCmd,"PEN")==0)
			{
				cfg.EEN_on = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,1,m_nCamID);
				m_cfg.EEN_on = cfg.EEN_on;
				if(bConvert)
				{
						ConvertCameraParameter(cfg,true);
				}
				nValue = cfg.EEN_on;
			}
			else if (strcmp(cfg.chCmd,"pei")==0)
			{
				cfg.EEN_delay = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PEI,1,m_nCamID);
				m_cfg.EEN_delay = cfg.EEN_delay;
				if(bConvert)
				{
						ConvertCameraParameter(cfg,true);
				}
				nValue = cfg.EEN_delay;
			}
			else if (strcmp(cfg.chCmd,"pew")==0)
			{
				cfg.EEN_width = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PEW,1,m_nCamID);
				m_cfg.EEN_width = cfg.EEN_width;
				if(bConvert)
				{
						ConvertCameraParameter(cfg,true);
				}
				nValue = cfg.EEN_width;
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
					//nValue = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_FREQUENCY,1);
				}
				else if(cfg.nMode == 1 )
				{
					nValue = m_cfg.nFrequency;
				}
			}
			else if (strcmp(cfg.chCmd,"iris")==0)
			{
				//nValue = cntl_get("get 824 \r\n");
				float fvalue = nValue&0xfff;
				nValue = (fvalue/0xfff)*100;
			}			
			else if (strncmp(cfg.chCmd,"0x",2)==0)
			{
				nValue = g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,1,m_nCamID);
				g_CameraSerialComm.SaveCamPara(m_nCamID);//永久保存
			}
			printf("before nValue\n");
			if(nValue != -1)
			{
				memset(cfg.chCmd,0,sizeof(cfg.chCmd));
				sprintf(cfg.chCmd,"=%d",nValue);
			}
			printf("after nValue\n");
		}
		else if(cfg.uType == 0)//多项
		{
		   cfg = m_cfg;
		   ConvertCameraParameter(cfg,true);
		}
	}
	else
	{
		//读取相机设置
		if(cfg.uType == 2)//单项
		{
		  CAMERA_CONFIG cfg_tmp = cfg;
		  g_CameraSerialComm.GetMessage(cfg);
		  std::string strCmd(cfg.chCmd);
		  printf("========ReadCameraSetting===strCmd = %s,strCmd.size()=%d\n",strCmd.c_str(),strCmd.size());
		  #ifdef CAMERA_CONTROL
		  FILE* fp = fopen("c.txt","a");
		   std::string strTime = GetTimeCurrent();
		   fprintf(fp,"time =%s,strCmd=%s\n",strTime.c_str(),strCmd.c_str());
		  fclose(fp);
		  #endif

		  if(strCmd.size() <= 0)
		  {
			  cfg.uSM = -1;
			  cfg.EEN_on = -1;
			  return 0;
		  }


		  int nPosEqual = strCmd.find("=");
		  if(nPosEqual== -1)
		   {
				strCmd.insert(0,"=",1);
				std::string strTmpCmd(cfg_tmp.chCmd);
				strCmd.insert(0,(char*)strTmpCmd.c_str(),strTmpCmd.size());
				nPosEqual = strCmd.find("=");
		   }
		   else if(nPosEqual == 0)
		   {
				std::string strTmpCmd(cfg_tmp.chCmd);
				strCmd.insert(0,(char*)strTmpCmd.c_str(),strTmpCmd.size());
				nPosEqual = strCmd.find("=");
		   }

		  int nPosR = strCmd.find("\r");
		  std::string strID = strCmd.substr(0,nPosEqual);
		  std::string strValue =strCmd.substr(nPosEqual+1,nPosR-nPosEqual-1);
		  int nValue = strtol(strValue.c_str(),NULL,10);

		  printf("nPosEqual=%d,strCmd = %s,strID=%s,strValue=%s\n",nPosEqual,strCmd.c_str(),strID.c_str(),strValue.c_str());

		  bool bConvertParam = false;
		  if (strcmp(strID.c_str(),"PEN")==0||
			  strcmp(strID.c_str(),"pen")==0)
		  {
				cfg.EEN_on = nValue;
				if(bConvert)
				{
					ConvertCameraParameter(cfg,true);
					nValue = cfg.EEN_on;
					bConvertParam = true;
				}
		  }
		  else if (strcmp(strID.c_str(),"GA")==0||
				   strcmp(strID.c_str(),"ga")==0)
		  {
				cfg.uGain = nValue;
				if(bConvert)
				{
					ConvertCameraParameter(cfg,true);
					nValue = cfg.uGain;
					bConvertParam = true;
				}
		  }
		  else if (strcmp(strID.c_str(),"PE")==0||
				   strcmp(strID.c_str(),"pe")==0)
		  {
				cfg.uPE = nValue;
				if(bConvert)
				{
					ConvertCameraParameter(cfg,true);
					nValue = cfg.uPE;
					bConvertParam = true;
				}
		  }
		  else if (strcmp(strID.c_str(),"PEI")==0||
				   strcmp(strID.c_str(),"pei")==0)
		  {
				cfg.EEN_delay = nValue;
				if(bConvert)
				{
					ConvertCameraParameter(cfg,true);
					nValue = cfg.EEN_delay;
					bConvertParam = true;
				}
		  }
		  else if (strcmp(strID.c_str(),"PEW")==0||
				   strcmp(strID.c_str(),"pew")==0)
		  {
				cfg.EEN_width = nValue;
				if(bConvert)
				{
					ConvertCameraParameter(cfg,true);
					nValue = cfg.EEN_width;
					bConvertParam = true;
				}
		  }
		  else if (strcmp(strID.c_str(),"ASC")==0||
				   strcmp(strID.c_str(),"asc")==0)
		  {
				cfg.ASC = nValue;
		  }
		  else if (strcmp(strID.c_str(),"AGC")==0||
				   strcmp(strID.c_str(),"agc")==0)
		  {
				cfg.AGC = nValue;
		  }
		  else if (strcmp(strID.c_str(),"SM")==0||
				   strcmp(strID.c_str(),"sm")==0)
		  {
				cfg.uSM = nValue;
		  }
		  else
		  {
				bConvertParam = true;
		  }

		  if(bConvertParam&&bConvert)
		  {
			memset(cfg.chCmd,0,sizeof(cfg.chCmd));
			sprintf(cfg.chCmd,"%s=%d\r\n",strID.c_str(),nValue);
		  }
		}
		else if(cfg.uType == 0)//多项
		{
		   cfg = m_cfg;
		   ConvertCameraParameter(cfg,true);
		}
	}
    return 1;
}

//自动控制
int  CSkpRoadVideoYuv::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
{
	if(!m_bControlOpen &&(!m_bComRet))
	{
		if(g_CameraSerialComm.OpenDev(m_strControlIP, 3600))
		{
			m_bControlOpen = true;
		}
		else
		{
			m_bControlOpen = false;
			return 0;
	    }
	}

	if(m_bComRet && (m_nPixelFormat == VEDIO_H264))
	{
		return 0;
	}
	
	float fZero = -0.00001;
    CAMERA_CONFIG cfg = m_cfg;//获取当前相机配置
	
    //重新设置快门模式
	if((m_nCameraType != BOCOM_301_200) && (m_nCameraType != BOCOM_302_500) && (m_nCameraType != BOCOM_301_500)&&(m_nCameraType != BOCOM_302_200))
    {
		if(uTimeStamp >= m_uShutterModeTime + 300)//每5分钟判断一次
		{
			int nSM = GetShutterMode();

			//LogNormal("nSM=%d",nSM);
			if(nSM != 1)
			{
				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_ASC);
				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_AGC);
				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_SM);
				usleep(1000*5);

				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_POL);
				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_PEI);
				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_PEW);
				usleep(1000*5);

				LogNormal("重新设置快门模式");
			}
        }

        m_uShutterModeTime = uTimeStamp;
    }

	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500)||(m_nCameraType == BOCOM_302_200))
	{
		//LogNormal("1 before convert uGain=%.2f,uSH=%.2f\n",cfg.fGain,cfg.fSH);
		ConvertCameraParameter(cfg,true);//在变幻之前先转变为客户端的值
		//LogNormal("1 after convert uGain=%.2f,uSH=%.2f\n",cfg.fGain,cfg.fSH);
	}
	else
	{
		fIncrement *= 32;
	}

    int nMaxGain = m_cfg.nMaxGain;
    int nMaxPE = m_cfg.nMaxPE;
    int nMinGain = 0;
    int nMinPE = 5;
    if(!bDetectCarnum)//非机动车道
    {
        nMaxGain = m_cfg.nMaxGain2;
        nMaxPE = m_cfg.nMaxPE2;
        printf("AutoControl======m_cfg.nMaxGain2=%d,m_cfg.nMaxPE2=%d=======\r\n",m_cfg.nMaxGain2,m_cfg.nMaxPE2);
    }
	
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	{
		//LogNormal("AutoControl fRate=%.2f,fIncrement=%.2f,nMaxPE=%d,nMaxGain=%d\n",fRate,fIncrement,nMaxPE,nMaxGain);

		if(((cfg.fSH-nMaxPE) > fZero) && (fRate > 1)) //PE不能再大
		{
			fRate = 1;
			cfg.fSH = nMaxPE;
		}
		else if( ((nMinPE - cfg.fSH) > fZero)&& (fRate < 1))  //PE不能再小
		{
			fRate = 1;
			cfg.fSH = nMinPE;
		}
		if(((cfg.fGain-nMaxGain) > fZero) && (fIncrement > 0)) //GAIN不能再大
		{
			fIncrement = 0;
			cfg.fGain = nMaxGain;
		}
		else if( ((nMinGain - cfg.fGain) > fZero) && (fIncrement < 0)) //GAIN不能再小
		{
			fIncrement = 0;
			cfg.fGain = nMinGain;
		}
		if(fRate>1)
		cfg.fSH = cfg.fSH*fRate+0.5;
		else if(fRate<1)
		cfg.fSH = cfg.fSH*fRate;

		if( fIncrement >0  || fIncrement < 0)
		cfg.fGain += fIncrement;

		{
			if(cfg.fGain>=nMaxGain)
					cfg.fGain =nMaxGain;
			else if(cfg.fGain<=nMinGain)
					cfg.fGain = nMinGain;

			if(cfg.fSH>=nMaxPE)
					cfg.fSH = nMaxPE;
			else if(cfg.fSH<=nMinPE)
					cfg.fSH = nMinPE;
		}
		ConvertCameraParameter(cfg,false);//在变换之后再转变为相机端的值
		//LogNormal("2 after convert uGain=%.2f,uSH=%.2f\n",cfg.fGain,cfg.fSH);
	}
	else
	{
		if( (cfg.uPE == nMaxPE)&& (fRate > 1)) //PE不能再大
		{
			fRate = 1;
		}
		if( (cfg.uGain == nMaxGain)&& (fIncrement > 0)) //GAIN不能再大
		{
			fIncrement = 0;
		}


		if( (cfg.uPE == nMinPE)&& (fRate < 1))  //PE不能再小
		{
			fRate = 1;
		}
		if( (cfg.uGain == nMinGain)&& (fIncrement < 0)) //GAIN不能再小
		{
			fIncrement = 0;
		}

		if(fRate>1)
		cfg.uPE = cfg.uPE*fRate+0.5;
		else if(fRate<1)
		cfg.uPE = cfg.uPE*fRate;

		if( fIncrement >0  || fIncrement < 0)
		cfg.uGain += fIncrement;

		{
			if(cfg.uGain>=nMaxGain)
					cfg.uGain =nMaxGain;
			else if(cfg.uGain<=nMinGain)
					cfg.uGain = nMinGain;

			if(cfg.uPE>=nMaxPE)
					cfg.uPE = nMaxPE;
			else if(cfg.uPE<=nMinPE)
					cfg.uPE = nMinPE;
		}
	}

	if(cfg.ASC == 0)
    {
        if( fRate > 1 || fRate < 1)
        {
			if((m_nCameraType == BOCOM_301_200) ||(m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
			{	
				g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PE,0,m_nCamID);
			}
			else
			{
				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_PE);
			}
        }
    }

    if(cfg.AGC == 0)
    {
        if( fIncrement >0  || fIncrement < 0)
        {
			if((m_nCameraType == BOCOM_301_200) ||(m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
			{
				g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_GAIN,0,m_nCamID);
			}
			else
			{
				g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_GAIN);
			}
        }
    }

    /*if(nIris != m_cfg.nIris)//光圈控制
    {
        cfg.nIris = nIris;
        g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_IRIS);
    }*/

    if(fRate > 1 || fRate < 1 ||
       fIncrement >0  || fIncrement < 0)
    {
       m_bCameraControl = true;
       //写配置文件
	    if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	   {
			WriteCameraFloatIni(cfg,m_nCamID);
			LogNormal("AutoControl uGain=%.2f,uSH=%.2f,nEn=%d\n",cfg.fGain,cfg.fSH,nEn);
	   }
	   else
	   {
		    WriteCameraIni(cfg);
	   }
    }

    #ifdef CAMERA_CONTROL
    FILE* fp = fopen("c.txt","a");
    std::string strTime = GetTime(uTimeStamp);
    fprintf(fp,"time =%s,fRate=%.2f,fIncrement=%.2f,uGain=%d,uPE=%d,nEn=%d\n",strTime.c_str(),fRate,fIncrement,cfg.uGain,cfg.uPE,nEn);
    fclose(fp);
    #endif

    //相机开关灯
    cfg.EEN_on = LightControl();

   if(cfg.EEN_on > -1)
   {
        m_cfg.EEN_on = cfg.EEN_on;

        if(g_nHasHighLight == 0)
        {
            if(g_LightTimeInfo.nLightTimeControl == 0)
            {
                //如果增益和曝光时间很大仍然未开灯则开灯
                if( nEn == 1)
                {
                    if( (cfg.EEN_on != 0))//根据亮度去开灯
                    {
                        LogNormal("OpenLight,uGain=%d,nEn=%d\n",cfg.uGain,nEn);
                        cfg.EEN_on = 0;//打开
                        {
							if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
							{
								g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,0,m_nCamID);
							}
							else
							{
								g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_EEN);
							
								sprintf(cfg.chCmd,"fg");
								cfg.fValue = 1;
								g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);

								sprintf(cfg.chCmd,"th2");
								cfg.fValue = 128;
								g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
							}
                        }
                    }
                }
                else if( nEn == 0)//关灯
                {
                        if( (cfg.EEN_on != 1))//根据亮度去关灯
                        {
                                LogNormal("CloseLight,uGain=%d,m_nCountGain=%d,nEn=%d\n",cfg.uGain,m_nCountGain,nEn);
                                cfg.EEN_on = 1;//关闭
                                {
									if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
									{
										g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,0,m_nCamID);
									}
									else
									{
										g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_EEN);
									
										sprintf(cfg.chCmd,"fg");
										cfg.fValue = 0;
										g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);

										sprintf(cfg.chCmd,"th2");
										cfg.fValue = 0;
										g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
									}
                                }
                        }
                }
            }
            //到达一定时间不管自动判断如何都开关灯
            else if(g_LightTimeInfo.nLightTimeControl == 1)
            {
                int nDayNight = DayOrNight(1);
                if(nDayNight == 0)
                {
                    if(cfg.EEN_on != 0)
                    {
                        LogNormal("强制开灯,uGain=%d,nEn=%d\n",cfg.uGain,nEn);
                        cfg.EEN_on = 0;//打开
						if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
						{
							g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,0,m_nCamID);
						}
						else
						{
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_EEN);
					
							sprintf(cfg.chCmd,"fg");
							cfg.fValue = 1;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);

							sprintf(cfg.chCmd,"th2");
							cfg.fValue = 128;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
						}
                    }
                }
                else if(nDayNight == 1)
                {
                    if(cfg.EEN_on != 1)
                    {
                        LogNormal("强制关灯,uGain=%d,nEn=%d\n",cfg.uGain,nEn);

                        cfg.EEN_on = 1;//关灯
						if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
						{
							g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,0,m_nCamID);
						}
						else
						{
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_EEN);
						
							sprintf(cfg.chCmd,"fg");
							cfg.fValue = 0;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);

							sprintf(cfg.chCmd,"th2");
							cfg.fValue = 0;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
						}
                    }
                }
            }
        }
        else if(g_nHasHighLight == 1)
        {
            if(cfg.EEN_on != 0)
            {
                LogNormal("强制开灯,uGain=%d,nEn=%d\n",cfg.uGain,nEn);
                cfg.EEN_on = 0;//打开
				if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
				{
					g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,0,m_nCamID);
				}
				else
				{
					g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_EEN);
				}
            }
        }
   }
    return 1;
}

//获取快门模式
int CSkpRoadVideoYuv::GetShutterMode()
{
    CAMERA_CONFIG cfg = m_cfg;

    //判断是否开灯
    cfg.uType = 2;
    memset(cfg.chCmd,0,sizeof(cfg.chCmd));
    memcpy(cfg.chCmd,"SM",2);
    ReadCameraSetting(cfg,false);

    return cfg.uSM;
}

//灯控制
//返回开关灯状态
int  CSkpRoadVideoYuv::LightControl(bool bInit)
{
		///////////////////////////////闪光灯控制(需要根据月份调整开灯时间)
		CAMERA_CONFIG cfg = m_cfg;

        if(!bInit)
        {
            //判断是否开灯
            cfg.uType = 2;
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            memcpy(cfg.chCmd,"PEN",3);
            ReadCameraSetting(cfg,false);

            #ifdef CAMERA_CONTROL
            FILE* fp = fopen("c.txt","a");
            std::string strTime = GetTimeCurrent();
            fprintf(fp,"time =%s,EEN_on=%d\n",strTime.c_str(),cfg.EEN_on);
            fclose(fp);
            #endif
        }
        else
        {
            int nDayNight = DayOrNight(1);
            if(nDayNight==0)
            {

                {
                    LogNormal("OpenLight\n");
                    cfg.EEN_on = 0;//打开
                    {
                        
						if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
						{
							g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,0,m_nCamID);
						}
						else
						{
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_EEN);
							
						   // usleep(1000*10);

							sprintf(cfg.chCmd,"fg");
							cfg.fValue = 1;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
						  //  usleep(1000*10);

							sprintf(cfg.chCmd,"th2");
							cfg.fValue = 128;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
						   // usleep(1000*10);
						}
                    }
                }
            }
            else if(nDayNight==1)
            {
                {
                    LogNormal("CloseLight\n");
                    {
                        if(g_nHasHighLight == 0)
						{						
							cfg.EEN_on = 1;//关闭
						}

                        else if(g_nHasHighLight == 1)
						{							
							cfg.EEN_on = 0;//开灯
						}
						if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
						{
							g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_EEN,0,m_nCamID);
						}
						else
						{
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_EEN);
						
							
						   // usleep(1000*10);

							sprintf(cfg.chCmd,"fg");
							cfg.fValue = 0;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
						   // usleep(1000*10);

							sprintf(cfg.chCmd,"th2");
							cfg.fValue = 0;
							g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
						}
                    }
                   
                }
            }
        }
		return cfg.EEN_on;
}

//相机参数设置
void CSkpRoadVideoYuv::SetCaMeraPara()
{
	printf("===m_bComRet=%d,m_bControlOpen=%d\n",m_bComRet,m_bControlOpen);
	/*if( (g_ytControlSetting.nNeedControl == 0)&& (g_ytControlSetting.nSerialPort == 0) )
	{
		return ;
	}*/

	if(!m_bControlOpen &&(!m_bComRet))
	{
		return;
	}

	if(m_bComRet && (m_nPixelFormat == VEDIO_H264))
	{
		return;
	}
    //相机控制初始化
    //printf("===g_nCameraControl=%d,m_nCameraType=%d\n",g_nCameraControl,m_nCameraType);
	CAMERA_CONFIG cfg = m_cfg;

	//初始
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	{
		printf("before cfg.nIndex,2\n");
		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,2,m_nCamID);
		printf("after cfg.nIndex,2\n");
		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,3,m_nCamID);

		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_ASC,0,m_nCamID);
		usleep(1000*20);
		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_AGC,0,m_nCamID);
		usleep(1000*20);
		/*g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_SM,m_nCamID);
		usleep(1000*20);*/
		//初始化光圈
		cfg.nIris = 0;//0表示关闭
		//g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_IRIS,m_nCamID);
	}
	else
	{
		//初始
		g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_ASC);
		usleep(1000*20);
		g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_AGC);
		usleep(1000*20);
		g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_SM);
		usleep(1000*20);
	}


    //初始增益和曝光时间
    if(g_uTime - g_uLastTime>=1200)
    {
            printf("InitialGainAndPE,g_uTime=%lld,g_uLastTime=%lld\n",g_uTime,g_uLastTime);
            InitialGainAndPE();
    }
    else
    {
		if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
		{
			//ConvertCameraParameter(cfg,false);
			if(cfg.ASC == 0)
			g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PE,0,m_nCamID);
			if(cfg.AGC == 0)
			g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_GAIN,0,m_nCamID);
		}
		else
		{
			if(cfg.ASC == 0)
			g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_PE);
			if(cfg.AGC == 0)
			g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_GAIN);
		}
        //usleep(1000*10);
    }

    //灯控制相关
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	{
		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_POL,0,m_nCamID);
		//usleep(1000*10);
		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PEI,0,m_nCamID);
		//usleep(1000*10);
		g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PEW,0,m_nCamID);
	}
	else
	{
		g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_POL);
		//usleep(1000*10);
		g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_PEI);
		//usleep(1000*10);
		g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_PEW);
		//usleep(1000*10);
		cfg.nIris = 0;//
		//保存当前参数
		g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_SA);
	}
    //usleep(1000*10);
    //开关灯控制
	LightControl(true);	
}

//根据时间设置不同的增益、曝光时间初始值
void CSkpRoadVideoYuv::InitialGainAndPE()
{

	//获取当前时间
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r( &now,newTime);

	unsigned int uTime =  newTime->tm_hour*100+newTime->tm_min;

	CAMERA_CONFIG cfg;

	//初始增益和曝光时间
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	{
		if((uTime>=0)&&(uTime<=500))//0->5//后半夜
		{
			cfg.fGain =400;//固定增益
			cfg.fSH = 50;
		}
		else if((uTime>=500)&&(uTime<=830))//5->8.30//早晨
		{
			cfg.fGain =200;//固定增益
			cfg.fSH = 25;
		}
		else if((uTime>=830)&&(uTime<=1030))//8.30->10.30//上午
		{
			cfg.fGain = 100;
			cfg.fSH = 10;
		}
		else if((uTime>=1030)&&(uTime<=1430))//10.30->14.30//中午
		{
			cfg.fGain = 50;
			cfg.fSH = 5;
		}
		else if((uTime>=1430)&&(uTime<=1630))//14.30->16.30//下午
		{
			cfg.fGain = 100;
			cfg.fSH = 10;
		}
		else if((uTime>=1630)&&(uTime<=1800))//16.30->18//黄昏
		{
			cfg.fGain = 200;
			cfg.fSH = 15;
		}
		else if((uTime>=1800)&&(uTime<=2400))//18->24//前半夜
		{
			cfg.fGain =400;//固定增益
			cfg.fSH = 50;
		}
		cfg.fSH = cfg.fSH * 33;
		cfg.fGain = cfg.fGain  * 24 / 768;
		ConvertCameraParameter(cfg,false);
		if(m_cfg.ASC == 0)
		{
			//ConvertCameraParameter(cfg,false);
			g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PE,0,m_nCamID);
		}
		//usleep(1000*10);
		if(m_cfg.AGC == 0)
		{
			g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_GAIN,0,m_nCamID);
		}
	}
	else
	{
		if((uTime>=0)&&(uTime<=500))//0->5//后半夜
		{
			cfg.uGain =400;//固定增益
			cfg.uPE = 50;
		}
		else if((uTime>=500)&&(uTime<=830))//5->8.30//早晨
		{
			cfg.uGain =200;//固定增益
			cfg.uPE = 25;
		}
		else if((uTime>=830)&&(uTime<=1030))//8.30->10.30//上午
		{
			cfg.uGain = 100;
			cfg.uPE = 10;
		}
		else if((uTime>=1030)&&(uTime<=1430))//10.30->14.30//中午
		{
			cfg.uGain = 50;
			cfg.uPE = 5;
		}
		else if((uTime>=1430)&&(uTime<=1630))//14.30->16.30//下午
		{
			cfg.uGain = 100;
			cfg.uPE = 10;
		}
		else if((uTime>=1630)&&(uTime<=1800))//16.30->18//黄昏
		{
			cfg.uGain = 200;
			cfg.uPE = 15;
		}
		else if((uTime>=1800)&&(uTime<=2400))//18->24//前半夜
		{
			cfg.uGain =400;//固定增益
			cfg.uPE = 50;
		}

		if(m_cfg.ASC == 0)
		{
			g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_PE);
		}
		//usleep(1000*10);
		if(m_cfg.AGC == 0)
		{
			g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_GAIN);
		}
	}
}

/*******************采集jpg*******************/
void CSkpRoadVideoYuv::DealJpgFrame()
{
     GetNextJpgframe();
}

//采集一帧jpg视频数据
int CSkpRoadVideoYuv::GetNextJpgframe()
{
	return 1;
}

//根据转换盒参数自适应分辨率
void CSkpRoadVideoYuv::GetFrameInfo()
{
    int nValue;

    if ((nValue = fcntl(m_nYuvFd, F_GETFL, 0)) < 0)
    {
        return ;
    }
    nValue |= O_NONBLOCK;   // set non-block.
    if (fcntl(m_nYuvFd, F_SETFL, nValue) < 0)
    {
        return ;
    }

    int nCount = 0;
     //接收参数socket
    struct sockaddr_in recv_addr;
	socklen_t addrlen = sizeof(recv_addr);
    int bytes = 0;

    yuv_video_header* header = NULL;

    char buffer[MAX_LEN] = {0};

    while(!m_bEndCapture)
    {
        bytes = 0;

        bytes = recvfrom(m_nYuvFd, buffer, MAX_LEN, MSG_NOSIGNAL,(struct sockaddr *) &recv_addr, &addrlen);


        //printf("====GetFrameInfo=======errno=%d,strerror(errno)=%s,bytes=%d\n",errno,strerror(errno),bytes);

        if((errno == 11)&&(bytes == -1))
        {
            LogNormal("====GetFrameInfo=======errno=%d,strerror(errno)=%s,bytes=%d\n",errno,strerror(errno),bytes);
       //     break;
        }

        if(bytes > 0)
        {
            if(bytes >= sizeof(yuv_video_header))
            {
               // LogNormal("====GetFrameInfo===bytes=%d\n",bytes);

                if( (memcmp(buffer,"$$$$",4)==0) ||
                        (memcmp(buffer,">>>>",4)==0))
                {
                    header = (yuv_video_header*)buffer;

                    if(header)
                    {
                        LogNormal("===m_nWidth = %d,m_nHeight = %d,nWidth=%d,nHeight=%d\r\n",m_nWidth,m_nHeight,header->nWidth,header->nHeight);

                        if(header->nWidth != m_nWidth)
                        {
                            if(header->nWidth ==1600)
                            {
                                m_nWidth = header->nWidth;
                                m_nMarginX = 0;
                            }
                        }

                        if(header->nHeight != m_nHeight)
                        {
                            if(header->nHeight ==1200)
                            {
                                m_nHeight = header->nHeight;
                                m_nMarginY = 0;
                            }
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            nCount++;
            usleep(500*1000);
        }

        if(nCount >= 3)
        {
            LogNormal("====GetFrameInfo===bytes=%d,nCount=%d\n",bytes,nCount);
            break;
        }
    }

    if ((nValue = fcntl(m_nYuvFd, F_GETFL, 0)) < 0)
    {
        return ;
    }
    nValue &= ~O_NONBLOCK;  // set block.
    if (fcntl(m_nYuvFd, F_SETFL, nValue) < 0)
    {
        return ;
    }
}

/*
*
* 接收yuv图像数据(3块yuv图像)
*/
int CSkpRoadVideoYuv::GetNextframe( )
{
	//std::ofstream B("FRAMERATE.txt",std::ios::app);
	
	//printf("GetNextframe:::::::::::::::::::::::::::::::::\n");
    //接收标志
    int nFlag = -1;

    //每次接收的字节
    int bytes = 0;
    //接收数据位置
	int index = 0;
	//剩余接收的字节
	int left = 0;
	//yuv大小
	int nSize = 0;
	int nWidth = 0;
	int nHeight = 0;

    //接收参数socket
    struct sockaddr_in recv_addr;
	socklen_t addrlen = sizeof(recv_addr);

	/////////单播
	if(m_bUnicast)
	{
		recv_addr.sin_family = AF_INET;
		recv_addr.sin_port = htons(9001);
		recv_addr.sin_addr.s_addr = inet_addr(m_strCameraIP.c_str());
	}
	/////////



	char buf[MAX_LEN]={'\0'};

	//yuv头
	yuv_video_header* header = NULL;
	unsigned char nBlockSeq = 0;
	unsigned int nFieldSeq = 0;
	unsigned short nFrameRate = 0;
	int nHeaderSize = sizeof(yuv_video_header);

	//yuv数据缓冲区
    yuv_video_buf* buffer=NULL;

    //采集时间(以微妙为单位)
    struct timeval tv;
    int64_t nSystemTime;
    unsigned int uTimeStamp;


	//接收yuv
     while(!m_bEndCapture)
	{
	    //如果接收的字节等于头的大小，添加在数据区后
       if(bytes==nHeaderSize)
		{
		    //判断接收的数据是那块yuv数据
		    {
		        nBlockSeq = 1;
		        memcpy(buf,m_pFrameBuffer+sizeof(yuv_video_buf)+index,bytes);
		    }

            index= 0;
            bytes = 0;
		}
		//如果接收的字节不是头，接收头
		else
		{
		    //printf("========before recvfrom======m_nYuvFd=%d,m_strHost=%s,m_nPort=%d\r\n",m_nYuvFd,m_strHost.c_str(),m_nPort);
		    index = 0;
		    bytes = 0;
			//接收码流头
			bytes = recvfrom(m_nYuvFd, &buf[index], MAX_LEN, MSG_NOSIGNAL,(struct sockaddr *) &recv_addr, &addrlen);

            //重新接收yuv头
			if(bytes <= 0)
			{
			    LogError("接收YUV头错误,bytes=%d,strerror(errno)=%s",bytes,strerror(errno));
			    break;
			}
		}
        //printf("==========bytes=======%d\r\n",bytes);
        //同步判断
        bool bSynchron =  false;
        if( (memcmp(buf,"$$$$",4)==0) ||
            (memcmp(buf,">>>>",4)==0) ||
			(memcmp(buf,"****",4)==0))
        {
            bSynchron = true;
            gettimeofday(&tv,NULL);
			uTimeStamp = tv.tv_sec;
        }
      //  if(bytes==nHeaderSize)
      // printf("========receive bytes=%d,%c%c%c%c,%c%c%c%c,bSynchron=%d\r\n",bytes,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],bSynchron);

        if(bSynchron)
		{
		    header = (yuv_video_header*)buf;

			if(((memcmp(header->cType,"UYVY",4)!=0)&&(memcmp(header->cType,"VYUY",4)!=0)&&(memcmp(header->cType,"YVYU",4)!=0)&&(memcmp(header->cType,"YUYV",4)!=0))
				&&((memcmp(header->cType,"RGGB",4)!=0)&&(memcmp(header->cType,"GBRG",4)!=0)&&(memcmp(header->cType,"GRBG",4)!=0)&&(memcmp(header->cType,"BGGR",4)!=0)))
			{
				LogError("YUV格式错误");
                continue;
			}
             //判断头后，根据yuv的类型，确定图像大小
             {
                 nWidth =  m_nWidth;
                 nHeight = m_nHeight;

                 if(m_nCameraType==JAI_CAMERA_LINK_FRAME)
                 {
                    if(header->nWidth != m_nWidth)
                    {
                        if(header->nWidth ==1600)
                        {
                            nWidth = header->nWidth;
                            m_nWidth = nWidth;
                            m_nMarginX = 0;
                        }
                    }

                    if(header->nHeight != m_nHeight)
                    {
                        if(header->nHeight ==1200)
                        {
                            nHeight = header->nHeight;
                            m_nHeight = nHeight;
                            m_nMarginY = 0;
                        }
                    }
                 }
             }
			 
			 if(m_nPixelFormat == VIDEO_BAYER)
			 {
				 nSize = nWidth * nHeight;
			 }
             else
			 {
				 nSize = nWidth * nHeight*2;
			 }
             //printf("header->nWidth=%d,header->nHeight=%d,header->nSize=%d,header->nSeq=%x,nSize=%d\r\n",header->nWidth,header->nHeight,header->nSize,header->nSeq,nSize);
			
			 //**********************************测试使用
			 if(m_bUseTestResult)
			 {
				 if(g_nFileID != header->cReserved[0])
				 {
					g_nFileID = header->cReserved[0];//文件ID
					LogNormal("g_nFileID=%d,%c%c%c%c-%c%c%c%c",g_nFileID,header->cSynchron[0],header->cSynchron[1],header->cSynchron[2],header->cSynchron[3],header->cType[0],header->cType[1],header->cType[2],header->cType[3]);
					m_uSeq  = 0;
					m_nPreFieldSeq = 0;
				 }
			 }
			 //********************************************

		   //初始化接收数据参数
		   bytes = 0;
		   index = 0;
		   left = nSize;
		   //根据类型接收数据
		   while(left > 0)
		   {
		       {
		            bytes = recvfrom(m_nYuvFd, m_pFrameBuffer+sizeof(yuv_video_buf)+index, left > MAX_LEN ? MAX_LEN : left, MSG_NOSIGNAL,(struct sockaddr *) &recv_addr, &addrlen);
		       }
				//如果再次接收头就跳回循环
		    	if(bytes==nHeaderSize || bytes <= 0)
			   {
                  break;
			   }
               //修改接收参数
				left -= bytes;
				index +=bytes;
			}
            //接收错误，重新接收
			if(bytes==nHeaderSize)
			{
			    LogError("接收数据时接收到yuv头");
				continue;
			}

			if(bytes <=  0)
			{
			    LogError("接收YUV数据错误,bytes=%d,strerror(errno)=%s",bytes,strerror(errno));
                break;
			}
			{
			    nFrameRate =  (header->nSeq) & 0xffff;
                nFieldSeq =   (header->nSeq)>>16;
                nBlockSeq = 1;

                if(m_nPreFieldSeq>nFieldSeq)
                {
                    LogNormal("m_nPreFieldSeq= %lld,nFieldSeq=%lld\n",m_nPreFieldSeq,nFieldSeq);
                    if(nFieldSeq > 10)
                    {
                        m_uSeq  =  nFieldSeq;
                    }
                    else
                    {
                        m_uSeq  = m_uSeq + nFieldSeq+1;
                    }
                }
                else
                {
					if(m_uSeq == 0)
					{
						LogNormal("first nFieldSeq=%lld\n",nFieldSeq);
					}

                    m_uSeq  = m_uSeq + nFieldSeq - m_nPreFieldSeq;
                }
                 //yuv自定义头
                buffer = (yuv_video_buf*)m_pFrameBuffer;
                //保存数据
                buffer->data = m_pFrameBuffer+sizeof(yuv_video_buf);
                //记住上一场场号
                m_nPreFieldSeq = nFieldSeq ;
                 //当前帧号
                buffer->nSeq = m_uSeq;

			    if((m_nCameraType==JAI_CAMERA_LINK_FIELD) || (m_nCameraType == BOCOM_302_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500)) //场图像
                {
                    buffer->uFrameType = 1;
                    buffer->nFieldSeq = nFieldSeq;
                    //buffer->nOE = ((header->nSize)>>24)&0xff;
                    //test the nOE value
                    //printf("---------------------buffer->nOE = %d\n",buffer->nOE );
                    //test over
                }
                else if( (m_nCameraType==JAI_CAMERA_LINK_FRAME)||
                         (m_nCameraType==JAI_CAMERA_LINK_FIELD_P)||
						 (m_nCameraType==JAI_CL_500)||
						 (m_nCameraType == BOCOM_301_200))//帧图像
                {
                    buffer->uFrameType = 2;
                    buffer->nFieldSeq = nFieldSeq;
                }

                if(m_nCameraType==JAI_CAMERA_LINK_FIELD)
                {
                    buffer->nFrameRate = 25;
                }
                else if( (m_nCameraType==JAI_CAMERA_LINK_FIELD_P) || (m_nCameraType==BOCOM_302_200) )//贵阳帧率是25
                {
                    buffer->nFrameRate = 12;
                }
				else if( (m_nCameraType==BOCOM_302_500) || (m_nCameraType==BOCOM_301_500) )
				{
					buffer->nFrameRate = 7;
				}
				else if(m_nCameraType==JAI_CL_500)
                {
                    buffer->nFrameRate = 10;
                }
                else
                {
					if(m_nCameraType==BOCOM_301_200)
					{
						if (15 == nFrameRate)
						{
							buffer->nFrameRate = nFrameRate;
						}
						else if(25 == nFrameRate)
						{
							buffer->nFrameRate = nFrameRate/2;
						}
						else
						{
							LogError("BOCOM_301_200相机帧率错误\r\n");
						}
					}
					else
					{
						buffer->nFrameRate = nFrameRate/buffer->uFrameType;
					}
                }
                buffer->uMarginX = m_nMarginX;
                buffer->uMarginY = m_nMarginY;

                buffer->width = nWidth;//header->nWidth;
                buffer->height = nHeight;//(header->nHeight)*(buffer->uFrameType)/2;
                buffer->size = nSize;//(header->nSize)& 0x00ffffff;
                //test the size value
                //printf("--------------buffer->nFrameRate  = %d,buffer->uFrameType=%d,nFrameRate=%d\n", buffer->nFrameRate,buffer->uFrameType,nFrameRate);
                //test over
                buffer->uGain = m_cfg.uGain/32.0;
                buffer->uMaxGain = m_cfg.nMaxGain/32.0;

                //printf("========nx=%d,nFieldSeq=%d,header->nSize=%x,header->nWidth=%d,header->nHeight=%d\n",nx,nFieldSeq,header->nSize,header->nWidth,header->nHeight);

                if(m_bCameraControl)//控制过相机
                {
                    buffer->nCameraControl = 1;
                    m_bCameraControl = false;
                }
                else
                {
                    buffer->nCameraControl = 0;
                }
			}
            //共同参数
             //时间戳
            //buffer->ts = (buffer->nSeq)*((int64_t)40000)*(buffer->uFrameType);
            //gettimeofday(&tv,NULL);
            buffer->ts = tv.tv_sec*1000000+tv.tv_usec;;
			buffer->uTimestamp = uTimeStamp;
//printf("=========buffer->uTimestamp=%d\n",buffer->uTimestamp);
			memcpy(buffer->cType,header->cType,4);
			memcpy(buffer->cSynchron,header->cSynchron,4);
			//printf("nFieldSeq=%d,nFrameRate=%d,uFrameType=%d,nWidth=%d,nHeight=%d,nSize=%d,m_nMarginX=%d,m_nMarginY=%d\n", nFieldSeq,buffer->nFrameRate,buffer->uFrameType,nWidth,nHeight,nSize,m_nMarginX,m_nMarginY);

            
			//场图像需要判断是否黑屏
			if(m_nCameraType==JAI_CAMERA_LINK_FIELD)
			{
                int nBlackCount = 0;

                int nLineStep =   m_nWidth*2;
                unsigned char* pDataOri =  m_pFrameBuffer+sizeof(yuv_video_buf)+nLineStep*(m_nHeight*2/3)+m_nMarginX*2+1000;
                unsigned char* pData = NULL;

                //10*10共一百个点
                for(int i = 0; i < 100;i+=10)
                {
                    pData  =  pDataOri + nLineStep*i;

                    for(int j = 0; j < 300;j+=30)
                    {
                        if( (pData[j] == 0x80)  && (pData[j+1] == 0x10) && (pData[j+2] == 0x80)  && (pData[j+3] == 0x10))
                        {
                            nBlackCount++;
                        }
                    }
                }

               if(nBlackCount==100)
               {
                   nFlag = -1;
                   LogNormal("检测到黑屏!\n");

				   //存图确认是否真正是黑屏
				   //////////////////////////////////
				   /* yuv_video_buf* pCurBuffer = (yuv_video_buf*)(m_pFrameBuffer);
					int n = 2;
					pCurBuffer->data = (BYTE*)(m_pFrameBuffer + sizeof(yuv_video_buf)) - n;

					int nWidth  = m_nWidth;
					int nHeight = m_nHeight;
					IppiSize srcSize= {nWidth, nHeight};
					int desstep = nWidth * 3 ;
					int srcstep = nWidth * 2;

					IplImage* pImageData = cvCreateImage(cvSize(nWidth,nHeight),8,3);
					IplImage* pImageData1 = cvCreateImage(cvSize(nWidth/8,nHeight/8),8,3);

					//yuv->rgb
				    ippiCbYCr422ToRGB_8u_C2C3R(pCurBuffer->data,srcstep,(unsigned char*)pImageData->imageData,desstep,srcSize);

					cvResize(pImageData,pImageData1);
					
					if(access("./BlackPic",0) != 0) //目录不存在
					{
						mkdir("./BlackPic",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
						sync();
					}

					char buf[256] = {0};
					sprintf(buf,"./BlackPic/%d.jpg",m_uSeq);
					cvSaveImage(buf,pImageData1);

					
					if(pImageData)
					{
						cvReleaseImage(&pImageData);
						pImageData = NULL;
					}

					if(pImageData1)
					{
						cvReleaseImage(&pImageData1);
						pImageData1 = NULL;
					}*/
                    
				   //////////////////////////////////
               }
               else
               {
                   nFlag = nBlockSeq;
               }
			}
			else
			{
			    nFlag = nBlockSeq;
			}
           break;
		}
		else
		{
			bytes = 0;
	        index = 0;
	        //LogError("未找到yuv同步头");
		}
	}
	return nFlag;
}

//重新连接相机
bool CSkpRoadVideoYuv:: ReOpen()
{
    if(!m_bReadFile)
    {
        
		if (m_nPixelFormat == VEDIO_H264)
		{

			if(tcp_set_fd!=-1)
			{
				shutdown(tcp_set_fd,2);
				close(tcp_set_fd);
				tcp_set_fd = -1;
			}

			if(tcp_data_fd!=-1)
			{
				shutdown(tcp_data_fd,2);
				close(tcp_data_fd);
				tcp_data_fd = -1;
			}

			if (!conect_set_tcp())
			{
				LogError("无法与相机建立控制TCP连接\r\n");
				return false;
			}

			StartSendStream();

			if (!conect_data_tcp())
			{
				LogError("无法与相机建立控制TCP连接\r\n");
				return false;
			}
		}

		else
		{
			if(m_nYuvFd > 0)
			{
				shutdown(m_nYuvFd,2);
				close(m_nYuvFd);
				m_nYuvFd = -1;
			}

			if(m_bUnicast)
			{
				if(!connect_udp_unicast())
			 {
				 LogError("重新连接相机失败!\n");
				 return false;
			 }
			}
			else
			{
				if(!Connect_udp())
				{
					LogError("重新连接相机失败!\n");
					return false;
				}
			}
		}
		//关闭
     

		if (!m_bComRet)//如果串口打开失败，就用网口控制
		{   if (m_nPixelFormat == VEDIO_H264)
		    {
				UINT32 nTempIP = m_cfg.uCameraIP ;//相机控制IP等于相机视频IP加一
				string strControlIP = ConvertIPToString(nTempIP);
				LogNormal("strControlIP=%s",strControlIP.c_str());
				//strControlIP = "192.168.58.1";
				//g_CameraSerialComm.OpenDev(strControlIP, 3600);

		    }
		else
		   {
			   UINT32 nTempIP = m_cfg.uCameraIP + (1<<24);//相机控制IP等于相机视频IP加一
			   string strControlIP = ConvertIPToString(nTempIP);
			   LogNormal("strControlIP=%s",strControlIP.c_str());
			   //strControlIP = "192.168.58.1";
			   //g_CameraSerialComm.OpenDev(strControlIP, 3600);

		   }
			
		}
        g_uTime = g_uLastTime;
         //相机参数设置
        SetCaMeraPara();

        LogNormal("相机自动重新连接\r\n");
    }
    return true;
}

bool CSkpRoadVideoYuv:: ChangeMode()
{

	/*0x01a0 工作模式
	"0= 连续模式（25Hz）    5= Edge Pre Select（边沿触发）"	

	0x01a6 外触发输入
	"0=内触发（内部产生15Hz的触发源）  1=外部触发源"	*/
	CAMERA_CONFIG cfg = m_cfg;//获取当前相机配置
	if(m_mode == 0)//连续方式
    {
		g_CameraSerialComm.ChangeMode(m_mode,m_nCamID);
						  //      cfg.fValue = 0;
								//cfg.nMode = (int)cfg.fValue;
								//cfg.nIndex = (int)CAMERA_MODE;
						  //      //g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_MODE);
								////g_CameraSerialComm.SendMessage(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex);
								//g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,0,m_nCamID);
								//g_CameraSerialComm.SaveCamPara(m_nCamID);
		return true;
    }
    else if(m_mode == 1)//触发方式
    {
		g_CameraSerialComm.ChangeMode(m_mode,m_nCamID);
						  //      //设置相机为外触发
								////cfg.fValue = 5;
								//cfg.fValue = 2;
								//cfg.nMode = (int)cfg.fValue;
								//cfg.nIndex = (int)CAMERA_MODE;
								////g_CameraSerialComm.SendMessage(cfg,m_cfg,CAMERA_CMD);
								////g_CameraSerialComm.SendMessage(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex);
								//g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,(CAMERA_MESSAGE)cfg.nIndex,0,m_nCamID);
								//g_CameraSerialComm.SaveCamPara(m_nCamID);
		return true;
    }
	else
	{
		return false;
	}
}

//将客户端设置的相机参数转换为相机自身能识别的参数
void CSkpRoadVideoYuv::ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad)
{
   //以下3种相机的参数转换公式与别的不同
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	{
		if(cfg.EEN_on == 1)//开灯
		{
			cfg.EEN_on = 0;
		}
		else if(cfg.EEN_on == 0)//关灯
		{
			cfg.EEN_on = 1;
		}
		if(cfg.uPol == 1)//
		{
			cfg.uPol = 0;
		}
		else if(cfg.uPol == 0)//
		{
			cfg.uPol = 1;
		}
		if(!bReverseConvert)//给相机的值
		{
			cfg.fGain = cfg.fGain / 0.035;
			if(m_nCameraType == BOCOM_301_200)//最小值是2行
			{
				cfg.fSH = cfg.fSH  / 32;
				cfg.EEN_width = cfg.EEN_width  / 32;
				cfg.EEN_delay = cfg.EEN_delay  / 32;
				if(cfg.fSH - 2 < 0.001)
				{
					cfg.fSH = 2;
				}
			}
			else if(m_nCameraType == BOCOM_302_200)//最小值是1行
			{
				cfg.fSH = cfg.fSH  / 34.725;
				cfg.EEN_width = cfg.EEN_width  / 34.725;
				cfg.EEN_delay = cfg.EEN_delay  / 34.725;
				if(cfg.fSH - 1 < 0.001)
				{
					cfg.fSH = 1;
				}
			}
			else if(m_nCameraType == BOCOM_302_500)
			{
				cfg.fSH = cfg.fSH  / 36.95;
				cfg.EEN_width = cfg.EEN_width  / 36.95;
				cfg.EEN_delay = cfg.EEN_delay  / 36.95;
			}
			else if(m_nCameraType == BOCOM_301_500)//最小值是1行
			{
				cfg.fSH = cfg.fSH  / 32.067;
				cfg.EEN_width = cfg.EEN_width  / 32.067;
				cfg.EEN_delay = cfg.EEN_delay  / 32.067;
				if(cfg.fSH - 1 < 0.001)
				{
					cfg.fSH = 1;
				}
			}
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
			}
		}
		else if(bReverseConvert)
		{
			cfg.fGain = cfg.fGain * 0.035 + 0.5;
			if(m_nCameraType == BOCOM_302_200)
			{
				cfg.fSH = (cfg.fSH * 34.725) +0.5;
				cfg.EEN_width = (cfg.EEN_width * 34.725) +0.5;
				cfg.EEN_delay = (cfg.EEN_delay * 34.725) +0.5;
			}
			else if(m_nCameraType == BOCOM_301_200)
			{
				cfg.fSH = (cfg.fSH * 32) +0.5;
				cfg.EEN_width = (cfg.EEN_width * 32) +0.5;
				cfg.EEN_delay = (cfg.EEN_delay * 32) +0.5;
			}
			else if(m_nCameraType == BOCOM_302_500)
			{
				cfg.fSH = (cfg.fSH * 36.95) +0.5;
				cfg.EEN_width = (cfg.EEN_width * 36.95) +0.5;
				cfg.EEN_delay = (cfg.EEN_delay * 36.95) +0.5;
			}
			else if(m_nCameraType == BOCOM_301_500)
			{
				cfg.fSH = (cfg.fSH * 32.067) +0.5;
				cfg.EEN_width = (cfg.EEN_width * 32.067) +0.5;
				cfg.EEN_delay = (cfg.EEN_delay * 32.067) +0.5;
			}
		}
	}

	else
	{
		if(cfg.EEN_on == 1)//开灯
		{
			cfg.EEN_on = 0;
		}
		else if(cfg.EEN_on == 0)//关灯
		{
			cfg.EEN_on = 1;
		}

		if(!bReverseConvert)//客户端转相机
		{
			cfg.EEN_delay = cfg.EEN_delay * 1/33.3+0.5;
			cfg.EEN_width = cfg.EEN_width * 1/33.3+0.5;

			{
				cfg.nMaxPE = cfg.nMaxPE * 1/33.3+0.5;
				cfg.nMaxPE2 = cfg.nMaxPE2 * 1/33.3+0.5;
				cfg.nMaxSH = cfg.nMaxSH * 1/33.3+0.5;
				cfg.nMaxSH2 = cfg.nMaxSH2 * 1/33.3+0.5;
				cfg.nMaxGain = cfg.nMaxGain*768/24.0;
				cfg.nMaxGain2 = cfg.nMaxGain2*768/24.0;

				if(!bFirstLoad)
				{
					cfg.uGain = cfg.uGain*768/24.0;
					cfg.uPE = cfg.uPE * 1/33.3+0.5;
				}
			}

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
			}
		}
		else//相机转客户端
		{
			cfg.EEN_delay = cfg.EEN_delay * 33.3+0.5;
			cfg.EEN_width = cfg.EEN_width * 33.3+0.5;
			{
				cfg.uGain = cfg.uGain*24.0/768;
				cfg.uPE = cfg.uPE * 33.3+0.5;
				cfg.nMaxGain = cfg.nMaxGain*24.0/768;
				cfg.nMaxPE = cfg.nMaxPE * 33.3+0.5;
				cfg.nMaxGain2 = cfg.nMaxGain2*24.0/768;
				cfg.nMaxPE2 = cfg.nMaxPE2 * 33.3+0.5;
			}
		}
	}
}

//获取相机默认模板
void CSkpRoadVideoYuv::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{
    cfg.AGC = 0;
    cfg.ASC = 0;
    cfg.EEN_delay = 67;
    cfg.EEN_width = 1000;
    cfg.uSM = 1;
    cfg.uPol = 1;
    cfg.nMaxSH = 1665;
    cfg.nMaxPE = 1665;
    cfg.nMaxPE2 = 6000;
    cfg.nMaxGain = 12;
    cfg.nMaxGain2 = 12;
    cfg.nMode = 0;
    cfg.nLightType = 1;
    cfg.UseLUT = 0;
    cfg.nFrequency = 15;
}

//
string CSkpRoadVideoYuv::ConvertIPToString(UINT32 nIP)
{
	string strIP = "";
	int nTempIP[4] = {0};
	nTempIP[3] = (nIP & 0xFF000000) >> 24; 
	nTempIP[2] = (nIP & 0xFF0000) >> 16; 
	nTempIP[1] = (nIP & 0xFF00) >> 8; 
	nTempIP[0] = nIP & 0xFF;
	int i =0;
	while(1)
	{
		char chTempIp[5] = {0};
		sprintf(chTempIp,"%u",nTempIP[i]);
		strIP += chTempIp;
		if(i == 3)
		{
			break;
		}
		strIP += ".";
		i++;
	}
	return strIP;
}

UINT32 CSkpRoadVideoYuv::ConvertStringToIP(string strIP)
{
	bool flag = true;
	int nHighIP1 = 0;//高位IP的第一位
	int nHighIP2 = 0;//高位IP的第二位
	int nLowIP1 = 0;//低位IP的第一位
	int nLowIP2 = 0;//低位IP的第二位
	int nPoint = 0;//记录找到的点的个数
	string tempstrIP;
	while(flag)
	{
		int nLocate = strIP.find(".");
		if(nLocate == string::npos)
		{
				printf("mei you\n");
				flag = false;
		}
		else
		{
				nPoint++;
				tempstrIP = strIP.substr(0,nLocate);
				strIP = strIP.substr(nLocate +1);
		}
		if(nPoint == 1)
		{
				nHighIP1 = strtoul(tempstrIP.c_str(),0,10);
		}
		if(nPoint == 2)
		{
				nHighIP2 = strtoul(tempstrIP.c_str(),0,10);
		}
		if(nPoint == 3)
		{
				nLowIP1 = strtoul(tempstrIP.c_str(),0,10);
		}
	}
	if(strIP != "")
	{
		tempstrIP = strIP;
		nLowIP2 = strtoul(tempstrIP.c_str(),0,10);
	}

	UINT32 IpAddress = 0;

	IpAddress =	 nHighIP1;
	IpAddress |= nHighIP2 << 8;
	IpAddress |= nLowIP1 << 16;
	IpAddress |= nLowIP2 << 24;
	return IpAddress;
}

//触发模式下的相机参数调节（根据时间调节）
void CSkpRoadVideoYuv::AdjustCamByTime()
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
		cfg.fGain =400;//固定增益
		cfg.fSH = 50;
	}
	else if((uTime>=500)&&(uTime<=830))//5->8.30//早晨
	{
		cfg.fGain =200;//固定增益
		cfg.fSH = 25;
	}
	else if((uTime>=830)&&(uTime<=1030))//8.30->10.30//上午
	{
		cfg.fGain = 100;
		cfg.fSH = 10;
	}
	else if((uTime>=1030)&&(uTime<=1430))//10.30->14.30//中午
	{
		cfg.fGain = 50;
		cfg.fSH = 5;
	}
	else if((uTime>=1430)&&(uTime<=1630))//14.30->16.30//下午
	{
		cfg.fGain = 100;
		cfg.fSH = 10;
	}
	else if((uTime>=1630)&&(uTime<=1800))//16.30->18//黄昏
	{
		cfg.fGain = 200;
		cfg.fSH = 15;
	}
	else if((uTime>=1800)&&(uTime<=2400))//18->24//前半夜
	{
		cfg.fGain =400;//固定增益
		cfg.fSH = 50;
	}
	if((m_nCameraType == BOCOM_301_200) || (m_nCameraType == BOCOM_302_500) || (m_nCameraType == BOCOM_301_500) || (m_nCameraType == BOCOM_302_200))
	{
		cfg.fSH = cfg.fSH * 33;
		cfg.fGain = cfg.fGain  * 24 / 768;
		ConvertCameraParameter(cfg,false);
		if(m_cfg.ASC == 0)
		{
			g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_PE,0,m_nCamID);
		}
		else
		{
			cfg.fGain = m_cfg.fGain;
		}
		//usleep(1000*10);
		if(m_cfg.AGC == 0)
		{
			g_CameraSerialComm.SendMessageByKodak(cfg,m_cfg,CAMERA_GAIN,0,m_nCamID);
		}
		else
		{
			cfg.fSH = m_cfg.fSH;
		}
		if ((m_cfg.ASC == 0) || (m_cfg.AGC == 0))
		{
			WriteCameraFloatIni(cfg, m_nCamID);
		}		
	}
}

int CSkpRoadVideoYuv::ConvertQBoxToH264( unsigned char *src, unsigned char  *dst)
{
	int i,boxSize;
	unsigned char *pSrc, *pDst;
	unsigned int nalStartCode = 0x01000000;
	unsigned int nalSize;
	unsigned int elemSize;

	boxSize = BE32(*(unsigned int *) (src));

	printf("boxSize = %d\n",boxSize);
	pSrc = src + QBOX_HEADER_SIZE;
	pDst = dst;
	boxSize -= QBOX_HEADER_SIZE;

	// Iterate through each NAL unit
	while (boxSize >= 4)
	{
		nalSize = (*(pSrc) << 24) | (*(pSrc + 1) << 16) | (*(pSrc + 2) << 8) | (*(pSrc + 3));
		pSrc += sizeof(unsigned int);
		boxSize -= sizeof(unsigned int);
		printf("nalu size = %u \n",nalSize);
		memcpy(pDst, &nalStartCode, sizeof(unsigned int));
		pDst += sizeof(unsigned int);  
		printf("memcpy one \n");
		memcpy(pDst, pSrc, nalSize);
		printf("memcpy two \n");
		pSrc += nalSize;
		pDst += nalSize;
		boxSize -= nalSize;
	}
	elemSize = (int) pSrc - (int) src;
	elemSize -= QBOX_HEADER_SIZE;    
	// Returns size of the elementary packet
	if(elemSize>60*1024)
	{
		printf("*********************************\n");	
		printf("frame size %d\n",elemSize);
		printf("*********************************\n");	
	}
	return elemSize;
}