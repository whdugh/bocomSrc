#include "DioComSignalProtocol.h"
#include "CMDToCamera.h"

DioComSignalProtocol g_DioComSignalProtocol;


int DioComSignalProtocol::m_fd = 0;
volatile int DioComSignalProtocol::m_road1IsOpen = 0;
volatile int DioComSignalProtocol::m_road2IsOpen = 0;
volatile int DioComSignalProtocol::m_road3IsOpen = 0;
volatile int DioComSignalProtocol::m_road4IsOpen = 0;


DioComSignalProtocol::DioComSignalProtocol()
{
	pthread_mutex_init(&m_ComPoseWriteLock, NULL);
}

DioComSignalProtocol::~DioComSignalProtocol()
{
	pthread_mutex_destroy(&m_ComPoseWriteLock);
	close(m_fd);
}

int DioComSignalProtocol::SetCommSpeed(int fd, int nSpeed)
{

	int   status;
	struct termios   Opt;
	if(tcgetattr(fd, &Opt)==-1)
	{
		printf("无法获取串口属性！\r\n");
		return -1;
	}

	Opt.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);	/*Input*/
	Opt.c_oflag  &= ~OPOST;							/*Output*/

	tcflush(fd, TCIOFLUSH);
	switch(nSpeed)
	{
	case 9600:
		cfsetispeed(&Opt,B9600);
		cfsetospeed(&Opt,B9600);
		break;
	case 57600:
		cfsetispeed(&Opt,B57600);
		cfsetospeed(&Opt,B57600);
		break;
	case 115200:
		cfsetispeed(&Opt,B115200);
		cfsetospeed(&Opt,B115200);
		break;

	}
	status = tcsetattr(fd, TCSANOW, &Opt);
	if  (status != 0)
	{
		printf("无法设置串口波特率！\r\n");
		return -1;
	}
	tcflush(fd,TCIOFLUSH);

	return 0;
}

//打开设备
int DioComSignalProtocol::OpenDev()
{  

	m_fd = open_port(g_DioComSetting.nComPort, g_DioComSetting.nBaud,\
		g_DioComSetting.nDataBits, g_DioComSetting.nStopBits, 0);

	LogNormal("ut 5510 oepnDev fun！%d\n", g_DioComSetting.nComPort);
	if (m_fd == -1)
	{
		LogNormal("dio - ut5510版  串口打开失败");
		return -1;
	}

	// 设置ut 5510 的波特率
	string  sendPositive = "01SB3"; 
	int iResul = sendMessage(sendPositive);

	// 设置 串口的波特率
	SetCommSpeed(m_fd, 9600);

	return 0;
}

// 根据车道号 
int DioComSignalProtocol::CreatethreadForLeanNo(int RoadIndex)
{
	cerr<< "车道号是"<<RoadIndex<<endl;
	if (RoadIndex ==  1)
	{
		// 车道1 的线程
		pthread_attr_t attr1;
		pthread_attr_init(&attr1);
		pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
		pthread_t pthredId1;
		if (pthread_create(&pthredId1, NULL, RoadIndex1, this) != 0)
		{
			pthread_attr_destroy(&attr1);
			return -1;
		}
	}
	else if (RoadIndex == 2)
	{
		//车道2 的线程
		pthread_attr_t attr2;
		pthread_attr_init(&attr2);
		pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
		pthread_t pthredId2;
		if (pthread_create(&pthredId2, NULL, RoadIndex2, this) != 0)
		{
			pthread_attr_destroy(&attr2);
			return -1;
		}
	}
	else if (RoadIndex == 3)
	{
		//车道3 的线程
		pthread_attr_t attr3;
		pthread_attr_init(&attr3);
		pthread_attr_setdetachstate(&attr3, PTHREAD_CREATE_DETACHED);
		pthread_t pthredId3;
		if (pthread_create(&pthredId3, NULL, RoadIndex3, this) != 0)
		{
			pthread_attr_destroy(&attr3);
			return -1;
		}
	}
	else if (RoadIndex == 4)
	{
		//车道4 的线程
		pthread_attr_t attr4;
		pthread_attr_init(&attr4);
		pthread_attr_setdetachstate(&attr4, PTHREAD_CREATE_DETACHED);
		pthread_t pthreadId4;
		if (pthread_create(&pthreadId4, NULL, RoadIndex4, this) != 0)
		{
			pthread_attr_destroy(&attr4);
			return -1;
		}
	}
	return 0;
}

int DioComSignalProtocol::sendMessage(string strAnser)
{

	char anser[20] = {0};
	strcpy(anser, strAnser.c_str());
	char * sendPack = pack(anser);
	int sendSize = write(m_fd, sendPack, strlen(sendPack));

	//cerr<<"发送的大小是："<<strlen(sendPack)<<" 发送的内容是："<<sendPack<<endl;

	if (sendSize <= 0)
	{
		return -1;
	}

	return 0;
}


void DioComSignalProtocol::ComPoseMsgAndSendMsg(int type)
{
	if (type == 1)    //发高电频
	{
		int nVal = 0;
		if (m_road1IsOpen == 1)
		{
			nVal += 1;
		}
		if (m_road2IsOpen == 1)
		{
			nVal += 2;
		}
		if (m_road3IsOpen == 1)
		{
			nVal += 4;
		}
		if (m_road4IsOpen == 1)
		{
			nVal += 8;
		}

		char  cVal;					//转换成16 进制的形式
		sprintf(&cVal, "%x", nVal);  
		char szVal[15] = {0};			//先转换成大些的形式在组合起来       
		sprintf(szVal, "01DOP00%c", toupper(cVal));
		string strSendUpper(szVal);
		cout<<strSendUpper<<endl;
		sendMessage(strSendUpper);

	}
	else          // 发低电频
	{
		int nVal = 15;

		if (m_road1IsOpen == 0)
		{
			nVal -= 1;
		}
		if (m_road2IsOpen == 0)
		{
			nVal -= 2;
		}
		if (m_road3IsOpen == 0)
		{
			nVal -= 4;
		}
		if (m_road4IsOpen == 0)
		{
			nVal -= 8;
		}

		char cVal;					//转换成16 进制的形式
		sprintf(&cVal, "%x", nVal);  

		char szVal[15] = {0};			//先转换成大些的形式在组合起来       
		sprintf(szVal, "01DOP00%c", toupper(cVal));
		string strSendLower(szVal);
		cout<<nVal<<"\t"<<m_road1IsOpen<<m_road2IsOpen<<m_road3IsOpen<<m_road4IsOpen<<"\t"<<strSendLower<<endl;
		sendMessage(strSendLower);
	}	
}

void * DioComSignalProtocol::RoadIndex1(void * arg)
{
	DioComSignalProtocol * pThis = (DioComSignalProtocol  *)arg;

	pThis->SetComPoseWriteLock();			   // 连接 
	m_road1IsOpen = 1;
	pThis->ComPoseMsgAndSendMsg(1);
	pThis->SetComPoseWriteUnlock();	

	usleep(400 * 1000);						// 信号的维持时间

	pThis->SetComPoseWriteLock();				// 断开
	m_road1IsOpen = 0;
	pThis->ComPoseMsgAndSendMsg(0);
	pThis->SetComPoseWriteUnlock();	
}

void * DioComSignalProtocol::RoadIndex2(void * arg)
{
	DioComSignalProtocol * pThis = (DioComSignalProtocol *)arg;

	pThis->SetComPoseWriteLock();			    // 连接 
	m_road2IsOpen = 1;
	pThis->ComPoseMsgAndSendMsg(1);
	pThis->SetComPoseWriteUnlock();	

	usleep(400 * 1000);						// 信号的维持时间

	pThis->SetComPoseWriteLock();				// 断开
	m_road2IsOpen = 0;
	pThis->ComPoseMsgAndSendMsg(0);
	pThis->SetComPoseWriteUnlock();	
}

void * DioComSignalProtocol::RoadIndex3(void * arg)
{
	DioComSignalProtocol * pThis = (DioComSignalProtocol *)arg;

	pThis->SetComPoseWriteLock();			    // 连接 
	m_road3IsOpen = 1;
	pThis->ComPoseMsgAndSendMsg(1);
	pThis->SetComPoseWriteUnlock();	

	usleep(400 * 1000);						// 信号的维持时间

	pThis->SetComPoseWriteLock();				// 断开
	m_road3IsOpen = 0;
	pThis->ComPoseMsgAndSendMsg(0);
	pThis->SetComPoseWriteUnlock();	

}

void * DioComSignalProtocol::RoadIndex4(void * arg)
{
	DioComSignalProtocol * pThis = (DioComSignalProtocol *)arg;

	pThis->SetComPoseWriteLock();			    // 连接 
	m_road4IsOpen = 1;
	pThis->ComPoseMsgAndSendMsg(1);
	pThis->SetComPoseWriteUnlock();	

	usleep(400 * 1000);						// 信号的维持时间

	pThis->SetComPoseWriteLock();				// 断开
	m_road4IsOpen = 0;
	pThis->ComPoseMsgAndSendMsg(0);
	pThis->SetComPoseWriteUnlock();	
}


char * DioComSignalProtocol::pack(char buffer[])
{
	char bStart=1;
	char bEnd=4;
	char bCheckSum;
	char bStrData[1024] = {0};
	bCheckSum=checkSum(buffer);

	bStrData[0] = bStart;
	strcat(bStrData,buffer);
	bStrData[strlen(bStrData)] = bCheckSum;
	bStrData[strlen(bStrData)] = bEnd;

	return bStrData;
}

char DioComSignalProtocol::checkSum(char *sData)
{
	char checksum=0;
	int i,j;
	j=strlen(sData);
	for(i=0;i<=j-1;i++)
	{
		checksum=checksum+strHex(sData[i])&127;
	}
	if(checksum<=32)
		checksum+=32;

	return checksum;
}

char DioComSignalProtocol::strHex(char temp)
{
	char value=0;
	if((temp>='0')&&(temp<='9'))
		value=temp;
	else if((temp>='A')&&(temp<='Z'))
		value=temp;
	else
		value=-1;
	return value;
}



