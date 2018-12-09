/*
* 旅行时间xml传输数据格式
*/

#ifndef ROAD_XML_DATA_H
#define ROAD_XML_DATA_H

#include "global.h"
#include "XmlParser.h"
//#include "VehicleConfig.h"
#include "TravelConsdef.h"

class CRoadXmlData
{
public:
     /*构造函数*/
    CRoadXmlData();
    /*析构函数*/
    ~CRoadXmlData();

public:
   //生成数据包头
    bool SetPackageHead(XMLNode& PackageHeadNode);
    //生成车牌数据
    string AddCarNumberData(const string &strMsg,string &strNewMsg,int nUploadType=1);
    //生成交通事件数据
    string AddEventData(const string &strMsg, string &strNewMsg,int nUploadType=1);
    //生成违法数据
    string AddViolationData(const string &strMsg,string &strNewMsg,int nUploadType=1);
    //生成车流量数据
    string AddFlowData(const string &strMsg, string &strNewMsg);
    //生成日志数据
    string AddLogData(string &strNewMsg);
    //生成运行日志数据
    string AddRunLogData(const string &strMsg, string &strNewMsg);

    //获取行驶方向编码--参见1.8 行驶方向编码
    std::string GetDirectionStr(std::string strDirection);

    //获取事件类型编码
    char GetEventType(int nEvent);

    //获取车牌颜色类型编码--见1.15车辆号牌颜色
    int GetCarNumColor(int nColor);

    //获取车牌标志类型编码--见1.16车辆标志分类--（暂保留）
    int GetCarLogo(int nLogo);

    //解析业务控制参数内容
    bool GetControlInfo(const string &strMsg, TRAVEL_CONTROL_INFO &travelControlInfo);

    //获取设备配置信息
    bool GetDeviceInfo(const string &strMsg);

    //获取未上传违法数据量
    int GetUnsendCount();

    //获取车身位置
    CvRect GetCarPos(RECORD_PLATE plate);

    //车牌事件结构转换
    void ConvertPlateToEvent(const string &strMsg,string &strViolationMsg);

private:
    //车道数目
    int m_nCarRoadNum;
    //车道方向
    int m_nDirection;

};

#endif
