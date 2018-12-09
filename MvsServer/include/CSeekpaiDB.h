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

//CSeekpaiDB.h
#ifndef CSEEKPAIDB_H
#define CSEEKPAIDB_H
#include "MysqlQuery.h"
#include "MysqlDB.h"
#include "Common.h"
#include "VehicleConfig.h"
#include "RoadXmlValue.h"

//*********************************************************
//*		作用：服务端后端访问类
//*		日期：08.04.03
//*		作者：闫
//*
//*********************************************************

using namespace std;

typedef std::string String;
typedef std::list<String> StrList;
typedef std::list<int> IntList;
typedef std::map<int,int> IntMap;


//用户信息
typedef struct _UserStru
{
	String strName;
	String strPw;
	int    nRight;
	_UserStru()
	{
		strName="";
		strPw="";
		nRight=0;
	}
}UserStru;


typedef std::map<String,UserStru> UserMap;

//中心端查询结构
typedef struct _RecordStru
{
	int nSocket;  //套接字
	int nSearchKind;//查询类别
	int nCmdFlag;//是否需要发送图像
	_RecordStru()
	{
		nSocket=0;
		nSearchKind = 0;
		nCmdFlag=0;
	}
}RecordStru;


class CSeekpaiDB:public MysqlDB
{

	public:
		CSeekpaiDB();

	    ~CSeekpaiDB();

	public:
		//初始化DB
		bool Init();
		//释放资源
		void UnInit();

		//记录系统首次运行时间
		bool WriteTimeInfo();
		//获取检测器信息
        string GetDeviceInfo();

		//获取上次数据库记录时间
		String GetLastRecorderTime();

		//添加新用户
		int AddUser(String strName,String strPw,int nKind);
		//修改用户password
	    int ModyUser(String strName,String strPw,int nKind);
		//删除用户
		int DelUser(String strName);

		//读取用户列表
		void ReadUser();
		//验证用户密码
		int CheckLogin(String strName,String strPw);
		//读取用户权限
		int GetUserPriv(String strName,int nAction);
		//读取所有用户列表
		String GetUsrList();

		//获取日期路径
        String GetDatePicPath(time_t uTime);

		//读取车牌信息
		String GetCarNum(SEARCH_ITEM& search_item,int nPageSize=0);
        //修改车牌信息
		int ModifyCarNum(int nSeq,String strRequest);

		//读取日志信息
		String GetSysEvent(SEARCH_ITEM& search_item,int nPageSize=0);

		//获取事件信息
		String GetEvent(SEARCH_ITEM& search_item,int nPageSize=0);

		//获取统计信息
		String GetStatistic(SEARCH_ITEM& search_item,int nPageSize=0);

		//添加录像通道
		int AddChan(SRIP_CHANNEL sChannel);
		//删除录像通道
		int DelChan(SRIP_CHANNEL sChannel);
		//修改通道信息
		int ModyChanInfo(SRIP_CHANNEL sChannel);
		//读取视频通道列表
		String GetChannelList();

		/*更新通道运行状态*/
		int UpdateChannelStatus(int nChannel,int nRun);

		//获取源文件名
		String GetSrcFileByID(int nID);

		//定期删除db中的内容
		int DeleteOldContent(/*int nChannel,unsigned int uTimeStamp,int nType =0*/);
		//删除已经存在的记录
		int DeleteOldRecord(string strPath,bool bCleanDisk,bool bVideo);

		//取得最旧的全天录像名
		String GetOldDayVideoPath();

        //获取录象信息
		String GetVideoRecord(SEARCH_ITEM& search_item,int nPageSize=0);
		//删除录象文件
		int DeleteRecord(unsigned int uBegin,unsigned int uEnd);
		//获得录像时长
		UINT32 GetVideoTimeSize(int nChannelId);

		//获得图表数据信息
		String GetChartQuery(int nChannelId,int nRoadIndex,String strDate, int dateType, int queryType, int typeValue);

		/*********************Backup*********************/

		//备份数据库
		int BackupDB(String strDBName, String strBackupPath);


		/********************Channel********************/
	
		int AddVideoRecord(string strVideoPath, int nVideoType,int nCameraId);
		string GetVideoRecord(int& nType,int& nCameraID);
		int UpdateVideoRecord(string strVideoPath, int nFlag);
		int DeleteVideoRecord();
		RECORD_PLATE GetPlateByVideoPath(string strVideoPath);

		/* 保存车道坐标信息 */
		int SaveRoadSettingInfo(CSkpRoadXmlValue& xmlValue, int nChannelId = 0);
		/* 车道参数设置 */
		int SaveRoadParaMeterInfo(CSkpRoadXmlValue& xmlValue);
		/* 车道模板参数设置 */
		int SaveRoadModelInfo(CSkpRoadXmlValue& xmlValue, int& nModelID);

		/* 获取车道参数 */
		String GetRoadParaMeterInfo(int nID);

		/* 删除车道信息, 成功返回true, 不成功返回false */
		int DeleteRoad(int nChannel);
		/* 保存共有区域信息 */
		String SaveCommonRegion(const char *strConfigName,CSkpRoadXmlValue& xmlValue, CHANNEL_INFO& channel_info);


		/**********************************/

		//初始化通道采集列表----2008-5-7 徐永丰
		void InitChannelList();
		//设置视频参数[亮度+饱和度+对比度+色调]
		int ModifyVideoParams(SRIP_CHANNEL_ATTR& sAttr);
        //读取视频参数[亮度+饱和度+对比度+色调]
        int GetAdjustPara(SRIP_CHANNEL_ATTR& sAttr);
		///////////////////////////////////////////////////
		//获取记录状态
		RECORD_STATUS GetRecordStatus(int nKind);

		//保存日志记录
		int SaveLog(String strTime,String strText,unsigned int uCode,bool bAlarm = false,int nCameraId = 0);
		//获取日志记录
		void GetLog(unsigned int uBegin,unsigned int uEnd,int nKind,int nSocket = 0);
		//删除日志记录
		int DeleteLog(unsigned int uBegin,unsigned int uEnd,int nKind);

		//保存统计信息
		int SaveStatisticInfo(int nChannel,RECORD_STATISTIC& statistic,int nType = 1);
		//删除统计信息
		int DelStatisticInfo(unsigned int uBegin,unsigned int uEnd,int nKind);
		//读取统计信息
		void GetStatisticInfo(unsigned int uBegin,unsigned int uEnd,int nKind,int nSocket = 0);
		///////////////////////////////////////////////////

		//保存事件信息
		int SaveTraEvent(int nChannel,RECORD_EVENT& event,unsigned int uPicId,unsigned int uVideoId = 0,bool bSendEventToCenter = true,int nRecordType = 0);
		//通知录象结束
		void VideoSaveUpdate(String strVideoPath,int nChannelID,int nVideoType = 0);
		void GetVideoSaveString(String &strVideoPath,unsigned int uID,int nVideoType = 0);

		//获取录象状态
		bool GetVideoState(String strVideoPath);
		//获取录象列表
        void GetVideoList(StrList& listVideoPath,int nVideoSave);
		//删除事件信息
		int DelTraEvent(unsigned int uBegin,unsigned int uEnd,int nKind);
		//读取事件信息,根据时间,通道,车牌
		void GetTraEvent(unsigned int uBegin,unsigned int uEnd);

		//检测器之间通讯，保存上一个点位发送过来的车牌信息（用于区间测速）
		int MvsCommunicationSaveRecPlate(RECORD_PLATE& plate);

		//检测器之间通讯，保存本点位和上一个点位之间的车辆区间速度相关信息（用于区间测速）
		int MvsCommunicationSaveRegionSpeed(RECORD_PLATE& plate);

		//监测器之间通讯，删除临时表中的内容
		int MvsCommunicationDelPlate(bool IsAll,string plateNum);

		//电科磁盘管理：根据卡口，违章，录像的路径删除对应的记录
		int DianKeDeleteOldRecord(string& strPath,int nType); //nType = 0:卡口，违章。1：录像
		//区间测速：删除已经存在的记录
		int RegionSpeedDeleteOldRecord(string strPath);
		//保存车牌信息
		int SavePlate(int nChannel,RECORD_PLATE& plate,unsigned int uPicId,SYN_CHAR_DATA* syn_char_data = NULL);
		//保存事件信息
		int SaveEventNoPlate(int nChannel,RECORD_PLATE& plate,unsigned int uPicId,SYN_CHAR_DATA* syn_char_data);
		//保存车牌信息无事件
		int SavePlateNoEvent(int nChannel,RECORD_PLATE& plate,unsigned int uPicId,SYN_CHAR_DATA* syn_char_data);
		// DSP相机，违章作为事件处理
		bool IsEvent(UINT32 uViolationType);

		//删除车牌信息
		int DelPlate(unsigned int uBegin,unsigned int uEnd,int nKind);
		//读取车牌信息,
		void GetPlate(unsigned int uBegin,unsigned int uEnd);
		//获取车牌区域大小
		String GetPlateRegion();
		//获取图片信息
		String GetPicInfoByID(unsigned int uID,int nKind);

		//utf8转gbk
		int UTF8ToGBK( std::string& str);
		//gbk转utf8
		int GBKToUTF8( std::string& str);

        //获取当前通道最新的图片编号
		unsigned int  GetLastPicId();

		//保存图片编号
        int SavePicID(unsigned int uPicId,unsigned int uVideoId = 0,int nType = 0);

		//读取车牌信息
		String GetCarNumHigh(SEARCH_ITEM_CARNUM& search_item_carnum,int nPageSize=0);

		//读取特征搜索信息
		String GetTextureHigh(SEARCH_ITEM_CARNUM& search_item_carnum, int nPageSize=0);

        //获取车牌大图后面的特征信息
        bool GetTextureFromPicFile(std::string &strPicPath, unsigned short* texture);

		//////////黑白名单
		int IsSpecialCard(String &strNum);
		int AddSpecialCard(SPECIALCARD& specialcard_item, int &nIndex);
		int DeleteSpecialCard(SPECIALCARD& specialcard_item);
		int ModifySpecialCard(SPECIALCARD& specialcard_item);
		String SearchSpecialCard(SPECIALCARD& specialcard_item);
		///////////

		//更新记录状态
		void UpdateRecordStatus(unsigned int uCmd,unsigned int uSeq,unsigned int nId = 1, bool bObject = false);
		//更新记录状态
		void UpdateRecordStatus(unsigned int uCmd,std::list<unsigned int>& listSeq,unsigned int nId = 1);

		//获取裁剪区域坐标
		RegionCoordinate GetClipRegionCoordinate(int nChannel);

        /*******************************/

        //从数据库读取指定通道信息
         void GetChannelInfoFromDB(int nCameraID, SRIP_CHANNEL &sChannel);
		 //通过通道ID从数据库中读取指定通道信息
		 bool GetChannelInfoFromDBById(int id, SRIP_CHANNEL &sChannel);

         //设置相机状态
         int SetCameraStateToDBByChannelID(int nChannelID, CameraState state);
         //获取相机状态
         int GetCameraState(int nCameraID);

         //zhangyaoyao: get user's right by name
         int GetRightByName(String strUserName);

         //zhangyaoyao:获取对应通道当前最大的event_id
         long mvGetLastEventId(int nChannelId);
         //zhangyaoyao:获取对应通道当前最大的车牌记录ID
         long mvGetLastCarNumId(int nChannelId);
         //zhangyaoyao:更新事件event_id关联的车牌记录ID
         bool mvUpdatePlateIdOfEvent(int nChannelId, long lnEventId, long lnPlateId);

         //特征比对查询
         String GetObjectFeature(SEARCH_ITEM& search_item, int nPageSize);

         //设置预置位
         int SetPreSet(int nPreSet,int nChannelId);
         //获取预置位
         int GetPreSet(int nChannelId);
         //获取检测类型
         CHANNEL_DETECT_KIND GetDetectKind(int nChannelId);

         //获取monitor编号--MONITOR_ID
        int GetMonitorID(int nChannelId);
        //获取相机编号
        int GetCameraID(int nChannelId);
        //获取地点
        String GetPlace(int nChannelId);
		//获取地点
		String GetPlaceByCamID(int nCameraId);
        //获取方向
        int GetDirection(int nChannelId);
        //根据相机编号获取通道编号
        int GetChannelIDByCameraID(int nCameraID);

        //获取车牌历史记录
        bool GetPlateHistoryRecord(StrList& strListRecord,int nDataType = 0, unsigned int nId = 1);

        //获取事件历史记录
        bool GetEventHistoryRecord(StrList& strListRecord, unsigned int nId = 1);

        //获取日志历史记录
        bool GetLogHistoryRecord(string &strRecord, unsigned int nId = 1);

        //获取统计历史记录
        bool GetStatisticHistoryRecord(string &strRecord, unsigned int nId = 1);

        //存储配置文件
        int SaveMonitorSettings(int nMonitorID,int nChannelID,int nKind);

        //获取录像文件列表
        unsigned short GetVideoFileList(string strBeginTime,string strEndTime,string& strPathList);

        //判断历史视频文件是否已经存在
        bool IsHistoryVideoDealed(String strPath);
        //保存历史视频文件列表
        bool SaveHistoryVideoInfo(int nDeviceID,String strPath,String strBeginTime,int uBeginMiTime,String strEndTime,int uEndMiTime,int nStatus);

        //获取远程录像视频名称
        StrList GetRemoteEventVideoPath(String strVideoPath);

		//获取录像时间
		String GetVideoTime(String strVideoPath,int nVideoType = 0);

		//获取相机IP
		string GetCameraIP(int channelId);
		//清空数据库BASE_PLATE_INFO
		bool ClearPlateInfo();
		//添加车牌数据到数据库BASE_PLATE_INFO
		bool AddPlateInfo(string strCarNumver);
		//在数据库BASE_PLATE_INFO中查询指定车牌，返回结果集中的车牌的编号。若string为空表示没有此车牌
		string FindPlateInfo(string strCarNumver);
		bool LoadPlateInfo(string strLoadDir, string strMysqlName);
		//获取BASE_PLATE_INFO中的所有车牌
		void GetAllPlate(list<string> &listBASE_PLATE_INFO, int nFirstId, int nCount);
		int GetId(string strTableName,int n = 1);

		//读入检测结果文件
		void LoadResult(char* filename,std::list<RECORD_PLATE>& listTestResult);

		//更新检测状态
		void UpdateDetectStatus(int nCameraID);

		bool SaveCamIpByID(string strCamIp, string strCamMultiIp, int nNewCamId, int nOldCamId);
		
		void GetCamIpByID(string &strCamIp, string &strCamMultiIp, int nCamID);

		void GetCamIpByChannelID(string &strCamIp, string &strCamMultiIp, int nChannelID);
		//获取相机编号
		int GetCameraSelfID(int nChannel);
		//批量设置检测类型
		bool UpdateDetectKind(int kind,int nChannelID = 0);

		//切换相机和预置位
		void UpdateCameraIDAndPresetID(int oldCameraID, int newCameraID, int nPresetID);

		//刷新通道信息
		bool UpdateChannelInfo(SRIP_CHANNEL sChannel);
		//替换通道信息
		bool UpdateSwitchChannelInfo(SRIP_CHANNEL sChannel, int nOldCameraID, int &channelId);
		//创建通道编号
		int CreateChannelId();

		//删除某段时间以前的所有记录
		int DeleteRecordByTime(string& strTime,int nType);

		unsigned int XingTaiGetLastPicId();

		//数据库导入
		bool DBImport();

		//数据库升级
		bool DBUpdate();

		//获取设备Id
		String GetDeviceId(UINT32 uCameraId);
		//根据通道号获取设备Id
		String GetDeviceByChannelId(UINT32 uChannelId);

		//报警枚举类型转换为十六进制
		void EnumConvertIntoHex(RECORD_LOG *ptr);

		//获取所有通道的相机状态
		int GetCameraStateByChannelId(UINT32 uChannelId);
//h264
 /*   int IfDiskFull(void);
    int check_disk();
    int FindAndDeleteSmallIndex(void);
    int DeleteVideo();
    std::string  GetEncodeFileName(){return strH264EncodeFileName;};*/
    int SaveVideo(int nChannel,UINT32 uBeginTime, UINT32 uEndTime,String strPath, int VideoType);
    //删除全天录像
    void DeleteVideoFile(int nCount);
public:
 /*   std::string strH264EncodeFileName; //保存编码文件名
    std::string strH264earliest_FileName; //保存当前时间编号最早的文件名
    std::string strH264earlier_FileName; //保存当前第二早的文件名
    int m_diskFull; //取值：0---磁盘不需要清理，1---需要清空一个文件，2---需要清空2个文件
    char saveDeleteDir[45];//备份要删除的空目录名
    bool m_delete_dir_flag; //=true:第一次清空文件； =false:不需要清空文件(默认初始值)
*/

private:

		//合法化字符串
		String RationStr(String strTemp,int nMaxLen);

		//读取视频记录数量
		int GetVideoCount(int nChannel,int nType, String strDateBegin,String strDateEnd);

		//读取车牌记录数量
		int GetCarNumCount(int nChannel,String strTimeBeg,String strTimeEnd,String strCarNum);

		//读取事件记录数量
		int GetTraEventCount(int nChannel,int nRoadIndex,int nType, String strDateBegin,String strDateEnd);

		//读取统计记录数量
		int GetStatisticCount(int nChannel, int nRoadIndex, int nType, String strDateBegin,String strDateEnd);

		//读取系统运行信息数量
		int GetSystEventCount(int nLevel,String strDateBegin,String strDateEnd);

		//获取记录序号
		unsigned int GetSeq(int nKind);

private:
        //数据库连接开关
		bool			m_bConnect;
		//日志记录序号
		unsigned int m_nLogSeq;
		//事件记录序号
		unsigned int m_nEventSeq;
		//统计记录序号
		unsigned int m_nStatisticSeq;
		//车牌记录序号
		unsigned int m_nPlateSeq;

		//utf8转gbk文件标识
		iconv_t m_nCvtUTF;
		//gbk转utf8文件标识
		iconv_t m_nCvtGBK;

		//上一次删除日期
		String m_strDeleteDate;
};

//全局DB
extern CSeekpaiDB g_skpDB;
#endif
