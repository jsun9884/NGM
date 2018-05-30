#include "SampleDevice.h"

SampleDevice::SampleDevice(std::string name, std::string configFileName, std::string paramFileName) :
Device(name, configFileName, paramFileName), m_regularMsgTransmissionRate(2), m_autoHrEnabled(false)
{
}

SampleDevice::~SampleDevice()
{
}

void SampleDevice::ApplyNativeParameter(ParameterPtr param)
{
	std::string name = param->GetName();
	if (name == "regularMsgTransmissionRate") {
		m_regularMsgTransmissionRate = (int)param->GetValue();

		log::info() << "ApplyNativeParameter: regularMsgTransmissionRate changed to " << m_regularMsgTransmissionRate << std::endl;
	}
	else if (name == "autoHrEnabled") {
		m_autoHrEnabled = param->GetValue();

		log::info() << "ApplyNativeParameter: autoHrEnabled changed to " << (m_autoHrEnabled ? "true" : "false") << std::endl;
	}
	else {
		throw NativeParameterNotImplementedEx();
	}
}

DevicePtr SampleDevice::create(std::string name, std::string configFileName, std::string paramFileName)
{
	DevicePtr device(new SampleDevice(name, configFileName, paramFileName));
	device->Initialize();
	return device;
}