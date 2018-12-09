#include "Axis.h"
#include "Common.h"
#include "CommonHeader.h"

Axis::Axis(int nCameraType,int nPixelFormat)
{
	m_nCameraType=nCameraType;
	m_nPixelFormat=nPixelFormat;


	m_nMaxBufferSize = FRAMESIZE;//?

	m_nWidth=1280;
	m_nHeight=960;
	m_nMarginX=0;
	m_nMarginY=0;
	m_pFrameBuffer=NULL;
    tcp_fd = -1;
	m_nThreadId=0;

	
}

Axis::~Axis()
{
	printf("Axis::~Axis\n");
}


void* ThreadAxisCapture(void* pArg)
{
	Axis* axis=(Axis*)pArg;
	if(axis==NULL)
		return pArg;

	axis->CaptureFrame();
	pthread_exit((void*)0);
	return pArg;
}

bool Axis::Open()
{
	m_cfg.uKind=m_nCameraType;
	LoadCameraProfile(m_cfg);
	ConvertCameraParameter(m_cfg,false,true);

	usleep(1000*100);

	m_bReadFile=false;

	if(connect_tcp()==-1)
	{
		LogError("无法与相机建立tcp连接\r\n");
		return false;
	}

	Init();

	return true;
}

bool Axis::Close()
{
	m_bEndCapture = true;

	if(tcp_fd!=-1)
	{
		shutdown(tcp_fd,2);
		close(tcp_fd);
		tcp_fd = -1;
	}

	if (m_nThreadId != 0)
	{
		pthread_join(m_nThreadId, NULL);
		m_nThreadId = 0;
	}

	pthread_mutex_lock(&m_FrameMutex);

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

	pthread_mutex_unlock(&m_FrameMutex);

	return true;
}

void Axis::Init()
{
	for (int i=0;i<m_nMaxBufferSize;i++)
		m_FrameList[i]=(unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth*m_nHeight,sizeof(unsigned char));

	m_uSeq = 0;
	m_nPreFieldSeq  = 0;//?
	m_nCurIndex = 0;
	m_nFirstIndex = 0;
	m_nFrameSize=0;
	m_pFrameBuffer = m_FrameList[0];

	m_bCameraControl = false;
	m_nCountGain = 0;//
	m_bEndCapture = false;

	char *pData =new char [1024*100];
	std::string strUserName = g_monitorHostInfo.chUserName;
	std::string strPassWord = g_monitorHostInfo.chPassWord;

	sprintf(pData,"%s:%s",strUserName.c_str(),strPassWord.c_str());
	std::string request(pData);

	std::string strBase64;
	EncodeBase64(strBase64,(unsigned char*)request.c_str(),request.size());

	delete []pData;

	m_nThreadId = 0;


	int nret=pthread_create(&m_nThreadId,NULL,ThreadAxisCapture,this);
	printf("Axis m_nThreadId=%lld",m_nThreadId);

	if (nret!=0)
	{
		 Close();
		 LogError("创建yuv采集线程失败,无法进行采集！\r\n");
	}

}


void Axis::CaptureFrame()
{
	RecvData();
	LogTrace(NULL,"after RecvData\n");
}



int Axis::RecvData()
{
	struct sockaddr_in recv_addr;
	socklen_t addrlen=sizeof(recv_addr);

	struct timeval tv;

	char buf[256]={0};
	char *pData =new char [1024*100];
	int n=0;
	int nBPos=-1;
	int nEPos=-1;
	int nSize = 1024*100;
	int nLeft = nSize;
	int nDataSize = 0;
	std::string strImage("");

	/*std::string strUserName = g_monitorHostInfo.chUserName;
	std::string strPassWord = g_monitorHostInfo.chPassWord;

	sprintf(pData,"%s:%s",strUserName.c_str(),strPassWord.c_str());
	std::string request(pData);

	std::string strBase64;
	EncodeBase64(strBase64,(unsigned char*)request.c_str(),request.size());*/

	//sprintf(buf,"GET http://%s/axis-cgi/mjpg/video.cgi?resolution=%dx%d&camera=1 HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,m_nWidth,m_nHeight,strBase64.c_str());
    
	/*sprintf(buf,"GET http://%s/axis-cgi/mjpg/video.cgi?camera=1 HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());

	printf("command_send = %s",buf);

	int sg=command_send(buf);*/

	bool sg=videoRequest();

	if (sg)
	{
		nLeft = 200;
		while(1)
		{
			n = recv(tcp_fd,pData,nLeft,MSG_NOSIGNAL);
			if(n>0)
			{
				nLeft -= n;
				printf("n = %d,pData=%s\n",n,pData);
			}

			if(nLeft <= 0)
			{
				break;
			}
		}

		if (strncmp((char *)pData,"HTTP/1.0 200 OK",strlen("HTTP/1.0 200 OK")) != 0)
		{
			perror("error data");
			return -1;
		}
	}
	else
	{
		perror("send command failed");
		return -1;
	}

	strImage.append((char*)pData,n);

	nBPos = strImage.find("boundary=");
	nEPos = strImage.find("\r\n\r\n");
	string strBoundary = strImage.substr(nBPos+strlen("boundary="),nEPos-nBPos-strlen("boundary="));
	printf("strBoundary=%s\n",strBoundary.c_str());
	strImage.erase(0,nEPos+strlen("\r\n\r\n"));

	while (!m_bEndCapture)
	{
		nDataSize = 0;
		nLeft = nSize;
		while(1)
		{
			n=recv(tcp_fd,pData,nLeft,MSG_NOSIGNAL);

			if(n > 0)
			{
				//printf("n=%d\n",n);
				strImage.append((char*)pData,n);
				nLeft -= n;
				nDataSize += n;
			}

			if(nLeft <= 0)
			{
				break;
			}
		}
		
		printf("nDataSize=%d,strImage.size()=%d\n",nDataSize,strImage.size());

		while (!m_bEndCapture)
		{
				nBPos=strImage.find("Content-Length:");
				nEPos=strImage.find("\r\n\r\n");

				printf("nBPos=%d,nEPos=%d,strImage.size()=%d\n",nBPos,nEPos,strImage.size());
				if (nBPos>0&&nEPos>0)
				{
					string strLength=strImage.substr(nBPos+strlen("Content-Length:"),nEPos-nBPos-strlen("Content-Length:"));
					strImage.erase(0,nEPos+strlen("\r\n\r\n"));

					int nLength = atoi(strLength.c_str());

					printf("strLength=%s,nLength=%d\n",strLength.c_str(),nLength);

					if(strImage.size() < nLength)
					{
						nLeft = nLength-strImage.size();

						while(1)
						{
							n=recv(tcp_fd,pData,nLeft,MSG_NOSIGNAL);

							if(n > 0)
							{
								//printf("n=%d\n",n);
								strImage.append((char*)pData,n);
								nLeft -= n;
							}

							if(nLeft <= 0)
							{
								break;
							}
						}
					}
					memcpy(m_pFrameBuffer+sizeof(yuv_video_buf),strImage.c_str(),nLength);
					gettimeofday(&tv,NULL);//?
					strImage.erase(0,nLength);

					yuv_video_buf camera_header;
					memcpy(camera_header.cType,"VYUY",4);
					camera_header.height = m_nHeight;
					camera_header.width = m_nWidth;
					camera_header.size = nLength;

					camera_header.nFrameRate = m_cfg.nFrequency;

					camera_header.uFrameType = 2;

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
					AddFrame(1);

					m_uSeq++;
					//usleep(1000*1);
				}
				else
				{  
					//未找到符合要求的数据则继续读取
					break;
				}
		}

	}

	delete []pData;

	return 1;
}


int Axis::connect_tcp()
{
	struct sockaddr_in tcp_addr;

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

	printf("chMonitorHost=%s,uMonitorPort=%d\n",g_monitorHostInfo.chMonitorHost,g_monitorHostInfo.uMonitorPort);

	bzero(&tcp_addr,sizeof(tcp_addr));
	tcp_addr.sin_family=AF_INET;
	tcp_addr.sin_addr.s_addr=inet_addr(g_monitorHostInfo.chMonitorHost);
	tcp_addr.sin_port=htons(g_monitorHostInfo.uMonitorPort);

	int nCount = 0;
	while(nCount < 3)
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
		nCount++;
		usleep(1000*10);
	}

	return 1;
}

int Axis::command_send(char *buf)
{
	char *buffer;
	buffer=buf;

	int res;

	res=send(tcp_fd,buffer,strlen(buffer),MSG_NOSIGNAL);

	if (res<0)
	{
		perror("send");
		return 0;
	}
	return 1;
}



bool Axis::ReOpen()
{

	char buf[256]={0};
	
	if(!m_bReadFile)
	{
		if(tcp_fd!=-1)
		{
			shutdown(tcp_fd,2);
			close(tcp_fd);
			tcp_fd = -1;
		}

		usleep(1000*100);

		if(connect_tcp()==-1)
		{
			LogError("无法与相机建立tcp连接\r\n");
			return false;
		}

		g_uTime = g_uLastTime;
		
		Init();
		
		LogNormal("视频请求发送成功\r\n");
		LogNormal("相机自动重新连接\r\n");
	}

	return true;
}

bool Axis::videoRequest()
{
	char buf[256]={0};
	int sg;

	sprintf(buf,"GET http://%s/axis-cgi/mjpg/video.cgi?camera=1 HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
	
	printf("command_send = %s",buf);

	sg=command_send(buf);
	if (0==sg)
	{
		LogError("发送请求失败\r\n");
		return false;
	}
	
	return true;
}



int Axis::axis_mode(int a_nMode)//模式设置
{
	char buf[256]={0};
	int sg;

	if(a_nMode==0)//连续模式
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.Image.TriggerDataEnabled=no HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	else if(a_nMode==1)//触发模式
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.Image.TriggerDataEnabled=yes HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	return 1;
}

int Axis::axis_shutterControl(int a_nMode)//是否自动快门
{
	char buf[256]={0};
	int sg;

	if(a_nMode==0)//快门自动
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.ImageSource.I0.Sensor.ManualShutterControl=no HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	else if(a_nMode==1)//手动快门
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.ImageSource.I0.Sensor.ManualShutterControl=yes HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	return 1;
}


int Axis::axis_gainControl(int a_nMode)//是否手动增益
{
	char buf[256]={0};
	int sg;

	if(a_nMode==0)//自动
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.ImageSource.I0.Sensor.ManualGainControl=no HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	else if(a_nMode==1)//手动增益
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.ImageSource.I0.Sensor.ManualGainControl=yes HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	return 1;
}

bool Axis::setPulseTime(int a_nTime)//脉冲宽度设置
{
	char buf[256]={0};
	int sg;
	sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.IOPort.I1.Output.PulseTime=d% HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,a_nTime,strBase64.c_str());
	sg=command_send(buf);
	if (0==sg)
	{
		LogError("发送请求失败\r\n");
		return false;
	}
	return true;
}


bool Axis::setDelayTime(int a_nTime)//延时时间设置
{
	char buf[256]={0};
	int sg;
	sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.IOPort.I1.Output.PulseTime=d% HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,a_nTime,strBase64.c_str());
	sg=command_send(buf);
	if (0==sg)
	{
		LogError("发送请求失败\r\n");
		return false;
	}
	return true;
}


int Axis::axis_outputActiveControl(int a_nMode)//脉冲开关 0：关闭 1：开启
{
	char buf[256]={0};
	int sg;

	if(a_nMode==0)//关闭
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.IOPort.I1.Output.Active=closed HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	else if(a_nMode==1)//开启
	{
		sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.IOPort.I1.Output.Active=open HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,strBase64.c_str());
		sg=command_send(buf);
		if (0==sg)
		{
			LogError("发送请求失败\r\n");
			return 0;
		}
	}
	return 1;
}

bool Axis::setOutputMode(int a_level)//1 OR 0?脉冲极性设置
{
	char buf[256]={0};
	int sg;
	sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.IOPort.I1.Output.PulseTime=d% HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,a_level,strBase64.c_str());
	sg=command_send(buf);
	if (0==sg)
	{
		LogError("发送请求失败\r\n");
		return false;
	}
	return true;
}

bool Axis::setManualShutter(int a_nValue)
{
	char buf[256]={0};
	int sg;
	sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.ImageSource.I0.Sensor.ManualShutter=d% HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,a_nValue,strBase64.c_str());
	sg=command_send(buf);
	if (0==sg)
	{
		LogError("发送请求失败\r\n");
		return false;
	}
	return true;
}

bool Axis::setManualGain(int a_nValue)
{
	char buf[256]={0};
	int sg;
	sprintf(buf,"GET http://%s/axis-cgi/param.cgi?action=update&root.ImageSource.I0.Sensor.ManualGain=d% HTTP/1.0\r\nAuthorization: Basic %s\r\n\r\n",g_monitorHostInfo.chMonitorHost,a_nValue,strBase64.c_str());
	sg=command_send(buf);
	if (0==sg)
	{
		LogError("发送请求失败\r\n");
		return false;
	}
	return true;
}


