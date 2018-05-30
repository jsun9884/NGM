#include "HardwareManager.h"
#include "Exceptions.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "Leds.h"

#pragma once

class CLICommand;
typedef std::shared_ptr<CLICommand> CLICommandPtr;

class CLICommand {		// abstract class
protected:
	std::string m_name;
	std::string m_description;

public:
	CLICommand(std::string name, std::string description) : m_name(name), m_description(description) {}
    virtual bool Execute(std::vector<std::string>& args) = 0;

protected:
    std::string GetName() { return m_name; }
    std::string GetDescription() { return m_description; }
    void CheckParamNumber(std::vector<std::string>& args, int minParams);
};

///////////////////////////////////////////////////////////////////////

class CLIDeviceCommand : public CLICommand {		// abstract class
protected:
	HardwareManager* m_hardwareManager;
	std::string m_deviceName;
    ISupportConfigurationPtr m_device;

public:
	CLIDeviceCommand(std::string name, std::string description, HardwareManager* hwManager) :
		CLICommand(name, description), m_hardwareManager(hwManager), m_deviceName("") {}
	
protected:
    void handleDevice(std::vector<std::string>& args) {
        ExtractDeviceName(args);
        SetDevice();
    }

private:
	void ExtractDeviceName(std::vector<std::string>& args) {
		if (args.empty()) {
            THROW(DeviceMissingEx);
		}
		m_deviceName = args[0];
	}
    void SetDevice() { m_device = m_hardwareManager->getDevice(m_deviceName); }

};

//////////////////////

class CLILedsCommand : public CLICommand {
protected:
    LedsPtr m_leds;

public:
    CLILedsCommand(std::string name, std::string description, LedsPtr leds) :
        CLICommand(name, description), m_leds(leds) {}
};

class CLICommandLedsOn : public CLILedsCommand {
public:
    CLICommandLedsOn(std::string name, std::string description, LedsPtr leds) :
        CLILedsCommand(name, description, leds) {}

    virtual bool Execute(std::vector<std::string>& args);
};

class CLICommandLedsOff : public CLILedsCommand {
public:
    CLICommandLedsOff(std::string name, std::string description, LedsPtr leds) :
        CLILedsCommand(name, description, leds) {}

    virtual bool Execute(std::vector<std::string>& args);
};

/////////////////////////^^^ ABSTRACT ^^^//////////////////////////////
///////////////////////////////////////////////////////////////////////

class CLICommandDeviceList : public CLIDeviceCommand {
public:
    CLICommandDeviceList(std::string name, std::string description, HardwareManager* hwManager) : 
		CLIDeviceCommand(name, description, hwManager) {}
    
    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandGetParams : public CLIDeviceCommand {
protected:
    void PrintParams(std::vector<ParameterPtr>& vParams);
    //void PrintSubParams(ParameterPtr params, int padding);
    //void PrintPadding(int padding);
	
public:
	CLICommandGetParams(std::string name, std::string description, HardwareManager* hwManager) :
		CLIDeviceCommand(name, description, hwManager) {}

	virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandSetParam : public CLIDeviceCommand {
public:
    CLICommandSetParam(std::string name, std::string description, HardwareManager* hwManager) :
		CLIDeviceCommand(name, description, hwManager) {}

	virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandGetHRParams : public CLIDeviceCommand {
protected:
    void PrintParams(std::vector<HRParameterPtr> &vParams);

public:
    CLICommandGetHRParams(std::string name, std::string description, HardwareManager* hwManager) :
        CLIDeviceCommand(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandGetHRParam : public CLIDeviceCommand {
protected:
    void PrintParam(HRParameterPtr Param);
    std::string ExtractParamName(std::vector<std::string> &args);

public:
    CLICommandGetHRParam(std::string name, std::string description, HardwareManager* hwManager) :
        CLIDeviceCommand(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandGetData : public CLIDeviceCommand {
protected:


public:
    CLICommandGetData(std::string name, std::string description, HardwareManager* hwManager) :
        CLIDeviceCommand(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandSetHRParam : public CLIDeviceCommand {
protected:
    typedef enum {
        Low,
        High
    } ThresholdType;

    ValueType m_paramValueType;
    HRParameterPtr m_paramPtr;

    ValueType findParam(std::string paramName);
    void setThreshold(ThresholdType thresType, std::string value);
    void setStatisticsEnabled(std::string value);

public:
    CLICommandSetHRParam(std::string name, std::string description, HardwareManager* hwManager) :
        CLIDeviceCommand(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandSetLowThreshold : public CLICommandSetHRParam {
public:
    CLICommandSetLowThreshold(std::string name, std::string description, HardwareManager* hwManager) :
        CLICommandSetHRParam(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandSetHighThreshold : public CLICommandSetHRParam {
public:
    CLICommandSetHighThreshold(std::string name, std::string description, HardwareManager* hwManager) :
        CLICommandSetHRParam(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandGetRegister : public CLIDeviceCommand {
public:
    CLICommandGetRegister(std::string name, std::string description, HardwareManager* hwManager) :
        CLIDeviceCommand(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandGetRegistersDump : public CLIDeviceCommand {
public:
    CLICommandGetRegistersDump(std::string name, std::string description, HardwareManager* hwManager) :
        CLIDeviceCommand(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandSetRegister : public CLIDeviceCommand {
public:
    CLICommandSetRegister(std::string name, std::string description, HardwareManager* hwManager) :
        CLIDeviceCommand(name, description, hwManager) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

class CLICommandWifiInit : public CLICommand {
public:
    CLICommandWifiInit(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////

class CLICommandWifiStatus : public CLICommand {
public:
    CLICommandWifiStatus(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};

class CLICommandWifiOn : public CLICommand {
public:
    CLICommandWifiOn(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};

class CLICommandWifiOff : public CLICommand {
public:
    CLICommandWifiOff(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

class CLICommandGsmCreate : public CLICommand {
public:
    CLICommandGsmCreate(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};

class CLICommandGsmSet : public CLICommand {
public:
    CLICommandGsmSet(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};

class CloudManager;
class CLICommandGsmStatus : public CLICommand {
public:
    CLICommandGsmStatus(std::string name, std::string description, CloudManager *cloud) :
        CLICommand(name, description), m_cloud(cloud) {}

    virtual bool Execute(std::vector<std::string>& args);

private:
    CloudManager *m_cloud;
};

class CLICommandGsmOn : public CLICommand {
public:
    CLICommandGsmOn(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};

class CLICommandGsmOff : public CLICommand {
public:
    CLICommandGsmOff(std::string name, std::string description) :
        CLICommand(name, description) {}

    virtual bool Execute(std::vector<std::string>& args);
};
