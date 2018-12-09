#ifndef ROAD_EVENT_VIS_VIDEO_H
#define ROAD_EVENT_VIS_VIDEO_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <list>
#include <string>
using namespace std;

#include "Common.h"

// #ifndef UINT8
// typedef unsigned char	UINT8
// #endif

#ifndef ROADEVENT_UPLOAD_FTP
#define ROADEVENT_UPLOAD_FTP 0x11
#endif

#ifndef ROADEVENT_HEART_BEAT
#define ROADEVENT_HEART_BEAT 0x13
#endif

typedef struct tagstrEventVisVidoe 
{
	int dwBeginTime;   //请求视频的开始时间
	int dwEntTime;     //请求视频的结束时间
	char szDeviceId[30];    //设备编号
	int dwDir;         //方向
	char szEventTime[30];        //事件发生事件
	int dwSendTime;    //开始请求视频的时间
	int dwResult;      //视频请求结果      // 0 表示未发送， 1，表示正在发送 
}strEventVisVidoe;

struct strVisMsgHead
{
	 unsigned char bHead1;     // 协议头 0xDF
	 unsigned char bHead2;     // 协议头 0xDF
	 unsigned char bVersion;   //协议版本 Ox01
	 unsigned char bMsgType;   //消息类型 详见eBcmMsgType
	 unsigned char bLength;    //消息长度 不包含消息头长度
	 unsigned char bChecksum;  //校验位 以上五个字节之和 & 0xFF
};

typedef struct tagReqUploadVideo
{
	unsigned long long  ubiSessionId;	//任务号，依次累加，用于标识当前任务 由请求方生成
	unsigned long long ubiCameraId;	//摄像机编号 VIS系统中的摄像机长编号
	int uiStartTime;	//视频开始时间 VIS系统中的时间格式
	int uiEndTime;		//视频结束时间 VIS系统中的时间格式
	char szFtpUserName[20];	//FTP 用户名
	char szFtpPassword[20];	//FTP 密码
	char szDeviceCode[20];	//设备编码
	char szDriveDir[10];	//行驶方向编码
	char szLaneFlag[10];	//车道标识符
	char szDataTime[20];	//日期和时间 四位表示的年份+两位表示的月份+两位表示的日期＋两位表示的小时数+两位表示的分钟数+两位表示的秒数+三位表示的毫秒数
}REQUPLOADVIDEO;

class CEventVisVideo
{
public:
	CEventVisVideo();
	~CEventVisVideo();
	/*************************************
	功  能：设置VIS服务器
	参  数：szVisIp    -- 服务器地址
			dwVisPort  -- 服务器端口
			dwCameraID -- 摄像机编号
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int SetVisInfo(const char* szFtpUserName,const char* szFtpPwd,const char *szVisIp,int dwVisPort,char* szCameraID);
	/*************************************
	功  能：添加请求视频信息
	参  数：dwEventBeginTime -- 开始时间
			dwEventEndTime   -- 结束时间
			dwVideoNo        -- 录像编号
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int AddEventVideoReq(int dwEventBeginTime,int dwEventEndTime,const char* szDeviceId,int dwDir,int dwEventTime);
	/*************************************
	功  能：添加请求视频业务 比如短线重连，删除冗余请求
	参  数：
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int DealEventVideo();

private:
	int SentHeartBeat();
	int DealEvent();
private:
	/*************************************
	功  能：连接VIS服务器
	参  数：szVisIp    -- 服务器地址
			dwVisPort  -- 服务器端口
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int ConnectToVis(char *szVisIp,int dwVisPort);

	/*************************************
	功  能：发送数据
	参  数：szMsg    -- 发送数据
			uiMsgLen -- 发送数据长度
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int SendMsg(char* szMsg,int uiMsgLen);
	/*************************************
	功  能：发送数据
	参  数：
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int RecvMsg();
	/*************************************
	功  能：发送数据
	参  数：dwSocketFd socket句柄
			fTimeout   超时时间
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int WaitRecv(int dwSocketFd, float fTimeout);
	/*************************************
	功  能：发送数据
	参  数：dwSocketFd socket句柄
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int CloseSocket(int &dwSocketFd);
	/*************************************
	功  能：获取当前时间
	参  数：
	返回值：成功返回0，失败返回错误码
	*****************************************/
	int GetTimeT();
	/*************************************
	功  能：将字符串转换为unsigned long long
	参  数：szID  字符串
	返回值：成功返回0，
	*****************************************/
	unsigned long long  roadatoull(const char* szID); 

private:
	//socket句柄
	int m_ClientSocket;
	typedef std::list<strEventVisVidoe> listEvent;  
	listEvent m_listEvent;//请求列表
	//vis ip
	char m_szFtpUserName[50];
	char m_szFtpPwd[50];
	char m_szVisIp[50];
	int  m_dwVisPort;
	unsigned long long  m_dwCameraID;       //VIS端相机编号
	int  m_dwHeartBeatTime;  //上次发送心跳的时间
};

#endif