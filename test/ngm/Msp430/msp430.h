#ifndef MSP430_H
#define MSP430_H

#include <thread>
#include <string>
#include <memory>
#include <atomic>
#include <list>
#include <mutex>

#include "Log.h"
#include "serialport.h"
#include "dlt645msg.h"
#include "SensorState.h"
#include "Event.h"
#include "Device.h"

class Msp430 : public Device
{
    LOG_MODULE(Msp430, g_msp430_log);

public:
    enum {
        MaxTries = 1
    };

    Msp430(std::string name, std::string path);
    ~Msp430();

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;

protected:
    virtual bool onOverBounds(HRParameterPtr param, AnyValue &value, uint64_t ts);

protected:
    void checkHrParameters();
    bool setPassword();
    bool getReadings(int phase, HRData::RecordData &recordData);

    virtual bool applyNativeParameter(ParameterPtr param) override;

private:
    bool Exchange(const Dlt645Msg &message, Dlt645Msg &responce, int tries);

private:
    Msp430HRParameters m_hrParameters;

    std::string m_portPath;
    SerialPort m_port;
    std::mutex m_exchangeMutex;
};

#endif // MSP430_H
