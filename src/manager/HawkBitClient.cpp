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

#include "core/HttpCall.h"
#include "manager/Bootloader.h"
#include "util/Logger.h"
#include "util/Socket.h"

const string HawkBitClient::HAWKBIT_TENANT = "hawkbit_tenant";
const string HawkBitClient::HAWKBIT_URL = "hawkbit_url";
const string HawkBitClient::HAWKBIT_ID = "hawkbit_id";
const string HawkBitClient::HAWKBIT_TOKEN = "hawkbit_token";

const int HawkBitClient::POLLING_INTERVAL_DEFAULT = 600;

HawkBitClient::HawkBitClient()
    : m_pollingSrc(nullptr)
    , m_pollingInterval(-1)
    , m_listener(nullptr)
{
    setName("HawkBitClient");
}

HawkBitClient::~HawkBitClient()
{
}

bool HawkBitClient::onInitialization()
{
    // Init libcurl
    const CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        Logger::error(m_name, __FUNCTION__, curl_easy_strerror(code));
        return false;
    }

    // hawkBit configuration
    string hawkBitUrl = Bootloader::getInstance().getEnv(HAWKBIT_URL);
    string hawkBitTenant = Bootloader::getInstance().getEnv(HAWKBIT_TENANT);
    string hawkBitId = Bootloader::getInstance().getEnv(HAWKBIT_ID);

    if (hawkBitId.empty()) {
        hawkBitId = Socket::getMacAddress("eth0");
        Bootloader::getInstance().setEnv(HAWKBIT_ID, hawkBitId);
    }

    m_hawkBitToken = Bootloader::getInstance().getEnv(HAWKBIT_TOKEN);
    m_hawkBitUrl = hawkBitUrl + "/" + hawkBitTenant + "/controller/v1/" + hawkBitId;

    if (hawkBitUrl.empty() || hawkBitTenant.empty() || m_hawkBitToken.empty()) {
        Logger::error(m_name, "HawkBit connection info could not be found");
        return false;
    }

    start(POLLING_INTERVAL_DEFAULT);
    return true;
}

bool HawkBitClient::onFinalization()
{
    stop();
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

bool HawkBitClient::sendToServer(const string& url, HttpCall::MethodType method, const JValue& request, long& responseCode, JValue& response)
{
    HttpCall httpCall(url, method);
    httpCall.appendHeader("Accept", "application/hal+json");
    httpCall.appendHeader("Authorization", "GatewayToken " + m_hawkBitToken);
    httpCall.appendHeader("Content-Type", "application/json;charset=UTF-8");
    if (!request.isNull() && request.isValid()) {
        httpCall.setBody(request);
    }
    if (!httpCall.perform()) {
        Logger::error(m_name, "Error while requesting", url);
        return false;
    }
    responseCode = httpCall.getResponseCode();
    if (httpCall.getResponseCode() != 200L) {
        Logger::error(m_name, "HTTPCODE", to_string(httpCall.getResponseCode()));
        return false;
    }

    string bodyStr = httpCall.getResponse().str();
    if (!bodyStr.empty()) {
        JValue body = JDomParser::fromString(bodyStr);
        if (!body.isValid() || body.isNull()) {
            Logger::error(m_name, "Error while parsing: " + httpCall.getResponse().str());
            return false;
        }
        response = body;
    }
    return true;
}

void HawkBitClient::pollOnce()
{
    HawkBitClient::poll(this);
}

guint HawkBitClient::poll(gpointer data)
{
    HawkBitClient* self = (HawkBitClient*) data;
    if (!self) {
        Logger::error(HawkBitClient::getInstance().m_name, __FUNCTION__, "data is null");
        return G_SOURCE_REMOVE;
    }

    JValue response;
    long responseCode = 0;
    if (!self->sendToServer(self->m_hawkBitUrl, HttpCall::MethodType_GET, JValue(), responseCode, response)) {
        Logger::error(self->m_name, to_string(responseCode), "url: " + self->m_hawkBitUrl);
        return G_SOURCE_CONTINUE;
    }

    string sleep;
    if (CONV_OK == response["config"]["polling"]["sleep"].asString(sleep)) {
        Logger::info(self->m_name, "config.polling.sleep", sleep);
        int hours, minutes, seconds;
        sscanf(sleep.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
        int pollingInterval = hours * 3600 + minutes * 60 + seconds;
        self->start(pollingInterval);
    }

    if (self->m_listener == nullptr)
        return G_SOURCE_CONTINUE;

    shared_ptr<Action> action;
    JValue _links = response["_links"];
    string href;
    if (_links.hasKey("cancelAction") && CONV_OK == _links["cancelAction"]["href"].asString(href)) {
        Logger::info(self->m_name, "cancelAction.href", href);
        action = make_shared<ActionCancel>();
    } else if (_links.hasKey("deploymentBase") && CONV_OK == response["_links"]["deploymentBase"]["href"].asString(href)) {
        Logger::info(self->m_name, "deploymentBase.href", href);
        action = make_shared<ActionInstall>();
    } else {
        return G_SOURCE_CONTINUE;
    }

    responseCode = 0;
    response = JValue();
    if (!self->sendToServer(href, HttpCall::MethodType_GET, JValue(), responseCode, response)) {
        Logger::error(self->m_name, to_string(responseCode), "url: " + self->m_hawkBitUrl);
        return G_SOURCE_CONTINUE;
    }
    if (!action->fromJson(response)) {
        Logger::error(self->m_name, "Error while parsing action");
        return G_SOURCE_CONTINUE;
    }

    switch (action->getType()) {
    case ActionType::ActionType_CANCEL:
        self->m_listener->onCancelUpdate(action);
        break;
    case ActionType::ActionType_INSTALL:
        self->m_listener->onInstallUpdate(action);
        break;
    case ActionType::ActionType_NONE:
        break;
    }

    return G_SOURCE_CONTINUE;
}

/*
   long responseCode = 0;
   Feedback feedback(action->getId(), ExecutionType_CLOSED, FinishedType_SUCCESS);
   if (!HawkBitClient::getInstance().feedback(false, feedback, responseCode)) {
      Logger::error(m_name, to_string(responseCode), "[cancel] feedback failed");
   }
 */
bool HawkBitClient::feedback(bool isInstall, Feedback& feedback, long& responseCode)
{
    string url;
    if (isInstall) {
        url = m_hawkBitUrl + "/deploymentBase/" + feedback.getActionId() + "/feedback";
    } else {
        url = m_hawkBitUrl + "/cancelAction/" + feedback.getActionId() + "/feedback";
    }

    JValue request = Object();
    feedback.toJson(request);
    JValue response = Object();
    return sendToServer(url, HttpCall::MethodType_POST, request, responseCode, response);
}
