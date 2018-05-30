#ifndef GPS_H
#define GPS_H

#include <thread>
#include <string>
#include <memory>
#include <atomic>
#include <list>
#include <mutex>
#include <time.h>
#include <iostream>

#include "Log.h"
#include "SensorState.h"
#include "Event.h"
#include "Device.h"
#include "HRData.h"
#include "serialport.h"

#define RECDATA_TIMEOUT 4000
#define RECDATA_BUFF_S 1000
#define RECDATA_PAUSE 5000
#define GSM_CONNECTION_WAIT_SEC 20

enum class GpsState: int {
    Starting,
    Up,
    Stopping,
    Down
};

struct GpsHRParameters {
    HRParameterPtr m_latitude;
    HRParameterPtr m_longitude;
    HRParameterPtr m_altitude;
    HRParameterPtr m_satelliteNumber;
    HRParameterPtr m_time;
    HRParameterPtr m_timestamp;
};

struct GpsReadings {
    double latitude = 0.0;
    double longitude = 0.0;
    uint32_t altitude = 0;
    uint32_t satelliteNumber = 0;
    uint64_t time = 0;

    void setLatitude(int deg, double min, char car) {
        int carMultip = (car == 'S') ? -1 : 1;
        latitude = ((double)deg + (min/60)) * carMultip;
    }
    void setLongitude(int deg, double min, char car) {
        int carMultip = (car == 'W') ? -1 : 1;
        longitude = ((double)deg + (min/60)) * carMultip;
    }
    void setAltitude(uint32_t alt) { altitude = alt; }
    void setSatellites(uint32_t sat) { satelliteNumber = sat; }
    void setTime(uint64_t time) { this->time = time; }

};

struct Date;

class Gps : public Device {
    LOG_MODULE(Gps, g_gps_log);

public:
    Gps(std::string name, std::string portPath, bool &gsmStatus, bool &gpsStatus, std::atomic<int> &gsmRSSI);
    virtual ~Gps() {}

    virtual void initialize() override;
    virtual bool connect() override;
    virtual bool readData(HRData::RecordData &data) override;
    virtual bool disconnect() override;
    virtual void reset() override {}

    virtual bool applyNativeParameter(ParameterPtr param) override;

    static void GpsLog(std::string toLog) { log::info() << toLog; }

    bool gpsIsUp();
    void startGps();
    void stopGps();

private:
    void Run();
    void Do();
    void checkHrParameters();
    int FindID();
    void Split(const std::string &input, char delimiter);
    bool CheckGPGGA(std::string &word) { return (word.substr(0,6) == "$GPGGA"); }
    bool CheckGPRMC(std::string &word) { return (word.substr(0,6) == "$GPRMC"); }
    void fixWord(std::string &word) const;
    void Sleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

    void ParseMessages();
    void ParseGPRMC(std::string sentence, tm &time);

    void ReadingsLogging();

    std::shared_ptr<std::thread> m_readThread;

    bool m_isUp;
    bool m_readingState;
    bool m_fileError;
    GpsState m_gpsState;

    std::chrono::system_clock::time_point m_start;

    SerialPort m_serialPort;
    std::string m_portPath;

    std::vector<std::string> m_buffer;
    std::vector<std::string> m_GPGGAsentences;
    std::vector<std::string> m_GPRMCsentences;
    GpsHRParameters m_hrParameters;
    GpsReadings m_readings;
    std::shared_ptr<Date> m_date;
    std::mutex m_mutex;
    bool &m_gsmStatus;
    bool &m_gpsStatus;
    std::atomic<int> &m_gsmRSSI;
};

typedef std::shared_ptr<Gps> GpsPtr;

struct Date {
    int m_day, m_mon, m_year;
    int m_hour, m_min, m_sec;
    bool isSet;

    Date() : m_day(0), m_mon(0), m_year(0), m_hour(0), m_min(0), m_sec(0), isSet(false) {}

    void set(int day, int mon, int year, int hour, int min, int sec) {
        m_day = day; m_mon = mon; m_year = year;
        m_hour = hour; m_min = min; m_sec = sec;
        isSet = true;
    }

    uint64_t unix_timestamp() {
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);     //or: rawtime = time(0);
        timeinfo = localtime(&rawtime);

        timeinfo->tm_year   = m_year;
        timeinfo->tm_mon    = m_mon;
        timeinfo->tm_mday   = m_day;
        timeinfo->tm_hour   = m_hour;
        timeinfo->tm_min    = m_min;
        timeinfo->tm_sec    = m_sec;

        Gps::GpsLog("");

        return mktime(timeinfo);        // call mktime: create unix time stamp from timeinfo struct
    }

};



#endif // GPS_H
