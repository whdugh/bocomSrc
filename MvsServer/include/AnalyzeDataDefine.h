/*****************************************************
Copyright 2008-2011 Hikvision Digital Technology Co., Ltd.

FileName: AnalyzeDataDefine.h

Description: data structure for analyze data

current version: 4.0.0.4
 	 
Modification History: 2011/8/15
*****************************************************/

#ifndef _ANALYZEDATA_DEFINE_H_
#define _ANALYZEDATA_DEFINE_H_

//packet type
#define FILE_HEAD			0 
#define VIDEO_I_FRAME		1
#define VIDEO_B_FRAME		2
#define VIDEO_P_FRAME		3
#define AUDIO_PACKET		10
#define PRIVT_PACKET        11
//E frame
#define HIK_H264_E_FRAME    (1 << 6)
	    
//error code		    
#define ERROR_NOOBJECT      -1
#define ERROR_NO            0
#define ERROR_OVERBUF       1
#define ERROR_PARAMETER     2
#define ERROR_CALL_ORDER    3
#define ERROR_ALLOC_MEMORY  4
#define ERROR_OPEN_FILE     5
#define ERROR_MEMORY        11
#define ERROR_SUPPORT       12
#define ERROR_UNKNOWN       99

typedef struct _PACKET_INFO
{	
	unsigned int   dwTimeStamp;  //time stamp
	unsigned int   nYear;        //year
	unsigned int   nMonth;       //month
	unsigned int   nDay;         //day
	unsigned int   nHour;        //hour
	unsigned int   nMinute;      //minute
	unsigned int   nSecond;	     //second
	unsigned int   nPacketType;  //packet type
	unsigned int   dwPacketSize; //packet size
	unsigned char* pPacketBuffer;//packet buffer
} PACKET_INFO;

typedef struct _PACKET_INFO_EX
{
	unsigned short  uWidth;         //width
	unsigned short  uHeight;        //height
	unsigned int    dwTimeStamp;    //lower time stamp
	unsigned int    dwTimeStampHigh;//higher time stamp 
	unsigned int    nYear;	        //year
	unsigned int    nMonth;         //month
	unsigned int    nDay;           //day
	unsigned int    nHour;          //hour
	unsigned int    nMinute;        //minute
	unsigned int    nSecond;        //second
	unsigned int    dwFrameNum;     //frame num
	unsigned int    dwFrameRate;    //frame rate
	unsigned int    dwFlag;         //flag
	unsigned int    dwFilePos;      //file pos
	unsigned int	nPacketType;    //packet type
	unsigned int	dwPacketSize;   //packet size
	unsigned char*	pPacketBuffer;  //packet buffer
	unsigned int    Reserved[4];    //reserved
} PACKET_INFO_EX;

#endif