// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _CMDTOCAMERA_H_
#define _CMDTOCAMERA_H_

#include "Common.h"

enum RUN_MODE
{
	ROSEEK_TrgMode,
	ROSEEK_FconMode
};

static const int CMD_DATA_BUF_SIZE = 100*1024;

static const int CMD_DATA_BIG_BUF_SIZE = 3*1024*1024;

enum EE_METHOD_TYPE
{
	EE_METHOD_UNKNOWN,
	EE_METHOD_GET,
	EE_METHOD_SET,
	EE_METHOD_EXECUTE
};

typedef struct _DspCmdHeader
{
    BYTE aCmdID;
    BYTE aReplyFlag;
    BYTE aParamLen[4];

    _DspCmdHeader()
    {
        aCmdID = 0x00;
        aReplyFlag = 0x00;
        memset(aParamLen, 0, 4);
    }
} DspCmdHeader;

#define EE_EXPOSURE							0x00010000 //曝光
#define EE_EXPOSURE_GAIN					0x00010001 //增益
#define EE_EXPOSURE_SHUTTERTIME				0x00010002 //快门
#define EE_EXPOSURE_GAINLIMIT				0x00010003 //增益上限
#define EE_EXPOSURE_SHUTTERTIMEUPPERLIMIT	0x00010004 //快门上限
#define EE_EXPOSURE_MEANBRIGHTNESSTHRESHOLD	0x00010005 //平均亮度期望值
#define EE_EXPOSURE_MODE					0x00010006 //自动或手动
#define EE_EXPOSURE_FLASHLAMPENABLE			0x00010007 //触发闪光灯

#define EE_EXPOSURE_TRG								0x00010100 //
#define EE_EXPOSURE_TRG_GAIN						0x00010101 //
#define EE_EXPOSURE_TRG_SHUTTERTIME					0x00010102 //
#define EE_EXPOSURE_TRG_GAINLIMIT					0x00010103 //
#define EE_EXPOSURE_TRG_SHUTTERTIMEUPPERLIMIT		0x00010104 //
#define EE_EXPOSURE_TRG_MEANBRIGHTNESSTHRESHOLD		0x00010105 //
#define EE_EXPOSURE_TRG_MODE						0x00010106 //
#define EE_EXPOSURE_TRG_FLASHLAMPENABLE				0x00010107 //

#define EE_EXPOSURE_FCON							0x00010200 //
#define EE_EXPOSURE_FCON_GAIN						0x00010201 //
#define EE_EXPOSURE_FCON_SHUTTERTIME				0x00010202 //
#define EE_EXPOSURE_FCON_GAINLIMIT					0x00010203 //
#define EE_EXPOSURE_FCON_SHUTTERTIMEUPPERLIMIT		0x00010204 //
#define EE_EXPOSURE_FCON_MEANBRIGHTNESSTHRESHOLD	0x00010205 //
#define EE_EXPOSURE_FCON_MODE						0x00010206 //
#define EE_EXPOSURE_FCON_FLASHLAMPENABLE			0x00010207 //

#define EE_EXPOSURE_HFR								0x00010300 //
#define EE_EXPOSURE_HFR_GAIN						0x00010301 //
#define EE_EXPOSURE_HFR_SHUTTERTIME					0x00010302 //
#define EE_EXPOSURE_HFR_GAINLIMIT					0x00010303 //
#define EE_EXPOSURE_HFR_SHUTTERTIMEUPPERLIMIT		0x00010304 //
#define EE_EXPOSURE_HFR_MEANBRIGHTNESSTHRESHOLD		0x00010305 //
#define EE_EXPOSURE_HFR_MODE						0x00010306 //
#define EE_EXPOSURE_HFR_FLASHLAMPENABLE				0x00010307 //

#define EE_EXPOSURE_HFRBIN							0x00010400 //
#define EE_EXPOSURE_HFRBIN_GAIN						0x00010401 //
#define EE_EXPOSURE_HFRBIN_SHUTTERTIME				0x00010402 //
#define EE_EXPOSURE_HFRBIN_GAINLIMIT				0x00010403 //
#define EE_EXPOSURE_HFRBIN_SHUTTERTIMEUPPERLIMIT	0x00010404 //
#define EE_EXPOSURE_HFRBIN_MEANBRIGHTNESSTHRESHOLD	0x00010405 //
#define EE_EXPOSURE_HFRBIN_MODE						0x00010406 //
#define EE_EXPOSURE_HFRBIN_FLASHLAMPENABLE			0x00010407 //

#define EE_APPEARANCE						0x00020000 //图像
#define EE_APPEARANCE_WHITEBALANCEENABLE	0x00020001 //使能白平衡
#define EE_APPEARANCE_WHITEBALANCEMODE		0x00020002 //白平衡光照条件
#define EE_APPEARANCE_IMGSAMPLEFMT			0x00020003 //图像采集格式
#define EE_APPEARANCE_LUTENABLE				0x00020004 //是否打开LUT转换
#define EE_APPEARANCE_LUTTABLE				0x00020005 //LUT表

#define EE_METERING							0x00030000 //测光
#define EE_METERING_AUTOMR					0x00030001 //使能自动测光
#define EE_METERING_AUTOMRPERIOD			0x00030002 //自测光周期
#define EE_METERING_MRBOTTOM				0x00030003 //测光坐标底(bottom)
#define EE_METERING_MRLEFT					0x00030004 //测光坐标左(left)
#define EE_METERING_MRRIGHT					0x00030005 //(right)
#define EE_METERING_MRTOP					0x00030006 //(top)

#define EE_GRABINFO							0x00040000 //抓拍信息
#define EE_GRABINFO_REMARK					0x00040001 //注释
#define EE_GRABINFO_LANENUM					0x00040002 //车道
#define EE_GRABINFO_TRGTYPE					0x00040003 //触发方式
#define EE_GRABINFO_COILPITCH				0x00040004 //线圈间距
#define EE_GRABINFO_SWITCHAUTOGRAB			0x00040005 //转换自动抓拍

#define EE_NETWORK							0x00050000 //网络
#define EE_NETWORK_DEVIP					0x00050001 //IP地址
#define EE_NETWORK_DEVMAC					0x00050002 //MAC地址

#define EE_SYSTEM							0x00060000 //系统
#define EE_SYSTEM_DATETIME					0x00060001 //日期时间
#define EE_SYSTEM_WATCHDOGENABLE			0x00060002 //使能看门狗
#define EE_SYSTEM_WATCHDOGTIMER				0x00060003 //看门狗定时
#define EE_SYSTEM_TEMPERATURE				0x00060004 //温度
#define EE_SYSTEM_ENCRYPTCHIPINFO			0x00060005 //加密芯片信息
#define EE_SYSTEM_SERIAL					0x00060006 //

#define EE_STREAM1							0x00070000 //视频流
#define EE_STREAM1_RUNMODE					0x00070001 //工作模式（值为0 表示触发模式，值为1 表示全分辩率连续模式，值为2 表示高帧率模式，值为3 表示高灵敏度高帧率模式）

#define EE_EXECUTE							0x00080000 //相机调用
#define EE_EXECUTE_REBOOT					0x00080001 //重启
#define EE_EXECUTE_GRAB						0x00080002 //抓拍
#define EE_EXECUTE_GRABFORADJUST			0x00080003 //为调整参数的抓拍
#define EE_EXECUTE_RESETGRABSVR				0x00080004 //重置抓拍图片发送服务
#define EE_EXECUTE_SAVECONFIG				0x00080005 //保存配置
#define EE_EXECUTE_RESTOREDEFAULT			0x00080006 //恢复出厂配置

#define EE_IO								0x00090000 //IO口
#define EE_IO_OPTION						0X00090001 //IO口配置

#define EE_ISO								0x000a0000 //光耦口
#define EE_ISO_INMODE						0x000a0001 //光偶输入模式
#define EE_ISO_OUTMODE						0x000a0002 //光偶输出模式
#define EE_ISO_POWERSYNENABLE				0x000a0003 //电源同步使能
#define EE_ISO_POWERSYNACMODE				0x000a0004 //电源同步模式
#define EE_ISO_POWERSYNDELAYTIME			0x000a0005 //电源同步延时
#define EE_ISO_FLASHLAMPENABLE				0x000a0006 //闪光灯使能
#define EE_ISO_FLASHLAMPMODE				0x000a0007 //闪光灯模式
#define EE_ISO_FLASHLAMPOUTWIDTH			0x000a0008 //闪光灯输出宽度

///////////////////////////DSP相机///////////////////////////////////////////////
#define EE_EXECUTE_REBOOT_DSP					    0x00 //重启
#define EE_STREAM1_RUNMODE_DSP					    0x01 //工作模式（值为0 表示触发模式，值为1 表示全分辩率连续模式，值为2 表示高帧率模式，值为3 表示高灵敏度高帧率模式）
//0x02 //上位机软触发抓取一帧图像#define EE_HFR_MOD_GET_ONE_FRAME
//0x03	读取相机的控制状态

#define EE_EXPOSURE_SHUTTERTIME_DSP					0x05  //设置全分辨率连续模式下的曝光时间
#define EE_EXPOSURE_GAIN_DSP						0x07 //设置传感器信号增益
//0x08 设置白平衡光照条件
//0x09	是否自动白平衡
//0x0a	设置是否使能图像LUT映射变换
//0x0b	重置图像LUT映射表
#define EE_EXPOSURE_FLASHLAMPENABLE_DSP				0x0c //设置闪光灯同步输出使能
#define EE_EXPOSURE_MODE_DSP						0x0f //自动或手动

#define EE_EXPOSURE_SHUTTERTIMEUPPERLIMIT_DSP		0x10 //设置全分辨率连续模式下的相机进行亮度自动调节的曝光时间上限
#define EE_EXPOSURE_GAINLIMIT_DSP					0x13 //设置相机进行亮度自动调节的信号增益上限
#define EE_EXPOSURE_MEANBRIGHTNESSTHRESHOLD_DSP		0x14 //设置全分辨率连续模式下相机进行亮度自动调节的图像亮度阈值范围
/////////////////////////////////////////////////////////////////////////////////

#ifndef _SOCKET_ROSEEK
    typedef int SOCKET;
    #define SOCKET_ERROR            (-1)
    #define INVALID_SOCKET  (SOCKET)(~0)
#endif

#ifndef _BOOL_ROSEEK
    #define BOOL bool
#endif

class CCMDToCamera
{
public:
	CCMDToCamera();
	virtual ~CCMDToCamera();

public:
	void	SetTargetAddr(const char *pszIP, unsigned short wPort);
	BOOL	SendCMD(unsigned int dwAttribute, unsigned int dwMethod);
	BOOL	SendCMD(unsigned int dwAttribute, unsigned int dwMethod, unsigned int dwParam);
	BOOL	SendCMD(unsigned int dwAttribute, unsigned int dwMethod, const char *pParam);
	BOOL	SendCMD(unsigned int dwAttribute, unsigned int dwMethod, const char *pParam, unsigned int dwParamLen);

    //DSP相机控制
	BOOL    SendDspCMD(const DspCmdHeader &header, const char * pParam);

	char* GetBackDataPtr();
	unsigned int	GetBackDataDWORD();
	float	GetBackDataFloat();
	void	Close();

	//DSP相机控制2
	BOOL  SendDspCMD2(const DspCmdHeader &header, const char * pParam);

	//DSP相机控制3
	BOOL  SendDspCMD3(const DspCmdHeader &header, const char * pParam);

	char* GetBackDataBigPtr()
	{		
		return m_pBackDataBig;
	}

protected:
	BOOL	CreateAndConnect();
	SOCKET		m_sockThis;
	char		m_pBackData[CMD_DATA_BUF_SIZE+1];
	char		m_pSendData[CMD_DATA_BUF_SIZE+1];
	std::string	m_sTargetIP;
	unsigned short		m_wTargetPort;

	char *m_pBackDataBig;

	pthread_mutex_t m_sockMutex;
};

#endif



