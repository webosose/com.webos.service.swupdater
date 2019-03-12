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
        Logger::getInstance().normal("initialize starts", m_name);
        m_mainloop = mainloop;
        result = onInitialization();
        Logger::getInstance().normal("initialize ends", m_name);
        return result;
    }

    virtual bool finalize() final
    {
        bool result;
        Logger::getInstance().normal("finalize starts", m_name);
        result = onFinalization();
        Logger::getInstance().normal("finalize ends", m_name);
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
