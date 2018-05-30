#ifndef LOGINTERFACE_H
#define LOGINTERFACE_H

#include "Logger.h"
#include "LogStream.h"

using sprintf_type = void(*)(const char *format, ...);

template<class Mod, class Time = LogTime>
struct LogInterface
{
    static LogStream<Logger::Error, Mod, Time> error() {
        return LogStream<Logger::Error, Mod, Time>();
    }

    template<class... Args>
    static void error(const char *format, Args&&... args) {
        LogStream<Logger::Error, Mod, Time>::sprintf(format, std::forward<Args>(args)...);
    }

    static LogStream<Logger::Warn, Mod, Time> warn() {
        return LogStream<Logger::Warn, Mod, Time>();
    }

    template<class... Args>
    static void warn(const char *format, Args&&... args) {
        LogStream<Logger::Warn, Mod, Time>::sprintf(format, std::forward<Args>(args)...);
    }

    static LogStream<Logger::Info, Mod, Time> info() {
        return LogStream<Logger::Info, Mod, Time>();
    }

    template<class... Args>
    static void info(const char *format, Args&&... args) {
        LogStream<Logger::Info, Mod, Time>::sprintf(format, std::forward<Args>(args)...);
    }

    static LogStream<Logger::Debug, Mod, Time> debug() {
        return LogStream<Logger::Debug, Mod, Time>();
    }

    template<class... Args>
    static void debug(const char *format, Args&&... args) {
        LogStream<Logger::Debug, Mod, Time>::sprintf(format, std::forward<Args>(args)...);
    }
};

#define INIT_LOGGER(module, tname, logger_to) \
    struct module##LogInfo { \
        static const char *name() { return #module; } \
        static Logger *logger() { return logger_to; } \
    }; \
    using tname = LogInterface<module##LogInfo>

#define DBG_INFO "[" <<  __FILE__ << ": " << __LINE__ << "] {" << __FUNCTION__ << "}: "

#endif // LOGINTERFACE_H
