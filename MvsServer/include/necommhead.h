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
#ifndef NECOMMHEAD_ZLG_H
#define NECOMMHEAD_ZLG_H

#ifdef WIN32
#define INT64  LONGLONG
#define UINT64  ULONGLONG
#else
#define UINT32 unsigned int
#define INT64 long long
#define UINT64 unsigned long long
#endif

#define NE_SERVER_IP_ADDR	"127.0.0.1"		//服务器IP地址
#define NE_AGENT_SERVER_PORT	35858		//网元agent程序作为Server时，绑定端口
#define NE_PROGRAM_SERVER_PORT	35959		//网元业务程序作为Server时，绑定端口

#define HEARTBIT_TIME_OUT 120
#define HEARTBIT_TIME 30

#define VIS_S_OK 0
#define VIS_E_FAIL -1

//消息协议类型
enum eNeMsgType
{
	NE_MT_UNKNOWN =	0,							//未知类型消息
	NE_REQ_NOTE_HEARTBIT =	1,					//连接心跳，可不发
	NE_RET_NOTE_HEARTBIT =	2,					//返回连接心跳
	//公共协议类型，每个业务系统都要实现
	//NE_NOTE_COMMON_PROGRAM_START = 0x1801,	//业务程序启动时发送通知//已停用
	//NE_NOTE_COMMON_PROGRAM_RESTART = 0x1802,	//业务程序需要重启时发送通知//已停用
	NE_NOTE_COMMON_PROGRAM_STATUS = 0x1803,		//业务程序启动和重启时发送通知

	//智能检测设备相关
	NE_REQ_IDE_CAMERA_STATUS = 0x1820,			//摄像机状态查询
	NE_RET_IDE_CAMERA_STATUS = 0x1821,			//摄像机状态查询返回
	//NE_NOTE_IDE_CAMERA_VIDEOLOST = 0x1822,	//摄像机视频丢失时告警//已停用
	//NE_NOTE_IDE_CAMERA_POSITIONMOVE = 0x1823,	//摄像机位置移动告警//已停用
	NE_NOTE_IDE_DEVICE_ALARM = 0x1824,			//智能检测设备，统一设备告警

	//VIS相关
	NE_REQ_VIS_USER_ONLINE_LIST = 0x1840,		//请求用户状态列表
	NE_RET_VIS_USER_ONLINE_LIST = 0x1841,		//请求用户状态列表返回
	NE_REQ_VIS_TRUNK_STATUS_LIST = 0x1842,		//请求干线状态列表
	NE_RET_VIS_TRUNK_STATUS_LIST = 0x1843,		//请求干线状态列表返回

	//VSTORE相关
	NE_REQ_VSTORE_CAMERA_STATUS = 0x1860,		//请求摄像机存储状态
	NE_RET_VSTORE_CAMERA_STATUS = 0x1861,		//请求摄像机存储状态返回
	NE_REQ_VSTORE_REDUCE_STATUS = 0x1862,		//请求摄像机裁剪状态
	NE_RET_VSTORE_REDUCE_STATUS = 0x1863,		//请求摄像机裁剪状态返回
	NE_REQ_VSTORE_VOD_STATUS = 0x1864,			//请求摄像机点播状态
	NE_RET_VSTORE_VOD_STATUS = 0x1865,			//请求摄像机点播状态返回
	NE_REQ_VSTORE_DOWNLOAD_STATUS = 0x1866,		//请求录像下载状态
	NE_RET_VSTORE_DOWNLOAD_STATUS = 0x1867,		//请求录像下载状态返回

	NE_REQ_VSTORE_TEMPERATORE_STATUS = 0x1868,	//请求机箱温度状态
	NE_RET_VSTORE_TEMPERATORE_STATUS = 0x1869,	//请求机箱温度状态

	NE_NOTE_VSTORE_DEVICE_ALARM = 0x186a,		//通知设备告警状态信息

};

typedef eNeMsgType eMsgType;

#pragma pack(1)

#define NE_PROTOCOL_SYNCHEADER "$head$"

//消息头包头
typedef struct __sNeMsgHead
{
	char 			m_szSyncHeader[6];	//"$head$"
	char 			m_szVersion[4];		//0x01010102
	unsigned int	m_uiMsgType;		//见包类型表格
	UINT64			m_ubiWorkflowId;	//会话编号，唯一
	unsigned int	uiPacketBodyLen;	//包体长度(不含包头)
	char 			m_Reserved[8];		//保留字
}sNeMsgHead,sMsgHead;


//普通返回消息包体
typedef struct __sNeRetMsgBase
{
	int iRetValue;							//0代表成功,>0代表错误
	char sDescription[50];					//成功、错误描述
	char sReserved[16];						//保留字节
}sNeRetMsgBase;

//服务器类型,业务程序类型
enum eNeSvrMode
{
	NE_SVR_MODE_STORAGE = 0x01,			//存储服务器
	NE_SVR_MODE_VOD = 0x02,				//点播服务器
	NE_SVR_MODE_TRANSMIT = 0x04,		//转发服务器
	NE_SVR_MODE_DOWNLOAD = 0x08,		//下载服务器
	NE_SVR_MODE_MANAGER = 0x80,			//管理服务器

	NE_SVR_MODE_VSTOR =	0x10,			//VStore视频存储设备服务器
	NE_SVR_MODE_IDE = 0x20,				//Intelligent Detect Element智能检测设备服务器

};

//业务程序重启原因类型
enum eNeSvrRestartReason
{
	NE_RESTART_CONFIGURE_UPDATE = 0x01,	//服务器配置更新
	NE_RESTART_PROGRAM_UPDATE = 0x02,	//服务器更新程序
	NE_RESTART_RUN_ERROR = 0x03,		//服务器运行错误
	NE_RESTART_DEVICE_TROUBLE = 0x04,	//服务器设备故障
	NE_RESTART_UNKNOWN_REASON = 0x05,	//服务器未知异常

};

//业务程序状态类型
enum eNeSvrProgramStatus
{
	NE_PROGRAM_START = 0x01,		//服务器启动
	NE_PROGRAM_RESTART = 0x02,		//服务器重启
	NE_PROGRAM_RUN = 0x03,			//服务器运行中

};

/************************************************************************/
/* 公共协议部分                                                                    */
/************************************************************************/

//2.2.2.3	业务程序启动和需要重启时触发通知
typedef struct __sNeNoteProgramStatus
{
	char	szServerName[32];		        //服务器名称
	eNeSvrMode	uiBusinessType;		        //业务程序类型
	char	szProgramName[32];		        //业务程序名称
	eNeSvrProgramStatus	uiProgramStatus;	//业务程序状态:启动，重启
	eNeSvrRestartReason	uiRestartReason;	//业务程序重启原因类型
	bool	bSelfRestart;					//是否为自我启动 1是 0否
	UINT32  uiRunTime;				        //业务程序已经运行的时间,若程序刚启动则返回0
	UINT32  uiTimeStamp;				    //当前系统时间，即告警触发时间
	char	szReserverd[16];		        //预留字节

}sNeNoteProgramStatus;

/************************************************************************/
/* IDE协议部分                                                                    */
/************************************************************************/

//摄像机状态查询
typedef struct __sNeIdeReqCameraStatus
{
	UINT32	szCameraId;			//摄像机编号为请求所有摄像机
	char	szReserved[16];		//预留字节
}sNeIdeReqCameraStatus;

//安装类型
enum eNeInstallType
{
	NE_CAMERA_INSTALL_FIXED = 1,		//固定安装，不可动
	NE_CAMERA_INSTALL_ACTIVITY = 2,		//云台安装，可活动
};
enum eNeInstallDirect
{
	NE_CAMERA_DIRECT_EAST = 1,		//安装方向，东
	NE_CAMERA_DIRECT_WEST = 2,		//安装方向，西
	NE_CAMERA_DIRECT_SOUTH = 3,		//安装方向，南
	NE_CAMERA_DIRECT_NORTH = 4,		//安装方向，北
};
//摄像机状态置位或
enum eNeCameraStatus
{
	NE_CAMERASTATUS_PTZ = 0x01,		//摄像机PTZ
	NE_CAMERASTATUS_LIVE = 0x02,	//摄像机视频直播
	NE_CAMERASTATUS_RECORD = 0x03,	//摄像机录像
	NE_CAMERASTATUS_VOD = 0x04,		//摄像机视频回放
	NE_CAMERASTATUS_LOCKED = 0x10,	//摄像机锁定
	NE_CAMERASTATUS_SCAN = 0x20,	//摄像机巡检

};
//摄像机状态结构
typedef struct __sNeIdeCameraStatus
{
	UINT32	uiCameraId;			//摄像机编号
	char	szCameraName[32];	//摄像机名称，可用“摄像机+编号”，唯一即可
	UINT32 uiSideId;			//所属断面编号
	char szInstallPlace[32];	//所属安装地点
	eNeInstallType uiInstallType;		//安装类型
	eNeInstallDirect uiInstallDirect;	//安装方向
	UINT32	uiEnable;			//摄像机是否可用
	UINT32	uiCtrlable;			//摄像机是否可控
	UINT32	uiStatus;	//摄像机当前状态,状态置位表示eCameraStatus
	char	szUserName[50];		//正在操作的用户名称
	char	szReserved[16];		//预留字节
}sNeIdeCameraStatus;

//摄像机状态查询返回
typedef struct __sNeIdeRetCameraStatus
{
	sNeRetMsgBase sRetBase;
	unsigned int uiCount;
	//后面跟sNeIdeCameraStatus结构体组
}sNeIdeRetCameraStatus;


//告警严重级别  4(最高)-1(最低)
enum eNeAlarmLevel
{
	NE_ALARMLEVEL_WARNING = 0x01,		//警告(warning)
	NE_ALERMLEVEL_MINOR = 0x02,			//次要(minor)
	NE_ALERMLEVEL_MAJOR = 0x03,			//严重(major)
	NE_ALERMLEVEL_CRITICAL = 0x04,		//危急(critical)
};

//告警种类
enum eNeAlarmCategory
{
	NE_ALARMCATEGORY_COMMUNICATE = 0x01,	//通信
	NE_ALARMCATEGORY_QUALITY = 0x02,		//服务质量
	NE_ALARMCATEGORY_DEVICE = 0x03,			//设备
	NE_ALARMCATEGORY_ERROR = 0x04,			//处理差错
	NE_ALARMCATEGORY_ENVIRONMENT = 0x05,	//环境
};

//具体告警类型	给定的值暂时定在-900 范围内
enum eNeAlarmType
{
	NE_ALARMTYPE_TYPE_UNKNOWN = 850,		//未知告警

	//IDE
	NE_ALARMTYPE_CAMERA_VIDEOLOST = 851,	//摄像机视频丢失时告警
	NE_ALARMTYPE_CAMERA_POSITIONMOVE = 852, //摄像机位置移动告警
	//VIS
	//VSTORE
	NE_ALARMTYPE_DISK_INSERT = 880,			//硬盘插入时触发事件
	NE_ALARMTYPE_DISK_PULLOUT = 881,		//硬盘拔出时触发告警
	NE_ALARMTYPE_CAMERA_STREAM_TOOLOW = 882,//每路摄像机录像码率过低报警，阈值可设，缺省Kbps

};

//设备结构
typedef struct __sNeIdeDevice
{
	UINT32	uiDeviceId;			//设备编号/摄像机编号
	char	szDeviceName[32];	//设备名称/摄像机名称
	UINT32	uiSideId;			//所属断面编号
	char	szInstallPlace[32];	//所属安装地点
	char	szReserverd[16];	//预留字节

}sNeIdeDevice;

//2.2.2.5	通知设备告警状态信息
typedef struct __sNeIdeNoteDeviceAlarm
{
	sNeIdeDevice   sDevice;
	eNeAlarmLevel	uiAlarmLevel;			//告警严重级别eNeAlarmLevel
	eNeAlarmCategory	uiAlarmCategory;	//告警种类eNeAlarmCategory
	eNeAlarmType	uiAlarmType;			//告警类型eNeAlarmType
	UINT32  uiTimeStamp;					//当前系统时间，即告警触发时间
	char	szDescription[50];				//报警描述信息
	char	szReserved[16];					//预留字节
}sNeIdeNoteDeviceAlarm;

/************************************************************************/
/* VIS协议部分                                                                    */
/************************************************************************/


/************************************************************************/
/* VSTORE协议部分                                                                    */
/************************************************************************/

/************************************************************************/
/* 其他                                                                    */
/************************************************************************/

enum eNeErrorCode
{
	//通信模块
	ERR_COM_NO_OPSVR = 3003001,					//OP服务端未启动
	ERR_COM_NO_VISLSVR,							//VisSvr常连接服务端未初始化
	ERR_COM_NO_VISSSVR,							//VisSvr短连接服务端未初始化
	ERR_COM_NO_SUBVISSSVR,						//VisSvr sub服务器未初始化
	ERR_COM_NO_VISLCLIENT,						//VisSvr常连接客户端未初始化
	ERR_COM_NO_VISSCLIENT,						//VisSvr短连接客户端未初始化
	ERR_COM_NO_VISSUBCLIENT,					//VisSvr sub客户端未初始化
	ERR_COM_CONNECT_VISLCLIENT = 4003006,		//VisSvr常连接客户端连接失败
	ERR_COM_CONNECT_VISSCLIENT,					//VisSvr短连接客户端连接失败
	ERR_COM_CLOSE_VISLCLIENT,					//VisSvr常连接客户端关闭失败
	ERR_COM_CLOSE_VISSCLIENT,					//VisSvr短连接客户端关闭失败
	ERR_COM_NOT_FIND_CLIENT,					//找不到对应客户端
	ERR_COM_NOT_FIND_SESSION,					//找不到OP对应Session,可能WorkFlowId出错
	ERR_COM_DEL_WORKFLOWID,						//删除WorkFlowId出错失败，找不到WorkFlowId
	ERR_COM_SEND,								//发送数据失败
	ERR_COM_CLIENT_RECVHEAD,					//短连接等待返回接收头失败
	ERR_COM_CLIENT_RECVTYPE,					//短连接等待返回接收消息类型失败
	ERR_COM_CLIENT_RECVBODY,					//短连接等待返回接收体失败
	ERR_COM_CLIENT_SHORT_INIT,                  //短连接信号量初始化失败

	//级联模块
	ERR_CSM_GET_OUTPUTSTATUS = 3010001,			//获取监视器关系表出错
	ERR_CSM_GET_SERVERLIST,						//获取Server视图出错
	ERR_CSM_CANT_FIND_DESTSERVER,				//找不到目标服务器
	ERR_CSM_GET_LOCALSERVER,					//找不到本地服务器
	ERR_CSM_FIND_PATH = 4010005,				//寻找路径出错
	ERR_CSM_SEND_GLOBAL_SWITCH,					//发送全局切换失败
	ERR_CSM_GLOBAL_SWITCH_RET,					//全局切换返回失败
	ERR_CSM_SEND_SWITCH,						//发送局部切换失败
	ERR_CSM_SWITCH_RET,							//局部切换返回失败
	ERR_CSM_RELEASE,							//释放干线失败
};


#pragma pack()

#endif
