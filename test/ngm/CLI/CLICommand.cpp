#include "CLICommand.h"
#include "CloudManager.h"

void CLICommand::CheckParamNumber(std::vector<std::string>& args, int minParams) {
    if (args.size() < minParams) {
        THROW(NoParametersEx);
    }
}

/////////////////////////////////////////////////////> GET_PARAMS <////////////////////////////////////////////////////////

bool CLICommandGetParams::Execute(std::vector<std::string>& args) {
	try {
        handleDevice(args);
        std::vector<ParameterPtr> vParams = m_device->getParameters();
		PrintParams(vParams);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return false;
	}
	
	return true; 
}

void CLICommandGetParams::PrintParams(std::vector<ParameterPtr>& vParams) {
    std::cout << "Parametes for device " << m_device->getName() << " >>>\n";
    for (int i = 0; i < vParams.size(); i++) {
        std::cout << i+1 << ": " << vParams[i]->getName() << " = " << vParams[i]->getValue() << "\n";
        /*if (vParams[i]->hasSubParameters()) {
            PrintSubParams(*it, 1);
        }*/
	}
}

/*void CLICommandGetParams::PrintSubParams(ParameterPtr param, int padding) {
	using std::cout;
	using std::endl;

	std::vector<ParameterPtr>& vSubParams = param->getSubParameters();
	int i = 1;
	PrintPadding(padding);
	cout << "Sub Parametes for parameter " << param->getName() << " >>>" << endl;
	for (auto it = vSubParams.begin(); it != vSubParams.end(); it++) {
		PrintPadding(padding);
        cout << i << ": " << (*it)->getName() << " = " << (*it)->getValue() << endl;
		i++;
        if ((*it)->hasSubParameters()) {
			PrintSubParams(*it,padding+1);
		}
	}
}

void CLICommandGetParams::PrintPadding(int padding) {
	for (int i = 0; i < padding; i++) {
		std::cout << '\t';
	}
}*/

/////////////////////////////////////////////////////> SET_PARAM <////////////////////////////////////////////////////////

bool CLICommandSetParam::Execute(std::vector<std::string>& args) {
    try {
        handleDevice(args);
        CheckParamNumber(args,4);

        ParameterPtr paramPtr = m_device->getParameter(args[2]);
        if (!paramPtr) {
            THROWEX(NoSuchParameterEx, args[2], m_device->getName());
        }
        ValueType paramValueType = paramPtr->getType();
        AnyValue paramValue = AnyValue::fromString(paramValueType, args[3]);
        m_device->setParameter(args[2], paramValue);

        m_hardwareManager->saveConfig();
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return false;
	}

    m_device->applyParams(true);

	return true;
}

/////////////////////////////////////////////////////> DEVICE_LIST <////////////////////////////////////////////////////////

bool CLICommandDeviceList::Execute(std::vector<std::string> &args) {
    for (int i=0; i<m_hardwareManager->getDevicesCount(); i++) {
        std::cout << i+1 << ": " << m_hardwareManager->getDevice(i)->getName() << "\n";
    }
    return true;
}

/////////////////////////////////////////////////////> GET_HR_PARAMS <////////////////////////////////////////////////////////

bool CLICommandGetHRParams::Execute(std::vector<std::string> &args) {
    try {
        handleDevice(args);
        std::vector<HRParameterPtr> vHRParams = m_device->getHRParameters();
        PrintParams(vHRParams);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

void CLICommandGetHRParams::PrintParams(std::vector<HRParameterPtr> &vParams) {
    std::cout << "HR Parametes for device " << m_device->getName() << " >>>\n";
    int countAddition = 1;

    for (int i=0; i<vParams.size(); i++) {
        if (vParams[i]->getName() != "timestamp") {
            std::cout << i+countAddition << ": " << vParams[i]->getName() << ":\n"
                      << "\tLow threshold: " << vParams[i]->getThresholdLow() << "\n"
                      << "\tHigh threshold: " << vParams[i]->getThresholdHigh() << "\n"
                      << "\tStatistics enabled: " << (vParams[i]->getStatisticsEnabled() ? "true" : "false") << "\n";
            }
        else { countAddition--; }
    }
}

/////////////////////////////////////////////////////> GET_HR_PARAM <////////////////////////////////////////////////////////

bool CLICommandGetHRParam::Execute(std::vector<std::string> &args) {
    try {
        handleDevice(args);
        CheckParamNumber(args,3);

        std::string HRParamName = args[2];
        HRParameterPtr HRParamPtr = m_device->getHRParameter(HRParamName);
        if (HRParamPtr == nullptr) {
            THROWEX(ParameterNotFoundEx,HRParamName);
        }
        PrintParam(HRParamPtr);
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

void CLICommandGetHRParam::PrintParam(HRParameterPtr Param) {
    if (Param->getName() != "timestamp") {
        std::cout << Param->getName() << ":\n"
                  << "\tLow threshold: " << Param->getThresholdLow() << "\n"
                  << "\tHigh threshold: " << Param->getThresholdHigh() << "\n"
                  << "\tStatistics enabled: " << (Param->getStatisticsEnabled() ? "true" : "false") << "\n";
    }
}

/////////////////////////////////////////////////////> SET_HR_PARAM <////////////////////////////////////////////////////////

bool CLICommandSetHRParam::Execute(std::vector<std::string> &args) {
    try {
        handleDevice(args);
        CheckParamNumber(args,5);

        m_paramValueType = findParam(args[2]);
        setThreshold(Low, args[3]);
        setThreshold(High, args[4]);
        std::string isStatistics = (args.size()>5) ? args[5] : "false";
        setStatisticsEnabled(isStatistics);

        m_hardwareManager->saveConfig();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

ValueType CLICommandSetHRParam::findParam(std::string paramName) {
    m_paramPtr = m_device->getHRParameter(paramName);
    if (!m_paramPtr) {
        THROWEX(ParameterNotFoundEx, paramName);
    }
    return m_paramPtr->getType();
}

void CLICommandSetHRParam::setThreshold(ThresholdType thresType, std::string value) {
    AnyValue thresValue = AnyValue::fromString(m_paramValueType, value);

    if (thresType == Low) {
        m_paramPtr->setThresholdLow(thresValue);
    }
    else {
        m_paramPtr->setThresholdHigh(thresValue);
    }
}

void CLICommandSetHRParam::setStatisticsEnabled(std::string value) {
    AnyValue isStatistics = AnyValue::fromString(ValueType::Bool,value);
    m_paramPtr->setStatisticsEnabled(isStatistics);
}

///***************************************************> SET_LOW_THRESHOLD ***************************************************

bool CLICommandSetLowThreshold::Execute(std::vector<std::string> &args) {
    try {
        handleDevice(args);
        CheckParamNumber(args,4);

        m_paramValueType = findParam(args[2]);
        setThreshold(Low, args[3]);

        m_hardwareManager->saveConfig();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

///***************************************************> SET_HIGH_THRESHOLD ***************************************************

bool CLICommandSetHighThreshold::Execute(std::vector<std::string> &args) {
    try {
        handleDevice(args);
        CheckParamNumber(args,4);

        m_paramValueType = findParam(args[2]);
        setThreshold(High, args[3]);

        m_hardwareManager->saveConfig();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////> GET_REGISTER <////////////////////////////////////////////////////////

bool CLICommandGetRegister::Execute(std::vector<std::string> &args) {
    try {
        handleDevice(args);
        CheckParamNumber(args,3);

        AnyValue regAddress = AnyValue::fromString(ValueType::Long,args[2]);
        AnyValue regValue = m_device->readRegister(regAddress);

        std::cout << m_deviceName << std::hex << "[0x" << regAddress << "]: 0x" << regValue << "\n";
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////> SET_REGISTER <////////////////////////////////////////////////////////

bool CLICommandSetRegister::Execute(std::vector<std::string> &args) {
    try {
        handleDevice(args);
        CheckParamNumber(args,4);

        AnyValue regAddress = AnyValue::fromString(ValueType::Long,args[2]);
        AnyValue regValue = AnyValue::fromString(ValueType::Long,args[3]);

        m_device->writeRegister(regAddress,regValue);

        std::cout << m_deviceName << "[" << regAddress << "] <= " << regValue << "\n";
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////> WIFI_INIT <////////////////////////////////////////////////////////

bool CLICommandWifiInit::Execute(std::vector<std::string> &args) {
    try {
        CheckParamNumber(args, 3);
        std::stringstream ss;
        ss << "nmcli d wifi connect " << args[1] << " password " << args[2]; // << " name WifiCon";

        //::system("nmcli c delete WifiCon && nmcli c reload");
        ::system(ss.str().c_str());
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////> WIFI_STATUS <////////////////////////////////////////////////////////

bool CLICommandWifiStatus::Execute(std::vector<std::string> &args) {
    try {
        ::system("ifconfig wlan0");
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////> GET_DATA <////////////////////////////////////////////////////////

bool CLICommandGetData::Execute(std::vector<std::string> &args)
{
    try {
        handleDevice(args);

        HRData::RecordData rd = m_device->getCurrentData();
        std::vector<HRParameterPtr> params = m_device->getHRParameters();

        std::cout << m_deviceName << ": \n\t";
        for (int i = 0; i < params.size(); ++i) {
            std::cout << params[i]->getName() << " = " << rd.get(params[i]) << "\n\t";
        }
        std::cout << "\n";
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandLedsOn::Execute(std::vector<std::string> &args)
{
    try {
        CheckParamNumber(args, 5);
        AnyValue num, r, g, b;

        num = AnyValue::fromString(ValueType::Int, args[1]);
        r = AnyValue::fromString(ValueType::Int, args[2]);
        g = AnyValue::fromString(ValueType::Int, args[3]);
        b = AnyValue::fromString(ValueType::Int, args[4]);

        m_leds->setLedOn(num, r, g, b);
        std::cout << "OK" << std::endl;
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandLedsOff::Execute(std::vector<std::string> &args)
{
    try {
        CheckParamNumber(args, 2);
        AnyValue num;

        num = AnyValue::fromString(ValueType::Int, args[1]);
        m_leds->setLedOff(num);
        std::cout << "OK" << std::endl;
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandGsmSet::Execute(std::vector<std::string> &args)
{
    try {
        CheckParamNumber(args, 3);

        std::string param = args[1];
        std::string value = args[2];
        std::string cmd = "nmcli c modify \"GSMInternet\" ";

        if (param == "Apn") {
            cmd += "gsm.apn " + value;
        }
        else if (param == "Username") {
            cmd += "gsm.username " + value;
        }
        else if (param == "Password") {
            cmd += "gsm.password " + value;
        }
        else if (param == "Number") {
            cmd += "gsm.number " + value;
        }

        if (::system(cmd.c_str()) != 0) {
            std::cout << "Error: may be Gsm connection not created?... (To create use command: Gsm Create <Apn>)" << std::endl;
        }
        else {
            std::cout << "OK" << std::endl;
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandGsmStatus::Execute(std::vector<std::string> &args)
{
    try {
        int level = m_cloud->getGsmRSSI();
        ::system("ifconfig wwan0");
        std::cout << "Gsm RSSI level: " << level << " dBm\n";
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandGsmCreate::Execute(std::vector<std::string> &args)
{
    try {
        CheckParamNumber(args, 2);

        std::string cmd = "nmcli c add type gsm ifname cdc-wdm0 con-name \"GSMInternet\" apn " + args[1];
        ::system("nmcli c delete \"GSMInternet\" > /dev/null 2> /dev/null");
        if (::system(cmd.c_str()) != 0) {
            std::cout << "Error..." << std::endl;
        }
        else {
            ::system("sync");
            std::cout << "OK" << std::endl;
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandWifiOn::Execute(std::vector<std::string> &args)
{
    try {
        ::system("nmcli r wifi on");
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandWifiOff::Execute(std::vector<std::string> &args)
{
    try {
        ::system("nmcli r wifi off");
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandGsmOff::Execute(std::vector<std::string> &args)
{
    try {
        ::system("nmcli r wwan off");
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandGsmOn::Execute(std::vector<std::string> &args)
{
    try {
        ::system("nmcli r wwan on");
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool CLICommandGetRegistersDump::Execute(std::vector<std::string> &args)
{
    try {
        handleDevice(args);
        CheckParamNumber(args,4);

        AnyValue regAddress = AnyValue::fromString(ValueType::Long,args[2]);
        AnyValue regInc(ValueType::Long);
        AnyValue regValue;
        int c = AnyValue::fromString(ValueType::Long,args[3]);
        regInc = 1;

        for (int i = 0; i < c; ++i) {
            regValue = m_device->readRegister(regAddress);
            std::cout << m_deviceName << std::hex << "[0x" << regAddress << "]: 0x" << regValue << "\n";
            regAddress += regInc;
        }
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    return true;
}
