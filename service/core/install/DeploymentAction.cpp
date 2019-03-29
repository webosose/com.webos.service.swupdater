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
#include "util/JValueUtil.h"
#include "util/Logger.h"

DeploymentAction::DeploymentAction(JValue& json)
    : AbsAction()
    , m_isForceDownload(false)
    , m_isForceUpdate(false)
{
    setClassName("DeploymentAction");
    setName("DeploymentAction");

    setType(ActionType_INSTALL);
    fromJson(json);
}

DeploymentAction::~DeploymentAction()
{
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        (*it)->startInstallation();
    }
}

bool DeploymentAction::onReadyDownloading()
{
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        (*it)->readyDownloading();
    }
    return true;
}

bool DeploymentAction::onReadyInstallation()
{
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        (*it)->readyInstallation();
    }
    return true;
}

bool DeploymentAction::onStartDownloading()
{
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        (*it)->startDownloading();
    }
    return true;
}

bool DeploymentAction::onStartInstallation()
{
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        (*it)->startInstallation();
    }
    return true;
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
            softwareModule->setListener(this);
            m_softwareModules.push_back(softwareModule);
        }
    }
    return true;
}

bool DeploymentAction::toJson(JValue& json)
{
    AbsAction::toJson(json);
    json.put("status", this->getStatus());

    JValue softwareModules = pbnjson::Array();
    for (auto it = m_softwareModules.begin(); it != m_softwareModules.end(); ++it) {
        JValue softwareModule = pbnjson::Object();
        (*it)->toJson(softwareModule);
        softwareModules.append(softwareModule);
    }
    json.put("softwareModules", softwareModules);
    return true;
}
