/*
 * CpuTempSensor.h
 *
 *  Created on: Jun 19, 2016
 *      Author: sergei
 */

#ifndef SRC_SYSSENSOR_H_
#define SRC_SYSSENSOR_H_

#include <thread>
#include <mutex>
#include <string>
#include <fstream>

#include "Log.h"

class SysSensor
{
    LOG_MODULE(SysSensor, g_main_log);

public:
	SysSensor(std::string path);
	~SysSensor();

	double value();

private:
	void sensorThread();

private:
	std::mutex m_mutex;
	bool m_bRunning;
	std::string m_path;
	double m_temperature;
	std::ifstream m_file;
	std::thread m_thread;
};

#endif /* SRC_SYSSENSOR_H_ */
