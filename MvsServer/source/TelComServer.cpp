// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "Common.h"
#include "CommonHeader.h"
#include "TelConstdef.h"
#include "TelComServer.h"

CTelComServer g_TelComServer;

#define ARRAY_SIZE	     4
string AlgArray[ARRAY_SIZE][2]={
	{"101","越线检测,目标物穿越警戒线"},
	{"102","越界检测,目标物穿越警戒区"},
	{"111","计数统计,对一定区域场景中按照一定方向穿越统计线的人数进行累计计数"},
	{"112","密度检测,划定区域内客流密度分析"}
};

/*
* 函数介绍：接收消息线程入口
* 输入参数：pArg-线程入口参数，接收套接字
* 输出参数：无
* 返回值 ：无
*/
void *RecvIVMMsgThread(void *pArg)
{
    while (!g_bEndThread)
    {
		if (FAILED == g_TelComServer.m_bLoginIVM)
		{
			break;
		}
		else
		{
		   g_TelComServer.mvRecvIVMMsg();
		}
        usleep(100);
    }

    LogError("接收AMS消息线程退出\r\n");

    pthread_exit((void *)0);
}

//接收事件发送回应(拌线、流量、密度回复)
void *ThreadRecvResultRep(void *pArg)
{
    while (!g_bEndThread)
    {

		g_TelComServer.mvRecvCMSRep(g_TelComServer.m_nResultUpSocket);
        usleep(100);
    }

    LogError("接收事件回复线程退出\r\n");

    pthread_exit((void *)0);
}

//接收图片发送回应
void *ThreadRecvPicRep(void *pArg)
{
	while (!g_bEndThread)
	{
		g_TelComServer.mvRecvCMSRep(g_TelComServer.m_nPicUpSocket);
		usleep(100);
	}

	LogError("接收图片回复线程退出\r\n");

	pthread_exit((void *)0);
}

//记录发送线程
void* ThreadTelCMSResult(void* pArg)
{
	//处理一条数据
	g_TelComServer.DealResult();

    pthread_exit((void *)0);
	return pArg;
}

CTelComServer::CTelComServer()
{
    m_bIVMLink = false;
    m_nIVMLinkCount = 0;
    
    m_nIvmSocket = 0;
	m_nResultUpSocket = 0;
	m_nPicUpSocket = 0;    

    m_sIvmServerHost = "";//IVM ip地址
    m_nIvmServerPort = 0;// IVM 端口号
    m_bLoginIVM = NOLOGIN;
    m_nSEQCount = 0;

	m_bCMSResultLink = false;
	m_bCMSPicLink = false;
	m_sTaskSession = "";

    m_sResultUpHost = "";//CSM ip地址
    m_nResultUpPort = 0;// CSM 端口号
	m_sDataPicUpHost = "";//上传图片ip地址
	m_nDataPicUpPort = 0;// 上传图片端口号

	m_nTelCMSThreadId = 0;
	m_nResultRepThreadId = 0;
	m_nPicRepThreadId = 0;
	m_nRecvIVMMsgThreadId = 0;

    pthread_mutex_init(&m_Result_Mutex,NULL);
	pthread_mutex_init(&m_Image_Mutex,NULL);
}


CTelComServer::~CTelComServer()
{
    pthread_mutex_destroy(&m_Result_Mutex);
	pthread_mutex_destroy(&m_Image_Mutex);
}

//启动收发线程
bool CTelComServer::Init()
{
	//读配置文件
	map<string,string> config;
	LoadTelComConfig(config);

	map<string,string>::iterator it = config.find("IvmServerIp");
	if (it != config.end())
	{
		m_sIvmServerHost = it->second;
	}
	it = config.find("IvmServerPort");
	if (it != config.end())
	{
		m_nIvmServerPort = atoi((it->second).c_str());
	}
cerr<<"m_sIvmServerHost="<<m_sIvmServerHost<<endl;
cerr<<"m_nIvmServerPort="<<m_nIvmServerPort<<endl;
	//线程属性
	pthread_attr_t attr;
	//初始化
	pthread_attr_init(&attr);
	//连接线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//启动CMS事件回复监听线程
	int nret = pthread_create(&m_nResultRepThreadId,&attr,ThreadRecvResultRep,this);
	if(nret!=0)
	{
		//失败
		LogError("创建CMS接收事件响应线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//启动CMS文件回复监听线程
	nret = pthread_create(&m_nPicRepThreadId, &attr, ThreadRecvPicRep, this);
	if(nret!=0)
	{
		//失败
		LogError("创建CMS接收图片文件响应线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

    //启动CMS发送线程
	nret=pthread_create(&m_nTelCMSThreadId,&attr,ThreadTelCMSResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	//启动IVM监听线程
	if (pthread_create(&m_nRecvIVMMsgThreadId, &attr, RecvIVMMsgThread, NULL) != 0)
	{
		//失败
		LogError("创建IVM监听线程失败,服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	pthread_attr_destroy(&attr);
	return true;
}

//释放
bool CTelComServer::UnInit()
{
    //停止线程
	if(m_nRecvIVMMsgThreadId != 0)
	{
		pthread_join(m_nRecvIVMMsgThreadId,NULL);
		m_nRecvIVMMsgThreadId = 0;
	}
	if(m_nTelCMSThreadId != 0)
	{
		pthread_join(m_nTelCMSThreadId,NULL);
		m_nTelCMSThreadId = 0;
	}
	
	if(m_nResultRepThreadId != 0)
	{
		pthread_join(m_nResultRepThreadId,NULL);
		m_nResultRepThreadId = 0;
	}
	if(m_nPicRepThreadId != 0)
	{
		pthread_join(m_nPicRepThreadId,NULL);
		m_nPicRepThreadId = 0;
	}

    m_ResultList.clear();
	return true;
}

/*
* 函数介绍：断开重连或发送心跳
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void CTelComServer::mvConnOrLinkTest()
{
    if (!g_bEndThread)
    {
        if(FAILED == m_bLoginIVM)
        {
			//登陆失败就不再操作
            cerr<<"login failed."<<endl;
            return;
        }
        if (!m_bIVMLink)
        {
			//未连接就做一次连接操作
            if (mvConnectToIVM())
            {
                LogNormal("连接IVM服务器成功!\n");
                m_bIVMLink = true;
                m_nIVMLinkCount = 0;

				if (NOLOGIN == m_bLoginIVM)
				{
					//没登陆过就发送登陆IVM请求
					string loginMsg = CreateLoginXml();
					if(!SendMsgToIVM(TEL_LOGIN, loginMsg))
					{
						LogError("向IVM发送登陆信息失败\n");
					}
				}
            }
        }
        else
        {
			if (NOLOGIN == m_bLoginIVM)
			{
				//没登陆过就发送登陆IVM请求
				string loginMsg = CreateLoginXml();
				if(!SendMsgToIVM(TEL_LOGIN, loginMsg))
				{
					LogError("向IVM发送登陆信息失败\n");
				}
				return;
			}
		    if (!mvSendLinkTest())
            {
                LogError("发送心跳包失败\n");

				//3次发送失败,即认为连接断开
				if (m_nIVMLinkCount++ > SRIP_LINK_MAX)
				{
					mvCloseSocket(m_nIvmSocket);
					m_bLoginIVM = NOLOGIN;
					m_bIVMLink = false;
					m_nIVMLinkCount = 0;
					LogError("长时间发送心跳失败，连接断开\n");
				}
            }
			//----------测试代码---------
			vector<string> vec;
			time_t now = time(NULL);
			string time = GetTime(now, 2);
			vec.push_back(time);
			vec.push_back("异常描述:XXX异常");
			TelIVMException(vec);
			//----------测试代码---------
        }
    }
    sleep(25);
}

/*
* 函数介绍：向IVM发送异常
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CTelComServer::TelIVMException(const vector<string>& vec)
{
	if (vec.size() < 2)
	{
		return false;
	}
	string strMsg = CreateExNotifyXml(vec);

	if(!SendMsgToIVM(TEL_EXCEPTION, strMsg))
	{
		LogError("向IVM发送异常信息失败\n");
		return false;
	}
	return true;
}

/*
* 函数介绍：发送消息给IVM
* 输入参数：cmd命令字，strMsg报文数据部分, 如果是回复需要本次连接的seq流水号
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CTelComServer::SendMsgToIVM(BYTE cmd, string strData, unsigned short seq)
{
    TEL_CENTER_HEADER tHeader;
    tHeader.uHead = htons(TEL_IVM_HEAD);
    tHeader.uCmd = cmd;
    tHeader.uLen = htons(strData.size());

    if(seq)
    {
        tHeader.uSeq = htons(seq);
    }
    else
    {
        tHeader.uSeq = htons(m_nSEQCount++);
    }
cerr<<endl<<"send header="<<endl;
char buf[20]={0};
sprintf(buf,"%x", ntohs(tHeader.uHead));cerr<<buf<<endl;memset(buf,0,20);
sprintf(buf,"%x", tHeader.uCmd);cerr<<buf<<endl;
cerr<<ntohs(tHeader.uSeq)<<endl
<<strData.size()<<endl;
	string strMsg = "";
    strMsg.append((char*)&tHeader, sizeof(TEL_CENTER_HEADER));
	strMsg.append(strData);

    //追加Md5验证摘要
	unsigned char digest[TEL_MD5_16] = {0};
    GetStringMd5(strMsg, digest);
    strMsg.append((char*)digest, TEL_MD5_16);

cerr<<strData<<endl;;
cerr<<"send MD5摘要="<<endl;;
for(int i=0; i<16; i++)
{
	cerr<<(int)digest[i]<<" ";
}cerr<<endl;

    if(!mvSendMsgToSocket(m_nIvmSocket, strMsg))
    {
        mvCloseSocket(m_nIvmSocket);
        m_bIVMLink = false;
        LogError("向IVM发送消息失败\n");
cerr<<"向IVM发送消息失败"<<endl;
		return false;
    }
	m_nIVMLinkCount = 0;
    return true;
}

/*
* 函数介绍：发送消息给CMS
* 输入参数：socket套接字
* 输入参数：strMsg报文头+数据部分
* 输入参数：type=0发送事件  tpye=1发送文件
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CTelComServer::SendMsgToCMS(int& socket, string strMsg, int type)
{
	//网络字节转换
	TEL_CENTER_HEADER* tHeader = (TEL_CENTER_HEADER*)strMsg.c_str();
	tHeader->uHead = htons(tHeader->uHead);
	tHeader->uSeq = htons(tHeader->uSeq);
	tHeader->uLen = htons(tHeader->uLen);

	//追加Md5验证摘要
	unsigned char digest[TEL_MD5_16] = {0};
	GetStringMd5(strMsg, digest);
	strMsg.append((char*)digest, TEL_MD5_16);

cerr<<endl<<"send CMS header="<<endl;
char buf[20]={0};
sprintf(buf,"%x", ntohs(tHeader->uHead));cerr<<buf<<endl;memset(buf,0,20);
sprintf(buf,"%x", tHeader->uCmd);cerr<<buf<<endl;
cerr<<ntohs(tHeader->uSeq)<<endl
<<ntohs(tHeader->uLen)<<endl;

cerr<<"send CMS MD5摘要="<<endl;
for(int i=0; i<16; i++)
{
	cerr<<(int)digest[i]<<" ";
}cerr<<endl;
	if(!mvSendMsgToSocket(socket, strMsg))
	{
		//重新连接到CMS，再次发送消息
		connectCMS(socket, type);
		if(!mvSendMsgToSocket(socket, strMsg))
		{
			mvCloseSocket(socket);
			LogError("向%s发送数据失败，数据丢弃\n", type ? "文件服务器" : "事件服务器");
			return false;
		}
	}
	return true;
}

/*
* 函数介绍：连接到IVM
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CTelComServer::mvConnectToIVM()
{
    if (m_sIvmServerHost.empty() || m_sIvmServerHost == "0.0.0.0" || m_nIvmServerPort <= 0)
    {
        LogError("\nIVM连接参数失败:host=%s,port=%d\n", m_sIvmServerHost.c_str(), m_nIvmServerPort);
        return false;
    }

	LogNormal("开始连接IVM服务器! host=%s,port=%d\n", m_sIvmServerHost.c_str(), m_nIvmServerPort);

    if (!mvPrepareSocket(m_nIvmSocket))
    {
		m_nIvmSocket = 0;
        LogError("\n准备连接IVM套接字失败!\n");
        return false;
    }
    if (!mvWaitConnect(m_nIvmSocket, m_sIvmServerHost, m_nIvmServerPort))
    {
		m_nIvmSocket = 0;
        LogError("\n尝试连接IVM失败!\n");
        return false;
    }
    return true;
}

/*
* 函数介绍：连接到CMS
* 输入参数：socket描述符
* 输入参数：type类型。1:连接到文件服务器  0:连接到事件服务器
* 返回值 ：成功返回true，否则返回false
*/
bool CTelComServer::connectCMS(int& socket, int type)
{
	if (!g_bEndThread)
	{
		string host ="";
		int port = 0;

		if (0 == type)
		{
			host = m_sResultUpHost;
			port = m_nResultUpPort;
			LogNormal("开始连接事件服务器! host=%s,port=%d\n", host.c_str(), port);
		}
		else
		{
			host = m_sDataPicUpHost;
			port = m_nDataPicUpPort;
			LogNormal("开始连接文件服务器! host=%s,port=%d\n", host.c_str(), port);
		}
		//connect to center server and set socket's option.
		if (host.empty() || host == "0.0.0.0" || port <= 0)
		{
			LogError("\n服务器连接参数失败:host=%s,port=%d\n", host.c_str(), port);
			return false;
		}

		if (!mvPrepareSocket(socket))
		{
			socket = 0;
			LogError("\n准备连接服务器套接字失败!\n");
			return false;
		}
		string server = type ? "文件服务器" : "事件服务器";
		if (!mvWaitConnect(socket, host, port))
		{
			socket = 0;
			LogError("尝试连接%s失败!\n", server.c_str());
			return false;
		}
		LogNormal("尝试连接%s成功!\n", server.c_str());
	}
    return true;
}

/*
* 函数介绍：向IVM发送心跳测试
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CTelComServer::mvSendLinkTest()
{
    string strMsg = CreateLinkXml();

    return SendMsgToIVM(TEL_LINK, strMsg);
}

//接收IVM消息
bool CTelComServer::mvRecvIVMMsg()
{
    string strMsg("");
    if(mvRecvMsg(m_nIvmSocket, strMsg))
    {
		LogNormal("从IVM收到MSG!\n");
		//处理一条消息
        doIVMMsg(strMsg);
        return true;
    }
    return false;
}

//接收CMS中心端消息
bool CTelComServer::mvRecvCMSRep(int socket)
{
    string strMsg("");

	//接收套接字的消息
    if(mvRecvMsg(socket, strMsg))
    {
        doRepResult(strMsg);
        return true;
    }
	return false;
}

//处理收到的CMS消息
void CTelComServer::doRepResult(string strMsg)
{
    TEL_CENTER_HEADER tHeader;
    memcpy(&tHeader,(char*)strMsg.c_str(),sizeof(TEL_CENTER_HEADER));

	BYTE ret;
    if(tHeader.uHead == TEL_CMS_EVENT_HEAD && tHeader.uCmd == TEL_BEYOND_LINE) //拌线回应
    {
		memcpy(&ret, strMsg.c_str() + sizeof(TEL_CENTER_HEADER), 1);

		switch ((int)ret)
		{
		case 0:
			LogNormal("拌线报警返回成功!\n");
			break;
		case 1:
			LogError("拌线报警返回失败:未知CommandID!\n");
			break;
		case 2:
			LogError("拌线报警返回失败:任务编号不存在!\n");
			break;
		default:
			LogError("拌线报警返回失败:其他错误!\n");
			break;
		}
    }
    else if(tHeader.uHead == TEL_CMS_EVENT_HEAD && tHeader.uCmd == TEL_FLUX) //客流人数统计回应
    {
		memcpy(&ret, strMsg.c_str() + sizeof(TEL_CENTER_HEADER), 1);

		switch ((int)ret)
		{
		case 0:
			LogNormal("客流人数统计返回:成功!\n");
			break;
		case 1:
			LogError("客流人数统计返回失败:未知CommandID!\n");
			break;
		case 2:
			LogError("客流人数统计返回失败:任务编号不存在!\n");
			break;
		default:
			LogError("客流人数统计返回失败:其他错误!\n");
			break;
		}
    }
    else if(tHeader.uHead == TEL_CMS_EVENT_HEAD && tHeader.uCmd == TEL_DENSITY) //客流密度统计回应
    {
		memcpy(&ret, strMsg.c_str() + sizeof(TEL_CENTER_HEADER), 1);

		switch ((int)ret)
		{
		case 0:
			LogNormal("客流密度统计返回:成功!\n");
			break;
		case 1:
			LogError("客流密度统计返回失败:未知CommandID!\n");
			break;
		case 2:
			LogError("客流密度统计返回失败:任务编号不存在!\n");
			break;
		default:
			LogError("客流密度统计返回失败:其他错误!\n");
			break;
		}
    }
	else if(tHeader.uHead == TEL_CMS_PIC_HEAD && tHeader.uCmd == TEL_FILE_ID) //文件发送请求回应
	{
		UINT32 fileId = 0;
		memcpy(&ret, strMsg.c_str() + sizeof(TEL_CENTER_HEADER), 1);
		memcpy(&fileId, strMsg.c_str() + sizeof(TEL_CENTER_HEADER) + 1, 4);

		LogNormal("收到文件发送请求回应!ret=%d\n", (int)ret);
		switch ((int)ret)
		{
		case 0:
			LogNormal("文件传输请求成功!\n");

			//收到请求回应后 发送图片
			if (fileId)
			{
				//取得保存的图片
				string image = "";
				pthread_mutex_lock(&m_Image_Mutex);
				map<UINT16, std::string>::iterator iter = m_ImageMap.find(tHeader.uSeq);
				if (iter != m_ImageMap.end())
				{
					image = iter->second;
					m_ImageMap.erase(iter);
					pthread_mutex_unlock(&m_Image_Mutex);
				}
				else
				{
					LogError("失败:收到FileId, 但没有找到本地保存的图片数据\n");
					pthread_mutex_unlock(&m_Image_Mutex);
					return;
				}

				//报头
				TEL_CENTER_HEADER header;
				header.uHead = TEL_CMS_PIC_HEAD;	//起始字头
				header.uCmd = TEL_IMAGE_FILE;		//命令字
				header.uSeq = tHeader.uSeq;			//流水号

				//数据包
				TEL_CENTER_DATA data;
				data.uFileId = fileId;
cerr<<"发送图片总大小="<<image.size()<<endl;
				int dataMaxSize = TEL_MAX_DATA_SIZE - sizeof(data);
				for (; image.size() > 0; image.erase(0, dataMaxSize))
				{
					string cutImage = "";
					if (image.size() > dataMaxSize)
					{
						data.uEndFlag = (BYTE)0;
						data.uDataLen = dataMaxSize;
						header.uLen = sizeof(data) + dataMaxSize;  //数据长度
						cutImage.append(image.c_str(), dataMaxSize);
					}
					else
					{
						data.uEndFlag = (BYTE)1;
						data.uDataLen = image.size();
						header.uLen = sizeof(data) + image.size();  //数据长度
						cutImage.append(image);
					}
					string strMsg = "";
					strMsg.append((char*)&header, sizeof(header));
					strMsg.append((char*)&data, sizeof(data));
					strMsg.append(cutImage);

cerr<<"分段数据包="<<endl;
cerr<<"data.uFileId="<<data.uFileId<<endl;
cerr<<"data.uEndFlag="<<data.uEndFlag<<endl;
cerr<<"data.uDataLen="<<data.uDataLen<<endl;
					//发送图片文件
					if (!SendMsgToCMS(m_nPicUpSocket, strMsg, 1))
					{
						LogError("发送图片失败\n");
						break;
					}
				}
			}
			else
				LogError("失败:收到的文件传输ID为空!\n");
			break;
		case 1:
			LogError("文件传输请求返回失败: 未知CommandID!\n");
			break;
		case 2:
			LogError("文件传输请求返回失败: 任务编号不存在!\n");
			break;
		default:
			LogError("文件传输请求返回失败: 其他错误!\n");
			break;
		}
	}
	else if(tHeader.uHead == TEL_CMS_PIC_HEAD && tHeader.uCmd == TEL_IMAGE_FILE) //数据包发送回应
	{
		memcpy(&ret, strMsg.c_str() + sizeof(TEL_CENTER_HEADER), 1);
		switch ((int)ret)
		{
		case 0:
			LogNormal("文件数据报上传成功!\n");
			break;
		case 1:
			LogError("文件数据报上传返回失败:未知CommandID!\n");
			break;
		case 2:
			LogError("文件数据报上传返回失败:任务编号不存在!\n");
			break;
		default:
			LogError("文件数据报上传返回失败:其他错误!\n");
			break;
		}
	}
	else
	{
		LogError("失败:从CMS收到未知命令字!\n");
	}
}

/*
* 函数介绍：接收nSocket消息
* 输入参数：nSocket-要接收消息的套接字；strMsg-将接收到的消息内容
* 输出参数：strMsg-接收到的消息内容
* 返回值 ：成功返回true，否则false
*/
bool CTelComServer::mvRecvMsg(int nSocket, string& strMsg)
{
    if (!strMsg.empty())
    {
        strMsg.clear();
    }
    if (nSocket <= 0)
    {
        return false;
    }
    TEL_CENTER_HEADER tHeader; 

	//接收报文头
    if (recv(nSocket, (void*)&tHeader,sizeof(tHeader), MSG_NOSIGNAL) <= 0)
    {
        return false;
    }
	string doMd5 = "";
	doMd5.append((char*)&tHeader, sizeof(tHeader));

	//网络字节转换
	tHeader.uHead = ntohs(tHeader.uHead);
	tHeader.uSeq = ntohs(tHeader.uSeq);
	tHeader.uLen = ntohs(tHeader.uLen);

    strMsg.append((char*)&tHeader,sizeof(TEL_CENTER_HEADER));

	//接收数据及摘要
	int bytes = 0;
	int size = tHeader.uLen + TEL_MD5_16;
	char chBuffer[size];
	memset(chBuffer, 0, size);
	if ((bytes = recv(nSocket, (void*)&chBuffer, size, MSG_NOSIGNAL)) < 0)
	{
		return false;
	}
	//保存数据
	strMsg.append(chBuffer, bytes);
	doMd5.append(chBuffer, tHeader.uLen);

    //md5验证
	unsigned char md5Buf[TEL_MD5_16 + 1] = {0};
	unsigned char digest[TEL_MD5_16 + 1] = {0};
	GetStringMd5(doMd5, digest);
	memcpy(md5Buf, strMsg.c_str() + sizeof(tHeader) + tHeader.uLen, TEL_MD5_16);

cerr<<endl<<"recv header="<<endl;
char buf[20]={0};
sprintf(buf,"%x", tHeader.uHead);cerr<<buf<<endl;memset(buf,0,20);
sprintf(buf,"%x", tHeader.uCmd);cerr<<buf<<endl;
cerr<<tHeader.uSeq<<endl
<<tHeader.uLen<<endl;
if (tHeader.uLen == 1)
{
cerr<<"ret="<<(int)chBuffer[0]<<endl;
}
else if (tHeader.uLen == 5)
{
	cerr<<"ret="<<(int)chBuffer[0]<<endl;
	int fileid=0;
	memcpy(&fileid, &chBuffer[1],4);
	cerr<<"fileID="<<fileid<<endl;
}
else{
string temp(chBuffer, tHeader.uLen);
cerr<<temp<<endl;
}

cerr<<"recv MD5--摘要="<<endl;;
for(int i=0; i<16; i++)
{
	cerr<<(int)md5Buf[i]<<" ";
}cerr<<endl;

    if(strcmp((char*)digest, (char*)md5Buf))
    {
cerr<<"收到的数据包MD5验证出错"<<endl;
        LogError("收到的数据包MD5验证出错\n");
//        return false;暂时用不着验证
    }

    return true;
}

/*
* 函数介绍：处理收到的IVM消息
* 输入参数：strMsg 收到的一条消息
* 输出参数：无
* 返回值 ：无
*/
void CTelComServer::doIVMMsg(string strMsg)
{
cerr<<"处理收到的IVM消息--"<<endl;
	
	static bool sendOnce = false;//发送算法能力成功标志
    TEL_CENTER_HEADER tHeader;
    memcpy(&tHeader, (char*)strMsg.c_str(), sizeof(tHeader));

    char buf[tHeader.uLen + 1];
    memset(buf, 0, tHeader.uLen + 1);
    memcpy(buf, strMsg.c_str() + sizeof(tHeader), tHeader.uLen);

    if(tHeader.uCmd == TEL_LOGIN) //登录回复
    {
		sendOnce = false;//每登陆一次，发送标志置为false
        XMLNode xml;
        string result = "";
        AnalyseLoginResXml(buf, result);

        if(result == "0")
        {
            LogNormal("成功登录到IVM!\n");
            m_bLoginIVM = SUCCSESS;
        }
        else
        {
            m_bLoginIVM = FAILED;
            if(result == "1")
                LogError("登录IVM失败: 设备不存在\n");
            else if(result == "2")
                LogError("登录IVM失败: 密码错误\n");
            else if(result == "3")
                LogError("登录IVM失败: 协议版本不支持\n");
            else
                LogError("登录IVM失败: 其他错误\n");
        }
    }
    else 
	{
		if (m_bLoginIVM != SUCCSESS)
		{
			//如果未登录或登录失败就退出
			return;
		}
		//if (tHeader.uCmd == TEL_LINK) //心跳回复
		//{
		//	LogNormal("收到IVM心跳回复!\n");
		//}
		else if(tHeader.uCmd == TEL_ALG_ABILITY) //算法上报回复
		{
			string ret = "";
			AnalyseAlgAbilityXml(buf, ret);
			if(ret == "0")
			{
				LogNormal("算法上报回复:成功!\n");
				sendOnce = true;
			}
			else
			{
				LogError("算法上报回复:失败!\n");
			}
		}
		else if(tHeader.uCmd == TEL_SEARCH_ALG) //智能分析结果数据项查询
		{
			string ret = "";
			AnalyseSearchXml(buf, ret);
			if (!ret.compare(""))
			{
				LogError("失败:解析算法查询要求中算法编码为空值!\n");
			}
			LogNormal("成功收到智能分析结果数据项查询请求! 算法编码=%s!\n", ret.c_str());

			string strMsg = CreateSearchXml(ret);
			if(SendMsgToIVM(TEL_SEARCH_ALG, strMsg, tHeader.uSeq))
			{
				LogNormal("发送智能分析查询结果成功!\n");
			}
			else
			{
				LogError("发送智能分析查询结果失败!\n");
			}
		}
		else if(tHeader.uCmd == TEL_START_DETECT) //开始智能分析请求
		{
			LogNormal("收到开始智能分析请求!\n");
cerr<<"开始智能分析标志00"<<endl;
			MONITOR_HOST_INFO cmsInfo;
			string ret = "1";
			if (m_sTaskSession.empty()) //如果智能分析已经停止
			{
				cerr<<"开始智能分析标志01"<<endl;
				AnalyseDetectStartXml(buf, cmsInfo, ret);
				cerr<<"开始智能分析标志02"<<endl;
				//?? 将cmsInfo传给视频调用模块

				if (!ret.compare("0"))
				{
					//调用开始视频分析接口
					g_skpChannelCenter.RestartDetect();
					LogNormal("智能分析开始!\n");
					cerr<<"开始智能分析标志03"<<endl;
				}
				else
				{
					LogError("失败:开始智能分析请求数据包错误!\n");
					cerr<<"开始智能分析标志04"<<endl;
				}
			}
			else
			{
				cerr<<"开始智能分析标志08"<<endl;
				LogError("失败:上次开启的智能分析未关闭,新收到的智能分析开启请求不操作!\n");
			}
			string strRet = CreateDetectStartRepXml(ret);
			SendMsgToIVM(TEL_START_DETECT, strRet, tHeader.uSeq);
			cerr<<"开始智能分析标志09"<<endl;
		}
		else if(tHeader.uCmd == TEL_STOP_DETECT) //停止智能分析请求
		{
			LogNormal("收到停止智能分析请求!\n");

			string ret = "1";
			AnalyseDetectStopXml(buf, ret);
			if (!ret.compare("0"))
			{
				//调用视频分析模块的接口
				g_skpChannelCenter.PauseDetect();
				m_sTaskSession = ""; //清空任务会话
				//清空数据
				pthread_mutex_lock(&m_Result_Mutex);
				m_ResultList.clear();
				pthread_mutex_unlock(&m_Result_Mutex);
				pthread_mutex_lock(&m_Image_Mutex);
				m_ImageMap.clear();
				pthread_mutex_unlock(&m_Image_Mutex);
				//关闭socket
				mvCloseSocket(m_nResultUpSocket);
				mvCloseSocket(m_nPicUpSocket);
				LogNormal("智能分析停止!\n");
			}
			else
			{
				LogError("失败:智能分析停止请求中任务ID出错!\n");
			}

			string strMsg = CreateDetectStopXml(ret);
			SendMsgToIVM(TEL_STOP_DETECT, strMsg, tHeader.uSeq);
		}
		else if(tHeader.uCmd == TEL_EXCEPTION) //异常上报回复
		{
			string ret = "1";
			AnalyseExRepXml(buf, ret);
			if (ret.compare("0"))
			{
				LogError("失败:异常上报回复错误!\n");
			}
			else
			{
				LogNormal("异常上报回复成功!\n");
			}
		} 
	}

	//登陆成功后算法能力上报
	if (SUCCSESS == m_bLoginIVM && sendOnce == false)
	{
		string algXml = CreateAlgAbilityXml();
		if (SendMsgToIVM(TEL_ALG_ABILITY, algXml))
		{
			LogNormal("发送算法能力成功!\n");
		}
		else
		{
			LogError("发送算法能力失败!\n");
		}
	}
}

//添加一条分析结果
bool CTelComServer::AddResult(std::string& strResult)
{
    //加锁
    pthread_mutex_lock(&m_Result_Mutex);

    m_ResultList.push_front(strResult);
    //解锁
    pthread_mutex_unlock(&m_Result_Mutex);

	return true;
}

//处理检测结果
void CTelComServer::OnResult(string& result)
{
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();
	string strEvent = "";

	switch(sDetectHeader->uDetectType)
	{
	case MIMAX_STATISTIC_REP: //人数流量或密度统计
		{
			RECORD_STATISTIC *record = (RECORD_STATISTIC*)(result.c_str() + sizeof(SRIP_DETECT_HEADER));

			int iDensity = -1;
			UINT16 num = 0;
			for (int i = 0; i < MAX_ROADWAY; i++)
			{
				if (record->uFlux[i] != 0xFFFFFFFF)
				{
					num = record->uFlux[i] & 0xFFFF;//低16位存放正反双方向的流量
				}
				if (record->uOccupancy[i] != 0xFFFFFFFF)
				{
					iDensity = record->uOccupancy[i];
				}
			}
			if (iDensity > -1)//密度统计
			{
				TEL_CENTER_DENSITY density;
				memcpy(density.sSession, m_sTaskSession.c_str(), m_sTaskSession.size());
				density.uNum = num;
				sprintf(density.sDensity, "%d", iDensity);

				string time = GetTime(record->uTime, 2);
				memcpy(density.sTime, time.c_str(), sizeof(density.sTime));
				density.uDirection = (BYTE)1; //统计方向、暂时无效

				//追加数据段
				strEvent.append((char*)&density, sizeof(density));

				TEL_CENTER_HEADER tHeader;
				tHeader.uHead = TEL_CMS_EVENT_HEAD;
				tHeader.uCmd = (BYTE)TEL_DENSITY;
				tHeader.uSeq = m_nSEQCount++;
				tHeader.uLen = strEvent.size();
				//追加报头
				strEvent.insert(0, (char*)&tHeader, sizeof(tHeader));
cerr<<"发送密度统计数据包="<<endl
<<"sSession="<<string(density.sSession,32)<<endl   	 
<<"uNum="<<density.uNum<<endl			 
<<"sDensity="<<string(density.sDensity,3)<<endl     		 	 
<<"sTime="<<string(density.sTime, 14)<<endl     		 	 
<<"uDirection="<<(int)density.uDirection<<endl;
			}
			else //流量统计
			{
				TEL_CENTER_FLUX flux;
				memcpy(flux.sSession, m_sTaskSession.c_str(), m_sTaskSession.size());

				string beginTime = GetTime(record->uTime - record->uStatTimeLen, 2);
				string endTime = GetTime(record->uTime, 2);
				memcpy(flux.sBeginTime, beginTime.c_str(), sizeof(flux.sBeginTime));
				memcpy(flux.sEndTime, endTime.c_str(), sizeof(flux.sEndTime));
				
				flux.uNum = num;
				flux.uDirection = (BYTE)1; //统计方向、暂时无效

				//追加数据段
				strEvent.append((char*)&flux, sizeof(flux));

				TEL_CENTER_HEADER tHeader;
				tHeader.uHead = TEL_CMS_EVENT_HEAD;
				tHeader.uCmd = (BYTE)TEL_FLUX;
				tHeader.uSeq = m_nSEQCount++;
				tHeader.uLen = strEvent.size();
				//追加报头
				strEvent.insert(0, (char*)&tHeader, sizeof(tHeader));
cerr<<"发送客流量统计数据包="<<endl
<<"sSession="<<string(flux.sSession,32)<<endl 
<<"uNum="<<flux.uNum<<endl 
<<"sBeginTime="<<string(flux.sBeginTime,14)<<endl 
<<"sEndTime="<<string(flux.sEndTime,14)<<endl 
<<"uDirection="<<(int)flux.uDirection<<endl; 
			}
			if (!SendMsgToCMS(m_nResultUpSocket, strEvent, 0))
			{
				LogError("发送统计数据失败\n");
			}
		}
		break;
	case MIMAX_EVENT_REP:	
		{
			RECORD_EVENT* pEvent = (RECORD_EVENT*)(result.c_str() + sizeof(SRIP_DETECT_HEADER));

			//不是越界或闯入 即跳出
			if(!(pEvent->uCode == DETECT_RESULT_EVENT_INSIDE || pEvent->uCode == DETECT_RESULT_EVENT_OUTSIDE))//非拌线
			{
				break;
			}

			string strImage = "";

			//追加报头
			TEL_CENTER_HEADER tHeader;
			tHeader.uHead = TEL_CMS_EVENT_HEAD;
			tHeader.uCmd = TEL_BEYOND_LINE;
			tHeader.uSeq = m_nSEQCount++;
			tHeader.uLen = sizeof(TEL_CENTER_BEYONDLINE);
			strEvent.append((char*)&tHeader, sizeof(tHeader));

			//追加数据段
			TEL_CENTER_BEYONDLINE beyondLine;
			memcpy(beyondLine.sSession, m_sTaskSession.c_str(), m_sTaskSession.size());
			beyondLine.uLineID = (BYTE)pEvent->uSeq;
			string time = GetTime(pEvent->uEventBeginTime, 2);
			memcpy(beyondLine.sTime, time.c_str(), sizeof(beyondLine.sTime));
			beyondLine.uDirection = (BYTE)pEvent->uDirection;
			
			strEvent.append((char*)&beyondLine, sizeof(beyondLine));
cerr<<"发送拌线="<<endl
<<"sSession="<<string(beyondLine.sSession,32)<<endl 
<<"uLineID="<<(int)beyondLine.uLineID<<endl 
<<"sTime="<<string(beyondLine.sTime,14)<<endl 
<<"uDirection="<<(int)beyondLine.uDirection<<endl; 
			//发送拌线数据
			if (!SendMsgToCMS(m_nResultUpSocket, strEvent, 0))
			{
				LogError("发送拌线报警失败\n");
cerr<<"发送拌线报警失败！"<<endl;
				return;
			}
cerr<<"发送拌线报警成功！"<<endl;

			//取得图片、发送图片传输请求
			string strPicPath(pEvent->chPicPath);
			strImage = GetImageByPath(strPicPath);
			if (strImage.size() > 0)
			{
				strEvent = "";

				//追加报头
				tHeader.uHead = TEL_CMS_PIC_HEAD;
				tHeader.uCmd = TEL_FILE_ID;
				tHeader.uSeq = m_nSEQCount;
				tHeader.uLen = sizeof(TEL_CENTER_FILEID);
				strEvent.append((char*)&tHeader, sizeof(tHeader));

				//追加报文
				TEL_CENTER_FILEID fileReq;
				memcpy(fileReq.sSession, m_sTaskSession.c_str(), m_sTaskSession.size());
				fileReq.uFileSize = strImage.size();
				memcpy(fileReq.sEventTime, time.c_str(), sizeof(fileReq.sEventTime));
				memcpy(fileReq.sFileTime, time.c_str(), sizeof(fileReq.sFileTime));

				strEvent.append((char*)&fileReq, sizeof(fileReq));
cerr<<"发送文件请求="<<endl
<<"file.sSession="<<string(fileReq.sSession,32)<<endl
<<"file.uFileSize="<<fileReq.uFileSize<<endl
<<"file.sEventTime="<<string(fileReq.sEventTime, 14)<<endl
<<"file.sFileTime="<<string(fileReq.sFileTime,14)<<endl;
				if (!SendMsgToCMS(m_nPicUpSocket, strEvent, 1))
				{
					m_nSEQCount++;
					LogError("发送文件传送请求失败\n");
					return;
				}
				//保存图片，以便收到确认后发送
				pthread_mutex_lock(&m_Image_Mutex);
				m_ImageMap.insert(make_pair(m_nSEQCount, strImage));
				pthread_mutex_unlock(&m_Image_Mutex);
				m_nSEQCount++;
			}
		}
		break;
	default:
		LogError("失败:未知类型[%x],取消操作!\r\n",sDetectHeader->uDetectType);
	}
}
//处理实时数据
void CTelComServer::DealResult()
{
    while(!g_bEndThread)
	{
		std::string response;

	    //加锁
	    pthread_mutex_lock(&m_Result_Mutex);

		//判断是否有命令
		if(m_ResultList.size()>0)
		{
			//取最早命令
			std::list<std::string>::iterator it = m_ResultList.begin();
			//保存数据
			response = *it;
			//删除取出的命令
			m_ResultList.pop_front();
		}
		//解锁
		pthread_mutex_unlock(&m_Result_Mutex);

		//处理消息
		if(response.size()>0)
		{
			OnResult(response);
		}

		//1毫秒
		usleep(1000*1);
	}
}


//读配置文件
void CTelComServer::LoadTelComConfig(map<string,string>& map)
{
	XMLNode xml, temp, setting;
	string xmlFile = "./TelComCenter.xml";
	//判断CameraPara.xml是否存在
	if(access(xmlFile.c_str(),F_OK) != 0)//不存在
	{
		xml = XMLNode::createXMLTopNode("Setting");
		temp = xml.addChild("ProtoVer");
		temp.addText("1.0");

		temp = xml.addChild("DevID");
		temp.addText("");
		temp = xml.addChild("Password");
		temp.addText("");
		temp = xml.addChild("IvmServerIp");
		temp.addText("");
		temp = xml.addChild("IvmServerPort");
		temp.addText("");
		if(!xml.writeToFile(xmlFile.c_str()))
		{
			LogError("生成模板失败!\r\n");
		}
		else
		{
			LogError("没有找到配置文件,生成默认配置文件./TelComCenter.xml\n");
		}
	}
	else
	{
		xml = XMLNode::parseFile(xmlFile.c_str());
		setting = xml.getChildNode("Setting");
		if (!setting.isEmpty())
		{
			temp = setting.getChildNode("ProtoVer");
			if (!temp.isEmpty() && temp.getText())
			{
				map.insert(make_pair("ProtoVer", temp.getText()));
			}
			temp = setting.getChildNode("DevID");
			if (!temp.isEmpty() && temp.getText())
			{
				map.insert(make_pair("DevID", temp.getText()));
			}
			temp = setting.getChildNode("Password");
			if (!temp.isEmpty() && temp.getText())
			{
				map.insert(make_pair("Password", temp.getText()));
			}
			temp = setting.getChildNode("IvmServerIp");
			if (!temp.isEmpty() && temp.getText())
			{
				map.insert(make_pair("IvmServerIp", temp.getText()));
			}
			temp = setting.getChildNode("IvmServerPort");
			if (!temp.isEmpty() && temp.getText())
			{
				map.insert(make_pair("IvmServerPort", temp.getText()));
			}
		}
	}
}

//生成登录报文
string CTelComServer::CreateLoginXml()
{
	//读配置文件
	string protoVer, devID, password;
	map<string,string> config;
	LoadTelComConfig(config);
	map<string,string>::iterator it = config.find("ProtoVer");
	if (it != config.end())
	{
		protoVer = it->second;
	}
	it = config.find("DevID");
	if (it != config.end())
	{
		devID = it->second;
	}
	it = config.find("Password");
	if (it != config.end())
	{
		password = it->second;
	}

	//密码用MD5加密
	//unsigned char digest[TEL_MD5_16];
	//memset(digest, 0, TEL_MD5_16);
 	//GetStringMd5(password, digest);

	//密码转换成十六进制
	//string result;
	//for (int i = 0; i < TEL_MD5_16; i++) 
	//{
	//	char str[3] = {0};
	//	if (digest[i] == 0) {
	//		result.append("00",2);
	//	}
	//	else if (digest[i] <= 15) 	{
	//		sprintf(str,"0%x",digest[i]);
	//		result.append(str,2);
	//	}
	//	else {
	//		sprintf(str,"%x",digest[i]);
	//		result.append(str,2);
	//	}
	//}
	
	XMLNode xml,temp,setting;
	xml = XMLNode::createXMLTopNode("LOGIN_REQ");

    temp = xml.addChild("ProtoVer");
    temp.addText(protoVer.c_str()); // 协议版本号
    temp = xml.addChild("DevID");
    temp.addText(devID.c_str()); // IVU识别码
    temp = xml.addChild("DevPassword");
    temp.addText(password.c_str()); // 密码
	
	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	string strMsg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	strMsg.append(strData, nSize);
    return strMsg;
}

//生成心跳报文
string CTelComServer::CreateLinkXml()
{
	char buf[10] = {0};
	XMLNode xml,temp,setting;
	xml = XMLNode::createXMLTopNode("HEARTBEAT_REQ");
    temp = xml.addChild("CpuPerformance");
	sprintf(buf,"%.0f", g_sysInfo.fCpu); 
    temp.addText(buf);
    temp = xml.addChild("MemoryPerformance");
	memset(buf,0,10);
	sprintf(buf,"%.0f", g_sysInfo.fMemory);
    temp.addText(buf);
    temp = xml.addChild("CurTaskNum");
    temp.addText("1");

    setting = xml.addChild("DynamicAbility");
    temp = setting.addChild("AlgCode");
    temp.addText("101");
    temp = setting.addChild("AlgReservesLine");
    temp.addText("99");

	setting = xml.addChild("DynamicAbility");
	temp = setting.addChild("AlgCode");
	temp.addText("102");
	temp = setting.addChild("AlgReservesLine");
	temp.addText("99");

	setting = xml.addChild("DynamicAbility");
	temp = setting.addChild("AlgCode");
	temp.addText("111");
	temp = setting.addChild("AlgReservesLine");
	temp.addText("99");

	setting = xml.addChild("DynamicAbility");
	temp = setting.addChild("AlgCode");
	temp.addText("112");
	temp = setting.addChild("AlgReservesLine");
	temp.addText("99");

	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	string strMsg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	strMsg.append(strData, nSize);
	return strMsg;
}

//登录信息
void CTelComServer::AnalyseLoginResXml(string strXml, string& result)
{
	XMLNode xml,temp,setting;
	xml = XMLNode::parseString(strXml.c_str());
	if(!xml.isEmpty())
	{
cerr<<"登陆回复="<<endl;
cerr<<xml.createXMLString()<<endl;
		setting = xml.getChildNode("LOGIN_RES");
cerr<<"LOGIN_RES.nChildNode="<<xml.nChildNode("LOGIN_RES")<<endl;
		if(!setting.isEmpty())
		{
			temp = setting.getChildNode("Result");
			if(!temp.isEmpty() && temp.getText())
			{
				result = temp.getText();
			}
		}
	}
}

//生成算法能力上报xml
string CTelComServer::CreateAlgAbilityXml()
{
	XMLNode xml,temp,setting;
	xml = XMLNode::createXMLTopNode("ABILITY_NOTIFY_REQ");
	temp = xml.addChild("VideoSource");
	temp.addText("3");
	for(int i = 0; i < ARRAY_SIZE; i++)
	{
		setting = xml.addChild("AlgAbility");
		temp = setting.addChild("AlgCode");
		temp.addText(AlgArray[i][0].c_str());
		temp = setting.addChild("AlgDesc");
		temp.addText(AlgArray[i][1].c_str());
		temp = setting.addChild("AlgMaxLine");
		temp.addText("1");
		temp = setting.addChild("AlgOther");
		temp.addText("");
	}
	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	string strMsg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	strMsg.append(strData, nSize);
	return strMsg;
}

//解析算法上报回应
void CTelComServer::AnalyseAlgAbilityXml(string strXml, string& result)
{
	XMLNode xml,temp,setting;
	xml = XMLNode::parseString(strXml.c_str());
	if(!xml.isEmpty())
	{
cerr<<"算法能力回复="<<endl;
cerr<<xml.createXMLString()<<endl;
		setting = xml.getChildNode("ABILITY_NOTIFY_RES");
		if(!setting.isEmpty())
		{
cerr<<"ABILITY_NOTIFY_RES.nChildNode="<<xml.nChildNode("ABILITY_NOTIFY_RES")<<endl;
			temp = setting.getChildNode("Result");
			if(!temp.isEmpty() && temp.getText())
			{
				result = temp.getText();
			}
		}
	}
}

//异常上报回应
void CTelComServer::AnalyseExRepXml(string strXml, string& result)
{
	XMLNode xml,temp,setting;
	xml = XMLNode::parseString(strXml.c_str());

	if(!xml.isEmpty())
	{
cerr<<"异常上报回应="<<endl;
cerr<<xml.createXMLString()<<endl;
		setting = xml.getChildNode("EXCEPTION_NOTIFY_RES");
cerr<<"EXCEPTION_NOTIFY_RES.nChildNode="<<xml.nChildNode("EXCEPTION_NOTIFY_RES")<<endl;

		if(!setting.isEmpty())
		{
			temp = setting.getChildNode("Result");
			if(!temp.isEmpty() && temp.getText())
			{
				result = temp.getText();
			}
		}
	}
}

//解析开始分析请求
void CTelComServer::AnalyseDetectStartXml(string strXml, MONITOR_HOST_INFO& info, string& result)
{
	result = "1";
	XMLNode xml,temp,setting;
	xml = XMLNode::parseString(strXml.c_str());
cerr<<"解析智能分析开启请求="<<endl;
cerr<<xml.createXMLString()<<endl;
    if(!xml.isEmpty())
    {
        setting = xml.getChildNode("TASK_DOWN_REQ");
        if(!setting.isEmpty())
        {
            temp = setting.getChildNode("VideoSource");
            if(!temp.isEmpty() && temp.getText())
            {
               if (strcmp(temp.getText(), "6") != 0)
               {
					LogError("失败:智能分析开始请求中VideoSource不支持! VideoSource=%s\n", temp.getText());
					//return;
               }
            }
			else
			{
				LogError("失败:智能分析开始请求中VideoSource为空!\n");
				//return;
			}

			XMLNode GlobleEye = setting.getChildNode("GlobleEye");
			if(!GlobleEye.isEmpty())
			{
				temp = GlobleEye.getChildNode("UserName");
				if(!temp.isEmpty() && temp.getText())
				{
					memcpy(info.chUserName, temp.getText(), strlen(temp.getText()));
				}
				else
				{
					LogError("失败:智能分析开始请求中UserName为空!\n");
					//return;
				}
				temp = GlobleEye.getChildNode("UserPassword");
				if(!temp.isEmpty() && temp.getText())
				{
					memcpy(info.chPassWord, temp.getText(), strlen(temp.getText()));
				}
				else
				{
					LogError("失败:智能分析开始请求中UserPassword为空!\n");
					//return;
				}
				temp = GlobleEye.getChildNode("CmsIP");
				if(!temp.isEmpty() && temp.getText())
				{
					memcpy(info.chMonitorHost, temp.getText(), strlen(temp.getText()));
				}
				else
				{
					LogError("失败:智能分析开始请求中CmsIP为空!\n");
					//return;
				}
				temp = GlobleEye.getChildNode("CmsPort");
				if(!temp.isEmpty() && temp.getText())
				{
					info.uMonitorPort = xmltol(temp.getText());
				}
				else
				{
					LogError("失败:智能分析开始请求中CmsPort为空!\n");
					//return;
				}
				temp = GlobleEye.getChildNode("CmsGuId");
				if(!temp.isEmpty() && temp.getText())
				{
					//memcpy(info.chReserved, temp.getText(), strlen(temp.getText()));
				}
				else
				{
					LogError("失败:智能分析开始请求中CmsGuId为空!\n");
					//return;
				}
			}
			else
			{
				LogError("失败:智能分析开始请求中GlobleEye为空!\n");
				//return;
			}
			
            temp = setting.getChildNode("ResultUpType");
            if(!temp.isEmpty() && temp.getText())
            {
				if (strcmp(temp.getText(), "1") != 0)
				{
					LogError("失败:智能分析开始请求中ResultUpType不支持!目前只支持TCP协议! ResultUpType=%s\n", temp.getText());
					//return;
				}
            }
			else
			{
				LogError("失败:智能分析开始请求中ResultUpType为空!\n");
				//return;
			}
            temp = setting.getChildNode("ResultUpIP");
            if(!temp.isEmpty() && temp.getText())
            {
               m_sResultUpHost = temp.getText();
            }
			else
			{
				LogError("失败:智能分析开始请求中ResultUpIP为空!\n");
				return;
			}
            temp = setting.getChildNode("ResultUpPort");
            if(!temp.isEmpty() && temp.getText())
            {
               m_nResultUpPort = xmltol(temp.getText());
            }
			else
			{
				LogError("失败:智能分析开始请求中ResultUpPort为空!\n");
				return;
			}
            temp = setting.getChildNode("PicDataUpIP");
            if(!temp.isEmpty() && temp.getText())
            {
               m_sDataPicUpHost = temp.getText();
            }
			else
			{
				LogError("失败:智能分析开始请求中PicDataUpIP为空!\n");
				return;
			}
            temp = setting.getChildNode("PicDataUpPort");
            if(!temp.isEmpty() && temp.getText())
            {
               m_nDataPicUpPort = atoi(temp.getText());
            }
			else
			{
				LogError("失败:智能分析开始请求中PicDataUpPort为空!\n");
				return;
			}
            temp = setting.getChildNode("AlgCode");
            if(!temp.isEmpty() && temp.getText())
            {
				LogNormal("智能分析开始请求中算法规则=%s\n", temp.getText());
            }
			else
			{
				LogError("失败:智能分析开始请求中AlgCode为空!\n");
				//return;
			}
			/*
            temp = setting.getChildNode("AlgPolicy");
            if(!temp.isEmpty())
            {
               map.insert(make_pair("AlgPolicy", temp.getText()));
            }
			*/
			temp = setting.getChildNode("TaskSession");
			if(!temp.isEmpty() && temp.getText())
			{
				m_sTaskSession = temp.getText();
			}
			else
			{
				LogError("失败:智能分析开始请求中TaskSession为空!\n");
				return;
			}
        }
		else
		{
			LogError("失败:智能分析开始请求为空!\n");
			return;
		}
    }
	else
	{
		LogError("失败:智能分析开始请求为空!\n");
		return;
	}
	result = "0";
}

//生成开始分析请求回应报文
string CTelComServer::CreateDetectStartRepXml(string result)
{
    XMLNode xml, setting;
	xml = XMLNode::createXMLTopNode("TASK_DOWN_RES");
    
    setting = xml.addChild("Result");
    setting.addText(result.c_str());

	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	string strMsg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	strMsg.append(strData, nSize);
	return strMsg;
}

//解析结束分析请求
void CTelComServer::AnalyseDetectStopXml(string strXml, string& result)
{
	XMLNode xml,temp,setting;
	xml = XMLNode::parseString(strXml.c_str());
    if(!xml.isEmpty())
    {
cerr<<"TASK_STOP_REQ.child="<<xml.nChildNode("TASK_STOP_REQ")<<endl;
        setting = xml.getChildNode("TASK_STOP_REQ");
        if(!setting.isEmpty())
        {
            temp = setting.getChildNode("TaskSession");
			if(!temp.isEmpty() && temp.getText())
			{
				if (strcmp(temp.getText(), m_sTaskSession.c_str()) != 0)
				{
					LogError("失败:智能分析停止请求中TaskSession不匹配! 开始请求中的TaskSession=%s,收到的TaskSession=%s\n", m_sTaskSession.c_str(), temp.getText());
				}
				else
				{
					result = "0";
					return;
				}
			}
        }
    }
}

//生成结束分析请求回应报文
string CTelComServer::CreateDetectStopXml(string result)
{
    XMLNode xml, setting;
	xml = XMLNode::createXMLTopNode("TASK_STOP_RES");
    setting = xml.addChild("Result");
    setting.addText(result.c_str());

	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	string strMsg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	strMsg.append(strData, nSize);
	return strMsg;
}

//解析算法查询要求
void CTelComServer::AnalyseSearchXml(string strXml, string& result)
{
	XMLNode xml,temp,setting;
	xml = XMLNode::parseString(strXml.c_str());
	if(!xml.isEmpty())
	{
cerr<<"算法查询请求="<<endl;
cerr<<xml.createXMLString()<<endl;
		setting = xml.getChildNode("ALG_STRUCT_REQ");
cerr<<"ALG_STRUCT_REQ.nChildNode="<<xml.nChildNode("ALG_STRUCT_REQ")<<endl;
		if(!setting.isEmpty())
		{
			temp = setting.getChildNode("AlgCode");
			if(!temp.isEmpty() && temp.getText())
			{
				result = temp.getText();
			}
		}
	}
}

//生成算法查询xml
string CTelComServer::CreateSearchXml(string algCode)
{
	string ret = "";
	int i = 0;
	for(; i < ARRAY_SIZE; i++)
	{
		if (!AlgArray[i][0].compare(algCode))
		{
			ret = "0";
			break;
		}
	}
	if (i >= ARRAY_SIZE)
	{
		ret = "1";
	}

	XMLNode xml,temp,setting;
	xml = XMLNode::createXMLTopNode("ALG_STRUCT_RES");
	temp = xml.addChild("Result");
	temp.addText(ret.c_str());
	setting = xml.addChild("ResultDesc");
	temp = setting.addChild("TagID");
	temp.addText(ret.compare("0")? "":algCode.c_str());
	temp = setting.addChild("TagDesc");
	temp.addText(ret.compare("0")? "":AlgArray[i][1].c_str());
	temp = setting.addChild("DataType");
	temp.addText("2");

	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	string strMsg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	strMsg.append(strData, nSize);
	return strMsg;
}


//生成异常数据报文
//输入参数vec: vec[0]异常发生时间；vec[1]异常描述
string CTelComServer::CreateExNotifyXml(const vector<string>& vec)
{
	XMLNode xml, setting;
	xml = XMLNode::createXMLTopNode("EXCEPTION_NOTIFY_REQ");
	setting = xml.addChild("TaskSession");
	setting.addText("异常任务ID");
	setting = xml.addChild("Time");
	setting.addText(vec[0].c_str());
	setting = xml.addChild("ExceptionDesc");
	setting.addText(vec[1].c_str());

	int nSize;
	XMLSTR strData = xml.createXMLString(1, &nSize);
	string strMsg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	strMsg.append(strData, nSize);
	return strMsg;
}

//获取MD5验证码
void CTelComServer::GetStringMd5(string str, unsigned char* digest)
{
	unsigned char buf[str.size()];
	memcpy(buf, str.c_str(), str.size());
	MD5_CTX md5;
	md5.MD5Update(buf, str.size());
	md5.MD5Final(digest);
}