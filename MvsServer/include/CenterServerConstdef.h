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
#ifndef CENTER_SERVER_CONSTDEF_H
#define CENTER_SERVER_CONSTDEF_H

//开始工作
#define START_WORK "101"
#define START_WORK_REP "601"

//结束工作
#define END_WORK "102"
#define END_WORK_REP "602"

//实时记录
#define REALTIME_RECORD "103"
#define REALTIME_RECORD_REP "603"

//历史记录
#define HISTORY_RECORD "104"
#define HISTORY_RECORD_REP "604"

//设备校时(use ntp instead)
#define SYSTIME_SETUP "105"
#define SYSTIME_SETUP_REP "605"

//识别程序重启
#define DETECT_RESTART "106"
#define DETECT_RESTART_REP "606"

//设备工作状态
#define DEVICE_STATUS "107"
#define DEVICE_STATUS_REP "607"

//设备报警
#define DEVICE_ALARM "108"
#define DEVICE_ALARM_REP "608"

//未上传数据查询
#define RECORD_QUERY "109"
#define RECORD_QUERY_REP "609"

//重新标识为未上传记录，强制重传
#define MARK_UNSEND "110"
#define MARK_UNSEND_REP "610"

//心跳
#define LINK_TEST "111"
#define LINK_TEST_REP "611"

//黑名单下载成功反馈
#define HMD_QUERY "114"
#define HMD_QUERY_REP "614"

#endif
