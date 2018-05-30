#include "HRStatistics.h"
#include "HardwareManager.h"
#include "Device.h"

#include <unistd.h>
#include <syscall.h>

extern "C" {
#include "aws_iot_log.h"
}

HRStatistics::HRStatistics():
    m_isRunning(false)
{

}

HRStatistics::~HRStatistics()
{
    stop();
}

bool HRStatistics::init(Device *device)
{
    if (m_isRunning || m_thread.get()) {
        log::error("HRStatistics already running!");
        return false;
    }

    m_device = device;
    m_isRunning = true;
    m_firstRunTime = std::chrono::system_clock::now();
    m_thread = std::make_shared<std::thread>(&HRStatistics::threadProc, this);

    return true;
}

void HRStatistics::threadProc()
{
    auto timeNow = std::chrono::system_clock::now();
    auto timeInterval = std::chrono::milliseconds(5000);
    auto nextTime = timeNow + timeInterval;

    log::info() << "Statistics thread for <" << m_device->getName() << "> is started! TPID: " <<  ::syscall(SYS_gettid);

    while(m_isRunning) {

        //log::debug() << DBG_INFO << m_deviceName;

        addValues();

        timeNow = std::chrono::system_clock::now();
        if (timeNow > nextTime) {
            log::warn() << "Statistics thread for " << m_device->getName() << " too slow!...";
            nextTime += timeInterval;
            if (timeNow >= nextTime) {
                nextTime = timeNow + timeInterval;
            }
        }
        else {
            std::this_thread::sleep_until(nextTime);
            nextTime += timeInterval;
        }
    }

    log::info() << "Statistics thread for " << m_device->getName() << " is stopped!";
}

void HRStatistics::addValues() {
    //log::info() << "HRStatistics::addValues....";

    if (m_device->state() == SensorState::Faulty) {
        return;
    }

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    bool isLongIntervalPassed =  std::chrono::duration_cast<std::chrono::minutes>(now - m_firstRunTime).count() > STAT_MAX_INTERVAL;
    bool isShortIntervalPassed = std::chrono::duration_cast<std::chrono::minutes>(now - m_firstRunTime).count() > STAT_MIN_INTERVAL;

    size_t count = m_device->m_hrData->getParametersCount();
    auto d = m_device->m_hrData->getCurrentData();

    bool bNeedLogStatCounters = true;
    auto lg = log::info();

    if (m_device->m_devStatEnabled) {
        for(size_t i = 0; i < count; ++i) {
            HRParameterPtr param = m_device->m_hrData->getParameter(i);
            if (param->getStatisticsEnabled()) {
                param->addValue(std::move(d.get(param)), isLongIntervalPassed, isShortIntervalPassed);

                if (bNeedLogStatCounters) {
                    bNeedLogStatCounters = false;
                    lg << "\'" << m_device->getName() << "\' statistics counters: ";
                }
                else {
                    lg << ", ";
                }

                lg << param->getLastValuesCount();
            }
        }
    }
}

bool HRStatistics::stop()
{
    if (m_thread.get()) {
        m_isRunning = false;
        m_thread->join();
        m_thread = nullptr;
        return true;
    }
    return false;
}
