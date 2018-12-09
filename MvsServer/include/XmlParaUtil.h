// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef XML_PARA_UTIL_H
#define XML_PARA_UTIL_H

#include "global.h"
#include "XmlParser.h"
#include "VehicleConfig.h"

#include "VideoShutter.h"
#ifndef ALGORITHM_YUV
#include "MvFindTargetByVirtualLoop.h"
#include "mvdspapi.h"
#else
#include "MvDspAPI.h"
#endif


class CXmlParaUtil {
public:

    //生成相机模板
    bool AddCameraParaModel(CAMERA_CONFIG& cfg);
    //载入相机模板
    bool LoadCameraParaModel(CAMERA_CONFIG& cfg,bool bLoadByServer = true);

    //生成设备车道坐标参数
    bool AddDeviceSettingInfo(LIST_CHANNEL_INFO& list_channel_info, int nDeviceID,int nPreSet = 0);
    //生成车道参数模板
    bool AddRoadSettingInfo(XMLNode& ChannelsNode, CHANNEL_INFO& channel_info, int nChannel);
    //生成车道参数模板1
    bool AddRoadSettingInfoByList(LIST_CHANNEL_INFO& list_channel_info, int nChannel);
    //载入车道参数模板
    bool LoadRoadSettingInfo(LIST_CHANNEL_INFO& list_channel_info, int nChannel,bool bModel = false);

    //从车道设置文件中获取车道列表
    bool GetChannelWaysFromRoadSettingInfo(int nChannel,std::string& strChannelWays);
    //更新参数设置文件(闯红灯，线圈，事件等)
    void UpdateParaSettingInfo(int nChannel,std::string strChannelWays,int nPreSet);

    //客户端获取车道坐标
    std::string GetRoadSettingInfo(int nChannel,int nModel);

    //清空车道
    bool DeleteRoadSettingInfo(int nChannel);

    ////生成闯红灯参数list
    bool AddTrafficParameterByList(VTSParaMap &mapVTSPara, int nChannel,VTS_GLOBAL_PARAMETER& vtsGlobalPara);
    //生成闯红灯参数
    bool AddTrafficParameter(XMLNode& ChannelsNode,PARAMETER_VTS &vtsPara, int nChannel,VTS_GLOBAL_PARAMETER& vtsGlobalPara);
    //载入闯红灯参数
    bool LoadVTSParameter(VTSParaMap &mapVTSPara, int nChannel,VTS_GLOBAL_PARAMETER& vtsGlobalPara);

    ////生成线圈参数list
    bool AddLoopParameterByList(LoopParaMap &MapLoopPara, int nChannel);
    //生成线圈设置参数
    bool AddLoopParameter(XMLNode& ChannelsNode,PARAMETER_LOOP &loopPara, int nChannel);
    //获取线圈设置参数
    bool LoadLoopParameter(LoopParaMap& MapLoopPara,int nChannel);

    //获取模板编号
    int GetMaxModelId(XMLNode& ModelsNode);
    //更新事件检测参数
    void UpdateRoadParameterNode(XMLNode& ParentNode,VEHICLE_PARAM_FOR_EVERY_FRAME & roadParamIn);
    //生成车道检测模板
    bool AddRoadParameterModel(VEHICLE_PARAM_FOR_EVERY_FRAME & roadParamIn,int nModelKind,int& nModelID);
    //生成车道检测参数
    bool AddRoadParameter(XMLNode& ChannelsNode, VEHICLE_PARAM_FOR_EVERY_FRAME & roadParamIn,SRIP_CHANNEL_EXT& sChannel);
    //生成车道检测参数1
    bool AddRoadParameterByList(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT& sChannel);
    //生成设备检测参数
    bool AddDeviceParaMeterInfo(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT sChannel,int nDeviceID,int nPreSet = 0);
    //删除模板节点
    bool DeleteModelNode(XMLNode ModelsNode,int nModel);

    //给指定节点添加链表内的点
    void AddListPointsToNode(XMLNode &pNode, Point32fList &list);
    //给指定节点添加区域内的点
    void AddRegionPointsToNode(XMLNode &pNode, COMMON_REGION  &region);
    //给指定节点添加链表内的点，3维坐标
    void AddListPoints32ToNode(XMLNode &pNode, Point32fList &list);

    //给指定节点添加nPoints个点
    void AddPointsToNode(XMLNode &pNode, int nPoints);

    //给指定节点添加1个点
    void AddPointToNode(XMLNode &pPointsNode, int nIndex, const CPoint32f &point);

    //载入系统设置
    bool LoadSystemSetting();
    //更新系统设置
    bool UpdateSystemSetting(std::string strSetting,std::string strNode);

    //核查节点的有效性，若不存在则新建一个此节点
    bool CheckXMLNode(XMLNode& ParentNode, XMLNode& SonNode, int iIndex, XMLCSTR strTag);

    //从指定节点获取到链表内的点
    void GetListPointsFromNode(Point32fList &list, XMLNode &pNode);
    //获取指定节点，到区域内的点
    void GetRegionPointsFromNode(COMMON_REGION  & common_region, XMLNode &pNode);

    //从指定节点获得点坐标--nIndex:点的序号--base 0
    void GetPointFromNode(CPoint32f &point, int nIndex, XMLNode &pPointsNode);

    //删除通道节点
    bool DeleteChannelNode(XMLNode ChannelsNode,int nChannel);

    bool UpdateDataToNode(XMLNode& ParentNode, XMLNode& SonNode, int iIndex, XMLCSTR strTag, double nValue,int nValueType = 0);
    bool UpdateStrDataToNode(XMLNode& ParentNode, XMLNode& SonNode, int iIndex, XMLCSTR strTag, XMLCSTR strValue);

    //载入全部车道检测参数及其模板
    bool LoadAllRoadParameter(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT& sChannel);
    //载入车道检测参数
    bool LoadRoadParameter(XMLNode &ParentNode,VEHICLE_PARAM_FOR_EVERY_FRAME &roadParamIn);
    //载入车道检测参数
    bool LoadRoadParameter(paraDetectList &roadParamInlist,SRIP_CHANNEL_EXT& sChannel);
    //载入车道检测参数模板
    bool LoadRoadParameterModel(paraDetectList &roadParamInlist);
    //获取模板参数设置
    void GetModelParameter(VEHICLE_PARAM_FOR_EVERY_FRAME& roadParamIn);

    //获取雷达检测参数
    bool LoadRadarParameter(int nChannel,RadarParaMap& MapRadarPara);

    //客户端获取雷达检测参数
    bool GetRadarParameter(int nChannel,string& strRadarPara);

    //设置雷达参数
    bool SetRadarParameter(int nChannel,string strRadarPara);


     //获取目标检测参数
    bool LoadObjectParameter(int nChannel,_ObjectPara& ObjectPara);

    //客户端获取目标检测参数
    bool GetObjectParameter(int nChannel,string& strObjectPara);

    //设置目标参数
    bool SetObjectParameter(int nChannel,string strObjectPara);

    //获取裁剪区域坐标(数据库中存放的是jpg图像坐标)
    RegionCoordinate GetClipRegionCoordinate(int nChannel);

    //获取稳像区域
    void GetStabBackRegion(int nChannel,RegionList& listStabBack);
    //获取同步区域
    void GetSynchRegion(TRACK_GROUP& sLeftTrack,TRACK_GROUP& sRightTrack,CvRect& rtDetectArea);

    //设置停车检测区域
    bool SetStopRegion(int nChannel,string strStopRegion);
    //获取停车检测区域
    bool GetStopRegion(int nChannel,string& strStopRegion);
    //获取车道标定信息
    bool GetCalibrationAndDirectionInfo(int nChannel,CALIBRATION& calibration,CHANNEL_PROPERTY& chDirection);
    //获取区域
    bool GetRegion(int nChannel,string& strRegion);
    //设置区域
    bool SetRegion(string& strRegion);
    //判断点是否在多边形内
    bool PointInRec(Point32fList& vList,CPoint32f point);
    //扩充越界区域
    bool GetBeyondMarkRegion(Point32fList& ptList,int nMaxWidth,int nMaxHeight,CALIBRATION calibration);
    //获取车宽度
    int GetCarSize(CPoint32f  point,CALIBRATION calibration);
    //获取区域中心点
    void GetCenterPoint(Point32fList ptList,CPoint32f& ptCenter);
    //获取车道区域,虚拟检测区域以及车牌区域宽高
    void GetVirtualLoopRegion(int nChannel,vector<mvvideorgnstru>& mChannelReg,CvRect& VideoRect,CvRect& rtCarnumber);
    //获取图像坐标和世界坐标
    bool GetCalibration(float image_cord[12], float world_cord[12], int channelID);
    //获取预置位信息
    bool GetPreSetInfo(string& strPreset,string& response);
    //更新预置位信息
    bool UpdatePreSetInfo(string& strPreset);
	//更新预置位检测类型
	bool UpdatePreSetDetectKind(int nChannel,int nPreSet,int nDetectKind);
    //加载预置位信息
    bool LoadPreSetInfo(int nChannel,int nPreSet,PreSetInfoList& ListPreSetInfo);
    //删除预置位信息
    bool DeletePreSetInfo(string& strPreset);
	//获取远景预置位列表
	bool LoadRemotePreSet(int nChannel,vector<int>& vRemotePreSet);
	//获取预置位检测类型
	int LoadPreSetDetectKind(int nChannel,int nPreSet);

    //设置行为分析参数
    bool SetBehaviorParameter(int nChannel,string strBehaviorPara);

    //获取行为分析参数设置
    bool GetBehaviorParameter(int nChannel,string& strBehaviorPara);

    //载入行为分析参数
    bool LoadBehaviorParameter(paraBehaviorList &listParaBehavior,SRIP_CHANNEL_EXT& sChannel);

    //载入行为分析参数
    bool LoadBehaviorParameter(XMLNode &ParentNode,BEHAVIOR_PARAM &ParaBehavior);

	//载入违章数据表
	bool LoadBasePlateInfo();

	//载入模板配置
	void LoadSysModel(int nSysModel);
	
	//取得配置文件中的限速 Map<key:车道号, value:最大速度>;
	//void GetMaxSpeed(map<UINT32,UINT32>& raodWayMap, UINT32 channelId);
	//取得配置文件中的限速
	void GetMaxSpeedStr(mapMaxSpeedStr &roadWayMapStr, const UINT32 channelId);

	//北京H264视频传输协议，封装Event成xml
	void CreateEventToXml(int cameraID, const RECORD_EVENT* sEvent, string& xmlstr);

	//北京H264视频传输协议，解析登陆报文
	void ParserLoginXml(const string& xmlstr, string& name, string& passwd);

	//北京H264视频传输协议，解析视频切换报文
	bool ParserSwitchCameraXml(const string& xmlstr, map<string, UINT32>&cameraMap);


	//生成检测配置获取请求(从AMS获取检测配置)
	void CreateRequestAMS(vector<UINT32>& cameraVec, string& xmlStr);

	//解析xml并从AMS取得的配置文件,并替换本地文件
	bool ParseAndUpdateSetting(string& xmlStr);

	//解析xml并修改检测类型
	int ParseAndSetDetectKind(string& xmlStr);

	//解析xml并更新系统设置
	bool ParseAndSetSystemSetting(string& xmlStr);

	//载入打印模块时间信息设置
	bool LoadTimeLogSetting();

	//解析视频切换请求
	bool ParseSwitchCamera(string& xmlStr);

	//载入dsp配置文件
	bool LoadDspSettingFile(MvDspGlobalSetting& mvdspGlobalSetting,int nChannel);

	//载入DspSetting参数
	bool LoadDspSettingInfo(MvDspGlobalSetting& dspSettingStrc, CALIBRATION &calibration, int nChannel);

	//解析MvRect结构
	void GetRectFromNode(MvRect &rect, XMLNode &rectNode);

	//解析DspMvLine结构
	#ifndef ALGORITHM_YUV
	void GetMvLineFromNode(DspMvLine &dspMvLine, XMLNode &pNode);
	#else
	void GetMvLineFromNode(MvLine &dspMvLine, XMLNode &pNode);
	#endif

	//解析DspMvPoint[16]列表结构
	#ifndef ALGORITHM_YUV
	int GetMvPointsFromNode(DspMvPoint DspMvPointsArray [],	XMLNode &pNode);
	#else
	int GetMvPointsFromNode(MvPoint DspMvPointsArray [],	XMLNode &pNode);
	#endif

	//获取解析整型值，返回整型值	
	int GetIntValFromNode(XMLNode &TempNode, bool bFloatFlag = false);

	//解析mvDspRgn[8]列表结构
	int GetMvDspRgnsFromNode(MvDspRegion mvDspRgn[], XMLNode &mvDspRgnsNode);

	//解析DspMvLine[]列表结构,返回列表数目
	#ifndef ALGORITHM_YUV
	int GetMvLinesFromNode(DspMvLine dspMvLineArray[], XMLNode &mvLinesNode);
	#else
	int GetMvLinesFromNode(MvLine dspMvLineArray[], XMLNode &mvLinesNode);
	#endif

	//设置图片格式参数
	bool SetPicFormatInfo(int nChannel,string strPicFormatInfo);

	//客户端获取图片格式参数
	bool GetPicFormatInfo(int nChannel,string& strPicFormatInfo);

	//获取图片格式参数
	bool LoadPicFormatInfo(int nChannel,REGION_ROAD_CODE_INFO& picFormatInfo);	

	//生成操作系统信息
	bool WriteOsInfo();

	//获取检测器程序版本
	bool GetVersion(string& strVersion);

	//生成xml格式的文件
	bool SetRoadItudeInfo(string strData);
	//获取路段信息(经纬度，路段名)
	bool LoadRoadItudeInfo(RoadNameInfoList &listRoadInfo);
	bool GetRoadItudeInfo(string& strData);
	
private:
	//载入模板中的系统设置部分
	void GetSystem(XMLNode setting);
	//载入模板中的视频设置部分
	void GetVideoFormatInfo(XMLNode setting);
	//载入模板中的图片设置部分
	void GetPicFormatInfo(XMLNode setting);

	//取得模板中的限速
	//UINT32 GetMaxSpeedModel(int modelId);	
	//取得模板中的限速
	MaxSpeedStr GetMaxSpeedStrModel(int modelId);
};

#endif
