#include "HardwareManager.h"
#include "Exceptions.h"


HardwareManager::~HardwareManager()
{}

ISupportConfigurationPtr HardwareManager::getDevice(std::string name)
{
    //std::lock_guard<std::recursive_mutex> l(m_mutex);

	auto it = m_mdevices.find(name);
	if (it == m_mdevices.end()) {
        THROW(NoSuchDeviceEx);
	}

	return it->second;
}

ISupportConfigurationPtr HardwareManager::getDevice(int index)
{
    //std::lock_guard<std::recursive_mutex> l(m_mutex);
    if (m_vdevices.size() <= index) {
        THROW(NoSuchDeviceEx);
	}
	
	return m_vdevices[index];
}

void HardwareManager::addDevice(ISupportConfigurationPtr device)
{
    //std::lock_guard<std::recursive_mutex> l(m_mutex);
	if (m_mdevices.find(device->getName()) != m_mdevices.end()) {  
        THROW(DeviceDuplicateNameEx);
	}

    m_mdevices.insert(std::pair<std::string, ISupportConfigurationPtr>(device->getName(), device));
	m_vdevices.push_back(device);
}

void HardwareManager::startAll()
{
    //std::lock_guard<std::recursive_mutex> l(m_mutex);
    for(int i=0; i < m_vdevices.size(); i++) {
        std::static_pointer_cast<Device>(m_vdevices[i])->start();
    }
}

void HardwareManager::stopAll()
{
    //std::lock_guard<std::recursive_mutex> l(m_mutex);
    for(int i=0; i < m_vdevices.size(); i++) {
        std::static_pointer_cast<Device>(m_vdevices[i])->stop();
    }
}

void HardwareManager::setSaveConfigFunc(std::function<void ()> func)
{
    m_saveConfigFunc = func;
}

void HardwareManager::saveConfig()
{
    m_saveConfigFunc();
}

int HardwareManager::getDevicesCount()
{
     return m_vdevices.size();
}
