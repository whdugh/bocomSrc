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

#include "RoadVideoV4l2.h"
#include "Common.h"

//构造
CSkpRoadV4l2::CSkpRoadV4l2()
{
	//结构指针
	m_pV4l2Handle = NULL;

	m_AviCapture = NULL;

	m_AviFp = NULL;
	m_pBuffer = NULL;

	m_pVideoHandle = NULL;

	return;
}
	//析构
CSkpRoadV4l2::~CSkpRoadV4l2()
{

	return;
}

//打开设备,设备编号
bool CSkpRoadV4l2::OpenVideo(int nNo,unsigned int uWidth,unsigned int uHeight,int nCameraType)
{
    unsigned int uMode = VIDEO_YUYV;
    //设备名称
    char chDeviceName[SRIP_DEVICE_MAXLEN];
    //判断设备编号是否有效
    if (nNo < 0 || nNo >= LINUX_VIDEO_MAX)
    {
        LogError("无效的设备编号，目前最大编号为%d \r\n",LINUX_VIDEO_MAX);
        return false;
    }
    m_nCameraType = nCameraType;

     if(m_nCameraType == ANALOG_FIELD_DH||
       m_nCameraType == ANALOG_FRAME_DH)
    {
        //如果设备已经打开，返回错误
        if(m_pV4l2Handle != NULL)
        {
            LogError("采集设备已经打开，请确认!\r\n");
            return false;
        }
        //设备名称
        sprintf (chDeviceName, "%s%d", "/dev/video", nNo);
        printf("nNo=%d,chDeviceName=%s,uWidth=%d,uHeight=%d\n",nNo,chDeviceName,uWidth,uHeight);
        //v4l2结构,打开设备
        m_pV4l2Handle = (struct v4l2_handle *)v4l2_open(chDeviceName);
        //判断是否返回结构
        if (m_pV4l2Handle != NULL)
        {
            struct v4l2_video_fmt fmt;
            fmt.fmtid = uMode;
            fmt.width = uWidth;
            fmt.height = uHeight;
            fmt.bytesperline = fmt.width * 2;


            //设置视频参数
            if(v4l2_setformat(m_pV4l2Handle,&fmt) != 0)
            {
                //失败
                v4l2_close(m_pV4l2Handle);
                m_pV4l2Handle = 0;
                LogError("设置视频参数失败，关闭采集设备\r\n");
                return false;
            }
            m_bReadFile = false;
            //成功
            return true;
        }
        //返回失败
        return false;
    }
    else if(m_nCameraType == ANALOG_FIELD||
       m_nCameraType == ANALOG_FRAME)
    {
        //如果设备已经打开，返回错误
        if(m_pVideoHandle != NULL)
        {
            LogError("采集设备已经打开，请确认!\r\n");
            return false;
        }
        //设备名称
        sprintf (chDeviceName, "%s%d", "/dev/video", nNo);
        printf("nNo=%d,chDeviceName=%s,uWidth=%d,uHeight=%d\n",nNo,chDeviceName,uWidth,uHeight);
        //v4l2结构,打开设备
        m_pVideoHandle = (struct v4l2_video_handle *)open_device(chDeviceName,uWidth,uHeight);
        //判断是否返回结构
        if (m_pVideoHandle != NULL)
        {
            printf("open ok\n");
            //成功
            return true;
        }
        //返回失败
        return false;
    }
}

//关闭设备
bool CSkpRoadV4l2::CloseVideo()
{
    if(m_bReadFile)
    {
        if(m_AviCapture!=NULL)
        {
            cvReleaseCapture(&m_AviCapture);
            m_AviCapture=NULL;
        }
        if(m_AviFp!=NULL)
        {
            fclose(m_AviFp);
            m_AviFp = NULL;

            delete [] m_pBuffer;
            m_pBuffer = NULL;
        }
    }

    if(m_nCameraType == ANALOG_FIELD_DH||
       m_nCameraType == ANALOG_FRAME_DH)
    {
        //关闭设备
        if (v4l2_close(m_pV4l2Handle) !=  0)
        {
            LogError("关闭采集设备失败!\r\n");
            return false;
        }
        m_pV4l2Handle = NULL;
        return true;
    }
    else if(m_nCameraType == ANALOG_FIELD||
       m_nCameraType == ANALOG_FRAME)
    {
        if(close_device(m_pVideoHandle)!=0)
        {
            return false;
        }
        m_pVideoHandle = NULL;
        return true;
    }
}

//开始采集数据
bool CSkpRoadV4l2::StartCapture()
{
    if(m_nCameraType == ANALOG_FIELD_DH||
       m_nCameraType == ANALOG_FRAME_DH)
    {
        //源路
        int nRet = (v4l2_setsource(m_pV4l2Handle,1) == 0);
        //启动采集
        if(v4l2_startvideo(m_pV4l2Handle,-1,8) != 0)
        {
            LogError("启动采集设备失败!\r\n");
            return false;
        }
        return true;
    }
    else if(m_nCameraType == ANALOG_FIELD||
            m_nCameraType == ANALOG_FRAME)
    {
        if(start_capturing(m_pVideoHandle)!=0)
        {
            return false;
        }
        return true;
    }
}

//停止采集数据
bool CSkpRoadV4l2::StopCapture()
{
    if(m_nCameraType == ANALOG_FIELD_DH||
       m_nCameraType == ANALOG_FRAME_DH)
    {
        //停止采集
        v4l2_stopvideo(m_pV4l2Handle);
    }
    else if(m_nCameraType == ANALOG_FIELD||
            m_nCameraType == ANALOG_FRAME)
    {
        stop_capturing(m_pVideoHandle);
    }
    return true;
}

//采集数据
struct v4l2_video_buf* CSkpRoadV4l2::NextFrame()
{
    if(m_nCameraType == ANALOG_FIELD_DH||
       m_nCameraType == ANALOG_FRAME_DH)
    {
        //取下一帧数据
		//printf("CSkpRoadV4l2::before NextFrame NULL\n");
		v4l2_video_buf* pBuf = v4l2_nextframe(m_pV4l2Handle);
		//printf("CSkpRoadV4l2::after NextFrame NULL\n");

		if(pBuf != NULL)
		{
			return pBuf;
		}
		else
		{

			LogError("重启采集卡!\r\n");

			//停止采集
			v4l2_stopvideo(m_pV4l2Handle);

			//源路
			int nRet = (v4l2_setsource(m_pV4l2Handle,1) == 0);
			//启动采集
			if(v4l2_startvideo(m_pV4l2Handle,-1,8) != 0)
			{
				LogError("启动采集设备失败!\r\n");
			}

			return NULL;
		}
    }
    else if(m_nCameraType == ANALOG_FIELD||
            m_nCameraType == ANALOG_FRAME)
    {
        v4l2_video_buffer* pBuf = GetNextFrame(m_pVideoHandle);

        if(pBuf != NULL)
        {
            m_video_buf.data = pBuf->data;
            m_video_buf.info.ts = pBuf->ts;
            m_video_buf.size = pBuf->size;
        }
        else
        {
            printf("CSkpRoadV4l2::NextFrame NULL\n");
			return NULL;
        }
        return &m_video_buf;
    }
}


//释放数据
bool CSkpRoadV4l2::ReleaseVideoBuffer(struct v4l2_video_buf* buf)
{
    if(m_nCameraType == ANALOG_FIELD_DH||
       m_nCameraType == ANALOG_FRAME_DH)
    {
        //释放数据
        return v4l2_release_video_buf(buf) == 0;
    }
    return true;
}

//设置视频参数[亮度+对比度+饱和度+色调]
bool CSkpRoadV4l2::SetVideoParams(SRIP_CHANNEL_ATTR& sAttr)
{
    if(m_nCameraType == ANALOG_FIELD_DH||
       m_nCameraType == ANALOG_FRAME_DH)
    {
            //参数变化,调用对应的接口设置视频参数
            if(m_sChannelAttr.uBrightness != sAttr.uBrightness)
                v4l2_setbrightness(m_pV4l2Handle,sAttr.uBrightness);//设置亮度
            if(m_sChannelAttr.uContrast != sAttr.uContrast)
                v4l2_setcontrast(m_pV4l2Handle,sAttr.uContrast);	//设置对比度
            if(m_sChannelAttr.uSaturation != sAttr.uSaturation)
                v4l2_setsaturation(m_pV4l2Handle,sAttr.uSaturation);	//设置饱和度
            if(m_sChannelAttr.uHue != sAttr.uHue)
                v4l2_sethue(m_pV4l2Handle,sAttr.uHue);					//设置色调
    }
	else if(m_nCameraType == ANALOG_FIELD||
            m_nCameraType == ANALOG_FRAME)
    {
            if(sAttr.uBrightness > 0)
            {
                Setting_Parameter(RESETCOLOR, V4L2_CID_BRIGHTNESS,m_pVideoHandle);
                Setting_Parameter(sAttr.uBrightness, V4L2_CID_BRIGHTNESS,m_pVideoHandle);
            }
        //设置亮度
           if(sAttr.uContrast > 0)
           {
                Setting_Parameter(RESETCOLOR, V4L2_CID_CONTRAST,m_pVideoHandle);
                Setting_Parameter(sAttr.uContrast, V4L2_CID_CONTRAST,m_pVideoHandle);
           }
        //设置对比度
            if(sAttr.uSaturation > 0)
            {
                Setting_Parameter(RESETCOLOR, V4L2_CID_SATURATION,m_pVideoHandle);
                Setting_Parameter(sAttr.uSaturation, V4L2_CID_SATURATION,m_pVideoHandle);
            }
            //设置饱和度
            if(sAttr.uHue > 0)
            {
                Setting_Parameter(RESETCOLOR, V4L2_CID_HUE,m_pVideoHandle);
                Setting_Parameter(sAttr.uHue, V4L2_CID_HUE,m_pVideoHandle);
            }
        //设置色调
    }

	//保存新的视频参数值
	m_sChannelAttr = sAttr;

	return true;
}


//打开avi文件
bool CSkpRoadV4l2::OpenAvi(const char* strFileName,unsigned int& uWidth,unsigned int& uHeight)
{
	printf("strFileName=%s\r\n",strFileName);
	{
		m_strFileName = strFileName;
		m_AviCapture = NULL;
	//	m_AviCapture = cvCreateFileCapture(strFileName);
		m_AviCapture = cvCaptureFromAVI(strFileName);
		if(m_AviCapture)
		{
			m_bReadFile = true;

			m_nSeq = 0;

			IplImage*  frame = cvQueryFrame(m_AviCapture);

            uWidth = frame->width;
		    uHeight = frame->height;
		    printf("uWidth=%d\r\n",uWidth);

			return true;
		}
	}
	printf("============11111\r\n");

	return false;
}

//获取avi图像
IplImage* CSkpRoadV4l2::NextAviFrame( unsigned int& nSeq)
{
	IplImage* frame = NULL;
	int nFlag = 0;
	int64_t ts;
	struct timeb tp;

	int64_t ts1;
	struct timeb tp1;

	while(1)
	{
		ftime(&tp);
		ts = tp.time*1000+tp.millitm;

		if(m_nSeq==0)
		m_uTs = ts;//记住首帧时间戳

		nSeq = m_nSeq;
		long low = ((long)nSeq)*40-20;
		long up = ((long)nSeq)*40+20;
		long dx = (long)ts-m_uTs;

		if((dx>low) &&(dx<up))
		{
			frame = cvQueryFrame(m_AviCapture);
			if(frame)
			cvConvertImage(frame,frame,CV_CVTIMG_SWAP_RB);
			m_nSeq++;
			nSeq = m_nSeq;
			printf("capture one frame, ts = %lld\r\n",ts);
			fflush(stdout);
			break;
		}
		else if(dx>=up)
		{
		    printf("ts = %lld,m_uTs=%lld,dx=%lld,up=%lld,low=%lld\n",ts,m_uTs,dx,up,low);

		    ftime(&tp1);
            ts1 = tp1.time*1000+tp1.millitm;

			printf("before cvQueryFrame ts1 = %lld,m_uTs=%lld,dx=%lld,up=%lld,low=%lld\n",ts1,m_uTs,dx,up,low);

			frame = cvQueryFrame(m_AviCapture);

			ftime(&tp1);
            ts1 = tp1.time*1000+tp1.millitm;

			printf("after cvQueryFrame ts1 = %lld,m_uTs=%lld,dx=%lld,up=%lld,low=%lld\n",ts1,m_uTs,dx,up,low);
			m_nSeq++;
			printf("nextaviframe continue\r\n");
			fflush(stdout);
			continue;
		}
		else
		{
			nFlag = 1;
			frame = NULL;
			break;
		}
	}

	if((nFlag==0)&&(frame==NULL))//到头了
	{
		printf("====end\r\n");
		cvReleaseCapture(&m_AviCapture);
		m_AviCapture=NULL;
		m_AviCapture = cvCreateFileCapture(m_strFileName.c_str());
		m_nSeq = 0;
	}
	return frame;
}



