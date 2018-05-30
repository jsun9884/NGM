#include "ParameterBase.h"

ParameterBase::ParameterBase(std::string name, const AnyValue &defaultValue):
    m_name(name), m_type(defaultValue.type()), m_defaultValue(defaultValue)
{
}

const AnyValue &ParameterBase::getDefaultValue() const
{
    return m_defaultValue;
}

void ParameterBase::setDefaultValue(const AnyValue &value)
{
    m_defaultValue = value;
}

const AnyValue &ParameterBase::getThresholdLow() const
{
    return m_thresholdLow;
}

void ParameterBase::setThresholdLow(const AnyValue &value)
{
    m_thresholdLow = value;
}

const AnyValue &ParameterBase::getThresholdHigh() const
{
    return m_thresholdHigh;
}

void ParameterBase::setThresholdHigh(const AnyValue &value)
{
    m_thresholdHigh = value;
}

int ParameterBase::IsExceededThreshold(const AnyValue &value)
{
    int res = 0;
    if (!m_thresholdLow.empty() && (value < m_thresholdLow)) {
        res = -1;
    }
    if (!m_thresholdHigh.empty() && (value > m_thresholdHigh)) {
        res = 1;
    }
    return res;
}
