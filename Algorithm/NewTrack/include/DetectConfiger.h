//检测项配置
#ifndef _AN_BG_JUDGE_H_
#define _AN_BG_JUDGE_H_

#include "libHeader.h"

#ifndef LINUX
//	#define CREATE_DETCONF_FILE  //创建检测配置文件
	#ifdef CREATE_DETCONF_FILE
		//#define	CREATE_UNNORMAL_STOP_FILE      //创建异常停车检测配置文件
		//#define	CREATE_TIME_CTRL_4SAVE_FILE    //创建通过时间控制来保存信息的文件

		//#define	CREATE_DETSTOP_SAVE_CTRL_FILE       //创建停车检测保存的控制文件
		//#define	CREATE_LOSSDETCHECK_SAVE_CTRL_FILE  //创建存图来查检测是否遗漏的控制文件
		//#define	CREATE_FKIMGCHECK_SAVE_CTRL_FILE    //创建存图来查前景图的控制文件
		//#define	CREATE_BGIMGCHECK_SAVE_CTRL_FILE    //创建存图来查背景图的控制文件
	#endif
#endif

#ifndef AN_CFG_READ_MOD
	#define AN_CFG_READ_MOD
	enum
	{
		READ_ON_INIT = 0,
		READ_ON_EVERYTIME
	};
#endif


//是否进行***检测的配置
#ifndef AN_CFG_SOME_DETECT
#define AN_CFG_SOME_DETECT
	typedef struct an_cfg_someDet_stru  
	{
	public:
		bool bDetect;

	public:
		an_cfg_someDet_stru( );		

		void initVar( );
		
		void readConfig( char *pChCfgFileName );
	
	}AnCfgSomeDetStru;
#endif


//读入保存的时间控制
#ifndef AN_TIME_CTRL_FOR_SAVE
	#define AN_TIME_CTRL_FOR_SAVE
	typedef struct an_timeCtrl4saveStru  
	{
	private:
		FILE  *fp_r;	//读文件指针
	#ifdef CREATE_DETCONF_FILE
		FILE  *fp_w;    //写文件指针
	#endif
		char *pChCfgFileName;    //文件名称
	
	public:
		bool bTimeCtr;    //是否进行时间控制

		//开始的日期和时间
		int  nSYear, nSMon, nSDay, nSHour, nSMin, nSSec;  

		//结束的日期和时间
		int  nEYear, nEMon, nEDay, nEHour, nEMin, nESec;  

	public:
		an_timeCtrl4saveStru( );
		
	#ifdef CREATE_DETCONF_FILE
		void writeConfig(  );
	#endif

		void initVar( );	

		void setConfigFileName( char *_pChFileName );

		void readConfig(  );		

		//给定的日期和时间在配置范围内
		bool giveDateTimeInCfg(
				int nYear, int nMon, int nDay,
				int nHour, int nMin, int nSec ); //当前的日期和时间

	}AnTimeCtrl4SaveStru;
#endif
	
//存信息控制配置
#ifndef AN_SAVE_CTRL
	#define AN_SAVE_CTRL
	typedef struct an_save_ctrl_stru  
	{
	private:
		FILE *fp_r;     //读文件指针
	#ifdef CREATE_DETCONF_FILE
		FILE  *fp_w;    //写文件指针
	#endif
		char *pChCfgFileName;    //文件名称

	public:
		bool bSave;				 //是否存图非正常停车
		int  nSaveFreq;          //存图的间隔(单位为毫秒) 
		double dSaveFre;		 //存图的间隔(单位为秒) 

	public:
		an_save_ctrl_stru( );		

		void setConfigFileName( char *_pChFileName );
	
	#ifdef CREATE_DETCONF_FILE
		void writeConfig( );
	#endif

		void initVar( );

		void readConfig( );

	}AnSaveCtrlStru;
#endif

class CDetectConfiger
{
public:
	CDetectConfiger(void);
	~CDetectConfiger(void);

	void mvInitDetectConfiger( );
	void mvInitDetectConfigerVar(  );

	void mvUninitDetectConfiger( );

public:
	AnCfgSomeDetStru  m_cfgUnnormalStop;  //异常停车检测配置

	//存图和信息的控制配置
	AnTimeCtrl4SaveStru m_cfgTimeCtrl4Save; //控制只在指定时间段内保存信息

	AnSaveCtrlStru m_ctrlSave4DetStopCar;   //控制:保存检测到的停车
	AnSaveCtrlStru m_ctrlSave4LossDetCheck; //控制:存图来查是否遗漏检测
	AnSaveCtrlStru m_ctrlSave4FkImgCheck;	//控制:存图来查前景图
	AnSaveCtrlStru m_ctrlSave4BgImgCheck;   //控制:存图来查背景图

};



#endif