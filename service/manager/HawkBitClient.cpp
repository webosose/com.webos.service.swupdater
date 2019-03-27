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

#include "HawkBitClient.h"

#include <curl/curl.h>
#include <glib.h>
#include <hardware/AbsHardware.h>
#include "core/HttpCall.h"
#include "util/Logger.h"
#include "util/Socket.h"

const string HawkBitClient::HAWKBIT_TENANT = "hawkbit_tenant";
const string HawkBitClient::HAWKBIT_URL = "hawkbit_url";
const string HawkBitClient::HAWKBIT_ID = "hawkbit_id";
const string HawkBitClient::HAWKBIT_TOKEN = "hawkbit_token";

const int HawkBitClient::POLLING_INTERVAL_DEFAULT = 15;

HawkBitClient::HawkBitClient()
    : m_pollingSrc(nullptr)
    , m_pollingInterval(-1)
    , m_listener(nullptr)
{
    setClassName("HawkBitClient");
}

HawkBitClient::~HawkBitClient()
{
}

bool HawkBitClient::onInitialization()
{
    if (!HttpCall::initialize()) {
        return false;
    }

    // hawkBit configuration
    string hawkBitUrl = AbsHardware::getHardware().getEnv(HAWKBIT_URL);
    string hawkBitTenant = AbsHardware::getHardware().getEnv(HAWKBIT_TENANT);
    string hawkBitId = AbsHardware::getHardware().getEnv(HAWKBIT_ID);

    if (hawkBitId.empty()) {
        hawkBitId = Socket::getMacAddress("eth0");
        AbsHardware::getHardware().setEnv(HAWKBIT_ID, hawkBitId);
    }

    m_hawkBitToken = AbsHardware::getHardware().getEnv(HAWKBIT_TOKEN);
    m_hawkBitUrl = hawkBitUrl + "/" + hawkBitTenant + "/controller/v1/" + hawkBitId;

    if (hawkBitUrl.empty() || hawkBitTenant.empty() || m_hawkBitToken.empty()) {
        Logger::error(getClassName(), "HawkBit connection info could not be found");
        return false;
    }

    start(POLLING_INTERVAL_DEFAULT);
    return true;
}

bool HawkBitClient::onFinalization()
{
    stop();
    HttpCall::finalize();
    return true;
}

bool HawkBitClient::sendFeedback(Action& action, Feedback& feedback)
{
    string url;
    if (action.getType() == ActionType_INSTALL) {
        url = m_hawkBitUrl + "/deploymentBase/" + feedback.getActionId() + "/feedback";
    } else {
        url = m_hawkBitUrl + "/cancelAction/" + feedback.getActionId() + "/feedback";
    }

    HttpCall httpCall(MethodType_POST, url, m_hawkBitToken);
    JValue requestPayload = pbnjson::Object();

    feedback.toJson(requestPayload);
    httpCall.setBody(requestPayload);

    if (!httpCall.performSync()) {
        Logger::error(getClassName(), "Failed to post feedback");
        return false;
    }
    return true;
}

bool HawkBitClient::downloadApplication(Chunk& chunk)
{
    HttpCall call(MethodType_GET, chunk.getArtifacts().front().getDownloadHttp(), m_hawkBitToken);
    cout << "start download" << endl;
    cout << chunk.getJson().stringify("    ") << endl;
    call.performSync();
    cout << "size - " << call.getResponsePayloadSize() << endl;
    cout << "end download" << endl;
    return true;
}

void HawkBitClient::start(int seconds)
{
    if (seconds == 0) {
        seconds = POLLING_INTERVAL_DEFAULT;
    }
    if (isStarted() && seconds == m_pollingInterval) {
        return;
    }
    stop();

    m_pollingSrc = g_timeout_source_new_seconds(seconds);
    g_source_set_callback(m_pollingSrc, (GSourceFunc)&HawkBitClient::poll, this, NULL);
    g_source_attach(m_pollingSrc, g_main_context_default());
    g_source_unref(m_pollingSrc);
    m_pollingInterval = seconds;
}

bool HawkBitClient::isStarted()
{
    return (m_pollingSrc && !g_source_is_destroyed(m_pollingSrc));
}

void HawkBitClient::stop()
{
    if (isStarted()) {
        g_source_destroy(m_pollingSrc);
        m_pollingSrc = nullptr;
    }
}

void HawkBitClient::checkPollingInterval(const JValue& responsePayload)
{
    // TODO: this should be removed after demo
    return;

    string sleep;
    if (CONV_OK == responsePayload["config"]["polling"]["sleep"].asString(sleep)) {
        Logger::info(getClassName(), "config.polling.sleep", sleep);
        int hours, minutes, seconds;
        sscanf(sleep.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
        int pollingInterval = hours * 3600 + minutes * 60 + seconds;
        start(pollingInterval);
    }
}

enum ActionType HawkBitClient::checkLink(const JValue& responsePayload, string& link)
{
    enum ActionType type = ActionType_NONE;

    if (!responsePayload.hasKey("_links"))
        return type;

    if (responsePayload["_links"].hasKey("cancelAction") &&
        responsePayload["_links"]["cancelAction"].hasKey("href") &&
        responsePayload["_links"]["cancelAction"]["href"].asString(link) == CONV_OK) {
        type = ActionType_CANCEL;
    } else if (responsePayload["_links"].hasKey("deploymentBase") &&
               responsePayload["_links"]["deploymentBase"].hasKey("href") &&
               responsePayload["_links"]["deploymentBase"]["href"].asString(link) == CONV_OK) {
        type = ActionType_INSTALL;
    }

    return type;
}

guint HawkBitClient::poll(gpointer data)
{
    HawkBitClient* self = (HawkBitClient*) data;
    enum ActionType type = ActionType_NONE;
    ActionInstall actionInstall;
    JValue responsePayload;
    string link = "";
    Action action;

    Logger::info(self->getClassName(), "== POLLING START ==");
    Logger::verbose(self->getClassName(), "checking hawkBit server request");
    if (!self->getRequest(responsePayload)) {
        Logger::error(self->getClassName(), "Failed to get request from hawkBit server");
        goto Done;
    }

    cout << responsePayload.stringify("    ") << endl;
    Logger::verbose(self->getClassName(), "Checking polling interval");
    self->checkPollingInterval(responsePayload);
    if (self->m_listener == nullptr) {
        Logger::error(self->getClassName(), "Listener is null");
        goto Done;
    }

    Logger::verbose(self->getClassName(), "Checking 'link' from request");
    type = self->checkLink(responsePayload, link);
    if (link.empty() || type == ActionType_NONE) {
        Logger::error(self->getClassName(), "Action type is none");
        goto Done;
    }

    Logger::verbose(self->getClassName(), "Getting 'action' from hawkBit server");
    if (!self->getAction(link, responsePayload)) {
        Logger::error(self->getClassName(), "Failed to get link");
        goto Done;
    }

    switch (type) {
    case ActionType_CANCEL:
        Logger::verbose(self->getClassName(), "Try to handle 'cancel' action");
        action.setType(type);
        action.fromJson(responsePayload);
        self->m_listener->onCancelUpdate(action);
        break;

    case ActionType_INSTALL:
        Logger::verbose(self->getClassName(), "Try to handle 'install' action");
        actionInstall.fromJson(responsePayload);
        self->m_listener->onInstallUpdate(actionInstall);
        break;

    default:
        break;
    }

Done:
    Logger::info(self->getClassName(), "== POLLING END ==");
    return G_SOURCE_CONTINUE;
}

bool HawkBitClient::getRequest(JValue& responsePayload)
{
    HttpCall httpCall(MethodType_GET, m_hawkBitUrl, m_hawkBitToken);

    if (!httpCall.performSync()) {
        Logger::error(getClassName(), "Failed to get request");
        return false;
    }

    responsePayload = JDomParser::fromString(httpCall.getResponsePayload());
    return true;
}

bool HawkBitClient::getAction(const string& link, JValue& responsePayload)
{
    HttpCall httpCall(MethodType_GET, link, m_hawkBitToken);
    if (!httpCall.performSync()) {
        Logger::error(getClassName(), "Failed to get action");
        return false;
    }

    responsePayload = JDomParser::fromString(httpCall.getResponsePayload());
    return true;
}
