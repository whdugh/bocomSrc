//#include "RedLightSerial.h"
#include "Common.h"
#include "CommonHeader.h"

RedLightSerial r_SerialComm;
void *revred(void *arg)
{
   RedLightSerial *p=(RedLightSerial*)arg;

   switch(g_RedLightComSetting.nComUse)
   {
   case 2:
	   p->CapureRedSignal();
	   break;
   case 14:
	   p->CapureRedSignal1();
	   break;
   }
}
//构造函数
RedLightSerial::RedLightSerial()
{
    fd_com = -1;
    //检测红灯线程初始化
    m_uTrafficSignal=0;

	pthread_mutex_init(&m_redmutex,NULL);
}
//析构函数
RedLightSerial::~RedLightSerial()
{
    pthread_mutex_destroy(&m_redmutex);
}

//启动红灯线程
bool RedLightSerial::BeginRedSignalCapture()
{
    OpenDev();
    //创建读串口线程
    m_bEndTsCapture = false;
    pthread_attr_t   attr;
    //线程ID
    pthread_t m_red;
	//初始化
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    int nret=pthread_create(&m_red,&attr,revred,this);
    //成功
	if(nret!=0)
	{
		//失败
		LogError("创建读红灯串口线程失败,无法读取红灯状态!\r\n");
		return false;
	}
    pthread_attr_destroy(&attr);
}

//结束红灯线程
bool RedLightSerial::EndRedSignalCapture()
{
    m_bEndTsCapture = true;

	Close();
}

//打开红灯串口
bool RedLightSerial::OpenDev()
{
    printf("OpenDev g_RedLightComSetting.nComPort=%d,g_RedLightComSetting.nBaud=%d\n",g_RedLightComSetting.nComPort,g_RedLightComSetting.nBaud);

    fd_com=open_port(g_RedLightComSetting.nComPort,g_RedLightComSetting.nBaud);

    return true;
}

/*
    获得红灯信号函数
    directions：     方向参数 N-向北，S-向南，W-向西，E-向东
    redsig：         红灯信号数组
*/
int  RedLightSerial::GetRedSignal(unsigned short& uTrafficSignal)
{
    pthread_mutex_lock(&m_redmutex);
    uTrafficSignal=m_uTrafficSignal;
    pthread_mutex_unlock(&m_redmutex);
    return uTrafficSignal;
}
//捕获红灯信号
void RedLightSerial::CapureRedSignal()
{
    int i=0;
    int nread;
    unsigned char buff[3];
    unsigned short redsig=0;
    unsigned short presignal=0;


    while(!m_bEndTsCapture)
    {
        nread=read(fd_com,buff,3);//读串口
        if(nread > 0)
        {
            unsigned char sumsig = 0;
            /*判断是否获得到正确信号*/
            if(buff[0]==0xAA)
            {
                sumsig = buff[0]+buff[1];
                if(sumsig == buff[2])//累加和校验
                {
                    char light =buff[1];
                    redsig=0;
                    for(i=0;i<8;i++)
                    {
                        if(light &(1<< i))
                        {
                            redsig |=1<<i;
                        }
                    }
                    printf("redsig=%d,time=%d\n",redsig,time(NULL));

                    //获取信号与前一个信号不一致，添加到list
                    //if((presignal[0]!=redsig[0])||(presignal[1]!=redsig[1])||(presignal[2]!=redsig[2]))
                    if(redsig!=presignal)
                    {
                        presignal=redsig;
                        pthread_mutex_lock(&m_redmutex);
                        m_uTrafficSignal = redsig;
                        pthread_mutex_unlock(&m_redmutex);
                    }
                }
            }
        }
        usleep(1000*1);
    }
}

//捕获红灯信号 TGC-HD-04J红灯信号转发器
void RedLightSerial::CapureRedSignal1()
{
	int nread;
	unsigned short redsig=0;
	bool bReadFinish = false;
	while(!m_bEndTsCapture)
	{
		bReadFinish = false;
		char buff[10] = {0};
		int nIndex = 0;
		char ch;
		while(1)
		{
			bReadFinish = false;

			int r_size = read(fd_com, &ch, 1);
			if (r_size == 0)
			{
				continue;
			}
			if (ch == 'Z')
			{
				memset(buff,0x00,10);
				nIndex = 0;

				buff[nIndex++] = ch;

				r_size = read(fd_com, &ch, 1);
				if (r_size == 0)
				{
					continue;
				}
				if (ch == 'F')
				{
					buff[nIndex++] = ch;
					
					int nCnt = 0;
					while(1)
					{
						r_size = read(fd_com, &ch, 1);
						if (r_size == 0)
						{
							continue;
						}
						nCnt++;
						buff[nIndex++] = ch;
						if (nCnt == 7)
						{
							bReadFinish = true;
							break;
						}
					}
				}
			}

			if (bReadFinish == true)
			{
				break;
			}
		}

		//LogNormal("CapureRedSignal1 :%s\n",buff);
		if (bReadFinish == true)
		{
			char chNo[3] = {0};
			memcpy(chNo,&buff[3],1);

			int nNo = atoi(chNo);
			char chSig[4] = {0};
			memcpy(chSig,&buff[4],3);

			if (strcmp(chSig,"RED") == 0)
			{
				switch(nNo)
				{
				case 0:
					redsig |=1;
					break;
				case 1:
					redsig |=2;
					break;
				case 2:
					redsig |=4;
					break;
				case 3:
					redsig |=8;
					break;
				}
			}
			else if (strcmp(chSig,"SHT") == 0)
			{
				switch(nNo)
				{
				case 0:
					redsig &=14;
					break;
				case 1:
					redsig &=13;
					break;
				case 2:
					redsig &=11;
					break;
				case 3:
					redsig &=7;
					break;
				}
			}
			//LogNormal("CapureRedSignal1 :%d\n",redsig);
			pthread_mutex_lock(&m_redmutex);
			m_uTrafficSignal = redsig;
			pthread_mutex_unlock(&m_redmutex);
		}
	}
}
