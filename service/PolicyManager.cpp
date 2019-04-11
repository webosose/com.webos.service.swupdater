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
    if (m_currentAction) {
        m_currentAction = nullptr;
    }
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
    m_statusPoint = nullptr;
    LS2Handler::getInstance().setListener(nullptr);
    HawkBitClient::getInstance().setListener(nullptr);

    return true;
}

void PolicyManager::onChangeStatus()
{
    static JValue prev;
    JValue cur = pbnjson::Object();

    // post subscription
    if (m_statusPoint && m_statusPoint->getSubscribersCount() > 0) {
        if (!m_currentAction) {
            cur.put("id", nullptr);
            cur.put("download", nullptr);
            cur.put("update", nullptr);
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

    if (!m_currentAction)
        return;

    // check installation status
    if (m_currentAction->getStatus().getStatus() == StatusType_COMPLETED) {
        JValue responsePayload;
        HawkBitClient::getInstance().postDeploymentActionSuccess(responsePayload, m_currentAction->getId());
        m_currentAction = nullptr;
    } else if (m_currentAction->getStatus().getStatus() == StatusType_FAILED) {
        JValue responsePayload;
        HawkBitClient::getInstance().postDeploymentActionFailed(responsePayload, m_currentAction->getId());
        m_currentAction = nullptr;
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

void PolicyManager::onInstall(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->install()) {
        return;
    }
}

void PolicyManager::onPause(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->pause()) {
        return;
    }
}

void PolicyManager::onResume(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->resume()) {
        return;
    }
}

void PolicyManager::onCancel(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (!m_currentAction) {
        responsePayload.put("errorText", "No active deployment action");
        return;
    }
    if (!m_currentAction->cancel()) {
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
            Logger::info(getClassName(), "Update is in progress");
        } else {
            Logger::error(getClassName(), "'Id' is not same - " + m_currentAction->getId() + " : " + id);
        }
        return;
    }
    m_currentAction = make_shared<DeploymentAction>();
    m_currentAction->fromJson(responsePayload);
    if (!m_currentAction->prepare()) {
        Logger::info(getClassName(), "Failed to download");
        return;
    }
    if (m_currentAction->isForceDownload()) {
        m_currentAction->install();
    }
}

void PolicyManager::onConfigDataAction(JValue& responsePayload)
{

}
