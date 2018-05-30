#pragma once

#include <sstream>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <string>
#include "Exceptions.h"
#include "json.hpp"
#include "json_fixed.hpp"

#include "ObjectCounter.h"

enum class ValueType {
    Empty = 0,
    Char,
    Short,
    Int,
    Long,
    Byte,
    Word,
    DWord,
    QWord,
    Float,
    Double,
    Bool,
    String,
    COUNT
};

class AnyValue
#ifdef ENABLE_OBJECT_COUNTER
        : public ObjectCounter
#endif
{
private:
    ValueType m_type;
    void* m_value;
    static const char* s_typeNames[];

protected:
    template<ValueType VT>
    struct ctype;

    template<ValueType VT>
    using native = typename AnyValue::ctype < VT >::type;

    template<ValueType VT>
    using ss_type = typename AnyValue::ctype < VT >::ss_type;

    template<ValueType VT>
    static inline native<VT> *native_cast(void *value) {
        return static_cast<native<VT> *>(value);
    }

    template<ValueType VT>
    inline void allocCopy(void * const value) {
        m_value = static_cast<void *>(new native<VT>());
        *native_cast<VT>(m_value) = *native_cast<VT>(value);
    }

    template<ValueType VT>
    inline void alloc() {
        m_value = static_cast<void *>(new native<VT>());
    }

    template<ValueType VT>
    inline void release() {
        delete native_cast<VT>(m_value);
    }

    template<ValueType VT, class Next = void>
    struct BaseTypeTraits {
        static inline void allocCopy(AnyValue &o, void * const value) {
            if (o.m_type == VT) {
                o.allocCopy<VT>(value);
            }
            else {
                Next::allocCopy(o, value);
            }
        }

        static inline void alloc(AnyValue &o) {
            if (o.m_type == VT) {
                o.alloc<VT>();
            }
            else {
                Next::alloc(o);
            }
        }

        static inline void release(AnyValue &o) {
            if (o.m_type == VT) {
                o.release<VT>();
            }
            else {
                Next::release(o);
            }
        }

        static inline std::ostream& print(const AnyValue &o, std::ostream &os) {
            if (o.m_type == VT) {
                os << static_cast<ss_type<VT>>(*native_cast<VT>(o.m_value));
                return os;
            }
            else {
                return Next::print(o, os);
            }
        }

        static inline int compare(const AnyValue &o, const AnyValue &value) {
            if (o.m_type == VT) {
                if( (*native_cast<VT>(o.m_value)) > value.cast<native<VT>>() ) {
                    return 1;
                }
                else if ( *native_cast<VT>(o.m_value) == value.cast<native<VT>>() ) {
                    return 0;
                }
                else {
                    return -1;
                }
            }
            else {
                return Next::compare(o, value);
            }
        }

        static inline void add(AnyValue &o, const AnyValue &value) {
            if (o.m_type == VT) {
                *native_cast<VT>(o.m_value) += value.cast<native<VT>>();
            }
            else {
                Next::add(o, value);
            }
        }

        static inline void sub(AnyValue &o, const AnyValue &value) {
            Next::sub(o, value);
        }

        static inline void mul(AnyValue &o, const AnyValue &value) {
            Next::mul(o, value);
        }

        static inline void div(AnyValue &o, const AnyValue &value) {
            Next::div(o, value);
        }

        static inline void assign(ValueType t, void *out, void *in) {
            if (t == VT) {
                *native_cast<VT>(out) = *native_cast<VT>(in);
            }
            else {
                Next::assign(t, out, in);
            }
        }

        static inline void jsonGet(ValueType t, void *out, nlohmann::json &js) {
            if (t == VT) {
                *native_cast<VT>(out) = js.get<ss_type<VT>>();
            }
            else {
                Next::jsonGet(t, out, js);
            }
        }

        static inline void setJson(const AnyValue &o, std::string key, nlohmann::json &js) {
            if (o.m_type == VT) {
                js[key] = *native_cast<VT>(o.m_value);
            }
            else {
                Next::setJson(o, key, js);
            }
        }

        static inline void setJsonFixed(const AnyValue &o, std::string key, nlohmann_fixed::json &js) {
            if (o.m_type == VT) {
                js[key] = *native_cast<VT>(o.m_value);
            }
            else {
                Next::setJsonFixed(o, key, js);
            }
        }

        static inline AnyValue fromString(ValueType t, std::string value) {
            if (t == VT) {
                return AnyValue(NativeFromString<ctype<VT>>(value));
            }
            else {
                return Next::fromString(t, value);
            }
        }

        static inline size_t nativeSize(ValueType t) {
            if (t == VT) {
                return sizeof(native<VT>);
            }
            else {
                return Next::nativeSize(t);
            }
        }

        static inline size_t isSigned(ValueType t) {
            return Next::isSigned(t);
        }
    };

    template<ValueType VT, class Next = void>
    struct NumTypeTraits: public BaseTypeTraits<VT, Next>
    {
        template<class T>
        static typename std::enable_if<not std::is_same<T, std::string>::value, T>::type
        cast(ValueType t, void *value, T*p) {
            if (t == VT) {
                return static_cast<T>(*native_cast<VT>(value));
            }
            else {
                return Next::cast(t, value, p);
            }
        }

        template<class T>
        static typename std::enable_if<std::is_same<T, std::string>::value, T>::type
        cast(ValueType t, void *value, T*p) {
            return Next::cast(t, value, p);
        }

        using BaseTypeTraits<VT, Next>::assign;

        template<class T>
        static inline typename std::enable_if<not std::is_same<T, std::string>::value, void>::type
        assign(ValueType t, void *out, const T &value) {
            if (t == VT) {
                *native_cast<VT>(out) = static_cast<native<VT>>(value);
            }
            else {
                Next::assign(t, out, value);
            }
        }

        template<class T>
        static inline typename std::enable_if<std::is_same<T, std::string>::value, void>::type
        assign(ValueType t, void *out, const T &value) {
            Next::assign(t, out, value);
        }

        static inline void sub(AnyValue &o, const AnyValue &value) {
            if (o.m_type == VT) {
                *native_cast<VT>(o.m_value) -= value.cast<native<VT>>();
            }
            else {
                Next::sub(o, value);
            }
        }

        static inline void mul(AnyValue &o, const AnyValue &value) {
            if (o.m_type == VT) {
                *native_cast<VT>(o.m_value) *= value.cast<native<VT>>();
            }
            else {
                Next::mul(o, value);
            }
        }

        static inline void div(AnyValue &o, const AnyValue &value) {
            if (o.m_type == VT) {
                *native_cast<VT>(o.m_value) /= value.cast<native<VT>>();
            }
            else {
                Next::div(o, value);
            }
        }

        static inline size_t isSigned(ValueType t) {
            if (t == VT) {
                return std::numeric_limits<native<VT>>::min() < 0;
            }
            else {
                return Next::isSigned(t);
            }
        }
    };

    template<ValueType VT, class Next = void>
    struct StrTypeTraits: public BaseTypeTraits<VT, Next>
    {
        template<class T>
        static inline typename std::enable_if<std::is_same<T, std::string>::value, T>::type
        cast(ValueType t, void *value, T*p) {
            if (t == VT) {
                return static_cast<T>(*native_cast<VT>(value));
            }
            else {
                return Next::cast(t, value, p);
            }
        }

        template<class T>
        static inline typename std::enable_if<not std::is_same<T, std::string>::value, T>::type
        cast(ValueType t, void *value, T*p) {
            return Next::cast(t, value, p);
        }

        using BaseTypeTraits<VT, Next>::assign;

        template<class T>
        static inline typename std::enable_if<std::is_same<T, std::string>::value, void>::type
        assign(ValueType t, void *out, const T &value) {
            if (t == VT) {
                *native_cast<VT>(out) = value;
            }
            else {
                Next::assign(t, out, value);
            }
        }

        template<class T>
        static inline typename std::enable_if<not std::is_same<T, std::string>::value, void>::type
        assign(ValueType t, void *out, const T &value) {
            Next::assign(t, out, value);
        }
    };

    struct EndTypeTraits {
        static inline void allocCopy(AnyValue &o, void * const value) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void alloc(AnyValue &o) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void release(AnyValue &o) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline std::ostream& print(const AnyValue &o, std::ostream &os) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void assign(ValueType t, void *out, void *in) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline int compare(const AnyValue &o, const AnyValue &value) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void add(AnyValue &o, const AnyValue &value) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void sub(AnyValue &o, const AnyValue &value) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void mul(AnyValue &o, const AnyValue &value) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void div(AnyValue &o, const AnyValue &value) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void jsonGet(ValueType t, void *out, nlohmann::json &js) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void setJson(const AnyValue &o, std::string key, nlohmann::json &js) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline void setJsonFixed(const AnyValue &o, std::string key, nlohmann_fixed::json &js) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline AnyValue fromString(const AnyValue &o, std::string value) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline size_t nativeSize(ValueType t) {
            THROW(UnknownValueTypeNameEx);
        }

        static inline size_t isSigned(ValueType t) {
            THROW(UnknownValueTypeNameEx);
        }

        template<class T>
        static inline T cast(ValueType t, void *value, T*p) {
            THROW(UnknownValueTypeNameEx);
        }

        template<class T>
        static inline void assign(ValueType t, void *out, const T &value) {
            THROW(UnknownValueTypeNameEx);
        }
    };

    typedef NumTypeTraits<ValueType::Char,
            NumTypeTraits<ValueType::Byte,
            NumTypeTraits<ValueType::Short,
            NumTypeTraits<ValueType::Word,
            NumTypeTraits<ValueType::Int,
            NumTypeTraits<ValueType::DWord,
            NumTypeTraits<ValueType::Long,
            NumTypeTraits<ValueType::QWord,
            NumTypeTraits<ValueType::Float,
            NumTypeTraits<ValueType::Double,
            NumTypeTraits<ValueType::Bool,
            StrTypeTraits<ValueType::String,
            EndTypeTraits> > > > > > > > > > > > TypeTraitsList;

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::Byte>>::value ||
        std::is_same<cT, AnyValue::ctype<ValueType::Short>>::value ||
        std::is_same<cT, AnyValue::ctype<ValueType::Word>>::value ||
        std::is_same<cT, AnyValue::ctype<ValueType::Int>>::value ||
        std::is_same<cT, AnyValue::ctype<ValueType::DWord>>::value,
            typename cT::type>::type
    NativeFromString(std::string value) {
        try {
            return (typename cT::type)std::stol(value, 0, 0);
        }
        catch (std::exception &e) {
            THROWEX(CantLoadParameterValueEx, value);
        }
    }

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::Long>>::value,
            typename cT::type>::type
    NativeFromString(std::string value) {
        try {
            return (typename cT::type)std::stoll(value, 0, 0);
        }
        catch (std::exception &e) {
            THROWEX(CantLoadParameterValueEx, value);
        }
    }

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::QWord>>::value,
            typename cT::type>::type
    NativeFromString(std::string value) {
        try {
            return (typename cT::type)std::stoull(value, 0, 0);
        }
        catch (std::exception &e) {
            THROWEX(CantLoadParameterValueEx, value);
        }
    }

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::Float>>::value,
            typename cT::type>::type
    NativeFromString(std::string value) {
        try {
            return (typename cT::type)std::stof(value, 0);
        }
        catch (std::exception &e) {
            THROWEX(CantLoadParameterValueEx, value);
        }
    }

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::Double>>::value,
            double>::type
    NativeFromString(std::string value) {
        try {
            return (typename cT::type)std::stod(value, 0);
        }
        catch (std::exception &e) {
            THROWEX(CantLoadParameterValueEx, value);
        }
    }

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::String>>::value,
            typename cT::type>::type
    NativeFromString(std::string value) {
        return value;
    }

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::Bool>>::value,
            typename cT::type>::type
    NativeFromString(std::string value) {
        if ((value == "true") || (value == "True")) {
            return true;
        }
        else if ((value == "false") || (value == "False")) {
            return false;
        }
        else {
            THROWEX(CantLoadParameterValueEx, value);
        }
    }

    template<class cT>
    static typename std::enable_if<
        std::is_same<cT, AnyValue::ctype<ValueType::Char>>::value ||
        (!std::is_same<cT, AnyValue::ctype<ValueType::String>>::value &&
           std::is_class<typename cT::type>::value),
            typename cT::type>::type
    NativeFromString(std::string value) {
        std::stringstream sstr(value);
        typename cT::ss_type v;
        sstr >> v;
        if (sstr.fail())
            THROWEX(CantLoadParameterValueEx, value);
        return v;
    }

public:
    template<class T, int N=0>
    static constexpr ValueType typeOf();

    AnyValue();
    AnyValue(ValueType vType);
    AnyValue(const AnyValue &o);
    AnyValue(AnyValue &&r);

    template<class T>
    AnyValue(const T &value) :
#ifdef ENABLE_OBJECT_COUNTER
        ObjectCounter(ObjectCounter::ANY_VALUE),
#endif
        m_type(typeOf<T>()), m_value(static_cast<void *>(new native<typeOf<T>()>)) {
        *native_cast<typeOf<T>()>(m_value) = value;
    }

    ~AnyValue();

    bool empty() const;

    ValueType type() const { return m_type; }

    AnyValue &operator=(const AnyValue &value);
    AnyValue &operator=(AnyValue&& r);
    AnyValue &operator=(void *in);

    template<class T>
    AnyValue &operator=(const T &value) {
        if (m_type == ValueType::Empty) {
            m_type = typeOf<T>();
            TypeTraitsList::alloc(*this);
        }
        TypeTraitsList::assign(m_type, m_value, value);
        return *this;
    }

    AnyValue &operator+=(const AnyValue &value);
    AnyValue operator+(const AnyValue &value);

    AnyValue &operator-=(const AnyValue &value);
    AnyValue operator-(const AnyValue &value);

    AnyValue &operator*=(const AnyValue &value);
    AnyValue operator*(const AnyValue &value);

    AnyValue &operator/=(const AnyValue &value);
    AnyValue operator/(const AnyValue &value);


    static void assign(ValueType type, void *out, const AnyValue &value);

    template<class T>
    static void assign(ValueType type, void *out, const T &value) {
        TypeTraitsList::assign(type, out, value);
    }

    void setJson(std::string key, nlohmann::json &js) const;
    void setJsonFixed(std::string key, nlohmann_fixed::json &js) const;

    void jsonGet(nlohmann::json &js);

    bool operator> (const AnyValue &value) const;
    bool operator< (const AnyValue &value) const;
    bool operator>= (const AnyValue &value) const;
    bool operator<= (const AnyValue &value) const;
    bool operator== (const AnyValue &value) const;
    bool operator!= (const AnyValue &value) const;

    template<class T>
    operator T() const {
        return TypeTraitsList::cast<T>(m_type, m_value, nullptr);
    }

    template<class T>
    T cast() const {
        return TypeTraitsList::cast<T>(m_type, m_value, nullptr);
    }

    size_t nativeSize() const;
    static size_t nativeSizeOf(ValueType t);
    static bool isSigned(ValueType t);
    static ValueType typeFromString(std::string type);
    static std::string typeName(ValueType t);
    static AnyValue fromString(ValueType valType, std::string value);

    std::ostream& print(std::ostream& os) const;
};

template<> struct AnyValue::ctype < ValueType::Char >   { typedef int8_t type;                  typedef int ss_type; };
template<> struct AnyValue::ctype < ValueType::Byte >   { typedef uint8_t type;                 typedef unsigned int ss_type; };
template<> struct AnyValue::ctype < ValueType::Short >  { typedef int16_t type;                 typedef int16_t ss_type; };
template<> struct AnyValue::ctype < ValueType::Word >   { typedef uint16_t type;                typedef uint16_t ss_type; };
template<> struct AnyValue::ctype < ValueType::Int >    { typedef int32_t type;                 typedef int32_t ss_type; };
template<> struct AnyValue::ctype < ValueType::DWord >  { typedef uint32_t type;                typedef uint32_t ss_type; };
template<> struct AnyValue::ctype < ValueType::Long >   { typedef int64_t type;                 typedef int64_t ss_type; };
template<> struct AnyValue::ctype < ValueType::QWord >  { typedef uint64_t type;                typedef uint64_t ss_type; };
template<> struct AnyValue::ctype < ValueType::Float >  { typedef float type;                   typedef float ss_type; };
template<> struct AnyValue::ctype < ValueType::Double > { typedef double type;                  typedef double ss_type; };
template<> struct AnyValue::ctype < ValueType::Bool >   { typedef bool type;                    typedef bool ss_type; };
template<> struct AnyValue::ctype < ValueType::String > { typedef std::string type;             typedef std::string ss_type; };

template<> constexpr ValueType AnyValue::typeOf<int8_t>()       { return ValueType::Char; }
template<> constexpr ValueType AnyValue::typeOf<uint8_t>()      { return ValueType::Byte; }
template<> constexpr ValueType AnyValue::typeOf<int16_t>()      { return ValueType::Short; }
template<> constexpr ValueType AnyValue::typeOf<uint16_t>()     { return ValueType::Word; }
template<> constexpr ValueType AnyValue::typeOf<int32_t>()      { return ValueType::Int; }
template<> constexpr ValueType AnyValue::typeOf<uint32_t>()     { return ValueType::DWord; }
template<> constexpr ValueType AnyValue::typeOf<int64_t>()      { return ValueType::Long; }
template<> constexpr ValueType AnyValue::typeOf<uint64_t>()     { return ValueType::QWord; }
template<> constexpr ValueType AnyValue::typeOf<float>()        { return ValueType::Float; }
template<> constexpr ValueType AnyValue::typeOf<double>()       { return ValueType::Double; }
template<> constexpr ValueType AnyValue::typeOf<bool>()         { return ValueType::Bool; }
template<> constexpr ValueType AnyValue::typeOf<std::string>()  { return ValueType::String; }




std::ostream& operator<<(std::ostream& os, const AnyValue& any);

