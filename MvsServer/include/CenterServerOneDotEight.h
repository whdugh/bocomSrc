/*
 * File:   CenterServerOneDotEight.h
 * Author: wantao 
 *
 * Created on 2012年7月4日, 下午13:44 based on CenterServer.h
 */


#ifndef _CENTERSERVERONEDOTEIGHT_H
#define _CENTERSERVERONEDOTEIGHT_H

#include "CSocketBase.h"
//#include "RoadCarnumDetect.h"

#define  LIMITSPEED 60
#define  WAITTIME 5

#define HISRECORDTRANSFERWAY 1 //1每隔一小时补传 2隔天补传
#define TRANSFREMOMENT 7 //在每个小时的多少分开始补传
//#define CS_FILE_LOG

extern bool g_MybCenterLink;
extern string g_MystrCSMsgVer;

#ifndef UINT
typedef unsigned int UINT;
#endif

/*
#ifndef mapMaxSpeed
typedef map<UINT32,UINT32> mapMaxSpeed; //车道对应的限速map
#endif

#ifndef mapChanMaxSpeed
typedef map<UINT32,mapMaxSpeed> mapChanMaxSpeed; //某通道对应的限速map
#endif
*/

#ifndef CS_MSG_QUEUE
typedef multimap<string, string> CS_MSG_QUEUE;
#endif

#ifndef CSResultMsg
//通道数据列表
typedef std::list<std::string> CSResultMsg;
#endif

//黄标车信息
typedef struct _VEHICLE_INFO
{
	int nHPYS;
	int nWFLX;

	_VEHICLE_INFO()
	{
		nHPYS = 0;
		nWFLX = 0;
	}
}VEHICLE_INFO;

class mvCSocketBase;

class mvCCenterServerOneDotEight:public mvCSocketBase
{
    public:
        /*构造函数*/
        mvCCenterServerOneDotEight();
        /*析构函数*/
        ~mvCCenterServerOneDotEight();
		map<string, VEHICLE_INFO> m_PlateMap;

    public:
        //初始化
        bool Init();
        //释放
        bool UnInit();
        /*主线程调用接口*/
        void mvConnOrLinkTest();

        //添加一条数据,普通未检测图片
        bool AddResult(std::string& strMsg);

        //处理数据
        void DealResult();

    public:
        /*压入一条消息*/
        void mvPushOneMsg(string strMsg);
        /*弹出一条消息*/
        bool mvPopOneMsg(string &strCode, string &strMsg);
        /*设置检测器ID*/
        void mvSetDetectorId(const char *pDetectorId);

    public:
        /*处理一条消息*/
        bool mvOnDealOneMsg(const char *pCode, const string &strMsg);
        /*重组并发送消息*/
        bool mvRebMsgAndSend(int& nSocket,const char *pCode, const string &strMsg);

    public:
		void FtpFownLoad();
		bool mvSendHMDToCS(string strFileName);
		bool mvOnFtpDownloadRep(const string &strMsg);
		unsigned long long  GetFileLine(string strFileName);
		bool DownloadFile();
		int FoundPlateInfo(char* strText,int nColor);
		void AddPlate2Map(string strLine);
		std::string GetLastFileName(std::string strList);

        /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg, bool bRealTime = true);
        /*结束工作*/
        bool mvOnEndWork();
        /*检查是否空闲，空闲则处理历史记录*/
        void mvCheckFreeAndDealHistoryRecord();
        //接收中心端消息
        void mvRecvCenterServerMsg();

        /*接收消息*/
        bool mvRecvSocketMsg(int nSocket, string& strMsg);

		//设置线圈状态
		void SetLoopStatus(mapLoopStatus& vLoopStatus);

		//设置通道对应的线圈状态
		void SetChanLoopStatus(mapChanIdLoopStatus& vChanLoopStatus);

		/*检查设备状态*/
        void mvCheckDeviceStatus(int nDeviceStatusKind);

		//获取线圈状态
		mapLoopStatus GetLoopStatus();

		//获取通道线圈对应的线圈状态
		mapChanIdLoopStatus GetChanLoopStatus();

		//设置发送车牌图片消息体
		string SetPlatePicAndVideoMsg(RECORD_PLATE& plate,bool bRealTime);//(string path);
		//设置发送事件图片消息体
		string SetEventPicAndVideoMsg(RECORD_EVENT& pevent,bool bRealTime);//(string path)

		//获取车牌违法代码
		UINT GetPlateVioCode(RECORD_PLATE& plate);
		//获取事件违法代码
		UINT GetEventVioCode(RECORD_EVENT& pevent);
		//根据时间戳获取日时分秒数字
		void ToGetDateHourMinuteSec(long Time,int &day,int& hour,int& minute,int &second);
		//根据时间戳确定是否在补传时间段
		bool IsTransferMoment(unsigned int uTimeStamp);
		//设置补传起止时刻
		bool SetBeginAndEndTime(unsigned int uTimeStamp);
		//设置事件消息体中，违法超速设备的限速值设置
		void SetEventLimitSpeed(RECORD_EVENT& pevent,UINT& uLimitSpeed);
		//设置车牌消息体中，违法超速设备的限速值设置
		void SetPlateLimitSpeed(RECORD_PLATE& plate,UINT& uLimitSpeed);
		//实时记录发送后是否收到中心端的回复
		bool IsRealTimeRepTrue();

		string SeparatePlateVtsPicture(RECORD_PLATE& plate);
		string SeparateEventVtsPicture(RECORD_EVENT& pevent);

		//获取车道总数
		void SetRoadCount();
		//获取车道状态
		string GetRoadStatus();
		//获取相机的状态
		string GetCameraStatus();

		//红灯随即时间 2 - 8s;
		int GetRedLightTime();

		//当卡口图片不存储小图时，从大图中抠取小图(对于机动车)
		string GetSmallPicFromBigPic(RECORD_PLATE* nPlate,int& nSmallPicSize);
		//当卡口图片不存储小图时，从大图中抠取小图(对于非机动车，行人等事件)
		string GetSmallPicFromBigPic(RECORD_EVENT* nPevent,int& nSmallPicSize);
		//设置文本框的高度
		void SetExtentHeight(int nExtentHeight);
		//设置检测方向
		void setDirection(int nDirection);
		//设置解码后图片宽度
		void SetPicWidth(int nPicWidth);
		//设置解码后图片高度
		void SetPicHeight(int nPicHeight);
		//对行人和非机动车抠图
		CvRect GetPos(RECORD_EVENT pevent,int nType);
		//获取车身位置
		CvRect GetCarPos(RECORD_PLATE plate,int nType);
		//设置图片宽高比例
		void SetRatio(float nRatio);
		//将对方传过来的代码转换为我方代码
		string mvTransCode(UINT nCode);
		//返回强制重传类型
		char GetForceType();

		//获取图片路径
		void GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath,int nType = 0);

		//是否为空目录 
		bool IsEmptyDir(string& strPath);
		//删除最早文件及空目录
		void DelEarlistFileAndDir(string& strPath,int nType);
		//卡口记录磁盘管理
		void ManagerDisk();

		//获取最小的目录编号
		int GetMinPath(string& strPath);
		//删除最小的目录
		void DelMinDir();
		//删除最小的小时目录
		void DelMinHourDir(string& strPath);

		//获取卡口图像的宽高比
		float GetRatio();

		//重连
		void LinkTest();

		void CreateWCDMAThread();
		static void * DealWCDMAFunc(void * lpParam);
		void * DealWCDMA();
		int GetTimeT();

    private:
        /*请求并连接中心端*/
        bool mvRequestAndConnCS();
        /*连接中心端并开始接收消息*/
        bool mvConnCSAndRecvMsg();
        /*连接到中心端*/
        bool mvConnectToCS();
        /*启动接收消息线程*/
        bool mvStartRecvThread();

        /*取出一条历史记录*/
        bool mvGetOneHistoryRecord(std::list<string>& strListRecord);

        //处理检测结果
        bool OnResult(std::string& result);

    private:
        /*发送路由请求并等待回复*/
        bool mvSendRouterReqAndRecv();
        /*发送路由请求*/
        bool mvSendConnReqToRouter();
        /*接收路由回复*/
        bool mvRecvIpPortFromRouter();

    private:
        /*开始工作*/
        bool mvOnDetectRestart(); //(const string &strMsg);
        /*标记未发送，强制重传*/
        bool mvOnMarkUnsend(const string &strMsg);
        /*记录查询*/
        bool mvOnRecordQuery(const string &strMsg);
        /*获取设备状态*/
        bool mvOnDeviceStatus(const string &strMsg);
        /*收到结束工作回复*/
        bool mvOnEndWorkRep(); //(const string &strMsg);
        /*实时记录回复*/
        bool mvOnRealTimeRecordRep(); //(const string &strMsg);
        /*历史记录回复*/
        bool mvOnHistoryRecordRep(); //(const string &strMsg);
        /*时钟设置*/
        bool mvOnSysTimeSetup(const string &strMsg);

    private:
        /*发送心跳测试*/
        bool mvSendLinkTest();
        /*发送重启应答*/
        bool mvSendRestartRep();
        /*发送标记未发送应答*/
        bool mvSendMarkUnsendRep(const string &strMsg);
        /*发送记录查询应答*/
        bool mvSendRecordQueryRep(const string &strMsg);
        /*发送设备状态查询应答*/
        bool mvSendDeviceStatusRep(const string &strMsg);
        /*发送设备报警*/
        bool mvSendDeviceAlarm(const string &strMsg);
        /*发送结束工作报告*/
        bool mvSendEndWork();
		
    private:
        /*CRC编码*/
        void mvCRCEncode(const string &strSrc, string &strRes);
        /*获取日期*/
        void mvGetDateTimeFromSec(const long &uSec, string &strDate, int nType = 0);
		string MD5_File(const char* path,int md5_len);

    public:
//#ifdef CS_FILE_LOG
//        FILE *m_pCSRecvLog;
//        FILE *m_pCSSendLog;
//        FILE *m_pCSConnLog;
//#endif

    private:
        int m_nControlSocket;
        int m_nCenterSocket;
        int m_nCSLinkCount;
        int m_nConnectTime;//统计连接次数
        int m_nAllDisk;
        bool m_bHistoryRep;
        string m_strHeader;
        string m_strEnd;
        string m_strDetectorId; //including ' ' as valid character.
        unsigned int m_uHistoryRepTime; //历史记录回复时刻
        unsigned int m_uCSLinkTime;//心跳发送时间

		bool m_bEndWCDMAThread;

		bool m_bFtpUpdate;
		int m_nFtpUpdateHour;
		bool m_bUpdateOnce;
		string m_strRemoteFile;

		pid_t pidoff;
		pid_t pidon;

    private:
        CS_MSG_QUEUE m_mapCSMsg;
        pthread_mutex_t m_mutexMsg;

        //线程ID
        pthread_t m_nThreadId;
        //检测结果信号互斥
        pthread_mutex_t m_Result_Mutex;
        //检测结果消息列表
        CSResultMsg	m_ChannelResultList;

		//线圈状态
		mapLoopStatus m_vLoopStatus;

		//通道对应的线圈状态
		mapChanIdLoopStatus m_vChanIdLoopStatus;

		std::string m_uBeginTime;
		std::string m_uEndTime;
		char m_uForceTransferType;
		int	m_uForceTransCount;
		string m_uForceTransBeginTime;
		string m_uForceTransEndTime;
		bool m_uRealTimeRep;
		//int m_uSavePicCount;

		map<UINT32, UINT32> m_uMaxSpeedMap;

		mapChanMaxSpeedStr m_uChanMaxSpeedStrMap;

		int m_uRoadCount;
		int m_uCameraCount;
		//CRoadCarnumDetect pCarnumDetect;

		//解码后图片的宽高（用于抠取小图）
		int m_uPicWidth;
		int m_uPicHeight;

		//文本框高度
		int m_uExtentHeight;
		//检测方向
		int m_uDetectDirection;
		//大图的宽高比
		float m_uRatio;

		//磁盘管理线程ID
		pthread_t m_nDiskManagerThreadId;
		//重连线程
		pthread_t m_nLinkThreadId;
		pthread_t m_nHistoryThreadId;
		int m_nCpuAlarmCount;
		bool m_bEndRecvThread;
		  //线程ID
	    pthread_t m_nRecvThreadId;
		//连接数据服务器次数
		int m_nConnectCount;
		unsigned int m_uCSTime;//上次连接成功数据服务器时间
		int m_nSendFailCount;//记录连续发送失败次数
};

extern mvCCenterServerOneDotEight g_MyCenterServer;

#endif // _CENTERSERVER_H


