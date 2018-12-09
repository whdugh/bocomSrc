// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef	SJSERVER_H
#define SJSERVER_H

#include "CSocketBase.h"
#include "AMSConstdef.h"
#include "FtpCommunication.h"
#include "XmlParaUtil.h"


#define SJ_IJL_DIB_ALIGN (sizeof(int) - 1)

#define SJ_IJL_DIB_UWIDTH(width,nchannels) \
	((width) * (nchannels))

#define SJ_IJL_DIB_AWIDTH(width,nchannels) \
	( ((SJ_IJL_DIB_UWIDTH(width,nchannels) + SJ_IJL_DIB_ALIGN) & (~SJ_IJL_DIB_ALIGN)) )

#define SJ_IJL_DIB_PAD_BYTES(width,nchannels) \
	( SJ_IJL_DIB_AWIDTH(width,nchannels) - SJ_IJL_DIB_UWIDTH(width,nchannels) )

#ifndef	SJ_Result
typedef std::list<std::string> SJ_Result;
#endif

class CSJServer:public mvCSocketBase
{

public:
	//构造
	CSJServer();
	//析构
	virtual ~CSJServer();

	//启动侦听服务
	bool Init();

	//释放
	bool UnInit();

	//添加一条数据
	bool AddResult(std::string& strResult);

	//根据通道号，车道号获取相应车道的限速值
	//UINT32 GetSpeed(int nChannel,int nRoadId);

	string CarTypeConvert(UINT32 nCarType);

	//通过ftp发送数据
	bool SendDataByFtp(const string& strMsg,string& strRemotePath);

	bool mvSendRecordToCS(const string &strMsg);

	//处理检测结果
	bool OnResult(std::string& result);

	//处理实时数据
	void DealResult();

	string GetSingleImageByPath(string& strPicPath,int& index);

	//获取历史记录
	void mvOrdOneRecord();

	void mvDealHistoryRecord();

private:
	pthread_t m_nThreadId;
	pthread_t m_nHistoryThreadId;
	
	SJ_Result m_ResultList;
	pthread_mutex_t m_Result_Mutex;

	//bool m_FtpConnectedFlag;

};


extern CSJServer g_SJServer;
#endif