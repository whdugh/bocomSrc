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

#ifndef SKP_ROAD_SERIALCOMM_H
#define SKP_ROAD_SERIALCOMM_H


#include "global.h"

/******************************************************************************/
//	描述:智能交通检测系统串口通信类
//	作者:於锋
//	日期:2008-10-28
/******************************************************************************/
	//打开设备
	int OpenDev(int port);

	//关闭设备
	int Close();

	uint8 WriteMessage(uint8 TX);

	//打开串口
	int open_port(int port);

	//设置串口波特率
	void set_speed(speed_t speed);
	//设置奇偶校验位
	int set_Parity(int databits,int stopbits,int parity);

#endif
