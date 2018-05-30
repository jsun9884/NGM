/*
 * CpuTempSensor.cpp
 *
 *  Created on: Jun 19, 2016
 *      Author: sergei
 */

#include "SysSensor.h"

#include <sstream>
#include <memory>

extern "C" {
#include "aws_iot_log.h"
}

SysSensor::SysSensor(std::string path):
m_bRunning(true),
m_path(path),
m_temperature(0.0),
m_thread(&SysSensor::sensorThread, this)
{
}

SysSensor::~SysSensor()
{
	m_bRunning = false;
	m_thread.join();
}

double SysSensor::value()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_temperature;
}

void SysSensor::sensorThread()
{
	m_file.open(m_path, std::ifstream::in);

    log::debug ("SysSensor \'%s\' started!", m_path.c_str());

	uint32_t data = 0;
    double temperature;

	while (m_bRunning) {
		m_file >> data;
		m_file.seekg(0, m_file.beg);

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_temperature = data / 1000.0;
            temperature = m_temperature;
		}
        log::info("CPU Temperature: %lf", temperature);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	m_file.close();

    log::debug ("SysSensor \'%s\' stopped!", m_path.c_str());
}
