// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2013 上海博康智能信息技术有限公司
// Copyright 2008-2013 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//



#include "AbstractCamera.h"
#include "Common.h"
#include "CommonHeader.h"

#include "CSocketBase.h"
#include "DspDealVideoTcp.h"
#include "CenterServerOneDotEight.h"

//接收公交h264码流线程
void* ThreadDspVideoTcpAccept(void* pArg)
{
	//取类指针
	CDspDealVideoTcp* pH264Accept = (CDspDealVideoTcp*)pArg;
	if(pH264Accept == NULL)
		return pArg;

	pH264Accept->h264Accept();
	pthread_exit((void *)0);
	return pArg;
}


//监测接收码流是否工作正常线程
void* ThreadDspVideoLinkStatus(void* pArg)
{
	//取类指针
	CDspDealVideoTcp* pDspDealVideoTcp = (CDspDealVideoTcp*)pArg;
	if(pDspDealVideoTcp == NULL)
		return pArg;

	pDspDealVideoTcp->DetectStatus();
	pthread_exit((void *)0);
	return pArg;
}

CDspDealVideoTcp::CDspDealVideoTcp()
{
	m_nH264SocketBase = new mvCSocketBase;
	m_bEndCapture = false;

	m_MapIdIp.clear();
	pthread_mutex_init(&m_mapIdIp_mutex, NULL);

	m_MapDspClient.clear();
	pthread_mutex_init(&m_dspClientMapLock, NULL);

	m_nThreadId7 = 0;
	m_nThreadStatusId = 0;
}

CDspDealVideoTcp::~CDspDealVideoTcp()
{
	pthread_mutex_destroy(&m_mapIdIp_mutex);	
	pthread_mutex_destroy(&m_dspClientMapLock);
}

void CDspDealVideoTcp::CaptureFrame2()
{
	//RecvData2();
}

bool CDspDealVideoTcp::InitTcp(bool bH264Capture, bool bSkpRecorder, 
	CRoadH264Capture * pH264Capture, CSkpRoadRecorder * pSkpRecorder)
{
	m_bH264Capture  = bH264Capture;
	m_bSkpRecorder = bSkpRecorder;

	m_pH264Capture = pH264Capture;	
	m_pSkpRecorder = pSkpRecorder;

	if( (!m_bH264Capture && !m_bSkpRecorder) || (NULL == m_pH264Capture) && (NULL == m_pSkpRecorder))
	{
		return false;
	}

	if(m_nH264SocketBase)
	{
		//创建套接字
		if(m_nH264SocketBase->mvCreateSocket(m_nH264TcpFd,1)==false)
		{
			LogError("创建侦听接收h264套接字失败!\r\n");
			g_bEndThread = true;
			return false;
		}

		LogNormal("侦听h264套接字=%d\n", m_nH264TcpFd);

		//重复使用套接字
		if(m_nH264SocketBase->mvSetSocketOpt(m_nH264TcpFd,SO_REUSEADDR)==false)
		{
			LogError("设置重复使用套接字失败,服务无法启动!,%s\r\n",strerror(errno));
			g_bEndThread = true;
			return false;
		}

		//绑定服务端口
		if(m_nH264SocketBase->mvBindPort(m_nH264TcpFd, ROSEEK_DSP_TCP_H264_PORT)==false)
		{
			LogError("绑定到 ROSEEK_DSP_TCP_H264_PORT 端口失败,服务无法启动!\r\n");
			printf("%s\n",strerror(errno));
			g_bEndThread = true;
			return false;
		}

		//开始监听
		if (m_nH264SocketBase->mvStartListen(m_nH264TcpFd) == false)
		{
			LogError("监听连接失败，服务无法启动!\r\n");
			g_bEndThread = true;
			return false;
		}			
	}
	else
	{
		return false;
	}

	return true;	
}

//开启录像服务线程
bool CDspDealVideoTcp::StartServer()
{
	printf("----StartServer---------\n");
	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动线程
	int nretGj = pthread_create(&m_nThreadId7, NULL, ThreadDspVideoTcpAccept, this);
	if(nretGj != 0)
	{
		printf("==m_nThreadIdGj=%x==\n",m_nThreadId7);
		Close7();
		//失败
		LogError("创建接收公交H264码流线程失败!\n");
	}

	/*
	//启动线程
	int nretDetec = pthread_create(&m_nThreadStatusId, NULL, ThreadDspVideoLinkStatus, this);
	if(nretDetec != 0)
	{
		printf("==m_nThreadStatusId=%x==\n",m_nThreadStatusId);
		CloseDetectStatus();
		//失败
		LogError("创建Video码流监测线程失败!\n");
	}
	*/
	//pthread_detach(m_nThreadStatusId);

	LogNormal("Thrad Destroy id:%d StartServer 11", (int)m_nThreadStatusId);
	pthread_attr_destroy(&attr);

	return true;
}

//建立接收h264码流sock
bool CDspDealVideoTcp::h264Accept()
{
	printf("------h264Accept-\n");
	int nSocket = Geth264MoniSockFd();
	if(nSocket < 0)
	{
		LogNormal("=获取监听H264码流sock=失败!\n");
		return false;
	}
	else
	{
		LogNormal("=获取监听H264码流成功sock=%d!\n", nSocket);
	}

	//中心端连接
	struct sockaddr_in clientaddr;
	//长度
	socklen_t sin_size = sizeof(struct   sockaddr_in);
	int nClient;
	while(!g_bEndThread)
	{
		//接受连接
		if((nClient = accept(nSocket,(struct sockaddr*)&clientaddr,&sin_size)) ==  -1)
		{
			//printf("accept nSocket=%d, nClient = %d\r\n", nSocket, nClient);

			usleep(1000*10);
			//LogNormal("accept nClient = %d\r\n",nClient);
			//自动重启
			continue;
		}

		//输出用户连接
		LogNormal("dspvideo[IP:%s]连接,[nClient = %d,端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));

		std::string strIp = std::string(inet_ntoa(clientaddr.sin_addr));
		UINT32 uId = 0;

		//通过相机Ip获取相机id
		GetIdFromMap(strIp, uId);

		//if(uId > 0)
		{
			printf("--2222-----GetIdFromMap---uId=%d--\n", uId);

			DspClassAndFd classfd;
			//classfd.p = this;
			//classfd.id = 0;

			classfd.cfd.uCameraId = uId;
			classfd.cfd.fd = nClient;
			classfd.cfd.TimeCount = 0;
			classfd.cfd.connectFlag = true;
			classfd.cfd.strIp = strIp;

			Addh264Client(classfd);
			printf("----222--Addh264Client--uId=%d--fd=%d \n", uId, nClient);

			//sleep(5);
		}

		usleep(1000*10);
	}

	return true;
}

//关闭H264码流采集
bool CDspDealVideoTcp::Close()
{
	MapDspVideoUnit::iterator it_b = m_MapDspClient.begin();
	for(;it_b != m_MapDspClient.end(); it_b++)
	{
		pair<std::string, CDspVideoUnit*> p = *it_b;
		CDspVideoUnit* lp = p.second;
		if (lp)
		{
			LogNormal("CDspDealVideoTcp Close \n");
			lp->UnInit();
		}
	}

	if(m_pH264Capture != NULL)
    {
		m_pH264Capture->UnInit();
        m_pH264Capture = NULL;
		printf("===after m_pH264Capture->UnInit()==\n");
    }

	if(m_pSkpRecorder != NULL)
	{
		m_pSkpRecorder->UnInit();
		m_pSkpRecorder = NULL;

		printf("===after m_pSkpRecorder->UnInit()==\n");
	}
    return true;
}

bool CDspDealVideoTcp::Close7()
{
	LogNormal("=CRoSeekCameraDsp::Close7()==\n");
	if (m_nThreadId7 != 0)
	{
		pthread_cancel(m_nThreadId7);
		pthread_join(m_nThreadId7, NULL);
		m_nThreadId7 = 0;
	}

	return true;
}

bool CDspDealVideoTcp::Addh264Client(DspClassAndFd& classcfd)
{
	m_nH264SocketBase->mvSetSocketBlock(classcfd.cfd.fd,false);
	
	CDspVideoUnit * pUnit = NULL;
	pUnit = IsExistDspClient(classcfd.cfd.strIp);	

	if (pUnit == NULL)
	{
		pUnit = new CDspVideoUnit();

		bool bSet = pUnit->Seth264RecFd(classcfd);
		
		if(bSet)
		{
			pUnit->InitTcp(m_bEndCapture, m_bH264Capture);

			LogNormal("Add Dsp Video Client !\n");
			pthread_mutex_lock(&m_dspClientMapLock);
			m_MapDspClient.insert(MapDspVideoUnit::value_type (classcfd.cfd.strIp, pUnit));
			pthread_mutex_unlock(&m_dspClientMapLock);
		}
		else
		{
			return false;
		}		
	}
	else
	{
		LogNormal("Dsp Client[%s] is Exist!\n",classcfd.cfd.strIp.c_str());

		pUnit->CloseFd();

		//相同Ip, 连接而不删除,只重设
		pUnit->Seth264RecFd(classcfd);
		usleep(5*1000);	
	}
	return true;
}

bool CDspDealVideoTcp::Delh264Client(DspClassAndFd& classcfd)
{		
	MapDspVideoUnit::iterator it = m_MapDspClient.find(classcfd.cfd.strIp);
	if(it != m_MapDspClient.end())
	{
		LogNormal("=Delh264Client=uId:%d cfd.fd=%d==ip:%s\n", classcfd.cfd.uCameraId, classcfd.cfd.fd, classcfd.cfd.strIp.c_str());

		pair<std::string, CDspVideoUnit*> p = *it;
		p.first.erase();


		CDspVideoUnit* lp = p.second;
		if (lp)
		{
			LogNormal("Stop Dsp Video Client[IP:%s sock:%d]\n", classcfd.cfd.strIp.c_str(), classcfd.cfd.fd);
			LogNormal("---CloseFd video client-\n");
			lp->UnInit();

			delete lp;
			lp = NULL;
		}

		/*classcfd.cfd.TimeCount = 0;
		classcfd.cfd.connectFlag = false;
		shutdown(classcfd.cfd.fd,2);
		close(classcfd.cfd.fd);
		classcfd.cfd.fd = -1;
		classcfd.p = NULL;
		*/

		pthread_mutex_lock(&m_dspClientMapLock);
		m_MapDspClient.erase(it);
		pthread_mutex_unlock(&m_dspClientMapLock);
	}

	return true;
}

//更新ip,列表
void CDspDealVideoTcp::RefreshIpMap(const UINT32 uCamId, const int nSocket, const std::string strIp)
{
	//m_nSock = nSocket;//暂存

	pthread_mutex_lock(&m_mapIdIp_mutex);
	MapIdIp::iterator it = m_MapIdIp.find(uCamId);
	if(it != m_MapIdIp.end())
	{
		printf("--RefreshIpMap uId = %d, nSock=%d, ip=%s", uCamId, nSocket, strIp.c_str());
		it->second = strIp; //Refresh
	}
	else
	{
		if(uCamId > 0)
		{			
			for(MapIdIp::iterator it_b = m_MapIdIp.begin(); it_b != m_MapIdIp.end(); it_b++)
			{
				if(strIp == it_b->second)
				{
					LogNormal("Same ip diff CamId1:%d, CamId2:%d sock:%d ", it_b->first, uCamId, nSocket);
					//删除老的CamId记录
					m_MapIdIp.erase(it_b);
					break;
				}
			}

			LogNormal("-inser-uCamId:%d-nSock:%d,ip:%s \n", uCamId, nSocket, strIp.c_str());
			m_MapIdIp.insert(MapIdIp::value_type(uCamId,strIp));
		}		
	}	
	pthread_mutex_unlock(&m_mapIdIp_mutex);


	//更新uId
	//pthread_mutex_lock(&m_dspClientMapLock);
	//ip相同,更新CamId
	MapDspVideoUnit::iterator iter;
	iter = m_MapDspClient.find(strIp);
	if(iter != m_MapDspClient.end())
	{
		UINT32 uId = iter->second->GetId();

		if(uId != uCamId)
		{
			iter->second->RefreshId(uCamId);
		}		
	}

	//pthread_mutex_unlock(&m_dspClientMapLock);
}

//通过相机Ip获取相机id
bool CDspDealVideoTcp::GetIdFromMap(const std::string strIp, UINT32 &uCamId)
{
	bool bRet = false;
	//pthread_mutex_lock(&m_mapIdIp_mutex);
	MapIdIp::iterator it_b = m_MapIdIp.begin();
	for(;it_b != m_MapIdIp.end(); it_b++)
	{		
		if(strIp == it_b->second)
		{		
			if(it_b->first > 0)
			{
				uCamId = it_b->first;
				bRet = true;
				//LogNormal("--GetIdFromMap uId = %d, ip=%s", uCamId, strIp.c_str());	
			}
			else
			{
				printf("--GetIdFromMap---uCamId=%d ip=%s \n", uCamId, strIp.c_str());
			}
		}
	}

	//pthread_mutex_unlock(&m_mapIdIp_mutex);

	return bRet;
}

// 通过IP和Port查找Dsp是否存在
CDspVideoUnit* CDspDealVideoTcp::IsExistDspClient(std::string strIp)
{
	//pthread_mutex_lock(&m_dspClientMapLock);
	MapDspVideoUnit::iterator iter;
	iter = m_MapDspClient.find(strIp);
	if(iter != m_MapDspClient.end())
	{
		//pthread_mutex_unlock(&m_dspClientMapLock);
		return iter->second;
	}
	//pthread_mutex_unlock(&m_dspClientMapLock);
	return NULL;
}

//检查连接
void CDspDealVideoTcp::DetectStatus()
{
	while(!m_bEndCapture)
	{
		if(m_bH264Capture)
		{
			DetectStatusAndDeal();
			printf("---DetectStatusAndDeal-size:%d--\n", m_MapDspClient.size());
		}

		sleep(10);
	}
}

//检查连接，并处理
void CDspDealVideoTcp::DetectStatusAndDeal()
{
	MapDspVideoUnit::iterator it_b = m_MapDspClient.begin();
	for(;it_b != m_MapDspClient.end(); it_b++)
	{
		pair<std::string, CDspVideoUnit*> p = *it_b;
		CDspVideoUnit* lp = p.second;
		if (lp)
		{
			bool bStatus = lp->GetUnitStatus();
			if(!bStatus)
			{
				LogNormal("2 Stop Dsp Video Client[IP:%s ]\n", p.first.c_str());

				lp->CloseFd();

				//LogNormal("---CloseFd video client-\n");
				//lp->UnInit();
				//delete lp;
				//lp = NULL;

				//pthread_mutex_lock(&m_dspClientMapLock);
				//m_MapDspClient.erase(it_b);
				//pthread_mutex_unlock(&m_dspClientMapLock);

				break;//一次只删除一个
			}
		}
	}
}

bool CDspDealVideoTcp::CloseDetectStatus()
{
	LogNormal("=CloseDetectStatus:==\n");
	if (m_nThreadStatusId != 0)
	{
		pthread_cancel(m_nThreadStatusId);
		pthread_join(m_nThreadStatusId, NULL);
		m_nThreadStatusId = 0;
	}

	return true;
}