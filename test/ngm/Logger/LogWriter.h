#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <string>
#include <memory>
#include <mutex>
#include <fstream>
#include <stdio.h>

class LogWriter
{
public:
    virtual bool open() = 0;
    virtual bool write(std::string record) = 0;
    virtual void close() = 0;

protected:
    std::mutex m_mutex;
};

typedef std::shared_ptr<LogWriter> LogWriterPtr;

class OStreamLogWriter: public LogWriter
{
public:
    OStreamLogWriter(std::ostream *os);

    virtual bool open();
    virtual bool write(std::string record);
    virtual void close();

private:
    std::ostream *m_os;
};

class FileLogWriter: public LogWriter
{
public:
    FileLogWriter(std::string filename, int num_files, long max_size, bool append, mode_t mode = 00644);
    FileLogWriter(int fd);

    virtual bool open();
    virtual bool write(std::string record);
    virtual void close();

private:
    void rotateFiles();

private:
    int m_fd;
    std::string m_filename;
    int m_num_files;
    long m_max_size;
    int m_flags;
    mode_t m_mode;
};

#endif // LOGWRITER_H
