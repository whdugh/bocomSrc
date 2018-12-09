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

#ifndef SKP_ROAD_COMMON_H
#define SKP_ROAD_COMMON_H

#include "structdef.h"
typedef std::string String;


#include     <stdio.h>
#include     <stdlib.h> 
#include     <unistd.h>  
#include     <sys/types.h>
#include     <sys/stat.h>
#include	 <sys/mman.h>
#include     <fcntl.h> 
#include     <termios.h>
#include     <errno.h>
#include     <string.h>
#include     <string>
using namespace std;

//#define _LOGDEBUG
//#define LogTime
//调试线程
//#define _THREAD_DEBUG
//#define PRESSURE_TEST
//#define DEBUG_INFO
//#define g_nGongJiaoMode 0

//线程运行标识位
extern volatile bool g_bEndThread;
//usb挂载路径
extern std::string g_usbMountPath;
//系统状态
extern SRIP_SYSTEM_INFO g_sysInfo;
extern time_t g_sysTime; //系统时间  add by  taojun   2013/4/11
//系统信息扩展
extern SYSTEM_INFO_EX g_sysInfo_ex;
//监控主机信息
extern MONITOR_HOST_INFO g_monitorHostInfo;
//智能控制器主机信息
extern EXPO_MONITOR_INFO g_ExpoMonitorInfo;
//Dsp相机服务器主机信息
extern DSP_SERVER_HOST_INFO g_DspServerHostInfo;
//图片格式信息
extern PIC_FORMAT_INFO g_PicFormatInfo;
//录像格式信息
extern VIDEO_FORMAT_INFO g_VideoFormatInfo;
//比对服务器信息
extern MATCH_HOST_INFO g_MatchHostInfo;

//区间测速主机信息
extern DISTANCE_HOST_INFO g_DistanceHostInfo;
//GPS设置
extern GPS_SET_INFO g_GpsSetInfo;
//信号机设置
extern SIGNAL_SET_INFO g_SignalSetInfo;
//应用管理服务器信息
extern AMS_HOST_INFO g_AmsHostInfo;
//系统配置系统
//程序版本
extern std::string        g_strVersion;
//检测器端对应DSP版本
extern char  g_chVersionDsp[4];

//ip地址(百兆网卡)
extern std::string        g_ServerHost;
//子网掩码
extern std::string        g_strNetMask;
//默认网关
extern std::string        g_strGateWay;
//eth0子网掩码
extern std::string  g_strCameraNetMask;
//eth1 mtu
extern int g_uNetMTU;
//eth0 mtu
extern int g_uCameraNetMTU;
//认证服务器地址
extern std::string        g_strAuthenticationHost;
//ip地址(千兆网卡)
extern std::string        g_CameraHost;
//时钟同步服务器
extern std::string        g_strSynClockHost;

//服务端口
extern int		   g_nPPort;
extern int		   g_nEPort;

extern int		   g_nPPort2;
extern int		   g_nPPort3;

//DB地址
extern std::string g_strDbHost;
//DB端口
extern int		   g_nDbPort;
//登录名
extern std::string g_strUser;
//密码
extern std::string g_strPw;
//数据库
extern std::string g_strDatabase;
//录像路径
extern std::string g_strVideo;
//录像缓存路径
extern std::string g_strVideoTemp;
//车牌图片路径
extern std::string g_strPic;
//备份路径
extern std::string g_strBack;
//Ftp Home默认路径
extern std::string g_strFtpHome;
//备份数据库执行文件
extern std::string g_strBackExeFile;
//车牌首字符
extern std::string g_strCarNum;
//车标
extern char g_strCarLabel[][20];

extern unsigned char rgb_Kfactor9_25[(255*33+1)*17];
// 夜晚检测开始时间
extern unsigned int g_nNight;
// 白天检测开始时间
extern unsigned int g_nDay;
// 傍晚检测开始时间
extern unsigned int g_nDusk;

//磁盘清理时间间隔
extern unsigned int g_uDiskDay;

//是否存储相机日志
extern int g_nDspLog;

//存图数量
extern int g_nSaveImageCount;

//上次程序停止时间
extern unsigned int g_uLastTime;
//启动时间
extern unsigned int g_uTime;
//是否需要相机控制
extern int g_nCameraControl;
//相机编号
extern int g_nCameraID;
//串口设置列表
extern COM_PARAMETER_MAP g_mapComSetting;
//相机串口设置
extern COM_PARAMETER g_CameraComSetting;
//交通灯串口设置
extern COM_PARAMETER g_RedLightComSetting;
//车检器串口设置
extern COM_PARAMETER g_DHComSetting;
//补光灯串口设置
extern COM_PARAMETER g_LightComSetting;
//VIS球机控制串口设置
extern COM_PARAMETER g_VisComSetting;
//GPS校时串口设置
extern COM_PARAMETER g_GpsComSetting;
//雷达串口设置
extern COM_PARAMETER g_RadarComSetting;
//爆闪灯串口设置
extern COM_PARAMETER g_FlashComSetting;
//开关门串口设置
extern COM_PARAMETER g_DoorComSetting;
//DIO串口设置
extern COM_PARAMETER g_DioComSetting;  // dio 
//液晶显示
extern COM_PARAMETER g_ScreenComSetting; 
//球机云台控制参数
extern YUNTAI_CONTROL_PARAMETER g_ytControlSetting;
//开关灯信息
extern LIGHT_TIME_INFO g_LightTimeInfo;
//是否进行同步（从配置文件读）
extern int g_nDoSynProcess;
//是否有同步标记点
extern int g_nHasSynTrack;
//是否有中心端服务器
extern int g_nHasCenterServer;

//中心端类型(0:博康中心端；1：应用服务器；2：电科中心端)
extern int g_nServerType;

//访问控制服务器地址
extern std::string g_strControlServerHost;
//访问控制服务器端口
extern int g_nControlServerPort;
//中心端地址
extern std::string g_strCenterServerHost;
//中心端端口
extern int g_nCenterServerPort;
//ftp服务器地址
extern std::string g_strFtpServerHost;
//ftp用户名
extern std::string g_strFtpUserName;
//ftp密码
extern std::string g_strFtpPassWord;
//ftp端口
extern int g_nFtpPort;
//检测器编号
extern std::string g_strDetectorID;
//VIS服务器地址
extern std::string g_strVisHost;
//VIS端口
extern int g_nVisPort;
//图片ID
extern unsigned int g_uPicId;
//录象ID
extern unsigned int g_uVideoId;
//最大图片编号
extern unsigned int g_uMaxPicId;
//最大录像编号
extern unsigned int g_uMaxVideoId;
//车道类型
extern int g_nRoadType; //1:机动车道，2：非机动车道，3：机非混合车道
//是否进行以下两种事件检测
extern int g_nMoreEvent; //0:不检测，1：检测
//是否有中心网管
extern int g_nHasNeServer;
//是否只发送违章数据
extern int g_nSendViolationOnly;
//是否抖动检测
extern int g_nCheckShake;
//是否布控报警
extern int g_nDetectSpecialCarNum;
//回写等待时间
extern int g_nWriteBackTime;
//编码格式
extern int g_nEncodeFormat;
//录像帧率
extern float g_fFrameRate;
//录像水平分辨率
extern int g_nVideoWidth;
//录像垂直分辨率
extern int g_nVideoHeight;
//键盘编号
extern int g_nKeyBoardID;
//多张图片组合方式:0:叠加存放,1:分开存放
extern int g_nPicMode;
//扩展avi头
extern int g_nAviHeaderEx;
//信号互斥
extern pthread_mutex_t g_Id_Mutex;
//是否强制红灯
extern int g_nForceRedLlight;
//控制灯状态
extern UINT16 g_uTrafficSignal;

//校时方式
extern int g_nClockMode;
//车道->方向描述映射
extern ROAD_DIRECTION_MAP g_roadDirectionMap;
//是否发送RTSP实时视频
extern int g_nSendRTSP;
//球机控制方式
extern int g_nControlMode;

//是否存在多个预置位
extern int g_nMultiPreSet;
//是否需要切换相机
extern int g_nSwitchCamera;
//检测器工作方式(0:连续方式；1：触发方式;3:混合方式)
extern int g_nWorkMode;
//是否登录vis或I2C系统
extern int g_nLoginState;
//闯红灯图片组合方式(0:3x1,1:2x2)
extern int g_nVtsPicMode;
//是否有智能控制器
extern int g_nHasExpoMonitor;
//是否有爆闪灯
extern int g_nHasHighLight;
//系统设置模板编号
extern int g_nSettingModelID;
//图片占用磁盘空间
extern int g_nPicDisk;
//录像占用磁盘空间
extern int g_nVideoDisk;
//硬盘容量极大值
extern int g_nMaxDisk;
//历史视频播放方式
extern int g_nHistoryPlayMode;
//是否视频爆闪控制
extern int g_nFlashControl;
//旅行时间通讯方式
extern int g_nCommunicationMode;
 //是否发送历史记录
extern int g_nSendHistoryRecord;
//是否有ntp-server
extern int g_nNtpServer;
//图片文件数量
extern unsigned int g_nMaxPicFileCount;

//本地车牌号码
extern string g_strLocalPlate;
//是否需要载入违章车牌数据
extern int  g_nLoadBasePlateInfo;

//工作模式为连续的通道的通道号
extern int g_WhichContinuous;
//车牌库是否需要加载 0表示成功下载了，可以更新了。 1 表示成功更新了。 2 表示下载失败，不能更新
extern int g_bLoadXml;
//是否ftp服务端
extern int  g_nFtpServer;

//是否发送图片
extern int  g_nSendImage;
//是否按时间段检测
extern int  g_nDetectByTime;
//开始检测时间
extern int  g_nBeginDetectTime;
//结束检测时间
extern int  g_nEndDetectTime;
//是否图像增强
extern int g_nImageEnhance;
//本地黄标车数量
extern int g_nType1;
//本地柴油车数量
extern int g_nType2;
//文件编号(供测试使用)
extern int g_nFileID;
//检测器工作模式,0:连续模式(默认);1:触发模式;2:DSP模式
extern int g_nDetectMode;

extern string g_OsRelese;
extern string g_KernelRelese;
extern string g_HostName;
extern string g_strDiskId;//硬盘ID号
extern string g_strDiskName;//硬盘名
extern string g_strDiskFileSys;//硬盘文件系统
extern string g_strDiskMountPath;//硬盘挂载路径

//SNMP网管服务器报警Ip和报警端口
extern string g_strSnmpHost;
extern int g_nSnmpPort;

//是否打印模块时间信息
extern int g_nPrintfTime;

//预置位模式
extern int g_nPreSetMode;

//是否将图片存放在特定文件夹
extern int g_nPicSaveMode;
//是否短连接控制自产相机
extern bool g_bShortConnect;

////电科中心端限速
//extern int g_dkLimitSpeed;
//电科中心端补传时刻分钟设置
extern int g_dkMinute;
//电科中心端补传方式设置
extern int g_dkTransWay;
////电科中心端1.8路口号（有中心端分配）
//extern string g_dkCrossingCode;


//检测器之间通讯，此路径用于存储上一个点位发送过来的图片
extern string g_strMvsRecvPic;
//检测器之间通讯，此图片ID表示上一个点位发送过来的图片存储于本点位PIC_INFO_RECV表中的PIC_ID；
extern int g_strMvsRecvPicId;
//检测器之间通讯，图片Id互斥
extern  pthread_mutex_t g_MvsRecvPicId_Mutex;
//检测器之间通讯,上一个点距本点位之间的距离
extern float g_MvsDistance;
//检测器之间通讯，下一个点位的IP
extern string g_MvsNextHostIp;
//检测器之间通讯，下一个点位的侦听接收端口
extern int g_MvsNextRecPort;
//检测器之间通讯，本点位的侦听接收端口
extern int g_MvsRecPort;
//检测器之间通讯,是否需要区间测速
extern int g_bMvsDistanceCal;

//公交模式 1.公交模式 0.非公交模式
extern int g_nGongJiaoMode;
//是否存在3G网卡
extern int g_nExist3G;

extern ServerKafka g_Kafka;
//尾号限行
extern PlateLimit g_PlateLimit;
//路段信息
extern sRoadNameInfo g_RoadInfo;
// 3G拨号成功后分配的ip
extern string g_str3GIp;
//3G类型 0:内置，1：外置
extern int g_n3GTYPE;

// FTP配置结构
extern SET_FTP_PATH g_Ftp_Path;

extern FASTINGIUM_TIME g_CheckTime;

// Ntp配置结构
extern SET_NTP_TIME_INFO g_Ntp_Time;

//全局记录编号
extern UINT64 g_uImgKey;

//获取当前时间的小时和分钟的和
int GetHourAndMinute();

//取当前时间 格式: 2008-05-14 15:12:38
std::string GetTimeCurrent();

//根据时间戳还原时间
std::string GetTime(long lTime, int nType = 0);

//取当前时间 [时+分]
unsigned int GetHmTime();

//字符串时间转换成时间戳
unsigned long MakeTime(std::string& strTime);

//取时间戳
unsigned int GetTimeStamp();

//取系统信息所有缓冲区
extern char g_chSysBuffer[SRIP_MAX_BUFFER + 1];
//ftp存储路径
extern char g_ftpRemotePath[256];
//取CPU利用率
void GetSysInfo_Cpu();
//取内存利用率
void GetSysInfo_Mem();
//是否是有数据盘
bool IsDataDisk();
//取磁盘使用情况
void GetSysInfo_Disk();
//取cpu温度，风扇转速及系统温度
void GetSysInfo_Ctf();
//取系统信息
void GetSysInfo();
//获取CPU信息
void GetCpuInfo();
//获取CPU核数
int GetCpuCore();

void ShowPCInfoToSmallScreen();
void ShowPCInfoToJHCSmallScreen();
void ShowPCInfoToWQSmallScreen();
//错误日志
void LogError(const char* format,...);
//警告日志
void LogWarning(const char* format,...);
//普通日志
void LogNormal(const char* format,...);
//获得md5
std::string getStringMd5(const char* buff,int nsize);

void WriteMvsVersionToFile(); 
//取配置串
std::string GetSysConfig();
//设置配置信息
int			SetSysConfig(SYSTEM_CONFIG& sys_config);
//设置扩展配置信息
int	SetSysConfigEx(SYSTEM_CONFIG_EX& sys_config_ex);

//读配置文件
void ReadIni();
//写配置文件
void WriteCameraIni(CAMERA_CONFIG cfg);
//写Dsp相机临时配置文件
void WriteCameraIniDsp(CAMERA_CONFIG cfg);


//载入相机配置（不同的应用场景对应不同的相机配置）
void LoadCameraProfile(CAMERA_CONFIG& cfg);
//根据相机编号载入相机配置
void LoadCameraProfileDsp(CAMERA_CONFIG& cfg);


//utf8转unicode
void UTF8ToUnicode(wchar_t *pwchOut, std::string strText);

//CRC32校验
UINT32 Crc( const void *s, int len );
bool EncodeBuff( char* pBuff, int BuffSize );

//是否后台运行
 bool Detach( int chdirtoroot, int closestd );

//获取网卡ip地址
std::string GetIpAddress(char* eth,int nIndex);
//获取子网掩码
std::string GetNetMask(char* eth);

//判断白天还是晚上
int DayOrNight(int nMode = 0);

int mac_addr_sys( u_char *addr);

// zhangyaoyao: format string to lower case
void ToLowerCase(string& strSrc);

//根据路由信息获取网关地址
int get_gateway(char *gateway);
void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo,char *gateway);
int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId);

//寻找进程号
int GetProcessCountByName( const char* Command ,int nSize, std::string &pid);
//杀死testcarnum.out进程
void KillProcess(std::string strProcessName,int nMaxCount = 1);

//读取CPU温度(zhangyaoyao)
void GetCpuAndSysTemp(float &fCpuTemp, float &fSysTemp);

//int cvWaitKey(int delay);

//获取识别时间(40ms-100ms之间)
int GetDetectTime();

//获取车头位置
void GetCarPostion(RECORD_PLATE& plate);

//获取单张图,车身位置,抠图区域
bool GetCarPosRect(
	const RECORD_PLATE &plate,
	CvRect &rtDst);

//核查rtSmall是否在rtBig内部
bool CheckRectInRect(const CvRect &rtSmall, const CvRect &rtBig);


//获取Rect,向外扩,指定比例,抠图区域
bool GetCarPosRect(
	const int nHeightC1,
	const int nWidthExtC1,
	const int nHeightExtC2,
	const int nHeightExtC3,
	const CvRect &rtSrc,
	const CvRect &rtSrcImg,
	CvRect &rtDst);

//获取图片
std::string GetImageByPath(std::string strPicPath);
//获取视频
std::string GetVideoByPath(std::string strVideoPath);

//base64编码
void EncodeBase64(std::string& result,unsigned char* pSrc,int nSrcLen);
//根据事件类型获取目录名称
std::string GetFtpPathNameByEvent(UINT32 uViolationType);
//获取目标颜色
std::string GetObjectColor(UINT32 uObjectColor);
//获取号牌颜色
std::string GetPlateColor(UINT32 uPlateColor);
//获取车道方向
std::string GetDirection(UINT32 uDirection);
//获取车道方向2
std::string GetDirection2(UINT32 uDirection);
//获取行驶方向
int GetDRIVEDIR(const char *szDiretion);
//获取违章类型
std::string GetViolationType(UINT32 uViolationType,int nType = 0);

//载入系统设置模板
void LoadSysModel(int nSysModel);
//获取指定目录磁盘空间大小
void GetDiskPathInfo();
//调试信息输出
void LogTrace(const char* pFileName, const char* format,...);

//检查boost版本号
int Mv_mvGet_BoostVision( char* strPath, char &cProvince, float &fVision  );

//车牌号码转换
void CarNumConvert(std::string& strCarNum,char wj[2]);

//检测时间范围，返回值 true=当前时间在范围内,false=不在范围内
bool CheckDetectTime();
//清空目录中的文件
void RemoveDir(const char *dirName);
//写临时配置文件(为了保存float型的相机参数)
void WriteCameraFloatIni(CAMERA_CONFIG cfg, int nChannel);
//根据相机型号载入相机配置(为了取出float型的相机参数)
void LoadCameraFloatProfile(CAMERA_CONFIG& cfg, int nChannel);
//读取配置文件，根据键获取值
bool GetProfileStringName(const char *key, string& value, const char *filename="sytem.ini");
//取得操作系统信息
void GetOsInfo();
//系统运行时间
int GetSystemUptime();
//系统运行时间（XX days XX hours XX minutes XX seconds ）
string MyGetSystemUptime();
//取虚拟内存
void GetSwapMem(int* used, int* total);
//取MAC地址
string GetMacAddr();
//硬盘读写速度测试
void GetDiskRwSpeed(int* readSpeed, int* writeSpeed);
//判断网卡是否连接
int check_nic(char *nic_name);
//获得网卡上行和下行速度
void GetRxTX(int &rx, int & tx);

//判断相机类型，是否为Roseek相机
bool IsRoSeekCamera(const int &nCamType);

//数字水印
void GetWaterMark(const char* pData,int nSize,std::string& strWaterMark);

//获取特征图像
int GetSmallImage(IplImage* pImg,unsigned char* pSmallJpgImage,CvRect rect);

//保存特征图像
int SaveSmallImage(IplImage* pImg,std::string strPicPath,CvRect rect,unsigned char * pOutImg);

//保存图像
void SaveExistImage(std::string strPicPath,BYTE* pOutImage,int nSize);

//车型细分转换
bool CarTypeConvert(RECORD_PLATE &plate);

//违章类型格式转换
bool VtsTypeConvert(RECORD_PLATE &plate);

//获取车型细分类型
int GetTypeDetail(int nRet);

//车辆类型文本
string GetDetailCarType(int uType,int uTypeDetail,int uDetailCarType);

//根据通道编号来获取车道限速列表
void GetMapMaxSpeedStrByChan(mapMaxSpeedStr &mapMaxSpeedStrTmp, int nChanId);

//根据车道编号来获取车道限速
UINT32 GetMaxSpeedStrByRoadId(mapMaxSpeedStr &mapMaxSpeedStrTmp, const int uType, const int nRoadId);

//根据车亮类型，通道号，车道号获取相应车道的限速值
UINT32 GetMaxSpeed(const int uType, const int nChannel, const int nRoadId);

//写文件多次完成
bool SafeWrite(FILE *fpOut, const char *pBuffer, const int nFrameSize);

//yuv->rgb
void ConvertYUVtoRGBImage(BYTE* pSrc,BYTE* pDest,int nWidth,int nHeight);

std::string GetFileName(std::string strFullFileName);
bool IsFileNameWithPath(std::string strFullFileName);
std::string FtpFormatConvert(std::string strFtpFmt,RECORD_PLATE* sPlate);

//递归删除目录
bool RemoveDir2( const char* dirname);
// 判断是否闰年
int IsLeapYear(int year);
// 根据月份判断日是否有效
int CheckDayValide(int nDay, int nMonth, int nYear);

void intToByte(unsigned char abyte0[4],int i);

//获取检测器程序版本
string GetVersion();

#ifdef DRAW_RECT_DEBUG
//在图像上,画矩形
void myRectangle(IplImage *pImg, const CvRect &rt, const CvScalar &color, const int &thickness);
#endif

void GlobalInit();

//信号互斥
extern pthread_mutex_t g_uImgKeyMutex;
//获取全局记录编号
UINT64 GetGlobalImgKey();

#endif
