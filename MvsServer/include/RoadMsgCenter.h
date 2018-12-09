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

#ifndef SKP_ROAD_MSGCENTER_H
#define SKP_ROAD_MSGCENTER_H


#include "global.h"
#include "ManageClient.h"
/******************************************************************************/
//	描述:智能交通检测系统命令处理模块。
/******************************************************************************/

//处理列表
//typedef std::list<std::string> ListMsg;
//映射列表
//typedef std::multimap<int,ListMsg::iterator> ListMap;
typedef std::multimap<int, std::string> MsgListMap;

class CSkpRoadMsgCenter
{
public:
	//构造
	CSkpRoadMsgCenter();
	//析构
	~CSkpRoadMsgCenter();

public:
	//初始化
	bool Init();
	//释放
	bool UnInit();
public:
	//接收到一条命令
	bool AddMsg(const int nSocket,const unsigned int uCommand,const std::string request);
	//客户端断开，清除对应的命令
	bool DelMsg(int nSocket = 0);
	//处理一条命令
	bool ProcessMsg();

	//发送系统信息
	bool SendSysMsg();

	void SetManageClient(CManageClient *p)
	{
		m_pManageClient = p;
	}
private:
	//启动命令处理线程
	bool BeginProcessThread();
	//停止处理线程
	void EndProcessThread();

	//弹出消息
	bool PopMsg(int& nSocket,std::string& response);
	//消息处理
	bool OnMsg(int nSocket,std::string response);


	//登录处理
	bool OnLogin(const int nSocket,const unsigned int uCommand,std::string request);
	//心跳处理
	bool OnLink(const int nSocket,std::string request);

	//添加用户
	bool OnUserAdd(const int nSocket,std::string request);
	//删除用户
	bool OnUserDel(const int nSocket,std::string request);
	//修改用户
	bool OnUserModify(const int nSocket,std::string request);
	//用户列表
	bool OnUserList(const int nSocket,std::string request);

	//添加通道
	bool OnChannelAdd(const int nSocket,std::string request);
	//删除通道
	bool OnChannelDel(const int nSocket,std::string request);
	//修改通道
	bool OnChannelModify(const int nSocket,std::string request);
	//暂停通道
	bool OnChannelPause(const int nSocket,std::string request);
	//激活通道
	bool OnChannelResume(const int nSocket,std::string request);
	//连接断开通道
	bool OnSetChannelConnect(const int nSocket,std::string request);

	//事件查询
	bool OnSearchEvent(const int nSocket,std::string request);
	//统计查询
	bool OnSearchStatistic(const int nSocket,std::string request);
	//日志查询
	bool OnSearchLog(const int nSocket,std::string request);
	//车牌查询
	bool OnSearchPlate(const int nSocket,std::string request);


	//录像查询
	bool OnSearchRecord(const int nSocket,std::string request);

	//调节通道视频参数
	bool OnAdjustChannelPara(const int nSocket,std::string request);
	//保存通道视频参数
	bool OnSaveChannelPara(const int nSocket,std::string request);

	//保存车道信息
	bool OnRoadSave(const int nSocket,std::string request);
	//清空车道
    bool OnRoadDel(const int nSocket,std::string request);
	//获得车道数据
	bool OnRoadWay(const int nSocket,std::string request);
	//获取车道设置模板
	bool OnRoadWayModel(const int nSocket,std::string request);
	//获取车道参数设置
	bool OnRoadWayParaMeter(const int nSocket,std::string request);
	//保存车道参数
	bool OnRoadParaMeterSave(const int nSocket,std::string request);
	//保存车道模板参数
	bool OnRoadModelSave(const int nSocket,std::string request);

	//系统设置
	bool OnSysSetting(const int nSocket,std::string request);
	//备份数据库
	bool OnBackupDB(const int nSocket,std::string request);

	//图表查询
	bool OnChartQuery(const int nSocket,std::string request);
	//车牌图片查询
	bool OnCardPic(const int nSocket,std::string request);
	//删除各种查询结果
	bool OnDeleteResult(const int nSocket,std::string request);

	//车牌布控管理
	bool OnAddSpecialCard(const int nSocket,std::string request);
	bool OnDeleteSpecialCard(const int nSocket,std::string request);
	bool OnModifySpecialCard(const int nSocket,std::string request);
	bool OnSearchSpecialCard(const int nSocket,std::string request);

	//车牌检测参数设置
	bool OnCardNumParam(const int nSocket,std::string request);
	//修改车牌检测参数
	bool OnSaveCardNumParam(const int nSocket,std::string request);
	//修改车牌检测结果
	bool OnModifyCarNum(const int nSocket,std::string request);

    //获取闯红灯检测参数
	bool OnVTSParam(const int nSocket,std::string request);
   //保存闯红灯检测参数
	bool OnSaveVTSParam(const int nSocket,std::string request);
	//获取线圈检测参数
	bool OnLoopParam(const int nSocket,std::string request);
   //保存线圈检测参数
	bool OnSaveLoopParam(const int nSocket,std::string request);

	//雷达参数设置
    bool OnRadarParameterSet(const int nSocket,std::string request);

    //目标检测参数设置
    bool OnObjectParameterSet(const int nSocket,std::string request);

    //行为分析参数设置
    bool OnBehaviorParameterSet(const int nSocket,std::string request);

	//相机控制
	bool OnCameraSetup(const int nSocket,std::string request);
	//确定事件录象是否完成
	bool OnVideoState(const int nSocket,std::string request);
	//截取一帧图象
	bool OnCaptureOneFrame(const int nSocket,std::string request);
	//获取及设置系统时间
	bool OnSysTime(const int nSocket,std::string request);
	////////////////////////

	//车牌高级查询
	bool OnSearchPlateHigh(const int nSocket,std::string request);

	//特征搜索高级查询
	bool OnSearchTextureHigh(const int nSocket, std::string request);

	//获取通道列表
	bool OnGetChannelList(const int nSocket, std::string request);

	//发送显示区域图像命令
	bool OnRegionImage(const int nSocket, std::string request);

    //相机模板
	bool OnCameraModel(const int nSocket, std::string request);

    //设置ip地址
	bool OnIpSetup(const int nSocket, std::string request);

	//删除测试程序
    bool OnDeleteTest(const int nSocket, std::string request);

    //时钟同步设置
    bool OnSysClockSetup(const int nSocket, std::string request);

    //串口设置
    bool OnSysComSetting(const int nSocket, std::string request);
    //球机及云台控制参数设置
    bool OnSysYunTaiSetting(const int nSocket, std::string request);
    //监控主机参数设置
    bool OnSysMonitorHostSetting(const int nSocket, std::string request);
    //智能控制器主机参数设置
    bool OnExpoMonitorSetting(const int nSocket, std::string request);
    //系统参数模板设置
    bool OnSysModelSetting(const int nSocket, std::string request);
    //图片格式信息
    bool OnPicFormatSetting(const int nSocket, std::string request);
    //录像格式信息
    bool OnVideoFormatSetting(const int nSocket, std::string request);
    //获取版本号码
    bool OnVerSion(const int nSocket, std::string request);

    //认证服务器设置
    bool OnAuthenticationSetup(const int nSocket, std::string request);
    //中心控制服务器设置
    bool OnControlServerSetup(const int nSocket, std::string request);
    //检测器ID设置
    bool OnDetectorIDSetup(const int nSocket, std::string request);
    //检测器复位
    bool OnDetectorReset(const int nSocket, std::string request);
    //检测器关机
    bool OnDetectorShutDown(const int nSocket, std::string request);
    //软件复位
    bool OnSoftWareReset(const int nSocket, std::string request);
    //ftp服务器设置
    bool OnFtpServerSetup(const int nSocket, std::string request);
    //系统硬件配置信息
    bool OnSysHwInfo(const int nSocket, std::string request);
    //强制红灯
    bool OnForceRedLight(const int nSocket, std::string request);
    //特征比对查询
    bool OnSearchObjectFeature(const int nSocket, std::string request);

    //获取以及设置区域
    bool OnRegionInfo(const int nSocket, std::string request);

    //通道参数获取
    bool OnGetAdjustPara(const int nSocket,std::string request);
    //获取预置位信息
    bool OnPreSetInfo(const int nSocket,std::string request);
    //增加和更新预置位信息
    bool OnUpdatePreSetInfo(const int nSocket,std::string request);
    //删除预置位
    bool OnDeletePreSetInfo(const int nSocket,std::string request);

    //开关灯控制
    bool OnLightTimeControl(const int nSocket,std::string request);

    //比对服务器设置
    bool OnMatchHostSetup(const int nSocket, std::string request);

	//应用管理服务器设置
	bool OnAmsHostSetup(const int nSocket, std::string request);
	//区间测速主机设置
	bool OnDistanceHostSetup(const int nSocket, std::string request);
	//gps设置
	bool OnGpsSetup(const int nSocket, std::string request);
	//通道图片格式设置
	bool OnPicFormatSetup(const int nSocket, std::string request);

	//配置信息手动上传
	bool OnSettingUpload(const int nSocket, std::string request);
	//录像手动上传
	bool OnVideoUpload(const int nSocket, string request);

	//数据库修复
	bool OnDBRepair(const int nSocket, std::string request);

	//数据库恢复
	bool OnDBDefault(const int nSocket, std::string request);
	
	//强制报警
	bool OnForceAlert(const int nSocket, std::string request);

	//信号机设置
	bool OnSignalSetup(const int nSocket, std::string request);

	//停车严管区域命令
	bool OnDetectRegionRectImage(const int nSocket, std::string request);
	//停车严管目标区域操作
	bool OnDetectParkObjectsRect(const int nSocket,UINT32 uMsgCommandID, std::string request);

	//获取相机列表命令
	bool OnGetDspList(const int nSocket, std::string request);
	
	// 配置3G信息
	bool OnSet3GMode(const int nSocket, string request);
	// 获取配置3G信息
	bool OnRead3GMode(const int nSocket, string request);
	// 获取配置3G信息
	bool OnRead3GModeByOpen3G(const int nSocket, string request);
	bool OnRead3GModeByOpenUserInfo(const int nSocket, string request);
	// 获取DSP相机配置信息
	bool OnGetDspCamera(const int nSocket, string request);
	// 设置DSP相机配置信息
	bool OnSetDspCamera(const int nSocket, string request);
	// 设置Ftp路径
	bool Set_Ftp_Path(const int nSocket, string request);
	// 获取Ftp路径
	bool Get_Ftp_Path(const int nSocket, string request);
	bool SetSysClockForSHJJ();
	bool Set_Ntp_Time(const int nSocket, string request);
	bool Get_Ntp_Time(const int nSocket, string request);
	bool SetCheckTime(const int nSocket, string request);
	bool GetCheckTime(const int nSocket, string request);
	bool SetKafka(const int nSocket, string request);
	bool GetKafka(const int nSocket, string request);
	bool SetPlateLimit(const int nSocket, string request);
	bool GetPlateLimit(const int nSocket, string request);
	//设置获取路段信息(路段,经纬度)
	bool SetRoadInfo(const int nSocket, string request);
	bool GetRoadInfo(const int nSocket, string request);

private:
	//可能存在客户端断开后，队列中仍然存在命令的情况，采用列表加上映射的方式，记录列表指针。
	//接收的命令队列
	//ListMsg m_MsgList;
	//接收的命令队列映射列表
	//ListMap m_MapList;
	MsgListMap m_mapMsgList;
	//信号互斥
	pthread_mutex_t m_list_mutex;

	//线程ID
	pthread_t m_nThreadId;
	//返回设置参数用
//	SRIP_CHANNEL_ATTR *sAttr;

	CManageClient *m_pManageClient;

};
//全局调用
extern CSkpRoadMsgCenter g_skpRoadMsgCenter;
#endif
