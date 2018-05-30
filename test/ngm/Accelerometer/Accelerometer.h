#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <vector>
#include <chrono>

#include "Log.h"
#include "Device.h"
#include "Spi.h"
#include "Algorithm.h"

class Accelerometer : public Device, public IAccelerometer
{
    LOG_MODULE(Accelerometer, g_main_log);

public:
    Accelerometer(std::string name, std::string devPath, bool bInvert);
    virtual ~Accelerometer();

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;

    virtual AnyValue readRegister(AnyValue address) override;
    virtual void writeRegister(AnyValue address, AnyValue value) override;


    virtual double calcAccel(int64_t input) override;
    virtual double calcGyro(int64_t input) override;

    virtual std::string getDeviceName() override;

protected:
    virtual bool applyNativeParameter(ParameterPtr param);
    virtual bool onOverBounds(HRParameterPtr param, AnyValue &value, uint64_t ts);

protected:
    void checkParameters();

    bool chipConnect();

protected:
    uint8_t read(uint8_t address);
    void readBuffer(uint8_t address, void *recvData, size_t recvDataSize);
    void write(uint8_t address, uint8_t data);
    bool setRegister(uint8_t address, uint8_t data);

protected:
    virtual void readAccRaw(Vector<int64_t> &v) override;
    virtual void readGyroRaw(Vector<int64_t> &v) override;

private:
    bool m_bInvert;
    std::string m_devPath;
    Spi m_spi;
    std::shared_ptr<Algorithm> m_algorithm;
    AccParams m_params;
    AccHrParams m_hrParams;

    friend class Algorithm;
};

#endif // ACCELEROMETER_H
