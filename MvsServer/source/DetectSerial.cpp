/********************************************************************








*********************************************************************/

#include <fstream>
#include "DetectSerial.h"
#include "Common.h"
#include "RoadChannelCenter.h"
//#define DEBUG 1
#ifdef DEBUG
#define PRINT(str) printf(str)
#define PRINT2(str,arg) printf(str,arg)
#else
#define PRINT(str)
#define PRINT2(str,arg)
#endif

//#define RECORD_LOOP_SERIAL

static void BinaryPrint(int n)
{
    int a;
    a=n%2;
    n=n>>1;
    if(n==0)
    ;
    else
    BinaryPrint(n);
    printf("%d",a);
}

CDetectSerial d_SerialComm;
void *revspeed(void *arg)
{
   CDetectSerial *pDetectSerial= (CDetectSerial*)arg;
    pDetectSerial->CapureSpeedSignal();
}

CDetectSerial::CDetectSerial()
{
    //ctor
    fd_com = -1;
	pthread_mutex_init(&m_speedmutex,NULL);
}

CDetectSerial::~CDetectSerial()
{
    //dtor
     pthread_mutex_destroy(&m_speedmutex);
}

//启动线圈采集线程
bool CDetectSerial::BeginLoopSignalCapture()
{
    OpenDev();
    //创建读串口线程
    m_bEndDlCapture = false;
    pthread_attr_t   attr;
    //线程ID
    pthread_t id;
	//初始化
	pthread_attr_init(&attr);
	struct   sched_param   param;
	pthread_attr_getschedparam(&attr,   &param);
	param.sched_priority   =   20;
	pthread_attr_setschedparam(&attr,   &param);

	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    int nret=pthread_create(&id,&attr,revspeed,this);
    //成功
	if(nret!=0)
	{
		//失败
		LogError("创建线圈采集线程失败,无法读取线圈状态!\r\n");
		return false;
	}
    pthread_attr_destroy(&attr);
}

//结束线圈采集线程
bool CDetectSerial::EndLoopSignalCapture()
{
    m_bEndDlCapture = true;

	Close();
}

bool CDetectSerial::OpenDev()
{
    //fd_com=open_port(3,57600);
    fd_com=open_port(g_DHComSetting.nComPort,g_DHComSetting.nBaud,g_DHComSetting.nDataBits,g_DHComSetting.nStopBits,g_DHComSetting.nParity,false);
    printf("fd_com=%d+++g_DHComSetting.nComPort=%d,g_DHComSetting.nBaud=%d\n",fd_com,g_DHComSetting.nComPort,g_DHComSetting.nBaud);

    return true;
}

/*
    获得速度信号函数

*/
int CDetectSerial::GetSpeedSignal(Speed_Signal& uSpeed)
{
    //读取车速
    pthread_mutex_lock(&m_speedmutex);
    if(m_listLoop.size()>0)
    {
        listSpeedSignal::iterator it = m_listLoop.begin();
        uSpeed = *it;
        m_sLoop = uSpeed;
        m_listLoop.pop_front();
    }
    else
    {
        uSpeed = m_sLoop;
    }
    pthread_mutex_unlock(&m_speedmutex);
						
    return 1;
}
/*
采集车辆速度
*/
void CDetectSerial::CapureSpeedSignal()
{
    unsigned char  buf[SRIP_MAX_BUFFER];
    unsigned char  tmp;
    int            nReadBytes=0;
	int            nFreeBuffer = 0;

	int            nSusHead = 0;
	int            nSusDataLen = 0;

    //unsigned short signal =0;
    //unsigned int   lastTime=0;
    unsigned int   uCounter = 0;
	unsigned int   uLastCounter = 0;

	int64_t        nOutputCounter = 0;
    //unsigned int   TimeGap = 0;
    //int64_t        nTime =0;


    //循环控制变量
    int i = 0;
    int j = 0;
    //int64_t jj =0;
    //unsigned int tm = 0;

    //周期数
    //static unsigned int k = 0;
    int64_t nSystemTime;
    struct timeval tv;

	char cbuf[100];



	#ifdef RECORD_LOOP_SERIAL
		ofstream ofs("./LoopSerial.txt", ios::app);
	#endif

	int nTotalBytes = 0;


    while(!m_bEndDlCapture)
	//while(true)
    {
        gettimeofday(&tv,NULL);
        nSystemTime = tv.tv_sec*1000000+tv.tv_usec;//当前系统时间


        //读串口
        nReadBytes = read(fd_com, buf + nFreeBuffer, SRIP_MAX_BUFFER - nFreeBuffer);



		if ((nSusDataLen + nReadBytes) % 6 != 0 )
		{
			nReadBytes += read(fd_com, buf + nFreeBuffer + nReadBytes, SRIP_MAX_BUFFER - nFreeBuffer - nReadBytes);
		}

		nTotalBytes += nReadBytes;


		//nReadBytes = read(fd_com, buf, SRIP_MAX_BUFFER);
		#ifdef RECORD_LOOP_SERIAL

			if (nReadBytes > 0)
			{
				ofs << "nTotalBytes = " << nTotalBytes << ", nReadBytes = " << nReadBytes << ": ";
			}

			for (i=nFreeBuffer; i<nFreeBuffer + nReadBytes; i++)
			{
				sprintf(cbuf, "%02X", buf[i]);
				ofs << cbuf << " ";
				//printf("%d ", cbuf[i]);
			}

			//for (i=0; i<nReadBytes; i++)
			//{
			//	ofs << (int)(buf[i]) << " ";
			//	//printf("%d ", cbuf[i]);
			//}

			if (nReadBytes > 0)
			{
				ofs << endl;
				//printf("\n");
			}

		#endif


		nSusDataLen += nReadBytes;

		while (nSusDataLen >= 6)
		{
			if ((buf[nSusHead] & 0x0F) == 0x0A &&
				(buf[nSusHead] ^ buf[nSusHead+1] ^ buf[nSusHead+2] ^ buf[nSusHead+3] ^ buf[nSusHead+4]) == buf[nSusHead+5] )
			{
				// 获取信号值
				unsigned short tmptmp = (buf[nSusHead] & 0xF0);
				unsigned short uSignal = buf[nSusHead+1];
				uSignal = (uSignal << 4) + (tmptmp >> 4);


                uCounter = 0;

				//获取Counter值
                for(j = 2;j < 5;j++)
                {
                    tmp=buf[nSusHead + j];
                    for(i=0;i<8;i++)
                    {
                        if(tmp & (1<<i))
                        uCounter |= (1<<(i+(j-2)*8));
                    }
                }

				if (uLastCounter > uCounter)
				{
					nOutputCounter += (uCounter + 16777216  - uLastCounter);
				}
				else
				{
					nOutputCounter += (uCounter - uLastCounter);
				}


				uLastCounter = uCounter;

                Speed_Signal sLoop;
                sLoop.SpeedSignal=uSignal;//各个线圈状态
                sLoop.SpeedTime=nOutputCounter;//线圈检测器时间计数
                sLoop.SystemTime = nSystemTime;//系统时间
				
				/*把线圈信号送给触发方式的通道*/
					pthread_mutex_lock(&m_speedmutex);
					if(m_listLoop.size()>10)//最多存10个
					{
						m_listLoop.pop_front();
					}
					m_listLoop.push_back(sLoop);
					pthread_mutex_unlock(&m_speedmutex);

				#ifdef RECORD_LOOP_SERIAL
					//sprintf(cbuf, "AS: %02d, Counter = %lld, ts=%lld, uSignal=%d", buf[nSusHead], nOutputCounter, nSystemTime, uSignal);

					unsigned short s0 = ((uSignal & 0x0003) >> 0);
					unsigned short s1 = ((uSignal & 0x000C) >> 2);
					unsigned short s2 = ((uSignal & 0x0030) >> 4);
					unsigned short s3 = ((uSignal & 0x00C0) >> 6);
					unsigned short s4 = ((uSignal & 0x0300) >> 8);
					unsigned short s5 = ((uSignal & 0x0C00) >> 10);


					sprintf(cbuf, "Counter = %020lld, ts=%lld, signal = %3X%3X%3X%3X%3X%3X  ", nOutputCounter, nSystemTime, s5, s4, s3, s2, s1, s0);
					ofs << cbuf << " ";





					struct tm *p;
					time_t t = nSystemTime/1000000;
					p = localtime(&t);
					sprintf(cbuf, "%d_%d_%d %02d:%02d:%02d",
						p->tm_year + 1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
					ofs << cbuf << endl;



					//printf("AS: %02d Counter = %lld\n", buf[nSusHead], nOutputCounter);
				#endif


				//printf("\nSL: ");
				//BinaryPrint(buf[nSusHead+1]);


				nSusDataLen -= 6;
				nSusHead    += 6;
				//break;

			}
			else
			{
				nSusHead    += 1;
				nSusDataLen -= 1;
			}
		}


		int nNewBufferStart = nFreeBuffer + nReadBytes;
		if (nNewBufferStart == SRIP_MAX_BUFFER)
		{
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
			LogNormal("线圈采集错误\n");
			exit(-1);
		}
		else
		{
			nFreeBuffer = nNewBufferStart;
		}

		usleep(1);
    }

    //变量定义





	#ifdef RECORD_LOOP_SERIAL
		ofs.close();
	#endif
}
