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

#include "PolicyManager.h"
#include "core/AbsAction.h"
#include "hawkbit/HawkBitInfo.h"
#include "ls2/AppInstaller.h"
#include "ls2/NotificationManager.h"
#include "ls2/SettingsService.h"
#include "ls2/SystemService.h"
#include "updater/AbsUpdater.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"
#include "util/Util.h"

const string PolicyManager::FILE_NON_VOLITILE_REBOOTCHECK = "/var/luna/preferences/swupdater_reboot_required";
const string PolicyManager::FILE_VOLITILE_REBOOTCHECK = "/tmp/swupdater_reboot_required";

gboolean PolicyManager::_tick(gpointer user_data)
{
    if (getInstance().m_pendingClearRequest) {
        Logger::info(getInstance().getClassName(), "Current Action is cleared");
        getInstance().m_currentAction = nullptr;
        getInstance().m_pendingClearRequest = false;
    }
    HawkBitClient::getInstance().poll();
    return G_SOURCE_CONTINUE;
}

PolicyManager::PolicyManager()
    : m_currentAction(nullptr)
    , m_proceedingJson(pbnjson::Object())
    , m_statusPoint(nullptr)
    , m_tickInterval(0)
    , m_tickSrc(0)
    , m_pendingClearRequest(false)
    , m_isAutoUpdateOn(false)
{
    setClassName("PolicyManager");
}

PolicyManager::~PolicyManager()
{
    if (m_currentAction) {
        m_currentAction = nullptr;
    }
}

bool PolicyManager::onInitialization()
{
    HawkBitClient::getInstance().setListener(this);
    LS2Handler::getInstance().setListener(this);
    AbsUpdaterFactory::getInstance().initialize(NULL);
    ConnectionManager::getInstance().getStatus(this);
    SettingsService::getInstance().getSystemSettings(this);

    m_statusPoint = new LS::SubscriptionPoint();
    m_statusPoint->setServiceHandle(&LS2Handler::getInstance());

    onPollingSleepAction(DEFAULT_TICK_INTERVAL);

    if (Util::isFileExist(FILE_NON_VOLITILE_REBOOTCHECK) &&
        !Util::isFileExist(FILE_VOLITILE_REBOOTCHECK)) {
        // poll now, os update is in progress.
        HawkBitClient::getInstance().poll();
    }

    return true;
}

bool PolicyManager::onFinalization()
{
    delete m_statusPoint;
    m_statusPoint = nullptr;
    AbsUpdaterFactory::getInstance().finalize();
    LS2Handler::getInstance().setListener(nullptr);
    HawkBitClient::getInstance().setListener(nullptr);

    return true;
}

void PolicyManager::onRequestStatusChange()
{
    postStatus();

    if (!m_currentAction)
        return;

    if (m_currentAction->getStatus().isWaitingReboot()) {
        // feedback proceeding message only when running status
        if (m_currentAction->getStatus().getStatus() == StatusType_RUNNING) {
            JValue proceedingJson = pbnjson::Object();
            m_currentAction->toProceedingJson(proceedingJson);
            if (proceedingJson != m_proceedingJson) {
                HawkBitClient::getInstance().proceeding(m_currentAction->getId(), proceedingJson.stringify());
                m_proceedingJson = proceedingJson.duplicate();
            }
            AbsBootloader::getBootloader().notifyUpdate();
            Logger::info(getClassName(), "Update installed, but reboot required.");
            Util::touchFile(FILE_NON_VOLITILE_REBOOTCHECK);
            Util::touchFile(FILE_VOLITILE_REBOOTCHECK);
            // Util::reboot();
            m_currentAction->createRebootAlert(SoftwareModuleType_OS);
        }
        return;
    }

    if (m_currentAction->getStatus().getStatus() == StatusType_RUNNING ||
        m_currentAction->getStatus().getStatus() == StatusType_PAUSED) {
        JValue proceedingJson = pbnjson::Object();
        m_currentAction->toProceedingJson(proceedingJson);
        if (proceedingJson != m_proceedingJson) { // softwaremodule status changed.
            if (m_currentAction->isOnlyOSModuleCompleted()) {
                m_currentAction->setWaitingReboot();
                return;
            }
            HawkBitClient::getInstance().proceeding(m_currentAction->getId(), proceedingJson.stringify());
            m_proceedingJson = proceedingJson.duplicate();
        }
    }

    if (m_currentAction->getStatus().getStatus() == StatusType_COMPLETED) {
        HawkBitClient::getInstance().postDeploymentAction(m_currentAction->getId(), true);
        m_pendingClearRequest = true;
        Logger::info(getClassName(), "Update completed.");
    } else if (m_currentAction->getStatus().getStatus() == StatusType_FAILED) {
        HawkBitClient::getInstance().postDeploymentAction(m_currentAction->getId(), false);
        m_pendingClearRequest = true;
        Logger::info(getClassName(), "Update failed.");
    }
}

void PolicyManager::onRequestProgressUpdate()
{
    postStatus();
}

void PolicyManager::onGetStatusSubscription(pbnjson::JValue subscriptionPayload)
{
    string wifiState;
    string wifiOnInternet;
    if (subscriptionPayload["returnValue"].asBool() &&
        JValueUtil::getValue(subscriptionPayload, "wifi", "state", wifiState) &&
        wifiState == "connected" &&
        JValueUtil::getValue(subscriptionPayload, "wifi", "onInternet", wifiOnInternet) &&
        wifiOnInternet == "yes") {
        Logger::info(getClassName(), "WiFi ON");
    } else {
        Logger::info(getClassName(), "WiFi OFF");
    }
}

void PolicyManager::onGetSystemSettingsSubscription(pbnjson::JValue subscriptionPayload)
{
    bool hasAutoUpdate = JValueUtil::hasKey(subscriptionPayload, "settings", "autoUpdate");
    // first return & the requesting key/value does not exist
    // To successfully subscribe from SettingsService, all the requesting key/values should exist.
    // So, if there is a key that has no value assigned, fill in the default value.
    if (subscriptionPayload.hasKey("subscribed") && !hasAutoUpdate) {
        JValue settingsObject = Object();
        settingsObject.put("autoUpdate", m_isAutoUpdateOn);
        if (SettingsService::getInstance().setSystemSettings(settingsObject)) {
            SettingsService::getInstance().getSystemSettings(this);
        } else {
            Logger::error(getClassName(), "Fail to subscribe SettingsService");
        }
        return;
    }

    if (hasAutoUpdate)
        m_isAutoUpdateOn = subscriptionPayload["settings"]["autoUpdate"].asBool();
    Logger::info(getClassName(), subscriptionPayload["settings"].stringify());
    if (m_isAutoUpdateOn && m_currentAction && m_currentAction->getStatus().getStatus() == StatusType_READY) {
        m_currentAction->start();
    }
}

void PolicyManager::onConnect(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    string tmp;
    if (!JValueUtil::getValue(requestPayload, "deviceId", tmp) || tmp.empty()) {
        JValue getInfoPayload = Object();
        if (!ConnectionManager::getInstance().getinfo(getInfoPayload)) {
            responsePayload.put("errorText", "Fail to get MAC address");
            return;
        }
        if (JValueUtil::getValue(getInfoPayload, "wiredInfo", "macAddress", tmp) && !tmp.empty()) {
            requestPayload.put("deviceId", "webOS_" + tmp);
        } else if (JValueUtil::getValue(getInfoPayload, "wifiInfo", "macAddress", tmp) && !tmp.empty()) {
            requestPayload.put("deviceId", "webOS_" + tmp);
        } else {
            responsePayload.put("errorText", "Fail to find wired/wireless MAC address");
            return;
        }
    }
    if (!JValueUtil::getValue(requestPayload, "address", tmp) || tmp.empty()) {
        responsePayload.put("errorText", "'address' does not exist");
        return;
    }
    if (!JValueUtil::getValue(requestPayload, "token", tmp) || tmp.empty()) {
        responsePayload.put("errorText", "'token' does not exist");
        return;
    }
    if (!HawkBitInfo::getInstance().setJson(requestPayload)) {
        responsePayload.put("errorText", "Fail to write payload");
    }
}

void PolicyManager::onGetStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (m_currentAction) {
        Logger::debug(getClassName(), "Try to post current action status");
        m_currentAction->toJson(responsePayload);
    } else {
        Logger::debug(getClassName(), "Current is null.");
        responsePayload.put("id", nullptr);
        responsePayload.put("status", nullptr);
    }
    if (m_statusPoint && request.isSubscription()) {
        Logger::debug(getClassName(), "Add subscription");
        m_statusPoint->subscribe(request);
        responsePayload.put("subscribed", true);
    }
}

void PolicyManager::onSetConfig(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    JValue data = requestPayload["data"];
    HawkBitClient::getInstance().putConfigData(data);
}

void PolicyManager::onStartDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->startDownload()) {
        responsePayload.put("errorText", "Cannot start download");
        return;
    }
}

void PolicyManager::onPauseDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->pauseDownload()) {
        responsePayload.put("errorText", "Cannot pause download");
        return;
    }
}

void PolicyManager::onResumeDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->resumeDownload()) {
        responsePayload.put("errorText", "Cannot resume download");
        return;
    }
}

void PolicyManager::onCancelDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->cancelDownload()) {
        responsePayload.put("errorText", "Cannot cancel download");
        return;
    }
}

void PolicyManager::onCancellationAction(JValue& responsePayload)
{
    string id;

    if (!JValueUtil::getValue(responsePayload, "id", id)) {
        Logger::error(getClassName(), "'id' does not exist");
        return;
    }
    if (m_currentAction) {
        if (m_currentAction->getId() != id) {
            Logger::error(getClassName(), "'Id' is not same - " + m_currentAction->getId() + " : " + id);
            return;
        }

        if (m_currentAction->cancel()) {
            Logger::info(getClassName(), "Update is canceled");
        } else {
            Logger::info(getClassName(), "Failed to cancel update");
        }
        m_currentAction = nullptr;
        return;
    }
}

void PolicyManager::onInstallationAction(JValue& responsePayload)
{
    string id;

    if (!JValueUtil::getValue(responsePayload, "id", id)) {
        Logger::error(getClassName(), "'id' does not exist");
        return;
    }
    if (m_currentAction) {
        if (m_currentAction->getId() == id) {
            Logger::info(getClassName(), "Update is in progress");
        } else {
            Logger::error(getClassName(), "'Id' is not same - " + m_currentAction->getId() + " : " + id);
        }
        return;
    }
    m_currentAction = make_shared<DeploymentActionComposite>();
    m_currentAction->setListener(this);
    m_currentAction->fromJson(responsePayload);

    // XXX: Temporarily added for {start,pause,resume,cancel} Download implementation
    if (m_currentAction->isForceDownload())
        m_currentAction->startDownload();
    return;

    // process actionHistory
    string messageStr;
    JValue message;
    if (JValueUtil::hasKey(responsePayload, "actionHistory", "messages") &&
        responsePayload["actionHistory"]["messages"].isArray() &&
        responsePayload["actionHistory"]["messages"].arraySize() > 0 &&
        responsePayload["actionHistory"]["messages"][0].asString(messageStr) == CONV_OK &&
        messageStr.find("Auto assignment by target filter") == string::npos && // set by hawkBit
        !(message = JDomParser::fromString(messageStr)).isNull()) {
        Logger::info(getClassName(), "ActionHistory", message.stringify());
        // restore previous proceeding message
        m_proceedingJson = message.duplicate();
        if (Util::isFileExist(FILE_NON_VOLITILE_REBOOTCHECK) &&
            !Util::isFileExist(FILE_VOLITILE_REBOOTCHECK)) {
            Logger::info(getClassName(), "Reboot detected!");
            m_currentAction->restoreActionHistory(message, true);
            AbsBootloader::getBootloader().setRebootOK();
            Util::removeFile(FILE_NON_VOLITILE_REBOOTCHECK);
        } else{
            Logger::info(getClassName(), "Waiting reboot..");
            m_currentAction->restoreActionHistory(message, false);
        }
        return;
    }

    if (!m_currentAction->prepare()) {
        Logger::info(getClassName(), "Failed to prepare update");
        return;
    }
    if (m_currentAction->isForceDownload() || m_isAutoUpdateOn) {
        m_currentAction->start();
    }
}

void PolicyManager::onPollingSleepAction(int seconds)
{
    if (m_tickInterval == seconds) {
        Logger::verbose(getClassName(), "Same polling interval. Ignored");
        return;
    }
    if (m_tickSrc > 0) {
        g_source_remove(m_tickSrc);
        m_tickSrc = 0;
        m_tickInterval = 0;
        Logger::info(getClassName(), "Tick source was removed");
    }
    m_tickSrc = g_timeout_add_seconds(seconds, _tick, nullptr);
    if (m_tickSrc < 0) {
        Logger::error(getClassName(), "Failed to start tick timer");
        return;
    }
    Logger::info(getClassName(), "Started tick timer");
    m_tickInterval = seconds;
}

void PolicyManager::onSettingConfigData()
{
    JValue osInfo = pbnjson::Object();
    JValue deviceInfo = pbnjson::Object();

    if (!SystemService::getInstance().queryOSInfo(osInfo) ||
        !SystemService::getInstance().queryDeviceInfo(deviceInfo)) {
        return;
    }

    JValue configData = pbnjson::Object();
    configData.put("device_name", deviceInfo["device_name"]);
    configData.put("webos_prerelease", osInfo["webos_prerelease"]);
    configData.put("webos_release_codename", osInfo["webos_release_codename"]);
    configData.put("webos_build_id", osInfo["webos_build_id"]);
    configData.put("webos_build_datetime", osInfo["webos_build_datetime"]);
    HawkBitClient::getInstance().putConfigData(configData);
}

void PolicyManager::onChangedStatus(DeploymentActionComposite* deploymentAction)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    postStatus();
}

void PolicyManager::onCompletedDownload(DeploymentActionComposite* deploymentAction)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);
}

void PolicyManager::onCompletedInstall(DeploymentActionComposite* deploymentAction)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    m_pendingClearRequest = true;
    HawkBitClient::getInstance().postDeploymentAction(m_currentAction->getId(), true);
    Logger::info(getClassName(), "Install completed.");
}

void PolicyManager::onFailedDownload(DeploymentActionComposite* deploymentAction)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    m_pendingClearRequest = true;
    HawkBitClient::getInstance().postDeploymentAction(m_currentAction->getId(), false);
    Logger::info(getClassName(), "Download failed.");
}

void PolicyManager::onFailedInstall(DeploymentActionComposite* deploymentAction)
{
    Logger::getInstance().debug(getClassName(), __FUNCTION__);

    m_pendingClearRequest = true;
    HawkBitClient::getInstance().postDeploymentAction(m_currentAction->getId(), false);
    Logger::info(getClassName(), "Install failed.");
}

void PolicyManager::postStatus()
{
    static JValue prev;
    JValue cur = pbnjson::Object();

    // post subscription
    if (!m_statusPoint || m_statusPoint->getSubscribersCount() == 0)
        return;

    if (!m_currentAction) {
        cur.put("id", nullptr);
        cur.put("status", nullptr);
    } else {
        m_currentAction->toJson(cur);
    }
    cur.put("subscribed", true);
    cur.put("returnValue", true);
    if (prev != cur) {
        LS2Handler::writeBLog("Post", "/getStatus", cur);
        m_statusPoint->post(cur.stringify().c_str());
        prev = cur.duplicate();
    }
}
