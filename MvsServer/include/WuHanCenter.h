#ifndef WUHANCENTER_H
#define WUHANCENTER_H

#include "CSocketBase.h"
#include <string>
#include <iostream>
#include <list>
#include "CSeekpaiDB.h"
#include "MysqlQuery.h"
#include "Common.h"
#include "FtpCommunication.h"
#include "structdef.h"

using namespace  std;


class WuHanCenter : public mvCSocketBase
{
public:
	WuHanCenter();
	~WuHanCenter();
	// 创建实时发送线程， 和历史发送线程
	int Init();
	bool Uninit();
	// 发送实时数据的入口
	static void * SendRealTimeData(void * pAg);
	// 发送历史数据信息的入口
	static void * SendHistoryData(void * pAg);
	// 增加一条消息未处理的消息
	bool AddResult(const std::string& strResult);
	// 取出一条数据进行处理
	void DealResult(void);
	//  发送数据
	int OnSendResult(string strResult);
	// 处理和发送历史数据
	//int OnSendHistoryResult();
	// 制作保存的路径
	string MakeRemoteImageName(RECORD_PLATE * pPlate );
	//string MakeRemoteImageName(MysqlQuery sqlData );
	
	//车牌颜色的转换
	int CardColorChange(int nColor);
	// 行驶方向的转换
	string DirectionChange(int nDirection);
	//获取违章类型
	int CarViolationTypeChange(int nType);
	//车身颜色
	string CarColorChange(int nColor);
	//车辆品牌
	string WuHanCenter::CarFactoryChange(UINT32 nLable);
	// 把数据库数据转换成 RECORD_PLATE
	string SqlDataTransitionRECORD_PLATE();
	
private:
	pthread_t			m_nThreadRealTime;
	pthread_t			m_nThreadHistory;

	bool					m_bIsCenterLink;	
	pthread_mutex_t		m_Result_Mutex;
	std::list<string>	 	m_listRecord;
	int m_nNo;// 顺序号
};


extern WuHanCenter g_WuHanCenter;

#endif
