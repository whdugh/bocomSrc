#include "DoorAlarm.h"
#include "RoadLog.h"
CDoorAlarm g_DoorAlarm;

void * ThreadDoorAlarm(void * arg)
{

	g_DoorAlarm.RecData();

	return arg;
}


CDoorAlarm::CDoorAlarm()
{
}

CDoorAlarm::~CDoorAlarm()
{
	
}

void CDoorAlarm::BeginThread()
{
	//线程id
	pthread_t id;
	 //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
   
    if(pthread_create(&id,NULL,ThreadDoorAlarm,this)!=0)
    {
		printf("报警线程初始化失败 ！\r\n");
    }

    pthread_attr_destroy(&attr);

	return;
}

//接收串口消息
void CDoorAlarm::RecData()
{
	int nCount = 0;
	char buf[256]={0};
	while (!g_bEndThread)                                   
	{
		memset(buf,0,256);
		int nread = read(fd_com, buf, 4);

		if(nread != -1)
		{
			 if(nread >= 4)
			 {
				// LogNormal("DoorAlarm::RecData %x,%x,%x,%x\n",buf[0],buf[1],buf[2],buf[3]);
				 if((buf[0] == 0xfffffff6) &&( buf[3] == 0xffffffaa))
				 {
					// LogNormal("0xf6,0xaa\n");
					 
					 if(buf[2] == 0x01)
					 {
						 if(nCount == 0)
						 {
							 g_skpRoadLog.WriteLog("机箱开门报警\n",ALARM_CODE_BOX_OPEN_DOOR,true);
							 
						 }
						  
					 }

					 nCount++;
					 if(nCount == 120)
					 {
						 nCount = 0;
					 }
				 }
			 }
			 else if(nread >= 1)
			 {
				LogNormal("DoorAlarm::RecData %x,%x,%x,%x\n",buf[0],buf[1],buf[2],buf[3]);
			 }
		}
		
		usleep(100);
	}
}


bool CDoorAlarm::OpenDev()
{

	fd_com = open_port(g_DoorComSetting.nComPort, g_DoorComSetting.nBaud, g_DoorComSetting.nDataBits, g_DoorComSetting.nStopBits, 0);


	if (fd_com < 0)
	{
		return false;
	}
	else
	{
		BeginThread();
		return true;
	}

}

	

