#ifndef KAFKA_PROCESS_H
#define KAFKA_PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <iconv.h>
#include "global.h"

#ifdef KAFKA_SERVER
extern "C"{
#include "rdkafka.h"
};


class KafkaProcess 
{
public:
	KafkaProcess();
	~KafkaProcess();

	bool Init(std::string brokers,std::string topic_str);
	void UnInit();
	void PushMessage(std::string strMsg);

	std::string toDecString(const char* strMsg);

private:
	rd_kafka_t *m_rk;
	char m_topic_str[1024];
};

#endif //#ifdef KAFKA_SERVER

#endif