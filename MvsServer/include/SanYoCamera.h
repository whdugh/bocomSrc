// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _SANYOCAMERA_H_
#define _SANYOCAMERA_H_


#include"AbstractCamera.h"
#ifdef H264_DECODE
#include "RoadDecode.h"
#endif

#include "VmuxSocket.h"

/******************************************************************************/
//	描述:SANYO相机类.
//	作者:於锋
//	日期:2011-5-7
/******************************************************************************/

struct SYO_COMMAND
{
    short uLength;
    short uCommand;

    SYO_COMMAND()
    {
        uLength = 4;
        uCommand = 0x7001;
    }
};

struct ID_HEADER
{
    char chHNCV[4];
    short uLength;
    short uDataID;

    ID_HEADER()
    {
        uLength = 0;
        uDataID = 0;
        memset(chHNCV,0,4);
    }
};


class CSanYoCamera:public AbstractCamera
{
    public:

        CSanYoCamera(int nCameraType);
        ~CSanYoCamera();

        virtual bool Open();
		virtual bool Close();

		//手动控制
		virtual int ManualControl(CAMERA_CONFIG cfg);

		//自动控制
        virtual int  AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn);

        //切换工作方式
		virtual bool ChangeMode();

		//获取相机默认模板
        virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);

        //采集图像
        void CaptureFrame();
        //重新获取视频流
        virtual bool ReOpen();

    private:

        //初始化
        void Init();

        //相机控制
		bool Control(CAMERA_CONFIG cfg);

		//将客户端设置的相机参数转换为相机自身能识别的参数
        virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

         //建立连接
		bool connect_tcp();
		bool connect_udp();

        //关闭连接
		void close_tcp();
		void close_udp();

		//HTTPP 控制
        bool HttpControl();

		//UDP 控制
        bool UdpControl(SYO_COMMAND cmd);

        //发送tcp数据
        bool SendTcpData(char *buff);

        //接收tcp数据
        bool RecvTcpData(char *buff);

        //镜头远近缩放
        bool ZoomControl(CAMERA_CONFIG  cfg);

        //保存预置位(范围1-8)
        bool SetPreSet(int cmd);

        //读取预置位(范围1-8)
        bool GotoPreSet(int cmd);

		//获得通道地址信息
		void GetChannelPlace();

#ifdef VMUX_PROC
		int CheckSum(const VissHead *visHead);
#endif
    private:

		//数据
		int m_nTcpFd;
		int m_nUdpFd;

		//ip
        std::string m_strHost;
        //tcp port
        int m_nTcpPort;
        //udp port
        int m_nUdpPort;

        //用户名
        std::string m_strUserName;
        //密码
        std::string m_strPassWord;

        //用户名密码的base64编码
        std::string m_strBase64;

        //Cookie
        std::string m_strCookie;

		//GOP信息
		unsigned int m_uGop;

		//通道地址
		char m_channelPlace[128];

        #ifdef H264_DECODE
        RoadDecode m_Decoder;
        #endif
};
#endif
