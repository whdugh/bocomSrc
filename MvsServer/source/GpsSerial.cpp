
#include "Common.h"
#include "GpsSerial.h"

CGpsSerial g_GpsSerial;

//记录接收线程
void* ThreadRecSerialCmd(void *pArg)
{
    g_GpsSerial.RecData();
}

CGpsSerial::CGpsSerial()
{
     fd_com=-1;

    //m_nTimesWait = 0;
}

CGpsSerial::~CGpsSerial()
{

}


bool CGpsSerial::OpenDev()
{
    //打开相机控制串口
    printf("CamerSerial::OpenDev g_GpsComSetting.nComPort=%d,g_GpsComSetting.nBaud=%d\n",\
				g_GpsComSetting.nComPort,g_GpsComSetting.nBaud);

	fd_com = open_port(g_GpsComSetting.nComPort, g_GpsComSetting.nBaud, g_GpsComSetting.nDataBits, g_GpsComSetting.nStopBits, g_GpsComSetting.nParity);

	if(fd_com == -1)
	{
	    return false;
	}
	else
	{
	    BeginThread();
	    return true;
	}
}

//开启串口控制命令发送线程
void CGpsSerial::BeginThread()
{
    printf("===========CGpsSerial::BeginThread()=========\n");

	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//启动记录发送线程

	int nret=pthread_create(&id,&attr,ThreadRecSerialCmd,NULL);
	if(nret!=0) //失败
	{
		printf("启动串口发送线程失败!\r\n");
	}
	else
	{
		printf("启动串口发送线程成功!\r\n");
	}
	pthread_attr_destroy(&attr);
}


//接收串口GPS消息
void CGpsSerial::RecData()
{
    bool bGetTime = false; //是否获得了时间
    char buf[256]={0};
    int nCounts = 0; //时间计数器--每到3600归零一次，（2分钟一次）
	while(!g_bEndThread)
	{
		printf("======ThreadRecSerialCmd=====Receiving....\n");
		//fcntl(fd_com, F_SETFL, FNDELAY);
		int nread = read(fd_com, buf, 10);
		if(nread != -1)
		{
		    printf("======ThreadRecSerialCmd=====Receiving....nread=%d\n",nread);


            if(nread >= 10)
            {
				//LogNormal("=nread=%d=buf:%x=%x=%x=%x=%x=%x=%x=%x=%x=%x\n", nread,buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],buf[8],buf[9]);

                nCounts ++; //时间计数器加一

                //转换成时间格式
                if(0x59 == buf[0] && 0x00 ==buf[8])
                {
                    unsigned int nYear, nMonth, nDay, nHour, nMinute, nSecond;

                    nYear = ( (unsigned char)buf[1] * 256 ) + ((unsigned char)buf[2]);
                    nMonth = (unsigned char)buf[3];
                    nDay = (unsigned char)buf[4];
                    nHour = (unsigned char)buf[5];
                    nMinute = (unsigned char)buf[6];
                    nSecond = (unsigned char)buf[7];

                    std::string strTime;
                    char szBuff[256] = {0};
                    sprintf(szBuff, "%4d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond);

                    strTime = string(szBuff);

                    //printf("===nCounts=%d==TimeFromGps: %s===\n", nCounts, strTime.c_str());
					//LogNormal("strTime=%s",strTime.c_str());

                    //容错性判断
                    if(nYear < 1 || nYear < 2010 || nYear > 2050 ||
                       nMonth < 1 || nMonth > 12 ||
                       nDay < 1  || nDay > 31 ||
                       nHour < 0 || nHour > 24 ||
                       nMinute < 0 || nMinute > 59 ||
                       nSecond < 0 || nSecond > 59)
                    {
                        bGetTime = false ;
                    }
                    else
                    {
                        bGetTime = true;

                    }

                    if(strTime.size() > 0 && bGetTime) //取到正确格式的时间，精确到秒
                    {
                        if(nCounts >= 1200 || nCounts == 1) //重设时间
                        {
							UINT32 timeSec = MakeTime(strTime);
							timeval timer;
							timer.tv_sec = timeSec;
							timer.tv_usec = 0;
							if (settimeofday(&timer, NULL) == 0)
							{
								LogNormal("通过GPS校时成功%s",strTime.c_str());
								system("hwclock --systohc");
							}

                            nCounts = 2; //归零
                        } //End of if(nCounts >= 3600)
                    }
					
                } //End of if(0x59 == buf[0] && 0x00 ==buf[8])
				else
				{
					if(0x4e == buf[0] && 0x4e ==buf[9])
					{
						LogNormal("GPS无法定位");
					}
				}

                memset(buf, 0, 256);
                //sleep(5);
            } //End of if(nread > 0)
		}

        usleep(100*1000); //收完一条串码等500ms

	}
}
