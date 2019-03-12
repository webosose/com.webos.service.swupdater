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

#ifndef UTIL_LOGGER_H_
#define UTIL_LOGGER_H_

#include <iostream>

#include "../util/Fifo.h"

using namespace std;

enum LogLevel {
    LogLevel_VERBOSE,
    LogLevel_DEBUG,
    LogLevel_NORMAL,
    LogLevel_WARNING,
    LogLevel_ERROR,
};

enum LogType {
    LogType_CONSOLE,
    LogType_PMLOG
};

class Logger {
public:
    static void append(string& msg, string log);
    static void prepend(string& msg, string log);

    static void verbose(string msg, string name = "");
    static void debug(string msg, string name = "");
    static void normal(string msg, string name = "");
    static void warning(string msg, string name = "");
    static void error(string msg, string name = "");

    static Logger& getInstance()
    {
        static Logger _instance;
        return _instance;
    }

    virtual ~Logger();

    void setLevel(enum LogLevel level);
    void setType(enum LogType type);

private:
    static string& convertLevel(enum LogLevel& level);

    Logger();

    void write(string& msg, string& name, enum LogLevel level = LogLevel_DEBUG);
    void writeConsole(string& log, enum LogLevel& level);

    enum LogLevel m_level;
    enum LogType m_type;
};

#endif /* UTIL_LOGGER_H_ */
