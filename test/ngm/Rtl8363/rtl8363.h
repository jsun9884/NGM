#ifndef RTL8363_H
#define RTL8363_H

#include "Log.h"
#include "Device.h"

struct RtlHRParameters
{
    HRParameterPtr m_portA_rx;
    HRParameterPtr m_portA_tx;
    HRParameterPtr m_portB_rx;
    HRParameterPtr m_portB_tx;
    HRParameterPtr m_timestamp;
};

class Rtl8363 : public Device
{
    LOG_MODULE(Rtl8363, g_rtl_log);

public:
    Rtl8363(std::string name);
    virtual ~Rtl8363();

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override;

    virtual bool applyNativeParameter(ParameterPtr param) override;

private:
    void checkHrParameters();
    bool getPortStatistics(int port, void *data);
    bool setPortEnabled(bool enabled, int port);
    bool setRgmiiRxDelay(int delay);
    bool setRgmiiTxDelay(int delay);

    RtlHRParameters m_hrParameters;
    int m_fd;
};

#endif // RTL8363_H
