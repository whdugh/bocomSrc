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

#ifndef SKP_ROAD_CHANNELCENTER_H
#define SKP_ROAD_CHANNELCENTER_H


#include "global.h"
#include "RoadChannel.h"
#include "RoadXmlValue.h"
/******************************************************************************/
//	描述:智能交通检测系统通道处理中心。
//	作者:徐永丰
//	日期:2008-4-18
/******************************************************************************/

//通道实体映射
typedef std::map<int,CSkpChannelEntity*> ChannelEntityMap;
//通道数据列表
typedef std::list<std::string> ChannelMsg;


class CSkpChannelCenter
{
public:
	//构造
	CSkpChannelCenter();
	//析构
	~CSkpChannelCenter();

public:
	//初始化
	bool Init();
	//释放
	bool UnInit();

public:
	//添加一个通道实体,默认为不需要启动，动态添加时需要实时启动
	bool AddChannel(SRIP_CHANNEL sChannel,bool bStart = false,bool bConnect = false);
	//修改通道
	bool ModifyChannel(SRIP_CHANNEL sChannel);
	//删除通道
	bool DeleteChannel(SRIP_CHANNEL sChannel);
	//激活通道
	void ResumeChannel(int nChannelID);
	//暂停通道
	void PauseChannel(int nChannelID);
	//暂停检测
	void PauseDetect(int nChannelID = 0);
	//重新启动检测
    void RestartDetect();
	//连接断开通道
	void SetChannelConnect(bool bConnect,int nChannel = 0);
	//获取通道状态
	bool GetChannelStatus(int nChannelID);
	//获取连接的通道数目
	int GetConnectChannelCount();
    //相机控制
	void CameraControl(int nChannel,CAMERA_CONFIG& cfg);
    //返回通道检测类型
	CHANNEL_DETECT_KIND GetChannelDetectKind(int nChannel);

	//设置通道视频参数
	bool SetVideoParams(SRIP_CHANNEL_ATTR& sAttr);

	//处理数据
	void DealMsg();

	//添加一条数据,普通未检测图片
	bool AddResult(std::string& sResult);

    //清除数据
	void Clear();

	//重新加载通道检测参数
	void ReloadChannelParaMeter(SRIP_CHANNEL_EXT& sChannel,bool bEventParam=true);
	//重新加载车道检测参数
	void ReloadDetect(int nChannelID);

	//是否推送实时事件
	void SetRealTimeEvent(bool bRealTimeEvent,int nEventKind=0)
		{
			m_bRealTimeEvent = bRealTimeEvent;
			m_nEventKind = nEventKind;
		}
	bool GetRealTimeEvent(){ return m_bRealTimeEvent ;}

	//是否推送实时车牌
	void SetRealTimePlate(bool bRealTimePlate,int nPlateKind=0)
	{
	    m_bRealTimePlate = bRealTimePlate;
	    m_nPlateKind = nPlateKind;
    }
	bool GetRealTimePlate(){ return m_bRealTimePlate;}

	//发送实时事件
	int SendTraEvent(SRIP_DETECT_HEADER sDHeader,RECORD_EVENT event,SYN_CHAR_DATA* syn_char_data = NULL,bool bObject = false);
	//发送实时车牌
	int SendPlateInfo(SRIP_DETECT_HEADER sDHeader,RECORD_PLATE plate,SYN_CHAR_DATA* syn_char_data = NULL);

	//是否推送实时统计
	void SetRealTimeStat(bool bRealTimeStat){m_bRealTimeStat = bRealTimeStat;}
	bool GetRealTimeStat(){ return m_bRealTimeStat;}

	//是否推送实时日志
	void SetRealTimeLog(bool bRealTimeLog){m_bRealTimeLog = bRealTimeLog;}
	bool GetRealTimeLog(){ return m_bRealTimeLog;}

	//捕获一帧图象
	void CaptureOneFrame(std::string& result,int nChannel,ImageRegion imgRegion);

	//发送区域图像
   void SendRegionImage(int nChannel,ImageRegion imgRegion);

    //获取检测结果队列大小
    int GetResultListSize();

    //获取通道情况
    void GetChannelInfo(ChannelInfoList& chan_info_list);

    //获取通道图像分辨率
    void GetImageSize(int nChannel,int& nWidth,int& nheight);

	//发送相机控制消息
	void CameraControl(CAMERA_CONFIG& info);

	//获取相机编号
	int GetChannelCamId(int nChannel);

	//刷新通道
	bool UpdateChannel(SRIP_CHANNEL sChannel);

	//获取相机型号
	int GetChannelCamType(int nChannel);

	//获取通道方向
	int GetChannelDirection(int nChannel);

	//获取红灯方向
	int GetChannelRedDirection(int nChannel);

	//清空输出JpgMap列表
	void ClearChannelJpgMap(int nChannel);

	bool AddForceAlert(int nChannelID,FORCEALERT *pAlert);

	//发送停车严管区域位置
	void DetectRegionRectImage(int nChannel,ImageRegion imgRegion);
	//发送停车严管增加或删除目标区域
	void DetectParkObjectsRect(int nChannel,UINT32 uMsgCommandID,RectObject &ObjectRect);

	int AddRecordEvent(int nChannel,std::string result);

	//获取通道列表
	void GetAllChannelsInfo(CHANNEL_INFO_LIST& chan_info_list);

	//添加通道列表
	bool AddChannelList(CHANNEL_INFO_LIST& chan_info_list);
	//删除对应通道列表
	bool DelChannelList(CHANNEL_INFO_LIST& chan_info_list);

	//指定IP是否已经在通道列表,若存在返回对应通道ID
	UINT32 CheckChannel(SRIP_CHANNEL &chan_info);

	//清空数据前把,有图未输出的数据全部输出
	bool OutPutChannelResult(const int nChannelId);
	
#ifdef REDADJUST
	//红绿灯增强
	bool RedLightAdjust(const int nChannelId, IplImage *pImage);
#endif

	//通过uImgKey，核查记录状态
	bool CheckImgKeyState(const int nChannelId, const UINT64 &uKey);
	//更新通道记录标记
	bool UpdateImgKeyByChannelID(const int nChannelId, const UINT64 &uKey, const int &bState);

private:
	//处理检测结果
	bool OnResult(std::string& result);

	//启动检测数据处理线程
	bool BeginMsgThread();
	//停止检测数据处理线程
	void EndMsgThread();
private:
	//线程ID
	pthread_t m_nThreadId;
//	pthread_t m_nMutiThreadId;
	//通道实体信号互斥
	pthread_mutex_t m_Entity_Mutex;
	//通道映射 id->entity
	ChannelEntityMap m_ChannelEntityMap;


	//图片信号互斥
	pthread_mutex_t m_Pic_Mutex;
	//消息列表
	ChannelMsg	m_ChannelPicList;

	//检测结果信号互斥
	pthread_mutex_t m_Result_Mutex;
	//检测结果消息列表
	ChannelMsg	m_ChannelResultList;


	//是否推送实时事件
	 bool m_bRealTimeEvent;
	 //发送事件信息是否需要带上快照图片
	 int m_nEventKind;
	 //是否推送实时车牌
	 bool m_bRealTimePlate;
	 //发送车牌信息是否需要带上大小图片
     int m_nPlateKind;

	//是否推送实时统计
	 bool m_bRealTimeStat;
	//是否推送实时日志
	 bool m_bRealTimeLog;
};

//全局调用
extern CSkpChannelCenter g_skpChannelCenter;
#endif
