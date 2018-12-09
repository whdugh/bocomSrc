// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2014 上海博康智能信息技术有限公司
// Copyright 2008-2014 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "RoadRecordTemp.h"
#include "Common.h"
#include "CommonHeader.h"
#include "FileManage.h"
#include "AviFile.h"

//#define BEFOR_EVENT_TIME 5
#define MAX_IMG_FRAME_SIZE 500000 //500k

//录像线程
void* ThreadRecordTemp(void * pArg)
{
	CRoadRecordTemp * pRoadRecordTemp = (CRoadRecordTemp*)pArg;
	if(pRoadRecordTemp)
	{
		pRoadRecordTemp->Run();
	}
	pthread_exit((void *)0);
	return pArg;
}

CRoadRecordTemp::CRoadRecordTemp()
{
	//事件消息列表互斥
	pthread_mutex_init(&m_Event_Mutex,NULL);

	//事件录象长度
	m_nCaptureTime = 5;
	//线程结束标志
	m_bEndRecorder = false;
	//录象线程数目
	m_nEventCount = 0;

	//相机类型
	m_nCamType = 0;

	//是否正在录像
	m_bEndCoding = false; 

	//录像名称
	m_strVideoPath = "";
	m_pVideoCut = NULL;
	m_nThreadId = 0;
}

CRoadRecordTemp::~CRoadRecordTemp()
{
	LogNormal("CRoadRecordTemp::~CRoadRecordTemp 11");
	this->Unit();

	//事件消息列表互斥
	pthread_mutex_destroy(&m_Event_Mutex);
	LogNormal("CRoadRecordTemp::~CRoadRecordTemp 22");
}

//初始化
void CRoadRecordTemp::Init()
{
	//线程结束标志
	m_bEndRecorder = false;
	m_nEventCount = 0;
	m_nThreadId = 0;

	//加锁
	pthread_mutex_lock(&m_Event_Mutex);
	m_EventList.clear();
	//解锁
	pthread_mutex_unlock(&m_Event_Mutex);	

	//启动录像主线程
	bool bStart = StartThread();
	/*if(bStart)
	{
		if(NULL == m_pVideoCut)
		{
			//事件转换为待处理VIDEO_INFO记录
			m_pVideoCut = new CVideoCut();
			if(m_pVideoCut)
			{
				//初始化
				m_pVideoCut->Init();
			}
		}
	}*/	
}

//反初始化
void CRoadRecordTemp::Unit()
{
	LogNormal("--CRoadRecordTemp::Unit 11");
	//线程结束标志
	m_bEndRecorder = true;

	/*if(m_pVideoCut)
	{
		//LogNormal("--CRoadRecordTemp::Unit--111--m_pVideoCut->Unit---\n");
		m_pVideoCut->Unit();
		//LogNormal("--CRoadRecordTemp::Unit--222--m_pVideoCut->Unit---\n");

		delete m_pVideoCut;

		LogNormal("--CRoadRecordTemp::Unit--after--delete m_pVideoCut---\n");

		m_pVideoCut = NULL;
	}*/

	//停止录像线程
	CloseThread();

	//加锁
	pthread_mutex_lock(&m_Event_Mutex);
	m_EventList.clear();
	//解锁
	pthread_mutex_unlock(&m_Event_Mutex);

	m_nEventCount = 0;
	LogNormal("--CRoadRecordTemp::Unit 22");
}

//添加事件消息
bool CRoadRecordTemp::AddEvent(std::string& event)
{
	if(event.size() > 0 && (!m_bEndRecorder))
	{
		SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());
		//LogTrace("Deal-AddEvent-temp.log", "=add=AddEvent=sDetectHeader1->uSeq=%d==sDetectHeader1->uTimestamp=%lld, m_uPrevTimestamp=%d", \
			sDetectHeader1->uSeq, sDetectHeader1->uTimestamp, m_uPrevTimestamp);

		//加锁
		pthread_mutex_lock(&m_Event_Mutex);
		//添加到事件消息列表
		m_EventList.push_back(event);
		//LogNormal("addevent id:%d size:%d tsH:%lld %s", \
			sDetectHeader1->uChannelID, m_EventList.size(),\
			sDetectHeader1->uTimestamp, (GetTime(sDetectHeader1->uTimestamp,2)).c_str());

#ifdef RECORDER_DEBUG
		LogTrace("VideoPP.log", "addevent id:%d eventsize:%d tsH:%lld %s", \
			sDetectHeader1->uChannelID, m_EventList.size(),\
			sDetectHeader1->uTimestamp, (GetTime(sDetectHeader1->uTimestamp,3)).c_str());
#endif
		//解锁
		pthread_mutex_unlock(&m_Event_Mutex);

		m_nEventCount++;
	}
	return false;
}

//弹出一条事件
std::string CRoadRecordTemp::PopEvent()
{
	std::string response;

	//加锁
	pthread_mutex_lock(&m_Event_Mutex);

	//判断是否有命令
	if(m_EventList.size() <= 0)
	{
		//解锁
		pthread_mutex_unlock(&m_Event_Mutex);
		return response;
	}
	//取最早命令
	std::list<std::string>::iterator it = m_EventList.begin();
	//保存数据
	response = *it;
	//删除取出的命令
	m_EventList.pop_front();

	//解锁
	pthread_mutex_unlock(&m_Event_Mutex);

	return response;
}

//启动线程
bool CRoadRecordTemp::StartThread()
{
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	//启动事件录像线程
	int nret = pthread_create(&m_nThreadId,&attr,ThreadRecordTemp,this);
	if(nret != 0)
	{
		CloseThread();
		LogError("创建录像线程CRoadRecordTemp失败!\n");
		return false;
	}
	LogNormal("==CRoadRecordTemp=start=ThreadRecordTemp=m_nThreadId=%x==\n", m_nThreadId);

	pthread_attr_destroy(&attr);

	return true;
}

//退出线程
void CRoadRecordTemp::CloseThread()
{
	if(m_nThreadId != 0)
	{
		//pthread_cancel(m_nThreadId);
		pthread_join(m_nThreadId, NULL);
		m_nThreadId = 0;
	}
	LogNormal("CRoadRecordTemp::CloseThread()\n");
}

//主循环
void CRoadRecordTemp::Run()
{
	while(!m_bEndRecorder)
	{
		/*if(m_pVideoCut)
		{
			m_bEndCoding = m_pVideoCut->GetDealState();
			//LogNormal("-CRoadRecordTemp::Run-bDealState:%d ", bDealState);
			if(!m_bEndCoding)
			{
				DealTempVideo();
			}
		}*/
		DealTempVideo();

		//响应下一个录象请求
		usleep(10*1000);
	}
}

//进行录象,缓存录像处理
void CRoadRecordTemp::DealTempVideo()
{
	//弹出一个事件
	std::string strEvent = PopEvent();
	if(strEvent.size() > 0)
	{
		RECORD_EVENT eventElem;

		//FIX 多个事件,只处理第一个事件?
		//获取单个待处理事件
		int nSize = GetEventElem(strEvent, eventElem);
		if(nSize > 0)
		{
			//延时处理
			UINT32 nNow = GetTimeStamp();
			int nDis = (int)(nNow - eventElem.uEventBeginTime);	
			int nMaxTempWait = eventElem.uEventEndTime - eventElem.uEventBeginTime + 80;//等待录像完成时间
			if(nDis < nMaxTempWait)//时差MAX_TEMP_RECORD_WAIT秒以上才处理
			{
				int nS = nMaxTempWait - nDis;

#ifdef RECORDER_DEBUG
				//LogNormal("DealTempVideo sleep %d \n", nS);
				LogTrace("VideoPP.log","chan:%d DealTempVideo sleep %d ",eventElem.uChannelID, nS);
#endif
				while(nS > 0 && !m_bEndRecorder)
				{
					sleep(1);
					nS --;
				}
				//sleep(nS);
			}
			else
			{
				//超过50分钟的事件录像不处理
				if(nDis > 3000)
				{
					LogNormal("Event video time:%s too old!", GetTime(eventElem.uEventBeginTime, 3));
					return;
				}
				//LogNormal("DealTempVideo sleep 0 nDis:%d \n", nDis);
			}

#ifdef RECORDER_DEBUG
			//LogNormal("event deal id:%d [%lld, %lld] %s ", \
				eventElem.uChannelID, eventElem.uEventBeginTime, eventElem.uEventEndTime, GetTime(eventElem.uEventBeginTime,2).c_str());
			LogTrace("VideoPP.log","event deal id:%d [%lld, %lld] %s m_bEndCoding:%d", \
				eventElem.uChannelID, eventElem.uEventBeginTime, \
				eventElem.uEventEndTime, GetTime(eventElem.uEventBeginTime,3).c_str(), m_bEndCoding);
#endif

			if(strEvent.size() > 0 && (!m_bEndRecorder))
			{
				//根据相机类型调用			
				if(g_nDetectMode == 2)
				{
					//1.获取临时录像文件列表
					GetSrcPathList();
					

					//2.直接读文件进行录像
					if(m_pathSrcList.size() > 0)
					{
						if(!DealDspH264Recorder(strEvent))
						{
							LogError("video error\n");
						}
					}
				}
				else
				{
					//DealH264Recorder(strEvent);
				}		
			}
		}
	}

	m_nEventCount--;
}

//获取单个待处理事件
int CRoadRecordTemp::GetEventElem(const std::string &event, RECORD_EVENT &re_event)
{
	int nRetSize = 0;

	int nOffset = sizeof(SRIP_DETECT_HEADER);
	//事件消息头
	SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());

	nRetSize =  (event.size()- nOffset)/(sizeof(RECORD_EVENT));
	if(nRetSize > 0)
	{
		memcpy(&re_event,event.c_str()+nOffset,sizeof(RECORD_EVENT));

		re_event.uChannelID = sDetectHeader1->uChannelID;
		m_uChannelId = re_event.uChannelID;
		m_uBeginTime = re_event.uEventBeginTime;
		m_uEndTime = re_event.uEventEndTime;

		//LogNormal("EventElem:%lld %lld %s \n", re_event.uEventBeginTime, re_event.uEventEndTime, GetTime(re_event.uEventBeginTime, 3).c_str());
	}
	return nRetSize;
}

//获取原始录像文件列表
void CRoadRecordTemp::GetSrcPathList()
{
	int nMinStart = 0;
	int nMinDis = 0;
	bool bGetMinDis = GetVideoMinDis(nMinStart, nMinDis);

	if(bGetMinDis)
	{
		char buf[MAX_PATH_LEN] = {0};
		m_pathSrcList.clear();
		while(nMinDis > 0)
		{
			memset(buf, 0, MAX_PATH_LEN);			
			sprintf(buf, "%s/%02d_%02d.h264", g_strVideoTemp.c_str(), nMinStart, m_uChannelId);
			std::string strPath(buf);

			//LogNormal("-strPath:%s-11-", strPath.c_str());

			m_pathSrcList.push_back(strPath);

			//LogNormal("-strPath:%s-22-", strPath.c_str());

			nMinStart = (nMinStart + 1) % 60;

			nMinDis--;
		}		
	}	
}

//由时间戳获到小时,分钟,秒
void CRoadRecordTemp::GetVideoTime(const UINT32 uTime, UINT32 &uHour, UINT32 &uMin, UINT32 &uSec)
{
	long lTime = uTime;
	char buf[128];
	memset(buf, 0, sizeof(buf));
	struct tm *newTime,timenow;
	newTime = &timenow;
	localtime_r( &lTime,newTime );

	uHour = newTime->tm_mon + 1;
	uMin = newTime->tm_min;
	uSec = newTime->tm_sec;
}

//通过时间戳,获取录像时间,分钟区间
bool CRoadRecordTemp::GetVideoMinDis(int &nMinStart, int &nMinDis)
{
	bool bRet = true;

	//时间转换,m_uBeginTime,m_uEndTime
	UINT32 hour1 = 0;
	UINT32 hour2 = 0;
	UINT32 min1 = 0;
	UINT32 min2 = 0;
	UINT32 sec1 = 0;
	UINT32 sec2 = 0;

	GetVideoTime(m_uBeginTime, hour1, min1, sec1);
	GetVideoTime(m_uEndTime, hour2, min2, sec2);

	//nMinStart = min1;

	//取录像前一分钟开始,防止前面的数据没有
	nMinStart = min1 - 1;
	if(nMinStart < 0)
	{
		nMinStart = 59;
	}

	nMinDis = 0;
	if(hour2 >= hour1)
	{
		if(min2 >= min1)
		{
			nMinDis = (min2 - min1) + 60 * (hour2 - hour1);
		}
		else
		{
			nMinDis = (min2 - min1) + 60;
		}
	}
	else
	{	
		nMinDis = (min2 - min1) + 60 * (24 + hour2 - hour1);	
	}

	//往后延3分钟结束,防止后面的数据没有
	nMinDis += 3;

	if(nMinDis < 0 || nMinDis > 10)
	{
		//时间有问题
		bRet = false;
	}

	return bRet;
}


//录像格式为MP4-H264
bool CRoadRecordTemp::DealDspH264Recorder(std::string& event)
{
	//printf("===in=DealDspH264Recorder==\n");
	char* pImgBuff = new char[MAX_IMG_FRAME_SIZE];
	int nOffset = sizeof(SRIP_DETECT_HEADER);
	SRIP_DETECT_HEADER* sDetectHeader1 = (SRIP_DETECT_HEADER*)(event.c_str());
	RECORD_EVENT re_event;
	int nSize =  (event.size()-nOffset)/(sizeof(RECORD_EVENT));
	if(nSize > 0)
	{
		memcpy(&re_event,event.c_str()+nOffset,sizeof(RECORD_EVENT));
	}
	std::string strVideoPath(re_event.chVideoPath);

	std::string strVideoFileName = strVideoPath;
	std::string strTmpPath = "ftp://"+g_ServerHost;
	//LogNormal("strTmpPath:%s ", strTmpPath.c_str());

	strVideoFileName.erase(0,strTmpPath.size());

	if(g_nServerType == 13 && g_nFtpServer == 1)
	{
		std::string strDataPath = "/home/road/dzjc";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/dzjc";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else if(g_nServerType == 7)
	{
		std::string strDataPath = "/home/road/red";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/red";
		}
		strVideoFileName = strDataPath + strVideoFileName;
	}
	else
	{
		strVideoFileName = g_strVideo + strVideoFileName;
	}
	//LogNormal("outFile=%s\n",strVideoFileName.c_str());
	FILE *fpOut = NULL;
	if(0 == re_event.nVideoFlag)
	{
		fpOut = fopen(strVideoFileName.c_str(), "wb");
	}
	else
	{
		fpOut = fopen(strVideoFileName.c_str(), "ab+");
	}

	//LogTrace("VideoP.log","outFile=%s fpOut:%x type:%d \n",\
	//	strVideoFileName.c_str(), fpOut, re_event.uCode);
	CAviFile m_AviFile;
	bool bAviFlag = false;
	bool bEndCoding = false;

#ifdef RECORDER_DEBUG
	int nSeqCounts = 0;
	LogTrace("VideoPP.log", "DealDspH264Recorder chan:%d B:%s E:%s ", \
		re_event.uChannelID, GetTime(re_event.uEventBeginTime,3).c_str(), GetTime(re_event.uEventEndTime,3).c_str());
#endif

	if(fpOut)
	{
		if(g_nEncodeFormat != 1)//非裸h264流
		{
			char  fcc[5] = "H264";

			/* Init avi */
			if(fpOut)
			{
				m_AviFile.avi_init( &(m_AviFile.avi), fpOut, static_cast<float>(g_fFrameRate), fcc, g_nVideoWidth, g_nVideoHeight);
				m_AviFile.vbuf_init( &(m_AviFile.vb));
			}			
		}

		//从临时缓冲文件中读取录像数据
		std::list<std::string>::iterator it = m_pathSrcList.begin();
		while((it != m_pathSrcList.end()) && (!m_bEndRecorder))
		{
			string strPath = *it;
			//LogNormal("Read file = %s\n",strPath.c_str());
			FILE *fpIn = fopen(strPath.c_str(), "rb");
		
			if(fpIn != NULL)
			{	
				int nBytesReadHead = 0;
				int nBytesRead = 0;
				int nBytesLeft = 0;

				int nHeaderSize = sizeof(Image_header_dsp);
				Image_header_dsp imgHead;

				//读取文件,解帧	
				while((!feof(fpIn))&& (!m_bEndRecorder) && !bEndCoding)
				{
					memset((char*)(&imgHead), 0, nHeaderSize);
					nBytesReadHead = fread((char*)(&imgHead), 1, nHeaderSize, fpIn);
					if(nBytesReadHead == 0)
					{
						continue;
					}

					//核查帧正确性
					if(imgHead.nSize > 0 && imgHead.nSize < MAX_IMG_FRAME_SIZE*2)
					{
						if(nBytesReadHead == nHeaderSize)
						{	
							string strData;
							nBytesLeft = imgHead.nSize;
							while( (nBytesLeft > 0) && (!m_bEndRecorder) && (!feof(fpIn)))
							{
								nBytesRead = fread((char*)(pImgBuff),sizeof(unsigned char),nBytesLeft,fpIn);
								if(nBytesRead > 0)
								{
									//写入帧数据
									UINT32 uTime = (UINT32)(imgHead.ts/1000/1000);

									//LogTrace("VideoP.log", "imgHead.nSize:%d uTime:%lld re_event.BTime:%d", imgHead.nSize, uTime, re_event.uEventBeginTime);
									if(uTime <= re_event.uEventEndTime)
									{
											if(uTime >= re_event.uEventBeginTime)
											{
												if(g_nEncodeFormat != 1)//非裸h264流
												{
													strData.append(pImgBuff,nBytesRead);
												}
												else
												{
													if(fpOut)
													{
														fwrite((char*)(pImgBuff), 1, nBytesRead, fpOut);
													}
#ifdef RECORDER_DEBUG
													++nSeqCounts;
#endif
												}
											}
									}
									else
									{
											bEndCoding = true;
											break;
									}
									nBytesLeft -= nBytesRead;
								}
							}
							if(g_nEncodeFormat != 1)//非裸h264流
							{
								if(fpOut != NULL)
								{
										if(strData.size() > 0)
										{
												bool bKeyFrame = false;
												if ((( *(strData.c_str()+4)&0x1f) == 0x07) || (( *(strData.c_str()+4)&0x1f) == 0x08)  || (( *(strData.c_str()+4)&0x1f) == 0x05))
												{
													bKeyFrame = true;
												}
												m_AviFile.vbuf_add( &m_AviFile.vb, strData.size(), (char*)(strData.c_str()) );
												if( m_AviFile.vb.i_data > 0 )
												{
													bAviFlag = true;
													m_AviFile.avi_write( &m_AviFile.avi, &m_AviFile.vb, bKeyFrame ? AVIIF_KEYFRAME : 0);
													m_AviFile.vbuf_reset(&m_AviFile.vb);
												}
										}
								}
							}
						}
						else
						{
							LogNormal("Read file header error:%d\n",nBytesReadHead);
							break;
						}
					}
					else
					{
						LogNormal("imgHead.nSize=%d,nBytesReadHead=%d error!\n",imgHead.nSize,nBytesReadHead);
						break;
					}

					usleep(1*1000);
				}//End of while

				fclose(fpIn);
				fpIn = NULL;
			}

			if(bEndCoding)
			{
				break;
			}
			it++;
		}

		if(g_nEncodeFormat != 1)//非裸h264流
		{
			//停止一段录像
			if(bAviFlag)
			{
				m_AviFile.avi.i_width  = g_nVideoWidth;
				m_AviFile.avi.i_height = g_nVideoHeight;
				m_AviFile.avi_end( &m_AviFile.avi );
			}
		}
	}//End of if(fpOut)

#ifdef RECORDER_DEBUG
	LogTrace("VideoPP.log", "DealDspH264Recorder chan:%d nSeqCounts:%d event:car:%s %s", \
		re_event.uChannelID, nSeqCounts, re_event.chText, strVideoPath.c_str());
#endif

	//通知违章录像完成
	g_skpDB.VideoSaveUpdate(strVideoFileName, sDetectHeader1->uChannelID,2);

	if (fpOut)
	{
		fclose(fpOut);
		fpOut = NULL;
	}

	if(pImgBuff)
	{
		delete pImgBuff;
		pImgBuff = NULL;
	}

	return bEndCoding;
}
