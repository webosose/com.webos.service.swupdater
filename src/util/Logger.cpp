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

void Logger::append(string& msg, string log)
{
    msg += "[" + log + "]";
}

void Logger::prepend(string& msg, string log)
{
    msg = "[" + log + "]" + msg;
}

void Logger::verbose(string msg, string name)
{
    getInstance().write(msg, name, LogLevel_VERBOSE);
}

void Logger::debug(string msg, string name)
{
    getInstance().write(msg, name, LogLevel_DEBUG);
}

void Logger::normal(string msg, string name)
{
    getInstance().write(msg, name, LogLevel_NORMAL);
}

void Logger::warning(string msg, string name)
{
    getInstance().write(msg, name, LogLevel_WARNING);
}

void Logger::error(string msg, string name)
{
    getInstance().write(msg, name, LogLevel_ERROR);
}

string& Logger::convertLevel(enum LogLevel& level)
{
    static string verbose = "VERBOSE";
    static string debug = "DEBUG";
    static string normal = "NORMAL";
    static string warning = "WARNING";
    static string error = "ERROR";

    switch(level) {
    case LogLevel_VERBOSE:
        return verbose;

    case LogLevel_DEBUG:
        return debug;

    case LogLevel_NORMAL:
        return normal;

    case LogLevel_WARNING:
        return warning;

    case LogLevel_ERROR:
       return error;
    }
    return debug;
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

void Logger::write(string& msg, string& name, enum LogLevel level)
{
    if (level < m_level)
        return;

    msg = " " + msg;
    Logger::prepend(msg, (name.empty() ? "UNKNOWN" : name));
    Logger::prepend(msg, convertLevel(level));

    switch (m_type) {
    case LogType_CONSOLE:
        writeConsole(msg, level);
        break;

    default:
        cerr << "Unsupported Log Type" << endl;
        break;
    }
}

void Logger::writeConsole(string& log, enum LogLevel& level)
{
    switch(level) {
    case LogLevel_VERBOSE:
    case LogLevel_DEBUG:
    case LogLevel_NORMAL:
        cout << log << endl;
        break;

    case LogLevel_WARNING:
    case LogLevel_ERROR:
        cerr << log << endl;
        break;
    }
}
