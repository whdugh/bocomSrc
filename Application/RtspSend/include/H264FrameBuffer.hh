#ifndef H264_FRAME_BUFFER_HH
#define H264_FRAME_BUFFER_HH

#include <stdlib.h>
#include <string>
#include <list>
using namespace std;

typedef unsigned char uint8_t;

class H264FrameBuffer {

public:
    H264FrameBuffer();
    void AddNalData(uint8_t* data,int dSize);
    bool PopOneFrameData(std::string& strData);
    void ResetBufferSize(int bufSize);
    ~H264FrameBuffer();

private:
  //  uint8_t* m_FrameBuffer;
    uint8_t* m_LastNal;
    int m_FrameBufferSize;
    int m_LastNalSize;
    int m_FrameBufferMaxSize;
    int m_LastNalMaxSize;
    bool m_isGetOneFrame;
    bool m_hasIDR;
    bool m_hasPPS;
    bool m_hasSPS;
    int m_nalNumber;
    bool m_isBeginSave;

	std::list<std::string> m_listStrData;
	//¶ÁÐ´ÐÅºÅ»¥³â
	pthread_mutex_t m_FrameMutex;
};

#endif
