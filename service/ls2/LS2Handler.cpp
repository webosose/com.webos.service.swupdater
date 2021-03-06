// Copyright (c) 2019-2020 LG Electronics, Inc.
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
#include "Setting.h"
#include "hawkbit/HawkBitInfo.h"
#include "ls2/AppInstaller.h"
#include "ls2/ConnectionManager.h"
#include "ls2/NotificationManager.h"
#include "ls2/SettingsService.h"
#include "ls2/SystemService.h"
#include "util/Logger.h"

const unsigned long LS2Handler::LSCALL_TIMEOUT = 5000;
const string LS2Handler::NAME = NAME_SWUPDATER;
const LSMethod LS2Handler::ROOT_METHODS[] = {
    { "getStatus", LS2Handler::onRequest, LUNA_METHOD_FLAGS_NONE },
    { "startDownload", LS2Handler::onRequest, LUNA_METHOD_FLAGS_NONE },
    { "pauseDownload", LS2Handler::onRequest, LUNA_METHOD_FLAGS_NONE },
    { "resumeDownload", LS2Handler::onRequest, LUNA_METHOD_FLAGS_NONE },
    { "cancelDownload", LS2Handler::onRequest, LUNA_METHOD_FLAGS_NONE },
    { "startInstall", LS2Handler::onRequest, LUNA_METHOD_FLAGS_NONE },
    { "cancelInstall", LS2Handler::onRequest, LUNA_METHOD_FLAGS_NONE },
    { 0, 0, LUNA_METHOD_FLAGS_NONE }
};

bool LS2Handler::onRequest(LSHandle *sh, LSMessage *msg, void *category_context)
{
    // All LS2 requests are handled in queue
    LS2Handler::getInstance().m_requests.emplace(msg);

    if (!PolicyManager::getInstance().isInitalized()) {
        Logger::info(LS2Handler::getInstance().getClassName(), "Requested " + string(LSMessageGetKind(msg)) + ", but waiting to be initialized");
        return true;
    }

    return LS2Handler::getInstance().handleRequest();
}

bool LS2Handler::handleRequest()
{
    static bool pending = false;

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
        before(request, requestPayload, responsePayload);
        string kind = request.getKind();

        if (requestPayload.isNull()) {
            responsePayload.put("errorText", "Json parsing error");
        } else if (LS2Handler::getInstance().m_listener == nullptr) {
            responsePayload.put("errorText", "API handler is null");
        } else if (!HawkBitInfo::getInstance().isHawkBitInfoSet()) {
            responsePayload.put("errorText", "HawkBitInfo is NOT set");
        } else if (kind == "/getStatus") {
            PolicyManager::getInstance().onGetStatus(request, requestPayload, responsePayload);
        } else if (kind == "/setConfig") {
            PolicyManager::getInstance().onSetConfig(request, requestPayload, responsePayload);
        } else if (kind == "/startDownload") {
            PolicyManager::getInstance().onStartDownload(request, requestPayload, responsePayload);
        } else if (kind == "/pauseDownload") {
            PolicyManager::getInstance().onPauseDownload(request, requestPayload, responsePayload);
        } else if (kind == "/resumeDownload") {
            PolicyManager::getInstance().onResumeDownload(request, requestPayload, responsePayload);
        } else if (kind == "/cancelDownload") {
            PolicyManager::getInstance().onCancelDownload(request, requestPayload, responsePayload);
        } else if (kind == "/startInstall") {
            PolicyManager::getInstance().onStartInstall(request, requestPayload, responsePayload);
        } else if (kind == "/cancelInstall") {
            PolicyManager::getInstance().onCancelInstall(request, requestPayload, responsePayload);
        } else {
            responsePayload.put("errorText", "Please extend API handlers");
        }
        after(request, requestPayload, responsePayload);
    }
    pending = false;
    return true;
}

LS2Handler::LS2Handler()
    : Handle(LS::registerService(NAME.c_str()))
{
    setClassName("LS2Handler");
    this->registerCategory("/", ROOT_METHODS, NULL, NULL);
}

LS2Handler::~LS2Handler()
{
}

bool LS2Handler::onInitialization()
{
    m_connection = PolicyManager::getInstance().signalOnInitialized.connect(std::bind(&LS2Handler::handleRequest, this));
    attachToLoop(m_mainloop);
    AppInstaller::getInstance().initialize(m_mainloop);
    ConnectionManager::getInstance().initialize(m_mainloop);
    NotificationManager::getInstance().initialize(m_mainloop);
    SettingsService::getInstance().initialize(m_mainloop);

    return true;
}

bool LS2Handler::onFinalization()
{
    SettingsService::getInstance().finalize();
    NotificationManager::getInstance().finalize();
    ConnectionManager::getInstance().finalize();
    AppInstaller::getInstance().finalize();
    detach();
    m_connection.disconnect();
    return true;
}

void LS2Handler::before(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    requestPayload = JDomParser::fromString(request.getPayload());
    responsePayload = pbnjson::Object();

    writeALog("Request", request, requestPayload);
}

void LS2Handler::after(LS::Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (responsePayload.hasKey("errorText")) {
        Logger::warning(NAME, responsePayload["errorText"].asString());
        responsePayload.put("returnValue", false);
    }
    if (!responsePayload.hasKey("returnValue")) {
        responsePayload.put("returnValue", true);
    }
    request.respond(responsePayload.stringify().c_str());
    writeALog("Response", request, requestPayload);
}

void LS2Handler::writeALog(const string& type, LS::Message& request, JValue& payload)
{
    string log = request.getKind();
    log += " - " + string(request.getSenderServiceName() ? request.getSenderServiceName() : request.getApplicationID());

    if (Logger::getInstance().isVerbose()) {
        log += "\n" + payload.stringify("    ");
        Logger::verbose(NAME, type, log);
    } else {
        Logger::debug(NAME, type, log);
    }
}

void LS2Handler::writeBLog(const string& type, const string& kind, JValue& payload)
{
    string log = kind;

    if (Logger::getInstance().isVerbose()) {
        log += "\n" + payload.stringify("    ");
        Logger::verbose(NAME, type, log);
    } else {
        Logger::debug(NAME, type, log);
    }
}
