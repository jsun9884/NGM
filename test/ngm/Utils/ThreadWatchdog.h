#ifndef THREADWATCHDOG_H
#define THREADWATCHDOG_H

#include <mutex>
#include <chrono>

class ThreadWatchdog
{
public:
    ThreadWatchdog();
    ~ThreadWatchdog();

    void kick();
    std::chrono::system_clock::time_point lastKickTime();
    std::chrono::system_clock::duration elapsed();
    size_t secElapsed();
    size_t msecElapsed();

private:
    bool m_bStarted;
    std::mutex m_mutex;
    std::chrono::system_clock::time_point m_kickTime;
};

#endif // THREADWATCHDOG_H
