#ifdef YUHUANCENTER_OK

#ifndef YUHUANCENTER_H
#define YUHUANCENTER_H

/*
 *  文件名： YuHuanCenter.h
 *  功能： 玉环项目， 把检测的图片结果经过webService 传送给对方
 *  作者： 牛河山
 *  时间： 2013、05
 */
#include "CSocketBase.h"
#include <string>
#include <iostream>
#include <list>
#include "CSeekpaiDB.h"
#include "MysqlQuery.h"
#include "Common.h"
#include "FtpCommunication.h"
#include "structdef.h"

#include "soapH.h"


class YuhuanCenter : public mvCSocketBase
{

public:

	YuhuanCenter();
	~YuhuanCenter();

	// 创建实时发送线程， 和历史发送线程
	int Init();
	bool Uninit();
	// 发送实时数据的入口
	static void * SendRealTimeData(void * pAg);
	// 发送历史数据信息的入口
	static void * SendHistoryData(void * pAg);
	// 定时清除数据图片
	static void * TimingClearPic(void * pAg);
	// 增加一条消息未处理的消息
	bool AddResult(const std::string& strResult);
	// 取出一条数据进行处理
	void DealResult(void);
	//  发送数据
	int OnSendResult(string strResult);
	//  数据的转换和发送
	int DataChangeAndSend(RECORD_PLATE * pPlate);
	// 行驶方向的转换
	string DirectionChange(int nDirection);
	int    CardColorChange(int nColor);
	string SqlDataTransitionRECORD_PLATE();
	//获取车辆类型
	string GetCarType(int nType);
	// 初始化soap 及其登陆服务器
	int InitGsoap();
	//获取随机数
	int GetRandCode();
	string MakeImageName(RECORD_PLATE * pPlate);
	void CpFile(char *spathname,char *tpathname);
	//得到磁盘的剩余空间
	int GetDiskFree();

	string GetFirstFileName(string strPath);
	void DelOldPic();

private:
	pthread_t			m_nThreadRealTime;
	pthread_t			m_nThreadHistory;

	pthread_mutex_t		m_Result_Mutex;
	std::list<string>	 	m_listRecord;

	string m_strLogId;
	string m_strServiceCode; 

	int m_isConnetdServer;

	
	struct soap m_CDSoap;

};

extern YuhuanCenter g_YuHuanCenter;

#endif

#endif