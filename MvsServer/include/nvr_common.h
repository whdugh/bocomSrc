/*
 * AVS NVR SDK common header
 */
#ifndef _NVR_COMMON
#define _NVR_COMMON
#include <stdlib.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif

#define NET_DVR_SYSHEAD 1             //header
#define NET_DVR_STREAMDATA 2   //data stream

#define CameraIDLen 256
#define camerNameLEN 256
#define ReadNum 500000    //the bytes of a unit of recieved data
//#define MAX_STREAMS 500

#ifndef BUFFER_SIZE_
#define BUFFER_SIZE_
//#define SECOND_STREAM_SIZE 500000 //the bytes of buffer to store
#endif

typedef enum{
	SEND_BUFFER = 0,
	RECEIVE_BUFFER = 1
}BufferType;

#ifndef _CB_GETHEADER_
#define _CB_GETHEADER_
struct CB_GETHEADER{
		unsigned int recHandle;
		char* pBuffer;
		unsigned int BufferSize;
		unsigned int pDownParam;
	};
typedef struct CB_GETHEADER CB_GETHEADER;
#endif

#ifndef _CB_STORE_
#define _CB_STORE_
struct CB_STORE{
		unsigned int loginID;
		char  camerID[CameraIDLen];
		char  camerName[camerNameLEN];
		int recordType;
		int recVlmID;
		char * header;
		unsigned int headLen;
		char * desc;
		int descLen;
	};
typedef struct CB_STORE CB_STORE;
#endif

#ifndef CB_FMSG
#define CB_FMSG
struct CB_FMsg{
	unsigned int playTask;
	void* dwUser;
	int iErrorCode;
};
typedef struct CB_FMsg CB_FMsg;
#endif

#ifndef CB_RECORDINFO
#define CB_RECORDINFO
struct CB_RINFO{
	unsigned int loginID;
	char* CamerID;
	unsigned int beginTime;
	unsigned int endTime;
	const char* AlarmType;
	const char* describe;
	unsigned int describeLen;
	unsigned int pDownParam;
};
typedef struct CB_RINFO CB_RINFO;
#endif


#ifndef FRECVINFOCALLBACK
#define FRECVINFOCALLBACK
typedef void (*fRecVlmInfoCallBack)(int lvID , const char* lvName , 
			int activeDevices , int totalDevices , unsigned int gSizeHigh , 
			unsigned int gSizeLow , unsigned int UserHigh , unsigned int UserLow ,
			int wdays , int policy , unsigned int pDownParam);
#endif

#ifndef FRECORDINFOCALLBACK
#define FRECORDINFOCALLBACK
typedef void (*fRecordInfoCallBack)(unsigned int loginID , const char* CamerID ,
			unsigned int beginTime , unsigned int endTime , const char* AlarmType ,
			const char* describe , unsigned int describeLen ,  unsigned int pDownParam);
#endif

#ifndef FGETRECORDCALLBACK
#define FGETRECORDCALLBACK
typedef void (*fGetRecordCallBack)(unsigned int recHandle, char* pBuffer ,
			unsigned int BufferSize , unsigned int pDownParam);
#endif

#ifndef FMSGCALLBACK
#define FMSGCALLBACK
typedef void(*fMsgCallback)(unsigned int playTask, void * dwUser, int iErrorCode);
#endif

//void AVS_setRecvBufSize(unsigned int);

//启动录象
unsigned int AVS_startRecord(unsigned int loginID, char *camerID, char *camerName,int recordType,
						int recVlmID, char *header,unsigned int headLen, char *desc, int descLen);
//发送录象数据
int AVS_record(unsigned int recordHandle,const char *data ,int dataLen,unsigned int beginTime,unsigned int endTime);
//停止录象
int AVS_stopRecord(unsigned int recordHandle);
//获取录象卷信息
int AVS_getRecVlmInfo(unsigned int loginID,fRecVlmInfoCallBack cbVlmInfo, void* cbArgs);
//查询录象段
int AVS_getRecordInfo(unsigned int loginID,char *camerID,int recordType,unsigned int beginTime,unsigned int endTime,
					fRecordInfoCallBack cbRecordInfo,void* cbArgs);
//修改录象段描述信息
int AVS_setRecordInfo (unsigned int loginID,const char *camerID,unsigned int beginTime,unsigned int endTime,
					const char *desc,int descLen);
//删除录象段
int AVS_deleteRecord(unsigned int loginID,const char *camerID,unsigned int beginTime,unsigned int endTime,int recordType);
//获取录象段头数据
int AVS_getRecordHeader(unsigned int loginID,const char *camerID,int recordType,unsigned int beginTime,
					unsigned int endTime,char **recvBuffer,unsigned int *recvLen);
//下载录象
unsigned int AVS_getRecord(unsigned int loginID,char *camerID,int recordType,unsigned int beginTime,
						unsigned int endTime,fGetRecordCallBack cbRecordData, void* cbArgs);
//停止下载录象
int AVS_stopGetRecord(unsigned int playHandle);
//登录视频存储设备
unsigned int AVS_login(const char *ip,int port, char *userName,char *pwd,fMsgCallback cbPlayMsg,void* pUserData);
//退出登录
int AVS_logout (unsigned int loginID);
//倒序下载
unsigned int AVS_getRecordReverse(unsigned int loginID,char *camerID,int recordType,unsigned int beginTime,
								unsigned int endTime,fGetRecordCallBack cbRecordData, void* cbArgs);
//停止倒序下载
int AVS_stopGetRecordReverse(unsigned int playHandle);
//获取错误代码
unsigned long AVS_getLastError();
//获取指定区间的录象段大小
int AVS_getRecordSegSize(unsigned int loginID,const char *cameraID,unsigned int startTime,unsigned int endTime);
//创建录象卷（为摄像头分配空间）
int AVS_createRecordVol(unsigned int loginID,char *name,char *alias,
		short savedDays,char delPolicy, char encodeType, unsigned int blocks,unsigned int blockSize);
//删除录象卷（回收摄像头所占空间）
int AVS_deleteRecordVol(unsigned int loginID,const char *cameraID,int mode);

//set buffer size and the interval to get record from the server
void AVS_setRecvBufSize(unsigned int size, unsigned int  interval);

#ifdef __cplusplus
}
#endif



#endif /* !_NVR_COMMON */

