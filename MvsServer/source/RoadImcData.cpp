// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary

#include "Common.h"
#include "CommonHeader.h"
#include "ximage.h"
#include "RoadImcData.h"
#include "XmlParaUtil.h"
#include <sys/mount.h>

CRoadImcData g_RoadImcData;

CRoadImcData::CRoadImcData()
{
	m_strLocation = "";
	pthread_mutex_init(&m_StatisticMutex,NULL);
}

CRoadImcData::~CRoadImcData()
{
	pthread_mutex_destroy(&m_StatisticMutex);
}

/* 函数介绍：生成卡口车牌数据解析文件
 * 输入参数：strMsg：车牌数据内容
 * 输出参数：strNewMsg：生成的卡口车牌数据解析文件
 *           strPicHead：名称前缀
 * 返回值：生成解析文件的磁盘绝对路径
 */
void  CRoadImcData::AddCarNumberData(RECORD_PLATE& plate,std::string strPicPath)
{
	//一，图片相关信息
	String strBeginMsg = "";
	String strNewMsg  = "";
	String strValue;

	strNewMsg = AddNewItemValue(strBeginMsg, true, "RecordInfo", "");

	char buf[256] = {0};

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%08d", plate.uSeq);
	strValue = buf;
	strNewMsg = AddNewItemValue(strNewMsg, false, "LogNo", strValue); //记录编号

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", plate.uRoadWayID);
	strValue = buf;
	strNewMsg = AddNewItemValue(strNewMsg, false, "LaneNo", strValue); //车道号

	int nDirection = 1;
	String strDirection = GetDirectionStr(plate.uDirection,nDirection);

	strNewMsg = AddNewItemValue(strNewMsg, false, "Direction", strDirection);//行驶方向

	if(g_nDetectMode == 2)
	{
		std::string strLocation(plate.chPlace);
		strLocation += strDirection;
		strNewMsg = AddNewItemValue(strNewMsg, false, "Location", strLocation);//路口名称
	}
	else
	{
		strNewMsg = AddNewItemValue(strNewMsg, false, "Location", m_strLocation);//路口名称
	}

	strNewMsg = AddNewItemValue(strNewMsg, false, "LocationNo", g_ftpRemotePath);//路口编号

	if(7 == g_nServerType)
	{		
		std::string strDeviceId = g_skpDB.GetDeviceByChannelId(plate.uChannelID);
		strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceNo", strDeviceId);//设备Id
	}
	else
	{
		strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceNo", g_strDetectorID);//设备编号
	}	

	strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceName", "高清闯红灯");//设备名称

	String strCarNumber(plate.chText);

	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateText", strCarNumber);//号牌号码

	String strPlateType = GetPlateTypeStr(strCarNumber);
	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateType", strPlateType);//号牌种类

	int nCarNumberColor = 2;
	String strCarNumColor = GetCarNumberColorStr(plate.uColor, nCarNumberColor);
	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateColor", strCarNumColor);//号牌颜色
	
	String strVechileType = GetVechileType(plate.uType);
	strNewMsg = AddNewItemValue(strNewMsg, false, "VechileType", strVechileType);//车辆类型
	

	String strPath;
	if(g_nSaveImageCount == 2)
	{
		strPath.append(strPicPath.c_str(),strPicPath.size()-6);
		strPath += ".ini";
	}
	else
	{
		strPath.append(strPicPath.c_str(),strPicPath.size()-3);
		strPath += "ini";
	}
	//g_skpDB.UTF8ToGBK(strPath);

	g_skpDB.UTF8ToGBK(strNewMsg);

	FILE* fp = NULL;
	fp = fopen(strPath.c_str(),"wb");
	if(fp!=NULL)
	{
	fwrite(strNewMsg.c_str(),1,strNewMsg.size(),fp);
	fclose(fp);
	}
	chmod(strPath.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
}


/* 函数介绍：生成卡口车牌数据解析文件
 * 输入参数：strMsg：车牌数据内容
 * 输出参数：strNewMsg：生成的卡口车牌数据解析文件
 *           strPicHead：名称前缀
 * 返回值：生成解析文件的磁盘绝对路径
 */
void  CRoadImcData::AddViolationData(RECORD_PLATE& plate,std::string strPicPath,UINT32 uSignalBeginTime,UINT32 uSignalEndTime,int nMiTime)
{
	//一，图片相关信息
	String strBeginMsg = "";
	String strNewMsg  = "";
	String strValue;

	strNewMsg = AddNewItemValue(strBeginMsg, true, "RecordInfo", "");
	
	char buf[256] = {0};

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%08d", plate.uSeq);
	strValue = buf;
	strNewMsg = AddNewItemValue(strNewMsg, false, "LogNo", strValue); //记录编号

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", plate.uRoadWayID);
	strValue = buf;
	strNewMsg = AddNewItemValue(strNewMsg, false, "LaneNo", strValue); //车道号

	int nDirection = 1;
	String strDirection = GetDirectionStr(plate.uDirection,nDirection);

	strNewMsg = AddNewItemValue(strNewMsg, false, "Direction", strDirection);//行驶方向

	if(g_nDetectMode == 2)
	{
		std::string strLocation(plate.chPlace);
		strLocation += strDirection;
		strNewMsg = AddNewItemValue(strNewMsg, false, "Location", strLocation);//路口名称
	}
	else
	{
		strNewMsg = AddNewItemValue(strNewMsg, false, "Location", m_strLocation);//路口名称
	}

	strNewMsg = AddNewItemValue(strNewMsg, false, "LocationNo", g_ftpRemotePath);//路口编号

	if(7 == g_nServerType)
	{		
		std::string strDeviceId = g_skpDB.GetDeviceByChannelId(plate.uChannelID);
		strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceNo", strDeviceId);//设备Id
	}
	else
	{
		strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceNo", g_strDetectorID);//设备编号
	}	

	strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceName", "高清闯红灯");//设备名称

	String strCarNumber(plate.chText);

	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateText", strCarNumber);//号牌号码

	String strPlateType = GetPlateTypeStr(strCarNumber);
	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateType", strPlateType);//号牌种类

	int nCarNumberColor = 2;
	String strCarNumColor = GetCarNumberColorStr(plate.uColor, nCarNumberColor);
	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateColor", strCarNumColor);//号牌颜色

	String strVechileType = GetVechileType(plate.uType);
	strNewMsg = AddNewItemValue(strNewMsg, false, "VechileType", strVechileType);//车辆类型

	string strBeginTime = GetTime(plate.uTime,0);
	sprintf(buf," %03d", plate.uMiTime);
	string strMiTime(buf);
	strBeginTime += strMiTime;
	strNewMsg = AddNewItemValue(strNewMsg, false, "StartTime", strBeginTime);// 开始时间

	string strEndTime = GetTime(plate.uTime+10,0);
	strEndTime += strMiTime;
	strNewMsg = AddNewItemValue(strNewMsg, false, "EndTime", strEndTime);// 结束时间


    if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
	{
		string strRedLightBeginTime("");
		strRedLightBeginTime = GetTime(uSignalBeginTime,0);
		sprintf(buf," %03d", nMiTime);
		strMiTime = buf;

		strRedLightBeginTime += strMiTime;

		strNewMsg = AddNewItemValue(strNewMsg, false, "RedLightBeginTime", strRedLightBeginTime);// 红灯开始时间

		string strRedLightEndTime("");
		strRedLightEndTime = GetTime(uSignalEndTime,0);
		sprintf(buf," %03d", (nMiTime+6)%1000);
		strMiTime = buf;

		strRedLightEndTime += strMiTime;

		strNewMsg = AddNewItemValue(strNewMsg, false, "RedLightEndTime", strRedLightEndTime);// 红灯结束时间
	}

	int nViolationType = 0;
	strValue = GetViolationTypeStr(plate.uViolationType,nViolationType);
	strNewMsg = AddNewItemValue(strNewMsg, false, "ViolationType", strValue);//违法行为名称

	String strPath;
	strPath.append(strPicPath.c_str(),strPicPath.size()-15);
	strPath += ".ini";
	//g_skpDB.UTF8ToGBK(strPath);

	g_skpDB.UTF8ToGBK(strNewMsg);

	FILE* fp = NULL;
	fp = fopen(strPath.c_str(),"wb");
	if(fp!=NULL)
	{
		fwrite(strNewMsg.c_str(),1,strNewMsg.size(),fp);
		fclose(fp);
	}
	chmod(strPath.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
}

void CRoadImcData::AddStopEventViolationData(RECORD_PLATE& plate,std::string strPicPath,string strDeviceId)
{
		//一，图片相关信息
	String strBeginMsg = "";
	String strNewMsg  = "";
	String strValue;

	strNewMsg = AddNewItemValue(strBeginMsg, true, "RecordInfo", "");
	
	char buf[256] = {0};

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%08d", plate.uSeq);
	strValue = buf;
	strNewMsg = AddNewItemValue(strNewMsg, false, "LogNo", strValue); //记录编号

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", 0);
	strValue = buf;
	strNewMsg = AddNewItemValue(strNewMsg, false, "LaneNo", strValue); //车道号

	int nDirection = 1;
	String strDirection = GetDirectionStr(plate.uDirection,nDirection);

	strNewMsg = AddNewItemValue(strNewMsg, false, "Direction", " ");//行驶方向

	strNewMsg = AddNewItemValue(strNewMsg, false, "Location", m_strLocation);//路口名称

	strNewMsg = AddNewItemValue(strNewMsg, false, "LocationNo", g_ftpRemotePath);//路口编号

	strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceNo", strDeviceId);//设备Id

	strNewMsg = AddNewItemValue(strNewMsg, false, "DeviceName", "违停抓拍");//设备名称

	String strCarNumber(plate.chText);
	//g_skpDB.UTF8ToGBK(strCarNumber);
	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateText", strCarNumber);//号牌号码

	//String strPlateType = GetPlateTypeStr(strCarNumber);
	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateType", "普通");//号牌种类

	int nCarNumberColor = 2;
	//String strCarNumColor = GetCarNumberColorStr(plate.uColor, nCarNumberColor);
	strNewMsg = AddNewItemValue(strNewMsg, false, "PlateColor", "蓝色");//号牌颜色

	//String strVechileType = GetVechileType(plate.uType);
	strNewMsg = AddNewItemValue(strNewMsg, false, "VechileType", "小");//车辆类型

	string strBeginTime = GetTime(plate.uTime,0);
	sprintf(buf," %03d", plate.uMiTime);
	string strMiTime(buf);
	strBeginTime += strMiTime;
	strNewMsg = AddNewItemValue(strNewMsg, false, "StartTime", strBeginTime);// 开始时间

	string strEndTime = GetTime(plate.uTime2,0);
	sprintf(buf," %03d", plate.uMiTime2);
	string strMiTime2(buf);
	strEndTime += strMiTime2;
	strNewMsg = AddNewItemValue(strNewMsg, false, "EndTime", strEndTime);// 结束时间

	strNewMsg = AddNewItemValue(strNewMsg, false, "ViolationType", "违反禁令标志");//违法行为名称

	String strPath;
	strPath.append(strPicPath.c_str(),strPicPath.size()-15);
	strPath += ".ini";
	//g_skpDB.UTF8ToGBK(strPath);

	FILE* fp = NULL;
	fp = fopen(strPath.c_str(),"wb");
	if(fp!=NULL)
	{
		fwrite(strNewMsg.c_str(),1,strNewMsg.size(),fp);
		fclose(fp);
	}
	chmod(strPath.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
}


/* 函数介绍：生成违章数据解析文件
 * 输入参数：strMsg：车牌数据内容
 * 输出参数：strNewMsg：生成的卡口车牌数据解析文件
 *           strPicHead：名称前缀
 * 返回值：生成解析文件的磁盘绝对路径
 */
string  CRoadImcData::GetViolationData(RECORD_PLATE& plate)
{
	char buf[1024] = {0};

	std::string strDirection;

    if(plate.uDirection==EAST_TO_WEST)
    {
        strDirection = "东";
    }
    else if(plate.uDirection == WEST_TO_EAST)
    {
        strDirection = "西";
    }
    else if(plate.uDirection == SOUTH_TO_NORTH)
    {
        strDirection = "南";
    }
    else if(plate.uDirection == NORTH_TO_SOUTH)
    {
        strDirection = "北";
    }

	sprintf(buf,"%d,%s,%s,%s,%s,%d, , , , .",GetViolationType(plate.uViolationType),g_ftpRemotePath,plate.chPlace,strDirection.c_str(),GetTime(plate.uTime).c_str(),plate.uRoadWayID);

	string strViolationData(buf);
	g_skpDB.UTF8ToGBK(strViolationData);

	printf("strViolationData=%s\n",strViolationData.c_str());
	
	return strViolationData;
}

/* 函数介绍：生成违章数据解析文件
* 输入参数：strMsg：车牌数据内容
* 输出参数：strNewMsg：生成的卡口车牌数据解析文件
*           strPicHead：名称前缀
* 返回值：生成解析文件的磁盘绝对路径
*/
string  CRoadImcData::GetViolationDataForNanning(RECORD_PLATE& plate, int nSpeedMax, string strJpgFile,string strFtpPath,string chJpgFileBuf1,string chJpgFileBuf2,string chJpgFileBuf3,string chJpgFileBuf4)
{
	char buf[256] = {0};

	string strText = plate.chText;
	g_skpDB.GBKToUTF8(strText);
	XMLNode InfoNode,childNode;
	InfoNode = XMLNode::createXMLTopNode("PicInfo");
	//经过时间
	childNode = InfoNode.addChild("JGSJ");
	sprintf(buf,"%s%03d",GetTime(plate.uTime, 2).c_str(),plate.uMiTime);
	childNode.addText(buf);
	//红灯开始时间
	childNode = InfoNode.addChild("HDKSSJ");
	sprintf(buf,"%s%03d",GetTime(plate.uRedLightBeginTime, 2).c_str(),plate.uRedLightBeginMiTime);
	childNode.addText(buf);
	//车牌号码
	childNode = InfoNode.addChild("HPHM");
	sprintf(buf,"%s",strText);
	childNode.addText(buf);
	//号牌颜色
	childNode = InfoNode.addChild("HPYS");
	sprintf(buf,"%d",plate.uColor);
	childNode.addText(buf);
	//号牌种类
	childNode = InfoNode.addChild("HPZL");
	sprintf(buf,"%02d",GetVechileTypeForNanning(plate.uType));
	childNode.addText(buf);
	//车辆类型
	childNode = InfoNode.addChild("CLLX");
	sprintf(buf,"%d",GetVechileTypeForNanning(plate.uType));
	childNode.addText(buf);
	//车辆颜色
	childNode = InfoNode.addChild("CLYS");
	sprintf(buf,"%s",GetCarColorForNanning(plate.uCarColor1).c_str());
	childNode.addText(buf);
	//车辆速度
	childNode = InfoNode.addChild("CLSD");
	sprintf(buf,"%d",plate.uSpeed);
	childNode.addText(buf);
	//测速类型 0:雷达 1:识别
	childNode = InfoNode.addChild("CSLX");
	sprintf(buf,"0");
	childNode.addText(buf);
	//速度差值
	childNode = InfoNode.addChild("SDCZ");
	sprintf(buf,"%d",(int)plate.uSpeed - nSpeedMax);
	childNode.addText(buf);
	//限速比
	childNode = InfoNode.addChild("XSB");
	sprintf(buf,"%d",(nSpeedMax > 0) ? (((int)plate.uSpeed - nSpeedMax)*100/nSpeedMax):0);
	childNode.addText(buf);
	//车辆标牌
	childNode = InfoNode.addChild("CLBP");
	sprintf(buf,"%s",g_strCarLabel[plate.uCarBrand]);
	childNode.addText(buf);
	//车外廓长
	childNode = InfoNode.addChild("CWKC");
	sprintf(buf,"%s","");
	childNode.addText(buf);
	//车外廓宽
	childNode = InfoNode.addChild("CWKK");
	sprintf(buf,"%s","");
	childNode.addText(buf);
	//车头距
	childNode = InfoNode.addChild("CTJ");
	sprintf(buf,"%s","");
	childNode.addText(buf);
	//号牌图片
	childNode = InfoNode.addChild("HPTP");
	sprintf(buf,"%s",chJpgFileBuf4.c_str());
	childNode.addText(buf);
	//全景1
	childNode = InfoNode.addChild("QJTP1");
	sprintf(buf,"%s",chJpgFileBuf1.c_str());
	childNode.addText(buf);
	//全景2 
	childNode = InfoNode.addChild("QJTP2");
	sprintf(buf,"%s",chJpgFileBuf2.c_str());
	childNode.addText(buf);
	//全景3
	childNode = InfoNode.addChild("QJTP3");
	sprintf(buf,"%s",chJpgFileBuf3.c_str());
	childNode.addText(buf);
	//合成图片
	childNode = InfoNode.addChild("HCTP");
	sprintf(buf,"%s",strJpgFile.c_str());
	childNode.addText(buf);
	//违法类型
	childNode = InfoNode.addChild("WFLX");
	sprintf(buf,"0");
	childNode.addText(buf);
	
	//设备编号
	childNode = InfoNode.addChild("SBBH");
	sprintf(buf,"%s%d%d",g_strDetectorID.c_str(),g_RoadImcData.GetDirectionForNanning(plate.uDirection),plate.uChannelID);
	childNode.addText(buf);

	//地点编号
	childNode = InfoNode.addChild("DDBH");
	sprintf(buf,"%s",g_strDetectorID.c_str());
	childNode.addText(buf);	
		
	//车道方向
	childNode = InfoNode.addChild("XSFX");
	sprintf(buf,"%d",GetDirectionForNanning(plate.uDirection));
	childNode.addText(buf);
	//车道编号，车道驶向
	childNode = InfoNode.addChild("CDBH");
	sprintf(buf,"%d",plate.uRoadWayID);
	childNode.addText(buf);
	//绝对路径或相对路径
	childNode = InfoNode.addChild("TPGML");
	sprintf(buf,"%s",strFtpPath.c_str());
	childNode.addText(buf);
	//违法行为
	childNode = InfoNode.addChild("WFXW");
	sprintf(buf,"%d",GetViolationTypeForNanning2(plate.uViolationType));
	childNode.addText(buf);
	//车牌左
	childNode = InfoNode.addChild("HPLEFT");
	sprintf(buf,"%d",plate.uPosLeft);
	childNode.addText(buf);
	//车牌右
	childNode = InfoNode.addChild("HPRIGHT");
	sprintf(buf,"%d",plate.uPosRight);
	childNode.addText(buf);
	//车牌上
	childNode = InfoNode.addChild("HPTOP");
	sprintf(buf,"%d",plate.uPosTop);
	childNode.addText(buf);
	//车牌下
	childNode = InfoNode.addChild("HPBOTTOM");
	sprintf(buf,"%d",plate.uPosBottom);
	childNode.addText(buf);

	string strXmlData;
	int nSize;
	XMLSTR strData = InfoNode.createXMLString(1, &nSize);
	if(strData)
	{
		strXmlData.append(strData, sizeof(XMLCHAR)*nSize);
		freeXMLString(strData);
	}
	char strXmlBuf[1024];
	sprintf(strXmlBuf,"%s","<?xml version =\"1.0\" encoding = \"GB2312\" standalone =\"yes\" ?>\n");
	char strBuf[102400];
	sprintf(strBuf,"%s%s",strXmlBuf,strXmlData.c_str());
	string strViolationData(strBuf);
	g_skpDB.UTF8ToGBK(strViolationData);

	printf("strViolationData=%s\n",strViolationData.c_str());

	return strViolationData;
}

// 车牌类型
int CRoadImcData::GetVechileTypeForNanning(int uTypeDetail)
{
	int nRetType = 0;

	switch (uTypeDetail)
	{
	case SMALL_CAR:
		nRetType = 1;
		break;
	case MIDDLE_CAR:
		nRetType = 2;
		break;
	case BIG_CAR:
		nRetType = 3;
		break;
	}

	return nRetType;
}

//获取方向解析
int CRoadImcData::GetDirectionForNanning(int nDirection)
{
	int nRetDirection = 1;

	switch (nDirection)
	{
	case EAST_TO_WEST://从东到西
		nRetDirection = 5;
		break;
	case WEST_TO_EAST://从西到东
		nRetDirection = 1;
		break;
	case SOUTH_TO_NORTH://从南到北
		nRetDirection = 3;
		break;
	case NORTH_TO_SOUTH://从北到南
		nRetDirection = 7;
		break;
	case SOUTHEAST_TO_NORTHWEST://由东南到西北
		nRetDirection = 4;
		break;
	case NORTHWEST_TO_SOUTHEAST: //由西北到东南
		nRetDirection = 8;
		break;
	case NORTHEAST_TO_SOUTHWEST: //由东北到西南
		nRetDirection = 6;
		break;
	case SOUTHWEST_TO_NORTHEAST: //由西南到东北
		nRetDirection = 2;
		break;
	}
	return nRetDirection;
}

/* 函数介绍：生成违章数据解析文件
* 输入参数：strMsg：车牌数据内容
* 输出参数：strNewMsg：生成的卡口车牌数据解析文件
*           strPicHead：名称前缀
* 返回值：生成解析文件的磁盘绝对路径
*/
string  CRoadImcData::GetViolationDataForJinan(RECORD_PLATE& plate, string strJpgFile)
{
	char buf[1024] = {0};

	int nDirection = GetDirectionForJinan(plate.uDirection);

	string strText = plate.chText;
	g_skpDB.GBKToUTF8(strJpgFile);
		

	int nCarNumberColor = 2;
	GetCarNumberColorStr(plate.uColor, nCarNumberColor);
	std::string strDeviceId = g_skpDB.GetDeviceId(plate.uCameraId);	

	if (g_nGongJiaoMode == 1)
	{
		sprintf(buf, "PASSTIME=%s\nCARSTATE=%d\nCAEPLATE=%s\nPLATETYPE=%02d\nSPEED=%d\nPLATECOLOR=%d\nLOCATIONID=%s\nDRIVEWAY=%d\nDRIVEDIR=%d\nCAPTUREDIR=2\nCARCOLOR=%02d\nCARBRAND=%02d\nTGSID=%s\nPLATECOORD=%d,%d,%d,%d\nCABCOORD=\nIMGID1=%s\nIMGID2=\nIMGID3=\n",			
			GetTime(plate.uTime, 0).c_str(),
			GetViolationTypeForJinan(plate.uViolationType),
			strText,GetVechileTypeForJinan(plate),
			plate.uSpeed,nCarNumberColor,plate.szLoctionID,
			plate.uRoadWayID,nDirection,GetCarColorForJinan(plate.uCarColor1),
			GetCarBrandForJinan(plate.uCarBrand),plate.szKaKouItem,plate.uPosLeft,plate.uPosTop,plate.uPosRight,plate.uPosBottom,
			strJpgFile.c_str());
	}
	else
	{
		if(g_DistanceHostInfo.bDistanceCalculate)
		{
			sprintf(buf, "PASSTIME=%s\nPASSTIME1=%s\nCARSTATE=%d\nCAEPLATE=%s\nPLATETYPE=%02d\nSPEED=%d\nPLATECOLOR=%d\nLOCATIONID=%s\nDRIVEWAY=%d\nDRIVEDIR=%d\nCAPTUREDIR=1\nCARCOLOR=%02d\nCARBRAND=%02d\nTGSID=%s\nPLATECOORD=%d,%d,%d,%d\nCABCOORD=\nIMGID1=%s\nIMGID2=\nIMGID3=\n",			
				GetTime(plate.uTime2, 0).c_str(),
				GetTime(plate.uTime, 0).c_str(),
				GetViolationTypeForJinan(plate.uViolationType),
				strText,GetVechileTypeForJinan(plate),
				plate.uSpeed,nCarNumberColor,strDeviceId.c_str(),
				plate.uRoadWayID,nDirection,GetCarColorForJinan(plate.uCarColor1),
				GetCarBrandForJinan(plate.uCarBrand),g_ftpRemotePath,plate.uPosLeft,plate.uPosTop,plate.uPosRight,plate.uPosBottom,
				strJpgFile.c_str());
		}
		else
		{
			sprintf(buf, "PASSTIME=%s\nCARSTATE=%d\nCAEPLATE=%s\nPLATETYPE=%02d\nSPEED=%d\nPLATECOLOR=%d\nLOCATIONID=%s\nDRIVEWAY=%d\nDRIVEDIR=%d\nCAPTUREDIR=1\nCARCOLOR=%02d\nCARBRAND=%02d\nTGSID=%s\nPLATECOORD=%d,%d,%d,%d\nCABCOORD=\nIMGID1=%s\nIMGID2=\nIMGID3=\n",
				GetTime(plate.uTime, 0).c_str(),
				GetViolationTypeForJinan(plate.uViolationType),
				strText,GetVechileTypeForJinan(plate),
				plate.uSpeed,nCarNumberColor,strDeviceId.c_str(),
				plate.uRoadWayID,nDirection,GetCarColorForJinan(plate.uCarColor1),
				GetCarBrandForJinan(plate.uCarBrand),g_ftpRemotePath,plate.uPosLeft,plate.uPosTop,plate.uPosRight,plate.uPosBottom,
				strJpgFile.c_str());
		}
	}
	
	string strViolationData(buf);
	//g_skpDB.UTF8ToGBK(strViolationData);

	printf("strViolationData=%s\n",strViolationData.c_str());

	return strViolationData;
}

string  CRoadImcData::GetViolationDataForkafka(RECORD_PLATE& plate, string strJpgFile)
{
	const int nBufSize = 2048;
	char buf[nBufSize] = {0};

	int nDirection = GetDirectionForJinan(plate.uDirection);

	string strText = plate.chText;
	if('0' == plate.chText[0] && '0'== plate.chText[1] && '0' == plate.chText[6] || \
		'1' == plate.chText[0] && '1'== plate.chText[1] && '1' == plate.chText[6])
	{
		//strText = "XXXXXXX";
		strText = "无牌";
	}

	//LogNormal("plate:%s text:%s ", plate.chText, strText);
	//LogNormal("1GetViolationDataForkafka strText:%s ", strText.c_str());
	//g_skpDB.GBKToUTF8(strText);
	//LogNormal("2GetViolationDataForkafka strText:%s ", strText.c_str());

	//卡口编号_车道号_行驶方向_通行时间_号牌号码_号牌种类_顺序号（000-999）
	sprintf(buf, "%s_%d_%d_%s_%s_%d_01", \
		g_ftpRemotePath,
		plate.uRoadWayID,
		nDirection,
		GetTime(plate.uTime,2),
		strText.c_str(),
		GetVechileTypeForJinan(plate)
		);
	strJpgFile = buf;
	//LogNormal("strJpgFile:%s ", strJpgFile.c_str());

	//if (strJpgFile.size() > 0)
	//{
	//	g_skpDB.GBKToUTF8(strJpgFile);
	//}

	int nCarNumberColor = 2;
	GetCarNumberColorStr(plate.uColor, nCarNumberColor);
	std::string strDeviceId = g_skpDB.GetDeviceId(plate.uCameraId);	
	#ifdef KAF_DEBUG
	LogNormal("uCameraId=%d,strDeviceId=%s\n",plate.uCameraId,strDeviceId.c_str());
	#endif

	struct timeval now;
	gettimeofday(&now, NULL);
	UINT32 nts = now.tv_usec/1000;
	memset(buf, 0, nBufSize);	
	
	//CAPTUREDIR=2,抓车尾部
	//CABCOORD驾驶舱坐标=0,0,0,0 
	sprintf(buf, 
		"IPADDR=%s\nSENDTIME=%s %03d\nPASSTIME=%s %03d\nCARSTATE=%d\nCARPLATE=%s\nPLATETYPE=%02d\nSPEED=%d\nPLATECOLOR=%d\nLOCATIONID=%s\nDRIVEWAY=%d\nDRIVEDIR=%d\nCAPTUREDIR=2\nCARCOLOR=%02d\nCARBRAND=%02d\nTGSID=%s\nPLATECOORD=%d,%d,%d,%d\nCABCOORD=0,0,0,0\nIMGID1=%s\nIMGID2=\nIMGID3=\n",
		GetIpAddress("eth1",1).c_str(),
		GetTimeCurrent().c_str(), nts,
		GetTime(plate.uTime, 0).c_str(), plate.uMiTime,
		GetViolationTypeForJinan(plate.uViolationType),
		strText.c_str(),
		GetVechileTypeForJinan(plate),
		plate.uSpeed,
		nCarNumberColor,
		strDeviceId.c_str(),
		plate.uRoadWayID,
		nDirection,
		GetCarColorForJinan(plate.uCarColor1),
		GetCarBrandForJinan(plate.uCarBrand),
		g_ftpRemotePath,
		plate.uPosLeft,	plate.uPosTop, plate.uPosRight, plate.uPosBottom,
		strJpgFile.c_str());

	string strViolationData(buf);

//	g_skpDB.GBKToUTF8(strViolationData);
//	printf("strViolationData=\n%s\n",strViolationData.c_str());
//	LogTrace("KafKa-mvs.log", "strViolationData: ## \n%s \n", strViolationData.c_str());

	return strViolationData;
}

// 获取车牌品牌识别
int CRoadImcData::GetCarBrandForJinan(int nCarBrand)
{
	int nRetCarBrand = 99;

	switch (nCarBrand)
	{
	case 0:
		nRetCarBrand = 3;
		break;
	case 1:
		nRetCarBrand = 4;
		break;
	case 2:
		nRetCarBrand = 89;
		break;
	case 3:
		nRetCarBrand = 7;
		break;
	case 4:
		nRetCarBrand = 9;
		break;
	case 5:
		nRetCarBrand = 21;
		break;
	case 6:
		nRetCarBrand = 64;
		break;
	case 7:
		nRetCarBrand = 30;
		break;
	case 8:
		nRetCarBrand = 12;
		break;
	case 9:
		nRetCarBrand = 90;
		break;
	case 10:
		nRetCarBrand = 91;
		break;
	case 11: //标志
		nRetCarBrand = 99;
		break;
	case 12:
		nRetCarBrand = 31;
		break;
	case 13:// 凌志
		nRetCarBrand = 99;
		break;
	case 14: // 尼桑
		nRetCarBrand = 99;
		break;
	case 15:
		nRetCarBrand = 71;
		break;
	case 16:
		nRetCarBrand = 10;
		break;
	case 17:
		nRetCarBrand = 72;
		break;
	case 18:
		nRetCarBrand = 75;
		break;
	case 19:
		nRetCarBrand = 78;
		break;
	case 20:
		nRetCarBrand = 83;
		break;
	case 21:
		nRetCarBrand = 60;
		break;
	case 22:
		nRetCarBrand = 15;
		break;
	case 23:
		nRetCarBrand = 29;
		break;
	case 24:
		nRetCarBrand = 86;
		break;
	case 25:
		nRetCarBrand = 43;
		break;
	case 26:
		nRetCarBrand = 62;
		break;
	case 27://通用
		nRetCarBrand = 99;
		break;
	case 28:
		nRetCarBrand = 37;
		break;
	case 29:
		nRetCarBrand = 36;
		break;
	case 30:
		nRetCarBrand = 46;
		break;
	case 31:
		nRetCarBrand = 44;
		break;
	case 32://江铃
		nRetCarBrand = 99;
		break;
	case 33:
		nRetCarBrand = 42;
		break;
	case 34:
		nRetCarBrand = 61;
		break;
	case 35:
		nRetCarBrand = 56;
		break;
	case 36://名爵
		nRetCarBrand = 99;
		break;
	case 37:
		nRetCarBrand = 68;
		break;
	case 38:
		nRetCarBrand = 94;
		break;
	case 39:
		nRetCarBrand = 96;
		break;
	case 40:
		nRetCarBrand = 98;
		break;
	case 41:
		nRetCarBrand = 85;
		break;
	case 42://斯巴鲁
		nRetCarBrand = 99;
		break;
	case 43:
		nRetCarBrand = 79;
		break;
	case 44:
		nRetCarBrand = 80;
		break;
	case 45:
		nRetCarBrand = 77;
		break;
	case 46:
		nRetCarBrand = 84;
		break;
	case 47:
		nRetCarBrand = 73;
		break;
	case 48:
		nRetCarBrand = 41;
		break;
	case 49:
		nRetCarBrand = 39;
		break;
	case 50:
		nRetCarBrand = 38;
		break;
	case 51:
		nRetCarBrand = 34;
		break;
	case 52:
		nRetCarBrand = 26;
		break;
	case 53:
		nRetCarBrand = 23;
		break;
	case 54:
		nRetCarBrand = 16;
		break;
	case 55:
		nRetCarBrand = 18;
		break;
	case 56:
		nRetCarBrand = 17;
		break;
	case 57://大宇
		nRetCarBrand = 99;
		break;
	case 58://五十铃
		nRetCarBrand = 99;
		break;
	case 59:
		nRetCarBrand = 20;
		break;
	case 60:
		nRetCarBrand = 45;
		break;
	case 61:
		nRetCarBrand = 69;
		break;
	case 62:
		nRetCarBrand = 51;
		break;
	case 63://罗密欧
		nRetCarBrand = 99;
		break;
	case 64:
		nRetCarBrand = 59;
		break;
	case 65:
		nRetCarBrand = 53;
		break;
	case 66:
		nRetCarBrand = 27;
		break;
	case 67:
		nRetCarBrand = 5;
		break;
	case 68:
		nRetCarBrand = 57;
		break;
	case 69://阿斯顿马丁
		nRetCarBrand = 99;
		break;
	case 70://皇冠
		nRetCarBrand = 99;
		break;
	case 71:
		nRetCarBrand = 8;
		break;
	case 72:
		nRetCarBrand = 24;
		break;
	case 73://中顺
		nRetCarBrand = 99;
		break;
	case 74:
		nRetCarBrand = 32;
		break;
	case 75:
		nRetCarBrand = 87;
		break;
	case 76:
		nRetCarBrand = 49;
		break;
	case 77:
		nRetCarBrand = 65;
		break;
	case 78:
		nRetCarBrand = 88;
		break;
	case 79:
		nRetCarBrand = 55;
		break;
	case 80://依维柯
		nRetCarBrand = 99;
		break;
	case 81:
		nRetCarBrand = 13;
		break;
	case 82:
		nRetCarBrand = 67;
		break;
	case 83://双龙
		nRetCarBrand = 99;
		break;
	case 84://汇众
		nRetCarBrand = 99;
		break;
	case 85:
		nRetCarBrand = 93;
		break;
	case 86://开瑞
		nRetCarBrand = 99;
		break;
	case 87://南汽
		nRetCarBrand = 99;
		break;
	case 88://跃进
		nRetCarBrand = 99;
		break;
	case 89://解放
		nRetCarBrand = 99;
		break;
	case 90://金龙
		nRetCarBrand = 99;
		break;
	case 91:
		nRetCarBrand = 99;
		break;
	case 92:
		nRetCarBrand = 97;
		break;
	case 93:
		nRetCarBrand = 76;
		break;
	case 94://曙光
		nRetCarBrand = 99;
		break;
	case 95:
		nRetCarBrand = 6;
		break;
	case 96:
		nRetCarBrand = 35;
		break;
	case 97://中国重汽
		nRetCarBrand = 99;
		break;
	case 98://陕汽重卡
		nRetCarBrand = 99;
		break;
	case 99://北奔重卡
		nRetCarBrand = 99;
		break;
	case 100://宇通客车
		nRetCarBrand = 99;
		break;
	case 101://羊城汽车
		nRetCarBrand = 99;
		break;
	case 102://福迪汽车
		nRetCarBrand = 99;
		break;
	case 103:
		nRetCarBrand = 33;
		break;
	case 104://华德
		nRetCarBrand = 99;
		break;
	case 105://楚风
		nRetCarBrand = 99;
		break;
	case 106://春兰
		nRetCarBrand = 99;
		break;
	case 107://大运
		nRetCarBrand = 99;
		break;
	case 108:
		nRetCarBrand = 24;
		break;
	case 109://佛山飞驰
		nRetCarBrand = 99;
		break;
	case 110://日野
		nRetCarBrand = 99;
		break;
	case 111://红岩
		nRetCarBrand = 99;
		break;
	case 112://华菱
		nRetCarBrand = 99;
		break;
	case 113://金旅
		nRetCarBrand = 99;
		break;
	case 114://联合
		nRetCarBrand = 99;
		break;
	case 115://曼
		nRetCarBrand = 99;
		break;
	case 116://青年
		nRetCarBrand = 99;
		break;
	case 117:
		nRetCarBrand = 74;
		break;
	case 118://神野
		nRetCarBrand = 99;
		break;
	case 119://
		nRetCarBrand = 99;
		break;
	case 120://斯堪尼亚
		nRetCarBrand = 99;
		break;
	case 121://五征
		nRetCarBrand = 99;
		break;
	case 122://徐工
		nRetCarBrand = 99;
		break;
	case 123://中通
		nRetCarBrand = 99;
		break;
	case 124://王牌
		nRetCarBrand = 99;
		break;
	}
	return nRetCarBrand;
}

//获取车辆违法类型解析
int CRoadImcData::GetViolationTypeForJinan(int nViolationType)
{
	int nRetViolationType = 1;
	switch(nViolationType)
	{
	case DETECT_RESULT_EVENT_GO_FAST://6 超速
		{
			nRetViolationType = 3;
			break;
		}
	case DETECT_RESULT_RED_LIGHT_VIOLATION: //16 闯红灯
		{
			nRetViolationType = 2;
			break;
		}
	case DETECT_RESULT_TAKE_UP_NONMOTORWAY: //55 机占非
		{
			nRetViolationType = 7;
			break;
		}
	case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD: //18 小车出现在禁行车道
	case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD: //19 大车出现在禁行车道
	case DETECT_RESULT_EVNET_GO_EMERGENCY_CHANNEL: //22 走应急车道
	case DETECT_RESULT_OBV_TAKE_UP_BUSWAY://32占用公交道
	case DETECT_RESULT_NO_PASSING: //28 禁行所有车辆
		{
			nRetViolationType = 8;
			break;
		}
	case DETECT_RESULT_FORBID_LEFT: //23 禁止左拐
	case DETECT_RESULT_FORBID_RIGHT: //24 禁止右拐
	case DETECT_RESULT_FORBID_STRAIGHT: //25 禁止前行
		{
			nRetViolationType = 6;
			break;
		}
	case DETECT_RESULT_RETROGRADE_MOTION: //26 逆行
		{
			nRetViolationType = 4;
			break;
		}
	case DETECT_RESULT_PRESS_LINE: //27 压线
	case DETECT_RESULT_PRESS_WHITELINE: //27 压线
		{
			nRetViolationType = 5;
			break;
		}
	case DETECT_RESULT_ELE_EVT_BIANDAO: //29 变道
		{
			nRetViolationType = 6;
			break;
		}
	case DETECT_RESULT_NOCARNUM:  //38 无牌车出现
		{
			nRetViolationType = 10;
			break;
		}
	case DETECT_RESULT_NO_PARKING:  //65 黄网格停车
		{
			nRetViolationType = 9;
			break;
		}
	case DETECT_NOT_CUTRTESY_DRIVE:  //69 礼让行人
		{
			nRetViolationType = 12;
			break;
		}
	default:
		{
			break;
		}
	}

	return nRetViolationType;
}

//获取车辆违法类型解析
int CRoadImcData::GetViolationTypeForNanning(int nViolationType)
{
	int nRetViolationType = 2;
	switch(nViolationType)
	{
	case DETECT_RESULT_EVENT_GO_FAST://6 超速
		{
			nRetViolationType = 3;
			break;
		}
	case DETECT_RESULT_RED_LIGHT_VIOLATION: //16 闯红灯
		{
			nRetViolationType = 1;
			break;
		}
	case DETECT_RESULT_PARKING_VIOLATION: //17 违章停车
		{
			nRetViolationType = 9;
			break;
		}
	case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD: //18 小车出现在禁行车道
		{
			nRetViolationType = 6;
			break;
		}
	case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD: //19 大车出现在禁行车道
		{
			nRetViolationType = 6;
			break;
		}
	case DETECT_RESULT_EVNET_GO_EMERGENCY_CHANNEL: //22 走应急车道
		{
			nRetViolationType = 11;
			break;
		}
	case DETECT_RESULT_FORBID_LEFT: //23 禁止左拐
		{
			nRetViolationType = 7;
			break;
		}
	case DETECT_RESULT_FORBID_RIGHT: //24 禁止右拐
		{
			nRetViolationType = 7;
			break;
		}
	case DETECT_RESULT_FORBID_STRAIGHT: //25 禁止前行
		{
			nRetViolationType = 6;
			break;
		}
	case DETECT_RESULT_RETROGRADE_MOTION: //26 逆行
		{
			nRetViolationType = 4;
			break;
		}
	case DETECT_RESULT_PRESS_LINE: //27 压线
	case DETECT_RESULT_PRESS_WHITELINE: //27 压线
		{
			nRetViolationType = 8;
			break;
		}
	case DETECT_RESULT_NO_PASSING: //28 禁行所有车辆
		{
			nRetViolationType = 6;
			break;
		}
	case DETECT_RESULT_ELE_EVT_BIANDAO: //29 变道
		{
			nRetViolationType = 5;
			break;
		}
	case DETECT_RESULT_NOCARNUM:  //38 无牌车出现
		{
			nRetViolationType = 2;
			break;
		}
	default:
		{
			break;
		}
	}

	return nRetViolationType;
}

//获取车辆违法类型解析
int CRoadImcData::GetViolationTypeForNanning2(int nViolationType)
{
	int nRetViolationType = 0;
	switch(nViolationType)
	{
	case DETECT_RESULT_EVENT_GO_FAST://6 超速
		{
			nRetViolationType = 1231;
			break;
		}
	case DETECT_RESULT_RED_LIGHT_VIOLATION: //16 闯红灯
		{
			nRetViolationType = 1229;
			break;
		}
	case DETECT_RESULT_PARKING_VIOLATION: //17 违章停车
		{
			nRetViolationType = 1230;
			break;
		}
	case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD: //18 小车出现在禁行车道
		{
			nRetViolationType = 1229;
			break;
		}
	case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD: //19 大车出现在禁行车道
		{
			nRetViolationType = 1229;
			break;
		}
	case DETECT_RESULT_EVNET_GO_EMERGENCY_CHANNEL: //22 走应急车道
		{
			nRetViolationType = 1232;
			break;
		}
	case DETECT_RESULT_FORBID_LEFT: //23 禁止左拐
		{
			nRetViolationType = 1229;
			break;
		}
	case DETECT_RESULT_FORBID_RIGHT: //24 禁止右拐
		{
			nRetViolationType = 1229;
			break;
		}
	case DETECT_RESULT_FORBID_STRAIGHT: //25 禁止前行
		{
			nRetViolationType = 1229;
			break;
		}
	case DETECT_RESULT_RETROGRADE_MOTION: //26 逆行
		{
			nRetViolationType = 1301;
			break;
		}
	case DETECT_RESULT_PRESS_LINE: //27 压线
	case DETECT_RESULT_PRESS_WHITELINE: //27 压线
		{
			nRetViolationType = 1231;
			break;
		}
	case DETECT_RESULT_NO_PASSING: //28 禁行所有车辆
		{
			nRetViolationType = 1229;
			break;
		}
	case DETECT_RESULT_ELE_EVT_BIANDAO: //29 变道
		{
			nRetViolationType = 1231;
			break;
		}
	case DETECT_RESULT_NOCARNUM:  //38 无牌车出现
		{
			nRetViolationType = 1231;
			break;
		}
	default:
		{
			break;
		}
	}

	return nRetViolationType;
}

//获取车辆违法类型解析
int CRoadImcData::GetViolationType(int nViolationType)
{
    int nRetViolationType = 0;
    switch(nViolationType)
    {
		case DETECT_RESULT_EVENT_GO_FAST://6 超速
			{
				nRetViolationType = 8713;
				break;
			}
        case DETECT_RESULT_RED_LIGHT_VIOLATION: //16 闯红灯
            {
                nRetViolationType = 8633;
                break;
            }
        case DETECT_RESULT_PARKING_VIOLATION: //17 违章停车
            {
				nRetViolationType = 1039;
                break;
            }
        case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD: //18 小车出现在禁行车道
            {
				nRetViolationType = 1355;
                break;
            }
        case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD: //19 大车出现在禁行车道
            {
				nRetViolationType = 1355;
                break;
            }
        case DETECT_RESULT_EVNET_GO_EMERGENCY_CHANNEL: //22 走应急车道
            {
				nRetViolationType = 8635;
                break;
            }
        case DETECT_RESULT_FORBID_LEFT: //23 禁止左拐
            {
				nRetViolationType = 8340;
                break;
            }
        case DETECT_RESULT_FORBID_RIGHT: //24 禁止右拐
            {
				nRetViolationType = 8340;
                break;
            }
        case DETECT_RESULT_FORBID_STRAIGHT: //25 禁止前行
            {
				nRetViolationType = 83039;
                break;
            }
        case DETECT_RESULT_RETROGRADE_MOTION: //26 逆行
            {
				 nRetViolationType = 1301;
                break;
            }
        case DETECT_RESULT_PRESS_LINE: //27 压线
		case DETECT_RESULT_PRESS_WHITELINE: //27 压线
            {
				 nRetViolationType = 1345;
                break;
            }
        case DETECT_RESULT_NO_PASSING: //28 禁行所有车辆
            {
				nRetViolationType = 1355;
                break;
            }
        case DETECT_RESULT_ELE_EVT_BIANDAO: //29 变道
            {
				nRetViolationType = 1043;
                break;
            }
        case DETECT_RESULT_NOCARNUM:  //38 无牌车出现
            {
				nRetViolationType = 8715;
                break;
            }
        default:
            {
                break;
            }
    }

    return nRetViolationType;
}

//生成流量统计配置文件(nRegionType:区域类型;nStatisticType:流量数据类型0:流量统计数据，1：流量评价数据)
string  CRoadImcData::AddStatisticData(const String &strMsg,int nRegionType,int nStatisticType)
{
	char buf[256] = {0};

	RECORD_STATISTIC* pStatistic = (RECORD_STATISTIC *)(strMsg.c_str() + sizeof(SRIP_DETECT_HEADER));

	int nDirection  = g_skpDB.GetDirection(1);
	int nDirection2 = 1;
	GetDirectionStr(nDirection,nDirection2);

	string strTime = GetTime(pStatistic->uTime,2);//时间
	string strData = GetTime(pStatistic->uTime,4);//日期
	string strHour1 = GetTime(pStatistic->uTime,5);//小时

	string strFtpDir = "../flow/";
	if (IsDataDisk())
	{
	    strFtpDir = "/detectdata/flow/";
    }

	 if(access(strFtpDir.c_str(),0) != 0) //目录不存在
	{
		mkdir(strFtpDir.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		chmod(strFtpDir.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	}

	string strFtpTimeDir = strFtpDir + strData;
	strFtpTimeDir += strHour1;

	//if(g_nFtpServer == 1)
	{
		if(access(strFtpTimeDir.c_str(),0) != 0) //目录不存在
		{
			mkdir(strFtpTimeDir.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
		}
	}
	
	
	String strNewMsg;
	strNewMsg = AddNewItemValue(strNewMsg, false, "RoadNO", g_ftpRemotePath);//路口编号
	
	sprintf(buf, "%d",nDirection2);
	string strDirection(buf);
	strNewMsg = AddNewItemValue(strNewMsg, false, "DirectionNo", strDirection);//方向编码
	
	int nCount = 0;
	for(int i=0; i<MAX_ROADWAY; i++)
	{
		if(pStatistic->uFlux[i]!=0xffffffff)
		{
			nCount++;
		}
	}
	
	sprintf(buf, "%d",nCount);
	string strCount(buf);
	strNewMsg = AddNewItemValue(strNewMsg, false, "ActRowNum", strCount);//实际车道数

	//流量类型
	int nStatType = 16;

	if(nStatisticType == 0)
	{
		if(pStatistic->uStatTimeLen == 30)
		{
			nStatType = 13;
		}
		else if(pStatistic->uStatTimeLen == 60)
		{
			nStatType = 16;
		}
		else if(pStatistic->uStatTimeLen == 300)
		{
			nStatType = 15;
		}
	}
	else
	{
		nStatType = 02;

		string strDetectTime = GetTime(pStatistic->uTime, 0);//采集时间
		strDetectTime = strDetectTime.substr(11, 8);
		strNewMsg = AddNewItemValue(strNewMsg, false, "ConnectTime", strDetectTime);//采集时间

		sprintf(buf, "%d",pStatistic->uStatTimeLen);
		string strTimeLegth(buf);
		strNewMsg = AddNewItemValue(strNewMsg, false, "ConnectDuration", strTimeLegth);//统计时长

		sprintf(buf, "%d", pStatistic->uSeq);
		string strNumber(buf);
		strNewMsg = AddNewItemValue(strNewMsg, false, "Connectnumber", strNumber);//采集序数
		
		if(pStatistic->uSeq < 4)
		{
			strNewMsg = AddNewItemValue(strNewMsg, false, "Connecttriger", "1");//采集触发端
		}
		else
		{
			strNewMsg = AddNewItemValue(strNewMsg, false, "Connecttriger", "2");//采集触发端
		}
		
	}

	if( (nStatisticType == 1)&& (nRegionType == PERSON_ROAD))
	{
		//车道是否占有
		for(int i=0; i<MAX_ROADWAY; i++)
		{
			if(pStatistic->uFlux[i]!=0xffffffff)
			{
				unsigned int uRoadIndex = pStatistic->uRoadType[i];
				uRoadIndex   =    uRoadIndex>>16;

				sprintf(buf, "Road%dPoss",uRoadIndex);
				string strRoadIndex(buf);

				sprintf(buf, "%d",pStatistic->uSpace[i]);
				string strPoss(buf);

				strNewMsg = AddNewItemValue(strNewMsg, false, strRoadIndex.c_str(), strPoss);//
			}
		}

		//车道占有率
		for(int i=0; i<MAX_ROADWAY; i++)
		{
			if(pStatistic->uFlux[i]!=0xffffffff)
			{
				unsigned int uRoadIndex = pStatistic->uRoadType[i];
				uRoadIndex   =    uRoadIndex>>16;

				sprintf(buf, "Road%dShare",uRoadIndex);
				string strRoadIndex(buf);

				sprintf(buf, "%d",pStatistic->uOccupancy[i]);
				string strOccupancy(buf);

				strNewMsg = AddNewItemValue(strNewMsg, false, strRoadIndex.c_str(), strOccupancy);//
			}
		}
	}
	else
	{
		for(int i=0; i<MAX_ROADWAY; i++)
		{
			if(pStatistic->uFlux[i]!=0xffffffff)
			{
				unsigned int uRoadIndex = pStatistic->uRoadType[i];
				uRoadIndex   =    uRoadIndex>>16;

				sprintf(buf, "%d",uRoadIndex);
				string strRoadIndex(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "CarRoad", strRoadIndex);//车道号

				unsigned int uObjectNum = (pStatistic->uFlux[i])&(0x0000ffff);
				sprintf(buf, "%d",uObjectNum);
				string strFlow(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "Flow", strFlow);//流量

				unsigned int uBigNum = (pStatistic->uFlux[i])>>16;
				sprintf(buf, "%d",uBigNum);
				string strBigFlow(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "LongCarFlow", strBigFlow);//大车流量

				unsigned int uSmallNum = (pStatistic->uFluxCom[i])&(0x0000ffff);
				sprintf(buf, "%d",uSmallNum);
				string strSmallFlow(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "SmallCarFlow", strSmallFlow);//小车流量

				sprintf(buf, "%d", pStatistic->uSpace[i]);//暂时为空
				string strTotalInterval(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "TotalInterval", strTotalInterval);//累计间隔时间

				sprintf(buf, "%d", pStatistic->uOccupancy[i]);//暂时为空
				string strTotalPossessTime(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "TotalPossessTime", strTotalPossessTime);//累计占有时间

				sprintf(buf, "%d",pStatistic->uBigCarSpeed[i]);
				string strBigCarSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "LongCarSpeed", strBigCarSpeed);//大车平均车速

				sprintf(buf, "%d",pStatistic->uSmallCarSpeed[i]);
				string strSmallCarSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "SmallCarSpeed", strSmallCarSpeed);//小车平均车速

				sprintf(buf, "%d",pStatistic->uSpeed[i]);
				string strAverageSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "AverageSpeed", strAverageSpeed);//平均车速

				sprintf(buf, "%d",pStatistic->uMaxSpeed[i]);
				string strMaxSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "BiggestSpeed", strMaxSpeed);//最大车速
			}
		}
	}

	//
	string strHour = strTime.substr(8,2);
	string strMinute = strTime.substr(10,2);
	string strSecond = strTime.substr(12,2);

	int nHour = atoi(strHour.c_str());
	int nMinute = atoi(strMinute.c_str());
	int nSecond = atoi(strSecond.c_str());

	int nTime = nHour*3600+nMinute*60+nSecond;

	int nSeq = nTime/pStatistic->uStatTimeLen;
	
	if(nStatisticType == 0)
	{
		sprintf(buf, "%s/%s-%d-%02d-%d-%s-%d.ini", \
			strFtpTimeDir.c_str(),g_ftpRemotePath,nDirection2,nStatType,nRegionType,strData.c_str(),nSeq);
	}
	else
	{
		string strDateTime = GetTime(pStatistic->uTime,6);
		sprintf(buf, "%s/%s-%d-%02d-%d-%s-%s.ini", \
			strFtpTimeDir.c_str(),g_ftpRemotePath,nDirection2,nStatType,nRegionType,strData.c_str(),strDateTime.c_str());
	}
	

	string strPath(buf);

	printf("===strPath.c_str()=%s\n",strPath.c_str());
	
	//if(g_nFtpServer == 1)
	{
		FILE* fp = NULL;
		fp = fopen(strPath.c_str(),"wb");
		if(fp!=NULL)
		{
			fwrite(strNewMsg.c_str(),1,strNewMsg.size(),fp);
			fclose(fp);
		}
	}
	chmod(strPath.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);

	//g_skpDB.UTF8ToGBK(strPath);
	return strPath;
}



/* 函数介绍：加入新类型和值
 * 输入参数：strMsg：需要后面添加数据的字符串，bFather:是否是父节点，strItem：节点名称，strValue：节点内容
 * 输出参数：无
 * 返回值：添加完新类型数据的字符串（strNewMsg）
 */
std::string  CRoadImcData::AddNewItemValue(const String &strMsg, bool bFather, const char *szItem,\
                                            const String &strValue, const bool bCharType)
{
    printf("==CRoadImcData::AddNewItemValue==szItem=%s===strValue=%s====\n", szItem, strValue.c_str());
    String strNewMsg;
    String strItem(szItem);

    if(bFather)
    {
        strNewMsg += strMsg;
        strNewMsg += "[";
        strNewMsg += strItem;
        strNewMsg += "]\r\n";
    }
    else
    {
        string strValue2 = strValue;
        if(bCharType)
        {
           // g_skpDB.UTF8ToGBK(strValue2); //使用GBK2132标准
        }

        strNewMsg += strMsg;
        strNewMsg += strItem;
        strNewMsg += "=";
        strNewMsg += strValue2;
        strNewMsg += "\r\n";
    }

    return strNewMsg;
}


/* 函数介绍：图像切分
 * 输入参数：strMsg：源图片的内容，
 *           nPicSize: 图像大小
 *           nDataType:数据类型 1：车牌 2：事件，
 *           nCount：分解的图片数量，
 *           uPicWidth:源图像宽度
 *            uPicHeight:源图像高度
 *           strPicHead:图片名称前缀
 * 输出参数：strPicPath：切分图像后的存放地址
 *           nValueArry: 图像大小
 * 返回值：bool 是否切分图像成功
 */
bool CRoadImcData::CutImgData(const String &strMsg, const int nPicSize, const int nDataType, const int nCount,\
                            const unsigned int uPicWidth, const unsigned int uPicHeight,\
                             String& strPic0,String& strPic1,String& strPic2, int nValueArry[3])
{
    printf("======CRoadImcData::CutImgData===uPicWidth=%d==uPicHeight=%d==\n", uPicWidth, uPicHeight);
    bool bCutImgOK = false;

    UINT32 uPicHeight2 = uPicHeight / nCount; //取平均等分
    UINT32 uImageSize = uPicWidth*uPicHeight2*3;

    CxImage image;

    //大图-先解码
    image.Decode((unsigned char*)strMsg.c_str() + sizeof(MIMAX_HEADER) + sizeof(RECORD_PLATE), nPicSize, 3);



    if(image.IsValid()&&(image.GetWidth() == uPicWidth)&&(image.GetHeight() == uPicHeight))
    {
        CxImage imageA;
        unsigned char* pImageData = NULL;
        BYTE* pJpgImage = new BYTE[uPicWidth*uPicHeight2/4];
        for(int i=0; i<nCount; i++)
        {
            pImageData = image.GetBits()+i*uImageSize;

            int srcstep = 0;
            imageA.IppEncode(pImageData, uPicWidth, uPicHeight2, 3, &srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);

            if(i == 2)
            {
                strPic2.append((char*)pJpgImage,srcstep);
                //nValueArry[2] = srcstep; //取得图片大小
            }
            else if(i == 1)
            {
                strPic1.append((char*)pJpgImage,srcstep);
                ///nValueArry[1] = srcstep; //取得图片大小
            }
            else if(i == 0)
            {
                strPic0.append((char*)pJpgImage,srcstep);
                //nValueArry[0] = srcstep; //取得图片大小
            }
        }
        if(pJpgImage)
        delete []pJpgImage;
        bCutImgOK = true;
    }

    return bCutImgOK;
}

//获取方向解析
int CRoadImcData::GetDirectionForJinan(int nDirection)
{
	int nRetDirection = 1;

	if(nDirection==EAST_TO_WEST)
	{
		nRetDirection = 2;
	}
	else if(nDirection == WEST_TO_EAST)
	{
		nRetDirection = 4;
	}
	else if(nDirection == SOUTH_TO_NORTH)
	{
		nRetDirection = 1;
	}
	else if(nDirection == NORTH_TO_SOUTH)
	{
		nRetDirection = 3;
	}
	return nRetDirection;
}

//获取方向解析
std::string CRoadImcData::GetDirectionStr(int nDirection, int &nDirection2)
{
    String strDirection = "";
    switch(nDirection)
    {
        case EAST_TO_WEST:
        {
            nDirection2 = 1;
            strDirection = "东向西";
            break;
        }
        case WEST_TO_EAST:
        {
            nDirection2 = 3;
            strDirection = "西向东";
            break;
        }
        case SOUTH_TO_NORTH:
        {
            nDirection2 = 2;
            strDirection = "南向北";
            break;
        }
        case NORTH_TO_SOUTH:     //从北到南
        {
            nDirection2 = 4;
            strDirection = "北向南";
            break;
        }
        case SOUTHEAST_TO_NORTHWEST: //由东南到西北
        {
            nDirection2 = 34;
            strDirection = "东南向西北";
            break;
        }
        case NORTHWEST_TO_SOUTHEAST: //由西北到东南
        {
            nDirection2 = 12;
            strDirection = "西北向东南";
            break;
        }
        case NORTHEAST_TO_SOUTHWEST: //由东北到西南
        {
            nDirection2 = 32;
            strDirection = "东北向西南";
            break;
        }
        case  SOUTHWEST_TO_NORTHEAST: //由西南到东北
        {
            nDirection2 = 14;
            strDirection = "西南向东北";
            break;
        }
        default:
        {
            strDirection = "";
            break;
        }
    }
   // if(strDirection.size() > 0)
	//g_skpDB.UTF8ToGBK(strDirection);
    return strDirection;
}

//获取车身颜色解析
string CRoadImcData::GetCarColorForNanning(int nColor)
{
	char strCarColor[8];
	memset(strCarColor,0x00,8);

	switch(nColor)
	{
	case 0://白色 0 by default
		sprintf(strCarColor,"A");
		break;
	case 1://银色 1
		sprintf(strCarColor,"Z");
		break;
	case 2://黑色 2
		sprintf(strCarColor,"J");
		break;
	case 3://红色 3
		sprintf(strCarColor,"E");
		break;
	case 4://紫色 4
		sprintf(strCarColor,"F");
		break;
	case 5://蓝色 5
		sprintf(strCarColor,"H");
		break;
	case 6://黄色 6
		sprintf(strCarColor,"C");
		break;
	case 7://绿色 7
		sprintf(strCarColor,"G");
		break;
	case 8://褐色 8
		sprintf(strCarColor,"I");
		break;
	case 9://粉红色 9
		sprintf(strCarColor,"D");
		break;
	case 10://灰色 10
		sprintf(strCarColor,"B");
		break;
	default://未知 11
		sprintf(strCarColor,"Z");
	}

	return strCarColor;
}
//获取车身颜色解析
int CRoadImcData::GetCarColorForJinan(int nColor)
{
	int nCarColor = 99;
	switch(nColor)
	{
	case 0://白色 0 by default
		nCarColor = 1;
		break;
	case 1://银色 1
		nCarColor = 1;
		break;
	case 2://黑色 2
		nCarColor = 10;
		break;
	case 3://红色 3
		nCarColor = 5;
		break;
	case 4://紫色 4
		nCarColor = 6;
		break;
	case 5://蓝色 5
		nCarColor = 8;
		break;
	case 6://黄色 6
		nCarColor = 3;
		break;
	case 7://绿色 7
		nCarColor = 7;
		break;
	case 8://褐色 8
		nCarColor = 9;
		break;
	case 9://粉红色 9
		nCarColor = 4;
		break;
	case 10://灰色 10
		nCarColor = 2;
		break;
	default://未知 11
		nCarColor = 99;
	}

	return nCarColor;
}

//获取车身颜色解析
std::string CRoadImcData::GetCarColorStr(int nColor)
{
    String strCarColor = "";
    switch(nColor)
    {
        case 0://白色 0 by default
            strCarColor = "白色";
            break;
        case 1://银色 1
            strCarColor = "银色";
            break;
        case 2://黑色 2
            strCarColor = "黑色";
            break;
        case 3://红色 3
            strCarColor = "红色";
            break;
        case 4://紫色 4
            strCarColor = "紫色";
            break;
        case 5://蓝色 5
            strCarColor = "蓝色";
            break;
        case 6://黄色 6
            strCarColor = "黄色";
            break;
        case 7://绿色 7
            strCarColor = "绿色";
            break;
        case 8://褐色 8
            strCarColor = "褐色";
            break;
        case 9://粉红色 9
            strCarColor = "粉红色";
            break;
        case 10://灰色 10
            strCarColor = "灰色";
            break;
        default://未知 11
            strCarColor = "未知";
    }
    // if(strCarColor.size() > 0)
	//g_skpDB.UTF8ToGBK(strCarColor);
    return strCarColor;
}

//获取号牌颜色解析
//0:白 1:黄 2:蓝 3:黑 4:其他 见GA497-2009附录A
std::string CRoadImcData::GetCarNumberColorStr(int nNumberColor, int &nNumberColor2)
{
    String strCarNumColor = "";
    switch(nNumberColor)
    {
        case 1: //blue
            nNumberColor2 = 2;
            strCarNumColor = "蓝色";
            break;
        case 2: //black
            nNumberColor2 = 3;
            strCarNumColor = "黑色";
            break;
        case 3: //yellow
            nNumberColor2 = 1;
            strCarNumColor = "黄色";
            break;
        case 4: //white
            nNumberColor2 = 0;
            strCarNumColor = "白色";
            break;
        default: //other
            nNumberColor2 = 4;
            strCarNumColor = "其他";
    }
  //   if(strCarNumColor.size() > 0)
	//g_skpDB.UTF8ToGBK(strCarNumColor);
    return strCarNumColor;
}

//获取号牌种类解析(1.普通，2.警车，3其他)
std::string CRoadImcData::GetPlateTypeStr(std::string strCarNumber)
{
    String strPlateType = "";

    int nSize = strCarNumber.size();

    if(nSize > 0)
    {
		char ch3 = *((char*)(strCarNumber.c_str()+nSize-3));
        char ch2 = *((char*)(strCarNumber.c_str()+nSize-2));
        char ch1 = *((char*)(strCarNumber.c_str()+nSize-1));        

        if((ch3 == 0xffffffe8) && (ch2 == 0xffffffad) && (ch1 == 0xffffffa6))
        {
            strPlateType = "警车";
        }
        else if((ch1 >= '0' && ch1 <= '9') ||(ch1 >= 'a' && ch1 <= 'z') ||(ch1 >= 'A' && ch1 <= 'Z') )
        {
            strPlateType = "普通";
        }
        else
        {
            strPlateType = "其他";
        }

		//LogNormal("Number=%s,ch1=%x,ch2=%x strPlateType:%s \n", \
		//	strCarNumber.c_str(),ch1,ch2, strPlateType.c_str());
    }
    else
    {
        strPlateType = "其他";
    }

  //   if(strPlateType.size() > 0)
//	g_skpDB.UTF8ToGBK(strPlateType);
    return strPlateType;
}

//获取车辆类型
std::string CRoadImcData::GetVechileType(int nVechileType)
{
	String strVechileType = "";

	if(nVechileType == 1)
	{
		
		strVechileType = "小";
	}
	else if(nVechileType == 2)
	{
		
		strVechileType = "中";
	}
	else if(nVechileType == 3)
	{
		
		strVechileType = "大";
	}
	else
	{
		strVechileType = "其他";
	}
	

//	if(strVechileType.size() > 0)
//		g_skpDB.UTF8ToGBK(strVechileType);

	return strVechileType;
}

//获取车辆类型
int CRoadImcData::GetVechileTypeForJinan(RECORD_PLATE& plate)
{
	int nVechileType = 99;
	//LogNormal("[%s]:plateType = %d\n",__FUNCTION__,plate.uType);
	if(plate.uType == 1)
	{
		nVechileType = 2;
	}
	else if(plate.uType == 2 || plate.uType == 3)
	{
		nVechileType = 1;
	}

	string strCarNumber = plate.chText;
	int nSize = strCarNumber.size();

	if(nSize > 0)
	{
		char ch1 = *((char*)(strCarNumber.c_str()+nSize-2));
		char ch2 = *((char*)(strCarNumber.c_str()+nSize-1));

		if( (ch1 == 0xffffffad) && (ch2 == 0xffffffa6))
		{
			nVechileType = 23;
		}
	}
	return nVechileType;
}


std::string CRoadImcData::GetDetailCarType(int typeCode, bool easyDisplay)
{
	String result = "";
	if (easyDisplay)
	{
		if (typeCode == PERSON_TYPE)
			result = "行人";
		else if (typeCode == BUS_TYPE || typeCode == TRUCK_TYPE || typeCode == MIDDLEBUS_TYPE)
			result = "车辆";
		else if (typeCode == NO_JUDGE)
			result = "未判断";
		else if (typeCode == WRONG_POS)
			result = "位置太偏";
		else if (typeCode == MINI_TRUCK || typeCode == TAXI)
			result = "车辆";
		else if (typeCode == TWO_WHEEL_TYPE)
			result = "两轮车";
	}
	else
	{
		if (typeCode == PERSON_TYPE)
			result = "行人";
		else if (typeCode == BUS_TYPE)
			result = "大型客车";
		else if (typeCode == TRUCK_TYPE)
			result = "大型货车";
		else if (typeCode == MIDDLEBUS_TYPE)
			result = "中型客车";
		else if (typeCode == NO_JUDGE)
			result = "未判断";
		else if (typeCode == WRONG_POS)
			result = "位置太偏";
		else if (typeCode == MINI_TRUCK)
			result = "小型货车";
		else if (typeCode == TAXI)
			result = "小型客车";
		else if (typeCode == TWO_WHEEL_TYPE)
			result = "两轮车";
	}

	return result;
}


string CRoadImcData::GetEventTypeStr(int typeCode)
{
	string result;
	if(typeCode == DETECT_RESULT_ALL)
		result = "正常行驶";
	else if(typeCode == DETECT_RESULT_EVENT_STOP)
		result = "车辆停驶";
	else if(typeCode == DETECT_RESULT_EVENT_GO_AGAINST)
		result = "车辆逆行";
	else if(typeCode == DETECT_RESULT_EVENT_DERELICT)
		result = "遗弃物";
	else if(typeCode == DETECT_RESULT_EVENT_PASSERBY)
		result = "行人横穿";
	else if(typeCode == DETECT_RESULT_EVENT_GO_SLOW)
		result = "车辆慢行";
	else if(typeCode == DETECT_RESULT_EVENT_GO_FAST)
		result = "车辆超速";
	else if(typeCode == DETECT_RESULT_EVENT_JAM)
		result = "交通拥堵";
	else if(typeCode == DETECT_RESULT_EVENT_GO_CHANGE)
		result = "违章变道";
	else if(typeCode == DETECT_RESULT_EVENT_PERSON_STOP)
		result = "行人停留";
	else if(typeCode == DETECT_RESULT_EVENT_WRONG_CHAN)
		result = "非机动车出现";
	else if(typeCode == DETECT_RESULT_EVENT_PERSON_AGAINST)
		result = "行人逆行";
	else if(typeCode == DETECT_RESULT_EVENT_CROSS)
		result = "车辆横穿";
	else if(typeCode == DETECT_RESULT_EVENT_PERSON_APPEAR)
		result = "行人出现";
	else if(typeCode == DETECT_RESULT_EVENT_APPEAR)
		result = "机动车出现";
	else if(typeCode == DETECT_RESULT_EVENT_CARNUM)
		result = "事件车牌";
	else
	{
		int noUsed;
		result = GetViolationTypeStr(typeCode, noUsed);
	}
	return result;
}

//获取车辆违法类型解析
std::string CRoadImcData::GetViolationTypeStr(int nViolationType,int& nViolationType2)
{
    nViolationType2 = 0;
    String strViolationType = "";
    switch(nViolationType)
    {
        case DETECT_RESULT_RED_LIGHT_VIOLATION: //16 闯红灯
            {
                strViolationType = "闯红灯";
                nViolationType2 = 1625;
                break;
            }
        case DETECT_RESULT_PARKING_VIOLATION: //17 违章停车
            {
                strViolationType = "违章停车";
                break;
            }
        case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD: //18 小车出现在禁行车道
            {
                strViolationType = "小车出现在禁行车道";
                break;
            }
        case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD: //19 大车出现在禁行车道
            {
                strViolationType = "大车出现在禁行车道";
                break;
            }
        case DETECT_RESULT_BIG_IN_FORBIDDEN_TIME: //20 大车出现在禁行时间
            {
                strViolationType = "大车出现在禁行时间";
                break;
            }
        case DETECT_RESULT_EVENT_TRAFIC: //21 交通事故
            {
                strViolationType = "交通事故";
                break;
            }
        case DETECT_RESULT_EVNET_GO_EMERGENCY_CHANNEL: //22 走应急车道
            {
                strViolationType = "走应急车道";
                break;
            }
        case DETECT_RESULT_FORBID_LEFT: //23 禁止左拐
            {
                strViolationType = "禁止左拐";
				nViolationType2 = 1208;
                break;
            }
        case DETECT_RESULT_FORBID_RIGHT: //24 禁止右拐
            {
                strViolationType = "禁止右拐";
				nViolationType2 = 1208;
                break;
            }
        case DETECT_RESULT_FORBID_STRAIGHT: //25 禁止前行
            {
                strViolationType = "禁止前行";
				nViolationType2 = 1208;
                break;
            }
        case DETECT_RESULT_RETROGRADE_MOTION: //26 逆行
            {
                strViolationType = "逆行";
                nViolationType2 = 1301;
                break;
            }
        case DETECT_RESULT_PRESS_LINE: //27 压线
		case DETECT_RESULT_PRESS_WHITELINE: //27 压线
            {
                strViolationType = "压线";
				nViolationType2 = 1345;
                break;
            }
        case DETECT_RESULT_NO_PASSING: //28 禁行所有车辆
            {
                strViolationType = "禁行所有车辆";
				nViolationType2 = 1018;
                break;
            }
        case DETECT_RESULT_ELE_EVT_BIANDAO: //29 变道
            {
                strViolationType = "变道";
                break;
            }
        case DETECT_RESULT_EVENT_INSIDE: //30 闯入
            {
                strViolationType = "闯入";
                break;
            }
        case DETECT_RESULT_EVENT_OUTSIDE: //31 越界
            {
                strViolationType = "越界";
                break;
            }
        case DETECT_RESULT_OBV_TAKE_UP_BUSWAY: //32占用公交道
            {
                strViolationType = "占用公交道";
                break;
            }
        case DETECT_RESULT_BLACK_PLATE:  //33 黑名单
            {
                strViolationType = "黑名单";
                break;
            }
        case DETECT_RESULT_WHITE_PLATE: //34 白名单
            {
                strViolationType = "白名单";
                break;
            }
        case DETECT_RESULT_CROWD: //35 人群聚集
            {
                strViolationType = "人群聚集";
                break;
            }
        case DETECT_RESULT_PERSON_RUN: //36 行人奔跑
            {
                strViolationType = "行人奔跑";
                break;
            }
        case DETECT_RESULT_CYC: //37 柴油车出现
            {
                strViolationType = "柴油车出现";
                break;
            }
        case DETECT_RESULT_NOCARNUM:  //38 无牌车出现
            {
                strViolationType = "无牌车出现";
                break;
            }
		case DETECT_RESULT_NO_PARKING:  //65 黄网格停车
            {
                strViolationType = "黄网格停车";
				nViolationType2 = 1025;
                break;
            }
		case DETECT_RESULT_TAKE_UP_NONMOTORWAY: //55 机动车占用非机动车道
			{
				strViolationType = "机动车占用非机动车道";
				nViolationType2 = 1018;
				break;
			}
        default:
            {
                strViolationType = "其他";
                break;
            }
    }

  //  if(strViolationType.size() > 0)
  //  g_skpDB.UTF8ToGBK(strViolationType);

    return strViolationType;
}

//获取浮点类型时间（从1900年1月1日0时0分0秒到现在的秒数，1天=1,1秒 = 1/3600*24）
std::string  CRoadImcData::GetFloatStringTime(UINT32 uTime)
{
    std::string strTime = GetTime(uTime);

    //strTime = "2010-11-10 08:02:01";

    int year = atoi(strTime.substr(0,4).c_str());
    int month = atoi(strTime.substr(5,2).c_str());
    int day = atoi(strTime.substr(8,2).c_str());

    int hour = atoi(strTime.substr(11,2).c_str());
    int mintinue = atoi(strTime.substr(14,2).c_str());
    int second = atoi(strTime.substr(17,2).c_str());

    int nDtYear = year - 1900;//经历的年数
    int nRN = nDtYear/4;//计算闰年数量

    int nExDay = 0;
    if( (year%4) == 0)//判断当年是否闰年
    {
        nExDay = 1; //闰年多一天
    }

    int nDtDay = nDtYear*365+nRN;//一年中经历的天数
    switch (month)
    {
        case 1:
        nDtDay += day;
        break;
        case 2:
        nDtDay += (31+day);
        break;
        case 3:
        nDtDay += (31+28+nExDay+day);
        break;
        case 4:
        nDtDay += (31+28+nExDay+31+day);
        break;
        case 5:
        nDtDay += (31+28+nExDay+31+30+day);
        break;
        case 6:
        nDtDay += (31+28+nExDay+31+30+31+day);
        break;
        case 7:
        nDtDay += (31+28+nExDay+31+30+31+30+day);
        break;
        case 8:
        nDtDay += (31+28+nExDay+31+30+31+30+31+day);
        break;
        case 9:
        nDtDay += (31+28+nExDay+31+30+31+30+31+31+day);
        break;
        case 10:
        nDtDay += (31+28+nExDay+31+30+31+30+31+31+30+day);
        break;
        case 11:
        nDtDay += (31+28+nExDay+31+30+31+30+31+31+30+31+day);
        break;
        case 12:
        nDtDay += (31+28+nExDay+31+30+31+30+31+31+30+31+30+day);
        break;
    }

    double nDtSecond = hour*3600+mintinue*60+second;//一天中经历的秒数

    double fTime = nDtDay+(nDtSecond*1.0)/(24.0*3600)+1;//将秒数转换为小数

    char buf[256] = {0};
    sprintf(buf,"%.12f",fTime);

    std::string strFloatTime(buf);

    return strFloatTime;
}

//判断目录是否为空
bool CRoadImcData::CheckDir(const char * dirName)
{
	struct dirent* ptr = NULL;
	DIR* dir = opendir(dirName);
	bool isEmpty = true;

	if(dir)
	{
		while((ptr=readdir(dir)) != NULL)
		{
			if((strcmp(".", ptr->d_name) != 0)&&
				(strcmp("..", ptr->d_name) != 0))
			{
				isEmpty = false;
				break;
			}
		}
	}
	closedir(dir);
	return isEmpty;
}

//获取路口名称
std::string CRoadImcData::GetPlace()
{
	if(m_strLocation.size() <= 0)
	{
		std::string strLocation = g_skpDB.GetPlace(1);
		int nDirection = g_skpDB.GetDirection(1);
		std::string strDirection = GetDirection(nDirection);


		m_strLocation = strLocation + strDirection;
	}

	return m_strLocation;
}

//取得天津中心端的FTP远程路径 type:0 卡口ftp路径 type:1违章图片名字前缀 type:2 录像ftp路径
string CRoadImcData::GetFtpRemoteDir(RECORD_PLATE *pPlate)
{
	string strTime = GetTime(pPlate->uTime,2);//时间
	string strData = GetTime(pPlate->uTime,4);//日期
	string strHour = GetTime(pPlate->uTime,5);//小时

	char buf[256] = {0};
	string strFtpDir = m_strLocation + "/" + strData + strHour;

	int nDirection = 1;
	String strDirection = GetDirectionStr(pPlate->uDirection,nDirection);

	//录像文件名
	sprintf(buf,"%s/%s-%d-%s.avi",strFtpDir.c_str(),g_strDetectorID.c_str(),nDirection,strTime.c_str());

	string remoteDir(buf);
	g_skpDB.UTF8ToGBK(remoteDir);

	return remoteDir;
}


//获取图片路径
void CRoadImcData::GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath,int nType,int nRandCode,int nIndex)
{
	//LogNormal("GetPlatePicPath plate:%s type:%d ", plate.chText, nType);
	string strDataPath;

	if(nType == 0 || nType == 4)
	{
		if(g_nServerType == 29)
		{
			strDataPath = g_strPic;
			strDataPath += '/';
		}
		else
		{
			strDataPath = "/home/road/kakou/";
			if (IsDataDisk())
			{
				strDataPath = "/detectdata/kakou/";
			}
		}
		if(access(strDataPath.c_str(),0) != 0) //目录不存在
		{
			mkdir(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			chmod(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
	}
	else if(nType == 1 || nType == 3 || nType == 5 || nType == 6)
	{
		strDataPath = "/home/road/red/";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/red/";
		}
		if(access(strDataPath.c_str(),0) != 0) //目录不存在
		{
			mkdir(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			chmod(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		
	}
	else if(nType == 2 || nType == 7)
	{
		strDataPath = "/home/road/video/";
		if(g_nServerType == 29)
		{
			strDataPath = "/home/road/server/video/";
		}
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/video/";
		}
		if(access(strDataPath.c_str(),0) != 0) //目录不存在
		{
			mkdir(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			chmod(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
	}

	int nDirection = 1;
	string strDirection = GetDirectionStr(plate.uDirection,nDirection);

	if((g_nDetectMode == 2) && (g_nServerType == 7))
	{
		strDataPath += '/';
		strDataPath += strDirection;
		strDataPath += '/';
		if(access(strDataPath.c_str(),0) != 0) //目录不存在
		{
			mkdir(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
			chmod(strDataPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
	}

	UINT32 uTimestamp = plate.uTime;
	//时间
	std::string strTime = GetTime(uTimestamp,2);
	std::string strDate = GetTime(uTimestamp,4);
	string strHour = GetTime(uTimestamp,5);

	string strFtpDataDir = strDataPath + strDate;
	string strFtpHourDir = strFtpDataDir + strHour;

	if(access(strFtpHourDir.c_str(),0) != 0) //目录不存在
	{
		mkdir(strFtpHourDir.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
		chmod(strFtpHourDir.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
	}

	

	char buf[256] = {0};

	if(nType == 0)
	{
		string strCarNumber;
		if(plate.chText[0]=='*')
		{
			strCarNumber = "00000000";
		}
		else
		{
			strCarNumber = plate.chText;
		}

		int nCarNumberColor = 2;
		GetCarNumberColorStr(plate.uColor, nCarNumberColor);

		int nViolationType = 0;
		GetViolationTypeStr(plate.uViolationType,nViolationType);

		sprintf(buf, "%s/%s-%d-%d-%s-%s-%d-%02d-%03d-%03d.jpg", \
			strFtpHourDir.c_str(),g_ftpRemotePath,nDirection,plate.uRoadWayID,strTime.c_str(), strCarNumber.c_str(), nCarNumberColor, 0, \
			plate.uSpeed, plate.uLimitSpeed);
	}
	else if(nType == 1)
	{
		string strCarNumber;
		if(plate.chText[0]=='*')
		{
			strCarNumber = "00000000";
		}
		else
		{
			strCarNumber = plate.chText;
		}

		int nCarNumberColor = 2;
		GetCarNumberColorStr(plate.uColor, nCarNumberColor);

		int nViolationType = 0;
		GetViolationTypeStr(plate.uViolationType,nViolationType);

		sprintf(buf, "%s/%s-%d-%d-%s-%s-%d-%02d-%03d-%03d-%04d-%08x-%d.jpg", \
			strFtpHourDir.c_str(),g_ftpRemotePath,nDirection,plate.uRoadWayID,strTime.c_str(), strCarNumber.c_str(), nCarNumberColor, 0, \
			plate.uSpeed, plate.uLimitSpeed,nViolationType,nRandCode,nIndex+1);
	}
	else if(nType == 2)//全天录像
	{
		if(g_nServerType == 29)
		{
		sprintf(buf, "%s/%s-%02d.avi", \
			strFtpHourDir.c_str(),strTime.c_str(),nIndex);
		}
		else
		{
		sprintf(buf,"%s/%s-%d-%s.avi",strFtpHourDir.c_str(),g_ftpRemotePath,nDirection,strTime.c_str());
		}
	}
	else if(nType == 3)//违章录像
	{
		string strCarNumber;
		if(plate.chText[0]=='*')
		{
			strCarNumber = "00000000";
		}
		else
		{
			strCarNumber = plate.chText;
		}

		int nCarNumberColor = 2;
		GetCarNumberColorStr(plate.uColor, nCarNumberColor);

		int nViolationType = 0;
		GetViolationTypeStr(plate.uViolationType,nViolationType);


			sprintf(buf, "%s/%s-%d-%d-%s-%s-%d-%02d-%03d-%03d-%04d.avi", \
			strFtpHourDir.c_str(),g_ftpRemotePath,nDirection,plate.uRoadWayID,strTime.c_str(), strCarNumber.c_str(), nCarNumberColor, 0, \
			plate.uSpeed, plate.uLimitSpeed,nViolationType);
		
	}
	else if(nType == 4)//卡口两张图命名
	{
		string strCarNumber;
		if(plate.chText[0]=='*')
		{
			strCarNumber = "00000000";
		}
		else
		{
			strCarNumber = plate.chText;
		}

		int nCarNumberColor = 2;
		GetCarNumberColorStr(plate.uColor, nCarNumberColor);

		int nViolationType = 0;
		GetViolationTypeStr(plate.uViolationType,nViolationType);

		if(plate.uViolationType > 0 && plate.uViolationType != DETECT_RESULT_NOCARNUM)
		{
			sprintf(buf, "%s/%s%03d-%02d-%04d.jpg", \
					strFtpHourDir.c_str(),strTime.c_str(),plate.uMiTime, plate.uRoadWayID,nViolationType);
		}
		else
		{
			sprintf(buf, "%s/%s%03d-%02d-%03d.jpg", \
					strFtpHourDir.c_str(),strTime.c_str(),plate.uMiTime, plate.uRoadWayID,nIndex);
		}
		
	}
	else if(nType == 5)
	{
		string strCarNumber;
		if(plate.chText[0]=='*')
		{
			strCarNumber = "00000000";
		}
		else
		{
			strCarNumber = plate.chText;
		}

		sprintf(buf, "%s/%s-%d-%d-%s-%s-%d-%02d-%03d-%03d-%05d-%08x-%d.jpg", \
			strFtpHourDir.c_str(),g_ftpRemotePath,0,0,strTime.c_str(), strCarNumber.c_str(), 2, 0, \
			plate.uSpeed, plate.uLimitSpeed,13441,nRandCode,nIndex+1);
	}
	else if(nType == 6)//违章录像
	{
		string strCarNumber;
		if(plate.chText[0]=='*')
		{
			strCarNumber = "00000000";
		}
		else
		{
			strCarNumber = plate.chText;
		}

		sprintf(buf, "%s/%s-%d-%d-%s-%s-%d-%02d-%03d-%03d-%05d.avi", \
			strFtpHourDir.c_str(),g_ftpRemotePath,0,0,strTime.c_str(), strCarNumber.c_str(), 2, 0, \
			plate.uSpeed, plate.uLimitSpeed,13441);
	}
	else if(nType == 7)//违章录像
	{
		int nViolationType = 0;
		GetViolationTypeStr(plate.uViolationType,nViolationType);

			sprintf(buf, "%s/%s-%d-%04d.avi", \
			strFtpHourDir.c_str(),strTime.c_str(),plate.uRoadWayID,nViolationType);
	}
	
	strPicPath = buf;
	//g_skpDB.UTF8ToGBK(strPicPath);
}

//获取随机防伪码
int CRoadImcData::GetRandCode()
{
	//srand( (unsigned)time( NULL ) );
	getpid();
	int nRandCode = 1 + (int)(0x7fffffff*1.0*rand()/(RAND_MAX+1.0));
	return nRandCode;
}

//从jpg文件中获取防伪码
void CRoadImcData::GetRandCodeFromPic(string strPicPath,int& nRandCode1,int& nRandCode2)
{
	nRandCode1 = 0;
	nRandCode2 = 0;

	FILE* fp =NULL;
	fp = fopen(strPicPath.c_str(),"rb");
	if(fp != NULL)
	{
		struct stat s;
		stat(strPicPath.c_str(), &s);
		int nBytesTotal = s.st_size;

		if(nBytesTotal > 0)
        {
				fseek(fp,-32,SEEK_END);

                fread(&nRandCode1,sizeof(unsigned char),4,fp);
				fread(&nRandCode2,sizeof(unsigned char),4,fp);
        }

		fclose(fp);
	}
}

//生成天津流量统计配置文件,相同断面,文件合并
//生成流量统计配置文件(nRegionType:区域类型;nStatisticType:流量数据类型0:流量统计数据，1：流量评价数据)
string  CRoadImcData::AddStatisticDataTj(const String &strMsg,int nRegionType,int nStatisticType)
{
	//LogNormal("AddStatisticDataTj nRegionType:%d nStatisticType:%d \n", \
	//	nRegionType, nStatisticType);
	char buf[256] = {0};

	RECORD_STATISTIC* pStatistic = (RECORD_STATISTIC *)(strMsg.c_str() + sizeof(SRIP_DETECT_HEADER));

	int nDirection  = g_skpDB.GetDirection(1);
	int nDirection2 = 1;
	GetDirectionStr(nDirection,nDirection2);

	string strTime = GetTime(pStatistic->uTime,2);//时间
	string strData = GetTime(pStatistic->uTime,4);//日期
	string strHour1 = GetTime(pStatistic->uTime,5);//小时

	string strFtpDir = "../flow/";
	if (IsDataDisk())
	{
		strFtpDir = "/detectdata/flow/";
	}

	if(access(strFtpDir.c_str(),0) != 0) //目录不存在
	{
		mkdir(strFtpDir.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		chmod(strFtpDir.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	}

	string strFtpTimeDir = strFtpDir + strData;
	strFtpTimeDir += strHour1;

	//if(g_nFtpServer == 1)
	{
		if(access(strFtpTimeDir.c_str(),0) != 0) //目录不存在
		{
			mkdir(strFtpTimeDir.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
		}
	}


	String strNewMsgHead;
	strNewMsgHead = AddNewItemValue(strNewMsgHead, false, "RoadNO", g_ftpRemotePath);//路口编号

	sprintf(buf, "%d",nDirection2);
	string strDirection(buf);
	strNewMsgHead = AddNewItemValue(strNewMsgHead, false, "DirectionNo", strDirection);//方向编码

	int nCount = 0;
	for(int i=0; i<MAX_ROADWAY; i++)
	{
		if(pStatistic->uFlux[i]!=0xffffffff)
		{
			nCount++;
		}
	}

	sprintf(buf, "%d",nCount);
	//LogNormal("uFlux nCount = %d ", nCount);

	string strCount(buf);
	strNewMsgHead = AddNewItemValue(strNewMsgHead, false, "ActRowNum", strCount);//实际车道数

	String strNewMsg;
	//流量类型
	int nStatType = 16;

	if(nStatisticType == 0)
	{
		if(pStatistic->uStatTimeLen == 30)
		{
			nStatType = 13;
		}
		else if(pStatistic->uStatTimeLen == 60)
		{
			nStatType = 16;
		}
		else if(pStatistic->uStatTimeLen == 300)
		{
			nStatType = 15;
		}
	}
	else
	{
		nStatType = 02;

		string strDetectTime = GetTime(pStatistic->uTime, 0);//采集时间
		strDetectTime = strDetectTime.substr(11, 8);
		strNewMsg = AddNewItemValue(strNewMsg, false, "ConnectTime", strDetectTime);//采集时间

		sprintf(buf, "%d",pStatistic->uStatTimeLen);
		string strTimeLegth(buf);
		strNewMsg = AddNewItemValue(strNewMsg, false, "ConnectDuration", strTimeLegth);//统计时长

		sprintf(buf, "%d", pStatistic->uSeq);
		string strNumber(buf);
		strNewMsg = AddNewItemValue(strNewMsg, false, "Connectnumber", strNumber);//采集序数

		if(pStatistic->uSeq < 4)
		{
			strNewMsg = AddNewItemValue(strNewMsg, false, "Connecttriger", "1");//采集触发端
		}
		else
		{
			strNewMsg = AddNewItemValue(strNewMsg, false, "Connecttriger", "2");//采集触发端
		}

	}

	if( (nStatisticType == 1)&& (nRegionType == PERSON_ROAD))
	{
		//车道是否占有
		for(int i=0; i<MAX_ROADWAY; i++)
		{
			if(pStatistic->uFlux[i]!=0xffffffff)
			{
				unsigned int uRoadIndex = pStatistic->uRoadType[i];
				uRoadIndex   =    uRoadIndex>>16;

				sprintf(buf, "Road%dPoss",uRoadIndex);
				string strRoadIndex(buf);

				sprintf(buf, "%d",pStatistic->uSpace[i]);
				string strPoss(buf);

				strNewMsg = AddNewItemValue(strNewMsg, false, strRoadIndex.c_str(), strPoss);//
			}
		}

		//车道占有率
		for(int i=0; i<MAX_ROADWAY; i++)
		{
			if(pStatistic->uFlux[i]!=0xffffffff)
			{
				unsigned int uRoadIndex = pStatistic->uRoadType[i];
				uRoadIndex   =    uRoadIndex>>16;

				sprintf(buf, "Road%dShare",uRoadIndex);
				string strRoadIndex(buf);

				sprintf(buf, "%d",pStatistic->uOccupancy[i]);
				string strOccupancy(buf);

				strNewMsg = AddNewItemValue(strNewMsg, false, strRoadIndex.c_str(), strOccupancy);//
			}
		}
	}
	else
	{
		for(int i=0; i<MAX_ROADWAY; i++)
		{
			if(pStatistic->uFlux[i]!=0xffffffff)
			{
				unsigned int uRoadIndex = pStatistic->uRoadType[i];
				uRoadIndex   =    uRoadIndex>>16;

				sprintf(buf, "%d",uRoadIndex);
				string strRoadIndex(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "CarRoad", strRoadIndex);//车道号

				unsigned int uObjectNum = (pStatistic->uFlux[i])&(0x0000ffff);
				sprintf(buf, "%d",uObjectNum);
				string strFlow(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "Flow", strFlow);//流量

				unsigned int uBigNum = (pStatistic->uFlux[i])>>16;
				sprintf(buf, "%d",uBigNum);
				string strBigFlow(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "LongCarFlow", strBigFlow);//大车流量

				unsigned int uSmallNum = (pStatistic->uFluxCom[i])&(0x0000ffff);
				sprintf(buf, "%d",uSmallNum);
				string strSmallFlow(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "SmallCarFlow", strSmallFlow);//小车流量

				sprintf(buf, "%d", pStatistic->uSpace[i]);//暂时为空
				string strTotalInterval(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "TotalInterval", strTotalInterval);//累计间隔时间

				sprintf(buf, "%d", pStatistic->uOccupancy[i]);//暂时为空
				string strTotalPossessTime(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "TotalPossessTime", strTotalPossessTime);//累计占有时间

				sprintf(buf, "%d",pStatistic->uBigCarSpeed[i]);
				string strBigCarSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "LongCarSpeed", strBigCarSpeed);//大车平均车速

				sprintf(buf, "%d",pStatistic->uSmallCarSpeed[i]);
				string strSmallCarSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "SmallCarSpeed", strSmallCarSpeed);//小车平均车速

				sprintf(buf, "%d",pStatistic->uSpeed[i]);
				string strAverageSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "AverageSpeed", strAverageSpeed);//平均车速

				sprintf(buf, "%d",pStatistic->uMaxSpeed[i]);
				string strMaxSpeed(buf);
				strNewMsg = AddNewItemValue(strNewMsg, false, "BiggestSpeed", strMaxSpeed);//最大车速
			}
		}
	}

	//
	string strHour = strTime.substr(8,2);
	string strMinute = strTime.substr(10,2);
	string strSecond = strTime.substr(12,2);

	int nHour = atoi(strHour.c_str());
	int nMinute = atoi(strMinute.c_str());
	int nSecond = atoi(strSecond.c_str());

	int nTime = nHour*3600+nMinute*60+nSecond;

	int nSeq = nTime/pStatistic->uStatTimeLen;

	if(nStatisticType == 0)
	{
		sprintf(buf, "%s/%s-%d-%02d-%d-%s-%d.ini", \
			strFtpTimeDir.c_str(),g_ftpRemotePath,nDirection2,nStatType,nRegionType,strData.c_str(),nSeq);
	}
	else
	{
		string strDateTime = GetTime(pStatistic->uTime,6);
		sprintf(buf, "%s/%s-%d-%02d-%d-%s-%s.ini", \
			strFtpTimeDir.c_str(),g_ftpRemotePath,nDirection2,nStatType,nRegionType,strData.c_str(),strDateTime.c_str());
	}


	string strPath(buf);

	printf("===strPath.c_str()=%s\n",strPath.c_str());

	
	//若已存在strPath,则追加
	pthread_mutex_lock(&m_StatisticMutex);

	//strPath,核查是否已经存在
	if(access(strPath.c_str(), F_OK) == 0)//已存在
	{
		std::string strLeftValue = "ActRowNum";
		///strPath = "/home/road/aa.ini";//test
		
		bool bUpdate = UpdateRoadCounts(strPath, strLeftValue, nCount);//更新车道数目

		if(bUpdate)
		{			
			//追加
			FILE* fp = NULL;
			fp = fopen(strPath.c_str(),"a+");
			if(fp!=NULL)
			{
				strNewMsg = "\n" + strNewMsg;//换行
				fwrite(strNewMsg.c_str(),1,strNewMsg.size(),fp);
				fclose(fp);
			}			
		}
		else
		{
			LogNormal("更新流量数据:%s失败!", strPath.c_str());
		}
		
	}
	else //不存在
	{
		FILE* fp = NULL;
		fp = fopen(strPath.c_str(),"wb");
		if(fp!=NULL)
		{
			strNewMsg = strNewMsgHead + strNewMsg;// FIX
			fwrite(strNewMsg.c_str(),1,strNewMsg.size(),fp);
			fclose(fp);
		}
	}

	chmod(strPath.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);

	pthread_mutex_unlock(&m_StatisticMutex);

	//g_skpDB.UTF8ToGBK(strPath);
	return strPath;
}

//更新道路数量
bool CRoadImcData::UpdateRoadCounts(
	const std::string &strPath, 
	const std::string &strLeftValue, 
	int nRoadCounts)
{
	//LogNormal("UpdateRoadCounts \n");
	bool bRet = false;

	int nRightValue = 0;
	std::string strMidValue = "=";
	std::string strNewRightValue = "";

	//读取对应行格式,字符串
	bool bGetValue = GetValueFromFile(strPath, strLeftValue, strMidValue, nRightValue);
	if(bGetValue)
	{
		//更新int值
		int nNewRoadCounts = nRightValue + nRoadCounts;
		//重组int值,字符串
		char buf[1024] = {0};
		sprintf(buf, "%s%s%d", strLeftValue.c_str(), strMidValue.c_str(), nNewRoadCounts);
		strNewRightValue = std::string(buf);

		//更新文件为新的字符串
		bool bUpdate = UpdateStrLine(strPath, strLeftValue, strNewRightValue);
		if(bUpdate)
		{
			bRet = true;
		}
	}

	return bRet;
}

//从文件取得约定格式字符串
//[strLeftValue strMidValue strRightValue]
bool CRoadImcData::GetValueFromFile(
	const std::string &strPath, 
	const std::string &strLeftValue, 
	const std::string &strMidValue, 
	int &nRightValue)
{
	//LogNormal("GetValueFromFile");
	bool bRet = false;

	if(strPath.size() > 0)
	{
		FILE* fp = fopen(strPath.c_str(), "r");
		if(fp != NULL)
		{
			char szBuff[1024] = {0};
			fread(szBuff,sizeof(szBuff),1,fp);
			char *pBuff = strstr(szBuff, strLeftValue.c_str());
			if (pBuff != NULL)
			{
				std::stringstream stream;
				std::string sub_str;
				stream.clear();
				stream.str(pBuff);
				getline(stream, sub_str, '\n');
				int nPos = sub_str.find(strMidValue.c_str());
				if (nPos > 0)
				{
					if (sub_str.size() >= (nPos + 1))
					{
						nRightValue = atoi(sub_str.substr(nPos + 1, 2).c_str());
						bRet = true;
					}
				}
			}
			fclose(fp);
		}
		else
		{
			LogNormal("GetValueFromFile path err! %s", strPath.c_str());
		}
	}	

	return bRet;
}

//更新,一行约定,开始格式字符串
bool CRoadImcData::UpdateStrLine(
	const std::string &strPath, 
	const std::string &strLeftValue, 
	const std::string &strNewValue)
{
	//LogNormal("UpdateStrLine");
	bool bRet = false;
	
	if(strPath.size() > 0)
	{
		std::string strMsg = "";
		FILE* fp = fopen(strPath.c_str(), "r");
		if(fp != NULL)
		{
			fseek(fp, SEEK_SET, SEEK_END);
			int nFileSize = ftell(fp);

			fseek(fp, 0, SEEK_SET);
			char *pBuff = new char[nFileSize];
			memset(pBuff, 0, nFileSize);
			if(pBuff)
			{
				fread(pBuff, nFileSize, 1, fp);				
				strMsg.append(pBuff, nFileSize);

				int nPos_B = strMsg.find(strLeftValue.c_str(),0);
				int nPos_E = strMsg.find("\n", nPos_B);
				
				//LogNormal("nPos_B:%d, nPos_E:%d, strNewValue: %s ", nPos_B, nPos_E, strNewValue.c_str());

				strMsg.erase(nPos_B, nPos_E - nPos_B);
				strMsg.insert(nPos_B, strNewValue.c_str(), 0, strNewValue.size());
				bRet = true;
			}
			
			if(pBuff)
			{
				delete pBuff;
				pBuff = NULL;
			}
			fclose(fp);
		}
		else
		{
			LogNormal("UpdateStrLine path err! %s", strPath.c_str());
		}

		if(bRet)
		{
			//重新生成新的文件
			FILE* fp = fopen(strPath.c_str(), "w");
			if(fp)
			{
				fwrite(strMsg.c_str(),1,strMsg.size(),fp);
				fclose(fp);
			}
		}
	}	

	return bRet;
}

