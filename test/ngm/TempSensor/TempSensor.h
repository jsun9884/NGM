#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H

#include <thread>
#include <string>
#include <memory>
#include <atomic>
#include <list>
#include <mutex>

#include "Log.h"
#include "SensorState.h"
#include "Event.h"
#include "Device.h"
#include "HRData.h"
#include "I2c.h"
#include "TempHRParameters.h"

class TempSensor: public Device
{
    LOG_MODULE(TempSensor, g_main_log);

public:
    TempSensor(std::string name, I2cPtr i2c, int address);
    ~TempSensor();

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;

    bool applyNativeParameter(ParameterPtr param);

protected:
    void checkHrParameters();

private:
    I2cPtr m_i2c;
    int m_address;

    TempHRParameters m_hrParameters;

};

#endif // TEMPSENSOR_H
