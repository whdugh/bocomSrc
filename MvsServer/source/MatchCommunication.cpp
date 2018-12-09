// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "Common.h"
#include "CommonHeader.h"
#include "MatchCommunication.h"
#include "XmlParaUtil.h"
#include "FeatureSearch.h"

CMatchCommunication g_MatchCommunication;


//记录发送线程
void* ThreadMatchResult(void* pArg)
{
	//处理一条数据
	g_MatchCommunication.DealResult();

    pthread_exit((void *)0);
	return pArg;
}

//历史记录发送线程
void* ThreadMatchHistoryResult(void* pArg)
{
	//处理一条数据
	CMatchCommunication *pMatch = (CMatchCommunication *)pArg;
	if (pMatch)
	{
	//	LogTrace("ams_communicain.log","begin to deal one record.");
		pMatch->DealHistoryResult();
	}

    pthread_exit((void *)0);
	return pArg;
}

CMatchCommunication::CMatchCommunication()
{
    m_bCenterLink = false;
    m_nCenterSocket = 0;
    m_nCSLinkCount = 0;
    m_nThreadId = 0;
    m_nHistoryThreadId = 0;

    pthread_mutex_init(&m_Result_Mutex,NULL);
}


CMatchCommunication::~CMatchCommunication()
{
    pthread_mutex_destroy(&m_Result_Mutex);
}

//启动侦听服务
bool CMatchCommunication::Init()
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
	int nret=pthread_create(&m_nThreadId,&attr,ThreadMatchResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建检测结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}

	/*nret=pthread_create(&m_nHistoryThreadId,&attr,ThreadMatchHistoryResult,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建历史结果发送线程失败,服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}*/

	pthread_attr_destroy(&attr);

	return true;
}

//释放
bool CMatchCommunication::UnInit()
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

    m_ResultList.clear();
	return true;
}

/*
* 函数介绍：主线程调用接口，断开重连或发送心跳及检查历史记录
* 输入参数：无
* 输出参数：无
* 返回值 ：无
*/
void CMatchCommunication::mvConnOrLinkTest()
{
    if (!g_bEndThread)
    {
        if (!m_bCenterLink)
        {
                if (mvConnCSAndRecvMsg())
                {
                    LogNormal("连接比对服务器成功!\n");
                    m_bCenterLink = true;
                    m_nCSLinkCount = 0;
                }
        }
        else
        {         
			
        }
    }
}

/*
* 函数介绍：连接到中心并开启接收消息线程
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CMatchCommunication::mvConnCSAndRecvMsg()
{
    //connect to center server;
    if (!mvConnectToCS())
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
bool CMatchCommunication::mvConnectToCS()
{
    //connect to center server and set socket's option.
    string strMatchHost(g_MatchHostInfo.chMatchHost);
    int nMatchPort = g_MatchHostInfo.uMatchPort;

    if (strMatchHost.empty() || strMatchHost == "0.0.0.0" || nMatchPort <= 0)
    {
        //printf("\n中心数据服务器连接参数异常:host=%s,port=%d\n", strMatchHost.c_str(), nMatchPort);
        return false;
    }

    if (!mvPrepareSocket(m_nCenterSocket))
    {
        //printf("\n准备连接中心数据服务器套接字失败!\n");
        return false;
    }

    if (!mvWaitConnect(m_nCenterSocket, strMatchHost, nMatchPort,2))
    {
        //printf("\n尝试连接中心数据服务器失败!\n");
        return false;
    }

    return true;
}


/*
* 函数介绍：发送心跳测试
* 输入参数：无
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CMatchCommunication::mvSendLinkTest()
{
	if(!m_bCenterLink)
	{
		return false;
	}

    return true;
}


/*
* 函数介绍：发送记录到中心
* 输入参数：strMsg-消息内容；bRealTime-是否是实时记录
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CMatchCommunication::mvSendRecordToCS(const string &strMsg)
{
    if(!m_bCenterLink)
    {
        return false;
    }
	
	//LogNormal("mvSendRecordToCS strMsg.size()=%d",strMsg.size());
    return mvRebMsgAndSend(m_nCenterSocket,strMsg);
}

/*
* 函数介绍：重组消息并发送
* 输入参数：pCode-要发送的消息类型；strMsg-要发送的消息核心内容
* 输出参数：无
* 返回值 ：成功返回true，否则返回false
*/
bool CMatchCommunication::mvRebMsgAndSend(int& nSocket,const string &strMsg)
{
    //rebuild the message according to the msg-code and then send it to the center server.
    string strFullMsg("");

    FEATURE_DETECT_HEADER* sDetectHeader = (FEATURE_DETECT_HEADER *)strMsg.c_str();

    MATCH_MSG_HEADER mHeader;
    mHeader.uCameraID = sDetectHeader->uCameraID;
    mHeader.uCmdID = sDetectHeader->uCmdID;
	
	printf("mvRebMsgAndSend strMsg.size()=%d\n",strMsg.size());
    string strXmlResult;
    if(strMsg.size() > sizeof(sDetectHeader))
    {
		strXmlResult.append(strMsg.c_str()+sizeof(FEATURE_DETECT_HEADER),strMsg.size()-sizeof(FEATURE_DETECT_HEADER));
    }
    mHeader.uCmdLen = sizeof(MATCH_MSG_HEADER)+strXmlResult.size();
    strFullMsg.append((char*)&mHeader,sizeof(MATCH_MSG_HEADER));

    strFullMsg += strXmlResult;

    if (!mvSendMsgToSocket(nSocket, strFullMsg))
    {
        mvCloseSocket(nSocket);
        m_bCenterLink = false;
        LogError("发送特征信息失败，连接断开\n");
        return false;
    }
	printf("mvSendMsgToSocket strFullMsg.size()=%d\n",strFullMsg.size());

    return true;
}

//添加一条数据
bool CMatchCommunication::AddResult(std::string& strResult)
{
    //加锁
    pthread_mutex_lock(&m_Result_Mutex);

    m_ResultList.push_front(strResult);
    //解锁
    pthread_mutex_unlock(&m_Result_Mutex);

	return true;
}

//处理检测结果
bool CMatchCommunication::OnResult(std::string& result)
{
    //发送数据
    if (mvSendRecordToCS(result))
    {
        return true;
    }

    return false;
}

//处理实时数据
void CMatchCommunication::DealResult()
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
			Match_Result::iterator it = m_ResultList.begin();
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

//处理历史数据
void CMatchCommunication::DealHistoryResult()
{

}

//生成特征数据
void CMatchCommunication::CreateXmlMsg(string& strXmlResult,const string& strMsg)
{
    
}

//输出特征特征标定信息
void CMatchCommunication::OutPutCalibration(CvRect& farrect,CvRect& nearrect,SRIP_DETECT_HEADER* sDetectHeader)
{
	if(farrect.width <= 0 || nearrect.width <= 0 || farrect.height <= 0|| nearrect.height <= 0)
	{
		return;
	}

	string strResult;
	
	sDetectHeader->uDetectType = AMS_FEATURE_CALIBRATION;
	
	strResult.append((char*)sDetectHeader,sizeof(SRIP_DETECT_HEADER));

	XMLNode FeatureInfoNode,WidthNode,HeightNode,PosNode,TempNode;

	char buf[256] = {0};

	FeatureInfoNode = XMLNode::createXMLTopNode("FeatureCalibration");

	WidthNode = FeatureInfoNode.addChild("Width");
	sprintf(buf,"%d",sDetectHeader->uWidth);
	WidthNode.addText(buf);

	HeightNode = FeatureInfoNode.addChild("Height");
	sprintf(buf,"%d",sDetectHeader->uHeight);
	HeightNode.addText(buf);

	PosNode = FeatureInfoNode.addChild("NearPos");
	TempNode = PosNode.addChild("PosX");
	sprintf(buf,"%d",nearrect.x);
	TempNode.addText(buf);
	TempNode = PosNode.addChild("PosY");
	sprintf(buf,"%d",nearrect.y);
	TempNode.addText(buf);
	TempNode = PosNode.addChild("PosW");
	sprintf(buf,"%d",nearrect.width);
	TempNode.addText(buf);
	TempNode = PosNode.addChild("PosH");
	sprintf(buf,"%d",nearrect.height);
	TempNode.addText(buf);

	PosNode = FeatureInfoNode.addChild("FarPos");
	TempNode = PosNode.addChild("PosX");
	sprintf(buf,"%d",farrect.x);
	TempNode.addText(buf);
	TempNode = PosNode.addChild("PosY");
	sprintf(buf,"%d",farrect.y);
	TempNode.addText(buf);
	TempNode = PosNode.addChild("PosW");
	sprintf(buf,"%d",farrect.width);
	TempNode.addText(buf);
	TempNode = PosNode.addChild("PosH");
	sprintf(buf,"%d",farrect.height);
	TempNode.addText(buf);
	
	string strResultBody;
	int nSize;
	XMLSTR strData = FeatureInfoNode.createXMLString(1, &nSize);
	if(strData)
	{
		strResultBody.append(strData, sizeof(XMLCHAR)*nSize);
		freeXMLString(strData);

		strResult += strResultBody;
	}

	g_AMSCommunication.AddResult(strResult);


	//需要给次控服务器发送一份
	strResult.clear();
	FEATURE_DETECT_HEADER header;
	header.uCameraID = sDetectHeader->uChannelID;
	header.uCmdID = FEATURE_CALIBRATION;

	strResult.append((char*)&header,sizeof(FEATURE_DETECT_HEADER));
	strResult += strResultBody;
	AddResult(strResult);
}
