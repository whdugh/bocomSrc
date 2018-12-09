// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef ROAD_MONITOR_H264_H
#define ROAD_MONITOR_H264_H

#include "global.h"
#include "CFmfsTel.h"

#ifdef H264_DECODE
#include "RoadDecode.h"
#endif

#define FMFS_MONITOR

typedef std::map<int,int> FileIndexMap;

/******************************************************************************/
//	描述:监控系统视频接入类
/******************************************************************************/

class CRoadMonitorH264
{
    public:
        //构造
        CRoadMonitorH264();
        //析构
        ~CRoadMonitorH264();

    public:
        //打开设备,设备编号
        bool OpenVideo(int nNo,VOD_FILE_INFO& vod_info,int nVideoType=0/*是否实时视频*/);
        //关闭设备
        bool CloseVideo();

        //开始采集数据
        bool StartCapture();
        //停止采集数据
        bool StopCapture(bool bSaveVideoInfo = false);

        void DealMonitorCapture();

        //启动视频流
        int64_t PullStream(int nIndex =0);

        //获取视频源类型
        int GetVideoType() { return m_nVideoType;}

        //获取指定相机指定时间段内的文件列表
        int GetFileCount();

        //
        void ReOpen();

        //添加一帧
        void AddFrame(unsigned char* pData,int dataLen,long long pts,bool bKeyFrame = false);
        //获取一帧数据
        int PopFrame(yuv_video_buf& header,unsigned char* response);
        //修改缓冲队列长度
        //void DecreaseSize();

        //设置视频信息
        void SetVideoInfo(yuv_video_buf* pVideoInfo);
        //获取视频信息
        yuv_video_buf* GetVideoInfo();

        //获取视频编号
        void GetDeviceId();
		//获取FMFS接收数据端口
		 static int GetFmfsPort()
		 {
			 return m_nFmfsPort++;
		 }
private:
	TimeStamp_t MakeTimeStamp(unsigned int uTime);

    private:

        int m_nDeviceId;
        unsigned int m_nUnitDeviceId;

        void* m_handle;

        //信号互斥
        pthread_mutex_t m_Frame_Mutex;
        //列表
        //std::list<std::string> m_FrameList;
        std::list<std::string> m_listFrame;

        yuv_video_buf m_VideoInfo;

        FileIndexMap m_mapFileIndex;

        #ifdef H264_DECODE
        RoadDecode m_Decoder;
        #endif
        bool m_bEndCapture;
        //线程ID
        pthread_t m_nThreadId;

        std::string m_strFileName;

        VOD_FILE_INFO m_vod_info;
        //实时还是历史视频
        int m_nVideoType;//0:实时视频，1：远程历史视频，2：本地历史视频

        int64_t m_pts; //记录历史视频

		#ifdef FMFS_MONITOR
		  CFmfsTel m_fmfsTel;
		#endif
	  //FMFS接收数据端口
       static int m_nFmfsPort;

	   //pixel格式
	   PixelFormat m_nPixelFormat;
};
#endif
