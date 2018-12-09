// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2013 上海博康智能信息技术有限公司
// Copyright 2008-2013 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _DSP_DEALVIDEO_H_
#define _DSP_DEALVIDEO_H_

//#include "AbstractCamera.h"
//#include "CSocketBase.h"
//#include "mvdspapi.h"
#include "RoadH264Capture.h"
#include "RoadRecorder.h"

#include "global.h"

#ifndef NUM_VIDEO_BUFF
#define NUM_VIDEO_BUFF	3
#endif

#ifndef MAX_RTSP_HEAD
#define MAX_RTSP_HEAD 4
#endif

#define MAX_VIDEO_BUFF 3000000
typedef struct tag_DSP_BuffNode {
	int		sizeBuff;
	char	pBuff[MAX_VIDEO_BUFF];
	bool	bIsLocked;

	tag_DSP_BuffNode()
	{
		sizeBuff = 0;
		memset(pBuff, 0, MAX_VIDEO_BUFF);
		bIsLocked = false;
	}
} DSP_BUFFNODE;



#ifdef DIO_RTSP_RECV
	#ifndef DIO_API
		#include "DIO.h"
		using namespace DIO;
	#endif
#endif

class CDspDealVideo
{
public:
	CDspDealVideo();
	~CDspDealVideo();	

	//初始化
	bool Init(int nChannel);
	bool InitUnit(DspVideoClient_Fd &dspVideoCfd, 
		bool bH264Capture, bool bSkpRecorder, 
		CRoadH264Capture * pH264Capture, CSkpRoadRecorder * pSkpRecorder);

	//采集录像数据
	void CaptureVideo();

	//添加一个Rtsp接收
	bool AddRtspcfd(DspVideoClient_Fd &dspVideoCfd,
		bool bH264Capture, bool bSkpRecorder, 
		CRoadH264Capture * pH264Capture, CSkpRoadRecorder * pSkpRecorder);

	//拷贝接受Rtsp数据
	void RecvRtspDioData(unsigned char *pData, const int iDataLen);

	//释放一个接收单元
	bool UnInitUnit(DspVideoClient_Fd &dspVideoCfd);

#ifdef DIO_RTSP_RECV
	//初始化rtsp库DIO
	int RtspDIOInit(UINT64 &ubiRtspStrmId, UINT64 &ubiTCPStrmId, const int iConnType,
		const char *pIp, const int uPort, const char *pUrl,
		const char *pUser, const char *pPw);
	//接入基础RTSP库
	void RecvRTSP();	
#endif
	
	//采集数据存录像
	void CaptureVideoFrame(const int64_t &tick, const DSP_BUFFNODE * pDspBufferNode);
private:
	pthread_t m_nThreadVideo;

	int m_nRtspHeadLen;
	char m_chRtspHead[MAX_RTSP_HEAD];//接收rtspHead
	DSP_BUFFNODE	listVideoBuff[NUM_VIDEO_BUFF];		//装载h264数据的环形缓冲数组
	unsigned int	m_uPrevVideoBuff;				//环形缓冲的前一次使用索引
	unsigned int	m_uCurrVideoBuff;				//环形缓冲的当前使用索引

	pthread_mutex_t m_rtspdio_mutx;//装载JPG图像数据缓冲区锁

	DspVideoClient_Fd m_cfd;

	//h264录象类
	CRoadH264Capture *m_pH264Capture;
	//是否接收H264码流
	bool m_bH264Capture;

	//h264违章事件录象类
	CSkpRoadRecorder *m_pSkpRecorder;
	//是否接收违章事件H264码流
	bool m_bSkpRecorder;

	bool m_bEndCapture;

	FILE *m_fpOut;

	int64_t m_uBeginTime;
	int64_t m_uEndTime;
	int m_nChannelId;
	std::string m_strH264FileName;	
};

#endif
