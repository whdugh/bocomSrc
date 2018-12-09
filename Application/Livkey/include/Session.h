/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：Session.h
* 摘要: usbkey接口
* 版本: V2.0
* 作者: 於锋
* 完成日期: 2010年1月28日
*/

#ifndef USB_SESSION_H
#define USB_SESSION_H

#include	"PORT.h"
#include	"OWERROR.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif
	//打开虚拟串口
    int OpenUsbMaxim(int port);
    //打开虚拟串口
    int OpenUsb(int port);
	//关闭虚拟串口
    int CloseUsbMaxim();
    //关闭虚拟串口
    int CloseUsb();
    //获取随机数
    uint8 GetRanData(uint8* RanData);
    //获取本地信息
    uint8 GetLocalInfo(uint8* RanData,uint8* resLocal);
    //比对本地和服务器结果
    uint8 CompareLocalAndServer(uint8* resLocal,uint8* resServer);
    //写入默认日期
    uint8 WriteDefaultDate(int nDay);
    //写入日期
    uint8 WriteDate(char* chPicPath,time_t uTime,uint8* date);
    //检查日期
    uint8 CheckDate(char* chPicPath,time_t uTime,uint8* picdate,int nIndex);
    //获取日期
    uint8 GetDate(uint8* date);
    /////////////////////////////////////////
    //获取随机数
    uint8 GetData(uint8* Ran);
	//比较认证结果
    uint8 CompareLocalInfoMaxim();
    //比较认证结果
    uint8 CompareLocalInfo();

    /************for usbkeyburn*************/
    uint8	GetRomId(uint8* id);
    uint8	SetSecret(uint8,uint8* secret);
    uint8	Authentication(uint8,uint8,uint8,uint8*,uint8*);
    uint8 	ReadMemo(uint8, uint8, uint8, uint8, uint8*);
    uint8	WriteMemo(uint8, uint8, uint8, uint8, uint8, uint8*);
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
