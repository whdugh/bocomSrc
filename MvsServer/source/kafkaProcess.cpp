#ifdef KAFKA_SERVER

#include "kafkaProcess.h"

#include "Common.h"

KafkaProcess::KafkaProcess()
{
	memset(m_topic_str,0x00,1024);
}

KafkaProcess::~KafkaProcess()
{

}

bool KafkaProcess::Init(std::string brokers,std::string topic_str)
{
	LogNormal("brokers:%s, topic_str:%s #", brokers.c_str(), topic_str.c_str());

	/* Create Kafka handle */
	if (!(m_rk = rd_kafka_new(RD_KAFKA_PRODUCER, brokers.c_str(), NULL))) {
		printf("kafka_new producer if failed\n");
		return false;
	}

	sprintf(m_topic_str,"%s",topic_str.c_str());

	return true;
}

void KafkaProcess::UnInit()
{
	/* Destroy the handle */
	rd_kafka_destroy(m_rk);

}

void KafkaProcess::PushMessage(std::string strMsg)
{
	/* Send/Produce message. */
	//LogNormal("m_topic_str:%s\n",m_topic_str);
	if (m_rk)
	{
		int len = strMsg.size();
		char *opbuf = (char*)malloc(len + 1);
		memcpy(opbuf, strMsg.c_str(), len);

		rd_kafka_produce(m_rk, m_topic_str, 0, RD_KAFKA_OP_F_FREE, opbuf, len);
	}
}

#endif
