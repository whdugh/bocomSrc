#ifndef DIOCONNET_H
#define DIOCONNET_H
/*
	文件 dioconnct.h
	功能：dio的发送信号的控制
	作者： niuheshan
	时间： 20121218
*/


#include <stdio.h>
#include <asm/ioctl.h>

#define MAGIC_NUM 'T'

#define DAS_IOC_SET_PATH  \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 0, 4*sizeof(__u32))
#define DAS_IOC_SOFT_OUT  \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 1, sizeof(__u32))
#define DAS_IOC_GET_SBGIO  \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 2, sizeof(__u32))
#define DAS_IOC_WDT_LED_CTRL  \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 3, 2*sizeof(__u32))
#define DAS_IOC_WDT_CTRL  \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 4, 2*sizeof(__u32))

#define DAS_IOC_GET_GPI \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 5, sizeof(__u32))
#define DAS_IOC_SET_GPO \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 6, sizeof(__u32))
#define DAS_IOC_GET_GPO \
	_IOC(_IOC_READ|_IOC_WRITE, MAGIC_NUM, 7, sizeof(__u32))

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

class DioConnect
{
public:
	
	DioConnect();
	~DioConnect();

	// 发送信号
	short GPO_Write(void* pState);	
	// 打开设备！
	short GPIO_Init(void);						
	// 关闭设备！
	void GPIO_unInit(void);						
	// 根据车道号 
	int CreatethreadForLeanNo(int RoadIndex);

	unsigned short m_initFlag;   //是否初始化
	int m_DevFd;

private:

	pthread_mutex_t		m_ComPoseWriteLock;
};

void * SendDioSignal(void * pAg);
#endif

extern DioConnect g_dioConnect;
