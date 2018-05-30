#include "HRParameter.h"
#include "Event.h"

extern "C" {
#include "aws_iot_log.h"
}


HRParameter::HRParameter(std::string name, AnyValue defaultValue):
    ParameterBase(name, defaultValue), m_offset(0), m_shortIntervalSize(0), m_statisticsEnabled(false),
    m_max_long(defaultValue),
    m_min_long(defaultValue),
    m_avg_long(0.0),
    m_max_short(defaultValue),
    m_min_short(defaultValue),
    m_avg_short(0.0),
    m_current_value(defaultValue),
    m_rowReadPeriod(1)
{
    m_eventInProcess.store(false);
}

HRParameterPtr HRParameter::create(std::string name, AnyValue defaultValue)
{
    return HRParameterPtr(new HRParameter(name, defaultValue));
}

bool HRParameter::getStatisticsEnabled() const
{
    return m_statisticsEnabled;
}

void HRParameter::setStatisticsEnabled(bool value)
{
    m_statisticsEnabled = value;
}

void HRParameter::addValue(AnyValue &&value, bool isLongIntervalPassed, bool isShortIntervalPassed)
{
    /*if(isLongIntervalPassed)*/
    while (m_lastValues.size() > STAT_MAX_COUNT) {
        m_lastValues.pop_front();
    }

    m_lastValues.push_back(std::move(value));

    /*if(isShortIntervalPassed && m_shortIntervalSize==0)*/
    if (!m_shortIntervalSize && (m_lastValues.size() >= STAT_MIN_COUNT)) {
        m_shortIntervalSize = STAT_MIN_COUNT;
    }

    size_t size = m_lastValues.size();
    size_t shortSize = m_shortIntervalSize == 0 ? size : m_shortIntervalSize;

    AnyValue min_long = m_lastValues.front();
    AnyValue max_long = m_lastValues.front();
    AnyValue avg_long = 0.0;

    AnyValue min_short = m_lastValues.front();
    AnyValue max_short = m_lastValues.front();
    AnyValue avg_short = 0.0;

    for (std::list<AnyValue>::reverse_iterator it=m_lastValues.rbegin(); it != m_lastValues.rend(); ++it)
    {
        min_long = *it < min_long ? *it : min_long;
        max_long = *it > max_long ? *it : max_long;
        avg_long += *it;

        if(shortSize-- > 0){
            min_short =  *it < min_short ? *it : min_short;
            max_short = *it > max_short ? *it : max_short;
            avg_short += *it;
        }
    }
    avg_long /= double(size);
    avg_short /= double(m_shortIntervalSize == 0 ? size : m_shortIntervalSize);

    std::unique_lock<std::mutex> l(m__valuesMutex);
    m_max_long = max_long;
    m_min_long = min_long;
    m_avg_long = avg_long;
    m_max_short = max_short;
    m_min_short = min_short;
    m_avg_short = avg_short;
    m_current_value = m_lastValues.back();
}

void HRParameter::getStatistics(nlohmann::json &js)
{
    /*std::list<AnyValue> d;
    std::lock_guard<std::mutex> ls(m__resetStatisticsMutex);
    {
        std::lock_guard<std::mutex> l(m__valuesMutex);
        d.swap(m_lastValues);
    }


    if(d.size() == 0){
        return;
    }



    d.back().setJson("current_value", js[m_name]);
    min_long.setJson("min_60", js[m_name]);
    max_long.setJson("max_60", js[m_name]);
    avg_long.setJson("avg_60", js[m_name]);
    min_short.setJson("min_15", js[m_name]);
    max_short.setJson("max_15", js[m_name]);
    avg_short.setJson("avg_15", js[m_name]);

    {
        std::lock_guard<std::mutex> l(m__valuesMutex);
        d.swap(m_lastValues);
        m_lastValues.splice(m_lastValues.end(), std::move(d));
        while(m_lastValues.size() > (STAT_MAX_INTERVAL * 60)) {
            m_lastValues.pop_front();
        }
    }*/

    std::lock_guard<std::mutex> l(m__valuesMutex);
    m_current_value.setJson("current_value", js[m_name]);
    m_min_long.setJson("min_60", js[m_name]);
    m_max_long.setJson("max_60", js[m_name]);
    m_avg_long.setJson("avg_60", js[m_name]);
    m_min_short.setJson("min_15", js[m_name]);
    m_max_short.setJson("max_15", js[m_name]);
    m_avg_short.setJson("avg_15", js[m_name]);
}

size_t HRParameter::getLastValuesCount()
{
    return m_lastValues.size();
}

void HRParameter::resetStatistics()
{
    std::lock_guard<std::mutex> ls(m__resetStatisticsMutex);
    std::lock_guard<std::mutex> l(m__valuesMutex);
    m_lastValues.clear();
}

bool HRParameter::eventInProcess() const
{
    return m_eventInProcess;
}

void HRParameter::setEventInProcess(bool value)
{
    m_eventInProcess = value;
}

EventPtr HRParameter::getEvent()
{
    return m_event;
}

void HRParameter::setEvent(EventPtr event)
{
    m_event = event;
}

void HRParameter::setSqlRow(SqlRowPtr row)
{
    m_sqlRow = row;
}


