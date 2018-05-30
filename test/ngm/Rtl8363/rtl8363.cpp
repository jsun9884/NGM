#include "rtl8363.h"

#include <fcntl.h>      /* open */
#include <unistd.h>
#include <sys/ioctl.h>  /* ioctl */

#include "rtl8367_ioctl.h"

#define DEVICE_FILE_NAME RTL8367_DEVPATH


Rtl8363::Rtl8363(std::string name):
    Device(name),
    m_fd(-1)
{
    log::info() << "Rtl8363 was created.";
}

Rtl8363::~Rtl8363()
{
    log::info() << "Rtl8363 destroyed.";
}

void Rtl8363::initialize()
{
    log::info() << "Initializing...";
}

bool Rtl8363::connect()
{
    log::info() << "Connecting to <" << DEVICE_FILE_NAME << "> character device...";
    if (m_fd >=0) {
        log::error() << "<" << DEVICE_FILE_NAME << "> already opened!";
        return false;
    }

    m_fd = ::open(DEVICE_FILE_NAME, 0);
    if (m_fd < 0) {
        log::error() << "Can't open <" << DEVICE_FILE_NAME << ">...";
        return false;
    }
    checkHrParameters();
    return true;
}

bool Rtl8363::readData(HRData::RecordData &data)
{
    // TODO: reading rx/tx counter via ioctrl
    port_bytes_t pb;

    data.set(m_hrParameters.m_timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    if (!getPortStatistics((1<<0), &pb)) {
        data.set(m_hrParameters.m_portA_rx, 0);
        data.set(m_hrParameters.m_portA_tx, 0);
        return false;
    }

    data.set(m_hrParameters.m_portA_rx, pb.RX);
    data.set(m_hrParameters.m_portA_tx, pb.TX);

    //log::info() << "got portA bytes: rx = " << pb.RX << ", tx = " << pb.TX;

    if (!getPortStatistics((1<<2), &pb)) {
        data.set(m_hrParameters.m_portB_rx, 0);
        data.set(m_hrParameters.m_portB_tx, 0);
        return false;
    }

    data.set(m_hrParameters.m_portB_rx, pb.RX);
    data.set(m_hrParameters.m_portB_tx, pb.TX);

    //log::info() << "got portB bytes: rx = " << pb.RX << ", tx = " << pb.TX;

    /*for(int i = 0; i < 8; ++i) {
        if (getPortStatistics((1<<i), &pb)) {
            log::info() << "Port" << i << ": " << "rx = " << pb.RX << ", tx = " << pb.TX;
        }
    }*/

    return true;
}

bool Rtl8363::disconnect()
{
    if (m_fd < 0) {
        log::warn() << "<" << DEVICE_FILE_NAME << "> not opened...";
        return false;
    }
    ::close(m_fd);
    m_fd = -1;
    return true;
}

void Rtl8363::reset()
{
    log::info() << "Reset counters!!!";
    int req = RTL8367_IOCTL_STATUS_CNT_RESET_ALL | (0 << RTL8367_IOCTL_CMD_LENGTH_BITS);
    int ret = ioctl(m_fd, req, NULL);
    if (ret < 0) {
        log::error() << "ioctrl RTL8367_IOCTL_STATUS_CNT_RESET_ALL return error: " << ret;
    }
}

bool Rtl8363::applyNativeParameter(ParameterPtr param)
{
    bool ret = false;
    log::info() << param->getName() << " = " << param->getValue();
    if (param->getName() == "PortAEnabled") {
        ret = setPortEnabled(param->getValue(), 0x01);
    }
    else if (param->getName() == "PortBEnabled") {
        ret = setPortEnabled(param->getValue(), (0x01 << 2));
    }
    else if (param->getName() == "rgmiiRxDelay") {
        ret = setRgmiiRxDelay(param->getValue());
    }
    else if (param->getName() == "rgmiiTxDelay") {
        ret = setRgmiiTxDelay(param->getValue());
    }

    return ret;
}

void Rtl8363::checkHrParameters()
{
    m_hrParameters.m_portA_rx = m_hrData->getParameter("PortA_rx");
    m_hrParameters.m_portA_tx = m_hrData->getParameter("PortA_tx");
    m_hrParameters.m_portB_rx = m_hrData->getParameter("PortB_rx");
    m_hrParameters.m_portB_tx = m_hrData->getParameter("PortB_tx");
    m_hrParameters.m_timestamp = m_hrData->getParameter("timestamp");
}

bool Rtl8363::getPortStatistics(int port, void *data)
{
    port_bytes_t *pb = (port_bytes_t *)data;
    int req = RTL8367_IOCTL_STATUS_PORT_BYTES | (port << RTL8367_IOCTL_CMD_LENGTH_BITS);
    int ret = ioctl(m_fd, req, pb);
    if (ret < 0) {
        log::error() << "ioctrl RTL8367_IOCTL_STATUS_PORT_BYTES return error: " << ret;
        return false;
    }
    return true;
}

bool Rtl8363::setPortEnabled(bool enabled, int port)
{
    int en = (enabled ? 1 : 0);
    int req = RTL8367_IOCTL_PORTS_POWER | (en << RTL8367_IOCTL_CMD_LENGTH_BITS);
    int ret = ioctl(m_fd, req, &port);
    if (ret < 0) {
        log::error() << "ioctrl RTL8367_IOCTL_PORTS_POWER return error: " << ret;
        return false;
    }
    return true;
}

bool Rtl8363::setRgmiiRxDelay(int delay)
{
    log::info() << "setRgmiiRxDelay " << delay;
    int req = RTL8367_IOCTL_RGMII_DELAY_RX | (0 << RTL8367_IOCTL_CMD_LENGTH_BITS);
    int ret = ioctl(m_fd, req, &delay);
    if (ret < 0) {
        log::error() << "ioctrl RTL8367_IOCTL_RGMII_DELAY_RX return error: " << ret;
        return false;
    }
    return true;
}

bool Rtl8363::setRgmiiTxDelay(int delay)
{
    log::info() << "setRgmiiRxDelay " << delay;
    int req = RTL8367_IOCTL_RGMII_DELAY_TX | (0 << RTL8367_IOCTL_CMD_LENGTH_BITS);
    int ret = ioctl(m_fd, req, &delay);
    if (ret < 0) {
        log::error() << "ioctrl RTL8367_IOCTL_RGMII_DELAY_TX return error: " << ret;
        return false;
    }
    return true;
}

