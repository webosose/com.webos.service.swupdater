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

#ifndef CORE_STATUS_H_
#define CORE_STATUS_H_

#include <iostream>
#include <deque>
#include <functional>

#include "util/Logger.h"

using namespace std;

enum StatusType {
    StatusType_UNKNOWN,
    StatusType_IDLE,
    StatusType_DOWNLOAD_READY,
    StatusType_DOWNLOAD_STARTED,
    StatusType_DOWNLOAD_PAUSED,
    StatusType_INSTALL_READY,
    StatusType_INSTALL_STARTED,
    StatusType_INSTALL_COMPLETED,
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
        case StatusType_IDLE:
            return "idle";

        case StatusType_DOWNLOAD_READY:
            return "downloadReady";

        case StatusType_DOWNLOAD_STARTED:
            return "downloadStarted";

        case StatusType_DOWNLOAD_PAUSED:
            return "downloadPaused";

        case StatusType_INSTALL_READY:
            return "installReady";

        case StatusType_INSTALL_STARTED:
            return "installStarted";

        case StatusType_INSTALL_COMPLETED:
            return "installCompleted";

        case StatusType_FAILED:
            return "failed";

        case StatusType_UNKNOWN:
            return "unknown";
        }
        return "unknown";
    }

    static enum StatusType toStatusType(const string& statusStr)
    {
        if (statusStr == toString(StatusType_IDLE))
            return StatusType_IDLE;
        if (statusStr == toString(StatusType_DOWNLOAD_READY))
            return StatusType_DOWNLOAD_READY;
        if (statusStr == toString(StatusType_DOWNLOAD_STARTED))
            return StatusType_DOWNLOAD_STARTED;
        if (statusStr == toString(StatusType_DOWNLOAD_PAUSED))
            return StatusType_DOWNLOAD_PAUSED;
        if (statusStr == toString(StatusType_INSTALL_READY))
            return StatusType_INSTALL_READY;
        if (statusStr == toString(StatusType_INSTALL_STARTED))
            return StatusType_INSTALL_STARTED;
        if (statusStr == toString(StatusType_INSTALL_COMPLETED))
            return StatusType_INSTALL_COMPLETED;
        if (statusStr == toString(StatusType_FAILED))
            return StatusType_FAILED;
        if (statusStr == toString(StatusType_IDLE))
            return StatusType_IDLE;
        return StatusType_UNKNOWN;
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
        : m_status(StatusType_IDLE)
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
        if (statusStr == toString(StatusType_DOWNLOAD_READY))
            m_status = StatusType_DOWNLOAD_READY;
        else if (statusStr == toString(StatusType_DOWNLOAD_STARTED))
            m_status = StatusType_DOWNLOAD_STARTED;
        else if (statusStr == toString(StatusType_DOWNLOAD_PAUSED))
            m_status = StatusType_DOWNLOAD_PAUSED;
        else if (statusStr == toString(StatusType_INSTALL_READY))
            m_status = StatusType_DOWNLOAD_STARTED;
        else if (statusStr == toString(StatusType_INSTALL_STARTED))
            m_status = StatusType_INSTALL_STARTED;
        else if (statusStr == toString(StatusType_INSTALL_COMPLETED))
            m_status = StatusType_INSTALL_COMPLETED;
        else if (statusStr == toString(StatusType_FAILED))
            m_status = StatusType_FAILED;
        else if (statusStr == toString(StatusType_IDLE))
            m_status = StatusType_IDLE;
        else
            m_status = StatusType_UNKNOWN;
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
