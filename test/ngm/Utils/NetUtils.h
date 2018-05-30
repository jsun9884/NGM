#pragma once

#include "Log.h"

namespace netutils {
    LOG_MODULE(netutils, g_main_log);

    bool isInterfaceUp(std::string ifname);
    bool getInterfaceRxConter(std::string ifname, int &rxCount);
    bool getInterfaceTxConter(std::string ifname, int &txCount);

} // namespace netutils
