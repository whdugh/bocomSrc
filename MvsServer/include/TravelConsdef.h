// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef TRAVEL_CONSTDEF_H
#define TRAVEL_CONSTDEF_H

//心跳
#define TRA_LINK_TEST 0xA0

//实时过车文本数据记录上传
#define TRA_PLATE_INFO 0xA2

//点播放业务数据
#define TRA_ORDER_OPRATION 0xA3

//询问
#define TRA_ASK 0xA4

//历史视频分析时间查询
#define TRA_ORDER_VIDEOTIME 0xA5

//获取实时视频流
#define TRA_ORDER_REALTIME_VIDEO 0xA6

//业务控制参数
#define TRA_OPRATION_CONTROL 0xA7

//设备认证
#define TRA_DEVICE_SUPPLY 0xA8

//////////////////////////////////////////////
typedef struct _TRAVEL_CONTROL_INFO
{
    bool m_bIsUploadRealTime; //是否上传实时文本数据
    int m_nUploadOplog; //上传设备日志周期
    int m_nUploadDevState; //上传设备状态日志周期
    int m_nSampleTime; //流量检测周期
    int m_nVideo; //历史视频存储时间
    int m_nTraficLine; //交通拥堵，队列长度
    int m_nCarStopTime; //车辆停驶时间阀值
    int m_nCarSpeedSlow; //车辆慢行速度

    _TRAVEL_CONTROL_INFO()
    {
        m_bIsUploadRealTime = false;
        m_nUploadOplog = 0;
        m_nUploadDevState = 0;
        m_nSampleTime = 0;
        m_nVideo = 0;
        m_nTraficLine = 0;
        m_nCarStopTime = 0;
        m_nCarSpeedSlow = 0;
    }
}TRAVEL_CONTROL_INFO;


#endif
