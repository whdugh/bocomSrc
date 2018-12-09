#include "Common.h"
#include "CommonHeader.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
FlashSerial * FlashSerial::m_flashSerial = new FlashSerial();

FlashSerial::FlashSerial(void)
{
    fd_com = -1;
}

FlashSerial::~FlashSerial(void)
{
}

bool FlashSerial::OpenDev()
{
    fd_com = open_port((g_FlashComSetting.nComPort),g_FlashComSetting.nBaud,g_FlashComSetting.nDataBits,g_FlashComSetting.nStopBits,g_FlashComSetting.nParity,true);

	if(fd_com == -1)
	{
	    return false;
	}
	else
	{
	    return true;
	}
}

//±¬ÉÁµÆ´®¿Ú¿ØÖÆ
//nOperateType=0:open;1:close;2:open and close
bool FlashSerial::InputCmd(int selectCmd,int nOperateType)
{
    int nwrite = -1;
    printf("selectCmd = %d\n",selectCmd);
    char *openCmd = (char*) malloc(32);
    char *closeCmd = (char*) malloc(32);
    char *answerCmd = (char*) malloc(32);
    if(selectCmd!=0 && selectCmd!=1 && selectCmd!=2 && selectCmd!=3)
    {
        printf("no this command\n");
        return false;
    }
    printf("in the InputCmd\n");


    unsigned char open[11] = {0x01,0x30,0x31,0x44,0x4F,0x50,0x30,0x30,0x31,0x00,0x04};
    unsigned char close[11] = {0x01,0x30,0x31,0x44,0x4F,0x50,0x30,0x30,0x30,0x00,0x04};
    unsigned char answer[9] = {0x01,0x30,0x31,0x52,0x4F,0x50,0x30,0x00,0x04};
	unsigned char goback[255] = {0};

    memset(openCmd, 0, 32);
    memset(closeCmd, 0, 32);
    memset(answerCmd, 0, 32);

    switch(selectCmd)
    {
        case 0:
        {
            printf("case1\n");
            open[8] = 0x31;
            memcpy(openCmd, "01DOP001", 9);
            memcpy(closeCmd, "01DOP000", 9);
            memcpy(answerCmd, "01ROP0", 6);
            break;
        }

        case 1:
        {
            printf("case2\n");
            open[8] = 0x32;
            memcpy(openCmd, "01DOP002", 9);
            memcpy(closeCmd, "01DOP000", 9);
            memcpy(answerCmd, "01ROP0", 6);
            break;
        }

        case 2:
        {
            printf("case3\n");
            open[8] = 0x34;
            memcpy(openCmd, "01DOP004", 9);
            memcpy(closeCmd, "01DOP000", 9);
            memcpy(answerCmd, "01ROP0", 6);
            break;
        }
        case 3:
        {
            printf("case4\n");
            open[8] = 0x38;
            memcpy(openCmd, "01DOP008", 9);
            memcpy(closeCmd, "01DOP000", 9);
            memcpy(answerCmd, "01ROP0", 6);
            break;
        }
    }

    open[9] = Checksum(openCmd);
    close[9] = Checksum(closeCmd);
    answer[7] = Checksum(answerCmd);
	
	if(nOperateType == 2 || nOperateType == 0)
	{
		nwrite = write(fd_com, close, 11);
		printf("nwrite0 = %d\n",nwrite);
		//usleep(10000);
		nwrite = write(fd_com, answer, 9);
		printf("nwrite0 = %d\n",nwrite);
		nwrite = read(fd_com, goback, 255);
		printf("nwrite0 = %d\n",nwrite);
	
		nwrite = write(fd_com, open, 11);
		printf("nwrite1 = %d\n",nwrite);
		nwrite = write(fd_com, answer, 9);
		printf("nwrite3 = %d\n",nwrite);
		nwrite = read(fd_com, goback, 255);
		printf("nwrite4 = %d\n",nwrite);
		//printf("open is:%s\n",goback);
	}

	if(nOperateType == 2)
	{
		usleep(2000);
	}
	
	if(nOperateType == 2 || nOperateType == 1)
	{
		nwrite = write(fd_com, close, 11);
		printf("nwrite2 = %d\n",nwrite);
		//usleep(10000);
		nwrite = write(fd_com, answer, 9);
		printf("nwrite3 = %d\n",nwrite);
		nwrite = read(fd_com, goback, 255);

		printf("nwrite4 = %d\n",nwrite);
		//printf("close is:%s\n",goback);
		printf("fd_com = %d\n",fd_com);
		printf("g_FlashComSetting.nComPort = %d-------->\n",g_FlashComSetting.nComPort);
	}

    if(openCmd)
        free(openCmd);
    if(closeCmd)
        free(closeCmd);
    if(answerCmd)
        free(answerCmd);
    return true;
}


unsigned char FlashSerial::Checksum(char* chksumData)
{
    printf("in the chksum\n");
    unsigned char totalASCII = 0;
    unsigned char tempASCII = 0;
    int i = 0;
    int chksumDataLen = strlen(chksumData);
    for(i=0; i<chksumDataLen; i++)
    {
        tempASCII = 0;
        tempASCII = chksumData[i];
        totalASCII += tempASCII;
        totalASCII = totalASCII & 0x7F;

    }
        if(totalASCII <= 32)
        {
                totalASCII += 32;
        }

        printf("totalASCII=%d\n",totalASCII);
        return totalASCII;
}
