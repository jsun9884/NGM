#include "Accelerometer.h"
#include "lsm6ds3.h"

extern "C" {
#include "aws_iot_log.h"
}

Accelerometer::Accelerometer(std::string name, std::string devPath, bool bInvert):
    Device(name),
    m_bInvert(bInvert),
    m_devPath(devPath),
    m_algorithm()
{

}

Accelerometer::~Accelerometer()
{

}

void Accelerometer::initialize()
{

}

bool Accelerometer::connect()
{
    if (!m_spi.open(m_devPath))
        return false;

    if (!chipConnect()) {
        m_spi.close();
        return false;
    }

    checkParameters();

    if (m_hrParams.m_accX && m_params.m_useCalibOffsetX) {
        m_hrParams.m_accX->setThresholdHigh(m_params.m_triggerOffset);
        m_hrParams.m_accX->setThresholdLow(-m_params.m_triggerOffset);
    }
    if (m_hrParams.m_accY && m_params.m_useCalibOffsetY) {
        m_hrParams.m_accY->setThresholdHigh(m_params.m_triggerOffset);
        m_hrParams.m_accY->setThresholdLow(-m_params.m_triggerOffset);
    }
    if (m_hrParams.m_accZ && m_params.m_useCalibOffsetZ) {
        m_hrParams.m_accZ->setThresholdHigh(m_params.m_triggerOffset);
        m_hrParams.m_accZ->setThresholdLow(-m_params.m_triggerOffset);
    }

    m_algorithm = std::make_shared<Algorithm>(this, m_bInvert);
    m_algorithm->reset(m_params);
    return true;
}

bool Accelerometer::readData(HRData::RecordData &data)
{
    m_algorithm->Task();
    m_algorithm->getData(data, m_hrParams);
    return true;
}

bool Accelerometer::disconnect()
{
    m_algorithm.reset();
    m_spi.close();
    return true;
}

void Accelerometer::reset()
{
    m_algorithm->reset(m_params);
}

AnyValue Accelerometer::readRegister(AnyValue address)
{
    return read(address);
}

void Accelerometer::writeRegister(AnyValue address, AnyValue value)
{
    write(address, value);
}

bool Accelerometer::applyNativeParameter(ParameterPtr param)
{
    return false;
}

bool Accelerometer::onOverBounds(HRParameterPtr param, AnyValue &value, uint64_t ts)
{
    bool ret = true;
    //m_algorithm->reset(m_params);
    if (m_hrParams.m_accX->eventInProcess() || m_hrParams.m_accY->eventInProcess() || m_hrParams.m_accZ->eventInProcess()) {
        ret = false;
    }
    log::info() << "[" << getName() << "] " << "onOverBounds: return " << (ret ? "true" : "false");
    return ret;
}

uint8_t Accelerometer::read(uint8_t address)
{
    uint8_t recvData[1] = {0};
    uint8_t sendData[1] = {(LSM6DS3_REG) (LSM6DS3_REG_READ_FLAG | address)};

    m_spi.tx_rx(sendData, 1, recvData, 1, false);

    return recvData[0];
}

void Accelerometer::readBuffer(uint8_t address, void *recvData, size_t recvDataSize)
{
    uint8_t sendBuffer[1] = {0};
    uint8_t recvBuffer[recvDataSize] = {0};

    sendBuffer[0] = LSM6DS3_REG_READ_FLAG | address;
    memset(recvBuffer, 0, sizeof(recvBuffer));

    m_spi.tx_rx(sendBuffer, 1, recvBuffer, recvDataSize, false);

    memcpy(recvData, recvBuffer, recvDataSize);
}

void Accelerometer::write(uint8_t address, uint8_t data)
{
    //uint8_t recvData[2] = {0};
    uint8_t sendData[2] = {address & ~LSM6DS3_REG_READ_FLAG, data};

    if (!m_spi.tx_rx(sendData, 2, nullptr, 0, false)) {
        THROW(RegisterWriteEx);
    }
}

bool Accelerometer::setRegister(uint8_t address, uint8_t data)
{
    for (int i = 0; i < IMU_RETRIES; ++i)
    {
        write(address, data);

        if (read(address) == data)
        {
            return true;
        }
    }

    return false;
}

void Accelerometer::readAccRaw(Vector<int64_t> &v)
{
    int16_t Acc[3];

    readBuffer(LSM6DS3_REG_OUTX_XL, Acc, sizeof(Acc));

    v.m_x = Acc[0];
    v.m_y = Acc[1];
    v.m_z = Acc[2];
}

void Accelerometer::readGyroRaw(Vector<int64_t> &v)
{
    int16_t Gyr[3];

    readBuffer(LSM6DS3_REG_OUTX_G, Gyr, sizeof(Gyr));

    // Get Gyroscope raw data in IMU units
    v.m_x = Gyr[0];
    v.m_y = Gyr[1];
    v.m_z = Gyr[2];
}

/*
void Accelerometer::readGyroBias(Vector<long> &v)
{
    int16_t dataSize = 200;
    int16_t i;
    Vector<long> raw;
    Vector<long> sum;

    for (i=0; i < dataSize; i++)
    {
        readGyroRaw(raw);
        sum += raw;
    }

    v = sum / dataSize;
}

void Accelerometer::convertGyroToDPS(Vector<float> &result, Vector<long> &v)
{
    result.m_x = calcGyro(v.m_x);
    result.m_y = calcGyro(v.m_y);
    result.m_z = calcGyro(v.m_z);
}

*/

double Accelerometer::calcAccel(int64_t input)
{
    return (double) input * LSM6DS3_ACC_SENSITIVITY_FOR_FS_2G * (ACCEL_Range >> 1) * 0.001;
}

/*
float Accelerometer::calcAccelBack(long inputG)
{
    return (float) (inputG / (LSM6DS3_ACC_SENSITIVITY_FOR_FS_2G * (ACCEL_Range >> 1) * 0.001f));
}
*/

double Accelerometer::calcGyro(int64_t input)
{
    return (double) input * LSM6DS3_GYRO_SENSITIVITY_FOR_FS_245DPS * 0.001 / IMU_TIME_DIVIDER;
}

std::string Accelerometer::getDeviceName()
{
    return getName();
}

/*
float Accelerometer::calcGyroBack(long inputDps)
{
    return (float) (inputDps / (LSM6DS3_GYRO_SENSITIVITY_FOR_FS_245DPS * 0.001f / IMU_TIME_DIVIDER));
}
*/

bool Accelerometer::chipConnect()
{
    int retries;
    bool succeeded = false;

    //HAL_Delay(1000);

    // Wait until "Who Am I" result is correct
    for (retries = 0; retries < IMU_RETRIES; ++retries)
    {
        uint8_t whoAmI_response = read(LSM6DS3_REG_WHO_AM_I);
        succeeded = 0x69 == whoAmI_response;
        if (succeeded)
        {
            log::debug("Accelerometer <%s> is Ready!", m_devPath.c_str());
            break;
        }

        log::error("Accelerometer <%s> - FAIL (response 0x%02X)!!!", m_devPath.c_str(), whoAmI_response);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    //
    // Print registers
    //
    //	for (int reg = 1; reg <= 0x2D; ++reg)
    //	{
    //		uint8_t data = SPI_IMU_read((LSM6DS3_REG) reg);
    //		DebugPrint("Reg 0x%02X = 0x%02X", reg, data);
    //	}

    // Disable access to embedded functions configuration register
    //succeeded &= setRegister(LSM6DS3_REG_FUNC_CFG_ACCESS, 0);

    //
    // Configure LSM6DS3 Accelerometer
    //

    // Block Data Update
    // Register address automatically incremented
    //succeeded &= setRegister(LSM6DS3_REG_CTRL3_C, LSM6DS3_CTRL3_C_BDU | LSM6DS3_CTRL3_C_IF_INC);

    // Output Data Rate:    13Hz
    // Full Scale:          2G
    // AntiAlias filter:    400Hz
    //succeeded &= setRegister(LSM6DS3_REG_CTRL1_XL, LSM6DS3_ODR_13_HZ);

    //
    // Configure LSM6DS3 Gyroscope
    //

    // Output Data Rate:    833Hz
    // Full Scale:          245dps
    //succeeded &= setRegister(LSM6DS3_REG_CTRL2_G, LSM6DS3_ODR_833_HZ);

    // Roll axis (Y) is negative (TODO: Ask Igor about inconsistency - Z axis)
    //succeeded &= setRegister(LSM6DS3_REG_ORIENT_CFG_G,0x10);

    // Timestamp LSB: 25 uS
    //succeeded &= IMU_SetRegister(LSM6DS3_REG_WAKE_UP_DUR, LSM6DS3_WAKE_UP_DUR_TIMER_HR);

    // Timestamp Enable
    //succeeded &= IMU_SetRegister(LSM6DS3_REG_TAP_CFG, LSM6DS3_TAP_CFG_TIMER_EN);

    // Reset timestamp counter
    //IMU_ResetTimestamp();

    return succeeded;
}

void Accelerometer::checkParameters()
{
    ParameterPtr p = getParameter("HRHistorySec");
    m_params.m_buffTime = p ? (int)p->getValue() : 90;

    p = getParameter("TriggerOffset");
    m_params.m_triggerOffset = p ? (double)p->getValue() : 0.02;

    p = getParameter("UseCalibrationOffsetX");
    m_params.m_useCalibOffsetX = p ? (double)p->getValue() : true;

    p = getParameter("UseCalibrationOffsetY");
    m_params.m_useCalibOffsetY = p ? (double)p->getValue() : true;

    p = getParameter("UseCalibrationOffsetZ");
    m_params.m_useCalibOffsetZ = p ? (double)p->getValue() : true;

    m_hrParams.m_accX = m_hrData->getParameter("accX");
    m_hrParams.m_accY = m_hrData->getParameter("accY");
    m_hrParams.m_accZ = m_hrData->getParameter("accZ");
    m_hrParams.m_accRawX = m_hrData->getParameter("accRawX");
    m_hrParams.m_accRawY = m_hrData->getParameter("accRawY");
    m_hrParams.m_accRawZ = m_hrData->getParameter("accRawZ");

    m_hrParams.m_timestamp = m_hrData->getParameter("timestamp");
}
