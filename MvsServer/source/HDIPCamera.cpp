// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifdef HDIPCAMERA
#include "trdefine.h"
#include "Trclient.h"
#include "tmTransDefine.h"
//#include "tmControlClient.h"
#endif
#include "HDIPCamera.h"
#include "Common.h"

#ifdef HDIPCAMERA
void  MessageCallBack(void * hClient, unsigned int dwCode, void *context)
{
	char mess[128];
	int  bError = 0;

	switch( dwCode )
	{
	case ERROR_CODE_CONNECTSUCCESS:
		bError = 0;
		sprintf(mess, "connect success");
		break;
	case ERROR_CODE_BEGINCONNECT:
		bError = 0;
		sprintf(mess, "begin connect server");
		break;
	case ERROR_CODE_CLOSECONNECT:
		//g_bSystemRun = 0;
	case ERROR_CODE_HANDLE:
	case ERROR_CODE_PARAM:
	case ERROR_CODE_CREATETHREAD:
	case ERROR_CODE_CREATESOCKET:
	case ERROR_CODE_OUTMEMORY:
	case ERROR_CODE_MORECHANNEL:
	case ERROR_CODE_NETWORK:
	case ERROR_CODE_CONNECTERROR:
	case ERROR_CODE_CONNECTERROR_1000:
	case ERROR_CODE_SERVERSTOP:
	case ERROR_CODE_SERVERSTOP_1000:
	case ERROR_CODE_TIMEOUT:
	case ERROR_CODE_TIMEOUT_1000:
	case ERROR_CODE_SENDDATA:
	case ERROR_CODE_SENDDATA_1000:
	case ERROR_CODE_RECVDATA:
	case ERROR_CODE_RECVDATA_1000:
	case ERROR_CODE_VERSION:
	case ERROR_CODE_SERVERNOSTART:
	case ERROR_CODE_SERVERERROR:
	case ERROR_CODE_CHANNELLIMIT:
	case ERROR_CODE_SERVERLIMIT:
	case ERROR_CODE_SERVERREFUSE:
	case ERROR_CODE_IPLIMIT:
	case ERROR_CODE_PORTLIMIT:
	case ERROR_CODE_TYPEERROR:
	case ERROR_CODE_USERERROR:
	case ERROR_CODE_PASSWORDERROR:
	default:
		if( ( dwCode & 0xC0000000 ) == 0xC0000000 )
		{
			sprintf(mess, "connect fail<%X>", (unsigned int)dwCode);
			bError = 1;
			break;
		}
	}

	fprintf(stderr, "%s\n", mess);
}

int  AvFrameCallBack(void * hClient, StreamData_t* pData, int iDataLen, int bLost, void *context)
{
	if( pData->iFrameType == 0 )
	{
		fprintf(stderr, "vo[%dx%d-%dx%d], size=%06d, rst=%d : %X, %X, %d, %d, %d, %u\n", \
				pData->iWidth, pData->iHeight, pData->iDisplayWidth, pData->iDisplayHeight, \
				pData->iDataLen, bLost, \
				(unsigned int)pData->iFrameRate, (unsigned int)pData->dwStreamTag, (int)pData->dwStreamId, \
				pData->iFrameType, pData->bKeyFrame, pData->dwTimeStamp);

		    /*FILE* fp = fopen("test.h264","a+");
			fwrite(pData->pDataBuffer,1,pData->iDataLen,fp);
			fclose(fp);*/

			if(context)
			{
			    CHDIPCamera* pHDIPCamera = (CHDIPCamera*)context;
                pHDIPCamera->DecodeFrame(pData->pDataBuffer,pData->iDataLen,pData->dwTimeStamp,pData->iFrameRate,pData->bKeyFrame);
			}

	}else
	if( pData->iFrameType == 1 )
	{
		fprintf(stderr, "audio : bufsize=%d\n", pData->iDataLen);
	}else
	if( pData->iFrameType == 2 )
	{
		fprintf(stderr, "head : bufsize=%d\n", pData->iDataLen);
	}


	return 0;
}
#endif

//H264采集线程
void* ThreadHDIPH264Capture(void* pArg)
{
    //取类指针
    CHDIPCamera* pHDIPCamera = (CHDIPCamera*)pArg;
    if(pHDIPCamera == NULL)
        return pArg;

    pHDIPCamera->CaptureFrame();
    pthread_exit((void *)0);
    return pArg;
}

//心跳线程
void* ThreadHDIPLinkTest(void* pArg)
{
    //取类指针
    CHDIPCamera* pHDIPCamera = (CHDIPCamera*)pArg;
    if(pHDIPCamera == NULL)
        return pArg;

    pHDIPCamera->LinkTest();
    pthread_exit((void *)0);
    return pArg;
}


CHDIPCamera::CHDIPCamera(int nCameraType)
{
    m_hObject = NULL;
    m_hControlObject = NULL;
    m_nCameraType = nCameraType;

    m_nWidth = 1920;
    m_nHeight = 1080;

    m_nMarginX = 0;
    m_nMarginY = 0;
    m_pFrameBuffer = NULL;
	m_nTcpSocket = -1;

	m_nMaxBufferSize = 10;
	m_nDeviceId = 1;
}

CHDIPCamera::~CHDIPCamera()
{

}

void CHDIPCamera::Init()
{
    for(int i=0; i< m_nMaxBufferSize; i++)
    {
        m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight*3,sizeof(unsigned char));
    }

    m_uSeq = 0;
    m_nPreFieldSeq  = 0;
    m_nCurIndex = 0;
    m_nFirstIndex = 0;
    m_nFrameSize=0;
    m_pFrameBuffer = m_FrameList[0];

    #ifdef H264_DECODE
    m_Decoder.SetVideoSize(m_nWidth,m_nHeight);
    m_Decoder.SetVideoCodeID(0);
    m_Decoder.InitDecode(NULL);
    #endif

    m_nDecodeCount = 0;
	
	#ifndef HDIPCAMERA
	//线程ID
    m_nThreadId = 0;
	m_nLinkTestId = 0;
	m_bEndCapture = false;
	m_bRequestStatus = false;

	//线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    int nret=pthread_create(&m_nThreadId,NULL,ThreadHDIPH264Capture,this);

    if(nret!=0)
    {
        Close();
        //失败
        LogError("创建h264采集线程失败,无法进行采集！\r\n");
    }
    
	nret=pthread_create(&m_nLinkTestId,NULL,ThreadHDIPLinkTest,this);
    if(nret!=0)
    {
        Close();
        //失败
        LogError("创建心跳线程失败！\r\n");
    }
    pthread_attr_destroy(&attr);

	#endif
}


bool CHDIPCamera::Connect()
{
    #ifdef HDIPCAMERA
    m_hObject = Trclient_Init(0);

    if(m_hObject == NULL)
    {
        printf("Trclient_Init error\n");
        return false;
    }

    Trclient_RegisterMessageCallBack(m_hObject, MessageCallBack, this);
	Trclient_RegisterAvFrameOutCallBack(m_hObject, AvFrameCallBack, this);

    ConnectConfig_t		info;
	memset(&info, 0, sizeof(ConnectConfig_t));
	info.iSize = sizeof(ConnectConfig_t);
	sprintf(info.szAddress, "%s",g_monitorHostInfo.chMonitorHost);
	sprintf(info.szTurnAddress, "");
	info.iChannel = 0;
	info.iStream = 0;
	info.iUseTurnAddress = 0;
	info.iPort = g_monitorHostInfo.uMonitorPort;
	info.iTimeOut = 30000;
	info.iReConnectNum = 3;
	info.iAutoConnect = 0;
	info.iConnectType = ListenTcpIp;
	info.iTranstType = TRANST_TYPE_PACKET;
	info.iTranstPackSize = 4096;
	info.iReConnectTime = 3000;
	sprintf(info.szUser, "%s", g_monitorHostInfo.chUserName);
	sprintf(info.szPass, "%s", g_monitorHostInfo.chPassWord);

    fprintf(stderr, "address : %s:%d\n", info.szAddress, info.iPort);
	fprintf(stderr, "stream  : %d\n", info.iStream);
	fprintf(stderr, "user    : %s\n", info.szUser);
	fprintf(stderr, "pass    : %s\n", info.szPass);
	int iRet = Trclient_ConnectA(m_hObject, &info);

	printf("Trclient_ConnectA iRet=%d\n",iRet);
	/*if(iRet != ERROR_CODE_CONNECTSUCCESS)
	{
	    printf("Trclient_ConnectA error\n");
        return false;
	}*/

    /////////////////////////相机控制
    /*m_hControlObject = TMCC_Init(0);
	if( m_hControlObject == NULL )
	{
	    printf("TMCC_Init error\n");
	    return false;
	}
	//连接设备
	tmConnectInfo_t	cninfo;
	memset(&cninfo, 0, sizeof(tmConnectInfo_t));
	cninfo.dwSize = sizeof(tmConnectInfo_t);	//该结构的大小，必须填写
	sprintf(cninfo.pIp, "%s",g_monitorHostInfo.chMonitorHost);		//连接服务器的IP地址，根据设备设置而变
	cninfo.iPort = g_monitorHostInfo.uMonitorPort;						//服务器连接的端口，根据设备设置而变
	sprintf(cninfo.szUser, "%s", g_monitorHostInfo.chUserName);			//登录用户名，根据设备设置而变
	sprintf(cninfo.szPass, "%s", g_monitorHostInfo.chPassWord);			//登录用户口令，根据设备设置而变

	int iRetControl = TMCC_Connect(m_hControlObject, &cninfo, 1);
	if( iRetControl != TMCC_ERR_SUCCESS )
	{
	    printf("TMCC_Connect error\n");
        return false;
	}*/
	#else
	if(!connect_tcp())
	{
		return false;
	}
	#endif

	return true;
}

bool CHDIPCamera::Open()
{
     //读取相机参数
    m_cfg.uKind = m_nCameraType;
    LoadCameraProfile(m_cfg);
    ConvertCameraParameter(m_cfg,false,true);

    Init();

    if(!Connect())
    {
        LogError("无法与相机建立连接\r\n");
    }
	else
	{
		//相机参数设置
		//SetCaMeraPara();
		
		#ifndef HDIPCAMERA
		//usleep(1000*500);
		//发送视频请求命令
		UINT32 uCmd = 0x00000001;
		SendCMD(uCmd);

		//接收视频请求回复
		UINT32 nRep = RecvCMD();

		LogNormal("RecvCMD nRep=%d\n",nRep);

		if(nRep == 1)
		{
			m_bRequestStatus = true;
		}
		else
		{
			LogError("接收视频请求回复失败!\r\n");
			m_bRequestStatus = false;
		}
		#endif
	}

    return true;
}

bool CHDIPCamera::Close()
{
    #ifdef HDIPCAMERA
    if(m_hObject)
    {
        Trclient_DisConnect(m_hObject);
        Trclient_UInit(m_hObject);
        m_hObject = NULL;
    }

   /* if(m_hControlObject)
    {
        TMCC_DisConnect(m_hControlObject);
        TMCC_Done(m_hControlObject);
        m_hControlObject = NULL;
    }*/
    
	#else

	m_bEndCapture = true;


	//发送视频中止命令
	UINT32 uCmd = 0x00000002;
	SendCMD(uCmd);

	m_bRequestStatus = false;

	if(m_nTcpSocket!=-1)
    {
        shutdown(m_nTcpSocket,2);
        close(m_nTcpSocket);
        m_nTcpSocket = -1;
    }

	if (m_nThreadId != 0)
    {
        pthread_join(m_nThreadId, NULL);
        m_nThreadId = 0;
    }

	if(m_nLinkTestId != 0)
	{
		pthread_join(m_nLinkTestId, NULL);
        m_nLinkTestId = 0;
	}
	#endif

    #ifdef H264_DECODE
    m_Decoder.UinitDecode();
    #endif

        //加锁
    pthread_mutex_lock(&m_FrameMutex);
    //释放内存
    for (int i=0; i<MAX_SIZE; i++)
    {
        if(m_FrameList[i] != NULL)
        {
            free(m_FrameList[i]);
            m_FrameList[i] = NULL;
        }
        m_nFrameSize = 0;
        m_pFrameBuffer = NULL;
    }
    //解锁
    pthread_mutex_unlock(&m_FrameMutex);

    return true;
}

//打开文件
bool CHDIPCamera::OpenFile(const char* strFileName)
{
    return true;
}


//设置相机参数
bool CHDIPCamera::Cntl_Set()
{
    #ifdef HDIPCAMERA
   /* if(m_hControlObject == NULL)
    {
        return false;
    }
    tmCommandInfo_t cmd;			//此接口主要为了给TMCC_SetConfig接口传参数使用
    tmVideoInCfg_t	struVideoInCfg;	//此接口主要设置跟VideoIn相关的参数

    memset(&cmd, 0, sizeof(tmCommandInfo_t));
    cmd.dwSize = sizeof(tmCommandInfo_t);				//该结构的大小，必须填写为sizeof(tmCommandInfo_t)
    cmd.dwMajorCommand = TMCC_MAJOR_CMD_VIDEOINCFG;		//主消息数据命令即数据类型在tmTransDefine.h中定义
    cmd.dwMinorCommand = TMCC_MINOR_CMD_VIDEOIN;		//次消息数据命令即数据类型在tmTransDefine.h中定义
    cmd.iChannel = 0;									//通道号，该通道号要根据dwMajorCommand来判断是否有效
    cmd.iStream = 0;									//子通道号，该通道号要根据dwMajorCommand来判断是否有效
    cmd.pCommandBuffer = &struVideoInCfg;				//消息数据缓冲，本次处理的tmVideoInCfg_t接口
    cmd.iCommandBufferLen = sizeof(tmVideoInCfg_t);		//消息数据缓冲大小，本次需要设置sizeof(tmVideoInCfg_t)
    cmd.iCommandDataLen = sizeof(tmVideoInCfg_t);		//消息数据实际大小，本次需要设置sizeof(tmVideoInCfg_t)
    cmd.dwResult = TMCC_ERR_SUCCESS;					//消息控制返回结果，同步模式右函数直接返回，异步模式通过此成员变量返回

    //填写VideoIn参数，不同控制命令对应不同的结构参考tmTransDefine.h中定义(或者Windows SDK Demo DvsManager)
    memset(&struVideoInCfg, 0, sizeof(tmVideoInCfg_t));
    struVideoInCfg.dwSize = sizeof(tmVideoInCfg_t);


    struVideoInCfg.byAgc = m_cfg.uGain;
    struVideoInCfg.byShutterSpeed = m_cfg.uSH;


    //通过TMCC_SetConfig设置此参数
    int iRet = TMCC_SetConfig(m_hControlObject, &cmd);
    if( iRet != TMCC_ERR_SUCCESS )
    {
        return false;
    }*/
    #endif

    return true;
}

//获取相机参数
bool CHDIPCamera::Cntl_Get()
{
    #ifdef HDIPCAMERA
   /*  if(m_hControlObject == NULL)
    {
        return false;
    }
    tmCommandInfo_t cmd;			//此接口主要为了给TMCC_SetConfig接口传参数使用
    tmVideoInCfg_t	struVideoInCfg;	//此接口主要设置跟VideoIn相关的参数

    memset(&cmd, 0, sizeof(tmCommandInfo_t));
    cmd.dwSize = sizeof(tmCommandInfo_t);				//该结构的大小，必须填写为sizeof(tmCommandInfo_t)
    cmd.dwMajorCommand = TMCC_MAJOR_CMD_VIDEOINCFG;		//主消息数据命令即数据类型在tmTransDefine.h中定义
    cmd.dwMinorCommand = TMCC_MINOR_CMD_VIDEOIN;		//次消息数据命令即数据类型在tmTransDefine.h中定义
    cmd.iChannel = 0;									//通道号，该通道号要根据dwMajorCommand来判断是否有效
    cmd.iStream = 0;									//子通道号，该通道号要根据dwMajorCommand来判断是否有效
    cmd.pCommandBuffer = &struVideoInCfg;				//消息数据缓冲，本次处理的tmVideoInCfg_t接口
    cmd.iCommandBufferLen = sizeof(tmVideoInCfg_t);		//消息数据缓冲大小，本次需要设置sizeof(tmVideoInCfg_t)
    cmd.iCommandDataLen = sizeof(tmVideoInCfg_t);		//消息数据实际大小，本次需要设置sizeof(tmVideoInCfg_t)
    cmd.dwResult = TMCC_ERR_SUCCESS;					//消息控制返回结果，同步模式右函数直接返回，异步模式通过此成员变量返回

    //填写VideoIn参数，不同控制命令对应不同的结构参考tmTransDefine.h中定义(或者Windows SDK Demo DvsManager)
    memset(&struVideoInCfg, 0, sizeof(tmVideoInCfg_t));
    struVideoInCfg.dwSize = sizeof(tmVideoInCfg_t);


    //通过TMCC_SetConfig设置此参数
    int iRet = TMCC_GetConfig(m_hControlObject, &cmd);
    if( iRet != TMCC_ERR_SUCCESS )
    {
        return false;
    }

    printf("ShutterSpeed=%d\n",struVideoInCfg.byShutterSpeed);
    printf("byAgc=%d\n",struVideoInCfg.byAgc);

    m_cfg.uGain = struVideoInCfg.byAgc;
    m_cfg.uSH = struVideoInCfg.byShutterSpeed;*/

    #endif

    return true;
}

//相机控制
bool CHDIPCamera::Control(CAMERA_CONFIG cfg)
{
    int nKind = cfg.nIndex;
    if(nKind == CAMERA_SH)
    {
        m_cfg.uSH = cfg.uSH;
        if(Cntl_Set())
        {
            m_cfg.ASC = 0;
            return true;
        }
    }
    else if(nKind == CAMERA_GAIN)
    {
        m_cfg.uGain = cfg.uGain;
        if(Cntl_Set())
        {
            m_cfg.AGC = 0;
            return true;
        }
    }
    return false;
}

//手动控制
int CHDIPCamera::ManualControl(CAMERA_CONFIG  cfg)
{
    if(cfg.nIndex == (int)CAMERA_CMD)
    {
        if (strcmp(cfg.chCmd,"sh")==0)
        {
            cfg.nIndex = (int)CAMERA_SH;
            cfg.uSH = (int)cfg.fValue;
        }
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            cfg.nIndex = (int)CAMERA_GAIN;
            cfg.uGain = (int)cfg.fValue;
        }
    }

    if((cfg.nIndex == (int)CAMERA_GAIN)
            ||(cfg.nIndex == (int)CAMERA_PEI)
            ||(cfg.nIndex == (int)CAMERA_PEW)
            ||(cfg.nIndex == (int)CAMERA_MAXGAIN)
            ||(cfg.nIndex == (int)CAMERA_SH)
            ||(cfg.nIndex == (int)CAMERA_MAXSH))
    {
        ConvertCameraParameter(cfg,false);
    }

    if(cfg.nIndex == CAMERA_SH)
    {
        cfg.uPE = cfg.uSH;
        cfg.uGain = m_cfg.uGain;
        WriteCameraIni(cfg);
    }
    else if(cfg.nIndex == CAMERA_GAIN)
    {
        cfg.uPE = m_cfg.uSH;
        WriteCameraIni(cfg);
    }

    return Control(cfg);
}

//自动控制
int CHDIPCamera::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
{
    return true;
}

//读取
int CHDIPCamera::ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert)
{
    Cntl_Get();

    if(cfg.uType == 2)//单项
    {
        int nValue = -1;
        if (strcmp(cfg.chCmd,"sh")==0)
        {
            cfg.uSH = m_cfg.uSH;
            ConvertCameraParameter(cfg,true);
            nValue = cfg.uSH;
        }
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            cfg.uGain = m_cfg.uGain;
            ConvertCameraParameter(cfg,true);
            nValue = cfg.uGain;
        }

        if(nValue != -1)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%d",nValue);
        }

        printf("cfg.uType=%d,nValue=%d\n",cfg.uType,nValue);
    }
    else
    {
        cfg.uSH = m_cfg.uSH;
        cfg.uGain = m_cfg.uGain;
        ConvertCameraParameter(cfg,true);
    }

    return true;
}

//reconnect the camera
bool CHDIPCamera::ReOpen()
{
	#ifdef HDIPCAMERA
    if(m_hObject)
    {
        Trclient_DisConnect(m_hObject);
        Trclient_UInit(m_hObject);
        m_hObject = NULL;
    }

	usleep(1000*500);

	if(!Connect())
    {
        LogError("无法与相机建立连接\r\n");
        return false;
    }
	#else
	
	m_bRequestStatus = false;

	if(!Connect())
    {
        LogError("无法与相机建立连接\r\n");
        return false;
    }

	//发送视频请求命令
	UINT32 uCmd = 0x00000001;
	SendCMD(uCmd);

    //接收视频请求回复
	UINT32 nRep = RecvCMD();

	LogNormal("RecvCMD nRep=%d\n",nRep);

    if(nRep == 1)
	{
		m_bRequestStatus = true;
	}
	else
	{
		LogError("接收视频请求回复失败!\r\n");
		m_bRequestStatus = false;
	}

	#endif

	LogNormal("相机自动重新连接\r\n");

    return true;
}

//切换工作方式
bool CHDIPCamera::ChangeMode()
{
    return true;
}

//获取相机默认模板
void CHDIPCamera::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{
    cfg.AGC = 0;
    cfg.ASC = 0;
    cfg.EEN_delay = 0;
    cfg.EEN_width = 1000;
    cfg.uSM = 0;
    cfg.uPol = 0;
    cfg.nMaxSH = 2000;
    cfg.nMaxSH2 = 6000;
    cfg.nMaxGain = 20;
    cfg.nMaxGain2 = 20;
    cfg.nGamma = 130;
    cfg.nMaxGamma = 130;
    cfg.nMode = 0;
    cfg.nLightType = 1;
    cfg.UseLUT = 0;
    cfg.nFrequency = 25;
}

//
int CHDIPCamera::GetSH(int nSH,bool bReverseConvert)
{
    if(!bReverseConvert)//客户端转相机
    {
        if( nSH >= 250000 ) //250000:11
        {
            nSH = 11;
        }
        else if(nSH >= 125000) //125000:12
        {
            nSH = 12;
        }
        else if(nSH >= 66666) //66666:13
        {
            nSH = 13;
        }
        else if(nSH >= 40000) //40000:0
        {
            nSH = 0;
        }
        else if(nSH >= 33333) //33333:1
        {
            nSH = 1;
        }
        else if(nSH >= 20000) //20000:2
        {
            nSH = 2;
        }
        else if(nSH >= 16666) //16666:3
        {
            nSH = 3;
        }
        else if(nSH >= 10000) //10000:4
        {
            nSH = 4;
        }
        else if(nSH >= 8333) //8333:5
        {
            nSH = 5;
        }
        else if(nSH >= 5555) //5555:14
        {
            nSH = 14;
        }
        else if(nSH >= 4166) //4166:6
        {
            nSH = 6;
        }
        else if(nSH >= 2083) //2083:7
        {
            nSH = 7;
        }
        else if(nSH >= 1042) //1042:8
        {
            nSH = 8;
        }
        else if(nSH >= 977) //977:9
        {
            nSH = 9;
        }
        else if(nSH >= 500)  //500:15
        {
            nSH = 15;
        }
        else if(nSH >= 250)  //250:16
        {
            nSH = 16;
        }
        else if(nSH >= 100)  //100:17
        {
            nSH = 17;
        }
    }
    else
    {
        if(nSH == 0)
        {
            nSH = 40000;
        }
        else if(nSH == 1)
        {
            nSH = 33333;
        }
        else if(nSH == 2)
        {
            nSH = 20000;
        }
        else if(nSH == 3)
        {
            nSH = 16666;
        }
        else if(nSH == 4)
        {
            nSH = 10000;
        }
        else if(nSH == 5)
        {
            nSH = 8333;
        }
        else if(nSH == 6)
        {
            nSH = 4166;
        }
        else if(nSH == 7)
        {
            nSH = 2083;
        }
        else if(nSH == 8)
        {
            nSH = 1042;
        }
        else if(nSH == 9)
        {
            nSH = 977;
        }
        else if(nSH == 11)
        {
            nSH = 250000;
        }
        else if(nSH == 12)
        {
            nSH = 125000;
        }
        else if(nSH == 13)
        {
            nSH = 66666;
        }
        else if(nSH == 14)
        {
            nSH = 5555;
        }
        else if(nSH == 15)
        {
            nSH = 500;
        }
        else if(nSH == 16)
        {
            nSH = 250;
        }
        else if(nSH == 17)
        {
            nSH = 100;
        }
    }

    return nSH;
}


//相机参数设置
void CHDIPCamera::SetCaMeraPara()
{
    //相机控制初始化
    //printf("===g_nCameraControl=%d,m_nCameraType=%d\n",g_nCameraControl,m_nCameraType);

    //初始增益和曝光时间
    if(g_uTime - g_uLastTime>=1200)
    {
        printf("InitialGainAndSH,g_uTime=%lld,g_uLastTime=%lld\n",g_uTime,g_uLastTime);
        //获取当前时间
        time_t now;
        struct tm *newTime,timenow;
        newTime = &timenow;
        time( &now );
        localtime_r( &now,newTime);

        unsigned int uTime =  newTime->tm_hour*100+newTime->tm_min;

        if((uTime>=0)&&(uTime<=500))//0->5//后半夜
        {
            m_cfg.uGain =20;//固定增益
            m_cfg.uSH = 2000;
        }
        else if((uTime>=500)&&(uTime<=830))//5->8.30//早晨
        {
            m_cfg.uGain =10;//固定增益
            m_cfg.uSH = 1500;
        }
        else if((uTime>=830)&&(uTime<=1030))//8.30->10.30//上午
        {
            m_cfg.uGain = 5;
            m_cfg.uSH = 1000;
        }
        else if((uTime>=1030)&&(uTime<=1430))//10.30->14.30//中午
        {
            m_cfg.uGain = 1;
            m_cfg.uSH = 500;
        }
        else if((uTime>=1430)&&(uTime<=1630))//14.30->16.30//下午
        {
            m_cfg.uGain = 5;
            m_cfg.uSH = 1000;
        }
        else if((uTime>=1630)&&(uTime<=1800))//16.30->18//黄昏
        {
            m_cfg.uGain = 10;
            m_cfg.uSH = 1500;
        }
        else if((uTime>=1800)&&(uTime<=2400))//18->24//前半夜
        {
            m_cfg.uGain =20;//固定增益
            m_cfg.uSH = 2000;
        }

        ConvertCameraParameter(m_cfg,false);
    }
    printf("SetCaMeraPara,m_cfg.uGain=%d,m_cfg.uSH=%d\n",m_cfg.uGain,m_cfg.uSH);
    Cntl_Set();
}

//将客户端设置的相机参数转换为相机自身能识别的参数
void CHDIPCamera::ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad)
{
    if(!bReverseConvert)//客户端转相机
    {
        cfg.nMaxGain = 224-cfg.nMaxGain * 16.0/3;
        cfg.nMaxGain2 = 224-cfg.nMaxGain2 * 16.0/3;

        cfg.nMaxSH = GetSH(cfg.nMaxSH,bReverseConvert);
        cfg.nMaxSH2 = GetSH(cfg.nMaxSH2,bReverseConvert);

        if(!bFirstLoad)
        {
            cfg.uGain = 224-cfg.uGain*16.0/3;
            cfg.uSH = GetSH(cfg.uSH,bReverseConvert);
        }


        if( (cfg.nIndex == (int)CAMERA_MAXGAIN)
                ||(cfg.nIndex == (int)CAMERA_MAXPE)
                ||(cfg.nIndex == (int)CAMERA_MAXSH))
        {
            m_cfg.nMaxPE = cfg.nMaxPE;
            m_cfg.nMaxSH = cfg.nMaxSH;
            m_cfg.nMaxGain = cfg.nMaxGain;
            m_cfg.nMaxPE2 = cfg.nMaxPE2;
            m_cfg.nMaxSH2 = cfg.nMaxSH2;
            m_cfg.nMaxGain2 = cfg.nMaxGain2;
            m_cfg.nMaxGamma = cfg.nMaxGamma;
            printf("==========m_cfg.nMaxSH2=%d\n",m_cfg.nMaxSH2);
        }
    }
    else//相机转客户端
    {
        cfg.nMaxGain = (224-cfg.nMaxGain)*3/16.0;
        cfg.nMaxGain2 = (224-cfg.nMaxGain2)*3/16.0;

        cfg.nMaxSH = GetSH(cfg.nMaxSH,bReverseConvert);
        cfg.nMaxSH2 = GetSH(cfg.nMaxSH2,bReverseConvert);

        cfg.uGain = (224-cfg.uGain)*3/16.0;
        cfg.uSH = GetSH(cfg.uSH,bReverseConvert);
    }

    cfg.nFrequency = 25;
}

//解码
bool CHDIPCamera::DecodeFrame(unsigned char* pData,int dataLen,unsigned int pts,int iFrameRate,int nKeyFrame)
{
    if(pData == NULL || dataLen <= 0)
    return false;

    //此处需要解码
    int nSize = 0;

    bool bRet = false;

    #ifdef H264_DECODE
    bRet = m_Decoder.DecodeFrame(pData,dataLen,m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
    #endif

    if(bRet&& nSize > 0)
    {
        yuv_video_buf header;
        struct timeval tv;
        gettimeofday(&tv,NULL);
        header.ts = tv.tv_sec*1000000+tv.tv_usec;//(int64_t)pts*1000*1000;
		header.uTimestamp = tv.tv_sec;
        header.nFrameRate = iFrameRate/1000;
        header.width = m_nWidth;
        header.height = m_nHeight;

        memcpy(m_pFrameBuffer,&header,sizeof(yuv_video_buf));

        AddFrame(1);
    }
    return bRet;
}

//TCP 连接
bool CHDIPCamera::connect_tcp()
{
	if(m_nTcpSocket > 0)
	{
		shutdown(m_nTcpSocket,2);
        close(m_nTcpSocket);
        m_nTcpSocket = -1;
	}

    struct sockaddr_in tcp_addr;
    // socket
    if ((m_nTcpSocket=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        return false;
    }

    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec=1;
    timeo.tv_usec=500000;//超时1.5s


    if(setsockopt(m_nTcpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
    {
        shutdown(m_nTcpSocket,2);
        close(m_nTcpSocket);
        m_nTcpSocket = -1;
        perror("setsockopt");
        return false;
    }

    if(setsockopt(m_nTcpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {
        shutdown(m_nTcpSocket,2);
        close(m_nTcpSocket);
        m_nTcpSocket = -1;
        perror("setsockopt");
        return false;
    }

	std::string strHost = g_monitorHostInfo.chMonitorHost;

    bzero(&tcp_addr,sizeof(tcp_addr));
    tcp_addr.sin_family=AF_INET;
    tcp_addr.sin_addr.s_addr=inet_addr(strHost.c_str());
    tcp_addr.sin_port=htons(g_monitorHostInfo.uMonitorPort);

    int nCount = 0;
    while(nCount < 3)//连接不上最多重试三次
    {
        if(connect(m_nTcpSocket,(struct sockaddr *)&tcp_addr,sizeof(struct sockaddr))==-1)
        {
            if(nCount == 2)
            {
                LogError("connect:%s\n",strerror(errno));
                if(m_nTcpSocket!=-1)
                {
                    shutdown(m_nTcpSocket,2);
                    close(m_nTcpSocket);
                    m_nTcpSocket = -1;
                }
                return false;
            }
        }
        else
        {
            break;
        }
        nCount++;
        usleep(1000*10);
    }

    return true;
}

//h264图像采集
void CHDIPCamera::CaptureFrame()
{
	 unsigned char* pData = new unsigned char[m_nHeight*m_nWidth];

     while(!m_bEndCapture)
     {
		 if((m_nTcpSocket > 0) && m_bRequestStatus)
		 {
			 HDIP_Frame_t header;

			 int nLen = recv(m_nTcpSocket,&header,sizeof(header),MSG_NOSIGNAL);

			 if( (nLen == sizeof(HDIP_Frame_t)) )
			 {
				 //LogNormal("cSynchron=%c%c%c%c\r\n",header.cSynchron[0],header.cSynchron[1],header.cSynchron[2],header.cSynchron[3]);
			 }
			 else if(nLen > 0)
			 {
				 LogNormal("nLen = %d\r\n",nLen);
			 }

			 if( (nLen == sizeof(HDIP_Frame_t)) && (strncmp(header.cSynchron,"$$$$",4) == 0) )
			 {
				int nLeft =  ntohl(header.length);
				int dataLen  = nLeft;

				int nBytes = 0;
				int nIndex = 0;

				while(nLeft > 0)
				{
					nBytes = recv(m_nTcpSocket,pData+nIndex,nLeft,MSG_NOSIGNAL);

					if(nBytes < 0)
					{
						break;
					}

					nLeft -= nBytes;
					nIndex += nBytes;
				}

				if(nLeft == 0)
				{
					 //此处需要解码
					int nSize = 0;
					bool bRet = false;
					#ifdef H264_DECODE
					bRet = m_Decoder.DecodeFrame(pData,dataLen,m_pFrameBuffer+sizeof(yuv_video_buf),nSize);
					#endif

					if(bRet&& nSize > 0)
					{
						yuv_video_buf header;
						struct timeval tv;
						gettimeofday(&tv,NULL);
						header.ts = tv.tv_sec*1000000+tv.tv_usec;//(int64_t)pts*1000*1000;
						header.nFrameRate = 25;
						header.width = m_nWidth;
						header.height = m_nHeight;

						memcpy(m_pFrameBuffer,&header,sizeof(yuv_video_buf));

						 //LogNormal("dataLen = %d\r\n",dataLen);

						AddFrame(1);
					}
				}
			 }
		 }

		usleep(1000*1);
     }

	 if(pData != NULL)
	 {
		 delete []pData;
	 }
}

//心跳信号
void CHDIPCamera::LinkTest()
{
	int nCount = 0;
	while(!m_bEndCapture)
	{
		if(nCount >= 400)
		{
			if((m_nTcpSocket > 0) && m_bRequestStatus)
			{
				UINT32 uCmd = 0x00000003;
				SendCMD(uCmd);

				//LogNormal("发送心跳成功");
			}
			nCount = 0;
		}
		nCount++;

		usleep(1000*10);
	}
}

//发送命令给相机
bool CHDIPCamera::SendCMD(UINT32 uCmd)
{
	HDIP_Header_t header;
	header.nCodeId = htonl(uCmd);
	header.nDeviceId = htonl(m_nDeviceId);

	int res = send(m_nTcpSocket,&header,sizeof(header),MSG_NOSIGNAL);

	if(res <= 0)
	{
		shutdown(m_nTcpSocket,2);
        close(m_nTcpSocket);
        m_nTcpSocket = -1;
		return false;
	}
	
	return true;
}

//接收相机命令
UINT32 CHDIPCamera::RecvCMD()
{
	char buffer[256];
    memset(buffer,0,sizeof(buffer));

	int res = recv(m_nTcpSocket,buffer,sizeof(buffer),MSG_NOSIGNAL);

	if(res <= 0)
	{
		shutdown(m_nTcpSocket,2);
        close(m_nTcpSocket);
        m_nTcpSocket = -1;
		return false;
	}

	UINT32  nRep = ntohl(*((UINT32*)buffer));
	
	return nRep;
}

//设置设备编号
void CHDIPCamera::SetDeviceID(int nDeviceID)
{
	m_nDeviceId = nDeviceID;

	LogNormal("m_nDeviceId=%d,%d\n",m_nDeviceId,htonl(m_nDeviceId));
}