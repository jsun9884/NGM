#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "CLICommand.h"
#include "Exceptions.h"

#pragma once

typedef std::unordered_map<std::string, CLICommandPtr> HtCliCommand;
typedef std::vector<std::pair<std::string,std::string>> vecCliCommand;

class CLIManager {		// singleton class
private:
	CLIManager(HardwareManager* hwMgr) { m_hardwareManager = hwMgr; }
	CLIManager(const HardwareManager &);
	const CLIManager& operator=(const HardwareManager&);
	~CLIManager();
	//----------------------------------------
	
	bool m_isUP;
	HardwareManager* m_hardwareManager;
	HtCliCommand m_CLICommands;
    vecCliCommand m_vCLICommands;
	std::string m_currentCommand;
	std::vector<std::string> m_commandVector;
    std::vector<std::string> m_commandHistory;

	void SplitCommand(char delimiter);
    void AddCommand(CLICommandPtr ptr, std::string command, std::string name, std::string description);

    void DeviceCommand();
    void CommonCommand();
    void SpecialCommand(int &returnValue);

    void HistoryCommand(int &returnValue);
    void HelpCommand();

    void saveCommand();
    std::string space(std::string first) const;

public:
	static CLIManager* Instance(HardwareManager* hwMgr) {
		static CLIManager* singleHWM = new CLIManager(hwMgr);
		return singleHWM;
	}
    void Init(LedsPtr leds, CloudManager *cloud);
    int Run();
	void ReceiveCommand();
    int Execute();
    bool isUP() { return m_isUP; }

};

