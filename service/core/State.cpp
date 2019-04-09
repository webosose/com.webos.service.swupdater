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

#include "core/State.h"
#include "PolicyManager.h"

enum TransitionType State::checkTransition(enum StateType state)
{
    switch (m_state) {
    case StateType_NONE:
        if (state == StateType_NONE) {
            return TransitionType_Same;
        } else if (state == StateType_READY) {
            return TransitionType_Allowed;
        } else if (state == StateType_WAITING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_RUNNING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_PAUSED) {
            return TransitionType_Allowed;
        } else if (state == StateType_CANCELED) {
            return TransitionType_Allowed;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_Allowed;
        } else if (state == StateType_FAILED) {
            return TransitionType_Allowed;
        }
        return TransitionType_Unknown;

    case StateType_READY:
        if (state == StateType_NONE) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_READY) {
            return TransitionType_Same;
        } else if (state == StateType_WAITING) {
            return TransitionType_Allowed;
        } else if (state == StateType_RUNNING) {
            return TransitionType_Allowed;
        } else if (state == StateType_PAUSED) {
            return TransitionType_Allowed;
        } else if (state == StateType_CANCELED) {
            return TransitionType_Allowed;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_Allowed;
        } else if (state == StateType_FAILED) {
            return TransitionType_Allowed;
        }
        return TransitionType_Unknown;

    case StateType_WAITING:
        if (state == StateType_NONE) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_READY) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_WAITING) {
            return TransitionType_Same;
        } else if (state == StateType_RUNNING) {
            return TransitionType_Allowed;
        } else if (state == StateType_PAUSED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_CANCELED) {
            return TransitionType_Allowed;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_Allowed;
        } else if (state == StateType_FAILED) {
            return TransitionType_Allowed;
        }
        return TransitionType_Unknown;

    case StateType_RUNNING:
        if (state == StateType_NONE) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_READY) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_WAITING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_RUNNING) {
            return TransitionType_Same;
        } else if (state == StateType_PAUSED) {
            return TransitionType_Allowed;
        } else if (state == StateType_CANCELED) {
            return TransitionType_Allowed;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_Allowed;
        } else if (state == StateType_FAILED) {
            return TransitionType_Allowed;
        }
        return TransitionType_Unknown;

    case StateType_PAUSED:
        if (state == StateType_NONE) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_READY) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_WAITING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_RUNNING) {
            return TransitionType_Allowed;
        } else if (state == StateType_PAUSED) {
            return TransitionType_Same;
        } else if (state == StateType_CANCELED) {
            return TransitionType_Allowed;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_Allowed;
        } else if (state == StateType_FAILED) {
            return TransitionType_Allowed;
        }
        return TransitionType_Unknown;

    case StateType_CANCELED:
        if (state == StateType_NONE) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_READY) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_WAITING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_RUNNING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_PAUSED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_CANCELED) {
            return TransitionType_Same;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_FAILED) {
            return TransitionType_NotAllowed;
        }
        return TransitionType_Unknown;

    case StateType_COMPLETED:
        if (state == StateType_NONE) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_READY) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_WAITING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_RUNNING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_PAUSED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_CANCELED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_Same;
        } else if (state == StateType_FAILED) {
            return TransitionType_NotAllowed;
        }
        return TransitionType_Unknown;

    case StateType_FAILED:
        if (state == StateType_NONE) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_READY) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_WAITING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_RUNNING) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_PAUSED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_CANCELED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_COMPLETED) {
            return TransitionType_NotAllowed;
        } else if (state == StateType_FAILED) {
            return TransitionType_Same;
        }
        return TransitionType_Unknown;
    }
    return TransitionType_Unknown;
}

void State::changeState(enum StateType nextState)
{
    if (m_state == nextState)
        return;

    enum StateType prev = m_state;
    m_state = nextState;

    Logger::verbose("ChangeState", m_name, toString(prev) + " ==> " + toString(m_state));
    if (m_callbackFunc) {
        m_callbackFunc(prev, m_state);
    }
    PolicyManager::getInstance().onChangeStatus();
}
