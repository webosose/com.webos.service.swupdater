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

#include "util/Logger.h"

using namespace std;

template <class T>
class IManageable {
public:
    static T& getInstance()
    {
        static T _instance;
        return _instance;
    }

    virtual ~IManageable() {};

    virtual bool initialize(GMainLoop* mainloop) final
    {
        bool result;
        Logger::info(m_name, "Start initialization");
        m_mainloop = mainloop;
        result = onInitialization();
        Logger::info(m_name, "End initialization");
        return result;
    }

    virtual bool finalize() final
    {
        bool result;
        Logger::info(m_name, "Start finalization");
        result = onFinalization();
        Logger::info(m_name, "End finalization");
        return result;
    }

    virtual bool onInitialization() = 0;
    virtual bool onFinalization() = 0;

protected:
    IManageable()
        : m_mainloop(nullptr)
        , m_name("")
    {
    };

    string& getName()
    {
        return m_name;
    }

    void setName(string name)
    {
        m_name = name;
    }

    GMainLoop* m_mainloop;
    string m_name;

};


#endif /* INTERFACE_IMANAGEABLE_H_ */
