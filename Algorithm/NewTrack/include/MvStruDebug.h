//进行调试的结构体
#ifndef _MV_STRU_DEBUG_H_
#define _MV_STRU_DEBUG_H_

#include "libHeader.h"
#include "comHeader.h"  

#include "MvReadDebugCfg.h"

#include "MvImgProUtility.h"
using namespace MV_IMGPRO_UTILITY;

enum ENUM_LEVEL0_TYPE{ TIME_USE_D,      //时间消耗
					   IMG_PROCESS_D,   //图像处理
					   OBJECT_TYPE_D,   //目标类型判断
					   AGANIST_ALERT_D, //逆行报警检测
					   CARSTOP_ALERT_D  //停车报警检测
					};

enum ENUM_LEVEL1_TYPE{  HOG_RESULT_D,   //HoG检测结果
						TYPE_CONFIRM_D  //类型确认结果
					};

//enum ENUM_LEVEL2_TYPE{  };


//调试信息结构体
typedef struct StructDebugInfo
{ 
	int  nLevel0Type;  //第一层信息
	int  nLevel1Type;  //第二层信息
	int  nLevel2Type;  //第三层信息 

	double  dAddTs;  //加入时的时间戳
	string  str;     //调试信息
	
	StructDebugInfo( )
	{
		nLevel0Type = nLevel1Type = nLevel2Type = -1;
	}
}MvDebugInfo;


//用于调试字符串列表结构
typedef struct MvStruStringList
{
public:	
	MvStruStringList( );
	~MvStruStringList( );

	//初始化
	void mvInitStruStringList( int MaxLineCnt );

	//将信息压入列表中
	bool push_string( const char *chTempInfo, double dTsNow, 
				 int nLevel0Type = -1, int nLevel1Type = -1, 
				 int nLevel2Type= -1 );

	//得到最近的信息
	bool getRecentString( int nLineCnt );

public:
	int     m_nTempCnt;  //注意最多控制在64行
	char	**m_chTempInfo;

private:
	vector<MvDebugInfo>   *m_pVectString;	
	vector<MvDebugInfo *>  m_vectStrRecAdd;  //最近加入的

	void initVar( );	

private:
	vector<MvDebugInfo> m_vectString0;
	vector<MvDebugInfo> m_vectString1;
	int m_nMaxLineCnt;		//最多行数
	int m_nNowLineIdx;		//当前的行序号
	int m_nVectStringIdx;   //当前string的序号

}StruStringList, MvDebugInfoList;



#define MAX_STRU_SAVEIMG_NUM 10  //存图结构体的最多个数
enum _image_save_mod
{
	SAVE_AS_ORI_IMG = 0,
	SAVE_AS_USUAL_IMG 
};

//总的调试结构体
typedef struct MvStruTotalDebug
{
public:
	bool m_bDebug;			//debug是否为打开的
	bool m_bUpdateStaus;	//是否更新状态
	MvDebugCfgReader m_debugCfg;  //调试配置读入器

public:
	MvStruTotalDebug( );

	//初始化变量
	void initStruTotalDebugVar( );
		
	//判断是否需要打开debug
	bool isDebugStatus( );

	//获取得到调试的配置选项
	bool getDebugConfigOption( );

	//存原始图
	bool saveImgAndTxt( int nSaveMod,   //保存模式
			IplImage *pSImg,       //要存的图片
			int64_t nFrameNum,     //图像帧号(外部传入)
			vector<MvCarNumInfo> *vecCarNum, //车牌信息
			double dTsNow,         //当前的时间戳
			char *chIfSaveCfgFN,   //是否保存的配置文件
			char *chSaveTimeCfgFN, //保存的时间配置文件
			char *chSaveDirName    //保存目录  //"OriImg"
		);

	//存车牌信息文件
	bool saveCarnumTxt(	int nSaveIdx,  //保存的序号
			int64_t nFrameNum, //保存的图像帧号(外部传入)
			vector<MvCarNumInfo> *vecCarNum, //车牌信息
			char *chSaveDirName  	 //保存目录  //"OriImg"
		);

private:
	//打开控制文件，看是否需要保存
	bool isNeedSave( char *chIfSaveCfgFN );

private:
	bool m_bInit;	  //是否初始化过了

	AnDateStruct m_initDgbugDate;  //初始化时的日期

	bool m_bAHadSaveImg[MAX_STRU_SAVEIMG_NUM];   //是否存过图像

public:
	ANImgSaveStruct m_struAImgSave[MAX_STRU_SAVEIMG_NUM];  //存图结构体
	
}StruTotalDebug;


//轨迹调试
typedef struct StruDebugTracks
{
   static bool mvDrawOneTrack( IplImage *pDrawImg, const MyTrack &tr, 
	      const CvScalar &colorTr=CV_RGB(255,0,0), int nThickness=0 );

}MvDebugTracks;

//
////group调试
//typedef struct MvStruGroupsDebug{
//
//
//}StruGroupsDebug;
//

//目标调试
typedef struct StruDebugObjects
{
	static bool mvDrawOneObject( IplImage *pDrawImg, 
			const MyGroup &obj, int nObjIdx = -1, 
			const CvScalar &colorObj = CV_RGB(255,0,0),
			int nThickness = 0 );

}MvDebugObjects;

#endif