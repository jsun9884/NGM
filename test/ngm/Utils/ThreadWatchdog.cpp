#include "ThreadWatchdog.h"

ThreadWatchdog::ThreadWatchdog():
    m_bStarted(false),
    m_kickTime(std::chrono::system_clock::now())
{

}

ThreadWatchdog::~ThreadWatchdog()
{

}

void ThreadWatchdog::kick()
{
    std::lock_guard<std::mutex> l(m_mutex);
    m_kickTime = std::chrono::system_clock::now();
    if (!m_bStarted) m_bStarted = true;
}

std::chrono::system_clock::time_point ThreadWatchdog::lastKickTime()
{
    std::lock_guard<std::mutex> l(m_mutex);
    return m_kickTime;
}

std::chrono::system_clock::duration ThreadWatchdog::elapsed()
{
    return (m_bStarted ? (std::chrono::system_clock::now() - m_kickTime): std::chrono::system_clock::duration(0));
}

size_t ThreadWatchdog::secElapsed()
{
    return std::chrono::duration_cast<std::chrono::seconds>(elapsed()).count();
}

size_t ThreadWatchdog::msecElapsed()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed()).count();
}
