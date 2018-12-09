// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef _ROSEEKCAMERA_H_
#define _ROSEEKCAMERA_H_

#include"AbstractCamera.h"

#include "CMDToCamera.h"
#include "GECamCfgInfo.h"

#define ROSEEK_IP "192.168.0.2"
#define ROSEEK_TCP_PORT 8881
#define ROSEEK_TCP_CONTROL_PORT 35001

class CRoSeekCamera:public AbstractCamera
{
    public:
        CRoSeekCamera(int nCameraType);
        ~CRoSeekCamera();

        virtual bool Open();
		virtual bool Close();
		//打开文件
        virtual bool OpenFile(const char* strFileName);

		//手动控制
		virtual int ManualControl(CAMERA_CONFIG cfg);
		//相机控制
        int Control(CAMERA_CONFIG  cfg);

		//自动控制
        virtual int  AutoControl(unsigned int uTimeStamp,float fRate,float fIncrement,bool bDetectCarnum,int nIris,int nEn);

		//读取
		virtual int ReadCameraSetting(CAMERA_CONFIG& cfg,bool bConvert = true);
		virtual bool ReOpen();
		//切换工作方式
		virtual bool ChangeMode();

        //获取相机默认模板
        virtual void GetDefaultCameraModel(CAMERA_CONFIG& cfg);

        //相机参数设置
        void SetCaMeraPara();

         //灯控制
		int LightControl(bool bInit = false);

        void CaptureFrame();

        CCMDToCamera			m_CMDToCamera;
        CGECamCfgInfo			m_GECamCfgInfo;

    private:
        //将客户端设置的相机参数转换为相机自身能识别的参数
        virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

    private:

        //初始化
        void Init();

        //接收
		void RecvData();

        //接收数据包
		bool SafeRecv(char*pBuf, int nLen);

        //发送数据包
		bool SafeSend(const char* pBuf, int nLen);

		//TCP 控制set
        //bool SendCMD(UINT32 dwAttribute, UINT32 dwMethod, const char *pParam, UINT32 dwParamLen);

        //连接
		bool connect_tcp();
		bool connect_udp();

		//数据
		int m_nTcpFd;
		int m_nUdpFd;

		//ip
        std::string m_strHost;
        //yuv port
        int m_nPort;
};
#endif
