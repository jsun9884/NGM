#include "LteModem.h"
#include "NetUtils.h"

LteModem::LteModem(std::string name):
    Device(name)
{

}

void LteModem::initialize()
{
    log::info() << "initializing <" << getName() << ">...";
}

bool LteModem::connect()
{
    int val;
    if (netutils::getInterfaceRxConter("wwan0", val)) {
        checkHrParameters();
        return true;
    }
    return false;
}

bool LteModem::readData(HRData::RecordData &data)
{
    int rx, tx;

    if (netutils::getInterfaceRxConter("wwan0", rx)) {
        if (netutils::getInterfaceTxConter("wwan0", tx)) {
            //log::info() << "rx: " << rx << ", tx: " << tx;
            data.set(m_hrParameters.m_rx, rx);
            data.set(m_hrParameters.m_tx, tx);
            uint64_t ts = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            data.set(m_hrParameters.m_timestamp, ts);
            return true;
        }
    }

    return false;
}

bool LteModem::disconnect()
{
    return true;
}

void LteModem::reset()
{
    log::info() << "reset() not implemented...";
}

bool LteModem::applyNativeParameter(ParameterPtr param)
{
    return false;
}

void LteModem::checkHrParameters()
{
    m_hrParameters.m_rx = m_hrData->getParameter("rx");
    m_hrParameters.m_tx = m_hrData->getParameter("tx");
    m_hrParameters.m_timestamp = m_hrData->getParameter("timestamp");
}
