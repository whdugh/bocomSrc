// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "RoSeekCameraDsp.h"
#include "Common.h"
#include "global.h"
#include "CSeekpaiDB.h"
#include "XmlParaUtil.h"
#include "RoadChannelCenter.h"

#include "MvDspMaxSize.h"

#include "RoadLog.h"

//#define REINIT_RTSP

#define CMD_DATA_BUF_SIZE  4096
#define IMG_RECV_BUFF_SIZE 6000000

#define CIF_H264_WIDTH 352
#define CIF_H264_HEIGHT 288

/*
#ifndef ROSEEK_SAVE_INFO
    #define ROSEEK_SAVE_INFO
#endif
*/
/*
#ifndef DEBUG_DSP_CMD
	#define DEBUG_DSP_CMD
#endif
*/
#define SLEEPCOUNT 7000
#define H264_COUNT 500 //单位(10ms)--5秒

void* ThreadRoSeekDspCtrl(void* pArg);

//JPG图采集线程
void* ThreadRoSeekDspCapture(void* pArg)
{
    //取类指针
    CRoSeekCameraDsp* pRoSeekCameraDsp = (CRoSeekCameraDsp*)pArg;
    if(pRoSeekCameraDsp == NULL)
        return pArg;

    pRoSeekCameraDsp->CaptureFrame();

	LogNormal("ThreadRoSeekDspCapture exit!\n");
    pthread_exit((void *)0);
    return pArg;
}


//H264采集线程
void* ThreadRoSeekDspCapture2(void* pArg)
{
    //取类指针
    CRoSeekCameraDsp* pRoSeekCameraDsp2 = (CRoSeekCameraDsp*)pArg;
    if(pRoSeekCameraDsp2 == NULL)
        return pArg;

    pRoSeekCameraDsp2->CaptureFrame2();

	LogNormal("ThreadRoSeekDspCapture2 exit!\n");
    pthread_exit((void *)0);
    return pArg;
}


//校时线程
void* ThreadRoSeekDspSynClock(void* pArg)
{
    //取类指针
    CRoSeekCameraDsp* pRoSeekCameraDsp3 = (CRoSeekCameraDsp*)pArg;
    if(pRoSeekCameraDsp3 == NULL)
        return pArg;

    pRoSeekCameraDsp3->SynClock();

	LogNormal("ThreadRoSeekDspSynClock exit!\n");
    pthread_exit((void *)0);
    return pArg;
}

CRoSeekCameraDsp::CRoSeekCameraDsp(int nCameraType)
{
    printf("====CRoSeekCameraDsp::CRoSeekCameraDsp()====\n");
    m_nCameraType = nCameraType;
	m_nCameraBrand = 0;

	//设置相机帧率
	//200W 15
	//500W 10
	m_cfg.nFrequency = GetDspCameraFrequency(m_nCameraType);

    printf("====CRoSeekCameraDsp::CRoSeekCameraDsp===m_nWidth=%d==m_nHeight=%d=\n", m_nWidth, m_nHeight);

	m_nPort = ROSEEK_DSP_TCP_PORT;

    m_nMarginX = 0;
    m_nMarginY = 0;
    m_pFrameBuffer = NULL;

    m_nTcpFd = -1;

    m_bEndCapture = false;

    //线程ID
    m_nThreadId = 0;
    m_nThreadId2 = 0;
    m_nThreadId3 = 0;
	m_nThreadId5 = 0;

	m_uCurrBuff = 0;
	m_uPrevBuff = -1;

	pthread_mutex_init(&listJpgBuffMutex,NULL);

	m_pH264Capture = NULL;
	m_bOpenCamera = false;
	m_nCfgIndex = 0;
	m_nRecvNothingFlag = 0;

	m_nReOpenCount = 0;

	m_bCamReboot = false;
	m_pTempCapture = NULL;
	m_nRecvLastTime = 0;
	m_nRecvRtspFlag = 0;
	m_bSendCapturePic = false;
	m_bAutoCaptureTime = false;
	
#ifdef LED_STATE	
	m_nWarmLastTime = 0;
	
	for(int i=0; i<8; i++)
	{
		m_bPreLedArray[i] = false;
	}
#endif
}

CRoSeekCameraDsp::~CRoSeekCameraDsp()
{
	LogNormal("=in=~CRoSeekCameraDsp==\n");	

	pthread_mutex_destroy(&listJpgBuffMutex);

	LogNormal("=out=~CRoSeekCameraDsp==\n");
}


void CRoSeekCameraDsp::Init()
{
    printf("=====CRoSeekCameraDsp::Init()====\n");
    printf("###=DSP=IP=m_strCameraIP=%s==\n", m_strCameraIP.c_str());

    //初始化环形缓冲区
    for (int nBuff=0; nBuff<NUM_BUFF; nBuff++)
    {
        listJpgBuff[nBuff].sizeBuff = 0;
        listJpgBuff[nBuff].bIsLocked = false;
    }

    for(int i=0; i< MAX_SIZE; i++)
    {
		//LogNormal("=CRoSeekCameraDsp::Init()=m_nWidth=%d=m_nHeight=%d\n", m_nWidth, m_nHeight);
		m_FrameList2[i] = (unsigned char*)calloc(sizeof(yuv_video_buf) + 1024*1024*3, sizeof(unsigned char));      
	}

    m_uSeq = 0;
    m_nPreFieldSeq  = 0;

    m_nCurIndex = 0;
    m_nFirstIndex = 0;
    m_nFrameSize=0; 

    m_nCurIndex2 = 0;
    m_nFirstIndex2 = 0;
    m_nFrameSize2 = 0;
    m_pFrameBuffer2 = m_FrameList2[0];

	//初始化相机配置
	m_GECamCfgInfo.pCMDToCamera = &m_CMDToCamera;
	m_GECamCfgInfo.pCMDToCamera->SetTargetAddr(m_strCameraIP.c_str(), ROSEEK_DSP_TCP_CONTROL_PORT);

    m_bEndCapture = false;
    //线程ID
    m_nThreadId = 0;
    m_nThreadId2 = 0;
    m_nThreadId3 = 0;
	m_nThreadId5 = 0;
	
	//初始化环形缓冲区
	for (int i=0; i<NUM_H264_BUFF; i++)
	{
		listJpgBuff[i].sizeBuff = 0;
		listJpgBuff[i].bIsLocked = false;
	}
}

void CRoSeekCameraDsp::CaptureFrame()
{
	while(!m_bEndCapture)
	{
		RecvData();

		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		select(0,NULL, NULL, NULL, &tv);
	}
}


void CRoSeekCameraDsp::CaptureFrame2()
{
	RecvRTSP();
}

bool CRoSeekCameraDsp::Open()
{
    //读取相机参数
    m_cfg.uKind = m_nCameraType;
	m_cfg.uCameraID = m_nCamID;
    LogNormal("=m_cfg.uCameraID=%d\n", m_cfg.uCameraID);

	Init();

	if(connect_tcp()==false)
	{
		m_bOpenCamera = false;
		//失败
		LogError("dsp=Open=无法与相机[%s]建立tcp连接\r\n", m_strCameraIP.c_str());
		g_skpRoadLog.WriteLog("相机断开\n",ALARM_CAMERA_BREAK ,true,m_nCamID);
		g_skpDB.SetCameraStateToDBByChannelID(m_nChannelId, CM_BREAK);
	}
	else
	{
		m_bOpenCamera = true;
		g_skpRoadLog.WriteLog("相机连接\n",ALARM_CAMERA_LINK ,true,m_nCamID);		
		//sleep(1);
	}
	
	//创建接收图片线程
	CreateCaptureThread();

	//创建定时校时线程
	CreateSynClockThread();

	//创建接收h264录像线程
	CreateH264CaptureThread();	

	//SetClock();

    return true;
}

bool CRoSeekCameraDsp::Close()
{
    m_bEndCapture = true;

    if(m_nTcpFd!=-1)
    {    
#ifdef DEBUG_DSP_CMD
		LogNormal("CloseFd [1] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
		CloseFd(m_nTcpFd);
    }

	CloseJoinThread(m_nThreadId);

	//LogNormal("==after Close1==\n");

	this->Close2();
   //LogNormal("==after Close2==\n");

    this->Close3();
    //LogNormal("==after Close3==\n");

	//加锁
	pthread_mutex_lock(&m_FrameMutex2);
	//释放内存
	for (int i=0; i<MAX_SIZE; i++)
	{
		if(m_FrameList2[i] != NULL)
		{
			delete (m_FrameList2[i]);
			m_FrameList2[i] = NULL;
		}
		m_nFrameSize2 = 0;
		m_pFrameBuffer2 = NULL;
	}
	//解锁
	pthread_mutex_unlock(&m_FrameMutex2);

    return true;
}

//关闭H264码流采集
bool CRoSeekCameraDsp::Close2()
{
    if(m_pH264Capture != NULL)
    {
		m_pH264Capture->UnInit();
        m_pH264Capture = NULL;
		printf("===after m_pH264Capture->UnInit()==\n");
    }

	if(m_pTempCapture != NULL)
	{
		m_pTempCapture->Unit();
		m_pTempCapture = NULL;
	}
    CloseJoinThread(m_nThreadId2);

    return true;
}


//关闭DSP校时
bool CRoSeekCameraDsp::Close3()
{
    LogNormal("=CRoSeekCameraDsp::Close3()==\n");   
    CloseJoinThread(m_nThreadId3);

    return true;
}


//手动控制
int CRoSeekCameraDsp::ManualControl(CAMERA_CONFIG  cfg)
{
	cfg.uCameraID = m_nCamID;
	LogNormal("=ManualControl=m_nCamID=%d=cfg.nIndex=%d,cfg.chCmd=%s\n", \
		m_nCamID, cfg.nIndex, cfg.chCmd);
    printf("=ManualControl==cfg.nIndex=%d==\n", cfg.nIndex);
    if(cfg.nIndex != (int)CAMERA_MODE)
    {
        cfg.nMode = m_cfg.nMode;
        printf("=====ManualControl====cfg.nMode=%d======\n", cfg.nMode);
    }

    if(cfg.nIndex != (int)CAMERA_LIGHTTYPE)
        cfg.nLightType = m_cfg.nLightType;

    if(cfg.nIndex == (int)CAMERA_CMD)
    {
        if (strcmp(cfg.chCmd,"asc")==0)
        {
            cfg.nIndex = (int)CAMERA_ASC;
            cfg.ASC = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"agc")==0)
        {
            cfg.nIndex = (int)CAMERA_AGC;
            cfg.AGC = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"sh")==0)
        {
            cfg.nIndex = (int)CAMERA_SH;
            cfg.uSH = (int)cfg.fValue;
            printf("===CRoSeekCameraDsp::ManualControl==cfg.uSH=%d==\n", cfg.uSH);
        }
        else if (strcmp(cfg.chCmd,"maxsh")==0)
        {
            cfg.nIndex = (int)CAMERA_MAXSH;
            cfg.nMaxSH = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            cfg.nIndex = (int)CAMERA_GAIN;
            cfg.uGain = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"maxga")==0)
        {
            cfg.nIndex = (int)CAMERA_MAXGAIN;
            cfg.nMaxGain = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"gama")==0)
        {
            cfg.nIndex = (int)CAMERA_GAMMA;
            cfg.nGamma = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pe")==0)
        {
            cfg.nIndex = (int)CAMERA_PE;
            cfg.uPE = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"epl")==0)
        {
            cfg.nIndex = (int)CAMERA_POL;
            cfg.uPol = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pen")==0)
        {
            cfg.nIndex = (int)CAMERA_EEN;
            cfg.EEN_on = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pei")==0)
        {
            cfg.nIndex = (int)CAMERA_PEI;
            cfg.EEN_delay = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"pew")==0)
        {
            cfg.nIndex = (int)CAMERA_PEW;
            cfg.EEN_width = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"restart")==0)
        {
            cfg.nIndex = (int)CAMERA_RESTART;
        }
        else if (strcmp(cfg.chCmd,"overtext")==0)
        {
            cfg.nIndex = (int)CAMERA_H264_STRING;
        }
        else if(strcmp(cfg.chCmd, "enhance")==0)
        {
            cfg.nIndex = (int)CAMERA_ENHANCE;
            cfg.nEnhance = (int)cfg.fValue; //由客户端传过来
            printf("=====cfg.nEnhance=%d===\n", m_cfg.nEnhance);
        }
        else if(strcmp(cfg.chCmd, "getonepic")==0)
        {
            cfg.nIndex = (int)CAMERA_GET_ONE_PIC;
            cfg.nGetOnePic = (int)cfg.fValue; //由客户端传过来
            printf("=====cfg.nGetOnePic=%d===\n", cfg.nGetOnePic);
        }
        else if(strcmp(cfg.chCmd, "gettime")==0)
        {
            cfg.nIndex = (int)CAMERA_GET_CLOCK;
            //由客户端传过来
            printf("=====cfg.szTime=%s===\n", cfg.szTime);
        }
        else if(strcmp(cfg.chCmd, "redflag")==0)
        {
            cfg.nIndex = (int)CAMERA_SET_REDFLAG;
            cfg.nRedFlag = (int)cfg.fValue; //由客户端传过来
            printf("=====cfg.nRedFlag=%d===\n", cfg.nRedFlag);
        }
		else if(strcmp(cfg.chCmd, "mvsport")==0)
        {
            cfg.nIndex = (int)CAMERA_SET_DSP_SERVER_HOST;
            LogNormal("==mvsport===cfg.fValue=%d===\n", (int)cfg.fValue);
        }
		else if(strcmp(cfg.chCmd, "setvtsrgn")==0)
		{
			cfg.nIndex = (int)CAMERA_SET_ROAD_CHANNEL_AREA;
			LogNormal("==setvtsrgn===cfg.fValue=%d===\n", (int)cfg.fValue);
		}
		else if(strcmp(cfg.chCmd, "setcardrgn")==0)
		{
			cfg.nIndex = (int)CAMERA_SET_DSP_CARNUM_RGN;
			LogNormal("==setcardrgn===cfg.fValue=%d===\n", (int)cfg.fValue);
		}
		else if(strcmp(cfg.chCmd, "setredrgn")==0)
		{
			cfg.nIndex = (int)CAMERA_SET_DSP_REDLIGHT_RGN;
			LogNormal("==setredrgn===cfg.fValue=%d===\n", (int)cfg.fValue);
		}
		else if(strcmp(cfg.chCmd, "setstream")==0)
		{
			cfg.nIndex = (int)CAMERA_DSP_STREAM_TYPE;
			LogNormal("==setstream===cfg.fValue=%d===\n", (int)cfg.fValue);
		}
		else if(strcmp(cfg.chCmd, "setsyn")==0) //设置电源同步
		{
			cfg.nIndex = (int)CAMERA_SET_DSP_ELEC_SYN;
			LogNormal("==setsyn===cfg.fValue=%d===\n", (int)cfg.fValue);
		}
		else if(strcmp(cfg.chCmd, "setredflag")==0) //设置红灯信号来源
		{
			cfg.nIndex = (int)CAMERA_SET_DSP_RED_FLAG;
			LogNormal("==setredflag===cfg.fValue=%d===\n", (int)cfg.fValue);
		}
		//0x5a
		else if(strcmp(cfg.chCmd, "setvtsxml")==0)
		{
			printf("==setvtsxml==\n");
			cfg.nIndex = (int)CAMERA_SET_XML_VTS_PARAM;
			LogNormal("==setvtsxml==\n");
		}
		//0x5e
		else if(strcmp(cfg.chCmd, "setdspstr")==0)
		{
			printf("==setdspstr==\n");
			cfg.nIndex = (int)CAMERA_SET_STRC;
		}
		//0x5f
		else if(strcmp(cfg.chCmd, "getdspstr")==0)
		{
			printf("==getdspstr==\n");
			cfg.nIndex = (int)CAMERA_GET_STRC;
		}
		//0x70
		else if(strcmp(cfg.chCmd, "getdspflag")==0)
		{
			printf("==getdspflag==\n");
			cfg.nIndex = (int)CAMERA_GET_DSP_GET_STRC_FLAG;
		}
		//0x84
		else if(strcmp(cfg.chCmd, "checkver")==0)
		{
			cfg.nIndex = (int)CAMERA_CHECK_DSP_VERSION;
		}
		//0x6f
		else if(strcmp(cfg.chCmd,"getbrand") == 0)
		{
			cfg.nIndex = (int)CAMERA_GET_DSP_BRAND;
		}
        else{}

    }

	if(cfg.nIndex == (int)CAMERA_MAXGAIN)
	{
		m_cfg.nMaxPE = cfg.nMaxPE;
		m_cfg.nMaxSH = cfg.nMaxSH;
		m_cfg.nMaxGain = cfg.nMaxGain;
		m_cfg.nMaxPE2 = cfg.nMaxPE2;
		m_cfg.nMaxSH2 = cfg.nMaxSH2;
		m_cfg.nMaxGain2 = cfg.nMaxGain2;
	}


    if(cfg.nIndex == CAMERA_SH)
    {
        cfg.uPE = cfg.uSH;
        cfg.uGain = m_cfg.uGain;
        WriteCameraIniDsp(cfg);
    }
    else if(cfg.nIndex == CAMERA_GAIN)
    {
        cfg.uPE = m_cfg.uSH;
        WriteCameraIniDsp(cfg);
    }

    return Control(cfg);
}



int CRoSeekCameraDsp::Control(CAMERA_CONFIG  &cfg)
{
	//LogTrace("DspControl-A.log", "=cfg.uKind=%d=cfg.nIndex=%d=[%x] \n", \
		cfg.uKind, cfg.nIndex, cfg.nIndex);
	//cerr << "CRoSeekCameraDsp::Control==cfg.nIndex=\n" <<cfg.nIndex<< endl;
	int nRet = 0;

	if(!m_bOpenCamera)
	{
		//LogNormal("Control err! cfg.nIndex:%x ", cfg.nIndex);
		return 0;
	}

	if((cfg.nIndex == CAMERA_SET_DSP_CARNUM_RGN) || (cfg.nIndex == CAMERA_SET_DSP_REDLIGHT_RGN)\
		|| (cfg.nIndex == CAMERA_GET_ONE_PIC) )
	{
		//m_cfg = cfg;
		m_nCfgIndex = cfg.nIndex;

		//线程属性
		pthread_attr_t   attr;
		//初始化
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);//PTHREAD_CREATE_DETACHED,PTHREAD_CREATE_JOINABLE

		int nret = pthread_create(&m_nThreadId5, NULL, ThreadRoSeekDspCtrl, this);

		LogTrace("DspControlThread.log", "=Dsp=after==pthread_create==m_nThreadId=%lld==\n", \
			m_nThreadId);

		LogTrace("DspControlThread.log", "=m_cfg.uKind=%d=\n", \
			m_cfg.uKind);

		pthread_attr_destroy(&attr);

		if(nret != 0)
		{
			CloseCamCtrl();
			//失败
			LogError("创建Dsp控制线程失败！\r\n");

			nRet = -1;
		}
	}//End of if
	else
	{
		nRet = this->ThreadControl(cfg);
	}

	return nRet;
}

//相机控制
//int CRoSeekCameraDsp::Control(CAMERA_CONFIG  &cfg)
int CRoSeekCameraDsp::ThreadControl(CAMERA_CONFIG  &cfg)
{
    //LogNormal("===CRoSeekCameraDsp::Control===cfg.uType=%d, cfg.nIndex=%d=\n", cfg.uType, cfg.nIndex);

	int nKind = cfg.nIndex;

    //先取得控制模式，连续模式，触发模式
    int m_nRunMode = 0;

    UINT32 dwCMDLenth = 0x00000000;
    UINT32 dwTemp = 0x00000000;
    int nAutoMode = 0; //是否自动控制相机，曝光，增益
    DspCmdHeader dspHeader;
    UINT32 dwAttribute = 0x00000000;
    UINT32 dwParamLen = 0x00000000; //参数个数
    char *pParam = NULL;//参数内容指针

    bool bSendCmdOK = false;
    bool nRet = false; //返回值

    //0:连续方式,1:触发方式
    if(cfg.nMode == 0)
    {
        m_nRunMode = ROSEEK_FconMode;
    }
    else if(cfg.nMode == 1)//!!触发方式关闭
    {
        //m_nRunMode = ROSEEK_TrgMode;
    }

    //if(nKind == CAMERA_SET_IP) //设置相机的IP
    printf("==11==nKind=%d=====\n", nKind);

    //////////////////////曝光，增益////////////////////////////////////
    if (nKind == CAMERA_ASC || nKind == CAMERA_AGC) //自动曝光或自动增益
    {
        dspHeader.aCmdID = 0x0f;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 1; //用1个字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];

            nAutoMode = cfg.ASC;
            dwTemp = nAutoMode?1:0;

            dwAttribute = dwTemp; //1:自动 0:手动

            this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);

            //设置是否自动曝光，或自动增益
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                m_cfg.ASC = cfg.ASC;
                m_cfg.AGC = cfg.AGC;
                bSendCmdOK = true;
            }
        }
    }
    else if(nKind == CAMERA_GAIN) //增益
    {
        dspHeader.aCmdID = 0x07;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 4; //用4个字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];
            dwAttribute = cfg.uGain;

            this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);

            //设置是否自动曝光，或自动增益
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                m_cfg.uGain = cfg.uGain;
                m_cfg.AGC = 0;
                bSendCmdOK = true;
            }
        }
    }
    else if(nKind == CAMERA_SH) //快门,曝光时间
    {
        dspHeader.aCmdID = 0x05;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 3; //用3个字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];
            dwAttribute = cfg.uSH;

            this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);

            //设置是否自动曝光，或自动增益
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                printf("=====Control==cfg.uSH = %d====\n", cfg.uSH);
                m_cfg.uSH = cfg.uSH;
                m_cfg.ASC = 0;
                bSendCmdOK = true;
            }
        }
    }
    else if(nKind == CAMERA_RESTART) //重启相机
    {
        //完整命令：0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        dspHeader.aCmdID = 0x00;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 0; //用0个字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = NULL;
            //dwAttribute = 0;
            //this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);

            //发送相机重启命令
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                bSendCmdOK = true;
                //return 1;
            }
        }

		//重启相机命令发送后,断开与相机的连接
		this->DisConnectCamera();
    }
    else if(nKind == CAMERA_SET_CLOCK) //设置时钟
    {
        time_t now;
        struct tm *newTime,timenow;
        newTime = &timenow;
        time( &now );
        localtime_r( &now,newTime );

        //转换为秒
        //return newTime->tm_hour * 60 * 60 + newTime->tm_min * 60 +newTime->tm_sec;
        int nYear = 0;
        int nMonth = 0;
        int nDay = 0;
        int nHour = 0;
        int nMin = 0;
        int nSec = 0;
        int nWeek = 0;
        int nMMsec = 0;

        printf("==1111===nYear, nMonth, nDay, nWeek, nHour, nMin, nSec==[%2d-%2d-%2d-%2d-%2d-%2d-%2d]===\n", \
               nYear, nMonth, nDay, nWeek, nHour, nMin, nSec);

        nYear = (newTime->tm_year + 1900) - 2000; //计算从2000年开始的年数
        nMonth = newTime->tm_mon+1; //[1-12]
        nDay = newTime->tm_mday; //[0-30]
        nHour = newTime->tm_hour; //[0-23] 东8区北京时间
        nMin = newTime->tm_min; //[0-59]
        nSec = newTime->tm_sec; //[0-59]
        nWeek = newTime->tm_wday; //[0-6]
        nMMsec = 0;

        sprintf(cfg.szTime, "%c%c%c%c%c%c%c", nYear, nMonth, nDay, nWeek, nHour, nMin, nSec);

        //LogNormal("==2222==nYear, nMonth, nDay, nWeek, nHour, nMin, nSec=\n");
		LogNormal("=CAMERA_SET_CLOCK==[%2d/%d/%d Week:%d %d:%d:%d]===\n", \
               nYear, nMonth, nDay, nWeek, nHour, nMin, nSec);

        //完整命令：0x18, 0x00, 0x07, 0x00, 0x00, 0x00,param
        dspHeader.aCmdID = 0x18;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 8; //用个7字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];

            memset(pParam, 0, 8);
            memcpy(pParam, cfg.szTime, 7);

            /*for(int i=0; i<8; i++)
            {
                printf("==###====cfg.szTime[%d]=%x==\n", i, cfg.szTime[i]);
                printf("==pParam[%d]=%x=\n", i, pParam[i]);
            }*/

            //发送
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD2(dspHeader, pParam))
            {
                memcpy(m_cfg.szTime, cfg.szTime, 7);

				unsigned char * pBackDataPtr = (unsigned char*)(m_GECamCfgInfo.pCMDToCamera->GetBackDataPtr());

				if(pBackDataPtr != NULL)
				{
					//LogNormal("Time Back[%d,%d,%d,%d,%d,%d] ", \
						pBackDataPtr[0], pBackDataPtr[1], pBackDataPtr[2], pBackDataPtr[3], pBackDataPtr[4], pBackDataPtr[5]);
					unsigned char uCode = pBackDataPtr[1];
					if(0xFF == uCode)
					{
						LogNormal("设置时钟失败! time:[%d,%d,%d,%d,%d,%d,%d]", \
							cfg.szTime[0],cfg.szTime[1],cfg.szTime[2],cfg.szTime[3],cfg.szTime[4],cfg.szTime[5],cfg.szTime[6]);
					}
					else if(0x00 == uCode)
					{
						LogNormal("设置时钟成功! time:[%d,%d,%d,%d,%d,%d,%d]", \
							cfg.szTime[0],cfg.szTime[1],cfg.szTime[2],cfg.szTime[3],cfg.szTime[4],cfg.szTime[5],cfg.szTime[6]);
					}
					else
					{
						LogNormal("设置时钟异常! uCode:%d time:[%d,%d,%d,%d,%d,%d,%d]", \
							uCode, cfg.szTime[0],cfg.szTime[1],cfg.szTime[2],cfg.szTime[3],cfg.szTime[4],cfg.szTime[5],cfg.szTime[6]);
					}
				}

                bSendCmdOK = true;
            }
			else
			{				
				LogNormal("ID:%d 设置时钟失败! time:[%d,%d,%d,%d,%d,%d,%d]", \
						m_nChannelId, cfg.szTime[0],cfg.szTime[1],cfg.szTime[2],cfg.szTime[3],cfg.szTime[4],cfg.szTime[5],cfg.szTime[6]);				
			}
        }
    }
    else if(nKind == CAMERA_GET_CLOCK) //获取时钟
    {
        printf("==Control=CAMERA_GET_CLOCK====cfg.szTime=%s===\n", cfg.szTime);

        //完整命令：0x18, 0x00, 0x07, 0x00, 0x00, 0x00,param
        dspHeader.aCmdID = 0x19;
        dspHeader.aReplyFlag = 0x01;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 1; //用个7字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];

            //发送
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                unsigned char * pBackDataPtr = (unsigned char*)(m_GECamCfgInfo.pCMDToCamera->GetBackDataPtr());

                if(pBackDataPtr != NULL)
                {
                    for(int i=0; i<7; i++)
                    {
                        printf("==pBackDataPtr[%d]=%x=\n", i, pBackDataPtr[i]);
                    }

                    memcpy(cfg.szTime, pBackDataPtr, 7);
                    printf("===DSP===pBackDataPtr=%s==\n", pBackDataPtr);
                    memcpy(m_cfg.szTime, cfg.szTime, 7);
                }
                bSendCmdOK = true;
            }
        }
    }
    else if(nKind == CAMERA_SET_IP)
    {
        printf("===CAMERA_SET_IP=====\n");
        //完整命令：0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        dspHeader.aCmdID = 0x1a;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 12; //用12个字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];
            UINT32 uTmp = 0;
//uCameraIP
            //uTmp = htonl(cfg.uCameraIP);
            uTmp = cfg.uCameraIP;
            memcpy(pParam, (char*)&uTmp, sizeof(UINT32));
//uGateWay
            uTmp = cfg.uGateWay;
            memcpy(pParam + sizeof(UINT32), (char*)&uTmp, sizeof(UINT32));
//uNetMask
            uTmp = cfg.uNetMask;
            memcpy(pParam + 2*sizeof(UINT32), (char*)&uTmp, sizeof(UINT32));

            printf("###=CAMERA_SET_IP===cfg.uCameraIP=%x=cfg.uGateWay=%x=cfg.uNetMask=%x===\n", cfg.uCameraIP, cfg.uGateWay, cfg.uNetMask);

            //发送设置相机IP命令
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                m_cfg.uCameraIP = cfg.uCameraIP;
                m_cfg.uGateWay = cfg.uGateWay;
                m_cfg.uNetMask = cfg.uNetMask;
                bSendCmdOK = true;
            }
        }
    }
	//CAMERA_DSP_STREAM_TYPE
	else if(nKind == CAMERA_DSP_STREAM_TYPE) //设置图像分辨率
    {
        printf("==Control=CAMERA_DSP_STREAM_TYPE====\n");
        dspHeader.aCmdID = 0x39;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 4; //用个1字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];

            dwAttribute = (int)cfg.fValue;
			int nTemp1 = (int)DSP_HIGH_1600_1200;
			int nTemp2 = (int)DSP_LOW_CIF;

			LogNormal("=DSP_HIGH_1600_1200=%d==DSP_LOW_CIF=%d=\n", nTemp1, nTemp2);

            if(dwAttribute != DSP_HIGH_1600_1200 && dwAttribute != DSP_LOW_CIF)
            {
				LogNormal("==set CAMERA_DSP_STREAM_TYPE==eRRor!==dwAttribute=%d=", dwAttribute);
                dwAttribute = 0;			
            }
			else
			{
				this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);
				//发送
				if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
				{
					LogNormal("=Control=CAMERA_DSP_STREAM_TYPE==ok!=type=%d=\n", dwAttribute);
					bSendCmdOK = true;
				}
			}
        }
    }
    else if(nKind == CAMERA_ENHANCE) //图像增强
    {
        printf("==Control=CAMERA_ENHANCE=cfg.nEnhance=%d===\n", cfg.nEnhance);
        //完整命令：0x41, 0x00, 0x01, 0x00, 0x00, 0x00,param
        dspHeader.aCmdID = 0x41;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 1; //用个1字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];

            dwAttribute = cfg.nEnhance;
            if(dwAttribute > 3)
            {
                dwAttribute = 3;
            }
            else if(dwAttribute < 0)
            {
                dwAttribute = 0;
            }

            this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);

            //发送
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                m_cfg.nEnhance = cfg.nEnhance;
                bSendCmdOK = true;
            }

        }
    }
    else if(nKind == CAMERA_MODE)
    {
        printf("====set===CAMERA_MODE===========\n");
        //命令0x43设置相机发图模式
        dspHeader.aCmdID = 0x43;
        dspHeader.aReplyFlag = 0x00;

        dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
        dwParamLen = 1; //用1个字节表示装载指令
        nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            pParam = new char[dwParamLen];

            UINT32 nCameraMode = 0; //param:0x00:正常运行模式，0x01安装模式
            if(m_nRunMode == ROSEEK_FconMode) //安装模式
            {
                printf("=====ROSEEK_FconMode===========\n");
                nCameraMode = 0x01;
            }
            else
            {
                printf("=====ROSEEK_TrgMode===========\n");
                nCameraMode = 0x00;
            }
            dwAttribute = nCameraMode;

            this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);

            //设置是否自动曝光，或自动增益
            if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
            {
                printf("=====Control==cfg.nMode = %d====\n", cfg.nMode);
                m_cfg.nMode = cfg.nMode;
                bSendCmdOK = true;
            }
        }
    }
    else if(nKind == CAMERA_GET_ONE_PIC)
    {
        printf("====set===CAMERA_GET_ONE_PIC===========\n");
		dspHeader.aCmdID = 0x02;
		dspHeader.aReplyFlag = 0x00;

		dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
		dwParamLen = 0; //用1个字节表示装载指令
		nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {     
			if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, NULL))
            {
                LogNormal("==ok===Control==cfg.nGetOnePic = %d====\n", cfg.nGetOnePic);
                m_cfg.nGetOnePic = cfg.nGetOnePic;
                bSendCmdOK = true;
            }
        }
    }
	else if(nKind == CAMERA_GET_STRC)
	{
		LogNormal("====get===CAMERA_GET_STRC===========\n");
		
		dspHeader.aCmdID = 0x5f;
		dspHeader.aReplyFlag = 0x01;

		dwCMDLenth = 4; //表示后面参数个数的变量固定长度
		dwParamLen = 0; //表示装载指令长度

		printf("==SendDspCMD3==----=\n");

			pParam = NULL;
			if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD3(dspHeader, pParam))
			{
				//指向接收数据字符串指针
				unsigned char * pBackDataPtr = (unsigned char*)(m_GECamCfgInfo.pCMDToCamera->GetBackDataBigPtr());
				if(pBackDataPtr != NULL)
				{
					unsigned char uCmd = *((unsigned char*)(pBackDataPtr));

					unsigned char uLenth[4];
					memcpy(uLenth, pBackDataPtr+2, 4);

					LogNormal("cmd=[%x]=uLenth [0]=%x=[1]=%x=[2]=%x=[3]=%x=", \
						dspHeader.aCmdID, uLenth[0], uLenth[1], uLenth[2], uLenth[3]);

					int nLenth = 0;

					nLenth += uLenth[0] & 0x000000FF;
					nLenth += (uLenth[1] & 0x000000FF) << 8;
					nLenth += (uLenth[2] & 0x000000FF)  << 16;
					nLenth += (uLenth[3] & 0x000000FF)  << 24;

					printf("=sizeof(MvDspGlobalSetting)=%d==", sizeof(MvDspGlobalSetting));
					if(sizeof(MvDspGlobalSetting) == (nLenth - 4))
					{
						//写文件
						FILE *fpOut = NULL;
						//char file_name[256] = "./config/DspSetting.aa";
						char file_name[256] = {0};
						sprintf(file_name, "./config/DspSetting%d.aa", m_nChannelId);
						fpOut = fopen(file_name, "wb+");
						if(fpOut != NULL)
						{
							fwrite(pBackDataPtr+6, 1, nLenth - 4, fpOut);
						}

						if(fpOut != NULL)
						{
							fclose(fpOut);
							fpOut = NULL;
						}

						LogNormal("====get===CAMERA_GET_STRC===nLenth=%d==ok!\n", nLenth);
					}
					else
					{
						LogNormal("====get===CAMERA_GET_STRC==111=nLenth=%d==error!\n", nLenth);
					}
				}
				bSendCmdOK = true;
			}
			else
			{
				LogNormal("====get===CAMERA_GET_STRC=====error!\n");
				bSendCmdOK = false;
			}
			
	}
	else if(nKind == CAMERA_SEND_LINK)
    {
        printf("====set===CAMERA_SEND_LINK===========\n");

		dspHeader.aCmdID = 0x68;
		dspHeader.aReplyFlag = 0x01;

		dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
		dwParamLen = 4; //用1个字节表示装载指令
		nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

        if(nRet)
        {
            //pParam = new char[dwParamLen];
			char chParam[4] = {'*','*','*','*'};

            //if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
			if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, chParam))
            {
				//LogNormal("====set===CAMERA_SEND_LINK==ok!!====\n");
                bSendCmdOK = true;
            }
			else
			{
				bSendCmdOK = false;
				LogNormal("====set===CAMERA_SEND_LINK==error!!==\n");
			}
        }
    }
	////
	else if(nKind == CAMERA_POL)  //设置高低电平
	{
		dspHeader.aCmdID = 0x6a;
		dspHeader.aReplyFlag = 0x00;

		dwCMDLenth = 4;
		dwParamLen = 4;
		nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

		if(nRet)
		{
			pParam = new char[dwParamLen];
			dwAttribute = (int)cfg.uPol;

			if(0 == dwAttribute || 1 == dwAttribute)
			{
				this->ConvertCMDToString(dwAttribute, pParam, dwParamLen);

				//设置是否自动曝光，或自动增益
				if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD(dspHeader, pParam))
				{
					printf("===CAMERA_POL control==dwAttribute=%d=ok !\n", dwAttribute);
					bSendCmdOK = true;
				}
			}//End of if(0 == dwAttribute || 1 == dwAttribute)
			else
			{
				bSendCmdOK = false;
			}
		}
	}
	else if(nKind == CAMERA_CHECK_DSP_VERSION)//0x84
	{
		dspHeader.aCmdID = 0x84;
		dspHeader.aReplyFlag = 0x01;

		dwCMDLenth = 4;
		dwParamLen = 4;
		nRet = this->ConvertCMDToString(dwParamLen, (char*)(dspHeader.aParamLen), dwCMDLenth);

		if(nRet)
		{
			pParam = new char[dwParamLen];
			//eg.ver3.3.2.1
			//unsigned char chParam[4] = {0x03, 0x03, 0x01, 0x01};
			//memcpy(pParam, chParam, 4);
			memcpy(pParam, g_chVersionDsp, 4);
			

			if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD2(dspHeader, pParam))
			{
				LogNormal("=Dsp=version==check=%d,%d,%d,%d===!\n", pParam[0], pParam[1], pParam[2], pParam[3]);

				//指向接收数据字符串指针
				unsigned char * pBackDataPtr = (unsigned char*)(m_GECamCfgInfo.pCMDToCamera->GetBackDataPtr());
				if(pBackDataPtr != NULL)
				{
					unsigned char uCmd = *((unsigned char*)(pBackDataPtr));

					unsigned char uLenth[4];
					memcpy(uLenth, pBackDataPtr+2, 4);

					LogNormal("cmd=[%x]=uLenth [0]=%x=[1]=%x=[2]=%x=[3]=%x=", \
						dspHeader.aCmdID, uLenth[0], uLenth[1], uLenth[2], uLenth[3]);

					unsigned char uVerDsp[4];

					for(int i=0; i<4; i++)
					{
						uVerDsp[i] = *(unsigned char*)(pBackDataPtr+6+i);
					}

					LogNormal("-dsp--ver[%x.%x.%x.%x]\n", \
						*(unsigned char*)(pBackDataPtr+6), *(unsigned char*)(pBackDataPtr+7), *(unsigned char*)(pBackDataPtr+8), *(unsigned char*)(pBackDataPtr+9));
					
					if(uVerDsp[0] == g_chVersionDsp[0] && \
						uVerDsp[1] == g_chVersionDsp[1] && \
						uVerDsp[2] == g_chVersionDsp[2])
					{
						//版本符合
						LogNormal("===版本匹配正确!===\n");
					}
					else
					{
						//版本不一致
						LogNormal("===版本匹配不一致!=DSP:[%d.%d.%d.%d]==\n", \
							uVerDsp[0], uVerDsp[1], uVerDsp[2], uVerDsp[3]);
					}
				}
				bSendCmdOK = true;
			}
			else
			{
				bSendCmdOK = false;
			}
		}
	}
	else if(nKind == CAMERA_GET_DSP_BRAND)//0x6f
	{
		dspHeader.aCmdID = 0x6f;
		dspHeader.aReplyFlag = 0x01;

		dwCMDLenth = 4; //表示后面参数个数的变量固定长度为4个字节
		dwParamLen = 0; //用1个字节表示装载指令
		
		//printf("==SendDspCMD3==----=\n");

		pParam = NULL;
		if(m_GECamCfgInfo.pCMDToCamera->SendDspCMD3(dspHeader, pParam))
		{
			//指向接收数据字符串指针
			unsigned char * pBackDataPtr = (unsigned char*)(m_GECamCfgInfo.pCMDToCamera->GetBackDataBigPtr());
			if(pBackDataPtr != NULL)
			{
				//unsigned char uCmd = *((unsigned char*)(pBackDataPtr));
				
				short srtType;
				memcpy(&srtType,pBackDataPtr+6,2);
				m_nCameraBrand = srtType;
				bSendCmdOK = true;
			}
		}
		else
		{
			LogNormal("====get===dsp商标CAMERA_GET_STRC=====error!\n");
			m_nCameraBrand = 0;
			bSendCmdOK = false;
		}
	}
	else{}

	//清理内存
	if(pParam != NULL)
	{
		//LogNormal("...deleting...pParam==\n");
		delete [] pParam;
		pParam == NULL;
	}
	
	return bSendCmdOK;
}


//reconnect the camera
bool CRoSeekCameraDsp::ReOpen()
{
	bool bRet = false;

	//重连前等待5秒,防止反复重连
	//sleep(5);
	if(connect_tcp() == false)
	{
		if(m_nReOpenCount > 4)
		{
			//失败
			LogError("ReOpen==无法与相机[%s]建立tcp连接\r\n", m_strCameraIP.c_str());
			g_skpRoadLog.WriteLog("相机断开\n",ALARM_CAMERA_BREAK ,true,m_nCamID);
			m_nReOpenCount = 0; //计数归0
			g_skpDB.SetCameraStateToDBByChannelID(m_nChannelId, CM_BREAK);
		}
		g_skpDB.SetCameraStateToDBByChannelID(m_nChannelId, CM_NO_VIDEO);
		m_nReOpenCount++;
		m_bOpenCamera = false;
		sleep(1);

		bRet = false;
	}
	else
	{
		LogNormal("=ReOpen=connect_tcp OK!=m_nTcpFd=%d==\n", m_nTcpFd);		
		m_bOpenCamera = true;
		g_skpRoadLog.WriteLog("相机连接\n",ALARM_CAMERA_LINK ,true,m_nCamID);		

		SetClock();//重连成功,同步时间
		bRet = true;
	}
    //LogNormal("=ReOpen=相机自动重新连接[%s]==m_nTcpFd=%d==", m_strCameraIP.c_str(), m_nTcpFd);

    return bRet;
}

//TCP 接收数据
void CRoSeekCameraDsp::RecvData()
{
    printf("=CRoSeekCameraDsp::RecvData()=\n");
    int	 nFrameSize = 0; //包体数据大小
    bool bRet = false; //接收数据成功标志

    char buffer[1024] = {0};
    char buff[1024] = {0};
    int nRecevCell = 512;
    int bytes = 0;
    int bytesRecved = 0;
    int nLeftSize = 0;
    int bytesRecveHead = 0;
    int nDspHeaderSize = sizeof(Image_header_dsp);
    Image_header_dsp* pDsp_header = NULL; //Dsp数据包头
    int nTypeFlag = 0; //数据体类型标志

    //采集时间(以微妙为单位)
    struct timeval tv;

    //同步判断
    bool bSynchron =  false;
    bool bSynchron2 =  false;

    int i=0;
    int j=0;
    int iFindFlag = 0; //标记开始找到$的位置
    int index = 0; //标记同步头$$$$后的位置
    int indexContent = 0; //标记去头后紧跟着的具体数据的位置
    int nFlags = 0;//连续出现$的个数
    int iFindFlagR = 0; //标记开始找最右边连续4个$开始的位置

    int nRecvNothingFlag = 0; //未接收到数据标志
    //printf("===11111=======CRoSeekCameraDsp::RecvData========m_bEndCapture=%d====\n", m_bEndCapture);
	//LogNormal("in RecvData m_nTcpFd=%d,m_bEndCapture=%d\n",m_nTcpFd,m_bEndCapture);//add by wantao

	int retRead = 0;
	fd_set readfds;
	bool bRetRead2=false;
	memset(buffer,0,1024);
	memset(buff,0,1024);

        while(m_nTcpFd!=-1 && !m_bEndCapture)
        {
			usleep(1000*10);
			m_nRecvNothingFlag ++;

			if(m_nRecvNothingFlag > 6000) //1分钟以上未收到数据
			{
				m_nRecvNothingFlag = 0; //计数归0
				LogError("=RecvData()==长时间未收到数据=相机连接断开重连!==\n");
				
				//断开重连
				if(m_nTcpFd != -1)
				{
					LogNormal("==RecvData()=长时间未收到数据=重连！\n");	
#ifdef DEBUG_DSP_CMD
					LogNormal("CloseFd [2] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
					CloseFd(m_nTcpFd);
					g_skpRoadLog.WriteLog("相机断开\n",ALARM_CAMERA_BREAK ,true,m_nCamID);
					g_skpDB.SetCameraStateToDBByChannelID(m_nChannelId, CM_BREAK);
				}			
			}

                ///////////////////////////////////////////
                bSynchron =  false;
                bSynchron2 = false;

                    bytes = 0;
                    //接收码流头    
                    bRet = SafeRecv(buffer, nDspHeaderSize, bytes);
                    if(!bRet)
                    {
                        //LogError("接收DSPImg头错误!\n");
						//LogError("接收DSPImg头错误!\n");
                        //break;
						//usleep(20*1000);
                        continue;
                    }
                    else //接收码流头
                    {
                        if( memcmp(buffer, "$$$$", 4) == 0)
                        {                         
                            bSynchron = true; //同步上
                        }
                        else //继续接收数据
                        {
                            bRet = SafeRecv(buff, nRecevCell, bytes);
                            if(!bRet)
                            {                                
                                continue;
                            }
                            else //333
                            {
                                index = 0; //index [0, 4095]
                                //////////////////////////寻找同步头/////////////////////////
                                if(bytes >= nDspHeaderSize)
                                {
                                    bytesRecveHead = bytes;
                                    i=0;
                                    //寻找同步头$$$$
                                    while((i<bytes) && (!m_bEndCapture))
                                    {
                                        if(buff[i] == '$')
                                        {
                                            iFindFlag = i;
                                            iFindFlagR = i;

                                            if( (buff[i+1] == '$') && (buff[i+2] == '$') && (buff[i+3] == '$') )
                                            {
                                                nFlags = 4;

                                                //获取连续出现$的个数
                                                int j=4; //从iFindFlag开始第4个数起
                                                while((buff[iFindFlag+j] == '$')&& (!m_bEndCapture))
                                                {
                                                    j++;
                                                    nFlags++;
                                                }

                                                iFindFlagR = iFindFlag + nFlags - 4;

                                                #ifdef _DSP_DEBUG
                                                if(nFlags > 4)
                                                {
                                                    LogNormal("===find the $$$$=iFindFlagR=%d==nFlags=%d\n", iFindFlagR, nFlags);
                                                }
                                                #endif

                                                index = iFindFlagR + 4;


                                                if( (iFindFlagR + nDspHeaderSize) > nRecevCell )
                                                {
                                                    int nToRecSize = (iFindFlagR + nDspHeaderSize) - nRecevCell;
                                                    //接着接收nToRecSize大小数据
                                                    bRet = SafeRecv(buff+nRecevCell, nToRecSize, bytes);
                                                    if(!bRet)
                                                    {                                                        
                                                        break;
                                                    }
                                                    else
                                                    {                                                        
                                                        bytesRecveHead += bytes;
                                                        //LogNormal("===Recv append header bytes=%d, bytesRecveHead=%d==\n", bytes, bytesRecveHead);
                                                    }
                                                }

                                                bSynchron2 = true;

#ifdef _DSP_DEBUG
		int kk = 0;
		while(kk < 100)
		{
			LogTrace("Recv.log", "[%d] %x", kk, buff[kk]);
			++kk;
		}
#endif
                                                break;

                                            }//End of if( (buff[i+1] == '$') && (buff[i+2] == '$') && (buff[i+3] == '$') )
                                        }//End of if(buff[i] == '$')

                                        i++;
                                    }//End of while(i<bytes)


                                    if(bSynchron2)
                                    {
                                        #ifdef _DSP_DEBUG
                                        printf("===bSynchron2 ok!===\n");
                                        //printf("===iFindFlag=%d===\n", iFindFlag);
                                        #endif

                                        memset(buffer, 0, sizeof(buffer));
                                        memcpy(buffer, "$$$$", 4);
                                        memcpy(buffer+4, buff+index, nDspHeaderSize-4);

                                        bytesRecved = bytesRecveHead - (index-4) - nDspHeaderSize;
                                        indexContent = index + 92;

                                        #ifdef _DSP_DEBUG
                                        printf("===set the buffer==bytesRecved=%d==index=%d==indexContent=%d==\n", bytesRecved, index, indexContent);
                                        #endif

                                        if(bytesRecved < 0)
                                        {
                                            //LogError("==ERROR!!=bytesRecved=%d===\n", bytesRecved);
                                            bSynchron2 = false;
                                            bytesRecved = 0;
                                        }
                                    }//End of if(bSynchron2)
                                }//End of if(bytes >= nDspHeaderSize)
                                //////////////////////////寻找同步头/////////////////////////
                            }//End of else //333
                        }//End of //继续接收数据

                    } //End of else //接收码流头

            #ifdef _DSP_DEBUG
                printf("==========bytes*=======%d\r\n",bytes);
            #endif
                if(bSynchron)
                {
                    #ifdef _DSP_DEBUG
                    printf("======bSynchron ok!==\n");
                    #endif                        

                    //////////////接收数据///Begin/////////////
                    pDsp_header = (Image_header_dsp*)buffer;
                    nFrameSize = pDsp_header->nSize;

                    #ifdef _DSP_DEBUG
					if(pDsp_header->nType != 20)
						LogTrace("Recv.log", "11 type:%d nSeq:%lld size:%d ", pDsp_header->nType, pDsp_header->nSeq, nFrameSize);
                    printf("===nFrameSize=%d===\n", nFrameSize);
                    #endif

					//雷达断开判断
					if(pDsp_header->nType == 2)
					{
						if (pDsp_header->radarState == 0)
						{
							//printf("雷达连接正常\n");
						}
						else if (pDsp_header->radarState == 1)
						{
							//LogNormal("id:%d cam:%d 雷达断开\n", m_nChannelId, m_nCamID);
							g_skpRoadLog.WriteLog("雷达断开\n",ALARM_RADAR_BREAK,true,m_nCamID);
						}
						else if (pDsp_header->radarState == 2)
						{
							printf("雷达未使用\n");
						}
					}
					                     
					memset(listJpgBuff[m_uCurrBuff].pBuff,0,sizeof(listJpgBuff[m_uCurrBuff].pBuff));
                    memcpy(listJpgBuff[m_uCurrBuff].pBuff, buffer, nDspHeaderSize); //拷贝数据包头
					bRet = SafeRecv(listJpgBuff[m_uCurrBuff].pBuff + nDspHeaderSize, nFrameSize, bytes);
                    listJpgBuff[m_uCurrBuff].sizeBuff = nFrameSize;
                    //printf("======RecvData()===bRet=%d=m_uCurrBuff=%d===%d==\n", bRet, m_uCurrBuff);

					m_uPrevBuff = m_uCurrBuff;
					m_uCurrBuff++;
					if(m_uCurrBuff == NUM_BUFF)
					{
						m_uCurrBuff = 0;
					}                    
                    //////////////接收数据///End///////////////

                    //////////////处理接收数据///Begin///////////////
                    if(!bRet) //接收包体数据失败
                    {

                        //LogError("接收DSPImg包体数据错误,strerror(errno)=%s",strerror(errno));
                        continue;
                    }
                    else //接收包体数据成功
                    {
						//LogNormal("接收包体数据成功m_nTcpFd=%d\n",m_nTcpFd);
						//LogTrace("DealOneFrame.log", "=uSeq=%d, nFrameSize=%d=m_uPrevBuff=%d=m_uCurrBuff=%d\n", \
							pDsp_header->nSeq, nFrameSize, m_uPrevBuff, m_uCurrBuff);

						this->DealOneFrame();
                    }
                    //////////////处理接收数据///End///////////////
					//usleep(1000);
                }//End of if(bSynchron)
                else //if(bSynchron)
                {
                    if(bSynchron2)
                    {
                        #ifdef _DSP_DEBUG
                        printf("===bSynchron2 ok!====\n");
                        #endif
                        //////////////接收数据///Begin/////////////
                        pDsp_header = (Image_header_dsp*)buffer;

                        nFrameSize = pDsp_header->nSize;

                        /*****数据类型*****************************************************
                      *  0:jpg小图  1:jpg大图       2:车牌记录  3:卡口抓图  4:闯红灯抓图
                      *  5:逆行抓图  6:线圈触发信息  7:心跳信号  8:YUV源    9:H264源
                      ******************************************************************/
                        nTypeFlag = pDsp_header->nType;

                        #ifdef _DSP_DEBUG
						if(pDsp_header->nType != 20)
							LogTrace("Recv.log", "22 type:%d seq:%lld size:%d ", pDsp_header->nType, pDsp_header->nSeq, nFrameSize);
                        printf("===nFrameSize=%d===nTypeFlag=%d=\n", nFrameSize, nTypeFlag);
                        #endif

                        
                        if(nFrameSize > bytesRecved)
                        {

							memset(listJpgBuff[m_uCurrBuff].pBuff,0,sizeof(listJpgBuff[m_uCurrBuff].pBuff));
                            memcpy(listJpgBuff[m_uCurrBuff].pBuff, buffer, nDspHeaderSize); //拷贝数据包头

                            #ifdef _DSP_DEBUG
                            printf("===indexContent=%d,bytesRecved=%d==111==\n", indexContent, bytesRecved);
                            #endif

                            //拷贝已经接收成功的数据
                            memcpy(listJpgBuff[m_uCurrBuff].pBuff + nDspHeaderSize, buff+indexContent, bytesRecved);
                            bRet = SafeRecv(listJpgBuff[m_uCurrBuff].pBuff + nDspHeaderSize + bytesRecved, nFrameSize-bytesRecved, bytes);

                            if(!bRet)
                            {
                                #ifdef _DSP_DEBUG
                                printf("====bytes=%d==\n", bytes);
                                #endif
                            }
                            /*
                            else
                            {
                                printf("====ERROR!!\n");
                            }*/

                            listJpgBuff[m_uCurrBuff].sizeBuff = nFrameSize;
                            #ifdef _DSP_DEBUG
                            printf("===1111===RecvData()===bRet=%d=m_uCurrBuff=%d====\n", bRet, m_uCurrBuff);
                            #endif
                        }
                        else
                        {
							memset(listJpgBuff[m_uCurrBuff].pBuff,0,sizeof(listJpgBuff[m_uCurrBuff].pBuff));
                            memcpy(listJpgBuff[m_uCurrBuff].pBuff, buffer, nDspHeaderSize); //拷贝数据包头

                            #ifdef _DSP_DEBUG
                            printf("===indexContent=%d,bytesRecved=%d==222==\n", indexContent, bytesRecved);
                            #endif

                            //拷贝已经接收成功的数据
                            memcpy(listJpgBuff[m_uCurrBuff].pBuff+ nDspHeaderSize, buff+indexContent, nFrameSize);
                            listJpgBuff[m_uCurrBuff].sizeBuff = nFrameSize;

                            #ifdef _DSP_DEBUG
                            printf("===222===RecvData()==m_uCurrBuff=%d===%d==\n", m_uCurrBuff);
                            #endif
                        }


						m_uPrevBuff = m_uCurrBuff;
						m_uCurrBuff++;
						if(m_uCurrBuff == NUM_BUFF)
						{
							m_uCurrBuff = 0;
						}
                       
                        //////////////接收数据///End///////////////
                            //////////////处理接收数据///Begin///////////////
                            if(!bRet) //接收包体数据失败
                            {
                                LogTrace("RecvData-TT.log", "接收DSPImg包体2数据错误,strerror(errno)=%s, nFrameSize=%d",\
									strerror(errno), nFrameSize);
                                continue;
                            }
                            else //接收包体数据成功
                            {
								//LogTrace("RecvData-TT.log", "line 2541接收包体数据成功m_nTcpFd=%d, nFrameSize=%d\n",\
									m_nTcpFd, nFrameSize);

								LogTrace("DealOneFrame.log", "=22=uSeq=%lld, nFrameSize=%d=m_uPrevBuff=%d=m_uCurrBuff=%d\n", \
									pDsp_header->nSeq, pDsp_header->nSize, m_uPrevBuff, m_uCurrBuff);
								this->DealOneFrame();

                            }
                            //////////////处理接收数据///End///////////////
                    }//End of if(bSynchron2)
                    else
                    {
                        //LogNormal("bSynchron2=%d=bytesRecved=%d\n", bSynchron2, bytesRecved);
                    }
                }//End of else if(bSynchron)

				memset(buffer,0,1024);
				memset(buff,0,1024);
		} // End of while(m_nTcpFd!=-1 && !m_bEndCapture)  

		LogNormal("--CRoSeekCameraDsp::RecvData--out!\n");
}


//TCP 连接
bool CRoSeekCameraDsp::connect_tcp()
{
    //LogNormal("===CRoSeekCameraDsp::connect_tcp===m_nTcpFd=%d==\n", m_nTcpFd);

    //如果m_nTcpFd还存活，就关闭它
    if(m_nTcpFd != -1)
    {
        //LogError("=shutdown==m_nTcpFd=%d==\n", m_nTcpFd); 
#ifdef DEBUG_DSP_CMD
		LogNormal("CloseFd [3] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
		CloseFd(m_nTcpFd);
    }

    //LogNormal("===CRoSeekCameraDsp::connect_tcp=close==m_nTcpFd=%d==", m_nTcpFd);
    //////////////////////////////

    struct sockaddr_in tcp_addr;
    // socket
    //if ((m_nTcpFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET)
	if ((m_nTcpFd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");

		//LogTrace("ConnTcp.log", "=1111=aconnect_tcp===m_nTcpFd=%d=", m_nTcpFd);
        return false;
    }
    //LogNormal("==CRoSeekCameraDsp::connect_tcp==after=socket Init=m_nTcpFd=%d===\n", m_nTcpFd);

	LogNormal("connect_tcp m_nTcpFd=%d", m_nTcpFd);

	if(m_nTcpFd == 0)
	{
		LogError("===close 0!\n");
		//关闭标准输入
		//	close(0);
		//	printf("===after close 0!\n");
		//	return false;
	}
	else
	{
		linger InternalLinger = { 1, 0 };
		setsockopt(m_nTcpFd, SOL_SOCKET, SO_LINGER, (const char*)&InternalLinger, sizeof(linger));
		int on = 1;
		setsockopt(m_nTcpFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
	}

    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec = 1;
    timeo.tv_usec=500000;//超时1.5s

    if(setsockopt(m_nTcpFd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
    {
        perror("setsockopt");
        if(m_nTcpFd!=-1)
        {
#ifdef DEBUG_DSP_CMD
			LogNormal("CloseFd [4] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
			CloseFd(m_nTcpFd);
        }
        return false;
    }

	timeo.tv_sec = 1;
    timeo.tv_usec=500000;//超时1.5s

    if(setsockopt(m_nTcpFd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {
        if(m_nTcpFd!=-1)
        {        
#ifdef DEBUG_DSP_CMD
			LogNormal("CloseFd [5] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
			CloseFd(m_nTcpFd);
        }
        perror("setsockopt");
        return false;
    }

    bzero(&tcp_addr,sizeof(tcp_addr));
    tcp_addr.sin_family=AF_INET;
    tcp_addr.sin_addr.s_addr=inet_addr(m_strCameraIP.c_str());
    tcp_addr.sin_port=htons(m_nPort);

    //LogNormal("==connect_tcp==m_strCameraIP.c_str()=%s,m_nPort=%d\n", m_strCameraIP.c_str(), m_nPort);
    int nCount = 0;
    while(nCount < 3 && (!m_bEndCapture))//连接不上最多重试三次
    {
        if(connect(m_nTcpFd,(struct sockaddr *)&tcp_addr,sizeof(struct sockaddr))==-1)
        {
			//LogError("connect:%s,m_nTcpFd=%d\n",strerror(errno),m_nTcpFd);

            if(nCount == 2)
            {
                //perror("connect");
                if(m_nTcpFd!=-1)
                {
#ifdef DEBUG_DSP_CMD
					LogNormal("CloseFd [6] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
					CloseFd(m_nTcpFd);
                }
				//LogTrace("ConnTcp.log", "=4444=aconnect_tcp===m_nTcpFd=%d=", m_nTcpFd);
                return false;
            }
        }
        else
        {
            break;
        }
        //LogError("connect_tcp nCount=%d",nCount);
        nCount++;
        usleep(1000*10);
    }

	/*linger InternalLinger = { 1, 0 };
	setsockopt(m_nTcpFd, SOL_SOCKET, SO_LINGER, (const char*)&InternalLinger, sizeof(linger));
	int on = 1;
	setsockopt(m_nTcpFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );*/

	//LogTrace("ConnTcp.log", "==after==CRoSeekCameraDsp::connect_tcp===m_nTcpFd=%d=", m_nTcpFd);

	ConnectCamera();

	LogNormal("# 连接相机成功!=m_nTcpFd=%d=ip=[%s]\n", m_nTcpFd,m_strCameraIP.c_str());

    return true;
}

//
void CRoSeekCameraDsp::RecvRTSP()
{
		//采集时间(以微妙为单位)
		struct timeval tv;
		bool bInitRtsp = false;
		CRtspClient RtspClient;
		
//#ifdef REINIT_RTSP
		int nNoDataTime = 12000;//60s = 0.005*12000
		int nNoDataCounts = 0;//未收到数据标志位
		m_nRecvRtspFlag = 0;
//#endif

		UINT64 uSeq = 0;
		while(!m_bEndCapture)
		{
			 //LogNormal("进入dsp相机接收视频\n");
			
			if(bInitRtsp)
			{
				if(m_bH264Capture || m_bSkpRecorder)
				{
					std::string strData;
					RtspClient.PopFrame(strData);

					if(strData.size() > 0)
					{
						gettimeofday(&tv,NULL);
						uSeq ++;

//#ifdef REINIT_RTSP
						nNoDataCounts = 0;//计数归0
						m_nRecvRtspFlag = 0;
//#endif
#ifdef RECORDER_DEBUG
						//if(strData.size() < 1000)
						{
							if(access("/detectdata",0) != 0) //目录不存在
							{
								LogTrace("video.log", "chan:%d uSeq:%d strData:%d ", \
									m_nChannelId, uSeq, strData.size());
							}
							else
							{
								LogTrace("/detectdata/video.log", "chan:%d uSeq:%d strData:%d ", \
									m_nChannelId, uSeq, strData.size());
							}							
						}						
#endif

						if(m_pH264Capture != NULL && m_bH264Capture)
						{
							//printf("===bytes=*%d==\n", bytes);
							Image_header_dsp header; //添加H264图像头
							header.nType = DSP_IMG_H264_PIC; //标记H264码流类型
							//header.nHeight = 1200;
							//header.nWidth = 1600;
							unsigned short w1, h1;
							GetCamVideoInfo(w1, h1);
							header.nWidth = w1;
							header.nHeight = h1;							

							header.nFrameRate = 10;
							header.nSize = strData.size();
							header.nSeq = uSeq;
							//取当前机器时间
							header.ts = tv.tv_sec * 1000000 + tv.tv_usec;

							m_pH264Capture->AddFrame2((unsigned char*)strData.c_str(), header);
						}
						
						if(m_pTempCapture != NULL && m_bSkpRecorder)
						{
							Image_header_dsp header; //添加H264图像头
							header.nType = DSP_IMG_H264_PIC; //标记H264码流类型				
							unsigned short w1, h1;
							GetCamVideoInfo(w1, h1);
							header.nWidth = w1;
							header.nHeight = h1;							

							header.nFrameRate = 10;
							header.nSize = strData.size();
							header.nSeq = uSeq;
							//取当前机器时间
							header.ts = tv.tv_sec * 1000000 + tv.tv_usec;

							m_pTempCapture->AddFrame((unsigned char*)strData.c_str(), header);
						}			
					}//End of if(strData.size() > 0)
					else
					{
//#ifdef REINIT_RTSP
						++nNoDataCounts;
						if(nNoDataCounts > nNoDataTime)
						{
							LogNormal("chanId:%d 重设bInitRtsp nNoDataCounts:%d !", m_nChannelId, nNoDataCounts);
							RtspClient.UnInit();
							bInitRtsp = false;
							LogNormal("id:%d bInitRtsp :%d", m_nChannelId, bInitRtsp);

							nNoDataCounts = 0;//计数归0
							++m_nRecvRtspFlag;

							if(m_nRecvRtspFlag > 9)//10分钟未收到rtsp数据,重启相机
							{
								CAMERA_CONFIG cfg; 
								cfg.uType = 1; 
								cfg.nIndex = (int)CAMERA_RESTART;
								this->ManualControl(cfg);
								LogNormal("10分钟未收到rtsp数据,重启相机!");
								m_nRecvRtspFlag = 0;
							}
						}
//#endif
					}
				}//End of if(m_bH264Capture || m_bSkpRecorder)
				else
				{		
					RtspClient.UnInit();
					bInitRtsp = false;
					LogNormal("id:%d bInitRtsp :%d", m_nChannelId, bInitRtsp);
				}
			}//End of if(bInitRtsp)
			else
			{
				bool bConnect = this->GetCameraState();
				if(bConnect)
				{
					//获取相机类型
					for(int index = 0;index<3;index++)
					{
						if(m_nCameraBrand > 0)
							break;

						GetCameraBrand();
						if(m_nCameraBrand > 0)
						{
							LogNormal("dsp:%s 相机厂商值%d\n",m_strCameraIP.c_str(),m_nCameraBrand);
							break;
						}
						//usleep(10*1000);
						sleep(5);
					}
				}

				if(m_bH264Capture || m_bSkpRecorder)
				{		
					//相机商标1:锐视 2:大华 3:博康 4:英泰智 5:港宇 6:低噪度（英泰智）
					int nServerPort = 1888 + m_nChannelId * 3;
					RtspClient.SetReceivePort(nServerPort); //1888 + 通道号做为端口号传进来
				
					if(m_nCameraBrand == 2 || m_nCameraType == DH_203_M || m_nCameraType == DH_523_M || m_nCameraType == DH_213_M || m_nCameraType == DSP_DH)//大华相机
					{
						RtspClient.Init(m_strCameraIP.c_str(),554,"cam/realmonitor?channel=1&subtype=0");
					}
					else if(m_nCameraBrand == 5 || m_nCameraBrand == 8 ||m_nCameraBrand == 9 ||m_nCameraBrand == 10 || m_nCameraPort == 554)//港宇相机
					{
						RtspClient.Init(m_strCameraIP.c_str(),554,"h264");
					}
					else//锐视和英泰智相机
					{
						RtspClient.Init(m_strCameraIP.c_str(),8557,"h264");
					}	

					bInitRtsp = true;
					LogNormal("id:%d bInitRtsp :%d", m_nChannelId, bInitRtsp);
				}
			}
			usleep(1000*5);
		}		
		
		if(bInitRtsp)
		{
			RtspClient.UnInit();
			bInitRtsp = false;
			LogNormal("id:%d bInitRtsp :%d", m_nChannelId, bInitRtsp);
		}
}


//接收数据包
bool CRoSeekCameraDsp::SafeRecv(char*pBuf, const int nLen, int &nRecvSize)
{
	if(0 == nLen)
	{
		return true;
	}

	if(m_nTcpFd < 0 || nLen > MAX_BUFF_TAG_SIZE)
	{
		if(nLen > MAX_BUFF_TAG_SIZE)
		{
			LogNormal("SafeRecv error! nLen:%d ", nLen);
		}

		return false;
	}
	
	int nRet = 0;
	int nRecved = 0;
	bool bRet = true;

	int nSockFlag = WaitForSockCanRead(m_nTcpFd, 2000);

	if(nSockFlag <= 0)
	{
		m_nReadCount ++;

		//LogNormal("-id[%d]--SafeRecv-m_nReadCount-=%d-\n", m_nChannelId, m_nReadCount);

		if(m_nReadCount >= 15)//30秒，未收到数据
		{
			if(m_nTcpFd >= 0)
			{
				//LogNormal("=close =m_nTcpFd=%d=SafeRecv=\n", m_nTcpFd);
#ifdef DEBUG_DSP_CMD
				LogNormal("CloseFd [7] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
				CloseFd(m_nTcpFd);
			}
			m_nReadCount = 0;
		}
	}

	int nCount = 0;
	const int nCountNums = 3000;
	while ((nRecved < nLen) && nSockFlag > 0 && (!m_bEndCapture) && (nCount < nCountNums))
	{
		if(m_nTcpFd < 0)
		{
			return false;
		}
		
		nCount++;
		m_nRecvNothingFlag++;

		//nRet = recv(m_nTcpFd, pBuf + nRecved, nLen - nRecved, MSG_NOSIGNAL);
		nRet = recv(m_nTcpFd, pBuf + nRecved, nLen - nRecved, MSG_WAITALL);
		if (nRet <= 0)
		{
			int nErrCode = errno;
			bRet = false;
			//printf("nRet=%d,nErrCode=%d,errno=%d,[%s] nCount:%d m_nReadCount:%d\n",\
			//	nRet, nErrCode, errno, strerror(errno), nCount, m_nReadCount);
			//LogError("recv error nRet=%d,errno=%s\n",nRet,strerror(errno));

			if((nRet == 0) && (errno == 0) || (nRet < 0))
			{
				if(nCount >= 1000) //未收到数据次数过多关闭当前soket,10s
				{
					//LogNormal("未收到数据次数过多关闭当前soket. nRet:%d ", nRet);
#ifdef DEBUG_DSP_CMD
					LogNormal("CloseFd [9] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
					CloseFd(m_nTcpFd);
					break;
				}
				
			}
			else
			{
				bool bDeal = DealSafeRecvError(nErrCode);
				if(bDeal)
				{
					continue;
				}
				else
				{
					break;
				}
			}

			usleep(10*1000);
			continue;
		}
		
		nRecved += nRet;
		m_nReadCount = 0;
		
		//printf("nLen:%d nRecvSize:%d nRecved:%d nRet:%d \n", nLen, nRecvSize, nRecved, nRet);
	}

	if(nCount >= nCountNums)
	{
//#ifdef DEBUG_DSP_CMD
		LogNormal("CloseFd [13] ID:%d m_nTcpFd:%d nCount:%d \n", m_nChannelId, m_nTcpFd, nCount);
//#endif
		CloseFd(m_nTcpFd);
	}

	if(nSockFlag > 0 && nRecved == 0)
	{
		m_nReadCount ++;
		//LogNormal("-22--SafeRecv-m_nReadCount-=%d-\n", m_nReadCount);

		if(m_nReadCount >= 60)//120秒，未收到数据
		{
			if(m_nTcpFd >= 0)
			{
				//LogNormal("22=close =m_nTcpFd=%d=SafeRecv=\n", m_nTcpFd);	
#ifdef DEBUG_DSP_CMD
				LogNormal("CloseFd [10] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
				CloseFd(m_nTcpFd);
			}
			m_nReadCount = 0;
		}
	}

	if (nRecved < nLen)
	{
		bRet = false;
	}
	else
	{
		bRet = true;
	}
	nRecvSize = nRecved;

	return bRet;
}

//命令转换，数字->字符串
/*
dwCmd:命令数值
pParam:命令字符串
dwParamLen:命令长度(dwParamLen<4)
*/
bool CRoSeekCameraDsp::ConvertCMDToString(const unsigned int dwCmd, char * pParam, const unsigned int  &dwParamLen)
{
    if(dwParamLen > 4 || pParam == NULL)
    {
        return false;
    }

    //pParam = new char[dwParamLen];

    unsigned char chTmp;

    for(int i=0; i<dwParamLen; i++)
    {
        chTmp = (unsigned char)((dwCmd >> (8 * i)) & 0x000000FF);
        pParam[i] = chTmp;
        printf("==CRoSeekCameraDsp::ConvertCMDToString===###########===pParam[%d]=%x====\n", i, pParam[i]);
    }

    return true;
}

//组织处理一帧接收的图像
bool CRoSeekCameraDsp::DealOneFrame()
{
    #ifdef _DSP_DEBUG
    printf("====in CRoSeekCameraDsp::DealOneFrame()...===\n");
    #endif

	int nTimeNow = GetTimeStamp();//记录当前时间戳
	m_nRecvLastTime = nTimeNow;
	m_nRecvNothingFlag = 0; //计数归0
    int nTypeFlag = 0; //数据体类型标志
    struct timeval tv;
    bool bCheckCrc = true;

    bool bRightPlateFlag = true; //车牌结果信息合法性标志
    {
        #ifdef _DSP_DEBUG
            printf("=======DealOneFrame()=m_uPrevBuff=%d==m_uCurrBuff=%d==\n", m_uPrevBuff, m_uCurrBuff);
        #endif
            gettimeofday(&tv,NULL);

            //转换图像头Image_header_dsp为yuv_video_buf 格式
            Image_header_dsp *pDsp_header = (Image_header_dsp *)(listJpgBuff[m_uPrevBuff].pBuff);

            //类型
            /*
            * 0:jpg小图       1:jpg大图       2:车牌记录    3:卡口抓图      4:闯红灯抓图
            * 5:逆行抓图      6:线圈触发信息  7:心跳信号    8:YUV源         9:H264源
            * 10:事件记录    11:线圈抓拍图    12:视频抓拍图
            */
            nTypeFlag = pDsp_header->nType;
			//printf("---###===nTypeFlag=%d---\n", pDsp_header->nType);

			//LogNormal("tts:%lu s", pDsp_header->ts/1000/1000);
			//LogNormal("id:%d,type:%d,size:%d,time:%s", \
			//	m_nChannelId, nTypeFlag, pDsp_header->nSize,GetTime((pDsp_header->ts/1000/1000),3));
#ifdef DSP_LOG_ON
			LogTrace("Add-Frame.log", "Id[%d]=nTypeFlag=%d=pDsp_header->nSeq=%d size:%d timeDsp:%s ", \
				m_nChannelId, nTypeFlag, pDsp_header->nSeq, pDsp_header->nSize, GetTime((pDsp_header->ts/1000/1000),3));
#endif

			//LogNormal("=#=DealOneFrame()=nTypeFlag=%d=\n", nTypeFlag);
			//判断心跳信号
			if(nTypeFlag == DSP_IMG_LINK && pDsp_header->nSize == 0) //心跳
			{
				//LogTrace("DspLinkTest.log", "==DSP=====Link TEST!!===\n");							
				//LogNormal("-==DSP=====Link TEST!!=m_nReadCount=%d--\n", m_nReadCount);
				//m_nReadCount = 0;
				return true;
			}
			else if(nTypeFlag == DSP_REBOOT) //收到相机重启状态
			{
				LogNormal("DSP_REBOOT!\n");
				m_bCamReboot = true;				
				CheckCameraReboot();
				return true;
			}
			else
			{
				//
			}
			
#ifdef LED_STATE
			//led灯状态判断
			if(pDsp_header->nType == 1)
			{
				int nTimeNow = GetTimeStamp();
				int nTimeDis = nTimeNow - m_nWarmLastTime;
				
				if(nTimeDis > 300)
				{
					char chLedState = pDsp_header->ledState;
					
					//LogNormal("sq:%lld, ledstate 0x:%x", pDsp_header->nSeq, pDsp_header->ledState);					
					//test
					//for(int i=0; i<257; i++)
					//{
					//	CheckLedState(i);
					//}
					//核查爆闪灯状态
					CheckLedState(chLedState);
					
					m_nWarmLastTime = nTimeNow;
				}						
			}
#endif

			//检测器接收相机数据时间戳,单位s.
			pDsp_header->uRecvTs = nTimeNow;

			//比较时间判断
			int nTempTs = (pDsp_header->ts) * 0.001 * 0.001;
			int nTempDis = abs(tv.tv_sec - nTempTs);
			if(nTempDis > 1000)
			{
				LogNormal("=Pc_time:[%d][%s]=Cam_time:[%d][%s]=nDis=%d \n", tv.tv_sec,GetTime(tv.tv_sec).c_str(), nTempTs,GetTime(nTempTs).c_str(), nTempDis);
			}

            UINT32 HeaderLength = sizeof(Image_header_dsp);
			pDsp_header->nFrameRate = GetDspCameraFrequency(m_nCameraType);

            yuv_video_buf camera_header;						
			SetCameraHeader(pDsp_header, camera_header);
            
            if (!m_bEndCapture)
            {
                
                if(nTypeFlag == DSP_IMG_PLATE_INFO)
                {

                    RECORD_PLATE_DSP plate;
                    memcpy((BYTE*)(&plate), listJpgBuff[m_uPrevBuff].pBuff + HeaderLength, sizeof(RECORD_PLATE_DSP));
					
					
					//LogNormal("id:%d uSeq:%d, car:%s ", m_nChannelId, plate.uSeq, plate.chText);
#ifdef DSP_LOG_ON			 
					LogTrace("Vts-Text.log", "=nSeq=%d=nTypeFlag=%d=uViolationType=%d=uSeq=[%d,%d,%d]-ndire=%d plate:%s \n", \
					pDsp_header->nSeq, nTypeFlag, plate.uViolationType, plate.uSeq, plate.uSeq2, plate.uSeq3, plate.uDirection, plate.chText);					              
                    //printf("==###########################=====m_dspPlatelist.size()=%d=====\n", m_dspPlatelist.size());             
#endif

					//车牌数据强制违章类型为99
					plate.uViolationType = 99;

					#ifdef CENTOS_32BIT
					if(plate.uSeq != 0xFFFFFFFF) //非保持连接测试包
					#else
                    if(plate.uSeq != 0xFFFFFFFFFFFFFFFF) //非保持连接测试包
					#endif
					{
					    //添加判断车牌相关信息合法性判断--暂时只判断车牌号的前7位是否满足
					    bRightPlateFlag = CheckPlateInfo(plate);                   
						if(bRightPlateFlag)
                        {
							//图片与视频不关联时直接取图片接收时间
							//plate.uTime = tv.tv_sec;
							//plate.uMiTime = tv.tv_usec/1000;						
							{
								plate.uTime = (pDsp_header->ts / 1000) / 1000; //单位秒s
								plate.uMiTime = (pDsp_header->ts / 1000) % 1000;
							}


                            AddPlateFrame(plate);//传大图
                        }
                        else
                        {							
							if(plate.chText[0] == '*' && plate.chText[6] == '*')
							{
								plate.uTime = (pDsp_header->ts / 1000) / 1000; //单位秒s
								plate.uMiTime = (pDsp_header->ts / 1000) % 1000;

								plate.uColor = CARNUM_OTHER; //其他颜色
								plate.uPlateType = 99;//其他（无单双之说）

								AddPlateFrame(plate);//传大图
							}
							else
							{
								//记录日志
								LogNormal("can't AddPlateFrame===plate.chText=%s,uViolationType=%d\n", plate.chText,plate.uViolationType);
							}							
                        }

                        // 存文件
                #ifdef ROSEEK_SAVE_INFO
						int nRand = rand();
                        FILE *fpOut = NULL;
                        char jpg_name[256] = {0};
                        //存信息
                        if(pDsp_header->nType == 2)
                        {
                            sprintf(jpg_name, "./text/%d_img_%d_%d_plate_%d_%d.data", \
								pDsp_header->nSeq, nTypeFlag, plate.uViolationType, plate.uSpeed, nRand);
                            fpOut = fopen(jpg_name, "wb+");
							if(fpOut)
							{
								fwrite((char*)(&plate), 1, sizeof(RECORD_PLATE_DSP) , fpOut);
								fclose(fpOut);
							}
                        }
                #endif
                    }//End of if(plate.uSeq != 0xFFFFFFFFFFFFFFFF) //非保持连接测试包
                }
                else if(nTypeFlag == DSP_IMG_GATE_PIC || nTypeFlag == DSP_IMG_VTS_PIC \
                        || nTypeFlag == DSP_IMG_CONVERS_PIC || nTypeFlag == DSP_IMG_LOOP_INFO \
                        || nTypeFlag == DSP_IMG_BIG_PIC || nTypeFlag == DSP_IMG_LOOP_PIC \
                        || nTypeFlag == DSP_IMG_VIDEO_PIC || nTypeFlag == DSP_IMG_BIG_PIC2)//大图-线圈卡口-电警方案
                {
				#ifdef DEBUG_GAIN
					LogNormal("id:%d uSeq:%d, nBglight:%d,nSh:%d,nGain:%d ", \
							 m_nChannelId, pDsp_header->nSeq, pDsp_header->nBglight, pDsp_header->nSh, pDsp_header->nGain);
				#endif
					//LogNormal("in CRoSeekCameraDsp::DealOneFrame() 传大图\n");
                    camera_header.size = pDsp_header->nSize + HeaderLength;
                    camera_header.nFrameRate = pDsp_header->nFrameRate;
                    camera_header.data = m_pFrameBuffer2 + sizeof(yuv_video_buf);
                    memcpy(m_pFrameBuffer2, &camera_header, sizeof(yuv_video_buf));
                    //数据结构：[yuv_video_buf] + [Image_header_dsp] + [JpgData]
                    memcpy(m_pFrameBuffer2 + sizeof(yuv_video_buf), listJpgBuff[m_uPrevBuff].pBuff, camera_header.size);

                  //  printf("=Before=AddJpgFrame=camera_header.size=%d====\n", camera_header.size);
                  //  printf("==###########################=====m_nFrameSize2=%d=====\n", this->m_nFrameSize2);
                   // printf("==####=====pDsp_header->nCount=%d===\n", pDsp_header->nCount);
					//LogNormal("传大图之前相机编号:%d\n",pDsp_header->uCameraId);

					AddJpgFrame(2);//传大图
					
					if(g_nServerType == 13)
					{
						if(m_bSendCapturePic)
						{
							GenerateRecord(pDsp_header);
							m_bSendCapturePic = false;
						}
					}

#ifdef DSP_LOG_ON
                    LogTrace("Add-Big-Pic.log", "Id[%d]=nTypeFlag=%d=pDsp_header->nSeq=%d\n", \
						m_nChannelId, nTypeFlag, pDsp_header->nSeq);
#endif
                #ifdef ROSEEK_SAVE_INFO
                            FILE *fpOut = NULL;
                            char jpg_name[256] = {0};
                            //if(pDsp_header->nType == 1)
                            {
                                sprintf(jpg_name, "./text/%d_%d_seq_Header.txt", pDsp_header->nSeq, pDsp_header->nRoadIndex);
                                fpOut = fopen(jpg_name, "wb");
								if(fpOut)
								{
									fwrite(listJpgBuff[m_uPrevBuff].pBuff, 1, HeaderLength, fpOut);
									fclose(fpOut);
								}
                            }
                #endif

               #ifdef ROSEEK_SAVE_INFO
                        if(nTypeFlag == DSP_IMG_LOOP_INFO)
                        {
                            FILE *fpOut = NULL;
                            char jpg_name[256] = {0};
                            //if(pDsp_header->nType == 1)
                            {
                                sprintf(jpg_name, "./text/%d_%d_seq_loopinfo.txt", pDsp_header->nSeq, pDsp_header->nRoadIndex);
                                fpOut = fopen(jpg_name, "wb");
								if(fpOut)
								{
									fwrite(listJpgBuff[m_uPrevBuff].pBuff + HeaderLength, 1, pDsp_header->nSize, fpOut);
									fclose(fpOut);
								}
                            }

                            LogNormal("Add Frame Loop nSeq=%d=\n", pDsp_header->nSeq);
                        }
                        else
                        {
                            LogNormal("Add Frame Big Pic nSeq=%d=\n", pDsp_header->nSeq);
                        }
                #endif


                #ifdef ROSEEK_SAVE_INFO

                    if(nTypeFlag >= 1)
                    {
						int nRand = rand();
                        FILE *fpOut = NULL;
                        char jpg_name[256] = {0};
                        //if(pDsp_header->nType == 1)
                        {
                            sprintf(jpg_name, "./text/%d_Big_%d_%d_%d_seq.jpg", \
								pDsp_header->nSeq, nTypeFlag, pDsp_header->nRoadIndex, nRand);
                            fpOut = fopen(jpg_name, "wb");
							if(fpOut)
							{
								fwrite(listJpgBuff[m_uPrevBuff].pBuff + sizeof(Image_header_dsp), 1, camera_header.size, fpOut);
								fclose(fpOut);
							}
                        }
                    }

                #endif
                }
				else if(nTypeFlag == DSP_IMG_EVENT_INFO)
				{
					RECORD_PLATE_DSP plate;
					memcpy((BYTE*)(&plate), listJpgBuff[m_uPrevBuff].pBuff + HeaderLength, sizeof(RECORD_PLATE_DSP));

#ifdef DSP_LOG_ON
					LogTrace("Vts-Text.log", "ID[%d]==22=nSeq=%d=nTypeFlag=%d=uViolationType=%d=uSeq=[%d,%d,%d,%d]-ndir=%d carnum:%s \n", \
						m_nChannelId, pDsp_header->nSeq, nTypeFlag, plate.uViolationType, plate.uSeq, plate.uSeq2, plate.uSeq3, plate.uSeq4, plate.uDirection, plate.chText);
					//LogTrace("Vts-Text.log", "CarPost [%d,%d,%d,%d]\n", plate.uCarPosLeft, plate.uCarPosTop, plate.uCarPosRight, plate.uCarPosBottom);
#endif


					plate.uSeq4 = pDsp_header->nSeq;


						//添加判断车牌相关信息合法性判断--暂时只判断车牌号的前7位是否满足
						bRightPlateFlag = CheckPlateInfo(plate);

						if(bRightPlateFlag)
						{
							//图片与视频不关联时直接取图片接收时间
							//plate.uTime = tv.tv_sec;
							//plate.uMiTime = tv.tv_usec/1000;
							
							{
								plate.uTime = (pDsp_header->ts / 1000) / 1000; //单位秒s
								plate.uMiTime = (pDsp_header->ts / 1000) % 1000;
							}
							

							AddPlateFrame(plate);//传大图
						}
						else
						{							
							if(plate.chText[0] == '*' && plate.chText[6] == '*')
							{
								plate.uTime = (pDsp_header->ts / 1000) / 1000; //单位秒s
								plate.uMiTime = (pDsp_header->ts / 1000) % 1000;

								plate.uColor = CARNUM_OTHER; //其他颜色
								plate.uPlateType = 99;//其他（无单双之说）

								AddPlateFrame(plate);//传大图
							}
							else if(ELE_PARKING_VIOLATION == plate.uViolationType || /* 1	违章停车 */
								DSP_EVT_JAM == plate.uViolationType ||				/*24    交通拥堵，对应事件DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_JAM*/
								DSP_EVT_PASSBY == plate.uViolationType ||				/*25	行人横穿, DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_PASSERBY,	*/
								DSP_EVT_SLOW == plate.uViolationType ||					/*26	车辆慢行, DETECT_RESULT_TYPE::DETECT_RESULT_EVENT_GO_SLOW,  */
								DSP_EVT_PERSON_STOP == plate.uViolationType ||			/*27	DETECT_RESULT_EVENT_PERSON_STOP,	//9 行人停留			*/
								DSP_EVT_WRONG_CHAN == plate.uViolationType ||			/*28	DETECT_RESULT_EVENT_WRONG_CHAN,		//10 非机动车出现		*/
								DSP_EVT_PERSON_AGAINST == plate.uViolationType ||		/*29	DETECT_RESULT_EVENT_PERSON_AGAINST, //11 行人逆行			*/
								DSP_EVT_CROSS == plate.uViolationType ||				/*30	DETECT_RESULT_EVENT_CROSS,		    //12 车辆横穿			*/
								DSP_EVT_PERSON_APPEAR == plate.uViolationType ||		/*31	DETECT_RESULT_EVENT_PERSON_APPEAR,  //13 行人出现			*/
								DSP_EVT_APPEAR == plate.uViolationType 	||				/*32	DETECT_RESULT_EVENT_APPEAR,		    //14 机动车出现			*/
								DSP_EVT_DECILIT == plate.uViolationType  || 				/*33	遗弃物 */
								EVT_NO_PARKING == plate.uViolationType 	 ||			/*34	非停车区域等候(黄网格)	*/
								EVT_BREAK_GATE == plate.uViolationType ||
								EVT_NOT_CUTRTESY_DRIVE == plate.uViolationType)
							{
								plate.uTime = (pDsp_header->ts / 1000) / 1000; //单位秒s
								plate.uMiTime = (pDsp_header->ts / 1000) % 1000;

								plate.uColor = CARNUM_OTHER; //其他颜色
								plate.uPlateType = 99;//其他（无单双之说）

								AddPlateFrame(plate);//传大图
							}
							else
							{
								//记录日志
								LogNormal("====can't AddPlateFrame==22=plate.chText=%s,uViolationType=%d\n", plate.chText,plate.uViolationType);
							}						
						}

						// 存文件
				#ifdef ROSEEK_SAVE_INFO
						int nRand = rand();
						FILE *fpOut = NULL;
						char jpg_name[256] = {0};
						//存信息
						if(pDsp_header->nType == 2)
						{
							sprintf(jpg_name, "./text/%d_img_%d_VTS_%d_plate_%d_%d.data", \
								pDsp_header->nSeq, nTypeFlag,  plate.uViolationType, plate.uSpeed, nRand);
							fpOut = fopen(jpg_name, "wb+");
							if(fpOut)
							{
								fwrite((char*)(&plate), 1, sizeof(RECORD_PLATE_DSP) , fpOut);
								fclose(fpOut);
							}
						}
				#endif
				}
				else if(DSP_LOG == nTypeFlag)
				{					
					//char *pBuff = listJpgBuff[m_uPrevBuff].pBuff;
					//int nBuffSize = listJpgBuff[m_uPrevBuff].sizeBuff;

					if((access("DspLog",F_OK) == 0)||(g_nDspLog == 1))
					{
						//LogNormal("logDsp size:%d ", nBuffSize);
						//处理相机日志
						DealDspLog(listJpgBuff[m_uPrevBuff].pBuff, listJpgBuff[m_uPrevBuff].sizeBuff);
					}
				}
                else
                {}
            }//End of if (!m_bEndCapture)

#ifdef _DSP_DEBUG
            printf("==========camera_header.size=%d==\n", camera_header.size);
#endif
    }

    ////////////////////////////////////////////
#ifdef _DSP_DEBUG
    printf("====out CRoSeekCameraDsp::DealOneFrame()...=================\n");
#endif

    return true;
}

//校时处理
bool CRoSeekCameraDsp::SynClock()
{
    int nCount = 0;
	int nLinkFailCount = 0;//心跳发送失败计数
    while(!m_bEndCapture)
	{
	    #ifdef _DSP_DEBUG
		printf("enter SynClock control nCount:%d \n", nCount);
		#endif

		//监测接收图片数据线程状态
		bool bCheck = CheckCaptureStatus();

		if(bCheck)
		{
			//每隔20min校时一次
			if(nCount%1200 == 0)
			{
				SetClock();
			}

			//每隔3分钟发一次心跳给相机
			if((nCount+5) % 180 == 0)
			{
				bool bDealLink = DealCmdLink(nLinkFailCount);
			}

			//自动抓拍图片
			if(g_nServerType == 13)
			{
				if(nCount%20 == 0)
				{
					//LogNormal("===AutoCapturePic\n");
					AutoCapturePic();
				}
			}

		}//End of if(bCheck)
		
		sleep(5);

		nCount += 5;
	}

	LogNormal("exit SynClock control! id:%d \n", m_nChannelId);
}

//每隔3分钟发一次心跳给相机
bool CRoSeekCameraDsp::DealCmdLink(int &nLinkFailCount)
{
	bool bRet = false;

	CAMERA_CONFIG  cfg = m_cfg;
	cfg.nIndex = CAMERA_SEND_LINK;

	//LogNormal("=SynClock==nCount=%d=\n", nCount);
	int nLink = Control(cfg);
	//LogNormal("=nLink=%d=\n", nLink);

	if(nLink < 1)
	{
		//LogNormal("==发送心跳失败==");
		nLinkFailCount ++;
		int nReSendTimes = 3;
		while(nReSendTimes > 0 && !m_bEndCapture)
		{
			sleep(5);
			//发送失败重发，间隔20s
			nLink = Control(cfg);

			if(nLink > 0)
			{
				nLinkFailCount = 0;
				break;
			}
			else
			{
				nLinkFailCount++;
			}

			--nReSendTimes;
		}//End of while(nReSendTimes > 0)					

		if((nLinkFailCount > 2) && (nLink < 1))
		{
			LogNormal("ID:%d nLinkFailCount=%d,发送心跳失败!\n", m_nChannelId, nLinkFailCount);
			nLinkFailCount = 0;
		}
	}

	return bRet;
}

//车牌相关信息校验
bool CRoSeekCameraDsp::CheckPlateInfo(RECORD_PLATE_DSP &plate)
{	
	//LogNormal("id:%d seq:%lld, %lld, %lld car:%s vio:%d dir:%d", \
	//	m_nChannelId,plate.uSeq, plate.uSeq2, plate.uSeq3, plate.chText, plate.uViolationType, plate.uDirection);
	
#ifdef DSP_LOG_ON
	LogTrace("Add-Frame.log", "seq:%lld, %lld, %lld car:%s vio:%d", \
		plate.uSeq, plate.uSeq2, plate.uSeq3, plate.chText, plate.uViolationType);
#endif

    bool bRightPlateFlag = false;
    bool bNumberLen = true;
    bool bFirstChar = true;

	int  nOriWidth = 0;
	int  nOriHeight = 0;
	printf("Width:%d,heigth:%d\n",plate.uPicWidth,plate.uPicHeight);

    //车牌长度校验
    if( (strlen(plate.chText) == 7) || (strlen(plate.chText) == 9) )
    {
        bNumberLen = true;
    }
    else
    {
		//LogNormal("-plate.chText=%s-size=%d-\n", plate.chText, strlen(plate.chText));
        bNumberLen = false;
    }

    //车牌首字校验
    //试  挂  澳 港 学 警
    // (  @   +  -   $  %
    char chFirst = plate.chText[0];

    if(chFirst == '(' || chFirst == '@' ||
        chFirst == '+' || chFirst == '-' ||
        chFirst == '$' ||  chFirst == '%' ||
		chFirst == '*')
   {
       bFirstChar = false;
       //LogNormal("===chFirst=%c \n", chFirst);
   }

   if(chFirst == 'L')
   {
        if(strlen(plate.chText) < 9) //武警车牌位数少于9位的为不合法
        {
            LogNormal("WJ=CheckPlateInfo=plate.chText=%s\n", plate.chText);
            return false;
        }
   }

   if(bNumberLen && bFirstChar)
   {
       bRightPlateFlag = true;
   }

    return bRightPlateFlag;
}

//发送相机控制命令线程
void* ThreadRoSeekDspCtrl(void* pArg)
{
	//取类指针
	CRoSeekCameraDsp* pRoSeekCameraDsp5 = (CRoSeekCameraDsp*)pArg;
	if(pRoSeekCameraDsp5 == NULL)
		return pArg;

	pRoSeekCameraDsp5->DspCameraCtrl();

	pthread_exit((void *)0);

	return pArg;
}

bool CRoSeekCameraDsp::DspCameraCtrl()
{
	if(!m_bEndCapture)
	{
		CAMERA_CONFIG cfg;
		cfg.nIndex = m_nCfgIndex;

		this->ThreadControl(cfg);
	}
}

bool CRoSeekCameraDsp::CloseCamCtrl()
{
	LogTrace("DspControlThread.log", "CRoSeekCameraDsp::CloseCamCtrl() \n");	
	CloseJoinThread(m_nThreadId5);

	return true;
}

//等待接收缓冲有数据可读
int CRoSeekCameraDsp::WaitForSockCanRead(int sock, int nTimeOut /* = 1000 */)
{
	int nWaitSock = 0;
	int maxfd;
	fd_set fsVal;
	FD_ZERO(&fsVal);

	if( sock > 0)
		FD_CLR(sock, &fsVal);

	FD_SET(sock, &fsVal);
	struct timeval tvTimeOut;
	tvTimeOut.tv_sec = nTimeOut/1000;
	tvTimeOut.tv_usec = (nTimeOut%1000)*1000;

	maxfd = (sock > 0) ? (sock+1) : 1;

	int nRet = select(maxfd, &fsVal, NULL, NULL, &tvTimeOut);

	bool bFdSet = FD_ISSET(sock, &fsVal);

	if(nRet < 0)
	{
		if((errno == EINTR) && nRet == -1)
		{
			LogNormal("11=errno=%d=[%s]\n", errno, strerror(errno));
			nWaitSock = -1;
		}
	}
	else
	{
		if(nRet > 0 && bFdSet)
		{
			nWaitSock = 1;
		}
		//else if((nRet == 0) && (!bFdSet))
		else
		{
			nWaitSock = 0;
		}
	}

	return nWaitSock;
}

//获取相机录像分辨率
void CRoSeekCameraDsp::GetCamVideoInfo(unsigned short &uWidth, unsigned short &uHeight)
{
	uWidth = 400;
	uHeight = 300;
	return;

	if(DSP_ROSEEK_200_310 == m_nCameraType || DSP_ROSEEK_200_385  == m_nCameraType)
	{
		uWidth = 1600;
		uHeight = 1216;
	}
	else if(DSP_ROSEEK_500_335 == m_nCameraType)
	{
		uWidth = 1600;
		uHeight = 1200;
	}
	else if(DSP_ROSEEK_400_340 == m_nCameraType)
	{
		uWidth = 1600;
		uHeight = 1200;
	}
	else if(DSP_ROSEEK_200_380 == m_nCameraType)
	{
		uWidth = 1920;
		uHeight = 1088;
	}
	else if(DSP_ROSEEK_500_330 == m_nCameraType)
	{
		uWidth = 1600;
		uHeight = 1200;
	}
	else if(DSP_500_C501K == m_nCameraType)
	{
		uWidth = 1920;
		uHeight = 1080;
	}
	else if(DSP_200_C201K == m_nCameraType)
	{
		uWidth = 1920;
		uHeight = 1080;
	}
	else if(DH_203_M ==  m_nCameraType)
	{
		uWidth = 1600;
		uHeight = 1200;
	}
	else if(DH_523_M ==  m_nCameraType)
	{
		uWidth = 1600;
		uHeight = 1200;
	}
	else if(DH_213_M == m_nCameraType || DSP_ROSEEK_600_465 == m_nCameraType)//大华相机
	{
		uWidth = 1920;
		uHeight = 1080;
	}
	else if(DSP_200_C203K ==  m_nCameraType)
	{
		uWidth = 1600;
		uHeight = 1200;
	}
	else{}
}

//处理相机日志
bool CRoSeekCameraDsp::DealDspLog(const char *pBuffer, const int nSize)
{
	Image_header_dsp* pHeader = (Image_header_dsp*)pBuffer;
	if(pHeader == NULL)
	{
		return false;
	}

	int HeaderLength = sizeof(Image_header_dsp);
	FILE *fpOut = NULL;
	char jpg_name[256] = {0};

	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r( &now,newTime );

	time_t dspTime;
	dspTime = (pHeader->ts/1000/1000);
	struct tm *tm_dspTime, tm_dsptime_now;
	tm_dspTime = &tm_dsptime_now;
	localtime_r(&dspTime, tm_dspTime);
	//sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d", , newTime->tm_mon+1, newTime->tm_mday, newTime->tm_hour, newTime->tm_min, newTime->tm_sec);

	//log
	{		
		//sprintf(jpg_name, "./DspLog_%d_%4d_%02d_%02d_%02d_DSP_%4d_%02d_%02d_%02d.log", \
		//	pHeader->uCameraId, newTime->tm_year+1900, newTime->tm_mon+1,newTime->tm_mday, newTime->tm_hour,\
		//	tm_dspTime->tm_year+1900, tm_dspTime->tm_mon+1,tm_dspTime->tm_mday, tm_dspTime->tm_hour);

		//清除后2个小时日志
		char jpg_name_del[256] = {0};
		char buf[256] = {0};

		bool bDiskData = IsDataDisk();
		if(bDiskData)
		{
			std::string strTemp = "/detectdata/log";

			// 判断目录是否存在,不存在则建立目录
			if(access(strTemp.c_str(),0) != 0) //目录不存在
			{
				mkdir(strTemp.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
				sync();
			}
			if(tm_dspTime->tm_wday == 0)
			{
				tm_dspTime->tm_wday = 7;
			}
			sprintf(buf,"%s/%02d",strTemp.c_str(),tm_dspTime->tm_wday);
			if(access(buf,0) != 0) //目录不存在
			{
				mkdir(buf,S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
				sync();
			}

			sprintf(jpg_name, "%s/DspLog_%d_DSP_%02d.log", \
				buf,m_nChannelId, tm_dspTime->tm_hour);
			
			if(tm_dspTime->tm_hour + 2 >= 24)
			{
				if(tm_dspTime->tm_wday == 7)
				{
					tm_dspTime->tm_wday = 0;
				}
				sprintf(buf,"%s/%02d",strTemp.c_str(),tm_dspTime->tm_wday+1);
			}
			sprintf(jpg_name_del, "%s/DspLog_%d_DSP_%02d.log", \
				buf,m_nChannelId, (tm_dspTime->tm_hour + 2)%24);
		}
		else
		{
			std::string strTemp = "log";

			// 判断目录是否存在,不存在则建立目录
			if(access(strTemp.c_str(),0) != 0) //目录不存在
			{
				mkdir(strTemp.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
				sync();
			}
			if(tm_dspTime->tm_wday == 0)
			{
				tm_dspTime->tm_wday = 7;
			}
			sprintf(buf,"%s/%02d",strTemp.c_str(),tm_dspTime->tm_wday);
			if(access(buf,0) != 0) //目录不存在
			{
				mkdir(buf,S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
				sync();
			}

			sprintf(jpg_name, "./%s/DspLog_%d_DSP_%02d.log", \
				buf,m_nChannelId, tm_dspTime->tm_hour);

			if(tm_dspTime->tm_hour + 2 >= 24)
			{
				if(tm_dspTime->tm_wday == 7)
				{
					tm_dspTime->tm_wday = 0;
				}

				sprintf(buf,"%s/%02d",strTemp.c_str(),tm_dspTime->tm_wday+1);
			}
			sprintf(jpg_name_del, "./%s/DspLog_%d_DSP_%02d.log", \
				buf,m_nChannelId, (tm_dspTime->tm_hour + 2)%24);
		}

		if(access(jpg_name_del,F_OK) == 0)
		{
			FILE* fp = fopen(jpg_name_del,"wb");
			if(fp)
			{
				fwrite("",0,1,fp);
				fclose(fp);
			}			
		}		

		fpOut = fopen(jpg_name, "ab+");

		char textEx[1024] = {0};
		sprintf(textEx, "dsp Time: %4d_%02d_%02d_%02d  %d:%d \n",\
			tm_dspTime->tm_year+1900, tm_dspTime->tm_mon+1,tm_dspTime->tm_mday, tm_dspTime->tm_hour, tm_dspTime->tm_min, tm_dspTime->tm_sec);

		if(fpOut)
		{
			fwrite((BYTE*)textEx, 1, strlen(textEx), fpOut);
			fwrite((BYTE*)pBuffer + HeaderLength, 1, pHeader->nSize, fpOut);
			fclose(fpOut);
		}		
	}

	return true;
}

//获取相机帧率
int CRoSeekCameraDsp::GetDspCameraFrequency(const int &nCamType)
{
	int nFrequency = 10;
	int nCamFlag = 0;//200:200W 500:500W

	switch(nCamType)
	{
		case DSP_ROSEEK_200_310:	//锐视200万dsp相机
		case BOCOM_301_200:			//bocom200万相机(301)
		case BOCOM_302_200:			//bocom200万触发相机(302)
		case DSP_ROSEEK_200_380:	//dsp200万触发
		case DSP_ROSEEK_400_340:	//锐视400万dsp相机
		case DSP_200_C201K:			//1920*1080分辨率相机
		case DH_203_M:				//大华200W 1600*1200
		case DH_213_M:				//大华200W 1920*1080,
		case DSP_ROSEEK_200_385:	//1616*1232
		case DSP_200_C203K:			//1600*1200
			{
				nCamFlag = 200;
				break;
			}
		case DSP_ROSEEK_500_335:	//锐视500万dsp相机
		case BOCOM_302_500:			//bocom500万相机(302)
		case BOCOM_301_500:			//bocom500万相机(301)
		case DSP_ROSEEK_500_330:	//dsp500万触发 24
		case DSP_500_C501K:			//2592*1936分辨率相机
		case DH_523_M:				//大华500W 2592*2048,
			{
				nCamFlag = 500;
				break;
			}
		case DSP_ROSEEK_600_465:    //锐视600万dsp相机
			{
				nCamFlag = 600;
				break;
			}
		default:
			//
	}
	
	if(200 == nCamFlag)
	{
		nFrequency  = 15;
	}
	else if(500 == nCamFlag)
	{
		nFrequency = 10;
	}
	else
	{
		//LogNormal("GetDspCameraFrequency error! nCamType:%d ", nCamType);
		nFrequency = 10;
	}

	//LogNormal("nCamType:%d frequency:%d", nCamType, nFrequency);

	return nFrequency;
}

//核查相机是否重启
void CRoSeekCameraDsp::CheckCameraReboot()
{
	if(m_bCamReboot)/*相机重启清空缓存图片*/
	{
		printf("=Roseek ==1111 ClearChannelJpgMap=\n");

		LogNormal("m_bCamReboot:%d \n", m_bCamReboot);
	
		//清空数据前把,有图未输出的数据全部输出
		//g_skpChannelCenter.OutPutChannelResult(m_nChannelId);
		//清空输出Jpgmap
		//g_skpChannelCenter.ClearChannelJpgMap(m_nChannelId);

		m_bCamReboot = false;
		
		RECORD_PLATE_DSP plate;//传入特殊的记录，让车牌线程自动清除数据
		plate.uTime = time(NULL);
		plate.uSeq = 0;
		plate.uRoadWayID = 0;
		plate.uViolationType = 99;
		AddPlateFrame(plate);

		printf("=Roseek ==2222 ClearChannelJpgMap=\n");
	}
}

//关闭fd
void CRoSeekCameraDsp::CloseFd(int &fd)
{
	if(fd != -1)
	{
		LogNormal("Close CRoSeekCameraDsp fd:%d ", fd);
		if(fd > 0)
		{
			shutdown(fd, 2);
			close(fd);
		}
		fd = -1;
	}
}

void CRoSeekCameraDsp::GetCameraBrand()
{
	m_cfg.nIndex = CAMERA_GET_DSP_BRAND;
	Control(m_cfg);
}

//创建接收图片线程
bool CRoSeekCameraDsp::CreateCaptureThread()
{
	bool bRet = false;

	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	int nret = pthread_create(&m_nThreadId,NULL,ThreadRoSeekDspCapture,this);

	if(nret != 0)
	{
		bRet = false;
		Close();
		LogError("创建Jpg采集线程失败,无法进行采集！\r\n");
	}
	else
	{
		bRet = true;
		LogNormal("pthread_create==m_nThreadId=%x==\n", m_nThreadId);
	}
	pthread_attr_destroy(&attr);

	return bRet;
}

//创建接收h264录像线程
bool CRoSeekCameraDsp::CreateH264CaptureThread()
{
	bool bRet = false;

	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);	

	int nret2 = pthread_create(&m_nThreadId2, NULL, ThreadRoSeekDspCapture2, this);
	if(nret2!=0)
	{
		bRet = false;
		Close2();
		LogError("创建Dsp-H264码流采集线程失败,无法进行采集！\r\n");
	}
	else
	{
		bRet = true;
		LogNormal("pthread_create==m_nThreadId2=%x==\n", m_nThreadId2);
	}
	pthread_attr_destroy(&attr);

	return bRet;
}

//创建定时校时线程
bool CRoSeekCameraDsp::CreateSynClockThread()
{
	bool bRet = false;

	pthread_attr_t   attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);	

	int nret3=pthread_create(&m_nThreadId3, NULL, ThreadRoSeekDspSynClock, this);
	if(nret3!=0)
	{
		bRet = false;
		Close3();
		LogError("创建DSP校时线程失败,无法进行校时！\r\n");
	}
	else
	{
		bRet = true;
		LogNormal("pthread_create==m_nThreadId3=%x==\n", m_nThreadId3);
	}
	pthread_attr_destroy(&attr);

	return bRet;
}

//关闭线程
void CRoSeekCameraDsp::CloseJoinThread(pthread_t &nThreadId)
{
	//LogNormal("CloseJoinThread nThreadId 11:%x ", nThreadId);
	if(nThreadId != 0)
	{
		pthread_join(nThreadId, NULL);
		nThreadId = 0;
	}
	//LogNormal("CloseJoinThread nThreadId 22: %x ", nThreadId);
}

//强行关闭线程
void CRoSeekCameraDsp::KillThread(pthread_t &nThreadId)
{
	//LogNormal("KillThread nThreadId 11:%x ", nThreadId);
	if(nThreadId != 0)
	{
		pthread_cancel(nThreadId);
		nThreadId = 0;
	}
	//LogNormal("KillThread nThreadId 22: %x ", nThreadId);
}

//监测接收图片数据线程状态
bool CRoSeekCameraDsp::CheckCaptureStatus()
{
	bool bRet = false;

	//LogNormal("CheckCaptureStatus m_nTcpFd:%d \n", m_nTcpFd);

	if(!m_bEndCapture)
	{
		bool bConnect = GetCameraState();
		if(!bConnect)
		{
#ifdef DEBUG_DSP_CMD
			LogNormal("CloseFd [15] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
			CloseFd(m_nTcpFd);
		}
		else
		{
			int nTimeNow = GetTimeStamp();
			int nTimeDis = nTimeNow - m_nRecvLastTime;
			if(m_nRecvLastTime > 0 && nTimeDis > 300) //5分钟以上未收到数据
			{
				LogNormal("RecvNothing in nTimeDis:%d !", nTimeDis);

#ifdef DEBUG_DSP_CMD
				LogNormal("CloseFd [14] ID:%d m_nTcpFd:%d \n", m_nChannelId, m_nTcpFd);
#endif
				CloseFd(m_nTcpFd);

				m_nRecvLastTime = nTimeNow;
				bRet = false;
			}
			else
			{
				bRet = true;
			}
		}
		

		if(m_nTcpFd == -1)
		{
			LogNormal("CheckCaptureStatus() m_nThreadId 11:%x m_bOpenCamera:%d", m_nThreadId, m_bOpenCamera);

			if(m_bOpenCamera)//退出线程,前面状态为通道打开正常
			{
				//退出线程
				//CloseJoinThread(m_nThreadId);
				//KillThread(m_nThreadId);
			}

			//LogNormal("CheckCaptureStatus() m_nThreadId 22:%x", m_nThreadId);

			bool bOpen = ReOpen();
			if(bOpen)
			{
				//创建接收图片线程
				//bool bCapture = CreateCaptureThread();

				//if(bCapture)
				{
					LogNormal("ReOpen Camera m_nTcpFd:%d !", m_nTcpFd);

					m_nRecvNothingFlag = 0; //计数归0
					g_skpDB.SetCameraStateToDBByChannelID(m_nChannelId, CM_OK);

					bRet = true;
				}
			}
			else
			{
				bRet = false;
			}
		}
		
	}//End of if(!m_bEndCapture)
	
	
	return bRet;
}

//让相机时间与检测器时间同步,SetClock
bool CRoSeekCameraDsp::SetClock()
{
	bool bRet = false;

	CAMERA_CONFIG  cfg = m_cfg;
	cfg.nIndex = CAMERA_SET_CLOCK;

	LogNormal("==ReOpen Cam reset CLOCK!=\n");
	int nRet = Control(cfg);
	if(nRet > 0)
	{
		bRet = true;
	}

	return bRet;
}

//设置数据包头
void CRoSeekCameraDsp::SetCameraHeader(const Image_header_dsp *pDsp_header, yuv_video_buf &camera_header)
{
	int nTypeFlag = pDsp_header->nType;

	static unsigned int uSeqT1 = 0;
	camera_header.nVideoType = 1;//视频编码格式(0:h264,1:表示mjpeg)
	//取到实际图像宽高
	camera_header.height = pDsp_header->nHeight;
	camera_header.width = pDsp_header->nWidth;
	camera_header.size = pDsp_header->nSize;
	camera_header.nSeq = pDsp_header->nSeq; //帧号
	camera_header.uRecvTs = pDsp_header->uRecvTs;//接收记录时间

	//LogTrace("DealOneFrame.log", "=in Deal==uSeq=%d, nFrameSize=%d=m_uPrevBuff=%d=m_uCurrBuff=%d\n", \
	pDsp_header->nSeq, pDsp_header->nSize, m_uPrevBuff, m_uCurrBuff);
		
	camera_header.uTimestamp = (pDsp_header->ts / 1000) / 1000; //单位秒s
	camera_header.ts = pDsp_header->ts;
	camera_header.nFrameRate = pDsp_header->nFrameRate;
	
	//传入线圈信号
	camera_header.uSpeedSignal.SpeedSignal = pDsp_header->nSpeedSignal; //
	camera_header.uSpeedSignal.SystemTime =  pDsp_header->ts;//us
	camera_header.uSpeedSignal.SpeedTime = pDsp_header->SpeedTime;
	//红灯信号
	camera_header.uTrafficSignal = pDsp_header->nRedColor;
	camera_header.uFlashSignal = pDsp_header->nFlashFlag; //爆闪灯状态

#ifdef DEBUG_GAIN
	camera_header.uGain = pDsp_header->nGain;
	camera_header.nSh = pDsp_header->nSh;
#endif

#ifdef REDADJUST_2_DEBUG
		LogTrace("RedSignal.log","id:%d sig:%d type:%d seq:%d", m_nChannelId,pDsp_header->nRedColor,pDsp_header->nType,pDsp_header->nSeq);	
#endif

#ifdef ROSEEK_DSP_LOG
	printf("===SystemTime:sec:%lld usec:%lld ===\n", tv.tv_sec, tv.tv_usec, tv.tv_sec * 1000000 + tv.tv_usec);

	printf("2222=pDsp_header->nSize=%d=pDsp_header->nSeq=%d=nRoadIndex=%d=====camera_header.uTimestamp=%d===\n",\
		pDsp_header->nSize, pDsp_header->nSeq, pDsp_header->nRoadIndex, camera_header.uTimestamp);
	//printf("===pDsp_header->ts=%lld===[%x]=\n", pDsp_header->ts, pDsp_header->ts);

	printf("=11=88888888###===pDsp_header->nSeq=%d, pDsp_header->nCount=%d, pDsp_header->nOrder=%d==pDsp_header->nType=%d==\n", \
		pDsp_header->nSeq, pDsp_header->nCount, pDsp_header->nOrder, pDsp_header->nType);
#endif

#ifdef ROSEEK_SAVE_INFO
	FILE *fpOut = NULL;
	char jpg_name[256] = {0};
	//if(pDsp_header->nType == 1)
	{
		sprintf(jpg_name, "./text/%d_%d_Seq_DSP.head", pDsp_header->nSeq, pDsp_header->nType);
		fpOut = fopen(jpg_name, "wb+");
		if(fpOut)
		{
			fwrite((char*)(pDsp_header), 1, sizeof(Image_header_dsp), fpOut);
			fclose(fpOut);
		}
	}
#endif

#ifdef _DSP_DEBUG
	printf("=====pDsp_header->nFrameRate=%d=== pDsp_header->nSeq=%d,camera_header.uTimestamp=%lld\n", \
		pDsp_header->nFrameRate, pDsp_header->nSeq, camera_header.uTimestamp);
#endif
}

//接收数据出错处理
bool CRoSeekCameraDsp::DealSafeRecvError(const int nErrCode)
{
	bool bRet = false;//异常

	if(nErrCode > 0)
	{
		LogNormal("CRoSeekCameraDsp::SafeRecv nErrCode:%d \n", nErrCode);

		switch(nErrCode)
		{
		case 4:
		case 11:
			{
				usleep(10*1000);
				bRet = true;
				//continue;	
				break;
			}
		case 104:
		case 107:
		case 111:
		case 113:
			{
				usleep(10*1000); //10ms
				if(m_nTcpFd > 0)
				{					
#ifdef DEBUG_DSP_CMD
					LogNormal("CloseFd [8] ID:%d m_nTcpFd:%d nErrCode:%d \n", \
						m_nChannelId, m_nTcpFd, nErrCode);
#endif
					CloseFd(m_nTcpFd);
				}
				bRet = false;
				break;
			}
		default:
			{
#ifdef DEBUG_DSP_CMD
				LogNormal("CloseFd [12] ID:%d m_nTcpFd:%d nErrCode:%d \n", \
					m_nChannelId, m_nTcpFd, nErrCode);
#endif
				CloseFd(m_nTcpFd);
				bRet = false;
				break;
			}					
		}
	}
	
	return bRet;
}

//自动抓拍图片
void CRoSeekCameraDsp::AutoCapturePic()
{
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r( &now,newTime );
	
	if(newTime->tm_hour == 11 && newTime->tm_min == 0 )//11点抓拍图片
	{
		if(!m_bAutoCaptureTime)
		{
			m_bAutoCaptureTime = true;

			CAMERA_CONFIG  cfg;
			cfg.uCameraID = m_nCamID;
			cfg.nIndex = (int)CAMERA_GET_ONE_PIC;
			Control(cfg);

			m_bSendCapturePic = true;
		}
	}
	else
	{
		m_bAutoCaptureTime = false;
	}
}

//自动产生一条记录
void CRoSeekCameraDsp::GenerateRecord(Image_header_dsp *pDsp_header)
{
	LogNormal("===GenerateRecord=%02d\n",m_nCamID);
	RECORD_PLATE_DSP plate;
	
	sprintf(plate.chText,"777777%02d",m_nCamID);
	//车牌数据强制违章类型为99
	plate.uViolationType = 99;
	plate.uSeq = pDsp_header->nSeq;
	plate.uSeq2 = pDsp_header->nSeq;
	plate.uRoadWayID = 1;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	plate.uTime = tv.tv_sec;
	plate.uMiTime = tv.tv_usec/1000;
	plate.uTime2 = tv.tv_sec;
	plate.uMiTime2 = tv.tv_usec/1000;
	plate.uColor = CARNUM_OTHER; //其他颜色
	plate.uPlateType = 1;
	plate.uPosLeft = 100;//车牌在全景图片中的位置左
    plate.uPosTop = 100;//车牌在全景图片中的位置上
    plate.uPosRight = 200;//车牌在全景图片中的位置右
    plate.uPosBottom = 120;//车牌在全景图片中的位置下
	plate.uCarPosLeft = 100;//车辆在全景图片中的位置左
	plate.uCarPosTop = 100;//车辆在全景图片中的位置上
	plate.uCarPosRight = 200;//车辆在全景图片中的位置右
	plate.uCarPosBottom = 200;//车辆在全景图片中的位置下	
	plate.uType = SMALL;


		int m_mode;

		//实时还是历史视频
		int m_nVideoType;//0:实时视频，1：远程历史视频，2：本地历史视频

		//组播
    AddPlateFrame(plate);//传大图           
}

#ifdef LED_STATE
//核查爆闪灯状态
void CRoSeekCameraDsp::CheckLedState(const char chLedState)
{
	bool bA[8] = {0};	
	
	for(int i=0; i<8; i++)
	{
		bA[i] = (bool)(chLedState & (0x01 << i));		
	}
	LogNormal("chLedState[L][%d,%d,%d,%d,%d,%d,%d,%d][H] ", \
			 bA[0], bA[1], bA[2], bA[3], bA[4], bA[5], bA[6], bA[7]);
	
	for(int i=0; i<4; i++)
	{
		int nChan = i+1;
		if(bA[2*i])//灯关闭
		{
			if(bA[2*i+1])
			{
				//未连接或普通灯
				LogNormal("id:%d RoadId:%d 未连接或普通灯!", m_nChannelId, nChan);
			}
			else
			{
				if(m_bPreLedArray[2*i+1] != bA[2*i+1])//灯状态变化才打印日志
				{
					//正常关闭
					LogNormal("id:%d RoadId:%d 灯正常关闭", m_nChannelId, nChan);
				}
				
			}
		}
		else//灯打开
		{
			if(bA[2*i+1])//
			{
				LogNormal("id:%d RoadId:%d 灯损坏!", m_nChannelId, nChan);
			}
			else
			{
				if(m_bPreLedArray[2*i+1] != bA[2*i+1])//灯状态变化才打印日志
				{
					//灯完好
					LogNormal("id:%d RoadId:%d 灯正常打开", m_nChannelId, nChan);
				}			
			}
		}		
	}
	
	//记录上一次led状态
	for(int i=0; i<8; i++)
	{
		m_bPreLedArray[i] = bA[i];
	}	
}
#endif