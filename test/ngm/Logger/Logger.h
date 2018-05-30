#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>

#define MAX_LOG_QUEUE_SIZE 50
#define MAX_LOG_PRIORITY Logger::Debug

#include "LogWriter.h"

template<int N>
struct ThreadBuffer
{
    char *data;
    ThreadBuffer(): data(nullptr) { data = new char[N]; }
    ~ThreadBuffer() { delete [] data; }
};

class Logger
{
public:
    enum {
        Error = 0,
        Warn = 1,
        Info = 2,
        Debug = 3
    };
    static const char *s_priorityNames[];

public:
    Logger();
    ~Logger();

    void addWriter(LogWriterPtr writer);

    void start();
    void stop();

    void append(std::string message);

private:
    void logLoop();

private:
    bool m_bRunning;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<std::string> m_inMsg;
    std::queue<std::string> m_outMsg;
    std::vector<LogWriterPtr> m_writers;
    std::shared_ptr<std::thread> m_thread;
};

typedef std::shared_ptr<Logger> LoggerPtr;

#endif // LOGGER_H
