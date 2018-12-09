/* 版权所有 2013 上海博康智能信息技术有限公司
* Copyright (c) 2013，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvBocomDebug.h
* 摘要: 对系统进行调试的控制
* 版本: V1.0
* 作者: 贺岳平
* 完成日期: 2013年12月
*/ 
//////////////////////////////////////////////////////////////////////

#ifndef _MV_BOCOM_DEBUG_H_
#define _MV_BOCOM_DEBUG_H_

#include "libHeader.h"
#include "BaseComStruct.h"


//进行打印设置
enum MvEnumPrintf{
	INIT_PRINTF = 0,

	PRINTF_EVNET_MAIN_STEP, //打印事件的主要步骤
	
	OTHER_PRINTF
};

//进行敏感设置
enum MvEnumSensitivenss{
	INIT_SEN = 0,

	SEN_FIRE_DET,	  //火检测是否敏感
	SEN_SMOKE_DET,    //烟检测是否敏感
	SEN_SHEET_DET,    //撒传单检测是否敏感
	SEN_BILL_DET,     //打条幅检测是否敏感
	SEN_JAM_DET,      //聚集检测是否敏感
	SEN_RUN_DET,      //奔跑检测是否敏感
	SEN_DERELICT_DET, //遗弃物检测是否敏感

	OTHER_SEN
};

//进行调试信息保存设置
enum MvEnumDebugInfoSave{
	INIT_DEBUG_INFO_SAVE = 0,

	FIRE_DET_SAVE,	 //火检测是否敏感
	SMOKE_DET_SAVE,  //烟检测是否敏感

	OTHER_DEBUG_INFO_SAVE
};


//调试信息结构体
typedef struct StructDebugInfo
{ 
	int  nPrintfEnum;  //信息模式

	string  strInfo; //调试信息

	StructDebugInfo( )
	{
		nPrintfEnum = -1;
	}
}MvDebugInfo;


//用于调试字符串列表结构
#define  MAX_TEMP_LINE_CNT 1024  //最多行数
typedef struct MvStruStringList
{
public:	
	MvStruStringList( );

	~MvStruStringList( );

	//初始化
	void mvInitStruStringList( 
			int nMaxLineCnt,       //IN:最多的行数
			char *pFileName = NULL //IN:保存的文件名
		);

	//将信息压入列表中
	bool push_string( const char *chTempInfo, 
					  int nPrintfEnum = -1 );

	//将所有的字符串，进行保存
	bool mvSaveAllString( );

public:
	//注意最多控制在N行,每行104个字符
	int   m_nTempCnt;  
	char  **m_chTempInfo;

private:
	//初始化变量
	void mvInitVar( );	

	//释放
	void mvUninitStruStringList( );

private:
	bool m_bInit;   //是否初始化过

	FILE *m_fp_w;   //保存打印信息的文件句柄
	char *m_pFileName; //保存打印信息的文件名

	vector<MvDebugInfo> *m_pVectString;	//指向字符串的存储器

	vector<MvDebugInfo> m_vectString0;  //字符串的存储器1
	vector<MvDebugInfo> m_vectString1;  //字符串的存储器2

	int m_nMaxLineCnt;		//最多行数
	int m_nNowLineIdx;		//当前的行序号

}StruStringList, MvDebugInfoList;


//-------------------屏幕信息打印配置的读入-----------------//
#define MAX_PRINT_CFG_CNT 100

enum MvEnumInfoSavePrintfMod
{
	NORMAL_PRINTF,  //正常打印
	FLUSH_PRINTF,	//强制打印--用于异常定位  
	ERROR_PRINTF    //错误打印--用于异常定位，且不顾应用端不支持屏幕打印
};

typedef struct StructPrintfCfg 
{
public:
	StructPrintfCfg( );
	~StructPrintfCfg( );

	void mvGetPrintfConfig( char *pSaveFileName ); //获取屏幕打印的配置
	
	//打印和保存调试信息
	bool mvPrintfAndSaveInfoToList(
			const char *chTempInfo, //IN:当前打印信息
			int nPrintfEnum //IN:打印信息所对应的模式
		);

	//将所有的字符串，进行保存
	bool mvSaveAllString( )
	{		
		return m_DebugInfoList.mvSaveAllString( );
	}

	bool m_bAPrintf[MAX_PRINT_CFG_CNT];  //屏幕打印项

	int  m_nInfoPrintfMod;       //信息打印所采用的模式
	bool m_bSavePrintfInfoAsTxt; //是否将打印信息保存为文本

	MvDebugInfoList m_DebugInfoList;  //调试信息列表

private:
	void mvInitVar( );

	void mvGetEventPrintfConfig( ); //获取事件的打印配置

}MvPrintCfg;


//-------------------敏感检测配置的读入-----------------//
#define MAX_SEN_CFG_CNT 50
typedef struct StructDebugSensCfg 
{
public:
	StructDebugSensCfg( );
	~StructDebugSensCfg( );

	//获取敏感的配置
	void mvGetSensitivenessConfig( );

public:
	bool  m_bSenStatus;   //程序是否为敏感状态
	bool  m_bASens[MAX_SEN_CFG_CNT];  //各检测项是否为敏感模式

private:
	void mvInitVar( );

}MvDebugSensCfg;


//-------------------调试信息保存配置的读入-----------------//
#define MAX_SAVE_CFG_CNT 100
typedef struct StructDebugInfoSaveCfg 
{
public:
	StructDebugInfoSaveCfg( );
	~StructDebugInfoSaveCfg( );

	//获取调试信息保存的配置
	void mvGetDebugInfoSaveConfig( );

public:
	bool m_bSaveStatus;    //程序是否为保存状态
	bool m_bInCfgSaveTime; //是否在配置好的保存时间段内

	bool   m_bASave[MAX_SAVE_CFG_CNT];    //进行保存的配置项

	float  m_fACfgSaveInterval[MAX_SAVE_CFG_CNT]; //保存间隔
	double m_dATsLastSave[MAX_SAVE_CFG_CNT]; //上次保存的时间戳

private:
	MvDateStruct m_DateInit;
	MvDateStruct m_DateNow;

private:
	void mvInitVar( );

	void MvIsSaveStatus( );   //是否为保存状态
	void MvInSaveDateTime( ); //是否在保存时间段

	void mvGetGongYongXiangSaveConfig( ); //各公用项的保存配置
	void mvGetGeZiXiangSaveConfig( );     //各子项的保存配置

}MvDebugInfoSaveCfg;



//--------------------------调试配置的读入-------------------------//
typedef struct StructBocomDebugCfg 
{
public:
	StructBocomDebugCfg( ){ };
	~StructBocomDebugCfg( ){ };

	void mvGetDebugConfig( char *pLogSaveFileName );

public:
	MvPrintCfg		m_PrintfCfg;    //信息打印配置
	MvDebugSensCfg  m_SensDetCfg;   //敏感检测配置
	MvDebugInfoSaveCfg  m_SaveCfg;  //调试信息保存配置

private:
	void mvInitVar( );

}MvBocomDebugCfg;



#endif //_AN_IMGPEO_H_
