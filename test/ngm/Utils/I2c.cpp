#include <iostream>
#include <sstream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "I2c.h"

I2c::I2c():m_fd(0), m_bus(-1)
{

}

bool I2c::open(int bus)
{
    if (m_fd) {
        log::error() << "I2C already opened...";
        return false;
    }

    std::stringstream ss;
    ss << "/dev/i2c-" << bus;
    std::string path = ss.str();

    m_fd = ::open(path.c_str(), O_RDWR);
    if (m_fd < 0) {
        log::error() << "I2C: open <" << path << "> failed...";
        m_fd = 0;
        return false;
    }

    m_bus = bus;
    return true;
}

I2c::control I2c::acquire()
{
    return control(*this);
}

void I2c::close()
{
    if (!m_fd) {
        return;
    }

    ::close(m_fd);
    m_fd = 0;
    m_bus = -1;
}

I2c::control::control(I2c::control &&r): m_owner(nullptr)
{
    std::swap(m_owner, r.m_owner);
}

I2c::control::control(): m_owner(nullptr)
{

}

I2c::control::control(I2c &owner): m_owner(&owner)
{
    m_owner->m_mutex.lock();
}

I2c::control::~control()
{
    if (m_owner)
        m_owner->m_mutex.unlock();
}

bool I2c::control::slave(int addr)
{
    if (!m_owner) {
        log::error() << "I2C slave: " << "Using empty control...";
        return false;
    }

    if (::ioctl(m_owner->m_fd, I2C_SLAVE, addr) < 0) {
        log::error() << "I2C" << m_owner->m_bus << ": failed to set slave address " << addr;
        return false;
    }
    return true;
}

bool I2c::control::read(void *data, size_t size)
{
    if (!m_owner) {
        log::error() << "I2C read: " << "Using empty control...";
        return false;
    }

    if (::read(m_owner->m_fd, data, size) != size) {
        log::error() << "I2C" << m_owner->m_bus << ": read failed, size: " << size << " (" << strerror(errno) << ")";
        return false;
    }
    return true;
}

bool I2c::control::write(void *data, size_t size)
{
    if (!m_owner) {
        log::error() << "I2C write: " << "Using empty control...";
        return false;
    }

    if (::write(m_owner->m_fd, data, size) != size) {
        log::error() << "I2C" << m_owner->m_bus << ": write failed, size: " << size << " (" << strerror(errno) << ")";
        return false;
    }
    return true;
}

I2c::control::operator bool() const
{
    return m_owner != nullptr;
}
