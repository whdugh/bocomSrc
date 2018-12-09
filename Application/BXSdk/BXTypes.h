#ifndef _BXTYPES_H_
#define _BXTYPES_H_

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<poll.h>
#include<pthread.h>
#include <netinet/tcp.h>
#include <sys/time.h>

#ifndef BX_Int8
typedef char BX_Int8;
#endif

#ifndef BX_Uint8
typedef unsigned char BX_Uint8;
#endif

#ifndef BX_Int16
typedef short BX_Int16;
#endif
#ifndef BX_Uint16
typedef unsigned short BX_Uint16;
#endif

#ifndef BX_Int32
typedef int BX_Int32;
#endif
#ifndef BX_Uint32
typedef unsigned int BX_Uint32;
#endif

#ifndef BX_Int64
typedef long long BX_Int64;
#endif

#ifndef BX_Uint64
typedef unsigned long long BX_Uint64;
#endif

//获取设备时间回调，返回值0表示成功，-1失败
typedef BX_Int32 (*BX_GetDeviceTime)( BX_Uint64 *pU64msSecond );
//设置设备时间回调，返回值0表示成功，-1失败
typedef BX_Int32 (*BX_SetDeviceTime)( BX_Uint64 U64msSecond );
//获取设备分辨率回调，返回值0表示成功，-1失败
typedef BX_Int32 (*BX_GetDeviceWH)(BX_Uint32 *pWidth,BX_Uint32 *pHeight);
typedef BX_Int32 (*BX_LOG)(const BX_Int8* format,...);
typedef struct
{
	BX_GetDeviceTime GetDeviceTime;
	BX_SetDeviceTime SetDeviceTime;
	BX_GetDeviceWH GetDeviceWH;
}BX_CB_FUNS;


//返回值定义
typedef enum
{
	BX_OK = 0,
	BX_RE_INIT,			//重复初始化
	BX_PARAM_ERROR ,	//参数错误
	BX_NO_CONNECT ,		//数据端口无连接
	BX_SERVER1_FAILED ,	//创建35000数据端口服务失败
	BX_SERVER2_FAILED ,	//创建45000数据端口服务失败
	BX_SEND_ERROR,		//发送数据失败
}BX_ERRNO;



typedef enum __E_CarMarkType
{
	BX_NOTYPE = 0,		//不识别
	BX_DAZHONG = 1,		//大众
	BX_BIEKE = 2,		//别克
	BX_BENTIAN = 3,		//本田
	BX_FENGTIAN = 4,	//丰田
	BX_XIANDAI = 5,		//现代
	BX_BIAOZHI = 6,		//标致
	BX_BAOMA = 7,		//宝马
	BX_NISANG = 8,		//尼桑
	BX_AODI = 9,		//奥迪
	BX_FUTE = 10,		//福特
	BX_XUFL = 11,		//雪弗兰
	BX_QIRUI = 12,		//奇瑞
	BX_MAZD = 13,		//马自达
	BX_BENCHI = 14,		//奔驰
	BX_DONGFENG = 15,	//东风
	BX_JIBEI = 16,		//金杯
	BX_QIYA = 17,		//起亚
	BX_XUETL = 18,		//雪铁龙
	BX_WULING = 19,		//五菱
	BX_EWK = 20,		//依维柯
	BX_QITA = 255,		//其他
}E_CarMarkType;

//返回值定义
typedef enum
{
	BX_KAKOU = 0,
	BX_WEITING = 1,
}BX_EVNET_TYPE;


typedef struct __BX_HeadData
{
	BX_Uint32 uiEventState;//事件状态，0-实时，1-历史，用于统计流量

	BX_Uint8 ui8TotalSnapTimes;//一次触发抓拍张数
	BX_Uint8 ui8CurTimes;//当前是第几次抓拍，0-表示卡口，1-表示违停
	BX_Uint64 ui8Time;//事件时间毫秒级，%1000为毫秒/1000为秒
	BX_Uint8 uiCarMarkType;//车标志
	BX_Uint16 ui16EventType;  //触发类型
	BX_Uint32 uiIllegalCode;	//违法代码
	BX_Uint32 uiRoadNo;		//车道号，为虚拟车到号，统一为0
	BX_Uint32 uiRecordID;		//图片记录了索引
	BX_Uint32 uiRedtime;	//红灯经过时间
	BX_Uint32 uiSpeed;		//车速
	BX_Uint32 uiCarLength;	//车辆长度
	BX_Uint8 bHasVideo;		//是否有录像，1表示有，0表示无
	BX_Uint8 bIsOverLap;	//是否跨越线圈
	BX_Uint8 uiCameraId;	//摄像机Id编号
	BX_Uint8 uiSourceCam;	//源相机
	BX_Uint8 uiOtherCam;	//联动相机
	BX_Uint32 ui32EventID;		//事件ID同一个事件的图片此id相同
	BX_Uint8 uiVideoFtpPath[125];//事件录像存储的ftp地址，只有违停需要录像
//**********车牌信息
	BX_Uint8 szPlateCode[20];	//车牌号码
	BX_Uint16 ui16PlateColor;//车牌颜色
	BX_Uint32 uiCarNumW;	//车牌宽
	BX_Uint32 uiCarNumH;	//车牌高
	BX_Uint32 uiCarNumLUY;	//车牌左上角Y坐标
	BX_Uint32 uiCarNumRDY;	//车牌右下角Y坐标
	BX_Uint32 uiCarNumLUX;	//车牌左上角X坐标
	BX_Uint32 uiCarNumRDX;	//车牌右下角X坐标
	BX_Uint32 uiRecPro;		//识别概率
	BX_Uint16 uiCarType;	//车型，1小型车，2中型车，3大型车，4未知车型
	BX_Uint16 uiCarColorCode;	//车身颜色代码
	BX_Uint16 uiCarColorSatCode;//车身颜色饱和度代码
	BX_Uint32 uiCarColor;		//车身颜色
	BX_Uint32 uiCarColorSat;	//车身颜色饱和度

	BX_Uint32 uiGrayX;			//灰度拉伸区域左上角X坐标
	BX_Uint32 uiGrayY;			//灰度拉伸区域左上角Y坐标
	BX_Uint32 uiGrayW;			//灰度拉伸区域宽度
	BX_Uint32 uiGrayH;			//灰度拉伸区域高度
	BX_Uint32 uiCarNumStru;		//车牌结构
	BX_Uint32 uiCarNumTime;		//车牌识别时间


	//备注：子图1-2用于保存号牌抠图、子图3-4用于保存车廓图、子图5-6用于保存人脸识别
	//如果相应的子图不存在，则将该偏移地址和长度填0

	BX_Uint32 uiChildPicNum;	//子图个数，最多6张
	BX_Uint32 uiChildPic1Skew;	//子图1偏移地址
	BX_Uint32 uiChildPic1Len;	//子图1长度
	BX_Uint32 uiChildPic2Skew;	//子图2偏移地址
	BX_Uint32 uiChildPic2Len;	//子图2长度
	BX_Uint32 uiChildPic3Skew;	//子图3偏移地址
	BX_Uint32 uiChildPic3Len;	//子图3长度
	BX_Uint32 uiChildPic4Skew;	//子图4偏移地址
	BX_Uint32 uiChildPic4Len;	//子图4长度
	BX_Uint32 uiChildPic5Skew;	//子图5偏移地址
	BX_Uint32 uiChildPic5Len;	//子图5长度
	BX_Uint32 uiChildPic6Skew;	//子图6偏移地址
	BX_Uint32 uiChildPic6Len;	//子图6长度


}BX_HeadData;


#endif	//_BXTYPES_H_