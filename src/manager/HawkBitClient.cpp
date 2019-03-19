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

#include "manager/Bootloader.h"
#include "util/HttpCall.h"
#include "util/Logger.h"
#include "util/Util.h"

const string HawkBitClient::BLKEY_HAWKBIT_TENENT = "hawkbit_tenent";
const string HawkBitClient::BLKEY_HAWKBIT_URL = "hawkbit_url";
const string HawkBitClient::BLKEY_HAWKBIT_ID = "hawkbit_id";
const string HawkBitClient::BLKEY_HAWKBIT_TOKEN = "hawkbit_token";

const int HawkBitClient::POLLING_INTERVAL_DEFAULT = 600;

HawkBitClient::HawkBitClient()
    : m_pollingSrc(NULL)
    , m_pollingIntervalSec(-1)
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
        Logger::error(m_name, "Failed in curl_global_init", curl_easy_strerror(code));
        return false;
    }

    // hawkBit configuration
    m_url = Bootloader::getInstance().getEnv(BLKEY_HAWKBIT_URL);
    m_tenent = Bootloader::getInstance().getEnv(BLKEY_HAWKBIT_TENENT);
    m_token = Bootloader::getInstance().getEnv(BLKEY_HAWKBIT_TOKEN);
    m_controllerId = Bootloader::getInstance().getEnv(BLKEY_HAWKBIT_ID);
    if (m_url.empty() || m_tenent.empty() || m_token.empty()) {
        Logger::error(m_name, "HawkBit connection info could not be found");
        return false;
    }
    if (m_controllerId.empty()) {
        m_controllerId = Util::getMacAddress("eth0");
        Bootloader::getInstance().setEnv(BLKEY_HAWKBIT_ID, m_controllerId);
    }

    registerPollingInterval(POLLING_INTERVAL_DEFAULT);

    // first polling
    HawkBitClient::poll(this);

    return true;
}

bool HawkBitClient::onFinalization()
{
    unregisterPollingInterval();
    return true;
}

void HawkBitClient::registerPollingInterval(int seconds)
{
    if (m_pollingSrc && !g_source_is_destroyed(m_pollingSrc) && seconds == m_pollingIntervalSec) {
        return;
    }

    unregisterPollingInterval();

    m_pollingSrc = g_timeout_source_new_seconds(seconds);
    g_source_set_callback(m_pollingSrc, (GSourceFunc)&HawkBitClient::poll, this, NULL);
    g_source_attach(m_pollingSrc, g_main_context_default());
    g_source_unref(m_pollingSrc);
    m_pollingIntervalSec = seconds;
}

void HawkBitClient::unregisterPollingInterval()
{
    if (m_pollingSrc && !g_source_is_destroyed(m_pollingSrc)) {
        g_source_destroy(m_pollingSrc);
    }
}

guint HawkBitClient::poll(gpointer data)
{
    HawkBitClient* self = (HawkBitClient*) data;
    if (!self) {
        Logger::error("HawkBitClient", __FUNCTION__, "data is null");
        return G_SOURCE_REMOVE;
    }

    string url = self->m_url + "/" + self->m_tenent + "/controller/v1/" + self->m_controllerId;

    HttpCall httpCall(url, HttpCall::kMethodGET);
    httpCall.appendHeader("Accept", "application/hal+json");
    httpCall.appendHeader("Authorization", "GatewayToken " + self->m_token);
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

    string pollingInterval;
    if (CONV_OK == body["config"]["polling"]["sleep"].asString(pollingInterval)) {
        Logger::info(self->m_name, "config.polling.sleep", pollingInterval);
        int hours, minutes, seconds;
        sscanf(pollingInterval.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
        int pollingIntervalSec = hours * 3600 + minutes * 60 + seconds;
        self->registerPollingInterval(pollingIntervalSec);
    }

    JValue linksObj = body["_links"];
    string configDataHref;
    if (linksObj.hasKey("configData") &&
            CONV_OK == linksObj["configData"]["href"].asString(configDataHref)) {
        Logger::info(self->m_name, "_links.configData.href", configDataHref);
    }

    string cancelActionHref;
    string deploymentBaseHref;
    if (linksObj.hasKey("cancelAction") &&
            CONV_OK == linksObj["cancelAction"]["href"].asString(cancelActionHref)) {
        // TODO update_canceled
        Logger::info(self->m_name, "_links.cancelAction.href", cancelActionHref);
    } else if (linksObj.hasKey("deploymentBase") &&
            CONV_OK == body["_links"]["deploymentBase"]["href"].asString(deploymentBaseHref)) {
        // TODO update_available
        Logger::info(self->m_name, "_links.deploymentBase.href", deploymentBaseHref);
    } else {
        // TODO no_update_available
    }

    return G_SOURCE_CONTINUE;
}
