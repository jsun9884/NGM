#include "Device.h"
#include "tinyxml.h"
#include "CloudManager.h"
#include "HRStatistics.h"

#include <unistd.h>
#include <syscall.h>

Device::Device(std::string name) :
    m_name(name),
    m_hrData(),
    m_devStatEnabled(0),
    m_bRunning(false)
{
    setState(SensorState::Faulty);
}

/*DevicePtr Device::create(std::string name, std::string configFileName, std::string paramFileName)
{
    DevicePtr device(new Device(name, configFileName, paramFileName));
    device->Initialize();
    return device;
}*/
void Device::saveConfiguration(TiXmlElement* elem)
{

    for(elem; elem; elem = elem->NextSiblingElement()) {
        if(elem->ValueStr() == "Parameter"){
            ParameterPtr param = m_htParameters[elem->FirstAttribute()->ValueStr()];
            if(param){
                saveParameter(elem, param);
            }
        }
        if(elem->ValueStr() == "HRData"){
            TiXmlElement* elemHr = elem->FirstChildElement();
            for(elemHr; elemHr; elemHr = elemHr->NextSiblingElement())
            {
                std::string hrParamName = elemHr->Attribute("name");
                //log::info("HR element first attribute %s", hrParamName.c_str());
                HRParameterPtr param = m_hrData->getParameter(hrParamName);
                if(param){
                    //log::info("Update HR element %s to %s", hrParamName.c_str(), param->getName().c_str());
                    saveHRParameter(elemHr, param);
                }
            }
        }
    }
}

void Device::resetStatistics()
{
    if (m_hrData) {
        log::info() << "Reset statistics for <" << m_name << ">";
        m_hrData->resetStatistics();
    }
}

bool Device::popIsStateChanged()
{
    return m_statusChanged.exchange(false);
}

void Device::saveParameter(TiXmlElement* elem, ParameterPtr param)
{
    std::ostringstream stream;
    param->getValue().print(stream);
    elem->Clear();
    elem->LinkEndChild(new TiXmlText(stream.str()));
}

void Device::saveHRParameter(TiXmlElement* elem, HRParameterPtr param)
{
    try {
        saveAttribute(elem, "defaultValue", param->getDefaultValue());
    }
    catch(std::exception&) {
        //log::info() << "Xml: defaultValue does not exist";
    }
    try {
        saveAttribute(elem, "thresholdHigh", param->getThresholdHigh());
    }
    catch(std::exception&) {
        //log::info() << "Xml: thresholdHigh does not exist";
    }
    try {
        saveAttribute(elem, "thresholdLow", param->getThresholdLow());
    }
    catch(std::exception&) {
        //log::info() << "Xml: thresholdLow does not exist";
    }
    try {
        saveAttribute(elem, "statisticsEnabled", param->getStatisticsEnabled());
    }
    catch(std::exception&) {
        //log::info() << "Xml: statisticsEnabled does not exist";
    }
}

void Device::saveAttribute(TiXmlElement* elem, std::string attributeName, AnyValue value)
{
    try {
        std::ostringstream stream;
        value.print(stream);
        //log::debug("Attribute %s, value %s",attributeName.c_str(), stream.str().c_str());
        elem->SetAttribute(attributeName.c_str(), stream.str().c_str());
    }
    catch(std::exception&) {
        //log::debug() << "Xml: defaultValue attr does not exist";
    }
}

void Device::loadConfiguration(TiXmlElement* elem)
{
    TiXmlElement* innerElem = elem->FirstChildElement();

    for (innerElem; innerElem; innerElem = innerElem->NextSiblingElement()) {
        std::string tag = innerElem->Value();
        if (tag == "Parameter") {
            loadParameter(innerElem);
        }
        else if (tag == "Register") {
            loadRegister(innerElem);
        }
        else if (tag == "HRData") {
            TiXmlElement* HRElem = innerElem->FirstChildElement();

            for (HRElem; HRElem; HRElem = HRElem->NextSiblingElement()) {
                loadHRParameter(HRElem);
            }
        }
        else {
            THROWEX(UnknownXmlSectionEx,tag);
        }
	}
}

AnyValue Device::loadValueByAttributeName(TiXmlElement *elem, ValueType valueType, std::string name)
{
    const char *value = elem->Attribute(name.c_str());
    if (!value)
        THROWEX(CantLoadParameterValueEx, name);
    return AnyValue::fromString(valueType, value);
}

AnyValue Device::loadValue(TiXmlElement *elem, ValueType valueType)
{
    const char *value = elem->GetText();
    if (!value)
        THROWEX(CantLoadParameterValueEx, "null");

    return AnyValue::fromString(valueType, value);
}


void Device::loadParameter(TiXmlElement *elem)
{
	std::string section = elem->Value();
	std::string name = elem->Attribute("name");

    ParameterPtr param = getParameter(name);
    ValueType valueType;
    AnyValue defaultValue;

    if (!param) {
        std::string type = elem->Attribute("type");
        valueType = AnyValue::typeFromString(type);

        AnyValue value          = loadValue(elem, valueType);
        try {
            defaultValue   = loadValueByAttributeName(elem, valueType, "defaultValue");
        }
        catch (...) {
            defaultValue = value;
        }

        param = Parameter::create(name, defaultValue);
        addParameter(param);
    }
    else {
        valueType = param->getType();

        try {
            defaultValue = loadValueByAttributeName(elem, valueType, "defaultValue");
            param->setDefaultValue(defaultValue);
        }
        catch(std::exception&) {
            defaultValue = param->getDefaultValue();
        }

        AnyValue value          = loadValue(elem, valueType);
        param->setValue(value);
    }

    try {
        AnyValue thresholdHigh = loadValueByAttributeName(elem, valueType, "thresholdHigh");
        param->setThresholdHigh(thresholdHigh);
        log::info() << name << ": thresholdHigh = " << thresholdHigh;
    }
    catch(std::exception&) {
        //log::info() << "Xml: thresholdHigh attr does not exist for " << name;
    }

    try {
        AnyValue thresholdLow = loadValueByAttributeName(elem, valueType, "thresholdLow");
        param->setThresholdLow(thresholdLow);
        log::info() << name <<  ": thresholdLow = " << thresholdLow;
    }
    catch(std::exception&) {
        //log::info() << "Xml: thresholdLow attr does not exist for " << name;
    }

    glog::info() << "Loading parameter: name = " << name
              << ", type = " << AnyValue::typeName(valueType)
              << ", value = " << param->getValue()
              << ", defaultValue = " << defaultValue;

    // TODO: Sample for registers
    /*TiXmlElement *child = elem->FirstChildElement();
	for (child; child; child = child->NextSiblingElement()) {
		LoadParameter(child, param);
    }*/
}

void Device::loadRegister(TiXmlElement *elem)
{
    RegisterPtr reg;
    ValueType valueType;

    std::string type = elem->Attribute("type");
    valueType = AnyValue::typeFromString(type);

    AnyValue address = loadValueByAttributeName(elem, ValueType::DWord, "address");
    AnyValue value          = loadValue(elem, valueType);

    reg = Register::create(address, value);
    addRegister(reg);

    glog::info() << "Loading register: " << "type = " << AnyValue::typeName(valueType)
              << ", address = " << address
              << ", value = " << value;
}

void Device::loadHRParameter(TiXmlElement *elem)
{
    std::string section = elem->Value();
    std::string name = elem->Attribute("name");

    HRParameterPtr param = m_hrData->getParameter(name);
    ValueType valueType;
    AnyValue defaultValue;

    if (!param){
        std::string type = elem->Attribute("type");
        valueType = AnyValue::typeFromString(type);
        if (valueType == ValueType::String) {
            THROW(UnsupportedTypeEx);
        }
        defaultValue = loadValueByAttributeName(elem, valueType, "defaultValue");

        param = HRParameter::create(name, defaultValue);
        m_hrData->AddParameter(param);
    }
    else {
        valueType = param->getType();

        try {
            defaultValue = loadValueByAttributeName(elem, valueType, "defaultValue");
            param->setDefaultValue(defaultValue);
        }
        catch(std::exception&) {
            defaultValue = param->getDefaultValue();
        }
    }

    try {
        AnyValue thresholdHigh = loadValueByAttributeName(elem, param->getType(), "thresholdHigh");
        param->setThresholdHigh(thresholdHigh);
        log::info() << name << ": thresholdHigh = " << thresholdHigh;
    }
    catch(std::exception&) {
        //log::info() << "Xml: thresholdHigh attr does not exist for " << name;
    }

    try {
        AnyValue thresholdLow = loadValueByAttributeName(elem, param->getType(), "thresholdLow");
        param->setThresholdLow(thresholdLow);
        log::info() << name <<  ": thresholdLow = " << thresholdLow;
    }
    catch(std::exception&) {
        //log::info() << "Xml: thresholdLow attr does not exist for " << name;
    }


    try {
        //should be bool - doesn't exist in AnyValue
        AnyValue statisticsEnabled = loadValueByAttributeName(elem, ValueType::Bool, "statisticsEnabled");
        param->setStatisticsEnabled((bool)statisticsEnabled);
        //log::info() << "statisticsEnabled = " << statisticsEnabled;
    }
    catch(std::exception&) {
        //log::info() << "Xml: statisticsEnabled attr does not exist for " << name;
    }

    glog::info() << "Loading HR parameter: name = " << name
              << ", type = " << AnyValue::typeName(valueType)
              << ", defaultValue = " << defaultValue
              << ", statisticsEnabled = " << (param->getStatisticsEnabled() ? "true" : "false");
}

void Device::addParameter(ParameterPtr param)
{
    //std::unique_lock<std::recursive_mutex> l(m__paramsMutex);
    if (m_htParameters.find(param->getName()) != m_htParameters.end()) {
        THROWEX(ParameterAlreadyExistsEx, param->getName());
	}
    m_htParameters.insert(std::pair<std::string, ParameterPtr>(param->getName(), param));
    m_vParameters.push_back(param);
}

void Device::addRegister(RegisterPtr reg)
{
    m_vRegisters.push_back(reg);
}

void Device::createEvent(DeviceEventType type, HRParameterPtr parameter, AnyValue value, uint64_t ts)
{
    EventPtr event;
    if(type == DeviceEventType::OverBounds) {
        event = std::make_shared<OverBoundsEvent>(/*getHRData(), getHRDataSize(), */m_name, parameter, value, ts);
    }
    else if(type == DeviceEventType::TamperSwitch) {
        event = std::make_shared<TamperSwitchEvent>(m_name, parameter, value, ts);
    }

    if (event) {
        parameter->setEvent(event);
        std::lock_guard<std::mutex> lock(m__eventsMutex);
        m__events.push_back(event);
        if (m__events.size() > MAXIMUM_EVENTS) {
            m__events.pop_front();
        }
    }
}

void Device::setState(SensorState state)
{
    m_state.store(state);
    m_statusChanged.store(true);
}

void Device::setParameter(std::string name, AnyValue value)
{
    //std::unique_lock<std::recursive_mutex> l(m__paramsMutex);
	auto it = m_htParameters.find(name);
	if (it == m_htParameters.end()) {
        THROWEX(NoSuchParameterEx, name, m_name);
	}

    it->second->setValue(value);
}

ParameterPtr Device::getParameter(std::string name)
{
    //std::unique_lock<std::recursive_mutex> l(m__paramsMutex);
	auto it = m_htParameters.find(name);
	if (it == m_htParameters.end()) {
        //THROWEX(NoSuchParameterEx, name, m_name);
        return nullptr;
	}

    return it->second;
}

HRParameterPtr Device::getHRParameter(std::string name)
{
    return m_hrData->getParameter(name);
}

std::vector<ParameterPtr> &Device::getParameters()
{
    return m_vParameters;
}

std::vector<HRParameterPtr> Device::getHRParameters()
{
    std::vector<HRParameterPtr> vHRParameters;
    for (int i=0; i<m_hrData->getParametersCount(); i++) {
        vHRParameters.push_back(m_hrData->getParameter(i));
    }

    return vHRParameters;
}

HRData::RecordData Device::getCurrentData()
{
    return m_hrData->getCurrentData();
}


ByteBufferPtr Device::getHRData()
{
    return m_hrData->getHRDataBuffer();
}

size_t Device::getHRDataSize()
{
    return m_hrData->getHRDataSize();
}

size_t Device::getPostEventTime()
{
    if (m_postEventTime) {
        return m_postEventTime->getValue();
    }
    log::warn() << "postEventTime not exist!";
    return 0;
}

void Device::applyParams(bool onlyChanges)
{
    //std::unique_lock<std::recursive_mutex> l(m__paramsMutex);
    size_t size = m_vParameters.size();
	
	for (size_t i = 0; i < size; ++i) {
        applyParameter(m_vParameters[i], onlyChanges);
    }
}

void Device::applyGeneralParams(bool onlyChanges)
{
    //std::unique_lock<std::recursive_mutex> l(m__paramsMutex);
    size_t size = m_vParameters.size();

    for (size_t i = 0; i < size; ++i) {
        applyGeneralParameter(m_vParameters[i], onlyChanges);
    }
}

void Device::applyRegisters(bool onlyChanges)
{
    try {
        size_t size = m_vRegisters.size();

        for (size_t i = 0; i < size; ++i) {
            applyRegister(m_vRegisters[i], onlyChanges);
        }
    }
    catch(std::exception &e) {
        log::warn() << e.what() << " #device: " << m_name;
    }
}

AnyValue Device::readRegister(AnyValue address)
{
    THROW(RegistersNotImplementedEx);
}

void Device::writeRegister(AnyValue address, AnyValue value)
{
    THROW(RegistersNotImplementedEx);
}

EventPtr Device::popFrontEvent()
{
    std::unique_lock<std::mutex> l(m__eventsMutex);
    EventPtr ev;
    if (m__events.size()) {
        ev = m__events.front();
        m__events.pop_front();
    }
    return ev;
}

void Device::pushBackEvent(EventPtr event)
{
    std::unique_lock<std::mutex> l(m__eventsMutex);
    m__events.push_back(event);
}

std::string Device::getName() const
{
    return m_name;
}

void Device::applyParameter(ParameterPtr param, bool onlyChanges)
{
    if (!param->hasSubParameters()) {
        if (onlyChanges && !param->isDirty())
            return;

        if( !applyGeneralParameter(param) ) {
            if( applyNativeParameter(param) )
                param->clearDirty();
        }
        else {
            param->clearDirty();
        }
	}
	else {
        std::vector<ParameterPtr> &vp = param->getSubParameters();
		size_t size = vp.size();
		for (size_t i = 0; i < size; ++i) {
            applyParameter(vp[i], onlyChanges);
		}
    }
}

void Device::applyRegister(RegisterPtr reg, bool onlyChanges)
{
    if (onlyChanges && !reg->isDirty())
        return;

    log::info() << m_name << " applyRegister: addr = " << reg->getAddress() << ", value = " << reg->getValue();
    writeRegister(reg->getAddress(), reg->getValue());
    reg->clearDirty();
}

void Device::applyGeneralParameter(ParameterPtr param, bool onlyChanges)
{
    if (!param->hasSubParameters()) {
        if (onlyChanges && !param->isDirty())
            return;

        if( !applyGeneralParameter(param) ) {

        }
        else {
            param->clearDirty();
        }
    }
    else {
        std::vector<ParameterPtr> &vp = param->getSubParameters();
        size_t size = vp.size();
        for (size_t i = 0; i < size; ++i) {
            applyGeneralParameter(vp[i], onlyChanges);
        }
    }
}

bool Device::applyGeneralParameter(ParameterPtr param)
{
    // TODO: Make parameter dependencies
    //log::info() << getName() << ": " << param->getName() << " = " << param->getValue();
    if (param->getName() == "HRFrequency") {
        size_t hr_t = 2; // min
        ParameterPtr hrTime = getParameter("HRHistorySec");
        if (hrTime) {
            if (!hrTime->isDirty()) {
                hr_t = hrTime->getValue();
            }
            else {
                return true;
            }
        }
        log::info() << "Parameter " << param->getName() << ": pass";
        size_t bsize = (size_t)param->getValue()  * hr_t;
        m_hrData->setRecordsBufferSize(bsize);
    }
    else if (param->getName() == "HRHistorySec") {
        size_t freq = 2; // min
        ParameterPtr frequency = getParameter("HRFrequency");
        if (frequency) {
            if (!frequency->isDirty()) {
                freq = frequency->getValue();
            }
            else {
                return true;
            }
        }
        log::info() << "Parameter " << param->getName() << ": pass";
        size_t bsize = freq * (size_t)param->getValue();
        m_hrData->setRecordsBufferSize(bsize);
    }
    else {
        return false;
    }
    return true;
}

void Device::preInit()
{
    // Defaults
    ParameterPtr param = Parameter::create("HRFrequency", (uint64_t)1);
    addParameter(param);

    param = Parameter::create("HRHistorySec", (uint64_t)60);
    addParameter(param);

    param = Parameter::create("HRPostEventTimeSec", (uint64_t)5);
    addParameter(param);
    m_postEventTime = param;

    param = Parameter::create("DevStatisticsEnabled", false);
    addParameter(param);

    m_hrData = std::make_shared<HRData>();
    m_hrData->initialize();
}

void Device::postInit()
{
    applyRegisters(true);
}

bool Device::start()
{
    if (m_bRunning || m_procThread.get()) {
        log::error("-- <%s> already started!", m_name.c_str());
        return false;
    }

    m_devStatEnabled = (int)getParameter("DevStatisticsEnabled")->getValue();
    if(m_devStatEnabled){
        m_hrStatistics = std::make_shared<HRStatistics>();
        m_hrStatistics->init(this);
    }

    m_bRunning = true;
    m_procThread = std::make_shared<std::thread>(&Device::threadProc, this);
    return true;
}

bool Device::isRunning()
{
    return m_bRunning;
}

bool Device::stop()
{
    if (m_hrStatistics)
        m_hrStatistics->stop();

    if (m_procThread.get()) {
        m_bRunning = false;
        m_procThread->join();
        m_procThread = nullptr;
        return true;
    }
    log::error("<%s> not started!", m_name.c_str());
    return false;
}

void Device::checkThresholds(const HRData::RecordData &rd, const HRData::RecordData &prevRD)
{
    size_t count = m_hrData->getParametersCount();
    size_t prevTS = prevRD.get(m_hrData->timestampParameter());
    HRParameterPtr param;
    int exceeded;
    int prevExceeded;

    for(size_t i = 0; i < count; ++i) {
        param = m_hrData->getParameter(i);

        if (param->eventInProcess()) {
            DeviceEventPtr e = std::dynamic_pointer_cast<DeviceEvent>(param->getEvent());
            uint64_t curr_ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            if ((e->state() == UploadState::WaitingPostData) &&
                    (curr_ts - e->timestamp()) >= (uint64_t)(getPostEventTime() * 1000)) {
                e->setHRData(getHRData());
                e->setHRDataSize(getHRDataSize());
                e->setState(UploadState::GotData);
                log::info("DeviceEvent got HRData: device=%s, id=%s, type=%s", getName().c_str(),
                          e->id().c_str(), e->getEventTypeName().c_str());
            }
            continue;
        }

        prevExceeded = false;
        if (prevTS && param->IsExceededThreshold(prevRD.get(param))) {
            prevExceeded = true;
        }

        AnyValue val = rd.get(param);
        exceeded = param->IsExceededThreshold(val);

        // Not need "phase lost" event at start time
        if (!prevTS) {
            //log::info() << param->getName() << ": prevTS == 0";
            if(exceeded < 0) {
                prevExceeded = exceeded;
            }
        }

        if (exceeded && !prevExceeded) {

            log::info() << "Threshold is exceeded: <" << m_name << ">.<" << param->getName() << "> = " << val;
            uint64_t ts = rd.get(m_hrData->timestampParameter());
            if ( onOverBounds(param, val, ts) ) {
                log::info() << "*** Create event: <" << m_name << ">.<" << param->getName() << "> = " << val;
                createEvent(DeviceEventType::OverBounds, param, val, ts);
                param->setEventInProcess(true);
            }
        }
    }
}

bool Device::onOverBounds(HRParameterPtr param, AnyValue &value, uint64_t ts)
{
    return true;
}

void Device::addStatusData(nlohmann::json &js)
{
    js["name"] = m_name;
    js["state"] = state() == SensorState::Active ? "Active" : "Faulty";
}

void Device::addRegularData(nlohmann::json &js)
{
    js["name"] = m_name;
    js["state"] = state() == SensorState::Active ? "Active" : "Faulty";

    auto d = m_hrData->getCurrentData();
    size_t count = m_hrData->getParametersCount();

    for(size_t i = 0; i < count; ++i) {
        HRParameterPtr param = m_hrData->getParameter(i);   

        if(m_devStatEnabled && param->getStatisticsEnabled()){
            param->getStatistics(js);
        }
        else{
            d.get(param).setJson("current_value", js[param->getName()]);
        }
    }
}

void Device::addShadowData(nlohmann_fixed::json &js)
{    
    nlohmann_fixed::json jsParams;
    getDeviceParamsFixed(jsParams);
    js[m_name] = jsParams;

    //auto d = m_hrData->getCurrentData();
    size_t count = m_hrData->getParametersCount();

    nlohmann_fixed::json jsHrParams;
    for(size_t i = 0; i < count; ++i) {
        HRParameterPtr param = m_hrData->getParameter(i);
        try {
            //jsHrParams[param->getName()]["defaultValue"] = param->getDefaultValue();
            param->getDefaultValue().setJsonFixed("defaultValue", jsHrParams[param->getName()]);
        }
        catch(std::exception&){}
        try {
            //jsHrParams[param->getName()]["thresholdHigh"] = param->getThresholdHigh();
            param->getThresholdHigh().setJsonFixed("thresholdHigh", jsHrParams[param->getName()]);
        }
        catch(std::exception&){}
        try {
            //jsHrParams[param->getName()]["thresholdLow"] = param->getThresholdLow();
            param->getThresholdLow().setJsonFixed("thresholdLow", jsHrParams[param->getName()]);
        }
        catch(std::exception&){}
    }
    js[m_name]["HrData"] = jsHrParams;
}

std::string Device::getEventMetadata(uint64_t ts) {
    std::stringstream ss;
    ss.precision(3);

    nlohmann::json out;
    nlohmann::json js_arr = nlohmann::json::array();
    {
        nlohmann::json js;

        size_t count = m_hrData->getParametersCount();
        for(size_t i = 0; i < count; ++i) {
            HRParameterPtr param = m_hrData->getParameter(i);
            makeJsonElem(js, param->getName(), AnyValue::typeName(param->getType()));
            addJsonElemToArr(js_arr, js);
        }
    }
    out["rows"] = js_arr;
    out["triggerTimestamp"] = ts;

    ss << std::fixed << out;

    return ss.str();
}

void Device::getDeviceParams(nlohmann::json &js)
{
    //std::unique_lock<std::recursive_mutex> l(m__paramsMutex);
    size_t count = m_vParameters.size();

     for(size_t i = 0; i < count; ++i) {
        ParameterPtr param = m_vParameters[i];
        //js[param->getName()] = param->getValue();
        param->getValue().setJson(param->getName(), js);
    }
}

void Device::getDeviceParamsFixed(nlohmann_fixed::json &js)
{
    //std::unique_lock<std::recursive_mutex> l(m__paramsMutex);
    size_t count = m_vParameters.size();

     for(size_t i = 0; i < count; ++i) {
        ParameterPtr param = m_vParameters[i];
        //js[param->getName()] = param->getValue();
        param->getValue().setJsonFixed(param->getName(), js);
    }
}

void Device::threadProc()
{
    log::info ("Device %s started! TPID: %d", m_name.c_str(), ::syscall(SYS_gettid));

    auto recordData = m_hrData->getCurrentData();
    auto prevRD = recordData;

    ParameterPtr freq = getParameter("HRFrequency");
    uint32_t dataFreq = freq ? (uint32_t)freq->getValue() : 1;
    auto timeInterval = std::chrono::microseconds(1000000) / dataFreq;

    while (m_bRunning) {
        //log::debug() << DBG_INFO << m_name;

        log::info("Connecting to <%s>", m_name.c_str());

        if (!connect()) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        log::info() << "-- PostInit";
        postInit();
        log::info() << "-- ApplyParams";
        applyParams(true);
        setState(SensorState::Active);

        log::info("Starting reading data loop for %s", m_name.c_str());
        auto timeNow = std::chrono::system_clock::now();
        auto nextTick = timeNow + timeInterval;
        auto nextSec = timeNow + std::chrono::seconds(1);
        size_t ticksPerSec = 0;
        //size_t currentIndex = 0;
        while (m_bRunning) {
            //log::debug() << DBG_INFO << m_name;

            {
                prevRD = recordData;
                if (!readData(recordData)) {
                    break;
                }

                {
                    auto record = m_hrData->getCurrent();
                    m_hrData->setCurrent(++record);
                    record.setData(recordData);

                    checkThresholds(recordData, prevRD);
                }
                ++ticksPerSec;

            }

            timeNow = std::chrono::system_clock::now();
            if (timeNow > nextTick) {
                //log::warn("<%s> reading data too slow!...", m_name.c_str());
                int i = 0;
                while (timeNow >= nextTick) {
                    nextTick += timeInterval;
                    ++i;
                    if (i > 10) {
                        nextTick = timeNow + timeInterval;
                    }
                }
            }
            else {
                if (timeNow < nextTick) {
                    std::this_thread::sleep_until(nextTick);
                }
                nextTick += timeInterval;
            }

            if (timeNow >= nextSec) {
                if (ticksPerSec < dataFreq) {
                    log::warn() << "<" << m_name << "> reading data slow! " << dataFreq - ticksPerSec << " lines are lost!";
                }
                /*else {
                    log::info() << "<" << m_name << "> " << ticksPerSec << " lines are readed...";
                }*/
                ticksPerSec = 0;
                int i = 0;
                do {
                    nextSec += std::chrono::seconds(1);
                    ++i;
                    if (i > 10) {
                        nextSec = timeNow + std::chrono::seconds(1);
                    }
                } while (timeNow >= nextSec);
            }
        }
        log::info("Reading data loop stopped for %s", m_name.c_str());
        disconnect();

        setState(SensorState::Faulty);
    }

    log::info ("Device %s stopped!", m_name.c_str());
}

void Device::makeJsonElem(nlohmann::json& js, std::string param, std::string type) {
    js["Name"] = param;
    js["Type"] = type;
}

void Device::addJsonElemToArr(nlohmann::json& js_arr, nlohmann::json& js) {
    js_arr.push_back(js);
    js.clear();
}


void Device::setShadowFromJson(nlohmann::json& json)
{
    size_t count = m_vParameters.size();
    bool bRes = false;

    for(size_t i = 0; i < count; ++i) {
        ParameterPtr param = m_vParameters.at(i);
        auto it = json.find(param->getName());
        if (it != json.end()) {

            bRes |= true;
            AnyValue val(param->getType());
            val.jsonGet(*it);
            log::info() << "setShadow param: " << it.key() << " = " << val
                        << " type: " << AnyValue::typeName(param->getType());
            param->setValue(val);
            applyParameter(param, true);
        }
    }

    auto it = json.find("HrData");
    if (it != json.end()) {
        count = m_hrData->getParametersCount();
        //log::info("HR data %s", ss.str().c_str());
        for(size_t i = 0; i < count; ++i) {
            HRParameterPtr param = m_hrData->getParameter(i);
            log::info("HR Parameter %s", param->getName().c_str());
            bRes |= CloudManager::setParamFromJson(*it, param->getName(), [&](nlohmann::json::iterator &pm_it)
            {
                    log::info() << *pm_it;

                    bRes |= CloudManager::setParamFromJson(*pm_it, "thresholdHigh", [&](nlohmann::json::iterator &it_t)
                    {
                            AnyValue val(param->getType());
                            val.jsonGet(*it_t);
                            log::info() << "Threshold high: " << val;
                            param->setThresholdHigh(val);
                     });
                    bRes |= CloudManager::setParamFromJson(*pm_it, "thresholdLow", [&](nlohmann::json::iterator &it_t){
                            AnyValue val(param->getType());
                            val.jsonGet(*it_t);
                            log::info() << "Threshold low: " << val;
                            param->setThresholdLow(val);
                     });
                    bRes |= CloudManager::setParamFromJson(*pm_it, "defaultValue", [&](nlohmann::json::iterator &it_t){
                            AnyValue val(param->getType());
                            val.jsonGet(*it_t);
                            log::info() << "Default value: " << val;
                            param->setDefaultValue(val);
                     });
                    bRes |= CloudManager::setParamFromJson(*pm_it, "statisticsEnabled", [&](nlohmann::json::iterator &it_t){
                            AnyValue val(ValueType::Bool);
                            val.jsonGet(*it_t);
                            log::info() << "Statistics Enabled value: " << val;
                            param->setStatisticsEnabled(val);
                     });

            });
            if (!bRes) {
                log::error() << "\'HrData\' json parsing error...";
            }
        }
    }
}
