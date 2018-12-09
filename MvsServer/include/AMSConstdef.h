// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef AMS_CONSTDEF_H
#define AMS_CONSTDEF_H

//心跳
#define AMS_LINK_TEST 0x60000002

//车牌记录
#define AMS_PLATE_INFO 0x60100004
//车牌记录回复
#define AMS_PLATE_INFO_REP 0x70100004

//事件记录
#define AMS_EVENT_INFO 0x60100006
//事件记录回复
#define AMS_EVENT_INFO_REP 0x70100006

//历史视频分析时间查询
#define AMS_VIDEOTIME_INFO 0x60100007
//历史视频分析时间回复
#define AMS_VIDEOTIME_INFO_REP 0x70100007

//通道情况报告
#define AMS_CHANNEL_INFO 0x60100008

//统计记录
#define AMS_STATISTIC_INFO 0x60100009
//统计记录回复
#define AMS_STATISTIC_INFO_REP 0x70100009

//日志记录
#define AMS_LOG_INFO 0x6010000A
//日志记录回复
#define AMS_LOG_INFO_REP 0x7010000A

//历史视频分析请求
#define AMS_VOD_REQUEST 0x6010000B
//历史视频分析请求回复
#define AMS_VOD_REQUEST_REP 0x7010000B

//历史视频分析完成报告
#define AMS_VOD_FINISH 0x6010000C
//历史视频分析完成报告回复
#define AMS_VOD_FINISH_REP 0x7010000C

//发送比对标定信息
#define AMS_FEATURE_CALIBRATION 0x6010000d

//发送组织相机列表信息
#define AMS_CAMERA_INFO 0x6010000e

//视频切换请求
#define AMS_SWITCH_CAMERA 0x6010000f

//视频切换请求回复
#define AMS_SWITCH_CAMERA_REP 0x7010000f

//删除分析通道请求
#define AMS_DELETE_CHANNEL 0x60100010

//删除分析通道请求回复
#define AMS_DELETE_CHANNEL_REP 0x70100010


/*#ifndef MVSBAK
	#define MVSBAK
#endif*/

#ifdef MVSBAK
//MVS通道列表即相机IP信息
#define AMS_CHANNEL_LIST_INFO	0x80100001
//AMS通知备份MVS接管DSP
#define AMS_BAK_START_DSP		0x80100002
//AMS通知备份MVS停止接管DSP
#define AMS_BAK_STOP_DSP		0x80100003
#endif
//////////////////////////////////////////////
//历史视频分析时间回复包
typedef struct _HISTORY_VIDEO_TIME_INFO
{
	UINT32 uCameraID;     //视频ID
	UINT32 uBeginTime;		//最早时间
	UINT32 uEndTime;		//最晚时间
	UINT32 uRealTime;		//是否有实时在检测
	char chReserved[124];	//扩展

	_HISTORY_VIDEO_TIME_INFO()
	{
		uCameraID = 0;
		uBeginTime = 0;
		uEndTime = 0;
		uRealTime = 0;
		memset(chReserved,0,sizeof(124));
	}
}HISTORY_VIDEO_TIME_INFO;


//通道情况
typedef struct _CHANNEL_INFO_RECORD
{
    UINT32 uDeviceID;          //设备ID
    UINT32 uChannelID;		//通道ID
    UINT32 uWorkStatus;		//工作状态
	UINT32 uCameraID;     //视频ID
	UINT32 uRealTime;		//是否实时
	UINT32 DetectorType;//检测类型(0:停车;1:逆行;2:横穿;3:堵塞;4:变道;5:丢弃物;6:超速;7:目标出现)
    char      chReserved[128];


	_CHANNEL_INFO_RECORD()
	{
	    uDeviceID = 0;
		uChannelID = 0;
		uWorkStatus = 0;
		uCameraID = 0;
		uRealTime = 0;
		DetectorType = 0;
		memset(chReserved,0,sizeof(128));
	}
}CHANNEL_INFO_RECORD;
//通道情况链表
typedef std::list<CHANNEL_INFO_RECORD> ChannelInfoList;


//历史视频文件记录
typedef struct _VOD_FILE_INFO
{
	UINT32 uCameraID;     //视频ID
	UINT32 uBeginTime;		//开始时间
	UINT32 uEndTime;		//结束时间
	char chFilePath[256];	//文件名称
	char chReserved[64];	//扩展

	_VOD_FILE_INFO()
	{
		uCameraID = 0;
		uBeginTime = 0;
		uEndTime = 0;
		memset(chFilePath,0,sizeof(256));
		memset(chReserved,0,sizeof(64));
	}
}VOD_FILE_INFO;
#endif
