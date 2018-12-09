
#ifndef H264_STREAMER_CPP
#define H264_STREAMER_CPP


#include <iostream>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"

//header about list
#include <string>
#include <list>
#include <pthread.h>


void afterPlaying(void *) ;

class LiveRTSPH264
{
 public:
      LiveRTSPH264();
	 ~LiveRTSPH264();
      void RtspH264Init();
      void play();
      void RunRtsp();
     // void addFrame(unsigned char *srcBuffer, int length);
      void addFrame(unsigned char *srcBuffer, int length, int *nalAray, int nalNum,int64_t end_encode_time);
      void RtspH264UnInit();


      friend void afterPlaying(void*);
      UsageEnvironment* env;
      char const* inputFileName;
      H264VideoStreamFramer* videoSource;
      RTPSink* videoSink;
      ByteStreamFileSource* fileSource;
      TaskScheduler* scheduler;
      Groupsock* rtpGroupsock;
      Groupsock* rtcpGroupsock;
      RTCPInstance* rtcp;
      RTSPServer* rtspServer;
      ServerMediaSession* sms;


      pthread_t m_nThreadId;
      //pthread_mutex_t m_video_Mutex;
};


#endif
