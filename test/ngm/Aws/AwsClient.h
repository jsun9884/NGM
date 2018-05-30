/*
 * Cloud.h
 *
 *  Created on: Jun 9, 2016
 *      Author: sergei
 */

#ifndef CLOUD_H_
#define CLOUD_H_

#include <string>
#include <mutex>
#include <unordered_map>
#include <memory>

#include "Log.h"
#include "AwsHandler.h"

extern "C" {

#include <aws_iot_mqtt_client.h>
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_config.h"

}

typedef AwsHandler<std::string, IoT_Publish_Message_Params *> AwsSubscribeHandler;

class AwsClient
{
    LOG_MODULE(AwsClient, g_main_log);

public:
	AwsClient();
	virtual ~AwsClient();
	void reset();

    bool initialize(std::string mqttHost, int mqttPort);
    bool connect(std::string deviceId);
	bool is_connected();

	bool subscribe(std::string topic, QoS qos, AwsSubscribeHandler::Handler handleData);
	bool unsubscribe(std::string topic);

	bool publish(std::string topic, QoS qos, void *data, size_t length);
	bool publish(std::string topic, QoS qos, std::string payload);

    bool yield(uint32_t timeout_ms);

private:
	void MQTTcallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
			 IoT_Publish_Message_Params *params);
	void disconnectCallbackHandler(AWS_IoT_Client* pClient);

private:
	static void s_MQTTcallbackHandler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
			 IoT_Publish_Message_Params *params, void *pData);
	static void s_disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data);

private:
	std::recursive_mutex m_mutex;
    std::recursive_mutex m_publishMutex;
    std::unordered_map<std::string, std::shared_ptr<AwsSubscribeHandler>> m_map;
	AWS_IoT_Client m_client;
};

#endif /* CLOUD_H_ */
