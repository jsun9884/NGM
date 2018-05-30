/*
 * MosquittoClient.cpp
 *
 *  Created on: Aug 9, 2016
 *      Author: sergei
 */

#include "MosquittoClient.h"
#include "CloudManager.h"

MosquittoClient::MosquittoClient(CloudManager *mgr):
    m_mgr(mgr)
{
	//connect("localhost", 1883, 60);
}

MosquittoClient::~MosquittoClient() {
}

const char *MosquittoClient::configUpdateTopic() const
{
    return "device/config/update";
}

const char *MosquittoClient::configGetTopic() const
{
    return "device/config/get";
}

const char * MosquittoClient::commandTopic() const
{
    return "device/command";
}

const char *MosquittoClient::bluetoothPresenceTopic() const
{
    return "device/events/bluetooth/presence";
}

const char *MosquittoClient::networksGetTopic() const
{
    return "device/networks/get";
}

const char *MosquittoClient::networksAddTopic() const
{
    return "device/networks/add";
}

const char *MosquittoClient::networksDeleteTopic() const
{
    return "device/networks/delete";
}

void MosquittoClient::on_connect(int rc)
{
    log::info() << "Subscribe to <" << configUpdateTopic() << ">";
    subscribe(NULL, configUpdateTopic());

    log::info() << "Subscribe to <" << configGetTopic() << ">";
    subscribe(NULL, configGetTopic());

    log::info() << "Subscribe to <" << commandTopic() << ">";
    subscribe(NULL, commandTopic());

    log::info() << "Subscribe to <" << bluetoothPresenceTopic() << ">";
    subscribe(NULL, bluetoothPresenceTopic());

    log::info() << "Subscribe to <" << networksGetTopic() << ">";
    subscribe(NULL, networksGetTopic());

    log::info() << "Subscribe to <" << networksAddTopic() << ">";
    subscribe(NULL, networksAddTopic());

    log::info() << "Subscribe to <" << networksDeleteTopic() << ">";
    subscribe(NULL, networksDeleteTopic());

}

void MosquittoClient::on_message(const struct mosquitto_message* message)
{
    std::string topic = message->topic;
    if (topic == commandTopic()) {

        log::info("<mosquitto> Command callback...");
        log::info("json: %.*s", message->payloadlen, (const char*)message->payload);

        try {
            nlohmann::json js = nlohmann::json::parse(std::string((const char*)message->payload, message->payloadlen));

            m_mgr->prepareCommand(js, true);
        }
        catch(std::string &e) {
            log::warn("json parsing error: %s", e.c_str());
            return;
        }
        catch(...) {
            log::warn("json parsing error...");
            return;
        }
    }
    else if (topic == bluetoothPresenceTopic()) {
        log::info() << "bluetooth/presence callback...";
        log::info("json: %.*s", message->payloadlen, (const char*)message->payload);

        try {
            nlohmann::json js = nlohmann::json::parse(std::string((const char*)message->payload, message->payloadlen));

            nlohmann::json::iterator it = js.find("eventType");
            if (it == js.end()) {
                throw ("\'eventType' parsing error");
            }

            if (*it == "connected") {
                m_mgr->setBTClientStatus(true);
            }
            else if (*it == "disconnected") {
                m_mgr->setBTClientStatus(false);
            }
            else {
                throw ("unknown \'eventType\'...");
            }

        }
        catch(std::string &e) {
            log::warn("json parsing error: %s", e.c_str());
            return;
        }
        catch(...) {
            log::warn("json parsing error...");
            return;
        }
    }
    else if (topic == configUpdateTopic()) {
        try {
            nlohmann::json js = nlohmann::json::parse(std::string((const char*)message->payload, message->payloadlen));

            log::info() << "configUpdate: " << js;

            m_mgr->setShadowFromJson(js["state"]["desired"]);
            m_mgr->upgradeShadow();
        }
        catch(...) {
            log::warn("json parsing error...");
            return;
        }

    }
    else if (topic == configGetTopic()) {
        log::info() << "Mosquitto GET!";
        try {
            std::string id;
            nlohmann::json js = nlohmann::json::parse(std::string((const char*)message->payload, message->payloadlen));

            nlohmann::json::iterator it = js.find("MqttRequestId");
            if (it == js.end()) {
                throw ("\'MqttRequestId' parsing error");
            }

            id = *it;

            nlohmann_fixed::json js_s;
            for(int i=0; i < m_mgr->m_hwMgr->getDevicesCount(); i++) {
                std::static_pointer_cast<Device>(m_mgr->m_hwMgr->getDevice(i))->addShadowData(js_s);
            }

            std::string configStr;
            std::string topic = configGetTopic();
            topic += "/" + id;

            {
                std::stringstream ss;
                ss.precision(3);
                ss << std::fixed << js_s;
                configStr = ss.str();
            }

            publish(NULL, topic.c_str(), configStr.length(), configStr.c_str());
            log::info() << "publish: " << topic << ": " << configStr;
        }
        catch(std::string &e) {
            log::warn("json parsing error: %s", e.c_str());
            return;
        }
        catch(...) {
            log::warn("json parsing error...");
            return;
        }
    }
    else if (topic == networksGetTopic()) {
        log::info() << "networks/get request...";
        try {
            std::string id;
            nlohmann::json js = nlohmann::json::parse(std::string((const char*)message->payload, message->payloadlen));

            nlohmann::json::iterator it = js.find("MqttRequestId");
            if (it == js.end()) {
                throw ("\'MqttRequestId' parsing error");
            }

            id = *it;

            std::string availableNetworks = GeneralDevice::exec("nmcli d wifi list");
            std::string connections = GeneralDevice::exec("nmcli c");

            nlohmann_fixed::json js_s;
            js_s["AvailableNetworks"] = availableNetworks;
            js_s["Connections"] = connections;

            std::string out;
            std::string topic = networksGetTopic();
            topic += "/" + id;
            {
                std::stringstream ss;
                ss.precision(3);
                ss << std::fixed << js_s;
                out = ss.str();
            }

            publish(NULL, topic.c_str(), out.length(), out.c_str());
            log::info() << "publish: " << topic << ": " << out;
        }
        catch(std::string &e) {
            log::warn("json parsing error: %s", e.c_str());
            return;
        }
    }
    else if (topic == networksAddTopic()) {
        log::info() << "networks/add request...";
        try {
            std::string ssid;
            std::string passwd;
            std::string payload = std::string((const char*)message->payload, message->payloadlen);
            log::info() << "networks/add: " << payload;

            nlohmann::json js = nlohmann::json::parse(payload);
            nlohmann::json::iterator it = js.find("WifiSSID");
            if (it == js.end()) {
                throw std::string("\'WifiSSDI' parsing error");
            }
            ssid = *it;

            it = js.find("WifiPassword");
            if (it == js.end()) {
                throw std::string("\'WifiPassword' parsing error");
            }
            passwd = *it;

            {
                std::stringstream ss;
                ss << "nmcli d wifi connect " << ssid << " password " << passwd;

                ::system(ss.str().c_str());
            }
        }
        catch(std::string &e) {
            log::warn("json parsing error: %s", e.c_str());
            return;
        }
    }
    else if (topic == networksDeleteTopic()) {
        log::info() << "networks/delete request...";
        try {
            std::string uuid;
            std::string payload = std::string((const char*)message->payload, message->payloadlen);
            log::info() << "networks/add: " << payload;

            nlohmann::json js = nlohmann::json::parse(payload);
            nlohmann::json::iterator it = js.find("NetworkUUID");
            if (it == js.end()) {
                throw ("\'NetworkUUID' parsing error");
            }
            uuid = *it;

            {
                std::stringstream ss;
                ss << "nmcli c delete " << uuid;

                ::system(ss.str().c_str());
            }
        }
        catch(std::string &e) {
            log::warn("json parsing error: %s", e.c_str());
            return;
        }
    }
    else {
        log::info("unknown message: json: %.*s", message->payloadlen, (const char*)message->payload);
    }
}
