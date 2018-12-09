#include "DspVideoUnit.h"
#include "Common.h"
#include "CommonHeader.h"


CDspVideoUnit::CDspVideoUnit()
{
	m_pathInfo.fpOut = NULL;
	m_pathInfo.strPath = "";
	m_pathInfo.uBeginTime = 0;
	m_pathInfo.uEndTime;
	m_pathInfo.uId = 0;
	m_nRecvNodataCounts = 0;
}

CDspVideoUnit::~CDspVideoUnit()
{
	m_bEndCapture = true;
	m_bH264Capture = false;
}

//接收h264数据线程
void* RecvDsph264Thread(void* pArg)
{
	printf("------recv---RecvDsph264Thread----->>>>\n");
	DspClassAndFd classfd = *(DspClassAndFd *)pArg;
	(classfd.p)->RecvH264Fd();

	LogNormal("客户端线程退出\n");
	pthread_exit((void *)0);

	return pArg;
}

bool CDspVideoUnit::InitTcp(bool bEndCapture, bool bH264Capture)
{
	m_bEndCapture = bEndCapture;
	m_bH264Capture  = bH264Capture;
	m_nChannelId = 1;

	//启动接收线程
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动监控线程
	int nret = pthread_create(&m_classCfd.id,&attr, RecvDsph264Thread, (void*)&m_classCfd);

	//成功
	if(nret != 0)
	{
		//失败
		LogError("为客户端:%d创建接收h264数据线程失败,连接断开!\r\n",m_classCfd.cfd.fd);
		return false;
	}

	m_bH264Capture = true;

	LogNormal("Thrad Destroy id:%x CDspVideoUnit", m_classCfd.id);

	pthread_attr_destroy(&attr);

	return true;
}

//bool CDspVideoUnit::Seth264RecFd(int& fd, UINT32 uId)
bool CDspVideoUnit::Seth264RecFd(DspClassAndFd classfd)
{
	LogNormal("----Seth264RecFd-oldfd:%d--fd=%d--\n", m_classCfd.cfd.fd, classfd.cfd.fd);
	
	classfd.p = this;
	m_classCfd.p = this;

	m_classCfd.cfd.uCameraId = classfd.cfd.uCameraId;
	m_classCfd.cfd.fd = classfd.cfd.fd;
	m_classCfd.cfd.TimeCount = classfd.cfd.TimeCount;
	m_classCfd.cfd.connectFlag = classfd.cfd.connectFlag;

	return true;
}

//TCP 接收数据
void CDspVideoUnit::RecvH264Fd()
{	
	//printf("---------RecvH264Fd---->>>>>>>>>-m_bH264Capture=%d m_bEndCapture=%d \n", m_bH264Capture, m_bEndCapture);
	
	//采集时间(以微妙为单位)
	struct timeval tv;
	int nBytes = 0;
	char chBuffer[204800];
	int nRecvSize = 0;
	bool bRet = false;

	int64_t tick = 0;
	int64_t tickPre = 0;
	int64_t tickDis = 0;

	while(!m_bEndCapture)
	{
		if(m_bH264Capture)
		{
			//printf("-m_nRecvNodataCounts=%d--\n", m_nRecvNodataCounts);
			if(m_classCfd.cfd.fd < 0)
			{
				//m_nRecvNodataCounts++;
				usleep(10 * 1000);
				continue;
			}

			{
				nBytes = 0;
				char buf[4] = {0};
				int  nRepeatCountPre(0),nRepeatCountNow(0);
				bool bJustBegin = false;
				int  nJustEndIndex = 0;
				//收数据
				while(!m_bEndCapture) //while-2
				{
					nRepeatCountNow=0;
					nJustEndIndex=0;
					
					//接收的数据
					std::string response;
					response.clear();
					//接收同步头"$$$$"
					memset(buf,0,4);

					//if((nBytes = recv(nRecvFd,buf,4,MSG_NOSIGNAL)) <= 0)
					bRet = SafeRecvh264Fd(m_classCfd.cfd, buf, 4, nRecvSize);
					//printf("=######========nRecvSize=%d==\n", nRecvSize);
					if(!bRet)
					{
						//接收出错
						if(errno == 4)
						{
							//LogError("continue 接收h264同步头出错errno=%d\n",nBytes,errno);
							continue;
						}
						//LogError("接收h264同步头出错nBytes=%d,errno=%d\n",nBytes,errno);
						//m_nRecvNodataCounts += 500;
						break;
					}

					//即使某包错位，可以保证找到下包
					if('$'==buf[3])
					{
						++nRepeatCountNow;
						if('$'==buf[2])
						{
							++nRepeatCountNow;
							if('$'==buf[1])
							{
								++nRepeatCountNow;
								if('$'==buf[0])
								{
									++nRepeatCountNow;
									bJustBegin=true;
								}
							}
						}
					}
					if(false==bJustBegin)
					{
						if(3==nRepeatCountPre)
						{
							if('$'!=buf[0])
							{
								nRepeatCountPre=nRepeatCountNow;
								continue;
							}
							else
							{
								nJustEndIndex=1;
							}
						}
						else if(2==nRepeatCountPre)
						{
							if('$'!=buf[0]||'$'!=buf[1])
							{
								nRepeatCountPre=nRepeatCountNow;
								continue;
							}
							else
							{
								nJustEndIndex=2;
							}
						}
						else if(1==nRepeatCountPre)
						{
							if('$'!=buf[0]||'$'!=buf[1]||'$'!=buf[2])
							{
								nRepeatCountPre=nRepeatCountNow;
								continue;
							}
							else
							{
								nJustEndIndex=3;
							}
						}
						else if(0==nRepeatCountPre)
						{
							if('$'!=buf[0]||'$'!=buf[1]||'$'!=buf[2]||'$'!=buf[3])
							{
								nRepeatCountPre=nRepeatCountNow;
								continue;
							}
						}
					}
					nRepeatCountPre=0;
					bJustBegin=false;

					//接收帧长度
					UINT32 nVideo_Size = 0;
					//if((nBytes = recv(nRecvFd,(char*)&nVideo_Size,sizeof(UINT32),MSG_NOSIGNAL)) != sizeof(UINT32))
					bRet = SafeRecvh264Fd(m_classCfd.cfd, (char*)&nVideo_Size, 4, nRecvSize);
					if(!bRet)
					{
						//LogError("接收h264帧长度出错nBytes=%d\n",nBytes);
						//m_nRecvNodataCounts += 500;
						break;
					}

					//printf("=##?????????####SafeRecvh264Fd=nVideo_Size=%d=\n", nVideo_Size);

					if(nVideo_Size <= 0)
					{
						//m_nRecvNodataCounts++;
						//LogError("h264帧长度错误\n");
						break;
					}

					bool bRecvData = true;
					int nLeft = nVideo_Size;
					int nDataLength = 0;

					while(nLeft >  0 && !m_bEndCapture)
					{
						//if((nBytes = recv(cfd.fd,chBuffer,204800<nLeft?204800:nLeft,MSG_NOSIGNAL)) <= 0)
						memset(chBuffer, 0, 204800);
						bRet = SafeRecvh264Fd(m_classCfd.cfd, chBuffer, nLeft, nRecvSize);
						if(!bRet)
						{
							if(errno == 4)
							{
								//LogError("continue 接收h264数据出错errno=%d\n",nBytes,errno);
								continue;
							}
							//LogError("接收h264数据出错nBytes=%d,%d=%s\n",nBytes,errno,strerror(errno));
							//m_nRecvNodataCounts += 500;
							bRecvData = false;
							break;
						}
						else
						{							
							//保存数据
							response.append(chBuffer,nRecvSize);

							nLeft -= nRecvSize;
							nDataLength += nRecvSize;
						}
					}

					if(!bRecvData)
					{						
						break;
					}

					//printf("-------nDataLength=%d,-nVideo_Size=%d---\n", nDataLength, nVideo_Size);
					if(nDataLength == nVideo_Size)
					{
						//m_nRecvNodataCounts = 0;
						gettimeofday(&tv,NULL);
						tick = tv.tv_sec * 1000000 + tv.tv_usec;

						tickDis = (tick - tickPre)/1000/1000;
						//30秒发送心跳回复
						if(tickDis >= 30)
						{
							bool bLink = SendToDspLinkFd(m_classCfd.cfd);

							if(!bLink)
							{
								LogNormal("发送video 心跳失败!\n");
							}

							tickPre = tick;//心跳不管是否发送成功与否都只隔30秒一次
						}

						//printf("-id:%lld--sock:%d, ip:%s-CaptureVideoFrame response.size()=%d  \n", \
							m_classCfd.cfd.uCameraId, cfd.fd, cfd.strIp.c_str(), response.size());

						if(m_classCfd.cfd.uCameraId > 0)
						{								
							//m_bWriting = true;
							CaptureVideoFrame(tick, response, m_classCfd.cfd.uCameraId, m_classCfd.cfd.fd, m_classCfd.cfd.strIp);
							//m_bWriting = false;
						}
						else
						{
							//printf("-id:%lld--sock:%d, ip:%s-CaptureVideoFrame response.size()=%d  \n", \
								cfd.uCameraId, cfd.fd, cfd.strIp.c_str(), response.size());
						}

					}//End of if(nDataLength == nVideo_Size)
				}//End of while(!m_bEndCapture) //while-2
			}

		}//End of if(m_bH264Capture)
		else
		{
			//m_nRecvNodataCounts++;
			usleep(10*1000);
		}
	}//End of while(!m_bEndCapture)
}

//接收h264数据包
bool CDspVideoUnit::SafeRecvh264Fd(Client_Fd& cfd,char*pBuf, int nLen, int &nRecvSize)
{
	if(cfd.fd <= 0)
	{
		return false;
	}

	int nRet = 0;
	int nRecved = 0;
	bool bRet = true;
	//int nCount = 0; //记录未接收到数据的次数
	//int flag = 0; //套接字锁是否被销毁，1：是，0:否
	cfd.TimeCount = 0;

	//pthread_mutex_lock(&cfd.FdMutex);
	while (nRecved < nLen)
	{
		nRet = recv(cfd.fd, pBuf + nRecved, nLen - nRecved, MSG_NOSIGNAL);
		if(nRet > 0)
		{
			nRecved += nRet;
			cfd.TimeCount = 0;

			//printf("===---->#===SafeRecvh264Fd=nRecved=%d=nLen=%d \n", nRecved, nLen);
			continue;
		}
		else if((nRet == 0) && (errno == 0))
		{
			//pthread_mutex_unlock(&cfd.FdMutex);
			//pthread_mutex_destroy(&cfd.FdMutex);
			//flag = 1;
			break;
		}
		else
		{
			usleep(10*1000);
			cfd.TimeCount += 1;
			#ifndef ALGORITHM_YUV
			if(cfd.TimeCount%H264_DSP_COUNT == 0)
			{
				//LogNormal("cfd.TimeCount=%d\n",cfd.TimeCount);
				//LogNormal("strerror=%s,errno=%d,m_nTcpFd=%d\n",strerror(errno),errno,cfd.fd);

				//pthread_mutex_unlock(&cfd.FdMutex);
				//pthread_mutex_destroy(&cfd.FdMutex);
				//flag = 1;
				break;
			}
			#endif
			continue;
		}
	}
	if (nRecved < nLen)
	{
		bRet = false;
	}

	nRecvSize = nRecved;

	//if(flag == 0)
	//{
		//pthread_mutex_unlock(&cfd.FdMutex);
	//}
	return bRet;
}

//采集数据存录像
void CDspVideoUnit::CaptureVideoFrame(const int64_t &tick, const std::string &response, const UINT32 uId, const int nSock, const std::string strIp)
{
	//printf("--111--CaptureVideoFrame---uId=%d---tick=%lld \n", uId, tick);

	struct timeval tv;
	int nFrameSize = response.size();
	//const char * pBuffer = response.c_str();
	int iWrite = 0;

	std::string strH264FileName = "";

	//printf("--111--CaptureVideoFrame---uId=%d---tick=%lld path=%s \n", \
		uId, tick, m_pathInfo.strPath.c_str());
	//printf("		\t-fp:%x, uBe=%lld, uEn=%lld ---\n", m_pathInfo.fpOut, m_pathInfo.uBeginTime, m_pathInfo.uEndTime);

	//printf("-CaptureVideoFrame--nFrameSize=%d m_uBeginTime=%d m_uEndTime=%d \n", nFrameSize, m_uBeginTime, m_uEndTime);
	if(nFrameSize > 0)
	{
		if(0 == m_pathInfo.uBeginTime)
		{
			m_pathInfo.uBeginTime = tick; //单位 us
			m_pathInfo.uEndTime = m_pathInfo.uBeginTime + g_VideoFormatInfo.nTimeLength * 60 * 1000 * 1000;			

			/*if(g_nServerType == 13 && g_nFtpServer == 1)
			{
				RECORD_PLATE plate;
				plate.uTime = (m_pathInfo.uBeginTime/1000)/1000;
				plate.uMiTime = (m_pathInfo.uBeginTime/1000)%1000;
				plate.uRoadWayID = 1;
				g_MyCenterServer.GetPlatePicPath(plate,m_pathInfo.strPath,2);
			}
			else*/
			{
				//strH264FileName = g_FileManage.GetDspVideoPath(m_nChannelId);
				strH264FileName = g_FileManage.GetMulDspVideoPath(uId);				
				//RefreshIpPath(uId, strH264FileName);

				m_pathInfo.fpOut = fopen(strH264FileName.c_str(), "ab+");
				if(NULL == m_pathInfo.fpOut)
				{
					LogNormal("Open file err! path=%s. \n", m_pathInfo.strPath.c_str());
				}
				else
				{
					m_pathInfo.strPath = strH264FileName;
				}
				
			}
			g_skpDB.SaveVideo(m_nChannelId,(m_pathInfo.uBeginTime/1000)/1000,(m_pathInfo.uEndTime/1000)/1000,m_pathInfo.strPath,0);
		}		

		if(m_pathInfo.uBeginTime != 0)
		{
			//存信息
			if(m_pathInfo.fpOut != NULL && m_pathInfo.strPath.size() > 0)
			{			
				bool bWrite = SafeWrite(m_pathInfo.fpOut, response.c_str(), nFrameSize);

				if(!bWrite)
				{
					LogNormal("Write dspVideo file error!");
				}
				//printf("---id=%d---fwrite--111-\n", uId);
				/*
				iWrite = fwrite(response.c_str(), 1, nFrameSize, m_pathInfo.fpOut);
				//printf("--H264--iWrite=%d--\n", iWrite);

				if(iWrite != nFrameSize)
				{
					printf("11 write file failed<to write<%d bytes> writed<%d bytes>\n", nFrameSize, iWrite);
					int iWrite2 = fwrite(response.c_str()+iWrite, 1, nFrameSize - iWrite, m_pathInfo.fpOut);

					if(iWrite2 != nFrameSize - iWrite)
					{
						LogNormal("22 write file failed<to write<%d bytes> writed<%d bytes>\n", nFrameSize - iWrite, iWrite2);
					}
				}	
				*/

				usleep(1000*20);
				//printf("---id=%d---fwrite--222-\n", uId);
			}			

			if(tick > m_pathInfo.uEndTime)
			{
				if(m_pathInfo.fpOut && m_pathInfo.strPath.size() > 0)
				{
					printf("---id=%d---fclose--333-\n", uId);
					{
						fclose(m_pathInfo.fpOut);
					}
					printf("---id=%d---fclose--444-\n", uId);					

					m_pathInfo.fpOut = NULL;
					m_pathInfo.strPath = "";

					//RefreshIpPathInfo(uId, m_pathInfo);
				}	
				//通知事件录象完成           
				g_skpDB.VideoSaveUpdate(m_pathInfo.strPath,m_nChannelId,1);
				m_pathInfo.uBeginTime = 0;

				m_pathInfo.uEndTime = 0;
			}
		}//End of if(pathInfo.uBeginTime != 0)		
	}//End of if(nFrameSize > 0)

	//printf("--222--CaptureVideoFrame---uId=%d---tick=%lld \n", uId, tick);
}

void CDspVideoUnit::CloseFd()
{
	//LogNormal("--EndRecFd--uId=%d-cfd.fd=%d-ip:%s \n", \
		m_classCfd.cfd.uCameraId, m_classCfd.cfd.fd, m_classCfd.cfd.strIp.c_str());

	m_classCfd.cfd.TimeCount = 0;
	m_classCfd.cfd.connectFlag = false;
	shutdown(m_classCfd.cfd.fd,2);
	close(m_classCfd.cfd.fd);
	m_classCfd.cfd.fd = -1;

}

void CDspVideoUnit::RefreshId(const UINT32 uId)
{
	LogNormal("-RefreshId---old:%d, new uId:%d \n", m_pathInfo.uId, uId);
	m_classCfd.cfd.uCameraId = uId;
}

bool CDspVideoUnit::UnInit()
{
	m_bEndCapture = true;
	m_bH264Capture = false;

	this->CloseFd();

	//线程退出
	/*if (m_classCfd.id != 0)
	{
	pthread_cancel(m_classCfd.id);
	pthread_join(m_classCfd.id, NULL);

	usleep(200*1000);

	m_classCfd.id = 0;
	}*/

	//LogNormal("-CDspVideoUnit::UnInit()-\n");
}

bool CDspVideoUnit::GetUnitStatus()
{
	if(m_nRecvNodataCounts >= 6000)//60s
	{
		LogNormal("--GetUnitStatus-m_nRecvNodataCounts:%d-\n",m_nRecvNodataCounts);
		return false;
	}

	return true;
}

//发送DSP回复心跳包
bool CDspVideoUnit::SendToDspLinkFd(const Client_Fd& cfd)
{
	bool bRet = false;
	if(cfd.fd < 0)
	{
		usleep(5 * 1000);
		return false;
	}

	int nSendFailCount = 0;
	int nCount = 3;

	char buf[4] = {'$', '$', '$', '$'};

	while(nCount > 0)
	{
		int nBytes = send(cfd.fd, (char*)&buf, 4, MSG_NOSIGNAL);

		if(nBytes != 4)
		{
			//LogError("发送心跳包出错,Sock=[%d] \n", cfd.fd);
			nSendFailCount++;

			if(nSendFailCount > 2)
			{
				bRet = false;
				//LogNormal("-er SendToDspLinkFd-video bRet=%d \n", bRet);
				break;
			}
		}	
		else
		{			
			bRet = true;
			break;
		}

		nCount --;
		usleep(10*1000);
	}

	//LogNormal("-SendToDspLinkFd-video-bRet=%d \n", bRet);

	return bRet;
}