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

#ifndef INTERFACE_IINSTALLER_H_
#define INTERFACE_IINSTALLER_H_

#include <iostream>
#include <vector>

#include "util/Logger.h"
#include "IListener.h"

using namespace std;

enum InstallerState {
    InstallerState_NONE,
    InstallerState_READY,
    InstallerState_PAUSED,
    InstallerState_RUNNING,
    InstallerState_COMPLETED,
    InstallerState_FAILED,
};

class IInstaller : public IListener<IInstaller> {
public:
    static string toString(enum InstallerState type)
    {
        switch (type) {
        case InstallerState_NONE:
            return "None";

        case InstallerState_READY:
            return "Ready";

        case InstallerState_PAUSED:
            return "Paused";

        case InstallerState_RUNNING:
            return "Running";

        case InstallerState_COMPLETED:
            return "Completed";

        case InstallerState_FAILED:
            return "Failed";
        }
        return "Unknown";
    }

    IInstaller()
        : m_name("Installable")
        , m_state(InstallerState_NONE)
    {
    }

    virtual ~IInstaller()
    {
        if (m_state == InstallerState_PAUSED || m_state == InstallerState_RUNNING)
            cancel();
    }

    virtual bool ready()
    {
        switch(m_state) {
        case InstallerState_PAUSED:
        case InstallerState_COMPLETED:
        case InstallerState_FAILED:
        case InstallerState_RUNNING:
            return false;

        case InstallerState_READY:
            return true;

        case InstallerState_NONE:
            break;
        }
        changeStatus(InstallerState_READY);
        return true;
    }

    virtual bool start()
    {
        switch(m_state) {
        case InstallerState_NONE:
        case InstallerState_PAUSED:
        case InstallerState_COMPLETED:
        case InstallerState_FAILED:
            return false;

        case InstallerState_RUNNING:
            return true;

        case InstallerState_READY:
            break;
        }

        changeStatus(InstallerState_RUNNING);
        return true;
    }

    virtual bool pause()
    {
        switch(m_state) {
        case InstallerState_NONE:
        case InstallerState_READY:
        case InstallerState_COMPLETED:
        case InstallerState_FAILED:
            return false;

        case InstallerState_PAUSED:
            return true;

        case InstallerState_RUNNING:
            break;
        }

        changeStatus(InstallerState_PAUSED);
        return true;
    }

    virtual bool resume()
    {
        switch(m_state) {
        case InstallerState_NONE:
        case InstallerState_READY:
        case InstallerState_COMPLETED:
        case InstallerState_FAILED:
            return false;

        case InstallerState_RUNNING:
            return true;

        case InstallerState_PAUSED:
            break;
        }

        changeStatus(InstallerState_RUNNING);
        return true;
    }

    virtual bool cancel()
    {
        switch(m_state) {
        case InstallerState_NONE:
        case InstallerState_READY:
        case InstallerState_COMPLETED:
        case InstallerState_FAILED:
            return false;

        case InstallerState_PAUSED:
        case InstallerState_RUNNING:
            break;
        }

        changeStatus(InstallerState_READY);
        return true;
    }

    enum InstallerState getState()
    {
        return m_state;
    }

protected:
    virtual bool complete()
    {
        switch(m_state) {
        case InstallerState_NONE:
        case InstallerState_READY:
        case InstallerState_FAILED:
        case InstallerState_PAUSED:
            return false;

        case InstallerState_COMPLETED:
            return true;

        case InstallerState_RUNNING:
            break;
        }

        changeStatus(InstallerState_COMPLETED);
        return true;
    }

    virtual void fail()
    {
        switch(m_state) {
        case InstallerState_NONE:
        case InstallerState_READY:
        case InstallerState_PAUSED:
        case InstallerState_COMPLETED:
        case InstallerState_FAILED:
            return;

        case InstallerState_RUNNING:
            break;
        }
        changeStatus(InstallerState_FAILED);
    }

    const string& getName()
    {
        return m_name;
    }

    void setName(const string& name)
    {
        m_name = name;
    }

    virtual void onStateChange(IInstaller *installer, enum InstallerState prev, enum InstallerState cur)
    {
        // Nothing
    }

    void changeStatus(enum InstallerState nextStatus)
    {
        if (m_state == nextStatus)
            return;

        enum InstallerState prev = m_state;
        m_state = nextStatus;

        Logger::info(m_name, toString(prev) + " ==> " + toString(m_state));

        if (m_listener)
            m_listener->onStateChange(this, prev, m_state);
    }

private:
    string m_name;

    enum InstallerState m_state;
};

#endif /* INTERFACE_IINSTALLER_H_ */
