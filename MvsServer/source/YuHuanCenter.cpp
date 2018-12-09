#ifdef YUHUANCENTER_OK

#include "YuHuanCenter.h"
#include "../soap/soHTSAPServerSoapBinding.nsmap"

#include <dirent.h>

YuhuanCenter g_YuHuanCenter;

YuhuanCenter::YuhuanCenter()
{
	m_nThreadRealTime = 0;
	m_nThreadHistory = 0;
	pthread_mutex_init(&m_Result_Mutex, NULL);

	m_isConnetdServer = 0;
}

YuhuanCenter::~YuhuanCenter()
{
	Uninit();
	pthread_mutex_destroy(&m_Result_Mutex);

}

/*
 *创建实时发送线程， 和历史发送线程
 */
int YuhuanCenter::Init()
{
	soap_init(&m_CDSoap);

	if(access("/home/dzjc",0) != 0) //目录不存在
	{
		LogNormal("增加dzjc 用户\n"); 
		system("useradd -m -s /bin/bash -p $(perl -e 'print crypt(\"dzjc\", \"aa\")') -N dzjc");
	}

	system("mkdir -p /home/road/dzjc");

	//    struct soap add_soap;
	pthread_t		id;
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	if (pthread_create(&m_nThreadRealTime, &attr, SendRealTimeData, this) != 0)
	{
		LogNormal("玉环中心端 创建实时发送线程失败\n");
		return -1;
	}

	if (pthread_create(&m_nThreadHistory, &attr, SendHistoryData, this) != 0)
	{
		LogNormal("玉环中心端， 创建历史发送线程失败");
	}

	if (pthread_create(&m_nThreadHistory, &attr, TimingClearPic, this) != 0)
	{
		LogNormal("玉环中心端， 创建删除发送线程失败");
	}

	//cerr<<"玉环 "<<GetDiskFree()<<"\n";
	return 0;
}

bool YuhuanCenter::Uninit()
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


	soap_destroy(&m_CDSoap);			//删除类实例（仅用于C++中）;
	soap_end(&m_CDSoap);              //清除运行环境变量;
	soap_done(&m_CDSoap);             //卸载运行环境变量;

}

/*
 * 增加一条消息未处理的消息
*/
bool YuhuanCenter::AddResult(const std::string& strResult)
{
	//cerr<<"玉环"<<__FUNCTION__<<"\n";
	pthread_mutex_lock(&m_Result_Mutex);
	m_listRecord.push_front(strResult);
	pthread_mutex_unlock(&m_Result_Mutex);
	return true;
}

int YuhuanCenter::InitGsoap()
{

	//cerr<<"玉环"<<__FUNCTION__<<"\n";
	if (m_isConnetdServer == 1)
	{
		return 0;
	}
	
	int index = 0;
	string strLogId;
	while(!g_bEndThread)
	{

		char szPort[10] = {0};
		sprintf(szPort, "%d", g_nFtpPort);
		string strPort(szPort);

		m_strServiceCode = "http://" + g_strFtpServerHost + ":" + strPort + "/HTServer/services/HTSAPServer";

		ns1__login user;
		user.strDeviceId = (char *)g_strFtpUserName.c_str();
		user.strDeviceKey = (char *)g_strFtpPassWord.c_str();
		user.soap = &m_CDSoap;

		ns1__loginResponse  longinResponse;

		//cerr<<" yuhuan 调用 longin \n";
		int nResult = soap_call___ns1__login(&m_CDSoap, m_strServiceCode.c_str(), NULL, &user, &longinResponse);
		if (nResult != SOAP_OK)
		{
			
			sleep(1);
			continue;
		}
		//cerr<<" yuhuan 结束调用 longin \n";
		string strstrLogId(longinResponse.return_);
		strLogId = strstrLogId;
		if (strstrLogId.empty())
		{
			if (index++ > 3)
			{
				return -1;
			}
		}
		if (strLogId.substr(0, 4) != "0000")
		{
			printf("连接失败。 ");
			continue;
		}

		sleep(3);
		break;
		m_isConnetdServer = 1;
	}

	m_strLogId = strLogId.substr(5, strLogId.size() -5);

	return 0;

}

// 发送实时数据的入口
void * YuhuanCenter::SendRealTimeData(void * pAg)
{
	YuhuanCenter * pThis = (YuhuanCenter *)pAg;

	while (!g_bEndThread)
	{
		pThis->DealResult();
		usleep(1000);
	}

	return NULL;
}

// 发送历史数据信息的入口
void * YuhuanCenter::SendHistoryData(void * pAg)
{
	YuhuanCenter * pThis = (YuhuanCenter *)pAg;
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

// 定时清除数据图片
void * YuhuanCenter::TimingClearPic(void * pAg)
{
	YuhuanCenter * pThis = (YuhuanCenter *)pAg;
	while(!g_bEndThread)
	{
		if (pThis->GetDiskFree() < 10)
		{
			pThis->DelOldPic();
		}
		sleep(5 * 60);
	}
}

string YuhuanCenter::SqlDataTransitionRECORD_PLATE()
{
	//cerr<<"玉环 数据库转换\n";
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

	//cerr<<strSql<<"\n";
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

	string strCarCard = sqlData.getStringFileds("NUMBER");
	memcpy(plate.chText, strCarCard.c_str(), strCarCard.size());		// 车牌号
	plate.uSpeed =sqlData.getIntFileds("SPEED");						// 车速
	plate.uDirection = sqlData.getIntFileds("DIRECTION");				// 行驶方向
	plate.uRoadWayID = sqlData.getIntFileds("ROAD");					// 车道号
	plate.uViolationType = sqlData.getIntFileds("PECCANCY_KIND");		// 违法代码
	plate.uPosLeft   = sqlData.getIntFileds("POSLEFT");              // 车牌在图片的坐标
	plate.uPosTop    = sqlData.getIntFileds("POSTOP");
	plate.uPosRight  = sqlData.getIntFileds("POSRIGHT");
	plate.uPosBottom = sqlData.getIntFileds("POSBOTTOM");
	plate.uCarColor1 = sqlData.getIntFileds("CARCOLOR");				// 车身颜色
	plate.uCarBrand	 = sqlData.getIntFileds("FACTORY");				// 车辆品牌
	string strPicPath = sqlData.getStringFileds("PICPATH");			// 车辆大图
	memcpy(plate.chPicPath, strPicPath.c_str(), strPicPath.size());
	plate.uColor = sqlData.getIntFileds("COLOR");
	plate.uTypeDetail  = sqlData.getIntFileds("TYPE_DETAIL");
	plate.uSignalTime =  sqlData.getIntFileds("REDLIGHT_TIME");

	string strResult;
	SRIP_DETECT_HEADER sripTemp;

	strResult.append((char *)&sripTemp, sizeof(SRIP_DETECT_HEADER));
	strResult.append((char *)&plate, sizeof(RECORD_PLATE));

	return strResult;
}


// 取出一条数据进行处理
void YuhuanCenter::DealResult(void)
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
		response = *it;					// 保存数据
		m_listRecord.pop_front();			// 删除取出的命令
	}
	pthread_mutex_unlock(&m_Result_Mutex);	// 解锁

	if(response.size() > 0)				// 处理消息
	{
		OnSendResult(response);
	}

	usleep(1000*1);
}

// 处理和发送数据
int YuhuanCenter::OnSendResult( string strResult)
{
	//cerr<<"玉环 处理和发送\n";
	string strResultData;
	strResultData.append((char *)strResult.c_str() + sizeof(SRIP_DETECT_HEADER),
		strResult.size() - sizeof(SRIP_DETECT_HEADER) );

	RECORD_PLATE *pPlate = (RECORD_PLATE *)(strResultData.c_str());

	DataChangeAndSend(pPlate);


	return 0;
}

int YuhuanCenter::DataChangeAndSend(RECORD_PLATE * pPlate)
{
	//cerr<<"玉环"<<__FUNCTION__<<"\n";
	// longin 的返回值
	string strSid;
	// 设备编号
	string strDeviceId = g_strDetectorID;
	// 车辆编号
	char szVihicle[30] = {0};
	sprintf(szVihicle, "%d", pPlate->uSeq);
	string strVehicleId(szVihicle);
	// 设备类型
	string strDeviceType = "02";	// 电子警察
	// 方向
	string strDirection = DirectionChange(pPlate->uDirection);
	// 车道编号
	char szRoadWayId[20] = {0};
	sprintf(szRoadWayId, "%d", pPlate->uRoadWayID);
	string strDriveWayId(szRoadWayId);

	// 车牌号码
	string strCarCard(pPlate->chText);
	if (*(strCarCard.c_str()) == '*' || *(strCarCard.c_str()) == '1' || *(strCarCard.c_str()) == '0')
	{
		strCarCard = "未识别";
	}
	// 号牌种类
	string strCardType = "";	// 可以为空
	// 经过时间
	string strPassDateTime = GetTime(pPlate->uTime, 0);
	// 红灯亮时间， 可以为空
	string strRedLightBeginTime = ""; //GetTime(pPlate->uTime - pPlate->uSignalTime, 0) ;	// 可以为空
	//红灯的持续时间， 可以为空
	string strRedLightLast = "";
	// 车辆速度
	long lSpeed = pPlate->uSpeed;
	// 大车限速
	long lLargeLimiteSpeed = 0;
	// 小车限速
	long lMiniLimitSpeed = 0;

	int limitSpeed = GetMaxSpeed(pPlate->uType, pPlate->uChannelID, pPlate->uRoadWayID);
	lLargeLimiteSpeed = lMiniLimitSpeed = limitSpeed;

	// 违章行为代码
	string strViolationType = "";	// 可以为空
	// 第二种违章行为代码
	string strViolationType2 = "";	// 可以为空
	// 车外廓长
	long lCarLengrh = 0;			// 可以为空
	// 号牌颜色
	char szCardColor[10] = {0};
	sprintf(szCardColor,  "%d", CardColorChange(pPlate->uColor));
	string strCardColor(szCardColor);		// 可以为空
	cerr<<strCardColor<<"\n";
	// 车辆类型
	string strCarType = GetCarType(pPlate->uTypeDetail);
	// 图片证据1的路径
	string strImageName = MakeImageName(pPlate);
	string strPicLocalPath1 = "/home/road/dzjc/" + strImageName;
	string strPicLocalPath2 = "";
	string strPicLocalPath3 = "";
	string strPicLocalPath4 = "";
	string strPicLocalPath5 = "";
	string strPicLocalPath6 = "";
	string strPicLocalPath7 = "";
	// ftp 路径
	string strRemotePath = "ftp://" + g_strFtpUserName + ":" + g_strFtpPassWord + "@" + g_ServerHost+ ":21";
	// 是否违章
	string strViolate = "1";
	if (pPlate->uViolationType == 0)
	{
		strViolate = "0";
	}
	//发送标志：1位字符, "0"代表正常，"1"代表滞后发送
	string strSendType = "0";

	if (InitGsoap() != 0)
	{
		m_isConnetdServer = 0;
		return -1;
	}



	ns1__writeVehicleInfo msgParma;
	msgParma.sid =				(char *)m_strLogId.c_str();
	msgParma.strDeviceId =			(char *)strDeviceId.c_str();
	msgParma.strVehicleId =		(char *)strVehicleId.c_str();
	msgParma.strDeviceType =		(char *)strDeviceType.c_str();
	msgParma.strDirectionId =		(char *)strDirection.c_str();
	msgParma.strDriveWayId =		(char *)strDriveWayId.c_str();
	msgParma.strLicense	=		(char *)strCarCard.c_str();
	msgParma.strLicenseType =		(char *)strCardType.c_str();
	msgParma.strPassDateTime =		(char *)strPassDateTime.c_str();
	msgParma.strRedLightBeginTime =	(char *)strRedLightBeginTime.c_str();
	msgParma.strRedLightLast =		(char *)strRedLightLast.c_str();
	msgParma.lSpeed =				lSpeed;
	msgParma.lLargeLimitSpeed =	lLargeLimiteSpeed;
	msgParma.lMiniLimitSpeed =		lMiniLimitSpeed;
	msgParma.strViolationType =	(char *)strViolationType.c_str();
	msgParma.strViolationType2 =	(char *)strViolationType2.c_str();
	msgParma.lCarLength =			lCarLengrh;
	msgParma.strLicenseColor =		(char *)strCardColor.c_str();
	msgParma.strCarType =			(char *)strCarType.c_str();
	msgParma.strPicLocalPath1 =	(char *)strPicLocalPath1.c_str();
	msgParma.strPicLocalPath2 =	(char *)strPicLocalPath2.c_str();
	msgParma.strPicLocalPath3 =	(char *)strPicLocalPath3.c_str();
	msgParma.strPicLocalPath4 =	(char *)strPicLocalPath4.c_str();
	msgParma.strPicLocalPath5 =	(char *)strPicLocalPath5.c_str();
	msgParma.strPicLocalPath6 =	(char *)strPicLocalPath6.c_str();
	msgParma.strPicLocalPath7 =	(char *)strPicLocalPath7.c_str();
	msgParma.strPicRemotePath =	(char *)strRemotePath.c_str();
	msgParma.strViolate =			(char *)strViolate.c_str();
	msgParma.strSendType =			(char *)strSendType.c_str();
	msgParma.soap =				&(m_CDSoap);

	ns1__writeVehicleInfoResponse Response;

	int nResult = soap_call___ns1__writeVehicleInfo(&m_CDSoap, m_strServiceCode.c_str(), "",	&msgParma, &Response);
	if (nResult != SOAP_OK)
	{
		m_isConnetdServer = 0;
		return -1;
	}

	string strResult(Response.return_);
	if (strResult.empty())
	{
		return -1;
	}
	if (strResult.substr(0, 4) != "0000")		// 是否发送成功
	{
		m_isConnetdServer = 0;
		return -1;
	}

	//cerr<<"玉环 开始copy\n";
	string strftppath = "/home/road/dzjc/" + strImageName;
	string strimageDate = GetImageByPath(pPlate->chPicPath);
	//cerr<<"大小："<<strimageDate.size()<<"\n";
	CpFile(pPlate->chPicPath, (char *)strftppath.c_str());
	cerr<<strftppath<<"\n";
	//cerr<<"玉环 copy 结束\n";


	char szSqlBuf [125] = {0};
		sprintf(szSqlBuf, "update NUMBER_PLATE_INFO set STATUS = 1 where ID = %u", pPlate->uSeq);
		string strSql(szSqlBuf);
		g_skpDB.execSQL(strSql);
	return 0;
}


// 制作保存的路径
string YuhuanCenter::MakeImageName(RECORD_PLATE * pPlate)
{
	string strTime = GetTime(pPlate->uTime, 0);

	string strYear	= strTime.substr(0,4);
	string strMonth	= strTime.substr(5,2);
	string strDay		= strTime.substr(8,2);
	string strHour	= strTime.substr(11,2);
	string strMin		= strTime.substr(14,2);
	string strSec		= strTime.substr(17,2);
	char  szRoadWayId[10] = {0};
	sprintf(szRoadWayId, "%02d%d", pPlate->uRoadWayID, GetRandCode());
	string strRoadWayId(szRoadWayId);

	string strImageame = strYear + strMonth + strDay + strHour + strMin + strSec + strRoadWayId + ".jpg";

	return strImageame;
}


// 行驶方向的转换
string YuhuanCenter::DirectionChange(int nDirection)
{
	string strDirection;

	switch(nDirection)
	{
	case 1:
		strDirection = "01";
		break;
	case 2:
		strDirection = "02";
		break;
	case 3:
		strDirection = "03";
		break;
	case 4:
		strDirection = "04";
		break;
	case 5:
		strDirection = "05";
		break;
	case 6:
		strDirection = "06";
		break;
	case 7:
		strDirection = "07";
		break;
	case 8:
		strDirection = "08";
		break;
	}

	return strDirection;
}

// 车牌颜色的转换
int  YuhuanCenter::CardColorChange(int nColor)
{
	int nCardColor;

	switch (nColor)
	{
	case 1://蓝色
		nCardColor = 2;
		break;
	case 2://黑色
		nCardColor = 3;
		break;
	case 3://黄色
		nCardColor = 1;
		break;
	case 4://白色
		nCardColor = 0;
		break;
	case 5://其他
		nCardColor = 4;
		break;
	default:
		nCardColor = 4;
		break;
	}
	return nCardColor;
}

//获取车辆类型
string YuhuanCenter::GetCarType(int nType)
{
	string strCarType;

	switch (nType)
	{
	case SMALL_CAR:		//小
	case TAXI:			//小型客车
	case MINI_TRUCK:   //小型货车 11
		strCarType = "0";
		break;
	case BIG_CAR:			//大
		strCarType = "1";
		break;
	case BUS_TYPE:		//大巴
		strCarType = "K";
		break;
	case TRUCK_TYPE://大货
		strCarType = "H11";
		break;
	case MIDDLEBUS_TYPE://中巴
		strCarType = "K";
		break;
	case OTHER_TYPE:  // 其他
		strCarType = "2";
		break;
	default:
		strCarType = "2";
			break;
	}
	return strCarType;
}

//获取随机数
int YuhuanCenter::GetRandCode()
{
	//srand( (unsigned)time( NULL ) );
	getpid();
	int nRandCode = 1 + (int)(9*1.0*rand()/(RAND_MAX+1.0));
	return nRandCode;
}



void YuhuanCenter::CpFile(char *spathname,char *tpathname)
{
	int sfd,tfd;
	struct stat s,t;
	char c;
	sfd=open(spathname,O_RDONLY);
	tfd=open(tpathname,O_RDWR|O_CREAT);
	while(read(sfd,&c,1)>0)
		write(tfd,&c,1);
	fstat(sfd,&s);
	chown(tpathname,s.st_uid,s.st_gid);
	chmod(tpathname,s.st_mode);

	close(sfd);
	close(tfd);
}


int YuhuanCenter::GetDiskFree()
{

	int have =g_sysInfo_ex.fTotalDisk  * (100 - g_sysInfo.fDisk)/100;

	return have;
}


void YuhuanCenter::DelOldPic()
{
	string strpath = "/home/road/dzjc/";
	string strImageName = GetFirstFileName(strpath);
	if (strImageName.empty())
	{
		return ;
	}

	string strdelImageName = strImageName.substr(0, 8);
	strdelImageName = "rm -f " + strpath + strdelImageName +"*";

	system(strdelImageName.c_str());

}

string YuhuanCenter::GetFirstFileName(string strPath)
{

	DIR * dir;
	struct dirent *ptr;
	dir = opendir(strPath.c_str());
	if(dir == NULL)
	{
		return "";
	}
	while( (ptr = readdir(dir)) != NULL)
	{

		if (strcmp (ptr->d_name, ".") == 0 || strcmp (ptr->d_name, "..") == 0)
			continue;
		char imageName[100] = {0};

		//strcmp(imageName, ptr->d_name);
		string strImageName(ptr->d_name);
		return strImageName;
	}

	return "";
}

#endif








/*
string strResult = soap_call___ns1__writeVehicleInfo(&m_CDSoap, m_strServiceCode.c_str(), "",	\
	m_strLogId, strDeviceId, strVehicleId, strDeviceType, strDirection, strDriveWayId,		\
	strCarCard, strCardType, strPassDateTime, strRedLightBeginTime, lSpeed, limitSpeed,		\
	limitSpeed, strViolationType, strViolationType2, lCarLengrh, strCardColor, strCarType,	\
	strPicLocalPath1, strPicLocalPath2, strPicLocalPath3, strPicLocalPath4,strPicLocalPath5,	\
	strPicLocalPath6, strPicLocalPath7, strRemotePath, strViolate, strSendType);*/
