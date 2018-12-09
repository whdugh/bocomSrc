#include "MatchPlate.h"

CMatchPlate g_matchPlate;

int CMatchPlate::m_exit = 0;

CMatchPlate::CMatchPlate()
{
	pthread_mutex_init(&m_BackPlate_Mutex,NULL);
	pthread_mutex_init(&m_FrontPlate1_Mutex,NULL);
	pthread_mutex_init(&m_FrontPlate2_Mutex,NULL);

	m_FrontPlateList1.clear();
	m_FrontPlateList2.clear();

	m_BackPlateList.clear();

	m_pDataProc200W = NULL;
	m_pDataProc500W = NULL;
}

CMatchPlate::~CMatchPlate()
{
	CMatchPlate::m_exit = 1;
	pthread_mutex_destroy(&m_BackPlate_Mutex);
	pthread_mutex_destroy(&m_FrontPlate1_Mutex);
	pthread_mutex_destroy(&m_FrontPlate2_Mutex);

	if (m_pDataProc200W)
	{
		delete m_pDataProc200W;
		m_pDataProc200W = NULL;
	}
	if (m_pDataProc500W)
	{
		delete m_pDataProc500W;
		m_pDataProc500W = NULL;
	}
}


int CMatchPlate::InitMatchPlate()
{
	LogNormal("---InitMatchPlate()---\n");
	pthread_t m_hId = 0;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	if(pthread_create(&m_hId, &attr, BeginCheckPlate, this) != 0)
	{
		printf("创建线程失败。");
		return -1;
	}
	pthread_attr_destroy(&attr);
	
	return 0;
}

threadfunc_t STDPREFIX CMatchPlate::BeginCheckPlate(threadparam_t lpParam)
{
	CMatchPlate *pThis = (CMatchPlate *)lpParam;
	if (pThis)
	{
		pThis->CheckPlate();
	}
	return NULL;
}

void CMatchPlate::CheckPlate()
{
	bool bHas = false;

	while(CMatchPlate::m_exit == 0)
	{
		bHas = false;
		RECORD_PLATE_DSP_MATCH record_plate;
		memset(&record_plate,0x00, sizeof(RECORD_PLATE_DSP_MATCH));

		pthread_mutex_lock(&m_BackPlate_Mutex);
		if (m_BackPlateList.size() > 0)
		{
			std::list<RECORD_PLATE_DSP_MATCH>::iterator it = m_BackPlateList.begin();
			record_plate = *it;
			//删除取出的数据
			m_BackPlateList.pop_front();
			bHas = true;
		}
		pthread_mutex_unlock(&m_BackPlate_Mutex);
		
		if (bHas == true)
		{
			DealPlate(record_plate);
		}

		usleep(1000*10);
	}
}


// 插入后排检测结果到待检查List
void CMatchPlate::PushBackPlate(RECORD_PLATE_DSP_MATCH data)
{
	pthread_mutex_lock(&m_BackPlate_Mutex);
	m_BackPlateList.push_back(data);
	pthread_mutex_unlock(&m_BackPlate_Mutex);
	//LogNormal("BackPlate:%d\n", m_BackPlateList.size());
}

// 插入前排检测结果到list
void CMatchPlate::PushFrontPlate(int nChannel, RECORD_PLATE_DSP_MATCH data)
{
	/*if (m_pDataProc500W)
		m_pDataProc500W->SaveImgTest(data.pImg,data.dspRecord.chText);*/

	if (nChannel == 1) //车道1
	{
		pthread_mutex_lock(&m_FrontPlate1_Mutex);
		if (m_FrontPlateList1.size() > 100)
		{
			m_FrontPlateList1.pop_front();
		}
		m_FrontPlateList1.push_back(data);
		pthread_mutex_unlock(&m_FrontPlate1_Mutex);
	}
	else if (nChannel == 2) //车道2
	{
		pthread_mutex_lock(&m_FrontPlate2_Mutex);
		if (m_FrontPlateList2.size() > 100)
		{
			m_FrontPlateList2.pop_front();
		}
		m_FrontPlateList2.push_back(data);
		pthread_mutex_unlock(&m_FrontPlate2_Mutex);
	}
	//LogNormal("Front1:%d Front2:%d\n", m_FrontPlateList1.size(),m_FrontPlateList2.size());
}

//处理后排检查结果
void CMatchPlate::DealPlate(RECORD_PLATE_DSP_MATCH record_plate)
{
	int bFound = -1;
	RECORD_PLATE_DSP_MATCH foundRecord;
	memset(&foundRecord, 0x00, sizeof(RECORD_PLATE_DSP_MATCH));

	if (record_plate.dspRecord.uRoadWayID == 1)
	{
		bFound = SearchFrontPlate1(record_plate, foundRecord);
		if (bFound == -1)
		{
			bFound = SearchFrontPlate2(record_plate,foundRecord);
		}
	}
	else if (record_plate.dspRecord.uRoadWayID == 2)
	{
		bFound = SearchFrontPlate2(record_plate, foundRecord);
		if (bFound == -1)
		{
			bFound = SearchFrontPlate1(record_plate,foundRecord);
		}
	}
	
	if (bFound == -1)
	{
		//LogNormal("MatchPlate: Not Found\n");
	}
	else if (bFound == 0)
	{
		OutPutPlate(record_plate, foundRecord);
		//LogNormal("MatchPlate: found and Alarm\n");
	}
	else if (bFound == 1)
	{
		//LogNormal("MatchPlate: found and not alarm\n");
	}
	
//	OutPutPlate(record_plate, record_plate);
}

// 返回值：-1:没有找到；0:找到；1：找到但都有车牌
int CMatchPlate::SearchFrontPlate1(RECORD_PLATE_DSP_MATCH record_plate, RECORD_PLATE_DSP_MATCH& foundRecord)
{
	int bFound = -1;
	pthread_mutex_lock(&m_FrontPlate1_Mutex);
	for (std::list<RECORD_PLATE_DSP_MATCH>::iterator it = m_FrontPlateList1.begin(); it != m_FrontPlateList1.end();it++)
	{
		RECORD_PLATE dspRecord = ((RECORD_PLATE_DSP_MATCH)*it).dspRecord;

		// 如果都有车牌，先把识别出的前车牌去除
		if ((record_plate.dspRecord.chText[0] != '*') &&
			((strcmp(record_plate.dspRecord.chText, dspRecord.chText) == 0) || // 前后都识别出车牌，且一致
			(record_plate.dspRecord.chText[2] == dspRecord.chText[2] &&
			record_plate.dspRecord.chText[3] == dspRecord.chText[3] &&
			record_plate.dspRecord.chText[4] == dspRecord.chText[4]) ||
			(record_plate.dspRecord.chText[3] == dspRecord.chText[3] &&
			record_plate.dspRecord.chText[4] == dspRecord.chText[4] &&
			record_plate.dspRecord.chText[5] == dspRecord.chText[5]) ||
			(record_plate.dspRecord.chText[4] == dspRecord.chText[4] &&
			record_plate.dspRecord.chText[5] == dspRecord.chText[5] &&
			record_plate.dspRecord.chText[6] == dspRecord.chText[6]) ||
			(record_plate.dspRecord.chText[5] == dspRecord.chText[5] &&
			record_plate.dspRecord.chText[6] == dspRecord.chText[6] &&
			record_plate.dspRecord.chText[7] == dspRecord.chText[7])))// 后面6位只要3位相同就认为是相同车牌
		{
			m_FrontPlateList1.erase(it);
			bFound = 1;
			break;
		}
		else if (record_plate.dspRecord.chText[0] == '*')//只检查没有识别出的后车牌
		{
			if (IsLastPlate(dspRecord.uTime) == false)
			{
				continue;
			}
			if (record_plate.dspRecord.uType == dspRecord.uType)// 检查车型
			{
				if ((record_plate.dspRecord.uCarColor1 == dspRecord.uCarColor1) ||
					(record_plate.dspRecord.uCarColor1 == dspRecord.uCarColor2) ||
					(record_plate.dspRecord.uCarColor2 == dspRecord.uCarColor1) ||
					(dspRecord.uCarColor2 != 11 && record_plate.dspRecord.uCarColor2 == dspRecord.uCarColor2))
				{
					foundRecord = (RECORD_PLATE_DSP_MATCH)(*it);
					m_FrontPlateList1.erase(it);
					bFound = 0;
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&m_FrontPlate1_Mutex);
	return bFound;
}


int CMatchPlate::SearchFrontPlate2(RECORD_PLATE_DSP_MATCH record_plate, RECORD_PLATE_DSP_MATCH& foundRecord)
{
	int bFound = -1;
	pthread_mutex_lock(&m_FrontPlate2_Mutex);
	for (std::list<RECORD_PLATE_DSP_MATCH>::iterator it = m_FrontPlateList2.begin(); it != m_FrontPlateList2.end();it++)
	{
		RECORD_PLATE dspRecord = ((RECORD_PLATE_DSP_MATCH)*it).dspRecord;
		// 如果都有车牌，先把识别出的前车牌去除
		if ((record_plate.dspRecord.chText[0] != '*') &&
			((strcmp(record_plate.dspRecord.chText, dspRecord.chText) == 0) || // 前后都识别出车牌，且一致
			(record_plate.dspRecord.chText[2] == dspRecord.chText[2] &&
			record_plate.dspRecord.chText[3] == dspRecord.chText[3] &&
			record_plate.dspRecord.chText[4] == dspRecord.chText[4]) ||
			(record_plate.dspRecord.chText[3] == dspRecord.chText[3] &&
			record_plate.dspRecord.chText[4] == dspRecord.chText[4] &&
			record_plate.dspRecord.chText[5] == dspRecord.chText[5]) ||
			(record_plate.dspRecord.chText[4] == dspRecord.chText[4] &&
			record_plate.dspRecord.chText[5] == dspRecord.chText[5] &&
			record_plate.dspRecord.chText[6] == dspRecord.chText[6]) ||
			(record_plate.dspRecord.chText[5] == dspRecord.chText[5] &&
			record_plate.dspRecord.chText[6] == dspRecord.chText[6] &&
			record_plate.dspRecord.chText[7] == dspRecord.chText[7])))// 后面6位只要3位相同就认为是相同车牌
		{
			m_FrontPlateList2.erase(it);
			bFound = 1;
			break;
		}
		else if (record_plate.dspRecord.chText[0] == '*')//只检查没有识别出的后车牌
		{
			if (IsLastPlate(dspRecord.uTime) == false)
			{
				continue;
			}
			if (record_plate.dspRecord.uType == dspRecord.uType)// 检查车型
			{
				if ((record_plate.dspRecord.uCarColor1 == dspRecord.uCarColor1) ||
					(record_plate.dspRecord.uCarColor1 == dspRecord.uCarColor2) ||
					(record_plate.dspRecord.uCarColor2 == dspRecord.uCarColor1) ||
					(dspRecord.uCarColor2 != 11 && record_plate.dspRecord.uCarColor2 == dspRecord.uCarColor2))
				{
					foundRecord = (RECORD_PLATE_DSP_MATCH)(*it);
					m_FrontPlateList2.erase(it);
					bFound = 0;
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&m_FrontPlate2_Mutex);
	return bFound;
}

void CMatchPlate::OutPutPlate(RECORD_PLATE_DSP_MATCH record_plate, RECORD_PLATE_DSP_MATCH foundRecord)
{
	#ifndef ALGORITHM_YUV
	//LogTrace("DspPlateMatch.txt", \
	//	"1 OutPutPlate-seq-%d,uChannel=%d-nRoad=%d---plate.uDirection=%d-chText=%s-img=%x c1:%d c2:%d \n", \
	//	record_plate.dspRecord.uSeq, record_plate.dspRecord.uChannelID, record_plate.dspRecord.uRoadWayID,record_plate.dspRecord.uDirection, record_plate.dspRecord.chText, record_plate.pImg, record_plate.dspRecord.uCarColor1, record_plate.dspRecord.uCarColor2);	
	//LogTrace("DspPlateMatch.txt", \
	//	"2 OutPutPlate-seq-%d,uChannel=%d-nRoad=%d---plate.uDirection=%d-chText=%s-img=%x c1:%d c2:%d \n", \
	//	foundRecord.dspRecord.uSeq, foundRecord.dspRecord.uChannelID, foundRecord.dspRecord.uRoadWayID,foundRecord.dspRecord.uDirection, foundRecord.dspRecord.chText, foundRecord.pImg, foundRecord.dspRecord.uCarColor1, foundRecord.dspRecord.uCarColor2);	

	//LogNormal("--OutPutPlate- seq[%d:%d][%s][%s]\n", record_plate.dspRecord.uSeq,foundRecord.dspRecord.uSeq,record_plate.dspRecord.chText, foundRecord.dspRecord.chText);
//	LogNormal("****************W:%d H:%d\n",record_plate.dspRecord.uPicWidth,record_plate.dspRecord.uPicHeight);
	//LogNormal("Color1:[%d:%d]\n",record_plate.dspRecord.uCarColor1,foundRecord.dspRecord.uCarColor1);
	//LogNormal("Color2:[%d:%d]\n",record_plate.dspRecord.uCarColor2,foundRecord.dspRecord.uCarColor2);
	RECORD_PLATE plate;

	//记录合成
	if (record_plate.dspRecord.uPicWidth == DSP_500_BIG_WIDTH_SERVER)// && record_plate.dspRecord.uPicHeight == (DSP_500_BIG_HEIGHT_SERVER + 80))
	{
		if(NULL == m_pDataProc500W)
		{
			m_pDataProc500W = new CDspDataProcess(NULL);
			m_pDataProc500W->Init(1,DSP_500_BIG_WIDTH_SERVER,DSP_500_BIG_HEIGHT_SERVER,DSP_500_BIG_WIDTH_SERVER,DSP_500_BIG_HEIGHT_SERVER);
		}
		//m_pDataProc500W->SaveImgTest(record_plate.pImg,record_plate.dspRecord.chText);
		//m_pDataProc500W->SaveImgTest(foundRecord.pImg,foundRecord.dspRecord.chText);

		printf("-------m_pDataProc->OutPutVtsMatch-----1111----\n");
		m_pDataProc500W->OutPutVtsMatch(record_plate, foundRecord,plate);
		printf("-------m_pDataProc->OutPutVtsMatch-----2222----\n");
	}
	else if (record_plate.dspRecord.uPicWidth == DSP_200_BIG_WIDTH_SERVER)// && record_plate.dspRecord.uPicHeight == (DSP_200_BIG_HEIGHT_SERVER + 80))
	{
		if(NULL == m_pDataProc200W)
		{
			m_pDataProc200W = new CDspDataProcess(NULL);
			m_pDataProc200W->Init(1,DSP_200_BIG_WIDTH_SERVER,DSP_200_BIG_HEIGHT_SERVER,DSP_200_BIG_WIDTH_SERVER,DSP_200_BIG_HEIGHT_SERVER);
		}
		printf("-------m_pDataProc->OutPutVtsMatch-----1111----\n");
		m_pDataProc200W->OutPutVtsMatch(record_plate, foundRecord,plate);
		printf("-------m_pDataProc->OutPutVtsMatch-----2222----\n");
	}
	//usleep(1000*2000);
	SendResult(plate,1);
	#endif
}

//发送检测结果到客户端
void CMatchPlate::SendResult(RECORD_PLATE& plate,unsigned int uSeq)
{
	SRIP_DETECT_HEADER sDetectHeader;
	sDetectHeader.uChannelID = plate.uChannelID;
	//车牌检测类型
	sDetectHeader.uDetectType = SRIP_CARD_RESULT;
	sDetectHeader.uTimestamp = plate.uTime;
	sDetectHeader.uSeq = uSeq;

	std::string result;

	//判断车牌位置是否需要扩充
	GetCarPostion(plate);

	RECORD_PLATE_CLIENT plate_client;
	memcpy(&plate_client,&plate,sizeof(RECORD_PLATE_CLIENT));

	result.append((char*)&plate_client,sizeof(plate_client));

	result.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
	g_skpChannelCenter.AddResult(result);
}

//超过15s前的车牌记录删除
bool CMatchPlate::IsLastPlate(UINT32 uPlateTime)
{
	bool bRet = true;

	time_t ctTime;
	time(&ctTime);

	if ((ctTime - uPlateTime) > 15)
	{
		bRet = false;
	}
	
	return bRet;
}
