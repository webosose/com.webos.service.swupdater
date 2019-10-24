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
    StatusType_READY,
    StatusType_RUNNING,
    StatusType_PAUSED,
    StatusType_CANCELED,
    StatusType_COMPLETED,
    StatusType_FAILED,
    // TODO rename
    ST_NONE,
    ST_IDLE,
    ST_DOWNLOAD,
    ST_PAUSED,
    ST_DOWNLOAD_DONE,
    ST_INSTALL,
    ST_INSTALL_DONE,
    ST_FAILED,
};

enum TransitionType {
    TransitionType_Unknown = -2,
    TransitionType_NotAllowed = -1,
    TransitionType_Same = 0,
    TransitionType_Allowed = 1
};

typedef std::function<void(enum StatusType prev, enum StatusType cur)> Callback;

class Status {
public:
    static string toString(enum StatusType type)
    {
        switch (type) {
        case StatusType_NONE:
            return "none";

        case StatusType_READY:
            return "ready";

        case StatusType_RUNNING:
            return "running";

        case StatusType_PAUSED:
            return "paused";

        case StatusType_CANCELED:
            return "canceled";

        case StatusType_COMPLETED:
            return "completed";

        case StatusType_FAILED:
            return "failed";

        // TODO rename
        case ST_NONE:
            return "none";

        case ST_IDLE:
            return "idle";

        case ST_DOWNLOAD:
            return "download";

        case ST_PAUSED:
            return "paused";

        case ST_DOWNLOAD_DONE:
            return "download-done";

        case ST_INSTALL:
            return "install";

        case ST_INSTALL_DONE:
            return "install-done";

        case ST_FAILED:
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

    Status(const string& name)
        : m_name(name)
        , m_status(StatusType_NONE)
        , m_waitingReboot(false)
    {
    }

    virtual ~Status()
    {
        if (m_status == StatusType_PAUSED || m_status == StatusType_RUNNING) {
            cancel();
        }
        m_callbacks.clear();
    }

    enum TransitionType canPrepare()
    {
        return checkTransition(StatusType_READY);
    }

    bool prepare()
    {
        if (checkTransition(StatusType_READY) == TransitionType_NotAllowed)
            return false;
        changeStatus(StatusType_READY);
        return true;
    }

    enum TransitionType canInstall()
    {
        return checkTransition(StatusType_RUNNING);
    }

    bool install()
    {
        if (canInstall() == TransitionType_NotAllowed)
            return false;
        changeStatus(StatusType_RUNNING);
        return true;
    }

    enum TransitionType canPause()
    {
        return checkTransition(StatusType_PAUSED);
    }

    bool pause()
    {
        if (canPause() == TransitionType_NotAllowed)
            return false;
        changeStatus(StatusType_PAUSED);
        return true;
    }

    enum TransitionType canResume()
    {
        return checkTransition(StatusType_RUNNING);
    }

    bool resume()
    {
        if (canResume() == TransitionType_NotAllowed)
            return false;
        changeStatus(StatusType_RUNNING);
        return true;
    }

    enum TransitionType canCancel()
    {
        return checkTransition(StatusType_CANCELED);
    }

    bool cancel()
    {
        if (canCancel() == TransitionType_NotAllowed)
            return false;
        changeStatus(StatusType_CANCELED);
        return true;
    }

    enum TransitionType canComplete()
    {
        return checkTransition(StatusType_COMPLETED);
    }

    bool complete(bool notify = true)
    {
        if (canComplete() == TransitionType_NotAllowed)
            return false;
        changeStatus(StatusType_COMPLETED);
        return true;
    }

    enum TransitionType canFail()
    {
        return checkTransition(StatusType_FAILED);
    }

    bool fail()
    {
        if (canFail() == TransitionType_NotAllowed)
            return false;
        changeStatus(StatusType_FAILED);
        return true;
    }

    enum TransitionType canWaitingReboot()
    {
        if (m_waitingReboot)
            return TransitionType_Same;
        return TransitionType_Allowed;
    }

    bool setWaitingReboot();

    bool isWaitingReboot()
    {
        return m_waitingReboot;
    }

    bool transit(enum StatusType status)
    {
        switch (status) {
        case StatusType_NONE:
            return false;

        case StatusType_READY:
            return prepare();

        case StatusType_RUNNING:
            return install();

        case StatusType_PAUSED:
            return pause();

        case StatusType_CANCELED:
            return cancel();

        case StatusType_COMPLETED:
            return complete();

        case StatusType_FAILED:
            return fail();
        }
        return false;
    }

    void setStatus(enum StatusType status)
    {
        m_status = status;
    }

    enum StatusType getStatus()
    {
        return m_status;
    }

    string getStatusStr()
    {
        return toString(m_status);
    }

    void setName(const string& name)
    {
        m_name = name;
    }

    const string& getName()
    {
        return m_name;
    }

    void addCallback(Callback callback)
    {
        m_callbacks.push_back(std::move(callback));
    }

    void clearCallback()
    {
        m_callbacks.clear();
    }

private:
    enum TransitionType checkTransition(enum StatusType status);
    void changeStatus(enum StatusType nextStatus, bool notify = true);

    string m_name;
    enum StatusType m_status;
    bool m_waitingReboot;

    deque<Callback> m_callbacks;
};

#endif /* CORE_STATUS_H_ */
