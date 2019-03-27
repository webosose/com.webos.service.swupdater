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

#ifndef INTERFACE_IMANAGEABLE_H_
#define INTERFACE_IMANAGEABLE_H_

#include <iostream>
#include <glib.h>

#include "IClassName.h"
#include "util/Logger.h"

using namespace std;

template <class T>
class IManageable : public IClassName {
public:
    static T& getInstance()
    {
        static T _instance;
        return _instance;
    }

    virtual ~IManageable() {};

    virtual bool initialize(GMainLoop* mainloop) final
    {
        Logger::info(getClassName(), "Start initialization");
        m_mainloop = mainloop;
        m_isInitalized = onInitialization();
        Logger::info(getClassName(), "End initialization");
        return m_isInitalized;
    }

    virtual bool finalize() final
    {
        Logger::info(getClassName(), "Start finalization");
        m_isFinalized = onFinalization();
        Logger::info(getClassName(), "End finalization");
        return m_isFinalized;
    }

    virtual bool isReady()
    {
        return m_isReady;
    }

    virtual bool isInitalized()
    {
        return m_isInitalized;
    }

    virtual bool isFinalized()
    {
        return m_isFinalized;
    }

    virtual bool onInitialization() = 0;
    virtual bool onFinalization() = 0;

protected:
    IManageable()
        : m_mainloop(nullptr)
        , m_isReady(false)
        , m_isInitalized(false)
        , m_isFinalized(false)
    {
    };

    void ready()
    {
        Logger::info(getClassName(), "Ready");
        m_isReady = true;
    }

    GMainLoop* m_mainloop;

private:
    bool m_isReady;
    bool m_isInitalized;
    bool m_isFinalized;

};


#endif /* INTERFACE_IMANAGEABLE_H_ */
