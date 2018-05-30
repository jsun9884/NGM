#ifndef OBJECTCOUNTER_H
#define OBJECTCOUNTER_H

#ifdef ENABLE_OBJECT_COUNTER

#include "Log.h"
#include <memory>
#include <mutex>
#include <unordered_map>

class ObjectCounter
{
    LOG_MODULE(ObjectCounter, g_object_counter_log);

public:
    enum {
        ANY_VALUE = 0,
        RECORD_DATA,
        END_INDEX
    };

private:
    struct ThreadCounters
    {
        const int tid;
        long counters[END_INDEX];
        ThreadCounters(int thread_id);
        ~ThreadCounters();
    };

    class ThreadsMap
    {
    public:
        void insert(int tid, ThreadCounters *obj);
        ThreadCounters *get(int tid);
        void logOut();
        void release(int tid);

    private:
        std::mutex m_mutex;
        std::unordered_map<int, ThreadCounters *> m_threads;
    };

public:
    ObjectCounter(int index);
    ~ObjectCounter();

    static void logOutCounters();

private:
    int m_index;

    static const char *m_names[];
    static ThreadsMap m_threads;
};

#endif // ENABLE_OBJECT_COUNTER

#endif // OBJECTCOUNTER_H
