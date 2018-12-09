#include "MonitoringAndAlarm.h"

int isChoiceAlarm = 0;

MonitoringAndAlarm g_AlarmSerial;

void * ThreadMonitoringAndAlarm(void * arg)
{

	g_AlarmSerial.RecData();

	return arg;
}


MonitoringAndAlarm::MonitoringAndAlarm(int freq, int length , int reps, int delay)
{
	m_freq = freq;
	m_length = length;
	m_reps = reps;
	m_delay =delay;
	//intAlarm();
	m_nThreadId = 0;
}

MonitoringAndAlarm::~MonitoringAndAlarm()
{
	
}

bool MonitoringAndAlarm::iniAlarmThread()
{
	 //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
   
    if(pthread_create(&m_nThreadId,NULL,ThreadMonitoringAndAlarm,this)!=0)
    {
		printf("报警线程初始化失败 ！\r\n");
       
		pthread_attr_destroy(&attr);
		return false;
    }

    pthread_attr_destroy(&attr);

	return true;
}

//接收串口消息
void MonitoringAndAlarm::RecData()
{
	printf("监视报警设备线程开始\n");
      
	int n = 0;

	while (!g_bEndThread)                                   
	{

		int write_zize = sendPack();

		if (write_zize == -1 )
		{
			continue;
		}
		//sleep(1);
		usleep(100);		
		char buff[20] = {0};

		int read_size = read(fd_com, buff, sizeof buff);

		printf("读到的数是 %d \n" ,buff[3]);
        printf("读到 %d 个字节 \n" ,read_size);
		if (buff[3] == 1)
		{   n++;
		//	beginAlarm();

			if(n == 1)
				LogNormal("非法打开机箱\r\n");
		}
		else
		{   n = 0;
		//	endAlarm();
		}
		sleep(1);
	}
}


int MonitoringAndAlarm::sendPack()
{

	char  anser[20] = "01ROP0";
	char * sendPack = pack(anser);
	int w = write(fd_com, sendPack, strlen(sendPack));
	
	if (w == -1)
	{
		return -1;
	}
	printf("%d  %s \n", strlen(sendPack), sendPack);

	return 1;
}

int MonitoringAndAlarm::OpenDev()
{

	//打开ut5510 报警串口
	//while(1)
	{
		fd_com = open_port(g_DoorComSetting.nComPort, g_DoorComSetting.nBaud, g_DoorComSetting.nDataBits, g_DoorComSetting.nStopBits, 0);
    // 打开串口进行对ut5510 进行通讯
	//m_fd_alarm = open ( "/dev/console", O_WRONLY );   // 打开报警器


	if (fd_com < 0 /*|| m_fd_alarm == -1*/)
	{
		printf(" open MonitoringAndAlarm device error \n");
		return -1;
	}
	else
	{
		iniAlarmThread();
	//	break;
	}

	}

	return 1;
}

int MonitoringAndAlarm::beginAlarm()   // 发音
{
	for (int i = 0; i < m_reps; i++)
	{ 
		//数字 1190000 从何而来， 主板频率
		int magical_fairy_number = 1190000/m_freq; 
		//ioctl(m_fd_alarm, KIOCSOUND, magical_fairy_number); //开始发音
						   //停止发音
		usleep(1000*m_delay);						 
	} 
	return 1;
}

int MonitoringAndAlarm::endAlarm()
{
	ioctl(m_fd_alarm, KIOCSOUND, 0);				   //停止发音
	return 1;
}

char * MonitoringAndAlarm::pack(char buffer[])
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

	printf("%c, %d\n", bCheckSum, bCheckSum);
	return bStrData;
}

char MonitoringAndAlarm::checkSum(char *sData)
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

char MonitoringAndAlarm::strHex(char temp)
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

/*
int main()
{
	MonitoringAndAlarm * alarm = new MonitoringAndAlarm();
	alarm->iniAlarmThread();
	sleep(1000);
	return 1;
}*/









/*	int fd_com = open("/dev/ttyS0", O_RDWR);
	char  anser[20] = "01ROP0";
	char * sendPack = Pack(anser);
	int w = write(fd_com, sendPack, strlen(sendPack));
	printf("%d  %s \n", strlen(sendPack), sendPack);
	
	sleep(1);
	
	char buff[10];
	while (1)
	{
		int w = write(fd_com, sendPack, strlen(sendPack));
		char ch;
		usleep(100);		
		int read_size = read(fd_com, buff, 10);
				
		printf("收到一个字符 %d %c\n", buff[3],buff[3] );	 
	 }*/