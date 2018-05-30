#include <sstream>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "Uio.h"

Uio::Uio(int index):
    m_index(index),
    m_fd(-1)
{}

bool Uio::open()
{
    std::stringstream ss;
    ss << "/dev/uio" << m_index;

    m_fd = ::open(ss.str().c_str(), O_RDWR);
    if (m_fd < 0) {
        log::error() << "open \'" << ss.str() << "\' failed...";
        return false;
    }

    return true;
}

bool Uio::poll(int timeout)
{
    uint32_t info = 1;

    int sz = ::write(m_fd, &info, sizeof(info));
    if (sz < sizeof(info)) {
        log::error() << "write to \'/dev/uio" << m_index << "\' failed...";
        return false;
    }

    struct pollfd fds = {0};

    fds.fd = m_fd;
    fds.events = POLLIN;

    sz = ::poll(&fds, 1, timeout);
    if (sz >= 1) {
        sz = ::read(m_fd, &info, sizeof(info));
        if (sz == sizeof(info)) {

            log::info() << "/dev/uio" << m_index << " got Interrupt!!!\n";
            return true;
        }
    }

    return false;
}

void Uio::close()
{
    if (m_fd > 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}
