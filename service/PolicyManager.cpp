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
    : m_currentAction(nullptr)
    , m_statusPoint(nullptr)
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

    m_statusPoint = new LS::SubscriptionPoint();
    m_statusPoint->setServiceHandle(&LS2Handler::getInstance());

    // first polling
    HawkBitClient::getInstance().poll(&HawkBitClient::getInstance());
    return true;
}

bool PolicyManager::onFinalization()
{
    delete m_statusPoint;
    LS2Handler::getInstance().setListener(nullptr);
    HawkBitClient::getInstance().setListener(nullptr);

    return true;
}

void PolicyManager::onStateChanged(State *installer, enum StateType prev, enum StateType cur)
{
    onChangeStatus();
    if (m_currentAction->isComplete()) {
        JValue responsePayload;
        HawkBitClient::getInstance().postDeploymentAction(responsePayload, m_currentAction->getId());
        m_currentAction = nullptr;
        onChangeStatus();
    }
}

void PolicyManager::onChangeStatus()
{
    static JValue prev;
    JValue current = pbnjson::Object();

    if (!m_currentAction) {
        current.put("id", nullptr);
        current.put("download", nullptr);
        current.put("update", nullptr);
    } else {
        m_currentAction->toJson(current);
    }
    current.put("subscribed", true);
    current.put("returnValue", true);
    if (prev != current) {
        m_statusPoint->post(current.stringify().c_str());
        prev = current.duplicate();
    }
}

void PolicyManager::onGetStatus(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("id", nullptr);
        responsePayload.put("download", nullptr);
        responsePayload.put("update", nullptr);
    } else {
        m_currentAction->toJson(responsePayload);
    }
    if (request.isSubscription()) {
        m_statusPoint->subscribe(request);
        responsePayload.put("subscribed", true);
    }
}

void PolicyManager::onStartDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->start(true)) {
        return;
    }
}

void PolicyManager::onPauseDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->pause(true)) {
        return;
    }
}

void PolicyManager::onResumeDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        return;
    }
    if (!m_currentAction->resume(true)) {
        return;
    }
}

void PolicyManager::onCancelDownload(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        return;
    }
    if (!m_currentAction->cancel(true)) {
        return;
    }
}

void PolicyManager::onStartInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        return;
    }
    if (!m_currentAction->start(false)) {
        return;
    }
}

void PolicyManager::onPauseInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        return;
    }
    if (!m_currentAction->pause(false)) {
        return;
    }
}

void PolicyManager::onResumeInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        return;
    }
    if (!m_currentAction->resume(false)) {
        return;
    }
}

void PolicyManager::onCancelInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        return;
    }
    if (!m_currentAction->cancel(false)) {
        return;
    }
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
    if (m_currentAction) {
        if (m_currentAction->getId() == id) {
            Logger::info(getClassName(), "Deployment is still in progress");
        } else {
            Logger::error(getClassName(), "Unknown error");
        }
        return;
    }
    m_currentAction = make_shared<DeploymentAction>(responsePayload);
    m_currentAction->getUpdateState().setCallback( // @suppress("Invalid arguments")
        std::bind(&PolicyManager::onStateChanged,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3
         )
    );
    m_currentAction->getDownloadState().setCallback( // @suppress("Invalid arguments")
        std::bind(&PolicyManager::onStateChanged,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3
        )
    );
    if (!m_currentAction->ready(true)) {
        Logger::info(getClassName(), "Failed to download");
        return;
    }
    if (!m_currentAction->ready(false)) {
        Logger::info(getClassName(), "Failed to install");
        return;
    }
    if (m_currentAction->isForceDownload()) {
        m_currentAction->start(true);
    }
    if (m_currentAction->isForceUpdate()) {
        m_currentAction->start(false);
    }
}

void PolicyManager::onConfigDataAction(JValue& responsePayload)
{

}
