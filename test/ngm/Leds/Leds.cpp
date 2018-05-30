#include <iostream>
#include <chrono>
#include <thread>
#include "Leds.h"
#include "lp55231.h"

Leds::Leds(I2cPtr i2c, int address):
    m_i2c(i2c), m_address(address), m_onOffCtrlMsb(0), m_onOffCtrlLsb(0)
{

}

bool Leds::initialize()
{
    log::info() << "Leds initializing...";
    I2c::control c = m_i2c->acquire();
    if (c) {
        c.slave(m_address);

        writeReg(c, LP5523_REG_ENABLE, LP5523_ENABLE);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));

        writeReg(c, LP5523_REG_CONFIG, LP5523_AUTO_INC | LP5523_PWR_SAVE | LP5523_CP_AUTO | LP5523_AUTO_CLK | LP5523_PWM_PWR_SAVE);
        readReg(c, LP5523_REG_ENABLE_LEDS_MSB, m_onOffCtrlMsb);
        readReg(c, LP5523_REG_ENABLE_LEDS_LSB, m_onOffCtrlLsb);
    }

}

bool Leds::setLedOn(int num, uint8_t r, uint8_t g, uint8_t b)
{
    I2c::control c = m_i2c->acquire();
    if (c) {
        c.slave(m_address);

        if (num == 0) {
            m_onOffCtrlLsb |= (1 | (1 << 1) | (1 << 6));
            writeReg(c, LP5523_REG_ENABLE_LEDS_LSB, m_onOffCtrlLsb);

            writeReg(c, LP5523_REG_LED_PWM_BASE, g);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 1, b);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 6, r);
        }
        if (num == 1) {
            m_onOffCtrlLsb |= ((1 << 2) | (1 << 3) | (1 << 7));
            writeReg(c, LP5523_REG_ENABLE_LEDS_LSB, m_onOffCtrlLsb);

            writeReg(c, LP5523_REG_LED_PWM_BASE + 2, g);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 3, b);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 7, r);
        }
        else if(num == 2) {
            m_onOffCtrlLsb |= ((1 << 4) | (1 << 5));
            writeReg(c, LP5523_REG_ENABLE_LEDS_LSB, m_onOffCtrlLsb);

            m_onOffCtrlMsb = 0x01;
            writeReg(c, LP5523_REG_ENABLE_LEDS_MSB, m_onOffCtrlMsb);

            writeReg(c, LP5523_REG_LED_PWM_BASE + 4, g);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 5, b);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 8, r);
        }
        else {
            return false;
        }
    }
    return true;
}

bool Leds::setLedOff(int num)
{
    I2c::control c = m_i2c->acquire();
    if (c) {
        c.slave(m_address);
        uint8_t v = 0;

        if (num == 0) {
            m_onOffCtrlLsb &= ~(1 | (1 << 1) | (1 << 6));
            writeReg(c, LP5523_REG_ENABLE_LEDS_LSB, m_onOffCtrlLsb);

            writeReg(c, LP5523_REG_LED_PWM_BASE, v);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 1, v);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 6, v);
        }
        if (num == 1) {
            m_onOffCtrlLsb &= ~((1 << 2) | (1 << 3) | (1 << 7));
            writeReg(c, LP5523_REG_ENABLE_LEDS_LSB, m_onOffCtrlLsb);

            writeReg(c, LP5523_REG_LED_PWM_BASE + 2, v);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 3, v);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 7, v);
        }
        else if(num == 2) {
            m_onOffCtrlLsb &= ~((1 << 4) | (1 << 5));
            writeReg(c, LP5523_REG_ENABLE_LEDS_LSB, m_onOffCtrlLsb);

            m_onOffCtrlMsb = 0x00;
            writeReg(c, LP5523_REG_ENABLE_LEDS_MSB, m_onOffCtrlMsb);

            writeReg(c, LP5523_REG_LED_PWM_BASE + 4, v);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 5, v);
            writeReg(c, LP5523_REG_LED_PWM_BASE + 8, v);
        }
        else {
            return false;
        }
    }
    return true;
}

bool Leds::initDisclosingSensor()
{
    I2c::control c = m_i2c->acquire();
    if (c) {
        if (!c.slave(m_address))
            return false;

        writeReg(c, 0x3b, 0x00);
        return true;
    }
    return false;
}

bool Leds::checkDisclosingSensor(bool &result)
{
    I2c::control c = m_i2c->acquire();
    if (c) {
        if (!c.slave(m_address))
            return false;

        uint8_t reg;
        readReg(c, 0x3b, reg);
        //log::info() << "...DisclosingSensor reg: " << std::hex << (unsigned int)reg << std::dec;
        if (reg & 0x01)
            result = true;
        else
            result = false;
        return true;
    }
    return false;
}

bool Leds::readRegister(uint8_t address, uint8_t &value)
{
    I2c::control c = m_i2c->acquire();
    if (c) {
        if (!c.slave(m_address))
            return false;

        return readReg(c, address, value);
    }
    return false;
}

bool Leds::writeRegister(uint8_t address, uint8_t value)
{
    I2c::control c = m_i2c->acquire();
    if (c) {
        if (!c.slave(m_address))
            return false;

        return writeReg(c, address, value);
    }
    return false;
}

bool Leds::writeReg(I2c::control &c, uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {0};
    buf[0] = reg;
    buf[1] = data;

    //log::info() << "Write reg(" << std::hex << (int)reg << "), data(" << (int)data << ")";
    return c.write(buf, 2);
}

bool Leds::readReg(I2c::control &c, uint8_t reg, uint8_t &data)
{
    bool ret = c.write(&reg, 1);
    if (ret) {
        ret = c.read(&data, 1);
    }
    //log::info() << "Readed reg(" << std::hex << (int)reg << "), data(" << (int)data << ")";
    return ret;
}
