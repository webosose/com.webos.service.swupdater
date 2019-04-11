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

DeploymentAction::DeploymentAction()
    : AbsAction()
    , m_isForceDownload(false)
    , m_isForceUpdate(false)
{
    setClassName("DeploymentAction");
    setType(ActionType_INSTALL);
}

DeploymentAction::~DeploymentAction()
{
}

bool DeploymentAction::fromJson(const JValue& json)
{
    ISerializable::fromJson(json);
    string dummy;

    JValueUtil::getValue(json, "id", m_id);
    JValueUtil::getValue(json, "deployment", "download", dummy);
    if (dummy == "forced") m_isForceDownload = true;
    JValueUtil::getValue(json, "deployment", "update", dummy);
    if (dummy == "forced") m_isForceUpdate = true;

    if (!json["deployment"].hasKey("chunks") || !json["deployment"]["chunks"].isArray()) {
        return false;
    }

    for (JValue chunk : json["deployment"]["chunks"].items()) {
        shared_ptr<SoftwareModule> module = make_shared<SoftwareModule>();
        module->fromJson(chunk);
        this->add(module);
    }
    return true;
}

bool DeploymentAction::toJson(JValue& json)
{
    Component::toJson(json);

    json.put("id", m_id);

    JValue softwareModules = pbnjson::Array();
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        JValue softwareModule = pbnjson::Object();
        (*it)->toJson(softwareModule);
        softwareModules.append(softwareModule);
    }
    json.put("softwareModules", softwareModules);
    return true;
}
