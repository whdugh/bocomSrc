// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#include "KeyBoardCodeSerial.h"
#include "Common.h"
#include "CommonHeader.h"

#ifndef DETECT_EVENT_STOP_LOG
    //#define DETECT_EVENT_STOP_LOG
#endif

CVisKeyBoardControl g_VisKeyBoardControl;

//记录发送线程
void* ThreadSendSerialCmd(void *pArg)
{
    while(!g_bEndThread)
    {
        bool bBallControl = g_VisKeyBoardControl.GetBallControl();
        if(!bBallControl)
        {
            g_VisKeyBoardControl.DealCmdMsg();
            usleep(10*1000); //发完一条命令等10ms
        }
        else
        {
            g_VisKeyBoardControl.DealCmdMsgAndWait(); //球机控制有等待时间
        }
    }
}
//
CVisKeyBoardControl::CVisKeyBoardControl()
{
    m_bTimeWait = false; //默认发码完不等待时间

    m_bBallCamera = true;

    pthread_mutex_init(&m_mutexMsg, NULL);
}
//
CVisKeyBoardControl::~CVisKeyBoardControl()
{
    pthread_mutex_destroy(&m_mutexMsg);
}

//发送镜头控制命令(pelco protocol)
bool CVisKeyBoardControl::SendKeyData(CAMERA_CONFIG& cfg, int nMonitorID, int nCount,bool bTimeWait)
{
    printf("=============Enter CamerSerial::SendKeyData()================\n");
    CAMERA_MESSAGE nMsg = (CAMERA_MESSAGE)cfg.nIndex;
	int ndata = (int)cfg.fValue;

	std::string strCodeSend;
    std::string strCodeTemp = "";

    int nSleepTime = 0;

	CKeyBoardCodeSerial keyBoardSerial;
	keyBoardSerial.SetMonitorID(nMonitorID);
	keyBoardSerial.SetKeyBoardID(g_nKeyBoardID);
	keyBoardSerial.SetCameraID(cfg.nAddress);
	keyBoardSerial.SetPreSetID(ndata);

	printf("=============MonitorID = %d==g_nKeyBoardID = %d==cfg.nAddress = %d===\n", nMonitorID, g_nKeyBoardID, cfg.nAddress);

	bool bSwitchCameraFlag = true;
	//int nDownFlag = 0; //发送键盘控制状态--0，按下又抬起 1：按下 2：抬起
	bool bTimeControl = false; //是否采用时间间隔控制球机

	//SetBallControl(true); //设置是球机控制

	if(nCount > 10 || nCount < -10) //由控制命令参数来决定是否由时间间隔控制
	{
	    bTimeControl = true;
	}

	if(nCount < 0)
    {
        int nTemp = abs(nCount);
        nCount = nTemp;

        switch(nMsg) //遇见负数取反方向
        {
            case ZOOM_NEAR:
            {
                cfg.nIndex = ZOOM_FAR;
                break;
            }
            case ZOOM_FAR:
            {
                cfg.nIndex = ZOOM_NEAR;
                break;
            }
            case UP_DIRECTION:
            {
                cfg.nIndex = DOWN_DIRECTION;
                break;
            }
            case DOWN_DIRECTION:
            {
                cfg.nIndex = UP_DIRECTION;
                break;
            }
            case LEFT_DIRECTION:
            {
                cfg.nIndex = RIGHT_DIRECTION;
                break;
            }
            case RIGHT_DIRECTION:
            {
                cfg.nIndex = LEFT_DIRECTION;
                break;
            }
            default:
            {
                break;
            }
        }
    }

#ifdef DETECT_EVENT_STOP_LOG
    struct timeval tv;
    struct tm *newTime,timenow;
    newTime = &timenow;
    char cTimeBuf[100];

#endif

    if(bTimeWait)
    {

    if(bTimeControl) //采用时间间隔控制球机
    {
        SetIsWaitTime(true); //设置发送码是否需要等待

         //获取按下键发送码
        strCodeTemp = keyBoardSerial.GetControlCode(cfg, bSwitchCameraFlag, 1);

        int nSpaceTime = GetSpaceTimes(cfg.nAddress);
        //nSleepTime = GetSleepTimes(); //获取发命令前的等待时间
    //1. 发按下键命令，
        AddCmdMsgAndWait(strCodeTemp, nSpaceTime, cfg.nAddress);

    #ifdef DETECT_EVENT_STOP_LOG
        gettimeofday(&tv,NULL);
        localtime_r( &tv.tv_sec,newTime);
        sprintf( cTimeBuf, "H:%02d M:%02d S:%02d MS:%04d",
            newTime->tm_hour, newTime->tm_min, newTime->tm_sec, (int)(tv.tv_usec/1000.0));

        char cInfoStr[50];
        sprintf(cInfoStr, "event_stop_%02d.txt", cfg.nAddress);
        FILE *pEvent = fopen(cInfoStr, "a+");
        //需要应用端添加相机放大的代码
        if (pEvent != NULL)
        {
            fprintf(pEvent, "===TIME:%s=nMonitorID=%d==Key_DOWN==cfg.nIndex=%d====cfg.nAddress=%d====strCodeSend.size()=%d \
                    ===strCodeSend=%s======nSpaceTime=%d============!\n",
                    cTimeBuf, nMonitorID, cfg.nIndex, cfg.nAddress, strCodeTemp.size(), strCodeTemp.c_str(), nSpaceTime);
            fflush(pEvent);
        }
        //fclose(pEvent);
    #endif


        //获取抬起键发送码
        strCodeTemp = keyBoardSerial.GetControlCode(cfg, bSwitchCameraFlag, 2);


    //3. 发抬起键命令 2.并传入发此条命令之前，与第一条命令间隔，需要等待的时间
        nSleepTime = nCount;
        AddCmdMsgAndWait(strCodeTemp, nSleepTime, cfg.nAddress);



    #ifdef DETECT_EVENT_STOP_LOG
        gettimeofday(&tv,NULL);
        localtime_r( &tv.tv_sec,newTime);
        sprintf( cTimeBuf, "H:%02d M:%02d S:%02d MS:%04d",
            newTime->tm_hour, newTime->tm_min, newTime->tm_sec, (int)(tv.tv_usec/1000.0));

        if (pEvent != NULL)
        {
            fprintf(pEvent, "===TIME:%s===nMonitorID=%d==Key_UP==cfg.nIndex=%d====cfg.nAddress=%d====strCodeSend.size()=%d \
                    ===strCodeSend=%s===nSleepTime=%d=============!\n",
                    cTimeBuf, nMonitorID, cfg.nIndex, cfg.nAddress, strCodeTemp.size(), strCodeTemp.c_str(), nSleepTime);
            fflush(pEvent);
        }
        fclose(pEvent);
    #endif

    } //End of if(bTimeControl)
    else
    {
        //获取按下和抬起键命令发送码
        strCodeTemp = keyBoardSerial.GetControlCode(cfg, bSwitchCameraFlag,0);

        for(int i=0; i<nCount; i++)
        {
            {
                SetIsWaitTime(true); //设置发送码之间需要等待时间
                int nSpaceTime = GetSpaceTimes(cfg.nAddress);

                AddCmdMsgAndWait(strCodeTemp, nSpaceTime, cfg.nAddress); //并传入发送码之间需要等待时间
                printf("===========AddCmdMsgAndWait(strCodeTemp, 500);==\n");

            #ifdef DETECT_EVENT_STOP_LOG
                gettimeofday(&tv,NULL);
                localtime_r( &tv.tv_sec,newTime);
                sprintf( cTimeBuf, "H:%02d M:%02d S:%02d MS:%04d",
                    newTime->tm_hour, newTime->tm_min, newTime->tm_sec, (int)(tv.tv_usec/1000.0));

                char cInfoStr[50];
                sprintf(cInfoStr, "event_stop_%02d.txt", cfg.nAddress);
                FILE *pEvent = fopen(cInfoStr, "a+");

                if (pEvent != NULL)
                {
                    fprintf(pEvent, "===TIME:%s====AddCmdMsgAndWait(strCodeTemp, 500)====nSpaceTime=%d==GetSpaceTimes()===========!\n",
                            cTimeBuf, nSpaceTime);
                    fflush(pEvent);
                }
                fclose(pEvent);
            #endif

            }

        }
    }//End of else -->if(bTimeControl)

        //发完码后将等待时间归0
        SetSleepTimes(cfg.nAddress, 0);
        SetSpaceTimes(cfg.nAddress, 500);
    }
    else //不需要等待直接发送
    {
        int nDownFlag = 0;
        int nSpeed = 5; //速率
        int nSpaceTime = 500;

        if(cfg.nIndex == ZOOM_NEAR ||
           cfg.nIndex == ZOOM_FAR ||
           cfg.nIndex == UP_DIRECTION ||
           cfg.nIndex == DOWN_DIRECTION ||
           cfg.nIndex == LEFT_DIRECTION ||
           cfg.nIndex == RIGHT_DIRECTION )
        {
            if(cfg.nOperationType == 1)
            {
                nDownFlag = 1;
            }
            else if(cfg.nOperationType == 2) //抬起命令前不需要等待
            {
                nSpaceTime = 0;
                nDownFlag = 2;
            }
            nSpeed = 1;
        }

        //获取按下和抬起键命令发送码
        strCodeTemp = keyBoardSerial.GetControlCode(cfg, bSwitchCameraFlag,nDownFlag,nSpeed);

        SetIsWaitTime(true); //设置发送码之间需要等待时间

        AddCmdMsgAndWait(strCodeTemp, nSpaceTime, cfg.nAddress);


        #ifdef DETECT_EVENT_STOP_LOG

                char cInfoStr[50];
                sprintf(cInfoStr, "event_stop_%02d.txt", cfg.nAddress);
                FILE *pEvent = fopen(cInfoStr, "a+");

                if (pEvent != NULL)
                {
                    fprintf(pEvent, "strCodeTemp=%s===strCodeTemp.size()=%d=======!\n",strCodeTemp.c_str(),strCodeTemp.size());
                    fflush(pEvent);
                }
                fclose(pEvent);
            #endif

        //SetSpaceTimes(cfg.nAddress, 500);
    }

	return true;
}

//开启串口控制命令发送线程
void CVisKeyBoardControl::BeginThread()
{
    printf("===========CVisKeyBoardControl::BeginThread()=========\n");

        //线程id
        pthread_t id;
        //线程属性
        pthread_attr_t   attr;
        //初始化
        pthread_attr_init(&attr);
        //分离线程
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        //启动记录发送线程

        int nret=pthread_create(&id,&attr,ThreadSendSerialCmd,NULL);
        if(nret!=0) //失败
        {
            LogError("启动串口发送线程失败!\r\n");
        }
        else
        {
            LogNormal("启动串口发送线程成功!\r\n");
        }
        pthread_attr_destroy(&attr);
}

//压入串口控制命令
void CVisKeyBoardControl::AddCmdMsg(std::string& sCmdMsg)
{
    pthread_mutex_lock(&m_mutexMsg);

    m_cmdMsgList.push_back(sCmdMsg);
    //解锁
    pthread_mutex_unlock(&m_mutexMsg);
}

//取出串口控制命令
int CVisKeyBoardControl::PopCmdMsg(std::string& sCmdMsg)
{
    int ncmdMsgSize = 0;
    //加锁
    pthread_mutex_lock(&m_mutexMsg);

    ncmdMsgSize = m_cmdMsgList.size();

    //判断是否有命令
    if(ncmdMsgSize>0)
    {
        //取最早命令
        SerialCmdMsg::iterator it = m_cmdMsgList.begin();
        //保存数据
        sCmdMsg = *it;
        //删除取出的命令
        m_cmdMsgList.pop_front();
    }
    //解锁
    pthread_mutex_unlock(&m_mutexMsg);

    return ncmdMsgSize;
}

//压入串口控制命令和需要间隔时间
void CVisKeyBoardControl::AddCmdMsgAndWait(std::string& sCmdMsg, int nTimesWait, int nIndex)
{
    printf("====CVisKeyBoardControl::AddCmdMsgAndWait=====nIndex=%d============\n", nIndex);

    //加锁
    pthread_mutex_lock(&m_mutexMsg);

    CmdMsgAndTime sCmdMsgTemp; //临时消息
    CmdMsgMap::iterator it_cmdMsgMap = m_CmdMsgMap.find(nIndex);
    if(it_cmdMsgMap != m_CmdMsgMap.end() && it_cmdMsgMap->first == nIndex)
    {
        sCmdMsgTemp.nIndex = nIndex;
        sCmdMsgTemp.cmdMsg = sCmdMsg;
        sCmdMsgTemp.nTimesWait = nTimesWait;

        #ifdef _DEBUG
            printf("=============CVisKeyBoardControl::AddCmdMsgAndWait=========add one Msg======\n");
        #endif

        #ifdef _DEBUG
            printf("================m_CmdMsgMap.size()=%d==============\n", m_CmdMsgMap.size());
            printf("==============Msg=sCmdMsgTemp:(%d, %s, %d)=======\n",\
                    sCmdMsgTemp.nIndex, sCmdMsgTemp.cmdMsg.c_str(), sCmdMsgTemp.nTimesWait);
        #endif

        (it_cmdMsgMap->second).push_back(sCmdMsgTemp); //添加新消

    }
    else //未找到对应消息列表
    {
        printf("=======Can't find the MsgMap==CVisKeyBoardControl::AddCmdMsgAndWait=====nIndex=%d============\n", nIndex);

    #ifdef _DEBUG
        printf("=======CVisKeyBoardControl::AddDev()=====Init MsgList=======nIndex=%d==\n", nIndex);
    #endif

        //加入list
        CmdMsgAndTimeList sCmdMsgList;
        //插入map
        m_CmdMsgMap.insert(CmdMsgMap::value_type(nIndex, sCmdMsgList));

        it_cmdMsgMap = m_CmdMsgMap.find(nIndex);
        sCmdMsgTemp.nIndex = nIndex;
        sCmdMsgTemp.cmdMsg = sCmdMsg;
        sCmdMsgTemp.nTimesWait = nTimesWait;

        #ifdef _DEBUG
            printf("=============CVisKeyBoardControl::AddCmdMsgAndWait=222========add one Msg======\n");
        #endif

        #ifdef _DEBUG
            printf("============222====m_CmdMsgMap.size()=%d==============\n", m_CmdMsgMap.size());
            printf("==========222====Msg=sCmdMsgTemp:(%d, %s, %d)=======\n",\
                    sCmdMsgTemp.nIndex, sCmdMsgTemp.cmdMsg.c_str(), sCmdMsgTemp.nTimesWait);
        #endif

        (it_cmdMsgMap->second).push_back(sCmdMsgTemp); //添加新消
    }
    //解锁
    pthread_mutex_unlock(&m_mutexMsg);
}

//调用时间，在发完一条命令之后，就调用
//取得所有设备待发命令列表中找出最先发送命令的 设备编号，设备命令和等待时间
bool CVisKeyBoardControl::GetCmdMsgAndWait(CmdMsgAndTime& cmdMsg/*std::string& sCmdMsg, int& nTimesWait, int& nIndex*/)
{
    //printf("======CVisKeyBoardControl::GetCmdMsgAndWait()===============\n");
    if( 0 == m_CmdMsgMap.size())
    {
        return false;
    }

    bool bGetMsgFlag = false;
    CmdMsgAndTimeList sCmdMsgListTemp; //临时消息链表
   // int nTimsWaitMin = 0; //最短等待时间时间
   // int nTimsWaitTemp = 0;

    //遍历所有设备
    CmdMsgMap::iterator it_cmdMsgMap = m_CmdMsgMap.begin();
    for(; it_cmdMsgMap!= m_CmdMsgMap.end(); it_cmdMsgMap++)
    {

        if( (it_cmdMsgMap->second).size() > 0)
        {
            //取每个消息链表的第一条消息,放到临时消息链表中
            sCmdMsgListTemp.push_back( *((it_cmdMsgMap->second).begin()) );

            bGetMsgFlag = true;
        }
    }

    #ifdef _DEBUG
        printf("===========CVisKeyBoardControl::GetCmdMsgAndWait()=======11111====sCmdMsgListTemp.size()=%d====\n",sCmdMsgListTemp.size());
    #endif

    //int nID; //设备ID
    int nIdTemp = 0;
    int i = 0;
    int64_t tvWaitMin = 0;
    //发命令列表中找出最先发送命令的 设备编号
    CmdMsgAndTimeList::iterator it_cmdMsg_b = sCmdMsgListTemp.begin();
    //nTimsWaitMin = (*it_cmdMsg_b).nTimesWait;
    //nID = (*it_cmdMsg_b).nIndex;
    for( ; it_cmdMsg_b != sCmdMsgListTemp.end(); it_cmdMsg_b++)
    {
        nIdTemp = (*it_cmdMsg_b).nIndex;

        struct timeval tvPre;
        tvPre.tv_sec = 0;
        tvPre.tv_usec = 0;
        GetTimesPreById(&tvPre,nIdTemp);
        int64_t tvCur = ((tvPre.tv_sec) * 1000 + (tvPre.tv_usec) / 1000.0) + (*it_cmdMsg_b).nTimesWait;

        if(i == 0)
        {
            tvWaitMin = tvCur;
            //nID = nIdTemp;
            cmdMsg = *it_cmdMsg_b;
        }

        if(tvWaitMin > tvCur)
        {
            tvWaitMin = tvCur;
            //nID = nIdTemp;
            cmdMsg = *it_cmdMsg_b;
        }

        i++;
        // printf("===========CVisKeyBoardControl::GetCmdMsgAndWait()======nTimsWaitTemp=%lld,nTimsWaitMin=%lld\n",nTimsWaitTemp,nTimsWaitMin);
    }

    CmdMsgMap::iterator it = m_CmdMsgMap.find(cmdMsg.nIndex);
    if(it != m_CmdMsgMap.end()) //找到设备对应消息链表
    {
        it->second.pop_front();
    }

    return bGetMsgFlag;
}

//获取指定设备编号，上一条命令与现在时间差-单位毫秒
int CVisKeyBoardControl::GetTimesWaitById(int nIndex)
{
    int nTimsToWait = 0;
    struct timeval tvPre; //前一条命令的发送时刻

    //获取前一条命令发送时刻
    TimeMap::iterator it = m_timeMapPre.find(nIndex);
    if(it != m_timeMapPre.end()) //找到前一条命令的发送时刻
    {
        tvPre.tv_sec = (it->second).tv_sec;
        tvPre.tv_usec = (it->second).tv_usec;
    }
    else
    {
        printf("GetTimesWaitById find nothing\n");
        return -1; //未找到，返回-1
    }

    //计算本条命令发送时需要等待时间
    //还需等待时间 = 需要等待时间 - (当前时间 - 前一条命令发送时间)
    struct timeval tvNow; //当前时间
    gettimeofday(&tvNow,NULL);
    nTimsToWait = (int)( (tvNow.tv_sec - tvPre.tv_sec) * 1000 + (tvNow.tv_usec - tvPre.tv_usec) / 1000.0);

    if(nTimsToWait < 0)
    {
        //ERROR
        return -1;
    }

    return nTimsToWait;
}

//取出串口控制命令 和 需要间隔时间
int CVisKeyBoardControl::PopCmdMsgAndWait(CmdMsgAndTime& cmdMsg)
{
    int ncmdMsgSize = 0;

    //加锁
    pthread_mutex_lock(&m_mutexMsg);

    //获取下一条命令发送的 设备编号，设备命令和等待时间
    CmdMsgAndTime cmdMsg1;
    bool bGetMsgFlag = GetCmdMsgAndWait(cmdMsg1);

    //把下一条命令插入到待发命令列表中
    if(bGetMsgFlag)
    {
        #ifdef _DEBUG
            printf("=========Add a Msg to m_cmdMsgSendList====Msg=cmdMsg:(%d, %s, %d)=======\n",\
                    cmdMsg1.nIndex, cmdMsg1.cmdMsg.c_str(), cmdMsg1.nTimesWait);
        #endif

        m_cmdMsgSendList.push_back(cmdMsg1);
    }

    ncmdMsgSize = m_cmdMsgSendList.size();

    //printf("===CVisKeyBoardControl::PopCmdMsgAndWait====ncmdMsgSize=%d====11====\n", ncmdMsgSize);
    //判断是否有命令-且命令能一一对应
    if(ncmdMsgSize>0)
    {
    #ifdef _DEBUG
        printf("===CVisKeyBoardControl::PopCmdMsgAndWait====ncmdMsgSize=%d====11====\n", ncmdMsgSize);
    #endif

        //取最早命令
        CmdMsgAndTimeList::iterator it = m_cmdMsgSendList.begin();

        //CmdMsgAndTime sCmdMsgSend; //要发送消息
        //保存数据
        //sCmdMsgSend = *it;

        cmdMsg = (*it);

        //删除取出的命令
        m_cmdMsgSendList.pop_front();
    }

    //解锁
    pthread_mutex_unlock(&m_mutexMsg);

    return ncmdMsgSize;
}

//处理串口控制命令和需要间隔时间
void CVisKeyBoardControl::DealCmdMsgAndWait()
{
    std::string sCmdMsg;
    int nTimesWait = 0;
    int nId;//设备编号

    int ncmdMsgSize = 0;
    CmdMsgAndTime cmdMsg;
    ncmdMsgSize = PopCmdMsgAndWait(cmdMsg);

    //printf("========CVisKeyBoardControl::DealCmdMsgAndWait()======ncmdMsgSize=%d=cmdMsg.nIndex=%d,cmdMsg.nTimesWait=%d,cmdMsg.cmdMsg.c_str()=%s=\n", ncmdMsgSize,cmdMsg.nIndex,cmdMsg.nTimesWait,cmdMsg.cmdMsg.c_str());
    //若没有取到要等待的时间，按默认，发命令之前等1ms
    nTimesWait = cmdMsg.nTimesWait;
    if(0 == nTimesWait)
    {
        usleep(1*1000); //sleep 1 ms
    }

    if(ncmdMsgSize > 0)
    {
        printf("==============CVisKeyBoardControl::DealCmdMsgAndWait()========ncmdMsgSize=%d========\n", ncmdMsgSize);
        nId = cmdMsg.nIndex;
        sCmdMsg = cmdMsg.cmdMsg;
        //发送串口命令之前等待一定时间
        if(m_bTimeWait && nTimesWait >= 0) //需要等待一定时间
        {
            struct timeval tvNow; //当前时间
            gettimeofday(&tvNow,NULL);
            int nTimsSleep = 0; //需要sleep时间 毫秒
            int nTimsGone = 0; //已经用去时间 毫秒

            //计算已经花去的时间 nTimsGone
            //tvMid.tv_sec = tvNow.tv_sec - m_tvTimesPre.tv_sec; //秒s
            //tvMid.tv_usec = tvNow.tv_usec - m_tvTimesPre.tv_usec; //微秒us

            //获取对应设备Id,对应的上一条命令发送瞬时时刻
            struct timeval tvTimesPre;
            bool bGetTimesPre = GetTimesPreById(&tvTimesPre, nId);


            if(bGetTimesPre)
            {
                nTimsGone = (int)( (tvNow.tv_sec - tvTimesPre.tv_sec) * 1000 + (tvNow.tv_usec - tvTimesPre.tv_usec) / 1000.0);
            }

            if(nTimsGone < 0)
            {

            #ifdef DETECT_EVENT_STOP_LOG
                char cInfoStr[50];
                sprintf(cInfoStr, "event_time.txt");
                FILE *pEvent = fopen(cInfoStr, "a+");
                //需要应用端添加相机放大的代码
                if (pEvent != NULL)
                {
                    fprintf(pEvent, "==DealCmdMsgAndWait::===nTimsGone=%d==!\n", nTimsGone);
                    fflush(pEvent);
                }
                fclose(pEvent);
            #endif

                nTimsGone = 0;
            }

            nTimsSleep = nTimesWait - nTimsGone;

            #ifdef DETECT_EVENT_STOP_LOG
                char cInfoStr2[50];
                sprintf(cInfoStr2, "event_time-wait.txt");
                FILE *pEvent2 = fopen(cInfoStr2, "a+");
                //需要应用端添加相机放大的代码
                if (pEvent2 != NULL)
                {
                    fprintf(pEvent2, "==DealCmdMsgAndWait::==nTimesWait=%d===nTimsGone=%d==nTimsSleep=%d==!\n", nTimesWait, nTimsGone,nTimsSleep);
                    fflush(pEvent2);
                }
                fclose(pEvent2);
            #endif

            if(nTimsSleep < 0)
            {
                nTimsSleep = 0;
            }

            usleep(nTimsSleep * 1000); //sleep nTimsSleep毫秒



        #ifdef DETECT_EVENT_STOP_LOG
            struct timeval tv;
            gettimeofday(&tv,NULL);

            struct tm *newTime,timenow;
            newTime = &timenow;

            localtime_r( &tv.tv_sec,newTime);
            int nHour = newTime->tm_hour;
            int nMin = newTime->tm_min;
            int nSec = newTime->tm_sec;
            int nMiniSec = (int)(tv.tv_usec/1000.0);
            char cTimeBuf[100];
            sprintf( cTimeBuf, "H:%02d M:%02d S:%02d MS:%04d",nHour,nMin,nSec,nMiniSec);

            char cInfoStr[50];
            sprintf(cInfoStr, "event_time.txt");
            FILE *pEvent = fopen(cInfoStr, "a+");
            //需要应用端添加相机放大的代码
            if (pEvent != NULL)
            {
                fprintf(pEvent, "==DealCmdMsgAndWait::==TIME_NOW:%s===nTimesWait=%d==nTimsSleep=%d==nTimsGone=%d==!\n", cTimeBuf, nTimesWait, nTimsSleep, nTimsGone);
                fflush(pEvent);
            }
            fclose(pEvent);
        #endif

        }//End of if(m_bTimeWait && nTimesWait > 0)

        #ifdef DETECT_EVENT_STOP_LOG
            struct timeval tv;
            gettimeofday(&tv,NULL);

            struct tm *newTime,timenow;
            newTime = &timenow;

            localtime_r( &tv.tv_sec,newTime);
            int nHour = newTime->tm_hour;
            int nMin = newTime->tm_min;
            int nSec = newTime->tm_sec;
            int nMiniSec = (int)(tv.tv_usec/1000.0);
            char cTimeBuf[100];
            sprintf( cTimeBuf, "H:%02d M:%02d S:%02d MS:%04d",nHour,nMin,nSec,nMiniSec);

            char cInfoStr[50];
            sprintf(cInfoStr, "event_time.txt");
            FILE *pEvent = fopen(cInfoStr, "a+");
            //需要应用端添加相机放大的代码
            if (pEvent != NULL)
            {
                fprintf(pEvent, "===TIME:%s==\n ------------==nId=%d====-----sCmdMsg=%s \n \
                        ========m_bTimeWait=%d==nTimesWait=%d===!\n\n",
                        cTimeBuf, nId, sCmdMsg.c_str(), m_bTimeWait, nTimesWait);
                fflush(pEvent);
            }
            fclose(pEvent);
        #endif

        bool bRet = false;
        if(0 == g_nControlMode)
        {
          bRet = g_VisSerialCommunication.WriteCmdToVis(sCmdMsg);
        }
        else
        {
          bRet = g_VisNetCommunication.WriteCmdToVis(sCmdMsg);
        }

        if(bRet)
        {
            SetTimeMap(nId);
            printf("write sCmdMsg ok!\r\n");
        }

        //删除消息列表中设备号nId的第一条消息
        //DelCmdMsgById(nId);
    }
    return;
}

//处理串口控制命令
void CVisKeyBoardControl::DealCmdMsg()
{
    std::string sCmdMsg;

    int ncmdMsgSize = PopCmdMsg(sCmdMsg);

    if(sCmdMsg.size() > 0)
    {
        g_VisSerialCommunication.WriteCmdToVis(sCmdMsg);
    }
}

//添加新设备控制消息列表
bool CVisKeyBoardControl::AddDev(int nIndex)
{
    printf("========CVisKeyBoardControl::AddDev()============nIndex=%d====\n", nIndex);

    //加锁
    pthread_mutex_lock(&m_mutexMsg);

    //清空原有消息列表
    CmdMsgMap::iterator it_cmdMsgMap = m_CmdMsgMap.find(nIndex);
    if(it_cmdMsgMap != m_CmdMsgMap.end() && it_cmdMsgMap->first == nIndex)
    {
        //找到对应消息列表
        (it_cmdMsgMap->second).clear(); //清空链表
    }
    else //不存在，则添加新的消息列表
    {
    #ifdef _DEBUG
        printf("=======CVisKeyBoardControl::AddDev()=====Init MsgList=======nIndex=%d==\n", nIndex);
    #endif

        //加入list
        CmdMsgAndTimeList sCmdMsgList;

        //插入map
        m_CmdMsgMap.insert(CmdMsgMap::value_type(nIndex, sCmdMsgList));
    }

    //解锁
    pthread_mutex_unlock(&m_mutexMsg);

    return true;
}

//删除消息链表中发出去的消息
bool CVisKeyBoardControl::DelCmdMsgById(int nIndex)
{
    printf("==============CVisKeyBoardControl::DelCmdMsgById()===nIndex=%d===\n", nIndex);

    bool bDelFlag = true;

    //加锁
    pthread_mutex_lock(&m_mutexMsg);

    printf("=======CVisKeyBoardControl::DelCmdMsgById============22222222222222222222=============\n");

    //清空原有消息列表
    CmdMsgMap::iterator it_cmdMsgMap = m_CmdMsgMap.find(nIndex);
    if(it_cmdMsgMap != m_CmdMsgMap.end() && it_cmdMsgMap->first == nIndex)
    {
        printf("=======CVisKeyBoardControl::DelCmdMsgById============33333333333333=============\n");
        if( (it_cmdMsgMap->second).size() > 0 )
        {
            printf("=======CVisKeyBoardControl::DelCmdMsgById============4444444444==222222222111==========\n");
            //删除列表内第一条消息
            (it_cmdMsgMap->second).pop_front();
        }
        else
        {
            printf("=======CVisKeyBoardControl::DelCmdMsgById============4444444444==222222222--222==========\n");
        }
    }
    else //不存在，则添加新的消息列表
    {
        bDelFlag = false;
    }

    printf("=======CVisKeyBoardControl::DelCmdMsgById============555555555555555=============\n");

    //解锁
    pthread_mutex_unlock(&m_mutexMsg);

    printf("=======CVisKeyBoardControl::DelCmdMsgById============6666666666666=============\n");

    return bDelFlag;
}

//记录设备nIndex,发送记录时的瞬时时刻
bool CVisKeyBoardControl::SetTimeMap(int nIndex)
{
    printf("===========CVisKeyBoardControl::SetTimeMap()===nIndex=%d==\n", nIndex);

    int nTimsToWait = 0;
    //struct timeval tvPre; //前一条命令的发送时刻

    struct timeval tvNow; //当前时间
    gettimeofday(&tvNow,NULL);

    //获取前一条命令发送时刻
    TimeMap::iterator it = m_timeMapPre.find(nIndex);
    if(it != m_timeMapPre.end() && (it->first == nIndex)) //找到前一条命令的发送时刻
    {
        //tvPre.tv_sec = (*(it->second)).tv_sec;
        //tvPre.tv_usec = (*(it->second)).tv_usec;

        //更新发送记录时的瞬时时刻map
        (it->second).tv_sec = tvNow.tv_sec;
        (it->second).tv_usec = tvNow.tv_usec;
    }
    else  //未找到需要插入新记录
    {
        m_timeMapPre.insert(TimeMap::value_type(nIndex, tvNow));

        printf("====CVisKeyBoardControl::SetTimeMap====nIndex=%d,  tvNow.tv_sec = %d==tvNow.tv_usec=%d==!!\n",\
                nIndex, tvNow.tv_sec, tvNow.tv_usec);

        return false; //未找到
    }

    return true;
}

//获取设备Id,对应前一条命令发出时间
bool CVisKeyBoardControl::GetTimesPreById(struct timeval* pTimePretv, int nIndex)
{
    printf("===============CVisKeyBoardControl::GetTimesPreById()===========nIndex=%d=====\n", nIndex);

    //获取前一条命令发送时刻
    TimeMap::iterator it = m_timeMapPre.find(nIndex);
    if(it != m_timeMapPre.end() && (it->first == nIndex) ) //找到前一条命令的发送时刻
    {
        //获取发送记录map的瞬时时刻
        pTimePretv->tv_sec = (it->second).tv_sec;
        pTimePretv->tv_usec = (it->second).tv_usec;
    }
    else
    {
        return false; //未找到
    }

    return true;
}

//设置等待时间
void CVisKeyBoardControl::SetSleepTimes(int nIndex, int nTimes)
{
    SleepTimeMap::iterator it = m_SleepTimesMap.find(nIndex);
    if(it != m_SleepTimesMap.end() && (it->first == nIndex) )
    {
        //更新映射
        it->second = nTimes;
    }
    else
    {
        m_SleepTimesMap.insert(SleepTimeMap::value_type(nIndex, nTimes));
    }
}

//获取等待时间
int CVisKeyBoardControl::GetSleepTimes(int nIndex)
{
    int nTimes = 0;
    SleepTimeMap::iterator it = m_SleepTimesMap.find(nIndex);
    if(it != m_SleepTimesMap.end() && (it->first == nIndex) )
    {
        nTimes = it->second;
    }

    return nTimes;
}

//设置按格数控制，两条命令之间最少要间隔的时间-单位毫秒
void CVisKeyBoardControl::SetSpaceTimes(int nIndex, int nTimes)
{
    SpaceTimeMap::iterator it = m_SpaceTimesMap.find(nIndex);
    if(it != m_SpaceTimesMap.end() && (it->first == nIndex) )
    {
        //更新映射
        it->second = nTimes;
    }
    else
    {
        m_SpaceTimesMap.insert(SpaceTimeMap::value_type(nIndex, nTimes));
    }
}

//获取按格数控制，两条命令之间最少要间隔的时间-单位毫秒
int CVisKeyBoardControl::GetSpaceTimes(int nIndex)
{
    int nTimes = 0;
    SpaceTimeMap::iterator it = m_SpaceTimesMap.find(nIndex);
    if(it != m_SpaceTimesMap.end() && (it->first == nIndex) )
    {
        nTimes = it->second;
    }

    return nTimes;
}

