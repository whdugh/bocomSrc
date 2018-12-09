//进行调试的结构体
#ifndef _MV_READ_DEBUG_CFG_H_
#define _MV_READ_DEBUG_CFG_H_

//头文件
#include <string>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>

#ifndef LINUX
	#include <io.h>
	#include <assert.h>
#endif

#include <vector>

#include "SmallVaildStruct.h"

using namespace std;

#define  MAX_DEBUG_CFG_CNT 100   //最多的调试项

//调试的各选项
enum ENUM_DEBUG_OPTITION
{ 
	ENUM_DEBUG_GRID_STOP = 0,   //网格停车法检测
	ENUM_DEBUG_BANK_PEOAPP,		//银行行人出现检测
	ENUM_DEBUG_TRAIFFIC_JAM,	//交通拥堵检测
	ENUM_MAX_DEBUG_CFG_CNT      //最大的调试配置项
};

//调试配置读入器
typedef struct MvDebugCfgReader
{ 
public:
	MvDebugCfgReader( )	{ mvInitVar( ); }

	//初始化变量值
	void mvInitVar( );

	//获取调试选项
	bool mvGetDebugConfig(
			char* chDir,		//当前工作目录下的子目录
			char* chCfgFileName //读入的文件名称不带后缀(后缀为.cfg) 
		);

	//判断给定的调试项当前是否可以保存
	bool mvIsCanSaveNow(
			bool bTimeRequire,	  //是否有时间要求的约束
			int nEnumDebug,		  //配置项的枚举值
			double dTsNow,		  //当前的时间戳
			bool bWriteLog=false  //是否写log
		);

	//设置保存的时间戳
	void mvSetSaveTimeStamp( 
			int nEnumDebug,  //配置项的枚举值
			double dTsNow    //当前的时间戳
		);

private:
	int    m_nACfgOption[MAX_DEBUG_CFG_CNT];  //配置项
	float  m_fACfgSaveInterval[MAX_DEBUG_CFG_CNT];   //保存间隔
	double m_dATsLastSave[MAX_DEBUG_CFG_CNT]; //上次保存的时间戳

	char   m_debugCfgDir[104];	 //调试配置目录
	vector<string> m_vStringInfo; //信息的列表

private:

	//读取配置时判断字段的类型
	int mvCharToInt( char flag[] );

}MvDebugCfgRead;



#endif