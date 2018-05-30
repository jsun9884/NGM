#ifndef HRSTATISTICS_H
#define HRSTATISTICS_H

#include <string>
#include <atomic>
#include <chrono>
#include <thread>
#include "Log.h"
#include "ParameterBase.h"

class Device;
class HRStatistics
{
    LOG_MODULE(HRStat, g_main_log);

    std::chrono::system_clock::time_point m_firstRunTime;
    bool m_isRunning;
    std::shared_ptr<std::thread> m_thread;
    Device *m_device;

    void threadProc();
    void addValues();

public:
    HRStatistics();
    ~HRStatistics();

    bool init(Device *device);
    bool stop();
};

class HRStatistics;
typedef std::shared_ptr<HRStatistics> HRStatisticsPtr;

#endif // HRSTATISTICS_H
