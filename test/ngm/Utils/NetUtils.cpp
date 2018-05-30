#include <fstream>
#include <string>
#include <sstream>

#include "Log.h"
#include "NetUtils.h"


bool netutils::isInterfaceUp(std::string ifname)
{
    std::ifstream file;
    std::string path = "/sys/class/net/" + ifname + "/operstate";
    std::string res;
    bool bRes = false;

    file.open(path.c_str());
    if (file.is_open()) {
        file >> res;
        //glog::info() << ifname << " status: " << res;
        if (res != "down") {
            bRes = true;
        }
    }
    return bRes;
}

bool netutils::getInterfaceRxConter(std::string ifname, int &rxCount)
{
    std::ifstream file;
    std::string path = "/sys/class/net/" + ifname + "/statistics/rx_bytes";

    file.open(path.c_str());
    if (file.is_open()) {
        std::string sval;
        file >> sval;
        std::stringstream ss(sval);
        ss >> rxCount;
        //glog::info() << ifname << " rx_bytes: " << rxCount;
        return true;
    }
    return false;
}

bool netutils::getInterfaceTxConter(std::string ifname, int &txCount)
{
    std::ifstream file;
    std::string path = "/sys/class/net/" + ifname + "/statistics/tx_bytes";

    file.open(path.c_str());
    if (file.is_open()) {
        std::string sval;
        file >> sval;
        std::stringstream ss(sval);
        ss >> txCount;
        //glog::info() << ifname << " tx_bytes: " << txCount;
        return true;
    }
    return false;
}
