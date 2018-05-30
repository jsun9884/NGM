#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include "json_fixed.hpp"

#include "Log.h"
#include "HRStatistics.h"
#include "ISupportConfiguration.h"
#include "Parameter.h"
#include "Register.h"
#include "HRParameter.h"
#include "tinyxml.h"
#include "SimpleIni.h"
#include "SensorState.h"
#include "Event.h"
#include "HRData.h"

#define MAXIMUM_EVENTS 7

#pragma once

class Device;
typedef std::shared_ptr<Device> DevicePtr;

class Device : public ISupportConfiguration {
    LOG_MODULE(Device, g_main_log);

protected:
    std::atomic<SensorState> m_state;
    std::atomic_bool m_statusChanged;

	std::string m_name;
    //std::recursive_mutex m__paramsMutex;
	std::unordered_map<std::string, ParameterPtr> m_htParameters;
	std::vector<ParameterPtr> m_vParameters;
    std::vector<RegisterPtr> m_vRegisters;
    HRDataPtr m_hrData;
    std::atomic<int> m_devStatEnabled;
    HRStatisticsPtr m_hrStatistics;
    ParameterPtr m_postEventTime;

    std::list<EventPtr> m__events;
    std::mutex m__eventsMutex;

    bool m_bRunning;
    std::shared_ptr<std::thread> m_procThread;
    void threadProc();

    void makeJsonElem(nlohmann::json &js, std::string param, std::string type);
    void addJsonElemToArr(nlohmann::json &js_arr, nlohmann::json &js);

protected:
    Device(std::string name);

protected:
    void loadParameter(TiXmlElement *elem);
    void loadRegister(TiXmlElement *elem);
    void loadHRParameter(TiXmlElement *elem);
    void saveParameter(TiXmlElement* elem, ParameterPtr param);
    void saveHRParameter(TiXmlElement* elem, HRParameterPtr param);

    void saveAttribute(TiXmlElement* elem, std::string attributeName, AnyValue value);

    AnyValue loadValueByAttributeName(TiXmlElement *elem, ValueType valueType, std::string name);
    AnyValue loadValue(TiXmlElement *elem, ValueType valueType);

    void addParameter(ParameterPtr param);
    void addRegister(RegisterPtr reg);

    void createEvent(DeviceEventType type, HRParameterPtr parameter, AnyValue value, uint64_t ts);

protected:
    void setState(SensorState state);
    void applyParameter(ParameterPtr param, bool onlyChanges);
    void applyRegister(RegisterPtr reg, bool onlyChanges);
    void applyGeneralParameter(ParameterPtr param, bool onlyChanges);
    bool applyGeneralParameter(ParameterPtr param);
    virtual bool applyNativeParameter(ParameterPtr param) = 0;

public:
    Device() = delete;
	Device(const Device &) = delete;
	Device(Device &&) = delete;

    void preInit();
    void postInit();
    bool start();
    bool isRunning();
    bool stop();
    void loadConfiguration(TiXmlElement *elem);
    void saveConfiguration(TiXmlElement* elem);

    void resetStatistics();

    EventPtr popFrontEvent();
    void pushBackEvent(EventPtr event);

public:
    virtual std::string getName() const override;

    virtual SensorState state() const { return m_state.load(); }
    virtual bool popIsStateChanged();

    virtual void setParameter(std::string name, AnyValue value) override;
    virtual ParameterPtr getParameter(std::string name) override;
    virtual std::vector<ParameterPtr>& getParameters() override;

    virtual HRParameterPtr getHRParameter(std::string name) override;
    virtual std::vector<HRParameterPtr> getHRParameters() override;
    virtual HRData::RecordData getCurrentData() override;

    virtual void applyParams(bool onlyChanges) override;
    virtual void applyGeneralParams(bool onlyChanges) override;
    virtual void applyRegisters(bool onlyChanges) override;

    virtual AnyValue readRegister(AnyValue address) override;
    virtual void writeRegister(AnyValue address, AnyValue value) override;

    virtual ByteBufferPtr getHRData();
    virtual size_t getHRDataSize();
    virtual size_t getPostEventTime();
    virtual void checkThresholds(const HRData::RecordData &rd, const HRData::RecordData &prevRD);
    virtual bool onOverBounds(HRParameterPtr param, AnyValue &value, uint64_t ts);
    virtual std::string getEventMetadata(uint64_t ts);

    virtual void getDeviceParams(nlohmann::json& json);
    virtual void getDeviceParamsFixed(nlohmann_fixed::json& json);
    virtual void addStatusData(nlohmann::json &js);
    virtual void addRegularData(nlohmann::json &js);

    virtual void setShadowFromJson(nlohmann::json& json);
    virtual void addShadowData(nlohmann_fixed::json &js);

    virtual void initialize() = 0;
    virtual bool connect() = 0;
    virtual bool readData(HRData::RecordData &data) = 0;
    virtual bool disconnect() = 0;
    virtual void reset() = 0;

    friend class HRStatistics;
};


