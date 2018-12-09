/*
 * File: CFmfsTel.h
 * Author:lijh
 * FMFS点播服务协议规定了点播客户端（视频数据用户）向服务器发起的指令请求规则，
 * 默认情况下FMFS点播服务器通过TCP端口8999向点播客户端提供协议服务。
 * Created on 2011年5月17日, 上午9：13
 */
#ifndef _FMFSTEL_H_
#define _FMFSTEL_H_

#include "CSocketBase.h"

#ifdef H264_DECODE
#include "RoadDecode.h"
#endif

typedef struct
{
	unsigned short	year;
	unsigned char	month;
	unsigned char	day;
	unsigned char	hour;
	unsigned char	minute;
	unsigned char	second;
	unsigned char	frame;

}TimeStamp_t, *pTimeStamp_t;

typedef enum onDemandMode
{
	    ServerInit, //初始化点播服务器
		GetVolumeScope,//获得卷上的视频数据时间范围
		StartOnlyForward,//有起点无终点顺序点播
		StartOnlyBackward,//有起点无终点逆序点播
		SectionVideoForward,//顺序点播一段视频
		SectionVideoBackward,//逆序点播一段视频
		EndofStartOnly//终止通知
}onDemandMode;

typedef struct onDemandCommand//发送结构定义
{
	char	commandUuid[16];
	onDemandMode	onDemandModeCode;
	TimeStamp_t		startTime;
	TimeStamp_t		endTime;
	short			onDemandSpeed;
	char			answerIP[20];
	unsigned short	answerPort;
	char			arrayIP[20];
	unsigned int	recBytes;
	unsigned int	volumeID;
	unsigned int	camID;
}onDemandCommand;

typedef struct //帧描述结构定义
{
	unsigned char flag; 
	TimeStamp_t timeStamp;
	unsigned int length;
	unsigned int data;
}Frame_t, *pFrame_t;

typedef struct onDemandGetScope{
	unsigned int	volumeID;
	unsigned int	camID;
	TimeStamp_t		m_currentTime;
	unsigned int	frames;
	int			isSuccess;
}onDemandGetScope;

typedef struct onDemandStatus{
	int fetchStatus;
	unsigned int videoHeight;
	unsigned int videoWidth;
}onDemandStatus;


class CFmfsTel : public mvCSocketBase
{
public:
	CFmfsTel();
	~CFmfsTel();
	bool ConnectWithFmfs(const string& strHost, int nPort=8999, int nOpt=SO_REUSEADDR);
	bool SendToFmfs(const onDemandCommand &command, onDemandGetScope &param);
	//打开接收数据服务器
	bool OpenDataServer(int nPort);
	//接收连接
   void AcceptConnects();
   //设置视频信息
   void SetVideoInfo(yuv_video_buf* pVideoInfo);
   //获取视频信息
   yuv_video_buf* GetVideoInfo();
   //处理视频数据
   void DealSteamData();
   //增加一帧数据
   void AddFrame(unsigned char* pData,int dataLen, TimeStamp_t timestamp);
   //获取一帧数据
   int PopFrame(yuv_video_buf& header,unsigned char* response);
   //关闭接收视频流
   bool CloseVideo();
   //重新打开视频流
   bool ReopenVideo();
   //视频高
   unsigned int GetHeight()
   {
	   return m_uHeight;
   }
   //视频宽
   unsigned int GetWidth()
   {
	   return m_uWidth;
   }
   //关闭与FMFS的连接
   void CloseFmfsConnect();
protected:
   bool Send(int nSocket, char *szBuf, int nLength);
   bool Recv(int nSocket, int nLen,string& strMsg);
   bool WaitConnect(int nSocket, string strHost, int nPort, int nSec);
private:
	string m_strHost;//FMFS服务器IP
	int m_nCtrlPort;//与FMFS服务器连接的端口

	int m_nCtrlSocket;//控制与FMFS建立连接发命令
	int m_nDataSocket;//接收帧数据
	int m_nListenSocket;//监听传送数据
	int m_nDataPort;//接收视频数据端口
	pthread_mutex_t m_mutex;
	//信号互斥
	pthread_mutex_t m_Frame_Mutex;

	pthread_t m_nListenId, m_nRecvId;
	bool m_bEndRecv;
	static void * ThreadFrame(void *param);
	static void * ThreadAccept(void *param);
   long MakeTime(TimeStamp_t time);
   //开启数据接收线程
   bool StartThreads();
#ifdef H264_DECODE
	RoadDecode m_Decoder;
#endif
protected:
	yuv_video_buf m_VideoInfo;

	unsigned int m_uWidth;//视频宽
	unsigned int m_uHeight;//视频高

	list<string> m_listFrame;//存放一帧数据
   long m_lEndTime;
};
#endif