#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <stdio.h>

extern "C" {
#include "aws_iot_log.h"
}

#include "GeneralDevice.h"

GeneralDevice::GeneralDevice(std::string name) :
    Device(name)
{
}

GeneralDevice::~GeneralDevice() {
}

void GeneralDevice::initialize()
{
    // Defaults
    ParameterPtr param = Parameter::create("KeepAliveMsgTransmissionRate", (uint64_t)1);
    addParameter(param);

    param = Parameter::create("RegularMsgTransmissionRate", (uint64_t)60);
    addParameter(param);

    param = Parameter::create("AutoHrEnabled", false);
    addParameter(param);

    param = Parameter::create("AccAutoHrEnabled", false);
    addParameter(param);

    param = Parameter::create("GpsEnabled", false);
    addParameter(param);

    param = Parameter::create("EventsEnabled", true);
    addParameter(param);

    param = Parameter::create("AccEventsEnabled", true);
    addParameter(param);

    std::string deviceId = generateDeviceID();
    param = Parameter::create("DeviceID", deviceId);
    addParameter(param);

    param = Parameter::create("MqttHost", (std::string)"A7RDHYR0RGGZ.iot.us-west-2.amazonaws.com");
    addParameter(param);

    param = Parameter::create("MqttPort", 8883);
    addParameter(param);

    param = Parameter::create("GsmNumber", (std::string)"", false);
    addParameter(param);

    param = Parameter::create("GsmApn", (std::string)"", false);
    addParameter(param);

    param = Parameter::create("GsmUsername", (std::string)"", false);
    addParameter(param);

    param = Parameter::create("GsmPassword", (std::string)"", false);
    addParameter(param);

    //param = Parameter::create("WifiSSID", (std::string)"", false);
    //addParameter(param);

    //param = Parameter::create("WifiPassword", (std::string)"", false);
    //addParameter(param);

    checkGeneralParameters();
}

bool GeneralDevice::connect() {

    return true;
}

bool GeneralDevice::readData(HRData::RecordData &data)
{
    return true;
}

bool GeneralDevice::disconnect() {
    return true;
}
void GeneralDevice::reset() {}

bool GeneralDevice::applyNativeParameter(ParameterPtr param)
{
    bool bRes = false;
    log::info() << "applingNativeParameter: " << param->getName() << " = " << param->getValue();
    if (param->getName() == "DeviceID") {
        std::ifstream cfg;
        cfg.open("/etc/bluetooth/main.conf");

        if (param == m_generalParameters.m_deviceID || !cfg.is_open()) {
            system("echo \"[General]\" > /etc/bluetooth/main.conf");
            std::string line = "echo \"Name = NGM-" + param->getValue().cast<std::string>() + "\" >> /etc/bluetooth/main.conf";
            system(line.c_str());
            system("sync");
        }

        bRes = true;
    }
    else if (param->getName() == "GsmNumber") {
        setNmGsmParameter("gsm.number", param->getValue());
        bRes = true;
    }
    else if (param->getName() == "GsmApn") {
        setNmGsmParameter("gsm.apn", param->getValue());
        bRes = true;
    }
    else if (param->getName() == "GsmUsername") {
        setNmGsmParameter("gsm.username", param->getValue());
        bRes = true;
    }
    else if (param->getName() == "GsmPassword") {
        setNmGsmParameter("gsm.password", param->getValue());
        bRes = true;
    }
    /*else if (param->getName() == "WifiSSID") {
        //if (m_generalParameters.m_wifiPassword->isDirty()) {
            initWifi(m_generalParameters.m_wifiSSID->getValue(),
                     m_generalParameters.m_wifiPassword->getValue());

            //m_generalParameters.m_wifiPassword->clearDirty();
            bRes = true;
        //}
    }
    else if (param->getName() == "WifiPassword") {
        //if (m_generalParameters.m_wifiSSID->isDirty()) {
            initWifi(m_generalParameters.m_wifiSSID->getValue(),
                     m_generalParameters.m_wifiPassword->getValue());

            //m_generalParameters.m_wifiSSID->clearDirty();
            bRes = true;
        //}
    }*/
    /*else if (param->getName() == "WifiSSID") {
        setNmWifiParameter("wifi.ssid", param->getValue());
        bRes = true;
    }
    else if (param->getName() == "WifiPassword") {
        setNmWifiParameter("wifi-security.psk", param->getValue());
        bRes = true;
    }*/

    return bRes;
}

void GeneralDevice::checkGeneralParameters()
{
    m_generalParameters.m_keepAliveMsgTransmissionRate = getParameter("KeepAliveMsgTransmissionRate");
    m_generalParameters.m_regularMsgTransmissionRate = getParameter("RegularMsgTransmissionRate");
    m_generalParameters.m_autoHrEnabled = getParameter("AutoHrEnabled");
    m_generalParameters.m_accAutoHrEnabled = getParameter("AccAutoHrEnabled");
    m_generalParameters.m_gpsEnabled = getParameter("GpsEnabled");
    m_generalParameters.m_eventsEnabled = getParameter("EventsEnabled");
    m_generalParameters.m_accEventsEnabled = getParameter("AccEventsEnabled");
    m_generalParameters.m_deviceID = getParameter("DeviceID");
    m_generalParameters.m_mqttHost = getParameter("MqttHost");
    m_generalParameters.m_mqttPort = getParameter("MqttPort");
    m_generalParameters.m_gsmNumber = getParameter("GsmNumber");
    m_generalParameters.m_gsmApn = getParameter("GsmApn");
    m_generalParameters.m_gsmUsername = getParameter("GsmUsername");
    m_generalParameters.m_gsmPassword = getParameter("GsmPassword");
    //m_generalParameters.m_wifiSSID = getParameter("WifiSSID");
    //m_generalParameters.m_wifiPassword = getParameter("WifiPassword");
}

int GeneralDevice::getRegularMsgRate(){
    return (int)m_generalParameters.m_regularMsgTransmissionRate->getValue();
}

int GeneralDevice::getKeepAliveMsgRate()
{
    return (int)m_generalParameters.m_keepAliveMsgTransmissionRate->getValue();
}

bool GeneralDevice::getAutoHrEnabled()
{
    return (bool)m_generalParameters.m_autoHrEnabled->getValue();
}

bool GeneralDevice::getAccAutoHrEnabled()
{
    return (bool)m_generalParameters.m_accAutoHrEnabled->getValue();
}

bool GeneralDevice::getGpsEnabled()
{
    return (bool)m_generalParameters.m_gpsEnabled->getValue();
}

bool GeneralDevice::getEventsEnabled()
{
    return (bool)m_generalParameters.m_eventsEnabled->getValue();
}

bool GeneralDevice::getAccEventsEnabled()
{
    return (bool)m_generalParameters.m_accEventsEnabled->getValue();
}

std::string GeneralDevice::getDeviceId()
{
    return m_generalParameters.m_deviceID->getValue();
}

std::string GeneralDevice::getMqttHost()
{
    return m_generalParameters.m_mqttHost->getValue();
}

int GeneralDevice::getMqttPort()
{
    return (int)m_generalParameters.m_mqttPort->getValue();
}

void GeneralDevice::loadGsmSettings()
{
    CSimpleIniA ini(true, false, false);
    SI_Error rc = ini.LoadFile("/etc/NetworkManager/system-connections/GSMInternet");
    if (rc < 0) {
        std::string cmd = "nmcli c add type gsm ifname cdc-wdm0 con-name \"GSMInternet\" apn internetg";
        if (::system(cmd.c_str()) != 0) {
            log::error() << "command: [" << cmd << "] failed...";
            return;
        }

        rc = ini.LoadFile("/etc/NetworkManager/system-connections/GSMInternet");
        if (rc < 0)
            return;
    }

    const char *number = ini.GetValue("gsm", "number");
    if (number) {
        m_generalParameters.m_gsmNumber->setValue(std::string(number));
    }

    const char *apn = ini.GetValue("gsm", "apn");
    if (apn) {
        m_generalParameters.m_gsmApn->setValue(std::string(apn));
    }

    const char *username = ini.GetValue("gsm", "username");
    if (username) {
        m_generalParameters.m_gsmUsername->setValue(std::string(username));
    }

    const char *password = ini.GetValue("gsm", "password");
    if (password) {
        m_generalParameters.m_gsmPassword->setValue(std::string(password));
    }
}

/*void GeneralDevice::loadWifiSettings()
{
    CSimpleIniA ini(true, false, false);
    SI_Error rc = ini.LoadFile("/etc/NetworkManager/system-connections/WifiCon");

    if (rc < 0) {
        return;
    }

    const char *ssid = ini.GetValue("wifi", "ssid");
    if (ssid) {
        m_generalParameters.m_wifiSSID->setValue(std::string(ssid));
    }

    const char *password = ini.GetValue("wifi-security", "psk");
    if (password) {
        m_generalParameters.m_wifiPassword->setValue(std::string(password));
    }
}*/

std::string GeneralDevice::exec(std::string cmd)
{
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

void GeneralDevice::setNmGsmParameter(std::string param, std::string value)
{
    std::stringstream ss;
    ss << "nmcli c modify \"GSMInternet\"" << (value.length() ? " " : " -") << param << " \"" << value << "\"";
    if (::system(ss.str().c_str()) != 0) {
        log::error() << "command: [" << ss.str() << "] failed...";
    }
}

void GeneralDevice::setNmWifiParameter(std::string param, std::string value)
{
    std::stringstream ss;
    ss << "nmcli c modify \"WifiCon\"" << (value.length() ? " " : " -") << param << " \"" << value << "\"";
    if (::system(ss.str().c_str()) != 0) {
        log::error() << "command: [" << ss.str() << "] failed...";
    }
}

void GeneralDevice::initWifi(AnyValue ssid, AnyValue passwd)
{

    try {
        std::string s_ssid = ssid;
        std::string s_passwd = passwd;
        std::string c_ssid, c_passwd;

        CSimpleIniA ini(true, false, false);
        SI_Error rc = ini.LoadFile("/etc/NetworkManager/system-connections/WifiCon");

        if (rc >= 0) {
            const char *cc_ssid = ini.GetValue("wifi", "ssid");
            const char *cc_passwd = ini.GetValue("wifi-security", "psk");
            c_ssid = cc_ssid ? cc_ssid : "";
            c_passwd = cc_passwd ? cc_passwd : "";
        }

        if ((s_ssid != c_ssid) || (s_passwd != c_passwd)) {
            std::string cmd_del;
            std::stringstream ss;
            cmd_del = "nmcli c delete WifiCon && nmcli c reload";
            ss << "nmcli d wifi connect " << s_ssid;
            if (s_passwd.length())
                ss << " password " << s_passwd;
            ss << " name WifiCon";

            glog::info() << cmd_del << " && " << ss.str();
            ::system(cmd_del.c_str());
            //std::this_thread::sleep_for(std::chrono::seconds(3));
            ::system(ss.str().c_str());
            //std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }
}

std::string GeneralDevice::generateDeviceID()
{
    std::string devId;
    std::ifstream dev;
    dev.open("/sys/class/net/wlan0/address");
    if (!dev.is_open()) {
        return "DEV_ID";
    }

    dev >> devId;

    if (devId.size() >= 17) {
        devId.erase(2, 1);
        devId.erase(4, 1);
        devId.erase(6, 1);
        devId.erase(8, 1);
        devId.erase(10, 1);
    }

    std::transform(devId.begin(), devId.end(), devId.begin(), ::toupper);
    return devId;
}

void GeneralDevice::addStatusData(nlohmann::json &js) {}
void GeneralDevice::addRegularData(nlohmann::json &js) {}
