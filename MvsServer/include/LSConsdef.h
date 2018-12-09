// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
//验证请求
#define LS_AUTH_CODE 0xefff0001
//验证状态回复
#define LS_AUTH_REP 0xefff0002
//验证失败
#define LS_AUTH_ERROR 0xefff0003

//卡口识别数据头
#define LS_PLATE_INFO 0xefff0004
//接收数据回复
#define LS_PLATE_REP 0xefff0005
//接收记录失败
#define LS_PLATE_ERROR 0xefff0006

//心跳信号
#define LS_LINK_TEST 0xefff0007

//信息头
typedef struct _LS_HEADER
{
	UINT32 uCmdID;		//命令类型
	UINT32 uCmdLen;		//命令长度

	_LS_HEADER()
	{
		uCmdLen = 0;
		uCmdID = 0;
	}
}LS_HEADER;

//关于全景图的信息
typedef struct _LS_PIC_INFO
{
    UINT64 uPicId; //图片编号，填默认值填0
    UINT32 uPicSize; //图片字节数
    UINT32 uPicDataCount; //关于图片的识别数据个数n条
    UINT32 uPicWidth; //图片宽度
    UINT32 uPicHeight; //图片高度
    char chChannelId[48]; //通道编号
    char chPicPath[48]; //图片存储路径，填默认值”\0”

    _LS_PIC_INFO()
    {
        uPicId = 0;
        uPicSize = 0;
        uPicDataCount = 0;
        uPicWidth = 0;
        uPicHeight = 0;
        memset(chChannelId, 0, 48);
        memset(chPicPath, 0, 48);
    }
}LS_PIC_INFO;


