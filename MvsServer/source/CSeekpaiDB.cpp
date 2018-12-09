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
//CSeekpaiDB.cpp
#include "Common.h"
#include "CommonHeader.h"
#include "CSeekpaiDB.h"
#include "XmlParaUtil.h"
#include "RoadXmlData.h"
#include "RoadImcData.h"
#include "ximage.h"
#include "BrandSubSection.h"
#include "CenterServerOneDotEight.h"

#define BUFF_SIZE 1024

/*
#ifndef _DEBUG
   // #define _DEBUG
#endif
*/

//用户信息列表
UserMap g_vUserMap;

//全局DB
CSeekpaiDB g_skpDB;


CSeekpaiDB::CSeekpaiDB()
{
    //db init and connect
    m_bConnect=false;
    //统计记录序号
    m_nStatisticSeq = 1;

    m_strDeleteDate = "";
    return;
}

CSeekpaiDB::~CSeekpaiDB()
{
    return;
}

//初始化
bool CSeekpaiDB::Init()
{
    //连接参数
    initCondition(g_strDbHost,g_nDbPort,g_strDatabase,g_strUser,g_strPw);
    printf("g_strDbHost=%s,g_nDbPort=%d,g_strDatabase=%s,g_strUser=%s,g_strPw=%s\r\n",g_strDbHost.c_str(),g_nDbPort,g_strDatabase.c_str(),g_strUser.c_str(),g_strPw.c_str());
    //打开连接DB
    if(connectDB())
    {
        m_bConnect=true;
        //获取检测器信息
        string strDeviceInfo = GetDeviceInfo();
        memcpy(g_sysInfo_ex.szDetectorType,strDeviceInfo.c_str(),strDeviceInfo.size());
        printf("======g_sysInfo_ex.szDetectorType=%s\n",g_sysInfo_ex.szDetectorType);
    }
    else
    {
        m_bConnect=false;
        return false;
    }
    //初始化用户信息
    ReadUser();

    //utf8转gbk文件标识
    m_nCvtUTF = iconv_open("GBK","UTF8");

    //gbk转utf8文件标识
    m_nCvtGBK = iconv_open("UTF8","GBK");

    //统计记录序号
    m_nStatisticSeq = GetSeq(1);

    //获取开机时间
    g_uTime = GetTimeStamp();
    //获取上次数据库记录时间
    std::string strTime = GetLastRecorderTime();
    g_uLastTime = MakeTime(strTime);

    //获取最新的图片编号
    g_skpDB.GetLastPicId();

    pthread_mutex_init(&g_Id_Mutex,NULL);
	pthread_mutex_init(&g_MvsRecvPicId_Mutex,NULL);
	pthread_mutex_init(&g_uImgKeyMutex,NULL);
	
    return true;
}

//释放资源
void CSeekpaiDB::UnInit()
{
    //关闭数据库连接
    if(m_bConnect)
    {
        closeDB();
        m_bConnect=false;
    }

    if(m_nCvtUTF > (iconv_t)(-1))
        iconv_close(m_nCvtUTF);

    if(m_nCvtGBK > (iconv_t)(-1))
        iconv_close(m_nCvtGBK);

    //清楚用户链表
    g_vUserMap.clear();

    pthread_mutex_destroy(&g_Id_Mutex);
	pthread_mutex_destroy(&g_MvsRecvPicId_Mutex);
	pthread_mutex_destroy(&g_uImgKeyMutex);
    return;
}

//获取检测器信息
string CSeekpaiDB::GetDeviceInfo()
{
    string strDeviceInfo;
    char buf[BUFF_SIZE];
    memset(buf,0,BUFF_SIZE);
    sprintf(buf,"Select * from DEVICE_INFO");
    String sql(buf);

    MysqlQuery q = execQuery(sql);

    if(!q.eof())
    {
        strDeviceInfo = q.getStringFileds("DETECTOR_TYPE");
    }
    q.finalize();

    return strDeviceInfo;
}

bool CSeekpaiDB::WriteTimeInfo()
{
    char buf[BUFF_SIZE];
    memset(buf,0,BUFF_SIZE);
    sprintf(buf,"Select count(*) from TIME_INFO");
    String sql(buf);

    int nRow =0;
    nRow = getIntFiled(sql);

    if(nRow==0)
    {
        String strTime = GetTimeCurrent();
        sprintf(buf,"Insert into TIME_INFO(TIME) values('%s')",strTime.c_str());
        sql = buf;
        if(execSQL(sql)!=0)
            return false;
    }
    return true;
}

//添加用户1:存在 0:成功 -1:异常
int CSeekpaiDB::AddUser(String strName,String strPw,int nKind)
{
    //合法化字符
    String strNameTemp=RationStr(strName,10);

    //判断用户是否存在
    String sql="select count(*) from USER_INFO where USER_NAME='"+strNameTemp+"'";
    int nRow =0;
    nRow = getIntFiled(sql);

    //printf("%d",nRow);


    if(nRow>0)
    {
        printf("用户已经存在");
        return SRIP_ERROR_USER_EXIST;
    }

    char buf[BUFF_SIZE];
    sprintf(buf,"Insert into USER_INFO(USER_NAME,USER_PW,USER_RIGHT1) values('%s','%s',%d)",strNameTemp.c_str(),strPw.c_str(),nKind);
    String temp(buf);
    sql = temp;

//	#ifdef _DEBUG
    printf("%s \n",sql.c_str());
//	#endif

    if(execSQL(sql)!=0)
        return SRIP_ERROR_USER_FAILE;


    UserStru user;
    user.strName=strName;
    user.strPw=strPw;
    user.nRight=nKind;

    UserMap::iterator it = g_vUserMap.find(strName);
    if (it != g_vUserMap.end())
        g_vUserMap.erase(it);
    //插入映射
    g_vUserMap.insert(UserMap::value_type(strName,user));

    return SRIP_OK;
}

//修改用户密码
int CSeekpaiDB::ModyUser(String strName,String strPw,int nKind)
{
    //合法化字符
    String strNameTemp=RationStr(strName,10);
    //
    if(strPw.size() == 0)
    {
        printf("pass can not be null\n");
        return SRIP_ERROR_PASSW_EMPTY;
    }

    char buf[BUFF_SIZE];
    //if(strPw.size() == 0)
    //sprintf(buf,"Update USER_INFO set USER_RIGHT1=%d where USER_NAME='%s'",nKind,strNameTemp.c_str());
    //else
    sprintf(buf,"Update USER_INFO set USER_PW='%s', USER_RIGHT1=%d where USER_NAME='%s'",strPw.c_str(),nKind,strNameTemp.c_str());

    String sql(buf);

    printf("%s \n",sql.c_str());
    if(execSQL(sql)!=0)
    {
        printf("update database error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    UserMap::iterator it=g_vUserMap.find(strName);
    if (it != g_vUserMap.end())
    {
        if(strPw.size() > 0)
        {
            (it->second).strPw=strPw;
        }
        (it->second).nRight=nKind;

    }
    else
    {
        printf("Mody UserMap error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}

//删除用户
int CSeekpaiDB::DelUser(String strName)
{
    //合法化字符
    String strNameTemp=RationStr(strName,10);
    //
    if(!strName.compare("admin"))
    {
        printf("Cann't del admin \n");
        return SRIP_ERROR_ADMIN_DEL;
    }

    String sql="Delete from USER_INFO where USER_NAME='"+strNameTemp+"'";
#ifdef _DEBUG
    printf("%s \n",sql.c_str());
#endif
    if(execSQL(sql)!=0)
    {
        printf("Delete from database error! \n");
        return SRIP_ERROR_USER_FAILE;
    }
    UserMap::iterator it=g_vUserMap.find(strName);
    if (it != g_vUserMap.end())
    {
        g_vUserMap.erase(it);
    }
    else
    {
        printf("Del UserMap error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}

//读取用户列表
void CSeekpaiDB::ReadUser()
{
    String sql="Select USER_NAME,USER_PW,USER_RIGHT1 from USER_INFO";

    int right;
    MysqlQuery q = execQuery(sql);

    String name(""), password("");
    g_vUserMap.clear();
    while(!q.eof())
    {
        name.clear();
        password.clear();
        name=q.getStringFileds("USER_NAME");
        password=q.getStringFileds("USER_PW");
        right=q.getIntFileds("USER_RIGHT1");

        ToLowerCase(name);

        UserStru user;
        user.strName=name;
        user.strPw=password;
        user.nRight=right;
        //插入映射
        g_vUserMap.insert(make_pair(name, user));
        //	printf("User:%s,Pw:%s,password.size=%d,right:%d \r\n",name.c_str(),password.c_str(),password.size(),right);
        q.nextRow();
    }
    //printf("g_vUserMap.size()=%d\n", (int)g_vUserMap.size());
    q.finalize();

    return;
}

//获得所有用户
String CSeekpaiDB::GetUsrList()
{
    std::string response;

    //printf("g_vUserMap.size()=%d\n", (int)g_vUserMap.size());
    UserMap::iterator it=g_vUserMap.begin();
    while (it != g_vUserMap.end())
    {
        LOGIN_INFO login_info;

        memcpy(login_info.chUserName,it->second.strName.c_str(),it->second.strName.size());
        memcpy(login_info.chPass,it->second.strPw.c_str(),it->second.strPw.size());

        login_info.Priv = it->second.nRight;
        //printf("=============login_info.chUserName=%s, login_info.chPass=%s, login_info.Priv=%d\n",login_info.chUserName,login_info.chPass,login_info.Priv);
        response.append((char*)&login_info,sizeof(login_info));

        it ++;
    }
    return response;
}

//验证用户密码
int CSeekpaiDB::CheckLogin(String strName,String strPw)
{
    String sql;

    ToLowerCase(strName);

    //取出用户密码(MD5)
    UserMap::iterator it=g_vUserMap.find(strName);
    if (it == g_vUserMap.end())
    {
        LogError("用户名[%s]不存在\r\n",strName.c_str());
        return SRIP_LOGIN_USER_ERROR;
    }
    String password=(it->second).strPw;

    printf("strPw:%s,password:%s,strPw.size()=%d,password.size()=%d\r\n",strPw.c_str(),password.c_str(),strPw.size(),password.size());

    if(strncmp(strPw.c_str(),password.c_str(),32) != 0)
//	if(strPw != password)
        return SRIP_LOGIN_PASS_ERROR;

    return SRIP_OK;
}

//获得用户权限
int CSeekpaiDB::GetUserPriv(String strName, int nAction)
{
    //合法化字符
    String strNameTemp=RationStr(strName,10);
    // 查询字段
    String strField;
//	printf("strName = %s,size=%d,nAction=%x\r\n",strName.c_str(),strName.size(),nAction);
    //获取操作行为
    switch (nAction)
    {
    case SRIP_LOGIN:				// 用户登陆
        strField = "USER_RIGHT1";
        break;
    case SRIP_USER_ADD:				//添加用户
        strField = "ADD_USER";
        break;
    case SRIP_USER_DEL:				//删除用户
        strField =  "DEL_USER";
        break;
    case SRIP_USER_MOD:				//修改用户
        strField = "MODIFY_USER";
        break;
    case SRIP_USER_LIST:			//用户列表
        strField = "LIST_USER";
        break;
    case SRIP_CHANNEL_ADD:			//添加通道
        strField = "ADD_CHANNEL";
        break;
    case SRIP_CHANNEL_MOD:			//修改通道
        strField = "MODIFY_CHANNEL";
        break;
    case SRIP_CHANNEL_DEL:			//删除通道
        strField = "DEL_CHANNEL";
        break;
    case SRIP_SEARCH_EVENT:			//事件查询
        strField = "SEARCH_EVENT";
        break;
    case SRIP_SEARCH_ALARM:			//报警查询
        strField = "SEARCH_ALARM";
        break;
    case SRIP_SEARCH_CARD:			//车牌识别查询
        strField = "SEARCH_CARD";
        break;
    case SRIP_SEARCH_RECORD:		//录像查询
        strField = "SEARCH_RECORD";
        break;
    case SRIP_SEARCH_LOG:			//日志查询
        strField = "SEARCH_LOG";
        break;
    case SRIP_CHANNEL_PARA:			//通道视频参数
        strField = "CHANNEL_PARA";
        break;
    case SRIP_CHANNEL_PARA_ADJUST:	//调整视频参数
        strField = "ADJUST_CHANNEL_PARA";
        break;
    case SRIP_CHANNEL_PARA_SAVE:	//保存视频参数
        strField = "SAVE_CHANNEL_PARA";
        break;
    case SRIP_ROAD_WAY:				//车道数据
        strField = "GET_ROADWAY";
        break;
    case SRIP_ROAD_DEL:				//删除车道数据
        strField = "DELETE_ROADWAY";
        break;
    case SRIP_ROAD_SAVE:			//保存车道数据
        strField = "SAVE_ROADWAY";
        break;
    case SRIP_SYS_SETTING:				//设置系统信息
        strField = "GET_SYSSETTING";
        break;
    case SRIP_BACKUP_DATABASE:		//备份数据库
        strField = "BACKUPDB";
        break;
    case SRIP_DELETE_EVENT:		//删除事件
        strField = "DEL_EVENT";
        break;
    case SRIP_DELETE_ALARM:		//删除统计
        strField = "DEL_ALARM";
        break;
    case SRIP_DELETE_RECORD:	//删除录象
        strField = "DEL_RECORD";
        break;
    case SRIP_DELETE_CARD:		//删除车牌
        strField = "DEL_CARD";
        break;
    case SRIP_DELETE_LOG:
        strField = "DEL_LOG"; //删除日志
        break;
    default:
        break;
    }
    //
    String sql="Select " + strField + " from USER_INFO where USER_NAME='"+strNameTemp+"'";
    // 权限
    int nPriv = getIntFiled(sql);

//	printf("sql = %s value = %d\r\n", sql.c_str(), nPriv);

    return nPriv;
}

//存储视频文件
/*int CSeekpaiDB::SaveVideo(int nChannel,UINT32 uBeginTime, UINT32 uEndTime,String strPath,int VideoType)
{
    String strBeginTime = GetTime(uBeginTime);
    String strEndTime = GetTime(uEndTime);
    char buf[BUFF_SIZE]= {0};
    //保存文件
    sprintf(buf,"INSERT INTO VIDEO_FILE_INFO(CHANNEL,BEGIN_TIME,END_TIME,PATH,VIDEO_TYPE) VALUES(%d,'%s','%s','%s',%d)",
            nChannel,strBeginTime.c_str(),strEndTime.c_str(),strPath.c_str(),VideoType);
    String sql(buf);
    if(execSQL(sql)!=0)
    {
        LogError("保存录像记录失败[%d][%s][%s][%s]\r\n",nChannel,strBeginTime.c_str(),strEndTime.c_str(),strPath.c_str());
        return 0;
    }
    return 1;
}*/


//读取视频信息量
int CSeekpaiDB::GetVideoCount(int nChannel, int nType, String strDateBegin,String strDateEnd)
{
    char buf[BUFF_SIZE]= {0};

    if(nChannel == 0)
    {
        sprintf(buf,"SELECT count(*) FROM VIDEO_FILE_INFO WHERE BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",strDateBegin.c_str(),strDateEnd.c_str());
    }
    else
    {
        sprintf(buf,"SELECT count(*) FROM VIDEO_FILE_INFO WHERE CHANNEL = %d AND BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",nChannel,strDateBegin.c_str(),strDateEnd.c_str());
    }

    String sql(buf);
//	printf("sql = %s\r\n",sql.c_str());

    return getIntFiled(sql);
}

//获取录象信息
String CSeekpaiDB::GetVideoRecord(SEARCH_ITEM& search_item,int nPageSize/*=0*/)
{
    String strDateBegin = GetTime(search_item.uBeginTime);
    String strDateEnd = GetTime(search_item.uEndTime);

    int nPage = search_item.uPage;
    int nSortId = search_item.uSortId;
    int nSortKind = search_item.uSortKind;
    int nChannel = search_item.uChannelId;
    int ntype = search_item.uType;

    String response;


    if(nPage<1 ||nPageSize<0)
    {
#ifdef _DEBUG
        printf("nPage or nPageSize is error! \n");
#endif
        return response;
    }

    char SortKind[2][5]= {"ASC","DESC"};
    char SortId[5][20]= {"ID","CHANNEL","BEGIN_TIME","END_TIME","PATH"};

    char buf[BUFF_SIZE]= {0};

    /* 页大小不为0 */
    if(nChannel == 0)	/* 查询所有车道 */
    {

        sprintf(buf,"Select ID,CHANNEL,BEGIN_TIME,END_TIME,PATH from VIDEO_FILE_INFO where BEGIN_TIME>='%s' and BEGIN_TIME<='%s' ORDER BY %s %s limit %d,%d",strDateBegin.c_str(),strDateEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);

    }
    else
    {

        sprintf(buf,"Select ID,CHANNEL,BEGIN_TIME,END_TIME,PATH from VIDEO_FILE_INFO where CHANNEL = %d and BEGIN_TIME>='%s' and BEGIN_TIME<='%s' ORDER BY %s %s limit %d,%d",nChannel,strDateBegin.c_str(),strDateEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
    }

    //获得查询数量
    int nCount=0;
    if(nPage==1)
    {
        nCount=GetVideoCount(nChannel,ntype,strDateBegin,strDateEnd);
        int nNum=nCount % nPageSize;
        if(nNum>0)
            nCount=nCount/nPageSize+1;
        else
            nCount=nCount/nPageSize;
    }

    // 执行查询
    String sql(buf);
    MysqlQuery q = execQuery(sql);

    response.append((char*)&nCount,sizeof(nCount));

    while(!q.eof())
    {
        RECORD_EVENT event;
        event.uSeq=q.getUnIntFileds("ID");

        int nChannelId = q.getIntFileds("CHANNEL");
        memcpy(event.chReserved,&nChannelId,sizeof(int));//暂时借用保留字段

        String strTime = q.getStringFileds("BEGIN_TIME");
        event.uVideoBeginTime = MakeTime(strTime);

        strTime = q.getStringFileds("END_TIME");
        event.uVideoEndTime = MakeTime(strTime);

        String strPath = q.getStringFileds("PATH");
        memcpy(event.chVideoPath,strPath.c_str(),strPath.size());
        response.append((char*)&event,sizeof(event));

        q.nextRow();
    }

    q.finalize();

    return response;

}




//添加录像通道
int CSeekpaiDB::AddChan(SRIP_CHANNEL sChannel)
{
    char buf[BUFF_SIZE]= {0};

    //合法化字符
	String strPlace;
	strPlace.append(sChannel.chPlace,sizeof(sChannel.chPlace));
	strPlace.append(sChannel.chExPlace,sizeof(sChannel.chExPlace));

    String strSynHost(sChannel.chSynHost);
    String strMonitorHost(sChannel.chMonitorHost);

    String strVideoBeginTime = GetTime(sChannel.uVideoBeginTime);
    String strVideoEndTime = GetTime(sChannel.uVideoEndTime);

    String strFileName(sChannel.chFileName);
	String strIP(sChannel.chCameraHost);
	String strDeviceId(sChannel.chDeviceID);
	String strCode;
	strCode.append(sChannel.chCode,48);

    sprintf(buf,"Select count(CHAN_ID) from CHAN_INFO where CHAN_ID=%d",sChannel.uId);
    String temp(buf);

    int nRow  = getIntFiled(temp);
    if(nRow>0)
    {
        LogError("通道已经存在[%d],添加失败!\r\n",sChannel.uId);
        return SRIP_ERROR_CHAN_EXIST;
    }

    //限制不能同时有两个yuv视频通道存在
    /*if(sChannel.eVideoFmt == VEDIO_YUV)
    {
        sprintf(buf,"Select count(CHAN_ID) from CHAN_INFO where CHAN_FORMAT = 0");
        String temp(buf);

        int nRow  = getIntFiled(temp);
        if(nRow>0)
        {
            LogError("不能同时有两个yuv视频通道存在,添加通道失败!\r\n");
            return SRIP_ERROR_CHAN_EXIST;
        }
    }*/

    //添加通道
    sprintf(buf,"Insert into CHAN_INFO\
    (CHAN_ID, CHAN_PLACE, CHAN_KIND, CHAN_FORMAT, CHAN_EVENTCAPTURETIME,\
        VIDEO_BEGINTIME, VIDEO_ENDTIME, CHAN_CAP_TYPE, CHAN_CAP_BEGINTIME, CHAN_CAP_ENDTIME,\
        CHAN_YUV_HOST, CHAN_YUV_PORT, CHAN_DETECT_KIND, CHAN_DETECT_TIME, CHAN_EVENT_CAPTURE,\
        YUV_FORMAT, CHAN_RMSHADE, CHAN_RMTINGLE, CHAN_SENSITIVE, CHAN_RUN,\
        CAMERA_ID, CHAN_DIRECTION, CHAN_SRC_FILE, CHAN_SHOWTIME, CAMERA_TYPE,\
        PANNEL_ID, SYN_HOST, SYN_PORT,CHAN_PRESET,MONITOR_ID,\
		VIDEO_ID, WORKMODE,CAMERAIP,DEVICE_ID,REGISTER_ID)\
    values(%d, '%s', %d, %d, %d,\
        '%s', '%s', %d, %d, %d,\
        '%s', %d, %d, %d, %d,\
        %d, %d, %d, %d, %d,\
        %d, %d, '%s', %d, %d,\
        %d, '%s', %d, %d, %d,\
		%d, %d,'%s','%s','%s')",\
            sChannel.uId, strPlace.c_str(), sChannel.eType, sChannel.eVideoFmt, sChannel.uEventCaptureTime,\
            strVideoBeginTime.c_str(), strVideoEndTime.c_str(), sChannel.eCapType, sChannel.uCapBeginTime, sChannel.uCapEndTime,\
            strMonitorHost.c_str(), sChannel.uMonitorPort, sChannel.uDetectKind, sChannel.uDetectTime, sChannel.bEventCapture,\
            sChannel.nYUVFormat, sChannel.bRmShade, sChannel.bRmTingle, sChannel.bSensitive, sChannel.bRun,\
            sChannel.nCameraId, sChannel.uDirection, strFileName.c_str(), 0, sChannel.nCameraType,\
            sChannel.nPannelID, strSynHost.c_str(), sChannel.uSynPort,sChannel.nPreSet,sChannel.nMonitorId,\
			sChannel.nVideoIndex,sChannel.nWorkMode, strIP.c_str(), strDeviceId.c_str(),strCode.c_str());

    String sql(buf);
//cout << buf << endl;
#ifdef _DEBUG
    printf("添加通道:%s \n",sql.c_str());
#endif
    if(execSQL(sql)!=0)
    {
        LogError("添加通道失败[%d]!\r\n",sChannel.uId);
        return SRIP_ERROR_USER_FAILE;
    }

#ifdef MVSBAK
	g_skpChannelCenter.AddChannel(sChannel,sChannel.bRun);//传输通道之前Run状态
#else
    //同步通道，并启动
    g_skpChannelCenter.AddChannel(sChannel,true);
#endif

    //记录日志
    LogNormal("用户[xxxx]添加通道[%d]!\r\n",sChannel.uId);
    return SRIP_OK;
}

//修改通道信息
int CSeekpaiDB::ModyChanInfo(SRIP_CHANNEL sChannel)
{
    //合法化字符
    String strPlace;
	strPlace.append(sChannel.chPlace,sizeof(sChannel.chPlace));
	strPlace.append(sChannel.chExPlace,sizeof(sChannel.chExPlace));
	String strCode;
	strCode.append(sChannel.chCode,48);

	printf("strPlace.size()=%d\n",strPlace.size());

    String strSynHost(sChannel.chSynHost);
    String strMonitorHost(sChannel.chMonitorHost);

    String strVideoBeginTime = GetTime(sChannel.uVideoBeginTime);
    String strVideoEndTime = GetTime(sChannel.uVideoEndTime);

    String strFileName(sChannel.chFileName);

    printf("CSeekpaiDB::ModyChanInfo==%s \n",strFileName.c_str());

    char buf[BUFF_SIZE]= {0};
    sprintf(buf,"Update CHAN_INFO set CHAN_PLACE='%s', CHAN_DIRECTION=%d, CHAN_KIND=%d, CHAN_FORMAT=%d, CHAN_EVENTCAPTURETIME=%d,\
        CHAN_CAP_TYPE = %d, CHAN_CAP_BEGINTIME = %d, CHAN_CAP_ENDTIME = %d,\
        CHAN_YUV_HOST = '%s',CHAN_YUV_PORT = %d ,CHAN_DETECT_KIND = %d,CHAN_DETECT_TIME = %d, CHAN_EVENT_CAPTURE = %d,\
        YUV_FORMAT = %d,CHAN_SRC_FILE = '%s', SYN_HOST = '%s', SYN_PORT = %d,CAMERA_TYPE = %d, \
        CAMERA_ID = %d, PANNEL_ID = %d,MONITOR_ID = %d,VIDEO_ID = %d,VIDEO_BEGINTIME = '%s',VIDEO_ENDTIME = '%s',\
		WORKMODE = %d, CAMERAIP = '%s', DEVICE_ID = '%s', REGISTER_ID = '%s' \
		where CHAN_ID=%d",
            strPlace.c_str(), sChannel.uDirection, sChannel.eType, sChannel.eVideoFmt, sChannel.uEventCaptureTime,\
            sChannel.eCapType, sChannel.uCapBeginTime, sChannel.uCapEndTime,\
            strMonitorHost.c_str(), sChannel.uMonitorPort, sChannel.uDetectKind, sChannel.uDetectTime, sChannel.bEventCapture,\
            sChannel.nYUVFormat,strFileName.c_str(), strSynHost.c_str(), sChannel.uSynPort,sChannel.nCameraType, \
            sChannel.nCameraId, sChannel.nPannelID,sChannel.nMonitorId,sChannel.nVideoIndex,strVideoBeginTime.c_str(),strVideoEndTime.c_str(),\
            sChannel.nWorkMode, sChannel.chCameraHost, sChannel.chDeviceID, strCode.c_str(),sChannel.uId);

    //执行更新
    String sql(buf);

    //LogTrace("updateChannel.log", "更新通道:%s", buf);

    if(execSQL(sql)!=0)
    {
        LogError("更新通道失败[%d]!\r\n",sChannel.uId);
        return SRIP_ERROR_USER_FAILE;
    }

    //同步通道，更新
    g_skpChannelCenter.ModifyChannel(sChannel);

    //记录日志
    LogNormal("用户[xxxx]更新通道[%d]!\r\n",sChannel.uId);
    return SRIP_OK;
}
//删除录像通道
int CSeekpaiDB::DelChan(SRIP_CHANNEL sChannel)
{

    char buf[BUFF_SIZE]= {0};
    sprintf(buf,"Delete from CHAN_INFO where CHAN_ID=%d",sChannel.uId);

    String sql(buf);

#ifdef _DEBUG
    printf("删除通道:%s \n",sql.c_str());
#endif
    if(execSQL(sql)!=0)
    {
        LogError("删除通道失败[%d]!\r\n",sChannel.uId);
        return SRIP_ERROR_USER_FAILE;
    }
    //同步通道,删除
    g_skpChannelCenter.DeleteChannel(sChannel);

    //记录日志
    LogNormal("用户[xxxx]删除通道[%d]!\r\n",sChannel.uId);

    return SRIP_OK;
}

//获得视频通道
String CSeekpaiDB::GetChannelList()
{
    String response;

    String sql;

    sql="SELECT CHAN_ID,CHAN_PLACE,CHAN_KIND,CHAN_FORMAT,CHAN_EVENTCAPTURETIME,\
		VIDEO_BEGINTIME,VIDEO_ENDTIME,CHAN_BRIGHTNESS,CHAN_CONTRAST,CHAN_SATURATION,\
		CHAN_HUE,CHAN_CAP_TYPE,CHAN_CAP_BEGINTIME,CHAN_CAP_ENDTIME,CHAN_YUV_HOST,\
		CHAN_YUV_PORT,CHAN_DETECT_KIND,CHAN_DETECT_TIME,CHAN_EVENT_CAPTURE,YUV_FORMAT,\
		CHAN_RMSHADE,CHAN_RMTINGLE,CHAN_SENSITIVE,CHAN_RUN,CHAN_SRC_FILE,\
		CHAN_SHOWTIME,CAMERA_TYPE,CHAN_DIRECTION,CAMERA_ID,PANNEL_ID,\
		SYN_HOST, SYN_PORT,CHAN_PRESET,MONITOR_ID,VIDEO_ID,\
		WORKMODE,CAMERAIP, DEVICE_ID,REGISTER_ID \
		from CHAN_INFO\
		ORDER BY CHAN_ID;";

    MysqlQuery q = execQuery(sql);
    printf("q.numFileds()======%d\n",q.numFileds());
    while(!q.eof())
    {
        //通道结构
        SRIP_CHANNEL sChannel;
        //车道类型
        sChannel.nChanWayType = g_nRoadType;
        //通道ID
        sChannel.uId = q.getIntFileds("CHAN_ID");
        //通道地点
        String strLoc = q.getStringFileds("CHAN_PLACE");

		if(strLoc.size() <= sizeof(sChannel.chPlace))
		{
			memcpy(sChannel.chPlace,strLoc.c_str(),strLoc.size());
		}
		else
		{
			memcpy(sChannel.chPlace,strLoc.c_str(),sizeof(sChannel.chPlace));
			memcpy(sChannel.chExPlace,strLoc.c_str()+sizeof(sChannel.chPlace),strLoc.size()-sizeof(sChannel.chPlace));
		}
        //通道类型
        sChannel.eType = (CHANNEL_TYPE)q.getIntFileds("CHAN_KIND");
        //视频格式
        sChannel.eVideoFmt = (VEDIO_FORMAT)q.getIntFileds("CHAN_FORMAT");
        //事件录像时间
        sChannel.uEventCaptureTime = q.getIntFileds("CHAN_EVENTCAPTURETIME");

        String strBeginTime = q.getStringFileds("VIDEO_BEGINTIME");
        sChannel.uVideoBeginTime = MakeTime(strBeginTime);

        String strEndTime = q.getStringFileds("VIDEO_ENDTIME");
        sChannel.uVideoEndTime = MakeTime(strEndTime);

        //录像参数
        sChannel.eCapType = (CAPTURE_TYPE)q.getIntFileds("CHAN_CAP_TYPE");
        //录像开始时间
        sChannel.uCapBeginTime = q.getIntFileds("CHAN_CAP_BEGINTIME");
        //录像结束时间
        sChannel.uCapEndTime = q.getIntFileds("CHAN_CAP_ENDTIME");

        //主机
        String strMonitorHost =q.getStringFileds("CHAN_YUV_HOST");
        memcpy(sChannel.chMonitorHost,strMonitorHost.c_str(),strMonitorHost.size());
        memcpy(sChannel.chHost,g_ServerHost.c_str(),g_ServerHost.size());
        //端口
        sChannel.uMonitorPort = q.getIntFileds("CHAN_YUV_PORT");
        sChannel.uPort = g_nEPort;

		 //预位置信息
        sChannel.nPreSet = q.getIntFileds("CHAN_PRESET");

        //通道检测类型
        CHANNEL_DETECT_KIND uDetectKind = (CHANNEL_DETECT_KIND)q.getIntFileds("CHAN_DETECT_KIND");
        if((uDetectKind&DETECT_VTS)==DETECT_VTS)
        {
            uDetectKind = (CHANNEL_DETECT_KIND)(uDetectKind|DETECT_VIOLATION);
        }
		if(g_nMultiPreSet == 1)//存在多个预置位
		{
			CXmlParaUtil xml;
			int nPreSetDetectKind = xml.LoadPreSetDetectKind(sChannel.uId,sChannel.nPreSet);
			if(nPreSetDetectKind != 0)
			{
				uDetectKind = (CHANNEL_DETECT_KIND)nPreSetDetectKind;
			}
		}
        sChannel.uDetectKind = uDetectKind;
        //白天晚上检测
        sChannel.uDetectTime = (CHANNEL_DETECT_TIME)q.getIntFileds("CHAN_DETECT_TIME");

        //是否事件录象
        sChannel.bEventCapture = q.getIntFileds("CHAN_EVENT_CAPTURE");
        //yuv格式
        sChannel.nYUVFormat = q.getIntFileds("YUV_FORMAT");
        sChannel.bLocal = true;
        sChannel.bRun = q.getIntFileds("CHAN_RUN");

        String strFileName =q.getStringFileds("CHAN_SRC_FILE");
        memcpy(sChannel.chFileName,strFileName.c_str(),strFileName.size());

        //停留显示时间
        //	sChannel.nShowTime = q.getIntFileds(25);
        //相机型号
        sChannel.nCameraType = q.getIntFileds("CAMERA_TYPE");

        //通道方向
        //String strDirection = q.getStringFileds(27);
        //memcpy(sChannel.chDirection,strDirection.c_str(),strDirection.size());
        sChannel.uDirection = q.getIntFileds("CHAN_DIRECTION");

        //相机编号
        sChannel.nCameraId = q.getIntFileds("CAMERA_ID");

        //断面编号
        sChannel.nPannelID = q.getIntFileds("PANNEL_ID");

        //相邻同步主机
        String strSynHost =q.getStringFileds("SYN_HOST");
        memcpy(sChannel.chSynHost,strSynHost.c_str(),strSynHost.size());

        //相邻同步主机端口
        sChannel.uSynPort = SYNCH_PORT;//q.getIntFileds(31);


        //monitor编号
        sChannel.nMonitorId = q.getIntFileds("MONITOR_ID");

        //视频编号
        sChannel.nVideoIndex = q.getIntFileds("VIDEO_ID");
		//工作方式
		sChannel.nWorkMode = q.getIntFileds("WORKMODE");
		//通道的相机IP
		String strCameraIP = q.getStringFileds("CAMERAIP");
		memcpy(sChannel.chCameraHost,strCameraIP.c_str(),strCameraIP.size());	

		String strDeviceId = q.getStringFileds("DEVICE_ID");
		memcpy(sChannel.chDeviceID, strDeviceId.c_str(), strDeviceId.size());

		String strRegCode = q.getStringFileds("REGISTER_ID");
		memcpy(sChannel.chCode, strRegCode.c_str(), strRegCode.size());

        /*String strUserName =q.getStringFileds(35);
        memcpy(sChannel.chUserName,strUserName.c_str(),strUserName.size());

        String strPassWord =q.getStringFileds(36);
        memcpy(sChannel.chPassWord,strPassWord.c_str(),strPassWord.size());*/

        response.append((char*)&sChannel,sizeof(SRIP_CHANNEL));

        q.nextRow();
    }
    q.finalize();
	//g_skpChannelCenter.g_WhichContinuousSet();
    return response;
}

//合法化字符串,单引号变为两个单引号 'a'->''a''
String CSeekpaiDB::RationStr(String strTemp,int nMaxLen)
{
    //
    StrList pList;
    int nPos=0;
    int nLen=strTemp.find_last_of("'");

    int nSize=strTemp.length();
    if(nLen<0)
        return strTemp;

    bool bFlag=false;
    if(nLen==nSize-1)
        bFlag=true;

    int nStar=0;
    while(nPos<nLen)
    {
        nPos=strTemp.find("'",nPos);//找到"'"

        String s=strTemp.substr(nStar,nPos-nStar);//
        nPos++;
        nStar=nPos;
        pList.push_back(s);
    }
    //other sub
    if(!bFlag)
        pList.push_back(strTemp.substr(nPos,nSize-nPos));

    String strRet;
    while(!pList.empty())
    {
        String temp=*(pList.begin());
        pList.pop_front();

        strRet+=temp;
        if(!pList.empty())
            strRet+="''";
    }

    if(bFlag)
        strRet+="''";

    return strRet;
}

/* 函数介绍：中心服务器获取日志信息
 * 输入参数：uBegin-开始时间或序号，uEnd-结束时间或序号，nKind-按时间、序号或者是推送非实时记录
 * 输出参数：无
 * 返回值：无
 */

void CSeekpaiDB::GetLog(unsigned int uBegin,unsigned int uEnd,int nKind,int nSocket)
{
}
//报警枚举类型转换
void CSeekpaiDB::EnumConvertIntoHex(RECORD_LOG *ptr)
{
	if (26 == g_nServerType || 0 == g_nServerType)
	{
		if (ptr->uCode==ALARM_CODE_DETECTOR_START)
		{
			ptr->uCode=0x00000001;
		}
		else if (ptr->uCode==ALARM_CODE_DETECTOR_REBOOT)
		{
			ptr->uCode=0x00000002;
		}
		else if(ptr->uCode==ALARM_CODE_DETECTOR_HALT)
		{
			ptr->uCode=0x00000004;
		}
		else if (ptr->uCode==ALARM_CODE_CPU_OVER_HEAT)
		{
			ptr->uCode=0x00000010;
		}
		else if (ptr->uCode==ALARM_CODE_LOW_FAN_SPEED)
		{
			ptr->uCode=0x00000020;
		}
		else if(ptr->uCode==ALARM_CODE_CPU_OVER_LOAD)
		{
			ptr->uCode=0x00000040;
		}
		else if(ptr->uCode==ALARM_CODE_DETECTOR_OVER_HEAT)
		{
			ptr->uCode=0x00000080;
		}
		else if(ptr->uCode==ALARM_CODE_NO_VIDEO)
		{
			ptr->uCode=0x00000008;
		}
		else if (ptr->uCode==ALARM_CODE_CAMERA_MOVED)
		{
			ptr->uCode=0x00000100;
		}
		else if (ptr->uCode==ALARM_CODE_BOX_ALERT)
		{
			ptr->uCode=0x00000200;
		}
		else if (ptr->uCode==ALARM_CODE_BOX_OPEN_DOOR)
		{
			ptr->uCode=0x00000400;
		}
		else if (ptr->uCode==ALARM_CODE_BOX_LINE_CUT)
		{
			ptr->uCode =0x00000800;
		}
		else if (ptr->uCode==ALARM_CODE_BOX_VIBRATE)
		{
			ptr->uCode=0x00001000;
		}
		else if (ptr->uCode==ALARM_CODE_DDISK_FULL)
		{
			ptr->uCode=0x00002000;
		}
		else if (ptr->uCode==ALARM_VIDEO_RESUME)
		{
			ptr->uCode=0x00008000;
		}
		else if (ptr->uCode==ALARM_CAMERA_BREAK )
		{
			ptr->uCode =0x00010000;
		}
		else if (ptr->uCode==ALARM_RADAR_BREAK)
		{
			ptr->uCode=0x00020000;
		}
		else if (ptr->uCode==ALARM_CAMERA_LINK)
		{
			ptr->uCode=0x00040000;
		}
	}
}
//保存日志信息
int CSeekpaiDB::SaveLog(String strTime,String strText,unsigned int uCode,bool bAlarm,int nCameraId)
{
    //合法化字符
    strText=RationStr(strText,128);
    //
    char buf[BUFF_SIZE]= {0};

	unsigned int uType = 0;

	
    sprintf(buf,"Insert into SYSTEM_EVENT_INFO(TIME,EVENT,LEVEL,STATUS,CODE) values('%s','%s',%d,%d,%d)"\
            ,strTime.c_str(),strText.c_str(),uType,0,uCode);


    String sql(buf);

    //printf("sql=%s\n",sql.c_str());
    RECORD_LOG log;
    //执行
    if(execSQL(sql,&log.uSeq)!=0)
    {
		/*system("myisamchk -o /var/lib/mysql/bocom_db/SYSTEM_EVENT_INFO ");    //sql_myisamchk
		int fd = open("sql_myisamchk",O_RDWR);
		write(fd, "sql_\n", 5);
		close(fd);*/

        return SRIP_ERROR_USER_FAILE;
    }

    if(bAlarm)
	{
        std::string strLog;

        log.uTime = MakeTime(strTime);
        log.uCode = uCode;

        ////////////////////////////////utf8->gbk
        printf("strText=%s,strText.size()=%d\n",strText.c_str(),strText.size());
        UTF8ToGBK(strText);
        //	printf("strText=%s\n",strText.c_str());
        memcpy(log.chText,strText.c_str(),strText.size());
        //	printf("log.chText=%s\n",log.chText);
        ////////////////////////////////////////////////

        SRIP_DETECT_HEADER sDHeader;
        sDHeader.uDetectType = PLATE_LOG_REP;
        sDHeader.uRealTime = 0x00000001;
        sDHeader.uChannelID  = nCameraId;//相机报警（0表示检测器报警）


		EnumConvertIntoHex(&log);//报警枚举类型转换十六进制

        strLog.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));
        strLog.append((char*)&log,sizeof(RECORD_LOG));
        //发送实时间日志

        if(log.uCode != ALARM_CODE_CPU_OVER_LOAD)
        {
            if(g_nServerType == 1)
            {
               g_AMSCommunication.AddResult(strLog);
            }
            else if(g_nServerType == 3)
            {
                g_TravelServer.AddResult(strLog);
            }
            else if(g_nServerType == 5)
            {
                g_LSServer.AddResult(strLog);
		#ifdef LS_QINGTIAN_IVAP
				g_BocomServerManage.AddResult(strLog);
		#endif
            }
            else if(g_nServerType == 0 || g_nServerType == 4||g_nServerType == 6 || g_nServerType == 26)
            {
				g_BocomServerManage.AddResult(strLog);
            }
        }
    }

    return SRIP_OK;
}


//获得系统信息日志数量
int CSeekpaiDB::GetSystEventCount(int nLevel, String strDateBegin,String strDateEnd)
{
    char buf[BUFF_SIZE]= {0};

    //包含本天
    strDateEnd+=" 23:59:59";

    if(nLevel == ALARM_CODE_ALL)	//所有日志
    {
        sprintf(buf,"Select count(*) from SYSTEM_EVENT_INFO where TIME >='%s' and TIME <='%s'",strDateBegin.c_str(),strDateEnd.c_str());
    }
    else			//特定日志
    {
        //sprintf(buf,"Select count(*) from SYSTEM_EVENT_INFO where LEVEL = %d and TIME >='%s' and TIME <='%s'",nLevel, strDateBegin.c_str(),strDateEnd.c_str());
        /*
        * nLevel 内部代表表中Code字段
        */
        sprintf(buf,"Select count(*) from SYSTEM_EVENT_INFO where CODE = %d and TIME >='%s' and TIME <='%s'",nLevel, strDateBegin.c_str(),strDateEnd.c_str());
    }

    String sql(buf);

    int nRet=0;
    nRet=getIntFiled(sql);

    return nRet;
}
/*
	功能：	获得系统日志信息(分页技术)
	参数：	strDateBegin	开始时间
			strDateEnd		结束时间
			nLevel			日志级别(0:所有, 1:普通, 2:警告, 3:错误)
 */
String CSeekpaiDB::GetSysEvent(SEARCH_ITEM& search_item,int nPageSize)
{
    String strDateBegin = GetTime(search_item.uBeginTime);
    String strDateEnd = GetTime(search_item.uEndTime);

    int nPage = search_item.uPage;
    int nSortId = search_item.uSortId;
    int nSortKind = search_item.uSortKind;
    int nLevel = search_item.uLevel;

    String response;
    if(nPage<1 ||nPageSize<0)
    {
#ifdef _DEBUG
        printf("nPage or nPageSize is error! \n");
#endif
        return response;
    }

    char SortKind[2][5]= {"ASC","DESC"};
    char SortId[4][6]= {"ID","TIME","EVENT","LEVEL"};

    char buf[BUFF_SIZE]= {0};


    if(nLevel == ALARM_CODE_ALL)	/* 所有日志 */
    {
        sprintf(buf,"Select ID,TIME,LEVEL,EVENT from SYSTEM_EVENT_INFO where TIME >='%s' and TIME <='%s' ORDER BY %s %s limit %d,%d"
                ,strDateBegin.c_str(),strDateEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
    }
    else	/* 查询指定级别日志 */
    {
        /*
        sprintf(buf,"Select ID,TIME,LEVEL,EVENT from SYSTEM_EVENT_INFO where TIME >='%s' and TIME <='%s' and LEVEL=%d ORDER BY %s %s limit %d,%d"
        	,strDateBegin.c_str(),strDateEnd.c_str(),nLevel,SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
        */
        /*
        * nLevel 内部代表表中Code字段
        */
        sprintf(buf,"Select ID,TIME,LEVEL,EVENT from SYSTEM_EVENT_INFO where TIME >='%s' and TIME <='%s' and CODE=%d ORDER BY %s %s limit %d,%d"
                ,strDateBegin.c_str(),strDateEnd.c_str(),nLevel,SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
    }


    //获得页面数据总数
    int nCount=0;
    if(nPage==1)
    {
        nCount=GetSystEventCount(nLevel,strDateBegin,strDateEnd);
        int nNum=nCount % nPageSize;
        if(nNum>0)
            nCount=nCount/nPageSize+1;
        else
            nCount=nCount/nPageSize;

    }

    String sql(buf);

//#ifdef _DEBUG
    printf("%s \n",sql.c_str());
//#endif

    MysqlQuery q = execQuery(sql);

    printf("nCount=%d, numFileds=%d\n",nCount,q.numFileds());

    response.append((char*)&nCount,sizeof(nCount));


    while(!q.eof())
    {
        RECORD_LOG log;
        log.uSeq=q.getUnIntFileds("ID");
		String strTime = q.getStringFileds("TIME");
        log.uTime = MakeTime(strTime);
        log.uCode = q.getIntFileds("LEVEL");
        String text = q.getStringFileds("EVENT");

        memcpy(log.chText,text.c_str(),text.size());
        response.append((char*)&log,sizeof(log));

        q.nextRow();
    }
    q.finalize();

    return response;
}

//删除录象信息
int CSeekpaiDB::DeleteRecord(unsigned int uBegin,unsigned int uEnd)
{
    char buf[BUFF_SIZE]= {0};

    String strBegin,strEnd,strVideo;

    //事件删除同时删除相应的录象和快照
    //strBegin = GetTime(uBegin);
    //strEnd = GetTime(uEnd);
    sprintf(buf,"select PATH  from VIDEO_FILE_INFO  where ID >=%u and ID <=%u",uBegin,uEnd);


    String sqlSel(buf);

    MysqlQuery q = execQuery(sqlSel);
    //获得录象列表

    while(!q.eof())
    {
        strVideo = q.getStringFileds("PATH");
        // 如果文件存在
        if(access(strVideo.c_str(),F_OK) == 0)//删除录象文件
        {
            remove(strVideo.c_str());
            sync();
        }

        q.nextRow();
    }
    q.finalize();

    memset(buf,0,BUFF_SIZE);
    //更新事件数据库中的内容

    sprintf(buf,"delete from  VIDEO_FILE_INFO where ID >=%u and ID <=%u",uBegin,uEnd);

    String sqlDel(buf);

    if(execSQL(sqlDel)!=0)
    {
        printf("Delete VIDEO error! sqlDel=%s\n",sqlDel.c_str());
        return SRIP_ERROR_USER_FAILE;
    }

    printf("Delete VIDEO ok \n");
    return SRIP_OK;
}

//删除log记录
int CSeekpaiDB::DeleteLog(unsigned int uBegin,unsigned int uEnd,int nKind)
{
    char buf[BUFF_SIZE]= {0};
    if(nKind==0)
    {
        String strBegin = GetTime(uBegin);
        String strEnd = GetTime(uEnd);
        printf("Delete uBegin=%s,uEnd=%s \n",strBegin.c_str(),strEnd.c_str());
        sprintf(buf,"Delete from SYSTEM_EVENT_INFO where TIME >='%s' and TIME <='%s'",strBegin.c_str(),strEnd.c_str());
    }
    else if(nKind==1)
    {
        printf("Delete uBegin=%u,uEnd=%u \n",uBegin,uEnd);
        sprintf(buf,"Delete from SYSTEM_EVENT_INFO where ID>=%u and ID<=%u",uBegin,uEnd);
    }

    String sql(buf);

    if(execSQL(sql)!=0)
    {
        printf("Delete SYSTEM_EVENT_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    printf("DeleteLog ok\n");

    return SRIP_OK;
}



/*
	Purpose:	插入事故信息
	Parameter:	nChannel	-	车道
				nKind		-	类型
				strNumber	-	车牌
				strDateTime	-	记录时间
				strPath		-	记录路径
	Comment:	由视频服务自己生成视频，数据库只存放路径
 */

int CSeekpaiDB::SaveTraEvent(int nChannel,RECORD_EVENT& event,unsigned int uPicId,unsigned int uVideoId,bool bSendEventToCenter,int nRecordType)
{
	//LogNormal("uVideoBeginTime=%lld,uEventBeginTime=%lld\n",event.uVideoBeginTime,event.uEventBeginTime);
    //////////////////////////////////
    char buf[BUFF_SIZE]= {0};

    String strVideoBeginTime = GetTime(event.uVideoBeginTime);
    String strVideoEndTime = GetTime(event.uVideoEndTime);

    String strPath = event.chVideoPath;
    String strPicPath(event.chPicPath);
    String strPlace = event.chPlace;

    String strBeginTime = GetTime(event.uEventBeginTime);
    String strEndTime = GetTime(event.uEventEndTime);
    UINT32 uEventId = 0;

	bool bStatus = 0;//是否已发送
	#ifdef SENSITIVE
	bStatus = 1;
	#endif

    sprintf(buf,"Insert into TRAFFIC_EVENT_INFO(KIND,CHANNEL,ROAD,BEGIN_TIME,\
			BEGIN_MITIME,END_TIME,END_MITIME,PICSIZE,PICWIDTH,\
			PICHEIGHT,POSX,POSY,TEXT,STATUS,\
			PICPATH,BEGIN_VIDEO_TIME,BEGIN_VIDEO_MITIME,END_VIDEO_TIME,END_VIDEO_MITIME,\
			VIDEOPATH,COLOR,TYPE,SPEED,VIDEOSAVE,\
			PIC_ID,VIDEO_ID,COLORWEIGHT,COLORSECOND,COLORWEIGHTSECOND,\
			COLORTHIRD,COLORWEIGHTTHIRD,EVENT_ID,DIRECTION) \
			values(%d,%d,%d,'%s',\
			%d,'%s',%d,%d,%d,\
			%d,%d,%d,'%s',%d,\
			'%s','%s',%d,'%s',%d,\
			'%s',%d,%d,%d,%d,\
			%u,%u,%d,%d,%d,\
			%d,%d,%u,%d)",\
            event.uCode,nChannel,event.uRoadWayID,strBeginTime.c_str(),\
            event.uMiEventBeginTime,strEndTime.c_str(),event.uMiEventEndTime,event.uPicSize,event.uPicWidth,\
            event.uPicHeight,event.uPosX,event.uPosY,event.chText,bStatus,\
            strPicPath.c_str(),strVideoBeginTime.c_str(),event.uMiVideoBeginTime,strVideoEndTime.c_str(),event.uMiVideoEndTime,\
            strPath.c_str(),event.uColor1,event.uType,event.uSpeed,nRecordType,\
            uPicId,uVideoId,event.uWeight1,event.uColor2,event.uWeight2,\
            event.uColor3,event.uWeight3,uEventId,event.uDirection);

    String sql(buf);
	printf("SaveTraEvent sql=%s\n",buf);

    if(execSQL(sql,&event.uSeq)!=0)
    {
        LogError("保存事件信息失败!\n");

        return SRIP_ERROR_USER_FAILE;
    }

    event.uChannelID = nChannel;

    SRIP_DETECT_HEADER sDHeader;
    sDHeader.uChannelID = GetCameraID(nChannel);
    sDHeader.uDetectType = MIMAX_EVENT_REP;
    sDHeader.uRealTime = 0x00000001;

    //推送实时事件给中心端
	if(bSendEventToCenter)
    g_skpChannelCenter.SendTraEvent(sDHeader,event);
	
	//LogNormal("nRecordType:%d \n",nRecordType);
	if(nRecordType == 1)//录像已经完成
	{
		if(g_nServerType == 7)
		{
			std::string strVideoFileName = strPath;
			String strTmpPath = "ftp://"+g_ServerHost;
			strVideoFileName.erase(0,strTmpPath.size());

			string strDataPath = "/home/road/red";
			if (IsDataDisk())
			{
				strDataPath = "/detectdata/red";
			}
			strVideoFileName = strDataPath + strVideoFileName;

			AddVideoRecord(strVideoFileName,0,sDHeader.uChannelID);
		}
	}

    return SRIP_OK;
}


//删除事件信息
int CSeekpaiDB::DelTraEvent(unsigned int uBegin,unsigned int uEnd,int nKind)
{
    char buf[BUFF_SIZE]= {0};

    String strBegin,strEnd,strVideo,strPic;

    //事件删除同时删除相应的录象和快照
    if(nKind==0)
    {
        strBegin = GetTime(uBegin);
        strEnd = GetTime(uEnd);
        sprintf(buf,"select PICPATH,VIDEOPATH from TRAFFIC_EVENT_INFO where BEGIN_TIME >='%s' and BEGIN_TIME <='%s'",strBegin.c_str(),strEnd.c_str());
    }
    else if(nKind==1)
    {
        sprintf(buf,"select PICPATH,VIDEOPATH from TRAFFIC_EVENT_INFO where ID>=%u and ID<=%u",uBegin,uEnd);
    }
    String sqlSel(buf);

    MysqlQuery q = execQuery(sqlSel);
    //获得录象列表

    while(!q.eof())
    {
        strPic = q.getStringFileds("PICPATH");
        // 如果文件存在
        if(access(strPic.c_str(),F_OK) == 0)//删除事件快照文件
        {
            FILE* fp = fopen(strPic.c_str(),"wb");
			if(fp)
			{
				fwrite("",0,1,fp);
				fclose(fp);
			}
        }

        strVideo = q.getStringFileds("VIDEOPATH");
        // 如果文件存在
        if(access(strVideo.c_str(),F_OK) == 0)//删除事件录象文件
        {
            FILE* fp = fopen(strVideo.c_str(),"wb");
			if(fp)
			{
				fwrite("",0,1,fp);
				fclose(fp);
			}
        }

        q.nextRow();
    }
    q.finalize();

    memset(buf,0,BUFF_SIZE);
    //删除事件数据库中内容
    if(nKind==0)
    {
        //	printf("Delete uBegin=%s,uEnd=%s \n",strBegin.c_str(),strEnd.c_str());
        sprintf(buf,"Delete from TRAFFIC_EVENT_INFO where BEGIN_TIME >='%s' and BEGIN_TIME <='%s'",strBegin.c_str(),strEnd.c_str());
    }
    else if(nKind==1)
    {
        //	printf("Delete uBegin=%d,uEnd=%d \n",uBegin,uEnd);
        sprintf(buf,"Delete from TRAFFIC_EVENT_INFO where ID>=%u and ID<=%u",uBegin,uEnd);
    }

    String sqlDel(buf);

    if(execSQL(sqlDel)!=0)
    {
        printf("Delete TRAFFIC_EVENT_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    printf("Delete event ok \n");
    return SRIP_OK;
}

/* 函数介绍：中心服务器获取事件信息
 * 输入参数：uBegin-开始时间或序号，uEnd-结束时间或序号，nKind-按时间、序号或者是推送非实时记录，nType-是否需要推送快照图像
 * 输出参数：无
 * 返回值：无
 */

void CSeekpaiDB::GetTraEvent(unsigned int uBegin,unsigned int uEnd)
{
	String strTimeBeg = GetTime(uBegin);
    String strTimeEnd = GetTime(uEnd);

	LogNormal("%s-%s",strTimeBeg.c_str(),strTimeEnd.c_str());

	char buf[BUFF_SIZE]= {0};

	sprintf(buf,"update TRAFFIC_EVENT_INFO set STATUS = 0 where BEGIN_TIME >= '%s' and BEGIN_TIME <= '%s'",strTimeBeg.c_str(),strTimeEnd.c_str());

	String sqlPlate(buf);

    if(execSQL(sqlPlate)!=0)
    {
        printf("update TRAFFIC_EVENT_INFO error! \n");
    }
}

//获得查询记录数量
int CSeekpaiDB::GetTraEventCount(int nChannel,int nRoadIndex,int nType,String strDateBegin,String strDateEnd)
{
    char buf[BUFF_SIZE]= {0};

    if(nChannel == 0)
    {
        if(nType == 0)		// 所有类型
        {
            sprintf(buf,"SELECT count(*) FROM TRAFFIC_EVENT_INFO WHERE BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",strDateBegin.c_str(),strDateEnd.c_str());
        }
        else				// 特定类型
        {
            sprintf(buf,"SELECT count(*) FROM TRAFFIC_EVENT_INFO WHERE KIND = %d AND BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",nType, strDateBegin.c_str(),strDateEnd.c_str());
        }
    }
    else
    {
        if(nRoadIndex==0)
        {
            if(nType == 0)		// 所有类型
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_EVENT_INFO WHERE CHANNEL = %d AND BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",nChannel,strDateBegin.c_str(),strDateEnd.c_str());
            }
            else				// 特定类型
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_EVENT_INFO WHERE CHANNEL = %d AND KIND = %d AND BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",nChannel,nType, strDateBegin.c_str(),strDateEnd.c_str());
            }
        }
        else
        {
            if(nType == 0)		// 所有类型
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_EVENT_INFO WHERE CHANNEL = %d AND ROAD = %d AND BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",nChannel,nRoadIndex,strDateBegin.c_str(),strDateEnd.c_str());
            }
            else				// 特定类型
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_EVENT_INFO WHERE CHANNEL = %d AND ROAD = %d AND KIND = %d AND BEGIN_TIME>='%s' AND BEGIN_TIME<='%s'",nChannel,nRoadIndex,nType, strDateBegin.c_str(),strDateEnd.c_str());
            }
        }
    }

    String sql(buf);
    return getIntFiled(sql);
}

//获取事件信息
String CSeekpaiDB::GetEvent(SEARCH_ITEM& search_item, int nPageSize)
{
    String strDateBegin = GetTime(search_item.uBeginTime);
    String strDateEnd = GetTime(search_item.uEndTime);

    int nPage = search_item.uPage;
    int nSortId = search_item.uSortId;
    int nSortKind = search_item.uSortKind;
    int nChannel = search_item.uChannelId;
    int nRoadIndex = search_item.uRoadId;
    int type = search_item.uType;

    String response;

    if(nPage<1||nPageSize<0)
    {
#ifdef _DEBUG
        printf("nPage or nPageSize is Error! \n");
#endif
        return response;
    }

    char buf[BUFF_SIZE]= {0};



    char SortKind[2][5]= {"ASC","DESC"};
    char SortId[13][15]= {"ID","BEGIN_TIME","CHAN_PLACE","CHANNEL","ROAD",\
                            "DIRECTION","KIND","SPEED","COLOR","TYPE",\
                            "PICPATH","VIDEOPATH", "STATUS"};


    if(nChannel == 0)	/* 查询所有车道 */
    {
        if((int)type== 0) // 查询所有车道上所有事件类型
        {
            sprintf(buf,"Select ID,CHANNEL,ROAD,TEXT,BEGIN_TIME,KIND,\
					POSX,POSY,VIDEOPATH,CHAN_PLACE,PICPATH,\
					COLOR,TYPE,SPEED,DIRECTION,STATUS,COLORSECOND,COLORTHIRD \
				   from TRAFFIC_EVENT_INFO, CHAN_INFO \
				   where BEGIN_TIME>='%s' and BEGIN_TIME<='%s' and CHANNEL = CHAN_ID\
				   ORDER BY %s %s limit %d,%d",\
                    strDateBegin.c_str(), strDateEnd.c_str(),\
                    SortId[nSortId], SortKind[nSortKind], (nPage-1)*nPageSize, nPageSize);
        }
        else  //查询所有车道上某一事件类型
        {
            sprintf(buf,"Select ID,CHANNEL,ROAD,TEXT,BEGIN_TIME,KIND,\
					POSX,POSY,VIDEOPATH,CHAN_PLACE,PICPATH,\
					COLOR,TYPE,SPEED,DIRECTION,STATUS,COLORSECOND,COLORTHIRD  \
				   from TRAFFIC_EVENT_INFO, CHAN_INFO \
				   where KIND=%d and BEGIN_TIME>='%s' and BEGIN_TIME<='%s' and CHANNEL = CHAN_ID\
				   ORDER BY %s %s limit %d,%d",\
                    (int)type, strDateBegin.c_str(),strDateEnd.c_str(),\
                    SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
        }
    }
    else
    {
        if(nRoadIndex==0)//所有车道
        {
            if((int)type == 0) /* 查询某一车道上所有类型 */
            {
                sprintf(buf,"Select ID,CHANNEL,ROAD,TEXT,BEGIN_TIME,KIND,\
						POSX,POSY,VIDEOPATH,CHAN_PLACE,PICPATH,\
						COLOR,TYPE,SPEED,DIRECTION,STATUS,COLORSECOND,COLORTHIRD  \
						from TRAFFIC_EVENT_INFO, CHAN_INFO \
						where CHANNEL =%d and BEGIN_TIME>='%s' and BEGIN_TIME<='%s' and CHANNEL = CHAN_ID\
						ORDER BY %s %s limit %d,%d",\
                        nChannel, strDateBegin.c_str(), strDateEnd.c_str(),\
                        SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
            }
            else /* 查询某一车道上某一车牌 */
            {
                sprintf(buf,"Select ID,CHANNEL,ROAD,TEXT,BEGIN_TIME,KIND,\
						POSX,POSY,VIDEOPATH,CHAN_PLACE,PICPATH,\
						COLOR,TYPE,SPEED,DIRECTION,STATUS,COLORSECOND,COLORTHIRD  \
						from TRAFFIC_EVENT_INFO, CHAN_INFO \
						where CHANNEL=%d and KIND=%d and BEGIN_TIME>='%s' and BEGIN_TIME<='%s' and CHANNEL = CHAN_ID\
						ORDER BY %s %s limit %d,%d",\
                        nChannel, (int)type, strDateBegin.c_str(), strDateEnd.c_str(),\
                        SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
            }
        }
        else
        {
            if((int)type == 0) /* 查询某一车道上所有类型 */
            {
                sprintf(buf,"Select ID,CHANNEL,ROAD,TEXT,BEGIN_TIME,KIND,\
						POSX,POSY,VIDEOPATH,CHAN_PLACE,PICPATH,\
						COLOR,TYPE,SPEED,DIRECTION,STATUS,COLORSECOND,COLORTHIRD  \
						from TRAFFIC_EVENT_INFO, CHAN_INFO \
						where CHANNEL =%d and ROAD = %d and BEGIN_TIME>='%s' and BEGIN_TIME<='%s' and CHANNEL = CHAN_ID\
						ORDER BY %s %s limit %d,%d",\
                        nChannel,nRoadIndex,strDateBegin.c_str(),strDateEnd.c_str(),\
                        SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
            }
            else /* 查询某一车道上某一车牌 */
            {
                sprintf(buf,"Select ID,CHANNEL,ROAD,TEXT,BEGIN_TIME,KIND,\
				        POSX,POSY,VIDEOPATH,CHAN_PLACE,PICPATH,\
						COLOR,TYPE,SPEED,DIRECTION,STATUS,COLORSECOND,COLORTHIRD  \
						from TRAFFIC_EVENT_INFO, CHAN_INFO \
						where CHANNEL=%d and ROAD = %d and KIND=%d and BEGIN_TIME>='%s' and BEGIN_TIME<='%s' and CHANNEL = CHAN_ID\
						ORDER BY %s %s limit %d,%d",\
                        nChannel,nRoadIndex,(int)type,strDateBegin.c_str(),strDateEnd.c_str(),\
                        SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
            }
        }
    }

    //获得查询数量
    int nCount=0;
    if(nPage==1)
    {
        nCount=GetTraEventCount(nChannel,nRoadIndex,(int)type,strDateBegin,strDateEnd);
        int nNum=nCount % nPageSize;
        if(nNum>0)
            nCount=nCount/nPageSize+1;
        else
            nCount=nCount/nPageSize;
    }

    String sql(buf);
    MysqlQuery q = execQuery(sql);

//#ifdef _DEBUG
    printf("%s\r\n", buf);
//#endif

    printf("nCount=%d, numFileds=%d\n",nCount,q.numFileds());

    response.append((char*)&nCount,sizeof(nCount));

    while(!q.eof())
    {
        RECORD_EVENT event;
        event.uSeq=q.getUnIntFileds("ID");

        int nChannelId = q.getIntFileds("CHANNEL");
        memcpy(event.chReserved,&nChannelId,sizeof(int));//暂时借用保留字段

        event.uRoadWayID = q.getIntFileds("ROAD");

		String strText = q.getStringFileds("TEXT");
		memcpy(event.chText,strText.c_str(),strText.size());

        String strTime = q.getStringFileds("BEGIN_TIME");
        event.uEventBeginTime = MakeTime(strTime);

        event.uCode = q.getIntFileds("KIND");

        event.uPosX = q.getIntFileds("POSX");
        event.uPosY = q.getIntFileds("POSY");

        String strPath = q.getStringFileds("VIDEOPATH");
        memcpy(event.chVideoPath,strPath.c_str(),strPath.size());

        String strPlace = q.getStringFileds("CHAN_PLACE");
        memcpy(event.chPlace,strPlace.c_str(),strPlace.size());

        String strPicPath = q.getStringFileds("PICPATH");
//		strPicPath.erase(0,g_strPic.size());
        memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size());

        event.uColor1 = q.getIntFileds("COLOR");
        event.uType = q.getIntFileds("TYPE");

        event.uSpeed = q.getIntFileds("SPEED");
        event.uDirection = q.getIntFileds("DIRECTION");
        event.uStatusType = q.getIntFileds("STATUS");
        event.uColor2 = q.getIntFileds("COLORSECOND");
        event.uColor3 = q.getIntFileds("COLORTHIRD");

        response.append((char*)&event,sizeof(event));

        q.nextRow();
    }

    q.finalize();

    return response;
}

//读取统计信息量
int CSeekpaiDB::GetStatisticCount(int nChannel, int nRoadIndex, int nType, String strDateBegin,String strDateEnd)
{
    char buf[BUFF_SIZE]= {0};

    if(nChannel == 0)
    {
        if(nType == 99)	// 查询所有
        {
            sprintf(buf,"SELECT count(*) FROM TRAFFIC_STATISTIC_INFO WHERE TIME>='%s' AND TIME<='%s'",strDateBegin.c_str(),strDateEnd.c_str());
        }
        else			// 查询特定
        {
            sprintf(buf,"SELECT count(*) FROM TRAFFIC_STATISTIC_INFO WHERE KIND = %d AND TIME>='%s' AND TIME<='%s'", nType, strDateBegin.c_str(),strDateEnd.c_str());
        }
    }
    else
    {
        if(nRoadIndex==0)
        {
            if (nType == 99)// 查询所有
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_STATISTIC_INFO WHERE CHANNEL = %d AND TIME>='%s' AND TIME<='%s'",nChannel,strDateBegin.c_str(),strDateEnd.c_str());
            }
            else			// 查询特定
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_STATISTIC_INFO WHERE CHANNEL = %d AND KIND = %d AND TIME>='%s' AND TIME<='%s'",nChannel,nType,strDateBegin.c_str(),strDateEnd.c_str());
            }
        }
        else
        {
            if (nType == 99)// 查询所有
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_STATISTIC_INFO WHERE CHANNEL = %d AND ROAD = %d AND TIME>='%s' AND TIME<='%s'",nChannel,nRoadIndex,strDateBegin.c_str(),strDateEnd.c_str());
            }
            else			// 查询特定
            {
                sprintf(buf,"SELECT count(*) FROM TRAFFIC_STATISTIC_INFO WHERE CHANNEL = %d AND ROAD = %d AND KIND = %d AND TIME>='%s' AND TIME<='%s'",nChannel,nRoadIndex,nType,strDateBegin.c_str(),strDateEnd.c_str());
            }
        }
    }

    String sql(buf);
    return getIntFiled(sql);
}

//获取统计信息
String CSeekpaiDB::GetStatistic(SEARCH_ITEM& search_item,int nPageSize)
{
    String strTimeBegin = GetTime(search_item.uBeginTime);
    String strTimeEnd = GetTime(search_item.uEndTime);

    int nPage = search_item.uPage;
    int nSortId = search_item.uSortId;
    int nSortKind = search_item.uSortKind;
    int nChannel = search_item.uChannelId;
    int nRoadIndex = search_item.uRoadId;
    int type = search_item.uType + 99;
  /*******************************************
  *    0、所有统计
  *  100、车道流量
  *  101、平均车速
  *  102、平均车道占有率
  *  103、队列长度
  *  104、平均车头间距
  *  105、车辆分型
  *******************************************/
    printf("type=%d\n",type);
    if(nSortId > 4) //排序归一化，按Value值排序
    {
        nSortId = 4;
    }

    String response;

    if(nPage<1||nPageSize<0)
    {
#ifdef _DEBUG
        printf("nPage or nPageSize is Error! \n");
#endif
        return response;
    }

    char buf[BUFF_SIZE]= {0};

    char SortKind[2][5]= {"ASC","DESC"};
    char SortId[5][16]= {"S_ID","TIME","CHANNEL","ROAD","VALUE&(0xffff)"};

    int nSizePage = nPageSize*6;  //only for type<99
    if(nChannel == 0)	// 查询所有车道
    {
        if(type==99)
            sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from TRAFFIC_STATISTIC_INFO where TIME>='%s' and TIME<='%s' ORDER BY %s %s limit %d,%d",strTimeBegin.c_str(),strTimeEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nSizePage,nSizePage);
        else
            sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from TRAFFIC_STATISTIC_INFO where KIND=%d and TIME>='%s' and TIME<='%s' ORDER BY %s %s limit %d,%d",type,strTimeBegin.c_str(),strTimeEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
    }
    else
    {
        if(nRoadIndex==0)
        {
            if((int)type == 99) // 查询某一车道上所有统计信息
            {
                sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from TRAFFIC_STATISTIC_INFO where CHANNEL =%d and TIME>='%s' and TIME<='%s' ORDER BY %s %s limit %d,%d"
                        ,nChannel,strTimeBegin.c_str(),strTimeEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nSizePage,nSizePage);
            }
            else // 查询某一车道上某一统计信息
            {
                sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from \
				TRAFFIC_STATISTIC_INFO where CHANNEL =%d and KIND=%d and TIME>='%s' and TIME<='%s' ORDER BY %s %s limit %d,%d"
                        ,nChannel,(int)type,strTimeBegin.c_str(),strTimeEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
            }
        }
        else
        {
            if((int)type == 99) //查询某一车道上所有统计信息
            {
                sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from TRAFFIC_STATISTIC_INFO where CHANNEL =%d and ROAD = %d and TIME>='%s' and TIME<='%s' ORDER BY %s %s limit %d,%d"
                        ,nChannel,nRoadIndex,strTimeBegin.c_str(),strTimeEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nSizePage,nSizePage);
            }
            else // 查询某一车道上某一统计信息
            {
                sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from \
				TRAFFIC_STATISTIC_INFO where CHANNEL =%d and ROAD = %d and KIND=%d and TIME>='%s' and TIME<='%s' ORDER BY %s %s limit %d,%d"
                        ,nChannel,nRoadIndex,(int)type,strTimeBegin.c_str(),strTimeEnd.c_str(),SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
            }
        }
    }

    String sql(buf);
    MysqlQuery q = execQuery(sql);
    //获得查询数量
    int nCount=0;
    if(nPage==1)
    {
        nCount=GetStatisticCount(nChannel,nRoadIndex,(int)type,strTimeBegin,strTimeEnd);

        printf("===========nCount=%d\n",nCount);

        if(type<100)
            nCount = nCount/6;

        int nNum=nCount % nPageSize;
        if(nNum>0)
            nCount=nCount/nPageSize+1;
        else
            nCount=nCount/nPageSize;
    }


    printf("nCount=%d, numFileds=%d,sql=%s\n",nCount,q.numFileds(),sql.c_str());

    response.append((char*)&nCount,sizeof(nCount));

    std::stringstream stream;
    std::string sub_str;

    String strFlux,strSpeed,strQueue,strOccupancy,strSpace;

    RECORD_STATISTIC statistic;
    unsigned int i = 0;
    int nChannelId,nRoadId;
    while(!q.eof())
    {
        bool bRead = true;
        if(type<100)
        {
            if(i%6==0)
            {
                bRead = true;
            }
            else
            {
                bRead = false;
            }
        }

        if(bRead)
        {
            statistic.uSeq=q.getUnIntFileds("ID");

            String strTime = q.getStringFileds("TIME");
            statistic.uTime = MakeTime(strTime);

            nChannelId = q.getIntFileds("CHANNEL");
            memcpy(statistic.chReserved,&nChannelId,sizeof(int));//暂时借用保留字段

            nRoadId = q.getIntFileds("ROAD");
            memcpy(statistic.chReserved+sizeof(int),&nRoadId,sizeof(int));//暂时借用保留字段

            statistic.uStatTimeLen = q.getIntFileds("STATTIMELEN");

			unsigned int uRoadType = q.getIntFileds("TYPE");
			unsigned int uRoadIndex = nRoadId;
			uRoadIndex = uRoadIndex<<16;
			statistic.uRoadType[nRoadId-1] = uRoadIndex|uRoadType;
        }

        int nKind = q.getIntFileds("KIND");
        unsigned int value = (unsigned int)(q.getFloatFileds(6)+0.5);
        //	printf("nRoadId=%d,nKind=%d,value=%d\n",nRoadId,nKind,value);
		
        switch(nKind)
        {
        case 100:
            statistic.uFlux[nRoadId-1] = value;
			statistic.uFluxCom[nRoadId-1] = 0;
            break;
        case 101:
            statistic.uSpeed[nRoadId-1] = value;
            break;
        case 102:
            statistic.uQueue[nRoadId-1] = value;
            break;
        case 103:
            statistic.uOccupancy[nRoadId-1] = value;
            break;
        case 104:
            statistic.uSpace[nRoadId-1] = value;
            break;
        case 105:
            statistic.uFluxCom[nRoadId-1] = value;
            break;
        }

        if(type<100)
        {
            if((i+1)%6==0)
            {
                bRead = true;
            }
            else
            {
                bRead = false;
            }
        }

        if(bRead)
            response.append((char*)&statistic,sizeof(statistic));

        q.nextRow();
        i++;
    }
    q.finalize();

    return response;
}

//检测器之间通讯，保存上一个点位发送过来的车牌信息（用于区间测速）
int CSeekpaiDB::MvsCommunicationSaveRecPlate(RECORD_PLATE& plate)
{
	char buf[BUFF_SIZE] = {0};
	int nStatus = 0;
	int nSaveState = 0;//录像保存状态,初始值为0表示没有完成录像
	unsigned int uPicId = 0;
	int nChannel = plate.uChannelID;
	String strTime = GetTime(plate.uTime);

	String strVideoPath(plate.chVideoPath);//SMALLPICPATH中用于存放车牌事件录像
	String strPicPath(plate.chPicPath);

	String strTime2 = GetTime(plate.uTime2);
	sprintf(buf,"Insert into NUMBER_PLATE_INFO_RECV(NUMBER,COLOR,CREDIT,CHANNEL,\
				ROAD,TIME,MITIME,SMALLPICSIZE,SMALLPICWIDTH,\
				SMALLPICHEIGHT,PICSIZE,PICWIDTH,PICHEIGHT,POSLEFT,\
				POSTOP,POSRIGHT,POSBOTTOM,PICPATH,SMALLPICPATH,\
				STATUS,CARCOLOR,TYPE,SPEED,PIC_ID,\
				CARCOLORWEIGHT,CARCOLORSECOND,CARCOLORWEIGHTSECOND,CARNUMBER_TYPE,TYPE_DETAIL,\
				PECCANCY_KIND,FACTORY,DIRECTION,VIDEOPATH,VIDEOSAVE,TIMESECOND,MITIMESECOND,PLACE) \
				values('%s',%d,%d,%d,\
				%d,'%s',%d,%d,%d,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s','%s',\
				%d,%d,%d,%d,%u,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s',%d,'%s',%d,'%s')",\
				plate.chText,plate.uColor,plate.uCredit,nChannel,\
				plate.uRoadWayID,strTime.c_str(),plate.uMiTime,plate.uSmallPicSize,plate.uSmallPicWidth,\
				plate.uSmallPicHeight,plate.uPicSize,plate.uPicWidth,plate.uPicHeight,plate.uPosLeft,\
				plate.uPosTop,plate.uPosRight,plate.uPosBottom,strPicPath.c_str(),"",\
				nStatus,plate.uCarColor1,plate.uType,plate.uSpeed,uPicId,\
				plate.uWeight1,plate.uCarColor2,plate.uWeight2,plate.uPlateType,plate.uTypeDetail,\
				plate.uViolationType,plate.uCarBrand,plate.uDirection,strVideoPath.c_str(),nSaveState,strTime2.c_str(),plate.uMiTime2,plate.chPlace);

	String sql(buf);
	if(execSQL(sql,&plate.uSeq)!=0)
	{
		LogError("MvsCommunicationSaveRecPlate insert error\n");
		return SRIP_ERROR_USER_FAILE;
	}

	return SRIP_OK;
}

//检测器之间通讯，保存本点位和上一个点位之间的车辆区间速度相关信息（用于区间测速）
int CSeekpaiDB::MvsCommunicationSaveRegionSpeed(RECORD_PLATE& plate)
{
	char buf[BUFF_SIZE] = {0};
	int nStatus = 0;
	int nSaveState = 0;//录像保存状态,初始值为0表示没有完成录像
	unsigned int uPicId = 0;
	int nChannel = plate.uChannelID;
	String strTime = GetTime(plate.uTime);

	String strVideoPath(plate.chVideoPath);//SMALLPICPATH中用于存放车牌事件录像
	String strPicPath(plate.chPicPath);

	String strTime2 = GetTime(plate.uTime2);
	sprintf(buf,"Insert into NUMBER_PLATE_INFO(NUMBER,COLOR,CREDIT,CHANNEL,\
				ROAD,TIME,MITIME,SMALLPICSIZE,SMALLPICWIDTH,\
				SMALLPICHEIGHT,PICSIZE,PICWIDTH,PICHEIGHT,POSLEFT,\
				POSTOP,POSRIGHT,POSBOTTOM,PICPATH,SMALLPICPATH,\
				STATUS,CARCOLOR,TYPE,SPEED,PIC_ID,\
				CARCOLORWEIGHT,CARCOLORSECOND,CARCOLORWEIGHTSECOND,CARNUMBER_TYPE,TYPE_DETAIL,\
				PECCANCY_KIND,FACTORY,DIRECTION,VIDEOPATH,VIDEOSAVE,TIMESECOND,MITIMESECOND) \
				values('%s',%d,%d,%d,\
				%d,'%s',%d,%d,%d,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s','%s',\
				%d,%d,%d,%d,%u,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s',%d,'%s',%d)",\
				plate.chText,plate.uColor,plate.uCredit,nChannel,\
				plate.uRoadWayID,strTime.c_str(),plate.uMiTime,plate.uSmallPicSize,plate.uSmallPicWidth,\
				plate.uSmallPicHeight,plate.uPicSize,plate.uPicWidth,plate.uPicHeight,plate.uPosLeft,\
				plate.uPosTop,plate.uPosRight,plate.uPosBottom,strPicPath.c_str(),"",\
				nStatus,plate.uCarColor1,plate.uType,plate.uSpeed,uPicId,\
				plate.uWeight1,plate.uCarColor2,plate.uWeight2,plate.uPlateType,plate.uTypeDetail,\
				plate.uViolationType,plate.uCarBrand,plate.uDirection,strVideoPath.c_str(),nSaveState,strTime2.c_str(),plate.uMiTime2);

	String sql(buf);
	if(execSQL(sql,&plate.uSeq)!=0)
	{
		LogError("MvsCommunicationSaveRegionSpeed insert error\n");

		/*system("myisamchk -o /var/lib/mysql/bocom_db/NUMBER_PLATE_INFO");    //sql_myisamchk
		int fd = open("sql_myisamchk",O_RDWR);
		write(fd, "sql\n", 4);
		close(fd);*/

		return SRIP_ERROR_USER_FAILE;
	}
	SRIP_DETECT_HEADER sDHeader;
	sDHeader.uChannelID = GetCameraID(nChannel);
	sDHeader.uRealTime = 0x00000001;//此时均设为实时车牌
	sDHeader.uDetectType = MIMAX_PLATE_REP;
	g_skpChannelCenter.SendPlateInfo(sDHeader,plate,NULL);
	return SRIP_OK;
}

// DSP相机，违章作为事件处理
bool CSeekpaiDB::IsEvent(UINT32 uViolationType)
{
	bool bRet = false;
#ifdef LS_QINGTIAN_IVAP
	if (g_nServerType == 5)
#else
	if (g_nHasCenterServer == 1 && (g_nServerType == 0 || g_nServerType == 3))
#endif
	{
		if (g_nDetectMode == 2) //Dsp模式
		{
			if ((uViolationType > DETECT_RESULT_ALL && uViolationType < DETECT_RESULT_RED_LIGHT_VIOLATION && (uViolationType != DETECT_RESULT_EVENT_GO_FAST) )
				|| uViolationType == DETECT_RESULT_PARKING_VIOLATION || uViolationType == DETECT_RESULT_NO_STOP)
			{
				bRet = true;
			}
		}
	}
	return bRet;
}

//保存车牌信息
int CSeekpaiDB::SavePlate(int nChannel,RECORD_PLATE& plate,unsigned int uPicId,SYN_CHAR_DATA* syn_char_data)
{
	int nRet = 0;
	if (IsEvent(plate.uViolationType) == true)
	{
		//LogNormal("Type:%d\n",plate.uViolationType);
		nRet = SaveEventNoPlate(nChannel, plate, uPicId, syn_char_data);
	}
	else
	{	
		//LogNormal("Type:%d\n",plate.uViolationType);
		nRet = SavePlateNoEvent(nChannel, plate, uPicId, syn_char_data);
	}
	return nRet;
}

//保存事件信息
int CSeekpaiDB::SaveEventNoPlate(int nChannel,RECORD_PLATE& plate,unsigned int uPicId,SYN_CHAR_DATA* syn_char_data)
{
	//LogNormal("uTime=%lld,uTime2=%lld\n",plate.uTime,plate.uTime2);
	char buf[BUFF_SIZE] = {0};
	int nStatus = 0;
	int nSaveState = 0;//录像保存状态,初始值为0表示没有完成录像
	String strTime = GetTime(plate.uTime);
	String strBeginTime = GetTime(plate.uRedLightBeginTime);
	String strEndTime = GetTime(plate.uRedLightEndTime);

	String strVideoPath(plate.chVideoPath);//SMALLPICPATH中用于存放车牌事件录像
	String strPicPath(plate.chPicPath);

	String strTime2 = GetTime(plate.uTime2);

	plate.uChannelID = nChannel;

	SRIP_DETECT_HEADER sDHeader;
	sDHeader.uChannelID = GetCameraID(nChannel);
	sDHeader.uRealTime = 0x00000001;//此时均设为实时车牌

	//////////////////行人和非机动车需要封装成事件接口传送给中心端
	RECORD_EVENT  event;
	event.uSeq  = plate.uSeq;
	event.uEventBeginTime = plate.uTime;
	event.uMiEventBeginTime = plate.uMiTime;
	event.uTime2 = plate.uTime2;
	event.uMiTime2 = plate.uMiTime2;
	event.uEventEndTime = plate.uTime+5;
	event.uType = plate.uType;
	event.uRoadWayID = plate.uRoadWayID;
	event.uPicSize = plate.uPicSize;
	event.uPicWidth = plate.uPicWidth;
	event.uPicHeight = plate.uPicHeight;
	event.uPosX = (plate.uPosLeft+plate.uPosRight)/2;
	event.uPosY = (plate.uPosTop+plate.uPosBottom)/2;
	event.uColor1 = plate.uCarColor1;
	event.uSpeed = plate.uSpeed;
	event.uColor2 = plate.uCarColor2;
	event.uWeight1 = plate.uWeight1;
	event.uWeight2 = plate.uWeight2;
	event.uDirection = plate.uDirection;
	event.uDetailCarType = plate.uTypeDetail;
	event.uChannelID = plate.uChannelID;
	memcpy(event.chPlace,plate.chPlace,sizeof(plate.chPlace));

	if(plate.uViolationType == 0)
	{
		if(event.uType==PERSON_TYPE)
			event.uCode  = DETECT_RESULT_EVENT_PERSON_APPEAR;//行人出现
		else if(event.uType==OTHER_TYPE) //非机车
			event.uCode  =  DETECT_RESULT_EVENT_WRONG_CHAN;//非机动车出现
	}
	else
	{
		event.uCode = plate.uViolationType;
	}
	//非机动车出现 强制转换成行人出现
	if (plate.uViolationType == DETECT_RESULT_EVENT_WRONG_CHAN)
	{
		event.uCode = DETECT_RESULT_EVENT_PERSON_APPEAR;
	}
	if (plate.uViolationType == DETECT_RESULT_PARKING_VIOLATION || plate.uViolationType == DETECT_RESULT_NO_STOP)
	{
		event.uCode = DETECT_RESULT_EVENT_STOP;
	}
	if (plate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION)
	{
		event.uCode = DETECT_RESULT_EVENT_GO_AGAINST;
	}
	if (plate.uViolationType == DETECT_RESULT_PRESS_WHITELINE || plate.uViolationType == DETECT_RESULT_PRESS_LINE)
	{
		event.uCode = DETECT_RESULT_PRESS_LINE;
	}
	memcpy(event.chPicPath,plate.chPicPath,sizeof(plate.chPicPath));
	memcpy(event.chVideoPath,plate.chVideoPath,sizeof(plate.chVideoPath));
	memcpy(event.chText,plate.chText,sizeof(plate.chText));

	SaveTraEvent(nChannel,event,0,0,true);

	//sDHeader.uDetectType = MIMAX_EVENT_REP;
	////if(g_nServerType != 7)
	//{
	//	/*if (g_nServerType == 13)
	//	{
	//	SaveTraEvent(nChannel,event,0,0,true);
	//	}*/
	//	if (g_nServerType != 13)
	//	{
	//		g_skpChannelCenter.SendTraEvent(sDHeader,event,syn_char_data,true);
	//	}

	//}

	return SRIP_OK;
}

//保存车牌信息
int CSeekpaiDB::SavePlateNoEvent(int nChannel,RECORD_PLATE& plate,unsigned int uPicId,SYN_CHAR_DATA* syn_char_data)
{

	//LogNormal("SavePlateNoEvent locationId:%s, KakouItem:%s ", plate.szLoctionID, plate.szKaKouItem);	

	//LogNormal("uTime=%lld,uTime2=%lld\n",plate.uTime,plate.uTime2);
    char buf[BUFF_SIZE] = {0};
    int nStatus = 0;
	int nSaveState = 0;//录像保存状态,初始值为0表示没有完成录像
    String strTime = GetTime(plate.uTime);
	String strBeginTime = GetTime(plate.uRedLightBeginTime);
	String strEndTime = GetTime(plate.uRedLightEndTime);

    String strVideoPath(plate.chVideoPath);//SMALLPICPATH中用于存放车牌事件录像
    String strPicPath(plate.chPicPath);

    String strTime2 = GetTime(plate.uTime2);


	bool bSendToServer = true;
	if (( ((23 == g_nServerType || 26 == g_nServerType) && (g_Kafka.uSwitchUploading == 0)) || (g_nSendViolationOnly == 1) )&& (plate.uViolationType == 0))
	{
		nStatus = 1;
		bSendToServer = false;
	}

	if( (13 == g_nServerType) &&(g_nSendViolationOnly == 1)&&(plate.uViolationType == 0))
	{
		nStatus = 1;
		bSendToServer = false;
	}

	 printf("\n\n-SaveDBStart----Num:%s, platepos:%d,%d,%d,%d\n", plate.chText,plate.uPosLeft,  plate.uPosTop,  plate.uPosRight,  plate.uPosBottom);

	if(g_nGongJiaoMode == 1)
	{
		sprintf(buf,"Insert into NUMBER_PLATE_INFO(NUMBER,COLOR,CREDIT,CHANNEL,\
				ROAD,TIME,MITIME,SMALLPICSIZE,SMALLPICWIDTH,\
				SMALLPICHEIGHT,PICSIZE,PICWIDTH,PICHEIGHT,POSLEFT,\
				POSTOP,POSRIGHT,POSBOTTOM,PICPATH,SMALLPICPATH,\
				STATUS,CARCOLOR,TYPE,SPEED,PIC_ID,\
				CARCOLORWEIGHT,CARCOLORSECOND,CARCOLORWEIGHTSECOND,CARNUMBER_TYPE,TYPE_DETAIL,\
				PECCANCY_KIND,FACTORY,DIRECTION,VIDEOPATH,VIDEOSAVE,\
				TIMESECOND,MITIMESECOND,LATITUDE,LONGITUDE,CAMERA_ID,CAMERA_CODE,LOCTION_ID,KAKOU_ID,PLACE) \
				values('%s',%d,%d,%d,\
				%d,'%s',%d,%d,%d,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s','%s',\
				%d,%d,%d,%d,%u,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s',%d,\
				'%s',%d,%d,%d,%d,'%s','%s','%s','%s')",\
            plate.chText,plate.uColor,plate.uCredit,nChannel,\
            plate.uRoadWayID,strTime.c_str(),plate.uMiTime,plate.uSmallPicSize,plate.uSmallPicWidth,\
            plate.uSmallPicHeight,plate.uPicSize,plate.uPicWidth,plate.uPicHeight,plate.uPosLeft,\
            plate.uPosTop,plate.uPosRight,plate.uPosBottom,strPicPath.c_str(),"",\
            nStatus,plate.uCarColor1,plate.uType,plate.uSpeed,uPicId,\
            plate.uWeight1,plate.uCarColor2,plate.uWeight2,plate.uPlateType,plate.uTypeDetail,\
            plate.uViolationType,plate.uCarBrand,plate.uDirection,strVideoPath.c_str(),nSaveState,\
			strTime2.c_str(),plate.uMiTime2,plate.uLatitude,plate.uLongitude, plate.uCameraId,plate.szCameraCode,plate.szLoctionID,plate.szKaKouItem,plate.chPlace);
	}
	else
	{
		#ifndef GLOBALCARCLASSIFY
		sprintf(buf,"Insert into NUMBER_PLATE_INFO(NUMBER,COLOR,CREDIT,CHANNEL,\
				ROAD,TIME,MITIME,SMALLPICSIZE,SMALLPICWIDTH,\
				SMALLPICHEIGHT,PICSIZE,PICWIDTH,PICHEIGHT,POSLEFT,\
				POSTOP,POSRIGHT,POSBOTTOM,PICPATH,SMALLPICPATH,\
				STATUS,CARCOLOR,TYPE,SPEED,PIC_ID,\
				CARCOLORWEIGHT,CARCOLORSECOND,CARCOLORWEIGHTSECOND,CARNUMBER_TYPE,TYPE_DETAIL,\
				PECCANCY_KIND,FACTORY,DIRECTION,VIDEOPATH,VIDEOSAVE,TIMESECOND,MITIMESECOND,REDLIGHT_TIME,REDLIGHT_BEGIN_TIME,REDLIGHT_BEGIN_MITIME,\
				REDLIGHT_END_TIME,REDLIGHT_END_MITIME,LIMIT_SPEED,OVER_SPEED) \
				values('%s',%d,%d,%d,\
				%d,'%s',%d,%d,%d,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s','%s',\
				%d,%d,%d,%d,%u,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s',%d,'%s',%d,%d,'%s',%d,\
				'%s',%d,%d,%d)",\
            plate.chText,plate.uColor,plate.uCredit,nChannel,\
            plate.uRoadWayID,strTime.c_str(),plate.uMiTime,plate.uSmallPicSize,plate.uSmallPicWidth,\
            plate.uSmallPicHeight,plate.uPicSize,plate.uPicWidth,plate.uPicHeight,plate.uPosLeft,\
            plate.uPosTop,plate.uPosRight,plate.uPosBottom,strPicPath.c_str(),"",\
            nStatus,plate.uCarColor1,plate.uType,plate.uSpeed,uPicId,\
            plate.uWeight1,plate.uCarColor2,plate.uWeight2,plate.uPlateType,plate.uTypeDetail,\
            plate.uViolationType,plate.uCarBrand,plate.uDirection,strVideoPath.c_str(),nSaveState,strTime2.c_str(),plate.uMiTime2,plate.uSignalTime,strBeginTime.c_str(),plate.uRedLightBeginMiTime,\
			strEndTime.c_str(),plate.uRedLightEndMiTime,plate.uLimitSpeed,plate.uOverSpeed);
		#else
		sprintf(buf,"Insert into NUMBER_PLATE_INFO(NUMBER,COLOR,CREDIT,CHANNEL,\
				ROAD,TIME,MITIME,SMALLPICSIZE,SMALLPICWIDTH,\
				SMALLPICHEIGHT,PICSIZE,PICWIDTH,PICHEIGHT,POSLEFT,\
				POSTOP,POSRIGHT,POSBOTTOM,PICPATH,SMALLPICPATH,\
				STATUS,CARCOLOR,TYPE,SPEED,PIC_ID,\
				CARCOLORWEIGHT,CARCOLORSECOND,CARCOLORWEIGHTSECOND,CARNUMBER_TYPE,TYPE_DETAIL,\
				PECCANCY_KIND,FACTORY,DIRECTION,VIDEOPATH,VIDEOSAVE,TIMESECOND,MITIMESECOND,REDLIGHT_TIME,REDLIGHT_BEGIN_TIME,REDLIGHT_BEGIN_MITIME,\
				REDLIGHT_END_TIME,REDLIGHT_END_MITIME,LIMIT_SPEED,OVER_SPEED,SUBTYPE_DETAIL,SUBFACTORY) \
				values('%s',%d,%d,%d,\
				%d,'%s',%d,%d,%d,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s','%s',\
				%d,%d,%d,%d,%u,\
				%d,%d,%d,%d,%d,\
				%d,%d,%d,'%s',%d,'%s',%d,%d,'%s',%d,\
				'%s',%d,%d,%d,%d,%d)",\
            plate.chText,plate.uColor,plate.uCredit,nChannel,\
            plate.uRoadWayID,strTime.c_str(),plate.uMiTime,plate.uSmallPicSize,plate.uSmallPicWidth,\
            plate.uSmallPicHeight,plate.uPicSize,plate.uPicWidth,plate.uPicHeight,plate.uPosLeft,\
            plate.uPosTop,plate.uPosRight,plate.uPosBottom,strPicPath.c_str(),"",\
            nStatus,plate.uCarColor1,plate.uType,plate.uSpeed,uPicId,\
            plate.uWeight1,plate.uCarColor2,plate.uWeight2,plate.uPlateType,plate.uTypeDetail,\
            plate.uViolationType,plate.uCarBrand,plate.uDirection,strVideoPath.c_str(),nSaveState,strTime2.c_str(),plate.uMiTime2,plate.uSignalTime,strBeginTime.c_str(),plate.uRedLightBeginMiTime,\
			strEndTime.c_str(),plate.uRedLightEndMiTime,plate.uLimitSpeed,plate.uOverSpeed,plate.uDetailCarType,plate.uDetailCarBrand);
		#endif
	}

    String sql(buf);
    //printf("insert plate info, %s\n", buf);

    if(execSQL(sql,&plate.uSeq)!=0)
    {
		/*system("myisamchk -o /var/lib/mysql/bocom_db/NUMBER_PLATE_INFO ");    //sql_myisamchk
		int fd = open("sql_myisamchk",O_RDWR);
		write(fd, "sql\n", 4);
		close(fd);*/
      
		return SRIP_ERROR_USER_FAILE;
    }
    plate.uChannelID = nChannel;
	

    ///////////////////////////////////////////////
	#ifdef NO_SEND_KAKOU_DATA
	   bSendToServer = false;
	#endif

    //	发送信息给中心端
    SRIP_DETECT_HEADER sDHeader;
    sDHeader.uChannelID = GetCameraID(nChannel);
	plate.uCameraId = sDHeader.uChannelID;
    sDHeader.uRealTime = 0x00000001;//此时均设为实时车牌

	if(bSendToServer)
    {
        sDHeader.uDetectType = MIMAX_PLATE_REP;
		//LogNormal("-Send-plate.uChannelID:%d \n", plate.uChannelID);
		g_skpChannelCenter.SendPlateInfo(sDHeader,plate,NULL);
    }

    return SRIP_OK;
}

//监测器之间通讯，删除临时表中的内容
int CSeekpaiDB::MvsCommunicationDelPlate(bool IsAll,string plateNum)
{
	char buf[BUFF_SIZE]= {0};
	if (IsAll)
	{
		//删除临时图片文件夹
		char *picDir = (char *)strstr(g_strMvsRecvPic.c_str(),"recvPic");
		string strPicDir = "";
		strPicDir.append(picDir,7);
		sprintf(buf,"rm -rf %s",strPicDir.c_str());
		system(buf);
		picDir = NULL;
		
		//删除临时表NUMBER_PLATE_INFO_RECV中的所有信息
		MysqlQuery q;
		memset(buf,0,BUFF_SIZE);
		sprintf(buf,"delete from NUMBER_PLATE_INFO_RECV");
		string strSqlDel(buf);
		if(execSQL(strSqlDel)!=0)
		{
			LogError("Delete NUMBER_PLATE_INFO_RECV All data error! \n");
			return SRIP_ERROR_USER_FAILE;
		}
		else
		{
			LogNormal("Delete NUMBER_PLATE_INFO_RECV All data success\n");
		}
		q.finalize();
	}
	else
	{
		memset(buf,0,BUFF_SIZE);
		sprintf(buf,"select * from NUMBER_PLATE_INFO_RECV where NUMBER = '%s'",plateNum.c_str());
		string strSql(buf);
		MysqlQuery q = g_skpDB.execQuery(strSql);
		if (!q.eof())
		{
			string strPicPath = q.getStringFileds("PICPATH");
			// 如果文件存在
			if(access(strPicPath.c_str(),F_OK) == 0)//删除车牌大图片文件
			{
				FILE* fp = NULL;
				fp = fopen(strPicPath.c_str(),"wb");
				if (fp == NULL)
				{
					return SRIP_ERROR_USER_FAILE;
				}
				fwrite("",0,1,fp);
				fclose(fp);
			}
			memset(buf,0,BUFF_SIZE);
			sprintf(buf,"Delete from NUMBER_PLATE_INFO_RECV where NUMBER = '%s'",plateNum.c_str());
			String sqlDel(buf);

			if(execSQL(sqlDel)!=0)
			{
				printf("Delete NUMBER_PLATE_INFO_RECV error! \n");
				return SRIP_ERROR_USER_FAILE;
			}

			printf("MvsCommunicationDelPlate ok!\n");
		}
		q.finalize();
	}
	return SRIP_OK;
}

//删除车牌信息
int CSeekpaiDB::DelPlate(unsigned int uBegin,unsigned int uEnd,int nKind)
{
    String strBegin,strEnd,strPic;
    char buf[BUFF_SIZE]= {0};

    //删除车牌大小图片
    if(nKind==0)
    {
        strBegin = GetTime(uBegin);
        strEnd = GetTime(uEnd);
        sprintf(buf,"select PICPATH from NUMBER_PLATE_INFO where TIME >='%s' and TIME <='%s'",strBegin.c_str(),strEnd.c_str());
    }
    else if(nKind==1)
    {
        sprintf(buf,"select PICPATH from NUMBER_PLATE_INFO where ID>=%u and ID<=%u",uBegin,uEnd);
    }
    String sqlSel(buf);

    MysqlQuery q = execQuery(sqlSel);

    while(!q.eof())
    {
        strPic = q.getStringFileds("PICPATH");
        // 如果文件存在
        if(access(strPic.c_str(),F_OK) == 0)//删除车牌大图片文件
        {
            FILE* fp = fopen(strPic.c_str(),"wb");
			if(fp)
			{
				fwrite("",0,1,fp);
				fclose(fp);
			}
        }
        q.nextRow();
    }
    q.finalize();


    //删除车牌数据库中的内容
    if(nKind==0)
    {
        //	printf("Delete uBegin=%s,uEnd=%s \n",strBegin.c_str(),strEnd.c_str());
        sprintf(buf,"Delete from NUMBER_PLATE_INFO where TIME >='%s' and TIME <='%s'",strBegin.c_str(),strEnd.c_str());
    }
    else if(nKind==1)
    {
        //	printf("Delete uBegin=%d,uEnd=%d \n",uBegin,uEnd);
        sprintf(buf,"Delete from NUMBER_PLATE_INFO where ID>=%u and ID<=%u",uBegin,uEnd);
    }

    String sqlDel(buf);

    if(execSQL(sqlDel)!=0)
    {
        printf("Delete NUMBER_PLATE_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    printf("DelPlate ok!\n");

    return SRIP_OK;

}

RECORD_PLATE CSeekpaiDB::GetPlateByVideoPath(string strVideoPath)
{
	RECORD_PLATE plate;
	char buf[BUFF_SIZE]= {0};

	sprintf(buf,"Select NUMBER_PLATE_INFO.*,CHAN_PLACE from NUMBER_PLATE_INFO,CHAN_INFO where VIDEOPATH ='%s' and CHANNEL = CHAN_ID order by TIME desc limit 1",strVideoPath.c_str());
	String sql(buf);
	MysqlQuery q = execQuery(sql);
	if(!q.eof())
	{
		plate.uSeq = q.getUnIntFileds("ID");
		String strPlace = q.getStringFileds("CHAN_PLACE");
        memcpy(plate.chPlace,strPlace.c_str(),strPlace.size());
		int nChannelId = q.getIntFileds("CHANNEL");
		plate.uChannelID = nChannelId;
		plate.uRoadWayID = q.getIntFileds("ROAD");
		String strTime = q.getStringFileds("TIME");
		plate.uTime = MakeTime(strTime);
		String text = q.getStringFileds("NUMBER");
		memcpy(plate.chText,text.c_str(),text.size());
		plate.uColor = q.getIntFileds("COLOR");
		plate.uType = q.getIntFileds("TYPE");
		plate.uCarColor1 = q.getIntFileds("CARCOLOR");
		plate.uDirection = q.getIntFileds("DIRECTION");
		plate.uSpeed = q.getIntFileds("SPEED");
		String strPath = q.getStringFileds("VIDEOPATH");
		memset(plate.chVideoPath,0,sizeof(plate.chVideoPath));
		memcpy(plate.chVideoPath,strPath.c_str(),strPath.size());
		plate.uLimitSpeed = q.getIntFileds("LIMIT_SPEED");
		plate.uTypeDetail = q.getIntFileds("TYPE_DETAIL");
		plate.uViolationType = q.getIntFileds("PECCANCY_KIND");
		plate.uPlateType = q.getIntFileds("CARNUMBER_TYPE");//车牌结构
		plate.uStatusType = q.getIntFileds("STATUS");
		plate.uCarBrand = q.getIntFileds("FACTORY");
		plate.uPicSize = q.getIntFileds("PICSIZE");
		plate.uPicWidth = q.getIntFileds("PICWIDTH");
		plate.uPicHeight = q.getIntFileds("PICHEIGHT");
		plate.uPosLeft = q.getIntFileds("POSLEFT");
		plate.uPosTop = q.getIntFileds("POSTOP");
		plate.uPosRight = q.getIntFileds("POSRIGHT");
		plate.uPosBottom = q.getIntFileds("POSBOTTOM");
		plate.uMiTime = q.getIntFileds("MITIME");
		plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
		plate.uSmallPicSize = q.getIntFileds("SMALLPICSIZE");
	}
	q.finalize();
	return plate;
}

/* 函数介绍：客户端获取图片信息
 * 输入参数：uID-序号，nKind-0事件图片；1车牌图片
 * 输出参数：无
 * 返回值：事件或车牌信息
 */
String CSeekpaiDB::GetPicInfoByID(unsigned int uID,int nKind)
{
    String response;
    char buf[BUFF_SIZE]= {0};
    String strPicPath;
    UINT32 uPicSize = 0;
    UINT32 uPicWidth = 0;
    UINT32 uPicHeight = 0;
    UINT32 uSmallPicSize = 0;

    if(nKind == 0)
    {
        RECORD_EVENT event;
        sprintf(buf,"Select POSX,POSY,PICPATH,PICWIDTH,PICHEIGHT,PICSIZE from TRAFFIC_EVENT_INFO where ID ='%u'",uID);
        String sql(buf);
        MysqlQuery q = execQuery(sql);

        if(!q.eof())
        {
            event.uPosX = q.getIntFileds("POSX");
            event.uPosY = q.getIntFileds("POSY");
            strPicPath = q.getStringFileds("PICPATH");
            event.uPicWidth = q.getIntFileds("PICWIDTH");
            event.uPicHeight = q.getIntFileds("PICHEIGHT");
            event.uPicSize = q.getIntFileds("PICSIZE");
            uPicSize = event.uPicSize;
            uPicWidth = event.uPicWidth;
            uPicHeight = event.uPicHeight;
        }
        q.finalize();

        response.append((char*)&event,sizeof(RECORD_EVENT));
    }
    else if( nKind == 1)
    {
        RECORD_PLATE plate;
		

		if(g_nGongJiaoMode == 1)
		{
			//sprintf(buf,"Select POSLEFT,POSTOP,POSRIGHT,POSBOTTOM,PICPATH,PICWIDTH,PICHEIGHT,PICSIZE,SMALLPICSIZE from NUMBER_PLATE_INFO where ID ='%u'",uID);
			sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
						COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
						VIDEOPATH,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
						CARNUMBER_TYPE,STATUS,FACTORY,PICSIZE,PICWIDTH, \
						PICHEIGHT,POSLEFT,POSTOP,POSRIGHT,POSBOTTOM, \
						MITIME,CARCOLORSECOND,SMALLPICSIZE,LATITUDE,LONGITUDE \
						from NUMBER_PLATE_INFO where ID ='%u'", uID);

			String sql(buf);
			//printf("-get pic-sql=%s \n", sql.c_str());

			MysqlQuery q = execQuery(sql);

			if(!q.eof())
			{
				plate.uSeq = q.getUnIntFileds("ID");

				int nChannelId = q.getIntFileds("CHANNEL");
				plate.uChannelID = nChannelId;
				//memcpy(plate.chReserved,&nChannelId,sizeof(int));//暂时借用保留字段

				plate.uRoadWayID = q.getIntFileds("ROAD");

				String strTime = q.getStringFileds("TIME");
				plate.uTime = MakeTime(strTime);

				String text = q.getStringFileds("NUMBER");
				memcpy(plate.chText,text.c_str(),text.size());

				plate.uColor = q.getIntFileds("COLOR");

				plate.uType = q.getIntFileds("TYPE");

				plate.uCarColor1 = q.getIntFileds("CARCOLOR");

				plate.uDirection = q.getIntFileds("DIRECTION");

				plate.uSpeed = q.getIntFileds("SPEED");

				String strPath = q.getStringFileds("VIDEOPATH");
				//		strPath.erase(0,g_strPic.size());
				memset(plate.chVideoPath,0,sizeof(plate.chVideoPath));
				memcpy(plate.chVideoPath,strPath.c_str(),strPath.size());

				strPicPath = q.getStringFileds("PICPATH");
				//		strPath.erase(0,g_strPic.size());
				memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

				plate.uTypeDetail = q.getIntFileds("TYPE_DETAIL");

				plate.uViolationType = q.getIntFileds("PECCANCY_KIND");
				//车牌结构
				plate.uPlateType = q.getIntFileds("CARNUMBER_TYPE");

				plate.uStatusType = q.getIntFileds("STATUS");
				plate.uCarBrand = q.getIntFileds("FACTORY");

				plate.uPicSize = q.getIntFileds("PICSIZE");
				plate.uPicWidth = q.getIntFileds("PICWIDTH");
				plate.uPicHeight = q.getIntFileds("PICHEIGHT");

				plate.uPosLeft = q.getIntFileds("POSLEFT");
				plate.uPosTop = q.getIntFileds("POSTOP");
				plate.uPosRight = q.getIntFileds("POSRIGHT");
				plate.uPosBottom = q.getIntFileds("POSBOTTOM");


				plate.uMiTime = q.getIntFileds("MITIME");
				plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
				plate.uSmallPicSize = q.getIntFileds("SMALLPICSIZE");			

				plate.uLatitude = q.getIntFileds("LATITUDE");
				plate.uLongitude = q.getIntFileds("LONGITUDE");

				uPicSize = plate.uPicSize;
				uPicWidth = plate.uPicWidth;
				uPicHeight = plate.uPicHeight;
				uSmallPicSize = plate.uSmallPicSize;


				//判断车牌位置是否需要扩充df
				GetCarPostion(plate);

				printf("RECORD_PLATE_CLIENT size:%d RECORD_PLATE:size:%d id:%d \n", sizeof(RECORD_PLATE_CLIENT), sizeof(RECORD_PLATE), plate.uSeq);
			}
			q.finalize();
		}
		else
		{
			sprintf(buf,"Select * from NUMBER_PLATE_INFO where ID ='%u'",uID);
			String sql(buf);
			MysqlQuery q = execQuery(sql);
			if(!q.eof())
			{
				plate.uSeq = q.getUnIntFileds("ID");

				int nChannelId = q.getIntFileds("CHANNEL");
				plate.uChannelID = nChannelId;
				plate.uRoadWayID = q.getIntFileds("ROAD");
				String strTime = q.getStringFileds("TIME");
				plate.uTime = MakeTime(strTime);
				String text = q.getStringFileds("NUMBER");
				memcpy(plate.chText,text.c_str(),text.size());
				plate.uColor = q.getIntFileds("COLOR");
				plate.uType = q.getIntFileds("TYPE");
				plate.uCarColor1 = q.getIntFileds("CARCOLOR");
				plate.uDirection = q.getIntFileds("DIRECTION");
				plate.uSpeed = q.getIntFileds("SPEED");
				String strPath = q.getStringFileds("VIDEOPATH");
				memset(plate.chVideoPath,0,sizeof(plate.chVideoPath));
				memcpy(plate.chVideoPath,strPath.c_str(),strPath.size());
				strPicPath = q.getStringFileds("PICPATH");
				memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());
				plate.uTypeDetail = q.getIntFileds("TYPE_DETAIL");
				plate.uViolationType = q.getIntFileds("PECCANCY_KIND");
				plate.uPlateType = q.getIntFileds("CARNUMBER_TYPE");//车牌结构
				plate.uStatusType = q.getIntFileds("STATUS");
				plate.uCarBrand = q.getIntFileds("FACTORY");
				plate.uPicSize = q.getIntFileds("PICSIZE");
				plate.uPicWidth = q.getIntFileds("PICWIDTH");
				plate.uPicHeight = q.getIntFileds("PICHEIGHT");
				plate.uPosLeft = q.getIntFileds("POSLEFT");
				plate.uPosTop = q.getIntFileds("POSTOP");
				plate.uPosRight = q.getIntFileds("POSRIGHT");
				plate.uPosBottom = q.getIntFileds("POSBOTTOM");
				plate.uMiTime = q.getIntFileds("MITIME");
				plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
				plate.uSmallPicSize = q.getIntFileds("SMALLPICSIZE");

				uPicSize = plate.uPicSize;
				uPicWidth = plate.uPicWidth;
				uPicHeight = plate.uPicHeight;
				uSmallPicSize = plate.uSmallPicSize;

				//判断车牌位置是否需要扩充df
				GetCarPostion(plate);
			}
			q.finalize();
		}        

		RECORD_PLATE_CLIENT plate_client;
		//memcpy(&plate_client,&plate,sizeof(RECORD_PLATE_CLIENT));

		plate_client.uSeq = plate.uSeq;						//序列号
		plate_client.uTime = plate.uTime;						//识别车牌时间(秒)
		plate_client.uMiTime = plate.uMiTime;					//识别车牌时间(毫秒)
		memcpy(plate_client.chText, plate.chText, strlen(plate.chText));					//车牌文本
		plate_client.uColor = plate.uColor;					//车牌类型（颜色）
		plate_client.uCredit = plate.uCredit;					//识别可靠度
		plate_client.uRoadWayID = plate.uRoadWayID;				//车道号
		plate_client.uType = plate.uType;						//车辆类型(低16位大、中、小，高16位卡车、巴士、轿车等）
		plate_client.uSmallPicSize = plate.uSmallPicSize;				//车牌小图大小
		plate_client.uSmallPicWidth = plate.uSmallPicWidth;			//车牌小图宽度
		plate_client.uSmallPicHeight = plate.uSmallPicHeight;			//车牌小图高度
		plate_client.uPicSize = plate.uPicSize;					//车牌全景图片大小
		plate_client.uPicWidth = plate.uPicWidth;					//车牌全景图片宽度
		plate_client.uPicHeight = plate.uPicHeight;				//车牌全景图片高度
		plate_client.uPosLeft = plate.uPosLeft;					//车牌在全景图片中的位置左
		plate_client.uPosTop = plate.uPosTop;					//车牌在全景图片中的位置上
		plate_client.uPosRight = plate.uPosRight;					//车牌在全景图片中的位置右
		plate_client.uPosBottom = plate.uPosBottom;				//车牌在全景图片中的位置下
		plate_client.uCarColor1 = plate.uCarColor1;				//车身颜色
		plate_client.uSpeed = plate.uSpeed;					//车速
		plate_client.uDirection = plate.uDirection;				//行驶方向
		plate_client.uCarBrand = plate.uCarBrand;				//产商标志
		memcpy(plate_client.chPlace, plate.chPlace, strlen(plate.chPlace));				//经过地点
		memcpy(plate_client.chVideoPath, plate.chVideoPath, strlen(plate.chVideoPath));	//录像路径
		memcpy(plate_client.chPicPath, plate.chPicPath, strlen(plate.chPicPath));		//大图片路径
		plate_client.uCarColor2 = plate.uCarColor2;                    //车身颜色2
		plate_client.uWeight1 = plate.uWeight1;                    //车身颜色权重1
		plate_client.uWeight2 = plate.uWeight2;                    //车身颜色权重2
		plate_client.uDetailCarType = plate.uDetailCarType;        //车型细分(由uTypeDetail决定更细的车型细分结果)
		plate_client.uPlateType = plate.uPlateType;            //车牌结构
		plate_client.uViolationType = plate.uViolationType;       //违章类型(闯红灯等)
		plate_client.uTypeDetail = plate.uTypeDetail;       //车型细分
		plate_client.uStatusType = plate.uStatusType;       //记录类型
		plate_client.Position = plate.Position;                   // 图象帧在文件中的开始位置（单位为字节）
		plate_client.uChannelID = plate.uChannelID;                  //通道编号
		plate_client.uLongitude = plate.uLongitude;//地点纬度(实际精度*1000000, 精确到小数点后六位,单位为度)
		plate_client.uLatitude = plate.uLatitude; //地点经度(实际精度*1000000, 精确到小数点后六位,单位为度)
		plate_client.uTime2 = plate.uTime2;						//第二车牌时间(秒)
		plate_client.uMiTime2 = plate.uMiTime2;					//第二车牌时间(毫秒)
		plate_client.uAlarmKind = plate.uAlarmKind;            //黑白名单报警1黑名单；2白名单
		plate_client.uDetailCarBrand = plate.uDetailCarBrand;			//厂商标志细分(由uCarBrand决定更细的厂商标志细分结果)

        response.append((char*)&plate_client,sizeof(RECORD_PLATE_CLIENT));
	}

    //////////////////////////////////////////

    if(strPicPath.size()>0)
    {
        string strPic1 = GetImageByPath(strPicPath);

        printf("strPic1.size()=%d,uPicSize=%d,uSmallPicSize=%d\n",strPic1.size(),uPicSize,uSmallPicSize);

        if(strPic1.size() == uPicSize+uSmallPicSize)
        {
			if(uSmallPicSize > 0)//1张大图1张小图的情况
			{
				CxImage image1;
				if(uPicSize > 0)
				image1.Decode((unsigned char*)strPic1.c_str(),uPicSize,3);//先解码

				CxImage image2;
				image2.Decode((unsigned char*)(strPic1.c_str()+uPicSize),uSmallPicSize,3);//先解码

				if(image2.IsValid() && image1.IsValid())
				{
					uPicWidth = image1.GetWidth();
					uPicHeight = image1.GetHeight();

					UINT32 uSmallPicWidth = image2.GetWidth();
					UINT32 uSmallPicHeight = image2.GetHeight();

					 printf("uPicWidth=%d,uPicHeight=%d,uSmallPicWidth=%d,uSmallPicHeight=%d\n",uPicWidth,uPicHeight,uSmallPicWidth,uSmallPicHeight);
					
					UINT32 uSmallWidth = uSmallPicWidth;

					if(uSmallPicWidth %4 > 0)
					{
						uSmallWidth = uSmallPicWidth+ 4- uSmallPicWidth %4;
					}

					if(uPicWidth > 0 && uPicHeight > 0 && uSmallPicWidth > 0 && uSmallPicHeight > 0)
					{
						UINT32 uImageSize = (uPicWidth+uSmallWidth)*uPicHeight*3;
						unsigned char* pImageData = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
						unsigned char* pJpgImage = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));

						for(int i = 0;i < uPicHeight;i++)
						{
							if(image1.IsValid())
							memcpy(pImageData+i*(uPicWidth+uSmallWidth)*3,image1.GetBits(i),uPicWidth*3);

							if(image2.IsValid())
							{
								if( i < uSmallPicHeight)
								{
									memcpy(pImageData+i*(uPicWidth+uSmallWidth)*3+uPicWidth*3,image2.GetBits(i),uSmallWidth*3);
								}
							}
						}

						int srcstep = 0;
						CxImage image;
						image.IppEncode(pImageData,uPicWidth+uSmallWidth,uPicHeight,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);

						if(srcstep > 0)
						response.append((char*)pJpgImage,srcstep);

						if(pImageData)
						{
							free(pImageData);
							pImageData = NULL;
						}

						if(pJpgImage)
						{
							free(pJpgImage);
							pJpgImage = NULL;
						}
					}
					else
					{
						response.append((char*)strPic1.c_str(),strPic1.size());
					}
				}
				else
				{
					response.append((char*)strPic1.c_str(),strPic1.size());
				}
			}
			else
			{
				response.append((char*)strPic1.c_str(),strPic1.size());
			}
        }
        else if(strPic1.size() > uPicSize)
        {
            //对于分开存储的图片需要重新组装成一张图片
            CxImage image1;
            if(uPicSize > 0)
            image1.Decode((unsigned char*)strPic1.c_str(),uPicSize,3);//先解码
            printf("s1=%d,s2=%d,w=%d,h=%d,s=%d\n",uPicSize,strPic1.size(),image1.GetWidth(),image1.GetHeight(),image1.GetSize());

            CxImage image2;
            if( strPic1.size()-uPicSize > 0 )
            image2.Decode((unsigned char*)(strPic1.c_str()+uPicSize),strPic1.size()-uPicSize,3);//先解码
            printf("s1=%d,s2=%d,w=%d,h=%d,s=%d\n",uPicSize,strPic1.size(),image2.GetWidth(),image2.GetHeight(),image2.GetSize());

            if(image2.IsValid() && image1.IsValid())
            {
				uPicWidth = image1.GetWidth();
				uPicHeight = image1.GetHeight();

                if((image2.GetWidth() == image1.GetWidth()) && (image1.GetWidth() == uPicWidth))
                {
                    UINT32 uImageSize = uPicWidth*uPicHeight*3;
                    unsigned char* pImageData = new unsigned char[uImageSize*2];
                    unsigned char* pJpgImage = new unsigned char[uImageSize];

                    for(int i = 0;i < uPicHeight;i++)
                    {
                        if(image1.IsValid())
                        memcpy(pImageData+i*uPicWidth*6,image1.GetBits(i),uPicWidth*3);

                        if(image2.IsValid())
                        memcpy(pImageData+i*uPicWidth*6+uPicWidth*3,image2.GetBits(i),uPicWidth*3);
                    }

                    int srcstep = 0;
                    CxImage image;
                    image.IppEncode(pImageData,uPicWidth*2,uPicHeight,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);

                    if(srcstep > 0)
                    response.append((char*)pJpgImage,srcstep);

                    if(pImageData)
                    delete []pImageData;
                    pImageData = NULL;

                    if(pJpgImage)
                    delete []pJpgImage;
                }
                else
                {
                    response.append((char*)strPic1.c_str(),strPic1.size());
                }
            }
            else
            {
                response.append((char*)strPic1.c_str(),strPic1.size());
            }
        }
		else
		{
			strPicPath.erase(strPicPath.size()-6,6);
			strPicPath += "tx.jpg";
			string strPic2 = GetImageByPath(strPicPath);

			if(strPic2.size() == uSmallPicSize &&(uSmallPicSize > 0))
			{
				CxImage image1;
				if(uPicSize > 0)
				image1.Decode((unsigned char*)strPic1.c_str(),uPicSize,3);//先解码

				CxImage image2;
				image2.Decode((unsigned char*)strPic2.c_str(),uSmallPicSize,3);//先解码

				if(image2.IsValid() && image1.IsValid())
				{
					uPicWidth = image1.GetWidth();
					uPicHeight = image1.GetHeight();

					UINT32 uSmallPicWidth = image2.GetWidth();
					UINT32 uSmallPicHeight = image2.GetHeight();

					 printf("uPicWidth=%d,uPicHeight=%d,uSmallPicWidth=%d,uSmallPicHeight=%d\n",uPicWidth,uPicHeight,uSmallPicWidth,uSmallPicHeight);
					
					UINT32 uSmallWidth = uSmallPicWidth;

					if(uSmallPicWidth %4 > 0)
					{
						uSmallWidth = uSmallPicWidth+ 4- uSmallPicWidth %4;
					}

					if(uPicWidth > 0 && uPicHeight > 0 && uSmallPicWidth > 0 && uSmallPicHeight > 0)
					{
						UINT32 uImageSize = (uPicWidth+uSmallWidth)*uPicHeight*3;
						unsigned char* pImageData = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
						unsigned char* pJpgImage = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));

						for(int i = 0;i < uPicHeight;i++)
						{
							if(image1.IsValid())
							memcpy(pImageData+i*(uPicWidth+uSmallWidth)*3,image1.GetBits(i),uPicWidth*3);

							if(image2.IsValid())
							{
								if( i < uSmallPicHeight)
								{
									memcpy(pImageData+i*(uPicWidth+uSmallWidth)*3+uPicWidth*3,image2.GetBits(i),uSmallWidth*3);
								}
							}
						}

						int srcstep = 0;
						CxImage image;
						image.IppEncode(pImageData,uPicWidth+uSmallWidth,uPicHeight,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);

						if(srcstep > 0)
						response.append((char*)pJpgImage,srcstep);

						if(pImageData)
						{
							free(pImageData);
							pImageData = NULL;
						}

						if(pJpgImage)
						{
							free(pJpgImage);
							pJpgImage = NULL;
						}
					}
					else
					{
						response.append((char*)strPic1.c_str(),strPic1.size());
					}
				}
				else
				{
					response.append((char*)strPic1.c_str(),strPic1.size());
				}
			}
			else
			{
				response.append((char*)strPic1.c_str(),strPic1.size());
			}
		}
    }
    //////////////////////////////////////////

    return response;
}


/* 函数介绍：中心服务器获取车牌信息
 * 输入参数：uBegin-开始时间或序号，uEnd-结束时间或序号，nKind-按时间、序号或者是推送非实时记录，nType-是否需要推送快照图像
 * 输出参数：无
 * 返回值：无
 */
void CSeekpaiDB::GetPlate(unsigned int uBegin,unsigned int uEnd)
{
	String strTimeBeg = GetTime(uBegin);
    String strTimeEnd = GetTime(uEnd);

	LogNormal("%s-%s",strTimeBeg.c_str(),strTimeEnd.c_str());

	char buf[BUFF_SIZE]= {0};

	sprintf(buf,"update NUMBER_PLATE_INFO set STATUS = 0 where TIME >= '%s' and TIME <= '%s'",strTimeBeg.c_str(),strTimeEnd.c_str());

	String sqlPlate(buf);

    if(execSQL(sqlPlate)!=0)
    {
        printf("update NUMBER_PLATE_INFO error! \n");
    }
}


//获取车牌识别结果
String CSeekpaiDB::GetCarNum(SEARCH_ITEM& search_item,int nPageSize/*=0*/)
{
    String strTimeBeg = GetTime(search_item.uBeginTime);
    String strTimeEnd = GetTime(search_item.uEndTime);

    int nPage = search_item.uPage;
    int nSortId = search_item.uSortId;
    int nSortKind = search_item.uSortKind;
    int nChannel = search_item.uChannelId;
	int nRoadIndex = search_item.uRoadId;
    String strCarNum(search_item.chText);
    //查询附加条件
    unsigned int uConditionType = search_item.uConditionType;

    char bufChannel[80] = {0};
    char bufCarNum[80] = {0};
    char bufObject[80] = {0};
    char bufViolation[80] = {0};

    String sChannel("");
    String sCarNum("");
    String sObject("");
    String sViolation("");

    if(nChannel > 0)
    {
        sprintf(bufChannel, " and CHANNEL = %d ", nChannel);
        sChannel = bufChannel;
    }

    if(strCarNum.size() > 0)
    {
        sprintf(bufCarNum, " and NUMBER like '%%%s%%' ", strCarNum.c_str());
        sCarNum = bufCarNum;
    }

    if( (uConditionType & 0x0001) == 0x0001 )
    {
        sprintf(bufObject, " and NUMBER not like '***%%' ");
        sObject = bufObject;
    }
    if( (uConditionType & 0x0002) == 0x0002 )
    {
        sprintf(bufViolation, " and PECCANCY_KIND = %d ",(int)DETECT_RESULT_RED_LIGHT_VIOLATION);
        sViolation = bufViolation;
    }

    /*
    	char bufTemp[BUFF_SIZE];
        sprintf(bufTemp, "%s%s%s%s", sChannel.c_str(), sCarNum.c_str(), sObject.c_str(), sViolation.c_str());

        String strTemp(bufTemp);
    */
    printf("%s, %s, %s, %s\n", sChannel.c_str(), sCarNum.c_str(), sObject.c_str(), sViolation.c_str());
    String strTemp("");
    if (!sChannel.empty())
    {
        strTemp += sChannel;
    }
    if (!sCarNum.empty())
    {
        strTemp += sCarNum;
    }
    if (!sObject.empty())
    {
        strTemp += sObject;
    }
    if (!sViolation.empty())
    {
        strTemp += sViolation;
    }

    printf("strCarNum=%s,strCarNum.size()=%d\n",strCarNum.c_str(),strCarNum.size());

    String response("");

    if(nPage<1||nPageSize<0)
    {
#ifdef _DEBUG
        printf("nPage or nPageSize is Error! \n");
#endif
        return response;
    }

    char buf[BUFF_SIZE]= {0};


    char SortKind[2][5]= {"ASC","DESC"};
    char SortId[16][15]= {"ID","TIME","CHAN_PLACE","CHANNEL","ROAD",
                          "DIRECTION","NUMBER","COLOR","CARNUMBER_TYPE","SPEED",
                          "CARCOLOR","TYPE","TYPE_DETAIL","PECCANCY_KIND","STATUS",
                          "FACTORY"
                         };

if(g_nGongJiaoMode == 1)
{
	if(nRoadIndex == 0)
	{
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
				COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
				VIDEOPATH,PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
				CARNUMBER_TYPE,STATUS,FACTORY,PICSIZE,PICWIDTH, \
				PICHEIGHT,POSLEFT,POSTOP,POSRIGHT,POSBOTTOM,MITIME,CARCOLORSECOND,\
				LATITUDE,LONGITUDE \
				from NUMBER_PLATE_INFO,CHAN_INFO \
				where TIME >= '%s' and TIME <= '%s' \
				%s \
				and CHANNEL = CHAN_ID \
				ORDER BY %s %s limit %d,%d",\
				strTimeBeg.c_str(),strTimeEnd.c_str(),\
				strTemp.c_str(),\
				SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
	}



	else
	{
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
                    COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
                    VIDEOPATH,PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
                    CARNUMBER_TYPE,STATUS,FACTORY,PICSIZE,PICWIDTH, \
                    PICHEIGHT,POSLEFT,POSTOP,POSRIGHT,POSBOTTOM,MITIME,CARCOLORSECOND, \
					LATITUDE,LONGITUDE \
                    from NUMBER_PLATE_INFO,CHAN_INFO \
                    where TIME >= '%s' and TIME <= '%s' and ROAD = %d\
                    %s \
                    and CHANNEL = CHAN_ID \
                    ORDER BY %s %s limit %d,%d",\
            strTimeBeg.c_str(),strTimeEnd.c_str(),nRoadIndex,\
            strTemp.c_str(),\
            SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
	}
}
else
{
	if(nRoadIndex == 0)
	{
		#ifndef GLOBALCARCLASSIFY
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
				COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
				VIDEOPATH,CHAN_PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
				CARNUMBER_TYPE,STATUS,FACTORY,PICSIZE,PICWIDTH, \
				PICHEIGHT,POSLEFT,POSTOP,POSRIGHT,POSBOTTOM,MITIME,CARCOLORSECOND \
				from NUMBER_PLATE_INFO,CHAN_INFO \
				where TIME >= '%s' and TIME <= '%s' \
				%s \
				and CHANNEL = CHAN_ID \
				ORDER BY %s %s limit %d,%d",\
				strTimeBeg.c_str(),strTimeEnd.c_str(),\
				strTemp.c_str(),\
				SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
		#else
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
				COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
				VIDEOPATH,CHAN_PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
				CARNUMBER_TYPE,STATUS,FACTORY,PICSIZE,PICWIDTH, \
				PICHEIGHT,POSLEFT,POSTOP,POSRIGHT,POSBOTTOM,MITIME,CARCOLORSECOND,SUBTYPE_DETAIL,SUBFACTORY \
				from NUMBER_PLATE_INFO,CHAN_INFO \
				where TIME >= '%s' and TIME <= '%s' \
				%s \
				and CHANNEL = CHAN_ID \
				ORDER BY %s %s limit %d,%d",\
				strTimeBeg.c_str(),strTimeEnd.c_str(),\
				strTemp.c_str(),\
				SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
		#endif
	}



	else
	{
		#ifndef GLOBALCARCLASSIFY
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
					COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
					VIDEOPATH,CHAN_PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
					CARNUMBER_TYPE,STATUS,FACTORY,PICSIZE,PICWIDTH, \
                    PICHEIGHT,POSLEFT,POSTOP,POSRIGHT,POSBOTTOM,MITIME,CARCOLORSECOND \
                    from NUMBER_PLATE_INFO,CHAN_INFO \
                    where TIME >= '%s' and TIME <= '%s' and ROAD = %d\
                    %s \
                    and CHANNEL = CHAN_ID \
                    ORDER BY %s %s limit %d,%d",\
            strTimeBeg.c_str(),strTimeEnd.c_str(),nRoadIndex,\
            strTemp.c_str(),\
            SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
		#else
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
					COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
					VIDEOPATH,CHAN_PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
					CARNUMBER_TYPE,STATUS,FACTORY,PICSIZE,PICWIDTH, \
                    PICHEIGHT,POSLEFT,POSTOP,POSRIGHT,POSBOTTOM,MITIME,CARCOLORSECOND,SUBTYPE_DETAIL,SUBFACTORY \
                    from NUMBER_PLATE_INFO,CHAN_INFO \
                    where TIME >= '%s' and TIME <= '%s' and ROAD = %d\
                    %s \
                    and CHANNEL = CHAN_ID \
                    ORDER BY %s %s limit %d,%d",\
            strTimeBeg.c_str(),strTimeEnd.c_str(),nRoadIndex,\
            strTemp.c_str(),\
            SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
		#endif
	}
}

    //获得查询数量
    int nCount=0;
    char bufCount[BUFF_SIZE];
    memset(bufCount, 0, BUFF_SIZE);


    if(nRoadIndex == 0)
	{
		sprintf(bufCount, "Select count(*) from NUMBER_PLATE_INFO,CHAN_INFO \
            Where TIME >= '%s' and TIME <= '%s' and CHANNEL = CHAN_ID %s ", \
            strTimeBeg.c_str(),strTimeEnd.c_str(), strTemp.c_str() );
	}
	else
	{
		sprintf(bufCount, "Select count(*) from NUMBER_PLATE_INFO,CHAN_INFO \
					  Where TIME >= '%s' and TIME <= '%s' and ROAD = %d and CHANNEL = CHAN_ID %s ", \
					  strTimeBeg.c_str(),strTimeEnd.c_str(), nRoadIndex, strTemp.c_str() );
	}

    String sqlCount(bufCount);
    String sql(bufCount);
    nCount = getIntFiled(sql);

#ifdef _DEBUG
    printf("\n=================sqlCount= %s!!!\n", sqlCount.c_str());
#endif

    if(nPage==1)
    {
        //nCount=GetCarNumCount(nChannel,strTimeBeg,strTimeEnd,strCarNum);
        int nNum=nCount % nPageSize;
        if(nNum>0)
            nCount=nCount/nPageSize+1;
        else
            nCount=nCount/nPageSize;
    }

    String sql2(buf);
    printf("\n=================sql2= %s!!!\n", sql2.c_str());
#ifdef _DEBUG
    printf("\n=================sql2= %s!!!\n", sql2.c_str());
#endif

    MysqlQuery q = execQuery(sql2);
    printf("nCount=%d, numFileds=%d\n",nCount,q.numFileds());

    response.append((char*)&nCount,sizeof(nCount));

    while(!q.eof())
    {
        RECORD_PLATE plate;
        plate.uSeq=q.getUnIntFileds("ID");

        int nChannelId = q.getIntFileds("CHANNEL");
		plate.uChannelID = nChannelId;
        //memcpy(plate.chReserved,&nChannelId,sizeof(int));//暂时借用保留字段

        plate.uRoadWayID = q.getIntFileds("ROAD");

        String strTime = q.getStringFileds("TIME");
        plate.uTime = MakeTime(strTime);

        String text = q.getStringFileds("NUMBER");
        memcpy(plate.chText,text.c_str(),text.size());

        plate.uColor = q.getIntFileds("COLOR");

        plate.uType = q.getIntFileds("TYPE");

        plate.uCarColor1 = q.getIntFileds("CARCOLOR");

        plate.uDirection = q.getIntFileds("DIRECTION");

        plate.uSpeed = q.getIntFileds("SPEED");

       if (1 != g_nGongJiaoMode)
       {
		   String strPlace = q.getStringFileds("CHAN_PLACE");
		   memcpy(plate.chPlace,strPlace.c_str(),strPlace.size());
       }

		String strPath = q.getStringFileds("VIDEOPATH");
		//		strPath.erase(0,g_strPic.size());
		memset(plate.chVideoPath,0,sizeof(plate.chVideoPath));
		memcpy(plate.chVideoPath,strPath.c_str(),strPath.size());

        strPath = q.getStringFileds("PICPATH");
//		strPath.erase(0,g_strPic.size());
        memcpy(plate.chPicPath,strPath.c_str(),strPath.size());

        plate.uTypeDetail = q.getIntFileds("TYPE_DETAIL");

        plate.uViolationType = q.getIntFileds("PECCANCY_KIND");
        //车牌结构
        plate.uPlateType = q.getIntFileds("CARNUMBER_TYPE");

        plate.uStatusType = q.getIntFileds("STATUS");
        plate.uCarBrand = q.getIntFileds("FACTORY");

        plate.uPicSize = q.getIntFileds("PICSIZE");
        plate.uPicWidth = q.getIntFileds("PICWIDTH");
        plate.uPicHeight = q.getIntFileds("PICHEIGHT");
        plate.uPosLeft = q.getIntFileds("POSLEFT");
        plate.uPosTop = q.getIntFileds("POSTOP");
        plate.uPosRight = q.getIntFileds("POSRIGHT");
        plate.uPosBottom = q.getIntFileds("POSBOTTOM");
        plate.uMiTime = q.getIntFileds("MITIME");
        plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
		if(g_nGongJiaoMode == 1)
		{
			plate.uLatitude = q.getIntFileds("LATITUDE");
			plate.uLongitude = q.getIntFileds("LONGITUDE");
			String strPlace = q.getStringFileds("PLACE");
			memcpy(plate.chPlace,strPlace.c_str(),strPlace.size());
		}
		#ifdef GLOBALCARCLASSIFY
		plate.uDetailCarType = q.getIntFileds("SUBTYPE_DETAIL");
		plate.uDetailCarBrand = q.getIntFileds("SUBFACTORY");
		#endif
		

        //判断车牌位置是否需要扩充
        GetCarPostion(plate);
		
		RECORD_PLATE_CLIENT plate_client;
		memcpy(&plate_client,&plate,sizeof(RECORD_PLATE_CLIENT));
		plate_client.uDetailCarType =  plate.uDetailCarType;
		plate_client.uDetailCarBrand =  plate.uDetailCarBrand;
        response.append((char*)&plate_client,sizeof(plate_client));

        q.nextRow();
    }

    q.finalize();

    return response;
}

//获得车牌数量
int CSeekpaiDB::GetCarNumCount(int nChannel,String strTimeBeg,String strTimeEnd,String strCarNum)
{
    int nRet=0;

    //包含本天
    strTimeEnd+=" 23:59:59";

    char buf[BUFF_SIZE]= {0};

    if(strCarNum.size()<=0)
    {
        if(nChannel==0)
            sprintf(buf,"Select count(*) from NUMBER_PLATE_INFO where TIME >='%s' and TIME <='%s'",strTimeBeg.c_str(),strTimeEnd.c_str());
        else
            sprintf(buf,"Select count(*) from NUMBER_PLATE_INFO where TIME >='%s' and TIME <='%s' and CHANNEL = %d",strTimeBeg.c_str(),strTimeEnd.c_str(),nChannel);
    }
    else
    {
        if(nChannel==0)
            sprintf(buf,"Select count(*)  from NUMBER_PLATE_INFO where TIME >='%s' and TIME <='%s' and NUMBER like '%%%s%%'",strTimeBeg.c_str(),strTimeEnd.c_str(),strCarNum.c_str());
        else
            sprintf(buf,"Select count(*)  from NUMBER_PLATE_INFO where TIME >='%s' and TIME <='%s' and CHANNEL = %d and NUMBER like '%%%s%%'",strTimeBeg.c_str(),strTimeEnd.c_str(),nChannel,strCarNum.c_str());
    }

    String sql(buf);

    //
    nRet=getIntFiled(sql);

    return nRet;
}


/* 函数介绍：保存统计信息
 * 输入参数：nChannel-通道号，statistic-统计记录，nType-车道类型
 * 输出参数：无
 * 返回值：SRIP_OK-正常插入；SRIP_ERROR_USER_FAILE-插入失败
 */
int CSeekpaiDB::SaveStatisticInfo(int nChannel,RECORD_STATISTIC& statistic,int nType)
{
    String strTime = GetTime(statistic.uTime);

    //bool status = g_skpChannelCenter.GetRealTimeStat();//判断是否实时统计

    char buf[BUFF_SIZE]= {0};

    //加锁
//	pthread_mutex_lock(&m_statistic_mutex);
    statistic.uSeq= m_nStatisticSeq;
    //解锁
//	pthread_mutex_unlock(&m_statistic_mutex);

    for(int i=0; i<MAX_ROADWAY; i++)
    {
        unsigned int uRoadIndex = statistic.uRoadType[i];
        uRoadIndex   =    uRoadIndex>>16;

        if(statistic.uFlux[i]!=0xffffffff)
        {
            //同时插入6条语句
            sprintf(buf,"Insert into TRAFFIC_STATISTIC_INFO(ID,TIME,CHANNEL,STATTIMELEN,ROAD,KIND,VALUE,STATUS,TYPE) \
                            values(%u,'%s',%d,%d,%d,%d,%.2f,%d,%d),\
                            (%u,'%s',%d,%d,%d,%d,%.2f,%d,%d),\
                            (%u,'%s',%d,%d,%d,%d,%.2f,%d,%d),\
                            (%u,'%s',%d,%d,%d,%d,%.2f,%d,%d),\
                            (%u,'%s',%d,%d,%d,%d,%.2f,%d,%d),\
                            (%u,'%s',%d,%d,%d,%d,%.2f,%d,%d)",\
                    statistic.uSeq,strTime.c_str(),nChannel,statistic.uStatTimeLen,uRoadIndex,DETECT_RESULT_STAT_FLUX,(double)statistic.uFlux[i],0,nType,\
                    statistic.uSeq,strTime.c_str(),nChannel,statistic.uStatTimeLen,uRoadIndex,DETECT_RESULT_STAT_SPEED_AVG,(double)statistic.uSpeed[i],0,nType,\
                    statistic.uSeq,strTime.c_str(),nChannel,statistic.uStatTimeLen,uRoadIndex,DETECT_RESULT_STAT_QUEUE,(double)statistic.uQueue[i],0,nType,\
                    statistic.uSeq,strTime.c_str(),nChannel,statistic.uStatTimeLen,uRoadIndex,DETECT_RESULT_STAT_ZYL,(double)statistic.uOccupancy[i],0,nType,\
                    statistic.uSeq,strTime.c_str(),nChannel,statistic.uStatTimeLen,uRoadIndex,DETECT_RESULT_STAT_CTJJ,(double)statistic.uSpace[i],0,nType,\
                    statistic.uSeq,strTime.c_str(),nChannel,statistic.uStatTimeLen,uRoadIndex,DETECT_RESULT_STAT_CAR_TYPE,(double)statistic.uFluxCom[i],0,nType);

            String sql(buf);
			
            if(execSQL(sql)!=0)
            {
				LogNormal("SaveStatisticInfo　函数中　保存到数据库失败！");
                printf("Add statistic error! sql=%s\n",sql.c_str());
                return SRIP_ERROR_USER_FAILE;
            }
        }
    }

    if(m_nStatisticSeq>=MAX_SEQ)
    {
        m_nStatisticSeq = 0;
    }
    m_nStatisticSeq++;

    //发送实时统计
    //if(status)
    {
        printf("send statistic\n");
        std::string strStatistic;
        SRIP_DETECT_HEADER sDHeader;
        sDHeader.uChannelID = GetCameraID(nChannel);
        sDHeader.uDetectType = MIMAX_STATISTIC_REP;
        sDHeader.uRealTime = 0x00000001;//此时均设为实时统计
        strStatistic.append((char*)&sDHeader,sizeof(SRIP_DETECT_HEADER));
        strStatistic.append((char*)&statistic,sizeof(RECORD_STATISTIC));

        if(g_nServerType == 1)
        {
           g_AMSCommunication.AddResult(strStatistic);
        }
        else if(g_nServerType == 3)
        {
            g_TravelServer.AddResult(strStatistic);
        }
        else if(g_nServerType == 5)
        {
            g_LSServer.AddResult(strStatistic);
	#ifdef LS_QINGTIAN_IVAP
			g_BocomServerManage.AddResult(strStatistic);
	#endif
        }
        else if(g_nServerType == 0 || g_nServerType == 4||g_nServerType == 6 || g_nServerType == 26)
        {
			g_BocomServerManage.AddResult(strStatistic);
        }
		else if(g_nServerType == 7) //7是天津电警 
		{
			//g_RoadImcData.AddStatisticData(strStatistic,nType);
			g_RoadImcData.AddStatisticDataTj(strStatistic,nType);
		}
		else if(g_nServerType == 10) //7是天津电警 10是鞍山交警,都写在一个类体内
		{
			g_TJServer.AddResult(strStatistic);
		}
		else if(g_nServerType == 8)//add by Gaoxiang
		{
			g_TelComServer.AddResult(strStatistic);
		}
		else if (12 == g_nServerType)
		{
			g_BaoKangServer.AddResult(strStatistic);
		}
    }

    return SRIP_OK;
}

//删除统计
int CSeekpaiDB::DelStatisticInfo(unsigned int uBegin,unsigned int uEnd,int nKind)
{
    char buf[BUFF_SIZE]= {0};
    if(nKind==0)
    {
        String strBegin = GetTime(uBegin);
        String strEnd = GetTime(uEnd);
        printf("Delete uBegin=%s,uEnd=%s \n",strBegin.c_str(),strEnd.c_str());
        sprintf(buf,"Delete from TRAFFIC_STATISTIC_INFO where TIME >='%s' and TIME <='%s'",strBegin.c_str(),strEnd.c_str());
    }
    else if(nKind==1)
    {
        printf("Delete uBegin=%u,uEnd=%u \n",uBegin,uEnd);
        sprintf(buf,"Delete from TRAFFIC_STATISTIC_INFO where ID>=%u and ID<=%u",uBegin,uEnd);
    }

    String sql(buf);

    if(execSQL(sql)!=0)
    {
        printf("Delete TRAFFIC_STATISTIC_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    printf("DelStatisticInfo ok\n");
    return SRIP_OK;
}

/* 函数介绍：中心服务器读取统计信息
 * 输入参数：uBegin-开始时间或序号，uEnd-结束时间或序号，nKind-按时间、序号或者是推送非实时记录
 * 输出参数：无
 * 返回值：无
 */
void CSeekpaiDB::GetStatisticInfo(unsigned int uBegin,unsigned int uEnd,int nKind,int nSocket)
{
}

//初始化通道采集列表--2008-5-7 徐永丰
void CSeekpaiDB::InitChannelList()
{
    char buf[BUFF_SIZE]= {0};

	sprintf(buf,"SELECT CHAN_ID,CHAN_PLACE, CHAN_KIND,CHAN_FORMAT,CHAN_EVENTCAPTURETIME,\
				VIDEO_BEGINTIME,VIDEO_ENDTIME,CHAN_BRIGHTNESS,CHAN_CONTRAST,CHAN_SATURATION,\
				CHAN_HUE,CHAN_CAP_TYPE,CHAN_CAP_BEGINTIME,CHAN_CAP_ENDTIME,CHAN_YUV_HOST,\
				CHAN_YUV_PORT,CHAN_DETECT_KIND,CHAN_DETECT_TIME,CHAN_EVENT_CAPTURE,YUV_FORMAT,\
				CHAN_RMSHADE,CHAN_RMTINGLE,CHAN_SENSITIVE,CHAN_RUN,CAMERA_ID,\
				CHAN_SRC_FILE,CHAN_SHOWTIME,CAMERA_TYPE,CHAN_DIRECTION,PANNEL_ID,\
				SYN_HOST, SYN_PORT,CHAN_PRESET,MONITOR_ID,VIDEO_ID,\
				WORKMODE,CAMERAIP,DEVICE_ID \
				from CHAN_INFO ORDER BY CHAN_ID");
    //SQL 语句
    String sql(buf);

//	printf("======初始化通道列表!sql=%s\r\n",sql.c_str());
    //执行查询
    MysqlQuery q = execQuery(sql);

//	printf("======q.numFileds()=%d\r\n",q.numFileds());
    //循环读数据
    while(!q.eof())
    {
        //通道结构
        SRIP_CHANNEL sChannel;
        sChannel.nChanWayType = g_nRoadType;
        //通道ID
        sChannel.uId = q.getIntFileds("CHAN_ID");

        //通道地点
        String strLoc = q.getStringFileds("CHAN_PLACE");
        memcpy(sChannel.chPlace,strLoc.c_str(),strLoc.size());
        //通道类型
        sChannel.eType = (CHANNEL_TYPE)q.getIntFileds("CHAN_KIND");
        //视频格式
        sChannel.eVideoFmt = (VEDIO_FORMAT)q.getIntFileds("CHAN_FORMAT");
        //事件录像时间
        sChannel.uEventCaptureTime = q.getIntFileds("CHAN_EVENTCAPTURETIME");

        String strBeginTime = q.getStringFileds("VIDEO_BEGINTIME");
        sChannel.uVideoBeginTime = MakeTime(strBeginTime);

        String strEndTime = q.getStringFileds("VIDEO_ENDTIME");
        sChannel.uVideoEndTime = MakeTime(strEndTime);

        //通道ID
        //sChannel.sAttr.uId = sChannel.uId;
        //视频参数亮度
        //sChannel.sAttr.uBrightness = q.getIntFileds(7);
        //视频参数对比度
        //sChannel.sAttr.uContrast = q.getIntFileds(8);
        //视频参数饱和度
        //sChannel.sAttr.uSaturation = q.getIntFileds(9);
        //视频参数色调
        //sChannel.sAttr.uHue = q.getIntFileds(10);


        //录像参数
        sChannel.eCapType = (CAPTURE_TYPE)q.getIntFileds("CHAN_CAP_TYPE");
        //录像开始时间
        sChannel.uCapBeginTime = q.getIntFileds("CHAN_CAP_BEGINTIME");
        //录像结束时间
        sChannel.uCapEndTime = q.getIntFileds("CHAN_CAP_ENDTIME");

        //YUV参数
        //主机
        String strMonitorHost =q.getStringFileds("CHAN_YUV_HOST");
        memcpy(sChannel.chMonitorHost,strMonitorHost.c_str(),strMonitorHost.size());
        memcpy(sChannel.chHost,g_ServerHost.c_str(),g_ServerHost.size());
        //端口
        sChannel.uMonitorPort = q.getIntFileds("CHAN_YUV_PORT");

		 //预置位
        sChannel.nPreSet = q.getIntFileds("CHAN_PRESET");

        //通道检测类型
        //sChannel.uDetectKind = (CHANNEL_DETECT_KIND)q.getIntFileds(16);
        CHANNEL_DETECT_KIND uDetectKind = (CHANNEL_DETECT_KIND)q.getIntFileds("CHAN_DETECT_KIND");
        if((uDetectKind&DETECT_VTS)==DETECT_VTS)
        {
            uDetectKind = (CHANNEL_DETECT_KIND)(uDetectKind|DETECT_VIOLATION);
        }
		if(g_nMultiPreSet == 1)//存在多个预置位
		{
			CXmlParaUtil xml;
			int nPreSetDetectKind = xml.LoadPreSetDetectKind(sChannel.uId,sChannel.nPreSet);
			if(nPreSetDetectKind != 0)
			{
				uDetectKind = (CHANNEL_DETECT_KIND)nPreSetDetectKind;
			}
		}
        sChannel.uDetectKind = uDetectKind;
        //白天晚上检测
        sChannel.uDetectTime = (CHANNEL_DETECT_TIME)q.getIntFileds("CHAN_DETECT_TIME");

        //是否事件录象
        sChannel.bEventCapture = q.getIntFileds("CHAN_EVENT_CAPTURE");
        //yuv格式
        sChannel.nYUVFormat = q.getIntFileds("YUV_FORMAT");


        int nRun = q.getIntFileds("CHAN_RUN");
        bool bRun = false;

        if(nRun>0)
        {
            bRun = true;
        }

        //保存相机编号
#ifndef PRESSURE_TEST
        g_nCameraID = q.getIntFileds("CAMERA_ID");
        sChannel.nCameraId = g_nCameraID;
#endif

        String strFileName =q.getStringFileds("CHAN_SRC_FILE");
        memcpy(sChannel.chFileName,strFileName.c_str(),strFileName.size());

        //sChannel.nShowTime = q.getIntFileds(26);
        sChannel.nCameraType = q.getIntFileds("CAMERA_TYPE");

        //String strDirection =q.getStringFileds(28);
        //memcpy(sChannel.chDirection,strDirection.c_str(),strDirection.size());
        sChannel.uDirection = q.getIntFileds("CHAN_DIRECTION");

        sChannel.nPannelID = q.getIntFileds("PANNEL_ID");

        //相邻同步主机
        String strSynHost =q.getStringFileds("SYN_HOST");
        memcpy(sChannel.chSynHost,strSynHost.c_str(),strSynHost.size());

        //相邻同步主机端口
        sChannel.uSynPort = SYNCH_PORT;//q.getIntFileds(31);

        //monitor编号
        sChannel.nMonitorId = q.getIntFileds("MONITOR_ID");

        //视频编号
        sChannel.nVideoIndex = q.getIntFileds("VIDEO_ID");
		//工作方式
		sChannel.nWorkMode = q.getIntFileds("WORKMODE");
		//通道的相机IP
		String strCameraIP = q.getStringFileds("CAMERAIP");
		memcpy(sChannel.chCameraHost, strCameraIP.c_str(), strCameraIP.size());

		String strDeviceId = q.getStringFileds("DEVICE_ID");
		memcpy(sChannel.chDeviceID, strDeviceId.c_str(), strDeviceId.size());


#ifdef _DEBUG
        LogNormal("通道[%d]添加数据!\r\n", sChannel.uId);
#endif
        //添加通道数据
        g_skpChannelCenter.AddChannel(sChannel,bRun,false);
        q.nextRow();
    }
    q.finalize();
    return;
}
//保存视频参数[亮度+饱和度+对比度+色调]
int CSeekpaiDB::ModifyVideoParams(SRIP_CHANNEL_ATTR& sAttr)
{
    char buf[BUFF_SIZE]= {0};
    sprintf(buf,"Update CHAN_INFO set CHAN_BRIGHTNESS = %d,CHAN_CONTRAST = %d,CHAN_SATURATION = %d ,CHAN_HUE = %d where CHAN_ID=%d",
            sAttr.uBrightness,sAttr.uContrast,sAttr.uSaturation,sAttr.uHue,sAttr.uId);

    //执行更新
    String sql(buf);
#ifdef _DEBUG
    printf("保存通道视频参数:%s \n",sql.c_str());
#endif
    if(execSQL(sql)!=0)
    {
        LogError("保存通道视频参数失败[%d]!\r\n",sAttr.uId);
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}

int CSeekpaiDB::GetAdjustPara(SRIP_CHANNEL_ATTR& sAttr)
{

     char buf[BUFF_SIZE]= {0};

        sprintf(buf,"SELECT *  from CHAN_INFO WHERE CHAN_ID = %d",sAttr.uId);
printf("CHAN_ID=%d______________________________________\n",sAttr.uId);
    //SQL 语句
    String sql(buf);
    //执行查询
    MysqlQuery q = execQuery(sql);

//    SRIP_CHANNEL_ATTR *sAttr = (SRIP_CHANNEL_ATTR *) malloc(sizeof(SRIP_CHANNEL_ATTR));
    if(!q.eof())
    {
        //通道ID
    //    sAttr->uId = q.getIntFileds(0);
        //视频参数亮度
        sAttr.uBrightness = q.getIntFileds("CHAN_BRIGHTNESS");
        //视频参数对比度

        sAttr.uContrast = q.getIntFileds("CHAN_CONTRAST");
        //视频参数饱和度
        sAttr.uSaturation = q.getIntFileds("CHAN_SATURATION");
        //视频参数色调
        sAttr.uHue = q.getIntFileds("CHAN_HUE");
    }
    q.finalize();

    return SRIP_OK;
}

/* 车道参数设置 */
int CSeekpaiDB::SaveRoadParaMeterInfo(CSkpRoadXmlValue& xmlValue)
{

    char buf[8192]= {0};						/* 缓冲区			*/
    char buffer[256] = {0};

    SRIP_CHANNEL_EXT sChannel;					/* 通道检测参数*/
    int nChannel;							/* 通道号           */
    int nRoadIndex;							/* 车道的序号	 	*/
    int nModelID;					/* 模板编号			*/

    VEHICLE_PARAM_FOR_EVERY_FRAME sFrame;	/* 车道检测参数		*/
    paraDetectList sFrameList; //链表

    String strQueueLength;					/* 车道队列长度     */
    String strDusaiSpeed;					/* 平均最小速度     */

    nChannel = (int)xmlValue[0]["Channel"];

    //修改通道检测参数
    {

        sChannel.uId = nChannel;
        sChannel.uTrafficStatTime = (int)xmlValue[0]["STAT"];
        sChannel.uEventDetectDelay = (int)xmlValue[0]["EVENT"];
        sChannel.bRmShade = (int)xmlValue[0]["RMSHADE"];
        sChannel.bRmTingle = (int)xmlValue[0]["RMTINGLE"];
        sChannel.bSensitive = (int)xmlValue[0]["SENSITIVE"];
        sChannel.nShowTime = (int)xmlValue[0]["SHOWTIME"];

        sChannel.nHolizonMoveWeight = (int)xmlValue[0]["HOLIZON_MOVE_WEIGHT"];
        sChannel.nVerticalMoveWeight = (int)xmlValue[0]["VERTICAL_MOVE_WEIGHT"];
        sChannel.nZoomScale = (int)xmlValue[0]["ZOOM_SCALE"];
        sChannel.nZoomScale2 = (int)xmlValue[0]["ZOOM_SCALE2"];
        sChannel.nZoomScale3 = (int)xmlValue[0]["ZOOM_SCALE3"];
    }

    for(int i = 0; i < xmlValue.size(); i++)
    {
        nChannel					=	(int)xmlValue[i]["Channel"];		//通道号
        nRoadIndex					=	(int)xmlValue[i]["Index"];			//车道的序号
        sFrame.nChannelID = nRoadIndex;

        String strModelName = (String)xmlValue[i]["ModelName"]; //模板名称
        memcpy(sFrame.chModelName, strModelName.c_str(), strModelName.size());


        sFrame.m_bNixing			=	(bool)xmlValue[i]["IsNixing"];		//是否逆行监测
        sFrame.m_nNixingIgn			=	(int)xmlValue[i]["NixingIgn"];		//逆行事件忽略

        sFrame.m_bPressYellowLine			=	(bool)xmlValue[i]["IsPressLine"];		//是否压线监测
        sFrame.m_nPressYellowLineIgn		=	(int)xmlValue[i]["PressLineIgn"];		//压线监测忽略
		
		sFrame.m_bStatQueueLen           = (bool)xmlValue[i]["StatQueueLen"];   //是否统计排队长度//nahs
		sFrame.m_fVeloStartStat4QL       = (double)xmlValue[i]["VeloStartStat4QL"];    //排队长度开始统计的速度(m/s)
		sFrame.m_fLengthStartStat4QL      = (double)xmlValue[i]["LengthStartStat4QL"];    //排队长度开始统计的长度要求(m)/////////
		sFrame.m_fQLStatIntervalTime     = (double)xmlValue[i]["QLStatIntervalTime"];   //排队长度统计间隔时间(s)

		sFrame.m_bPressLeadStreamLine			=	(bool)xmlValue[i]["IsPressLeadStreamLine"];		//是否压导流线监测
        sFrame.m_nPressLeadStreamLineIgn		=	(int)xmlValue[i]["PressLeadStreamLineIgn"];		//压导流线监测忽略

		sFrame.m_bBianDao			=	(bool)xmlValue[i]["IsBianDao"];		//是否变道监测
        sFrame.m_nBianDaoIgn		=	(int)xmlValue[i]["BianDaoIgn"];		//变道监测忽略
		sFrame.m_nChangeDetectMod = (int)xmlValue[i]["ChangeDetectMod"]; //变道报警类型

        sFrame.m_bStop				=	(bool)xmlValue[i]["IsStop"];		//是否停车监测
        sFrame.m_nStopIgn 			=	(int)xmlValue[i]["StopIgn"];		//停车监测忽略

        sFrame.m_bDusai 			=	(bool)xmlValue[i]["IsDusai"];		//是否堵塞监测
        sFrame.m_nDusaiIgn 			=	(int)xmlValue[i]["DusaiIgn"];		//堵塞监测忽略

        strQueueLength = "";
        strDusaiSpeed = "";
        memset(buf, 0, sizeof(buf));
        for(int j=0; j<5; j++)
        {
            sFrame.m_nWayRate[j] 	=	(int)xmlValue[i]["DusaiSetup"][j]["QueueLength"];		//道路占有率
            int nDusaiSpeed 		=	(int)xmlValue[i]["DusaiSetup"][j]["DusaiSpeed"];		//平均最小速度
            sFrame.m_nDusaiSpeed[j] = nDusaiSpeed;
            printf("nDusaiSpeed=%d\n",nDusaiSpeed);

            if(j != 4)
            {
                sprintf(buf,"%d,",sFrame.m_nWayRate[j]);
            }
            else						/* 避免最后分号	*/
            {
                sprintf(buf,"%d",sFrame.m_nWayRate[j]);
            }
            strQueueLength += buf;

            if(j != 4)
            {
                sprintf(buf,"%d,",nDusaiSpeed);
            }
            else						/* 避免最后分号	*/
            {
                sprintf(buf,"%d",nDusaiSpeed);
            }
            strDusaiSpeed += buf;
        }
        printf("strQueueLength=%s,strDusaiSpeed=%s\n",strQueueLength.c_str(),strDusaiSpeed.c_str());

        sFrame.m_bOnlyOverSped		=	(bool)xmlValue[i]["IsOnlyOverSpeed"];//是否单独超速报警
        sFrame.m_nOnlyOverSpedIgn	=	(int)xmlValue[i]["OnlyOverSpedIgn"]; //单独超速忽略
        sFrame.m_nOnlyOverSpedMax	=	(int)xmlValue[i]["OnlyOverSpedMax"]; //单独超速限制

        sFrame.m_bDiuQi 			=	(bool)xmlValue[i]["IsDiuQi"];		//是否丢弃物监测
        sFrame.m_nDiuQiIgn 			=	(int)xmlValue[i]["DiuQiIgn"];		//丢弃物监测忽略
        sFrame.m_nDiuQiIDam 		=	(int)xmlValue[i]["DiuQiIDam"];		//丢弃物的直径

        sFrame.m_bAvgSpeed 			=	(bool)xmlValue[i]["IsAvgSpeed"];	//是否平均车速监测
        sFrame.m_nAvgSpeedIgn 		=	(int)xmlValue[i]["AvgSpeedIgn"];	//平均车速忽略
        sFrame.m_nAvgSpeedMax 		=	(int)xmlValue[i]["AvgSpeedMax"];	//平均车速最大值
        sFrame.m_nAvgSpeedMin 		=	(int)xmlValue[i]["AvgSpeedMin"];	//平均车速最小值

        sFrame.m_bCross 			=	(bool)xmlValue[i]["IsPerson"];		//是否行人监测
        sFrame.m_nCrossIgn 		   =	(int)xmlValue[i]["PersonIgn"];		//行人监测忽略

        sFrame.m_bStatFlux 			=	(bool)xmlValue[i]["FlowStat"];		//是否监测车道流量
        sFrame.m_bStatSpeedAvg 		=	(bool)xmlValue[i]["AvgSpeedStat"];	//是否监测平均车速
        sFrame.m_bStatZyl 			=	(bool)xmlValue[i]["ZylStat"];		//是否监测平均车道占有率
        sFrame.m_bStatQueue 		=	(bool)xmlValue[i]["QueueStat"];		//是否监测队列长度
        sFrame.m_bStatCtjj 			=	(bool)xmlValue[i]["CtjjStat"];		//是否监测平均车头间距
        sFrame.m_bStatCarType 		=	(bool)xmlValue[i]["CarTypeStat"];	//是否监测车辆分型

        sFrame.m_bBargeIn 		=	(bool)xmlValue[i]["BargeIn"];	//是否白板检测
        //sFrame.is_person_channel  =	(bool)xmlValue[i]["IsFj"];	//是否机动车道检测
        sFrame.nChannelMod =  (int)xmlValue[i]["IsFj"];	//是否机动车道检测
        //printf("sFrame.is_person_channel=%d\n",sFrame.is_person_channel);

        sFrame.m_bBeyondMark  =	(bool)xmlValue[i]["BeyondMark"];	//是否混行检测
        //	sFrame.m_nWrongChanIgn  =(int)xmlValue[i]["HxIgn"];	   //混行检测忽略
        sFrame.m_nAppearIgn  =	(int)xmlValue[i]["AppearIgn"];	//出现检测忽略

        sFrame.alert_level = (int)xmlValue[i]["AlertLevel"];
        sFrame.m_nStopIgnJam = (int)xmlValue[i]["IgnStop"];
        sFrame.m_nStopIgnAlert = (int)xmlValue[i]["IgnStopNotJam"];
        sFrame.m_nDusaiIgnAlert = (int)xmlValue[i]["Jam"];
        sFrame.is_right_side = (bool)xmlValue[i]["is_right_side"];

        sFrame.is_half_maxsize  =	(bool)xmlValue[i]["bHalfMaxSize"];
        sFrame.m_nAgainstDetectMod = (int)xmlValue[i]["bPersonAgainst"];
        sFrame.m_bObjectAppear  =	(bool)xmlValue[i]["bAppear"];
        sFrame.m_nCrossDetectMod   =   (int)xmlValue[i]["bCross"];
        sFrame.m_nObjeceDetectMod = (int)xmlValue[i]["bCarAppear"];
        int nAngle = (int)xmlValue[i]["Angle"];
        sFrame.m_nAngle = nAngle;

        sFrame.m_nForbidType = (int)xmlValue[i]["ForbidType"];
        sFrame.m_nAllowBigBeginTime = (int)xmlValue[i]["AllowBigBeginTime"];
        sFrame.m_nAllowBigEndTime = (int)xmlValue[i]["AllowBigEndTime"];

        sFrame.m_bCrowd = (bool)xmlValue[i]["Crowd"];
        sFrame.m_nPersonCount = (int)xmlValue[i]["PersonCount"];
        sFrame.m_nAreaPercent = (int)xmlValue[i]["AreaPercent"];
        sFrame.m_bPersonRun = (bool)xmlValue[i]["PersonRun"];
        sFrame.m_fMaxRunSpeed = (double)xmlValue[i]["MaxRunSpeed"];

    //    printf("================sFrame.m_fMaxRunSpeed=%lf,%d\n",sFrame.m_fMaxRunSpeed,xmlValue[i]["MaxRunSpeed"]);

        bool bModel = (bool) xmlValue[i]["IsModel"];

        if(!bModel)
        {
            sFrame.nModelId = 0;
        }
        else
        {
            nModelID = (int) xmlValue[i]["ModelID"];
            sFrame.nModelId = nModelID;
        }

        sFrameList.push_back(sFrame);
    }
#ifdef _DEBUG
    printf("===========Before CSeekpaiDB::SaveRoadParaMeterInfo======AddRoadParameterByList=============\n");
#endif
    //写入到xml文件中
    CXmlParaUtil xml;
    if( (g_nMultiPreSet == 0) && (g_nSwitchCamera == 0))
    {
        xml.AddRoadParameterByList(sFrameList,sChannel);
    }
    else
    {
        int nCameraID = GetCameraID(nChannel);
        int nPreSet = 0;
        if(g_nMultiPreSet == 1)
        {
          nPreSet = GetPreSet(nChannel);
        }

        if(nPreSet >= 0)
        xml.AddDeviceParaMeterInfo(sFrameList,sChannel,nCameraID,nPreSet);
    }
    ////////////////////////////
#ifdef _DEBUG
    printf("===========After CSeekpaiDB::SaveRoadParaMeterInfo======AddRoadParameterByList=============\n");
#endif
    //要求检测重新读取通道配置
    g_skpChannelCenter.ReloadChannelParaMeter(sChannel);

	//鞍山交警中心端
	/*if(g_nServerType == 10)
	{
		CXmlParaUtil xml;
		//if(nChannel == 1)
		//	xml.GetMaxSpeed(g_TJServer.m_maxSpeedMap, nChannel);
		if(nChannel == 1)
		{
			xml.GetMaxSpeedStr(g_TJServer.m_maxSpeedMapStr, nChannel);
		}
	}
	*/

    return SRIP_OK;
}

/* 车道模板参数设置 */
int CSeekpaiDB::SaveRoadModelInfo(CSkpRoadXmlValue& xmlValue, int& nModelID)
{
#ifdef _DEBUG
    printf("===========Begin CSeekpaiDB::SaveRoadModelInfo===================\n");
#endif
    char buf[8192]= {0};						/* 缓冲区			*/

    VEHICLE_PARAM_FOR_EVERY_FRAME sFrame;	/* 通道检测参数		*/

    String strQueueLength;					/* 车道队列长度     */
    String strDusaiSpeed;					/* 平均最小速度     */

    int nAngle;
    for(int i = 0; i < xmlValue.size(); i++)
    {
        int nModelKind = (int)xmlValue[i]["ModelKind"];
        nModelID =  (int)xmlValue[i]["ModelID"];
        sFrame.nModelId = nModelID;
        printf("==========sFrame.nModelId===%d================\n",sFrame.nModelId);

        if( nModelKind <2 )
        {
            String strModelName = (String)xmlValue[i]["ModelName"]; //模板名称
            memcpy(sFrame.chModelName, strModelName.c_str(), strModelName.size());

            sFrame.m_bNixing			=	(bool)xmlValue[i]["IsNixing"];		//是否逆行监测
            sFrame.m_nNixingIgn			=	(int)xmlValue[i]["NixingIgn"];		//逆行事件忽略

            sFrame.m_bBianDao			=	(bool)xmlValue[i]["IsBianDao"];		//是否变道监测
            sFrame.m_nBianDaoIgn		=	(int)xmlValue[i]["BianDaoIgn"];		//变道监测忽略
			sFrame.m_nChangeDetectMod = (int)xmlValue[i]["ChangeDetectMod"]; //变道报警类型

			sFrame.m_bPressYellowLine	=	(bool)xmlValue[i]["IsPressLine"];		//是否压线监测
			sFrame.m_nPressYellowLineIgn=	(int)xmlValue[i]["PressLineIgn"];		//压线监测忽略

			sFrame.m_bStatQueueLen           = (bool)xmlValue[i]["StatQueueLen"];      //是否统计排队长度       //nahs
			sFrame.m_fVeloStartStat4QL       = (double)xmlValue[i]["VeloStartStat4QL"];   //排队长度开始统计的速度(m/s)
			sFrame.m_fLengthStartStat4QL     = (double)xmlValue[i]["LengthStartStat4QL"];    //排队长度开始统计的长度要求(m)/////////
			sFrame.m_fQLStatIntervalTime     = (double)xmlValue[i]["QLStatIntervalTime"];   //排队长度统计间隔时间(s)


			 sFrame.m_bPressLeadStreamLine			=	(bool)xmlValue[i]["IsPressLeadStreamLine"];		//是否压导流线监测
			sFrame.m_nPressLeadStreamLineIgn		=	(int)xmlValue[i]["PressLeadStreamLineIgn"];		//压导流线监测忽略

            sFrame.m_bStop				=	(bool)xmlValue[i]["IsStop"];		//是否停车监测
            sFrame.m_nStopIgn 			=	(int)xmlValue[i]["StopIgn"];		//停车监测忽略

            sFrame.m_bDusai 			=	(bool)xmlValue[i]["IsDusai"];		//是否堵塞监测
            sFrame.m_nDusaiIgn 			=	(int)xmlValue[i]["DusaiIgn"];		//堵塞监测忽略

            strQueueLength = "";
            strDusaiSpeed = "";
            memset(buf, 0, sizeof(buf));
            for(int j=0; j<5; j++)
            {
                sFrame.m_nWayRate[j] 	=	(int)xmlValue[i]["DusaiSetup"][j]["QueueLength"];		//道路占有率
                int nDusaiSpeed 		=	(int)xmlValue[i]["DusaiSetup"][j]["DusaiSpeed"];		//平均最小速度
                sFrame.m_nDusaiSpeed[j] = nDusaiSpeed;
                printf("nDusaiSpeed=%d\n",nDusaiSpeed);

                if(j != 4)
                {
                    sprintf(buf,"%d,",sFrame.m_nWayRate[j]);
                }
                else						/* 避免最后分号	*/
                {
                    sprintf(buf,"%d",sFrame.m_nWayRate[j]);
                }
                strQueueLength += buf;

                if(j != 4)
                {
                    sprintf(buf,"%d,",nDusaiSpeed);
                }
                else						/* 避免最后分号	*/
                {
                    sprintf(buf,"%d",nDusaiSpeed);
                }
                strDusaiSpeed += buf;
            }
            printf("strQueueLength=%s,strDusaiSpeed=%s\n",strQueueLength.c_str(),strDusaiSpeed.c_str());

            sFrame.m_bOnlyOverSped		=	(bool)xmlValue[i]["IsOnlyOverSpeed"];//是否单独超速报警
            sFrame.m_nOnlyOverSpedIgn	=	(int)xmlValue[i]["OnlyOverSpedIgn"]; //单独超速忽略
            sFrame.m_nOnlyOverSpedMax	=	(int)xmlValue[i]["OnlyOverSpedMax"]; //单独超速限制

            sFrame.m_bDiuQi 			=	(bool)xmlValue[i]["IsDiuQi"];		//是否丢弃物监测
            sFrame.m_nDiuQiIgn 			=	(int)xmlValue[i]["DiuQiIgn"];		//丢弃物监测忽略
            sFrame.m_nDiuQiIDam 		=	(int)xmlValue[i]["DiuQiIDam"];		//丢弃物的直径

            sFrame.m_bAvgSpeed 			=	(bool)xmlValue[i]["IsAvgSpeed"];	//是否平均车速监测
            sFrame.m_nAvgSpeedIgn 		=	(int)xmlValue[i]["AvgSpeedIgn"];	//平均车速忽略
            sFrame.m_nAvgSpeedMax 		=	(int)xmlValue[i]["AvgSpeedMax"];	//平均车速最大值
            sFrame.m_nAvgSpeedMin 		=	(int)xmlValue[i]["AvgSpeedMin"];	//平均车速最小值

            sFrame.m_bCross 			=	(bool)xmlValue[i]["IsPerson"];		//是否行人监测
            sFrame.m_nCrossIgn 		   =	(int)xmlValue[i]["PersonIgn"];		//行人监测忽略

            sFrame.m_bStatFlux 			=	(bool)xmlValue[i]["FlowStat"];		//是否监测车道流量
            sFrame.m_bStatSpeedAvg 		=	(bool)xmlValue[i]["AvgSpeedStat"];	//是否监测平均车速
            sFrame.m_bStatZyl 			=	(bool)xmlValue[i]["ZylStat"];		//是否监测平均车道占有率
            sFrame.m_bStatQueue 		=	(bool)xmlValue[i]["QueueStat"];		//是否监测队列长度
            sFrame.m_bStatCtjj 			=	(bool)xmlValue[i]["CtjjStat"];		//是否监测平均车头间距
            sFrame.m_bStatCarType 		=	(bool)xmlValue[i]["CarTypeStat"];	//是否监测车辆分型

            sFrame.m_bBargeIn 		=	(bool)xmlValue[i]["BargeIn"];	//是否白板检测
            //sFrame.is_person_channel  =	(bool)xmlValue[i]["IsFj"];	//是否机动车道检测
            sFrame.nChannelMod =  (int)xmlValue[i]["IsFj"];	//是否机动车道检测
            //printf("sFrame.is_person_channel=%d\n",sFrame.is_person_channel);

            sFrame.m_bBeyondMark  =	(bool)xmlValue[i]["BeyondMark"];	//是否混行检测
            //	sFrame.m_nWrongChanIgn  =(int)xmlValue[i]["HxIgn"];	   //混行检测忽略
            sFrame.m_nAppearIgn  =	(int)xmlValue[i]["AppearIgn"];	//出现检测忽略

            sFrame.alert_level = (int)xmlValue[i]["AlertLevel"];
            sFrame.m_nStopIgnJam = (int)xmlValue[i]["IgnStop"];
            sFrame.m_nStopIgnAlert = (int)xmlValue[i]["IgnStopNotJam"];
            sFrame.m_nDusaiIgnAlert = (int)xmlValue[i]["Jam"];
            sFrame.is_right_side = (bool)xmlValue[i]["is_right_side"];

            sFrame.is_half_maxsize  =	(bool)xmlValue[i]["bHalfMaxSize"];
            sFrame.m_nAgainstDetectMod = (int)xmlValue[i]["bPersonAgainst"];
            sFrame.m_bObjectAppear  =	(bool)xmlValue[i]["bAppear"];
            sFrame.m_nCrossDetectMod   =   (int)xmlValue[i]["bCross"];
            sFrame.m_nObjeceDetectMod = (int)xmlValue[i]["bCarAppear"];
            nAngle = (int)xmlValue[i]["Angle"];

            sFrame.m_nAngle = (int)xmlValue[i]["Angle"];

            sFrame.m_nForbidType = (int)xmlValue[i]["ForbidType"];
            sFrame.m_nAllowBigBeginTime = (int)xmlValue[i]["AllowBigBeginTime"];
            sFrame.m_nAllowBigEndTime = (int)xmlValue[i]["AllowBigEndTime"];


            sFrame.m_bCrowd = (bool)xmlValue[i]["Crowd"];
            sFrame.m_nPersonCount = (int)xmlValue[i]["PersonCount"];
            sFrame.m_nAreaPercent = (int)xmlValue[i]["AreaPercent"];
            sFrame.m_bPersonRun = (bool)xmlValue[i]["PersonRun"];
            sFrame.m_fMaxRunSpeed = (double)xmlValue[i]["MaxRunSpeed"];

            printf("m_nPersonCount=%d,m_nAreaPercent=%d,m_bPersonRun=%d,m_fMaxRunSpeed=%f\n",sFrame.m_nPersonCount,sFrame.m_nPersonCount,sFrame.m_bPersonRun,sFrame.m_fMaxRunSpeed);
        }

        //写入到xml文件中
        CXmlParaUtil xml;
        printf("nModelKind=%d,nModelID=%d\n",nModelKind,nModelID);
        xml.AddRoadParameterModel(sFrame,nModelKind,nModelID);
    } //End of for i


#ifdef _DEBUG
    printf("===========After CSeekpaiDB::SaveRoadModelInfo===================\n");
#endif
    return SRIP_OK;
}


// 保存车道坐标信息
int CSeekpaiDB::SaveRoadSettingInfo(CSkpRoadXmlValue& xmlValue, int nChannelId)
{

#ifdef _DEBUG
    printf("===========begin CSeekpaiDB::SaveRoadSettingInfo===================\n");
#endif

    LIST_CHANNEL_INFO list_channel_info; //车道列表--对应某个通道下的

    char buf[8192]= {0};					/* 缓冲区			*/
    char buffer[256] = {0};

    String strRoadway;                  /* 道路区域         */
    String strChannelway;				/* 车道坐标集合		*/
    String strParkCoordin;				/* 停车区域集合		*/
    String strPersonCoordin;			/* 行人监测区域集合	*/
    String strTrashCoordin;				/* 丢弃物区域集合	*/

    //String strFlowLineCoordin;			/* 流量线集合		*/
    String strAmountLineCoordin;			/* 流量线集合		*/

    String strRefLineCoordin;			/* 参考线集合		*/
    String strCalibrationCoordin;		/* 车道区域集合		*/
    String strCalibrationPara;			/* 车道区域坐标		*/
    String strDirectionCoordin;		    /* 车道方向坐标		*/
    String strTurnRoadCoordin;		    /* 变道区域集合		*/
    String strSkipCoordin;				/*	排除区域        */
    String strCardCoordin;		        /* 车牌区域集合		*/
    String strVirtualLoopCoordin;      /*虚拟线圈区域*/
    String strRemotePerson;      /*远处行人框*/
    String strLocalPerson;      /*近处行人框*/

    int nRoadwayCoordinNum = 0;			/* 道路点数量		*/
    int nChannelwayCoordinNum = 0;		/* 车道点数量		*/

    int nChannel;						/* 通道号			*/
    int nRoadIndex;						/* 车道的序号	 	*/
    int nModelIndex;					/* 模板编号			*/
    int nVTSIndex;                     /*闯红灯检测参数编号*/
    String strRoadName;					/* 车道的名称		*/
    int nDirection;						/* 车道路的方向		*/

    /////////////////////////////////////////线圈
    String strFlowBackCoordin;          /* 线圈背景区域集合		*/
    String strFlowLineCoordin;          /* 线圈流量检测线集合	*/
    String strFlowFramegetCoordin;      /* 线圈取景区域集合		*/
    /////////////////////////////////////////

    /////////////////////////////////////////////闯红灯--begin
    String strViolationCoordin;     //违章检测区域
    String strEventCoordin;         //事件检测区域
    String strTrafficSignalCoordin; //红灯检测区域
    String strStopLineCoordin;      //停止检测线
    String strStraightLineCoordin;  //直行检测线
    String strTurnLeftCoordin;      //左转检测线
    String strTurnRightCoordin;     //右转检测线
    String strYellowLineCoordin;    //黄线
	String strWhiteLineCoordin;      //白线
	String strPressLeadStreamLineCoordin; //导流线
    String strLineStopCoordin;      //单车道停止线
    String strRedLightRegionCoordin; //红灯区域
    String strGreenLightRegionCoordin; //绿灯区域
    String strLeftLightRegionCoordin; //左转灯区域
    String strRightLightRegionCoordin; //右转灯区域
    String strStraightLightRegionCoordin; //直行灯区域
    String strTurnAroundLightRegionCoordin; //禁止转向灯区域
    String strLeftRedLightRegionCoordin; //左转红灯区域
    String strLeftGreenLightRegionCoordin; //左转绿灯区域
    String strRightRedLightRegionCoordin; //右转红灯区域
    String strRightGreenLightRegionCoordin; //右转绿灯区域
    String strStraightRedLightRegionCoordin; //直行红灯区域
    String strStraightGreenLightRegionCoordin; //直行绿灯区域
    String strTurnAroundRedLightRegionCoordin; //禁止转向红灯区域
    String strTurnAroundGreenLightRegionCoordin; //禁止转向绿灯区域
    String strLineStraightCoordin;      //单车道前行线
	String strHoldForeLineFirst;      //待转区第一前行线
	String strHoldForeLineSecond;      //待转区第二前行线
	String strHoldStopLineFirst;      //待转区第一停止线
	String strHoldStopLineSecond;      //待转区第二停止线
    /////////////////////////////////////////////闯红灯--end

    String strStabBackCoordin;      //稳像背景区域
    String strSynchLeftCoordin;         //同步标志点区域Left
    String strSynchRightCoordin;         //同步标志点区域Right

    String strCardnumberCoordin;    //车牌区域2
    String strLoopCoordin; //测速线圈

    String strBargeInRegion; //闯入区域
    String strBeyondMarkRegion; //越界区域
    String strRadarRegion; //雷达区域
	String strDensityRegion; //密度区域
	String strGetPhotoRegion; //抓图区域

	String strYelGridRegion; //黄网格区域
	//电警第一触发线
    String strViolationFirstLineCoordin;
	//电警第一触发线
    String strViolationSecondLineCoordin;
	//禁右初始触发线
    String strRightFirstLineCoordin;
	//禁左初始触发线
    String strLeftFirstLineCoordin;
    //禁前初始触发线
	String strForeFirstLineCoordin;

    double x;									/* x坐标		*/
    double y;									/* y坐标		*/
    int nCoordin;							/* k值			*/
    int i = 0;
    int j = 0;

	if(nChannelId == 0)
	{
		nChannel		=	(int)xmlValue[0]["Channel"];		//通道号
	}
	else
	{
		nChannel = nChannelId;
	}

    

    //获取车道编号列表
    String strNewChannelways;
    for(i = 0; i < xmlValue.size(); i++)
    {
        nRoadIndex	=	(int)xmlValue[i]["Index"];			//车道的序号
        strNewChannelways.append((char*)&nRoadIndex,sizeof(int));
    }
    //////////////////////////////////////
    for(i = 0; i < xmlValue.size(); i++)
    {
        CHANNEL_INFO Channel_info;								//通道信息

        nChannel					=	(int)xmlValue[i]["Channel"];		//通道号
        nRoadIndex					=	(int)xmlValue[i]["Index"];			//车道的序号

        strRoadName					=	(String)xmlValue[i]["RoadName"];	//车道的名称
        nDirection					=	(int)xmlValue[i]["Direction"];		//车道的方向

        memset(buf, 0, sizeof(buf));
        nRoadwayCoordinNum	= (int)xmlValue[i]["Roadway"].size();
        nChannelwayCoordinNum = (int)xmlValue[i]["Channelway"].size();

        /* Save to struct */
        strcpy(Channel_info.chProp_index.strName, "CHANNEL_INDEX");
        Channel_info.chProp_index.value.nValue = nRoadIndex;
#ifdef _DEBUG
        printf("Channel_info.chProp_index.strName:%s \n",Channel_info.chProp_index.strName);
#endif

        strcpy(Channel_info.chProp_name.strName, "CHANNEL_NAME");
        strcpy(Channel_info.chProp_name.value.strValue , strRoadName.c_str());

        strcpy(Channel_info.chProp_direction.strName, "CHANNEL_DIRECT");
        Channel_info.chProp_direction.value.nValue = nDirection;

        // 更新车道信息
        CPoint32f pt;
        if(nChannelwayCoordinNum > 0)
        {
            // 道路
            strRoadway = "";
            strcpy(Channel_info.roadRegion.chProperty.strName, "ROAD_REGION");
            Channel_info.roadRegion.chProperty.value.nValue = nRoadwayCoordinNum;
            for(int j = 0; j < nRoadwayCoordinNum; j++)
            {
                //直接使用xml传过来的数据
                pt.x = xmlValue[i]["Roadway"][j]["x"];
                pt.y = xmlValue[i]["Roadway"][j]["y"];
                Channel_info.roadRegion.listPT.push_back(pt);

                /*
                * 存入数据库中时坐标保留3位小数
                */
                if(j != nRoadwayCoordinNum-1)
                {
                    sprintf(buf,"%.3f,%.3f|",pt.x, pt.y);
                }
                else						//避免最后分号
                {
                    sprintf(buf,"%.3f,%.3f",pt.x, pt.y);
                }
                strRoadway += buf;

#ifdef _DEBUG
                printf("--------------------%s\r\n", strRoadway.c_str());
#endif
            }
            // 车道
            memset(buf, 0, sizeof(buf));
            strChannelway = "";
            strcpy(Channel_info.chRegion.chProperty.strName, "CHANNEL_REGION");
            Channel_info.chRegion.chProperty.value.nValue = nChannelwayCoordinNum;
            printf("===============nChannelwayCoordinNum=%d\n",nChannelwayCoordinNum);
            for(j = 0; j < nChannelwayCoordinNum; j++)
            {
                //直接使用xml传过来的数据
                pt.x = xmlValue[i]["Channelway"][j]["x"];
                pt.y = xmlValue[i]["Channelway"][j]["y"];
                Channel_info.chRegion.listPT.push_back(pt);

                /*
                * 存入数据库中时坐标保留3位小数
                */
                if(j != nChannelwayCoordinNum-1)
                {
                    sprintf(buf,"%.3f,%.3f|", pt.x, pt.y);
                }
                else						/* 避免最后分号	*/
                {
                    sprintf(buf,"%.3f,%.3f", pt.x, pt.y);
                }
                strChannelway += buf;
            }

            // 停车区域
            strParkCoordin		= SaveCommonRegion("STOP_REGION",xmlValue[i]["ParkArea"],Channel_info);

            // 行人监测区域
            strPersonCoordin	= SaveCommonRegion("PER_REGION",xmlValue[i]["PersonArea"],Channel_info);

            // 丢弃物区域
            strTrashCoordin		= SaveCommonRegion("DROP_REGION",xmlValue[i]["TrashArea"],Channel_info);

            // 流量线区域
            strAmountLineCoordin	= SaveCommonRegion("AMOUNT_LINE",xmlValue[i]["AmountLine"],Channel_info);

            // 参考线区域
            strRefLineCoordin	= SaveCommonRegion("REF_LINE",xmlValue[i]["RefLine"],Channel_info);

            //变道线区域
            strTurnRoadCoordin  = SaveCommonRegion("TURNROAD_LINE",xmlValue[i]["TurnRoad"],Channel_info);

            //排除区域
            strSkipCoordin  = SaveCommonRegion("SKIP_REGION",xmlValue[i]["SkipArea"],Channel_info);

            //车牌区域
            strCardCoordin = SaveCommonRegion("CARNUM_REGION",xmlValue[i]["CardArea"],Channel_info);

            //虚拟线圈区域
            strVirtualLoopCoordin = SaveCommonRegion("VirtualLoop",xmlValue[i]["VirtualLoop"],Channel_info);

            //远处行人框
            strRemotePerson = SaveCommonRegion("RemotePerson",xmlValue[i]["RemotePerson"],Channel_info);

            //近处行人框
            strLocalPerson = SaveCommonRegion("LocalPerson",xmlValue[i]["LocalPerson"],Channel_info);

            /////////////////////////////////////////线圈
            //线圈背景区域
            strFlowBackCoordin	    = SaveCommonRegion("FLOWBACK_REGION",xmlValue[i]["FlowBackArea"],Channel_info);
            //线圈流量检测线
            strFlowLineCoordin	    = SaveCommonRegion("FLOW_LINE",xmlValue[i]["FlowLine"],Channel_info);
            //线圈取景区域
            strFlowFramegetCoordin	= SaveCommonRegion("FLOWFRAMEGET_REGION",xmlValue[i]["FlowFreamegetArea"],Channel_info);
            /////////////////////////////////////////

            /////////////////////////////////////////////闯红灯--begin
            //违章检测区域
            strViolationCoordin	    = SaveCommonRegion("VIOLATION_REGION",xmlValue[i]["ViolationArea"],Channel_info);
            //事件检测区域
            strEventCoordin	    = SaveCommonRegion("EVENT_REGION",xmlValue[i]["EventArea"],Channel_info);
            //红灯检测区域
            strTrafficSignalCoordin	    = SaveCommonRegion("TRAFFIC_SIGNAL_REGION",xmlValue[i]["TrafficSignalArea"],Channel_info);

            //停止检测线
            strStopLineCoordin	    = SaveCommonRegion("STOP_LINE",xmlValue[i]["StopLine"],Channel_info);
            //直行检测线MysqlQuery
            strStraightLineCoordin	    = SaveCommonRegion("STRAIGHT_LINE",xmlValue[i]["StraightLine"],Channel_info);
            //左转检测线
            strTurnLeftCoordin	    = SaveCommonRegion("TURN_LEFT_LINE",xmlValue[i]["TurnLeftLine"],Channel_info);
			 //右转检测线
            strTurnRightCoordin	    = SaveCommonRegion("TURN_RIGHT_LINE",xmlValue[i]["TurnRightLine"],Channel_info);

			//电警第一触发线
            strViolationFirstLineCoordin	    = SaveCommonRegion("VIOLATION_FIRST_LINE",xmlValue[i]["ViolationFirstLine"],Channel_info);
			//电警第一触发线
            strViolationSecondLineCoordin	    = SaveCommonRegion("VIOLATION_SECOND_LINE",xmlValue[i]["ViolationSecondLine"],Channel_info);
			//禁右初始触发线
            strRightFirstLineCoordin	    = SaveCommonRegion("RIGHT_FIRST_LINE",xmlValue[i]["RightFirstLine"],Channel_info);
			//禁左初始触发线
            strLeftFirstLineCoordin	    = SaveCommonRegion("LEFT_FIRST_LINE",xmlValue[i]["LeftFirstLine"],Channel_info);
			//禁前初始触发线
            strForeFirstLineCoordin	    = SaveCommonRegion("FORE_FIRST_LINE",xmlValue[i]["ForeFirstLine"],Channel_info);
           
            //黄线
            strYellowLineCoordin	    = SaveCommonRegion("YELLOW_LINE",xmlValue[i]["YellowLine"],Channel_info);
			//白线
			strWhiteLineCoordin	    = SaveCommonRegion("WHITE_LINE",xmlValue[i]["WhiteLine"],Channel_info);
			//导流线
            strPressLeadStreamLineCoordin	    = SaveCommonRegion("LEAD_STREAM_LINE",xmlValue[i]["LeadStreamLine"],Channel_info);
			//待转区第一前行线
			strHoldForeLineFirst	    = SaveCommonRegion("HOLD_FORE_FIRST_LINE",xmlValue[i]["HoldForeFirstLine"],Channel_info);
			//待转区第二前行线
			strHoldForeLineSecond	    = SaveCommonRegion("HOLD_FORE_SECOND_LINE",xmlValue[i]["HoldForeSecondLine"],Channel_info);
			//待转区第一停止线
			strHoldStopLineFirst	    = SaveCommonRegion("HOLD_STOP_FIRST_LINE",xmlValue[i]["HoldStopFirstLine"],Channel_info);
			//待转区第二停止线
			strHoldStopLineSecond	    = SaveCommonRegion("HOLD_STOP_SECOND_LINE",xmlValue[i]["HoldStopSecondLine"],Channel_info);

            //单车道停止线
            strLineStopCoordin	    = SaveCommonRegion("LINE_STOP",xmlValue[i]["LineStop"],Channel_info);
            //红灯区域
            strRedLightRegionCoordin	    = SaveCommonRegion("RedLightRegion",xmlValue[i]["RedLightArea"],Channel_info);
            //绿灯区域
            strGreenLightRegionCoordin	    = SaveCommonRegion("GreenLightRegion",xmlValue[i]["GreenLightArea"],Channel_info);
            //左转灯区域
            strLeftLightRegionCoordin	    = SaveCommonRegion("LeftLightRegion",xmlValue[i]["LeftLightArea"],Channel_info);
            //右转灯区域
            strRightLightRegionCoordin	    = SaveCommonRegion("RightLightRegion",xmlValue[i]["RightLightArea"],Channel_info);
            //直行灯区域
            strStraightLightRegionCoordin	    = SaveCommonRegion("StraightLightRegion",xmlValue[i]["StraightLightArea"],Channel_info);
            //禁止转向灯区域
            strTurnAroundLightRegionCoordin	    = SaveCommonRegion("TurnAroundLightRegion",xmlValue[i]["TurnAroundLightArea"],Channel_info);
            //左转红灯区域
            strLeftRedLightRegionCoordin	    = SaveCommonRegion("LeftRedLightRegion",xmlValue[i]["LeftRedLightArea"],Channel_info);
            //左转绿灯区域
            strLeftGreenLightRegionCoordin	    = SaveCommonRegion("LeftGreenLightRegion",xmlValue[i]["LeftGreenLightArea"],Channel_info);
            //右转红灯区域
            strRightRedLightRegionCoordin	    = SaveCommonRegion("RightRedLightRegion",xmlValue[i]["RightRedLightArea"],Channel_info);
            //右转绿灯区域
            strRightGreenLightRegionCoordin	    = SaveCommonRegion("RightGreenLightRegion",xmlValue[i]["RightGreenLightArea"],Channel_info);
            //直行红灯区域
            strStraightRedLightRegionCoordin	    = SaveCommonRegion("StraightRedLightRegion",xmlValue[i]["StraightRedLightArea"],Channel_info);
            //直行绿灯区域
            strStraightGreenLightRegionCoordin	    = SaveCommonRegion("StraightGreenLightRegion",xmlValue[i]["StraightGreenLightArea"],Channel_info);
            //禁止转向红灯区域
            strTurnAroundRedLightRegionCoordin	    = SaveCommonRegion("TurnAroundRedLightRegion",xmlValue[i]["TurnAroundRedLightArea"],Channel_info);
            //禁止转向绿灯区域
            strTurnAroundGreenLightRegionCoordin	    = SaveCommonRegion("TurnAroundGreenLightRegion",xmlValue[i]["TurnAroundGreenLightArea"],Channel_info);
            //单车道前行线
            strLineStraightCoordin = SaveCommonRegion("LineStraight",xmlValue[i]["LineStraight"],Channel_info);
            /////////////////////////////////////////////闯红灯--end

            //稳像背景区域
            strStabBackCoordin	    = SaveCommonRegion("STAB_BACK_REGION",xmlValue[i]["StabBackArea"],Channel_info);
            //同步标志点区域Left
            strSynchLeftCoordin	    = SaveCommonRegion("SYNCH_LEFT_REGION",xmlValue[i]["SynchLeftArea"],Channel_info);
            //同步标志点区域Right
            strSynchRightCoordin	= SaveCommonRegion("SYNCH_RIGHT_REGION",xmlValue[i]["SynchRightArea"],Channel_info);

            //车牌区域2
            strCardnumberCoordin    = SaveCommonRegion("CARDNUMBER_REGION",xmlValue[i]["CardnumberArea"],Channel_info);
            //测速线圈
            strLoopCoordin    = SaveCommonRegion("LOOP_LINE",xmlValue[i]["LoopLine"],Channel_info);

            // 闯入区域
            strBargeInRegion		= SaveCommonRegion("BARGEIN_REGION",xmlValue[i]["BargeInRegion"],Channel_info);

            // 越界区域
            strBeyondMarkRegion		= SaveCommonRegion("BEYONDMARK_REGION",xmlValue[i]["BeyondMarkRegion"],Channel_info);

            // 雷达区域
            strRadarRegion = SaveCommonRegion("RADAR_REGION",xmlValue[i]["RadarRegion"],Channel_info);

			// 密度区域
			strDensityRegion = SaveCommonRegion("DENSITY_REGION",xmlValue[i]["Density"],Channel_info);

			//抓图区域
			strGetPhotoRegion = SaveCommonRegion("GETPHOTO_REGION",xmlValue[i]["GetPhotoRegion"],Channel_info);

			//黄网格区域
			strYelGridRegion = SaveCommonRegion("NO_PARKING_REGION",xmlValue[i]["NoParkingArea"],Channel_info);

            /* 车道标定参数 */
            double dRoadLegth		= (double) xmlValue[i]["Calibration-Length"];			// 车道长度
            double dRoadWidth		= (double) xmlValue[i]["Calibration-Width"];				// 车道宽度
            double dCameraHeight	= (double) xmlValue[i]["Calibration-CameraHeight"];		// 摄像机高度

            Channel_info.calibration.length = dRoadLegth;
            Channel_info.calibration.width = dRoadWidth;
            Channel_info.calibration.cameraHeight = dCameraHeight;


            memset(buf, 0, sizeof(buf));		/* Clean the buffer	*/
            //sprintf(buf, "%.2f|%.2f|%.2f|%.2f|%.2f",dRoadLegth,dRoadWidth,dCameraHeight,dCameraDistance,dCameraOrientation);
            sprintf(buf, "%.2f|%.2f|%.2f",dRoadLegth,dRoadWidth,dCameraHeight);
            strCalibrationPara = buf;



            /*车道标定辅助点世界坐标*/
            memset(buf, 0, sizeof(buf));		/* Clean the buffer	*/
            nCoordin = xmlValue[i]["Calibration32fPoint"].size();
            for(int j = 0; j < nCoordin; j++)
            {
                pt.x = xmlValue[i]["Calibration32fPoint"][j]["x"];
                pt.y = xmlValue[i]["Calibration32fPoint"][j]["y"];
                pt.z = xmlValue[i]["Calibration32fPoint"][j]["z"];
                Channel_info.calibration.list32fPT.push_back(pt);

                //将标定辅助点世界坐标作为参数放在标定区域参数后面
                sprintf(buf, "|%.2f,%.2f,%.2f", pt.x,pt.y,pt.z);
                strCalibrationPara += buf;

                printf("pt.x = %.2f,%.2f,%.2f\n", pt.x,pt.y,pt.z);
            }

            printf( "dRoadLegth=%.2f|dRoadWidth=%.2f|dCameraHeight=%.2f,nCoordin=%d\n",dRoadLegth,dRoadWidth,dCameraHeight,nCoordin);

            /* 车道标定区域 */
            memset(buf, 0, sizeof(buf));		/* Clean the buffer	*/
            nCoordin = xmlValue[i]["Calibration"].size();

            strCalibrationCoordin = "";
            strcpy(Channel_info.calibration.region.chProperty.strName, "CALIBRATION");
            Channel_info.calibration.region.chProperty.value.nValue = nCoordin+ xmlValue[i]["CalibrationPoint"].size();//标定区域点个数加上辅助点个数

            for(j = 0; j < nCoordin; j++)
            {
                //直接使用xml传过来的数据
                pt.x = xmlValue[i]["Calibration"][j]["x"];
                pt.y = xmlValue[i]["Calibration"][j]["y"];
                Channel_info.calibration.region.listPT.push_back(pt);

#ifdef _DEBUG
                printf("Calibration:(x, y): %.3f, %.3f", pt.x, pt.y);
#endif

                if(j != nCoordin-1)
                {
                    sprintf(buf,"%.3f,%.3f|", pt.x, pt.y);
                }
                else						/* 避免最后分号	*/
                {
                    sprintf(buf,"%.3f,%.3f", pt.x, pt.y);
                }
                strCalibrationCoordin += buf;
            }
            /*车道标定辅助点*/
            memset(buf, 0, sizeof(buf));		/* Clean the buffer	*/
            nCoordin = xmlValue[i]["CalibrationPoint"].size();

            for(j = 0; j < nCoordin; j++)
            {
                //直接使用xml传过来的数据
                pt.x = xmlValue[i]["CalibrationPoint"][j]["x"];
                pt.y = xmlValue[i]["CalibrationPoint"][j]["y"];

                Channel_info.calibration.listPT.push_back(pt);

                //将标定辅助点坐标放在标定区域后面,避免修改数据库
                sprintf(buf,"|%.3f,%.3f", pt.x, pt.y);
                strCalibrationCoordin += buf;
            }


            /*车道方向坐标(坐标不用写到配置文件)*/
            memset(buf, 0, sizeof(buf));		/* Clean the buffer	*/
            nCoordin = xmlValue[i]["DirectionCoordin"].size();
            strDirectionCoordin = "";
            pt.x = 0;
            pt.y = 0;
            //nCoordin必须等于2
            if(nCoordin==2)
            {
                for(j = 0; j < nCoordin; j++)
                {
                    //直接使用xml传过来的数据
                    x = xmlValue[i]["DirectionCoordin"][j]["x"];
                    y = xmlValue[i]["DirectionCoordin"][j]["y"];

                    pt.x += x;
                    pt.y += y;

                    if(j != nCoordin-1)
                    {
                        sprintf(buf,"%.3f,%.3f|", x, y);
                        Channel_info.chProp_direction.ptBegin.x = x; //起点
                        Channel_info.chProp_direction.ptBegin.y = y;
                    }
                    else						/* 避免最后分号	*/
                    {
                        sprintf(buf,"%.3f,%.3f", x, y);
                        Channel_info.chProp_direction.ptEnd.x = x;  //中点
                        Channel_info.chProp_direction.ptEnd.y = y;
                    }
                    strDirectionCoordin += buf;
                }
                /*车道方向线的中点*/
                //直接使用xml传过来的数据
                Channel_info.chProp_direction.point.x = pt.x*0.5;  //中点
                Channel_info.chProp_direction.point.y = pt.y*0.5;
            }

        }// if


#ifdef _DEBUG
        printf("AddRoadSettingInfo===i=%d ===nChannel = %d==\n\r", i, nChannel);
#endif

        list_channel_info.push_back(Channel_info);
    }//End of for i
    //fclose(fp);
    printf("db  nChannel=%d,strNewChannelways=%s\n",nChannel,strNewChannelways.c_str());
    //获取已经存在的车道列表
    CXmlParaUtil xml;

	//写入xml配置文件
    if( (g_nMultiPreSet == 0) && (g_nSwitchCamera == 0))
    {
        if(xml.AddRoadSettingInfoByList(list_channel_info, nChannel))
        {
            //更新参数设置文件，去除不存在的通道
            xml.UpdateParaSettingInfo(nChannel,strNewChannelways,0);
        }
    }
    else//如果存在多个预置位
    {
        int nCameraID = GetCameraID(nChannel);
        int nPreSet = 0;
        if(g_nMultiPreSet == 1)
        {
           nPreSet = GetPreSet(nChannel);
        }
        if(nPreSet >= 0)
        {
            if(xml.AddDeviceSettingInfo(list_channel_info,nCameraID,nPreSet))
            {
                //更新参数设置文件，去除不存在的通道
                xml.UpdateParaSettingInfo(nCameraID,strNewChannelways,nPreSet);
            }
        }
    }
    ////////////////////////////
	

    //记录日志
    LogNormal("用户更新车道信息!\r\n");


    //要求检测重新读取配置
    g_skpChannelCenter.ReloadDetect(nChannel);

#ifdef _DEBUG
    printf("==========after CSeekpaiDB::SaveRoadSettingInfo===================\n");
#endif

    return SRIP_OK;
}

//获取车道参数设置
String CSeekpaiDB::GetRoadParaMeterInfo(int nID)
{
    std::string response;
    CSkpRoadXmlValue xml;


#ifdef _DEBUG
    printf("=======Before===LoadAllRoadParameter=============\n\r");
#endif

    CXmlParaUtil roadParaxml;

    paraDetectList roadParamInlist;

    SRIP_CHANNEL_EXT sChannelExt; //车道扩展参数
    sChannelExt.uId = nID;
    roadParaxml.LoadAllRoadParameter(roadParamInlist,sChannelExt);


    /*SRIP_CHANNEL sChannel;
    sChannelExt.uTrafficStatTime = sChannel.uTrafficStatTime;
    sChannelExt.uEventDetectDelay = sChannel.uEventDetectDelay;
    sChannelExt.nShowTime = sChannel.nShowTime;
    sChannelExt.bRmShade = sChannel.bRmShade;
    sChannelExt.bRmTingle = sChannel.bRmTingle;
    sChannelExt.bSensitive = sChannel.bSensitive;
    sChannelExt.nHolizonMoveWeight = sChannel.nHolizonMoveWeight;
    sChannelExt.nVerticalMoveWeight = sChannel.nVerticalMoveWeight;
    sChannelExt.nZoomScale = sChannel.nZoomScale;
    sChannelExt.nZoomScale2 = sChannel.nZoomScale2;
    sChannelExt.nZoomScale3 = sChannel.nZoomScale3;*/

    response.append((char*)&sChannelExt, sizeof(SRIP_CHANNEL_EXT));

#ifdef _DEBUG
    printf("=======After===LoadAllRoadParameter=============\n\r");
#endif

    //通道个数
    int nModelsCount = roadParamInlist.size();

#ifdef _DEBUG
    printf("===After load the Model=========nModelsCount=%d=======\n\r", nModelsCount);
#endif

    paraDetectList::iterator it_b = roadParamInlist.begin();
    paraDetectList::iterator it_e = roadParamInlist.end();

    int i=0,j=0;
    while (it_b != it_e)
    {
        xml[i]["Channel"] = nID;    //通道号

        xml[i]["Index"] = it_b->nChannelID;   //车道的序号
        xml[i]["ModelID"] = it_b->nModelId;          //模板号

        if(it_b->nChannelID==0)
        {
            xml[i]["IsModel"] = 1;			//是否模板
        }
        else
        {
            xml[i]["IsModel"] = 0;			//是否模板
        }


        String strModelName(it_b->chModelName); //模板名称
        xml[i]["ModelName"] = strModelName; //模板名称

        xml[i]["IsNixing"] = it_b->m_bNixing;			//是否逆行监测
        xml[i]["NixingIgn"] = it_b->m_nNixingIgn;		//逆行事件忽略

        xml[i]["IsBianDao"] = it_b->m_bBianDao;		//是否变道监测
        xml[i]["BianDaoIgn"] = it_b->m_nBianDaoIgn;		//变道监测忽略
		xml[i]["ChangeDetectMod"] = it_b->m_nChangeDetectMod ; //变道报警类型

		xml[i]["IsPressLine"] = it_b->m_bPressYellowLine; //是否压线监测
        xml[i]["PressLineIgn"] = it_b->m_nPressYellowLineIgn;	//压线监测忽略

		xml[i]["StatQueueLen"] = it_b->m_bStatQueueLen;           //是否统计排队长度         // nahs
		xml[i]["VeloStartStat4QL"] = it_b->m_fVeloStartStat4QL;    //排队长度开始统计的速度(m/s)
		xml[i]["LengthStartStat4QL"] = it_b->m_fLengthStartStat4QL;    //排队长度开始统计的长度要求(m)///////////
		xml[i]["QLStatIntervalTime"] = it_b->m_fQLStatIntervalTime;  //排队长度统计间隔时间(s)

		xml[i]["IsPressLeadStreamLine"] = it_b->m_bPressLeadStreamLine; //是否压导流线监测
        xml[i]["PressLeadStreamLineIgn"] = it_b->m_nPressLeadStreamLineIgn;	//压导流线监测忽略

        xml[i]["IsStop"]= it_b->m_bStop;			//是否停车监测
        xml[i]["StopIgn"]= it_b->m_nStopIgn;			//停车监测忽略

        xml[i]["IsDusai"]= it_b->m_bDusai;			//是否堵塞监测
        xml[i]["DusaiIgn"]= it_b->m_nDusaiIgn;			//堵塞监测忽略

        for(j=0; j<5; j++)
        {
            xml[i]["DusaiSetup"][j]["QueueLength"] = it_b->m_nWayRate[j];//车道队列长度
            xml[i]["DusaiSetup"][j]["DusaiSpeed"] = (int)it_b->m_nDusaiSpeed[j];
        }

        xml[i]["IsOnlyOverSpeed"]= it_b->m_bOnlyOverSped;	//是否单独超速报警
        xml[i]["OnlyOverSpedIgn"]= it_b->m_nOnlyOverSpedIgn;	// 单独超速忽略
        xml[i]["OnlyOverSpedMax"]= (int)it_b->m_nOnlyOverSpedMax;	//单独超速限制

        xml[i]["IsDiuQi"] = it_b->m_bDiuQi;			// 是否丢弃物监测
        xml[i]["DiuQiIgn"] = it_b->m_nDiuQiIgn;			// 丢弃物监测忽略
        xml[i]["DiuQiIDam"] = it_b->m_nDiuQiIDam;		// 丢弃物的直径
        xml[i]["IsAvgSpeed"] = it_b->m_bAvgSpeed;		// 是否平均车速监测
        xml[i]["AvgSpeedIgn"] = it_b->m_nAvgSpeedIgn;		// 平均车速忽略
        xml[i]["AvgSpeedMax"] = (int)it_b->m_nAvgSpeedMax;		// 平均车速最大值
        xml[i]["AvgSpeedMin"] = (int)it_b->m_nAvgSpeedMin;		// 平均车速最小值

        xml[i]["IsPerson"] = it_b->m_bCross;			// 是否行人监测
        xml[i]["PersonIgn"] = it_b->m_nCrossIgn;		// 行人监测忽略

        xml[i]["FlowStat"] = it_b->m_bStatFlux;			// 是否监测车道流量
        xml[i]["AvgSpeedStat"] = it_b->m_bAvgSpeed;		// 是否监测平均车速
        xml[i]["ZylStat"] = it_b->m_bStatZyl;			// 是否监测平均车道占有率
        xml[i]["QueueStat"] = it_b->m_bStatQueue;		// 是否监测队列长度
        xml[i]["CtjjStat"] = it_b->m_bStatCtjj;			// 是否监测平均车头间距
        xml[i]["CarTypeStat"] = it_b->m_bStatCarType;		// 是否监测车辆分型

        xml[i]["BargeIn"] = it_b->m_bBargeIn;		// 是否白板检测

        xml[i]["IsFj"] = it_b->nChannelMod;		// 是否非机动车道

        xml[i]["BeyondMark"]= it_b->m_bBeyondMark;		// 是否混行检测

//		xml[i]["HxIgn"]= q.getIntFileds(33);		// 混行检测忽略

        xml[i]["AlertLevel"]= it_b->alert_level;

        xml[i]["IgnStop"]= it_b->m_nStopIgnJam;

        xml[i]["IgnStopNotJam"]= it_b->m_nStopIgnAlert;

        xml[i]["Jam"]= it_b->m_nDusaiIgnAlert;
        xml[i]["is_right_side"]= (int)it_b->is_right_side;

        xml[i]["AppearIgn"]= it_b->m_nAppearIgn;		// 出现检测忽略

        xml[i]["bHalfMaxSize"]= (int)it_b->is_half_maxsize;

        xml[i]["bPersonAgainst"]= (int)it_b->m_nAgainstDetectMod;

        xml[i]["bAppear"]= (int)it_b->m_bObjectAppear;

        xml[i]["bCross"]= (int)it_b->m_nCrossDetectMod;

        xml[i]["bCarAppear"]= (int)it_b->m_nObjeceDetectMod;


        xml[i]["Angle"]= it_b->m_nAngle;

        xml[i]["ForbidType"] = it_b->m_nForbidType;
        xml[i]["AllowBigBeginTime"] = it_b->m_nAllowBigBeginTime;
        xml[i]["AllowBigEndTime"] = it_b->m_nAllowBigEndTime;


        xml[i]["Crowd"] = it_b->m_bCrowd;
        xml[i]["PersonCount"] = it_b->m_nPersonCount;
        xml[i]["AreaPercent"] = it_b->m_nAreaPercent;
        xml[i]["PersonRun"] = it_b->m_bPersonRun;
        xml[i]["MaxRunSpeed"] = it_b->m_fMaxRunSpeed;

#ifdef _DEBUG
        printf("========i=%d===============\n\r", i);
#endif
        printf("1111111.nModelId=%d,roadParamIn.nChannelID=========%d,it_b->m_bCarAppear=%d,it_b->nChannelMod=%d\n",it_b->nModelId,it_b->nChannelID,it_b->m_bCarAppear,it_b->nChannelMod);

        i++;
        it_b++;
    }//End of while

    std::string xmlResponse = xml.toXml();

#ifdef _DEBUG
    printf("========xmlResponse.size()=%d===============\n\r", xmlResponse.size());
#endif

    response.append(xmlResponse.c_str(),xmlResponse.size());
    return response;
}


String CSeekpaiDB::SaveCommonRegion(const char *strConfigName,CSkpRoadXmlValue& xmlValue, CHANNEL_INFO& channel_info)
{
    String str,strTemp;
    char buf[1024];
    char *coordinBuf = new char[8192];
    REGION_PROPERTY region_property;
    CPoint32f pt;
    /* Fill in CHANNEL_INFO struct	*/
    int nNumber	= (int)xmlValue.size();

    /* Determine the type */
    int nType = 0;
    if(strcmp (strConfigName,"STOP_REGION") == 0)
    {
        nType = 1;
        strcpy(channel_info.stopRegion.chProperty.strName, "STOP_REGION");
        channel_info.stopRegion.chProperty.value.nValue = nNumber;
    }
    else if (strcmp(strConfigName,"PER_REGION") == 0)
    {
        nType = 2;
        strcpy(channel_info.personRegion.chProperty.strName, "PER_REGION");
        channel_info.personRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"DROP_REGION") == 0)
    {
        nType = 3;
        strcpy(channel_info.dropRegion.chProperty.strName, "DROP_REGION");
        channel_info.dropRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"AMOUNT_LINE") == 0)
    {
        nType = 4;
        strcpy(channel_info.AmountLine.chProperty.strName, "AMOUNT_LINE");
        channel_info.AmountLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"REF_LINE") == 0)
    {
        nType = 5;
        strcpy(channel_info.RefLine.chProperty.strName, "REF_LINE");
        channel_info.RefLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"TURNROAD_LINE") == 0)
    {
        nType = 6;
        strcpy(channel_info.TurnRoadLine.chProperty.strName, "TURNROAD_LINE");
        channel_info.TurnRoadLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"SKIP_REGION") == 0)
    {
        nType = 7;
        strcpy(channel_info.eliminateRegion.chProperty.strName, "SKIP_REGION");
        channel_info.eliminateRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"CARNUM_REGION") == 0)
    {
        nType = 8;
        strcpy(channel_info.carnumRegion.chProperty.strName, "CARNUM_REGION");
        channel_info.carnumRegion.chProperty.value.nValue = nNumber;
    }
    /////////////////////////////////////////线圈
    else if(strcmp(strConfigName,"FLOWBACK_REGION") == 0)
    {
        nType = 15;
        strcpy(channel_info.FlowBackRegion.chProperty.strName, "FLOWBACK_REGION");
        channel_info.FlowBackRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"FLOW_LINE") == 0)
    {
        nType = 16;
        strcpy(channel_info.FlowLine.chProperty.strName, "FLOW_LINE");
        channel_info.FlowLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"FLOWFRAMEGET_REGION") == 0)
    {
        nType = 17;
        strcpy(channel_info.FlowFramegetRegion.chProperty.strName, "FLOWFRAMEGET_REGION");
        channel_info.FlowFramegetRegion.chProperty.value.nValue = nNumber;
    }
    /////////////////////////////////////////
    /////////////////////////////////////////////闯红灯--begin
    else if(strcmp(strConfigName,"VIOLATION_REGION") == 0)
    {
        nType = 18;
        strcpy(channel_info.ViolationRegion.chProperty.strName, "VIOLATION_REGION");
        channel_info.ViolationRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"EVENT_REGION") == 0)
    {
        nType = 19;
        strcpy(channel_info.EventRegion.chProperty.strName, "EVENT_REGION");
        channel_info.EventRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"TRAFFIC_SIGNAL_REGION") == 0)
    {
        nType = 20;
        strcpy(channel_info.TrafficSignalRegion.chProperty.strName, "TRAFFIC_SIGNAL_REGION");
        channel_info.TrafficSignalRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"STOP_LINE") == 0)
    {
        nType = 21;
        strcpy(channel_info.StopLine.chProperty.strName, "STOP_LINE");
        channel_info.StopLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"STRAIGHT_LINE") == 0)
    {
        nType = 22;
        strcpy(channel_info.StraightLine.chProperty.strName, "STRAIGHT_LINE");
        channel_info.StraightLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"TURN_LEFT_LINE") == 0)
    {
        nType = 23;
        strcpy(channel_info.TurnLeftLine.chProperty.strName, "TURN_LEFT_LINE");
        channel_info.TurnLeftLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"TURN_RIGHT_LINE") == 0)
    {
        nType = 24;
        strcpy(channel_info.TurnRightLine.chProperty.strName, "TURN_RIGHT_LINE");
        channel_info.TurnRightLine.chProperty.value.nValue = nNumber;
    }
    /////////////////////////////////////////////闯红灯--end
    else if(strcmp(strConfigName,"STAB_BACK_REGION") == 0)
    {
        nType = 25;
        strcpy(channel_info.StabBackRegion.chProperty.strName, "STAB_BACK_REGION");
        channel_info.StabBackRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"SYNCH_LEFT_REGION") == 0)
    {
        nType = 26;
        strcpy(channel_info.SynchLeftRegion.chProperty.strName, "SYNCH_LEFT_REGION");
        channel_info.SynchLeftRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"SYNCH_RIGHT_REGION") == 0)
    {
        nType = 27;
        strcpy(channel_info.SynchRightRegion.chProperty.strName, "SYNCH_RIGHT_REGION");
        channel_info.SynchRightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"CARDNUMBER_REGION") == 0)
    {
        nType = 28;
        strcpy(channel_info.CardnumberRegion.chProperty.strName, "CARDNUMBER_REGION");
        channel_info.CardnumberRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"LOOP_LINE") == 0)
    {
        nType = 29;
        strcpy(channel_info.LoopRegion.chProperty.strName, "LOOP_LINE");
        channel_info.LoopRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"BARGEIN_REGION") == 0)
    {
        nType = 30;
        strcpy(channel_info.BargeInRegion.chProperty.strName, "BARGEIN_REGION");
        channel_info.BargeInRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"BEYONDMARK_REGION") == 0)
    {
        nType = 31;
        strcpy(channel_info.BeyondMarkRegion.chProperty.strName, "BEYONDMARK_REGION");
        channel_info.BeyondMarkRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"RADAR_REGION") == 0)
    {
        nType = 32;
        strcpy(channel_info.RadarRegion.chProperty.strName, "RADAR_REGION");
        channel_info.RadarRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"YELLOW_LINE") == 0)
    {
        nType = 33;
        strcpy(channel_info.YellowLine.chProperty.strName, "YELLOW_LINE");
        channel_info.YellowLine.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"RedLightRegion") == 0)
    {
        nType = 34;
        strcpy(channel_info.RedLightRegion.chProperty.strName, "RedLightRegion");
        channel_info.RedLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"GreenLightRegion") == 0)
    {
        nType = 35;
        strcpy(channel_info.GreenLightRegion.chProperty.strName, "GreenLightRegion");
        channel_info.GreenLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"LeftLightRegion") == 0)
    {
        nType = 36;
        strcpy(channel_info.LeftLightRegion.chProperty.strName, "LeftLightRegion");
        channel_info.LeftLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"RightLightRegion") == 0)
    {
        nType = 37;
        strcpy(channel_info.RightLightRegion.chProperty.strName, "RightLightRegion");
        channel_info.RightLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"StraightLightRegion") == 0)
    {
        nType = 38;
        strcpy(channel_info.StraightLightRegion.chProperty.strName, "StraightLightRegion");
        channel_info.StraightLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"TurnAroundLightRegion") == 0)
    {
        nType = 39;
        strcpy(channel_info.TurnAroundLightRegion.chProperty.strName, "TurnAroundLightRegion");
        channel_info.TurnAroundLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"LeftRedLightRegion") == 0)
    {
        nType = 40;
        strcpy(channel_info.LeftRedLightRegion.chProperty.strName, "LeftRedLightRegion");
        channel_info.LeftRedLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"LeftGreenLightRegion") == 0)
    {
        nType = 41;
        strcpy(channel_info.LeftGreenLightRegion.chProperty.strName, "LeftGreenLightRegion");
        channel_info.LeftGreenLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"RightRedLightRegion") == 0)
    {
        nType = 42;
        strcpy(channel_info.RightRedLightRegion.chProperty.strName, "RightRedLightRegion");
        channel_info.RightRedLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"RightGreenLightRegion") == 0)
    {
        nType = 43;
        strcpy(channel_info.RightGreenLightRegion.chProperty.strName, "RightGreenLightRegion");
        channel_info.RightGreenLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"StraightRedLightRegion") == 0)
    {
        nType = 44;
        strcpy(channel_info.StraightRedLightRegion.chProperty.strName, "StraightRedLightRegion");
        channel_info.StraightRedLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"StraightGreenLightRegion") == 0)
    {
        nType = 45;
        strcpy(channel_info.StraightGreenLightRegion.chProperty.strName, "StraightGreenLightRegion");
        channel_info.StraightGreenLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"TurnAroundRedLightRegion") == 0)
    {
        nType = 46;
        strcpy(channel_info.TurnAroundRedLightRegion.chProperty.strName, "TurnAroundRedLightRegion");
        channel_info.TurnAroundRedLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"TurnAroundGreenLightRegion") == 0)
    {
        nType = 47;
        strcpy(channel_info.TurnAroundGreenLightRegion.chProperty.strName, "TurnAroundGreenLightRegion");
        channel_info.TurnAroundGreenLightRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"LINE_STOP") == 0)
    {
        nType = 48;
        strcpy(channel_info.LineStop.chProperty.strName, "LineStop");
        channel_info.LineStop.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"LineStraight") == 0)
    {
        nType = 49;
        strcpy(channel_info.LineStraight.chProperty.strName, "LineStraight");
        channel_info.LineStraight.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"VirtualLoop") == 0)
    {
        nType = 50;
        strcpy(channel_info.VirtualLoopRegion.chProperty.strName, "VirtualLoop");
        channel_info.VirtualLoopRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"RemotePerson") == 0)
    {
        nType = 51;
        strcpy(channel_info.RemotePersonRegion.chProperty.strName, "RemotePerson");
        channel_info.RemotePersonRegion.chProperty.value.nValue = nNumber;
    }
    else if(strcmp(strConfigName,"LocalPerson") == 0)
    {
        nType = 52;
        strcpy(channel_info.LocalPersonRegion.chProperty.strName, "LocalPerson");
        channel_info.LocalPersonRegion.chProperty.value.nValue = nNumber;
    }
	else if(strcmp(strConfigName,"DENSITY_REGION") == 0)
	{
		nType = 53;
		strcpy(channel_info.DensityRegion.chProperty.strName, "DENSITY_REGION");
		channel_info.DensityRegion.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"GETPHOTO_REGION") == 0)
	{
		nType = 54;
		strcpy(channel_info.GetPhotoRegion.chProperty.strName, "GETPHOTO_REGION");
		channel_info.GetPhotoRegion.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"WHITE_LINE") == 0)
	{
		nType = 55;
		strcpy(channel_info.WhiteLine.chProperty.strName, "WHITE_LINE");
		channel_info.WhiteLine.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"HOLD_FORE_FIRST_LINE") == 0)
	{
		nType = 56;
		strcpy(channel_info.HoldForeLineFirst.chProperty.strName, "HOLD_FORE_FIRST_LINE");
		channel_info.HoldForeLineFirst.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"HOLD_FORE_SECOND_LINE") == 0)
	{
		nType = 57;
		strcpy(channel_info.HoldForeLineSecond.chProperty.strName, "HOLD_FORE_SECOND_LINE");
		channel_info.HoldForeLineSecond.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"HOLD_STOP_FIRST_LINE") == 0)
	{
		nType = 58;
		strcpy(channel_info.HoldStopLineFirst.chProperty.strName, "HOLD_STOP_FIRST_LINE");
		channel_info.HoldStopLineFirst.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"HOLD_STOP_SECOND_LINE") == 0)
	{
		nType = 59;
		strcpy(channel_info.HoldStopLineSecond.chProperty.strName, "HOLD_STOP_SECOND_LINE");
		channel_info.HoldStopLineSecond.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"LEAD_STREAM_LINE") == 0)
    {
        nType = 60;
        strcpy(channel_info.LeadStreamLine.chProperty.strName, "LEAD_STREAM_LINE");
        channel_info.LeadStreamLine.chProperty.value.nValue = nNumber;
    }
	else if(strcmp(strConfigName,"NO_PARKING_REGION") == 0)
	{
		nType = 65; //65 DETECT_RESULT_NO_PARKING;
		strcpy(channel_info.YelGridRgn.chProperty.strName, "NO_PARKING_REGION");
		channel_info.YelGridRgn.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"VIOLATION_FIRST_LINE") == 0)
	{
		nType = 66; //66 VIOLATION_FIRST_LINE;
		strcpy(channel_info.ViolationFirstLine.chProperty.strName, "VIOLATION_FIRST_LINE");
		channel_info.ViolationFirstLine.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"VIOLATION_SECOND_LINE") == 0)
	{
		nType = 67; //67 VIOLATION_SECOND_LINE;
		strcpy(channel_info.ViolationSecondLine.chProperty.strName, "VIOLATION_SECOND_LINE");
		channel_info.ViolationSecondLine.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"RIGHT_FIRST_LINE") == 0)
	{
		nType = 68; //68 RIGHT_FIRST_LINE;
		strcpy(channel_info.RightFirstLine.chProperty.strName, "RIGHT_FIRST_LINE");
		channel_info.RightFirstLine.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"LEFT_FIRST_LINE") == 0)
	{
		nType = 69; //69 LEFT_FIRST_LINE;
		strcpy(channel_info.LeftFirstLine.chProperty.strName, "LEFT_FIRST_LINE");
		channel_info.LeftFirstLine.chProperty.value.nValue = nNumber;
	}
	else if(strcmp(strConfigName,"FORE_FIRST_LINE") == 0)
	{
		nType = 70; //70 FORE_FIRST_LINE;
		strcpy(channel_info.ForeFirstLine.chProperty.strName, "FORE_FIRST_LINE");
		channel_info.ForeFirstLine.chProperty.value.nValue = nNumber;
	}


    for(int j = 0; j < nNumber; j++)
    {
        region_property.Clear();
        memset(coordinBuf, 0, sizeof(coordinBuf));
        int nCoordin = xmlValue[j]["Coordin"].size();
        strTemp = "";

        region_property.nValue = nCoordin;

        for(int k = 0; k < nCoordin; k++)
        {
            //直接使用xml传过来的数据
            pt.x = xmlValue[j]["Coordin"][k]["x"];
            pt.y = xmlValue[j]["Coordin"][k]["y"];
            region_property.listPt.push_back(pt);

            if(k != nCoordin-1)
            {
                sprintf(buf,"%.3f,%.3f|", pt.x, pt.y);
            }
            else		 /*避免最后分号*/
            {
                sprintf(buf,"%.3f,%.3f", pt.x, pt.y);
            }
            strTemp += buf;
        }

        sprintf(coordinBuf,"(%s)",strTemp.c_str());
        str += coordinBuf;

        /* Insert struct */
        /* Fill in CHANNEL_INFO struct	*/
        switch(nType)
        {
        case 1:			/* Stop region	*/
        {
            channel_info.stopRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 2:			/* Person region */
        {
            channel_info.personRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 3:			/* Drop region	*/
        {
            channel_info.dropRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 4:			/* Amount line	*/
        {
			//add by GaoXiang
			if (xmlValue[j]["direction"].size() > 0)
			{
				CPoint32f begin, end;
				begin.x = xmlValue[j]["direction"]["x1"];
				begin.y = xmlValue[j]["direction"]["y1"];
				end.x = xmlValue[j]["direction"]["x2"];
				end.y = xmlValue[j]["direction"]["y2"];

				region_property.directionListPt.push_back(begin);
				region_property.directionListPt.push_back(end);
			}
            channel_info.AmountLine.listRegionProp.push_back(region_property);
            break;
        }
        case 5:			/* Reference line*/
        {
            channel_info.RefLine.listRegionProp.push_back(region_property);
            break;
        }
        case 6:			/* TurnRoad line*/
        {
			if (xmlValue[j]["direction"].size() > 0)
			{
				CPoint32f begin, end;
				begin.x = xmlValue[j]["direction"]["x1"];
				begin.y = xmlValue[j]["direction"]["y1"];
				end.x = xmlValue[j]["direction"]["x2"];
				end.y = xmlValue[j]["direction"]["y2"];

				region_property.directionListPt.push_back(begin);
				region_property.directionListPt.push_back(end);
			}
            channel_info.TurnRoadLine.listRegionProp.push_back(region_property);
            break;
        }
        case 7:			/*skip region		*/
        {
            channel_info.eliminateRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 8:			/*carnum region		*/
        {
            channel_info.carnumRegion.listRegionProp.push_back(region_property);
            break;
        }
        /////////////////////////////////////////线圈
        case 15:			/*flowBack region		*/
        {
            channel_info.FlowBackRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 16:			/*flowLine      		*/
        {
            channel_info.FlowLine.listRegionProp.push_back(region_property);
            break;
        }
        case 17:			/*flowFreameget region	*/
        {
            channel_info.FlowFramegetRegion.listRegionProp.push_back(region_property);
            break;
        }
        /////////////////////////////////////////
        /////////////////////////////////////////////闯红灯--begin
        case 18:			/*Violation region		*/
        {
            channel_info.ViolationRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 19:			/*Event region		*/
        {
            channel_info.EventRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 20:			/*TrafficSignal region		*/
        {
            channel_info.TrafficSignalRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 21:			/*StopLine */
        {
            channel_info.StopLine.listRegionProp.push_back(region_property);
            break;
        }
        case 22:			/*StraightLine */
        {
            channel_info.StraightLine.listRegionProp.push_back(region_property);
            break;
        }
        case 23:			/*TurnLeftLine */
        {
            channel_info.TurnLeftLine.listRegionProp.push_back(region_property);
            break;
        }
        case 24:			/*TurnRightLine */
        {
            channel_info.TurnRightLine.listRegionProp.push_back(region_property);
            break;
        }
        /////////////////////////////////////////////闯红灯--end
        case 25:			/*StabBackArea */
        {
            channel_info.StabBackRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 26:			/*SynchLeftArea */
        {
            channel_info.SynchLeftRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 27:			/*SynchRightArea */
        {
            channel_info.SynchRightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 28:			/*CardnumberArea */
        {
            channel_info.CardnumberRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 29:			/*LoopLine */
        {
            channel_info.LoopRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 30:			/*BargeInRegion */
        { 
			//add by GaoXiang
			if (xmlValue[j]["direction"].size() > 0)
			{
				CPoint32f begin, end;
				begin.x = xmlValue[j]["direction"]["x1"];
				begin.y = xmlValue[j]["direction"]["y1"];
				end.x = xmlValue[j]["direction"]["x2"];
				end.y = xmlValue[j]["direction"]["y2"];

				region_property.directionListPt.push_back(begin);
				region_property.directionListPt.push_back(end);
			}
			channel_info.BargeInRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 31:			/*BeyondMarkRegion */
        {
			//add by GaoXiang
			if (xmlValue[j]["direction"].size() > 0)
			{
				CPoint32f begin, end;
				begin.x = xmlValue[j]["direction"]["x1"];
				begin.y = xmlValue[j]["direction"]["y1"];
				end.x = xmlValue[j]["direction"]["x2"];
				end.y = xmlValue[j]["direction"]["y2"];

				region_property.directionListPt.push_back(begin);
				region_property.directionListPt.push_back(end);
			}
			channel_info.BeyondMarkRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 32:			/*RadarRegion */
        {
            channel_info.RadarRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 33:			/*YellowLine */
        {
            channel_info.YellowLine.listRegionProp.push_back(region_property);
            break;
        }
        case 34:			/*RedLightRegion */
        {
            channel_info.RedLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 35:			/*GreenLightRegion */
        {
            channel_info.GreenLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 36:			/*LeftLightRegion */
        {
            channel_info.LeftLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 37:			/*RightLightRegion */
        {
            channel_info.RightLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 38:			/*StraightLightRegion */
        {
            channel_info.StraightLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 39:			/*TurnAroundLightRegion */
        {
            channel_info.TurnAroundLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 40:			/*LeftRedLightRegion */
        {
            channel_info.LeftRedLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 41:			/*LeftGreenLightRegion */
        {
            channel_info.LeftGreenLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 42:			/*RightRedLightRegion */
        {
            channel_info.RightRedLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 43:			/*RightGreenLightRegion */
        {
            channel_info.RightGreenLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 44:			/*StraightRedLightRegion */
        {
            channel_info.StraightRedLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 45:			/*StraightGreenLightRegion */
        {
            channel_info.StraightGreenLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 46:			/*TurnAroundRedLightRegion */
        {
            channel_info.TurnAroundRedLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 47:			/*TurnAroundGreenLightRegion */
        {
            channel_info.TurnAroundGreenLightRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 48:			/*LineStop */
        {
            channel_info.LineStop.listRegionProp.push_back(region_property);
            break;
        }
        case 49:			/*LineStraight */
        {
            channel_info.LineStraight.listRegionProp.push_back(region_property);
            break;
        }
        case 50:			/*VirtualLoopRegion */
        {
            channel_info.VirtualLoopRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 51:			/*RemotePersonRegion */
        {
            channel_info.RemotePersonRegion.listRegionProp.push_back(region_property);
            break;
        }
        case 52:			/*LocalPersonRegion */
        {
            channel_info.LocalPersonRegion.listRegionProp.push_back(region_property);
            break;
        }
		case 53:			/*DensityRegion */
		{
			channel_info.DensityRegion.listRegionProp.push_back(region_property);
			break;
		}
		case 54:			/*GetPhotoRegion */
		{
			channel_info.GetPhotoRegion.listRegionProp.push_back(region_property);
			break;
		}
		case 55:			/*WhiteLine */
		{
			channel_info.WhiteLine.listRegionProp.push_back(region_property);
			break;
		}
		case 56:			/*HoldForeLineFirst */
		{
			channel_info.HoldForeLineFirst.listRegionProp.push_back(region_property);
			break;
		}
		case 57:			/*HoldForeLineSecond */
		{
			channel_info.HoldForeLineSecond.listRegionProp.push_back(region_property);
			break;
		}
		case 58:			/*HoldStopLineFirst */
		{
			channel_info.HoldStopLineFirst.listRegionProp.push_back(region_property);
			break;
		}
		case 59:			/*HoldStopLineSecond */
		{
			channel_info.HoldStopLineSecond.listRegionProp.push_back(region_property);
			break;
		}
		case 60:			/*LeadStreamLine */
        {
            channel_info.LeadStreamLine.listRegionProp.push_back(region_property);
            break;
        }
		case 65:			/*YelGrid Region */
		{
			channel_info.YelGridRgn.listRegionProp.push_back(region_property);
			break;
		}
		case 66:			/*ViolationFirstLine */
		{
			channel_info.ViolationFirstLine.listRegionProp.push_back(region_property);
			break;
		}
		case 67:			/*ViolationSecondLine */
		{
			channel_info.ViolationSecondLine.listRegionProp.push_back(region_property);
			break;
		}
		case 68:			/*RightFirstLine */
		{
			channel_info.RightFirstLine.listRegionProp.push_back(region_property);
			break;
		}
		case 69:			/*LeftFirstLine */
		{
			channel_info.LeftFirstLine.listRegionProp.push_back(region_property);
			break;
		}
		case 70:			/*ForeFirstLine */
		{
			channel_info.ForeFirstLine.listRegionProp.push_back(region_property);
			break;
		}
        default:
            break;
        }
    }

    /* Release heap memory */
    delete []coordinBuf;
    coordinBuf = NULL;

    return str;
}


/* 清空车道信息 */
int CSeekpaiDB::DeleteRoad(int nChannel)
{
    CXmlParaUtil xml;
    xml.DeleteRoadSettingInfo(nChannel);

    //记录日志
    LogNormal("用户删除车道信息!\r\n");

    return SRIP_OK;
}


//备份数据库
int CSeekpaiDB::BackupDB(String strDBName, String strBackupPath)
{
    //Execuate shell command, back current data
    //Make sure the shell script is in the same directory as this file(CSeekpaiDB.h)
    system(g_strBackExeFile.c_str());

    return SRIP_OK;
}

// 图表查询
String CSeekpaiDB::GetChartQuery(int nChannelId,int nRoadIndex,String strDate, int dateType, int queryType, int typeValue)
{
    std::string response;
    char buf[8192];
    char buffer[255];
    char temp[8];
    int year, month, day;
    int countNumber = 0;

    // strDate格式: 2008-07-09
    switch (dateType)
    {
    case 2:		// 按日查询
        day = atoi(strDate.substr(8,2).c_str());
    case 1:		// 按月查询
        month = atoi(strDate.substr(5,2).c_str());
    case 0:		// 按年查询
        // 取前4个字符
        year = atoi(strDate.substr(0,4).c_str());
        break;
    }

    memset(buf, 0, sizeof(buf));
    memset(buffer,0, sizeof(buffer));
    memset(temp,0,sizeof(temp));
    switch (queryType)
    {
    case 1:		// 事件
    {
        switch (dateType)
        {
        case 0:		// 按年查询
        {
            countNumber = 12;
            for(int i = 1; i <= countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-01 00:00:00' and BEGIN_TIME <= '%4d-%02d-31 23:59:59' and KIND=%d;", i,year, i, year, i, typeValue);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-01 00:00:00' and BEGIN_TIME <= '%4d-%02d-31 23:59:59' and KIND=%d and CHANNEL=%d;", i,year, i, year, i, typeValue,nChannelId);
                    }
                    else
                    {
                        sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-01 00:00:00' and BEGIN_TIME <= '%4d-%02d-31 23:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i,year, i, year, i, typeValue,nChannelId,nRoadIndex);
                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        case 1:		// 按月查询
        {
            countNumber = 31;
            for(int i = 1; i <= countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-%02d 00:00:00' and BEGIN_TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d;", i,year, month, i, year, month, i, typeValue);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-%02d 00:00:00' and BEGIN_TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d and CHANNEL=%d;", i,year, month, i, year, month, i, typeValue,nChannelId);
                    }
                    else
                    {
                        sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-%02d 00:00:00' and BEGIN_TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i,year, month, i, year, month, i, typeValue,nChannelId,nRoadIndex);
                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        case 2:		// 按日查询
        {
            countNumber = 24;
            for(int i = 0; i < countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-%02d %02d:00:00' and BEGIN_TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d;", i+1, year, month, day, i, year, month, day, i, typeValue);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-%02d %02d:00:00' and BEGIN_TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d and CHANNEL=%d;", i+1, year, month, day, i, year, month, day, i, typeValue,nChannelId);
                    }
                    else
                    {
                        sprintf(buffer, "select @t%d:=count(*) from TRAFFIC_EVENT_INFO where BEGIN_TIME >= '%4d-%02d-%02d %02d:00:00' and BEGIN_TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i+1, year, month, day, i, year, month, day, i, typeValue,nChannelId,nRoadIndex);

                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        }
    }
    break;
    case 2:		// 统计
    {
        typeValue += 99;
        switch (dateType)
        {
        case 0:		// 按年查询
        {
            countNumber = 12;
            for(int i = 1; i <= countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    if(typeValue==100)
                        sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and KIND=%d;", i,year, i, year, i,typeValue);
                    else
                        sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and KIND=%d;", i,year, i, year, i,typeValue);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and KIND=%d and CHANNEL=%d;", i,year, i, year, i, typeValue,nChannelId);
                        else
                            sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and KIND=%d and CHANNEL=%d;", i,year, i, year, i, typeValue,nChannelId);
                    }
                    else
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i,year, i, year, i, typeValue,nChannelId,nRoadIndex);
                        else
                            sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i,year, i, year, i, typeValue,nChannelId,nRoadIndex);
                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        case 1:		// 按月查询
        {
            countNumber = 31;
            for(int i = 1; i <= countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    if(typeValue==100)
                        sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d;", i,year, month, i, year, month, i, typeValue);
                    else
                        sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d;", i,year, month, i, year, month, i, typeValue);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d and CHANNEL=%d;", i,year, month, i, year, month, i, typeValue,nChannelId);
                        else
                            sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d and CHANNEL=%d;", i,year, month, i, year, month, i, typeValue,nChannelId);
                    }
                    else
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i,year, month, i, year, month, i, typeValue,nChannelId,nRoadIndex);
                        else
                            sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i,year, month, i, year, month, i, typeValue,nChannelId,nRoadIndex);
                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        case 2:		// 按日查询
        {
            countNumber = 24;
            for(int i = 0; i < countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    if(typeValue==100)
                        sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d;", i+1, year, month, day, i, year, month, day, i, typeValue);
                    else
                        sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d;", i+1, year, month, day, i, year, month, day, i, typeValue);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d and CHANNEL=%d;", i+1, year, month, day, i, year, month, day, i, typeValue,nChannelId);
                        else
                            sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d and CHANNEL=%d;", i+1, year, month, day, i, year, month, day, i, typeValue,nChannelId);
                    }
                    else
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=sum(VALUE&(0xffff)) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i+1, year, month, day, i, year, month, day, i, typeValue,nChannelId,nRoadIndex);
                        else
                            sprintf(buffer, "select @t%d:=avg(VALUE) from TRAFFIC_STATISTIC_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and KIND=%d and CHANNEL=%d and ROAD = %d;", i+1, year, month, day, i, year, month, day, i, typeValue,nChannelId,nRoadIndex);
                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        }
    }
    break;
    case 3:	//车牌统计
    {
        typeValue += 99;
        switch (dateType)
        {
        case 0:		// 按年查询
        {
            countNumber = 12;
            for(int i = 1; i <= countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    if(typeValue==100)
                        sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and NUMBER != '*******';", i,year, i, year, i);
                    else  if(typeValue==101)
                        sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and NUMBER != '*******';", i,year, i, year, i);
                    else
                        sprintf(buffer, "select @t%d:=0;",i+1);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and CHANNEL=%d and NUMBER != '*******';", i,year, i, year, i,nChannelId);
                        else if(typeValue==101)
                            sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and CHANNEL=%d and NUMBER != '*******';", i,year, i, year, i,nChannelId);
                        else
                            sprintf(buffer, "select @t%d:=0;",i+1);
                    }
                    else
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and CHANNEL=%d and ROAD = %d and NUMBER != '*******';", i,year, i, year, i,nChannelId,nRoadIndex);
                        else if(typeValue==101)
                            sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-01 00:00:00' and TIME <= '%4d-%02d-31 23:59:59' and CHANNEL=%d and ROAD = %d and NUMBER != '*******';", i,year, i, year, i,nChannelId,nRoadIndex);
                        else
                            sprintf(buffer, "select @t%d:=0;",i+1);
                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        case 1:		// 按月查询
        {
            countNumber = 31;
            for(int i = 1; i <= countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    if(typeValue==100)
                        sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and NUMBER != '*******';", i,year, month, i, year, month, i);
                    else  if(typeValue==101)
                        sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and NUMBER != '*******';", i,year, month, i, year, month, i);
                    else
                        sprintf(buffer, "select @t%d:=0;",i+1);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and CHANNEL=%d and NUMBER != '*******';", i,year, month, i, year, month, i,nChannelId);
                        else if(typeValue==101)
                            sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and CHANNEL=%d and NUMBER != '*******';", i,year, month, i, year, month, i,nChannelId);
                        else
                            sprintf(buffer, "select @t%d:=0;",i+1);
                    }
                    else
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and CHANNEL=%d and ROAD = %d and NUMBER != '*******';", i,year, month, i, year, month, i,nChannelId,nRoadIndex);
                        else if(typeValue==101)
                            sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d 00:00:00' and TIME <= '%4d-%02d-%02d 23:59:59' and CHANNEL=%d and ROAD = %d and NUMBER != '*******';", i,year, month, i, year, month, i,nChannelId,nRoadIndex);
                        else
                            sprintf(buffer, "select @t%d:=0;",i+1);
                    }
                }
                strcat(buf, buffer);
            }

            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        case 2:		// 按日查询
        {
            countNumber = 24;
            for(int i = 0; i < countNumber; i++ )
            {
                if(nChannelId==0)
                {
                    if(typeValue==100)
                        sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and NUMBER != '*******';", i+1, year, month, day, i, year, month, day, i);
                    else  if(typeValue==101)
                        sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and NUMBER != '*******';", i+1, year, month, day, i, year, month, day, i);
                    else
                        sprintf(buffer, "select @t%d:=0;",i+1);
                }
                else
                {
                    if(nRoadIndex==0)
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and CHANNEL=%d and NUMBER != '*******';", i+1, year, month, day, i, year, month, day, i,nChannelId);
                        else if(typeValue==101)
                            sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and CHANNEL=%d and NUMBER != '*******';", i+1, year, month, day, i, year, month, day, i,nChannelId);
                        else
                            sprintf(buffer, "select @t%d:=0;",i+1);
                    }
                    else
                    {
                        if(typeValue==100)
                            sprintf(buffer, "select @t%d:=count(*) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and CHANNEL=%d and ROAD = %d and NUMBER != '*******';", i+1, year, month, day, i, year, month, day, i,nChannelId,nRoadIndex);
                        else if(typeValue==101)
                            sprintf(buffer, "select @t%d:=avg(speed) from NUMBER_PLATE_INFO where TIME >= '%4d-%02d-%02d %02d:00:00' and TIME <= '%4d-%02d-%02d %02d:59:59' and CHANNEL=%d and ROAD = %d and NUMBER != '*******';", i+1, year, month, day, i, year, month, day, i,nChannelId,nRoadIndex);
                        else
                            sprintf(buffer, "select @t%d:=0;",i+1);
                    }
                }
                strcat(buf, buffer);
            }
            strcat(buf, "select @t1");
            for (int j =2; j <= countNumber; j++)
            {
                sprintf(temp, ",@t%d", j);
                strcat(buf, temp);
            }
        }
        break;
        }
    }
    break;
    }

    String sql(buf);


    MysqlQuery q = execMultiQuery(sql);

    //printf("Chart query  = %s,num=%d\r\n", sql.c_str(),q.numFileds());

    if( !q.eof() )
    {
        for(int i = 0; i < countNumber; i++)
        {
            int nCount = (int)q.getIntFileds(i);
            response.append((char*)&nCount,sizeof(nCount));
            printf("count%d=%d\r\n",i,(int) nCount);
        }
    }
    q.finalize();

    return response;
}

// 读取文件名称，根据ID
String CSeekpaiDB::GetSrcFileByID(int nID)
{
    char buf[BUFF_SIZE]= {0};
    sprintf(buf,"SELECT CHAN_SRC_FILE from CHAN_INFO where CHAN_ID = %d",nID);
    String sql(buf);
    String str = getStringFiled(sql);

    return str;
}

//区间测速：删除已经存在的记录
int CSeekpaiDB::RegionSpeedDeleteOldRecord(string strPath)
{
	char buf[BUFF_SIZE]= {0};
	sprintf(buf,"Delete from NUMBER_PLATE_INFO_RECV where PICPATH = '%s'", strPath.c_str());

	String sqlPlate(buf);
	if(execSQL(sqlPlate)!=0)
	{
		return SRIP_ERROR_USER_FAILE;
	}

	return SRIP_OK;

}

//删除某段时间以前的所有记录
int CSeekpaiDB::DeleteRecordByTime(string& strTime,int nType)
{
	char buf[BUFF_SIZE]= {0};
	if (nType == 0)
	{
		sprintf(buf,"Delete from NUMBER_PLATE_INFO where TIME <= '%s'", strTime.c_str());
	}
	else if (nType == 1)
	{
		sprintf(buf,"Delete from VIDEO_FILE_INFO where BEGIN_TIME <= '%s'", strTime.c_str());
	}
	String sqlPlate(buf);
	if(execSQL(sqlPlate)!=0)
	{
		return SRIP_ERROR_USER_FAILE;
	}
	return SRIP_OK;
}

//电科磁盘管理：根据卡口，违章，录像的路径删除对应的记录
int CSeekpaiDB::DianKeDeleteOldRecord(string& strPath,int nType) //nType = 0:卡口，违章。1：录像
{
	char buf[BUFF_SIZE]= {0};
	if (nType == 0)
	{
		sprintf(buf,"Delete from NUMBER_PLATE_INFO where PICPATH = '%s'", strPath.c_str());
	}
	else if (nType == 1)
	{
		sprintf(buf,"Delete from VIDEO_FILE_INFO where VIDEOPATH = '%s'", strPath.c_str());
	}
	String sqlPlate(buf);
	if(execSQL(sqlPlate)!=0)
	{
		return SRIP_ERROR_USER_FAILE;
	}
	cerr<<"delete Record "<<strPath.c_str()<<" success"<<endl;
	return SRIP_OK;

} 

//////////////////////////////////////
//删除已经存在的记录 参数:strPath -文件路径, bCleanDisk -是否在做磁盘管理, bVideo -是否录像
int CSeekpaiDB::DeleteOldRecord(string strPath,bool bCleanDisk,bool bVideo)
{
	char buf[BUFF_SIZE]= {0};

	if(bVideo)
	{
		

		sprintf(buf,"Delete from NUMBER_PLATE_INFO where VIDEOPATH = '%s'",strPath.c_str());
		String sqlPlate(buf);

		if(execSQL(sqlPlate)!=0)
		{
			return SRIP_ERROR_USER_FAILE;
		}

		sprintf(buf,"Delete from VIDEO_FILE_INFO where PATH = '%s'",strPath.c_str());
		String sqlVideo(buf);

		if(execSQL(sqlVideo)!=0)
		{
			return SRIP_ERROR_USER_FAILE;
		}
	}
	else
	{
		if (bCleanDisk)
		{
			sprintf(buf,"select * from NUMBER_PLATE_INFO where PICPATH = '%s'",strPath.c_str());
			String sqlSel(buf);
			MysqlQuery q = execQuery(sqlSel);

			UINT32 id = 0;
			while(!q.eof())
			{
				id = q.getUnIntFileds("ID");
				q.nextRow();
			}
			q.finalize();

		
			//删除之前的所有车牌记录
			sprintf(buf,"Delete from NUMBER_PLATE_INFO where ID <= %d", id);
		}
		else
		{
			sprintf(buf,"Delete from NUMBER_PLATE_INFO where PICPATH = '%s'", strPath.c_str());
		}
		String sqlPlate(buf);
		if(execSQL(sqlPlate)!=0)
		{
			return SRIP_ERROR_USER_FAILE;
		}
		else
		{
			//printf("DELETE plate %s ok\n",strPath.c_str());
		}
	}

	return SRIP_OK;
}

//删除一周前的数据库内容
int CSeekpaiDB::DeleteOldContent()
{
    unsigned int uCurrentTime = GetTimeStamp();
    unsigned int uDateTime = g_uDiskDay*DATE_TIME;
    unsigned int uTime  = uCurrentTime - uDateTime;

    String strTime = GetTime(uTime);
    char buf[BUFF_SIZE]= {0};

    printf("=================strTime=%s\n",strTime.c_str());

    memset(buf,0,BUFF_SIZE);
    sprintf(buf,"Delete from TRAFFIC_STATISTIC_INFO where TIME <= '%s'",strTime.c_str());
    String sql4(buf);

    if(execSQL(sql4)!=0)
    {
        printf("Delete TRAFFIC_STATISTIC_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    memset(buf,0,BUFF_SIZE);
    sprintf(buf,"Delete from SYSTEM_EVENT_INFO where TIME <= '%s' ",strTime.c_str());
    String sql5(buf);

    if(execSQL(sql5)!=0)
    {
        printf("Delete SYSTEM_EVENT_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}

/*更新通道运行状态*/
int CSeekpaiDB::UpdateChannelStatus(int nChannel,int nRun)
{
    char buf[BUFF_SIZE]= {0};

    sprintf(buf,"update CHAN_INFO set CHAN_RUN = %d where CHAN_ID = %d",nRun,nChannel);
    String sql(buf);

    if(execSQL(sql)!=0)
    {
        printf("update CHAN_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}



//获取记录状态
RECORD_STATUS CSeekpaiDB::GetRecordStatus(int nKind)
{
    RECORD_STATUS status;

    char buf[BUFF_SIZE]= {0};

    if (nKind == 0)
    {
        sprintf(buf,"Select MIN(ID),MAX(ID),MIN(BEGIN_TIME),MAX(BEGIN_TIME) from TRAFFIC_EVENT_INFO");
    }
    else if (nKind == 1)
    {
        sprintf(buf,"Select MIN(ID),MAX(ID),MIN(TIME),MAX(TIME) from TRAFFIC_STATISTIC_INFO");
    }
    else if (nKind == 3)
    {
        sprintf(buf,"Select MIN(ID),MAX(ID),MIN(TIME),MAX(TIME) from NUMBER_PLATE_INFO");
    }
    else if (nKind == 4)
    {
        sprintf(buf,"Select MIN(ID),MAX(ID),MIN(TIME),MAX(TIME) from SYSTEM_EVENT_INFO");
    }

    String sql(buf);

    MysqlQuery q = execQuery(sql);

    if(!q.eof())
    {
        status.uBeginSeq  = q.getUnIntFileds(0);
        status.uEndSeq  = q.getUnIntFileds(1);
        String strBegin = q.getStringFileds(2);
        String strEnd = q.getStringFileds(3);
        status.uBeginTime  = MakeTime(strBegin);
        status.uEndTime  = MakeTime(strEnd);
    }
    q.finalize();

    if (nKind == 0)
    {
        sprintf(buf,"Select count(*) from TRAFFIC_EVENT_INFO");
    }
    else if (nKind == 1)
    {
        sprintf(buf,"Select COUNT(DISTINCT ID) from TRAFFIC_STATISTIC_INFO");
    }
    else if (nKind == 3)
    {
        sprintf(buf,"Select count(*) from NUMBER_PLATE_INFO");
    }
    else if (nKind == 4)
    {
        sprintf(buf,"Select count(*) from SYSTEM_EVENT_INFO");
    }

    status.uCount = getIntFiled(buf);

    printf("status.uBeginSeq=%u,status.uEndSeq=%u,status.uBeginTime=%d,status.uEndTime=%d,status.uCount=%d\n",status.uBeginSeq,status.uEndSeq,status.uBeginTime,status.uEndTime,status.uCount);

    return status;
}

//获取车牌区域信息
String CSeekpaiDB::GetPlateRegion()
{
    String strPlateRegion;
    /*	char buf[BUFF_SIZE]={0};
    	sprintf(buf, "select CHAN_CAR_AREA from CHAN_INFO");

    	//sql查询
    	String sql(buf);

    	MysqlQuery q = execQuery(sql);

    	String strCarArea = "";
    	strCarArea = q.getStringFileds(0);
    	if(strCarArea.size()>0)
    	q.finalize();


        std::stringstream stream;
    	std::string sub_str;
    	stream.str(strCarArea);
    	unsigned int x,y,k,len,nId,count=0;

    	//多个车牌检测区域
    	printf("====================strCarArea=%s\n",strCarArea.c_str());
    	while(getline(stream,sub_str,')'))
    	{
    		sub_str.erase(0,1);
    		count=0;
    		k = 0;
    		std::stringstream streamTemp(sub_str);
    		std::string subTemp;
    		while(getline(streamTemp,subTemp,'|'))
    		{
    			memset(buf,0,BUFF_SIZE);
    			strcpy(buf, subTemp.c_str());
    			if(k==0)
    			{
    				sscanf(buf,"%d",&nId);
    			//	printf("================nId=%d\n",nId);
    			}
    			else
    			{
    				sscanf(buf,"%d,%d",&x,&y);
    			//	printf("===============x=%d,y=%d\n",x,y);
    				x*=2;
    				strPlateRegion.append((char*)&x,sizeof(unsigned int));
    				strPlateRegion.append((char*)&y,sizeof(unsigned int));
    				count++;
    			}
    			k++;

    		}
    		len = strPlateRegion.size();
    		if(len>=8)
    		strPlateRegion.insert(len-8*count,(char*)&count,sizeof(count));
    	}*/

    return strPlateRegion;
}

unsigned int CSeekpaiDB::GetSeq(int nKind)
{
    char buf[BUFF_SIZE]= {0};

    if(nKind==0)
        sprintf(buf,"Select MAX(ID) from TRAFFIC_EVENT_INFO");
    else if(nKind==1)
        sprintf(buf,"Select MAX(ID) from TRAFFIC_STATISTIC_INFO");
    else if(nKind==3)
        sprintf(buf,"Select MAX(ID) from NUMBER_PLATE_INFO");
    else if(nKind==4)
        sprintf(buf,"Select MAX(ID) from SYSTEM_EVENT_INFO");

    String sql(buf);

//	printf("%s\n",buf);
    MysqlQuery q = execQuery(sql);
//	printf("q.numFileds()=%d\n",q.numFileds());
    unsigned int uSeq = 0;
    if(!q.eof())
    {

        uSeq = q.getUnIntFileds(0);
    }
    q.finalize();


    if(uSeq >= MAX_SEQ)
    {
        uSeq = 0;
    }
    uSeq++;

    // printf("uSeq=%u\n",uSeq);

    return uSeq;
}


//获取上次数据库记录时间
String CSeekpaiDB::GetLastRecorderTime()
{
    char buf[BUFF_SIZE]= {0};

    //取数据库初始时间
    memset(buf,0,BUFF_SIZE);
    sprintf(buf,"Select max(TIME) from SYSTEM_EVENT_INFO");
    String sql(buf);
    MysqlQuery q = execQuery(sql);

    String strTime;
    if(!q.eof())
    strTime = q.getStringFileds(0);

    q.finalize();

    return strTime;
}

//录象结束通知(nVideoType ＝ 0：事件录像，1：全天录像，2：违章录像)
void CSeekpaiDB::VideoSaveUpdate(String strVideoPath,int nChannelID,int nVideoType)
{
	String strFTP = strVideoPath;
    String strTmpPath = strVideoPath;

	if(g_nServerType == 13 && g_nFtpServer == 1)
	{
		string strDataPath = "/home/road/dzjc";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/dzjc";
		}
		strTmpPath.erase(0,strDataPath.size());
	}
	else if(g_nServerType == 7)
	{
		string strDataPath = "/home/road/video";
		if(nVideoType == 1)
		{
			if (IsDataDisk())
			{
				strDataPath = "/detectdata/video";
			}
		}
		else
		{
			strDataPath = "/home/road/red";
			if (IsDataDisk())
			{
				strDataPath = "/detectdata/red";
			}
		}
		
		strTmpPath.erase(0,strDataPath.size());
	}
	else
	{
		strTmpPath.erase(0,g_strVideo.size());
	}
    strTmpPath = g_ServerHost+strTmpPath;
    String strPath = "ftp://"+strTmpPath;

    char buf[BUFF_SIZE]= {0};

//    if(nVideoType <= 1 )//事件录像
	{
		sprintf(buf,"update TRAFFIC_EVENT_INFO set VIDEOSAVE = 1 where VIDEOPATH = '%s'",strPath.c_str());

		String sql(buf);

		if(execSQL(sql)!=0)
		{
			//LogError("通知录象结束失败!\n");
		}
	}

    if(nVideoType == 1)//全天录像
    {
        sprintf(buf,"update VIDEO_FILE_INFO set STATUS = 1 where PATH = '%s'",strVideoPath.c_str());

        String sql1(buf);

        if(execSQL(sql1)!=0)
        {
          //  LogError("通知录象结束失败!\n");
        }
    }

	if(nVideoType >= 1)//违章录像
	{
		sprintf(buf,"update NUMBER_PLATE_INFO set VIDEOSAVE = 1 where VIDEOPATH = '%s'",strPath.c_str());

		String sql2(buf);

		if(execSQL(sql2)!=0)
		{
			//LogError("通知录象结束失败!\n");
		}
	}

    //需要发送事件录象
	
#ifdef LS_QINGTIAN_IVAP
	if (g_nServerType == 5)
#else
	if(g_nServerType == 0 || g_nServerType == 7)
#endif	
	{
		if(nVideoType != 1)//不是全天录像
		{
			int nCamera = GetCameraID(nChannelID);
			AddVideoRecord(strVideoPath,nVideoType,nCamera);
		}
	}
    else if(g_nServerType == 3)
    {
        g_TravelServer.CreateVideoSendThread(strVideoPath);
    }
}

void CSeekpaiDB::GetVideoSaveString(String &strVideoPath,unsigned int uID,int nVideoType)
{
	char buf[BUFF_SIZE]= {0};

	if(nVideoType == 0)//车牌录像
	{
		sprintf(buf,"select VIDEOPATH from NUMBER_PLATE_INFO where ID = %u ",uID);

		String sql(buf);
		MysqlQuery q = execQuery(sql);

		if(!q.eof())
		{
			strVideoPath = q.getStringFileds("VIDEOPATH");
		}
		q.finalize();
	}
	else if(nVideoType == 1 )//事件录像
	{
		 sprintf(buf,"Select VIDEOPATH from TRAFFIC_EVENT_INFO where ID = %u ",uID);

		String sql(buf);
		MysqlQuery q = execQuery(sql);

		if(!q.eof())
		{
			strVideoPath = q.getStringFileds("VIDEOPATH");
		}
		q.finalize();
	}
	else if(nVideoType == 2)//全天录像
	{
		sprintf(buf,"Select PATH from VIDEO_FILE_INFO where ID = %u ",uID);
		//sprintf(buf,"Select * from VIDEO_FILE_INFO where ID = %u ",uID);

		//LogError("下载端口：查询-%s\r\n",buf);
		String sql(buf);
		MysqlQuery q = execQuery(sql);

		if(!q.eof())
		{
			strVideoPath = q.getStringFileds("PATH");
			//LogError("下载端口：查询数据---%s\r\n",strVideoPath);
		}
		q.finalize();
	}

}

//获取录像时间
String CSeekpaiDB::GetVideoTime(String strVideoPath,int nVideoType)
{
    String strTmpPath = strVideoPath;
    strTmpPath.erase(0,g_strVideo.size());
    strTmpPath = g_ServerHost+strTmpPath;
    String strPath = "ftp://"+strTmpPath;

	String strVideoTime = "";
	string strTime = "";
	UINT32 nMiTime = 0;

	char buf[BUFF_SIZE]= {0};

	if(nVideoType == 0)//事件录像
	{
			sprintf(buf,"select BEGIN_TIME,BEGIN_MITIME from TRAFFIC_EVENT_INFO where VIDEOPATH = '%s' order by ID desc limit 1",strPath.c_str());
	}
	else if(nVideoType == 1)//全天录像
	{
			sprintf(buf,"select BEGIN_TIME,BEGIN_MITIME from VIDEO_FILE_INFO where PATH = '%s'",strVideoPath.c_str());	
	}
	else if(nVideoType == 2)//违章录像
	{
			sprintf(buf,"Select TIME,MITIME from NUMBER_PLATE_INFO where VIDEOPATH = '%s' order by ID desc limit 1",strPath.c_str());	
	}

	String sqlSel(buf);

	MysqlQuery q = execQuery(sqlSel);
	if(!q.eof())
	{
		strTime = q.getStringFileds(0);//2011-06-23 14:50:54
		nMiTime = q.getIntFileds(1);
		//LogNormal("nMiTime=%d,strTime=%s,q.m_nCols=%d\n",nMiTime,strTime.c_str(),q.m_nCols);
	}
	q.finalize();

	
	if(strTime.size() > 0)
	{
			memset(buf, 0, BUFF_SIZE);
			string strYear = strTime.substr(0,4);
			string strMonth = strTime.substr(5,2);
			string strDay = strTime.substr(8,2);
			string strHour = strTime.substr(11,2);
			string strMinute = strTime.substr(14,2);
			string strSecond = strTime.substr(17,2);
			sprintf(buf,"%s%s%s%s%s%s%03d",strYear.c_str(),strMonth.c_str(),strDay.c_str(),strHour.c_str(),strMinute.c_str(),strSecond.c_str(),nMiTime);
			    
			strVideoTime = buf;
	}

	return strVideoTime;
}

//获取远程录像视频名称
StrList CSeekpaiDB::GetRemoteEventVideoPath(String strVideoPath)
{
        StrList strRemoteVideoPathList;

        String strTmpPath = strVideoPath;
        strTmpPath.erase(0,g_strVideo.size());
        strTmpPath = g_ServerHost+strTmpPath;
        String strPath = "ftp://"+strTmpPath;

        char buf[BUFF_SIZE]= {0};
        sprintf(buf,"select ROAD,BEGIN_TIME,BEGIN_MITIME,KIND from TRAFFIC_EVENT_INFO where VIDEOPATH = '%s'",strPath.c_str());

        String sql(buf);

        MysqlQuery q = execQuery(sql);


        while(!q.eof())
        {
            int nRoadWayID = 0;
            int nMiEventBeginTime = 0;
            int nEventKind = 0;
            nRoadWayID = q.getIntFileds("ROAD");
            String sBegin_Time = q.getStringFileds("BEGIN_TIME");
            nMiEventBeginTime = q.getIntFileds("BEGIN_MITIME");
            nEventKind = q.getIntFileds("KIND");

            if( nRoadWayID > 0 )
            {
                string strTime;
                strTime.append(sBegin_Time.c_str(),4);
                strTime.append(sBegin_Time.c_str()+5,2);
                strTime.append(sBegin_Time.c_str()+8,2);
                strTime.append(sBegin_Time.c_str()+11,2);
                strTime.append(sBegin_Time.c_str()+14,2);
                strTime.append(sBegin_Time.c_str()+17,2);

                string strDirection;
                ROAD_DIRECTION_MAP::iterator it =  g_roadDirectionMap.find(nRoadWayID);
                if(it != g_roadDirectionMap.end())
                {
                    strDirection = it->second.chDirection;
                }
                else
                {
                    strDirection = "1A";
                }

                CRoadXmlData xmlData;
                char chCode = xmlData.GetEventType(nEventKind);

                sprintf(buf,"EVENTVIDEO_%8s_%s_%d_%s%03d%c.mp4",g_strFtpUserName.c_str(),strDirection.c_str(),nRoadWayID,strTime.c_str(),nMiEventBeginTime,chCode);
                printf("nEventKind=%d,chCode=%c,buf=%s\n",nEventKind,chCode,buf);
                string strRemoteVideoPath(buf);

                strRemoteVideoPathList.push_back(strRemoteVideoPath);
            }

            q.nextRow();
        }
        q.finalize();

        return strRemoteVideoPathList;
}

bool CSeekpaiDB::GetVideoState(String strVideoPath)
{
    bool bSave = false;
    char buf[BUFF_SIZE]= {0};

    sprintf(buf,"select VIDEOSAVE from TRAFFIC_EVENT_INFO where VIDEOPATH = '%s'",strVideoPath.c_str());

    String sql(buf);

    MysqlQuery q = execQuery(sql);

    int nCount = 0;
    int nVideoSave = 0;
    if(!q.eof())
    {
        nVideoSave = q.getIntFileds(0);
        nCount = 1;
    }
    printf("nVideoSave=%d\n",nVideoSave);
    q.finalize();


	if(nCount == 0)
    {
        sprintf(buf,"select VIDEOSAVE from NUMBER_PLATE_INFO where VIDEOPATH = '%s'",strVideoPath.c_str());

        String sql1(buf);
        MysqlQuery q1 = execQuery(sql1);
        if(!q1.eof())
        {
            nVideoSave = q1.getIntFileds(0);
			 nCount = 1;
        }
        q1.finalize();

        printf("==========+++++++++++strVideoPath=%s,nVideoSave=%d\n",strVideoPath.c_str(),nVideoSave);
    }

    if(nCount == 0)
    {
        sprintf(buf,"select STATUS from VIDEO_FILE_INFO where PATH = '%s'",strVideoPath.c_str());

        String sql1(buf);
        MysqlQuery q1 = execQuery(sql1);
        if(!q1.eof())
        {
            nVideoSave = q1.getIntFileds(0);
			 nCount = 1;
        }
        q1.finalize();

        printf("==========+++++++++++strVideoPath=%s,nVideoSave=%d\n",strVideoPath.c_str(),nVideoSave);
    }



    if(nVideoSave == 1)
    bSave = true;

    return bSave;
}

//获取录象列表
void CSeekpaiDB::GetVideoList(StrList& listVideoPath,int nVideoSave)
{
    listVideoPath.clear();

    char buf[BUFF_SIZE]= {0};

    if(nVideoSave == 0)
    sprintf(buf,"select distinct(VIDEOPATH)  from TRAFFIC_EVENT_INFO where VIDEOSAVE = %d and VIDEOPATH <> '' order by ID",nVideoSave);
    else
    sprintf(buf,"select distinct(VIDEOPATH)  from TRAFFIC_EVENT_INFO where VIDEOSAVE = %d order by ID",nVideoSave);

    String sql(buf);

    MysqlQuery q = execQuery(sql);

    while(!q.eof())
    {
        String strVideoPath = q.getStringFileds(0);
        listVideoPath.push_back(strVideoPath);
        q.nextRow();
    }

    q.finalize();

}


//utf8转gbk
int CSeekpaiDB::UTF8ToGBK( std::string& str)
{
    if(str.size() <= 0)
    {
        return 0;
    }

    iconv_t convt = m_nCvtUTF;

    if( convt== (iconv_t)(-1) )
        return -1;

    std::string from = str;
    char* fptr = (char*)from.c_str();
    char* tptr = (char*)str.c_str();

    size_t fl = from.length();
    size_t tl = str.length();

    int ret;
	//printf("****************UTF8:%s\n",str.c_str());
	//printf("****************fptr:%s  size:%d tptr:%s  size:%d\n",fptr,fl,tptr,tl);

    ret = iconv( convt, &fptr, &fl, &tptr, &tl );

    if(from.length() > tl)
    str.resize( from.length() - tl );

    return ret;
}

//gbk转utf8
int CSeekpaiDB::GBKToUTF8( std::string& str)
{
    iconv_t convt = m_nCvtGBK;

    if( convt== (iconv_t)(-1) )
        return -1;
    std::string from = str;
    char* fptr = (char*)from.c_str();
    char* tptr = (char*)str.c_str();

    size_t fl = from.length();
    size_t tl = str.length();

    int ret;

    size_t len = 255;
    char  out[255]= {0};
    char* pChar = out;
    memcpy(out,str.c_str(),str.length());
    ret = iconv( convt, &fptr, &fl, &pChar, &len );
    std::string strOut(out);
    str = strOut;

    return ret;
}

/* 函数介绍：获取当前通道最新的图片编号
 * 输入参数：nChannel-通道编号,nType=0车牌图片，nType=1事件图片,nType=2车牌事件图片混合编号
 * 输出参数：无
 * 返回值：图片编号
 */
unsigned int  CSeekpaiDB::GetLastPicId()
{
    char buf[BUFF_SIZE]= {0};
    unsigned int uPicId = 0;
    unsigned int uVideoId = 0;

    sprintf(buf,"Select max(PIC_ID),max(VIDEO_ID) from PIC_INFO");


    String sql(buf);
    MysqlQuery q = execQuery(sql);

    if(q.numFileds() > 0)
    {
        uPicId = q.getUnIntFileds("max(PIC_ID)");
        uVideoId = q.getUnIntFileds("max(VIDEO_ID)");
    }
    q.finalize();

    if(uPicId<=0)
        uPicId = 1;

    if(uVideoId<=0)
        uVideoId = 1;

    g_uPicId = uPicId;
    g_uVideoId = uVideoId;


    printf("g_uPicId=%d,g_uVideoId=%d\n",g_uPicId,g_uVideoId);
    return uPicId;
}

/* 函数介绍：获取车牌识别结果
 * 输入参数：search_item_carnum-查询结构,nPageSize--页码
 * 输出参数：无
 * 返回值：车牌记录
 */
String CSeekpaiDB::GetCarNumHigh(SEARCH_ITEM_CARNUM& search_item_carnum,int nPageSize/*=0*/)
{
    String strTimeBeg = GetTime(search_item_carnum.uBeginTime);
    String strTimeEnd = GetTime(search_item_carnum.uEndTime);

    int nPage = search_item_carnum.uPage;
    int nSortId = search_item_carnum.uSortId;
    int nSortKind = search_item_carnum.uSortKind;
    int nChannel = search_item_carnum.uChannelId;
	int nRoadId = search_item_carnum.uRoadId;

	//LogNormal("------>uCarBrand=%d,uDetailCarBrand=%d\n",search_item_carnum.uCarBrand,search_item_carnum.uDetailCarBrand);
    unsigned int uColor = search_item_carnum.uColor;    //车牌颜色
    unsigned int uCarColor = search_item_carnum.uCarColor; //车身颜色
    unsigned int uCarType = search_item_carnum.uCarType;  //车辆类型
    unsigned int uCarBrand = search_item_carnum.uCarBrand; //产商标志

    unsigned int uDetailCarType = search_item_carnum.uDetailCarType;  //车型细分子类
    unsigned int uDetailCarBrand = search_item_carnum.uDetailCarBrand;  //产商细分子类
    unsigned int uDirection = search_item_carnum.uDirection;//行驶方向

    String strPlace(search_item_carnum.chPlace); //经过地点
    String strCarNum(search_item_carnum.chText);//车牌号

    String strExceptText(search_item_carnum.chExceptText); //排除在外的车牌省份缩写或非警车字符串

    unsigned int uTypeDetail = search_item_carnum.uTypeDetail; //车辆类型细分
    unsigned int uViolationType = search_item_carnum.uViolationType; //违法类型
    unsigned int uConditionType = search_item_carnum.uConditionType;//查询附加条件
    unsigned int uStatusType = search_item_carnum.uStatusType; //记录类型

    char bufChannel[80];
    char bufColor[80];
    char bufCarColor[80];
    char bufCarType[80];
    char bufCarBrand[80];

    char bufSpeed[80];
    char bufDirection[80];

    char bufPlace[80];
    char bufCarNum[80];
    char bufTypeDetail[80];
    char bufViolationType[80];

    char bufObject[80];
    //char bufViolation[80];
    char bufStatusType[80];
	char bufRoadId[80];
	char bufDetailCarType[80];
	char bufDetailCarBrand[80];

    memset(bufChannel, 0, 80);
    memset(bufColor, 0, 80);
    memset(bufCarColor, 0, 80);
    memset(bufCarType, 0, 80);
    memset(bufCarBrand, 0, 80);

    memset(bufSpeed, 0, 80);
    memset(bufDirection, 0, 80);
    memset(bufPlace, 0, 80);
    memset(bufCarNum, 0, 80);
    memset(bufTypeDetail, 0, 80);
    memset(bufViolationType, 0, 80);

    memset(bufObject, 0, 80);
    //memset(bufViolation, 0, 80);
    memset(bufStatusType, 0, 80);
	memset(bufRoadId, 0, 80);
	memset(bufDetailCarType, 0, 80);
	memset(bufDetailCarBrand, 0, 80);


	if (nRoadId != 0)
	{
		sprintf(bufRoadId, "and ROAD =%d ", nRoadId);
	}
    if(nChannel != 0)
    {
        sprintf(bufChannel, "and CHANNEL =%d ", nChannel);
    }
    if(uColor != 1000)
    {
        sprintf(bufColor, "and COLOR = %d ", uColor);
    }
    if(uCarColor != 1000)
    {
        sprintf(bufCarColor, "and CARCOLOR = %d ", uCarColor);
    }
    if(uCarType != 1000)
    {
        sprintf(bufCarType, "and TYPE = %d ", uCarType);
    }

    if(uCarBrand != 1001 && uCarBrand != 200001)
    {
        sprintf(bufCarBrand, " and FACTORY = %d ", uCarBrand);
    }

	if(uDetailCarType != 0)
    {
        sprintf(bufDetailCarType, " and SUBTYPE_DETAIL = %d ", uDetailCarType);
    }

	if(uDetailCarBrand != 1000)
	{
		sprintf(bufDetailCarBrand, " and SUBFACTORY = %d ", uDetailCarBrand);
	}

    if(uDirection != 1000)
    {
        sprintf(bufDirection, "and DIRECTION = %d ", uDirection);
    }

    if(strPlace.size() > 0)
    {
		if (1 == g_nGongJiaoMode)
		{
			sprintf(bufPlace, "and PLACE like '%%%s%%' ", strPlace.c_str());
		}
		else
		{
			sprintf(bufPlace, "and CHAN_PLACE like '%%%s%%' ", strPlace.c_str());
		}
    }

    if(strCarNum.size() > 0)
    {
		if (strCarNum == "_") //查询全部省份的车牌
			sprintf(bufCarNum, "and NUMBER like '%s%%' and NUMBER not like '%%*%%' ", strCarNum.c_str());
		else
		  sprintf(bufCarNum, "and NUMBER like '%s%%' ", strCarNum.c_str());
    }

    if(uTypeDetail != 1000)
    {
        sprintf(bufTypeDetail, "and TYPE_DETAIL = %d ", uTypeDetail);
    }

    if( (uConditionType & 0x0001) == 0x0001 )
    {
        sprintf(bufObject, "and NUMBER not like '***%%' ");
    }
    if( (uConditionType & 0x0002) == 0x0002 )
    {
        //sprintf(bufViolation, "and PECCANCY_KIND = 15 ");
        uViolationType = DETECT_RESULT_RED_LIGHT_VIOLATION;
    }

    if(uViolationType != 0)
    {
        sprintf(bufViolationType, "and PECCANCY_KIND = %d ", uViolationType);
    }

    if(uStatusType != 1000)
    {
        sprintf(bufStatusType, " and STATUS = %d ", uStatusType);
    }


	String sRoadId(bufRoadId);
    String sChannel(bufChannel);
    String sColor(bufColor);
    String sCarColor(bufCarColor);
    String sCarType(bufCarType);
    String sCarBrand(bufCarBrand);

    String sSpeed(bufSpeed);
    String sDirection(bufDirection);
    String sPlace(bufPlace);
    String sCarNum(bufCarNum);
    String sTypeDetail(bufTypeDetail);
    String sViolationType(bufViolationType);

    String sObject(bufObject);
    //String sViolation(bufViolation);
    String sStatusType(bufStatusType);
	String sDetailCarType(bufDetailCarType);
	String sDetailCarBrand(bufDetailCarBrand);

    String sTempExcept;
    String sTempCmp1;
    String sTempCmp2;

	//LogNormal("------>bufCarBrand=%s    sCarBrand=%s------>\n",bufCarBrand,sCarBrand);
    int nExceptTextSize = strExceptText.size();
    if(nExceptTextSize > 0)
    {
        int nIndex = 0;
#ifdef _DEBUG
        printf("==================nExceptTextSize=%d, strExceptText=%s !!\n", nExceptTextSize, strExceptText.c_str());
#endif
        String sExceptTextOne;
        char bufCarNumPro[80];
        memset(bufCarNumPro, 0, 80);

        sTempCmp1 = "武";
        sTempCmp2 = "警";
        while( nIndex < nExceptTextSize)
        {
            sExceptTextOne.assign(strExceptText, nIndex, 3);
            if(sExceptTextOne == sTempCmp1)
            {
                sExceptTextOne = "WJ";
            }
            else if(sExceptTextOne == sTempCmp2)
            {
                sExceptTextOne = "%警";
            }
            else
            {
            }
            sprintf(bufCarNumPro, " and NUMBER not like '%s%%' ", sExceptTextOne.c_str());
            String sCarNumPro(bufCarNumPro);

            sTempExcept.append(sCarNumPro);

            memset(bufCarNumPro, 0, 80);
            sExceptTextOne.erase(0, 3);

            nIndex += 3;
        }

        //sCarNum = "";
    }

    //增加查询条件
    char bufTemp[BUFF_SIZE*2];
    sprintf(bufTemp, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",\
            sChannel.c_str(), sRoadId.c_str(), sColor.c_str(), sCarColor.c_str(), sCarType.c_str(), sDetailCarType.c_str(),\
            sDetailCarBrand.c_str(),sDirection.c_str(), sPlace.c_str(), sCarNum.c_str(), sTypeDetail.c_str(), sViolationType.c_str(),\
            sObject.c_str(), sStatusType.c_str(), sCarBrand.c_str() );

    String strTemp(bufTemp);
//////////////////////

    printf("strTemp=%s,strTemp.size()=%d\n",strTemp.c_str(),strTemp.size());

    printf("strCarNum=%s,strCarNum.size()=%d\n",strCarNum.c_str(),strCarNum.size());

    String response;

    if(nPage<1||nPageSize<0)
    {
#ifdef _DEBUG
        printf("nPage or nPageSize is Error! \n");
#endif
        return response;
    }

    char buf[BUFF_SIZE*3]= {0};

    char SortKind[2][5]= {"ASC","DESC"};
    char SortId[16][15]= {"ID","TIME","CHAN_PLACE","CHANNEL","ROAD",
                          "DIRECTION","NUMBER","COLOR","CARNUMBER_TYPE","SPEED",
                          "CARCOLOR","TYPE","TYPE_DETAIL","PECCANCY_KIND","STATUS",
                          "FACTORY"
                         };
	if(g_nGongJiaoMode == 1)
	{
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
                    COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
                    SMALLPICPATH,PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
                    CARNUMBER_TYPE,STATUS,FACTORY,VIDEOPATH,LATITUDE,LONGITUDE \
                    from NUMBER_PLATE_INFO,CHAN_INFO \
                    where TIME >= '%s' and TIME <= '%s' \
                    %s %s\
                    and CHANNEL = CHAN_ID \
                    ORDER BY %s %s limit %d,%d",\
            strTimeBeg.c_str(),strTimeEnd.c_str(),\
            strTemp.c_str(), sTempExcept.c_str(),\
            SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
	}
	else
	{
		#ifndef GLOBALCARCLASSIFY
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
                    COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
                    SMALLPICPATH,CHAN_PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
                    CARNUMBER_TYPE,STATUS,FACTORY,VIDEOPATH \
                    from NUMBER_PLATE_INFO,CHAN_INFO \
                    where TIME >= '%s' and TIME <= '%s' \
                    %s %s\
                    and CHANNEL = CHAN_ID \
                    ORDER BY %s %s limit %d,%d",\
            strTimeBeg.c_str(),strTimeEnd.c_str(),\
            strTemp.c_str(), sTempExcept.c_str(),\
            SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
		#else
		sprintf(buf,"Select ID,CHANNEL,ROAD,TIME,NUMBER,\
                    COLOR,TYPE,CARCOLOR,DIRECTION,SPEED,\
                    SMALLPICPATH,CHAN_PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
                    CARNUMBER_TYPE,STATUS,FACTORY,VIDEOPATH,SUBTYPE_DETAIL,SUBFACTORY \
                    from NUMBER_PLATE_INFO,CHAN_INFO \
                    where TIME >= '%s' and TIME <= '%s' \
                    %s %s\
                    and CHANNEL = CHAN_ID \
                    ORDER BY %s %s limit %d,%d",\
            strTimeBeg.c_str(),strTimeEnd.c_str(),\
            strTemp.c_str(), sTempExcept.c_str(),\
            SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);
		//LogNormal("------>SQL buf=%s\n",buf);
		#endif
	}

    //获得查询数量
    int nCount=0;
    char bufCount[BUFF_SIZE*2];
    memset(bufCount, 0, BUFF_SIZE);

    sprintf(bufCount, "Select count(*) from NUMBER_PLATE_INFO, CHAN_INFO \
            Where TIME >= '%s' and TIME <= '%s'  %s %s and CHANNEL = CHAN_ID;", \
            strTimeBeg.c_str(),strTimeEnd.c_str(), strTemp.c_str(), sTempExcept.c_str() );

    String sqlCount(bufCount);
    //printf("\n=================sqlCount= %s!!!\n", sqlCount.c_str());
    nCount = getIntFiled(sqlCount);

    if(nPage==1)
    {
        int nNum = nCount % nPageSize;
        if(nNum > 0)
            nCount = nCount/nPageSize + 1;
        else
            nCount = nCount/nPageSize;
    }

    //String sql(buf);
    String sql2(buf);

    //test
    printf("\n=================sql2= %s!!!\n", sql2.c_str());

    MysqlQuery q = execQuery(sql2);
    printf("nCount=%d, numFileds=%d\n",nCount,q.numFileds());

    response.append((char*)&nCount,sizeof(nCount));

	int k_sql = 0;
    while(!q.eof())
    {
        RECORD_PLATE_CLIENT plate;
        plate.uSeq=q.getUnIntFileds("ID");

        int nChannelId = q.getIntFileds("CHANNEL");
		plate.uChannelID = nChannelId;
        //memcpy(plate.chReserved,&nChannelId,sizeof(int));//暂时借用保留字段

        plate.uRoadWayID = q.getIntFileds("ROAD");

        String strTime = q.getStringFileds("TIME");
        plate.uTime = MakeTime(strTime);

        String text = q.getStringFileds("NUMBER");
        memcpy(plate.chText,text.c_str(),text.size());

        plate.uColor = q.getIntFileds("COLOR");

        plate.uType = q.getIntFileds("TYPE");

        plate.uCarColor1 = q.getIntFileds("CARCOLOR");

        plate.uDirection = q.getIntFileds("DIRECTION");

        plate.uSpeed = q.getIntFileds("SPEED");

        String strPath = q.getStringFileds("SMALLPICPATH");
//		strPath.erase(0,g_strPic.size());
        memcpy(plate.chVideoPath,strPath.c_str(),strPath.size());

		if (1 != g_nGongJiaoMode)
		{
			String strPlace = q.getStringFileds("CHAN_PLACE");
			memcpy(plate.chPlace,strPlace.c_str(),strPlace.size());
		}

        strPath = q.getStringFileds("PICPATH");
//		strPath.erase(0,g_strPic.size());
        memcpy(plate.chPicPath,strPath.c_str(),strPath.size());

        plate.uTypeDetail = q.getIntFileds("TYPE_DETAIL");
        plate.uViolationType = q.getIntFileds("PECCANCY_KIND");
        plate.uPlateType = q.getIntFileds("CARNUMBER_TYPE");

        plate.uStatusType = q.getIntFileds("STATUS");
        plate.uCarBrand = q.getIntFileds("FACTORY");

		String videoPath = q.getStringFileds("VIDEOPATH");
		memcpy(plate.chVideoPath, videoPath.c_str(), videoPath.size());

		if(g_nGongJiaoMode == 1)
		{
			plate.uLatitude = q.getIntFileds("LATITUDE");
			plate.uLongitude = q.getIntFileds("LONGITUDE");
			String strPlace = q.getStringFileds("PLACE");
			memcpy(plate.chPlace,strPlace.c_str(),strPlace.size());
		}
		#ifdef GLOBALCARCLASSIFY
		plate.uDetailCarType = q.getIntFileds("SUBTYPE_DETAIL");
		plate.uDetailCarBrand = q.getIntFileds("SUBFACTORY");
		#endif

		/*if(k_sql == 0)
			LogNormal("------>SQL uCarBrand=%d,uDetailCarBrand=%d\n",plate.uCarBrand,plate.uDetailCarBrand);
		k_sql = 1;*/

        response.append((char*)&plate,sizeof(plate));

        q.nextRow();
    }

    q.finalize();

    return response;
}

/* 函数介绍：获取特征搜索高级查询车牌信息
* 输入参数：search_item_carnum-查询结构,nPageSize--页码
* 输出参数：无
* 返回值：对应特征搜索高级查询车牌记录
*/
String CSeekpaiDB::GetTextureHigh(SEARCH_ITEM_CARNUM& search_item_carnum, int nPageSize)
{
    //for test
    printf("==================Enter CSeekpaiDB::GetTextureHigh()===========!!!\n");

    String strTimeBeg = GetTime(search_item_carnum.uBeginTime);
    String strTimeEnd = GetTime(search_item_carnum.uEndTime);

    int nPage = search_item_carnum.uPage;
    int nSortId = search_item_carnum.uSortId;
    int nSortKind = search_item_carnum.uSortKind;
    int nChannel = search_item_carnum.uChannelId;

    unsigned int uColor = search_item_carnum.uColor;    //车牌颜色
    unsigned int uCarColor = search_item_carnum.uCarColor; //车身颜色
    unsigned int uCarType = search_item_carnum.uCarType;  //车辆类型
    unsigned int uDetailCarType = search_item_carnum.uDetailCarType;  //车型细分子类
    unsigned int uDetailCarBrand = search_item_carnum.uDetailCarBrand;  //产商细分子类
    unsigned int uDirection = search_item_carnum.uDirection;//行驶方向
    //unsigned int uCarBrand = search_item_carnum.uCarBrand; //产商标志

    String strPlace(search_item_carnum.chPlace); //经过地点
    String strCarNum(search_item_carnum.chText);//车牌号

    String strExceptText(search_item_carnum.chExceptText); //排除在外的车牌省份缩写或非警车字符串

    unsigned int uTypeDetail = search_item_carnum.uTypeDetail; //车辆类型细分
    unsigned int uViolationType = search_item_carnum.uViolationType; //违法类型
    unsigned int uConditionType = search_item_carnum.uConditionType;//查询附加条件
    unsigned int uStatusType = search_item_carnum.uStatusType; //记录类型

    char bufChannel[80];
    char bufColor[80];
    char bufCarColor[80];
    char bufCarType[80];
    char bufSpeed[80];
    char bufDirection[80];
    char bufPlace[80];
    char bufCarNum[80];
    char bufTypeDetail[80];
    char bufViolationType[80];
    char bufObject[80];
    //char bufViolation[80];
    char bufStatusType[80];

    memset(bufChannel, 0, 80);
    memset(bufColor, 0, 80);
    memset(bufCarColor, 0, 80);
    memset(bufCarType, 0, 80);
    memset(bufSpeed, 0, 80);
    memset(bufDirection, 0, 80);
    memset(bufPlace, 0, 80);
    memset(bufCarNum, 0, 80);
    memset(bufTypeDetail, 0, 80);
    memset(bufViolationType, 0, 80);
    memset(bufObject, 0, 80);
    //memset(bufViolation, 0, 80);
    memset(bufStatusType, 0, 80);

    if(nChannel != 0)
    {
        sprintf(bufChannel, "and CHANNEL =%d ", nChannel);
    }
    if(uColor != 1000)
    {
        sprintf(bufColor, "and COLOR = %d ", uColor);
    }
    if(uCarColor != 1000)
    {
        sprintf(bufCarColor, "and CARCOLOR = %d ", uCarColor);
    }
    if(uCarType != 1000)
    {
        sprintf(bufCarType, "and TYPE = %d ", uCarType);
    }

    if(uDirection != 1000)
    {
        sprintf(bufDirection, "and DIRECTION = %d ", uDirection);
    }

    if(strPlace.size() > 0)
    {
        sprintf(bufPlace, "and CHAN_PLACE like '%%%s%%' ", strPlace.c_str());
    }

    if(strCarNum.size() > 0)
    {
        sprintf(bufCarNum, "and NUMBER like '%s%%' ", strCarNum.c_str());
    }

    if(uTypeDetail != 1000)
    {
        sprintf(bufTypeDetail, "and TYPE_DETAIL = %d ", uTypeDetail);
    }

    if( (uConditionType & 0x0001) != 0x0001 )
    {
        sprintf(bufObject, "and NUMBER not like '***%%' ");
    }
    if( (uConditionType & 0x0002) == 0x0002 )
    {
        //sprintf(bufViolation, "and PECCANCY_KIND = 15 ");
        uViolationType = 15;
    }

    if(uViolationType != 0)
    {
        sprintf(bufViolationType, " and PECCANCY_KIND =%d ", uViolationType);
    }

    if(uStatusType != 1000)
    {
        sprintf(bufStatusType, " and STATUS = %d ", uStatusType);
    }



    String sChannel(bufChannel);
    String sColor(bufColor);
    String sCarColor(bufCarColor);
    String sCarType(bufCarType);
    String sSpeed(bufSpeed);
    String sDirection(bufDirection);
    String sPlace(bufPlace);
    String sCarNum(bufCarNum);
    String sTypeDetail(bufTypeDetail);
    String sViolationType(bufViolationType);
    String sObject(bufObject);
    //String sViolation(bufViolation);
    String sStatusType(bufStatusType);

    String sTempExcept;
    String sTempCmp1;
    String sTempCmp2;
    int nExceptTextSize = strExceptText.size();
    if(nExceptTextSize > 0)
    {
        int nIndex = 0;
#ifdef _DEBUG
        printf("==================nExceptTextSize=%d, strExceptText=%s !!\n", nExceptTextSize, strExceptText.c_str());
#endif
        String sExceptTextOne;
        char bufCarNumPro[80];
        memset(bufCarNumPro, 0, 80);

        sTempCmp1 = "武";
        sTempCmp2 = "警";
        while( nIndex < nExceptTextSize)
        {
            sExceptTextOne.assign(strExceptText, nIndex, 3);
            if(sExceptTextOne == sTempCmp1)
            {
                sExceptTextOne = "WJ";
            }
            else if(sExceptTextOne == sTempCmp2)
            {
                sExceptTextOne = "%警";
            }
            else
            {
            }

            sprintf(bufCarNumPro, " and NUMBER not like '%s%%' ", sExceptTextOne.c_str());
            String sCarNumPro(bufCarNumPro);

            sTempExcept.append(sCarNumPro);

            memset(bufCarNumPro, 0, 80);
            sExceptTextOne.erase(0, 3);

            nIndex += 3;
        }

        //sCarNum = "";
    }

    //增加查询条件
    char bufTemp[BUFF_SIZE];
    sprintf(bufTemp, "%s%s%s%s%s%s%s%s%s%s%s%s", \
            sChannel.c_str(), sColor.c_str(), sCarColor.c_str(), sCarType.c_str(), sSpeed.c_str(), \
            sDirection.c_str(), sPlace.c_str(), sCarNum.c_str(), sTypeDetail.c_str(), sViolationType.c_str(),\
            sObject.c_str(), sStatusType.c_str() );

    String strTemp(bufTemp);

//////////////////////

#ifdef _DEBUG
    printf("=====nPage=%d nPageSize =%d=====================!!\n",nPage, nPageSize);

    printf("strTemp=%s,strTemp.size()=%d\n",strTemp.c_str(),strTemp.size());

    printf("strCarNum=%s,strCarNum.size()=%d\n",strCarNum.c_str(),strCarNum.size());
#endif

    String response;

    if(nPage<1||nPageSize<0)
    {
#ifdef _DEBUG
        printf("nPage or nPageSize is Error! \n");
#endif
        return response;
    }

    char buf[BUFF_SIZE*3]= {0};


    char SortKind[2][5]= {"ASC","DESC"};
    char SortId[16][15]= {"ID","TIME","CHANNEL","ROAD","NUMBER",
                          "COLOR","TYPE","CARCOLOR","DIRECTION","SPEED",
                          "SMALLPICPATH","CHAN_PLACE","PICPATH","TYPE_DETAIL","PECCANCY_KIND",
                          "STATUS"
                         };

    sprintf(buf,"Select ID, NUMBER, SMALLPICPATH, PICPATH, \
                COLOR,TYPE,CARCOLOR,DIRECTION,SPEED, \
                SMALLPICPATH,CHAN_PLACE,PICPATH,TYPE_DETAIL,PECCANCY_KIND,\
                CARNUMBER_TYPE,STATUS \
                from NUMBER_PLATE_INFO,CHAN_INFO \
                where TIME >= '%s' and TIME <= '%s' \
                %s %s\
                and NUMBER not like '***%%' \
                and CHANNEL = CHAN_ID \
                ORDER BY %s %s limit %d,%d",\
            strTimeBeg.c_str(),strTimeEnd.c_str(),\
            strTemp.c_str(), sTempExcept.c_str(), \
            SortId[nSortId],SortKind[nSortKind],(nPage-1)*nPageSize,nPageSize);


    //获得查询数量
    int nCount=0;
    char bufCount[BUFF_SIZE*2];
    memset(bufCount, 0, BUFF_SIZE);

    sprintf(bufCount, "Select count(*) from NUMBER_PLATE_INFO, CHAN_INFO \
            Where TIME >= '%s' and TIME <= '%s'  %s %s and CHANNEL = CHAN_ID;", \
            strTimeBeg.c_str(),strTimeEnd.c_str(), strTemp.c_str(), sTempExcept.c_str() );

    String sqlCount(bufCount);
    //printf("\n=================sqlCount= %s!!!\n", sqlCount.c_str());
    nCount = getIntFiled(sqlCount);

    if(nPage==1)
    {
        int nNum=nCount % nPageSize;
        if(nNum>0)
            nCount=nCount/nPageSize+1;
        else
            nCount=nCount/nPageSize;
    }

    //String sql(buf);
    String sql2(buf);

    //test
    printf("\n=================sql2= %s!!!\n", sql2.c_str());

    MysqlQuery q = execQuery(sql2);
    printf("nCount=%d, numFileds=%d\n",nCount,q.numFileds());

    response.append((char*)&nCount,sizeof(nCount));

    while(!q.eof())
    {
        RECORD_PLATE_TEXTURE plate_texture;
        plate_texture.uSeq=q.getUnIntFileds("ID");

        String text = q.getStringFileds("NUMBER");
        memcpy(plate_texture.chText,text.c_str(),text.size());

        String strPath = q.getStringFileds("SMALLPICPATH");
//		strPath.erase(0,g_strPic.size());
        memcpy(plate_texture.chSmallPicPath,strPath.c_str(),strPath.size());

        strPath = q.getStringFileds("PICPATH");
//		strPath.erase(0,g_strPic.size());
        memcpy(plate_texture.chPicPath,strPath.c_str(),strPath.size());


        //把特征信息也插入到每条车牌信息后面
        if(!GetTextureFromPicFile(strPath, plate_texture.nTexture))
        {
            printf("\n============Can't get the Texture Info!!!\n");
        }

        //for test
        //for(int i=0; i<400; i++)
        //{
        //   printf("%u ", plate_texture.nTexture[i]);
        //}

        response.append((char*)&plate_texture,sizeof(plate_texture));

        q.nextRow();
    }

    q.finalize();

    //for test
    printf("==================Out CSeekpaiDB::GetTextureHigh()===========!!!\n");

    return response;
}

/* 函数介绍：获取车牌大图后面的特征信息
 * 输入参数：strPicPath:图片路径 texture:保存读到的特征信息
 * 输出参数：无
 * 返回值：对应车牌大图后面的特征信息
 */
bool CSeekpaiDB::GetTextureFromPicFile(std::string &strPicPath, unsigned short* texture)
{
    //得到完整路径
//    strPicPath.insert(0,g_strPic.c_str(),g_strPic.size());

    //////////////////////////////////////////
    FILE* fp =NULL;
    fp = fopen(strPicPath.c_str(),"rb");

    //for test
    //printf("\nCSeekpaiDB::GetTextureFromPicFile==================strPicPath.c_str() = %s\n",strPicPath.c_str());

    if(fp!=NULL)
    {
        int nSize = DIM_FEATURE * sizeof(unsigned short);

        if (fseek(fp, -nSize, SEEK_END)==0)
        {
            //for test
            //printf("\nCSeekpaiDB::GetTextureFromPicFile==================strPicPath.c_str() = %s !\n",strPicPath.c_str());

            fread(texture, nSize, 1, fp);
            fclose(fp);

            //for test
            //for(int i=0; i<400; i++)
            //{
            //    printf("%u ", texture[i]);
            //}

            return true;
        }
        else
        {
            fclose(fp);
        }
    }

    //for test
    //printf("\nCSeekpaiDB::GetTextureFromPicFile==================strPicPath.c_str() = %s ERROR!\n",strPicPath.c_str());
    return false;
}

//根据相机编号获取通道编号
int CSeekpaiDB::GetChannelIDByCameraID(int nCameraID)
{
    int nChannelID = 0;
    char buf[BUFF_SIZE] = {0};
    sprintf(buf, "SELECT CHAN_ID FROM CHAN_INFO WHERE CAMERA_ID=%d", nCameraID);
    std::string sql(buf);
    MysqlQuery q = g_skpDB.execQuery(sql);
    if (!q.eof())
    {
        nChannelID = q.getIntFileds(0);
    }
    q.finalize();
    return nChannelID;
}


/**
 * 从数据库中读取指定通道信息
 */
void CSeekpaiDB::GetChannelInfoFromDB(int nCameraID, SRIP_CHANNEL &sChannel)
{
    char buf[BUFF_SIZE] = {0};
    sprintf(buf, "SELECT CHAN_ID,CHAN_PLACE,CHAN_KIND,CHAN_FORMAT,CHAN_EVENT_CAPTURE,\
                       CHAN_RUN,CHAN_DIRECTION,CAMERA_TYPE,PANNEL_ID,CAMERA_STATE \
                        FROM CHAN_INFO WHERE CAMERA_ID=%d", nCameraID);
    std::string sql(buf);
    MysqlQuery q = g_skpDB.execQuery(sql);
    if (!q.eof())
    {
        //通道ID
        sChannel.uId = q.getIntFileds("CHAN_ID");
        //通道地点
        String strLoc = q.getStringFileds("CHAN_PLACE");
        memcpy(sChannel.chPlace,strLoc.c_str(),strLoc.size());
        //通道类型
        sChannel.eType = (CHANNEL_TYPE)q.getIntFileds("CHAN_KIND");
        //视频格式
        sChannel.eVideoFmt = (VEDIO_FORMAT)q.getIntFileds("CHAN_FORMAT");
        //是否事件录象
        sChannel.bEventCapture = q.getIntFileds("CHAN_EVENT_CAPTURE");
        //运行状态
        sChannel.bRun = q.getIntFileds("CHAN_RUN");
        //通道方向
        sChannel.uDirection = q.getIntFileds("CHAN_DIRECTION");
        //相机型号
        sChannel.nCameraType = q.getIntFileds("CAMERA_TYPE");
        //断面编号
        sChannel.nPannelID = q.getIntFileds("PANNEL_ID");
        //相机状态
        sChannel.nCameraStatus = q.getIntFileds("CAMERA_STATE");
        //相机编号
        sChannel.nCameraId = nCameraID;
    }
    q.finalize();
}

/**
* 通过通道ID从数据库中读取指定通道信息
*/
bool CSeekpaiDB::GetChannelInfoFromDBById(int id, SRIP_CHANNEL &sChannel)
{
	bool bRet = false;
    char buf[BUFF_SIZE] = {0};
	sprintf(buf, "SELECT CHAN_ID,CHAN_PLACE, CHAN_KIND,CHAN_FORMAT,CHAN_EVENTCAPTURETIME,\
		VIDEO_BEGINTIME,VIDEO_ENDTIME,CHAN_BRIGHTNESS,CHAN_CONTRAST,CHAN_SATURATION,\
		CHAN_HUE,CHAN_CAP_TYPE,CHAN_CAP_BEGINTIME,CHAN_CAP_ENDTIME,CHAN_YUV_HOST,\
		CHAN_YUV_PORT,CHAN_DETECT_KIND,CHAN_DETECT_TIME,CHAN_EVENT_CAPTURE,YUV_FORMAT,\
		CHAN_RMSHADE,CHAN_RMTINGLE,CHAN_SENSITIVE,CHAN_RUN,CAMERA_ID,\
		CHAN_SRC_FILE,CHAN_SHOWTIME,CAMERA_TYPE,CHAN_DIRECTION,PANNEL_ID,\
		SYN_HOST, SYN_PORT,CHAN_PRESET,MONITOR_ID,VIDEO_ID,\
		WORKMODE,CAMERAIP \
		from CHAN_INFO WHERE CHAN_ID=%d", id);

    std::string sql(buf);
    MysqlQuery q = g_skpDB.execQuery(sql);
    if (!q.eof())
    {
		//sChannel.nChanWayType = g_nRoadType;
		//通道ID
		sChannel.uId = q.getIntFileds("CHAN_ID");
		//通道地点
		String strLoc = q.getStringFileds("CHAN_PLACE");
		memcpy(sChannel.chPlace,strLoc.c_str(),strLoc.size());
		//通道类型
		sChannel.eType = (CHANNEL_TYPE)q.getIntFileds("CHAN_KIND");
		//视频格式
		sChannel.eVideoFmt = (VEDIO_FORMAT)q.getIntFileds("CHAN_FORMAT");
		//事件录像时间
		sChannel.uEventCaptureTime = q.getIntFileds("CHAN_EVENTCAPTURETIME");
		String strBeginTime = q.getStringFileds("VIDEO_BEGINTIME");
		sChannel.uVideoBeginTime = MakeTime(strBeginTime);
		String strEndTime = q.getStringFileds("VIDEO_ENDTIME");
		sChannel.uVideoEndTime = MakeTime(strEndTime);
		//录像参数
		sChannel.eCapType = (CAPTURE_TYPE)q.getIntFileds("CHAN_CAP_TYPE");
		//录像开始时间
		sChannel.uCapBeginTime = q.getIntFileds("CHAN_CAP_BEGINTIME");
		//录像结束时间
		sChannel.uCapEndTime = q.getIntFileds("CHAN_CAP_ENDTIME");
		//YUV参数
		//主机
		String strMonitorHost =q.getStringFileds("CHAN_YUV_HOST");
		memcpy(sChannel.chMonitorHost,strMonitorHost.c_str(),strMonitorHost.size());
		memcpy(sChannel.chHost,g_ServerHost.c_str(),g_ServerHost.size());
		//端口
		sChannel.uMonitorPort = q.getIntFileds("CHAN_YUV_PORT");
		//预置位
		sChannel.nPreSet = q.getIntFileds("CHAN_PRESET");
		//通道检测类型
		CHANNEL_DETECT_KIND uDetectKind = (CHANNEL_DETECT_KIND)q.getIntFileds("CHAN_DETECT_KIND");
		if((uDetectKind&DETECT_VTS)==DETECT_VTS)
		{
			uDetectKind = (CHANNEL_DETECT_KIND)(uDetectKind|DETECT_VIOLATION);
		}
		if(g_nMultiPreSet == 1)//存在多个预置位
		{
			CXmlParaUtil xml;
			int nPreSetDetectKind = xml.LoadPreSetDetectKind(sChannel.uId,sChannel.nPreSet);
			if(nPreSetDetectKind != 0)
			{
				uDetectKind = (CHANNEL_DETECT_KIND)nPreSetDetectKind;
			}
		}
		sChannel.uDetectKind = uDetectKind;
		//白天晚上检测
		sChannel.uDetectTime = (CHANNEL_DETECT_TIME)q.getIntFileds("CHAN_DETECT_TIME");

		//是否事件录象
		sChannel.bEventCapture = q.getIntFileds("CHAN_EVENT_CAPTURE");
		//yuv格式
		sChannel.nYUVFormat = q.getIntFileds("YUV_FORMAT");
		sChannel.bRun = q.getIntFileds("CHAN_RUN");
		sChannel.nCameraId = q.getIntFileds("CAMERA_ID");
		String strFileName =q.getStringFileds("CHAN_SRC_FILE");
		memcpy(sChannel.chFileName,strFileName.c_str(),strFileName.size());

		sChannel.nCameraType = q.getIntFileds("CAMERA_TYPE");
		sChannel.uDirection = q.getIntFileds("CHAN_DIRECTION");
		sChannel.nPannelID = q.getIntFileds("PANNEL_ID");

		//相邻同步主机
		String strSynHost =q.getStringFileds("SYN_HOST");
		memcpy(sChannel.chSynHost,strSynHost.c_str(),strSynHost.size());

		//相邻同步主机端口
		sChannel.uSynPort = SYNCH_PORT;

		//monitor编号
		sChannel.nMonitorId = q.getIntFileds("MONITOR_ID");
		//视频编号
		sChannel.nVideoIndex = q.getIntFileds("VIDEO_ID");
		//工作方式
		sChannel.nWorkMode = q.getIntFileds("WORKMODE");
		//通道的相机IP
		String strCameraIP = q.getStringFileds("CAMERAIP");
		memcpy(sChannel.chCameraHost, strCameraIP.c_str(), strCameraIP.size());

		bRet = true;
    }
    q.finalize();

	return bRet;
}

/**
 * 根据通道编号设置相机状态信息
 */
int CSeekpaiDB::SetCameraStateToDBByChannelID(int nChannelID, CameraState state)
{
    char buf[BUFF_SIZE] = {0};
    sprintf(buf, "UPDATE CHAN_INFO SET CAMERA_STATE=%d WHERE CHAN_ID=%d", (int)state, nChannelID);

    std::string sql(buf);

    if(execSQL(sql)!=0)
    {
        printf("UPDATE CHAN_INFO SET CAMERA_STATE error! \n");
        return SRIP_ERROR_USER_FAILE;
    }
    return SRIP_OK;
}
//获取相机状态
int CSeekpaiDB::GetCameraState(int nCameraID)
{
    char buf[BUFF_SIZE] = {0};
    sprintf(buf, "select CAMERA_STATE from CHAN_INFO where CAMERA_ID=%d", nCameraID);

    std::string sql(buf);
    MysqlQuery q = execQuery(sql);

    int nState = 0;
    if(!q.eof())
    {
        nState = q.getIntFileds(0);
    }
    q.finalize();
    return nState;
}

//zhangyaoyao: get user's right by name
int CSeekpaiDB::GetRightByName(String strUserName)
{
    char szBuff[BUFF_SIZE] = {0};
    int nRight = 0;

    sprintf(szBuff, "select USER_RIGHT1 from USER_INFO where USER_NAME='%s';", strUserName.c_str());

    String strSql(szBuff);
    MysqlQuery q = execQuery(strSql);

    if(!q.eof())
    {
        nRight = q.getIntFileds(0);
    }
    q.finalize();
    return nRight;
}

int CSeekpaiDB::AddVideoRecord(string strVideoPath, int nVideoType,int nCameraId)
{
	//LogNormal("AddVideo,%d,%s\n",nCameraId,strVideoPath.c_str());
	char buf[BUFF_SIZE]= {0};

	sprintf(buf,"Insert into VIDEO_RECORD (VIDEOPATH, TYPE, FLAG,CAMERAID) Values('%s',%d,0,%d)",strVideoPath.c_str(),nVideoType,nCameraId);
	String sql(buf);

    if(execSQL(sql)!=0)
    {
        printf("Insert into VIDEO_RECORD error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}

string CSeekpaiDB::GetVideoRecord(int& nType,int& nCameraID)
{
	char buf[BUFF_SIZE]= {0};
	String strPath;

	sprintf(buf,"select VIDEOPATH,TYPE,CAMERAID from VIDEO_RECORD where FLAG=0 order by ID desc limit 1");
	String sql(buf);

	MysqlQuery q1 = execQuery(sql);
	if(!q1.eof())
	{
		strPath = q1.getStringFileds("VIDEOPATH");
		nType = q1.getIntFileds("TYPE");
		nCameraID = q1.getIntFileds("CAMERAID");
	}
	q1.finalize();

	return strPath.c_str();
}

int CSeekpaiDB::UpdateVideoRecord(string strVideoPath, int nFlag)
{
	char buf[BUFF_SIZE]= {0};
	sprintf(buf,"update VIDEO_RECORD set FLAG = %d where VIDEOPATH = '%s'",nFlag,strVideoPath.c_str());

	String sql(buf);

	if(execSQL(sql)!=0)
	{
		return SRIP_ERROR_USER_FAILE;
	}
	return SRIP_OK;
}

int CSeekpaiDB::DeleteVideoRecord()
{
	char buf[BUFF_SIZE]= {0};
	sprintf(buf,"delete from VIDEO_RECORD where FLAG = 1");
	String sql(buf);

	if(execSQL(sql)!=0)
	{
		return SRIP_ERROR_USER_FAILE;
	}
	return SRIP_OK;
}

//添加车牌黑白名单
int CSeekpaiDB::AddSpecialCard(SPECIALCARD& specialcard_item, int &nIndex)
{
    //unsigned int uId = specialcard_item.uId;
    String strDepartment(specialcard_item.chDepartment);
    unsigned int uBehavior_Kind = specialcard_item.uBehavior_Kind;
    String strTimeBegin = GetTime(specialcard_item.uBegin_Time);
    String strTimeEnd = GetTime(specialcard_item.uEnd_Time);
    String strCarNum(specialcard_item.chText);
    unsigned int uCarNumType = specialcard_item.uCarNumType;
    unsigned int uCarType = specialcard_item.uCarType;
    unsigned int uCarNumColor = specialcard_item.uColor;
    unsigned int uKind = specialcard_item.uKind;

    char buf[BUFF_SIZE]= {0};
    sprintf(buf,"Insert into SPECIAL_CARD_INFO ( \
				DEPARTMENT, BEHAVIOR_KIND, BEGIN_TIME, END_TIME, NUMBER, \
				CARNUMBER_TYPE, CAR_TYPE, COLOR, KIND) \
				Values('%s', %d , '%s', '%s', '%s', \
						%d, %d, %d, %d)",\
            strDepartment.c_str(), uBehavior_Kind, strTimeBegin.c_str(), strTimeEnd.c_str(), strCarNum.c_str(), \
            uCarNumType, uCarType, uCarNumColor, uKind);
    String sql(buf);

    if(execSQL(sql,(unsigned int*)&nIndex)!=0)
    {
        printf("Insert into SPECIAL_CARD_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}

//删除车牌黑白名单
int CSeekpaiDB::DeleteSpecialCard(SPECIALCARD& specialcard_item)
{
    unsigned int uId = specialcard_item.uId;

    char buf[BUFF_SIZE]= {0};
#ifdef _DEBUG
    printf("==================Enter CSeekpaiDB::DeleteSpecialCard()===========!!!\n");
#endif
    sprintf(buf,"Delete from SPECIAL_CARD_INFO where ID = %d", uId);
    String sql(buf);
#ifdef _DEBUG
    printf("\n=================sql= %s!!!\n", sql.c_str());
#endif
    if(execSQL(sql)!=0)
    {
        printf("Delete from SPECIAL_CARD_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }
#ifdef _DEBUG
    printf("==================Out CSeekpaiDB::DeleteSpecialCard()===========!!!\n");
#endif
    return SRIP_OK;

}

//修改车牌黑白名单
int CSeekpaiDB::ModifySpecialCard(SPECIALCARD& specialcard_item)
{
    unsigned int uId = specialcard_item.uId;
    String strDepartment(specialcard_item.chDepartment);
    unsigned int uBehavior_Kind = specialcard_item.uBehavior_Kind;
    String strTimeBegin = GetTime(specialcard_item.uBegin_Time);
    String strTimeEnd = GetTime(specialcard_item.uEnd_Time);
    String strCarNum(specialcard_item.chText);
    unsigned int uCarNumType = specialcard_item.uCarNumType;
    unsigned int uCarType = specialcard_item.uCarType;
    unsigned int uCarNumColor = specialcard_item.uColor;
    unsigned int uKind = specialcard_item.uKind;

    char buf[BUFF_SIZE]= {0};

    sprintf(buf,"Update SPECIAL_CARD_INFO \
				set DEPARTMENT = '%s', BEHAVIOR_KIND = %d, BEGIN_TIME = '%s', END_TIME = '%s', NUMBER = '%s',\
				CARNUMBER_TYPE = %d, CAR_TYPE = %d, COLOR = %d\
				where Kind = %d and ID = %d",\
            strDepartment.c_str(), uBehavior_Kind, strTimeBegin.c_str(), strTimeEnd.c_str(), strCarNum.c_str(), \
            uCarNumType, uCarType, uCarNumColor,
            uKind,uId);
    String sql(buf);
#ifdef _DEBUG
    printf("\n=================sql= %s!!!\n", sql.c_str());
#endif
    if(execSQL(sql)!=0)
    {
        printf("Update SPECIAL_CARD_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

    return SRIP_OK;
}

//修改车牌
int CSeekpaiDB::ModifyCarNum(int nSeq,String strRequest)
{
	if(strRequest.size() < sizeof(RECORD_PLATE_CLIENT))
	{
		 return SRIP_ERROR_USER_FAILE;
	}

    char buf[BUFF_SIZE]= {0};
	RECORD_PLATE_CLIENT plate;
	plate = *((RECORD_PLATE_CLIENT*)(strRequest.c_str()));
	string strCarNum(plate.chText);
	String strTime = GetTime(plate.uTime);

	#ifndef GLOBALCARLABEL
    sprintf(buf,"Update NUMBER_PLATE_INFO \
				set NUMBER = '%s',\
				CARCOLOR = %d,\
				FACTORY =%d,\
				TYPE = %d,\
				TYPE_DETAIL = %d,\
				CARNUMBER_TYPE = %d,\
				PECCANCY_KIND = %d,\
				SPEED = %d, \
				TIME = '%s', \
				MITIME = %d, \
				DIRECTION = %d,\
				COLOR = %d,\
				CARCOLORSECOND = %d,\
				STATUS = %d, \
				ROAD = %d \
				where ID = %d",\
				strCarNum.c_str(),plate.uCarColor1,plate.uCarBrand,plate.uType,plate.uTypeDetail, plate.uPlateType, \
				plate.uViolationType,plate.uSpeed,strTime.c_str(), plate.uMiTime,plate.uDirection,\
				plate.uColor,plate.uCarColor2, plate.uStatusType, plate.uRoadWayID,\
				plate.uSeq);
	#else

			#ifndef DETAIL_OLDBRAND
			 sprintf(buf,"Update NUMBER_PLATE_INFO \
						set NUMBER = '%s',\
						CARCOLOR = %d,\
						TYPE = %d,\
						TYPE_DETAIL = %d,\
						CARNUMBER_TYPE = %d,\
						PECCANCY_KIND = %d,\
						SPEED = %d, \
						TIME = '%s', \
						MITIME = %d, \
						DIRECTION = %d,\
						COLOR = %d,\
						CARCOLORSECOND = %d,\
						STATUS = %d, \
						ROAD = %d \
						where ID = %d",\
						strCarNum.c_str(),plate.uCarColor1,plate.uType,plate.uTypeDetail, plate.uPlateType, \
						plate.uViolationType,plate.uSpeed,strTime.c_str(), plate.uMiTime,plate.uDirection,\
						plate.uColor,plate.uCarColor2, plate.uStatusType, plate.uRoadWayID,\
						plate.uSeq);
		#else

				sprintf(buf,"Update NUMBER_PLATE_INFO \
				set NUMBER = '%s',\
				CARCOLOR = %d,\
				FACTORY =%d,\
				TYPE = %d,\
				TYPE_DETAIL = %d,\
				CARNUMBER_TYPE = %d,\
				PECCANCY_KIND = %d,\
				SPEED = %d, \
				TIME = '%s', \
				MITIME = %d, \
				DIRECTION = %d,\
				COLOR = %d,\
				CARCOLORSECOND = %d,\
				STATUS = %d, \
				ROAD = %d \
				where ID = %d",\
				strCarNum.c_str(),plate.uCarColor1,plate.uCarBrand,plate.uType,plate.uTypeDetail, plate.uPlateType, \
				plate.uViolationType,plate.uSpeed,strTime.c_str(), plate.uMiTime,plate.uDirection,\
				plate.uColor,plate.uCarColor2, plate.uStatusType, plate.uRoadWayID,\
				plate.uSeq);
		#endif
	#endif
	
    String sql(buf);
	printf("--ModifyCarNum buf: %s-\n", sql.c_str());

    if(execSQL(sql)!=0)
    {
        printf("Update NUMBER_PLATE_INFO error! \n");
        return SRIP_ERROR_USER_FAILE;
    }

	sprintf(buf,"select * from NUMBER_PLATE_INFO where ID = %d",plate.uSeq);
    String sqlSel(buf);

    MysqlQuery q = execQuery(sqlSel);
	
    if(!q.eof())
    {
        string strPicPath = q.getStringFileds("PICPATH");
		string strPic = GetImageByPath(strPicPath);

		//string strDeviceId = q.getStringFileds("DEVICE_ID");

		int uType = q.getIntFileds("TYPE");
		int  uTypeDetail = q.getIntFileds("TYPE_DETAIL");
		int nChannel = q.getIntFileds("CHANNEL");
		int uCarBrand = q.getIntFileds("FACTORY");
		int uCarBrandDetail = q.getIntFileds("SUBFACTORY");

		string strPlace = GetPlace(nChannel);
		string strDeviceId = GetDeviceByChannelId(nChannel);

		int nDirection = g_skpDB.GetDirection(nChannel);
		string strDirection = ::GetDirection(nDirection);

		//LogNormal("strPic.size()=%d",strPic.size());
		if(strPic.size() > 0)
		{
			CxImage image;
			image.Decode((unsigned char*)(strPic.c_str()),strPic.size(),3);//先解码
			if(image.IsValid())
			{
					int uPicWidth = image.GetWidth();
					int uPicHeight = image.GetHeight();

					//LogNormal("uPicWidth=%d,uPicHeight=%d",uPicWidth,uPicHeight);
					UINT32 uImageSize = uPicWidth*uPicHeight*3;
					unsigned char* pImageData = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
					unsigned char* pJpgImage = (unsigned char*)calloc(uImageSize,sizeof(unsigned char));
					
					for(int i = 0;i < uPicHeight;i++)
					{
							if( i > uPicHeight - g_PicFormatInfo.nExtentHeight)
							{
								memset(pImageData+i*(uPicWidth)*3,0,uPicWidth*3);
							}
							else
							{
								memcpy(pImageData+i*(uPicWidth)*3,image.GetBits(i),uPicWidth*3);
							}
					}

					IplImage *pImage = cvCreateImageHeader(cvSize(uPicWidth,uPicHeight),8,3);
					cvSetData(pImage,pImageData,pImage->widthStep);
					
					//叠加文字
					CvxText cvText;
					cvText.Init(g_PicFormatInfo.nFontSize);
					wchar_t wchOut[255] = {'\0'};
					char chOut[255] = {'\0'};

					int nWidth = 10;
					int nHeight = uPicHeight-g_PicFormatInfo.nExtentHeight/2-5;

					//设备编号
					sprintf(chOut,"设备编号:%s 地点名称:%s 方向:%s 车道编号:%d 车牌号码:%s",strDeviceId.c_str(),strPlace.c_str(),strDirection.c_str(),plate.uRoadWayID,strCarNum.c_str());
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					
					//时间
					std::string strTime;
					strTime = GetTime(plate.uTime,0);
					//号牌颜色
					std::string strPlateColor = GetPlateColor(plate.uColor);
					 //车身颜色
					std::string strCarColor = GetObjectColor(plate.uCarColor1);

					//LogNormal("uType=%d,uTypeDetail=%d",uType,uTypeDetail);
					//车辆类型
					if(uTypeDetail == 1)
					{
						uTypeDetail = 6;
					}
					else if(uTypeDetail == 2)
					{
						uTypeDetail = 8;
					}
					else if(uTypeDetail == 3)
					{
						uTypeDetail = 12;
					}
					else if(uTypeDetail == 4)
					{
						uTypeDetail = 7;
					}
					else if(uTypeDetail == 5)
					{
						uTypeDetail = 11;
					}
					else if(uTypeDetail == 6)
					{
						uTypeDetail = 13;
					}
					std::string strCarType = GetDetailCarType(uType,uTypeDetail,0);
					 //车标
					CBrandSusection BrandSub;
					//std::string strCarLabel = BrandSub.GetCarLabelText(plate.uCarBrand);
					std::string strCarLabel = BrandSub.GetCarLabelText(uCarBrand+uCarBrandDetail);

					string strPlace(plate.chPlace);//临时借用此字段修改车标细分
					if(strPlace.size() <= 0)
					{
						sprintf(chOut,"时间:%s.%03d 速度:%dkm/h 车身颜色:%s 号牌颜色:%s 车辆类型:%s 车标:%s",strTime.c_str(),plate.uMiTime,plate.uSpeed,strCarColor.c_str(),strPlateColor.c_str(),strCarType.c_str(),strCarLabel.c_str());
					}
					else
					{
						#ifdef GLOBALCARLABEL
						sprintf(chOut,"时间:%s.%03d 速度:%dkm/h 车身颜色:%s 号牌颜色:%s 车辆类型:%s 车标:%s",strTime.c_str(),plate.uMiTime,plate.uSpeed,strCarColor.c_str(),strPlateColor.c_str(),strCarType.c_str(),strPlace.c_str());
						#else
						sprintf(chOut,"时间:%s.%03d 速度:%dkm/h 车身颜色:%s 号牌颜色:%s 车辆类型:%s 车标:%s",strTime.c_str(),plate.uMiTime,plate.uSpeed,strCarColor.c_str(),strPlateColor.c_str(),strCarType.c_str(),strCarLabel.c_str());
						#endif
					}
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += g_PicFormatInfo.nExtentHeight/2-5;
					cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					cvText.UnInit();


					int srcstep = 0;
					CxImage image1;
					image1.IppEncode(pImageData,uPicWidth,uPicHeight,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
					if(srcstep > 0)
					{
						//LogNormal("srcstep=%d",srcstep);
						FILE* fp = fopen((char*)strPicPath.c_str(),"wb");
						if(fp != NULL)
						{
							fwrite(pJpgImage,1,srcstep,fp);
							fclose(fp);
						}
					}

					if(pImageData)
					{
						free(pImageData);
						pImageData = NULL;
					}

					if(pJpgImage)
					{
						free(pJpgImage);
						pJpgImage = NULL;
					}

					 if(pImage != NULL)
					{
						cvReleaseImageHeader(&pImage);
						pImage = NULL;
					}
			}
		}

    }
    q.finalize();

    return SRIP_OK;
}

String CSeekpaiDB::SearchSpecialCard(SPECIALCARD& specialcard_item)
{
#ifdef _DEBUG
    printf("==================Enter CSeekpaiDB::SearchSpecialCard()===========!!!\n");
#endif
    /**
    * 1. 记录编号	2.布控单位 3.布控行为 4.布控时间 5.布控截止日期
    * 6.号牌号码	7.号牌种类 8.车辆类型 9.号牌颜色 10.布控种类
    */

    unsigned int uId = specialcard_item.uId;
    String strDepartment(specialcard_item.chDepartment);
    unsigned int uBehavior_Kind = specialcard_item.uBehavior_Kind;
    String strTimeBegin = GetTime(specialcard_item.uBegin_Time);
    String strTimeEnd = GetTime(specialcard_item.uEnd_Time);
    String strCarNum(specialcard_item.chText);
    unsigned int uCarNumType = specialcard_item.uCarNumType;
    unsigned int uCarType = specialcard_item.uCarType;
    unsigned int uCarNumColor = specialcard_item.uColor;
    unsigned int uKind = specialcard_item.uKind;

    char bufDepartment[80];
    char bufBehavior_Kind[80];
    char bufCarNum[80];
    char bufCarNumType[80];
    char bufCarType[80];
    char bufCarNumColor[80];
    char bufKind[80];

    memset(bufDepartment, 0, 80);
    memset(bufBehavior_Kind, 0, 80);
    memset(bufCarNum, 0, 80);
    memset(bufCarNumType, 0, 80);
    memset(bufCarType, 0, 80);
    memset(bufCarNumColor, 0, 80);
    memset(bufKind, 0, 80);


    if(strDepartment.size() > 0)
    {
        sprintf(bufDepartment, "and DEPARTMENT like '%%%s%%' ", strDepartment.c_str());
    }
    if(uBehavior_Kind != 1000)
    {
        sprintf(bufBehavior_Kind, "and BEHAVIOR_KIND = %d ", uBehavior_Kind);
    }
    if(strCarNum.size() > 0)
    {
        sprintf(bufCarNum, "and NUMBER like '%s%%' ", strCarNum.c_str());
    }
    if(uCarNumType != 1000)
    {
        sprintf(bufCarNumType, "and CARNUMBER_TYPE = %d ", uCarNumType);
    }
    if(uCarType != 1000)
    {
        sprintf(bufCarType, "and CAR_TYPE = %d ", uCarType);
    }
    if(uCarNumColor != 1000)
    {
        sprintf(bufCarNumColor, "and COLOR = %d ", uCarNumColor);
    }

    sprintf(bufKind, "and KIND = %d ", uKind);

    String sDepartment(bufDepartment);
    String sBehavior_Kind(bufBehavior_Kind);
    String sCarNum(bufCarNum);
    String sCarNumType(bufCarNumType);
    String sCarType(bufCarType);
    String sCarNumColor(bufCarNumColor);
    String sKind(bufKind);

    //增加查询条件
    char bufTemp[BUFF_SIZE];
    sprintf (bufTemp, "%s%s%s%s%s%s%s", \
             sDepartment.c_str(), sBehavior_Kind.c_str(), sCarNum.c_str(), sCarNumType.c_str(), sCarType.c_str(), \
             sCarNumColor.c_str(), sKind.c_str() );

    String strTemp(bufTemp);

//////////////////////
    printf("strTemp=%s,strTemp.size()=%d\n",strTemp.c_str(),strTemp.size());

    printf("strCarNum=%s,strCarNum.size()=%d\n",strCarNum.c_str(),strCarNum.size());

    String response;

    char buf[BUFF_SIZE]= {0};

    //获得查询数量
    int nCount=0;
    char bufCount[BUFF_SIZE];
    memset(bufCount, 0, BUFF_SIZE);

    if(uId == 0)
    {
        sprintf(buf,"Select ID, DEPARTMENT, BEHAVIOR_KIND, BEGIN_TIME, END_TIME, \
					NUMBER, CARNUMBER_TYPE, CAR_TYPE, COLOR, KIND \
                    from SPECIAL_CARD_INFO \
                    where BEGIN_TIME >= '%s' and BEGIN_TIME <= '%s' \
                    %s ", \
                strTimeBegin.c_str(),strTimeEnd.c_str(),\
                strTemp.c_str() );


        sprintf(bufCount, "Select count(*) from SPECIAL_CARD_INFO Where BEGIN_TIME >= '%s' and BEGIN_TIME <= '%s'  %s ", \
                strTimeBegin.c_str(),strTimeEnd.c_str(), strTemp.c_str() );

        String sqlCount(bufCount);
#ifdef _DEBUG
        printf("\n=================sqlCount= %s!!!\n", sqlCount.c_str());
#endif
        String sql(bufCount);
        nCount = getIntFiled(sql);

    }
    else
    {
        sprintf(buf,"Select ID, DEPARTMENT, BEHAVIOR_KIND, BEGIN_TIME, END_TIME, \
					NUMBER, CARNUMBER_TYPE, CAR_TYPE, COLOR, KIND \
                    from SPECIAL_CARD_INFO \
                    where ID = %d ", \
                uId );

        sprintf(bufCount, "Select count(*) from SPECIAL_CARD_INFO Where ID = %d", uId);

        String sqlCount(bufCount);
#ifdef _DEBUG
        printf("\n=================sqlCount= %s!!!\n", sqlCount.c_str());
#endif
        String sql(bufCount);
        nCount = getIntFiled(sql);
    }

    String sql2(buf);

#ifdef _DEBUG
    printf("\n=================sql2= %s!!!\n", sql2.c_str());
#endif
    MysqlQuery q = execQuery(sql2);
    printf("nCount=%d, numFileds=%d\n",nCount,q.numFileds());

    response.append((char*)&nCount,sizeof(nCount));

    while(!q.eof())
    {
        /**
        * 0.记录编号 1.布控单位 2.布控行为 3.布控时间 4.布控截止日期
        * 5.号牌号码 6.号牌种类 7.车辆类型 8.号牌颜色 9.布控种类
        */
        SPECIALCARD specialcard;

        specialcard.uId = q.getUnIntFileds("ID");

        sDepartment.erase(0, sDepartment.size());
        sDepartment = q.getStringFileds("DEPARTMENT");
        memcpy(specialcard.chDepartment, sDepartment.c_str(), sDepartment.size());

        specialcard.uBehavior_Kind = q.getIntFileds("BEHAVIOR_KIND");

        String sBegin_Time = q.getStringFileds("BEGIN_TIME");
        specialcard.uBegin_Time = MakeTime(sBegin_Time);

        String sEnd_Time = q.getStringFileds("END_TIME");
        specialcard.uEnd_Time = MakeTime(sEnd_Time);

        sCarNum.erase(0, sCarNum.size());
        sCarNum = q.getStringFileds("NUMBER");
        memcpy(specialcard.chText, sCarNum.c_str(), sCarNum.size());

        specialcard.uCarNumType = q.getIntFileds("CARNUMBER_TYPE");

        specialcard.uCarType = q.getIntFileds("CAR_TYPE");

        specialcard.uColor = q.getIntFileds("COLOR");

        specialcard.uKind = q.getIntFileds("KIND");

        response.append((char*)&specialcard,sizeof(SPECIALCARD));
        q.nextRow();
    }

    q.finalize();

#ifdef _DEBUG
    printf("==================Out CSeekpaiDB::SearchSpecialCard()===========!!!\n");
#endif
    return response;

}

/**
* 函数简介: 车牌比对，决定是否报警，
* 输入参数: 实时车牌号
* 返回值: 是否报警 0:不报警 1:报警
* other: 以后考虑支持车牌的模糊形式，例如：沪M?8*
*/
int CSeekpaiDB::IsSpecialCard(String &strNum)
{
#ifdef _DEBUG
    printf("\n==========Enter CSeekpaiDB::IsSpecialCar ========!!!\n");
#endif
    int nAlarmKind = 0;
    char bufCount[BUFF_SIZE]= {0};

    int nCount = 0; //黑白名单记录数量

    String strTime = GetTimeCurrent();

    sprintf(bufCount,"Select Kind from SPECIAL_CARD_INFO \
			where END_TIME >= '%s' and NUMBER = '%s' ",\
            strTime.c_str(), strNum.c_str());

    String sql2(bufCount);
#ifdef _DEBUG
    printf("\n==========CSeekpaiDB::IsSpecialCar d========sql= %s!!!\n", sql2.c_str());
#endif
    MysqlQuery q = execQuery(sql2);

    if(!q.eof())
    {
        nCount  = q.numFileds();
        if(nCount > 0)
        {
            nAlarmKind = q.getIntFileds(0);
        }
    }
    q.finalize();


#ifdef _DEBUG
    printf("\n==========Out CSeekpaiDB::IsSpecialCar ========!!!\n");
#endif
    return nAlarmKind;
}

//根据不同中心端编号,更新记录状态
void CSeekpaiDB::UpdateRecordStatus(unsigned int uCmd,unsigned int uSeq,unsigned int nId, bool bObject)
{
	char buf[BUFF_SIZE]= {0};

	if(1 == nId)
	{
		if(uCmd == MIMAX_PLATE_REP)
		{
			sprintf(buf,"update NUMBER_PLATE_INFO set STATUS = 1 where ID = %u",uSeq);
		}
		else if(uCmd == MIMAX_EVENT_REP)
		{
			if(bObject)
			{
				sprintf(buf,"update NUMBER_PLATE_INFO set STATUS = 1 where ID = %u",uSeq);
			}
			else
			{
				printf("=========UpdateRecordStatus=======uSeq=%u=\n",uSeq);
				sprintf(buf,"update TRAFFIC_EVENT_INFO set STATUS = 1 where ID = %u",uSeq);
			}

		}
		else if(uCmd == MIMAX_STATISTIC_REP)
		{
			sprintf(buf,"update TRAFFIC_STATISTIC_INFO set STATUS = 1 where ID = %u",uSeq);
		}
		else if(uCmd == PLATE_LOG_REP || uCmd == EVENT_LOG_REP)
		{
			sprintf(buf,"update SYSTEM_EVENT_INFO set STATUS = 1 where ID = %u",uSeq);
		}
	}//End of if(1 == nId)
	else
	{
		if(uCmd == MIMAX_PLATE_REP)
		{
			sprintf(buf,"update NUMBER_PLATE_INFO set STATUS%d = 1 where ID = %u", nId, uSeq);
		}
		else if(uCmd == MIMAX_EVENT_REP)
		{
			if(bObject)
			{
				sprintf(buf,"update NUMBER_PLATE_INFO set STATUS%d = 1 where ID = %u", nId, uSeq);
			}
			else
			{
				//printf("=========UpdateRecordStatus=======uSeq=%u=\n",uSeq);
				sprintf(buf,"update TRAFFIC_EVENT_INFO set STATUS%d = 1 where ID = %u", nId, uSeq);
			}

		}
		else if(uCmd == MIMAX_STATISTIC_REP)
		{
			sprintf(buf,"update TRAFFIC_STATISTIC_INFO set STATUS%d = 1 where ID = %u", nId, uSeq);
		}
		else if(uCmd == PLATE_LOG_REP || uCmd == EVENT_LOG_REP)
		{
			sprintf(buf,"update SYSTEM_EVENT_INFO set STATUS%d = 1 where ID = %u", nId, uSeq);
		}
	}	

    String sqlModify(buf);
	
	#ifdef LS_DEBUG
		LogNormal("sql:%s", buf);
	#endif

	if(sqlModify.size() > 0)
	{
		if(g_skpDB.execSQL(sqlModify)!=0)
		{
			LogError("UpdateRecordStatus error\n");
		}
	}    
}

//更新记录状态
void CSeekpaiDB::UpdateRecordStatus(unsigned int uCmd,std::list<unsigned int>& listSeq,unsigned int nId)
{
	if(listSeq.size() > 0)
	{
		char buf[8192];
		memset(buf, 0, 8192);
		
		int nCount = 0;
		string strSeq("(");
		std::list<unsigned int>::iterator it = listSeq.begin();
		while(it != listSeq.end())
		{
			unsigned int uSeq = *it;

			char bufSeq[1024] = {0};
			if(nCount == 0)
			{
				sprintf(bufSeq,"%u",uSeq);
			}
			else
			{
				sprintf(bufSeq,",%u",uSeq);
			}
			string strTmp(bufSeq);
			strSeq += strTmp;
			
			nCount++;
			it++;
		}
		strSeq += ")";

		if(1 == nId)
		{
			if(uCmd == MIMAX_PLATE_REP)
			{
				sprintf(buf, "update NUMBER_PLATE_INFO set STATUS = 1 where ID in %s;",strSeq.c_str());
			}
			else if(uCmd == MIMAX_EVENT_REP)
			{
				sprintf(buf,"update TRAFFIC_EVENT_INFO set STATUS = 1 where ID in %s;",strSeq.c_str());
			}
			else if(uCmd == MIMAX_STATISTIC_REP)
			{
				sprintf(buf,"update TRAFFIC_STATISTIC_INFO set STATUS = 1 where ID in %s;",strSeq.c_str());
			}
			else if(uCmd == PLATE_LOG_REP || uCmd == EVENT_LOG_REP)
			{
				sprintf(buf,"update SYSTEM_EVENT_INFO set STATUS = 1 where ID in %s;",strSeq.c_str());
			}
		}
		else
		{
			if(uCmd == MIMAX_PLATE_REP)
			{
				sprintf(buf,"update NUMBER_PLATE_INFO set STATUS%d = 1 where ID in %s;", nId, strSeq.c_str());
			}
			else if(uCmd == MIMAX_EVENT_REP)
			{
				sprintf(buf,"update TRAFFIC_EVENT_INFO set STATUS%d = 1 where ID in %s;", nId, strSeq.c_str());
			}
			else if(uCmd == MIMAX_STATISTIC_REP)
			{
				sprintf(buf,"update TRAFFIC_STATISTIC_INFO set STATUS%d = 1 where ID in %s;", nId, strSeq.c_str());
			}
			else if(uCmd == PLATE_LOG_REP || uCmd == EVENT_LOG_REP)
			{
				sprintf(buf,"update SYSTEM_EVENT_INFO set STATUS%d = 1 where ID in %s;", nId, strSeq.c_str());
			}
		}
		g_skpDB.execSQL(string(buf));
	}

	return;
}

//获取裁剪区域坐标(数据库中存放的是jpg图像坐标)
RegionCoordinate CSeekpaiDB::GetClipRegionCoordinate(int nChannel)
{
    RegionCoordinate ord;

    char buf[BUFF_SIZE]= {0};

    sprintf(buf,"select  ViolationArea,EventArea,TrafficSignalArea from ROAD_SETTING_INFO where Channel = %d",nChannel);

    String sql(buf);
    MysqlQuery q = execQuery(sql);

    std::stringstream stream;
    std::string sub_str;
    char buffer[256]= {0};
    int i=0,j=0,k=0;
    float x,y;

    if(!q.eof())
    {
        //违章检测区域
        String strViolationArea = q.getStringFileds("ViolationArea");
        printf("strViolationArea = %s\r\n",strViolationArea.c_str());

        j=0;
        memset(buffer,0,sizeof(buffer));
        stream.clear();
        stream.str(strViolationArea);

        while(getline(stream,sub_str,')'))		/* Get a segment */
        {
            sub_str.erase(0,1);		/* Get rid of the first character '(' */
            k=0;
            //printf("%s\r\n",sub_str.c_str());
            std::stringstream streamTemp(sub_str);
            std::string subTemp;
            while(getline(streamTemp,subTemp,'|'))
            {
                strcpy(buffer, subTemp.c_str());
                sscanf(buffer,"%f,%f",&x,&y);

                if(k == 0)
                {
                    ord.uViolationX = x;
                    ord.uViolationY = y;
                }
                else if(k == 2)
                {
                    ord.uViolationWidth  = x - ord.uViolationX;
                    ord.uViolationHeight = y - ord.uViolationY;
                }

                k++;
            }

            j++;
        }

        //事件检测区域
        String strEventArea = q.getStringFileds("EventArea");
        printf("strEventArea = %s\r\n",strEventArea.c_str());

        j=0;
        memset(buffer,0,sizeof(buffer));
        stream.clear();
        stream.str(strEventArea);

        while(getline(stream,sub_str,')'))		/* Get a segment */
        {
            sub_str.erase(0,1);		/* Get rid of the first character '(' */
            k=0;
            //printf("%s\r\n",sub_str.c_str());
            std::stringstream streamTemp(sub_str);
            std::string subTemp;
            while(getline(streamTemp,subTemp,'|'))
            {
                strcpy(buffer, subTemp.c_str());
                sscanf(buffer,"%f,%f",&x,&y);

                if(k == 0)
                {
                    ord.uEventX = x;
                    ord.uEventY = y;
                }
                else if(k == 2)
                {
                    ord.uEventWidth  = x - ord.uEventX;
                    ord.uEventHeight = y - ord.uEventY;
                }

                k++;
            }

            j++;
        }

        //红灯检测区域
        String strTrafficSignalArea = q.getStringFileds("TrafficSignalArea");
        printf("strTrafficSignalArea = %s\r\n",strTrafficSignalArea.c_str());

        j=0;
        memset(buffer,0,sizeof(buffer));
        stream.clear();
        stream.str(strTrafficSignalArea);

        while(getline(stream,sub_str,')'))		/* Get a segment */
        {
            sub_str.erase(0,1);		/* Get rid of the first character '(' */
            k=0;
            //printf("%s\r\n",sub_str.c_str());
            std::stringstream streamTemp(sub_str);
            std::string subTemp;
            while(getline(streamTemp,subTemp,'|'))
            {
                strcpy(buffer, subTemp.c_str());
                sscanf(buffer,"%f,%f",&x,&y);

                if(k == 0)
                {
                    ord.uTrafficSignalX = x;
                    ord.uTrafficSignalY = y;
                }
                else if(k == 2)
                {
                    ord.uTrafficSignalWidth  = x - ord.uTrafficSignalX;
                    ord.uTrafficSignalHeight = y - ord.uTrafficSignalY;
                }

                k++;
            }

            j++;
        }
    }
    q.finalize();

    return ord;
}


//获取日期图片路径
String CSeekpaiDB::GetDatePicPath(time_t uTime)
{
    struct tm *newTime,timenow;
    newTime = &timenow;
    localtime_r( &uTime,newTime );
    char buf[BUFF_SIZE]= {0};
    sprintf(buf, "%4d-%02d-%02d 00:00:00", newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday);
    String strBeginTime(buf);

    sprintf(buf, "%4d-%02d-%02d 23:59:59", newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday);
    String strEndTime(buf);

    String strPicPath;

    //先从车牌数据库中查找
    if(strPicPath.size()<=0)
    {
        sprintf(buf,"select PICPATH from NUMBER_PLATE_INFO where TIME = (select min(TIME) from NUMBER_PLATE_INFO where TIME >='%s' and TIME <='%s')",strBeginTime.c_str(),strEndTime.c_str());
        String sqlPlatePic(buf);
        //printf("sqlPlatePic.c_str()=%s\n",sqlPlatePic.c_str());
        MysqlQuery q = execQuery(sqlPlatePic);

        if(!q.eof())
        {
            strPicPath = q.getStringFileds(0);
        }
        q.finalize();
    }

    //再从事件数据库中查找
    if(strPicPath.size()<=0)
    {
        sprintf(buf,"select PICPATH from TRAFFIC_EVENT_INFO where BEGIN_TIME = (select min(BEGIN_TIME) from TRAFFIC_EVENT_INFO where BEGIN_TIME >='%s' and BEGIN_TIME <='%s')",strBeginTime.c_str(),strEndTime.c_str());
        String sqlEventPic(buf);
        //printf("sqlEventPic.c_str()=%s\n",sqlEventPic.c_str());
        MysqlQuery q = execQuery(sqlEventPic);

        if(!q.eof())
        {
            strPicPath = q.getStringFileds(0);
        }
        q.finalize();
    }
    return strPicPath;
}

//zhangyaoyao:获取对应通道当前最大的event_id
long CSeekpaiDB::mvGetLastEventId(int nChannelId)
{
    char szBuff[BUFF_SIZE] = {0};
    long lnEventId = -1;

    sprintf(szBuff, "Select max(EVENT_ID) from TRAFFIC_EVENT_INFO where CHANNEL=%d;", nChannelId);

    MysqlQuery q = execQuery(string(szBuff));
    if(!q.eof())
    lnEventId = q.getLongFileds(0);
    q.finalize();

    //printf("szBuff=%s\nlnEventId=%d\n", szBuff, lnEventId);
    return lnEventId;
}

//zhangyaoyao:获取对应通道当前最大的车牌表
long CSeekpaiDB::mvGetLastCarNumId(int nChannelId)
{
    char szBuff[BUFF_SIZE] = {0};
    long lnCarNumId = -1;

    sprintf(szBuff, "Select max(ID) from NUMBER_PLATE_INFO where CHANNEL=%d;", nChannelId);

    MysqlQuery q = execQuery(string(szBuff));
    if(!q.eof())
    lnCarNumId = q.getLongFileds(0);
    q.finalize();

    return lnCarNumId;
}

//zhangyaoyao:更新事件event_id关联的车牌记录ID(2010-01-26,添加更新STATUS=1)
bool CSeekpaiDB::mvUpdatePlateIdOfEvent(int nChannelId, long lnEventId, long lnPlateId)
{
    char szBuff[BUFF_SIZE] = {0};

    sprintf(szBuff, "update TRAFFIC_EVENT_INFO set STATUS=1,PLATE_ID=%lld where CHANNEL=%d and EVENT_ID=%lld;", lnPlateId, nChannelId, lnEventId);

    return (execSQL(string(szBuff))==0);
}

//保存图片编号
int CSeekpaiDB::SavePicID(unsigned int uPicId,unsigned int uVideoId,int nType)
{
    char szBuff[BUFF_SIZE] = {0};

    if(nType == 0)
    {
        sprintf(szBuff, "update PIC_INFO set PIC_ID=%u", uPicId);
    }
    else
    {
        sprintf(szBuff, "update PIC_INFO set PIC_ID=%u, VIDEO_ID = %u", uPicId,uVideoId);
    }

    return (execSQL(string(szBuff))==0);
}

//特征比对查询
String CSeekpaiDB::GetObjectFeature(SEARCH_ITEM& search_item, int nPageSize)
{
//for test
    printf("==================Enter CSeekpaiDB::GetTextureHigh()=======search_item.chText==%s==!!!\n",search_item.chText);

    String strTimeBeg = GetTime(search_item.uBeginTime);
    String strTimeEnd = GetTime(search_item.uEndTime);

    int nPage = search_item.uPage;

    bool bCarNum = true;
    if(search_item.chText[0]=='*')
        bCarNum = false;

    String strCarNum(&search_item.chText[4]);

//////////////////////
    String response;

    if(nPage<1||nPageSize<0)
    {
        return response;
    }

    char buf[BUFF_SIZE*3]= {0};

    if(bCarNum)
    {
        sprintf(buf,"Select ID, NUMBER, SMALLPICPATH, PICPATH,TYPE,TYPE_DETAIL,TIME,CARCOLOR,SPEED from NUMBER_PLATE_INFO \
                where TIME >= '%s' and TIME <= '%s' and NUMBER like '%%%s'\
                ORDER BY ID limit %d,%d",\
                strTimeBeg.c_str(),strTimeEnd.c_str(),strCarNum.c_str(),(nPage-1)*nPageSize,nPageSize);
    }
    else
    {
        sprintf(buf,"Select ID, NUMBER, SMALLPICPATH, PICPATH,TYPE,TYPE_DETAIL,TIME,CARCOLOR,SPEED from NUMBER_PLATE_INFO \
                where TIME >= '%s' and TIME <= '%s' and NUMBER = '*******'\
                ORDER BY ID limit %d,%d",\
                strTimeBeg.c_str(),strTimeEnd.c_str(),(nPage-1)*nPageSize,nPageSize);
    }

    //获得查询数量
    int nCount=0;
    char bufCount[BUFF_SIZE*2];
    memset(bufCount, 0, BUFF_SIZE);

    if(bCarNum)
    {
        sprintf(bufCount, "Select count(*) from NUMBER_PLATE_INFO \
                Where TIME >= '%s' and TIME <= '%s' and NUMBER like '%%%s'", \
                strTimeBeg.c_str(),strTimeEnd.c_str(),strCarNum.c_str());
    }
    else
    {
        sprintf(bufCount, "Select count(*) from NUMBER_PLATE_INFO \
                Where TIME >= '%s' and TIME <= '%s' and NUMBER = '*******'", \
                strTimeBeg.c_str(),strTimeEnd.c_str());
    }

    String sqlCount(bufCount);
#ifdef _DEBUG
    printf("\n=================sqlCount= %s!!!\n", sqlCount.c_str());
#endif
    String sql(bufCount);
    nCount = getIntFiled(sql);
    printf("nCount=%d\n",nCount);

    if(nPage==1)
    {
        int nNum=nCount % nPageSize;
        if(nNum>0)
            nCount=nCount/nPageSize+1;
        else
            nCount=nCount/nPageSize;
    }

    //String sql(buf);
    String sql2(buf);

    //test
    printf("\n=================sql2= %s!!!\n", sql2.c_str());

    MysqlQuery q = execQuery(sql2);
    printf("nCount=%d, numFileds=%d\n",nCount,q.numFileds());

    response.append((char*)&nCount,sizeof(nCount));

    while(!q.eof())
    {
        RECORD_PLATE_TEXTURE plate_texture;
        plate_texture.uSeq=q.getUnIntFileds("ID");

        String text = q.getStringFileds("NUMBER");
        memcpy(plate_texture.chText,text.c_str(),text.size());

        String strPath = q.getStringFileds("SMALLPICPATH");
//		strPath.erase(0,g_strPic.size());
        memcpy(plate_texture.chSmallPicPath,strPath.c_str(),strPath.size());

        strPath = q.getStringFileds("PICPATH");
//		strPath.erase(0,g_strPic.size());
        memcpy(plate_texture.chPicPath,strPath.c_str(),strPath.size());


        //把特征信息也插入到每条车牌信息后面
        if(!GetTextureFromPicFile(strPath, plate_texture.nTexture))
        {
            printf("\n============Can't get the Texture Info!!!\n");
        }
        plate_texture.nType = q.getIntFileds("TYPE");
        plate_texture.nDetailedType = q.getIntFileds("TYPE_DETAIL");
        String strTime = q.getStringFileds("TIME");
        plate_texture.uTimestamp = MakeTime(strTime);
        plate_texture.nColor = q.getIntFileds("CARCOLOR");
        plate_texture.fSpeed = q.getIntFileds("SPEED");

        response.append((char*)&plate_texture,sizeof(plate_texture));

        q.nextRow();
    }

    q.finalize();

    //for test
    printf("==================Out CSeekpaiDB::GetTextureHigh()====response.size()=%d======!!!\n",response.size());

    return response;
}

//设置预置位
int CSeekpaiDB::SetPreSet(int nPreSet,int nChannelId)
{
    char szBuff[BUFF_SIZE] = {0};

    sprintf(szBuff, "update CHAN_INFO set CHAN_PRESET = %d where CHAN_ID=%d", nPreSet, nChannelId);
    printf("=========szBuff=%s\n",szBuff);
    return (execSQL(string(szBuff))==0);
}

//获取预置位
int CSeekpaiDB::GetPreSet(int nChannelId)
{
    int nPreSet = -1;
    char szBuff[BUFF_SIZE] = {0};
    sprintf(szBuff, "select  CHAN_PRESET from CHAN_INFO where CHAN_ID=%d",nChannelId);
    String sql(szBuff);
    MysqlQuery q = execQuery(sql);
    if(!q.eof())
    {
        nPreSet = q.getIntFileds(0);
    }
    q.finalize();

    return nPreSet;
}

//获得录像时长
UINT32 CSeekpaiDB::GetVideoTimeSize(int nChannelId)
{
	UINT32 nVideoTime = -1;
	char szBuff[BUFF_SIZE] = {0};
	sprintf(szBuff, "select  CHAN_EVENTCAPTURETIME from CHAN_INFO where CHAN_ID=%d",nChannelId);
	String sql(szBuff);
	MysqlQuery q = execQuery(sql);
	if(!q.eof())
	{
		nVideoTime = q.getUnIntFileds(0);
	}
	q.finalize();

	return nVideoTime;
}

//获取检测类型
CHANNEL_DETECT_KIND CSeekpaiDB::GetDetectKind(int nChannelId)
{
    CHANNEL_DETECT_KIND uDetectKind = DETECT_NONE;
    char szBuff[BUFF_SIZE] = {0};
    sprintf(szBuff, "select  CHAN_DETECT_KIND from CHAN_INFO where CHAN_ID=%d",nChannelId);
    String sql(szBuff);
    MysqlQuery q = execQuery(sql);
    if(!q.eof())
    {
        uDetectKind = (CHANNEL_DETECT_KIND)q.getIntFileds(0);
    }
    q.finalize();

    return uDetectKind;
}

//获取monitor编号--MONITOR_ID
int CSeekpaiDB::GetMonitorID(int nChannelId)
{
    int nMonitorID = -1;
    char szBuff[BUFF_SIZE] = {0};
    sprintf(szBuff, "select  MONITOR_ID from CHAN_INFO where CHAN_ID=%d",nChannelId);
    String sql(szBuff);
    MysqlQuery q = execQuery(sql);
    if(!q.eof())
    {
        nMonitorID = q.getIntFileds(0);
    }
    q.finalize();

    return nMonitorID;
}

//获取相机编号
int CSeekpaiDB::GetCameraID(int nChannelId)
{		
    int nCameraID = -1;
    char szBuff[BUFF_SIZE] = {0};
    sprintf(szBuff, "select  CAMERA_ID from CHAN_INFO where CHAN_ID=%d",nChannelId);
    String sql(szBuff);
    MysqlQuery q = execQuery(sql);
    if(!q.eof())
    {
        nCameraID = q.getIntFileds(0);
    }
    q.finalize();

    return nCameraID;
}


//获取地点
String CSeekpaiDB::GetPlace(int nChannelId)
{
    String strPlace;

    char szBuff[BUFF_SIZE] = {0};
    sprintf(szBuff, "select  CHAN_PLACE from CHAN_INFO where CHAN_ID=%d",nChannelId);
    String sql(szBuff);
    MysqlQuery q = execQuery(sql);
    if(!q.eof())
    {
        strPlace = q.getStringFileds(0);
    }
    q.finalize();

    return strPlace;

}

//获取地点
String CSeekpaiDB::GetPlaceByCamID(int nCameraId)
{
	String strPlace;

	char szBuff[BUFF_SIZE] = {0};
	sprintf(szBuff, "select  CHAN_PLACE from CHAN_INFO where CAMERA_ID=%d",nCameraId);
	String sql(szBuff);
	MysqlQuery q = execQuery(sql);
	if(!q.eof())
	{
		strPlace = q.getStringFileds(0);
	}
	q.finalize();

	return strPlace;

}

//获取方向
int CSeekpaiDB::GetDirection(int nChannelId)
{
    int nDirection = 0;

    char szBuff[BUFF_SIZE] = {0};
    sprintf(szBuff, "select  CHAN_DIRECTION from CHAN_INFO where CHAN_ID=%d",nChannelId);
    String sql(szBuff);
    MysqlQuery q = execQuery(sql);
    if(!q.eof())
    {
        nDirection = q.getIntFileds(0);
    }
    q.finalize();

    return nDirection;
}

//获取车牌历史记录(nDataType == 0全部发送，1：只是卡口记录，2：只是违章)
bool CSeekpaiDB::GetPlateHistoryRecord(StrList& strListRecord,int nDataType, unsigned int nId)
{
	#ifdef NO_SEND_KAKOU_DATA
	return false;
	#endif
	
    unsigned int uTimeStamp = GetTimeStamp()-600;//600秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
    String strTime = GetTime(uTimeStamp);
	unsigned int uMonSecs = 10*24*3600;
	unsigned int uBeginTimeStamp = GetTimeStamp()-uMonSecs;
	if (uBeginTimeStamp < 0)
	{
		uBeginTimeStamp = 0;
	}
	string strBeginTime = GetTime(uBeginTimeStamp);

    char buf[1024]= {0};
	if(1 == nId)
	{
		if(g_nSendViolationOnly == 1)
		{
			sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and PECCANCY_KIND > 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",strTime.c_str(),strBeginTime.c_str());
		}
		else
		{
			if(nDataType == 1)
			{
				sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and PECCANCY_KIND == 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",strTime.c_str(),strBeginTime.c_str());
			}
			else if(nDataType == 2)
			{
				sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and PECCANCY_KIND > 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",strTime.c_str(),strBeginTime.c_str());
			}
			else
			{
				sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS = 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",strTime.c_str(),strBeginTime.c_str());
			}
		}
	}//end of if(1 == nId)
	else{
		if(g_nSendViolationOnly == 1)
		{
			sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS%d = 0 and PECCANCY_KIND > 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",nId, strTime.c_str(),strBeginTime.c_str());
		}
		else
		{
			if(nDataType == 1)
			{
				sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS%d = 0 and PECCANCY_KIND == 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",nId, strTime.c_str(),strBeginTime.c_str());
			}
			else if(nDataType == 2)
			{
				sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS%d = 0 and PECCANCY_KIND > 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",nId, strTime.c_str(),strBeginTime.c_str());
			}
			else
			{
				sprintf(buf,"Select * from NUMBER_PLATE_INFO where STATUS%d = 0 and TIME <= '%s' and TIME >= '%s' ORDER BY TIME desc limit 100",nId, strTime.c_str(),strBeginTime.c_str());
			}
		}
	}
	
	

    String strSql(buf);
    MysqlQuery q = g_skpDB.execQuery(strSql);
    //printf("plate numFileds=%d \n",q.numFileds());

    while(!q.eof())
    {
		string strRecord("");

        RECORD_PLATE plate;
        plate.uSeq = q.getUnIntFileds("ID");
        string strCarNum = q.getStringFileds("NUMBER");
         if(g_nServerType!=1 && g_nServerType!=5 && g_nServerType!=10 && \
			g_nServerType!=15 && g_nServerType!=18 && g_nServerType!=20 && \
			g_nServerType!=23 && g_nServerType!=26 && g_nServerType!=7&& g_nServerType!=29 && g_nServerType != 30)
        g_skpDB.UTF8ToGBK(strCarNum);
        memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
        ////////////////////////////////////////////////
        plate.uColor = q.getIntFileds("COLOR");
        if(plate.uColor <= 0)
        {
            plate.uColor =  CARNUM_OTHER;
        }
        plate.uCredit = q.getIntFileds("CREDIT");
        int nChannel = q.getIntFileds("CHANNEL");
		plate.uChannelID = nChannel;
        plate.uRoadWayID = q.getIntFileds("ROAD");

        string strTime = q.getStringFileds("TIME");
        plate.uTime = MakeTime(strTime);
        plate.uMiTime = q.getIntFileds("MITIME");
		//LogNormal("plate.uMiTime=%d",plate.uMiTime);

        plate.uSmallPicSize = q.getIntFileds("SMALLPICSIZE");
        plate.uSmallPicWidth = q.getIntFileds("SMALLPICWIDTH");
        plate.uSmallPicHeight = q.getIntFileds("SMALLPICHEIGHT");

        plate.uPicSize = q.getIntFileds("PICSIZE");
        plate.uPicWidth = q.getIntFileds("PICWIDTH");
        plate.uPicHeight = q.getIntFileds("PICHEIGHT");

        plate.uPosLeft = q.getIntFileds("POSLEFT");
        plate.uPosTop = q.getIntFileds("POSTOP");
        plate.uPosRight = q.getIntFileds("POSRIGHT");
        plate.uPosBottom = q.getIntFileds("POSBOTTOM");

        //车牌大图
        string strPicPath = q.getStringFileds("PICPATH");

        ///////////////车身颜色，车辆类型，速度，方向,地点等
        plate.uCarColor1 = q.getIntFileds("CARCOLOR");
        plate.uType = q.getIntFileds("TYPE");

        plate.uSpeed = q.getIntFileds("SPEED");

		std::string strPlace = GetPlace(nChannel);
		memcpy(plate.chPlace,strPlace.c_str(),strPlace.size());

        ///////////////
        plate.uWeight1 = q.getIntFileds("CARCOLORWEIGHT");
        plate.uCarColor2 = q.getIntFileds("CARCOLORSECOND");
        plate.uWeight2 = q.getIntFileds("CARCOLORWEIGHTSECOND");
        plate.uViolationType = q.getIntFileds("PECCANCY_KIND");//事件类型
        plate.uTypeDetail = q.getIntFileds("TYPE_DETAIL");//车型细分
		plate.uDirection = q.getIntFileds("DIRECTION");
		String strVideoPath = q.getStringFileds("VIDEOPATH");
        memcpy(plate.chVideoPath,strVideoPath.c_str(),strVideoPath.size());

		 //第2车牌时间
        string strTime2 = q.getStringFileds("TIMESECOND");
		//printf("strTime2.size()=%d\n",strTime2.size());
        if(strTime2.size() > 8)
            plate.uTime2 = MakeTime(strTime2);
         else
            plate.uTime2 = 0; 
         plate.uMiTime2 = q.getIntFileds("MITIMESECOND");

		strTime = q.getStringFileds("REDLIGHT_BEGIN_TIME");
        plate.uRedLightBeginTime = MakeTime(strTime);
        plate.uRedLightBeginMiTime = q.getIntFileds("REDLIGHT_BEGIN_MITIME");

		strTime = q.getStringFileds("REDLIGHT_END_TIME");
        plate.uRedLightEndTime = MakeTime(strTime);
        plate.uRedLightEndMiTime = q.getIntFileds("REDLIGHT_END_MITIME");

		plate.uLimitSpeed = q.getIntFileds("LIMIT_SPEED");
		plate.uOverSpeed = q.getIntFileds("OVER_SPEED");


		if(plate.uViolationType == DETECT_RESULT_NOCARNUM)
		{
			plate.uViolationType = 0;
		}

        MIMAX_HEADER mHeader;
        mHeader.uCmdFlag = 0x00000002;//此时均设为非实时车牌
        mHeader.uCameraID = GetCameraID(nChannel);
		plate.uCameraId = mHeader.uCameraID;

		if (g_nGongJiaoMode == 1)
		{
			String strLoctionID = q.getStringFileds("LOCTION_ID");
			memcpy(plate.szLoctionID,strLoctionID.c_str(),strLoctionID.size());
			String strKaKouItem = q.getStringFileds("KAKOU_ID");
			memcpy(plate.szKaKouItem,strKaKouItem.c_str(),strKaKouItem.size());
		}
        {
            mHeader.uCmdID = MIMAX_PLATE_REP;

            if(plate.chText[0]=='*')
            {
                if((plate.chText[1]=='-'))//无牌车
                {
					if (12 == g_nServerType)
						memcpy(plate.chText,"00000000",8);
					else
						memcpy(plate.chText,"11111111",8);
                }
                else//未识别出结果
                {
                    memcpy(plate.chText,"00000000",8);
                }
            }
            memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());
            strRecord.append((char*)&plate,sizeof(RECORD_PLATE));
        }
        mHeader.uCmdLen = sizeof(MIMAX_HEADER)+strRecord.size();
        strRecord.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));
        ///////////////////////////////////////
		q.nextRow();
		strListRecord.push_back(strRecord);
	}
    q.finalize();
	
	if(strListRecord.size() <= 0)
	{
		return false;
	}
    return true;
}

//获取事件历史记录
bool CSeekpaiDB::GetEventHistoryRecord(StrList& strListRecord, unsigned int nId)
{
            unsigned int uTimeStamp = GetTimeStamp()-600;//600秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
            string strTime = GetTime(uTimeStamp);
            char buf[1024]={0};

			unsigned int uMonSecs = 7*24*3600;
			unsigned int uBeginTimeStamp = GetTimeStamp()-uMonSecs;
			if (uBeginTimeStamp < 0)
			{
				uBeginTimeStamp = 0;
			}
			string strBeginTime = GetTime(uBeginTimeStamp);
			if(1 == nId)
			{
				sprintf(buf,"Select * from TRAFFIC_EVENT_INFO where STATUS = 0 and BEGIN_TIME <= '%s' and BEGIN_TIME >= '%s' ORDER BY BEGIN_TIME desc limit 100",strTime.c_str(),strBeginTime.c_str());
			}
			else
			{
				sprintf(buf,"Select * from TRAFFIC_EVENT_INFO where STATUS%d = 0 and BEGIN_TIME <= '%s' and BEGIN_TIME >= '%s' ORDER BY BEGIN_TIME desc limit 100",nId, strTime.c_str(),strBeginTime.c_str());
			}

			string strSql(buf);
			MysqlQuery q = g_skpDB.execQuery(strSql);
	    	//printf("event numFileds=%d \n",q.numFileds());

			while(!q.eof())
			{
				string strRecord("");

			    RECORD_EVENT event;
				event.uSeq = q.getUnIntFileds("ID");
				int nChannel = q.getIntFileds("CHANNEL");
		
				{
					event.uChannelID = nChannel;
					string strCarnum("");
					strCarnum = q.getStringFileds("TEXT");
					memcpy(event.chText,strCarnum.c_str(),strCarnum.size());
				}
				event.uRoadWayID = q.getIntFileds("ROAD");
				event.uCode = q.getIntFileds("KIND");
				strTime = q.getStringFileds("BEGIN_TIME");
				event.uEventBeginTime = MakeTime(strTime);
				event.uMiEventBeginTime = q.getIntFileds("BEGIN_MITIME");
				strTime = q.getStringFileds("END_TIME");
				event.uEventEndTime = MakeTime(strTime);
				event.uMiEventEndTime = q.getIntFileds("END_MITIME");

				event.uPicSize = q.getIntFileds("PICSIZE");
				event.uPicWidth = q.getIntFileds("PICWIDTH");
				event.uPicHeight = q.getIntFileds("PICHEIGHT");

				event.uPosX = q.getIntFileds("POSX");
				event.uPosY = q.getIntFileds("POSY");

				//事件快照路径
				String strPicPath = q.getStringFileds("PICPATH");
                memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size());

				strTime = q.getStringFileds("BEGIN_VIDEO_TIME");
				event.uVideoBeginTime = MakeTime(strTime);
				event.uMiVideoBeginTime = q.getIntFileds("BEGIN_VIDEO_MITIME");
				strTime = q.getStringFileds("END_VIDEO_TIME");
				event.uVideoEndTime = MakeTime(strTime);
				event.uMiVideoEndTime = q.getIntFileds("END_VIDEO_MITIME");

				String strVideoPath = q.getStringFileds("VIDEOPATH");
				memcpy(event.chVideoPath,strVideoPath.c_str(),strVideoPath.size());

				////////////////////////////////////////车身颜色，类型，速度，方向等
				event.uColor1 = q.getIntFileds("COLOR");
				event.uType = q.getIntFileds("TYPE");
				if(event.uType==1)
                {
                    event.uType = PERSON_TYPE;//行人
                }
                else
                {
                    event.uType = OTHER_TYPE;//非机动车
                }

				event.uSpeed = q.getIntFileds("SPEED");
				event.uWeight1 = q.getIntFileds("COLORWEIGHT");
				event.uColor2 = q.getIntFileds("COLORSECOND");
				event.uWeight2 = q.getIntFileds("COLORWEIGHTSECOND");
				event.uColor3 = q.getIntFileds("COLORTHIRD");
				event.uWeight3 = q.getIntFileds("COLORWEIGHTTHIRD");
				event.uDirection = q.getIntFileds("DIRECTION");

                if(event.uCode == DETECT_RESULT_EVENT_WRONG_CHAN)
                {
                    event.uCode = 14;
                }
                else if(event.uCode == DETECT_RESULT_EVENT_APPEAR)
                {
                    event.uCode = 10;
                }

                ////////////////////////////////////////
				MIMAX_HEADER mHeader;
				mHeader.uCameraID = GetCameraID(nChannel);
                mHeader.uCmdID = MIMAX_EVENT_REP;
				mHeader.uCmdFlag = 0x00000002;//此时均设为非实时事件

			#ifdef LS_QINGTIAN_IVAP
				if (g_nServerType == 5)
			#else
				if(g_nServerType == 0 || g_nServerType == 6 || g_nServerType == 4 || g_nServerType == 26)
			#endif                
                {
                    if(g_nServerType == 0)
                    {
                         event.uColor1 += 1;//送中心端颜色值加1
                         event.uColor2 += 1;
                         event.uColor3 += 1;
                    }

                    BOCOM_RECORD_EVENT br_event;
                    memcpy(&br_event,&event,sizeof(BOCOM_RECORD_EVENT));
					string strFtpVideoPath(event.chVideoPath);
					if(strFtpVideoPath.size() > 0)
					{
						int nPos = g_ServerHost.size()+6;
						string strVideoPath = g_strVideo;
						strFtpVideoPath.insert(nPos,strVideoPath.c_str(),strVideoPath.size());
						printf("%s\n",strFtpVideoPath.c_str());
						strFtpVideoPath.insert(6,"road:road@",10);
						printf("%s\n",strFtpVideoPath.c_str());
						printf("event.chVideoPath=%s\n",event.chVideoPath);
						memset(event.chVideoPath,0,MAX_VIDEO);
						printf("event.chVideoPath=%s\n",event.chVideoPath);
						memcpy(event.chVideoPath,strFtpVideoPath.c_str(),strFtpVideoPath.size());
						printf("event.chVideoPath=%s\n",event.chVideoPath);
					}
					memcpy(br_event.chVideoPath,event.chVideoPath,MAX_VIDEO);//录像路径
                    memcpy(br_event.chPicPath,event.chPicPath,MAX_VIDEO);//大图片路径

                    br_event.uColor2 = event.uColor2;   //车身颜色2
                    br_event.uColor3 = event.uColor3;   //车身颜色3

                    br_event.uWeight1 = event.uWeight1;      //车身颜色权重1
                    br_event.uWeight2 = event.uWeight2;      //车身颜色权重2
                    br_event.uWeight3 = event.uWeight3;      //车身颜色权重3
                    br_event.uDetailCarType = event.uDetailCarType;       //车型细分

                    strRecord.append((char*)&br_event,sizeof(BOCOM_RECORD_EVENT));

                    //add by Gaoxiang
                    if(g_nSendImage == 1) //按照设置判断是否发图片
                    {
                        string strPic = GetImageByPath(strPicPath);
                        strRecord.append((char*)strPic.c_str(),strPic.size());
                    }
                    else
                    {
                    	br_event.uPicSize = 0;
	                    br_event.uPicWidth = 0;
	                    br_event.uPicHeight = 0;
                        memset(br_event.chPicPath, 0, MAX_VIDEO);
                    }
                }
                else
                {
                    //change by Gaoxiang
                    strRecord.append((char*)&event,sizeof(RECORD_EVENT));
                    string strPic = GetImageByPath(strPicPath);
                    strRecord.append((char*)strPic.c_str(),strPic.size());
                }
                
                mHeader.uCmdLen = sizeof(MIMAX_HEADER)+strRecord.size();
				strRecord.insert(0,(char*)&mHeader,sizeof(MIMAX_HEADER));

				q.nextRow();
				strListRecord.push_back(strRecord);
			}
			q.finalize();

			if(strListRecord.size() <= 0)
			{
				return false;
			}
			return true;
}

//获取日志历史记录
bool CSeekpaiDB::GetLogHistoryRecord(string &strRecord, unsigned int nId)
{
            if (!strRecord.empty())
            {
                strRecord.clear();
            }
            unsigned int uTimeStamp = GetTimeStamp()-20;//10秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
            String strTime = GetTime(uTimeStamp);
            char buf[1024]={0};
			if(1 == nId)
			{
				sprintf(buf,"Select ID,TIME,CODE,EVENT from SYSTEM_EVENT_INFO where STATUS = 0  and CODE > 0 and TIME <= '%s' ORDER BY TIME desc limit 1",strTime.c_str());
			}
            else
			{
				sprintf(buf,"Select ID,TIME,CODE,EVENT from SYSTEM_EVENT_INFO where STATUS%d = 0  and CODE > 0 and TIME <= '%s' ORDER BY TIME desc limit 1", nId, strTime.c_str());
			}
            String strSql(buf);
            MysqlQuery q = g_skpDB.execQuery(strSql);
            //printf("log numFileds=%d \n",q.numFileds());

            if(!q.eof())
            {
                RECORD_LOG log;
                log.uSeq = q.getUnIntFileds("ID");
                strTime = q.getStringFileds("TIME");
                log.uTime = MakeTime(strTime);
                log.uCode = q.getIntFileds("CODE");
                String text = q.getStringFileds("EVENT");
                ////////////////////////////////utf8->gbk
                printf("text=%s,text.size()=%d\n",text.c_str(),text.size());
                g_skpDB.UTF8ToGBK(text);
            //	printf("text=%s\n",text.c_str());
                memcpy(log.chText,text.c_str(),text.size());
            //	printf("log.chText=%s\n",log.chText);
                ////////////////////////////////////////////////
                MIMAX_HEADER mHeader;
                mHeader.uCmdID = PLATE_LOG_REP;
                mHeader.uCmdFlag = 0x00000002;//此时均设为非实时日志
                mHeader.uCmdLen = sizeof(MIMAX_HEADER)+sizeof(RECORD_LOG);

                  if(  (log.uCode != ALARM_CODE_NO_VIDEO) &&
                     (log.uCode != ALARM_CODE_CAMERA_MOVED))
                {
                       mHeader.uCameraID  = g_nCameraID + 20000;//检测器报警
                }
                else
                {
                      mHeader.uCameraID  = g_nCameraID;//相机报警
                }

                strRecord.append((char*)&mHeader,sizeof(MIMAX_HEADER));
                strRecord.append((char*)&log,sizeof(RECORD_LOG));
            }
            q.finalize();
            return (!strRecord.empty());
}


//获取统计历史记录
bool CSeekpaiDB::GetStatisticHistoryRecord(string &strRecord, unsigned int nId)
{
            if (!strRecord.empty())
            {
                strRecord.clear();
            }
            unsigned int uTimeStamp = GetTimeStamp()-20;//10秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
            String strTime = GetTime(uTimeStamp);
            char buf[1024]={0};
			if(1 == nId)
			{
				sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from TRAFFIC_STATISTIC_INFO where STATUS = 0 and TIME <= '%s' ORDER BY TIME desc limit 6",strTime.c_str());
			}
			else
			{
				sprintf(buf,"Select ID,TIME,CHANNEL,ROAD,STATTIMELEN,KIND,VALUE,TYPE from TRAFFIC_STATISTIC_INFO where STATUS%d = 0 and TIME <= '%s' ORDER BY TIME desc limit 6", nId, strTime.c_str());
			}
            
            String strSql(buf);
            MysqlQuery q = g_skpDB.execQuery(strSql);
            //printf("static numFileds=%d \n",q.numFileds());

            RECORD_STATISTIC statistic;
            int nChannel,nRoadId,nKind;
            while(!q.eof())//
            {
                unsigned int uSeq = q.getUnIntFileds("ID");
                if(statistic.uSeq != uSeq)
                {
                    statistic.uSeq = uSeq;
                    strTime = q.getStringFileds("TIME");
                    statistic.uTime = MakeTime(strTime);
                    statistic.uStatTimeLen = q.getIntFileds("STATTIMELEN");
                }
                nChannel = q.getIntFileds("CHANNEL");
                nRoadId = q.getIntFileds("ROAD");
                nKind = q.getIntFileds("KIND");
                double value = q.getFloatFileds("VALUE");
                unsigned int uRoadIndex = 0;
                unsigned int uRoadType = q.getIntFileds("TYPE");

                switch(nKind)
                {
                    case 100:
                        uRoadIndex = nRoadId;
                        uRoadIndex = uRoadIndex<<16;

                        statistic.uRoadType[nRoadId-1] = uRoadIndex|uRoadType;
                        //printf("statistic.uRoadType[nRoadId-1]=%d\n",statistic.uRoadType[nRoadId-1]);
                        statistic.uFlux[nRoadId-1] = (unsigned int)(value+0.5);
                        //printf("statistic.uFlux[nRoadId-1]=%d\n",statistic.uFlux[nRoadId-1]);
                        break;
                    case 101:
                        statistic.uSpeed[nRoadId-1] = (unsigned int)(value+0.5);
                        break;
                    case 102:
                        statistic.uQueue[nRoadId-1] = (unsigned int)(value+0.5);
                        break;
                    case 103:
                        statistic.uOccupancy[nRoadId-1] = (unsigned int)(value+0.5);
                        break;
                    case 104:
                        statistic.uSpace[nRoadId-1] = (unsigned int)(value+0.5);
                        break;
                    case 105:
                        statistic.uFluxCom[nRoadId-1] = (unsigned int)(value+0.5);
                        //printf("statistic.uFluxCom[nRoadId-1]=%d\n",statistic.uFluxCom[nRoadId-1]);
                        break;
                }
                q.nextRow();
            }

            if(q.numFileds()>0)
            {
                ///////////////////////////////////////
                MIMAX_HEADER mHeader;
                mHeader.uCmdID = MIMAX_STATISTIC_REP;
                mHeader.uCmdFlag = 0x00000002;//此时均设为非实时事件
                mHeader.uCmdLen = sizeof(MIMAX_HEADER)+sizeof(RECORD_STATISTIC);
                mHeader.uCameraID = GetCameraID(nChannel);
                strRecord.append((char*)&mHeader,sizeof(MIMAX_HEADER));
                strRecord.append((char*)&statistic,sizeof(RECORD_STATISTIC));
                //printf("send 111 statistic statistic.uSeq=%u\n",statistic.uSeq);
            }
            q.finalize();

            return (!strRecord.empty());
}

/*******************************************************************************************/

//保存H264编码格式录像文件
//  ID---记录编号(从0开始自增)，nChannel---通道号，uBeginTime---开始时间，uEndTime---结束时间， strPath---路径，
//  VideoType---录像类型(0---全天录像，1---定时录像)
int CSeekpaiDB::SaveVideo(int nChannel,UINT32 uBeginTime, UINT32 uEndTime,String strPath,int VideoType)
{
    String strBeginTime = GetTime(uBeginTime);
    String strEndTime = GetTime(uEndTime);
	char buf[BUFF_SIZE]={0};
	//保存文件信息到数据库
	sprintf(buf,"INSERT INTO VIDEO_FILE_INFO(CHANNEL, BEGIN_TIME, END_TIME, PATH, VIDEO_TYPE,STATUS) VALUES(%d,'%s','%s','%s',%d,0)",
		nChannel,strBeginTime.c_str(),strEndTime.c_str(),strPath.c_str(), VideoType);
	String sql(buf);
	if(execSQL(sql)!=0)
	{
      LogError("保存录像记录失败[%d][%s][%s][%s][%d]\r\n",nChannel,strBeginTime.c_str(),strEndTime.c_str(),strPath.c_str(),VideoType);
	}
	return 0;
}

//取得最旧的全天录像时间
String CSeekpaiDB::GetOldDayVideoPath()
{
	char buf[100] = {0};
	string strFileName = "";
	unsigned int id = 0;

	while(true)
	{
		memset(buf, 0, 100);
		sprintf(buf, "Select ID, PATH from VIDEO_FILE_INFO order by ID limit 20");
		String strSql(buf);

		MysqlQuery mysql = execQuery(strSql);

		//数据库为空的时候return
		if (mysql.eof())
		{
			mysql.finalize();//释放结果集
			return "";
		}

		while(!mysql.eof())
		{
			id = mysql.getUnIntFileds("ID");
			strFileName = mysql.getStringFileds("PATH");

			struct stat s;
			stat(strFileName.c_str(), &s);

			//文件不存在或空 则删除不存在的记录
			if(access(strFileName.c_str(),F_OK) != 0 || s.st_size <= 0)
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "Delete from VIDEO_FILE_INFO where ID = '%u'", id);

				String sqldel(buf);
				execSQL(sqldel);
			}
			else
			{
				mysql.finalize();//释放结果集

				if((g_nFtpServer == 1) &&(g_nServerType == 13))//交警3期ftp-server模式下的全天录像不在此处删除
				{
					int nPos = -1;
					nPos = strFileName.find("lx");
					if(nPos > 0)
					{
						strFileName = "";
					}
				}

				return strFileName;
			}
			mysql.nextRow();
		}
	}
}


//删除全天录像
void CSeekpaiDB::DeleteVideoFile(int nCount)
{
        char buf[256]= {0};
/*        sprintf(buf, "Select count(*) from VIDEO_FILE_INFO");
        String strSqlCount(buf);
        int nRow =0;
        nRow = getIntFiled(strSqlCount);

       //printf("===============DeleteVideoFile nRow=%d\n",nRow);
       UINT32 uMaxVideoCount = 500;

		if(g_sysInfo_ex.fTotalDisk > g_nMaxDisk)
        {
           if(g_VideoFormatInfo.nTimeLength == 2)
			{
				uMaxVideoCount = 6000;
			}
			else
			{
				uMaxVideoCount = 3000;
			}
        }

        if(nRow >= uMaxVideoCount)
*/        {
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "Select * from VIDEO_FILE_INFO order by ID limit %d",nCount);
            String strSql(buf);

            MysqlQuery H264_q = execQuery(strSql);
            while(!H264_q.eof())
            {
                  unsigned int uH264_ID = 0; //
                  string strFileName = "";
                  uH264_ID = H264_q.getUnIntFileds("ID");
                  strFileName = H264_q.getStringFileds("PATH"); //获取当前拥有最小ID的录像文件路径名

                  String strDeleteDate;
				  int nIndex = 32;//for /home/road/server/video/20110308
				  if (IsDataDisk())
				  {
					  nIndex = 26;
				  }
				  strDeleteDate.append(strFileName.c_str(),nIndex);//for /detectdata/video/20110308
				  
                  if(m_strDeleteDate.size() >= nIndex)
                  {
                      if(strncmp(strDeleteDate.c_str(), m_strDeleteDate.c_str(),nIndex) != 0)  //"年月日"目录名比较
                      {
                        //删掉日目录
						  remove(m_strDeleteDate.c_str());
                      }
                  }
                  m_strDeleteDate = strDeleteDate;
              //如果存在则删除文件同时删除记录，直到找到为止
                if(access(strFileName.c_str(),F_OK) == 0)
                {
					 remove(strFileName.c_str());

					 if(uH264_ID > 0)
					{
						memset(buf, 0, sizeof(buf));
						sprintf(buf, "Delete from VIDEO_FILE_INFO where ID = '%u'", uH264_ID);

						String sqldel(buf);
						execSQL(sqldel);
					}
				}
         
				usleep(10);

                H264_q.nextRow();
            }
            H264_q.finalize();//释放结果集
        }
}


//获取录像文件列表
unsigned short CSeekpaiDB::GetVideoFileList(string strBeginTime,string strEndTime,string& strPathList)
{
    unsigned short nCount = 0;

    string strTime1;
    strTime1.append(strBeginTime.c_str(),4);//year
    strTime1 += "-";
    strTime1.append(strBeginTime.c_str()+4,2);//month
    strTime1 += "-";
    strTime1.append(strBeginTime.c_str()+6,2);//day
    strTime1 += " ";
    strTime1.append(strBeginTime.c_str()+8,2);//hour
    strTime1 += ":";
    strTime1.append(strBeginTime.c_str()+10,2);//minute
    strTime1 += ":";
    strTime1.append(strBeginTime.c_str()+12,2);//second

    string strTime2;
    strTime2.append(strEndTime.c_str(),4);
    strTime2 += "-";
    strTime2.append(strEndTime.c_str()+4,2);
    strTime2 += "-";
    strTime2.append(strEndTime.c_str()+6,2);
    strTime2 += " ";
    strTime2.append(strEndTime.c_str()+8,2);
    strTime2 += ":";
    strTime2.append(strEndTime.c_str()+10,2);//minute
    strTime2 += ":";
    strTime2.append(strEndTime.c_str()+12,2);//second

    char buf[1024]={0};
    sprintf(buf,"Select PATH from VIDEO_FILE_INFO where BEGIN_TIME >= '%s' and BEGIN_TIME <= '%s';",strTime1.c_str(),strTime2.c_str());

    String strSql(buf);
    MysqlQuery q = g_skpDB.execQuery(strSql);

    nCount = q.numFileds();

    printf("GetVideoFileList =strBeginTime=%s,strEndTime.c_str()=%s\n",strBeginTime.c_str(),strEndTime.c_str());
    printf("GetVideoFileList =strSql=%s,nCount=%d\n",strSql.c_str(),nCount);

    string strDirection;
    ROAD_DIRECTION_MAP::iterator it =  g_roadDirectionMap.begin();
    if(it != g_roadDirectionMap.end())
    {
        strDirection = it->second.chDirection;
    }
    else
    {
        strDirection = "1A";
    }

    string strVideoList;
    while(!q.eof())
    {
        string strPath = q.getStringFileds(0);

        string strVideoPath;
        strVideoPath.append("VIDEO_");
        strVideoPath.append(g_strFtpUserName.c_str());
        strVideoPath.append("_");
        strVideoPath.append(strDirection.c_str());
        strVideoPath.append("_0_");
        strVideoPath.append(strPath.c_str()+24,8);
        strVideoPath.append(strPath.c_str()+33,2);
        strVideoPath.append(strPath.c_str()+36,2);
        strVideoPath.append("00000.mp4");

        strVideoList += strVideoPath;
        strVideoList += ";";

        q.nextRow();
    }
    q.finalize();

    if(strVideoList.size() > 0)
    {
        strPathList.append((char*)strVideoList.c_str(),strVideoList.size()-1);//去除最后一个;号
    }

    return nCount;
}


//判断历史视频文件是否已经被检测过
bool CSeekpaiDB::IsHistoryVideoDealed(String strPath)
{
    char buf[BUFF_SIZE]={0};

    sprintf(buf,"select * from HISTORY_VIDEO_INFO where VIDEO_NAME = '%s' and DETECT_STATUS = 1",strPath.c_str());
    String strSql(buf);
    MysqlQuery q = g_skpDB.execQuery(strSql);
    int nCount = q.numFileds();
    q.finalize();

    if(nCount <= 0)
    {
        return false;
    }

    return true;
}

//保存历史视频文件列表
//  ID---记录编号(从0开始自增)nDeviceID---设备号，uBeginTime---开始时间，uEndTime---结束时间， strPath---路径，
//  nStatus---(0---未完成，1---已完成)
bool CSeekpaiDB::SaveHistoryVideoInfo(int nDeviceID,String strPath,String strBeginTime,int uBeginMiTime,String strEndTime,int uEndMiTime,int nStatus)
{
	char buf[BUFF_SIZE]={0};

	if(nStatus == 0)//分析前入库
	{
	    sprintf(buf,"select * from HISTORY_VIDEO_INFO where VIDEO_NAME = '%s'",strPath.c_str());
        String strSql(buf);
        MysqlQuery q = g_skpDB.execQuery(strSql);
        int nCount = q.numFileds();
        q.finalize();

        if(nCount <= 0)
        {
            memset(buf,0,BUFF_SIZE);
            sprintf(buf,"INSERT INTO HISTORY_VIDEO_INFO(DEVICE_ID,VIDEO_NAME,VIDEO_FORMAT,VIDEO_BEGIN_TIME,VIDEO_BEGIN_MITIME, VIDEO_END_TIME, VIDEO_END_MITIME, DETECT_STATUS) VALUES(%d,'%s',%d,'%s',%d,'%s',%d,%d)",
            nDeviceID,strPath.c_str(),1,strBeginTime.c_str(),uBeginMiTime,strEndTime.c_str(),uEndMiTime,nStatus);
        }
        else//如果已经存在则不需要入库
        {
            return true;
        }
	}
	else if(nStatus == 1)//分析后修改库
	{
        sprintf(buf,"UPDATE HISTORY_VIDEO_INFO SET DETECT_STATUS = 1 where VIDEO_NAME = '%s'",strPath.c_str());
	}

	String sql(buf);
	if(execSQL(sql)!=0)
	{
      LogError("保存历史视频文件列表失败[%d][%s][%s][%s][%d]\r\n",nDeviceID,strBeginTime.c_str(),strEndTime.c_str(),strPath.c_str(),nStatus);
      return false;
	}
	return true;
}

//更新检测状态
void CSeekpaiDB::UpdateDetectStatus(int nCameraID)
{
	char buf[BUFF_SIZE]={0};
	sprintf(buf, "update CHAN_INFO set VIDEO_ENDTIME = VIDEO_BEGINTIME where CAMERA_ID = %d", nCameraID);
	String sql(buf);
	execSQL(sql);
}

string CSeekpaiDB::GetCameraIP(int channelId)
{
	char buf[BUFF_SIZE]={0};
	sprintf(buf, "select CAMERAIP from CHAN_INFO where CHAN_ID = %d", channelId);
	String sql(buf);
	String str = getStringFiled(sql);
	return str;
}

//清空数据库BASE_PLATE_INFO
bool CSeekpaiDB::ClearPlateInfo()
{
	char buf[BUFF_SIZE]={0};
	sprintf(buf,"delete from BASE_PLATE_INFO");//在载入前先清空该表
	String sql(buf);
	if(execSQL(sql)!=0)
	{
		LogError("清空数据库BASE_PLATE_INFO失败!\n");
		return false;
	}
	return true;
}


//添加车牌数据到数据库BASE_PLATE_INFO
bool CSeekpaiDB::AddPlateInfo(string strCarNumver)
{
	char buf[BUFF_SIZE]={0};
	sprintf(buf, "Insert into BASE_PLATE_INFO(NUMBER) values('%s')", strCarNumver.c_str());
	//sprintf(buf,"Insert into TIME_INFO(TIME) values('%s')",strTime.c_str());
	String sql(buf);
	if(execSQL(sql)!=0)
	{
		LogError("添加车牌失败!\n");
		return false;
	}
	return true;
}

//strLoadDir加载原的文件名，strMysqlName被加载的数据库表名
bool CSeekpaiDB::LoadPlateInfo(string strLoadDir, string strMysqlName)
{
	char buf[BUFF_SIZE];
	memset(buf,0,BUFF_SIZE);
	sprintf(buf, "load data infile './%s' into table %s FIELDS TERMINATED BY'.'",strLoadDir.c_str(),strMysqlName.c_str());
	String sql(buf);
	if(execSQL(sql)!=0)
	{
		LogError("加载车牌失败!\n");
		return false;
	}
	return true;
}


//在数据库BASE_PLATE_INFO中查询指定车牌，找到返回ture，没有返回false
string CSeekpaiDB::FindPlateInfo(string strCarNumver)
{
	string strCarPID = "";
	char buf[BUFF_SIZE];
	memset(buf,0,BUFF_SIZE);
	sprintf(buf, "select P_ID from BASE_PLATE_INFO where NUMBER = '%s'", strCarNumver.c_str());
	//sprintf(buf, "select * from BASE_PLATE_INFO", strCarNumver.c_str());
	String sql(buf);
	MysqlQuery q = execQuery(sql);
	if(!q.eof())
    {
        strCarPID = q.getStringFileds(0);
    }
    q.finalize();
	return strCarPID;
}

//获得所有车牌
void CSeekpaiDB::GetAllPlate(list<string> &listBASE_PLATE_INFO, int nFirstId, int nCount)
{
	//在加载前先清空list中数据
	listBASE_PLATE_INFO.clear();
	string strTemp;
	char buf[BUFF_SIZE];
	memset(buf,0,BUFF_SIZE);
	if (nCount == 0)
	{
		sprintf(buf, "select NUMBER from BASE_PLATE_INFO where P_ID > %d", (nFirstId-1));
	}
	else
	{
		sprintf(buf, "select NUMBER from BASE_PLATE_INFO where P_ID > %d limit %d", (nFirstId-1), nCount);
	}
	String sql(buf);
    String strPlateInfo;
    MysqlQuery q = execQuery(sql);
	while(!q.eof())
    {
        strTemp = q.getStringFileds(0);
		//strPlateInfo += strTemp;
		listBASE_PLATE_INFO.push_back(strTemp);
        q.nextRow();
    }
    q.finalize();
	//return strPlateInfo;
}

//获得前n条记录的id号（主键号）,这里暂时只做了获取第一条的功能
int CSeekpaiDB::GetId(string strTableName,int n)
{
	//在加载前先清空list中数据
	int nRet = -1;
	char buf[BUFF_SIZE];
	memset(buf,0,BUFF_SIZE);
	sprintf(buf, "select P_ID from %s limit %d",strTableName.c_str(),n);
	String sql(buf);
	String strPlateInfo;
	MysqlQuery q = execQuery(sql);
	while(!q.eof())
	{
		nRet = q.getIntFileds(0);
		q.nextRow();
	}
	q.finalize();
	return nRet;
}


//读入检测结果文件
void CSeekpaiDB::LoadResult(char* filename, std::list<RECORD_PLATE>& listTestResult)
{
	FILE *IN = fopen64(filename, "r");
	if(!IN)
	{
		cerr<<"can not open read "<<filename<<endl;
		return;
	}

	char buf[BUFF_SIZE] = {0};
	while(fgets(buf, BUFF_SIZE, IN))
	{
		if (strlen(buf) < 10)
		{
			continue;
		}
		//cerr<<buf<<endl;

		//读入一条车牌记录
		RECORD_PLATE plate;

		plate.uSeqID = (UINT32)atoi(strtok(buf, "\t"));//帧号1
		plate.uRoadWayID = (UINT32)atoi(strtok(NULL, "\t"));//道路ID
		plate.uDirection = (UINT32)atoi(strtok(NULL, "\t"));//行驶方向

		char *p = strtok(NULL, "\t");
		memcpy(plate.chText, p, strlen(p));//车牌文本

		plate.uColor = (UINT32)atoi(strtok(NULL, "\t"));//车牌颜色
		plate.uPlateType = (UINT32)atoi(strtok(NULL, "\t"));//车牌结构
		plate.uSpeed = (UINT32)atoi(strtok(NULL, "\t"));//车速
		plate.uCarColor1 = (UINT32)atoi(strtok(NULL, "\t"));//车身颜色
		plate.uType = (UINT32)atoi(strtok(NULL, "\t"));//车辆类型
		plate.uTypeDetail = (UINT32)atoi(strtok(NULL, "\t"));//车型细分
		plate.uViolationType = (UINT32)atoi(strtok(NULL, "\t"));//违章类型
		plate.uCarBrand = (UINT32)atoi(strtok(NULL, "\t"));//车标

		//位置坐标
		plate.uPosLeft = (UINT32)atoi(strtok(NULL, "\t"));
		plate.uPosTop = (UINT32)atoi(strtok(NULL, "\t"));
		plate.uPosRight = (UINT32)atoi(strtok(NULL, "\t"));
		plate.uPosBottom = (UINT32)atoi(strtok(NULL, "\t"));

		//电警图片2,3两张帧号
		char* pSeq2 = strtok(NULL, "\t");
		if(pSeq2!=NULL)
		{
			plate.uTime2 = (UINT32)atoi(pSeq2);//帧号2
		}
		
		char* pSeq3 = strtok(NULL, "\t");
		if(pSeq3!=NULL)
		{
			plate.uMiTime2 = (UINT32)atoi(pSeq3);//帧号3
		}

		//LogNormal("uSeq2=%d,uSeq3=%d",plate.uTime2,plate.uMiTime2);

		//向数据库插入记录
		listTestResult.push_back(plate);
	}

	fclose(IN);
}

bool CSeekpaiDB::SaveCamIpByID(string strCamIp, string strCamMultiIp, int nNewCamId, int nOldCamId)
{
	char buf[BUFF_SIZE];
	memset(buf,0,BUFF_SIZE);
	sprintf(buf, "update CHAN_INFO set CAMERAIP = '%s', CAM_MULTI_IP = '%s', CAMERASELF_ID = %d where CAMERASELF_ID = %d",strCamIp.c_str(), strCamMultiIp.c_str(), nNewCamId, nOldCamId);
	String sql(buf);
	if(execSQL(sql)!=0)
	{
		LogError("保存相机IP失败!\n");
		return false;
	}
	return true;
}

void CSeekpaiDB::GetCamIpByChannelID(string &strCamIp, string &strCamMultiIp, int nChannelID)
{
	char buf[BUFF_SIZE];
	memset(buf,0,BUFF_SIZE);
	sprintf(buf, "select CAMERAIP,CAM_MULTI_IP from CHAN_INFO where CHAN_ID = %d",nChannelID);
	String sql1(buf);
	MysqlQuery q1 = execQuery(sql1);
	while(!q1.eof())
	{
		strCamIp = q1.getStringFileds(0);
		strCamMultiIp = q1.getStringFileds(1);
		q1.nextRow();
	}
	q1.finalize();
}

void CSeekpaiDB::GetCamIpByID(string &strCamIp, string &strCamMultiIp, int nCamID)
{
	char buf[BUFF_SIZE];
	memset(buf,0,BUFF_SIZE);
	sprintf(buf, "select CAMERAIP,CAM_MULTI_IP from CHAN_INFO where CAMERASELF_ID = %d",nCamID);
	String sql1(buf);
	MysqlQuery q1 = execQuery(sql1);
	while(!q1.eof())
	{
		strCamIp = q1.getStringFileds(0);
		strCamMultiIp = q1.getStringFileds(1);
		q1.nextRow();
	}
	q1.finalize();
}

int CSeekpaiDB::GetCameraSelfID(int nChannel)
{
	char buf[BUFF_SIZE]={0};
	sprintf(buf, "select CAMERASELF_ID from CHAN_INFO where CHAN_ID = %d", nChannel);
	String sql(buf);
	int nRet = getIntFiled(sql);
	return nRet;
}

//批量设置检测类型
bool CSeekpaiDB::UpdateDetectKind(int kind,int nChannelID)
{
	char buf[BUFF_SIZE]={0};
	if(nChannelID == 0)
	{
		sprintf(buf, "Update CHAN_INFO set CHAN_DETECT_KIND=%d", kind);
	}
	else
	{
		sprintf(buf, "Update CHAN_INFO set CHAN_DETECT_KIND=%d where CHAN_ID = %d", kind,nChannelID);
	}
	String sql(buf);
	if(execSQL(sql)!=0)
	{
		LogError("批量更新检测类型失败!\r\n");
		return false;
	}
	else
	{
		if(nChannelID == 0)
		{
			SRIP_CHANNEL* sChannel;
			string channels = GetChannelList();
			int size = channels.size()/sizeof(SRIP_CHANNEL);
			for(int i = 0; i < size; i++)
			{
				//同步通道
				sChannel = (SRIP_CHANNEL*)(channels.c_str() + i * sizeof(SRIP_CHANNEL));
				g_skpChannelCenter.ModifyChannel(*sChannel);
			}
		}
	}
	return true;
}

//切换相机和预置位
void CSeekpaiDB::UpdateCameraIDAndPresetID(int oldCameraID, int newCameraID, int nPresetID)
{
	char buf[BUFF_SIZE]={0};
	sprintf(buf, "update CHAN_INFO set CAMERA_ID=%d, CHAN_PRESET=%d where CAMERA_ID = %d", newCameraID, nPresetID, oldCameraID);
	String sql(buf);
	execSQL(sql);
}


//刷新通道信息
bool CSeekpaiDB::UpdateChannelInfo(SRIP_CHANNEL sChannel)
{
	char buf[BUFF_SIZE]={0};
	int nRow = 0;

	sprintf(buf, "select count(CAMERA_ID) from CHAN_INFO where CAMERA_ID = %d",sChannel.nCameraId);
	String sqlCount(buf);

	nRow = getIntFiled(sqlCount);

	if(nRow <= 0)
	{
		return false;
	}

	String strVideoBeginTime = GetTime(sChannel.uVideoBeginTime);
	String strVideoEndTime = GetTime(sChannel.uVideoEndTime);
	
	sprintf(buf, "update CHAN_INFO set CAMERA_ID=%d, CHAN_PRESET=%d,CHAN_DETECT_KIND = %d,VIDEO_BEGINTIME = '%s',VIDEO_ENDTIME = '%s' where CHAN_ID = %d", sChannel.nCameraId, sChannel.nPreSet,sChannel.uDetectKind ,strVideoBeginTime.c_str(),strVideoEndTime.c_str(),sChannel.uId);
	String sql(buf);
	if(execSQL(sql)!=0)
	{
		LogError("刷新通道信息失败!\n");
		return false;
	}

	return true;
}

//替换通道信息
bool CSeekpaiDB::UpdateSwitchChannelInfo(SRIP_CHANNEL sChannel, int nOldCameraID, int &channelId)
{
	char buf[BUFF_SIZE]={0};
	int nRow = 0;
	int iChannelId = 0;

	sprintf(buf, "select count(CAMERA_ID),CHAN_ID from CHAN_INFO where CAMERA_ID = %d",nOldCameraID);
	String sqlCount(buf);
	MysqlQuery q1 = execQuery(sqlCount);
	printf("===CSeekpaiDB::UpdateSwitchChannelInfo===================q1.numFileds = %d \n",q1.numFileds());
	while(!q1.eof())
	{
		nRow = q1.getIntFileds(0);
		iChannelId = q1.getIntFileds(1);
		q1.nextRow();
	}
	q1.finalize();

	if(nRow <= 0)
	{
		return false;
	}
	channelId = iChannelId;

	String strVideoBeginTime = GetTime(sChannel.uVideoBeginTime);
	String strVideoEndTime = GetTime(sChannel.uVideoEndTime);
	
	sprintf(buf, "update CHAN_INFO set CAMERA_ID=%d, CHAN_PRESET=%d,CHAN_DETECT_KIND = %d,VIDEO_BEGINTIME = '%s',VIDEO_ENDTIME = '%s' where CHAN_ID = %d", sChannel.nCameraId, sChannel.nPreSet,sChannel.uDetectKind ,strVideoBeginTime.c_str(),strVideoEndTime.c_str(),iChannelId);
	String sql(buf);
	if(execSQL(sql)!=0)
	{
		LogError("刷新通道信息失败 !\n");
		return false;
	}

	return true;
}

int CSeekpaiDB::CreateChannelId()
{
	char buf[BUFF_SIZE]={0};
	int arr[20];
	memset(arr, '\0', sizeof(arr));
	int i = 0;
	int id = 0;
	sprintf(buf, "select distinct CHAN_ID from CHAN_INFO order by CHAN_ID asc");
	String sqlCount(buf);
	MysqlQuery q1 = execQuery(sqlCount);
	printf("===CSeekpaiDB::CreateChannelId===================q1.numFileds = %d \n",q1.numFileds());
	while(!q1.eof())
	{
		id = q1.getIntFileds("CHAN_ID");
		arr[i] = id;
		i++;
		q1.nextRow();
	}
	q1.finalize();
	int iChannelId = 1;
	int j = 0;

	while (j <= i)
	{
		if(iChannelId == arr[j])
		{
			iChannelId++;
			j++;
		}
		else
		{
			break;
		}
	}
	
//printf("===CSeekpaiDB::CreateChannelId===================iChannelId = %d \n",iChannelId);
	return iChannelId;
}

/*
函数功能：邢台中心端获取最新图片序号，用于填充车牌信息包体中自增包标志序号
输入：无
输出：无
返回值：最新图片ID
*/
unsigned int  CSeekpaiDB::XingTaiGetLastPicId()
{
	char buf[BUFF_SIZE]= {0};
	unsigned int uPicId = 0;
	unsigned int uVideoId = 0;

	sprintf(buf,"Select max(PIC_ID),max(VIDEO_ID) from PIC_INFO");


	String sql(buf);
	MysqlQuery q = execQuery(sql);

	if(q.numFileds() > 0)
	{
		uPicId = q.getUnIntFileds("max(PIC_ID)");
	}
	q.finalize();

	if(uPicId<=0)
		uPicId = 1;

	return uPicId;
}

//数据库导入
bool CSeekpaiDB::DBImport()
{
	if(access("bocom_db.sql",F_OK)!=0)//文件不存在
	{
		return false;
	}
	struct stat st;
	int fd = open("bocom_db.sql", O_RDONLY);
	fstat(fd, &st);
	char *sqlstr = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	printf("sqlstr=%s\n",sqlstr);
	if (sqlstr == NULL)
	{
		return false;
	}
	close(fd);

	if(execSQL(sqlstr)!=0)
	{
		munmap(sqlstr, st.st_size);
		return false;
	}
	munmap(sqlstr, st.st_size);
	return true;
}

//数据库升级
bool CSeekpaiDB::DBUpdate()
{
	if(access("db_upd_sql.xml",F_OK)!=0)//文件不存在
	{
		return false;
	}
	XMLNode xml = XMLNode::parseFile("db_upd_sql.xml");
	XMLNode xmlNodes = xml.getChildNode("UpdSqls");
	int nNodeNum = xmlNodes.nChildNode("Sql");
    if(0 == nNodeNum)
    {
		return false;
	}

	 for (int i = 0; i < nNodeNum; i++)
    {
        XMLNode xmlNode = xmlNodes.getChildNode("Sql", i);
        const char* pSql = xmlNode.getText();

        if (pSql != NULL)
        {
           execSQL(pSql);
        }
    }

	return true;
}

//获取设备Id
String CSeekpaiDB::GetDeviceId(UINT32 uCameraId)
{
	String strDeviceId;

	char szBuff[BUFF_SIZE] = {0};
	sprintf(szBuff, "select DEVICE_ID from CHAN_INFO where CAMERA_ID=%d",uCameraId);
	String sql(szBuff);
	MysqlQuery q = execQuery(sql);
	if(!q.eof())
	{
		strDeviceId = q.getStringFileds(0);
	}
	q.finalize();

	return strDeviceId;
}

//根据通道号获取设备Id
String CSeekpaiDB::GetDeviceByChannelId(UINT32 uChannelId)
{
	String strDeviceId;

	char szBuff[BUFF_SIZE] = {0};
	sprintf(szBuff, "select DEVICE_ID from CHAN_INFO where CHAN_ID=%d",uChannelId);
	String sql(szBuff);
	MysqlQuery q = execQuery(sql);
	if(!q.eof())
	{
		strDeviceId = q.getStringFileds(0);
	}
	q.finalize();

	return strDeviceId;
}

int CSeekpaiDB::GetCameraStateByChannelId( UINT32 uChannelId )
{
	char buf[BUFF_SIZE] = {0};
	sprintf(buf, "select CAMERA_STATE from CHAN_INFO where CHAN_ID=%d", uChannelId);

	std::string sql(buf);
	MysqlQuery q = execQuery(sql);

	int nState = 0;
	if(!q.eof())
	{
		nState = q.getIntFileds(0);
	}
	q.finalize();
	return nState;
}


