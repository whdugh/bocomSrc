// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


/**
 *   文件：MvsCommunication.h
 *   功能：检测器间通讯类
 *   作者：WanTao
 *   时间：2012-08-07
**/


#ifndef MVSCOMMUNICATION_H
#define MVSCOMMUNICATION_H

#include "CSocketBase.h"
#include "Common.h"
#include "XmlParaUtil.h"
#include "RoadChannelCenter.h"
#include "CenterServerOneDotEight.h"

//#define RECVPORT 6000
//#define SENDPORT 6001
//#define DISTANCE 3000

#ifndef CSResultMsg
typedef std::list<std::string> CSResultMsg;
#endif


//保存客户端套接字
typedef struct _MvsClientFd
{
	int fd;
	string HostIp;
	char chReserved[64];
	_MvsClientFd()
	{
		fd = 0;
		HostIp = "";
		memset(chReserved,0,64);
	}
}MvsClientFd;

typedef std::map<int,MvsClientFd> mapMvsClientFd;

class CMvsCommunication : public mvCSocketBase
{
public:
		//构造函数
		CMvsCommunication();
		//析构函数
		virtual ~CMvsCommunication();

		//启动侦听连接服务
		bool Init();
		//释放
		bool UnInit();

		//主动发起连接
		bool SendConnect();
		//接收连接
		bool RecvConnect();

		//获取接收套接字
		int& GetRecvSocket();
		//增加客户端套接字
		void AddClient(MvsClientFd& clientFd);
		//删除客户端套接字
		void DelClient(int fd);

		//获取主动连接成功标志
		bool& GetSendConnectFlag();
		//获取发送套接字
		int& GetSendSocket();

		//处理检测结果
		bool OnResult(std::string& result);
		//发送车牌信息
		bool SendPlateMsg(std::string& result);
		//接收某套接的发来的信息
		bool RecvFdMsg(int fd,string& buf,int nLength,int& nRecvSize);
		//接收车牌和图片信息
		bool RecvPlateAndPictureMsg(int fd);
		//获取图片路径并将图片编号存储在数据库中
		int GetPicPathAndSaveDB(std::string& strPicPath);
		//获取临时图片路径
		int GetPicPath(std::string& strPicPath);

		//根据车牌号码从数据库找对应的记录
		bool GetOneRecord(string plateNum,string& strMsg,string& strCrossingNumber);

		//保存图片数据
		bool SavePicData(string& picData,int PicDataSize,string& strPicPath);

		//区间速度
		float CalRegionSpeed(float distance,UINT32 seond);

		//将某号牌在上一个点位被抓的图片和本点位抓到的图片解码编码成一副图像
		//string ComposePicture(UINT32 nFromPicWidth,UINT32 nFromPicHeight,string& strFromPath,UINT32 nPicWidth,UINT32 nPicHeight,string& strPath);
		//将某号牌在上一个点位被抓的图片和本点位抓到的图片解码编码成一副图像(田字形摆放 小大小大)
		string ComposePicture(RECORD_PLATE nFromPlate,RECORD_PLATE& nPlate,UINT32& uRegionSpeed,string& strPic1,string& strPic2);
		//当表中记录大于1000条时，删除最早的记录及其对应的图片数据
		void ManageTmpRecordAndPic();

		void PutTextOnImage(IplImage* pImage,RECORD_PLATE nFromPlate,RECORD_PLATE nPlate,UINT32& uRegionSpeed,int nType);
		
		//设置字体大小
		void SetFontSize(int FontSize);
		//设置文本区域高度
		void SetExtentHeight(int nHeight);
		//准备发送给下一个点位的卡口数据，用于做区间测速
		bool PreDataForNextLocation(string& strMsg);
		//获取数据，并发送给下一个点位主机
		void GetDataAndSend();
		//添加区间超速数据到列表
		bool AddRegionOverSpeedData(string& strMsg);
		//根据车牌在区间超速数据列表中找到相应的记录
		string GetRegionOverSpeedData(string& strPlate);
		//区间超速，点超速取舍发送函数
		//函数输入 p：点超速车牌记录指针，nFlag：返回发送标志，0,表示发送点超速，1，表示发送区间超速
		//函数返回： 区间超速记录(车牌+图片相关信息);
		string RecordSendWay(RECORD_PLATE* plate, int& nFlag);
		//设置推送标志
		void SetFlag(bool nFlag); 

		//将某点位抓到的图片解码编码成一副图像
		string ComposePicture(RECORD_PLATE& nPlate,UINT32 nSpeed,int nType,string& strCrossingNumber);
		//在单点超速的图片上叠加文字
		void PutTextOnImage(IplImage* pComposePicture,RECORD_PLATE nPlate,UINT32 nSpeed,string& strCrossingNumber);
		//nDirection 为抓拍方向，函数返回行驶方向
		string GetstrDirection(RECORD_PLATE& nPlate,string& nDirection);

		//设置X,Y偏移量
		void SetOffsetXY(int nOffsetX,int nOffsetY);

		//设置抓拍方向
		void SetDetectDirection(int nDetectDirection);

		//区间超速的网络断开时，在起点做点超速

		void StartingPointOverSpeed(RECORD_PLATE* &sPlate);

		int& GetSendConnectFailureCount();

		pthread_mutex_t& GetConncetMutex_t();

		//拼区间测速图
		string ComposePic1x2(RECORD_PLATE nFromPlate,RECORD_PLATE& nPlate,UINT32& uRegionSpeed);
		//区间超速叠加文字
		void PutTextOnImage1x2(
			IplImage* pComposePicture,
			RECORD_PLATE fromPlate,
			RECORD_PLATE plate,
			UINT32 uSpeed,
			string& strCrossingNumber);
		//获取区间测速,1x2格式拼图叠加文字
		void CMvsCommunication::GetText1x2(
			const RECORD_PLATE &fromPlate, 
			const RECORD_PLATE &plate, 
			const UINT32 uSpeed,
			std::string &strText11, 
			std::string &strText12,
			std::string &strText21, 
			std::string &strText22);
		//按比例缩放图像
		bool ResizeComposePic(IplImage* pImg);
private:
		//接收套接字
		int m_uRecvSocket;
		//接收端口
		//int m_uRecvPort;
		//发送套接字
		int m_uSendSocket;
		//发送端口
		//int m_uSendPort;

		//客户端套接字列表
		mapMvsClientFd m_uMapMvsClientFd;
		//客户端套接字列表锁
		pthread_mutex_t m_uMapClientMutex;

		//主动连接成功标志
		bool m_uSendConnectFlag;

		//区间超速数据列表
		CSResultMsg m_RegionOverSpeedDataList;
		//区间超速数据列表信号互斥
		pthread_mutex_t m_RegionOverSpeed_Mutex;


		//本点位将要发送给下一个点位的数据列表
		CSResultMsg m_NextLocationDataList;
		//信号互斥
		pthread_mutex_t m_NextLocationData_Mutex;

		//接收连接线程ID
		pthread_t m_nRecLinkThreadId;
		//发送连接线程ID
		pthread_t m_nSendLinkThreadId;
		//记录处理线程ID
		pthread_t m_nRecordDealThreadId;
		//字体大小
		int m_FontSize;
		//文本区域高度
		int m_ExtentHeight;

		//将区间超速（点未超速）车牌推送给客户端
		bool m_nConnect;

		int m_nOffsetX;
		int m_nOffsetY;

		int m_nDetectDirection;

		//失败连接次数
		int m_nSendConnectFailureCount;
		//失败连接次数
		pthread_mutex_t m_nConnect_Mutex;

		//缩放图像
		IplImage* m_imgResize;

};
extern CMvsCommunication g_mvsCommunication;
#endif

