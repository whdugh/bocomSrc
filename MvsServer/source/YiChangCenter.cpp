#include "YiChangCenter.h"
#include "CommonHeader.h"
#include <string>
#include <iostream>
#include "FtpCommunication.h"
#include "mvdspapi.h"
#include "VehicleConfig.h"
#include "XmlParaUtil.h"
#include "CSeekpaiDB.h"
#include "BrandSubSection.h"
#include "CarLabel.h"
using namespace std;

CYiChangCenter g_YiChangCenter;

int GetRoadIndexType(int nChannel, int RoadId)
{

	MvDspGlobalSetting dspSettingStrc;
	CALIBRATION calibration;

	CXmlParaUtil xml;
	//if(xml.LoadDspSettingInfo(dspSettingStrc,calibration,nChannel))
	if(xml.LoadDspSettingFile(dspSettingStrc,nChannel))
	{
		for(int i = 0;i<dspSettingStrc.nChannels;i++)
		{
			if (dspSettingStrc.ChnlRegion[i].nVerRoadIndex == RoadId)
			{
				if (dspSettingStrc.ChnlRegion[i].nChannelDriveDir <=  4)
				{
					return dspSettingStrc.ChnlRegion[i].nChannelDriveDir + 1;
				}
				else
				{
					return 99;
				}
			}
		}
	}

	return 99;
}

// 车牌的颜色
int CardClolr(int nColor)
{
	switch (nColor)
	{
	case 1://蓝色
		nColor = 2;
		break;
	case 2://黑色
		nColor = 3;
		break;
	case 3://黄色
		nColor = 1;
		break;
	case 4://白色
		nColor = 0;
		break;
	case 5://其他
		nColor = 4;
		break;
	}
	return nColor;
}

// 车身颜色
string CarColor(int nColor)
{
	string strColor = "Z";
	switch (nColor)
	{
	case 0://白色
		strColor = "A";
		break;
	case 1://银色
		strColor = "Z";
		break;
	case 2://黑色
		strColor = "J";
		break;
	case 3://红色
		strColor = "E";
		break;
	case 4://紫色
		strColor = "F";
		break;
	case 5://蓝色
		strColor = "H";
		break;
	case 6://黄色
		strColor = "C";
		break;
	case 7://绿色
		strColor = "G";
		break;
	case 8://褐色
		strColor = "Z";
		break;
	case 9://粉红色
		strColor = "D";
		break;
	case 10://灰色
		strColor = "Z";
		break;
	case 11://未知
		strColor = "Z";
		break;
	}

	return strColor;
}

//获取号牌种类
int GetPlateType(int nColor)
{
	int nPlateType = 99;
    switch (nColor)
	{
	case 1://蓝色
		nPlateType = 2;
		break;
	case 2://黑色
		nPlateType = 5;
		break;
	case 3://黄色
		nPlateType = 1;
		break;
	case 4://白色
		nPlateType = 22;
		break;
	}
	return nPlateType;
}

//获取车辆类型
string GetCarType(int nType)
{
	string strCarType = "X99";
    switch (nType)
	{
	case BUS_TYPE://大巴
		strCarType = "K11";
		break;
	case TRUCK_TYPE://大货
		strCarType = "H11";
		break;
	case MIDDLEBUS_TYPE://中巴 
		strCarType = "K21";
		break;
	case TAXI://小型客车
		strCarType = "K31";
		break;
	}
	return strCarType;
}

//获取车辆分类
int GetCarClass(string strCarNumber)
{
	int nSize = strCarNumber.size();
	
	int nCarClass = 9;
	if(nSize > 0)
    {
		if(strCarNumber == "00000000")
		{
			nCarClass = 2;//无牌车
		}
		else
		{
			nCarClass = 3;
		}
	}
	return nCarClass;
}

//获取违章类型
int GetCarViolationType(int nType)
{
	int nViolationType = 1230;
    switch (nType)
	{
	case DETECT_RESULT_RED_LIGHT_VIOLATION://闯红灯
		nViolationType = 1302;
		break;
	case DETECT_RESULT_RETROGRADE_MOTION://逆行
		nViolationType = 1301;
		break;
	case DETECT_RESULT_EVENT_GO_FAST://超速
		nViolationType = 1303;
		break;
	case DETECT_RESULT_FORBID_LEFT://违反禁止标线指示
	case DETECT_RESULT_FORBID_RIGHT://违反禁止标线指示
	case DETECT_RESULT_FORBID_STRAIGHT://违反禁止标线指示
	case DETECT_RESULT_PRESS_LINE://违反禁止标线指示
	case DETECT_RESULT_ELE_EVT_BIANDAO://违反禁止标线指示
	case DETECT_RESULT_PRESS_WHITELINE://违反禁止标线指示
		nViolationType = 1230;
		break;
	}
	return nViolationType;
}

string CarLable(UINT32 nLable)
{
	CAR_LABEL lable = (CAR_LABEL)nLable;
	std::map<CAR_LABEL, string> mapLable;

	mapLable[AUDI] = "奥迪";
	mapLable[BMW] = "宝马";
	mapLable[HYUNDAI] = "现代";
	mapLable[BENZ] = "奔驰";
	mapLable[HONDA] = "本田";
	mapLable[VW] = "大众";
	mapLable[MAZDA] = "马自达";
	mapLable[TOYOTA] = "丰田";
	mapLable[BUICK] = "别克";
	mapLable[CHEVROLET] = "雪弗兰";
	mapLable[CITERON] = "雪铁龙";
	mapLable[PEUGEOT] = "标志";
	mapLable[FORD] = "福特";
	mapLable[LEXUS] = "凌志";
	mapLable[NISSAN] = "尼桑";
	mapLable[CHERY] = "奇瑞";
	mapLable[BYD] = "比亚迪";
	mapLable[KIA] = "起亚";
	mapLable[ROWE] = "荣威";
	mapLable[MITSUBISHI] = "三菱";
	mapLable[SKODA] = "斯柯达";
	mapLable[CHANGHE] = "铃木";
	mapLable[FIAT] = "菲亚特";
	mapLable[VOLVO] = "沃尔沃";
	mapLable[JEEP] = "吉普";
	mapLable[LANDROVER] = "路虎";
	mapLable[GMC] = "通用";
	mapLable[RedFlags] = "红旗";
	mapLable[HUMMER] = "悍马";
	mapLable[JINBEI] = "金杯";
	mapLable[JAC] = "江淮";
	mapLable[JMC] = "江铃";
	mapLable[GEELY] = "吉利";
	mapLable[LANDWIND] = "陆风";
	mapLable[LIFAN] = "力帆";
	mapLable[MG] = "名爵";
	mapLable[ACURA] = "讴歌";
	mapLable[INFINITI] = "英菲尼迪";
	mapLable[FLYWITHOUT] = "中华";
	mapLable[ZOTYEAUTO] = "众泰";
	mapLable[RELY] = "威麟";
	mapLable[SUBARU] = "斯巴鲁";
	mapLable[SPYKER] = "世爵";
	mapLable[SHUANGHUANAUTO] = "双环";
	mapLable[SAAB] = "萨博";
	mapLable[WIESMANN] = "威兹曼";
	mapLable[GLEAGLE] = "全球鹰";
	mapLable[GONOW] = "吉奥";
	mapLable[HAWTAIAUTOMOBILE] = "华泰";
	mapLable[SMA] = "华普";
	mapLable[HAFEI] = "哈飞";
	mapLable[SOUEAST] = "东南";
	mapLable[EMGRANO] = "帝豪";
	mapLable[CHANA] = "长安";
	mapLable[CHANGFENAMOTOR] = "长丰";
	mapLable[GREATWALL] = "长城";
	mapLable[DAEWOO] = "大宇";
	mapLable[ISUZU] = "五十铃";
	mapLable[DAIHATSU] = "大发";
	mapLable[JAGUAR] = "捷豹";
	mapLable[OPEL] = "欧宝";
	mapLable[CHRYSLER] = "克莱斯勒";
	mapLable[ALFAROMEO] = "阿尔法-罗密欧";
	mapLable[LINCOLN] = "林肯";
	mapLable[ROLLSROYCE] = "劳斯莱斯";
	mapLable[FERRARI] = "法拉利";
	mapLable[PORSCHE] = "保时捷";
	mapLable[LOTUS] = "莲花";
	mapLable[ASTONMARTIN] = "阿斯顿马丁";
	mapLable[CROWN] = "皇冠";
	mapLable[BESTURN] = "奔腾";
	mapLable[DONGFENG] = "东风";
	mapLable[POLARSUN] = "中顺";
	mapLable[FOTON] = "长安福田";
	mapLable[WULING] = "五菱";
	mapLable[CADILLAC] = "凯迪拉克";
	mapLable[MASERATI] = "玛莎拉蒂";
	mapLable[XIALI] = "夏利";
	mapLable[RENAULT] = "雷诺";
	mapLable[IVECO] = "依维柯";
	mapLable[BNTLY] = "宾利";
	mapLable[MINI] = "迷你";
	mapLable[SSANGYONG] = "汇众";
	mapLable[YIQI] = "一汽";
	mapLable[KARRY] = "开瑞";
	mapLable[NANQI] = "南汽";
	mapLable[YUEJIN] = "跃进";
	mapLable[FAW] = "解放";
	mapLable[KINGLONG] = "金龙";
	mapLable[RANGEROVER] = "罗福";
	mapLable[ZX] = "中兴";
	mapLable[RIICH] = "瑞麒";
	mapLable[SG] = "曙光";
	mapLable[BEIQI] = "北汽";
	mapLable[OTHERS] = "其他";

	return mapLable[lable];
}

/*typedef struct _SYSTEMTIME {                  
	unsigned short wYear;                               
	unsigned short wMonth;                              
	unsigned short wDayOfWeek;                          
	unsigned short wDay;                                
	unsigned short wHour;                               
	unsigned short wMinute;                             
	unsigned short wSecond;
	unsigned short wMilliseconds;    
	_SYSTEMTIME()
	{
		wYear = 0;
		wMonth = 0;
		wDayOfWeek = 0;
		wDay = 0;
		wHour = 0;
		wMinute = 0;
		wSecond = 0;
		wMilliseconds = 0;
	}
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME; */
//连接中心端线程
void* ConnectProcess(void* pArg)
{
	//取类指针
	CYiChangCenter* pThis = (CYiChangCenter*)pArg;
	if(pThis == NULL) return pArg;

	while (g_bEndThread)
	{
		while (!pThis->GetCenterLinkFlag())
		{
			pThis->ConnectToCS();
			sleep(5);
		}

		usleep(1000);
	}

	pthread_exit((void *)0);
	return pArg;
}

//发送心跳给中心端线程
void* SendHeartBeatProcess(void* pArg)
{
	//取类指针
	CYiChangCenter* pThis = (CYiChangCenter*)pArg;
	if(pThis == NULL) return pArg;

	/*HeartBeatPacket nHeartBeatPacket;*/
	UINT16 nHeader = 0xAAAA;
	UINT16 nType	 = 1103;			//包类型
	UINT32 nLength = 4;			//值的长度
	UINT32 nValue = 0xAAAAAAAA;	//值
	UINT16 nTail = 0x5555;		//包尾

	string strHeader("");
	string strType("");
	string strLength("");
	string strValue("");
	string strTail("");

	string strData("");

	int nHeartBeatCount = 0;
	//strData.append((const char*)&nHeartBeatPacket,sizeof(HeartBeatPacket));
	strHeader.append((char*)&nHeader,sizeof(nHeader));
	strType.append((char*)&nType,sizeof(nType));
	strLength.append((char*)&nLength,sizeof(nLength));
	strValue.append((char*)&nValue,sizeof(nValue));
	strTail.append((char*)&nTail,sizeof(nTail));

	strData += strHeader;
	strData += strType;
	strData += strLength;
	strData += strValue;
	strData += strTail;

	while (!g_bEndThread)
	{
		while (pThis->GetCenterLinkFlag())
		{
			sleep(3);

			pThis->SendMsg(pThis->GetCenterSocket(),strData,0);

			nHeartBeatCount = pThis->GetHeartBeatFailureCount() + 1;

			pThis->SetHeartBeatCount(nHeartBeatCount);

			if (pThis->GetHeartBeatFailureCount() == 10)
			{
			//	pThis->CloseCenterSocket();/////////////////////////////////////////////////
			}
		}

		usleep(1000);
	}

	pthread_exit((void *)0);
	return pArg;
}

/*
 * 历史记录发送 当每次连接断开之后， 把没有发送过去的数据发送过去
*/
void * SendHistoryDataThread(void * pArg)
{
	CYiChangCenter * pThis = (CYiChangCenter *)pArg;
	while(!g_bEndThread)////////////////////////////////////
	{
		if(g_nSendHistoryRecord == 1)
		{
			pThis->SendHistory();
		}
		sleep(1);
	}
	
}

//接收中心信息线程
void* RecvCenterMsgProcess(void* pArg)
{
	//取类指针
	CYiChangCenter* pThis = (CYiChangCenter*)pArg;
	if(pThis == NULL) return pArg;

	while (!g_bEndThread)
	{
		while (pThis->GetCenterLinkFlag())
		{
			pThis->mvRecvSocketMsg(pThis->GetCenterSocket());

			usleep(1000);
		}

		usleep(1000);
	}

	pthread_exit((void *)0);
	return pArg;
}

//发送车牌信息图片信息给中心端线程
void* SendPlateMsgProcess(void* pArg)
{
	CYiChangCenter* pThis = (CYiChangCenter*)pArg;		//取类指针
	if(pThis == NULL) return pArg;

	while (!g_bEndThread)		///////////////////////////////////////////////
	{
		
		while (!g_bEndThread)
		{
			pThis->DealResult();
			usleep(1000);
		}

		usleep(1000);
	}

	pthread_exit((void *)0);
	return pArg;
}

CYiChangCenter::CYiChangCenter(void)
	: m_nCenterSocket(0)
	, m_bCenterLink(false)
{
	pthread_mutex_init(&m_send_mutex, NULL);
}

CYiChangCenter::~CYiChangCenter(void)
{
}

/*
 * 发送历史数据 ，从数据库里里面查询出来历史的数据， 并且把这些信息发送过去
*/
int CYiChangCenter::SendHistory()
{
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
	{
		return false;
	}

	unsigned int uTimeStamp = GetTimeStamp() - 60 * 3;		//3分钟之前的数据 防止实时记录正在发送又将其作为历史数据发送的情况
	string strTime = GetTime(uTimeStamp);
	
	char szSql[1024]={0};
	sprintf(szSql, "select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME <= '%s'\
				  ORDER BY TIME desc limit 1; ", strTime.c_str());
	string  strSql(szSql);
	
	MysqlQuery sqlData = g_skpDB.execQuery(strSql);
	if ( sqlData.eof())	// 说明没有数据							
	{
		return 0;
	}
	string strImageData = GetImageByPath(sqlData.getStringFileds("PICPATH") );
	if (strImageData.size() <= 0)
	{
		return -1;
	}

	string strRemotePath = MakeSaveImagePath(sqlData);	//发图片的路径
	m_strHistoryRemotePath = strRemotePath;
	// 先发送图片
	int nRet = g_FtpCommunication.VideoDoPut(NULL, (char*)strRemotePath.c_str(),
		strImageData, true, true, (char*)strRemotePath.c_str(), true);
	if (!nRet)
	{
		LogError("宜昌 ftp 历史图片传送失败 \n");
		sqlData.finalize();
		return -1;
	}

	// 然后再发送xml 文件
	string strXML = ComposeSqlDataToXML( sqlData);

	if ( !ConnectToCS())
	{
		sqlData.finalize();
		return -1;
	}
	//pthread_mutex_lock(&m_send_mutex);
	if (SendMsg(m_nCenterSocket, strXML, 0))
	{
		char szSqlBuf [125] = {0};
		sprintf(szSqlBuf, "update NUMBER_PLATE_INFO set STATUS = 1 where ID = %u", sqlData.getIntFileds("ID") );
		string strSql(szSqlBuf);
		g_skpDB.execSQL(strSql);
		//pthread_mutex_unlock(&m_send_mutex);
		sqlData.finalize();
		return 0;
	}

	sqlData.finalize();
	//pthread_mutex_unlock(&m_send_mutex);
	return -1;
}


/** 
 * 制作 保存到ftp 的目录结构 
 * 参数是： 
*/
string CYiChangCenter::MakeSaveImagePath(MysqlQuery sqlData)
{
	string strTime	= sqlData.getStringFileds("TIME");
	string strYear	= strTime.substr(0,4);
	string strMonth	= strTime.substr(5,2);
	string strDay		= strTime.substr(8,2);
	string strHour	= strTime.substr(11,2);
	string strMin		= strTime.substr(14,2);
	string strSec		= strTime.substr(17,2);
	// 时间的目录 （第一层目录）
	string strTimePath = strYear + strMonth + strDay;
	// 设备号目录 （第二层目录）
	string strDevCode = g_strDetectorID;
	// 方向目录   （第三层目录） 
	string strDirectionPath = "0" + sqlData.getStringFileds("DIRECTION");
	// 图片的名字
	static int num = 0;
	if (num++ == 10)
	{
		num = 0;
	}
	char szNum[4] = {0};
	sprintf(szNum, "%02d", num);
	string strNum(szNum);
	string strImageName = strHour + strMin + strSec 
						+ "_"+ sqlData.getStringFileds("ROAD") + "_" + strNum + ".jpg";

	string  strDistantPath = strTimePath + "/" + strDevCode + "/"+ strDirectionPath \
							+ "/" + strImageName;

	return strDistantPath;
}

string CYiChangCenter::MakeSaveImagePath(RECORD_PLATE * pPlate)
{
	string strTime = GetTime(pPlate->uTime, 0);
	
	string strYear	= strTime.substr(0,4);
	string strMonth	= strTime.substr(5,2);
	string strDay		= strTime.substr(8,2);
	string strHour	= strTime.substr(11,2);
	string strMin		= strTime.substr(14,2);
	string strSec		= strTime.substr(17,2);
	// 时间的目录 （第一层目录）
	string strTimePath = strYear + strMonth + strDay;
	// 设备号目录 （第二层目录）
	string strDevCode = g_strDetectorID;
	// 方向目录   （第三层目录） 
	char szDirection[10] = {0};
	sprintf(szDirection, "%02d", pPlate->uDirection);
	string strDirectionPath(szDirection);
	// 图片的名字
	char   szRoadWayId[10] = {0};
	sprintf(szRoadWayId, "%02d", pPlate->uRoadWayID);
	string  strRoadWayId(szRoadWayId);
	static int num = 11;
	if (num++ == 20)
	{
		num = 11;
	}
	char szNum[4] = {0};
	sprintf(szNum, "%d", num);
	string strNum(szNum);
	string strImageName = strHour + strMin + strSec + "_" + strRoadWayId + "_" + strNum+ ".jpg";

	string  strDistantPath = strTimePath + "/" + strDevCode + "/"+ strDirectionPath \
		+ "/" + strImageName;

	return strDistantPath;
}

string CYiChangCenter::ComposeSqlDataToXML( MysqlQuery sqlQuery)
{

	cerr<<"-----"<<m_strHistoryRemotePath<<"\n";
	string	strDevCode = g_strDetectorID;
	string	strCurrentTime = GetTimeCurrent();
	string	strTime = sqlQuery.getStringFileds("TIME");
	string	strCard = CardChang( sqlQuery.getStringFileds("NUMBER") );
	string	strCarColor = CarColor( sqlQuery.getIntFileds("CARCOLORWEIGHT") ); 
	int		nDirection = sqlQuery.getIntFileds("DIRECTION");
	int		fSpeed = sqlQuery.getIntFileds("SPEED");
	int		nRoadID = sqlQuery.getIntFileds("ROAD");
	string	strCarFactory = CarLable( sqlQuery.getIntFileds("FACTORY") );
	string	strCarType = GetCarType(sqlQuery.getIntFileds("TYPE_DETAIL"));
	int		nCardColor = sqlQuery.getIntFileds("COLOR");
	
	string strPlace = g_skpDB.GetPlace(sqlQuery.getIntFileds("CHANNEL") );
		
	char szBuf[1024 * 2] = {0};

	if (sqlQuery.getIntFileds("PECCANCY_KIND") == 0)
	{
		snprintf(szBuf, 1024 * 2, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
			"<rootInfo>"
				"<headInfo>"
					"<devCode>%s</devCode>"			// 设备编号
					"<license></license>"			// 设备许可证     
					"<comName></comName>"			// 设备厂家注册名
					"<comWord></comWord>"			// 设备厂家注册密码
					"<sendTime>%s</sendTime>"			// 发送时间
					"<ver>1.1</ver>"                // 版本
				"</headInfo>"
				"<listInfo>"
					"<roadcode>%s</roadcode>"			/* 道路编号*/
					"<dcode>%02d</dcode>"			/* 方向编号*/ 
					"<lcode>%d</lcode>"				/* 车道编号*/
					"<ltype>%02d</ltype>"			/* 车道类型*/
					"<plateTime>%s:%03d</plateTime>"	/* 通行时间*/
					"<plateFlag>%d</plateFlag>"		/* 号牌标志*/
					"<bplateNum>%s</bplateNum>"			// 号牌号码
					"<bplateColor>%d</bplateColor>"		// 号牌颜色
					"<bplateType>%02d</bplateType>"	/* 号牌种类*/
					"<fplateNum></fplateNum>"		/* 车牌号码*/
					"<fplateColor></fplateColor>"	/* 车牌颜色*/
					"<fplateType></fplateType>"		/* 车牌种类*/
					"<carColor>%s</carColor>"			/* 车身颜色*/
					"<carType>%s</carType>"			/* 车连类型*/////////////////////////////////%s
					"<carBrand>%s</carBrand>"			/* 车辆品牌*/
					"<carLength></carLength>"			// 车长
					"<carWidth></carWidth>"			// 车宽
					"<img1_path>%s</img1_path>"			// 图片1的相对路径
					"<img2_path></img2_path>"			// 图片2的相对路径
					"<img3_path></img3_path>"			// 图片3的相对路径
					"<img4_path></img4_path>"			// 图片4的相对路径
					"<img_flag>%d</img_flag>"			/* 图片的存储方式*/
					"<ftp_flag>%d</ftp_flag>"			/* ftp 服务器的位置*/
					"<platePos></platePos>"			// 号牌定位
					"<driverPos></driverPos>"			// 人脸定位
					"<carSpeed>%d</carSpeed>"			/* 车速*/
					"<valdate>%d</valdate>"			/* 上传校验字段*/	
					"<isUpFile>%d</isUpFile>"			/* 是否同步上传图片*/
				"</listInfo>"
			"</rootInfo>\n",
			strDevCode.c_str()/*设备编号*/,   GetTimeCurrent().c_str()/*发送时间*/, 
			strPlace.c_str(), nDirection/*道路方向*/,   nRoadID/*方向编号*/,
			GetRoadIndexType(sqlQuery.getIntFileds("CHANNEL"), nRoadID),
			sqlQuery.getStringFileds("TIME").c_str(), sqlQuery.getIntFileds("MITIME")/*违章时间*/,	
			2/*车牌标志*/, strCard.c_str()/*车牌号码*/,  CardClolr(nCardColor)/* 车牌颜色*/,GetPlateType(nCardColor)/*车牌种类*/, strCarColor.c_str()/*车身颜色*/, 
			strCarType.c_str()/*车辆类型*/,  strCarFactory.c_str()/*车辆的品牌*/, m_strHistoryRemotePath.c_str(),
			2/*图片存储方式*/,  2/*ftp的服务位置*/, 	fSpeed/*车速*/,   GetRandCode()/*上传校验字段*/,	1/*是否同步上传图片*/ );
	}
	else
	{
		UINT32 nLimitSpeed = 0;
		UINT32 nRedLightTime = 0;
		GetSpeedAndRedLightTime(sqlQuery.getIntFileds("CHANNEL"), sqlQuery.getIntFileds("ROAD"),nLimitSpeed,nRedLightTime);	// 道路最大速度

		string strRedBeginTime = "";
		string strRedEndTime = "";
		if (sqlQuery.getIntFileds("PECCANCY_KIND") == DETECT_RESULT_RED_LIGHT_VIOLATION)  // 闯红灯
		{
			MakeRedLightBeginEndTime(sqlQuery, strRedBeginTime, strRedEndTime,nRedLightTime);
		}


		snprintf(szBuf, 1024 * 2, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
			"<rootInfo>"
				"<headInfo>"
					"<devCode>%s</devCode>"				/* 设备编号*/
					"<license></license>"				// 设备许可证
					"<comName></comName>"				// 设备厂家注册名
					"<comWord></comWord>"				// 设备注册密码
					"<sendTime>%s</sendTime>"				/* 发送时间*/
					"<ver>1.1</ver>"						// 版本号
				"</headInfo>"							
				"<vioInfo>"
					"<roadcode>%s</roadcode>"				/* 道路编号*/
					"<dcode>%02d</dcode>"				/* 方向编号*/
					"<lcode>%d</lcode>"					/* 车道编号*/
					"<ltype>%02d</ltype>"				/* 车道类型*/
					"<vioType>%d</vioType>"				/* 违法类型*/
					"<vioTime>%s:%03d</vioTime>"			/* 违法时间*/
					"<red_start_time>%s</red_start_time>"		/* 红灯开始时间*/
					"<red_end_time>%s</red_end_time>"	/* 红灯结束时间*/
					"<platenum>%s</platenum>"				/* 车牌号码*/
					"<plateColor>%d</plateColor>"			/* 车牌的颜色*/
					"<plateType>%02d</plateType>"			/* 车牌的种类*/
					"<carClass>%d</carClass>"				/* 车辆分类*/
					"<carColor>%s</carColor>"				/* 车身颜色*/
					"<carType>%s</carType>"				/* 车辆类型*/
					"<carBrand>%s</carBrand>"				/* 车辆品牌*/
					"<speedCar>%d</speedCar>"				/* 车辆速度*/
					"<speedStd>%d</speedStd>"				// 使用的道路速度
					"<speedStd_big></speedStd_big>"		// 道路打车限速
					"<speedStd_sml></speedStd_sml>"		// 车辆小车限速
					"<img1_path>%s</img1_path>"				// 第一张违法图片的ftp 路径
					"<img2_path></img2_path>"				// 第二张违法图片的发图片路径
					"<img3_path></img3_path>"				// 第三张违法图片的路径
					"<img4_path></img4_path>"				// 第三张违法图片的路径
					"<video_path></video_path>"			// 录像文件的相对路径
					"<img_flag>%d</img_flag>"				/* 图片和录像文件的存储方式*/
					"<ftp_flag>%d</ftp_flag>n"				/* ftp服务器的位置*/
					"<valdate>%d</valdate>"				/* 验证码*/
					"<isUpFile>%d</isUpFile>"				/* 是否同步上传图片*/
				"</vioInfo>"
			"</rootInfo>\n",
			strDevCode.c_str()/*设备编号*/,  strCurrentTime.c_str()/*传送的时间*/,
			strPlace.c_str()/*道路编号*/, nDirection/*方向 */, nRoadID/* 车道编号*/,
			GetRoadIndexType(sqlQuery.getIntFileds("CHANNEL"),nRoadID)
			/*车道类型*/,  GetCarViolationType(sqlQuery.getIntFileds("PECCANCY_KIND"))/*pPlate->uViolationType违法类型*/,
			sqlQuery.getStringFileds("TIME").c_str(), sqlQuery.getIntFileds("MITIME")/*违法时间*/,
			strRedBeginTime.c_str()/*红灯开始时间*/, strRedEndTime.c_str()/*红灯结束时间*/,
			strCard.c_str()/*车牌号码*/,  CardClolr(nCardColor)/*车牌颜色*/,   GetPlateType(nCardColor)/*车牌种类*/,
			GetCarClass(strCard.c_str())/*车辆分类*/,  strCarColor.c_str()/*车身颜色*/,  strCarType.c_str()/*车辆类型*/,
			strCarFactory.c_str()/*车辆品牌*/,  fSpeed/*车速*/,nLimitSpeed, m_strHistoryRemotePath.c_str(),
			2/*图片和图像的存储方式*/, 2/*ftp服务器的位置*/,  GetRandCode()/*上传校验字段*/,  1/*是否同步上传*/);		
	}

	string strXML(szBuf);
	return strXML;
}

bool CYiChangCenter::AddResult(const std::string& strResult)
{
	//添加到列表
	//SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)strResult.c_str();

	//switch(sDetectHeader->uDetectType)
	//{
	//	//case MIMAX_EVENT_REP:	//事件(送中心端)
	//case MIMAX_PLATE_REP:   //车牌
	//	//case MIMAX_STATISTIC_REP:  //统计
	//	//case PLATE_LOG_REP:   //日志
	//	//case EVENT_LOG_REP:
	//加锁
	pthread_mutex_lock(&m_Result_Mutex);
	//if(m_listRecord.size() > 5)//防止堆积的情况发生
	//{
	//	//LogError("记录过多，未能及时发送!\r\n");
	//	m_listRecord.pop_back();
	//}
	m_listRecord.push_front(strResult);
	//LogError("记录=%d\r\n",m_ChannelResultList.size());
	//解锁
	pthread_mutex_unlock(&m_Result_Mutex);
	//	break;
	//default:
	//	LogError("11111未知数据[%x],取消操作!\r\n",sDetectHeader->uDetectType);
	//	return false;
	//}
	return true;
}


//初始化 启动线程
bool CYiChangCenter::Init(void)
{
	
	pthread_t id;				//线程id
	pthread_attr_t attr;		//线程属性
	pthread_attr_init(&attr);	//初始化
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);		//分离线程
	/*int nret=pthread_create(&id,&attr,ConnectProcess,this);			//连接中心端
	if(nret!=0)
	{
		LogError("ConnectProcess服务无法启动!\r\n");		//失败
		g_bEndThread = true;
		return false;
	}*/

	//接收中心端消息
	int nret=pthread_create(&id,&attr,RecvCenterMsgProcess,this);
	if(nret!=0)
	{
		//失败
		LogError("RecvCenterMsgProcess服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//发送实时数据
	nret=pthread_create(&id,&attr,SendPlateMsgProcess,this);
	if(nret!=0)
	{
		//失败
		LogError("宜昌 Init,SendPlateMsgProcess服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}

	//给中心端发送心跳
	/*nret=pthread_create(&id,&attr,SendHeartBeatProcess,this);
	if(nret!=0)
	{
		//失败
		LogError("SendHeartBeatProcess服务无法启动!\r\n");
		g_bEndThread = true;
		return false;
	}*/
	//发送历史数据的
	nret = pthread_create(&id, &attr, SendHistoryDataThread, this);
	if (nret != 0)
	{
		LogError("宜昌创建历史发送线程失败 \n");
		return false;
	}

	pthread_attr_destroy(&attr);
	return true;
}

void CYiChangCenter::Unit(void)
{
	m_bCenterLink = false;
}

/*
函数功能：连接到指定IP和端口的服务器
输入：无
输出：无
返回值：连接成功返回true，否则返回false
*/
bool CYiChangCenter::ConnectToCS()
{
	if (m_bCenterLink)
	{
		return true;
	}
	if (g_strControlServerHost.empty() || g_strControlServerHost == "0.0.0.0" || g_nControlServerPort <= 0)
	{
		LogError("宜昌 中心数据服务器连接参数异常:host=%s,port=%d\n",g_strControlServerHost.c_str(), g_nControlServerPort);
		return false;
	}

	if (!mvPrepareSocket(m_nCenterSocket))
	{
		LogError("宜昌 ConnectToCS准备连接中心数据服务器套接字失败!\n");
		return false;
	}

	if (!mvWaitConnect(m_nCenterSocket, g_strControlServerHost, g_nControlServerPort))
	{
		LogError("宜昌 尝试连接中心数据服务器失败，host=%s,port=%d!\n",g_strControlServerHost.c_str(), g_nControlServerPort);
		return false;
	}
	LogNormal("Connect to CS %s,port:%d success\n",g_strControlServerHost.c_str(),g_nControlServerPort);
	m_bCenterLink = true;
	return true;
}

/*
函数功能：向中心端发送心跳包和车牌信息包
输入：nSocket：中心端套接字，strData要发送的数据（心跳包不需加密，车牌包后续需加密），nType：0：心跳包，1：车牌包
输出：无
返回值：发送成功返回true，否则返回false
*/
bool CYiChangCenter::SendMsg(const int nSocket, const string& strData, int nType)
{
	if(nSocket <= 0)
	{
		return false;
	}

	string strFullMsg("");
	strFullMsg.append( (char *)strData.c_str(), strData.size() );

	if (nType == 0)
	{
		if(!mvSendMsgToSocket(nSocket,strFullMsg,true))
		{
			mvCloseSocket(m_nCenterSocket);
			m_bCenterLink = false;
			LogError("宜昌 SendMsg SendHeartBeatMsg error\n");
			return false;
		}
	}
	//else if (nType == 1)
	//{
	//	if (strFullMsg.size() <= 10+sizeof(Type_Struct_Frt_Vhc_Data))//没有图片数据关闭连接
	//	{
	//		LogError("1111SendMsg no PicData\n");
	//		CloseCenterSocket();
	//	}
	//	if (!mvSendMsgToSocket(nSocket,strFullMsg,true))
	//	{
	//		LogError("2222SendMsg SendPlatAndPicMsg error\n");
	//		return false;
	//	}
	//}
	m_nHeartBeatCount = 0;
	return true;
}

/*
* 函数介绍：接收消息
* 输入参数：nSocket-要接收消息的套接字
* 
* 返回值 ：成功返回true，否则false
*/
bool CYiChangCenter::mvRecvSocketMsg(int nSocket)
{
	if (nSocket <= 0)
	{
		return false;
	}

	UINT16 nHeader = 0;
	UINT16 nType = 0;
	UINT32 nlength = 0;
	UINT32 nValue = 0;
	UINT16 nTail = 0;
	SYSTEMTIME nSysTemTime;


	if (recv(nSocket, &nHeader, sizeof(nHeader), MSG_NOSIGNAL) <= 0)   // 接受头
	{
		return false;
	}

	if (nHeader  != 0xAAAA)		// 如果头不对 返回 						
	{
		LogError("recv Error Header:%d\n",nHeader);
		return false;
	}

	if (recv(nSocket, &nType, sizeof(nType), MSG_NOSIGNAL) <= 0)   // 接受数据类型
	{
		return false;
	}

	if (nType == 1103)		//心跳包  
	{
		m_nHeartBeatCount = 0;
		if (recv(nSocket, &nlength, sizeof(nlength), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg HeartBeat recv nlength error\n");
			return false;
		}
		if (recv(nSocket, &nValue, sizeof(nValue), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg HeartBeat recv nValue error\n");
			return false;
		}
		if (recv(nSocket, &nTail, sizeof(nTail), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg HeartBeat recv nTail error\n");
			return false;
		}
		//LogNormal("HeartBeat Package\n");
	}
	else if (nType == 1104)//车辆信息返回包
	{
		m_nHeartBeatCount = 0;
		if (recv(nSocket, &nlength, sizeof(nlength), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg mvRecvSocketMsg PlateMsg Return recv nlength error\n");
			return false;
		}
		if (recv(nSocket, &nValue, sizeof(nValue), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg mvRecvSocketMsg PlateMsg Return recv nValue error\n");
			return false;
		}
		if (recv(nSocket, &nTail, sizeof(nTail), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg mvRecvSocketMsg PlateMsg Return recv nTail error\n");
			return false;
		}
	}
	else if (nType == 1108)//网络校时包
	{
		m_nHeartBeatCount = 0;
		if (recv(nSocket, &nlength, sizeof(nlength), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg to make time right recv nlength error\n");
			return false;
		}
		if (recv(nSocket, &nSysTemTime, sizeof(nSysTemTime), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg to make time right recv SysTemTime error\n");
			return false;
		}
		if (recv(nSocket, &nTail, sizeof(nTail), MSG_NOSIGNAL) <= 0)
		{
			LogError("mvRecvSocketMsg to make time right recv nTail error\n");
			return false;
		}

		//OnSysTimeSetup(nSysTemTime);
		//LogNormal("Package to modify our time\n");
	}
	else 
	{
		LogError("%d is not the Type we need\n",nType);
		return false;
	}

	return true;
}

/*
函数功能：关闭中心端套接字，并置中心端连接标志为false
输入：无
输出：无
返回值:无 
*/
void CYiChangCenter::CloseCenterSocket(void)
{
	LogError("30s Later recv nothing,close the socket\n");
	shutdown(m_nCenterSocket,2);
	close(m_nCenterSocket);
	m_nCenterSocket = -1;
	m_bCenterLink = false;
	m_nHeartBeatCount = 0;
}

// 取出一条数据进行处理
void CYiChangCenter::DealResult(void)
{
	std::string response("");
	if (!response.empty())
	{
		response.clear();
	}
	
	pthread_mutex_lock(&m_Result_Mutex);	//加锁
	if(m_listRecord.size()>0)				//判断是否有命令
	{
		std::list<string>::iterator it = m_listRecord.begin();   //取最早命令
		response = *it;					//保存数据
		m_listRecord.pop_front();			//删除取出的命令
	}
	pthread_mutex_unlock(&m_Result_Mutex);	//解锁

	if(response.size()>0)					//处理消息
	{
		OnSendResult(response);
	}

	usleep(1000*1);
}

// 通过ftp 协议发送数据
int CYiChangCenter::SendImageByFtp(string strPicData, string strRemotePath)
{
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
	{
		return false;
	}

	int bRet = g_FtpCommunication.VideoDoPut(NULL, (char*)strRemotePath.c_str(), strPicData,\
		true, true, (char*)strRemotePath.c_str(), false);
	if (!bRet)
	{
		LogError("宜昌 ftp 发送图片错误！ ");
		return -1;
	}

	return 0;
}

//发送 结果 
void CYiChangCenter::OnSendResult(std::string& strResult)
{
	string strResultData;
	strResultData.append((char*)(strResult.c_str()+sizeof(SRIP_DETECT_HEADER)),strResult.size()-sizeof(SRIP_DETECT_HEADER));
	RECORD_PLATE *pPlate = (RECORD_PLATE *)(strResultData.c_str());
	
	
	string strLocalImagePath(pPlate->chPicPath);				// 本地图片路径 
	string strImageData = GetImageByPath(strLocalImagePath);
	string strRemotePath = MakeSaveImagePath(pPlate);		//发图片的路径
	// 先发送图片
	int nRet = g_FtpCommunication.VideoDoPut(NULL, (char*)strRemotePath.c_str(),
		strImageData, true, true, (char*)strRemotePath.c_str(), true);
	if (!nRet)
	{
		LogError("宜昌 ftp 历史图片传送失败 \n");
		return ;
	}

	string strPlace = g_skpDB.GetPlace(pPlate->uChannelID );

	if (pPlate)
	{
		char szBuf[1024 * 2] = {0};
		if (pPlate->uViolationType == 0) //车辆通行
		{			
			snprintf(szBuf, 1024 * 2, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
				"<rootInfo>"
				"<headInfo>"
				"<devCode>%s</devCode>"
				"<license></license>"
				"<comName></comName>"
				"<comWord></comWord>"
				"<sendTime>%s</sendTime>"
				"<ver>1.1</ver>"
				"</headInfo>"
				"<listInfo>"
				"<roadcode>%s</roadcode>"
				"<dcode>%02d</dcode>"
				"<lcode>%d</lcode>"
				"<ltype>%02d</ltype>"
				"<plateTime>%s:%03d</plateTime>"
				"<plateFlag>%d</plateFlag>"
				"<bplateNum>%s</bplateNum>"
				"<bplateColor>%d</bplateColor>"
				"<bplateType>%02d</bplateType>"
				"<fplateNum></fplateNum>"
				"<fplateColor></fplateColor>"
				"<fplateType></fplateType>"
				"<carColor>%s</carColor>"
				"<carType>%s</carType>"
				"<carBrand>%s</carBrand>"
				"<carLength></carLength>"
				"<carWidth></carWidth>"
				"<img1_path>%s</img1_path>"
				"<img2_path></img2_path>"
				"<img3_path></img3_path>"
				"<img4_path></img4_path>"
				"<img_flag>%d</img_flag>"
				"<ftp_flag>%d</ftp_flag>"
				"<platePos></platePos>"
				"<driverPos></driverPos>"
				"<carSpeed>%d</carSpeed>"
				"<valdate>%d</valdate>"
				"<isUpFile>%d</isUpFile>"
				"</listInfo>"
				"</rootInfo>\n",
				g_strDetectorID.c_str(), GetTimeCurrent().c_str(),
				strPlace.c_str(), pPlate->uDirection, pPlate->uRoadWayID, 
				GetRoadIndexType(pPlate->uChannelID, pPlate->uRoadWayID), GetTime(pPlate->uTime,0).c_str(),pPlate->uMiTime,
				2, pPlate->chText, CardClolr(pPlate->uColor), GetPlateType(pPlate->uColor)/*车牌种类*/, CarColor(pPlate->uCarColor1),  GetCarType(pPlate->uTypeDetail).c_str()/*车辆类型*/,
				CarLable(pPlate->uCarBrand), strRemotePath.c_str(), 2, 2, pPlate->uSpeed, GetRandCode()/*上传校验字段*/,1);
		}
		else	//车辆违章
		{
			UINT32 nLimitSpeed = 0;
			UINT32 nRedLightTime = 0;
			GetSpeedAndRedLightTime(pPlate->uChannelID, pPlate->uRoadWayID,nLimitSpeed,nRedLightTime);	// 道路最大速度

			string strRedBeginTime = "";
			string strRedEndTime = "";
			if (pPlate->uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)  // 闯红灯
			{
				MakeRedLightBeginEndTime(pPlate, strRedBeginTime, strRedEndTime,nRedLightTime);
			}

			snprintf(szBuf, 1024 * 2, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
				"<rootInfo>"
				"<headInfo>"
				"<devCode>%s</devCode>"
				"<license></license>"
				"<comName></comName>"
				"<comWord></comWord>"
				"<sendTime>%s</sendTime>"
				"<ver>1.1</ver>"
				"</headInfo>"
				"<vioInfo>"
				"<roadcode>%s</roadcode>"
				"<dcode>%02d</dcode>"
				"<lcode>%d</lcode>"
				"<ltype>%02d</ltype>"
				"<vioType>%d</vioType>"
				"<vioTime>%s:%03d</vioTime>"
				"<red_start_time>%s</red_start_time>"
				"<red_end_time>%s</red_end_time>"
				"<platenum>%s</platenum>"
				"<plateColor>%d</plateColor>"
				"<plateType>%02d</plateType>"
				"<carClass>%d</carClass>"
				"<carColor>%s</carColor>"
				"<carType>%s</carType>"
				"<carBrand>%s</carBrand>"
				"<speedCar>%d</speedCar>"
				"<speedStd>%d</speedStd>"
				"<speedStd_big></speedStd_big>"
				"<speedStd_sml></speedStd_sml>"
				"<img1_path>%s</img1_path>"
				"<img2_path></img2_path>"
				"<img3_path></img3_path>"
				"<img4_path></img4_path>"
				"<video_path></video_path>"
				"<img_flag>%d</img_flag>"
				"<ftp_flag>%d</ftp_flag>"
				"<valdate>%d</valdate>"
				"<isUpFile>%d</isUpFile>"
				"</vioInfo>"
				"</rootInfo>\n",
				g_strDetectorID.c_str(), GetTimeCurrent().c_str(),
				strPlace.c_str(), pPlate->uDirection, pPlate->uRoadWayID, 
				GetRoadIndexType(pPlate->uChannelID, pPlate->uRoadWayID)/*车道类型*/, GetCarViolationType(pPlate->uViolationType)/*违法类型*/,
				GetTime(pPlate->uTime,0).c_str(),pPlate->uMiTime, strRedBeginTime.c_str(), strRedEndTime.c_str(),
				pPlate->chText, CardClolr(pPlate->uColor), GetPlateType(pPlate->uColor)/*车牌种类*/, GetCarClass(pPlate->chText)/*车辆分类*/, CarColor(pPlate->uCarColor1), GetCarType(pPlate->uTypeDetail).c_str()/*车辆类型*/,
				CarLable(pPlate->uCarBrand), pPlate->uSpeed, nLimitSpeed, strRemotePath.c_str(), 2, 2, GetRandCode()/*上传校验字段*/, 1);		
		}

		string strXML(szBuf);
	
		if ( !ConnectToCS())
		{
			return ;
		}
		//pthread_mutex_lock(&m_send_mutex);
		if (SendMsg(m_nCenterSocket,strXML,0))
		{
			char szSqlBuf [125] = {0};
			sprintf(szSqlBuf, "update NUMBER_PLATE_INFO set STATUS = 1 where ID = %u", pPlate->uSeq);
			string strSql(szSqlBuf);
			g_skpDB.execSQL(strSql);
			cerr<<"宜昌 socket 发送成功		\n";
		}
		//pthread_mutex_unlock(&m_send_mutex);
	}
		
}
// 制作红灯的开始时间 和红灯的结束时间
int CYiChangCenter::MakeRedLightBeginEndTime(RECORD_PLATE * pPlate, \
										string & strBeginTime, string & strEndTime,int nRedLightTime)
{
	strBeginTime = GetTime(pPlate->uTime - pPlate->uSignalTime, 0) ;

	strEndTime = GetTime(pPlate->uTime - pPlate->uSignalTime + nRedLightTime, 0);

	return 0;
}

int CYiChangCenter::MakeRedLightBeginEndTime(MysqlQuery sqlQuery, string & strBeginTime, string & strEndTime,int nRedLightTime)
{
	string strTime = sqlQuery.getStringFileds("TIME");
	unsigned long uTime = MakeTime(strTime);

	strBeginTime =GetTime( uTime -  sqlQuery.getIntFileds("REDLIGHT_TIME"), 0);
	
	strEndTime = GetTime( uTime -  sqlQuery.getIntFileds("REDLIGHT_TIME") + nRedLightTime, 0);
	return 0;
}

// 得到这个车道的类型
int CYiChangCenter::GetRoadIDType(int nChannelID, int RoadWayID)
{
	//违章检测参数
	VTS_GLOBAL_PARAMETER vtsGlobalPara;
	CXmlParaUtil xmlvts;
	VTSParaMap  vtsObjectParaMap;
	xmlvts.LoadVTSParameter(vtsObjectParaMap, nChannelID, vtsGlobalPara);

	VTSParaMap::iterator it = vtsObjectParaMap.find(RoadWayID);
            if(it == vtsObjectParaMap.end())
            return 99;
	if (it->second.nRoadDirection == 0)		// 直行车道
	{
		return 01;
	}
	if (it->second.nRoadDirection == 1)		// 左转车道
	{
		return 02;
	}
	if (it->second.nRoadDirection == 3)		// 右转车道
	{
		return 03;
	}
	if (it->second.nRoadDirection == 2)		//直行加左转
	{
		return 04;
	}
	if (it->second.nRoadDirection == 2)		//直行加右转
	{
		return 05;
	}
	if (it->second.nRoadType == 3)
	{
		return 06;
	}
	if (it->second.nRoadType == 2)
	{
		return 07;
	}

	return 99;
}

//根据通道号，车道号获取相应车道的限速值

void CYiChangCenter::GetSpeedAndRedLightTime(int nChannel,int nRoadId,UINT32& nSpeed,UINT32& nRedLightTime)
{
	nSpeed = 0;
	nRedLightTime = 0;

	map<UINT32,UINT32> nMaxSpeedMap;
	nMaxSpeedMap.clear();

	map<UINT32,UINT32> nRedLightTimeMap;
	nRedLightTimeMap.clear();

	if(g_nDetectMode == 2)
	{
		MvDspGlobalSetting dspSettingStrc;
		CALIBRATION calibration;

		CXmlParaUtil xml;
		//if(xml.LoadDspSettingInfo(dspSettingStrc,calibration,nChannel))
		if(xml.LoadDspSettingFile(dspSettingStrc,nChannel))
		{
			for(int i = 0;i<dspSettingStrc.nChannels;i++)
			{
				nMaxSpeedMap.insert(make_pair(dspSettingStrc.ChnlRegion[i].nVerRoadIndex,dspSettingStrc.ChnlRegion[i].nRadarAlarmVelo));
				nRedLightTimeMap.insert(make_pair(dspSettingStrc.ChnlRegion[i].nVerRoadIndex,dspSettingStrc.ChnlRegion[i].m_pRedLightDelayTime[1]));
			}
		}
	}
	else 
	{
		//CXmlParaUtil xml;
		//xml.GetMaxSpeed(nMaxSpeedMap,nChannel);
	}


	if (nMaxSpeedMap.size() <= 0)
	{
		//LogError("通道%d不存在，或者没有限速的配置文件\n",nChannel);
		return;
	}
	map<UINT32,UINT32>::iterator it = nMaxSpeedMap.find(nRoadId);
	if (it != nMaxSpeedMap.end())
	{
		nSpeed = it->second;
	}
	
	it = nRedLightTimeMap.find(nRoadId);
	if (it != nRedLightTimeMap.end())
	{
		nRedLightTime = it->second;
	}
}



/*
 * 函数的功能是： 车牌转换， 如果车牌号是****（无车牌） 要转换成00000 的形式
 * 参数是： 车牌号码
*/
string CYiChangCenter::CardChang(string  strCard)
{
	if (*(strCard.c_str()) == '*')
	{
		strCard = "00000000";
	}
	return strCard;
}

/*
 *  如果该图片信息成功发送到对方的ftp上， 则要把数据库的对应的标志位设置为1 
*/
int CYiChangCenter::UpdateMysl(int id)
{
	char szSql[124] = {0};
	sprintf(szSql, "update NUMBER_PLATE_INFO set STATUS = 1 where ID = %u", id);
	string strSql(szSql);
	
	if (g_skpDB.execSQL(strSql) != 0 )
	{
		LogNormal("宜昌 updateSql Set STATUS error \n");
		return -1;
	}
	return 0;
}

//获取随机防伪码
int CYiChangCenter::GetRandCode()
{
	//srand( (unsigned)time( NULL ) );
	getpid();
	int nRandCode = 1 + (int)(9*1.0*rand()/(RAND_MAX+1.0));
	return nRandCode;
}


