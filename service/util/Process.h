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

#ifndef UTIL_PROCESS_H_
#define UTIL_PROCESS_H_

#include <iostream>
#include <fstream>

using namespace std;

class Process {
public:
    Process();
    virtual ~Process();

    bool isRunning();
    void stop();
    void setArgs(string args);
    void execute();

};

#endif /* UTIL_PROCESS_H_ */
