#ifndef CAMERASERIAL_H
#define CAMERASERIAL_H

#include "AbstractSerial.h"
typedef unsigned char   uint8;		// 无符号8位字符
//typedef unsigned short  uint16;	// 无符号16位整型变量

class CamerSerial:public AbstractSerial
{
	public:
		CamerSerial();
		~CamerSerial();
		  //打开设备
        bool OpenDev();

		//打开相机控制网口
		bool OpenDev(string strIP, int nPort);

        //获取相机参数
        bool GetMessage(CAMERA_CONFIG& cfg);
        //设置相机参数
        bool SendMessage(CAMERA_CONFIG cfg,CAMERA_CONFIG& cam_cfg,CAMERA_MESSAGE kind);
		//通过串口设置相机参数（柯达相机用）
		int SendMessageByKodak(CAMERA_CONFIG cfg,CAMERA_CONFIG& cam_cfg,CAMERA_MESSAGE kind,int nMethod,int nCamId = 0);
		//校验和函数
		unsigned short msg_drv_create_crc(uint8 * pData , unsigned short len);
		//相机升级
		int UpdateCam(int type, int Index);
		//获取相机升级所需文件
		int GetUpdateFile(FILE* fp, int& nSize, string strPath);
		//获取相机版本号
		void GetVersion();
		//永久保存参数
		void SaveCamPara(int nCamID);
		//设置相机IP地址(视频)
		int SetIPAddress(string strIP,int nCamId);
		//获取相机IP地址(视频)
		int GetIPAddress(string &strIP,int nCamId);
		//设置相机IP地址(视频)
		int SetControlAddress(string strIP,int nCamId);
		//获取相机控制地址
		int GetControlAddress(string &strAddr,int nCamId);
		//获取组播地址(视频)
		int GetAddress(string &strAddr,int nCamId);
		//设置组播地址(视频)
		int SetAddress(string strAddr,int nCamId);
		//设置相机ID
		void SetCamId();
		//获取相机ID
		int GetCamId();

		void ChangeMode(int nMode,int nCamID);

		//
		void SetControlIP(string strIP) {m_strControlIP = strIP;}

	protected:
		

	private:
		string m_strVersion;//相机版本
		string m_strIP;//相机IP
		pthread_t m_nThreadId;
		string m_strControlIP;
};
extern CamerSerial g_CameraSerialComm;
#endif
