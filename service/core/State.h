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

#include "util/Logger.h"

using namespace std;

enum StateType {
    StateType_NONE,
    StateType_READY,
    StateType_WAITING,
    StateType_RUNNING,
    StateType_PAUSED,
    StateType_CANCELED,
    StateType_COMPLETED,
    StateType_FAILED,
};

enum TransitionType {
    TransitionType_Unknown = -2,
    TransitionType_NotAllowed = -1,
    TransitionType_Same = 0,
    TransitionType_Allowed = 1
};

typedef std::function<void(enum StateType prev, enum StateType cur)> Callback;

class State {
public:
    static string toString(enum StateType type)
    {
        switch (type) {
        case StateType_NONE:
            return "none";

        case StateType_READY:
            return "ready";

        case StateType_WAITING:
            return "waiting";

        case StateType_RUNNING:
            return "running";

        case StateType_PAUSED:
            return "paused";

        case StateType_CANCELED:
            return "canceled";

        case StateType_COMPLETED:
            return "completed";

        case StateType_FAILED:
            return "failed";
        }
        return "unknown";
    }

    static bool writeCommonLog(State& state, enum TransitionType type, const string& className, const string& transition)
    {
        if (type == TransitionType_NotAllowed) {
            Logger::info(className, transition + " is not allowed from " + state.getStateStr());
            return false;
        } else if (type == TransitionType_Same) {
            Logger::info(className, transition + " is already called");
            return true;
        }
        return true;
    }

    State(const string& name)
        : m_name(name)
        , m_state(StateType_NONE)
    {
    }

    virtual ~State()
    {
        if (m_state == StateType_PAUSED || m_state == StateType_RUNNING || m_state == StateType_WAITING) {
            cancel();
        }
    }

    enum TransitionType canPrepare()
    {
        return checkTransition(StateType_READY);
    }

    bool prepare()
    {
        if (checkTransition(StateType_READY) == TransitionType_NotAllowed)
            return false;
        changeState(StateType_READY);
        return true;
    }

    enum TransitionType canWait()
    {
        return checkTransition(StateType_WAITING);
    }

    bool wait()
    {
        if (canWait() == TransitionType_NotAllowed)
            return false;
        changeState(StateType_WAITING);
        return true;
    }

    enum TransitionType canStart()
    {
        return checkTransition(StateType_RUNNING);
    }

    bool start()
    {
        if (canStart() == TransitionType_NotAllowed)
            return false;
        changeState(StateType_RUNNING);
        return true;
    }

    enum TransitionType canPause()
    {
        return checkTransition(StateType_PAUSED);
    }

    bool pause()
    {
        if (canPause() == TransitionType_NotAllowed)
            return false;
        changeState(StateType_PAUSED);
        return true;
    }

    enum TransitionType canResume()
    {
        return checkTransition(StateType_RUNNING);
    }

    bool resume()
    {
        if (canResume() == TransitionType_NotAllowed)
            return false;
        changeState(StateType_RUNNING);
        return true;
    }

    enum TransitionType canCancel()
    {
        return checkTransition(StateType_CANCELED);
    }

    bool cancel()
    {
        if (canCancel() == TransitionType_NotAllowed)
            return false;
        changeState(StateType_CANCELED);
        return true;
    }

    enum TransitionType canComplete()
    {
        return checkTransition(StateType_COMPLETED);
    }

    bool complete()
    {
        if (canComplete() == TransitionType_NotAllowed)
            return false;
        changeState(StateType_COMPLETED);
        return true;
    }

    enum TransitionType canFail()
    {
        return checkTransition(StateType_FAILED);
    }

    bool fail()
    {
        if (canFail() == TransitionType_NotAllowed)
            return false;
        changeState(StateType_FAILED);
        return true;
    }

    bool transit(enum StateType state)
    {
        switch (state) {
        case StateType_NONE:
            return false;

        case StateType_READY:
            return prepare();

        case StateType_WAITING:
            return wait();

        case StateType_RUNNING:
            return start();

        case StateType_PAUSED:
            return pause();

        case StateType_CANCELED:
            return cancel();

        case StateType_COMPLETED:
            return complete();

        case StateType_FAILED:
            return fail();
        }
        return false;
    }

    enum StateType getState()
    {
        return m_state;
    }

    string getStateStr()
    {
        return toString(m_state);
    }

    const string& getName()
    {
        return m_name;
    }

    void setCallback(Callback callbackFunc)
    {
        m_callbackFunc = std::move(callbackFunc);
    }

private:
    enum TransitionType checkTransition(enum StateType state);
    void changeState(enum StateType nextState);

    string m_name;
    enum StateType m_state;

    Callback m_callbackFunc;
};

#endif /* CORE_STATE_H_ */
