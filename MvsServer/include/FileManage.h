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

#ifndef SKP_ROAD_FILEMANAGE_H
#define SKP_ROAD_FILEMANAGE_H


#include "global.h"

/******************************************************************************/
//	描述:智能交通检测系统文件管理类
//	作者:於锋
//	日期:2008-11-03
/******************************************************************************/

class CFileManage
{
public:
	//构造
	CFileManage();
	//析构
	~CFileManage();

public:

    //获取事件录像路径
    std::string GetEventVideoPath(unsigned int&  uVideoId);
    //获取车牌图片路径
    std::string GetPicPath();
	std::string GetSpecialPicPath(int nType);

    //检查硬盘使用率并获取图片或录象文件编号
    void CheckDisk(bool bEvent,bool bVideo);

    //删除图片及录象文件
    void RemoveFile(bool bVideo,int nMaxCount,time_t *fileTime = NULL);
/*
    //删除超过最大数量的图片文件(500G硬盘)
    bool RemoveExtendFile(int nMaxCount);
*/
	//获取全天录象文件名称
	std::string GetVideoPath();
    //获取正在录像的文件名称
	std::string GetEncodeFileName(){return m_strH264FileName;};

    //获取指定目录下的最大编号
	int GetMaxIDFromPath( const char* dirname);

	//获取最大图片或录像编号
    UINT32 GetMaxID(bool bVideo);

     //获取指定目录下的最小编号
	int GetMinIDFromPath( const char* dirname,int nIndex);

	//获取最小图片或录像编号
    UINT32 GetMinID(bool bVideo);

	//取得最旧的全天录像时间
	void GetOldDayVideoTime(time_t *fileTime);
	//取得最旧的文件时间
	void GetOldTime(bool bVideo, time_t *fileTime);

	//初始化
	bool Init();
	//释放
	bool UnInit();
	//启动磁盘管理线程
	bool BeginManage();
	//结束磁盘管理线程
	bool EndManage();
	//运行磁盘管理线程
	void RunManage();

	//磁盘清理
    void CleanUpDisk();
	//获取以数字文件命名的文件列表,文件名称升序排列
    int GetDigitalPathList(const char *dirname, vector<long> &pathList);

	//取Dsp相机视频文件路径 格式: ./video/channel_id/20081104/10/10/vod10_100.mp4
	std::string GetDspVideoPath(int nChannelId);

	//检测器之间通讯，获取用于存储从上一个点位发送过来的图片的路径(用于区间测速)
	std::string GetRecvPicPath();

	//函数介绍：获取Dsp事件录像路径：(格式./video/channel_1/sub1/sub2/100.avi)
	std::string GetDspEventVideoPath(unsigned int&  uVideoId);
	//获取正在录像的文件名称
	std::string GetDspEncodeFileName(){ return m_strDspH264FileName; }

	//通过相机编号，取视频文件路径 格式: ./video/channel_id/20081104/10/10/11_22.mp4
	std::string GetMulDspVideoPath(UINT32 uCamId);

	//取缓存录像路径,格式:./video/temp/00-01~59.mp4 [Hour-ChannelID]
	std::string GetDspVideoTempPath(int nChannelId);

protected:
   static void * ThreadOptimizeTable(void *param);
   static void * ThreadBmpManage(void *param);
//私有变量
private:
	//线程ID
	pthread_t m_nThreadId;
	//当前删除的图片序号(当磁盘空间>=95%)
	long m_nCurDeletePicID;
    //当前删除的录像序号(当磁盘空间>=95%)
	long m_nCurDeleteVideoID;
	 //全天录像文件名
    std::string m_strH264FileName;
    //500G硬盘对应的最大图片编号
    UINT32 m_uExtendMaxPicID;
    //500G硬盘对当前删除的图片编号
    UINT32 m_uExtendPicID;
    //500G硬盘超过最大数量的最小图片编号
    UINT32 m_uExtendMinPicID;
	//优化数据库表线程
	pthread_t m_nThreadTableId;
	//BMP文件管理线程
	pthread_t m_nThreadBmpId;
	//图片及录像所占用磁盘空间的检测线程
	pthread_t m_nThreadCleanDskId;

	//最近一次删除事件录像的时间
	time_t m_tDeteteVideoTime;
	//最近一次删除事件图片的时间
	time_t m_tDetetePicTime;

	//查找及删除操作对图片序列的循环次数
	UINT32 m_uRemovePicLoopCount;
	//查找及删除操作对录像序列的循环次数
	UINT32 m_uRemoveVideoLoopCount;
	//取得图片ID的循环次数
	UINT32 m_uGetPicIdLoopCount;
	//取得录像ID的循环次数
	UINT32 m_uGetVideoIdLoopCount;

	//违章录像文件名
	std::string m_strDspH264FileName;
};
extern CFileManage g_FileManage;
#endif
