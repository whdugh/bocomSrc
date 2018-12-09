// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "RoadChannelCenter.h"
#include "Common.h"
#include "CommonHeader.h"
#include "CenterServerOneDotEight.h"
#include "XingTaiCenter.h"
#include "YiChangCenter.h"
#include "SJServer.h"
#include "WuHanCenter.h"
#include "BXServer.h"
#ifdef YUHUANCENTER_OK
#include "YuHuanCenter.h"
#endif

//#define DEBUG_CHANNELCENTER
//#define _DEBUG


//全局调用
CSkpChannelCenter g_skpChannelCenter;


//监控线程
void* ThreadDetectResult(void* pArg)
{
	//取类指针
	CSkpChannelCenter* pChannelCenter = (CSkpChannelCenter*)pArg;
	if(pChannelCenter == NULL) return pArg;

	//处理一条数据
	pChannelCenter->DealMsg();
    pthread_exit((void *)0);
	return pArg;
}

//构造
CSkpChannelCenter::CSkpChannelCenter()
{
	//线程ID
	m_nThreadId = 0;
	//m_nMutiThreadId=0;
	m_bRealTimeEvent = true;
	m_nEventKind = 0;
	m_nPlateKind = 0;
	m_bRealTimeStat = true;
	m_bRealTimeLog = true;
	m_bRealTimePlate = true;
	//结果列表互斥
	pthread_mutex_init(&m_Pic_Mutex,NULL);
	pthread_mutex_init(&m_Result_Mutex,NULL);
	//通道实体互斥
	pthread_mutex_init(&m_Entity_Mutex,NULL);
	return;
}
	//析构
CSkpChannelCenter::~CSkpChannelCenter()
{
	//结果列表互斥
	pthread_mutex_destroy(&m_Pic_Mutex);
	pthread_mutex_destroy(&m_Result_Mutex);
	//通道实体互斥
	pthread_mutex_destroy(&m_Entity_Mutex);
	return;
}

//初始化
bool CSkpChannelCenter::Init()
{
	//从DB初始化通道列表
	g_skpDB.InitChannelList();
//#ifdef _DEBUG
	LogNormal("初始化通道列表成功!\r\n");
//#endif

	//启动数据处理线程
	if(!BeginMsgThread())
		return false;
	return true;
}
	//释放
bool CSkpChannelCenter::UnInit()
{
    printf("CSkpChannelCenter::UnInit\n");
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	//停止所有的检测动作
	ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
	while(it != m_ChannelEntityMap.end())
	{
	    if(it->second != NULL)
	    {
            if(it->second->IsOpen())
            {
                //停止检测
                it->second->EndDetect();
                //释放空间
                it->second->Close();
            }
            delete it->second;
	    }
		it ++;
	}
	//清空列表
	m_ChannelEntityMap.clear();

	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
	//停止数据处理线程
	EndMsgThread();

	return true;
}


//启动数据处理线程
bool CSkpChannelCenter::BeginMsgThread()
{
    printf("====== CSkpChannelCenter::BeginMsgThread()===\n");
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	//启动监控线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadDetectResult,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建通信检测结果处理线程失败,服务无法处理通道数据！\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}
    pthread_attr_destroy(&attr);
	return true;
}

//停止处理线程
void CSkpChannelCenter::EndMsgThread()
{
	//停止线程
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}
//	if(m_nMutiThreadId != 0)
//	{
//		pthread_join(m_nMutiThreadId,NULL);
//		m_nMutiThreadId = 0;
//	}
	m_ChannelPicList.clear();
	m_ChannelResultList.clear();
	return;
}

//处理数据
void CSkpChannelCenter::DealMsg()
{
    printf("=========CSkpChannelCenter::DealMsg()=========\n");
    //
	while(!g_bEndThread)
	{
		std::string response1,response2;
		//////////////////////////////////////////////////////////先取检测
	    //加锁
	    pthread_mutex_lock(&m_Result_Mutex);

		//判断是否有命令
		if(m_ChannelResultList.size()>0)
		{
			//取最早命令
			ChannelMsg::iterator it = m_ChannelResultList.begin();
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

        //////////////////////////////////////////////////////////////////再取图片
		//加锁
	    pthread_mutex_lock(&m_Pic_Mutex);
		//判断是否有命令
		if(m_ChannelPicList.size()>0)
		{
			//取最早命令
			ChannelMsg::iterator it = m_ChannelPicList.begin();
			//保存数据
			response2 = *it;
			//删除取出的命令
			m_ChannelPicList.pop_front();
		}
		//解锁
		pthread_mutex_unlock(&m_Pic_Mutex);

		//处理消息
		if(response2.size()>0)
		{
			OnResult(response2);
		}

        //////////////
        /*FILE * fp = fopen("ChannelCenter.txt","wb");
        fprintf(fp,"m_ChannelResultList.size=%d,m_ChannelPicList.size=%d\n",m_ChannelResultList.size(),m_ChannelPicList.size());
        fclose(fp);*/
        //////////////

		//1毫秒
		usleep(1000*1);
	}
}

//获取检测结果队列大小
int CSkpChannelCenter::GetResultListSize()
{
    int nResultListSize = 0;
    //加锁
    pthread_mutex_lock(&m_Result_Mutex);

    nResultListSize = m_ChannelResultList.size();

    //解锁
    pthread_mutex_unlock(&m_Result_Mutex);

    return nResultListSize;
}

//添加一条数据,普通未检测图片
bool CSkpChannelCenter::AddResult(std::string& sResult)
{
    //printf("==========CSkpChannelCenter::AddResult==\n");
	//添加到列表
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)sResult.c_str();

	switch(sDetectHeader->uDetectType)
	{
       case SRIP_NORMAL_PIC:	//普通未检测图片(送客户端或中心端)
	        //加锁
	        pthread_mutex_lock(&m_Pic_Mutex);

	        //
	      /*  string result;
	        EncodeBase64(result,(unsigned char*)sResult.c_str(),sResult.size());
	        m_ChannelPicList.push_back(result);*/
			m_ChannelPicList.push_back(sResult);
			//解锁
	        pthread_mutex_unlock(&m_Pic_Mutex);
			break;
	   case SRIP_DETECT_EVENT:	//事件检测结果(送客户端)
	   case SRIP_CARD_RESULT://车牌检测结果
	   case SRIP_DETECT_STATISTIC://统计
	   case SRIP_CHANNEL_STATUS://通道状态
	   case SRIP_LOOP_SIGNAL://线圈信号
	   case SRIP_OBJECT_STATUS: //客户端绘图
	        //加锁
	        pthread_mutex_lock(&m_Result_Mutex);
			m_ChannelResultList.push_back(sResult);
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
bool CSkpChannelCenter::OnResult(std::string& result)
{
	//发送数据
	MIMAX_HEADER mHeader;
	//取类型
	SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)result.c_str();

	switch(sDetectHeader->uDetectType)
	{
		case SRIP_NORMAL_PIC:	//图片(送客户端)
			 ///////////////////////////////////////////
		{
		    //printf("==========result.size=%d=====\n", result.size());

			 mHeader.uCmdID = (MIMAX_FRAME);			//图片帧
			 mHeader.uCmdFlag = 0x00000001;
			 mHeader.uCameraID = g_nCameraID;
			 #ifdef DEBUG_CHANNELCENTER
			 printf("                                                                OnPic\r\n");
			 printf("                                                                  %d\r\n",sDetectHeader->uSeq);
			 #endif
			 break;
		}
		case SRIP_DETECT_EVENT:	//事件检测结果(送客户端)
		case SRIP_DETECT_STATISTIC:
			mHeader.uCmdID = (SRIP_FRAME_RESULT);	//事件检测结果
			#ifdef DEBUG_CHANNELCENTER
			printf("                                                                         OnEvent\r\n");
			printf("                                                                           %d\r\n",sDetectHeader->uSeq);
			#endif
			break;
        case SRIP_CARD_RESULT://车牌检测结果(送客户端)
		    mHeader.uCmdID = sDetectHeader->uDetectType;
			#ifdef DEBUG_CHANNELCENTER
			printf("                                                                         OnCardEvent\r\n");
			printf("                                                                           %d\r\n",sDetectHeader->uSeq);
			#endif
		    break;
        case SRIP_CHANNEL_STATUS://往客户端发送通道状态
            {
                mHeader.uCmdID = sDetectHeader->uDetectType;
                result.erase(0,sizeof(SRIP_DETECT_HEADER));
            }
            break;
        case SRIP_LOOP_SIGNAL://往客户端发送线圈信号
            {
                mHeader.uCmdID = sDetectHeader->uDetectType;
            }
            break;
		case SRIP_OBJECT_STATUS://客户端绘图
			{
				mHeader.uCmdID = sDetectHeader->uDetectType;
			}
			break;
		default:
			LogError("未知数据[%d],取消操作!\r\n",sDetectHeader->uChannelID);
			return false;
	}

/*
	//存图
	if(sDetectHeader->uDetectType == SRIP_NORMAL_PIC)
	{
	    printf("=======save pic=========\n");
	    FILE *fpOut;
            char jpg_name[256] = {0};
            sprintf(jpg_name, "./jpg/%d_qq.jpg",sDetectHeader->uSeq);
            fpOut = fopen(jpg_name,"wb");
            //fwrite(pBuffer + sizeof(yuv_video_buf) + sizeof(Image_header_dsp), 1, header.size - sizeof(Image_header_dsp), fpOut);
            fwrite(result.c_str() + sizeof(SRIP_DETECT_HEADER) , 1, result.size() - sizeof(SRIP_DETECT_HEADER) , fpOut);
            fclose(fpOut);
	}
*/



    //数据长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + result.size();
	//添加头
	result.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));


    //发送数据
    if(g_skpRoadServer.SendToClient(result))
    {
        return true;
    }

    return false;
}

//添加一个通道实体
bool CSkpChannelCenter::AddChannel(SRIP_CHANNEL sChannel,bool bStart,bool bConnect)
{
    ///////////////////////////
	//生成通道对象
	//CSkpChannelEntity entity;
	CSkpChannelEntity* pCSkpChannelEntity = new CSkpChannelEntity;
	//通道ID
	pCSkpChannelEntity->SetChannelID(sChannel.uId);
	//通道地点
	pCSkpChannelEntity->SetChannelLocation(sChannel.chPlace);
	//通道方向
    pCSkpChannelEntity->SetChannelDirection(sChannel.uDirection);
	//相机型号
	pCSkpChannelEntity->SetCameraType(sChannel.nCameraType);
	//相机编号
	pCSkpChannelEntity->SetCameraID(sChannel.nCameraId);
	g_nCameraID = sChannel.nCameraId;

	//设备ID
	pCSkpChannelEntity->SetDeviceID(sChannel.chDeviceID);

	//设置通道工作方式
	pCSkpChannelEntity->SetChannelWorkMode(sChannel.nWorkMode);

	//相机IP
	pCSkpChannelEntity->SetCameraHost(sChannel.chCameraHost);
	//g_CameraHost = sChannel.chCameraHost;

	//断面编号
	pCSkpChannelEntity->SetPannelID(sChannel.nPannelID);
	//监测器编号
    pCSkpChannelEntity->SetMonitorID(sChannel.nMonitorId);
    //视频编号
    pCSkpChannelEntity->SetVideoID(sChannel.nVideoIndex);
	//通道类型
	pCSkpChannelEntity->SetChannelType(sChannel.eType);
	//通道格式
	pCSkpChannelEntity->SetChannelFormat(sChannel.eVideoFmt);
	//通道事件录像时间
	pCSkpChannelEntity->SetChannelEventCaptureTime(sChannel.uEventCaptureTime);
	//通道视频参数,添加通道时视频参数默认为50%/////////////////////
	//亮度
	pCSkpChannelEntity->SetChannelBrightness(0);
	//对比度
	pCSkpChannelEntity->SetChannelContrast(0);
	//饱和度
	pCSkpChannelEntity->SetChannelSaturation(0);
	//色调
	pCSkpChannelEntity->SetChannelHue(0);


	//录像类型
	pCSkpChannelEntity->SetChannelCaptureType(sChannel.eCapType);
	//录像开始时间
	pCSkpChannelEntity->SetChannelCapBeginTime(sChannel.uCapBeginTime);
	//录像结束时间
	pCSkpChannelEntity->SetChannelCapEndTime(sChannel.uCapEndTime);

	//YUV参数
	//YUV主机
	std::string strMonitorHost(sChannel.chMonitorHost);
	pCSkpChannelEntity->SetChannelYuvHost(strMonitorHost);
	//YUV端口
	pCSkpChannelEntity->SetChannelYuvPort(sChannel.uMonitorPort);


	pCSkpChannelEntity->SetChannelDetectKind(sChannel.uDetectKind);

	pCSkpChannelEntity->SetChannelDetectTime(sChannel.uDetectTime);

	pCSkpChannelEntity->SetEventCapture(sChannel.bEventCapture);
	pCSkpChannelEntity->SetYUVFormat(sChannel.nYUVFormat);

    pCSkpChannelEntity->SetVideoBeginTime(sChannel.uVideoBeginTime);
    pCSkpChannelEntity->SetVideoEndTime(sChannel.uVideoEndTime);

	pCSkpChannelEntity->SetChannelFileName(sChannel.chFileName);

	pCSkpChannelEntity->SetConnect(bConnect);

    //加锁
	pthread_mutex_lock(&m_Entity_Mutex);
    LogTrace(NULL, "===before m_ChannelEntityMap.insert\n");
	//保存到映射
	if(m_ChannelEntityMap.find(sChannel.uId)==m_ChannelEntityMap.end())
	{
	    printf("==111=before m_ChannelEntityMap.insert\n");
        m_ChannelEntityMap.insert(ChannelEntityMap::value_type(sChannel.uId,pCSkpChannelEntity));
        printf("==222==after m_ChannelEntityMap.insert\n");
	}
	else
	{
	    pthread_mutex_unlock(&m_Entity_Mutex);
        return false;
	}

    LogTrace(NULL, "====after m_ChannelEntityMap.insert\n");

//printf("打开设备成功，开始采集数据[%d] \r\n",sChannel.uId);

	//是否需要启动
	if(bStart)
	{
    #ifdef _DEBUG
        printf("=======begin Run channnel=====\n");
    #endif
		//需要启动
		ChannelEntityMap::iterator it = m_ChannelEntityMap.find(sChannel.uId);

		//printf("=======it->second->m_RoadCarnumDetect.m_ftVlp.m_pBgStatistic =%d ===\n", it->second->m_RoadCarnumDetect.m_ftVlp.m_pBgStatistic);
    #ifdef _DEBUG
        printf("=======after m_ChannelEntityMap.find==nChannel =%d ===\n", sChannel.uId);
    #endif
		if(it != m_ChannelEntityMap.end())
		{
		    bool bOpenOk = false; //是否打开成功

        #ifdef _DEBUG
            printf("=======begin Open==nChannel =%d ===\n", sChannel.uId);
        #endif

            bOpenOk = it->second->Open();

        #ifdef _DEBUG
            printf("=======after Open=111====\n");
        #endif

			//启动通道实体对象
			//if(it->second->Open())
			if(bOpenOk)
			{
				#ifdef _DEBUG
                LogNormal("打开设备成功，开始采集数据[%d] \r\n",sChannel.uId);
				#endif
				//打开设备，开始检测
				it->second->BeginDetect();
			}
			else
			{
			    #ifdef _DEBUG
                    LogError("打开设备成功! nChannel = [%d] \r\n", sChannel.uId);
			    #endif
			}
        #ifdef _DEBUG
            printf("=======after Open=222====\n");
        #endif
		}

    #ifdef _DEBUG
        printf("=======after Run channnel=====\n");
    #endif
	}
	//g_WhichContinuousSet();
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

	return true;
}

//修改通道
bool CSkpChannelCenter::ModifyChannel(SRIP_CHANNEL sChannel)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(sChannel.uId);
	if(it != m_ChannelEntityMap.end())
	{
		//找到实体
		//通道地点
		it->second->SetChannelLocation(sChannel.chPlace);
		//通道方向
		bool bModifyDirection =  (it->second->GetChannelDirection() != sChannel.uDirection);
		it->second->SetChannelDirection(sChannel.uDirection);
        //相机型号
        bool bModifyCameraType = it->second->GetCameraType()!=sChannel.nCameraType;
		//相机编号
		//it->second->SetCameraID(sChannel.nCameraId);
		bool bModifyCameraID = it->second->GetCameraID()!=sChannel.nCameraId;
		g_nCameraID = sChannel.nCameraId;

		//设备Id
		//printf("--id:%s, id-new:%s", (it->second->GetDeviceID()).c_str(), sChannel.chDeviceID);
		String strDeviceId = sChannel.chDeviceID;
		it->second->SetDeviceID(strDeviceId);

		//断面编号
		it->second->SetPannelID(sChannel.nPannelID);
		//监测器编号
        it->second->SetMonitorID(sChannel.nMonitorId);
        //视频编号
        //it->second->SetVideoID(sChannel.nVideoIndex);
        bool bModifyVideoID = it->second->GetVideoID()!=sChannel.nVideoIndex;
		//通道类型
		it->second->SetChannelType(sChannel.eType);
		//判断视频格式是否改变
		bool bModifyFormat = it->second->GetChannelFormat() != sChannel.eVideoFmt;
		//通道事件录像时间
		it->second->SetChannelEventCaptureTime(sChannel.uEventCaptureTime);

		//录像参数
        //判断录象类型是否改变
        bool bModifyCapType = it->second->GetChannelCaptureType() != sChannel.eCapType;
		//录像开始时间
		it->second->SetChannelCapBeginTime(sChannel.uCapBeginTime);
		//录像结束时间
		it->second->SetChannelCapEndTime(sChannel.uCapEndTime);

		//YUV参数

		//YUV主机
		std::string strMonitorHost(sChannel.chMonitorHost);
		it->second->SetChannelYuvHost(strMonitorHost);
		//YUV端口
		it->second->SetChannelYuvPort(sChannel.uMonitorPort);

		bool bModifyYUVFormat = it->second->GetYUVFormat() != sChannel.nYUVFormat;

		bool bModifyVideoBeginTime = it->second->GetVideoBeginTime() != sChannel.uVideoBeginTime;
        bool bModifyVideoEndTime = it->second->GetVideoEndTime() != sChannel.uVideoEndTime;

        bool bModeifyCameraHost = it->second->GetCameraHost() != sChannel.chCameraHost;

        if(bModeifyCameraHost)
        {
			std::string strCameraHost(sChannel.chCameraHost);
            it->second->SetCameraHost(strCameraHost);
        }

		bool bModifyWokeMode = it->second->GetChannelWorkMode() != sChannel.nWorkMode;

        printf("sChannel.uVideoBeginTime=%lld,sChannel.uVideoEndTime=%lld\n",sChannel.uVideoBeginTime,sChannel.uVideoEndTime);

        //相邻同步主机
		std::string strSynHost(sChannel.chSynHost);
		it->second->SetChannelSynHost(strSynHost);
		it->second->SetChannelSynPort(sChannel.uSynPort);

		if(g_nMultiPreSet == 1)//存在多个预置位
		{
			CXmlParaUtil xml;
			int nPreSetDetectKind = xml.LoadPreSetDetectKind(sChannel.uId,sChannel.nPreSet);
			if(nPreSetDetectKind != 0)
			{
				sChannel.uDetectKind = (CHANNEL_DETECT_KIND)nPreSetDetectKind;
			}
		}

        //检测类型
		bool bChangeDetectKind = it->second->GetChannelDetectKind() != sChannel.uDetectKind;
        //检测时间
		it->second->SetChannelDetectTime(sChannel.uDetectTime);

        //文件名或目录名
		bool bChangeFileName = false;
		if(sChannel.eVideoFmt==VEDIO_YUV_FILE ||
           sChannel.eVideoFmt== VEDIO_AVI||
           sChannel.eVideoFmt==VEDIO_PICLIB||
           sChannel.eVideoFmt==VEDIO_H264||
           sChannel.eVideoFmt==VEDIO_MJPEG||
           sChannel.eVideoFmt==VEDIO_H264_FILE||
           sChannel.eVideoFmt==VEDIO_H264_FILE_N)
		{
		    std::string strFileName = it->second->GetChannelFileName();
		    std::string strNewFileName(sChannel.chFileName);

		    printf("strFileName=%s,strNewFileName=%s\r\n",strFileName.c_str(),strNewFileName.c_str());
			if(strFileName!= strNewFileName)
			{
				bChangeFileName = true;
			}
		}

		////工作模式
		//bool bChangeWorkmode = false;
		//int Workmode = it->second->GetChannelWorkmode();
		//int newWorkmode = sChannel.nWorkMode;
		//int channelId = it->second->GetChannelID();
		//if(Workmode != newWorkmode)
		//{
		//	bChangeWorkmode = true;
		//	//从触发模式变为连续模式
		//	/*if((Workmode == 1) && (newWorkmode == 0))
		//	{
		//	//AutoChangeWorkMode(channelId);//自动把别的通道变成触发模式
		//	}*/
		//}

		bool bRestart = false;
        printf("bModifyFormat=%d,bChangeFileName=%d,bModifyCameraType=%d,bModifyVideoID=%d,bModifyCameraID=%d,bModifyYUVFormat=%d,bModifyVideoBeginTime=%d,bModifyVideoEndTime=%d,bModifyCapType=%d,bChangeDetectKind=%d\r\n",bModifyFormat,bChangeFileName,bModifyCameraType,bModifyVideoID,bModifyCameraID,bModifyYUVFormat,bModifyVideoBeginTime,bModifyVideoEndTime,bModifyCapType,bChangeDetectKind);
		//视频源改变、相机类型改变、文件名称改变、相机模式改变需要重新启动通道处理
		if(bModifyFormat||bChangeFileName||bModifyCameraType||
           bModifyVideoID||bModifyCameraID||bModifyYUVFormat||
           bModifyVideoBeginTime||bModifyVideoEndTime ||
           bModeifyCameraHost||bModifyWokeMode)
		{
		    LogNormal("==Restart channel!!=\n");
		    it->second->Restart(sChannel);
			bRestart = true;
		}

        //录象类型改变
        if(bModifyCapType)
        {
            it->second->RestartCapture(sChannel.eCapType);
        }

        //检测类型改变或相机工作模式改变需要重启算法检测模块
//        if(bChangeDetectKind || bModifyWokeMode)
        if(bChangeDetectKind && (!bRestart))
        {
            it->second->RestartDetect(sChannel.uDetectKind);
        }

        //获取相机状态
        sChannel.bRun = it->second->GetStatus();
        //获取通道状态
        sChannel.nCameraStatus = it->second->GetCameraStatus();
        //事件录像改变需要重启事件录像线程
        it->second->ModifyEventCapture(sChannel.bEventCapture);
	}
	else
	{
	    // what would you like to do with it if this certain channel doesn't exist,
	    // return false or add another new channel with this channel ID?
        pthread_mutex_unlock(&m_Entity_Mutex);
	    return false;
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

	return true;
}

//删除通道
bool CSkpChannelCenter::DeleteChannel(SRIP_CHANNEL sChannel)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(sChannel.uId);
	if(it != m_ChannelEntityMap.end())
	{
	    if(it->second != NULL)
	    {
            //找到实体
            if(it->second->IsOpen())
            {
                //停止检测
                it->second->EndDetect();
                //释放空间
                it->second->Close();
            }
            delete it->second;
        }
		//删除实体
		m_ChannelEntityMap.erase(it);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
	//g_WhichContinuousSet();
	return true;
}

//设置通道视频参数
bool CSkpChannelCenter::SetVideoParams(SRIP_CHANNEL_ATTR& sAttr)
{
	bool bResult = false;
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(sAttr.uId);
	if(it != m_ChannelEntityMap.end())
	{
		//找到实体,设置视频参数
		bResult = it->second->SetChannelParams(sAttr);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
	return bResult;
}

//重新加载通道检测参数
void CSkpChannelCenter::ReloadChannelParaMeter(SRIP_CHANNEL_EXT& sChannel,bool bEventParam)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	int nChannelID = sChannel.uId;
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelID);
	if(it != m_ChannelEntityMap.end())
	{
           //重新加载车道检测参数
		   it->second->SetReloadConfig(true);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//重新加载车道坐标参数
void CSkpChannelCenter::ReloadDetect(int nChannelID)
{
    //加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelID);
	if(it != m_ChannelEntityMap.end())
	{
		   it->second->SetReloadConfig(false);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

void CSkpChannelCenter::PauseChannel(int nChannelID)
{
	LogNormal("==CSkpChannelCenter::PauseChannel()=ID[%d]\n", nChannelID);
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);

	if(nChannelID<=0)//暂停所有通道(认证不成功时执行此操作)
	{
        ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
        while(it != m_ChannelEntityMap.end())
        {
            if(it->second->IsOpen())
            {
                //停止检测
                it->second->EndDetect();
                //释放空间
                it->second->Close();
                #ifdef DEBUG_CHANNELCENTER
                   LogNormal("暂停通道成功!\r\n");
                #endif

                nChannelID = it->second->GetChannelID();
                g_skpDB.UpdateChannelStatus(nChannelID,PAUSE_STATUS);
                it->second->SetStatus(PAUSE_STATUS);
            }
            it++;
        }
	}
	else
	{
        ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelID);
        if(it != m_ChannelEntityMap.end())
        {
            if(it->second->IsOpen())
            {
                //停止检测
                it->second->EndDetect();
                //释放空间
                it->second->Close();
                #ifdef DEBUG_CHANNELCENTER
                   LogNormal("暂停通道成功!\r\n");
                #endif
                g_skpDB.UpdateChannelStatus(nChannelID,PAUSE_STATUS);
                it->second->SetStatus(PAUSE_STATUS);
            }
        }
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

void CSkpChannelCenter::ResumeChannel(int nChannelID)
{
	LogNormal("==CSkpChannelCenter::ResumeChannel()=ID[%d]\n", nChannelID);
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelID);
	if(it != m_ChannelEntityMap.end())
	{
		 printf("before i'm open nChannelID=%d!\r\n",nChannelID);

		  //printf("before i'm open it->second->IsOpen()=%d!\r\n",it->second->IsOpen());
		if(!(it->second->IsOpen()))
		{
			 printf("i'm open !\r\n");
			//启动通道实体对象
			if(it->second->Open())
			{
				//打开设备，开始检测
				it->second->BeginDetect();
				#ifdef DEBUG_CHANNELCENTER
				   LogNormal("激活通道成功!\r\n");
				#endif
			}
		}
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//暂停检测
void CSkpChannelCenter::PauseDetect(int nChannelID)
{
    //加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	if(nChannelID <= 0)
	{
		ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
		while(it != m_ChannelEntityMap.end())
		{
			//停止检测
			it->second->RestartDetect(DETECT_NONE);
			it++;
		}
	}
	else
	{
		ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelID);
		if(it != m_ChannelEntityMap.end())
		{
			//停止检测
			it->second->RestartDetect(DETECT_NONE);
		}
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//重新启动检测
void CSkpChannelCenter::RestartDetect()
{
	LogNormal("==CSkpChannelCenter::RestartDetect()=\n");
    //加锁
	pthread_mutex_lock(&m_Entity_Mutex);
    ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
    while(it != m_ChannelEntityMap.end())
    {
        //需要从数据库中重新获取检测类型
        CHANNEL_DETECT_KIND uDetectKind = g_skpDB.GetDetectKind(it->second->GetChannelID());
		if(g_nMultiPreSet == 1)//存在多个预置位
		{
			CXmlParaUtil xml;
			int nPreSetDetectKind = xml.LoadPreSetDetectKind(it->second->GetChannelID(),it->second->GetPreSet());
			if(nPreSetDetectKind != 0)
			{
				uDetectKind = (CHANNEL_DETECT_KIND)nPreSetDetectKind;
			}
		}
        it->second->RestartDetect(uDetectKind);
        it++;
    }
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

void CSkpChannelCenter::SetChannelConnect(bool bConnect,int nChannel)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);

	if(nChannel==0)
	{
		ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
		while(it != m_ChannelEntityMap.end())
		{
			it->second->SetConnect(bConnect);
			it++;
		}
	}
	else
	{
		ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
		if(it != m_ChannelEntityMap.end())
		{
			#ifdef DEBUG_CHANNELCENTER
			printf("connect to channelid = %d\r\n",it->second->GetChannelID());
			#endif
			it->second->SetConnect(bConnect);
		}
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//通道是否打开成功
bool CSkpChannelCenter::GetChannelStatus(int nChannelID)
{
	bool bOpen = false;
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelID);
	if(it != m_ChannelEntityMap.end())
	{

		bOpen = it->second->IsOpen();
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
	return bOpen;
}


//发送事件给中心端(非机动车道)
int CSkpChannelCenter::SendTraEvent(SRIP_DETECT_HEADER sDHeader,RECORD_EVENT event,SYN_CHAR_DATA* syn_char_data,bool bObject)
{
    //printf("SendTraEvent=m_bRealTimeEvent=%d,m_nEventKind=%x\n",m_bRealTimeEvent,m_nEventKind);


	//判断是否实时事件（增加有无中心端判断：zhangyaoyao）
	if (1 == g_nHasCenterServer)
	{
		std::string strEvent("");
		///////////////
        {
            if(event.uType == 1)
            {
                event.uType = PERSON_TYPE;
            }
            else if(event.uType == 2)
            {
                event.uType = OTHER_TYPE;
            }
            else if(event.uType == 0)//事件报不出大中小车，只能报出机动车，默认给出小车
            {
                event.uType = SMALL_CAR;
            }
        }

        if(event.uCode == DETECT_RESULT_EVENT_WRONG_CHAN)
        {
            event.uCode = 14;
        }
        else if(event.uCode == DETECT_RESULT_EVENT_APPEAR)
        {
            event.uCode = 10;
        }

        strEvent.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));

		
	#ifdef LS_QINGTIAN_IVAP
		if (g_nServerType == 5)
	#else
		if(g_nServerType == 0 || g_nServerType == 6 || g_nServerType == 4 || g_nServerType == 26)
	#endif 
        {
			if(event.chText[0]=='*')
            {
                if((event.chText[1]=='-'))//无牌车
                {
					memcpy(event.chText,"11111111",8);
                }
                else//未识别出结果
                {
                    memcpy(event.chText,"00000000",8);
                }
            }

			std::string strCarNum(event.chText);

			if(strCarNum.size() > 0)
			{
				printf("CSkpChannelCenter::SendTraEvent strCarNum=%s\n",strCarNum.c_str());

				g_skpDB.UTF8ToGBK(strCarNum);
				memset(event.chText,0,MAX_EVENT);
				memcpy(event.chText,strCarNum.c_str(),strCarNum.size());
			}

            if(g_nServerType == 0)
            {
                event.uColor1 += 1;//送中心端颜色值加1
                event.uColor2 += 1;
                event.uColor3 += 1;
            }

            if(event.uDetailCarType==TRUCK_TYPE)//车型细分
            {
                event.uDetailCarType = 1;
            }
            else if(event.uDetailCarType==BUS_TYPE)
            {
                event.uDetailCarType = 2;
            }
            else if(event.uDetailCarType==MIDDLEBUS_TYPE)
            {
                event.uDetailCarType = 3;
            }
            else if(event.uDetailCarType==TAXI)
            {
                event.uDetailCarType = 4;
            }
            else if(event.uDetailCarType==TWO_WHEEL_TYPE)//两轮车
            {
                event.uDetailCarType = 5;
            }
            else
            {
                event.uDetailCarType = 6;
            }

            BOCOM_RECORD_EVENT br_event;
            memcpy(&br_event,&event,sizeof(BOCOM_RECORD_EVENT));

			string strFtpVideoPath(event.chVideoPath);
			if(strFtpVideoPath.size() > 0)
			{
				int nPos = g_ServerHost.size()+6;
				string strVideoPath = g_strVideo;
				strFtpVideoPath.insert(nPos,strVideoPath.c_str(),strVideoPath.size());
				printf("%s\n",strFtpVideoPath.c_str());
				strFtpVideoPath.insert(6,"road:road@",10);
				printf("%s\n",strFtpVideoPath.c_str());
				printf("event.chVideoPath=%s\n",event.chVideoPath);
				memset(event.chVideoPath,0,MAX_VIDEO);
				printf("event.chVideoPath=%s\n",event.chVideoPath);
				memcpy(event.chVideoPath,strFtpVideoPath.c_str(),strFtpVideoPath.size());
				printf("event.chVideoPath=%s\n",event.chVideoPath);
			}
			memcpy(br_event.chVideoPath,event.chVideoPath,MAX_VIDEO);//录像路径
            memcpy(br_event.chPicPath,event.chPicPath,MAX_VIDEO);//大图片路径

            br_event.uColor2 = event.uColor2;   //车身颜色2
            br_event.uColor3 = event.uColor3;   //车身颜色3

            br_event.uWeight1 = event.uWeight1;      //车身颜色权重1
            br_event.uWeight2 = event.uWeight2;      //车身颜色权重2
            br_event.uWeight3 = event.uWeight3;      //车身颜色权重3
            br_event.uDetailCarType = event.uDetailCarType;       //车型细分

            strEvent.append((char*)&br_event,sizeof(BOCOM_RECORD_EVENT));
        }
        else
        {
            strEvent.append((char*)&event,sizeof(RECORD_EVENT));
        }


        {
                if(0 == g_nServerType ||
                   4 == g_nServerType ||
				   6 == g_nServerType ||
				   26 == g_nServerType)
                {
					g_BocomServerManage.AddResult(strEvent);
                }
                else if(1 == g_nServerType)
                {
                    g_AMSCommunication.AddResult(strEvent);
                }
                else if(2 == g_nServerType)
                {
                    g_CenterServer.AddResult(strEvent);
                }

                else if(3 == g_nServerType)
                {
                    g_TravelServer.AddResult(strEvent);
                }
                else if(5 == g_nServerType)
                {
                    g_LSServer.AddResult(strEvent);
			#ifdef LS_QINGTIAN_IVAP
					g_BocomServerManage.AddResult(strEvent);
			#endif
                }
                else if(10 == g_nServerType || 21 == g_nServerType|| 23 == g_nServerType || 24 == g_nServerType )
                {
                    g_TJServer.AddResult(strEvent);
                }
				else if(7 == g_nServerType)
				{
					#ifdef CAMERAAUTOCTRL
					g_TJServer.AddResult(strEvent);
					#endif
				}
				else if(8 == g_nServerType)
				{
					g_TelComServer.AddResult(strEvent);
				}
				else if(9 == g_nServerType)
				{
					g_HttpCommunication.AddResult(strEvent);
				}
				else if (13 == g_nServerType)
				{
					g_MyCenterServer.AddResult(strEvent);
				}
				else if (16 == g_nServerType)
				{
					g_SJServer.AddResult(strEvent);
				}
				else if(29 == g_nServerType)
				{
					g_HttpUpload.AddResult(strEvent);
				}
				

        }
	}
	return true;
}

//发送车牌给中心端(机动车道)
int CSkpChannelCenter::SendPlateInfo(SRIP_DETECT_HEADER sDHeader,RECORD_PLATE plate,SYN_CHAR_DATA* syn_char_data)
{
    //printf("SendPlateInfo=m_bRealTimePlate=%d,m_nPlateKind=%x\n",m_bRealTimePlate,m_nPlateKind);

	//判断是否实时车牌（增加有无中心端判断：zhangyaoyao）
	if (1 == g_nHasCenterServer)
	{
        //if(g_nServerType == 1)
        {
            if(plate.chText[0]=='*')
            {
                if((plate.chText[1]=='-'))//无牌车
                {
					if (12 == g_nServerType || 18 == g_nServerType)
						memcpy(plate.chText,"00000000",8);
					else
						memcpy(plate.chText,"11111111",8);
                }
                else//未识别出结果
                {
                    memcpy(plate.chText,"00000000",8);
                }
            }
        }

        std::string strCarNum(plate.chText);

        if(g_nServerType!=1 && g_nServerType!=5 && g_nServerType!=10 && \
			g_nServerType!=15 && g_nServerType!=18 && g_nServerType!=20 && \
			g_nServerType!=23 && g_nServerType!=26 && g_nServerType!=7&& g_nServerType!=29)
        g_skpDB.UTF8ToGBK(strCarNum);

        memset(plate.chText,0,MAX_PLATE);
        memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

        std::string strPlate("");
        strPlate.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));
        strPlate.append((char*)&plate,sizeof(RECORD_PLATE));

        {
                if(g_nServerType == 0 || g_nServerType == 4 ||g_nServerType == 6 || g_nServerType == 26)
                {
					g_BocomServerManage.AddResult(strPlate);
                }
                else if(1 == g_nServerType)
                {
                    g_AMSCommunication.AddResult(strPlate);
                }
                else if(2 == g_nServerType)
                {
                    g_CenterServer.AddResult(strPlate);
                }
                else if(3 == g_nServerType || 22 == g_nServerType)
                {
                    g_TravelServer.AddResult(strPlate);
                }
                else if(5 == g_nServerType)
                {
                    g_LSServer.AddResult(strPlate);
			#ifdef LS_QINGTIAN_IVAP
					//LogNormal("SendPlateInfo seqId:%d,type:%d car:%s \n", plate.uSeqID, plate.uViolationType, plate.chText);
					//LogNormal("plate.uColor:%d, uCarColor1:%d uType:%d", plate.uColor, plate.uCarColor1, plate.uType);
					g_BocomServerManage.AddResult(strPlate);
			#endif
                }
                else if(10 == g_nServerType || 7 == g_nServerType || 21 == g_nServerType|| 23 == g_nServerType || 24 == g_nServerType)
                {
                    g_TJServer.AddResult(strPlate);
                }
				else if(9 == g_nServerType)
				{
					g_HttpCommunication.AddResult(strPlate);
				}
				else if (12 == g_nServerType)
				{
					g_BaoKangServer.AddResult(strPlate);
				}
				if (13 == g_nServerType)
				{
					g_MyCenterServer.AddResult(strPlate);

				}
				if (15 == g_nServerType)
				{
					g_XingTaiCenter.AddResult(strPlate);
				}
				if (18 == g_nServerType)
				{
					g_YiChangCenter.AddResult(strPlate);
				}
				if (19 == g_nServerType)
				{
					g_WuHanCenter.AddResult(strPlate);
				}
				if (20 == g_nServerType)
				{
					#ifdef YUHUANCENTER_OK
					g_YuHuanCenter.AddResult(strPlate);
					#endif
				}
				else if(29 == g_nServerType)
				{
					g_HttpUpload.AddResult(strPlate);

					//LogError("添加车牌记录成功!\r\n");
				}
				else if (30 == g_nServerType)
				{
					g_BXServer.AddResult(strPlate);
				}
        }
	}
	return true;
}

//获取连接的通道数目
int CSkpChannelCenter::GetConnectChannelCount()
{
	int count = 0;
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);

	ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
	while(it != m_ChannelEntityMap.end())
	{
		bool bConnect = false;
		bConnect = it->second->GetConnect();

		if(bConnect)
		{
			count++;
		}

		it++;
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
	return count;
}

//截取图像
void CSkpChannelCenter::CaptureOneFrame(std::string& result,int nChannel,ImageRegion imgRegion)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		it->second->CaptureOneFrame(result,imgRegion);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//清除数据
void CSkpChannelCenter::Clear()
{
    printf("begin clear============\n");
      pthread_mutex_lock(&m_Pic_Mutex);
      m_ChannelPicList.clear();
      pthread_mutex_unlock(&m_Pic_Mutex);

      pthread_mutex_lock(&m_Result_Mutex);
     m_ChannelResultList.clear();
     pthread_mutex_unlock(&m_Result_Mutex);

      printf("end clear============\n");
}

 /* 函数介绍：相机控制(for gige)
 * 输入参数：cfg-相机控制参数
 * 输出参数：无
 * 返回值：无
 */
void CSkpChannelCenter::CameraControl(int nChannel,CAMERA_CONFIG& cfg)
{
     //加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		it->second->CameraControl(cfg);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//发送区域图像
void CSkpChannelCenter::SendRegionImage(int nChannel,ImageRegion imgRegion)
{
         //加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		it->second->SendRegionImage(imgRegion);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//返回通道检测类型
CHANNEL_DETECT_KIND CSkpChannelCenter::GetChannelDetectKind(int nChannel)
{
    CHANNEL_DETECT_KIND uDetectKind = DETECT_NONE;
    pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		uDetectKind = it->second->GetChannelDetectKind();
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
	return uDetectKind;
}

//获取通道情况
void CSkpChannelCenter::GetChannelInfo(ChannelInfoList& chan_info_list)
{
    pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
	while(it != m_ChannelEntityMap.end())
	{
	    CHANNEL_INFO_RECORD chan_info;
		it->second->GetChannelInfo(chan_info);
        chan_info_list.push_back(chan_info);
		it++;
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

//获取通道图像分辨率
void CSkpChannelCenter::GetImageSize(int nChannel,int& nWidth,int& nheight)
{
    pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		it->second->GetImageSize(nWidth,nheight);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

 /*
  *函数介绍：对所有通道的相机进行控制(for gige)
 * 输入参数：cfg-相机控制参数
 * 输出参数：无
 * 返回值：无
 */
void CSkpChannelCenter::CameraControl(CAMERA_CONFIG& cfg)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
	for ( ; it != m_ChannelEntityMap.end(); it++)
	{
		it->second->CameraControl(cfg);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}


//获取相机编号
int CSkpChannelCenter::GetChannelCamId(int nChannel)
{
	int nCamId = 0;
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		nCamId = it->second->GetCameraID();
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

	return nCamId;
}

//刷新通道
bool CSkpChannelCenter::UpdateChannel(SRIP_CHANNEL sChannel)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(sChannel.uId);
	if(it != m_ChannelEntityMap.end())
	{
		//相机编号
		bool bModifyCameraID = it->second->GetCameraID()!=sChannel.nCameraId;
		g_nCameraID = sChannel.nCameraId;

		bool bModifyVideoBeginTime = it->second->GetVideoBeginTime() != sChannel.uVideoBeginTime;
		bool bModifyVideoEndTime = it->second->GetVideoEndTime() != sChannel.uVideoEndTime;

		printf("sChannel.uVideoBeginTime=%lld,sChannel.uVideoEndTime=%lld\n",sChannel.uVideoBeginTime,sChannel.uVideoEndTime);

		//检测类型
		bool bChangeDetectKind = it->second->GetChannelDetectKind() != sChannel.uDetectKind;


		sChannel.nCameraType = it->second->GetCameraType();
		sChannel.eVideoFmt = it->second->GetChannelFormat();
		sChannel.nVideoIndex = it->second->GetVideoID();
		sChannel.nYUVFormat = it->second->GetYUVFormat();
		sChannel.nWorkMode = it->second->GetChannelWorkMode();
		string strFileName = it->second->GetChannelFileName();
		memcpy(sChannel.chFileName,strFileName.c_str(),strFileName.size());

		//视频源改变、相机类型改变、文件名称改变、相机模式改变需要重新启动通道处理
		if(bModifyCameraID||bModifyVideoBeginTime||bModifyVideoEndTime)
		{
			LogNormal("==Restart channel!!=\n");
			it->second->Restart(sChannel);
		}

		//检测类型改变或相机工作模式改变需要重启算法检测模块
		if(bChangeDetectKind)
		{
			it->second->RestartDetect(sChannel.uDetectKind);
		}
	}
	else
	{
		// what would you like to do with it if this certain channel doesn't exist,
		// return false or add another new channel with this channel ID?
		pthread_mutex_unlock(&m_Entity_Mutex);
		return false;
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

	return true;
}

//获取相机型号
int CSkpChannelCenter::GetChannelCamType(int nChannel)
{
	int nCamId = 0;
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		nCamId = it->second->GetCameraType();
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

	return nCamId;
}

//获取通道方向
int CSkpChannelCenter::GetChannelDirection(int nChannel)
{
	int nDirection = 0;
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		nDirection = it->second->GetChannelDirection();
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

	return nDirection;
}


/*
* 1.东向西==>西直，西左亮 2
* 2.西向东==>东直，东左亮 1
* 3.南向北==>北直，北左亮 4
* 4.北向南==>南直，南左亮 3
*/
//获取红灯方向
int CSkpChannelCenter::GetChannelRedDirection(int nChannel)
{
	int nRedDirection = 0;
	int nDirection = this->GetChannelDirection(nChannel);
	switch(nDirection)
	{
	case 1:
		{
			nRedDirection = 2;
			break;
		}
	case 2:
		{
			nRedDirection = 1;
			break;
		}
	case 3:
		{
			nRedDirection = 4;
			break;
		}
	case 4:
		{
			nRedDirection = 3;
			break;
		}
	default:
		break;
	}

	return nRedDirection;
}


//清空输出JpgMap列表
void CSkpChannelCenter::ClearChannelJpgMap(int nChannel)
{
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		it->second->ClearJpgMap();
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

}

bool CSkpChannelCenter::AddForceAlert(int nChannelID,FORCEALERT *pAlert)
{
	bool rv = false;

	pthread_mutex_lock(&m_Entity_Mutex);

	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelID);

	if(it != m_ChannelEntityMap.end())
	{
		rv = it->second->AddForceAlert(pAlert);
	}

	pthread_mutex_unlock(&m_Entity_Mutex);

	return rv;
}


void CSkpChannelCenter::DetectRegionRectImage(int nChannel,ImageRegion imgRegion)
{
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		it->second->DetectRegionRectImage(imgRegion);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

void CSkpChannelCenter::DetectParkObjectsRect(int nChannel,UINT32 uMsgCommandID,RectObject &ObjectRect)
{
	//加锁
	printf("CSkpChannelCenter::DetectParkObjectsRect \n");
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		printf("CSkpChannelCenter::DetectParkObjectsRect1 \n");
		it->second->DetectParkObjectsRect(uMsgCommandID,ObjectRect);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}

int CSkpChannelCenter::AddRecordEvent(int nChannel,std::string result)
{
	printf("CSkpChannelCenter::AddRecordEvent \n");
	//加锁
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannel);
	if(it != m_ChannelEntityMap.end())
	{
		printf("CSkpChannelCenter::AddRecordEvent111 \n");
		it->second->AddRecordEvent(nChannel,result);
	}
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
}


//获取通道列表
void CSkpChannelCenter::GetAllChannelsInfo(CHANNEL_INFO_LIST& chan_info_list)
{
	printf("--11111--GetAllChannelsInfo-----\n");
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
	while(it != m_ChannelEntityMap.end())
	{
		SRIP_CHANNEL chan_info;
		bool bGet = g_skpDB.GetChannelInfoFromDBById(it->first, chan_info);

		if(bGet)
		{
			LogNormal("--GetAllChannelsInfo-chanid:%d chPlace:%s Run:%d\n", chan_info.uId, chan_info.chPlace,chan_info.bRun);
			chan_info_list.push_back(chan_info);
		}

		it++;
	}

	printf("--22222--GetAllChannelsInfo-----\n");
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);
	printf("--3333--GetAllChannelsInfo-----\n");
}

//添加通道列表
bool CSkpChannelCenter::AddChannelList(CHANNEL_INFO_LIST& chan_info_list)
{
	LogNormal("-111--AddChannelList--\n");
	bool bRet = false;
	
	//判断是否有命令
	if(chan_info_list.size()>0)
	{		
		SRIP_CHANNEL chan_info;
		UINT32 uId = 0;

		while(chan_info_list.size() > 0)
		{
			CHANNEL_INFO_LIST::iterator it = chan_info_list.begin();
			chan_info = *it;
			uId = CheckChannel(chan_info);

			LogNormal("-AddChannelList--uId:%d ip:%s Run:%d \n", chan_info.uId, chan_info.chCameraHost, chan_info.bRun);

			if(0 == uId)
			{
				uId = m_ChannelEntityMap.size() + 1;//取一个可用的ID号
				chan_info.uId = uId;
				bRet = g_skpDB.AddChan(chan_info);
			}			

			chan_info_list.pop_front();
		}		
#ifdef MVSBAK
		//更新AMS,DSP通道信息
		g_AMSCommunication.mvSendChannelListXml();
#endif
	}

	LogNormal("--22222--AddChannelList--bRet:%d---\n", bRet);

	return bRet;
}

//删除对应通道列表
bool CSkpChannelCenter::DelChannelList(CHANNEL_INFO_LIST& chan_info_list)
{
	LogNormal("-111--DelChannelList--\n");
	bool bRet = false;

	//判断是否有命令
	if(chan_info_list.size()>0)
	{		
		SRIP_CHANNEL chan_info;
		UINT32 uId = 0;

		while(chan_info_list.size() > 0)
		{
			CHANNEL_INFO_LIST::iterator it = chan_info_list.begin();
			chan_info = *it;
			uId = CheckChannel(chan_info);

			if(uId > 0)
			{
				chan_info.uId = uId;
				LogNormal("-To Del uId:%d--\n", uId);
				bRet = g_skpDB.DelChan(chan_info);
			}			

			chan_info_list.pop_front();
		}		

#ifdef MVSBAK
		//更新AMS,DSP通道信息
		g_AMSCommunication.mvSendChannelListXml();
#endif
	}

	LogNormal("--22222--DelChannelList--bRet:%d---\n", bRet);

	return bRet;
}

//指定IP是否已经在通道列表,若存在返回对应通道ID
UINT32 CSkpChannelCenter::CheckChannel(SRIP_CHANNEL &chan_info)
{
	UINT32 uId = 0;
	int nCompare = -1;
	pthread_mutex_lock(&m_Entity_Mutex);
	ChannelEntityMap::iterator it = m_ChannelEntityMap.begin();
	while(it != m_ChannelEntityMap.end())
	{
		SRIP_CHANNEL chan_info_src;		
		bool bGet = g_skpDB.GetChannelInfoFromDBById(it->first, chan_info_src);

		if(bGet)
		{
			nCompare = strcmp(chan_info.chCameraHost, chan_info_src.chCameraHost);

			if(0 == nCompare)
			{
				uId = chan_info_src.uId;
				printf("--CheckChannel-ok-chanid:%d \n", chan_info_src.uId);
				break;
			}
		}		
		
		it++;
	}

	printf("---CheckChannel---chanid:%d \n", uId);
	//解锁
	pthread_mutex_unlock(&m_Entity_Mutex);

	return uId;
}

//清空数据前,把有图未输出的数据全部输出
bool CSkpChannelCenter::OutPutChannelResult(const int nChannelId)
{
	bool bRet = false;

	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelId);
	if(it != m_ChannelEntityMap.end())
	{
		bRet = it->second->OutPutResultAll();
	}
	
	return bRet;
}

#ifdef REDADJUST
//红绿灯增强
bool CSkpChannelCenter::RedLightAdjust(const int nChannelId, IplImage *pImage)
{
	bool bRet = false;

	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelId);
	if(it != m_ChannelEntityMap.end())
	{
		bRet = it->second->RedLightAdjust(pImage);
	}
	
	return bRet;
}
#endif

//通过uImgKey，核查记录状态
bool CSkpChannelCenter::CheckImgKeyState(const int nChannelId, const UINT64 &uKey)
{
	bool bRet = false;

	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelId);
	if(it != m_ChannelEntityMap.end())
	{
		bRet = it->second->CheckImgKeyState(uKey);
	}

	return bRet;
}

//更新通道记录标记
bool CSkpChannelCenter::UpdateImgKeyByChannelID(const int nChannelId, const UINT64 &uKey, const int &bState)
{
	bool bRet = false;

	ChannelEntityMap::iterator it = m_ChannelEntityMap.find(nChannelId);
	if(it != m_ChannelEntityMap.end())
	{
		bRet = it->second->UpdateImgListByKey(uKey, bState);
	}

	return bRet;
}