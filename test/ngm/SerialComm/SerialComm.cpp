#include "SerialComm.h"
extern "C" {
#include "aws_iot_log.h"
}

SerialComm::SerialComm(std::string portPath1, std::string portPath2):
    m_portPath1(portPath1),
    m_portPath2(portPath2),
    m_isRunning(false)
{}

bool SerialComm::init()
{
    if (m_isRunning || m_commThread.get()) {
        log::error("SerialComm already running!");
        return false;
    }
    m_isRunning = true;
    m_commThread = std::make_shared<std::thread>(&SerialComm::threadProc, this);
    return true;
}
void SerialComm::threadProc()
{
    if (!m_port1.open(m_portPath1, 57600)) {
        glog::error ("Can't open %s...", m_portPath1.c_str());
        return;
    }
    if (!m_port2.open(m_portPath2, 57600)) {
        glog::error ("Can't open %s...", m_portPath2.c_str());
        return;
    }

    while(m_isRunning) {
        SendData(m_port1, m_port2);
        SendData(m_port2, m_port1);
    }

    m_port1.close();
    m_port2.close();
}

bool SerialComm::SendData(SerialPort &source, SerialPort &target)
{
    int sz = source.read(m_buffer, 200, 1);
    if(sz){
        if(target.write(m_buffer, sz) != sz){
            log::error ("Error writing to port %s", target.portPath().c_str());
        }
        else {
            //log::info("%d bytes from %s to %s", sz, source.portPath().c_str(), target.portPath().c_str());
        }
    }
    else{
        //log::error ("Error reading from port");
    }
}

void SerialComm::join()
{
    m_commThread->join();
}

bool SerialComm::stop()
{
    if (m_commThread.get()) {
        m_isRunning = false;
        m_commThread->join();
        m_commThread = nullptr;
        return true;
    }
    return false;
}
