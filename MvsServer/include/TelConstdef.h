#ifndef TEL_CONSTDEF_H
#define TEL_CONSTDEF_H

#define TEL_MAX_DATA_SIZE      65535
#define TEL_MD5_16			   16

#define TEL_IVM_HEAD              0xFAF5
#define TEL_CMS_EVENT_HEAD        0xFAF6
#define TEL_CMS_PIC_HEAD          0xFAF7

#define TEL_LOGIN           0x31      //登陆认证请求/确认
#define TEL_LINK            0x32      //链路检测请求/确认
#define TEL_ALG_ABILITY		0x33		//算法能力上报/确认
#define TEL_EXCEPTION	    0x34		//异常上报/确认

#define TEL_START_DETECT    0x41  	 //下发智能分析任务开始请求/确认
#define TEL_STOP_DETECT     0x42     //下发智能分析任务停止请求/确认
#define TEL_SEARCH_ALG      0x43     //智能分析结果数据项请求/确认

#define TEL_BEYOND_LINE   0x31	    //拌线/回应
#define TEL_FLUX	    0x32	    //客流人数统计/回应
#define TEL_DENSITY     0x33	    //客流密度统计/回应

#define TEL_FILE_ID			0x31	    //文件描述信息
#define TEL_IMAGE_FILE      0x32	    //文件数据包

#pragma pack(1)

enum LOGIN_STATE
{
	NOLOGIN,
	SUCCSESS,
	FAILED
};

//电信中心端报文头
typedef struct _TEL_CENTER_HEADER
{
	UINT16  uHead;		//起始字头
	BYTE    uCmd;		//命令字
	UINT16	uSeq;	    //流水号
	BYTE    bRemark;	//保留字段
	UINT16  uLen;		//数据长度

	_TEL_CENTER_HEADER()
	{
        uHead = 0;
        uCmd = 0;
        uSeq = 0;
        bRemark = 0;
        uLen = 0;
	}
}TEL_CENTER_HEADER;

//电信中心端拌线
typedef struct _TEL_CENTER_BEYONDLINE
{
    char    sSession[32];    //任务会话
	BYTE    uLineID;		//报警源编号
    char    sTime[14];       //报警时间
    BYTE    uDirection;		//报警方向

	_TEL_CENTER_BEYONDLINE()
	{
        memset(sSession,0,32);
        memset(sTime,0,14);
        uLineID = 0;
        uDirection = 0;
	}
}TEL_CENTER_BEYONDLINE;

//电信中心端客流人数统计
typedef struct _TEL_CENTER_FLUX
{
	char    sSession[32];    //任务会话
	UINT16	uNum;			 //统计人数
	char    sBeginTime[14]; //报警开始时间
	char    sEndTime[14];   //报警结束时间
	BYTE    uDirection;		//统计方向

	_TEL_CENTER_FLUX()
	{
		memset(sSession,0,32);
		uNum = 0;
		memset(sBeginTime,0,14);
		memset(sEndTime,0,14);
		uDirection = 0;
	}
}TEL_CENTER_FLUX;

//电信中心端客流密度统计
typedef struct _TEL_CENTER_DENSITY
{
	char    sSession[32];    //任务会话
	UINT16	uNum;			 //统计人数
	char    sDensity[3];	//统计密度
	char    sTime[14];		//统计时间
	BYTE    uDirection;		//统计方向

	_TEL_CENTER_DENSITY()
	{
		memset(sSession,0,32);
		uNum = 0;
		memset(sDensity,0,3);
		memset(sTime,0,14);
		uDirection = 0;
	}
}TEL_CENTER_DENSITY;

//电信中心端单个文件传输请求
typedef struct _TEL_CENTER_FILEID
{
	char    sSession[32];    //任务会话
	UINT32	uFileSize;		//文件大小
	char    sEventTime[14];	//事件发生时间
	char    sFileTime[14];  //文件产生时间

	_TEL_CENTER_FILEID()
	{
		memset(sSession,0,32);
		uFileSize = 0;
		memset(sEventTime,0,14);
		memset(sFileTime,0,14);
	}
}TEL_CENTER_FILEID;

//电信中心端文件数据包
typedef struct _TEL_CENTER_DATA
{
	UINT32	uFileId;		//文件标识
	BYTE	uEndFlag;		//文件结束标志
	UINT16  uDataLen;		//数据包大小

	_TEL_CENTER_DATA()
	{
		uFileId = 0;
		uEndFlag = 0;
		uDataLen = 0;
	}
}TEL_CENTER_DATA;

#endif
