#include "Common.h"
#include "CommonHeader.h"
#include "CenterServer.h"
#include "XmlParaUtil.h"
#include <sys/statvfs.h>
#include "BrandSubSection.h"
#include "CarLabel.h"

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

bool g_bCenterLink = false;
string g_strCSMsgVer = "1.7";
mvCCenterServer g_CenterServer;
pthread_t g_uRecvCSMsgThreadId = 0;

//#define CS_STUPID_LOG

#ifdef CS_STUPID_LOG
void CSSLog(const char *pLog)
{
    FILE *pCSSLog = fopen("cs_stupid.log", "a");
    if (pCSSLog != NULL)
    {
        fprintf(pCSSLog, pLog);
        fflush(pCSSLog);
        fclose(pCSSLog);
    }
}
#endif  //CS_STUPID_LOG

static const unsigned int crcTable[256] =
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
void *RecvCSMsgThread(void *pArg)
{
    while (!g_bEndThread)
    {
        g_CenterServer.mvRecvCenterServerMsg();

        usleep(100);
    }

    LogError("接收中心端消息线程退出\r\n");
    g_uRecvCSMsgThreadId = (pthread_t)0;
    pthread_exit((void *)0);
}

//记录发送线程
void* ThreadCSRecorderResult(void* pArg)
{
	//取类指针
	mvCCenterServer* pCenterServer = (mvCCenterServer*)pArg;
	if(pCenterServer == NULL) return pArg;

	//处理一条数据
	pCenterServer->DealResult();
    pthread_exit((void *)0);
	return pArg;
}

/*
* 函数介绍：构造函数
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCCenterServer::mvCCenterServer()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvCCenterServer()\n");
#endif
    m_nControlSocket = 0;
    m_nCenterSocket = 0;
    m_nCSLinkCount = 0;
    m_nConnectTime = 0;
    m_uCSLinkTime = 0;
    m_bHistoryRep = false;
    m_strDetectorId = "01234567890abcde";
    m_uHistoryRepTime = 0;

    char szBuff[2] = {0};
    sprintf(szBuff, "%c", 0x2);
    m_strHeader = szBuff;

    memset(szBuff, 0, 2);
    sprintf(szBuff, "%c", 0x3);
    m_strEnd = szBuff;

    m_nAllDisk = 64 * 1024;

#ifdef CS_FILE_LOG
    m_pCSRecvLog = fopen("cs_recv.log", "w");
    m_pCSSendLog = fopen("cs_send.log", "w");
    m_pCSConnLog = fopen("cs_conn.log", "w");
#endif

    pthread_mutex_init(&m_mutexMsg, NULL);
    pthread_mutex_init(&m_Result_Mutex,NULL);

    m_ChannelResultList.clear();
    m_nThreadId = 0;
	m_vLoopStatus.clear();
}

/*
* 函数介绍：析构函数
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
mvCCenterServer::~mvCCenterServer()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::~mvCCenterServer()\n");
#endif
    if (!m_mapCSMsg.empty())
    {
        m_mapCSMsg.clear();
    }

#ifdef CS_FILE_LOG
    if (m_pCSRecvLog != NULL)
    {
        fclose(m_pCSRecvLog);
        m_pCSRecvLog = NULL;
    }
    if (m_pCSSendLog != NULL)
    {
        fclose(m_pCSSendLog);
        m_pCSSendLog = NULL;
    }
    if (m_pCSConnLog != NULL)
    {
        fclose(m_pCSConnLog);
        m_pCSConnLog = NULL;
    }
#endif

    pthread_mutex_destroy(&m_mutexMsg);
    pthread_mutex_destroy(&m_Result_Mutex);
}

//初始化
bool mvCCenterServer::Init()
{
    //线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);

	//启动检测结果发送线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	int nret=pthread_create(&m_nThreadId,&attr,ThreadCSRecorderResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);
}

//释放
bool mvCCenterServer::UnInit()
{
    mvOnEndWork();
    //停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}

    m_ChannelResultList.clear();
	return true;
}

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void mvCCenterServer::mvConnOrLinkTest()
{
#ifdef CS_FILE_LOG
    fprintf(m_pCSConnLog, "\nmvConnOrLinkTest, host=%s\tport=%d\n",
                            g_strControlServerHost.c_str(), g_nControlServerPort);
                    fflush(m_pCSConnLog);
#endif
    if (!g_bEndThread)
    {
        if (!g_bCenterLink)
        {
            {
                if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
                {
#ifdef CS_FILE_LOG
                    fprintf(m_pCSConnLog, "\n中心路由服务器连接参数异常! host=%s\tport=%d\n",
                            g_strControlServerHost.c_str(), g_nControlServerPort);
                    fflush(m_pCSConnLog);
#endif
                    return;
                }
                else
                {
                    if (!mvPrepareSocket(m_nControlSocket))
                    {
#ifdef CS_FILE_LOG
                        fprintf(m_pCSConnLog, "\n准备连接中心路由服务器套接字失败!\n");
                        fflush(m_pCSConnLog);
#endif
                        return;
                    }

                    if (!mvWaitConnect(m_nControlSocket, g_strControlServerHost, g_nControlServerPort))
                    {
#ifdef CS_FILE_LOG
                        fprintf(m_pCSConnLog, "\n尝试连接中心路由服务器失败，将直接连接原中心数据服务器!\n");
                        fflush(m_pCSConnLog);
#endif
                        if(m_nConnectTime >5)
                        {
                            m_nConnectTime = 0;
                        }
                        if(m_nConnectTime++==5)//尝试连接中心路由服务器5次失败后连接上次分配的中心数据服务器
                        {
                            LogError("连接中心路由服务器5次失败后直接连接原中心数据服务器");
                            if (mvConnCSAndRecvMsg())
                            {
                                g_bCenterLink = true;
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
#ifdef CS_FILE_LOG
                        fprintf(m_pCSConnLog, "\n连接中心路由服务器成功，正在请求中心数据服务器IP和PORT...\n");
                        fflush(m_pCSConnLog);
#endif
                        LogNormal("连接中心路由服务器成功，正在请求中心数据服务器IP和PORT...\n");
                        if (mvRequestAndConnCS())
                        {
#ifdef CS_FILE_LOG
                            fprintf(m_pCSConnLog, "\n请求中心数据服务器IP和PORT成功，并连接成功!\n");
                            fflush(m_pCSConnLog);
#endif
                            LogNormal("请求中心数据服务器IP和PORT成功!\n");
                            g_bCenterLink = true;
                            m_uCSLinkTime = GetTimeStamp();
                            m_nCSLinkCount = 0;
                        }
                        else
                        {
#ifdef CS_FILE_LOG
                            fprintf(m_pCSConnLog, "\n请求中心数据服务器IP和PORT失败，或请求成功后连接失败!\n");
                            fflush(m_pCSConnLog);
#endif
                            LogError("请求中心数据服务器IP和PORT失败，或请求成功后连接失败");
                            //printf("\n请求中心数据服务器IP和PORT失败!\n");
                            mvCloseSocket(m_nControlSocket);

                            if (mvConnCSAndRecvMsg())//请求失败后直接连接
                            {
                                g_bCenterLink = true;
                                LogNormal("请求失败后，直接连接原先的中心数据服务器成功!\n");
                                m_uCSLinkTime = GetTimeStamp();
                                m_nCSLinkCount = 0;
                            }
                            else
                            {
                                LogError("请求失败后，直接连接原先的中心数据服务器失败\n");
                            }
                        }
                    }
                }
            }
        }
        else
        {
#ifdef CS_FILE_LOG
            fprintf(m_pCSConnLog, "中心数据服务器连接正常，ip=%s,port=%d,socket=%d\n",
                    g_strCenterServerHost.c_str(), g_nCenterServerPort, m_nCenterSocket);
            fflush(m_pCSConnLog);
#endif
            unsigned int uTimeStamp = GetTimeStamp();
            //printf("\n中心数据服务器连接正常，ip=%s,port=%d,socket=%d\n", g_strCenterServerHost.c_str(), g_nCenterServerPort, m_nCenterSocket);
            if ( (uTimeStamp >= m_uCSLinkTime+120) && (m_uCSLinkTime >0) )
            {
                if (!mvSendLinkTest())
                {
                    LogError("发送心跳包失败\n");
                    return;
                }
                else
                {
                    m_uCSLinkTime = uTimeStamp;
                    LogNormal("发送心跳包成功!\n");
                }

                if (m_nCSLinkCount++ > SRIP_LINK_MAX)
                {
                    mvCloseSocket(m_nCenterSocket);
                    g_bCenterLink = false;
                    m_nCSLinkCount = 0;
                    LogError("长时间未收到心跳包，连接断开\n");
                    return;
                }
            }
            //检测设备状态
            mvCheckDeviceStatus(0);

            //处理历史记录
            if(g_nSendHistoryRecord == 1)
            mvCheckFreeAndDealHistoryRecord();
        }
    }
}

/*
* 函数介绍：压入一条消息
* 输入参数：strMsg-要压入的消息
* 输出参数：无
* 返回值 ：无
*/
void mvCCenterServer::mvPushOneMsg(string strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvPushOneMsg()\n");
#endif
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

#ifdef CS_FILE_LOG
        fprintf(m_pCSRecvLog, "Error!#####strDetectorId=%s, m_strDetectorId=%s\n",
                strDetectorId.c_str(), m_strDetectorId.c_str());
        fflush(m_pCSRecvLog);
#endif
        printf("Error!#####strDetectorId=%s, m_strDetectorId=%s\n", strDetectorId.c_str(), m_strDetectorId.c_str());
        return ;
    }

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
bool mvCCenterServer::mvPopOneMsg(string &strCode, string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvPopOneMsg()\n");
#endif
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
void mvCCenterServer::mvSetDetectorId(const char *pDetectorId)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSetDetectorId()\n");
#endif
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
bool mvCCenterServer::mvOnDealOneMsg(const char *pCode, const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnDealOneMsg()\n");
#endif
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
bool mvCCenterServer::mvRebMsgAndSend(int& nSocket,const char *pCode, const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvRebMsgAndSend()\n");
#endif
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

    if (!mvSendMsgToSocket(nSocket, strFullMsg))
    {
#ifdef CS_FILE_LOG
        fprintf(m_pCSSendLog, "\nerror发送消息失败，套接字=%d，消息大小=%d(Bytes)，CRC32校验值=%s，消息内容为：\n%s\n\n",
                nSocket, (int)strFullMsg.size(), strFullMsg.c_str()+strFullMsg.size()-9, strFullMsg.c_str());
        fflush(m_pCSSendLog);
#endif
        mvCloseSocket(nSocket);
        g_bCenterLink = false;
        LogError("发送消息失败，连接断开\n");
        return false;
    }

#ifdef CS_FILE_LOG
    fprintf(m_pCSSendLog, "\nsuccess发送消息成功，套接字=%d，消息大小=%d(Bytes)，CRC32校验值=%s，消息内容为：\n%s\n\n",
            nSocket, (int)strFullMsg.size(), strFullMsg.c_str()+strFullMsg.size()-9, strFullMsg.c_str());
    fflush(m_pCSSendLog);
#endif

    return true;
}

/*
* 函数介绍：发送路由请求并连接到中心
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvRequestAndConnCS()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvRequestAndConnCS()\n");
#endif
    //send connect request;
    //recv connect reply;
    if (!mvSendRouterReqAndRecv())
    {
        return false;
    }

    //connect to center server and start to receive cs's msgs;
    if (!mvConnCSAndRecvMsg())
    {
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
bool mvCCenterServer::mvConnCSAndRecvMsg()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvConnCSAndRecvMsg()\n");
#endif
    //connect to center server;
    if (!mvConnectToCS())
    {
        return false;
    }

    //start to receive cs's msgs;
    if (!mvStartRecvThread())
    {
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
bool mvCCenterServer::mvConnectToCS()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvConnectToCS()\n");
#endif
    //connect to center server and set socket's option.
    if (g_strCenterServerHost.empty() || g_strCenterServerHost == "0.0.0.0" || g_nCenterServerPort <= 0)
    {
#ifdef CS_FILE_LOG
        fprintf(m_pCSConnLog, "\n中心数据服务器连接参数异常:host=%s,port=%d\n",
                g_strCenterServerHost.c_str(), g_nCenterServerPort);
        fflush(m_pCSConnLog);
#endif
        //printf("\n中心数据服务器连接参数异常:host=%s,port=%d\n", g_strCenterServerHost.c_str(), g_nCenterServerPort);
        return false;
    }

    if (!mvPrepareSocket(m_nCenterSocket))
    {
#ifdef CS_FILE_LOG
        fprintf(m_pCSConnLog, "\n准备连接中心数据服务器套接字失败!\n");
        fflush(m_pCSConnLog);
#endif
        //printf("\n准备连接中心数据服务器套接字失败!\n");
        return false;
    }

    if (!mvWaitConnect(m_nCenterSocket, g_strCenterServerHost, g_nCenterServerPort))
    {
#ifdef CS_FILE_LOG
        fprintf(m_pCSConnLog, "\n尝试连接中心数据服务器失败，host=%s,port=%d!\n",
                g_strCenterServerHost.c_str(), g_nCenterServerPort);
        fflush(m_pCSConnLog);
#endif
        //printf("\n尝试连接中心数据服务器失败!\n");
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
bool mvCCenterServer::mvStartRecvThread()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvStartRecvThread()\n");
#endif
    pthread_attr_t   attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    mvJoinThread(g_uRecvCSMsgThreadId);
    if (pthread_create(&g_uRecvCSMsgThreadId, &attr, RecvCSMsgThread, NULL) != 0)
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
void mvCCenterServer::mvCheckFreeAndDealHistoryRecord()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvCheckFreeAndDealHistoryRecord()\n");
#endif
    unsigned int uTimeStamp = GetTimeStamp();
    if (m_bHistoryRep || (uTimeStamp >= m_uHistoryRepTime+30))
    {
        printf("==============mvCheckFreeAndDealHistoryRecord\n");
        string strMsg("");
        {
            if (mvGetOneHistoryRecord(strMsg))
            {
                if (mvSendRecordToCS(strMsg, false))
                {
                    MIMAX_HEADER* sHeader = (MIMAX_HEADER*)strMsg.c_str();
                    UINT32 uSeq =*((unsigned int*)(strMsg.c_str()+sizeof(MIMAX_HEADER)));

                    if (MIMAX_EVENT_REP == sHeader->uCmdID)
                    {
                        if (PERSON_ROAD == g_nRoadType)
                        {
                            g_skpDB.UpdateRecordStatus(MIMAX_EVENT_REP, uSeq);
                        }
                        else
                        {
                            g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, uSeq);
                        }
                    }
                    else if (MIMAX_PLATE_REP == sHeader->uCmdID)
                    {
                        g_skpDB.UpdateRecordStatus(MIMAX_PLATE_REP, uSeq);
                    }

                    #ifdef CS_FILE_LOG
                    FILE* fp = fopen("mvDealRealTimeRecorderror.txt","a+");
                    fprintf(fp,"===mvCheckFreeAndDealHistoryRecord,sHeader->uCmdID=%x,uSeq=%d\n",sHeader->uCmdID,uSeq);
                    fclose(fp);
                    #endif
                }
            }
        }
    }
}

/*
* 函数介绍：发送路由请求并接收应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendRouterReqAndRecv()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendRouterReqAndRecv()\n");
#endif
    //make up one msg and send to router.
    if (!mvSendConnReqToRouter())
    {
        return false;
    }

    //wait to receive router's reply and set center server's IP and port.
    if (!mvRecvIpPortFromRouter())
    {
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
bool mvCCenterServer::mvSendConnReqToRouter()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendConnReqToRouter()\n");
#endif
    char szBuff[20] = {0};

    sprintf(szBuff, "%15s%4s", g_ServerHost.c_str(), g_strCSMsgVer.c_str());

    return mvRebMsgAndSend(m_nControlSocket,START_WORK, string(szBuff));
}


/*
* 函数介绍：接收路由信息
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvRecvIpPortFromRouter()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvRecvIpPortFromRouter()\n");
#endif
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
#ifdef CS_FILE_LOG
        fprintf(m_pCSRecvLog, "\n接收消息成功: \n%s\n", strMsg.c_str());
        fflush(m_pCSRecvLog);
#endif

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
#ifdef CS_FILE_LOG
        fprintf(m_pCSRecvLog, "After parse, strDetectorId=%s, m_strDetectorId=%s\n",
                strDetectorId.c_str(), m_strDetectorId.c_str());
        fflush(m_pCSRecvLog);
#endif
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
bool mvCCenterServer::mvOnEndWork()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnEndWork()\n");
#endif
    g_bCenterLink = false;
    mvCloseSocket(m_nCenterSocket);

    if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
    {
#ifdef CS_FILE_LOG
        fprintf(m_pCSConnLog, "\n结束工作时，中心路由服务器连接参数异常! host=%s\tport=%d\n",
                g_strControlServerHost.c_str(), g_nControlServerPort);
        fflush(m_pCSConnLog);
#endif
        return false;
    }
    else
    {
        if (!mvPrepareSocket(m_nControlSocket))
        {
#ifdef CS_FILE_LOG
            fprintf(m_pCSConnLog, "\n结束工作时，准备连接套接字失败!\n");
            fflush(m_pCSConnLog);
#endif
            return false;
        }

        if (!mvWaitConnect(m_nControlSocket, g_strControlServerHost, g_nControlServerPort))
        {
#ifdef CS_FILE_LOG
            fprintf(m_pCSConnLog, "\n结束工作时，尝试连接中心路由服务器失败!\n");
            fflush(m_pCSConnLog);
#endif
            return false;
        }

        if (!mvSendEndWork())
        {
#ifdef CS_FILE_LOG
            fprintf(m_pCSConnLog, "\n发送结束工作消息失败!\n");
            fflush(m_pCSConnLog);
#endif
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
bool mvCCenterServer::mvOnDetectRestart() //(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnDetectRestart()\n");
#endif
    mvSendRestartRep();
    mvCloseSocket(m_nCenterSocket);
    g_bCenterLink = false;
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
bool mvCCenterServer::mvGetOneHistoryRecord(string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvGetOneHistoryRecord()\n");
#endif
    char szBuff[128] = {0};

    if (!strMsg.empty())
    {
        strMsg.clear();
    }
    char buf[1024]={0};
    unsigned int uTimeStamp = GetTimeStamp()-30;//30秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
    String strTime = GetTime(uTimeStamp);

	if (PERSON_ROAD == g_nRoadType)
	{
        sprintf(buf,"Select * from TRAFFIC_EVENT_INFO where STATUS = 0 and BEGIN_TIME <= '%s' ORDER BY BEGIN_TIME desc limit 1",strTime.c_str());
        string strSql(buf);
        MysqlQuery q = g_skpDB.execQuery(strSql);

        if (!q.eof())
        {
            MIMAX_HEADER sHeader;
            sHeader.uCmdID = MIMAX_EVENT_REP;

            RECORD_EVENT event;

            event.uSeq = q.getUnIntFileds("ID");
            event.uRoadWayID = q.getIntFileds("ROAD");
            event.uCode = q.getIntFileds("KIND");
            string strTime = q.getStringFileds("BEGIN_TIME");
            event.uEventBeginTime = MakeTime(strTime);
            event.uMiEventBeginTime = q.getIntFileds("BEGIN_MITIME");
            strTime = q.getStringFileds("END_TIME");
            event.uEventEndTime = MakeTime(strTime);
            event.uMiEventEndTime = q.getIntFileds("END_MITIME");
            event.uPicSize = q.getIntFileds("PICSIZE");
            event.uPicWidth = q.getIntFileds("PICWIDTH");
            event.uPicHeight = q.getIntFileds("PICHEIGHT");
            event.uPosX = q.getIntFileds("POSX");
            event.uPosY = q.getIntFileds("POSY");

            //事件快照路径
            string strPicPath = q.getStringFileds("PICPATH");
            memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size());

            strTime = q.getStringFileds("BEGIN_VIDEO_TIME");
            event.uVideoBeginTime = MakeTime(strTime);
            event.uMiVideoBeginTime = q.getIntFileds("BEGIN_VIDEO_MITIME");
            strTime = q.getStringFileds("END_VIDEO_TIME");
            event.uVideoEndTime = MakeTime(strTime);
            event.uMiVideoEndTime = q.getIntFileds("END_VIDEO_MITIME");

            string strVideoPath = q.getStringFileds("VIDEOPATH");
            memcpy(event.chVideoPath,strVideoPath.c_str(),strVideoPath.size());

            //车身颜色，类型，速度，方向等
            event.uColor1 = q.getIntFileds("COLOR");
            event.uType = q.getIntFileds("TYPE");
            if(event.uType == 1)
            {
                event.uType = PERSON_TYPE; //行人
            }
            else
            {
                event.uType = OTHER_TYPE; //非机动车
            }

            event.uSpeed = q.getIntFileds("SPEED");
            event.uWeight1 = q.getIntFileds("COLORWEIGHT");
            event.uColor2 = q.getIntFileds("COLORSECOND");
            event.uWeight2 = q.getIntFileds("COLORWEIGHTSECOND");
            event.uColor3 = q.getIntFileds("COLORTHIRD");
            event.uWeight3 = q.getIntFileds("COLORWEIGHTTHIRD");

            strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
            strMsg.append((char*)&event,sizeof(RECORD_EVENT));
            strMsg = strMsg+GetImageByPath(strPicPath);
        }
        q.finalize();
	}
	else
	{
	    sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME <= '%s' ORDER BY TIME desc limit 1",strTime.c_str());
        string strSql(buf);
        MysqlQuery q = g_skpDB.execQuery(strSql);

        if (!q.eof())
        {
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

            bool bPlateInfo = true;
            if(plate.uType >= OTHER_TYPE)
            {
                bPlateInfo = false;
            }

            if (bPlateInfo) //有牌车
            {
                if(plate.chText[0]=='*')//
                {
                    memcpy(plate.chText,"00000000",8);
                }
                memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

                sHeader.uCmdID = MIMAX_PLATE_REP;
                strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
                strMsg.append((char*)&plate, sizeof(RECORD_PLATE));
            }
            else //行人非机动车以事件形式发送
            {
                RECORD_EVENT  event;
                event.uSeq = plate.uSeq;
                event.uEventBeginTime = plate.uTime;
                event.uEventEndTime = plate.uTime + 5;
                event.uMiEventBeginTime = plate.uMiTime;
                event.uType = plate.uType;

                event.uRoadWayID = plate.uRoadWayID;
                event.uPicSize = plate.uPicSize;
                event.uPicWidth = plate.uPicWidth;
                event.uPicHeight = plate.uPicHeight;
                event.uPosX = (plate.uPosLeft+plate.uPosRight)/2;
                event.uPosY = (plate.uPosTop+plate.uPosBottom)/2;
                //event.uType =  plate.uType;
                event.uColor1 = plate.uCarColor1;
                event.uSpeed = plate.uSpeed;
                event.uColor2 = plate.uCarColor2;
                event.uWeight1 = plate.uWeight1;
                event.uWeight2 = plate.uWeight2;
                memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size());

                sHeader.uCmdID = MIMAX_EVENT_REP;
                strMsg.append((char*)&sHeader, sizeof(MIMAX_HEADER));
                strMsg.append((char*)&event, sizeof(RECORD_EVENT));
            }
            strMsg = strMsg+GetImageByPath(strPicPath);
        }
        q.finalize();
	}

    return (!strMsg.empty());
}

/*
* 函数介绍：标记未发送，重传
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvOnMarkUnsend(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnMarkUnsend()\n");
#endif
    char szDate1[11] = {0};
    char szTime1[13] = {0};
    char szDate2[11] = {0};
    char szTime2[13] = {0};

    sscanf(strMsg.c_str(), "%10s%12s%10s%12s", szDate1, szTime1, szDate2, szTime2);
    string strTime1 = szDate1;
    strTime1 += " ";
    strTime1.append(szTime1, 8);
    string strTime2 = szDate2;
    strTime2 += " ";
    strTime2.append(szTime2, 8);

    char szBuff[1024] = {0};
    UINT uCount = 0;
    string strResMsg(""), strTableName(""), strTimeName("");

    if (PERSON_ROAD == g_nRoadType)
    {
        strTableName = "TRAFFIC_EVENT_INFO";
        strTimeName = "BEGIN_TIME";
    }
    else
    {
        strTableName = "NUMBER_PLATE_INFO";
        strTimeName = "TIME";
    }

    sprintf(szBuff, "select count(*) from %s where STATUS=1 and %s>='%s' and %s<='%s';",
            strTableName.c_str(), strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
    MysqlQuery count = g_skpDB.execQuery(string(szBuff));
    if (!count.eof())
    {
        uCount = count.getUnIntFileds(0);
    }
    count.finalize();

    memset(szBuff, 0, 1024);
    sprintf(szBuff, "update %s set STATUS=0 where STATUS=1 and %s>='%s' and %s<='%s';",
            strTableName.c_str(), strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
    g_skpDB.execSQL(string(szBuff));

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

/*
* 函数介绍：记录查询
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvOnRecordQuery(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnRecordQuery()\n");
#endif
    char szDate1[11] = {0};
    char szTime1[13] = {0};
    char szDate2[11] = {0};
    char szTime2[13] = {0};

    sscanf(strMsg.c_str(), "%10s%12s%10s%12s", szDate1, szTime1, szDate2, szTime2);
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
    }
    else
    {
        strTableName = "NUMBER_PLATE_INFO";
        strTimeName = "TIME";
    }

    sprintf(szBuff, "select count(*) from %s where STATUS!=2 and %s>='%s' and %s<='%s';",
            strTableName.c_str(), strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
    MysqlQuery count1 = g_skpDB.execQuery(string(szBuff));
    if (!count1.eof())
    {
        uTotalCount = count1.getUnIntFileds(0);
    }
    count1.finalize();

    memset(szBuff, 0, 1024);
    sprintf(szBuff, "select count(*) from %s where STATUS=0 and %s>='%s' and %s<='%s';",
            strTableName.c_str(), strTimeName.c_str(), strTime1.c_str(), strTimeName.c_str(), strTime2.c_str());
    MysqlQuery count2 = g_skpDB.execQuery(string(szBuff));
    if (!count2.eof())
    {
        uCount = count2.getUnIntFileds(0);
    }
    count2.finalize();

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
bool mvCCenterServer::mvOnDeviceStatus(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnDeviceStatus()\n");
#endif
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
	char chLoopStatus = 'y';
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
	}

    //printf("m_nAllDisk=%07d, nRestDisk=%07d, g_sysInfo.fCpu=%03d\n", m_nAllDisk, nRestDisk, (int)g_sysInfo.fCpu);
    sprintf(szBuff, "%15s%02d%c%02d%c%03d%c%07d%07d%03d",
            g_ServerHost.c_str(), 1,chLoopStatus, 1, chCameraState, nTemperature, chGateStatus, m_nAllDisk, nRestDisk, (int)g_sysInfo.fCpu);

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
bool mvCCenterServer::mvOnSysTimeSetup(const string &strMsg)
{
    char szDate[11] = {0};
    char szTime[13] = {0};

    sscanf(strMsg.c_str(), "%10s%12s", szDate, szTime);
    string strTime = szDate;
    strTime += " ";
    strTime.append(szTime, 12);
    printf("strTime=%s,strTime.size()=%d\n",strTime.c_str(),strTime.size());
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
		UINT32 timeSec = MakeTime(strTime);
		string timeUsec = strTime.substr(20, 3);
		timeval timer;
		timer.tv_sec = timeSec;
		timer.tv_usec = atoi(timeUsec.c_str()) * 1000;

		if (settimeofday(&timer, NULL) == 0)
		{
			printf("================mvOnSysTimeSetup=%s\n",strTime.c_str());
			system("hwclock --systohc");
		}
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

/*
* 函数介绍：结束工作
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvOnEndWorkRep() //(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnEndWorkRep()\n");
#endif
    LogNormal("结束工作\n");
    //exit(1);
    //return true;
}

/*
* 函数介绍：实时记录应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvOnRealTimeRecordRep() //(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnRealTimeRecordRep()\n");
#endif
    m_nCSLinkCount = 0;
    return true;
}

/*
* 函数介绍：历史记录应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvOnHistoryRecordRep() //(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvOnHistoryRecordRep()\n");
#endif
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
bool mvCCenterServer::mvSendLinkTest()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendLinkTest()\n");
#endif
    return mvRebMsgAndSend(m_nCenterSocket,LINK_TEST, string(""));
}

/*
* 函数介绍：发送结束工作报告
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendEndWork()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendEndWork()\n");
#endif
    return mvRebMsgAndSend(m_nControlSocket,END_WORK, string(""));
}

/*
* 函数介绍：发送重启应答
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendRestartRep()
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendRestartRep()\n");
#endif
    return mvRebMsgAndSend(m_nCenterSocket,DETECT_RESTART_REP, string(""));
}

/*
* 函数介绍：发送标记未发送应答
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendMarkUnsendRep(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendMarkUnsendRep()\n");
#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,MARK_UNSEND_REP, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送记录查询应答
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendRecordQueryRep(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendRecordQueryRep()\n");
#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,RECORD_QUERY_REP, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送设备状态查询应答
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendDeviceStatusRep(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendDeviceStatusRep()\n");
#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,DEVICE_STATUS_REP, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送设备状态报警
* 输入参数：strMsg-消息内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendDeviceAlarm(const string &strMsg)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendDeviceAlarm()\n");
#endif
    //string strNewMsg("");
    return mvRebMsgAndSend(m_nCenterSocket,DEVICE_ALARM, strMsg); //strNewMsg);
}

/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool mvCCenterServer::mvSendRecordToCS(const string &strMsg, bool bRealTime)
{
    if(!g_bCenterLink)
    {
        return false;
    }
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvSendRecordToCS()\n");
#endif
    //including events and plates.
    //strMsg = MIMAX_HEADER+RECORD_EVENT/RECORD_PLATE+picture.
    string strNewMsg("");
    char szBuff[1024] = {0};
    string strTime1(""), strTime2(""), strPlateNum(""), strPlace(""), strIcon("");
    int nPlateColor, nPlateStruct, nPlateCount, nType;
    char chBodyColor;
    int nPicSize;

    MIMAX_HEADER* sHeader = (MIMAX_HEADER *)strMsg.c_str();

    if (MIMAX_EVENT_REP == sHeader->uCmdID)
    {
        nPicSize = strMsg.size() - sizeof(MIMAX_HEADER) - sizeof(RECORD_EVENT);
        strPlace = "999";
        strIcon = "99";
        chBodyColor = 'Z';
        nPlateStruct = 5;
        nType = 9;
        nPlateCount = 0;

        RECORD_EVENT *pEvent = (RECORD_EVENT *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
        mvGetDateTimeFromSec(pEvent->uEventBeginTime, strTime1);
        mvGetDateTimeFromSec(pEvent->uEventBeginTime, strTime2, 1);
        //convertion here.
        UINT uColor = pEvent->uColor1;
        int nTplx = 0;
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
            nTplx = 5;
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
            nTplx = 3;
        }
        else
        {
            //机动车
            strPlateNum = "00000000";
            nPlateColor = 4;
            nPlateCount = 1;
            nTplx = 1;
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
        sprintf(szBuff, "%15s%02d%14s%03d%15s%02d%19s%03d%3s%c%02d%02d%010d%d%d%d%05d%05d%2s%d%d%d%07d",
                m_strDetectorId.c_str(), pEvent->uRoadWayID, strTime1.c_str(), pEvent->uMiEventBeginTime, strPlateNum.c_str(),
                nPlateColor, strTime2.c_str(), pEvent->uSpeed, strPlace.c_str(), chBodyColor, 0, 0, nDetectTime, nPlateStruct, nPlateCount, 1,
                pEvent->uPosX, pEvent->uPosY, strIcon.c_str(), nType, 1, nTplx, nPicSize);

        strNewMsg = szBuff;
        strNewMsg.append(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_EVENT), strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_EVENT));
    }
    else if (MIMAX_PLATE_REP == sHeader->uCmdID)
    {
        nPicSize = strMsg.size() - sizeof(MIMAX_HEADER) - sizeof(RECORD_PLATE);
        nPlateCount = 1;

        RECORD_PLATE *pPlate = (RECORD_PLATE *)(strMsg.c_str() + sizeof(MIMAX_HEADER));
        mvGetDateTimeFromSec(pPlate->uTime, strTime1);
        mvGetDateTimeFromSec(pPlate->uTime, strTime2, 1);

		if(pPlate->chText[0]=='1')//
        {
            memcpy(pPlate->chText,"00000000",8);
        }
        //convertion here.
        strPlateNum = pPlate->chText;
        strPlace.append(pPlate->chText, 3);
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
                nPlateColor = 4;
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

        sprintf(szBuff, "%15s%02d%14s%03d%15s%02d%19s%03d%3s%c%02d%02d%010d%d%d%d%05d%05d%2s%d%d%d%07d",
                m_strDetectorId.c_str(), pPlate->uRoadWayID, strTime1.c_str(), pPlate->uMiTime, strPlateNum.c_str(),
                nPlateColor, strTime2.c_str(), pPlate->uSpeed, strPlace.c_str(), chBodyColor, 0, 0, nDetectTime, nPlateStruct, nPlateCount, 1,
                pPlate->uPosLeft, pPlate->uPosTop, strIcon.c_str(), nType, 1, 1, nPicSize);

        strNewMsg = szBuff;
        strNewMsg.append(strMsg.c_str()+sizeof(MIMAX_HEADER)+sizeof(RECORD_PLATE), strMsg.size()-sizeof(MIMAX_HEADER)-sizeof(RECORD_PLATE));
    }

    int nMsgLength = strNewMsg.size() + 8;
    char szLength[8] = {0};
    sprintf(szLength, "%07d", nMsgLength);
    strNewMsg.insert(0, szLength, 7);

    string strType("");
    if (bRealTime)
    {
#ifdef CS_FILE_LOG
        fprintf(m_pCSSendLog, "\n=========发送====实时数据=====>>>到电科中心端!\n");
        fflush(m_pCSSendLog);
#endif
        strType = REALTIME_RECORD;
    }
    else
    {
#ifdef CS_FILE_LOG
        fprintf(m_pCSSendLog, "\n=========发送====历史数据=====>>>到电科中心端!\n");
        fflush(m_pCSSendLog);
#endif
        strType = HISTORY_RECORD;
        m_bHistoryRep = false;
        m_uHistoryRepTime = GetTimeStamp();//记住发送时刻
    }
    return mvRebMsgAndSend(m_nCenterSocket,strType.c_str(), strNewMsg);
}

/*
* 函数介绍：CRC编码
* 输入参数：strSrc-编码前内容；strRes-编码后内容变量
* 输出参数：strRes-编码后内容
* 返回值 ：成功返回true，否则返回false
*/
void mvCCenterServer::mvCRCEncode(const string &strSrc, string &strRes)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvCRCEncode()\n");
#endif
    unsigned int crc = 0xffffffff;

    for (int i = 0; i < strSrc.size(); i++)
    {
        crc = ((crc >> 8) & 0xFFFFFF) ^ crcTable[(BYTE)((crc & 0xff) ^ strSrc[i])];
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
void mvCCenterServer::mvGetDateTimeFromSec(const long &uSec, string &strDate, int nType)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvGetDateTimeFromSec()\n");
#endif
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
void mvCCenterServer::mvCheckDeviceStatus(int nDeviceStatusKind)
{
#ifdef CS_STUPID_LOG
    CSSLog("In mvCCenterServer::mvCheckDeviceStatus()\n");
#endif

	if(nDeviceStatusKind == 0)//硬件报警
	{
		if (g_sysInfo.fCpu > 99)
		{
			char szBuff[4] = {0};
			sprintf(szBuff, "%c%2s", '0', "01");
			mvSendDeviceAlarm(string(szBuff));
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
bool mvCCenterServer::mvRecvCenterServerMsg()
{
        string strMsg("");
        //receive msg and push it into the msg queue.
        if (mvRecvSocketMsg(m_nCenterSocket, strMsg))
        {
#ifdef CS_FILE_LOG
            fprintf(m_pCSRecvLog, "\n接收消息成功:\n %s\n", strMsg.c_str());
            fflush(m_pCSRecvLog);
#endif
            mvPushOneMsg(strMsg);
            return true;
        }
        else
        {
            return false;
        }
}

//添加一条数据
bool mvCCenterServer::AddResult(std::string& strMsg)
{
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
	        if(m_ChannelResultList.size() > 3)//防止堆积的情况发生
	        {
	            //LogError("记录过多，未能及时发送!\r\n");
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
bool mvCCenterServer::OnResult(std::string& result)
{
	//发送数据
	MIMAX_HEADER mHeader;
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();

	bool bSendToServer = false;
	bool bObject = false;

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

    //此时去取图片
    if(mHeader.uCmdID == MIMAX_EVENT_REP)
    {
        if( (mHeader.uCmdFlag & 0x00010000) == 0x00010000)
        {
            bObject = true;
            mHeader.uCmdFlag = 0x00000001;
        }
        RECORD_EVENT* sEvent = (RECORD_EVENT*)(result.c_str());
        String strPicPath(sEvent->chPicPath);
        result = result + GetImageByPath(strPicPath);
    }
    else if(mHeader.uCmdID == MIMAX_PLATE_REP)
    {
        RECORD_PLATE* sPlate = (RECORD_PLATE*)(result.c_str());
        String strPicPath(sPlate->chPicPath);
        result = result + GetImageByPath(strPicPath);
    }

    //数据长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + result.size();
	//添加头
	result.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

    //发送数据
    if(bSendToServer)
    {
        if( (mHeader.uCmdID == MIMAX_EVENT_REP)||(mHeader.uCmdID == MIMAX_PLATE_REP))
        {
            if (g_CenterServer.mvSendRecordToCS(result,true))
            {
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

//处理记录结果
void mvCCenterServer::DealResult()
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
bool mvCCenterServer::mvRecvSocketMsg(int nSocket, string& strMsg)
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
        nLeft = 43+15;
    }
    else if (HISTORY_RECORD_REP == strCode)
    {
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
        nLeft = 53+15;
    }
    else if (MARK_UNSEND == strCode)
    {
        nLeft = 53+15;
    }
    else if (LINK_TEST_REP == strCode)
    {
        nLeft = 9+15;
    }
    else if (SYSTIME_SETUP == strCode) //Receive it but will not process it.
    {
        nLeft = 31+15;
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
void mvCCenterServer::SetLoopStatus(mapLoopStatus& vLoopStatus)
{
	m_vLoopStatus = vLoopStatus;
}

//获取线圈状态
mapLoopStatus mvCCenterServer::GetLoopStatus()
{
	return m_vLoopStatus;
}
