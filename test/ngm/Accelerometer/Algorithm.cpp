#include "Algorithm.h"

Algorithm::Algorithm(IAccelerometer *acc, bool bInvert):
    m_acc(acc),
    m_bInvert(bInvert),
    accAveragesDone(false),
    averageX(0.0),
    averageY(0.0),
    averageZ(0.0)
{
}

void Algorithm::reset(AccParams &p)
{
    accAveragesDone = false;
    averageX.reset(0.0);
    averageY.reset(0.0);
    averageZ.reset(0.0);

#ifdef ACC_DEBUG_OUT
    nextLogTime = std::chrono::system_clock::now();
#endif

    startAvTime = std::chrono::system_clock::now() + std::chrono::seconds(5);
    endAvTime = startAvTime + std::chrono::seconds(p.m_buffTime);
}

void Algorithm::Task()
{
    /****************  ACCELEROMETER  ****************************/
    Vector<int64_t> accRaw;
    Vector<double> accVal;
    m_acc->readAccRaw(accRaw);

    accVal.m_x = m_acc->calcAccel(accRaw.m_x);
    accVal.m_y = m_acc->calcAccel(accRaw.m_y);
    accVal.m_z = m_acc->calcAccel(accRaw.m_z);

    /****************  Gyro  ****************************/
    //Vector<long> gyroRaw;
    //m_acc->readGyroRaw(gyroRaw);

    if (!accAveragesDone) {
        auto nowpoint = std::chrono::system_clock::now();
        if (nowpoint < startAvTime) {
            accVal.m_x = 0.0;
            accVal.m_y = 0.0;
            accVal.m_z = 0.0;
        }
        else if (nowpoint >= endAvTime) {
            accAveragesDone = true;
            averagedValueX = averageX.average();
            averagedValueY = averageY.average();
            averagedValueZ = averageZ.average();

            accVal.m_x -= averagedValueX;
            accVal.m_y -= averagedValueY;
            accVal.m_z -= averagedValueZ;
        }
        else {
            averageX.add(accVal.m_x);
            averageY.add(accVal.m_y);
            averageZ.add(accVal.m_z);

            accVal.m_x = 0.0;
            accVal.m_y = 0.0;
            accVal.m_z = 0.0;
        }
    }
    else {
        accVal.m_x -= averagedValueX;
        accVal.m_y -= averagedValueY;
        accVal.m_z -= averagedValueZ;
    }

    {
        std::lock_guard<std::mutex> l(m_mutex);
        Acc_raw = accRaw;
        Acc__ = accVal;


        //Gyro_raw = gyroRaw;

        //Gyro__.m_x = m_acc->calcGyro(Gyro_raw.m_x);
        //Gyro__.m_y = m_acc->calcGyro(Gyro_raw.m_y);
        //Gyro__.m_z = m_acc->calcGyro(Gyro_raw.m_z);
    }

#ifdef ACC_DEBUG_OUT
    {
        auto nowpoint = std::chrono::system_clock::now();
        if (nowpoint > nextLogTime) {
            log::info()  << "[" << m_acc->getDeviceName() << "] "<< "Acc last: " << accVal.m_x << ", " <<
                           accVal.m_y << ", " << accVal.m_z;
            nextLogTime = nowpoint + std::chrono::seconds(1);
        }
    }
#endif
}

void Algorithm::getData(HRData::RecordData &d, AccHrParams &p)
{
    Vector<double> acc;
    Vector<int64_t> accRaw;
    Vector<double> gyro;
    {
        std::lock_guard<std::mutex> l(m_mutex);
        acc = Acc__;
        accRaw = Acc_raw;
        //gyro = Gyro__;
    }

    d.set(p.m_accX, (m_bInvert ? -acc.x() : acc.x()));
    d.set(p.m_accY, acc.y());
    d.set(p.m_accZ, (m_bInvert ? -acc.z() : acc.z()));

    d.set(p.m_accRawX, accRaw.m_x);
    d.set(p.m_accRawY, accRaw.m_y);
    d.set(p.m_accRawZ, accRaw.m_z);

    //d.set(p.m_gyroX, (m_bInvert ? -gyro.x(): gyro.x()));
    //d.set(p.m_gyroY, gyro.y());
    //d.set(p.m_gyroZ, (m_bInvert ? -gyro.z(): gyro.z()));

    d.set(p.m_timestamp, (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}
