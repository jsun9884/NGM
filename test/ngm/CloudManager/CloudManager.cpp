/*
 * CloudManager.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: sergei
 */

#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <functional>
#include <random>
#include <fstream>

#include <unistd.h>
#include <syscall.h>

#include "CloudCommands.h"
#include "CloudManager.h"
#include "AwsHandler.h"
#include "PeriodicalAction.h"
#include "json.h"
#include "json.hpp"
#include "json_fixed.hpp"
#include "Event.h"
#include "HardwareManager.h"
#include "Version.h"
#include "NetUtils.h"
#include "SimpleIni.h"

#define SET_CFG_CMD_ID_FILE ".set_cfg_commandId"

CloudManager::CloudManager():
    m_shadowState(ShadowState::INIT),
    m_mosquitto(this),
    m_shadow(AwsShadow::init()),
    //m_cpuTemp("/sys/class/hwmon/hwmon0/temp1_input"),
    m_isRunning(false),
    //m__awsClientID(AWS_IOT_MQTT_CLIENT_ID),
    m_i2c0(new I2c()),
    m_bluetoothUp(true),
    m_bluetoothClientConnected(false),
    m_gsmUp(false),
    m_gpsUp(false),
    m_wifiUp(false),
    m_gsmRSSI(0),
    m_cfgUpdateState(CfgUpdateState::None),
    m_bLastGaspInterrupt(false),
    m_needUpgradeShadow(false),
    m_db(DATABASE_FILENAME, "c12ngm-data-state-common-create.sql")
{
    log::info() << "Creating CloudManager...";

    if (m_shadowState.is_lock_free()) {
        log::info() << "std::atomic<ShadowState> is lock free!!!";
    }
    else {
        log::warn() << "std::atomic<ShadowState> is not lock free!!!";
    }
    m_i2c0->open(0);

    m_leds = std::make_shared<Leds>(m_i2c0, 0x32);
    m_leds->initialize();

    {
        std::ifstream f;
        f.open("/home/linaro/.boot_attr");
        if (f.is_open()) {
            f.close();
            m_leds->setLedOn(0, 0xff, 0xff, 0xff);
            m_leds->setLedOn(1, 0xff, 0xff, 0xff);
            m_leds->setLedOn(2, 0xff, 0xff, 0xff);

            ::system("rm /home/linaro/.boot_attr");
            std::this_thread::sleep_for(std::chrono::milliseconds(700));

            m_leds->setLedOff(0);//On(0, 0x0f, 0x01, 0x01);
            m_leds->setLedOff(1);//On(1, 0x01, 0x0f, 0x01);
            m_leds->setLedOff(2);//On(2, 0x01, 0x01, 0x1f);
        }
    }

    m_hwMgr = HardwareManager::instance();
    m_uio = std::make_shared<Uio>(0);

    // Initialize General Device
    //m_generalDevice = std::make_shared<GeneralDevice>("general");
    //m_hwMgr->addDevice(m_generalDevice);
}

CloudManager::~CloudManager()
{
    AwsShadow::release();
    delete m_hwMgr;
}

std::string CloudManager::keepAliveTopic()     { return "iot/device/" + m_generalDevice->getDeviceId() + "/msgType/ping/fmt/json";}
std::string CloudManager::statusTopic()     { return "iot/device/" + m_generalDevice->getDeviceId() + "/msgType/status/fmt/json";}
std::string CloudManager::regularTopic()    { return "iot/device/" + m_generalDevice->getDeviceId() + "/msgType/regular/fmt/json";}
std::string CloudManager::eventTopic()      { return "iot/device/" + m_generalDevice->getDeviceId() + "/msgType/event/fmt/json";}
std::string CloudManager::commandTopic()    { return "iot/device/" + m_generalDevice->getDeviceId() + "/command";}
std::string CloudManager::debugTopic()      { return "iot/device/" + m_generalDevice->getDeviceId() + "/debug";   }

void CloudManager::loadConfiguration(std::string configFileName) {
    try {
        TiXmlDocument doc(configFileName);
        if (!doc.LoadFile()) {
            THROW(CantLoadParametersXmlEx);
        }

        TiXmlElement* elem = doc.FirstChildElement();

        for (elem; elem; elem = elem->NextSiblingElement()) {
            std::string tag = elem->Value();
            if (tag == "Device")
                loadDevice(elem);
            else {
                THROWEX(UnknownXmlSectionEx,tag);
            }
        }
    }
    catch(CantLoadParametersXmlEx& ex) {
        throw ex;
    }
    catch(std::exception& ex) {
        glog::error() << "*** Exception: " << ex.what();
        throw ex;
    }

    HardwareManager::instance()->setSaveConfigFunc([&, configFileName](){
        saveConfiguration(configFileName);
    });

    m_db.readMap(m_hwMgr);
}

void CloudManager::setUpgradeCommandId(std::string commandId)
{
    m_upgradeCommandId = commandId;
}

void CloudManager::loadDevice(TiXmlElement* elem) {
    std::string name = elem->Attribute("name");
    std::string type = elem->Attribute("type");

    glog::info() << "Loading configuration for " << type << "(" << name << ")";

    DevicePtr device;
    if (type == "msp430") {
        device = std::make_shared<Msp430>(name, "/dev/ttymxc3");
    }
    else if (type == "gps") {
        std::string devPath = elem->Attribute("devPath");
        m_gpsDevice = std::make_shared<Gps>(name, devPath, m_gsmUp, m_gpsUp, m_gsmRSSI);
        device = m_gpsDevice;
    }
    else if (type == "general") {
        device = std::make_shared<GeneralDevice>(name);
        m_generalDevice = std::static_pointer_cast<GeneralDevice>(device);
    }
    else if (type == "accelerometer") {
        std::string devPath = elem->Attribute("devPath");
        device = std::make_shared<Accelerometer>(name, devPath, false);
        m_accDevices.push_back(device);
    }
    else if (type == "accelerometer_inverted") {
        std::string devPath = elem->Attribute("devPath");
        device = std::make_shared<Accelerometer>(name, devPath, true);
        m_accDevices.push_back(device);
    }
    else if (type == "rtl8363") {
        device = std::make_shared<Rtl8363>(name);
    }
    else if (type == "temperature") {
        device = std::make_shared<TempSensor>(name, m_i2c0, 0x48);
    }
    else if (type == "cpu_temp") {
        device = std::make_shared<CpuTemperature>(name, "/sys/class/hwmon/hwmon0/temp1_input");
    }
    else if (type == "disclosing_sensor") {
        device = std::make_shared<DisclosingSensor>(name, m_leds);
    }
    else if (type == "lte_modem") {
        device = std::make_shared<LteModem>(name);
    }
    else {
        THROWEX(UnknownXmlDeviceEx, type);
    }
    log::info() << "-- preInit";
    device->preInit();
    log::info() << "-- initialize";
    device->initialize();
    log::info() << "-- LoadConfiguration";
    device->loadConfiguration(elem);
    log::info() << "-- addDevice";
    m_hwMgr->addDevice(device);
    log::info() << "-- ApplyGeneralParameters";
    device->applyGeneralParams(true);
    log::info() << "-- Done";
}

void CloudManager::saveConfiguration(std::string configFileName) {

    try{
        log::info() << "saveConfiguration: " << configFileName;

        TiXmlDocument doc(configFileName);
        if (!doc.LoadFile()) {
            THROW(CantLoadParametersXmlEx);
        }
        TiXmlHandle hDoc(&doc);
        TiXmlElement* elem;
        TiXmlHandle root(0);

        elem = hDoc.FirstChildElement().Element();
        if (!elem) return;
        for (elem; elem; elem = elem->NextSiblingElement()) {
            std::string deviceName = elem->FirstAttribute()->ValueStr();
            DevicePtr device = std::static_pointer_cast<Device>(m_hwMgr->getDevice(deviceName));
            if(device){
               saveDevice(device, elem->FirstChildElement());
            }

        }
        doc.SaveFile();
        system("sync");
    }
    catch(std::exception& ex){
        log::info() << ex.what();
        throw ex;
    }
}

void CloudManager::setCfgUpdateState(CfgUpdateState state)
{
    m_cfgUpdateState = state;
}

LedsPtr CloudManager::getLeds()
{
    return m_leds;
}

void CloudManager::setBTClientStatus(bool status)
{
    m_bluetoothClientConnected = status;
}

int CloudManager::getGsmRSSI()
{
    return m_gsmRSSI.load();
}

void CloudManager::saveDevice(DevicePtr device, TiXmlElement* param) {
    for(param; param; param = param->NextSiblingElement()) {
        device->saveConfiguration(param);
    }
}

void CloudManager::createStatusMessage(nlohmann::json &js)
{
    uint64_t timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    js["deviceId"] = m_generalDevice->getDeviceId();
    js["FWVersion"] = FW_VERSION;
    js["GsmRSSI"] = m_gsmRSSI.load();
    js["timestamp"] = timestamp_ms;
}

void CloudManager::createRegularMessage(nlohmann::json &js)
{
    uint64_t timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    js["deviceId"] = m_generalDevice->getDeviceId();
    js["timestamp"] = timestamp_ms;
}

void CloudManager::run()
{
    bool bConnected = false, bPrevConnected = false;

    if (m_isRunning) {
        log::error("CloudManager for %s already running!");
        return;
    }

    log::info() << "Starting CloudManager::run loop TPID: " << ::syscall(SYS_gettid);

    if (!m_generalDevice) {
        log::warn() << "GeneralDevice is not initialized, using default parameters...";
        DevicePtr device = std::make_shared<GeneralDevice>("General");
        device->initialize();
        m_hwMgr->addDevice(device);
        m_generalDevice = std::static_pointer_cast<GeneralDevice>(device);
    }

    m_generalDevice->loadGsmSettings();
    //m_generalDevice->loadWifiSettings();

    m_isRunning = true;

    m_hwMgr->startAll();

    std::thread shadowThread(&CloudManager::shadowLoop, this);
    std::thread highResThread(&CloudManager::eventsLoop, this);
    std::thread publishThread(&CloudManager::publishLoop, this);
    std::thread commandsThread(&CloudManager::commandsLoop, this);
    std::thread ledsThread(&CloudManager::ledsLoop, this);
    std::thread lastGaspThread(&CloudManager::lastGaspLoop, this);

    m_db.start();

    int mret = 0;

    //log::info() << "*** exec test:\n" << GeneralDevice::exec("nmcli c");

    while (m_isRunning) {
        //log::debug() << DBG_INFO;
        if (m_publishWatchdog.secElapsed() > 10) {
            log::error() << "Publish Thread Is Freezed by " << m_publishWatchdog.msecElapsed()
                         << " msec... Restarting application...";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            system("sync && killall ngm");
        }

        if (bConnected) {
            mret = m_mosquitto.loop(300, 10);
            if ((mret == MOSQ_ERR_NO_CONN) || (mret == MOSQ_ERR_CONN_LOST)) {
                bConnected = false;
            }
            else {
                bConnected = true;
            }
        }

        if (bConnected) {
            if (!bPrevConnected) {
                glog::info() << "Mosquitto Client connected!...";
            }
        }
        else{
            if (bPrevConnected) {
                glog::warn() << "Mosquitto not connected...";
            }

            log::info("Mosquitto Client connecting....");
            if( m_mosquitto.connect("localhost", 1883, 60) == MOSQ_ERR_SUCCESS ) {
                bConnected = true;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        bPrevConnected = bConnected;
    }

    m_db.stop();

    shadowThread.join();
    highResThread.join();
    publishThread.join();
    commandsThread.join();
    ledsThread.join();
    lastGaspThread.join();

    m_hwMgr->stopAll();
}

void CloudManager::stop() {
    glog::info() << "*** Stopping NGM Services. Please wait... ***";
    m_isRunning = false;
}

void CloudManager::subscribe() {
    /*
    log::info("Subscribing to \"%s\":%ld", statusTopic.c_str(), statusTopic.length());
    if (!m_client.subscribe(statusTopic, QOS0, std::bind(&CloudManager::handler, this, std::placeholders::_1, std::placeholders::_2))) {
        log::error("subscribe failed: %s", statusTopic.c_str());
    }

    log::info("Subscribing to \"%s\":%ld", regularTopic.c_str(), regularTopic.length());
    if (!m_client.subscribe(regularTopic, QOS0, std::bind(&CloudManager::handler, this, std::placeholders::_1, std::placeholders::_2))) {
        log::error("subscribe failed: %s", regularTopic.c_str());
    }
    */
    const std::string cmdTopic = commandTopic();
    log::info("Subscribing to \"%s\":%ld", cmdTopic.c_str(), cmdTopic.length());
    if (!m_client.subscribe(cmdTopic, QOS0, std::bind(&CloudManager::commandHandler, this, std::placeholders::_1, std::placeholders::_2))) {
        log::error("subscribe failed: %s", cmdTopic.c_str());
    }

    log::info() << "Subscribing to " << debugTopic();
    if (!m_client.subscribe(debugTopic(), QOS0, std::bind(&CloudManager::debugHandler, this, std::placeholders::_1, std::placeholders::_2))) {
        log::error() << "subscribe failed: " << debugTopic();
    }
}

void CloudManager::pereodicalPublish()
{
    static int cloudRateCounter = -1;
    static int cloudKeepAliveCounter = -1;

    nlohmann::json statusJs, regularJs;
    //log::info() << "@ regular yield start...";

    ++cloudRateCounter;
    ++cloudKeepAliveCounter;
    createStatusMessage(statusJs);
    createRegularMessage(regularJs);

    //log::info() << "@ regular messages created";
    {
        nlohmann::json js_arr_s = nlohmann::json::array();
        nlohmann::json js_arr_r = nlohmann::json::array();
        for(int i=0; i < m_hwMgr->getDevicesCount(); i++) {
            nlohmann::json js_s, js_r;
            DevicePtr device = std::static_pointer_cast<Device>(m_hwMgr->getDevice(i));

            device->addStatusData(js_s);
            device->addRegularData(js_r);

            if(!js_s.empty())
                js_arr_s.push_back(js_s);

            if(!js_r.empty())
                js_arr_r.push_back(js_r);
        }

        statusJs["payload"]["sensors"]  = js_arr_s;
        regularJs["payload"]["sensors"] = js_arr_r;
    }

    int regularMessageRate = getRegularMessageRate();
    std::string statusStr, regularStr;
    {
        std::stringstream ss;
        ss.precision(3);
        ss << std::fixed << statusJs;
        statusStr = ss.str();
    }

    {
        std::stringstream ss;
        ss.precision(3);
        ss << std::fixed << regularJs;
        regularStr = ss.str();
    }

    if ((!cloudRateCounter) || (cloudRateCounter >= regularMessageRate)) {
        cloudRateCounter = 0;

        //log::info() << "@ regular publish start (2 messages)";
        m_client.publish(statusTopic(), (regularMessageRate > 60 ? QOS1 : QOS0), statusStr);
        m_publishWatchdog.kick();

        m_client.publish(regularTopic(), (regularMessageRate > 60 ? QOS1 : QOS0), regularStr);
        m_publishWatchdog.kick();
        //log::info() << "@ regular publish finished, status size = " << statusStr.length() << ", regular size = " << regularStr.length();

        //log::info() << "publish regular: " << regularStr;
    }

    //if (regularMessageRate > 60) {
        if ((!cloudKeepAliveCounter) || (cloudKeepAliveCounter >= getKeepAliveMessageRate())) {
            cloudKeepAliveCounter = 0;
            m_client.publish(keepAliveTopic(), QOS0, "{}");
            m_publishWatchdog.kick();
            //log::info() << "publish: keepAlive...";
        }
    //}

    //log::info() << "@ regular Mosquitto publish start";
    m_mosquitto.publish(NULL, "device/regular", regularStr.length(), regularStr.c_str());
    m_mosquitto.publish(NULL, "device/status", statusStr.length(), statusStr.c_str());
    m_publishWatchdog.kick();
    //log::info() << "@ regular Mosquitto publish finished";

    //log::info() << "@ regular finished";
}

void CloudManager::publishLoop() {


    log::info() << "publishLoop started, TPID: " << ::syscall(SYS_gettid);

    bool bPrevConnected = false;
    bool bConnected = false;
    std::chrono::milliseconds regularDuration = std::chrono::milliseconds(1000);
    std::chrono::system_clock::time_point regularLastPoint = std::chrono::system_clock::now() - regularDuration;

    std::queue<MqttMessage> mqttMsgs;

    /*{
        int ct = 3;
        for (; ct > 0; --ct) {
            if (m_client.initialize(m_generalDevice->getMqttHost(), m_generalDevice->getMqttPort())) {
                break;
            }
        }
        if (ct == 0) {
            THROW(AwsClientInitializationFailedEx);
        }
    }*/

    while (m_isRunning) {
        m_publishWatchdog.kick();

        bConnected = m_client.is_connected();
        if (bConnected) {
            if (!bPrevConnected) {
                glog::info("AWS Client is connected!...");
                subscribe();
                if (m_upgradeCommandId.size()) {
                    publishCommandAcceptedMessage(m_upgradeCommandId, (uint16_t)CloudCommands::FW_UPGRADE, "DONE", "Upgrade successful", m_generalDevice->getDeviceId(), false);
                    publishCommandAcceptedMessage(m_upgradeCommandId, (uint16_t)CloudCommands::FW_UPGRADE, "DONE", "Upgrade successful", m_generalDevice->getDeviceId(), true);
                    m_upgradeCommandId = "";
                }

                if (m_cfgUpdateState != CfgUpdateState::None) {
                    std::ifstream idf;
                    idf.open(SET_CFG_CMD_ID_FILE);
                    if (idf.is_open()) {
                        std::string commandId;
                        idf >> commandId;
                        log::info() << "SET_CFG_FILE CommandID readed from file  <" << SET_CFG_CMD_ID_FILE << ">: " << commandId;
                        std::string status, msg;
                        if (m_cfgUpdateState == CfgUpdateState::Success) {
                            status = "DONE";
                            msg = "Update successful";
                        }
                        else if (m_cfgUpdateState == CfgUpdateState::Failed) {
                            status = "REJECTED";
                            msg = "Wrong configuration file";
                        }
                        publishCommandAcceptedMessage(commandId, (uint16_t)CloudCommands::SET_CFG_FILE, status, msg, m_generalDevice->getDeviceId(), false);
                        publishCommandAcceptedMessage(commandId, (uint16_t)CloudCommands::SET_CFG_FILE, status, msg, m_generalDevice->getDeviceId(), true);
                        idf.close();
                        std::remove(SET_CFG_CMD_ID_FILE);
                    }
                    else {
                        log::error() << "Can't open file <" << SET_CFG_CMD_ID_FILE << "> for read";
                    }

                    m_cfgUpdateState = CfgUpdateState::None;
                }
            }
        }
        else{
            if (bPrevConnected) {
                glog::error("AWS Client disconnected...");
            }

            log::info("AWS Client connecting....");
            m_client.initialize(m_generalDevice->getMqttHost(), m_generalDevice->getMqttPort());
            m_client.connect(/*AWS_IOT_MQTT_CLIENT_ID*/m_generalDevice->getDeviceId());
        }

        bPrevConnected = bConnected;
        if (!bConnected) {
            m_publishWatchdog.kick();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        m_client.yield(50);
        m_publishWatchdog.kick();
        /*{
            // Not need now
            int msgRate = getRegularMessageRate() * 1000;
            if (msgRate != regularMsg.duration()) {
                regularMsg.set_duration(msgRate);
            }
        }*/
        if (m_gpsDevice) {
            bool bGpsEnabled = m_generalDevice->getGpsEnabled();
            if (bGpsEnabled != m_gpsDevice->gpsIsUp()) {
                if (bGpsEnabled) {
                    m_gpsDevice->startGps();
                }
                else {
                    m_gpsDevice->stopGps();
                }
            }
        }

        if ((std::chrono::system_clock::now() - regularLastPoint) >= regularDuration)
        {
            regularLastPoint = std::chrono::system_clock::now();
            pereodicalPublish();
        }

        {
            std::lock_guard<std::mutex> l(m_mqttMsgsMutex);
            std::swap(mqttMsgs, m_mqttMsgs);
        }

        while (mqttMsgs.size()) {
            MqttMessage m(std::move(mqttMsgs.front()));
            mqttMsgs.pop();

            m_publishWatchdog.kick();
            log::info() << "AWS: publish message...";
            m_client.publish(m.topic, QOS0, m.payload);

            if (!m_client.is_connected()) {
                std::lock_guard<std::mutex> l(m_mqttMsgsMutex);
                m_mqttMsgs.push(std::move(m));
                while(mqttMsgs.size()) {
                    log::info() << "AWS: pushing back messages to m_mqttMsgs... (client disconnected...)";
                    m_mqttMsgs.push(std::move(mqttMsgs.front()));
                    mqttMsgs.pop();
                }
            }
        }
    }
}
int CloudManager::getRegularMessageRate(){
    return m_generalDevice->getRegularMsgRate();
}

int CloudManager::getKeepAliveMessageRate()
{
    return m_generalDevice->getKeepAliveMsgRate();
}
void CloudManager::handler(std::string topic, IoT_Publish_Message_Params* params) {
    log::info("publish callback: %s\t\'%.*s\'", topic.c_str(), (int)params->payloadLen, (char *)params->payload);
}

void CloudManager::shadowLoop() {
    log::info() << "Shadow Loop started, TPID: " << ::syscall(SYS_gettid);
    bool bConnected = false, bPrevConnected = false;

    while (m_isRunning) {
        //log::debug() << DBG_INFO;
        bConnected = m_shadow->is_connected();
        if (!bConnected) {
            if (bPrevConnected) {
                glog::error("AWS Shadow disconnected!!!");
            }
            m_shadow->reset();

            if (m_shadow->connect(m_generalDevice->getDeviceId() + "-Shadow", m_generalDevice->getDeviceId(), m_generalDevice->getMqttHost(), m_generalDevice->getMqttPort())) {
                for(int i=0; i < m_hwMgr->getDevicesCount(); i++){

                    if (!m_shadow->register_delta(m_hwMgr->getDevice(i)->getName().c_str(), std::bind(&CloudManager::deviceDeltaHandler, this, std::placeholders::_1, std::placeholders::_2))) {
                        log::error("Shadow: register delta %s failed...", m_hwMgr->getDevice(i)->getName().c_str());
                    }
                    else {
                        log::info("register_delta %s successful!", m_hwMgr->getDevice(i)->getName().c_str());
                    }
                }
                deleteShadow();
                updateShadow();
            }
            else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        else {
            if (!bPrevConnected) {
                glog::info("AWS Shadow is connected!");
            }
            if (m_needUpgradeShadow) {
                m_needUpgradeShadow = false;
                deleteShadow();
                updateShadow();
            }
            m_shadow->yield(300);
        }
        bPrevConnected = bConnected;
    }

    log::info ("Shadow Loop stopped!");
}

void CloudManager::deviceDeltaHandler(std::string key, std::string json)
{
    try {
        log::info() << "Shadow delta: " << json;
        nlohmann::json js = nlohmann::json::parse(json);
        std::static_pointer_cast<Device>(m_hwMgr->getDevice(key))->setShadowFromJson(js);
        updateShadow();
        saveConfiguration("Configuration.xml");
    }
    catch(std::exception e){
        log::warn() << "json parsing error...";
    }
}

void CloudManager::actionHandler(std::string json, Shadow_Ack_Status_t status)
{
    log::info("Cloud: Shadow Action callback, status: %d", status);
    log::info("json: %s", json.c_str());
}

void CloudManager::getHandler(std::string json, Shadow_Ack_Status_t status)
{
    log::info("Cloud: Shadow GET callback, status: %d", status);
    log::info("json: %s", json.c_str());

    try {
        nlohmann::json js = nlohmann::json::parse(json);
        setShadowFromJson(js["state"]["desired"]);
    }
    catch(...) {
        log::warn("json parsing failed...");
    }
}

void CloudManager::upgradeShadow()
{
    m_needUpgradeShadow = true;
}

void CloudManager::setShadowFromJson(nlohmann::json& js)
{
    bool bChanaged = false;
    for(int i=0; i < m_hwMgr->getDevicesCount(); i++){
        auto it = js.find(m_hwMgr->getDevice(i)->getName());
        if (it != js.end()) {
            try {
                std::static_pointer_cast<Device>(m_hwMgr->getDevice(i))->setShadowFromJson(*it);
                bChanaged = true;
            }
            catch(...){

            }
        }
    }
    if (bChanaged)
        saveConfiguration("Configuration.xml");
}


void CloudManager::updateShadow() {
    if (m_shadow->is_connected()) {
        nlohmann_fixed::json js_out;
        nlohmann_fixed::json js_s;
        for(int i=0; i < m_hwMgr->getDevicesCount(); i++) {
            std::static_pointer_cast<Device>(m_hwMgr->getDevice(i))->addShadowData(js_s);
        }

        js_out["state"]["reported"] = js_s;

        std::stringstream ss;
        ss.precision(3);
        ss << std::fixed << js_out;

        log::info() << "Upgrade shadow: " << ss.str();
        if (!m_shadow->update_shadow(ss.str(),
            std::bind(&CloudManager::actionHandler, this, std::placeholders::_1,
            std::placeholders::_2), 5))
        {
            log::error("shadow update failed...");
        }
        else {
            log::info() << "update shadow success!";
        }
    }
}
void CloudManager::deleteShadow() {
    if (m_shadow->is_connected()) {
        if (!m_shadow->delete_shadow(std::bind(&CloudManager::actionHandler, this, std::placeholders::_1,
        std::placeholders::_2), 5)) { log::error("shadow delete failed...");
        }
        else {
            log::info("delete shadow");
        }
    }
}

void CloudManager::commandHandler(std::string topic, IoT_Publish_Message_Params* params) {

    log::info("<aws> Command callback...");
    log::info("json: %.*s", params->payloadLen, (const char*)params->payload);

    try {
        nlohmann::json js = nlohmann::json::parse(std::string((const char*)params->payload, params->payloadLen));

        prepareCommand(js, false);
    }
    catch(std::string &e) {
        log::warn("json parsing error: %s", e.c_str());
        return;
    }
    catch(...) {
        log::warn("prepareCommand error...");
        return;
    }
}

void CloudManager::debugHandler(std::string topic, IoT_Publish_Message_Params *params)
{
    log::info("<aws> Debug callback...");
    log::info("json: %.*s", params->payloadLen, (const char*)params->payload);

    std::function<void()> func = [&]() {
        ::system("zip -9 logs.zip logs/* WebUI/server/*.log");
        std::string key = m_generalDevice->getDeviceId() + "-logs.zip";

        if (m_awsS3.Upload("logs.zip", key.c_str(), "ngm-debug", 0, asFile)) {
            log::info() << "Logs uploaded to S3 seerver successfuly!";
        }
        else {
            log::error() << "upload logs to S3 failed...";
        }
    };

    std::thread t(func);
    t.detach();
}

void CloudManager::prepareCommand(nlohmann::json &js, bool bLocal)
{
    int commandCode = 0;
    std::string commandId;
    std::string deviceId;

    nlohmann::json::iterator it = js.find("commandId");
    if (it == js.end()) {
        throw ("\'commandId' parsing error");
    }
    commandId = *it;

    it = js.find("commandCode");
    if (it == js.end()) {
        throw ("\'commandCode' parsing error");
    }
    commandCode = *it;

    it = js.find("params");
    if (it == js.end()) {
        publishCommandRejectedMessage(commandId, commandCode, "REJECTED", "json: \'params\' parsing error", m_generalDevice->getDeviceId(), bLocal);
        throw ("\'params' parsing error");
    }

    try {
        prepareCommandParams(commandCode, commandId, it, bLocal);
    }
    catch(...) {
        publishCommandRejectedMessage(commandId, commandCode, "REJECTED", "Runtime error", m_generalDevice->getDeviceId(), bLocal);
    }
}

void CloudManager::prepareCommandParams(uint16_t commandCode, std::string commandId, nlohmann::json::iterator it, bool bLocal)
{
    switch (CloudCommands(commandCode)) {

    case CloudCommands::FW_UPGRADE:
        log::info(">>> FW_UPGRADE command");
        {
            std::string url;
            auto pit = it->find("url");
            if (pit == it->end()) {
                publishCommandRejectedMessage(commandId, commandCode, "REJECTED", "<url> not found in json", m_generalDevice->getDeviceId(), bLocal);
            }
            else {
                url = *pit;
                CloudCmdPtr command = std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [url, commandId, commandCode, bLocal, this](CloudCmd &o) {
                    publishCommandAcceptedMessage(commandId, commandCode, "ACCEPTED", "Download", m_generalDevice->getDeviceId(), bLocal);
                    std::string bash_cmd = "cd ./upgrade/ && ./download.sh \"" + url + "\"";
                    if (system(bash_cmd.c_str()) == 0) {
                        std::string saveCmdIdBashCmd = "echo \"" + o.commandId + "\" > /root/upgrade/commandId";
                        system(saveCmdIdBashCmd.c_str());

                        publishCommandAcceptedMessage(commandId, commandCode, "PROGRESS", "Rebooting", m_generalDevice->getDeviceId(), bLocal);
                        system("sync");
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        system("reboot");
                    }
                    else {
                        o.state = CloudCmd::REJECTED;
                        o.message = "Download or unpack failed";
                    }
                });

                std::unique_lock<std::mutex> l(m__commandsMutex);
                m__commands.push_back(command);
            }
        }
        break;

    case CloudCommands::SOM_RESET:
        log::info(">>> SOM_RESET command");
        {
            std::unique_lock<std::mutex> l(m__commandsMutex);
            m__commands.push_back(std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [this, commandId, commandCode, bLocal](CloudCmd &o) {
                log::info() << "Reseting NGM Device...";
                o.state = CloudCmd::DONE;
                publishCommandAcceptedMessage(commandId, commandCode, "DONE", "Rebooting", m_generalDevice->getDeviceId(), bLocal);
                std::this_thread::sleep_for(std::chrono::microseconds(500));
                system("sync");
                system("reboot");
            }));
        }
        break;

    case CloudCommands::GET_HR_DATA:
        log::info(">>> GET_HR_DATA command");
        {
            CloudCmdPtr command = std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [&, this](CloudCmd &o) {
                o.state = CloudCmd::ACCEPTED;
            });

            std::string deviceId;
            DevicePtr dev;

            auto pit = it->find("sensor");
            if (pit == it->end()) {
                deviceId = "PowerMeter";
            }
            else {
                deviceId = *pit;
            }
            dev = std::static_pointer_cast<Device>(m_hwMgr->getDevice(deviceId));

            EventPtr event = std::make_shared<GetHRDEvent>(dev->getHRData(), dev->getHRDataSize(), command);
            dev->pushBackEvent(event);

            std::unique_lock<std::mutex> l(m__commandsMutex);
            m__commands.push_back(command);
        }
        break;

    case CloudCommands::COUNTERS_RESET:
        log::info(">>> COUNTERS_RESET command");
        {
            std::string deviceId;
            auto pit = it->find("sensor");
            if (pit == it->end()) {
                deviceId = "";
            }
            else {
                deviceId = *pit;
            }

            CloudCmdPtr command = std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [&, this, deviceId](CloudCmd &o) {
                o.state = CloudCmd::DONE;

                if (!deviceId.size()) {
                    for (size_t i = 0, sz = m_hwMgr->getDevicesCount(); i < sz; ++i) {
                        DevicePtr dev = std::static_pointer_cast<Device>(m_hwMgr->getDevice(i));
                        dev->resetStatistics();
                        dev->reset();
                    }
                }
                else {
                    try {
                        DevicePtr dev = std::static_pointer_cast<Device>(m_hwMgr->getDevice(deviceId));
                        dev->resetStatistics();
                        dev->reset();
                    }
                    catch(...) {
                        log::error() << "device " << deviceId << " not found...\n";
                    }
                }
            });

            std::unique_lock<std::mutex> l(m__commandsMutex);
            m__commands.push_back(command);
        }
        break;

    case CloudCommands::GET_CFG_FILE:
        log::info() << ">>> GET_CFG_FILE command";
        {
            CloudCmdPtr command = std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [commandId, commandCode, bLocal, this](CloudCmd &o) {

                publishCommandAcceptedMessage(commandId, commandCode, "ACCEPTED", "Uploading", m_generalDevice->getDeviceId(), bLocal);
                std::string key = m_generalDevice->getDeviceId() + ".xml";

                if (m_awsS3.Upload("Configuration.xml", key.c_str(), "ngm-config", 0, asFile)) {
                    log::info() << "Configuration.xml uploaded successfuly!";
                    o.state = CloudCmd::DONE;
                }
                else {
                    log::error() << "Configuration.xml upload failed!";
                    o.state = CloudCmd::REJECTED;
                    o.message = "Upload failed!";
                }
            });

            std::unique_lock<std::mutex> l(m__commandsMutex);
            m__commands.push_back(command);
        }
        break;

    case CloudCommands::GET_ACCREG_DUMP:
        log::info() << ">>> GET_ACCREG_DUMP command";
        {
            std::string logicName;
            auto pit = it->find("logicName");
            if (pit == it->end()) {
                logicName = "";
            }
            else {
                logicName = *pit;
            }
            CloudCmdPtr command = std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [commandId, commandCode, logicName, bLocal, this](CloudCmd &o) {

                publishCommandAcceptedMessage(commandId, commandCode, "ACCEPTED", "Uploading", m_generalDevice->getDeviceId(), bLocal);

                if (m_accDevices.size()) {

                    std::ofstream ofile;
                    ofile.open("acc-reg-dump.txt", std::ios_base::out | std::ios_base::trunc);
                    if (ofile.is_open()) {

                        AnyValue regAddress(ValueType::Long);
                        AnyValue regInc(ValueType::Long);
                        AnyValue regValue;
                        int c = 0x6b;
                        regAddress = 0;
                        regInc = 1;

                        {
                            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                            std::time_t now_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(24));
                            char ts_str[32];
                            if (strftime(ts_str, sizeof(ts_str), "%F %T", std::localtime(&now_c)) < 0) {
                                ts_str[0] = 0;
                            }

                            ofile << "NGM Accelerometer register settings for: NGM " << logicName << ", at " <<
                                     ts_str << "\n\n";
                        }

                        for (int i = 0; i <= c; ++i) {
                            auto it = m_accDevices.begin();
                            auto it_end = m_accDevices.end();
                            for (;it != it_end; ++it) {
                                regValue = (*it)->readRegister(regAddress);
                                ofile << (*it)->getName() << std::hex << " [0x" << regAddress << "]: 0x" << regValue << "\t\t";
                            }
                            regAddress += regInc;
                            ofile << "\n";
                        }
                        ofile.flush();
                        ofile.close();

                        std::string key = m_generalDevice->getDeviceId() + "-AccRegDump.txt";
                        if (m_awsS3.Upload("acc-reg-dump.txt", key.c_str(), "ngm-config", 0, asFile)) {
                            log::info() << "Registers dump of accelerometer uploaded successfuly!";
                            o.state = CloudCmd::DONE;
                        }
                        else {
                            log::error() << "Registers dump of accelerometer upload failed!";
                            o.state = CloudCmd::REJECTED;
                            o.message = "Upload failed!";
                        }
                    }
                    else {
                        o.state = CloudCmd::REJECTED;
                        o.message = "Can't save registers dump...";
                    }

                }
                else {
                    o.state = CloudCmd::REJECTED;
                    o.message = "Accelerometer disabled in configuration file...";
                }

            });


            //std::cout << "11\n";
            std::unique_lock<std::mutex> l(m__commandsMutex);
            m__commands.push_back(command);
            //std::cout << "12\n";
        }
        break;

    case CloudCommands::ACC_CALIBRATE:
        log::info(">>> ACC_CALIBRATE command");
        {
            std::string deviceId;
            auto pit = it->find("sensor");
            if (pit == it->end()) {
                deviceId = "";
            }
            else {
                deviceId = *pit;
            }

            log::info() << "AccCalibrate: creating command...";

            CloudCmdPtr command = std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [&, this, deviceId](CloudCmd &o) {
                o.state = CloudCmd::DONE;

                auto it = m_accDevices.begin();
                auto it_end = m_accDevices.end();
                for (;it != it_end; ++it) {
                    (*it)->reset();
                    log::info() << "Accelerometer <" << (*it)->getName() << "> was calibratred...";
                }
            });

            std::unique_lock<std::mutex> l(m__commandsMutex);
            m__commands.push_back(command);
        }
        break;

    case CloudCommands::SET_CFG_FILE:
        log::info() << ">>> SET_CFG_FILE command";
        {
            std::string url;
            auto pit = it->find("url");
            if (pit == it->end()) {
                publishCommandRejectedMessage(commandId, commandCode, "REJECTED", "<url> not found in json", m_generalDevice->getDeviceId(), bLocal);
            }
            else {
                url = *pit;
                CloudCmdPtr command = std::make_shared<CloudCmd>(bLocal, commandId, commandCode, [url, commandId, commandCode, bLocal, this](CloudCmd &o) {
                    publishCommandAcceptedMessage(commandId, commandCode, "ACCEPTED", "Download", m_generalDevice->getDeviceId(), bLocal);
                    std::string bash_cmd = "cd ./configuration/ && ./download.sh \"" + url + "\"";
                    if (system(bash_cmd.c_str()) == 0) {
                        std::ofstream idf;
                        idf.open(SET_CFG_CMD_ID_FILE);
                        if (idf.is_open()) {
                            idf << commandId;
                        }
                        else {
                            log::error() << "Can't open <" << SET_CFG_CMD_ID_FILE << "> for write";
                        }
                        idf.flush();
                        idf.close();

                        publishCommandAcceptedMessage(commandId, commandCode, "PROGRESS", "Rebooting", m_generalDevice->getDeviceId(), bLocal);
                        system("sync");
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        system("reboot");
                    }
                    else {
                        o.state = CloudCmd::REJECTED;
                        o.message = "Download or unpack failed";
                    }
                });

                std::unique_lock<std::mutex> l(m__commandsMutex);
                m__commands.push_back(command);
            }
        }
        break;

    default:
        publishCommandRejectedMessage(commandId, commandCode, "REJECTED", "Unknown command", m_generalDevice->getDeviceId(), bLocal);
        break;
    }
}

void CloudManager::publishCommandAcceptedMessage(std::string commandId, uint16_t commandCode,
                                                 std::string state, std::string message, std::string deviceId, bool bLocal) {
    nlohmann::json js;
    js["deviceId"] = deviceId;
    js["commandId"] = commandId;
    js["commandCode"] = commandCode;
    js["state"] = state;
    if (message.length()) {
        js["stateMetadata"]["message"] = message;
    }
    js["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::stringstream ss;
    ss.precision(3);
    ss << std::fixed << js;

    std::string topic;
    std::string payload = ss.str();
    if (!bLocal) {
        topic = "iot/device/" + deviceId + "/command/accepted";

        //std::cout << "33\n";
        std::lock_guard<std::mutex> l(m_mqttMsgsMutex);
        m_mqttMsgs.push(MqttMessage(topic, payload));
        //std::cout << "34\n";
        //m_client.publish(topic, QOS0, payload);
    }
    else {
        topic = "device/command/accepted";
        m_mosquitto.publish(NULL, topic.c_str(), payload.length(), payload.c_str());
    }
    log::info("<%s> publish: %s %s", (bLocal ? "mosquitto" : "aws"), topic.c_str(), ss.str().c_str());
}

void CloudManager::publishCommandRejectedMessage(std::string commandId, uint16_t commandCode,
                                                 std::string state, std::string message, std::string deviceId, bool bLocal) {
    nlohmann::json js;
    js["deviceId"] = deviceId;
    js["commandId"] = commandId;
    js["commandCode"] = commandCode;
    js["state"] = state;
    if (message.length()) {
        js["stateMetadata"]["message"] = message;
    }
    js["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::stringstream ss;
    ss.precision(3);
    ss << std::fixed << js;

    std::string topic;
    std::string payload = ss.str();
    if (!bLocal) {
        topic = "iot/device/" + deviceId + "/command/rejected";

        std::lock_guard<std::mutex> l(m_mqttMsgsMutex);
        m_mqttMsgs.push(MqttMessage(topic, payload));
        //m_client.publish(topic, QOS0, payload);
    }
    else {
        topic = "device/command/rejected";
        m_mosquitto.publish(NULL, topic.c_str(), payload.length(), payload.c_str());
    }
    log::info("<%s> publish: %s %s", (bLocal ? "mosquitto" : "aws"), topic.c_str(), ss.str().c_str());
}

void CloudManager::commandProc() {
    CloudCmdPtr cmd;
    {
        std::unique_lock<std::mutex> l(m__commandsMutex);
        if (m__commands.size()) {
            cmd = m__commands.front();
            m__commands.pop_front();
        }
    }


    if (cmd) {
        if (cmd->state == CloudCmd::INIT) {
            cmd->commandFunc(*cmd);
            if (cmd->state == CloudCmd::ACCEPTED) {
                publishCommandAcceptedMessage(cmd->commandId, cmd->commandCode, "ACCEPTED", cmd->message, m_generalDevice->getDeviceId(), cmd->bLocal);
            }
        }

        if (cmd->state == CloudCmd::DONE) {
            publishCommandAcceptedMessage(cmd->commandId, cmd->commandCode, "DONE", cmd->message,  m_generalDevice->getDeviceId(), cmd->bLocal);
        }
        else if (cmd->state == CloudCmd::REJECTED) {
            publishCommandRejectedMessage(cmd->commandId, cmd->commandCode, "REJECTED", cmd->message,  m_generalDevice->getDeviceId(), cmd->bLocal);
        }
        else {
            std::unique_lock<std::mutex> l(m__commandsMutex);
            m__commands.push_back(cmd);
        }
    }
}

bool CloudManager::setParamFromJson(nlohmann::json &js, std::string param,
                                    std::function<void(nlohmann::json::iterator&)> func) {
    auto it = js.find(param);
    if (it != js.end()) {
        try {
            func(it);
            return true;
        }
        catch(std::exception &e) {
            log::error() << "Exception: " << e.what();
        }

        catch(...){

        }
    }

    return false;
}

void CloudManager::eventsLoop() {
    log::info() << "Events Loop started, TPID: " << ::syscall(SYS_gettid);

    m_awsS3.Init();
    std::string deviceId =  m_generalDevice->getDeviceId();

    while (m_isRunning) {

        for(int i=0; i<m_hwMgr->getDevicesCount(); i++) {

            DevicePtr device = std::static_pointer_cast<Device>(m_hwMgr->getDevice(i));

            EventPtr ev = device->popFrontEvent();
            if (!ev)
                continue;

            bool bIsAcc = isAcc(device);
            if (bIsAcc ? !m_generalDevice->getAccEventsEnabled() : !m_generalDevice->getEventsEnabled()) {
                continue;
            }

            if (DeviceEventPtr e = std::dynamic_pointer_cast<DeviceEvent>(ev)) {
                if (e->state() == UploadState::New) {
                    log::info() << "DeviceEvent to Send: device=" << device->getName() << ", id=" << e->id() << ", type="
                                << e->getEventTypeName() << ", AutoHRData: " << (m_generalDevice->getAutoHrEnabled() ? "Enabled" : "Disabled");
                    {
                        std::string eventMessage = e->getEventMessage(deviceId);
                        m_mosquitto.publish(NULL, "device/event", eventMessage.length(), eventMessage.c_str());

                        std::lock_guard<std::mutex> l(m_mqttMsgsMutex);
                        m_mqttMsgs.push(MqttMessage(eventTopic(), eventMessage));
                    }

                    if (bIsAcc ? m_generalDevice->getAccAutoHrEnabled() : m_generalDevice->getAutoHrEnabled()) {
                        e->setState(UploadState::WaitingPostData);
                        device->pushBackEvent(ev);
                    }
                    else {
                        e->getHRParameter()->setEvent(nullptr);
                        e->getHRParameter()->setEventInProcess(false);
                    }
                }
                else if ( e->state() == UploadState::WaitingPostData ) {
                    /*uint64_t curr_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    if ((curr_ts - e->timestamp()) >= (uint64_t)(device->getPostEventTime() * 1000)) {
                        //log::info() << "curr_ts: " << curr_ts << ", e_ts: " << e->timestamp();
                        e->setHRData(device->getHRData());
                        e->setHRDataSize(device->getHRDataSize());
                        e->setState(UploadState::GotData);
                        log::info("DeviceEvent got HRData: device=%s, id=%s, type=%s", device->getName().c_str(),
                                  e->id().c_str(), e->getEventTypeName().c_str());
                    }*/
                    device->pushBackEvent(ev);
                }
                else if (    e->state() == UploadState::GotData ||
                        e->state() == UploadState::Failed &&
                        e->retries()++ < MAX_RETRIES) {

                    e->setState(UploadState::Uploading);

                    if ( m_awsS3.Send(e->id(), e->getHRData(), e->getHRDataSize(), device->getEventMetadata(e->timestamp()))) {
                        e->setState(UploadState::Uploaded);
                        e->getHRParameter()->setEventInProcess(false);
                    }
                    else {
                        e->setState(UploadState::Failed);
                        device->pushBackEvent(ev);
                    }
                }
                else {
                    log::error("DeviceEvent send failed: device=%s, id=%s, tries=%d", device->getName().c_str(),
                              e->id().c_str(), MAX_RETRIES);
                }
            }
            else if(GetHRDEventPtr e = std::dynamic_pointer_cast<GetHRDEvent>(ev)) {
                if (    e->state() == UploadState::New ||
                        e->state() == UploadState::Failed &&
                        e->retries()++ < MAX_RETRIES) {

                    CloudCmdPtr command = e->getCommand();
                    log::info("GetHRDEvent to Send: device=%s, cmdId=%s", device->getName().c_str(),
                              command->commandId.c_str());

                    e->setState(UploadState::Uploading);

                    if ( m_awsS3.Send(command->commandId, e->getHRData(), e->getHRDataSize(), device->getEventMetadata(e->timestamp()))) {
                        e->setState(UploadState::Uploaded);
                        command->state = CloudCmd::DONE;
                    }
                    else {
                        e->setState(UploadState::Failed);
                        command->state = CloudCmd::REJECTED;
                        device->pushBackEvent(ev);
                    }
                }
                else {
                    CloudCmdPtr command = e->getCommand();
                    command->state = CloudCmd::REJECTED;
                    log::error("GetHRDEvent send failed: device=%s, cmdId=%s, tries=%d", device->getName().c_str(),
                              command->commandId.c_str(), MAX_RETRIES);
                }
            }
            else {
                log::warn("Unknown Event: device=%s, id=%s", device->getName().c_str(), ev->id().c_str());
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
   m_awsS3.Done();
}

void CloudManager::commandsLoop()
{
    log::info() << "Commands Thread Started, TPID: " << ::syscall(SYS_gettid);
    while (m_isRunning) {

        commandProc();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    log::info() << "Commands Thread Stopped...";
}

void CloudManager::ledsLoop()
{
    m_gsmUp = netutils::isInterfaceUp("wwan0");
    bool bluetoothUpPrev = !m_bluetoothUp;
    bool bluetoothClientConnectedPrev = !m_bluetoothClientConnected;
    bool gsmUpPrev = !m_gsmUp;
    bool wifiUpPrev = !m_wifiUp;
    bool gpsUpPrev = !m_gpsUp;

    log::info() << "Leds loop started, TPID: " << ::syscall(SYS_gettid);
    while (m_isRunning) {

        if (m_bluetoothClientConnected != bluetoothClientConnectedPrev) {
            bluetoothUpPrev = !m_bluetoothUp;
            bluetoothClientConnectedPrev = m_bluetoothClientConnected;
        }

        if (m_bluetoothUp != bluetoothUpPrev) {
            if (m_bluetoothUp) {
                if (m_bluetoothClientConnected) {
                    m_leds->setLedOn(1, 25, 25, 25);
                }
                else {
                    m_leds->setLedOn(1, 0, 5, 25);
                }
            }
            else {
                m_leds->setLedOff(1);
            }
            bluetoothUpPrev = m_bluetoothUp;
        }

        m_gsmUp = netutils::isInterfaceUp("wwan0");
        m_wifiUp = netutils::isInterfaceUp("wlan0");

        if (m_gsmUp != gsmUpPrev) {
            if (m_gsmUp) {
                m_leds->setLedOn(2, 0, 25, 5);
                ::system("ifmetric wwan0 20");
                if (m_wifiUp) {
                    ::system("dhclient wlan0");
                    ::system("ifmetric wlan0 10");
                }
            }
            else {
                m_leds->setLedOff(2);
            }
            gsmUpPrev = m_gsmUp;
        }


        if (m_wifiUp != gsmUpPrev) {
            if (m_wifiUp) {
                ::system("ifmetric wlan0 10");
                if (m_gsmUp) {
                    //::system("dhclient wwan0");
                    ::system("ifmetric wwan0 20");
                }
            }
            wifiUpPrev = m_wifiUp;
        }

        if (m_gpsUp != gpsUpPrev) {
            if (m_gpsUp) {
                m_leds->setLedOn(0, 25, 25, 0);
            }
            else {
                m_leds->setLedOff(0);
            }
            gpsUpPrev = m_gpsUp;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }
    log::info() << "Leds loop stopped...";
}

void CloudManager::lastGaspLoop()
{
    log::info() << "Last Gasp thread started, TPID: " << ::syscall(SYS_gettid);
    if (!m_uio->open()) {
        log::error() << "Last Gasp Thread Stopped!...";
        return;
    }

    std::string deviceId =  m_generalDevice->getDeviceId();

    while(m_isRunning) {
        if (m_uio->poll(1000)) {
            glog::error() << "*** Last Gasp Interrupt!!! ***";

            log::info() << "Sending LastGaspEvent...";
            DeviceEventPtr event = std::make_shared<LastGaspEvent>();
            std::string eventMessage = event->getEventMessage(deviceId);

            std::queue<MqttMessage> msgs;
            msgs.push(MqttMessage(eventTopic(), eventMessage));

            {
                std::lock_guard<std::mutex> l(m_mqttMsgsMutex);
                m_mqttMsgs.swap(msgs);
            }
            break;
        }
#ifdef ENABLE_OBJECT_COUNTER
        ObjectCounter::logOutCounters();
#endif
    }

    m_uio->close();
    log::info() << "Last Gasp thread stopping...";
}

bool CloudManager::isAcc(DevicePtr dev)
{
    bool res = false;
    auto it = m_accDevices.begin();
    auto it_end = m_accDevices.end();
    for(; it != it_end; ++it) {
        if ((*it).get() == dev.get()) {
            res = true;
            break;
        }
    }
    return res;
}


MqttMessage::MqttMessage(std::string t, std::string p):
    topic(t), payload(p)
{}

MqttMessage::MqttMessage(MqttMessage &&r):
    topic(std::move(r.topic)),
    payload(std::move(r.payload))
{}
