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

#include "ls2/NotificationManager.h"
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
    disbleCallback();
    m_status.clearCallback();
}

void DeploymentActionComposite::onStatusChanged(enum StatusType prev, enum StatusType cur)
{
    if (cur != StatusType_COMPLETED)
        return;

    bool isRebootRequired = false;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        JValue metadata = std::dynamic_pointer_cast<SoftwareModuleComposite>(*it)->getMetadata();
        string value = JValueUtil::getMeta(metadata, "installer");

        if (value == "opkg") {
            // TODO need to find better solution
            isRebootRequired = true;
            break;
        }
    }

    if (!isRebootRequired) {
        return;
    }

    string title = "System has been updated. Reboot required!";
    string message;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        std::shared_ptr<SoftwareModuleComposite> module = std::dynamic_pointer_cast<SoftwareModuleComposite>(*it);
        message += " - " + module->getName() + " (" + module->getVersion() + ")<br>";
    }

    JValue buttons = pbnjson::Array();
    JValue button = pbnjson::Object();
    button.put("label", "Reboot");
    button.put("onclick", "luna://com.webos.service.power/shutdown/machineReboot");

    JValue params = pbnjson::Object();
    params.put("reason", "ota");
    button.put("params", params);

    buttons.append(button);

    NotificationManager::getInstance().createAlert(title, message, buttons);
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
        if (module->getType() == SoftwareModuleType_OS) // process OS type first
            m_children.push_front(module);
        else
            m_children.push_back(module);
    }
    enableCallback();
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

bool DeploymentActionComposite::isOnlyOSModuleCompleted()
{
    bool hasOSModule = false;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        SoftwareModuleType type = std::dynamic_pointer_cast<SoftwareModuleComposite>(*it)->getType();
        if (type == SoftwareModuleType_OS) {
            hasOSModule = true;
            break;
        }
    }
    if (!hasOSModule)
        return false;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        shared_ptr<SoftwareModuleComposite> module = std::dynamic_pointer_cast<SoftwareModuleComposite>(*it);
        if (module->getType() == SoftwareModuleType_OS && module->getStatus().getStatus() == StatusType_COMPLETED)
            continue;
        if (module->getType() == SoftwareModuleType_Application && module->getStatus().getStatus() == StatusType_READY)
            continue;
        return false;
    }
    return true;
}

bool DeploymentActionComposite::toProceedingJson(JValue& json)
{
    Component::toJson(json);

    int index = 0;
    int size = m_children.size();
    for ( ; index < size; index++) {
        if (m_children[index]->getStatus().getStatus() != StatusType_COMPLETED) {
            break;
        }
    }
    json.put("completedSoftwareModule", index);
    return true;
}

bool DeploymentActionComposite::restoreActionHistory(const JValue& json, bool isRebootDetected) {
    bool waitingReboot = json["waitingReboot"].asBool();
    string status = json["status"].asString();
    m_current = json["completedSoftwareModule"].asNumber<int>();
    unsigned int size = m_children.size();
    for (unsigned int i = 0; i < size; i++) {
        shared_ptr<SoftwareModuleComposite> softwareModule = std::dynamic_pointer_cast<SoftwareModuleComposite>(m_children[i]);
        if (i < m_current) {
            softwareModule->restore(StatusType_COMPLETED);
        } else {
            softwareModule->restore(StatusType_READY);
        }
    }
    getStatus().prepare();
    if (waitingReboot && !isRebootDetected) {
        setWaitingReboot();
    }
    if (m_current == m_children.size())
        getStatus().complete();
    else if (status == Status::toString(StatusType_RUNNING))
        resume();
    else if (status == Status::toString(StatusType_PAUSED))
        pause();

    return true;
}
