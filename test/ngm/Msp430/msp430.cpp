

extern "C" {
#include "aws_iot_log.h"
}

#include <chrono>
#include <iostream>
#include "msp430.h"
#include "dlt645msg.h"
#include "string.h"
#include "Event.h"
#include "CloudManager.h"

Msp430::Msp430(std::string name, std::string path) :
    Device(name),
    m_portPath(path)
{
    log::info() << "Created...";
}

void Msp430::checkHrParameters()
{
    m_hrParameters.m_voltage1 = m_hrData->getParameter("voltage1");
    m_hrParameters.m_current1 = m_hrData->getParameter("current1");
    m_hrParameters.m_activePower1 = m_hrData->getParameter("active_power1");
    m_hrParameters.m_reactivePower1 = m_hrData->getParameter("reactive_power1");
    m_hrParameters.m_apparentPower1 = m_hrData->getParameter("apparent_power1");
    m_hrParameters.m_powerFactor1 = m_hrData->getParameter("power_factor1");
    m_hrParameters.m_frequency1 = m_hrData->getParameter("frequency1");
    m_hrParameters.m_consumedActivePower1 = m_hrData->getParameter("consumed_active_power1");
    m_hrParameters.m_consumedReactivePower1 = m_hrData->getParameter("consumed_reactive_power1");

    m_hrParameters.m_voltage2 = m_hrData->getParameter("voltage2");
    m_hrParameters.m_current2 = m_hrData->getParameter("current2");
    m_hrParameters.m_activePower2 = m_hrData->getParameter("active_power2");
    m_hrParameters.m_reactivePower2 = m_hrData->getParameter("reactive_power2");
    m_hrParameters.m_apparentPower2 = m_hrData->getParameter("apparent_power2");
    m_hrParameters.m_powerFactor2 = m_hrData->getParameter("power_factor2");
    m_hrParameters.m_frequency2 = m_hrData->getParameter("frequency2");
    m_hrParameters.m_consumedActivePower2 = m_hrData->getParameter("consumed_active_power2");
    m_hrParameters.m_consumedReactivePower2 = m_hrData->getParameter("consumed_reactive_power2");

    m_hrParameters.m_voltage3 = m_hrData->getParameter("voltage3");
    m_hrParameters.m_current3 = m_hrData->getParameter("current3");
    m_hrParameters.m_activePower3 = m_hrData->getParameter("active_power3");
    m_hrParameters.m_reactivePower3 = m_hrData->getParameter("reactive_power3");
    m_hrParameters.m_apparentPower3 = m_hrData->getParameter("apparent_power3");
    m_hrParameters.m_powerFactor3 = m_hrData->getParameter("power_factor3");
    m_hrParameters.m_frequency3 = m_hrData->getParameter("frequency3");
    m_hrParameters.m_consumedActivePower3 = m_hrData->getParameter("consumed_active_power3");
    m_hrParameters.m_consumedReactivePower3 = m_hrData->getParameter("consumed_reactive_power3");

    m_hrParameters.m_timestamp = m_hrData->getParameter("timestamp");
}

bool Msp430::setPassword()
{
    Dlt645Msg msg = Dlt645Msg().createSetPassword(0x123456789abcdef0ll);
    Dlt645Msg responce;

    if (!Exchange(msg, responce, MaxTries)) {
        return false;
    }

    return true;
}

bool Msp430::getReadings(int phase, HRData::RecordData &recordData)
{
    Dlt645Msg msg = Dlt645Msg().createGetReadings(phase);
    Dlt645Msg responce;

    if (!Exchange(msg, responce, MaxTries)) {
        return false;
    }

    //log::info() << "rx: " << responce.hexString();
    return responce.getReadings(phase, recordData, m_hrParameters);
}

bool Msp430::applyNativeParameter(ParameterPtr param)
{
    return false;
}

Msp430::~Msp430()
{
    stop();
}

void Msp430::initialize()
{
    log::info() << "Initializing...";
}

bool Msp430::connect()
{
    log::info() << "Connecting to MSP430...";
    if (!m_port.open(m_portPath, 57600)) {
        log::error ("Can't open %s for read...", m_portPath.c_str());
        return false;
    }

    if (!setPassword()) {
        m_port.close();
        return false;
    }

    checkHrParameters();

    log::info() << "Connected!...";

    return true;
}

bool Msp430::readData(HRData::RecordData &data)
{
    data.set(m_hrParameters.m_timestamp, (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    if (!getReadings(0, data) || !getReadings(1, data) || !getReadings(2, data))
        return false;
    return true;
}

bool Msp430::disconnect()
{
    log::info() << "Disconnected...";
    m_port.close();
    return true;
}

void Msp430::reset()
{
    log::info() << "Reset Energy...";

    Dlt645Msg msg = Dlt645Msg().createResetEnergy();
    Dlt645Msg responce;

    if (!Exchange(msg, responce, MaxTries)) {
        log::error() << "Reset Energy failed...";
    }
}

bool Msp430::onOverBounds(HRParameterPtr param, AnyValue &value, uint64_t ts)
{
    bool ret = true;
    int i, count = m_hrData->getParametersCount();
    for (i = 0; i < count; ++i) {
        if (m_hrData->getParameter(i)->eventInProcess()) {
            ret = false;
            break;
        }
    }
    log::info() << "[" << getName() << "] " << "onOverBounds: return " << (ret ? "true" : "false");
    return ret;
}


bool Msp430::Exchange(const Dlt645Msg &message, Dlt645Msg &responce, int tries)
{
    std::lock_guard<std::mutex> l(m_exchangeMutex);
    for (int i = 0; i < tries; ++i) {
        //DEBUG("SendMessage: %s", message.hexString().c_str());

        if (!m_port.write(message.data(), message.size())) {
            return false;
        }

        int ret;
        if ((ret = m_port.read(responce.data(), responce.capacity(), 200)) > 0) {
            responce.set_size(ret);

            //DEBUG("RecvMessage (ret: %d): %s", ret, responce.hexString().c_str());

            if (!responce.CheckMessage()) {
            	log::error("responce not valid! (ret: %d): %s", ret, responce.hexString().c_str());
                return false;
            }
            return true;
        }
    }

    return false;
}

