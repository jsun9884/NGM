#ifdef ENABLE_OBJECT_COUNTER

#include "ObjectCounter.h"

#include <syscall.h>
#include <unistd.h>

const char *ObjectCounter::m_names[] = { "AnyValue", "RecordData" };
ObjectCounter::ThreadsMap ObjectCounter::m_threads;

ObjectCounter::ObjectCounter(int index):
    m_index(index)
{
    static thread_local ThreadCounters counters(::syscall(SYS_gettid));
    counters.counters[index]++;
}

ObjectCounter::~ObjectCounter()
{
    ThreadCounters *obj = m_threads.get(::syscall(SYS_gettid));
    obj->counters[m_index]--;
}

void ObjectCounter::logOutCounters()
{
    m_threads.logOut();
}

ObjectCounter::ThreadCounters::ThreadCounters(int thread_id):
    tid(thread_id)
{
    for(int i = 0; i < END_INDEX; ++i)
        counters[i] = 0L;
    m_threads.insert(tid, this);
}

ObjectCounter::ThreadCounters::~ThreadCounters()
{
    m_threads.release(tid);
}

void ObjectCounter::ThreadsMap::insert(int tid, ObjectCounter::ThreadCounters *obj)
{
    std::lock_guard<std::mutex> l(m_mutex);
    m_threads.insert({tid, obj});
}

void ObjectCounter::ThreadsMap::release(int tid)
{
    std::lock_guard<std::mutex> l(m_mutex);
    auto it = m_threads.find(tid);
    if (it != m_threads.end()) {
        m_threads.erase(it);
    }
}

ObjectCounter::ThreadCounters *ObjectCounter::ThreadsMap::get(int tid)
{
    std::lock_guard<std::mutex> l(m_mutex);
    auto it = m_threads.find(tid);
    if (it == m_threads.end()) {
        return nullptr;
    }
    return (*it).second;
}

void ObjectCounter::ThreadsMap::logOut()
{
    auto out = log::info();
    out << "*** Object Counters:";
    for (auto it = m_threads.begin(), it_end = m_threads.end(); it != it_end; ++it) {
        out << "\n[" << (*it).first << "]: ";
        for (int i = 0; i < END_INDEX; ++i) {
            if (i != 0) out << ", ";
            out << m_names[i] << ": " << (*it).second->counters[i];
        }
    }
}

#endif
