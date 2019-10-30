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

#ifndef CORE_STATUS_H_
#define CORE_STATUS_H_

#include <iostream>
#include <deque>
#include <functional>

#include "util/Logger.h"

using namespace std;

enum StatusType {
    StatusType_NONE,
    StatusType_IDLE,
    StatusType_DOWNLOAD,
    StatusType_PAUSED,
    StatusType_DOWNLOAD_DONE,
    StatusType_INSTALL,
    StatusType_INSTALL_DONE,
    StatusType_FAILED,
};

enum TransitionType {
    TransitionType_Unknown = -2,
    TransitionType_NotAllowed = -1,
    TransitionType_Same = 0,
    TransitionType_Allowed = 1
};

class Status {
public:
    static string toString(enum StatusType type)
    {
        switch (type) {
        case StatusType_NONE:
            return "none";

        case StatusType_IDLE:
            return "idle";

        case StatusType_DOWNLOAD:
            return "download";

        case StatusType_PAUSED:
            return "paused";

        case StatusType_DOWNLOAD_DONE:
            return "download-done";

        case StatusType_INSTALL:
            return "install";

        case StatusType_INSTALL_DONE:
            return "install-done";

        case StatusType_FAILED:
            return "failed";
        }
        return "unknown";
    }

    static bool writeCommonLog(Status& status, enum TransitionType type, const string& className, const string& transition)
    {
        if (type == TransitionType_NotAllowed) {
            Logger::info(className, transition + " is not allowed from " + status.getStatusStr());
            return false;
        } else if (type == TransitionType_Same) {
            Logger::info(className, transition + " is already called");
            return true;
        }
        return true;
    }

    Status()
        : m_status(StatusType_NONE)
    {
    }

    virtual ~Status()
    {
    }

    void setStatus(enum StatusType status)
    {
        m_status = status;
    }

    void setStatus(const string& statusStr)
    {
        if (statusStr == toString(StatusType_IDLE))
            m_status = StatusType_IDLE;
        else if (statusStr == toString(StatusType_DOWNLOAD))
            m_status = StatusType_DOWNLOAD;
        else if (statusStr == toString(StatusType_PAUSED))
            m_status = StatusType_PAUSED;
        else if (statusStr == toString(StatusType_DOWNLOAD_DONE))
            m_status = StatusType_DOWNLOAD;
        else if (statusStr == toString(StatusType_INSTALL))
            m_status = StatusType_INSTALL;
        else if (statusStr == toString(StatusType_INSTALL_DONE))
            m_status = StatusType_INSTALL_DONE;
        else if (statusStr == toString(StatusType_FAILED))
            m_status = StatusType_FAILED;
        else
            m_status = StatusType_NONE;
    }

    enum StatusType getStatus()
    {
        return m_status;
    }

    string getStatusStr()
    {
        return toString(m_status);
    }

private:
    enum StatusType m_status;
};

#endif /* CORE_STATUS_H_ */
