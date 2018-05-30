#pragma once

#include <thread>
#include <functional>
#include <mutex>

enum class CloudCommands: int {
	FW_UPGRADE      = 100,  //(firmware upgrade)
	TI_RESET        = 110,  //(clear registers)
	SOM_RESET       = 120,  //(clear buffers)
    COUNTERS_RESET  = 130,  // Reset average counters
	GET_HR_DATA     = 300,  //(Data commands start with 300)
    GET_ACCREG_DUMP = 301,  // Acc Registers dump
    ACC_CALIBRATE   = 302,  // Reset Acc Averages
	SET_CFG_FILE    = 310,  //(upload config file)
	GET_CFG_FILE    = 320
};

class CloudCmd {
public:
    const bool                              bLocal;
	const std::string 						commandId;
	const uint16_t							commandCode;
	const std::function<void(CloudCmd&)>	commandFunc;
	enum {
		INIT,
        ACCEPTED,
		REJECTED,
		DONE
	} 					state;
	std::string			message;
	std::mutex			mutex;

public:
    CloudCmd(bool isLocal, const std::string cmdId, const uint16_t cmdCode, std::function<void(CloudCmd&)> func):
        bLocal(isLocal), commandId(cmdId), commandCode(cmdCode), commandFunc(func), state(INIT) {}
};

typedef std::shared_ptr<CloudCmd> CloudCmdPtr;
