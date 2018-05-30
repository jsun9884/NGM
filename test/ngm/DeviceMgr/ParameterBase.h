#ifndef PARAMETERBASE_H
#define PARAMETERBASE_H

#include "AnyValue.h"

class ParameterBase
{
protected:
    std::string m_name;
    ValueType m_type;
    AnyValue m_defaultValue;
    AnyValue m_thresholdLow;
    AnyValue m_thresholdHigh;

protected:
    ParameterBase(std::string name, const AnyValue &defaultValue);
    ParameterBase() = delete;

public:
    std::string getName() const { return m_name; }
    ValueType getType() const { return m_type; }

    const AnyValue& getDefaultValue() const;
    void setDefaultValue(const AnyValue &value);

    const AnyValue& getThresholdLow() const;
    void setThresholdLow(const AnyValue &value);

    const AnyValue& getThresholdHigh() const;
    void setThresholdHigh(const AnyValue &value);

    int IsExceededThreshold(const AnyValue &value);
    
};

#endif // PARAMETERBASE_H
