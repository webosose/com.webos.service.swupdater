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

#ifndef INTERFACE_IINSTALLABLE_H_
#define INTERFACE_IINSTALLABLE_H_

#include <iostream>

#include "util/Logger.h"
#include "IListener.h"

using namespace std;

#define LOG_NAME                "IInstallableListener"
#define LOG_REQUEST_INVALID     "Invalid Request "
#define LOG_REQUEST_VALID       "Valid Request "
#define LOG_REQUEST_IGNORE      "Ignore Request "

enum InstallableState {
    InstallableState_NONE,
    InstallableState_READY,
    InstallableState_PAUSED,
    InstallableState_RUNNING,
    InstallableState_COMPLETED,
    InstallableState_FAILED,
};

class IInstallable : public IListener<IInstallable> {
public:
    static string toString(enum InstallableState& type)
    {
        switch (type) {
        case InstallableState_NONE:
            return "None";

        case InstallableState_READY:
            return "Ready";

        case InstallableState_PAUSED:
            return "Paused";

        case InstallableState_RUNNING:
            return "Running";

        case InstallableState_COMPLETED:
            return "Completed";

        case InstallableState_FAILED:
            return "Failed";
        }
        return "Unknown";
    }

    IInstallable()
        : m_name("Installable")
        , m_downloading(InstallableState_NONE)
        , m_installation(InstallableState_NONE)
    {
    }

    virtual ~IInstallable()
    {
        if (m_downloading == InstallableState_PAUSED || m_downloading == InstallableState_RUNNING)
            cancelDownloading();
        if (m_installation == InstallableState_PAUSED || m_installation == InstallableState_RUNNING)
            cancelInstallation();
    }

    string getStatus()
    {
        string result = "none";
        if (m_downloading == InstallableState_NONE && m_installation == InstallableState_NONE) {
            return result;
        }
        if (m_downloading != InstallableState_COMPLETED) {
            result = "download" + toString(m_downloading);
        } else {
            result = "install" + toString(m_installation);
        }
        return result;
    }

    bool readyDownloading()
    {
        switch(m_downloading) {

        case InstallableState_PAUSED:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_READY:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_NONE:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onReadyDownloading()) {
            changeStatusDownloading(InstallableState_READY);
            return true;
        } else {
            changeStatusDownloading(InstallableState_FAILED);
            return false;
        }
    }

    bool startDownloading()
    {
        switch(m_downloading) {
        case InstallableState_NONE:
        case InstallableState_PAUSED:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_READY:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onStartDownloading()) {
            changeStatusDownloading(InstallableState_RUNNING);
            return true;
        } else {
            changeStatusDownloading(InstallableState_FAILED);
            return false;
        }
    }

    bool pauseDownloading()
    {
        switch(m_downloading) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_PAUSED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onPauseDownloading()) {
            changeStatusDownloading(InstallableState_PAUSED);
            return true;
        } else {
            changeStatusDownloading(InstallableState_FAILED);
            return false;
        }
    }

    bool resumeDownloading()
    {
        switch(m_downloading) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_PAUSED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onResumeDownloading()) {
            changeStatusDownloading(InstallableState_RUNNING);
            return true;
        } else {
            changeStatusDownloading(InstallableState_FAILED);
            return false;
        }
    }

    bool cancelDownloading()
    {
        switch(m_downloading) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_PAUSED:
        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onCancelDownloading()) {
            changeStatusDownloading(InstallableState_READY);
            return true;
        } else {
            changeStatusDownloading(InstallableState_FAILED);
            return false;
        }
    }

    bool completeDownloading()
    {
        switch(m_downloading) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_FAILED:
        case InstallableState_PAUSED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_COMPLETED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onCompleteDownloading()) {
            changeStatusDownloading(InstallableState_COMPLETED);
            return true;
        } else {
            changeStatusDownloading(InstallableState_FAILED);
            return false;
        }
    }

    bool failDownloading()
    {
        switch(m_downloading) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_PAUSED:
        case InstallableState_COMPLETED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        onFailDownloading();
        changeStatusDownloading(InstallableState_FAILED);
        return true;
    }

    bool readyInstallation()
    {
        switch(m_installation) {
        case InstallableState_PAUSED:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_READY:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            return true;

        case InstallableState_NONE:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            break;
        }

        if (onReadyInstallation()) {
            changeStatusInstallation(InstallableState_READY);
            return true;
        } else {
            changeStatusInstallation(InstallableState_FAILED);
            return false;
        }
    }

    bool startInstallation()
    {
        switch(m_installation) {
        case InstallableState_NONE:
        case InstallableState_PAUSED:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_READY:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onStartInstallation()) {
            changeStatusInstallation(InstallableState_RUNNING);
            return true;
        } else {
            changeStatusInstallation(InstallableState_FAILED);
            return false;
        }
    }

    bool pauseInstallation()
    {
        switch(m_installation) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_PAUSED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onPauseInstallation()) {
            changeStatusInstallation(InstallableState_PAUSED);
            return true;
        } else {
            changeStatusInstallation(InstallableState_FAILED);
            return false;
        }
    }

    bool resumeInstallation()
    {
        switch(m_installation) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_PAUSED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onResumeInstallation()) {
            changeStatusInstallation(InstallableState_RUNNING);
            return true;
        } else {
            changeStatusInstallation(InstallableState_FAILED);
            return false;
        }
    }

    bool cancelInstallation()
    {
        switch(m_installation) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_COMPLETED:
        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_PAUSED:
        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onCancelInstallation()) {
            changeStatusInstallation(InstallableState_READY);
            return true;
        } else {
            changeStatusInstallation(InstallableState_FAILED);
            return false;
        }
    }

    bool completeInstallation()
    {
        switch(m_installation) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_FAILED:
        case InstallableState_PAUSED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_COMPLETED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        if (onCompleteInstallation()) {
            changeStatusInstallation(InstallableState_COMPLETED);
            return true;
        } else {
            changeStatusInstallation(InstallableState_FAILED);
            return false;
        }
    }

    bool failInstallation()
    {
        switch(m_installation) {
        case InstallableState_NONE:
        case InstallableState_READY:
        case InstallableState_PAUSED:
        case InstallableState_COMPLETED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_INVALID);
            return false;

        case InstallableState_FAILED:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_IGNORE);
            return true;

        case InstallableState_RUNNING:
            Logger::verbose(m_name, __FUNCTION__, LOG_REQUEST_VALID);
            break;
        }

        onFailInstallation();
        changeStatusInstallation(InstallableState_FAILED);
        return true;
    }

    enum InstallableState getDownloadingState()
    {
        return m_downloading;
    }

    enum InstallableState getInstallationState()
    {
        return m_installation;
    }

    const string& getName()
    {
        return m_name;
    }

    virtual void onCompletedChildDownloading(IInstallable* installable)
    {
        completeDownloading();
    }

    virtual void onFailedChildDownloading(IInstallable* installable)
    {
        failDownloading();
    }

    virtual void onCompletedChildInstallation(IInstallable* installable)
    {
        completeInstallation();
    }

    virtual void onFailedChildInstallation(IInstallable* installable)
    {
        failInstallation();
    }

    virtual void onProgressChildDownloading(IInstallable* installable)
    {
        if (m_listener)
            m_listener->onProgressChildDownloading(installable);
        else
            Logger::warning(m_name, "'onProgressDownloading' is called. Please override this interface");
    }

    virtual void onProgressChildInstallation(IInstallable* installable)
    {
        if (m_listener)
            m_listener->onProgressChildInstallation(installable);
        else
            Logger::warning(m_name, "'onProgressInstallation' is called. Please override this interface");
    }

protected:
    void setName(const string& name)
    {
        m_name = name;
    }

    virtual bool onReadyDownloading()
    {
        Logger::warning(m_name, "Default 'onReadyDownloading' is called.");
        return true;
    }

    virtual bool onStartDownloading()
    {
        Logger::warning(m_name, "Default 'onStartDownloading' is called.");
        return true;
    }

    virtual bool onPauseDownloading()
    {
        Logger::warning(m_name, "Default 'onPauseDownloading' is called.");
        return true;
    }

    virtual bool onResumeDownloading()
    {
        Logger::warning(m_name, "Default 'onResumeDownloading' is called.");
        return true;
    }

    virtual bool onCancelDownloading()
    {
        Logger::warning(m_name, "Default 'onCancelDownloading' is called.");
        return true;
    }

    virtual bool onCompleteDownloading()
    {
        Logger::warning(m_name, "Default 'onCompleteDownloading' is called.");
        return true;
    }

    virtual void onFailDownloading()
    {
        Logger::warning(m_name, "Default 'onFailDownloading' is called.");
        return;
    }

    virtual bool onReadyInstallation()
    {
        Logger::warning(m_name, "Default 'onReadyInstallation' is called.");
        return true;
    }

    virtual bool onStartInstallation()
    {
        Logger::warning(m_name, "Default 'onStartInstallation' is called.");
        return true;
    }

    virtual bool onPauseInstallation()
    {
        Logger::warning(m_name, "Default 'onPauseInstallation' is called.");
        return true;
    }

    virtual bool onResumeInstallation()
    {
        Logger::warning(m_name, "Default 'onResumeInstallation' is called.");
        return true;
    }

    virtual bool onCancelInstallation()
    {
        Logger::warning(m_name, "Default 'onCancelInstallation' is called.");
        return true;
    }

    virtual bool onCompleteInstallation()
    {
        Logger::warning(m_name, "Default 'onCompleteInstallation' is called.");
        return true;
    }

    virtual void onFailInstallation()
    {
        Logger::warning(m_name, "Default 'onFailInstallation' is called.");
        return;
    }

private:
    void changeStatusDownloading(enum InstallableState status)
    {
        if (m_downloading == status)
            return;

        Logger::info(m_name, "Downloading", toString(m_downloading) + " ==> " + toString(status));
        m_downloading = status;

        if (!m_listener)
            return;

        switch (m_downloading) {
        case InstallableState_COMPLETED:
            m_listener->onCompletedChildDownloading(this);
            break;

        case InstallableState_FAILED:
            m_listener->onFailedChildDownloading(this);
            break;

        default:
            break;
        }
    }

    void changeStatusInstallation(enum InstallableState status)
    {
        if (m_installation == status)
            return;

        Logger::info(m_name, "Installation", toString(m_installation) + " ==> " + toString(status));
        m_installation = status;

        if (!m_listener)
            return;

        switch (m_installation) {
        case InstallableState_COMPLETED:
            m_listener->onCompletedChildInstallation(this);
            break;

        case InstallableState_FAILED:
            m_listener->onFailedChildInstallation(this);
            break;

        default:
            break;
        }
    }

    string m_name;
    enum InstallableState m_downloading;
    enum InstallableState m_installation;
};

#endif /* INTERFACE_IINSTALLABLE_H_ */
