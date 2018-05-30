#include "buffer.h"

Buffer::Buffer(size_t capacity):
	m_capacity(capacity),
	m_size(0),
    m_vec(new uint8_t[capacity])
{
}

Buffer::Buffer(Buffer &&buf):
	m_capacity(0),
	m_size(0),
    m_vec(nullptr)
{
	std::swap(m_capacity, buf.m_capacity);
	std::swap(m_size, buf.m_size);
	std::swap(m_vec, buf.m_vec);
}

Buffer::~Buffer()
{
	if (m_vec)
		delete [] m_vec;
}

uint8_t *Buffer::data()
{
    return m_vec;
}

const uint8_t *Buffer::data() const
{
    return m_vec;
}

size_t Buffer::size() const
{
    return m_size;
}

size_t Buffer::capacity() const
{
	return m_capacity;
}

uint8_t &Buffer::operator[](size_t index)
{
    return m_vec[index];
}

uint8_t Buffer::operator[](size_t index) const
{
    return m_vec[index];
}

void Buffer::append(uint8_t byte, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        m_vec[m_size++] = byte;
    }
}

void Buffer::set_size(size_t size)
{
	m_size = size;
}

void Buffer::append(const Buffer &buffer)
{
    for (size_t i = 0; i < buffer.size(); ++i) {
        m_vec[m_size++] = buffer[i];
    }
}
