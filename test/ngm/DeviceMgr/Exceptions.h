#include <exception>
#include <string>

#pragma once

#define THROW(ex) throw ex(__FILE__, __FUNCTION__, __LINE__);
#define THROWEX(ex, ...) throw ex(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);

class Exception : public std::exception {
protected:
    std::string m_msg;

public:
    Exception(std::string file, std::string function, int line, std::string msg) {
        m_msg = file + " " + function + " L#" + std::to_string(line) + ": ";
        m_msg += msg;
    }
    virtual const char* what() const throw() {
        return m_msg.c_str();
    }
};
///////////////////////// ^^^ BASE EX ^^^ //////////////////////////

class DeviceListEmptyEx : public Exception {
public:
    DeviceListEmptyEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Device list is empty") {}
};

class NoSuchDeviceEx : public Exception {
public:
    NoSuchDeviceEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Device is not found") {}
};

class NoSuchParameterEx : public Exception {
public:
    NoSuchParameterEx(const char* file, const char* function, int line, std::string param, std::string device) :
        Exception(file, function, line, "No such parameter ")
    {
        m_msg += param + " in device " + device;
    }
};

class NoSuchSubParameterEx : public Exception {
public:
    NoSuchSubParameterEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "SubParameter is not found") {}
};

class WrongTypeCastEx : public Exception {
public:
    WrongTypeCastEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Wrong type cast") {}
};

class WrongTypeAssignmentEx : public Exception {
public:
    WrongTypeAssignmentEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Wrong type assignment") {}
};

class NativeParameterNotImplementedEx : public Exception {
public:
    NativeParameterNotImplementedEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Native parameter is not implemented") {}
};

class DeviceDuplicateNameEx : public Exception {
public:
    DeviceDuplicateNameEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Device with same name already exists in the structure") {}
};

class CantLoadParametersXmlEx : public Exception {
public:
    CantLoadParametersXmlEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Can't load device parameters xml file") {}
};

class CantLoadParameterValueEx : public Exception {
public:
    CantLoadParameterValueEx(const char* file, const char* function, int line, std::string param) :
        Exception(file, function, line, "Can't load device parameter value: ")
    {
        m_msg += std::string("\'") + param + "\'";
    }
};

class ParameterAlreadyExistsEx : public Exception {
public:
    ParameterAlreadyExistsEx(const char* file, const char* function, int line, std::string param) :
        Exception(file, function, line, "Parameter already exists") {
        m_msg += " \"" + param + "\"";
    }
};

class ParameterNotFoundEx : public Exception {
public:
    ParameterNotFoundEx(const char* file, const char* function, int line, std::string param) :
        Exception(file, function, line, "Parameter not found") {
        m_msg += " \"" + param + "\"";
    }
};

class UnknownSectionTypeEx : public Exception {
public:
    UnknownSectionTypeEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Unknown parameter section type") {}
};

class UnknownValueTypeNameEx : public Exception {
public:
    UnknownValueTypeNameEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Unknown ValueType name") {}
};

class UnsupportedTypeEx : public Exception {
public:
    UnsupportedTypeEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Unsupported type...") {}
};

class NoValueEx : public Exception {
public:
    NoValueEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Parameter value is missing") {}
};

class DeviceMissingEx : public Exception {
public:
    DeviceMissingEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Device is missing") {}
};

class AnyValueComparingBetweenDiffTypesEx : public Exception {
public:
    AnyValueComparingBetweenDiffTypesEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "AnyValue: Comparing values with different types") {}
};

class AnyValueAssignWithDiffTypeEx : public Exception {
public:
    AnyValueAssignWithDiffTypeEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "AnyValue: Assigning value with different type") {}
};

class CommandNotFoundEx : public Exception {
public:
    CommandNotFoundEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Command not found") {}
};

class NoArgsEx : public Exception {
public:
    NoArgsEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Missing arguments") {}
};

class NoParametersEx : public Exception {
public:
    NoParametersEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Missing parameters") {}
};

class IndexOutOfRangeEx : public Exception {
public:
    IndexOutOfRangeEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Index out of range") {}
};

class CantAssignDiffObjectsEx : public Exception {
public:
    CantAssignDiffObjectsEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Can't assign different objects") {}
};

class RegistersNotImplementedEx : public Exception {
public:
    RegistersNotImplementedEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Read / Write Registers not implemented...") {}
};

class RegisterReadEx : public Exception {
public:
    RegisterReadEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Read Register failed...") {}
};

class RegisterWriteEx : public Exception {
public:
    RegisterWriteEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "Write Register failed...") {}
};

class UnknownXmlSectionEx : public Exception {
public:
    UnknownXmlSectionEx(const char* file, const char* function, int line, std::string param) :
        Exception(file, function, line, "Unknown Xml Section: ")
    {
        m_msg += std::string(" \"") + param + "\"";
    }
};

class UnknownXmlDeviceEx : public Exception {
public:
    UnknownXmlDeviceEx(const char* file, const char* function, int line, std::string device) :
        Exception(file, function, line, "Unknown Xml Device: ")
    {
        m_msg += std::string(" \"") + device + "\"";
    }
};

// -----------

class AwsClientInitializationFailedEx : public Exception {
public:
    AwsClientInitializationFailedEx(const char* file, const char* function, int line) :
        Exception(file, function, line, "AwsClient initialization failed") {}
};
