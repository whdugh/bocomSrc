#include "AbstractSerial.h"
#include "Common.h"

#ifndef DETECT_EVENT_STOP_LOG
    //#define DETECT_EVENT_STOP_LOG
#endif

/*
#ifndef _DEBUG
    #define _DEBUG
#endif
*/

//构造函数
AbstractSerial::AbstractSerial()
{
    fd_com=-1;

}
//析构函数
AbstractSerial::~AbstractSerial()
{

}

 bool AbstractSerial::OpenDev()
{
    return true;
}

 bool AbstractSerial::Close()
{
    if(fd_com!=-1)
	{
		close(fd_com);
		fd_com = -1;
	}

	return true;
}
void AbstractSerial::set_speed(int fd,int nSpeed)
{
	if(fd!=-1)
	{
		int   status;
		struct termios   Opt;
		if(tcgetattr(fd, &Opt)==-1)
		{
			LogError("无法获取串口属性！\r\n");
			return;
		}

		Opt.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
		Opt.c_oflag  &= ~OPOST;   /*Output*/


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
			LogError("无法设置串口波特率！\r\n");
			return;
		}
		tcflush(fd,TCIOFLUSH);
	}
}
int AbstractSerial::set_Parity(int fd,int databits,int stopbits,int parity)
{
	if(fd!=-1)
	{
		struct termios options;
		if  ( tcgetattr( fd,&options)  !=  0)
		{
			LogError("无法获取串口属性！\r\n");
			return -1;
		}
		options.c_cflag &= ~CSIZE;
		switch (databits) /*设置数据位数*/
		{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			return -1;
		}
		switch (parity) /*设置效验位*/
		{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
			options.c_iflag |= INPCK;             /* Disnable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			return -1;
		}
		/* 设置停止位*/
		switch (stopbits)
		{
			case 1:
				options.c_cflag &= ~CSTOPB;
				break;
			case 2:
				options.c_cflag |= CSTOPB;
			   break;
			default:
				 return -1;
		}
		/* Set input parity option */
		/*if (parity != 'n')
			options.c_iflag |= INPCK;*/

		tcflush(fd,TCIFLUSH);
		//options.c_cc[VSTART] =;
		options.c_cc[VTIME] = 5; /* 设置超时0.5 seconds*/
		options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
		if (tcsetattr(fd,TCSANOW,&options) != 0)
		{
			LogError("无法设置串口校验位！\r\n");
			return -1;
		}
		return 0;
	}
	else
		return -1;
}


int AbstractSerial::set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop,bool bVTime)
{
    struct termios newtio,oldtio;

    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if ( tcgetattr( fd,&oldtio) != 0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    /*步骤一，设置字符大小，分别是本地链接和接受使能*/
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    /*设置停止位*/
    switch( nBits )
    {
        case 7:
        newtio.c_cflag |= CS7;
        break;
        case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    /*设置奇偶校验位*/
    switch( nEvent )
    {
        case 'O': //奇数
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
        case 'E': //偶数
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
        case 'N': //无奇偶校验位
        newtio.c_cflag &= ~PARENB;
        break;
    }
    /*设置波特率*/
    switch( nSpeed )
    {
        case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
        case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
        case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
        case 19200:
			cfsetispeed(&newtio, B19200);
			cfsetospeed(&newtio, B19200);
			break;
		case 57600:
			cfsetispeed(&newtio, B57600);
			cfsetospeed(&newtio, B57600);
			break;
        case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
        case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
        default:
			//cfsetispeed(&newtio, B9600);
			//cfsetospeed(&newtio, B9600);
			printf("unsupported baud rate\n");
			//exit(-1);
			break;
    }
    /*设置停止位*/
    if( nStop == 1 )
    newtio.c_cflag &= ~CSTOPB;
    else if ( nStop == 2 )
    newtio.c_cflag |= CSTOPB;
    /*设置等待时间和最小接收字符*/
   /* if(bVTime)
    {
        newtio.c_cc[VTIME] = 1;
    }
    else*/
    {
        newtio.c_cc[VTIME] = 0;
    }
    newtio.c_cc[VMIN] = 0;
    /*处理未接收字符*/
    tcflush(fd,TCIFLUSH);
    /*激活新配置*/
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}
/*
    打开端口函数
    comport：        打开串口参数
*/
int AbstractSerial::open_port(int comport,int nSpeed,int nDataBit,int nStopBit,int nParity, bool bVTime)
{
    int fd = -1;
    char buf[128] = {0};
    sprintf(buf,"/dev/ttyS%d",comport-1);

    if(bVTime)
    fd = open(buf,O_RDWR|O_NDELAY);//非阻塞方式打开
    else
    fd = open(buf,O_RDWR);
    if (-1 == fd)
    {
        LogError("无法打开串口%d\r\n",comport);
        return(-1);
    }
    else
    {
        LogNormal("打开串口%d成功!\r\n",comport);
    }
    //set_speed(fd,nSpeed);


    char chParity = 'N';
    if(nParity == 0)
    {
        chParity = 'N';
    }
    else if(nParity == 1)
    {
        chParity = 'O';
    }
    else if(nParity == 2)
    {
        chParity = 'E';
    }
    //设置奇偶校验位等
    set_opt(fd,nSpeed,nDataBit,chParity,nStopBit,bVTime);

    return fd;
}

//判断串口是否已经打开
bool AbstractSerial::IsOpen()
{
    if(fd_com!=-1)
	{
		return true;
	}
	else
	{
        return false;
	}
}
