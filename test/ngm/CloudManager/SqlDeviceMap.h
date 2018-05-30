#pragma once

#include <string>

struct SqlDeviceMap
{
    const char *device;
    const char *param;
    const char *table;
    const char *row;
};

extern SqlDeviceMap g_sqlDevMap[];
extern bool g_bSqlNeedReset;
extern std::string g_SqlDbFile;
