/*
 * AwsShadow.cpp
 *
 *  Created on: Jun 27, 2016
 *      Author: sergei
 */

#include <thread>

#include "AwsShadow.h"

extern "C" {
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"
#include "aws_iot_config.h"
}

AwsShadow *AwsShadow::s_object = nullptr;

AwsShadow::AwsShadow()
{
}

AwsShadow::~AwsShadow()
{
}

bool AwsShadow::connect(std::string deviceId, std::string thingName, std::string mqttHost, int mqttPort)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (aws_iot_mqtt_is_client_connected(&m_client)) {
        log::warn("AwsShadowClient already connected!");
		return false;
	}

	IoT_Error_t rc = SUCCESS;
	int32_t i = 0;

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

	ShadowInitParameters_t sp = ShadowInitParametersDefault;
    sp.pHost = (char*)mqttHost.c_str();//AWS_IOT_MQTT_HOST;
    sp.port = mqttPort;//AWS_IOT_MQTT_PORT;
	sp.pClientCRT = clientCRT;
	sp.pClientKey = clientKey;
	sp.pRootCA = rootCA;
	sp.enableAutoReconnect = false;
	sp.disconnectHandler = NULL;

    log::info("Shadow Init");
	rc = aws_iot_shadow_init(&m_client, &sp);
	if (SUCCESS != rc) {
        log::error("Shadow Connection log::error");
		return false;
	}

	ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
    scp.pMyThingName = (char*)thingName.c_str();//AWS_IOT_MY_THING_NAME;
	scp.pMqttClientId = (char *)deviceId.c_str();
	scp.mqttClientIdLen = deviceId.length();

    log::info("Shadow Connect");
	rc = aws_iot_shadow_connect(&m_client, &scp);
	if (SUCCESS != rc) {
        log::error("Shadow Connection log::error");
		return false;
	}

    m_deviceId = deviceId;
    m_thinkName = thingName;
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	/*
	rc = aws_iot_shadow_set_autoreconnect_status(&m_client, true);
	if(SUCCESS != rc){
        log::error("Unable to set Auto Reconnect to true - %d", rc);
		return false;
	}
	*/

	return true;
}

bool AwsShadow::register_delta(const char *key, AwsDeltaHandler::Handler callback)
{
	IoT_Error_t rc = SUCCESS;

	std::shared_ptr<AwsDeltaData> holder = std::make_shared<AwsDeltaData>(callback);
	holder->deltaObj.pData = (void*)&holder->handler;
	holder->deltaObj.pKey = key;
	holder->deltaObj.type = SHADOW_JSON_OBJECT;
	holder->deltaObj.cb = &AwsShadow::s_deltaCallbackHandler;

	/*
	 * Register the jsonStruct object
	 */
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

    //log::info("AwsShadow::register_delta: key: %s", holder->deltaObj.pKey);
	rc = aws_iot_shadow_register_delta(&m_client, &holder->deltaObj);
	if(SUCCESS != rc){
        log::error("Unable to set Auto Reconnect to true - %d", rc);
		return false;
	}

	m_deltaMap.insert({key, holder});
	return true;
}

AwsShadow* AwsShadow::init()
{
	if (s_object == nullptr) {
		s_object = new AwsShadow();
	}
	return s_object;
}

void AwsShadow::release()
{
	if (s_object != nullptr) {
		delete s_object;
		s_object = nullptr;
	}
}

bool AwsShadow::yield(uint32_t timeout_ms)
{
    //log::info("lock %x", std::this_thread::get_id());
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
    //log::info("yeld");
	return aws_iot_shadow_yield(&m_client, timeout_ms) == SUCCESS;
}

bool AwsShadow::update_shadow(std::string json, AwsShadowHandler::Handler handler,
		uint8_t timeout_sec)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    /*if (m_updateHandler.get()) {
        log::error() << "update_shadow: handler in use...";
		return false;
    }*/

	m_updateHandler = std::make_shared<AwsShadowHandler>(handler);

    IoT_Error_t rc = aws_iot_shadow_update(&m_client, /*AWS_IOT_MY_THING_NAME*/ m_thinkName.c_str(), (char*)json.c_str(),
			&AwsShadow::s_actionCallback, (void*)this, timeout_sec, true);

	if (rc != SUCCESS) {
        m_updateHandler.reset();
        log::error() << "aws_iot_shadow_update return error: " << (int)rc;
		return false;
	}

	return true;
}

bool AwsShadow::delete_shadow(AwsShadowHandler::Handler handler,
		uint8_t timeout_sec)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    /*if (m_deleteHandler.get()) {
		return false;
    }*/

	m_deleteHandler = std::make_shared<AwsShadowHandler>(handler);

    IoT_Error_t rc = aws_iot_shadow_delete(&m_client, /*AWS_IOT_MY_THING_NAME*/ m_thinkName.c_str(),
			&AwsShadow::s_actionCallback, (void*)this, timeout_sec, true);

	if (rc != SUCCESS) {
		m_deleteHandler.reset();
		return false;
	}

	return true;
}

bool AwsShadow::get_shadow(AwsShadowHandler::Handler handler,
		uint8_t timeout_sec)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    /*if (m_getHandler.get()) {
		return false;
    }*/

	m_getHandler = std::make_shared<AwsShadowHandler>(handler);

    IoT_Error_t rc = aws_iot_shadow_get(&m_client, /*AWS_IOT_MY_THING_NAME*/ m_thinkName.c_str(),
			&AwsShadow::s_actionCallback, (void*)this, timeout_sec, true);

	if (rc != SUCCESS) {
		m_getHandler.reset();
		return false;
	}

	return true;
}

void AwsShadow::s_actionCallback(const char* pThingName, ShadowActions_t action,
		Shadow_Ack_Status_t status, const char* pReceivedJsonDocument,
		void* pContextData)
{
	AwsShadow *obj = (AwsShadow *)pContextData;
	std::shared_ptr<AwsShadowHandler> handler;

	switch(action) {
	case SHADOW_GET:
		handler.swap(obj->m_getHandler);
		break;

	case SHADOW_UPDATE:
		handler.swap(obj->m_updateHandler);
		break;

	case SHADOW_DELETE:
		handler.swap(obj->m_deleteHandler);
		break;
	}
	handler->invoke(std::string(pReceivedJsonDocument), status);
}

void AwsShadow::s_deltaCallbackHandler(const char* pJsonValueBuffer,
		uint32_t valueLength, jsonStruct_t* pJson)
{
    log::info ("DeltaCallbackHandler: %.*s", valueLength, pJsonValueBuffer);
    AwsDeltaHandler *handler = (AwsDeltaHandler *)pJson->pData;
    handler->invoke(std::string(pJson->pKey), std::string(pJsonValueBuffer, valueLength));
}

bool AwsShadow::is_connected()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return aws_iot_mqtt_is_client_connected(&m_client);
}

void AwsShadow::reset()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_deltaMap.clear();
}
