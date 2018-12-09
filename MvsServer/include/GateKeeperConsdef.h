// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
/*
//验证请求
#define LS_AUTH_CODE 0xefff0001
//验证成功
#define LS_AUTH_OK 0xefff0002
//验证失败
#define LS_AUTH_ERROR 0xefff0003
//车牌记录
#define LS_PLATE_INFO 0xefff0004
//接收记录成功
#define LS_PLATE_OK 0xefff0005
//接收记录失败
#define LS_PLATE_ERROR 0xefff0006
//心跳信号
#define LS_LINK_TEST 0xefff0007
*/

//心跳
#define GATE_LINK_TEST 0xefff0001

//获取机箱门状态
#define GATE_GET_DOOR_STATUS 0xefff0002
//获取机箱门状态回复
#define GATE_GET_DOOR_STATUS_REP 0xefff1002

//获取机箱温度
#define GATE_GET_TEMPERATURE 0xefff0004
//获取机箱温度回复
#define GATE_GET_TEMPERATURE_REP 0xefff1004

//机箱震动报警
#define GATE_SHAKE_STATUS 0xefff0006
//机箱震动报警回复
#define GATE_SHAKE_STATUS_REP 0xefff1006

//对主动上传正确数据回复
#define GATE_WO_OK 0xefff0010
//对主动上传错误数据回复
#define GATE_WO_ER 0xefff0011

//信息头
typedef struct _GATEKEEPER_HEADER
{
	BYTE chCmd;		//命令字
	BYTE nSpknHigh;  //请求数据报号
	BYTE nSpknLow;  //
	BYTE nRpknHigh; //响应数据报号
	BYTE nRpknLow; //
	BYTE nDataLenthHigh; //说明SDU数据的长度（不包括header的长度）
	BYTE nDataLenthLow; //
	BYTE chPid; //保留位

	_GATEKEEPER_HEADER()
	{
	    chCmd = 0;
		nSpknHigh= 0;
		nSpknLow = 0;
		nRpknHigh = 0;
		nRpknLow = 0;
		nDataLenthHigh = 0;
		nDataLenthLow = 0;
		chPid = 0;
	}
}GATEKEEPER_HEADER;
