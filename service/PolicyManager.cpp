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

#include "core/AbsAction.h"
#include "PolicyManager.h"
#include "ls2/AppInstaller.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

PolicyManager::PolicyManager()
{
    setClassName("PolicyManager");
}

PolicyManager::~PolicyManager()
{
}

bool PolicyManager::onInitialization()
{
    HawkBitClient::getInstance().setListener(this);
    LS2Handler::getInstance().setListener(this);
    AppInstaller::getInstance().setListener(this);

    // first polling
    HawkBitClient::getInstance().poll(&HawkBitClient::getInstance());
    return true;
}

bool PolicyManager::onFinalization()
{
    AppInstaller::getInstance().setListener(nullptr);
    LS2Handler::getInstance().setListener(nullptr);
    HawkBitClient::getInstance().setListener(nullptr);

    return true;
}

void PolicyManager::onChangeStatus()
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
}

void PolicyManager::onInstallSubscription(const string& id, const string& status)
{
}

bool PolicyManager::onGetStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
    return true;
}

bool PolicyManager::onStartDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->start();
    return true;
}

bool PolicyManager::onPauseDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->pause();
    return true;
}

bool PolicyManager::onResumeDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->resume();
    return true;
}

bool PolicyManager::onCancelDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->cancel();
    return true;
}

bool PolicyManager::onStartInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->start();
    return true;
}

bool PolicyManager::onPauseInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->pause();
    return true;
}

bool PolicyManager::onResumeInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->resume();
    return true;
}

bool PolicyManager::onCancelInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentDeploymentAction) {
        return false;
    }
    m_currentDeploymentAction->cancel();
    return true;
}

void PolicyManager::onCancellationAction(JValue& responsePayload)
{
}

void PolicyManager::onInstallationAction(JValue& responsePayload)
{
    string id;

    if (!JValueUtil::getValue(responsePayload, "id", id)) {
        Logger::error(getClassName(), "'id' does not exist");
        return;
    }
    if (m_currentDeploymentAction) {
        if (m_currentDeploymentAction->getId() == id) {
            Logger::info(getClassName(), "Deployment is still in progress");
        } else {
            Logger::error(getClassName(), "Unknown error");
        }
        return;
    }
    m_currentDeploymentAction = make_shared<DeploymentAction>(responsePayload);
    if (!m_currentDeploymentAction->ready()) {
        Logger::info(getClassName(), "Failed to download");
        return;
    }
    if (!m_currentDeploymentAction->ready()) {
        Logger::info(getClassName(), "Failed to install");
        return;
    }
    if (m_currentDeploymentAction->isForceDownload()) {
        m_currentDeploymentAction->start();
    }
    if (m_currentDeploymentAction->isForceUpdate()) {
        m_currentDeploymentAction->start();
    }
}

void PolicyManager::onConfigData(JValue& responsePayload)
{

}
