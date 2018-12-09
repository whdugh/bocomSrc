#ifndef VISSERIALCOMMUNICATION_H
#define VISSERIALCOMMUNICATION_H

#include "AbstractSerial.h"
/**
    文件：VisSerialCommunication.h
    功能：Vis串口通讯类
    作者：於锋
    时间：2010-9-7
**/

class CVisSerialCommunication:public AbstractSerial
{
	public:
		CVisSerialCommunication();
		~CVisSerialCommunication();
		  //打开设备
        bool OpenDev();

         //镜头控制命令
        bool SendData(CAMERA_CONFIG& cfg);

        //发送命令到vis
        bool WriteCmdToVis(std::string& sCmdMsg);

	protected:

};
extern CVisSerialCommunication g_VisSerialCommunication;
#endif

