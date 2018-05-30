#include <fstream>
#include "DisclosingSensor.h"

DisclosingSensor::DisclosingSensor(std::string name, LedsPtr leds):
    Device(name), m_leds(leds)
{}

AnyValue DisclosingSensor::readRegister(AnyValue address)
{
    uint8_t addr = address, val;
    if (!m_leds->readRegister(addr, val)) {
        THROW(RegisterReadEx);
    }
    return val;
}

void DisclosingSensor::writeRegister(AnyValue address, AnyValue value)
{
    uint8_t addr = address, val = value;
    if (!m_leds->writeRegister(addr, val)) {
        THROW(RegisterWriteEx);
    }
}

void DisclosingSensor::initialize()
{

}

bool DisclosingSensor::connect()
{
    checkHrParameters();

    return m_leds->initDisclosingSensor();
}

bool DisclosingSensor::readData(HRData::RecordData &data)
{
    bool closed;
    /*if (!m_leds->checkDisclosingSensor(closed)) {
        return false;
    }*/
    {
        std::ifstream file;
        file.open("/sys/class/gpio/gpio122/value");
        if (!file.is_open()) {
            system("echo 122 > /sys/class/gpio/export");
            system("echo in > /sys/class/gpio/gpio122/direction");
            file.open("/sys/class/gpio/gpio122/value");
            if (!file.is_open()) {
                return false;
            }
        }

        int i;
        file >> i;

        //log::info() << "gpio122 is " << i;

        if (i == 0) {
            closed = true;
        }
        else {
            closed = false;
        }
    }
    uint64_t ts = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    data.set(m_hrParameters.m_closed, closed);
    data.set(m_hrParameters.m_timestamp, ts);

    // For test
    //createEvent(DeviceEventType::TamperSwitch, m_hrParameters.m_closed, false, ts);
    //createEvent(DeviceEventType::TamperSwitch, m_hrParameters.m_closed, true, ts);

    if (m_bPrevClosed) {
        if(*m_bPrevClosed != closed) {
            createEvent(DeviceEventType::TamperSwitch, m_hrParameters.m_closed, closed, ts);
        }
    }
    else {
        m_bPrevClosed.reset(new bool());
    }
    *m_bPrevClosed = closed;
    return true;
}

bool DisclosingSensor::disconnect()
{
    return true;
}

void DisclosingSensor::reset()
{

}

bool DisclosingSensor::applyNativeParameter(ParameterPtr param)
{
    return false;
}

void DisclosingSensor::checkHrParameters()
{
    m_hrParameters.m_closed = m_hrData->getParameter("closed");
    m_hrParameters.m_timestamp = m_hrData->getParameter("timestamp");
}

