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

#include "bootloader/AbsBootloader.h"
#include "hawkbit/HawkBitClient.h"
#include "ls2/NotificationManager.h"
#include "updater/AbsUpdater.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"
#include "util/Util.h"

const string DeploymentActionComposite::FILE_NON_VOLITILE_REBOOTCHECK = "/var/luna/preferences/swupdater_reboot_required";
const string DeploymentActionComposite::FILE_VOLITILE_REBOOTCHECK = "/tmp/swupdater_reboot_required";

DeploymentActionComposite::DeploymentActionComposite()
    : AbsAction()
    , m_isForceDownload(false)
    , m_isForceUpdate(false)
    , m_status()
{
    setClassName("DeploymentActionComposite");
    setType(ActionType_INSTALL);
}

DeploymentActionComposite::~DeploymentActionComposite()
{
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

    setStatus(StatusType_IDLE, false);
    return true;
}

bool DeploymentActionComposite::toJson(JValue& json)
{
    Composite::toJson(json);

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
    Logger::debug(getClassName(), __FUNCTION__);

    if (m_listener)
        m_listener->onChangedStatus(this);
}

void DeploymentActionComposite::onCompletedDownload(SoftwareModuleComposite* softwareModule)
{
    Logger::debug(getClassName(), __FUNCTION__, to_string(m_current));

    m_current++;
    if (m_current < m_children.size()) {
        if (!m_children[m_current]->startDownload()) {
            onFailedDownload((SoftwareModuleComposite*)m_children[m_current].get());
        }
        return;
    }

    setStatus(StatusType_DOWNLOAD_DONE);

    if (m_listener)
        m_listener->onCompletedDownload(this);
}

void DeploymentActionComposite::onCompletedInstall(SoftwareModuleComposite* softwareModule)
{
    Logger::debug(getClassName(), __FUNCTION__);

    JValue proceedingJson = pbnjson::Object();
    toProceedingJson(proceedingJson);

    if (softwareModule->getType() == SoftwareModuleType::SoftwareModuleType_OS) {
        // feedback to hawkBit (reboot required)
        proceedingJson.put("isRebootRequired", true);
        HawkBitClient::getInstance().proceeding(m_id, proceedingJson.stringify());
        // reboot toast
        AbsBootloader::getBootloader().notifyUpdate();
        Logger::info(getClassName(), "OS installed, and reboot required.");
        Util::touchFile(FILE_NON_VOLITILE_REBOOTCHECK);
        Util::touchFile(FILE_VOLITILE_REBOOTCHECK);
        createRebootAlert(SoftwareModuleType_OS);
        return;
    }

    // feedback to hawkBit (softwaremodule completed)
    HawkBitClient::getInstance().proceeding(m_id, proceedingJson.stringify());

    m_current++;
    if (m_current < m_children.size()) {
        if (!m_children[m_current]->startInstall()) {
            onFailedInstall((SoftwareModuleComposite*)m_children[m_current].get());
        }
        return;
    }

    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        JValue metadata = std::dynamic_pointer_cast<SoftwareModuleComposite>(*it)->getMetadata();
        string value = JValueUtil::getMeta(metadata, "installer");
        if (value == "opkg") {
            // TODO need to find better solution
            createRebootAlert(SoftwareModuleType_Application);
            break;
        }
    }

    setStatus(StatusType_INSTALL_DONE);

    if (m_listener)
        m_listener->onCompletedInstall(this);
}

void DeploymentActionComposite::onFailedDownload(SoftwareModuleComposite* softwareModule)
{
    Logger::debug(getClassName(), __FUNCTION__);

    setStatus(StatusType_FAILED);

    if (m_listener)
        m_listener->onFailedDownload(this);
}

void DeploymentActionComposite::onFailedInstall(SoftwareModuleComposite* softwareModule)
{
    Logger::debug(getClassName(), __FUNCTION__);

    setStatus(StatusType_FAILED);

    if (m_listener)
        m_listener->onFailedInstall(this);
}

bool DeploymentActionComposite::startDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == StatusType_DOWNLOAD)
        return true;
    if (m_status.getStatus() != StatusType_IDLE &&
        m_status.getStatus() != StatusType_PAUSED &&
        m_status.getStatus() != StatusType_DOWNLOAD_DONE)
        return false;

    m_current = 0;
    if (!m_children[m_current]->startDownload())
        return false;

    setStatus(StatusType_DOWNLOAD);
    return true;
}

bool DeploymentActionComposite::pauseDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == StatusType_PAUSED)
        return true;
    if (m_status.getStatus() != StatusType_DOWNLOAD)
        return false;
    if (m_current < 0 || m_current >= m_children.size())
        return false;

    if (!m_children[m_current]->pauseDownload())
        return false;

    setStatus(StatusType_PAUSED);
    return true;
}

bool DeploymentActionComposite::resumeDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == StatusType_DOWNLOAD)
        return true;
    if (m_status.getStatus() != StatusType_PAUSED)
        return false;
    if (m_current < 0 || m_current >= m_children.size())
        return false;

    if (!m_children[m_current]->resumeDownload())
        return false;

    setStatus(StatusType_DOWNLOAD);
    return true;
}

bool DeploymentActionComposite::cancelDownload()
{
    Logger::debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == StatusType_IDLE)
        return true;
    if (m_status.getStatus() != StatusType_DOWNLOAD &&
        m_status.getStatus() != StatusType_PAUSED &&
        m_status.getStatus() != StatusType_DOWNLOAD_DONE)
        return false;

    m_current = -1;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        (void) (*it)->cancelDownload();
    }

    setStatus(StatusType_IDLE);
    return true;
}

bool DeploymentActionComposite::startInstall()
{
    Logger::debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == StatusType_INSTALL)
        return true;
    if (m_status.getStatus() != StatusType_DOWNLOAD_DONE)
        return false;

    m_current = 0;
    if (!m_children[m_current]->startInstall())
        return false;

    setStatus(StatusType_INSTALL);
    return true;
}

bool DeploymentActionComposite::cancelInstall()
{
    Logger::debug(getClassName(), __FUNCTION__);

    if (m_status.getStatus() == StatusType_DOWNLOAD_DONE)
        return true;
    if (m_status.getStatus() != StatusType_INSTALL)
        return false;

    for (unsigned int i = 0; i < m_children.size(); i++) {
        shared_ptr<SoftwareModuleComposite> module = std::dynamic_pointer_cast<SoftwareModuleComposite>(m_children[i]);
        // Application cannot be canceled, if the installation starts.
        // Only OS can be canceled, before rebooting.
        if (module->getType() != SoftwareModuleType::SoftwareModuleType_OS && i <= m_current)
            return false;
    }

    m_current = -1;
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        (void) (*it)->cancelInstall();
    }

    setStatus(StatusType_DOWNLOAD_DONE);
    return true;
}

bool DeploymentActionComposite::toProceedingJson(JValue& json)
{
    json.put("status", m_status.getStatusStr());
    json.put("currentSoftwareModule", (int)m_current);
    return true;
}

bool DeploymentActionComposite::restoreActionHistory(const JValue& json) {
    bool isRebootDetected = Util::isFileExist(FILE_NON_VOLITILE_REBOOTCHECK) && !Util::isFileExist(FILE_VOLITILE_REBOOTCHECK);
    bool isRebootRequired = json["isRebootRequired"].asBool();
    string status = json["status"].asString();
    m_current = json["currentSoftwareModule"].asNumber<int>();

    if (isRebootRequired) {
        if (!isRebootDetected) {
            Logger::info(getClassName(), "Waiting reboot..");
            setStatus(StatusType_INSTALL, false);
            return true;
        }
        if (!AbsUpdaterFactory::getInstance().isUpdated()) {
            Logger::error(getClassName(), "Fail to apply OS update");
            onFailedInstall((SoftwareModuleComposite*)m_children[m_current].get());
            return true;
        }
        Logger::info(getClassName(), "Reboot detected!");
        AbsBootloader::getBootloader().setRebootOK();
        Util::removeFile(FILE_NON_VOLITILE_REBOOTCHECK);
    }

    if (status == Status::toString(StatusType_IDLE)) {
        setStatus(StatusType_IDLE, false);
        return true;
    } else if (status == Status::toString(StatusType_DOWNLOAD)) {
        if (!m_children[m_current]->startDownload())
            return false;
        setStatus(StatusType_DOWNLOAD, false);
        return true;
    } else if (status == Status::toString(StatusType_PAUSED)) {
        setStatus(StatusType_PAUSED, false);
        return true;
    } else if (status == Status::toString(StatusType_DOWNLOAD_DONE)) {
        setStatus(StatusType_DOWNLOAD_DONE, false);
        return true;
    } else if (status == Status::toString(StatusType_INSTALL)) {
        if (isRebootRequired) {
            // m_current has been installed already.
            m_current++;
        }
        if (m_current < m_children.size()) {
            if (!m_children[m_current]->startInstall())
                return false;
            setStatus(StatusType_INSTALL, false);
            return true;
        }
        setStatus(StatusType_INSTALL_DONE, false);
        if (m_listener)
            m_listener->onCompletedInstall(this);
        return true;
    }

    Logger::warning(getClassName(), "actionHistory", "Unknown status: " + status);
    return false;
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

bool DeploymentActionComposite::setStatus(enum StatusType status, bool doFeedback)
{
    m_status.setStatus(status);

    if (doFeedback) {
        JValue proceedingJson = pbnjson::Object();
        toProceedingJson(proceedingJson);
        HawkBitClient::getInstance().proceeding(m_id, proceedingJson.stringify());
    }

    if (m_listener)
        m_listener->onChangedStatus(this);

    return true;
}
