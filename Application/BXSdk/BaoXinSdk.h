/************************************************************************/
/* 
	接入宝信平台协议SDK。
	add_by mawei 20150906
*/
/************************************************************************/


#ifndef _BAOXINSDK_H_
#define _BAOXINSDK_H_

#include "BXTypes.h"

//初始化
BX_Int32 BX_Init();
//清理
BX_Int32 BX_UnInit();
//设置获取状态回调，用于获取设备状态信息
BX_Int32 BX_Reg_CB_FUNS( BX_CB_FUNS cb_funs );

//发送图片信息,返回值参考BX_ERRNO
BX_Int32 BX_SendImage( const BX_HeadData *pbxHead,const BX_Uint8 *JpegData, BX_Uint32 picLen);

#endif //_BAOXINSDK_H_
