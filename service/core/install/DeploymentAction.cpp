// Copyright (c) 2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "core/install/DeploymentAction.h"
#include "PolicyManager.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

DeploymentAction::DeploymentAction(JValue& json)
    : AbsAction()
    , m_isForceDownload(false)
    , m_isForceUpdate(false)
    , m_download("DeploymentAction-download")
    , m_update("DeploymentAction-update")
    , m_curDownload(0)
    , m_curUpdate(0)
{
    setClassName("DeploymentAction");
    setType(ActionType_INSTALL);
    fromJson(json);
}

DeploymentAction::~DeploymentAction()
{
    removeCallback();
    m_modules.clear();
}

void DeploymentAction::onDownloadStateChanged(enum StateType prev, enum StateType cur)
{
    switch (cur) {
    case StateType_NONE:
        break;

    case StateType_READY:
        break;

    case StateType_WAITING:
        break;

    case StateType_RUNNING:
        break;

    case StateType_PAUSED:
        break;

    case StateType_CANCELED:
        m_download.cancel();
        break;

    case StateType_COMPLETED:
        if (m_curDownload == m_modules.size() -1) {
            m_download.complete();
            break;
        }
        m_curDownload++;
        m_modules[m_curDownload].startDownload();
        break;

    case StateType_FAILED:
        m_download.fail();
        break;
    }

    // All artifacts are downloaded
    if (m_download.getState() == StateType_COMPLETED && m_update.getState() == StateType_RUNNING) {
        m_modules[m_curUpdate].startUpdate();
    }
}

bool DeploymentAction::prepareDownload()
{
    enum TransitionType type = m_download.canPrepare();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_download, type, getClassName(), "prepare");
    }
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        if (!it->prepareDownload()) {
            m_download.fail();
            return false;
        }
    }
    return m_download.prepare();
}

bool DeploymentAction::startDownload()
{
    enum TransitionType type = m_download.canStart();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_download, type, getClassName(), "start");
    }

    m_curDownload = 0;
    if (!m_modules[m_curDownload].startDownload()) {
        Logger::verbose(getClassName(), m_modules[m_curDownload].getName(), "Start download");
        return false;
    }
    return m_download.start();
}

bool DeploymentAction::pauseDownload()
{
    enum TransitionType type = m_download.canPause();
    if (type == TransitionType_Same) {
        return true;
    } else if (type == TransitionType_NotAllowed) {
        return false;
    }
    return m_download.pause();
}

bool DeploymentAction::resumeDownload()
{
    enum TransitionType type = m_download.canResume();
    if (type == TransitionType_Same) {
        return true;
    } else if (type == TransitionType_NotAllowed) {
        return false;
    }
    return m_download.resume();
}

bool DeploymentAction::cancelDownload()
{
    enum TransitionType type = m_download.canCancel();
    if (type == TransitionType_Same) {
        return true;
    } else if (type == TransitionType_NotAllowed) {
        return false;
    }
    return m_download.cancel();
}

void DeploymentAction::onUpdateStateChanged(enum StateType prev, enum StateType cur)
{
    switch (cur) {
     case StateType_NONE:
         break;

     case StateType_READY:
         break;

     case StateType_WAITING:
         break;

     case StateType_RUNNING:
         break;

     case StateType_PAUSED:
         break;

     case StateType_CANCELED:
         m_update.cancel();
         break;

     case StateType_COMPLETED:
         if (m_curUpdate == m_modules.size() -1) {
             m_update.complete();
             break;
         }
         m_curUpdate++;
         m_modules[m_curUpdate].startUpdate();
         break;

     case StateType_FAILED:
         m_update.fail();
         break;
     }
}

bool DeploymentAction::prepareUpdate()
{
    enum TransitionType type = m_update.canPrepare();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_update, type, getClassName(), "prepare");
    }
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        if (!it->prepareUpdate()) {
            m_update.fail();
            return false;
        }
    }
    return m_update.prepare();
}

bool DeploymentAction::startUpdate()
{
    enum TransitionType type = m_update.canStart();
    if (type != TransitionType_Allowed) {
        return State::writeCommonLog(m_update, type, getClassName(), "start");
    }

    m_curUpdate = 0;
    if (m_download.getState() == StateType_COMPLETED) {
        m_modules[m_curUpdate].startUpdate();
    }
    return m_update.start();
}

bool DeploymentAction::pauseUpdate()
{
    return m_update.pause();
}

bool DeploymentAction::resumeUpdate()
{
    return m_update.resume();
}

bool DeploymentAction::cancelUpdate()
{
    return m_update.cancel();
}

bool DeploymentAction::fromJson(const JValue& json)
{
    AbsAction::fromJson(json);

    string dummy;
    JValueUtil::getValue(json, "deployment", "download", dummy);
    if (dummy == "forced") m_isForceDownload = true;
    JValueUtil::getValue(json, "deployment", "update", dummy);
    if (dummy == "forced") m_isForceUpdate = true;

    if (!json["deployment"].hasKey("chunks") || !json["deployment"]["chunks"].isArray()) {
        return false;
    }

    for (JValue chunk : json["deployment"]["chunks"].items()) {
        m_modules.emplace_back(chunk);
    }
    addCallback();
    return true;
}

bool DeploymentAction::toJson(JValue& json)
{
    AbsAction::toJson(json);
    json.put("download", m_download.getStateStr());
    json.put("update", m_update.getStateStr());

    JValue softwareModules = pbnjson::Array();
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        JValue softwareModule = pbnjson::Object();
        it->toJson(softwareModule);
        softwareModules.append(softwareModule);
    }
    json.put("softwareModules", softwareModules);
    return true;
}


void DeploymentAction::addCallback()
{
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        it->getDownload().setCallback( // @suppress("Invalid arguments")
            std::bind(&DeploymentAction::onDownloadStateChanged,
                      this,
                      std::placeholders::_1,
                      std::placeholders::_2
            )
        );

        it->getUpdate().setCallback( // @suppress("Invalid arguments")
            std::bind(&DeploymentAction::onUpdateStateChanged,
                      this,
                      std::placeholders::_1,
                      std::placeholders::_2
            )
        );
    }
}

void DeploymentAction::removeCallback()
{
    for (auto it = m_modules.begin(); it != m_modules.end(); ++it) {
        it->getDownload().setCallback(nullptr);
        it->getUpdate().setCallback(nullptr);
    }
}
