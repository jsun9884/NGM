#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#ifdef LINUX
#include <unistd.h>
#endif
#include <vector>
#include <cstdlib>

//#ifndef size_t
//#define size_t std::size_t
//#endif

class Buffer
{
public:
    Buffer(size_t capacity);
    Buffer(Buffer &&buf);
    Buffer(const Buffer &) = delete;
    ~Buffer();

    uint8_t *data();
    const uint8_t *data() const;
    size_t size() const;
    size_t capacity() const;

    void set_size(size_t size);

    uint8_t& operator[] (size_t index);
    uint8_t operator[] (size_t index) const;

    void append(uint8_t byte, size_t size = 1);
    void append(const Buffer &buffer);

protected:
    size_t m_capacity;
    size_t m_size;
    uint8_t *m_vec;
};

#endif // BUFFER_H
