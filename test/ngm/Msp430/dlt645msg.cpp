#include "dlt645msg.h"
#include "string.h"
#include <stdio.h>
#include <chrono>
#include <iostream>

Dlt645Msg::Dlt645Msg():
    Buffer(512)
{
}

Dlt645Msg::Dlt645Msg(Dlt645Msg &&msg):
    Buffer(std::move(msg))
{
}

Dlt645Msg& Dlt645Msg::operator=(Dlt645Msg &&o)
{
    m_vec = std::move(o.m_vec);
    return *this;
}

bool Dlt645Msg::CheckMessage()
{
    long i = size() - 2;

    if (i < 0 || m_vec[i++] != Checksum()) {
        return false;
    }

    if (m_vec[i] != 0x16) {
        return false;
    }

    return true;
}

int32_t Dlt645Msg::Extract32(size_t &i)
{
    uint32_t value;
    const uint8_t *s = &m_vec[i];

    value = (s[3] << 24) | (s[2] << 16) | (s[1] << 8) | s[0];
    i += 4;
    return value;
}

int16_t Dlt645Msg::Extract16(size_t &i)
{
    uint16_t value;
    const uint8_t *s = &m_vec[i];

    value = (s[1] << 8) | s[0];
    i += 2;
    return value;
}

Dlt645Msg&& Dlt645Msg::createSetPassword(uint64_t passwd) &&
{
    Buffer msg(128);
    msg.append(HOST_CMD_SET_PASSWORD);
    msg.append(0x00);
    msg.append((passwd >> 48) & 0xFF);
    msg.append((passwd >> 56) & 0xFF);
    msg.append((passwd >> 32) & 0xFF);
    msg.append((passwd >> 40) & 0xFF);
    msg.append((passwd >> 16) & 0xFF);
    msg.append((passwd >> 24) & 0xFF);
    msg.append((passwd >> 0) & 0xFF);
    msg.append((passwd >> 8) & 0xFF);
    Init(msg);
    return std::move(*this);
}

Dlt645Msg&& Dlt645Msg::createGetProductInfo() &&
{
    Buffer msg(16);
    msg.append((uint8_t)HOST_CMD_GET_METER_CONFIGURATION);
    msg.append(0x00);
    Init(msg);
    return std::move(*this);
}

Dlt645Msg &&Dlt645Msg::createGetReadings(int phase) &&
{
    Buffer msg(16);
    msg.append((uint8_t)(HOST_CMD_GET_NGM_PHASE_1 + phase));
    msg.append(0x00);
    Init(msg);
    return std::move(*this);
}

Dlt645Msg &&Dlt645Msg::createResetEnergy() &&
{
    Buffer msg(16);
    msg.append((uint8_t)(HOST_CMD_RESET_ENERGY));
    msg.append(0x00);
    Init(msg);
    return std::move(*this);
}

void Dlt645Msg::Init(const Buffer &msg)
{
    append(0xFE, 4);
    append(0x68);
    append(0x99, 6);
    append(0x68);
    append(0x23);
    append((uint8_t)msg.size());
    append(msg);
    append(Checksum());
    append(0x16);
}

std::string Dlt645Msg::hexString() const
{
	size_t len = size() * 3;
	char buf[len];
	for (size_t i = 0; i < size(); ++i) {
		sprintf(buf + 3 * i, "%02x ", m_vec[i]);
	}
	return std::string(buf, len);
}

bool Dlt645Msg::getReadings(int phase, HRData::RecordData &d, const Msp430HRParameters &p)
{
	size_t i = 16;
    int32_t value;

    //log::info() << "*** TI Start reading message...";
    if (phase == 0) {
        // V_rms
        d.set(p.m_voltage1, Extract16(i) / 100.0);

        // I_rms
        d.set(p.m_current1, Extract16(i) / 1000.0);

        value = -Extract32(i);
        //if (value < 0) value = -value;
        d.set(p.m_activePower1, (value / 100.0));

        d.set(p.m_reactivePower1, Extract32(i) / 100.0);

        d.set(p.m_apparentPower1, Extract32(i) / 100.0);

        d.set(p.m_powerFactor1, (value >= 0 ? -(Extract16(i) / 10000.0) : Extract16(i) / 10000.0));

        d.set(p.m_frequency1, Extract16(i) / 100.0);

        d.set(p.m_consumedActivePower1, (Extract32(i) / 100.0) / 64.0);

        d.set(p.m_consumedReactivePower1, (Extract32(i) / 100.0) / 64.0);
    }
    else if (phase == 1) {
        // V_rms
        d.set(p.m_voltage2, Extract16(i) / 100.0);

        // I_rms
        d.set(p.m_current2, Extract16(i) / 1000.0);

        value = -Extract32(i);
        //if (value < 0) value = -value;
        d.set(p.m_activePower2, (value / 100.0));

        d.set(p.m_reactivePower2, Extract32(i) / 100.0);

        d.set(p.m_apparentPower2, Extract32(i) / 100.0);

        d.set(p.m_powerFactor2, (value >= 0 ? -(Extract16(i) / 10000.0) : Extract16(i) / 10000.0));

        d.set(p.m_frequency2, Extract16(i) / 100.0);

        d.set(p.m_consumedActivePower2, (Extract32(i) / 100.0) / 64.0);

        d.set(p.m_consumedReactivePower2, (Extract32(i) / 100.0) / 64.0);
    }
    else if (phase == 2) {
        // V_rms
        d.set(p.m_voltage3, Extract16(i) / 100.0);

        // I_rms
        d.set(p.m_current3, Extract16(i) / 1000.0);

        value = -Extract32(i);
        //if (value < 0) value = -value;
        d.set(p.m_activePower3, (value / 100.0));

        d.set(p.m_reactivePower3, Extract32(i) / 100.0);

        d.set(p.m_apparentPower3, Extract32(i) / 100.0);

        d.set(p.m_powerFactor3, (value >= 0 ? -(Extract16(i) / 10000.0) : Extract16(i) / 10000.0));

        d.set(p.m_frequency3, Extract16(i) / 100.0);

        d.set(p.m_consumedActivePower3, (Extract32(i) / 100.0) / 64.0);

        d.set(p.m_consumedReactivePower3, (Extract32(i) / 100.0) / 64.0);
    }

    //d.set(p.m_timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());


    //log::info().precision(3);
    //log::info() << "*** TI Message readed!...";


	return true;
}

uint8_t Dlt645Msg::Checksum() const
{
    long i = 0;
    long end = size();

    for (; i < end; ++i) {
        if (m_vec[i] == 0x68) {
            break;
        }
    }
    if (i >= end) return 0;

    int sum = 0;
    long j = i + 9;
    do {
        sum += m_vec[i++];
    } while (i < j && i < end);

    if (i >= end) return 0;

    for (j = i + m_vec[i] + 1; i < j && i < end; ) {
        sum += m_vec[i++];
    }

    return sum & 0xff;
}

