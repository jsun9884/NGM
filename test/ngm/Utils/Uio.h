#ifndef UIO_H
#define UIO_H

#include <memory>
#include "Log.h"

class Uio
{
    LOG_MODULE(Uio, g_main_log);
public:
    Uio(int index);
    bool open();
    bool poll(int timeout = -1);
    void close();

private:
    int m_index;
    int m_fd;
};

typedef std::shared_ptr<Uio> UioPtr;

#endif // UIO_H
