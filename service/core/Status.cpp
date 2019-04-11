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

#include <core/Status.h>
#include "PolicyManager.h"

enum TransitionType Status::checkTransition(enum StatusType status)
{
    if (status == StatusType_CANCELED ||
        status == StatusType_COMPLETED ||
        status == StatusType_FAILED) {
        return TransitionType_Allowed;
    }

    switch (m_status) {
    case StatusType_NONE:
        if (status == StatusType_NONE) {
            return TransitionType_Same;
        } else if (status == StatusType_READY) {
            return TransitionType_Allowed;
        } else if (status == StatusType_RUNNING) {
            return TransitionType_NotAllowed;
        } else if (status == StatusType_PAUSED) {
            return TransitionType_NotAllowed;
        } else {
            return TransitionType_Unknown;
        }

    case StatusType_READY:
        if (status == StatusType_NONE) {
            return TransitionType_NotAllowed;
        } else if (status == StatusType_READY) {
            return TransitionType_Same;
        } else if (status == StatusType_RUNNING) {
            return TransitionType_Allowed;
        } else if (status == StatusType_PAUSED) {
            return TransitionType_NotAllowed;
        } else {
            return TransitionType_Unknown;
        }

    case StatusType_RUNNING:
        if (status == StatusType_NONE) {
            return TransitionType_NotAllowed;
        } else if (status == StatusType_READY) {
            return TransitionType_NotAllowed;
        } else if (status == StatusType_RUNNING) {
            return TransitionType_Same;
        } else if (status == StatusType_PAUSED) {
            return TransitionType_Allowed;
        } else {
            return TransitionType_Unknown;
        }

    case StatusType_PAUSED:
        if (status == StatusType_NONE) {
            return TransitionType_NotAllowed;
        } else if (status == StatusType_READY) {
            return TransitionType_NotAllowed;
        } else if (status == StatusType_RUNNING) {
            return TransitionType_Allowed;
        } else if (status == StatusType_PAUSED) {
            return TransitionType_Same;
        } else {
            return TransitionType_Unknown;
        }

    case StatusType_CANCELED:
        if (status == StatusType_CANCELED) {
            return TransitionType_Same;
        } else {
            return TransitionType_NotAllowed;
        }

    case StatusType_COMPLETED:
        if (status == StatusType_COMPLETED) {
            return TransitionType_Same;
        } else {
            return TransitionType_NotAllowed;
        }

    case StatusType_FAILED:
        if (status == StatusType_FAILED) {
            return TransitionType_Same;
        } else {
            return TransitionType_NotAllowed;
        }
    }
    return TransitionType_Unknown;
}

void Status::changeStatus(enum StatusType nextStatus)
{
    if (m_status == nextStatus)
        return;

    enum StatusType prev = m_status;
    m_status = nextStatus;

    Logger::verbose("ChangeStatus", m_name, toString(prev) + " ==> " + toString(m_status));
    if (m_callbackFunc) {
        m_callbackFunc(prev, m_status);
    }
    PolicyManager::getInstance().onChangeStatus();
}
