#include "Common.h"
#include "CommonHeader.h"

//#define DEBUG_DETECT

CVisSerialCommunication g_VisSerialCommunication;

CVisSerialCommunication::CVisSerialCommunication()
{
     fd_com=-1;
}

CVisSerialCommunication::~CVisSerialCommunication()
{

}

bool CVisSerialCommunication::OpenDev()
{
    //打开相机控制串口
    printf("CamerSerial::OpenDev g_VisComSetting.nComPort=%d,g_VisComSetting.nBaud=%d\n",g_VisComSetting.nComPort,g_VisComSetting.nBaud);

	fd_com = open_port(g_VisComSetting.nComPort,g_VisComSetting.nBaud,g_VisComSetting.nDataBits,g_VisComSetting.nStopBits,g_VisComSetting.nParity);

	if(fd_com == -1)
	{
	    return false;
	}
}

//发送镜头控制命令(pelco protocol)
bool CVisSerialCommunication::SendData(CAMERA_CONFIG& cfg)
{
	char protoclCmd[16];

	CAMERA_MESSAGE nMsg = (CAMERA_MESSAGE)cfg.nIndex;
	int ndata = (int)cfg.fValue;

	switch(nMsg)
	{
		case ZOOM_FAR:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x40;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x00;
			}
			break;
		case ZOOM_NEAR:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x20;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x00;
			}
			break;
		case FOCUS_FAR:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x80;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x00;
			}
			break;
		case FOCUS_NEAR:
			{
				protoclCmd[2] = 0x01;
				protoclCmd[3] = 0x00;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x00;
			}
			break;
		case IRS_INCREASE:
			{
				protoclCmd[2] = 0x04;
				protoclCmd[3] = 0x00;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x00;
			}
			break;
		case IRS_DECREASE:
			{
				protoclCmd[2] = 0x02;
				protoclCmd[3] = 0x00;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x00;
			}
			break;
        case SET_PRESET:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x03;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = ndata;
			}
			break;
        case CLEAR_PRESET:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x05;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = ndata;
			}
			break;
        case GOTO_PRESET:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x07;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = ndata;
			}
			break;
        case LEFT_DIRECTION:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x04;
				protoclCmd[4] = 0x20;
				protoclCmd[5] = 0x00;
			}
			break;
        case RIGHT_DIRECTION:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x02;
				protoclCmd[4] = 0x20;
				protoclCmd[5] = 0x00;
			}
			break;
        case UP_DIRECTION:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x08;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x20;
			}
			break;
        case DOWN_DIRECTION:
			{
				protoclCmd[2] = 0x00;
				protoclCmd[3] = 0x10;
				protoclCmd[4] = 0x00;
				protoclCmd[5] = 0x20;
			}
			break;
		default:
			return false;
	}

    protoclCmd[0] = 0xff;
	protoclCmd[1] = cfg.nAddress;//address

    int nwrite = 0;

	if(cfg.nOperationType == 1 ||
       cfg.nOperationType == 0) //按下又抬起或只按下
	{
            protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];//check_sum
            printf("%x,%x,%x,%x,%x,%x,%x\n",protoclCmd[0],protoclCmd[1],protoclCmd[2],protoclCmd[3],protoclCmd[4],protoclCmd[5],protoclCmd[6]);

            nwrite = write(fd_com,protoclCmd,7);

            if(nwrite < 7)
            {
                LogError("发送串口控制信息失败！\r\n");
                return false;
            }
            else
            {
                printf("发送串口控制信息成功！\r\n");

                //fcntl(fd_com, F_SETFL, FNDELAY);
                char buf[256]={0};
                int nread = read(fd_com,buf,20);
                //if(nread!=-1)
                {
                    printf("buf=%s,nread=%d\n",buf,nread);
                }
            }
	}

	if(cfg.nOperationType == 2 ||
       cfg.nOperationType == 0)//按下又抬起或只抬起
	{
	    printf("cfg.nIndex=%d,GOTO_PRESET=%d\n",cfg.nIndex,GOTO_PRESET);
	    if(cfg.nIndex == ZOOM_NEAR ||
           cfg.nIndex == ZOOM_FAR ||
           cfg.nIndex == UP_DIRECTION ||
           cfg.nIndex == DOWN_DIRECTION ||
           cfg.nIndex == LEFT_DIRECTION ||
           cfg.nIndex == RIGHT_DIRECTION )
        {
            //发停止命令
            protoclCmd[2] = 0x00;
            protoclCmd[3] = 0x00;
            protoclCmd[4] = 0x00;
            protoclCmd[5] = 0x00;
            protoclCmd[6] = protoclCmd[1]+protoclCmd[2]+protoclCmd[3]+protoclCmd[4]+protoclCmd[5];

            nwrite = 0;
            nwrite = write(fd_com,protoclCmd,7);

            if(nwrite < 7)
            {
                LogError("发送串口停止控制信息失败！\r\n");
                return false;
            }
            else
            {
                printf("发送串口停止控制信息成功！\r\n");
                return true;
            }
	    }
	}
}

//发送命令到vis
bool CVisSerialCommunication::WriteCmdToVis(std::string& sCmdMsg)
{
    int nwrite = 0;
    nwrite = write(fd_com,sCmdMsg.c_str(),sCmdMsg.size());
    printf("sCmdMsg=%s,sCmdMsg.size()=%d\r\n",sCmdMsg.c_str(),sCmdMsg.size());

    if(nwrite<sCmdMsg.size())
    {
        LogError("发送键盘码控制信息失败！\r\n");
        return false;
    }
    return true;
}
