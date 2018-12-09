// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "RoSeekCamera.h"
#include "Common.h"
#include "CommonHeader.h"

#define CMD_DATA_BUF_SIZE  4096

//YUV采集线程
void* ThreadRoSeekCapture(void* pArg)
{
    //取类指针
    CRoSeekCamera* pRoSeekCamera = (CRoSeekCamera*)pArg;
    if(pRoSeekCamera == NULL)
        return pArg;

    pRoSeekCamera->CaptureFrame();
    pthread_exit((void *)0);
    return pArg;
}

CRoSeekCamera::CRoSeekCamera(int nCameraType)
{
    m_nCameraType = nCameraType;

    m_nWidth = 1616;
    m_nHeight = 1232;


	//m_nPort = 8881;
	m_strHost = ROSEEK_IP;
	//m_strHost = "192.168.1.218";
	//m_strHost = "192.168.0.2";
	m_nPort = ROSEEK_TCP_PORT;

    m_nMarginX = 0;
    m_nMarginY = 0;
    m_pFrameBuffer = NULL;
    m_nTcpFd = -1;
    m_nUdpFd = -1;
    m_bEndCapture = false;
    //线程ID
    m_nThreadId = 0;
}

CRoSeekCamera::~CRoSeekCamera()
{

}


void CRoSeekCamera::Init()
{
    printf("=====CRoSeekCamera::Init()====\n");

    for(int i=0; i< MAX_SIZE; i++)
    {
        m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf)+m_nWidth * m_nHeight,sizeof(unsigned char));
    }

    m_uSeq = 0;
    m_nPreFieldSeq  = 0;
    m_nCurIndex = 0;
    m_nFirstIndex = 0;
    m_nFrameSize=0;
    m_pFrameBuffer = m_FrameList[0];

//初始化相机配置
    m_GECamCfgInfo.pCMDToCamera = &m_CMDToCamera;

    m_GECamCfgInfo.pCMDToCamera->SetTargetAddr(m_strHost.c_str(), ROSEEK_TCP_CONTROL_PORT);
    //m_GECamCfgInfo.GetAllCfgInfo();

    m_bEndCapture = false;
    //线程ID
    m_nThreadId = 0;


    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    int nret=pthread_create(&m_nThreadId,NULL,ThreadRoSeekCapture,this);

    if(nret!=0)
    {
        Close();
        //失败
        LogError("创建yuv采集线程失败,无法进行采集！\r\n");
    }

    pthread_attr_destroy(&attr);
}

void CRoSeekCamera::CaptureFrame()
{
    RecvData();

    printf("after RecvData\n");
}

bool CRoSeekCamera::Open()
{
    //读取相机参数
    m_cfg.uKind = m_nCameraType;
    LoadCameraProfile(m_cfg);

    printf("m_cfg.uSH=%d,m_cfg.EEN_width=%d\n",m_cfg.uSH,m_cfg.EEN_width);

    //启动采集线程并分配空间
    Init();

    if(connect_tcp()==false)
    {
        //失败
        LogError("无法与相机建立tcp连接\r\n");
        return false;
    }

    SetCaMeraPara();

    return true;
}

bool CRoSeekCamera::Close()
{
    m_bEndCapture = true;

    if(m_nTcpFd!=-1)
    {
        shutdown(m_nTcpFd,2);
        close(m_nTcpFd);
        m_nTcpFd = -1;
    }
    printf("CRoSeekCamera=111========m_bEndCapture=%d\n",m_bEndCapture);
    if (m_nThreadId != 0)
    {
        //pthread_cancel(m_nThreadId);
        pthread_join(m_nThreadId, NULL);
        m_nThreadId = 0;
    }
    printf("===22======m_bEndCapture=%d\n",m_bEndCapture);
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
bool CRoSeekCamera::OpenFile(const char* strFileName)
{
    return true;
}

//相机参数设置
void CRoSeekCamera::SetCaMeraPara()
{
    //相机参数设置
    CAMERA_CONFIG  cfg = m_cfg;

    cfg.nIndex = CAMERA_ASC;
    Control(cfg);
    usleep(1000*5);

    cfg.nIndex = CAMERA_PEI;
    Control(cfg);
    usleep(1000*5);

    cfg.nIndex = CAMERA_PEW;
    Control(cfg);
    usleep(1000*5);

    if(cfg.AGC == 0)
    {
        cfg.nIndex = CAMERA_GAIN;
        Control(cfg);
        usleep(1000*5);
    }

    if(cfg.ASC == 0)
    {
        cfg.nIndex = CAMERA_SH;
        Control(cfg);
    }

    //开关灯控制
    LightControl(true);
}


//灯控制
int  CRoSeekCamera::LightControl(bool bInit)
{
    CAMERA_CONFIG cfg = m_cfg;

    if(!bInit)
    {
        //判断是否开灯
        cfg.uType = 2;
        memset(cfg.chCmd,0,sizeof(cfg.chCmd));
        memcpy(cfg.chCmd,"PEN",3);
        ReadCameraSetting(cfg);
    }
    else
    {
        ///////////////////////////////闪光灯控制
        int nDayNight = DayOrNight(1);
        if(nDayNight==0)//晚间
        {
            LogNormal("OpenLight\n");

            cfg.EEN_on = 1;//打开
            cfg.nIndex = CAMERA_EEN;
            Control(cfg);

            //if(g_nWorkMode == 0)
            {
                if(cfg.nMode == 1)//触发方式
                {
                    LogNormal("相机进入触发工作模式\n");
                }
            }
        }
        else if(nDayNight==1)//白天
        {
            LogNormal("CloseLight\n");

            //if(g_nWorkMode == 0)
            {
                if(cfg.nMode == 1)//触发方式
                {
                    LogNormal("相机进入连续工作模式\n");
                }
            }

            if(g_nHasHighLight == 0)
            {
                cfg.EEN_on = 0;//关闭
                cfg.nIndex = CAMERA_EEN;
                Control(cfg);
            }
            else if(g_nHasHighLight == 1)
            {
                cfg.EEN_on = 1;//打开
                cfg.nIndex = CAMERA_EEN;
                Control(cfg);
            }
        }
    }
    return cfg.EEN_on;
}

//手动控制
int CRoSeekCamera::ManualControl(CAMERA_CONFIG  cfg)
{
    if(cfg.nIndex != (int)CAMERA_MODE)
        cfg.nMode = m_cfg.nMode;

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
        }
        /*else if (strcmp(cfg.chCmd,"maxsh")==0)
        {
            cfg.nIndex = (int)CAMERA_MAXSH;
            cfg.nMaxSH = (int)cfg.fValue;
        }*/
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            cfg.nIndex = (int)CAMERA_GAIN;
            cfg.uGain = (int)cfg.fValue;
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
    }

    if(cfg.nIndex == (int)CAMERA_LUT)
    {
        m_cfg.UseLUT = cfg.UseLUT;
        //return UpdateLUT();
        return 1;
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

//相机控制
int CRoSeekCamera::Control(CAMERA_CONFIG  cfg)
{
    int nKind = cfg.nIndex;

    //先取得控制模式，连续模式，触发模式
    int m_nRunMode = 0;

    UINT32 dwAttriCMD = 0x00000000;
    UINT32 dwTemp = 0;
    int nAutoMode = 0; //是否自动控制相机，曝光，增益

    UINT32 dwAttribute;
    UINT32 dwMethod;
    UINT32 dwParam; //const char *pParam,
    UINT32 dwParamLen;

    //0:连续方式,1:触发方式
    if(cfg.nMode == 0)
    {
        m_nRunMode = ROSEEK_FconMode;
    }
    else if(cfg.nMode == 1)
    {
        m_nRunMode = ROSEEK_TrgMode;
    }

    dwMethod = EE_METHOD_SET; //设置相机
    //////////////////////曝光，增益////////////////////////////////////
    if (nKind == CAMERA_ASC)
    {
        dwAttriCMD = EE_EXPOSURE_MODE; //自动或手动
        dwAttribute = dwAttriCMD|((m_nRunMode+1)<<8);
        nAutoMode = cfg.ASC;
        dwTemp = nAutoMode?1:0;

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.ASC = cfg.ASC;
            m_GECamCfgInfo.m_CfgInfo.ExposureInfo[m_nRunMode].dwMode = dwTemp;
            return 1;
        }
        //pCMDToCamera->SendCMD(EE_EXPOSURE_MODE|((m_nRunMode+1)<<8), EE_METHOD_SET, dwTemp)
    }
    else if (nKind == CAMERA_AGC)
    {
        dwAttriCMD = EE_EXPOSURE_MODE; //自动或手动
        dwAttribute = dwAttriCMD|((m_nRunMode+1)<<8);
        nAutoMode = cfg.AGC;
        dwTemp = nAutoMode?1:0;

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.AGC = cfg.AGC;
            m_GECamCfgInfo.m_CfgInfo.ExposureInfo[m_nRunMode].dwMode = dwTemp;
            return 1;
        }
    }
    else if(nKind == CAMERA_GAIN) //增益
    {
        dwAttriCMD = EE_EXPOSURE_GAIN;
        dwAttribute = dwAttriCMD|((m_nRunMode+1)<<8);
        dwTemp = cfg.uGain;

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.uGain = cfg.uGain;
            m_cfg.AGC = 0;
            m_GECamCfgInfo.m_CfgInfo.ExposureInfo[m_nRunMode].dwGain = dwTemp;

            return 1;
        }
    }
    else if(nKind == CAMERA_SH) //快门
    {
        dwAttriCMD = EE_EXPOSURE_SHUTTERTIME;
        dwAttribute = dwAttriCMD|((m_nRunMode+1)<<8);
        dwTemp = cfg.uSH;

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            printf("=====Control==cfg.uSH = %d====\n", cfg.uSH);
            m_cfg.uSH = cfg.uSH;
            m_cfg.ASC = 0;
            m_GECamCfgInfo.m_CfgInfo.ExposureInfo[m_nRunMode].dwShutterTime = dwTemp;

            return 1;
        }
    }
    /*else if(nKind == CAMERA_MAXGAIN) //增益上限
    {
        dwAttriCMD = EE_EXPOSURE_GAINLIMIT;
        dwAttribute = dwAttriCMD|((m_nRunMode+1)<<8);
        dwTemp = cfg.nMaxGain;//增益上限

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.nMaxGain = cfg.nMaxGain;
            m_GECamCfgInfo.m_CfgInfo.ExposureInfo[m_nRunMode].dwGainLimit = dwTemp;

            return 1;
        }
    }
    else if(nKind == CAMERA_MAXSH) //快门上限
    {
        dwAttriCMD = EE_EXPOSURE_SHUTTERTIMEUPPERLIMIT;
        dwAttribute = dwAttriCMD|((m_nRunMode+1)<<8);
        dwTemp = cfg.nMaxSH;//快门上限

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.nMaxSH = cfg.nMaxSH;
            m_GECamCfgInfo.m_CfgInfo.ExposureInfo[m_nRunMode].dwGainLimit = dwTemp;

            return 1;
        }
    }*/
    //EE_EXPOSURE_MEANBRIGHTNESSTHRESHOLD 平均亮度期望值
    //////////////////////光耦合////////////////////////////////////
    /*

//光偶输入模式
    EE_ISO_INMODE
    pCfgInfo->ISOInfo.dwInMode = dwTemp;

//光偶输出模式 通用模式,同步闪光灯
    EE_ISO_OUTMODE
    pCfgInfo->ISOInfo.dwOutMode = dwTemp;

//电源同步使能 关闭,打开
    EE_ISO_POWERSYNENABLE
    pCfgInfo->ISOInfo.dwPowerSynEnable = dwTemp;

//电源同步模式 CAMERA_FREQUENCY cfg.nFrequency
	EE_ISO_POWERSYNACMODE
	pCfgInfo->ISOInfo.dwPowerSynACMode = dwTemp;

//电源同步延时 CAMERA_PEI cfg.EEN_delay
    EE_ISO_POWERSYNDELAYTIME
    pCfgInfo->ISOInfo.dwPowerSynDelayTime = dwTemp;

//闪光灯使能 CAMERA_EEN cfg.EEN_on
    EE_ISO_FLASHLAMPENABLE
    pCfgInfo->ISOInfo.dwFlashLampEnable = dwTemp;

//闪光灯模式 --通用模式,触发模式专用,连续模式专用,频闪模式(触发模式曝光时无输出),频闪模式
    EE_ISO_FLASHLAMPMODE
    pCfgInfo->ISOInfo.dwFlashLampMode = dwTemp;

//闪光灯输出宽度 CAMERA_PEW cfg.EEN_width
    EE_ISO_FLASHLAMPOUTWIDTH
    pCfgInfo->ISOInfo.dwFlashLampOutWidth = dwTemp;
	*/
    else if(nKind == CAMERA_PEI)
    {
        dwAttribute = EE_ISO_POWERSYNDELAYTIME;
        dwTemp = cfg.EEN_delay;//电源同步延时

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.EEN_delay = cfg.EEN_delay;
            m_GECamCfgInfo.m_CfgInfo.ISOInfo.dwPowerSynDelayTime = dwTemp;

            return 1;
        }
    }
    else if(nKind == CAMERA_PEW)//设置脉宽
    {
        /*
        if(cfg.nMode ==0)//连续方式
        {
            if(cntl_duration(cfg.EEN_width)==1)
            {
                m_cfg.EEN_width = cfg.EEN_width;
                return 1;
            }
        }
        else if(cfg.nMode ==1)//触发方式
        {
            if(cfg.nLightType==0)//主灯
            {
                L_SerialComm.AdjustPuls(cfg.EEN_width);
            }
            m_cfg.EEN_width = cfg.EEN_width;
            return 1;
        }
        */
        dwAttribute = EE_ISO_FLASHLAMPOUTWIDTH;
        dwTemp = cfg.EEN_width;//闪光灯输出宽度

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.EEN_width = cfg.EEN_width;
            m_GECamCfgInfo.m_CfgInfo.ISOInfo.dwFlashLampOutWidth = dwTemp;

            return 1;
        }
    }
    else if(nKind == CAMERA_POL)//极性
    {
        /*
        if(cntl_polarity(cfg.uPol)==1)
        {
            m_cfg.uPol = cfg.uPol;
            return 1;
        }
        */
        return 1;
    }
    else if(nKind == CAMERA_EEN)//开关灯
    {
        //设置，光偶输出模式 为同步闪光灯(1)
        //光偶输出模式 通用模式,同步闪光灯
        dwAttribute = EE_ISO_OUTMODE;
        //pCfgInfo->ISOInfo.dwOutMode = dwTemp;
        dwTemp = 1;
        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_GECamCfgInfo.m_CfgInfo.ISOInfo.dwOutMode = dwTemp;
            printf("==CRoSeekCamera::Control==EE_ISO_OUTMODE===\n");
        }

        //设置，闪光灯模式，连续模式专用
        //闪光灯模式 --通用模式,触发模式专用,连续模式专用,频闪模式(触发模式曝光时无输出),频闪模式
        dwAttribute = EE_ISO_FLASHLAMPMODE;
        dwTemp = 2;
        //pCfgInfo->ISOInfo.dwFlashLampMode = dwTemp;
        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_GECamCfgInfo.m_CfgInfo.ISOInfo.dwFlashLampMode = dwTemp;
            printf("==CRoSeekCamera::Control==EE_ISO_FLASHLAMPMODE===\n");
        }

        //闪光灯使能
        dwAttribute = EE_ISO_FLASHLAMPENABLE;
        dwTemp = cfg.EEN_on;

        if( m_GECamCfgInfo.pCMDToCamera->SendCMD(dwAttribute, dwMethod, dwTemp) )
        {
            m_cfg.EEN_on = cfg.EEN_on;
            m_GECamCfgInfo.m_CfgInfo.ISOInfo.dwFlashLampEnable = dwTemp;

            return 1;
        }
/*
        printf("==cfg.nMode=%d\n",cfg.nMode);
        if(cfg.nMode ==0)//连续方式
        {
            if(cntl_strobe(cfg.EEN_on)==1)
            {
                m_cfg.EEN_on = cfg.EEN_on;
                return 1;
            }
        }
        else if(cfg.nMode ==1)//触发方式
        {
            if(L_SerialComm.SetOpenAndHost(cfg.EEN_on,cfg.EEN_on))
            {
                m_cfg.EEN_on = cfg.EEN_on;
                return 1;
            }
        }
        */
    } //End of else if(nKind == CAMERA_EEN)//开关灯

    /////////////////////图像/////////////////////
/*
EE_APPEARANCE_WHITEBALANCEENABLE
			pCfgInfo->ApperanceInfo.dwWhiteBalanceEnable = dwTemp;
EE_APPEARANCE_WHITEBALANCEMODE
			pCfgInfo->ApperanceInfo.dwWhiteBalanceMode = dwTemp;
EE_APPEARANCE_IMGSAMPLEFMT
			pCfgInfo->ApperanceInfo.dwIMGSampleFMT = dwTemp;
EE_APPEARANCE_LUTENABLE
		{
			pCfgInfo->ApperanceInfo.dwLUTEnable = dwTemp;
		}
		if (dwTemp)
		{
			int nLUTLen = 4096;
			BYTE *pLUTTable = new BYTE[nLUTLen];
			for (int i = 0; i < nLUTLen; i++)
			{
				pLUTTable[i] = (BYTE)((255*2*2048 - 255*i)*i/(2048*2048));
			}

			pCMDToCamera->SendCMD(EE_APPEARANCE_LUTTABLE, EE_METHOD_SET, (char*)pLUTTable, nLUTLen);
			delete[] pLUTTable;
		}
*/
    else if(nKind == CAMERA_GAMMA)
    {
        /*
        if(cntl_gamma_Abs(cfg.nGamma)==1)
        {
            m_cfg.nGamma = cfg.nGamma;
            return 1;
        }
        */
        printf("===Control===CAMERA_GAMMA===\n");
        return 1;
    }
    else if(nKind == CAMERA_MODE)//设置工作方式
    {
        //EE_STREAM1_RUNMODE
        /****************************************
       * 工作模式 CAMERA_MODE（
       * 值为0 表示触发模式，
       * 值为1 表示全分辩率连续模式，
       * 值为2 表示高帧率模式，
       * 值为3 表示高灵敏度高帧率模式）
       ******************************************/
       /*
        if(cntl_mode(cfg.nMode)==1)
        {
            m_cfg.nMode = cfg.nMode;

            //设置帧率
            if(cfg.nMode ==0)
                cntl_frameRate_Abs(m_cfg.nFrequency);
            return 1;
        }
        */
        printf("===Control===CAMERA_MODE===\n");
        return 1;
    }
    else if(nKind == CAMERA_LIGHTTYPE)//设置灯类型
    {
        /*
        LogNormal("设置主从灯类型=%d\n",cfg.nLightType);
        L_SerialComm.SetOpenAndHost(true,(cfg.nLightType==0));

        m_cfg.nLightType = cfg.nLightType;
        */
        printf("===Control===CAMERA_LIGHTTYPE===\n");
        return 1;
    }
    else if(nKind == CAMERA_FREQUENCY) //设置触发频率
    {
        /*
        if(cfg.nFrequency < 10 || cfg.nFrequency > 25)
            cfg.nFrequency = 15;


        //设置帧率
        if(cntl_frameRate_Abs(cfg.nFrequency))
        {
            m_cfg.nFrequency = cfg.nFrequency;
        }

        if(cfg.nMode ==1)//触发方式
        {
            if(cfg.nLightType==0)//主灯
            {
                L_SerialComm.SetFrequency(cfg.nFrequency);
            }
        }
        */

        //电源同步模式 CAMERA_FREQUENCY cfg.nFrequency
        //EE_ISO_POWERSYNACMODE
        //pCfgInfo->ISOInfo.dwPowerSynACMode = dwTemp;
        printf("===Control===CAMERA_FREQUENCY===\n");
        return 1;
    }
    else if( nKind == CAMERA_IRIS)//光圈控制
    {
        /*
        if(cntl_iris(cfg.nIris)==1)
        {
            m_cfg.nIris = cfg.nIris;
            return 1;
        }
        */
        printf("===Control===CAMERA_IRIS===\n");
        return 1;
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

    return 0;
}

//自动控制
int CRoSeekCamera::AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn)
{
    //fIncrement *= 1.5;

    CAMERA_CONFIG cfg = m_cfg;//获取当前相机配置

    //LogNormal("==CRoSeekCamera::AutoControl fRate=%0.2f,fIncrement=%0.2f,uGain=%d,uSH=%d\n",fRate,fIncrement,cfg.uGain,cfg.uSH);

    int nMaxGain = m_cfg.nMaxGain;//2000;
    int nMaxSH = m_cfg.nMaxSH;//2000;
    int nMinGain = 0;
    int nMinSH = 100;
    if(!bDetectCarnum)
    {
        nMaxGain = m_cfg.nMaxGain2;//非机动车道
        nMaxSH = m_cfg.nMaxSH2;
    }

    if( (cfg.uSH == nMaxSH)&& (fRate > 1)) //SH不能再大
    {
        fRate = 1;
    }
    if( (cfg.uGain == nMaxGain)&& (fIncrement > 0.000001)) //GAIN不能再大
    {
        fIncrement = 0;
    }


    if( (cfg.uSH == nMinSH)&& (fRate < 1))  //SH不能再小
    {
        fRate = 1;
    }
    if( (cfg.uGain == nMinGain)&& (fIncrement < 0.000001)) //GAIN不能再小
    {
        fIncrement = 0;
    }

    if(fRate>1)
        cfg.uSH= cfg.uSH*fRate+0.5;
    else if(fRate<1)
        cfg.uSH = cfg.uSH*fRate;

    if( fIncrement > 0.000001  || fIncrement < -0.000001)
    {
        cfg.uGain += ((int)fIncrement);

        LogNormal("=====cfg.uGain==%d===\n", cfg.uGain);
    }


    if(cfg.uGain >=nMaxGain)//增益
        cfg.uGain = nMaxGain;
    else if(cfg.uGain<=nMinGain)
        cfg.uGain = nMinGain;

    if(cfg.uSH >=nMaxSH)//快门
        cfg.uSH = nMaxSH;
    else if(cfg.uSH<=nMinSH)
        cfg.uSH = nMinSH;

    if(cfg.ASC == 0)
    {
        if( fRate > 1 || fRate < 1)
        {
            cfg.nIndex = CAMERA_SH;
            Control(cfg);
            //LogNormal("AutoControl uSH=%d\n",cfg.uSH);
        }
    }

    if(cfg.AGC == 0)
    {
        if( fIncrement >0  || fIncrement < 0)
        {
            cfg.nIndex = CAMERA_GAIN;
            Control(cfg);
            //LogNormal("AutoControl uGain=%d\n",cfg.uGain);
        }
    }

    /*
    if(nIris != m_cfg.nIris)//光圈控制
    {
        cfg.nIris = nIris;
        cfg.nIndex = CAMERA_IRIS;
        //Control(cfg);
    }
    */

    if(fRate > 1 || fRate < 1 ||
            fIncrement >0  || fIncrement < 0)
    {
        m_bCameraControl = true;
        //写配置文件
        cfg.uPE = cfg.uSH;
        WriteCameraIni(cfg);
        //LogNormal("AutoControl uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);
    }

    //判断是否开灯
    cfg.EEN_on = LightControl();

    if(cfg.EEN_on > -1)
    {
        m_cfg.EEN_on = cfg.EEN_on;
    }

    if(g_nHasHighLight == 0)
    {

        if(g_LightTimeInfo.nLightTimeControl == 0)
        {
            //如果增益和曝光时间很大仍然未开灯则开灯
            if((nEn == 1))//进入晚间模式
            {
                if(cfg.EEN_on != 1)//根据亮度去开灯
                {

                    LogNormal("AutoControl OpenLight,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);
                    cfg.EEN_on = 1;//打开
                    cfg.nIndex = CAMERA_EEN;
                    Control(cfg);
                    usleep(1000*5);
                }
            }
            else if((nEn == 0))
            {
                    if((cfg.EEN_on != 0))//根据亮度去关灯
                    {
                        LogNormal("AutoControl CloseLight,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);

                        //关灯
                        cfg.EEN_on = 0; //关闭
                        cfg.nIndex = CAMERA_EEN;
                        Control(cfg);

                        m_nCountGain = 0;
                        usleep(1000*1);
                    }
            }
        }
        else if(g_LightTimeInfo.nLightTimeControl == 1)
        {
                int nDayNight = DayOrNight(1);
                if(nDayNight == 0)
                {
                    if(cfg.EEN_on != 1)//根据亮度去开灯
                    {
                        LogNormal("强制开灯,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);
                        cfg.EEN_on = 1;//打开
                        cfg.nIndex = CAMERA_EEN;
                        Control(cfg);
                        usleep(1000*5);
                    }
                }
                else if(nDayNight == 1)
                {
                    if((cfg.EEN_on != 0))//根据亮度去关灯
                    {
                        LogNormal("强制关灯,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);

                        //关灯
                        cfg.EEN_on = 0; //关闭
                        cfg.nIndex = CAMERA_EEN;
                        Control(cfg);

                        m_nCountGain = 0;
                        usleep(1000*1);
                    }
                }
        }
    }
    else if(g_nHasHighLight == 1)
    {
        if(cfg.EEN_on != 1)
        {
            LogNormal("强制开灯,uGain=%d,uSH=%d\n",cfg.uGain,cfg.uSH);
            cfg.EEN_on = 1;//打开
            cfg.nIndex = CAMERA_EEN;
            Control(cfg);
        }
    }

    return true;
}

//读取
int CRoSeekCamera::ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert)
{
    printf("=======CRoSeekCamera::ReadCameraSetting=====cfg.uType=%d===\n",cfg.uType);
    cfg.nMode = m_cfg.nMode;

    BOOL bRet =  m_GECamCfgInfo.GetAllCfgInfo(m_cfg);

    if(!bRet)
    {
        return 0;
    }
    ///////////////////////////////////////////////////////

    //读取相机设置
    if(cfg.uType == 2)//单项
    {
        int nValue = -1;
        if (strcmp(cfg.chCmd,"asc")==0)
        {
            nValue = m_cfg.ASC;
        }
        else if (strcmp(cfg.chCmd,"agc")==0)
        {
            nValue = m_cfg.AGC;
        }
        else if (strcmp(cfg.chCmd,"sh")==0)
        {
            //float fvalue = Abs_cntl_get("get 918 \r\n");
            //nValue = fvalue*1000000;
            nValue = m_cfg.uSH;
        }
        /*else if (strcmp(cfg.chCmd,"maxsh")==0)
        {
            nValue = m_cfg.nMaxSH;
        }*/
        else if (strcmp(cfg.chCmd,"ga")==0)
        {
            //float fvalue = Abs_cntl_get("get 928 \r\n");
            //nValue = fvalue;
            nValue = m_cfg.uGain;
        }
        else if (strcmp(cfg.chCmd,"gama")==0)
        {
            //float fvalue = Abs_cntl_get("get 948 \r\n");
            //nValue = fvalue/0.01;
            nValue = -1;
        }
    /*
        else if (strcmp(cfg.chCmd,"bn")==0)//brightness
        {
            float fvalue = Abs_cntl_get("get 938 \r\n");
            nValue = fvalue/0.01;
        }
        else if (strcmp(cfg.chCmd,"hue")==0)//hue
        {
            float fvalue = Abs_cntl_get("get 978 \r\n");
            nValue = fvalue/0.01;
        }
        else if (strcmp(cfg.chCmd,"sat")==0)//saturation
        {
            float fvalue = Abs_cntl_get("get 988 \r\n");
            nValue = fvalue/0.01;
        }
        else if (strcmp(cfg.chCmd,"pe")==0)
        {
            nValue = cntl_get("get 804 \r\n");
            nValue = nValue&0xfff;
        }
        else if (strcmp(cfg.chCmd,"epl")==0)
        {
            if(cfg.nMode == 0 )
                nValue =  get_polarity();
        }
        */
        else if (strcmp(cfg.chCmd,"pen")==0 ||
                 strcmp(cfg.chCmd,"PEN")==0)
        {
            if(cfg.nMode == 0 )
            {
                //nValue = get_strobe();
                nValue = m_cfg.EEN_on;
                cfg.EEN_on = nValue;
            }
            else if(cfg.nMode == 1 )
            {
                nValue = m_cfg.EEN_on;
                cfg.EEN_on = nValue;
            }
        }
        else if (strcmp(cfg.chCmd,"pei")==0)
        {
            //nValue = get_delay();
            //nValue *= 2;
            nValue = m_cfg.EEN_delay;
        }
        else if (strcmp(cfg.chCmd,"pew")==0)
        {
            /*
            if(cfg.nMode == 0 )
            {
                nValue = get_duration();
                nValue *= 2;
            }
            else if(cfg.nMode == 1 )
            {
                nValue = m_cfg.EEN_width;
            }
            */
            nValue = m_cfg.EEN_width;
        }
        /*
        else if (strcmp(cfg.chCmd,"luta")==0)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%f",m_cfg.LUT_a);
            return 1;
        }
        else if (strcmp(cfg.chCmd,"lutb")==0)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%f",m_cfg.LUT_b);
            return 1;
        }
        else if (strcmp(cfg.chCmd,"lutd")==0)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%d",m_cfg.LUT_d);
            return 1;
        }
        */
        else if (strcmp(cfg.chCmd,"fr")==0)
        {
            /*
            if(cfg.nMode == 0 )
            {
                float fvalue = Abs_cntl_get("get 968 \r\n");
                nValue = fvalue;
            }
            else if(cfg.nMode == 1 )
            {
                nValue = m_cfg.nFrequency;
            }
            */
            nValue = m_cfg.nFrequency;
        }
        else if (strcmp(cfg.chCmd,"iris")==0)
        {
            /*
            nValue = cntl_get("get 824 \r\n");
            float fvalue = nValue&0xfff;
            nValue = (fvalue/0xfff)*100;
            */
            nValue = -1;
        }

        if(nValue != -1)
        {
            memset(cfg.chCmd,0,sizeof(cfg.chCmd));
            sprintf(cfg.chCmd,"=%d",nValue);
        }

    }
    else if(cfg.uType == 0)//多项
    {
       // m_GECamCfgInfo.GetAllCfgInfo();
        cfg = m_cfg;
        printf("============cfg.uGain = %d,cfg.uSH=%d,cfg.EEN_width=%d\n", cfg.uGain, cfg.uSH,cfg.EEN_width);
        //ConvertCameraParameter(cfg,true);
        printf("===after=====ConvertCameraParameter\n");
    }

    return 1;
}

//reconnect the camera
bool CRoSeekCamera::ReOpen()
{
    if(m_nTcpFd!=-1)
    {
        shutdown(m_nTcpFd,2);
        close(m_nTcpFd);
        m_nTcpFd = -1;
    }

    if(connect_tcp()==false)
    {
        //失败
        LogError("无法与相机建立tcp连接\r\n");
        return false;
    }

    SetCaMeraPara();

    LogNormal("相机自动重新连接\r\n");

    return true;
}

//切换工作方式
bool CRoSeekCamera::ChangeMode()
{
    return true;
}

//获取相机默认模板
void CRoSeekCamera::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
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
    cfg.nFrequency = 15;
}

//将客户端设置的相机参数转换为相机自身能识别的参数
void CRoSeekCamera::ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad)
{
    printf("======CRoSeekCamera::ConvertCameraParameter==bReverseConvert=%d==bFirstLoad=%d==\n", bReverseConvert, bFirstLoad);
    /*
    if(!bReverseConvert)//客户端转相机
    {
        if(cfg.nMode == 0)//连续方式
        {
            cfg.EEN_delay = cfg.EEN_delay /2.0;
            cfg.EEN_width = cfg.EEN_width /2.0;
        }
        cfg.nMaxGain = cfg.nMaxGain * 100;
        cfg.nMaxGain2 = cfg.nMaxGain2 * 100;

        if(!bFirstLoad)
            cfg.uGain = cfg.uGain*100.0;

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
        if(cfg.nMode == 0)//连续方式
        {
            cfg.EEN_delay = cfg.EEN_delay*2.0;
            cfg.EEN_width = cfg.EEN_width*2.0;
        }
        cfg.nMaxGain = cfg.nMaxGain/100.0;
        cfg.nMaxGain2 = cfg.nMaxGain2/100.0;

        //printf();
        //cfg.uGain = m_GECamCfgInfo.m_CfgInfo.ExposureInfo[0].dwGain;
        printf("=CRoSeekCamera::ConvertCameraParameter=====cfg.uGain = %d====\n", cfg.uGain);
        cfg.uGain = cfg.uGain/100.0;
    }

    if(cfg.nFrequency < 10 || cfg.nFrequency > 25)
        cfg.nFrequency = 15;
    */
}

//TDP 接收数据
void CRoSeekCamera::RecvData()
{
    int		nFrameSize = 0;
    bool bRet = false;

    struct timeval tv;
    printf("===11111=======CRoSeekCamera::RecvData========m_bEndCapture=%d\n",m_bEndCapture);

    while (!m_bEndCapture)
    {
        //printf("==================m_nTcpFd=%d\n",m_nTcpFd);
        if(m_nTcpFd != -1)
        {
            //printf("==========before SafeRecv m_bEndCapture=%d\n",m_bEndCapture);

            if (!SafeRecv((char*)&nFrameSize, 4))
            {
              //  break;
            }
            //printf("==========after SafeRecv m_bEndCapture=%d\n",m_bEndCapture);
            nFrameSize = ntohl(nFrameSize);
            //printf("after ntohl nFrameSize=%d\n",nFrameSize);

            if(nFrameSize == m_nWidth*m_nHeight)
            {
                gettimeofday(&tv,NULL);

                bRet = SafeRecv((char*)(m_pFrameBuffer+sizeof(yuv_video_buf)),nFrameSize);

                if (bRet)
                {
                    yuv_video_buf camera_header;
                    memcpy(camera_header.cType,"VYUY",4);
                    camera_header.height = m_nHeight;
                    camera_header.width = m_nWidth;
                    camera_header.size = nFrameSize;

                    camera_header.nFrameRate = 15;

                    camera_header.uFrameType = 2;

                    camera_header.nSeq = m_uSeq;
                    camera_header.uTimestamp = tv.tv_sec;
                    camera_header.ts = tv.tv_sec*1000000+tv.tv_usec;

                    camera_header.uGain = m_cfg.uGain;
                    camera_header.uMaxGain = m_cfg.nMaxGain;

                    //printf("========= m_uSeq=%d,camera_header.uTimestamp=%lld\n",m_uSeq,camera_header.uTimestamp);
                    if(m_pFrameBuffer != NULL && !m_bEndCapture)
                    {
                        camera_header.data = m_pFrameBuffer+sizeof(yuv_video_buf);
                        memcpy(m_pFrameBuffer,&camera_header,sizeof(yuv_video_buf));
                    }

                    if (!m_bEndCapture)
                    {
                        AddFrame(1);
                    }

                    m_uSeq++;
                    usleep(1000);
                }
                else
                {
                    LogError("discard one frame\n");
                }
            }
        }
    }
    printf("====22222======CRoSeekCamera::RecvData========m_bEndCapture=%d\n",m_bEndCapture);
}


//TCP 连接
bool CRoSeekCamera::connect_tcp()
{
    struct sockaddr_in tcp_addr;
    // socket
    if ((m_nTcpFd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket");
        return false;
    }

    struct timeval timeo;
    socklen_t len = sizeof(timeo);
    timeo.tv_sec=1;
    timeo.tv_usec=500000;//超时1.5s


    if(setsockopt(m_nTcpFd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
    {
        perror("setsockopt");
        if(m_nTcpFd!=-1)
        {
            shutdown(m_nTcpFd,2);
            close(m_nTcpFd);
            m_nTcpFd = -1;
        }
        return false;
    }

    if(setsockopt(m_nTcpFd, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
    {
        if(m_nTcpFd!=-1)
        {
            shutdown(m_nTcpFd,2);
            close(m_nTcpFd);
            m_nTcpFd = -1;
        }
        perror("setsockopt");
        return false;
    }

    bzero(&tcp_addr,sizeof(tcp_addr));
    tcp_addr.sin_family=AF_INET;
    tcp_addr.sin_addr.s_addr=inet_addr(m_strHost.c_str());
    tcp_addr.sin_port=htons(m_nPort);

    printf("=======connect_tcp=======m_strHost.c_str()=%s,m_nPort=%d\n",m_strHost.c_str(),m_nPort);
    int nCount = 0;
    while(nCount < 3)//连接不上最多重试三次
    {
        if(connect(m_nTcpFd,(struct sockaddr *)&tcp_addr,sizeof(struct sockaddr))==-1)
        {
            if(nCount == 2)
            {
                perror("connect");
                LogError("connect:%s\n",strerror(errno));
                if(m_nTcpFd!=-1)
                {
                    shutdown(m_nTcpFd,2);
                    close(m_nTcpFd);
                    m_nTcpFd = -1;
                }
                return false;
            }
        }
        else
        {
            break;
        }
        //LogError("nCount=%d",nCount);
        nCount++;
        usleep(1000*10);
    }

    return true;
}

//UDP 连接
bool CRoSeekCamera::connect_udp()
{
    /* connect */
    struct sockaddr_in udp_addr;
    bzero(&udp_addr,sizeof(udp_addr));
    udp_addr.sin_family=AF_INET;
    udp_addr.sin_addr.s_addr= htonl(INADDR_ANY);
    udp_addr.sin_port=htons(m_nPort);

    // socket
    if ((m_nUdpFd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        return false;

    }

    int nLen = 81920000; //接收缓冲区大小(8M)
//设置接收缓冲区大小
    if (setsockopt(m_nUdpFd,SOL_SOCKET,SO_RCVBUF,&nLen,sizeof(int)) < 0)
    {
        return false;
    }

    if(bind(m_nUdpFd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) == -1)
    {
        return false;

    }
    printf("star receive date\n");
    return true;
}

//接收数据包
bool CRoSeekCamera::SafeRecv(char*pBuf, int nLen)
{
	int nRet = 0;
	int nRecved = 0;
	int nIndex = 0;
	while (nRecved < nLen)
	{
		nRet = recv(m_nTcpFd, pBuf + nRecved, nLen - nRecved, MSG_NOSIGNAL);

		//printf("==============nIndex=%d==============nRet=%d\n",nIndex,nRet);
		if (nRet <= 0)
		{
		    printf("receive date nIndex=%d,nLen=%d,nRecved=%d,strerror=%s,errno=%d\n",nIndex,nLen,nRecved,strerror(errno),errno);
			break;
		}
		nRecved += nRet;
		nIndex++;
	}
	if (nRecved < nLen)
	{
		return false;
	}
	return true;
}

//发送数据包
bool CRoSeekCamera::SafeSend(const char* pBuf, int nLen)
{
	int nRet = 0;
	int nSended = 0;
	while (nSended < nLen)
	{
		nRet = send(m_nTcpFd, pBuf + nSended, nLen - nSended, MSG_NOSIGNAL);
		if (nRet <= 0)
		{
			break;
		}
		nSended += nRet;
	}
	if (nSended < nLen)
	{
		return false;
	}
	return true;
}

/*
//TCP 控制set
bool CRoSeekCamera::SendCMD(UINT32 dwAttribute, UINT32 dwMethod, const char *pParam, UINT32 dwParamLen)
{
    if(m_nTcpFd < 0)
    return false;

	bool bRet = false;
	do
	{
        char pBackData[CMD_DATA_BUF_SIZE+1] = {0};
        char pSendData[CMD_DATA_BUF_SIZE+1] = {0};
		*(UINT32*)(pSendData) = htonl(dwAttribute);
		*(UINT32*)(pSendData + 4) = htonl(dwMethod);
		*(UINT32*)(pSendData + 8) = htonl(dwParamLen);
		if (dwParamLen > 0)
		{
			memcpy(pSendData + 12, pParam, dwParamLen);
		}
		printf("====dwParamLen=%d\n",dwParamLen);

		if (!SafeSend(pSendData, dwParamLen + 12))
		{
			break;
		}

		UINT32 dwRecvDataLenTemp = 0;
		if (!SafeRecv((char*)&dwRecvDataLenTemp, 4))
		{
			break;
		}
		UINT32 dwRecvDataLen = ntohl(dwRecvDataLenTemp);

		printf("====dwRecvDataLen=%d\n",dwRecvDataLen);

		if (dwRecvDataLen > CMD_DATA_BUF_SIZE)
		{
			break;
		}

		if (!SafeRecv(pBackData, dwRecvDataLen))
		{
			break;
		}

		printf("dwRecvDataLen=%d,BackData=%d\n",dwRecvDataLen,*((UINT32*)pBackData));

		bRet = true;
	} while(0);

	if (!bRet)
	{
		LogError("发送命令失败! 命令ID:0x%X, 方法:%d.", dwAttribute, dwMethod);
		if(m_nTcpFd!=-1)
        {
            shutdown(m_nTcpFd,2);
            close(m_nTcpFd);
            m_nTcpFd = -1;
        }
	}

	return bRet;
}
*/
