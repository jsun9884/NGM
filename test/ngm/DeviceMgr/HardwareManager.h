#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <functional>
#include "Device.h"

#pragma once

class HardwareManager {		// singleton class
private:
	//------------------------
    HardwareManager() {}
	HardwareManager(const HardwareManager&) = delete;
	const HardwareManager& operator=(const HardwareManager&) = delete;

private:
    //std::recursive_mutex m_mutex;
    std::unordered_map<std::string, ISupportConfigurationPtr> m_mdevices;
    std::vector<ISupportConfigurationPtr> m_vdevices;
    std::function<void(void)> m_saveConfigFunc;

public:
    ~HardwareManager();

    static HardwareManager *instance() {
		static HardwareManager* singleHWM = new HardwareManager();
		return singleHWM;
	}

    ISupportConfigurationPtr getDevice(std::string name);
    ISupportConfigurationPtr getDevice(int index);
    int getDevicesCount();
    void addDevice(ISupportConfigurationPtr device);

    void startAll();
    void stopAll();

    void setSaveConfigFunc(std::function<void(void)> func);
    void saveConfig();
};
