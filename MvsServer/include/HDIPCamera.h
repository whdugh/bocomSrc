// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _HDIPCAMERA_H_
#define _HDIPCAMERA_H_

#include"AbstractCamera.h"
#ifdef H264_DECODE
#include "RoadDecode.h"
#endif


typedef struct
{
    UINT32  nCodeId;//0x00000001表示视频请求
    UINT32	nDeviceId; //码流号(即相机编号)
}HDIP_Header_t;


typedef struct
{
    char cSynchron[4];//同步头，由4个’$’字符组成
	int64_t timeStamp;//帧数据时间戳(单位:毫秒)
    UINT32 width;//视频宽度(1920)
    UINT32 height; //视频高度(1080)
    UINT32 nFlag;//0:非关键帧，1：关键帧
    UINT32	length; //数据包体长度(帧数据长度)
}HDIP_Frame_t;



class CHDIPCamera:public AbstractCamera
{
    public:
        CHDIPCamera(int nCameraType);
        ~CHDIPCamera();

        virtual bool Open();
		virtual bool Close();
		//打开文件
        virtual bool OpenFile(const char* strFileName);

		//手动控制
		virtual int ManualControl(CAMERA_CONFIG cfg);

		//自动控制
        virtual int  AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn);

		//读取
		virtual int ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert = true);
		virtual bool ReOpen();
		//切换工作方式
		virtual bool ChangeMode();

        //获取相机默认模板
        virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);
		
		//设置设备编号
        virtual void SetDeviceID(int nDeviceID);

        //解码
        bool DecodeFrame(unsigned char* pData,int dataLen,unsigned int pts,int iFrameRate,int nKeyFrame);

		//h264图像采集
		void CaptureFrame();

		//心跳信号
		void LinkTest();

    private:
        //初始化
        void Init();

        //连接
		bool Connect();

		//相机控制
		bool Control(CAMERA_CONFIG cfg);

		//获取相机参数
        bool Cntl_Get();
        //设置相机参数
        bool Cntl_Set();

        int GetSH(int nSH,bool bReverseConvert);

        //将客户端设置的相机参数转换为相机自身能识别的参数
        virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

        //相机参数设置
        void SetCaMeraPara();

		//TCP 连接
		bool connect_tcp();

		//接收相机命令
		UINT32 RecvCMD();

		//发送命令给相机
		bool SendCMD(UINT32 uCmd);

    private:
        void* m_hObject;

        void* m_hControlObject;

        #ifdef H264_DECODE
        RoadDecode m_Decoder;
        #endif

        int m_nDecodeCount;

		//tcp连接
		int m_nTcpSocket;

		//相机编号
		int m_nDeviceId;

		//视频请求成功
		bool m_bRequestStatus;

		//心跳线程id
		pthread_t m_nLinkTestId;

};
#endif
