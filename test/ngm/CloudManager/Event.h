/*
 * Cloud.h
 *
 *  Created on: Jun 9, 2016
 *      Author: sergei
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <string.h>
#include <mutex>
#include <vector>
#include "json.hpp"
#include "CloudCommands.h"
#include "HRData.h"
#include <aws/core/utils/UUID.h>

#define MAX_RETRIES 2
#pragma once

enum class UploadState: int {
    New = 0,
    WaitingPostData = 1,
    GotData = 2,
    Uploading = 3,
    Uploaded = 4,
    Failed = 4
};

class Event {
public:
    Event();
    virtual ~Event() {}

    std::string id() const { return m_id; }
    UploadState state() const { return m_uploadState.load(); }
    uint64_t timestamp() const { return m_timestamp; }
    void setState(UploadState state) { m_uploadState.store(state); }
    uint16_t& retries() { return m_retries; }

protected:
    std::string getUploadText (int uploadState);

    // members
    std::string m_id;
    uint64_t m_timestamp;
    std::atomic<UploadState> m_uploadState;
    uint16_t m_retries;

    static std::mutex s_mutex;
};
typedef std::shared_ptr<Event> EventPtr;

class HRDEvent : virtual public Event {
public:
    HRDEvent(uint64_t ts);
    HRDEvent(ByteBufferPtr data, std::size_t size, uint64_t ts);
    ByteBufferPtr getHRData() const;
    void setHRData(ByteBufferPtr buf);
    size_t getHRDataSize() const;
    void setHRDataSize(size_t size);

protected:
    ByteBufferPtr m_data;
    std::size_t m_size;
};

class CommEvent : virtual public Event {
public:
    CommEvent(CloudCmdPtr command);
    CloudCmdPtr getCommand();

protected:
    CloudCmdPtr m_cmd;
};

class GetHRDEvent : virtual public HRDEvent, virtual public CommEvent {
public:
    GetHRDEvent(ByteBufferPtr data, std::size_t size, CloudCmdPtr command);
};
typedef std::shared_ptr<GetHRDEvent> GetHRDEventPtr;


enum class DeviceEventType: int
{
    None = 0,
    OverBounds,
    LastGasp,
    TamperSwitch
};

class DeviceEvent : public HRDEvent {
public:
    DeviceEvent(DeviceEventType type, std::string deviceName, uint64_t ts, HRParameterPtr parameter);
    DeviceEvent(DeviceEventType type, ByteBufferPtr data, std::size_t size, std::string deviceName, uint64_t ts, HRParameterPtr parameter);

    virtual ~DeviceEvent();

    std::string getEventMessage(std::string ngmId) const;
    std::string getEventTypeName() const;
    HRParameterPtr getHRParameter();

protected:
    virtual void getEventData(nlohmann::json &js) const = 0;

protected:
    std::string m_name;
    DeviceEventType m_type;
    HRParameterPtr m_parameter;
};
typedef std::shared_ptr<DeviceEvent> DeviceEventPtr;


class OverBoundsEvent: public DeviceEvent {
public:
    OverBoundsEvent(std::string deviceName, HRParameterPtr parameter, AnyValue value, uint64_t ts);
    OverBoundsEvent(ByteBufferPtr data, std::size_t size, std::string deviceName, HRParameterPtr parameter, AnyValue value, uint64_t ts);

protected:
    virtual void getEventData(nlohmann::json &js) const override;

protected:
    AnyValue m_value;
};

class LastGaspEvent: public DeviceEvent {
public:
    LastGaspEvent();

protected:
    virtual void getEventData(nlohmann::json &js) const override;

};

class TamperSwitchEvent: public DeviceEvent {
public:
    TamperSwitchEvent(std::string deviceName, HRParameterPtr parameter, AnyValue value, uint64_t ts);

protected:
    virtual void getEventData(nlohmann::json &js) const override;

protected:
    AnyValue m_value;

};

#endif /* EVENT_H_ */
