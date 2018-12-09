
#include "Common.h"
#include "RadarSerial.h"

CRadarSerial g_RadarSerial;

//记录接收线程
void* ThreadRadarSerialCmd(void *pArg)
{
	if(g_RadarSerial.m_nRadarType == 1)
	{
		g_RadarSerial.RecData();
	}
	else if(g_RadarSerial.m_nRadarType == 2)
	{
		g_RadarSerial.RecDataHuichang();
	}
	else
	{
		g_RadarSerial.RecData();
	}	
}

CRadarSerial::CRadarSerial()
{
     fd_com=-1;
     pthread_mutex_init(&m_speedmutex,NULL);

	 m_nRadarType = 1;
}

CRadarSerial::~CRadarSerial()
{
    pthread_mutex_destroy(&m_speedmutex);
}

bool CRadarSerial::OpenDev()
{
    //打开相机控制串口
    printf("CamerSerial::OpenDev g_RadarComSetting.nComPort=%d,g_RadarComSetting.nBaud=%d\n",\
				g_RadarComSetting.nComPort,g_RadarComSetting.nBaud);

	fd_com = open_port(g_RadarComSetting.nComPort, g_RadarComSetting.nBaud, g_RadarComSetting.nDataBits, g_RadarComSetting.nStopBits, g_RadarComSetting.nParity);

	if(fd_com == -1)
	{
	    return false;
	}
	else
	{
	    return true;
	}
}

//开启串口控制命令发送线程
void CRadarSerial::BeginRadarSignalCapture()
{
    printf("===========CRadarSerial::BeginRadarSignalCapture()=========\n");

    OpenDev();
    //创建读串口线程
    m_bEndRadarCapture = false;

	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//启动记录发送线程

	int nret=pthread_create(&id,&attr,ThreadRadarSerialCmd,NULL);
	if(nret!=0) //失败
	{
		printf("启动雷达串口接收线程失败!\r\n");
	}
	else
	{
		printf("启动雷达串口接收线程成功!\r\n");
	}
	pthread_attr_destroy(&attr);
}

//结束线圈采集线程
bool CRadarSerial::EndRadarSignalCapture()
{
    m_bEndRadarCapture = true;

	Close();
}

/*
    获得速度信号函数

*/
int CRadarSerial::GetSpeedSignal(Speed_Signal& uSpeed)
{
    //读取车速
    pthread_mutex_lock(&m_speedmutex);
    /*if(m_listRadar.size() > 0)
    {
        listSpeedSignal::iterator it = m_listRadar.begin();
        uSpeed = *it;
        m_sRadar = uSpeed;
        m_listRadar.pop_front();
    }
    else*/
    {
        uSpeed = m_sRadar;
    }

    pthread_mutex_unlock(&m_speedmutex);

    return 1;
}

//接收雷达串口消息
void CRadarSerial::RecData()
{
    unsigned char buf[SRIP_MAX_BUFFER] = {0};
    //unsigned char tmp;

    int nReadBytes = 0;
    int nFreeBuffer = 0;
    int nSusHead = 0;
    int nSusDataLen = 0;

    //bool bGetData = false; //是否获得了雷达消息
    int nTargetSpeed = 0; //目标车速（LSB）	16位数字最强目标速度
    int nFastSpeed = 0; //最快车速（LSB）	16位数字最快目标速度
    int nLockSpeed = 0; //锁定车速（LSB）	16位数字（最强或最快）锁定速度
    int nPatrolSpeed = 0;//巡逻车速（LSB）	16位数字巡逻车速度-只在移动模式

    int64_t nSystemTime;
    struct timeval tv;

    int nTotalBytes = 0;

	while(!m_bEndRadarCapture)
	{
	    gettimeofday(&tv,NULL);
        nSystemTime = tv.tv_sec*1000000+tv.tv_usec;//当前系统时间

		//printf("======ThreadRadarSerialCmd=====Receiving....\n");
		//fcntl(fd_com, F_SETFL, FNDELAY);

		//int nread = read(fd_com, buf, 21);
		//读串口
		nReadBytes = read(fd_com, buf + nFreeBuffer, SRIP_MAX_BUFFER - nFreeBuffer);
		if(nReadBytes != -1)
		{
		    //printf("======ThreadRadarSerialCmd=====Receiving....\n");

		    if((nSusDataLen + nReadBytes) % 21 != 0)
		    {
		        nReadBytes += read(fd_com, buf + nFreeBuffer + nReadBytes, SRIP_MAX_BUFFER - nFreeBuffer - nReadBytes);
		    }
		    nTotalBytes += nReadBytes;


			//printf("buf=%s,nReadBytes=%d\n", buf, nReadBytes);

            if(nReadBytes > 0)
            {
                /*
                for(int i=0; i<21; i++)
                {
                    printf("=buf[%d]=0X%x =====\n", i, buf[i]);
                }
                */

                /* for test
                buf[0] = 0x59;
                buf[1] = 0x07;
                buf[2] = 0xDA;
                buf[3] = 0x05;
                buf[4] = 0x07;
                buf[5] = 0x10;
                buf[6] = 0x1B;
                buf[7] = 0x2D;
                buf[8] = 0x00;
                buf[9] = 0xA0;
                */

                nSusDataLen += nReadBytes;

                while(nSusDataLen >= 21)
                {
                    if(0xEF == buf[nSusHead] && 0xFF == buf[nSusHead + 1] &&
                       0x02 == buf[nSusHead + 2] && 0x01 == buf[nSusHead + 3] &&
                       0x0D == buf[nSusHead + 4] && 0x00 == buf[nSusHead + 5] &&
                       0x00 == buf[nSusHead + 6] && 0x01 == buf[nSusHead + 7])
                    {
                        nTargetSpeed = (unsigned char)buf[nSusHead + 8] ;
                        nFastSpeed   = (unsigned char)buf[nSusHead + 10] ;
                        nLockSpeed   = (unsigned char)buf[nSusHead + 12] ;
                        nPatrolSpeed = (unsigned char)buf[nSusHead + 14] ;

                        /*
                        printf("=======nTargetSpeed=%d, nFastSpeed = %d, nLockSpeed = %d, nPatrolSpeed = %d===\n",
                               nTargetSpeed, nFastSpeed, nLockSpeed, nPatrolSpeed);
                        */

                        //if(nTargetSpeed > 0)
                        {
                            /*
                            for(int i=0; i<21; i++)
                            {
                                printf("====buf[%d + %d]=0X %x =====\n", nSusHead, i, buf[nSusHead + i]);
                            }
                            */


                            Speed_Signal sRadar;
                            sRadar.SpeedSignal=nTargetSpeed;//雷达状态
                            sRadar.FastSpeed = nFastSpeed;
                            sRadar.SystemTime = nSystemTime;//系统时间
                            pthread_mutex_lock(&m_speedmutex);

                            /*if(m_listRadar.size()>10)//最多存10个
                            {
                                m_listRadar.pop_front();
                            }
                            m_listRadar.push_back(sRadar);*/

                            m_sRadar = sRadar;
                            pthread_mutex_unlock(&m_speedmutex);
                        }



                        nSusDataLen -= 21;
                        nSusHead    += 21;
                    }
                    else
                    {
                        nSusHead    += 1;
                        nSusDataLen -= 1;
                    }
                } //end of while(nSusDataLen >= 21)

            } //End of if(nReadBytes > 0)
		}

		int nNewBufferStart = nFreeBuffer + nReadBytes;
		if (nNewBufferStart == SRIP_MAX_BUFFER)
		{
		    int i, j;
			for (i=nSusHead, j=0; i<nSusHead + nSusDataLen; i++, j++)
			{
				buf[j] = buf[i];
			}

			nFreeBuffer = nSusDataLen;
			nSusHead    = 0;
		}
		else if (nNewBufferStart > SRIP_MAX_BUFFER)
		{
			printf("Error\n");
			throw 0;
		}
		else
		{
			nFreeBuffer = nNewBufferStart;
		}

        usleep(10);
        //usleep(46*1000); //收完一条串码等30ms

	}//End of While

	memset(buf, 0, SRIP_MAX_BUFFER);
}

//接收慧昌雷达串口消息
void CRadarSerial::RecDataHuichang()
{
	unsigned char buf[SRIP_MAX_BUFFER] = {0};
    int nReadBytes = 0;
    int nFreeBuffer = 0;
    int nSusHead = 0;
	int nSusDataLen = 0;

    int nTargetSpeed = 0; //目标车速（LSB）	16位数字最强目标速度

    int64_t nSystemTime;
    struct timeval tv;

    int nTotalBytes = 0;
	long long iCnt = 0;

	while(!m_bEndRadarCapture)
	{
	    gettimeofday(&tv,NULL);
        nSystemTime = tv.tv_sec*1000000+tv.tv_usec;//当前系统时间

		//读串口
		nReadBytes = read(fd_com, buf + nFreeBuffer, SRIP_MAX_BUFFER - nFreeBuffer);
		if(nReadBytes != -1)
		{
            if(nReadBytes > 0)
            { 
                nSusDataLen += nReadBytes;
				printf("------------nSusDataLen:%d ,nReadBytes:%d \n", nSusDataLen, nReadBytes);

				while(nSusDataLen > 0)
				{
					nTargetSpeed = (unsigned int)(buf[nSusHead]);

					Speed_Signal sRadar;
					sRadar.SpeedSignal = nTargetSpeed;//雷达状态
					sRadar.FastSpeed = nTargetSpeed;
					sRadar.SystemTime = nSystemTime;//系统时间
					pthread_mutex_lock(&m_speedmutex);              

					m_sRadar = sRadar;
					pthread_mutex_unlock(&m_speedmutex);

					//LogTrace("Radar.txt", "[%d], %d, 0x:%x  bGetSpeed:%d uSpeed:%d", \
						nSusDataLen - 1, buf[nSusDataLen - 1], buf[nSusDataLen - 1], bGetSpeed, uSpeed);

					//LogNormal("-nSusHead:%d,nSusDataLen:%d,nTargetSpeed:%d", nSusHead, nSusDataLen, nTargetSpeed);
					//LogNormal("[%d], %d, 0x:%x  uSpeed:%d nSusDataLen:%d",\
						nSusHead, buf[nSusHead], buf[nSusHead], nTargetSpeed, nSusDataLen);

					nSusHead    += 1;
					nSusDataLen -= 1;
				}			
            } //End of if(nReadBytes > 0)
		}

		int nNewBufferStart = nFreeBuffer + nReadBytes;
		if (nNewBufferStart == SRIP_MAX_BUFFER)
		{
		    int i, j;
			for (i=nSusHead, j=0; i<nSusHead + nSusDataLen; i++, j++)
			{
				buf[j] = buf[i];
			}

			nFreeBuffer = nSusDataLen;
			nSusHead    = 0;
		}
		else if (nNewBufferStart > SRIP_MAX_BUFFER)
		{
			printf("Error\n");
			throw 0;
		}
		else
		{
			nFreeBuffer = nNewBufferStart;
		}       
		
		/*
		iCnt++;
		if(iCnt%600000 == 0)
		{
			iCnt = 0;
			LogNormal("---recving...Radardata ...%d \n", nSusHead);			
		}
		*/
		usleep(10);
	}//End of While

	memset(buf, 0, SRIP_MAX_BUFFER);
}
