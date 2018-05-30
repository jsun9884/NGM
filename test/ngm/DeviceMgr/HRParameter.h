#ifndef HRPARAMETER_H
#define HRPARAMETER_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <list>
#include <chrono>
#include <atomic>
#include "ParameterBase.h"
#include "SQLite3DB.h"

class HRData;

#define STAT_MIN_INTERVAL  15
                       //  15
#define STAT_MAX_INTERVAL  60
                       //  60
#define STAT_MIN_COUNT (STAT_MIN_INTERVAL * 60 / 5)
#define STAT_MAX_COUNT (STAT_MAX_INTERVAL * 60 / 5)

class HRParameter;
typedef std::shared_ptr<HRParameter> HRParameterPtr;

class Event;
typedef std::shared_ptr<Event> EventPtr;

class HRParameter: public ParameterBase
{
private:
    size_t m_offset;
    std::list<AnyValue> m_lastValues;

    size_t m_shortIntervalSize;
    std::mutex m__valuesMutex;
    std::mutex m__resetStatisticsMutex;
    bool m_statisticsEnabled;
    std::atomic_bool m_eventInProcess;
    EventPtr m_event;

    AnyValue m_min_long;
    AnyValue m_max_long;
    AnyValue m_avg_long;

    AnyValue m_min_short;
    AnyValue m_max_short;
    AnyValue m_avg_short;

    AnyValue m_current_value;

    SqlRowPtr m_sqlRow;
    std::chrono::system_clock::time_point m_lastRowReadTime;
    std::chrono::seconds m_rowReadPeriod;

private:
    HRParameter(std::string name, AnyValue defaultValue);
    void setOffset(size_t offset) { m_offset = offset; }

public:
    HRParameter() = delete;
    HRParameter(const HRParameter &) = delete;
    HRParameter(HRParameter &&) = delete;

    static HRParameterPtr create(std::string name, AnyValue defaultValue);

    size_t getOffset() const { return m_offset; }

    friend class HRData;

    bool getStatisticsEnabled() const;
    void setStatisticsEnabled(bool value);
    void addValue(AnyValue&& value, bool isLongIntervalPassed, bool isShortIntervalPassed);
    void getStatistics(nlohmann::json &js);
    size_t getLastValuesCount();
    void resetStatistics();

    bool eventInProcess() const;
    void setEventInProcess(bool value);

    EventPtr getEvent();
    void setEvent(EventPtr event);

    void setSqlRow(SqlRowPtr row);

    template<class T>
    void setSqlRowValue(const T &value) {
        if (m_sqlRow) {
            auto now = std::chrono::system_clock::now();
            if ( (now - m_lastRowReadTime) >= m_rowReadPeriod) {
                m_sqlRow->setValue(value);
                m_lastRowReadTime = now;
            }
        }
    }
};

#endif // HRPARAMETER_H
