#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <iostream>
#include <sys/ioctl.h>
#include "serialport.h"

extern "C" {
#include "aws_iot_log.h"
}

SerialPort::SerialPort():
    m_fd(0)
{
}

bool SerialPort::open(std::string portPath, int freq, bool isRTS, bool isDTR)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_fd) {
    	log::error("%s already opened!", portPath.c_str());
        return false;
    }

    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_cflag = CS8 | CREAD | CLOCAL;           // 8n1, see termios.h for more information
    tio.c_lflag = 0;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;

    m_fd = ::open(portPath.c_str(), O_RDWR | O_NOCTTY);
    if (m_fd < 0) {
    	log::error("open: %s failed... %d", portPath.c_str(), m_fd);
        m_fd = 0;
        return false;
    }

    speed_t speed;
    switch(freq) {
        case 9600:
            speed = B9600;
            break;

        case 19200:
            speed = B19200;
            break;

        case 38400:
            speed = B38400;
            break;

        case 57600:
            speed = B57600;
            break;

        case 115200:
            speed = B115200;
            break;

        case 230400:
            speed = B230400;
            break;

        case 460800:
            speed = B460800;
            break;

        case 921600:
            speed = B921600;
            break;

        case 1000000:
            speed = B1000000;
            break;

        case 1152000:
            speed = B1152000;
            break;

        case 2000000:
            speed = B2000000;
            break;

        case 3000000:
            speed = B3000000;
            break;

        case 4000000:
            speed = B4000000;
            break;

        default:
            speed = B115200;
            break;
    }

    cfsetospeed(&tio, speed);
    cfsetispeed(&tio, speed);

    if (tcsetattr(m_fd, TCSANOW, &tio) < 0) {
    	log::error("tcsetattr: %s failed...", portPath.c_str());
        ::close(m_fd);
        m_fd = 0;
        return false;
    }

    int flag;

    if (isRTS) {
        // Enable RTS
        flag = TIOCM_RTS;
        ioctl(m_fd, TIOCMBIC, &flag);
    }

    if (isDTR) {
        // Enable DTR
        flag = TIOCM_DTR;
        ioctl(m_fd, TIOCMBIC, &flag);
    }

    ioctl(m_fd, TIOCMGET, &flag);
    if (flag & TIOCM_RTS) {
    	log::debug("port: support RTS");
    }
    if (flag & TIOCM_DTR) {
    	log::debug("port: support DTR");
    }

    m_portPath = portPath;

    return true;
}


int SerialPort::read(uint8_t *buffer, size_t size, int timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    int ret;

    if (timeout > 0) {
        fd_set rset;
        struct timeval tv;

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        FD_ZERO(&rset);
        FD_SET(m_fd, &rset);

        ret = ::select(m_fd + 1, &rset, NULL, NULL, &tv);
        if (ret == 0) {
           return 0; // timeout
        }

        if (ret < 0 && errno != EINTR) {
            char buf[256];
            strerror_r(errno, buf, 256);
            log::error("read: %s select return error: %s", m_portPath.c_str(), buf);
            return ret; // error
        }
    }

    do {
        ret = ::read(m_fd, buffer, size);
        if(ret < 0) {
            if (errno == EINTR) {
                log::warn() << "read EINTR return: " << ret;
                continue;
            }
        }
        break;
    } while(true);

    if (ret < 0) {
        char buf[1024];
        strerror_r(errno, buf, 1024);
        log::error("read: %s error: %s", m_portPath.c_str(), buf);
    }

    return ret;
}

bool SerialPort::write(const uint8_t *buffer, size_t size)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    int ret = ::write(m_fd, buffer, size);
    if (ret != (int)size) {
    	log::error("write: %s failed...", m_portPath.c_str());
        return false;
    }
    return true;
}

std::string SerialPort::portPath()
{
    return m_portPath;
}

bool SerialPort::close()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_fd) {
    	log::error("%s is not opened!", m_portPath.c_str());
        return false;
    }

    if (::close(m_fd) < 0) {
    	log::error("close: %s failed...", m_portPath.c_str());
        m_fd = 0;
        return false;
    }
    m_fd = 0;
    return true;
}
