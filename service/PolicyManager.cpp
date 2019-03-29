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

void PolicyManager::onCompletedChildDownloading(IInstallable* installable)
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
}

void PolicyManager::onFailedChildDownloading(IInstallable* installable)
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
}

void PolicyManager::onProgressChildDownloading(IInstallable* installable)
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
}

void PolicyManager::onCompletedChildInstallation(IInstallable* installable)
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
}

void PolicyManager::onFailedChildInstallation(IInstallable* installable)
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
}

void PolicyManager::onProgressChildInstallation(IInstallable* installable)
{
    Logger::verbose(getClassName(), __FUNCTION__);
    JValue postPayload = pbnjson::Object();
    m_currentDeploymentAction->toJson(postPayload);
    cout << postPayload.stringify("    ") << endl;
}

void PolicyManager::onInstallSubscription(const string& id, const string& status)
{
}

bool PolicyManager::onCheck(JValue& responsePayload)
{
    return true;
}

bool PolicyManager::onInstall(JValue& responsePayload/**/)
{
    return true;
}

bool PolicyManager::onCancel(JValue& responsePayload/**/)
{
    return true;
}

bool PolicyManager::onGetStatus(JValue& responsePayload/**/)
{
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
    m_currentDeploymentAction->setListener(this);
    m_currentDeploymentAction->readyDownloading();
    m_currentDeploymentAction->readyInstallation();
    if (m_currentDeploymentAction->isForceDownload()) {
        m_currentDeploymentAction->startDownloading();
    }
    if (m_currentDeploymentAction->isForceUpdate()) {
        m_currentDeploymentAction->startInstallation();
    }
}

void PolicyManager::onConfigData(JValue& responsePayload)
{
}
