#include "Logger.h"
#include <iostream>

const char *Logger::s_priorityNames[] = { " Error", " Warn", "", "" };

Logger::Logger():
    m_bRunning(false)
{}

Logger::~Logger()
{
    stop();
}

void Logger::addWriter(LogWriterPtr writer)
{
    if (!m_bRunning)
        m_writers.push_back(writer);
}

void Logger::start()
{
    if (!m_bRunning) {
        for (int i = 0, sz = m_writers.size(); i < sz; ++i) {
            m_writers[i]->open();
        }
        m_bRunning = true;
        m_thread = std::make_shared<std::thread>(&Logger::logLoop, this);
    }
}

void Logger::stop()
{
    if (m_bRunning) {
        m_bRunning = false;
        m_cond.notify_one();
        m_thread->join();
        for (int i = 0, sz = m_writers.size(); i < sz; ++i) {
            m_writers[i]->close();
        }
    }
}

void Logger::append(std::string message)
{
    std::lock_guard<std::mutex> l(m_mutex);
    m_inMsg.push(message);
    if (m_inMsg.size() > MAX_LOG_QUEUE_SIZE) {
        m_inMsg.pop();
    }
    m_cond.notify_one();
}

void Logger::logLoop()
{
    while (m_bRunning) {
        {
            std::unique_lock<std::mutex> l(m_mutex);
            if (!m_inMsg.size()) {
                m_cond.wait(l);
                if (!m_inMsg.size())
                    continue;
            }
            m_inMsg.swap(m_outMsg);
        }
        if (m_outMsg.size() > MAX_LOG_QUEUE_SIZE) {
            std::cout << "WARNING: LOGGER QUEUE > " << MAX_LOG_QUEUE_SIZE << std::endl;
        }
        while (m_outMsg.size()) {
            std::string msg = m_outMsg.front();
            for(int i = 0, sz = m_writers.size(); i < sz; ++i) {
                if (!m_writers[i]->write(msg)) {
                    m_writers[i]->close();
                    if (m_writers[i]->open()) {
                        m_writers[i]->write(msg);
                    }
                }
            }
            m_outMsg.pop();
        }
    }
}
