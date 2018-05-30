#pragma once

#include "Device.h"

class SampleDevice : public Device
{
private:
	int m_regularMsgTransmissionRate;
	bool m_autoHrEnabled;

private:
	SampleDevice(std::string name, std::string configFileName, std::string paramFileName);

protected:
	virtual void ApplyNativeParameter(ParameterPtr param);

public:
	~SampleDevice();

	static DevicePtr create(std::string name, std::string configFileName, std::string paramFileName);
};

