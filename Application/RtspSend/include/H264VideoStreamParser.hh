/**********
解析到一个NAL,送到H264FUAframgmenter中
**********/

// An abstract parser for H264 video streams
// C++ header

#ifndef _H264_VIDEO_STREAM_PARSER_HH
#define _H264_VIDEO_STREAM_PARSER_HH

#ifndef _STREAM_PARSER_HH
#include "StreamParser.hh"
#endif

#include<string>
using namespace std;

////////// H264VideoStreamParser definition //////////

#include<time.h>

#define TEST_TIME

enum H264ParseState {
  PARSING_START_SEQUENCE,
  PARSING_NAL_UNIT
};


class H264VideoStreamParser: public StreamParser {
public:
  H264VideoStreamParser(H264VideoStreamFramer* usingSource,
			FramedSource* inputSource);
  virtual ~H264VideoStreamParser();

public:
  void registerReadInterest(unsigned char* to, unsigned maxSize);

  virtual unsigned parse(int64_t &DurationTime);
  int64_t Current_Encode_Time;
  int64_t Last_Encode_Time;

  #ifdef TEST_TIME
  uint64_t lastFindTime;

  struct timeval tv;
  #endif

//protected:
 // void setParseState(H264ParseState parseState);
 // unsigned parseStartSequence();
 // unsigned parseNALUnit();



protected:
  class  H264VideoStreamFramer* fUsingSource;

  // state of the frame that's currently being read:
  unsigned char* fTo;


private: // redefined virtual functions
 // virtual void restoreSavedParserState();
  H264ParseState fCurrentParseState;

};

#endif
