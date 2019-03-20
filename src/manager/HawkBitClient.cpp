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

#include <core/HttpCall.h>
#include "HawkBitClient.h"

#include <curl/curl.h>
#include <glib.h>

#include "manager/Bootloader.h"
#include "util/Logger.h"
#include "util/Socket.h"

const string HawkBitClient::HAWKBIT_TENENT = "hawkbit_tenent";
const string HawkBitClient::HAWKBIT_URL = "hawkbit_url";
const string HawkBitClient::HAWKBIT_ID = "hawkbit_id";
const string HawkBitClient::HAWKBIT_TOKEN = "hawkbit_token";

const int HawkBitClient::POLLING_INTERVAL_DEFAULT = 600;

HawkBitClient::HawkBitClient()
    : m_pollingSrc(NULL)
    , m_pollingInterval(-1)
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
    string hawkBitTenet = Bootloader::getInstance().getEnv(HAWKBIT_TENENT);
    string hawkBitId = Bootloader::getInstance().getEnv(HAWKBIT_ID);

    m_hawkBitToken = Bootloader::getInstance().getEnv(HAWKBIT_TOKEN);
    m_hawkBitUrl = hawkBitUrl + "/" + hawkBitTenet + "/controller/v1/" + hawkBitId;

    if (hawkBitUrl.empty() || hawkBitTenet.empty() || m_hawkBitToken.empty()) {
        Logger::error(m_name, "HawkBit connection info could not be found");
        return false;
    }
    if (hawkBitId.empty()) {
        hawkBitId = Socket::getMacAddress("eth0");
        Bootloader::getInstance().setEnv(HAWKBIT_ID, hawkBitId);
    }

    start(POLLING_INTERVAL_DEFAULT);

    // first polling
    HawkBitClient::poll(this);

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

guint HawkBitClient::poll(gpointer data)
{
    HawkBitClient* self = (HawkBitClient*) data;
    if (!self) {
        Logger::error(HawkBitClient::getInstance().m_name, __FUNCTION__, "data is null");
        return G_SOURCE_REMOVE;
    }

    HttpCall httpCall(self->m_hawkBitUrl, HttpCall::MethodType_GET);
    httpCall.appendHeader("Accept", "application/hal+json");
    httpCall.appendHeader("Authorization", "GatewayToken " + self->m_hawkBitToken);
    if (!httpCall.perform()) {
        Logger::error(self->m_name, "Error while polling");
        return G_SOURCE_CONTINUE;
    }
    if (httpCall.getResponseCode() != 200L) {
        Logger::error(self->m_name, "HTTPCODE", to_string(httpCall.getResponseCode()));
        return G_SOURCE_CONTINUE;
    }

    JValue body = JDomParser::fromString(httpCall.getResponse().str());
    if (!body.isValid() || body.isNull()) {
        Logger::error(self->m_name, "Error while parsing");
        return G_SOURCE_CONTINUE;
    }

    string sleep;
    if (CONV_OK == body["config"]["polling"]["sleep"].asString(sleep)) {
        Logger::info(self->m_name, "config.polling.sleep", sleep);
        int hours, minutes, seconds;
        sscanf(sleep.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
        int pollingInterval = hours * 3600 + minutes * 60 + seconds;
        self->start(pollingInterval);
    }

    if (self->m_listener == nullptr)
        return G_SOURCE_CONTINUE;

    JValue _links = body["_links"];
    string href;
    if (_links.hasKey("cancelAction") && CONV_OK == _links["cancelAction"]["href"].asString(href)) {
        self->m_listener->onCancelUpdate();
        // TODO update_canceled
        Logger::info(self->m_name, "_links.cancelAction.href", href);
    } else if (_links.hasKey("deploymentBase") && CONV_OK == body["_links"]["deploymentBase"]["href"].asString(href)) {
        // TODO update_available
        Logger::info(self->m_name, "_links.deploymentBase.href", href);
    } else {
        // TODO no_update_available
    }

    return G_SOURCE_CONTINUE;
}
