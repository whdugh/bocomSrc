
// Abstract class for parsing a byte stream
// C++ header

#ifndef _STREAM_PARSER_HH
#define _STREAM_PARSER_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include<string.h>

#include<string>
#define NALNUM 20

#include<stdio.h>
#include<time.h>
//#define TEST_TIME

using namespace std;



class StreamParser {
public:
  virtual void flushInput();

  unsigned int maxNumBytesToRead;
  int nalNumber;//一帧中包含多少个nal单元；
 // unsigned int headerPosition;//一帧的当前位置(字节数)
  int haveReadNalNum;//已经读取的nal单元个数
  int nal[NALNUM]; //依次存储每个nal单元的字节数
  bool  b_first; //标记本次是rtsp服务器首次启动来获取数据
  int64_t EndEncodeTime;
#ifdef TEST_TIME
 struct timeval tv;
#endif


//protected: // we're a virtual base class
  typedef void (clientContinueFunc)(void* clientData,
				    unsigned char* ptr, unsigned size,
				    struct timeval presentationTime);
  StreamParser(FramedSource* inputSource,
	       FramedSource::onCloseFunc* onInputCloseFunc,
	       void* onInputCloseClientData,
	       clientContinueFunc* clientContinueFunc,
	       void* clientContinueClientData);
  virtual ~StreamParser();
  bool has_Call_beforeGetNAL0;


public:
     //获取数据函数
     void toGetFromEncodeList();

     bool  beforeGetNAL(int64_t &EndTime)
     {
          if(b_first == true)
          { //第一次没有数据可以解析出NAL,返回,然后从list中去获取数据
             b_first = false;
             return false;
          }

         if(has_Call_beforeGetNAL0 == true) //对于每个编码帧进入此语句块一次
         { //获取这一帧中nal单元的个数，以及每个对应的nal单元的长度
               //获取一帧中nal单元的个数
               memcpy(&nalNumber, Get_From_Encoder_Buffer, sizeof(int));

               memcpy(&EndEncodeTime,Get_From_Encoder_Buffer+sizeof(int),sizeof(int64_t));

               EndTime =  EndEncodeTime;
               //获取一帧中每个nal单元的字节数，放到数组nal[]中
               memcpy(nal+1, Get_From_Encoder_Buffer+sizeof(int)+sizeof(int64_t),(nalNumber)*sizeof(int));

               //nal个数以及各个nal大小的信息已经获取,定位实际数据开始的位置
               Get_From_Encoder_Buffer =
               Get_From_Encoder_Buffer + sizeof(int)+ sizeof(int64_t) +(nalNumber)*sizeof(int);//实际数据开始的位置

               has_Call_beforeGetNAL0 = false;
         }

                haveReadNalNum++;

                if(haveReadNalNum > nalNumber)//当前这一帧的nal单元已经发送完毕
                {
                   has_Call_beforeGetNAL0 = true;
                   return false;
                }

             printf("beforeGetNAL()--------------------\n");
      return true;

    }


     unsigned  getNAL(unsigned char* to)
	 {
          //拷贝一个nal单元（不包括起始头0x00000001）
          memcpy(to, Get_From_Encoder_Buffer+4, nal[haveReadNalNum]-4);

         //定位下一个nal单元的起始位置
          Get_From_Encoder_Buffer += nal[haveReadNalNum];
          printf("getNAL()--------------------------\n");
        #ifdef TEST_TIME
        gettimeofday(&tv, NULL);
        printf("getNAL() time using=%d---------------------\n", (tv.tv_sec*1000000+tv.tv_usec)/1000-time_begin);
        #endif

          //返回本次获取的nal单元的字节数(不包括0x00000001 头)
        return nal[haveReadNalNum]-4;

	 }



static void afterGettingBytes(void* clientData, unsigned numBytesRead,
				unsigned numTruncatedBytes,
				struct timeval presentationTime,
				unsigned durationInMicroseconds);

private:
  FramedSource* fInputSource; // should be a byte-stream source??
  FramedSource::onCloseFunc* fOnInputCloseFunc;
  void* fOnInputCloseClientData;
  clientContinueFunc* fClientContinueFunc;
  void* fClientContinueClientData;

  unsigned char *Get_From_Encoder_Buffer;
  unsigned char *Save_Encoder_Buffer;

};

#endif
