/*
 * AwsShadow.h
 *
 *  Created on: Jun 27, 2016
 *      Author: sergei
 */

#ifndef SRC_AWSSHADOW_H_
#define SRC_AWSSHADOW_H_

#include <string>
#include <mutex>
#include <unordered_map>
#include <list>
#include <memory>

#include "Log.h"
#include "AwsDeltaData.h"

typedef AwsHandler<std::string, Shadow_Ack_Status_t> AwsShadowHandler;

class AwsShadow
{
    LOG_MODULE(AwsShadow, g_main_log);

protected:
	AwsShadow();
	AwsShadow(const AwsShadow &) = delete;
	AwsShadow(AwsShadow &&) = delete;
	virtual ~AwsShadow();

public:
	static AwsShadow *init();
	static void release();
	void reset();

    bool connect(std::string deviceId, std::string thingName, std::string mqttHost, int mqttPort);
	bool is_connected();
	bool register_delta(const char *key, AwsDeltaHandler::Handler handler);
	bool update_shadow(std::string json, AwsShadowHandler::Handler handler, uint8_t timeout_sec);
	bool delete_shadow(AwsShadowHandler::Handler handler, uint8_t timeout_sec);
	bool get_shadow(AwsShadowHandler::Handler handler, uint8_t timeout_sec);

    bool yield(uint32_t timeout_ms);

private:
	static void s_deltaCallbackHandler(const char *pJsonValueBuffer, uint32_t valueLength, jsonStruct_t *pJson);
	static void s_actionCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
		const char *pReceivedJsonDocument, void *pContextData);
	static AwsShadow *s_object;

private:
	std::recursive_mutex m_mutex;
	std::unordered_map<std::string, std::shared_ptr<AwsDeltaData>> m_deltaMap;
	std::shared_ptr<AwsShadowHandler> m_getHandler;
	std::shared_ptr<AwsShadowHandler> m_updateHandler;
	std::shared_ptr<AwsShadowHandler> m_deleteHandler;
	AWS_IoT_Client m_client;
    std::string m_deviceId;
    std::string m_thinkName;
};

#endif /* SRC_AWSSHADOW_H_ */
