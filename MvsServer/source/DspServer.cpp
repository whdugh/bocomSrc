// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "DspServer.h"
#include "Common.h"
#include "global.h"
#ifndef ALGORITHM_YUV

//监控客户端连接线程
void* ThreadMvsDspAccept(void* pArg)
{
	//取类指针
	CDspServer* pRoadDspServer = (CDspServer*)pArg;
	if(pRoadDspServer == NULL)
		return pArg;

	pRoadDspServer->MvsDspAccept();
	LogNormal("------********---OUT---thread---ThreadMvsDspAccept---***************---\n");
	pthread_exit((void *)0);
	
	return pArg;
}

//监控客户端录像线程
void* ThreadMvsDspRecordAccept(void* pArg)
{
	//取类指针
	CDspServer* pRoadDspServer = (CDspServer*)pArg;
	if(pRoadDspServer == NULL)
		return pArg;

	pRoadDspServer->MvsDspRecordAccept();
	LogNormal("------********---OUT---thread---ThreadMvsDspRecordAccept---***************---\n");
	pthread_exit((void *)0);

	return pArg;
}

//接收数据线程
void* ThreadRecvDspData(void* pArg)
{
	//取类指针
	CDspServer* pRoadDspServer = (CDspServer*)pArg;
	if(pRoadDspServer == NULL)
		return pArg;

	int nClient = pRoadDspServer->GetClientFd();
	pRoadDspServer->RecvDspData(nClient);
	LogNormal("------********---OUT---thread---ThreadRecvDspData---***************---\n");
	pthread_exit((void *)0);
	
	return pArg;
}

CDspServer::CDspServer(int nCameraType)
{
    printf("====CDspServer::CDspServer()====\n");
    m_nCameraType = nCameraType;

    if(nCameraType == DSP_SERVER)
    {
        //m_nOriWidth = DSP_500_BIG_WIDTH;
        //m_nOriHeight = DSP_500_BIG_HEIGHT;

		m_nOriWidth = DSP_200_BIG_WIDTH_SERVER;
		m_nOriHeight = DSP_200_BIG_HEIGHT_SERVER;

		//m_nWidth = m_nOriWidth / 4;
		//m_nHeight = m_nOriHeight / 4;
	}
	
	m_nPort = MVS_SERVER_TCP_PORT;


    //m_nTcpFd = -1;
	m_nSerTcpFd = -1;

    //线程ID
    m_nThreadId = 0;
	m_bOpenCamera = false;

	m_bEndCapture = false;
	m_nSocketBase = new mvCSocketBase;	
	m_bInit = false;

	m_pDspDealVideo = NULL;
	m_pDspDealVideoTcp = NULL;
	m_nDisConnectCount = 0;
}

CDspServer::~CDspServer()
{
	LogNormal("=in=~CRoSeekCameraDsp==\n");	

	usleep(1000*1000);

	if(m_nSocketBase != NULL)
	{
		LogNormal("=##==delete m_nSocketBase=\n");
		delete m_nSocketBase;
		m_nSocketBase = NULL;
	}	

	if(m_pManageClient != NULL)
	{
		LogNormal("=##==delete m_pManageClient=\n");
		delete m_pManageClient;
		m_pManageClient = NULL;
	}

	if(m_pDataProcess != NULL)
	{
		LogNormal("=##==delete m_pDataProcess=\n");
		delete m_pDataProcess;
		m_pDataProcess = NULL;
	}
	m_CameraIPMap.clear();
	
	LogNormal("=out=~CDspServer==\n");
}

//绑定和监听
bool CDspServer::InitDspServer()
{
	LogNormal("----------InitDspServer--m_bInit=%d \n", m_bInit);	

	InitDspServerSocket();
	
	m_pManageClient = new CManageClient();
	if(m_pManageClient != NULL)
	{
		LogNormal("---m_pManageClient--new ok!-\n");
	}

	//m_pDataProcess = new CDspDataProcess();
	printf("*********************   g_sysInfo_ex.fTotalMemory :%f **************\n",g_sysInfo_ex.fTotalMemory);
	int nMemory = 2;
	if (g_sysInfo_ex.fTotalMemory < 1.5)
	{
		nMemory = 2;
	}
	else if (g_sysInfo_ex.fTotalMemory < 2.5)
	{
		nMemory = 4;
	}
	else if (g_sysInfo_ex.fTotalMemory < 4.5)
	{
		nMemory = 6;
	}
	else if (g_sysInfo_ex.fTotalMemory < 6.5)
	{
		nMemory = 8;
	}
	else if (g_sysInfo_ex.fTotalMemory < 8.5)
	{
		nMemory = 12;
	}
	else
	{
		nMemory = 15;
	}
	printf("********************* nMemory : %d\n",nMemory);
	m_pDataProcess = new CDspDataManage(nMemory);
	if(m_pDataProcess != NULL)
	{
		LogNormal("---m_pDataProcess--new ok!-\n");
		//int nChannelId = m_nChannelId;

		//LogNormal("----m_nOriWidth=%d, m_nOriHeight=%d-m_nChannelId=%d------###########-\n", \
			m_nOriWidth, m_nOriHeight, m_nChannelId);

		CHANNEL_DETECT_KIND nDetectKind = GetDetectKind();
		m_pDataProcess->InitDspData(m_nChannelId, nDetectKind);
	}

	m_bInit = true;	

	return true;
}

void CDspServer::RecvDspData(int nRecvFd)
{
	char buf[2000000];
	memset(buf, 0, 2000000);
	int nLen = 0;

	struct timeval timeo;
	socklen_t len = sizeof(timeo);
	timeo.tv_sec = 2;
	timeo.tv_usec = 0;//超

	if(setsockopt(nRecvFd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
	{
		perror("setsockopt");
		//DelClient(cfd);
		return;
	}

	while(!g_bEndThread)
	{
		//RecvDspDataFd(nRecvFd, buf, nLen);
		if(nLen > 0)
		{
			printf("----after----------------RecvDspDataFd---nLen=%d--\n", nLen);
		}

		usleep(500*1000);
	}
}

//接收对应Fd内容
bool CDspServer::RecvDspDataFd(const DspSocketFd &dspSocket, char * bufRecv, int &nLen)
{
	if(dspSocket.cfd.nRecvFlag > 1 || !dspSocket.cfd.connectFlag || (dspSocket.cfd.nRecvFlag == 0))
	{
		LogTrace("RecvSize.txt", "fd:%d, flag:%d cnFlag:%d", \
			dspSocket.cfd.fd, dspSocket.cfd.nRecvFlag, dspSocket.cfd.connectFlag);
		return false;
	}


	bool bSendAck = false;
	bool bGetDate = false;
	int	 nFrameSize = 0; //包体数据大小
	bool bRet = false; //接收数据成功标志
	bool bRecvAll = false; //完整包是否接收完全

	bool bRetHead = false;//是否接收包头成功

	bool bRefreshIp = false;//是否已更新ip列表
	bool bGetHeadData = false;//是否已取得包头

	char buffer[1024] = {0};
	int bytes = 0;
	int bytesRecved = 0;

	int nDspHeaderSize = sizeof(Image_header_dsp);
	Image_header_dsp* pDsp_header = NULL; //Dsp数据包头
	int nTypeFlag = 0; //数据体类型标志

	//同步判断
	bool bSynchron1 =  false;
	bool bSynchron2 =  false;
	bool bLinkFlag = false;

	int nTimeCount = 240;//180s 8min(240s) 10min(300s) 15min(450) 60min 300min
	int iTimeCount = 0;
	UINT32 tickPre = GetTimeStamp();
	UINT32 tickIn = tickPre;

	nLen = 0;
	bytesRecved = 0;
	while(dspSocket.cfd.fd != -1 && !bGetDate)
	{
		UINT32 tickNow = GetTimeStamp();
		UINT32 tickDis = tickNow - tickIn;
		if(iTimeCount >= nTimeCount || tickDis > 480)//超时480s退出
		{
			LogNormal("timeout-nRecvFd=%d bGetDate=%d-iTimeCount:%d--tickDis:%d \n", \
				dspSocket.cfd.fd, bGetDate, iTimeCount, tickDis);

			iTimeCount = 0;
			bGetDate = false;

			if(m_pManageClient)
			{			
				char chIP[16] = {0};
				memcpy(chIP,dspSocket.cfd.ip,sizeof(dspSocket.cfd.ip));

				m_pManageClient->DisConnectClient(dspSocket);
				//dspSocket.cfd.fd = -1;		
				if (1 == g_nGongJiaoMode)
				{
					//LogNormal("[%d] chIp =%s,size=%d\n",iCount++,chIP,m_CameraIPMap.size());
					std::string strCameraId = GetCameraIDByIp(chIP);

					if (strCameraId.size() > 0)
					{
						LogNormal("[%s]11:strCameraId=%s\n",__FUNCTION__,strCameraId.c_str());
						g_YiHuaLuCenter.SendCameraStatusOfGJMode(strCameraId,CM_BREAK);
					}					
				}
			}		
			else
			{
				printf("==m_pManageClient NULL=\n");
			}
			
			break;
		}

		memset(buffer,0,1024);
		if(dspSocket.cfd.fd != -1)
		{
			///////////////////////////////////////////
			bSynchron1 =  false;
			bSynchron2 = false;			
			bytes = 0;

			if(!bRetHead)
			{
				//包头
				bRetHead = RecvHeadFd(dspSocket, buffer, bytes, bSynchron1, bSynchron2);
				
				if(!bRetHead)				
				{
					UINT32 tickNow = GetTimeStamp();
					if(tickNow - tickPre >= 2)
					{
						iTimeCount++;
						tickPre = tickNow;
					}
					if(bytes == 0)
					{
						usleep(1000*10);
						continue;
					}			
				}
				else
				{
					if(bSynchron1 || bSynchron2)
					{
						bytesRecved = bytes;
					}
				}
			}			

			if(bSynchron1 || bSynchron2)
			{
				iTimeCount = 0;//归0

				//////////////接收数据///Begin/////////////
				pDsp_header = (Image_header_dsp*)buffer;
				nFrameSize = pDsp_header->nSize;
				nTypeFlag = pDsp_header->nType;
				SetCameraIPAndDeviceID(dspSocket.cfd.ip,pDsp_header->szCameraCode);

				if(DSP_IMG_LINK == nTypeFlag)
				{
					bLinkFlag = true;
				}
				//printf("+++++++++++++++++++++++-------%d %d %d ====\n", pHeader->nType, pHeader->nSeq, pHeader->nSize);
				
				if(!bLinkFlag)
				{
					if(bSynchron1)
					{
						LogTrace("RecvSize.txt","--uSeq:%lld=uType:%d==nFrameSize=%d=fd:%d==ip:%s", \
							pDsp_header->nSeq, nTypeFlag, nFrameSize, dspSocket.cfd.fd, dspSocket.cfd.ip);

						if(!bGetHeadData)
						{
							memcpy(bufRecv, buffer, nDspHeaderSize); //拷贝数据包头							
							bRet = SafeRecvFd(dspSocket, bufRecv + nDspHeaderSize, nFrameSize, bytes);
							bGetHeadData = true;
						}
						else
						{
							bRet = SafeRecvFd(dspSocket, bufRecv + nDspHeaderSize + bytesRecved, nFrameSize - bytesRecved, bytes);
						}						

						if(!bRet)
						{
							LogError("---Recv DSPImg bytes:%d,nFram:%d,strerror(errno)=%s",\
								bytes, nFrameSize, strerror(errno));
							LogTrace("RecvSize.txt","---Recv DSPImg bytes:%d,nFram:%d,errno:%d,strerror(errno)=%s fd:%d ip:%s",\
								bytes, nFrameSize, errno, strerror(errno), dspSocket.cfd.fd, dspSocket.cfd.ip);

							//LogTrace("TimeCount.txt", "iTimeCount=%d --接收DSPImg包体数据错误,strerror(errno)=%s===bytes%d-fd:%d ip:%s", \
							iTimeCount, strerror(errno), bytes, dspSocket.cfd.fd, dspSocket.cfd.ip);

							UINT32 tickNow = GetTimeStamp();
							if(tickNow - tickPre >= 2)
							{
								iTimeCount++;
								tickPre = tickNow;
							}
							if(bytes >= 0)
							{
								bytesRecved += bytes;
								usleep(1000*10);
								continue;
							}
						}
						else
						{
							bRecvAll = true;
						}
					}//End of if(bSynchron1)
					else //if(bSynchron1)
					{
						
						if(bSynchron2)
						{
							LogNormal("bSynchron2=-bytesRecved:%d-uType:%d==nFrameSize=%d=fd:%d==ip:%s\n", \
								bytesRecved, nTypeFlag, nFrameSize, dspSocket.cfd.fd, dspSocket.cfd.ip);

							if(nFrameSize > bytesRecved)
							{
								if(!bGetHeadData)
								{
									memcpy(bufRecv, buffer, nDspHeaderSize + bytesRecved);//拷贝数据包头和已经接收成功的数据
									bRet = SafeRecvFd(dspSocket, bufRecv + nDspHeaderSize + bytesRecved, nFrameSize-bytesRecved, bytes);

									bGetHeadData = true;
								}
								else
								{
									bRet = SafeRecvFd(dspSocket, bufRecv + nDspHeaderSize + bytesRecved, nFrameSize-bytesRecved, bytes);
								}

								if(!bRet)
								{
									printf("bSynchron2 iTimeCount=%d --bytesRecved-=%d-nFrameSize-%d -fd:%d ip:%s", \
										iTimeCount, bytes, nFrameSize, dspSocket.cfd.fd, dspSocket.cfd.ip);

									UINT32 tickNow = GetTimeStamp();
									if(tickNow - tickPre >= 2)
									{
										iTimeCount++;
										tickPre = tickNow;
									}
									if(bytes >= 0)
									{
										bytesRecved += bytes;
										usleep(1000*10);
										continue;
									}											
								}
								else
								{
									bRecvAll = true;
								}
							}
							else
							{
								if(!bGetHeadData)
								{							
									memcpy(bufRecv, buffer, nDspHeaderSize + nFrameSize); //拷贝数据包头和已经接收成功的数据
									bGetHeadData = true;
								}
								else
								{
									memcpy(bufRecv + nDspHeaderSize, buffer, nFrameSize); //拷贝数据包
								}

								bRecvAll = true;
							}
						}//End of if(bSynchron2)
						
					}//End of else if(bSynchron1)
				 }//End of if(!bLinkFlag)

				 break;
			}//End of if(bSynchron1 || bSynchron2) 
			else
			{
				bGetDate = false;
			}
		} // End of if
		else
		{
			bGetDate = false;
			//nLen = nDspHeaderSize + nFrameSize;
			LogTrace("RecvSize.txt","---eRRRR---111---bSynchron1-%d-bSynchron2-%d-- \n", bSynchron1, bSynchron2);
		}

		usleep(1000*2);
	} //End of while
	
	if(-1 == dspSocket.cfd.fd)//sock 无效立即返回
	{
		LogTrace("RecvSize.txt","---eRRRR------bSynchron1-%d-bSynchron2-%d-- \n", bSynchron1, bSynchron2);
		nLen = 0;
		return false;
	}

	//处理ACK回复
	if(bRecvAll || bLinkFlag)
	{
		bSendAck  = DealDspAck(dspSocket, pDsp_header, true);

		if(bLinkFlag)
		{
			bGetDate = false;
			nLen = 0;
		}
		else if(bRecvAll)
		{
			bGetDate = true;
			nLen = nDspHeaderSize + nFrameSize;	
		}
		else{}
	}//end of if(bRecvAll || bLinkFlag)
	else
	{
		if(!bGetDate && bytesRecved > 0)
		{
			LogTrace("RecvSize.txt", "--bGetDate:%d bytesRecved:%d \n", bGetDate, bytesRecved);
			bytesRecved = 0;
			nLen = 0;				
			bSendAck = DealDspAck(dspSocket, pDsp_header, false);
		}
	}

	if(bGetDate || bLinkFlag)
	{
		Image_header_dsp* pHeader = (Image_header_dsp*)bufRecv;

		//camid 取2字节
		UINT32 id2 = pHeader->uCameraId & 0x0000FFFF;
		pHeader->uCameraId = id2;

#ifndef DSP_SERVER_TEST
		//更新列表
		if(m_pDspDealVideoTcp && !bRefreshIp)
		{
			std::string strIp = dspSocket.cfd.ip;
			m_pDspDealVideoTcp->RefreshIpMap(pHeader->uCameraId, dspSocket.cfd.fd, strIp);					
			bRefreshIp = true;
		}
#endif

		if(pHeader->nType == DSP_IMG_PLATE_INFO || pHeader->nType == DSP_IMG_EVENT_INFO)
		{
			RECORD_PLATE_DSP * pPlate = (RECORD_PLATE_DSP*)(bufRecv + sizeof(Image_header_dsp));
			pPlate->uCameraId = id2;
		}
	}
	
//存图测试
#ifdef ROSEEK_SAVE_INFO
	if(bGetDate)
	{
		Image_header_dsp* pHeader = (Image_header_dsp*)bufRecv;
		if(pHeader != NULL)
		{
			int HeaderLength = sizeof(Image_header_dsp);
			FILE *fpOut = NULL;
			char jpg_name[256] = {0};

			UINT32 uTs = (pHeader->ts/1000)/1000;
			UINT32 uTsSys = GetTimeStamp();

			if(fpOut)
			{
				//head
				{
					sprintf(jpg_name, "./text/%d_seq_%d_%d_%d_%d.head", pHeader->nSeq, pHeader->nType, uTs, uTsSys, pHeader->uCameraId);
					fpOut = fopen(jpg_name, "wb");
					fwrite(bufRecv, 1, HeaderLength, fpOut);
					fclose(fpOut);
				}

				//jpg
				if(pHeader->nType == 1)
				{
					sprintf(jpg_name, "./text/%d_seq_%d_%d_%d.jpg", pHeader->nSeq, pHeader->nType, uTs, pHeader->uCameraId);
					fpOut = fopen(jpg_name, "wb");
					fwrite(bufRecv + HeaderLength, 1, pHeader->nSize - 4, fpOut);
					fclose(fpOut);
				}

				//palte
				if(pHeader->nType > 1)
				{
					sprintf(jpg_name, "./text/%d_seq_%d_%d_%d_%d.data", pHeader->nSeq, pHeader->nType, uTs, uTsSys, pHeader->uCameraId);
					fpOut = fopen(jpg_name, "wb");
					fwrite(bufRecv + HeaderLength, 1, pHeader->nSize - 4, fpOut);
					fclose(fpOut);
				}
			}			
		}
	}
#endif

	return bGetDate;
}

//设置套接字并开启接收线程
bool CDspServer::SetRecFd(const DspSocketFd dspSocket)
{
	//设置为非阻塞模式
	//AddClient(classfd.cfd);
	LogNormal("---Add Client--fd-=%d--######\n", dspSocket.cfd.fd);

	if(m_pManageClient == NULL)
	{
		printf("----m_pManageClient NULL...-------\n");
		return false;
	}

	m_pManageClient->AddDspClient(dspSocket);	

	return true;
}

bool CDspServer::Open()
{
	m_bEndCapture = false;

	if(m_bInit)
	{		
		InitDspServerSocket();
		return true;
	}

	//启动监听服务线程
	bool bStart = InitDspServer();
	LogNormal("----bStart=%d----\n", bStart);

	if(bStart == false)
	{
		LogError("启动MvsDspServer服务失败\r\n");
		return false;
	}
	else
	{
		m_bOpenCamera = true;			
	}

    return true;
}

bool CDspServer::Close()
{
	m_bEndCapture = true;

	LogNormal("#### before close=m_nSerTcpFd=%d=\n", m_nSerTcpFd);
	
	UnInitDspServerSocket();

	if(m_pManageClient != NULL)
	{
		LogNormal("=##== m_pManageClient=\n");
		m_pManageClient->CleanMap();
	}

	return true;
}

//接收数据包
bool CDspServer::SafeRecvFd(const DspSocketFd &dspSocket, char*pBuf, int nLen, int &nRecvSize)
{	
	//printf("-111-SafeRecvFd-nLen:%d,nRecvSize:%d \n", nLen, nRecvSize);
	LogTrace("SafeRecvFd.txt", "-111-SafeRecvFd-nLen:%d,nRecvSize:%d fd:%d, ip:%s,  nTimeFlag:%d nRecvFlag:%d \n", \
		nLen, nRecvSize,dspSocket.cfd.fd, dspSocket.cfd.ip, dspSocket.cfd.nTimeFlag, dspSocket.cfd.nRecvFlag);

	int nRecveFd = dspSocket.cfd.fd;

	if(nRecveFd <= 0 || !dspSocket.cfd.connectFlag || (dspSocket.cfd.nRecvFlag == 0))
	{
		return false;
	}

	int nBytes = 0;
	int nRecved = 0;
	bool bRet = true;

	int iCounts = 0;
	int nCountElem = 0;
	bool bBreakFlag = false;
	int nSleep = 1000;
	int nSockFlag = WaitForSockCanRead(nRecveFd, nSleep);
	int nLeft = nLen;

	
	if(nLen < 100)
	{
		nCountElem = 3;
	}
	else if(nLen < 1000)
	{
		nCountElem = 10;
	}
	else if(nLen < MAX_DSP_BUF_LEN)
	{
		nCountElem = 300;
	}
	else
	{
		return false;
	}

	if(nSockFlag > 0)
	{
		int nRecvCounts = 0;
		int nRecvLast = 0;
		while (nRecved < nLen && nRecveFd > 0)
		{
			if(iCounts >= nCountElem)
			{
				usleep(nSleep*1000);
				break;
			}

			nBytes = recv(nRecveFd, pBuf + nRecved, 1024>nLeft?nLeft:1024, MSG_WAITALL);

			if(nBytes > 0)
			{
				if(1024 == nBytes)
				{
					nRecvCounts ++;
				}
				else
				{
					nRecvLast = nBytes;
				}
			}
			

			if (nBytes <= 0)
			{
				bRet = false;
				iCounts++;

				if(nBytes < 0)
				{
					if(EAGAIN == errno || EWOULDBLOCK == errno)
					{			
						LogTrace("RecvSize.txt", "22 tsIn:%d nBytes:%d nRecveFd =%d receive data2 nLen=%d,nRecved=%d,errno=%d,strerror=%s,iCounts:%d ",\
							 dspSocket.cfd.nTimeFlag, nBytes, nRecveFd, nLen,nRecved,errno,strerror(errno), iCounts);

						usleep(20*1000);//20ms (400s)
						continue;
					}
					else
					{
						LogTrace("RecvSize.txt", "44 nBytes:%d nRecveFd =%d receive data2 nLen=%d,nRecved=%d,errno=%d,strerror=%s,iCounts:%d ",\
							nBytes, nRecveFd, nLen,nRecved,errno,strerror(errno), iCounts);

						usleep(nSleep*1000);
						break;
					}
				}
				else
				{
						//LogTrace("TimeCount.txt", "44 nSockFlag = %d  fd:%d sleep:%d ms\n",\
							nSockFlag, nRecveFd, nSleep);
				}

				usleep(nSleep*1000);
				LogTrace("RecvSize.txt", "11 nBytes:%d nRecveFd =%d receive data2 nLen=%d,nRecved=%d,strerror=%s,errno=%d, iCounts:%d nCountElem:%d",\
					nBytes, nRecveFd, nLen,nRecved,strerror(errno),errno, iCounts, nCountElem);
			}
			nRecved += nBytes;
			nLeft  -= nBytes; 
		}

		if(nRecved != nLen)
		{
			LogNormal("fd:%d nRecvCounts:%d ,nRecvLast:%d", nRecveFd, nRecvCounts, nRecvLast);
			LogTrace("RecvSize.txt", "fd:%d nRecvCounts:%d ,nRecvLast:%d", nRecveFd, nRecvCounts, nRecvLast);
		}
		
	}//End if(nSockFlag > 0)
	else if(nSockFlag == 0)
	{		
		//LogTrace("TimeCount.txt", "11 nSockFlag = %d fd:%d\n",\
			nSockFlag, nRecveFd);		
	}
	else // <0
	{	
		LogTrace("TimeCount.txt", "22 nSockFlag = %d fd:%d errno=%d,strerror=%s \n",\
			nSockFlag, nRecveFd, errno,strerror(errno));			
	}	

	if (nRecved < nLen)
	{
		if(nSockFlag > 0)
		{
			LogTrace("RecvSize.txt", "nBytes:%d nRecved:%d nLen:%d fd:%d ip:%s iCounts:%d nCountElem:%d", \
				nBytes, nRecved, nLen, dspSocket.cfd.fd, dspSocket.cfd.ip, iCounts, nCountElem);
		}		

		bRet = false;
	}
	else if(nRecved == nLen)
	{
		if(!bRet)
		{
			LogTrace("RecvSize.txt", "33 ok nBytes:%d nRecveFd =%d receive data2 nLen=%d,nRecved=%d,strerror=%s,errno=%d, iCounts:%d ",\
				nBytes, nRecveFd, nLen,nRecved,strerror(errno),errno, iCounts);
			bRet = true;
		}
	}

	nRecvSize = nRecved;

	printf("--SafeRecvFd-nLen:%d,nRecvSize:%d \n", nLen, nRecvSize);

	LogTrace("SafeRecvFd.txt", "-222-SafeRecvFd-nCountElem:%d nLen:%d,nRecvSize:%d fd:%d, ip:%s, nTimeFlag:%d \n", \
		nCountElem, nLen, nRecvSize,dspSocket.cfd.fd, dspSocket.cfd.ip, dspSocket.cfd.nTimeFlag);

	return bRet;
}


bool CDspServer::CloseFd(int &nSock)
{
	if(nSock != -1)
	{
		LogTrace("RecvSize.txt","-CloseFd--nSock=%d\n", nSock);
		LogNormal("-CloseFd--nSock=%d\n", nSock);
		shutdown(nSock,2);
		close(nSock);
		nSock = -1;
	}
	return true;
}

//接收DSP客户端连接
void CDspServer::MvsDspAccept()
{
	int nClient = 0;
	int nCount = 0;

	while(!m_bEndCapture)
	{
		LogNormal("Accp 11 nCount=%d nClient:%d \n", nCount, nClient);
		//中心端连接
		struct sockaddr_in clientaddr;
		memset(&clientaddr,0,sizeof(clientaddr));
		//长度
		socklen_t sin_size = sizeof(struct   sockaddr_in);

		//接受连接		
		//LogNormal("端口:%d\n",g_DspServerHostInfo.uDspServerPort);
		if( ( nClient = accept(GetMvsDspSockFd(),(struct sockaddr*)&clientaddr, &sin_size) ) ==  -1 )
		{
			nCount++;
			LogNormal("Accp 22 er nCount=%d nClient:%d \n", nCount, nClient);

			//自动重启
			continue;
		}

		if(nClient > 0)
		{			
			//输出用户连接		
			LogNormal("Accpt dsp [IP:%s][nClient = %d,端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));
			LogNormal("侦听套接字=%d\n",GetMvsDspSockFd());

			LogTrace("RecvSize.txt", "Accpt dsp [IP:%s][nClient = %d,端口:%d]!\r\n",\
				inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));


			nCount = 0;
			DspSocketFd dspSocket;
			dspSocket.p = this;
			dspSocket.pProcess = this->GetPDataProcess();

			memcpy(dspSocket.cfd.ip, inet_ntoa(clientaddr.sin_addr), 16);
			dspSocket.cfd.port = ntohs(clientaddr.sin_port);
			dspSocket.cfd.fd = nClient;
			dspSocket.cfd.connectFlag = true;
			this->SetRecFd(dspSocket);
/*
			//开启h264录像
	#ifdef DSP_RTSP_RECORD
			DspVideoClient_Fd dspVideocfd;
		
			memcpy(dspVideocfd.szIp, inet_ntoa(clientaddr.sin_addr), 16);
			dspVideocfd.nPort = 8557;
			dspVideocfd.nConnType = 1;//SDK_ID_RTSP_CLIENT
			sprintf(dspVideocfd.szUrl, "rtsp://%s:%d/h264", dspVideocfd.szIp, dspVideocfd.nPort);
			this->StartRecord(dspVideocfd);
	#endif
*/
		}

		nCount++;
		LogNormal("Accp 66 ok nCount=%d nClient:%d \n", nCount, nClient);

		usleep(1*1000);
	}
}


//监控客户端录像
void CDspServer::MvsDspRecordAccept()
{
	//TCP收数据
	m_pDspDealVideoTcp = new CDspDealVideoTcp();
	bool bInitDealVideo = false;
	bool bDealVideo = false;

	while(!m_bEndCapture)
	{
		if(!bDealVideo)
		{
			if(m_pDspDealVideoTcp)
			{
#ifndef TEMP_VIDEO_ON
				//printf("-----11-----m_pDspDealVideoTcp->InitTcp--\n");
				bInitDealVideo =  m_pDspDealVideoTcp->InitTcp(m_bH264Capture, m_bSkpRecorder, m_pH264Capture, m_pSkpRecorder);
				if(bInitDealVideo)
				{
					bDealVideo = m_pDspDealVideoTcp->StartServer();
				}
#endif
			}
		}		
		
		sleep(1);
	}		

	if(m_pDspDealVideoTcp)
	{
		delete m_pDspDealVideoTcp;
		LogNormal("---delete m_pDspDealVideoTcp---\n");
		m_pDspDealVideoTcp = NULL;
	}		
}

//初始化侦听套接字，创建接收线程
bool CDspServer::InitDspServerSocket()
{
	//创建套接字
	if(m_nSocketBase->mvCreateSocket(m_nSerTcpFd,1)==false)
	{
		LogError("创建套接字失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	LogNormal("in InitDspServer() 侦听套接字=%d\n",m_nSerTcpFd);

	//重复使用套接字
	if(m_nSocketBase->mvSetSocketOpt(m_nSerTcpFd,SO_REUSEADDR)==false)
	{
		LogError("设置重复使用套接字失败,服务无法启动!,%s\r\n",strerror(errno));
		g_bEndThread = true;
		return false;
	}

	//绑定服务端口
	g_DspServerHostInfo.uDspServerPort = MVS_SERVER_TCP_PORT;
	if(m_nSocketBase->mvBindPort(m_nSerTcpFd, g_DspServerHostInfo.uDspServerPort)==false)
	{
		LogError("绑定到 %d 端口失败,服务无法启动!\r\n",m_nPort);
		printf("%s\n",strerror(errno));
		g_bEndThread = true;
		return false;
	}

	//开始监听
	if (m_nSocketBase->mvStartListen(m_nSerTcpFd) == false)
	{
		LogError("监听连接失败，服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//启动接收连接线程
	//线程id
	pthread_t id1;
	pthread_t id2;

	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动事件监控线程
	int nret=pthread_create(&id1,&attr,ThreadMvsDspAccept,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建事件监控线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	LogNormal("------********---cr---thread---ThreadMvsDspAccept---***************---\n");

	
	pthread_attr_t   attr2;
	//初始化
	pthread_attr_init(&attr2);
	//分离线程
	pthread_attr_setdetachstate(&attr2,PTHREAD_CREATE_DETACHED);

	//启动事件监控线程
	int nret2=pthread_create(&id2,&attr2,ThreadMvsDspRecordAccept,this);
	//成功
	if(nret2!=0)
	{
		//失败
		LogError("创建事件监控录像线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	LogNormal("------********---cr---thread---ThreadMvsDspRecordAccept---***************---\n");


	LogNormal("Thrad Destroy id:%lld CDspServer 11", (int)id1);

	pthread_attr_destroy(&attr);

	LogNormal("Thrad Destroy id:%lld CDspServer 22", (int)id2);

	pthread_attr_destroy(&attr2);

	return true;
}

//销毁侦听套接字，停止接收线程
void CDspServer::UnInitDspServerSocket()
{
	if(m_nSocketBase != NULL)
	{
		delete m_nSocketBase;
		m_nSocketBase = NULL;
	}

	if(m_nSerTcpFd!=-1)
	{
		shutdown(m_nSerTcpFd,2);
		close(m_nSerTcpFd);
		m_nSerTcpFd = -1;
	}
}

//发送DSP回复心跳包
bool CDspServer::SendToDspLinkFd(DspSocketFd &dspSocket)
{
	bool bRet = false;
	if(dspSocket.cfd.fd < 0)
	{
		usleep(5 * 1000);
		return false;
	}

	int nSendFailCount = 0;
	int nCount = 3;
	//int nSizeHeader = sizeof(Image_header_dsp);
	DSP_ACK ack;
	
	char buf[4] = {'$', '$', '$', '$'};
	memcpy(ack.cSynchron, buf, 4);
	ack.nType = DSP_IMG_LINK;
	ack.uSeq = 0;
	//char buf[4] = {0x01, 0x02, 0x03, 0x04};
	while(nCount > 0)
	{
		int nBytes = send(dspSocket.cfd.fd, (char*)&ack, sizeof(DSP_ACK), MSG_NOSIGNAL);
		char bufAck[18] = {0};
		memcpy(bufAck, (char*)&ack, sizeof(ack));

		LogTrace("SendAck.log", "[%x,%x,%x,%x, %x,%x,%x,%x,%x,%x,%x,%x, %x,%x] ip:%s fd:%d", \
			bufAck[0], bufAck[1], bufAck[2], bufAck[3],\
			//bufAck[4], bufAck[5], bufAck[6], bufAck[7],
			bufAck[8], bufAck[9], bufAck[10], bufAck[11],
			bufAck[12], bufAck[13],bufAck[14], bufAck[15],
			bufAck[16], bufAck[17], dspSocket.cfd.ip, dspSocket.cfd.fd);

		if(nBytes != sizeof(DSP_ACK))
		{
			LogError("发送心跳包出错,Sock=[%d]ip:[%s]\n", dspSocket.cfd.fd, dspSocket.cfd.ip);
			nSendFailCount++;

			if(nSendFailCount > 2)
			{
				bRet = false;
				LogNormal("-er SendToDspLinkFd-bRet=%d \n", bRet);
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

	LogNormal("-SendToDspLinkFd-bRet=%d \n", bRet);

	return bRet;
}

//开起录像
bool CDspServer::StartRecord(DspVideoClient_Fd &dspVideocfd)
{
#ifdef DIO_RTSP_RECV

	//录像处理类
	m_pDspDealVideo = new CDspDealVideo;

	/*
	DspVideoClient_Fd dspVideocfd;
	memcpy(dspVideocfd.szIp, m_strCameraIP.c_str(), 16);
	dspVideocfd.nPort = 8557;
	dspVideocfd.nConnType = 1;//SDK_ID_RTSP_CLIENT
	sprintf(dspVideocfd.szUrl, "rtsp://%s:%d/h264", dspVideocfd.szIp, dspVideocfd.nPort);
	*/

	bool bDealVideo = false;	

	while(!m_bEndCapture)
	{
		if(m_pDspDealVideo != NULL)
		{
			if(!bDealVideo)
			{
				m_pDspDealVideo->Init(m_nChannelId);

#ifndef TEMP_VIDEO_ON
				printf("---BEFORE------------>AddRtspcfd----dspVideocfd.szUrl=%s \n", dspVideocfd.szUrl);
				bDealVideo = m_pDspDealVideo->AddRtspcfd(dspVideocfd, m_bH264Capture, m_bSkpRecorder, m_pH264Capture, m_pSkpRecorder);
				printf("--END------------->AddRtspcfd----\n");
#endif
			}
		}
		sleep(2);
	}

	m_pDspDealVideo->UnInitUnit(dspVideocfd);
	printf("---m_pDspDealVideo->UnInitUnit--\n");

	if(m_pDspDealVideo != NULL)
	{
		delete m_pDspDealVideo;
		m_pDspDealVideo = NULL;
		printf("--delete m_pDspDealVideo--\n");
	}
	
	return true;
#endif
}

//给特定套接字发送数据包
bool CDspServer::SafeSendFd(int nFd,const char* pBuf, int nLen)
{
	int nRet = 0;
	int nSended = 0;
	while (nSended < nLen)
	{
		nRet = send(nFd, pBuf + nSended, nLen - nSended, MSG_NOSIGNAL);
		if (nRet <= 0)
		{
			break;
		}
		nSended += nRet;
	}
	if (nSended < nLen)
	{
		return false;
	}
	return true;
}

//重启服务端
bool CDspServer::ReOpen()
{
	return true;

	LogNormal("-111-CDspServer::ReOpen-\n");
	this->Close();
	LogNormal("-222-CDspServer::ReOpen-\n");

	//软件复位
	//throw 0;
	g_bEndThread = true;
	sleep(3);
	LogNormal("22 软件复位成功\n");
	//LogNormal("应用软件故障");
	exit(-1);

	//this->Open();
	LogNormal("-333-CDspServer::ReOpen-\n");

	return true;	
}


//发送检测器时间给DSP
bool CDspServer::SendToDspTimeFd(DspSocketFd &dspSocket)
{
	bool bRet = false;
	if(dspSocket.cfd.fd < 0)
	{
		usleep(5 * 1000);
		return false;
	}

	int nSendFailCount = 0;
	int nCount = 3;
	//int nSizeHeader = sizeof(Image_header_dsp);
	DSP_ACK ack;

	char buf[4] = {'$', '$', '$', '$'};
	memcpy(ack.cSynchron, buf, 4);
	ack.nType = DSP_IMG_DSPTIME;
	ack.uSeq = GetTimeStamp();//填充SysTime
	
	LogNormal("To Send SysTime:%d \n", ack.uSeq);
	//char buf[4] = {0x01, 0x02, 0x03, 0x04};
	while(nCount > 0)
	{
		int nBytes = send(dspSocket.cfd.fd, (char*)&ack, sizeof(DSP_ACK), MSG_NOSIGNAL);
		char bufAck[18] = {0};
		memcpy(bufAck, (char*)&ack, sizeof(ack));
		if(nBytes != sizeof(DSP_ACK))
		{
			LogError("发送SysTime包出错,Sock=[%d]ip:[%s]\n", dspSocket.cfd.fd, dspSocket.cfd.ip);
			nSendFailCount++;

			if(nSendFailCount > 2)
			{
				bRet = false;
				LogNormal("-er SendToDspTimeFd-bRet=%d \n", bRet);
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

	LogNormal("-SendToDspTimeFd-bRet=%d \n", bRet);

	return bRet;
}

//等待接收缓冲有数据可读
int CDspServer::WaitForSockCanRead(int sock, int nTimeOut /* = 1000 */)
{
	int nWaitSock = 0;
	int maxfd;
	fd_set fsVal;
	FD_ZERO(&fsVal);

	if( sock > 0)
		FD_CLR(sock, &fsVal);

	FD_SET(sock, &fsVal);
	struct timeval tvTimeOut;
	tvTimeOut.tv_sec = nTimeOut/1000;
	tvTimeOut.tv_usec = (nTimeOut%1000)*1000;

	maxfd = (sock > 0) ? (sock+1) : 1;

	int nRet = select(maxfd, &fsVal, NULL, NULL, &tvTimeOut);

	bool bFdSet = FD_ISSET(sock, &fsVal);

	if(nRet < 0)
	{
		if((errno == EINTR) && nRet == -1)
		{
			LogNormal("11=errno=%d=[%s]\n", errno, strerror(errno));
			nWaitSock = -1;

			//LogTrace("TimeCount.txt", "nWaitSock:%d nTimeOut:%d fd:%d sleep :%d ms", nWaitSock, nTimeOut, sock, nTimeOut);
			usleep(nTimeOut*1000);
		}
	}
	else
	{
		if(nRet > 0 && bFdSet)
		{
			nWaitSock = 1;
			//LogTrace("TimeCount.txt", "nWaitSock:%d nTimeOut:%d fd:%d no sleep", nWaitSock, nTimeOut, sock);
		}
		else
		{
			nWaitSock = 0;

			//LogTrace("TimeCount.txt", "nWaitSock:%d nTimeOut:%d fd:%d sleep :%d ms", nWaitSock, nTimeOut, sock, nTimeOut);			
			usleep(nTimeOut*1000);
		}
	}

	return nWaitSock;
}

//接收包头
bool CDspServer::RecvHeadFd(const DspSocketFd &dspSocket, char * buffer, int &bytesRecved, bool &bSynchron1, bool &bSynchron2)
{
	if(dspSocket.cfd.nRecvFlag == 0)
	{
		LogTrace("RecvSize.txt", "error! RecvHeadFd fd:%d, flag:%d ", \
			dspSocket.cfd.fd, dspSocket.cfd.nRecvFlag);
		return false;
	}

	//LogTrace("RecvSize.txt", "RecvHeadFd fd:%d, flag:%d ", dspSocket.cfd.fd, dspSocket.cfd.nRecvFlag);
	
	bool bRet = false;

	//LogTrace("RecvSize.txt", "--------------------------in--RecvHeadFd--- bytesRecved:%d ", bytesRecved);

	int nDspHeaderSize = sizeof(Image_header_dsp);
	char buff[1024] = {0};
	int nRecevCell = 256;//512 //256
	int bytes = 0;
	int bytesRecveHead = 0;
	int i=0;
	int j=0;
	int iFindFlag = 0; //标记开始找到$的位置
	int index = 0; //标记同步头$$$$后的位置
	int indexContent = 0; //标记去头后紧跟着的具体数据的位置
	int nFlags = 0;//连续出现$的个数

	bool bRet1 = false;
	bool bRet2 = false;
	bool bRet3 = false;

	//接收码流头
	bRet1 = SafeRecvFd(dspSocket, buffer, nDspHeaderSize, bytes);

	//接收码流头
	if(bRet1)
	{
		if(memcmp(buffer, "$$$$", 4) == 0)
		{
			bSynchron1 = true; //同步上
		}
		else //继续接收数据
		{
			memset(buff,0,1024);

			memcpy(buff, buffer, nDspHeaderSize);
			bRet2 = SafeRecvFd(dspSocket, buff + nDspHeaderSize, nRecevCell, bytes);
			
			/*
			if(!bRet2)
			{
				printf("+++++++++++++++++++ 1111 bytes:%d \n", bytes);
				//return false;
			}
			else //333
			*/
			{
				index = 0; //index [0, 1024]
				//////////////////////////寻找同步头/////////////////////////
				if(bytes >= nDspHeaderSize)
				{					
					bytesRecveHead = bytes + nDspHeaderSize;

					printf("[1]--bytesRecveHead:%d--m_bEndCapture:%d \n", bytesRecveHead, m_bEndCapture);

					LogTrace("RecvHead.txt", "\n [1]--bytesRecveHead:%d-- \n", bytesRecveHead);
					/*
					//print buff
					for(int i=0; i<bytesRecveHead; i++)
					{
						LogTrace("RecvHead.txt", "(%d)  %x \t char: %c", i, buff[i], buff[i]);
					}
					*/

					i=0;
					//寻找同步头$$$$
					while(i<bytesRecveHead)
					{
						if(buff[i] == '$')
						{
							iFindFlag = i;

							printf("[2]--i:%d--\n", i);
							//printf("[2]--buff :%x,%x,%x,%x,%x--\n", buff[i], buff[i+1], buff[i+2], buff[i+3],buff[i+4]);

							if( (buff[i+1] == '$') && (buff[i+2] == '$') && (buff[i+3] == '$') && (buff[i+4] != '$'))
							{
								index = iFindFlag + 4;								

								printf("[3]--i:%d--\n", i);
								if( (iFindFlag + nDspHeaderSize) > bytesRecveHead )
								{
									int nToRecSize = (iFindFlag + nDspHeaderSize) - bytesRecveHead;

									printf("[4]--i:%d--nToRecSize:%d \n", i, nToRecSize);
									//接着接收nToRecSize大小数据
									bRet3 = SafeRecvFd(dspSocket, buff+bytesRecveHead, nToRecSize, bytes);
									if(!bRet3)
									{
										printf("[5]-\n nToRecSize:%d bytes:%d \n", \
											nToRecSize, bytes);
										break;
									}
									else
									{
										bytesRecveHead += bytes;
										//LogNormal("===Recv append header bytes=%d, bytesRecveHead=%d==\n", bytes, bytesRecveHead);
									}
								}

								//LogTrace("Head2.txt","\n++++++++--i:%d,j:%d,index:%d,iFindFlag:%d, nFlags:%d,bytes \n", \
									i, j, index, iFindFlag, nFlags, bytes);

								printf("[6] \n++++++++--i:%d,j:%d,index:%d,iFindFlag:%d, nFlags:%d,bytes \n", \
									i, j, index, iFindFlag, nFlags, bytes);
								LogNormal("[6]i:%d,j:%d,index:%d,iFindFlag:%d,nFlags:%d,bytes \n", \
									i, j, index, iFindFlag, nFlags, bytes);

								bSynchron2 = true;			
								break;
							}//End of if( (buff[i+1] == '$') && (buff[i+2] == '$') && (buff[i+3] == '$') )
						}//End of if(buff[i] == '$')

						i++;
					}//End of while(i<bytes)

					if(bSynchron2)
					{
						memset(buffer, 0, sizeof(buffer));
						memcpy(buffer, buff + iFindFlag, bytesRecveHead);
						bytesRecved = bytesRecveHead - iFindFlag - nDspHeaderSize;

						if(bytesRecved < 0)
						{
							bSynchron2 = false;
							bytesRecved = 0;
						}

						//LogNormal("bSynchron2:%d, bytesRecved:%d \n", bSynchron2, bytesRecved);
					}//End of if(bSynchron2)
				}//End of if(bytes >= nDspHeaderSize)
				//////////////////////////寻找同步头/////////////////////////
			}//End of else //333
		}//End of //继续接收数据
	} //End of else //接收码流头

	//验证包头是否合法
	if(bSynchron1 || bSynchron2)
	{
		Image_header_dsp* pDsp_header = (Image_header_dsp*)buffer; //Dsp数据包头
		if(pDsp_header != NULL)
		{
			if(pDsp_header->nType > 0 && pDsp_header->nType < 100 \
				&& pDsp_header->nSize >= 0 && pDsp_header->nSize < MAX_DSP_BUF_LEN)
			{
				bRet = true;				
			}
			else
			{
				LogTrace("RecvErr.txt", "Recv data err! nTypeFlag:%d nFrameSize:%d fd:%d, ip:%d \n", \
					pDsp_header->nType, pDsp_header->nSize, dspSocket.cfd.fd, dspSocket.cfd.ip);
				bRet = false;
			}
			LogTrace("RecvSize.txt", "--bRet:%d--bytesRecved:%d bytes:%d--uSeq:%lld, type:%d nSize:%lld", \
				bRet, bytesRecved, bytes, pDsp_header->nSeq, pDsp_header->nType, pDsp_header->nSize);
		}		
	}
	else
	{
		if(bytes > 0)
		{
			LogTrace("RecvSize.txt", "-----bRet:%d--out--RecvHeadFd---bytesRecved:%d bytes:%d", bRet, bytesRecved, bytes);
		}
	}

	return bRet;
}

//处理dsp相机回复
bool CDspServer::DealDspAck(const DspSocketFd &dspSocket, const Image_header_dsp* pDsp_header, const bool &bAckFlag)
{
	bool bSendAck = false;

	DSP_ACK nDsp_Ack;
	char bufAck[18] = {0};
	memcpy(bufAck, (char*)&nDsp_Ack, sizeof(nDsp_Ack));

	if(bAckFlag)
	{
		//回复ACK
		char szAckHead[4] = {'$', '$', '$', '$'};
		memcpy(nDsp_Ack.cSynchron, szAckHead, 4);
		nDsp_Ack.uSeq = pDsp_header->nSeq;
		nDsp_Ack.nType = pDsp_header->nType;

		LogTrace("RecvData.txt", "uType:%d uSeq:%lld fd:%d,ip:%s", nDsp_Ack.nType, nDsp_Ack.uSeq, dspSocket.cfd.fd, dspSocket.cfd.ip);

		bSendAck = SafeSendFd(dspSocket.cfd.fd,(const char *)&nDsp_Ack,sizeof(nDsp_Ack));
		memcpy(bufAck, (char*)&nDsp_Ack, sizeof(nDsp_Ack));
		if(bSendAck)
		{
			LogTrace("SendAck.log","SendAck %d ok. fd:%d, size:%d ", nDsp_Ack.nType, dspSocket.cfd.fd, pDsp_header->nSize);
		}
		else
		{
			LogNormal("SendAck 1 err.");
		}
	}
	else
	{
		//回复ACK,接收到错误数据包		
		char szAckHead[4] = {'$', '$', '$', '$'};
		memcpy(nDsp_Ack.cSynchron, szAckHead, 4);
		nDsp_Ack.uSeq = 0;
		nDsp_Ack.nType = DSP_ERROR;

		bSendAck = SafeSendFd(dspSocket.cfd.fd,(const char *)&nDsp_Ack,sizeof(nDsp_Ack));
		memcpy(bufAck, (char*)&nDsp_Ack, sizeof(nDsp_Ack));

		if(bSendAck)
		{
			LogNormal("SendAck 99 ok. nSeq:%d \n", pDsp_header->nType);
		}
		else
		{
			LogNormal("SendAck 99 err.");
		}
	}

	LogTrace("SendAck.log", "[%x,%x,%x,%x, %x,%x,%x,%x,%x,%x,%x,%x, %x,%x] ip:%s fd:%d seq:%d size:%d", \
		bufAck[0], bufAck[1], bufAck[2], bufAck[3],\
		//bufAck[4], bufAck[5], bufAck[6], bufAck[7],
		bufAck[8], bufAck[9], bufAck[10], bufAck[11],
		bufAck[12], bufAck[13],bufAck[14], bufAck[15],
		bufAck[16], bufAck[17], dspSocket.cfd.ip, dspSocket.cfd.fd, pDsp_header->nSeq, pDsp_header->nSize);

	return bSendAck;
}

std::string CDspServer::GetCameraIDByIp(const char* szAddress)
{
	std::string strRet("");
	if (m_CameraIPMap.size() > 0)
	{
		map<string,string>::iterator iter = m_CameraIPMap.find(szAddress);
		if (iter != m_CameraIPMap.end())
		{
			strRet.append(iter->second);
		}
	}
		
	return strRet;
}

void CDspServer::SetCameraIPAndDeviceID(const char* szAddress,const char *szCameraID)
{
	m_CameraIPMap.insert(make_pair(szAddress,szCameraID)); 
	printf("ip = %s,id = %s\n",szAddress,szCameraID);
}

#endif