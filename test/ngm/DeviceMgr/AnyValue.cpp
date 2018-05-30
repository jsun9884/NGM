#include <sstream>
#include "AnyValue.h"

AnyValue::AnyValue() :
#ifdef ENABLE_OBJECT_COUNTER
    ObjectCounter(ObjectCounter::ANY_VALUE),
#endif
    m_type(ValueType::Empty), m_value(nullptr) {}

AnyValue::AnyValue(AnyValue &&r) :
#ifdef ENABLE_OBJECT_COUNTER
    ObjectCounter(ObjectCounter::ANY_VALUE),
#endif
    m_type(ValueType::Empty), m_value(nullptr)
{
    std::swap(m_type, r.m_type);
    std::swap(m_value, r.m_value);
}

AnyValue::AnyValue(ValueType vType) :
#ifdef ENABLE_OBJECT_COUNTER
    ObjectCounter(ObjectCounter::ANY_VALUE),
#endif
    m_type(vType) {
    TypeTraitsList::alloc(*this);
}


AnyValue::AnyValue(const AnyValue &o) :
#ifdef ENABLE_OBJECT_COUNTER
    ObjectCounter(ObjectCounter::ANY_VALUE),
#endif
    m_type(o.m_type) {
    TypeTraitsList::allocCopy(*this, o.m_value);
}

AnyValue::~AnyValue()
{
    if (m_type != ValueType::Empty) {
        TypeTraitsList::release(*this);
        //m_type = ValueType::Empty;
    }
}

bool AnyValue::empty() const
{
    return m_value == nullptr;
}

AnyValue &AnyValue::operator+=(const AnyValue &value)
{
    TypeTraitsList::add(*this, value);
    return *this;
}

AnyValue AnyValue::operator+(const AnyValue &value)
{
    AnyValue out = *this;
    out += value;
    return out;
}

AnyValue &AnyValue::operator-=(const AnyValue &value)
{
    TypeTraitsList::sub(*this, value);
    return *this;
}

AnyValue AnyValue::operator-(const AnyValue &value)
{
    AnyValue out = *this;
    out -= value;
    return out;
}

AnyValue &AnyValue::operator*=(const AnyValue &value)
{
    TypeTraitsList::mul(*this, value);
    return *this;
}

AnyValue AnyValue::operator*(const AnyValue &value)
{
    AnyValue out = *this;
    out *= value;
    return out;
}

AnyValue &AnyValue::operator/=(const AnyValue &value)
{
    TypeTraitsList::div(*this, value);
    return *this;
}

AnyValue AnyValue::operator/(const AnyValue &value)
{
    AnyValue out = *this;
    out /= value;
    return out;
}

void AnyValue::assign(ValueType type, void *out, const AnyValue &value)
{
    if (type != value.m_type) {
        THROW(AnyValueAssignWithDiffTypeEx);
    }
    TypeTraitsList::assign(type, out, value.m_value);
}

void AnyValue::setJson(std::string key, nlohmann::json &js) const
{
    TypeTraitsList::setJson(*this, key, js);
}

void AnyValue::setJsonFixed(std::string key, nlohmann_fixed::json &js) const
{
    TypeTraitsList::setJsonFixed(*this, key, js);
}

void AnyValue::jsonGet(nlohmann::json &js)
{
    TypeTraitsList::jsonGet(m_type, m_value, js);
}

AnyValue &AnyValue::operator=(void *in)
{
    if (m_type == ValueType::Empty) {
        THROW(UnknownValueTypeNameEx);
    }
    TypeTraitsList::assign(m_type, m_value, in);
    return *this;
}

AnyValue &AnyValue::operator=(const AnyValue &value)
{
    if (m_type == ValueType::Empty) {
        m_type = value.m_type;
        TypeTraitsList::alloc(*this);
    }
    else if (m_type != value.m_type) {
        THROW(AnyValueAssignWithDiffTypeEx);
    }
    TypeTraitsList::assign(m_type, m_value, value.m_value);
    return *this;
}

AnyValue &AnyValue::operator=(AnyValue &&r)
{
    std::swap(m_type, r.m_type);
    std::swap(m_value, r.m_value);
    return *this;
}

bool AnyValue::operator>(const AnyValue &value) const
{
    if (m_type != value.m_type) {
        THROW(AnyValueComparingBetweenDiffTypesEx);
    }
    return TypeTraitsList::compare(*this, value) > 0;
}

bool AnyValue::operator<(const AnyValue &value) const
{
    if (m_type != value.m_type) {
        THROW(AnyValueComparingBetweenDiffTypesEx);
    }
    return TypeTraitsList::compare(*this, value) < 0;
}

bool AnyValue::operator>=(const AnyValue &value) const
{
    if (m_type != value.m_type) {
        THROW(AnyValueComparingBetweenDiffTypesEx);
    }
    return TypeTraitsList::compare(*this, value) >= 0;
}

bool AnyValue::operator<=(const AnyValue &value) const
{
    if (m_type != value.m_type) {
        THROW(AnyValueComparingBetweenDiffTypesEx);
    }
    return TypeTraitsList::compare(*this, value) <= 0;
}

bool AnyValue::operator==(const AnyValue &value) const
{
    if (m_type != value.m_type) {
        THROW(AnyValueComparingBetweenDiffTypesEx);
    }
    return TypeTraitsList::compare(*this, value) == 0;
}

bool AnyValue::operator!=(const AnyValue &value) const
{
    if (m_type != value.m_type) {
        THROW(AnyValueComparingBetweenDiffTypesEx);
    }
    return TypeTraitsList::compare(*this, value) != 0;
}

size_t AnyValue::nativeSize() const
{
    return TypeTraitsList::nativeSize(m_type);
}

size_t AnyValue::nativeSizeOf(ValueType t)
{
    return TypeTraitsList::nativeSize(t);
}

bool AnyValue::isSigned(ValueType t)
{
    return TypeTraitsList::isSigned(t);
}

AnyValue AnyValue::fromString(ValueType valType, std::string value)
{
    try {
        return TypeTraitsList::fromString(valType, value);
    }
    catch (std::exception &e) {
        THROWEX(CantLoadParameterValueEx, value);
    }
}

std::ostream& AnyValue::print(std::ostream& os) const
{
	switch (m_type)
	{
	case ValueType::Bool:
		os << (*static_cast<bool *>(m_value) ? "true" : "false");
		break;
    default:
        TypeTraitsList::print(*this, os);
		break;
	}
	return os;
}

const char * AnyValue::s_typeNames[] = { "Empty", "Char", "Short", "Int", "Long", "Byte", "Word", "DWord", "QWord", "Float", "Double", "Bool", "String" };

ValueType AnyValue::typeFromString(std::string name)
{
    for(int i = 0; i < (unsigned int)ValueType::COUNT; ++i) {
        if (name == s_typeNames[i]) {
            return ValueType(i);
        }
    }

    THROW(UnknownValueTypeNameEx);
}

std::string AnyValue::typeName(ValueType t)
{
    unsigned int i = (unsigned int)t;
    if (i < (unsigned int)ValueType::COUNT) {
        return s_typeNames[i];
    }

    THROW(UnknownValueTypeNameEx);
}

std::ostream& operator<<(std::ostream& os, const AnyValue& any) {
	return any.print(os);
}
