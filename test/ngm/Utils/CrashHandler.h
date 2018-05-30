#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include "Log.h"

#include <signal.h>
#include <stdint.h>

class CloudManager;
class CrashHandler
{
    LOG_MODULE(CrashHandler, g_main_log);
public:
    CrashHandler();

    void initialize();
    void setCloudObj(CloudManager *obj);

private:
    static void abortHandler(int signum, siginfo_t *, void *);
    void printStackTrace(unsigned int max_frames = 63);
    CloudManager *m_cloud;
    static CrashHandler *m_this;
};

#endif // CRASHHANDLER_H
