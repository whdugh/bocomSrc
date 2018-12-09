#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include <signal.h>
#include <pthread.h>
#include "FileSink.hh"
//extern static bool isRevDataNow;

class H264RtspRevDecode{

public:
	H264RtspRevDecode();

	~H264RtspRevDecode();

public:

	int RegistServer(std::string strHost,unsigned short sport,unsigned short dport,char* rgbFileName);

    int RegistServer(char* rtspUrl,char* uname,
                            char* pwd,char* psname,
                            unsigned short psport,unsigned short dport,
                            char* rgbFileName);

    
    void ShutDownNow();

	void StartRecv();

	void EndRecv();

	void RunRtsp();

	void PopOneFrameData(std::string& strData);

protected:
    void TearDownStreams();
    void CloseMediaSinks();
    void SetupStreams(bool forceSetup,char* chHost,int nPort);
    void StartPlayingStreams();
    static void SessionAfterPlaying(void* clientData = NULL);
    static void SessionTimerHandler(void* clientData);
    static void SubsessionByeHandler(void* clientData);
    Medium* createClient(UsageEnvironment& envs, int verbosityLevel,
			    char const* applicationName);
    char* getOptionsResponse(Medium* client, char const* url,
				char* username, char* password);
    char* getSDPDescriptionFromURL(Medium* client, char const* url,
				      char const* username,
				      char const* password,
				      char const* proxyServerName,
				      unsigned short proxyServerPortNum,
				      unsigned short clientStartPortNum);

    Boolean clientSetupSubsession(Medium* client,
				     MediaSubsession* subsession,
				     Boolean streamUsingTCP,bool forceSetup,char* chHost,int nPort);

    Boolean clientStartPlayingSession(Medium* client,
					 MediaSession* sessions);

    Boolean clientTearDownSession(Medium* client,
				     MediaSession* sessions);

private:
	UsageEnvironment* env;
	pthread_t m_nThreadId;
	 TaskScheduler* scheduler;
	 Medium* ourClient;
	 MediaSession* session;
	 FileSink* fileSink;

	 TaskToken sessionTimerTask;

	 struct timeval startTime;
	 double duration;
	 double initialSeekTime;
	 double scale;
	 double durationSlop;

	 unsigned short proxyServerPortNum;
	 unsigned short desiredPortNum;

	 bool isRegisted;
	 bool isRevDataNow;

};
