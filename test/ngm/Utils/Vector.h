#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

template<class T>
class Vector
{
public:
    Vector(): m_x(0), m_y(0), m_z(0) {}
    Vector(const Vector &o): m_x(o.m_x), m_y(o.m_y), m_z(o.m_z) {}
    Vector(T x, T y, T z): m_x(x), m_y(y), m_z(z) {}

    T x() const { return m_x; }
    T y() const { return m_y; }
    T z() const { return m_z; }

    Vector& operator+=(const Vector &o) {
        m_x += o.m_x;
        m_y += o.m_y;
        m_z += o.m_z;
        return *this;
    }

    Vector operator+(const Vector &o) const {
        Vector r(*this);
        r += o;
        return r;
    }

    Vector& operator-=(const Vector &o) {
        m_x -= o.m_x;
        m_y -= o.m_y;
        m_z -= o.m_z;
        return *this;
    }

    Vector operator-(const Vector &o) const {
        Vector r(*this);
        r -= o;
        return r;
    }

    Vector& operator*=(const T s) {
        m_x *= s;
        m_y *= s;
        m_z *= s;
        return *this;
    }

    Vector operator*(const T s) const {
        Vector r(*this);
        r *= s;
        return r;
    }

    Vector& operator/=(const T s) {
        m_x /= s;
        m_y /= s;
        m_z /= s;
        return *this;
    }

    Vector operator/(const T s) const {
        Vector r(*this);
        r /= s;
        return r;
    }

    T scalarProduct(const Vector &o) const {
        return m_x * o.m_x + m_y * o.m_y + m_z * o.m_z;
    }

    Vector crossProduct(const Vector &o) const {
        Vector r;
        r.m_x = m_y * o.m_z - m_z * o.m_y;
        r.m_y = m_z * o.m_x - m_x * o.m_z;
        r.m_z = m_x * o.m_y - m_y * o.m_x;
        return r;
    }

    T length() const {
        return sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
    }

    T lenghtSquared() const {
        return m_x * m_x + m_y * m_y + m_z * m_z;
    }

    Vector normalize() const {
        return *this / length();
    }

    Vector eulerAngles() const {
        Vector r;
        r.m_x = atan2(m_y, m_z);
        r.m_y = atan2(m_x, m_z);
        r.m_z = atan2(m_y, m_x);
        return r;
    }

    bool isZero(T epsilon) const {
        return fabs(m_x) <= epsilon && fabs(m_y) <= epsilon && fabs(m_z) <= epsilon;
    }

public:
    T m_x;
    T m_y;
    T m_z;
};

#endif // VECTOR_H
