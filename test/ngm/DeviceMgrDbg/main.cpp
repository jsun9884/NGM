#include <iostream>

#include "HardwareManager.h"
#include "SampleDevice.h"

int main(int argc, char **argv)
{
	try {
		HardwareManager *hwMgr = HardwareManager::Instance();

		DevicePtr device = SampleDevice::create("msp430", "msp430Config.ini", "msp430Params.xml");
		hwMgr->AddDevice(device);

		
	}
	catch (std::exception &e) {
		log::info() << "Exception: " << e.what();
	}
	std::cin.get();

	return 0;
}