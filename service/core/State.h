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

#ifndef CORE_STATE_H_
#define CORE_STATE_H_

#include <iostream>
#include <functional>
#include <vector>

#include "util/Logger.h"
#include "interface/IListener.h"

using namespace std;

enum StateType {
    StateType_NONE,
    StateType_READY,
    StateType_PAUSED,
    StateType_RUNNING,
    StateType_COMPLETED,
    StateType_FAILED,
};

class State;

typedef std::function<void(State* state, enum StateType prev, enum StateType cur)> Callback;

class State {
public:
    static string toString(enum StateType type)
    {
        switch (type) {
        case StateType_NONE:
            return "none";

        case StateType_READY:
            return "ready";

        case StateType_PAUSED:
            return "paused";

        case StateType_RUNNING:
            return "running";

        case StateType_COMPLETED:
            return "completed";

        case StateType_FAILED:
            return "failed";
        }
        return "unknown";
    }

    static bool transition(State& state, enum StateType type) {
        switch (type) {
        case StateType_NONE:
        case StateType_READY:
            return false;

        case StateType_PAUSED:
            return state.pause();

        case StateType_RUNNING:
            return state.start();

        case StateType_COMPLETED:
            return state.complete();

        case StateType_FAILED:
            state.fail();
            return true;
        }
        return false;
    }

    State()
        : m_name("Installable")
        , m_state(StateType_NONE)
    {
    }

    virtual ~State()
    {
        if (m_state == StateType_PAUSED || m_state == StateType_RUNNING)
            cancel();
    }

    virtual bool ready()
    {
        switch(m_state) {
        case StateType_PAUSED:
        case StateType_COMPLETED:
        case StateType_FAILED:
        case StateType_RUNNING:
            return false;

        case StateType_READY:
            return true;

        case StateType_NONE:
            break;
        }
        changeStatus(StateType_READY);
        return true;
    }

    virtual bool start()
    {
        switch(m_state) {
        case StateType_NONE:
        case StateType_PAUSED:
        case StateType_COMPLETED:
        case StateType_FAILED:
            return false;

        case StateType_RUNNING:
            return true;

        case StateType_READY:
            break;
        }

        changeStatus(StateType_RUNNING);
        return true;
    }

    virtual bool pause()
    {
        switch(m_state) {
        case StateType_NONE:
        case StateType_READY:
        case StateType_COMPLETED:
        case StateType_FAILED:
            return false;

        case StateType_PAUSED:
            return true;

        case StateType_RUNNING:
            break;
        }

        changeStatus(StateType_PAUSED);
        return true;
    }

    virtual bool resume()
    {
        switch(m_state) {
        case StateType_NONE:
        case StateType_READY:
        case StateType_COMPLETED:
        case StateType_FAILED:
            return false;

        case StateType_RUNNING:
            return true;

        case StateType_PAUSED:
            break;
        }

        changeStatus(StateType_RUNNING);
        return true;
    }

    virtual bool cancel()
    {
        switch(m_state) {
        case StateType_NONE:
        case StateType_READY:
        case StateType_COMPLETED:
        case StateType_FAILED:
            return false;

        case StateType_PAUSED:
        case StateType_RUNNING:
            break;
        }

        changeStatus(StateType_READY);
        return true;
    }

    virtual bool complete()
    {
        switch(m_state) {
        case StateType_NONE:
        case StateType_READY:
        case StateType_FAILED:
        case StateType_PAUSED:
            return false;

        case StateType_COMPLETED:
            return true;

        case StateType_RUNNING:
            break;
        }

        changeStatus(StateType_COMPLETED);
        return true;
    }

    virtual void fail()
    {
        switch(m_state) {
        case StateType_NONE:
        case StateType_READY:
        case StateType_PAUSED:
        case StateType_COMPLETED:
        case StateType_FAILED:
            return;

        case StateType_RUNNING:
            break;
        }
        changeStatus(StateType_FAILED);
    }


    enum StateType getState()
    {
        return m_state;
    }

    const string& getName()
    {
        return m_name;
    }

    void setName(const string& name)
    {
        m_name = name;
    }

    void setCallback(Callback callback)
    {
        m_callback = std::move(callback);
    }

protected:
    void changeStatus(enum StateType nextStatus)
    {
        if (m_state == nextStatus)
            return;

        enum StateType prev = m_state;
        m_state = nextStatus;

        Logger::info(m_name, toString(prev) + " ==> " + toString(m_state));

        if (m_callback)
            m_callback(this, prev, m_state);
    }

private:
    string m_name;
    enum StateType m_state;

    Callback m_callback;
};

#endif /* CORE_STATE_H_ */
