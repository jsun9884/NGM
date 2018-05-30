#include "TempSensor.h"

extern "C" {
#include "aws_iot_log.h"
}


TempSensor::TempSensor(std::string name, I2cPtr i2c, int address):
    Device(name), m_i2c(i2c), m_address(address)
{
}

TempSensor::~TempSensor()
{

}

void TempSensor::initialize()
{

}

bool TempSensor::connect()
{
    {
        unsigned int buf[2] = {0};
        I2c::control c = m_i2c->acquire();
        if (c) {
            if (!c.slave(m_address) || !c.read(buf, 2)) {
                return false;
            }
        }
    }

    checkHrParameters();

    return true;
}

bool TempSensor::disconnect()
{
    return true;
}

void TempSensor::reset()
{

}

bool TempSensor::applyNativeParameter(ParameterPtr param)
{
    return false;
}

bool TempSensor::readData(HRData::RecordData &data)
{
    float out;
    unsigned char buf[2] = {0};

    data.set(m_hrParameters.m_timestamp, (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    {
        I2c::control c = m_i2c->acquire();
        if (c) {
            if (!c.slave(m_address) || !c.read(buf, 2)) {
                data.set(m_hrParameters.m_temperature, 0.0f);
                return false;
            }
        }
    }

    //log::info("Temp: %02x:%02x", buf[0], buf[1]);

    out = (float)((buf[0] & ~(1 << 7)) << 1 | ((buf[1] & (1 << 7))) >> 7);

    if (buf[1] & (1 << 6))
        out += 0.5f;

    if (buf[1] & (1 << 5))
        out += 0.25f;

    data.set(m_hrParameters.m_temperature, out);
    return true;
}

void TempSensor::checkHrParameters()
{
    m_hrParameters.m_temperature = m_hrData->getParameter("temperature");
    m_hrParameters.m_timestamp = m_hrData->getParameter("timestamp");
}
