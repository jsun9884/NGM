#include <stdint.h>
#include "CpuTemperature.h"

CpuTemperature::CpuTemperature(std::string name, std::string path):
    Device(name), m_path(path)
{

}

CpuTemperature::~CpuTemperature()
{

}

void CpuTemperature::initialize()
{

}

bool CpuTemperature::connect()
{
    m_file.open(m_path, std::ifstream::in);
    checkHrParameters();
    return m_file.is_open();
}

bool CpuTemperature::readData(HRData::RecordData &rd)
{
    if (!m_file.is_open()) {
        return false;
    }

    uint32_t data = 0;
    m_file >> data;
    m_file.seekg(0, m_file.beg);
    rd.set(m_hrParameters.m_temperature, data / 1000.0f);
    rd.set(m_hrParameters.m_timestamp, (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    return true;
}

bool CpuTemperature::disconnect()
{
    m_file.close();
    return true;
}

void CpuTemperature::reset()
{

}

bool CpuTemperature::applyNativeParameter(ParameterPtr param)
{
    return false;
}

void CpuTemperature::checkHrParameters()
{
    m_hrParameters.m_temperature = m_hrData->getParameter("temperature");
    m_hrParameters.m_timestamp = m_hrData->getParameter("timestamp");
}
