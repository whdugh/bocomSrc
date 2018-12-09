#include "WuHanCenter.h"
#include "CommonHeader.h"
#include "BrandSubSection.h"
#include "CarLabel.h"
/*
 *  文件名： WuHanCenter.cpp
 *  功能： 武汉市局中心端类型， 把卡口的图片， 违章的图片 按照协议放到对方的ftp 上
 * 作者： 牛河山
*/

WuHanCenter g_WuHanCenter;

WuHanCenter::WuHanCenter()
{
	m_nThreadRealTime = 0;
	m_nThreadHistory = 0;
	pthread_mutex_init(&m_Result_Mutex, NULL);
	m_bIsCenterLink = false;
	m_nNo = 1;
}
WuHanCenter::~WuHanCenter()
{
	Uninit();
	m_bIsCenterLink = false;
	pthread_mutex_destroy(&m_Result_Mutex);
}

/*
 *创建实时发送线程， 和历史发送线程
 */
int WuHanCenter::Init()
{
	pthread_t		id;
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	
	if (pthread_create(&m_nThreadRealTime, &attr, SendRealTimeData, this) != 0)
	{
		LogNormal("武汉中心端 创建实时发送线程失败\n");
		return -1;
	}
	
	if (pthread_create(&m_nThreadHistory, &attr, SendHistoryData, this) != 0)
	{
		LogNormal("武汉中心端， 创建历史发送线程失败");
	}
	m_nNo = 1;
	return 0;
}


bool WuHanCenter::Uninit()
{
	//停止线程
	if(m_nThreadRealTime != 0)
	{
		pthread_join(m_nThreadRealTime, NULL);
		m_nThreadRealTime = 0;
	}
	if(m_nThreadHistory != 0)
	{
		pthread_join(m_nThreadHistory, NULL);
		m_nThreadHistory = 0;
	}
	m_listRecord.clear();
}

/*
 * 增加一条消息未处理的消息
*/
bool WuHanCenter::AddResult(const std::string& strResult)
{
	//cerr<<"武汉 addResult \n";
	pthread_mutex_lock(&m_Result_Mutex);
	m_listRecord.push_front(strResult);
	pthread_mutex_unlock(&m_Result_Mutex);
	return true;
}

// 发送实时数据的入口
void * WuHanCenter::SendRealTimeData(void * pAg)
{
	WuHanCenter * pThis = (WuHanCenter *)pAg;
	
	//cerr<<" 武汉 实时数据\n";
	while (!g_bEndThread)
	{
		pThis->DealResult();
		usleep(1000);
	}

	return NULL;
}

// 发送历史数据信息的入口
void * WuHanCenter::SendHistoryData(void * pAg)
{
	WuHanCenter * pThis = (WuHanCenter *)pAg;
	while(!g_bEndThread)
	{
		if(g_nSendHistoryRecord == 1)
		{
			string strResultData = pThis->SqlDataTransitionRECORD_PLATE();
			if (!strResultData.empty())
			{
				pThis->OnSendResult(strResultData);
			}
			usleep(1000);
		}
		usleep(1000);
	}
}

// 取出一条数据进行处理
void WuHanCenter::DealResult(void)
{
	std::string response("");
	if (!response.empty())
	{
		response.clear();
	}
	pthread_mutex_lock(&m_Result_Mutex);	// 加锁
	if(m_listRecord.size()>0)				// 判断是否有命令
	{
		std::list<string>::iterator it = m_listRecord.begin();   // 取最早命令
		response = *it;						// 保存数据
		m_listRecord.pop_front();			// 删除取出的命令
	}
	pthread_mutex_unlock(&m_Result_Mutex);	// 解锁

	if(response.size() > 0)					// 处理消息
	{
		OnSendResult(response);
	}

	usleep(1000*1);
}


string WuHanCenter::SqlDataTransitionRECORD_PLATE()
{
	if (g_strFtpServerHost.empty() || g_strFtpServerHost == "0.0.0.0" || g_nFtpPort <= 0)
	{
		return "";
	}

	//3分钟之前的数据 防止实时记录正在发送又将其作为历史数据发送的情况
	unsigned int uTimeStamp = GetTimeStamp() - 60 * 3;		
	string strTime = GetTime(uTimeStamp);

	char szSql[1024]={0};
	sprintf(szSql, "select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME <= '%s'\
				   ORDER BY TIME desc limit 1; ", strTime.c_str());
	string  strSql(szSql);

	MysqlQuery sqlData = g_skpDB.execQuery(strSql);
	if ( sqlData.eof())							
	{
		return "";		// 说明没有数据		
	}

	RECORD_PLATE  plate;
	plate.uSeq = sqlData.getUnIntFileds("ID");							// 编号id
	string strPlace = sqlData.getStringFileds("PLACE");					// 通道地点
	memcpy(plate.chPlace, strPlace.c_str(), strPlace.size() ); 
	
	plate.uChannelID = sqlData.getIntFileds("CHANNEL");

	string strTime2 = sqlData.getStringFileds("TIME");
	unsigned long uTime = MakeTime(strTime2);
	plate.uTime = uTime;
	//plate.uTime = sqlData.getIntFileds("TIME");					// 时间
	
	string strCarCard = sqlData.getStringFileds("NUMBER");
	memcpy(plate.chText, strCarCard.c_str(), strCarCard.size());		// 车牌号
	//cerr<<" wuhan "<<plate.uSeq<<"\t--"<<plate.chText<<"\t---"<<strCarCard<<"\n";
	plate.uSpeed =sqlData.getIntFileds("SPEED");						// 车速
	plate.uDirection = sqlData.getIntFileds("DIRECTION");				// 行驶方向
	plate.uRoadWayID = sqlData.getIntFileds("ROAD");					// 违法分类
	plate.uViolationType = sqlData.getIntFileds("PECCANCY_KIND");		// 违法代码
	plate.uPosLeft   = sqlData.getIntFileds("POSLEFT");              // 车牌在图片的坐标
	plate.uPosTop    = sqlData.getIntFileds("POSTOP");
	plate.uPosRight  = sqlData.getIntFileds("POSRIGHT"); 
	plate.uPosBottom = sqlData.getIntFileds("POSBOTTOM");
	plate.uCarColor1 = sqlData.getIntFileds("CARCOLOR");				// 车身颜色
	plate.uCarBrand	 = sqlData.getIntFileds("FACTORY");				// 车辆品牌
	string strPicPath = sqlData.getStringFileds("PICPATH");			// 车辆大图
	memcpy(plate.chPicPath, strPicPath.c_str(), strPicPath.size());

	string strResult;
	SRIP_DETECT_HEADER sripTemp;

	strResult.append((char *)&sripTemp, sizeof(SRIP_DETECT_HEADER));
	strResult.append((char *)&plate, sizeof(RECORD_PLATE));

	return strResult;
}

// 处理和发送数据
int WuHanCenter::OnSendResult( string strResult)
{
	//cerr<<"武汉 onSendResult \n";
	string strResultData;
	strResultData.append((char *)strResult.c_str() + sizeof(SRIP_DETECT_HEADER),
		strResult.size() - sizeof(SRIP_DETECT_HEADER) );

	RECORD_PLATE *pPlate = (RECORD_PLATE *)(strResultData.c_str());

	string strImageData = GetImageByPath(pPlate->chPicPath );	// 得到图片
	if (strImageData.size() <= 0)
	{
		return -1;
	}
	//cerr<<"武汉 图片大小"<<strImageData.size()<<"\n";
	string strRemotePath = MakeRemoteImageName(pPlate);			// 发图片的路径

	//cerr<<"path =  "<<strRemotePath<<"\n";
	int nRet = g_FtpCommunication.VideoDoPut(NULL, (char*)strRemotePath.c_str(),strImageData, 
		true, true, (char*)strRemotePath.c_str(), true);

	if (!nRet)
	{
		//cerr <<"nRet = "<<nRet<<"\n";
		LogError("武汉 ftp 历史图片传送失败 \n");
		return -1;
	}
	else
	{
		char szSqlBuf [125] = {0};
		sprintf(szSqlBuf, "update NUMBER_PLATE_INFO set STATUS = 1 where ID = %u", pPlate->uSeq);
		string strSql(szSqlBuf);
		g_skpDB.execSQL(strSql);

	}

	return 0;
}

// 制作保存的路径
string WuHanCenter::MakeRemoteImageName(RECORD_PLATE * pPlate)
{ 
	int nIsHaveCard = 0;
	// 地点 （第一级目录）
	string strPlacePath = g_skpDB.GetPlace(pPlate->uChannelID );
	g_skpDB.UTF8ToGBK(strPlacePath);
	
	// 时间 （第二级目录）
	string strTime = GetTime(pPlate->uTime, 0);
	string strYear	= strTime.substr(0,4);
	string strMonth	= strTime.substr(5,2);
	string strDay	= strTime.substr(8,2);
	string strTimePath = strYear + strMonth + strDay;
	// 图片类型 （第三级目录）
	string strViolationPath;
	if (pPlate->uViolationType == 0 )
	{
		strViolationPath = "卡口图片";
	}
	else
	{
		strViolationPath = "违法图片";
	}
	g_skpDB.UTF8ToGBK(strViolationPath);

	// 设备编号
	string strDevCode = "0272012LF";
	char szChanSql[127] = {0};
	sprintf(szChanSql, "SELECT * FROM CHAN_INFO WHERE CHAN_ID = %d ;", pPlate->uChannelID);
	string strChanSql(szChanSql);

	MysqlQuery sqlChanData = g_skpDB.execQuery(strChanSql);
	if ( !sqlChanData.eof())							
	{
		string strPannelID = sqlChanData.getStringFileds("PANNEL_ID");
		strDevCode += strPannelID;
	}

	// 时间
	string strHour	= strTime.substr(11,2);
	string strMin	= strTime.substr(14,2);
	string strSec	= strTime.substr(17,2);
	strTime = GetTime(pPlate->uTime, 4);
	char szuMiTime[7] = {0};
	sprintf(szuMiTime, "%03d", pPlate->uMiTime);
	string struMiTime(szuMiTime);
	strTime =strTime + strHour + strMin + strSec + struMiTime;
	// 车牌号
	string strCarCard(pPlate->chText);
	if (*(strCarCard.c_str()) == '*' || *(strCarCard.c_str()) == '1' || *(strCarCard.c_str()) == '0')
	{
		nIsHaveCard = 0;
		strCarCard = "无牌";
	}
	else 
	{
		nIsHaveCard = 1;
	}
	g_skpDB.UTF8ToGBK(strCarCard);
	// 车牌颜色
	char szCardColor[5] = {0};
	sprintf(szCardColor, "%d", CardColorChange(pPlate->uColor));
	string strCardColor(szCardColor);
	// 行驶速度
	char szSpeed[10] = {0};
	sprintf(szSpeed, "%03d", pPlate->uSpeed);
	string strSpeed(szSpeed);
	// 行驶方向
	string strDirection = DirectionChange(pPlate->uDirection);
	// 车道
	char szRoadId[5] = {0};
	sprintf(szRoadId, "%d", pPlate->uRoadWayID);
	string strRoadId(szRoadId);
	// 违法行为代码
	char szViolationType[10] = {0};
	sprintf(szViolationType, "%d", CarViolationTypeChange(pPlate->uViolationType) );
	if (pPlate->uViolationType == 0 )
	{
		sprintf(szViolationType, "00000");
	}
	string strViolation(szViolationType);
	// 号牌在图片的位置
	char szTopLeftCorner[20] = {0};
	char sZDownRightCorner[20] = {0};
	
	if (nIsHaveCard == 1)
	{
		sprintf(szTopLeftCorner, "%d,%d,", pPlate->uPosLeft, pPlate->uPosTop);
		sprintf(sZDownRightCorner, "%d,%d", pPlate->uPosRight, pPlate->uPosBottom);
	}
	else
	{
		sprintf(szTopLeftCorner, "0,0,");
		sprintf(sZDownRightCorner, "0,0");
	}
	string strTopLiftCorner(szTopLeftCorner);
	string strDownRightCorner(sZDownRightCorner);
	string strCoordinate = strTopLiftCorner + strDownRightCorner;
	// 车身颜色
	string strCarColor = CarColorChange(pPlate->uCarColor1);
	g_skpDB.UTF8ToGBK(strCarColor);
	// 车辆品牌
	string strCarFactory = CarFactoryChange(pPlate->uCarBrand);
	g_skpDB.UTF8ToGBK(strCarFactory);
	char strCaptureNo[100] ={0};
	sprintf(strCaptureNo, "%03d-1-1",m_nNo++);
	if (m_nNo > 999)
	{
		m_nNo = 1;
	}
	std::string strDeviceId = g_skpDB.GetDeviceId(pPlate->uCameraId);

	string strImageName = strPlacePath +"/" + strViolationPath +"/"+ strTimePath +"/" + strHour +"/" +		\
		strDevCode +"-"+ strTime + "-" + strCaptureNo +"-"+ strCarCard +"-"+ strCardColor +"-"+ strSpeed +		\
		"-"+ strDirection +"-"+ strRoadId +"-"+ strViolation +"-"+ strCoordinate +			\
		"-"+ strCarColor + "-"+ strCarFactory + "-" + strDeviceId + ".jpg";
	return strImageName;
}

// 车牌颜色的转换
int WuHanCenter::CardColorChange(int nColor)
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
		default:
			nColor = 4;
	}
	return nColor;
}

// 行驶方向的转换
string WuHanCenter::DirectionChange(int nDirection)
{
	string strDirection;

	switch(nDirection)
	{
		case 1:
			strDirection = "E";
			break;
		case 2:
			strDirection = "W";
			break;
		case 3:
			strDirection = "S";
			break;
		case 4:
			strDirection = "N";
			break;
	}

	return strDirection;
}

// 获取违章类型
int WuHanCenter::CarViolationTypeChange(int nType)
{
	int nViolationType = 13450;
	switch (nType)
	{
	case DETECT_RESULT_RED_LIGHT_VIOLATION:		//闯红灯
		nViolationType = 13020;
		break;
	case DETECT_RESULT_RETROGRADE_MOTION:		//逆行
		nViolationType = 13020;
		break;
	case DETECT_RESULT_EVENT_GO_FAST:			//超速
		nViolationType = 13030;
		break;
	case DETECT_RESULT_EVENT_GO_CHANGE:			//8 违章变道
	case DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD: //18 小车出现在禁行车道
	case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD:   //19 大车出现在禁行车道
	case DETECT_RESULT_BIG_IN_FORBIDDEN_TIME:   //20 大车出现在禁行时间
	case DETECT_RESULT_FORBID_LEFT:				// 违反禁止标线指示
	case DETECT_RESULT_FORBID_RIGHT:			// 违反禁止标线指示
	case DETECT_RESULT_FORBID_STRAIGHT:			// 违反禁止标线指示
	case DETECT_RESULT_PRESS_WHITELINE:			// 违反禁止标线指示
		nViolationType = 12080;
		break;
	case DETECT_RESULT_PRESS_LINE:				//27 压黄线
		nViolationType = 13450;
		break;
	case DETECT_RESULT_NO_TURNAROUND:           //59 禁止调头
		nViolationType = 12090;
		break;

	}
	return nViolationType;
}

// 车身颜色
string WuHanCenter::CarColorChange(int nColor)
{
	string strColor = "未知";
	switch (nColor)
	{
	case 0://白色
		strColor = "白色";
		break;
	case 1://银色
		strColor = "银色";
		break;
	case 2://黑色
		strColor = "黑色";
		break;
	case 3://红色
		strColor = "红色";
		break;
	case 4://紫色
		strColor = "紫色";
		break;
	case 5://蓝色
		strColor = "蓝色";
		break;
	case 6://黄色
		strColor = "黄色";
		break;
	case 7://绿色
		strColor = "绿色";
		break;
	case 8://褐色
		strColor = "褐色";
		break;
	case 9://粉红色
		strColor = "粉红色";
		break;
	case 10://灰色
		strColor = "灰色";
		break;
	case 11://未知
		strColor = "未知";
		break;
	}

	return strColor;
}

// 车辆品牌
string WuHanCenter::CarFactoryChange(UINT32 nLable)
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


//制作路径 
/*
string WuHanCenter::MakeRemoteImageName(MysqlQuery sqlData )
{
	// 时间
	string strTime = GetTime(sqlData.getIntFileds("TIME") );
	string strUMinTime = sqlData.getStringFileds("MITIME");
	strTime += strUMinTime;
	// 车牌号码
	string strCarCard = sqlData.getStringFileds("NUMBER");
	// 车牌颜色
	string strCardColor = CarColorChange(sqlData.getIntFileds("COLOR") );
	// 行驶速度
	string strSpeed =  sqlData.getStringFileds("SPEED");
	// 行驶方向
	string strDirection = DirectionChange(sqlData.getIntFileds("DIRECTION") );
	// 车道
	string strRoadId = sqlData.getStringFileds("ROAD");
	// 违章分类
	string strViolationType  = CarViolationTypeChange(sqlData.getIntFileds("PECCANCY_KIND"));
	// 车牌在图片的坐标
	string strCoordinate = sqlData.getStringFileds("POSLEFT") +","+ sqlData.getStringFileds("POSTOP")  \
		+","+ sqlData.getStringFileds("POSRIGHT") +","+ sqlData.getStringFileds("POSBOTTOM");
	// 车身的颜色
	string strCardColor = CarColorChange(sqlData.getIntFileds("CARCOLOR") );
	// 车辆品牌
	string strFactory = CarFactoryChange(sqlData.getIntFileds("FACTORY") );

	string strImageName = g_strDetectorID +"-"+ strTime +"-"+ strCarCard +"-"+ strCardColor +"-"+ strSpeed  \
		+"-"+ strDirection +"-"+ strRoadId +"-"+ strViolationType +"-"+   \
		strViolationType +"-"+ strCardColor +"-"+ strFactory +".jpg";
	return "";
}



// 处理和发送历史数据
int WuHanCenter::OnSendHistoryResult()
{
string strResultData = SqlDataTransitionRECORD_PLATE();
if (!strResultData.empty())
{
OnSendRealTimeResult(strResultData);
}

return 0;
}

*/

