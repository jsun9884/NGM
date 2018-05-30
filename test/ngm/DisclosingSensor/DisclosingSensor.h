#ifndef DISCLOSINGSENSOR_H
#define DISCLOSINGSENSOR_H

#include "Device.h"
#include "Leds.h"

struct DisclosingSensorParams
{
    HRParameterPtr m_closed;
    HRParameterPtr m_timestamp;
};

class DisclosingSensor: public Device
{
    LOG_MODULE(DisclosionSensor, g_main_log);
public:
    DisclosingSensor(std::string name, LedsPtr leds);
    virtual ~DisclosingSensor() {}

    virtual AnyValue readRegister(AnyValue address) override;
    virtual void writeRegister(AnyValue address, AnyValue value) override;

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;

    bool applyNativeParameter(ParameterPtr param);

protected:
    void checkHrParameters();

private:
    std::shared_ptr<bool> m_bPrevClosed;
    LedsPtr m_leds;
    DisclosingSensorParams m_hrParameters;
};

#endif // DISCLOSINGSENSOR_H
