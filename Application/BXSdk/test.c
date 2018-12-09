#include "BaoXinSdk.h"
//获取设备时间回调，返回值0表示成功，-1失败
BX_Int32 GetDeviceTime( BX_Uint64 *pU64msSecond )
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	BX_Uint64 nowtime = tv.tv_sec;
	nowtime = nowtime*1000+tv.tv_usec/1000;
	*pU64msSecond = nowtime;

	printf("[%s]:second=%ld,msecond=%ld\n",__FUNCTION__,tv.tv_sec,tv.tv_usec/1000);


	return 0;
}
//设置设备时间回调，返回值0表示成功，-1失败
BX_Int32 SetDeviceTime( BX_Uint64 U64msSecond )
{
	struct timeval tv;
	tv.tv_sec = U64msSecond/1000;
	tv.tv_usec = U64msSecond%1000 * 1000;
	printf("[%s]:second=%ld,msecond=%ld\n",__FUNCTION__,tv.tv_sec,tv.tv_usec/1000);
	settimeofday(&tv,NULL);
	
	return 0;
}
//获取设备分辨率回调，返回值0表示成功，-1失败
BX_Int32 GetDeviceWH(BX_Uint32 *pWidth,BX_Uint32 *pHeight)
{
	*pWidth = 1920;
	*pHeight = 1080;
	return 0;
}

void testBX_SDK( char *picPath)
{
	BX_Init();
	BX_CB_FUNS funs;
	funs.GetDeviceTime = GetDeviceTime;
	funs.GetDeviceWH = GetDeviceWH;
	funs.SetDeviceTime = SetDeviceTime;

	BX_Reg_CB_FUNS( funs );


	printf("press any key continue...\n");
	getchar();

	BX_HeadData bxHead;
	memset(&bxHead,0,sizeof(bxHead));
	memcpy(bxHead.szPlateCode,"沪ABCD",20);
	bxHead.ui8TotalSnapTimes = 1;
	bxHead.ui8CurTimes = 0;

	bxHead.uiCarMarkType = BX_QITA;
	bxHead.ui16EventType = 10;
	bxHead.uiIllegalCode = 2;
	bxHead.ui32EventID = 10;
	bxHead.uiRecordID = 11;
	bxHead.uiRoadNo = 3;
	bxHead.uiSourceCam = 1;

	memcpy(bxHead.uiVideoFtpPath,"ftp://road:road@192.168.200.171/home/road/server/video/00/02/99.mp4",125);
#define MAXLEN 10*1024*1024
	unsigned char *picData = (unsigned char*)malloc(MAXLEN);
	memset(picData,0,MAXLEN);
	FILE* fp = fopen(picPath,"r");
	if (fp == NULL)
	{
		printf("打开文件失败，%s\n",picPath);
		return;
	}
	fread((void*)picData,1,MAXLEN,fp);

	fseek(fp,0,SEEK_END);
	int len = ftell(fp);
	
	fclose(fp);
	printf("picLen = %d\n",len);

	while( getchar() != '1')
	{
		GetDeviceTime( &bxHead.ui8Time );

		BX_SendImage( &bxHead,picData,len );

		printf("input c to exist,or press other key continue...\n");
	}

	free(picData);
	BX_UnInit();
}

int main(int argc,char* argv[])
{
	if (argc <2)
	{
		printf("请输入文件名\n");
		return -1;
	}
	char *path = argv[1];
	testBX_SDK(path );
	return 0;
}