#ifndef LEDS_H
#define LEDS_H

#include <memory>
#include <stdint.h>

#include "Log.h"
#include "I2c.h"

class Leds
{
    LOG_MODULE(Leds, g_main_log);
public:
    Leds(I2cPtr i2c, int address);

    bool initialize();
    bool setLedOn(int num, uint8_t r, uint8_t g, uint8_t b);
    bool setLedOff(int num);

    bool initDisclosingSensor();
    bool checkDisclosingSensor(bool &result);

    bool readRegister(uint8_t address, uint8_t &value);
    bool writeRegister(uint8_t address, uint8_t value);

protected:
    bool writeReg(I2c::control &c, uint8_t reg, uint8_t data);
    bool readReg(I2c::control &c, uint8_t reg, uint8_t& data);

private:
    I2cPtr m_i2c;
    int m_address;
    uint8_t m_onOffCtrlMsb;
    uint8_t m_onOffCtrlLsb;
};

typedef std::shared_ptr<Leds> LedsPtr;

#endif // LEDS_H
