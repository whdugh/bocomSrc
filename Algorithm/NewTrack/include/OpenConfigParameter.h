//对外开放的参数
#ifndef __OPEN_CONFIG_PARAMETER_H
#define __OPEN_CONFIG_PARAMETER_H
	
#include "libHeader.h"

//事件统计和报警配置参数的阅读器
#define MAX_CFG_PARA_FILE_CNT 30  //最多的参数配置文件数
enum DET_CFG_PARM_ENUM  //不要超过MAX_CFG_PARA_FILE_CNT
{
	QUEUE_LEN_STAT_PC = 0,   //队列长度统计
	JAM_ALERT_PC,			 //车道拥堵报警
	ACCIDENT_STOP_ALERT_PC,   //事故停车报警
	VEHICLE_STOP_ALERT_PC,    //停车报警
	AGAINST_MOVE_ALERT_PC,    //逆行报警
	FAST_MOVE_ALERT_PC,		  //超速报警
	CHANGE_ALERT_PC,		  //变道报警
	CROSS_MOVE_ALERT_PC,	  //横穿报警
	SAMEPOINT_MAX_OFFSET_PC   //同一点的最大偏移率
};

#define MAX_DET_PARA_CNT 10  //最多配置的检测参数数目


//---------part1----------//
typedef struct StruDetectParameterCfg
{
public:
	StruDetectParameterCfg( );

	bool mvInitDetectParaCfg(
			int nChannelIdx,    //通道的序号
			int nPresetIdx,     //预置位的序号
			char *cTxtFileName  //文件的名称
		);

	float* mvGetCfgPara( int &nParaCnt );

private:
	bool m_bInit;         //是否初始化
	bool m_bReadSucc;     //是否读入了

	char m_cFileName[104];  //读写的文件名称

	int m_nChannelIdx;      //通道的序号
	int m_nPresetIdx;       //预置位的序号

	int   m_nCfgDataCnt;				  //检测配置数据的总个数
	float m_fACfgData[MAX_DET_PARA_CNT];  //检测配置的具体数据

private:
	void mvInitCfgParaVar( );


	//将检测参数写入文件
	bool mvWriteCfgParaToFile( );

	//从文件中读入检测参数
	bool mvReadCfgParaFromFile( );  

}AnDetectParameterCfg;


//---------part2----------//

//队列长度
struct StructQueueLenStaCfgPara
{
public:
	bool m_bMontionConstrain4QL;  //是否运动约束,默认false

	StructQueueLenStaCfgPara( );
	void mvInitVar( );
};

//拥堵检测
struct StructJamAlertCfgPara
{
public:
	bool m_bMontionConstrain4Jam;  //是否运动约束,默认false

	StructJamAlertCfgPara( );
	void mvInitVar( );
};

//事故停车检测
struct StructAccidentStopAlertCfgPara
{
public:
	bool m_bDetAccidentStop;  //是否需对检测事故停车
	
	StructAccidentStopAlertCfgPara( );
	void mvInitVar( );
};

//停车检测
struct StructVehicleStopAlertCfgPara
{
public:
	bool m_bStrictObjType;   //是否需对目标的类型严格要求,默认true
	bool m_bConfirmObjExist; //是否需对存在目标进行确认,默认false

	StructVehicleStopAlertCfgPara( );
	void mvInitVar( );
};

//逆行检测
struct StructAgainstMoveCfgPara
{
public:
	bool m_bDetectSmallObject;   //是否需对小目标检测逆行,默认true
	bool m_bJugeObjIsLight;		 //是否判断目标为灯光,默认为false

	StructAgainstMoveCfgPara( );
	void mvInitVar( );
};

//超速检测
struct StructFastMoveCfgPara
{
public:
	bool m_bJugeFastObjIsLight;		 //是否判断目标为灯光,默认为false

	StructFastMoveCfgPara( );
	void mvInitVar( );
};

//变道检测
struct StructChangeCfgPara
{
public:
	bool m_bUseEasyMode;		 //是否使用宽松模式检测,默认为true
	bool m_bDetectInBottom;      //是否对图像独步进行变道检测，默认为true
	StructChangeCfgPara( );
	void mvInitVar( );
};

//横穿检测
struct StructCrossMoveCfgPara
{
public:
	bool m_bUseWorldDist;		 //是否使用世界坐标约束检测,默认为true

	StructCrossMoveCfgPara( );
	void mvInitVar( );
};

//同一点的偏移
struct StructSamePtOffsetCfgPara
{
public:
	float m_fOffRateWidth;	   //宽度方向的最大偏移比率,默认为0.002
	float m_fOffRateHeight;    //高度方向的最大偏移比率,默认为0.002

	StructSamePtOffsetCfgPara( );
	void mvInitVar( );
};

//可在此继续添加其他的各项检测和统计的配置参数集合
//----add here----


//检测和统计参数配置器
struct StructDetStaParaConfiger
{
public:
	StructQueueLenStaCfgPara		m_QueueLenStaCfgPara;
	StructJamAlertCfgPara			m_JamAlertCfgPara;
	StructAccidentStopAlertCfgPara  m_AccidentStopAlertCfgPara;
	StructVehicleStopAlertCfgPara   m_VehiclStopAlertCfgPara;
	StructAgainstMoveCfgPara		m_AgainstMoveCfgPara;
	StructFastMoveCfgPara			m_FastMoveCfgPara;
	StructChangeCfgPara				m_ChangeCfgPara;
	StructCrossMoveCfgPara			m_CrossMoveCfgPara;
	StructSamePtOffsetCfgPara       m_SamePtOffsetCfgPara;

	StructDetStaParaConfiger( );
	void mvSetDefaultValue( );

	bool mvCvtData2Cfg( const DET_CFG_PARM_ENUM nEnumMod,
						int nParaCnt, float *fData );
};

//---------part3----------//
typedef struct StructEventDetStatCfgParaReader
{
public:
	StructEventDetStatCfgParaReader( );
	bool mvReadAllFiles( int nChannelIdx, int nPresetIdx );
	void mvResetToInitVar( );

	StructDetStaParaConfiger m_DetStaParaConfiger;	//检测和统计参数配置器

private:
	void mvInitVar( );
	bool mvReadOneFile( const DET_CFG_PARM_ENUM nEnumMod );

	//将读入的数据进行转换
	bool mvConvertData( );

private:
	bool m_bInit;

	int m_nChannelIdx;      //通道的序号
	int m_nPresetIdx;       //预置位的序号

	//各不同种类的检测参数配置器
	StruDetectParameterCfg  m_DetCfgParaA[MAX_CFG_PARA_FILE_CNT];  
	char  m_cDetCfgParaFileName[MAX_CFG_PARA_FILE_CNT][40];  

}AnEventDSCfgParaReader;



//--------------------------------

#endif