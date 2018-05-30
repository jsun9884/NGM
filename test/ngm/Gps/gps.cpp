

extern "C" {
#include "aws_iot_log.h"
}

#include <chrono>
#include <iostream>
#include <nmea.h>
#include "gpgll.h"
#include "gpgga.h"
#include "gps.h"
#include "dlt645msg.h"
#include "string.h"
#include "Event.h"
#include "NetUtils.h"

bool Gps::connect() {
    log::info() << "CONNECT!";

    if (!m_gsmStatus) {
        auto time_elapsed = std::chrono::system_clock::now() - m_start;
        if (time_elapsed < std::chrono::seconds(GSM_CONNECTION_WAIT_SEC)) {
            return false;
        }
    }

    checkHrParameters();

    if (!m_serialPort.open(m_portPath, 9600, false, false)) {
        log::error() << "Failed to open serial port: " << m_portPath;
        m_gpsStatus = false;
        return false;
    }

    log::info() << "Serial port " << m_portPath << " opened successfully";
    Run();

    //m_gpsStatus = true;
    return true;
}

void Gps::Run() {
    log::info() << "RUN!";
    m_fileError = false;
    m_isUp = true;
    m_readThread = std::make_shared<std::thread>(&Gps::Do, this);
}

void Gps::Do() {
    log::info() << "Starting Gps Loop...";

    std::string startCommand = "AT+XLCSLSR=2,3,,,,,3,,,2,511\r\n";
    std::string stopCommandCore = "AT+XLSRSTOP=0,";                 // <-- <request ID> should be added from response
    std::string stopCommand;
    std::string rssiCommand = "AT+CSQ\r\n";
    std::string enterKey = "\r\n";

    int ret;
    uint8_t buff[RECDATA_BUFF_S];

    m_date = std::make_shared<Date>();

    //m_serialPort.write((const uint8_t*)startCommand.c_str(),startCommand.size());
    //m_serialPort.write((const uint8_t*)enterKey.c_str(),enterKey.size());

    Sleep(RECDATA_PAUSE);

    while (m_isUp) {
        if (m_gpsState == GpsState::Starting) {
            log::info() << "Starting GPS Stream...";
            m_serialPort.write((const uint8_t*)startCommand.c_str(),startCommand.size());

            ret = m_serialPort.read(buff, RECDATA_BUFF_S, RECDATA_TIMEOUT);
            if (ret > 0) {
                int id;
                std::string response((char*)buff);
                Split(response,' ');
                id = FindID();
                if (id != -1) {
                    std::ofstream idfile;
                    idfile.open("._gps_id", std::ofstream::trunc);
                    if (idfile.is_open()) {
                        idfile << id;
                        log::info() << "GPS ID saved to <._gps_id>";
                    }
                    else {
                        log::error() << "Can't open <._gps_id> file for write";
                    }
                    idfile.close();
                }
                else {
                    std::ifstream idfile;
                    idfile.open("._gps_id");
                    if (idfile.is_open()) {
                        idfile >> id;
                        log::info() << "GPS ID readed from <._gps_id>";
                    }
                    else {
                        log::error() << "Can't open <._gps_id> file for read";
                    }
                }
                log::info() << "Request ID: " << id;
                stopCommand = stopCommandCore + std::to_string(id) + enterKey;
                m_gpsState = GpsState::Up;
            }
        }

        if (m_gpsState == GpsState::Up) {
            ret = m_serialPort.read(buff, RECDATA_BUFF_S, RECDATA_TIMEOUT);
            if (ret > 0) {
                std::string response((char*)buff);
                Split(response,' ');
                //log::info() << "GPS: " << response;
                ParseMessages();
            }
            else {
                m_gpsState = GpsState::Starting;
            }
        }

        if (m_gpsState == GpsState::Stopping) {
            log::info() << "Stopping GPS Stream...";
            m_serialPort.write((const uint8_t*)stopCommand.c_str(), stopCommand.size());
            m_serialPort.read(buff, RECDATA_BUFF_S, 100);
            m_gpsState = GpsState::Down;
        }

        //log::info() << ">> " << rssiCommand;
        m_serialPort.write((const uint8_t*)rssiCommand.c_str(), rssiCommand.size());
        ret = m_serialPort.read(buff, RECDATA_BUFF_S, 100);
        if (ret > 0) {
            std::string response((char*)buff);
            //log::info() << "RSSI: " << response;
            std::istringstream iss(response);
            int level;
            char ch;
            for (size_t i = 0, sz = response.size(); i < sz; ++i) {
                iss >> ch;
                if (ch == ':') {
                    break;
                }
            }
            iss >> level;
            level = -113 + 2 * level;
            m_gsmRSSI.store(level);
            //log::info() << "GSM RSSI Level: " << (int)(-113 + 2 * level) << " dBm";
        }
        else {
            m_fileError = true;
            log::error() << "UART IO error...";
        }
    }

    m_serialPort.read(buff, RECDATA_BUFF_S, 100);
    m_serialPort.write((const uint8_t*)stopCommand.c_str(), stopCommand.size());
    //m_serialPort.write((const uint8_t*)enterKey.c_str(),enterKey.size());
    m_serialPort.read(buff, RECDATA_BUFF_S, 100);

    log::info() << "Gps loop stopped";

}

void Gps::ParseMessages() {
    nmea_gpgga_s *gpgga = NULL;
    nmea_s *data = NULL;

    for (int i = 0; i < m_GPGGAsentences.size(); i++) {
        //log::info() << "parsing: " << m_GPGGAsentences[i];
        data = nmea_parse((char*)m_GPGGAsentences[i].c_str(), m_GPGGAsentences[i].size(), 0);

        if (data == NULL) {
            m_readingState = false;
        }
        else {
            m_readingState = true;
            gpgga = (nmea_gpgga_s *)data;

            try {
                {
                    std::lock_guard<std::mutex> l(m_mutex);

                    if (i < m_GPRMCsentences.size() && gpgga->n_satellites > 0) {
                        ParseGPRMC(m_GPRMCsentences[i], gpgga->time);
                    }

                    m_readings.setLatitude(gpgga->latitude.degrees, gpgga->latitude.minutes, (char)gpgga->latitude.cardinal);
                    m_readings.setLongitude(gpgga->longitude.degrees, gpgga->longitude.minutes, (char)gpgga->longitude.cardinal);
                    m_readings.setAltitude(gpgga->altitude);
                    m_readings.setSatellites(gpgga->n_satellites);
                    if (m_readings.satelliteNumber > 2) {
                        m_gpsStatus = true;
                    }
                    if (m_date->isSet) {
                        m_readings.setTime(m_date->unix_timestamp());
                    }
                }

                //ReadingsLogging();
            }
            catch(std::exception& ex) {
                log::error() << "Parsing exception: " << ex.what();
            }
        }
        nmea_free(data);
    }
    m_GPGGAsentences.clear();
    m_GPRMCsentences.clear();

}

void Gps::ReadingsLogging() {
    log::info() << "Latitude: "  << m_readings.latitude;
    log::info() << "Longitude: " << m_readings.longitude;
    log::info() << "Altitude: "  << m_readings.altitude;
    log::info() << "# satellites: " << m_readings.satelliteNumber << "\n";

    log::info() << "GPS Time: "  << m_date->m_hour << ":" << m_date->m_min << ":" << m_date->m_sec;
    log::info() << "GPS Date: "  << m_date->m_day << "/" << m_date->m_mon << "/" << m_date->m_year;
    log::info() << "Timestamp: "  << m_readings.time;
}

void Gps::ParseGPRMC(std::string sentence, struct tm& time) {
    int i, commaCount = 9;
    for (i = 0; i < sentence.size() && commaCount > 0; i++) {
        if (sentence[i] == ',') commaCount--;
    }
    if (commaCount > 0) { return; }

    std::string dateStr = sentence.substr(i,6);
    m_date->set(std::stoi(dateStr.substr(0,2)),
               std::stoi(dateStr.substr(2,2)),
               std::stoi(dateStr.substr(4,2)),
               time.tm_hour,
               time.tm_min,
               time.tm_sec);
}

int Gps::FindID() {
    for (int i = 0; i < m_buffer.size(); i++) {
        if (m_buffer[i] == "id") {
            return std::stoi(m_buffer[i+1]);
        }
    }
    return -1;
}

void Gps::Split(const std::string& input, char delimiter) {
    std::string word;
    std::string::const_iterator sit;

    m_buffer.clear();

    for (sit = input.begin(); sit != input.end(); sit++) {
        while (sit != input.end() && *sit != delimiter && *sit != '\n') {
            word += *sit;
            sit++;
        }
        m_buffer.push_back(word);
        if (word[0] == '$') {
            fixWord(word);
            if (CheckGPRMC(word)) {
                m_GPRMCsentences.push_back(word);
            }
            else if (CheckGPGGA(word)) {
                m_GPGGAsentences.push_back(word);
            }
        }
        if (sit == input.end()) break;
        word.clear();
    }
}

void Gps::fixWord(std::string& word) const {
    word = word.substr(0,word.size()-1);
    word[2] = 'P';
    word += "\n\n";
}

bool Gps::disconnect() {
    log::info() << "DISCONNECT!";
    m_isUp = false;
    m_readThread->join();

    /*{
        uint8_t buff[RECDATA_BUFF_S];
        std::string atcmd = "AT#ENHRST=1,0\r\n";
        log::info() << ">> " << atcmd;
        m_serialPort.write((const uint8_t*)atcmd.c_str(),atcmd.size());
        int ret = m_serialPort.read(buff, RECDATA_BUFF_S, 300);
        log::info() << ">> " << std::string((const char *)buff, ret);
    }*/

    m_serialPort.close();
    //::system("sleep 1 && nmcli c up \"GSMInternet\" > /dev/null 2>> logs/gsm.errors");

    m_start = std::chrono::system_clock::now();
    m_gpsStatus = false;
    return true;
}

bool Gps::applyNativeParameter(ParameterPtr param) {
    return false;
}

bool Gps::gpsIsUp()
{
    return (m_gpsState == GpsState::Up) || (m_gpsState == GpsState::Starting);
}

void Gps::startGps()
{
    switch(m_gpsState)
    {
    case GpsState::Down:
    case GpsState::Stopping:
        m_gpsState = GpsState::Starting;
        break;
    }
}

void Gps::stopGps()
{
    switch(m_gpsState)
    {
    case GpsState::Up:
    case GpsState::Starting:
        m_gpsState = GpsState::Stopping;
        break;
    }
}

bool Gps::readData(HRData::RecordData &data) {
    using namespace std::chrono;
    static thread_local system_clock::time_point lastGsmCheck = std::chrono::system_clock::now();

    data.set(m_hrParameters.m_timestamp, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    if (!m_isUp) {
        return true;
    }

    if (m_readingState) {
        std::lock_guard<std::mutex> l(m_mutex);
        data.set(m_hrParameters.m_latitude, m_readings.latitude);
        data.set(m_hrParameters.m_longitude, m_readings.longitude);
        data.set(m_hrParameters.m_altitude, m_readings.altitude);
        data.set(m_hrParameters.m_satelliteNumber, m_readings.satelliteNumber);
        data.set(m_hrParameters.m_time, m_readings.time);
    }

    system_clock::time_point currentTime = std::chrono::system_clock::now();
    if ((currentTime - lastGsmCheck) > std::chrono::seconds(GSM_CONNECTION_WAIT_SEC)) {
        lastGsmCheck = currentTime;
        if (!m_gsmStatus) {
            m_start = currentTime;
            return true; //false;
        }
    }
    return !m_fileError;
}

void Gps::checkHrParameters() {
    m_hrParameters.m_latitude = m_hrData->getParameter("latitude");
    m_hrParameters.m_longitude = m_hrData->getParameter("longitude");
    m_hrParameters.m_altitude = m_hrData->getParameter("altitude");
    m_hrParameters.m_satelliteNumber = m_hrData->getParameter("satelliteNumber");
    m_hrParameters.m_time = m_hrData->getParameter("time");
    m_hrParameters.m_timestamp = m_hrData->getParameter("timestamp");
}

Gps::Gps(std::string name, std::string portPath, bool &gsmStatus, bool &gpsStatus, std::atomic<int> &gsmRSSI):
    Device(name), m_isUp(false), m_readingState(true), m_fileError(false), m_gpsState(GpsState::Down), m_portPath(portPath),
    m_start(std::chrono::system_clock::now()), m_gsmStatus(gsmStatus), m_gpsStatus(gpsStatus), m_gsmRSSI(gsmRSSI)
{}

void Gps::initialize()
{
    ::system("mv -f logs/gsm.errors logs/gsm.errors.old");
}
