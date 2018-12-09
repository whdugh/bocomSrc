// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#include "H3CCamera.h"
#include "XmlParaUtil.h"

#ifndef NOH3CCAMERA

#define VIDEO_ID "1"
#define FRAME_RATE 5

void *cThis = NULL;

void STDCALL H264VideoCallBack(IN const USER_LOGIN_ID_INFO_S *pstUserLoginIDInfo,
                                        IN const CHAR *pcChannelCode,
                                        IN const XP_PARSE_VIDEO_DATA_S *pstParseVideoData,
                                        IN LONG lUserParam,
                                        IN LONG lReserved)
{
	H3CCamera *pH3c = (H3CCamera*)cThis;
    if(!pH3c->m_bLogin)
            return;
    if(pstParseVideoData->ulWidth != pH3c->getWidth() || pstParseVideoData->ulHeight != pH3c->getHeight())
    {
            LogError("recv H3C video width=%d, height=%d\n", pstParseVideoData->ulWidth, pstParseVideoData->ulHeight);
            LogError("H3c video width and height set error!\n");
            return;
    }

    //H264解码
    int nSize = 0;

    bool bRet = false;
    bRet = pH3c->m_Decoder.DecodeFrame(pstParseVideoData->pucData, pstParseVideoData->ulDataLen, pH3c->getFrameBuffer()+sizeof(yuv_video_buf), nSize);
    if(!bRet)
    {
		LogError("DecodeFrame error!\n");
		return;
    }

	struct timeval now;
	gettimeofday(&now, NULL);

	yuv_video_buf header;
    header.ts = (int64_t)now.tv_sec * 1000000 + now.tv_usec; //pstParseVideoData->dlTimeStamp;
	header.uTimestamp = now.tv_sec;			// pstParseVideoData->dlTimeStamp;
    header.nFrameRate = FRAME_RATE;
    header.width = pstParseVideoData->ulWidth;
    header.height = pstParseVideoData->ulHeight;
    memcpy(pH3c->getFrameBuffer(), &header, sizeof(yuv_video_buf));

	pH3c->addH3cFrame(1);
}

//YUV回调函数
void STDCALL YuvVideoCallBack(IN const USER_LOGIN_ID_INFO_S *pstUserLoginIDInfo,
							   IN const CHAR *pcChannelCode,
							   IN const XP_PICTURE_DATA_S *pstParseVideoData,
							   IN LONG lUserParam,
							   IN LONG lReserved)
{
	H3CCamera *pH3c = (H3CCamera*)cThis;	
	if(!pH3c->m_bLogin)
		return;
	
	if(pstParseVideoData->ulPicWidth != pH3c->getWidth() || pstParseVideoData->ulPicHeight != pH3c->getHeight())
	{
		printf("recv H3C camera's width=%d, height=%d\n", pstParseVideoData->ulPicWidth, pstParseVideoData->ulPicHeight);
		printf("H3c camera's width and height set error!\n");
		return;
	}
	
	unsigned char *pBuf = pH3c->getFrameBuffer() + sizeof(yuv_video_buf);
	int size = pH3c->getWidth() * pH3c->getHeight();
	memcpy(pBuf, pstParseVideoData->pucData[0], size);
	pBuf += size;
	memcpy(pBuf, pstParseVideoData->pucData[1], size/4);
	pBuf += size/4;
	memcpy(pBuf, pstParseVideoData->pucData[2], size/4);

	yuv_video_buf header;
	header.ts = pstParseVideoData->dlRenderTime;
	header.uTimestamp = pstParseVideoData->dlRenderTime;
	header.nFrameRate = FRAME_RATE;
	header.width = pstParseVideoData->ulPicWidth;
	header.height = pstParseVideoData->ulPicHeight;
	memcpy(pH3c->getFrameBuffer(), &header, sizeof(yuv_video_buf));

	pH3c->addH3cFrame(1);
}

/* 用于定时保活的线程 */
void *thr_keepAlive(void *arg)
{
	H3CCamera *pH3CCamera = (H3CCamera*)arg;
	while((!g_bEndThread) && TRUE == pH3CCamera->m_bLogin)
	{
		ULONG ulRet = IMOS_UserKeepAlive(&pH3CCamera->m_login.stUserLoginIDInfo);
		if(ERR_COMMON_SUCCEED != ulRet)
		{
			LogError("IMOS_UserKeepAlive error ,errcode is [%d] \r\n",ulRet);
		}
		sleep(20);
	}
	pthread_exit((void *)0);
}

H3CCamera::H3CCamera(int nCameraType)
{
	m_nCameraType = nCameraType;

	m_nWidth = 720;
	m_nHeight = 576;
	m_nMarginX = 0;
	m_nMarginY = 0;
	m_pFrameBuffer = NULL;

	m_nMaxBufferSize = 10;
	m_nDeviceId = 1;

	m_bLogin = FALSE;
	memset(m_serverHost, 0, IMOS_IPADDR_LEN);
	m_ulServerPort = 0;
	
	m_pFrameBuffer = NULL;

	m_nKeepAliveId = 0;

	//相机编号
	m_nDeviceId = 0;
}

H3CCamera::~H3CCamera()
{
}

bool H3CCamera::Open()
{
	m_Decoder.SetVideoSize(m_nWidth,m_nHeight);
  	m_Decoder.SetVideoCodeID(0);
	m_Decoder.InitDecode(NULL);
    for(int i=0; i< MAX_BUFFER_SIZE; i++)
    {
            m_FrameList[i] = (unsigned char*)calloc(sizeof(yuv_video_buf) + m_nWidth*m_nHeight*3, sizeof(unsigned char));
    }
    m_pFrameBuffer = m_FrameList[0];
	
	if (!imos_init(g_monitorHostInfo.chMonitorHost, g_monitorHostInfo.uMonitorPort))
	{
		return false;
	}
	if (!imos_login(g_monitorHostInfo.chUserName, g_monitorHostInfo.chPassWord))
	{
		return false;
	}
	if (!imos_StartPlayer())
	{
		return false;
	}
	if (!imos_setVideoDataCallBack())
	{
		return false;
	}
	if (!imos_startReal(VIDEO_ID))
	{
		return false;
	}
	return true;
}

bool H3CCamera::Close()
{
	m_Decoder.UinitDecode();
    imos_stopReal(VIDEO_ID);
    imos_stopPlayer();
    imos_logout();
    imos_clean();

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
    pthread_mutex_unlock(&m_FrameMutex);

	if(m_nKeepAliveId != 0)
	{
		pthread_join(m_nKeepAliveId, NULL);
		m_nKeepAliveId = 0;
	}
	return true;
}

bool H3CCamera::ReOpen()
{
	Close();
	Open();
}

//切换工作方式
bool H3CCamera::ChangeMode()
{
	return true;
}

//获取相机默认模板
void H3CCamera::GetDefaultCameraModel(CAMERA_CONFIG& cfg)
{
}

void H3CCamera::ConvertCameraParameter(CAMERA_CONFIG& cfg, bool bReverseConvert, bool bFirstLoad)
{
}

//设置设备编号
void H3CCamera::SetDeviceID(int nDeviceID)
{
	m_nDeviceId = nDeviceID;
}

/* IMOS sdk 初始化*/
bool H3CCamera::imos_init(char serverHost[IMOS_IPADDR_LEN], ULONG ulServerPort)
{
	memcpy(m_serverHost, serverHost, IMOS_IPADDR_LEN);
	m_ulServerPort = ulServerPort;
	LogNormal("H3C IMOS start init; ServerHost=%s, ServerPort=%d \r\n",serverHost,ulServerPort);

	ULONG ulRet = ERR_COMMON_FAIL;
	ulRet = IMOS_Initiate((char*)serverHost, ulServerPort, BOOL_TRUE, BOOL_TRUE);
	if(ERR_COMMON_SUCCEED != ulRet)
	{
		LogError("IMOS_initiate error ,errcode is [%d] \r\n",ulRet);
		return false;
	}
	LogNormal("IMOS_initiate success!\r\n");
	return true;
}


/* 登陆，注意需要为test用户配置对应的权限功能 */
bool H3CCamera::imos_login(char username[], char passwd[])
{
	LogNormal("username=%s,passwd=%s \r\n",username,passwd);
	ULONG ulRet = ERR_COMMON_FAIL;

	if(FALSE == m_bLogin)
	{
		CHAR md5Passwd[IMOS_PASSWD_ENCRYPT_LEN] = {0};

		/* change to MD5 passwd */
		IMOS_Encrypt(passwd, strlen(passwd), md5Passwd);
		ulRet = ERR_COMMON_FAIL;
		ulRet = IMOS_Login ((char*)username, md5Passwd, "" ,&m_login);
		if(ERR_COMMON_SUCCEED != ulRet)
		{
			LogError("IMOS_Login error ,errcode is [%d] \r\n",ulRet);
			return false ;
		}

		int ret = 0;
		m_bLogin = TRUE;
		ret = pthread_create(&m_nKeepAliveId, NULL, thr_keepAlive, this);
		if(0 != ret)
		{
			LogError("can not create thread \r\n");
			return false;
		}
		LogNormal("crate KeepAlive thread success\n");
		LogNormal("IMOS login success \r\n");
		return true;
	}
	return false;
}

//注册通道
bool H3CCamera::imos_StartPlayer ()
{
	ULONG ulRet = ERR_COMMON_FAIL;
	ulRet = IMOS_StartPlayer(&m_login.stUserLoginIDInfo, 1, m_playInfo);
	if(ERR_COMMON_SUCCEED != ulRet)
	{
		LogError("imos_StartPlayer error ,errcode is [%d] (0x[%x]) \r\n",ulRet, ulRet);
		return false;
	}

	LogNormal("imos_StartPlayer success \r\n");
	return true;
}

bool H3CCamera::imos_setVideoDataCallBack()
{
/*	XP_DECODE_VIDEO_DATA_CALLBACK_PF pfnCBFun = YuvVideoCallBack;
	ULONG ulRet = IMOS_SetDecodeVideoDataCB(&m_login.stUserLoginIDInfo,
											m_playInfo[0].szPlayWndCode,
											pfnCBFun, 
											false,
											(unsigned long)this);
*/	
	XP_PARSE_VIDEO_DATA_CALLBACK_PF pfnCBFun = H264VideoCallBack;
    ULONG ulRet = IMOS_SetParseVideoDataCB(&m_login.stUserLoginIDInfo,
										   m_playInfo[0].szPlayWndCode,
										   pfnCBFun, 
										   false,
										   0);

	if(ERR_COMMON_SUCCEED != ulRet)
	{
		LogError("IMOS_SetDecoderVideoDataCB error ,errcode is [%d] \r\n",ulRet);
		return false;
	}
	cThis = (void*)this;

	LogNormal("IMOS_SetDecoderVideoDataCB success \r\n");
	return true;
}

bool H3CCamera::imos_startReal(char *pXpCode)
{
	ULONG ulRet = ERR_COMMON_FAIL;
	ulRet = IMOS_StartMonitor(&m_login.stUserLoginIDInfo,
								(char*)pXpCode,
								m_playInfo[0].szPlayWndCode,
								IMOS_FAVORITE_STREAM_ANY,
								USER_OPERATE_SERVICE);

	if(ERR_COMMON_SUCCEED != ulRet)
	{
		LogError("IMOS_StartMonitor error ,errcode is [%d] \r\n",ulRet);
		return false;
	}
	LogNormal("IMOS_StartMonitor success  \r\n");
	return true;
}

bool H3CCamera::imos_stopReal(char *pXpCode)
{
	ULONG ulRet = ERR_COMMON_FAIL;
	ulRet = IMOS_StopMonitor(&m_login.stUserLoginIDInfo,
							(char*)pXpCode,
							USER_OPERATE_SERVICE);
	if(ERR_COMMON_SUCCEED != ulRet)
	{
		LogError("IMOS_StopMonitor error ,errcode is [%d] \r\n",ulRet);
		return false;
	}

	LogNormal("IMOS_StopMonitor success \r\n");
	return true;
}

bool H3CCamera::imos_stopPlayer()
{
	ULONG ulRet = ERR_COMMON_FAIL;
	ulRet = IMOS_StopPlayer(&m_login.stUserLoginIDInfo);
	if(ERR_COMMON_SUCCEED != ulRet)
	{
		LogError("IMOS_StopPlayer error ,errcode is [%d] \r\n",ulRet);
		return false;
	}

	LogNormal("IMOS_StopPlayer success \r\n");
	return true;
}

bool H3CCamera::imos_logout()
{
	ULONG ulRet = ERR_COMMON_FAIL;
	if(TRUE == m_bLogin)
	{
		m_bLogin = FALSE;
		ulRet = IMOS_Logout(&m_login.stUserLoginIDInfo);
		if(ERR_COMMON_SUCCEED != ulRet)
		{
			LogError("imos_logout error ,errcode is [%d] \r\n",ulRet);
			return false;
		}
		LogNormal("imos_logout success\r\n");
		return true;
	}
	else
	{
		LogError("imos_logout failed\r\n");
		return false;
	}
}


/* IMOS sdk 反初始化，参数说明可参考使用手册 */
bool H3CCamera::imos_clean()
{
	ULONG ulRet = ERR_COMMON_FAIL;
	ulRet = IMOS_CleanUp(&m_login.stUserLoginIDInfo);
	if(ERR_COMMON_SUCCEED != ulRet)
	{
		LogError("imos_clean error ,errcode is [%d] \r\n",ulRet);
		return false;
	}
	LogNormal("imos_clean success\r\n");
	return true;
}

#endif


