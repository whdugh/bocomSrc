#include "DeviceStatus.h"

DeviceStatus g_CheckupDeviceStatus;


void * ThreadCheckupDeviceStatus(void *)
{
	g_CheckupDeviceStatus.GetMessageAndSave();
}

// 构造函数
DeviceStatus::DeviceStatus()
{

}

//析构函数
DeviceStatus::~DeviceStatus()
{

}
int DeviceStatus::m_saitci_ch_ok = 0;

// 启动一个线程 
void DeviceStatus::Init()
{
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	// 启动记录发送线程
	int nret = pthread_create(&id, &attr, ThreadCheckupDeviceStatus, NULL);

}

// 得到一些数据 然后把这些数据保存到 DEVICE_STATUS这个表中
int DeviceStatus::GetMessageAndSave()
{

	ProgramStart();                  //程序刚运行的时候执行这个 
	int iClock = 0;

	string jxdm(Dispose(g_ServerHost));               //机箱代码
	string date = GetTimeCurrent();                   // 时间

	char chSql_jxcq[256] = {0};      // 机器重启  
	sprintf(chSql_jxcq, "INSERT INTO DEVICE_STATUS values('%s', '%s', 'JQCQ', 1, ' '); ", jxdm.c_str(), date.c_str());    // 机器重启
	g_skpDB.execSQL(string(chSql_jxcq));

	while(!g_bEndThread)                       //  如果状态发生变化则 记录不发生变化则不记录
	{
		
		iClock++;

		if (m_saitci_ch_ok != m_changeIsok)
		{
			char chSql_ntp[256] = {0};
			if (m_changeIsok != 0)
			{
				sprintf(chSql_ntp, "INSERT INTO DEVICE_STATUS values('%s', '%s', 'NTP', 0, ' ');", jxdm.c_str(), date.c_str());
			}
			else
			{
				sprintf(chSql_ntp, "INSERT INTO DEVICE_STATUS values('%s', '%s', 'NTP', 1, ' ');", jxdm.c_str(), date.c_str());
			}
			printf("sql dev -------\%s \n", chSql_ntp);
			g_skpDB.execSQL(string(chSql_ntp));
			m_changeIsok = m_saitci_ch_ok;
		}

		sleep(1);
		if (iClock >= 3600)
		{
			iClock = 0;
			OneHourCheckDisk();
		}
	}
	return 1;
}

int DeviceStatus::OneHourCheckDisk()
{

	if (GetDiskFree() != m_diskSurplus)
	{
		string jxdm;
		jxdm = Dispose(g_ServerHost);                    //机箱代码
		string date = GetTimeCurrent();                  // 时间
		char chSql_ypsykl[256] = {0};
		sprintf(chSql_ypsykl, "INSERT INTO DEVICE_STATUS values('%s', '%s', 'YPSYKJ', %d, ' '); ", jxdm.c_str(), date.c_str(), GetDiskFree()); //硬盘剩余空间
		printf("sql dev -------\%s \n", chSql_ypsykl);
		g_skpDB.execSQL(string(chSql_ypsykl));
	}
	
	//删除一个月前的记录  // 只保存一个月的记录
	DeleMessage();       
	
	return 1;
}

int DeviceStatus::ProgramStart()
{

	int ntpChangeIsOk;
	string jxdm;
	jxdm = Dispose(g_ServerHost);              //机箱代码
	m_idAddr = jxdm;
	
	m_diskSurplus = GetDiskFree();

	string date = GetTimeCurrent();            // 时间

	string spcjk;                              //视频采集卡

	if (g_nClockMode == 0 )                    // 如果是 ntp校时
	{
		ntpChangeIsOk = m_saitci_ch_ok;
	}
	else 
	{
		ntpChangeIsOk = 0;
	}

	
	char  chSql_spcjk[256] = {0};    
	char  chSql_ypsykl[256] = {0};
	char  chSql_jxcq[256] = {0};
	char  chSql_ntp[256] = {0};
	int ntp = 0;
	if (ntpChangeIsOk ==0)
	{
		ntp = 0;
	}
	else
		ntp = 1;

	m_changeIsok = m_saitci_ch_ok ;
	sprintf(chSql_spcjk, "INSERT INTO DEVICE_STATUS values('%s', '%s', 'SPCJK', 1, ' '); ", jxdm.c_str(),date.c_str());  // 视频采集卡
	sprintf(chSql_ypsykl,"INSERT INTO DEVICE_STATUS values('%s', '%s', 'YPSYKJ', %d, ' '); ", jxdm.c_str(), date.c_str(), GetDiskFree()); //硬盘剩余空间
	sprintf(chSql_jxcq, "INSERT INTO DEVICE_STATUS values('%s', '%s', 'JQCQ', 1,  ' '); ", jxdm.c_str(), date.c_str());    // 机器重启
	sprintf(chSql_ntp,  "INSERT INTO DEVICE_STATUS values('%s', '%s', 'NTP', %d,  ' ');", jxdm.c_str(), date.c_str(),ntp);   //ntp校时

	printf("sql dev -------\%s \n", chSql_spcjk);
	printf("sql dev -------\%s \n", chSql_ypsykl);
	printf("sql dev -------\%s \n", chSql_jxcq);
	printf("sql dev -------\%s \n", chSql_ntp);
	g_skpDB.execSQL(string(chSql_spcjk));      
	g_skpDB.execSQL(string(chSql_ypsykl));
	g_skpDB.execSQL(string(chSql_jxcq));
	g_skpDB.execSQL(string(chSql_ntp));


	//删除一个月前的记录  // 只保存一个月的记录
	DeleMessage();       
	return 1;
}

int DeviceStatus::DeleMessage()
{
	long ltime = time((time_t *)NULL);
	ltime = ltime - 60*60*24*30;
	string tempTime = GetTime(ltime);
	char delsql[256] = {0};
	sprintf(delsql, "delete from DEVICE_STATUS where JCSJ < '%s' ;", GetTime(ltime).c_str());
	printf("\n del message-------------%s \n", delsql);
	g_skpDB.execSQL(string(delsql));

	return 1;
}

int DeviceStatus::GetDiskFree()
{

	int have =g_sysInfo_ex.fTotalDisk  * (100 - g_sysInfo.fDisk)/100;

	return have;
}

string  DeviceStatus::Dispose(string &str )
{
	int index = str.find(".");                            // 1
	string str1 = str.substr(0, index);                
  
	int end = str.find(".", index+1);                     // 2
	string str2 = str.substr(index+1, end-index-1);  

	index = end;                                        // 3
	end = str.find(".", index+1);
	string str3 = str.substr(index+1, end-index-1);

	string str4 = str.substr(end+1, str.length()-end);      // 4

	printf("%s----%s----%s----%s---- \n",str1.c_str(), str2.c_str(), str3.c_str(), str4.c_str());

	int i1 = atoi(str1.c_str());
	int i2 = atoi(str2.c_str());
	int i3 = atoi(str3.c_str());
	int i4 = atoi(str4.c_str()); 
	char buf[30] = {0};
	sprintf(buf, "%03d%03d%03d%03d", i1, i2, i3, i4);
	printf("%s\n", buf);
	return string(buf);
}