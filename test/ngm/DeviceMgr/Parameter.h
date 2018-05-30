#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include "ParameterBase.h"

#pragma once

class Parameter;
typedef std::shared_ptr<Parameter> ParameterPtr;

class Parameter: public ParameterBase {
private:
	ParameterPtr m_parent;
    mutable std::mutex m_valueMutex;
    AnyValue m_value;
    bool m_isDirty;

    std::unordered_map<std::string, ParameterPtr> m_htSubParameters;
	std::vector<ParameterPtr> m_vSubParameters;

private:
    Parameter(ParameterPtr parent, std::string name, const AnyValue &defaultValue, bool isDirty);
    Parameter(std::string name, const AnyValue &defaultValue, bool isDirty);
    void addSubParameter(ParameterPtr param);

public:
	Parameter() = delete;
	Parameter(const Parameter &) = delete;
	Parameter(Parameter &&) = delete;
	
    static ParameterPtr create(std::string name, const AnyValue &defaultValue, bool isDirty = true);
    static ParameterPtr create(ParameterPtr parent, std::string name, const AnyValue &defaultValue, bool isDirty = true);

    AnyValue getValue()	const;
    void setValue(const AnyValue &value);

	//AnyValue GetValueOrDefault() const;
    int getSubParamsCount() const;
    ParameterPtr getSubParameter(std::string name) const;
    std::vector<ParameterPtr>& getSubParameters();
    std::unordered_map<std::string, ParameterPtr>& getHtSubParameters();
    bool hasSubParameters() const { return !m_vSubParameters.empty(); }
	
	void clearDirty() { m_isDirty = false; }
    bool isDirty() { return m_isDirty; }
};

