#ifndef GENERALDEVICE
#define GENERALDEVICE

#include "Parameter.h"
#include "Device.h"

struct GeneralParameters {
    ParameterPtr m_keepAliveMsgTransmissionRate;
    ParameterPtr m_regularMsgTransmissionRate;
    ParameterPtr m_autoHrEnabled;
    ParameterPtr m_accAutoHrEnabled;
    ParameterPtr m_gpsEnabled;
    ParameterPtr m_eventsEnabled;
    ParameterPtr m_accEventsEnabled;
    ParameterPtr m_deviceID;
    ParameterPtr m_mqttHost;
    ParameterPtr m_mqttPort;

    ParameterPtr m_gsmNumber;
    ParameterPtr m_gsmApn;
    ParameterPtr m_gsmUsername;
    ParameterPtr m_gsmPassword;
    //ParameterPtr m_wifiSSID;
    //ParameterPtr m_wifiPassword;
};

class GeneralDevice;
typedef std::shared_ptr<GeneralDevice> GeneralDevicePtr;

class GeneralDevice : public Device
{
private:
    LOG_MODULE(General, g_main_log);

    GeneralParameters m_generalParameters;

public:
    GeneralDevice(std::string name);
    ~GeneralDevice();

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;
    virtual bool applyNativeParameter(ParameterPtr param) override;

    virtual void addStatusData(nlohmann::json &js) override;
    virtual void addRegularData(nlohmann::json &js) override;

    void checkGeneralParameters();
    int getRegularMsgRate();
    int getKeepAliveMsgRate();
    bool getAutoHrEnabled();
    bool getAccAutoHrEnabled();
    bool getGpsEnabled();
    bool getEventsEnabled();
    bool getAccEventsEnabled();
    std::string getDeviceId();
    std::string getMqttHost();
    int getMqttPort();
    void loadGsmSettings();
    //void loadWifiSettings();

    static std::string exec(std::string cmd);

private:
    void setNmGsmParameter(std::string param, std::string value);
    void setNmWifiParameter(std::string param, std::string value);
    void initWifi(AnyValue ssid, AnyValue passwd);
    std::string generateDeviceID();
};
#endif // GENERALDEVICE

