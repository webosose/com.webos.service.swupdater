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
    , m_status("XXX_name")
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
            createRebootAlert(SoftwareModuleType_Application);
            break;
        }
    }
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
        module->setListener(this);
        module->fromJson(chunk);
        if (module->getType() == SoftwareModuleType_OS) // process OS type first
            m_children.push_front(module);
        else
            m_children.push_back(module);
    }
    enableCallback();

    setStatus(ST_IDLE);
    return true;
}

bool DeploymentActionComposite::toJson(JValue& json)
{
    Component::toJson(json);
    json.put("status", m_status.getStatusStr());

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

void DeploymentActionComposite::onChangedStatus(SoftwareModuleComposite* softwareModule)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    if (m_listener)
        m_listener->onChangedStatus(this);
}

void DeploymentActionComposite::onCompletedDownload(SoftwareModuleComposite* softwareModule)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__, to_string(m_current));

    m_current++;
    if (m_current < m_children.size()) {
        if (!m_children[m_current]->startDownload()) {
            onFailedDownload((SoftwareModuleComposite*)m_children[m_current].get());
        }
        return;
    }

    setStatus(ST_DOWNLOAD_DONE);

    if (m_listener)
        m_listener->onCompletedDownload(this);
}

void DeploymentActionComposite::onCompletedInstall(SoftwareModuleComposite* softwareModule)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);
}

void DeploymentActionComposite::onFailedDownload(SoftwareModuleComposite* softwareModule)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    setStatus(ST_FAILED);

    if (m_listener)
        m_listener->onFailedDownload(this);
}

void DeploymentActionComposite::onFailedInstall(SoftwareModuleComposite* softwareModule)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);
}

bool DeploymentActionComposite::startDownload()
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == ST_DOWNLOAD)
        return true;
    if (m_status.getStatus() != ST_IDLE &&
        m_status.getStatus() != ST_PAUSED &&
        m_status.getStatus() != ST_DOWNLOAD_DONE)
        return false;

    m_current = 0;
    if (!m_children[m_current]->startDownload())
        return false;

    setStatus(ST_DOWNLOAD);
    return true;
}

bool DeploymentActionComposite::pauseDownload()
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == ST_PAUSED)
        return true;
    if (m_status.getStatus() != ST_DOWNLOAD)
        return false;
    if (m_current < 0 || m_current >= m_children.size())
        return false;

    if (!m_children[m_current]->pauseDownload())
        return false;

    setStatus(ST_PAUSED);
    return true;
}

bool DeploymentActionComposite::resumeDownload()
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == ST_DOWNLOAD)
        return true;
    if (m_status.getStatus() != ST_PAUSED)
        return false;
    if (m_current < 0 || m_current >= m_children.size())
        return false;

    if (!m_children[m_current]->resumeDownload())
        return false;

    setStatus(ST_DOWNLOAD);
    return true;
}

bool DeploymentActionComposite::cancelDownload()
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == ST_IDLE)
        return true;
    if (m_status.getStatus() != ST_DOWNLOAD &&
        m_status.getStatus() != ST_PAUSED &&
        m_status.getStatus() != ST_DOWNLOAD_DONE)
        return false;

    m_current = -1;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        (void) (*it)->cancelDownload();
    }

    setStatus(ST_IDLE);
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

bool DeploymentActionComposite::createRebootAlert(SoftwareModuleType type)
{
    string title = "System has been updated. Reboot required!";
    string message;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        std::shared_ptr<SoftwareModuleComposite> module = std::dynamic_pointer_cast<SoftwareModuleComposite>(*it);
        if (type == module->getType())
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

    return NotificationManager::getInstance().createAlert(title, message, buttons);
}

bool DeploymentActionComposite::setStatus(enum StatusType status)
{
    m_status.setStatus(status);

    if (m_listener)
        m_listener->onChangedStatus(this);

    return true;
}
