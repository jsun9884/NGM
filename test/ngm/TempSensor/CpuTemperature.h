#ifndef CPUTEMPERATURE_H
#define CPUTEMPERATURE_H

#include <fstream>
#include <string>

#include "Log.h"
#include "Device.h"
#include "TempHRParameters.h"

class CpuTemperature: public Device
{
    LOG_MODULE(CpuTemperature, g_main_log);

public:
    CpuTemperature(std::string name, std::string path);
    ~CpuTemperature();

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;

    bool applyNativeParameter(ParameterPtr param);

protected:
    void checkHrParameters();

private:
    std::string m_path;
    std::ifstream m_file;
    TempHRParameters m_hrParameters;

};

#endif // CPUTEMPERATURE_H
