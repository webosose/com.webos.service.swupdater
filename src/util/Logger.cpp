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

#include "Logger.h"

#include <string.h>

const string Logger::EMPTY = "";

void Logger::verbose(const string& main, string msg)
{
    getInstance().write(LogLevel_VERBOSE, main, EMPTY, msg);
}

void Logger::verbose(const string& main, const string& sub, string msg)
{
    getInstance().write(LogLevel_VERBOSE, main, sub, msg);
}

void Logger::debug(const string& main, string msg)
{
    getInstance().write(LogLevel_DEBUG, main, EMPTY, msg);
}

void Logger::debug(const string& main, const string& sub, string msg)
{
    getInstance().write(LogLevel_DEBUG, main, sub, msg);
}

void Logger::info(const string& main, string msg)
{
    getInstance().write(LogLevel_INFO, main, EMPTY, msg);
}

void Logger::info(const string& main, const string& sub, string msg)
{
    getInstance().write(LogLevel_INFO, main, sub, msg);
}

void Logger::warning(const string& main, string msg)
{
    getInstance().write(LogLevel_WARNING, main, EMPTY, msg);
}

void Logger::warning(const string& main, const string& sub, string msg)
{
    getInstance().write(LogLevel_WARNING, main, sub, msg);
}

void Logger::error(const string& main, string msg)
{
    getInstance().write(LogLevel_ERROR, main, EMPTY, msg);
}

void Logger::error(const string& main, const string& sub, string msg)
{
    getInstance().write(LogLevel_ERROR, main, sub, msg);
}

const string& Logger::toString(const enum LogLevel& level)
{
    static const string VERBOSE = "[V]";
    static const string DEBUG = "[D]";
    static const string INFO = "[I]";
    static const string WARNING = "[W]";
    static const string ERROR = "[E]";

    switch(level) {
    case LogLevel_VERBOSE:
        return VERBOSE;

    case LogLevel_DEBUG:
        return DEBUG;

    case LogLevel_INFO:
        return INFO;

    case LogLevel_WARNING:
        return WARNING;

    case LogLevel_ERROR:
       return ERROR;
    }
    return DEBUG;
}

Logger::Logger()
    : m_level(LogLevel_VERBOSE)
    , m_type(LogType_CONSOLE)
{
}

Logger::~Logger()
{
}

void Logger::setLevel(enum LogLevel level)
{
    m_level = level;
}

void Logger::setType(enum LogType type)
{
    m_type = type;
}

void Logger::write(const enum LogLevel level, const string& main, const string& sub, const string& msg)
{
    if (level < m_level)
        return;

    string log = toString(level);
    if (!main.empty()) {
        log += "[" + main + "]";
    }
    if (!sub.empty()) {
        log += "[" + sub + "]";
    }
    log += " " + msg;

    switch (m_type) {
    case LogType_CONSOLE:
        writeConsole(level, log);
        break;

    default:
        cerr << "Unsupported Log Type" << endl;
        break;
    }
}

void Logger::writeConsole(const enum LogLevel& level, const string& log)
{
    switch(level) {
    case LogLevel_VERBOSE:
    case LogLevel_DEBUG:
    case LogLevel_INFO:
        cout << log << endl;
        break;

    case LogLevel_WARNING:
    case LogLevel_ERROR:
        cerr << log << endl;
        break;
    }
}
