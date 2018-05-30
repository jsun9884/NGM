#ifndef DLT645MSG_H
#define DLT645MSG_H

#include <string>

#ifdef LINUX
#include <unistd.h>
#endif
#include "buffer.h"
#include "HRData.h"
#include "Log.h"

// if not NULL then determined in .xml
struct Msp430HRParameters {
    HRParameterPtr m_voltage1;
    HRParameterPtr m_current1;
    HRParameterPtr m_activePower1;
    HRParameterPtr m_reactivePower1;
    HRParameterPtr m_apparentPower1;
    HRParameterPtr m_powerFactor1;
    HRParameterPtr m_frequency1;
    HRParameterPtr m_consumedActivePower1;
    HRParameterPtr m_consumedReactivePower1;

    HRParameterPtr m_voltage2;
    HRParameterPtr m_current2;
    HRParameterPtr m_activePower2;
    HRParameterPtr m_reactivePower2;
    HRParameterPtr m_apparentPower2;
    HRParameterPtr m_powerFactor2;
    HRParameterPtr m_frequency2;
    HRParameterPtr m_consumedActivePower2;
    HRParameterPtr m_consumedReactivePower2;

    HRParameterPtr m_voltage3;
    HRParameterPtr m_current3;
    HRParameterPtr m_activePower3;
    HRParameterPtr m_reactivePower3;
    HRParameterPtr m_apparentPower3;
    HRParameterPtr m_powerFactor3;
    HRParameterPtr m_frequency3;
    HRParameterPtr m_consumedActivePower3;
    HRParameterPtr m_consumedReactivePower3;

    HRParameterPtr m_timestamp;
};

class Dlt645Msg: protected Buffer
{
    LOG_MODULE(Dlt645Msg, g_msp430_log);

public:
    enum {
        MaxSize = 128
    };

    enum {
        HOST_CMD_GET_METER_CONFIGURATION            = 0x56,
        HOST_CMD_SET_METER_CONSUMPTION              = 0x57,
        HOST_CMD_SET_RTC                            = 0x58,
        HOST_CMD_GET_RTC                            = 0x59,
        HOST_CMD_ALIGN_WITH_CALIBRATION_FACTORS     = 0x5A,
        HOST_CMD_SET_PASSWORD                       = 0x60,
        HOST_CMD_GET_READINGS_PHASE_1               = 0x61,
        HOST_CMD_GET_READINGS_PHASE_2               = 0x62,
        HOST_CMD_GET_READINGS_PHASE_3               = 0x63,
        HOST_CMD_GET_READINGS_NEUTRAL               = 0x64,
        HOST_CMD_GET_EXTRA_READINGS_PHASE_1         = 0x69,
        HOST_CMD_GET_EXTRA_READINGS_PHASE_2         = 0x6A,
        HOST_CMD_GET_EXTRA_READINGS_PHASE_3         = 0x6B,
        HOST_CMD_GET_EXTRA_READINGS_NEUTRAL         = 0x6C,
        HOST_CMD_ERASE_FLASH_SEGMENT                = 0x70,
        HOST_CMD_SET_FLASH_POINTER                  = 0x71,
        HOST_CMD_FLASH_DOWNLOAD                     = 0x72,
        HOST_CMD_FLASH_UPLOAD                       = 0x73,
        HOST_CMD_ZAP_MEMORY_AREA                    = 0x74,
        HOST_CMD_SUMCHECK_MEMORY                    = 0x75,
        HOST_CMD_GET_RAW_POWER_PHASE_1              = 0x91,
        HOST_CMD_GET_RAW_POWER_PHASE_2              = 0x92,
        HOST_CMD_GET_RAW_POWER_PHASE_3              = 0x93,
        HOST_CMD_GET_RAW_DELAYED_POWER_PHASE_1      = 0x95,
        HOST_CMD_GET_RAW_DELAYED_POWER_PHASE_2      = 0x96,
        HOST_CMD_GET_RAW_DELAYED_POWER_PHASE_3      = 0x97,
        HOST_CMD_GET_RAW_POWER_NEUTRAL              = 0x99,
        HOST_CMD_GET_RAW_DELAYED_POWER_NEUTRAL      = 0x9D,
        HOST_CMD_CHECK_RTC_ERROR                    = 0xA0,
        HOST_CMD_RTC_CORRECTION                     = 0xA1,
        HOST_CMD_MULTIRATE_SET_PARAMETERS           = 0xC0,
        HOST_CMD_MULTIRATE_GET_PARAMETERS           = 0xC1,
        HOST_CMD_MULTIRATE_CLEAR_USAGE              = 0xC2,
        HOST_CMD_MULTIRATE_GET_USAGE                = 0xC3,
        HOST_CMD_GET_NGM_PHASE_1                    = 0xD1,
        HOST_CMD_GET_NGM_PHASE_2                    = 0xD2,
        HOST_CMD_GET_NGM_PHASE_3                    = 0xD3,
        HOST_CMD_RESET_ENERGY                       = 0xD4
    };

    Dlt645Msg();
    Dlt645Msg(Dlt645Msg &&msg);

    using Buffer::data;
    using Buffer::size;
    using Buffer::set_size;
    using Buffer::capacity;
    using Buffer::operator [];

    Dlt645Msg &operator= (Dlt645Msg&& o);

    bool CheckMessage();
    bool getReadings(int phase, HRData::RecordData &d, const Msp430HRParameters &p);

private:
    int32_t Extract32(size_t &i);
    int16_t Extract16(size_t &i);

public:
    std::string hexString() const;

public:
    Dlt645Msg &&createSetPassword(uint64_t passwd) &&;
    Dlt645Msg &&createGetProductInfo() &&;
    Dlt645Msg &&createGetReadings(int phase) &&;
    Dlt645Msg &&createResetEnergy() &&;

private:
    void Init(const Buffer &msg);
    uint8_t Checksum() const;
};

#endif // DLT645MSG_H
