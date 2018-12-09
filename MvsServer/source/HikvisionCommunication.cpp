#include "Common.h"

#ifdef HIKVISIONCAMERA
#include "HikvisionCommunication.h"
#include "CSeekpaiDB.h"

FILE *log_file = NULL;
void open_log_file(const char *filename)
{
	log_file = fopen(filename, "w");
}

void close_log_file()
{
	if (log_file != NULL)
	{
		fflush(log_file);
		fclose(log_file);
		log_file = NULL;
	}
}

void log_info_callback(int lvl, const char *log_info)
{
	//if (lvl == WARN_LVL || lvl == ERROR_LVL)	// only warn and error info
	{
		printf("%s\n", log_info);
	}

	if (log_file != NULL);
	{
		fprintf(log_file, "%d: %s\n", lvl, log_info);
		fflush(log_file);
	}
}

CHikvisionCommunication g_HikvisionCommunication;

void* ThreadLoginHikServer(void* pArg)
{
	//取类指针
	CHikvisionCommunication* pRoadDetect = (CHikvisionCommunication*)pArg;
	if(pRoadDetect == NULL) 
		return 0;

	pRoadDetect->LoginHikServer();
	pthread_exit((void *)0);
	return pArg;
}

CHikvisionCommunication::CHikvisionCommunication()
{
	m_bLogOnServer = false;
	m_Userhandle  = -2;
}

CHikvisionCommunication::~CHikvisionCommunication()
{

}

bool CHikvisionCommunication::Init()
{
	open_log_file("./hikplat.log");
	Plat_SetLogCallback(log_info_callback);

	const char *sdk_version = Plat_GetVersion();
	printf("Hikvision iVMS8200 \n Platform SDK v%s\n Demo v1.0.0", sdk_version);

	m_bLogOnServer = false;
	m_nInit = Plat_Init();

	printf("00 before Plat_Login===ip=%s,port=%d,user=%s,passwd=%s\n",g_monitorHostInfo.chMonitorHost,g_monitorHostInfo.uMonitorPort,g_monitorHostInfo.chUserName,g_monitorHostInfo.chPassWord);
	m_Userhandle = Plat_Login(g_monitorHostInfo.chMonitorHost,g_monitorHostInfo.uMonitorPort,g_monitorHostInfo.chUserName,g_monitorHostInfo.chPassWord);
	printf("00 after Plat_Login m_Userhandle=%d\n", m_Userhandle);

	//线程ID
	m_nThreadId = 0;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	int nret=pthread_create(&m_nThreadId,NULL,ThreadLoginHikServer,this);

	if(nret!=0)
	{
		//失败
		LogError("创建h264采集线程失败,无法进行采集！\r\n");
	}
	pthread_attr_destroy(&attr);

	return true;
}

bool CHikvisionCommunication::UserLogOnServer()
{
	//m_Userhandle = Plat_Login(g_monitorHostInfo.chMonitorHost,g_monitorHostInfo.uMonitorPort,g_monitorHostInfo.chUserName,g_monitorHostInfo.chPassWord);
	
	if (m_Userhandle == -1)
	{
		LogError("Login error");
		g_nLoginState = 2;
		m_bLogOnServer = false;
	}
	else
	{
		LogNormal("login success!");
		g_nLoginState = 1;

	}
	return true;
}

bool CHikvisionCommunication::UnInit()
{
	g_nLoginState = 0;
	if ( m_Userhandle > 0)
	{
		int	nLogout = Plat_Logout(m_Userhandle);//退出
		if (nLogout == 0)
		{
			printf("退出成功\n");
		}
		else
		{
			printf("退出失败\n");
		}
	}

	Plat_UnInit();

	close_log_file();

	return true;
}
void CHikvisionCommunication::LoginHikServer()
{
	while(!g_bEndThread)
	{
		while(!m_bLogOnServer)
		{
			m_bLogOnServer = UserLogOnServer();
			sleep(5);
		}

		sleep(5);
	}
}

#endif

