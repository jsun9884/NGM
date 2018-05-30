#ifndef SPI_H
#define SPI_H

#include <string>
#include <mutex>
#include <memory>

#include <stdint.h>
#include <linux/spi/spidev.h>

#include "Log.h"

class Spi
{
    LOG_MODULE(Spi, g_main_log);

public:
    Spi();
    bool open(std::string devPath, uint8_t iocWrMode = SPI_MODE_0, uint8_t bitsPerWord = 8, uint32_t maxSpeed = 2000000);
    bool tx_rx(uint8_t *tx, uint32_t tx_sz, uint8_t *rx, uint32_t rx_sz, bool doMultiTransactions);
    std::string devPath();
    void close();

private:
    std::mutex m_mutex;
    std::string m_devPath;
    int m_fd;
    uint8_t m_bitsPerWord;
};

#endif // SPI_H
