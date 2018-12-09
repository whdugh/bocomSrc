// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef _TJSERVER_H
#define _TJSERVER_H

#include "CSocketBase.h"
#include "global.h"

#ifdef KAFKA_SERVER
#include "kafkaManage.h"
#endif

/**
    文件：TJServer.h
    功能：天津电警通讯类
    作者：yuwenxian
    时间：2010-12-20
**/


//通道数据列表
typedef std::list<std::string> TJResultMsg;

typedef std::list<RECORD_PLATE> VIDEO_LIST;

class mvCTJServer:public mvCSocketBase
{
    public:
        //构造
        mvCTJServer();
        //析构
        virtual ~mvCTJServer();

    public:
        //启动侦听服务
        bool Init();
        //释放
        bool UnInit();

        //通过ftp发送数据
        bool SendDataByFtp(string strLocalPath,string strRemotePath,string& strMsg, CFtpCommunication *pFtp);
		bool SendDataByFtpForNanning(string strLocalPath,string strRemotePath,string& strMsg, CFtpCommunication *pFtp);

    public:
        /*发送记录到中心端*/
        bool mvSendRecordToCS(const string &strMsg,bool bSymlink = true);

        void mvDealHistoryRecord();

        //处理数据
        void DealResult();
		void DealStatusResult();
		void SendStatusInfo();

        //添加一条数据,普通未检测图片
        bool AddResult(std::string& strMsg);

         //添加一条录像数据
        void AddVideo(RECORD_PLATE& plate);

        //删除时间目录
        void DelDir();
		//发送录像
		void SendVideo();
		bool SendVideoForTianjin();
		bool SendDataByFtpForTianjin(string strLocalFilePath);
		bool SendDataByFtpForTianjinMatch(string strLocalFilePath);
		//根据检测方向,取不同路径录像,发送
		bool SendFileByDirection(string strLocalFilePath, const int nDetectDirection);

		//从配置文件中读入的限速数值Map; <key:车道号 value:最大速度>
		//map<UINT32, UINT32> m_maxSpeedMap;
		//mapMaxSpeedStr m_maxSpeedMapStr;

    private:

        //处理检测结果
        bool OnResult(std::string& result);

        //获取历史记录
        void mvOrdOneRecord();

		//递归删除目录
		bool RemoveDir( const char* dirname,int nType);

		//载入FTP配置文件
		bool LoadFtpConfig();

		//通过大图路径,拆分成小文件
		bool DepachBigJpg(
			std::string &strPicPath, 
			int nWidthBig, 
			int nHeightBig, 
			unsigned char* pOutJpgBuf[], 
			int nJpgSize[]);
		//拆分大图成小图
		bool DePachBigImg(IplImage* pImgBig, int nPicMode, IplImage* pImgArray[], int nWidth, int nHeight);
		//获取小图拆分区域
		bool GetImgRect(int nPicMode, int nWidth , int nHeight, int i, CvRect &rect);

		bool InitImg200(int w, int h);
		bool InitImg500(int w, int h);
		//过滤军车,武警,警车记录
		bool JunCheck(const RECORD_PLATE & plate);

     private:
#ifdef KAFKA_SERVER
		CKafakaManage* m_kafkaManage;
#endif
        pthread_mutex_t m_Video_Mutex;

        //线程ID
        pthread_t m_nThreadId;
		pthread_t m_nStatusThreadId;
        pthread_t m_nHistoryThreadId;
        //检测结果信号互斥
        pthread_mutex_t m_Result_Mutex;
        //检测结果消息列表
        TJResultMsg	m_ChannelResultList;

        //录像列表
		VIDEO_LIST m_listVideo;
        //中心端目录结构路径
        string m_strFtpRemoteDir;
        //中心端目录结构时间路径
        string m_strFtpRemoteTimeDir;

		//FTP配置
		FTP_HOST_INFO m_FtpSetting[5];
		FTP_HOST_INFO* m_sKakouSetting;
		FTP_HOST_INFO* m_sBreakRulesSetting;
		FTP_HOST_INFO* m_sStatisticSetting;
		FTP_HOST_INFO* m_sVideoSetting;
		FTP_HOST_INFO* m_sStatus;

		CFtpCommunication m_FtpKakou;
		CFtpCommunication m_FtpBreakRules;
		CFtpCommunication m_FtpStatistic;
		CFtpCommunication m_FtpVideo;
		CFtpCommunication m_FtpStatus;

		int m_nNo;// 顺序号 GCS
		int m_nVideoType;
		int m_nStatusNo;// 顺序号
		bool bInitImgBig200;
		bool m_bStarting;
		IplImage* pImgBig200;
		IplImage* pImgArray200[4];

		bool bInitImgBig500;
		IplImage* pImgBig500;
		IplImage* pImgArray500[4];
};

extern mvCTJServer g_TJServer;

#endif // _TRAVELSERVER_H
