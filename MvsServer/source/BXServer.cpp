#include "BXServer.h"
#include "structdef.h"
#include "Common.h"
#include "CSeekpaiDB.h"
CBXServer g_BXServer;

CBXServer::CBXServer()
{

	m_picWidth = 1920;
	m_picHeight = 1080;
	m_SendRealTimeThrd = 0;

	m_stopSend = 0;

	m_SendHistoryThrd = 0;
}

CBXServer::~CBXServer()
{
	BX_UnInit();
}

int CBXServer::Init()
{
	//初始化宝信SDK
	int iRet = BX_Init();
	if (iRet == 0)
	{
		BX_CB_FUNS funs;
		funs.GetDeviceTime = GetDeviceTime;
		funs.GetDeviceWH = GetDeviceWH;
		funs.SetDeviceTime = SetDeviceTime;
		BX_Reg_CB_FUNS( funs );
	}

	//开启发送实时结果线程

	iRet = pthread_create(&m_SendRealTimeThrd, NULL, CreateSendRealTimeResult, (void*)this);
	if (iRet == 0)
	{
		pthread_mutex_init(&m_ResultLock,NULL);
		LogNormal("[%s]:开启实时数据发送线程\n",__FUNCTION__);
	}
	else
	{
		m_SendRealTimeThrd = 0;
		UnInit();
	}



	//开启发送历史结果线程

	iRet = pthread_create(&m_SendHistoryThrd, NULL, CreateSendHistoryResult, (void*)this);
	if (iRet != 0)
	{
		UnInit();
	}
	else
	{
		LogNormal("[%s]:开启历史数据发送线程\n",__FUNCTION__);
	}

	return iRet;
}

int CBXServer::UnInit()
{
	m_stopSend = 1;

	if (m_SendRealTimeThrd != 0)
	{
		pthread_join(m_SendRealTimeThrd,NULL);
		m_SendRealTimeThrd = 0;
	}
	if (m_SendHistoryThrd != 0)
	{
		pthread_join(m_SendHistoryThrd,NULL);
		m_SendHistoryThrd = 0;
	}

	pthread_mutex_lock(&m_ResultLock);
	m_ResultList.clear();
	pthread_mutex_unlock(&m_ResultLock);
	
	pthread_mutex_destroy(&m_ResultLock);

	LogNormal("[%s]:退出宝信服务\n",__FUNCTION__);

	return BX_UnInit();
}

BX_Int32 CBXServer::GetDeviceTime( BX_Uint64 *pU64msSecond )
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	BX_Uint64 nowtime = tv.tv_sec;
	nowtime = nowtime*1000+tv.tv_usec/1000;
	*pU64msSecond = nowtime;

	printf("[%s]:second=%ld,msecond=%ld\n",__FUNCTION__,tv.tv_sec,tv.tv_usec/1000);


	return 0;
}

BX_Int32 CBXServer::SetDeviceTime( BX_Uint64 U64msSecond )
{
	struct timeval tv;
	tv.tv_sec = U64msSecond/1000;
	tv.tv_usec = U64msSecond%1000 * 1000;
	printf("[%s]:second=%ld,msecond=%ld\n",__FUNCTION__,tv.tv_sec,tv.tv_usec/1000);
	settimeofday(&tv,NULL);

	return 0;
}

BX_Int32 CBXServer::GetDeviceWH( BX_Uint32 *pWidth,BX_Uint32 *pHeight )
{
	*pWidth = 1920;
	*pHeight = 1080;
	return 0;
}

int CBXServer::AddResult( string &result )
{
	if(m_SendRealTimeThrd == 0)
	{
		LogNormal("[%s]:BXServer  is not  start\n",__FUNCTION__);
		return -1;
	}
	int iRet = -1;

	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();
	if (sDetectHeader->uDetectType != MIMAX_EVENT_REP)
	{
		return iRet;
	}
	//数据格式转换
	MIMAX_HEADER mHeader;
	mHeader.uCameraID = g_skpDB.GetCameraID( sDetectHeader->uChannelID);
	mHeader.uCmdID = sDetectHeader->uDetectType;
	mHeader.uCmdLen = result.size()-sizeof(SRIP_DETECT_HEADER);
	string strRecord;
	strRecord.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	strRecord.append(result.c_str()+sizeof(SRIP_DETECT_HEADER),mHeader.uCmdLen);
	//防止过多
	if ( m_ResultList.size()>10)
	{
		printf("记录过多\n");
		m_ResultList.pop_back();
	}
	else
	{
		m_ResultList.push_back(strRecord);

		iRet = 0;
	}
	
	pthread_mutex_unlock(&m_ResultLock);


	return iRet;
}

void* CBXServer::CreateSendRealTimeResult( void *arg )
{
	if (arg == NULL)
	{
		return arg;
	}

	CBXServer *pBXServer = (CBXServer*)arg;
	
	pBXServer->DealRealTimeResult();

	
	return NULL;
}

void CBXServer::DealRealTimeResult()
{

	while( m_stopSend == 0 )
	{

		string result;
		//获取数据
		pthread_mutex_lock(&m_ResultLock);
		if (m_ResultList.size() != 0)
		{

			list<string>::iterator it = m_ResultList.begin();
			if (it->size() != 0)
			{
				//保存数据
				result = *it;
			}
			//删除取出的命令
			m_ResultList.pop_front(); 
		}
		pthread_mutex_unlock(&m_ResultLock);

		sendDataToBX(result,0);
		sleep(1);


	}

	LogNormal("[%s]:退出实时发送线程\n",__FUNCTION__);
}

void* CBXServer::CreateSendHistoryResult( void *arg )
{
	if (arg == NULL)
	{
		return arg;
	}

	CBXServer *pBXServer = (CBXServer*)arg;
	pBXServer->DealHistoryResult();


	return NULL;
}

void CBXServer::DealHistoryResult()
{
	while( m_stopSend == 0 )
	{
		//LogNormal("[%s]:g_nSendHistoryRecord=%d\n",__FUNCTION__,g_nSendHistoryRecord);
		//string result;
		 StrList strListEvent;
		//获取数据
		//发送历史记录
		if(g_nSendHistoryRecord == 1)
		{
			if ( g_skpDB.GetPlateHistoryRecord(strListEvent))
			{
				LogNormal("[%s]:获取一条历史记录\n",__FUNCTION__);
				StrList::iterator it_b = strListEvent.begin();
				while(it_b != strListEvent.end())
				{
					string strEvent("");
					strEvent = *it_b;

					if(sendDataToBX(strEvent,1) == 0)
					{
						break ;
					}
					++it_b;
					//1s发送一条
					sleep(5);

				}
			}
		}

		
		
	}

	LogNormal("[%s]:退出历史发送线程\n",__FUNCTION__);
}

int CBXServer::sendDataToBX( string &result,int dataType )
{

	if (result.size() != 0)
	{
		//转化为宝信数据发送
		BX_HeadData bxHead;
		memset(&bxHead,0,sizeof(bxHead));

		MIMAX_HEADER *pmHeader = (MIMAX_HEADER*)result.c_str();

		RECORD_PLATE* pEvent = (RECORD_PLATE*)(result.c_str()+sizeof(MIMAX_HEADER));
		string strPicPath(pEvent->chPicPath);
		string strPic = GetImageByPath(strPicPath);
		printf("%s,strPicPath=%s\n",__FUNCTION__,strPicPath.c_str());
		bxHead.ui8TotalSnapTimes = 1;

		if (pEvent->uViolationType == DETECT_RESULT_PARKING_VIOLATION)
		{
			bxHead.ui8CurTimes = BX_KAKOU;//未满三分
		}
		else if (pEvent->uViolationType == DETECT_RESULT_NO_TURNAROUND)
		///else if (pEvent->uViolationType != 0) //for test
		{
			bxHead.ui8CurTimes = BX_WEITING;//违章为1
			bxHead.uiIllegalCode = 68;  //禁止调头违法代码68
			//录像文件地址
			//ftp://192.168.200.171/00/02/99.mp4->
			//ftp://road:road@192.168.200.171/home/road/server/video/00/02/99.mp4

			string strVideoPath = "/home/road/server/video";
			//单盘双盘
			if (IsDataDisk())
			{
				strVideoPath="/detectdata/video";
			}

			string strOldFtpPath;
			strOldFtpPath.append(pEvent->chVideoPath);
			LogNormal("[%s]:oldVideoFtpPath=%s",__FUNCTION__,strOldFtpPath.c_str());

			string strftp = "ftp://";
			string strTemp =strftp+ g_ServerHost;

			strOldFtpPath.erase(0,strTemp.size());//"/00/02/99.mp4"
			string ftpuserpwd="road:road@";
			strTemp.clear();
			strTemp = strftp+ftpuserpwd+g_ServerHost+strVideoPath;//ftp://road:road@192.168.200.171

			string strNewFtpPath = strTemp+strOldFtpPath;
			memcpy(bxHead.uiVideoFtpPath,strNewFtpPath.c_str(),strNewFtpPath.size());

			printf("[%s]:BxVideoFtpPath=%s",__FUNCTION__,bxHead.uiVideoFtpPath);
		}
		else
		{
			LogNormal("[%s]:event code error=%d,carnum=%s",__FUNCTION__,pEvent->uViolationType,pEvent->chText);
			return -1;
		}

		bxHead.uiCameraId =pmHeader->uCameraID;
		bxHead.ui8Time =  ((BX_Uint64)pEvent->uTime*1000)+pEvent->uMiTime;
		
		//车牌转GBK
		std::string strCarNum(pEvent->chText);
		if(strCarNum.size() > 0)
		{
			//LogNormal("CSkpChannelCenter::SendTraEvent strCarNum=%s\n",strCarNum.c_str());

			g_skpDB.UTF8ToGBK(strCarNum);
			memset(pEvent->chText,0,MAX_EVENT);
			memcpy(pEvent->chText,strCarNum.c_str(),strCarNum.size());
		}

		memcpy(bxHead.szPlateCode,pEvent->chText,sizeof(bxHead.szPlateCode));
		bxHead.uiEventState = dataType;

		LogNormal("strPic.size() = %d\n",strPic.size());
		int ret = BX_SendImage(&bxHead,(BX_Uint8*)strPic.c_str(),strPic.size());

		if (ret == 0)
		{

			LogNormal("[%s]:%d数据发送成功，更改成[已发送][%s]\n",__FUNCTION__,dataType,pEvent->chText);

			//修改数据库已发送字
			g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP,pEvent->uSeq);

		}
		else
		{
			LogNormal("[%s]:%d数据发送失败，请检查配置carnum%s\n",__FUNCTION__,dataType,pEvent->chText);
			return -1;
		}

	}
	return 0;
}
