#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>
#include <mutex>
#include <stdint.h>

#include "Log.h"

class SerialPort
{
    LOG_MODULE(Serial, g_main_log);
public:
    SerialPort();

    bool open(std::string portPath, int freq, bool isRTS = true, bool isDTR = true);
    int read(uint8_t *buffer, size_t size, int timeout);
    bool write(const uint8_t *buffer, size_t size);
    std::string portPath();
    bool close();

private:
    int m_fd;
    std::string m_portPath;
    std::mutex m_mutex;
};

#endif // SERIALPORT_H
