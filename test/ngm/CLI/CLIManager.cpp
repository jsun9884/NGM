#include "CLIManager.h"
#include "CloudManager.h"

void CLIManager::Init(LedsPtr leds, CloudManager *cloud) {
	CLICommandPtr ptr;

	m_currentCommand = "";
    
    ptr = static_cast<CLICommandPtr>(new CLICommandDeviceList("List", "", m_hardwareManager));
    AddCommand(ptr, "DeviceList", "Device List", "Display list of devices in the system");

	ptr = static_cast<CLICommandPtr>(new CLICommandGetParams("GetParams", "", m_hardwareManager));
    AddCommand(ptr, "DeviceGetParams", "Device <device_name> GetParams", "Display list of parameters for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandSetParam("SetParam", "", m_hardwareManager));
    AddCommand(ptr, "DeviceSetParam", "Device <device_name> SetParam <param_name> <value>", "Set value to parameter for a given device");

    // HR:

    ptr = static_cast<CLICommandPtr>(new CLICommandGetHRParams("GetHRParams", "", m_hardwareManager));
    AddCommand(ptr, "DeviceGetHRParams", "Device <device_name> GetHRParams", "Display list of high resolution parameters for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandGetHRParam("GetHRParam", "", m_hardwareManager));
    AddCommand(ptr, "DeviceGetHRParam", "Device <device_name> GetHRParam <param_name>", "Display a single high resolution parameter for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandGetData("GetHRData", "", m_hardwareManager));
    AddCommand(ptr, "DeviceGetHRData", "Device <device_name> GetHRData", "Display high resolution data for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandSetHRParam("SetHRParam", "", m_hardwareManager));
    AddCommand(ptr, "DeviceSetHRParam", "Device <device_name> SetHRParam <param_name> <lowThres_value> <highThres_value> <isStatistics_ value(opt.)>", "Set value to high resolution parameter for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandSetLowThreshold("SetLowThreshold", "", m_hardwareManager));
    AddCommand(ptr, "DeviceSetLowThreshold", "Device <device_name> SetLowThreshold <param_name> <value>", "Set value to low threshold of high resolution parameter for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandSetHighThreshold("SetHighThreshold", "", m_hardwareManager));
    AddCommand(ptr, "DeviceSetHighThreshold", "Device <device_name> SetHighThreshold <param_name> <value>", "Set value to high threshold of high resolution parameter for a given device");

    // registers:

    ptr = static_cast<CLICommandPtr>(new CLICommandGetRegister("GetRegister", "", m_hardwareManager));
    AddCommand(ptr, "DeviceGetRegister", "Device <device_name> GetRegister <address>", "Display register by address for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandGetRegistersDump("GetRegDump", "", m_hardwareManager));
    AddCommand(ptr, "DeviceGetRegDump", "Device <device_name> GetRegDump <address> <count>", "Display registers dump from address for a given device");

    ptr = static_cast<CLICommandPtr>(new CLICommandSetRegister("SetRegister", "", m_hardwareManager));
    AddCommand(ptr, "DeviceSetRegister", "Device <device_name> SetRegister <address> <value>", "Set value to register by address for a given device");

    // Wifi:

    ptr = static_cast<CLICommandPtr>(new CLICommandWifiInit("Init", ""));
    AddCommand(ptr, "WifiInit", "Wifi Init <SSID> <password>", "Initialize Wifi parameters");

    ptr = static_cast<CLICommandPtr>(new CLICommandWifiStatus("Status", ""));
    AddCommand(ptr, "WifiStatus", "Wifi Status", "Display Wifi status");

    ptr = static_cast<CLICommandPtr>(new CLICommandWifiOn("On", ""));
    AddCommand(ptr, "WifiOn", "Wifi On", "Swith on WiFi");

    ptr = static_cast<CLICommandPtr>(new CLICommandWifiOff("Off", ""));
    AddCommand(ptr, "WifiOff", "Wifi Off", "Swith off WiFi");

    // Leds:

    ptr = static_cast<CLICommandPtr>(new CLICommandLedsOn("On", "", leds));
    AddCommand(ptr, "LedsOn", "Leds On <LedNum> <reg> <green> <blue>", "Switch on led number <LedNum>");

    ptr = static_cast<CLICommandPtr>(new CLICommandLedsOff("Off", "", leds));
    AddCommand(ptr, "LedsOff", "Leds Off <LedNum>", "Switch off led number <LedNum>");

    // Gsm:

    ptr = static_cast<CLICommandPtr>(new CLICommandGsmCreate("Create", ""));
    AddCommand(ptr, "GsmCreate", "Gsm Create <Apn>", "Create Gsm connection with APN <Apn>");

    ptr = static_cast<CLICommandPtr>(new CLICommandGsmSet("Set", ""));
    AddCommand(ptr, "GsmSet", "Gsm Set <Parameter> <Value>", "Set GSM <Parameter> (Apn, Username, Password or Numer) to <Value>");

    ptr = static_cast<CLICommandPtr>(new CLICommandGsmStatus("Status", "", cloud));
    AddCommand(ptr, "GsmStatus", "Gsm Status", "Display Gsm Status");

    ptr = static_cast<CLICommandPtr>(new CLICommandGsmOn("On", ""));
    AddCommand(ptr, "GsmOn", "Gsm On", "Switch on GSM");

    ptr = static_cast<CLICommandPtr>(new CLICommandGsmOff("Off", ""));
    AddCommand(ptr, "GsmOff", "Gsm Off", "Switch off GSM");


    // Special:

    AddCommand(nullptr, "CommBridge", "CommBridge", "Stop main application and connect directly to TI via comm bridge");
    AddCommand(nullptr, "Reboot", "Reboot", "Reboot the system");
    AddCommand(nullptr, "H", "H", "Display command history");
    AddCommand(nullptr, "H", "H <command_index>", "Execute command from history by command_index");
    AddCommand(nullptr, "Help", "Help", "Display help manual");

	m_isUP = true;
}

void CLIManager::AddCommand(CLICommandPtr ptr, std::string command, std::string name, std::string description) {
    if (ptr != nullptr) {
        m_CLICommands.insert(std::pair<std::string, CLICommandPtr>(command, ptr));
    }
    m_vCLICommands.push_back(std::pair<std::string,std::string>(name,description));
}

int CLIManager::Run() {
    int returnValue = 0;

    while (m_isUP) {
        if (returnValue != 10) ReceiveCommand();
        returnValue = Execute();
    }
    return returnValue;
}

void CLIManager::ReceiveCommand() {
    std::cout << "NGM> ";
	m_currentCommand.clear();
	std::getline(std::cin, m_currentCommand);

}

int CLIManager::Execute() {
    int returnValue = 0;

    SplitCommand(' ');
    if (m_commandVector.empty()) { return 0; }

    try {
        if (m_commandVector[0] == "Device") {
            DeviceCommand();
        }
        else if (m_commandVector[0] == "Wifi") {
            CommonCommand();
        }
        else if (m_commandVector[0] == "Leds") {
            CommonCommand();
        }
        else if (m_commandVector[0] == "Gsm") {
            CommonCommand();
        }
        else {
            SpecialCommand(returnValue);
        }
    }
    catch(std::exception &ex) {
        std::cout << ex.what() << "\n";
    }

    m_commandVector.clear();
    return returnValue;
}

void CLIManager::DeviceCommand() {
    if (m_commandVector.size() == 1) {
        THROW(DeviceMissingEx);
    }
    else if (m_commandVector[1] == "List") {
        std::vector<std::string> dummyVector;
        auto command_it = m_CLICommands.find("DeviceList");
        command_it->second->Execute(dummyVector);
        saveCommand();
        return;
    }
    else if (m_commandVector.size() < 3) {
        THROW(NoParametersEx);
    }

    auto command_it = m_CLICommands.find(m_commandVector[0] + m_commandVector[2]);
    if (command_it == m_CLICommands.end() || m_commandVector[2] == "List") {
        THROW(CommandNotFoundEx);
    }

    std::vector<std::string> deviceCommandVector;
    for (int i=1; i<m_commandVector.size(); i++) {
        deviceCommandVector.push_back(m_commandVector[i]);
    }

    command_it->second->Execute(deviceCommandVector);
    saveCommand();
}

void CLIManager::CommonCommand() {
    if (m_commandVector.size() == 1) {
        THROW(NoParametersEx);
    }

    auto command_it = m_CLICommands.find(m_commandVector[0] + m_commandVector[1]);
    if (command_it == m_CLICommands.end()) {
        THROW(CommandNotFoundEx);
    }

    std::vector<std::string> commandVector;
    for (int i = 1; i < m_commandVector.size(); i++) {
        commandVector.push_back(m_commandVector[i]);
    }

    command_it->second->Execute(commandVector);
    saveCommand();
}

void CLIManager::SpecialCommand(int& returnValue) {
    if (m_commandVector[0] == "Reboot") {
        m_isUP = false;
        saveCommand();
    }
    else if (m_commandVector[0] == "CommBridge") {
        m_isUP = false;
        returnValue = 1;
        saveCommand();
    }
    else if (m_commandVector[0] == "LinuxShell") {
        m_isUP = false;
        returnValue = 2;
        saveCommand();
    }
    else if (m_commandVector[0] == "H") {
        HistoryCommand(returnValue);
    }
    else if (m_commandVector[0] == "Help") {
        HelpCommand();
        saveCommand();
    }
    else {
        THROW(CommandNotFoundEx);
    }
}

void CLIManager::HistoryCommand(int& returnValue) {
    if (m_commandVector.size() == 1) {
        for (int i = 0; i < m_commandHistory.size() && i < 50; i++) {
            std::cout << i+1 << ": " << m_commandHistory[i] << "\n";
        }
    }
    else {
        try {
            int commandIndex = std::stoi(m_commandVector[1]) - 1;
            if (commandIndex < m_commandHistory.size() && commandIndex < 50) {
                std::cout << m_commandHistory[commandIndex] << "\n";
                m_currentCommand = m_commandHistory[commandIndex];
                returnValue = 10;
            }
        }
        catch(std::exception& ex) {
            std::cout << "command index must be numeric value\n";
        }
    }
}

void CLIManager::HelpCommand() {
    std::cout << "Welcome to CLI help and command catalog >>>\n\n";
    std::cout << "Type of commands:\n";
    std::cout << "\t1. Device commands\n";
    std::cout << "\t2. Special commands\n\n";
    std::cout << "1. Device commands structure:\n";
    std::cout << "\tDevice <device_name> <command_name> <arg1(optional)> <arg2(optional)> ...\n";
    std::cout << "\t* Special device command: Device List\n\n";
    std::cout << "2. Special commands are one-word commands.\n\n";
    std::cout << "Command catalog:\n";
    std::cout << "Command     /     Description\n";
    std::cout << "******************************\n";
    for (int i=0; i<m_vCLICommands.size(); i++) {
        std::cout << m_vCLICommands[i].first
                  << "\n\t* "
                  << m_vCLICommands[i].second << "\n\n";
    }
}

std::string CLIManager::space(std::string first) const {
    std::string sp;
    int spaceChars = 20-first.size();

    for (int i = 0; i < spaceChars; i++) { sp += " "; }
    return sp;
}

void CLIManager::SplitCommand(char delimiter) {
    std::string word;
	std::string::iterator sit;

	for (sit = m_currentCommand.begin(); sit != m_currentCommand.end(); sit++) {
        while (sit != m_currentCommand.end() && *sit != delimiter && *sit != '\n') {
            word += *sit;
			sit++;
		}
        m_commandVector.push_back(word);
        if (sit == m_currentCommand.end()) break;
        word.clear();
	}
}

void CLIManager::saveCommand() {
    m_commandHistory.insert(m_commandHistory.begin(),m_currentCommand);
}
