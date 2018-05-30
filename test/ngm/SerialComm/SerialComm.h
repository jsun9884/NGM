#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include <thread>
#include <string>
#include <atomic>
#include <mutex>

#include "Log.h"
#include "serialport.h"

class SerialComm
{
    LOG_MODULE(SerialComm, g_main_log);

private:
    bool m_isRunning;
    SerialPort m_port1;
    SerialPort m_port2;
    std::string m_portPath1;
    std::string m_portPath2;
    std::shared_ptr<std::thread> m_commThread;
    uint8_t m_buffer[200];

    void threadProc();
    bool SendData(SerialPort &source, SerialPort &target);

public:
    SerialComm(std::string portPath1, std::string portPath2);

    bool init();
    void join();
    bool stop();
};

#endif // SERIALCOMM_H
