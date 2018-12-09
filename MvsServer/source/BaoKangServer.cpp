// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
 *   文件：BaoKangServer.h
 *   功能：乐清宝康电警通讯类
 *   作者：GaoXiang
 *   时间：2012-03-11
**/

#include "Common.h"
#include "CommonHeader.h"
#include "BaoKangServer.h"
#include "RoadImcData.h"
#include "XmlParaUtil.h"
#include "BrandSubSection.h"
#include "CarLabel.h"

BaoKangServer g_BaoKangServer;

//记录发送线程
void* ThreadBKRecorderResult(void* pArg)
{
	//处理一条数据
	g_BaoKangServer.DealResult();
	pthread_exit((void *)0);
	return pArg;
}

//历史记录发送线程
void*ThreadBKHistoryResult(void* pArg)
{
	//处理一条历史数据
	g_BaoKangServer.DealHistoryRecord();
	pthread_exit((void *)0);
	return pArg;
}


//接收消息线程
void* ThreadBKRecvResponese(void* pArg)
{
	g_BaoKangServer.RecvMsg();
	LogError("接收中心端消息线程退出\r\n");
	pthread_exit((void *)0);
	return pArg;
}

//连接中心端并发心跳
void* ThreadBKConnectLinkTest(void* pArg)
{
	g_BaoKangServer.ConnectLinkTest();
	pthread_exit((void *)0);
	return pArg;
}

BaoKangServer::BaoKangServer()
{
    pthread_mutex_init(&m_Result_Mutex, NULL);
	pthread_mutex_init(&m_SendState_Mutex, NULL);

	m_nThreadId = 0;
	m_nHistoryThreadId = 0;
	m_strServerHost = "";
	m_nLinkState = 0;
	//m_uTimeout = time(NULL);
	m_uSecIndex = 0;
	m_nSocket = 0;
}


BaoKangServer::~BaoKangServer()
{
    pthread_mutex_destroy(&m_Result_Mutex);
	pthread_mutex_destroy(&m_SendState_Mutex);
}

//启动侦听服务
bool BaoKangServer::Init()
{
	//////////////////////////
	//启动接收连接线程
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    //启动检测结果发送线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadBKRecorderResult,this);
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}
	//历史记录发送线程
	nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadBKHistoryResult,this);
	if(nret!=0)
	{
		//失败
		LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动接收线程
	pthread_t nThreadId;
	nret=pthread_create(&nThreadId,&attr,ThreadBKRecvResponese,this);
	if(nret!=0)
	{
		//失败
		LogError("创建接收线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}
	
	//启动连接心跳线程
	nret=pthread_create(&nThreadId,&attr,ThreadBKConnectLinkTest,this);
	if(nret!=0)
	{
		//失败
		LogError("创建连接心跳线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);

	//CXmlParaUtil xml;
	//xml.GetMaxSpeed(m_maxSpeedMap, 1);
	//xml.GetMaxSpeedStr(m_maxSpeedMapStr, 1);

	return true;
}

//释放
bool BaoKangServer::UnInit()
{
    //停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}

	if(m_nHistoryThreadId != 0)
	{
		pthread_join(m_nHistoryThreadId,NULL);
		m_nHistoryThreadId = 0;
	}

    m_ChannelResultList.clear();
	return true;
}

//按照协议数据打包
bool BaoKangServer::InPackage(string &msg)
{
	BAOKANG_HEADER *sHeader = (BAOKANG_HEADER*)msg.c_str();
	sHeader->uLength = msg.size() - 1;//1为头结构中的END字节

	char END_buf[2] = {ESC, ESC_END};

	for (int i = 1; i < msg.size(); i++)
	{
		if (END == msg.at(i))
		{
			msg.erase(i, 1);
			msg.insert(i, END_buf, 2);
		}
		else if (ESC == msg.at(i))
		{
			msg.insert(i+1, (char*)&ESC_ESC, 1);
		}
	}
	msg.append((char*)&END, 1);
	return true;
}

//按照协议数据解包
bool BaoKangServer::UnPackage(string &msg)
{
	//删除数据尾的END字节
	if (END == msg.at(msg.size()-1))
		msg.erase(msg.size()-1, 1);
	else
		return false;

	if (END != msg.at(0))
		return false;

	//转换字符串
	for (int i = 1; i < (msg.size() - 1); i++)
	{
		if (ESC == msg.at(i))
		{
			char ch = msg.at(i+1);
			if (ESC_END == ch)
			{
				msg.erase(i, 2);
				msg.insert(i, (char*)&END, 1);
			}
			else if (ESC_ESC == ch)
			{
				msg.erase(i, 2);
				msg.insert(i, (char*)&ESC, 1);
			}
		}
	}
	return true;
}

//连接中心端并发心跳
bool BaoKangServer::ConnectLinkTest()
{
	while (!g_bEndThread)
	{
		if (m_nLinkState < BAOKANG_CONNECTED)
		{
			if (ConnectServer())
			{
				SendLinkTest();
				//m_uTimeout = time(NULL);
			}
		}
		else
		{
			//time_t now = time(NULL);
			//if (now - m_uTimeout > 21)
			//{
			//	cerr<<"now - m_uTimeout > 21"<<endl;
			//	//DelClient();
			//	sleep(1);//在重连之前等待1s
			//	continue;
			//}
			SendLinkTest();
		}
		sleep(10);
	}
	return true;
}

//连接中心端
bool BaoKangServer::ConnectServer()
{
	if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0")
	{
		LogError("\n宝康中心端参数错误:host=%s,port=%d\n", g_strControlServerHost.c_str(), BAOKANG_PORT);
		return false;
	}

	LogNormal("开始连接中心端服务器! host=%s,port=%d\n", g_strControlServerHost.c_str(), BAOKANG_PORT);

	if (!mvPrepareSocket(m_nSocket))
	{
		m_nSocket = 0;
		LogError("\n准备连接宝康套接字失败!\n");
		return false;
	}
	if (!mvWaitConnect(m_nSocket, g_strControlServerHost, BAOKANG_PORT))
	{
		m_nSocket = 0;
		LogError("\n尝试连接宝康中心端失败!\n");
		return false;
	}
	m_nLinkState = BAOKANG_CONNECTED;
	return true;
}

//删除连接
bool BaoKangServer::DelClient()
{
	if(m_nLinkState >= BAOKANG_CONNECTED)
	{
		//清空数据
		pthread_mutex_lock(&m_Result_Mutex);
		m_ChannelResultList.clear();
		pthread_mutex_unlock(&m_Result_Mutex);

		//关闭连接
		mvCloseSocket(m_nSocket);
		m_nSocket = 0;
		m_nLinkState = 0;

		return true;
	}

	return false;
}


//接收消息
void BaoKangServer::RecvMsg()
{
	while(!g_bEndThread)
	{
		if(m_nLinkState >= BAOKANG_CONNECTED)
		{
			int nBytes = 0;
			char chBuffer[SRIP_MAX_BUFFER];

			BAOKANG_HEADER bHeader;
			//接收HTTP 头，一次性接收
			if((nBytes = recv(m_nSocket,(void*)&bHeader,sizeof(bHeader),MSG_NOSIGNAL)) < 0)
			{
				LogError("接收协议头出错，连接断开! socket = %d,%s\r\n", m_nSocket, strerror(errno));
				DelClient();
				continue;
			}

			if((nBytes == 0) && (errno == 0))
			{
				LogNormal("中心端关闭连接, 连接断开!\n");
				DelClient();
				continue;
			}

			//判断结构中的数据长度，小于本身结构的长度，错误
			if(bHeader.uLength < sizeof(bHeader))
			{
				LogError("接收协议头结构错误，连接断开!,%s,nBytes=%d,bHeader.uCmdLen=%d,errno=%d\r\n",strerror(errno),nBytes,bHeader.uLength,errno);
				DelClient();
				continue;
			}

			//保存头
			string response((char*)&bHeader,sizeof(bHeader));

			//接收剩下的数据
			int nLeft = bHeader.uLength - sizeof(bHeader) + END_LEN;

			while(nLeft >  0)
			{
				//接收后面的数据
				if((nBytes = recv(m_nSocket,chBuffer,sizeof(chBuffer)<nLeft?sizeof(chBuffer):nLeft,MSG_NOSIGNAL)) <= 0)
				{
					LogError("接收后续消息出错，连接断开!,%s\r\n",strerror(errno));
					DelClient();
					break;
				}
				//保存数据
				response.append(chBuffer, nBytes);
				nLeft -= nBytes;
			}

			//将命令传送到处理模块,继续处理下一个命令
			if(response.size() == bHeader.uLength + END_LEN)
			{
				UnPackage(response);
				OnMsg(m_nSocket, response);
			}
		}

		usleep(1000);
	}
}

//添加一条命令
bool BaoKangServer::OnMsg(const int nSocket, const string& request)
{
	time_t now = time(NULL);
	BAOKANG_HEADER *sHeader = (BAOKANG_HEADER*)request.c_str();
	int recordId = *(int*)(request.c_str() + sizeof(BAOKANG_HEADER) + DEVID_LENGTH);
	//cerr<<"recordId="<<recordId<<endl;
	map<int, short>::iterator it;

	switch(sHeader->uCmd)
	{
	case ACK_CONN:		//心跳回复
		if(0 == recordId)
			m_nLinkState = BAOKANG_RULES_PIC;
		else
			m_nLinkState = BAOKANG_ALL_PIC;
		break;

	case ACK_RECVDATA:	//车牌回复
		g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, recordId);
		break;

	case ACK_RECVFLOW:	//统计回复
		recordId = recordId / MAX_ROADWAY;

		//更新数据库
		pthread_mutex_lock(&m_SendState_Mutex);
		it = m_mapSendState.find(recordId);
		if (it != m_mapSendState.end())
		{
			//cerr<<"it->second="<<it->second<<endl;
			int roadWayId = recordId % MAX_ROADWAY;
			it->second = it->second & ( ~(1 << roadWayId) );

			if (it->second == 0)
			{
				//cerr<<"DB Update"<<endl;
				g_skpDB.UpdateRecordStatus(MIMAX_STATISTIC_REP, recordId);
				m_mapSendState.erase(it);
			}
		}
		pthread_mutex_unlock(&m_SendState_Mutex);
		break;

	default:
		break;
	}

	//保存当前时间
	//m_uTimeout = time(NULL);
	return true;
}

//处理记录结果
void BaoKangServer::DealResult()
{
    while(!g_bEndThread)
	{
		string response1;

	    //加锁
	    pthread_mutex_lock(&m_Result_Mutex);

		//判断是否有命令
		if(m_ChannelResultList.size()>0)
		{
		    printf("==== BaoKangServer::DealResult()===\n");

			//取最早命令
			BaoKangResultMsg::iterator it = m_ChannelResultList.begin();
			//保存数据
			response1 = *it;
			//删除取出的命令
			m_ChannelResultList.pop_front();
		}
		//解锁
		pthread_mutex_unlock(&m_Result_Mutex);

		//处理消息
		if(response1.size()>0)
		{
			OnResult(response1);
		}
		//100毫秒
		usleep(100000);
	}
}

//添加一条数据
bool BaoKangServer::AddResult(std::string& strMsg)
{
	if (m_nLinkState < BAOKANG_CONNECTED)
		return false;

	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strMsg.c_str();

	switch(sDetectHeader->uDetectType)
	{
	   case MIMAX_PLATE_REP:   //车牌
	   case MIMAX_STATISTIC_REP:  //统计
	        //加锁
	        pthread_mutex_lock(&m_Result_Mutex);
	        if(m_ChannelResultList.size() > 10)//防止堆积的情况发生
	        {
                m_ChannelResultList.pop_back();
	        }
			m_ChannelResultList.push_front(strMsg);
			//解锁
	        pthread_mutex_unlock(&m_Result_Mutex);
			break;
	   default:
			return false;
	}
	return true;
}

//转换表
int BaoKangServer::Transfer(int type, int value)
{
	if (1 == type) //违章类型
	{
		if (value == DETECT_RESULT_RED_LIGHT_VIOLATION)	//闯红灯
			return BAOKANG_RED_LIGHT;
		else if (value == DETECT_RESULT_EVENT_GO_FAST)	//超速
			return BAOKANG_OVERSPEED;
		else if (value == DETECT_RESULT_ALL)	//正常行驶
			return 0;
		else	//违反标志线
			return BAOKANG_LINE;
	}
	else if (2 == type)//车身颜色
	{
		if (value == WHITE)			//白色
			return BAOKANG_WHITE;
		else if(value == SILVER)	//银色
			return BAOKANG_SILVER;
		else if(value == BLACK)     //黑色
			return BAOKANG_BLACK;
		else if(value == RED)       //红色
			return BAOKANG_RED;
		else if(value == PURPLE)    //紫色
			return BAOKANG_PURPLE;
		else if(value == BLUE)      //蓝色
			return BAOKANG_BLUE;
		else if(value == YELLOW)    //黄色
			return BAOKANG_YELLOW;
		else if(value == GREEN)     //绿色
			return BAOKANG_GREEN;
		else if(value == BROWN)     //褐色
			return BAOKANG_BROWN;
		else if(value == GRAY)      //灰色
			return BAOKANG_GRAY;
		else if(value == UNKNOWN)     //未知
			return BAOKANG_UNKNOWN;
	}
	else if (3 == type)//行驶方向
	{
		if (value == SOUTHEAST_TO_NORTHWEST) //由东南到西北
			return EAST_TO_WEST;
		else if (value == NORTHWEST_TO_SOUTHEAST)
			return WEST_TO_EAST;
		else if (value == NORTHEAST_TO_SOUTHWEST)
			return NORTH_TO_SOUTH;
		else if (value == SOUTHWEST_TO_NORTHEAST)
			return SOUTH_TO_NORTH;
	}
	else if (4 == type)//车标
	{
		if (value == VW)
			return BAOKANG_VW;
		else if (value == AUDI)
			return BAOKANG_AUDI;
		else if (value == BENZ)
			return BAOKANG_BENZ;
		else if (value == HONDA)
			return BAOKANG_HONDA;
		else if (value == TOYOTA)
			return BAOKANG_TOYOTA;
		else if (value == HYUNDAI)
			return BAOKANG_HYUNDAI;
		else if (value == BUICK)
			return BAOKANG_BUICK;
		else if (value == RedFlags)
			return BAOKANG_REDFLAGS;
		else if (value == JINBEI)
			return BAOKANG_JINBEI;
		else if (value == XIALI)
			return BAOKANG_XIALI;
		else if (value == FOTON)
			return BAOKANG_FOTON;
		else if (value == CITERON)
			return BAOKANG_CITERON;
		else if (value == MAZDA)
			return BAOKANG_MAZDA;
		else if (value == IVECO)
			return BAOKANG_IVECO;
		else
			return BAOKANG_OTHERS;
	}
	return 0;
}

//将博康车牌结构填入宝康车牌结构体
bool BaoKangServer::PlateCopy(BAOKANG_PLATE& bPlate, RECORD_PLATE* sPlate)
{
	memcpy(bPlate.Rec.chDevId, g_strDetectorID.c_str(), 10);
	bPlate.Rec.uIndex = sPlate->uSeq;
	memcpy(bPlate.Rec.chText, sPlate->chText, MAX_PLATE);

	//cerr<<"|| "<<bPlate.Rec.chText<<endl;
	bPlate.Rec.uTimeS = sPlate->uTime;
	bPlate.Rec.uTimeMs = sPlate->uMiTime;
	bPlate.Rec.speed = sPlate->uSpeed;
	bPlate.Rec.uViolationType = Transfer(1, sPlate->uViolationType);
	bPlate.Rec.uRoadWayId = sPlate->uRoadWayID;

	//map<UINT32, UINT32>::iterator it = m_maxSpeedMap.find(sPlate->uRoadWayID);
	//if(it != m_maxSpeedMap.end())
	//	bPlate.Rec.uMaxSpeed = it->second;
	bPlate.Rec.uMaxSpeed = GetMaxSpeed(sPlate->uType, sPlate->uChannelID, sPlate->uRoadWayID);

	bPlate.Rec.uCarCor = Transfer(2, sPlate->uCarColor1);
	bPlate.Rec.uRoadWayType = g_nRoadType>0 ? 1:0;

	bPlate.Rec.uCarType = sPlate->uType;
	bPlate.Rec.uDirection = Transfer(3, sPlate->uDirection);
	bPlate.Rec.uCarBrand = Transfer(4, sPlate->uCarBrand);
	if (sPlate->uViolationType == DETECT_RESULT_ALL)
		bPlate.Rec.uPicType = 1001;//1张图片
	else
		bPlate.Rec.uPicType = 1004;//4x1图片
	

	memcpy(bPlate.chDevId, g_strDetectorID.c_str(), 10);
	return true;
}

//处理检测结果
bool BaoKangServer::OnResult(std::string& result)
{
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();

	switch(sDetectHeader->uDetectType)
	{
		case MIMAX_PLATE_REP:  //车牌
			{
			RECORD_PLATE *sPlate = (RECORD_PLATE*)(result.c_str() + sizeof(SRIP_DETECT_HEADER));
			return SendPlate(sPlate);
			}

		case MIMAX_STATISTIC_REP:  //统计
			{
			RECORD_STATISTIC *sStatistic = (RECORD_STATISTIC*)(result.c_str() + sizeof(SRIP_DETECT_HEADER));
			return SendStatistic(sStatistic);
			}
	}
    return false;
}

//发送心跳
bool BaoKangServer::SendLinkTest()
{
	BAOKANG_HEADER header;
	header.uCmd = ASK_CONN;

	char buf[DEVID_LENGTH] = {0};
	memcpy(buf, g_strDetectorID.c_str(), 10);

	string strMsg((char*)&header, sizeof(BAOKANG_HEADER));
	strMsg.append(buf, sizeof(buf));

	int intAddr[4];
	int ipAddr;
	string tmp = g_ServerHost + "|";
	sscanf(tmp.c_str(), "%d.%d.%d.%d|", &intAddr[3], &intAddr[2], &intAddr[1], &intAddr[0]);

	for (int i = 3; i >= 0; i--)
	{
		ipAddr = ipAddr | (intAddr[i] << i*8);
	}
	strMsg.append((char*)&ipAddr, sizeof(int));
	SendMsg(strMsg);
	return true;
}

//发送车牌数据
bool BaoKangServer::SendPlate(RECORD_PLATE *sPlate)
{
	BAOKANG_PLATE bPlate;
	PlateCopy(bPlate, sPlate);

	BAOKANG_HEADER header;
	header.uCmd = REALTIME_DATA;

	if(g_nSendImage)
	{
		//取图片
		string strPath(sPlate->chPicPath);
		string pic = GetImageByPath(strPath);

		//分包发送,每个包不超过7K
		int packCount = pic.size()/BAOKANG_PACKAGE_SIZE;
		if (0 != pic.size()%BAOKANG_PACKAGE_SIZE)
			packCount++;

		//cerr<<"uIndex="<<bPlate.Rec.uIndex<<endl;
		//cerr<<"uPackageNum="<<packCount<<endl;

		bPlate.uPicNum = 1;
		bPlate.uCurPicIndex =1;
		bPlate.uPackageNum = packCount;

		int left = pic.size();
		for (int i = 1; i <= packCount; i++)
		{
			int onceBytes = left>BAOKANG_PACKAGE_SIZE ? BAOKANG_PACKAGE_SIZE:left;
			left -= onceBytes;

			bPlate.uCurPackageIndex = i;
			bPlate.uCurPicSize = onceBytes;

			string strMsg;
			strMsg.append((char*)&header, sizeof(BAOKANG_HEADER));
			strMsg.append((char*)&bPlate, sizeof(BAOKANG_PLATE));
			strMsg.append(pic.c_str() + (i-1)*BAOKANG_PACKAGE_SIZE, onceBytes);

			if (SendMsg(strMsg) == false)
			{
				return false;
			}
			usleep(BAOKANG_SLEEP);
		}
	}
	return true;
}

//发送统计数据
bool BaoKangServer::SendStatistic(RECORD_STATISTIC *sStatistic)
{
	BAOKANG_STATISTIC bStatistic;
	memcpy(bStatistic.chDevId, g_strDetectorID.c_str(), 10);
	bStatistic.uTime = sStatistic->uTime;
	bStatistic.uTimeLength = sStatistic->uStatTimeLen;

	BAOKANG_HEADER header;
	header.uCmd = LANE_REAL_FLOW;

	string strMsg;
	short sendState = 0; //有效车道位
	for (int i = 0; i < MAX_ROADWAY; i++)
	{
		if (sStatistic->uOccupancy[i] != 0xFFFFFFFF)
		{
			//cerr<<"SendStatistic(): "<<endl;
			//cerr<<"road[i]="<<i<<endl;
			//cerr<<"sStatistic->uSeq="<<sStatistic->uSeq<<endl;
			
			//有效的车道
			bStatistic.uIndex = sStatistic->uSeq * MAX_ROADWAY + i;
			bStatistic.uRoadWay = sStatistic->uRoadType[i] >> 16;
			bStatistic.uOccupancy = (float)sStatistic->uOccupancy[i]/100;
			bStatistic.uSmallCarNum = sStatistic->uFluxCom[i] & 0xFFFF;
			bStatistic.uMiddleCarNum = sStatistic->uFluxCom[i] >> 16;
			bStatistic.uBigCarNum = sStatistic->uFlux[i] >> 16;
			bStatistic.uAverageSpeed = sStatistic->uSpeed[i];

			sendState = sendState | (1<<i);

			//cerr<<"bStatistic.uIndex="<<bStatistic.uIndex<<endl;
			//cerr<<"bStatistic.uRoadWay="<<bStatistic.uRoadWay<<endl;
			//cerr<<"bStatistic.uOccupancy="<<bStatistic.uOccupancy<<endl;
			//cerr<<"bStatistic.uSmallCarNum="<<bStatistic.uSmallCarNum<<endl;
			//cerr<<"bStatistic.uMiddleCarNum="<<bStatistic.uMiddleCarNum<<endl;
			//cerr<<"bStatistic.uBigCarNum="<<bStatistic.uBigCarNum<<endl;
			//cerr<<"bStatistic.uAverageSpeed="<<bStatistic.uAverageSpeed<<endl;
			//cerr<<"sendState="<<sendState<<endl;


			strMsg.append((char*)&header, sizeof(BAOKANG_HEADER));
			strMsg.append((char*)&bStatistic, sizeof(BAOKANG_STATISTIC));
			if (SendMsg(strMsg) == false)
			{
				return false;
			}
		}

		pthread_mutex_lock(&m_SendState_Mutex);
		m_mapSendState.insert(make_pair(sStatistic->uSeq, sendState));
		pthread_mutex_unlock(&m_SendState_Mutex);

		usleep(BAOKANG_SLEEP);
	}
	return true;
}

//发送记录到中心
bool BaoKangServer::SendMsg(string &strMsg)
{
	InPackage(strMsg);
	if(!mvSendMsgToSocket(m_nSocket, strMsg))
	{
		LogError("向中心端发送失败, 连接断开!\n");
		DelClient();
		return false;
	}
	return true;
}

//处理历史数据
void BaoKangServer::DealHistoryRecord()
{
	while(!g_bEndThread)
	{
		if(g_nSendHistoryRecord == 1 && m_nLinkState >= BAOKANG_CONNECTED)
		{
				//车牌记录
				StrList strListRecord;
				strListRecord.clear();
				if(g_skpDB.GetPlateHistoryRecord(strListRecord))
				{
					StrList::iterator it_b = strListRecord.begin();
					while(it_b != strListRecord.end())
					{
						string strPlate("");
						strPlate = *it_b;
						
						RECORD_PLATE *sPlate = (RECORD_PLATE*)(strPlate.c_str() + sizeof(MIMAX_HEADER));
						SendPlate(sPlate);

						sleep(5);
						it_b++;
					}
					
				}
				else
				{
					sleep(60);
				}
		}
		else
		{
				sleep(5);
		}
	}
}
