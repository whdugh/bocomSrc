
#include "Common.h"
#include "CommonHeader.h"
#include "CenterServerOneDotEight.h"
#include "XmlParaUtil.h"
#include <sys/statvfs.h>
#include "ximage.h"
#include "ippi.h"
#include "ippcc.h"
#include "BrandSubSection.h"
#include "FtpCommunication.h"
#include "Filemd5.h"
#include "CarLabel.h"

#ifndef BYTE
typedef unsigned char BYTE;
#endif



bool g_MybCenterLink = false;
string g_MystrCSMsgVer = "1.81";
mvCCenterServerOneDotEight g_MyCenterServer;

//#define CS_STUPID_LOG

//#ifdef CS_STUPID_LOG
//void CSSLog(const char *pLog)
//{
//    FILE *pCSSLog = fopen("cs_stupid.log", "a");
//    if (pCSSLog != NULL)
//    {
//        fprintf(pCSSLog, pLog);
//        fflush(pCSSLog);
//        fclose(pCSSLog);
//    }
//}
//#endif  //CS_STUPID_LOG

static const unsigned int mycrcTable[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/*
* 函数介绍：接收消息线程入口
* 输入参数：pArg-线程入口参数，接收套接字
* 输出参数：无
* 返回值 ：无
*/
void *MyRecvCSMsgThread(void *pArg)
{
    g_MyCenterServer.mvRecvCenterServerMsg();
 
    LogError("Ver1.81接收中心端消息线程退出\r\n");
    pthread_exit((void *)0);
	return pArg;
}

//记录发送线程
void* MyThreadCSRecorderResult(void* pArg)
{
	//取类指针
	mvCCenterServerOneDotEight* pCenterServer = (mvCCenterServerOneDotEight*)pArg;
	if(pCenterServer == NULL) return pArg;

	//处理一条数据
	pCenterServer->DealResult();
    pthread_exit((void *)0);
	return pArg;
}

//卡口记录磁盘管理线程
void* MyThreadDiskManager(void* pArg)
{
	//取类指针
	mvCCenterServerOneDotEight* pCenterServer = (mvCCenterServerOneDotEight*)pArg;
	if(pCenterServer == NULL) return pArg;

	//处理一条数据
	pCenterServer->ManagerDisk();
	pthread_exit((void *)0);
	return pArg;
}

//重连线程
void* MyThreadLinkTest(void* pArg)
{
	//取类指针
	mvCCenterServerOneDotEight* pCenterServer = (mvCCenterServerOneDotEight*)pArg;
	if(pCenterServer == NULL) return pArg;

	//重连
	pCenterServer->LinkTest();
	pthread_exit((void *)0);
	return pArg;
}

//历史记录发送线程
void* MyThreadHistoryResult(void* pArg)
{
	//取类指针
	mvCCenterServerOneDotEight* pCenterServer = (mvCCenterServerOneDotEight*)pArg;
	if(pCenterServer == NULL)
		return pArg;

     while(!g_bEndThread)
     {
		 //LogNormal("History Send before:%s\n",GetTimeCurrent().c_str());
        //处理一条数据
	    pCenterServer->mvCheckFreeAndDealHistoryRecord();
		//LogNormal("History Send after:%s\n",GetTimeCurrent().c_str());
		//5秒
		struct timeval tv;
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
     }
     pthread_exit((void *)0);
	return pArg;
}


/*
* 函数介绍：构造函数
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCCenterServerOneDotEight::mvCCenterServerOneDotEight()
{
////#ifdef CS_STUPID_LOG
////    CSSLog("In mvCCenterServerOneDotEight::mvCCenterServerOneDotEight()\n");
////#endif
    m_nControlSocket = -1;
    m_nCenterSocket = -1;
    m_nCSLinkCount = 0;
    m_nConnectTime = 0;
    m_uCSLinkTime = 0;
    m_bHistoryRep = false;
    //m_strDetectorId = "01234567890abcd";
	m_strDetectorId = "";
    m_uHistoryRepTime = 0;

    char szBuff[2] = {0};
    sprintf(szBuff, "%c", 0x2);
    m_strHeader = szBuff;

    memset(szBuff, 0, 2);
    sprintf(szBuff, "%c", 0x3);
    m_strEnd = szBuff;

    m_nAllDisk = 64 * 1024;

	
	m_uBeginTime = "";
	m_uEndTime = "";
	m_uForceTransferType = ' ';
	m_uForceTransCount = 0;
	m_uForceTransBeginTime = "";
	m_uForceTransEndTime = "";
	m_uRealTimeRep = false;
	//m_uSavePicCount = 100;
	m_uMaxSpeedMap.clear();
	

//#ifdef CS_FILE_LOG
//    m_pCSRecvLog = fopen("cs_recv.log", "w");
//    m_pCSSendLog = fopen("cs_send.log", "w");
//    m_pCSConnLog = fopen("cs_conn.log", "w");
//#endif

    pthread_mutex_init(&m_mutexMsg, NULL);
    pthread_mutex_init(&m_Result_Mutex,NULL);

    m_ChannelResultList.clear();
    m_nThreadId = 0;
	m_vLoopStatus.clear();
	m_vChanIdLoopStatus.clear();
	m_uChanMaxSpeedStrMap.clear();
	m_uRoadCount = 0;
	m_uCameraCount = 0;

	m_uPicWidth = 0;
	m_uPicHeight = 0;
	m_uExtentHeight = 0;
	m_uRatio = 1.0;

	m_nDiskManagerThreadId = 0;
	pidon = -1;
	pidoff = -1;

	m_nCpuAlarmCount = 0;
	m_bEndWCDMAThread = false;

	m_bFtpUpdate = false;
	m_bUpdateOnce = false;
	m_nFtpUpdateHour = 0;
	m_PlateMap.clear();
	m_strRemoteFile = "";
	m_bEndRecvThread = false;
	m_nRecvThreadId = 0;
	m_nConnectCount = 0;
	m_nSendFailCount = 0;
	m_uCSTime = 0;

	m_nLinkThreadId = 0;
	m_nHistoryThreadId = 0;
}

/*
* 函数介绍：析构函数
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCCenterServerOneDotEight::~mvCCenterServerOneDotEight()
{
////#ifdef CS_STUPID_LOG
////    CSSLog("In mvCCenterServerOneDotEight::~mvCCenterServerOneDotEight()\n");
////#endif
    if (!m_mapCSMsg.empty())
    {
        m_mapCSMsg.clear();
    }

////#ifdef CS_FILE_LOG
////    if (m_pCSRecvLog != NULL)
////    {
////        fclose(m_pCSRecvLog);
////        m_pCSRecvLog = NULL;
////    }
////    if (m_pCSSendLog != NULL)
////    {
////        fclose(m_pCSSendLog);
////        m_pCSSendLog = NULL;
////    }
////    if (m_pCSConnLog != NULL)
////    {
////        fclose(m_pCSConnLog);
////        m_pCSConnLog = NULL;
////    }
////#endif

    pthread_mutex_destroy(&m_mutexMsg);
    pthread_mutex_destroy(&m_Result_Mutex);
	m_vLoopStatus.clear();
	m_vChanIdLoopStatus.clear();
	m_uChanMaxSpeedStrMap.clear();
	m_bEndWCDMAThread = true;
}

//初始化
bool mvCCenterServerOneDotEight::Init()
{
	if(g_nExist3G == 1)//存在无线网卡才拨号
	{
		if(g_n3GTYPE != 1)
		{
			LogNormal("CreateWCDMAThread\n");
			CreateWCDMAThread();
		}
	}

	if(access("../../dzjc",0) != 0) //目录不存在
	{
		system("useradd -m -s /bin/bash -p $(perl -e 'print crypt(\"d2z0j1c1\", \"aa\")') -N dzjc");
	}

	if(access("/etc/vsftpd_user_config",0) != 0)
	{
		mkdir("/etc/vsftpd_user_config",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
	}

	/*FILE* fp = fopen("/etc/vsftpd.chroot_list","wb");
	if(fp != NULL)
	{
		fprintf(fp,"road\r\ndzjc\r\n");
		fclose(fp);
	}*/

	FILE* fp = fopen("/etc/vsftpd_user_config/road","wb");
	if(fp != NULL)
	{
		fprintf(fp,"local_root=/home/road/server/update");
		fclose(fp);
	}

	if(!IsDataDisk())
	{
		if(access("/home/road/dzjc/",0) != 0) //目录不存在
		{
			mkdir("/home/road/dzjc/",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		}

		FILE* fp = fopen("/etc/vsftpd_user_config/dzjc","wb");
		if(fp != NULL)
		{
			fprintf(fp,"local_root=/home/road/dzjc");
			fclose(fp);
		}
	}
	else
	{
		if(access("/detectdata/dzjc/",0) != 0) //目录不存在
		{
			mkdir("/detectdata/dzjc/",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		}

		FILE* fp = fopen("/etc/vsftpd_user_config/dzjc","wb");
		if(fp != NULL)
		{
			fprintf(fp,"local_root=/detectdata/dzjc");
			fclose(fp);
		}
	}
	//LogNormal("in mvCCenterServerOneDotEight::Init()\n");
	CXmlParaUtil xml;
	//xml.UpdateSystemSetting("DkCenterServerSetting", "");
	/*LogNormal("g_dklimitSpeed:%d\n",g_dkLimitSpeed);*/
	LogNormal("g_dkTransWay:%d\n",g_dkTransWay);
	LogNormal("g_dkMinute:%d\n",g_dkMinute);
	//LogNormal("g_dkCrossingCode:%s\n",g_dkCrossingCode);

	char szBuff[20] = {0};

    sprintf(szBuff, "%15s", g_strDetectorID.c_str());
	printf("szBuff=%s\n",szBuff);
	m_strDetectorId = szBuff;
	LogNormal("m_strDetectorId:%s\n",m_strDetectorId.c_str());

	SetRoadCount();
	LogNormal("车道总数:%d\n",m_uRoadCount);

	//xml.GetMaxSpeed(m_uMaxSpeedMap, 1);
	////LogNormal("111m_uMaxSpeedMap.size():%d\n",m_uMaxSpeedMap.size());
	/////////////////////test
	/*map<UINT32,UINT32>::iterator it = m_uMaxSpeedMap.begin();
	while(it != m_uMaxSpeedMap.end())
	{
		LogNormal("车道%d,限速：%d\n",it->first,it->second);
		it++;
	}*/
	//////////////////test
    //线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);

	//启动检测结果发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	int nret=pthread_create(&m_nThreadId,&attr,MyThreadCSRecorderResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("Ver1.81创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动重连线程
	nret=pthread_create(&m_nLinkThreadId,&attr,MyThreadLinkTest,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("Ver1.81创建重连线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//历史记录发送线程
	nret=pthread_create(&m_nHistoryThreadId,&attr,MyThreadHistoryResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("Ver1.81创建历史记录发送线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}


	//启动磁盘管理线程
	nret=pthread_create(&m_nDiskManagerThreadId,&attr,MyThreadDiskManager,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("Ver1.81创建启动磁盘管理线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	char strcvsFile[1024] = {0};
	sprintf(strcvsFile,"./config/hmd_bocom.csv");
	GetFileLine(strcvsFile);

	pthread_attr_destroy(&attr);
	return true;
}

//释放
bool mvCCenterServerOneDotEight::UnInit()
{
	m_bEndWCDMAThread = true;
    mvOnEndWork();
    //停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}

	if (m_nDiskManagerThreadId != 0)
	{
		pthread_join(m_nDiskManagerThreadId,NULL);
		m_nDiskManagerThreadId = 0;
	}

	if (m_nLinkThreadId != 0)
	{
		pthread_join(m_nLinkThreadId,NULL);
		m_nLinkThreadId = 0;
	}

	if (m_nHistoryThreadId != 0)
	{
		pthread_join(m_nHistoryThreadId,NULL);
		m_nHistoryThreadId = 0;
	}
	
	m_uMaxSpeedMap.clear();
    m_ChannelResultList.clear();
	return true;
}

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCCenterServerOneDotEight::mvConnOrLinkTest()
{
	//LogNormal("in mvCCenterServerOneDotEight::mvConnOrLinkTest()\n");
//#ifdef CS_FILE_LOG
//    fprintf(m_pCSConnLog, "\nmvConnOrLinkTest, host=%s\tport=%d\n",
//                            g_strControlServerHost.c_str(), g_nControlServerPort);
//                    fflush(m_pCSConnLog);
//#endif
					//LogNormal("g_bEndThread:%d,g_MybCenterLink:%d\n",g_bEndThread,g_MybCenterLink);
    if (!g_bEndThread)
    {
        if (!g_MybCenterLink)
        {
			
            {
                if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
                {
					
//#ifdef CS_FILE_LOG
//                    fprintf(m_pCSConnLog, "\nVer1.8中心路由服务器连接参数异常! host=%s\tport=%d\n",
//                            g_strControlServerHost.c_str(), g_nControlServerPort);
//                    fflush(m_pCSConnLog);
//#endif
				//	LogError("Ver1.81中心路由服务器连接参数异常!\n");
				//	LogError("host=%s,port=%d\n",g_strControlServerHost,g_nControlServerPort);
                    return;
                }
                else
                {
                    if (!mvPrepareSocket(m_nControlSocket))
                    {
						
//#ifdef CS_FILE_LOG
//                        fprintf(m_pCSConnLog, "\nVer1.8准备连接中心路由服务器套接字失败!\n");
//                        fflush(m_pCSConnLog);
//#endif
						LogError("Ver1.81准备连接中心路由服务器套接字失败!\n");
                        return;
                    }
					

                    if (!mvWaitConnect(m_nControlSocket, g_strControlServerHost, g_nControlServerPort))
                    {
						sleep(5);
						
//#ifdef CS_FILE_LOG
//                        fprintf(m_pCSConnLog, "\nVer1.8尝试连接中心路由服务器失败，将直接连接原中心数据服务器!\n");
//                        fflush(m_pCSConnLog);
//#endif
						LogError("Ver1.81尝试连接中心路由服务器失败，将直接连接原中心数据服务器!\n");
                        if(m_nConnectTime >3)
                        {
                            m_nConnectTime = 0;
                        }
                        if(++m_nConnectTime==3)//尝试连接中心路由服务器3次失败后连接上次分配的中心数据服务器
                        {
                            LogError("three times over连接中心路由服务器3次失败后直接连接原中心数据服务器\n");
                            if (mvConnCSAndRecvMsg())
                            {
                                g_MybCenterLink = true;
                                LogNormal("直接连接中心数据服务器成功!\n");
                                m_uCSLinkTime = GetTimeStamp();
                                m_nCSLinkCount = 0;
                            }
                            else
                            {
                                LogError("直接连接中心数据服务器失败\n");
                            }
                            m_nConnectTime  = 0;
                        }
                    }
                    else
                    {
//#ifdef CS_FILE_LOG
//                        fprintf(m_pCSConnLog, "\nVer1.8连接中心路由服务器成功，正在请求中心数据服务器IP和PORT...\n");
//                        fflush(m_pCSConnLog);
//#endif
                        LogNormal("连接中心路由服务器Ver1.81成功，正在请求中心数据服务器IP和PORT...\n");
                        if (mvRequestAndConnCS())
                        {
//#ifdef CS_FILE_LOG
//                            fprintf(m_pCSConnLog, "\nVer1.8请求中心数据服务器IP和PORT成功，并连接成功!\n");
//                            fflush(m_pCSConnLog);
//#endif
                            LogNormal("请求中心数据服务器Ver1.81 IP和PORT成功!\n");
                            g_MybCenterLink = true;
                            m_nCSLinkCount = 0;
							unsigned int uTimeStamp = GetTimeStamp();
							m_uCSLinkTime = uTimeStamp;
                        }
                        else
                        {
/*#ifdef CS_FILE_LOG
                            fprintf(m_pCSConnLog, "\nVer1.8请求中心数据服务器IP和PORT失败，或请求成功后连接失败!\n");
                            fflush(m_pCSConnLog);
#endif
          */                  LogError("请求中心数据服务器Ver1.81 IP和PORT失败，或请求成功后连接失败\n");
                            //printf("\n请求中心数据服务器IP和PORT失败!\n");
                            mvCloseSocket(m_nControlSocket);

                            if (mvConnCSAndRecvMsg())//请求失败后直接连接
                            {
                                g_MybCenterLink = true;
                                LogNormal("请求失败后，Ver1.81直接连接原先的中心数据服务器成功!\n");
                                m_uCSLinkTime = GetTimeStamp();
                                m_nCSLinkCount = 0;
                            }
                            else
                            {
                                LogError("请求失败后，直接连接原先的中心数据服务器Ver1.81失败\n");
                            }
                        }
                    }
                }
            }
        }
        else
        {
////#ifdef CS_FILE_LOG
////            fprintf(m_pCSConnLog, "Ver1.81中心数据服务器连接正常，ip=%s,port=%d,socket=%d\n",
////                    g_strCenterServerHost.c_str(), g_nCenterServerPort, m_nCenterSocket);
////            fflush(m_pCSConnLog);
////#endif
			//LogNormal("ready to send Linkpackage\n");
            unsigned int uTimeStamp = GetTimeStamp();
           // LogNormal("中心数据服务器连接正常，ip=%s,port=%d,socket=%d\n", g_strCenterServerHost.c_str(), g_nCenterServerPort, m_nCenterSocket);
            if ( (uTimeStamp >= m_uCSLinkTime+120) && (m_uCSLinkTime >0) )
			{
                if (!mvSendLinkTest())
                {
                    LogError("发送心跳包失败\n");
                }
                else
                {
                   // LogNormal("发送心跳包成功!\n");
                }
				m_uCSLinkTime = uTimeStamp;

                if (m_nCSLinkCount++ >= SRIP_LINK_MAX)
                {
                    mvCloseSocket(m_nCenterSocket);
                    g_MybCenterLink = false;
                    m_nCSLinkCount = 0;
                    LogError("长时间未收到心跳包，连接断开\n");
					//LogError("No Linkpackage for a long time,Connect out\n");
                    return;
                }
            }
            //检测设备状态
            mvCheckDeviceStatus(0);
        }
    }
}

//返回强制重传类型
char mvCCenterServerOneDotEight::GetForceType()
{
	return m_uForceTransferType;
}

//根据时间戳确定是否在补传时间段
bool mvCCenterServerOneDotEight::IsTransferMoment(unsigned int uTimeStamp)
{
	int day;
	int hour;
	int minute;
	int second;
	//uTimeStamp = GetTimeStamp();
	ToGetDateHourMinuteSec(uTimeStamp,day,hour,minute,second);
	//if (HISRECORDTRANSFERWAY == 1)
	if (g_dkTransWay == 1)
	{
		int total = hour*60 + minute;
		int total1 = 7*60 + 30;
		int total2 = 9*60 + 30;
		int total3 = 16*60 + 30;
		int total4 = 18*60 + 30;
		if ((total > total1 && total < total2) || (total > total3 && total < total4) )
		{
			////LogError("每小时补传方式，此时不在补传时间段\n");
			return false;
		}
		else
		{
			//LogNormal("IsTransferMoment 1\n");
			return true;
		}
	}
	//else if (HISRECORDTRANSFERWAY == 2)
	else if (g_dkTransWay == 2)
	{
		//LogNormal("IsTransferMoment 2\n");
		return true;
/*		if (hour >= 1 && hour <= 5)
		{
			return true;
		}
		else
		{
			//LogError("隔天补传方式，此时不在补传时间段\n");
			return false;
		}*/
	}
	return false;
	
}

/*
* 函数介绍：压入一条消息
* 输入参数：strMsg-要压入的消息
* 输出参数：无
* 返回值 ：无
*/
void mvCCenterServerOneDotEight::mvPushOneMsg(string strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvPushOneMsg()\n");
//#endif
    if (strMsg.empty())
    {
        return ;
    }

    //some special messages will be processed here.
    string strCode(""), strDetectorId("");
    strCode.append(strMsg.c_str() + 1, 3);
    strDetectorId.append(strMsg.c_str() + 4, 15);

    if (strDetectorId != m_strDetectorId) //compare them including ' ' as valid character.
    {

//#ifdef CS_FILE_LOG
//        fprintf(m_pCSRecvLog, "Error!#####strDetectorId=%s, m_strDetectorId=%s\n",
//                strDetectorId.c_str(), m_strDetectorId.c_str());
//        fflush(m_pCSRecvLog);
//#endif
        printf("Error!#####strDetectorId=%s, m_strDetectorId=%s\n", strDetectorId.c_str(), m_strDetectorId.c_str());
        return ;
    }
	//LogNormal("strCode=%s\n",strCode.c_str());
    if (LINK_TEST_REP == strCode) //心跳回复
    {
        m_nCSLinkCount = 0;
        return ;
    }
    else if (DEVICE_ALARM_REP == strCode)
    {
        //do nothing, return directly.
        return ;
    }

    strMsg.erase(0, 19); //26); //erase header(1)+code(3)+detectorId(15) //+length(7).

    pthread_mutex_lock(&m_mutexMsg); //push msg into queue.

    m_mapCSMsg.insert(make_pair(strCode, strMsg));

    pthread_mutex_unlock(&m_mutexMsg);
}

/*
* 函数介绍：弹出一条消息
* 输入参数：strCode-要弹出的消息类型变量；strMsg-要弹出的消息变量
* 输出参数：strCode-弹出的消息类型；strMsg-弹出的消息
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvPopOneMsg(string &strCode, string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvPopOneMsg()\n");
//#endif
    if (!strMsg.empty())
    {
        strMsg.clear();
    }

    pthread_mutex_lock(&m_mutexMsg);

    CS_MSG_QUEUE::iterator iter = m_mapCSMsg.begin();
    if (iter != m_mapCSMsg.end())
    {
        strCode = iter->first;
        strMsg = iter->second;
        m_mapCSMsg.erase(iter);
    }

    pthread_mutex_unlock(&m_mutexMsg);

    return (!strMsg.empty());
}

/*
* 函数介绍：设置检测器ID
* 输入参数：pDetectorId-新的检测器ID字符串
* 输出参数：无
* 返回值 ：无
*/
void mvCCenterServerOneDotEight::mvSetDetectorId(const char *pDetectorId)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSetDetectorId()\n");
//#endif
    char szDetecotrId[16] = {0};

    sprintf(szDetecotrId, "%15s", pDetectorId);
    m_strDetectorId = szDetecotrId; // fill it's left with ' ' in case it's length less than 15.
}

/*
* 函数介绍：处理一条消息
* 输入参数：pCode-要处理的消息类型；strMsg-要处理的消息
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnDealOneMsg(const char *pCode, const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnDealOneMsg()\n");
//#endif
	//LogNormal("mvOnDealOneMsg[%s]\n",pCode);
    //deal one message.
    if (strcmp(pCode, DETECT_RESTART) == 0) //识别程序重启
    {
        return mvOnDetectRestart(); //(strMsg);
    }
    else if (strcmp(pCode, MARK_UNSEND) == 0) //重新标识为未上传记录，强制重传
    {
        return mvOnMarkUnsend(strMsg);
    }
    else if (strcmp(pCode, RECORD_QUERY) == 0) //未上传数据查询
    {
        return mvOnRecordQuery(strMsg);
    }
    else if (strcmp(pCode, DEVICE_STATUS) == 0) //设备工作状态查询
    {
        return mvOnDeviceStatus(strMsg);
    }
    else if (strcmp(pCode, END_WORK_REP) == 0) //结束工作回复
    {
        return mvOnEndWorkRep(); //(strMsg);
    }
    else if (strcmp(pCode, REALTIME_RECORD_REP) == 0) //实时记录上传回复
    {
        return mvOnRealTimeRecordRep(); //(strMsg);
    }
    else if (strcmp(pCode, HISTORY_RECORD_REP) == 0) //上传历史记录回复
    {
        return mvOnHistoryRecordRep(); //(strMsg);
    }
    else if (strcmp(pCode, SYSTIME_SETUP) == 0) //时钟设置
    {
        return mvOnSysTimeSetup(strMsg); //(strMsg);
    }
	else if (strcmp(pCode, HMD_QUERY_REP) == 0)
	{
		return mvOnFtpDownloadRep(strMsg);
	}
    else
    {
        return false;
    }
}

/*
* 函数介绍：重组消息并发送
* 输入参数：pCode-要发送的消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvRebMsgAndSend(int& nSocket,const char *pCode, const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvRebMsgAndSend()\n");
//#endif
    //rebuild the message according to the msg-code and then send it to the center server.
    string strFullMsg(""), strCrc("");
    strFullMsg += m_strHeader + string(pCode) + m_strDetectorId;

//    int nMsgLength = strMsg.size() + 8;
//    char szLength[8] = {0};
//    sprintf(szLength, "%07d", nMsgLength);
//    strFullMsg += szLength;

    if (!strMsg.empty())
    {
        strFullMsg += strMsg;
    }

    mvCRCEncode(strFullMsg, strCrc);
    strFullMsg += strCrc + m_strEnd;

    if (!mvSendMsgToSocket(nSocket, strFullMsg,true))
    {
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSSendLog, "\nerror发送消息失败，套接字=%d，消息大小=%d(Bytes)，CRC32校验值=%s，消息内容为：\n%s\n\n",
//                nSocket, (int)strFullMsg.size(), strFullMsg.c_str()+strFullMsg.size()-9, strFullMsg.c_str());
//        fflush(m_pCSSendLog);
//#endif
		LogError("发送消息失败，套接字=%d，code=%s,消息大小=%d,SendFail=%d\n",nSocket, pCode,(int)strFullMsg.size(),m_nSendFailCount);
		//LogError("消息内容为:%s",strFullMsg.c_str());
		//LogNormal("Send error,close Connect\n");
        //mvCloseSocket(nSocket);
        //g_MybCenterLink = false;
        //LogError("发送消息失败，连接断开\n");
		m_nSendFailCount++;
		if((m_nSendFailCount >= 5) && (strFullMsg.size() > 1000))//只针对图片数据
		{
			if(nSocket > 500)
			{
				g_bEndThread = true;
				sleep(3);
				LogNormal("套接字资源耗尽，软件自动复位\n");
				exit(-1);
			}
			mvCloseSocket(nSocket);
			g_MybCenterLink = false;
			LogError("连续发送失败大于5次，不再发送该记录,断开连接\n");
			m_nSendFailCount = 0;
			return true;
		}
        return false;
    }

////#ifdef CS_FILE_LOG
////    fprintf(m_pCSSendLog, "\nsuccess发送消息成功，套接字=%d，消息大小=%d(Bytes)，CRC32校验值=%s，消息内容为：\n%s\n\n",
////            nSocket, (int)strFullMsg.size(), strFullMsg.c_str()+strFullMsg.size()-9, strFullMsg.c_str());
////    fflush(m_pCSSendLog);
////#endif
	m_nSendFailCount = 0;
    return true;
}

/*
* 函数介绍：发送路由请求并连接到中心
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvRequestAndConnCS()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvRequestAndConnCS()\n");
//#endif
    //send connect request;
    //recv connect reply;
    if (!mvSendRouterReqAndRecv())
    {
		LogNormal("mvSendRouterReqAndRecv error\n");
        return false;
    }

    //connect to center server and start to receive cs's msgs;
    if (!mvConnCSAndRecvMsg())
    {
		LogNormal("mvConnCSAndRecvMsg error\n");
        return false;
    }

    return true;
}

/*
* 函数介绍：连接到中心并开启接收消息线程
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvConnCSAndRecvMsg()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvConnCSAndRecvMsg()\n");
//#endif
    //connect to center server;
    if (!mvConnectToCS())
    {
		LogError("v1.8 Connect Center Server error\n");
        return false;
    }

	//LogNormal("v1.8 Connect Server success\n");

    //start to receive cs's msgs;
    if (!mvStartRecvThread())
    {
		LogError("mvStartRecvThread error!");
        return false;
    }

    return true;
}

/*
* 函数介绍：连接到中心
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvConnectToCS()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvConnectToCS()\n");
//#endif
    //connect to center server and set socket's option.
    if (g_strCenterServerHost.empty() || g_strCenterServerHost == "0.0.0.0" || g_nCenterServerPort <= 0)
    {
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSConnLog, "\n中心数据服务器连接参数异常:host=%s,port=%d\n",
//                g_strCenterServerHost.c_str(), g_nCenterServerPort);
//        fflush(m_pCSConnLog);
//#endif
        //printf("\n中心数据服务器连接参数异常:host=%s,port=%d\n", g_strCenterServerHost.c_str(), g_nCenterServerPort);
		LogError("中心数据服务器连接参数异常:host=%s,port=%d\n",g_strCenterServerHost.c_str(), g_nCenterServerPort);
        return false;
    }

    if (!mvPrepareSocket(m_nCenterSocket))
    {
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSConnLog, "\n准备连接中心数据服务器套接字失败!\n");
//        fflush(m_pCSConnLog);
//#endif
        //printf("\n准备连接中心数据服务器套接字失败!\n");
		LogError("准备连接中心数据服务器套接字失败!\n");
        return false;
    }

	//设置接收超时
	if(!mvSetSocketOpt(m_nCenterSocket, SO_RCVTIMEO))
	{
		LogError("SO_RCVTIMEO error\n");
		return false;
	}
	

    if (!mvWaitConnect(m_nCenterSocket, g_strCenterServerHost, g_nCenterServerPort,5))
    {
////#ifdef CS_FILE_LOG
////        fprintf(m_pCSConnLog, "\n尝试连接中心数据服务器失败，host=%s,port=%d!\n",
////                g_strCenterServerHost.c_str(), g_nCenterServerPort);
////        fflush(m_pCSConnLog);
////#endif
        //printf("\n尝试连接中心数据服务器失败!\n");
		LogError("尝试连接中心数据服务器失败，host=%s,port=%d!\n",g_strCenterServerHost.c_str(), g_nCenterServerPort);
        return false;
    }	

    return true;
}

/*
* 函数介绍：接收消息线程入口
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvStartRecvThread()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvStartRecvThread()\n");
//#endif
	m_bEndRecvThread = true;
	LogNormal("before pthread_join\n");
	
	if(m_nRecvThreadId != 0)
	{
		pthread_join(m_nRecvThreadId,NULL);
		m_nRecvThreadId = 0;
	}
	LogNormal("after pthread_join\n");
	m_bEndRecvThread = false;
    pthread_attr_t   attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (pthread_create(&m_nRecvThreadId, &attr, MyRecvCSMsgThread, NULL) != 0)
    {
        pthread_attr_destroy(&attr);
        return false;
    }

    pthread_attr_destroy(&attr);
    return true;
}

/*
* 函数介绍：检查是否空闲，是则处理历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCCenterServerOneDotEight::mvCheckFreeAndDealHistoryRecord()
{
	 if(g_nSendHistoryRecord == 1)
	 {
	//#ifdef CS_STUPID_LOG
	//    CSSLog("In mvCCenterServerOneDotEight::mvCheckFreeAndDealHistoryRecord()\n");
	//#endif
		//LogNormal("in mvCheckFreeAndDealHistoryRecord()\n");
		unsigned int uTimeStamp = GetTimeStamp();
		if (IsTransferMoment(uTimeStamp) && m_uForceTransferType == ' ')
		{
			if (!SetBeginAndEndTime(uTimeStamp))
			{
				return;
			}
			//LogNormal("is Transfer Moment\n");
		}
		else
		{
			if (m_uForceTransferType == '1' || m_uForceTransferType == '2' || m_uForceTransferType == '3' || \
				m_uForceTransferType == '4' || m_uForceTransferType == '5' || m_uForceTransferType == '6')
			{
			}
			else
			{
				return;
			}
		}
		//LogNormal("berore mvGetOneHistoryRecord\n");
		std::list<unsigned int> listSeq;
		std::list<string> strListRecord;
		if(mvGetOneHistoryRecord(strListRecord))
		{
			//LogNormal("strListRecord.size=%d\n",strListRecord.size());
			StrList::iterator it_b = strListRecord.begin();
			while(it_b != strListRecord.end())
			{
				uTimeStamp = GetTimeStamp();
				//LogNormal("mvCheckFreeAndDealHistoryRecord \n");
				//printf("before (uTimeStamp=%d, m_uHistoryRepTime=%d)\n",uTimeStamp,m_uHistoryRepTime);
				if (m_bHistoryRep || (uTimeStamp >= m_uHistoryRepTime+30))
				{			
							//printf("after (uTimeStamp >= m_uHistoryRepTime+30)\n");
							string strMsg = *it_b;
							
							UINT32 uSeq =*((unsigned int*)(strMsg.c_str()+sizeof(MIMAX_HEADER)));
							RECORD_PLATE *sPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
							int uViolationType = sPlate->uViolationType;

							bool bSendStatus = false;
							
							if(g_nSendViolationOnly == 1)
							{
								if(uViolationType > 0)
								{
									bSendStatus = mvSendRecordToCS(strMsg, false);
								}
								else
								{
									bSendStatus = true;
									listSeq.push_back(uSeq);
									it_b++;
									continue;
								}
							}
							else//卡口违章都需要发送
							{
								//printf("before mvSendRecordToCS\n");
								bSendStatus = mvSendRecordToCS(strMsg, false);
							}

							//LogNormal("mvGetOneHistoryRecord ok\n");
							if (bSendStatus)
							{
								//LogNormal("mvSendRecordToCS OK \n");
								
								listSeq.push_back(uSeq);
								sleep(5);

								if (m_uForceTransferType == '1' || m_uForceTransferType == '2' || m_uForceTransferType == '3' || m_uForceTransferType == '4'
									|| m_uForceTransferType == '5' || m_uForceTransferType == '6')
								{
									if (m_uForceTransCount > 0)
									{
										m_uForceTransCount -= 1;
									}
									if (m_uForceTransCount == 0)
									{
										m_uForceTransferType = ' ';
									}
								}
							}
				}
				it_b++;
			}
			if(listSeq.size() > 0)
			g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, listSeq);
		}
		else
		{
			//LogNormal("strListRecord.size=0\n");
			sleep(60);
		}
	 }
}

//设置补传起止时刻
bool mvCCenterServerOneDotEight::SetBeginAndEndTime(unsigned int uTimeStamp)
{
	//LogNormal("SetBeginAndEndTime\n");
	int day;
	int minute;
	int hour;
	int second;
	ToGetDateHourMinuteSec(uTimeStamp,day,hour,minute,second);
	if (g_dkTransWay == 1)
	{
		//if (minute == TRANSFREMOMENT)
		if (minute == g_dkMinute)
		{
			m_uBeginTime.clear();
			m_uEndTime.clear();
			//String strTime = GetTime(uTimeStamp);
			unsigned int uEndTimeStamp = uTimeStamp - minute*60 - second;
			unsigned int uBeginTimeStamp = uEndTimeStamp - 3600; 
			m_uBeginTime = GetTime(uBeginTimeStamp);
			m_uEndTime = GetTime(uEndTimeStamp);
			return true;

		}
		if (m_uBeginTime.size() == 0 || m_uEndTime.size() == 0)
		{
			LogError("隔小时补传，设置补传起止时刻失败\n");
			return false;
		}
		else
		{
			return true;
		}
	}
	else if (g_dkTransWay == 2)
	{
		unsigned int uBeginTimeStamp = uTimeStamp - 10*24*3600;
		unsigned int uEndTimeStamp = uTimeStamp; 
		m_uBeginTime = GetTime(uBeginTimeStamp);
		m_uEndTime = GetTime(uEndTimeStamp);
		return true;
	}
	else
	{
		LogError("其它错误：不是协议中的两种补传方式\n");
		return false;
	}
	
	
}

/*
* 函数介绍：发送路由请求并接收应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendRouterReqAndRecv()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendRouterReqAndRecv()\n");
//#endif
    //make up one msg and send to router.
    if (!mvSendConnReqToRouter())
    {
		LogNormal("mvSendConnReqToRouter error\n");
        return false;
    }

    //wait to receive router's reply and set center server's IP and port.
    if (!mvRecvIpPortFromRouter())
    {
		LogNormal("mvRecvIpPortFromRouter error\n");
        return false;
    }

    //close connection with router.
    mvCloseSocket(m_nControlSocket);
    return true;
}

/*
* 函数介绍：发送路由请求
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendConnReqToRouter()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendConnReqToRouter()\n");
//#endif
    char szBuff[20] = {0};

	sprintf(szBuff, "%15s%4s", g_ServerHost.c_str(), g_MystrCSMsgVer.c_str());

	string strHost("");
	if(g_n3GTYPE == 1)
	{
		 strHost = g_str3GIp;
	}
	else
	{
		strHost = GetIpAddress("ppp0",1);
	}
	
	
	if (strHost.size() > 0)
	{
		sprintf(szBuff, "%15s%4s", strHost.c_str(), g_MystrCSMsgVer.c_str());
	}

	LogNormal("ReqToRouter=%s\n",szBuff);


    return mvRebMsgAndSend(m_nControlSocket,START_WORK, string(szBuff));
}


/*
* 函数介绍：接收路由信息
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvRecvIpPortFromRouter()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvRecvIpPortFromRouter()\n");
//#endif
    string strMsg("");
    //int nCount = 0;

    //mvSetSocketBlock(m_nCenterSocket, false);
    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec=5;
    timeo.tv_usec=0;//超时5s

    if(setsockopt(m_nControlSocket, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {
        perror("setsockopt");
        return false;
    }

    //while (nCount++ < 3)
    {
     //   sleep(1);
        if (!mvRecvSocketMsg(m_nControlSocket, strMsg))
        {
    //        continue;
              return false;
        }
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSRecvLog, "\n接收消息成功: \n%s\n", strMsg.c_str());
//        fflush(m_pCSRecvLog);
//#endif

        string strCode(""), strDetectorId(""), strRes(""), strIp(""),
                strPort(""), strDate(""), strTime(""), strCrc("");

        //?601000000000000LP11  192.168.60.77088882010-03-0416:00:53.053bed1326f?
        strCode.append(strMsg.c_str() + 1, 3);
        strDetectorId.append(strMsg.c_str() + 4, 15);
        strRes.append(strMsg.c_str() + 19, 1);
        strIp.append(strMsg.c_str() + 20, 15);
        strPort.append(strMsg.c_str() + 35, 5);
        strDate.append(strMsg.c_str() + 40, 10);
        strTime.append(strMsg.c_str() + 50, 12);
        strCrc.append(strMsg.c_str() + 62, 8);

        char szIp[16] = {0};
        int nPort;
        sscanf(strIp.c_str(), "%s", szIp);
        sscanf(strPort.c_str(), "%d", &nPort);
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSRecvLog, "After parse, strDetectorId=%s, m_strDetectorId=%s\n",
//                strDetectorId.c_str(), m_strDetectorId.c_str());
//        fflush(m_pCSRecvLog);
//#endif
        printf("After parse, strDetectorId=%s, m_strDetectorId=%s\n",
               strDetectorId.c_str(), m_strDetectorId.c_str());

        if (START_WORK_REP == strCode && strDetectorId == m_strDetectorId)
        {
            if ("1" == strRes)
            {
                //write the IP and port back into file.
                string strServerHost(szIp);

                //ip合理性判断
                if( (strServerHost.size() > 8 && strServerHost.size() < 16) && (g_strCenterServerHost != strServerHost))
                {
                    g_strCenterServerHost = strServerHost;
                    CXmlParaUtil xml;
                    xml.UpdateSystemSetting("OtherSetting", "CenterServerHost");
                }

                if(g_nCenterServerPort != nPort)
                {
                    g_nCenterServerPort = nPort;
                    CXmlParaUtil xml;
                    xml.UpdateSystemSetting("OtherSetting", "CenterServerPort");
                }
                //mvSetSocketBlock(m_nCenterSocket, true);
                return true;
            }
            else
            {
                //mvSetSocketBlock(m_nCenterSocket, true);
                return false;
            }
        }
    }

    //mvSetSocketBlock(m_nCenterSocket, true);
    return false;
}

/*
* 函数介绍：结束工作
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnEndWork()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnEndWork()\n");
//#endif
    g_MybCenterLink = false;
    mvCloseSocket(m_nCenterSocket);

    if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
    {
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSConnLog, "\n结束工作时，中心路由服务器连接参数异常! host=%s\tport=%d\n",
//                g_strControlServerHost.c_str(), g_nControlServerPort);
//        fflush(m_pCSConnLog);
//#endif
        return false;
    }
    else
    {
        if (!mvPrepareSocket(m_nControlSocket))
        {
//#ifdef CS_FILE_LOG
//            fprintf(m_pCSConnLog, "\n结束工作时，准备连接套接字失败!\n");
//            fflush(m_pCSConnLog);
//#endif
            return false;
        }

        if (!mvWaitConnect(m_nControlSocket, g_strControlServerHost, g_nControlServerPort))
        {
//#ifdef CS_FILE_LOG
//            fprintf(m_pCSConnLog, "\n结束工作时，尝试连接中心路由服务器失败!\n");
//            fflush(m_pCSConnLog);
//#endif
            return false;
        }

        if (!mvSendEndWork())
        {
////#ifdef CS_FILE_LOG
////            fprintf(m_pCSConnLog, "\n发送结束工作消息失败!\n");
////            fflush(m_pCSConnLog);
////#endif
        }

        mvCloseSocket(m_nControlSocket);
    }

    return true;
}

/*
* 函数介绍：开始工作
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnDetectRestart() //(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnDetectRestart()\n");
//#endif
    mvSendRestartRep();
    mvCloseSocket(m_nCenterSocket);
    g_MybCenterLink = false;
    LogNormal("识别程序重启\n");
    //system("reboot");
    return true;
}

/*
* 函数介绍：获取一条历史记录
* 输入参数：strMsg-要获取的历史记录存储变量
* 输出参数：strMsg-获取的历史记录
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvGetOneHistoryRecord(std::list<string>& strListRecord)
{
		char buf[1024]={0};
		unsigned int uTimeStamp = GetTimeStamp()-600;//600秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
		String strTime = GetTime(uTimeStamp);

		{
			//sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME <= '%s' ORDER BY TIME desc limit 1",strTime.c_str());
			if (m_uForceTransferType == '1' || m_uForceTransferType == '2'|| m_uForceTransferType == '3'|| m_uForceTransferType == '4')//暂有此4种强制重传违法数据类型
			{
				if(g_nSendViolationOnly == 1)
				{
					sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and PECCANCY_KIND > 0 and TIME >= '%s' and TIME <= '%s' ORDER BY TIME desc limit 100",m_uForceTransBeginTime.c_str(),m_uForceTransEndTime.c_str());
				}
				else
				{
					sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME >= '%s' and TIME <= '%s' ORDER BY TIME desc limit 100",m_uForceTransBeginTime.c_str(),m_uForceTransEndTime.c_str());
				}
			}
			else if (m_uForceTransferType == '5' || m_uForceTransferType == '6')//暂有此2种强制重传卡口数据 类型
			{
				if(g_nSendViolationOnly == 1)
				{
					sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and PECCANCY_KIND = 0 and TIME >= '%s' and TIME <= '%s' ORDER BY TIME desc limit 100",m_uForceTransBeginTime.c_str(),m_uForceTransEndTime.c_str());
				}
				else
				{
					sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME >= '%s' and TIME <= '%s' ORDER BY TIME desc limit 100",m_uForceTransBeginTime.c_str(),m_uForceTransEndTime.c_str());
				}
			}
			else
			{
				if(g_nSendViolationOnly == 1)
				{
					sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and PECCANCY_KIND > 0 and TIME >= '%s' and TIME <= '%s' ORDER BY TIME desc limit 100", m_uBeginTime.c_str(),strTime.c_str());
				}
				else
				{
					sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME >= '%s' and TIME <= '%s' ORDER BY TIME desc limit 100", m_uBeginTime.c_str(),strTime.c_str());
				}
			}
			string strSql(buf);
			MysqlQuery q = g_skpDB.execQuery(strSql);
//LogNormal("mvGetOneHistoryRecord 2\n");
			while (!q.eof())
			{
				string strMsg;
				//LogNormal("mvGetOneHistoryRecord:获取一条历史记录\n");
				MIMAX_HEADER sHeader;

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

				//车身颜色，车辆类型，速度，方向,地点等
				plate.uCarColor1 = q.getIntFileds("CARCOLOR");
				plate.uType = q.getIntFileds("TYPE");
				plate.uSpeed = q.getIntFileds("SPEED");
				plate.uWeight1 = q.getIntFileds("CARCOLORWEIGHT");
				plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
				plate.uWeight2 = q.getIntFileds("CARCOLORWEIGHTSECOND");

				plate.uViolationType = q.getIntFileds("PECCANCY_KIND");
				if(plate.uViolationType == DETECT_RESULT_NOCARNUM)
				{
					plate.uViolationType = 0;
				}

				string strTime2 = q.getStringFileds("TIMESECOND");
				if(strTime2.size() > 8)
				plate.uTime2 = MakeTime(strTime2);
				else
				plate.uTime2 = 0; 
				plate.uMiTime2 = q.getIntFileds("MITIMESECOND");

		
				string StrVideoPath = q.getStringFileds("VIDEOPATH");
				memcpy(plate.chVideoPath,StrVideoPath.c_str(),StrVideoPath.size());


					if(plate.chText[0]=='*')//
					{
						memcpy(plate.chText,"00000000",8);
					}
					memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

					sHeader.uCmdID = MIMAX_PLATE_REP;
					strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
					strMsg.append((char*)&plate, sizeof(RECORD_PLATE));
				//strMsg = strMsg+GetImageByPath(strPicPath);

					q.nextRow();
					strListRecord.push_back(strMsg);
			}
			q.finalize();
		}

    if(strListRecord.size() <= 0)
	{
		return false;
	}
    return true;
}

/*
* 函数介绍：标记未发送，重传
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnMarkUnsend(const string &strMsg)
{
////#ifdef CS_STUPID_LOG
////    CSSLog("In mvCCenterServerOneDotEight::mvOnMarkUnsend()\n");
////#endif
    char szDate1[11] = {0};
    char szTime1[13] = {0};
    char szDate2[11] = {0};
    char szTime2[13] = {0};
	//char szDataType;

    sscanf(strMsg.c_str(), "%10s%12s%10s%12s%c", szDate1, szTime1, szDate2, szTime2,&m_uForceTransferType);
    string strTime1 = szDate1;
    strTime1 += " ";
    strTime1.append(szTime1, 8);
    string strTime2 = szDate2;
    strTime2 += " ";
    strTime2.append(szTime2, 8);

	m_uForceTransBeginTime = strTime1;
	m_uForceTransEndTime = strTime2;
	

    char szBuff[1024] = {0};
	char buf[128] = {0};
    UINT uCount = 0;
    string strResMsg(""), strTableName(""), strTimeName("");

    if (PERSON_ROAD == g_nRoadType)
    {
        strTableName = "TRAFFIC_EVENT_INFO";
        strTimeName = "BEGIN_TIME";

		if (m_uForceTransferType == '1')// 所有违法数据(含图片)
		{
			sprintf(buf,"%s","KIND > 0");
		}
		else if (m_uForceTransferType == '2')//所有违法(不含图片)
		{
			sprintf(buf,"%s","KIND > 0");
		}
		else if (m_uForceTransferType == '3')//所有该时段内未成功上传的违法数据(含图片) 
		{
			sprintf(buf,"%s","STATUS=0 and KIND > 0");
		}
		else if (m_uForceTransferType == '4')//所有该时段内未成功上传得违法数据(不含图片)
		{
			sprintf(buf,"%s","STATUS=0 and KIND > 0");
		}
		else if (m_uForceTransferType == '5')//所有卡口数据(不含图片)
		{
			sprintf(buf,"%s","KIND = 0");
		}
		else if (m_uForceTransferType == '6')//所有该时段内未成功上传的卡口数据(不含图片)
		{
			sprintf(buf,"%s","STATUS=0 and KIND = 0");
		}
		else
		{

			LogNormal("not ForceTransferType\n");
			return false;
		}

		sprintf(szBuff, "select count(*) from %s where %s and %s>='%s' and %s<='%s';",
			strTableName.c_str(), buf, strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
		MysqlQuery count = g_skpDB.execQuery(string(szBuff));
		if (!count.eof())
		{
			uCount = count.getUnIntFileds(0);
			m_uForceTransCount = uCount;
		}
		count.finalize();

		
		if (m_uForceTransferType == '1' || m_uForceTransferType == '2' || m_uForceTransferType == '5')
		{
			memset(szBuff, 0, 1024);
			sprintf(szBuff, "update %s set STATUS=0 where STATUS=1 and %s>='%s' and %s<='%s';",
				strTableName.c_str(), strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
			g_skpDB.execSQL(string(szBuff));
		}
		
    }
    else
    {
        strTableName = "NUMBER_PLATE_INFO";
        strTimeName = "TIME";


		if (m_uForceTransferType == '1')// 所有违法数据(含图片)
		{
			sprintf(buf,"%s","PECCANCY_KIND > 0 and PECCANCY_KIND != 38");
		}
		else if (m_uForceTransferType == '2')//所有违法(不含图片)
		{
			sprintf(buf,"%s","PECCANCY_KIND > 0 and PECCANCY_KIND != 38");
		}
		else if (m_uForceTransferType == '3')//所有该时段内未成功上传的违法数据(含图片) 
		{
			sprintf(buf,"%s","STATUS=0 and PECCANCY_KIND > 0 and PECCANCY_KIND != 38");
		}
		else if (m_uForceTransferType == '4')//所有该时段内未成功上传得违法数据(不含图片)
		{
			sprintf(buf,"%s","STATUS=0 and PECCANCY_KIND > 0 and PECCANCY_KIND != 38");
		}
		else if (m_uForceTransferType == '5')//所有卡口数据(不含图片)
		{
			sprintf(buf,"%s","PECCANCY_KIND in(0,38)");
		}
		else if (m_uForceTransferType == '6')//所有该时段内未成功上传的卡口数据(不含图片)
		{
			sprintf(buf,"%s","STATUS=0 and PECCANCY_KIND in(0,38)");
		}
		else
		{
			LogNormal("not ForceTransferType\n");
			return false;
		}

		sprintf(szBuff, "select count(*) from %s where %s and %s>='%s' and %s<='%s';",
			strTableName.c_str(), buf, strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
		MysqlQuery count = g_skpDB.execQuery(string(szBuff));
		if (!count.eof())
		{
			uCount = count.getUnIntFileds(0);
			m_uForceTransCount = uCount;
		}
		count.finalize();

		if (m_uForceTransferType == '1' || m_uForceTransferType == '2' || m_uForceTransferType == '5')
		{
			memset(szBuff, 0, 1024);
			sprintf(szBuff, "update %s set STATUS=0 where STATUS=1 and %s>='%s' and %s<='%s';",
				strTableName.c_str(), strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
			g_skpDB.execSQL(string(szBuff));
		}
		
    }

  

    strResMsg.append(strMsg.c_str(), 44);
    memset(szBuff, 0, 1024);
    sprintf(szBuff, "%07u", uCount);
    strResMsg += szBuff;

    if(mvSendMarkUnsendRep(strResMsg))
    {
        LogNormal("标记未发送，重传%s\r\n",strResMsg.c_str());
        return true;
    }
    else
    {
        return false;
    }
}

//将对方传过来的代码转换为我方代码
string mvCCenterServerOneDotEight::mvTransCode(UINT nCode)
{
	string strCode("");
	if (nCode == 0)
	{
		strCode = "PECCANCY_KIND = 0";
	}
	else if (nCode == 1)
	{
		strCode = "PECCANCY_KIND in(5,6,54)";
	}
	else if (nCode == 42)
	{
		strCode = "PECCANCY_KIND = 26";
	}
	else if (nCode == 23)
	{
		strCode = "PECCANCY_KIND in(32,55)";
	}
	else if (nCode == 11)
	{
		strCode = "PECCANCY_KIND in(18,19,20,23,24,25,56,57,59)";
	}
	else if (nCode == 4)
	{
		strCode = "PECCANCY_KIND = 16";
	}
	else if (nCode == 99)
	{
		strCode = "PECCANCY_KIND = 17";
	}
	else
	{
		LogError("数据库中暂没有此类型Code：%d\n",nCode);
	}

	return strCode;

}
/*
* 函数介绍：记录查询
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnRecordQuery(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnRecordQuery()\n");
//#endif
    char szDate1[11] = {0};
    char szTime1[13] = {0};
    char szDate2[11] = {0};
    char szTime2[13] = {0};
	int violationCode = 0;

    sscanf(strMsg.c_str(), "%10s%12s%10s%12s%05d", szDate1, szTime1, szDate2, szTime2,&violationCode);
    string strTime1 = szDate1;
    strTime1 += " ";
    strTime1.append(szTime1, 8);
    string strTime2 = szDate2;
    strTime2 += " ";
    strTime2.append(szTime2, 8);

    char szBuff[1024] = {0};
    UINT uTotalCount = 0;
    UINT uCount = 0;
    string strResMsg(""), strTableName(""), strTimeName("");

    if (PERSON_ROAD == g_nRoadType)
    {
        strTableName = "TRAFFIC_EVENT_INFO";
        strTimeName = "BEGIN_TIME";

		sprintf(szBuff, "select count(*) from %s where STATUS!=2 and %s>='%s' and %s<='%s';",
			strTableName.c_str(),strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
		MysqlQuery count1 = g_skpDB.execQuery(string(szBuff));
		if (!count1.eof())
		{
			uTotalCount = count1.getUnIntFileds(0);
		}
		count1.finalize();

		memset(szBuff, 0, 1024);
		sprintf(szBuff, "select count(*) from %s where STATUS=0 and KIND = %d and %s>='%s' and %s<='%s';",
			strTableName.c_str(),violationCode,strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
		MysqlQuery count2 = g_skpDB.execQuery(string(szBuff));
		if (!count2.eof())
		{
			uCount = count2.getUnIntFileds(0);
		}
		count2.finalize();
    }
    else
    {
        strTableName = "NUMBER_PLATE_INFO";
        strTimeName = "TIME";

		sprintf(szBuff, "select count(*) from %s where STATUS!=2 and %s>='%s' and %s<='%s';",
			strTableName.c_str(),strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
		MysqlQuery count1 = g_skpDB.execQuery(string(szBuff));
		if (!count1.eof())
		{
			uTotalCount = count1.getUnIntFileds(0);
		}
		count1.finalize();

		memset(szBuff, 0, 1024);
		string strTmp("");
		strTmp = mvTransCode(violationCode);
		if (strTmp.size() == 0)
		{
			uCount = 0;
		}
		else
		{
			sprintf(szBuff, "select count(*) from %s where STATUS=0 and %s and %s>='%s' and %s<='%s';",
				strTableName.c_str(),strTmp.c_str(),strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
			MysqlQuery count2 = g_skpDB.execQuery(string(szBuff));
			if (!count2.eof())
			{
				uCount = count2.getUnIntFileds(0);
			}
			count2.finalize();
		}
    }


    strResMsg.append(strMsg.c_str(), 44);
    memset(szBuff, 0, 1024);
    sprintf(szBuff, "%07u%07u", uCount, uTotalCount);
    strResMsg += szBuff;

    if(mvSendRecordQueryRep(strResMsg))
    {
        LogNormal("查询记录成功%s\r\n",strResMsg.c_str());
        return true;
    }
    else
    {
        return false;
    }
}

/*
* 函数介绍：检查设备状态
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnDeviceStatus(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnDeviceStatus()\n");
//#endif
    char szBuff[1024] = {0};

    int nRestDisk = m_nAllDisk * ((100 - g_sysInfo.fDisk)/100.f);

    char chCameraState = 'y';
    if(g_skpDB.GetCameraState(g_nCameraID)==0)
    {
        chCameraState = 'y';
    }
    else
    {
        chCameraState = 'n';
    }

    //////////////////////机箱温度暂时系统温度
    //srand( (unsigned)time( NULL ) );
    //int nTemperature = 30 + (int)(35.0*rand()/(RAND_MAX+1.0));
    int nTemperature = (int)g_sysInfo.fSysT;
	 //获取环境温度
	if(g_nHasExpoMonitor)
	{
		//if(g_ExpoMonitorInfo.nTemperatureAlarm > 0)
		{
			if(g_ExpoMonitorInfo.nTemperatureValue > -100)
			nTemperature = g_ExpoMonitorInfo.nTemperatureValue;
		}
	}
    //////////////////////////
	
	//开关门状态
	char chGateStatus = 'n';
	if(g_nHasExpoMonitor)
    {
		//开关门
        //if(g_ExpoMonitorInfo.nGateStateAlarm > 0)
        {
			if(g_ExpoMonitorInfo.nGateValue == 1)//01：机箱开门
            {	
				chGateStatus = 'y';
			}
			else if(g_ExpoMonitorInfo.nGateValue == 0)//02：机箱关门
            {
				chGateStatus = 'n';
			}
		}
	}
	
	//线圈状态
	/*char chLoopStatus = 'y';
	if(m_vLoopStatus.size() > 0)
	{
		mapLoopStatus::iterator it_b = m_vLoopStatus.begin();
		mapLoopStatus::iterator it_e = m_vLoopStatus.end();
		while(it_b != it_e)
		{
			if(it_b->second < 1)
			{
				chLoopStatus = 'n';
			}
			it_b++;
		}
	}*/
	//车道总数及车道状态
	string strLoopStatus = "";
	strLoopStatus = GetRoadStatus();

	//相机状态
	string strCameraStatus = "";
	strCameraStatus = GetCameraStatus();
	/////////////test
	//m_vLoopStatus.insert(mapLoopStatus::value_type(1,1));
	//m_vLoopStatus.insert(mapLoopStatus::value_type(2,0));
	////////////test
	//信号灯状态暂时不能获取到状态
	char chSignalStatus = 'u';

    //printf("m_nAllDisk=%07d, nRestDisk=%07d, g_sysInfo.fCpu=%03d\n", m_nAllDisk, nRestDisk, (int)g_sysInfo.fCpu);
	sprintf(szBuff, "%15s%02d%s%02d%s%03d%c%07d%07d%03d%c",
		g_ServerHost.c_str(), m_uRoadCount,strLoopStatus.c_str(), m_uCameraCount, strCameraStatus.c_str(), nTemperature, chGateStatus, m_nAllDisk, nRestDisk, (int)g_sysInfo.fCpu,chSignalStatus);
	
	string strHost("");
	if(g_n3GTYPE == 1)
	{
		strHost = g_str3GIp;
	}
	else
	{
		strHost = GetIpAddress("ppp0",1);
	}
	
	if (strHost.size() > 0)
	{
		sprintf(szBuff, "%15s%02d%s%02d%s%03d%c%07d%07d%03d%c",
			strHost.c_str(), m_uRoadCount,strLoopStatus.c_str(), m_uCameraCount, strCameraStatus.c_str(), nTemperature, chGateStatus, m_nAllDisk, nRestDisk, (int)g_sysInfo.fCpu,chSignalStatus);
	}

    if( mvSendDeviceStatusRep(string(szBuff)))
    {
        LogNormal("查询设备状态成功%s\r\n",szBuff);
        return true;
    }
    else
    {
        return false;
    }
}

/*
* 函数介绍：设置系统时间
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnSysTimeSetup(const string &strMsg)
{
    char szDate[11] = {0};
    char szTime[13] = {0};

    sscanf(strMsg.c_str(), "%10s%12s", szDate, szTime);
    string strTime = szDate;
    strTime += " ";
    strTime.append(szTime, 12);
    printf("strTime=%s,strTime.size()=%d\n",strTime.c_str(),(int)strTime.size());
    LogNormal("mvOnSysTimeSetup=%s\r\n",strTime.c_str());

    //获取校时前时间
    struct timeval tv;
    gettimeofday(&tv,NULL);
    struct tm *oldTime,timenow;
	oldTime = &timenow;
	localtime_r( &tv.tv_sec,oldTime);
    char szBuff[1024] = {0};
    sprintf(szBuff, "%4d-%02d-%02d%02d:%02d:%02d.%03d",oldTime->tm_year+1900, oldTime->tm_mon+1, oldTime->tm_mday, oldTime->tm_hour, oldTime->tm_min, oldTime->tm_sec,(int)(tv.tv_usec/1000.0));
    //printf("szBuff=%s\n",szBuff);
    bool bRet = mvRebMsgAndSend(m_nCenterSocket,SYSTIME_SETUP_REP, string(szBuff));

    //设置时间，精确到豪秒
    if(strTime.size() > 0)
    {
		/*UINT32 timeSec = MakeTime(strTime);
		string timeUsec = strTime.substr(20, 3);
		timeval timer;
		timer.tv_sec = timeSec;
		timer.tv_usec = atoi(timeUsec.c_str()) * 1000;

		if (settimeofday(&timer, NULL) == 0)
		{
			printf("================mvOnSysTimeSetup=%s\n",strTime.c_str());
			system("hwclock --systohc");
		}*/
    }

    if(bRet)
    {
        LogNormal("设置时间成功%s\r\n",szBuff);
        return true;
    }
    else
    {
        return false;
    }
}

bool mvCCenterServerOneDotEight::mvOnFtpDownloadRep(const string &strMsg)
{
	string strFlag("");
	int nFlag = -1;
	bool bRet = false;

	if (strMsg.size() > 69)
	{
		strFlag.append(strMsg.c_str() + 69, 1);
		sscanf(strFlag.c_str(), "%d", &nFlag);
	}
	LogNormal("本次下载是否成功：%d\n",nFlag);
	if (nFlag != 0)
	{
		m_bFtpUpdate = false;
		m_bUpdateOnce = false;
		//bRet = DownloadFile();
		LogNormal("本次下载失败\n");
	}
	else
	{
		m_bFtpUpdate = true;
		m_bUpdateOnce = true;
		LogNormal("本次下载成功\n");
	}
	/*FILE* fp = fopen("md5.txt","a+");
	fprintf(fp,"[%s][mvCCenterServerOneDotEight::mvOnFtpDownloadRep]Get Message From Center:Success:%d SIZE:%d\nMSG:%s\n",GetTime(GetTimeStamp(),0).c_str(),nFlag,strMsg.size(),strMsg.c_str());
	fclose(fp);*/
	return true;
}

bool mvCCenterServerOneDotEight::DownloadFile()
{
	bool bRet = false;

	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r( &now,newTime );

	if (m_bFtpUpdate == true && newTime->tm_hour != m_nFtpUpdateHour)
	{
		m_bUpdateOnce = false;
		return true;
	}

	//获取黑名单表
	if(g_nLoadBasePlateInfo == 1)
	{
		if (!(g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0))
		{
			if(newTime->tm_min >=12 && newTime->tm_min <=24)//更新时间反馈为每小时的12分-24分
			{
				if (m_bUpdateOnce == false)
				{
					m_bFtpUpdate = false;
					if(g_MybCenterLink)
					{
						m_nFtpUpdateHour = newTime->tm_hour;
						char strFileName[1024] = {0};
						sprintf(strFileName,"hmd_bocom.csv");

						char strRemoteFileName[1024] = {0};

						string strFileList("");
						bool bLoad = g_FtpCommunication.DoList("/",strFileList);
						if (bLoad)
						{
							m_strRemoteFile = GetLastFileName(strFileList);
							sprintf(strRemoteFileName,"%s",m_strRemoteFile.c_str());
							LogNormal("黄标车文件名称: %s\n",m_strRemoteFile.c_str());

							/*FILE* fp = fopen("md5.txt","a+");
							fprintf(fp,"[%s][mvCCenterServerOneDotEight::DownloadFile]:\n%s\n",GetTime(GetTimeStamp(),0).c_str(),strFileList.c_str());
							fprintf(fp,"[%s][mvCCenterServerOneDotEight::DownloadFile]:strRemoteFileName:%s\n",GetTime(GetTimeStamp(),0).c_str(),strRemoteFileName);
							fclose(fp);*/
						}

						bool bLoadOK = g_FtpCommunication.DoGet("./config/hmd_bocom.csv", strRemoteFileName);//ftp上的文件路径和名称需要确认
						if(bLoadOK)//完成黑名单下载后给中心发送报告信息
						{
							bRet = mvSendHMDToCS(strFileName);
							//test 2 line
							//bRet = true;
							//m_bUpdateOnce = true;
							LogNormal("即将更新FTP:Time: %d-%d\n",m_nFtpUpdateHour,newTime->tm_min);
							/*FILE* fp = fopen("md5.txt","a+");
							fprintf(fp,"[%s][mvCCenterServerOneDotEight::DownloadFile]:DownloadTime: %d:%d\n",GetTime(GetTimeStamp(),0).c_str(),m_nFtpUpdateHour,newTime->tm_min);
							fclose(fp);*/
						}
						else
						{
							LogNormal("下载黄标车文件失败!\n");
						}
					}
				}
			}
		}
	}

	return bRet;
}

std::string mvCCenterServerOneDotEight::GetLastFileName(std::string strList)
{
	int nPos1 = 0;
	int nPos2 = -1;
	std::string strLast = "";

	if (strList.size() > 0)
	{
		std::string strSubList;
		std::string strLeftList;
		while(nPos1 >= 0)
		{
			strSubList = "";
			strLeftList = "";
			nPos1 = -1;
			nPos2 = -1;
			nPos1 = strList.find("hmd_");
			printf("GetLastFileName nPos1=%d\n",nPos1);
			if (nPos1 >= 0)
			{
				nPos2 = strList.find("\r");
				if (nPos2 > 0 && (nPos2>nPos1))
				{
					printf("GetLastFileName nPos2=%d\n",nPos2);
					strSubList.append(strList.c_str()+nPos1,nPos2-nPos1);//去除seariapp/hmd/hmd_20140701002400.csv

					printf("GetLastFileName strSubList=%s,strSubList.size()=%d\n",strSubList.c_str(),strSubList.size());

					if (strSubList.size() == 22)
					{
						int nPos3 = strSubList.find(".csv");
						if (nPos3 > 0)
						{		
							printf("GetLastFileName nPos3=%d\n",nPos3);
							if (strLast.size() > 0)
							{
								if (strLast.compare(strSubList) < 0)
								{
									strLast = strSubList;
								}
							}
							else
							{
								strLast = strSubList;
							}
							printf("GetLastFileName strLast=%s\n",strLast.c_str());
						}
					}
				}
				if (strList.size() > (nPos2+1))
				{
					strLeftList.append(strList.c_str() + nPos2+1,strList.size()-nPos2 - 1);
					strList = strLeftList;
				}
				else
				{
					strList = "";
				}

			}
		}
	}
	printf("3 ****:%s\n",strLast.c_str());
	return strLast;
}

/*
* 函数介绍：结束工作
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnEndWorkRep() //(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnEndWorkRep()\n");
//#endif
    LogNormal("结束工作\n");
    //exit(1);
    return true;
}

/*
* 函数介绍：实时记录应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnRealTimeRecordRep() //(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnRealTimeRecordRep()\n");
//#endif
	//LogNormal("in mvOnRealTimeRecordRep()\n");
	m_uRealTimeRep = true;
    m_nCSLinkCount = 0;
    return true;
}

/*
* 函数介绍：历史记录应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvOnHistoryRecordRep() //(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvOnHistoryRecordRep()\n");
//#endif
    m_bHistoryRep = true;
    m_nCSLinkCount = 0;
    return true;
}

/*
* 函数介绍：发送心跳测试
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendLinkTest()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendLinkTest()\n");
//#endif
    return mvRebMsgAndSend(m_nCenterSocket,LINK_TEST, string(""));
}

/*
* 函数介绍：发送结束工作报告
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendEndWork()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendEndWork()\n");
//#endif
    return mvRebMsgAndSend(m_nControlSocket,END_WORK, string(""));
}

/*
* 函数介绍：发送重启应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendRestartRep()
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendRestartRep()\n");
//#endif
    return mvRebMsgAndSend(m_nCenterSocket,DETECT_RESTART_REP, string(""));
}

/*
* 函数介绍：发送标记未发送应答
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendMarkUnsendRep(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendMarkUnsendRep()\n");
//#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,MARK_UNSEND_REP, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送记录查询应答
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendRecordQueryRep(const string &strMsg)
{
////#ifdef CS_STUPID_LOG
////    CSSLog("In mvCCenterServerOneDotEight::mvSendRecordQueryRep()\n");
////#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,RECORD_QUERY_REP, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送设备状态查询应答
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendDeviceStatusRep(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendDeviceStatusRep()\n");
//#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,DEVICE_STATUS_REP, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送设备状态报警
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendDeviceAlarm(const string &strMsg)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvSendDeviceAlarm()\n");
//#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,DEVICE_ALARM, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServerOneDotEight::mvSendRecordToCS(const string &strMsg, bool bRealTime)
{
	if(!g_MybCenterLink)
    {
		//LogNormal("g_MybCenterLink error\n");
        return false;
    }
////#ifdef CS_STUPID_LOG
////    CSSLog("In mvCCenterServerOneDotEight::mvSendRecordToCS()\n");
////#endif
    //including events and plates.
    //strMsg = MIMAX_HEADER+RECORD_EVENT/RECORD_PLATE+picture.
    string strNewMsg("");
    char szBuff[1024] = {0};
    string strTime1(""), strTime2(""), strTime3(""), strPlateNum(""), strPlace(""), strIcon("");
    int nPlateColor, nPlateStruct, nPlateCount, nType;
    char chBodyColor;
	string strPath;

    MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();

    if (MIMAX_EVENT_REP == sHeader->uCmdID)
    {
		return true;
        strPlace = "999";
        strIcon = "99";
        chBodyColor = 'Z';
        nPlateStruct = 5;
        nType = 9;
        nPlateCount = 0;

        RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
        mvGetDateTimeFromSec(pEvent->uEventBeginTime, strTime1);
		mvGetDateTimeFromSec(pEvent->uEventBeginTime, strTime2, 1);
        mvGetDateTimeFromSec(pEvent->uEventBeginTime, strTime3, 1);
        //convertion here.
        UINT uColor = pEvent->uColor1;
        ////int nTplx = 0;
        if (PERSON_TYPE == pEvent->uType)
        {
            strPlateNum = "88888888";
            switch(uColor)
            {
                case 0:
                    nPlateColor = 60;
                    break;
                case 1:
                    nPlateColor = 61;
                    break;
                case 2:
                    nPlateColor = 69;
                    break;
                case 3:
                    nPlateColor = 64;
                    break;
                case 4:
                    nPlateColor = 65;
                    break;
                case 5:
                    nPlateColor = 67;
                    break;
                case 6:
                    nPlateColor = 62;
                    break;
                case 7:
                    nPlateColor = 66;
                    break;
                case 8:
                    nPlateColor = 68;
                    break;
                case 9:
                    nPlateColor = 63;
                    break;
                case 10:
                    nPlateColor = 61;
                    break;
                case 11:
                    nPlateColor = 99;
                    break;
                default:
                    nPlateColor = 70;
            }
            //nTplx = 5;
        }
        else if (OTHER_TYPE == pEvent->uType)
        {
            strPlateNum = "99999999";
            switch(uColor)
            {
                case 0:
                    nPlateColor = 80;
                    break;
                case 1:
                    nPlateColor = 81;
                    break;
                case 2:
                    nPlateColor = 89;
                    break;
                case 3:
                    nPlateColor = 84;
                    break;
                case 4:
                    nPlateColor = 85;
                    break;
                case 5:
                    nPlateColor = 87;
                    break;
                case 6:
                    nPlateColor = 82;
                    break;
                case 7:
                    nPlateColor = 86;
                    break;
                case 8:
                    nPlateColor = 88;
                    break;
                case 9:
                    nPlateColor = 83;
                    break;
                case 10:
                    nPlateColor = 81;
                    break;
                case 11:
                    nPlateColor = 99;
                    break;
                default:
                    nPlateColor = 90;
            }
            //nTplx = 3;
        }
        else
        {
            //机动车
            strPlateNum = "00000000";
            nPlateColor = 4;
            nPlateCount = 1;
            ////nTplx = 1;
            switch(uColor)
            {
                case 0:
                    chBodyColor = 'A';
                    break;
                case 1:
                    chBodyColor = 'B';
                    break;
                case 2:
                    chBodyColor = 'J';
                    break;
                case 3:
                    chBodyColor = 'E';
                    break;
                case 4:
                    chBodyColor = 'F';
                    break;
                case 5:
                    chBodyColor = 'H';
                    break;
                case 6:
                    chBodyColor = 'C';
                    break;
                case 7:
                    chBodyColor = 'G';
                    break;
                case 8:
                    chBodyColor = 'I';
                    break;
                case 9:
                    chBodyColor = 'D';
                    break;
                case 10:
                    chBodyColor = 'B';
                    break;
                case 11:
                    chBodyColor = 'Z';
                    break;
                default:
                    chBodyColor = 'Z';
            }
        }

		

        //识别时间通过随机数产生(40ms-100ms之间)
        int nDetectTime = GetDetectTime();

		if (pEvent->uSpeed > 250)
		{
			pEvent->uSpeed = 255;
		}
		UINT uVioCode = GetEventVioCode(*pEvent);
		UINT nLimitSpeed = 0;
		///*UINT nRedTime = 0;*/
		SetEventLimitSpeed(*pEvent,nLimitSpeed);
        sprintf(szBuff, "%15s%02d%14s%03d%15s%02d%19s%03d%3s%c%02d%02d%010d%d%d%05d%05d%05d%2s%d%03d%06d",
                m_strDetectorId.c_str(), pEvent->uRoadWayID, strTime1.c_str(), pEvent->uMiEventBeginTime, strPlateNum.c_str(),
                nPlateColor, strTime2.c_str(), pEvent->uSpeed, strPlace.c_str(), chBodyColor, 0, 0, nDetectTime, nPlateStruct, nPlateCount, uVioCode,
                pEvent->uPosX, pEvent->uPosY, strIcon.c_str(), nType,nLimitSpeed, pEvent->uTime2);

		string tmpPicMsg = SetEventPicAndVideoMsg(*pEvent,bRealTime);

		strNewMsg = szBuff;
		strNewMsg.append(tmpPicMsg.c_str(),tmpPicMsg.size());
		
		char szJGSJ1[256] = {0};
		sprintf(szJGSJ1,"%19s",strTime3.c_str());
		string strJGSJ1 = szJGSJ1;
		strNewMsg.append(strJGSJ1.c_str(),strJGSJ1.size());
    }
    else if (MIMAX_PLATE_REP == sHeader->uCmdID)
    {
        nPlateCount = 1;

        RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));

		if (pPlate->uViolationType == 0 || pPlate->uViolationType == DETECT_RESULT_NOCARNUM) //卡口数据两张图片（一大图一小图）
		{
			//LogNormal("卡口数据两张图片（一大图一小图）\n");
			if(g_nSendViolationOnly == 1)
			{
				return true;
			}
		}
		//printf("send one kakou data\n");
        mvGetDateTimeFromSec(pPlate->uTime, strTime1);
		
		if(pPlate->uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)//区间超速取驶入时间
		{
			mvGetDateTimeFromSec(pPlate->uTime2, strTime2, 1);
		}
		else
		{
			mvGetDateTimeFromSec(pPlate->uTime, strTime2, 1);
		}
        
		mvGetDateTimeFromSec(pPlate->uTime, strTime3, 1);
		if(pPlate->chText[0]=='1')//
        {
            memcpy(pPlate->chText,"00000000",8);
        }
        //convertion here.
        strPlateNum = pPlate->chText;
		if (pPlate->chText[0] == '0')
		{
			strPlace = "999";
		}
		else
		{
			strPlace.append(pPlate->chText, 3);
		}
        nPlateColor = pPlate->uColor;
        switch(nPlateColor)
        {
            case 1: //blue
                nPlateColor = 2;
                break;
            case 2: //black
                nPlateColor = 3;
                break;
            case 3: //yellow
                nPlateColor = 1;
                break;
            case 4: //white
                nPlateColor = 0;
                break;
            default: //other
                nPlateColor = 99;
        }

        UINT uCarColor = pPlate->uCarColor1;
        switch(uCarColor)
        {
            case 0:
                chBodyColor = 'A';
                break;
            case 1:
                chBodyColor = 'B';
                break;
            case 2:
                chBodyColor = 'J';
                break;
            case 3:
                chBodyColor = 'E';
                break;
            case 4:
                chBodyColor = 'F';
                break;
            case 5:
                chBodyColor = 'H';
                break;
            case 6:
                chBodyColor = 'C';
                break;
            case 7:
                chBodyColor = 'G';
                break;
            case 8:
                chBodyColor = 'I';
                break;
            case 9:
                chBodyColor = 'D';
                break;
            case 10:
                chBodyColor = 'B';
                break;
            case 11:
                chBodyColor = 'Z';
                break;
            default:
                chBodyColor = 'Z';
        }

        UINT uStruct = pPlate->uPlateType;
        switch(uStruct)
        {
            case 1:
                nPlateStruct = 1;
                break;
            case 2:
                nPlateStruct = 4;
                break;
            default:
                nPlateStruct = 5;
        }

        UINT uType = pPlate->uType;
        switch(uType)
        {
            case 1:
                nType = 1;
                break;
            case 2:
                nType = 2;
                break;
            case 3:
                nType = 3;
                break;
            default:
                nType = 9;
        }

        UINT uIcon = pPlate->uCarBrand;
        switch(uIcon)
        {
            case AUDI:
                strIcon = "A0";
                break;
            case BMW:
                strIcon = "B5";
                break;
            case HYUNDAI:
                strIcon = "X0";
                break;
            case BENZ:
                strIcon = "B0";
                break;
            case HONDA:
                strIcon = "B7";
                break;
            case VW:
                strIcon = "D6";
                break;
            case MAZDA:
                strIcon = "M4";
                break;
            case TOYOTA:
                strIcon = "F2";
                break;
            case BUICK:
                strIcon = "B3";
                break;
            case CHEVROLET:
                strIcon = "X1";
                break;
            case CITERON:
                strIcon = "X3";
                break;
            case PEUGEOT:
                strIcon = "BA";
                break;
            case FORD:
                strIcon = "F3";
                break;
            case LEXUS:
                strIcon = "L7";
                break;
            case NISSAN:
                strIcon = "R0";
                break;
            case CHERY:
                strIcon = "Q1";
                break;
            case BYD:
                strIcon = "B9";
                break;
            case KIA:
                strIcon = "Q2";
                break;
            case ROWE:
                strIcon = "R2";
                break;
            case MITSUBISHI:
                strIcon = "S6";
                break;
            case SKODA:
                strIcon = "S5";
                break;
            case SUZUKI:
                strIcon = "L8";
                break;
            case CHANGHE:
                strIcon = "L8";
                break;
            case FIAT:
                strIcon = "F5";
                break;
            case VOLVO:
                strIcon = "W0";
                break;
            case JEEP:
                strIcon = "J6";
                break;
            case LANDROVER:
                strIcon = "L3";
                break;
            case GMC:
                strIcon = "G0";
                break;
            case RedFlags:
                strIcon = "H5";
                break;
            case HUMMER:
                strIcon = "H2";
                break;
            case JINBEI:
                strIcon = "J2";
                break;
            case JAC:
                strIcon = "J3";
                break;
            case JMC:
                strIcon = "J7";
                break;
            case GEELY:
                strIcon = "J4";
                break;
            case LANDWIND:
                strIcon = "L2";
                break;
            case LIFAN:
                strIcon = "L1";
                break;
            case MG:
                strIcon = "M1";
                break;
            case ACURA:
                strIcon = "M6";
                break;
            case INFINITI:
                strIcon = "Y0";
                break;
            case FLYWITHOUT:
                strIcon = "Z0";
                break;
            case ZOTYEAUTO:
                strIcon = "Z1";
                break;
            case RELY:
                strIcon = "W1";
                break;
            case SUBARU:
                strIcon = "S1";
                break;
            case SPYKER:
                strIcon = "S4";
                break;
            case SHUANGHUANAUTO:
                strIcon = "S2";
                break;
            case SAAB:
                strIcon = "S0";
                break;
            case WIESMANN:
                strIcon = "W3";
                break;
            case GLEAGLE:
                strIcon = "Q0";
                break;
            case GONOW:
                strIcon = "J0";
                break;
            case HAWTAIAUTOMOBILE:
                strIcon = "H6";
                break;
            case SMA:
                strIcon = "H4";
                break;
            case HAFEI:
                strIcon = "H0";
                break;
            case SOUEAST:
                strIcon = "D4";
                break;
            case EMGRANO:
                strIcon = "D3";
                break;
            case CHANA:
                strIcon = "C0";
                break;
            case CHANGFENAMOTOR:
                strIcon = "C2";
                break;
            case GREATWALL:
                strIcon = "C1";
                break;
            case DAEWOO:
                strIcon = "D7";
                break;
            case ISUZU:
                strIcon = "L8";
                break;
            case DAIHATSU:
                strIcon = "D1";
                break;
            case JAGUAR:
                strIcon = "J1";
                break;
            case OPEL:
                strIcon = "M5";
                break;
            case CHRYSLER:
                strIcon = "K1";
                break;
            case ALFAROMEO:
                strIcon = "A1";
                break;
            case LINCOLN:
                strIcon = "L6";
                break;
            case ROLLSROYCE:
                strIcon = "LA";
                break;
            case FERRARI:
                strIcon = "F1";
                break;
            case PORSCHE:
                strIcon = "B6";
                break;
            case LOTUS:
                strIcon = "L4";
                break;
            case ASTONMARTIN:
                strIcon = "A2";
                break;
            case CROWN:
                strIcon = "F2";
                break;
            case BESTURN:
                strIcon = "B8";
                break;
            case DONGFENG:
                strIcon = "D0";
                break;
            case POLARSUN:
                strIcon = "P1";
                break;
            case FOTON:
                strIcon = "F4";
                break;
            case WULING:
                strIcon = "W2";
                break;
            case CADILLAC:
                strIcon = "K0";
                break;
            case MASERATI:
                strIcon = "M3";
                break;
            default:    //If we need more, then add them here as these samples.
                strIcon = "99";
        }

        //识别时间通过随机数产生(40ms-100ms之间)
        int nDetectTime = GetDetectTime();

		if (pPlate->uSpeed > 250)
		{
			pPlate->uSpeed = 255;
		}
		UINT uVioCode = GetPlateVioCode(*pPlate);
		UINT nLimitSpeed = 0;
		/*UINT uRedTime = 0;*/
		SetPlateLimitSpeed(*pPlate,nLimitSpeed);
		//LogNormal("RedTime:%d\n",pPlate->uTime2);
        sprintf(szBuff, "%15s%02d%14s%03d%15s%02d%19s%03d%3s%c%02d%02d%010d%d%d%05d%05d%05d%2s%d%03d%06d",
                m_strDetectorId.c_str(), pPlate->uRoadWayID, strTime1.c_str(), pPlate->uMiTime,strPlateNum.c_str(),
                nPlateColor, strTime2.c_str(), pPlate->uSpeed, strPlace.c_str(), chBodyColor, 0, 0, nDetectTime, nPlateStruct, nPlateCount, uVioCode,
                pPlate->uPosLeft, pPlate->uPosTop, strIcon.c_str(), nType, nLimitSpeed,pPlate->uSignalTime);

		string tmpPicMsg = SetPlatePicAndVideoMsg(*pPlate,bRealTime);
		//如果违章图片不存在则不发送数据给中心
		if(tmpPicMsg.size() <= 0)
		{
			//printf("tmpPicMsg.size() <= 0\n");
			return true;
		}
        strNewMsg = szBuff;
		strNewMsg.append(tmpPicMsg.c_str(),tmpPicMsg.size());
		char szJGSJ1[256] = {0};
		sprintf(szJGSJ1,"%19s",strTime3.c_str());
		string strJGSJ1 = szJGSJ1;
		strNewMsg.append(strJGSJ1.c_str(),strJGSJ1.size());
    }

    int nMsgLength = strNewMsg.size() + 8;
    char szLength[8] = {0};
    sprintf(szLength, "%07d", nMsgLength);
    strNewMsg.insert(0, szLength, 7);

    string strType("");
    if (bRealTime)
    {
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSSendLog, "\n=========发送====实时数据=====>>>到电科中心端!\n");
//        fflush(m_pCSSendLog);
//#endif
        strType = REALTIME_RECORD;
		m_uRealTimeRep = false;
    }
    else
    {
//#ifdef CS_FILE_LOG
//        fprintf(m_pCSSendLog, "\n=========发送====历史数据=====>>>到电科中心端!\n");
//        fflush(m_pCSSendLog);
//#endif
        strType = HISTORY_RECORD;
        m_bHistoryRep = false;
        m_uHistoryRepTime = GetTimeStamp();//记住发送时刻
    }
    return mvRebMsgAndSend(m_nCenterSocket,strType.c_str(), strNewMsg);
}

unsigned long long  mvCCenterServerOneDotEight::GetFileLine(string strFileName)
{
	ifstream ReadFile;
	unsigned long long nLine = 0;
	string temp;
	ReadFile.open(strFileName.c_str(),ifstream::in);
	if (ReadFile.fail())
	{
		printf("打开文件失败:%s\n",strFileName.c_str());
		return 0;
	}
	else
	{
		m_PlateMap.clear();
		while(getline(ReadFile,temp))
		{
			if (nLine != 0)
			{
				AddPlate2Map(temp);
			}
			nLine++;
		}
	}
	ReadFile.close();

	if(nLine > 0)
	{
		nLine -= 1;
	}

	/*FILE* fp = fopen("md5.txt","a+");
	fprintf(fp,"[%s][mvCCenterServerOneDotEight::GetFileLine]:GetFileLine:Record:%d MapSize:%d \n",GetTime(GetTimeStamp(),0).c_str(),nLine,m_PlateMap.size());
	fclose(fp);*/

	return nLine;
}

void mvCCenterServerOneDotEight::AddPlate2Map(string strLine)
{
	string strHPHM("");
	int nHPYS = 0;
	int nWFLX = 0;
	string strTemp("");
	int nPos = -1;
	
	nPos = strLine.find(",");
	if (nPos > 0)
	{
		strHPHM = strLine.substr(0,nPos);
		strTemp = strLine.substr(nPos+1,strLine.size()- nPos - 1);

		nPos = -1;
		nPos = strTemp.find(",");
		if (nPos > 0)
		{
			nHPYS = atoi(strTemp.substr(0,nPos).c_str());
			if(strTemp.size()- nPos - 1 > 0)
			{
				strTemp = strTemp.substr(nPos+1,strTemp.size()- nPos - 1);
				
				if(strTemp.size() > 0)
				{
					nWFLX = atoi(strTemp.c_str());
				
					g_skpDB.GBKToUTF8(strHPHM);
				//	FoundPlateInfo((char*)(strHPHM.c_str()),nHPYS); //test
					VEHICLE_INFO info;
					info.nHPYS = nHPYS;
					info.nWFLX = nWFLX;
					m_PlateMap.insert(make_pair(strHPHM,info));
				}
			}
		}
	}
	//printf("***************%s:%d:%d\n",strHPHM,nHPYS,nWFLX);
}

int mvCCenterServerOneDotEight::FoundPlateInfo(char* strText,int nColor)
{
	string strPlateText;
	strPlateText = strText;
	CarNumConvert(strPlateText,"");
	int nWFLX = 0;
	map<string, VEHICLE_INFO>::iterator iter;
	if(m_PlateMap.size() > 0)
	{
		iter = m_PlateMap.find(strPlateText);
		if(iter != m_PlateMap.end())
		{
			pair<string, VEHICLE_INFO> p = *iter;
			int nPlateColor = (int)p.second.nHPYS;

			int nConvColor = 4;
			switch (nColor)
			{
				case CARNUM_BLUE:
					nConvColor = 2;
					break;
				case CARNUM_BLACK:
					nConvColor = 3;
					break;
				case CARNUM_YELLOW:
					nConvColor = 1;
					break;
				case CARNUM_WHITE:
					nConvColor = 0;
					break;
			}
			if (nPlateColor == nConvColor)
			{
				LogNormal("FoundPlateInfo:%s\n",strPlateText.c_str());
				nWFLX = (int)p.second.nWFLX;
			}
			/*FILE* fp = fopen("md5.txt","a+");
			fprintf(fp,"[%s][mvCCenterServerOneDotEight::FoundPlateInfo]:FoundPlateInfo:%s Color:%d MapColor:%d\n",GetTime(GetTimeStamp(),0).c_str(),strPlateText.c_str(),nColor,nPlateColor);
			fclose(fp);*/
		}
		else
		{
			printf("NOT FoundPlateInfo:%s\n",strPlateText.c_str());
		}
	}
	return nWFLX;
}

bool mvCCenterServerOneDotEight::mvSendHMDToCS(string strFileName)
{
	string strNewMsg;
	char szBuff[1024] = {0};
	unsigned long long ullRecordNum = 0;
	string strMd5;
	char strcvsFile[1024] = {0};

	sprintf(strcvsFile,"./config/%s",strFileName.c_str());

	string strTime = GetTime(GetTimeStamp(),0);

	ullRecordNum = GetFileLine(strcvsFile);

	strMd5 = MD5_File(strcvsFile,32);

	sprintf(szBuff,"%50s%07d%32s%19s",m_strRemoteFile.c_str(),ullRecordNum,strMd5.c_str(),strTime.c_str());
	strNewMsg = szBuff;
	LogNormal("记录总数:%d,MD5值:%s\n",ullRecordNum,strMd5.c_str());
	/*FILE* fp = fopen("md5.txt","a+");
	fprintf(fp,"[%s][mvCCenterServerOneDotEight::mvSendHMDToCS]Send:%50s:%07d:%32s:%19s\n",GetTime(GetTimeStamp(),0).c_str(),m_strRemoteFile.c_str(),ullRecordNum,strMd5,strTime.c_str());
	fclose(fp);*/

	return mvRebMsgAndSend(m_nCenterSocket,HMD_QUERY, strNewMsg);
}

string mvCCenterServerOneDotEight::MD5_File(const char* path,int md5_len)
{  
	string strMd5 = GetMd5((char*)path);
	return strMd5;
}

/*
* 函数介绍：CRC编码
* 输入参数：strSrc-编码前内容；strRes-编码后内容变量
* 输出参数：strRes-编码后内容
* 返回值 ：成功返回true，否则返回false
*/
void mvCCenterServerOneDotEight::mvCRCEncode(const string &strSrc, string &strRes)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvCRCEncode()\n");
//#endif
    unsigned int crc = 0xffffffff;

    for (int i = 0; i < strSrc.size(); i++)
    {
        crc = ((crc >> 8) & 0xFFFFFF) ^ mycrcTable[(BYTE)((crc & 0xff) ^ strSrc[i])];
    }
    crc = crc ^ 0xffffffff;

    char szRes[9] = {0};
    sprintf(szRes, "%8x", crc);

    if (!strRes.empty())
    {
        strRes.clear();
    }
    strRes = szRes;
}

/*
* 函数介绍：获得日期
* 输入参数：uSec-秒数；strDate-日期变量；nType-日期格式
* 输出参数：strDate-根据秒数换算得到的日期
* 返回值 ：成功返回true，否则返回false
*/
void mvCCenterServerOneDotEight::mvGetDateTimeFromSec(const long &uSec, string &strDate, int nType)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvGetDateTimeFromSec()\n");
//#endif
	char szBuff[128] = {0};
	struct tm *newTime, timenow;
	newTime = &timenow;
	localtime_r(&uSec, newTime);

    if (0 == nType) //yyyymmddhhmmss
    {
        sprintf(szBuff, "%4d%02d%02d%02d%02d%02d", newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday, newTime->tm_hour, newTime->tm_min, newTime->tm_sec);
    }
    else //yyyy-mm-dd hh:mm:ss
    {
        sprintf(szBuff, "%4d-%02d-%02d %02d:%02d:%02d", newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday, newTime->tm_hour, newTime->tm_min, newTime->tm_sec);
    }

	if (!strDate.empty())
	{
	    strDate.clear();
	}
	strDate = szBuff;
}

/*
* 函数介绍：检查设备状态
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCCenterServerOneDotEight::mvCheckDeviceStatus(int nDeviceStatusKind)
{
//#ifdef CS_STUPID_LOG
//    CSSLog("In mvCCenterServerOneDotEight::mvCheckDeviceStatus()\n");
//#endif

	//LogNormal("in mvCCenterServerOneDotEight::mvCheckDeviceStatus\n");
	if(nDeviceStatusKind == 0)//硬件报警
	{
		if (g_sysInfo.fCpu > 99)
		{
			/*上海交警项目反馈：检测器接两路视频，一天会报出将近20次左右CPU过高的情况，
			对此加以限制，当cpu过高的情况大于600次时，才给中心端发送一次cpu过高报警
			*/
			m_nCpuAlarmCount += 1;
			if (m_nCpuAlarmCount > 600)
			{
				char szBuff[4] = {0};
				sprintf(szBuff, "%c%2s", '0', "01");
				mvSendDeviceAlarm(string(szBuff));
				m_nCpuAlarmCount = 0;
			}
			
		}

		if (g_sysInfo.fDisk > 95)
		{
			char szBuff[4] = {0};
			sprintf(szBuff, "%c%2s", '0', "02");
			mvSendDeviceAlarm(string(szBuff));
		}
	}
	else if(nDeviceStatusKind == 2)//环境检测报警
	{
			/*********************
			0 硬件报警
			1 软件报警
			2 环境检测报警

			（环境检测报警类型）
			01：机箱开门
			02：机箱关门
			03：机箱内温度超限
			04：机箱内温度过低
			05：机箱震动报警

			（软件报警）
			01： 软件异常

			（硬件报警）
			01:CPU占有率高
			02:硬盘剩余空间不够
			*********************/

		//环境类报警(暂时模拟)
		//srand( (unsigned)time( NULL ) );
		//int nTemperature = 30 + (int)(35.0*rand()/(RAND_MAX+1.0));
		if(g_nHasExpoMonitor)
		{
			//开关门
		   // if(g_ExpoMonitorInfo.nGateStateAlarm > 0)
			{
				if(g_ExpoMonitorInfo.nGateValue == 1)
				{
					//01：机箱开门
					char szBuff[4] = {0};
					sprintf(szBuff, "%c%2s", '2', "01");
					mvSendDeviceAlarm(string(szBuff));
				}
				else if(g_ExpoMonitorInfo.nGateValue == 0)
				{
					//02：机箱关门
					char szBuff[4] = {0};
					sprintf(szBuff, "%c%2s", '2', "02");
					mvSendDeviceAlarm(string(szBuff));
				}
				else {}
			}

			//获取环境温度
		   // if(g_ExpoMonitorInfo.nTemperatureAlarm > 0) //环境温度报警状态
			{
				if(g_ExpoMonitorInfo.nTemperatureValue > -100 && g_ExpoMonitorInfo.nTemperatureUp > -100)
				{
					if(g_ExpoMonitorInfo.nTemperatureValue > g_ExpoMonitorInfo.nTemperatureUp)
					{
						//03：机箱内温度超限
						char szBuff[4] = {0};
						sprintf(szBuff, "%c%2s", '2', "03");
						mvSendDeviceAlarm(string(szBuff));
					}
					else if(g_ExpoMonitorInfo.nTemperatureValue < g_ExpoMonitorInfo.nTemperatureDown)
					{
						//04：机箱内温度过低
						char szBuff[4] = {0};
						sprintf(szBuff, "%c%2s", '2', "04");
						mvSendDeviceAlarm(string(szBuff));
					}
					else {}
				}
			}

		} //End of if(g_nHasExpoMonitor)
	}

}

//接收中心端消息
void mvCCenterServerOneDotEight::mvRecvCenterServerMsg()
{
	while(!m_bEndRecvThread)
	{
		if(m_nCenterSocket > 0)
		{
			string strMsg("");
			//receive msg and push it into the msg queue.
			if (mvRecvSocketMsg(m_nCenterSocket, strMsg))
			{
	////#ifdef CS_FILE_LOG
	////            fprintf(m_pCSRecvLog, "\n接收消息成功:\n %s\n", strMsg.c_str());
	////            fflush(m_pCSRecvLog);
	////#endif
				mvPushOneMsg(strMsg);
			}
		}
		usleep(100);
	}
}

//添加一条数据
bool mvCCenterServerOneDotEight::AddResult(std::string& strMsg)
{
	//LogNormal("AddResult start\n");
	if(!g_MybCenterLink)
    {
        return false;
    }
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strMsg.c_str();

	switch(sDetectHeader->uDetectType)
	{
       case MIMAX_EVENT_REP:	//事件(送中心端)
	   case MIMAX_PLATE_REP:   //车牌
	   case MIMAX_STATISTIC_REP:  //统计
	   case PLATE_LOG_REP:   //日志
	   case EVENT_LOG_REP:
	        //加锁
	        pthread_mutex_lock(&m_Result_Mutex);
	        if(m_ChannelResultList.size() > 5)//防止堆积的情况发生
	        {
	           // LogError("记录过多-%d，未能及时发送!\r\n",m_ChannelResultList.size());
                m_ChannelResultList.pop_back();
	        }
			m_ChannelResultList.push_front(strMsg);
			//解锁
	        pthread_mutex_unlock(&m_Result_Mutex);
			break;
	   default:
			LogError("未知数据[%x],取消操作!\r\n",sDetectHeader->uDetectType);
			return false;
	}
	return true;
}

//处理检测结果
bool mvCCenterServerOneDotEight::OnResult(std::string& result)
{
	//发送数据
	MIMAX_HEADER mHeader;
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();

	bool bSendToServer = false;
	bool bObject = false;
	//LogNormal("OnResult:处理检测结果\n");
	switch(sDetectHeader->uDetectType)
	{
			////////////////////////////////////////////
		case MIMAX_EVENT_REP:	//事件(送中心端)
		case MIMAX_STATISTIC_REP:  //统计
		case PLATE_LOG_REP:  //日志
		case EVENT_LOG_REP:
		case MIMAX_PLATE_REP:  //车牌
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
			LogError("未知数据[%d],取消操作!\r\n",sDetectHeader->uChannelID);
			return false;
	}
	//if(mHeader.uCmdID == MIMAX_STATISTIC_REP)
	//{
	//	LogNormal("MIMAX_STATISTIC_REP\n");
	//	RECORD_STATISTIC* pStatistic = (RECORD_STATISTIC *)(strMsg.c_str() + sizeof(SRIP_DETECT_HEADER));
	//	int nCount = 0;
	//	for(int i=0; i<MAX_ROADWAY; i++)
	//	{
	//		if(pStatistic->uFlux[i]!=0xffffffff)
	//		{
	//			nCount++;
	//		}
	//	}
	//}

    //此时去取图片
    if(mHeader.uCmdID == MIMAX_EVENT_REP)
    {
        if( (mHeader.uCmdFlag & 0x00010000) == 0x00010000)
        {
            bObject = true;
            mHeader.uCmdFlag = 0x00000001;
        }
    /*    RECORD_EVENT* sEvent = (RECORD_EVENT*)(result.c_str());
        String strPicPath(sEvent->chPicPath);
        result = result + GetImageByPath(strPicPath);*/
    }
    else if(mHeader.uCmdID == MIMAX_PLATE_REP)
    {
       /* RECORD_PLATE* sPlate = (RECORD_PLATE*)(result.c_str());
        String strPicPath(sPlate->chPicPath);
        result = result + GetImageByPath(strPicPath);*/
    }

    //数据长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + result.size();
	//添加头
	result.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

    //发送数据
    if(bSendToServer)
    {
		//LogNormal("bSendToServer:%d\n",bSendToServer);
        if( (mHeader.uCmdID == MIMAX_EVENT_REP)||(mHeader.uCmdID == MIMAX_PLATE_REP))
        {
            if (g_MyCenterServer.mvSendRecordToCS(result,true))
            {
				//LogNormal("g_MyCenterServer.mvSendRecordToCS\n");
				/*usleep(1000*10);
				if (!m_uRealTimeRep)
				{
					LogNormal("m_uRealTimeRep is false\n");
					return false;
				}*/
				/*if (!IsRealTimeRepTrue())
				{
					return false;
				}*/
                unsigned int uSeq =*((unsigned int*)(result.c_str()+sizeof(MIMAX_HEADER)));
                if(bObject)
                {
                    g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP,uSeq);
                }
                else
                {
                    g_skpDB.UpdateRecordStatus(mHeader.uCmdID,uSeq);
                }
                return true;
            }
        }
    }
    return false;
}

//实时记录发送后是否收到中心端的回复
bool mvCCenterServerOneDotEight::IsRealTimeRepTrue()
{
	int nCount = 200;
	/*unsigned int uTimeStamp1 = GetTimeStamp();
	LogNormal("uTimeStamp1:%d",uTimeStamp1);*/
	while (!m_uRealTimeRep)
	{
		usleep(1000);
		nCount -= 1;
		if (nCount == 0)
		{
			return false;
		}
	}
	/*unsigned int uTimeStamp2 = GetTimeStamp();
	LogNormal("uTimeStamp2:%d,nCount=%d",uTimeStamp2,nCount);*/
	return true;
}

//处理记录结果
void mvCCenterServerOneDotEight::DealResult()
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
			//取最早命令
			CSResultMsg::iterator it = m_ChannelResultList.begin();
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
			//LogNormal("DealResult:处理消息[response1.size：%d]\n",response1.size());
			OnResult(response1);
		}
		//10毫秒
		usleep(1000*1);
	}
}

/*
* 函数介绍：接收消息
* 输入参数：nSocket-要接收消息的套接字；strMsg-将接收到的消息内容
* 输出参数：strMsg-接收到的消息内容
* 返回值 ：成功返回true，否则false
*/
bool mvCCenterServerOneDotEight::mvRecvSocketMsg(int nSocket, string& strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }
    if (nSocket <= 0)
    {
        return false;
    }

    int nLeft = 0;
    char szHeader[5] = {0};    //header.
    char szBuffer[128] = {0};   //max length is 72 Bytes.
    string strCode("");

    //printf("receiving 1111111111111\n");

    if (recv(nSocket, szHeader, 4, MSG_NOSIGNAL) < 0)
    {
        return false;
    }
    strCode.append(&szHeader[1], 3);
    //printf("receiving 22222222222222, strCode=%s\n", strCode.c_str());
    if (START_WORK_REP == strCode)
    {
        nLeft = 52+15;
    }
    else if (END_WORK_REP == strCode)
    {
        nLeft = 10+15;
    }
    else if (REALTIME_RECORD_REP == strCode)
    {
		//m_uFlag = true;
        nLeft = 43+15;
    }
    else if (HISTORY_RECORD_REP == strCode)
    {
		//m_uFlag = true;
        nLeft = 43+15;
    }
    else if (DETECT_RESTART == strCode)
    {
        nLeft = 31+15;
    }
    else if (DEVICE_STATUS == strCode)
    {
        nLeft = 9+15;
    }
    else if (DEVICE_ALARM_REP == strCode)
    {
        nLeft = 9+15;
    }
    else if (RECORD_QUERY == strCode)
    {
       // nLeft = 53+15;
		nLeft = 58+15;
    }
    else if (MARK_UNSEND == strCode)
    {
        //nLeft = 53+15;
		nLeft = 54+15;
    }
    else if (LINK_TEST_REP == strCode)
    {
        nLeft = 9+15;
    }
    else if (SYSTIME_SETUP == strCode) //Receive it but will not process it.
    {
        nLeft = 31+15;
    }
	else if (HMD_QUERY_REP == strCode)
	{
		nLeft = 79+15;
	}
    else
    {
        return false;
    }

    //printf("receiving 3333333333333\n");
    if (recv(nSocket, szBuffer, nLeft, MSG_NOSIGNAL) < 0)
    {
        return false;
    }

    //printf("receiving 4444444444444\n");
    //printf("strlen(szHeader)=%d, szHeader=%s\n", strlen(szHeader), szHeader);
    //printf("strlen(szBuffer)=%d, szBuffer=%s\n", strlen(szBuffer), szBuffer);
    strMsg += szHeader;
    strMsg += szBuffer;

    return (!strMsg.empty());
}

//设置线圈状态
void mvCCenterServerOneDotEight::SetLoopStatus(mapLoopStatus& vLoopStatus)
{
	m_vLoopStatus = vLoopStatus;
}

//设置通道对应的线圈状态
void mvCCenterServerOneDotEight::SetChanLoopStatus(mapChanIdLoopStatus& vChanLoopStatus)
{
	m_vChanIdLoopStatus = vChanLoopStatus;
}

//获取线圈状态
mapLoopStatus mvCCenterServerOneDotEight::GetLoopStatus()
{
	return m_vLoopStatus;
}

//获取通道线圈对应的线圈状态
mapChanIdLoopStatus mvCCenterServerOneDotEight::GetChanLoopStatus()
{
	return m_vChanIdLoopStatus;
}

//设置文本框的高度
void mvCCenterServerOneDotEight::SetExtentHeight(int nExtentHeight)
{
	m_uExtentHeight = nExtentHeight;
}

//设置检测方向
void mvCCenterServerOneDotEight::setDirection(int nDirection)
{
	m_uDetectDirection = nDirection;
}

//设置解码后图片宽度
void mvCCenterServerOneDotEight::SetPicWidth(int nPicWidth)
{
	m_uPicWidth = nPicWidth;
}

//设置解码后图片高度
void mvCCenterServerOneDotEight::SetPicHeight(int nPicHeight)
{
	m_uPicHeight = nPicHeight;
}

//对行人和非机动车抠图
CvRect mvCCenterServerOneDotEight::GetPos(RECORD_EVENT pevent,int nType)
{
	int x = 0;
	int y = 0;
	int w = 0;//宽度
	int h = 0;//高度

	int nWidth = m_uPicWidth;
	int nHeight = m_uPicHeight;

	CvRect rtCar;

	int nExtentHeight = 0;
	if(g_PicFormatInfo.nWordPos == 1)
	{
		nExtentHeight = m_uExtentHeight;
	}
	if (nType == 0)
	{
		CvPoint point;
		point.x = pevent.uPosX*1.0;
		point.y = pevent.uPosY*1.0;

		if(m_uPicWidth > 2000)
		{
			rtCar.width = 800;
			rtCar.height = rtCar.width/m_uRatio;
		}
		else
		{
			rtCar.width = 500;
			rtCar.height = rtCar.width/m_uRatio;
		}

		x = point.x - rtCar.width/2;
		y = point.y - rtCar.height/2;
	}
	if(x > 0)
	{
		rtCar.x = x;
	}
	else
	{
		rtCar.x = 0;
	}

	if(y > nExtentHeight)
	{
		rtCar.y = y;
	}
	else
	{
		rtCar.y = nExtentHeight;
	}


	if(rtCar.x+rtCar.width>=nWidth)
	{
		rtCar.x = nWidth - rtCar.width-1;
	}

	if(rtCar.y+rtCar.height>=nHeight)
	{
		rtCar.y = nHeight - rtCar.height-1;
	}

	rtCar.y -= m_uExtentHeight;//抠图中可能会抠出黑边，去掉该黑边。

	// LogNormal("rtCar2=%d,%d,%d,%d\n",rtCar.x,rtCar.y,rtCar.width,rtCar.height);

	return rtCar;
}

//获取车身位置
CvRect mvCCenterServerOneDotEight::GetCarPos(RECORD_PLATE plate,int nType)
{
	int x = 0;
	int y = 0;
	int w = 0;//宽度
	int h = 0;//高度
	int mExtentHeight = g_PicFormatInfo.nExtentHeight;
	
	if (plate.uPicHeight ==  (DSP_500_BIG_HEIGHT + 96))
	{
		mExtentHeight = 96;
	}
	else if (plate.uPicHeight ==  (DSP_500_BIG_HEIGHT + 64))
	{
		mExtentHeight = 64;
	}
	else if (plate.uPicHeight ==  (DSP_600_BIG_465_HEIGHT + 112))
	{
		mExtentHeight = 112;
	}
	int nWidth = plate.uPicWidth - mExtentHeight;/*m_uPicWidth*/
	int nHeight = plate.uPicHeight - mExtentHeight;/*m_uPicHeight*/

	if (nWidth <= 0)
	{
		LogError("22222宽度小于等于0\n");
	}
	if (nHeight <= 0)
	{
		LogError("22222高度小于等于0\n");
	}

	/*cerr<<"11111Width:"<<nWidth<<endl;
	cerr<<"2222222Height:"<<nHeight<<endl;*/

	float nRatio = 0.0;
	nRatio = nWidth*1.0/nHeight;

	//cerr<<"33333333nRatio:"<<nRatio<<endl;

	CvRect rtCar;

	int nExtentHeight = 0;
	if(g_PicFormatInfo.nWordPos == 1)
	{
		nExtentHeight = mExtentHeight;
	}

	if(nType == 0)
	{
		if(plate.chText[0] != '1' && plate.chText[0] != '0')//有牌车
		{
			w = plate.uPosRight - plate.uPosLeft;//宽度
			h = plate.uPosBottom - plate.uPosTop;//高度
			
			int dw = 2*w;
			if( (w > 120) && (nWidth < 2000))//200万小场景
			{
				if(plate.uType == SMALL_CAR)
				{
					dw = 2*w;
					rtCar.width = 5*w;
				}
				else
				{
					dw = 3*w;
					rtCar.width = 7*w;
				}
			}
			else
			{
				if(plate.uType == SMALL_CAR)
				{
					dw = 3*w;
					rtCar.width = 7*w;
				}
				else
				{
					dw = 4*w;
					rtCar.width = 9*w;
				}
			}
			//rtCar.height = rtCar.width/m_uRatio;
			rtCar.height = rtCar.width/nRatio;
			x = plate.uPosLeft-dw;

			if(m_uDetectDirection == 0)
			{
				y = plate.uPosTop-rtCar.height+6*h;
			}
			else
			{
				y = plate.uPosTop-rtCar.height+8*h;
			}
		}
		else
		{
			CvPoint point;
			point.x = (plate.uPosRight + plate.uPosLeft)/2.0;
			point.y = (plate.uPosBottom + plate.uPosTop)/2.0;
				
			if(plate.uPicWidth > 2000)
			{
				rtCar.width = 800;
				//rtCar.height = rtCar.width/m_uRatio;
				rtCar.height = rtCar.width/nRatio;
			}
			else
			{
				rtCar.width = 500;
				//rtCar.height = rtCar.width/m_uRatio;
				rtCar.height = rtCar.width/nRatio;
			}

			x = point.x - rtCar.width/2;
			y = point.y - rtCar.height/2;
		}
	}
	else if(nType == 1)
	{
		CvPoint point;
		point.x = (plate.uPosRight + plate.uPosLeft)/2.0;
		point.y = (plate.uPosBottom + plate.uPosTop)/2.0;

		rtCar.width = 800;
		//rtCar.height = rtCar.width/m_uRatio;
		rtCar.height = rtCar.width/nRatio;

		x = point.x - rtCar.width/2;
		y = point.y - rtCar.height/2;

		nHeight -= mExtentHeight;
	}
	else if(nType == 2)
	{
		if(plate.chText[0] != '1' && plate.chText[0] != '0')//有牌车
		{
			w = plate.uPosRight - plate.uPosLeft;//宽度
			h = plate.uPosBottom - plate.uPosTop;//高度
			int dw = 2*w;
			if(plate.uType == SMALL_CAR)
			{
				dw = 2.5*w;
				rtCar.width = 6*w;
			}
			else
			{
				dw = 3.5*w;
				rtCar.width = 8*w;
			}
			//rtCar.height = rtCar.width/m_uRatio;
			rtCar.height = rtCar.width/nRatio;
			x = plate.uPosLeft-dw;

			if(m_uDetectDirection == 0)
			{
				y = plate.uPosTop-rtCar.height+12*h;
			}
			else
			{
				y = plate.uPosTop-rtCar.height+8*h;
			}
		}
		else
		{
			CvPoint point;
			point.x = (plate.uPosRight + plate.uPosLeft)/2.0;
			point.y = (plate.uPosBottom + plate.uPosTop)/2.0;

			if(plate.uPicWidth > 2000)
			{
				rtCar.width = 800;
				//rtCar.height = rtCar.width/m_uRatio;
				rtCar.height = rtCar.width/nRatio;
			}
			else
			{
				rtCar.width = 500;
				//rtCar.height = rtCar.width/m_uRatio;
				rtCar.height = rtCar.width/nRatio;
			}

			x = point.x - rtCar.width/2;
			y = point.y - rtCar.height/2;
		}

		nHeight -= mExtentHeight;
		nExtentHeight = 0;
	}

	if(x > 0)
	{
		rtCar.x = x;
	}
	else
	{
		rtCar.x = 0;
	}

	if(y > nExtentHeight)
	{
		rtCar.y = y;
	}
	else
	{
		rtCar.y = nExtentHeight;
	}


	if(rtCar.x+rtCar.width>=nWidth)
	{
		
		rtCar.x = nWidth - rtCar.width-1;
	}

	if(rtCar.y+rtCar.height>=nHeight)
	{
		rtCar.y = nHeight - rtCar.height-1;
	}

	//rtCar.y -= m_uExtentHeight;//抠图中可能会抠出黑边，去掉该黑边。
	// LogNormal("rtCar2=%d,%d,%d,%d\n",rtCar.x,rtCar.y,rtCar.width,rtCar.height);

	return rtCar;
}

//当卡口图片不存储小图时，从大图中抠取小图(对于机动车)
string mvCCenterServerOneDotEight::GetSmallPicFromBigPic(RECORD_PLATE* nPlate,int& nSmallPicSize)
{
	string nBigPicData("");
	string nSmallPicData("");
	unsigned char* nSmallPic = NULL;
	if (nPlate == NULL)
	{
		LogError("pPlate is NULL\n");
		nSmallPicSize = 0;
		return "";
	}
	else
	{
		nBigPicData = GetImageByPath(nPlate->chPicPath);
		CxImage nBigImage;
		//大图解码
		nBigImage.Decode((BYTE*)(nBigPicData.c_str()),nPlate->uPicSize,3);
		UINT32 uPicWidth = 0;
		UINT32 uPicHeight = 0;
		uPicWidth = nBigImage.GetWidth();
		uPicHeight = nBigImage.GetHeight();
		//解码的大图有效
		if (nBigImage.IsValid())
		{
			//为解码好的图片数据准备缓冲空间
			unsigned char* pImageData = NULL;
			UINT32 uImageSize = uPicWidth*uPicHeight*3;
			pImageData = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
			//缓冲空间无效
			if (pImageData == NULL)
			{
				LogError("GetSmallPicFromBigPic pImageData malloc error\n");
			}
			else//缓冲空间有效
			{
				//拷贝解码后的图片数据到缓冲空间
				for (int i = 0; i < uPicHeight; i++)
				{
					if (nBigImage.IsValid())
					{
						memcpy(pImageData + i*uPicWidth*3, nBigImage.GetBits(i), uPicWidth*3);
					}
				}
				IplImage* nBigSnap = NULL;
				nBigSnap = cvCreateImage(cvSize(uPicWidth,uPicHeight),8,3);
				if (nBigSnap != NULL)
				{
					CvRect srcRt;
					srcRt.x = 0;
					srcRt.y = 0;
					srcRt.height = uPicHeight;
					srcRt.width = uPicWidth;

					//拷贝数据到感性区域srcRt
					cvSetImageROI(nBigSnap,srcRt);
					memcpy(nBigSnap->imageData,pImageData,uImageSize);
					cvResetImageROI(nBigSnap);

					CvRect dstRt;
					dstRt = GetCarPos(*nPlate,0);
					if (dstRt.width > 0 && dstRt.height > 0)
					{
						if (dstRt.width + dstRt.x >= nBigSnap->width)
						{
							dstRt.width = nBigSnap->width - dstRt.x;
						}
						if (dstRt.height + dstRt.y >= nBigSnap->height)
						{
							dstRt.height = nBigSnap->height - dstRt.y;
						}
						//为小图分配空间
						nSmallPic = (unsigned char*)calloc(uPicWidth*(uPicHeight/* - m_uExtentHeight*/)/8,sizeof(unsigned char));
						if (nSmallPic != NULL)
						{
							//获取小图及小图的大小
							/*cerr<<"AAAAA plate Number:"<<nPlate->chText<<endl;
							cerr<<"nBigSnap->width:"<<nBigSnap->width<<endl;
							cerr<<"nBigSnap->hight:"<<nBigSnap->height<<endl;
							cerr<<"dstRt.x:"<<dstRt.x<<" dstRt.y:"<<dstRt.y<<endl;
							cerr<<"dstRt.width:"<<dstRt.width<<" dstRt.height:"<<dstRt.height<<endl;*/
							nSmallPicSize = GetSmallImage(nBigSnap,nSmallPic,dstRt);
							if (nSmallPicSize != 0)
							{
								nSmallPicData.append((char*)nSmallPic,nSmallPicSize);
							}
							if (nSmallPic != NULL)
							{
								free(nSmallPic);
								nSmallPic = NULL;
							}
						}
					}
					if (nBigSnap != NULL)
					{
						cvReleaseImage(&nBigSnap);
						nBigSnap = NULL;
					}

				}
				else
				{
					LogError("GetSmallPicFromBigPic nBigSnap malloc error\n");
				}
				if (pImageData != NULL)
				{
					free(pImageData);
					pImageData = NULL;
				}
			}
						
		}
		else //解码的大图无效
		{
			LogError("GetSmallPicFromBigPic nBigImage is InValid\n");
			nSmallPicSize = 0;
			return "";
		}
	}
	return nSmallPicData;
}

//当卡口图片不存储小图时，从大图中抠取小图(对于非机动车，行人等事件)
string mvCCenterServerOneDotEight::GetSmallPicFromBigPic(RECORD_EVENT* nPevent,int& nSmallPicSize)
{
	string nBigPicData("");
	string nSmallPicData("");
	unsigned char* nSmallPic = NULL;
	if (nPevent == NULL)
	{
		LogError("pPlate is NULL\n");
		nSmallPicSize = 0;
		return "";
	}
	else
	{
		nBigPicData = GetImageByPath(nPevent->chPicPath);
		CxImage nBigImage;
		//大图解码
		nBigImage.Decode((BYTE*)(nBigPicData.c_str()),nPevent->uPicSize,3);
		UINT32 uPicWidth = 0;
		UINT32 uPicHeight = 0;
		uPicWidth = nBigImage.GetWidth();
		uPicHeight = nBigImage.GetHeight();
		//解码的大图有效
		if (nBigImage.IsValid())
		{
			//为解码好的图片数据准备缓冲空间
			unsigned char* pImageData = NULL;
			UINT32 uImageSize = uPicWidth*uPicHeight*3;
			pImageData = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
			//缓冲空间无效
			if (pImageData == NULL)
			{
				LogError("GetSmallPicFromBigPic pImageData malloc error\n");
			}
			else//缓冲空间有效
			{
				//拷贝解码后的图片数据到缓冲空间
				for (int i = 0; i < uPicHeight; i++)
				{
					if (nBigImage.IsValid())
					{
						memcpy(pImageData + i*uPicWidth*3, nBigImage.GetBits(i), uPicWidth*3);
					}
				}
				IplImage* nBigSnap = NULL;
				nBigSnap = cvCreateImage(cvSize(uPicWidth,uPicHeight),8,3);
				if (nBigSnap != NULL)
				{
					CvRect srcRt;
					srcRt.x = 0;
					srcRt.y = 0;
					srcRt.height = uPicHeight;
					srcRt.width = uPicWidth;

					//拷贝数据到感性区域srcRt
					cvSetImageROI(nBigSnap,srcRt);
					memcpy(nBigSnap->imageData,pImageData,uImageSize);
					cvResetImageROI(nBigSnap);

					CvRect dstRt;
					dstRt = GetPos(*nPevent,0);
					if (dstRt.width > 0 && dstRt.height > 0)
					{
						//为小图分配空间
						nSmallPic = (unsigned char*)calloc(uPicWidth*(uPicHeight - g_PicFormatInfo.nExtentHeight)/8,sizeof(unsigned char));
						if (nSmallPic != NULL)
						{
							//获取小图及小图的大小
							nSmallPicSize = GetSmallImage(nBigSnap,nSmallPic,dstRt);
							if (nSmallPicSize != 0)
							{
								nSmallPicData.append((char*)nSmallPic,nSmallPicSize);
							}
							if (nSmallPic != NULL)
							{
								free(nSmallPic);
								nSmallPic = NULL;
							}
						}
					}
					if (nBigSnap != NULL)
					{
						cvReleaseImage(&nBigSnap);
						nBigSnap = NULL;
					}

				}
				else
				{
					LogError("GetSmallPicFromBigPic nBigSnap malloc error\n");
				}
				if (pImageData != NULL)
				{
					free(pImageData);
					pImageData = NULL;
				}
			}

		}
		else //解码的大图无效
		{
			LogError("GetSmallPicFromBigPic nBigImage is InValid\n");
			nSmallPicSize = 0;
			return "";
		}
	}
	return nSmallPicData;
}

//string mvCCenterServerOneDotEight::ReturnPicPath(RECORD_PLATE* plate)
//{
//	string strKaKouPath("");
//	if (plate->uViolationType == 0 || plate->uViolationType == 38)
//	{
//		strKaKouPath += "/kk/";
//		string strTime("");
//		string strYMD("");
//		string strHour("");
//		mvGetDateTimeFromSec(plate->uTime, strTime);
//		strYMD = strTime.substr(0,8);
//		strKaKouPath += strYMD;
//		strKaKouPath += "/";
//		strHour.substr(8,2);
//		strKaKouPath += strHour;
//		
//
//	}
//}
//设置发送车牌图片消息体//违法图片的类型暂时人为定为8；
string mvCCenterServerOneDotEight::SetPlatePicAndVideoMsg(RECORD_PLATE& plate,bool bRealTime)//(string path)
{
	//LogNormal("SetPlatePicAndVideoMsg车道号:%d\n",plate.uRoadWayID);
	char buf[256] = {0};
	string picMsg;
	string tmpPicPath;
	int nPicCount;
	int nBigPicType;
	int nSmallPicType;
	int nVioVideoCount;
	string tmpPicData;
	string nStrVioVideoPath;
	char nVioVideoType;

	int nSmallPicSize = 0;
	string nSmallPicData = "";

	int uPicSize1;
	int uPicSize2;
	tmpPicPath = plate.chPicPath;
	
	if (bRealTime)//实时数据消息体
	{
		if (plate.uViolationType == 0 || plate.uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据两张图片（一大图一小图）
		{
			string strKakouPicPath("");
			GetPlatePicPath(plate,strKakouPicPath,0);

			string strKakouSmallPicPath("");
			GetPlatePicPath(plate,strKakouSmallPicPath,1);

			nPicCount = 2;
				
			if(strKakouSmallPicPath.size() > 15)
			{
				if (IsDataDisk())
				{
					strKakouSmallPicPath.erase(0,16);
				}
				else
				{
					strKakouSmallPicPath.erase(0,15);
				}
			}
			if(strKakouPicPath.size() > 15)
			{
				if (IsDataDisk())
				{
					strKakouPicPath.erase(0,16);
				}
				else
				{
					strKakouPicPath.erase(0,15);
				}
			}

			nVioVideoType = ' ';
			nVioVideoCount = 0;
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%d%d0000000%d0000000%59s%59s%d%c",nPicCount,1,2,strKakouSmallPicPath.c_str(),strKakouPicPath.c_str(),nVioVideoCount,nVioVideoType);
				
			picMsg += buf;

		}
		//else if (plate.uViolationType > 0)//违法数据（一张田字形合成图片）
		else
		{
			string strWfhc1("");
			string strWfhc2("");
			string strWfData1("");
			string strWfData2("");
			string strWfhc1Tmp("");
			string strWfhc2Tmp("");
			string strVideoPathTmp("");

				int nUpPicType = 0;
				int nDownPicType = 0;
				nPicCount = 2;
				GetPlatePicPath(plate,strWfhc1,3);
				nUpPicType = 7;
				strWfData1 = GetImageByPath(strWfhc1);
				if(strWfData1.size() <= 0)
				{
					string strPathTemp(plate.chPicPath);
					strWfData1 = GetImageByPath(strPathTemp);
				}

				printf("=======strWfhc1=%s,strWfData1.size()=%d\n",strWfhc1.c_str(),strWfData1.size());
				if(strWfData1.size() <= 0)
				{
					return "";
				}

				GetPlatePicPath(plate,strWfhc2,4);
				nDownPicType = 8;
				strWfData2 = GetImageByPath(strWfhc2);

				memset(buf,0,sizeof(buf));
				sprintf(buf,"%d%d%07d%d%07d",nPicCount,nUpPicType,(int)strWfData1.size(),nDownPicType,(int)strWfData2.size());
				picMsg += buf;
				if(strWfData1.size() > 0)
				{
					picMsg += strWfData1;
				}
				if(strWfData2.size() > 0)
				{
					picMsg += strWfData2;
				}

				memset(buf,0,sizeof(buf));
				
				if(strWfhc1.size() > 16)
				{
					strWfhc1.erase(0,16);
					strWfhc1Tmp = strWfhc1;
				}
				if(strWfhc2.size() > 16)
				{
					strWfhc2.erase(0,16);
					strWfhc2Tmp = strWfhc2;
				}
				string chVideoPath("");
				chVideoPath = plate.chVideoPath;
				//cerr<<"111111111111111chVideoPath:"<<chVideoPath.c_str()<<endl;
				{
					nVioVideoType = ' ';
					nVioVideoCount = 0;
					sprintf(buf,"%59s%59s%d%c",strWfhc1Tmp.c_str(),strWfhc2Tmp.c_str(),nVioVideoCount,nVioVideoType);
				}
				picMsg += buf;
			//}
			
		}	
	}
	else//历史数据补传传消息体（包括强制重传处理）
	{
		if (plate.uViolationType == 0 || plate.uViolationType == DETECT_RESULT_NOCARNUM)//卡口数据两张图片（一大图一小图）
		{
			
			if (m_uForceTransferType == ' ' || m_uForceTransferType == '5' || m_uForceTransferType == '6')//卡口数据强制重传处理
			{
				string strKakouPicPath("");
				GetPlatePicPath(plate,strKakouPicPath,0);

				string strKakouSmallPicPath("");
				GetPlatePicPath(plate,strKakouSmallPicPath,1);

				nPicCount = 2;
					
				if(strKakouSmallPicPath.size() > 15)
				{
					if (IsDataDisk())
					{
						strKakouSmallPicPath.erase(0,16);
					}
					else
					{
						strKakouSmallPicPath.erase(0,15);
					}
				}
				if(strKakouPicPath.size() > 15)
				{
					if (IsDataDisk())
					{
						strKakouPicPath.erase(0,16);
					}
					else
					{
						strKakouPicPath.erase(0,15);
					}
				}

				nVioVideoType = ' ';
				nVioVideoCount = 0;
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%d%d0000000%d0000000%59s%59s%d%c",nPicCount,1,2,strKakouSmallPicPath.c_str(),strKakouPicPath.c_str(),nVioVideoCount,nVioVideoType);
				picMsg += buf;
			}
		}
		//else if (plate.uViolationType > 0)//违法数据（一张田字形合成图片）
		else
		{
			if (m_uForceTransferType == ' ' || m_uForceTransferType == '1' || m_uForceTransferType == '3')//不需要强制重传或者强制重传时需要图片数据
			{
				
					string strWfhc1("");
					string strWfhc2("");
					string strWfData1("");
					string strWfData2("");
					string strWfhc1Tmp("");
					string strWfhc2Tmp("");
					string strVideoPathTmp("");
					int nUpPicType = 0;
					int nDownPicType = 0;
					nPicCount = 2;
					GetPlatePicPath(plate,strWfhc1,3);
					nUpPicType = 7;
					strWfData1 = GetImageByPath(strWfhc1);
					if(strWfData1.size() <= 0)
					{
						string strPathTemp(plate.chPicPath);
						strWfData1 = GetImageByPath(strPathTemp);
					}
					printf("=======strWfhc1=%s,strWfData1.size()=%d\n",strWfhc1.c_str(),strWfData1.size());
					if(strWfData1.size() <= 0)
					{
						return "";
					}


					GetPlatePicPath(plate,strWfhc2,4);
					nDownPicType = 8;
					strWfData2 = GetImageByPath(strWfhc2);

					memset(buf,0,sizeof(buf));
					sprintf(buf,"%d%d%07d%d%07d",nPicCount,nUpPicType,(int)strWfData1.size(),nDownPicType,(int)strWfData2.size());
					picMsg += buf;
					if(strWfData1.size() > 0)
					{
						picMsg += strWfData1;
					}
					if(strWfData2.size() > 0)
					{
						picMsg += strWfData2;
					}

					memset(buf,0,sizeof(buf));

					if(strWfhc1.size() > 16)
					{
						strWfhc1.erase(0,16);
						strWfhc1Tmp = strWfhc1;
					}
					if(strWfhc2.size() > 16)
					{
						strWfhc2.erase(0,16);
						strWfhc2Tmp = strWfhc2;
					}
				
					string chVideoPath("");
					chVideoPath = plate.chVideoPath;
					
					{
						nVioVideoType = ' ';
						nVioVideoCount = 0;
						sprintf(buf,"%59s%59s%d%c",strWfhc1Tmp.c_str(),strWfhc2Tmp.c_str(),nVioVideoCount,nVioVideoType);
					}
					picMsg += buf;
				//}
			}
			else if (m_uForceTransferType == '2' || m_uForceTransferType == '4')//强制重传时不需要图片数据
			{
				//LogNormal("p history kakou 0 pictures\n");
				nPicCount = 0;
				sprintf(buf,"%d",nPicCount);
				picMsg = buf;

				memset(buf,0,sizeof(buf));

				string chVideoPath("");
				chVideoPath = plate.chVideoPath;
				
				{
					nVioVideoType = ' ';
					nVioVideoCount = 0;
					sprintf(buf,"%d%c",nVioVideoCount,nVioVideoType);
				}
				picMsg += buf;
				
			}

		}	
	}
	return picMsg;
}
//设置发送事件图片消息体//违法图片的类型暂时人为定为7
string mvCCenterServerOneDotEight::SetEventPicAndVideoMsg(RECORD_EVENT& pevent,bool bRealTime)//(string path)
{
	//LogNormal("SetEventPicAndVideoMsg车道号:%d\n",pevent.uRoadWayID);
	char buf[256] = {0};
	string picMsg;
	string tmpPicPath;
	int nPicCount;
	int nBigPicType;
	int nSmallPicType;
	int nVioVideoCount;
	string tmpPicData;
	string nStrVioVideoPath;
	char nVioVideoType;
	int nSmallPicSize = 0;
	string nSmallPicData = "";
	tmpPicPath = pevent.chPicPath;
	if (bRealTime)//实时消息体
	{
		if (pevent.uCode == 0 || pevent.uCode == DETECT_RESULT_EVENT_APPEAR || pevent.uCode == DETECT_RESULT_EVENT_PERSON_APPEAR)//卡口两张图片（一大图一小图）
		{
			//LogNormal("realTime kakou 2 pictures\n");
			nPicCount = 2;
			if(pevent.uType == PERSON_TYPE)
			{
				nBigPicType = 6;
				nSmallPicType = 5;
			}
			if (pevent.uType == OTHER_TYPE)
			{
				nBigPicType = 4;
				nSmallPicType = 3;
			}
			else
			{
				nBigPicType = 2;
				nSmallPicType = 1;
			}

			tmpPicData = GetImageByPath(tmpPicPath);

			if (g_PicFormatInfo.nSmallPic == 1)
			{
				nSmallPicSize = tmpPicData.size() - pevent.uPicSize;
			}
			else
			{
				nSmallPicData = GetSmallPicFromBigPic(&pevent,nSmallPicSize);
			}

			sprintf(buf,"%d%d%07d%d%07d",nPicCount,nBigPicType,pevent.uPicSize,nSmallPicType,nSmallPicSize);
			/*LogNormal("E kakou buf:%s\n",buf);*/
			picMsg = buf;
			picMsg += tmpPicData;
			picMsg += nSmallPicData;

			memset(buf,0,sizeof(buf));
			/*LogNormal("E kakou picMsg:%s\n",picMsg.c_str())*/;


			nStrVioVideoPath = pevent.chVideoPath; 
			if (nStrVioVideoPath.size() > 0)
			{
				nVioVideoType = '2';
				nVioVideoCount = 1;
				sprintf(buf,"%59s%59s%d%65s%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,pevent.chVideoPath,nVioVideoType);
			}
			else
			{
				nVioVideoType = ' ';
				nVioVideoCount = 0;
				sprintf(buf,"%59s%59s%d%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,nVioVideoType);
			}
			picMsg += buf;
		}
		//if (pevent.uCode > 0)//违法（一张合成图片）
		else
		{
			string nVtsMsg = SeparateEventVtsPicture(pevent);

			picMsg = nVtsMsg;

			memset(buf,0,sizeof(buf));


			nStrVioVideoPath = pevent.chVideoPath; 
			if (nStrVioVideoPath.size() > 0)
			{
				nVioVideoType = '2';
				nVioVideoCount = 1;
				sprintf(buf,"%59s%59s%d%65s%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,pevent.chVideoPath,nVioVideoType);
			}
			else
			{
				nVioVideoType = ' ';
				nVioVideoCount = 0;
				sprintf(buf,"%59s%59s%d%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,nVioVideoType);
			}
			picMsg += buf;
		}
	}
	else//历史数据消息体（包括补传和强制重传）
	{
		if (pevent.uCode == 0 || pevent.uCode == 10 || pevent.uCode == DETECT_RESULT_EVENT_PERSON_APPEAR)//卡口两张图片（一大图一小图）
		{
			if (m_uForceTransferType == ' ')//不需要强制重传
			{
				/*LogNormal("11history kakou 2 pictures\n");*/
				nPicCount = 2;
				if(pevent.uType==PERSON_TYPE)
				{
					nBigPicType = 6;
					nSmallPicType = 5;
				}
				if (pevent.uType == OTHER_TYPE)
				{
					nBigPicType = 4;
					nSmallPicType = 3;
				}
				else
				{
					nBigPicType = 2;
					nSmallPicType = 1;
				}

				tmpPicData = GetImageByPath(tmpPicPath);
				if (g_PicFormatInfo.nSmallPic == 1)
				{
					nSmallPicSize = tmpPicData.size() - pevent.uPicSize;
				}
				else
				{
					nSmallPicData = GetSmallPicFromBigPic(&pevent,nSmallPicSize);
					/*if (nSmallPicData.size() > 0)
					{
						LogNormal("444444Gain SmallPicData Success\n");
					}*/
				}
				sprintf(buf,"%d%d%07d%d%07d",nPicCount,nBigPicType,pevent.uPicSize,nSmallPicType,nSmallPicSize);
				picMsg = buf;
				picMsg += tmpPicData;
				picMsg += nSmallPicData;

				memset(buf,0,sizeof(buf));


				nStrVioVideoPath = pevent.chVideoPath; 
				if (nStrVioVideoPath.size() > 0)
				{
					nVioVideoType = '2';
					nVioVideoCount = 1;
					sprintf(buf,"%59s%59s%d%65s%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,pevent.chVideoPath,nVioVideoType);
				}
				else
				{
					nVioVideoType = ' ';
					nVioVideoCount = 0;
					sprintf(buf,"%59s%59s%d%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,nVioVideoType);
				}
				picMsg += buf;
			}
			if (m_uForceTransferType == '5' || m_uForceTransferType == '6')//强制重传不需要图片数据
			{
				/*LogNormal("11history kakou 0 pictures\n");*/
				nPicCount = 0;
				sprintf(buf,"%d",nPicCount);
				picMsg = buf;

				memset(buf,0,sizeof(buf));



				nStrVioVideoPath = pevent.chVideoPath; 
				if (nStrVioVideoPath.size() > 0)
				{
					nVioVideoType = '2';
					nVioVideoCount = 1;
					sprintf(buf,"%d%65s%c",nVioVideoCount,pevent.chVideoPath,nVioVideoType);
				}
				else
				{
					nVioVideoType = ' ';
					nVioVideoCount = 0;
					sprintf(buf,"%d%c",nVioVideoCount,nVioVideoType);
				}
				picMsg += buf;
			}


		}
		//if(pevent.uCode > 0)//违法（一张合成图片）
		else
		{
			if (m_uForceTransferType == ' ' || m_uForceTransferType == '1' || m_uForceTransferType == '3')//不需要强制重传或者重传需要图片数据
			{
				string nVtsMsg = SeparateEventVtsPicture(pevent);

				picMsg = nVtsMsg;

				memset(buf,0,sizeof(buf));


				nStrVioVideoPath = pevent.chVideoPath; 
				if (nStrVioVideoPath.size() > 0)
				{
					nVioVideoType = '2';
					nVioVideoCount = 1;
					sprintf(buf,"%59s%59s%d%65s%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,pevent.chVideoPath,nVioVideoType);
				}
				else
				{
					nVioVideoType = ' ';
					nVioVideoCount = 0;
					sprintf(buf,"%59s%59s%d%c",pevent.chPicPath,pevent.chPicPath,nVioVideoCount,nVioVideoType);
				}
				picMsg += buf;
			}
			else if (m_uForceTransferType == '2' || m_uForceTransferType == '4')//强制重传时不需要图片数据
			{
				//LogNormal("11history weifa 0 pictures\n");
				nPicCount = 0;
				sprintf(buf,"%d",nPicCount);
				picMsg = buf;

				memset(buf,0,sizeof(buf));



				nStrVioVideoPath = pevent.chVideoPath; 
				if (nStrVioVideoPath.size() > 0)
				{
					nVioVideoType = '2';
					nVioVideoCount = 1;
					sprintf(buf,"%d%65s%c",nVioVideoCount,pevent.chVideoPath,nVioVideoType);
				}
				else
				{
					nVioVideoType = ' ';
					nVioVideoCount = 0;
					sprintf(buf,"%d%c",nVioVideoCount,nVioVideoType);
				}
				picMsg += buf;
			}

		}
	}

	//LogNormal("333333picMsg.size:%d\n",picMsg.size());
		
	return picMsg;
}

//获取超速违法代码
UINT mvCCenterServerOneDotEight::GetPlateVioCode(RECORD_PLATE& plate)
{
	UINT ViolationType = 0;
	if (plate.uViolationType == 0 || plate.uViolationType == 38)
	{
		ViolationType = 0;
	}
	else if (plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST)
	{
		//LogNormal("GoFast111111111\n");
		mapChanMaxSpeedStr::iterator it = m_uChanMaxSpeedStrMap.find(plate.uChannelID);
		if (it != m_uChanMaxSpeedStrMap.end())
		{	
			int nSpeedMaxTmp = GetMaxSpeedStrByRoadId( (it->second), plate.uType, plate.uRoadWayID);
			//if (tmp != (it->second).end())
			if(nSpeedMaxTmp > 0)
			{
				if ((plate.uSpeed >= nSpeedMaxTmp*0.3) && (plate.uSpeed < nSpeedMaxTmp*0.5))//超速30%（含），不足50%
				{
					ViolationType = 1;
				}
				else if (plate.uSpeed > nSpeedMaxTmp*0.5)//超过规定时速50% 
				{
					ViolationType = 1;
				}
				else if ((plate.uSpeed >= nSpeedMaxTmp*0.2) && (plate.uSpeed < nSpeedMaxTmp*0.3))//超速20%（含），不足30%（不含）
				{
					ViolationType = 1;
				}
				else if ((plate.uSpeed >= nSpeedMaxTmp*0.1) && (plate.uSpeed < nSpeedMaxTmp*0.2))//超速10%（含）不足20%（不含）的
				{
					ViolationType = 1;
				}//还有待填
				else
				{
					ViolationType = 1;
				}
			}
			else
			{
				LogNormal("GetPlateVioCode 没有找到车道号：%d\n",plate.uRoadWayID);
				//ViolationType = plate.uViolationType;
			}
		}
		else
		{
			LogNormal("GetPlateVioCode 没有找到通道：%d\n",plate.uChannelID);
		}
		
		ViolationType = 1;
		
	}
	else if (plate.uViolationType == DETECT_RESULT_EVENT_GO_SLOW)
	{
		ViolationType = 1;
	}


	else if (plate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION)//车辆逆行 //逆向行驶
	{
		ViolationType = 42;
	}
	else if (plate.uViolationType == DETECT_RESULT_OBV_TAKE_UP_BUSWAY || plate.uViolationType == 55)//占用公交道 //违反规定使用专用车道
	{
		ViolationType = 23;
	}
	else if (plate.uViolationType == 23 || plate.uViolationType == 24 || plate.uViolationType == 25 || plate.uViolationType == 18 || plate.uViolationType == 59 || \
		 plate.uViolationType == 56 || plate.uViolationType == 57)
	{
		ViolationType = 11;
	}
	else if (plate.uViolationType == 16)
	{
		ViolationType = 4;
	}
	else if (plate.uViolationType == 17)
	{
		ViolationType = 99;
	}
	else if (plate.uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
	{
		ViolationType = 2;
	}
	else if (plate.uViolationType == DETECT_RESULT_YELLOW_CAR)
	{
		ViolationType = 12;
	}
	else if (plate.uViolationType == DETECT_RESULT_YELLOW_CRC)
	{
		ViolationType = 13;
	}
	else if (plate.uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME ||\
		plate.uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD)
	{
		ViolationType = 23;
	}
	else //我们的违章类型暂时不能完全满足电科中心端1.8版的违法代码。不能转换的违法代码暂定为10
	{
		//LogNormal("1111111111111111我们的违法类型：%d",plate.uViolationType);
		cerr<<""<<"1111111111111111我们的违法类型："<<plate.uViolationType<<endl;
		ViolationType = 10;//plate.uViolationType;
	}
	return ViolationType;
}


//获取超速违法代码
UINT mvCCenterServerOneDotEight::GetEventVioCode(RECORD_EVENT& pevent)
{
	UINT ViolationType = 0;
	if (pevent.uCode == 0)
	{
		ViolationType = 0;
	}
	else if (pevent.uCode == DETECT_RESULT_EVENT_GO_FAST)
	{
		mapChanMaxSpeedStr::iterator it = m_uChanMaxSpeedStrMap.find(pevent.uChannelID);
		if (it != m_uChanMaxSpeedStrMap.end())
		{			
			
			int nSpeedMaxTmp = GetMaxSpeedStrByRoadId( (it->second), pevent.uType, pevent.uRoadWayID);
			//if (tmp != (it->second).end())
			if(nSpeedMaxTmp > 0)
			{
				if ((pevent.uSpeed >= nSpeedMaxTmp*0.3) && (pevent.uSpeed < nSpeedMaxTmp*0.5))//超速30%（含），不足50%
				{
					ViolationType = 1;
				}
				else if (pevent.uSpeed > nSpeedMaxTmp*0.5)//超过规定时速50% 
				{
					ViolationType = 1;
				}
				else if ((pevent.uSpeed >= nSpeedMaxTmp*0.2) && (pevent.uSpeed < nSpeedMaxTmp*0.3))//超速20%（含），不足30%（不含）
				{
					ViolationType = 1;
				}
				else if ((pevent.uSpeed >= nSpeedMaxTmp*0.1) && (pevent.uSpeed < nSpeedMaxTmp*0.2))//超速10%（含）不足20%（不含）的
				{
					ViolationType = 1;
				}//还有待填
				else
				{
					ViolationType = 1;
				}
			}
			else
			{
				LogNormal("GetEventVioCode 没有找到车道号:%d\n",pevent.uRoadWayID);
			}
		}
		else
		{
			LogNormal("GetEventVioCode 没有找到对应的通道号:%d\n",pevent.uChannelID);
		}

	}

	else if (pevent.uCode == DETECT_RESULT_EVENT_GO_SLOW)
	{
		ViolationType = 1;
	}

	else if (pevent.uCode == DETECT_RESULT_RETROGRADE_MOTION)//车辆逆行 //逆向行驶
	{
		ViolationType = 42;
	}
	else if (pevent.uCode == DETECT_RESULT_OBV_TAKE_UP_BUSWAY || pevent.uCode == 55)//占用公交道 //违反规定使用专用车道
	{
		ViolationType = 23;
	}
	else if (pevent.uType == PERSON_TYPE && pevent.uCode == DETECT_RESULT_EVENT_PERSON_APPEAR)
	{
		ViolationType = 0;
	}
	else if (pevent.uType == OTHER_TYPE && (pevent.uCode == 14 || pevent.uCode == 10))
	{
		ViolationType = 0;
	}
	else if (pevent.uCode == 23 || pevent.uCode == 24 || pevent.uCode == 25 || pevent.uCode == 18 || pevent.uCode == 59 || \
		pevent.uCode == 56 || pevent.uCode == 57)
	{
		ViolationType = 11;
	}
	else if (pevent.uCode == DETECT_RESULT_EVENT_DISTANCE_FAST)
	{
		ViolationType = 2;
	}
	else if (pevent.uCode == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME)
	{
		ViolationType = 23;
	}
	else //我们的违章类型暂时不能完全满足电科中心端1.8版的违法代码。不能转换的违法代码暂定为10
	{
		ViolationType = 10;//pevent.uCode;
	}
	return ViolationType;
}

//根据时间戳获取时分数字
void mvCCenterServerOneDotEight::ToGetDateHourMinuteSec(long Time,int &day,int& hour,int& minute,int &second)
{
	std::string strTime = GetTime(Time,2);
	string strDate;
	string strHour;
	string strMinute;
	string strSecond;

	strDate.append(strTime.c_str()+6,2);
	day = atoi(strDate.c_str());
	
	strHour.append(strTime.c_str()+8,2);
	hour = atoi(strHour.c_str());

	strMinute.append(strTime.c_str()+10,2);
	minute = atoi(strMinute.c_str());

	strSecond.append(strTime.c_str()+12,2);
	second = atoi(strSecond.c_str());
}
//设置事件消息体中，违法超速设备的限速值
void mvCCenterServerOneDotEight::SetEventLimitSpeed(RECORD_EVENT& pevent,UINT& uLimitSpeed)
{
	
	if (pevent.uCode == DETECT_RESULT_EVENT_GO_FAST)
	{
		//uLimitSpeed = g_dkLimitSpeed;
		mapChanMaxSpeedStr::iterator it = m_uChanMaxSpeedStrMap.find(pevent.uChannelID);
		if (it != m_uChanMaxSpeedStrMap.end())
		{					
			int uLimitSpeed = GetMaxSpeedStrByRoadId( (it->second), pevent.uType, pevent.uRoadWayID);
		}
		else
		{
			LogNormal("SetEventLimitSpeed 没有找到对应的通道号：%d\n",pevent.uChannelID);
		}
	}
	else//不是违法设备限速值为0
	{
		uLimitSpeed = 0;
	}
	//if (pevent.uCode == DETECT_RESULT_RED_LIGHT_VIOLATION)
	//{
	//	uRedTime = GetRedLightTime();//暂时人为定为2秒
	//}
	//else
	//{
	//	uRedTime = 0;
	//}
}
//设置车牌消息体中，违法超速设备的限速值
void mvCCenterServerOneDotEight::SetPlateLimitSpeed(RECORD_PLATE& plate,UINT& uLimitSpeed)
{
	if (plate.uViolationType == DETECT_RESULT_EVENT_GO_FAST || plate.uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
	{
		mapChanMaxSpeedStr::iterator it = m_uChanMaxSpeedStrMap.find(plate.uChannelID);
		if (it != m_uChanMaxSpeedStrMap.end())
		{	
			int uLimitSpeed = GetMaxSpeedStrByRoadId( (it->second), plate.uType, plate.uRoadWayID);
		}
		else
		{
			LogNormal("SetPlateLimitSpeed 没有找到对应的通道号：%d\n",plate.uChannelID);
		}
	}
	else//不是违法设备限速值为0
	{
		uLimitSpeed = 0;
	}
	///*if (plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
	//{
	//	uRedTime = GetRedLightTime();
	//}
	//else
	//{
	//	uRedTime = 0;
	//}*/
}

//违章图片组合方式为2*2，并且叠加小图，将该图片分开成一小一大，两大两种合成图片
string mvCCenterServerOneDotEight::SeparatePlateVtsPicture(RECORD_PLATE& plate)
{
	string strPicData = GetImageByPath(plate.chPicPath);
	string nVtsPicMsg;
	string nStrTmp;
	int nPicCount = 2;
	
	if(strPicData.size()>0)
	{
		char buf[128] = {0};
		sprintf(buf,"%d",nPicCount);
		nStrTmp = buf; //图片数目2张
		//需要解码jpg图像
		CxImage image;
		CvRect rect;
		image.Decode((BYTE*)(strPicData.c_str()), strPicData.size(), 3); //违章合成大图（2*2 叠加小图）解码
		rect.x = 0;
		rect.y = 0;
		rect.height = plate.uPicHeight;
		rect.width = plate.uPicWidth;

		CxImage image1;
		CvRect rect1;
		rect1.x = 0;
		rect1.y = 0;
		rect1.height = plate.uPicHeight/2;
		rect1.width = plate.uPicWidth;

		CxImage image2;
		CvRect rect2;
		rect2.x = 0;
		rect2.y = plate.uPicHeight/2;
		rect2.height = plate.uPicHeight/2;
		rect2.width = plate.uPicWidth;

		RECT myRect1,myRect2;

		//田字形合成图片上面两张图片的矩形区域
		myRect1.left = rect1.x;
		myRect1.top = rect1.y;
		myRect1.right = rect1.x + rect1.width;
		myRect1.bottom = rect1.y + rect1.height;

		//田字形合成图片下面面两张图片的矩形区域
		myRect2.left = rect2.x;
		myRect2.top = rect2.y;
		myRect2.right = rect2.x + rect2.width;
		myRect2.bottom = rect2.y + rect2.height;

		if(image.IsValid()&&image.GetSize()>0)
		{
			image.Flip();//翻转180
			image.Crop(myRect1,&image1);//将解码翻转后的图像抠出田字形的上半部分到image1
			image1.Flip();//image1翻转180
			image.Crop(myRect2,&image2);//将解码翻转后的图像抠出田字形的下半部分到image2
			image2.Flip();//image2翻转180

			UINT32 uPicWidth1 = image1.GetWidth();
			UINT32 uPicHeight1 = image1.GetHeight();

			UINT32 uPicWidth2 = image2.GetWidth();
			UINT32 uPicHeight2 = image2.GetHeight();

			

			UINT32 uImageSize1 = uPicWidth1*uPicHeight1*3;
			UINT32 uImageSize2 = uPicWidth2*uPicHeight2*3;
			unsigned char* poutImageData1 = (unsigned char*)calloc(uImageSize1,sizeof(unsigned char));
			unsigned char* pjpgImag1 = (unsigned char*)calloc(uImageSize1,sizeof(unsigned char));

			int i = 0;
			for(i = 0;i < uPicHeight1;i++)//将image1的像素内容拷贝到poutImageData1
			{
				if(image1.IsValid())
					memcpy(poutImageData1+i*uPicWidth1*3,image1.GetBits(i),uPicWidth1*3);
			}

			int srcstep = 0;
			CxImage image;
			image.IppEncode(poutImageData1,uPicWidth1,uPicHeight1,3,&srcstep,pjpgImag1,g_PicFormatInfo.nJpgQuality);//将poutImageData1的内容编码到pjpgImag1
			int nUpPicType = 7;
			memset(buf,0,128);
			sprintf(buf,"%d%07d",nUpPicType,srcstep);
			nStrTmp += buf;
			nVtsPicMsg.append((char*)pjpgImag1,srcstep);//jpg图片内容
		
			if(poutImageData1)
			{
				free(poutImageData1);
				poutImageData1 = NULL;
			}

			if(pjpgImag1)
			{
				free(pjpgImag1);
				pjpgImag1 = NULL;
			}

			unsigned char* poutImageData2 = (unsigned char*)calloc(uImageSize2,sizeof(unsigned char));
			unsigned char* pjpgImage2 = (unsigned char*)calloc(uImageSize2,sizeof(unsigned char));

			for(i = 0;i < uPicHeight2;i++)
			{
				if(image2.IsValid())
					memcpy(poutImageData2+i*uPicWidth2*3,image2.GetBits(i),uPicWidth2*3);
			}
			srcstep = 0;
			CxImage imageTmp;
			imageTmp.IppEncode(poutImageData2,uPicWidth2,uPicHeight2,3,&srcstep,pjpgImage2,g_PicFormatInfo.nJpgQuality);
			int nDownPicType = 8;
			memset(buf,0,128);
			sprintf(buf,"%d%07d",nDownPicType,srcstep);
			nStrTmp += buf;
			nVtsPicMsg.append((char*)pjpgImage2,srcstep);
			if(poutImageData2)
			{
				free(poutImageData2);
				poutImageData2 = NULL;
			}


			if(pjpgImage2)
			{
				free(pjpgImage2);
				pjpgImage2 = NULL;
			}

			nVtsPicMsg.insert(0,nStrTmp.c_str(),nStrTmp.size());
		}
	}

	return nVtsPicMsg;
}

//违章图片组合方式为2*2，并且叠加小图，将该图片分开成一小一大，两大两种合成图片
string mvCCenterServerOneDotEight::SeparateEventVtsPicture(RECORD_EVENT& pevent)
{
	string strPicData = GetImageByPath(pevent.chPicPath);
	string nVtsPicMsg;
	string nStrTmp;
	int nPicCount = 2;

	if(strPicData.size()>0)
	{
		char buf[128] = {0};
		sprintf(buf,"%d",nPicCount);
		nStrTmp = buf; //图片数目2张
		//需要解码jpg图像
		CxImage image;
		CvRect rect;
		image.Decode((BYTE*)(strPicData.c_str()), strPicData.size(), 3); //违章合成大图（2*2 叠加小图）解码
		rect.x = 0;
		rect.y = 0;
		rect.height = pevent.uPicHeight;
		rect.width = pevent.uPicWidth;

		CxImage image1;
		CvRect rect1;
		rect1.x = 0;
		rect1.y = 0;
		rect1.height = pevent.uPicHeight/2;
		rect1.width = pevent.uPicWidth;

		CxImage image2;
		CvRect rect2;
		rect2.x = 0;
		rect2.y = pevent.uPicHeight/2;
		rect2.height = pevent.uPicHeight/2;
		rect2.width = pevent.uPicWidth;

		RECT myRect1,myRect2;

		//田字形合成图片上面两张图片的矩形区域
		myRect1.left = rect1.x;
		myRect1.top = rect1.y;
		myRect1.right = rect1.x + rect1.width;
		myRect1.bottom = rect1.y + rect1.height;

		//田字形合成图片下面面两张图片的矩形区域
		myRect2.left = rect2.x;
		myRect2.top = rect2.y;
		myRect2.right = rect2.x + rect2.width;
		myRect2.bottom = rect2.y + rect2.height;

		if(image.IsValid()&&image.GetSize()>0)
		{
			image.Flip();//翻转180
			image.Crop(myRect1,&image1);//将解码翻转后的图像抠出田字形的上半部分到image1
			image1.Flip();//image1翻转180
			image.Crop(myRect2,&image2);//将解码翻转后的图像抠出田字形的下半部分到image2
			image2.Flip();//image2翻转180

			UINT32 uPicWidth1 = image1.GetWidth();
			UINT32 uPicHeight1 = image1.GetHeight();

			UINT32 uPicWidth2 = image2.GetWidth();
			UINT32 uPicHeight2 = image2.GetHeight();



			UINT32 uImageSize1 = uPicWidth1*uPicHeight1*3;
			UINT32 uImageSize2 = uPicWidth2*uPicHeight2*3;
			unsigned char* poutImageData1 = (unsigned char*)calloc(uImageSize1,sizeof(unsigned char));
			unsigned char* pjpgImag1 = (unsigned char*)calloc(uImageSize1,sizeof(unsigned char));

			int i = 0;
			for(i = 0;i < uPicHeight1;i++)//将image1的像素内容拷贝到poutImageData1
			{
				if(image1.IsValid())
					memcpy(poutImageData1+i*uPicWidth1*3,image1.GetBits(i),uPicWidth1*3);
			}

			int srcstep = 0;
			CxImage image;
			image.IppEncode(poutImageData1,uPicWidth1,uPicHeight1,3,&srcstep,pjpgImag1,g_PicFormatInfo.nJpgQuality);//将poutImageData1的内容编码到pjpgImag1
			int nUpPicType = 7;
			memset(buf,0,128);
			sprintf(buf,"%d%07d",nUpPicType,srcstep);
			nStrTmp += buf;
			nVtsPicMsg.append((char*)pjpgImag1,srcstep);//jpg图片内容

			if(poutImageData1)
			{
				free(poutImageData1);
				poutImageData1 = NULL;
			}

			if(pjpgImag1)
			{
				free(pjpgImag1);
				pjpgImag1 = NULL;
			}

			unsigned char* poutImageData2 = (unsigned char*)calloc(uImageSize2,sizeof(unsigned char));
			unsigned char* pjpgImage2 = (unsigned char*)calloc(uImageSize2,sizeof(unsigned char));

			for(i = 0;i < uPicHeight2;i++)
			{
				if(image2.IsValid())
					memcpy(poutImageData2+i*uPicWidth2*3,image2.GetBits(i),uPicWidth2*3);
			}
			srcstep = 0;
			CxImage imageTmp;
			imageTmp.IppEncode(poutImageData2,uPicWidth2,uPicHeight2,3,&srcstep,pjpgImage2,g_PicFormatInfo.nJpgQuality);
			int nDownPicType = 8;
			memset(buf,0,128);
			sprintf(buf,"%d%07d",nDownPicType,srcstep);
			nStrTmp += buf;
			nVtsPicMsg.append((char*)pjpgImage2,srcstep);
			if(poutImageData2)
			{
				free(poutImageData2);
				poutImageData2 = NULL;
			}


			if(pjpgImage2)
			{
				free(pjpgImage2);
				pjpgImage2 = NULL;
			}

			nVtsPicMsg.insert(0,nStrTmp.c_str(),nStrTmp.size());
		}
	}

	return nVtsPicMsg;
}

//获取车道总数
void mvCCenterServerOneDotEight::SetRoadCount()
{
	char szBuff[256] = {0};
	string strTableName = "CHAN_INFO";
	int uCount = 0;
	int i = 1;
	sprintf(szBuff, "select count(CHAN_ID) from %s;",strTableName.c_str());
	MysqlQuery count = g_skpDB.execQuery(string(szBuff));
	if (!count.eof())
	{
		uCount = count.getUnIntFileds(0);
	}
	count.finalize();
	LogNormal("通道数：%d\n",uCount);
	m_uCameraCount = uCount;
	CXmlParaUtil xml;
	for (i = 1; i <= uCount; i++)
	{
		//map<UINT32,UINT32> nMaxSpeedMap;
		mapMaxSpeedStr mapMaxSpeedStrTmp;
		mapMaxSpeedStrTmp.clear();
		if(g_nDetectMode == 2)
		{			
			GetMapMaxSpeedStrByChan(mapMaxSpeedStrTmp, i);
		}
		else
		{
			xml.GetMaxSpeedStr(mapMaxSpeedStrTmp, i);
		}
		
		LogNormal("1111通道%d车道数:%d\n",i,mapMaxSpeedStrTmp.size());
		m_uRoadCount += mapMaxSpeedStrTmp.size();
		m_uChanMaxSpeedStrMap.insert(mapChanMaxSpeedStr::value_type(i,mapMaxSpeedStrTmp));		
	}
}

//获取车道状态
string mvCCenterServerOneDotEight::GetRoadStatus()
{
	char szBuff[256] = {0};
	string strTableName = "CHAN_INFO";
	int uCount = 0;
	int i = 1;
	string nRoadStatusMsg;
	nRoadStatusMsg = "";
	CXmlParaUtil xml;
	
	sprintf(szBuff, "select count(CHAN_ID) from %s;",strTableName.c_str());
	MysqlQuery count = g_skpDB.execQuery(string(szBuff));
	if (!count.eof())
	{
		uCount = count.getUnIntFileds(0);
	}
	count.finalize();

	////for (i = 1; i <= uCount; i++) //获得车道总数
	////{
	////	map<UINT32,UINT32> nMaxSpeedMap;
	////	nMaxSpeedMap.clear();
	////	xml.GetMaxSpeed(nMaxSpeedMap,i);
	////	m_uChanMaxSpeedMap.insert(m_uChanMaxSpeedMap::value_type(i,nMaxSpeedMap));//通道对应的限速map
	////	m_uRoadCount += nMaxSpeedMap.size();//车道总数
	////}

	for (i = 1; i <= uCount; i++)//<ChannelID,<RoadWayId,LoopStatus>>
	{
		mapMaxSpeedStr mapMaxSpeedStrTmp;
		mapMaxSpeedStrTmp.clear();

		mapChanIdLoopStatus::iterator it =  m_vChanIdLoopStatus.find(i);
		if (it != m_vChanIdLoopStatus.end())
		{
			if ((it->second).size() == 0)//没有线圈状态车道状态暂定为‘y’
			{
				CXmlParaUtil xml;
				//map<UINT32,UINT32> mapMaxSpeedTmp;
				
				int nChanRoadCount = 0;
				int j = 0;
				//mapMaxSpeedTmp.clear();
				mapMaxSpeedStrTmp.clear();
				if(g_nDetectMode == 2)
				{
					GetMapMaxSpeedStrByChan(mapMaxSpeedStrTmp, i);
				}
				else
				{
					//xml.GetMaxSpeed(mapMaxSpeedTmp,i);
					xml.GetMaxSpeedStr(mapMaxSpeedStrTmp, i);
				}
				
				nChanRoadCount = mapMaxSpeedStrTmp.size();
				for (j = 0; j < nChanRoadCount; j++)
				{
					nRoadStatusMsg += 'y';
				}
				
			}
			else
			{
				mapLoopStatus::iterator tmp = (it->second).begin();
				while (tmp != (it->second).end())
				{
					if (tmp->second < 1)//线圈状态小于1，该车道异常
					{
						nRoadStatusMsg += 'n';
					}
					else
					{
						nRoadStatusMsg += 'y';
					}
					tmp++;
				}
			}
		}
		else//没开线圈检测，m_vChanIdLoopStatus为空，该通道对应的车道状态暂时都定为'y';
		{
			CXmlParaUtil xml;
			int nChanRoadCount = 0;
			int j = 0;
			mapMaxSpeedStrTmp.clear();
			if(g_nDetectMode == 2)
			{				
				GetMapMaxSpeedStrByChan(mapMaxSpeedStrTmp, i);
			}
			else
			{
				xml.GetMaxSpeedStr(mapMaxSpeedStrTmp,i);
			}
			
			nChanRoadCount = mapMaxSpeedStrTmp.size();
			//LogNormal("2222nChanroadCount:%d\n",nChanRoadCount);
			for (j = 0; j < nChanRoadCount; j++)
			{
				nRoadStatusMsg += 'y';
			}
		}

		//m_uChanMaxSpeedStrMap.insert(mapChanMaxSpeedStr::value_type(i,mapMaxSpeedStrTmp));//通道对应的限速map
	}
	return nRoadStatusMsg;
}


//获取相机的状态
string mvCCenterServerOneDotEight::GetCameraStatus()
{
	int i = 0;
	int nCameraId = 0;
	string nCameraStatusMsg = "";
	for (i = 1; i <= m_uCameraCount; i++)
	{
		nCameraId = g_skpDB.GetCameraID(i);
		if (g_skpDB.GetCameraState(nCameraId) == 0)
		{
			nCameraStatusMsg += 'y';
		}
		else
		{
			nCameraStatusMsg += 'n';
		}
	}

	return nCameraStatusMsg;
	
}


//红灯随即时间 2 - 10s;
int mvCCenterServerOneDotEight::GetRedLightTime()
{
	srand( (unsigned)time( NULL ) );
	int nDetectTime = 2 + (int)(8.0*rand()/(RAND_MAX+1.0));
	return nDetectTime;
}

//设置图片宽高比例
void mvCCenterServerOneDotEight::SetRatio(float nRatio)
{
	m_uRatio = nRatio;
}

//获取图片路径
void mvCCenterServerOneDotEight::GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath,int nType)
{
	string strDataPath;
	strDataPath = "/home/road/dzjc/";
	if (IsDataDisk())
	{
		strDataPath = "/detectdata/dzjc/";
	}
	
	if(access(strDataPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
	}
	
	if(nType == 0)
	strDataPath += "kk/";
	else if(nType == 1)
	strDataPath += "kk/";
	else if(nType == 2)
	strDataPath += "lx/";
	else if(nType == 3)
	strDataPath += "wf/";
	else if(nType == 4)
	strDataPath += "wf/";

	if(access(strDataPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
	}

	UINT32 uTimestamp = plate.uTime;
	//时间
	std::string strTime = GetTime(uTimestamp,2);
	//日期
	std::string strDate = GetTime(uTimestamp,4);
	//小时
	string strHour = GetTime(uTimestamp,8);

	string strFtpDataDir = strDataPath + strDate;

	if(access(strFtpDataDir.c_str(),0) != 0) //目录不存在
	{
		mkdir(strFtpDataDir.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
	}

	strFtpDataDir += '/';
	string strFtpHourDir = strFtpDataDir + strHour;

	if(access(strFtpHourDir.c_str(),0) != 0) //目录不存在
	{
		mkdir(strFtpHourDir.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
	}
	

	char buf[256] = {0};

	if(nType == 0)
	sprintf(buf, "%s/%s%02d%s%03d_qj.jpg",strFtpHourDir.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uMiTime);
	else if(nType == 1)
	sprintf(buf, "%s/%s%02d%s%03d_tx.jpg",strFtpHourDir.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uMiTime);
	else if(nType == 2)
	sprintf(buf, "%s/%s%02d%s%03d_lx.avi",strFtpHourDir.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uMiTime);
	else if(nType == 3)
	sprintf(buf, "%s/%s%02d%s%03d_hc1.jpg",strFtpHourDir.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uMiTime);
	else if(nType == 4)
	sprintf(buf, "%s/%s%02d%s%03d_hc2.jpg",strFtpHourDir.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uMiTime);

	strPicPath = buf;
}

//是否为空目录 
bool mvCCenterServerOneDotEight::IsEmptyDir(string& strPath)
{
	cerr<<"1111111in IsEmptyDir :"<<strPath.c_str()<<endl;
	bool nEmptyFlag = true;
	DIR *dir;
	struct dirent *pdirent = NULL;
	if ((dir = opendir(strPath.c_str())) == NULL)
	{
		cerr<<"open dir "<<strPath.c_str()<<"error"<<endl;
		return true;
	}
	while ((pdirent=readdir(dir)) != NULL)
	{
		
		if( strcmp(".", pdirent->d_name) == 0 || strcmp("..", pdirent->d_name) == 0 )
		{
			cerr<<"1111111 in IsEmptyDir:"<<pdirent->d_name<<endl;
			continue;
		}
		else
		{
			cerr<<"in IsEmptyDir pdirent->d_name:"<<pdirent->d_name<<endl;
			nEmptyFlag = false;
			break;
		}
		
	}
	closedir(dir);
	cerr<<"in IsEmptyDir nFlag: "<<nEmptyFlag<<endl;
	return nEmptyFlag;
	
}

//删除最早文件及空目录
void mvCCenterServerOneDotEight::DelEarlistFileAndDir(string& strPath,int nType) 
{
	if (nType == 0) //YYYYMMDD 目录
	{
		if (strPath.size() > 0)
		{
			cerr<<"00000000000011 strPath:"<<strPath.c_str()<<endl;
			string strMiPath(""); //最早创建的路径
			string strTmp("");
			int nYYYYCount = 0; //YYYY 年
			int nMMCount = 0;   //MM 月
			int nDDCount = 0;	//DD 日
			int nCount = 0;
			int nMiCount = 3000*365 + 12*30 + 30; //存储年月日转换成日数的最小值

			DIR* dir=NULL;
			struct dirent* ptr=NULL;

			dir = opendir(strPath.c_str());
			//cerr<<"00000000000022"<<endl;
			if (dir == NULL)
			{
				cerr<<"0000000000033 opendir "<<strPath.c_str()<<"error"<<endl;
				return;
			}
			
			if(dir)
			{
				while((ptr=readdir(dir))!=NULL)
				{
					if((strcmp(".",ptr->d_name)!=0)&&
						(strcmp("..",ptr->d_name)!=0))
					{
						std::string strPathName = strPath;
						strPathName += ptr->d_name;

						//cerr<<"0000000000000strPathName.c_str():"<<strPathName.c_str()<<endl;

						if (strPathName.size() > 0)
						{

							strTmp = strPathName.substr(19,8); //hh
							//cerr<<"00000444"<<endl;
							nYYYYCount = atoi((strTmp.substr(0,4)).c_str());
							//cerr<<"0000555"<<endl;
							nMMCount = atoi((strTmp.substr(4,2)).c_str());
							//cerr<<"0000666"<<endl;
							nDDCount = atoi((strTmp.substr(6,2)).c_str());
							//cerr<<"0000777"<<endl;
							nCount = nYYYYCount*365 + nMMCount*30 + nDDCount;
							if(nCount <= nMiCount)
							{
								nMiCount = nCount;
								strMiPath = strPathName;
							}
						}
						else
						{
							cerr<<"BBBBBBBBin GetEarlistPath strPathName is NULL"<<endl;
						}

					}
				}
				closedir(dir);
				if (IsEmptyDir(strMiPath))
				{
					rmdir(strMiPath.c_str());//删除空目录
					cerr<<"000000000000delete Dir:"<<strMiPath.c_str()<<endl;
					return;
				}
				if (strMiPath.size() > 0)
				{
					cerr<<"000888strMiPath:"<<strMiPath.c_str()<<endl;
					DelEarlistFileAndDir(strMiPath,1);
				}
			}
		}
	}
	else if(nType == 1) //hh 目录
	{
		if (strPath.size() > 0)
		{
			string strMiPath(""); //最早创建的路径
			string strTmp("");
			int hhCount = 0;
			int nMihhCount = 24;
			
			DIR* dir=NULL;
			struct dirent* ptr=NULL;

			dir = opendir(strPath.c_str());

			if(dir)
			{
				while((ptr=readdir(dir))!=NULL)
				{
					if((strcmp(".",ptr->d_name)!=0)&&
						(strcmp("..",ptr->d_name)!=0))
					{
						std::string strPathName = strPath + "/";
						strPathName += ptr->d_name;

						//cerr<<"1111111111111strPathName.c_str():"<<strPathName.c_str()<<endl;

						if (strPathName.size() > 0)
						{
							
							strTmp = strPathName.substr(28,2); //hh
							hhCount = atoi(strTmp.c_str());
							if(hhCount <= nMihhCount)
							{
								nMihhCount = hhCount;
								strMiPath = strPathName;
							}
						}
						else
						{
							cerr<<"AAAAAAAAAAAAAin GetEarlistPath strPathName is NULL"<<endl;
						}

					}
				}
				closedir(dir);
				if (IsEmptyDir(strMiPath))
				{
					rmdir(strMiPath.c_str());//删除空目录
					cerr<<"111111111111111111delete Dir:"<<strMiPath.c_str()<<endl;
					return;
				}
				if (strMiPath.size() > 0)
				{
					cerr<<"111111111strMipath:"<<strMiPath.c_str()<<endl;
					DelEarlistFileAndDir(strMiPath,2);
				}
			}
		}
	}
	else if (nType == 2) //图片ID一级的文件名
	{
		string strMyPath("");
		strMyPath = strPath.substr(16,2);
		if (strcmp(strMyPath.c_str(),"kk") == 0)
		{
			if(strPath.size() > 0)
			{
				DIR* dir=NULL;
				struct dirent* ptr=NULL;

				dir = opendir(strPath.c_str());

				int nCount = 0; //mi*60*1000 + ss*1000 + zzz; 分秒最终转换为毫秒
				int nMiniCount = 3600000; //最小值 即 最早创建的时间
				string strMiPath(""); //最早创建的文件的路径 
				string strTmp("");//misszzz;
				int miCount = 0;
				int ssCount = 0;
				int zzzCount = 0;
				

				if(dir)
				{
					while((ptr=readdir(dir))!=NULL)
					{
						if((strcmp(".",ptr->d_name)!=0)&&
							(strcmp("..",ptr->d_name)!=0))
						{
							std::string strPathName = strPath + "/";
							strPathName += ptr->d_name;

							//cerr<<"22222222222222strPathName.c_str():"<<strPathName.c_str()<<endl;;
							
							if (strPathName.size() > 0)
							{
								strTmp = strPathName.substr(strPathName.size()-14,7);
								//cerr<<"222222strTmp:"<<strTmp.c_str()<<endl;
								miCount = atoi((strTmp.substr(0,2)).c_str());
								ssCount = atoi((strTmp.substr(2,2)).c_str());
								zzzCount = atoi((strTmp.substr(4,3)).c_str());
								nCount = miCount*60*1000 + ssCount*1000 + zzzCount; 
								if (nCount <= nMiniCount)
								{
									nMiniCount = nCount;
									strMiPath = strPathName;
								}

							}
							else
							{
								cerr<<"2222222222in GetEarlistPath strPathName is NULL"<<endl;
							}
							
						}
					}
					closedir(dir);
					cerr<<"222222222strMiPath:"<<strMiPath.c_str()<<endl;
					if (strMiPath.size() > 0)
					{
						if (strMiPath.substr(strMiPath.size()-6,1).find("q") == 0)
						{
							if (access(strMiPath.c_str(),F_OK) == 0)
							{
								remove(strMiPath.c_str());
								g_skpDB.DianKeDeleteOldRecord(strMiPath,0);
								cerr<<"1111111111111delete quanjing Picture:"<<strMiPath.c_str()<<endl;
							}
							strTmp.clear();
							strTmp = strMiPath.substr(0,strMiPath.size()-7);
							strTmp += "_tx.jpg";
							if (access(strTmp.c_str(),F_OK) == 0)
							{
								remove(strTmp.c_str());
								cerr<<"111111111111111delete tiexie Picture"<<strTmp.c_str()<<endl;
							}
						}
						else if (strMiPath.substr(strMiPath.size()-6,1).find("t") == 0)
						{
							if (access(strMiPath.c_str(),F_OK) == 0)
							{
								remove(strMiPath.c_str());
								cerr<<"222222222delete texie Picture:"<<strMiPath.c_str()<<endl;
							}
							strTmp.clear();
							strTmp = strMiPath.substr(0,strMiPath.size()-7);
							strTmp += "_qj.jpg";
							if (access(strTmp.c_str(),F_OK) == 0)
							{
								remove(strTmp.c_str());
								g_skpDB.DianKeDeleteOldRecord(strTmp,0);
								cerr<<"222222222delete quanjing Picture:"<<strTmp.c_str()<<endl;
							}
						}
					}
					else
					{
						LogError("Delete Kakou Picture error: strMiPath is NULL\n");
						cerr<<"Delete Kakou Picture error: strMiPath is NULL"<<endl;
					}
					
				}
			}
		}
		else if (strcmp(strMyPath.c_str(),"wf") == 0)
		{
			if(strPath.size() > 0)
			{
				DIR* dir=NULL;
				struct dirent* ptr=NULL;

				dir = opendir(strPath.c_str());

				int nCount = 0; //mi*60*1000 + ss*1000 + zzz; 分秒最终转换为毫秒
				int nMiniCount = 3600000; //最小值 即 最早创建的时间
				string strMiPath(""); //最早创建的文件的路径 
				string strTmp("");//misszzz;
				int miCount = 0;
				int ssCount = 0;
				int zzzCount = 0;


				if(dir)
				{
					while((ptr=readdir(dir))!=NULL)
					{
						if((strcmp(".",ptr->d_name)!=0)&&
							(strcmp("..",ptr->d_name)!=0))
						{
							std::string strPathName = strPath + "/";
							strPathName += ptr->d_name;

							//cerr<<"22222222222222WeiFa strPathName.c_str():"<<strPathName.c_str()<<endl;;

							if (strPathName.size() > 0)
							{
								strTmp = strPathName.substr(strPathName.size()-15,7);
								//cerr<<"222222strTmp:"<<strTmp.c_str()<<endl;
								miCount = atoi((strTmp.substr(0,2)).c_str());
								ssCount = atoi((strTmp.substr(2,2)).c_str());
								zzzCount = atoi((strTmp.substr(4,3)).c_str());
								nCount = miCount*60*1000 + ssCount*1000 + zzzCount; 
								if (nCount <= nMiniCount)
								{
									nMiniCount = nCount;
									strMiPath = strPathName;
								}

							}
							else
							{
								cerr<<"2222222222in GetEarlistPath strPathName is NULL"<<endl;
							}

						}
					}
					closedir(dir);
					cerr<<"222222222Weifa strMiPath:"<<strMiPath.c_str()<<endl;
					if (strMiPath.size() > 0)
					{
						if (strMiPath.substr(strMiPath.size()-5,1).find("1") == 0)
						{
							if (access(strMiPath.c_str(),F_OK) == 0)
							{
								remove(strMiPath.c_str());
								g_skpDB.DianKeDeleteOldRecord(strMiPath,0);
								cerr<<"1111111111111delete wfhc Picture1:"<<strMiPath.c_str()<<endl;
							}
							strTmp.clear();
							strTmp = strMiPath.substr(0,strMiPath.size()-8);
							strTmp += "_hc2.jpg";
							if (access(strTmp.c_str(),F_OK) == 0)
							{
								remove(strTmp.c_str());
								cerr<<"111111111111111delete wfhc Picture2"<<strTmp.c_str()<<endl;
							}
						}
						else if (strMiPath.substr(strMiPath.size()-5,1).find("2") == 0)
						{
							if (access(strMiPath.c_str(),F_OK) == 0)
							{
								remove(strMiPath.c_str());
								cerr<<"222222222delete wfhc Picture2:"<<strMiPath.c_str()<<endl;
							}
							strTmp.clear();
							strTmp = strMiPath.substr(0,strMiPath.size()-8);
							strTmp += "_hc1.jpg";
							if (access(strTmp.c_str(),F_OK) == 0)
							{
								remove(strTmp.c_str());
								g_skpDB.DianKeDeleteOldRecord(strTmp,0);
								cerr<<"222222222delete wfhc Picture1:"<<strTmp.c_str()<<endl;
							}
						}
					}
					else
					{
						LogError("Delete weifa Picture error: strMiPath is NULL\n");
						cerr<<"Delete weifa Picture error: strMiPath is NULL"<<endl;
						return;
					}

				}
			}
		}
		else if (strcmp(strMyPath.c_str(),"lx") == 0)
		{
			if(strPath.size() > 0)
			{
				DIR* dir=NULL;
				struct dirent* ptr=NULL;

				dir = opendir(strPath.c_str());

				int nCount = 0; //mi*60*1000 + ss*1000 + zzz; 分秒最终转换为毫秒
				int nMiniCount = 3600000; //最小值 即 最早创建的时间
				string strMiPath(""); //最早创建的文件的路径 
				string strTmp("");//misszzz;
				int miCount = 0;
				int ssCount = 0;
				int zzzCount = 0;


				if(dir)
				{
					while((ptr=readdir(dir))!=NULL)
					{
						if((strcmp(".",ptr->d_name)!=0)&&
							(strcmp("..",ptr->d_name)!=0))
						{
							std::string strPathName = strPath + "/";
							strPathName += ptr->d_name;

							//cerr<<"22222222222222LxstrPathName.c_str():"<<strPathName.c_str()<<endl;;

							if (strPathName.size() > 0)
							{
								strTmp = strPathName.substr(strPathName.size()-14,7);
								//cerr<<"222222strTmp:"<<strTmp.c_str()<<endl;
								miCount = atoi((strTmp.substr(0,2)).c_str());
								ssCount = atoi((strTmp.substr(2,2)).c_str());
								zzzCount = atoi((strTmp.substr(4,3)).c_str());
								nCount = miCount*60*1000 + ssCount*1000 + zzzCount; 
								if (nCount <= nMiniCount)
								{
									nMiniCount = nCount;
									strMiPath = strPathName;
								}

							}
							else
							{
								cerr<<"2222222222in GetEarlistPath luxiang strPathName is NULL"<<endl;
							}

						}
					}
					closedir(dir);
					cerr<<"222222222luxiang strMiPath:"<<strMiPath.c_str()<<endl;
					if (strMiPath.size() > 0)
					{
						if (strMiPath.substr(strMiPath.size()-6,1).find("l") == 0)
						{
							if (access(strMiPath.c_str(),F_OK) == 0)
							{
								remove(strMiPath.c_str());
								g_skpDB.DianKeDeleteOldRecord(strMiPath,1);
								cerr<<"1111111111111delete luxiang:"<<strMiPath.c_str()<<endl;
							}
						}
					}
					else
					{
						LogError("Delete luxiang error: strMiPath is NULL");
						cerr<<"Delete luxiang error: strMiPath is NULL"<<endl;
						return;
					}
				}
			}
		}
	}
}

//卡口记录磁盘管理
void mvCCenterServerOneDotEight::ManagerDisk()
{
	string strKaKouPath = "/home/road/dzjc/kk/";
	string strWfPath = "/home/road/dzjc/wf/";
	string strLxPath = "/home/road/dzjc/lx/";
	if (IsDataDisk())
	{
		strKaKouPath = "/detectdata/dzjc/kk/";
		strWfPath = "/detectdata/dzjc/wf/";
		strLxPath = "/detectdata/dzjc/lx/";
	}
	
	int nCount = 0;
	int nCount1 = 0;
	while (!g_bEndThread)
	{
		if( g_nFtpServer == 1)
		{
				if(g_sysInfo.fDisk >= 91)
				{
					DelMinDir();
				}
		}
		sleep(5);
	}
}

//重连
void mvCCenterServerOneDotEight::LinkTest()
{
	time_t startTime;
	time_t nowTime;
	time(&startTime);

	while (!g_bEndThread)
	{
		mvConnOrLinkTest();

		sleep(10);

		time(&nowTime);
		int offTime = (nowTime-startTime);
		if(offTime >= 60)
		{
			time(&startTime);
			DownloadFile();
		}
	}
}

void mvCCenterServerOneDotEight::FtpFownLoad()
{
	time_t startTime;
	time_t nowTime;
	time(&startTime);

	while (!g_bEndThread)
	{
		//string strHour = GetTime(GetTimeStamp(),8);
		//if (strHour.compare("03") == 0)
		time(&nowTime);
		int offTime = (nowTime-startTime);
		if(offTime >= 10)
		{
			time(&startTime);
			if (DownloadFile())
			{
				m_bFtpUpdate = true;
			}
		}
		
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
}

//删除最小的小时目录
void mvCCenterServerOneDotEight::DelMinHourDir(string& strPath)
{
	cerr<<"DelMinHourDir: %s"<<strPath.c_str()<<endl;
	if (strPath.size() > 0)
	{
			DIR* dir=NULL;
			struct dirent* ptr=NULL;

			dir = opendir(strPath.c_str());
			//cerr<<"00000000000022"<<endl;
			if(dir != NULL)
			{
				while((ptr=readdir(dir))!=NULL)
				{
					if((strcmp(".",ptr->d_name)!=0)&&
						(strcmp("..",ptr->d_name)!=0))
					{
						std::string strPathName = strPath + "/";
						strPathName += ptr->d_name;

						if (strPathName.size() > 0)
						{
							remove(strPathName.c_str());
							usleep(100*1000);
						}
					}
				}
				closedir(dir);

			    rmdir(strPath.c_str());//删除空目录
			}
	}
}

//删除最小的目录
void mvCCenterServerOneDotEight::DelMinDir()
{
	string strKaKouPath = "/home/road/dzjc/kk";
	string strWfPath = "/home/road/dzjc/wf";
	string strLxPath = "/home/road/dzjc/lx";
	if (IsDataDisk())
	{
		strKaKouPath = "/detectdata/dzjc/kk";
		strWfPath = "/detectdata/dzjc/wf";
		strLxPath = "/detectdata/dzjc/lx";
	}
	
	char buf[256] = {0};
	int nDaykk = GetMinPath(strKaKouPath);
	sprintf(buf,"%s/%08d",strKaKouPath.c_str(),nDaykk);
	string strDaykk(buf);
	int nHourkk = GetMinPath(strDaykk);
	int nMinkk = nDaykk*100+nHourkk;

	int nDaywf = GetMinPath(strWfPath);
	sprintf(buf,"%s/%08d",strWfPath.c_str(),nDaywf);
	string strDaywf(buf);
	int nHourwf = GetMinPath(strDaywf);
	int nMinwf = nDaywf*100+nHourwf;

	int nDaylx = GetMinPath(strLxPath);
	sprintf(buf,"%s/%08d",strLxPath.c_str(),nDaylx);
	string strDaylx(buf);
	int nHourlx = GetMinPath(strDaylx);
	int nMinlx = nDaylx*100+nHourlx;
	
	int nMinID = nMinkk;

	if(nDaywf != 0)
	{
		if(nMinID >= nMinwf)
		{
			nMinID = nMinwf;
		}
	}
	
	if(nDaylx != 0)
	{
		if(nMinID >= nMinlx)
		{
			nMinID = nMinlx;
		}
	}
	
	cerr<<"nMinkk:"<<nMinkk<<endl;
	cerr<<"nMinwf:"<<nMinwf<<endl;
	cerr<<"nMinlx:"<<nMinlx<<endl;
	
	cerr<<"nMinID:"<<nMinID<<endl;

	if(nMinID == nMinkk)
	{
		sprintf(buf,"%4d-%02d-%02d %02d:59:59",nDaykk/10000,(nDaykk%10000)/100,nDaykk%100,nHourkk);
		string strTime(buf);

		cerr<<"DeleteRecordByTime: %s"<<strTime.c_str()<<endl;

		g_skpDB.DeleteRecordByTime(strTime,0);

		sprintf(buf,"%s/%02d",strDaykk.c_str(),nHourkk);
		string strHourkk(buf);
		DelMinHourDir(strHourkk);

		if (IsEmptyDir(strDaykk))
		{
			rmdir(strDaykk.c_str());//删除空目录
		}
	}
	else if(nMinID == nMinwf)
	{
		sprintf(buf,"%4d-%02d-%02d %02d:59:59",nDaywf/10000,(nDaywf%10000)/100,nDaywf%100,nHourwf);
		string strTime(buf);
		cerr<<"DeleteRecordByTime: %s"<<strTime.c_str()<<endl;
		g_skpDB.DeleteRecordByTime(strTime,0);

		sprintf(buf,"%s/%02d",strDaywf.c_str(),nHourwf);
		string strHourwf(buf);
		DelMinHourDir(strHourwf);

		if (IsEmptyDir(strDaywf))
		{
			rmdir(strDaywf.c_str());//删除空目录
		}
	}
	else if(nMinID == nMinlx)
	{
		sprintf(buf,"%4d-%02d-%02d %02d:59:59",nDaylx/10000,(nDaylx%10000)/100,nDaylx%100,nHourlx);
		string strTime(buf);
		cerr<<"DeleteRecordByTime: %s"<<strTime.c_str()<<endl;

		g_skpDB.DeleteRecordByTime(strTime,0);
		g_skpDB.DeleteRecordByTime(strTime,1);
		
		printf("strDaylx.c_str()=%s,nHourlx=%d\n",strDaylx.c_str(),nHourlx);

		sprintf(buf,"%s/%02d",strDaylx.c_str(),nHourlx);

		string strHourlx(buf);
		printf("strHourlx.c_str()=%s\n",strHourlx.c_str());


		DelMinHourDir(strHourlx);

		if (IsEmptyDir(strDaylx))
		{
			rmdir(strDaylx.c_str());//删除空目录
		}
	}

}

//获取最小的目录编号
int mvCCenterServerOneDotEight::GetMinPath(string& strPath)
{
	int nMinID = 100000000;

	bool bFindMinID = false;

	if (strPath.size() > 0)
	{
			DIR* dir=NULL;
			struct dirent* ptr=NULL;

			dir = opendir(strPath.c_str());
			//cerr<<"00000000000022"<<endl;
			if (dir == NULL)
			{
				return 0;
			}
			if(dir)
			{
				while((ptr=readdir(dir))!=NULL)
				{
					if((strcmp(".",ptr->d_name)!=0)&&
						(strcmp("..",ptr->d_name)!=0))
					{
						std::string strPathName(ptr->d_name);

						if (strPathName.size() > 0)
						{
							int nID = atoi(strPathName.c_str());

							if(nID <= nMinID)
							{
								nMinID = nID;
								bFindMinID = true;
							}
						}
					}
				}
				closedir(dir);
			}
	}

	cerr<<"GetMinPath nMinID:"<<nMinID<<endl;

	if(!bFindMinID)
	{
		nMinID = 0;
	}

	return nMinID;
}

//获取卡口图像的宽高比
float mvCCenterServerOneDotEight::GetRatio()
{
	return m_uRatio;
}


void mvCCenterServerOneDotEight::CreateWCDMAThread()
{
	pthread_t m_hStreamId;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //设置线程分离
	if(pthread_create(&m_hStreamId, NULL, DealWCDMAFunc, this) != 0)
	{
		pthread_attr_destroy(&attr);
		printf("Start Thread Failed. \n");
		return ;
	}
	pthread_attr_destroy(&attr);

	usleep(10000); //开启线程后暂停10毫秒
	return ;
}
void * mvCCenterServerOneDotEight::DealWCDMAFunc(void * lpParam)
{
	mvCCenterServerOneDotEight *pCenterServer = (mvCCenterServerOneDotEight*)lpParam;
	if(pCenterServer)
	{
		pCenterServer->DealWCDMA();
	}
	return NULL;
}
void * mvCCenterServerOneDotEight::DealWCDMA()
{
	int m_nPingTime = 0;
	int m_nNowTime = 0;
	int m_nOpenCount = 1; //程序检查次数 当检查360次(即1个小时)失败 就重启机器
	int nConnect = 1;		//程序拨号次数
	bool bSuccess = false;//拨号成功标志
	while(!m_bEndWCDMAThread)
	{
		if(m_nOpenCount >= 180)
		{
			m_nOpenCount = 1;
//#ifdef REBOOTDEVICE
		LogNormal("3G拨号一直没成功，重启机器！\n");
		system("reboot -i");
//#endif
			
		}
		usleep(10*1000);
		/*string strHost = GetIpAddress("ppp0",1);
		if (strHost.size() > 0)
		{
			LogNormal("3G已经存在，不需要再次拨号！\n");
		}*/
		//else
		//{
		//	//600秒内没有拨号成功，重新拨号
		//	if( abs(GetTimeT() - m_nPingTime) > 600)
		//	{
		//		LogNormal("准备3G拨号......\n");
		//		system("sh /etc/ppp/Close3G.sh");
		//		while(1)
		//		{
		//			if(access("/var/run/ppp0.pid",0) == 0)
		//			{
		//				usleep(10*1000);
		//				continue;
		//			}
		//			else
		//			{
		//				break;
		//			}
		//		}
		//		sleep(5);
		//		LogNormal("正在3G拨号[%d]......\n",nConnect++);
		//		system("sh /etc/ppp/Open3G.sh");
		//		sleep(30);
		//		m_nPingTime = GetTimeT();
		//	}
		//}

		//10秒钟判断一次
		//printf("%d,%d \n",GetTimeT(),m_nNowTime);
		if( (GetTimeT()%10 == 0)&& (m_nNowTime != GetTimeT()) )
		{
			m_nNowTime = GetTimeT();
			//char pbody[64] = {0};
			//sprintf(pbody,"ping %s -c 4",g_strControlServerHost.c_str());
			//printf("center server ip:%s \n",pbody);
			string strHost = GetIpAddress("ppp0",1);
			if (strHost.size() > 0)
			//if(system(pbody) == 0)
			{
				if (bSuccess == false)
				{
					LogNormal("3G拨号成功[%d]\n",nConnect++);
				}
				bSuccess = true;
				m_nPingTime = GetTimeT();
				m_nOpenCount = 1;
				//nConnect = 1;
				//printf("m_nOpenCount=%d \n",m_nOpenCount);
			}
			else
			{
				LogNormal("3G拨号失败，检查次数[%d]\n",m_nOpenCount);
				m_nOpenCount++;
				bSuccess = false;
				//printf("m_nOpenCount=%d \n",m_nOpenCount);
			}
		}
	}

	return NULL;
}

int mvCCenterServerOneDotEight::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}



