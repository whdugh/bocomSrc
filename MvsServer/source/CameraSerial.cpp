//#include "CameraSerial.h"
//#include "KeyBoardCodeSerial.h"

#include "Common.h"
#include "CommonHeader.h"

/* X16+X12+X5+1 余式表*/
const unsigned short  g_CrcTab[256]={ 
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};








CamerSerial g_CameraSerialComm;
CamerSerial::CamerSerial()
{
     fd_com=-1;
	 m_strControlIP = "";
}
CamerSerial::~CamerSerial()
{

}
bool CamerSerial::OpenDev()
{
    //打开相机控制串口
    printf("CamerSerial::OpenDev g_CameraComSetting.nComPort=%d,g_CameraComSetting.nBaud=%d\n",g_CameraComSetting.nComPort,g_CameraComSetting.nBaud);

	fd_com = open_port(g_CameraComSetting.nComPort,g_CameraComSetting.nBaud,g_CameraComSetting.nDataBits,g_CameraComSetting.nStopBits,g_CameraComSetting.nParity,true);

	if(fd_com == -1)
	{
	    return false;
	}
	else
	{
	    return true;
	}
}

bool CamerSerial::GetMessage(CAMERA_CONFIG& cfg)
{
	if(fd_com!=-1)
	{
	    tcflush(fd_com,TCIFLUSH);

		char protoclCmd[32]={0};
		int nwrite = -1;
		std::string strCmd;
		int nread = -1;

		bool bBlockCmd = false;

        memset(protoclCmd,0,32);

        if(!bBlockCmd) //控制相机
        {
            memset(protoclCmd,0,32);
            sprintf(protoclCmd,"%s?\r\n",cfg.chCmd);
        }

		strCmd = protoclCmd;
		nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

		printf(" CSerialComm::GetMessage,strCmd=%s,cfg.uKind=%d\n",strCmd.c_str(),cfg.uKind);
		//usleep(1000*5);

		if(nwrite>=strCmd.size())
		{
                memset(protoclCmd,0,32);
                memset(cfg.chCmd,0,sizeof(cfg.chCmd));

				int nCount = 0;
                int nread = 0;
                memset(protoclCmd,0,32);

                while(true)
                {
                    nread = read(fd_com,protoclCmd,32);

                    if(nread > 0)
                    {
                     // LogError("读串口成功nCount=%d!\r\n",nCount);
                      break;
                    }
                    else
                    {
                        if(nCount >= 30)
                        {
                           // LogError("读串口超时!\r\n");
                            break;
                        }
                    }
                    usleep(1000*5);
                    nCount++;
                }

				if(nread > 0)
				{
				    {
				        memcpy(cfg.chCmd,protoclCmd,32);
				        printf("protoclCmd=%s\n",protoclCmd);
				    }
                    printf("nread=%d,cfg.chCmd=%s\n",nread,cfg.chCmd);
				}
		}
		return true;
	}
	return false;
}

bool CamerSerial::SendMessage(CAMERA_CONFIG cfg,CAMERA_CONFIG& cam_cfg,CAMERA_MESSAGE kind)
{
	if(fd_com!=-1)
	{
	    tcflush(fd_com,TCIFLUSH);

	    char protoclCmd[32]={0};
		int nwrite = -1;
		std::string strCmd;


	    bool bBlockCmd = false;
        printf("CSerialComm::SendMessage,cfg.chCmd=%s,cfg.uKind=%d,bBlockCmd=%d\n",cfg.chCmd,cfg.uKind,bBlockCmd);


		if(kind==CAMERA_PE)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"PE=%d\r\n",cfg.uPE);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.uPE=cfg.uPE;
				printf("发送串口控制信息成功！\r\n");
			}
		}

		else if(kind== CAMERA_POL)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"EPL=%d\r\n",cfg.uPol);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.uPol=cfg.uPol;
				printf("发送串口控制信息成功！\r\n");
			}
		}

		else if(kind==CAMERA_EEN)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"PEN=%d\r\n",cfg.EEN_on);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.EEN_on=cfg.EEN_on;
				printf("发送串口控制信息成功！\r\n");
			}
		}

		else if(kind==CAMERA_GAIN)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"GA=%d\r\n",(int)cfg.uGain);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.uGain=cfg.uGain;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_AGC)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"AGC=%d\r\n",cfg.AGC);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.AGC=cfg.AGC;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_ASC)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"ASC=%d\r\n",cfg.ASC);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,nwrite=%d,strCmd=%s",nwrite,strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.ASC=cfg.ASC;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_SH)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"SH=%d\r\n",(int)cfg.uSH);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.uSH=cfg.uSH;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_SM)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"SM=%d\r\n",cfg.uSM);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.uSM=cfg.uSM;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_PEI)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"PEI=%d\r\n",cfg.EEN_delay);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.EEN_delay=cfg.EEN_delay;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_PEW)
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"PEW=%d\r\n",cfg.EEN_width);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.EEN_width=cfg.EEN_width;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_MODE)//触发方式
		{
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"TR=%d\r\n",cfg.nMode);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.nMode=cfg.nMode;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_IRIS)//光圈控制
		{
            memset(protoclCmd,0,32);
			sprintf(protoclCmd,"LIS=%d\r\n",cfg.nIris);
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				cam_cfg.nIris=cfg.nIris;
				printf("发送串口控制信息成功！\r\n");
			}
		}
		else if(kind==CAMERA_CMD)
		{
			if( bBlockCmd)//设置裁剪卡
			{
			    char block_cmd[6] = {0};
			    unsigned short nValue = (unsigned short)cfg.fValue;
			    //取高8位
			    BYTE byteHigh = (nValue>>8)&0xff;
			     //取低8位
			    BYTE byteLow = nValue&0xff;

               //printf("===(int)cfg.fValue=%d=%x,%x\r\n",(int)cfg.fValue,byteHigh,byteLow);

               sprintf(block_cmd,"%c%c%c%c\r\n",cfg.chCmd[0],cfg.chCmd[1],byteHigh,byteLow);
               nwrite = write(fd_com,block_cmd,6);

               printf(" CSerialComm::SendMessage,nwrite=%d,block_cmd=%c%c%x%x\n",nwrite,block_cmd[0],block_cmd[1],(BYTE)block_cmd[2],(BYTE)block_cmd[3]);
                if(nwrite < 6)
                {
                    LogError("发送串口控制信息失败！\r\n");
                }
                else
                {
                    printf("发送串口控制信息成功！\r\n");
                }
			}
			else//设置相机
			{
                    memset(protoclCmd,0,32);
                    sprintf(protoclCmd,"%s=%d\r\n",cfg.chCmd,(int)cfg.fValue);
                    strCmd = protoclCmd;
                    nwrite = write(fd_com,strCmd.c_str(),strCmd.size());
                    printf(" CSerialComm::SendMessage,strCmd=%s,strCmd.size()=%d\n",strCmd.c_str(),strCmd.size());
                    if(nwrite < strCmd.size())
                    {
                        LogError("发送串口控制信息失败！\r\n");
                    }
                    else
                    {
                        printf("发送串口控制信息成功！\r\n");
                    }
			}
		}
		else if(kind==CAMERA_SA)
		{
			//保存设置
			memset(protoclCmd,0,32);
			sprintf(protoclCmd,"SA=1\r\n");
			strCmd = protoclCmd;
			nwrite = write(fd_com,strCmd.c_str(),strCmd.size());

			printf(" CSerialComm::SendMessage,strCmd=%s\n",strCmd.c_str());
			if(nwrite < strCmd.size())
			{
				LogError("发送串口控制信息失败！\r\n");
			}
			else
			{
				printf("发送串口控制信息成功！\r\n");
			}
		}

        //读回回码
        if(nwrite > 0)
        {
            int nCount = 0;
            int nread = 0;
            int nReadCount = 0;

            while(true)
            {
                memset(protoclCmd,0,32);
                nread = read(fd_com,protoclCmd,32);

                if(nread > 0)
                {
                    nReadCount += nread;
                    printf("nread=%d,protoclCmd=%s\n",nread,protoclCmd);

                    if(nReadCount >= 10)
                    {
                        break;
                    }
                }

                if(nCount >= 30)
                {
                    //LogError("读串口超时!\r\n");
                    break;
                }
                usleep(1000*5);
                nCount++;
            }
        }
		return true;
	}
	else
		return false;
}

//通过串口设置相机参数（柯达相机用）method:操作方法，是要设置还是获取等等，0表示设置，1表示获取
//返回-1表示串口没有打开成功
//返回-2表示读取串口超时或异常
//返回-100表示成功（非负数不能用）
//method=2表示设置CAMERA_PE为手动模式，method=3表示设置CAMERA_PE为编程模式
//method=4、5表示给相机升级，这里只被UpdateCam调用(5没有写完)
//method=100,表示永久保存相机参数。
//nCamId相机Id，在连接多个相机时，区分相机用。
int CamerSerial::SendMessageByKodak(CAMERA_CONFIG cfg,CAMERA_CONFIG& cam_cfg,CAMERA_MESSAGE kind,int method,int nCamId)
{
	if(fd_com < 0)
	{
		return -1;
	}

	unsigned char ProtoclCmd[256]={0};
	static short index = 0x8000;
	string strKind;
	char chIndex[3] = {0};
	index++;//计数加1
	int nPacketSize;
	int nChecksum = 0;
	ProtoclCmd[0] = 0xFF;
	ProtoclCmd[1] = 0x5A;
	
	if(method == 1)
	{
		//标志位,获取
		ProtoclCmd[3] = 0x20;
		nPacketSize = 14 + 2;
	}
	
	else if((method == 0) || (method == 2) || (method == 3))
	{
		//标志位,设置
		ProtoclCmd[3] = 0x21;
		nPacketSize = 14 + 2 + 2;
	}
	else if((method == 4) || (method == 100))
	{
		//标志位,设置
		ProtoclCmd[3] = 0x21;
		nPacketSize = 14 + 2;
	}
	else if(method == 5)
	{
		ProtoclCmd[3] = 0x21;
		nPacketSize = 14 + 8 + 128;//14字节的包头+8字节的数据头+128字节升级包的数据
	}
	else if((method == 6) || (method == 7))
	{
		ProtoclCmd[3] = 0x20;
		nPacketSize = 14 + 2;//14字节的包头
	}
	else if(method == 8)
	{
		ProtoclCmd[3] = 0x21;
		nPacketSize = 14 + 2;//14字节的包头
	}
	else if((method == 9) || (method == 10) || (method == 13) || (method == 14) || (method == 17) || (method == 18))
	{
		ProtoclCmd[3] = 0x21;
		nPacketSize = 14 + 2 +2;//14字节的包头
	}
	else if((method == 11) || (method == 12) || (method == 15) || (method == 16) || (method == 19) || (method == 20))
	{
		ProtoclCmd[3] = 0x20;
		nPacketSize = 14 + 2;//14字节的包头
	}
	ProtoclCmd[2] = 0x00;
	ProtoclCmd[4] = (nCamId & 0xFF00) >> 8;
	ProtoclCmd[5] = nCamId & 0xFF;
	//ProtoclCmd[4] = 0x00;
	//ProtoclCmd[5] = 0x00;
	ProtoclCmd[6] = 0x0F;
	ProtoclCmd[7] = 0x05;
	ProtoclCmd[8] = (index & 0xFF00) >> 8;
	ProtoclCmd[9] = index & 0xFF;
	ProtoclCmd[10] = (nPacketSize & 0xFF00) >> 8;
	ProtoclCmd[11] = nPacketSize & 0xFF;
	ProtoclCmd[12] = 0x00;
	ProtoclCmd[13] = 0x00;
	//	0x2000    0 获取
	//	0x2100    1 设置
	//	0x2200    2 设置返回
	//	0x2300    3 获取返回
	//	0x2400    4
	//strtoul(char*,0,16);
//////////////////////////////////////发送数据说明：	
	////一共2个字节
	//0xFF5A//包头标志
	////一共2个字节，16位
	//00//14、15保留位要置为0
	//01//13是否要校验位，这里设1表示要，设为1了校验位才有效
	//00//11、12保留位要置为0
	//00、01、10、11、100//8、9、10type类型0表示get，0表示get，1表示set，2表示Set-ACK，3表示get-ACK， 4表示alarm
	//00//第7位填0表示不要加密
	//00//第6位保留，填0
	//00//第0-5位插槽号，填0
	////一共2个字节
	//0000//DevID
	////一共2个字节
	//0x0F05//给相机收发命令的command号
	////一共2个字节
	//0x0000//index号最大到0x3fff,到0x3fff后加一位0x0000
	////一共2个字节
	//0x//整个数据包的大小
	////一共2个字节
	////校验和

	int nSendCount = 0;//发送次数
	
	while(nSendCount < 3)
	{
		if(g_bShortConnect)
		{
			//Close();
			OpenDev(m_strControlIP, 3600);	
		}

		if(fd_com!=-1)
		{
			//tcflush(fd_com,TCIFLUSH);

			unsigned char protoclCmd[128]={0};
			int nwrite = -1;
			std::string strCmd;


			bool bBlockCmd = false;


			if(method == 2)
			{
				ProtoclCmd[14] = 0x02;
				ProtoclCmd[15] = 0x10;			
				ProtoclCmd[16] = 0x00;
				ProtoclCmd[17] = 0x00;//0x00:手动模式   0x01:自动模式
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s,nPacketSize=%d\n",ProtoclCmd,nPacketSize);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				/*else
				{
					cam_cfg.uPE=cfg.uPE;
					printf("发送串口控制信息成功！\r\n");
				}*/
			}

			else if(method == 3)
			{
				//先设置成编程模式，这样才能对CAMERA_PE赋值，不然只能对CAMERA_PE设置固定档位的值
				ProtoclCmd[14] = 0x02;
				ProtoclCmd[15] = 0x11;			
				ProtoclCmd[16] = 0x00;
				ProtoclCmd[17] = 0x01;//0x00:预设模式   0x01:编程模式
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				/*else
				{
					cam_cfg.uPE=cfg.uPE;
					printf("发送串口控制信息成功！\r\n");
				}*/
			}

			else if(method == 4)//相机准备升级
			{
				int nTotalNum = 0;
				ProtoclCmd[6] = 0x08;//准备升级相机的命令代码
				ProtoclCmd[7] = 0x01;
				ProtoclCmd[14] = 0x02; //1＝网管程序 2＝FPGA程序
				ProtoclCmd[15] = 0x01; //升级第几个芯片（1～4）
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}

			else if(method == 5)//相机升级
			{
				char *chDate = (char*) new char[150];
				int nPacketSize = 0;//升级包的大小
				int nTotalNum = 0;//升级包总共被分为多少个包（这里每个包为128 Byte，所以nTotalNum=nPacketSize（升级包的大小）/ 128）
				int nPacketIndex = 0;//所发送的包的编号（从0开始计数）
				FILE *fp = NULL;
				GetUpdateFile(fp, nPacketSize ,"/home/road/updatecam");
				nTotalNum = nPacketSize / 128;
				if(nPacketSize % 128)//多余的不满128个字节的部分为最后一个包
				{
					nTotalNum += 1;
				}
				ProtoclCmd[6] = 0x08;//升级相机的命令代码
				ProtoclCmd[7] = 0x03;
				while(nPacketIndex < (nTotalNum))
				{
					ProtoclCmd[8] = (index & 0xFF00) >> 8;
					ProtoclCmd[9] = index & 0xFF;
					ProtoclCmd[12] = 0x00;
					ProtoclCmd[13] = 0x00;//每次发包先把校验位清0
					ProtoclCmd[14] = (nTotalNum & 0xFF000000) >> 24;
					ProtoclCmd[15] = (nTotalNum & 0xFF0000) >> 16;
					ProtoclCmd[16] = (nTotalNum & 0xFF00) >> 8;//14、15、16、17位是总的包数量
					ProtoclCmd[17] = nTotalNum & 0xFF;
					ProtoclCmd[18] = (nPacketIndex & 0xFF000000) >> 24;
					ProtoclCmd[19] = (nPacketIndex & 0xFF0000) >> 16;
					ProtoclCmd[20] = (nPacketIndex & 0xFF00) >> 8;//18、19、20、21位是数据包的编号
					ProtoclCmd[21] = nPacketIndex & 0xFF;
					nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
					ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
					ProtoclCmd[13] = nChecksum & 0xFF;
					nwrite = write(fd_com,ProtoclCmd,nPacketSize);

					printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
					if(nwrite < nPacketSize)
					{
						LogError("发送网口控制信息失败！\r\n");
						if(g_bShortConnect)
						{
							Close();
						}
						return -1;
					}
					nPacketIndex++;//包的编号+1
					index++;//命令的编号+1
				}
				delete []chDate;
			}
		
			else if(method == 6)//获取相机版本
			{
				ProtoclCmd[6] = 0x00;//升级相机的命令代码
				ProtoclCmd[7] = 0x02;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			/*else if(method == 9)//设置相机IP高位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x09;
				ProtoclCmd[16] = (nValue & 0xFF00) >> 8;
				ProtoclCmd[17] = nValue & 0xFF;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,stProtoclCmdrCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 10)//设置相机IP低位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x0a;
				ProtoclCmd[16] = (nValue & 0xFF00) >> 8;
				ProtoclCmd[17] = nValue & 0xFF;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 11)//获取相机IP低高位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x09;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 12)//获取相机IP高位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x0a;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}

			else if(method == 13)//设置相机组播高位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x07;
				ProtoclCmd[16] = (nValue & 0xFF00) >> 8;
				ProtoclCmd[17] = nValue & 0xFF;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 14)//设置相机组播低位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x08;
				ProtoclCmd[16] = (nValue & 0xFF00) >> 8;
				ProtoclCmd[17] = nValue & 0xFF;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 15)//获取相机组播高位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x07;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 16)//获取相机组播低位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x08;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}

			else if(method == 17)//设置相机控制IP高位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x26;
				ProtoclCmd[16] = (nValue & 0xFF00) >> 8;
				ProtoclCmd[17] = nValue & 0xFF;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 18)//设置相机控制IP低位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x27;
				ProtoclCmd[16] = (nValue & 0xFF00) >> 8;
				ProtoclCmd[17] = nValue & 0xFF;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 19)//获取相机控制IP高位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x26;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}
			else if(method == 20)//获取相机控制IP低位
			{
				int nValue = cfg.fValue;
				ProtoclCmd[14] = 0x0a;
				ProtoclCmd[15] = 0x27;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
			}*/


			else if(method == 100)//永久保存相机参数
			{
				int nTotalNum = 0;
				ProtoclCmd[6] = 0x0F;//永久保存相机参数的命令代码
				ProtoclCmd[7] = 0x06;
				ProtoclCmd[14] = 0x00;
				ProtoclCmd[15] = 0x00;
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}

			}

			else if(strncmp(cfg.chCmd,"0x",2) == 0)
			{	
				int nValue = cfg.fValue;
				int nCmd = 0;
				nCmd = strtoul(cfg.chCmd,0,16);//把16进制字符串转换成10进制int
				ProtoclCmd[14] = (nCmd & 0xFF00) >> 8;//14、15位是寄存器地址
				ProtoclCmd[15] = nCmd & 0xFF;
				if(method == 0)
				{				
					ProtoclCmd[16] = (nValue & 0xFF00) >> 8;//16、17位是对寄存器写的值
					ProtoclCmd[17] = nValue & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					printf("发送网口控制信息成功！\r\n");
				}
			}

			else if(kind==CAMERA_FREQUENCY)
			{
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					//cam_cfg.uPE=cfg.uPE;
					printf("发送网口控制信息成功！\r\n");
				}
			}



			else if(kind==CAMERA_PE)
			{
				if(cfg.fSH < 1)//限定cfg.fSH的值的下限值，要保证相机的值最小为一
					cfg.fSH = 1;
				//设置CAMERA_PE
				{
					ProtoclCmd[14] = 0x02;
					ProtoclCmd[15] = 0x13;
					if(method == 0)
					{
						ProtoclCmd[16] = ((int(cfg.fSH)) & 0xFF00) >> 8;//对于PE来说，没有float形的变量，只有float形的SH，所以这里就用fSH来传递PE的值
						ProtoclCmd[17] = (int(cfg.fSH)) & 0xFF;
					}
					nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
					ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
					ProtoclCmd[13] = nChecksum & 0xFF;

					nwrite = write(fd_com,ProtoclCmd,nPacketSize);
					printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
					if(nwrite < nPacketSize)
					{
						LogError("发送网口控制信息失败！\r\n");
						OpenDev(m_strControlIP, 3600);	//重连相机控制连接
					}
					else
					{
						cam_cfg.fSH=cfg.fSH;
						printf("发送网口控制信息成功！\r\n");
					}
				}
			}

			else if(kind== CAMERA_POL)
			{
				//0x01a4
				ProtoclCmd[14] = 0x01;
				ProtoclCmd[15] = 0xa4;
				if(method == 0)
				{	
					ProtoclCmd[16] = (cfg.uPol & 0xFF00) >> 8;
					ProtoclCmd[17] =  cfg.uPol & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.uPol=cfg.uPol;
					printf("发送网口控制信息成功！\r\n");
				}
			}

			else if(kind==CAMERA_EEN)
			{
				//0x01a3//不输出这个地址设为1
				ProtoclCmd[14] = 0x01;
				ProtoclCmd[15] = 0xa3;
				if(method == 0)
				{
					ProtoclCmd[16] = (cfg.EEN_on & 0xFF00) >> 8;
					ProtoclCmd[17] =  cfg.EEN_on & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;

				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.EEN_on=cfg.EEN_on;//获取实际值，也就是转换后的值
					printf("发送网口控制信息成功！\r\n");
				}
			}

			else if(kind==CAMERA_GAIN)
			{
				//0x0221
				ProtoclCmd[14] = 0x02;
				ProtoclCmd[15] = 0x21;
				if(method == 0)
				{
					int nGain = (int)cfg.fGain;
					ProtoclCmd[16] = (nGain & 0xFF00) >> 8;
					ProtoclCmd[17] = nGain & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;

				nwrite = write(fd_com,ProtoclCmd,nPacketSize);		
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
					OpenDev(m_strControlIP, 3600);	//重连相机控制连接

				}
				else
				{
					cam_cfg.fGain=cfg.fGain;
					printf("发送网口控制信息成功！\r\n");
				}
			}
			else if(kind==CAMERA_AGC)
			{
				//0x0210
				ProtoclCmd[14] = 0x02;
				ProtoclCmd[15] = 0x20;
				if(method == 0)
				{				
					ProtoclCmd[16] = (cfg.AGC & 0xFF00) >> 8;
					ProtoclCmd[17] =  cfg.AGC & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.AGC=cfg.AGC;
					printf("发送网口控制信息成功！\r\n");
				}
			}
			else if(kind==CAMERA_ASC)
			{
				//0x0210
				ProtoclCmd[14] = 0x02;
				ProtoclCmd[15] = 0x10;
				if(method == 0)
				{				
					ProtoclCmd[16] = (cfg.ASC & 0xFF00) >> 8;
					ProtoclCmd[17] =  cfg.ASC & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;

				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,nwrite=%d,ProtoclCmd=%s",nwrite,ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.ASC=cfg.ASC;
					printf("发送网口控制信息成功！\r\n");
				}
			}
			else if(kind==CAMERA_SH)
			{
				/*nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送串口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.uSH=cfg.uSH;
					printf("发送串口控制信息成功！\r\n");
				}*/
			}
			else if(kind==CAMERA_SM)
			{
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.uSM=cfg.uSM;
					printf("发送网口控制信息成功！\r\n");
				}
			}
			else if(kind==CAMERA_PEI)
			{
				//0x01a1
				ProtoclCmd[14] = 0x01;
				ProtoclCmd[15] = 0xa1;
				if(method == 0)
				{				
					ProtoclCmd[16] = (cfg.EEN_delay & 0xFF00) >> 8;
					ProtoclCmd[17] = cfg.EEN_delay & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.EEN_delay=cfg.EEN_delay;
					printf("发送网口控制信息成功！\r\n");
				}
			}
			else if(kind==CAMERA_PEW)
			{
				//0x01a2
				ProtoclCmd[14] = 0x01;
				ProtoclCmd[15] = 0xa2;
				if(method == 0)
				{				
					ProtoclCmd[16] = (cfg.EEN_width & 0xFF00) >> 8;
					ProtoclCmd[17] = cfg.EEN_width & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.EEN_width=cfg.EEN_width;
					printf("发送网口控制信息成功！\r\n");
				}
			}
			else if(kind==CAMERA_IRIS)//光圈控制
			{
				//0x0201
				ProtoclCmd[14] = 0x02;//大端
				ProtoclCmd[15] = 0x01;
				if(method == 0)
				{				
					ProtoclCmd[16] = (cfg.nIris & 0xFF00) >> 8;
					ProtoclCmd[17] = cfg.nIris & 0xFF;
				}
				nChecksum = msg_drv_create_crc((uint8*)ProtoclCmd, nPacketSize);
				ProtoclCmd[12] = (nChecksum & 0xFF00) >> 8;
				ProtoclCmd[13] = nChecksum & 0xFF;

				nwrite = write(fd_com,ProtoclCmd,nPacketSize);
				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					cam_cfg.nIris=cfg.nIris;
					printf("发送网口控制信息成功！\r\n");
				}
			}
			else if(kind==CAMERA_CMD)
			{
				if( bBlockCmd)//设置裁剪卡
				{
					char block_cmd[6] = {0};
					unsigned short nValue = (unsigned short)cfg.fValue;
					//取高8位
					BYTE byteHigh = (nValue>>8)&0xff;
					 //取低8位
					BYTE byteLow = nValue&0xff;


				   sprintf(block_cmd,"%c%c%c%c\r\n",cfg.chCmd[0],cfg.chCmd[1],byteHigh,byteLow);
				   nwrite = write(fd_com,block_cmd,6);

				   printf(" CSerialComm::SendMessage,nwrite=%d,block_cmd=%c%c%x%x\n",nwrite,block_cmd[0],block_cmd[1],(BYTE)block_cmd[2],(BYTE)block_cmd[3]);
					if(nwrite < 6)
					{
						LogError("发送网口控制信息失败！\r\n");
					}
					else
					{
						printf("发送网口控制信息成功！\r\n");
					}
				}
				else//设置相机
				{
					nwrite = write(fd_com,ProtoclCmd,nPacketSize);
					printf(" CSerialComm::SendMessage,strCmd=%s,strCmd.size()=%d\n",strCmd.c_str(),strCmd.size());
					if(nwrite < nPacketSize)
					{
						LogError("发送网口控制信息失败！\r\n");
					}
					else
					{
						printf("发送网口控制信息成功！\r\n");
					}
				}
			}
			else if(kind==CAMERA_SA)
			{
				//保存设置
				nwrite = write(fd_com,ProtoclCmd,nPacketSize);

				printf(" CSerialComm::SendMessage,ProtoclCmd=%s\n",ProtoclCmd);
				if(nwrite < nPacketSize)
				{
					LogError("发送网口控制信息失败！\r\n");
				}
				else
				{
					printf("发送网口控制信息成功！\r\n");
				}
			}
		

			//读回回码
			if(nwrite > 0)
			{
				int nCount = 0;
				int nread = 0;
				int nReadCount = 0;
				char readBuf[64] = {0};
				while(true)
				{
					memset(readBuf,0,64);
					nread = read(fd_com,readBuf,64);
					if(nread > 0)
					{
					
						{
						//	LogError("method=%d,nread=%d,kind=%d\r\n",method,nread,kind);
						}
						memcpy((protoclCmd+nReadCount),readBuf,nread);
						nReadCount += nread;
						if(nReadCount >= 14)//14是包头的大小
						{
							if(((method == 0)||(method == 2) || (method == 3)|| (method == 4)) && (nReadCount == 16))//设置相机参数或者是相机升级准备正常返回都是16个字节
							{
								if(g_bShortConnect)
								{
									Close();
								}
								return -100;
							}
							else if((method == 1) && (nReadCount == 18))//获取相机参数正常返回18个字节
							{
								int nRet = (protoclCmd[16] << 8) + protoclCmd[17];
								if(g_bShortConnect)
								{
									Close();
								}
								return nRet;
							}
							else if(((method == 11) || (method == 12) || (method == 15) || (method == 16) || (method == 19) || (method == 20)) && (nReadCount == 18))
							{
								if((method == 11) || (method == 15) || (method == 19))
								{
									m_strIP = "";//在获取IP前先清空，这个IP可以是视频IP，组播IP和控制和IP
									//m_strMultiIP = "";
								}
								char chIP[20] = {0};
								sprintf(chIP,"%d.",protoclCmd[16]);
								m_strIP += chIP;
								if((method == 12) || (method == 16) || (method == 20))
								{
									sprintf(chIP,"%d",protoclCmd[17]);
								}
								else if((method == 11) || (method == 15) || (method == 19))
								{
									sprintf(chIP,"%d.",protoclCmd[17]);
								}
								m_strIP += chIP;
								if(g_bShortConnect)
								{
									Close();
								}
								return -100;
							}
							else if((method == 6) && (nReadCount == 46))//获取相机版本正常返回46个字节
							{
								char chTemp[32] = {0};//存放版本字符串ASC码，从protoclCmd[14]开始
								memcpy(chTemp,protoclCmd+14,30);
								m_strVersion = chTemp;
								if(g_bShortConnect)
								{
									Close();
								}
								return -100;
							}
						}
					}

					if(nCount >= 30)
					{
						if(nSendCount >= 2)
						{
							LogError("读网口超时!method=%d,nread=%d,kind=%d\r\n",method,nread,kind);
						}
						break;
					}
					if(method == 4)//相机升级准备的命令返回时间需要很久所以需要等到很久
					{
						sleep(8);
					}
					usleep(1000*8);
					nCount++;
				}
			}
			//return -2;//表示超时
		}
		else
		{
			if(!g_bShortConnect)
			OpenDev(m_strControlIP, 3600);			
		}

		if(g_bShortConnect)
		{
			Close();
		}
		
		nSendCount++;
		usleep(5000);
	}
	return -1;//表示连接没有建立
}


/*************************************************
* Function      : msg_drv_create_crc
* Description  : create crc by crc table
* Input          : pData =data address
* 			  len = data len
* Output        : None
* Return        : None
*************************************************/
unsigned short CamerSerial::msg_drv_create_crc(uint8 * pData , unsigned short len)
{
	uint8 data;
	
	uint8 da;
	unsigned short crc=0;
	while(len--!=0)
	{
		da=crc>>8; // 以8 位二进制数暂存CRC 的高8 位

		crc<<=8; // 左移8 位
data = *pData;
		crc^=g_CrcTab[da^*pData]; // 高字节和当前数据XOR 再查表
		pData++;
	}
	return(crc);
}

//升级相机  type升级程序类型:1＝网管程序2＝FPGA程序   Index升级第几个芯片（1～4）
int CamerSerial::UpdateCam(int type, int Index)
{
	int n = 0;
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,4);
	//SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,5);
	//while(n)
	{
		//SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,5);
	}
}

//获取升级文件和它的大小
int CamerSerial::GetUpdateFile(FILE* fp, int& nSize, string strPath)
{
	DIR *dirptr = NULL;
    struct dirent *entry;
    if((dirptr = opendir(strPath.c_str())) == NULL)
    {
            printf("open error!!\n");
            return -1;
    }
    else
    {
        while(entry = readdir(dirptr))
        {
                if((strcmp(entry->d_name,".") == 0) || (strcmp(entry->d_name,"..") == 0))
                {
                        continue;
                }
				else
				{
					printf("FILE NAME:%s\n",entry->d_name);
					char chFILE[100] = {0};
					sprintf(chFILE,"/home/lch/test/txt/%s",entry->d_name);
					printf("chFILE=%s\n",chFILE);
					fp = fopen(chFILE,"r");
					if(!fp)
					{
						struct stat stat_buf;
						stat(chFILE,&stat_buf);
						nSize = stat_buf.st_size;
					}
					else
					{
						return -1;
					}
					break;//找到文件就退出
				}
        }
        closedir(dirptr);
    }
	return 1;
}

void CamerSerial::GetVersion()
{
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,6);
}

int CamerSerial::GetCamId()
{
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,7);
}

void CamerSerial::SetCamId()
{
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,8);
}

//设置组播地址(视频)
int CamerSerial::SetAddress(string strAddr,int nCamId)
{
	string strPreAddr = strAddr;
	bool flag = true;
	int nHighIP = 0;//高位IP
	int nLowIP = 0;//低位IP
	int nHighIP1 = 0;//高位IP的第一位
	int nHighIP2 = 0;//高位IP的第二位
	int nLowIP1 = 0;//低位IP的第一位
	int nLowIP2 = 0;//低位IP的第二位
	int nPoint = 0;//记录找到的点的个数
	//strAddr = "224.168.0.2";
	string tempstrIP;
	while(flag)
	{
			int nLocate = strAddr.find(".");
			if(nLocate == string::npos)
			{
					flag = false;
			}
			else
			{
					nPoint++;
					tempstrIP = strAddr.substr(0,nLocate);
					strAddr = strAddr.substr(nLocate +1);
			}
			if(nPoint == 1)
			{
					nHighIP1 = strtoul(tempstrIP.c_str(),0,10);//如224
			}
			if(nPoint == 2)
			{
					nHighIP2 = strtoul(tempstrIP.c_str(),0,10);//如168
			}
			if(nPoint == 3)
			{
					nLowIP1 = strtoul(tempstrIP.c_str(),0,10);//如0
			}
	}
	if(strAddr != "")
	{
			tempstrIP = strAddr;
			nLowIP2 = strtoul(tempstrIP.c_str(),0,10);//1

			nHighIP = (nHighIP1 << 8) + nHighIP2;
			nLowIP = (nLowIP1 << 8) + nLowIP2;
	}
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	cfg.fValue = nHighIP;
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,13,nCamId);
	usleep(5000);
	//SetIT(nCamId);
	cfg.fValue = nLowIP;
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,14,nCamId);
	usleep(5000);
	//SetIT(nCamId);
	usleep(5000);
	string strGetIP;
	GetAddress(strGetIP, nCamId);
	if(!(strPreAddr.compare(strGetIP)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//获取组播地址(视频)
int CamerSerial::GetAddress(string &strAddr,int nCamId)
{
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,15,nCamId);
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,16,nCamId);
	strAddr = m_strIP;
}

//获取相机控制地址
int CamerSerial::GetControlAddress(string &strAddr,int nCamId)
{
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,19,nCamId);
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,20,nCamId);
	strAddr = m_strIP;
}

//设置相机IP地址(视频)
int CamerSerial::SetControlAddress(string strIP,int nCamId)
{
	string strPreIP = strIP;
	bool flag = true;
	int nHighIP = 0;//高位IP
	int nLowIP = 0;//低位IP
	int nHighIP1 = 0;//高位IP的第一位
	int nHighIP2 = 0;//高位IP的第二位
	int nLowIP1 = 0;//低位IP的第一位
	int nLowIP2 = 0;//低位IP的第二位
	int nPoint = 0;//记录找到的点的个数
	string tempstrIP;
	while(flag)
	{
		int nLocate = strIP.find(".");
		if(nLocate == string::npos)
		{
			flag = false;
		}
		else
		{
			nPoint++;
			tempstrIP = strIP.substr(0,nLocate);
			strIP = strIP.substr(nLocate +1);
		}
		if(nPoint == 1)
		{
			nHighIP1 = strtoul(tempstrIP.c_str(),0,10);
		}
		if(nPoint == 2)
		{
			nHighIP2 = strtoul(tempstrIP.c_str(),0,10);
		}
		if(nPoint == 3)
		{
			nLowIP1 = strtoul(tempstrIP.c_str(),0,10);
		}
	}
	if(strIP != "")
	{
		tempstrIP = strIP;
		nLowIP2 = strtoul(tempstrIP.c_str(),0,10);
		nHighIP = (nHighIP1 << 8) + nHighIP2;
		nLowIP = (nLowIP1 << 8) + nLowIP2;
	}

	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	cfg.fValue = nHighIP;
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,17,nCamId);
	usleep(5000);
	//SetIT(nCamId);
	cfg.fValue = nLowIP;
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,18,nCamId);
	usleep(5000);
	//SetIT(nCamId);
	usleep(5000);
	string strGetIP;
	GetIPAddress(strGetIP, nCamId);
	if (!(strPreIP.compare(strGetIP)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//获取相机IP地址(视频)
int CamerSerial::GetIPAddress(string &strIP,int nCamId)
{
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,11,nCamId);
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,12,nCamId);
	strIP = m_strIP;
}

//设置相机IP地址(视频)
int CamerSerial::SetIPAddress(string strIP,int nCamId)
{
	string strPreIP = strIP;
	bool flag = true;
	int nHighIP = 0;//高位IP
	int nLowIP = 0;//低位IP
	int nHighIP1 = 0;//高位IP的第一位
	int nHighIP2 = 0;//高位IP的第二位
	int nLowIP1 = 0;//低位IP的第一位
	int nLowIP2 = 0;//低位IP的第二位
	int nPoint = 0;//记录找到的点的个数
	string tempstrIP;
	while(flag)
	{
			int nLocate = strIP.find(".");
			if(nLocate == string::npos)
			{
					flag = false;
			}
			else
			{
					nPoint++;
					tempstrIP = strIP.substr(0,nLocate);
					strIP = strIP.substr(nLocate +1);
			}
			if(nPoint == 1)
			{
					nHighIP1 = strtoul(tempstrIP.c_str(),0,10);
			}
			if(nPoint == 2)
			{
					nHighIP2 = strtoul(tempstrIP.c_str(),0,10);
			}
			if(nPoint == 3)
			{
					nLowIP1 = strtoul(tempstrIP.c_str(),0,10);
			}
	}
	if(strIP != "")
	{
			tempstrIP = strIP;
			nLowIP2 = strtoul(tempstrIP.c_str(),0,10);
			nHighIP = (nHighIP1 << 8) + nHighIP2;
			nLowIP = (nLowIP1 << 8) + nLowIP2;
	}

	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	cfg.fValue = nHighIP;
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,9,nCamId);
	usleep(5000);
	//SetIT(nCamId);
	cfg.fValue = nLowIP;
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,10,nCamId);
	usleep(5000);
	//SetIT(nCamId);
	usleep(5000);
	string strGetIP;
	GetIPAddress(strGetIP, nCamId);
	if (!(strPreIP.compare(strGetIP)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//永久保存参数，不执行此函数，相机断电后参数会恢复成默认值
void CamerSerial::SaveCamPara(int nCamID)
{
	CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,100,nCamID);
}

//打开相机控制网口
bool CamerSerial::OpenDev(string strIP, int nPort)
{
	m_strControlIP = strIP;
	
	if(fd_com!=-1)//如果之前已经建立了连接，先关闭
	{
		shutdown(fd_com,2);
		close(fd_com);
		fd_com = -1;
	}
	
	struct sockaddr_in tcp_addr;
	// socket
	if ((fd_com=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		return false;
	}

	struct timeval timeo;
	socklen_t len = sizeof(timeo);
	timeo.tv_sec=0;
	timeo.tv_usec=5000;//超时0.0005s


	if(setsockopt(fd_com, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == -1)
	{
		shutdown(fd_com,2);
		close(fd_com);
		fd_com = -1;
		return false;
	}

	int on = 1;
	if(setsockopt(fd_com, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(int)) == -1)
	{
		shutdown(fd_com,2);
		close(fd_com);
		fd_com = -1;
		return false;
	}

	if(setsockopt(fd_com, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == -1)
	{
		shutdown(fd_com,2);
		close(fd_com);
		fd_com = -1;
		return false;
	}

	bzero(&tcp_addr,sizeof(tcp_addr));
	tcp_addr.sin_family=AF_INET;
	tcp_addr.sin_addr.s_addr=inet_addr(strIP.c_str());
	tcp_addr.sin_port=htons(nPort);

	int nCount = 0;
	while(nCount < 3)//连接不上最多重试三次
	{
		if(connect(fd_com,(struct sockaddr *)&tcp_addr,sizeof(struct sockaddr))==-1)
		{
			if(nCount == 2)
			{
				LogError("相机网口控制建立失败connect:%s\n",strerror(errno));
				if(fd_com!=-1)
				{
					shutdown(fd_com,2);
					close(fd_com);
					fd_com = -1;
				}
				return false;
			}
		}
		else
		{
			break;
		}
		//LogError("nCount=%d",nCount);
		nCount++;
		usleep(1000*10);
	}
	//LogNormal("相机网口控制建立成功");
	
	return true;
}


void CamerSerial::ChangeMode(int nMode,int nCamID)
{
	/*0x01a0 工作模式
	"0= 连续模式（25Hz）    5= Edge Pre Select（边沿触发）"	

	0x01a6 外触发输入
	"0=内触发（内部产生15Hz的触发源）  1=外部触发源"	*/


	//CAMERA_CONFIG cfg;//为匹配参数用，没有实际意义
	//if(nMode == 0)
	//	cfg.fValue = 0;
	//else if(nMode == 1)
	//	cfg.fValue = 5;
	//cfg.nMode = (int)cfg.fValue;
	//cfg.nIndex = (int)CAMERA_MODE1;
	//SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,0,nCamID);
	//memset(&cfg, 0, sizeof(CAMERA_CONFIG));

	//if(nMode == 1)
	//{
	//	cfg.fValue = 1;
	//	cfg.nMode = (int)cfg.fValue;
	//	cfg.nIndex = (int)CAMERA_MODE2;
	//	SendMessageByKodak(cfg,cfg,(CAMERA_MESSAGE)cfg.nIndex,0,nCamID);
	//}
	//SaveCamPara(nCamID);
}
