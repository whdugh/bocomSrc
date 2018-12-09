#include "smallscreenctrl.h"

CSmallScreenCtrl g_SmallScreenCtrl; 

void * ThreadSmallScreenCtrl(void *)
{
	g_SmallScreenCtrl.RecData();
}
CSmallScreenCtrl::CSmallScreenCtrl()
{
	fd_com = -1;
}
CSmallScreenCtrl::~CSmallScreenCtrl()
{

}

bool CSmallScreenCtrl::OpenDev()
{
	char buf[128] = {0};
	sprintf(buf,"/dev/ttyS%d",g_ScreenComSetting.nComPort-1);

	fd_com = open(buf, O_RDWR | O_NOCTTY | O_NDELAY);  
	if (fd_com < 0) {  
		LogError("open %s\n",buf);  
		return false;  
	}  

	//.........termios <termios.h>  
	struct termios options;  

	/**1. tcgetattr............... 
	*..fd_com..................termios.... 
	*/  
	tcgetattr(fd_com, &options);  
	/**2. ........*/  
	options.c_cflag |= (CLOCAL | CREAD);//..................  
	options.c_cflag &= ~CSIZE;//....................  
	options.c_cflag &= ~CRTSCTS;//.....  
	options.c_cflag |= CS8;//8.....  
	options.c_cflag &= ~CSTOPB;//1....  
	options.c_iflag |= IGNPAR;//......  
	options.c_oflag = 0; //....  
	options.c_lflag = 0; //.......  
	options.c_cc[VTIME]=10;
	options.c_cc[VMIN]=1;
	cfsetospeed(&options, B9600);//.....  

	/**3. ......TCSANOW.........*/  
	tcflush(fd_com, TCIFLUSH);//............  
	tcsetattr(fd_com, TCSANOW, &options);  

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

void CSmallScreenCtrl::BeginThread()
{
	//printf("===========CGpsSerial::BeginThread()=========\n");

	//线程id
	pthread_t id;
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//启动记录发送线程

	int nret=pthread_create(&id,&attr,ThreadSmallScreenCtrl,NULL);
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

void CSmallScreenCtrl::screen_ctrl(char a,char b)
{
	char write_buf[256];
	memset(write_buf,0, sizeof(write_buf));
	write_buf[0] = a;
	write_buf[1] = b;

	write(fd_com, write_buf, strlen(write_buf));
}

void CSmallScreenCtrl::position_line(char a,char b,char c)
{
	char write_buf[256];
	memset(write_buf,0, sizeof(write_buf));
	write_buf[0] = a;
	write_buf[1] = b;
	write_buf[2] = c;
	write(fd_com, write_buf, strlen(write_buf));
}

void CSmallScreenCtrl::line_disp(char* buf)
{
	char write_buf[256];
	memset(write_buf,0, sizeof(write_buf));
	write_buf[0] = 0xF8;
	write_buf[1] = 0x03;
	int i;
	for(i=0;i<strlen(buf);i++)
	{
		write_buf[i+2] = buf[i];
	}
	write_buf[strlen(buf)+2] = 0xA0;
	//printf("%d\n",strlen(write_buf));
	write(fd_com, write_buf, strlen(write_buf));
}

void CSmallScreenCtrl::cursor(int key5)
{
	int i;
	switch(key5%12)
	{
		case 0:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<32;i++)
				screen_ctrl(0xf8,0x10);
			break;
		case 1:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<33;i++)
				screen_ctrl(0xf8,0x10);
			break;
		case 2:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<35;i++)
				screen_ctrl(0xf8,0x10);break;
		case 3:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<36;i++)
				screen_ctrl(0xf8,0x10);break;
		case 4:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<68;i++)
				screen_ctrl(0xf8,0x10);break;
		case 5:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<69;i++)
				screen_ctrl(0xf8,0x10);break;
		case 6:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<71;i++)
				screen_ctrl(0xf8,0x10);break;
		case 7:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<72;i++)
				screen_ctrl(0xf8,0x10);break;
		case 8:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<74;i++)
				screen_ctrl(0xf8,0x10);break;
		case 9:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<75;i++)
				screen_ctrl(0xf8,0x10);break;
		case 10:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<76;i++)
				screen_ctrl(0xf8,0x10);break;
		case 11:
			screen_ctrl(0xf8,0x02);
			for(i=0;i<77;i++)
				screen_ctrl(0xf8,0x10);break;
		default: break;
	}
	screen_ctrl(0xf8,0x0f);
}

void CSmallScreenCtrl::RecData()
{
	char buf1[20];
	char buf2[20];
	char recv[10];

	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;

	//清屏
	position_line(0xf8,0x01,0xa0);
	sleep(1);

	while (!g_bEndThread)
	{
		time( &now );
		localtime_r( &now,newTime );
		memset(buf1, 0, sizeof(buf1));
		memset(buf2, 0, sizeof(buf2));
		sprintf(buf1, "%4d-%02d-%02d", newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday);
		sprintf(buf2, "%02d:%02d:%02d", newTime->tm_hour, newTime->tm_min, newTime->tm_sec);

		position_line(0xf8,0x80,0x03);
		line_disp(buf1);
		position_line(0xf8,0x80,0x44);
		line_disp(buf2);
		memset(recv,0, sizeof(recv));
		int key3,key4,key5,i;
		key3=0;key4=0;key5=0;
		recv[0]=0;
		read(fd_com,recv,1);

		if(recv[0]==-125)//k3 
		{       
			key3=1;
			screen_ctrl(0xf8,0x02);
			for(i=0;i<32;i++)
				screen_ctrl(0xf8,0x10);

			screen_ctrl(0xf8,0x0f);

			while(key3)
			{
				memset(recv,0, sizeof(recv));
				read(fd_com,recv,1);
				if(recv[0]==-125)
					break;

				if(recv[0]==-123)//key5
				{
					key5 =key5+1;
					cursor(key5);
				}
				if(recv[0]==-124)//key4
				{
					key4=key4+1;

					switch(key5%12)
					{
					case 0:
						buf2[4]=(buf2[4]+1-48)%10+48;
						break;
					case 1:
						buf2[3]=(buf2[3]+1-48)%10+48;break;
					case 2:
						buf2[1]=(buf2[1]+1-48)%10+48;break;
					case 3:
						buf2[0]=(buf2[0]+1-48)%10+48;break;
					case 4:
						buf1[9]=(buf1[9]+1-48)%10+48;break;
					case 5:
						buf1[8]=(buf1[8]+1-48)%10+48;break;
					case 6:
						buf1[6]=(buf1[6]+1-48)%10+48;break;
					case 7:
						buf1[5]=(buf1[5]+1-48)%10+48;break;
					case 8:
						buf1[3]=(buf1[3]+1-48)%10+48;break;
					case 9:
						buf1[2]=(buf1[2]+1-48)%10+48;break;
					case 10:
						buf1[1]=(buf1[1]+1-48)%10+48;break;
					case 11:
						buf1[0]=(buf1[0]+1-48)%10+48;break;
					default: break;

					}
					position_line(0xf8,0x80,0x03);
					line_disp(buf1);
					position_line(0xf8,0x80,0x44);
					line_disp(buf2);
					cursor(key5);
				}
			}
			char temp[100];
			memset(temp,0, sizeof(temp));
			if(key4>0)
			{
				strcat(temp,"date -s '");
				strcat(temp,buf1);
				strcat(temp," ");
				strcat(temp,buf2);
				strcat(temp,"'");
				printf("%s\n",temp);
				system(temp);
				system("hwclock --systohc");
			}
		}
		sleep(1);
	}
}

