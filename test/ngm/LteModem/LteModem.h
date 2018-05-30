#ifndef LTEMODEM_H
#define LTEMODEM_H

#include "Log.h"
#include "Device.h"

struct LteModemParams
{
    HRParameterPtr m_rx;
    HRParameterPtr m_tx;
    HRParameterPtr m_timestamp;
};

class LteModem: public Device
{
    LOG_MODULE(LteModem, g_main_log);

public:
    LteModem(std::string name);
    virtual ~LteModem() {}

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;

    virtual bool applyNativeParameter(ParameterPtr param) override;

protected:
    void checkHrParameters();

private:
    LteModemParams m_hrParameters;
};

#endif // LTEMODEM_H
