//目标检测
#ifndef _MV_CONGIGINFO_READER_H_
#define _MV_CONGIGINFO_READER_H_

#ifndef LINUX
	#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include <list>
#include <vector>

#include "libHeader.h"

#ifndef LINUX		
	#include "XmlParser.h"
	#include "VehicleConfig.h"
#endif

#include "comHeader.h"

using namespace std;
using namespace MV_MATH_UTILITY;
using namespace MV_SMALL_FUNCTION;

#ifndef LINUX
	//#define MODIFY_CHANNEL_DRAWINFO  //在本地通过代码修改车道绘制信息
	#ifdef MODIFY_CHANNEL_DRAWINFO
		//#define MODIFY_CALBRATTE_IN_CODE	 //通过代码修改标定
		//#define ADD_LEADSTREAM_LINE_IN_CODE  //通过代码添加导流线
		//#define ADD_CHANGE_LINE_DIR_IN_CODE  //通过代码添加变道线方向
	#endif
#endif


#define MAX_POLYGON_CNT 10    //一次获取的多边形的最多数目
#define POLYGON_MAX_PTCNT 50  //一个多边形的最多顶点数

//得到停车的最长检测时间
enum enumPolyonAreaMod
{
	CET_ROAD_POLYGON_AREA = 0,
	GET_CHANNEL_POLYGON_AREA,
};

//-----------------公用的配置结构体-----------------//
typedef struct _ComConfStru
{
public:
	//得到给定的道路区域集的外包rect
	bool mvGetBoundingRectOfRoadList( 
			RgnList *pRoadRgnList, 
			CvRect &rectRoads );

	//对给定的道路区域按照给定的大小进行ROI设置和Resize
	bool mvSetROIResize( 
			RgnList *pRoadRgnList, int nDstImgArea, 
			ImgRoiResizeStru &struRoadRoiRes );

	//设置车道的maxsize减半标识(行人车道)和道路的模式
	void mvSetHalfFlagRoadMod( int nChanenlCnt, 
			VEHICLE_PARAM_FOR_EVERY_FRAME* pParamDetect,
			int &nRoadMod, bool &bHaveHalfMaxSzChan );

}ComConfStru; 


//-----------需要检测和统计的选项的结构体------------//
typedef struct _NeedDetStaOptStru
{
public:
	bool m_bDetObjAppear;       //目标检测
	bool m_bDetBargeIn;         //闯入检测 
	bool m_bDetForbid;          //禁行检测
	bool m_bDetCross;           //横穿监测
	bool m_bDetAgainst;         //逆行监测
	bool m_bDetChangeChannel;   //变道监测

	bool m_bDetStop;		    //监测停止
	bool m_bDetObjStop;         //采用目标跟踪进行停车检测

	bool m_bDetBeyondMark;		//检测越界
	bool m_bDetIndividualSped;  //检测超速或慢行
	bool m_bDetPersonRun;       //检测行人奔跑
	bool m_bDetChannelJam;      //检测堵塞
	bool m_bDetPeoCrowd ;       //检测聚集	
	bool m_bDetderelict;        //监测遗撒
    bool m_bDetPressYellowLine;	//检测压黄线
	bool m_bDetPressLeadStreamLine;	//检测压导流线

	bool m_bStatQueueLen;	    //统计队列长度

    bool m_bStatFlux ;     //行流量统计(实际为统计)

public:
	_NeedDetStaOptStru( )
	{
		initVar( );
	}

	void initVar( );

}DetStaStru;

//----------------XML配置文件的读入结构体----------------//
#ifndef LINUX	
	typedef struct _XMLConfReadStru
	{
	public:	

		//载入车道检测参数模板
		bool LoadRoadParameterModel( const char * strPath,
					 paraDetectList &roadParamInlist );

		//载入车道检测参数
		bool LoadRoadParameter(	XMLNode &ParentNode,
				VEHICLE_PARAM_FOR_EVERY_FRAME &roadParamIn );

		//载入车道检测参数
		bool LoadRoadParameter( char * configPath,
				paraDetectList &roadParamInlist,
				SRIP_CHANNEL& sChannel,
				int &nChannelCnt, 
				VEHICLE_PARAM **pParamChan );

		//载入车道参数模板
		bool LoadRoadSettingInfo( char *configPath,
				LIST_CHANNEL_INFO& list_channel_info, 
				int nChnl, bool bModel,
				VEHICLE_PARAM* paramChan, 
				int &nHomographyPtNum,
				RgnList &roadRgn  );

		//从车道设置文件中获取车道列表
		bool GetChannelWaysFromRoadSettingInfo(
				char *configPath, int nChannel, 
				std::string& strChannelWays );

		//获取事件模板检测参数
		void GetModelParameter( char *configPath,
				VEHICLE_PARAM_FOR_EVERY_FRAME& roadParamIn );

	private:
		//得到节点中的列表点
		void GetListPointsFromNode(
				POINTLIST &list_, XMLNode &pNode );

		//获取指定节点，到区域内的点
		void GetRegionPointsFromNode(
				COMMON_REGION  &common_region, 
				XMLNode &pNode );

		//从指定节点获得点坐标--nIndex:点的序号--base 0
		void GetPointFromNode( CHANNEL_POINT &point,
				int nIndex, XMLNode &pPointsNode );

	}XMLConfReadStru; 
#endif


//-----------------CConfigInfoReader-----------------//
class CConfigInfoReader
{
public:
	CConfigInfoReader(void);
	~CConfigInfoReader(void);
	void initVar(  );

	int  m_nXmlChannelId;
	void mvSetXmlChannelId( int nXmlChannelId )
	{
		m_nXmlChannelId = nXmlChannelId;
	}

	//配置图像的大小和服务器所用的大小的关系
	float  m_fSrc2Cfg_WRate; //原始图和标定图宽的比率
	double m_dCfg2ServerUse_x;
	double m_dCfg2ServerUse_y; 
	int    m_nDeinterlace;
	bool mvinit( char* config, 
		         double dCfg2ServerUse_x = 1.0,
		         double dCfg2ServerUse_y = 1.0, 
				 int    nDeinterlace = 1 );
	bool mvuninit( );

	//从原始图像坐标转换到server所用的图像坐标
	bool mvGetServerPtFromSrcPt( const CvPoint2D32f ptSrc,
								 CvPoint2D32f &ptUse );

	//说明：初始化函数,读取车道配置信息+配置检测参数
	bool mvSetConfigOnLocalPC(	char* chCfgFileName, 
								double dCfg2Src_x,
								double dCfg2Src_y,
								int &nChannel );

	//设置白天晚上模式(每隔一段时间调用一次)
	int    m_nDayParam; 
	bool   m_bDay;
	void mvconfige_isDayParam( int nDetectTime ); 
	
	//判断是否已分配内存(用于调用mvSetWidth之前判断)
	int    m_nImgWidth;
	bool mvwidthIsSet( );

	//判断是否已设置时间戳,用于调用mvSetTs之前判断
	double  m_dTsNow;
	bool mvtsIsSet( );
	bool mvsetTs(int64_t ts);

	//传递得到远近处的多个行人框
	int    m_nRectPeo;
	CvRect m_rectPeoA[5];
	bool mvGetPeoRectFromClient( int nCnt, CvRect rectA[5] );
	

	//设置是否需要进行车牌检测
	bool   m_bDetectCarnum;
	void SetDetectCarnum( bool bDetectCarnum )
	{
		m_bDetectCarnum = bDetectCarnum;
	}

	//设置是否需要进行颜色检测
	bool   m_bDetectColor;
	void SetDetectColor( bool bDetectColor )
	{
		m_bDetectColor = bDetectColor;
	}


	//从服务端传递得到车道绘制信息(从原始的车道配置信息转换到原始的实验图)
	int	      m_nHomographyPtNum;         //标定点数
	GLOBAL_SETING_PARAM m_paramGloabal;   //全局的区域信息
	
	int       m_nChannel; 	              //车道数
	RgnList   m_roadInitRgnList;          //道路区域信息       
	VEHICLE_PARAM*  m_paramInitChan;      //车道绘制信息
	bool mvSetChannelWaySetting( VerChanList& listVerChan,
								 RegionList& listStabBack,
								 RegionList& listSkip,
								 Calibration& calib, 
								 RoadList& listRoad,											 
								 float fSrc2cfg_x = 1.0f, 
								 float fSrc2cfg_y = 1.0f );	

	//算法初始化阶段的接口函数,配置哪些参数需要检测
	int    m_nShadowParam;       //是否含阴影模式
	bool   m_bShadow;
	int	   m_nSensitiveParam;     //是否为敏感模式
	int    m_nSameEventIgnParam;  //同类事件的忽略时间
	int    m_nShowTimeParam;      //报警的停留显示时间
	VEHICLE_PARAM_FOR_EVERY_FRAME *m_paramDetect;   //检测参数指针 
	DetStaStru  m_struDetStat;
	bool mvConfigParamToDetect( paraDetectList& paramin,
								ROAD_PARAM& param_road );


	//对设置不检测事件的车道及其所对应的道路,进行删除
	bool mvChangeRoadAndDetectParam( );

	//从车道绘制信息的坐标系转换(含平移和缩放)
	RgnList     m_roadRgnList;              
	VEHICLE_PARAM *m_paramChan;
	
	//从车道绘制信息的ROI坐标系转换(含平移和缩放)
	bool mvConvertChanCoordinate( VEHICLE_PARAM* src_paramChan, 
								RgnList src_roadRgnList,
								CvPoint2D32f ptOffSet, 
								double dS_x, double dS_y, 
								VEHICLE_PARAM* dst_paramChan, 
								VEHICLE_PARAM** pp_dst_paramChan, 
								RgnList &dst_roadRgnList );	
#ifdef MODIFY_CHANNEL_DRAWINFO
	//通过代码来修改初始的车道绘制信息
	bool mvModfiyInitChanInCode( VEHICLE_PARAM* src_paramChan, 
								 RgnList src_roadRgnList );	
#endif


	//确定最终的配置是否成功了
	void mvIsConfigSuccess( );

	//--------本地模拟-----------//
	//读入本地的车道配置文件来得到车道绘制信息
	bool mvConfigRoadDrawByReadFile( char* chConfListName, 
									double dSrc2cfg_x,
									double dSrc2cfg_y, 
									int	   nMode, 
									VEHICLE_PARAM* paramChan, 
									RgnList &dst_roadRgnList, 
									VEHICLE_PARAM** pp_paramChan );

	//在本地自己通过修改程序里的车道绘制配置数据来得到车道绘制信息
	bool mvConfigRoadDrawByYouSelf( CvSize srcSz,
									double dReal2conf_x, 
									double dReal2conf_y,
									VEHICLE_PARAM* paramChan, 
									VEHICLE_PARAM **pp_paramChan,
									RgnList &roadRgnList );	

	//在本地自己来修改程序里的车道检测配置数据来得到道路和车道检测信息
	ROAD_PARAM      m_paramRoadDetect;
	paraDetectList  m_paramChanDetectList;     //检测参数列表
	void mvSetDetectParamByYouSelf( int nChannel,
									ROAD_PARAM &paramRoad,	
									paraDetectList &paramChanList );

#ifndef LINUX
	//通过读入的xml配置文件来进行车道绘制等
	bool mvConfigByReadXMLFile( char* chConfigListName,
			int nChannelId, paraDetectList &pParamin,
			int &nChannelCnt, VEHICLE_PARAM *pParamChan, 
			int &nHomographyPtNum, RgnList &roadRgn,
			double dSrc2cfg_x, double dSrc2cfg_y,
			VEHICLE_PARAM** ppParamChan );
#endif

	//通过读入的txt配置文件来进行车道绘制等
	bool mvConfigByReadTxtFile( char* chConfigListName,
			VEHICLE_PARAM *pParamChan, RgnList &dst_roadRgnList,
			double dSrc2cfg_x, double dSrc2cfg_y,
			VEHICLE_PARAM** ppParamChan );


	//获取指定的多边形区域
	bool mvGetPolygonArea( int &nPolyonCnt, //获取的多边形数目
			int nAPtCnt[MAX_POLYGON_CNT],   //各多边形的顶点数目
			CvPoint2D32f fptA[MAX_POLYGON_CNT][POLYGON_MAX_PTCNT], //各多边形的顶点
			int nPolygonAreaMod,		    //多边形区域的模式
			int nChannelIdx                 //多边形所在的车道区域
		);

private:
	//获取检测和统计选项
	void mvGetDetStatOption(  );

	//释放读入配置时所分配的内存(含道路区域列表,
	//   车道绘制结构列表,全局的区域信息)
	void mvReleaseCfg( );

	//释放道路区域列表
	void mvReleaseRoadList( );

	//释放车道绘制结构列表
	void mvReleaseChannel( );

	//释放全局的区域信息
	void mvReleaseParamGloabal( );

public:
	//得到停车的最长检测时间
	float mvGetLongestStopTime( );

	//判断是否为只检测停车
	bool mvIsJustDetVehStop(  );

	//判断停车检测是否为全为长时间停车
	bool mvIsAllLongVehStopDet( float fLongTh );

	//将配置参数存为文本
	void mvSaveData2Txt(VEHICLE_PARAM* paramChan, 
			RgnList roadRgnList, char cTxtName[] );

public:
	bool   m_bGetInitChanInfoCfg;  //配置初始车道绘制信息是否成功
//	bool   m_bGetCvtChanInfoCfg;   //配置转换车道绘制信息是否成功
	bool   m_bGetRoiReszChanCfg;   //配置roi和resize车道配置成功
	bool   m_bGetInitDetParamCfg;  //配置初始检测信息是否成功
	bool   m_bGetObjRectCfg;       //获取目标框的配置是否成功
	bool   m_bCfgSuccess;

	int		m_nRoadMod;
	bool	m_bHaveHalfMaxSzChan;
	ImgRoiResizeStru m_struRoadRoiRes;  //道路区域的ROI和resize
	//ConfigParamSet m_configParam;
	//GetParamSet    m_getParam;
};


//---------------------------------

#endif