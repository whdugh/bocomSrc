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

#ifndef SKP_ROAD_VIDEO_YUV_H
#define SKP_ROAD_VIDEO_YUV_H

#include "AbstractCamera.h"
#include "structdef.h"

/******************************************************************************/
//	描述:YUV视频采集双缓冲类(cameralink)
/******************************************************************************/
//#include <string>
#ifdef H264_DECODE
#include "RoadDecode.h"
#endif
//#define MAX_SIZE 20
//#define MAX_LEN	204800

#ifdef DIO_RTSP
#include "StrmCore.h"
#include "BMErrNo.h"
#endif


#pragma pack(1)
typedef struct tagVmux6kHead
{
	unsigned char  bHead1;
	unsigned char  bHead2;
	unsigned char  bVersion;
	unsigned char  bFlag;
	unsigned char  bMsgType;
	unsigned char  bLength_h;
	unsigned char  bLength_l;
	unsigned char  bChecksum;
}VMUX6KHEAD;

//码流发送信息
typedef struct tagSetStreamParam
{
	short int iSlot;
	unsigned char bChannel; //通道号：1－4，根据板卡类型而定
	unsigned char bStreamNo; //码流号：1－4
	int iHeadFlag;  //码流属性：1 = 标准流,2 = 裸码流
	int iProtocol; /* 1=TCP, 2=UDP,3=MUTITYCAST*/
	int iPeerIP;
	int iPeerPort;
}SetStreamParam;

//解码器参数
typedef struct tagDecoderParam
{
	short int iSlot;
	unsigned char bChannel; //通道号：1－4，根据板卡类型而定
	unsigned char bStreamNo; //码流号：1－4
	short int uiResolutionH; //分辨率（H）
	short int uiResolutionV; //分辨率（V）
	int uiPeerIP;
	int uiPeerPort;
	unsigned char bAudioFlag; //音频开关 0=关闭 1=打开
	unsigned char bAudioImport; //音频输入源  0=line ,  1=mic
	unsigned char bProtocol; //1＝TCP，2＝UDP，3＝组播
	unsigned char bDecoderFomat; //编码格式: MPEG1=1, MPEG2=2, MPEG4=3 ,MJPEG=4 ,H264=5, JPEG2K=6 
	//BYTE bReserve[2];
}DecoderParam;
//数据包头结构体
typedef struct h264_head
{
	char m_Heads[4];		//标志头 "0x3E3E3E3E"
	char m_ChipType;		//芯片类型
	char m_PackType;		//封装格式,A/V,Version
	char m_FrameRate;		//帧率
	char m_CompressType;	//压缩格式，帧类型，P/N
	short m_DataRate;		//码率
	short m_SizeH;			//分辨率H
	short m_SizeV;			//分辨率V
	short m_PackSize;		//数据包长度,包含40字节的头的长度
	short m_GOP;			//GOP信息
	short m_PackCount;		//包数量
	short m_PackID;			//包序号
	int m_FrameID;			//帧ID（L）
	int m_FrameLEN;         //帧长度
	short m_CheckSum;		//校验
	unsigned int m_TimeStampSec;	//时间戳（秒）
	unsigned int m_TimeStampuSec;	//时间戳（微秒）
}h264_video_header;





/*typedef struct _h264_video_buf {
	unsigned char cSynchron[4];     //同步头
	unsigned char cType[4];         //排列类型
	unsigned short   width;			//宽
	unsigned short   height;			//高
	unsigned short nFrameRate; //帧率
	unsigned short nFieldSeq;  //场序号
	unsigned int   size;		//大小

	unsigned char uGain;     //当前增益
	unsigned char uMaxGain;  //最大增益
	unsigned char cReserved[2];  //预留
	unsigned short nVideoType;  //视频编码格式(0:h264,1:表示mjpeg)
	unsigned char nOE;           //奇场还是偶场
	unsigned char nCameraControl;    //是否控制过相机

	unsigned short uFrameType;//场图像还是帧图像
	unsigned long  nSeq;        //帧号
	int64_t          ts;       //时间戳(从零开始的相对时间)
    unsigned int   uTimestamp;  //系统实际时间

	unsigned short uTrafficSignal;//信号灯状态
	//Speed_Signal uSpeedSignal;//线圈状态
	//Speed_Signal uRadarSignal;//雷达状态
	unsigned short uMarginX;//裁切宽度
	unsigned short uMarginY;//裁切高度
	unsigned short uFlashSignal;//爆闪灯状态
	unsigned char  *data;		//数据

	_h264_video_buf()
	{
		memset(cSynchron,0,4);
		memset(cType,0,4);
		width = 0;
		height = 0;
		nFrameRate = 0;
		nFieldSeq = 0;
		size = 0;
		uGain = 0;
		uMaxGain = 20;
		memset(cReserved,0,2);

		nVideoType = 0;
		nOE = 0;
		nCameraControl = 0;

		uFrameType = 0;
		nSeq = 0;
		ts = 0;
		uTimestamp = 0;

		uTrafficSignal = 0;
		uMarginX = 0;
		uMarginY = 0;
		uFlashSignal = 0;
		data = NULL;
	}
}h264_video_buf;*/


//各命令ACK返回值
typedef struct tagCmdResponse
{
	 short int iSlot;
	 short int iChannel;
	   int iRet;
}CMDRESPONSE;
#pragma pack()
class CSkpRoadVideoYuv :public AbstractCamera
{
public:
	//构造
	CSkpRoadVideoYuv(int nCameraType,int nPixelFormat);
	//析构
	~CSkpRoadVideoYuv();

	//打开yuv视频流
    virtual bool Open();
	virtual bool Close();
	//打开文件
    virtual bool OpenFile(const char* strFileName);
    //控制
    virtual int ManualControl(CAMERA_CONFIG cfg);
	//相机自动控制
    virtual int  AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn);
    //读取
    virtual int ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert = true);
    //重启相机
    virtual bool ReOpen();
    //切换工作方式
    virtual bool ChangeMode();
     //采集yuv视频
	void DealYuvFrame();

	//采集jpg视频
	void DealJpgFrame();

	//码流发送信息
	int StartSendStream();
	//码流码流信息
	int EndSendStream();
	//设置控制命令
	int SendData(unsigned char *buf, int len);
	//接受返回信息
	int Recvdata(unsigned char *buf, int len);
	//检查协议头格式
	//int CheckHead(VMUX6KHEAD  head);
	int ConvertQBoxToH264( unsigned char *src, unsigned char  *dst);


	//关闭连接
	bool close_set_tcp();
	bool close_data_tcp();
	//int CheckHead(VMUX6KHEAD  head);
	bool conect_set_tcp();
	bool conect_data_tcp();

	int GetNextH264frame( );



	//std::string m_strTcpCameraIP;
	//NetGateWay
	int  m_nTcpCameraPort;

	int tcp_set_fd;

	int tcp_data_fd;

#ifdef H264_DECODE
	RoadDecode m_Decoder;
#endif

	string m_strData;

private:

     //初始化
	void Init();

    //灯控制
    int LightControl(bool bInit = false);

	//采集一场yuv视频数据
    int GetNextframe( );

	//采集一帧jpg视频数据
	int GetNextJpgframe();

	//相机参数设置
    void SetCaMeraPara();
    void InitialGainAndPE();

    //连接相机
    bool Connect_udp();

	//UDP 连接(单播方式)
	bool connect_udp_unicast();

    //将客户端设置的相机参数转换为相机自身能识别的参数
    virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert=false,bool bFirstLoad = false);

    //获取相机默认模板
    virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);

    //根据转换盒参数自适应分辨率
    void GetFrameInfo();

    //获取快门模式
    int GetShutterMode();

	UINT32 ConvertStringToIP(string strIP);

	string ConvertIPToString(UINT32 nIP);

	//触发模式下的相机参数调节（根据时间调节）
	void AdjustCamByTime();

//私有变量
private:

    //yuv套接字
    int m_nYuvFd;

    //快门时间
    UINT32 m_uShutterModeTime;

	//串口是否打开成功
	bool m_bComRet;

	//控制端口是否打开成功
	bool m_bControlOpen;

	//是否单播方式接收
	bool m_bUnicast;

	//相机控制ip
	string m_strControlIP;

#ifdef DIO_RTSP
	CStrmParser m_StreamParser; //码流分帧库
	sMediaSample *pMs;//提取分帧后的数据
	bool m_bisOk;
#endif

};
#endif //SKP_ROAD_VIDEO_YUV_H
