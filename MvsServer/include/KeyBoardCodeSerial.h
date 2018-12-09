// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#ifndef KEYBOARDCODESERIAL_H_INCLUDED
#define KEYBOARDCODESERIAL_H_INCLUDED

#include "global.h"

class CKeyBoardCodeSerial
{
public:
    CKeyBoardCodeSerial();
    ~CKeyBoardCodeSerial();

    void SetMonitorID(int nMonitorID);//设置监视器ID
    void SetCameraID(int nCameraID);//设置摄像机ID
    void SetPreSetID(int nPreSetID);//设置预置位ID
    void SetKeyBoardID(int nKeyBoardID);//设置键盘ID

    std::string GetKeyHeadCode(); //获取键前端码
    std::string GetKeyCode(const char chInput); //获取单字符键盘码字符串
    std::string GetControlCode(CAMERA_CONFIG& cfg, bool bSwitchFlag, int nDownFlag,int nSpeed = 5); //设置控制字符串码

    std::string GetNumberCode(int nInputNum); //获取整数字符串码

private:
    int m_nMonitorID; //监视器ID
    int m_nCameraID; //摄像机ID
    int m_nPreSetID; //预置位ID
    int m_nKeyBoardID; //键盘ID
};

//extern KbdSerial g_KdbSerial;

#endif // KEYBOARDCODESERIAL_H_INCLUDED
