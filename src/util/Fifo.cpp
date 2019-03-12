/* @@@LICENSE
 *
 * Copyright (c) 2019 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#include "../util/Fifo.h"

#include <error.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../util/Logger.h"

#define LOG_NAME "FIFO"

Fifo::Fifo()
    : m_fd(-1)
    , m_readonly(true)
{

}

Fifo::~Fifo()
{
    close();
}

bool Fifo::open(string path, bool readonly)
{
    m_readonly = readonly;
    mkfifo(path.c_str(), 0666);
    m_fd = ::open(path.c_str(), O_RDWR);
    if (m_fd < 0) {
        Logger::error(strerror(errno), LOG_NAME);
        return false;
    }
    return true;
}

bool Fifo::isClose()
{
    return (m_fd <= 0);
}

void Fifo::close()
{
    if (m_fd > 0)
        ::close(m_fd);
    m_fd = -1;
}

int Fifo::send(const void* buffer, int size)
{
    if (m_readonly || m_fd < 0) {
        Logger::error("Invalid write operation", LOG_NAME);
        return -1;
    }
    return ::write(m_fd, buffer, size);
}

int Fifo::receive(void *buffer, int size)
{
    if (!m_readonly || m_fd < 0) {
        Logger::error("Invalid read operation", LOG_NAME);
        return -1;
    }
    return ::read(m_fd, buffer, size);
}
