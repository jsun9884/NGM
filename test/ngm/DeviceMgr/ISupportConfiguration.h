#include <vector>
#include <memory>
#include "Parameter.h"
#include "HRParameter.h"
#include "HRData.h"
#include "SensorState.h"

#pragma once

class ISupportConfiguration {
public:
    virtual std::string getName() const = 0;

    virtual SensorState state() const = 0;
    virtual bool popIsStateChanged() = 0;

    virtual void setParameter(std::string name, AnyValue value) = 0;
    virtual ParameterPtr getParameter(std::string) = 0;
    virtual std::vector<ParameterPtr>& getParameters() = 0;

    // HR:
    virtual HRParameterPtr getHRParameter(std::string) = 0;
    virtual std::vector<HRParameterPtr> getHRParameters() = 0;
    virtual HRData::RecordData getCurrentData() = 0;

    virtual void applyParams(bool onlyChanges) = 0;
    virtual void applyGeneralParams(bool onlyChanges) = 0;

    virtual void applyRegisters(bool onlyChanges) = 0;

    virtual AnyValue readRegister(AnyValue address) = 0;
    virtual void writeRegister(AnyValue address, AnyValue value) = 0;
};

typedef std::shared_ptr<ISupportConfiguration> ISupportConfigurationPtr;
