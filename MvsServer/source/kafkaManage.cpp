
#ifdef KAFKA_SERVER

#include "kafkaManage.h"
#include "Common.h"
#include "CommonHeader.h"
#include "Filemd5.h"
#include "RoadImcData.h"

void* ThreadGetServerList(void* pArg)
{
	CKafakaManage* pKafa = (CKafakaManage*)pArg;
	if(pKafa == NULL)
	{
		return pArg;
	}

	pKafa->GetServerList();

	return pArg;
}

CKafakaManage::CKafakaManage()
{
	m_nThreadId = 0;
	m_bDownload = false;
	m_bServerConnect = false;
	m_bHasMacID = false;
	m_strMacID = "";
	m_doPross = new KafkaProcess();
}

CKafakaManage::~CKafakaManage()
{
	if (m_doPross)
	{
		delete m_doPross;
		m_doPross = NULL;
	}
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}
}

bool CKafakaManage::Init()
{
	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);

	//启动检测结果发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	int nret=pthread_create(&m_nThreadId,&attr,ThreadGetServerList,this);
	//成功
	if(nret!=0)
	{
		LogNormal("CKafakaManage::Init failed\n");
		g_bEndThread = true;
		return false;
	}
	pthread_attr_destroy(&attr);
	return true;
}

string CKafakaManage::GetMacID()
{
	if (m_bHasMacID == false)
	{
		BYTE strID[8];
		unsigned char abyte0[4];

		string strMacID= g_Kafka.chItem;
		string strLeftLine;
		string strFound;
		bool bFound = false;

		int nPos = -1;

		m_strMacID = "";
		if (strMacID.size() >0 )
		{
			nPos = strMacID.find("-");
			while (nPos > 0)
			{
				strLeftLine = "";
				strLeftLine.append(strMacID.c_str()+nPos+1,strMacID.size()-nPos-1);

				strFound = "";
				strFound.append(strMacID.c_str(),nPos);

				intToByte(abyte0,atoi(strFound.c_str()));
				m_strMacID.append((char*)&abyte0[3], 1);

				strMacID = strLeftLine;
				nPos = -1;
				nPos = strMacID.find("-");
				bFound = true;
			}
			if (bFound == true)
			{
				intToByte(abyte0,atoi(strLeftLine.c_str()));
				m_strMacID.append((char*)&abyte0[3], 1);
				m_bHasMacID = true;
			}
		}
	}
	return m_strMacID;
}

void CKafakaManage::GetServerList()
{
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;

	bool bDownFinish = false;
	char strUrl[1024] = {0};

	if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
	{
		LogNormal("ControlServerHost is empty!\n");
		return;
	}
	else
	{
		struct timeval now;
		gettimeofday(&now, NULL);
		UINT32 nts = now.tv_usec/1000;
		//sprintf(strUrl,"http://%s:%d/kafka-config/256-256-256-256-256-256-256-256-CITMS-v2.txt",g_strControlServerHost.c_str(),g_nControlServerPort);
		//sprintf(strUrl,"http://%s:%d/kafka-config/%s-%s-%s.txt",g_strControlServerHost.c_str(),g_nControlServerPort,g_Kafka.chItem,g_Kafka.chBrand,g_Kafka.chVersion); //测试
		sprintf(strUrl,"http://%s:%d/kafka-config/%s/%s/%d%d/%s.txt",g_strControlServerHost.c_str(),g_nControlServerPort,g_Kafka.chVersion,g_Kafka.chBrand,GetTimeStamp(), nts,g_Kafka.chItem); //发布

		//LogNormal("strUrl:%s ", strUrl);
	}

	while(!g_bEndThread)
	{
		time( &now );
		localtime_r( &now,newTime );

		if ((newTime->tm_min%15 == 0) || m_bDownload == false)
		{
			if (bDownFinish == false)
			{
				int nRet = HTTP_DownloadFile(strUrl,!m_bDownload);
				if (nRet == 0)
				{
					SelectServer();
					bDownFinish = true;
				}
				m_bDownload = true;
			}
		}
		else
		{
			bDownFinish = false;
		}

		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
}

void CKafakaManage::SelectServer()
{
	bool bRet = false;
	
	if (IsDiffFile() == true)
	{
		if(access("./config/kafkaServerListlast.txt",F_OK) == 0)
		{
			char strCmd[1024] = {0};
			sprintf(strCmd,"mv ./config/kafkaServerListlast.txt ./config/kafkaServerList.txt");
			system(strCmd);
			while(!bRet)
			{
				if(access("./config/kafkaServerList.txt",F_OK) == 0)
				{
					bRet = GetValidServer("./config/kafkaServerList.txt");
				}
				else
				{
					break;
				}
			}
		}
	}
	else
	{
		if (m_bServerConnect == false)
		{
			while(!bRet)
			{
				if(access("./config/kafkaServerList.txt",F_OK) == 0)
				{
					bRet = GetValidServer("./config/kafkaServerList.txt");
				}
				else
				{
					break;
				}
			}
		}
	}
}

bool CKafakaManage::IsDiffFile()
{
	bool bRet = false;

	if((access("./config/kafkaServerList.txt",F_OK) != 0) ||
		(access("./config/kafkaServerListlast.txt",F_OK) != 0))
	{
		return false;
	}

	string strMd5_1 = GetMd5("./config/kafkaServerList.txt");
	string strMd5_2 = GetMd5("./config/kafkaServerListlast.txt");
	if (strMd5_1 != strMd5_2)
	{
		bRet = true;
	}
	return bRet;
}

bool CKafakaManage::GetValidServer(const char* strFileName)
{
	bool bRet = false;
	ifstream ReadFile;
	unsigned long long nLine = 0;
	string temp;
	ReadFile.open(strFileName,ifstream::in);
	if (ReadFile.fail())
	{
		printf("打开文件失败:%s\n",strFileName);
		return;
	}
	else
	{
		while(getline(ReadFile,temp))
		{
			if (nLine > 0)
			{
				bool bRetTmp = GetIpAndPort(temp);
				if (bRetTmp == true)
				{
					bRet = true;
					break;
				}
				sleep(5*60);
			}
			nLine++;
		}
	}
	ReadFile.close();
	return bRet;
}

bool CKafakaManage::GetIpAndPort(string strLine)
{
	bool bRet = false;
	string strLeftLine = "";
	int nPos1 = -1;

	nPos1 = strLine.find(":");
	if(nPos1 > 0)
	{
		strLeftLine.append(strLine.c_str()+nPos1+1,strLine.size()-nPos1-1);

		nPos1 = -1;
		nPos1 = strLeftLine.find(":");
		if(nPos1 > 0)
		{
			m_strIp = "";
			m_strIp.append(strLeftLine.c_str(),nPos1);

			string strPort = "";
			strPort.append(strLeftLine.c_str()+nPos1+1,strLeftLine.size()-nPos1-1);
			m_nPort = atoi(strPort.c_str());

			bRet = CheckServerStatus(m_strIp.c_str(),m_nPort);
		}
	}
	return bRet;
}

bool CKafakaManage::CheckServerStatus(const char* strIp, int nPort)
{
	bool bRet = false;
	//std::string topic_str = "gongcs";
	char strBroker[1024] = {0};
	
	sprintf(strBroker,"%s:%d",strIp,nPort);
	bRet = m_doPross->Init(strBroker,g_Kafka.chTopic);

	if (bRet == true)
	{
		m_bServerConnect = true;
		LogNormal("Kafka服务器连接成功[%s:%d]\n",strIp,nPort);
	}
	else
	{
		LogNormal("Kafka服务器连接失败[%s:%d]\n",strIp,nPort);
		m_bServerConnect = false;
		m_doPross->UnInit();
	}
	return bRet;
}

bool CKafakaManage::SendMessage(string strMsg)
{
	bool bRet = true;

	if (m_bServerConnect == false)
	{
		return false;
	}

	if (strMsg.size() <= 0)
	{
		return false;
	}
	char strTmpIp[1024] ={0};
	sprintf(strTmpIp,"%s",m_strIp.c_str());
	//LogNormal("CKafakaManage::SendMessage:Size:%d\n",strMsg.size());

	m_doPross->PushMessage(strMsg);

	return bRet;
}

string CKafakaManage::CreateKafkaMsg(RECORD_PLATE* sPlate)
{
	string strRetMsg;
	unsigned char abyte0[4];

	intToByte(abyte0,2);
	strRetMsg.append((char*)&abyte0[3], 1);

	strRetMsg.append(this->GetMacID());

	intToByte(abyte0,2);
	strRetMsg.append((char*)&abyte0[3], 1);

	string strText = g_RoadImcData.GetViolationDataForkafka(*sPlate,"");
	intToByte(abyte0,strText.size());
	strRetMsg.append((char*)&abyte0,4);
	strRetMsg.append(strText);

	if(0 == g_Kafka.uUpdateType)//默认发卡口,文本和图片
	{
		string sBigPicData = GetImageByPath(sPlate->chPicPath);
		intToByte(abyte0,sBigPicData.size());
		strRetMsg.append((char*)&abyte0,4);
		strRetMsg.append(sBigPicData);
	}
	else
	{
		//只发卡口文本,不发图片
	}

	return strRetMsg;
}

#endif