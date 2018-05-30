#include <chrono>
#include <ctime>
#include "LogStream.h"

thread_local ThreadBuffer<4096> pf_buffer;

const char *LogTime::timestr()
{
    using namespace std;
    using namespace std::chrono;

    static thread_local ThreadBuffer<1024> buffer;

    system_clock::time_point now = system_clock::now();
    system_clock::duration tp = now.time_since_epoch();

    time_t tt = system_clock::to_time_t(now);
    //tm utc_tm = *gmtime(&tt);
    tm local_tm = *localtime(&tt);

    seconds s = duration_cast<seconds>(tp);
    tp -= s;
    milliseconds ms = duration_cast<milliseconds>(tp);

    std::snprintf(buffer.data, 256, "%d-%02d-%02d %02d:%02d:%02d.%03d",
                  local_tm.tm_year + 1900,
                  local_tm.tm_mon + 1,
                  local_tm.tm_mday,
                  local_tm.tm_hour,
                  local_tm.tm_min,
                  local_tm.tm_sec,
                  (int)ms.count());
    return buffer.data;
}
