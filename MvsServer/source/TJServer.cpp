// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
*   文件：TJServer.cpp
*   功能：天津电警通讯类
*   作者：yuwenxian
*   时间：2010-12-20
**/

#include "Common.h"
#include "CommonHeader.h"
#include "TJServer.h"
#include "RoadImcData.h"
#include "FtpCommunication.h"
#include "XmlParaUtil.h"
#include <sys/mount.h>
#include "ximage.h"
//#define TJCS_FILE_LOG
#ifdef TJCS_FILE_LOG
        FILE *g_pCSSendLog;
#endif
mvCTJServer g_TJServer;

//记录发送线程
void* ThreadTJRecorderResult(void* pArg)
{
	//取类指针
	mvCTJServer* pTJServer = (mvCTJServer*)pArg;
	if(pTJServer == NULL) return pArg;

	//处理一条数据
	pTJServer->DealResult();

	return pArg;
}

//历史记录发送线程
void*ThreadTJHistoryResult(void* pArg)
{
    g_TJServer.mvDealHistoryRecord();

	pthread_exit((void *)0);
}

void* ThreadTJStatusResult(void* pArg)
{
	//取类指针
	mvCTJServer* pTJServer = (mvCTJServer*)pArg;
	if(pTJServer == NULL) return pArg;

	//处理一条数据
	pTJServer->DealStatusResult();

	return pArg;
}

mvCTJServer::mvCTJServer()
{
#ifdef TJCS_FILE_LOG
    g_pCSSendLog = fopen("cs_send.log", "w");
#endif


    pthread_mutex_init(&m_Result_Mutex,NULL);
	pthread_mutex_init(&m_Video_Mutex,NULL);

    m_strFtpRemoteDir = "";
    m_strFtpRemoteTimeDir = "";
    //m_strVideoName = "";
	m_nThreadId = 0;
	m_nHistoryThreadId = 0;
	m_nStatusThreadId = 0;

	m_sKakouSetting = &m_FtpSetting[0];
	m_sBreakRulesSetting = &m_FtpSetting[1];
	m_sStatisticSetting = &m_FtpSetting[2];
	m_sVideoSetting = &m_FtpSetting[3];
	m_sStatus = &m_FtpSetting[4];
	m_nNo = 0;
	m_nStatusNo = 0;

	bInitImgBig200 = false;
	pImgBig200 = NULL;
	m_bStarting = false;

	for(int i=0; i<4; i++)
	{
		pImgArray200[i] = NULL;
	}

	bInitImgBig500 = false;
	pImgBig500 = NULL;

	for(int i=0; i<4; i++)
	{
		pImgArray500[i] = NULL;
	}
#ifdef KAFKA_SERVER
	m_kafkaManage = NULL;
#endif
}


mvCTJServer::~mvCTJServer()
{
    #ifdef TJCS_FILE_LOG
    if (g_pCSSendLog != NULL)
    {
        fclose(g_pCSSendLog);
        g_pCSSendLog = NULL;
    }
#endif

#ifdef KAFKA_SERVER
	if (m_kafkaManage)
	{
		delete m_kafkaManage;
		m_kafkaManage = NULL;
	}
#endif

    pthread_mutex_destroy(&m_Result_Mutex);
	pthread_mutex_destroy(&m_Video_Mutex);
}


//启动侦听服务
bool mvCTJServer::Init()
{
	//////////////////////////
	 {
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
		int nret = 0;

		//if(10 == g_nServerType)
		{
			nret=pthread_create(&m_nThreadId,&attr,ThreadTJRecorderResult,this);
			//成功
			if(nret!=0)
			{
				//失败
				LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
				g_bEndThread = true;
				return false;
			}
		}

		nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadTJHistoryResult,this);
		//成功
		if(nret!=0)
		{
			//失败
			LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
			g_bEndThread = true;
			return false;
		}
#ifdef SENDSTATUS
		if(7 == g_nServerType)
		{
			nret=pthread_create(&m_nStatusThreadId,&attr,ThreadTJStatusResult,this);
			//成功
			if(nret!=0)
			{
				//失败
				LogError("创建前端设备状态采集发送线程失败,服务无法启动!\r\n");
				g_bEndThread = true;
				return false;
			}
		}
#endif
		pthread_attr_destroy(&attr);
	 }

	string strLocation = g_RoadImcData.GetPlace();
	
	m_listVideo.clear();

	//CXmlParaUtil xml;
	//xml.GetMaxSpeed(m_maxSpeedMap, 1);
	//xml.GetMaxSpeedStr(m_maxSpeedMapStr, 1);


	/////////////////
	{
		if(7 == g_nServerType)
		{
			system("usermod -d /home/road/server road");
			chmod("/home/road/server",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			system("chown root:root /home/road/server");
			system("chmod 777 /home/road/server/update");
		}
		chmod("/home/road",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);

		if(access("../../red",0) != 0) //目录不存在
		{
			system("useradd -m -s /bin/bash -p $(perl -e 'print crypt(\"hpjj\", \"aa\")') -N red");
		}

		if(access("/home/road/red",0) != 0) //目录不存在
		{
			mkdir("/home/road/red",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		system("usermod -d /home/road/red red");
		chmod("/home/road/red",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		
		
		if(access("../../status",0) != 0) //目录不存在
		{
			system("useradd -m -s /bin/bash -p $(perl -e 'print crypt(\"hpjj\", \"aa\")') -N status");
		}
		if(access("/home/road/status",0) != 0) //目录不存在
		{
			mkdir("/home/road/status",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		system("usermod -d /home/road/status status");
		chmod("/home/road/status",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		

		if(access("../../flow",0) != 0) //目录不存在
		{
			system("useradd -m -s /bin/bash -p $(perl -e 'print crypt(\"hpjj\", \"aa\")') -N flow");
		}
		if(access("/home/road/flow",0) != 0) //目录不存在
		{
			mkdir("/home/road/flow",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		system("usermod -d /home/road/flow flow");
		chmod("/home/road/flow",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		

		if(access("../../kakou",0) != 0) //目录不存在
		{
			system("useradd -m -s /bin/bash -p $(perl -e 'print crypt(\"hpjj\", \"aa\")') -N kakou");
		}
		if(access("/home/road/kakou",0) != 0) //目录不存在
		{
			mkdir("/home/road/kakou",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		system("usermod -d /home/road/kakou kakou");
		chmod("/home/road/kakou",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		

		if(access("../../video",0) != 0) //目录不存在
		{
			system("useradd -m -s /bin/bash -p $(perl -e 'print crypt(\"hpjj\", \"aa\")') -N video");
		}
		if(access("/home/road/video",0) != 0) //目录不存在
		{
			mkdir("/home/road/video",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		system("usermod -d /home/road/video video");
		chmod("/home/road/video",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		

		if (IsDataDisk())
		{
			chmod("/detectdata",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			
			if(access("/detectdata/red",0) != 0) //目录不存在
			{
				mkdir("/detectdata/red",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			}
			system("usermod -d /detectdata/red red");
			chmod("/detectdata/red",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			
			if(access("/detectdata/status",0) != 0) //目录不存在
			{
				mkdir("/detectdata/status",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			}
			system("usermod -d /detectdata/status status");
			chmod("/detectdata/status",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			
			if(access("/detectdata/flow",0) != 0) //目录不存在
			{
				mkdir("/detectdata/flow",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			}
			system("usermod -d /detectdata/flow flow");
			chmod("/detectdata/flow",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			
			if(access("/detectdata/kakou",0) != 0) //目录不存在
			{
				mkdir("/detectdata/kakou",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			}
			system("usermod -d /detectdata/kakou kakou");
			chmod("/detectdata/kakou",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			
			if(access("/detectdata/video",0) != 0) //目录不存在
			{
				mkdir("/detectdata/video",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			}
			system("usermod -d /detectdata/video video");
			chmod("/detectdata/video",S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
	}

	if (23 == g_nServerType && g_Kafka.uSwitchUploading == 1)
	{
#ifdef KAFKA_SERVER
		if (m_kafkaManage == NULL)
		{
			m_kafkaManage = new CKafakaManage();
			m_kafkaManage->Init();
		}
#endif
	}

	m_nNo = 0;
	m_nStatusNo = 0;
	m_bStarting = false;
	return true;
}

//释放
bool mvCTJServer::UnInit()
{
	//析构
	if(bInitImgBig200)
	{
		if(pImgBig200 != NULL)
		{
			cvReleaseImage(&pImgBig200);
			pImgBig200 = NULL;
		}

		for(int i=0; i<4; i++)
		{
			cvReleaseImage(&pImgArray200[i]);
			pImgArray200[i] = NULL;
		}

		bInitImgBig200 = false;
	}

	if(bInitImgBig500)
	{
		if(pImgBig500 != NULL)
		{
			cvReleaseImage(&pImgBig500);
			pImgBig500 = NULL;
		}

		for(int i=0; i<4; i++)
		{
			cvReleaseImage(&pImgArray500[i]);
			pImgArray500[i] = NULL;
		}

		bInitImgBig500 = false;
	}
	

    //停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}

	if(m_nStatusThreadId != 0)
	{
		pthread_join(m_nStatusThreadId,NULL);
		m_nStatusThreadId = 0;
	}

	if(m_nHistoryThreadId != 0)
	{
		pthread_join(m_nHistoryThreadId,NULL);
		m_nHistoryThreadId = 0;
	}

    //g_FtpCommunication.DoClose();
    m_ChannelResultList.clear();

#ifdef KAFKA_SERVER
	if (m_kafkaManage)
	{
		delete m_kafkaManage;
		m_kafkaManage = NULL;
	}
#endif
	return true;
}


//处理记录结果
void mvCTJServer::DealResult()
{
    while(!g_bEndThread)
	{

		std::string response1;
		//////////////////////////////////////////////////////////先取检测
	    //加锁
	    pthread_mutex_lock(&m_Result_Mutex);

		//判断是否有命令
		if(m_ChannelResultList.size()>0)
		{
		    printf("==== mvCTJServer::DealResult()===\n");

			//取最早命令
			TJResultMsg::iterator it = m_ChannelResultList.begin();
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
		//1毫秒
		usleep(1000*100);
	}
}

void mvCTJServer::DealStatusResult()
{
	int nCount = 0;
	time_t startTime;
	time_t nowTime;
	time(&startTime);

	while(!g_bEndThread)
	{
		
		time(&nowTime);
		int offTime = (nowTime-startTime);
		if(offTime >= 60)
		{
			m_nStatusNo = GetHmTime()/60;

			SendStatusInfo();
			startTime = nowTime;
		}

		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
}

void mvCTJServer::SendStatusInfo()
{
	char buf[256] = {0};
	char localBuf[256] = {0};
	char localFile[256] = {0};
	char xmlFile[256] = {0};
	char localDir[256] = {0};
	char bufXml[1024] = {0};
	unsigned int nTime = GetTimeStamp();
	char bufCamera[1024] = {0};
	char bufsql[256]= {0};
	float fCpu=0.0;
	char strDisk[256] = {0};

	FILE* fp=fopen("/proc/cpuinfo", "r");
	if(fp!=NULL)
	{
		char szBuff[256] = {0};
		fread(szBuff,sizeof(szBuff),1,fp);
		char *pSysBuff = strstr(szBuff, "cpu MHz");
		if (pSysBuff != NULL)
		{
			std::stringstream stream;
			std::string sub_str;
			stream.clear();
			stream.str(pSysBuff);
			getline(stream,sub_str,'\n');
			int nPos = sub_str.find(":");
			if (nPos > 0)
			{
				if (sub_str.size() >= (nPos+9))
				{
					fCpu = atoi(sub_str.substr(nPos + 2,7).c_str())/1024.0;
				}
			}
		}
		fclose(fp);
	}

	string strCpu = g_sysInfo_ex.szCpu;
	float fCpuHz = 0.0;

	int nPos = strCpu.find("@");
	if (nPos > 0)
	{
		if (strCpu.size() >= (nPos+5))
		{
			fCpuHz = atof(strCpu.substr(nPos+2,strCpu.size() - nPos -3).c_str());
		}
	}
	if (IsDataDisk())
	{
		sprintf(strDisk,"/dev/sda");
	}
	else
	{
		sprintf(strDisk,"/dev/sda2");
	}
	sprintf(bufsql,"SELECT CAMERAIP,CAMERA_STATE,DEVICE_ID from CHAN_INFO ORDER BY CHAN_ID");
	String sql(bufsql);
	MysqlQuery q = g_skpDB.execQuery(sql);

	while(!q.eof())
	{
		sprintf(localDir,"设备状态/%s",GetTime(nTime,13).c_str());
		sprintf(xmlFile,"1401-%s-%s-%s-60-%d.xml",g_ftpRemotePath,q.getStringFileds(2).c_str(),GetTime(nTime,4).c_str(),m_nStatusNo);
		sprintf(bufXml,"<DeviceStatus>\n<Cpu Model=\"%s\" Frequency=\"%0.2fGHz\" CurrentFrequency=\"%0.2fGHz\" UsedPercent=\"%0.2f%\"/>\n" \
			"<Disk>\n<Disk ID=\"%s\" Used=\"%dG\" Free=\"%dG\" UsedPercent=\"%0.2f%\"/>\n</Disk>\n" \
			"<Memory TotalMem=\"%dG\" UsedMem=\"%0.2fG\" Percent=\"%0.2f%\"/>\n" \
			"<NetWork>\n<IP Address=\"%s\" Name=\"eth1\" Status=\"0\"/>\n" \
			"<IP Address=\"%s\" Name=\"相机网络\" Status=\"%d\"/>\n</NetWork>\n" \
			"</DeviceStatus>\n",
			g_sysInfo_ex.szCpu,fCpuHz,fCpu,fCpu/fCpuHz*100,
			strDisk,(int)(g_sysInfo_ex.fTotalDisk*g_sysInfo.fDisk/100),(int)(g_sysInfo_ex.fTotalDisk)-(int)(g_sysInfo_ex.fTotalDisk*g_sysInfo.fDisk/100),g_sysInfo.fDisk,
			(int)g_sysInfo_ex.fTotalMemory,(g_sysInfo_ex.fTotalMemory*1024*g_sysInfo.fCpu/100.0)/1024,g_sysInfo.fCpu,
			GetIpAddress("eth1",1).c_str(),q.getStringFileds(0).c_str(),q.getIntFileds(1));
		string strMsg = bufXml;

		string strDataPath = "/home/road/status";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/status";
		}
		if(access(strDataPath.c_str(),0) != 0)
		{
			mkdir(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		}
		sprintf(localBuf,"%s/%s",strDataPath.c_str(),GetTime(nTime,13).c_str());
		if(access(localBuf,0) != 0) //目录不存在
		{
			mkdir(localBuf,S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		}

		sprintf(localFile,"%s/%s",localBuf,xmlFile);
		FILE* fp = fopen(localFile,"w");
		if(fp!=NULL)
		{
			fwrite(bufXml, strlen(bufXml), 1, fp);
			fclose(fp);
		}
		sprintf(buf,"%s/%s",localDir,xmlFile);
		string remotetxtPath(buf);
		g_skpDB.UTF8ToGBK(remotetxtPath);

		int nLoop = 0;
		bool bRet = SendDataByFtp("", remotetxtPath, strMsg, &g_FtpCommunication);
		while(!bRet)
		{
			nLoop++;
			if (nLoop > 20)
			{
				LogNormal("FTP发送设备状态失败！\n");
				break;
			}
			bRet = SendDataByFtp("", remotetxtPath, strMsg, &g_FtpCommunication);
		}

		q.nextRow();
	}
	q.finalize();
}

//添加一条数据
bool mvCTJServer::AddResult(std::string& strMsg)
{
    printf("==== mvCTJServer::AddResult()===\n");
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strMsg.c_str();

	switch(sDetectHeader->uDetectType)
	{
       case MIMAX_EVENT_REP:	//事件(送中心端)
	   case MIMAX_PLATE_REP:   //车牌
	   case MIMAX_STATISTIC_REP:  //统计
	        //加锁
	        pthread_mutex_lock(&m_Result_Mutex);
	        if(m_ChannelResultList.size() > 10)//防止堆积的情况发生
	        {
	            //LogError("记录过多，未能及时发送!\r\n");
                m_ChannelResultList.pop_back();
	        }
			m_ChannelResultList.push_front(strMsg);
			//解锁
	        pthread_mutex_unlock(&m_Result_Mutex);
			break;
	   default:
			//LogError("未知数据[%x],取消操作!\r\n",sDetectHeader->uDetectType);
			return false;
	}
	return true;
}

//处理检测结果
bool mvCTJServer::OnResult(std::string& result)
{
	//发送数据
	MIMAX_HEADER mHeader;
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();
	bool bSendToServer = false;

	switch(sDetectHeader->uDetectType)
	{
			////////////////////////////////////////////
		case MIMAX_EVENT_REP:	//事件(送中心端)
		case MIMAX_PLATE_REP:  //车牌
		case MIMAX_STATISTIC_REP:  //统计
			{
				mHeader.uCmdID = (sDetectHeader->uDetectType);
				mHeader.uCmdFlag = sDetectHeader->uRealTime;
				mHeader.uCameraID = sDetectHeader->uChannelID;
			//	printf(" mHeader.uCmdID=%x ,sizeof(sDetectHeader)=%d\r\n",mHeader.uCmdID,sizeof(SRIP_DETECT_HEADER));
				//需要去掉SRIP_DETECT_HEADER头
				result.erase(0,sizeof(SRIP_DETECT_HEADER));
				bSendToServer = true;
			}
			break;
		default:
			//LogError("未知数据[%d],取消操作!\r\n",sDetectHeader->uChannelID);
			return false;
	}

    //此时去取图片
    if(mHeader.uCmdID == MIMAX_EVENT_REP)
    {
        RECORD_EVENT* sEvent = (RECORD_EVENT*)(result.c_str());
        String strPicPath(sEvent->chPicPath);
        result = result + GetImageByPath(strPicPath);
    }
    else if(mHeader.uCmdID == MIMAX_PLATE_REP)
    {
        RECORD_PLATE* sPlate = (RECORD_PLATE*)(result.c_str());
        String strPicPath(sPlate->chPicPath);
        result = result + GetImageByPath(strPicPath);
		//LogNormal("path = %s\n",strPicPath);
    }
	
    //数据长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + result.size();
	//添加头
	result.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

    //发送数据
    if(bSendToServer && m_bStarting == false)
    {
		m_bStarting = true;
		//if(10 == g_nServerType)//鞍山交警
        {
            if(mHeader.uCmdID == MIMAX_PLATE_REP)
            {
				//LogNormal("实时数据发送开始，【未发送】\n");
                if (g_TJServer.mvSendRecordToCS(result))
                {
//					LogNormal("实时数据发送成功，更改成【已发送】\n");
					unsigned int uSeq =*((unsigned int*)(result.c_str()+sizeof(MIMAX_HEADER)));
					g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
					m_bStarting = false;
                    return true;
                }
            }
			else if(mHeader.uCmdID == MIMAX_EVENT_REP)
			{
				#ifdef CAMERAAUTOCTRL
				if (g_TJServer.mvSendRecordToCS(result))
                {
					//LogNormal("实时数据发送成功，更改成【已发送】\n");
					unsigned int uSeq =*((unsigned int*)(result.c_str()+sizeof(MIMAX_HEADER)));
					g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
					m_bStarting = false;
                    return true;
                }
				#endif
			}
        }
		m_bStarting = false;
    }
    return false;
}



/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCTJServer::mvSendRecordToCS(const string &strMsg,bool bSymlink)
{
    printf("====mvCTJServer::mvSendRecordToCS(const string &strMsg)===\n");

    string strPicHead = "";
    char buf[256] = {0};

    MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();
    if (MIMAX_PLATE_REP == sHeader->uCmdID)
    {
		if(strMsg.size() <= sizeof(RECORD_PLATE)+sizeof(MIMAX_HEADER))
		{
			return true;
		}

        RECORD_PLATE* sPlate = (RECORD_PLATE*)(strMsg.c_str()+sizeof(MIMAX_HEADER));
		//LogTrace("mvSendRecordToCS.log", "sPlate: %s video:%s pic:%s \n", sPlate->chText, sPlate->chVideoPath, sPlate->chPicPath);

        string strFtpRemoteDir = m_strFtpRemoteDir;

        int nPicCount = 1;

		int nSpeedMax = sPlate->uLimitSpeed;

		if (23 == g_nServerType && g_Kafka.uSwitchUploading == 1)
		{
#ifdef KAFKA_SERVER
			if (sPlate->uViolationType == 0 && m_kafkaManage->GetServerStatus() == true)
			{
				string strKafkaMsg = m_kafkaManage->CreateKafkaMsg(sPlate);
				if (m_kafkaManage->SendMessage(strKafkaMsg))
				{
					return true;
				}
			}
#endif
		}
		if (24 == g_nServerType)// 南宁交警
		{
			int nPicSize = strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE);
			string strPicFull(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE), nPicSize);

			// 目录buf
			char chDirBuf[1024] ={0};
			char chDirBufOut[1024] ={0};
			// 文件名buf
			char chXmlFileBuf[1024] ={0};
			char chJpgFileBuf[1024] ={0};
			char chJpgFileBuf1[1024] ={0};
			char chJpgFileBuf2[1024] ={0};
			char chJpgFileBuf3[1024] ={0};
			char chJpgFileBuf4[1024] ={0};
			char chJpgFileBuf_1[1024] ={0};
			char chJpgFileBuf1_1[1024] ={0};
			char chJpgFileBuf2_1[1024] ={0};
			char chJpgFileBuf3_1[1024] ={0};
			char chJpgFileBuf4_1[1024] ={0};
			char strType[256] = {0};
			
			if (sPlate->uViolationType == DETECT_RESULT_EVENT_GO_FAST)//6 超速
			{
				sprintf(strType,"%s","超速图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION) //16 闯红灯
			{
				sprintf(strType,"%s","闯红灯图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION) //17 违章停车
			{
				sprintf(strType,"%s","违章停车图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD || 
				sPlate->uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD || 
				sPlate->uViolationType == DETECT_RESULT_NO_PASSING || 
				sPlate->uViolationType == DETECT_RESULT_FORBID_STRAIGHT)//禁行车道
			{
				sprintf(strType,"%s","禁行车道图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_EVNET_GO_EMERGENCY_CHANNEL) //走应急车道
			{
				sprintf(strType,"%s","走应急车道图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_FORBID_LEFT) //23 禁止左拐
			{
				sprintf(strType,"%s","禁止左拐图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_FORBID_RIGHT) //24 禁止右拐
			{
				sprintf(strType,"%s","禁止右拐图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_RETROGRADE_MOTION) //26 逆行
			{
				sprintf(strType,"%s","逆行图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_PRESS_LINE || sPlate->uViolationType == DETECT_RESULT_PRESS_WHITELINE) //27 压线
			{
				sprintf(strType,"%s","压线图片");
			}
			else if (sPlate->uViolationType == DETECT_RESULT_ELE_EVT_BIANDAO) //229 变道
			{
				sprintf(strType,"%s","变道图片");
			}
			else if((sPlate->uViolationType > 0) && (sPlate->uViolationType != DETECT_RESULT_NOCARNUM))
			{
				sprintf(strType,"%s","其它违章图片");
			}
			else
			{
				sprintf(strType,"%s","通行图片");
			}
			sprintf(chDirBuf, "%s/%s%d%d/%s/%s",sPlate->chPlace,g_strDetectorID.c_str(), g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,strType,GetTime(sPlate->uTime, 4).c_str());
			sprintf(chDirBufOut, "%s\\%s%d%d\\%s\\%s",sPlate->chPlace,g_strDetectorID.c_str(), g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,strType,GetTime(sPlate->uTime, 4).c_str());
			//XML
			int nViolatyionType = 2;
			nViolatyionType = g_RoadImcData.GetViolationTypeForNanning(sPlate->uViolationType);
			sprintf(chXmlFileBuf,"%s/%s%03d-%s%d%d-%02d-%02d-%03d.xml",chDirBuf, GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
			string strMsg;
			if((sPlate->uViolationType > 0) && (sPlate->uViolationType != DETECT_RESULT_NOCARNUM))
			{
				sprintf(chJpgFileBuf,"%s/%s%03d-%s%d%d-%02d-%02d-%03d.jpg",chDirBuf, GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf1,"%s/%s%03d-%s%d%d-%02d-%02d-%03d.jpg",chDirBuf,GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime-2,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf2,"%s/%s%03d-%s%d%d-%02d-%02d-%03d.jpg",chDirBuf, GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime+1,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf3,"%s/%s%03d-%s%d%d-%02d-%02d-%03d.jpg",chDirBuf, GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime+2,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf4,"%s/%s%03d-%s%d%d-%02d-%02d-%03d.jpg",chDirBuf, GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime-1,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf_1,"%s%03d-%s%d%d-%02d-%02d-%03d.jpg",GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf1_1,"%s%03d-%s%d%d-%02d-%02d-%03d.jpg",GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime-2,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf2_1,"%s%03d-%s%d%d-%02d-%02d-%03d.jpg",GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime+1,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf3_1,"%s%03d-%s%d%d-%02d-%02d-%03d.jpg",GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime+2,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				sprintf(chJpgFileBuf4_1,"%s%03d-%s%d%d-%02d-%02d-%03d.jpg", GetTime(sPlate->uTime, 2).c_str(),sPlate->uMiTime-1,g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(sPlate->uDirection),sPlate->uChannelID,sPlate->uRoadWayID,nViolatyionType,m_nNo);
				strMsg = g_RoadImcData.GetViolationDataForNanning(*sPlate,nSpeedMax,chJpgFileBuf_1,chDirBufOut,chJpgFileBuf1_1,chJpgFileBuf2_1,chJpgFileBuf3_1,chJpgFileBuf4_1);
			}
			else
			{
				strMsg = g_RoadImcData.GetViolationDataForNanning(*sPlate,nSpeedMax,"","","","","","");
			}
			string remotePath(chXmlFileBuf);
			g_skpDB.UTF8ToGBK(remotePath);
//			LogNormal("发送XML数据开始【%s】\n",chXmlFileBuf);
			if(!SendDataByFtpForNanning("", remotePath, strMsg, &g_FtpCommunication))
			{
				return false;
			}
			if((sPlate->uViolationType > 0) && (sPlate->uViolationType != DETECT_RESULT_NOCARNUM))
			{
//				LogNormal("违章图片数据处理开始\n");
				//图片命名
				string remotePath1(chJpgFileBuf);
				g_skpDB.UTF8ToGBK(remotePath1);
//				LogNormal("发送图片数据开始 全景图【%s】\n",chJpgFileBuf);
				if(!SendDataByFtpForNanning("", remotePath1, strPicFull, &g_FtpCommunication))
				{
					return false;
				}
				printf("************** 1\n");
				printf("**************DD [%d][%d]\n",sPlate->uPicWidth, sPlate->uPicHeight);

				/*
				if(sPlate->uPicWidth < 3000)
				{
					sPlate->uPicWidth += sPlate->uPicWidth;

					if(g_PicFormatInfo.nWordOnPic)
					{
						sPlate->uPicHeight -= g_PicFormatInfo.nExtentHeight;
					}

					sPlate->uPicHeight += sPlate->uPicHeight;
				}
				*/

				unsigned char * pJpgBuf = NULL;
				unsigned char * pJpgBufArray[4];
				int nJpgSize[4];

				pJpgBuf = new unsigned char[(sPlate->uPicWidth)*(sPlate->uPicHeight)];
				memset(pJpgBuf, 0, (sPlate->uPicWidth)*(sPlate->uPicHeight));

				for(int i=0; i<4; i++)
				{
					pJpgBufArray[i] = NULL;
					pJpgBufArray[i] = pJpgBuf + i*(sPlate->uPicWidth*sPlate->uPicHeight)/4;
					nJpgSize[i] = 0;
				}
				string strPicPath(sPlate->chPicPath);

				printf("---InitImg--111-\n");

				//核查是否创建大图缓存
				if(sPlate->uPicWidth/2 > 2000)
				{
					if(!bInitImgBig500)
					{
						InitImg500(sPlate->uPicWidth/2, sPlate->uPicHeight/2);
					}
				}
				else
				{
					if(!bInitImgBig200)
					{
						InitImg200(sPlate->uPicWidth/2, sPlate->uPicHeight/2);
					}
				}
				
				printf("---InitImg--222-\n");

				DepachBigJpg(strPicPath, sPlate->uPicWidth, sPlate->uPicHeight, (pJpgBufArray), nJpgSize);
				//do something...

				printf("---InitImg--333-\n");

				//Pic1
				{
					string remotePath2(chJpgFileBuf1);
					g_skpDB.UTF8ToGBK(remotePath2);

					string strPic1 = "";
					strPic1.append((char *)pJpgBufArray[0], nJpgSize[0]);
//LogNormal("发送图片数据开始 全景图1【%s】\n",chJpgFileBuf1);
					//LogNormal("strPic1.Size():%d, nJpgSize:%d AA \n", strPic1.size(), nJpgSize[0]);
					if(!SendDataByFtpForNanning("", remotePath2, strPic1, &g_FtpCommunication))
					{
						return false;
					}
				}
		
				//Pic2
				{
					string remotePath3(chJpgFileBuf2);
					g_skpDB.UTF8ToGBK(remotePath3);

					string strPic2 = "";
					strPic2.append((char *)pJpgBufArray[1], nJpgSize[1]);
	//				LogNormal("发送图片数据开始 全景图2【%s】\n",chJpgFileBuf2);
					//LogNormal("strPic1.Size():%d, nJpgSize:%d AA \n", strPic1.size(), nJpgSize[0]);
					if(!SendDataByFtpForNanning("", remotePath3, strPic2, &g_FtpCommunication))
					{
						return false;
					}
				}

				//Pic3
				{
					string remotePath4(chJpgFileBuf3);
					g_skpDB.UTF8ToGBK(remotePath4);

					string strPic3 = "";
					strPic3.append((char *)pJpgBufArray[2], nJpgSize[2]);
//LogNormal("发送图片数据开始 全景图3【%s】\n",chJpgFileBuf3);
					//LogNormal("strPic1.Size():%d, nJpgSize:%d AA \n", strPic1.size(), nJpgSize[0]);
					if(!SendDataByFtpForNanning("", remotePath4, strPic3, &g_FtpCommunication))
					{
						return false;
					}
				}
				//Pic4
				{
					string remotePath5(chJpgFileBuf4);
					g_skpDB.UTF8ToGBK(remotePath5);

					string strPic4 = "";
					strPic4.append((char *)pJpgBufArray[3], nJpgSize[3]);
//LogNormal("发送图片数据开始 全景图4【%s】\n",chJpgFileBuf4);
					//LogNormal("strPic1.Size():%d, nJpgSize:%d AA \n", strPic1.size(), nJpgSize[0]);
					if(!SendDataByFtpForNanning("", remotePath5, strPic4, &g_FtpCommunication))
					{
						return false;
					}
				}
				//释放内存
				if(pJpgBuf)
				{
					delete [] pJpgBuf;
					pJpgBuf = NULL;
				}

				for(int i=0; i<4; i++)
				{
					if(pJpgBufArray[i])
					{
						pJpgBufArray[i] = NULL;
					}
				}
			}

			if (m_nNo == 999)
			{
				m_nNo = 0;
			}
			else
			{
				m_nNo++;
			}
			return true;
		}

        if((sPlate->uViolationType > 0) && (sPlate->uViolationType != DETECT_RESULT_NOCARNUM))
        {
			//发送违章数据
			if(21 == g_nServerType)  //深圳交警
			{
				int nPicSize = strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE);
				string strPicFull(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE), nPicSize);

				//日期
				string strData = GetTime(sPlate->uTime, 4);
				//时间
				string strTime = GetTime(sPlate->uTime, 6);

				string strFtpHourDir = "data";

				int nDirection = 1;
				g_RoadImcData.GetDirectionStr(sPlate->uDirection,nDirection);

				sprintf(buf, "%s/V1_%s-%s%03d-%s-%d-%d-000000-%d.jpg",strFtpHourDir.c_str(), strData.c_str(),strTime.c_str(),sPlate->uMiTime, g_strDetectorID.c_str(),nDirection, g_RoadImcData.GetViolationType(sPlate->uViolationType), sPlate->uRoadWayID);

				string remotePath(buf);
				g_skpDB.UTF8ToGBK(remotePath);

				if(!SendDataByFtp("", remotePath, strPicFull, &g_FtpCommunication))
				{
					return false;
				}

				string remoteTxtPath;
				remoteTxtPath.append(remotePath.c_str(),remotePath.size()-3);
				remoteTxtPath += ".txt";
				string strMsg = g_RoadImcData.GetViolationData(*sPlate);
				
				if(!SendDataByFtp("", remoteTxtPath, strMsg, &g_FtpCommunication))
				{
					return false;
				}
				
				return true;
			}
			else if (10 == g_nServerType)  //鞍山交警
			{
				int nPicSize = strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE);
				string strPicFull(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE), nPicSize);

				//时间
				string strTime = GetTime(sPlate->uTime, 2);

				string strFtpHourDir = "鞍钢";

				int nonUse;				

				if(g_nFtpServer == 0)
				{
					strFtpHourDir = "data";
					//sprintf(buf, "%s/%s_%s_%s_%d_%d_%s_%s_%s.jpg", strFtpHourDir.c_str(), strTime.c_str(), sPlate->chPlace, GetDirection(sPlate->uDirection).c_str(), m_maxSpeedMap[sPlate->uRoadWayID], sPlate->uSpeed,
					//	g_RoadImcData.GetViolationTypeStr(sPlate->uViolationType, nonUse).c_str(), g_RoadImcData.GetVechileType(sPlate->uType).c_str(), sPlate->chText);

					//sprintf(buf, "%s/%s_%s_%s_%d_%d_%s_%s_%s.jpg", \
					//	strFtpHourDir.c_str(), strTime.c_str(), sPlate->chPlace, GetDirection(sPlate->uDirection).c_str(), nSpeedMax, \
					//	sPlate->uSpeed, g_RoadImcData.GetViolationTypeStr(sPlate->uViolationType, nonUse).c_str(), g_RoadImcData.GetVechileType(sPlate->uType).c_str(), sPlate->chText);

					int nCarNumberColor = 2;
					g_RoadImcData.GetCarNumberColorStr(sPlate->uColor, nCarNumberColor);

					sprintf(buf, "%s/%s-%d-%02d-%s-%s-%d-%d-%d-%d-%s.jpg", \
						strFtpHourDir.c_str(), g_ftpRemotePath,sPlate->uDirection, sPlate->uRoadWayID, strTime.c_str(), sPlate->chText,nCarNumberColor,sPlate->uCarColor1,\
						sPlate->uSpeed, nSpeedMax,g_strDetectorID.c_str());

				}
				else
				{
					//sprintf(buf, "%s/%s_%s_%s_%d_%d_%s_%s_%s_%d_%s.jpg", strFtpHourDir.c_str(), strTime.c_str(), sPlate->chPlace, GetDirection(sPlate->uDirection).c_str(), m_maxSpeedMap[sPlate->uRoadWayID], sPlate->uSpeed,
					//g_RoadImcData.GetViolationTypeStr(sPlate->uViolationType, nonUse).c_str(), g_RoadImcData.GetVechileType(sPlate->uType).c_str(), sPlate->chText, 3, " "/*公司名,暂时空着*/);
					sprintf(buf, "%s/%s_%s_%s_%d_%d_%s_%s_%s_%d_%s.jpg", strFtpHourDir.c_str(), strTime.c_str(), sPlate->chPlace, GetDirection(sPlate->uDirection).c_str(), nSpeedMax, sPlate->uSpeed,
						g_RoadImcData.GetViolationTypeStr(sPlate->uViolationType, nonUse).c_str(), g_RoadImcData.GetVechileType(sPlate->uType).c_str(), sPlate->chText, 3, " "/*公司名,暂时空着*/);

				}

				string remotePath(buf);
				g_skpDB.UTF8ToGBK(remotePath);

				return SendDataByFtp("", remotePath, strPicFull, &g_FtpCommunication);
			}
			else if (23 == g_nServerType)  //济南交警
			{
				int nPicSize = strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE);
				string strPicFull(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE), nPicSize);

				// 目录buf
				char chDirBuf[256] ={0};
				// 文件名buf
				char chFileBuf[256] ={0};
				//日期
				string strData = GetTime(sPlate->uTime, 4);
				//时间
				string strTime = GetTime(sPlate->uTime, 6);
				string strText = sPlate->chText;
				//g_skpDB.GBKToUTF8(strText);

				string strFtpHourDir = "kkwf";
				//sprintf(chDirBuf, "%s/%s/%s",strFtpHourDir.c_str(), strData.c_str(), GetTime(sPlate->uTime, 5).c_str());
				int nDirection = 1;
				nDirection = g_RoadImcData.GetDirectionForJinan(sPlate->uDirection);
				if (g_nGongJiaoMode == 1)
				{
					sprintf(chFileBuf, "%s_%d_%d_%s%s_%s_%02d_%03d.jpg", sPlate->szKaKouItem, sPlate->uRoadWayID, nDirection, strData.c_str(),strTime.c_str(),strText.c_str(),g_RoadImcData.GetVechileTypeForJinan(*sPlate),m_nNo);
				}
				else
				{
					sprintf(chFileBuf, "%s_%d_%d_%s%s_%s_%02d_%03d.jpg", g_ftpRemotePath, sPlate->uRoadWayID, nDirection, strData.c_str(),strTime.c_str(),strText.c_str(),g_RoadImcData.GetVechileTypeForJinan(*sPlate),m_nNo);
				}
				
				//sprintf(buf, "%s/%s", chDirBuf, chFileBuf);
				sprintf(buf, "%s", chFileBuf);
	
				string remotePath(buf);
				g_skpDB.UTF8ToGBK(remotePath);
				if(!SendDataByFtp("", remotePath, strPicFull, &g_FtpCommunication))
				{
					return false;
				}

				string strMsg = g_RoadImcData.GetViolationDataForJinan(*sPlate,remotePath);

				string remoteTxtPath = "kkwftxt";
				//sprintf(chDirBuf, "%s/%s/%s",remoteTxtPath.c_str(), strData.c_str(), GetTime(sPlate->uTime, 5).c_str());
				if (g_nGongJiaoMode == 1)
				{
					sprintf(chFileBuf, "%s_%d_%d_%s%s_%s_%02d_%03d.ini", sPlate->szKaKouItem, sPlate->uRoadWayID, nDirection, strData.c_str(),strTime.c_str(),strText.c_str(),g_RoadImcData.GetVechileTypeForJinan(*sPlate),m_nNo);
				}
				else
				{
					sprintf(chFileBuf, "%s_%d_%d_%s%s_%s_%02d_%03d.ini", g_ftpRemotePath, sPlate->uRoadWayID, nDirection, strData.c_str(),strTime.c_str(),strText.c_str(),g_RoadImcData.GetVechileTypeForJinan(*sPlate),m_nNo);
				}
				//sprintf(buf, "%s/%s", chDirBuf, chFileBuf);
				sprintf(buf, "%s", chFileBuf);

				string remotetxtPath(buf);
				g_skpDB.UTF8ToGBK(remotetxtPath);

				if(!SendDataByFtp("", remotetxtPath, strMsg, &g_FtpCommunication))
				{
					return false;
				}
				if (m_nNo == 999)
				{
					m_nNo = 0;
				}
				else
				{
					m_nNo++;
				}
				return true;
			}
			else
			{
				//printf("plate.chPicPath=%s\n",sPlate->chPicPath);

				/*if(7 == g_nServerType)
				{
					bool bJunChe = JunCheck(*sPlate);
					//军车牌过滤,不输出
					if(bJunChe)
					{
						return true;
					}
				}*/

				string strLocalPath(sPlate->chPicPath);
				string strDataPath = "/home/road/red";
				if (IsDataDisk())
				{
					strDataPath = "/detectdata/red";
				}
				string strFtpDir = strLocalPath;
				strFtpDir.erase(0,strDataPath.size());
				string remotePath;
				#ifdef TJPLACE
				std::string strLocation = g_skpDB.GetPlace(sPlate->uChannelID);
				int nDirection = g_skpDB.GetDirection(sPlate->uChannelID);
				std::string strDirection = GetDirection(nDirection);
				strLocation += strDirection;
				remotePath = "/" + strLocation;
				strFtpDir.erase(0,11);//去掉日期目录
				remotePath += strFtpDir;
				#else
				remotePath = strFtpDir;
				#endif
				g_skpDB.UTF8ToGBK(remotePath);
				
				string strMsg("");
				int nSendCount = 0;
				if(SendDataByFtp(strLocalPath, remotePath, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}

				//从jpg图片中获取防伪码
				int nRandCode1 = 0;
				int nRandCode2 = 0;
				g_RoadImcData.GetRandCodeFromPic(strLocalPath,nRandCode1,nRandCode2);
				
				
				char buf[256] = {0};
				string strLocalPath2;
				strLocalPath2.append(strLocalPath.c_str(),strLocalPath.size()-14);
				sprintf(buf,"%08x-2.jpg",nRandCode1);
				strLocalPath2 += buf;

				string remotePath2;
				remotePath2.append(remotePath.c_str(),remotePath.size()-14);
				remotePath2 += buf;

				if(SendDataByFtp(strLocalPath2, remotePath2, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}

				string strLocalPath3;
				strLocalPath3.append(strLocalPath.c_str(),strLocalPath.size()-14);
				sprintf(buf,"%08x-3.jpg",nRandCode2);
				strLocalPath3 += buf;

				string remotePath3;
				remotePath3.append(remotePath.c_str(),remotePath.size()-14);
				remotePath3 += buf;

				if(SendDataByFtp(strLocalPath3, remotePath3, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}

				string strLocalPath4;
				strLocalPath4.append(strLocalPath.c_str(),strLocalPath.size()-15);
				strLocalPath4 += ".ini";

				string remotePath4;
				remotePath4.append(remotePath.c_str(),remotePath.size()-15);
				remotePath4 += ".ini";

				if(SendDataByFtp(strLocalPath4, remotePath4, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}
				//LogNormal("nSendCount=%d\n",nSendCount);
				return true;
			}
        }
        else
        {
			if (10 == g_nServerType)  //鞍山交警
			{
				//青岛卡口数据也需要上传
				if(g_nFtpServer == 0)
				{
					int nPicSize = strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE);
					string strPicFull(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE), nPicSize);

					//时间
					string strTime = GetTime(sPlate->uTime, 2);

					int nCarNumberColor = 2;
					g_RoadImcData.GetCarNumberColorStr(sPlate->uColor, nCarNumberColor);

					string strFtpHourDir = "data";
					int nonUse;
					//sprintf(buf, "%s/%s_%s_%s_%d_%d_%s_%s.jpg", strFtpHourDir.c_str(), strTime.c_str(), sPlate->chPlace, GetDirection(sPlate->uDirection).c_str(), m_maxSpeedMap[sPlate->uRoadWayID], sPlate->uSpeed, g_RoadImcData.GetVechileType(sPlate->uType).c_str(), sPlate->chText);
					sprintf(buf, "%s/%s-%d-%02d-%s-%s-%d-%d-%d-%d-%s.jpg", \
						strFtpHourDir.c_str(), g_ftpRemotePath,sPlate->uDirection, sPlate->uRoadWayID, strTime.c_str(), sPlate->chText,nCarNumberColor,sPlate->uCarColor1,\
						sPlate->uSpeed, nSpeedMax,g_strDetectorID.c_str());

					string remotePath(buf);
					g_skpDB.UTF8ToGBK(remotePath);

					return SendDataByFtp("", remotePath, strPicFull, &g_FtpCommunication);
				}
			}
			if (23 == g_nServerType && g_Kafka.uSwitchUploading == 1)
			{
				return false;
			}
			else
			{
				return true;
			}
        }
    }
	else if(MIMAX_STATISTIC_REP == sHeader->uCmdID)
	{
		return true;
	}
	else if(MIMAX_EVENT_REP == sHeader->uCmdID)
	{
		RECORD_EVENT* sEvent = (RECORD_EVENT*)(strMsg.c_str()+sizeof(MIMAX_HEADER));

		//LogTrace("mvSendRecordToCS.log", "sEvent: %s video:%s pic:%s \n", \
			sEvent->chText, sEvent->chVideoPath, sEvent->chPicPath);

		if(sEvent->uCode == DETECT_RESULT_EVENT_STOP)
		{
				string strLocalPath(sEvent->chPicPath);
				string strDataPath = "/home/road/red";
				if (IsDataDisk())
				{
					strDataPath = "/detectdata/red";
				}
				string strFtpDir = strLocalPath;
				strFtpDir.erase(0,strDataPath.size());
				
				string remotePath;
				remotePath = strFtpDir;
				g_skpDB.UTF8ToGBK(remotePath);
				
				string strMsg("");
				int nSendCount = 0;
				if(SendDataByFtp(strLocalPath, remotePath, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}

				//从jpg图片中获取防伪码
				int nRandCode1 = 0;
				int nRandCode2 = 0;
				g_RoadImcData.GetRandCodeFromPic(strLocalPath,nRandCode1,nRandCode2);	
				
				char buf[256] = {0};
				string strLocalPath2;
				strLocalPath2.append(strLocalPath.c_str(),strLocalPath.size()-14);
				sprintf(buf,"%08x-2.jpg",nRandCode1);
				strLocalPath2 += buf;

				string remotePath2;
				remotePath2.append(remotePath.c_str(),remotePath.size()-14);
				remotePath2 += buf;

				if(SendDataByFtp(strLocalPath2, remotePath2, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}

				string strLocalPath3;
				strLocalPath3.append(strLocalPath.c_str(),strLocalPath.size()-14);
				sprintf(buf,"%08x-3.jpg",nRandCode2);
				strLocalPath3 += buf;

				string remotePath3;
				remotePath3.append(remotePath.c_str(),remotePath.size()-14);
				remotePath3 += buf;

				if(SendDataByFtp(strLocalPath3, remotePath3, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}

				string strLocalPath4;
				strLocalPath4.append(strLocalPath.c_str(),strLocalPath.size()-15);
				strLocalPath4 += ".ini";

				string remotePath4;
				remotePath4.append(remotePath.c_str(),remotePath.size()-15);
				remotePath4 += ".ini";

				if(SendDataByFtp(strLocalPath4, remotePath4, strMsg, &g_FtpCommunication))
				{
					nSendCount++;
				}
				else
				{
					return false;
				}
		}
		return true;
	}
    else
    {
        return true;
    }

	return false;
}

/*
* 函数介绍：获取一条历史记录
* 输入参数：strMsg-要获取的历史记录存储变量
* 输出参数：strMsg-获取的历史记录
* 返回值 ：成功返回true，否则返回false
*/
void mvCTJServer::mvDealHistoryRecord()
{
    int nCount = 0;
	time_t startTime;
	time_t nowTime;
	time(&startTime);

    //取类指针
    while(!g_bEndThread)
    {
        sleep(5);

        nCount++;

        if(nCount == 10)
        {
			if(g_sysInfo.fDisk>=90)
			{
				DelDir();
			}
            nCount = 0;
        }

		//发送历史记录
		if(g_nSendHistoryRecord == 1)
		{
			mvOrdOneRecord();
		}

		if( 10 == g_nServerType)
		{
			//发送录像
			SendVideo();
		}
		if( 7 == g_nServerType)
		{
			time(&nowTime);
			int offTime = (nowTime-startTime);
			if(offTime >= 60*60)
			{
				g_skpDB.DeleteVideoRecord();
				startTime = nowTime;
			}
			SendVideoForTianjin();

			struct timeval tv;
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			select(0,NULL, NULL, NULL, &tv);

		}
    }
}

bool mvCTJServer::SendVideoForTianjin()
{
	string strVideoPath;

	bool retServer = false;
	int nVideoType = 0;
	int nCameraID = 0;
	strVideoPath = g_skpDB.GetVideoRecord(nVideoType,nCameraID);
	m_nVideoType = nVideoType;

	
//#ifdef FBMATCHPLATE
	if(strVideoPath.size() > 0)
	{
		retServer =  SendDataByFtpForTianjinMatch(strVideoPath);
		if (retServer == true)
		{
			g_skpDB.UpdateVideoRecord(strVideoPath,1);
		}
	}
//#else
//	if(strVideoPath.size() > 0)
//	{
//		retServer =  SendDataByFtpForTianjin(strVideoPath);
//		if (retServer == true)
//		{
//			g_skpDB.UpdateVideoRecord(strVideoPath,1);
//		}
//	}
//#endif //FBMATCHPLATE
	
	return retServer;
}

bool mvCTJServer::SendDataByFtpForTianjin(string strLocalFilePath)
{
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
	{
		return false;
	}
	//LogNormal("strLocalFilePath:%s \n",strLocalFilePath.c_str());
	bool bRet = false;
	{
		char chRemoteDirPath[SRIP_MAX_BUFFER] = {0};
		string strDataPath = "/home/road/red/";
		if (IsDataDisk())
		{
				strDataPath = "/detectdata/red/";
		}
		String strTmpPath = strLocalFilePath;
		strTmpPath.erase(0,strDataPath.size());
		sprintf(chRemoteDirPath, "%s",strTmpPath.c_str());
		//LogNormal("chRemoteDirPath:%s \n",chRemoteDirPath);

		string strMsg = "";
		string remotePath(chRemoteDirPath);
		g_skpDB.UTF8ToGBK(remotePath);
		bRet = g_FtpCommunication.VideoDoPut((char *)strLocalFilePath.c_str(),(char *)remotePath.c_str(),strMsg,true,true,(char *)remotePath.c_str(),true);
	}

	return bRet;
}

//获取历史记录
void mvCTJServer::mvOrdOneRecord()
{
		//事件记录
		if(10 == g_nServerType)//鞍山交警
        {
		    sleep(2);
            StrList strListEvent;
			if(g_skpDB.GetEventHistoryRecord(strListEvent))//违章记录
			{
					StrList::iterator it_b = strListEvent.begin();
					while(it_b != strListEvent.end())
					{
						string strEvent("");
						strEvent = *it_b;
									
						bool bSendStatus = false;
						bSendStatus = mvSendRecordToCS(strEvent);

						if(bSendStatus)
						{
							UINT32 uSeq = *(UINT32*)(strEvent.c_str()+sizeof(MIMAX_HEADER));
							g_skpDB.UpdateRecordStatus(MIMAX_EVENT_REP, uSeq);
							sleep(5);
						}
						it_b++;
					}
			}
		}
		
		{
			#ifdef CAMERAAUTOCTRL
				StrList strListEvent;
				if(g_skpDB.GetEventHistoryRecord(strListEvent))//违章记录
				{
						StrList::iterator it_b = strListEvent.begin();
						while(it_b != strListEvent.end())
						{
							string strEvent("");
							strEvent = *it_b;
										
							bool bSendStatus = false;
							bSendStatus = mvSendRecordToCS(strEvent);

							if(bSendStatus)
							{
								UINT32 uSeq = *(UINT32*)(strEvent.c_str()+sizeof(MIMAX_HEADER));
								g_skpDB.UpdateRecordStatus(MIMAX_EVENT_REP, uSeq);
								sleep(5);
							}
							it_b++;
						}
				}
			#else
				//车牌记录
				if (m_bStarting == false)
				{
					m_bStarting = true;

					std::list<unsigned int> listSeq;
					listSeq.clear();
					StrList strListRecord;
					strListRecord.clear();
					if(g_skpDB.GetPlateHistoryRecord(strListRecord))//违章记录
					{
						StrList::iterator it_b = strListRecord.begin();
						while(it_b != strListRecord.end())
						{
							string strPlate("");
							strPlate = *it_b;

							RECORD_PLATE* sPlate = (RECORD_PLATE*)(strPlate.c_str()+sizeof(MIMAX_HEADER));
							int uViolationType = sPlate->uViolationType;

							UINT32 uSeq = *(UINT32*)(strPlate.c_str()+sizeof(MIMAX_HEADER));

							if(g_nSendImage == 1)
							{
								String strPicPath(sPlate->chPicPath);
								string strPic = GetImageByPath(strPicPath);
								MIMAX_HEADER* pHeader = (MIMAX_HEADER*)strPlate.c_str();
								pHeader->uCmdLen += strPic.size();
								strPlate.append((char*)strPic.c_str(),strPic.size());
							}

							bool bSendStatus = false;
							if(uViolationType > 0)
							{
								bSendStatus = mvSendRecordToCS(strPlate);
							}
							else
							{
								if(g_Kafka.uSwitchUploading == 1)
								{
									bSendStatus = mvSendRecordToCS(strPlate);
								}
								else
								{
									bSendStatus = true;
									listSeq.push_back(uSeq);
									it_b++;
									continue;
								}
							}

							if(bSendStatus)
							{
								//LogNormal("历史数据发送成功，【已发送】\n");
								listSeq.push_back(uSeq);
								sleep(5);
							}
							it_b++;
						}
						if(listSeq.size() > 0)
						g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, listSeq);
					}
					else
					{
						sleep(60);
					}
					m_bStarting = false;
				}
			#endif
		}
}

//通过ftp发送数据
bool mvCTJServer::SendDataByFtp(string strLocalPath,string strRemotePath,string& strMsg, CFtpCommunication *pFtp)
{
	if (pFtp->m_strFtpHost.empty() || pFtp->m_strFtpHost == "0.0.0.0" || pFtp->m_nFtpPort <= 0)
	{
		return false;
	}

	bool bRet = false;
	//LogNormal("mvCTJServer::[%s]:strLocalPath = %s\n",__FUNCTION__,strLocalPath);
	//LogNormal("%s\n",strRemotePath);
	if(strLocalPath.size() <= 0)
	{
		if (23 == g_nServerType)
		{
			bRet = pFtp->VideoDoPut(NULL,(char*)strRemotePath.c_str(),strMsg,true,true,(char*)strRemotePath.c_str(),false);
		}
		else
		{
			bRet = pFtp->VideoDoPut(NULL,(char*)strRemotePath.c_str(),strMsg,true,true,(char*)strRemotePath.c_str(),true);
		}
	}
	else
	{
		bRet = pFtp->VideoDoPut((char*)strLocalPath.c_str(),(char*)strRemotePath.c_str(),strMsg,true,true,(char*)strRemotePath.c_str(),true);
	}
	//cerr<<pFtp->m_strFtpHost<<"|"<<pFtp->m_nFtpPort<<"|"<<pFtp->m_strFtpUser<<"|"<<pFtp->m_strFtpPasswd<<endl;
	//cerr<<"DoPut return="<<bRet<<" |  "<<pFtp->m_strFtpUser<<endl;
	return bRet;
}
//通过ftp发送数据
bool mvCTJServer::SendDataByFtpForNanning(string strLocalPath,string strRemotePath,string& strMsg, CFtpCommunication *pFtp)
{
	if (pFtp->m_strFtpHost.empty() || pFtp->m_strFtpHost == "0.0.0.0" || pFtp->m_nFtpPort <= 0)
	{
		return false;
	}

	bool bRet = false;

	if(strLocalPath.size() <= 0)
	{
		bRet = pFtp->VideoDoPut(NULL,(char*)strRemotePath.c_str(),strMsg,true,true,(char*)strRemotePath.c_str(),true);
	}
	else
	{
		bRet = pFtp->VideoDoPut((char*)strLocalPath.c_str(),(char*)strRemotePath.c_str(),strMsg,true,true,(char*)strRemotePath.c_str(),true);
	}
	//cerr<<pFtp->m_strFtpHost<<"|"<<pFtp->m_nFtpPort<<"|"<<pFtp->m_strFtpUser<<"|"<<pFtp->m_strFtpPasswd<<endl;
	//cerr<<"DoPut return="<<bRet<<" |  "<<pFtp->m_strFtpUser<<endl;
	return bRet;
}

//建立录像软链接
void mvCTJServer::AddVideo(RECORD_PLATE& plate)
{
	pthread_mutex_lock(&m_Video_Mutex);
	m_listVideo.push_back(plate);
	pthread_mutex_unlock(&m_Video_Mutex);

	printf("======AddVideo=======**********************m_listVideo.size()=%d\n",m_listVideo.size());
}

//删除时间目录
void mvCTJServer::DelDir()
{
	   char buf[1024] = {0};
	   for(int i = 0;i<1;i++)
	   {
		   string strFtpDir = "/home/road/server/pic";
		   if (IsDataDisk())
		   {
			   strFtpDir = "/detectdata/pic";
		   }
		   if(strFtpDir.size() > 0)
			{
				   std::string strPathName2 = strFtpDir;

				   DIR* dir=NULL;
				   struct dirent* ptr=NULL;

				   dir = opendir(strPathName2.c_str());

					if(dir)
					{
						int nMinID = 0;
						int nID = 0;

						int nCountID = 0;

					   while((ptr=readdir(dir))!=NULL)
					   {
						 if((strcmp(".",ptr->d_name)!=0)&&
							 (strcmp("..",ptr->d_name)!=0))
						 {
							 std::string strPathName(ptr->d_name);

							 printf("***************strPathName.c_str()=%s\n",strPathName.c_str());

							 if(strPathName.size() == 8 || strPathName.size() == 10)
							 {
								nID = atoi(strPathName.c_str());

								if(nMinID == 0)
								{
									nMinID = nID;
								}

								if(nID < nMinID)
								{
									nMinID = nID;
								}

								nCountID++;
							 }
						 }
					   }
					   closedir(dir);

						if(nCountID > 7)
						{
							sprintf(buf,"%s/%d",strPathName2.c_str(),nMinID);
							std::string strPathName(buf);

							RemoveDir(strPathName.c_str(),i);//删掉日期目录
						}
					}
					//如果存在方向文件夹
					if( (g_nDetectMode == 2) && (g_nServerType == 7))
					{
						dir = opendir(strPathName2.c_str());
						if(dir)
						{
							 while((ptr=readdir(dir))!=NULL)
							 {
								if((strcmp(".",ptr->d_name)!=0)&&
										 (strcmp("..",ptr->d_name)!=0))
								{
									//方向目录
									std::string strPathName(ptr->d_name);
									// LogNormal("1 strPathName.c_str()=%s\n",strPathName.c_str());

									if( (strncmp(strPathName.c_str(),"20",2)!= 0) && (strncmp(strPathName.c_str(),"19",2)!= 0))
									{
										memset(buf,0,1024);
										sprintf(buf,"%s/%s",strPathName2.c_str(),strPathName.c_str());
										std::string strDirectionName(buf);
										// LogNormal("1 strDirectionName.c_str()=%s\n",strDirectionName.c_str());

										DIR* dir1=NULL;
										struct dirent* ptr1=NULL;
										dir1 = opendir(strDirectionName.c_str());
										if(dir1 != NULL)
										{
											int nMinID = 0;
											int nID = 0;
											int nCountID = 0;
											while((ptr1=readdir(dir1))!=NULL)
										   {
											 if((strcmp(".",ptr1->d_name)!=0)&&
												 (strcmp("..",ptr1->d_name)!=0))
											 {
												 std::string strPathName(ptr1->d_name);

												 //LogNormal("strPathName.c_str()=%s\n",strPathName.c_str());

												 if(strPathName.size() == 8 || strPathName.size() == 10)
												 {
													nID = atoi(strPathName.c_str());

													if(nMinID == 0)
													{
														nMinID = nID;
													}

													if(nID < nMinID)
													{
														nMinID = nID;
													}

													nCountID++;
												 }
											 }
										   }
										   closedir(dir1);


											if(nCountID > 7)
											{
												sprintf(buf,"%s/%d",strDirectionName.c_str(),nMinID);
												std::string strPathName(buf);

												RemoveDir(strPathName.c_str(),i);//删掉日期目录
											}

										}
									}
								}
							}
							closedir(dir);
						}
					}
			   }
	   }
}

//递归删除目录
bool mvCTJServer::RemoveDir( const char* dirname,int nType)
{
	DIR* dir=NULL;
	struct dirent* ptr=NULL;
	dir = opendir(dirname);

	printf("dirname=%s\n",dirname);

	// 初始化为0
	char buf[1024] = {0};

	if(dir)
	{
		while((ptr=readdir(dir))!=NULL)
		{
			if((strcmp(".",ptr->d_name)!=0)&&
				(strcmp("..",ptr->d_name)!=0))
			{
				sprintf(buf,"%s/%s",dirname,ptr->d_name);
				std::string strPath(buf);
				printf("strPath=%s\n",strPath.c_str());

				//判断是否为目录
				struct stat st;
				stat(strPath.c_str(),&st);
				int n = S_ISDIR(st.st_mode);

				if(n==1)//目录
				{
					RemoveDir(strPath.c_str(),nType);
				}
				else if(n==0) //文件
				{
					//umount(strPath.c_str());
					remove(strPath.c_str());
					//删除已经存在的记录
					if(nType == 0 || nType == 3)
					{
						g_skpDB.DeleteOldRecord(strPath,false,false);
					}
					else if(nType == 2)
					{
						g_skpDB.DeleteOldRecord(strPath,false,true);
					}
					usleep(1000*500);
				}
			}
		}
		closedir(dir);
	}

	if(dirname)
		rmdir(dirname);

	return true;
}

//发送录像
void mvCTJServer::SendVideo()
{	
	RECORD_PLATE plate;

	int nVideoSize = 0;

	pthread_mutex_lock(&m_Video_Mutex);
	if(m_listVideo.size() > 0)
	{
		//取最早录像
		VIDEO_LIST::iterator it = m_listVideo.begin();
		//保存数据
		plate = *it;
		//删除取出的录像
		m_listVideo.pop_front();

		nVideoSize = 1;
	}
	pthread_mutex_unlock(&m_Video_Mutex);

	printf("**********************m_listVideo.size()=%d,nVideoSize=%d\n",m_listVideo.size(),nVideoSize);
	
	if(nVideoSize > 0)
	{
		string strVideoPath(plate.chVideoPath);
	    
		if(strVideoPath.size() > 0)
		{
			string strMsg;
			string remotePath = g_RoadImcData.GetFtpRemoteDir(&plate);
			SendDataByFtp(strVideoPath, remotePath, strMsg, &m_FtpVideo);
		}
	}
}

bool mvCTJServer::LoadFtpConfig()
{
	string fatherNode[5] = {"KaKou", "BreakRules", "Statistic", "Video","Status"};
	string subNode[4] = {"HostIp", "UserName", "Password", "Port"};
	string defaultUser[5] = {"kakou", "red", "flow", "video", "status"};

	XMLNode xml, temp, setting;
	string xmlFile = "./TJCenterFtpCfg.xml";

	char buf[128] = {0};

	//判断xml文件是否存在
	if(access(xmlFile.c_str(),F_OK) != 0)//不存在
	{
		//载入默认配置
		m_FtpKakou.SetConfig();
		m_FtpBreakRules.SetConfig();
		m_FtpStatistic.SetConfig();
		m_FtpVideo.SetConfig();
		m_FtpStatus.SetConfig();
		m_FtpKakou.m_strFtpUser = defaultUser[0];
		m_FtpBreakRules.m_strFtpUser = defaultUser[1];
		m_FtpStatistic.m_strFtpUser = defaultUser[2];
		m_FtpVideo.m_strFtpUser = defaultUser[3];
		m_FtpStatus.m_strFtpUser = defaultUser[4];
		return false;
	}
	else
	{
		//xml文件存在
		XMLCSTR tmpStr;
		xml = XMLNode::parseFile(xmlFile.c_str());
		xml = xml.getChildNode("TJFtpSetting");
		if (!xml.isEmpty())
		{
			for (int i = 0; i < 5; i++)
			{
				setting = xml.getChildNode(fatherNode[i].c_str());
				if (!setting.isEmpty())
				{
					for (int j = 0; j < 5; j++)
					{
						temp = setting.getChildNode(subNode[j].c_str());
						if (!temp.isEmpty())
						{
							tmpStr = temp.getText();
							if (strcmp(temp.getName(), "HostIp") == 0)
								memcpy(&m_FtpSetting[i].chFtpServerHost, tmpStr, strlen(tmpStr));
							else if (strcmp(temp.getName(), "UserName") == 0)
								memcpy(&m_FtpSetting[i].chFtpUserName, tmpStr, strlen(tmpStr));
							else if (strcmp(temp.getName(), "Password") == 0)
								memcpy(&m_FtpSetting[i].chFtpPassword, tmpStr, strlen(tmpStr));
							else if (strcmp(temp.getName(), "Port") == 0)
								m_FtpSetting[i].uFtpPort = atoi(tmpStr);
						}
					}
				}
			}
		}
		//载入配置文件中的参数
		m_FtpKakou.SetConfig(m_sKakouSetting);
		m_FtpBreakRules.SetConfig(m_sBreakRulesSetting);
		m_FtpStatistic.SetConfig(m_sStatisticSetting);
		m_FtpVideo.SetConfig(m_sVideoSetting);
		m_FtpStatus.SetConfig(m_sStatus);
	}
	return true;
}

//通过大图路径,拆分成小文件
bool mvCTJServer::DepachBigJpg(
									 std::string &strPicPath, 
									 int nWidthBig, 
									 int nHeightBig, 
									 unsigned char* pOutJpgBuf[], 
									 int nJpgSize[])
{
	std::string strPic;
	int nWidth = nWidthBig/2;
	int nHeight = nHeightBig/2;

	//读取大图
	strPic = GetImageByPath(strPicPath);
	
	printf("---strPic.size()=%d \n", strPic.size());

	//解码大图
	//需要解码jpg图像
	CxImage image;
	image.Decode((BYTE*)(strPic.c_str()), strPic.size(), 3); //解码

	printf("-22----DepachBigJpg nWidthBig:%d, nHeightBig:%d", nWidthBig, nHeightBig);
		

	CxImage imageSmall;
	if(nWidth > 2000)
	{
		
		cvSet(pImgBig500, cvScalar( 0,0, 0 ));
		if(image.IsValid()&&image.GetSize()>0)
		{
			memcpy(pImgBig500->imageData,image.GetBits(),image.GetSize());
		}
		else
		{
			LogNormal("解码大图失败\n");
		}
				
		printf("pImgBig500 nSize:%d \n", pImgBig500->nSize);

		if(pImgBig500)
		{
			//拆分大图
			DePachBigImg(pImgBig500, g_nVtsPicMode, pImgArray500, nWidth, nHeight);

			for(int i=0; i<4; i++)
			{
				bool bJpgSmall = imageSmall.IppEncode(
					(BYTE*)(pImgArray500[i]->imageData), 
					nWidth, nHeight, 
					3,
					&nJpgSize[i], 
					pOutJpgBuf[i], 
					g_PicFormatInfo.nJpgQuality);

				if(!bJpgSmall)
				{
					LogError("切分小图失败!\n");
				}
			}
		}
		
	}
	else
	{
		cvSet(pImgBig200, cvScalar( 0,0, 0 ));
		if(image.IsValid()&&image.GetSize()>0)
		{
			memcpy(pImgBig200->imageData,image.GetBits(),image.GetSize());
		}
		else
		{
			LogNormal("解码大图失败\n");
		}

		printf("pImgBig200 nSize:%d \n", pImgBig200->nSize);

		if(pImgBig200)
		{
			//拆分大图
			DePachBigImg(pImgBig200, g_nVtsPicMode, pImgArray200, nWidth, nHeight);

			for(int i=0; i<4; i++)
			{
				bool bJpgSmall = imageSmall.IppEncode(
					(BYTE*)(pImgArray200[i]->imageData), 
					nWidth, nHeight, 
					3,
					&nJpgSize[i], 
					pOutJpgBuf[i], 
					g_PicFormatInfo.nJpgQuality);

				if(!bJpgSmall)
				{
					LogError("切分小图失败!\n");
				}
			}
		}
	}
	

	printf("-333----DepachBigJpg nWidthBig:%d, nHeightBig:%d", nWidthBig, nHeightBig);
}

//拆分大图成小图
bool mvCTJServer::DePachBigImg(IplImage* pImgBig, int nPicMode, IplImage* pImgArray[], int nWidth, int nHeight)
{
	bool bRet = false;

	int w = 0;
	int h = 0;
	CvRect rect;
	CvRect rt;	

	//组合方式(0:3x1,1:2x2,2:3x2,3:1x2,4:1x1,5:1x3)g_nVtsPicMode
	if(nPicMode == 0)
	{
		//
	}
	else if(nPicMode == 1)
	{
		rt.x = 0;
		rt.y = 0;
		rt.width = nWidth;
		rt.height = nHeight;

		for(int i=0; i<4; i++)
		{
			GetImgRect(nPicMode, nWidth*2, nHeight*2, i, rect);

			printf("111 rect:%d,%d,%d,%d \n",rect.x, rect.y, rect.width, rect.height);
			printf("222 rt:%d,%d,%d,%d \n", rt.x, rt.y, rt.width, rt.height);

			cvSetImageROI(pImgArray[i],rt);
			cvSetImageROI(pImgBig,rect);
			cvResize(pImgBig, pImgArray[i]);
			cvResetImageROI(pImgBig);
			cvResetImageROI(pImgArray[i]);
		}
	}
	else
	{
		//
	}

	return bRet;
}

//获取小图拆分区域
bool mvCTJServer::GetImgRect(int nPicMode, int nWidth , int nHeight, int i, CvRect &rect)
{
	bool bRet = false;

	int w = 0;
	int h = 0;

	//组合方式(0:3x1,1:2x2,2:3x2,3:1x2,4:1x1,5:1x3)g_nVtsPicMode
	if(nPicMode == 0)
	{
		//
	}
	else if(nPicMode == 1)
	{
		w = nWidth/2;
		h = nHeight/2;

		rect.x = w * (i%2);
		rect.y = h * (i/2);
		rect.width = w;
		rect.height = h;

		bRet = true;
	}
	else
	{
		//
	}
	return bRet;
}

bool mvCTJServer::InitImg200(int w, int h)
{
	bool bRet = false;
	//创建切分图片缓存
	pImgBig200 = cvCreateImage(cvSize(w*2,h*2),8,3);

	for(int i=0; i<4; i++)
	{
		pImgArray200[i] = NULL;
		pImgArray200[i] = cvCreateImage(cvSize(w, h),8,3);
		cvSet(pImgArray200[i], cvScalar( 0,0, 0 ));			 
	}

	if(pImgBig200)
	{
		bInitImgBig200 = true;
		bRet = true;
	}


	return bRet;
}

bool mvCTJServer::InitImg500(int w, int h)
{
	bool bRet = false;
	//创建切分图片缓存
	pImgBig500 = cvCreateImage(cvSize(w*2,h*2),8,3);

	for(int i=0; i<4; i++)
	{
		pImgArray500[i] = NULL;
		pImgArray500[i] = cvCreateImage(cvSize(w, h),8,3);
		cvSet(pImgArray500[i], cvScalar( 0,0, 0 ));			 
	}

	if(pImgBig500)
	{
		bInitImgBig500 = true;
		bRet = true;
	}

	return bRet;
}

//过滤军车,武警,警车记录
bool mvCTJServer::JunCheck(const RECORD_PLATE & plate)
{
	bool bRet = false;
	String strCarNumber(plate.chText);
	//g_skpDB.UTF8ToGBK(strCarNumber);
	String strTemp = "";
	string::size_type iPos;

	int nType = 0;

	//警
	strTemp = "警";
	iPos = strCarNumber.find(strTemp);
	if(iPos != string::npos)
	{
		nType = 1;
	}

	//军
	strTemp = "军";
	iPos = strCarNumber.find(strTemp);
	if(iPos != string::npos)
	{
		nType = 2;
	}

	//WJ(武警)
	strTemp = "WJ";
	iPos = strCarNumber.find(strTemp);
	if(iPos != string::npos)
	{
		nType = 3;
	}

	if(nType > 0)
	{
		//LogNormal("strCarNumber:%s ", strCarNumber.c_str());
		LogNormal("nType:%d iPos:%d bRet:%d", nType, iPos, bRet);
		bRet = true;
	}
	return bRet;
}

bool mvCTJServer::SendDataByFtpForTianjinMatch(string strLocalFilePath)
{
	bool bRet = false;
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
	{
		return false;
	}
	//LogNormal("strLocalFilePath:%s \n",strLocalFilePath.c_str());

	bool bRet1 = SendFileByDirection(strLocalFilePath, 0);
	bool bRet2 = SendFileByDirection(strLocalFilePath, 1);			

	if(bRet1 && bRet2)
	{
		bRet = true;
	}

	return bRet;
}


//根据检测方向,取不同路径录像,发送
bool mvCTJServer::SendFileByDirection(string strLocalFilePath, const int nDetectDirection)
{
	bool bRet = false;

	if(strLocalFilePath.size() > 0)
	{
		char chRemoteDirPath[SRIP_MAX_BUFFER] = {0};
		string strTmpPath = strLocalFilePath;

		if(g_nServerType == 7)
		{
			string strDataPath = "/home/road/red/";
			if (IsDataDisk())
			{
				strDataPath = "/detectdata/red/";
			}
			strTmpPath.erase(0,strDataPath.size());
			//strTmpPath = g_ServerHost+strTmpPath;
			//strTmpPath = "ftp://" + strTmpPath;
		}

		//去除文件后缀,X-M-0.avi-->X-M
		int pos = strTmpPath.find(".avi");
		//string strTmpPathHead = strTmpPath.substr(0, iFlag);		
		//sprintf(chRemoteDirPath, "%sM-%d.avi", strTmpPathHead.c_str(), nDetectDirection);
		if(1 == nDetectDirection)
		{
			strTmpPath.replace(pos-1, 1, "1");
		}
		else
		{
			strTmpPath.replace(pos-1, 1, "0");
		}		

		string remotePath = strTmpPath;
		LogTrace("FBMach.log", "remotePath:%s", remotePath.c_str());
		LogTrace("FBMach.log", "strLocalFilePath:%s", strLocalFilePath.c_str());

		string strMsg = "";
		//string remotePath(chRemoteDirPath);
		g_skpDB.UTF8ToGBK(remotePath);
		bRet = g_FtpCommunication.VideoDoPut((char *)strLocalFilePath.c_str(),
			(char *)remotePath.c_str(),strMsg,true,true,(char *)remotePath.c_str(),true);
	}	
	else
	{
		bRet = false;
		LogError("取SendFileByDirection ftp路径失败!\n");
	}

	return bRet;
}
