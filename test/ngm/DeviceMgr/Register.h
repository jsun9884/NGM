#ifndef REGISTER_H
#define REGISTER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include "AnyValue.h"

class Register;
typedef std::shared_ptr<Register> RegisterPtr;

class Register
{
private:
    mutable std::mutex m_valueMutex;
    AnyValue m_addr;
    AnyValue m_value;
    bool m_isDirty;

private:
    Register(AnyValue address, AnyValue value, bool isDirty = true);

public:
    Register() = delete;
    Register(const Register &) = delete;
    Register(Register &&) = delete;

    static RegisterPtr create(AnyValue address, AnyValue value, bool isDirty = true);

    AnyValue getAddress() const;
    AnyValue getValue()	const;
    void setValue(const AnyValue &value);

    void clearDirty() { m_isDirty = false; }
    bool isDirty() { return m_isDirty; }
};

#endif // REGISTER_H
