// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _DSPSERVER_H_
#define _DSPSERVER_H_

#ifndef ALGORITHM_YUV
#include "AbstractCamera.h"
#include "CSocketBase.h"
#include "mvdspapi.h"

#include "ManageClient.h"
#include "DspDataManage.h"
#include "DspDealVideo.h"
#include "DspDealVideoTcp.h"
#include "YiHuaLuCenter.h"
#include "global.h"

#define NUM_BUFF	5					//buffer number for motion jpeg
#define MAX_PLATE_BUF 512
#define NUM_H264_BUFF	3
//map<string,string> m_CameraIPMap;
class CManageClient;
class CDspServer:public AbstractCamera
{
public:
	CDspServer(int nCameraType);
	~CDspServer();
	
	virtual bool Open();
	virtual bool Close();

	bool CloseFd(int &nSock);

	//绑定和监听
	bool InitDspServer();
	//初始化侦听套接字，创建接收线程
	bool InitDspServerSocket();

	//销毁侦听套接字，停止接收线程
	void UnInitDspServerSocket();

	//接收DSP客户端连接
	void MvsDspAccept();

	void RecvDspData(int nRecvFd);
	//接收对应Fd内容
	bool RecvDspDataFd(const DspSocketFd &dspSocket, char * bufRecv, int &nLen);	
	//接收包头
	bool RecvHeadFd(const DspSocketFd &dspSocket, char * buffer, int &bytesRecved, bool &bSynchron1, bool &bSynchron2);

	//处理dsp相机回复
	bool DealDspAck(const DspSocketFd &dspSocket, const Image_header_dsp* pDsp_header, const bool &bAckFlag);

	//设置套接字并开启接收线程
	bool SetRecFd(const DspSocketFd dspSocket);

	//接收数据包
	bool SafeRecvFd(const DspSocketFd &dspSocket,char*pBuf, int nLen, int &nRecvSize);
	
	//等待接收缓冲有数据可读
	int WaitForSockCanRead(int sock, int nTimeOut /* = 1000 */);

	//发送DSP回复心跳包
	bool SendToDspLinkFd(DspSocketFd &dspSocket);

	int& GetMvsDspSockFd()
	{
		return m_nSerTcpFd;
	}

	int & GetClientFd()
	{
		return m_nClient;
	}
	
	CDspDataManage * GetPDataProcess()
	{
		return m_pDataProcess;
	}

	//开起录像
	bool StartRecord(DspVideoClient_Fd &dspVideocfd);
	//监控客户端录像
	void MvsDspRecordAccept();

	//给特定套接字发送数据包
	bool SafeSendFd(int nFd,const char* pBuf, int nLen);

	//重启服务端
	bool ReOpen();

	//根据相机IP获取相机编号
	std::string GetCameraIDByIp(const char* szAddress);

	//保存相机IP和编号
	void SetCameraIPAndDeviceID(const char* szAddress,const char *szCameraID);
	//void SetCameraIPAndDeviceID(string strAddress,string strCameraID);
private:
	//初始化
	void Init();

	//等待接收缓冲有数据可读
	//int WaitForSockCanRead(int sock, int nTimeOut /* = 1000 */);

	//发送检测器时间给DSP
	bool SendToDspTimeFd(DspSocketFd &dspSocket);


private:

	//数据
	int m_nSerTcpFd;
	//int m_nTcpFd;
	int m_nPort;
	
	int m_nOriWidth;//原图像宽度        
	int m_nOriHeight;//原图像高度

	mvCSocketBase *m_nSocketBase;//从侦听类中派生对象用于侦听

	//MapClient m_MapClient;//客户端连接映射列表
	//pthread_t m_nThreadId7;//线程ID7接收公交h264码流线程

	
	bool m_bOpenCamera; //打开相机标志
	int m_nClient;

	//管理客户端连接类
	CManageClient *m_pManageClient;

	//车牌检测类
	CDspDataManage *m_pDataProcess;
	bool m_bInit; //是否已经初始化过

	//录像处理类
	CDspDealVideo *m_pDspDealVideo;

	//TCP录像处理类
	CDspDealVideoTcp *m_pDspDealVideoTcp;

	//断开连接次数
	int m_nDisConnectCount;

	//相机IP和相机编号映射表
	map<string,string> m_CameraIPMap;
};

#endif
#endif
