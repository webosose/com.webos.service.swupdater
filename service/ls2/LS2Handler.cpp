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

#include "LS2Handler.h"

#include <lunaservice.h>

#include "PolicyManager.h"
#include "ls2/AppInstaller.h"
#include "ls2/NotificationManager.h"
#include "util/Logger.h"

const string LS2Handler::NAME = "com.webos.service.swupdater";
const LSMethod LS2Handler::ROOT_METHODS[] = {
    { "getStatus", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { 0, 0 , LUNA_METHOD_FLAGS_NONE }
};

const LSMethod LS2Handler::INSTALL_METHODS[] = {
    { "start", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "pause", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "resume", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "cancel", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { 0, 0 , LUNA_METHOD_FLAGS_NONE }
};

const LSMethod LS2Handler::DOWNLOAD_METHODS[] = {
    { "start", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "pause", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "resume", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { "cancel", LS2Handler::onRequest , LUNA_METHOD_FLAGS_NONE },
    { 0, 0 , LUNA_METHOD_FLAGS_NONE }
};

bool LS2Handler::onRequest(LSHandle *sh, LSMessage *msg, void *category_context)
{
    static bool pending = false;

    // All LS2 requests are handled in queue
    LS2Handler::getInstance().m_requests.emplace(msg);

    // Not allowed recursive request handling To avoid unexpected behavior
    if (pending) {
        return true;
    }

    JValue requestPayload, responsePayload;
    pending = true;
    while (LS2Handler::getInstance().m_requests.size() != 0) {
        // pop a request
        LS::Message request = LS2Handler::getInstance().m_requests.front();
        LS2Handler::getInstance().m_requests.pop();

        // pre processing before request handling
        pre(request, requestPayload, responsePayload);
        string kind = request.getKind();

        if (LS2Handler::getInstance().m_listener == nullptr) {
            responsePayload.put("errorText", "API handler is null");
        } else if (kind == "/getStatus") {
            PolicyManager::getInstance().onGetStatus(request, requestPayload, responsePayload);
        } else if (kind == "/install/start") {
            PolicyManager::getInstance().onStartInstall(request, requestPayload, responsePayload);
        } else if (kind == "/install/pause") {
            PolicyManager::getInstance().onPauseInstall(request, requestPayload, responsePayload);
        } else if (kind == "/install/resume") {
            PolicyManager::getInstance().onResumeInstall(request, requestPayload, responsePayload);
        } else if (kind == "/install/cancel") {
            PolicyManager::getInstance().onCancelInstall(request, requestPayload, responsePayload);
        } else if (kind == "/download/start") {
            PolicyManager::getInstance().onStartDownload(request, requestPayload, responsePayload);
        } else if (kind == "/download/pause") {
            PolicyManager::getInstance().onPauseDownload(request, requestPayload, responsePayload);
        } else if (kind == "/download/resume") {
            PolicyManager::getInstance().onResumeDownload(request, requestPayload, responsePayload);
        } else if (kind == "/download/cancel") {
            PolicyManager::getInstance().onCancelDownload(request, requestPayload, responsePayload);
        } else {
            responsePayload.put("errorText", "Please extend API handlers");
        }
        post(request, requestPayload, responsePayload);
    }
    pending = false;
    return true;
}

LS2Handler::LS2Handler()
    : Handle(LS::registerService(NAME.c_str()))
{
    setClassName("LS2Handler");
    this->registerCategory("/", ROOT_METHODS, NULL, NULL);
    this->registerCategory("/download", DOWNLOAD_METHODS, NULL, NULL);
    this->registerCategory("/install", INSTALL_METHODS, NULL, NULL);
}

LS2Handler::~LS2Handler()
{
}

bool LS2Handler::onInitialization()
{
    attachToLoop(m_mainloop);
    AppInstaller::getInstance().initialize(m_mainloop);
    NotificationManager::getInstance().initialize(m_mainloop);
    return true;
}

bool LS2Handler::onFinalization()
{
    NotificationManager::getInstance().finalize();
    AppInstaller::getInstance().finalize();
    detach();
    return true;
}

void LS2Handler::pre(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    requestPayload = JDomParser::fromString(request.getPayload());
    responsePayload = pbnjson::Object();

    Logger::info(NAME, "Request", request.getMethod());
}

void LS2Handler::post(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (responsePayload.hasKey("errorText")) {
        Logger::warning(NAME, responsePayload["errorText"].asString());
        responsePayload.put("returnValue", false);
    }
    if (!responsePayload.hasKey("returnValue")) {
        responsePayload.put("returnValue", true);
    }
    request.respond(responsePayload.stringify().c_str());

    Logger::info(NAME, "Response", request.getMethod());
}
