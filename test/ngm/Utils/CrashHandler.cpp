#include "CrashHandler.h"

#include <execinfo.h>
#include <unistd.h>
#include "CloudManager.h"
#include "SqlDeviceMap.h"

CrashHandler::CrashHandler():
    m_cloud(nullptr)
{
    m_this = this;
}

CrashHandler *CrashHandler::m_this;

void CrashHandler::initialize()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &CrashHandler::abortHandler;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGABRT, &sa, NULL );
    sigaction( SIGSEGV, &sa, NULL );
    sigaction( SIGBUS,  &sa, NULL );
    sigaction( SIGILL,  &sa, NULL );
    sigaction( SIGFPE,  &sa, NULL );
    sigaction( SIGPIPE, &sa, NULL );
    sigaction( SIGHUP , &sa, NULL );
    sigaction( SIGINT , &sa, NULL );
    sigaction( SIGQUIT , &sa, NULL );
    sigaction( SIGTRAP , &sa, NULL );
    //sigaction( SIGIOT , &sa, NULL );
    sigaction( SIGKILL , &sa, NULL );
    sigaction( SIGALRM , &sa, NULL );
    sigaction( SIGTERM , &sa, NULL );
    sigaction( SIGSTKFLT , &sa, NULL );
    //sigaction( SIGCHLD , &sa, NULL );
    sigaction( SIGSTOP , &sa, NULL );
    sigaction( SIGTSTP , &sa, NULL );
    sigaction( SIGXCPU , &sa, NULL );
    sigaction( SIGVTALRM , &sa, NULL );
    sigaction( SIGPWR , &sa, NULL );
}

void CrashHandler::setCloudObj(CloudManager *obj)
{
    m_cloud = obj;
}

void CrashHandler::abortHandler(int signum, siginfo_t *, void *)
{
    // associate each signal with a signal name string.
    const char* name = NULL;

    if (m_this->m_cloud) {
        m_this->m_cloud->stop();
    }

    switch( signum )
    {
    case SIGABRT:   name = "SIGABRT";  break;
    case SIGSEGV:   name = "SIGSEGV";  break;
    case SIGBUS:    name = "SIGBUS";   break;
    case SIGILL:    name = "SIGILL";   break;
    case SIGFPE:    name = "SIGFPE";   break;
    case SIGPIPE:   name = "SIGPIPE";  break;
    case SIGHUP:    name = "SIGHUP";   break;
    case SIGINT:    name = "SIGINT";   break;
    case SIGQUIT:   name = "SIGQUIT";  break;
    case SIGTRAP:   name = "SIGTRAP";  break;
    case SIGKILL:   name = "SIGKILL";  break;
    case SIGALRM:   name = "SIGALRM";  break;
    case SIGTERM:   name = "SIGTERM";  break;
    case SIGSTKFLT: name = "SIGSTKFLT";break;
    case SIGSTOP:   name = "SIGSTOP";  break;
    case SIGTSTP:   name = "SIGTSTP";  break;
    case SIGXCPU:   name = "SIGXCPU";  break;
    case SIGVTALRM: name = "SIGVTALRM";break;
    case SIGPWR:    name = "SIGPWR";   break;
    }

    // Notify the user which signal was caught. We use printf, because this is the
    // most basic output function. Once you get a crash, it is possible that more
    // complex output systems like streams and the like may be corrupted. So we
    // make the most basic call possible to the lowest level, most
    // standard print function.
    if ( name )
       log::error( "Caught signal %d (%s)", signum, name );
    else
       log::error( "Caught signal %d", signum );

    // Dump a stack trace.
    m_this->printStackTrace();

    // If you caught one of the above signals, it is likely you just
    // want to quit your program right now.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    sync();

    if (g_bSqlNeedReset) {
        std::string cmd = "rm -Rf " + g_SqlDbFile;
        system(cmd.c_str());
    }

    exit( signum );
}

void CrashHandler::printStackTrace(unsigned int max_frames)
{
    log::error("stack trace:");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    uint32_t addrlen = backtrace( addrlist, sizeof( addrlist ) / sizeof( void* ));

    if ( addrlen == 0 )
    {
       log::error( "  " );
       return;
    }

    // create readable strings to each frame.
    char** symbollist = backtrace_symbols( addrlist, addrlen );

    // print the stack trace.
    for ( uint32_t i = 4; i < addrlen; i++ )
       log::error( "%s", symbollist[i] );

    free(symbollist);
}
