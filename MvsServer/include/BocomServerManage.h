// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2014 上海博康智能信息技术有限公司
// Copyright 2008-2014 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef BOCOM_SERVER_MANAGE_H
#define BOCOM_SERVER_MANAGE_H

#include "BocomServer.h"

//端口号,Bocom中心端指针,映射
typedef std::map<int, CBocomServer *> BocomServerPtrMap;

/******************************************************************************/
//	描述:BOCOM中心端通讯管理模块
//	作者:yuwx
//	日期:2014-6-6
/******************************************************************************/
class CBocomServerManage
{
public:
	CBocomServerManage();
	~CBocomServerManage();

	//初始化,开始侦听服务
	bool Init();
	//反初始化,释放资源
	bool UnInit();

	//添加BocomServer
	bool AddBocomServer(int nPort, int nId);

	//添加一条数据
	bool AddResult(std::string& strResult);

	//处理一条命令
	bool mvOnDealOneMsg();
	//分发卡口记录
	//分发事件记录
	
	//获取某个server设备状态
	bool GetServerDeviceStatus();
private:
	BocomServerPtrMap m_pBocomServerMap;
};

//通讯服务
extern CBocomServerManage g_BocomServerManage;

#endif