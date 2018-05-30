#include "Parameter.h"

Parameter::Parameter(ParameterPtr parent, std::string name, const AnyValue &defaultValue, bool isDirty) :
    ParameterBase(name, defaultValue),
    m_parent(parent), m_value(defaultValue),
    m_isDirty(isDirty)
{
}

Parameter::Parameter(std::string name, const AnyValue &defaultValue, bool isDirty) :
    ParameterBase(name, defaultValue),
    m_value(defaultValue), m_isDirty(isDirty)
{
}

ParameterPtr Parameter::create(ParameterPtr parent, std::string name, const AnyValue &defaultValue, bool isDirty)
{
    ParameterPtr param(new Parameter(parent, name, defaultValue, isDirty));
    parent->addSubParameter(param);
	return param;
}

ParameterPtr Parameter::create(std::string name, const AnyValue &defaultValue, bool isDirty)
{
    return ParameterPtr(new Parameter(name, defaultValue, isDirty));
}

AnyValue Parameter::getValue() const
{
    std::lock_guard<std::mutex> l(m_valueMutex);
	return m_value;
}

void Parameter::setValue(const AnyValue &value)
{
    std::lock_guard<std::mutex> l(m_valueMutex);
    if (m_value != value) {
        m_value = value;
        m_isDirty = true;
    }
}

int Parameter::getSubParamsCount() const
{
	return m_htSubParameters.size();
}

ParameterPtr Parameter::getSubParameter(std::string name) const
{
	auto it = m_htSubParameters.find(name);
	if (it == m_htSubParameters.end()) {
        THROW(NoSuchSubParameterEx);
	}

	return it->second;
}

std::vector<ParameterPtr>& Parameter::getSubParameters()
{
	return m_vSubParameters;
}

std::unordered_map<std::string, ParameterPtr>& Parameter::getHtSubParameters()
{
	return m_htSubParameters;
}

void Parameter::addSubParameter(ParameterPtr param)
{
    if (m_htSubParameters.find(param->getName()) != m_htSubParameters.end()) {
        THROWEX(ParameterAlreadyExistsEx, param->getName());
	}
    m_htSubParameters.insert(std::pair<std::string, ParameterPtr>(param->getName(), param));
	m_vSubParameters.push_back(param);
}

