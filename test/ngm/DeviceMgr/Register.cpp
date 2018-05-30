#include "Register.h"

Register::Register(AnyValue address, AnyValue value, bool isDirty):
    m_addr(address), m_value(value), m_isDirty(isDirty)
{

}

RegisterPtr Register::create(AnyValue address, AnyValue value, bool isDirty)
{
    return RegisterPtr(new Register(address, value, isDirty));
}

AnyValue Register::getAddress() const
{
    return m_addr;
}

AnyValue Register::getValue() const
{
    std::lock_guard<std::mutex> l(m_valueMutex);
    return m_value;
}

void Register::setValue(const AnyValue &value)
{
    std::lock_guard<std::mutex> l(m_valueMutex);
    m_value = value;
    m_isDirty = true;
}

