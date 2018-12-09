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

#include "Common.h"
#include "CommonHeader.h"
#include "RoadMsgCenter.h"
#include "cv.h"
#include "highgui.h"
#include "XmlParaUtil.h"
#include "CenterServerOneDotEight.h"
#include "GpsChangeTimeSerial.h"
#include "DeviceStatus.h"
#include "Markup.h"

//#include "MysqlDB.h"
#define BUFF_SIZE 1024
//#define SEND_NOREALTIME_RECORD

//全局调用
CSkpRoadMsgCenter g_skpRoadMsgCenter;


//监控线程
void* ThreadProcessMsg(void* pArg)
{
	//取类指针
	CSkpRoadMsgCenter* pMsgCenter = (CSkpRoadMsgCenter*)pArg;
	if(pMsgCenter == NULL) return pArg;

	//
	while(!g_bEndThread)
	{

		//处理一条命令
		pMsgCenter->ProcessMsg();

		//0.1毫秒
		usleep(100);
	}
	pthread_exit((void *)0);
	return pArg;
}


//构造
CSkpRoadMsgCenter::CSkpRoadMsgCenter()
{
	//线程ID
	m_nThreadId = 0;
	pthread_mutex_init(&m_list_mutex,NULL);

	m_pManageClient = NULL;
	return;
}
//析构
CSkpRoadMsgCenter::~CSkpRoadMsgCenter()
{
	pthread_mutex_destroy(&m_list_mutex);
	return;
}

//初始化
bool CSkpRoadMsgCenter::Init()
{
	//启动处理线程
	if(!BeginProcessThread())
		return false;
	return true;
}
//释放
bool CSkpRoadMsgCenter::UnInit()
{
	//停止线程
	EndProcessThread();
	return true;
}

//添加一条命令
bool CSkpRoadMsgCenter::AddMsg(const int nSocket,const UINT32 uCommand,const std::string request)
{
    //printf(" nSocket = %d,uCommand = %x ,request.size()=%d\r\n",nSocket,uCommand,request.size());
	//判断命令类型，列表外处理部分紧急的命令
	switch(uCommand)
	{
		case SRIP_LOGIN:		//客户端登录
			return OnLogin(nSocket, uCommand,request);
		case PLATE_LINK:			//心跳
		case EVENT_LINK:
			return OnLink(nSocket, request);
		default:
			break;
	}

    // Other msgs will be put into queue.
	pthread_mutex_lock(&m_list_mutex);
	m_mapMsgList.insert(make_pair(nSocket, request));
	pthread_mutex_unlock(&m_list_mutex);


	return true;
}

//删除命令
bool CSkpRoadMsgCenter::DelMsg(int nSocket)
{
	//加锁
	pthread_mutex_lock(&m_list_mutex);

	if (!m_mapMsgList.empty())
	{
        //删除列表
        m_mapMsgList.erase(nSocket);
	}
	//解锁
	pthread_mutex_unlock(&m_list_mutex);

	return true;
}


//处理一条命令
bool CSkpRoadMsgCenter::ProcessMsg()
{
	//弹出一条命令
	int nSocket = 0;
	std::string response("");

	PopMsg(nSocket,response);
	if (!response.empty())
    {
        OnMsg(nSocket,response);
    }

    //zhangyaoyao(2010-02-25)
    if (1 == g_nHasCenterServer)
    {
        if(g_nServerType == 1|| g_AmsHostInfo.uHasAmsHost > 0)
        {
            string strMsg("");
            UINT32 uCmdID;
            if (g_AMSCommunication.mvPopOneMsg(uCmdID, strMsg))
            {
                g_AMSCommunication.mvOnDealOneMsg(uCmdID, strMsg);
            }
        }
        else if(g_nServerType == 2)
        {
            string strCode(""), strMsg("");
            if (g_CenterServer.mvPopOneMsg(strCode, strMsg))
            {
                g_CenterServer.mvOnDealOneMsg(strCode.c_str(), strMsg);
            }
        }
		else if(g_nServerType == 13)
		{
			string strCode(""), strMsg("");
			if (g_MyCenterServer.mvPopOneMsg(strCode, strMsg))
			{
				g_MyCenterServer.mvOnDealOneMsg(strCode.c_str(), strMsg);
			}
		}
        else if ( 0 == g_nServerType || 4 == g_nServerType || 6 == g_nServerType || 26 == g_nServerType)
        {
			g_BocomServerManage.mvOnDealOneMsg();
        }
    }

	return true;
}

//弹出消息
bool CSkpRoadMsgCenter::PopMsg(int& nSocket,std::string& response)
{
	//std::string response("");
	//加锁
	pthread_mutex_lock(&m_list_mutex);
	//判断是否有命令
	if (!m_mapMsgList.empty())
    {
        MsgListMap::iterator it = m_mapMsgList.begin(); //we can't fetch a message out by order of the time it was inserted in.
        //保存数据
        nSocket = it->first;
        response = it->second;
        m_mapMsgList.erase(it);
    }
	//解锁
	pthread_mutex_unlock(&m_list_mutex);

	return true;
}

bool CSkpRoadMsgCenter::OnMsg(int nSocket,std::string response)
{
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)response.c_str();

	printf("command:%x \n",sHeader->uMsgCommandID);
	//处理命令
	switch((sHeader->uMsgCommandID))
	{
		
		case SRIP_USER_ADD:				//添加用户
			return OnUserAdd(nSocket,response);
		case SRIP_USER_DEL:				//删除用户
			return OnUserDel(nSocket,response);
		case SRIP_USER_MOD:				//修改用户
			return OnUserModify(nSocket,response);
		case SRIP_USER_LIST:			//用户列表
			return OnUserList(nSocket,response);

		case SRIP_CHANNEL_ADD:			//添加通道
			return OnChannelAdd(nSocket,response);
		case SRIP_CHANNEL_MOD:			//修改通道
			return OnChannelModify(nSocket,response);
		case SRIP_CHANNEL_DEL:			//删除通道
			return OnChannelDel(nSocket,response);
		case SRIP_CHANNEL_PAUSE:        //暂停通道
			return OnChannelPause(nSocket,response);
		case SRIP_CHANNEL_RESUME:       // 激活通道
			return OnChannelResume(nSocket,response);

		case SRIP_CHANNEL_PARA_ADJUST:	//调整视频参数
			return OnAdjustChannelPara(nSocket,response);
		case SRIP_CHANNEL_PARA_GET:  //获取视频参数
            return OnGetAdjustPara(nSocket,response);

		case SRIP_CHANNEL_PARA_SAVE:	//保存视频参数
			return OnSaveChannelPara(nSocket,response);

		case SRIP_ROAD_WAY:				//获得车道坐标数据
			return OnRoadWay(nSocket,response);
        case ROAD_SETTING_MODEL:        //获得车道坐标模板
            return OnRoadWayModel(nSocket,response);
		case ROAD_PARAMETER:           //获取车道参数
			return OnRoadWayParaMeter(nSocket,response);
		case SRIP_ROAD_SAVE:			//车道设置
			return OnRoadSave(nSocket,response);
		case ROAD_PARAMETER_SAVE:		//保存车道参数
			return OnRoadParaMeterSave(nSocket,response);
		case ROAD_MODEL_SAVE:		//保存车道模板参数
			return OnRoadModelSave(nSocket,response);
		case SRIP_ROAD_DEL:				//清空车道
			return OnRoadDel(nSocket,response);

		case SRIP_SYS_SETTING:			//系统配置
			return OnSysSetting(nSocket,response);
		case SRIP_BACKUP_DATABASE:		//备份数据库
			return OnBackupDB(nSocket,response);
		case DB_REPAIR:		//数据库修复
			return OnDBRepair(nSocket,response);
		case DB_DEFAULT:		//数据库恢复
			return OnDBDefault(nSocket,response);

		case SRIP_CHARTQUERY:			//事件统计图表数据
			return OnChartQuery(nSocket,response);
		case SRIP_CARD_PIC:				//车牌图片查询
			return OnCardPic(nSocket,response);
		case SRIP_SEARCH_RECORD:		//录像查询
			return OnSearchRecord(nSocket,response);


        case SRIP_ADD_SPECIALCARD:  //车牌黑白名单
			return  OnAddSpecialCard(nSocket,response);
		case SRIP_DELETE_SPECIALCARD:
			return  OnDeleteSpecialCard(nSocket,response);
		case SRIP_MODIFY_SPECIALCARD:
			return  OnModifySpecialCard(nSocket,response);
		case SRIP_SEARCH_SPECIALCARD:
			return  OnSearchSpecialCard(nSocket,response);

		case SRIP_CARNUMPARAM:     //获取车牌检测参数
			return  OnCardNumParam(nSocket,response);
		case SRIP_CARNUMPARAM_SAVE:    //保存车牌检测参数
			return  OnSaveCardNumParam(nSocket,response);

        case MODIFY_CARNUM://修改车牌
			return OnModifyCarNum(nSocket,response);


        case VTS_PARAMETER:     //获取违章检测参数
			return  OnVTSParam(nSocket,response);
		case VTS_PARAMETER_SAVE:    //保存违章检测参数
			return  OnSaveVTSParam(nSocket,response);

        case LOOP_PARAMETER:     //获取线圈检测参数
			return  OnLoopParam(nSocket,response);
		case LOOP_PARAMETER_SAVE:    //保存线圈检测参数
			return  OnSaveLoopParam(nSocket,response);
        case RADAR_PARAMETER_SET: //雷达参数设置
            return OnRadarParameterSet(nSocket,response);
        case OBJECT_PARAMETER_SET: //目标检测参数设置
            return OnObjectParameterSet(nSocket,response);
        case BEHAVIOR_PARAMETER_SET: //行为分析参数设置
            return OnBehaviorParameterSet(nSocket,response);

		case SRIP_CAMERA_SETUP://相机设置
		case SRIP_CAMERA_CONTROL:	//调整镜头
			return  OnCameraSetup(nSocket,response);

        case PRESET_INFO: //获取预置位信息
            return OnPreSetInfo(nSocket,response);
        case ADD_PRESET_INFO: //增加预置位
        case MOD_PRESET_INFO://修改预置位
            return OnUpdatePreSetInfo(nSocket,response);
        case DEL_PRESET_INFO: //删除预置位
            return OnDeletePreSetInfo(nSocket,response);

		case VIDEO_STATE://录象是否已经完成
			return OnVideoState(nSocket,response);

		case SRIP_CHANNEL_BIG://截取大图
			return OnCaptureOneFrame(nSocket,response);

		case SRIP_SEARCH_CARD_HIGH://车牌高级查询--add by ywx--2009-08-31
			return OnSearchPlateHigh(nSocket, response);

        case SRIP_SEARCH_TEXTURE_HIGH://特征搜索高级查询获取特征信息--add by ywx--2009-09-11
            return OnSearchTextureHigh(nSocket, response);
        case SRIP_SEARCH_OBJECT_FEATURE:
            return OnSearchObjectFeature(nSocket, response);

        case SRIP_CHANNEL_LIST: //更新通道结构--add by ywx-2009-10-16
            return OnGetChannelList(nSocket, response);

        case SRIP_REGION_IMAGE://显示区域图像
           return OnRegionImage(nSocket,response);

        case ADD_CAMERA_MODEL:  //增加相机模板
        case MODIFY_CAMERA_MODEL:
        case LOAD_CAMERA_MODEL:
            return OnCameraModel(nSocket,response);

        case SET_IP:            //设置ip地址
            return OnIpSetup(nSocket,response);
        case DELETE_TEST:       //删除测试程序
            return OnDeleteTest(nSocket,response);
        case SYSCLOCK:          //时钟同步设置
            return OnSysClockSetup(nSocket,response);
        case AUTHENTICATION_SET:  //认证服务器设置返回
            return OnAuthenticationSetup(nSocket,response);
        case CONTROL_SERVER_SET: //中心控制服务器设置
            return OnControlServerSetup(nSocket,response);
        case FTP_SERVER_SET://ftp服务器设置
            return OnFtpServerSetup(nSocket,response);
        case MATCH_HOST_SET://比对服务器设置
            return OnMatchHostSetup(nSocket,response);
		case AMS_HOST_SET://应用管理服务器设置
			return OnAmsHostSetup(nSocket,response);
		case DISTANCE_HOST_SET://区间测速主机设置
			return OnDistanceHostSetup(nSocket,response);
		case GPS_SET://GPS设置
			return OnGpsSetup(nSocket,response);
		case SIGNAL_SET://信号机设置
			return OnSignalSetup(nSocket,response);
		case CHANNEL_PICFORMAT_SET://通道图片格式设置
			return OnPicFormatSetup(nSocket,response);
        case SYSTEM_HARDWARE_INFO://获取系统硬件配置信息
            return OnSysHwInfo(nSocket,response);
        case DETECTOR_ID_SET: //检测器ID设置
            return OnDetectorIDSetup(nSocket,response);
        case DETECTOR_RESET://检测器复位
            return OnDetectorReset(nSocket,response);
        case DETECTOR_SHUTDOWN://检测器关机
            return OnDetectorShutDown(nSocket,response);
        case SOFTWARE_RESET://软件复位
            return OnSoftWareReset(nSocket,response);
        case FORCE_RED_LIGHT://强制红灯
            return OnForceRedLight(nSocket,response);
        case SYS_COM_SETTING: //串口设置
            return OnSysComSetting(nSocket,response);
        case SYS_YUNTAI_SETTING: //云台参数设置
            return OnSysYunTaiSetting(nSocket,response);
        case SYS_MONITORHOST_SETTING: //监控主机设置参数设置
            return OnSysMonitorHostSetting(nSocket,response);
        case EXPO_MONITOR_SETTING://智能控制器参数
            return OnExpoMonitorSetting(nSocket,response);
        case SYS_SETTING_MODEL://系统参数模板设置
            return OnSysModelSetting(nSocket,response);
        case PIC_FORMAT_SETTING://图片格式信息
            return OnPicFormatSetting(nSocket,response);
        case VIDEO_FORMAT_SETTING://录像格式信息
            return OnVideoFormatSetting(nSocket,response);
        case SRIP_VERSION://获取版本号
            return OnVerSion(nSocket,response);
        //设置区域
        case REGION_SET:
        case REGION_GET:
            return OnRegionInfo(nSocket,response);
        //开光灯控制
        case LIGHT_TIME_CONTROL:
            return OnLightTimeControl(nSocket,response);
		case SRIP_ADD_FORCE_ALERT:      //强制报警
			return OnForceAlert(nSocket,response);
		case SRIP_GET_DSP_LIST:		//获取相机列表
			return OnGetDspList(nSocket,response);
		////////////////////////////////////////////////////////////////////////////////////以下是博康命令处理函数
		case MIMAX_PLATE:
		case MIMAX_SEQ_PLATE:
		case MIMAX_NONE_PLATE:
		case SRIP_SEARCH_CARD:			//车牌查询
			return OnSearchPlate(nSocket,response);
		case MIMAX_EVENT:
		case MIMAX_SEQ_EVENT:
		case MIMAX_NONE_EVENT:
		case SRIP_SEARCH_EVENT:         //事件查询
			return OnSearchEvent(nSocket,response);
		case MIMAX_STATISTIC:
		case MIMAX_SEQ_STATISTIC:
		case MIMAX_NONE_STATISTIC:
		case SRIP_SEARCH_ALARM:        //统计查询
			return OnSearchStatistic(nSocket,response);
		case PLATE_LOG:
		case EVENT_LOG:
		case PLATE_SEQ_LOG:
		case EVENT_SEQ_LOG:
		case PLATE_NONE_LOG:
		case EVENT_NONE_LOG:
		case SRIP_SEARCH_LOG:			//日志查询
			return OnSearchLog(nSocket,response);
		case MIMAX_DELSEQ_STATISTIC:
		case MIMAX_DELTIME_STATISTIC:
		case MIMAX_DELSEQ_EVENT:
		case MIMAX_DELTIME_EVENT:
		case MIMAX_DELSEQ_PLATE:
		case MIMAX_DELTIME_PLATE:
		case PLATE_DELSEQ_LOG:
		case EVENT_DELSEQ_LOG:
		case PLATE_DEL_LOG:
		case EVENT_DEL_LOG:
		case SRIP_DELETE_RECORD:        //删除各种查询结果
			return OnDeleteResult(nSocket,response);
		case MIMAX_CHANNEL_CONNECT:
		case MIMAX_CHANNEL_DISCONNECT:  //连接还是断开通道
		case MIMAX_SEND_FRAME:
		case MIMAX_NOSEND_FRAME:  //是否推送实时视频
			return OnSetChannelConnect(nSocket,response);
        case SYSTIME_SET:
		case PLATE_SYSTIME:
		case EVENT_SYSTIME:
		case PLATE_SYSTIME_SETUP:
		case EVENT_SYSTIME_SETUP:		//系统时间
			return OnSysTime(nSocket,response);
		case SETTING_UPLOAD:			//配置信息手动上传
			return OnSettingUpload(nSocket, response);
		case UPLOAD_VIDEO:			//录像手动上传
			return OnVideoUpload(nSocket, response);
		case DETECT_REGION_RECT:
			return OnDetectRegionRectImage(nSocket, response);
		case DETECT_ADD_REGION_RECT:
		case DETECT_DEL_REGION_RECT:
			return OnDetectParkObjectsRect(nSocket,sHeader->uMsgCommandID, response);
		case READ_3G_MODE_REQ:
			return OnRead3GMode(nSocket, response);
		case SET_3G_MODE:
			return OnSet3GMode(nSocket, response);
		case SET_DSP_CAMERA_REQ:
			return OnSetDspCamera(nSocket, response);
		case GET_DSP_CAMERA_REQ:
			return OnGetDspCamera(nSocket, response);
		case SET_FTP_PATH_REQ:
			return Set_Ftp_Path(nSocket, response);
		case GET_FTP_PATH_REQ:
			return Get_Ftp_Path(nSocket, response);
		case SET_NTP_TIME_REQ:
			return Set_Ntp_Time(nSocket, response);
		case GET_NTP_TIME_REQ:
			return Get_Ntp_Time(nSocket, response);
		case SET_CHECK_TIME_REQ:
			return SetCheckTime(nSocket, response);
		case GET_CHECK_TIME_REQ:
			return GetCheckTime(nSocket, response);
		case SET_SERVER_KAFKA_REQ:
			return SetKafka(nSocket, response);
		case GET_SERVER_KAFKA_REQ:
			return GetKafka(nSocket, response);
		//尾号限行
		case SET_PLATE_LIMIT_REQ:
			return SetPlateLimit(nSocket, response);
		case GET_PLATE_LIMIT_REQ:
			return GetPlateLimit(nSocket, response);
		//
		case SETTING_ROADITUDE:
			return SetRoadInfo(nSocket,response);
		case GET_ROADITUDE:
			return GetRoadInfo(nSocket,response);
		default:						//错误命令
			LogError("连接[%d]接收到错误的命令[%d]!\r\n",nSocket,(sHeader->uMsgCommandID));
			break;
	}
	return true;
}

// 设置DSP相机配置信息
bool CSkpRoadMsgCenter::OnSetDspCamera(const int nSocket, string request)
{
	CMarkup xml;
	xml.SetDoc("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	xml.AddElem("DspCameras");
	xml.IntoElem();
	FILE* fp = NULL;

	int nSize = request.size() - sizeof(SRIP_HEADER);
	int nStart = sizeof(SRIP_HEADER);
	while (nSize > 0)
	{
		xml.AddElem("Item");
		xml.IntoElem();
		
		SET_DSP_CAMERA setDspCamera;
		memcpy(&setDspCamera,request.c_str()+nStart,sizeof(SET_DSP_CAMERA));
		xml.AddElem("CameraIP",setDspCamera.chDspIp);
		xml.AddElem("CameraPort",setDspCamera.nDspPort);
		xml.AddElem("TcpPort",setDspCamera.nTcpPort);
		xml.OutOfElem();

		nStart += sizeof(SET_DSP_CAMERA);
		nSize -= sizeof(SET_DSP_CAMERA);
	}
	xml.OutOfElem();
	xml.Save("DspCamera.xml");

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SET_DSP_CAMERA_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (SRIP_OK);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送OnSetDspCamera信息返回数据包失败[%d]！\r\n",nSocket);
	}
	else
	{
		LogNormal("发送OnSetDspCamera信息成功！[%d]\n",response.size());
	}
}

bool CSkpRoadMsgCenter::OnGetDspCamera(const int nSocket, string request)
{
	CMarkup xml;
	xml.Load("DspCamera.xml");
	xml.ResetMainPos();

	int nCnt = 0;
	std::string responseXml;
	if (xml.FindElem("DspCameras"))
	{
		xml.IntoElem();
		while (xml.FindElem("Item"))
		{
			SET_DSP_CAMERA dspCamera;
			char strDspIp[1024] ={0};
			int nDspPort = 0;
			int nTcpPort = 0;

			if (xml.FindChildElem("CameraIP"))
			{
				sprintf(dspCamera.chDspIp,"%s",xml.GetChildData().c_str());
			}

			if (xml.FindChildElem("CameraPort"))
			{
				dspCamera.nDspPort = atoi(xml.GetChildData().c_str());
			}

			if (xml.FindChildElem("TcpPort"))
			{
				dspCamera.nTcpPort = atoi(xml.GetChildData().c_str());
			}
			nCnt++;
			responseXml.append((char*)&dspCamera, sizeof(SET_DSP_CAMERA));
		}
	}

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (GET_DSP_CAMERA_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(SET_DSP_CAMERA)*nCnt;
	//返回代码
	sHeader->uMsgCode = (SRIP_OK);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if (nCnt > 0)
	{
		response.append(responseXml.c_str(), sizeof(SET_DSP_CAMERA)*nCnt);
	}

	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送GET_DSP_CAMERA信息返回数据包失败[%d]！\r\n",nSocket);
	}
	else
	{
		LogNormal("发送GET_DSP_CAMERA信息成功！[%d]\n",response.size());
	}
}

bool CSkpRoadMsgCenter::OnRead3GMode(const int nSocket, string request)
{
	if(access("/etc/ppp/peers/userinfo.txt",F_OK) == 0)
	{
		bool bRet = OnRead3GModeByOpenUserInfo(nSocket, request);
		if (bRet == true)
		{
			return true;
		}
	}

	SET_3G_INFO set3GInfo;
	UINT32 uErrorCode = SRIP_OK;

	char strFileName[256] = {"/etc/ppp/peers/wcdma"};
	char strConfig[1024] = {0};
	char strUser[256] = {0};
	char strPwd[256] = {0};
	char strApn[256] = {0};

	memset(&set3GInfo, 0x00, sizeof(SET_3G_INFO));

	SET_3G_INFO_EX set3GInfoEx;
	set3GInfoEx.nExist3G = g_nExist3G;
	set3GInfoEx.n3GType = g_n3GTYPE;
	sprintf(set3GInfoEx.chIp, "%s", g_str3GIp.c_str());
	

	FILE* fp = NULL;
	fp = fopen(strFileName, "r");
	if(fp!=NULL)
	{
		fread(strConfig, 1, 1024, fp);
		string strSet = strConfig;
		int nPos1 = strSet.find("user");
		if (g_nServerType == 13)// 上海交警3G
		{
			sprintf(set3GInfo.chUserName, "%s", strSet.substr(nPos1+5,8).c_str());
			sprintf(set3GInfo.chApn, "jjzd.shapn");
		}
		else if (g_nServerType == 23)// 济南交警3G
		{
			sprintf(set3GInfo.chUserName, "%s", strSet.substr(nPos1+5,20).c_str());
			sprintf(set3GInfo.chApn, "jnjj.sd");
		}

		int nPos2 = strSet.find("password");
		sprintf(set3GInfo.chPass, "%s", strSet.substr(nPos2 +9 ,8).c_str());
		fclose(fp);
	}
	else
	{
		LogNormal("读取%s失败.\n",strFileName);
		uErrorCode = READ_3G_INFO;
	}
	//if(strlen(set3GInfo.chUserName) > 0 && strlen(set3GInfo.chPass) > 0 && strlen(set3GInfo.chApn) > 0 )
	{
		//取头
		SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

		//设置参数
		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (READ_3G_MODE_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(SET_3G_INFO)+sizeof(SET_3G_INFO_EX);
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		std::string response;
		response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
		response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
		response.append((char*)&set3GInfo, sizeof(SET_3G_INFO));
		response.append((char*)&set3GInfoEx, sizeof(SET_3G_INFO_EX));
		if(!g_skpRoadServer.SendMsg(nSocket,response))
		{
			LogError("发送SET_3G_INFO信息返回数据包失败[%d]！\r\n",nSocket);
		}
		else
		{
			LogNormal("发送SET_3G_INFO信息成功！[%d]\n",response.size());
		}
	}
	return true;
}

bool CSkpRoadMsgCenter::OnRead3GModeByOpenUserInfo(const int nSocket, string request)
{
	SET_3G_INFO set3GInfo;
	UINT32 uErrorCode = SRIP_OK;

	char strFileName[256] = {"/etc/ppp/peers/userinfo.txt"};
	char strConfig[1024] = {0};
	char strUser[256] = {0};
	char strPwd[256] = {0};
	char strApn[256] = {0};

	memset(&set3GInfo, 0x00, sizeof(SET_3G_INFO));

	SET_3G_INFO_EX set3GInfoEx;
	set3GInfoEx.nExist3G = g_nExist3G;
	set3GInfoEx.n3GType = g_n3GTYPE;
	sprintf(set3GInfoEx.chIp, "%s", g_str3GIp.c_str());
	
	FILE* fp = NULL;
	fp = fopen(strFileName, "r");
	if(fp!=NULL)
	{
		fread(strConfig, 1, 1024, fp);
		string strSet = strConfig;
		int nPos1 = -1;
		int nPos2 = -1;
		nPos1 = strSet.find("User:Bocom");
		printf("Pos1:%d\n",nPos1);
		if (nPos1 >= 0)
		{
			nPos1 += 10;
			nPos2 = strSet.find("Bocom",nPos1);
			sprintf(set3GInfo.chUserName, "%s", strSet.substr(nPos1,nPos2 - nPos1).c_str());
		}

		nPos1 = strSet.find("Pass:Bocom");
		if (nPos1 >= 0)
		{
			nPos1 += 10;
			nPos2 = strSet.find("Bocom",nPos1);
			sprintf(set3GInfo.chPass, "%s", strSet.substr(nPos1,nPos2 - nPos1).c_str());
		}

		nPos1 = strSet.find("VPN:Bocom");
		if (nPos1 >= 0)
		{
			nPos1 += 9;
			nPos2 = strSet.find("Bocom",nPos1);
			sprintf(set3GInfo.chApn, "%s", strSet.substr(nPos1,nPos2 - nPos1).c_str());
		}
		fclose(fp);
	}
	else
	{
		LogNormal("读取%s失败.\n",strFileName);
		uErrorCode = READ_3G_INFO;
	}
	//if(strlen(set3GInfo.chUserName) > 0 && strlen(set3GInfo.chPass) > 0 && strlen(set3GInfo.chApn) > 0 )
	{
		//取头
		SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

		//设置参数
		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (READ_3G_MODE_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(SET_3G_INFO)+sizeof(SET_3G_INFO_EX);
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		std::string response;
		response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
		response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
		response.append((char*)&set3GInfo, sizeof(SET_3G_INFO));
		response.append((char*)&set3GInfoEx, sizeof(SET_3G_INFO_EX));
		if(!g_skpRoadServer.SendMsg(nSocket,response))
		{
			LogError("发送SET_3G_INFO信息返回数据包失败[%d]！\r\n",nSocket);
		}
		else
		{
			LogNormal("发送SET_3G_INFO信息成功！[%d]\n",response.size());
		}
	}
	return true;
}

// 配置3G信息
bool CSkpRoadMsgCenter::OnSet3GMode(const int nSocket, string request)
{
	if(access("/etc/ppp/Open3G.sh",F_OK) == 0)
	{
		system("mv /etc/ppp/Open3G.sh /etc/ppp/Open3G.shBAK");
	}
	
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	char strFileName[256] = {"/etc/ppp/peers/wcdma"};
	char strConnectFileName[256] = {"/etc/ppp/peers/chat-wcdma-connect"};
	char strUserInfoFileName[256] = {"/etc/ppp/peers/userinfo.txt"};

	char strConfig[1024] = {0};
	char strConnectConfig[1024] = {0};
	char strUserInfoConfig[1024] = {0};
	FILE* fp = NULL;

	SET_3G_INFO set3g_info;
	memcpy(&set3g_info,request.c_str()+sizeof(SRIP_HEADER),sizeof(SET_3G_INFO));


	if(request.size() == sizeof(SRIP_HEADER)+sizeof(SET_3G_INFO)+sizeof(SET_3G_INFO_EX))
	{
		SET_3G_INFO_EX set3gInfo = *((SET_3G_INFO_EX*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(SET_3G_INFO)));
		g_nExist3G = set3gInfo.nExist3G;

		g_str3GIp = set3gInfo.chIp;
		g_n3GTYPE = set3gInfo.n3GType;

		 CXmlParaUtil xml;
		 xml.UpdateSystemSetting("3GInfo","3G");
		 xml.UpdateSystemSetting("3GInfo","3GIP");
		 xml.UpdateSystemSetting("3GInfo","3GTYPE");
	}
	
	char strPppFileName[256] = {"./shell/3Gstatus.sh"};
	//if(access(strPppFileName,F_OK) != 0)
	{
		char   buf[512]={0};
		if(g_nExist3G != 1)
		{
			sprintf(buf,"#!/bin/bash\nsed -i /chk3Gstatus/d /etc/crontab\nsed -i /chk3Gstatus/d /etc/rc.local\n\nsed -i /chkppp/d /etc/crontab\nsed -i /chkppp/d /etc/rc.local\nrm /etc/rc2.d/S99ping\n");
		}
		else
		{
			sprintf(buf,"#!/bin/bash\nchmod 777 /etc/chk3Gstatus.sh\nsed -i /chk3Gstatus/d /etc/crontab\nsed '$ i*/2 *  * * *   root    /etc/chk3Gstatus.sh > /tmp/3Gppp.log 2>&1' -i /etc/crontab\nsed -i /chk3Gstatus/d /etc/rc.local\nsed '$ i/etc/chk3Gstatus.sh > /tmp/3Gppprc.log' -i /etc/rc.local\nln -s /etc/ping.sh /etc/rc2.d/S99ping\nsed -i /chkppp/d /etc/crontab\nsed -i /chkppp/d /etc/rc.local\n");
		}


		FILE* fp = fopen(strPppFileName,"wb");
		if(fp)
		{
			fwrite(buf,1,512,fp);
			fclose(fp);
		}
		sprintf(buf, "chmod 777 %s", strPppFileName);
		system(buf);
	}

	if(access(strPppFileName,F_OK) == 0)
	{
		char   cmdline[256]={0};
		sprintf(cmdline,"%s",strPppFileName);
		system(cmdline);
		sync();
	}

	if (g_nServerType == 13)//华为模块
	{
		sprintf(strConfig,"debug\n\nnodetach\n\n/dev/ttyUSB0\n\n115200\n\nusepeerdns\n\nnoauth\n\nnoipdefault\n\n" \
			"novj\n\nnovjccomp\n\nnoccp\n\ndefaultroute\n\nuser %s\n\npassword %s\n\n0.0.0.0:0.0.0.0\n\n" \
			"ipcp-accept-local\n\nipcp-accept-remote\n\nconnect '/usr/sbin/chat -s -v -f /etc/ppp/peers/chat-wcdma-connect'\n\n" \
			"disconnect '/usr/sbin/chat -s -v -f /etc/ppp/peers/chat-wcdma-disconnect'\n",set3g_info.chUserName, set3g_info.chPass);
	}
	else//龙尚模块
	{
		sprintf(strConfig,"debug\n\nnodetach\n\n/dev/ttyUSB2\n\n115200\n\nusepeerdns\n\nnoauth\n\nnoipdefault\n\n" \
			"novj\n\nnovjccomp\n\nnoccp\n\ndefaultroute\n\nuser %s\n\npassword %s\n\n0.0.0.0:0.0.0.0\n\n" \
			"ipcp-accept-local\n\nipcp-accept-remote\n\nconnect '/usr/sbin/chat -s -v -f /etc/ppp/peers/chat-wcdma-connect'\n\n" \
			"disconnect '/usr/sbin/chat -s -v -f /etc/ppp/peers/chat-wcdma-disconnect'\n",set3g_info.chUserName, set3g_info.chPass);
	}
	fp = fopen(strFileName, "wb");
	if(fp!=NULL)
	{
		fwrite(strConfig, strlen(strConfig), 1, fp);
		fflush(fp);
		fclose(fp);
	}
	
	if (g_nServerType == 13)
	{
		sprintf(strConnectConfig,"TIMEOUT 50\n\nABORT 'NO CARRIER'\n\nABORT 'ERROR'\n\nABORT 'NODIALTONE'\n\nABORT 'BUSY'\n\n" \
			"ABORT 'NO ANSWER'\n\n'' \\rAT\n\nOK \\rATZ\n\nOK \\rATI\n\nOK \\rAT+CGMI\n\nOK \\rAT+CGSN\n\nOK \\rAT+CSQ\n\nOK \\rAT+CGDCONT=1,\"IP\",\"%s\",,0,0\n\n" \
			"OK-AT-OK ATDT*99#\n\nCONNECT \d\c\n",set3g_info.chApn);
	}
	else
	{
		sprintf(strConnectConfig,"TIMEOUT 15\nABORT \"DELAYED\"\nABORT \"BUSY\"\nABORT \"ERROR\"\nABORT \"NO DIALTONE\"\n" \
			"ABORT \"NO CARRIER\"\nTIMEOUT 40\n'' AT\nOK ATS0=0\nOK ATE0V1\nOK AT$QCPDPP=1,1,\"%s\",\"%s\"\nOK AT+CGDCONT=1,\"IP\",\"%s\"\n" \
			"OK ATDT*99***1#\nCONNECT ''\n",set3g_info.chUserName, set3g_info.chPass,set3g_info.chApn);
	}
	fp = fopen(strConnectFileName, "wb");
	if(fp!=NULL)
	{
		fwrite(strConnectConfig, strlen(strConnectConfig), 1, fp);
		fflush(fp);
		fclose(fp);
	}

	// 写用户信息，用于读取
	sprintf(strUserInfoConfig,"User:Bocom%sBocom\nPass:Bocom%sBocom\nVPN:Bocom%sBocom\n",set3g_info.chUserName, set3g_info.chPass,set3g_info.chApn);
	fp = fopen(strUserInfoFileName, "wb");
	if(fp!=NULL)
	{
		fwrite(strUserInfoConfig, strlen(strUserInfoConfig), 1, fp);
		fflush(fp);
		fclose(fp);
	}

	//返回验证结果
	std::string response;
	MIMAX_HEADER mHeader;
	//命令
	mHeader.uCmdID = (SET_3G_MODE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
	sHeader->uMsgCode = (SRIP_OK);

	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送3G配置设置回复数据包失败[%d]！\r\n",nSocket);
		return false;
	}
}

// 获取配置3G信息
bool CSkpRoadMsgCenter::OnRead3GModeByOpen3G(const int nSocket, string request)
{
	SET_3G_INFO set3GInfo;
	UINT32 uErrorCode = SRIP_OK;

	char strFileName[256] = {"/etc/ppp/Open3G.sh"};
	char strConfig[256] = {0};
	char strUser[256] = {0};
	char strPwd[256] = {0};
	char strApn[256] = {0};

	memset(&set3GInfo, 0x00, sizeof(SET_3G_INFO));

	SET_3G_INFO_EX set3GInfoEx;
	set3GInfoEx.nExist3G = g_nExist3G;
	sprintf(set3GInfoEx.chIp, "%s", g_str3GIp.c_str());

	FILE* fp = NULL;
	fp = fopen(strFileName, "r");
	if(fp!=NULL)
	{
		fread(strConfig, 1, 256, fp);
		string strSet = strConfig;
		int nPos1 = strSet.find("--usr=");
		int nPos2 = strSet.find("--pwd=");
		sprintf(set3GInfo.chUserName, "%s", strSet.substr(nPos1 + 6, nPos2 - nPos1 - 7).c_str());

		nPos1 = strSet.find("--pn=");
		sprintf(set3GInfo.chPass, "%s", strSet.substr(nPos2 + 6, nPos1 - nPos2 - 7).c_str());

		nPos1 = strSet.find("--apn=");
		nPos2 = strSet.find("--type");
		sprintf(set3GInfo.chApn, "%s", strSet.substr(nPos1 + 6, nPos2 - nPos1 - 7).c_str());
		fclose(fp);
	}
	else
	{
		LogNormal("读取%s失败.\n",strFileName);
		uErrorCode = READ_3G_INFO;
	}
	//if(strlen(set3GInfo.chUserName) > 0 && strlen(set3GInfo.chPass) > 0 && strlen(set3GInfo.chApn) > 0 )
	{
		//取头
		SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

		//设置参数
		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (READ_3G_MODE_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(SET_3G_INFO)+sizeof(SET_3G_INFO_EX);
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		std::string response;
		response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
		response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
		response.append((char*)&set3GInfo, sizeof(SET_3G_INFO));
		response.append((char*)&set3GInfoEx, sizeof(SET_3G_INFO_EX));
		if(!g_skpRoadServer.SendMsg(nSocket,response))
		{
			LogError("发送SET_3G_INFO信息返回数据包失败[%d]！\r\n",nSocket);
		}
		else
		{
			LogNormal("发送SET_3G_INFO信息成功！[%d]\n",response.size());
		}
	}
	return true;
}

//// 配置3G信息
//bool CSkpRoadMsgCenter::OnSet3GMode(const int nSocket, string request)
//{
//	//取头
//	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
//	char strFileName[256] = {"/etc/ppp/Open3G.sh"};
//	char strConfig[256] = {0};
//	FILE* fp = NULL;
//
//	SET_3G_INFO set3g_info;
//	memcpy(&set3g_info,request.c_str()+sizeof(SRIP_HEADER),sizeof(SET_3G_INFO));
//
//	sprintf(strConfig, "rm -f %s", strFileName);
//	system(strConfig);
//
//	sprintf(strConfig,"route del default\nsh /etc/ppp/g3-ppp-on --usr=%s --pwd=%s --pn=*99# --apn=%s --type=32 --module=10",set3g_info.chUserName, set3g_info.chPass, set3g_info.chApn);
//	fp = fopen(strFileName, "a+");
//	if(fp!=NULL)
//	{
//		fwrite(strConfig, strlen(strConfig), 1, fp);
//		fflush(fp);
//		fclose(fp);
//		sprintf(strConfig, "chmod 777 %s", strFileName);
//		system(strConfig);
//
//		//返回验证结果
//		std::string response;
//		MIMAX_HEADER mHeader;
//		//命令
//		mHeader.uCmdID = (SET_3G_MODE_REP);
//		//长度
//		mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
//		sHeader->uMsgCode = (SRIP_OK);
//
//		response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
//		response.append((char*)sHeader,sizeof(SRIP_HEADER));
//		//发送数据
//		if(!g_skpRoadServer.SendMsg(nSocket,response))
//		{
//			LogError("发送3G配置设置回复数据包失败[%d]！\r\n",nSocket);
//			return false;
//		}
//	}
//}

//为配置信息手动上传修改文件权限
bool CSkpRoadMsgCenter::OnSettingUpload(const int nSocket, string request)
{
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	int len = request.size() - sizeof(SRIP_HEADER);
	string fileName;
	fileName.append(request.c_str() + sizeof(SRIP_HEADER), len);

	string cmd = "chmod 666 " + fileName;
	int errFlag = system(cmd.c_str());
	if (!errFlag)
	{
		LogNormal("为配置信息手动上传修改文件权限成功!fileName=%s\n", fileName.c_str());
		return true;
	}
	else
	{
		LogError("为配置信息手动上传修改文件权限失败!fileName=%s\n", fileName.c_str());
		return false;
	}
}


//录像手动上传
bool CSkpRoadMsgCenter::OnVideoUpload(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	char chVideoPath[64] = {0};
	memcpy(chVideoPath,(char*)request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));

	printf("======OnVideoUpload===chVideoPath=%s\r\n",chVideoPath);

	//删除通道1
	//通道结构
	SRIP_CHANNEL sChannel;
	//通道ID
	sChannel.uId = 1;
	g_skpDB.DelChan(sChannel);
	//删除车道
	g_skpDB.DeleteRoad(sChannel.uId);

	//新建通道1
	sChannel.eVideoFmt = VEDIO_H264;
	sChannel.nCameraType = MONITOR_CAMERA;
	memcpy(sChannel.chPlace,"test",4);
	char buf[32] = {0};
	sprintf(buf,"./update/%s",chVideoPath);
	memcpy(sChannel.chFileName,buf,32);
	g_skpDB.AddChan(sChannel);


	std::string response;

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (UPLOAD_VIDEO_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送录像手动上传返回数据包失败[%d]！\r\n",nSocket);
}

//启动命令处理线程
bool CSkpRoadMsgCenter::BeginProcessThread()
{

	//启动命令处理线程
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	//启动监控线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadProcessMsg,this);

	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建命令处理线程失败,服务无法处理客户端命令!\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}
	pthread_attr_destroy(&attr);
	return true;
}

//停止处理线程
void CSkpRoadMsgCenter::EndProcessThread()
{
	//停止线程,等待线程结束
	if(m_nThreadId != 0)
	{
		pthread_join(m_nThreadId,NULL);
		m_nThreadId = 0;
	}
	return;
}


//登录处理
bool CSkpRoadMsgCenter::OnLogin(const int nSocket,const UINT32 uCommand,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	//客户端登录
	if(uCommand == SRIP_LOGIN)
	{
		LOGIN_INFO login_info;
		memcpy(&login_info,request.c_str()+sizeof(SRIP_HEADER),sizeof(LOGIN_INFO));

		std::string user(login_info.chUserName);

		std::string pass(login_info.chPass);

		bool bCheckPriv = login_info.bCheckPriv;
		int nPriv = login_info.Priv;
		int ClientPort = login_info.ClientPort;

		LogTrace("login.log", "用户名=[%s],密码=[%s],bCheckPriv=%d,nPriv=%d, ClientPort =%d\r\n",user.c_str(),pass.c_str(),bCheckPriv,nPriv, ClientPort);
		//取时间戳
		if(uErrorCode == SRIP_OK)
		{
			if(bCheckPriv)
			uErrorCode = g_skpDB.CheckLogin(user,pass);
		}

		//判断结果,更新连接列表
		if(uErrorCode == SRIP_OK)
			g_skpRoadServer.SaveUser(nSocket,user,ClientPort,1);
		else
			LogTrace("login.log", "SRIP_LOGIN_PASS_ERROR uErrorCode=%x\r\n",uErrorCode);


		//返回验证结果
		std::string response;

		if(bCheckPriv)
		{
			//用户权限
			nPriv = (g_skpDB.GetUserPriv(user, SRIP_LOGIN));
			if (nPriv == -1)
			{
				if (user.compare("admin") == 0)
				{
					LogNormal("获取[%s]的权限失败！\n",user.c_str());
					nPriv = 1;
				}
			}
		}

        //用户权限
        sHeader->uMsgSource = (nPriv);

		//对于普通用户直接推送视频
		if(nPriv == 0)
		{
            g_skpRoadServer.SetConnect(nSocket,true);
            g_skpChannelCenter.SetChannelConnect(true);
		}

		MIMAX_HEADER mHeader;
		//命令
		mHeader.uCmdID = (SRIP_LOGIN_REP);
		//长度
		mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);

		sHeader->uMsgCode = (uErrorCode);

		response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
		response.append((char*)sHeader,sizeof(SRIP_HEADER));
		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
		{
			LogError("发送登录回复数据包失败[%d]！\r\n",nSocket);
			return false;
		}


		//验证未通过，不发送列表信息
		if(uErrorCode != SRIP_OK) return false;

        printf("==========bCheckPriv=%d\n",bCheckPriv);
		if(bCheckPriv)
		{
			//送通道列表
			response = g_skpDB.GetChannelList();
			//命令
			mHeader.uCmdID = (SRIP_CHANNEL_LIST);
			//长度
			mHeader.uCmdLen = sizeof(MIMAX_HEADER)+response.size();
			//加上头
			response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
			//发送数据
			if(!g_skpRoadServer.SendMsg(nSocket,response))
				LogError("发送通道列表数据包失败[%d]！\r\n",nSocket);
		}
	}

	return true;
}

//心跳处理
bool CSkpRoadMsgCenter::OnLink(const int nSocket,std::string request)
{
    printf("=================================OnLink \r\n");
#ifdef _DEBUG
	printf("link \r\n");
#endif

        //重置相关标识
        if(!g_skpRoadServer.ResetLinker(nSocket))
		return false;

        //发送系统信息
        //取头
		MIMAX_HEADER mHeader;

		//取通道状态
		std::string response;

		g_sysInfo.uSysTime = GetTimeStamp();//系统当前时间
		response.insert(0,(char*)&g_sysInfo,sizeof(SRIP_SYSTEM_INFO));

		/////////////////////////////////////////////////////////////////////////
		//查询返回
		mHeader.uCmdID = (SRIP_SYSTEMINFOR);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + response.size();
		//插入头部
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

        //if(g_skpRoadServer.GetCenterSocket() != nSocket)
        {
            //发送数据
            if(!g_skpRoadServer.SendMsg(nSocket,response))
               LogError("发送系统信息数据包失败[%d]！\r\n",nSocket);
        }

	return true;
}


//添加用户
//zhangyaoyao: 添加权限检查
bool CSkpRoadMsgCenter::OnUserAdd(const int nSocket,std::string request)
{
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	UINT32 uErrorCode = SRIP_OK;

	LOGIN_INFO login_info;
	memcpy(&login_info,request.c_str()+sizeof(SRIP_HEADER),sizeof(LOGIN_INFO));

	std::string user;
	user = login_info.chUserName;
	std::string pass;
	pass = login_info.chPass;

	bool bCheckPriv = login_info.bCheckPriv;
	int nPriv = login_info.Priv;

    string strLower = user;
    ToLowerCase(strLower);

    string strOperatorName = g_skpRoadServer.GetClientName(nSocket);
    ToLowerCase(strOperatorName);
    if ((g_skpDB.GetRightByName(strOperatorName) != 1) || (strOperatorName.compare("admin")!=0 && nPriv == 1)) //have no right to operate
    {
        sHeader->uMsgCode = SRIP_ERROR_NO_PRIV;

        MIMAX_HEADER mHeader;
        mHeader.uCmdID = (SRIP_USER_ADD_REP);
        //长度
        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

        std::string response;
        response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
        response.append((char*)sHeader,sizeof(SRIP_HEADER));
        //发送数据
        if(!g_skpRoadServer.SendMsg(nSocket,response))
            LogError("非管理员添加用户操作[%d]！\r\n",nSocket);

        return true;
    }

    //添加用户
    uErrorCode = g_skpDB.AddUser(strLower,pass,nPriv);
    //返回代码
    sHeader->uMsgCode = (uErrorCode);

	//返回
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_USER_ADD_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);


	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送添加用户回复数据包失败[%d]！\r\n",nSocket);


	return true;
}
//删除用户
//zhangyaoyao: 添加权限检查
bool CSkpRoadMsgCenter::OnUserDel(const int nSocket,std::string request)
{
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	UINT32 uErrorCode = SRIP_OK;

	LOGIN_INFO login_info;
	memcpy(&login_info,request.c_str()+sizeof(SRIP_HEADER),sizeof(LOGIN_INFO));

	std::string user;
	user = login_info.chUserName;
	//printf("user=%s\n",user.c_str());

    string strLower = user;
    ToLowerCase(strLower);
    int nUserRight = g_skpDB.GetRightByName(strLower);

    string strOperatorName = g_skpRoadServer.GetClientName(nSocket);
    ToLowerCase(strOperatorName);
    if ((g_skpDB.GetRightByName(strOperatorName) != 1) ||
        (strOperatorName.compare("admin")!=0 && nUserRight == 1))  //have no right to operate
    {
        sHeader->uMsgCode = SRIP_ERROR_NO_PRIV;

        MIMAX_HEADER mHeader;
        mHeader.uCmdID = (SRIP_USER_DEL_REP);
        //长度
        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

        std::string response;
        response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
        response.append((char*)sHeader,sizeof(SRIP_HEADER));
        //发送数据
        if(!g_skpRoadServer.SendMsg(nSocket,response))
            LogError("非管理员删除用户操作[%d]！\r\n",nSocket);

        return true;
    }

    if (strLower.compare("admin") == 0)
    {
        sHeader->uMsgCode = SRIP_ERROR_ADMIN_DEL;
    }
    else
    {
        //删除用户
        uErrorCode = g_skpDB.DelUser(user);
        //返回代码
        sHeader->uMsgCode = (uErrorCode);
    }

	//返回删除结果
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_USER_DEL_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送删除用户回复数据包失败[%d]！\r\n",nSocket);


	return true;
}

//修改用户
//zhangyaoyao: 添加权限检查
bool CSkpRoadMsgCenter::OnUserModify(const int nSocket,std::string request)
{
    LOGIN_INFO login_info;
	memcpy(&login_info,request.c_str()+sizeof(SRIP_HEADER),sizeof(LOGIN_INFO));

	std::string user;
	user = login_info.chUserName;
	std::string pass;
	pass = login_info.chPass;

	bool bCheckPriv = login_info.bCheckPriv;
	int nPriv = login_info.Priv;

    string strLower = user;
    ToLowerCase(strLower);

	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    string strOperatorName = g_skpRoadServer.GetClientName(nSocket);
    ToLowerCase(strOperatorName);
    int nOperatorRight = g_skpDB.GetRightByName(strOperatorName);
    int nUserRight = g_skpDB.GetRightByName(strLower);

    if ((nOperatorRight != 1 && strOperatorName != strLower) || //操作者非管理员且非本人
        (strOperatorName.compare("admin")!=0 && nUserRight == 1 && strOperatorName != strLower))
    {
        sHeader->uMsgCode = SRIP_ERROR_NO_PRIV;

        MIMAX_HEADER mHeader;
        mHeader.uCmdID = (SRIP_USER_MOD_REP);
        //长度
        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

        std::string response;
        response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
        response.append((char*)sHeader,sizeof(SRIP_HEADER));
        //发送数据
        if(!g_skpRoadServer.SendMsg(nSocket,response))
            LogError("非管理员且非用户本身修改用户信息操作[%d]！\r\n",nSocket);

        return true;
    }

	UINT32 uErrorCode = SRIP_OK;

    if (pass.size() == 0)
    {
        sHeader->uMsgCode = SRIP_ERROR_PASSW_EMPTY;
    }
    else if (strLower.compare("admin") == 0 && nPriv != 1)
    {
        sHeader->uMsgCode = SRIP_ERROR_ADMIN_MODY;
    }
    else
    {
        //修改用户
        uErrorCode = g_skpDB.ModyUser(strLower,pass,nPriv);
        //返回代码
        sHeader->uMsgCode = (uErrorCode);
    }

	//返回修改结果
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_USER_MOD_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送修改用户回复数据包失败[%d]！\r\n",nSocket);


	return true;
}
//用户列表
bool CSkpRoadMsgCenter::OnUserList(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	//取列表
	std::string response =  g_skpDB.GetUsrList();
	//返回验证结果
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_USER_LIST_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();

	sHeader->uMsgCode = (uErrorCode);

	//加上头
	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送用户列表回复数据包失败[%d]！\r\n",nSocket);


	return true;
}


//添加通道
bool CSkpRoadMsgCenter::OnChannelAdd(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	//通道结构
	SRIP_CHANNEL sChannel;
	memcpy(&sChannel,request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));

    //修改车道类型
    g_nRoadType = sChannel.nChanWayType;
    CXmlParaUtil xml;
    xml.UpdateSystemSetting("OtherSetting","ChanWayType");

	//添加通道
	if(uErrorCode == SRIP_OK)
		uErrorCode = g_skpDB.AddChan(sChannel);

	std::string response;


	bool bOpen = g_skpChannelCenter.GetChannelStatus(sChannel.uId);

	response.append((char*)&bOpen,sizeof(bOpen));


	//添加返回
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CHANNEL_ADD_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	//加上头
	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送添加通道回复数据包失败[%d]！\r\n",nSocket);

#ifdef MVSBAK
	//更新AMS,DSP通道信息
	g_AMSCommunication.mvSendChannelListXml();
#endif

	return true;
}

//删除通道
bool CSkpRoadMsgCenter::OnChannelDel(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nId = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
	//通道结构
	SRIP_CHANNEL sChannel;
	//通道ID
	sChannel.uId = nId;


	//取时间戳
	if(uErrorCode == SRIP_OK)
	{
		uErrorCode = g_skpDB.DelChan(sChannel);

		//删除车道
		g_skpDB.DeleteRoad(sChannel.uId);
	}

	//返回删除结果
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CHANNEL_DEL_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送删除通道回复数据包失败[%d]！\r\n",nSocket);

#ifdef _DEBUG
	printf("删除通道OK!\r\n");
#endif

#ifdef MVSBAK
	//更新AMS,DSP通道信息
	g_AMSCommunication.mvSendChannelListXml();
#endif

	return true;
}
//修改通道
bool CSkpRoadMsgCenter::OnChannelModify(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//通道结构
	SRIP_CHANNEL sChannel;
	memcpy(&sChannel,request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));
    printf("sChannel.nChanWayType=%d,g_nRoadType=%d\n",sChannel.nChanWayType,g_nRoadType);
    if(sChannel.nChanWayType != g_nRoadType)
    {
        //修改车道类型
        g_nRoadType = sChannel.nChanWayType;
        CXmlParaUtil xml;
        xml.UpdateSystemSetting("OtherSetting","ChanWayType");
    }

	if(g_nMultiPreSet == 1)//存在多个预置位
	{
		CXmlParaUtil xml;
		xml.UpdatePreSetDetectKind(sChannel.uId,sChannel.nPreSet,(int)sChannel.uDetectKind);
	}

	//修改通道
	if(uErrorCode == SRIP_OK)
		uErrorCode = g_skpDB.ModyChanInfo(sChannel);

#ifdef MVSBAK
	//更新AMS,DSP通道信息
	g_AMSCommunication.mvSendChannelListXml();
#endif

    return true;

	/*
	//返回修改
	std::string response;

	bool bOpen = g_skpChannelCenter.GetChannelStatus(sChannel.uId);

	response.append((char*)&bOpen,sizeof(bOpen));


	MIMAX_HEADER mHeader;
	//修改返回
	mHeader.uCmdID = (SRIP_CHANNEL_MOD_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	//加上头
	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送修改通道回复数据包失败[%d]！\r\n",nSocket);	

	return true;
	*/
}


//录像查询
bool CSkpRoadMsgCenter::OnSearchRecord(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	//查询条件
	SEARCH_ITEM search_item;
	memcpy(&search_item,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM));


	std::string response  = g_skpDB.GetVideoRecord(search_item,SRIP_PAGE_SIZE);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_SEARCH_RECORD_REP);
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送录像查询数据包失败[%d]！\r\n",nSocket);

	return true;
}

	//通道车道信息
bool CSkpRoadMsgCenter::OnRoadSave(const int nSocket,std::string request)
{
	//验证操作权限-车道信息
//	if(!CheckPriv(nSocket, SRIP_ROAD_SAVE)) return false;
#ifdef _DEBUG
	printf("road save,array data \r\n");
#endif
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//解析XML
	int nOffset = sizeof(SRIP_HEADER);
	//构造
	CSkpRoadXmlValue xmlValue(request,&nOffset);
	//判断解析是否OK
	if(!xmlValue.valid())
		uErrorCode = SRIP_ERROR_FORMAT;
	//查询条件
#ifdef _DEBUG
	int size = xmlValue.size();
	printf("road save,array size = %d \r\n",size);
#endif
	//设置参数
	if(uErrorCode == SRIP_OK)
	{
		uErrorCode = g_skpDB.SaveRoadSettingInfo(xmlValue);
	}

	MIMAX_HEADER mHeader;
	//查询返回
	mHeader.uCmdID = (SRIP_ROAD_SAVE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送设置通道车道参数返回数据包失败[%d]！\r\n",nSocket);
	return true;

}

//清空车道
bool CSkpRoadMsgCenter::OnRoadDel(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//查询条件
	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
#ifdef _DEBUG
	printf("channel id = %d \r\n",nChannel);
#endif
	//车道清空信息
	if(uErrorCode == SRIP_OK)
	{
		uErrorCode = g_skpDB.DeleteRoad(nChannel);
		g_skpChannelCenter.PauseDetect(nChannel);
	}

	//查询返回
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_ROAD_DEL_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送清空车道参数返回数据包失败[%d]！\r\n",nSocket);
	return true;

}



//系统设置
bool CSkpRoadMsgCenter::OnSysSetting(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;


	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//判断数据长度, 如果只发送了数据头，则为取配置显示
	if(request.size() == sizeof(SRIP_HEADER))
	{
		//取系统配置信息
		std::string response = GetSysConfig();

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SYS_SETTING_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		//协议头
		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送系统配置参数返回数据包失败[%d]！\r\n",nSocket);
		return true;
	}

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
		{
			char strDBCheckFileName[256] = {"/etc/DBCheck.sh"};
			char   buf[1024]={0};
			sprintf(buf,"#!/bin/bash\nsed -i /mysqlErrorCheck/d /etc/crontab\nsed '$ i30 1   * * *   root    /usr/bin/mysqlErrorCheck' -i /etc/crontab\nsed -i /mysqlErrorRepair/d /etc/crontab\nsed '$ i30 2   * * *   root    /usr/bin/mysqlErrorRepair' -i /etc/crontab\n\n");
			FILE* fp = fopen(strDBCheckFileName,"wb");
			if(fp)
			{
				fwrite(buf,1,strlen(buf),fp);
				fclose(fp);
			}
			sprintf(buf, "chmod 777 %s", strDBCheckFileName);
			system(buf);
			system(strDBCheckFileName);
		}
	    request.erase(0,sizeof(SRIP_HEADER));
		SYSTEM_CONFIG sys_config;
		memcpy(&sys_config,request.c_str(),sizeof(SYSTEM_CONFIG));
		uErrorCode = SetSysConfig(sys_config);
        printf("OnSysSetting==%d,%d,%d\n",request.size(),sizeof(SYSTEM_CONFIG),sizeof(SYSTEM_CONFIG_EX));
		if(request.size()>sizeof(SYSTEM_CONFIG))
		{
		    //系统扩展参数设置
            request.erase(0,sizeof(SYSTEM_CONFIG));
            SYSTEM_CONFIG_EX sys_config_ex;
            if(request.size() == sizeof(SYSTEM_CONFIG_EX))
            {
                memcpy(&sys_config_ex,request.c_str(),sizeof(SYSTEM_CONFIG_EX));
                uErrorCode = SetSysConfigEx(sys_config_ex);
				if (g_nClockMode == 1)
				{
					char strNtpdateFileName[256] = {"./shell/ntpdate.sh"};
					{
						char   buf[1024]={0};
						sprintf(buf,"#!/bin/bash\nsed -i /ntpstart/d /etc/crontab\n");
						FILE* fp = fopen(strNtpdateFileName,"wb");
						if(fp)
						{
							fwrite(buf,1,strlen(buf),fp);
							fclose(fp);
						}
						sprintf(buf, "chmod 777 %s", strNtpdateFileName);
						system(buf);
						system(strNtpdateFileName);
					}
				}
            }
		}
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_SYS_SETTING_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送系统配置参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//获得车道数据
bool CSkpRoadMsgCenter::OnRoadWay(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
	printf("channel id = %d \r\n",nChannel);

	//设置参数
	CXmlParaUtil xml;
	std::string response = xml.GetRoadSettingInfo(nChannel,0);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_ROAD_WAY_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送车道返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//获取车道参数模板
bool CSkpRoadMsgCenter::OnRoadWayModel(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
	int nModel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(int)));
	printf("=========channel id = %d nModel=%d\r\n",nChannel,nModel);

	//获取车道设置模板
	CXmlParaUtil xml;
	//#ifdef _DEBUG
        printf("==xml.GetRoadSettingInfo========nChannel = %d, nModel = %d========\n\r", nChannel, nModel);
	//#endif
	std::string response = xml.GetRoadSettingInfo(nChannel,nModel);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (ROAD_SETTING_MODEL_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送车道坐标模板返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//获取车道参数设置
bool CSkpRoadMsgCenter::OnRoadWayParaMeter(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));


	//设置参数
	std::string response = g_skpDB.GetRoadParaMeterInfo(nChannel);
		printf("response.size() = %d \r\n",response.size());

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (ROAD_PARAMETER_REP);


	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送车道参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//保存车道参数
bool CSkpRoadMsgCenter::OnRoadParaMeterSave(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//解析XML
	int nOffset = sizeof(SRIP_HEADER);
	//构造
	CSkpRoadXmlValue xmlValue(request,&nOffset);
	//判断解析是否OK
	if(!xmlValue.valid())
		uErrorCode = SRIP_ERROR_FORMAT;

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
		uErrorCode = g_skpDB.SaveRoadParaMeterInfo(xmlValue);
	}

	MIMAX_HEADER mHeader;
	//查询返回
	mHeader.uCmdID = (ROAD_PARAMETER_SAVE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送保存车道参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//保存车道模板参数
bool CSkpRoadMsgCenter::OnRoadModelSave(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//解析XML
	int nOffset = sizeof(SRIP_HEADER);
	//构造
	CSkpRoadXmlValue xmlValue(request,&nOffset);
	//判断解析是否OK
	if(!xmlValue.valid())
		uErrorCode = SRIP_ERROR_FORMAT;

	int nModelID = 0;

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
		uErrorCode = g_skpDB.SaveRoadModelInfo(xmlValue,nModelID);
	}

	std::string response;
	response.append((char*)&nModelID,sizeof(nModelID));

	MIMAX_HEADER mHeader;
	//查询返回
	mHeader.uCmdID = (ROAD_MODEL_SAVE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);


	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送保存车道模板参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//通道参数调整
bool CSkpRoadMsgCenter::OnAdjustChannelPara(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//视频参数结构
	SRIP_CHANNEL_ATTR sAttr;
	memcpy(&sAttr,request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));


	//同步通道，更新
	g_skpChannelCenter.SetVideoParams(sAttr);


	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;

	MIMAX_HEADER mHeader;
	//修改返回
	mHeader.uCmdID = (SRIP_CHANNEL_PARA_ADJUST_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);


	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送设置通道视频参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//通道参数保存
bool CSkpRoadMsgCenter::OnSaveChannelPara(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//视频参数结构
	SRIP_CHANNEL_ATTR sAttr;
	memcpy(&sAttr,request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));


	//保存参数
	if(uErrorCode == SRIP_OK)
		uErrorCode = g_skpDB.ModifyVideoParams(sAttr);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	MIMAX_HEADER mHeader;
	//修改返回
	mHeader.uCmdID = (SRIP_CHANNEL_PARA_SAVE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据


	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送保存通道视频参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//备份数据库
bool CSkpRoadMsgCenter::OnBackupDB(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;

	std::string db_name,backup_path;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//保存参数
	if(uErrorCode == SRIP_OK)
		uErrorCode = g_skpDB.BackupDB(db_name,backup_path);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_BACKUP_DATABASE_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送备份数据库返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//数据库修复
bool CSkpRoadMsgCenter::OnDBRepair(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;

	std::string db_name,backup_path;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(uErrorCode == SRIP_OK)
	{
		static int way =0 ;
		LogNormal("OnDBRepair");
		if (way == 0)
		{
			if(IsDataDisk())
			{
				system("myisamchk -rof /detectdata/mysql/bocom_db/NUMBER_PLATE_INFO;   \
				   myisamchk -rof /detectdata/mysql/bocom_db/TRAFFIC_STATISTIC_INFO; \
				   myisamchk -rof /detectdata/mysql/bocom_db/TRAFFIC_EVENT_INFO;     \
				   myisamchk -rof /detectdata/mysql/bocom_db/SYSTEM_EVENT_INFO;      \
				   myisamchk -rof /detectdata/mysql/bocom_db/VIDEO_FILE_INFO;");
			}
			else
			{
				system("myisamchk -rof /var/lib/mysql/bocom_db/NUMBER_PLATE_INFO;   \
				   myisamchk -rof /var/lib/mysql/bocom_db/TRAFFIC_STATISTIC_INFO; \
				   myisamchk -rof /var/lib/mysql/bocom_db/TRAFFIC_EVENT_INFO;     \
				   myisamchk -rof /var/lib/mysql/bocom_db/SYSTEM_EVENT_INFO;      \
				   myisamchk -rof /var/lib/mysql/bocom_db/VIDEO_FILE_INFO;");
			}
			way = 1;
		}
		else
		{
			g_skpDB.execSQL(string("REPAIR TABLE NUMBER_PLATE_INFO;"));
			g_skpDB.execSQL(string("REPAIR TABLE TRAFFIC_STATISTIC_INFO;"));
			g_skpDB.execSQL(string("REPAIR TABLE TRAFFIC_EVENT_INFO;"));
			g_skpDB.execSQL(string("REPAIR TABLE SYSTEM_EVENT_INFO;"));
			g_skpDB.execSQL(string("REPAIR TABLE VIDEO_FILE_INFO"));
			
		}
	system("service mysql restart");      // 重启数据库的服务
	system("chown -R mysql:mysql /var/lib/mysql/bocom_db/*");
	g_skpDB.UnInit();                   //连接MYSQL 及 FS 初始化
	if (!g_skpDB.Init())
	{
		LogError("连接MYSQL或者初始化FS失败，系统启动中止！\r\n");
		g_bEndThread = true;

	}
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (DB_REPAIR_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送数据库修复返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//数据库恢复
bool CSkpRoadMsgCenter::OnDBDefault(const int nSocket,std::string request)
{
     
	UINT32 uErrorCode = SRIP_OK;

	std::string db_name,backup_path;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(uErrorCode == SRIP_OK)
	{	
		if(g_skpDB.DBImport())
		{
			LogNormal("导入数据库成功\n");
		}
		else
		{
			LogNormal("导入数据库失败\n");
		}

		if(g_skpDB.DBUpdate())
		{
			LogNormal("升级数据库成功\n");
		}
		else
		{
			LogNormal("升级数据库失败\n");
		}

		g_skpDB.UnInit();                   //连接MYSQL 及 FS 初始化
		
		/*if (g_nServerType == 7)
		{
			system("rm -rf /home/road/flow/*");
			system("rm -rf /home/road/kakou/*");
			system("rm -rf /home/road/red/*");
			system("rm -rf /home/road/video/*");

		}
	
		system("rm -rf /home/road/server/pic/*");
		system("rm -rf /home/road/server/video/*");
		system("rm -f  /var/lib/mysql/bocom_db/NUMBER_PLATE_INFO.* \
			   SYSTEM_EVENT_INFO.*  TRAFFIC_EVENT_INFO.* \
			   TRAFFIC_STATISTIC_INFO.* PIC_INFO.*  VIDEO_FILE_INFO.*  USER_INFO.*");*/
		//system("./dbimport db_default.sql");
		//system("./DBUpder.out db_upd_sql.xml");
		if (!g_skpDB.Init())
		{
			LogError("连接MYSQL或者初始化FS失败，系统启动中止！\r\n");
			g_bEndThread = true;

		}
		else
		{
			LogNormal("恢复重启数据库成功\n");
		}
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (DB_DEFAULT_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送数据库恢复返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//获取视频参数
bool CSkpRoadMsgCenter::OnGetAdjustPara(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CHANNEL_PARA_GET_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER) + sizeof(SRIP_CHANNEL_ATTR);

    SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//视频参数结构
	SRIP_CHANNEL_ATTR sAttr;
	memcpy(&sAttr,request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));

	g_skpDB.GetAdjustPara(sAttr);
    std::string response;

    sHeader->uMsgCode = uErrorCode;
    response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	response.append((char*)(&sAttr),sizeof(SRIP_CHANNEL_ATTR));

	printf("mHeader.uCmdID=%x--sAttr.uId = %d,sAttr.uBrightness-=%d-mHeader.uCmdLen=%d,response.size()=%d\n",mHeader.uCmdID,sAttr.uId,sAttr.uBrightness,mHeader.uCmdLen,response.size());
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送获取视频参数返回数据包失败[%d]！\r\n",nSocket);
	}

	return true;
}

//发送系统信息
bool CSkpRoadMsgCenter::SendSysMsg()
{

	//////有连接才发送
	{
		//取头
		MIMAX_HEADER mHeader;

		//取通道状态
		std::string response;

		g_sysInfo.uSysTime = GetTimeStamp();//系统当前时间
		response.insert(0,(char*)&g_sysInfo,sizeof(SRIP_SYSTEM_INFO));

		/////////////////////////////////////////////////////////////////////////
		//查询返回
		mHeader.uCmdID = (SRIP_SYSTEMINFOR);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + response.size();
		//插入头部
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
		//发送数据
		if(!g_skpRoadServer.SendToClient(response))
		{
		    LogError("发送系统信息数据包失败！\r\n");
		}
	}

	return true;
}


//图表查询
bool CSkpRoadMsgCenter::OnChartQuery(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	CHARTQUERY_ITEM chartquery_item;
	memcpy(&chartquery_item,request.c_str()+sizeof(SRIP_HEADER),sizeof(CHARTQUERY_ITEM));
	int nChannelId = chartquery_item.uChannelId;
	int nRoadIndex = chartquery_item.uRoadId;

	int uYear = chartquery_item.uYear;
	int uMonth = chartquery_item.uMonth;
	int uDay = chartquery_item.uDay;
	int uHour = chartquery_item.uHour;
	char buf[32];
	// 组成 2008-08-01的串形式
	sprintf(buf, "%4d-%02d-%02d", uYear, uMonth, uDay);
	String strDate(buf);
	int dateType =  chartquery_item.uDateType;
	int queryType = chartquery_item.uQueryType;
	int typeValue = chartquery_item.uTypeValue;

	std::string response = g_skpDB.GetChartQuery(nChannelId,nRoadIndex,strDate, dateType, queryType, typeValue);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CHARTQUERY_REP);
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送图表查询数据包失败[%d]！\r\n",nSocket);

	return true;
}

//车牌以及事件快照图片查询
bool CSkpRoadMsgCenter::OnCardPic(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//查询条件
	int nKind = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
	std::string strID(request.c_str()+sizeof(SRIP_HEADER)+sizeof(int));
    UINT32 uID = strtoul(strID.c_str(),NULL,10);

    //获取图片信息
    std::string response = g_skpDB.GetPicInfoByID(uID,nKind);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CARD_PIC_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER) +response.size();

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送车牌图片数据包失败[%d]！\r\n",nSocket);

	return true;
}


bool CSkpRoadMsgCenter::OnChannelPause(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //通道ID
	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));

	g_skpChannelCenter.PauseChannel(nChannel);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CHANNEL_PAUSE_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送暂停通道回复数据包失败[%d]！\r\n",nSocket);

#ifdef MVSBAK
	//更新AMS,DSP通道信息
	g_AMSCommunication.mvSendChannelListXml();
#endif

	return true;
}

bool CSkpRoadMsgCenter::OnChannelResume(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


    //通道ID
	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));

	g_skpChannelCenter.ResumeChannel(nChannel);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CHANNEL_RESUME_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送暂停通道回复数据包失败[%d]\r\n",nSocket);

#ifdef MVSBAK
	//更新AMS,DSP通道信息
	g_AMSCommunication.mvSendChannelListXml();
#endif

	return true;
}



//连接断开通道
bool CSkpRoadMsgCenter::OnSetChannelConnect(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	/////////////////////////////////////////////
	int nChannel = 0;
	if(request.size()>=sizeof(SRIP_HEADER))
	{
		nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
	}
	/////////////////////////////////////////////


	if(sHeader->uMsgCommandID == MIMAX_CHANNEL_CONNECT ||
		sHeader->uMsgCommandID == MIMAX_SEND_FRAME)
	{
		g_skpRoadServer.SetConnect(nSocket,true);
		printf("i'm connect nChannel=%d!!!!!\r\n",nChannel);
		g_skpChannelCenter.SetChannelConnect(true,nChannel);
	}
	else if(sHeader->uMsgCommandID==MIMAX_CHANNEL_DISCONNECT ||
		sHeader->uMsgCommandID == MIMAX_NOSEND_FRAME)
	{
		printf("i'm disconnect nChannel=%d!!!!!\r\n",nChannel);
		if(g_skpRoadServer.GetConnectClientCount()==1)
		g_skpChannelCenter.SetChannelConnect(false,nChannel);

		if(g_skpChannelCenter.GetConnectChannelCount()==0)
		g_skpRoadServer.SetConnect(nSocket,false);
	}


	return true;
}

//获取车牌检测参数
bool CSkpRoadMsgCenter::OnCardNumParam(const int nSocket,std::string request)
{

	return true;

}

//保存车牌检测参数
bool CSkpRoadMsgCenter::OnSaveCardNumParam(const int nSocket,std::string request)
{

	return true;
}


//获取闯红灯检测参数
bool CSkpRoadMsgCenter::OnVTSParam(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
	CXmlParaUtil xml;
    VTSParaMap MapVTSPara;
    VTS_GLOBAL_PARAMETER vtsGlobalPara;
	//设置参数
	xml.LoadVTSParameter(MapVTSPara,nChannel,vtsGlobalPara);

	std::string response;
    VTSParaMap::iterator it_b = MapVTSPara.begin();
    VTSParaMap::iterator it_e = MapVTSPara.end();
    while(it_b!=it_e)
    {
        PARAMETER_VTS vtsPara = it_b->second;
        response.append((char*)&vtsPara,sizeof(vtsPara));
        it_b++;
    }
    response.append((char*)&vtsGlobalPara,sizeof(vtsGlobalPara));

	//设置参数
	//std::string response = g_skpDB.GetVTSParaMeterInfo(nChannel);
    printf("response.size() = %d \r\n",response.size());

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (VTS_PARAMETER_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送闯红灯检测参数返回数据包失败[%d]\r\n",nSocket);
	return true;
}

//保存闯红灯检测参数
bool CSkpRoadMsgCenter::OnSaveVTSParam(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
    request.erase(0,sizeof(SRIP_HEADER)+sizeof(int));

    VTS_GLOBAL_PARAMETER vtsGlobalPara;
    VTSParaMap mapVTSPara;


    //判断是否包含全局变量
    bool bHasGlobal = false;
    if((request.size()-sizeof(VTS_GLOBAL_PARAMETER)) > 0)
    {
        if( ((request.size()-sizeof(VTS_GLOBAL_PARAMETER))%sizeof(PARAMETER_VTS)) == 0 )
        {
            bHasGlobal = true;
        }
    }

    unsigned int uSize = 0;
    if(bHasGlobal)
    {
        uSize = (request.size()-sizeof(VTS_GLOBAL_PARAMETER))/sizeof(PARAMETER_VTS);
    }
    else
    {
        uSize = (request.size())/sizeof(PARAMETER_VTS);
    }


    for(int i = 0;i< uSize; i++)
    {
        PARAMETER_VTS vtsPara;
        memcpy(&vtsPara,request.c_str()+i*sizeof(PARAMETER_VTS),sizeof(PARAMETER_VTS));
        printf("===before mapVTSPara\n");
        mapVTSPara.insert(VTSParaMap::value_type(vtsPara.nRoadIndex,vtsPara));
        printf("===end mapVTSPara\n");
    }

    if(bHasGlobal)
    memcpy(&vtsGlobalPara,request.c_str()+uSize*sizeof(PARAMETER_VTS),sizeof(vtsGlobalPara));

	//设置参数
	CXmlParaUtil xml;
    xml.AddTrafficParameterByList(mapVTSPara,nChannel,vtsGlobalPara);
    SRIP_CHANNEL_EXT sChannel;					/* 通道检测参数*/
    sChannel.uId = nChannel;
    //要求检测重新读取通道配置
	g_skpChannelCenter.ReloadChannelParaMeter(sChannel,false);

	//设置参数
    //uErrorCode = g_skpDB.SaveVTSParameterInfo(request);

	MIMAX_HEADER mHeader;
	//查询返回
	mHeader.uCmdID = (VTS_PARAMETER_SAVE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送设置闯红灯检测参数返回数据包失败[%d]\r\n",nSocket);
	return true;
}

//获取线圈检测参数
bool CSkpRoadMsgCenter::OnLoopParam(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));

    CXmlParaUtil xml;
    LoopParaMap MapLoopPara;
	//设置参数
	xml.LoadLoopParameter(MapLoopPara,nChannel);


    std::string response;
    LoopParaMap::iterator it_b = MapLoopPara.begin();
    LoopParaMap::iterator it_e = MapLoopPara.end();
    while(it_b!=it_e)
    {
        PARAMETER_LOOP loopPara = it_b->second;
        response.append((char*)&loopPara,sizeof(loopPara));
        it_b++;
    }
    printf("response.size() = %d \r\n",response.size());

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (LOOP_PARAMETER_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送线圈检测参数返回数据包失败[%d]\r\n",nSocket);
	return true;
}

//保存线圈检测参数
bool CSkpRoadMsgCenter::OnSaveLoopParam(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
    request.erase(0,sizeof(SRIP_HEADER)+sizeof(int));

    LoopParaMap MapLoopPara;
    unsigned int uSize = (request.size())/sizeof(PARAMETER_LOOP);

    for(int i = 0;i< uSize; i++)
    {
        PARAMETER_LOOP loopPara;
        memcpy(&loopPara,request.c_str()+i*sizeof(PARAMETER_LOOP),sizeof(PARAMETER_LOOP));
        printf("===before MapLoopPara\n");
        MapLoopPara.insert(LoopParaMap::value_type(loopPara.nRoadIndex,loopPara));
        printf("===end MapLoopPara\n");
    }

	//设置参数
	CXmlParaUtil xml;
    xml.AddLoopParameterByList(MapLoopPara,nChannel);
    SRIP_CHANNEL_EXT sChannel;					/* 通道检测参数*/
    sChannel.uId = nChannel;
    //要求检测重新读取通道配置
	g_skpChannelCenter.ReloadChannelParaMeter(sChannel,false);

	MIMAX_HEADER mHeader;
	//查询返回
	mHeader.uCmdID = (LOOP_PARAMETER_SAVE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送设置线圈检测参数返回数据包失败[%d]\r\n",nSocket);
	return true;
}


//雷达参数设置
bool CSkpRoadMsgCenter::OnRadarParameterSet(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
     //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    	//判断数据长度, 如果只发送了数据头，则为取配置显示
	if(request.size() == sizeof(SRIP_HEADER))
	{
	    int nChannel = sHeader->uMsgSource;

		//取雷达参数设置
		std::string response;

        CXmlParaUtil xml;
		xml.GetRadarParameter(nChannel,response);

		printf("======OnRadarParameterSet=====response=%d\n",response.size());

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (RADAR_PARAMETER_SET_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		//协议头
		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送获取雷达参数回复数据包失败[%d]！\r\n",nSocket);
		return true;
	}

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
        std::string strRadarPara;
        strRadarPara.append((char*)(request.c_str()+sizeof(SRIP_HEADER)),request.size() - sizeof(SRIP_HEADER));

        int nChannel = sHeader->uMsgSource;
        CXmlParaUtil xml;
        bool bRet = xml.SetRadarParameter(nChannel,strRadarPara);

        if(!bRet)
        {
            uErrorCode = SRIP_ERROR_USER_FAILE;
        }
        else
        {
            //要求检测重新读取配置
            g_skpChannelCenter.ReloadDetect(nChannel);
        }
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (RADAR_PARAMETER_SET_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送设置雷达参数回复数据包失败[%d]！\r\n",nSocket);

	return true;
}

//目标检测参数设置
bool CSkpRoadMsgCenter::OnObjectParameterSet(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
     //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    	//判断数据长度, 如果只发送了数据头，则为取配置显示
	if(request.size() == sizeof(SRIP_HEADER))
	{
	    int nChannel = sHeader->uMsgSource;

		//取目标检测参数设置
		std::string response;

        CXmlParaUtil xml;
		xml.GetObjectParameter(nChannel,response);

		printf("======OnRadarParameterSet=====response=%d\n",response.size());

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (OBJECT_PARAMETER_SET_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		//协议头
		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送获取目标检测参数回复数据包失败[%d]！\r\n",nSocket);
		return true;
	}

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
        std::string strObjectPara;
        strObjectPara.append((char*)(request.c_str()+sizeof(SRIP_HEADER)),request.size() - sizeof(SRIP_HEADER));

        int nChannel = sHeader->uMsgSource;
        CXmlParaUtil xml;
        bool bRet = xml.SetObjectParameter(nChannel,strObjectPara);

        if(!bRet)
        {
            uErrorCode = SRIP_ERROR_USER_FAILE;
        }
        else
        {
            //要求检测重新读取配置
            g_skpChannelCenter.ReloadDetect(nChannel);
        }
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (OBJECT_PARAMETER_SET_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送设置目标检测参数回复数据包失败[%d]！\r\n",nSocket);

	return true;
}

 //行为分析参数设置
bool CSkpRoadMsgCenter::OnBehaviorParameterSet(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
     //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    	//判断数据长度, 如果只发送了数据头，则为取配置显示
	if(request.size() == sizeof(SRIP_HEADER))
	{
	    int nChannel = sHeader->uMsgSource;

	    //取目标检测参数设置
		std::string response;

		printf("OnBehaviorParameterSet nChannel=%d\n",nChannel);

        CXmlParaUtil xml;
		xml.GetBehaviorParameter(nChannel,response);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (BEHAVIOR_PARAMETER_SET_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		//协议头
		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送获取行为分析参数回复数据包失败[%d]！\r\n",nSocket);
		return true;
	}

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
        std::string strBehaviorPara;
        strBehaviorPara.append((char*)(request.c_str()+sizeof(SRIP_HEADER)),request.size() - sizeof(SRIP_HEADER));

        int nChannel = sHeader->uMsgSource;
        CXmlParaUtil xml;
        bool bRet = xml.SetBehaviorParameter(nChannel,strBehaviorPara);

        if(!bRet)
        {
            uErrorCode = SRIP_ERROR_USER_FAILE;
        }
        else
        {
            //要求检测重新读取配置
            g_skpChannelCenter.ReloadDetect(nChannel);
        }
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (BEHAVIOR_PARAMETER_SET_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送设置行为分析参数回复数据包失败[%d]！\r\n",nSocket);

    return true;
}

//相机控制返回
bool CSkpRoadMsgCenter::OnCameraSetup(const int nSocket,std::string request)
{
  //  CXmlParaUtil xml;
  //  string strPreset = "<Channel><ChannelId>3</ChannelId><RemotePreSets><RemotePreSet><RemotePreSetID>2</RemotePreSetID><LocalPreSets><LocalPreSet><LocalPreSetID>2</LocalPreSetID><LocalPreSetArea><Points><Point><x>298.642534</x><y>329.411765</y></Point><Point><x>8.534</x><y>9.765</y></Point></Points></LocalPreSetArea></LocalPreSet></LocalPreSets></RemotePreSet></RemotePreSets></Channel>";

   // xml.UpdatePreSetInfo(strPreset);

   //  string strPreset = "<Channel><ChannelId>3</ChannelId><RemotePreSets><RemotePreSet><RemotePreSetID>2</RemotePreSetID></RemotePreSet></RemotePreSets></Channel>";

  //  xml.DeletePreSetInfo(strPreset);

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	CAMERA_CONFIG cfg;
	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));//获取消息长度
	memcpy(&cfg,request.c_str()+sizeof(SRIP_HEADER)+sizeof(int),request.size()-sizeof(SRIP_HEADER)-sizeof(int));//获取命令

	std::string response;

printf("=======CSkpRoadMsgCenter::OnCameraSetup=======cfg.uType=%d\n",cfg.uType);

	if(cfg.uType==0)//获取当前所有设置信息
	{
	    printf("=======CSkpRoadMsgCenter::OnCameraSetup======cfg.uType=%d\n",cfg.uType);
		g_skpChannelCenter.CameraControl(nChannel,cfg);
		response.append((char*)&cfg,sizeof(cfg));
	}
	else if(cfg.uType==1)//设置
	{
	    printf("=======CSkpRoadMsgCenter::OnCameraSetup======cfg.uType=%d\n",cfg.uType);

		int nPreSet = g_skpDB.GetPreSet(nChannel);

		g_skpChannelCenter.CameraControl(nChannel,cfg);

		if(cfg.nIndex == GOTO_PRESET)//调用预置位
		{
			int nPreSetID =(int)cfg.fValue;

			if(nPreSet != nPreSetID)//预置位变换需要重新启动检测
			{
				g_skpChannelCenter.ReloadDetect(nChannel);
			}
		}
	}
	else if(cfg.uType==2)//读取
	{
	    g_skpChannelCenter.CameraControl(nChannel,cfg);
	    response.append((char*)&cfg,sizeof(cfg));
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_CAMERA_SETUP_REP);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	if(cfg.uType==0||cfg.uType==2)
	{
		//长度
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();
	}
	else if(cfg.uType==1)
	{
		//长度
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	}

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//assert(response.nPort!=1 ||)

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送相机控制回复数据包失败[%d]\r\n",nSocket);

printf("=======CSkpRoadMsgCenter::CameraControl=====\n");

	return true;
}

//获取预置位信息
bool CSkpRoadMsgCenter::OnPreSetInfo(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
    SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    MIMAX_HEADER mHeader;
    std::string response;


    if(sHeader->uMsgCommandID == PRESET_INFO)//获取预置位信息
    {
        //
      /*  FILE* fp = fopen("PreSetInfo.xml","wb");
        fwrite(request.c_str()+sizeof(SRIP_HEADER),request.size()+sizeof(SRIP_HEADER),1,fp);
        fclose(fp);*/

        CXmlParaUtil xml;
        string strPreset;
        strPreset.append(request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));
        xml.GetPreSetInfo(strPreset,response);

        mHeader.uCmdID = PRESET_INFO_REP;
        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();

        /*  fp = fopen("PreSetInfo1.xml","wb");
            fwrite(response.c_str(),response.size(),1,fp);
            fclose(fp);*/
    }

    //返回代码
	sHeader->uMsgCode = (uErrorCode);
    response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
    response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送获取预置位信息返回数据包失败[%d]\r\n",nSocket);

    return true;
}

//增加和更新预置位信息
bool CSkpRoadMsgCenter::OnUpdatePreSetInfo(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
    SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    MIMAX_HEADER mHeader;
    std::string response;


    if( (sHeader->uMsgCommandID == ADD_PRESET_INFO) ||
        (sHeader->uMsgCommandID == MOD_PRESET_INFO))
    {
        //
        /*FILE* fp = fopen("region.xml","wb");
        fwrite(request.c_str()+sizeof(SRIP_HEADER),request.size()+sizeof(SRIP_HEADER),1,fp);
        fclose(fp);*/

        CXmlParaUtil xml;
        string strPreset;
        strPreset.append(request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));
        xml.UpdatePreSetInfo(strPreset);

        if(sHeader->uMsgCommandID == ADD_PRESET_INFO)
        mHeader.uCmdID = ADD_PRESET_INFO_REP;
        else if(sHeader->uMsgCommandID == MOD_PRESET_INFO)
        mHeader.uCmdID = MOD_PRESET_INFO_REP;

        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
    }

    //返回代码
	sHeader->uMsgCode = (uErrorCode);
    response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
    response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送增加预置位信息返回数据包失败[%d]\r\n",nSocket);

    return true;
}

//删除预置位
bool CSkpRoadMsgCenter::OnDeletePreSetInfo(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
    SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    MIMAX_HEADER mHeader;
    std::string response;


    if( (sHeader->uMsgCommandID == DEL_PRESET_INFO))
    {
        //
        /*FILE* fp = fopen("region.xml","wb");
        fwrite(request.c_str()+sizeof(SRIP_HEADER),request.size()+sizeof(SRIP_HEADER),1,fp);
        fclose(fp);*/

        CXmlParaUtil xml;
        string strPreset;
        strPreset.append(request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));
        xml.DeletePreSetInfo(strPreset);

        mHeader.uCmdID = DEL_PRESET_INFO_REP;

        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
    }

    //返回代码
	sHeader->uMsgCode = (uErrorCode);
    response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
    response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送删除预置位信息返回数据包失败[%d]\r\n",nSocket);

    return true;
}


//系统时间设置
bool CSkpRoadMsgCenter::OnSysTime(const int nSocket,std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    SYSTEM_CONFIG sys_config;
    memcpy(&sys_config,request.c_str()+sizeof(SRIP_HEADER),sizeof(SYSTEM_CONFIG));

    if(sys_config.uTimeStamp > 0)
    {
		struct timeval timer;
		timer.tv_sec = sys_config.uTimeStamp;
		timer.tv_usec = 0;
		if (settimeofday(&timer, NULL) == 0)
		{
			printf("================OnSysTime=%s\n",GetTime(sys_config.uTimeStamp));
			system("hwclock --systohc");
		}
    }

    MIMAX_HEADER mHeader;
    mHeader.uCmdID = SYSTIME_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送检测器时间设置回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//日志查询
bool CSkpRoadMsgCenter::OnSearchLog(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uMsgCommandID==PLATE_LOG||sHeader->uMsgCommandID==EVENT_LOG)
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetLog(uBeginTime,uEndTime,0);
	}
	else if(sHeader->uMsgCommandID==PLATE_SEQ_LOG||sHeader->uMsgCommandID==EVENT_SEQ_LOG)
	{
		UINT32 uBeginSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetLog(uBeginSeq,uEndSeq,1);
	}
	else if(sHeader->uMsgCommandID==PLATE_NONE_LOG)
	{
	    #ifdef SEND_NOREALTIME_RECORD
		g_skpDB.GetLog(0,0,2,nSocket);
		#endif
	}
	/////////////////////////////////////////////////////////////////////
	else if(sHeader->uMsgCommandID==(SRIP_SEARCH_LOG))
	{
		SEARCH_ITEM search_item;
		memcpy(&search_item,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM));

		std::string response = g_skpDB.GetSysEvent(search_item,SRIP_PAGE_SIZE);

		//返回代码
		sHeader->uMsgCode = uErrorCode;

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_LOG_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送日志回复数据包失败[%d]\r\n",nSocket);
	}

	return true;
}


//统计查询
bool CSkpRoadMsgCenter::OnSearchStatistic(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uMsgCommandID==(MIMAX_STATISTIC))
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetStatisticInfo(uBeginTime,uEndTime,0);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_SEQ_STATISTIC))
	{
		UINT32 uBeginSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetStatisticInfo(uBeginSeq,uEndSeq,1);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_NONE_STATISTIC))
	{
	    #ifdef SEND_NOREALTIME_RECORD
		g_skpDB.GetStatisticInfo(0,0,2,nSocket);
		#endif
	}
	else if(sHeader->uMsgCommandID==(SRIP_SEARCH_ALARM))
	{
		SEARCH_ITEM search_item;
		memcpy(&search_item,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM));

		std::string response = g_skpDB.GetStatistic(search_item,SRIP_PAGE_SIZE);

		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_ALARM_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送统计回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}

//事件查询
bool CSkpRoadMsgCenter::OnSearchEvent(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	//是否需要发送快照图片
	UINT32 nType = sHeader->uCmdFlag;


	if(sHeader->uMsgCommandID==(MIMAX_EVENT))
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetTraEvent(uBeginTime,uEndTime);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_SEQ_EVENT))
	{
		UINT32 uBeginSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetTraEvent(uBeginSeq,uEndSeq);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_NONE_EVENT))
	{
	    #ifdef SEND_NOREALTIME_RECORD
		g_skpDB.GetTraEvent(0,0);
		#endif
	}
	else if(sHeader->uMsgCommandID==(SRIP_SEARCH_EVENT))
	{
		SEARCH_ITEM search_item;
		memcpy(&search_item,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM));

		std::string response = g_skpDB.GetEvent(search_item,SRIP_PAGE_SIZE);

		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_EVENT_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送事件回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}

//车牌查询
bool CSkpRoadMsgCenter::OnSearchPlate(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//是否需要带上大小图片
	UINT32 nType = sHeader->uCmdFlag;

	if(sHeader->uMsgCommandID==(MIMAX_PLATE))
	{
		UINT32 uBeginTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndTime = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetPlate(uBeginTime,uEndTime);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_SEQ_PLATE))
	{
		UINT32 uBeginSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
		UINT32 uEndSeq = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));
		g_skpDB.GetPlate(uBeginSeq,uEndSeq);
	}
	else if(sHeader->uMsgCommandID==(MIMAX_NONE_PLATE))
	{
	    #ifdef SEND_NOREALTIME_RECORD
		g_skpDB.GetPlate(0,0);
		#endif
	}
	else if(sHeader->uMsgCommandID==(SRIP_SEARCH_CARD))
	{
		SEARCH_ITEM search_item;
		memcpy(&search_item,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM));

		std::string response = g_skpDB.GetCarNum(search_item,SRIP_PAGE_SIZE);

		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_CARD_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送车牌回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}


//删除各种记录
bool CSkpRoadMsgCenter::OnDeleteResult(const int nSocket,std::string request)
{

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	UINT32 uBegin = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER));
	UINT32 uEnd = *(UINT32*)(request.c_str()+sizeof(SRIP_HEADER)+sizeof(UINT32));

	//处理命令
	switch((sHeader->uMsgCommandID))
	{
		case MIMAX_DELTIME_EVENT:
			{
		          g_skpDB.DelTraEvent(uBegin,uEnd,0);

			}
			 break;
		case MIMAX_DELSEQ_EVENT:
			{
		           g_skpDB.DelTraEvent(uBegin,uEnd,1);
			}
			 break;
		case MIMAX_DELTIME_STATISTIC:
			{
				g_skpDB.DelStatisticInfo(uBegin,uEnd,0);
			}
			 break;
		case MIMAX_DELSEQ_STATISTIC:
			{
				g_skpDB.DelStatisticInfo(uBegin,uEnd,1);

			}
			 break;
		case PLATE_DEL_LOG:
		case EVENT_DEL_LOG:
			{
				g_skpDB.DeleteLog(uBegin,uEnd,0);
			}
			break;
		case PLATE_DELSEQ_LOG:
		case EVENT_DELSEQ_LOG:
			{
				g_skpDB.DeleteLog(uBegin,uEnd,1);
			}
			break;
		case MIMAX_DELTIME_PLATE:
			{
				g_skpDB.DelPlate(uBegin,uEnd,0);
			}
			break;
		case MIMAX_DELSEQ_PLATE:
			{
				g_skpDB.DelPlate(uBegin,uEnd,1);
			}
			break;
		case SRIP_DELETE_RECORD:	//删除录象
			{
				g_skpDB.DeleteRecord(uBegin,uEnd);
			}
			break;
	}

	return true;
}


//修改车牌号码
bool CSkpRoadMsgCenter::OnModifyCarNum(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	
	string strRequest;
	strRequest.append(request.c_str()+sizeof(SRIP_HEADER),sizeof(RECORD_PLATE_CLIENT));
	uErrorCode = g_skpDB.ModifyCarNum(0,strRequest);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = MODIFY_CARNUM_REP;
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送修改车牌回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//判断事件录像是否完成
bool CSkpRoadMsgCenter::OnVideoState(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	request.erase(0,sizeof(SRIP_HEADER));

	bool bSave = g_skpDB.GetVideoState(request);

	printf("strPath=%s,bSave=%d\n",request.c_str(),bSave);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = VIDEO_STATE_REP;
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+sizeof(bool);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	response.append((char*)&bSave,sizeof(bool));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送录象状态回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//截取图片
bool CSkpRoadMsgCenter::OnCaptureOneFrame(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


    std::string response;
    MIMAX_HEADER mHeader;
	ImageRegion imgRegion;

	if(sHeader->uMsgCommandID == SRIP_CHANNEL_BIG)
	{
        memcpy(&imgRegion,request.c_str()+sizeof(SRIP_HEADER)+sizeof(int),sizeof(ImageRegion));
        int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
        g_skpChannelCenter.CaptureOneFrame(response,nChannel,imgRegion);
        mHeader.uCmdID = (SRIP_CHANNEL_BIG_REP);
        //长度
        mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();
        //返回代码
        sHeader->uMsgCode = (uErrorCode);

        response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
        response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

        //发送数据
        if(!g_skpRoadServer.SendMsg(nSocket,response))
            LogError("发送截取大图回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}

//车牌高级查询--add-by-ywx-2009-08-31
bool CSkpRoadMsgCenter::OnSearchPlateHigh(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//是否需要带上大小图片
	UINT32 nType = sHeader->uCmdFlag;

	if(sHeader->uMsgCommandID==(SRIP_SEARCH_CARD_HIGH))
	{
		SEARCH_ITEM_CARNUM search_item_carnum;
		memcpy(&search_item_carnum,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM_CARNUM));

		std::string response = g_skpDB.GetCarNumHigh(search_item_carnum,SRIP_PAGE_SIZE);

		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_CARD_HIGH_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送车牌回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}


 /*************************************************
 *	函数简介：特征搜索高级查询--add-by-ywx-2009-09-11
 *  输入：nSocket: 对应套接字 request:请求消息
 *  返回值：TRUE:特征搜索高级查询成功 FALSE:失败
 **************************************************/
bool CSkpRoadMsgCenter::OnSearchTextureHigh(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//取消息类型
	UINT32 nType = sHeader->uCmdFlag;

	if(sHeader->uMsgCommandID==(SRIP_SEARCH_TEXTURE_HIGH))
	{
		SEARCH_ITEM_CARNUM search_item_carnum;
		memcpy(&search_item_carnum,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM_CARNUM));

        //for test
        printf("======================Enter GetTextureHigh()--------!!!\n");
		std::string response = g_skpDB.GetTextureHigh(search_item_carnum, MAX_TEXTURES);
        //for test
        printf("======================Out GetTextureHigh--------!!!\n");

		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_TEXTURE_HIGH_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

        //for test
        printf("======================CSkpRoadMsgCenter::OnSearchTextureHigh()--------!!!\n");

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送特征搜索回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}

 /*************************************************
 *	函数简介：特征比对查询
 *  输入：nSocket: 对应套接字 request:请求消息
 *  返回值：TRUE:特征搜索高级查询成功 FALSE:失败
 **************************************************/
bool CSkpRoadMsgCenter::OnSearchObjectFeature(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uMsgCommandID==(SRIP_SEARCH_OBJECT_FEATURE))
	{
		SEARCH_ITEM search_item;
		memcpy(&search_item,request.c_str()+sizeof(SRIP_HEADER),sizeof(SEARCH_ITEM));

        //for test
        printf("======================Enter GetObjectFeature()--------!!!\n");
		std::string response = g_skpDB.GetObjectFeature(search_item, MAX_TEXTURES);
        //for test
        printf("======================Out GetObjectFeature--------!!!\n");

		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_OBJECT_FEATURE_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

        //for test
        printf("======================CSkpRoadMsgCenter::OnSearchObjectFeature()----response.size()=%d---!!!\n",response.size());

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送比对查询回复数据包失败[%d]\r\n",nSocket);
	}
	return true;
}

bool CSkpRoadMsgCenter::OnAddSpecialCard(const int nSocket,std::string request)
{

	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	SPECIALCARD specialcard_item;
	memcpy(&specialcard_item, request.c_str()+sizeof(SRIP_HEADER), sizeof(SPECIALCARD));

	//返回添加的序号
	int nId = 0;
#ifdef _DEBUG
	printf("======================Enter AddSpecialCard()--------!!!\n");
#endif
	uErrorCode = g_skpDB.AddSpecialCard(specialcard_item, nId);
#ifdef _DEBUG
	printf("======================Out AddSpecialCard()--------!!!\n");
#endif

	std::string response;
	response.append((char*)&nId,sizeof(int));

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_ADD_SPECIALCARD_REP);
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送添加车牌黑白名单回复数据包失败[%d]\r\n",nSocket);

	return true;
}


bool CSkpRoadMsgCenter::OnDeleteSpecialCard(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	SPECIALCARD specialcard;
	memcpy(&specialcard, request.c_str()+sizeof(SRIP_HEADER), request.size()-sizeof(SRIP_HEADER));
#ifdef _DEBUG
		printf("======================Enter OnDeleteSpecialCard()--------!!!\n");
#endif
 	uErrorCode = g_skpDB.DeleteSpecialCard(specialcard);
#ifdef _DEBUG
		printf("======================Out OnDeleteSpecialCard()--------!!!\n");
#endif
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_DELETE_SPECIALCARD_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送删除车牌黑白名单回复数据包失败[%d]\r\n",nSocket);

	return true;
}

bool CSkpRoadMsgCenter::OnModifySpecialCard(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	SPECIALCARD specialcard;
	memcpy(&specialcard,request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));
#ifdef _DEBUG
		printf("======================Enter OnModifySpecialCard()--------!!!\n");
#endif
	uErrorCode = g_skpDB.ModifySpecialCard(specialcard);
#ifdef _DEBUG
		printf("======================Out OnModifySpecialCard()--------!!!\n");
#endif
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_MODIFY_SPECIALCARD_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送修改车牌黑白名单回复数据包失败[%d]\r\n",nSocket);

	return true;
}

bool CSkpRoadMsgCenter::OnSearchSpecialCard(const int nSocket,std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uMsgCommandID == SRIP_SEARCH_SPECIALCARD)
	{
#ifdef _DEBUG
		printf("======================Enter SearchSpecialCard()--------!!!\n");
#endif
		SPECIALCARD specialcard_item;
		memcpy(&specialcard_item, request.c_str()+sizeof(SRIP_HEADER), sizeof(SPECIALCARD));
		std::string response = g_skpDB.SearchSpecialCard(specialcard_item);

#ifdef _DEBUG
		printf("======================Out SearchSpecialCard()--------!!!\n");
#endif
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (SRIP_SEARCH_SPECIALCARD_REP);
		mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();

		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送查询车牌黑白名单回复数据包失败[%d]\r\n",nSocket);
	}

	return true;
}

//获取通道列表
bool CSkpRoadMsgCenter::OnGetChannelList(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	if(sHeader->uMsgCommandID == SRIP_CHANNEL_LIST)
	{
#ifdef _DEBUG
		printf("======================Enter OnGetChannelList-------!!!\n");
#endif
        //送通道列表
        std::string response = g_skpDB.GetChannelList();

        //返回代码
		sHeader->uMsgCode = (uErrorCode);

        //命令
        MIMAX_HEADER mHeader;
        mHeader.uCmdID = (SRIP_CHANNEL_LIST_REP);
        //长度
        mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER) + response.size();

        //加上头
        response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
        response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

        //发送数据
        if(!g_skpRoadServer.SendMsg(nSocket,response))
            LogError("发送通道列表数据包失败[%d]\r\n",nSocket);
    }

#ifdef _DEBUG
		printf("======================Out OnGetChannelList-------!!!\n");
#endif

	return true;
}

//发送显示区域图像命令
bool CSkpRoadMsgCenter::OnRegionImage(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));

	ImageRegion imgRegion;
	memcpy(&imgRegion,request.c_str()+sizeof(SRIP_HEADER)+sizeof(int),sizeof(ImageRegion));

	printf("nChannel=%d,nImageRegionType=%d,x=%d,y=%d,w=%d,h=%d,OnRegionImage\n",nChannel,imgRegion.nImageRegionType,imgRegion.x,imgRegion.y,imgRegion.width,imgRegion.height);

	g_skpChannelCenter.SendRegionImage(nChannel,imgRegion);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_REGION_IMAGE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

    std::string response;
	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送显示区域图像回复数据包失败[%d]\r\n",nSocket);

    return true;
}

 //相机模板
bool CSkpRoadMsgCenter::OnCameraModel(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	CAMERA_CONFIG cfg;
	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));
	memcpy(&cfg,request.c_str()+sizeof(SRIP_HEADER)+sizeof(int),request.size()-sizeof(SRIP_HEADER)-sizeof(int));

    std::string response;
    MIMAX_HEADER mHeader;
	if(sHeader->uMsgCommandID == ADD_CAMERA_MODEL)
	{
	    /////根据通道检测类型修改相机pe和gain的值
        /*CHANNEL_DETECT_KIND uDetectKind = g_skpChannelCenter.GetChannelDetectKind(nChannel);
        if((uDetectKind&DETECT_FLUX)==DETECT_FLUX)
        {
            cfg.nMaxGain2 = cfg.nMaxGain;
            cfg.nMaxPE2 = cfg.nMaxPE;
            cfg.nMaxSH2 = cfg.nMaxSH;
        }*/

		cfg.uCameraID = g_skpChannelCenter.GetChannelCamId(nChannel);

        CXmlParaUtil xml;
        if(xml.AddCameraParaModel(cfg))//模板设置成功后设置相机
        {
            cfg.uType = 1;//设置
            cfg.nIndex = CAMERA_MAXGAIN;
            g_skpChannelCenter.CameraControl(nChannel,cfg);
        }
        mHeader.uCmdID = ADD_CAMERA_MODEL_REP;
	}
	else if(sHeader->uMsgCommandID == LOAD_CAMERA_MODEL)
	{
		cfg.uCameraID = g_skpChannelCenter.GetChannelCamId(nChannel);

	    if(cfg.uType == 0)//载入当前模板
	    {
            CXmlParaUtil xml;
            xml.LoadCameraParaModel(cfg,false);
	    }
	    else if(cfg.uType == 3)//载入默认模板
	    {
            g_skpChannelCenter.CameraControl(nChannel,cfg);
	    }
        /////根据通道检测类型修改相机pe和gain的值
        CHANNEL_DETECT_KIND uDetectKind = g_skpChannelCenter.GetChannelDetectKind(nChannel);
        if(((uDetectKind&DETECT_CARNUM)!=DETECT_CARNUM)&& (uDetectKind != DETECT_NONE))
        {
            cfg.nMaxGain = cfg.nMaxGain2;
            cfg.nMaxPE = cfg.nMaxPE2;
            cfg.nMaxSH = cfg.nMaxSH2;
        }

        mHeader.uCmdID = LOAD_CAMERA_MODEL_REP;
        response.append((char*)&cfg,sizeof(cfg));
	}

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

    //长度
    mHeader.uCmdLen = sizeof(SRIP_HEADER)+sizeof(MIMAX_HEADER)+response.size();


	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送相机模板回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//设置ip地址
bool CSkpRoadMsgCenter::OnIpSetup(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
    SYSTEM_CONFIG sys_config;
    memcpy(&sys_config,request.c_str()+sizeof(SRIP_HEADER),sizeof(SYSTEM_CONFIG));

    std::string strIpAddress(sys_config.chServerHost);
    std::string strNetMask(sys_config.chNetMask);
    std::string strGateWay(sys_config.chGateWay);
    std::string strCameraHost(sys_config.chCameraHost);
	std::string strCameraNetMask(sys_config.chCameraNetMask);

    if( (strIpAddress.size() > 8) && (sys_config.uNetMTU > 0))
    {
        g_ServerHost = strIpAddress;
        g_strNetMask = strNetMask;
        g_strGateWay = strGateWay;
        g_CameraHost = strCameraHost;
		g_strCameraNetMask = strCameraNetMask;

        if(sys_config.uNetMTU < 500)
        sys_config.uNetMTU = 1500;
        else  if(sys_config.uNetMTU > 2000)
        sys_config.uNetMTU = 1500;
        g_uNetMTU = sys_config.uNetMTU;
		
		if(sys_config.uCameraNetMTU == 0)
		{
			sys_config.uCameraNetMTU = g_uCameraNetMTU;
		}
		else if(sys_config.uCameraNetMTU < 500)
		{
			sys_config.uCameraNetMTU = 1500;
		}
        else  if(sys_config.uCameraNetMTU > 2000)
		{
			sys_config.uCameraNetMTU = 9000;
		}
		g_uCameraNetMTU = sys_config.uCameraNetMTU;

        printf("strIpAddress=%s,strNetMask=%s,strGateWay=%s,strCameraHost=%s\n",strIpAddress.c_str(),strNetMask.c_str(),strGateWay.c_str(),strCameraHost.c_str());
		
		if(strCameraHost.size() <= 8)
		{
			strCameraHost = "192.168.0.50";
		}

		if(strCameraNetMask.size() <= 8)
		{
			strCameraNetMask = "255.255.255.0";
		}
		
		FILE* fp = fopen("./shell/ipSet.sh","wb");
		fprintf(fp,"#!/bin/bash\ntouch interfaces\necho \"auto lo\" >> interfaces\necho \"iface lo inet loopback\" >> interfaces\n"\
						   "echo \"auto eth0\" >> interfaces\necho \"iface eth0 inet static\" >> interfaces\necho \"address $4\" >> interfaces\n"\
						   "echo \"netmask $6\" >> interfaces\necho \"mtu $7\" >> interfaces\necho \"auto eth1\" >> interfaces\necho \"iface eth1 inet static\" >> interfaces\necho \"address $1\" >> interfaces\n"\
						   "echo \"netmask $2\" >> interfaces\necho \"gateway $3\" >> interfaces\necho \"mtu $5\" >> interfaces\nmv interfaces /etc/network/interfaces\n"\
						   "/etc/init.d/networking restart\nip ro add 224.0.0.0/4 dev eth0 scope global\nsysctl -w net.core.rmem_default=40000000\nsysctl -w net.core.rmem_max=80000000\n");
		fclose(fp);

        char   cmdline[256]={0};
		sprintf(cmdline,"chmod 777 ./shell/ipSet.sh");
		system(cmdline);

        sprintf(cmdline,"%s %s %s %s %s %d %s %d","./shell/ipSet.sh",strIpAddress.c_str(),strNetMask.c_str(),strGateWay.c_str(),strCameraHost.c_str(),sys_config.uNetMTU,strCameraNetMask.c_str(),sys_config.uCameraNetMTU);

        printf("cmdline=%s\n",cmdline);
        system(cmdline);
        sync();
    }

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SET_IP_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送ip配置参数返回数据包失败[%d]\r\n",nSocket);
	return true;
}

//删除测试程序
bool CSkpRoadMsgCenter::OnDeleteTest(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //杀死测试程序
    KillProcess("testcarnum.out");
    //删除测试程序目录
    char   cmdline[256]={0};
    sprintf(cmdline,"rm -rf ./testcarnum.out");
    system(cmdline);

    sprintf(cmdline,"rm -rf ./testusbkey.out");
    system(cmdline);

    /*sprintf(cmdline,"rm -rf ./*.txt");
    system(cmdline);*/

    sprintf(cmdline,"rm -rf /etc/rc2.d/S99testcarnum");
    system(cmdline);

    //删除测试视频文件
    std::string strVideoPath;
	if (IsDataDisk())
	{
		strVideoPath = "/detectdata/vod2009_04_09_18_03_40.yuv";
	}
	else
	{
		strVideoPath = "../vod2009_04_09_18_03_40.yuv";
	}
    if(access(strVideoPath.c_str(),F_OK) == 0)
    {
        FILE* fp = fopen(strVideoPath.c_str(),"wb");
		if(fp)
		{
			fwrite("",0,1,fp);
			fclose(fp);
		}
    }

	//删除测试视频文件
	if (IsDataDisk())
	{
		strVideoPath = "/detectdata/test.yuv";
	}
	else
	{
		strVideoPath = "../test.yuv";
	}
	if(access(strVideoPath.c_str(),F_OK) == 0)
	{
		FILE* fp = fopen(strVideoPath.c_str(),"wb");
		if(fp)
		{
			fwrite("",0,1,fp);
			fclose(fp);
		}
	}

    MIMAX_HEADER mHeader;
	mHeader.uCmdID = (DELETE_TEST_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

    std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送删除测试程序返回数据包失败[%d]\r\n",nSocket);
    return true;
}

bool CSkpRoadMsgCenter::SetSysClockForSHJJ()
{
	if( g_nClockMode == 1)
	{
		return false;
	}
	char strRoadId[256] = {"./shell/RoadId.txt"};
	{
		char   buf[1024]={0};
		sprintf(buf,"%s",g_strDetectorID.c_str());
		FILE* fp = fopen(strRoadId,"wb");
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
	}
	char strChkntpFileName[256] = {"./shell/chkntp.sh"};
	{
		char   buf[1024]={0};
		sprintf(buf,"#!/bin/sh\n" \
			"time=$(date -d \"now\" +\"%%H%%M\")\n" \
			"minus=$(date -d \"now\" +\"%%M\")\n" \
			"localtime=$(date -d \"now\" +\"%%Y-%%m-%%d %%H:%%M:%%S\")\n" \
			"if [ $time -eq %04d ];then\n" \
			"  echo $localtime>/tmp/ntp2.log\n" \
			"  /usr/sbin/ntpdate ntphost\n" \
			"fi\n" \
			"if [ $time -eq %04d ];then\n" \
			"  echo $localtime>/tmp/ntp2.log\n" \
			"  /usr/sbin/ntpdate ntphost\n" \
			"fi\n" \
			"if [ $minus -eq %02d ];then\n" \
			"  echo $localtime>/tmp/ntp2.log\n" \
			"  /usr/sbin/ntpdate ntphost\n" \
			"fi\n",g_Ntp_Time.nTime1,g_Ntp_Time.nTime2,g_Ntp_Time.nTime3);

		FILE* fp = fopen(strChkntpFileName,"wb");
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
		sprintf(buf, "chmod 777 %s", strChkntpFileName);
		system(buf);
	}
	char strNtpstartFileName[256] = {"./shell/ntpstart.sh"};
	{
		char   buf[1024]={0};
		sprintf(buf,"#!/bin/sh\n" \
			"/etc/chkntp.sh > /tmp/ntp.log 2>&1\n\n" \
			"checkprocess()\n{\n" \
			"  if [ \"$1\" = \"\" ];then\n" \
			"    return 0\n" \
			"  fi\n" \
			"  process_num=`ps -ef |grep \"$1\" |grep -v \"grep\" |wc -l`\n" \
			"  if [ $process_num -eq 0 ];\n\t\tthen\n" \
			"      return 0\n" \
			"    else\n" \
			"      return 1\n" \
			"  fi\n}\n\n" \
			"if [ -f \"/tmp/ntp2.log\" ];then\n" \
			"checkprocess \"sedlog\"\n" \
			"check_result=$?\n" \
			"if [ $check_result -eq 0 ];then\n" \
			"  /etc/sedlog.sh\nfi\n" \
			"else\nkillall -9 sedlog.sh\nfi\n");

		FILE* fp = fopen(strNtpstartFileName,"wb");
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
		sprintf(buf, "chmod 777 %s", strNtpstartFileName);
		system(buf);
	}
	char strSedlogFileName[256] = {"./shell/sedlog.sh"};
	{
		FILE* fp = fopen(strSedlogFileName,"wb");
		char   buf[1024]={0};
		sprintf(buf,"#!/bin/sh\n" \
			"if ! [ -d \"/detectdata/mysql\" ]; then\n"
			"  txtdir=\"/home/road\"\n" \
			"else\n  txtdir=\"/detectdata\"\nfi\n" \
			"detect=$txtdir\"/dzjc\"\n" \
			"if ! [ -d \"$detect\" ];then\n" \
			"  mkdir \"$detect\"\nfi\n\n" \
			"ntplog=$txtdir\"/dzjc/tsyn\"\n" \
			"if ! [ -d \"$ntplog\" ];then\n" \
			"  mkdir \"$ntplog\"\nfi\n\n" \
			"  roadid=$(sed -n '1p' /etc/RoadId.txt)\n" \
			"filename=$ntplog/$roadid\"_NTP_\"$(date -d \"now\" +\"\%Y%%m\").txt\n" \
			"title=\"LocalTime\\t\\tFlag\\t\\tNTPTime\\t\\t\\tNTPLocal\\t\\tOffSet\"\n");
		buf[strlen(buf)] ='\0';
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
		sprintf(buf,"nLoop=0\nwhile true;do\n" \
			"if [ -f \"/tmp/ntp2.log\" ];then\n" \
			"  bcheck=$(grep server /tmp/ntp.log |wc -c)\n" \
			"  if [ $bcheck -eq 0 ];then\n" \
			"    nLoop=$(($nLoop+1))\n" \
			"    if [ $nLoop -eq 12 ];then\n" \
			"       break;\n" \
			"    fi\n" \
			"    sleep 5\n" \
			"    continue\n" \
			"  else\n" \
			"    nLine=$(more /tmp/ntp.log |wc -l)\n" \
			"    if [ $nLine -eq 2 ];then \n" \
			"      sed -i '2d' /tmp/ntp.log\n" \
			"    fi\n\n" \
			"    localtime=$(sed -n '1p' /tmp/ntp2.log)\n" \
			"    if ! [ -e $filename ]; then\n");
		buf[strlen(buf)] ='\0';
		fp = fopen(strSedlogFileName,"a+");
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
		sprintf(buf,"      echo $title > $filename\n" \
			"    fi\n\n" \
			"    bflag=$(grep sec /tmp/ntp.log |wc -c)\n" \
			"    if [ $bflag -eq 0 ];then\n" \
			"      echo $localtime\"\\t\"\"false\" >> $filename\n" \
			"    else\n" \
			"       ntptime=$(date -d \"now\" +\"%%Y-%%m-%%d \")$(sed -rn 's/.*(.{9})ntpdate.*/\\%d/p' /tmp/ntp.log)\n" \
			"       ntplocal=$(date -d \"now\" +\"%%Y-%%m-%%d %%H:%%M:%%S\")\n" \
			"       offset=$(sed -r 's/.*offset (.*)/\\%d/' /tmp/ntp.log|sed -e 's/sec//')\n" \
			"       num=\"-1000000\"\n" \
			"       offset1=`echo \"scale=0;$offset*$num\"|bc`\n\n"
			"       first=`echo \"$offset\"|cut -c 1`\n" \
			"       if [ \"$first\"x = \"-\"x ];then\n" \
			"          offset=`echo \"$offset\"|sed 's/-//'`\n" \
			"       else\n" \
			"          offset=\"-\"$offset\n" \
			"       fi\n\n" \
			"       time1=$(date +%%s -d \"$ntptime\")\n" \
			"       offset1=${offset1%%.*}\n" \
			"       time2=$((($time1*1000000+$offset1)/1000000))\n" \
			"       localtime2=$(date +%%Y-%%m-%%d\\ %%H:%%M:%%S -d \"1970-01-01 UTC $time2 seconds\")\n\n" \
			"       echo $localtime2\"\\t\"\"true\\t\\t\"$ntptime\"\\t\"$ntplocal\"\\t\"$offset >> $filename\n" \
			"    fi\n\n" \
			"    rm /tmp/ntp.log\n" \
			"    rm /tmp/ntp2.log\n" \
			"    break\n" \
			"  fi\nfi\ndone\n",1,1);
		buf[strlen(buf)] ='\0';
		fp = fopen(strSedlogFileName,"a+");
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
		sprintf(buf, "chmod 777 %s", strSedlogFileName);
		system(buf);
	}
	char strNtpdateFileName[256] = {"./shell/ntpdate.sh"};
	{
		char   buf[1024]={0};
		sprintf(buf,"#!/bin/bash\nsed s#ntphost#$1#g ./shell/RoadId.txt > /etc/RoadId.txt\nsed s#ntphost#$1#g ./shell/chkntp.sh > /etc/chkntp.sh\nchmod 777 /etc/chkntp.sh\nsed s#ntphost#$1#g ./shell/ntpstart.sh > /etc/ntpstart.sh\nchmod 777 /etc/ntpstart.sh\nsed s#ntphost#$1#g ./shell/sedlog.sh > /etc/sedlog.sh\nchmod 777 /etc/sedlog.sh\nsed -i /chkntp/d /etc/crontab\nsed -i /ntpstart/d /etc/crontab\nsed '$ i*/1 *  * * *   root    /etc/ntpstart.sh' -i /etc/crontab\n");
		FILE* fp = fopen(strNtpdateFileName,"wb");
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
		sprintf(buf, "chmod 777 %s", strNtpdateFileName);
		system(buf);
	}
	if(access(strNtpdateFileName,F_OK) == 0)
	{
		char   cmdline[256]={0};
		sprintf(cmdline,"%s %s",strNtpdateFileName,g_strSynClockHost.c_str());
		system(cmdline);
		sync();

		char   buf[256]={0};
		sprintf(buf,"/usr/sbin/ntpdate %s",g_strSynClockHost.c_str());
		system(buf);
		printf("=====strSynClockHost.c_str()=%s\n",cmdline);
	}
	return true;
}

//时钟同步设置
bool CSkpRoadMsgCenter::OnSysClockSetup(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    SYSTEM_CONFIG sys_config;
    memcpy(&sys_config,request.c_str()+sizeof(SRIP_HEADER),sizeof(SYSTEM_CONFIG));

    std::string strSynClockHost(sys_config.chSynClockHost);
    g_strSynClockHost = strSynClockHost;

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("OtherSetting","SynClockHost");


    MIMAX_HEADER mHeader;
    mHeader.uCmdID = SYSCLOCK_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送时钟同步回复数据包失败[%d]\r\n",nSocket);

    if( g_nClockMode == 0)
    {
		char strChkntpFileName[256] = {"./shell/chkntp.sh"};
		//if(access(strChkntpFileName,F_OK) != 0)
		{
			char   buf[256]={0};
			sprintf(buf,"#!/bin/sh\n/usr/sbin/ntpdate ntphost\n");
			FILE* fp = fopen(strChkntpFileName,"wb");
			if(fp)
			{
				fwrite(buf,1,256,fp);
				fclose(fp);
			}
			sprintf(buf, "chmod 777 %s", strChkntpFileName);
			system(buf);
		}

		char strNtpdateFileName[256] = {"./shell/ntpdate.sh"};
		//if(access(strNtpdateFileName,F_OK) != 0)
		{
			char   buf[512]={0};
			sprintf(buf,"#!/bin/bash\nsed s#ntphost#$1#g ./shell/chkntp.sh > /etc/chkntp.sh\nchmod 777 /etc/chkntp.sh\nsed -i /ntpstart/d /etc/crontab\nsed -i /chkntp/d /etc/crontab\nsed '$ i*/5 *  * * *   root    /etc/chkntp.sh > /tmp/ntp.log 2>&1' -i /etc/crontab\n");
			FILE* fp = fopen(strNtpdateFileName,"wb");
			if(fp)
			{
				fwrite(buf,1,512,fp);
				fclose(fp);
			}
			
			sprintf(buf, "chmod 777 %s", strNtpdateFileName);
			system(buf);
		}
		
		//if(access(strNtpdateFileName,F_OK) == 0)
		{
			char   cmdline[256]={0};
			sprintf(cmdline,"%s %s",strNtpdateFileName,strSynClockHost.c_str());
			system(cmdline);
			sync();
		
			char   buf[256]={0};
			sprintf(buf,"/usr/sbin/ntpdate %s",strSynClockHost.c_str());
			system(buf);
			system("hwclock --systohc");
			printf("=====strSynClockHost.c_str()=%s\n",cmdline);
		}
    }
    else
    {
        if(g_GpsComSetting.nComUse == 6)
        {
			if(g_GpsSetInfo.nType == 0)
			{
				g_GpsSerial.OpenDev();
			 }
			else
			{
				 g_GpsChangeTime.OpenDev();
			}

        }
    }


    return true;
}

//串口设置
bool CSkpRoadMsgCenter::OnSysComSetting(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
	    COM_PARAMETER_MAP mapComSetting;
	    int nSize = (request.size() - sizeof(SRIP_HEADER))/sizeof(COM_PARAMETER);

	    for(int i =0;i<nSize;i++)
	    {
	        COM_PARAMETER com_para;
	        memcpy(&com_para,request.c_str()+sizeof(SRIP_HEADER)+i*sizeof(COM_PARAMETER),sizeof(COM_PARAMETER));
            mapComSetting.insert(COM_PARAMETER_MAP::value_type(com_para.nComPort,com_para));

            if(com_para.nComUse == 1)
            {
                if(g_CameraComSetting.nComPort != com_para.nComPort||
                   g_CameraComSetting.nBaud != com_para.nBaud ||
                   g_CameraComSetting.nDataBits != com_para.nDataBits ||
                   g_CameraComSetting.nStopBits != com_para.nStopBits ||
                   g_CameraComSetting.nParity != com_para.nParity)
                {
                    //if(g_CameraSerialComm.IsOpen())//重新打开串口
                    {
                        g_CameraSerialComm.Close();
                        g_CameraSerialComm.OpenDev();
                    }
                }
            }
            else if(com_para.nComUse == 2)
            {
                if(g_RedLightComSetting.nComPort != com_para.nComPort||
                   g_RedLightComSetting.nBaud != com_para.nBaud ||
                   g_RedLightComSetting.nDataBits != com_para.nDataBits ||
                   g_RedLightComSetting.nStopBits != com_para.nStopBits ||
                   g_RedLightComSetting.nParity != com_para.nParity)
                {
                    //if(r_SerialComm.IsOpen())//重新打开串口
                    {
                        r_SerialComm.Close();
                        r_SerialComm.OpenDev();
                    }
                }
            }
            else if(com_para.nComUse == 3)
            {
                if(g_DHComSetting.nComPort != com_para.nComPort||
                   g_DHComSetting.nBaud != com_para.nBaud ||
                   g_DHComSetting.nDataBits != com_para.nDataBits ||
                   g_DHComSetting.nStopBits != com_para.nStopBits ||
                   g_DHComSetting.nParity != com_para.nParity)
                {
                    //if(d_SerialComm.IsOpen())//重新打开串口
                    {
                        d_SerialComm.Close();
                        d_SerialComm.OpenDev();
                    }
                }
            }
            else if(com_para.nComUse == 4)
            {
                if(g_LightComSetting.nComPort != com_para.nComPort||
                   g_LightComSetting.nBaud != com_para.nBaud ||
                   g_LightComSetting.nDataBits != com_para.nDataBits ||
                   g_LightComSetting.nStopBits != com_para.nStopBits ||
                   g_LightComSetting.nParity != com_para.nParity)
                {
                    //if(L_SerialComm.IsOpen())//重新打开串口
                    {
                        L_SerialComm.Close();
                        L_SerialComm.OpenDev();
                    }
                }
            }
            else if(com_para.nComUse == 5)
            {
                if(g_VisComSetting.nComPort != com_para.nComPort||
                   g_VisComSetting.nBaud != com_para.nBaud ||
                   g_VisComSetting.nDataBits != com_para.nDataBits ||
                   g_VisComSetting.nStopBits != com_para.nStopBits ||
                   g_VisComSetting.nParity != com_para.nParity)
                {
                    //if(g_VisSerialCommunication.IsOpen())//重新打开串口
                    {
                        g_VisSerialCommunication.Close();
                        g_VisSerialCommunication.OpenDev();
                    }
                }
            }
            else if(com_para.nComUse == 6)
            {
                if(g_GpsComSetting.nComPort != com_para.nComPort||
                   g_GpsComSetting.nBaud != com_para.nBaud ||
                   g_GpsComSetting.nDataBits != com_para.nDataBits ||
                   g_GpsComSetting.nStopBits != com_para.nStopBits ||
                   g_GpsComSetting.nParity != com_para.nParity)
                {
                    //if(g_GpsSerial.IsOpen())//重新打开串口
                    {
						if(g_GpsSetInfo.nType == 0)
						{ 
							g_GpsSerial.Close();
							g_GpsSerial.OpenDev();
						}
						else
						{
							g_GpsChangeTime.Close();
							g_GpsChangeTime.OpenDev();
						} 
                 
                    }
                }
            }
            else if(com_para.nComUse == 7 || com_para.nComUse == 13) //S3雷达,慧昌雷达
            {
                if(g_RadarComSetting.nComPort != com_para.nComPort||
                   g_RadarComSetting.nBaud != com_para.nBaud ||
                   g_RadarComSetting.nDataBits != com_para.nDataBits ||
                   g_RadarComSetting.nStopBits != com_para.nStopBits ||
                   g_RadarComSetting.nParity != com_para.nParity)
                {
                    //if(g_RadarSerial.IsOpen())//重新打开串口
                    {
						LogNormal("Open Dev g_RadarSerial %d,%d,%d,%d,%d", \
							g_RadarComSetting.nComPort, g_RadarComSetting.nBaud, g_RadarComSetting.nDataBits, g_RadarComSetting.nStopBits, g_RadarComSetting.nParity);
                        g_RadarSerial.Close();
                        g_RadarSerial.OpenDev();
                    }
                }
            }
            else if(com_para.nComUse == 8) //爆闪灯
            {
                if(g_FlashComSetting.nComPort != com_para.nComPort||
                   g_FlashComSetting.nBaud != com_para.nBaud ||
                   g_FlashComSetting.nDataBits != com_para.nDataBits ||
                   g_FlashComSetting.nStopBits != com_para.nStopBits ||
                   g_FlashComSetting.nParity != com_para.nParity)
                {
                    {
                        //g_RadarSerial.Close();
                        //g_RadarSerial.OpenDev();
                    }
                }
            }			
			else{}
	    }
	    g_mapComSetting = mapComSetting;

        CXmlParaUtil xml;
        xml.UpdateSystemSetting("ComSetting","");
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SYS_COM_SETTING_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送系统配置参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//球机及云台控制参数设置
bool CSkpRoadMsgCenter::OnSysYunTaiSetting(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
	    YUNTAI_CONTROL_PARAMETER ytControlSetting;
        memcpy(&ytControlSetting,request.c_str()+sizeof(SRIP_HEADER),sizeof(YUNTAI_CONTROL_PARAMETER));
        g_ytControlSetting = ytControlSetting;

        g_nMultiPreSet = ytControlSetting.nMultiPreSet;
        g_nControlMode = ytControlSetting.nControlMode;
        g_nKeyBoardID = ytControlSetting.nKeyBoardID;
        string strVisHost(ytControlSetting.szVisHost);
        g_strVisHost = strVisHost;
        g_nVisPort = ytControlSetting.nVisPort;
		g_nPreSetMode = ytControlSetting.nPreSetMode;

        CXmlParaUtil xml;
        xml.UpdateSystemSetting("YunTaiSetting","");
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SYS_YUNTAI_SETTING_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送系统配置参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}


//监控主机参数设置
bool CSkpRoadMsgCenter::OnSysMonitorHostSetting(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
	    MONITOR_HOST_INFO monitorHostInfo;
        memcpy(&monitorHostInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(MONITOR_HOST_INFO));
        g_monitorHostInfo = monitorHostInfo;

        /*FILE* fp = fopen("request.txt","rb");
        fwrite(request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER),1,fp);
        fclose(fp);*/
        printf("monitorHostInfo.chUserName =%s,monitorHostInfo.chPassWord=%s,request=%s\n",monitorHostInfo.chUserName,monitorHostInfo.chPassWord,request.c_str()+sizeof(SRIP_HEADER));

        CXmlParaUtil xml;
        xml.UpdateSystemSetting("MonitorHostSetting","");
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SYS_MONITORHOST_SETTING_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送监控主机参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//系统参数模板设置
bool CSkpRoadMsgCenter::OnSysModelSetting(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
    request.erase(0,sizeof(SRIP_HEADER));
    SYSTEM_CONFIG_EX sys_config_ex;
    memcpy(&sys_config_ex,request.c_str(),sizeof(SYSTEM_CONFIG_EX));

    printf("==OnSysModelSetting sys_config_ex.nModelID=%d\n",sys_config_ex.nModelID);
    //获取模板ID
    g_nSettingModelID = sys_config_ex.nModelID;
    //载入系统设置模板
    LoadSysModel(g_nSettingModelID);

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("OtherSetting","");
	xml.UpdateSystemSetting("PicFormatInfo","");
	xml.UpdateSystemSetting("VideoFormatInfo","");


	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SYS_SETTING_MODEL_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送系统模板配置参数返回数据包失败[%d]！\r\n",nSocket);

	return true;
}

//智能控制器主机参数设置
bool CSkpRoadMsgCenter::OnExpoMonitorSetting(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
	    EXPO_MONITOR_INFO ExpoMonitorInfo;
        memcpy(&ExpoMonitorInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(EXPO_MONITOR_INFO));
        g_ExpoMonitorInfo = ExpoMonitorInfo;

        CXmlParaUtil xml;
        xml.UpdateSystemSetting("ExpoMonitorSetting","");
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (EXPO_MONITOR_SETTING_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送智能控制主机参数返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//图片格式信息设置
bool CSkpRoadMsgCenter::OnPicFormatSetting(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
	    PIC_FORMAT_INFO PicFormatInfo;
        memcpy(&PicFormatInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(PIC_FORMAT_INFO));
        g_PicFormatInfo = PicFormatInfo;

        CXmlParaUtil xml;
        xml.UpdateSystemSetting("PicFormatInfo","");
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (PIC_FORMAT_SETTING_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送图片格式信息设置返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//录像格式信息设置
bool CSkpRoadMsgCenter::OnVideoFormatSetting(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
	    VIDEO_FORMAT_INFO VideoFormatInfo;
        memcpy(&VideoFormatInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(VIDEO_FORMAT_INFO));
        g_VideoFormatInfo = VideoFormatInfo;

        g_nAviHeaderEx = g_VideoFormatInfo.nAviHeaderEx;
        g_nSendRTSP = g_VideoFormatInfo.nSendRtsp;
        g_nEncodeFormat = g_VideoFormatInfo.nEncodeFormat;

        if(g_VideoFormatInfo.nFrameRate == 0)
            g_fFrameRate = 1;
        else if(g_VideoFormatInfo.nFrameRate == 1)
            g_fFrameRate = 2.5;
        else if(g_VideoFormatInfo.nFrameRate == 2)
            g_fFrameRate = 5;
        else if(g_VideoFormatInfo.nFrameRate == 3)
            g_fFrameRate = 7.5;
        else if(g_VideoFormatInfo.nFrameRate == 4)
            g_fFrameRate = 10;
        else if(g_VideoFormatInfo.nFrameRate == 5)
            g_fFrameRate = 12.5;
        else if(g_VideoFormatInfo.nFrameRate == 6)
            g_fFrameRate = 15;


                        if(g_VideoFormatInfo.nResolution == 0)
                        {
                            g_nVideoWidth = 400;
                            g_nVideoHeight = 300;
                        }
                        else if(g_VideoFormatInfo.nResolution == 1)
                        {
                            g_nVideoWidth = 480;
                            g_nVideoHeight = 270;
                        }
                        else if(g_VideoFormatInfo.nResolution == 2)
                        {
                            g_nVideoWidth = 600;
                            g_nVideoHeight = 450;
                        }
                        else if(g_VideoFormatInfo.nResolution == 3)
                        {
                            g_nVideoWidth = 640;
                            g_nVideoHeight = 360;
                        }
                        else if(g_VideoFormatInfo.nResolution == 4)
                        {
                            g_nVideoWidth = 800;
                            g_nVideoHeight = 600;
                        }
                        else if(g_VideoFormatInfo.nResolution == 5)
                        {
                            g_nVideoWidth = 960;
                            g_nVideoHeight = 540;
                        }
                        else if(g_VideoFormatInfo.nResolution == 6)
                        {
                            g_nVideoWidth = 1000;
                            g_nVideoHeight = 750;
                        }
                        else if(g_VideoFormatInfo.nResolution == 7)
                        {
                            g_nVideoWidth = 1280;
                            g_nVideoHeight = 720;
                        }
                        else if(g_VideoFormatInfo.nResolution == 8)
                        {
                            g_nVideoWidth = 1200;
                            g_nVideoHeight = 900;
                        }
                        else if(g_VideoFormatInfo.nResolution == 9)
                        {
                            g_nVideoWidth = 1600;
                            g_nVideoHeight = 900;
                        }
                        else if(g_VideoFormatInfo.nResolution == 10)
                        {
                            g_nVideoWidth = 1600;
                            g_nVideoHeight = 1200;
                        }
                        else if(g_VideoFormatInfo.nResolution == 11)
                        {
                            g_nVideoWidth = 1920;
                            g_nVideoHeight = 1080;
                        }

        CXmlParaUtil xml;
        xml.UpdateSystemSetting("VideoFormatInfo","");
	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (VIDEO_FORMAT_SETTING_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送录像格式信息设置返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//获取版本号
bool CSkpRoadMsgCenter::OnVerSion(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_VERSION_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+g_strVersion.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.append((char*)g_strVersion.c_str(),g_strVersion.size());	//发送数据

	printf("OnVerSion response.c_str()=%s,response.size()=%d",response.c_str(),response.size());
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送版本信息返回数据包失败[%d]！\r\n",nSocket);
	return true;
}

//认证服务器设置
bool CSkpRoadMsgCenter::OnAuthenticationSetup(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    SYSTEM_CONFIG sys_config;
    memcpy(&sys_config,request.c_str()+sizeof(SRIP_HEADER),sizeof(SYSTEM_CONFIG));

    std::string strAuthenticationHost(sys_config.chAuthenticationHost);
    g_strAuthenticationHost = strAuthenticationHost;

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("OtherSetting","AuthenticationHost");


    MIMAX_HEADER mHeader;
    mHeader.uCmdID = AUTHENTICATION_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送认证服务器设置回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//中心控制服务器设置
bool CSkpRoadMsgCenter::OnControlServerSetup(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    SYSTEM_CONFIG_EX sys_config_ex;
    memcpy(&sys_config_ex,request.c_str()+sizeof(SRIP_HEADER),sizeof(SYSTEM_CONFIG_EX));

    std::string strControlServerHost (sys_config_ex.chControlServerHost);
    g_strControlServerHost = strControlServerHost;

    g_nControlServerPort = sys_config_ex.uControlServerPort;

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("OtherSetting","ControlServerHost");
    xml.UpdateSystemSetting("OtherSetting","ControlServerPort");
	

    MIMAX_HEADER mHeader;
    mHeader.uCmdID = CONTROL_SERVER_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送中心控制服务器设置回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//ftp服务器设置
bool CSkpRoadMsgCenter::OnFtpServerSetup(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    FTP_HOST_INFO ftp_info;
    memcpy(&ftp_info,request.c_str()+sizeof(SRIP_HEADER),sizeof(FTP_HOST_INFO));

    std::string strFtpServerHost(ftp_info.chFtpServerHost);
    g_strFtpServerHost = strFtpServerHost;

    std::string strFtpUserName(ftp_info.chFtpUserName);
    g_strFtpUserName = strFtpUserName;

    std::string strFtpPassword(ftp_info.chFtpPassword);
    g_strFtpPassWord = strFtpPassword;

    g_nFtpPort = ftp_info.uFtpPort;
    printf("===g_strFtpServerHost=%s\n",g_strFtpServerHost.c_str());

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("OtherSetting","FtpServerHost");
    xml.UpdateSystemSetting("OtherSetting","FtpUserName");
    xml.UpdateSystemSetting("OtherSetting","FtpPassWord");
    xml.UpdateSystemSetting("OtherSetting","FtpPort");

    request.erase(0,sizeof(SRIP_HEADER)+sizeof(FTP_HOST_INFO));
    if(request.size() > 0)
    {
        memcpy(g_ftpRemotePath,request.c_str(),sizeof(g_ftpRemotePath));
        xml.UpdateSystemSetting("OtherSetting","FtpRemotePath");
    }


    MIMAX_HEADER mHeader;
    mHeader.uCmdID = FTP_SERVER_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送FTP服务器设置回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//比对服务器设置
bool CSkpRoadMsgCenter::OnMatchHostSetup(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    MATCH_HOST_INFO MatchHostInfo;
    memcpy(&MatchHostInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(MATCH_HOST_INFO));
    g_MatchHostInfo = MatchHostInfo;

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("MatchHostSetting","");


    MIMAX_HEADER mHeader;
    mHeader.uCmdID = MATCH_HOST_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送比对服务器设置回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//应用管理服务器设置
bool CSkpRoadMsgCenter::OnAmsHostSetup(const int nSocket, std::string request)
{
	UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	AMS_HOST_INFO AmsHostInfo;
	memcpy(&AmsHostInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(AMS_HOST_INFO));
	g_AmsHostInfo = AmsHostInfo;

	CXmlParaUtil xml;
	xml.UpdateSystemSetting("AmsHostSetting","");


	MIMAX_HEADER mHeader;
	mHeader.uCmdID = AMS_HOST_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送应用管理服务器设置回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//区间测速主机设置
bool CSkpRoadMsgCenter::OnDistanceHostSetup(const int nSocket, std::string request)
{
	UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	DISTANCE_HOST_INFO DistanceHostInfo;
	memcpy(&DistanceHostInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(DISTANCE_HOST_INFO));
	g_DistanceHostInfo = DistanceHostInfo;

	CXmlParaUtil xml;
	xml.UpdateSystemSetting("DistanceHostSetting","");


	MIMAX_HEADER mHeader;
	mHeader.uCmdID = DISTANCE_HOST_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送区间测速主机设置回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//gps设置
bool CSkpRoadMsgCenter::OnGpsSetup(const int nSocket, std::string request)
{
	UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	GPS_SET_INFO GpsSetInfo;
	memcpy(&GpsSetInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(GPS_SET_INFO));
	g_GpsSetInfo = GpsSetInfo;

	CXmlParaUtil xml;
	xml.UpdateSystemSetting("GpsSetting","");


	MIMAX_HEADER mHeader;
	mHeader.uCmdID = GPS_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送gps设置回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//信号机设置
bool CSkpRoadMsgCenter::OnSignalSetup(const int nSocket, std::string request)
{
	UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	SIGNAL_SET_INFO SignalSetInfo;
	memcpy(&SignalSetInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(SIGNAL_SET_INFO));
	g_SignalSetInfo = SignalSetInfo;

	CXmlParaUtil xml;
	xml.UpdateSystemSetting("SignalSetting","");


	MIMAX_HEADER mHeader;
	mHeader.uCmdID = SIGNAL_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送信号机设置回复数据包失败[%d]\r\n",nSocket);

	return true;
}



//通道图片格式设置
bool CSkpRoadMsgCenter::OnPicFormatSetup(const int nSocket, std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
     //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    	//判断数据长度, 如果只发送了数据头，则为取配置显示
	if(request.size() == sizeof(SRIP_HEADER))
	{
	    int nChannel = sHeader->uMsgSource;

		//取雷达参数设置
		std::string response;

        CXmlParaUtil xml;
		xml.GetPicFormatInfo(nChannel,response);

		printf("======OnPicFormatSetup=====response=%d\n",response.size());

		MIMAX_HEADER mHeader;
		mHeader.uCmdID = (CHANNEL_PICFORMAT_SET_REP);
		//长度
		mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
		//返回代码
		sHeader->uMsgCode = (uErrorCode);

		//协议头
		response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
		response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

		//发送数据
		if(!g_skpRoadServer.SendMsg(nSocket,response))
			LogError("发送通道图片格式设置回复数据包失败[%d]\r\n",nSocket);
		return true;
	}

	//设置参数
	if(uErrorCode == SRIP_OK)
	{
        std::string strPicFormatInfo;
        strPicFormatInfo.append((char*)(request.c_str()+sizeof(SRIP_HEADER)),request.size() - sizeof(SRIP_HEADER));

        int nChannel = sHeader->uMsgSource;
        CXmlParaUtil xml;
        bool bRet = xml.SetPicFormatInfo(nChannel,strPicFormatInfo);

        if(!bRet)
        {
            uErrorCode = SRIP_ERROR_USER_FAILE;
        }

		/*bool bRet1 = xml.SetRegionRoadCodeInfo(nChannel,strPicFormatInfo);

		if(!bRet1)
		{
			LogError("error error 11111\n");
			uErrorCode = SRIP_ERROR_USER_FAILE;
		}*/


	}

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (CHANNEL_PICFORMAT_SET_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送通道图片格式设置回复数据包失败[%d]\r\n",nSocket);

	return true;
}

//系统硬件配置信息
bool CSkpRoadMsgCenter::OnSysHwInfo(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    std::string response;
    response.append((char*)&g_sysInfo_ex,sizeof(SYSTEM_INFO_EX));

    MIMAX_HEADER mHeader;
    mHeader.uCmdID = SYSTEM_HARDWARE_INFO_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送系统硬件配置信息回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//检测器ID设置
bool CSkpRoadMsgCenter::OnDetectorIDSetup(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    SYSTEM_CONFIG_EX sys_config_ex;
    memcpy(&sys_config_ex,request.c_str()+sizeof(SRIP_HEADER),sizeof(SYSTEM_CONFIG_EX));

    std::string strDetectorID (sys_config_ex.chDetectorID);
    g_strDetectorID = strDetectorID;
	
	char strRoadId[256] = {"/etc/RoadId.txt"};
	{
		char   buf[1024]={0};
		sprintf(buf,"%s",g_strDetectorID.c_str());
		FILE* fp = fopen(strRoadId,"wb");
		if(fp)
		{
			fwrite(buf,1,strlen(buf),fp);
			fclose(fp);
		}
	}

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("OtherSetting","DetectorID");


    MIMAX_HEADER mHeader;
    mHeader.uCmdID = DETECTOR_ID_SET_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送检测器ID设置回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//检测器复位
bool CSkpRoadMsgCenter::OnDetectorReset(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    MIMAX_HEADER mHeader;
	mHeader.uCmdID = (DETECTOR_RESET_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

    std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送检测器复位返回数据包失败[%d]\r\n",nSocket);

    //检测器复位
    LogNormal("检测器复位\n");
    //g_bEndThread = true;
    char   cmdline[256]={0};
    sprintf(cmdline,"reboot");
    system(cmdline);

    return true;
}

//检测器关机
bool CSkpRoadMsgCenter::OnDetectorShutDown(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    MIMAX_HEADER mHeader;
	mHeader.uCmdID = (DETECTOR_SHUTDOWN_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

    std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送检测器关机返回数据包失败[%d]\r\n",nSocket);

    LogNormal("检测器关机成功\n");

    //检测器关机
    char   cmdline[256]={0};
    sprintf(cmdline,"halt -p");
    system(cmdline);

    return true;
}

//软件复位
bool CSkpRoadMsgCenter::OnSoftWareReset(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SOFTWARE_RESET_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

    std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送软件复位返回数据包失败[%d]\r\n",nSocket);

    //软件复位
    //throw 0;
    g_bEndThread = true;
	sleep(3);
    LogNormal("软件复位成功\n");
	//LogNormal("应用软件故障");
    exit(-1);

    return true;
}

//开关灯控制
bool CSkpRoadMsgCenter::OnLightTimeControl(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

    //取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    //设置参数
    LIGHT_TIME_INFO LightTimeInfo;
    memcpy(&LightTimeInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(LIGHT_TIME_INFO));

    g_LightTimeInfo = LightTimeInfo;

    CXmlParaUtil xml;
    xml.UpdateSystemSetting("LightTimeSetting","");


    MIMAX_HEADER mHeader;
    mHeader.uCmdID = LIGHT_TIME_CONTROL_REP;
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));

	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送开关灯控制回复数据包失败[%d]\r\n",nSocket);

    return true;
}

//强制红灯
bool CSkpRoadMsgCenter::OnForceRedLight(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;

	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	memcpy(&g_nForceRedLlight,request.c_str()+sizeof(SRIP_HEADER),sizeof(int));
	memcpy(&g_uTrafficSignal,request.c_str()+sizeof(SRIP_HEADER)+sizeof(int),sizeof(UINT16));
    printf("=======g_nForceRedLlight=%d,g_uTrafficSignal=%x\r\n",g_uTrafficSignal,g_uTrafficSignal);

    MIMAX_HEADER mHeader;
	mHeader.uCmdID = (FORCE_RED_LIGHT_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

    std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送强制红灯返回数据包失败[%d]\r\n",nSocket);

    return true;
}

//实时界面获取以及设置区域
bool CSkpRoadMsgCenter::OnRegionInfo(const int nSocket, std::string request)
{
    UINT32 uErrorCode = SRIP_OK;
    SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

    MIMAX_HEADER mHeader;
    std::string response;

    CXmlParaUtil xml;
    if(sHeader->uMsgCommandID == REGION_GET)//获取区域
    {
        //
        /*FILE* fp = fopen("region.xml","wb");
        fwrite(request.c_str()+sizeof(SRIP_HEADER),request.size()+sizeof(SRIP_HEADER),1,fp);
        fclose(fp);*/

        XMLNode RegionNode = XMLNode::parseString(request.c_str()+sizeof(SRIP_HEADER));
        XMLNode ChannelNode = RegionNode.getChildNode("ChannelId");
        XMLCSTR strText = ChannelNode.getText();
        if(strText)
        {
            int nChannel = xmltoi(strText);

            printf("OnRegionInfo ======nChannel=%d\n",nChannel);
            xml.GetRegion(nChannel,response);
        }
        mHeader.uCmdID = REGION_GET_REP;
        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+response.size();
    }
    else if(sHeader->uMsgCommandID == REGION_SET)//设置区域
    {
        string strRegion;
        strRegion.append((char*)request.c_str()+sizeof(SRIP_HEADER),request.size()-sizeof(SRIP_HEADER));
        bool bRet = xml.SetRegion(strRegion);
        mHeader.uCmdID = REGION_SET_REP;
        mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER);

        if(!bRet)
        uErrorCode = SRIP_ERROR_USER_FAILE;
    }
    //返回代码
	sHeader->uMsgCode = (uErrorCode);
    response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
    response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
		LogError("发送获取区域返回数据包失败[%d]\r\n",nSocket);

    return true;
}

bool CSkpRoadMsgCenter::OnForceAlert(const int nSocket, std::string request)
{
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	
	XMLNode xmlBase = XMLNode::parseString(request.c_str()+sizeof(SRIP_HEADER));

	int nChannel = 0;
	int nAlertType = 0;
	int nX = 0;
	int nY = 0;

	do 
	{
		if (xmlBase.isEmpty())
		{
			break;
		}

		XMLNode xmlChild = xmlBase.getChildNode("ChannelID");

		if (xmlChild.isEmpty() || !xmlChild.getText())
		{
			break;
		}

		nChannel = atoi(xmlChild.getText());

		xmlChild = xmlBase.getChildNode("AlertType");

		if (xmlChild.isEmpty() || !xmlChild.getText())
		{
			break;
		}

		nAlertType = atoi(xmlChild.getText());

		xmlChild = xmlBase.getChildNode("X");

		if (xmlChild.isEmpty() || !xmlChild.getText())
		{
			break;
		}

		nX = atoi(xmlChild.getText());

		xmlChild = xmlBase.getChildNode("Y");

		if (xmlChild.isEmpty() || !xmlChild.getText())
		{
			break;
		}

		nY = atoi(xmlChild.getText());

		FORCEALERT *pAlert = new FORCEALERT;

		pAlert->nAlertType = nAlertType;
		pAlert->nX = nX;
		pAlert->nY = nY;

		g_skpChannelCenter.AddForceAlert(nChannel,pAlert);

	} while (false);
	

	return true;
}


bool CSkpRoadMsgCenter::OnDetectRegionRectImage(const int nSocket, std::string request)
{
	printf("CSkpRoadMsgCenter::OnDetectRegionRectImage \n");
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));

	ImageRegion imgRegion;
	memcpy(&imgRegion,request.c_str()+sizeof(SRIP_HEADER)+sizeof(int),sizeof(ImageRegion));

	printf("nChannel=%d,nImageRegionType=%d,x=%d,y=%d,w=%d,h=%d,OnRegionImage\n",nChannel,imgRegion.nImageRegionType,imgRegion.x,imgRegion.y,imgRegion.width,imgRegion.height);

	g_skpChannelCenter.DetectRegionRectImage(nChannel,imgRegion);

	return true;
}

bool CSkpRoadMsgCenter::OnDetectParkObjectsRect(const int nSocket,UINT32 uMsgCommandID, std::string request)
{
	printf("CSkpRoadMsgCenter::OnDetectParkObjectsRect \n");
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();


	int nChannel = *((int*)(request.c_str()+sizeof(SRIP_HEADER)));

	printf("nChannel=%d,\n",nChannel);


	RectObject ObjectRect;
	memcpy(&ObjectRect,request.c_str()+sizeof(SRIP_HEADER)+sizeof(int),sizeof(RectObject));
	g_skpChannelCenter.DetectParkObjectsRect(nChannel,uMsgCommandID,ObjectRect);
	return true;
}

//获取相机列表命令
bool CSkpRoadMsgCenter::OnGetDspList(const int nSocket, std::string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	std::string response  = "";
	if(m_pManageClient)
	{
		DSP_LIST dsplist;
		m_pManageClient->GetDspList(dsplist);

		if(dsplist.size() > 0)
		{
			int nSize = sizeof(DSP_INFO) * dsplist.size();
			response.append((char*)&dsplist, nSize);
		}
	}

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (SRIP_GET_DSP_LIST_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER) + response.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);	

	response.insert(0,(char*)sHeader,sizeof(SRIP_HEADER));
	response.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));	

	printf("OnGetDspList response.c_str()=%s,response.size()=%d",response.c_str(),response.size());
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送DspList信息返回数据包失败[%d]！\r\n",nSocket);
	}
	else
	{
		LogError("发送DspList信息返回数据包成功[%d]！\r\n",nSocket);
	}
	return true;
}

bool CSkpRoadMsgCenter::Set_Ftp_Path(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_ERROR_FORMAT;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	memcpy(&g_Ftp_Path,request.c_str()+sizeof(SRIP_HEADER),sizeof(SET_FTP_PATH));

	if(request.size() == sizeof(SRIP_HEADER)+sizeof(SET_FTP_PATH))
	{
		CXmlParaUtil xml;
		xml.UpdateSystemSetting("FtpPathInfo","ALL");
		uErrorCode = SRIP_OK;
	}

	//返回验证结果
	std::string response;
	MIMAX_HEADER mHeader;
	//命令
	mHeader.uCmdID = (SET_3G_MODE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
	sHeader->uMsgCode = (uErrorCode);

	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送FTP路径设置回复数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::Get_Ftp_Path(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (GET_FTP_PATH_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(SET_FTP_PATH);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);
	g_Ftp_Path.nServerType = g_nServerType;

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.append((char*)&g_Ftp_Path, sizeof(SET_FTP_PATH));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送SET_FTP_PATH信息返回数据包失败[%d]！\r\n",nSocket);
	}
	else
	{
		LogNormal("发送SET_FTP_PATH信息成功！[%d]\n",response.size());
	}
}

bool CSkpRoadMsgCenter::Set_Ntp_Time(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_ERROR_FORMAT;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	memcpy(&g_Ntp_Time,request.c_str()+sizeof(SRIP_HEADER),sizeof(SET_NTP_TIME_INFO));


	if(request.size() == sizeof(SRIP_HEADER)+sizeof(SET_NTP_TIME_INFO))
	{
		CXmlParaUtil xml;
		xml.UpdateSystemSetting("NtpTimeInfo","ALL");
		uErrorCode = SRIP_OK;
	}

	//返回验证结果
	std::string response;
	MIMAX_HEADER mHeader;
	//命令
	mHeader.uCmdID = (SET_NTP_TIME_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
	sHeader->uMsgCode = (uErrorCode);

	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送Ntp设置回复数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	SetSysClockForSHJJ();
	return true;
}

bool CSkpRoadMsgCenter::Get_Ntp_Time(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (GET_NTP_TIME_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(SET_NTP_TIME_INFO);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	sprintf(g_Ntp_Time.chHost,"%s",g_strSynClockHost.c_str());
	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.append((char*)&g_Ntp_Time, sizeof(SET_NTP_TIME_INFO));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送SET_NTP_TIME_INFO信息返回数据包失败[%d]！\r\n",nSocket);
	}
	else
	{
		LogNormal("发送SET_NTP_TIME_INFO信息成功！[%d]\n",response.size());
	}
}

bool CSkpRoadMsgCenter::SetCheckTime(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_ERROR_FORMAT;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	memcpy(&g_CheckTime,request.c_str()+sizeof(SRIP_HEADER),sizeof(FASTINGIUM_TIME));

	if(request.size() == sizeof(SRIP_HEADER)+sizeof(FASTINGIUM_TIME))
	{
		CXmlParaUtil xml;
		xml.UpdateSystemSetting("CheckTime","ALL");
		uErrorCode = SRIP_OK;
	}

	//返回验证结果
	std::string response;
	MIMAX_HEADER mHeader;
	//命令
	mHeader.uCmdID = (SET_CHECK_TIME_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
	sHeader->uMsgCode = (uErrorCode);

	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送检测设置回复数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::GetCheckTime(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (GET_CHECK_TIME_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(FASTINGIUM_TIME);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.append((char*)&g_CheckTime, sizeof(FASTINGIUM_TIME));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送检测信息返回数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::SetKafka(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_ERROR_FORMAT;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	memcpy(&g_Kafka,request.c_str()+sizeof(SRIP_HEADER),sizeof(ServerKafka));

	if(request.size() == sizeof(SRIP_HEADER)+sizeof(ServerKafka))
	{
		CXmlParaUtil xml;
		xml.UpdateSystemSetting("KafkaSet","ALL");
		uErrorCode = SRIP_OK;
	}

	//返回验证结果
	std::string response;
	MIMAX_HEADER mHeader;
	//命令
	mHeader.uCmdID = (SET_SERVER_KAFKA_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
	sHeader->uMsgCode = (uErrorCode);

	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送kafka设置回复数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::GetKafka(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (GET_SERVER_KAFKA_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(ServerKafka);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.append((char*)&g_Kafka, sizeof(ServerKafka));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送kafka信息返回数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::SetPlateLimit(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_ERROR_FORMAT;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	memcpy(&g_PlateLimit,request.c_str()+sizeof(SRIP_HEADER),sizeof(PlateLimit));

	if(request.size() == sizeof(SRIP_HEADER)+sizeof(PlateLimit))
	{
		CXmlParaUtil xml;
		xml.UpdateSystemSetting("PlateLimit","");
		uErrorCode = SRIP_OK;
	}

	//返回验证结果
	std::string response;
	MIMAX_HEADER mHeader;
	//命令
	mHeader.uCmdID = (SET_PLATE_LIMIT_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
	sHeader->uMsgCode = (uErrorCode);

	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送kafka设置回复数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::GetPlateLimit(const int nSocket, string request)
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//设置参数
	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (GET_PLATE_LIMIT_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+sizeof(PlateLimit);
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.append((char*)&g_PlateLimit, sizeof(PlateLimit));
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送kafka信息返回数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::SetRoadInfo( const int nSocket, string request )
{
	LogNormal("CSkpRoadMsgCenter::SetRoadInfo \n");
	UINT32 uErrorCode = SRIP_ERROR_FORMAT;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();

	//memcpy(&g_RoadInfo,request.c_str()+sizeof(SRIP_HEADER),sizeof(sRoadNameInfo));
	CXmlParaUtil xmlUtil;

	if(request.size() > sizeof(SRIP_HEADER))
	{
		uErrorCode = SRIP_OK;
	}
	else if (request.size() == sizeof(SRIP_HEADER))//如果发送只有头(清空),删除xml文件
	{
		LogNormal("delete RoadItude.xml\n");
		RoadNameInfoList listRoadInfoList;
		LogNormal("listRoadInfolist size1 = %d\n",listRoadInfoList.size());
		if (xmlUtil.LoadRoadItudeInfo(listRoadInfoList))
		{
			LogNormal("listRoadInfolist size2 = %d\n",listRoadInfoList.size());
			listRoadInfoList.clear();
		}

		char   szFileName[128]={0};
		sprintf(szFileName,"rm -rf ./RoadItude.xml");
		system(szFileName);

		uErrorCode = SRIP_OK;
	}
	xmlUtil.SetRoadItudeInfo(request.c_str()+sizeof(SRIP_HEADER));

	//返回验证结果
	std::string response;
	MIMAX_HEADER mHeader;
	//命令
	mHeader.uCmdID = (SETTING_ROADITUDE_REP);
	//长度
	mHeader.uCmdLen = sizeof(SRIP_HEADER) + sizeof(MIMAX_HEADER);
	sHeader->uMsgCode = (uErrorCode);

	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));
	response.append((char*)sHeader,sizeof(SRIP_HEADER));
	//发送数据
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送路段信息设置回复数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}

bool CSkpRoadMsgCenter::GetRoadInfo( const int nSocket, string request )
{
	UINT32 uErrorCode = SRIP_OK;
	//取头
	SRIP_HEADER* sHeader = (SRIP_HEADER*)request.c_str();
	LogNormal("CSkpRoadMsgCenter::GetRoadInfo \n");
	//设置参数

	//获取文件大小
    //string strResult = GetImageByPath("./RoadItude.xml");
	CXmlParaUtil xml;
	xml.GetRoadItudeInfo(request);

	MIMAX_HEADER mHeader;
	mHeader.uCmdID = (GET_ROADITUDE_REP);
	//长度
	mHeader.uCmdLen = sizeof(MIMAX_HEADER) + sizeof(SRIP_HEADER)+request.size();
	//返回代码
	sHeader->uMsgCode = (uErrorCode);

	std::string response;
	response.append((char*)&mHeader,sizeof(MIMAX_HEADER));	//发送数据
	response.append((char*)sHeader,sizeof(SRIP_HEADER));	//发送数据
	response.append((char*)request.c_str(), request.size());
	if(!g_skpRoadServer.SendMsg(nSocket,response))
	{
		LogError("发送路段信息返回数据包失败[%d]！\r\n",nSocket);
		return false;
	}
	return true;
}
