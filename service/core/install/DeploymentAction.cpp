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
{
    setClassName("DeploymentAction");
    setType(ActionType_INSTALL);
    fromJson(json);
    m_downloadState.setName("DeploymentAction-download");
    m_updateState.setName("DeploymentAction-update");
}

DeploymentAction::~DeploymentAction()
{
}

bool DeploymentAction::ready(bool download)
{
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        if (!(*it)->ready(download))
            return false;
    }
    if (download)
        return m_downloadState.ready();
    else
        return m_updateState.ready();
}

bool DeploymentAction::start(bool download)
{
    if (download) {
        for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
            if (!(*it)->startDownload()) {
                return false;
            }
        }
        return m_downloadState.start();
    } else {
        for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
            if (!(*it)->startUpdate()) {
                return false;
            }
        }
        return m_updateState.start();
    }
}

bool DeploymentAction::pause(bool download)
{
    if (download)
        return m_downloadState.pause();
    else
        return m_updateState.pause();
}

bool DeploymentAction::resume(bool download)
{
    if (download)
        return m_downloadState.resume();
    else
        return m_updateState.resume();
}

bool DeploymentAction::cancel(bool download)
{
    if (download)
        return m_downloadState.cancel();
    else
        return m_updateState.cancel();
}

void DeploymentAction::onDownloadStateChanged(State *installer, enum StateType prev, enum StateType cur)
{
    if (cur == StateType_FAILED) {
        m_downloadState.fail();
    }

    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        if ((*it)->getDownloadState().getState() != cur) {
            return;
        }
    }

    State::transition(m_downloadState, cur);
}

void DeploymentAction::onUpdateStateChanged(State *installer, enum StateType prev, enum StateType cur)
{
    if (cur == StateType_FAILED) {
        m_updateState.fail();
    }

    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        if ((*it)->getUpdateState().getState() != cur) {
            return;
        }
    }

    State::transition(m_updateState, cur);
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
        shared_ptr<SoftwareModule> softwareModule = SoftwareModule::createSoftwareModule(chunk);
        if (softwareModule) {
            m_softwareModules.push_back(softwareModule);
            m_softwareModules.back()->getUpdateState().setCallback( // @suppress("Invalid arguments")
                std::bind(&DeploymentAction::onUpdateStateChanged,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3
                 )
            );
            m_softwareModules.back()->getDownloadState().setCallback( // @suppress("Invalid arguments")
                std::bind(&DeploymentAction::onDownloadStateChanged,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3
                )
            );
        }
    }
    return true;
}

bool DeploymentAction::toJson(JValue& json)
{
    AbsAction::toJson(json);
    json.put("download", State::toString(m_downloadState.getState()));
    json.put("update", State::toString(m_updateState.getState()));

    JValue softwareModules = pbnjson::Array();
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        JValue softwareModule = pbnjson::Object();
        (*it)->toJson(softwareModule);
        softwareModules.append(softwareModule);
    }
    json.put("softwareModules", softwareModules);
    return true;
}