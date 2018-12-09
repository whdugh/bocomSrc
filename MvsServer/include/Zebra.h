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
#ifndef _ZEBRA_H_
#define _ZEBRA_H_

#include"AbstractCamera.h"

#define ZEBRA_IP "192.168.0.2"
#define ZEBRA_UDP_PORT 8881
#define ZEBRA_TCP_PORT 80
#define ZEBRA_TLE_PORT 23


//Initialize the camera to factory default settings:
#define Command_rev "set 000 80000000 \r\n"
//Power up the camera:
#define Command_power "set 610 80000000 \r\n"
//Set the frame rate to 15 fps:
#define Command_rate "set 600 60000000 \r\n"
//Set the mode to Mode_3:
#define Command_mode "set 604 60000000 \r\n"
//Set the format to 1600 X 1200 YUV 4:2:2:
#define Command_format "set 608 40000000 \r\n"
//Set 1E80
#define Command_compress "set 1E80 80000000 \r\n"
//Enable ISO stream:
#define Command_start "set 614 80000000 \r\n"
//Disable ISO stream:
#define Command_stop "set 614 00000000 \r\n"
//Setting White Balance
#define Command_wblance "set 80C 83000000 \r\n"
//Set the shutter:
#define Command_shutter "set 81C 83000000 \r\n"
//Setting Auto Gain
#define Command_gain "set 820 83000000 \r\n"
//Setting Auto Exposure
#define Command_exposure "set 804 83000000 \r\n"
//set sharpness
#define Command_sharp "set 808 83000000 \r\n"
//set satauation
#define Command_sataution "set 814 83000000 \r\n"
//set gamma
#define Command_gamma "set 818 83000000 \r\n"
//set brightness
#define Command_brightness "set 800 83000000 \r\n"
//锁定光圈
#define Command_IRS "set 824 820000C0 \r\n"


class Zebra :public AbstractCamera
{
	public:
	    //构造
		Zebra(int nCameraType,int nPixelFormat);
		//析构
		~Zebra();

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

        void CaptureFrame();
        //更新快表
        int UpdateLUT();
        //填写快表
        int SetTable();
        //读取快表寄存器
        int ReadTable();
        //生成快表
        int table(double a,double b,int d);
        //使用快表
        int UseTable();
        //关闭快表
        int CloseTable();



    private:
        //初始化
        void Init();
         //灯控制
		int LightControl(bool bInit = false);

		//相机控制
		int Control(CAMERA_CONFIG cfg);
        //接收
		int RecvData();

        //相机参数设置
        void SetCaMeraPara();
		void InitialGainAndSH();
		//将客户端设置的相机参数转换为相机自身能识别的参数
        virtual void ConvertCameraParameter(CAMERA_CONFIG& cfg,bool bReverseConvert,bool bFirstLoad = false);

        //连接
		int connect_tcp();
		int connect_udp();
       //控制和收取
		int cntl_get(char *buf);
		int cntl_set(char *buf);

        //控制
        int cntl_shutter_Abs(int nShutter);
		int cntl_gain_Abs(int nGain);
		int cntl_gamma_Abs(int nGamma);
		int cntl_brightness_Abs(int nBrightness);
		int cntl_saturtion_Abs(int nSaturation);
		int cntl_hue_Abs(int nHue);
		int cntl_mode(int nMode);
		//设置帧率
        int cntl_frameRate_Abs(int nFrameRate);
        //控制光圈
        int cntl_iris(int nIris);

		float Abs_cntl_get(char *buf);

		//控制灯光
 		int cntl_polarity(int pp);
 		int cntl_delay(int delay);
 		int cntl_duration(int duration);
 		int cntl_strobe(int s);
 		int get_light(char *buf);

 		//获取控制灯光参数
 		int get_polarity();
 		int get_delay();
 		int get_duration();
 		int get_strobe();
 		//获取工作方式
        int get_Mode();

		//数据
		int tcp_fd,udp_fd;

		//快表参数
       float tansfer[2048];

       //是否设置过查找表
       bool m_bSetTable;
};
#endif



