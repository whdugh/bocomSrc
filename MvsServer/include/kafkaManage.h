#ifndef KAFKA_MANAGE_H
#define KAFKA_MANAGE_H
#include "global.h"
#include "kafkaProcess.h"

#ifdef KAFKA_SERVER

extern "C"{
#include "http.h"
#include "rdkafka.h"
};

class CKafakaManage{
public:
	CKafakaManage();
	~CKafakaManage();

	void GetServerList();
	bool Init();
	bool SendMessage(string strMsg);
	bool GetServerStatus(){return m_bServerConnect;};
	string GetMacID();
	string CreateKafkaMsg(RECORD_PLATE* sPlate);

private:
	void SelectServer();
	bool GetValidServer(const char* strFileName);
	bool GetIpAndPort(string strLine);
	bool IsDiffFile();
	bool CheckServerStatus(const char* strIp, int nPort);

private:
	bool m_bDownload;
	string m_strIp;
	int m_nPort;
	KafkaProcess* m_doPross;
	pthread_t m_nThreadId;
	bool m_bServerConnect;

	string m_strMacID;
	bool m_bHasMacID;
};
#endif //#ifdef KAFKA_SERVER

#endif
