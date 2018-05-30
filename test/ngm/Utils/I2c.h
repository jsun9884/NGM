#ifndef I2C_H
#define I2C_H

#include <memory>
#include <mutex>

#include "Log.h"

class I2c
{
    LOG_MODULE(I2c, g_main_log);

public:
    class control {
    private:
        I2c *m_owner;

    public:
        control(const control &) = delete;
        control(control&&r);
        control();
        control(I2c &owner);
        ~control();
        bool slave(int addr);
        bool read(void *data, size_t size);
        bool write(void *data, size_t size);
        operator bool() const;
    };

public:
    I2c();

    bool open(int bus);
    control acquire();
    void close();

private:
    int m_fd;
    int m_bus;
    std::recursive_mutex m_mutex;
};

typedef std::shared_ptr<I2c> I2cPtr;

#endif // I2C_H
