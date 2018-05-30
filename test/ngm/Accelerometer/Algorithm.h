#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <chrono>
#include <functional>
#include <mutex>
#include "HRData.h"
#include "Parameter.h"
#include "Log.h"

#include "lsm6ds3.h"
#include "Vector.h"
#include "Filter.h"

//#define ACC_DEBUG_OUT

//
// Private Definitions
//
#define ACC_FILTER_SIZE         10                  //4
#define ACC_FILTER_COEFFICIENT  (1.0f/ACC_FILTER_SIZE)
#define CHECK_STABILITY_SIZE    8       //3

#define FROM_mS_TO_S            0.001f
#define FROM_clockFunc_TO_S     0.01f

#define ALPHA_FILTER_COEF               0.9f
#define ACC_WEIGHTING                   0.05f
#define GYRO_WEIGHTING                  (1.0f - ACC_WEIGHTING)
#define GYRO_VELOCITY_THRESHOLD_DPS     0.008f
#define ACC_AVERAGE_DEVIATION           0.004f

#define RADIANS_TO_DEGREES      (180.0f/3.14159265359f)

#define PRINT_PERIOD	500		// [ms]

#define ALGORITHM_PERIOD	10		// [ms] Period between iterations of the algorithm

/** @addtogroup algorithm selector
 * @{
 */
typedef enum _ALGORITHM_TYPE
{
    COMPLEMENTARY_FILTER_ALGORITHM  = 0x00,    // Weighted average for ACC and GYRO. Angle =  ALPHA*AccAngle + (1-ALPHA)*GyroAngle
    STATIC_DYNAMIC_ALGORITHM        = 0x01,    // (ACC ~ const AND Gyro == 0) ? ACC : GYRO --> Angle
    ACCELEROMETER_ONLY_ALGORITHM    = 0x02,    // ACC --> Angle
    GYROSCOPE_ONLY_ALGORITHM        = 0x03,    // Not in Use
    KALMAN_FILTER_ALGORITHM         = 0x04,    // Not in Use
} ALGORITHM_TYPE;

typedef enum _ALGORITHM_SOURCE
{
    ALGORITHM_SOURCE_UNDEFINED = 0x0,    // Nothing to printOut
    ALGORITHM_SOURCE_ACC       = 0x1,    // Read the data from ACCELEROMETER
    ALGORITHM_SOURCE_GYRO      = 0x2,    // Read the data from GYROSCOPE
    ALGORITHM_SOURCE_ACC_GYRO  = 0x3,    // Read the data from ACCELEROMETER and GYROSCOPE
} ALGORITHM_SOURCE;

/**
  Interface
*/
class IAccelerometer {
public:
    virtual void readAccRaw(Vector<int64_t> &v) = 0;
    virtual void readGyroRaw(Vector<int64_t> &v) = 0;
    virtual double calcAccel(int64_t input) = 0;
    virtual double calcGyro(int64_t input) = 0;
    virtual std::string getDeviceName() = 0;
};

struct AccParams
{
    bool m_useCalibOffsetX;
    bool m_useCalibOffsetY;
    bool m_useCalibOffsetZ;
    double m_triggerOffset;
    int m_buffTime;
};

struct AccHrParams
{
    HRParameterPtr m_accX;
    HRParameterPtr m_accY;
    HRParameterPtr m_accZ;
    HRParameterPtr m_accRawX;
    HRParameterPtr m_accRawY;
    HRParameterPtr m_accRawZ;
    HRParameterPtr m_gyroX;
    HRParameterPtr m_gyroY;
    HRParameterPtr m_gyroZ;
    HRParameterPtr m_timestamp;
};

template<class T>
class AverageValue
{
private:
    T m_sum;
    size_t m_count;

public:
    AverageValue(const T &value):
        m_sum(value), m_count(0) {}

    void add(const T &value) {
        m_sum += value;
        ++m_count;
    }

    T average() const {
        return (T) (m_sum / m_count);
    }

    void reset(T value) {
        m_sum = value;
        m_count = 0;
    }
};

/**
 * @brief The Algorithm class
 */
class Algorithm
{
    LOG_MODULE(AccAlgo, g_main_log);
public:
    Algorithm(IAccelerometer *acc, bool bInvert);

    void reset(AccParams &p);

    void Task();

    void getData(HRData::RecordData &d, AccHrParams &p);

private:
    IAccelerometer *m_acc;
    bool m_bInvert;

    std::function<void()> Callback;

    std::mutex m_mutex;
    Vector<int64_t> Acc_raw;
    Vector<int64_t> Gyro_raw;
    Vector<double> Acc__;
    Vector<double> Gyro__;

    std::chrono::system_clock::time_point startAvTime;
    std::chrono::system_clock::time_point endAvTime;
#ifdef ACC_DEBUG_OUT
    std::chrono::system_clock::time_point nextLogTime;
#endif
    bool accAveragesDone;
    AverageValue<double> averageX;
    AverageValue<double> averageY;
    AverageValue<double> averageZ;
    double averagedValueX;
    double averagedValueY;
    double averagedValueZ;
};

#endif // ALGORITHM_H
