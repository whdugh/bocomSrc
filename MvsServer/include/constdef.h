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

#ifndef SKP_ROAD_CONSTDEF_H
#define SKP_ROAD_CONSTDEF_H

#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef UINT16
typedef unsigned short UINT16;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#define IJL_DIB_ALIGN (sizeof(int) - 1)

#define IJL_DIB_UWIDTH(width,nchannels) \
  ((width) * (nchannels))

#define IJL_DIB_AWIDTH(width,nchannels) \
  ( ((IJL_DIB_UWIDTH(width,nchannels) + IJL_DIB_ALIGN) & (~IJL_DIB_ALIGN)) )

#define IJL_DIB_PAD_BYTES(width,nchannels) \
  ( IJL_DIB_AWIDTH(width,nchannels) - IJL_DIB_UWIDTH(width,nchannels) )

#define IJL_DIB_SCALE_SIZE(jpgsize,scale) \
  ( ((jpgsize) + (scale) - 1) / (scale) )

/******************************************以下是常量定义*****************************/

//版本号
#define VERSION						0001

//服务端口
#define SRIP_ROAD_PORT				8080
//最大请求处理数
#define MAX_PROCESS_SIZE			1
//最大HOST长度
#define SKP_MAX_HOST				16
//缓冲区大小
#define SRIP_MAX_BUFFER				1024

//日志记录最大长度
#define SRIP_MAX_LOG				1024

//心跳检测阀值
#define SRIP_LINK_MAX				2

//用户名最大长度
#define MIMAX_USERNAME			32
//密码最大长度
#define MIMAX_USERPASS			32

//查询每页显示数
#define SRIP_PAGE_SIZE				100

//通道地点长度
#define SRIP_LOCATION_MAXLEN		256

//设备名称长度
#define SRIP_DEVICE_MAXLEN			256

//报警级别
#define  ALERT_LEVEL     5

//最大特征维数
#define DIM_TEXTURE         400
#define DIM_FEATURE			326

//信号灯维数
#define TRAFFIC_SIGNAL_DIM  3

//缓冲队列大小
#define  FRAMESIZE       5
#define  CARNUMSIZE    30

//一天时间所经过的秒数
#define  DATE_TIME     86400

#define MAX_TEXTURES		4000 //最大匹配的特征数-add by ywx

#define DSP_500_BIG_WIDTH   2448
#define DSP_500_BIG_HEIGHT  2048

#define DSP_500_SMALL_WIDTH 408
#define DSP_500_SMALL_HEIGHT 342

#define DSP_500_BIG_WIDTH_TWO 2592
#define DSP_500_BIG_HEIGHT_TWO 1936

#define DSP_500_SMALL_WIDTH_TWO 432
#define DSP_500_SMALL_HEIGHT_TWO 324

#define DSP_200_BIG_WIDTH 1616
#define DSP_200_BIG_HEIGHT 1232

#define DSP_200_SMALL_WIDTH 404
#define DSP_200_SMALL_HEIGHT 308

#define DSP_200_BIG_WIDTH_WIDE 1920
#define DSP_200_BIG_HEIGHT_WIDE 1088

#define DSP_200_SMALL_WIDTH_WIDE 384
#define DSP_200_SMALL_HEIGHT_WIDE 217

#define DSP_400_BIG_WIDTH   2352
#define DSP_400_BIG_HEIGHT  1760

#define DSP_400_SMALL_WIDTH 392
#define DSP_400_SMALL_HEIGHT 352

#define DSP_200_BIG_WIDTH_SERVER 1600
#define DSP_200_BIG_HEIGHT_SERVER 1200

#define DSP_500_BIG_WIDTH_SERVER   2448
#define DSP_500_BIG_HEIGHT_SERVER  2048

#define DSP_200_BIG_WIDTH_TWO 1920
#define DSP_200_BIG_HEIGHT_TWO 1080

#define DSP_200_SMALL_WIDTH_TWO 480
#define DSP_200_SMALL_HEIGHT_TWO 270

#define DSP_200_BIG_DH_WIDTH 1600
#define DSP_200_BIG_DH_HEIGHT 1200

#define DSP_500_BIG_DH_WIDTH 2592
#define DSP_500_BIG_DH_HEIGHT 2048

#define DSP_200_BIG_DH_213_WIDTH 1920
#define DSP_200_BIG_DH_213_HEIGHT 1080

#define DSP_200_BIG_C203K_WIDTH 1600
#define DSP_200_BIG_C203K_HEIGHT 1200

#define DSP_300_BIG_WIDTH 1920
#define DSP_300_BIG_HEIGHT 1440

#define DSP_600_BIG_465_WIDTH 2752
#define DSP_600_BIG_465_HEIGHT 2208

#define ROSEEK_DSP_TCP_PORT 25000
#define ROSEEK_DSP_TCP_CONTROL_PORT 35000
#define ROSEEK_DSP_TCP_H264_PORT 61000
#define MVS_SERVER_TCP_PORT 65000 //检测器作为服务器时的侦听端口号

#define MAX_IP_LEN			16
#define MAX_URL_LEN			256
#define MAX_RTSP_USER_LEN	24
#define MAX_RTSP_PASSWD_LEN 24

#ifndef MAX_DSP_BUF_LEN
	#define MAX_DSP_BUF_LEN 2048000
#endif

#define MAX_DEVICE_CODE 20 //设备编号最大长度

//普通的采集图片
#define SRIP_NORMAL_PIC					0x10000003
//统计检测结果
#define	SRIP_DETECT_STATISTIC			0x10000004
//事件检测结果
#define SRIP_DETECT_EVENT				0x10000005
//车牌检测类型
#define SRIP_CARNUM				        0x10000006
//车牌检测结果
#define SRIP_CARD_RESULT		        0x10000007
//纯线圈信号
#define SRIP_LOOP_SIGNAL		        0x10000008
//违章录像
#define SRIP_VIOLATION_VIDEO            0x10000009
//事件车牌录像
#define SRIP_EVENT_PLATE_VIDEO          0x1000000A
//停车严管矩形目标状态
#define SRIP_OBJECT_STATUS		        0x1000000B

/******************************************以下是与客户端的命令定义*****************************/

//命令成功
#define SRIP_OK					0x00000000
//发送失败
#define SRIP_ERROR_SEND			0x00000001
//等待超时
#define SRIP_ERROR_TIMEOUT		0x00000002
//数据格式错误
#define SRIP_ERROR_FORMAT		0x00000003
//用户名错误
#define SRIP_LOGIN_USER_ERROR	0x00000004
//密码错误
#define SRIP_LOGIN_PASS_ERROR	0x00000005
//用户已经存在
#define SRIP_ERROR_USER_EXIST	0x00000006
//密码为空
#define SRIP_ERROR_PASSW_EMPTY	0x00000007
//用户操作失败
#define SRIP_ERROR_USER_FAILE	0x00000008
//通道存在
#define SRIP_ERROR_CHAN_EXIST	0x00000009
//不能修改admin用户信息
#define SRIP_ERROR_ADMIN_MODY	0x00000010
//不能删除admin用户信息
#define SRIP_ERROR_ADMIN_DEL	0x00000011
//无操作权限
#define SRIP_ERROR_NO_PRIV		0x00000012
//读取3G设置信息失败
#define READ_3G_INFO			0x00000013

//未完成
#define SRIP_ERROR_UNFINISH     0x00000060
//不存在
#define SRIP_ERROR_NOEXIST      0x00000061

//认证成功
#define AUTHENTICATION_OK		0x00000100
//认证失败
#define AUTHENTICATION_ERROR	0x00000101

//时间戳
#define SRIP_TIMESTAMP			0x40000001
//登录
#define SRIP_LOGIN				0x40000002
//登录返回
#define SRIP_LOGIN_REP			0x80000002

//登出
#define SRIP_LOGOUT				0x40000003
//登出返回
#define SRIP_LOGOUT_REP			0x80000003

//心跳
#define SRIP_LINK				0x40000004
//心跳返回
#define SRIP_LINK_REP			0x80000004

//添加用户
#define SRIP_USER_ADD			0x40000005
//添加用户返回
#define SRIP_USER_ADD_REP		0x80000005
//删除用户
#define SRIP_USER_DEL			0x40000006
//删除用户返回
#define SRIP_USER_DEL_REP		0x80000006
//修改用户
#define SRIP_USER_MOD			0x40000007
//修改用户返回
#define SRIP_USER_MOD_REP		0x80000007
//用户列表
#define SRIP_USER_LIST			0x40000008
//用户列表返回
#define SRIP_USER_LIST_REP		0x80000008

//捕捉帧结果
/*************************************/
/*1.事件							 */
/*2.报警							 */
/*3.车牌							 */
/*4.系统							 */
/*************************************/
#define SRIP_FRAME_RESULT		0x40000010


//通道列表
#define SRIP_CHANNEL_LIST				0x40000011
//通道列表返回
#define SRIP_CHANNEL_LIST_REP           0x80000011
//添加通道
#define SRIP_CHANNEL_ADD				0x40000012
//添加通道返回
#define SRIP_CHANNEL_ADD_REP			0x80000012
//删除通道
#define SRIP_CHANNEL_DEL				0x40000013
//删除通道返回
#define SRIP_CHANNEL_DEL_REP			0x80000013
//修改通道
#define SRIP_CHANNEL_MOD				0x40000014
//修改通道返回
#define SRIP_CHANNEL_MOD_REP			0x80000014
//事件查询
#define SRIP_SEARCH_EVENT				0x40000015
//事件查询返回
#define SRIP_SEARCH_EVENT_REP			0x80000015
//报警查询
#define SRIP_SEARCH_ALARM				0x40000016
//报警查询返回
#define	SRIP_SEARCH_ALARM_REP			0x80000016
//车牌查询
#define SRIP_SEARCH_CARD				0x40000017
//车牌识别返回
#define SRIP_SEARCH_CARD_REP			0x80000017
//录像查询
#define SRIP_SEARCH_RECORD				0x40000018
//录像查询返回
#define SRIP_SEARCH_RECORD_REP			0x80000018
//日志查询
#define SRIP_SEARCH_LOG					0x40000019
//日志查询返回
#define SRIP_SEARCH_LOG_REP				0x80000019
//设置通道视频参数
#define SRIP_CHANNEL_PARA				0x40000020
//设置通道视频参数返回
#define SRIP_CHANNEL_PARA_REP			0x80000020
//设置通道视频参数
#define SRIP_CHANNEL_PARA_ADJUST		0x40000021
//设置通道视频参数返回
#define SRIP_CHANNEL_PARA_ADJUST_REP	0x80000021
//保存通道视频参数
#define SRIP_CHANNEL_PARA_SAVE			0x40000022
//保存通道视频参数返回
#define SRIP_CHANNEL_PARA_SAVE_REP		0x80000022
//车道保存
#define SRIP_ROAD_SAVE					0x40000023
//车道保存返回
#define SRIP_ROAD_SAVE_REP				0x80000023
//清空车道
#define SRIP_ROAD_DEL					0x40000024
//清空车道返回
#define SRIP_ROAD_DEL_REP				0x80000024
//车道数据
#define SRIP_ROAD_WAY					0x40000025
//车道数据返回
#define SRIP_ROAD_WAY_REP				0x80000025
//系统配置
#define SRIP_SYS_SETTING				0x40000026
//系统配置返回
#define SRIP_SYS_SETTING_REP			0x80000026
//备份数据
#define SRIP_BACKUP_DATABASE			0x40000027
//备份数据返回
#define SRIP_BACKUP_DATABASE_REP		0x80000027
//权限错误
#define SRIP_ERROR_PRIV					0x40000028
//系统信息
#define SRIP_SYSTEMINFOR				0x40000029
//系统信息返回
#define SRIP_SYSTEMINFOR_REP			0x80000029
//图表信息
#define SRIP_CHARTQUERY					0x40000030
//图表信息返回
#define SRIP_CHARTQUERY_REP				0x80000030
//车牌图片
#define SRIP_CARD_PIC					0x40000031
//车牌图片返回
#define SRIP_CARD_PIC_REP				0x80000031

//删除事件
#define SRIP_DELETE_EVENT				0x40000032
//删除事件返回
#define SRIP_DELETE_EVENT_REP			0x80000032
//删除统计
#define SRIP_DELETE_ALARM				0x40000033
//删除统计返回
#define SRIP_DELETE_ALARM_REP			0x80000033
//删除录象
#define SRIP_DELETE_RECORD				0x40000034
//删除录象返回
#define SRIP_DELETE_RECORD_REP			0x80000034
//删除车牌
#define SRIP_DELETE_CARD				0x40000035
//删除车牌返回
#define SRIP_DELETE_CARD_REP			0x80000035
//删除日志
#define SRIP_DELETE_LOG					0x40000036
//删除日志返回
#define SRIP_DELETE_LOG_REP				0x80000036

//激活通道
#define SRIP_CHANNEL_RESUME				0x40000037
//激活通道返回
#define SRIP_CHANNEL_RESUME_REP			0x80000037

//暂停通道
#define SRIP_CHANNEL_PAUSE				0x40000038
//暂停通道返回
#define SRIP_CHANNEL_PAUSE_REP			0x80000038

//暂停通道
#define SRIP_CHANNEL_STATE				0x40000039
//暂停通道返回
#define SRIP_CHANNEL_STATE_REP			0x80000039

//模拟相机控制
#define SRIP_CAMERA_CONTROL			0x40000040
//模拟相机控制返回
#define SRIP_CAMERA_CONTROL_REP		0x80000040

//车道编号列表
#define SRIP_ROADINDEX_LIST				0x40000048

//通道状态返回
#define SRIP_CHANNEL_STATUS				0x40000049
//大图
#define SRIP_CHANNEL_BIG				0x40000051
//大图返回
#define SRIP_CHANNEL_BIG_REP			0x80000051

//小图
#define SRIP_CHANNEL_SMALL				0x40000052
//小图返回
#define SRIP_CHANNEL_SMALL_REP			0x80000052

//添加黑白名单车牌
#define SRIP_ADD_SPECIALCARD     0x40000053
//添加黑白名单车牌返回
#define SRIP_ADD_SPECIALCARD_REP   0x80000053

//删除车牌黑白名单
#define SRIP_DELETE_SPECIALCARD				0x40000054
//删除车牌黑白名单返回
#define SRIP_DELETE_SPECIALCARD_REP			0x80000054

//修改车牌黑白名单

#define SRIP_MODIFY_SPECIALCARD				0x40000055
//修改车牌黑白名单返回
#define SRIP_MODIFY_SPECIALCARD_REP			0x80000055

//查询车牌黑白名单

#define SRIP_SEARCH_SPECIALCARD				0x40000056
//查询车牌黑白名单返回
#define SRIP_SEARCH_SPECIALCARD_REP			0x80000056

//版本号
#define SRIP_VERSION			0x40000057
//版本号返回
#define SRIP_VERSION_REP			0x80000057

//保存车牌识别参数
#define SRIP_CARNUMPARAM_SAVE     0x40000058
//保存车牌识别参数返回
#define SRIP_CARNUMPARAM_SAVE_REP   0x80000058

//获取车牌识别参数
#define SRIP_CARNUMPARAM     0x40000059
//获取车牌识别参数返回
#define SRIP_CARNUMPARAM_REP   0x80000059

//相机控制
#define SRIP_CAMERA_SETUP     0x40000060
//相机控制返回
#define SRIP_CAMERA_SETUP_REP   0x80000060

//修改车牌
#define MODIFY_CARNUM     0x40000061
//修改车牌返回
#define MODIFY_CARNUM_REP   0x80000061

//车牌统计查询
#define CARNUM_STATISTIC     0x40000062
//车牌统计查询返回
#define CARNUM_STATISTIC_REP   0x80000062

//获取录象状态
#define VIDEO_STATE     0x40000063
//车牌统计查询返回
#define VIDEO_STATE_REP   0x80000063

//获取车道参数
#define ROAD_PARAMETER     0x40000064
//车道参数返回
#define ROAD_PARAMETER_REP   0x80000064

//保存车道参数
#define ROAD_PARAMETER_SAVE     0x40000065
//保存车道参数返回
#define ROAD_PARAMETER_SAVE_REP   0x80000065

//保存车道模板参数
#define ROAD_MODEL_SAVE     0x40000066
//保存车道模板参数返回
#define ROAD_MODEL_SAVE_REP   0x80000066

//连接通道
#define MIMAX_CHANNEL_CONNECT			0x40000067
//断开通道
#define MIMAX_CHANNEL_DISCONNECT		0x80000067

//软件更新
#define UPDATE_SERVER			0x40000068
//软件更新返回
#define UPDATE_SERVER_REP		0x80000068

//相机同步
#define CAMERA_SYNCH			0x40000069
//相机同步返回
#define CAMERA_SYNCH_REP		0x80000069

//车牌高级查询
#define SRIP_SEARCH_CARD_HIGH				0x40000070
//车牌识别高级查询返回
#define SRIP_SEARCH_CARD_HIGH_REP			0x80000070

//特征搜索高级查询获取特征信息
#define SRIP_SEARCH_TEXTURE_HIGH        0x40000071
//特征搜索高级查询获取特征信息返回
#define SRIP_SEARCH_TEXTURE_HIGH_REP    0x80000071

//车牌黑白名单报警
#define SRIP_SPECIAL_CARD			0x40000072
//车牌黑白名单报警返回
#define SRIP_SPECIAL_CARD_REP		0x80000072

//传送裁剪区域及对焦区域图像
#define SRIP_REGION_IMAGE			            0x40000073
//传送裁剪区域及对焦区域图像返回
#define SRIP_REGION_IMAGE_REP			0x80000073

//获取闯红灯检测参数
#define VTS_PARAMETER     0x40000074
//闯红灯检测参数返回
#define VTS_PARAMETER_REP   0x80000074

//保存闯红灯检测参数
#define VTS_PARAMETER_SAVE     0x40000075
//保存闯红灯检测参数返回
#define VTS_PARAMETER_SAVE_REP   0x80000075

//添加相机模板
#define ADD_CAMERA_MODEL     0x40000076
//添加相机模板返回
#define ADD_CAMERA_MODEL_REP   0x80000076

//修改相机模板
#define MODIFY_CAMERA_MODEL     0x40000077
//修改相机模板返回
#define MODIFY_CAMERA_MODEL_REP   0x80000077

//载入相机模板
#define LOAD_CAMERA_MODEL     0x40000078
//载入相机模板返回
#define LOAD_CAMERA_MODEL_REP   0x80000078

//设置检测器IP
#define SET_IP     0x40000079
//设置检测器IP返回
#define SET_IP_REP   0x80000079

//认证
#define AUTHENTICATION			0x40000080
//认证返回
#define AUTHENTICATION_REP		0x80000080

//删除测试程序
#define DELETE_TEST			0x40000081
//删除测试程序返回
#define DELETE_TEST_REP		0x80000081

//时钟同步
#define SYSCLOCK			0x40000082
//时钟同步返回
#define SYSCLOCK_REP		0x80000082

//串口设置
#define COM_SET			0x40000083
//串口设置返回
#define COM_SET_REP		0x80000083

//认证服务器设置
#define AUTHENTICATION_SET			0x40000084
//认证服务器设置返回
#define AUTHENTICATION_SET_REP		0x80000084

//获取车道设置模板
#define ROAD_SETTING_MODEL			0x40000085
//获取车道设置模板返回
#define ROAD_SETTING_MODEL_REP		0x80000085

//获取线圈检测参数
#define LOOP_PARAMETER			0x40000086
//获取线圈检测参数返回
#define LOOP_PARAMETER_REP		0x80000086

//保存线圈检测参数
#define LOOP_PARAMETER_SAVE			0x40000087
//保存线圈检测参数返回
#define LOOP_PARAMETER_SAVE_REP		0x80000087

//中心控制服务器设置
#define CONTROL_SERVER_SET			0x40000088
//中心控制服务器设置返回
#define CONTROL_SERVER_SET_REP		0x80000088

//检测器ID设置
#define DETECTOR_ID_SET			0x40000089
//检测器ID设置返回
#define DETECTOR_ID_SET_REP		0x80000089

//检测器复位
#define DETECTOR_RESET			0x40000090
//检测器复位返回
#define DETECTOR_RESET_REP		0x80000090

//检测器时间设置
#define SYSTIME_SET			0x40000091
//检测器时间设置返回
#define SYSTIME_SET_REP		0x80000091

//程序复位
#define SOFTWARE_RESET		0x40000092
//程序复位返回
#define SOFTWARE_RESET_REP		0x80000092

//特征比对查询
#define SRIP_SEARCH_OBJECT_FEATURE 0x40000093
//特征比对查询
#define SRIP_SEARCH_OBJECT_FEATURE_REP 0x80000093

//ftp服务器设置
#define FTP_SERVER_SET			0x40000094
//ftp服务器设置返回
#define FTP_SERVER_SET_REP		0x80000094

//获取系统硬件配置信息
#define SYSTEM_HARDWARE_INFO	 0x40000095
//获取系统硬件配置信息返回
#define SYSTEM_HARDWARE_INFO_REP 0x80000095

//强制红灯
#define FORCE_RED_LIGHT	 0x40000096
//强制红灯返回
#define FORCE_RED_LIGHT_REP 0x80000096

//串口参数
#define SYS_COM_SETTING	 0x40000097
//串口参数返回
#define SYS_COM_SETTING_REP 0x80000097

//球机及云台控制参数
#define SYS_YUNTAI_SETTING  0x40000098
//球机及云台控制参数返回
#define SYS_YUNTAI_SETTING_REP 0x80000098

//设置区域
#define REGION_SET	 0x40000099
//设置区域返回
#define REGION_SET_REP 0x80000099

//获取区域
#define REGION_GET	 0x4000009A
//获取区域返回
#define REGION_GET_REP 0x8000009A

//监控主机参数
#define SYS_MONITORHOST_SETTING  0x4000009B
//监控主机参数返回
#define SYS_MONITORHOST_SETTING_REP 0x8000009B

//智能控制器参数
#define EXPO_MONITOR_SETTING  0x4000009C
//智能控制器参数返回
#define EXPO_MONITOR_SETTING_REP 0x8000009C

//系统参数模板设置
#define SYS_SETTING_MODEL  0x4000009D
//系统参数模板设置返回
#define SYS_SETTING_MODEL_REP 0x8000009D

//图片格式信息设置
#define PIC_FORMAT_SETTING  0x4000009E
//图片格式信息设置返回
#define PIC_FORMAT_SETTING_REP 0x8000009E

//检测器关机
#define DETECTOR_SHUTDOWN		0x4000009F
//检测器关机返回
#define DETECTOR_SHUTDOWN_REP	0x8000009F

//雷达检测参数
#define RADAR_PARAMETER_SET			0x400000A0
//雷达检测参数返回
#define RADAR_PARAMETER_SET_REP		0x800000A0

//车牌检测参数
#define PLATE_PARAMETER_SET			0x400000A1
//车牌检测参数返回
#define PLATE_PARAMETER_SET_REP		0x800000A1

//目标检测参数
#define OBJECT_PARAMETER_SET		0x400000A2
//目标检测参数返回
#define OBJECT_PARAMETER_SET_REP	0x800000A2

//违章检测参数
#define VIOLATION_PARAMETER_SET		0x400000A3
//违章检测参数返回
#define VIOLATION_PARAMETER_SET_REP	0x800000A3

//行为分析检测参数
#define BEHAVIOR_PARAMETER_SET		0x400000A4
//行为分析检测参数返回
#define BEHAVIOR_PARAMETER_SET_REP	0x800000A4


//录像格式信息设置
#define VIDEO_FORMAT_SETTING  0x400000A5
//录像格式信息设置返回
#define VIDEO_FORMAT_SETTING_REP 0x800000A5
//获取调整视频参数
#define SRIP_CHANNEL_PARA_GET 0x400000A6
#define SRIP_CHANNEL_PARA_GET_REP 0x800000A6

//增加预置位
#define ADD_PRESET_INFO       0x400000A7
//增加预置位返回
#define ADD_PRESET_INFO_REP   0x800000A7

//修改预置位
#define MOD_PRESET_INFO       0x400000A8
//修改预置位返回
#define MOD_PRESET_INFO_REP   0x800000A8

//删除预置位
#define DEL_PRESET_INFO       0x400000A9
//删除预置位返回
#define DEL_PRESET_INFO_REP   0x800000A9

//获取预置位信息
#define PRESET_INFO       0x400000AA
//获取预置位返回
#define PRESET_INFO_REP   0x800000AA

//开关灯控制信息
#define LIGHT_TIME_CONTROL       0x400000AB
//开关灯控制信息返回
#define LIGHT_TIME_CONTROL_REP   0x800000AB

//比对服务器设置
#define MATCH_HOST_SET       0x400000AC
//比对服务器设置返回
#define MATCH_HOST_SET_REP   0x800000AC

//配置信息手动上传
#define SETTING_UPLOAD       0x400000AD
//配置信息手动上传返回
#define SETTING_UPLOAD_REP   0x800000AD

//应用管理服务器设置
#define AMS_HOST_SET       0x400000AE
//应用管理服务器设置返回
#define AMS_HOST_SET_REP   0x800000AE

//数据库修复
#define DB_REPAIR       0x400000AF
//数据库修复返回
#define DB_REPAIR_REP   0x800000AF

//数据库恢复
#define DB_DEFAULT       0x400000B0
//数据库恢复返回
#define DB_DEFAULT_REP   0x800000B0

//区间测速主机设置
#define DISTANCE_HOST_SET       0x400000B1
//区间测速主机设置返回
#define DISTANCE_HOST_SET_REP   0x800000B1

//GPS设置
#define GPS_SET       0x400000B2
//GPS设置返回
#define GPS_SET_REP   0x800000B2

//通道图片格式设置
#define CHANNEL_PICFORMAT_SET       0x400000B3
//通道图片格式设置返回
#define CHANNEL_PICFORMAT_SET_REP   0x800000B3

//停车严管框选区域
#define DETECT_REGION_RECT       0x400000B4
//停车严管框选区域返回
#define DETECT_REGION_RECT_REP   0x800000B4


//信号机设置
#define SIGNAL_SET       0x400000B5
//GPS设置返回
#define SIGNAL_SET_REP   0x800000B5

//获取Dsp相机列表
#define SRIP_GET_DSP_LIST 0x400000B6 
//获取Dsp相机列表返回
#define SRIP_GET_DSP_LIST_REP 0x800000B6

//添加车牌
#define ADD_CARNUM     0x400000B7
//添加车牌返回
#define ADD_CARNUM_REP   0x800000B7

//停车严管增加一个目标区域
#define DETECT_ADD_REGION_RECT       0x400000B8
//停车严管增加一个目标区域返回
#define DETECT_ADD_REGION_RECT_REP   0x800000B8

//停车严管删除一个目标区域
#define DETECT_DEL_REGION_RECT       0x400000B9
//停车严管删除一个目标区域返回
#define DETECT_DEL_REGION_RECT_REP   0x800000B9


/******************************************以下是与中心端的命令定义*****************************/
//最大序号
#define MAX_SEQ				4294967295
//日志内容长度
#define MAX_LOG				64
//最大车道数
#define MAX_ROADWAY			16
//事件描述长度
#define MAX_EVENT			16
//车牌文本长度
#define MAX_PLATE			16
//地点描述长度
#define MAX_PLACE			32
//检测方向描述长度
#define MAX_DIRECTION		16
//录象路径长度
#define MAX_VIDEO			128
//排除在外的车牌省份缩写或非警车字符串
#define MAX_EXCEPT_TEXT		128

//中心端端口
#define PLATE_PORT			 41010
//客户端端口
#define EVENT_PORT			 41020
//同步端口
#define SYNCH_PORT	         41030
//认证端口
#define AUTHEN_PORT	         41040
//录像下载端口
#define VIDEO_PORT	         41050
////////////////////////////
//车牌心跳信号
#define PLATE_LINK     0x0001FFFF
//事件心跳信号
#define EVENT_LINK     0x0003FFFF



//获取日志记录状态
#define PLATE_LOG_STATUS				0x00010001
#define EVENT_LOG_STATUS				0x00030001

//获取车牌记录状态
#define PLATE_STATUS				0x00010002
//获取统计记录状态
#define STATISTIC_STATUS			0x00030002
//获取事件记录状态
#define EVENT_STATUS				0x00030003


//获取指定时间的日志记录
#define PLATE_LOG						0x00010003
#define EVENT_LOG						0x00030004

//获取指定时间的车牌记录
#define MIMAX_PLATE						0x00010004
//获取指定时间的统计记录
#define MIMAX_STATISTIC					0x00030005
//获取指定时间的事件记录
#define MIMAX_EVENT						0x00030006

//获取指定序号的日志记录
#define PLATE_SEQ_LOG					0x00010005
#define EVENT_SEQ_LOG					0x00030007

//获取指定序号的车牌记录
#define MIMAX_SEQ_PLATE					0x00010006
//获取指定序号的统计记录
#define MIMAX_SEQ_STATISTIC				0x00030008
//获取指定序号的事件记录
#define MIMAX_SEQ_EVENT					0x00030009

//根据序号删除日志记录
#define PLATE_DELSEQ_LOG				0x00010007
#define EVENT_DELSEQ_LOG				0x0003000A

//根据序号删除车牌记录
#define MIMAX_DELSEQ_PLATE				0x00010008
//根据序号删除统计记录
#define MIMAX_DELSEQ_STATISTIC			0x0003000B
//根据序号删除事件记录
#define MIMAX_DELSEQ_EVENT				0x0003000C

//根据时间删除日志记录
#define PLATE_DEL_LOG					0x00010009
#define EVENT_DEL_LOG					0x0003000D

//根据时间删除车牌记录
#define MIMAX_DELTIME_PLATE				0x0001000A
//根据时间删除统计记录
#define MIMAX_DELTIME_STATISTIC			0x0003000E
//根据时间删除事件记录
#define MIMAX_DELTIME_EVENT				0x0003000F


//获取检测器当前时间
#define PLATE_SYSTIME					0x0001000B
#define EVENT_SYSTIME					0x00030010

//设置检测器当前时间
#define PLATE_SYSTIME_SETUP			    0x0001000C
#define EVENT_SYSTIME_SETUP			    0x00030011

//获取检测器当前配置
#define PLATE_CONFIG					0x0001000D
#define EVENT_CONFIG					0x00030012

//设置检测器当前配置
#define PLATE_CONFIG_SETUP				0x0001000E
#define EVENT_CONFIG_SETUP				0x00030013



//推送实时日志记录
#define PLATE_REALTIME_LOG				0x0001000F
//推送实时日志记录确认
#define PLATE_REALTIME_LOG_REP			0x0002000F
//推送实时事件日志记录
#define EVENT_REALTIME_LOG				0x00030014
//推送实时事件日志记录确认
#define EVENT_REALTIME_LOG_REP			0x00040014


//推送实时车牌记录
#define MIMAX_REALTIME_PLATE			0x00010010
//推送实时车牌记录确认
#define MIMAX_REALTIME_PLATE_REP		0x00020010
//推送实时统计记录
#define MIMAX_REALTIME_STATISTIC		0x00030015
//推送实时统计记录确认
#define MIMAX_REALTIME_STATISTIC_REP	0x00040015
//推送实时事件记录
#define MIMAX_REALTIME_EVENT			0x00030016
//推送实时事件记录确认
#define MIMAX_REALTIME_EVENT_REP		0x00040016


//不推送实时日志记录
#define PLATE_NOREALTIME_LOG			0x00010011
#define EVENT_NOREALTIME_LOG			0x00030017

//不推送实时车牌记录
#define MIMAX_NOREALTIME_PLATE			0x00010012
//不推送实时统计记录
#define MIMAX_NOREALTIME_STATISTIC		0x00030018
//不推送实时事件记录
#define MIMAX_NOREALTIME_EVENT			0x00030019

//获取车牌检测区域
#define MIMAX_PLATE_REGION				0x00010013


//获取非实时日志记录
#define PLATE_NONE_LOG					0x00010014
#define EVENT_NONE_LOG					0x0003001A

//获取非实时车牌记录
#define MIMAX_NONE_PLATE				0x00010015

//登录
#define PLATE_LOGIN						0x00010100
//登录返回
#define PLATE_LOGIN_REP					0x00020100


//获取非实时统计记录
#define MIMAX_NONE_STATISTIC			0x0003001B
//获取非实时事件记录
#define MIMAX_NONE_EVENT				0x0003001C


//推送视频
#define MIMAX_SEND_FRAME				0x0003001D
//不推送视频
#define MIMAX_NOSEND_FRAME				0x0003001E

//////////////////////////////////////////////////////////////////
//发送日志记录状态
#define PLATE_LOG_STATUS_REP			0x00020001
#define EVENT_LOG_STATUS_REP			0x00040001
//发送车牌记录状态
#define PLATE_STATUS_REP			0x00020002
//发送统计记录状态
#define STATISTIC_STATUS_REP		0x00040002
//发送事件记录状态
#define EVENT_STATUS_REP			0x00040003

//发送日志记录
#define PLATE_LOG_REP					0x00020003
#define EVENT_LOG_REP					0x00040004

//发送车牌记录
#define MIMAX_PLATE_REP					0x00020004
//发送统计记录
#define MIMAX_STATISTIC_REP				0x00040005
//发送事件记录
#define MIMAX_EVENT_REP					0x00040006


//发送检测器当前时间
#define PLATE_SYSTIME_REP				0x0002000B
#define EVENT_SYSTIME_REP				0x00040010

//发送检测器当前配置
#define PLATE_CONFIG_REP				0x0002000D
#define EVENT_CONFIG_REP				0x00040012

//发送车牌检测区域
#define MIMAX_PLATE_REGION_REP			0x00020013

//图片信息
#define MIMAX_FRAME						0x0004001D

//登录
#define EVENT_LOGIN						0x00030100
//登录返回
#define EVENT_LOGIN_REP					0x00040100

//获取图片信息
#define CAPTURE_PIC_INFO       0x00030101
//获取图片信息返回
#define CAPTURE_PIC_INFO_REP   0x00040101

//设置停车检测区域
#define SET_STOPREGION_INFO       0x00030102
//设置停车检测区域返回
#define SET_STOPREGION_INFO_REP   0x00040102

//设置预置位
#define SET_PRESET_INFO       0x00030103
//设置预置位返回
#define SET_PRESET_INFO_REP   0x00040103

//相机切换编号
#define SWITCH_CAMERA_ID       0x00030105
//相机切换编号返回
#define SWITCH_CAMERA_ID_REP   0x00040105

//预置位模式切换
#define SET_PRESET_MODE       0x00030107
//预置位模式切换返回
#define SET_PRESET_MODE_REP   0x00040107

//检测配置获取请求(由检测器发往AMS)
#define DETECT_SETTING_REQUEST_INFO  0x60001000
//检测配置响应(由AMS发往检测器)
#define DETECT_SETTING_RESPONSE_INFO  0x70001000

//通道检测设置(由AMS发往检测器)
#define DETECT_REQUEST_INFO  0x60001001
//通道检测设置响应(由检测器发往AMS)
#define DETECT_RESPONSE_INFO  0x70001001
//系统设置(由AMS发往检测器)
#define SYSTEM_REQUEST_INFO  0x60001002
//系统设置响应(由检测器发往AMS)
#define SYSTEM_RESPONSE_INFO  0x70001002

//强制报警命令
#define SRIP_ADD_FORCE_ALERT  0x00050001

// 3G配置命令
#define SET_3G_MODE			0x00060001
#define SET_3G_MODE_REP		0x00070001
#define READ_3G_MODE_REQ	0x00060002
#define READ_3G_MODE_REP	0x00070002

// Dsp相机配置
#define SET_DSP_CAMERA_REQ	0x00070003
#define SET_DSP_CAMERA_REP	0x00060003
#define GET_DSP_CAMERA_REQ	0x00070004
#define GET_DSP_CAMERA_REP	0x00060004

// FTP路径设置
#define SET_FTP_PATH_REQ		0x00060005
#define SET_FTP_PATH_REP		0x00070005
#define GET_FTP_PATH_REQ		0x00060006
#define GET_FTP_PATH_REP		0x00070006

// NTP对时点设置
#define SET_NTP_TIME_REQ		0x00060007
#define SET_NTP_TIME_REP		0x00070007

#define GET_NTP_TIME_REQ		0x00060008
#define GET_NTP_TIME_REP		0x00070008

#define SET_CHECK_TIME_REQ		0x00060009
#define SET_CHECK_TIME_REP		0x00070009

#define GET_CHECK_TIME_REQ		0x0006000a
#define GET_CHECK_TIME_REP		0x0007000a

#define SET_SERVER_KAFKA_REQ	0x0006000b
#define SET_SERVER_KAFKA_REP	0x0007000b

#define GET_SERVER_KAFKA_REQ	0x0006000c
#define GET_SERVER_KAFKA_REP	0x0007000c

//尾号限行设置
#define SET_PLATE_LIMIT_REQ 0x0006000d 
#define SET_PLATE_LIMIT_REP 0x0007000d 
//尾号限行设置回复
#define GET_PLATE_LIMIT_REQ 0x0006000e 
#define GET_PLATE_LIMIT_REP 0x0007000e

//录像下载
#define GET_VIDEO_REQ 0x0006000f 
#define GET_VIDEO_REP 0x0007000f

//路段起始经纬度设置(C-S)
#define SETTING_ROADITUDE		0x400000BC
//路段起始经纬度设置(S-C)
#define SETTING_ROADITUDE_REP   0x800000BC
//路段起始经纬度回调(C-S)
#define GET_ROADITUDE			0x400000BD
//路段起始经纬度回调(S-C)
#define GET_ROADITUDE_REP		0x800000BD

//录像手动上传
#define UPLOAD_VIDEO       0x400000BE
//录像手动上传返回
#define UPLOAD_VIDEO_REP   0x800000BE

#endif
