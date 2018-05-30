#include "SqlDeviceMap.h"

bool g_bSqlNeedReset = false;
std::string g_SqlDbFile;

SqlDeviceMap g_sqlDevMap[] = {
    { "PowerMeter", "voltage1", "native_registers_data", "V_A" },
    { "PowerMeter", "voltage2", "native_registers_data", "V_B" },
    { "PowerMeter", "voltage3", "native_registers_data", "V_C" },
    { "PowerMeter", "current1", "native_registers_data", "I_NET_A" },
    { "PowerMeter", "current2", "native_registers_data", "I_NET_B" },
    { "PowerMeter", "current3", "native_registers_data", "I_NET_C" },
    { "PowerMeter", "active_power1", "native_registers_data", "W_NET_A" },
    { "PowerMeter", "active_power2", "native_registers_data", "W_NET_B" },
    { "PowerMeter", "active_power3", "native_registers_data", "W_NET_C" },
    { "PowerMeter", "reactive_power1", "native_registers_data", "VAR_NET_A" },
    { "PowerMeter", "reactive_power2", "native_registers_data", "VAR_NET_B" },
    { "PowerMeter", "reactive_power3", "native_registers_data", "VAR_NET_C" },
    { "PowerMeter", "apparent_power1", "native_registers_data", "VA_NET_A" },
    { "PowerMeter", "apparent_power2", "native_registers_data", "VA_NET_B" },
    { "PowerMeter", "apparent_power3", "native_registers_data", "VA_NET_C" },
    { "PowerMeter", "power_factor1", "native_registers_data", "PF_NET_A" },
    { "PowerMeter", "power_factor2", "native_registers_data", "PF_NET_B" },
    { "PowerMeter", "power_factor3", "native_registers_data", "PF_NET_C" },
    { "PowerMeter", "frequency1", "native_registers_data", "FREQ_A" },
    { "PowerMeter", "frequency2", "native_registers_data", "FREQ_B" },
    { "PowerMeter", "frequency3", "native_registers_data", "FREQ_C" },
    { "PowerMeter", "consumed_active_power1", "native_registers_data", "WH_NET_A" },
    { "PowerMeter", "consumed_active_power2", "native_registers_data", "WH_NET_B" },
    { "PowerMeter", "consumed_active_power3", "native_registers_data", "WH_NET_C" },
    { "PowerMeter", "consumed_reactive_power1", "native_registers_data", "VARH_NET_A" },
    { "PowerMeter", "consumed_reactive_power2", "native_registers_data", "VARH_NET_B" },
    { "PowerMeter", "consumed_reactive_power3", "native_registers_data", "VARH_NET_C" },
    { "GPS", "latitude", "native_registers_data", "LATITUDE" },
    { "GPS", "longitude", "native_registers_data", "LONGITUDE" },
    { "GPS", "altitude", "native_registers_data", "ALTITUDE" },
    { "AccCore", "accX", "native_registers_data", "ACC_X" },
    { "AccCore", "accY", "native_registers_data", "ACC_Y" },
    { "AccCore", "accZ", "native_registers_data", "ACC_Z" },
    { "TempPCB", "temperature", "native_registers_data", "TEMP_PCB" },
    { "TempCPU", "temperature", "native_registers_data", "TEMP_CPU" },
    { nullptr, nullptr, nullptr, nullptr }
};
