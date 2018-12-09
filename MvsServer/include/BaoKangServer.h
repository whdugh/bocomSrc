// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/**
 *   文件：BaoKangServer.h
 *   功能：乐清宝康电警通讯类
 *   作者：GaoXiang
 *   时间：2012-03-11
**/

#ifndef _BAOKANG_SERVER_H
#define _BAOKANG_SERVER_H

#include "CSocketBase.h"

//命令字
#define ASK_CONN		101	//联机信号
#define REALTIME_DATA	111	//记录信息
#define LANE_REAL_FLOW	150	//流量信息

#define ACK_CONN		201	//应答联机信号
#define ACK_RECVDATA	220	//应答收到记录指令
#define ACK_RECVFLOW	221	//应答收到流量记录指令

//常量定义
#define DEVID_LENGTH 12
#define END_LEN		2
#define BAOKANG_PORT 6600
#define BAOKANG_PACKAGE_SIZE  6000
#define BAOKANG_SLEEP	20000

//违章类型
#define BAOKANG_OVERSPEED	1	//超速
#define BAOKANG_RED_LIGHT	4	//闯红灯
#define BAOKANG_LINE		11	//违反标志标线

//车身颜色
#define BAOKANG_UNKNOWN	0	//未知
#define BAOKANG_BLACK	1	//黑色
#define BAOKANG_WHITE	2	//白色
#define BAOKANG_RED		3	//红色
//#define BAOKANG_ORANGE	4	//橙色
#define BAOKANG_YELLOW	5	//黄色
#define BAOKANG_GREEN	6	//绿色
//#define BAOKANG_		7	//青色
#define BAOKANG_BLUE	8	//蓝色
#define BAOKANG_PURPLE	9	//紫色
#define BAOKANG_SILVER	10	//银色
#define BAOKANG_BROWN	11	//褐色
#define BAOKANG_GRAY	12	//灰色
//#define BAOKANG_		13	//银白

//车标
#define	BAOKANG_OTHERS	0	//未知
#define	BAOKANG_VW		1	//大众
#define	BAOKANG_AUDI	2	//奥迪
#define	BAOKANG_BENZ	3	//奔驰
#define	BAOKANG_HONDA	4	//本田
#define	BAOKANG_TOYOTA	5	//丰田
#define	BAOKANG_HYUNDAI 6	//现代
#define	BAOKANG_BUICK	7	//别克
#define	BAOKANG_REDFLAGS 8	//红旗
#define	BAOKANG_JINBEI	9	//金杯
#define	BAOKANG_XIALI	10	//夏利
#define	BAOKANG_FOTON	11	//福田
//#define	BAOKANG_BLAC	12	
#define	BAOKANG_CITERON 13	//雪铁龙
#define	BAOKANG_MAZDA	14	//马自达
#define	BAOKANG_IVECO	15	//依维柯
//#define	BAOKANG_ 16	松花江

enum BAOKANG_LINK_STATE
{
	BAOKANG_CONNECTED = 1, //连接上
	BAOKANG_RULES_PIC, //发送违章图片
	BAOKANG_ALL_PIC, //发送所有图片
};

//组包拆包字节
const char END = 0xFC;
const char ESC = 0xFD;
const char ESC_END = 0xFE;
const char ESC_ESC = 0xFF;

typedef struct _BAOKANG_HEADER
{
	BYTE bSyn;			//同步字END
	BYTE bVersion;		//主版本号
	BYTE bSubVersion;	//子版本号
	UINT32 uLength;		//包长度
	UINT32 uCompanyId;	//厂商ID
	UINT32 uCmd;		//命令字

	_BAOKANG_HEADER()
	{
		bSyn = (BYTE)END;
		bVersion = (char)0;
		bSubVersion = (char)0;
		uLength = 0;
		uCompanyId = 1;
		uCmd = 0;
	}
}BAOKANG_HEADER;

//抓拍记录信息
typedef struct _VHI_REC		
{
	char	chDevId[12];//设备编号，公安部标准要求10位，后面两位补’\0’
	int		uIndex;		//自增长记录id号，单台设备不能重复，系统应答时要使用
	char	chText[16];	//号牌号码
	UINT32	uTimeS;		//违法时间
	int		uTimeMs;	//违法毫秒
	float	speed;		//车速
	float	carLength;	//车长
	int		uPlateCor;	//号牌颜色
	int		uSecIndex;	//记录索引号，用于区别同一秒的多条记录，0-9
	int		uViolationType;//违法类型
	int		uRedLightTime; //红灯时间，默认为0。
	int		uRoadWayId;		//车道编号，1-99
	int		uMaxSpeed;		//限速，默认为0
	int		uCarCor;		//车身颜色，高两位用来表示车身颜色深浅。0表示未知，1表示浅，2表示深；低两位表示车身颜色，见车身颜色代码表。
	int		uRoadWayType;	//记录类别，0：非机动车道，1：机动车道；
	int		uCarType;		//车辆类型，1：小型车，2：中型车，3：大型车；4：其他
	int		uDirection;		//行驶方向，1：自东向西，2：自西向东，3：自南向北，4：自北向南
	int		uCarBrand;		//厂商标志，见厂商标志代码表。
	int		uPersonCor;		//行人衣着颜色，见衣着颜色代码表。
	int		uPicType;		//图片格式类型，见图片格式代码表
	int		res1;			//保留
	int		res2;			//保留

	_VHI_REC()
	{
		memset(chDevId, 0, 12);
		memset(chText, 0, 16);
		uIndex = 0;
		uTimeS = 0;
		uTimeMs = 0;
		speed = 0;
		carLength = 0;
		uPlateCor = 4;
		uSecIndex = 0;
		uViolationType = 0;
		uRedLightTime = 0;
		uRoadWayId = 0;
		uMaxSpeed = 0;
		uCarCor = 0;
		uRoadWayType = 0;
		uCarType = 0;
		uDirection = 0;
		uCarBrand = 0;
		uPersonCor = 0;
		uPicType = 0;
		res1 = 0;
		res2 = 0;
	}
}VHI_REC;

//宝康车牌记录
typedef struct _BAOKANG_PLATE
{
	char	chDevId[12];	//下位机设备编号
	VHI_REC Rec;			//记录信息
	int		uPicNum;		//本记录的图片张数
	int		uCurPicIndex;	//当前为第几张图片
	int		uPackageNum;	//当前图片一共几个数据包
	int		uCurPackageIndex;	//当前为第几个数据包
	int		uCurPicSize;	//当前图片大小

	_BAOKANG_PLATE()
	{
		memset(chDevId, 0, 12);
		uPicNum = 0;
		uCurPicIndex = 0;
		uPackageNum = 0;
		uCurPackageIndex = 0;
		uCurPicSize = 0;
	}
}BAOKANG_PLATE;

//宝康流量统计记录
typedef struct _BAOKANG_STATISTIC
{
	char chDevId[12];		//下位机设备编号
	int	 uIndex;  		//车道流量ID号，自增长的编号，系统应答时使用
	int  uRoadWay;		//车道号
	UINT32 uTime;			//统计时间
	int    uTimeLength; 	//统计间隔（秒），统计间隔应当小于300秒
	float  uOccupancy;		//占有率，小于1的浮点数
	int uSmallCarNum;	//小型车数
	int uMiddleCarNum;	//中型车数
	int uBigCarNum;		//大型车数
	int uOtherCarNum;	//其他类型车辆数
	int uAverageSpeed;	//平均速度
	int uPlateType[33];	//按照{沪,川,鄂,甘,赣,贵,桂,黑,吉,冀,京,津,晋,辽,鲁,蒙,宁,青,琼,陕,苏,湘,新,渝,粤,豫,云,藏,浙,皖,闽，其他，未知}的顺序，给出的流量
	char chReserved[8];	//保留

	_BAOKANG_STATISTIC()
	{
		memset(chDevId, 0, 12);
		uIndex = 0;
		uRoadWay = 0;
		uTime = 0;
		uTimeLength = 0;
		uOccupancy = 0;
		uSmallCarNum = 0;
		uMiddleCarNum = 0;
		uBigCarNum = 0;	
		uOtherCarNum = 0;
		uAverageSpeed = 0;
		memset(uPlateType, 0, 33*4);
		memset(chReserved, 0, 8);
	}
}BAOKANG_STATISTIC;

//通道数据列表
typedef std::list<std::string> BaoKangResultMsg;

class BaoKangServer:public mvCSocketBase
{
    public:
        //构造
        BaoKangServer();
        //析构
        virtual ~BaoKangServer();

        //启动侦听服务
        bool Init();
        //释放
        bool UnInit();
		//按照协议数据打包
		bool InPackage(string &msg);
		//按照协议数据解包
		bool UnPackage(string &msg);

		//连接中心端
		bool ConnectServer();
		//连接中心端并发心跳
		bool ConnectLinkTest();
		//删除客户端
		bool DelClient();
		//接收消息
		void RecvMsg();
		//处理收到的命令
		bool OnMsg(const int nSocket, const string& request);
		//处理记录结果
		void DealResult();
		//添加一条数据
		bool AddResult(std::string& strMsg);
		//转换表
		int Transfer(int type, int value);
		//将博康车牌结构填入宝康车牌结构体
		bool PlateCopy(BAOKANG_PLATE& bPlate, RECORD_PLATE* sPlate);
		//处理检测结果
		bool OnResult(std::string& result);
		//发送车牌数据
		bool SendPlate(RECORD_PLATE *sPlate);
		//发送统计数据
		bool SendStatistic(RECORD_STATISTIC *sStatistic);
		//发送记录到中心
		bool SendMsg(string &strMsg);
		//处理历史数据
		void DealHistoryRecord();
		//发送心跳
		bool SendLinkTest();

		//从配置文件中读入的限速数值Map; <key:车道号 value:最大速度>
		//map<UINT32, UINT32> m_maxSpeedMap;
		//mapMaxSpeedStr m_maxSpeedMapStr;
     private:

        //线程ID
        pthread_t m_nThreadId;
        pthread_t m_nHistoryThreadId;

        //检测结果消息列表
        BaoKangResultMsg  m_ChannelResultList;

		//宝康中心端IP
		string m_strServerHost;

		//重连中心端的超时时间
		UINT32 m_uTimeout;
		//自动增加的同一秒的序列号
		int m_uSecIndex;

		//统计记录发送成功Map  <key:流量记录表中的记录ID, value:16个车道对应的发送状态>
		map<int, short> m_mapSendState;
		
		//检测结果信号互斥
		pthread_mutex_t m_SendState_Mutex;
		pthread_mutex_t m_Result_Mutex;
		pthread_mutex_t m_thread_Mutex;

		//连接状态 0:未连接 1:连接上未确认 2:确认不发送卡口图片 3:确认发送所有图片 
		int m_nLinkState;
		//连接socket
		int m_nSocket;
};

extern BaoKangServer g_BaoKangServer;

#endif
