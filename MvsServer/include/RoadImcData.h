// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
/*
* 天津电警传输解析数据格式
*/

#ifndef ROAD_IMC_DATA_H
#define ROAD_IMC_DATA_H

#include "global.h"


class CRoadImcData
{
public:
     /*构造函数*/
    CRoadImcData();
    /*析构函数*/
    ~CRoadImcData();

public:
    //生成卡口车牌数据
    void  AddCarNumberData(RECORD_PLATE& plate,std::string strPicPath);
    //加入新类型和值
    std::string  AddNewItemValue(const String &strMsg, bool bFather, const char *szItem, const String &strValue, const bool bCharType = true);

    //图像切分
    bool CutImgData(const String &strMsg, const int nPicSize, const int nDataType, const int nCount,\
                     const unsigned int uPicWidth, const unsigned int uPicHeight, String& strPic0,String& strPic1,String& strPic2, int nValueArry[3]);
    //获取方向解析
    std::string GetDirectionStr(int nDirection, int &nDirection2);
	int GetDirectionForJinan(int nDirection);
	int GetDirectionForNanning(int nDirection);

    //获取车身颜色解析
    std::string GetCarColorStr(int nColor);
	int GetCarColorForJinan(int nColor);

	// 获取车牌品牌识别
	int GetCarBrandForJinan(int nCarBrand);

    //获取号牌颜色解析
    std::string GetCarNumberColorStr(int nNumberColor, int &nNumberColor2);

    //获取号牌种类解析
    std::string GetPlateTypeStr(std::string strCarNumber);

    //获取车辆违法类型解析
    std::string GetViolationTypeStr(int nViolationType,int& nViolationType2);

	//获取车辆类型
	std::string GetVechileType(int nVechileType);
	int GetVechileTypeForJinan(RECORD_PLATE& plate);
	int GetViolationTypeForNanning(int nViolationType);
	int GetViolationTypeForNanning2(int nViolationType);

    //获取浮点时间类型
    std::string  GetFloatStringTime(UINT32 uTime);

	//获取路口名称
	std::string GetPlace();

	void AddViolationData(RECORD_PLATE& plate,std::string strPicPath,UINT32 uSignalBeginTime,UINT32 uSignalEndTime,int nMiTime);

	void AddStopEventViolationData(RECORD_PLATE& plate,std::string strPicPath,string strDeviceId);

	string  GetViolationData(RECORD_PLATE& plate);
	string  GetViolationDataForJinan(RECORD_PLATE& plate, string strJpgFile);
	string  GetViolationDataForkafka(RECORD_PLATE& plate, string strJpgFile);
	string  GetViolationDataForNanning(RECORD_PLATE& plate, int nSpeedMax,string strJpgFile,string strFtpPath,string chJpgFileBuf1,string chJpgFileBuf2,string chJpgFileBuf3,string chJpgFileBuf4);

	//获取车辆违法类型解析
	int GetViolationType(int nViolationType);
	int GetViolationTypeForJinan(int nViolationType);

	//生成流量统计配置文件
	string  AddStatisticData(const String &strMsg,int nRegionType = 1,int nStatisticType = 0);
	
	int GetVechileTypeForNanning(int uTypeDetail);
	string GetCarColorForNanning(int nColor);

	//判断目录是否为空
	bool CheckDir(const char * dirName);

	//取得车型细分
	string GetDetailCarType(int typeCode, bool easyDisplay = 0);

	//取得事件类型
	string GetEventTypeStr(int typeCode);

	//取得天津中心端的FTP远程路径
	string GetFtpRemoteDir(RECORD_PLATE *pPlate);

	//获取图片路径
	void GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath,int nType,int nRandCode = 0,int nIndex = 1);

	//获取随机防伪码
	int GetRandCode();

	//从jpg文件中获取防伪码
	void GetRandCodeFromPic(string strPicPath,int& nRandCode1,int& nRandCode2);

	//生成天津流量统计配置文件,相同断面,文件合并
	string  AddStatisticDataTj(const String &strMsg,int nRegionType = 1,int nStatisticType = 0);

	//更新道路数量
	bool UpdateRoadCounts(
		const std::string &strPath, 
		const std::string &strLeftValue, 
		int nRoadCounts);

	//从文件取得约定格式字符串
	//[strLeftValue strMidValue strRightValue]
	bool GetValueFromFile(
		const std::string &strPath, 
		const std::string &strLeftValue, 
		const std::string &strMidValue, 
		int &nRightValue);

	//更新,一行约定格式字符串
	bool UpdateStrLine(
		const std::string &strPath, 
		const std::string &strLeftValue, 
		const std::string &strNewValue);

private:

	//路口名称
	std::string m_strLocation;
	//设备ID
	std::string m_strDeviceId;

	//流量统计锁
	pthread_mutex_t m_StatisticMutex;
};

extern CRoadImcData g_RoadImcData;

#endif
