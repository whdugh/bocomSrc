// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/******************************************************************************/
//	描述:SNMP协议agent代理端
//	作者:高翔
//	日期:2011-12-12
/******************************************************************************/

#ifdef SNMP_AGENT
#include <iostream>
#include "Common.h"
#include "RoadXmlValue.h"
#include "CSeekpaiDB.h"
#include "XmlParaUtil.h"
#include "SnmpAgent.h"
#include "CenterServer.h"

SnmpAgent g_SnmpAgent;

int SnmpAgent::m_nReadSpeed = 0;
int SnmpAgent::m_nWriteSpeed = 0;

extern "C" void init_vacm_vars();
extern "C" void init_usmUser();
extern "C" void init_mib_modules();
extern "C" int header_generic(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);

//Set操作句柄函数
int SnmpSetHaldle(int action,
				   u_char * var_val,
				   u_char var_val_type,
				   size_t var_val_len,
				   u_char * statP, oid * name, size_t name_len);

//MIB结构体
struct variable2 RootVar[] = { //数字2表示后一个参数的位数,{2,1}表示OID的子序列
	{NE_TYPE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {2,1}},
	{SERIAL_NUMBER, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,2}},
	{MIB_VERSION, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,3}},
	{HARDWARE_VERSION, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,4}},
	{SOFTWARE_VERSION, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,5}},
	{LABEL, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,6}},
	{LOCATION, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,7}},
	{NET_ADDR, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,8}},
	{NET_MASK, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,9}},
	{NET_GATEWAY, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,10}},
	{MAC, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,11}},
	{NTP_USED, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {2,12}},
	{NTP_Server, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,13}},
	{CONTACT, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,15}},
	{PRODUCT_CODE, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,16}},
	{CURRENT_TIME, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {2,18}},
	{DESCRIPTION, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,19}},
	{TRAP_IP, ASN_OCTET_STR, RWRITE, SnmpAgent::SnmpHandle, 2, {2,22}},
	{TRAP_PORT, ASN_INTEGER, RWRITE, SnmpAgent::SnmpHandle, 2, {2,23}},
	{OS_RELESE, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,24}},
	{KERNEL_RELESE, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,25}},
	{HOST_NAME, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,26}},
	{SYSTEM_UPTIME, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {2,27}},
	{SYSTEM_BITS, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {2,30}},
	{SYSTEM_FILE_TYPE, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {2,31}},
	{SYS_TEMPERATURE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {2,32}},
	{SYN_TIME, ASN_OCTET_STR, RWRITE, SnmpAgent::SnmpHandle, 2, {2,33}}
};

//MIB结构体-CPU
struct variable2 CpuVar[] = {
	{CPU_RUNTIME, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,1}},
	{CPU_USED, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,2}},
	{CPU_USER, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,3}},
	{CPU_SYSTEM, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,4}},
	{CPU_IDLE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,5}},
	{CPU_IOWAIT, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,6}},
	{CPU_NAME, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {1,7}},
	{CPU_TEMPERATURE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,8}},
	{CPU_FUNSPEED, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,9}}
};

//MIB结构体-内存
struct variable2 MemVar[] = {
	{MEM_TOTAL, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,1}},
	{MEM_USED, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,3}},
	{MEM_SWAP_TOTAL, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,2}},
	{MEM_SWAP_USED, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,4}}
};

//MIB结构体-硬盘
struct variable2 DiskVar[] = {
	{DISK_ID, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {1,1}},
	{DISK_MOUNTED, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,2}},
	{DISK_CANWRITE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,3}},
	{DISK_MOUNTPATH, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {1,4}},
	{DISK_NAME, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {1,5}},
	{DISK_TOTAL_SIZE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,6}},
	{DISK_USED_SIZE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,7}},
	{DISK_READRATE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,8}},
	{DISK_WRITERATE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,9}},
	{DISK_TEMPERATURE, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,10}}
};

//MIB结构体-网络                          ///-------
struct variable2 NetWorkVar[] = {
	{NET_IP_ADDR, ASN_OCTET_STR, RONLY, SnmpAgent::SnmpHandle, 2, {1,1}},
	{NET_LINKED, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,2}},
	{NET_RX, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,3}},
	{NET_TX, ASN_INTEGER, RONLY, SnmpAgent::SnmpHandle, 2, {1,4}}
};

string m_strSnmpLocation = "";

//SNMP服务线程
void* OpenService(void*)
{
	int agentx_subagent=0; //是否作为子agent启动
	int syslog = 0; //是否输出信息到log文件

	//打印信息输出到log文件或屏幕
	if (syslog)
	{
		snmp_enable_calllog();
	}
	else
	{
		snmp_enable_stderrlog();
	}

	if (agentx_subagent) {
		//作为子agent启动
		netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
	}

	//初始化服务
	SOCK_STARTUP;

	//初始化snmp服务, 默认路径下读入的配置文件snmpd.conf
	init_agent("snmpd");

	//初始化连接控制服务
	if (!agentx_subagent) {
		init_vacm_vars();
		init_usmUser();
	}

	//初始化snmp服务, 默认路径下读入的配置文件snmpd.conf
	init_snmp("snmpd");

	//加入系统MIB节点
	//init_mib_modules();

	//加入私有MIB节点
	g_SnmpAgent.InitBocomMibOid();
	
	if (!agentx_subagent)
		init_master_agent(); //作为主agent打开监控, 默认161端口

	snmp_log(LOG_INFO,"snmpagent is up and running.\n");

	int fds = 0, block = 0;
    fd_set fdset;

	struct timeval timeout;
	while(g_bEndThread == false) 
	{
		//10秒做一次超时轮询
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		snmp_select_info(&fds, &fdset, block ? NULL : &timeout, &block);
        fds = select(fds, &fdset, NULL, NULL, &timeout);//非阻塞等待
        if (fds)
        {
        	//收get请求并自动发送response
			agent_check_and_process(1); /* 0 == 不阻塞 */
        }
		g_SnmpAgent.CheckSystemStatus();
		usleep(100);
	}

	snmp_shutdown("snmpd");
	SOCK_CLEANUP;

	return NULL;
}

//Set操作句柄函数
int SnmpSetHaldle(int action,
				 u_char * var_val,
				 u_char var_val_type,
				 size_t var_val_len,
				 u_char * statP, oid * name, size_t name_len)
{
	CXmlParaUtil xml;
	string oidName;
	string timeStr;
	char buf[128];
	for (int i = 0; i < name_len; i++)
	{
		sprintf(buf, ".%d", name[i]);
		oidName += buf;
	}
	switch (action) 
	{
	case RESERVE1:
		if (oidName == ".1.3.6.1.4.1.31492.2.33.0") //Set操作用同步时间
		{
			if (var_val_type != ASN_OCTET_STR) {
				DEBUGMSGTL(("snmpagent", "%x not string type", var_val_type));
				return SNMP_ERR_WRONGTYPE;
			}

			snprintf(buf, var_val_len+1, "%s", var_val);
			UINT64 timeStap = (UINT64)atol(buf);

			struct timeval timer;
			timer.tv_sec = timeStap/1000;
			timer.tv_usec = timeStap%1000;
			settimeofday(&timer, NULL);
		}
		else if (oidName == ".1.3.6.1.4.1.31492.2.22.0") //Trap 目标IP
		{
			if (var_val_type != ASN_OCTET_STR) {
				DEBUGMSGTL(("snmpagent", "%x not string type", var_val_type));
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len < 7 || var_val_len > 15) {
				DEBUGMSGTL(("snmpagent", "wrong length %x", var_val_len));
				return SNMP_ERR_WRONGLENGTH;
			}
			g_strSnmpHost = "";
			g_strSnmpHost.append((char*)var_val, var_val_len);
			xml.UpdateSystemSetting("SnmpServerHostSetting", "");
		}
		else if (oidName == ".1.3.6.1.4.1.31492.2.23.0") //Trap 目标Port
		{
			if (var_val_type != ASN_INTEGER) {
				DEBUGMSGTL(("snmpagent", "%x not integer type", var_val_type));
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len <= sizeof(int)) {
				DEBUGMSGTL(("snmpagent", "wrong length %x", var_val_len));
				return SNMP_ERR_WRONGLENGTH;
			}
			g_nSnmpPort = *(int*)var_val;
			xml.UpdateSystemSetting("SnmpServerHostSetting", "");
		}
		
		break;

	case RESERVE2:
		break;

	case FREE:
		break;

	case ACTION:
		break;

	case UNDO:
		break;

	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

SnmpAgent::SnmpAgent()
{
	m_bCameraMoved = false;
}	

//初始化
bool SnmpAgent::Init()
{
	pthread_t id;
	int ret = pthread_create(&id, NULL, OpenService, NULL);
	if(ret != 0)
	{
		//失败
		LogError("开启snmp agent线程失败, 服务无法启动!\r\n");
	    g_bEndThread = true;
		return false;
	}
	return true;
}

//初始化私有Mib OID节点
void SnmpAgent::InitBocomMibOid(void)
{
    oid root[] ={OID_ROOT};
	oid cpu[] = {OID_ROOT, CPU_FATHER};
	oid mem[] = {OID_ROOT, MEM_FATHER};
	oid disk[] = {OID_ROOT, DISK_FATHER};
	oid netWork[] = {OID_ROOT, NETWORK_PATHER};   

	REGISTER_MIB("Public", RootVar, variable2, root);
	REGISTER_MIB("Cpu", CpuVar, variable2, cpu);
	REGISTER_MIB("Mem", MemVar, variable2, mem);
	REGISTER_MIB("Disk", DiskVar, variable2, disk);
	
	REGISTER_MIB("NetWork",NetWorkVar,variable2, netWork);
}

//GET的句柄函数
u_char* SnmpAgent::SnmpHandle(struct variable *vp, oid * name, size_t * length, 
					int exact, size_t * var_len, WriteMethod ** write_method)
{
    DEBUGMSGTL(("bocom", "snmp_handle entered\n"));
    if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
        return NULL;

	string tmpStr;
	static char buf[1024];
	static int iResult = 0;
	static UINT64 u64Integer = 0;
	int used, total;
    switch (vp->magic) 
	{
	case NE_TYPE:
		iResult = 37;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case SERIAL_NUMBER:
		memset(buf, 0, sizeof(buf));
		memcpy(buf, g_strDetectorID.c_str(), g_strDetectorID.size());
		*var_len = g_strDetectorID.size();
		return (u_char *)buf;

	case MIB_VERSION:
		memset(buf, 0, sizeof(buf));
		memcpy(buf, "MIB 1.0", strlen("MIB 1.0"));
		*var_len = strlen("MIB 1.0");
		return (u_char *)buf;

	case HARDWARE_VERSION:
		memset(buf, 0, sizeof(buf));
		memcpy(buf, g_strVersion.c_str(), g_strVersion.size());
		*var_len = g_strVersion.size();
		return (u_char *)buf;

	case SOFTWARE_VERSION:
		memset(buf, 0, sizeof(buf));
		memcpy(buf, g_strVersion.c_str(), g_strVersion.size());
		*var_len = g_strVersion.size();
		return (u_char *)buf;

	case LABEL:
		memset(buf, 0, sizeof(buf));
		memcpy(buf, g_strVersion.c_str(), g_strVersion.size());
		*var_len = g_strVersion.size();
		return (u_char *)buf;

	case NTP_USED:
		if ("" == g_strSynClockHost || "0.0.0.0" == g_strSynClockHost)
			iResult = 0;
		else
			iResult = 1;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case NTP_Server:
		memset(buf, 0, sizeof(buf));
		memcpy(buf, g_strSynClockHost.c_str(), g_strSynClockHost.size());
		*var_len = g_strSynClockHost.size();
		return (u_char *)buf;

	case CONTACT:
		tmpStr = "博康集团 TEL:33637763 地址:上海市徐汇区虹漕路456号光启大厦20层";
		g_skpDB.UTF8ToGBK(tmpStr);
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case LOCATION:
		if ("" == m_strSnmpLocation)
		{
			tmpStr = g_skpDB.GetChannelList();
			SRIP_CHANNEL* sChannel = (SRIP_CHANNEL*)tmpStr.c_str();
			m_strSnmpLocation = sChannel->chPlace;
			g_skpDB.UTF8ToGBK(m_strSnmpLocation);
		}

		memset(buf, 0, sizeof(buf));
		memcpy(buf, m_strSnmpLocation.c_str(), m_strSnmpLocation.size());
		*var_len = m_strSnmpLocation.size();
		return (u_char *)buf;

	case PRODUCT_CODE:
		tmpStr = "MVS1000 " + g_strVersion;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case DESCRIPTION:
		tmpStr = "智能检测器MVS1000";
		g_skpDB.UTF8ToGBK(tmpStr);
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case TRAP_IP:
		tmpStr = g_strSnmpHost;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		*write_method = SnmpSetHaldle;
		return (u_char *)buf;

	case TRAP_PORT:
		iResult = g_nSnmpPort;
		*var_len = sizeof(int);
		*write_method = SnmpSetHaldle;
		return (u_char *) &iResult;

	case SYSTEM_BITS:
		iResult = 64;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case SYSTEM_FILE_TYPE:
		tmpStr = g_strDiskFileSys;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	//为了管理端能顺序取得OID,返回空值
	case CPU_RUNTIME:
	case CPU_USER:
	case CPU_SYSTEM:
	case CPU_IDLE:
	case CPU_IOWAIT:
	case CPU_FUNSPEED:
		iResult = 0;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

    case CPU_USED:
		iResult = g_sysInfo.fCpu;
		*var_len = sizeof(int);
        return (u_char *) &iResult;

	case CPU_TEMPERATURE:
		iResult = g_sysInfo.fCpuT;
		*var_len = sizeof(int);
        return (u_char *) &iResult;

	case CPU_NAME:
		tmpStr = g_sysInfo_ex.szCpu;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case MEM_TOTAL:
		iResult = g_sysInfo_ex.fTotalMemory * 1000000;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case MEM_USED:
		iResult = g_sysInfo_ex.fTotalMemory * 1000000 * (g_sysInfo.fMemory/100);
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case MEM_SWAP_TOTAL:
		GetSwapMem(&used, &total);
		iResult = total;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case MEM_SWAP_USED:
		GetSwapMem(&used, &total);
		iResult = used;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case DISK_ID:
		tmpStr = g_strDiskId;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case DISK_NAME:
		tmpStr = g_strDiskName;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case DISK_READRATE:
		if (0 == m_nReadSpeed)
		{
			GetDiskRwSpeed(&m_nReadSpeed, &m_nWriteSpeed);
		}
		iResult = m_nReadSpeed;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case DISK_WRITERATE:
		if (0 == m_nWriteSpeed)
		{
			GetDiskRwSpeed(&m_nReadSpeed, &m_nWriteSpeed);
		}
		iResult = m_nWriteSpeed;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case DISK_MOUNTED:
		iResult = 1;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case DISK_CANWRITE:
		iResult = 2;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case DISK_TEMPERATURE:
		iResult = 0;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case DISK_MOUNTPATH:
		tmpStr = g_strDiskMountPath;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case DISK_TOTAL_SIZE:
		iResult = g_sysInfo_ex.fTotalDisk * 1000;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case DISK_USED_SIZE:
		iResult = g_sysInfo_ex.fTotalDisk *10 * g_sysInfo.fDisk;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case NET_ADDR:
		tmpStr = g_CameraHost + "; " + g_ServerHost;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case NET_MASK:
		tmpStr = g_strCameraNetMask + "; " +g_strNetMask;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case MAC:
		tmpStr = GetMacAddr();
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case NET_GATEWAY:
		tmpStr = g_strGateWay;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case OS_RELESE:
		tmpStr = g_OsRelese;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case KERNEL_RELESE:
		tmpStr = g_KernelRelese;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case HOST_NAME:
		tmpStr = g_HostName;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case SYSTEM_UPTIME:
		iResult = GetSystemUptime();
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case CURRENT_TIME:
		iResult = time(NULL);
		*var_len = sizeof(int);
		return (u_char *) &iResult;
		
	case SYS_TEMPERATURE:
		iResult = g_sysInfo.fSysT;
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case SYN_TIME:
		struct timeval now;
		gettimeofday(&now, NULL);
		UINT64 time = (UINT64)now.tv_sec * 1000 + now.tv_usec/1000;

		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%u", time);

		*var_len = strlen(buf);
		*write_method = SnmpSetHaldle;
		return (u_char *)buf;
	case NET_IP_ADDR:
		tmpStr = g_ServerHost;
		memset(buf, 0, sizeof(buf));
		memcpy(buf, tmpStr.c_str(), tmpStr.size());
		*var_len = tmpStr.size();
		return (u_char *)buf;

	case NET_LINKED:
		iResult = check_nic("eth1");
		*var_len = sizeof(int);
		return (u_char *) &iResult;

	case NET_RX:
		printf("1314-----1314 ---------rx\n");
		int rx1,tx1;
		GetRxTX(rx1, tx1);
		*var_len = sizeof(int);
		return (u_char *) &rx1;

	case NET_TX:
		printf("1314-----1314 ---------tx\n");
		int rx2,tx2;
		GetRxTX(rx2, tx2);;
		*var_len = sizeof(int);
		return (u_char *) &tx2;

    default:
        printf("unknown type\n");
    }

    return NULL;
}

//轮询系统状态,发送Trap报警
void SnmpAgent::CheckSystemStatus()
{
	if(g_sysInfo.fCpu > 95)
		SnmpSendTrap(TRAP_CPU_USED, "CPU负载过高");
	if(g_sysInfo.fCpuT > 75)
		SnmpSendTrap(TRAP_CPU_TEMPERATURE, "CPU温度过高");
	if(g_sysInfo.fSysT > 75)
		SnmpSendTrap(TRAP_SYS_TEMPERATURE, "检测器系统温度过高");
	if(g_sysInfo.fMemory > 85)
		SnmpSendTrap(TRAP_MEM_USED, "内存使用过高");
	if(g_sysInfo.fDisk > 90)
		SnmpSendTrap(TRAP_DISK_FREE_SIZE, "硬盘剩余空间不足");

	//相机状态
	if(g_skpDB.GetCameraState(g_nCameraID) != 0)
	{
		SnmpSendTrap(TRAP_NO_VIDEO, "无视频输入");
	}

	//开关门状态
	if(g_nHasExpoMonitor)
	{
		//开关门
		//if(g_ExpoMonitorInfo.nGateStateAlarm > 0)
		{
			if(g_ExpoMonitorInfo.nGateValue == 1)  //01：机箱开门
			{	
				SnmpSendTrap(TRAP_GATE_OPEN, "机箱门打开");
			}
		}
	}
		
	if (m_bCameraMoved == true)
		SnmpSendTrap(TRAP_CAMERA_MOVED, "相机位置移动");
}

//发送Trap报警
bool SnmpAgent::SnmpSendTrap(string typeId, string desc)
{
	if ("" == g_strSnmpHost || 0 == g_nSnmpPort)
	{
		LogError("SNMP 发送参数错误!\n");
		return false;
	}
	
    struct snmp_session session, *sess;
    struct snmp_pdu *pdu;
 
    init_snmp("snmp");
    snmp_sess_init(&session);
    session.version = SNMP_VERSION_2c;
	char buf[30];
	sprintf(buf, "%s:%d", g_strSnmpHost.c_str(), g_nSnmpPort);
	session.peername = buf;
	const char* community = "public";
    session.community = (u_char*)community;
    session.community_len = strlen(community);
    session.callback = NULL;
    session.callback_magic = NULL;

    SOCK_STARTUP;
    sess = snmp_open(&session);
    if(sess == NULL) {
        snmp_sess_perror("snmptrap",&session);
        SOCK_CLEANUP;
        return false;
    }

	//建立PDU
	pdu = snmp_pdu_create(SNMP_MSG_TRAP2);

	//加入报警标志
	oid OID_Alarm[] = {OID_ALARM_FLAG};
	int ret = snmp_add_var(pdu, OID_Alarm, sizeof(OID_Alarm) / sizeof(oid), 'i', "1");//i表示int型
	if (ret != 0)
	{
		snmp_perror("1");
	}

	//加入报警编号
	oid OID_AlarmTypeID[] = {OID_ALARM_INDEX};
	ret = snmp_add_var(pdu, OID_AlarmTypeID, sizeof(OID_AlarmTypeID) / sizeof(oid), 'i', "1");
	if (ret != 0)
	{
		snmp_perror("1");
	}

	//加入报警类型
	oid OID_AlarmType[] = {OID_ALARM_TYPE};
	ret = snmp_add_var(pdu, OID_AlarmType, sizeof(OID_AlarmType) / sizeof(oid), 'i', "1");
	if (ret != 0) 
	{
		snmp_perror("1");
	}

	//加入报警级别
	oid OID_AlarmDesc[] = {OID_ALARM_SEVERITY};
	ret = snmp_add_var(pdu, OID_AlarmDesc, sizeof(OID_AlarmDesc) / sizeof(oid), 'i', "1");
	if (ret != 0) 
	{
		snmp_perror(desc.c_str());
	}

	//加入报警类型
	oid OID_AlarmLevel[] = {OID_ALARM_TYPEID};
	ret = snmp_add_var(pdu, OID_AlarmLevel, sizeof(OID_AlarmLevel) / sizeof(oid), 'i', typeId.c_str());
	if (ret != 0) 
	{
		snmp_perror(typeId.c_str());
	}

	//加入报警种类
	oid OID_AlarmTime[] = {OID_ALARM_CATEGORY};
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", time(NULL));
	ret = snmp_add_var(pdu, OID_AlarmTime, sizeof(OID_AlarmTime) / sizeof(oid), 'i', "2");
	if (ret != 0) 
	{
		snmp_perror("2");
	}

	////加入设备ID
	//oid OID_DeviceId[] = {OID_TRAP_DEVICEID};
	//ret = snmp_add_var(pdu, OID_DeviceId, sizeof(OID_DeviceId) / sizeof(oid), 's', "37");
	//if (ret != 0) 
	//{
	//	snmp_perror("37");
	//}

	////加入设备名称
	//oid OID_DeviceName[] = {OID_TRAP_DEVICENAME};
	//ret = snmp_add_var(pdu, OID_DeviceName, sizeof(OID_DeviceName) / sizeof(oid), 's', "智能检测器");
	//if (ret != 0) 
	//{
	//	snmp_perror("智能检测器");
	//}

    if(snmp_send(sess, pdu) == 0)
	{
        snmp_sess_perror("snmptrap:", sess);
        snmp_free_pdu(pdu);
    }
	else
	{
        printf("snmp_send success\n");
    }
    snmp_close(sess);
    SOCK_CLEANUP;
	return true;
}

#endif

