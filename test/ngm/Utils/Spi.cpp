#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "Spi.h"

extern "C" {
#include "aws_iot_log.h"
}

Spi::Spi(): m_fd(0), m_bitsPerWord(0)
{

}

bool Spi::open(std::string devPath, uint8_t iocWrMode, uint8_t bitsPerWord, uint32_t maxSpeed)
{
    std::lock_guard<std::mutex> l(m_mutex);
    if(m_fd) {
        log::error("%s already opened...", devPath.c_str());
        return false;
    }

    m_fd = ::open(devPath.c_str(), O_RDWR);
    if (m_fd == -1) {
        log::error("can't open %s", devPath.c_str());
        m_fd = 0;
        return false;
    }

    if (::ioctl(m_fd, SPI_IOC_WR_MODE, &iocWrMode) == -1) {
        log::error("ioctl SPI_IOC_WR_MODE failed");
        ::close(m_fd);
        m_fd = 0;
        return false;
    }

    if (::ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) == -1) {
        log::error("ioctl SPI_IOC_WR_BITS_PER_WORD failed");
        ::close(m_fd);
        m_fd = 0;
        return false;
    }
    m_bitsPerWord = bitsPerWord;

    if (::ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &maxSpeed) == -1) {
        log::error("ioctl SPI_IOC_WR_MAX_SPEED_HZ failed");
        ::close(m_fd);
        m_fd = 0;
        return false;
    }
    m_devPath = devPath;

    return true;
}

bool Spi::tx_rx(uint8_t *tx, uint32_t tx_sz, uint8_t *rx, uint32_t rx_sz, bool doMultiTransactions)
{
    struct spi_ioc_transfer tr[2];//[size];
    uint8_t word_size_bytes = m_bitsPerWord / 8;

    memset(tr, 0, 2 * sizeof(struct spi_ioc_transfer));

    //for(int i=0; i < size; i++)
    {
        tr[0].tx_buf = (unsigned long)tx;
        tr[0].len = tx_sz;
        tr[1].rx_buf = (unsigned long)rx;
        tr[1].len = rx_sz; //word_size_bytes;
        //delay_usecs: couldn't understand the meaning of this parameter
        //tr[i].delay_usecs = 0;
        //tr[i].speed_hz = speed;
        //tr[i].bits_per_word = spi_devs[dev_id].bits;
        //cs_change: this changes the CS for ~7 usec to the opposite of the current value (0/1) after each transfer.
        // note: if the last transfer is set to 1 then the CS will be 0 after transfer!! it seems that there is a race between this cs_chnage and the end of message cs_change!
        if (doMultiTransactions)
        {
            tr[0].cs_change = 1; //( i == (size - 1) ) ? 0 : 1;
            tr[1].cs_change = 0;
        }
    }

    std::lock_guard<std::mutex> l(m_mutex);
    if (::ioctl(m_fd, SPI_IOC_MESSAGE(rx_sz ? 2 : 1), tr) == -1) {
        log::error("ioctl SPI_IOC_MESSAGE failed");
        return false;
    }
    return true;
}

std::string Spi::devPath()
{
    return m_devPath;
}

void Spi::close()
{
    std::lock_guard<std::mutex> l(m_mutex);
    if (m_fd) {
        ::close(m_fd);
        m_devPath = "";
        m_fd = 0;
        m_bitsPerWord = 0;
    }
}

