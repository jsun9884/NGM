#ifndef HRDATA_H
#define HRDATA_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <string.h>

#include "Log.h"
#include "AnyValue.h"
#include "HRParameter.h"

typedef std::shared_ptr<unsigned char> ByteBufferPtr;

class Device;

class HRData
{
    LOG_MODULE(HRData, g_main_log);

private:
    std::vector<HRParameterPtr> m_vParameters;
    std::unordered_map<std::string, HRParameterPtr> m_htParameters;

    mutable std::recursive_mutex m_mutex;
    ByteBufferPtr m_data;
    size_t m_size;
    size_t m_recordSize;
    size_t m_currentIndex;
    HRParameterPtr m_timestampParam;

protected:
    void initialize();
    void AddParameter(HRParameterPtr param);

public:
    class Record;
    class RecordData
#ifdef ENABLE_OBJECT_COUNTER
            : public ObjectCounter
#endif
    {
    private:
        HRData *m_owner;
        ByteBufferPtr m_data;
        size_t m_size;

    public:
        RecordData(HRData *b, unsigned char *data, size_t size):
#ifdef ENABLE_OBJECT_COUNTER
            ObjectCounter(ObjectCounter::RECORD_DATA),
#endif
            m_owner(b), m_data(new unsigned char[size], std::default_delete<unsigned char[]>() ),
            m_size(size) {
            memcpy(m_data.get(), data, size);
        }
        RecordData(RecordData&& o):
#ifdef ENABLE_OBJECT_COUNTER
            ObjectCounter(ObjectCounter::RECORD_DATA),
#endif
            m_owner(o.m_owner), m_data(o.m_data), m_size(o.m_size) {
            o.m_data = nullptr; o.m_size = 0;
        }
        RecordData() = delete;
        RecordData(const RecordData &o):
#ifdef ENABLE_OBJECT_COUNTER
            ObjectCounter(ObjectCounter::RECORD_DATA),
#endif
            m_owner(o.m_owner),
            m_data(new unsigned char[o.m_size], std::default_delete<unsigned char[]>() ),
            m_size(o.m_size){
            memcpy(m_data.get(), o.m_data.get(), o.m_size);
        }

        RecordData& operator=(const RecordData &o) {
            if ((m_owner != o.m_owner) || (m_size != o.m_size)) THROW(CantAssignDiffObjectsEx);
            memcpy(m_data.get(), o.m_data.get(), o.m_size);
        }

        AnyValue get(HRParameterPtr param) const {
            if (param.get()) {
                AnyValue value(param->getType());
                value = static_cast<void *>(m_data.get() + param->getOffset());
                return value;
            } else {
                return AnyValue();
            }
        }

        AnyValue get(size_t index) const {
            HRParameterPtr param = m_owner->getParameter(index);
            return get(param);
        }

        AnyValue get(std::string paramName) const {
            HRParameterPtr param = m_owner->getParameter(paramName);
            return get(param);
        }

        template<class T>
        void set(HRParameterPtr param, const T &value) {
            if (param) {
                void *out = static_cast<void *>(m_data.get() + param->getOffset());
                AnyValue::assign(param->getType(), out, value);
                param->setSqlRowValue(value);
            }
        }

        template<class T>
        void set(size_t index, const T &value) {
            HRParameterPtr param = m_owner->getParameter(index);
            set(param, value);
        }

        template<class T>
        void set(std::string paramName, const T &value) {
            HRParameterPtr param = m_owner->getParameter(paramName);
            set(param, value);
        }

        friend class Record;
    };

    class Record {
    private:
        HRData *m_owner;
        size_t m_index;

    public:
        Record():
            m_owner(nullptr), m_index(0) {}

        Record(HRData *b):
            m_owner(b), m_index(b->m_currentIndex) {
            m_owner->m_mutex.lock();
        }

        Record(const Record &) = default;

        ~Record() {
            if (m_owner) {
                m_owner->m_mutex.unlock();
            }
        }

        void setData(const RecordData &data) {
            memcpy(m_owner->m_data.get() + m_owner->m_recordSize * m_index, data.m_data.get(), data.m_size);
        }

        RecordData getData() const {
            return RecordData(m_owner, m_owner->m_data.get() + m_owner->m_recordSize * m_index, m_owner->m_recordSize);
        }

        Record &operator++() {
            ++m_index;
            if(m_index >= m_owner->m_size)
                m_index = 0;
            return *this;
        }

        Record &operator--() {
            if(m_index == 0)
                m_index = m_owner->m_size ? m_owner->m_size - 1 : 0;
            else
                --m_index;
            return *this;
        }

        size_t index() const {
            return m_index;
        }
    };

public:
    HRData();
    HRData(const HRData &) = delete;
    HRData(HRData&&) = delete;

    void setRecordsBufferSize(size_t size);

    Record getCurrent();
    void setCurrent(const Record &record);

    RecordData getCurrentData();

    size_t getParametersCount() const;
    HRParameterPtr getParameter(std::string name) const;
    HRParameterPtr getParameter(size_t index) const;
    HRParameterPtr timestampParameter() const;

    ByteBufferPtr getHRDataBuffer() const;
    size_t getHRDataSize() const;

    void resetStatistics();

    friend class Device;
};

typedef std::shared_ptr<HRData> HRDataPtr;

#endif // HRDATA_H
