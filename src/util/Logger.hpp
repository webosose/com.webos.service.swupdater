// Copyright (c) 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include "Environment.h"

#ifdef INCLUDE_WEBOS
#include <PmLogLib.h>
extern PmLogContext cmLogContext;
#endif

#define LOG_INFO_PAIRS(...)                     PmLogInfo(cmLogContext, ##__VA_ARGS__)
#define LOG_MESSAGE_PAIRS(...)                  PmLogMsg(cmLogContext, ##__VA_ARGS__)
#define LOG_DEBUG(...)                          PmLogDebug(cmLogContext, ##__VA_ARGS__)
#define LOG_WARNING_PAIRS(...)                  PmLogWarning(cmLogContext, ##__VA_ARGS__)
#define LOG_ERROR_PAIRS(...)                    PmLogError(cmLogContext, ##__VA_ARGS__)
#define LOG_CRITICAL_PAIRS(...)                 PmLogCritical(cmLogContext, ##__VA_ARGS__)

/* Define MSGID's */
#define MSGID_MAIN              "MAIN"
#define MSGID_MANAGER           "MANAGER"
#define MSGID_BASE              "BASE"
#define MSGID_DEVICE_STATE      "DEVICE_STATE"
#define MSGID_HTTP              "HTTP"
#define MSGID_KEY               "KEY"
#define MSGID_KEY_MANAGER       "KEY_MANAGER"
#define MSGID_MQTT_CONNECT      "MQTT_CONNECT"
#define MSGID_MQTT_SUBSCRIBE    "MQTT_SUBSCRIBE"
#define MSGID_MQTT_UPDATE       "MQTT_UPDATE"
#define MSGID_MQTT_YIELD        "MQTT_YIELD"
#define MSGID_PLATFORM          "PLATFORM"
#define MSGID_PLATFORM_WEBOS    "PLATFORM_WEBOS"
#define MSGID_PLATFORM_LGRP     "PLATFORM_LGRP"
#define MSGID_SERVICE           "SERVICE"
#define MSGID_SERVICE_WEBOS     "SERVICE_WEBOS"
#define MSGID_SERVICE_FOTA      "SERVICE_FOTA"
#define MSGID_UTIL              "UTIL"
#define MSGID_SETTING           "SETTING"
#define MSGID_CURL_MANAGER      "CURL_MANAGER"
#define MSGID_HTTP_REQUEST      "HTTP_REQUEST"
#define MSGID_HTTP_RESPONSE     "HTTP_RESPONSE"

using namespace std;

typedef enum {
    LogType_PmLog = 0,
    LogType_Console,
    LogType_File,
    LogType_Memory
} LogType;

typedef enum {
    LogLevel_Error = 0,
    LogLevel_Warning,
    LogLevel_Info,
    LogLevel_Debug,
    LogLevel_Verbose
} LogLevel;

static const char* LOGTYPETEXT_PMLOG = "pmlog";
static const char* LOGTYPETEXT_CONSOLE = "console";
static const char* LOGTYPETEXT_FILE = "file";
static const char* LOGTYPETEXT_MEMORY = "memory";

static const char* LOGLEVELTEXT_ERROR = "error";
static const char* LOGLEVELTEXT_WARNING = "warning";
static const char* LOGLEVELTEXT_INFO = "info";
static const char* LOGLEVELTEXT_DEBUG = "debug";
static const char* LOGLEVELTEXT_VERBOSE = "verbose";


class Logger {
public:
    static Logger& getInstance()
    {
        static Logger s_logger;
        return s_logger;
    }

    virtual ~Logger() {};

    void clear()
    {
        m_type = LogType_Console;
        m_level = LogLevel_Debug;
        m_logFilePath = "";

        m_strStream.str("");
        m_strStream.clear();

        m_fileStream.flush();
        m_fileStream.clear();
        m_fileStream.close();
    }

    LogType getLogType()
    {
        return m_type;
    }

    LogType getLogType(const char* typeText)
    {
        if (!strcmp(typeText, LOGTYPETEXT_PMLOG)) return LogType_PmLog;
        if (!strcmp(typeText, LOGTYPETEXT_CONSOLE)) return LogType_Console;
        if (!strcmp(typeText, LOGTYPETEXT_FILE)) return LogType_File;
        if (!strcmp(typeText, LOGTYPETEXT_MEMORY)) return LogType_Memory;
        return m_type;
    }

    const char* getLogTypeText(LogType type)
    {
        switch (type) {
        case LogType_PmLog:
            return LOGTYPETEXT_PMLOG;
        case LogType_Console:
            return LOGTYPETEXT_CONSOLE;
        case LogType_File:
            return LOGTYPETEXT_FILE;
        case LogType_Memory:
            return LOGTYPETEXT_MEMORY;
        }
        return nullptr;
    }

    bool setLogType(const char* typeText, string path = "")
    {
        return setLogType(getLogType(typeText), path);
    }

    bool setLogType(LogType type, string path = "")
    {
        Logger::info(MSGID_UTIL, "Change Log Type : type(%d), path(%s)", type, path.c_str());
        if (type == m_type) {
            return true;
        }

        if (type == LogType_File) {
            if (path.empty() || !setLogFilePath(path)) {
                return false;
            }
        }
        m_type = type;
        return true;
    }

    void setLogLevel(LogLevel lev)
    {
        m_level = lev;
    }

    void setLogLevel(const char* levName)
    {
        setLogLevel(getLogLevel(levName));
    }

    LogLevel getLogLevel()
    {
        return m_level;
    }

    LogLevel getLogLevel(const char* levName)
    {
        if (!strcmp(levName, LOGLEVELTEXT_ERROR)) return LogLevel_Error;
        if (!strcmp(levName, LOGLEVELTEXT_WARNING)) return LogLevel_Warning;
        if (!strcmp(levName, LOGLEVELTEXT_INFO)) return LogLevel_Info;
        if (!strcmp(levName, LOGLEVELTEXT_DEBUG)) return LogLevel_Debug;
        if (!strcmp(levName, LOGLEVELTEXT_VERBOSE)) return LogLevel_Verbose;
        return m_level;
    }

    const char* getLogLevelText(LogLevel lev)
    {
        switch (lev) {
        case LogLevel_Error:
            return LOGLEVELTEXT_ERROR;
        case LogLevel_Warning:
            return LOGLEVELTEXT_WARNING;
        case LogLevel_Info:
            return LOGLEVELTEXT_INFO;
        case LogLevel_Debug:
            return LOGLEVELTEXT_DEBUG;
        case LogLevel_Verbose:
            return LOGLEVELTEXT_VERBOSE;
        }
        return nullptr;
    }

    string getLogFilePath()
    {
        return m_logFilePath;
    }

    bool isExistLog(const char* level, const char* targetStr)
    {
        if (m_type == LogType_File) {
            return findFromFile(level, targetStr);
        } else if (m_type == LogType_Memory) {
            return findFromMemory(level, targetStr);
        }
        return false;
    }

    bool isEmpty()
    {
        if (LogType_Memory == m_type && m_strStream.tellp() == 0) {
            return true;
        }

        if (LogType_File == m_type && m_fileStream.tellp() == 0) {
            return true;
        }
        return false;
    }

    string getLogFromMemory()
    {
        return m_strStream.str();
    }

    template<typename... Ts>
    static bool verbose(const char* format, Ts ... args)
    {
        LogType type = Logger::getInstance().getLogType();
        if (!(LogType_Console == type || LogType_File == type)) {
            return false;
        }
        return (Logger::getInstance().checkEnabledLogLevel(LogLevel_Verbose)) ?
                Logger::getInstance().write("verbose", "", format, args...) : false;
    }

    template<typename... Ts>
    static bool debug(const char* format, Ts... args)
    {
        return (Logger::getInstance().checkEnabledLogLevel(LogLevel_Debug)) ?
                Logger::getInstance().write("debug", "", format, args...) : false;
    }

    template<typename... Ts>
    static bool info(const char* msgid, const char* format, Ts... args)
    {
        return (Logger::getInstance().checkEnabledLogLevel(LogLevel_Info)) ?
                Logger::getInstance().write("info", msgid, format, args...) : false;
    }

    template<typename ... Ts>
    static bool warning(const char* msgid, const char* format, Ts ... args)
    {
        return (Logger::getInstance().checkEnabledLogLevel(LogLevel_Warning)) ?
                Logger::getInstance().write("warning", msgid, format, args...) : false;
    }

    template<typename ... Ts>
    static bool error(const char* msgid, const char* format, Ts ... args)
    {
        return (Logger::getInstance().checkEnabledLogLevel(LogLevel_Error)) ?
                Logger::getInstance().write("error", msgid, format, args...) : false;
    }

private:
    bool checkEnabledLogLevel(LogLevel level)
    {
        return (m_level >= level);
    }

    template<typename ... Ts>
    bool write(const char* level, const char* msgid, const char* format, Ts ... args)
    {
        if (m_type == LogType_Console) {
            return writeConsole(level, msgid, format, args...);
        }

        if (m_type == LogType_Memory) {
            return writeMemory(level, msgid, format, args...);
        }

        if (m_type == LogType_File) {
            return writeFile(level, msgid, format, args...);
        }

        if (m_type == LogType_PmLog) {

#ifdef INCLUDE_WEBOS
            if (strcmp(level, "debug") == 0) {
                PmLogMsg(cmLogContext, Debug, NULL, 0, format, args...);
            } else if (strcmp(level, "info") == 0) {
                PmLogMsg(cmLogContext, Info, msgid, 0, format, args...);
            } else if (strcmp(level, "warning") == 0) {
                PmLogMsg(cmLogContext, Warning, msgid, 0, format, args...);
            } else if (strcmp(level, "error") == 0) {
                PmLogMsg(cmLogContext, Error, msgid, 0, format, args...);
            }
#endif
            return true;
        }
        return false;
    }

    template<typename... Ts>
    bool writeConsole(const char* logLevel, const char* msgid, const char* format, Ts... args)
    {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);

        printf("[%5ld.%09ld] [%-7s] %-15s ", time.tv_sec, time.tv_nsec, logLevel, msgid);
        printf(format, args...);
        printf("\n");
        return true;
    }

    template<typename... Ts>
    bool writeMemory(const char* logLevel, const char* msgid, const char* format, Ts... args)
    {
        if (sizeof...(args) == 0) {
            m_strStream << "[" << logLevel << "] " << msgid << " " << format << endl;
        } else {
            int cnt = 0;
            cnt = snprintf(m_buf, 1024, "[%s] %s ", logLevel, msgid);
            cnt += snprintf(m_buf + strlen(m_buf), 1024 - strlen(m_buf), format, args...);

            if (cnt < 0 || cnt > 1024) {
                return false;
            }

            m_strStream << m_buf << endl;
        }
        return true;
    }

    template<typename... Ts>
    bool writeFile(const char* logLevel, const char* msgid, const char* format, Ts... args)
    {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        int cnt = 0;

        if (m_fileStream.fail() || !m_fileStream.is_open() || m_logFilePath == "") {
            return false;
        }
        if (sizeof...(args) == 0) {
            m_fileStream << "[" << logLevel << "] " << msgid << " "<< format << endl;
        } else {
            cnt = snprintf(m_buf, 1024, "[%5ld.%09ld] [%-7s] %-15s ", time.tv_sec, time.tv_nsec, logLevel, msgid);
            cnt += snprintf(m_buf + strlen(m_buf), 1024 - strlen(m_buf), format, args...);
            if (cnt < 0 || cnt > 1024) {
                return false;
            }
            m_fileStream << m_buf << endl;
        }
        return true;
    }

    bool findFromFile(const char* level, const char* targetStr)
    {
        ifstream fin(m_logFilePath);
        if (!fin) {
            return false;
        }

        fin.seekg(std::ios::beg);

        string str(1024, '\0');
        while (!fin.fail() || !fin.eof()) {
            fin.getline(&str[0], 1024, '\n');
            if (str.find(targetStr) != string::npos && str.find(level) != string::npos) {
                return true;
            }
        }
        return false;
    }

    bool findFromMemory(const char* level, const char* targetStr)
    {
        m_strStream.seekg(ios_base::beg);
        for (string line ; getline(m_strStream, line, '\n') ; ) {
            if (line.find(targetStr) != string::npos && line.find(level) != string::npos) {
                return true;
            }
        }
        m_strStream.clear();
        return false;
    }

    bool setLogFilePath(string path)
    {
        if (path.empty()) {
            return false;
        }

        if (path == m_logFilePath) {
            return true;
        }

        if (!m_fileStream.fail() && m_fileStream.is_open()) {
            m_fileStream.flush();
            m_fileStream.clear();
            m_fileStream.close();
        }

        m_fileStream.open(path, ios_base::out | ios::app);

        if (!m_fileStream.fail() || m_fileStream.is_open()) {
            m_logFilePath = path;
            return true;
        }
        return false;
    }

    Logger()
        : m_type(LogType_Console),
          m_level(LogLevel_Verbose),
          m_logFilePath("")
    {
#ifdef INCLUDE_WEBOS
        PmLogGetContext("controlmanager", &cmLogContext);
#endif
    }

    LogType m_type;
    LogLevel m_level;
    stringstream m_strStream;
    ofstream m_fileStream;
    string m_logFilePath;

    char m_buf[1024];
};

#endif /* _LOGGER_H_ */
