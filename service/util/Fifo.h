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

#ifndef UTIL_FIFO_H_
#define UTIL_FIFO_H_

#include <iostream>

using namespace std;

class Fifo {
public:
    Fifo();
    virtual ~Fifo();

    bool open(string path, bool readonly = true);
    bool isClose();
    void close();

    int send(const void* buffer, int size);
    int receive(void *buffer, int size);

private:
    int m_fd;
    bool m_readonly;
};

#endif /* UTIL_FIFO_H_ */
