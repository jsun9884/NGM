#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>

#include "LogWriter.h"


OStreamLogWriter::OStreamLogWriter(std::ostream *os):
    m_os(os)
{
}

bool OStreamLogWriter::open()
{
    return true;
}

bool OStreamLogWriter::write(std::string record)
{
    std::lock_guard<std::mutex> l(m_mutex);
    (*m_os) << record;
    return true;
}

void OStreamLogWriter::close()
{
}


FileLogWriter::FileLogWriter(std::string filename, int num_files, long max_size, bool append, mode_t mode):
    m_fd(-1),
    m_filename(filename),
    m_num_files((num_files > 1 ? num_files : 2)),
    m_max_size(max_size),
    m_flags(O_CREAT | O_APPEND | O_WRONLY),
    m_mode(mode)
{
    if (!append) {
        m_flags |= O_TRUNC;
    }
}

FileLogWriter::FileLogWriter(int fd):
    m_fd(fd),
    m_num_files(0),
    m_max_size(0),
    m_flags(O_CREAT | O_APPEND | O_WRONLY),
    m_mode(00644)
{
}

bool FileLogWriter::open()
{
    if (m_fd == -1) {
        if (m_max_size)
            rotateFiles();
        m_fd = ::open((m_filename + ".0").c_str(), m_flags, m_mode);
    }
    return m_fd != -1;
}

bool FileLogWriter::write(std::string record)
{
    std::lock_guard<std::mutex> l(m_mutex);
    if (m_max_size) {
        long sz = ::lseek(m_fd, 0, SEEK_CUR);
        if (sz > m_max_size) {
            close();
            open();
        }
    }
    if (::write(m_fd, record.c_str(), record.length()) == -1) {
        return false;
    }
    return true;
}

void FileLogWriter::close()
{
    if (m_fd != -1) {
        ::close(m_fd);
        m_fd = -1;
    }
}

void FileLogWriter::rotateFiles()
{
    std::string n1, n2;

    {
        std::stringstream ss;
        ss << m_filename << "." << m_num_files - 1;
        n2 = ss.str();
        ::remove(n2.c_str());
    }

    for(int i = m_num_files - 1; i > 0; --i) {
        std::stringstream ss;
        ss << m_filename << "." << i - 1;
        n1 = ss.str();
        ::rename(n1.c_str(), n2.c_str());
        n2 = n1;
    }
}

