/*
 * Cloud.cpp
 *
 *  Created on: Jun 9, 2016
 *      Author: sergei
 */

#include <chrono>
#include "Event.h"

#include <string.h>
#include <unistd.h>

static const std::string uploadStateStr[] =  {"New", "Uploading", "Uploaded", "Failed"};

std::mutex Event::s_mutex;

std::string Event::getUploadText (int uploadState) {
    return uploadStateStr[uploadState];
}

//////////////////////

Event::Event() :
    m_id(),
    m_uploadState(UploadState::New),
    m_timestamp(0),
    m_retries(0)
{
    std::lock_guard<std::mutex> l(s_mutex);
    m_id = Aws::Utils::UUID::RandomUUID().operator Aws::String().c_str();
}


///////////////////////

HRDEvent::HRDEvent(uint64_t ts):
    Event(),
    m_size(0UL)
{
    m_timestamp = ts;
}

HRDEvent::HRDEvent(ByteBufferPtr data, std::size_t size, uint64_t ts) :
    Event(),
    m_data(data),
    m_size(size)
{
    m_timestamp = ts;
}

ByteBufferPtr HRDEvent::getHRData() const
{
    return m_data;
}

void HRDEvent::setHRData(ByteBufferPtr buf)
{
    m_data = buf;
}

size_t HRDEvent::getHRDataSize() const
{
    return m_size;
}

void HRDEvent::setHRDataSize(size_t size)
{
    m_size = size;
}

///////////////////////

CommEvent::CommEvent(CloudCmdPtr command) :
    Event(),
    m_cmd(command)
{
    m_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

CloudCmdPtr CommEvent::getCommand()
{
    return m_cmd;
}

///////////////////////

GetHRDEvent::GetHRDEvent(ByteBufferPtr data, std::size_t size, CloudCmdPtr command) :
    HRDEvent(data, size, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()),
    CommEvent(command)
{}

///////////////////////

DeviceEvent::DeviceEvent(DeviceEventType type, std::string deviceName, uint64_t ts, HRParameterPtr parameter):
    HRDEvent(ts),
    m_name(deviceName),
    m_type(type),
    m_parameter(parameter)
{}

DeviceEvent::DeviceEvent(DeviceEventType type, ByteBufferPtr data, std::size_t size, std::string deviceName, uint64_t ts, HRParameterPtr parameter) :
      HRDEvent(data, size, ts),
      m_name(deviceName),
      m_type(type),
      m_parameter(parameter)
{}

DeviceEvent::~DeviceEvent()
{
    if (m_parameter) {
        m_parameter->setEventInProcess(false);
    }
}

std::string DeviceEvent::getEventMessage(std::string ngmId) const {
    std::stringstream ss;
    ss.precision(3);

    nlohmann::json js;
    js["deviceId"] = ngmId;
    js["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    js["payload"]["eventId"] = m_id;
    js["payload"]["createdAt"] = m_timestamp;
    js["payload"]["eventType"] = getEventTypeName();
    js["payload"]["eventSource"] = m_name;
    js["payload"]["eventSeverity"] = "0"; //todo
    {
        nlohmann::json ed;
        getEventData(ed);
        js["payload"]["eventData"] = ed;
    }

    ss << std::fixed << js;
    return ss.str();
}

std::string DeviceEvent::getEventTypeName() const
{
    std::string out;
    switch (m_type) {
    case DeviceEventType::None:
        out = "None";
        break;

    case DeviceEventType::OverBounds:
        out = "OverBounds";
        break;

    case DeviceEventType::LastGasp:
        out = "LastGasp";
        break;

    case DeviceEventType::TamperSwitch:
        out = "TamperSwitch";
        break;

    default:
        out = "Unknown";
        break;
    }

    return out;
}

HRParameterPtr DeviceEvent::getHRParameter()
{
    return m_parameter;
}

///////////////////////


OverBoundsEvent::OverBoundsEvent(std::string deviceName, HRParameterPtr parameter, AnyValue value, uint64_t ts):
    DeviceEvent(DeviceEventType::OverBounds, deviceName, ts, parameter),
    m_value(value)
{
}

OverBoundsEvent::OverBoundsEvent(ByteBufferPtr data, std::size_t size, std::string deviceName, HRParameterPtr parameter, AnyValue value, uint64_t ts):
    DeviceEvent(DeviceEventType::OverBounds, data, size, deviceName, ts, parameter),
    m_value(value)
{
}

void OverBoundsEvent::getEventData(nlohmann::json &js) const
{
    js["parameter"] = m_parameter->getName();
    m_value.setJson("value", js);
    const AnyValue &high = m_parameter->getThresholdHigh();
    if(!high.empty())
        high.setJson("thresholdHigh", js);

    const AnyValue &low = m_parameter->getThresholdLow();
    if(!low.empty())
        low.setJson("thresholdLow", js);
}

LastGaspEvent::LastGaspEvent():
    DeviceEvent(DeviceEventType::LastGasp, "NGM",
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(), nullptr)
{}

void LastGaspEvent::getEventData(nlohmann::json &js) const
{
}

TamperSwitchEvent::TamperSwitchEvent(std::string deviceName, HRParameterPtr parameter, AnyValue value, uint64_t ts):
    DeviceEvent(DeviceEventType::TamperSwitch, deviceName, ts, parameter),
    m_value(value)
{
}

void TamperSwitchEvent::getEventData(nlohmann::json &js) const
{
    js["parameter"] = m_parameter->getName();
    m_value.setJson("value", js);
}
