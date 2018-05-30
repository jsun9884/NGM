/*
 * Cloud.h
 *
 *  Created on: Jun 13, 2016
 *      Author: sergei
 */

#ifndef SRC_CLOUDMANAGER_H_
#define SRC_CLOUDMANAGER_H_

#include <atomic>
#include <list>
#include <memory>
#include "Log.h"
#include "MosquittoClient.h"
#include "AwsClient.h"
#include "AwsShadow.h"
#include "AwsS3.h"
#include "msp430.h"
#include "gps.h"
#include "GeneralDevice.h"
#include "Accelerometer.h"
#include "rtl8363.h"
#include "TempSensor.h"
#include "CpuTemperature.h"
#include "DisclosingSensor.h"
#include "LteModem.h"
#include "json.hpp"
#include "CloudCommands.h"
#include "HardwareManager.h"
#include "I2c.h"
#include "Leds.h"
#include "Uio.h"
#include "ThreadWatchdog.h"
#include "SQLite3DB.h"

struct MqttMessage
{
    MqttMessage(std::string t, std::string p);
    MqttMessage(MqttMessage&& r);
    MqttMessage() = delete;
    MqttMessage(const MqttMessage &) = delete;
    const std::string topic;
    const std::string payload;
};

enum class CfgUpdateState: int {
    None = 0,
    Success,
    Failed
};

class CloudManager
{
    LOG_MODULE(CloudMgr, g_main_log);

public:
    CloudManager();
    virtual ~CloudManager();

    void init();
    void loadConfiguration(std::string configFileName);
    void setUpgradeCommandId(std::string commandId);
    void run();
    void stop();
    void saveConfiguration(std::string configFileName);

    void setCfgUpdateState(CfgUpdateState state);

    LedsPtr getLeds();
    void setBTClientStatus(bool status);
    int getGsmRSSI();

    std::thread runThread() { return std::thread([&] { run(); }); }

protected:
    void loadDevice(TiXmlElement *elem);
    void saveDevice(DevicePtr device, TiXmlElement *param);
	void handler(std::string topic, IoT_Publish_Message_Params *params);
	void commandHandler(std::string topic, IoT_Publish_Message_Params *params);
    void debugHandler(std::string topic, IoT_Publish_Message_Params *params);
    void prepareCommand(nlohmann::json &js, bool bLocal = false);
    void prepareCommandParams(uint16_t commandCode, std::string commandId, nlohmann::json::iterator it, bool bLocal = false);
	void settingsDeltaHandler(std::string json);
	void sensorsDeltaHandler(std::string json);
    void deviceDeltaHandler(std::string key, std::string json);
	void actionHandler(std::string json, Shadow_Ack_Status_t status);
	void getHandler(std::string json, Shadow_Ack_Status_t status);
    void upgradeShadow();

	void commandProc();
    void subscribe();

    void setShadowFromJson(nlohmann::json& json);
    void updateShadow();
    void deleteShadow();

    void pereodicalPublish();
    void publishLoop();
	void shadowLoop();
    void eventsLoop();
    void commandsLoop();
    void ledsLoop();
    void lastGaspLoop();

    bool isAcc(DevicePtr dev);

    void publishCommandAcceptedMessage(std::string commandId, uint16_t commandCode, std::string state, std::string message, std::string deviceId, bool bLocal);
    void publishCommandRejectedMessage(std::string commandId, uint16_t commandCode, std::string state, std::string message, std::string deviceId, bool bLocal);

    std::string keepAliveTopic();
    std::string statusTopic();
    std::string regularTopic();
    std::string eventTopic();
    std::string commandTopic();
    std::string debugTopic();

    void createStatusMessage(nlohmann::json &js);
    void createRegularMessage(nlohmann::json &js);
    int getRegularMessageRate();
    int getKeepAliveMessageRate();

	friend class AwsClient;
    HardwareManager* m_hwMgr;

public:
    static bool setParamFromJson(nlohmann::json &js, std::string param, std::function<void(nlohmann::json::iterator &)> func);

private: 
	enum class ShadowState: int {
		INIT,
		GET,
		REG_DELTA,
		UPDATE,
		LOOP
	};
	std::atomic<ShadowState> m_shadowState;
	AwsClient m_client;
    AwsS3 m_awsS3;
	MosquittoClient m_mosquitto;
    AwsShadow *m_shadow;
    GeneralDevicePtr m_generalDevice;
    GpsPtr m_gpsDevice;
    std::list<DevicePtr> m_accDevices;
    UioPtr m_uio;

    std::list< CloudCmdPtr > m__commands;
    std::mutex m__commandsMutex;
    std::atomic<bool> m_isRunning;
    I2cPtr m_i2c0;
    LedsPtr m_leds;
    bool m_bluetoothUp;
    bool m_bluetoothClientConnected;
    bool m_gsmUp;
    bool m_wifiUp;
    bool m_gpsUp;
    std::atomic<int> m_gsmRSSI;
    std::string m_upgradeCommandId;
    CfgUpdateState m_cfgUpdateState;

    bool m_bLastGaspInterrupt;
    bool m_needUpgradeShadow;

    std::queue<MqttMessage> m_mqttMsgs;
    std::mutex m_mqttMsgsMutex;

    ThreadWatchdog m_publishWatchdog;

    SQLite3DB m_db;

    friend class MosquittoClient;
};

#endif /* SRC_CLOUDMANAGER_H_ */
