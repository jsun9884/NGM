#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <sstream>
#include <stdio.h>
#include <cstdarg>
#include "Logger.h"

struct LogTime
{
    static const char * timestr();
};

template<int Pri, class Mod, class Time = LogTime, class Enable = void>
class LogStream {};

template<int Pri, class Mod, class Time>
class LogStream<Pri, Mod, Time, typename std::enable_if<(Pri > MAX_LOG_PRIORITY), void>::type> {
public:
    LogStream() {}
    ~LogStream() {}

    template<class T>
    LogStream &operator<<(const T &e) {
        return *this;
    }

    static void sprintf(const char *format, ...) {}
};

template<int Pri, class Mod, class Time>
class LogStream<Pri, Mod, Time, typename std::enable_if<Pri <= MAX_LOG_PRIORITY, void>::type>
{
public:
    LogStream(): m_ss(new std::ostringstream) {
        *m_ss << Time::timestr() << " [" << Mod::name() << "]" << Logger::s_priorityNames[Pri] << ": ";
    }

    LogStream(LogStream &&r): m_ss(nullptr) {
        std::swap(m_ss, r.m_ss);
    }

    ~LogStream() {
        if (m_ss) {
            *m_ss << std::endl;
            Mod::logger()->append(m_ss->str());
            delete m_ss;
        }
    }

    template<class T>
    LogStream &operator<<(const T &e) {
        *m_ss << e;
        return *this;
    }

    template<class... Args>
    static void sprintf(const char *format, Args&&... args) {
        // gcc 4.9.3 BUG: crash in thread_local with templates
        //static thread_local ThreadBuffer<4096> buffer;
        char buffer[2048];

        int sz = std::snprintf(buffer, 1024, "%s [%s]%s: ", Time::timestr(), Mod::name(), Logger::s_priorityNames[Pri]);
        sz += std::snprintf(buffer + sz, 2046 - sz, format, std::forward<Args>(args)...);
        std::snprintf(buffer + sz, 2046 - sz, "\n");
        Mod::logger()->append(std::string(buffer));
    }

    static void sprintf(const char *format) {
        // gcc 4.9.3 BUG: crash in thread_local with templates
        //static thread_local ThreadBuffer<4096> buffer;
        char buffer[2048];

        int sz = std::snprintf(buffer, 1024, "%s [%s]%s: ", Time::timestr(), Mod::name(), Logger::s_priorityNames[Pri]);
        sz += std::snprintf(buffer + sz, 2046 - sz, "%s", format);
        std::snprintf(buffer + sz, 2046 - sz, "\n");
        Mod::logger()->append(std::string(buffer));
    }

private:
    std::ostringstream *m_ss;
};

#endif // LOGSTREAM_H
