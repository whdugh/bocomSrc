/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：CopyToUSB.cpp
* 摘要: USB自动拷贝,需要将U盘格式化为Fat32格式
* 版本: V1.0
* 作者: qianwenming
* 完成日期: 2012年7月25日
*/

#include "XmlParaUtil.h"
#include "MvsCommunication.h"
#include "CopyToUSB.h"
#include "ximage.h"
#include "CSeekpaiDB.h"
#include <sys/vfs.h>


//拷贝服务
CopyToUSB g_copyToUSB;
//usb拷贝标识位
volatile bool g_bEndToUsb = false;

#define USB_MAX_STORAGE_DAYS 14

void* ThreadCopyFile(void *arg)
{
	#ifdef SQLITE_OK
	printf("USB拷贝服务启动 ! \n");
	CopyToUSB* copyToUSB = (CopyToUSB*)arg;
	
	while(!g_bEndThread)
	{	
		copyToUSB->RunToCopy();
		
		sleep(5);
	}
	#endif

	//printf("quit the ThreadCopyFile pthread \n");
	//cerr<<"quit the ThreadCopyFile pthread"<<endl;
	pthread_exit((void *)0);

	return arg;
}

bool CopyToUSB::IsExistUsb()
{
	string strDevice = "/dev/sg1";
	if(IsDataDisk())
	{
		strDevice = "/dev/sg2";
	}
	if (access(strDevice.c_str(),F_OK) == 0)
	{
		return true;
	}

	return false;
}

int  CopyToUSB::CopySql()
{
	//printf("----------------------------sql sql\n");
	sync();
	cpfile("/var/lib/mysql/bocom_db/DEVICE_STATUS.frm", "/data/DEVICE_STATUS.frm");
	//printf("----------------------------sql sql\n");
	sync();
	cpfile("/var/lib/mysql/bocom_db/DEVICE_STATUS.MYD", "/data/DEVICE_STATUS.MYD");
	sync();
	cpfile("/var/lib/mysql/bocom_db/DEVICE_STATUS.MYI", "/data/DEVICE_STATUS.MYI");
	sync();
	haveCopy_sql = 1;
	//cerr<<"22222GetHaveCopy_Sql():"<<GetHaveCopy_Sql()<<endl;
	return 1;
}


void CopyToUSB::cpfile(char *spathname,char *tpathname)
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

bool CopyToUSB::MountUsb()
{
	char dirCopy[50] = "/data/bocom.usb";
	if (access(dirCopy,F_OK) == 0)
	{
		return true;
	}
	return false;
}

bool CopyToUSB::HasUsb()
{
	char dirCopy[20] = "/data/data/";
	if (access(dirCopy,F_OK) == 0)
	{
		return true;
	}
	return false;
}

void CopyToUSB::RunToCopy()
{
	if (MountUsb())
	{
		char dirCopy[20] = "/data/data/";
		if(access(dirCopy,0) != 0) //目录不存在
		{
			mkdir(dirCopy,S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
			sync();
		}

		if (access(dirCopy,F_OK) == 0)
		{
			char buf[256] = {0};

			printf("++++++++++++++++++++++++++begin open Open Sqlite +++++++++\n");
			//cerr<<"++++++++++++++++++++++++++begin open Open Sqlite +++++++++\n"<<endl;
			#ifdef SQLITE_OK
				printf("+------------------------in the Sqlite +++++++++\n");
				printf("\n++++++++++++++++++++++++++in the SQLOTE_OK +++++++++\n\n");
				sleep(1);
				// 判断是否有db文件 如果没有则不创建 并打开
				sqlteCp = new MysqlTransitionSqlteCyToUSB();
				if(sqlteCp->openSqlite() == -1)
				{
					return ;
				}
				if (sqlteCp->AddField() != 1)
				{
					//LogNormal("db已经是最新的，不需要更新。\n");
				}
				else
				{
					//LogNormal("db是老版本，已更新。\n");
				}
				printf("\n++++++++++++++++++++++++++Open Sqlite ok +++++++++\n\n");
				// 判断是否有 PASSVEHICLE 这个表 如果没有创建表
				if (sqlteCp->createTablePassVehicle() != 1)  
				{
					cerr<<"sqlteCp->createTablePassVehicle() error"<<endl;
					return ;
				}
				printf("\n++++++++++++++++++++++++++create Sqlite PASSVEHICLE ok \n++++++\n\n");
			#endif
			
			// 判断是否满足文件删除条件
			CheckDataOutOfDate();

			while(!g_bEndThread)
			{
				if (HasUsb())
				{		
						//日志拷贝
						if (g_nServerType == 13)
						{
							CopyLog();
						}

						// 在GetPlaterPic() 这个方法中 并调用nsh写的把这条数据的信息插入到 passvehicle 这个表中
						if (!GetPlatePic())
						{
							printf("Copy the data from MVS to the mobile device over !\n");
							//cerr<<"Copy the data from MVS to the mobile device over !\n"<<endl;
							break;
						}	
				}
				else
				{
					break;
				}
				sleep(60);
			}
			
			// 关闭数据库
			#ifdef SQLITE_OK
			sqlteCp->closeSqlite();
			delete sqlteCp;
			#endif	
		}
	}
}

void CopyToUSB::CopyLog()
{
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r( &now,newTime );

	if((newTime->tm_hour == 4) && ((newTime->tm_min >= 0) || (newTime->tm_min <= 5)))//每天4点0分更新
	{
		char dirCopy[20] = "/data/data/";
		string strDataPath;
		strDataPath = "/home/road/dzjc/";
		if (IsDataDisk())
		{
			strDataPath = "/detectdata/dzjc/";
		}
		strDataPath += "tsyn/";
		if(access(strDataPath.c_str(),0) == 0)
		{
			char strBuf[1024] = {0};
			sprintf(strBuf,"cp -rf %s %s",strDataPath.c_str(),dirCopy);
			system(strBuf);
		}
	}
}

int CopyToUSB::GetDayTimeSpan(int nCheckDay, struct tm *nowTime, struct tm *oldTime, unsigned long & ulStart, unsigned long & ulEnd)
{
	// 判断是否已跨月
	bool bSpanMonth = (oldTime->tm_year != nowTime->tm_year || oldTime->tm_mon != nowTime->tm_mon) ? true : false;

	struct tm tmpTime = {0};
	if (!bSpanMonth) // 非跨月
	{
		if (nCheckDay > nowTime->tm_mday) // 日大于当天日，则为上月，oldtime为当月1号，删除的是上月数据
		{
			memcpy(&tmpTime, nowTime, sizeof(struct tm));
			if (nowTime->tm_mon == 1) // 当月为1月，则上月为上年12月
			{
				tmpTime.tm_mday = 0;
				tmpTime.tm_mon = 12;
				tmpTime.tm_year--;
			}
			else
			{
				tmpTime.tm_mday = 0;
				tmpTime.tm_mon--;
			}
		}
		else if (nCheckDay < oldTime->tm_mday) // 日小于N天前的日，则为本月
		{
			memcpy(&tmpTime, nowTime, sizeof(struct tm));
		}
	}
	else // 跨月
	{
		if (nCheckDay > nowTime->tm_mday) // 在当月，且日大于当日，算上月遗留
		{
			memcpy(&tmpTime, oldTime, sizeof(struct tm));
		}
		else if (nCheckDay < oldTime->tm_mday) // 在上月
		{
			memcpy(&tmpTime, oldTime, sizeof(struct tm));
		}
	}

	if (CheckDayValide(nCheckDay, tmpTime.tm_mon + 1, tmpTime.tm_year+1900) != 1)
	{// 日期无效
		return 0;
	}
	tmpTime.tm_mday = nCheckDay;

	tmpTime.tm_hour = 0;
	tmpTime.tm_min = 0;
	tmpTime.tm_sec = 0;
	ulStart = mktime(&tmpTime);
	tmpTime.tm_hour = 23;
	tmpTime.tm_min = 59;
	tmpTime.tm_sec = 59;
	ulEnd = mktime(&tmpTime);
	return 1;
}

// 删除超过N天的数据
int CopyToUSB::ProcDataOutOfLimitDate(int nLimitDays)
{
	if (nLimitDays > 28 || nLimitDays <= 0)
	{
		return 0;
	}

	char szDest[20] = "/data/data/";
	char  buf[256];
	memset(buf,0, sizeof(buf));
	sprintf(buf,"%sVhipict",szDest);

	std::string strSubPath(buf);
	memset(buf,0,sizeof(buf));
	if(access(strSubPath.c_str(),0) != 0) 
	{// 根目录目录不存在直接返回
		return 0;
	}

	// 获取当前时间
	time_t now;
	time( &now );
	struct tm timenow;
	localtime_r( &now, &timenow );

	// 计算出N天前的时间
	time_t oldtime = now - ( nLimitDays * 3600 * 24 );
	struct tm timeold;
	localtime_r( &oldtime, &timeold );
	printf("oldTime[%04d-%02d-%02d], newTime[%04d-%02d-%02d] \n"
		, timeold.tm_year+1900, timeold.tm_mon+1, timeold.tm_mday
		, timenow.tm_year+1900, timenow.tm_mon+1, timenow.tm_mday);

	// 判断是否已跨月
	bool bSpanMonth = (timeold.tm_year != timenow.tm_year || timeold.tm_mon != timenow.tm_mon) ? true : false;

	bool bIsProcessed = false;
	// 依次遍历每一天，判断是否该天的文件夹是否存在
	for (int n=1; n<=31; n++)
	{
		if ((!bSpanMonth && n >= timeold.tm_mday && n <= timenow.tm_mday) // 非跨月
			|| (bSpanMonth && (n >= timeold.tm_mday || n <= timenow.tm_mday))) // 跨月
		{// 在时间区间范围内
			printf("DAY IN SPAN:%d \n", n);
			continue;
		}

		// 不在时间区间范围内则删除
		memset(buf,0, sizeof(buf));
		sprintf(buf,"%sVhipict/%02d",szDest, n);
		std::string strSubPathDay(buf);
		if(access(strSubPathDay.c_str(),0) == 0) //日期文件夹存在
		{
			bIsProcessed = true;
			printf("FILE exist NEED remove:%s \n", strSubPathDay.c_str());
			// 删除日期目录及目录下的文件
			RemoveDir2(strSubPathDay.c_str());
			//LogNormal("CopyToUSB delete filePath [%s]", strSubPathDay.c_str());

#ifdef SQLITE_OK
			// 删除sqlite中记录
			unsigned long ulStart=0, ulEnd=0;
			if (GetDayTimeSpan(n, &timenow, &timeold, ulStart, ulEnd) == 1)
			{
				char  szSql[256];
				memset(szSql,0, sizeof(szSql));
				std::string strStart = GetTime(ulStart, 0);
				std::string strEnd = GetTime(ulEnd, 0);
				sprintf(szSql,"delete * from PASSVEHICLE where TGSJ >= \"%s\" and TGSJ <= \"%s\";", strStart.c_str(), strEnd.c_str());
				std::string strSql(szSql);
				if (sqlteCp != NULL)
				{
					int nRet = sqlteCp->execsqlInsqlite(strSql);
					//LogNormal("CopyToUSB delete from sqlite [%s]", szSql);
				}
			}
#endif
		}
	}

	if (bIsProcessed)
	{
		return 1;
	}
	return 0;
}

// 删除超过n天的数据
int CopyToUSB::CheckDataOutOfDate()
{
	time_t now;
	time( &now );
	struct tm timenow;
	localtime_r( &now, &timenow );
	printf("m_nCheckTm[%04d-%02d-%02d], nowTime[%04d-%02d-%02d] \n"
		, m_nCheckTm.tm_year+1900, m_nCheckTm.tm_mon+1, m_nCheckTm.tm_mday
		, timenow.tm_year+1900, timenow.tm_mon+1, timenow.tm_mday);

	// 跨天才需要进行判断
	if (m_nCheckTm.tm_yday != timenow.tm_yday || m_nCheckTm.tm_mon != timenow.tm_mon || m_nCheckTm.tm_mday != timenow.tm_mday)
	{
		ProcDataOutOfLimitDate(USB_MAX_STORAGE_DAYS);
		// 当前检测前重新赋值
		memcpy(&m_nCheckTm, &timenow, sizeof(struct tm));
		return 1;
	}	
	return 0;
}

CopyToUSB::CopyToUSB()
{
	uPicSize = 0;
	uSmallPicSize = 0;
	uPicWidth = 0;
	uPicHeight = 0;
	uViolationType = 0;
	//线程ID
	m_nThreadId = 0;
	m_nThreadLogId = 0;
	m_nThread = 0;
	g_bEndToUsb = false;

	haveCopy_sql = 0;
	memset(&m_nCheckTm, 0, sizeof(struct tm));		
}

UINT32 CopyToUSB::GetHaveCopy_Sql()
{
	return haveCopy_sql;
}


bool CopyToUSB::Init()
{
    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
   
    if(pthread_create(&m_nThreadId,NULL,ThreadCopyFile,this)!=0)
    {
		printf("创建拷贝线程失败，无法拷贝！\r\n");
        Close();
		pthread_attr_destroy(&attr);
		return false;
    }
    pthread_attr_destroy(&attr);

	return true;
}

int CopyToUSB::CopyFile(char *copyfrom, char *copyto, UINT32 isize)
{
	printf("信息拷贝后到路径 :%s\n", copyto);
	UINT32 nTotalSize = isize;
	FILE  *wFile = NULL;
	UINT32 drBytes = 4096000;
	UINT32 dwBytes = 0;

	wFile = fopen(copyto, "w+b");
	printf("拷贝路径打开: fopen() ! file = %s\n", copyto);
	//cerr<<"fopen(): file = "<<copyto<<endl;
	if(NULL == wFile)
	{
		printf("拷贝路径打开失败 ! \n");
		return -1;
	}

	if (nTotalSize < drBytes)
	{
		drBytes = nTotalSize;
	}

	while((dwBytes = fwrite(copyfrom, 1, drBytes, wFile)) > 0)
	{

		nTotalSize -= dwBytes;

		if (nTotalSize < drBytes)
		{
			drBytes = nTotalSize;
		}

		if (nTotalSize <= 0)
		{
			break;
		}

		copyfrom = copyfrom + drBytes;
		//printf("write nTotalSize=%u, dwBytes=%d, drBytes=%d\n",nTotalSize,dwBytes,drBytes);	
	}
	printf("==================================COPY SUCCESS!\n");
	if (wFile != NULL)
	{
		fclose(wFile);
	}
	//usleep(100);
	return 0;
}


bool CopyToUSB::GetPlatePic()
{
	printf("CopyToUSB::GetPlatePic() BEGIN=============\n");
	char buf[1024]={0};
	char m_dest[20] = "/data/data/";
	
	unsigned int uTimeStamp = GetTimeStamp()-600;//600秒以前的认为是历史记录,防止实时记录正在发送又将其作为历史数据发送的情况
	string strTime = GetTime(uTimeStamp);
	if(g_DistanceHostInfo.uUsbCopyTime > 0)
	{
		string strUsbCopyTime = GetTime(g_DistanceHostInfo.uUsbCopyTime);
		//sprintf(buf,"select * from NUMBER_PLATE_INFO where PECCANCY_KIND > 0 and PIC_FLAG = 0  and TIME >= '%s' and TIME <= '%s' ORDER BY TIME asc limit 1;",strUsbCopyTime.c_str(),strTime.c_str());
		sprintf(buf,"select * from NUMBER_PLATE_INFO where PECCANCY_KIND > 0 and PIC_FLAG = 0 and TIME >= '%s' and TIME <= '%s' limit 100;",strUsbCopyTime.c_str(),strTime.c_str());
	}
	else
	{
		//sprintf(buf,"select * from NUMBER_PLATE_INFO where PECCANCY_KIND > 0 and PIC_FLAG = 0 and TIME <= '%s' ORDER BY TIME asc limit 1;",strTime.c_str());
		sprintf(buf,"select * from NUMBER_PLATE_INFO where PECCANCY_KIND > 0 and PIC_FLAG = 0 and TIME <= '%s' limit 100;",strTime.c_str());
	}
	
	string strSql(buf);
	MysqlQuery q = g_skpDB.execQuery(strSql);
	//LogNormal("wf plate: numFileds=%d \n",q.numFileds());
	unsigned int uMinID = 0;
	unsigned int uMaxID = 0;


	while (!q.eof()&&!g_bEndThread)
	{
		//判断磁盘空间是否已满
		int nLimitDays = USB_MAX_STORAGE_DAYS;
		while(!g_bEndThread)
		{
			struct statfs diskStatfs;
			statfs(g_usbMountPath.c_str(),&diskStatfs);
			unsigned long long freeBlocks = diskStatfs.f_bfree;
			unsigned long long freeSize = freeBlocks * diskStatfs.f_bsize;
			unsigned long long freeSpace = freeSize >> 20;
			if(freeSpace <= 20)
			{
				if (nLimitDays <= 1)
				{// 未触发删除
					printf("USB可用空间不足!请更换USB或清理USB! \n");
					q.finalize();
					return false;
				}
				// 先删除一天数据，再检查磁盘空间，删一天不够删两天
				ProcDataOutOfLimitDate(nLimitDays);
				nLimitDays--;
				sleep(1);
				continue;
			}
			else
			{
				break;
			}
		}
		
		//LogNormal("1111111111111111111\n");
		//从内存中获取数据库记录
		string strTi;
		string strYear;
		string strDay;
		string strMonth;
		string strHour;
		string strMin;
		string strSec;
		string strMit;
		int RoadWayID;
		string PannelID("");
		unsigned int uID; 
		
		bool nInsertSuccess = false;
		string strSqlnhs;
		strSqlnhs = "";
		string strVideo;
		bool videoFlag = false;

		string chPlace("");
		string strDbPath("");

		// 产生一个随机数
		int strWjkzm = GetRandCode();
		printf("================strWjkzm = %1d \n",strWjkzm);

		RECORD_PLATE plate;
		uID = q.getUnIntFileds("ID");
	
		if(uMaxID < uID)
		{
			uMaxID = uID;
		}

		if(uMinID == 0)
		{
			uMinID = uID;
		}
		else if(uMinID > uID)
		{
			uMinID = uID;
		}
		strPicPath = q.getStringFileds("PICPATH");
		uViolationType = q.getIntFileds("PECCANCY_KIND");
		
		if (uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
		{
			strTi = q.getStringFileds("TIMESECOND");
		}
		else
		{
			strTi = q.getStringFileds("TIME");
		}
		strYear = strTi.substr(0,4);
		strMonth = strTi.substr(5,2);
	    strDay = strTi.substr(8,2);
		strHour = strTi.substr(11,2);
		strMin = strTi.substr(14,2);
		strSec = strTi.substr(17,2);

		RoadWayID = q.getIntFileds("ROAD");//

		if(RoadWayID > 99)
		{
			RoadWayID %= 100; 
		}

		
		//PannelID = q.getStringFileds("PANNEL_ID");
		if(uViolationType == DETECT_RESULT_NOCARNUM)
		{
			uViolationType = 0;
			//UpdateRecordStatus(uID);
		}
		if (/*uViolationType == DETECT_RESULT_EVENT_GO_FAST || */uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
		{
			PannelID = q.getStringFileds("CrossingNumber");
			//cerr<<"1111111111uViolationType:"<<uViolationType<<" PannelID:"<<PannelID<<endl;
		}
		else
		{
			if (g_strDetectorID.size() < 5)//长度小于5默认为“AAAAA”
			{
				PannelID = "AAAAA";
			}
			else
			{
				PannelID = g_strDetectorID.substr(0,5);
			}
			//cerr<<"222222uViolationType:"<<uViolationType<<" PannelID:"<<PannelID<<endl;
		}
		printf("================PannelID = %5s \n",PannelID);
		uPicSize = q.getIntFileds("PICSIZE");
		uSmallPicSize = q.getIntFileds("SMALLPICSIZE");
		strVideoPath = q.getStringFileds("VIDEOPATH");
		chPlace = q.getStringFileds("PLACE");
		//int catCopy = q.getIntFileds("VIDEOSAVE");
printf("-----strVideoPath = %s \n",strVideoPath.c_str());
		if (strVideoPath.size() > 0 && g_DistanceHostInfo.bCopyVideo > 0)
		{
			string strPath1("");
			string strAbsPath = g_strVideo;
			int found = strVideoPath.find("/");
			if (found != string::npos)
			{
				found = strVideoPath.find("/",found+2);
			}

			if (found < 0)
			{
				printf("---------------------\n");
				videoFlag =false;
			}
			else if (found > 0)
			{
				printf("----------------------2\n");
				printf("-----found = %d \n",found);
				size_t findPoint = strVideoPath.find(".",found + 1);
				strPath1 = strVideoPath.substr(found);
				printf("-----strPath1 = %s \n",strPath1.c_str());
				string strPath2 = strVideoPath.substr(findPoint);
				printf("-----strPath2 = %s \n",strPath2.c_str());
				strAbsPath = strAbsPath + strPath1;
				printf("-----videoPath = %s \n",strVideoPath.c_str());
				strVideo = GetVideoByPath(strAbsPath);
				videoFlag = true;
			}
			
			if (uViolationType == 6 || uViolationType == 16/* || catCopy == 0*/)
			{
				videoFlag =false;
			}
		}
	
		#ifdef SQLITE_OK

			char ch[128] = {0};
			string XH("NULL");					
			string HPHM(q.getStringFileds("NUMBER"));
			
			memset(ch, '\0', sizeof ch);
			sprintf(ch, "%d", sqlteCp->carType(q.getIntFileds("TYPE"), q.getStringFileds("NUMBER")) );
			string HPZL(ch);
			
			string TGSJ(q.getStringFileds("TIME"));
			string TGSJ1 = TGSJ;
			if(uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
			{
				string strTmp(q.getStringFileds("TIMESECOND"));
				TGSJ = strTmp;
			}
			
			string CLSD(q.getStringFileds("SPEED"));
			
			plate.uSpeed = q.getIntFileds("SPEED");

			//CMvsCommunication temp;
			//int limitSpeed = temp.GetSpeed(q.getIntFileds("CHANNEL"), RoadWayID);
			UINT32 nChannel = q.getIntFileds("CHANNEL");
			int limitSpeed = GetMaxSpeed(plate.uType, nChannel,RoadWayID);

			memset(ch, '\0', sizeof(ch));
			sprintf(ch, "%d", limitSpeed);
			string XZSD(ch);               //限速值
			//cerr<<"------限速值------------"<<XZSD<<endl;

			string CLCD("0");             //车长
		
			memset(ch, '\0', sizeof ch);
			sprintf(ch, "%d", sqlteCp->carColor(q.getIntFileds("CARCOLOR")) );
			string CSYS(ch);   			 //	  char
			
			memset(ch, '\0', sizeof ch);
			sprintf(ch, "%d", strWjkzm);
			string WJKZM(ch);		     //	                  WJKZM
			
			memset(ch, '\0', sizeof ch);
			sprintf(ch, "%d", sqlteCp->wfxw(uViolationType));
			string WFXW(ch);

			memset(ch, '\0', sizeof ch);
			if (uViolationType == 54)
			{
				sprintf(ch, "2");
			}
			else
			{
				sprintf(ch, "%d", sqlteCp->wflx(sqlteCp->wfxw(uViolationType)) );
			}
			
			string WFLX(ch);

			memset(ch, '\0', sizeof ch);
			sprintf(ch, "%d", q.getIntFileds("REDLIGHT_TIME") );
		//	LogError("hongdeng shijian %s", ch);
			string HDSJ(ch);			 //					  HDSJ
			
			char strCh[8] = {0};
			if (uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
			{
				RoadWayID = 1;
				sprintf(strCh, "%5s%02d",PannelID,1);//区间超速的车道编号为1
			}
			else
			{
				sprintf(strCh, "%5s%02d",PannelID,RoadWayID);
			}
			string LKHM(strCh);			 //					  LKHM 
			
			char ch2[10] = {0};
			sprintf(ch2, "%5s", PannelID);
			string SBHM(ch2);			 //					  SBHM      char
			string CSDM("6");			 //					  CSDM      char
			memset(ch, '\0', sizeof ch);
			sprintf(ch, "%d", sqlteCp->carLabel(q.getIntFileds("FACTORY")) );
			string CLBZ(ch);

			string SPLX("2");			 //					  SPLX 
			string SPLJ("0");			 //					  SPLJ       char 

			if(videoFlag)
			{
				SPLX = "0";
				sprintf(ch, "..\\Vhipict\\%2s\\%2s\\%5s%02d-%4s%2s%2s%2s%2s%2s-%d.avi",strDay.c_str(),strHour.c_str(),PannelID,RoadWayID,strYear.c_str(),strMonth.c_str(),strDay.c_str(),strHour.c_str(),strMin.c_str(),strSec.c_str(),strWjkzm);
				SPLJ = ch;
			}

			strSqlnhs.append("insert into PASSVEHICLE values(");
			strSqlnhs.append(XH);				strSqlnhs.append(",'");   // 序列号
			strSqlnhs.append(HPHM);			    strSqlnhs.append("',");   // 车牌号码     char
			strSqlnhs.append(HPZL);			    strSqlnhs.append(",'");   // 号牌类型
			strSqlnhs.append(TGSJ);             strSqlnhs.append("',");   // 通过时间
			strSqlnhs.append(CLSD);             strSqlnhs.append(",");     // 车速
			strSqlnhs.append(XZSD);				strSqlnhs.append(",");     // 限速
			strSqlnhs.append(CLCD);             strSqlnhs.append(",'");	 // 车长
			strSqlnhs.append(CSYS);				strSqlnhs.append("',");     // 车身颜色     char
			strSqlnhs.append(WJKZM);		    strSqlnhs.append(",");     // 文件扩展名
			strSqlnhs.append(WFLX);             strSqlnhs.append(",'");     // 违法分类
			strSqlnhs.append(WFXW);             strSqlnhs.append("',");     // 违法行为代码 char
			strSqlnhs.append(HDSJ);             strSqlnhs.append(",'");     // 红灯时间
			strSqlnhs.append(LKHM);             strSqlnhs.append("','");	 // 路口号       char 
			strSqlnhs.append(SBHM);             strSqlnhs.append("','");     // 设备号       char
			strSqlnhs.append(CSDM);             strSqlnhs.append("',");     // 厂商代码     char
			strSqlnhs.append(CLBZ);             strSqlnhs.append(",");     // 车辆 厂商标志
			strSqlnhs.append(SPLX);             strSqlnhs.append(",'");     // 视频类型
			strSqlnhs.append(SPLJ);             strSqlnhs.append("','");;     // 保存视频的相对路径  char
			strSqlnhs.append(TGSJ1);             strSqlnhs.append("'); ");   // 通过时间1

	#endif
		//	LogNormal("222222222222222222222222222222\n");
		std::string strOne("");
		std::string strTwo("");
		if (strPicPath.size() > 0)
		{
			if (uViolationType > 0) 
			{
				std::string strPic = GetImageByPath(strPicPath);
			
				//违法
				if(strPic.size() > 0)
				{
					//LogNormal("3333333333333333333333333333333\n");
					char path1[128];
					char path2[128];
					char saveVideoPath[128] = {0} ;
					char  buf[256];
					char chDbBuf[128];
					strOne = "";
					strTwo = "";
					memset(path1,'\0', sizeof(path1));
					memset(path2,'\0', sizeof(path2));
					memset(buf,'\0', sizeof(buf));
					memset(chDbBuf,0,sizeof(chDbBuf));

					
						if(access(m_dest,0) != 0) //目录不存在
						{
							if (HasUsb() == false)
							{
								q.finalize();
								return false;
							}
							printf("====create dir : %s \n",m_dest);
							mkdir(m_dest,S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
							sync();
						}

						//sprintf(buf,"%sVhipict",m_dest);
						if (uViolationType == DETECT_RESULT_EVENT_GO_FAST || uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
						{
							string strPlace = chPlace;
							g_skpDB.UTF8ToGBK(strPlace);
							sprintf(buf,"%s%s/Vhipict",m_dest,strPlace.c_str());
							sprintf(chDbBuf,"%s%s",m_dest,strPlace.c_str());
							
							string newSqlPath = "mkdir -p ";	
							newSqlPath += buf;
							if (HasUsb() == false)
							{
								q.finalize();
								return false;
							}
							system(newSqlPath.c_str());
							//LogNormal("newSqlPath=%s\n",newSqlPath.c_str());
						}
						else
						{
							sprintf(buf,"%sVhipict",m_dest);
						}
					
	#ifdef SQLITE_OK

						if (uViolationType == DETECT_RESULT_EVENT_GO_FAST || uViolationType == DETECT_RESULT_EVENT_DISTANCE_FAST)
						{
							//LogNormal("chDbBuf=%s\n",chDbBuf);
							if (sqlteCp->insetIntosqliteMessage(chDbBuf,strSqlnhs) == -1)
							{
								q.finalize();
								return false;
							}
							else
							{
								nInsertSuccess = true;
							}

						}
						else
						{
							if(sqlteCp->insetIntosqliteMessage(strSqlnhs) == -1)
							{
								q.finalize();
								return false;
							}
							else
							{
								nInsertSuccess = true;
							}
						}
	#endif
					//LogNormal("555555555555555555555555\n");
						if (nInsertSuccess)
						{
							std::string strSubPath1(buf);
							memset(buf,0,sizeof(buf));
							if(access(strSubPath1.c_str(),0) != 0) //根目录目录不存在
							{
								printf("====create dir : %s \n",strSubPath1.c_str());
								if (HasUsb() == false)
								{
									q.finalize();
									return false;
								}
								mkdir(strSubPath1.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
								sync();
							}
							sprintf(buf,"%s/%2s",strSubPath1.c_str(),strDay.c_str());
						//	sprintf(buf,"%s/%8s",strSubPath1.c_str(),strYmd.c_str());
							std::string strSubPath2(buf);
							memset(buf,0,sizeof(buf));
							if(access(strSubPath2.c_str(),0) != 0) //日目录不存在
							{
								printf("====create dir : %s \n",strSubPath2.c_str());
								if (HasUsb() == false)
								{
									q.finalize();
									return false;
								}
								mkdir(strSubPath2.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
								sync();
							}

							sprintf(buf,"%s/%2s",strSubPath2.c_str(),strHour.c_str());
							std::string savedir(buf);
							memset(buf,0,sizeof(buf));
							if(access(savedir.c_str(),0) != 0) //时目录不存在
							{
								printf("====create dir : %s \n",savedir.c_str());
								if (HasUsb() == false)
								{
									q.finalize();
									return false;
								}
								mkdir(savedir.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
								sync();
							}
							printf("====savedir: %s \n",savedir.c_str());
							//图片目标路径
							sprintf(path1, "%s/%5s%02d-%4s%2s%2s%2s%2s%2s-%d.jpg",savedir.c_str(),PannelID,RoadWayID,strYear.c_str(),strMonth.c_str(),strDay.c_str(),strHour.c_str(),strMin.c_str(),strSec.c_str(),strWjkzm);
							//视频目标路径
							if (videoFlag)
							{
								sprintf(saveVideoPath, "%s/%5s%02d-%4s%2s%2s%2s%2s%2s-%d.avi", savedir.c_str(),PannelID,RoadWayID,strYear.c_str(),strMonth.c_str(),strDay.c_str(),strHour.c_str(),strMin.c_str(),strSec.c_str(),strWjkzm);

								if (strVideo.size() > 0)
								{
									if (CopyFile((char *)strVideo.c_str(), saveVideoPath, strVideo.size()) == -1)
									{
										//cerr<<"USB "<<  strVideo.c_str() <<"to "<<saveVideoPath<<"failed_video"<<endl;
										UpdateRecordStatus(uID);
										q.nextRow();
										continue;
									}
								}
							}
							
							int nPos = strPicPath.find("hc1");
							if (nPos <= 0)
							{
								CxImage image;
								image.Decode((unsigned char*)strPic.c_str(),strPic.size(),3);//先解码

								if(image.IsValid()&&image.GetSize()>0)
								{
										uPicWidth = image.GetWidth();
										uPicHeight = image.GetHeight();
										int srcstep = 0;
										CxImage image1;
										UINT32 uImageSize = uPicWidth*uPicHeight*3;
										unsigned char* pImageData = new unsigned char[uImageSize/2];
										unsigned char* pJpgImage = new unsigned char[uImageSize/2];
										memcpy(pImageData,image.GetBits(0),uImageSize/2);
										{
											image1.IppEncode(pImageData,uPicWidth,uPicHeight/2,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
										}
										strOne.append((char *)pJpgImage, srcstep);
										if (CopyFile((char *)strOne.c_str(),path1,srcstep) == -1)
										{
											LogNormal("USB %s failed ! \n", path1);
											//cerr<<"USB "<<strOne.c_str()<<"to "<< path1<<"failed ! "<<endl;
											if (pImageData)
											{
												delete []pImageData;
											}
											pImageData = NULL;

											if (pJpgImage)
											{
												delete []pJpgImage;
											}
											pJpgImage = NULL;
											UpdateRecordStatus(uID);
											q.nextRow();
											continue;
										}
									
										srcstep = 0;
										memset(pJpgImage,'\0',sizeof(pJpgImage));

										sprintf(path2, "%s/%5s%02d-%4s%2s%2s%2s%2s%2s-%d-1.jpg",savedir.c_str(),PannelID,RoadWayID,strYear.c_str(),strMonth.c_str(),strDay.c_str(),strHour.c_str(),strMin.c_str(),strSec.c_str(),strWjkzm);
										
										memcpy(pImageData,image.GetBits(uPicHeight/2),uImageSize/2);
										image1.IppEncode(pImageData,uPicWidth,uPicHeight/2,3,&srcstep,pJpgImage,g_PicFormatInfo.nJpgQuality);
										strTwo.append((char *)(pJpgImage), srcstep);

										if (CopyFile((char *)strTwo.c_str(), path2, srcstep) == -1)
										{
											LogNormal("USB %s failed ! \n", path2);
											//cerr<<"USB "<<strTwo.c_str()<<"to "<< path2<<"failed ! "<<endl;
											if (pImageData)
											{
												delete []pImageData;
											}
											pImageData = NULL;

											if (pJpgImage)
											{
												delete []pJpgImage;
											}
											pJpgImage = NULL;
											UpdateRecordStatus(uID);
											q.nextRow();
											continue;
										}

										if (pImageData)
											{
												delete []pImageData;
											}
											pImageData = NULL;

											if (pJpgImage)
											{
												delete []pJpgImage;
											}
											pJpgImage = NULL;
								}
							}
							else
							{
								if (CopyFile((char *)strPic.c_str(),path1,strPic.size()) == -1)
								{
									LogNormal("USB %s failed ! \n", path1);
									//cerr<<"USB "<<strOne.c_str()<<"to "<< path1<<"failed ! "<<endl;
									UpdateRecordStatus(uID);
									q.nextRow();
									continue;
								}
								
								strPicPath.replace(nPos+2,1,"2");
								printf("strTwo strPicPath=%s\n",strPicPath);
								strTwo = GetImageByPath(strPicPath);

								if(strTwo.size() > 0)
								{
									sprintf(path2, "%s/%5s%02d-%4s%2s%2s%2s%2s%2s-%d-1.jpg",savedir.c_str(),PannelID,RoadWayID,strYear.c_str(),strMonth.c_str(),strDay.c_str(),strHour.c_str(),strMin.c_str(),strSec.c_str(),strWjkzm);

									if (CopyFile((char *)strTwo.c_str(), path2, strTwo.size()) == -1)
									{
										LogNormal("USB %s failed ! \n", path2);
										//cerr<<"USB "<<strTwo.c_str()<<"to "<< path2<<"failed ! "<<endl;
										UpdateRecordStatus(uID);
										q.nextRow();
										continue;
									}
								}
							}
						}
					
				}
			}
		}
		
		q.nextRow();
		sleep(1);
	}
	//LogNormal("uMinID=%u,uMaxID=%u\n",uMinID,uMaxID);
	q.finalize();
	UpdateRecordStatus(uMinID,uMaxID);
	return true;
}

//更新记录状态
bool CopyToUSB::UpdateRecordStatus(unsigned int uID)
{
	char bufTmp[1024];
	memset(bufTmp, 0, 1024);
	sprintf(bufTmp, "update NUMBER_PLATE_INFO set PIC_FLAG = 1 where ID = %u;",uID);
	g_skpDB.execSQL(string(bufTmp));

	return true;
}

//更新记录状态
bool CopyToUSB::UpdateRecordStatus(unsigned int uMinID,unsigned int uMaxID)
{
	char bufTmp[1024];
	memset(bufTmp, 0, 1024);
	sprintf(bufTmp, "update NUMBER_PLATE_INFO set PIC_FLAG = 1 where ID >= %u and ID <= %u;",uMinID,uMaxID);
	g_skpDB.execSQL(string(bufTmp));

	return true;
}

bool CopyToUSB::Close()
{
	//g_bEndToUsb = false;
	//printf("CopyToUSB::Close == unload the mobile device !\n");
}

//获取随机防伪码
int CopyToUSB::GetRandCode()
{
	//srand( (unsigned)time( NULL ) );
	getpid();
	int nRandCode = 1 + (int)(9*1.0*rand()/(RAND_MAX+1.0));
	return nRandCode;
}

CopyToUSB::~CopyToUSB()
{
	//printf("CopyToUSB::~CopyToUSB == unload the mobile device !\n");
}
