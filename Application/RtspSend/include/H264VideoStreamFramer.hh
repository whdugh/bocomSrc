#ifndef _H264_VIDEO_STREAM_FRAMER_HH
#define _H264_VIDEO_STREAM_FRAMER_HH

#ifndef _FRAMED_FILTER_HH
#include "FramedFilter.hh"
#endif


class H264VideoStreamFramer: public FramedFilter {
public:

  static H264VideoStreamFramer* createNew(UsageEnvironment& env, FramedSource* inputSource);
  virtual Boolean currentNALUnitEndsAccessUnit();
  Boolean& pictureEndMarker() { return fPictureEndMarker; }    // a hack for implementing the RTP 'M' bit

  virtual ~H264VideoStreamFramer();

protected:
  // Constructor called only by createNew(), or by subclass constructors
  H264VideoStreamFramer(UsageEnvironment& env,
                            FramedSource* inputSource,
                            Boolean createParser = True);



public:
  static void continueReadProcessing(void* clientData,
                     unsigned char* ptr, unsigned size,
                     struct timeval presentationTime);
  void continueReadProcessing();

private:
  virtual void doGetNextFrame();
  virtual Boolean isH264VideoStreamFramer() const;


protected:
  double   fFrameRate;    // Note: For MPEG-4, this is really a 'tick rate' ??
  unsigned fPictureCount; // hack used to implement doGetNextFrame() ??
  Boolean  fPictureEndMarker;
  bool firstFrame;
private:
  class H264VideoStreamParser* fParser;
  struct timeval fPresentationTimeBase;
};

#endif

