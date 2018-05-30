#pragma once

#include "LogInterface.h"
#define LOG_MODULE(module, logger) INIT_LOGGER(module, log, logger.get())

extern LoggerPtr g_main_log;
extern LoggerPtr g_cout_log;
extern LoggerPtr g_gps_log;
extern LoggerPtr g_rtl_log;
extern LoggerPtr g_msp430_log;
extern LoggerPtr g_object_counter_log;
extern LoggerPtr g_sqlite3_log;

INIT_LOGGER(, glog, g_cout_log.get());
