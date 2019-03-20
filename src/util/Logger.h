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
    LogLevel_INFO,
    LogLevel_WARNING,
    LogLevel_ERROR,
};

enum LogType {
    LogType_CONSOLE,
    LogType_PMLOG
};

class Logger {
public:
    static void verbose(const string& main, string msg);
    static void verbose(const string& main, const string& sub, string msg);
    static void debug(const string& main, string msg);
    static void debug(const string& main, const string& sub, string msg);
    static void info(const string& main, string msg);
    static void info(const string& main, const string& sub, string msg);
    static void warning(const string& main, string msg);
    static void warning(const string& main, const string& sub, string msg);
    static void error(const string& main, string msg);
    static void error(const string& main, const string& sub, string msg);

    virtual ~Logger();

    void setLevel(enum LogLevel level);
    void setType(enum LogType type);

private:
    static const string EMPTY;

    static const string& toString(const enum LogLevel& level);

    static Logger& getInstance()
    {
        static Logger _instance;
        return _instance;
    }

    Logger();

    void write(const enum LogLevel level, const string& main, const string& sub, const string& msg);
    void writeConsole(const enum LogLevel& level, const string& log);

    enum LogLevel m_level;
    enum LogType m_type;
};

#endif /* UTIL_LOGGER_H_ */
