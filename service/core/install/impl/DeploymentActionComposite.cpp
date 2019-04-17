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

#include "core/install/impl/DeploymentActionComposite.h"

#include <memory>

#include "PolicyManager.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

DeploymentActionComposite::DeploymentActionComposite()
    : AbsAction()
    , m_isForceDownload(false)
    , m_isForceUpdate(false)
{
    setClassName("DeploymentActionComposite");
    m_status.setName("DeploymentActionComposite");
    setType(ActionType_INSTALL);

    m_status.addCallback( // @suppress("Invalid arguments")
        std::bind(&DeploymentActionComposite::onStatusChanged,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2
        )
    );
}

DeploymentActionComposite::~DeploymentActionComposite()
{
}

void DeploymentActionComposite::onStatusChanged(enum StatusType prev, enum StatusType cur)
{
    return;

    // TODO currently, reboot is not supported in ICAS3 board.
    if (cur != StatusType_COMPLETED)
        return;

    int seconds = -1;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        JValue metadata = std::dynamic_pointer_cast<SoftwareModuleComposite>(*it)->getMetadata();
        string value = JValueUtil::getMeta(metadata, "reboot");

        if (!value.empty() && std::atoi(value.c_str()) > seconds) {
            seconds = std::atoi(value.c_str());
        }
    }

    if (seconds > 0)
        PolicyManager::getInstance().onRequestReboot(seconds);
}

bool DeploymentActionComposite::fromJson(const JValue& json)
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
        shared_ptr<SoftwareModuleComposite> module = make_shared<SoftwareModuleComposite>();
        module->fromJson(chunk);
        this->add(module);
    }
    return true;
}

bool DeploymentActionComposite::toJson(JValue& json)
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
