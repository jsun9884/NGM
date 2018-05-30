/*
 * Cloud.cpp
 *
 *  Created on: Jun 9, 2016
 *      Author: sergei
 */

#include "AwsClient.h"

#include <chrono>
#include <thread>

#include "string.h"
#include "unistd.h"


AwsClient::AwsClient()
{
	log::info("\nAWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
}

AwsClient::~AwsClient()
{
	log::info("\nAWS Destroyed...");
}

bool AwsClient::connect(std::string deviceId)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (aws_iot_mqtt_is_client_connected(&m_client)) {
        log::warn("AwsClient already connected!");
		return false;
	}

    IoT_Error_t rc = FAILURE;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    connectParams.keepAliveIntervalInSec = 30;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = (char *)deviceId.c_str();
	connectParams.clientIDLen = deviceId.length();
	connectParams.isWillMsgPresent = false;

	log::info("Connecting...");
	rc = aws_iot_mqtt_connect(&m_client, &connectParams);
	if(SUCCESS != rc) {
        log::error("Error(%d) connecting to cloud", rc);
		return false;
	}
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	/*
	rc = aws_iot_mqtt_autoreconnect_set_status(&m_client, true);
	if(SUCCESS != rc) {
        log::error("Unable to set Auto Reconnect to true - %d", rc);
		return false;
	}
	*/

	return true;
}

void AwsClient::MQTTcallbackHandler(AWS_IoT_Client* pClient, char* topicName,
		uint16_t topicNameLen, IoT_Publish_Message_Params* params)
{
	log::info("Subscribe callback");
	log::info("%.*s\t%.*s", topicNameLen, topicName, (int)params->payloadLen, (char*)params->payload);
}

void AwsClient::disconnectCallbackHandler(AWS_IoT_Client* pClient)
{
	IoT_Error_t rc = FAILURE;

    log::warn("MQTT Disconnect");

	/*
	if(aws_iot_is_autoreconnect_enabled(&m_client)){
		log::info("Auto Reconnect is enabled, Reconnecting attempt will start now");
	}else{
        log::warn("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect(&m_client);
		if(NETWORK_RECONNECTED == rc){
            log::warn("Manual Reconnect Successful");
		}else{
            log::warn("Manual Reconnect Failed - %d", rc);
		}
	}
	*/
}

bool AwsClient::is_connected()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return aws_iot_mqtt_is_client_connected(&m_client);
}

bool AwsClient::subscribe(std::string topic, QoS qos, AwsSubscribeHandler::Handler callback)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto handler = std::make_shared<AwsSubscribeHandler>(callback);
	if (aws_iot_mqtt_subscribe(&m_client, topic.c_str(), topic.length(), qos, &AwsClient::s_MQTTcallbackHandler, handler.get()) != SUCCESS) {
        log::error("Error subscribing: \"%s\"", topic.c_str());
		return false;
	}

	m_map.insert({topic, handler});
	return true;
}

bool AwsClient::unsubscribe(std::string topic)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (aws_iot_mqtt_unsubscribe(&m_client, topic.c_str(), topic.length()) != SUCCESS) {
        log::error("Error unsubscribing: \"%s\"", topic.c_str());
		return false;
	}

	auto it = m_map.find(topic);
	if (it != m_map.end()) {
		m_map.erase(it);
	}
	return true;
}

bool AwsClient::publish(std::string topic, QoS qos, void* data, size_t length)
{
    std::lock_guard<std::recursive_mutex> lock(m_publishMutex);
	IoT_Publish_Message_Params paramsMsg;
	paramsMsg.qos = qos;
	paramsMsg.payload = data;
	paramsMsg.payloadLen = length;

    //std::chrono::system_clock::time_point t_start = std::chrono::system_clock::now();
    //log::info() << "publishing " << topic;
    bool bSuccess = aws_iot_mqtt_publish(&m_client, topic.c_str(), topic.length(), &paramsMsg) == SUCCESS;
    //size_t dur = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t_start).count();

    if (!bSuccess) {
        log::error() << "Error publishing: \"" << topic << "\"";// duration: " << dur << " usec";
		return false;
	}
    //log::info() << "publish \"" << topic << "\" success...";// duration: " << dur << " usec";
	return true;
}

bool AwsClient::publish(std::string topic, QoS qos, std::string payload)
{
	return publish(topic, qos, (void *)payload.c_str(), payload.length());
}

bool AwsClient::yield(uint32_t timeout_ms)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return aws_iot_mqtt_yield(&m_client, timeout_ms) == SUCCESS;
}

void AwsClient::reset()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_map.clear();
}

bool AwsClient::initialize(std::string mqttHost, int mqttPort)
{
    IoT_Error_t rc = FAILURE;
    int32_t i = 0;

    m_map.clear();
    //uint32_t publishCount = 0;
    //bool infinitePublishFlag = true;

    //char HostAddress[255] = AWS_IOT_MQTT_HOST;
    //uint32_t port = AWS_IOT_MQTT_PORT;
    char certDirectory[PATH_MAX + 1] = /*"/home/root"; //*/ "./certs";
    char rootCA[PATH_MAX + 1];
    char clientCRT[PATH_MAX + 1];
    char clientKey[PATH_MAX + 1];

    snprintf(rootCA, PATH_MAX + 1, "%s/%s", certDirectory, AWS_IOT_ROOT_CA_FILENAME);
    snprintf(clientCRT, PATH_MAX + 1, "%s/%s", certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
    snprintf(clientKey, PATH_MAX + 1, "%s/%s", certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

    log::debug("rootCA %s", rootCA);
    log::debug("clientCRT %s", clientCRT);
    log::debug("clientKey %s", clientKey);

    IoT_Client_Init_Params mqttInitParams;
    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = (char*)mqttHost.c_str(); // HostAddress;
    mqttInitParams.port = mqttPort; //port;
    mqttInitParams.pRootCALocation = rootCA;
    mqttInitParams.pDeviceCertLocation = clientCRT;
    mqttInitParams.pDevicePrivateKeyLocation = clientKey;
    mqttInitParams.mqttCommandTimeout_ms = 2000;
    mqttInitParams.tlsHandshakeTimeout_ms = 2000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = NULL;//&AwsClient::s_disconnectCallbackHandler;
    //mqttInitParams.disconnectHandlerData = (void *)this;

    memset(&m_client, 0, sizeof(AWS_IoT_Client));
    rc = aws_iot_mqtt_init(&m_client, &mqttInitParams);
    if(SUCCESS != rc) {
        log::error("aws_iot_mqtt_init returned error : %d ", rc);
        return false;
    }
    return true;
}

void AwsClient::s_disconnectCallbackHandler(AWS_IoT_Client* pClient, void* data)
{
	if (data == nullptr) {
		return;
	}

	AwsClient *obj = (AwsClient *)data;
	obj->disconnectCallbackHandler(pClient);
}

void AwsClient::s_MQTTcallbackHandler(AWS_IoT_Client* pClient, char* topicName,
		uint16_t topicNameLen, IoT_Publish_Message_Params* params,
		void* pData)
{
    log::debug ("CallbackHandler: topic = %.*s", topicNameLen, topicName);
    if (pData) {
        AwsHandler<std::string, IoT_Publish_Message_Params *> *obj = (AwsHandler<std::string, IoT_Publish_Message_Params *> *)pData;
        obj->invoke(std::string(topicName, topicNameLen), params);
    }
    else {
        log::info() << "CallbackHandler without pData (handler)...";
    }
}
