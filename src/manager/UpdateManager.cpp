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

#include "UpdateManager.h"
#include "util/Logger.hpp"

const string UpdateManager::NAME = "com.webos.service.swupdater";

UpdateManager::UpdateManager()
    : LS::Handle(LS::registerService(NAME.c_str()))
    , m_progressWrapper(NULL)
    , m_prevProgress(0)
{
    m_prevStatus = "idle";
}

UpdateManager::~UpdateManager() {
}

bool UpdateManager::initialize(GMainLoop *mainLoop) {
    Logger::info("UpdateManager", "initialize", "");

    if (mainLoop == NULL) {
        return false;
    }

    try {
        attachToLoop(mainLoop);

        LS_CATEGORY_BEGIN(UpdateManager, "/")
            LS_CATEGORY_METHOD(status)
        LS_CATEGORY_END

        m_statusSubscription.setServiceHandle(this);

    } catch (const LS::Error& e) {
        Logger::error("UpdateManager", "Error in register category: %s", e.what());
    }

    return true;
}

bool UpdateManager::status(LSMessage &message) {
    LS::Message request(&message);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    ProgressManager::getInstance().initialize();
    ProgressManager::getInstance().setListener(this);

    bool subscribed = m_statusSubscription.subscribe(request);

    responsePayload.put("returnValue", true);
    responsePayload.put("subscribed", subscribed);
    responsePayload.put("status", m_prevStatus);
    responsePayload.put("progress", m_prevProgress);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

void UpdateManager::onUpdateProgress(string status, int progress)
{
    Logger::info("UpdateManager", "onUpdateProgress %s, %d", status.c_str(), progress, "");

    if (m_prevStatus == status && m_prevProgress == progress) {
        return;
    }

    m_prevStatus = status;
    m_prevProgress = progress;

    if (0 == m_statusSubscription.getSubscribersCount()) {
        return;
    }

    JValue responsePayload = Object();
    responsePayload.put("returnValue", true);
    responsePayload.put("subscribed", true);
    responsePayload.put("status", status);
    responsePayload.put("progress", progress);
    m_statusSubscription.post(responsePayload.stringify().c_str());
}



