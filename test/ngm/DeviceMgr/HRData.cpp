#include "HRData.h"
#include "string.h"

HRData::HRData():
    m_size(0),
    m_recordSize(0),
    m_currentIndex(0)
{

}

void HRData::setRecordsBufferSize(size_t size)
{
    if (!m_vParameters.size()) {
        THROW(NoParametersEx);
    }
    log::info() << "setRecordsBufferSize: " << std::dec << size;
    std::unique_lock<std::recursive_mutex> l(m_mutex);
    size_t native_size = m_recordSize * size;
    m_data.reset(new unsigned char[m_recordSize * size], std::default_delete<unsigned char[]>() );
    memset(m_data.get(), 0, size);
    m_size = size;

    log::info() << "HRBufferSize: " << native_size << ", RecordSize: " << m_recordSize;

    // set defaults
    Record r = getCurrent();
    RecordData d = r.getData();
    for (int i = 0; i < m_vParameters.size(); ++i) {
        HRParameterPtr p = m_vParameters[i];
        d.set(p, p->getDefaultValue());
    }
    for (int i = 0; i < size; ++i) {
        r.setData(d);
        ++r;
    }
}

HRData::Record HRData::getCurrent()
{
    return Record(this);
}

void HRData::setCurrent(const HRData::Record &record)
{
    m_currentIndex = record.index();
}

HRData::RecordData HRData::getCurrentData()
{
    return Record(this).getData();
}

size_t HRData::getParametersCount() const
{
    return m_vParameters.size();
}

HRParameterPtr HRData::getParameter(std::string name) const
{
    auto it = m_htParameters.find(name);
    if (it == m_htParameters.end()) {
        //THROWEX(ParameterNotFoundEx, name);
        return nullptr;
    }
    return it->second;
}

HRParameterPtr HRData::getParameter(size_t index) const
{
    if (index >= m_vParameters.size()) {
        //THROW(IndexOutOfRangeEx);
        return nullptr;
    }
    return m_vParameters[index];
}

HRParameterPtr HRData::timestampParameter() const
{
    return m_timestampParam;
}

ByteBufferPtr HRData::getHRDataBuffer() const
{
    size_t sz = getHRDataSize(), szToEnd;
    ByteBufferPtr data(new unsigned char[sz]);
    std::lock_guard<std::recursive_mutex> l(m_mutex);
    if (m_currentIndex < (m_size - 1)) {
        szToEnd = (m_size - m_currentIndex - 1) * m_recordSize;
        memcpy((void*)data.get(), (void*) (m_data.get() + (m_currentIndex + 1) * m_recordSize), szToEnd);
        memcpy((void*)(data.get() + szToEnd), (void*) m_data.get(), (m_currentIndex + 1) * m_recordSize);
    }
    else {
        memcpy((void*)(data.get()), (void*) m_data.get(), m_size * m_recordSize);
    }
    return data;
}

size_t HRData::getHRDataSize() const
{
    return m_recordSize * m_size;
}

void HRData::resetStatistics()
{
    for (size_t i = 0, sz = m_vParameters.size(); i < sz; ++i) {
        m_vParameters[i]->resetStatistics();
    }
}

void HRData::initialize()
{
    AnyValue defaultValue = AnyValue(ValueType::QWord);
    defaultValue = 0;
    m_timestampParam = HRParameter::create("timestamp", defaultValue);
    AddParameter(m_timestampParam);
}

void HRData::AddParameter(HRParameterPtr param)
{
    std::unique_lock<std::recursive_mutex> l(m_mutex);
    if (m_htParameters.find(param->getName()) != m_htParameters.end()) {
        THROWEX(ParameterAlreadyExistsEx, param->getName());
    }

    param->setOffset(m_recordSize);
    m_htParameters.insert(std::pair<std::string, HRParameterPtr>(param->getName(), param));
    m_vParameters.push_back(param);

    m_recordSize += AnyValue::nativeSizeOf(param->getType());

    if(m_size) {
        log::info() << "!!! Resize buffer: " << m_size;
        setRecordsBufferSize(m_size);
    }
}
