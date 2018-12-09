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
#ifndef SNMP_BOCOM_AGENT_H
#define SNMP_BOCOM_AGENT_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

using namespace std;

//本地OID注册的父节点
#define OID_ROOT	1,3,6,1,4,1,31492
#define CPU_FATHER	3,1,1,1,7
#define MEM_FATHER	3,1,1,1,8
#define DISK_FATHER	3,1,1,1,9
#define NETWORK_PATHER 3,1,1,1,10 //       




//OID的本地定义,与管理端无关
#define CPU_USED			0x00000001
#define CPU_TEMPERATURE		0x00000002
#define CPU_NAME			0x00000003
#define MEM_TOTAL			0x00000004
#define MEM_USED			0x00000005
#define MEM_USED_PERCENT	0x00000006 //尚未定义MIB
#define MEM_SWAP_TOTAL		0x00000007
#define MEM_SWAP_USED		0x00000008

#define NET_ADDR		0x00000010
#define NET_MASK		0x00000011
#define NET_GATEWAY		0x00000012
#define NETWORK_NAME	0x00000013
#define CPU_RUNTIME		0x00000014
#define CPU_USER		0x00000015
#define CPU_SYSTEM		0x00000016
#define CPU_IDLE		0x00000017
#define CPU_IOWAIT		0x00000018
#define CPU_FUNSPEED	0x00000019

#define OS_RELESE		0x00000020
#define KERNEL_RELESE	0x00000021
#define HOST_NAME		0x00000022
#define SYSTEM_UPTIME	0x00000023
#define CURRENT_TIME	0x00000024
#define SYS_TEMPERATURE	0x00000025

#define NE_TYPE		0x00000026				//网元类型
#define SERIAL_NUMBER		0x00000027		//网元序列号
#define MIB_VERSION			0x00000028	//网元mib版本号
#define HARDWARE_VERSION	0x00000029	//网元设备硬件版本号
#define SOFTWARE_VERSION	0x00000030	//网元业务版本号
#define LABEL			0x00000031		//网元的标签
#define LOCATION		0x00000032		//网元的位置
#define MAC				0x00000033		//网元MAC地址
#define NTP_USED		0x00000034		//时钟服务状态
#define NTP_Server		0x00000035		//时钟服务器
#define CONTACT			0x00000036		//联系信息
#define PRODUCT_CODE	0x00000037		//产品编号
#define DESCRIPTION		0x00000038		//网元描述
#define TRAP_IP			0x00000039		//trap接收地址
#define TRAP_PORT		0x00000040		//trap接收端口
#define SYSTEM_BITS		0x00000041		//操作系统位数
#define SYSTEM_FILE_TYPE	0x00000042	//文件系统类型
#define SYN_TIME	0x00000043		//SET操作用的同步时间

//硬盘相关OID
#define DISK_ID				0x00000050
#define DISK_MOUNTED		0x00000051
#define DISK_CANWRITE		0x00000052
#define DISK_MOUNTPATH		0x00000053
#define DISK_NAME			0x00000054
#define DISK_TOTAL_SIZE		0x00000055
#define DISK_USED_SIZE		0x00000056
#define DISK_READRATE		0x00000057
#define DISK_WRITERATE		0x00000058
#define DISK_TEMPERATURE	0x00000059 


//网络相关的OID

#define NET_IP_ADDR         0x00000060    
#define NET_LINKED          0x00000061    
#define NET_RX              0x00000062    
#define NET_TX              0x00000063    


//Trap 报警OID
#define OID_ALARM_FLAG	1,3,6,1,4,1,31492,3,1,1,3,1
#define OID_ALARM_INDEX	1,3,6,1,4,1,31492,3,1,1,3,2
#define OID_ALARM_TYPE	1,3,6,1,4,1,31492,3,1,1,3,3
#define OID_ALARM_SEVERITY	1,3,6,1,4,1,31492,3,1,1,3,4
#define OID_ALARM_TYPEID	1,3,6,1,4,1,31492,3,1,1,3,5
#define OID_ALARM_CATEGORY	1,3,6,1,4,1,31492,3,1,1,3,6

//报警类型编号
#define TRAP_SYS_TEMPERATURE	"801"
#define TRAP_CPU_USED			"890"
#define TRAP_CPU_TEMPERATURE	"891"
#define TRAP_DISK_FREE_SIZE		"892"
#define TRAP_MEM_USED			"894"
#define TRAP_CAMERA_MOVED		"852"
#define TRAP_GATE_OPEN			"808"
#define TRAP_NO_VIDEO			"806"

class SnmpAgent
{
public:
	SnmpAgent();
	//初始化
	bool Init();

	//初始化私有Mib
	void InitBocomMibOid(void);
	//Trap发送
	bool SnmpSendTrap(std::string typeId, std::string desc);

	//Get句柄函数
	static u_char* SnmpHandle(struct variable *vp, oid * name, size_t * length, 
                              int exact, size_t * var_len, WriteMethod ** write_method);

	//轮询发trap报警
	void CheckSystemStatus();

	//相机移动状态
	bool m_bCameraMoved;

	//硬盘读写速率
	static int m_nReadSpeed;
	static int m_nWriteSpeed;
};

extern SnmpAgent g_SnmpAgent;

#endif
#endif
