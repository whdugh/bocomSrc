// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


/**
 *   文件：MvsCommunication.h
 *   功能：检测器间通讯类
 *   作者：WanTao
 *   时间：2012-08-07
**/


#include "MvsCommunication.h"
#include "ximage.h"
#include "CSeekpaiDB.h"
//#include "CommonHeader.h"
#include "FileManage.h"
#include "ippi.h"
#include "ippcc.h"
#include "CvxText.h"
#include "RoadImcData.h"



CMvsCommunication g_mvsCommunication;

typedef struct _MvsClassAndFd
{
	CMvsCommunication* p;
	int fd;
	_MvsClassAndFd()
	{
		p = NULL;
		fd = 0;
	}

}MvsClassAndFd;

CMvsCommunication::CMvsCommunication()
{
	m_uRecvSocket = 0;
	m_uSendSocket = 0;
	

	m_uMapMvsClientFd.clear();
	pthread_mutex_init(&m_uMapClientMutex,NULL);

	m_uSendConnectFlag = false;
	m_NextLocationDataList.clear();
	m_RegionOverSpeedDataList.clear();
	
	pthread_mutex_init(&m_NextLocationData_Mutex,NULL);
	pthread_mutex_init(&m_RegionOverSpeed_Mutex,NULL);

	m_nRecLinkThreadId = 0;
	m_nSendLinkThreadId = 0;
	m_nRecordDealThreadId = 0;
	m_FontSize = 0;
	m_ExtentHeight = 0;
	m_nConnect = false;
	m_nOffsetX = 0;
	m_nOffsetY = 0;

	m_nDetectDirection = 0;

	m_nSendConnectFailureCount = 0;
	pthread_mutex_init(&m_nConnect_Mutex,NULL);
	
	m_imgResize = NULL;//缩放图像
}

CMvsCommunication::~CMvsCommunication()
{
	pthread_mutex_destroy(&m_uMapClientMutex);
	m_uMapMvsClientFd.clear();

	pthread_mutex_destroy(&m_NextLocationData_Mutex);
	pthread_mutex_destroy(&m_RegionOverSpeed_Mutex);

	pthread_mutex_destroy(&m_nConnect_Mutex);

	//关闭连接
	mvCloseSocket(m_uRecvSocket);
}

//准备发送给下一个点位的卡口数据，用于做区间测速
bool CMvsCommunication::PreDataForNextLocation(string& strMsg)
{
	pthread_mutex_lock(&m_NextLocationData_Mutex);
	if (m_NextLocationDataList.size() > 5)
	{
		LogError("PreDataForNextLocation 记录过多\n");
		m_NextLocationDataList.pop_back();
	}
	m_NextLocationDataList.push_front(strMsg);
	pthread_mutex_unlock(&m_NextLocationData_Mutex);
	return true;
}

//获取数据，并发送给下一个点位主机
void CMvsCommunication::GetDataAndSend()
{
	string nResponse;
	nResponse.clear();
	pthread_mutex_lock(&m_NextLocationData_Mutex);
	if (m_NextLocationDataList.size() > 0)
	{
		nResponse = m_NextLocationDataList.back();
		m_NextLocationDataList.pop_back();

	}
	pthread_mutex_unlock(&m_NextLocationData_Mutex);
	if (nResponse.size() > 0)
	{
		if(!SendPlateMsg(nResponse))//发送本点位的车牌图片信息给下个点位
		{
			LogError("SendPlateMsg error\n");
			return;
		}
	}
	return;
}

//发送数据给下一个点位的线程函数。
void* MvsThreadSendDatatoNextLocation(void* pArg)
{
	CMvsCommunication* pmvs = (CMvsCommunication*)pArg;
	if (pmvs == NULL)
	{
		return pArg;
	}
	while (!g_bEndThread)
	{
		usleep(10*1000);
		pmvs->GetDataAndSend();
		
	}
	pthread_exit((void *)0);
	return pArg;
}

bool CMvsCommunication::SendConnect()
{
	if (g_MvsNextHostIp.empty() || g_MvsNextHostIp == "0.0.0.0")
	{
		//LogError("\n主动连接参数设置错误:host=%s,port=%d\n", g_MvsNextHostIp.c_str(), g_MvsNextRecPort);
		return false;
	}

	//LogNormal("开始连接对方IP，端口! host=%s,port=%d\n", g_MvsNextHostIp.c_str(), g_MvsNextRecPort);

	if (!mvPrepareSocket(m_uSendSocket))
	{
		//m_uSendSocket = 0;
		//LogError("\n主动连接，准备套接字失败!\n");
		return false;
	}
	if (!mvWaitConnect(m_uSendSocket, g_MvsNextHostIp, g_MvsNextRecPort))
	{
		//m_uSendSocket = 0;
		LogError("\n尝试连接对方IP，端口失败!\n");
		return false;
	}
	else
	{
		LogNormal("Connect Host:%s success\n",g_MvsNextHostIp.c_str());
		m_uSendConnectFlag = true;
	
		GetSendConnectFailureCount() = 0;
	
		//启动发送数据给下一个点位的线程
		//线程id
		/*pthread_t id;
		//线程属性
		pthread_attr_t   attr;
		//初始化
		pthread_attr_init(&attr);
		//分离线程
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

		//启动发送数据给下一个点位的线程
		int nret=pthread_create(&id,&attr,MvsThreadSendDatatoNextLocation,this);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("启动发送数据给下一个点位的线程!\r\n");
			return false;
		}

		pthread_attr_destroy(&attr);*/
	}
	return true;
}

//监控线程
void* MvsThreadAccept(void* pArg)
{
	CMvsCommunication* pmvs =(CMvsCommunication*)pArg;
	
	struct sockaddr_in clientaddr;
	memset(&clientaddr,0,sizeof(clientaddr));
	//长度
	socklen_t sin_size = sizeof(struct sockaddr_in);
	int nClient;

	while(!g_bEndThread)
	{
		//接受连接
		if((nClient = accept(pmvs->GetRecvSocket(),(struct sockaddr*)&clientaddr,&sin_size)) ==  -1)
		{
			//断开连接
			if(g_bEndThread)
			{
				pthread_exit((void *)0);
				return pArg;
			}
			//自动重启
			usleep(1000);
			continue;
		}

		//输出用户连接
		LogNormal("客户端连接[IP:%s][nClient = %d,端口:%d]!\r\n",inet_ntoa(clientaddr.sin_addr),nClient,ntohs(clientaddr.sin_port));
		LogNormal("侦听套接字=%d\n",pmvs->GetRecvSocket());
		//将客户端套接字添加到套接字列表中
		MvsClientFd tmpFd;
		tmpFd.fd = nClient;
		tmpFd.HostIp = inet_ntoa(clientaddr.sin_addr);
		pmvs->AddClient(tmpFd);
	}
	pthread_exit((void *)0);
	return pArg;
}

bool CMvsCommunication::RecvConnect()
{

	if (mvCreateSocket(m_uRecvSocket,1)==false)
	{
		m_uRecvSocket = 0;
		LogError("接收连接，创建套接字失败\n");
		g_bEndThread = true;
		return false;
	}

	if(mvSetSocketOpt(m_uRecvSocket,SO_REUSEADDR)==false)
	{
		m_uRecvSocket = 0;
		LogError("接收连接，设置重复使用套接字失败：%s\r\n",strerror(errno));
		g_bEndThread = true;
		return false;
	}

	if(mvBindPort(m_uRecvSocket,g_MvsNextRecPort /*g_MvsRecPort*/)==false)
	{
		LogError("接收连接，绑定到 %d 端口失败:%s!\r\n",g_MvsNextRecPort/*g_MvsRecPort*/,strerror(errno));
		g_bEndThread = true;
		return false;
	}

	if (mvStartListen(m_uRecvSocket) == false)
	{
		LogError("接收连接,监听连接失败!\r\n");
		g_bEndThread = true;
		return false;
	}

	//启动接收连接线程
	//线程id
	//pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	//启动事件监控线程
	int nret=pthread_create(&m_nRecLinkThreadId,&attr,MvsThreadAccept,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("接收连接,创建事件监控线程失败!\r\n");
		return false;
	}

	pthread_attr_destroy(&attr);
	return true;
	
}

void* MvsThreadSendConnect(void* pArg)
{
	CMvsCommunication* pmvs = (CMvsCommunication*)pArg;
	if (pmvs == NULL)
	{
		return pArg;
	}
	while(!g_bEndThread)
	{
		pthread_mutex_lock(&(pmvs->GetConncetMutex_t()));
		if (!(pmvs->GetSendConnectFlag()))
		{
			if (!(pmvs->SendConnect()))
			{
				//LogError("发送主动连接失败\n");
				if (pmvs->GetSendSocket() > 0)
				{
					pmvs->mvCloseSocket(pmvs->GetSendSocket());
				}
				//cerr<<"befor pmvs->GetSendConnectFailureCount():"<<pmvs->GetSendConnectFailureCount()<<endl;
				++(pmvs->GetSendConnectFailureCount());
				//cerr<<"after pmvs->GetSendConnectFailureCount():"<<pmvs->GetSendConnectFailureCount()<<endl;

				if (pmvs->GetSendConnectFailureCount() > 1000)//大于1000次重新从12开始循环加
				{
					pmvs->GetSendConnectFailureCount() = 12;
				}
			}
		}
		pthread_mutex_unlock(&(pmvs->GetConncetMutex_t()));
		sleep(5);
	}
	pthread_exit((void *)0);
	return pArg;
}

//临时表和临时图片数据管理线程，
void* TmpTableAndPicManageThread(void* pArg)
{
	CMvsCommunication* pmvs = (CMvsCommunication*)pArg;
	if (pmvs == NULL)
	{
		return pArg;
	}
	while(!g_bEndThread)
	{
		sleep(2);
		pmvs->ManageTmpRecordAndPic();
	}
	pthread_exit((void *)0);
	return pArg;

}

pthread_mutex_t& CMvsCommunication::GetConncetMutex_t()
{
	return m_nConnect_Mutex;
}

int& CMvsCommunication::GetSendConnectFailureCount()
{
	return m_nSendConnectFailureCount;
} 

//当表中记录大于1000条时，删除最早的记录及其对应的图片数据
void CMvsCommunication::ManageTmpRecordAndPic()
{
	char buf[1024] = {0};
	int nCount = 0;
	sprintf(buf,"select count(ID) from NUMBER_PLATE_INFO_RECV");
	String sql(buf);
	nCount = g_skpDB.getIntFiled(sql);
	//LogNormal("NUMBER_PLATE_INFO_RECV count(ID):%d\n",nCount);
	if (nCount > 1000)
	{
		memset(buf,0,sizeof(buf));
		sprintf(buf,"select * from NUMBER_PLATE_INFO_RECV ORDER BY TIME asc limit 1");
		string strSql(buf);
		MysqlQuery q = g_skpDB.execQuery(strSql);
		if (!q.eof())
		{
			//string strPicPath = q.getStringFileds("PICPATH");
			string strCarNum = q.getStringFileds("NUMBER");
			//g_skpDB.UTF8ToGBK(strCarNum);
			if (g_skpDB.MvsCommunicationDelPlate(false,strCarNum) != SRIP_OK)
			{
				LogError("删除号牌为%s的记录失败\n",strCarNum.c_str());
			}
			else
			{
				//LogNormal("删除号牌为%s的记录成功\n",strCarNum.c_str());
			}
		}
		q.finalize();
	}

}

bool CMvsCommunication::Init()
{
	//加载配置文件
	/*CXmlParaUtil xml;
	xml.UpdateSystemSetting("MvsCommunicationRegionSpeedSetting", "");*/
	/*LogNormal("NextHostIp:%s\n",g_MvsNextHostIp.c_str());
	LogNormal("NextPort:%d\n",g_MvsNextRecPort);
	LogNormal("Port:%d\n",g_MvsRecPort);*/
	//先删除临时表和临时图片文件夹中的所有内容
	//g_skpDB.MvsCommunicationDelPlate(true,"All");
	if (IsDataDisk())
	{
		g_strMvsRecvPic = "/detectdata/recvPic";
	}

	//接受连接
	RecvConnect();
	//启动主动连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int nret;

	if (!(g_MvsNextHostIp.empty() || g_MvsNextHostIp == "0.0.0.0"))
	{
		//启动主动连接线程
		nret = pthread_create(&m_nSendLinkThreadId,&attr,MvsThreadSendConnect,this);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("接收连接,创建事件监控线程失败!\r\n");
			g_bEndThread = true;
			return false;
		}
	


		//启动发送数据给下一个点位的线程
		nret=pthread_create(&id,&attr,MvsThreadSendDatatoNextLocation,this);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("启动发送数据给下一个点位的线程!\r\n");
			return false;
		}
	}


	nret = pthread_create(&id,&attr,TmpTableAndPicManageThread,this); 
	if(nret != 0)
	{
		LogError("创建管理临时表和临时图片线程失败\n");
		g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);

	return true;
}

bool CMvsCommunication::UnInit()
{
	if(m_imgResize != NULL)
	{
		cvReleaseImage(&m_imgResize);
		m_imgResize = NULL;
	}

	if(m_nRecordDealThreadId != 0)
	{
		pthread_join(m_nRecordDealThreadId,NULL);
		m_nRecordDealThreadId = 0;

	}
	if (m_nSendLinkThreadId != 0)
	{
		pthread_join(m_nSendLinkThreadId,NULL);
		m_nSendLinkThreadId = 0;
	}
	if (m_nRecLinkThreadId != 0)
	{
		pthread_join(m_nRecLinkThreadId,NULL);
		m_nRecLinkThreadId = 0;
	}
	////先删除临时表和临时图片文件夹中的所有内容
	//g_skpDB.MvsCommunicationDelPlate(true,"All");
	return true;
}

//获取接收套接字
int& CMvsCommunication::GetRecvSocket()
{
	return m_uRecvSocket;
}

void* ThreadRecvPlateAndPic(void* pArg)
{
	MvsClassAndFd pmvs = *(MvsClassAndFd*)pArg;
	(pmvs.p)->RecvPlateAndPictureMsg(pmvs.fd);
	pthread_exit((void *)0);
	return pArg;
}

//增加客户端套接字
void CMvsCommunication::AddClient(MvsClientFd& clientFd)
{
	pthread_mutex_lock(&m_uMapClientMutex);
	mapMvsClientFd::iterator it = m_uMapMvsClientFd.find(clientFd.fd);
	if (it != m_uMapMvsClientFd.end())
	{
		m_uMapMvsClientFd.erase(it);
		LogNormal("AddClient 删除相同套接字%d,from IP:%s\n",clientFd.fd,clientFd.HostIp.c_str());
	}
	m_uMapMvsClientFd.insert(mapMvsClientFd::value_type(clientFd.fd,clientFd));
	
	pthread_mutex_unlock(&m_uMapClientMutex);


	//启动接收线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	MvsClassAndFd pmvs;
	pmvs.p = this;
	pmvs.fd = clientFd.fd;

	int nret = pthread_create(&id,&attr,ThreadRecvPlateAndPic,(void*)&pmvs);
	if(nret!=0)
	{
		DelClient(clientFd.fd);
		//失败
		LogError("123创建客户端接收数据线程失败,连接断开!\r\n");
		return;
	}
	pthread_attr_destroy(&attr);
	//usleep(100);
	return;
}
//删除客户端套接字
void CMvsCommunication::DelClient(int fd)
{
	pthread_mutex_lock(&m_uMapClientMutex);
	mapMvsClientFd::iterator it = m_uMapMvsClientFd.find(fd);
	if (it != m_uMapMvsClientFd.end())
	{
		mvCloseSocket(fd);
		m_uMapMvsClientFd.erase(it);
	}
	else
	{
		LogError("DelClient:没有找到该套接字:%d\n",fd);
	}
	pthread_mutex_unlock(&m_uMapClientMutex);
}

//获取主动连接成功标志
bool& CMvsCommunication::GetSendConnectFlag()
{
	return m_uSendConnectFlag;
}

//获取发送套接字
int& CMvsCommunication::GetSendSocket()
{
	return m_uSendSocket;
}

//添加区间超速数据到列表
bool CMvsCommunication::AddRegionOverSpeedData(string& strMsg)
{
	pthread_mutex_lock(&m_RegionOverSpeed_Mutex);
	if (m_RegionOverSpeedDataList.size() > 3)
	{
		LogError("区间超速记录过多\n");
		m_RegionOverSpeedDataList.pop_back();
	}
	m_RegionOverSpeedDataList.push_front(strMsg);
	pthread_mutex_unlock(&m_RegionOverSpeed_Mutex);
	return true;
}

//根据车牌在区间超速数据列表中找到相应的记录
string CMvsCommunication::GetRegionOverSpeedData(string& strPlate)
{
	string nResponse("");
	pthread_mutex_lock(&m_RegionOverSpeed_Mutex);
	if (m_RegionOverSpeedDataList.size() <= 0)
	{
		//LogError("区间超速列表中没有数据\n");
		nResponse = "";
	}
	else
	{
		CSResultMsg::iterator it = m_RegionOverSpeedDataList.begin();
		while (it != m_RegionOverSpeedDataList.end())
		{
			nResponse.clear();
			nResponse = *it;
			RECORD_PLATE* p = (RECORD_PLATE*)(nResponse.c_str());
			if (strcmp(p->chText,strPlate.c_str()) == 0)
			{
				m_RegionOverSpeedDataList.erase(it);
				break;
			}
			it++;
		}
	}
	pthread_mutex_unlock(&m_RegionOverSpeed_Mutex);
	return nResponse;
}

//区间超速，点超速取舍发送函数
//函数输入 p：点超速车牌记录指针，nFlag：返回发送标志，0,表示发送点超速，1，表示发送区间超速
//函数返回： 区间超速记录(车牌+图片相关信息);
string CMvsCommunication::RecordSendWay(RECORD_PLATE* plate, int& nFlag)
{
	string nStrRegionRecord("");
	string nStrNumTmp = plate->chText;
	int nPos = -1;
	nPos = nStrNumTmp.find("*");
	if (nPos <= -1)
	{
		nStrRegionRecord = g_mvsCommunication.GetRegionOverSpeedData(nStrNumTmp);
		if (nStrRegionRecord .size() == 0)
		{
			nFlag = 0;
		}
		else
		{
			RECORD_PLATE* pPlateRecord = (RECORD_PLATE*)(nStrRegionRecord.c_str());
			if (plate->uSpeed >= pPlateRecord->uSpeed)
			{
				nFlag = 0;
			}
			else
			{
				nFlag = 1;
			}
		}
	}
	else
	{
		nFlag = 0;
	}
	return nStrRegionRecord;
}

//处理检测结果
bool CMvsCommunication::OnResult(std::string& result)
{
		//cerr<<"44444444444444"<<endl;
		string strTmp = result;
		RECORD_PLATE* sPlateTmp = (RECORD_PLATE*)(result.c_str());

		/////////////add
		//string strMyPicdata("");
		//strMyPicdata = ComposePicture(*sPlateTmp,0,0);
		//result = result + strMyPicdata;

		////////////add

		String strPicPath(sPlateTmp->chPicPath);
		result = result + GetImageByPath(strPicPath);


		RECORD_PLATE* sPlate = (RECORD_PLATE*)(strTmp.c_str());
		string strPreRecord;
		strPreRecord.clear();
		string sPlateNum = sPlate->chText;
		string strCrossingNumber("");

		if (g_MvsNextHostIp.empty() || g_MvsNextHostIp == "0.0.0.0")//接收端（只比不发）
		{
			//合成数据
			string nStrComposePicData,strPic1,strPic2;
			nStrComposePicData.clear();
			//限速值
			
			//最大限制速度,输出限制为标准上浮10%
			//nSpeed = (UINT32)(nSpeed * 1.1 + 0.5);

			UINT32 nSpeed = sPlate->uOverSpeed;
			if(sPlate->uOverSpeed <= 0)
			{
				//cerr<<"11111111sPlate->uChannelID:"<<sPlate->uChannelID<<" sPlate->uRoadWayID:"<<sPlate->uRoadWayID<<endl;
				nSpeed = GetMaxSpeed(sPlate->uType, sPlate->uChannelID,sPlate->uRoadWayID);
				//cerr<<"222222sPlate->uChannelID:"<<sPlate->uChannelID<<" sPlate->uRoadWayID:"<<sPlate->uRoadWayID<<endl;				
			}
			//LogNormal("GetMaxSpeed:%d upMax:%d", nSpeed, sPlate->uOverSpeed);
			unsigned int IsCurrentLocationRecord = 1; //0：上一个点的超速记录，1：本点超速记录
			string strRoadCode("");
			//从缓冲表RECORD_PLATE_INFO_RECV中取相应的记
			//cerr<<"000000"<<endl;
			if(GetOneRecord(sPlateNum,strPreRecord,strCrossingNumber))
			{
				//cerr<<"111111111"<<endl;
				//cerr<<"GetOneRecord strCrossingNumber:"<<strCrossingNumber<<endl;
				//比较发送端，接收端，区间的速度
				RECORD_PLATE* sMvsRecPlate = (RECORD_PLATE *)(strPreRecord.c_str()); //发送端

				UINT32 nTime = (sPlate->uTime) - (sMvsRecPlate->uTime);
				///////////////just test
				//nTime += 280;
				///////////just tes
				//此时测区间速度
				float nRegionSpeed = CalRegionSpeed((float)g_MvsDistance,nTime);
				UINT32 uRegionSpeed = (UINT32)nRegionSpeed;
				//速度大于250，无意义，就取255
				if (uRegionSpeed > 250)
				{
					uRegionSpeed = 255;
				}
				
				UINT32 uMaxSpeed = sPlate->uSpeed;
				if(sMvsRecPlate->uSpeed >= sPlate->uSpeed)
				{
					uMaxSpeed = sMvsRecPlate->uSpeed;

					if(uRegionSpeed >= sMvsRecPlate->uSpeed)
					{
						uMaxSpeed = uRegionSpeed;
					}
				}
				else
				{
					if(uRegionSpeed >= sPlate->uSpeed)
					{
						uMaxSpeed = uRegionSpeed;
					}
				}

				uMaxSpeed = uRegionSpeed;

				if(uMaxSpeed >= nSpeed)//是否超速
				{
					if(uMaxSpeed == uRegionSpeed)
					{
						if(sMvsRecPlate->uTime > sPlate->uTime)
						{
							LogNormal("区间测速上一点位的时间大于本点位的时间，不输出\n");
							//删除临时表中对应的车牌数据
							g_skpDB.MvsCommunicationDelPlate(false,sPlateNum);
							return false;
						}
						sPlate->uViolationType = DETECT_RESULT_EVENT_DISTANCE_FAST;
						
						if(23 == g_nServerType || 26 == g_nServerType)
						{
							//合成上一个点位和本位之间图片并叠加相关文字信息 
							//LogNormal("p1:%s, p2:%s ", sPlate->chText, sMvsRecPlate->chText);
							nStrComposePicData = ComposePic1x2(*sMvsRecPlate,*sPlate,uRegionSpeed);
						}
						else
						{
							//合成上一个点位和本位之间图片并叠加相关文字信息 
							nStrComposePicData = ComposePicture(*sMvsRecPlate,*sPlate,uRegionSpeed,strPic1,strPic2);
						}
						
						sPlate->uSpeed = uRegionSpeed;
						sPlate->uTime2 = sMvsRecPlate->uTime;

						//sPlate->uRoadWayID = 1;//区间车道号为1

						nStrComposePicData.insert(0,(char*)sPlate,sizeof(RECORD_PLATE));

						strRoadCode = sMvsRecPlate->chPlace;
						strRoadCode += "-";
						strRoadCode += sPlate->chPlace;
						IsCurrentLocationRecord = 2;
					}
						
				}
				//删除临时表中对应的车牌数据
				g_skpDB.MvsCommunicationDelPlate(false,sPlateNum);
			}
			else
			{
			}

			//写数据库并发送客户端
			if(nStrComposePicData.size() >= sizeof(RECORD_PLATE))
			{
				RECORD_PLATE* pRegionRecord = (RECORD_PLATE*)(nStrComposePicData.c_str());
				if(pRegionRecord->uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
				{
					string strComposePicture("");
					string nComposePicPath(""),strPicPath1(""),strPicPath2("");
					
					int nRet = 0;
					bool nSaveFlag = false;

					if(g_nServerType == 13 && g_nFtpServer == 1)
					{
						g_MyCenterServer.GetPlatePicPath(*pRegionRecord,strPicPath1,3);
						g_MyCenterServer.GetPlatePicPath(*pRegionRecord,strPicPath2,4);
						nRet = 1;
						memset(pRegionRecord->chPicPath,0,sizeof(pRegionRecord->chPicPath));
						memcpy(pRegionRecord->chPicPath,strPicPath1.c_str(),strPicPath1.size());

						nSaveFlag = g_mvsCommunication.SavePicData(strPic1,strPic1.size(),strPicPath1);
						g_mvsCommunication.SavePicData(strPic2,strPic2.size(),strPicPath2);
					}
					else
					{
						strComposePicture.append(nStrComposePicData.c_str()+sizeof(RECORD_PLATE),nStrComposePicData.size()-sizeof(RECORD_PLATE));
						nRet = g_mvsCommunication.GetPicPathAndSaveDB(nComposePicPath);
						if (nRet > 0)
						{
							string strTmp1 = pRegionRecord->chPicPath;
							memset(pRegionRecord->chPicPath,0,strTmp1.size());
							memcpy(pRegionRecord->chPicPath,nComposePicPath.c_str(),nComposePicPath.size());
							
							nSaveFlag = g_mvsCommunication.SavePicData(strComposePicture,strComposePicture.size(),nComposePicPath);
						}
					}
	
					if (nSaveFlag)
					{
							g_skpDB.MvsCommunicationSaveRegionSpeed(*pRegionRecord);

							if (m_nConnect)
							{
								SRIP_DETECT_HEADER sDetectHeader;
								sDetectHeader.uChannelID = pRegionRecord->uChannelID;
								sDetectHeader.uDetectType = SRIP_CARD_RESULT;
								sDetectHeader.uTimestamp = pRegionRecord->uTime;
								sDetectHeader.uSeq = pRegionRecord->uSeq;

								std::string result;

								result.append((char*)pRegionRecord,sizeof(RECORD_PLATE));

								result.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
								g_skpChannelCenter.AddResult(result);
							}
					}
						
				}
				else
				{
					string nComposePicData("");
					nComposePicData.append(nStrComposePicData.c_str() + sizeof(RECORD_PLATE),nStrComposePicData.size() - sizeof(RECORD_PLATE));
					//获取图片路径并存储数据库中
					string nComposePicPath;
					/*string strHc1Path("");
					string strHc2Path("");*/

					/*g_MyCenterServer.GetPlatePicPath(*pRegionRecord,strHc1Path,3);
					g_MyCenterServer.GetPlatePicPath(*pRegionRecord,strHc2Path,4);*/
					nComposePicPath.clear();
					int nRet = g_mvsCommunication.GetPicPathAndSaveDB(nComposePicPath);
					if (/*strHc1Path.size() > 0 && strHc2Path.size() > 0*/ nRet > 0)
					{
						string strTmp1 = pRegionRecord->chPicPath;
						memset(pRegionRecord->chPicPath,0,strTmp1.size());
						/*memcpy(pRegionRecord->chPicPath,strHc1Path.c_str(),strHc1Path.size());*/
						memcpy(pRegionRecord->chPicPath,nComposePicPath.c_str(),nComposePicPath.size());
						bool nSaveFlag = false;
						nSaveFlag = g_mvsCommunication.SavePicData(nComposePicData,nComposePicData.size(),nComposePicPath);
						//g_mvsCommunication.SavePicData(nComposePicData,nComposePicData.size(),strHc2Path);
						if (IsCurrentLocationRecord == 0)
						{	
							string strText = pRegionRecord->chText;
							g_skpDB.GBKToUTF8(strText);
							memset(pRegionRecord->chText,0,strText.size());
							memcpy(pRegionRecord->chText,strText.c_str(),strText.size());
						}
						if (nSaveFlag)
						{
							g_skpDB.MvsCommunicationSaveRegionSpeed(*pRegionRecord);
							if (m_nConnect)
							{
								SRIP_DETECT_HEADER sDetectHeader;
								sDetectHeader.uChannelID = pRegionRecord->uChannelID;
								sDetectHeader.uDetectType = SRIP_CARD_RESULT;
								sDetectHeader.uTimestamp = pRegionRecord->uTime;
								sDetectHeader.uSeq = pRegionRecord->uSeq;

								std::string result;

								result.append((char*)pRegionRecord,sizeof(RECORD_PLATE));

								result.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
								g_skpChannelCenter.AddResult(result);
							}

							if (IsCurrentLocationRecord == 0)
							{

							}
							else if (IsCurrentLocationRecord == 1)
							{
							}
						}
						//cerr<<"555pRegionRecord->uChannelID:"<<pRegionRecord->uChannelID<<" pRegionRecord->uRoadWayID:"<<pRegionRecord->uRoadWayID<<endl;
						
					}
					else
					{
						LogError("获取路径失败\n");
					}
				}
				char buf[1024];
				memset(buf, 0, 1024);
				if (IsCurrentLocationRecord == 0)//上一个点超速
				{
					sprintf(buf, "update NUMBER_PLATE_INFO set PLACE = '%s',CrossingNumber = '%s' where ID = %u;",strRoadCode,strCrossingNumber,pRegionRecord->uSeq);
				}
				else if (IsCurrentLocationRecord == 1)//本点超速
				{
					string strCurrCrossingNumber("");
					if (g_strDetectorID.size() < 5)//长度小于5默认为“AAAAA”
					{
						strCurrCrossingNumber = "AAAAA";
					}
					else
					{
						strCurrCrossingNumber = g_strDetectorID.substr(0,5);
					}
					
					sprintf(buf, "update NUMBER_PLATE_INFO set PLACE = '%s',CrossingNumber = '%s' where ID = %u;",strRoadCode,strCurrCrossingNumber,pRegionRecord->uSeq);
				}
				else if (IsCurrentLocationRecord == 2)//区间超速
				{
					REGION_ROAD_CODE_INFO regionRoadCodeInfo;
					CXmlParaUtil xml;
					string nstrRegionCode("");
					if(xml.LoadPicFormatInfo(pRegionRecord->uChannelID,regionRoadCodeInfo))
					{
						nstrRegionCode = regionRoadCodeInfo.nRegionRoadCode;

					}
					sprintf(buf, "update NUMBER_PLATE_INFO set PLACE = '%s',CrossingNumber = '%s' where ID = %u;",strRoadCode,nstrRegionCode,pRegionRecord->uSeq);
				}
				g_skpDB.execSQL(string(buf));
			}
			/*else
			{
				LogError("图片大小为空\n");
			}*/
		}
		else //发送端（只发不比）
		{
			//cerr<<"22222222"<<endl;
			pthread_mutex_lock(&m_nConnect_Mutex);
			if (m_uSendConnectFlag)
			{
				char szLength[8] = {0};
				sprintf(szLength,"%07d",result.size());

				string strCrossingNumber("");

				if (g_strDetectorID.size() < 5)//长度小于5默认为“AAAAA”
				{
					strCrossingNumber = "AAAAA";
				}
				else
				{
					strCrossingNumber = g_strDetectorID.substr(0,5);//协议中路口号只有五位
				}	
				result.insert(0,strCrossingNumber.c_str(),5);

				//cerr<<"11111strCrossingNumber:"<<strCrossingNumber<<endl;
				result.insert(5,szLength,7);
				//cerr<<"333result.size:"<<result.size()<<endl;
				PreDataForNextLocation(result);
			}
			else
			{
				if (GetSendConnectFailureCount() >= 12)//断开一分钟认为网络出问题，起点开始做点超速
				{
					//StartingPointOverSpeed(sPlate);
				}
			}
			pthread_mutex_unlock(&m_nConnect_Mutex);
			
		}

		return true;
}

//区间超速的网络断开时，在起点做点超速

void CMvsCommunication::StartingPointOverSpeed(RECORD_PLATE* &sPlate)
{
}


//将某点位抓到的图片解码编码成一副图像
string CMvsCommunication::ComposePicture(RECORD_PLATE& nPlate,UINT32 nSpeed,int nType,string& strCrossingNumber)
{
	string strComposePicMsg;
	strComposePicMsg.clear();

	CxImage nBigImage;
	CxImage nSmallImage;

	int m_nExtentHeight = g_PicFormatInfo.nExtentHeight;


	if (nPlate.uPicHeight == (DSP_500_BIG_HEIGHT + 96))
	{
		m_nExtentHeight = 96;
	}

	if (nPlate.uPicHeight == (DSP_200_BIG_HEIGHT + 64))
	{
		m_nExtentHeight = 64;
	}

	if (nPlate.uPicHeight == (DSP_600_BIG_465_HEIGHT + 112))
	{
		m_nExtentHeight = 112;
	}
	
	if(nPlate.uPicWidth > 2* nPlate.uPicHeight)
	{
		LogError("卡口图片叠加方式不正确\n");
		return strComposePicMsg;
	}

	string nPicdata = GetImageByPath(nPlate.chPicPath);
	/////add
	//string nstrSmallPicPath("");
	//g_MyCenterServer.GetPlatePicPath(nPlate,nstrSmallPicPath,1);
	//string strSmallPicData("");
	//strSmallPicData = GetImageByPath(nstrSmallPicPath);
	//////////add

	if (g_PicFormatInfo.nSmallPic == 1)
	{
		
		//if (g_nPicMode == 1)
		//{
		//	nBigImage.Decode((BYTE*)(nPicdata.c_str()), nPlate.uPicSize, 3);//本点位大图解码
		//	if(nPlate.uSmallPicSize > 0)
		//	{
		//		nSmallImage.Decode((BYTE*)(nPicdata.c_str() + nPlate.uPicSize), nPlate.uSmallPicSize, 3);//本点位小图解码
		//	}
		//}
		//else
		//{
			LogNormal("暂不支持卡口叠加小图方式\n");
			//LogNormal("暂不支持卡口叠加小图,但卡口图片组合方式为叠加方式\n");
			return strComposePicMsg;
		//}
		
		//nBigImage.Decode((BYTE*)(nPicdata.c_str()), nPicdata.size(), 3);//本点位大图解码
		//nSmallImage.Decode((BYTE*)(strSmallPicData.c_str()), strSmallPicData.size(), 3);//本点位小图解码

			//if(nPlate.uSmallPicSize > 0)
			//{
			//	nSmallImage.Decode((BYTE*)(nPicdata.c_str() + nPlate.uPicSize), nPlate.uSmallPicSize, 3);//本点位小图解码
			//}
	}
	else
	{
		nBigImage.Decode((BYTE*)(nPicdata.c_str()), nPlate.uPicSize, 3);//本点位大图解码
		int nPicSmallSize = 0;
		string nSamllPicData = g_MyCenterServer.GetSmallPicFromBigPic(&nPlate,nPicSmallSize);
		if (nSamllPicData.size() > 0)
		{
			nSmallImage.Decode((BYTE*)(nSamllPicData.c_str()),nPicSmallSize,3);
		}
		else
		{
			LogError("获取本点位大图图片的小图失败\n");
			return strComposePicMsg;
		}
	}

		if (nBigImage.IsValid() && nSmallImage.IsValid())
		{

				UINT32 uPicWidth = nBigImage.GetWidth();
				UINT32 uPicHeight = nBigImage.GetHeight();

			
				CxImage nTmpImage;
				nSmallImage.Resample(uPicWidth,uPicHeight,1,&nTmpImage);//小图设成大图的大小
				/*CvRect rect;
				rect.x = 0;
				rect.y = 0;
				rect.width = uPicWidth;
				rect.height = uPicHeight;

				CvRect rt;
				rt.*/
				
				if (!nTmpImage.IsValid())
				{
					LogNormal("nTmpImage is InValid\n");
					return strComposePicMsg;
				}
				
				UINT32 uImageSize = (2*uPicWidth)*(uPicHeight);
				unsigned char* pJpgImage = NULL;
				pJpgImage = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));

				if (pJpgImage == NULL)
				{
					LogError("111111111pIpgImage malloc error\n");
					return strComposePicMsg;
				}

				IplImage *pComposePicture = NULL;
				pComposePicture = cvCreateImage(cvSize(2*uPicWidth,uPicHeight),8,3);
				
				if (pComposePicture == NULL)
				{
					LogError("11111111pComposePicture malloc error");
					return strComposePicMsg;
				}


				IplImage *pSmalPicture = NULL;
				pSmalPicture = cvCreateImageHeader(cvSize(uPicWidth,uPicHeight),8,3);
				if (pSmalPicture == NULL)
				{
					LogError("111111pSmallPicture malloc error\n");
					return strComposePicMsg;
				}


				cvSetData(pSmalPicture,nTmpImage.GetBits(),pSmalPicture->widthStep);
				
				CvRect dstRt;
				dstRt.x = 0;
				dstRt.y = 0;
				dstRt.height = uPicHeight;
				dstRt.width = uPicWidth;
				cvSetImageROI(pComposePicture,dstRt);
				cvCopy(pSmalPicture,pComposePicture);
				cvResetImageROI(pComposePicture);
				
				IplImage *pBigPicture = NULL;
				pBigPicture = cvCreateImageHeader(cvSize(uPicWidth,uPicHeight),8,3);
				if (pBigPicture == NULL)
				{
					LogError("1111111pBigPicture malloc error\n");
					return strComposePicMsg;
				}
				cvSetData(pBigPicture,nBigImage.GetBits(),pBigPicture->widthStep);

				dstRt.x = uPicWidth;
				dstRt.y = 0;
				dstRt.height = uPicHeight;
				dstRt.width = uPicWidth;
				cvSetImageROI(pComposePicture,dstRt);
				cvCopy(pBigPicture,pComposePicture);
				cvResetImageROI(pComposePicture);


			    dstRt.x = 0;
				dstRt.y = uPicHeight-m_nExtentHeight;//60;
				dstRt.height = m_nExtentHeight;//60;
				dstRt.width = 2*uPicWidth;

				cvSetImageROI(pComposePicture,dstRt);
				cvSet(pComposePicture,cvScalar(0,0,0));
				cvResetImageROI(pComposePicture);
				

				if (nType == 0)
				{
				}
				else
				{
					PutTextOnImage(pComposePicture,nPlate,nSpeed,strCrossingNumber);
				}
				

				//编码成JPG图片
				int srcstep = 0;
				CxImage image;
				//image.IppEncode(pImageData,uPicWidth*2,uPicHeight*2,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
				image.IppEncode((unsigned char*)pComposePicture->imageData,pComposePicture->width,pComposePicture->height,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);

				nPlate.uPicWidth = 2*uPicWidth;
				nPlate.uPicHeight = uPicHeight;
				nPlate.uPicSize = srcstep;
				nPlate.uSmallPicSize = 0;
				strComposePicMsg.append((char*)pJpgImage,srcstep);
				
				if (pComposePicture != NULL)
				{
					cvReleaseImage(&pComposePicture);
					pComposePicture = NULL;
				}

				if (pSmalPicture != NULL)
				{
					cvReleaseImageHeader(&pSmalPicture);
					pSmalPicture = NULL;
				}

				if (pBigPicture != NULL)
				{
					cvReleaseImageHeader(&pBigPicture);
					pBigPicture = NULL;
				}

				if (pJpgImage != NULL)
				{
					free(pJpgImage);
					pJpgImage = NULL;
				}
			
		}
	return strComposePicMsg;

}

//将某号牌在上一个点位被抓的图片和本点位抓到的图片解码编码成一副图像(田字形摆放 小大小大)
string CMvsCommunication::ComposePicture(RECORD_PLATE nFromPlate,RECORD_PLATE& nPlate,UINT32& uRegionSpeed,string& strPic1,string& strPic2)
{
	string strComposePicMsg;
	strComposePicMsg.clear();

	CxImage nFromBigImage;
	CxImage nFromSmallImage;
	CxImage nBigImage;
	CxImage nSmallImage;
	int m_nExtentFrom = g_PicFormatInfo.nExtentHeight;
	int m_nExtentTo = g_PicFormatInfo.nExtentHeight;
	int m_nExtentHeight = g_PicFormatInfo.nExtentHeight;

	string nFromPicData = GetImageByPath(nFromPlate.chPicPath);
	/*if(nFromPlate.uPicWidth > 2* nFromPlate.uPicHeight)
	{

	}
	else*/
	{
		nFromBigImage.Decode((BYTE*)(nFromPicData.c_str()), nFromPlate.uPicSize, 3); //上一个点位的大图解码
		int nFromSmallSize = 0;
		string nFromSmallPicData = g_MyCenterServer.GetSmallPicFromBigPic(&nFromPlate,nFromSmallSize);
		if (nFromSmallPicData.size() > 0)
		{
			nFromSmallImage.Decode((BYTE*)(nFromSmallPicData.c_str()),nFromSmallSize,3);
		}
		else
		{
			LogError("获取上一个点位大图图片的小图失败\n");
			return strComposePicMsg;
		}
	}
	
	string nPicdata = GetImageByPath(nPlate.chPicPath);
	/*if(nPlate.uPicWidth > 2* nPlate.uPicHeight)
	{
		
	}
	else*/
	{
		nBigImage.Decode((BYTE*)(nPicdata.c_str()), nPlate.uPicSize, 3);//本点位大图解码
		int nPicSmallSize = 0;
		string nSamllPicData = g_MyCenterServer.GetSmallPicFromBigPic(&nPlate,nPicSmallSize);
		if (nSamllPicData.size() > 0)
		{
			nSmallImage.Decode((BYTE*)(nSamllPicData.c_str()),nPicSmallSize,3);
		}
		else
		{
			LogError("获取本点位大图图片的小图失败\n");
			return strComposePicMsg;
		}
	}
		
		if (nFromBigImage.IsValid() && nFromSmallImage.IsValid() && nBigImage.IsValid() && nSmallImage.IsValid())
		{

				UINT32 uPicWidth1 = nFromBigImage.GetWidth();
				UINT32 uPicHeight1 = nFromBigImage.GetHeight();

				UINT32 uPicWidth2 = nBigImage.GetWidth();
				UINT32 uPicHeight2 = nBigImage.GetHeight();


				//cerr<<"CCCCCCC Width:"<<uPicWidth<<"Height:"<<uPicHeight<<endl;
				CxImage nTmpImage;
				nFromSmallImage.Resample(uPicWidth1,uPicHeight1,1,&nTmpImage);//上一个点位小图设成大图的大小
				if (!nTmpImage.IsValid())
				{
					LogNormal("nTmpImage is InValid\n");
				}
				CxImage nTmpImage1;
				nSmallImage.Resample(uPicWidth2,uPicHeight2,1,&nTmpImage1);//本点位小图设成大图的大小
				if (!nTmpImage1.IsValid())
				{
					LogNormal("nTmpImage1 is InValid\n");
				}

				UINT32 uPicWidth =  uPicWidth1 > uPicWidth2 ? uPicWidth1:uPicWidth2;
				UINT32 uPicHeight =  uPicHeight1 > uPicHeight2 ? uPicHeight1:uPicHeight2;

				if (nFromPlate.uPicHeight == (DSP_200_BIG_HEIGHT + 64))
				{
					m_nExtentFrom = 64;
				}
				else if (nFromPlate.uPicHeight == (DSP_500_BIG_HEIGHT + 96))
				{
					m_nExtentFrom = 96;
				}
				else if (nFromPlate.uPicHeight == (DSP_600_BIG_465_HEIGHT + 112))
				{
					m_nExtentFrom = 112;
				}

				if (nPlate.uPicHeight == (DSP_200_BIG_HEIGHT + 64))
				{
					m_nExtentTo = 64;
				}
				else if (nPlate.uPicHeight == (DSP_500_BIG_HEIGHT + 96))
				{
					m_nExtentTo = 96;
				}
				else if (nPlate.uPicHeight == (DSP_600_BIG_465_HEIGHT + 112))
				{
					m_nExtentTo = 112;
				}

				if (uPicHeight == DSP_200_BIG_HEIGHT)
				{
					m_nExtentHeight = 64;
					uPicHeight += m_nExtentHeight;
				}
				else if(uPicHeight == DSP_500_BIG_HEIGHT)
				{
					m_nExtentHeight = 96;
					uPicHeight += m_nExtentHeight;
				}
				else if(uPicHeight == DSP_600_BIG_465_HEIGHT)
				{
					m_nExtentHeight = 112;
					uPicHeight += m_nExtentHeight;
				}
				else
				{
					uPicHeight += m_nExtentHeight;
				}
				

				//
				UINT32 uImageSize = (2*uPicWidth)*(2*uPicHeight);
				//unsigned char* pImageData = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
				unsigned char* pJpgImage = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));


				IplImage *pToComposePicture = cvCreateImage(cvSize(2*uPicWidth2,uPicHeight2),8,3);
				if (pToComposePicture == NULL)
				{
					LogError("nComPosePicture CreateImage error\n");
					return strComposePicMsg;
				}
				IplImage *pFromComposePicture = cvCreateImage(cvSize(2*uPicWidth1,uPicHeight1),8,3);
				if (pFromComposePicture == NULL)
				{
					LogError("pFromComposePicture CreateImage error\n");
					return strComposePicMsg;
				}

				IplImage *pComposePicture = cvCreateImage(cvSize(2*uPicWidth,2*uPicHeight),8,3);
				if (pComposePicture == NULL)
				{
					LogError("nComPosePicture CreateImage error\n");
					return strComposePicMsg;
				}

				IplImage *pFromSmalPicture = cvCreateImageHeader(cvSize(uPicWidth1,uPicHeight1),8,3);
				cvSetData(pFromSmalPicture,nTmpImage.GetBits(),pFromSmalPicture->widthStep);

				IplImage *pFromBigPicture = cvCreateImageHeader(cvSize(uPicWidth1,uPicHeight1),8,3);
				cvSetData(pFromBigPicture,nFromBigImage.GetBits(),pFromBigPicture->widthStep);

				IplImage *pSmalPicture = cvCreateImageHeader(cvSize(uPicWidth2,uPicHeight2),8,3);
				cvSetData(pSmalPicture,nTmpImage1.GetBits(),pSmalPicture->widthStep);

				IplImage *pBigPicture = cvCreateImageHeader(cvSize(uPicWidth2,uPicHeight2),8,3);
				cvSetData(pBigPicture,nBigImage.GetBits(),pBigPicture->widthStep);

				
				//上一个点位的小图大图的像素拷贝
				CvRect dstRt;
				dstRt.x = 0;
				dstRt.y = 0;
				dstRt.height = uPicHeight1;
				dstRt.width = uPicWidth1;

				cvSetImageROI(pFromComposePicture,dstRt);
				//cerr<<"22222222222222"<<endl;
				cvCopy(pFromSmalPicture,pFromComposePicture);
				cvResetImageROI(pFromComposePicture);

				dstRt.x = uPicWidth1;
				dstRt.y = 0;
				dstRt.height = uPicHeight1;
				dstRt.width = uPicWidth1;

				cvSetImageROI(pFromComposePicture,dstRt);
				cvCopy(pFromBigPicture,pFromComposePicture);
				cvResetImageROI(pFromComposePicture);
				//test
				dstRt.x = 0;
				dstRt.y = uPicHeight1 - m_nExtentFrom;//60;
				dstRt.height = m_nExtentFrom;//60;
				dstRt.width = 2*uPicWidth1;
				cvSetImageROI(pFromComposePicture,dstRt);
				cvSet(pFromComposePicture,cvScalar(0,0,0));
				cvResetImageROI(pFromComposePicture);
				///test

				PutTextOnImage(pFromComposePicture,nFromPlate,nPlate,uRegionSpeed,0);

				//编码成JPG图片
				/*int srcstep = 0;
				CxImage image;
				//image.IppEncode(pImageData,uPicWidth*2,uPicHeight*2,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
				image.IppEncode((unsigned char*)pFromComposePicture->imageData,pFromComposePicture->width,pFromComposePicture->height,3,&srcstep,pJpgImage1,g_PicFormatInfo.nJpgQuality);
				int nstepTmp = srcstep;
				nPlate.uPicWidth = 2*uPicWidth;
				nPlate.uPicHeight = uPicHeight;
				nPlate.uPicSize = srcstep;
				nPlate.uSmallPicSize = 0;
				char buf[8] = {0};
				sprintf(buf,"%07d",srcstep);
				strComposePicMsg.append(buf,7);
				strComposePicMsg.append((char*)pJpgImage1,srcstep);//合成图片1的大小+合成图片1+合成图片2
				*/

				
				//本点位的小图大图的像素拷贝
				CvRect dstRt1;
				dstRt1.x = 0;
				dstRt1.y = 0;
				dstRt1.height = uPicHeight2;
				dstRt1.width = uPicWidth2;

				cvSetImageROI(pToComposePicture,dstRt1);
				cvCopy(pSmalPicture,pToComposePicture);
				cvResetImageROI(pToComposePicture);
				//cerr<<"666666666666666666"<<endl;
				dstRt1.x = uPicWidth2;
				dstRt1.y = 0;
				dstRt1.height = uPicHeight2;
				dstRt1.width = uPicWidth2;

				cvSetImageROI(pToComposePicture,dstRt1);
				cvCopy(pBigPicture,pToComposePicture);
				cvResetImageROI(pToComposePicture);
				dstRt1.x = 0;
				dstRt1.y = uPicHeight2 - m_nExtentTo;//60;
				dstRt1.height = m_nExtentTo;//60;
				dstRt1.width = 2*uPicWidth2;
				cvSetImageROI(pToComposePicture,dstRt1);
				cvSet(pToComposePicture,cvScalar(0,0,0));
				cvResetImageROI(pToComposePicture);
				
				PutTextOnImage(pToComposePicture,nFromPlate,nPlate,uRegionSpeed,1);

				int srcstep = 0;
				CxImage image;
				
				//对于ftp-server方式驶入图片和驶出图片分开存储
				if(g_nServerType == 13 && g_nFtpServer == 1)
				{
					image.IppEncode((unsigned char*)pFromComposePicture->imageData,pFromComposePicture->width,pFromComposePicture->height,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
					strPic1.append((char*)pJpgImage,srcstep);

					srcstep = 0;
					image.IppEncode((unsigned char*)pToComposePicture->imageData,pToComposePicture->width,pToComposePicture->height,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
					strPic2.append((char*)pJpgImage,srcstep);
				}
				else
				{
					//组合4张图
					dstRt.x = 0;
					dstRt.y = 0;
					dstRt.height = uPicHeight;
					dstRt.width = 2*uPicWidth;

					cvSetImageROI(pComposePicture,dstRt);
					if(uPicWidth != uPicWidth1 || uPicHeight != uPicHeight1)
					{
						cvResize(pFromComposePicture,pComposePicture);
					}
					else
					{
						cvCopy(pFromComposePicture,pComposePicture);
					}
					cvResetImageROI(pComposePicture);

					dstRt.x = 0;
					dstRt.y = uPicHeight;
					dstRt.height = uPicHeight;
					dstRt.width = 2*uPicWidth;

					cvSetImageROI(pComposePicture,dstRt);
					if(uPicWidth != uPicWidth2 || uPicHeight != uPicHeight2)
					{
						cvResize(pToComposePicture,pComposePicture);
					}
					else
					{
						cvCopy(pToComposePicture,pComposePicture);
					}
					cvResetImageROI(pComposePicture);

					image.IppEncode((unsigned char*)pComposePicture->imageData,pComposePicture->width,pComposePicture->height,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
					strComposePicMsg.append((char*)pJpgImage,srcstep);
				}

				nPlate.uPicWidth = 2*uPicWidth;
				nPlate.uPicHeight = 2*uPicHeight;
				nPlate.uPicSize = srcstep;
				nPlate.uSmallPicSize = 0;
				/////test
				//char buf22[6] = {0};
				//sprintf(buf22,"%d.jpg",m_Count);
				//string path6 = "/home/road/server/";
				//path6 += buf22;
				//FILE* fp6 = NULL;
				//fp6 = fopen(path6.c_str(),"wb");
				//if (fp6== NULL)
				//{
				//	cerr<<"foepn "<<path6.c_str()<<" error"<<endl;
				//}
				//else
				//{
				//	fwrite(pJpgImage,srcstep,1,fp6);
				//	fclose(fp6);
				//}
				//m_Count += 1;
				////////test
				/*if (pFromSmalPicture != NULL)
				{
					cvReleaseImageHeader(&pFromSmalPicture);
					pFromSmalPicture = NULL;
				}*/

				if (pFromBigPicture != NULL)
				{
					cvReleaseImageHeader(&pFromBigPicture);
					pFromBigPicture = NULL;
				}

				if (pFromSmalPicture != NULL)
				{
					cvReleaseImageHeader(&pFromSmalPicture);
					pFromSmalPicture = NULL;
				}

				if (pFromComposePicture != NULL)
				{
					cvReleaseImage(&pFromComposePicture);
					pFromComposePicture = NULL;
				}

				
				if (pSmalPicture != NULL)
				{
					cvReleaseImageHeader(&pSmalPicture);
					pSmalPicture = NULL;
				}

				if (pBigPicture != NULL)
				{
					cvReleaseImageHeader(&pBigPicture);
					pBigPicture = NULL;
				}

				if (pToComposePicture != NULL)
				{
					cvReleaseImage(&pToComposePicture);
					pToComposePicture = NULL;
				}


				if (pComposePicture != NULL)
				{
					cvReleaseImage(&pComposePicture);
					pComposePicture = NULL;
				}

				if (pJpgImage != NULL)
				{
					free(pJpgImage);
					pJpgImage = NULL;
				}
			
		}
	return strComposePicMsg;

}

//区间速度
float CMvsCommunication::CalRegionSpeed(float distance,UINT32 seond)
{
	if (seond <= 0)
	{
		float no = 0.0;
		LogError("CalRegionSpeed error second:%d\n",seond);
		return no;

	}
	float hour = seond*1.0/3600;
	float speed = (distance/1000)/hour;
	return speed;
}

//发送车牌信息
bool CMvsCommunication::SendPlateMsg(std::string& result)
{
	if (!m_uSendConnectFlag)
	{
		return false;
	}
	if (!mvSendMsgToSocket(m_uSendSocket,result))
	{
		LogNormal("SendPlateMsg: 发送消息失败\n");
		mvCloseSocket(m_uSendSocket);
		m_uSendConnectFlag = false;
		return false;
	}
	return true;
}


//接收某套接的发来的信息
bool CMvsCommunication::RecvFdMsg(int fd,string& buf,int nLength,int& nRecvSize)
{
	bool bRet = true;
	if (fd <= 0)
	{
		LogError("RecvFdMsg fd error\n");
		return false;
	}
	else
	{
		int nRecved = 0;
		int nRet = 0;
		//int nCount = 0;
		char* strMsgBuf = NULL;
		strMsgBuf = (char *)malloc((nLength + 1)*sizeof(char));
		if (strMsgBuf == NULL)
		{
			LogError("1231malloc error\n");
			shutdown(fd,2);
			close(fd);
			fd = 0;
			return false;
		}
		while (nRecved < nLength)
		{
			nRet = recv(fd, strMsgBuf + nRecved, nLength - nRecved, MSG_WAITALL);
			if (nRet <= 0)
			{
				LogError("RecvFdMsg error,nRecved:%d\n",nRecved);
				if (nRet == 0 && errno == 0)
				{
					LogError("RecvFdMsg error cause:%s\n",strerror(errno));
					shutdown(fd,2);
					close(fd);
					fd = 0;
					bRet = false;
					break;
				}
				else
				{
	
					LogError("RecvFdMsg error cause111:%s\n",strerror(errno));
					shutdown(fd,2);
					close(fd);
					fd = 0;
					nRet = false;
					break;
				}
			}
			nRecved += nRet;

		}
		if (nRecved < nLength)
		{
			bRet = false;
		}
		else
		{
			bRet = true;
			buf.append(strMsgBuf,nLength);
		}
		if (strMsgBuf != NULL)
		{
			free(strMsgBuf);
			strMsgBuf = NULL;
		}
		nRecvSize = nRecved;
	}
	return bRet;
}

//接收车牌和图片信息
bool CMvsCommunication::RecvPlateAndPictureMsg(int fd)
{
	char szLength[8] = {0};
	char buff[1024] = {0};
	char strCrossingNumber[6] = {0};
	string plateMsg;
	int plateMsgLength = sizeof(RECORD_PLATE);
	int nRecvSize = 0;
	int nPicSize = 0;
	bool nNormalRecFlag = true; 
	RECORD_PLATE *mvsRecPlate = NULL;
	plateMsg.clear();
	bool nPlateRecvOver = false;
	bool nPicRecvOver = false;
	while(!g_bEndThread)
	{
		nRecvSize = 0;
		mvsRecPlate = NULL;
		nNormalRecFlag = true;
		plateMsg.clear();
		memset(szLength,0,8);
		memset(buff,0,1024);
		memset(strCrossingNumber,0,6);

		if (recv(fd,strCrossingNumber,5,MSG_WAITALL) < 0) //消息长度
		{
			DelClient(fd);
			LogError("RecvPlateAndPictureMsg error\n");
			return false;
		}
		//cerr<<"111111111CrossingNumer:"<<strCrossingNumber<<endl;
		if (recv(fd,szLength,7,MSG_WAITALL) < 0) //消息长度
		{
			DelClient(fd);
			LogError("RecvPlateAndPictureMsg error\n");
			return false;
		}
		//cerr<<"111111111szLength:"<<szLength<<endl;
		if(!RecvFdMsg(fd,plateMsg,plateMsgLength,nRecvSize)) //车牌消息
		{
			DelClient(fd);
			LogError("RecvFdMsg error Fd:%d\n",fd);
			return false;
		}
		else
		{
			nPlateRecvOver = true;
		}
		mvsRecPlate = (RECORD_PLATE *)(plateMsg.c_str());
		nPicSize = atoi(szLength) - plateMsgLength;
		
		//图片消息
		cerr<<"nPicSize:"<<nPicSize<<endl;
		char* nPicMsgBuf = NULL;
		nPicMsgBuf = (char *)malloc((nPicSize+1)*sizeof(char));
		if (nPicMsgBuf == NULL)
		{
			LogError("nPicMsgBuf malloc error\n");
			DelClient(fd);
			return false;
		}
		int nCount = nPicSize/1024;
		int nLeft = nPicSize%1024;
		int i = 0;
		for (i = 0; i < nCount; i++)
		{
			if (recv(fd, nPicMsgBuf + i*1024, 1024, MSG_WAITALL) <= 0)
			{
				LogError("需接收总次数：%d,第%d出错\n",nCount,i+1);
				DelClient(fd);
				nNormalRecFlag = false;
				break;
			}
		}
		if (recv(fd, nPicMsgBuf + i*1024, nLeft, MSG_WAITALL) <= 0)
		{
			LogError("接收剩余数据部分出错\n");
			DelClient(fd);
			nNormalRecFlag = false;
			break;
		}

		if (nNormalRecFlag == false)
		{
			if (nPicMsgBuf != NULL)
			{
				free(nPicMsgBuf);
				nPicMsgBuf = NULL;
			}
			break;
		}
		else
		{
			nPicRecvOver = true;
			if (nPlateRecvOver && nPicRecvOver)
			{
				//将收到的车牌信息存于数据库中
				string strPicPath;
				strPicPath.clear();
				int nRet = GetPicPath(strPicPath);
				bool nSaveFlag = false;
				if (nRet > 0)
				{
					string strPicData = "";
					strPicData.append(nPicMsgBuf,nPicSize);
					nSaveFlag = SavePicData(strPicData,nPicSize,strPicPath);
				}
				if (nSaveFlag)
				{
					string strTmp = mvsRecPlate->chPicPath;
					memset(mvsRecPlate->chPicPath,0,strTmp.size());
					memcpy(mvsRecPlate->chPicPath,strPicPath.c_str(),strPicPath.size());
					g_skpDB.MvsCommunicationSaveRecPlate(*mvsRecPlate);

					char bufTmp[1024];
					memset(bufTmp, 0, 1024);
					sprintf(bufTmp, "update NUMBER_PLATE_INFO_RECV set CrossingNumber = '%s' where ID = %u",strCrossingNumber,mvsRecPlate->uSeq);
					g_skpDB.execSQL(string(bufTmp));
				}
				
			}
			
		}
		if (nPicMsgBuf != NULL)
		{
			free(nPicMsgBuf);
			nPicMsgBuf = NULL;
		}
		nPlateRecvOver = false;
		nPicRecvOver = false;
		usleep(1000*10);
	}
	if (nNormalRecFlag == false)
	{
		return false;
	}
	return true;
}

//保存图片数据
bool CMvsCommunication::SavePicData(string& picData,int PicDataSize,string& strPicPath)
{
	if (picData.size() == 0)
	{
		LogError("picData is NULL\n");
		return false;
	}
	if (strPicPath.size() == 0)
	{
		LogError("strPicPath is NULL");
		return false;
	}

	FILE* fp = NULL;
	fp = fopen(strPicPath.c_str(),"wb");
	if (fp == NULL)
	{
		LogError("Cannot open file %s\n",strPicPath.c_str());
		return false;
	}
	else
	{
		fwrite(picData.c_str(),PicDataSize,1,fp);
		fclose(fp);
		fp = NULL;
	}
	return true;
}

//获取图片路径并将图片编号存储在数据库中
int CMvsCommunication::GetPicPathAndSaveDB(std::string& strPicPath)
{
	pthread_mutex_lock(&g_Id_Mutex);
	////////////////////
	//需要判断磁盘是否已经满
	g_FileManage.CheckDisk(false,false);

	//存储大图片
	strPicPath  = g_FileManage.GetPicPath();

	int nSaveRet = g_skpDB.SavePicID(g_uPicId);
	//解锁
	pthread_mutex_unlock(&g_Id_Mutex);

	if (strPicPath.size() > 0)
	{
		//删除已经存在的记录
		g_skpDB.DeleteOldRecord(strPicPath,false,false);
	}

	return nSaveRet;
}


//获取临时图片路径
int CMvsCommunication::GetPicPath(std::string& strPicPath)
{

	//int nSaveRet = 0;
	pthread_mutex_lock(&g_MvsRecvPicId_Mutex);
	////////////////////
	//需要判断磁盘是否已经满
	//g_FileManage.CheckDisk(false,false);

	//存储大图片
	strPicPath  = g_FileManage.GetRecvPicPath();

	//int nSaveRet = g_skpDB.RegionSpeedSavePicID(g_strMvsRecvPicId,0);
	//加载配置文件
	CXmlParaUtil xml;
	xml.UpdateSystemSetting("RegionSpeedRecPicIdSetting", "");

	//解锁
	pthread_mutex_unlock(&g_MvsRecvPicId_Mutex);

	if (strPicPath.size() > 0)
	{
		//删除已经存在的记录
		g_skpDB.RegionSpeedDeleteOldRecord(strPicPath);
	}
	
	if (!strPicPath.empty())
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}

//根据车牌号码从数据库找对应的记录
bool CMvsCommunication::GetOneRecord(string plateNum,string& strMsg,string& strCrossingNumber)
{
	string nStrTableName = "NUMBER_PLATE_INFO_RECV";

	char buf[1024] = {0};

	sprintf(buf,"select * from %s where NUMBER = '%s' ORDER BY TIME desc limit 1",nStrTableName.c_str(),plateNum.c_str());

	string strSql(buf);
	MysqlQuery q = g_skpDB.execQuery(strSql);

	if (!q.eof())
	{
		RECORD_PLATE plate;

		plate.uSeq = q.getUnIntFileds("ID");
		string strCarNum = q.getStringFileds("NUMBER");
		g_skpDB.UTF8ToGBK(strCarNum);
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

		plate.uColor = q.getIntFileds("COLOR");
		plate.uCredit = q.getIntFileds("CREDIT");
		plate.uRoadWayID = q.getIntFileds("ROAD");

		string strTime = q.getStringFileds("TIME");
		plate.uTime = MakeTime(strTime);
		plate.uMiTime = q.getIntFileds("MITIME");

		plate.uSmallPicSize = q.getIntFileds("SMALLPICSIZE");
		plate.uSmallPicWidth = q.getIntFileds("SMALLPICWIDTH");
		plate.uSmallPicHeight = q.getIntFileds("SMALLPICHEIGHT");

		plate.uPicSize = q.getIntFileds("PICSIZE");
		plate.uPicWidth = q.getIntFileds("PICWIDTH");
		plate.uPicHeight = q.getIntFileds("PICHEIGHT");

		plate.uPosLeft = q.getIntFileds("POSLEFT");
		plate.uPosTop = q.getIntFileds("POSTOP");
		plate.uPosRight = q.getIntFileds("POSRIGHT");
		plate.uPosBottom = q.getIntFileds("POSBOTTOM");

		string strPicPath = q.getStringFileds("PICPATH");
		memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

		//车身颜色，车辆类型，速度，方向,地点等
		plate.uCarColor1 = q.getIntFileds("CARCOLOR");
		plate.uType = q.getIntFileds("TYPE");
		plate.uSpeed = q.getIntFileds("SPEED");
		plate.uWeight1 = q.getIntFileds("CARCOLORWEIGHT");
		plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
		plate.uWeight2 = q.getIntFileds("CARCOLORWEIGHTSECOND");

		plate.uViolationType = q.getIntFileds("PECCANCY_KIND");

		plate.uDirection = q.getIntFileds("DIRECTION");
		plate.uPlateType = q.getIntFileds("CARNUMBER_TYPE");
		string StrVideoPath = q.getStringFileds("VIDEOPATH");
		memcpy(plate.chVideoPath,StrVideoPath.c_str(),StrVideoPath.size());

		string strPlace = q.getStringFileds("PLACE");
		memcpy(plate.chPlace,strPlace.c_str(),strPlace.size());

		strMsg.append((char*)&plate,sizeof(RECORD_PLATE));

		strCrossingNumber = q.getStringFileds("CrossingNumber");
	}
	else
	{
		//LogError("Not find %s from %s\n",plateNum.c_str(),nStrTableName.c_str());
	}

	q.finalize();

	return (!strMsg.empty());
}


void CMvsCommunication::PutTextOnImage(IplImage* pImage,RECORD_PLATE nFromPlate,RECORD_PLATE nPlate,UINT32& uRegionSpeed,int nType)//unsigned char* pImageData,int size,RECORD_PLATE *nPlate)
{
	if (m_ExtentHeight <= 0)
	{
		return;
	}
	wchar_t wchOut[1024] = {'\0'};
	char chOut[1024] = {'\0'};

	int nStartX = 0;
	int nWidth = 10;
	int nHeight = 0;
	int m_nDeinterlace = 1;
	int m_nExtentHeight = m_ExtentHeight;//60;
	int m_nExtentHeightFrom = m_ExtentHeight;
	
	int m_nWordPos = 0;

	if (nFromPlate.uPicHeight == (DSP_500_BIG_HEIGHT + 96))
	{
		m_nExtentHeightFrom = 96;
	}

	if (nFromPlate.uPicHeight == (DSP_200_BIG_HEIGHT + 64))
	{
		m_nExtentHeightFrom = 64;
	}

	if (nFromPlate.uPicHeight == (DSP_600_BIG_465_HEIGHT + 112))
	{
		m_nExtentHeightFrom = 112;
	}

	if (nPlate.uPicHeight == (DSP_500_BIG_HEIGHT + 96))
	{
		m_nExtentHeight = 96;
	}

	if (nPlate.uPicHeight == (DSP_200_BIG_HEIGHT + 64))
	{
		m_nExtentHeight = 64;
	}

	if (nPlate.uPicHeight == (DSP_600_BIG_465_HEIGHT + 112))
	{
		m_nExtentHeight = 112;
	}

	UINT32 uPicHeight = 0;
	UINT32 uPicWidth = 0;

	if (nType == 0)
	{
		uPicWidth = nFromPlate.uPicWidth;
		uPicHeight = nFromPlate.uPicHeight - m_nExtentHeightFrom;
	}
	else if (nType == 1)
	{
		uPicWidth = nPlate.uPicWidth;
		uPicHeight = nPlate.uPicHeight - m_nExtentHeight;
	}

	//if (nFromPlate.uPicHeight >= nPlate.uPicHeight)
	//{
	//	uPicHeight = nFromPlate.uPicHeight;
	//}
	//else
	//{
	//	uPicHeight = nPlate.uPicHeight;
	//}

	//if (nFromPlate.uPicWidth >= nPlate.uPicWidth)
	//{
	//	uPicWidth = nFromPlate.uPicWidth;
	//}
	//else
	//{
	//	uPicWidth = nPlate.uPicWidth;
	//}

	/*if(m_nWordPos == 0)
		nHeight = uPicHeight - m_nExtentHeight;
	else
		nHeight = 0;*/

	CvxText m_cvText;
	//m_cvText.Init(40);
	if (nType == 0)
	{
		m_cvText.Init(m_nExtentHeightFrom);
	}
	else
	{
		m_cvText.Init(m_nExtentHeight);
	}
	
	
	char buf[10] = {0};

	//上一个点位相关信息叠加
	//经过时间
	string strTime("");
	if (nType == 0)
	{
		strTime = GetTime(nFromPlate.uTime,10);
		sprintf(buf,":%03d",nFromPlate.uMiTime);
		strTime += buf;
	}
	else if (nType == 1)
	{
		strTime = GetTime(nPlate.uTime,10);
		sprintf(buf,":%03d",nPlate.uMiTime);
		strTime += buf;
	}
	
	string strText;
	//经过地点
	string m_strFromLocation = nFromPlate.chPlace;
	string m_strLocation = nPlate.chPlace;
	//LogNormal("11111111Location:%s\n",m_strLocation.c_str());
	strText.clear();
	/*if (nPlate.uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
	{*/

	string nstrRegionCode("");
	string nstrRoadName("");
	CXmlParaUtil xml;
	/*PIC_FORMAT_INFO picFormatInfo;
	if(xml.LoadPicFormatInfo(nPlate.uChannelID,picFormatInfo))
	{
		nstrRegionCode = picFormatInfo.nRegionRoadCode;
		
	}*/
	REGION_ROAD_CODE_INFO regionRoadCodeInfo;
	if(xml.LoadPicFormatInfo(nPlate.uChannelID,regionRoadCodeInfo))
	{
		nstrRegionCode = regionRoadCodeInfo.nRegionRoadCode;
		nstrRoadName = regionRoadCodeInfo.chRegionName;

	}
	//驶入点
	if (nType == 0)
	{
		sprintf(chOut,"%s路段 车道 %s%02d 速度:%dkm/h 区间长度:%dm",nstrRoadName.c_str(),nstrRegionCode.c_str(),/*nFromPlate.uRoadWayID*/1,uRegionSpeed,(int)g_MvsDistance);
		nHeight = uPicHeight-5 /*- m_nExtentHeightFrom/2 + 20*/;
	}
	else //驶出点
	{
		sprintf(chOut,"%s路段 车道 %s%02d 速度:%dkm/h 区间长度:%dm",nstrRoadName.c_str(),nstrRegionCode.c_str(),/*nPlate.uRoadWayID*/1,uRegionSpeed,(int)g_MvsDistance);
		nHeight = uPicHeight-5 /*- m_nExtentHeight/2 + 20*/;
	}
	
	//}
	/*else
	{
		sprintf(chOut,"%s  车道:%d	%s ",m_strLocation.c_str(),nFromPlate.uRoadWayID,strTime.c_str());
	}*/
	strText += chOut;

	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,(char*)strText.c_str());

	
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

	//本点位相关信息叠加
	//strTime.clear();
	memset(wchOut,0,sizeof(wchOut));
	memset(chOut,0,sizeof(chOut));
	memset(buf,0,sizeof(buf));
	strText.clear();
	m_strLocation.clear();
	m_strLocation = nPlate.chPlace;
	/*string nDirection("");//路段方向
	string nstrDirection("");//抓拍方向
	nDirection = GetstrDirection(nPlate,nstrDirection);
	if (m_nDetectDirection == 1)
	{
		nstrDirection.clear();
		nstrDirection = nDirection;
	}*/
	std::string strDirection = GetDirection(nPlate.uDirection);
		if (nType == 0)
		{
			sprintf(chOut,"抓拍方向:%s 地点:%s",strDirection.c_str(),m_strFromLocation.c_str());
			nHeight = m_nExtentHeightFrom;
		}
		else if (nType == 1)
		{
			sprintf(chOut,"抓拍方向:%s 地点:%s",strDirection.c_str(),m_strLocation.c_str());
			nHeight = m_nExtentHeight;
		}
		CvxText cvText;
		//cvText.Init(30);
		if (nType == 0)
		{
			cvText.Init(m_nExtentHeightFrom);
		}
		else
		{
			cvText.Init(m_nExtentHeight);
		}
		
		wchar_t wchOutTmp[1024] = {'\0'};
		char chOutTmp[1024] = {'\0'};

		//nHeight = m_nExtentHeight;

		//偏移量
		nWidth += m_nOffsetX;
		nHeight += m_nOffsetY;

		string strTextTmp(chOut);
		UTF8ToUnicode(wchOutTmp,(char*)strTextTmp.c_str());
		cvText.putText(pImage, wchOutTmp, cvPoint(nWidth+uPicWidth, nHeight), CV_RGB(0,255,255));

		
		if (nType == 0)
		{
			sprintf(chOutTmp,"驶入时间:%s",strTime.c_str());
		}
		else if (nType == 1)
		{
			sprintf(chOutTmp,"驶出时间:%s",strTime.c_str());
		}

		if (nType == 0)
		{
			nHeight += m_nExtentHeightFrom;
		}
		else if (nType == 1)
		{
			nHeight += m_nExtentHeight;
		}
		
		strTextTmp.clear();
		strTextTmp = chOutTmp;
		memset(wchOutTmp,0,sizeof(wchOutTmp));
		UTF8ToUnicode(wchOutTmp,(char*)strTextTmp.c_str());
		cvText.putText(pImage, wchOutTmp, cvPoint(nWidth+uPicWidth, nHeight), CV_RGB(0,255,255));

		if (nType == 0)
		{
			nHeight += m_nExtentHeightFrom;
		}
		else if (nType == 1)
		{
			nHeight += m_nExtentHeight;
		}
		/*nHeight += m_nExtentHeight*/;
		memset(chOutTmp,0,sizeof(chOutTmp));
		
		UINT32 nSpeed = GetMaxSpeed(nPlate.uType, nPlate.uChannelID,nPlate.uRoadWayID);
		sprintf(chOutTmp,"限速值:%dkm/h\n",nSpeed);
		strTextTmp.clear();
		strTextTmp = chOutTmp;
		memset(wchOutTmp,0,sizeof(wchOutTmp));
		UTF8ToUnicode(wchOutTmp,(char*)strTextTmp.c_str());
		cvText.putText(pImage, wchOutTmp, cvPoint(nWidth+uPicWidth, nHeight), CV_RGB(0,255,255));
		
		cvText.UnInit();

	m_cvText.UnInit();

}

//在单点超速的图片上叠加文字
void CMvsCommunication::PutTextOnImage(IplImage* pComposePicture,RECORD_PLATE nPlate,UINT32 nSpeed,string& strCrossingNumber)
{
	if (m_ExtentHeight <= 0)
	{
		return;
	}
	wchar_t wchOut[1024] = {'\0'};
	char chOut[1024] = {'\0'};

	int nStartX = 0;
	int nWidth = 10;
	int nHeight = 0;
	int m_nDeinterlace = 1;
	int m_nExtentHeight = m_ExtentHeight;//60;
	int m_nWordPos = 0;

	if (nPlate.uPicHeight ==  (DSP_500_BIG_HEIGHT + 96))
	{
		m_nExtentHeight = 96;
	}
	if (nPlate.uPicHeight == (DSP_200_BIG_HEIGHT + 64))
	{
		m_nExtentHeight = 64;
	}
	if (nPlate.uPicHeight == (DSP_600_BIG_465_HEIGHT + 112))
	{
		m_nExtentHeight = 112;
	}
	UINT32 uPicHeight =  nPlate.uPicHeight - m_nExtentHeight;
	UINT32 uPicWidth = nPlate.uPicWidth;
	
	if (uPicHeight <= 0 || uPicWidth <= 0)
	{
		LogError("1111error,uPicHeight:%d,uPicWidth:%d\n",uPicHeight,uPicWidth);
		return;
	}
	
	if(m_nWordPos == 0)
		nHeight = uPicHeight - m_nExtentHeight;
	else
		nHeight = 0;

	CvxText m_cvText;
	//m_cvText.Init(40);
	m_cvText.Init(m_nExtentHeight);
	//m_cvText.Init(m_FontSize);
	
	char buf[10] = {0};

	//上一个点位相关信息叠加
	//经过时间
	string strTime = GetTime(nPlate.uTime,0);
	sprintf(buf,"%03d",nPlate.uMiTime);
	strTime += ".";
	strTime += buf;
	string strText;
	//经过地点
	string m_strLocation = nPlate.chPlace;
	//LogNormal("11111111Location:%s\n",m_strLocation.c_str());
	strText.clear();

	sprintf(chOut,"%s  车道:%s%02d	%s 速度:%dkm/h",m_strLocation.c_str(),strCrossingNumber.c_str()/*g_strDetectorID.c_str()*/,nPlate.uRoadWayID,strTime.c_str(),nPlate.uSpeed);
	/*else
	{
		sprintf(chOut,"%s  车道:%d	%s ",m_strLocation.c_str(),nFromPlate.uRoadWayID,strTime.c_str());
	}*/
	strText += chOut;

	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,(char*)strText.c_str());

	nHeight = uPicHeight;
	m_cvText.putText(pComposePicture, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	m_cvText.UnInit();

	//本点位相关信息叠加
	strTime.clear();
	memset(wchOut,0,sizeof(wchOut));
	memset(chOut,0,sizeof(chOut));
	memset(buf,0,sizeof(buf));
	strText.clear();
	m_strLocation.clear();
	strTime = GetTime(nPlate.uTime,0);
	sprintf(buf,"%03d",nPlate.uMiTime);
	strTime += ".";
	strTime += buf;
	m_strLocation = nPlate.chPlace;
	{
		CvxText cvText;
		//cvText.Init(30);
		cvText.Init(m_nExtentHeight);
		wchar_t wchOutTmp[1024] = {'\0'};
		char chOutTmp[1024] = {'\0'};
		string strTextTmp;
		
		//偏移量

		nHeight = m_nExtentHeight;

		nWidth += m_nOffsetX;
		nHeight += m_nOffsetY;

		/*string nDirection("");//路段方向
		string nstrDirection("");//抓拍方向
		nDirection = GetstrDirection(nPlate,nstrDirection);
		if (m_nDetectDirection == 1)
		{
			nstrDirection.clear();
			nstrDirection = nDirection;
		}*/
		std::string strDirection = GetDirection(nPlate.uDirection);
		sprintf(chOut,"抓拍方向:%s",strDirection.c_str());//
		string strText(chOut);

		UTF8ToUnicode(wchOutTmp,(char*)strText.c_str());
		cvText.putText(pComposePicture, wchOutTmp, cvPoint(nWidth+uPicWidth, nHeight), CV_RGB(0,255,255));
		nHeight += m_nExtentHeight;

		memset(wchOutTmp,0,sizeof(wchOutTmp));
		UTF8ToUnicode(wchOutTmp,(char*)strTime.c_str());
		cvText.putText(pComposePicture, wchOutTmp, cvPoint(nWidth+uPicWidth, nHeight), CV_RGB(0,255,255));

		nHeight += m_nExtentHeight; 

		sprintf(chOutTmp,"限速:%dkm/h\n",nSpeed);
		strTextTmp.clear();
		strTextTmp = chOutTmp;
		memset(wchOutTmp,0,sizeof(wchOutTmp));
		UTF8ToUnicode(wchOutTmp,(char*)strTextTmp.c_str());
		cvText.putText(pComposePicture, wchOutTmp, cvPoint(nWidth+uPicWidth, nHeight), CV_RGB(0,255,255));
		
		cvText.UnInit();
	}
	

}

//设置字体大小
void CMvsCommunication::SetFontSize(int FontSize)
{
	m_FontSize = FontSize;
	//LogNormal("EEEEEFontSize:%d\n",m_FontSize);
}

//设置文本区域高度
void CMvsCommunication::SetExtentHeight(int nHeight)
{
	m_ExtentHeight = nHeight;
}

//设置推送标志
void CMvsCommunication::SetFlag(bool nFlag) 
{
	m_nConnect = nFlag;
}

//nDirection 为抓拍方向，函数返回路段方向
string CMvsCommunication::GetstrDirection(RECORD_PLATE& nPlate,string& nDirection) 
{
	string strDirection("");
	if (nPlate.uDirection == 1)
	{
		strDirection = "东向西";
		nDirection = "西向东";
	}
	else if (nPlate.uDirection == 2)
	{
		strDirection = "西向东";
		nDirection = "东向西";
	}
	else if (nPlate.uDirection == 3)
	{
		strDirection = "南向北";
		nDirection = "北向南";
	}
	else if (nPlate.uDirection == 4)
	{
		strDirection = "北向南";
		nDirection = "南向北";
	}
	else if (nPlate.uDirection == 5)
	{
		strDirection = "东南向西北";
		nDirection = "西北向东南";
	}
	else if (nPlate.uDirection == 6)
	{
		strDirection = "西北向东南";
		nDirection = "东南向西北";
	}
	else if (nPlate.uDirection == 7)
	{
		strDirection = "东北向西南";
		nDirection = "西南向东北";
	}
	else if (nPlate.uDirection == 8)
	{
		strDirection = "西南向东北";
		nDirection = "东北向西南";
	}
	else
	{
		LogError("1111in CMvsCommunication::GetstrDirection error:%d\n",nPlate.uDirection);
	}
	return strDirection;
}

//设置X,Y偏移量
void CMvsCommunication::SetOffsetXY(int nOffsetX,int nOffsetY)
{
	m_nOffsetX = nOffsetX;
	m_nOffsetY = nOffsetY;
}

//设置抓拍方向
void CMvsCommunication::SetDetectDirection(int nDetectDirection)
{
	m_nDetectDirection = nDetectDirection;
}

//拼区间测速图
string CMvsCommunication::ComposePic1x2(
	RECORD_PLATE nFromPlate,
	RECORD_PLATE& nPlate,
	UINT32& uRegionSpeed)
{
	string strComposePicMsg;
	strComposePicMsg.clear();

	CxImage nFromBigImage;
	CxImage nBigImage;

	int m_nExtentHeight = g_PicFormatInfo.nExtentHeight;
	
	if(nPlate.uPicWidth > 2 * nPlate.uPicHeight)
	{
		LogError("卡口图片叠加方式不正确\n");
		return strComposePicMsg;
	}

	string nFromPicData = GetImageByPath(nFromPlate.chPicPath);
	string nPicdata = GetImageByPath(nPlate.chPicPath);	

	if (g_PicFormatInfo.nSmallPic == 1)
	{
		LogNormal("暂不支持卡口叠加小图方式\n");
		return strComposePicMsg;
	}
	else
	{
		nFromBigImage.Decode((BYTE*)(nFromPicData.c_str()), nFromPlate.uPicSize, 3);
		nBigImage.Decode((BYTE*)(nPicdata.c_str()), nPlate.uPicSize, 3);//本点位大图解码
	}

	if (nFromBigImage.IsValid() && nBigImage.IsValid())
	{
		UINT32 uPicWidth = nBigImage.GetWidth();
		UINT32 uPicHeight = nBigImage.GetHeight();				
		UINT32 uImageSize = (2*uPicWidth)*(uPicHeight);

		UINT32 uFromPicWidth = nFromBigImage.GetWidth();
		UINT32 uFromPicHeight = nFromBigImage.GetHeight();				
		UINT32 uFromImageSize = (2*uFromPicWidth)*(uFromPicHeight);

		//LogNormal("p1: w:%d,h:%d", uPicWidth, uPicHeight);
		//LogNormal("p2: w:%d,h:%d", uFromPicWidth, uFromPicHeight);

		unsigned char* pJpgImage = NULL;
		pJpgImage = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));

		if (pJpgImage == NULL)
		{
			LogError("111111111pIpgImage malloc error\n");
			return strComposePicMsg;
		}

		IplImage *pComposePicture = NULL;
		pComposePicture = cvCreateImage(cvSize(2*uPicWidth,uPicHeight),8,3);
				
		if (pComposePicture == NULL)
		{
			LogError("11111111pComposePicture malloc error");
			return strComposePicMsg;
		}
				
		IplImage *pBigPicture = NULL;
		IplImage *pFromBigPicture = NULL;
		pBigPicture = cvCreateImageHeader(cvSize(uPicWidth,uPicHeight),8,3);
		pFromBigPicture = cvCreateImageHeader(cvSize(uFromPicWidth,uFromPicHeight),8,3);
		if (pBigPicture == NULL)
		{
			LogError("1111111pBigPicture malloc error\n");
			return strComposePicMsg;
		}
		else
		{
			cvSetData(pBigPicture,nBigImage.GetBits(),pBigPicture->widthStep);
		}

		if (pFromBigPicture == NULL)
		{
			LogError("1111111pFromBigPicture malloc error\n");
			return strComposePicMsg;
		}
		else
		{
			cvSetData(pFromBigPicture,nFromBigImage.GetBits(),pFromBigPicture->widthStep);

		}
		
		CvRect srcRt;
		srcRt.x = 0;
		srcRt.y = 0;
		srcRt.width = uFromPicWidth;
		srcRt.height = uFromPicHeight;

		CvRect dstRt;
		dstRt.x = 0;
		dstRt.y = 0;
		dstRt.height = uPicHeight;
		dstRt.width = uPicWidth;

		cvSetImageROI(pFromBigPicture,srcRt);
		cvSetImageROI(pComposePicture,dstRt);
		cvResize(pFromBigPicture, pComposePicture);
		cvResetImageROI(pComposePicture);
		cvResetImageROI(pFromBigPicture);

		dstRt.x = uPicWidth;
		dstRt.y = 0;
		dstRt.height = uPicHeight;
		dstRt.width = uPicWidth;

		cvSetImageROI(pComposePicture,dstRt);
		cvCopy(pBigPicture,pComposePicture);
		cvResetImageROI(pComposePicture);
	
		//黑边叠字
		std::string strCrossingNumber = std::string(nPlate.chText);
		PutTextOnImage1x2(pComposePicture,nFromPlate, nPlate, uRegionSpeed,strCrossingNumber);
	
		int srcstep = 0;

		//图像缩放 
		if(g_PicFormatInfo.nResizeScale < 100 && g_PicFormatInfo.nResizeScale > 0) 
		{ 
			//按比例缩放
			bool bResize = ResizeComposePic(pComposePicture);
			if(!bResize)
			{
				LogError("ComposePic1x2 缩放图片失败!\n");
			}
			else
			{
				//LogNormal("suofang:%d,%d ", pComposePicture->width, pComposePicture->height);
				//编码成JPG图片		
				CxImage image;
				image.IppEncode((unsigned char*)m_imgResize->imageData,m_imgResize->width,\
					m_imgResize->height,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
			}			
		}
		else
		{
			//编码成JPG图片			
			CxImage image;
			image.IppEncode((unsigned char*)pComposePicture->imageData,pComposePicture->width,\
				pComposePicture->height,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
		}
		

		nPlate.uPicWidth = 2*uPicWidth;
		nPlate.uPicHeight = uPicHeight;
		nPlate.uPicSize = srcstep;
		nPlate.uSmallPicSize = 0;
		strComposePicMsg.append((char*)pJpgImage,srcstep);

		if (pComposePicture != NULL)
		{
			cvReleaseImage(&pComposePicture);
			pComposePicture = NULL;
		}

		if (pBigPicture != NULL)
		{
			cvReleaseImageHeader(&pBigPicture);
			pBigPicture = NULL;
		}

		if (pJpgImage != NULL)
		{
			free(pJpgImage);
			pJpgImage = NULL;
		}
	}
	return strComposePicMsg;
}

//区间超速叠加文字
void CMvsCommunication::PutTextOnImage1x2(
	IplImage* pComposePicture,
	RECORD_PLATE fromPlate,
	RECORD_PLATE plate,
	UINT32 uSpeed,
	string& strCrossingNumber)
{
	//LogNormal("fromPlate:%s", fromPlate.chPlace);
	//LogNormal("plate:%s", plate.chPlace);

	if (m_ExtentHeight <= 0)
	{
		return;
	}
	wchar_t wchOut1[1024] = {'\0'};	
	wchar_t wchOut2[1024] = {'\0'};
	
	int nWidth = 10;
	int nHeight = 0;
	int m_nExtentHeight = m_ExtentHeight;
	int m_nWordPos = 0;

	UINT32 uPicHeight =  plate.uPicHeight - m_nExtentHeight;
	UINT32 uPicWidth = plate.uPicWidth;
	
	if (uPicHeight <= 0 || uPicWidth <= 0)
	{
		LogError("1111error,uPicHeight:%d,uPicWidth:%d\n",uPicHeight,uPicWidth);
		return;
	}	

	CvxText cvText;
	//字库初始化
	cvText.Init(m_FontSize);	
	//经过时间
	nHeight = m_nExtentHeight + m_FontSize;

	std::string strText11 = "";
	std::string strText12 = "";	
	std::string strText21 = "";
	std::string strText22 = "";	

	char buf[255] = {0};
	//上一个点位相关信息叠加
	std::string strTime1 = GetTime(fromPlate.uTime,0);
	sprintf(buf,"%03d",fromPlate.uMiTime);

	strTime1 += ".";
	strTime1 += buf;

	memset(wchOut1,0,sizeof(wchOut1));
	UTF8ToUnicode(wchOut1,(char*)strTime1.c_str());	
	cvText.putText(pComposePicture, wchOut1, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));	

	//本地点位相关信息叠加
	string strTime2 = GetTime(plate.uTime,0);
	sprintf(buf,"%03d",plate.uMiTime);
	strTime2 += ".";
	strTime2 += buf;

	memset(wchOut2,0,sizeof(wchOut2));
	UTF8ToUnicode(wchOut2,(char*)strTime2.c_str());
	nWidth += uPicWidth;
	cvText.putText(pComposePicture, wchOut2, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

	//GetText1x2(fromPlate, plate, uSpeed, strText1, strText2);
	GetText1x2(fromPlate, plate, uSpeed, strText11, strText12, strText21, strText22);

	if(m_nWordPos == 0)
		nHeight = uPicHeight - m_nExtentHeight;
	else
		nHeight = 0;

	//偏移量
	nWidth = 10;
	nHeight = m_nExtentHeight/2;
	if(m_nOffsetX > 0 || m_nOffsetY > 0)
	{
		nWidth += m_nOffsetX;
		nHeight += m_nOffsetY;
	}
	int nHeightEx = (int)(nHeight * 0.7f + 0.5f) + 10;
	
	//上一个点位相关信息叠加
	if(strText11.size()>0)
	{
		memset(wchOut1,0,sizeof(wchOut1));
		UTF8ToUnicode(wchOut1,(char*)strText11.c_str());
		cvText.putText(pComposePicture, wchOut1, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	}
	
	if(strText12.size()>0)
	{		
		memset(wchOut2,0,sizeof(wchOut2));
		UTF8ToUnicode(wchOut2,(char*)strText12.c_str());
		cvText.putText(pComposePicture, wchOut2, cvPoint(nWidth, nHeight + nHeightEx), CV_RGB(255,255,255));
	}

	//本地点位相关信息叠加
	nWidth += uPicWidth;	
	if(strText21.size()>0)
	{
		memset(wchOut1,0,sizeof(wchOut1));
		UTF8ToUnicode(wchOut1,(char*)strText21.c_str());
		cvText.putText(pComposePicture, wchOut1, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	}

	if(strText22.size()>0)
	{		
		memset(wchOut2,0,sizeof(wchOut2));
		UTF8ToUnicode(wchOut2,(char*)strText22.c_str());
		cvText.putText(pComposePicture, wchOut2, cvPoint(nWidth, nHeight + nHeightEx), CV_RGB(255,255,255));
	}
	
	//释放字库
	cvText.UnInit();
}

//获取区间测速,1x2格式拼图叠加文字
void CMvsCommunication::GetText1x2(
	const RECORD_PLATE &fromPlate, 
	const RECORD_PLATE &plate, 
	const UINT32 uSpeed,
	std::string &strText11, 
	std::string &strText12,
	std::string &strText21, 
	std::string &strText22)
{		
	char chOut1[1024] = {'\0'};
	char chOut2[1024] = {'\0'};	

	int nOverSped = 0;
	//LogNormal("GetMaxSpeed,%d,%d,%d\n",plate.uType, plate.uChannelID,plate.uRoadWayID);
	UINT32 uLimitSpeed = GetMaxSpeed(plate.uType, plate.uChannelID,plate.uRoadWayID);
	if(uLimitSpeed > 0)
	{
		nOverSped = (uSpeed - uLimitSpeed) * 100 / uLimitSpeed;
	}
	//防伪码
	int nRandCode1 = g_RoadImcData.GetRandCode();
	int nRandCode2 = g_RoadImcData.GetRandCode();

	//经过地点
	string strLocation1 = fromPlate.chPlace;
	string strLocation2 = g_skpDB.GetPlace(plate.uChannelID);

	string strDirection1 = GetDirection(fromPlate.uDirection);
	string strDirection2 = GetDirection(plate.uDirection);

	string strTime1 = GetTime(fromPlate.uTime,0);
	string strTime2 = GetTime(plate.uTime,0);
	
	if(26 == g_nServerType || 23 == g_nServerType)
	{
		string strLocation1 = strLocation2;
		//sprintf(chOut1,"设备编号:%s 路段名称:%s %s 驶入时间:%s 距离:%dm 平均速度:%dkm/h 路段限速:%dkm/h 超速百分比:%d%%",\
		//	g_strDetectorID.c_str(),strLocation1.c_str(),strDirection1.c_str(),strTime1.c_str(), g_DistanceHostInfo.uDistance, uSpeed, uLimitSpeed, nOverSped);
		//sprintf(chOut2,"设备编号:%s 路段名称:%s %s 驶出时间:%s 距离:%dm 平均速度:%dkm/h 路段限速:%dkm/h 超速百分比:%d%%",\
		//	g_strDetectorID.c_str(),strLocation2.c_str(),strDirection2.c_str(),strTime2.c_str(), g_DistanceHostInfo.uDistance, uSpeed, uLimitSpeed, nOverSped);

		sprintf(chOut1,"设备编号:%s 路段名称:%s %s ",\
			g_strDetectorID.c_str(),strLocation1.c_str(),strDirection1.c_str());
		sprintf(chOut2,"设备编号:%s 路段名称:%s %s ",\
			g_strDetectorID.c_str(),strLocation2.c_str(),strDirection2.c_str());
		
		strText11 = chOut1;
		strText21 = chOut2;

		memset(chOut1,0,1024);
		sprintf(chOut1,"驶入时间:%s 距离:%dm 平均速度:%dkm/h 路段限速:%dkm/h 超速百分比:%d%%", \
			strTime1.c_str(), g_DistanceHostInfo.uDistance, uSpeed, uLimitSpeed, nOverSped);
		memset(chOut2,0,1024);
		sprintf(chOut2,"驶出时间:%s 距离:%dm 平均速度:%dkm/h 路段限速:%dkm/h 超速百分比:%d%%", \
			strTime2.c_str(), g_DistanceHostInfo.uDistance, uSpeed, uLimitSpeed, nOverSped);

		strText12 = chOut1;
		strText22 = chOut2;
	}
	else
	{
		sprintf(chOut1,"设备编号:%s 起点地点:%s %s 驶入时间:%s 平均速度:%dkm/h 路段限速:%dkm/h 超速百分比:%d%% 防伪码: %08x",\
			g_strDetectorID.c_str(),strLocation1.c_str(),strDirection1.c_str(),strTime1.c_str(),uSpeed, uLimitSpeed, nOverSped, nRandCode1);
		sprintf(chOut2,"设备编号:%s 终点地点:%s %s 驶出时间:%s 平均速度:%dkm/h 路段限速:%dkm/h 超速百分比:%d%% 防伪码: %08x",\
			g_strDetectorID.c_str(),strLocation2.c_str(),strDirection2.c_str(),strTime2.c_str(),uSpeed, uLimitSpeed, nOverSped, nRandCode2);

		strText11 = chOut1;
		strText21 = chOut2;
	}
}

//按比例缩放图像
bool CMvsCommunication::ResizeComposePic(IplImage* pImg)
{
	bool bRet = false;
	
	if(pImg != NULL) 
	{ 
		if(23 == g_nServerType || 26 == g_nServerType) 
		{ 
			if(pImg->width > 0) 
			{ 
				if(m_imgResize != NULL) 
				{ 
					cvReleaseImage(&m_imgResize); 
					m_imgResize = NULL; 
				}
				m_imgResize = cvCreateImage(cvSize(pImg->width*g_PicFormatInfo.nResizeScale/100.0,pImg->height*g_PicFormatInfo.nResizeScale/100.0),8,3); 
				cvResize(pImg,m_imgResize);

				//LogNormal("pImg:%d,%d ", pImg->width, pImg->height);
				//LogNormal("m_imgResize:%d,%d ", m_imgResize->width, m_imgResize->height);
				bRet = true;
			} 
		}
	}
	return bRet;
}